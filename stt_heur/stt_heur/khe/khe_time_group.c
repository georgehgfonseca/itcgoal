
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
/*  FILE:         khe_time_group.c                                           */
/*  DESCRIPTION:  A set of times                                             */
/*                                                                           */
/*****************************************************************************/
#include "khe_interns.h"

#define DEBUG1 0
#define DEBUG2 0


/*****************************************************************************/
/*                                                                           */
/*  KHE_TIME_GROUP - a set of times                                          */
/*                                                                           */
/*  Implementation note.  The times_sets attribute is always defined, and    */
/*  MArrayFirst(times_sets) holds the current value of the time group as a   */
/*  set of times.  Later elements of the array hold the same set, only       */
/*  shifted 1, 2 etc. places to the right; but only when finalized.          */
/*                                                                           */
/*  The time_indexes attribute is a redundant alternative representation     */
/*  of the same information, updated lazily: when the times change, it is    */
/*  cleared, and when a traversal is required, or the time group needs to    */
/*  become immutable, it is made consistent with MArrayFirst(times_sets).    */
/*                                                                           */
/*****************************************************************************/

struct khe_time_group_rec {
  void			*back;			/* back pointer              */
  KHE_TIME_GROUP_TYPE	time_group_type;	/* time group type           */
  KHE_INSTANCE		instance;		/* enclosing instance        */
  KHE_TIME_GROUP_KIND	kind;			/* Weeks, Days, etc          */
  char			*id;			/* Id                        */
  char			*name;			/* Name                      */
  LSET			times_set;		/* times as lset             */
  ARRAY_SHORT		time_indexes;		/* times as array            */
  ARRAY_INT		pos_in_group;		/* position in group         */
  KHE_TIME_GROUP_NHOOD	neighbourhood;		/* time group neighbourhood  */
  int			pos_in_nhood;		/* pos of this tg in nhood   */
};


/*****************************************************************************/
/*                                                                           */
/*  Submodule "internal operations"                                          */
/*                                                                           */
/*****************************************************************************/

/*****************************************************************************/
/*                                                                           */
/*  KHE_TIME_GROUP_TYPE KheTimeGroupType(KHE_TIME_GROUP tg)                  */
/*                                                                           */
/*  Return the time group type of tg.                                        */
/*                                                                           */
/*****************************************************************************/

KHE_TIME_GROUP_TYPE KheTimeGroupType(KHE_TIME_GROUP tg)
{
  return tg->time_group_type;
}


/*****************************************************************************/
/*                                                                           */
/*  KHE_TIME_GROUP KheTimeGroupMakeInternal(                                 */
/*    KHE_TIME_GROUP_TYPE time_group_type, KHE_INSTANCE ins,                 */
/*    KHE_TIME_GROUP_KIND kind, char *id, char *name, LSET times_set)        */
/*                                                                           */
/*  Make a time group of the given type, but do not add it to ins.           */
/*                                                                           */
/*****************************************************************************/

KHE_TIME_GROUP KheTimeGroupMakeInternal(KHE_TIME_GROUP_TYPE time_group_type,
  KHE_INSTANCE ins, KHE_TIME_GROUP_KIND kind, char *id, char *name,
  LSET times_set)
{
  KHE_TIME_GROUP res;
  MMake(res);
  res->back = NULL;
  res->time_group_type = time_group_type;
  res->instance = ins;
  res->kind = kind;
  res->id = id;
  res->name = name;
  res->times_set = times_set;
  MArrayInit(res->time_indexes);
  MArrayInit(res->pos_in_group);
  res->neighbourhood = NULL;  /* done when finalizing */
  res->pos_in_nhood = -1;     /* done when finalizing */
  return res;
}


/*****************************************************************************/
/*                                                                           */
/*  void KheTimeGroupAddTimeInternal(KHE_TIME_GROUP tg, KHE_TIME t)          */
/*                                                                           */
/*  Add t to tg.                                                             */
/*                                                                           */
/*****************************************************************************/

void KheTimeGroupAddTimeInternal(KHE_TIME_GROUP tg, KHE_TIME t)
{
  LSetInsert(&tg->times_set, KheTimeIndex(t));
  MArrayClear(tg->time_indexes);
}


/*****************************************************************************/
/*                                                                           */
/*  void KheTimeGroupSubTimeInternal(KHE_TIME_GROUP tg, KHE_TIME t)          */
/*                                                                           */
/*  Subtract t from tg.                                                      */
/*                                                                           */
/*****************************************************************************/

