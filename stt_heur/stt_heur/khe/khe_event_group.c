
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
/*  FILE:         khe_event_group.c                                          */
/*  DESCRIPTION:  A group of events                                          */
/*                                                                           */
/*****************************************************************************/
#include "khe_interns.h"


/*****************************************************************************/
/*                                                                           */
/*  KHE_EVENT_GROUP - an event group                                         */
/*                                                                           */
/*  Implementation note.  The events_set attribute is always defined, and    */
/*  it holds the true current value of the event group as a set of events.   */
/*  The event_indexes attribute is a redundant alternative representation    */
/*  of the same information, updated lazily:  when the events change, it     */
/*  is cleared, and when a traversal is required, or the event group needs   */
/*  to become immutable, it is made consistent with events_set.              */
/*                                                                           */
/*****************************************************************************/

struct khe_event_group_rec {
  void				*back;			/* back pointer      */
  KHE_EVENT_GROUP_TYPE		event_group_type;	/* event group type  */
  KHE_INSTANCE			instance;		/* enclosing instance*/
  KHE_EVENT_GROUP_KIND		kind;			/* course or ordinary*/
  char				*id;			/* Id                */
  char				*name;			/* Name              */
  LSET				events_set;		/* events set        */
  ARRAY_SHORT			event_indexes;		/* events array      */
  ARRAY_KHE_CONSTRAINT		constraints;		/* constraints       */
};


/*****************************************************************************/
/*                                                                           */
/*  Submodule "internal operations"                                          */
/*                                                                           */
/*****************************************************************************/

/*****************************************************************************/
/*                                                                           */
/*  KHE_EVENT_GROUP_TYPE KheEventGroupType(KHE_EVENT_GROUP eg)               */
/*                                                                           */
/*  Return the event group type of eg.                                       */
/*                                                                           */
/*****************************************************************************/

KHE_EVENT_GROUP_TYPE KheEventGroupType(KHE_EVENT_GROUP eg)
{
  return eg->event_group_type;
}


/*****************************************************************************/
/*                                                                           */
/*  KHE_EVENT_GROUP KheEventGroupMakeInternal(                               */
/*    KHE_EVENT_GROUP_TYPE event_group_type, KHE_INSTANCE ins,               */
/*    KHE_EVENT_GROUP_KIND kind, char *id, char *name)                       */
/*                                                                           */
/*  Make a event group of the given type, but do not add it to ins.          */
/*                                                                           */
/*****************************************************************************/

KHE_EVENT_GROUP KheEventGroupMakeInternal(
  KHE_EVENT_GROUP_TYPE event_group_type, KHE_INSTANCE ins,
  KHE_EVENT_GROUP_KIND kind, char *id, char *name)
{
  KHE_EVENT_GROUP res;
  MMake(res);
  res->back = NULL;
  res->event_group_type = event_group_type;
  res->instance = ins;
  res->kind = kind;
  res->id = id;
  res->name = name;
  res->events_set = LSetNew();
  MArrayInit(res->event_indexes);
  MArrayInit(res->constraints);
  return res;
}


/*****************************************************************************/
/*                                                                           */
/*  void KheEventGroupAddEventInternal(KHE_EVENT_GROUP eg, KHE_EVENT e)      */
/*                                                                           */
/*  Add e to eg.                                                             */
/*                                                                           */
/*****************************************************************************/

void KheEventGroupAddEventInternal(KHE_EVENT_GROUP eg, KHE_EVENT e)
{
  LSetInsert(&eg->events_set, KheEventIndex(e));
  MArrayClear(eg->event_indexes);
}


/*****************************************************************************/
/*                                                                           */
/*  void KheEventGroupSubEventInternal(KHE_EVENT_GROUP eg, KHE_EVENT e)      */
/*                                                                           */
/*  Subtract e from eg.                                                      */
/*                                                                           */
/*****************************************************************************/

void KheEventGroupSubEventInternal(KHE_EVENT_GROUP eg, KHE_EVENT e)
{
  LSetDelete(eg->events_set, KheEventIndex(e));
  MArrayClear(eg->event_indexes);
}


/*****************************************************************************/
/*                                                                           */
/*  void KheEventGroupUnionInternal(KHE_EVENT_GROUP eg, KHE_EVENT_GROUP eg2) */
/*                                                                           */
/*  Update eg's events to be their union with eg2's events.                  */
/*                                                                           */
/*****************************************************************************/

