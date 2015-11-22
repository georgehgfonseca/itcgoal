
/*****************************************************************************/
/*                                                                           */
/*  THE KHE HIGH SCHOOL TIMETABLING ENGINE                                   */
/*  COPYRIGHT (C) 2010 Jeffrey H. Kingston                                   */
/*                                                                           */
/*  Jeffrey H. Kingston (jeff@it.usyd.edu.au)                                */
/*  School of Information Technologies                                       */
/*  The University of Sydney 2006                                            */
/*  AUSTRALIA                                                                */
/*                                                                           */
/*  This program is free software; you can redistribute it and/or modify     */
/*  it under the terms of the GNU General Public License as published by     */
/*  the Free Software Foundation; either Version 3, or (at your option)      */
/*  any later version.                                                       */
/*                                                                           */
/*  This program is distributed in the hope that it will be useful,          */
/*  but WITHOUT ANY WARRANTY; without even the implied warranty of           */
/*  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the            */
/*  GNU General Public License for more details.                             */
/*                                                                           */
/*  You should have received a copy of the GNU General Public License        */
/*  along with this program; if not, write to the Free Software              */
/*  Foundation, Inc., 59 Temple Place, Suite 330, Boston MA 02111-1307 USA   */
/*                                                                           */
/*  FILE:         khe_event.c                                                */
/*  DESCRIPTION:  An event                                                   */
/*                                                                           */
/*****************************************************************************/
#include "khe_interns.h"
#define DEBUG2 0


/*****************************************************************************/
/*                                                                           */
/*  KHE_EVENT - an event                                                     */
/*                                                                           */
/*****************************************************************************/

struct khe_event_rec {
  void				*back;			/* back pointer      */
  KHE_INSTANCE			instance;		/* enclosing instance*/
  char				*id;			/* Id                */
  char				*name;			/* Name              */
  char				*color;			/* optional Color    */
  int				duration;		/* duration          */
  int				workload;		/* workload          */
  KHE_TIME			preassigned_time;	/* optional time     */
  int				index;			/* index in ins      */
  bool				partition_admissible;	/* when inferring    */
  ARRAY_KHE_EVENT_RESOURCE	event_resources;	/* event resources   */
  ARRAY_KHE_EVENT_RESOURCE_GROUP event_resource_groups;	/* resource groups   */
  ARRAY_KHE_CONSTRAINT		constraints;		/* constraints       */
  ARRAY_KHE_EVENT_GROUP		user_event_groups;	/* event groups      */
  KHE_EVENT_GROUP		singleton_event_group;	/* singleton         */
  /* ARRAY_KHE_TIME_GROUP	domains; */		/* time domains      */
};


/*****************************************************************************/
/*                                                                           */
/*  Submodule "construction and query"                                       */
/*                                                                           */
/*****************************************************************************/

/*****************************************************************************/
/*                                                                           */
/*  bool KheEventMake(KHE_INSTANCE ins, char *id, char *name, char *color,   */
/*    int workload, KHE_TIME preassigned_time, KHE_EVENT *e)                 */
/*                                                                           */
/*  Make a new event, add it to the instance, and return it.                 */
/*                                                                           */
/*****************************************************************************/

bool KheEventMake(KHE_INSTANCE ins, char *id, char *name, char *color,
  int duration, int workload, KHE_TIME preassigned_time, KHE_EVENT *e)
{
  KHE_EVENT res;
  MAssert(!KheInstanceComplete(ins),
    "KheEventMake called after KheInstanceMakeEnd");
  if( id != NULL && KheInstanceRetrieveEvent(ins, id, &res) )
  {
    *e = NULL;
    return false;
  }
  MMake(res);
  res->back = NULL;
  res->instance = ins;
  res->id = id;
  res->name = name;
  res->color = color;
  res->duration = duration;
  res->workload = workload;
  res->preassigned_time = preassigned_time;
  res->index = KheInstanceEventCount(ins);
  res->partition_admissible = false;
  MArrayInit(res->event_resources);
  MArrayInit(res->event_resource_groups);
  MArrayInit(res->constraints);
  MArrayInit(res->user_event_groups);
  res->singleton_event_group = KheEventGroupMakeInternal(
    KHE_EVENT_GROUP_TYPE_SINGLETON, ins, KHE_EVENT_GROUP_KIND_ORDINARY,
    NULL, NULL);
  KheEventGroupAddEventInternal(res->singleton_event_group, res);
  /* MArrayInit(res->domains); */  /* set after instance is complete */
  KheEventGroupAddEventInternal(KheInstanceFullEventGroup(ins), res);
  KheInstanceAddEvent(ins, res);
  *e = res;
  return true;
}


/*****************************************************************************/
/*                                                                           */
/*  void KheEventSetBack(KHE_EVENT e, void *back)                            */
/*                                                                           */
/*  Set the back pointer of e.                                               */
/*                                                                           */
/*****************************************************************************/

void KheEventSetBack(KHE_EVENT e, void *back)
{
  MAssert(!KheInstanceComplete(e->instance),
    "KheEventSetBack called after KheInstanceMakeEnd");
  e->back = back;
}