void KheTimeGroupSubTimeInternal(KHE_TIME_GROUP tg, KHE_TIME t)
{
  LSetDelete(tg->times_set, KheTimeIndex(t));
  MArrayClear(tg->time_indexes);
}


/*****************************************************************************/
/*                                                                           */
/*  void KheTimeGroupUnionInternal(KHE_TIME_GROUP tg, KHE_TIME_GROUP tg2)    */
/*                                                                           */
/*  Update tg's times to be their union with tg2's times.                    */
/*                                                                           */
/*****************************************************************************/

void KheTimeGroupUnionInternal(KHE_TIME_GROUP tg, KHE_TIME_GROUP tg2)
{
  LSetUnion(&tg->times_set, tg2->times_set);
  MArrayClear(tg->time_indexes);
}


/*****************************************************************************/
/*                                                                           */
/*  void KheTimeGroupIntersectInternal(KHE_TIME_GROUP tg, KHE_TIME_GROUP tg2)*/
/*                                                                           */
/*  Update tg's times to be their intersection with tg2's times.             */
/*                                                                           */
/*****************************************************************************/

void KheTimeGroupIntersectInternal(KHE_TIME_GROUP tg, KHE_TIME_GROUP tg2)
{
  LSetIntersection(tg->times_set, tg2->times_set);
  MArrayClear(tg->time_indexes);
}


/*****************************************************************************/
/*                                                                           */
/*  void KheTimeGroupDifferenceInternal(KHE_TIME_GROUP tg,                   */
/*    KHE_TIME_GROUP tg2)                                                    */
/*                                                                           */
/*  Update tg's times to be the set difference of them with tg2's times.     */
/*                                                                           */
/*****************************************************************************/

void KheTimeGroupDifferenceInternal(KHE_TIME_GROUP tg, KHE_TIME_GROUP tg2)
{
  LSetDifference(tg->times_set, tg2->times_set);
  MArrayClear(tg->time_indexes);
}


/*****************************************************************************/
/*                                                                           */
/*  void KheTimeGroupSetTimesArrayInternal(KHE_TIME_GROUP tg)                */
/*                                                                           */
/*  Make tg's time_indexes attribute consistent with its times_set.          */
/*                                                                           */
/*****************************************************************************/

void KheTimeGroupSetTimesArrayInternal(KHE_TIME_GROUP tg)
{
  if( MArraySize(tg->time_indexes) == 0 )
    LSetExpand(tg->times_set, &tg->time_indexes);
}


/*****************************************************************************/
/*                                                                           */
/*  char *KheTimeGroupTypeShow(KHE_TIME_GROUP_TYPE tgt)                      */
/*                                                                           */
/*  Return a string showing the time group type.                             */
/*                                                                           */
/*****************************************************************************/

static char *KheTimeGroupTypeShow(KHE_TIME_GROUP_TYPE tgt)
{
  switch( tgt )
  {
    case KHE_TIME_GROUP_TYPE_FULL:	return "full";
    case KHE_TIME_GROUP_TYPE_EMPTY:	return "empty";
    case KHE_TIME_GROUP_TYPE_SINGLETON:	return "singleton";
    case KHE_TIME_GROUP_TYPE_PACKING:	return "packing";
    case KHE_TIME_GROUP_TYPE_NEIGHBOUR:	return "neighbour";
    case KHE_TIME_GROUP_TYPE_USER:	return "user";
    case KHE_TIME_GROUP_TYPE_SOLN:	return "soln";
    default:				return "?";
  }
}


/*****************************************************************************/
/*                                                                           */
/*  void KheTimeGroupFinalize(KHE_TIME_GROUP tg,  KHE_TIME_GROUP_NHOOD tgn,  */
/*    int pos_in_tgn)                                                        */
/*                                                                           */
/*  Finalize tg.  If tgn is non-NULL, then it is tg's neighbourhood and      */
/*  pos_in_tgn is its position in that neighbourhood.  Otherwise need to     */
/*  build a neighbourhood.                                                   */
/*                                                                           */
/*****************************************************************************/

