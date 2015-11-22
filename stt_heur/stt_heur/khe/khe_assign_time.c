
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
/*  FILE:         khe_assign_time.c                                          */
/*  DESCRIPTION:  Miscellaneous time solvers                                 */
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
#define DEBUG7 0
#define DEBUG8 0

typedef MARRAY(KHE_MEET) ARRAY_KHE_MEET;


/*****************************************************************************/
/*                                                                           */
/*  Submodule "basic time solvers"                                           */
/*                                                                           */
/*****************************************************************************/

/*****************************************************************************/
/*                                                                           */
/*  bool KheNodeUnAssignTimes(KHE_NODE parent_node)                          */
/*                                                                           */
/*  Unassign all the meets in all the child nodes of parent_node.            */
/*                                                                           */
/*****************************************************************************/

bool KheNodeUnAssignTimes(KHE_NODE parent_node)
{
  int i, j;  KHE_NODE child_node;  KHE_MEET meet;
  for( i = 0;  i < KheNodeChildCount(parent_node);  i++ )
  {
    child_node = KheNodeChild(parent_node, i);
    for( j = 0;  j < KheNodeMeetCount(child_node);  j++ )
    {
      meet = KheNodeMeet(child_node, j);
      if( KheMeetAsst(meet) != NULL )
	KheMeetUnAssign(meet);
    }
  }
  return true;
}


/*****************************************************************************/
/*                                                                           */
/*  bool KheLayerUnAssignTimes(KHE_LAYER layer)                              */
/*                                                                           */
/*  Unassign all the meets in all the nodes of layer.                        */
/*                                                                           */
/*****************************************************************************/

bool KheLayerUnAssignTimes(KHE_LAYER layer)
{
  int i, j;  KHE_NODE child_node;  KHE_MEET meet;
  for( i = 0;  i < KheLayerChildNodeCount(layer);  i++ )
  {
    child_node = KheLayerChildNode(layer, i);
    for( j = 0;  j < KheNodeMeetCount(child_node);  j++ )
    {
      meet = KheNodeMeet(child_node, j);
      if( KheMeetAsst(meet) != NULL )
	KheMeetUnAssign(meet);
    }
  }
  return true;
}


/*****************************************************************************/
/*                                                                           */
/*  bool KheNodeAllChildMeetsAssigned(KHE_NODE parent_node)                  */
/*                                                                           */
/*  Return true if all meets in all child nodes of parent_node are assigned. */
/*                                                                           */
/*****************************************************************************/

bool KheNodeAllChildMeetsAssigned(KHE_NODE parent_node)
{
  int i, j;  KHE_NODE child_node;  KHE_MEET meet;
  for( i = 0;  i < KheNodeChildCount(parent_node);  i++ )
  {
    child_node = KheNodeChild(parent_node, i);
    for( j = 0;  j < KheNodeMeetCount(child_node);  j++ )
    {
      meet = KheNodeMeet(child_node, j);
      if( KheMeetAsst(meet) == NULL )
	return false;
    }
  }
  return true;
}


/*****************************************************************************/
/*                                                                           */
/*  bool KheLayerAllChildMeetsAssigned(KHE_LAYER layer)                      */
/*                                                                           */
/*  Return true if all meets in all child nodes of layer are assigned.       */
/*                                                                           */
/*****************************************************************************/

bool KheLayerAllChildMeetsAssigned(KHE_LAYER layer)
{
  int i, j;  KHE_NODE child_node;  KHE_MEET meet;
  for( i = 0;  i < KheLayerChildNodeCount(layer);  i++ )
  {
    child_node = KheLayerChildNode(layer, i);
    for( j = 0;  j < KheNodeMeetCount(child_node);  j++ )
    {
      meet = KheNodeMeet(child_node, j);
      if( KheMeetAsst(meet) == NULL )
	return false;
    }
  }
  return true;
}


/*****************************************************************************/
/*                                                                           */
/*  bool KheAllowAsst(KHE_MEET meet, KHE_MEET target_meet)                   */
/*                                                                           */
/*  Return true if meet is allowed to be assigned to target_meet, because    */
/*  none of meet's fellows are assigned to it already.                       */
/*                                                                           */
/*****************************************************************************/

static bool KheAllowAsst(KHE_MEET meet, KHE_MEET target_meet)
{
  KHE_EVENT e;  KHE_SOLN soln;  int i;
  e = KheMeetEvent(meet);
  if( e == NULL )
    return true;
  soln = KheMeetSoln(meet);
  for( i = 0;  i < KheEventMeetCount(soln, e);  i++ )
    if( KheMeetAsst(KheEventMeet(soln, e, i)) == target_meet )
      return false;
  return true;
}


/*****************************************************************************/
/*                                                                           */
/*  bool KheMakeBestAsst(KHE_MEET meet, KHE_NODE parent_node)                */
/*                                                                           */
/*  If meet can be assigned to any of the meets of parent_node at any        */
/*  offset, make the best of these assignments and return true.  Otherwise   */
/*  return false.                                                            */
/*                                                                           */
/*****************************************************************************/