/*****************************************************************************/
/*                                                                           */
/*  void *KheEventBack(KHE_EVENT e)                                          */
/*                                                                           */
/*  Return the back pointer of e.                                            */
/*                                                                           */
/*****************************************************************************/

void *KheEventBack(KHE_EVENT e)
{
  return e->back;
}


/*****************************************************************************/
/*                                                                           */
/*  void KheEventAddUserEventGroup(KHE_EVENT e, KHE_EVENT_GROUP eg)          */
/*                                                                           */
/*  Add user event group eg to e.                                            */
/*                                                                           */
/*****************************************************************************/

void KheEventAddUserEventGroup(KHE_EVENT e, KHE_EVENT_GROUP eg)
{
  MArrayAddLast(e->user_event_groups, eg);
}


/*****************************************************************************/
/*                                                                           */
/*  KHE_INSTANCE KheEventInstance(KHE_EVENT e)                               */
/*                                                                           */
/*  Return the instance attribute of e.                                      */
/*                                                                           */
/*****************************************************************************/

KHE_INSTANCE KheEventInstance(KHE_EVENT e)
{
  return e->instance;
}


/*****************************************************************************/
/*                                                                           */
/*  char *KheEventId(KHE_EVENT e)                                            */
/*                                                                           */
/*  Return the id attribute of e.                                            */
/*                                                                           */
/*****************************************************************************/

char *KheEventId(KHE_EVENT e)
{
  return e->id;
}


/*****************************************************************************/
/*                                                                           */
/*  char *KheEventName(KHE_EVENT e)                                          */
/*                                                                           */
/*  Return the name attribute of e.                                          */
/*                                                                           */
/*****************************************************************************/

char *KheEventName(KHE_EVENT e)
{
  return e->name;
}


/*****************************************************************************/
/*                                                                           */
/*  char *KheEventColor(KHE_EVENT e)                                         */
/*                                                                           */
/*  Return the color associated with e, possibly NULL.                       */
/*                                                                           */
/*****************************************************************************/

char *KheEventColor(KHE_EVENT e)
{
  return e->color;
}


/*****************************************************************************/
/*                                                                           */
/*  int KheEventDuration(KHE_EVENT e)                                        */
/*                                                                           */
/*  Return the duration attribute of e.                                      */
/*                                                                           */
/*****************************************************************************/

int KheEventDuration(KHE_EVENT e)
{
  return e->duration;
}


/*****************************************************************************/
/*                                                                           */
/*  int KheEventWorkload(KHE_EVENT e)                                        */
/*                                                                           */
/*  Return the workload attribute of e.                                      */
/*                                                                           */
/*****************************************************************************/

int KheEventWorkload(KHE_EVENT e)
{
  return e->workload;
}


/*****************************************************************************/
/*                                                                           */
/*  int KheEventDemand(KHE_EVENT e)                                          */
/*                                                                           */
/*  Return the demand of e (duration times number of event resources).       */
/*                                                                           */
/*****************************************************************************/

int KheEventDemand(KHE_EVENT e)
{
  return e->duration * MArraySize(e->event_resources);
}


/*****************************************************************************/
/*                                                                           */
/*  KHE_TIME KheEventPreassignedTime(KHE_EVENT e)                            */
/*                                                                           */
/*  Return the preassigned time of e, possibly NULL.                         */
/*                                                                           */
/*****************************************************************************/

KHE_TIME KheEventPreassignedTime(KHE_EVENT e)
{
  return e->preassigned_time;
}


/*****************************************************************************/
/*                                                                           */
/*  int KheEventIndex(KHE_EVENT e)                                           */
/*                                                                           */
/*  Return the index number of e.                                            */
/*                                                                           */
/*****************************************************************************/

int KheEventIndex(KHE_EVENT e)
{
  return e->index;
}


/*****************************************************************************/
/*                                                                           */
/*  Submodule "time domains"                                                 */
/*                                                                           */
/*****************************************************************************/

/*****************************************************************************/
/*                                                                           */
/*  void KheEventFinalize(KHE_EVENT e)                                       */
/*                                                                           */
/*  Finalize e, which means setting its time domains and finalizing its      */
/*  event resources.  This function is called by KheInstanceMakeEnd.         */
/*                                                                           */
/*****************************************************************************/