void KheTimeGroupFinalize(KHE_TIME_GROUP tg,  KHE_TIME_GROUP_NHOOD tgn,
  int pos_in_tgn)
{
  int i;  short ix;
  if( DEBUG1 )
    fprintf(stderr, "[ KheTimeGroupFinalize(%p %s)\n", (void *) tg,
      tg->id != NULL ? tg->id : "-");

  /* finalize the times array */
  KheTimeGroupSetTimesArrayInternal(tg);

  /* finalize pos_in_group */
  MArrayFill(tg->pos_in_group, KheInstanceTimeCount(tg->instance), -1);
  MArrayForEach(tg->time_indexes, &ix, &i)
    MArrayPut(tg->pos_in_group, ix, i);

  /* finalize the neighbourhood */
  switch( tg->time_group_type )
  {
    case KHE_TIME_GROUP_TYPE_SINGLETON:

      /* get from instance and update the shared singleton neighbourhood */
      MAssert(tgn == NULL, "KheTimeGroupFinalize internal error 1");
      MAssert(MArraySize(tg->time_indexes) == 1,
	"KheTimeGroupFinalize internal error 2");
      tg->neighbourhood =
	KheInstanceSingletonTimeGroupNeighbourhood(tg->instance);
      tg->pos_in_nhood = MArrayFirst(tg->time_indexes);
      KheTimeGroupNHoodSetTimeGroup(tg->neighbourhood, tg->pos_in_nhood, tg);
      break;

    case KHE_TIME_GROUP_TYPE_NEIGHBOUR:

      /* tgn is the neighbourhood, use it */
      MAssert(tgn != NULL, "KheTimeGroupFinalize internal error 3");
      tg->neighbourhood = tgn;
      tg->pos_in_nhood = pos_in_tgn;
      break;

    case KHE_TIME_GROUP_TYPE_FULL:
    case KHE_TIME_GROUP_TYPE_EMPTY:
    case KHE_TIME_GROUP_TYPE_PACKING:
    case KHE_TIME_GROUP_TYPE_USER:
    case KHE_TIME_GROUP_TYPE_SOLN:

      /* make a fresh neighbourhood */
      MAssert(tgn == NULL, "KheTimeGroupFinalize internal error 4");
      tg->neighbourhood = KheTimeGroupNhoodMake(tg, &tg->pos_in_nhood);
      break;

    default:

      MAssert(false, "KheTimeGroupFinalize internal error 4");
  }

  /* ***
  max_nonzero = KheInstanceTimeCount(tg->instance) - 1;
  if( tg->time_group_type == KHE_TIME_GROUP_TYPE_SINGLETON )
  {
    ** need as many more lsets as there are times following this one **
    for( i = MArrayFirst(tg->time_indexes) + 1;
	i < KheInstanceTimeCount(tg->instance);  i++ )
    {
      tg2 = KheTimeSingletonTimeGroup(KheInstanceTime(tg->instance, i));
      MArrayAddLast(tg->times_sets, MArrayFirst(tg2->times_sets));
    }
    if( DEBUG1 )
      fprintf(stderr, "singleton with %d time sets\n",
	MArraySize(tg->times_sets));
  }
  else
  {
    ** need as many lsets as the duration of the longest meeting **
    for( i = 1;  i < KheInstanceMaxEventDuration(tg->instance);  i++ )
    {
      lset = LSetNew();
      LSetShift(MArrayFirst(tg->times_sets), &lset, i, max_nonzero);
      MArrayAddLast(tg->times_sets, lset);
    }
    if( DEBUG1 )
      fprintf(stderr, "non-singleton with %d time sets\n",
	MArraySize(tg->times_sets));
  }
  *** */

  if( DEBUG1 )
  {
    fprintf(stderr, "  %s: ", KheTimeGroupTypeShow(tg->time_group_type));
    KheTimeGroupDebug(tg, 3, 0, stderr);
    fprintf(stderr, "] KheTimeGroupFinalize\n");
  }
}


/*****************************************************************************/
/*                                                                           */
/*  void KheTimeGroupDelete(KHE_TIME_GROUP tg)                               */
/*                                                                           */
/*  Delete tg.                                                               */
/*                                                                           */
/*****************************************************************************/

void KheTimeGroupDelete(KHE_TIME_GROUP tg)
{
  LSetFree(tg->times_set);
  MArrayFree(tg->time_indexes);
  MArrayFree(tg->pos_in_group);
  if( tg->time_group_type != KHE_TIME_GROUP_TYPE_SINGLETON &&
      tg->time_group_type != KHE_TIME_GROUP_TYPE_NEIGHBOUR &&
      tg->neighbourhood != NULL )
    KheTimeGroupNHoodDelete(tg->neighbourhood, tg->pos_in_nhood);
  MFree(tg);
}


