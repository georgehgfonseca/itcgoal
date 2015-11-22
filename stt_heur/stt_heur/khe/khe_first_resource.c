
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
/*  FILE:         khe_first_resource.c                                       */
/*  DESCRIPTION:  Most-constrained-first resource assignment solver          */
/*                                                                           */
/*****************************************************************************/
#include "khe.h"
#include "m.h"
#include "khe_priqueue.h"
#include <limits.h>

#define DEBUG1 0

/*****************************************************************************/
/*                                                                           */
/*  KHE_TASK_GROUP_NODE - a node representing one task group                 */
/*  KHE_RESOURCE_NODE - a node representing one resource.                    */
/*                                                                           */
/*****************************************************************************/

typedef struct khe_task_group_node_rec *KHE_TASK_GROUP_NODE;
typedef MARRAY(KHE_TASK_GROUP_NODE) ARRAY_KHE_TASK_GROUP_NODE;

typedef struct khe_resource_node_rec *KHE_RESOURCE_NODE;
typedef MARRAY(KHE_RESOURCE_NODE) ARRAY_KHE_RESOURCE_NODE;

struct khe_task_group_node_rec {
  int				priqueue_index;		/* for priqueue      */
  KHE_TASK_GROUP		task_group;		/* tasks to do       */
  int				unassigned_tasks;	/* no unassigned     */
  ARRAY_KHE_RESOURCE_NODE	resource_nodes;		/* avail resources   */
};

struct khe_resource_node_rec {
  KHE_RESOURCE			resource;		/* the resource      */
  ARRAY_KHE_TASK_GROUP_NODE	task_group_nodes;	/* suitable tasks    */
};


/*****************************************************************************/
/*                                                                           */
/*  Submodule "task group nodes" (private)                                   */
/*                                                                           */
/*****************************************************************************/

/*****************************************************************************/
/*                                                                           */
/*  KHE_TASK_GROUP_NODE KheTaskGroupNodeMake(KHE_TASK_GROUP task_group,      */
/*    int unassigned_tasks)                                                  */
/*                                                                           */
/*  Create a new task set node with these attributes.                        */
/*                                                                           */
/*****************************************************************************/

static KHE_TASK_GROUP_NODE KheTaskGroupNodeMake(KHE_TASK_GROUP task_group,
  int unassigned_tasks)
{
  KHE_TASK_GROUP_NODE res;
  MMake(res);
  res->priqueue_index = 0;  /* actually undefined */
  res->task_group = task_group;
  res->unassigned_tasks = unassigned_tasks;
  MArrayInit(res->resource_nodes);
  return res;
}


/*****************************************************************************/
/*                                                                           */
/*  void KheTaskGroupNodeDelete(KHE_TASK_GROUP_NODE tgn)                     */
/*                                                                           */
/*  Delete tgn.                                                              */
/*                                                                           */
/*****************************************************************************/

static void KheTaskGroupNodeDelete(KHE_TASK_GROUP_NODE tgn)
{
  MArrayFree(tgn->resource_nodes);
  MFree(tgn);
}


/*****************************************************************************/
/*                                                                           */
/*  int64_t KheTaskGroupNodeKey(void *tgn)                                   */
/*                                                                           */
/*  PriQueue callback function which returns the priority of tgn.            */
/*                                                                           */
/*****************************************************************************/

static int64_t KheTaskGroupNodeKey(void *tgn)
{
  return MArraySize(((KHE_TASK_GROUP_NODE) tgn)->resource_nodes) -
    ((KHE_TASK_GROUP_NODE) tgn)->unassigned_tasks;
}


/*****************************************************************************/
/*                                                                           */
/*  int KheTaskGroupNodeIndex(void *tgn)                                     */
/*                                                                           */
/*  PriQueue callback function which returns the index of tgn.               */
/*                                                                           */
/*****************************************************************************/

static int KheTaskGroupNodeIndex(void *tgn)
{
  return ((KHE_TASK_GROUP_NODE) tgn)->priqueue_index;
}


/*****************************************************************************/
/*                                                                           */
/*  void KheTaskGroupNodeSetIndex(void *tgn, int index)                      */
/*                                                                           */
/*  PriQueue callback function which sets the index of tgn.                  */
/*                                                                           */
/*****************************************************************************/

static void KheTaskGroupNodeSetIndex(void *tgn, int index)
{
  ((KHE_TASK_GROUP_NODE) tgn)->priqueue_index = index;
}


