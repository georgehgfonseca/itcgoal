
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
/*  FILE:         khe_time.c                                                 */
/*  DESCRIPTION:  A time                                                     */
/*                                                                           */
/*****************************************************************************/
#include "khe_interns.h"


/*****************************************************************************/
/*                                                                           */
/*  KHE_TIME - a time                                                        */
/*                                                                           */
/*****************************************************************************/

struct khe_time_rec {
  void			*back;			/* back pointer              */
  KHE_INSTANCE		instance;		/* enclosing instance        */
  char			*id;			/* Id                        */
  char			*name;			/* Name                      */
  bool			break_after;		/* true if break after       */
  /* bool		inferred_break_after;*/	/* inferred break after      */
  int			index;			/* in `all' group and lsets  */
  /* int	   cycle_soln_event_index; */	/* index into cycle soln e's */
  /* int	   cycle_soln_event_offset; */	/* offset within cycle se    */
  KHE_TIME_GROUP	singleton_time_group;	/* singleton time group      */
};


/*****************************************************************************/
/*                                                                           */
/*  Construction and query                                                   */
/*                                                                           */
/*****************************************************************************/

/*****************************************************************************/
/*                                                                           */
/*  bool KheTimeMake(KHE_INSTANCE ins, char *id, char *name,                 */
/*    bool break_after, KHE_TIME *t)                                         */
/*                                                                           */
/*  Make a time object with these atrributes and add it to ins.              */
/*                                                                           */
/*****************************************************************************/

bool KheTimeMake(KHE_INSTANCE ins, char *id, char *name,
  bool break_after, KHE_TIME *t)
{
  KHE_TIME res;
  MAssert(!KheInstanceComplete(ins),
    "KheTimeMake called after KheInstanceMakeEnd");
  if( id != NULL && KheInstanceRetrieveTime(ins, id, &res) )
  {
    *t = NULL;
    return false;
  }
  MMake(res);
  res->back = NULL;
  res->instance = ins;
  res->id = id;
  res->name = name;
  res->break_after = break_after;
  /* res->inferred_break_after = true; */  /* just the initial value */
  res->index = KheInstanceTimeCount(ins);
  res->singleton_time_group = KheTimeGroupMakeInternal(
    KHE_TIME_GROUP_TYPE_SINGLETON, ins, KHE_TIME_GROUP_KIND_ORDINARY,
    NULL, NULL, LSetNew());
  KheTimeGroupAddTimeInternal(res->singleton_time_group, res);
  KheTimeGroupAddTimeInternal(KheInstanceFullTimeGroupInternal(ins), res);
  /* res->cycle_soln_event_index = res->cycle_soln_event_offset = -1; */
  /* set later */
  KheInstanceAddTime(ins, res);
  *t = res;
  return true;
}


/*****************************************************************************/
/*                                                                           */
/*  void KheTimeSetBack(KHE_TIME t, void *back)                              */
/*                                                                           */
/*  Set the back pointer of t.                                               */
/*                                                                           */
/*****************************************************************************/

void KheTimeSetBack(KHE_TIME t, void *back)
{
  MAssert(!KheInstanceComplete(t->instance),
    "KheTimeSetBack called after KheInstanceMakeEnd");
  t->back = back;
}


/*****************************************************************************/
/*                                                                           */
/*  void *KheTimeBack(KHE_TIME t)                                            */
/*                                                                           */
/*  Return the back pointer of t.                                            */
/*                                                                           */
/*****************************************************************************/

void *KheTimeBack(KHE_TIME t)
{
  return t->back;
}


/*****************************************************************************/
/*                                                                           */
/*  KHE_INSTANCE KheTimeInstance(KHE_TIME t)                                 */
/*                                                                           */
/*  Return the enclosing instance of t.                                      */
/*                                                                           */
/*****************************************************************************/

KHE_INSTANCE KheTimeInstance(KHE_TIME t)
{
  return t->instance;
}


/*****************************************************************************/
/*                                                                           */
/*  char *KheTimeId(KHE_TIME t)                                              */
/*                                                                           */
/*  Return the id attribute of t.                                            */
/*                                                                           */
/*****************************************************************************/

char *KheTimeId(KHE_TIME t)
{
  return t->id;
}


/*****************************************************************************/
/*                                                                           */
/*  char *KheTimeName(KHE_TIME t)                                            */
/*                                                                           */
/*  Return the name attribute of t.                                          */
/*                                                                           */
/*****************************************************************************/

char *KheTimeName(KHE_TIME t)
{
  return t->name;
}


/*****************************************************************************/
/*                                                                           */
/*  bool KheTimeBreakAfter(KHE_TIME t)                                       */
/*                                                                           */
/*  Return the break_after attribute of t.                                   */
/*                                                                           */
/*****************************************************************************/

bool KheTimeBreakAfter(KHE_TIME t)
{
  return t->break_after;
}


/*****************************************************************************/
/*                                                                           */
/*  int KheTimeIndex(KHE_TIME t)                                             */
/*                                                                           */
/*  Return the index number of t.                                            */
/*                                                                           */
/*****************************************************************************/