void KheEventFinalize(KHE_EVENT e)
{
  KHE_EVENT_RESOURCE er;  int i;
  /* int duration, c_duration, count;  KHE_TIME_GROUP res, tg; */
  /* KHE_CONSTRAINT c; KHE_PREFER_TIMES_CONSTRAINT ptc, single_ptc; */
  if( DEBUG2 )
    fprintf(stderr, "[ KheEventFinalize(%s)\n", e->id != NULL ? e->id : "-");

  /* finalize the event resources */
  MArrayForEach(e->event_resources, &er, &i)
    KheEventResourceFinalize(er);

  /* finalize the domains */
  /* *** withdrawn
  for( duration = 1;  duration <= e->duration;  duration++ )
  {
    ** set res to the domain for this duration **
    if( e->preassigned_time != NULL )
      res = KheTimeSingletonTimeGroup(e->preassigned_time);
    else
    {
      ** set count to the number of applicable constraints, ptc to any one **
      count = 0;
      MArrayForEach(e->constraints, &c, &i)
	if( KheConstraintTag(c) == KHE_PREFER_TIMES_CONSTRAINT_TAG &&
	    KheConstraintRequired(c) && KheConstraintWeight(c) > 0 )
	{
	  ptc = KheToPreferTimesConstraint(c);
	  c_duration = KhePreferTimesConstraintDuration(ptc);
	  if( c_duration == KHE_NO_DURATION || c_duration == duration )
	  {
	    single_ptc = ptc;
	    count++;
	  }
	}

      ** set res to the actual domain; how to do that depends on count **
      if( count == 0 )
	res = KheInstanceFullTimeGroup(e->instance);
      else if( count == 1 )
	res = KhePreferTimesConstraintDomain(single_ptc);
      else
      {
	res = KheTimeGroupMakeInternal(KHE_TIME_GROUP_TYPE_EVENT, e->instance,
	  KHE_TIME_GROUP_KIND_ORDINARY, NULL, NULL, NULL);
	KheTimeGroupUnionInternal(res, KheInstanceFullTimeGroup(e->instance));
	MArrayForEach(e->constraints, &c, &i)
	  if( KheConstraintTag(c) == KHE_PREFER_TIMES_CONSTRAINT_TAG &&
	      KheConstraintRequired(c) && KheConstraintWeight(c) > 0 )
	  {
	    ptc = KheToPreferTimesConstraint(c);
	    c_duration = KhePreferTimesConstraintDuration(ptc);
	    if( c_duration == KHE_NO_DURATION || c_duration == duration )
	    {
	      tg = KhePreferTimesConstraintDomain(ptc);
	      KheTimeGroupIntersectInternal(res, tg);
	    }
	  }
	KheTimeGroupFinalize(res);
      }
    }

    ** either add res to the domains array, or use the previous value **
    if( duration > 1 && KheTimeGroupIntersectionEqual(MArrayLast(e->domains),
	KheInstancePackingTimeGroup(e->instance, duration), res) )
      res = MArrayLast(e->domains);
    if( DEBUG2 )
    {
      fprintf(stderr, "  domain for duration %d: ", MArraySize(e->domains) + 1);
      KheTimeGroupDebug(res, stderr);
      fprintf(stderr, "\n");
    }
    MArrayAddLast(e->domains, res);
  }
  *** */

  if( DEBUG2 )
    fprintf(stderr, "] KheEventFinalize(%s)\n", e->id != NULL ? e->id : "-");
}


/*****************************************************************************/
/*                                                                           */
/*  KHE_TIME_GROUP KheEventDomain(KHE_EVENT e, int duration)                 */
/*                                                                           */
/*  Return this special time group.                                          */
/*                                                                           */
/*****************************************************************************/

/* *** withdrawn
KHE_TIME_GROUP KheEventDomain(KHE_EVENT e, int duration)
{
  MAssert(MArraySize(e->domains) >= 1,
    "KheEventDomain called before KheInstanceMakeEnd");
  MAssert(duration <= e->duration, "KheEventDomain: duration out of range");
  return MArrayGet(e->domains, duration - 1);
}
*** */


/*****************************************************************************/
/*                                                                           */
/*  KHE_TIME_GROUP KheEventLooseDomain(KHE_EVENT e)                          */
/*                                                                           */
/*  Return this special time group.                                          */
/*                                                                           */
/*****************************************************************************/

/* ***
KHE_TIME_GROUP KheEventLooseDomain(KHE_EVENT e)
{
  MAssert(e->loose_domain != NULL,
    "KheEventLooseDomain called before KheInstanceMakeEnd");
  return e->loose_domain;
}
*** */


/*****************************************************************************/
/*                                                                           */
/*  Submodule "infer time breaks"                                            */
/*                                                                           */
/*****************************************************************************/

/*****************************************************************************/
/*                                                                           */
/*  void KheEventSubEventDurationRange(KHE_EVENT e,                          */
/*    int *min_duration, int *max_duration)                                  */
/*                                                                           */
/*  Return the minimum and maximum possible duration of any subevent of e,   */
/*  based on e's hard split events constraints.                              */
/*                                                                           */
/*****************************************************************************/

/* ***
static void KheEventSubEventDurationRange(KHE_EVENT e,
  int *min_duration, int *max_duration)
{
  KHE_CONSTRAINT c;  KHE_SPLIT_EVENTS_CONSTRAINT sec;  int i;
  *min_duration = 1;  *max_duration = e->duration;
  MArrayForEach(e->constraints, &c, &i)
    if( KheConstraintTag(c) == KHE_SPLIT_EVENTS_CONSTRAINT_TAG &&
	KheConstraintRequired(c) && KheConstraintWeight(c) > 0 )
    {
      sec = KheToSplitEventsConstraint(c);
      if( KheSplitEventsConstraintMinDuration(sec) > *min_duration )
	*min_duration = KheSplitEventsConstraintMinDuration(sec);
      if( KheSplitEventsConstraintMaxDuration(sec) < *max_duration )
	*max_duration = KheSplitEventsConstraintMaxDuration(sec);
    }
}
*** */


