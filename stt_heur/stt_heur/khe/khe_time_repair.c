
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
/*  FILE:         khe_time_repair.c                                          */
/*  DESCRIPTION:  Ejection chain time repair                                 */
/*                                                                           */
/*****************************************************************************/
#include "khe.h"
#include "m.h"
#include <limits.h>

#define DEBUG1 0

#define WITH_DEMAND_NODE_SWAPS 1
#define WITH_SPREAD_EVENTS_NODE_SWAPS 0
#define WITH_PREFER_TIMES_NODE_SWAPS 0

typedef MARRAY(KHE_MEET) ARRAY_KHE_MEET;


/*****************************************************************************/
/*                                                                           */
/*  Submodule "Kempe meet moves"                                             */
/*                                                                           */
/*****************************************************************************/

/*****************************************************************************/
/*                                                                           */
/*  bool KheKempeMeetMoveAugment(KHE_EJECTOR ej, KHE_MEET meet,              */
/*    KHE_MEET target_meet, int target_offset, KHE_TRANSACTION tn,           */
/*    KHE_TRACE tc)                                                          */
/*                                                                           */
/*  Try Kempe-moving meet to target_meet at target_offset.                   */
/*  Parameters tn and tc are scratch transaction and trace objects.          */
/*                                                                           */
/*****************************************************************************/

static bool KheKempeMeetMoveAugment(KHE_EJECTOR ej, KHE_MEET meet,
  KHE_MEET target_meet, int target_offset, KHE_TRANSACTION tn, KHE_TRACE tc)
{
  bool success;  int d;
  KheTransactionBegin(tn);
  KheTraceBegin(tc);
  success = KheKempeMeetMove(meet, target_meet, target_offset, &d);
  KheTraceEnd(tc);
  KheTransactionEnd(tn);
  if( success && KheEjectorSuccess(ej, tc, d) )
    return true;
  KheTransactionUndo(tn);
  return false;
}


/*****************************************************************************/
/*                                                                           */
/*  bool KheKempeMeetMoveToDomainAugment(KHE_EJECTOR ej, KHE_MEET meet,      */
/*    KHE_TIME_GROUP domain, int base, KHE_TRANSACTION tn, KHE_TRACE tc)     */
/*                                                                           */
/*  Try moving meet to somewhere else in its node's parent node so that its  */
/*  original meet, which is base times further along, goes into domain.      */
/*  Assume that the meet can be moved and has not been visited, but only     */
/*  move it if zones permit.                                                 */
/*                                                                           */
/*****************************************************************************/

static bool KheKempeMeetMoveToDomainAugment(KHE_EJECTOR ej, KHE_MEET meet,
  KHE_TIME_GROUP domain, int base, KHE_TRANSACTION tn, KHE_TRACE tc)
{
  KHE_MEET target_meet;  KHE_TIME t;  int i, max_offset, offset;
  KHE_NODE parent_node;
  parent_node = KheNodeParent(KheMeetNode(meet));
  for( i = 0;  i < KheNodeMeetCount(parent_node);  i++ )
  {
    target_meet = KheNodeMeet(parent_node, i);
    t = KheMeetAsstTime(target_meet);
    if( t != NULL )
    {
      max_offset = KheMeetDuration(target_meet) - KheMeetDuration(meet);
      for( offset = 0;  offset <= max_offset;  offset++ )
	if( KheTimeGroupContains(domain, KheTimeNeighbour(t, base + offset)) &&
	    KheMeetMovePreservesZones(KheMeetAsst(meet),KheMeetAsstOffset(meet),
	      target_meet, offset, KheMeetDuration(meet)) &&
	    KheKempeMeetMoveAugment(ej, meet, target_meet, offset, tn, tc) )
	  return true;
    }
  }
  return false;
}


/*****************************************************************************/
/*                                                                           */
/*  bool KheKempeMeetMoveToAnywhereAugment(KHE_EJECTOR ej, KHE_MEET meet,    */
/*    KHE_TRANSACTION tn, KHE_TRACE tc)                                      */
/*                                                                           */
/*  Like KheKempeMeetMoveToDomainAugment, except that the meet may move to   */
/*  any meet and offset in the parent node that zones permit.                */
/*                                                                           */
/*****************************************************************************/