void KheEventGroupUnionInternal(KHE_EVENT_GROUP eg, KHE_EVENT_GROUP eg2)
{
  LSetUnion(&eg->events_set, eg2->events_set);
  MArrayClear(eg->event_indexes);
}


/*****************************************************************************/
/*                                                                           */
/*  void KheEventGroupIntersectInternal(KHE_EVENT_GROUP eg,                  */
/*    KHE_EVENT_GROUP eg2)                                                   */
/*                                                                           */
/*  Update eg's events to be their intersection with eg2's events.           */
/*                                                                           */
/*****************************************************************************/

void KheEventGroupIntersectInternal(KHE_EVENT_GROUP eg, KHE_EVENT_GROUP eg2)
{
  LSetIntersection(eg->events_set, eg2->events_set);
  MArrayClear(eg->event_indexes);
}


/*****************************************************************************/
/*                                                                           */
/*  void KheEventGroupDifferenceInternal(KHE_EVENT_GROUP eg,                 */
/*    KHE_EVENT_GROUP eg2)                                                   */
/*                                                                           */
/*  Update eg's events to be the set difference of them with eg2's events.   */
/*                                                                           */
/*****************************************************************************/

void KheEventGroupDifferenceInternal(KHE_EVENT_GROUP eg, KHE_EVENT_GROUP eg2)
{
  LSetDifference(eg->events_set, eg2->events_set);
  MArrayClear(eg->event_indexes);
}


/*****************************************************************************/
/*                                                                           */
/*  void KheEventGroupSetEventsArrayInternal(KHE_EVENT_GROUP eg)             */
/*                                                                           */
/*  Make eg's event_indexes attribute consistent with its events_set, if     */
/*  this has not already been done.                                          */
/*                                                                           */
/*****************************************************************************/

void KheEventGroupSetEventsArrayInternal(KHE_EVENT_GROUP eg)
{
  int i;  short ix;
  if( MArraySize(eg->event_indexes) == 0 )
  {
    LSetExpand(eg->events_set, &eg->event_indexes);
    if( eg->event_group_type == KHE_EVENT_GROUP_TYPE_USER )
      MArrayForEach(eg->event_indexes, &ix, &i)
	KheEventAddUserEventGroup(KheInstanceEvent(eg->instance, ix), eg);
  }
}


/*****************************************************************************/
/*                                                                           */
/*  void KheEventGroupDelete(KHE_EVENT_GROUP eg)                             */
/*                                                                           */
/*  Delete eg.                                                               */
/*                                                                           */
/*****************************************************************************/

void KheEventGroupDelete(KHE_EVENT_GROUP eg)
{
  LSetFree(eg->events_set);
  MArrayFree(eg->event_indexes);
  MArrayFree(eg->constraints);  /* not freeing the constraints themselves */
  MFree(eg);
}


/*****************************************************************************/
/*                                                                           */
/*  Submodule "construction and query"                                       */
/*                                                                           */
/*****************************************************************************/

/*****************************************************************************/
/*                                                                           */
/*  bool KheEventGroupMake(KHE_INSTANCE ins, KHE_EVENT_GROUP_KIND kind,      */
/*    char *id, char *name, KHE_EVENT_GROUP *eg)                             */
/*                                                                           */
/*  Make a new, empty event group with these attributes and add it to ins.   */
/*                                                                           */
/*****************************************************************************/

bool KheEventGroupMake(KHE_INSTANCE ins, KHE_EVENT_GROUP_KIND kind,
  char *id, char *name, KHE_EVENT_GROUP *eg)
{
  KHE_EVENT_GROUP res;
  MAssert(!KheInstanceComplete(ins),
    "KheEventGroupMake called after KheInstanceMakeEnd");
  if( id != NULL && KheInstanceRetrieveEventGroup(ins, id, &res) )
  {
    *eg = NULL;
    return false;
  }
  res = KheEventGroupMakeInternal(KHE_EVENT_GROUP_TYPE_USER, ins, kind,
    id, name);
  KheInstanceAddEventGroup(ins, res);
  *eg = res;
  return true;
}


/*****************************************************************************/
/*                                                                           */
/*  void KheEventGroupSetBack(KHE_EVENT_GROUP eg, void *back)                */
/*                                                                           */
/*  Set the back pointer of eg.                                              */
/*                                                                           */
/*****************************************************************************/

void KheEventGroupSetBack(KHE_EVENT_GROUP eg, void *back)
{
  MAssert(eg->event_group_type == KHE_TIME_GROUP_TYPE_SOLN ||
      !KheInstanceComplete(eg->instance),
    "KheEventGroupSetBack called after KheInstanceMakeEnd");
  eg->back = back;
}