static bool KheMakeBestAsst(KHE_MEET meet, KHE_NODE parent_node)
{
  KHE_COST cost, best_cost;  int i, offset, offset_max, best_offset;
  KHE_SOLN soln;  KHE_MEET parent_meet, best_parent_meet;
  if( DEBUG1 )
  {
    fprintf(stderr, "  [ KheMakeBestAsst(");
    KheMeetDebug(meet, 1, -1, stderr);
    fprintf(stderr, " (durn %d), Node %d)\n",
      KheMeetDuration(meet), KheNodeIndex(parent_node));
  }
  MAssert(KheMeetAsst(meet) == NULL,
    "KheSimpleAssignTimes internal error 1");
  soln = KheNodeSoln(parent_node);
  best_parent_meet = NULL;
  best_cost = 0;
  best_offset = 0;
  for( i = 0;  i < KheNodeMeetCount(parent_node);  i++ )
  {
    parent_meet = KheNodeMeet(parent_node, i);
    if( DEBUG1 )
    {
      fprintf(stderr, "    trying parent_meet ");
      KheMeetDebug(parent_meet, 1, -1, stderr);
      fprintf(stderr, " (durn %d)\n", KheMeetDuration(parent_meet));
    }
    if( KheAllowAsst(meet, parent_meet) )
    {
      offset_max = KheMeetDuration(parent_meet) - KheMeetDuration(meet);
      for( offset = 0;  offset <= offset_max;  offset++ )
      {
	if( DEBUG1 )
	  fprintf(stderr, "      trying offset %d\n", offset);
	if( KheMeetAssign(meet, parent_meet, offset) )
	{
	  cost = KheSolnCost(soln);
	  if( DEBUG1 )
	  {
	    fprintf(stderr, "        assigned ");
	    KheMeetDebug(meet, 1, -1, stderr);
	    fprintf(stderr, " to ");
	    KheMeetDebug(parent_meet, 1, -1, stderr);
	    fprintf(stderr, "+%d cost %.4f%s\n", offset, KheCostShow(cost),
	      best_parent_meet == NULL || cost < best_cost ? " (best)" : "");
	    /* ***
	    KheMonitorDebug((KHE_MONITOR) KheSolnMatchingMonitor(soln),
	      2, 4, stderr);
	    *** */
	  }
	  if( best_parent_meet == NULL || cost < best_cost )
	  {
	    best_parent_meet = parent_meet;
	    best_offset = offset;
	    best_cost = cost;
	  }
	  KheMeetUnAssign(meet);
	}
      }
    }
  }
  if( best_parent_meet != NULL )
  {
    if( !KheMeetAssign(meet, best_parent_meet, best_offset) )
      MAssert(false, "KheSimpleAssignTimes internal error 2");
    if( DEBUG1 )
    {
      fprintf(stderr, "    finally assigned ");
      KheMeetDebug(meet, 1, -1, stderr);
      fprintf(stderr, " to ");
      KheMeetDebug(best_parent_meet, 1, -1, stderr);
      fprintf(stderr, "+%d\n", best_offset);
      fprintf(stderr, "  ] KheMakeBestAsst returning true\n");
    }
    return true;
  }
  else
  {
    return false;
  }
}


/*****************************************************************************/
/*                                                                           */
/*  bool KheNodeSimpleAssignTimes(KHE_NODE parent_node)                      */
/*                                                                           */
/*  Assign times to the meets of the child nodes of parent_node, using a     */
/*  simple algorithm.                                                        */
/*                                                                           */
/*****************************************************************************/

bool KheNodeSimpleAssignTimes(KHE_NODE parent_node)
{
  int i, j, durn, max_durn;  KHE_NODE child_node;
  KHE_MEET meet;  bool res;
  if( DEBUG1 )
    fprintf(stderr, "[ KheNodeSimpleAssignTimes(Node %d)\n",
      KheNodeIndex(parent_node));

  /* ensure everything is unassigned */
  KheNodeUnAssignTimes(parent_node);

  /* find the maximum duration */
  max_durn = 0;
  for( i = 0;  i < KheNodeChildCount(parent_node);  i++ )
  {
    child_node = KheNodeChild(parent_node, i);
    for( j = 0;  j < KheNodeMeetCount(child_node);  j++ )
    {
      meet = KheNodeMeet(child_node, j);
      if( KheMeetDuration(meet) > max_durn )
	max_durn = KheMeetDuration(meet);
    }
  }

  /* assign each meet in decreasing duration order */
  res = true;
  for( durn = max_durn;  durn > 0;  durn-- )
    for( i = 0;  i < KheNodeChildCount(parent_node);  i++ )
    {
      child_node = KheNodeChild(parent_node, i);
      for( j = 0;  j < KheNodeMeetCount(child_node);  j++ )
      {
	meet = KheNodeMeet(child_node, j);
	if( KheMeetDuration(meet) == durn )
	  res &= KheMakeBestAsst(meet, parent_node);
      }
    }
  if( DEBUG1 )
    fprintf(stderr, "] KheNodeSimpleAssignTimes returning %s\n",
      res ? "true" : "false");
  return res;
}


/*****************************************************************************/
/*                                                                           */
/*  bool KheLayerSimpleAssignTimes(KHE_LAYER layer)                          */
/*                                                                           */
/*  Assign times to the meets of the nodes of layer, using a simple          */
/*  algorithm.                                                               */
/*                                                                           */
/*****************************************************************************/

bool KheLayerSimpleAssignTimes(KHE_LAYER layer)
{
  int i, j, durn, max_durn;  KHE_NODE parent_node, child_node;
  KHE_MEET meet;  bool res;
  if( DEBUG1 )
    fprintf(stderr, "[ KheLayerSimpleAssignTimes(layer)\n");

  /* ensure everything is unassigned */
  parent_node = KheLayerParentNode(layer);
  KheLayerUnAssignTimes(layer);

  /* find the maximum duration */
  max_durn = 0;
  for( i = 0;  i < KheLayerChildNodeCount(layer);  i++ )
  {
    child_node = KheLayerChildNode(layer, i);
    for( j = 0;  j < KheNodeMeetCount(child_node);  j++ )
    {
      meet = KheNodeMeet(child_node, j);
      if( KheMeetDuration(meet) > max_durn )
	max_durn = KheMeetDuration(meet);
    }
  }

  /* assign each meet in decreasing duration order */
  res = true;
  for( durn = max_durn;  durn > 0;  durn-- )
    for( i = 0;  i < KheLayerChildNodeCount(layer);  i++ )
    {
      child_node = KheLayerChildNode(layer, i);
      for( j = 0;  j < KheNodeMeetCount(child_node);  j++ )
      {
	meet = KheNodeMeet(child_node, j);
	if( KheMeetDuration(meet) == durn )
	  res &= KheMakeBestAsst(meet, parent_node);
      }
    }
  if( DEBUG1 )
    fprintf(stderr, "] KheLayerSimpleAssignTimes returning %s\n",
      res ? "true" : "false");
  return res;
}