static bool KheKempeMeetMoveToAnywhereAugment(KHE_EJECTOR ej, KHE_MEET meet,
  KHE_TRANSACTION tn, KHE_TRACE tc)
{
  KHE_MEET target_meet;  int i, max_offset, offset;  KHE_NODE parent_node;
  parent_node = KheNodeParent(KheMeetNode(meet));
  for( i = 0;  i < KheNodeMeetCount(parent_node);  i++ )
  {
    target_meet = KheNodeMeet(parent_node, i);
    max_offset = KheMeetDuration(target_meet) - KheMeetDuration(meet);
    for( offset = 0;  offset <= max_offset;  offset++ )
      if( KheMeetMovePreservesZones(KheMeetAsst(meet), KheMeetAsstOffset(meet),
	    target_meet, offset, KheMeetDuration(meet)) &&
	  KheKempeMeetMoveAugment(ej, meet, target_meet, offset, tn, tc) )
	return true;
  }
  return false;
}


/*****************************************************************************/
/*                                                                           */
/*  Submodule "node swaps"                                                   */
/*                                                                           */
/*****************************************************************************/

/*****************************************************************************/
/*                                                                           */
/*  bool KheNodeSwapAugment(KHE_EJECTOR ej, KHE_NODE node1, KHE_NODE node2,  */
/*    KHE_TRANSACTION tn, KHE_TRACE tc)                                      */
/*                                                                           */
/*  Try a node swap of node1 and node2.                                      */
/*  Parameters tn and tc are scratch transaction and trace objects.          */
/*                                                                           */
/*****************************************************************************/

static bool KheNodeSwapAugment(KHE_EJECTOR ej, KHE_NODE node1, KHE_NODE node2,
  KHE_TRANSACTION tn, KHE_TRACE tc)
{
  bool success;
  KheTransactionBegin(tn);
  KheTraceBegin(tc);
  success = KheNodeMeetSwap(node1, node2);
  KheTraceEnd(tc);
  KheTransactionEnd(tn);
  if( success &&
      KheEjectorSuccess(ej, tc, KheNodeDemand(node1) + KheNodeDemand(node2)) )
    return true;
  KheTransactionUndo(tn);
  return false;
}


/*****************************************************************************/
/*                                                                           */
/*  bool KheNodeSwapToSimilarNodeAugment(KHE_EJECTOR ej, KHE_NODE node,      */
/*    KHE_TRANSACTION tn, KHE_TRACE tc)                                      */
/*                                                                           */
/*  Try swapping node with other nodes that it shares a layer with.          */
/*  The node is known to be not visited.                                     */
/*                                                                           */
/*****************************************************************************/

static bool KheNodeSwapToSimilarNodeAugment(KHE_EJECTOR ej, KHE_NODE node,
  KHE_TRANSACTION tn, KHE_TRACE tc)
{
  KHE_LAYER layer;  KHE_NODE node2;  int i;
  if( KheNodeParentLayerCount(node) > 0 )
  {
    layer = KheNodeParentLayer(node, 0);
    for( i = 0;  i < KheLayerChildNodeCount(layer);  i++ )
    {
      node2 = KheLayerChildNode(layer, i);
      if( node2 != node && KheNodeSameParentLayers(node, node2) &&
	  KheNodeSwapAugment(ej, node, node2, tn, tc) )
	return true;
    }
  }
  return false;
}


/*****************************************************************************/
/*                                                                           */
/*  Submodule "prefer times augment"                                         */
/*                                                                           */
/*****************************************************************************/

/*****************************************************************************/
/*                                                                           */
/*  bool KhePreferAugment(KHE_EJECTOR ej, KHE_PREFER_TIMES_MONITOR ptm,      */
/*    KHE_TRANSACTION tn, KHE_TRACE tc)                                      */
/*                                                                           */
/*  Try to repair ptm.  Parameters tn and tc are scratch objects.            */
/*                                                                           */
/*****************************************************************************/