/*****************************************************************************/
/*                                                                           */
/*  Submodule "construction and query"                                       */
/*                                                                           */
/*****************************************************************************/

/*****************************************************************************/
/*                                                                           */
/*  bool KheTimeGroupMake(KHE_INSTANCE ins, KHE_TIME_GROUP_KIND kind,        */
/*    char *id, char *name, KHE_TIME_GROUP *tg)                              */
/*                                                                           */
/*  Make a user-defined time group and add it to ins.                        */
/*                                                                           */
/*****************************************************************************/

bool KheTimeGroupMake(KHE_INSTANCE ins, KHE_TIME_GROUP_KIND kind,
  char *id, char *name, KHE_TIME_GROUP *tg)
{
  KHE_TIME_GROUP res;
  MAssert(!KheInstanceComplete(ins),
    "KheTimeGroupMake called after KheInstanceMakeEnd");
  if( id != NULL && KheInstanceRetrieveTimeGroup(ins, id, &res) )
  {
    *tg = NULL;
    return false;
  }
  *tg = KheTimeGroupMakeInternal(KHE_TIME_GROUP_TYPE_USER, ins, kind,
    id, name, LSetNew());
  KheInstanceAddTimeGroup(ins, *tg);
  return true;
}


/*****************************************************************************/
/*                                                                           */
/*  void KheTimeGroupSetBack(KHE_TIME_GROUP tg, void *back)                  */
/*                                                                           */
/*  Set the back pointer of tg.                                              */
/*                                                                           */
/*****************************************************************************/

void KheTimeGroupSetBack(KHE_TIME_GROUP tg, void *back)
{
  MAssert(tg->time_group_type == KHE_TIME_GROUP_TYPE_SOLN ||
      !KheInstanceComplete(tg->instance),
    "KheTimeGroupSetBack called after KheInstanceMakeEnd");
  tg->back = back;
}


/*****************************************************************************/
/*                                                                           */
/*  void *KheTimeGroupBack(KHE_TIME_GROUP tg)                                */
/*                                                                           */
/*  Return the back pointer of tg.                                           */
/*                                                                           */
/*****************************************************************************/

void *KheTimeGroupBack(KHE_TIME_GROUP tg)
{
  return tg->back;
}


/*****************************************************************************/
/*                                                                           */
/*  KHE_INSTANCE KheTimeGroupInstance(KHE_TIME_GROUP tg)                     */
/*                                                                           */
/*  Return the instance containing tg.                                       */
/*                                                                           */
/*****************************************************************************/

KHE_INSTANCE KheTimeGroupInstance(KHE_TIME_GROUP tg)
{
  return tg->instance;
}


/*****************************************************************************/
/*                                                                           */
/*  KHE_TIME_GROUP_KIND KheTimeGroupKind(KHE_TIME_GROUP tg)                  */
/*                                                                           */
/*  Return the kind of tg.                                                   */
/*                                                                           */
/*****************************************************************************/

KHE_TIME_GROUP_KIND KheTimeGroupKind(KHE_TIME_GROUP tg)
{
  return tg->kind;
}


/*****************************************************************************/
/*                                                                           */
/*  char *KheTimeGroupId(KHE_TIME_GROUP tg)                                  */
/*                                                                           */
/*  Return the Id of tg.                                                     */
/*                                                                           */
/*****************************************************************************/

char *KheTimeGroupId(KHE_TIME_GROUP tg)
{
  return tg->id;
}


/*****************************************************************************/
/*                                                                           */
/*  char *KheTimeGroupName(KHE_TIME_GROUP tg)                                */
/*                                                                           */
/*  Return the name of tg.                                                   */
/*                                                                           */
/*****************************************************************************/

char *KheTimeGroupName(KHE_TIME_GROUP tg)
{
  return tg->name;
}


/*****************************************************************************/
/*                                                                           */
/*  Submodule "times construction"                                           */
/*                                                                           */
/*****************************************************************************/

/*****************************************************************************/
/*                                                                           */
/*  void KheTimeGroupAddTime(KHE_TIME_GROUP tg, KHE_TIME t)                  */
/*                                                                           */
/*  Add t to tg, first checking that tg is a user-defined time group.        */
/*                                                                           */
/*****************************************************************************/

void KheTimeGroupAddTime(KHE_TIME_GROUP tg, KHE_TIME t)
{
  MAssert(!KheInstanceComplete(tg->instance),
    "KheTimeGroupAddTime called after KheInstanceMakeEnd");
  MAssert(tg->time_group_type == KHE_TIME_GROUP_TYPE_USER,
    "KheTimeGroupAddTime given unchangeable time group");
  KheTimeGroupAddTimeInternal(tg, t);
}


