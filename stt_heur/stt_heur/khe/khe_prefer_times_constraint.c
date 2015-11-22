
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
/*  FILE:         khe_prefer_times_constraint.c                              */
/*  DESCRIPTION:  A prefer times constraint                                  */
/*                                                                           */
/*****************************************************************************/
#include "khe_interns.h"


/*****************************************************************************/
/*                                                                           */
/*  KHE_PREFER_TIMES_CONSTRAINT - a prefer times constraint                  */
/*                                                                           */
/*****************************************************************************/

struct khe_prefer_times_constraint_rec {
  void				*back;			/* back pointer      */
  KHE_CONSTRAINT_TAG		tag;			/* type tag          */
  KHE_INSTANCE			instance;		/* enclosing instance*/
  char				*id;			/* id                */
  char				*name;			/* name              */
  bool				required;		/* hard or soft      */
  int				weight;			/* Weight            */
  KHE_COST			combined_weight;	/* Weight            */
  KHE_COST_FUNCTION		cost_function;		/* CostFunction      */
  int				index;			/* index number      */
  ARRAY_KHE_TIME_GROUP		time_groups;		/* variable domain   */
  ARRAY_KHE_TIME		times;			/* variable domain   */
  KHE_TIME_GROUP		domain;			/* variable domain   */
  int				duration;		/* optional duration */
  ARRAY_KHE_EVENT		events;			/* Events            */
  ARRAY_KHE_EVENT_GROUP		event_groups;		/* EventGroups       */
};


/*****************************************************************************/
/*                                                                           */
/*  Submodule "construction and query"                                       */
/*                                                                           */
/*****************************************************************************/

/*****************************************************************************/
/*                                                                           */
/*  bool KhePreferTimesConstraintMake(KHE_INSTANCE ins, char *id,            */
/*    char *name, bool required, int weight, KHE_COST_FUNCTION cf,           */
/*    int duration, KHE_PREFER_TIMES_CONSTRAINT *c)                          */
/*                                                                           */
/*  Make a prefer times constraint, add it to the instance, and return it.   */
/*                                                                           */
/*****************************************************************************/

bool KhePreferTimesConstraintMake(KHE_INSTANCE ins, char *id,
  char *name, bool required, int weight, KHE_COST_FUNCTION cf,
  int duration, KHE_PREFER_TIMES_CONSTRAINT *c)
{
  KHE_PREFER_TIMES_CONSTRAINT res;  KHE_CONSTRAINT cc;
  MAssert(!KheInstanceComplete(ins),
    "KhePreferTimesConstraintMake called after KheInstanceMakeEnd");
  if( id != NULL && KheInstanceRetrieveConstraint(ins, id, &cc) )
  {
    *c = NULL;
    return false;
  }
  MMake(res);
  res->back = NULL;
  res->tag = KHE_PREFER_TIMES_CONSTRAINT_TAG;
  res->instance = ins;
  res->id = id;
  res->name = name;
  res->required = required;
  res->weight = weight;
  res->combined_weight = required ? KheCost(weight, 0) : KheCost(0, weight);
  res->cost_function = cf;
  res->index = KheInstanceConstraintCount(ins);
  MArrayInit(res->time_groups);
  MArrayInit(res->times);
  res->domain = NULL;
  res->duration = duration;
  MArrayInit(res->events);
  MArrayInit(res->event_groups);
  KheInstanceAddConstraint(ins, (KHE_CONSTRAINT) res);
  *c = res;
  return true;
}


/*****************************************************************************/
/*                                                                           */
/*  int KhePreferTimesConstraintDuration(KHE_PREFER_TIMES_CONSTRAINT c)      */
/*                                                                           */
/*  Return the duration attribute of c.  This could be the special value     */
/*  KHE_NO_DURATION.                                                         */
/*                                                                           */
/*****************************************************************************/

int KhePreferTimesConstraintDuration(KHE_PREFER_TIMES_CONSTRAINT c)
{
  return c->duration;
}


/*****************************************************************************/
/*                                                                           */
/*  int KhePreferTimesConstraintAppliesToCount(KHE_PREFER_TIMES_CONSTRAINT c)*/
/*                                                                           */
/*  Return the number of points of application of c.                         */
/*                                                                           */
/*****************************************************************************/

int KhePreferTimesConstraintAppliesToCount(KHE_PREFER_TIMES_CONSTRAINT c)
{
  int i, res;  KHE_EVENT_GROUP eg;
  res = MArraySize(c->events);
  MArrayForEach(c->event_groups, &eg, &i)
    res += KheEventGroupEventCount(eg);
  return res;
}