/*****************************************************************************/
/*                                                                           */
/*  bool KheNodeRecursiveAssignTimes(KHE_NODE parent_node,                   */
/*    KHE_NODE_TIME_SOLVER solver)                                           */
/*                                                                           */
/*  Apply solver to the descendants of parent_node in postorder.             */
/*                                                                           */
/*****************************************************************************/

bool KheNodeRecursiveAssignTimes(KHE_NODE parent_node,
  KHE_NODE_TIME_SOLVER solver)
{
  int i;  bool res;
  res = true;
  for( i = 0;  i < KheNodeChildCount(parent_node);  i++ )
    res &= KheNodeRecursiveAssignTimes(KheNodeChild(parent_node, i), solver);
  res &= solver(parent_node);
  return res;
}


/*****************************************************************************/
/*                                                                           */
/*  Submodule "preassigned meet assignment"                                  */
/*                                                                           */
/*****************************************************************************/

/*****************************************************************************/
/*                                                                           */
/*  bool KheNodePreassignedAssignTimes(KHE_NODE root_node)                   */
/*                                                                           */
/*  Assuming that root_node is the overall root node, search its child       */
/*  nodes for unassigned preassigned meets, and assign them.                 */
/*                                                                           */
/*****************************************************************************/

bool KheNodePreassignedAssignTimes(KHE_NODE root_node)
{
  bool res;  int j, k;  KHE_NODE child_node;  KHE_MEET meet;  KHE_TIME t;
  if( DEBUG2 )
    fprintf(stderr, "[ KheNodePreassignedAssignTimes(Node %d)\n",
      KheNodeIndex(root_node));
  res = true;
  for( j = 0;  j < KheNodeChildCount(root_node);  j++ )
  {
    child_node = KheNodeChild(root_node, j);
    for( k = 0;  k < KheNodeMeetCount(child_node);  k++ )
    {
      meet = KheNodeMeet(child_node, k);
      if( KheMeetAsst(meet) == NULL &&
	  KheMeetIsAssignedPreassigned(meet, false, &t) )
      {
	if( DEBUG2 )
	{
	  fprintf(stderr, "  calling KheMeetAssignTime(");
	  KheMeetDebug(meet, 1, -1, stderr);
	  fprintf(stderr, ", %s)\n", KheTimeId(t) == NULL ? "-" : KheTimeId(t));
	}
	if( !KheMeetAssignTime(meet, t) )
	  res = false;
      }
    }
  }
  if( DEBUG2 )
    fprintf(stderr, "] KheNodePreassignedAssignTimes returning %s\n",
      res ? "true" : "false");
  return res;
}


/*****************************************************************************/
/*                                                                           */
/*  bool KheLayerPreassignedAssignTimes(KHE_LAYER layer)                     */
/*                                                                           */
/*  As for KheNodePreassignedAssignTimes, except search only the child       */
/*  nodes of the i'th segment of root_node.                                  */
/*                                                                           */
/*****************************************************************************/

bool KheLayerPreassignedAssignTimes(KHE_LAYER layer)
{
  bool res;  int i, j;  KHE_NODE child_node;  KHE_MEET meet;  KHE_TIME t;
  if( DEBUG2 )
    fprintf(stderr, "[ KheLayerPreassignedAssignTimes(layer)\n");
  res = true;
  for( i = 0;  i < KheLayerChildNodeCount(layer);  i++ )
  {
    child_node = KheLayerChildNode(layer, i);
    for( j = 0;  j < KheNodeMeetCount(child_node);  j++ )
    {
      meet = KheNodeMeet(child_node, j);
      if( KheMeetAsst(meet) == NULL &&
	  KheMeetIsAssignedPreassigned(meet, false, &t) )
      {
	if( DEBUG2 )
	{
	  fprintf(stderr, "  calling KheMeetAssignTime(");
	  KheMeetDebug(meet, 1, -1, stderr);
	  fprintf(stderr, ", %s)\n", KheTimeId(t) == NULL ? "-" : KheTimeId(t));
	}
	if( !KheMeetAssignTime(meet, t) )
	  res = false;
      }
    }
  }
  if( DEBUG2 )
    fprintf(stderr, "] KheLayerPreassignedAssignTimes returning %s\n",
      res ? "true" : "false");
  return res;
}


/*****************************************************************************/
/*                                                                           */
/*  Submodule "Kempe time moves"                                             */
/*                                                                           */
/*****************************************************************************/

/*****************************************************************************/
/*                                                                           */
/*  bool KheGetCompetitorMeets(KHE_ORDINARY_DEMAND_MONITOR dm,               */
/*    KHE_MEET target, ARRAY_KHE_MEET *meets, int first_unmoved)             */
/*                                                                           */
/*  Here dm is an ordinary demand monitor of non-zero cost (that is, it is   */
/*  unmatched) for a preassigned task.  Follow its competitors to find the   */
/*  competing meets, and add them to *meets.  Return true if successful,     */
/*  false if we get stuck on a problem:                                      */
/*                                                                           */
/*    * One of the competitors is a workload demand monitor, indicating      */
/*      that a resource is being used at an unavailable time;                */
/*                                                                           */
/*    * One of the competitors lies in a meet that does not lie anywhere     */
/*      under target, indicating that the problems have wandered beyond      */
/*      the usual scope and it's all getting out of hand;                    */
/*                                                                           */
/*    * A meet that we want to move has moved before, indicating that a      */
/*      Kempe move doesn't work in this case.                                */
/*                                                                           */
/*****************************************************************************/