/*****************************************************************************/
/*                                                                           */
/*  void KheTimeGroupSubTime(KHE_TIME_GROUP tg, KHE_TIME t)                  */
/*                                                                           */
/*  Remove t from tg, first checking that tg is user-defined.                */
/*                                                                           */
/*****************************************************************************/

void KheTimeGroupSubTime(KHE_TIME_GROUP tg, KHE_TIME t)
{
  MAssert(!KheInstanceComplete(tg->instance),
    "KheTimeGroupSubTime called after KheInstanceMakeEnd");
  MAssert(tg->time_group_type == KHE_TIME_GROUP_TYPE_USER,
    "KheTimeGroupSubTime given unchangeable time group");
  KheTimeGroupSubTimeInternal(tg, t);
}


/*****************************************************************************/
/*                                                                           */
/*  void KheTimeGroupUnion(KHE_TIME_GROUP tg, KHE_TIME_GROUP tg2)            */
/*                                                                           */
/*  Set tg's set of times to its union with tg2's.                           */
/*                                                                           */
/*****************************************************************************/

void KheTimeGroupUnion(KHE_TIME_GROUP tg, KHE_TIME_GROUP tg2)
{
  MAssert(!KheInstanceComplete(tg->instance),
    "KheTimeGroupUnion called after KheInstanceMakeEnd");
  MAssert(tg->time_group_type == KHE_TIME_GROUP_TYPE_USER,
    "KheTimeGroupUnion given unchangeable time group");
  KheTimeGroupUnionInternal(tg, tg2);
}


/*****************************************************************************/
/*                                                                           */
/*  void KheTimeGroupIntersect(KHE_TIME_GROUP tg, KHE_TIME_GROUP tg2)        */
/*                                                                           */
/*  Set tg's set of times to its intersection with tg2's.                    */
/*                                                                           */
/*****************************************************************************/

void KheTimeGroupIntersect(KHE_TIME_GROUP tg, KHE_TIME_GROUP tg2)
{
  MAssert(!KheInstanceComplete(tg->instance),
    "KheTimeGroupIntersect called after KheInstanceMakeEnd");
  MAssert(tg->time_group_type == KHE_TIME_GROUP_TYPE_USER,
    "KheTimeGroupIntersect given unchangeable time group");
  KheTimeGroupIntersectInternal(tg, tg2);
}


/*****************************************************************************/
/*                                                                           */
/*  void KheTimeGroupDifference(KHE_TIME_GROUP tg, KHE_TIME_GROUP tg2)       */
/*                                                                           */
/*  Set tg's set of times to its difference with tg2's.                      */
/*                                                                           */
/*****************************************************************************/

void KheTimeGroupDifference(KHE_TIME_GROUP tg, KHE_TIME_GROUP tg2)
{
  MAssert(!KheInstanceComplete(tg->instance),
    "KheTimeGroupDifference called after KheInstanceMakeEnd");
  MAssert(tg->time_group_type == KHE_TIME_GROUP_TYPE_USER,
    "KheTimeGroupDifference given unchangeable time group");
  KheTimeGroupDifferenceInternal(tg, tg2);
}


/*****************************************************************************/
/*                                                                           */
/*  Submodule "times queries"                                                */
/*                                                                           */
/*****************************************************************************/

/*****************************************************************************/
/*                                                                           */
/*  int KheTimeGroupTimeCount(KHE_TIME_GROUP tg)                             */
/*                                                                           */
/*  Return the number of times in tg.                                        */
/*                                                                           */
/*****************************************************************************/

int KheTimeGroupTimeCount(KHE_TIME_GROUP tg)
{
  if( MArraySize(tg->time_indexes) == 0 )
    KheTimeGroupSetTimesArrayInternal(tg);
  return MArraySize(tg->time_indexes);
}


/*****************************************************************************/
/*                                                                           */
/*  KHE_TIME KheTimeGroupTime(KHE_TIME_GROUP tg, int i)                      */
/*                                                                           */
/*  Return the i'th time of tg.                                              */
/*                                                                           */
/*****************************************************************************/

KHE_TIME KheTimeGroupTime(KHE_TIME_GROUP tg, int i)
{
  if( MArraySize(tg->time_indexes) == 0 )
    KheTimeGroupSetTimesArrayInternal(tg);
  return KheInstanceTime(tg->instance, MArrayGet(tg->time_indexes, i));
}