/*****************************************************************************/
/*                                                                           */
/*  void KhePreferTimesConstraintFinalize(KHE_PREFER_TIMES_CONSTRAINT c)     */
/*                                                                           */
/*  Finalize c, since KheInstanceMakeEnd has been called.                    */
/*                                                                           */
/*  Implementation note.  What needs to be done is to set the domain of c.   */
/*                                                                           */
/*****************************************************************************/

void KhePreferTimesConstraintFinalize(KHE_PREFER_TIMES_CONSTRAINT c)
{
  int i;  KHE_TIME_GROUP tg;  KHE_TIME t;
  if( MArraySize(c->time_groups) == 0 && MArraySize(c->times) == 0 )
    c->domain = KheInstanceEmptyTimeGroup(c->instance);
  else if( MArraySize(c->time_groups) == 0 && MArraySize(c->times) == 1 )
    c->domain = KheTimeSingletonTimeGroup(MArrayFirst(c->times));
  else if( MArraySize(c->time_groups) == 1 && MArraySize(c->times) == 0 )
    c->domain = MArrayFirst(c->time_groups);
  else
  {
    c->domain = KheTimeGroupMakeInternal(KHE_TIME_GROUP_TYPE_USER,
      c->instance, KHE_TIME_GROUP_KIND_ORDINARY, NULL, NULL, LSetNew());
    MArrayForEach(c->time_groups, &tg, &i)
      KheTimeGroupUnionInternal(c->domain, tg);
    MArrayForEach(c->times, &t, &i)
      KheTimeGroupAddTimeInternal(c->domain, t);
    KheTimeGroupFinalize(c->domain, NULL, -1);
  }
}


/*****************************************************************************/
/*                                                                           */
/*  Submodule "time groups"                                                  */
/*                                                                           */
/*****************************************************************************/

/*****************************************************************************/
/*                                                                           */
/*  void KhePreferTimesConstraintAddTimeGroup(KHE_PREFER_TIMES_CONSTRAINT c, */
/*    KHE_TIME_GROUP tg)                                                     */
/*                                                                           */
/*  Add a time group to c.                                                   */
/*                                                                           */
/*****************************************************************************/

void KhePreferTimesConstraintAddTimeGroup(KHE_PREFER_TIMES_CONSTRAINT c,
  KHE_TIME_GROUP tg)
{
  MArrayAddLast(c->time_groups, tg);
}


/*****************************************************************************/
/*                                                                           */
/*  int KhePreferTimesConstraintTimeGroupCount(KHE_PREFER_TIMES_CONSTRAINT c)*/
/*                                                                           */
/*  Return the number of time groups in c.                                   */
/*                                                                           */
/*****************************************************************************/

int KhePreferTimesConstraintTimeGroupCount(KHE_PREFER_TIMES_CONSTRAINT c)
{
  return MArraySize(c->time_groups);
}


/*****************************************************************************/
/*                                                                           */
/*  KHE_TIME_GROUP KhePreferTimesConstraintTimeGroup(                        */
/*    KHE_PREFER_TIMES_CONSTRAINT c, int i)                                  */
/*                                                                           */
/*  Return the i'th time group of c.                                         */
/*                                                                           */
/*****************************************************************************/

KHE_TIME_GROUP KhePreferTimesConstraintTimeGroup(
  KHE_PREFER_TIMES_CONSTRAINT c, int i)
{
  return MArrayGet(c->time_groups, i);
}


/*****************************************************************************/
/*                                                                           */
/*  Submodule "times"                                                        */
/*                                                                           */
/*****************************************************************************/

/*****************************************************************************/
/*                                                                           */
/*  void KhePreferTimesConstraintAddTime(KHE_PREFER_TIMES_CONSTRAINT c,      */
/*    KHE_TIME t)                                                            */
/*                                                                           */
/*  Add a time to c.                                                         */
/*                                                                           */
/*****************************************************************************/

void KhePreferTimesConstraintAddTime(KHE_PREFER_TIMES_CONSTRAINT c, KHE_TIME t)
{
  MArrayAddLast(c->times, t);
}


/*****************************************************************************/
/*                                                                           */
/*  int KhePreferTimesConstraintTimeCount(KHE_PREFER_TIMES_CONSTRAINT c)     */
/*                                                                           */
/*  Return the number of times in c.                                         */
/*                                                                           */
/*****************************************************************************/

int KhePreferTimesConstraintTimeCount(KHE_PREFER_TIMES_CONSTRAINT c)
{
  return MArraySize(c->times);
}


/*****************************************************************************/
/*                                                                           */
/*  KHE_TIME KhePreferTimesConstraintTime(KHE_PREFER_TIMES_CONSTRAINT c,     */
/*    int i)                                                                 */
/*                                                                           */
/*  Return the i'th time of c.                                               */
/*                                                                           */
/*****************************************************************************/