/*****************************************************************************/
/*                                                                           */
/*  void KheTaskGroupNodeDebug(KHE_TASK_GROUP_NODE tgn, int verbosity,       */
/*    int indent, FILE *fp)                                                  */
/*                                                                           */
/*  Debug print of tgn onto fp with the given verbosity and indent.          */
/*                                                                           */
/*****************************************************************************/

static void KheTaskGroupNodeDebug(KHE_TASK_GROUP_NODE tgn, int verbosity,
  int indent, FILE *fp)
{
  if( verbosity >= 1 )
  {
    if( indent >= 0 )
      fprintf(fp, "%*s", indent, "");
    fprintf(fp, "[ TaskGroupNode %ld (%d resources, %d unassigned tasks)",
      KheTaskGroupNodeKey(tgn), MArraySize(tgn->resource_nodes),
      tgn->unassigned_tasks);
    if( indent >= 0 )
    {
      fprintf(fp, ":\n");
      KheTaskGroupDebug(tgn->task_group, 2, 4, stderr);
      fprintf(fp, "%*s]\n", indent, "");
    }
    else
      fprintf(fp, " ]");
  }
}


/*****************************************************************************/
/*                                                                           */
/*  Submodule "resource nodes" (private)                                     */
/*                                                                           */
/*****************************************************************************/

/*****************************************************************************/
/*                                                                           */
/*  KHE_RESOURCE_NODE KheResourceNodeMake(KHE_RESOURCE r)                    */
/*                                                                           */
/*  Make a resource node for r.                                              */
/*                                                                           */
/*****************************************************************************/

static KHE_RESOURCE_NODE KheResourceNodeMake(KHE_RESOURCE r)
{
  KHE_RESOURCE_NODE res;
  MMake(res);
  res->resource = r;
  MArrayInit(res->task_group_nodes);
  return res;
}


/*****************************************************************************/
/*                                                                           */
/*  void KheResourceNodeDelete(KHE_RESOURCE_NODE rn)                         */
/*                                                                           */
/*  Delete rn.                                                               */
/*                                                                           */
/*****************************************************************************/

static void KheResourceNodeDelete(KHE_RESOURCE_NODE rn)
{
  MArrayFree(rn->task_group_nodes);
  MFree(rn);
}


/*****************************************************************************/
/*                                                                           */
/*  void KheAddEdge(KHE_TASK_GROUP_NODE tgn, KHE_RESOURCE_NODE rn)           */
/*                                                                           */
/*  Add an edge from tgn to rn.                                              */
/*                                                                           */
/*****************************************************************************/

static void KheAddEdge(KHE_TASK_GROUP_NODE tgn, KHE_RESOURCE_NODE rn)
{
  MArrayAddLast(tgn->resource_nodes, rn);
  MArrayAddLast(rn->task_group_nodes, tgn);
}


/*****************************************************************************/
/*                                                                           */
/*  void KheDeleteEdge(KHE_TASK_GROUP_NODE tgn, KHE_RESOURCE_NODE rn)        */
/*                                                                           */
/*  Delete the edge from tgn to rn.                                          */
/*                                                                           */
/*****************************************************************************/

static void KheDeleteEdge(KHE_TASK_GROUP_NODE tgn, KHE_RESOURCE_NODE rn)
{
  int pos;
  if( !MArrayContains(tgn->resource_nodes, rn, &pos) )
    MAssert(false, "KheDeleteEdge internal error 1");
  MArrayRemove(tgn->resource_nodes, pos);
  if( !MArrayContains(rn->task_group_nodes, tgn, &pos) )
    MAssert(false, "KheDeleteEdge internal error 2");
  MArrayRemove(rn->task_group_nodes, pos);
}


/*****************************************************************************/
/*                                                                           */
/*  bool KheEdgeAssignable(KHE_TASK_GROUP_NODE tgn, KHE_RESOURCE_NODE rn,    */
/*    KHE_SOLN soln, KHE_COST *cost)                                         */
/*                                                                           */
/*  If rn can be assigned to tgn without increasing the solution cost or     */
/*  the number of unassignable tixels in soln's matching, return true with   */
/*  *cost set to the solution cost after the assignment (but don't leave     */
/*  the assignment there).  Otherwise return false.                          */
/*                                                                           */
/*****************************************************************************/