/*****************************************************************************/
/*                                                                           */
/*  bool KhePreferTimesConstraintTimeIsLegal(                                */
/*    KHE_PREFER_TIMES_CONSTRAINT ptc, KHE_TIME t)                           */
/*                                                                           */
/*  Return true if t is a starting time for ptc.                             */
/*                                                                           */
/*****************************************************************************/

/* ***
static bool KhePreferTimesConstraintTimeIsLegal(
  KHE_PREFER_TIMES_CONSTRAINT ptc, KHE_TIME t)
{
  int i;  KHE_TIME time;  KHE_TIME_GROUP tg;
  for( i = 0;  i < KhePreferTimesConstraintTimeGroupCount(ptc);  i++ )
  {
    tg = KhePreferTimesConstraintTimeGroup(ptc, i);
    if( KheTimeGroupContains(tg, t) )
      return true;
  }
  for( i = 0;  i < KhePreferTimesConstraintTimeCount(ptc);  i++ )
  {
    time = KhePreferTimesConstraintTime(ptc, i);
    if( time == t )
      return true;
  }
  return false;
}
*** */


/*****************************************************************************/
/*                                                                           */
/*  bool KheStartingTimeIsLegal(KHE_EVENT e, int duration, KHE_TIME t)       */
/*                                                                           */
/*  Return true if t is a legal starting time for a subevent of e of the     */
/*  given duration.                                                          */
/*                                                                           */
/*  Implementation note.  KhePreferTimesConstraintDomain cannot be called    */
/*  by this function, since it is not ready to use yet.                      */
/*                                                                           */
/*****************************************************************************/

/* ***
static bool KheStartingTimeIsLegal(KHE_EVENT e, int duration, KHE_TIME t)
{
  KHE_CONSTRAINT c;  KHE_PREFER_TIMES_CONSTRAINT ptc;  int i, d;
  MArrayForEach(e->constraints, &c, &i)
    if( KheConstraintTag(c) == KHE_PREFER_TIMES_CONSTRAINT_TAG &&
	KheConstraintRequired(c) && KheConstraintWeight(c) > 0 )
    {
      ptc = KheToPreferTimesConstraint(c);
      d = KhePreferTimesConstraintDuration(ptc);
      if( d == duration || d == KHE_NO_DURATION )
      {
	** ptc is applicable; if it doesn't contain t, then return false **
        if( !KhePreferTimesConstraintTimeIsLegal(ptc, t) )
	  return false;
      }
    }
  return true;
}
*** */


/*****************************************************************************/
/*                                                                           */
/*  void KheInferTimeBreaksForDuration(KHE_EVENT e, int duration)            */
/*                                                                           */
/*  Infer time breaks based on subevents of e of duration d.                 */
/*                                                                           */
/*****************************************************************************/

/* ***
static void KheInferTimeBreaksForDuration(KHE_EVENT e, int duration)
{
  int i, j;  KHE_TIME t, t2;
  if( DEBUG1 )
    fprintf(stderr, "  [ KheEventInferTimeBreaksForDuration(%s, %d)\n",
      e->id == NULL ? "-" : e->id, duration);
  if( duration > 1 ) ** no useful information from duration == 1 **
  {
    if( KheEventPreassignedTime(e) != NULL )
    {
      t = KheEventPreassignedTime(e);
      for( j = 0;  j < duration - 1;  j++ )
	if( KheTimeHasNeighbour(t, j) )
	{
	  t2 = KheTimeNeighbour(t, j);
	  if( DEBUG1 )
	    fprintf(stderr, "    no break at %s\n",
	      KheTimeId(t2) == NULL ? "-" : KheTimeId(t2));
	  KheTimeInferNoBreak(t2);
	}
    }
    else
    {
      for( i = 0;  i < KheInstanceTimeCount(e->instance);  i++ )
      {
	t = KheInstanceTime(e->instance, i);
	if( KheStartingTimeIsLegal(e, duration, t) )
	  for( j = 0;  j < duration - 1;  j++ )
	    if( KheTimeHasNeighbour(t, j) )
	    {
	      t2 = KheTimeNeighbour(t, j);
	      if( DEBUG1 )
		fprintf(stderr, "    no break at %s\n",
		  KheTimeId(t2) == NULL ? "-" : KheTimeId(t2));
	      KheTimeInferNoBreak(t2);
	    }
      }
    }
  }
  if( DEBUG1 )
    fprintf(stderr, "  ] KheEventInferTimeBreaksForDuration returning\n");
}
*** */


/*****************************************************************************/
/*                                                                           */
/*  void KheEventInferTimeBreaks(KHE_EVENT e)                                */
/*                                                                           */
/*  Infer time breaks based on when e could be assigned.                     */
/*                                                                           */
/*****************************************************************************/