/* ***
static bool KheGetCompetitorMeets(KHE_ORDINARY_DEMAND_MONITOR dm,
  KHE_MEET target, ARRAY_KHE_MEET *meets, int first_unmoved)
{
  KHE_MONITOR c;  KHE_TASK task;
  KHE_MEET meet;  int pos;  KHE_RESOURCE junk;
  if( DEBUG7 )
  {
    fprintf(stderr, "    [ KheGetCompetitorMeets(dm)\n");
    fprintf(stderr, "      m: ");
    KheMonitorDebug((KHE_MONITOR) dm, 1, 0, stderr);
    for( c = KheMonitorFirstCompetitor((KHE_MONITOR) dm);  c != NULL; 
	 c = KheMonitorNextCompetitor((KHE_MONITOR) dm) )
    {
      fprintf(stderr, "      c: ");
      KheMonitorDebug(c, 1, 0, stderr);
    }
  }

  for( c = KheMonitorFirstCompetitor((KHE_MONITOR) dm);  c != NULL; 
       c = KheMonitorNextCompetitor((KHE_MONITOR) dm) )
  {
    if( KheMonitorTag(c) == KHE_WORKLOAD_DEMAND_MONITOR_TAG )
    {
      if( DEBUG7 )
      {
	KHE_RESOURCE r;
	r = KheWorkloadDemandMonitorResource((KHE_WORKLOAD_DEMAND_MONITOR) c);
	fprintf(stderr, "    ] KheGetCompetitorMeets false: wrkld of %s\n",
	  KheResourceId(r) == NULL ? "-" : KheResourceId(r));
      }
      if( DEBUG6 )
      {
	KHE_RESOURCE r;
	r = KheWorkloadDemandMonitorResource((KHE_WORKLOAD_DEMAND_MONITOR) c);
	fprintf(stderr, "  (Kempe failed, resource %s unavailable)\n",
	  KheResourceId(r) == NULL ? "-" : KheResourceId(r));
      }
      return false;
    }
    MAssert(KheMonitorTag(c) == KHE_ORDINARY_DEMAND_MONITOR_TAG,
      "KheGetCompetitorMeets internal error 1");
    task = KheOrdinaryDemandMonitorTask((KHE_ORDINARY_DEMAND_MONITOR) c);
    if( KheTaskIsPreassigned(task, &junk) )
    {
      ** find ancestor of meet directly under target, or fail if none **
      meet = KheTaskMeet(task);
      MAssert(meet != NULL, "KheGetCompetitorMeets internal error 2");
      while( KheMeetAsst(meet) != NULL && KheMeetAsst(meet) != target )
	meet = KheMeetAsst(meet);
      if( KheMeetAsst(meet) != target )
      {
	if( DEBUG7 )
	{
	  fprintf(stderr,"    ] KheGetCompetitorMeets false: competitor ");
	  KheMeetDebug(meet, 1, -1, stderr);
	  fprintf(stderr, "  not under target ");
	  KheMeetDebug(target, 1, -1, stderr);
	  fprintf(stderr, "\n");
	}
	if( DEBUG6 )
	{
	  fprintf(stderr, "  (Kempe failed, competitor ");
	  KheMeetDebug(meet, 1, -1, stderr);
	  fprintf(stderr, "out of bounds)\n");
	}
	return false;
      }

      ** ensure meet moves during the next step, or fail if moved previously **
      if( !MArrayContains(*meets, meet, &pos) )
      {
	** meet not seen before, enqueue it for the next step **
	if( DEBUG7 )
	{
	  fprintf(stderr, "      adding meet ");
	  KheMeetDebug(meet, 1, -1, stderr);
	  fprintf(stderr, "\n");
	}
	MArrayAddLast(*meets, meet);
      }
      else if( pos < first_unmoved )
      {
	** meet moved previously, can't move it again, so fail **
	if( DEBUG7 )
	{
	  fprintf(stderr, "    ] KheGetCompetitorMeets false: seen ");
	  KheMeetDebug(meet, 1, -1, stderr);
	  fprintf(stderr, " before\n");
	}
	if( DEBUG6 )
	{
	  fprintf(stderr, "  (Kempe failed, competitor ");
	  KheMeetDebug(meet, 1, -1, stderr);
	  fprintf(stderr, " moved previously)\n");
	}
	return false;
      }
      else
      {
	** meet2 already scheduled to move on the next step **
      }
    }
  }
  if( DEBUG6 )
    fprintf(stderr, "    ] KheGetCompetitorMeets returning true\n");
  return true;
}
*** */


/*****************************************************************************/
/*                                                                           */
/*  bool KheMeetHasPreassignedTask(KHE_MEET meet, KHE_TASK *task)            */
/*                                                                           */
/*  If meet, or any meet assigned to meet directly or indirectly, contains   */
/*  a preassigned task, then set *task to one such task and return true,     */
/*  otherwise return false.                                                  */
/*                                                                           */
/*****************************************************************************/