static bool KhePreferAugment(KHE_EJECTOR ej, KHE_PREFER_TIMES_MONITOR ptm,
  KHE_TRANSACTION tn, KHE_TRACE tc)
{
  KHE_SOLN soln;  KHE_EVENT e;  KHE_MEET meet;  int i, durn, base;
  KHE_PREFER_TIMES_CONSTRAINT ptc;  KHE_TIME_GROUP domain;  KHE_TIME t;
  KHE_NODE node;
  soln = KheEjectorSoln(ej);
  e = KhePreferTimesMonitorEvent(ptm);
  ptc = KhePreferTimesMonitorConstraint(ptm);
  domain = KhePreferTimesConstraintDomain(ptc);
  durn = KhePreferTimesConstraintDuration(ptc);
  for( i = 0;  i < KheEventMeetCount(soln, e);  i++ )
  {
    meet = KheEventMeet(soln, e, i);
    t = KheMeetAsstTime(meet);
    if( (durn == KHE_NO_DURATION || KheMeetDuration(meet) == durn) &&
	t != NULL && !KheTimeGroupContains(domain, t) )
    {
      /* try a repair at each ancestor of meet lying in a node */
      base = 0;
      while( KheMeetAsst(meet) != NULL )
      {
	node = KheMeetNode(meet);
	if( node != NULL )
	{
	  /* try moving meet */
	  if( !KheMeetVisited(meet, 0) )
	  {
	    KheMeetVisit(meet);
	    if( KheKempeMeetMoveToDomainAugment(ej, meet, domain, base,tn,tc) )
	      return true;
	    if( KheEjectorScheduleMayRevisit(KheEjectorCurrSchedule(ej)) )
	      KheMeetUnVisit(meet);
	  }

	  /* try swapping meet's node */
	  if( WITH_PREFER_TIMES_NODE_SWAPS && !KheNodeVisited(node, 0) )
	  {
	    KheNodeVisit(node);
	    if( KheNodeSwapToSimilarNodeAugment(ej, node, tn, tc) )
	      return true;
	    if( KheEjectorScheduleMayRevisit(KheEjectorCurrSchedule(ej)) )
	      KheNodeUnVisit(node);
	  }
	}
	base += KheMeetAsstOffset(meet);
	meet = KheMeetAsst(meet);
      } 
    }
  }
  return false;
}


/*****************************************************************************/
/*                                                                           */
/*  Submodule "spread augments"                                              */
/*                                                                           */
/*****************************************************************************/

/*****************************************************************************/
/*                                                                           */
/*  bool KheTimeGroupIsHigh(KHE_SPREAD_EVENTS_MONITOR sem, int i,            */
/*    KHE_TIME_GROUP *tg)                                                    */
/*                                                                           */
/*  If the i'th time group of sem is high, return true and set *tg to the    */
/*  time group, otherwise return false.                                      */
/*                                                                           */
/*****************************************************************************/

static bool KheTimeGroupIsHigh(KHE_SPREAD_EVENTS_MONITOR sem, int i,
  KHE_TIME_GROUP *tg)
{
  int minimum, maximum, inc;
  KheSpreadEventsMonitorTimeGroup(sem, i, tg, &minimum, &maximum, &inc);
  return inc > maximum;
}


/*****************************************************************************/
/*                                                                           */
/*  bool KheSpreadEventsMonitorContainsHighTimeGroup(                        */
/*    KHE_SPREAD_EVENTS_MONITOR sem)                                         */
/*                                                                           */
/*  Return true if sem contains a high time group.                           */
/*                                                                           */
/*****************************************************************************/

static bool KheSpreadEventsMonitorContainsHighTimeGroup(
  KHE_SPREAD_EVENTS_MONITOR sem)
{
  int i;  KHE_TIME_GROUP tg;
  for( i = 0;  i < KheSpreadEventsMonitorTimeGroupCount(sem);  i++ )
    if( KheTimeGroupIsHigh(sem, i, &tg) )
      return true;
  return false;
}


/*****************************************************************************/
/*                                                                           */
/*  bool KheTimeGroupIsLow(KHE_SPREAD_EVENTS_MONITOR sem, int i,             */
/*    KHE_TIME_GROUP *tg)                                                    */
/*                                                                           */
/*  If the i'th time group of sem is low, return true and set *tg to the     */
/*  time group, otherwise return false.                                      */
/*                                                                           */
/*****************************************************************************/

static bool KheTimeGroupIsLow(KHE_SPREAD_EVENTS_MONITOR sem, int i,
  KHE_TIME_GROUP *tg)
{
  int minimum, maximum, inc;
  KheSpreadEventsMonitorTimeGroup(sem, i, tg, &minimum, &maximum, &inc);
  return inc < minimum;
}


/*****************************************************************************/
/*                                                                           */
/*  bool KheSpreadEventsMonitorContainsLowTimeGroup(                         */
/*    KHE_SPREAD_EVENTS_MONITOR sem)                                         */
/*                                                                           */
/*  Return true if sem contains a low time group.                            */
/*                                                                           */
/*****************************************************************************/

static bool KheSpreadEventsMonitorContainsLowTimeGroup(
  KHE_SPREAD_EVENTS_MONITOR sem)
{
  int i;  KHE_TIME_GROUP tg;
  for( i = 0;  i < KheSpreadEventsMonitorTimeGroupCount(sem);  i++ )
    if( KheTimeGroupIsLow(sem, i, &tg) )
      return true;
  return false;
}


/*****************************************************************************/
/*                                                                           */
/*  bool KheMeetIsHigh(KHE_MEET meet, KHE_SPREAD_EVENTS_MONITOR sem)         */
/*                                                                           */
/*  Return true if meet is a high meet.                                      */
/*                                                                           */
/*****************************************************************************/