int KheTimeIndex(KHE_TIME t)
{
  return t->index;
}


/*****************************************************************************/
/*                                                                           */
/*  bool KheTimeHasNeighbour(KHE_TIME t, int delta)                          */
/*                                                                           */
/*  Return true if t has a neighbouring time, delta places away.             */
/*                                                                           */
/*****************************************************************************/

bool KheTimeHasNeighbour(KHE_TIME t, int delta)
{
  int index;
  index = KheTimeIndex(t) + delta;
  return index >= 0 && index < KheInstanceTimeCount(t->instance);
}


/*****************************************************************************/
/*                                                                           */
/*  KHE_TIME KheTimeNeighbour(KHE_TIME t, int delta)                         */
/*                                                                           */
/*  Return the neighbouring time delta places away.                          */
/*                                                                           */
/*****************************************************************************/

KHE_TIME KheTimeNeighbour(KHE_TIME t, int delta)
{
  MAssert(KheTimeHasNeighbour(t, delta),
    "KheTimeNeighbour: specified neighbour does not exist");
  return KheInstanceTime(t->instance, KheTimeIndex(t) + delta);
}


/*****************************************************************************/
/*                                                                           */
/*  Submodule "cycle layer index and offset"                                 */
/*                                                                           */
/*****************************************************************************/

/*****************************************************************************/
/*                                                                           */
/*  void KheTimeSetCycleMeetIndexAndOffset(KHE_TIME t, int index,            */
/*    int offset)                                                            */
/*                                                                           */
/*  Set the cycle soln event index and offset of t.                          */
/*                                                                           */
/*****************************************************************************/

/* ***
void KheTimeSetCycleMeetIndexAndOffset(KHE_TIME t, int index, int offset)
{
  t->cycle_soln_event_index = index;
  t->cycle_soln_event_offset = offset;
}
*** */


/*****************************************************************************/
/*                                                                           */
/*  int KheTimeCycleMeetIndex(KHE_TIME t)                                    */
/*                                                                           */
/*  Return the index of t's solution event in any solution.                  */
/*                                                                           */
/*****************************************************************************/

/* ***
int KheTimeCycleMeetIndex(KHE_TIME t)
{
  return t->cycle_soln_event_index;
}
*** */


/*****************************************************************************/
/*                                                                           */
/*  int KheTimeCycleMeetOffset(KHE_TIME t)                                   */
/*                                                                           */
/*  Return the offset of t within its cycle solution event.                  */
/*                                                                           */
/*****************************************************************************/

/* ***
int KheTimeCycleMeetOffset(KHE_TIME t)
{
  return t->cycle_soln_event_offset;
}
*** */


/*****************************************************************************/
/*                                                                           */
/*  Submodule "inferred breaks"                                              */
/*                                                                           */
/*****************************************************************************/

/*****************************************************************************/
/*                                                                           */
/*  void KheTimeInferNoBreak(KHE_TIME t)                                     */
/*                                                                           */
/*  Infer that there is no break after t.                                    */
/*                                                                           */
/*****************************************************************************/

/* ***
void KheTimeInferNoBreak(KHE_TIME t)
{
  t->inferred_break_after = false;
}
*** */


/*****************************************************************************/
/*                                                                           */
/*  bool KheTimeInferredBreak(KHE_TIME t)                                    */
/*                                                                           */
/*  Return true if there is an inferred break after t.                       */
/*                                                                           */
/*****************************************************************************/

/* ***
bool KheTimeInferredBreak(KHE_TIME t)
{
  return t->inferred_break_after;
}
*** */


/*****************************************************************************/
/*                                                                           */
/*  void KheTimeSetBreakIfInferred(KHE_TIME t)                               */
/*                                                                           */
/*  If a time break was inferred for t, set its break_after field.           */
/*                                                                           */
/*****************************************************************************/

/* ***
void KheTimeSetBreakIfInferred(KHE_TIME t)
{
  t->break_after = t->inferred_break_after;
}
*** */


/*****************************************************************************/
/*                                                                           */
/*  Auto-generated time group                                                */
/*                                                                           */
/*****************************************************************************/

/*****************************************************************************/
/*                                                                           */
/*  KHE_TIME_GROUP KheTimeSingletonTimeGroup(KHE_TIME t)                     */
/*                                                                           */
/*  Return a time group containing just t.                                   */
/*                                                                           */
/*****************************************************************************/

KHE_TIME_GROUP KheTimeSingletonTimeGroup(KHE_TIME t)
{
  return t->singleton_time_group;
}


/*****************************************************************************/
/*                                                                           */
/*  Reading and writing                                                      */
/*                                                                           */
/*****************************************************************************/

/*****************************************************************************/
/*                                                                           */
/*  bool KheTimeMakeFromKml(KML_ELT time_elt, KHE_INSTANCE ins,KML_ERROR *ke)*/
/*                                                                           */
/*  Add a time to ins based on time_elt.                                     */
/*                                                                           */
/*****************************************************************************/