static bool KheEdgeAssignable(KHE_TASK_GROUP_NODE tgn, KHE_RESOURCE_NODE rn,
  KHE_SOLN soln, KHE_COST *cost)
{
  int unmatched_before, unmatched_after;  bool res;  KHE_COST cost_before;
  KheSolnMatchingMarkBegin(soln);
  cost_before = KheSolnCost(soln);
  unmatched_before = KheSolnMatchingDefectCount(soln);
  if( KheTaskGroupAssign(tgn->task_group, rn->resource) )
  {
    *cost = KheSolnCost(soln);
    unmatched_after = KheSolnMatchingDefectCount(soln);
    KheTaskGroupUnAssign(tgn->task_group, rn->resource);
    res = unmatched_after <= unmatched_before && *cost < cost_before;
  }
  else
    res = false;
  KheSolnMatchingMarkEnd(soln, true);
  return res;
}


/*****************************************************************************/
/*                                                                           */
/*  bool KheTaskWantsResource(KHE_TASK task, KHE_RESOURCE r)                 */
/*                                                                           */
/*  Return true if task wants r, because one of its descendants is derived   */
/*  from an event resource which has a task which is already assigned r.     */
/*                                                                           */
/*****************************************************************************/

static bool KheTaskWantsResource(KHE_TASK task, KHE_RESOURCE r)
{
  int i;  KHE_EVENT_RESOURCE er;  KHE_SOLN soln;

  /* see whether task itself wants r */
  soln = KheTaskSoln(task);
  er = KheTaskEventResource(task);
  if( er != NULL )
    for( i = 0;  i < KheEventResourceTaskCount(soln, er);  i++ )
      if( KheTaskAsstResource(KheEventResourceTask(soln, er, i)) == r )
	return true;

  /* see whether any tasks assigned to task want r */
  for( i = 0;  i < KheTaskAssignedToCount(task);  i++ )
    if( KheTaskWantsResource(KheTaskAssignedTo(task, i), r) )
      return true;
  return false;
}


/*****************************************************************************/
/*                                                                           */
/*  Submodule "most constraint first resource assignment"                    */
/*                                                                           */
/*****************************************************************************/

/*****************************************************************************/
/*                                                                           */
/*  void KheMostConstrainedFirstAssignResources(KHE_TASKING tasking)         */
/*                                                                           */
/*  Assign resources for tasking.                                            */
/*                                                                           */
/*****************************************************************************/

