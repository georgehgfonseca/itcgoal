
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
/*  FILE:         khe_global_time.c                                          */
/*  DESCRIPTION:  Global time swaps                                          */
/*                                                                           */
/*****************************************************************************/
#include "khe.h"
#include "m.h"
#include <limits.h>

#define DEBUG1 0
#define DEBUG2 0
#define DEBUG3 0
#define DEBUG4 0
#define DEBUG5 0
#define DEBUG6 0

typedef MARRAY(KHE_MEET) ARRAY_KHE_MEET;

/*****************************************************************************/
/*                                                                           */
/*  bool KheFullBreak(KHE_MEET meet, int offset)                             */
/*                                                                           */
/*  Return true if the meets assigned directly to meet allow a full          */
/*  break just before this offset.                                           */
/*                                                                           */
/*****************************************************************************/

/* ***
static bool KheFullBreak(KHE_MEET meet, int offset)
{
  int i;  KHE_MEET child_meet;
  for( i = 0;  i < KheMeetAssignedToCount(meet);  i++ )
  {
    child_meet = KheMeetAssignedTo(meet, i);
    if( KheMeetAsstOffset(child_meet) < offset &&
        KheMeetAsstOffset(child_meet) + KheMeetDuration(child_meet) > offset )
      return false;
  }
  return true;
}
*** */


/*****************************************************************************/
/*                                                                           */
/*  KHE_MEET KheAddMeet(KHE_NODE swap_node, int durn,                        */
/*    KHE_MEET target_meet, int target_offset)                               */
/*                                                                           */
/*  Make and return a new meet with the given duration, lying in swap_node   */
/*  and assigned to target_meet at target_offset.                            */
/*                                                                           */
/*****************************************************************************/

/* ***
static KHE_MEET KheAddMeet(KHE_NODE swap_node, int durn,
  KHE_MEET target_meet, int target_offset)
{
  KHE_MEET res;
  res = KheMeetMake(KheNodeSoln(swap_node), durn, NULL);
  if( !KheNodeAddMeet(swap_node, res) )
    MAssert(false, "KheAddMeet internal error 1");
  if( !KheMeetAssign(res, target_meet, target_offset) )
    MAssert(false, "KheAddMeet internal error 2");
  return res;
}
*** */


/*****************************************************************************/
/*                                                                           */
/*  KHE_MEET KheFindSwapTarget(KHE_MEET meet, KHE_NODE swap_node)            */
/*                                                                           */
/*  Find the meet of swap_node such that assigning meet to it leaves meet's  */
/*  assignment in the parent node unchanged.                                 */
/*                                                                           */
/*****************************************************************************/

static KHE_MEET KheFindSwapTarget(KHE_MEET meet, KHE_NODE swap_node)
{
  int i;  KHE_MEET swap_meet;
  for( i = 0;  i < KheNodeMeetCount(swap_node);  i++ )
  {
    swap_meet = KheNodeMeet(swap_node, i);
    if( KheMeetAsst(swap_meet) == KheMeetAsst(meet) )
      return swap_meet;
  }
  MAssert(false, "KheFindSwapTargetAndOffset internal error");
  return NULL;
}


/*****************************************************************************/
/*                                                                           */
/*  void KheMoveChildNode(KHE_NODE child_node, KHE_NODE swap_node)           */
/*                                                                           */
/*  Move child_node to be a child of swap_node.  This includes saving the    */
/*  assignments of its meets, and bringing them back after the move.         */
/*                                                                           */
/*****************************************************************************/