bool KheTimeMakeFromKml(KML_ELT time_elt, KHE_INSTANCE ins, KML_ERROR *ke)
{
  char *id, *name, *ref;  KML_ELT elt, e;  KHE_TIME t;
  KHE_TIME_GROUP tg;  int j;
  if( !KmlCheck(time_elt, "Id : $Name +Week +Day +TimeGroups", ke) )
    return false;
  id = KmlExtractAttributeValue(time_elt, 0);
  name = KmlExtractText(KmlChild(time_elt, 0));
  if( !KheTimeMake(ins, id, name, false, &t) )
    return KmlErrorMake(ke, KmlLineNum(time_elt), KmlColNum(time_elt),
      "<Time> Id \"%s\" used previously", id);

  /* link to the time's Week time subgroup */
  if( KmlContainsChild(time_elt, "Week", &e) )
  {
    if( !KmlCheck(e, "Reference", ke) )
      return false;
    ref = KmlAttributeValue(e, 0);
    if( !KheInstanceRetrieveTimeGroup(ins, ref, &tg) )
      return KmlErrorMake(ke, KmlLineNum(e), KmlColNum(e),
	"<Week> Reference \"%s\" unknown", ref);
    KheTimeGroupAddTime(tg, t);
  }

  /* link to the time's Day time subgroup */
  if( KmlContainsChild(time_elt, "Day", &e) )
  {
    if( !KmlCheck(e, "Reference", ke) )
      return false;
    ref = KmlAttributeValue(e, 0);
    if( !KheInstanceRetrieveTimeGroup(ins, ref, &tg) )
      return KmlErrorMake(ke, KmlLineNum(e), KmlColNum(e),
	"<Day> Reference \"%s\" unknown", ref);
    KheTimeGroupAddTime(tg, t);
  }

  /* link to the time's TimeGroup time subgroups */
  if( KmlContainsChild(time_elt, "TimeGroups", &elt) )
  {
    if( !KmlCheck(elt, ": *TimeGroup", ke) )
      return false;
    for( j = 0;  j < KmlChildCount(elt);  j++ )
    {
      e = KmlChild(elt, j);
      if( !KmlCheck(e, "Reference", ke) )
	return false;
      ref = KmlAttributeValue(e, 0);
      if( !KheInstanceRetrieveTimeGroup(ins, ref, &tg) )
	return KmlErrorMake(ke, KmlLineNum(e), KmlColNum(e),
	  "<TimeGroup> Reference \"%s\" unknown", ref);
      KheTimeGroupAddTime(tg, t);
    }
  }
  return true;
}


/*****************************************************************************/
/*                                                                           */
/*  bool KheTimeWrite(KHE_TIME t, KML_FILE kf)                               */
/*                                                                           */
/*  Write t to kf.                                                           */
/*                                                                           */
/*****************************************************************************/

bool KheTimeWrite(KHE_TIME t, KML_FILE kf)
{
  static ARRAY_KHE_TIME_GROUP time_groups;
  KHE_TIME_GROUP tg;  int i;

  /* header and name */
  if( t->id == NULL || t->name == NULL )
    return false;
  KmlBegin(kf, "Time");
  KmlAttribute(kf, "Id", t->id);
  KmlEltPrintf(kf, "Name", "%s", t->name);

  /* find the time groups containing t */
  MArrayInit(time_groups);
  for( i = 0;  i < KheInstanceTimeGroupCount(t->instance);  i++ )
  {
    tg = KheInstanceTimeGroup(t->instance, i);
    if( KheTimeGroupContains(tg, t) )
      MArrayAddLast(time_groups, tg);
  }

  /* if the first is a Week, remove it and print it */
  if( MArraySize(time_groups) > 0 )
  {
    tg = MArrayFirst(time_groups);
    if( KheTimeGroupKind(tg) == KHE_TIME_GROUP_KIND_WEEK )
    {
      MArrayRemoveFirst(time_groups);
      KmlEltAttribute(kf, "Week", "Reference", KheTimeGroupId(tg));
    }
  }

  /* if the first remaining is a Day, remove it and print it */
  if( MArraySize(time_groups) > 0 )
  {
    tg = MArrayFirst(time_groups);
    if( KheTimeGroupKind(tg) == KHE_TIME_GROUP_KIND_DAY )
    {
      MArrayRemoveFirst(time_groups);
      KmlEltAttribute(kf, "Day", "Reference", KheTimeGroupId(tg));
    }
  }

  /* if there are any remaining time groups, print them ordinarily */
  if( MArraySize(time_groups) > 0 )
  {
    KmlBegin(kf, "TimeGroups");
    MArrayForEach(time_groups, &tg, &i)
      KmlEltAttribute(kf, "TimeGroup", "Reference", KheTimeGroupId(tg));
    KmlEnd(kf, "TimeGroups");
  }

  /* print footer and return */
  KmlEnd(kf, "Time");
  return true;
}