/* ***
void KheEventInferTimeBreaks(KHE_EVENT e)
{
  int min_duration, max_duration, d;
  if( DEBUG1 )
    fprintf(stderr, "[ KheEventInferTimeBreaks(%s)\n",
      e->id == NULL ? "-" : e->id);
  if( e->duration > 1 )
  {
    KheEventSubEventDurationRange(e, &min_duration, &max_duration);
    if( DEBUG1 )
      fprintf(stderr, "  min_duration %d, max_duration %d\n",
        min_duration, max_duration);
    if( 1 <= min_duration && min_duration <= max_duration )
    {
      ** all durations from min_duration to max_duration are legal for e **
      for( d = min_duration;  d <= max_duration;  d++ )
	KheInferTimeBreaksForDuration(e, d);
    }
  }
  if( DEBUG1 )
    fprintf(stderr, "] KheEventInferTimeBreaks returning\n");
}
*** */


/*****************************************************************************/
/*                                                                           */
/*  Submodule "similarity"                                                   */
/*                                                                           */
/*****************************************************************************/

/*****************************************************************************/
/*                                                                           */
/*  void KheEventPartitionSetAdmissible(KHE_EVENT e)                         */
/*                                                                           */
/*  Decide whether e is admissible and set its flag if so.                   */
/*                                                                           */
/*****************************************************************************/

void KheEventPartitionSetAdmissible(KHE_EVENT e)
{
  KHE_EVENT_RESOURCE er;  int i;
  MArrayForEach(e->event_resources, &er, &i)
    if( KheResourceGroupPartitionAdmissible(KheEventResourceHardDomain(er)) )
    {
      e->partition_admissible = true;
      return;
    }
}


/*****************************************************************************/
/*                                                                           */
/*  bool KheEventPartitionAdmissible(KHE_EVENT e)                            */
/*                                                                           */
/*  Return true if e is admissible when inferring resource partitions.       */
/*                                                                           */
/*****************************************************************************/

bool KheEventPartitionAdmissible(KHE_EVENT e)
{
  return e->partition_admissible;
}


/*****************************************************************************/
/*                                                                           */
/*  bool KheEventPartitionSimilar(KHE_EVENT e1, KHE_EVENT e2,                */
/*    ARRAY_KHE_RESOURCE_GROUP *domains1, ARRAY_KHE_RESOURCE_GROUP *domains2)*/
/*                                                                           */
/*  Return true if e1 and e2 are similar for the purposes of inferring       */
/*  resource partitions.  Parameters *domains1 and *domains2 are scratch     */
/*  arrays that are used to hold the admissible hard domains of the events.  */
/*                                                                           */
/*****************************************************************************/

bool KheEventPartitionSimilar(KHE_EVENT e1, KHE_EVENT e2,
  ARRAY_KHE_RESOURCE_GROUP *domains1, ARRAY_KHE_RESOURCE_GROUP *domains2)
{
  KHE_RESOURCE_GROUP rg1, rg2;  KHE_EVENT_RESOURCE er;  int i1, i2;

  /* carry out simple tests */
  if( e1 == e2 )
    return true;
  if( e1->duration != e2->duration )
    return false;
  if( !e1->partition_admissible || !e2->partition_admissible )
    return false;

  /* get the domains of the admissible event resources of e1 and e2 */
  MArrayClear(*domains1);
  MArrayForEach(e1->event_resources, &er, &i1)
    if( KheResourceGroupPartitionAdmissible(KheEventResourceHardDomain(er)) )
      MArrayAddLast(*domains1, KheEventResourceHardDomain(er));
  MArrayClear(*domains2);
  MArrayForEach(e2->event_resources, &er, &i2)
    if( KheResourceGroupPartitionAdmissible(KheEventResourceHardDomain(er)) )
      MArrayAddLast(*domains2, KheEventResourceHardDomain(er));

  /* check them for equality, in any order */
  if( MArraySize(*domains1) != MArraySize(*domains2) )
    return false;
  MArrayForEach(*domains1, &rg1, &i1)
  {
    MArrayForEach(*domains2, &rg2, &i2)
      if( KheResourceGroupEqual(rg1, rg2) )
	break;
    if( i2 >= MArraySize(*domains2) )
      return false;
    MArrayRemove(*domains2, i2);
  }
  return true;
}


/*****************************************************************************/
/*                                                                           */
/*  bool KheEventSimilar(KHE_EVENT e1, KHE_EVENT e2)                         */
/*                                                                           */
/*  Return true if e1 and e2 are similar.                                    */
/*                                                                           */
/*****************************************************************************/

bool KheEventSimilar(KHE_EVENT e1, KHE_EVENT e2)
{
  ARRAY_KHE_RESOURCE_GROUP domains1, domains2;  bool res;
  if( e1 == e2 )
    return true;
  MArrayInit(domains1);
  MArrayInit(domains2);
  res = KheEventPartitionSimilar(e1, e2, &domains1, &domains2);
  MArrayFree(domains1);
  MArrayFree(domains2);
  return res;
}