static bool KheMeetIsHigh(KHE_MEET meet, KHE_SPREAD_EVENTS_MONITOR sem)
{
  int i, minimum, maximum, inc;  KHE_TIME_GROUP tg;  KHE_TIME t;
  t = KheMeetAsstTime(meet);
  if( t != NULL )
  {
    /* find the first time group of sem containing meet's assigned time */
    for( i = 0;  i < KheSpreadEventsMonitorTimeGroupCount(sem);  i++ )
    {
      KheSpreadEventsMonitorTimeGroup(sem, i, &tg, &minimum, &maximum, &inc);
      if( KheTimeGroupContains(tg, t) )
	return inc > maximum;
    }
  }
  return false;
}


/*****************************************************************************/
/*                                                                           */
/*  bool KheMeetIsVeryMiddle(KHE_MEET meet, KHE_SPREAD_EVENTS_MONITOR sem)   */
/*                                                                           */
/*  Return true if meet is a very middle meet (it lies in a middle time      */
/*  group which would remain middle if meet was removed from it).            */
/*                                                                           */
/*****************************************************************************/

static bool KheMeetIsVeryMiddle(KHE_MEET meet, KHE_SPREAD_EVENTS_MONITOR sem)
{
  int i, minimum, maximum, inc;  KHE_TIME_GROUP tg;  KHE_TIME t;
  t = KheMeetAsstTime(meet);
  if( t != NULL )
  {
    /* find the first time group of sem containing meet's assigned time */
    for( i = 0;  i < KheSpreadEventsMonitorTimeGroupCount(sem);  i++ )
    {
      KheSpreadEventsMonitorTimeGroup(sem, i, &tg, &minimum, &maximum, &inc);
      if( KheTimeGroupContains(tg, t) )
	return inc <= maximum && inc > minimum;  /* NB ">" implements "very" */
    }
  }
  return false;
}


/*****************************************************************************/
/*                                                                           */
/*  bool KheTimeGroupIsVeryMiddle(KHE_SPREAD_EVENTS_MONITOR sem, int i,      */
/*    KHE_TIME_GROUP *tg)                                                    */
/*                                                                           */
/*  If the i'th time group of sem is very middle (if it is a middle time     */
/*  group and would remain so if another meet was added to it), return       */
/*  true and set *tg to the time group, otherwise return false.              */
/*                                                                           */
/*****************************************************************************/

static bool KheTimeGroupIsVeryMiddle(KHE_SPREAD_EVENTS_MONITOR sem, int i,
  KHE_TIME_GROUP *tg)
{
  int minimum, maximum, inc;
  KheSpreadEventsMonitorTimeGroup(sem, i, tg, &minimum, &maximum, &inc);
  return inc >= minimum && inc < maximum; /* NB "<" implements "very" */
}


/*****************************************************************************/
/*                                                                           */
/*  bool KheMoveToLowOrVeryMiddleAugment(KHE_EJECTOR ej, bool to_middle,     */
/*    KHE_SPREAD_EVENTS_MONITOR sem, KHE_MEET meet, KHE_TRANSACTION tn,      */
/*    KHE_TRACE tc)                                                          */
/*                                                                           */
/*  Try to augment by moving one of meet's ancestors so that meet lies in    */
/*  a low time group, or (but only if to_middle is true) a very middle time  */
/*  group.                                                                   */
/*                                                                           */
/*****************************************************************************/

static bool KheMoveToLowOrVeryMiddleAugment(KHE_EJECTOR ej, bool to_middle,
  KHE_SPREAD_EVENTS_MONITOR sem, KHE_MEET meet, KHE_TRANSACTION tn,
  KHE_TRACE tc)
{
  int base, i;  KHE_TIME_GROUP tg;  KHE_NODE node;

  /* visit every ancestor of meet */
  base = 0;
  while( KheMeetAsst(meet) != NULL )
  {
    node = KheMeetNode(meet);
    if( node != NULL )
    {
      /* try moving the meet in a targeted way */
      if( !KheMeetVisited(meet, 0) )
      {
	KheMeetVisit(meet);

	/* try assigning meet to each low time group */
	for( i = 0;  i < KheSpreadEventsMonitorTimeGroupCount(sem);  i++ )
	  if( KheTimeGroupIsLow(sem, i, &tg) &&
	      KheKempeMeetMoveToDomainAugment(ej, meet, tg, base, tn, tc) )
	    return true;

	/* optionally try assigning meet to each very middle time group */
	if( to_middle )
	  for( i = 0;  i < KheSpreadEventsMonitorTimeGroupCount(sem);  i++ )
	    if( KheTimeGroupIsVeryMiddle(sem, i, &tg) &&
		KheKempeMeetMoveToDomainAugment(ej, meet, tg, base, tn, tc) )
	      return true;

	if( KheEjectorScheduleMayRevisit(KheEjectorCurrSchedule(ej)) )
	  KheMeetUnVisit(meet);
      }

      /* try swapping the meet's node in an untargeted way */
      if( WITH_SPREAD_EVENTS_NODE_SWAPS && !KheNodeVisited(node, 0) )
      {
	KheNodeVisit(node);
	if( KheNodeSwapToSimilarNodeAugment(ej, node, tn, tc) )
	  return true;
	if( KheEjectorScheduleMayRevisit(KheEjectorCurrSchedule(ej)) )
	  KheNodeUnVisit(node);
      }
    }
    base += KheMeetAsstOffset(meet);
    meet = KheMeetAsst(meet);
  }
  return false;
}