/*****************************************************************************/
/*                                                                           */
/*  void *KheEventGroupBack(KHE_EVENT_GROUP eg)                              */
/*                                                                           */
/*  Return the back pointer of eg.                                           */
/*                                                                           */
/*****************************************************************************/

void *KheEventGroupBack(KHE_EVENT_GROUP eg)
{
  return eg->back;
}


/*****************************************************************************/
/*                                                                           */
/*  KHE_INSTANCE KheEventGroupInstance(KHE_EVENT_GROUP eg)                   */
/*                                                                           */
/*  Return the instance attribute of eg.                                     */
/*                                                                           */
/*****************************************************************************/

KHE_INSTANCE KheEventGroupInstance(KHE_EVENT_GROUP eg)
{
  return eg->instance;
}


/*****************************************************************************/
/*                                                                           */
/*  KHE_EVENT_GROUP_KIND KheEventGroupKind(KHE_EVENT_GROUP eg)               */
/*                                                                           */
/*  Return the kind attribute of eg.                                         */
/*                                                                           */
/*****************************************************************************/

KHE_EVENT_GROUP_KIND KheEventGroupKind(KHE_EVENT_GROUP eg)
{
  return eg->kind;
}


/*****************************************************************************/
/*                                                                           */
/*  char *KheEventGroupId(KHE_EVENT_GROUP eg)                                */
/*                                                                           */
/*  Return the id attribute of eg.                                           */
/*                                                                           */
/*****************************************************************************/

char *KheEventGroupId(KHE_EVENT_GROUP eg)
{
  return eg->id;
}


/*****************************************************************************/
/*                                                                           */
/*  char *KheEventGroupName(KHE_EVENT_GROUP eg)                              */
/*                                                                           */
/*  Return the name attribute of eg.                                         */
/*                                                                           */
/*****************************************************************************/

char *KheEventGroupName(KHE_EVENT_GROUP eg)
{
  return eg->name;
}


/*****************************************************************************/
/*                                                                           */
/*  Submodule "events construction"                                          */
/*                                                                           */
/*****************************************************************************/

/*****************************************************************************/
/*                                                                           */
/*  void KheEventGroupAddEvent(KHE_EVENT_GROUP eg, KHE_EVENT e)              */
/*                                                                           */
/*  Add e to eg, first checking that eg is a user-defined event group.       */
/*                                                                           */
/*****************************************************************************/

void KheEventGroupAddEvent(KHE_EVENT_GROUP eg, KHE_EVENT e)
{
  MAssert(!KheInstanceComplete(eg->instance),
    "KheEventGroupAddEvent called after KheInstanceMakeEnd");
  MAssert(eg->event_group_type == KHE_EVENT_GROUP_TYPE_USER,
    "KheEventGroupAddEvent given unchangeable event group");
  KheEventGroupAddEventInternal(eg, e);
}


/*****************************************************************************/
/*                                                                           */
/*  void KheEventGroupSubEvent(KHE_EVENT_GROUP eg, KHE_EVENT e)              */
/*                                                                           */
/*  Remove t from eg, first checking that eg is user-defined.                */
/*                                                                           */
/*****************************************************************************/

void KheEventGroupSubEvent(KHE_EVENT_GROUP eg, KHE_EVENT e)
{
  MAssert(!KheInstanceComplete(eg->instance),
    "KheEventGroupSubEvent called after KheInstanceMakeEnd");
  MAssert(eg->event_group_type == KHE_EVENT_GROUP_TYPE_USER,
    "KheEventGroupSubEvent given unchangeable event group");
  KheEventGroupSubEventInternal(eg, e);
}


/*****************************************************************************/
/*                                                                           */
/*  void KheEventGroupUnion(KHE_EVENT_GROUP eg, KHE_EVENT_GROUP eg2)         */
/*                                                                           */
/*  Set eg's set of events to its union with eg2's.                          */
/*                                                                           */
/*****************************************************************************/

void KheEventGroupUnion(KHE_EVENT_GROUP eg, KHE_EVENT_GROUP eg2)
{
  MAssert(!KheInstanceComplete(eg->instance),
    "KheEventGroupUnion called after KheInstanceMakeEnd");
  MAssert(eg->event_group_type == KHE_EVENT_GROUP_TYPE_USER,
    "KheEventGroupUnion given unchangeable event group");
  KheEventGroupUnionInternal(eg, eg2);
}