/*****************************************************************************/
/*                                                                           */
/*  Submodule "event resources"                                              */
/*                                                                           */
/*****************************************************************************/

/*****************************************************************************/
/*                                                                           */
/*  void KheEventAddEventResource(KHE_EVENT e, KHE_EVENT_RESOURCE er,        */
/*    int *index)                                                            */
/*                                                                           */
/*  Add er to e, and set *index to its index number in e.                    */
/*                                                                           */
/*****************************************************************************/

void KheEventAddEventResource(KHE_EVENT e, KHE_EVENT_RESOURCE er,
  int *index)
{
  *index = MArraySize(e->event_resources);
  MArrayAddLast(e->event_resources, er);
}


/*****************************************************************************/
/*                                                                           */
/*  int KheEventResourceCount(KHE_EVENT e)                                   */
/*                                                                           */
/*  Return the number of event resources of e.                               */
/*                                                                           */
/*****************************************************************************/

int KheEventResourceCount(KHE_EVENT e)
{
  return MArraySize(e->event_resources);
}


/*****************************************************************************/
/*                                                                           */
/*  KHE_EVENT_RESOURCE KheEventResource(KHE_EVENT e, int i)                  */
/*                                                                           */
/*  Return the i'th event resource of e.                                     */
/*                                                                           */
/*****************************************************************************/

KHE_EVENT_RESOURCE KheEventResource(KHE_EVENT e, int i)
{
  return MArrayGet(e->event_resources, i);
}


/*****************************************************************************/
/*                                                                           */
/*  bool KheEventRetrieveEventResource(KHE_EVENT e, char *role,              */
/*    KHE_EVENT_RESOURCE *er)                                                */
/*                                                                           */
/*  If e contains an event resource with the given role, set *er to it and   */
/*  return true; otherwise, return false without changing *er.               */
/*                                                                           */
/*  Implementation note.  We don't bother with a hash table, because         */
/*  most events have only a few event resources.                             */
/*                                                                           */
/*****************************************************************************/

bool KheEventRetrieveEventResource(KHE_EVENT e, char *role,
  KHE_EVENT_RESOURCE *er)
{
  int i;  KHE_EVENT_RESOURCE er2;  char *role2;
  for( i = 0;  i < MArraySize(e->event_resources);  i++ )
  {
    er2 = MArrayGet(e->event_resources, i);
    role2 = KheEventResourceRole(er2);
    if( role2 != NULL && strcmp(role2, role) == 0 )
    {
      *er = er2;
      return true;
    }
  }
  return false;
}


/*****************************************************************************/
/*                                                                           */
/*  Submodule "event resource groups"                                        */
/*                                                                           */
/*****************************************************************************/

/*****************************************************************************/
/*                                                                           */
/*  void KheEventAddEventResourceGroup(KHE_EVENT event,                      */
/*    KHE_EVENT_RESOURCE_GROUP erg)                                          */
/*                                                                           */
/*  Add erg to event.                                                        */
/*                                                                           */
/*****************************************************************************/

void KheEventAddEventResourceGroup(KHE_EVENT event,
  KHE_EVENT_RESOURCE_GROUP erg)
{
  MArrayAddLast(event->event_resource_groups, erg);
}


/*****************************************************************************/
/*                                                                           */
/*  int KheEventResourceGroupCount(KHE_EVENT e)                              */
/*                                                                           */
/*  Return the number of event resource groups of e.                         */
/*                                                                           */
/*****************************************************************************/

int KheEventResourceGroupCount(KHE_EVENT e)
{
  return MArraySize(e->event_resource_groups);
}


/*****************************************************************************/
/*                                                                           */
/*  KHE_EVENT_RESOURCE_GROUP KheEventResourceGroup(KHE_EVENT e, int i)       */
/*                                                                           */
/*  Return the i'th event resource group of e.                               */
/*                                                                           */
/*****************************************************************************/

KHE_EVENT_RESOURCE_GROUP KheEventResourceGroup(KHE_EVENT e, int i)
{
  return MArrayGet(e->event_resource_groups, i);
}


/*****************************************************************************/
/*                                                                           */
/*  Submodule "auto-generated event groups"                                  */
/*                                                                           */
/*****************************************************************************/

/*****************************************************************************/
/*                                                                           */
/*  KHE_EVENT_GROUP KheEventSingletonEventGroup(KHE_EVENT e)                 */
/*                                                                           */
/*  Return the event group containing just e.                                */
/*                                                                           */
/*****************************************************************************/

KHE_EVENT_GROUP KheEventSingletonEventGroup(KHE_EVENT e)
{
  return e->singleton_event_group;
}


/*****************************************************************************/
/*                                                                           */
/*  Submodule "constraints"                                                  */
/*                                                                           */
/*****************************************************************************/

/*****************************************************************************/
/*                                                                           */
/*  void KheEventAddConstraint(KHE_EVENT e, KHE_CONSTRAINT c)                */
/*                                                                           */
/*  Add c to e.                                                              */
/*                                                                           */
/*****************************************************************************/