/*****************************************************************************/
/*                                                                           */
/*  bool KheTimeGroupContainsIndex(KHE_TIME_GROUP tg, int time_index)        */
/*                                                                           */
/*  Return true if tg contains the time with this index.                     */
/*                                                                           */
/*****************************************************************************/

bool KheTimeGroupContainsIndex(KHE_TIME_GROUP tg, int time_index)
{
  return LSetContains(tg->times_set, time_index);
}


/*****************************************************************************/
/*                                                                           */
/*  bool TimeGroupContains(KHE_TIME_GROUP tg, KHE_TIME t)                    */
/*                                                                           */
/*  Return true if tg contains t.  May be called at any time.                */
/*                                                                           */
/*****************************************************************************/

bool KheTimeGroupContains(KHE_TIME_GROUP tg, KHE_TIME t)
{
  return LSetContains(tg->times_set, KheTimeIndex(t));
}


/*****************************************************************************/
/*                                                                           */
/*  bool KheTimeGroupEqual(KHE_TIME_GROUP tg1, KHE_TIME_GROUP tg2)           */
/*                                                                           */
/*  Return true if these two time groups are equal.                          */
/*                                                                           */
/*****************************************************************************/

bool KheTimeGroupEqual(KHE_TIME_GROUP tg1, KHE_TIME_GROUP tg2)
{
  return tg1 == tg2 || LSetEqual(tg1->times_set, tg2->times_set);
}


/*****************************************************************************/
/*                                                                           */
/*  bool KheTimeGroupIntersectionEqual(KHE_TIME_GROUP tg1a,                  */
/*    KHE_TIME_GROUP tg1b, KHE_TIME_GROUP tg2)                               */
/*                                                                           */
/*  Return true if the intersection of tg1a and tg1b equals tg2.             */
/*                                                                           */
/*****************************************************************************/

/* ***
bool KheTimeGroupIntersectionEqual(KHE_TIME_GROUP tg1a,
  KHE_TIME_GROUP tg1b, KHE_TIME_GROUP tg2)
{
  return LSetIntersectionEqual(MArrayFirst(tg1a->times_sets),
    MArrayFirst(tg1a->times_sets), MArrayFirst(tg2->times_sets));
}
*** */


/*****************************************************************************/
/*                                                                           */
/*  bool KheTimeGroupSubset(KHE_TIME_GROUP tg1, KHE_TIME_GROUP tg2)          */
/*                                                                           */
/*  Return true if tg1 is a subset of tg2.                                   */
/*                                                                           */
/*****************************************************************************/

bool KheTimeGroupSubset(KHE_TIME_GROUP tg1, KHE_TIME_GROUP tg2)
{
  return LSetSubset(tg1->times_set, tg2->times_set);
}


/*****************************************************************************/
/*                                                                           */
/*  bool KheTimeGroupDisjoint(KHE_TIME_GROUP tg1, KHE_TIME_GROUP tg2)        */
/*                                                                           */
/*  Return true if tg1 and tg2 are disjoint.                                 */
/*                                                                           */
/*****************************************************************************/

bool KheTimeGroupDisjoint(KHE_TIME_GROUP tg1, KHE_TIME_GROUP tg2)
{
  return LSetDisjoint(tg1->times_set, tg2->times_set);
}


/*****************************************************************************/
/*                                                                           */
/*  KHE_TIME_GROUP KheTimeGroupNeighbour(KHE_TIME_GROUP tg, int delta)       */
/*                                                                           */
/*  Return the time group obtained by shifting tg by delta.                  */
/*                                                                           */
/*****************************************************************************/

KHE_TIME_GROUP KheTimeGroupNeighbour(KHE_TIME_GROUP tg, int delta)
{
  MAssert(tg->neighbourhood != NULL,
    "KheTimeGroupNeighbour called before KheInstanceEnd");
  return KheTimeGroupNHoodNeighbour(tg->neighbourhood, tg->pos_in_nhood+delta);
}


/*****************************************************************************/
/*                                                                           */
/*  KHE_TIME_GROUP KheTimeGroupNeighbourInternal(KHE_TIME_GROUP tg,          */
/*    int delta)                                                             */
/*                                                                           */
/*  Unchecked version of KheTimeGroupNeighbour.  It is safe to call this     */
/*  from solution objects, since a neighbourhood must exist if they do.      */
/*                                                                           */
/*****************************************************************************/