/*****************************************************************************/
/*                                                                           */
/*  bool KheSpreadAugment(KHE_EJECTOR ej, KHE_SPREAD_EVENTS_MONITOR sem,     */
/*    KHE_TRANSACTION tn, KHE_TRACE tc)                                      */
/*                                                                           */
/*  Repair d.                                                                */
/*                                                                           */
/*****************************************************************************/

static bool KheSpreadAugment(KHE_EJECTOR ej, KHE_SPREAD_EVENTS_MONITOR sem,
  KHE_TRANSACTION tn, KHE_TRACE tc)
{
  KHE_SOLN soln;  KHE_EVENT_GROUP eg;  KHE_EVENT e;  KHE_MEET meet;  int i, j;
  soln = KheEjectorSoln(ej);
  eg = KheSpreadEventsMonitorEventGroup(sem);

  /* try to move a high meet to a low or very middle time group */
  if( KheSpreadEventsMonitorContainsHighTimeGroup(sem) )
    for( i = 0;  i < KheEventGroupEventCount(eg);  i++ )
    {
      e = KheEventGroupEvent(eg, i);
      for( j = 0;  j < KheEventMeetCount(soln, e);  j++ )
      {
	meet = KheEventMeet(soln, e, j);
	if( KheMeetIsHigh(meet, sem) &&
            KheMoveToLowOrVeryMiddleAugment(ej, true, sem, meet, tn, tc) )
	  return true;
      }
    }

  /* try to move a very middle meet to a low time group */
  if( KheSpreadEventsMonitorContainsLowTimeGroup(sem) )
    for( i = 0;  i < KheEventGroupEventCount(eg);  i++ )
    {
      e = KheEventGroupEvent(eg, i);
      for( j = 0;  j < KheEventMeetCount(soln, e);  j++ )
      {
	meet = KheEventMeet(soln, e, j);
	if( KheMeetIsVeryMiddle(meet, sem) &&
            KheMoveToLowOrVeryMiddleAugment(ej, false, sem, meet, tn, tc) )
	  return true;
      }
    }
  return false;
}


/*****************************************************************************/
/*                                                                           */
/*  Submodule "demand augments"                                              */
/*                                                                           */
/*****************************************************************************/

/*****************************************************************************/
/*                                                                           */
/*  int KheMeetIncreasingDemandCmp(const void *t1, const void *t2)           */
/*                                                                           */
/*  Comparison function for sorting meets by increasing demand.              */
/*                                                                           */
/*****************************************************************************/

static int KheMeetIncreasingDemandCmp(const void *t1, const void *t2)
{
  KHE_MEET meet1 = * (KHE_MEET *) t1;
  KHE_MEET meet2 = * (KHE_MEET *) t2;
  if( KheMeetDemand(meet1) != KheMeetDemand(meet2) )
    return KheMeetDemand(meet1) - KheMeetDemand(meet2);
  else
    return KheMeetIndex(meet1) - KheMeetIndex(meet2);
}


/*****************************************************************************/
/*                                                                           */
/*  void KheAddMeets(ARRAY_KHE_MEET *meets,                                  */
/*    KHE_ORDINARY_DEMAND_MONITOR odm, int *count)                           */
/*                                                                           */
/*  Add the ancestors of odm that lie in nodes to *meets, and                */
/*  increment *count if added anything at all.                               */
/*                                                                           */
/*****************************************************************************/

static void KheAddMeets(ARRAY_KHE_MEET *meets,
  KHE_ORDINARY_DEMAND_MONITOR odm, int *count)
{
  KHE_MEET meet;  bool contributed;
  meet = KheTaskMeet(KheOrdinaryDemandMonitorTask(odm));
  contributed = false;
  while( KheMeetAsst(meet) != NULL )
  {
    if( KheMeetNode(meet) != NULL )
    {
      MAssert(KheNodeParent(KheMeetNode(meet)) != NULL,
	"KheAddMeets internal error");
      MArrayAddLast(*meets, meet);
      contributed = true;
    }
    meet = KheMeetAsst(meet);
  }
  if( contributed )
    (*count)++;
}