void KheEventAddConstraint(KHE_EVENT e, KHE_CONSTRAINT c)
{
  MArrayAddLast(e->constraints, c);
}


/*****************************************************************************/
/*                                                                           */
/*  int KheEventConstraintCount(KHE_EVENT e)                                 */
/*                                                                           */
/*  Return the number of constraints applicable to e.                        */
/*                                                                           */
/*****************************************************************************/

int KheEventConstraintCount(KHE_EVENT e)
{
  return MArraySize(e->constraints);
}


/*****************************************************************************/
/*                                                                           */
/*  KHE_CONSTRAINT KheEventConstraint(KHE_EVENT e, int i)                    */
/*                                                                           */
/*  Return the i'th constraint applicable to e.                              */
/*                                                                           */
/*****************************************************************************/

KHE_CONSTRAINT KheEventConstraint(KHE_EVENT e, int i)
{
  return MArrayGet(e->constraints, i);
}


/*****************************************************************************/
/*                                                                           */
/*  Submodule "reading and writing"                                          */
/*                                                                           */
/*****************************************************************************/

/*****************************************************************************/
/*                                                                           */
/*  bool KheEventMakeFromKml(KML_ELT event_elt, KHE_INSTANCE ins,            */
/*    KML_ERROR *ke)                                                         */
/*                                                                           */
/*  Make an event from event_elt and add it to ins.                          */
/*                                                                           */
/*****************************************************************************/

bool KheEventMakeFromKml(KML_ELT event_elt, KHE_INSTANCE ins,
  KML_ERROR *ke)
{
  KML_ELT workload_elt, time_elt, event_groups_elt, event_group_elt;
  KML_ELT course_elt, resources_elt, resource_elt, rgroups_elt, rgroup_elt;
  int i, workload, duration;  char *id, *name, *color, *ref;
  KHE_EVENT e;  KHE_EVENT_GROUP eg;  KHE_TIME preassigned_time;
  KHE_RESOURCE_GROUP rg;

  /* check event_elt */
  if( !KmlCheck(event_elt, "Id +Color : $Name #Duration +#Workload +Course"
      " +Time +Resources +ResourceGroups +EventGroups", ke) )
    return false;

  /* Id, Name, and Color */
  id = KmlExtractAttributeValue(event_elt, 0);
  name = KmlExtractText(KmlChild(event_elt, 0));
  if( KmlAttributeCount(event_elt) == 2 )
    color = KmlExtractAttributeValue(event_elt, 1);
  else
    color = NULL;

  /* Duration and Workload */
  sscanf(KmlText(KmlChild(event_elt, 1)), "%d", &duration);
  if( KmlContainsChild(event_elt, "Workload", &workload_elt) )
    sscanf(KmlText(workload_elt), "%d", &workload);
  else
    workload = duration;

  /* Time */
  preassigned_time = NULL;
  if( KmlContainsChild(event_elt, "Time", &time_elt) )
  {
    if( !KmlCheck(time_elt, "Reference", ke) )
      return false;
    ref = KmlAttributeValue(time_elt, 0);
    if( !KheInstanceRetrieveTime(ins, ref, &preassigned_time) )
      return KmlErrorMake(ke, KmlLineNum(time_elt), KmlColNum(time_elt),
	"in <Time>, Reference \"%s\" unknown", ref);
  }

  /* make and add the event */
  if( !KheEventMake(ins,id,name,color,duration,workload,preassigned_time, &e) )
    return KmlErrorMake(ke, KmlLineNum(event_elt), KmlColNum(event_elt),
      "in <Event>, Id \"%s\" used previously", id);

  /* Course */
  if( KmlContainsChild(event_elt, "Course", &course_elt) )
  {
    if( !KmlCheck(course_elt, "Reference", ke) )
      return false;
    ref = KmlAttributeValue(course_elt, 0);
    if( !KheInstanceRetrieveEventGroup(ins, ref, &eg) )
      return KmlErrorMake(ke, KmlLineNum(course_elt), KmlColNum(course_elt),
	"in <Course>, Reference \"%s\" unknown", ref);
    KheEventGroupAddEvent(eg, e);
  }

  /* Resources */
  if( KmlContainsChild(event_elt, "Resources", &resources_elt) )
  {
    if( !KmlCheck(resources_elt, ": *Resource", ke) )
      return false;
    for( i = 0;  i < KmlChildCount(resources_elt);  i++ )
    {
      resource_elt = KmlChild(resources_elt, i);
      if( !KheEventResourceMakeFromKml(resource_elt, e, ke) )
	return false;
    }
  }

  /* ResourceGroups */
  if( KmlContainsChild(event_elt, "ResourceGroups", &rgroups_elt) )
  {
    if( !KmlCheck(rgroups_elt, ": *ResourceGroup", ke) )
      return false;
    for( i = 0;  i < KmlChildCount(rgroups_elt);  i++ )
    {
      rgroup_elt = KmlChild(rgroups_elt, i);
      if( !KmlCheck(rgroup_elt, "Reference", ke) )
	return false;
      ref = KmlAttributeValue(rgroup_elt, 0);
      if( !KheInstanceRetrieveResourceGroup(ins, ref, &rg) )
	return KmlErrorMake(ke, KmlLineNum(rgroup_elt), KmlColNum(rgroup_elt),
	  "in <ResourceGroup>, Reference \"%s\" unknown", ref);
      KheEventResourceGroupMake(e, rg);
    }
  }

  /* EventGroups */
  if( KmlContainsChild(event_elt, "EventGroups", &event_groups_elt) )
  {
    if( !KmlCheck(event_groups_elt, ": *EventGroup", ke) )
      return false;
    for( i = 0;  i < KmlChildCount(event_groups_elt);  i++ )
    {
      event_group_elt = KmlChild(event_groups_elt, i);
      if( !KmlCheck(event_group_elt, "Reference", ke) )
	return false;
      ref = KmlAttributeValue(event_group_elt, 0);
      if( !KheInstanceRetrieveEventGroup(ins, ref, &eg) )
	return KmlErrorMake(ke, KmlLineNum(event_group_elt),
	  KmlColNum(event_group_elt),
	  "in <EventGroup>, Reference \"%s\" unknown", ref);
      KheEventGroupAddEvent(eg, e);
    }
  }
  return true;
}