/* ***
static bool KheMeetHasPreassignedTask(KHE_MEET meet, KHE_TASK *task)
{
  int i;  KHE_RESOURCE junk;  KHE_MEET child_meet;

  ** try meet's own tasks **
  for( i = 0;  i < KheMeetTaskCount(meet);  i++ )
  {
    *task = KheMeetTask(meet, i);
    if( KheTaskIsPreassigned(*task, &junk) )
      return true;
  }

  ** try meets assigned to meet **
  for( i = 0;  i < KheMeetAssignedToCount(meet);  i++ )
  {
    child_meet = KheMeetAssignedTo(meet, i);
    if( KheMeetHasPreassignedTask(child_meet, task) )
      return true;
  }
  return false;
}
*** */


/*****************************************************************************/
/*                                                                           */
/*  bool KheMeetKempeMove(KHE_MEET meet, KHE_MEET target_meet,               */
/*    int target_offset)                                                     */
/*                                                                           */
/*  Make a Kempe time move of meet from wherever it is now to target_meet    */
/*  at target_offset.                                                        */
/*                                                                           */
/*****************************************************************************/

/* ***
bool KheMeetKempeMove(KHE_MEET meet, KHE_MEET target_meet, int target_offset)
{
  int junk;
  return KheMeetKempeMoveWithDemand(meet, target_meet, target_offset, &junk);
}
*** */


/*****************************************************************************/
/*                                                                           */
/*  bool KheMeetKempeMoveTime(KHE_MEET meet, KHE_TIME t)                     */
/*                                                                           */
/*  Make a Kempe move which moves meet to t.                                 */
/*                                                                           */
/*****************************************************************************/

/* ***
bool KheMeetKempeMoveTime(KHE_MEET meet, KHE_TIME t)
{
  int junk;
  return KheMeetKempeMoveTimeWithDemand(meet, t, &junk);
}
*** */


/*****************************************************************************/
/*                                                                           */
/*  bool KheMeetKempeMoveWithDemand(KHE_MEET meet, KHE_MEET target_meet,     */
/*    int target_offset, int *demand)                                        */
/*                                                                           */
/*  Make a Kempe time move of meet from wherever it is now to target_meet    */
/*  at target_offset, and set *demand to the total demand of the nodes of    */
/*  the meets that were moved.                                               */
/*                                                                           */
/*  Implementation note.  This function uses the fact that when a change     */
/*  is made, any new unmatched nodes in the matching lie in the part that    */
/*  changed, not in the part that didn't.                                    */
/*                                                                           */
/*****************************************************************************/

/* ***
bool KheMeetKempeMoveWithDemand(KHE_MEET meet, KHE_MEET target_meet,
  int target_offset, int *demand)
{
  ARRAY_KHE_MEET meets;  KHE_TRACE t;  bool res;
  KHE_MEET target[2], meet2;  int offset[2], step, i, j, first_unmoved;
  KHE_MONITOR m;  KHE_GROUP_MONITOR gm;  KHE_ORDINARY_DEMAND_MONITOR dm;
  KHE_TASK task;  KHE_RESOURCE junk;

  ** meet must have a current assignment **
  MAssert(KheMeetAsst(meet) != NULL, "KheMeetKempeMove: meet not assigned");
  *demand = 0;

  ** find the targets for the steps and fail if the move changes nothing **
  target[0] = KheMeetAsst(meet);
  offset[0] = KheMeetAsstOffset(meet);
  target[1] = target_meet;
  offset[1] = target_offset;
  if( DEBUG5 )
  {
    fprintf(stderr, "  [ KheMeetKempeMove(");
    KheMeetDebug(meet, 1, -1, stderr);
    fprintf(stderr, ", ");
    KheMeetDebug(target_meet, 1, -1, stderr);
    fprintf(stderr, ", %d)\n", target_offset);
  }
  if( target[0] == target[1] && offset[0] == offset[1] )
  {
    if( DEBUG5 )
      fprintf(stderr, "  ] KheMeetKempeMove returning false (non-move)\n");
    if( DEBUG6 )
      fprintf(stderr, "  (Kempe failed, non-move)\n");
    return false;
  }

  ** if there are no preassigned tasks, do an ordinary move **
  if( !KheMeetHasPreassignedTask(meet, &task) )
  {
    res = KheMeetMove(meet, target_meet, target_offset);
    if( KheMeetNode(meet) != NULL )
      *demand += KheNodeDemand(KheMeetNode(meet));
    if( DEBUG5 )
      fprintf(stderr, "  ] KheMeetKempeMove returning %s (trivial)\n",
	res ? "true" : "false");
    if( DEBUG6 && !res )
      fprintf(stderr, "  (Kempe failed trivial move)\n");
    return res;
  }

  ** start tracing, or fail if can't **
  MAssert(KheTaskDemandMonitorCount(task) > 0,
    "KheMeetKempeMove internal error");
  gm = KheMonitorParentMonitor((KHE_MONITOR) KheTaskDemandMonitor(task, 0));
  gm = KheMonitorParentMonitor((KHE_MONITOR) gm);
  if( gm == NULL )
  {
    if( DEBUG5 )
      fprintf(stderr, "  ] KheMeetKempeMove returning false (untraceable)\n");
    if( DEBUG6 )
      fprintf(stderr, "  (Kempe failed, untraceable)\n");
    return false;
  }
  t = KheTraceMake(gm);
  KheTraceBegin(t);

  ** build the initial set of meets to move (just meet) **
  MArrayInit(meets);
  MArrayAddLast(meets, meet);
  first_unmoved = 0;

  ** carry out as many steps as necessary to remove all defects, if possible **
  for( step = 1;  first_unmoved < MArraySize(meets);  step++ )
  {
    ** move the unmoved meets and fail if any refuse to move **
    for( ; first_unmoved < MArraySize(meets);  first_unmoved++ )
    {
      meet2 = MArrayGet(meets, first_unmoved);
      if( !KheMeetMove(meet2, target[step % 2], offset[step % 2]) )
      {
	MArrayFree(meets);
	KheTraceEnd(t);
	KheTraceDelete(t);
	if( DEBUG5 )
	{
	  fprintf(stderr, "  ] KheMeetKempeMove ret. false: cannot move ");
	  KheMeetDebug(meet2, 1, -1, stderr);
	  fprintf(stderr, "\n");
	}
	if( DEBUG6 )
	{
	  fprintf(stderr, "  (Kempe failed, ");
	  KheMeetDebug(meet2, 1, -1, stderr);
	  fprintf(stderr, " did not move)\n");
	}
	return false;
      }
      if( KheMeetNode(meet2) != NULL )
	*demand += KheNodeDemand(KheMeetNode(meet2));
      if( DEBUG5 || DEBUG6 )
      {
	fprintf(stderr, "    %s ", step % 2 == 1 ? "->" : "<-");
	KheMeetDebug(meet2, 1, -1, stderr);
	fprintf(stderr, " durn %d to ", KheMeetDuration(meet2));
	KheMeetDebug(target[step % 2], 1, -1, stderr);
	fprintf(stderr, "+%d (%d trace monitors)\n", offset[step % 2],
	  KheTraceMonitorCount(t));
      }
    }

    ** handle all preassigned demand monitors that increased in cost **
    for( i = 0;  i < KheTraceMonitorCount(t);  i++ )
    {
      m = KheTraceMonitor(t, i);
      if( KheMonitorTag(m) == KHE_GROUP_MONITOR_TAG &&
	  KheMonitorCost(m) > KheTraceMonitorInitCost(t, i) )
      {
	gm = (KHE_GROUP_MONITOR) m;
	for( j = 0;  j < KheGroupMonitorDefectCount(gm);  j++ )
	{
	  m = KheGroupMonitorDefect(gm, j);
	  if( KheMonitorTag(m) == KHE_ORDINARY_DEMAND_MONITOR_TAG )
	  {
	    dm = (KHE_ORDINARY_DEMAND_MONITOR) m;
	    task = KheOrdinaryDemandMonitorTask(dm);
	    if( KheTaskIsPreassigned(task, &junk) &&
		!KheGetCompetitorMeets(dm, target[step % 2],
		    &meets, first_unmoved) )
	    {
	      MArrayFree(meets);
	      KheTraceEnd(t);
	      KheTraceDelete(t);
	      if( DEBUG5 )
		fprintf(stderr, "  ] KheMeetKempeMove ret. false: dm\n");
	      return false;
	    }
	  }
	}
      }
    }
  }

  ** everything has moved and no more problems, so success **
  MArrayFree(meets);
  KheTraceEnd(t);
  KheTraceDelete(t);
  if( DEBUG5 )
    fprintf(stderr, "  ] KheMeetKempeMove ret true\n");
  return true;
}
*** */