/*****************************************************************************/
/*                                                                           */
/*  bool KheTailHasRepeats(ARRAY_KHE_MEET *meets, int count)                 */
/*                                                                           */
/*  Return true if the last count elements of *meets (which must exist)      */
/*  are all equal.                                                           */
/*                                                                           */
/*****************************************************************************/

static bool KheTailHasRepeats(ARRAY_KHE_MEET *meets, int count)
{
  KHE_MEET meet;  int i;
  meet = MArrayLast(*meets);
  for( i = 2;  i <= count;  i++ )
    if( MArrayGet(*meets, MArraySize(*meets) - i) != meet )
      return false;
  return true;
}


/*****************************************************************************/
/*                                                                           */
/*  bool KheOrdinaryDemandAugment(KHE_EJECTOR ej,                            */
/*    KHE_ORDINARY_DEMAND_MONITOR odm)                                       */
/*                                                                           */
/*  Try to fix the ordinary demand problem reported by odm, by moving        */
/*  its meet or one of its competitors' meets.                               */
/*                                                                           */
/*****************************************************************************/

static bool KheOrdinaryDemandAugment(KHE_EJECTOR ej,
  KHE_ORDINARY_DEMAND_MONITOR odm)
{
  KHE_MONITOR m;  int i, count;  ARRAY_KHE_MEET meets;  KHE_MEET meet;
  KHE_TRANSACTION tn;  KHE_TRACE tc;  KHE_NODE node;

  /* build a single array of all competitor meets that may move */
  MArrayInit(meets);
  count = 0;
  KheAddMeets(&meets, odm, &count);
  for( m = KheMonitorFirstCompetitor((KHE_MONITOR) odm);  m != NULL; 
       m = KheMonitorNextCompetitor((KHE_MONITOR) odm) )
    if( KheMonitorTag(m) == KHE_ORDINARY_DEMAND_MONITOR_TAG )
      KheAddMeets(&meets, (KHE_ORDINARY_DEMAND_MONITOR) m, &count);

  /* sort the meets by increasing demand, and bring identical meets together */
  MArraySort(meets, &KheMeetIncreasingDemandCmp);

  /* from the right, remove blocks of count identical meets; moving */
  /* these ones would just shift the whole problem, so we omit them */
  while( MArraySize(meets) >= count && KheTailHasRepeats(&meets, count) )
    MArrayDropFromEnd(meets, count);

  /* uniqueify the array */
  for( i = 1;  i < MArraySize(meets);  i++ )
    if( MArrayGet(meets, i-1) == MArrayGet(meets, i) )
    {
      MArrayRemove(meets, i);
      i--;
    }

  /* try a Kempe move of each element of meets, and a node swap of its node */
  if( MArraySize(meets) > 0 )
  {
    tn = KheTransactionMake(KheEjectorSoln(ej));
    tc = KheTraceMake(KheEjectorGroupMonitor(ej));
    MArrayForEach(meets, &meet, &i)
    {
      /* try a Kempe move of meet, to anywhere in its zone */
      if( !KheMeetVisited(meet, 0) )
      {
	KheMeetVisit(meet);
	if( KheKempeMeetMoveToAnywhereAugment(ej, meet, tn, tc) )
	{
	  MArrayFree(meets);
	  KheTransactionDelete(tn);
	  KheTraceDelete(tc);
	  return true;
	}
	if( KheEjectorScheduleMayRevisit(KheEjectorCurrSchedule(ej)) )
	  KheMeetUnVisit(meet);
      }

      /* try a node swap of meet's node */
      node = KheMeetNode(meet);
      if( WITH_DEMAND_NODE_SWAPS && !KheNodeVisited(node, 0) )
      {
	KheNodeVisit(node);
	if( KheNodeSwapToSimilarNodeAugment(ej, node, tn, tc) )
	{
	  MArrayFree(meets);
	  KheTransactionDelete(tn);
	  KheTraceDelete(tc);
	  return true;
	}
	if( KheEjectorScheduleMayRevisit(KheEjectorCurrSchedule(ej)) )
	  KheNodeUnVisit(node);
      }
    }
    MArrayFree(meets);
    KheTransactionDelete(tn);
    KheTraceDelete(tc);
  }
  return false;
}