KHE_TIME KhePreferTimesConstraintTime(KHE_PREFER_TIMES_CONSTRAINT c, int i)
{
  return MArrayGet(c->times, i);
}


/*****************************************************************************/
/*                                                                           */
/*  Submodule "domain"                                                       */
/*                                                                           */
/*****************************************************************************/

/*****************************************************************************/
/*                                                                           */
/*  KHE_TIME_GROUP KhePreferTimesConstraintDomain(                           */
/*    KHE_PREFER_TIMES_CONSTRAINT c)                                         */
/*                                                                           */
/*  Return the domain of c.                                                  */
/*                                                                           */
/*****************************************************************************/

KHE_TIME_GROUP KhePreferTimesConstraintDomain(KHE_PREFER_TIMES_CONSTRAINT c)
{
  MAssert(c->domain != NULL,
    "KhePreferTimesConstraintDomain called before KheInstanceMakeEnd");
  return c->domain;
}


/*****************************************************************************/
/*                                                                           */
/*  Submodule "events"                                                       */
/*                                                                           */
/*****************************************************************************/

/*****************************************************************************/
/*                                                                           */
/*  void KhePreferTimesConstraintAddEvent(KHE_PREFER_TIMES_CONSTRAINT c,     */
/*    KHE_EVENT e)                                                           */
/*                                                                           */
/*  Add an event to c.                                                       */
/*                                                                           */
/*****************************************************************************/

void KhePreferTimesConstraintAddEvent(KHE_PREFER_TIMES_CONSTRAINT c,
  KHE_EVENT e)
{
  MArrayAddLast(c->events, e);
  if( KheEventPreassignedTime(e) == NULL )
    KheEventAddConstraint(e, (KHE_CONSTRAINT) c);
}


/*****************************************************************************/
/*                                                                           */
/*  int KhePreferTimesConstraintEventCount(KHE_PREFER_TIMES_CONSTRAINT c)    */
/*                                                                           */
/*  Return the number of events of c.                                        */
/*                                                                           */
/*****************************************************************************/

int KhePreferTimesConstraintEventCount(KHE_PREFER_TIMES_CONSTRAINT c)
{
  return MArraySize(c->events);
}


/*****************************************************************************/
/*                                                                           */
/*  KHE_EVENT KhePreferTimesConstraintEvent(KHE_PREFER_TIMES_CONSTRAINT c,   */
/*    int i)                                                                 */
/*                                                                           */
/*  Return the i'th event of c.                                              */
/*                                                                           */
/*****************************************************************************/

KHE_EVENT KhePreferTimesConstraintEvent(KHE_PREFER_TIMES_CONSTRAINT c, int i)
{
  return MArrayGet(c->events, i);
}


/*****************************************************************************/
/*                                                                           */
/*  Submodule "event groups"                                                 */
/*                                                                           */
/*****************************************************************************/

/*****************************************************************************/
/*                                                                           */
/*  void KhePreferTimesConstraintAddEventGroup(                              */
/*    KHE_PREFER_TIMES_CONSTRAINT c, KHE_EVENT_GROUP eg)                     */
/*                                                                           */
/*  Add an event group to c.                                                 */
/*                                                                           */
/*****************************************************************************/

void KhePreferTimesConstraintAddEventGroup(
  KHE_PREFER_TIMES_CONSTRAINT c, KHE_EVENT_GROUP eg)
{
  int i;  KHE_EVENT e;
  for( i = 0;  i < KheEventGroupEventCount(eg);  i++ )
  {
    e = KheEventGroupEvent(eg, i);
    if( KheEventPreassignedTime(e) == NULL )
      KheEventAddConstraint(e, (KHE_CONSTRAINT) c);
  }
  MArrayAddLast(c->event_groups, eg);
}


/*****************************************************************************/
/*                                                                           */
/* int KhePreferTimesConstraintEventGroupCount(KHE_PREFER_TIMES_CONSTRAINT c)*/
/*                                                                           */
/*  Return the number of event groups in c.                                  */
/*                                                                           */
/*****************************************************************************/

int KhePreferTimesConstraintEventGroupCount(KHE_PREFER_TIMES_CONSTRAINT c)
{
  return MArraySize(c->event_groups);
}


/*****************************************************************************/
/*                                                                           */
/*  KHE_EVENT_GROUP KhePreferTimesConstraintEventGroup(                      */
/*    KHE_PREFER_TIMES_CONSTRAINT c, int i)                                  */
/*                                                                           */
/*  Return the i'th event group of c.                                        */
/*                                                                           */
/*****************************************************************************/

KHE_EVENT_GROUP KhePreferTimesConstraintEventGroup(
  KHE_PREFER_TIMES_CONSTRAINT c, int i)
{
  return MArrayGet(c->event_groups, i);
}