/*****************************************************************************/
/*                                                                           */
/*  bool KheMeetKempeMoveTimeWithDemand(KHE_MEET meet, KHE_TIME t,           */
/*    int *demand)                                                           */
/*                                                                           */
/*  Make a Kempe move which moves meet to t, and set *demand to the total    */
/*  demand of the nodes of the meets that were moved.                        */
/*                                                                           */
/*****************************************************************************/

/* ***
bool KheMeetKempeMoveTimeWithDemand(KHE_MEET meet, KHE_TIME t, int *demand)
{
  KHE_SOLN soln;  KHE_MEET target_meet;  int target_offset;
  soln = KheMeetSoln(meet);
  target_meet = KheSolnTimeCycleMeet(soln, t);
  target_offset = KheSolnTimeCycleMeetOffset(soln, t);
  return KheMeetKempeMoveWithDemand(meet, target_meet, target_offset, demand);
}
*** */


/*****************************************************************************/
/*                                                                           */
/*  Submodule "repairing time assignments"                                   */
/*                                                                           */
/*****************************************************************************/

/*****************************************************************************/
/*                                                                           */
/*  bool KheAllLayersAllAssigned(KHE_NODE parent_node)                       */
/*                                                                           */
/*  Return true if all parent_node's children's meets are assigned.          */
/*                                                                           */
/*****************************************************************************/

static bool KheAllLayersAllAssigned(KHE_NODE parent_node)
{
  int i, j;  KHE_NODE n;  KHE_MEET meet;
  for( i = 0;  i < KheNodeChildCount(parent_node);  i++ )
  {
    n = KheNodeChild(parent_node, i);
    for( j = 0;  j < KheNodeMeetCount(n);  j++ )
    {
      meet = KheNodeMeet(n, j);
      if( KheMeetAsst(meet) == NULL )
	return false;
    }
  }
  return true;
}


/*****************************************************************************/
/*                                                                           */
/*  void KheNodeMeetSwapRepairTimes(KHE_NODE parent_node)                    */
/*                                                                           */
/*  Use node meet swaps to repair the time assignments of the meets of       */
/*  the children of parent_node.                                             */
/*                                                                           */
/*****************************************************************************/