/*****************************************************************************/
/*                                                                           */
/*  bool KheWorkloadDemandAugment(KHE_EJECTOR ej,                            */
/*    KHE_WORKLOAD_DEMAND_MONITOR wdm)                                       */
/*                                                                           */
/*  Try to fix the workload demand problem reported by odm, by moving        */
/*  its competitors' meets.                                                  */
/*                                                                           */
/*  At present this function is just a stub, and it will probably remain     */
/*  that way.  Workload demands are inserted first and never removed, and    */
/*  this means that they get first bite at the available supply and so in    */
/*  practice are never unmatched.  So it is hardly worthwhile to implement   */
/*  this function properly.                                                  */
/*                                                                           */
/*****************************************************************************/

static bool KheWorkloadDemandAugment(KHE_EJECTOR ej,
  KHE_WORKLOAD_DEMAND_MONITOR wdm)
{
  return false;
}


/*****************************************************************************/
/*                                                                           */
/*  Submodule "main function and its augment functions"                      */
/*                                                                           */
/*****************************************************************************/

/*****************************************************************************/
/*                                                                           */
/*  bool KheTimeRepairEventMonitorAugment(KHE_EJECTOR ej, KHE_MONITOR d)     */
/*                                                                           */
/*  Augment from d.                                                          */
/*                                                                           */
/*****************************************************************************/

bool KheTimeRepairEventMonitorAugment(KHE_EJECTOR ej, KHE_MONITOR d)
{
  KHE_GROUP_MONITOR dgm;  int i;  bool res;
  KHE_TRANSACTION tn;  KHE_TRACE tc;
  MAssert(KheMonitorCost(d) > 0,
    "KheTimeRepairEventMonitorAugment: d does not have non-zero cost");
  tn = KheTransactionMake(KheEjectorSoln(ej));
  tc = KheTraceMake(KheEjectorGroupMonitor(ej));
  res = false;
  switch( KheMonitorTag(d) )
  {
    case KHE_PREFER_TIMES_MONITOR_TAG:

      res = KhePreferAugment(ej, (KHE_PREFER_TIMES_MONITOR) d, tn, tc);
      break;

    case KHE_SPREAD_EVENTS_MONITOR_TAG:

      res = KheSpreadAugment(ej, (KHE_SPREAD_EVENTS_MONITOR) d, tn, tc);
      break;

    case KHE_ASSIGN_TIME_MONITOR_TAG:
    case KHE_SPLIT_EVENTS_MONITOR_TAG:
    case KHE_DISTRIBUTE_SPLIT_EVENTS_MONITOR_TAG:
    case KHE_LINK_EVENTS_MONITOR_TAG:

      break;

    case KHE_GROUP_MONITOR_TAG:

      dgm = (KHE_GROUP_MONITOR) d;
      KheGroupMonitorCopyDefects(dgm);
      res = false;
      for( i = 0;  !res && i < KheGroupMonitorDefectCopyCount(dgm);  i++ )
      {
	d = KheGroupMonitorDefectCopy(dgm, i);
	if( KheMonitorCost(d) > 0 ) switch( KheMonitorTag(d) )
	{
	  case KHE_PREFER_TIMES_MONITOR_TAG:

	    res = KhePreferAugment(ej, (KHE_PREFER_TIMES_MONITOR) d, tn, tc);
	    break;

	  case KHE_SPREAD_EVENTS_MONITOR_TAG:

	    res = KheSpreadAugment(ej, (KHE_SPREAD_EVENTS_MONITOR) d, tn, tc);
	    break;

	  case KHE_ASSIGN_TIME_MONITOR_TAG:
	  case KHE_SPLIT_EVENTS_MONITOR_TAG:
	  case KHE_DISTRIBUTE_SPLIT_EVENTS_MONITOR_TAG:
	  case KHE_LINK_EVENTS_MONITOR_TAG:

	    break;

	  default:

	    MAssert(false, "KheTimeRepairEventMonitorAugment: "
	      "d has a child which is not an event monitor");
	    break;
	}
      }
      break;

    default:

      MAssert(false, "KheTimeRepairEventMonitorAugment: d is not an event monitor");
      break;
  }
  KheTransactionDelete(tn);
  KheTraceDelete(tc);
  return res;
}


/*****************************************************************************/
/*                                                                           */
/*  bool KheTimeRepairDemandMonitorAugment(KHE_EJECTOR ej, KHE_MONITOR d)    */
/*                                                                           */
/*  Augment from d.                                                          */
/*                                                                           */
/*****************************************************************************/