static void KheMoveChildNode(KHE_NODE child_node, KHE_NODE swap_node)
{
  int i, swap_offset;  KHE_MEET child_meet, swap_meet;
  ARRAY_KHE_MEET swap_meets;  ARRAY_INT swap_offsets;
  if( DEBUG3 )
  {
    fprintf(stderr, "[ KheMoveChildNode(");
    KheNodeDebug(child_node, 1, -1, stderr);
    fprintf(stderr, ", ");
    KheNodeDebug(swap_node, 1, -1, stderr);
    fprintf(stderr, ")\n");
  }

  /* store the current assignments and ensure all meets are unassigned */
  MArrayInit(swap_meets);
  MArrayInit(swap_offsets);
  for( i = 0;  i < KheNodeMeetCount(child_node);  i++ )
  {
    child_meet = KheNodeMeet(child_node, i);
    swap_meet = KheFindSwapTarget(child_meet, swap_node);
    MArrayAddLast(swap_meets, swap_meet);
    MArrayAddLast(swap_offsets, KheMeetAsstOffset(child_meet));
    if( DEBUG3 )
    {
      fprintf(stderr, "  saving child_meet: ");
      KheMeetDebug(child_meet, 3, 0, stderr);
      fprintf(stderr, "  swap_meet: ");
      KheMeetDebug(swap_meet, 3, 0, stderr);
    }
    KheMeetUnAssign(child_meet);
  }

  /* move the node */
  if( !KheNodeDeleteParent(child_node) )
    MAssert(false, "KheMoveChildNode internal error 1");
  if( !KheNodeAddParent(child_node, swap_node) )
    MAssert(false, "KheMoveChildNode internal error 2");

  /* assign the meets as recorded */
  for( i = 0;  i < KheNodeMeetCount(child_node);  i++ )
  {
    child_meet = KheNodeMeet(child_node, i);
    swap_meet = MArrayGet(swap_meets, i);
    swap_offset = MArrayGet(swap_offsets, i);
    if( DEBUG3 )
    {
      fprintf(stderr, "  restoring child_meet: ");
      KheMeetDebug(child_meet, 3, 0, stderr);
      fprintf(stderr, "  swap_meet: ");
      KheMeetDebug(swap_meet, 3, 0, stderr);
    }
    if( !KheMeetAssign(child_meet, swap_meet, swap_offset) )
      MAssert(false, "KheMoveChildNode internal error 3");
  }
  MArrayFree(swap_meets);
  MArrayFree(swap_offsets);
  if( DEBUG3 )
    fprintf(stderr, "] KheMoveChildNode returning\n");
}


/*****************************************************************************/
/*                                                                           */
/*  void KheMeetMaximizeDomain(KHE_MEET meet)                                */
/*                                                                           */
/*  Change the domain of meet so that it is as large as possible consistent  */
/*  with the domains of the meets assigned to it.                            */
/*                                                                           */
/*****************************************************************************/

/* *** using automatic domains now instead
static void KheMeetMaximizeDomain(KHE_MEET meet)
{
  KHE_TIME_GROUP tg, child_tg;  int i, count;
  KHE_MEET child_meet;  KHE_SOLN soln;
  if( DEBUG4 )
  {
    fprintf(stderr, "[ KheMeetMaximizeDomain(");
    KheMeetDebug(meet, 1, -1, stderr);
    fprintf(stderr, ")\n");
  }
  count = 0;  tg = NULL;  soln = KheMeetSoln(meet);
  for( i = 0;  i < KheMeetAssignedToCount(meet);  i++ )
  {
    child_meet = KheMeetAssignedTo(meet, i);
    child_tg = KheTimeGroupNeighbour(KheMeetDomain(child_meet),
      - KheMeetAsstOffset(child_meet));
    if( DEBUG4 )
    {
      fprintf(stderr, "  child_meet %2d: ", i);
      KheMeetDebug(child_meet, 4, 0, stderr);
    }
    if( count == 0 )
    {
      ** this is the first child domain, so just use it **
      tg = child_tg;
      count = 1;
    }
    else if( count == 1 )
    {
      ** tg is set to the first, avoid reducing it if we can **
      if( !KheTimeGroupSubset(tg, child_tg) )
      {
	count = 2;
	KheSolnTimeGroupBegin(soln);
	KheSolnTimeGroupUnion(soln, tg);
	KheSolnTimeGroupIntersect(soln, child_tg);
      }
    }
    else
      KheSolnTimeGroupIntersect(soln, child_tg);
  }
  if( count == 0 )
    tg = KheInstanceFullTimeGroup(KheSolnInstance(soln));
  else if( count >= 2 )
    tg = KheSolnTimeGroupEnd(soln);
  if( DEBUG4 )
    KheTimeGroupDebug(tg, 4, 2, stderr);
  if( !KheMeetSetDomain(meet, tg) )
    MAssert(false, "KheMeetMaximizeDomain internal error");
  if( DEBUG4 )
    fprintf(stderr, "] KheMeetMaximizeDomain returning\n");
}
*** */