bool KheNodeMeetSwapRepairTimes(KHE_NODE parent_node)
{
  bool res, add_layers;  KHE_GROUP_MONITOR gm, dgm;  KHE_EJECTOR ej;
  KHE_SOLN soln; int i;  KHE_NODE child_node;
  soln = KheNodeSoln(parent_node);
  if( DEBUG8 )
  {
    fprintf(stderr, "[ KheNodeMeetSwapRepairTimes(");
    KheNodeDebug(parent_node, 1, -1, stderr);
    fprintf(stderr, "), soln cost %.4f\n", KheCostShow(KheSolnCost(soln)));
    KheSolnCostByTypeDebug(soln, 2, 2, stderr);
  }

  /* build the group monitors and layers required by this algorithm */
  gm = KheNodeGroupMonitorsForNodeRepair(parent_node, NULL,
    KHE_SUBTAG_UNPREASSIGNED_DEMAND, "UnpreassignedDemandGroupMonitor",
    KHE_SUBTAG_NODE, "NodeGroupMonitor", (KHE_GROUP_MONITOR) soln);
  dgm = KheNodeGroupDemandMonitors(parent_node, NULL, true, false,
    KHE_SUBTAG_PREASSIGNED_DEMAND, "PreassignedDemandGroupMonitor",
    (KHE_GROUP_MONITOR) soln);
  add_layers = (KheNodeChildLayerCount(parent_node) == 0);
  if( add_layers )
    KheNodeChildLayersMake(parent_node);

  /* sort the meets of the child nodes */
  for( i = 0;  i < KheNodeChildCount(parent_node);  i++ )
  {
    child_node = KheNodeChild(parent_node, i);
    KheNodeMeetSort(child_node, &KheMeetDecreasingDurationCmp);
  }

  if( DEBUG8 )
    KheGroupMonitorDefectDebug(gm, 2, 2, stderr);

  /* set up the ejector and run the algorithm */
  ej = KheEjectorMake(soln);
  KheEjectorAddSchedule(ej, 1, INT_MAX, false);
  KheEjectorAddSchedule(ej, 2, INT_MAX, false);
  KheEjectorAddSchedule(ej, 3, INT_MAX, false);
  KheEjectorAddSchedule(ej, INT_MAX, INT_MAX, false);
  KheEjectorAddMonitorCostLimit(ej, (KHE_MONITOR) dgm,
    KheMonitorCost((KHE_MONITOR) dgm));
  KheEjectorAddGroupAugment(ej, KHE_SUBTAG_UNPREASSIGNED_DEMAND,
    &KheNodeDemandAugment);
  KheEjectorSolve(ej, KHE_EJECTOR_FIRST_SUCCESS, gm);
  KheEjectorDelete(ej);

  /* remove group monitors and layers and exit */
  if( add_layers )
    KheNodeChildLayersDelete(parent_node);
  KheUnGroupMonitors(gm);
  KheUnGroupMonitors(dgm);
  res = KheAllLayersAllAssigned(parent_node);
  if( DEBUG8 )
    fprintf(stderr, "] KheNodeMeetSwapRepairTimes ret %s, soln cost %.4f\n",
      res ? "true":"false", KheCostShow(KheSolnCost(KheNodeSoln(parent_node))));
  return res;
}


/*****************************************************************************/
/*                                                                           */
/*  bool KheEjectionChainRepairTimesOld(KHE_NODE parent_node)                */
/*                                                                           */
/*  Use an ejection chain to repair the time assignments of the meets of     */
/*  the descendants of parent_node.  Return true if all these meets are      */
/*  assigned on return.                                                      */
/*                                                                           */
/*****************************************************************************/