/*****************************************************************************/
/*                                                                           */
/*  bool KheEventWrite(KHE_EVENT e, KML_FILE kf)                             */
/*                                                                           */
/*  Write e to kf.                                                           */
/*                                                                           */
/*****************************************************************************/

bool KheEventWrite(KHE_EVENT e, KML_FILE kf)
{
  KHE_EVENT_GROUP eg;  int start, i;  bool resources_started;
  KHE_EVENT_RESOURCE er;  KHE_EVENT_RESOURCE_GROUP erg;

  /* header and name */
  if( e->id == NULL || e->name == NULL )
    return false;
  KmlBegin(kf, "Event");
  KmlAttribute(kf, "Id", e->id);
  if( e->color != NULL )
    KmlAttribute(kf, "Color", e->color);
  KmlEltPrintf(kf, "Name", "%s", e->name);

  /* duration and workload */
  KmlEltPrintf(kf, "Duration", "%d", e->duration);
  if( e->workload != e->duration )
    KmlEltPrintf(kf, "Workload", "%d", e->workload);

  /* optional Course */
  start = 0;
  if( MArraySize(e->user_event_groups) > 0 )
  {
    eg = MArrayFirst(e->user_event_groups);
    if( KheEventGroupKind(eg) == KHE_EVENT_GROUP_KIND_COURSE )
    {
      KmlEltAttribute(kf, "Course", "Reference", KheEventGroupId(eg));
      start = 1;
    }
  }

  /* optional preassigned time */
  if( e->preassigned_time != NULL )
    KmlEltAttribute(kf, "Time", "Reference", KheTimeId(e->preassigned_time));

  /* optional event resources */
  resources_started = false;
  MArrayForEach(e->event_resources, &er, &i)
  {
    if( KheEventResourceEventResourceGroup(er) == NULL )
    {
      if( !resources_started )
      {
	KmlBegin(kf, "Resources");
        resources_started = true;
      }
      if( !KheEventResourceWrite(er, kf) )
	return false;
    }
  }
  if( resources_started )
    KmlEnd(kf, "Resources");

  /* optional resource groups */
  if( MArraySize(e->event_resource_groups) > 0 )
  {
    KmlBegin(kf, "ResourceGroups");
    MArrayForEach(e->event_resource_groups, &erg, &i)
      KmlEltAttribute(kf, "ResourceGroup", "Reference",
	KheResourceGroupId(KheEventResourceGroupResourceGroup(erg)));
    KmlEnd(kf, "ResourceGroups");
  }

  /* optional event groups */
  if( MArraySize(e->user_event_groups) > start )
  {
    KmlBegin(kf, "EventGroups");
    for( i = start;  i < MArraySize(e->user_event_groups);  i++ )
    {
      eg = MArrayGet(e->user_event_groups, i);
      KmlEltAttribute(kf, "EventGroup", "Reference", KheEventGroupId(eg));
    }
    KmlEnd(kf, "EventGroups");
  }

  /* footer */
  KmlEnd(kf, "Event");
  return true;
}


/*****************************************************************************/
/*                                                                           */
/*  Submodule "debug"                                                        */
/*                                                                           */
/*****************************************************************************/

/*****************************************************************************/
/*                                                                           */
/*  void KheEventDebug(KHE_EVENT e, int verbosity, int indent, FILE *fp)     */
/*                                                                           */
/*  Debug print of e onto fp with the given verbosity and indent.            */
/*                                                                           */
/*****************************************************************************/

void KheEventDebug(KHE_EVENT e, int verbosity, int indent, FILE *fp)
{
  if( verbosity >= 1 )
  {
    if( indent >= 0 )
      fprintf(fp, "%*s", indent, "");
    fprintf(fp, "%s", e->id != NULL ? e->id : "-");
    if( indent >= 0 )
      fprintf(fp, "\n");
  }
}