/*****************************************************************************/
/*                                                                           */
/*  bool KheTrySwap(KHE_MEET meet1, KHE_MEET meet2, KHE_GROUP_MONITOR egm)   */
/*                                                                           */
/*  Swap the assignments of meet1 and meet2, and return true and retain the  */
/*  swap if it reduces solution cost without increasing egm's cost.          */
/*                                                                           */
/*****************************************************************************/

static bool KheTrySwap(KHE_MEET meet1, KHE_MEET meet2, KHE_GROUP_MONITOR egm)
{
  KHE_SOLN soln;  KHE_COST soln_cost_before, egm_cost_before;
  if( DEBUG6 )
  {
    fprintf(stderr, "  [ KheTrySwap(");
    KheMeetDebug(meet1, 1, -1, stderr);
    fprintf(stderr, ", ");
    KheMeetDebug(meet2, 1, -1, stderr);
    fprintf(stderr, ", egm)\n");
  }
  soln = KheMeetSoln(meet1);
  soln_cost_before = KheSolnCost(soln);
  egm_cost_before = KheMonitorCost((KHE_MONITOR) egm);
  if( KheMeetSwap(meet1, meet2) )
  {
    if( KheMonitorCost((KHE_MONITOR) egm) <= egm_cost_before &&
        KheSolnCost(soln) < soln_cost_before )
    {
      if( DEBUG6 || DEBUG1 )
	fprintf(stderr, "  %sKheTrySwap returning true (soln %.4f -> %.4f)\n",
	  DEBUG6 ? "] " : "",
	  KheCostShow(soln_cost_before), KheCostShow(KheSolnCost(soln)));
      return true;
    }
    KheMeetSwap(meet1, meet2);
  }
  if( DEBUG6 )
    fprintf(stderr, "  ] KheTrySwap returning false\n");
  return false;
}


/*****************************************************************************/
/*                                                                           */
/*  void KheNodeGlobalSwapDoSwaps(KHE_NODE swap_node, KHE_GROUP_MONITOR egm) */
/*                                                                           */
/*  Try to find swaps of the meets of swap_node that decrease the solution   */
/*  cost while not increasing the cost of egm.                               */
/*                                                                           */
/*****************************************************************************/

static void KheNodeGlobalSwapDoSwaps(KHE_NODE swap_node, KHE_GROUP_MONITOR egm)
{
  bool progressing;  int start, stop, i, j;  KHE_MEET meet1, meet2;
  if( DEBUG5 )
    fprintf(stderr, "[ KheNodeGlobalSwapDoSwaps(swap_node, egm)\n");
  progressing = true;
  do
  {
    progressing = false;

    /* find all ranges of equal-duration meets */
    for( start = 0;  start < KheNodeMeetCount(swap_node);  start = stop )
    {
      meet1 = KheNodeMeet(swap_node, start);
      for( stop = start + 1;  stop < KheNodeMeetCount(swap_node);  stop++ )
      {
	meet2 = KheNodeMeet(swap_node, stop);
	if( KheMeetDuration(meet2) != KheMeetDuration(meet1) )
	  break;
      }

      if( DEBUG5 )
	fprintf(stderr, "  trying range %d .. %d\n", start, stop);
      for( i = start;  i < stop;  i++ )
      {
	meet1 = KheNodeMeet(swap_node, i);
	for( j = i + 1;  j < stop;  j++ )
	{
	  meet2 = KheNodeMeet(swap_node, j);
	  if( KheTrySwap(meet1, meet2, egm) )
	    progressing = true;
	}
      }
    }

  } while( progressing );
  if( DEBUG5 )
    fprintf(stderr, "] KheNodeGlobalSwapDoSwaps returning\n");
}


/*****************************************************************************/
/*                                                                           */
/*  void KheNodeGlobalSwapRepairTimes(KHE_NODE parent_node)                  */
/*                                                                           */
/*  Repair the assignments in the child nodes of parent_node with global     */
/*  swaps.                                                                   */
/*                                                                           */
/*****************************************************************************/