static KHE_TIME_GROUP KheTimeGroupNeighbourInternal(KHE_TIME_GROUP tg,
  int delta)
{
  return KheTimeGroupNHoodNeighbour(tg->neighbourhood, tg->pos_in_nhood+delta);
}


/*****************************************************************************/
/*                                                                           */
/*  bool KheTimeGroupDomainsAllowAssignment(KHE_TIME_GROUP domain,           */
/*    KHE_TIME_GROUP target_domain, int target_offset)                       */
/*                                                                           */
/*  Return true if the domains allow the assignment of a meet with domain    */
/*  domain to a meet with domain target_domain at offset target_offset.      */
/*                                                                           */
/*****************************************************************************/

bool KheTimeGroupDomainsAllowAssignment(KHE_TIME_GROUP domain,
  KHE_TIME_GROUP target_domain, int target_offset)
{
  return KheTimeGroupSubset(
    KheTimeGroupNeighbourInternal(target_domain, target_offset), domain);
  /* ***
  if( DEBUG2 )
  {
    MAssert(target_offset >= 0,
      "KheTimeGroupDomainsAllowAssignment internal error 1");
    MAssert(target_offset < MArraySize(target_se_domain->times_sets),
      "KheTimeGroupDomainsAllowAssignment internal error 2 (%d !< %p: %d)",
      target_offset, (void *) target_se_domain,
      MArraySize(target_se_domain->times_sets));
    MAssert(MArraySize(se_domain->times_sets) > 0,
      "KheTimeGroupDomainsAllowAssignment internal error 3");
  }
  return LSetSubset(MArrayGet(target_se_domain->times_sets, target_offset),
    MArrayFirst(se_domain->times_sets));
  *** */
}


/*****************************************************************************/
/*                                                                           */
/*  unsigned int KheTimeGroupTimePos(KHE_TIME_GROUP tg, int time_index)      */
/*                                                                           */
/*  Return the position in tg of the time whose index number is time_index.  */
/*  For example, the chronologically first time of tg has position 0, the    */
/*  chronologically second time has position 1, and so on.                   */
/*                                                                           */
/*  This function may only be called after tg has been finalized, but since  */
/*  it is private to KHE and efficiency is important here, this condition    */
/*  is not checked.  It should only be called when the indicated time is     */
/*  an element of tg; when called with other times it returns -1.            */
/*                                                                           */
/*****************************************************************************/

int KheTimeGroupTimePos(KHE_TIME_GROUP tg, int time_index)
{
  return MArrayGet(tg->pos_in_group, time_index);
}


/*****************************************************************************/
/*                                                                           */
/*  ARRAY_SHORT KheTimeGroupTimeIndexes(KHE_TIME_GROUP tg)                   */
/*                                                                           */
/*  Return an array of the index numbers of the times of tg.                 */
/*                                                                           */
/*****************************************************************************/

ARRAY_SHORT KheTimeGroupTimeIndexes(KHE_TIME_GROUP tg)
{
  return tg->time_indexes;
}


/*****************************************************************************/
/*                                                                           */
/*  LSET KheTimeGroupTimeSet(KHE_TIME_GROUP tg)                              */
/*                                                                           */
/*  Return the time set defining tg.                                         */
/*                                                                           */
/*****************************************************************************/

LSET KheTimeGroupTimeSet(KHE_TIME_GROUP tg)
{
  return tg->times_set;
}


/*****************************************************************************/
/*                                                                           */
/*  Submodule "reading and writing"                                          */
/*                                                                           */
/*****************************************************************************/

/*****************************************************************************/
/*                                                                           */
/*  bool KheTimeGroupMakeFromKml(KML_ELT time_group_elt, KHE_INSTANCE ins,   */
/*    KML_ERROR *ke)                                                         */
/*                                                                           */
/*  Add a time group from time_group_elt to ins.                             */
/*                                                                           */
/*****************************************************************************/