bool KheTimeRepairDemandMonitorAugment(KHE_EJECTOR ej, KHE_MONITOR d)
{
  KHE_GROUP_MONITOR dgm;  int i;
  MAssert(KheMonitorCost(d) > 0,
    "KheTimeRepairDemandMonitorAugment: d does not have non-zero cost");
  switch( KheMonitorTag(d) )
  {
    case KHE_ORDINARY_DEMAND_MONITOR_TAG:

      return KheOrdinaryDemandAugment(ej, (KHE_ORDINARY_DEMAND_MONITOR) d);

    case KHE_WORKLOAD_DEMAND_MONITOR_TAG:

      return KheWorkloadDemandAugment(ej, (KHE_WORKLOAD_DEMAND_MONITOR) d);

    case KHE_GROUP_MONITOR_TAG:

      dgm = (KHE_GROUP_MONITOR) d;
      KheGroupMonitorCopyDefects(dgm);
      for( i = 0;  i < KheGroupMonitorDefectCopyCount(dgm);  i++ )
      {
	d = KheGroupMonitorDefectCopy(dgm, i);
	if( KheMonitorCost(d) > 0 )  switch( KheMonitorTag(d) )
	{
	  case KHE_ORDINARY_DEMAND_MONITOR_TAG:

	    if( KheOrdinaryDemandAugment(ej, (KHE_ORDINARY_DEMAND_MONITOR) d) )
	      return true;
	    break;

	  case KHE_WORKLOAD_DEMAND_MONITOR_TAG:

	    if( KheWorkloadDemandAugment(ej, (KHE_WORKLOAD_DEMAND_MONITOR) d) )
	      return true;
	    break;

	  default:

	    MAssert(false, "KheTimeRepairDemandMonitorAugment: "
	      "d has a child which is not a demand monitor");
	    break;
	}
      }
      break;

    default:

      MAssert(false, "KheTimeRepairDemandMonitorAugment: "
	"d is not a demand monitor");
      break;
  }
  return false;
}


/*****************************************************************************/
/*                                                                           */
/*  bool KheEjectionChainRepairTimes(KHE_NODE parent_node)                   */
/*                                                                           */
/*  Use an ejection chain to repair the time assignments of the meets of     */
/*  the descendants of parent_node.  The repair operation are Kempe meet     */
/*  moves (required to preserve zones, where present) and node swaps if      */
/*  nodes lie in layers.  Return true if all meets are assigned on return.   */
/*                                                                           */
/*****************************************************************************/

bool KheEjectionChainRepairTimes(KHE_NODE parent_node)
{
  KHE_SOLN soln;  KHE_GROUP_MONITOR gm;  KHE_EJECTOR ej;  bool res;

  if( DEBUG1 )
  {
    fprintf(stderr, "[ KheEjectionChainRepairTimes(");
    KheNodeDebug(parent_node, 1, -1, stderr);
    fprintf(stderr, ")\n");
  }

  /* build the group monitors required by this algorithm */
  soln = KheNodeSoln(parent_node);
  gm = KheNodeGroupMonitorsForMeetRepair(parent_node, parent_node,
    KHE_SUBTAG_EVENT, "EventGroupMonitor",
    KHE_SUBTAG_PREASSIGNED_DEMAND, "PreassignedDemandGroupMonitor",
    KHE_SUBTAG_UNPREASSIGNED_DEMAND, "UnpreassignedDemandGroupMonitor",
    KHE_SUBTAG_NODE, "NodeGroupMonitor", (KHE_GROUP_MONITOR) soln);

  /* set up the ejector and run the algorithm */
  ej = KheEjectorMake(soln);
  KheEjectorAddSchedule(ej, 1, INT_MAX, false);
  KheEjectorAddSchedule(ej, 2, INT_MAX, false);
  KheEjectorAddSchedule(ej, 3, INT_MAX, false);
  KheEjectorAddSchedule(ej, INT_MAX, INT_MAX, false);
  KheEjectorAddGroupAugment(ej, KHE_SUBTAG_EVENT,
    &KheTimeRepairEventMonitorAugment);
  KheEjectorAddGroupAugment(ej, KHE_SUBTAG_PREASSIGNED_DEMAND,
    &KheTimeRepairDemandMonitorAugment);
  KheEjectorAddGroupAugment(ej, KHE_SUBTAG_UNPREASSIGNED_DEMAND,
    &KheTimeRepairDemandMonitorAugment);
  KheEjectorSolve(ej, KHE_EJECTOR_FIRST_SUCCESS, gm);
  KheEjectorDelete(ej);

  /* remove group monitors and exit */
  KheUnGroupMonitors(gm);
  res = KheNodeAllChildMeetsAssigned(parent_node);

  if( DEBUG1 )
    fprintf(stderr, "] KheEjectionChainRepairTimes returning %s\n",
      res ? "true" : "false");
  return res;
}