/*****************************************************************************/
/*                                                                           */
/*  Submodule "reading and writing"                                          */
/*                                                                           */
/*****************************************************************************/

/*****************************************************************************/
/*                                                                           */
/*  bool KhePreferTimesConstraintMakeFromKml(KML_ELT cons_elt,               */
/*    KHE_INSTANCE ins, KML_ERROR *ke)                                       */
/*                                                                           */
/*  Add a prefer times constraint based on cons_elt to ins.                  */
/*                                                                           */
/*****************************************************************************/

bool KhePreferTimesConstraintMakeFromKml(KML_ELT cons_elt,
  KHE_INSTANCE ins, KML_ERROR *ke)
{
  char *id, *name;  bool reqd;  int wt;  KHE_COST_FUNCTION cf;
  KML_ELT elt;  KHE_PREFER_TIMES_CONSTRAINT res;  int duration;

  /* verify cons_elt and get the common fields */
  if( !KmlCheck(cons_elt, "Id : $Name $Required #Weight "
      "$CostFunction AppliesTo +TimeGroups +Times +#Duration", ke) )
    return false;
  if( !KheConstraintCheckKml(cons_elt, &id, &name, &reqd, &wt, &cf, ke) )
    return false;

  /* get the duration */
  if( KmlContainsChild(cons_elt, "Duration", &elt) )
    sscanf(KmlText(elt), "%d", &duration);
  else
    duration = KHE_NO_DURATION;

  /* make the constraint object and add it to ins */
  if( !KhePreferTimesConstraintMake(ins, id, name, reqd, wt, cf,
	duration, &res) )
    return false;

  /* add the event groups and events */
  elt = KmlChild(cons_elt, 4);
  if( !KmlCheck(elt, ": +EventGroups +Events", ke) )
    return false;
  if( !KheConstraintAddEventGroupsFromKml((KHE_CONSTRAINT) res, elt, ke) )
    return false;
  if( !KheConstraintAddEventsFromKml((KHE_CONSTRAINT) res, elt, ke) )
    return false;

  /* add the time groups and times */
  if( !KheConstraintAddTimeGroupsFromKml((KHE_CONSTRAINT) res, cons_elt, ke) )
    return false;
  if( !KheConstraintAddTimesFromKml((KHE_CONSTRAINT) res, cons_elt, ke) )
    return false;
  return true;
}


/*****************************************************************************/
/*                                                                           */
/*  bool KhePreferTimesConstraintWrite(KHE_PREFER_TIMES_CONSTRAINT c,        */
/*    KML_FILE kf)                                                           */
/*                                                                           */
/*  Write c to kf.                                                           */
/*                                                                           */
/*****************************************************************************/

bool KhePreferTimesConstraintWrite(KHE_PREFER_TIMES_CONSTRAINT c, KML_FILE kf)
{
  KHE_EVENT_GROUP eg;  KHE_EVENT e;  int i;
  KHE_TIME_GROUP tg;  KHE_TIME t;
  if( c->id == NULL )
    return false;
  KmlBegin(kf, "PreferTimesConstraint");
  KmlAttribute(kf, "Id", c->id);
  if( !KheConstraintWriteCommonFields((KHE_CONSTRAINT) c, kf) )
    return false;
  KmlBegin(kf, "AppliesTo");
  if( MArraySize(c->event_groups) > 0 )
  {
    KmlBegin(kf, "EventGroups");
    MArrayForEach(c->event_groups, &eg, &i)
      KmlEltAttribute(kf, "EventGroup", "Reference", KheEventGroupId(eg));
    KmlEnd(kf, "EventGroups");
  }
  if( MArraySize(c->events) > 0 )
  {
    KmlBegin(kf, "Events");
    MArrayForEach(c->events, &e, &i)
      KmlEltAttribute(kf, "Event", "Reference", KheEventId(e));
    KmlEnd(kf, "Events");
  }
  KmlEnd(kf, "AppliesTo");
  if( MArraySize(c->time_groups) > 0 )
  {
    KmlBegin(kf, "TimeGroups");
    MArrayForEach(c->time_groups, &tg, &i)
      KmlEltAttribute(kf, "TimeGroup", "Reference", KheTimeGroupId(tg));
    KmlEnd(kf, "TimeGroups");
  }
  if( MArraySize(c->times) > 0 )
  {
    KmlBegin(kf, "Times");
    MArrayForEach(c->times, &t, &i)
      KmlEltAttribute(kf, "Time", "Reference", KheTimeId(t));
    KmlEnd(kf, "Times");
  }
  if( c->duration != KHE_NO_DURATION )
    KmlEltPrintf(kf, "Duration", "%d", c->duration);
  KmlEnd(kf, "PreferTimesConstraint");
  return true;
}