bool KheTimeGroupMakeFromKml(KML_ELT time_group_elt, KHE_INSTANCE ins,
  KML_ERROR *ke)
{
  KHE_TIME_GROUP_KIND tg_kind;
  KHE_TIME_GROUP tg;  char *id, *name;
  if( !KmlCheck(time_group_elt, "Id : $Name", ke) )
    return false;
  id = KmlExtractAttributeValue(time_group_elt, 0);
  name = KmlExtractText(KmlChild(time_group_elt, 0));
  tg_kind =
    strcmp(KmlLabel(time_group_elt), "Week") == 0 ? KHE_TIME_GROUP_KIND_WEEK :
    strcmp(KmlLabel(time_group_elt), "Day")  == 0 ? KHE_TIME_GROUP_KIND_DAY :
    KHE_TIME_GROUP_KIND_ORDINARY;
  if( !KheTimeGroupMake(ins, tg_kind, id, name, &tg) )
    return KmlErrorMake(ke, KmlLineNum(time_group_elt),
      KmlColNum(time_group_elt), "<TimeGroup> Id \"%s\" used previously", id);
  return true;
}


/*****************************************************************************/
/*                                                                           */
/*  bool KheTimeGroupWrite(KHE_TIME_GROUP tg, KML_FILE kf)                   */
/*                                                                           */
/*  Write tg (just the name and Id) onto kf.                                 */
/*                                                                           */
/*****************************************************************************/

bool KheTimeGroupWrite(KHE_TIME_GROUP tg, KML_FILE kf)
{
  char *kind_str;
  if( tg->id == NULL || tg->name == NULL )
    return false;
  kind_str = tg->kind == KHE_TIME_GROUP_KIND_WEEK ? "Week" :
    tg->kind == KHE_TIME_GROUP_KIND_DAY ? "Day" : "TimeGroup";
  KmlEltAttributeEltPrintf(kf, kind_str, "Id", tg->id, "Name", "%s", tg->name);
  /* ***
  KmlBegin(kf, kind_str);
  KmlAttribute(kf, "Id", tg->id);
  KmlEltPrintf(kf, "Name", "%s", tg->name);
  KmlEnd(kf, kind_str);
  *** */
  return true;
}


/*****************************************************************************/
/*                                                                           */
/*  Submodule "debug"                                                        */
/*                                                                           */
/*****************************************************************************/

/*****************************************************************************/
/*                                                                           */
/*  void KheTimeGroupDebug(KHE_TIME_GROUP tg, int verbosity,                 */
/*    int indent, FILE *fp)                                                  */
/*                                                                           */
/*  Brief debug print of tg onto fp (no indent, no concluding newline).      */
/*                                                                           */
/*****************************************************************************/

void KheTimeGroupDebug(KHE_TIME_GROUP tg, int verbosity,
  int indent, FILE *fp)
{
  KHE_TIME t1, t2;  int i, ix;
  if( verbosity == 1 )
  {
    /* print as briefly as possible, just a name if that's available */
    if( indent >= 0 )
      fprintf(fp, "%*s", indent, "");
    if( KheTimeGroupTimeCount(tg) == 0 )
      fprintf(fp, "{}");
    else if( KheTimeGroupTimeCount(tg) == 1 )
    {
      t1 = KheTimeGroupTime(tg, 0);
      if( KheTimeId(t1) != NULL )
	fprintf(fp, "{%s}", KheTimeId(t1));
      else if( KheTimeGroupId(tg) != NULL )
	fprintf(fp, "%s", KheTimeGroupId(tg));
      else
	fprintf(fp, "{-}");
    }
    else if( KheTimeGroupId(tg) != NULL )
      fprintf(fp, "%s", KheTimeGroupId(tg));
    else
    {
      t1 = KheTimeGroupTime(tg, 0);
      t2 = KheTimeGroupTime(tg, KheTimeGroupTimeCount(tg) - 1);
      fprintf(fp, "{%s%s%s}",
	KheTimeId(t1) != NULL ? KheTimeId(t1) : "-",
	KheTimeGroupTimeCount(tg) == 2 ? "," : "..",
	KheTimeId(t2) != NULL ? KheTimeId(t2) : "-");
    }
    if( indent >= 0 )
      fprintf(fp, "\n");
  }
  else if( verbosity >= 2 )
  {
    /* print the name (if available) and the full value */
    if( indent >= 0 )
      fprintf(fp, "%*s", indent, "");
    if( KheTimeGroupId(tg) != NULL )
      fprintf(fp, "%s", KheTimeGroupId(tg));
    fprintf(stderr, "{");
    MArrayForEach(tg->time_indexes, &ix, &i)
    {
      t1 = KheInstanceTime(tg->instance, ix);
      if( i > 0 )
	fprintf(stderr, ", ");
      fprintf(stderr, KheTimeId(t1) != NULL ? KheTimeId(t1) : "-");
    }
    fprintf(stderr, "}");
    if( indent >= 0 )
      fprintf(fp, "\n");
  }
}