/*****************************************************************************/
/*                                                                           */
/*  void KheEventGroupIntersect(KHE_EVENT_GROUP eg, KHE_EVENT_GROUP eg2)     */
/*                                                                           */
/*  Set eg's set of events to its intersection with eg2's.                   */
/*                                                                           */
/*****************************************************************************/

void KheEventGroupIntersect(KHE_EVENT_GROUP eg, KHE_EVENT_GROUP eg2)
{
  MAssert(!KheInstanceComplete(eg->instance),
    "KheEventGroupIntersect called after KheInstanceMakeEnd");
  MAssert(eg->event_group_type == KHE_EVENT_GROUP_TYPE_USER,
    "KheEventGroupIntersect given unchangeable event group");
  KheEventGroupIntersectInternal(eg, eg2);
}


/*****************************************************************************/
/*                                                                           */
/*  void KheEventGroupDifference(KHE_EVENT_GROUP eg, KHE_EVENT_GROUP eg2)    */
/*                                                                           */
/*  Set eg's set of events to its difference with eg2's.                     */
/*                                                                           */
/*****************************************************************************/

void KheEventGroupDifference(KHE_EVENT_GROUP eg, KHE_EVENT_GROUP eg2)
{
  MAssert(!KheInstanceComplete(eg->instance),
    "KheEventGroupDifference called after KheInstanceMakeEnd");
  MAssert(eg->event_group_type == KHE_EVENT_GROUP_TYPE_USER,
    "KheEventGroupDifference given unchangeable event group");
  KheEventGroupDifferenceInternal(eg, eg2);
}


/*****************************************************************************/
/*                                                                           */
/*  Submodule "events queries"                                               */
/*                                                                           */
/*****************************************************************************/

/*****************************************************************************/
/*                                                                           */
/*  int KheEventGroupEventCount(KHE_EVENT_GROUP eg)                          */
/*                                                                           */
/*  Return the number of events in eg.                                       */
/*                                                                           */
/*****************************************************************************/

int KheEventGroupEventCount(KHE_EVENT_GROUP eg)
{
  if( MArraySize(eg->event_indexes) == 0 )
    KheEventGroupSetEventsArrayInternal(eg);
  return MArraySize(eg->event_indexes);
}


/*****************************************************************************/
/*                                                                           */
/*  KHE_EVENT KheEventGroupEvent(KHE_EVENT_GROUP eg, int i)                  */
/*                                                                           */
/*  Return the i'th event of eg.                                             */
/*                                                                           */
/*****************************************************************************/

KHE_EVENT KheEventGroupEvent(KHE_EVENT_GROUP eg, int i)
{
  if( MArraySize(eg->event_indexes) == 0 )
    KheEventGroupSetEventsArrayInternal(eg);
  return KheInstanceEvent(eg->instance, MArrayGet(eg->event_indexes, i));
}


/*****************************************************************************/
/*                                                                           */
/*  bool KheEventGroupContains(KHE_EVENT_GROUP eg, KHE_EVENT e)              */
/*                                                                           */
/*  Return true if eg contains e.                                            */
/*                                                                           */
/*****************************************************************************/

bool KheEventGroupContains(KHE_EVENT_GROUP eg, KHE_EVENT e)
{
  return LSetContains(eg->events_set, KheEventIndex(e));
}


/*****************************************************************************/
/*                                                                           */
/*  bool KheEventGroupEqual(KHE_EVENT_GROUP eg1, KHE_EVENT_GROUP eg2)        */
/*                                                                           */
/*  Return true if these event groups are equal as sets.                     */
/*                                                                           */
/*****************************************************************************/

bool KheEventGroupEqual(KHE_EVENT_GROUP eg1, KHE_EVENT_GROUP eg2)
{
  return eg1 == eg2 || LSetEqual(eg1->events_set, eg2->events_set);
}


/*****************************************************************************/
/*                                                                           */
/*  bool KheEventGroupSubset(KHE_EVENT_GROUP eg1, KHE_EVENT_GROUP eg2)       */
/*                                                                           */
/*  Return true if eg1 is a subset of eg2.                                   */
/*                                                                           */
/*****************************************************************************/

bool KheEventGroupSubset(KHE_EVENT_GROUP eg1, KHE_EVENT_GROUP eg2)
{
  return LSetSubset(eg1->events_set, eg2->events_set);
}