bool KheEjectionChainRepairTimesOld(KHE_NODE parent_node)
{
  KHE_EJECTOR ej;  KHE_GROUP_MONITOR gm;  bool res;  KHE_SOLN soln;
  if( DEBUG4 )
  {
    fprintf(stderr, "[ KheEjectionChainRepairTimesOld(parent_node)\n");
    if( KheNodeMeetCount(parent_node) > 0 &&
	!KheMeetIsCycleMeet(KheNodeMeet(parent_node, 0)) )
    {
      fprintf(stderr, "  timetable before:\n");
      KheNodePrintTimetable(parent_node, 15, 4, stderr);
    }
  }

  /* build the group monitors required by this algorithm */
  soln = KheNodeSoln(parent_node);
  gm = KheNodeGroupMonitorsForMeetRepair(parent_node, NULL,
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
  KheEjectorAddGroupAugment(ej, KHE_SUBTAG_EVENT, &KheEventAugment);
  KheEjectorAddGroupAugment(ej, KHE_SUBTAG_PREASSIGNED_DEMAND,
    &KheDemandAugment);
  KheEjectorAddGroupAugment(ej, KHE_SUBTAG_UNPREASSIGNED_DEMAND,
    &KheDemandAugment);
  KheEjectorSolve(ej, KHE_EJECTOR_FIRST_SUCCESS, gm);
  KheEjectorDelete(ej);

  /* remove group monitors and exit */
  KheUnGroupMonitors(gm);
  res = KheAllLayersAllAssigned(parent_node);
  if( DEBUG4 )
  {
    if( KheNodeMeetCount(parent_node) > 0 &&
	!KheMeetIsCycleMeet(KheNodeMeet(parent_node, 0)) )
    {
      fprintf(stderr, "  timetable after:\n");
      KheNodePrintTimetable(parent_node, 15, 4, stderr);
    }
    fprintf(stderr, "] KheEjectionChainRepairTimesOld returning %s\n",
      res ? "true" : "false");
  }
  return res;
}


/*****************************************************************************/
/*                                                                           */
/*  bool KheEjectionChainLongRepairTimes(KHE_NODE parent_node)               */
/*                                                                           */
/*  Use an extended ejection chain to repair the time assignments of the     */
/*  meets of the descendants of parent_node.  Return true if all these       */
/*  meets are assigned on return.                                            */
/*                                                                           */
/*****************************************************************************/

bool KheEjectionChainLongRepairTimes(KHE_NODE parent_node)
{
  KHE_EJECTOR ej;  KHE_GROUP_MONITOR gm;  bool res;  KHE_SOLN soln;
  KHE_EJECTOR_SOLVE_TYPE solve_type = KHE_EJECTOR_MIN_COST;
  if( DEBUG4 )
  {
    fprintf(stderr, "[ KheEjectionChainLongRepairTimes(parent_node)\n");
    if( KheNodeMeetCount(parent_node) > 0 &&
	!KheMeetIsCycleMeet(KheNodeMeet(parent_node, 0)) )
    {
      fprintf(stderr, "  timetable before:\n");
      KheNodePrintTimetable(parent_node, 15, 4, stderr);
    }
  }

  /* build the group monitors required by this algorithm */
  soln = KheNodeSoln(parent_node);
  gm = KheNodeGroupMonitorsForMeetRepair(parent_node, NULL,
    KHE_SUBTAG_EVENT, "EventGroupMonitor",
    KHE_SUBTAG_PREASSIGNED_DEMAND, "PreassignedDemandGroupMonitor",
    KHE_SUBTAG_UNPREASSIGNED_DEMAND, "UnpreassignedDemandGroupMonitor",
    KHE_SUBTAG_NODE, "NodeGroupMonitor", (KHE_GROUP_MONITOR) soln);

  /* set up the ejector and run the algorithm */
  ej = KheEjectorMake(soln);
  KheEjectorAddSchedule(ej, INT_MAX, 60, false);
  KheEjectorAddGroupAugment(ej, KHE_SUBTAG_EVENT, &KheEventAugment);
  KheEjectorAddGroupAugment(ej, KHE_SUBTAG_PREASSIGNED_DEMAND,
    &KheDemandAugment);
  KheEjectorAddGroupAugment(ej, KHE_SUBTAG_UNPREASSIGNED_DEMAND,
    &KheDemandAugment);
  KheEjectorSolve(ej, solve_type, gm);
  KheEjectorDelete(ej);

  /* remove group monitors and exit */
  KheUnGroupMonitors(gm);
  res = KheAllLayersAllAssigned(parent_node);
  if( DEBUG4 )
  {
    if( KheNodeMeetCount(parent_node) > 0 &&
	!KheMeetIsCycleMeet(KheNodeMeet(parent_node, 0)) )
    {
      fprintf(stderr, "  timetable after:\n");
      KheNodePrintTimetable(parent_node, 15, 4, stderr);
    }
    fprintf(stderr, "] KheEjectionChainLongRepairTimes returning %s\n",
      res ? "true" : "false");
  }
  return res;
}


/*****************************************************************************/
/*                                                                           */
/*  Submodule "putting it all together"                                      */
/*                                                                           */
/*****************************************************************************/

/*****************************************************************************/
/*                                                                           */
/*  bool KheCycleNodeAssignTimes(KHE_NODE cycle_node)                        */
/*                                                                           */
/*  Assign times to the meets of all the proper descendants of cycle_node,   */
/*  assumed to be the cycle node, using the best available algorithm.        */
/*                                                                           */
/*****************************************************************************/

bool KheCycleNodeAssignTimes(KHE_NODE cycle_node)
{
  int i;  KHE_SOLN soln;  bool res;  KHE_TRANSACTION t;
  bool with_tightening = true;
  /* bool with_global_swaps = true; */
  /* bool with_simple_repair = false; */
  bool with_ejection_repair = true;
  /* bool with_extended_repair = false; */
  bool with_vizier_node = false;

  if( DEBUG3 )
    fprintf(stderr, "[ KheCycleNodeAssignTimes(Node %d)\n",
      KheNodeIndex(cycle_node));

  /* coordinate layers and build runarounds */
  soln = KheNodeSoln(cycle_node);
  KheCoordinateLayers(cycle_node, true);
  KheBuildRunarounds(cycle_node, &KheNodeSimpleAssignTimes,
    &KheRunaroundNodeAssignTimes);
  for( i = 0;  i < KheNodeChildCount(cycle_node);  i++ )
    KheNodeRecursiveAssignTimes(KheNodeChild(cycle_node, i),
      &KheRunaroundNodeAssignTimes);

  /* assign preassigned meets */
  KheNodePreassignedAssignTimes(cycle_node);

  /* tighten resource domains */
  if( with_tightening )
  {
    t = KheTransactionMake(soln);
    KheTransactionBegin(t);
    for( i = 0;  i < KheSolnTaskingCount(soln);  i++ )
      KheTaskingTightenToPartition(KheSolnTasking(soln, i));
    KheTransactionEnd(t);
  }

  /* time assignment and repair */
  if( with_vizier_node )
  {
    KHE_NODE vizier_node = KheNodeInsertVizierNode(cycle_node);
    KheNodeLayeredAssignTimes(vizier_node, true, true);
    res = with_ejection_repair ? KheEjectionChainRepairTimes(cycle_node) : true;
    KheNodeFlatten(vizier_node);
    KheNodeDeleteZones(vizier_node);
    /* ***
    res = with_simple_repair ?
      KheSimpleRepairTimes(vizier_node, KheCost(1, 0)) : true;
    *** */
    res = with_ejection_repair ? KheEjectionChainRepairTimes(vizier_node):true;
    /* ***
    res = with_extended_repair ?
      KheEjectionChainLongRepairTimes(vizier_node) : true;
    *** */
    KheNodeRemoveVizierNode(vizier_node);
  }
  else
  {
    KheNodeLayeredAssignTimes(cycle_node, false, true);
    res = with_ejection_repair ? KheEjectionChainRepairTimes(cycle_node) : true;
    KheNodeFlatten(cycle_node);
    KheNodeDeleteZones(cycle_node);
    res = with_ejection_repair ? KheEjectionChainRepairTimes(cycle_node) : true;
  }

  /* undo resource domain tightening */
  if( with_tightening )
  {
    KheTransactionUndo(t);
    KheTransactionDelete(t);
  }
  
  /* all done */
  if( DEBUG3 )
    fprintf(stderr, "] KheCycleNodeAssignTimes returning\n");
  return res;
}