void KheNodeGlobalSwapRepairTimes(KHE_NODE parent_node)
{
  KHE_GROUP_MONITOR egm;  KHE_SOLN soln;  int i, duration1;
  KHE_MEET parent_meet, swap_meet, junk;  KHE_NODE swap_node, child_node;
  if( DEBUG1 )
  {
    fprintf(stderr, "[ KheNodeGlobalSwapRepairTimes(");
    KheNodeDebug(parent_node, 1, -1, stderr);
    fprintf(stderr, ")\n");
    fprintf(stderr, "  initial cost:\n");
    KheSolnCostByTypeDebug(KheNodeSoln(parent_node), 2, 2, stderr);
  }

  /* group event monitors below parent_node */
  soln = KheNodeSoln(parent_node);
  egm = KheNodeGroupEventMonitors(parent_node, NULL, KHE_SUBTAG_EVENT,
    "EventGroupMonitor", (KHE_GROUP_MONITOR) soln);

  /* build the new swap node and its swap meets */
  swap_node = KheNodeMake(soln);
  if( !KheNodeAddParent(swap_node, parent_node) )
    MAssert(false, "KheNodeGlobalSwapRepairTimes internal error 1");
  for( i = 0;  i < KheNodeMeetCount(parent_node);  i++ )
  {
    parent_meet = KheNodeMeet(parent_node, i);
    swap_meet = KheMeetMake(soln, KheMeetDuration(parent_meet), NULL);
    if( !KheMeetSetDomain(swap_meet, NULL) )
      MAssert(false, "KheNodeGlobalSwapRepairTimes internal error 2");
    /* ***
    if( !KheMeetSetDomain(swap_meet, KheMeetDomain(parent_meet)) )
      MAssert(false, "KheNodeGlobalSwapRepairTimes internal error 2");
    *** */
    if( !KheNodeAddMeet(swap_node, swap_meet) )
      MAssert(false, "KheNodeGlobalSwapRepairTimes internal error 3");
    if( !KheMeetAssign(swap_meet, parent_meet, 0) )
      MAssert(false, "KheNodeGlobalSwapRepairTimes internal error 4");
  }
  if( DEBUG2 )
    KheNodeDebug(swap_node, 4, 2, stderr);

  /* move every child node of parent_node except swap_node to under swap_node */
  for( i = 0;  i < KheNodeChildCount(parent_node);  i++ )
  {
    child_node = KheNodeChild(parent_node, i);
    if( child_node != swap_node &&
	KheNodeAssignedDuration(child_node) == KheNodeDuration(child_node) )
    {
      KheMoveChildNode(child_node, swap_node);
      i--;
    }
  }

  /* split the meets of swap_node at their full breaks.  NB junk, the */
  /* split-off fragment, goes at the end of the node so is done later */
  for( i = 0;  i < KheNodeMeetCount(swap_node);  i++ )
  {
    swap_meet = KheNodeMeet(swap_node, i);
    for( duration1 = 1;  duration1 < KheMeetDuration(swap_meet);  duration1++ )
      KheMeetSplit(swap_meet, duration1, false, &swap_meet, &junk);
      /* this really does work, whether the split succeeds or not */
  }
  KheNodeMeetSort(swap_node, &KheMeetDecreasingDurationCmp);

  /* find maximal domains for the swap meets */
  /* *** using automatic domains now
  for( i = 0;  i < KheNodeMeetCount(swap_node);  i++ )
  {
    swap_meet = KheNodeMeet(swap_node, i);
    KheMeetMaximizeDomain(swap_meet);
  }
  *** */
  if( DEBUG2 )
  {
    fprintf(stderr, "  swap_node ready for swapping:\n");
    KheNodeDebug(swap_node, 4, 4, stderr);
  }

  /* do the actual swapping */
  KheNodeGlobalSwapDoSwaps(swap_node, egm);

  /* remove swap node and grouping */
  KheNodeBypass(swap_node);
  while( KheNodeMeetCount(swap_node) > 0 )
    KheMeetDelete(KheNodeMeet(swap_node, 0));
  KheNodeDelete(swap_node);

  KheUnGroupMonitors(egm);
  if( DEBUG1 )
  {
    fprintf(stderr, "  final cost:\n");
    KheSolnCostByTypeDebug(KheNodeSoln(parent_node), 2, 2, stderr);
    fprintf(stderr, "] KheNodeGlobalSwapRepairTimes returning\n");
  }
}