/*****************************************************************************/
/*                                                                           */
/*  bool KheEventGroupDisjoint(KHE_EVENT_GROUP eg1, KHE_EVENT_GROUP eg2)     */
/*                                                                           */
/*  Return true if eg1 and eg2 are disjoint.                                 */
/*                                                                           */
/*****************************************************************************/

bool KheEventGroupDisjoint(KHE_EVENT_GROUP eg1, KHE_EVENT_GROUP eg2)
{
  return LSetDisjoint(eg1->events_set, eg2->events_set);
}


/*****************************************************************************/
/*                                                                           */
/*  Submodule "constraints"                                                  */
/*                                                                           */
/*****************************************************************************/

/*****************************************************************************/
/*                                                                           */
/*  void KheEventGroupAddConstraint(KHE_EVENT_GROUP eg, KHE_CONSTRAINT c)    */
/*                                                                           */
/*  Add c to eg.                                                             */
/*                                                                           */
/*****************************************************************************/

void KheEventGroupAddConstraint(KHE_EVENT_GROUP eg, KHE_CONSTRAINT c)
{
  MArrayAddLast(eg->constraints, c);
}


/*****************************************************************************/
/*                                                                           */
/*  int KheEventGroupConstraintCount(KHE_EVENT_GROUP eg)                     */
/*                                                                           */
/*  Return the number of constraints applicable to eg.                       */
/*                                                                           */
/*****************************************************************************/

int KheEventGroupConstraintCount(KHE_EVENT_GROUP eg)
{
  return MArraySize(eg->constraints);
}


/*****************************************************************************/
/*                                                                           */
/*  KHE_CONSTRAINT KheEventGroupConstraint(KHE_EVENT_GROUP eg, int i)        */
/*                                                                           */
/*  Return the i'th constraint applicable to eg.                             */
/*                                                                           */
/*****************************************************************************/

KHE_CONSTRAINT KheEventGroupConstraint(KHE_EVENT_GROUP eg, int i)
{
  return MArrayGet(eg->constraints, i);
}


/*****************************************************************************/
/*                                                                           */
/*  Submodule "reading and writing"                                          */
/*                                                                           */
/*****************************************************************************/

/*****************************************************************************/
/*                                                                           */
/*  bool KheEventGroupMakeFromKml(KML_ELT event_group_elt, KHE_INSTANCE ins, */
/*    KML_ERROR *ke)                                                         */
/*                                                                           */
/*  Make and event group based on event_group_elt and add it to ins.         */
/*                                                                           */
/*****************************************************************************/

bool KheEventGroupMakeFromKml(KML_ELT event_group_elt, KHE_INSTANCE ins,
  KML_ERROR *ke)
{
  char *id, *name;  KHE_EVENT_GROUP eg;  KHE_EVENT_GROUP_KIND kind;
  if( !KmlCheck(event_group_elt, "Id : $Name", ke) )
    return false;
  kind = strcmp(KmlLabel(event_group_elt), "Course") == 0 ?
    KHE_EVENT_GROUP_KIND_COURSE : KHE_EVENT_GROUP_KIND_ORDINARY;
  id = KmlExtractAttributeValue(event_group_elt, 0);
  name = KmlExtractText(KmlChild(event_group_elt, 0));
  if( !KheEventGroupMake(ins, kind, id, name, &eg) )
    return KmlErrorMake(ke, KmlLineNum(event_group_elt),
      KmlColNum(event_group_elt),
      "<%s> Id \"%s\" used previously", KmlLabel(event_group_elt), id);
  return true;
}


/*****************************************************************************/
/*                                                                           */
/*  bool KheEventGroupWrite(KHE_EVENT_GROUP eg, KML_FILE kf)                 */
/*                                                                           */
/*  Write eg to kf.                                                          */
/*                                                                           */
/*****************************************************************************/

bool KheEventGroupWrite(KHE_EVENT_GROUP eg, KML_FILE kf)
{
  char *kind_str;
  if( eg->id == NULL || eg->name == NULL )
    return false;
  kind_str = (eg->kind==KHE_EVENT_GROUP_KIND_COURSE ? "Course" : "EventGroup");
  if( strlen(eg->name) < 19 )
    KmlEltAttributeEltPrintf(kf, kind_str, "Id", eg->id, "Name","%s",eg->name);
  else
  {
    KmlBegin(kf, kind_str);
    KmlAttribute(kf, "Id", eg->id);
    KmlEltPrintf(kf, "Name", "%s", eg->name);
    KmlEnd(kf, kind_str);
  }
  return true;
}