void KheMostConstrainedFirstAssignResources(KHE_TASKING tasking)
{
  ARRAY_KHE_TASK_GROUP_NODE task_group_nodes;  KHE_TASK_GROUP_NODE tgn, tgn2;
  ARRAY_KHE_RESOURCE_NODE resource_nodes, best_resource_nodes;
  KHE_RESOURCE_NODE rn;  KHE_PRIQUEUE task_queue;  KHE_SOLN soln;
  KHE_INSTANCE ins;  int i, j, index, unassigned_tasks;
  KHE_COST best_cost, cost;
  KHE_TASK_GROUP task_group;  KHE_RESOURCE r;  KHE_RESOURCE_GROUP rg;
  KHE_TASK task;  KHE_TASK_GROUPS task_groups;
  if( DEBUG1 )
  {
    fprintf(stderr, "[ KheMostConstrainedFirstAssignResources(tasking):\n");
    KheTaskingDebug(tasking, 2, 2, stderr);
  }

  /* make task groups */
  soln = KheTaskingSoln(tasking);
  ins = KheSolnInstance(soln);
  task_groups = KheTaskGroupsMakeFromTasking(tasking);

  /* build one task set node for each incompletely assigned task group */
  MArrayInit(task_group_nodes);
  for( i = 0;  i < KheTaskGroupsTaskGroupCount(task_groups);  i++ )
  {
    task_group = KheTaskGroupsTaskGroup(task_groups, i);
    unassigned_tasks = KheTaskGroupUnassignedTaskCount(task_group);
    if( unassigned_tasks > 0 )
    {
      tgn = KheTaskGroupNodeMake(task_group, unassigned_tasks);
      MArrayAddLast(task_group_nodes, tgn);
    }
  }

  /* build one resource node for each resource in any domain, plus edges */
  MArrayInit(resource_nodes);
  MArrayFill(resource_nodes, KheInstanceResourceCount(ins), NULL);
  MArrayForEach(task_group_nodes, &tgn, &i)
  {
    rg = KheTaskGroupDomain(tgn->task_group);
    for( j = 0;  j < KheResourceGroupResourceCount(rg);  j++ )
    {
      r = KheResourceGroupResource(rg, j);
      index = KheResourceIndexInInstance(r);
      if( MArrayGet(resource_nodes, index) == NULL )
	MArrayPut(resource_nodes, index, KheResourceNodeMake(r));
      rn = MArrayGet(resource_nodes, index);
      if( KheEdgeAssignable(tgn, rn, soln, &cost) )
	KheAddEdge(tgn, rn);
    }
  }

  /* create and populate the priority queue */
  task_queue = KhePriQueueMake(&KheTaskGroupNodeKey, &KheTaskGroupNodeIndex,
    &KheTaskGroupNodeSetIndex);
  MArrayForEach(task_group_nodes, &tgn, &i)
    KhePriQueueInsert(task_queue, tgn);

  /* do the assigning */
  MArrayInit(best_resource_nodes);
  while( !KhePriQueueEmpty(task_queue) )
  {
    /* find the task set node that needs assigning next */
    tgn = (KHE_TASK_GROUP_NODE) KhePriQueueDeleteMin(task_queue);
    if( DEBUG1 )
      KheTaskGroupNodeDebug(tgn, 2, 2, stderr);

    /* find the best resources to assign to it */
    MArrayClear(best_resource_nodes);
    best_cost = KheCostMax;
    MArrayForEach(tgn->resource_nodes, &rn, &i)
      if( !KheEdgeAssignable(tgn, rn, soln, &cost) )
      {
	/* rn has gone bad, remove it permanently */
        KheDeleteEdge(tgn, rn);
	i--;
      }
      else if( cost <= best_cost )
      {
	/* rn is one of the best available */
	if( cost < best_cost )
	{
	  best_cost = cost;
	  MArrayClear(best_resource_nodes);
	}
	MArrayAddLast(best_resource_nodes, rn);
      }

    /* if there is a suitable resource, assign one (else do nothing) */
    if( MArraySize(best_resource_nodes) > 0 )
    {
      /* find task, an unassigned task of tgn */
      task = NULL;  /* keep compiler happy */
      for( i = 0;  i < KheTaskGroupTaskCount(tgn->task_group);  i++ )
      {
	task = KheTaskGroupTask(tgn->task_group, i);
	if( KheTaskAsst(task) == NULL )
	  break;
      }
      MAssert(i < KheTaskGroupTaskCount(tgn->task_group),
        "KheMostConstrainedFirstAssignResources internal error 1");

      /* find rn, the best of best_resource_nodes to assign to task */
      MArrayForEach(best_resource_nodes, &rn, &i)
	if( KheTaskWantsResource(task, rn->resource) )
	  break;
      if( i >= MArraySize(best_resource_nodes) )
	rn = MArrayFirst(best_resource_nodes);

      /* assign rn's resource to task */
      if( DEBUG1 )
      {
	fprintf(stderr, "    assigning ");
	KheResourceDebug(rn->resource, 1, -1, stderr);
	fprintf(stderr, " to ");
	KheTaskDebug(task, 1, 0, stderr);
      }
      if( !KheTaskAssignResource(task, rn->resource) )
	MAssert(false, "KheMostConstrainedFirstAssignResources internal err 2");
      KheDeleteEdge(tgn, rn);

      /* if tgn still has unassigned tasks, reinsert, else delete all edges */
      tgn->unassigned_tasks--;
      if( tgn->unassigned_tasks > 0 )
	KhePriQueueInsert(task_queue, tgn);
      else
      {
	while( MArraySize(tgn->resource_nodes) > 0 )
          KheDeleteEdge(tgn, MArrayFirst(tgn->resource_nodes));
      }

      /* update rn's availability over all its edges */
      MArrayForEach(rn->task_group_nodes, &tgn2, &i)
      {
	MAssert(tgn2 != tgn,
	  "KheMostConstrainedFirstAssignResources internal error 3");
	if( !KheEdgeAssignable(tgn2, rn, soln, &cost) )
	{
	  KheDeleteEdge(tgn2, rn);
	  KhePriQueueNotifyKeyChange(task_queue, tgn2);
	  i--;
	}
      }
    }
  }

  /* free memory */
  KhePriQueueDelete(task_queue);
  MArrayFree(best_resource_nodes);
  MArrayForEach(task_group_nodes, &tgn, &i)
    KheTaskGroupNodeDelete(tgn);
  MArrayFree(task_group_nodes);
  MArrayForEach(resource_nodes, &rn, &i)
    if( rn != NULL )
      KheResourceNodeDelete(rn);

  if( DEBUG1 )
    fprintf(stderr, "] KheMostConstrainedFirstAssignResources returning\n");
}
