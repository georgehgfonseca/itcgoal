
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
/*  FILE:         khe_split_resource.c                                       */
/*  DESCRIPTION:  KheFindSplitResourceAssignments()                          */
/*                                                                           */
/*****************************************************************************/
#include "khe.h"
#include "m.h"
#include "khe_priqueue.h"
#include <limits.h>

#define DEBUG1 0
#define DEBUG2 0
#define DEBUG3 0
#define DEBUG4 0
#define DEBUG5 0
#define DEBUG6 0
#define DEBUG7 0
#define DEBUG8 0

typedef MARRAY(KHE_TASK) ARRAY_KHE_TASK;

typedef struct khe_resource_node_rec *KHE_RESOURCE_NODE;
typedef MARRAY(KHE_RESOURCE_NODE) ARRAY_KHE_RESOURCE_NODE;

typedef struct khe_task_node_rec *KHE_TASK_NODE;
typedef MARRAY(KHE_TASK_NODE) ARRAY_KHE_TASK_NODE;

typedef struct khe_task_set_rec *KHE_TASK_SET;
typedef MARRAY(KHE_TASK_SET) ARRAY_KHE_TASK_SET;

typedef struct khe_split_solver_rec *KHE_SPLIT_SOLVER;

/*****************************************************************************/
/*                                                                           */
/*  KHE_RESOURCE_NODE                                                        */
/*                                                                           */
/*  This type represents one resource within this solver.  It remembers      */
/*  which task nodes the resource is assignable to at any moment.  There     */
/*  are also temporary fields used when assigning the resource.              */
/*                                                                           */
/*****************************************************************************/

struct khe_resource_node_rec {
  KHE_SPLIT_SOLVER	split_solver;		/* enclosing split solver    */
  KHE_RESOURCE		resource;		/* the resource represented  */
  ARRAY_KHE_TASK_NODE	task_nodes;		/* tasks resource avail for  */
};


/*****************************************************************************/
/*                                                                           */
/*  KHE_TASK_NODE                                                            */
/*                                                                           */
/*  This type represents one task (not the original combined task that       */
/*  needed to be split, but rather one of its fragments).  It remembers      */
/*  which resource nodes the task is assignable by at any moment.            */
/*                                                                           */
/*****************************************************************************/

struct khe_task_node_rec {
  KHE_TASK_SET			task_set;	/* enslosing task set        */
  KHE_TASK			task;		/* the task represented      */
  int				index;		/* index in priqueue         */
  ARRAY_KHE_RESOURCE_NODE	resource_nodes;	/* resources able to assign  */
  KHE_TASK_NODE			parent_node;	/* when clustering           */
};


/*****************************************************************************/
/*                                                                           */
/*  KHE_TASK_SET                                                             */
/*                                                                           */
/*  This type represents one of the original combined tasks that could       */
/*  not be assigned as a whole and so needs to be split.                     */
/*                                                                           */
/*****************************************************************************/

struct khe_task_set_rec {
  KHE_SPLIT_SOLVER		split_solver;	/* enclosing split solver    */
  ARRAY_KHE_TASK_NODE		task_nodes;	/* the set's members         */
  int				unassigned_count;  /* no of unassigned tn's  */
  ARRAY_KHE_RESOURCE_NODE	resource_nodes; /* initially available       */
  ARRAY_KHE_RESOURCE_NODE	used_resource_nodes; /* actually used        */
};


/*****************************************************************************/
/*                                                                           */
/*  KHE_SPLIT_SOLVER                                                         */
/*                                                                           */
/*  This type holds all the task sets needing split assignments and all      */
/*  the resource nodes available to be assigned to them.                     */
/*                                                                           */
/*****************************************************************************/

struct khe_split_solver_rec {
  KHE_TASKING			tasking;		/* original tasking  */
  ARRAY_KHE_RESOURCE_NODE	resource_nodes;		/* resource nodes    */
  ARRAY_KHE_TASK_SET		task_sets;		/* task sets         */
  KHE_PRIQUEUE			forced_task_nodes;	/* forced task nodes */
};


/*****************************************************************************/
/*                                                                           */
/*  Submodule "resource nodes"                                               */
/*                                                                           */
/*****************************************************************************/

/*****************************************************************************/
/*                                                                           */
/*  KHE_RESOURCE_NODE KheResourceNodeMake(KHE_SPLIT_SOLVER ss,               */
/*    KHE_RESOURCE r)                                                        */
/*                                                                           */
/*  Make a resource node with these attributes.                              */
/*                                                                           */
/*****************************************************************************/

static KHE_RESOURCE_NODE KheResourceNodeMake(KHE_SPLIT_SOLVER ss,
  KHE_RESOURCE r)
{
  KHE_RESOURCE_NODE res;
  MMake(res);
  res->split_solver = ss;
  res->resource = r;
  MArrayInit(res->task_nodes);
  return res;
}


/*****************************************************************************/
/*                                                                           */
/*  void KheResourceNodeDelete(KHE_RESOURCE_NODE rn)                         */
/*                                                                           */
/*  Delete rn.                                                               */
/*                                                                           */
/*****************************************************************************/
static void KheTaskNodeDeleteResourceNode(KHE_TASK_NODE tn,
  KHE_RESOURCE_NODE rn);
static void KheSplitSolverDeleteResourceNode(KHE_SPLIT_SOLVER ss,
  KHE_RESOURCE_NODE rn);

void KheResourceNodeDelete(KHE_RESOURCE_NODE rn)
{
  KHE_TASK_NODE tn;  int i;
  KheSplitSolverDeleteResourceNode(rn->split_solver, rn);
  MArrayForEach(rn->task_nodes, &tn, &i)
    KheTaskNodeDeleteResourceNode(tn, rn);
  MArrayFree(rn->task_nodes);
  MFree(rn);
}


/*****************************************************************************/
/*                                                                           */
/*  void KheResourceNodeAddTaskNode(KHE_RESOURCE_NODE rn, KHE_TASK_NODE tn)  */
/*                                                                           */
/*  Add tn to rn.                                                            */
/*                                                                           */
/*****************************************************************************/

static void KheResourceNodeAddTaskNode(KHE_RESOURCE_NODE rn, KHE_TASK_NODE tn)
{
  MArrayAddLast(rn->task_nodes, tn);
}


/*****************************************************************************/
/*                                                                           */
/*  void KheResourceNodeDeleteTaskNode(KHE_RESOURCE_NODE rn,                 */
/*    KHE_TASK_NODE tn)                                                      */
/*                                                                           */
/*  Delete tn from rn.                                                       */
/*                                                                           */
/*****************************************************************************/

static void KheResourceNodeDeleteTaskNode(KHE_RESOURCE_NODE rn,
  KHE_TASK_NODE tn)
{
  int pos;
  if( !MArrayContains(rn->task_nodes, tn, &pos) )
    MAssert(false, "KheResourceNodeDeleteTaskNode internal error");
  MArrayRemove(rn->task_nodes, pos);
}


/*****************************************************************************/
/*                                                                           */
/*  int KheResourceNodeIncreasingTaskNodesCmp(const void *t1, const void *t2)*/
/*                                                                           */
/*  Comparison function for sorting an array of resource nodes by            */
/*  increasing number of task nodes they are assignable to.  The point       */
/*  of this is that resources that are assignable to few task nodes need     */
/*  to be used up, so we put them first.                                     */
/*                                                                           */
/*****************************************************************************/

static int KheResourceNodeIncreasingTaskNodesCmp(const void *t1, const void *t2)
{
  KHE_RESOURCE_NODE rn1 = * (KHE_RESOURCE_NODE *) t1;
  KHE_RESOURCE_NODE rn2 = * (KHE_RESOURCE_NODE *) t2;
  if( MArraySize(rn1->task_nodes) != MArraySize(rn2->task_nodes) )
    return MArraySize(rn1->task_nodes) - MArraySize(rn2->task_nodes);
  else
    return KheResourceIndexInInstance(rn1->resource) -
      KheResourceIndexInInstance(rn2->resource);
}


/*****************************************************************************/
/*                                                                           */
/*  void KheResourceNodeDebug(KHE_RESOURCE_NODE rn, int verbosity,           */
/*    int indent, FILE *fp)                                                  */
/*                                                                           */
/*  Debug print of fn onto fp.                                               */
/*                                                                           */
/*****************************************************************************/

static void KheResourceNodeDebug(KHE_RESOURCE_NODE rn, int verbosity,
  int indent, FILE *fp)
{
  KHE_TASK_NODE tn;  int i;
  if( verbosity >= 1 && indent >= 0 )
  {
    fprintf(fp, "%*s[ Resource Node %s (%d task nodes)\n", indent, "",
      KheResourceId(rn->resource) == NULL ? "-" : KheResourceId(rn->resource),
      MArraySize(rn->task_nodes));
    if( verbosity >= 2 )
      MArrayForEach(rn->task_nodes, &tn, &i)
	KheTaskDebug(tn->task, verbosity, indent + 2, fp);
    fprintf(fp, "%*s]\n", indent, "");
  }
}


/*****************************************************************************/
/*                                                                           */
/*  Submodule "task nodes"                                                   */
/*                                                                           */
/*****************************************************************************/

/*****************************************************************************/
/*                                                                           */
/*  KHE_TASK_NODE KheTaskNodeMake(KHE_TASK_SET ts, KHE_TASK task)            */
/*                                                                           */
/*  Make a new task node with these attributes.                              */
/*                                                                           */
/*****************************************************************************/

static KHE_TASK_NODE KheTaskNodeMake(KHE_TASK_SET ts, KHE_TASK task)
{
  KHE_TASK_NODE res;
  MMake(res);
  res->task_set = ts;
  res->task = task;
  res->index = -1;  /* not in priqueue */
  MArrayInit(res->resource_nodes);
  res->parent_node = NULL;
  return res;
}


/*****************************************************************************/
/*                                                                           */
/*  void KheTaskNodeAddResourceNode(KHE_TASK_NODE tn, KHE_RESOURCE_NODE rn)  */
/*                                                                           */
/*  Add rn to tn.                                                            */
/*                                                                           */
/*****************************************************************************/

static void KheTaskNodeAddResourceNode(KHE_TASK_NODE tn, KHE_RESOURCE_NODE rn)
{
  MArrayAddLast(tn->resource_nodes, rn);
}


/*****************************************************************************/
/*                                                                           */
/*  void KheTaskNodeDeleteResourceNode(KHE_TASK_NODE tn,                     */
/*    KHE_RESOURCE_NODE rn)                                                  */
/*                                                                           */
/*  Delete rn from tn and handle priority queue stuff.                       */
/*                                                                           */
/*****************************************************************************/

static void KheTaskNodeDeleteResourceNode(KHE_TASK_NODE tn,
  KHE_RESOURCE_NODE rn)
{
  int pos;  KHE_SPLIT_SOLVER ss;
  if( !MArrayContains(tn->resource_nodes, rn, &pos) )
    MAssert(false, "KheTaskNodeDeleteResourceNode internal error");
  MArrayRemove(tn->resource_nodes, pos);
  ss = tn->task_set->split_solver;
  if( MArraySize(tn->resource_nodes) == 0 )
  {
    if( DEBUG3 )
    {
      fprintf(stderr, "  priqueue delete (duration %d) ",
	KheTaskDuration(tn->task));
      KheTaskDebug(tn->task, 1, 0, stderr);
    }
    KhePriQueueDeleteEntry(ss->forced_task_nodes, (void *) tn);
  }
  else if( MArraySize(tn->resource_nodes) == 1 )
  {
    if( DEBUG3 )
    {
      fprintf(stderr, "  priqueue insert (duration %d) ",
	KheTaskDuration(tn->task));
      KheTaskDebug(tn->task, 1, 0, stderr);
    }
    KhePriQueueInsert(ss->forced_task_nodes, (void *) tn);
  }
}


/*****************************************************************************/
/*                                                                           */
/*  void KheTaskNodeDelete(KHE_TASK_NODE tn)                                 */
/*                                                                           */
/*  Remove tn from the system.                                               */
/*                                                                           */
/*****************************************************************************/
static void KheTaskSetDeleteTaskNode(KHE_TASK_SET ts, KHE_TASK_NODE tn);

static void KheTaskNodeDelete(KHE_TASK_NODE tn)
{
  KHE_RESOURCE_NODE rn;  int i;
  MArrayForEach(tn->resource_nodes, &rn, &i)
    KheResourceNodeDeleteTaskNode(rn, tn);
  KheTaskSetDeleteTaskNode(tn->task_set, tn);
  MArrayFree(tn->resource_nodes);
  MFree(tn);
}


/*****************************************************************************/
/*                                                                           */
/*  bool KheTaskNodeComplete(KHE_TASK_NODE tn)                               */
/*                                                                           */
/*  Return true if tn is complete, i.e. if nothing further can be done       */
/*  with it.                                                                 */
/*                                                                           */
/*****************************************************************************/
static bool KheTaskNodeAssignable(KHE_TASK_NODE tn, KHE_RESOURCE_NODE rn);

static bool KheTaskNodeComplete(KHE_TASK_NODE tn)
{
  KHE_RESOURCE_NODE rn;  int i;

  /* if assigned, it's complete */
  if( KheTaskAsstResource(tn->task) != NULL )
  {
    MAssert(MArraySize(tn->resource_nodes) == 0,
      "KheTaskNodeComplete internal error");
    return true;
  }

  /* check assignability of all resources */
  MArrayForEach(tn->resource_nodes, &rn, &i)
    if( !KheTaskNodeAssignable(tn, rn) )
    {
      KheResourceNodeDeleteTaskNode(rn, tn);
      KheTaskNodeDeleteResourceNode(tn, rn);
      i--;
    }

  /* complete if no resources */
  return MArraySize(tn->resource_nodes) == 0;
}


/*****************************************************************************/
/*                                                                           */
/*  Priority queue callback functions                                        */
/*                                                                           */
/*****************************************************************************/

static KHE_COST KheTaskNodeKey(void *entry)
{
  return - KheTaskDuration(((KHE_TASK_NODE) entry)->task);
}

static int KheTaskNodeIndex(void *entry)
{
  return ((KHE_TASK_NODE) entry)->index;
}

static void KheTaskNodeSetIndex(void *entry, int index)
{
  ((KHE_TASK_NODE) entry)->index = index;
}


/*****************************************************************************/
/*                                                                           */
/*  int KheTaskNodeDecreasingWorkloadCmp(const void *t1, const void *t2)     */
/*                                                                           */
/*  Comparison function for sorting an array of task nodes by decreasing     */
/*  workload.                                                                */
/*                                                                           */
/*****************************************************************************/

/* ***
int KheTaskNodeDecreasingWorkloadCmp(const void *t1, const void *t2)
{
  KHE_TASK_NODE tn1 = * (KHE_TASK_NODE *) t1;
  KHE_TASK_NODE tn2 = * (KHE_TASK_NODE *) t2;
  float wk1 = KheTaskWorkload(tn1->task);
  float wk2 = KheTaskWorkload(tn2->task);
  if( wk1 > wk2 )
    return -1;
  else if( wk1 < wk2 )
    return 1;
  else
    return 0;
}
*** */


/*****************************************************************************/
/*                                                                           */
/*  KHE_TASK_NODE KheTaskNodeRoot(KHE_TASK_NODE tn)                          */
/*                                                                           */
/*  Return the root of tn's cluster.                                         */
/*                                                                           */
/*****************************************************************************/

static KHE_TASK_NODE KheTaskNodeRoot(KHE_TASK_NODE tn)
{
  while( tn->parent_node != NULL )
    tn = tn->parent_node;
  return tn;
}


/*****************************************************************************/
/*                                                                           */
/*  void KheTaskNodeMerge(KHE_TASK_NODE tn1, KHE_TASK_NODE tn2)              */
/*                                                                           */
/*  Merge these task nodes into one.                                         */
/*                                                                           */
/*****************************************************************************/

static void KheTaskNodeMerge(KHE_TASK_NODE tn1, KHE_TASK_NODE tn2)
{
  tn1 = KheTaskNodeRoot(tn1);
  tn2 = KheTaskNodeRoot(tn2);
  if( tn1 != tn2 )
    tn1->parent_node = tn2;
}


/*****************************************************************************/
/*                                                                           */
/*  Submodule "task sets"                                                    */
/*                                                                           */
/*****************************************************************************/

/*****************************************************************************/
/*                                                                           */
/*  void KheTaskSetAddTaskNodes(KHE_TASK_SET ts, KHE_TASK task,              */
/*    KHE_TASKING tasking)                                                   */
/*                                                                           */
/*  Add one task node to ts for each useful task in the subtree rooted at    */
/*  task, including task itself.  Ensure that every independent task lies    */
/*  in tasking.                                                              */
/*                                                                           */
/*****************************************************************************/
static void KheTaskSetAddTaskNode(KHE_TASK_SET ts, KHE_TASK_NODE tn);

static void KheTaskSetAddTaskNodes(KHE_TASK_SET ts, KHE_TASK task,
  KHE_TASKING tasking)
{
  KHE_TASK child_task;
  while( KheTaskAssignedToCount(task) > 0 )
  {
    child_task = KheTaskAssignedTo(task, 0);
    KheTaskUnAssign(child_task);
    KheTaskSetAddTaskNodes(ts, child_task, tasking);
  }
  if( KheTaskMeet(task) != NULL )
  {
    /* add a task node for task to ts */
    KheTaskSetAddTaskNode(ts, KheTaskNodeMake(ts, task));
    if( KheTaskTasking(task) != tasking )
    {
      if( KheTaskTasking(task) != NULL )
	KheTaskingDeleteTask(KheTaskTasking(task), task);
      KheTaskingAddTask(tasking, task);
    }
  }
  else
  {
    /* only used for grouping, no longer needed */
    KheTaskDelete(task);
  }
}


/*****************************************************************************/
/*                                                                           */
/*  KHE_TASK_SET KheTaskSetMake(KHE_SPLIT_SOLVER ss)                         */
/*                                                                           */
/*  Make a new task set for ss, initially with no task nodes.                */
/*                                                                           */
/*****************************************************************************/
static void KheSplitSolverAddTaskSet(KHE_SPLIT_SOLVER ss, KHE_TASK_SET ts);

static KHE_TASK_SET KheTaskSetMake(KHE_SPLIT_SOLVER ss)
{
  KHE_TASK_SET res;
  MMake(res);
  res->split_solver = ss;
  MArrayInit(res->task_nodes);
  res->unassigned_count = 0;
  MArrayInit(res->resource_nodes);
  MArrayInit(res->used_resource_nodes);
  KheSplitSolverAddTaskSet(ss, res);
  return res;
}


/*****************************************************************************/
/*                                                                           */
/*  bool KheTaskSetMakeFromTask(KHE_SPLIT_SOLVER ss, KHE_TASK task,          */
/*    KHE_TASK_SET *res)                                                     */
/*                                                                           */
/*  If a non-empty task set can be made by exploding the descendant tasks    */
/*  of task, do that, set *res to the resulting task set, and return true.   */
/*  Otherwise return false.                                                  */
/*                                                                           */
/*****************************************************************************/

static bool KheTaskSetMakeFromTask(KHE_SPLIT_SOLVER ss, KHE_TASK task,
  KHE_TASK_SET *res)
{
  KHE_TASK_SET ts;
  ts = KheTaskSetMake(ss);
  KheTaskSetAddTaskNodes(ts, task, ss->tasking);
  if( MArraySize(ts->task_nodes) > 0 )
  {
    *res = ts;
    return true;
  }
  else
  {
    MArrayFree(ts->task_nodes);
    MFree(ts);
    *res = NULL;
    return false;
  }
}


/*****************************************************************************/
/*                                                                           */
/*  void KheTaskSetAddTaskNode(KHE_TASK_SET ts, KHE_TASK_NODE tn)            */
/*                                                                           */
/*  Add tn to ts.                                                            */
/*                                                                           */
/*****************************************************************************/

static void KheTaskSetAddTaskNode(KHE_TASK_SET ts, KHE_TASK_NODE tn)
{
  MArrayAddLast(ts->task_nodes, tn);
  if( KheTaskAsstResource(tn->task) == NULL )
    ts->unassigned_count++;
}


/*****************************************************************************/
/*                                                                           */
/*  void KheTaskSetDeleteTaskNode(KHE_TASK_SET ts, KHE_TASK_NODE tn)         */
/*                                                                           */
/*  Delete tn from ts.                                                       */
/*                                                                           */
/*****************************************************************************/

static void KheTaskSetDeleteTaskNode(KHE_TASK_SET ts, KHE_TASK_NODE tn)
{
  int pos;
  if( KheTaskAsstResource(tn->task) == NULL )
    ts->unassigned_count--;
  if( !MArrayContains(ts->task_nodes, tn, &pos) )
    MAssert(false, "KheTaskSetDeleteTaskNode internal error");
  MArrayRemove(ts->task_nodes, pos);
}


/*****************************************************************************/
/*                                                                           */
/*  void KheTaskSetDelete(KHE_TASK_SET ts)                                   */
/*                                                                           */
/*  Remove all trace of ts.                                                  */
/*                                                                           */
/*****************************************************************************/
static void KheSplitSolverDeleteTaskSet(KHE_SPLIT_SOLVER ss, KHE_TASK_SET ts);

static void KheTaskSetDelete(KHE_TASK_SET ts)
{
  while( MArraySize(ts->task_nodes) > 0 )
    KheTaskNodeDelete(MArrayLast(ts->task_nodes));
  KheSplitSolverDeleteTaskSet(ts->split_solver, ts);
  MArrayFree(ts->used_resource_nodes);
  MArrayFree(ts->resource_nodes);
  MArrayFree(ts->task_nodes);
  MFree(ts);
}


/*****************************************************************************/
/*                                                                           */
/*  bool KheTaskSetExtendResourceUsage(KHE_TASK_SET ts)                      */
/*                                                                           */
/*  Add assignments to ts's task sets which use as much as possible of       */
/*  resources that are already being used by ts.  Return true if any         */
/*  progress was made.                                                       */
/*                                                                           */
/*****************************************************************************/
static bool KheTaskNodeAssign(KHE_TASK_NODE tn, KHE_RESOURCE_NODE rn);

static bool KheTaskSetExtendResourceUsage(KHE_TASK_SET ts)
{
  bool res;  int i, j;  KHE_RESOURCE_NODE rn;  KHE_TASK_NODE tn;
  res = false;
  MArraySort(ts->used_resource_nodes, &KheResourceNodeIncreasingTaskNodesCmp);
  MArrayForEach(ts->used_resource_nodes, &rn, &i)
    MArrayForEach(ts->task_nodes, &tn, &j)
      if( KheTaskAsstResource(tn->task) == NULL && KheTaskNodeAssign(tn, rn) )
      {
	res = true;
	if( DEBUG4 )
	{
	  fprintf(stderr,  "  extend %s ", KheResourceId(rn->resource)==NULL ?
	    "-" : KheResourceId(rn->resource));
	  KheTaskDebug(tn->task, 1, 0, stderr);
	}
      }
  return res;
}


/*****************************************************************************/
/*                                                                           */
/*  bool KheTaskSetComplete(KHE_TASK_SET ts)                                 */
/*                                                                           */
/*  Return true if nothing further can be done with ts.                      */
/*                                                                           */
/*  As a side effect, this function re-tests all assignabilities, and        */
/*  this is why we carry on to the end even when res is known earlier.       */
/*                                                                           */
/*****************************************************************************/

static bool KheTaskSetComplete(KHE_TASK_SET ts)
{
  KHE_TASK_NODE tn;  int i;  bool res;
  res = true;
  MArrayForEach(ts->task_nodes, &tn, &i)
    if( !KheTaskNodeComplete(tn) )
      res = false;
  return res;
}


/*****************************************************************************/
/*                                                                           */
/*  bool KheTaskNodeShareResourceNode(KHE_TASK_NODE tn1, KHE_TASK_NODE tn2)  */
/*                                                                           */
/*  Return true if tn1 and tn2 have a resource node in common.               */
/*                                                                           */
/*****************************************************************************/

static bool KheTaskNodeShareResourceNode(KHE_TASK_NODE tn1, KHE_TASK_NODE tn2)
{
  KHE_RESOURCE_NODE rn1, rn2;  int i, j;
  MArrayForEach(tn1->resource_nodes, &rn1, &i)
    MArrayForEach(tn2->resource_nodes, &rn2, &j)
      if( rn1 == rn2 )
	return true;
  return false;
}


/*****************************************************************************/
/*                                                                           */
/*  bool KheTaskSetUsesResourceNode(KHE_TASK_SET ts, KHE_RESOURCE_NODE rn)   */
/*                                                                           */
/*  Return true if ts uses rn.                                               */
/*                                                                           */
/*****************************************************************************/

static bool KheTaskSetUsesResourceNode(KHE_TASK_SET ts, KHE_RESOURCE_NODE rn)
{
  KHE_TASK_NODE tn;  int i, pos;
  MArrayForEach(ts->task_nodes, &tn, &i)
    if( MArrayContains(tn->resource_nodes, rn, &pos) )
      return true;
  return false;
}


/*****************************************************************************/
/*                                                                           */
/*  void KheTaskSetDeleteUselessResourceNodes(KHE_TASK_SET ts)               */
/*                                                                           */
/*  Delete useless resource nodes from ts.                                   */
/*                                                                           */
/*****************************************************************************/

static void KheTaskSetDeleteUselessResourceNodes(KHE_TASK_SET ts)
{
  int i;  KHE_RESOURCE_NODE rn;
  MArrayForEach(ts->resource_nodes, &rn, &i)
    if( !KheTaskSetUsesResourceNode(ts, rn) )
    {
      MArrayRemove(ts->resource_nodes, i);
      i--;
    }
}


/*****************************************************************************/
/*                                                                           */
/*  void KheTaskSetDeleteUselessTaskNodes(KHE_TASK_SET ts)                   */
/*                                                                           */
/*  Delete any assigned or unassignable task nodes of ts.                    */
/*                                                                           */
/*****************************************************************************/

static void KheTaskSetDeleteUselessTaskNodes(KHE_TASK_SET ts)
{
  int i;  KHE_TASK_NODE tn;
  MArrayForEach(ts->task_nodes, &tn, &i)
    if( KheTaskAsstResource(tn->task) != NULL ||
	MArraySize(tn->resource_nodes) == 0 )
    {
      KheTaskNodeDelete(tn);
      i--;
    }
}


/*****************************************************************************/
/*                                                                           */
/*  bool KheTaskSetPartition(KHE_TASK_SET ts)                                */
/*                                                                           */
/*  See whether ts needs to be partitioned into independent parts.  If       */
/*  it does, add new parts and remove ts and return true.  Otherwise         */
/*  change nothing and return false.                                         */
/*                                                                           */
/*****************************************************************************/
static void KheTaskSetDebug(KHE_TASK_SET ts, int verbosity,
  int indent, FILE *fp);

static bool KheTaskSetPartition(KHE_TASK_SET ts)
{
  KHE_TASK_NODE tn1, tn2;  int i, j, pos, parts;  KHE_RESOURCE_NODE rn;

  /* remove assigned and unassignable task nodes */
  KheTaskSetDeleteUselessTaskNodes(ts);

  /* clear out all the parent_node fields, just in case */
  MArrayForEach(ts->task_nodes, &tn1, &i)
    tn1->parent_node = NULL;

  /* merge pairs of task nodes that have a resource in common */
  parts = MArraySize(ts->task_nodes);
  MArrayForEach(ts->task_nodes, &tn1, &i)
    MArrayForEach(ts->task_nodes, &tn2, &j)
      if( KheTaskNodeRoot(tn1) != KheTaskNodeRoot(tn2) &&
          KheTaskNodeShareResourceNode(tn1, tn2) )
      {
        KheTaskNodeMerge(tn1, tn2);
	parts--;
      }

  /* if just one part, change nothing and return false */
  if( parts == 1 )
    return false;
  if( DEBUG5 )
  {
    fprintf(stderr, "  partitioning task set into %d parts:\n", parts);
    KheTaskSetDebug(ts, 2, 4, stderr);
  }

  /* clear the task sets out of all task nodes */
  MArrayForEach(ts->task_nodes, &tn1, &i)
    tn1->task_set = NULL;

  /* find the set of distinct root nodes and build task sets for them */
  MArrayForEach(ts->task_nodes, &tn1, &i)
  {
    tn1 = KheTaskNodeRoot(tn1);
    if( tn1->task_set == NULL )
    {
      tn1->task_set = KheTaskSetMake(ts->split_solver);
      MArrayForEach(ts->used_resource_nodes, &rn, &j)
	MArrayAddLast(tn1->task_set->used_resource_nodes, rn);
      KheTaskSetAddTaskNode(tn1->task_set, tn1);
      MArrayForEach(tn1->resource_nodes, &rn, &j)
	if( !MArrayContains(tn1->task_set->resource_nodes, rn, &pos) )
	  MArrayAddLast(tn1->task_set->resource_nodes, rn);
    }
  }

  /* add each non-root task node to its root's task set */
  MArrayForEach(ts->task_nodes, &tn1, &i)
    if( tn1->task_set == NULL )
    {
      tn2 = KheTaskNodeRoot(tn1);
      MAssert(tn2->task_set != NULL, "KheTaskSetPartition internal error 1");
      tn1->task_set = tn2->task_set;
      KheTaskSetAddTaskNode(tn1->task_set, tn1);
      MArrayForEach(tn1->resource_nodes, &rn, &j)
	if( !MArrayContains(tn1->task_set->resource_nodes, rn, &pos) )
	  MArrayAddLast(tn1->task_set->resource_nodes, rn);
    }

  /* debug */
  if( DEBUG5 )
  {
    fprintf(stderr, "  the parts (this list may have repetitions):\n");
    MArrayForEach(ts->task_nodes, &tn1, &i)
      KheTaskSetDebug(tn1->task_set, 2, 4, stderr);
  }

  /* remove the original ts */
  MArrayClear(ts->task_nodes);
  KheTaskSetDelete(ts);
  return true;
}


/*****************************************************************************/
/*                                                                           */
/*  int KheTaskSetDuration(KHE_TASK_SET ts)                                  */
/*                                                                           */
/*  Return the total duration of the task nodes of ts.                       */
/*                                                                           */
/*****************************************************************************/

static int KheTaskSetDuration(KHE_TASK_SET ts)
{
  int i, res;  KHE_TASK_NODE tn;
  res = 0;
  MArrayForEach(ts->task_nodes, &tn, &i)
    res += KheTaskDuration(tn->task);
  return res;
}


/*****************************************************************************/
/*                                                                           */
/*  int KheTaskSetDecreasingDurationCmp(const void *t1, const void *t2)      */
/*                                                                           */
/*  Comparison function for sorting an array of task sets by decreasing      */
/*  duration.                                                                */
/*                                                                           */
/*****************************************************************************/

static int KheTaskSetDecreasingDurationCmp(const void *t1, const void *t2)
{
  KHE_TASK_SET ts1 = * (KHE_TASK_SET *) t1;
  KHE_TASK_SET ts2 = * (KHE_TASK_SET *) t2;
  KHE_TASK task1, task2;
  int durn1 = KheTaskSetDuration(ts1);
  int durn2 = KheTaskSetDuration(ts2);
  if( durn1 != durn2 )
    return durn2 - durn1;
  else
  {
    task1 = MArrayFirst(ts1->task_nodes)->task;
    task2 = MArrayFirst(ts2->task_nodes)->task;
    return KheTaskIndexInSoln(task1) - KheTaskIndexInSoln(task2);
  }
}


/*****************************************************************************/
/*                                                                           */
/*  void KheTaskSetDebug(KHE_TASK_SET ts, int verbosity,                     */
/*    int indent, FILE *fp)                                                  */
/*                                                                           */
/*  Debug print of ts onto fp with the given verbosity and indent.           */
/*                                                                           */
/*****************************************************************************/

static void KheTaskSetDebug(KHE_TASK_SET ts, int verbosity,
  int indent, FILE *fp)
{
  KHE_TASK_NODE tn;  int i, j, pos;  KHE_RESOURCE_NODE rn;
  if( verbosity >= 1 && indent >= 0 )
  {
    /* print the tasks, along the first line */
    fprintf(fp, "%*s[ unass %d, durn %d: ", indent, "",
      ts->unassigned_count, KheTaskSetDuration(ts));
    MArrayForEach(ts->task_nodes, &tn, &i)
    {
      if( i > 0 )
	fprintf(fp, ", ");
      KheTaskDebug(tn->task, 1, -1, fp);
    }
    fprintf(fp, "\n");

    /* print the header of the assignability table */
    fprintf(fp, "%*s  %-20s", indent, "", "Resources");
    for( i = 0;  i < MArraySize(ts->task_nodes);  i++ )
      fprintf(fp, " %2d", i + 1);
    fprintf(fp, "\n");
    fprintf(fp, "%*s  --------------------", indent, "");
    for( i = 0;  i < MArraySize(ts->task_nodes);  i++ )
      fprintf(fp, "---");
    fprintf(fp, "\n");

    /* print the rows of the assignability table */
    MArrayForEach(ts->resource_nodes, &rn, &i)
    {
      fprintf(fp, "%*s  %-20s", indent, "",
	KheResourceId(rn->resource)!=NULL ? KheResourceId(rn->resource) : "-");
      MArrayForEach(ts->task_nodes, &tn, &j)
	fprintf(fp, "  %c",
	  KheTaskAsstResource(tn->task) == rn->resource ? '@' :
	  !MArrayContains(tn->resource_nodes, rn, &pos) ? ' ' :
          KheTaskNodeAssignable(tn, rn) ? '*' : '!');
      fprintf(fp, "\n");
    }
    fprintf(fp, "%*s  --------------------", indent, "");
    for( i = 0;  i < MArraySize(ts->task_nodes);  i++ )
      fprintf(fp, "---");
    fprintf(fp, "\n");
    fprintf(fp, "%*s]\n", indent, "");
  }
}


/*****************************************************************************/
/*                                                                           */
/*  Submodule "helper functions for resource and task nodes, and task sets"  */
/*                                                                           */
/*****************************************************************************/

/*****************************************************************************/
/*                                                                           */
/*  bool KheTaskNodeAssignable(KHE_TASK_NODE tn, KHE_RESOURCE_NODE rn)       */
/*                                                                           */
/*  Return true if tn is assignable to rn.                                   */
/*                                                                           */
/*****************************************************************************/

static bool KheTaskNodeAssignable(KHE_TASK_NODE tn, KHE_RESOURCE_NODE rn)
{
  KHE_SOLN soln;  int unmatchable_before, unmatchable_after;
  KHE_COST cost_before, cost_after;  bool res;
  soln = KheTaskSoln(tn->task);
  unmatchable_before = KheSolnMatchingDefectCount(soln);
  cost_before = KheSolnCost(soln);
  if( DEBUG8 )
  {
    fprintf(stderr, "  [ KheTaskNodeAssignable(");
    KheTaskDebug(tn->task, 1, -1, stderr);
    fprintf(stderr, ", ");
    KheResourceDebug(rn->resource, 1, -1, stderr);
    fprintf(stderr, "\n");
  }
  if( KheTaskAssignResource(tn->task, rn->resource) )
  {
    unmatchable_after = KheSolnMatchingDefectCount(soln);
    cost_after = KheSolnCost(soln);
    if( DEBUG8 && strcmp(KheResourceId(rn->resource), "Welfare01") == 0 )
    {
      int i;  KHE_MONITOR m;  KHE_EVENT_RESOURCE er;
      er = KheTaskEventResource(tn->task);
      fprintf(stderr, "    when assigned (%s event resource):\n",
	er != NULL ? "has" : "no");
      for( i = 0;  i < KheEventResourceMonitorCount(soln, er);  i++ )
      {
	m = KheEventResourceMonitor(soln, er, i);
	KheMonitorDebug(m, 1, 4, stderr);
      }
      KheSolnCostByTypeDebug(KheTaskSoln(tn->task), 1, 4, stderr);
    }
    KheTaskUnAssignResource(tn->task);
    if( DEBUG8 && strcmp(KheResourceId(rn->resource), "Welfare01") == 0 )
    {
      int i;  KHE_MONITOR m;  KHE_EVENT_RESOURCE er;
      er = KheTaskEventResource(tn->task);
      fprintf(stderr, "    when unassigned (%s event resource):\n",
	er != NULL ? "has" : "no");
      for( i = 0;  i < KheEventResourceMonitorCount(soln, er);  i++ )
      {
	m = KheEventResourceMonitor(soln, er, i);
	KheMonitorDebug(m, 1, 4, stderr);
      }
      KheSolnCostByTypeDebug(KheTaskSoln(tn->task), 1, 4, stderr);
    }
    res = cost_after < cost_before && unmatchable_after <= unmatchable_before;
    if( DEBUG8 )
      fprintf(stderr,
	"  ] KheTaskNodeAssignable returning %s: (%.4f < %.4f && %d <= %d)\n",
	res ? "true" : "false", KheCostShow(cost_after),
	KheCostShow(cost_before), unmatchable_after, unmatchable_before);
    return res;
  }
  else
  {
    if( DEBUG8 )
      fprintf(stderr,
	"  ] KheTaskNodeAssignable returning false (KheTaskAssignResource)\n");
    return false;
  }
}


/*****************************************************************************/
/*                                                                           */
/*  void KheResourceNodeReduceLinks(KHE_RESOURCE_NODE rn)                    */
/*                                                                           */
/*  Resource node rn has changed in some way, so now we need to reduce its   */
/*  links to task nodes to nodes that are still assignable.                  */
/*                                                                           */
/*****************************************************************************/

static void KheResourceNodeReduceLinks(KHE_RESOURCE_NODE rn)
{
  KHE_TASK_NODE tn;  int i;
  MArrayForEach(rn->task_nodes, &tn, &i)
    if( !KheTaskNodeAssignable(tn, rn) )
    {
      KheTaskNodeDeleteResourceNode(tn, rn);
      KheResourceNodeDeleteTaskNode(rn, tn);
      i--;
    }
  /* *** don't ever delete rn!
  if( MArraySize(rn->task_nodes) == 0 )
    KheResourceNodeDelete(rn);
  *** */
}


/*****************************************************************************/
/*                                                                           */
/*  bool KheTaskNodeAssign(KHE_TASK_NODE tn, KHE_RESOURCE_NODE rn)           */
/*                                                                           */
/*  Return true if tn is assignable to rn, and actually do it and update     */
/*  everything accordingly.                                                  */
/*                                                                           */
/*****************************************************************************/

static bool KheTaskNodeAssign(KHE_TASK_NODE tn, KHE_RESOURCE_NODE rn)
{
  KHE_SOLN soln;  int unmatchable_before, unmatchable_after, pos;
  KHE_COST cost_before, cost_after;
  soln = KheTaskSoln(tn->task);
  unmatchable_before = KheSolnMatchingDefectCount(soln);
  cost_before = KheSolnCost(soln);
  if( KheTaskAssignResource(tn->task, rn->resource) )
  {
    unmatchable_after = KheSolnMatchingDefectCount(soln);
    cost_after = KheSolnCost(soln);
    if( cost_after < cost_before && unmatchable_after <= unmatchable_before )
    {
      /* success; update everything */
      tn->task_set->unassigned_count--;
      while( MArraySize(tn->resource_nodes) > 0 )
      {
	rn = MArrayFirst(tn->resource_nodes);
	KheTaskNodeDeleteResourceNode(tn, rn);
	KheResourceNodeDeleteTaskNode(rn, tn);
      }
      KheResourceNodeReduceLinks(rn);
      if( !MArrayContains(tn->task_set->used_resource_nodes, rn, &pos) )
        MArrayAddLast(tn->task_set->used_resource_nodes, rn);
      return true;
    }
    else
    {
      /* failure; return everything to its previous state */
      KheTaskUnAssignResource(tn->task);
      return false;
    }
  }
  else
    return false;
}


/*****************************************************************************/
/*                                                                           */
/*  bool KheTaskSetAssignable(KHE_TASK_SET ts, KHE_RESOURCE_NODE rn,         */
/*    KHE_COST *cost)                                                        */
/*                                                                           */
/*  If rn is assignable to every unassigned task node of ts simultaneously,  */
/*  return true and set *cost to the solution cost when that is done.  Else  */
/*  return false.  Change nothing either way.                                */
/*                                                                           */
/*****************************************************************************/

static bool KheTaskSetAssignable(KHE_TASK_SET ts, KHE_RESOURCE_NODE rn,
  KHE_COST *cost)
{
  KHE_TASK_NODE tn;  int i, unmatchable_before;  KHE_SOLN soln;
  KHE_TRANSACTION t;  KHE_COST cost_before;
  MAssert(MArraySize(ts->task_nodes) > 0, "KheTaskSetAssign internal error");
  tn = MArrayFirst(ts->task_nodes);
  soln = KheTaskSoln(tn->task);
  unmatchable_before = KheSolnMatchingDefectCount(soln);
  t = KheTransactionMake(soln);
  KheTransactionBegin(t);
  MArrayForEach(ts->task_nodes, &tn, &i)
    if( KheTaskAsstResource(tn->task) == NULL )
    {
      cost_before = KheSolnCost(soln);
      if( !KheTaskAssignResource(tn->task, rn->resource) ||
	  KheSolnCost(soln) >= cost_before ||
	  KheSolnMatchingDefectCount(soln) > unmatchable_before )
      {
	/* undo and fail */
	KheTransactionEnd(t);
	KheTransactionUndo(t);
	KheTransactionDelete(t);
	return false;
      }
    }
  *cost = KheSolnCost(soln);
  KheTransactionEnd(t);
  KheTransactionUndo(t);
  KheTransactionDelete(t);
  return true;
}


/*****************************************************************************/
/*                                                                           */
/*  bool KheTaskSetAssignable2(KHE_TASK_SET ts, KHE_RESOURCE_NODE rn1,       */
/*    KHE_RESOURCE_NODE rn2, KHE_COST *cost)                                 */
/*                                                                           */
/*  If ts can be assigned rn1 and rn2 simultaneously by a greedy method,     */
/*  then return true and set *cost to the solution cost when that is done.   */
/*  Change nothing either way.                                               */
/*                                                                           */
/*****************************************************************************/

static bool KheTaskSetAssignable2(KHE_TASK_SET ts, KHE_RESOURCE_NODE rn1,
  KHE_RESOURCE_NODE rn2, KHE_COST *cost)
{
  KHE_TASK_NODE tn;  int i, unmatchable_before, pos;  KHE_SOLN soln;
  KHE_TRANSACTION t;  KHE_COST cost_before;

  /* a simple check first, to make sure rn1 and rn2 cover ts */
  MArrayForEach(ts->task_nodes, &tn, &i)
    if( !MArrayContains(tn->resource_nodes, rn1, &pos) &&
        !MArrayContains(tn->resource_nodes, rn1, &pos) )
      return false;

  /* bumf */
  MAssert(MArraySize(ts->task_nodes) > 0, "KheTaskSetAssign2 internal error");
  tn = MArrayFirst(ts->task_nodes);
  soln = KheTaskSoln(tn->task);
  unmatchable_before = KheSolnMatchingDefectCount(soln);
  t = KheTransactionMake(soln);
  KheTransactionBegin(t);

  /* assign rn1 or rn2 to as much as possible */
  MArrayForEach(ts->task_nodes, &tn, &i)
  {
    /* try assigning rn1 */
    cost_before = KheSolnCost(soln);
    if( KheTaskAsstResource(tn->task) == NULL &&
        KheTaskAssignResource(tn->task, rn1->resource) )
    {
      if( KheSolnCost(soln) >= cost_before ||
	  KheSolnMatchingDefectCount(soln) > unmatchable_before )
	KheTaskUnAssignResource(tn->task);
    }

    /* try assigning rn2 */
    cost_before = KheSolnCost(soln);
    if( KheTaskAsstResource(tn->task) == NULL &&
        KheTaskAssignResource(tn->task, rn2->resource) )
    {
      if( KheSolnCost(soln) >= cost_before ||
	  KheSolnMatchingDefectCount(soln) > unmatchable_before )
	KheTaskUnAssignResource(tn->task);
    }

    /* if still no luck, fail */
    if( KheTaskAsstResource(tn->task) == NULL )
    {
      /* undo and fail */
      KheTransactionEnd(t);
      KheTransactionUndo(t);
      KheTransactionDelete(t);
      return false;
    }
  }

  /* success, so record the cost and undo */
  *cost = KheSolnCost(soln);
  KheTransactionEnd(t);
  KheTransactionUndo(t);
  KheTransactionDelete(t);
  return true;
}


/*****************************************************************************/
/*                                                                           */
/*  void KheTaskSetAssign(KHE_TASK_SET ts, KHE_RESOURCE_NODE rn)             */
/*                                                                           */
/*  Assign rn to every unassigned task of ts.  This is assumed to be safe.   */
/*                                                                           */
/*****************************************************************************/

static void KheTaskSetAssign(KHE_TASK_SET ts, KHE_RESOURCE_NODE rn)
{
  KHE_TASK_NODE tn;  int i;
  MArrayForEach(ts->task_nodes, &tn, &i)
    if( KheTaskAsstResource(tn->task) == NULL )
    {
      if( !KheTaskNodeAssign(tn, rn) )
	MAssert(false, "KheTaskSetAssign internal error");
    }
}


/*****************************************************************************/
/*                                                                           */
/*  void KheTaskSetAssign2(KHE_TASK_SET ts, KHE_RESOURCE_NODE rn1,           */
/*    KHE_RESOURCE_NODE rn2)                                                 */
/*                                                                           */
/*  Assign rn1 and rn2 to every unassigned task of ts.  This is assumed to   */
/*  be safe.                                                                 */
/*                                                                           */
/*****************************************************************************/

static void KheTaskSetAssign2(KHE_TASK_SET ts, KHE_RESOURCE_NODE rn1,
  KHE_RESOURCE_NODE rn2)
{
  KHE_TASK_NODE tn;  int i;
  MArrayForEach(ts->task_nodes, &tn, &i)
    if( KheTaskAsstResource(tn->task) == NULL )
    {
      if( !KheTaskNodeAssign(tn, rn1) && !KheTaskNodeAssign(tn, rn2) )
	MAssert(false, "KheTaskSetAssign internal error");
    }
}


/*****************************************************************************/
/*                                                                           */
/*  bool KheTaskSetTrySingle(KHE_TASK_SET ts)                                 */
/*                                                                           */
/*  If a single resource will cover ts, make the best of those and return    */
/*  true, else change nothing and return false.                              */
/*                                                                           */
/*****************************************************************************/

static bool KheTaskSetTrySingle(KHE_TASK_SET ts)
{
  KHE_COST cost, best_cost;  KHE_RESOURCE_NODE rn, best_rn;  int i;
  best_rn = NULL;
  best_cost = KheCostMax;
  MArrayForEach(ts->resource_nodes, &rn, &i)
    if( KheTaskSetAssignable(ts, rn, &cost) &&
	(cost < best_cost || (cost == best_cost &&
	  MArraySize(rn->task_nodes) < MArraySize(best_rn->task_nodes))) )
    {
      cost = best_cost;
      best_rn = rn;
    }
  if( best_rn != NULL )
  {
    if( DEBUG6 )
    {
      fprintf(stderr, "  single %s to task set:\n",
	KheResourceId(best_rn->resource) == NULL ? "-" :
	KheResourceId(best_rn->resource));
      KheTaskSetDebug(ts, 1, 2, stderr);
    }
    KheTaskSetAssign(ts, best_rn);
    return true;
  }
  else
    return false;
}


/*****************************************************************************/
/*                                                                           */
/*  bool KheTaskSetTryDouble(KHE_TASK_SET ts)                                */
/*                                                                           */
/*  If a greedy assignment of two resources will cover ts, do that and       */
/*  return true, else change nothing and return false.                       */
/*                                                                           */
/*****************************************************************************/

static bool KheTaskSetTryDouble(KHE_TASK_SET ts)
{
  KHE_COST cost, best_cost;  KHE_RESOURCE_NODE rn1, rn2, best_rn1, best_rn2;
  int i, j;
  best_rn1 = best_rn2 = NULL;
  best_cost = KheCostMax;
  MArrayForEach(ts->resource_nodes, &rn1, &i)
    MArrayForEach(ts->resource_nodes, &rn2, &j)
      if( i < j && KheTaskSetAssignable2(ts, rn1, rn2, &cost) &&
	(cost < best_cost || (cost == best_cost &&
	 MArraySize(rn1->task_nodes) + MArraySize(rn2->task_nodes) <
	 MArraySize(best_rn1->task_nodes) + MArraySize(best_rn2->task_nodes))) )
    {
      cost = best_cost;
      best_rn1 = rn1;
      best_rn2 = rn2;
    }
  if( best_rn1 != NULL )
  {
    if( DEBUG6 )
    {
      fprintf(stderr, "  double %s, %s to task set (just before asst):\n",
	KheResourceId(best_rn1->resource) == NULL ? "-" :
	KheResourceId(best_rn1->resource),
	KheResourceId(best_rn2->resource) == NULL ? "-" :
	KheResourceId(best_rn2->resource));
      KheTaskSetDebug(ts, 1, 2, stderr);
    }
    KheTaskSetAssign2(ts, best_rn1, best_rn2);
    return true;
  }
  else
    return false;
}


/*****************************************************************************/
/*                                                                           */
/*  Submodule "split solvers"                                                */
/*                                                                           */
/*****************************************************************************/

/*****************************************************************************/
/*                                                                           */
/*  void KheSplitSolverAddResourceNode(KHE_SPLIT_SOLVER ss,                  */
/*    KHE_RESOURCE_NODE rn)                                                  */
/*                                                                           */
/*  Add rn to ss.                                                            */
/*                                                                           */
/*****************************************************************************/

static void KheSplitSolverAddResourceNode(KHE_SPLIT_SOLVER ss,
  KHE_RESOURCE_NODE rn)
{
  MArrayAddLast(ss->resource_nodes, rn);
}


/*****************************************************************************/
/*                                                                           */
/*  void KheSplitSolverDeleteResourceNode(KHE_SPLIT_SOLVER ss,               */
/*    KHE_RESOURCE_NODE rn)                                                  */
/*                                                                           */
/*  Delete rn from ss.                                                       */
/*                                                                           */
/*****************************************************************************/

static void KheSplitSolverDeleteResourceNode(KHE_SPLIT_SOLVER ss,
  KHE_RESOURCE_NODE rn)
{
  int pos;
  if( !MArrayContains(ss->resource_nodes, rn, &pos) )
    MAssert(false, "KheSplitSolverDeleteResourceNode internal error");
  MArrayRemove(ss->resource_nodes, pos);
}


/*****************************************************************************/
/*                                                                           */
/*  void KheSplitSolverAddTaskSet(KHE_SPLIT_SOLVER ss, KHE_TASK_SET ts)      */
/*                                                                           */
/*  Add ts to ss.                                                            */
/*                                                                           */
/*****************************************************************************/

static void KheSplitSolverAddTaskSet(KHE_SPLIT_SOLVER ss, KHE_TASK_SET ts)
{
  MArrayAddLast(ss->task_sets, ts);
}


/*****************************************************************************/
/*                                                                           */
/*  void KheSplitSolverDeleteTaskSet(KHE_SPLIT_SOLVER ss, KHE_TASK_SET ts)   */
/*                                                                           */
/*  Delete ts from ss.                                                       */
/*                                                                           */
/*****************************************************************************/

static void KheSplitSolverDeleteTaskSet(KHE_SPLIT_SOLVER ss, KHE_TASK_SET ts)
{
  int pos;
  if( !MArrayContains(ss->task_sets, ts, &pos) )
    MAssert(false, "KheSplitSolverDeleteTaskSet internal error");
  MArrayRemove(ss->task_sets, pos);
}


/*****************************************************************************/
/*                                                                           */
/*  KHE_SPLIT_SOLVER KheSplitSolverMake(KHE_TASKING tasking)                 */
/*                                                                           */
/*  Make a split solver for assigning the unassigned tasks of tasking.       */
/*                                                                           */
/*****************************************************************************/

static KHE_SPLIT_SOLVER KheSplitSolverMake(KHE_TASKING tasking)
{
  KHE_SPLIT_SOLVER res;  int i, j, k, pos;  KHE_TASK task;
  KHE_INSTANCE ins;  KHE_RESOURCE r;  ARRAY_KHE_TASK unassigned_tasks;
  KHE_TASK_SET ts;  KHE_TASK_NODE tn;  KHE_RESOURCE_NODE rn;
  KHE_RESOURCE_TYPE rt;
  if( DEBUG7 )
    fprintf(stderr, "[ KheSplitSolverMake(tasking)\n");

  /* initialize the split solver object */
  MMake(res);
  res->tasking = tasking;
  MArrayInit(res->resource_nodes);
  MArrayInit(res->task_sets);
  res->forced_task_nodes = KhePriQueueMake(&KheTaskNodeKey,
    &KheTaskNodeIndex, &KheTaskNodeSetIndex);

  /* gather the unassigned tasks of tasking */
  MArrayInit(unassigned_tasks);
  for( i = 0;  i < KheTaskingTaskCount(tasking);  i++ )
  {
    task = KheTaskingTask(tasking, i);
    if( KheTaskAsst(task) == NULL )
    {
      MArrayAddLast(unassigned_tasks, task);
      if( DEBUG7 )
      {
	fprintf(stderr, "  adding unassigned task ");
	KheTaskDebug(task, 1, 0, stderr);
      }
    }
  }

  /* add one task set for each unassigned task */
  MArrayForEach(unassigned_tasks, &task, &i)
    KheTaskSetMakeFromTask(res, task, &ts);

  /* add one resource node for each unpreass resource of the instance */
  ins = KheSolnInstance(KheTaskingSoln(tasking));
  for( i = 0;  i < KheInstanceResourceTypeCount(ins);  i++ )
  {
    rt = KheInstanceResourceType(ins, i);
    if( !KheResourceTypeDemandIsAllPreassigned(rt) )
      for( j = 0;  j < KheResourceTypeResourceCount(rt);  j++ )
      {
	r = KheResourceTypeResource(rt, j);
	KheSplitSolverAddResourceNode(res, KheResourceNodeMake(res, r));
      }
  }

  /* find the linkages between task nodes and resource nodes */
  MArrayForEach(res->task_sets, &ts, &i)
    MArrayForEach(ts->task_nodes, &tn, &j)
      MArrayForEach(res->resource_nodes, &rn, &k)
        if( KheTaskNodeAssignable(tn, rn) )
	{
	  KheResourceNodeAddTaskNode(rn, tn);
	  KheTaskNodeAddResourceNode(tn, rn);
	  if( !MArrayContains(ts->resource_nodes, rn, &pos) )
	    MArrayAddLast(ts->resource_nodes, rn);
	}

  /* remove useless resource nodes (once-off at start only) */
  MArrayForEach(res->resource_nodes, &rn, &i)
    if( MArraySize(rn->task_nodes) == 0 )
    {
      if( DEBUG7 )
	fprintf(stderr, "  removing useless resource node %s\n",
	  KheResourceId(rn->resource));
      KheResourceNodeDelete(rn);
      i--;
    }

  /* remove unassignable task nodes and task sets */
  MArrayForEach(res->task_sets, &ts, &i)
  {
    KheTaskSetDeleteUselessTaskNodes(ts);
    if( MArraySize(ts->task_nodes) == 0 )
    {
      if( DEBUG7 )
	fprintf(stderr, "  removing useless task set\n");
      KheTaskSetDelete(ts);
      i--;
    }
  }

  /* add forced task nodes to res->forced_task_nodes priqueue */
  MArrayForEach(res->task_sets, &ts, &i)
    MArrayForEach(ts->task_nodes, &tn, &j)
      if( MArraySize(tn->resource_nodes) == 1 )
      {
	if( DEBUG3 )
	{
	  fprintf(stderr, "  priqueue init insert (duration %d) ",
	    KheTaskDuration(tn->task));
	  KheTaskDebug(tn->task, 1, 0, stderr);
	}
	KhePriQueueInsert(res->forced_task_nodes, (void *) tn);
      }

  if( DEBUG7 )
    fprintf(stderr, "] KheSplitSolverMake returning\n");
  return res;
}


/*****************************************************************************/
/*                                                                           */
/*  void KheSplitSolverDelete(KHE_SPLIT_SOLVER ss)                           */
/*                                                                           */
/*  Delete ss and everything in it.                                          */
/*                                                                           */
/*****************************************************************************/

static void KheSplitSolverDelete(KHE_SPLIT_SOLVER ss)
{
  while( MArraySize(ss->resource_nodes) > 0 )
    KheResourceNodeDelete(MArrayFirst(ss->resource_nodes));
  while( MArraySize(ss->task_sets) > 0 )
    KheTaskSetDelete(MArrayFirst(ss->task_sets));
  KhePriQueueDelete(ss->forced_task_nodes);
  MArrayFree(ss->resource_nodes);
  MArrayFree(ss->task_sets);
  MFree(ss);
}


/*****************************************************************************/
/*                                                                           */
/*  void KheSplitSolverDebug(KHE_SPLIT_SOLVER ss, int verbosity,             */
/*    int indent, FILE *fp)                                                  */
/*                                                                           */
/*  Debug print of ss with onto fp with the given verbosity and indent.      */
/*                                                                           */
/*****************************************************************************/

static void KheSplitSolverDebug(KHE_SPLIT_SOLVER ss, int verbosity,
  int indent, FILE *fp)
{
  int i;  KHE_TASK_SET ts;  KHE_RESOURCE_NODE rn;  /* KHE_TASK_NODE tn; */
  if( verbosity >= 1 && indent >= 0 )
  {
    fprintf(fp, "%*s[ SplitSolver (%d task sets, %d resource nodes)\n",
      indent, "", MArraySize(ss->task_sets), MArraySize(ss->resource_nodes));
    MArrayForEach(ss->task_sets, &ts, &i)
      KheTaskSetDebug(ts, verbosity, indent + 2, fp);
    MArrayForEach(ss->resource_nodes, &rn, &i)
      KheResourceNodeDebug(rn, verbosity, indent + 2, fp);
    /* ***
    MArrayForEach(ss->forced_task_nodes, &tn, &i)
    {
      fprintf(fp, "%*s  forced (wk %6.2f) ", indent, "",
	KheTaskWorkload(tn->task));
      KheTaskDebug(tn->task, 1, 0, fp);
    }
    *** */
    fprintf(fp, "%*s]\n", indent, "");
  }
}


/*****************************************************************************/
/*                                                                           */
/*  Submodule "solving"                                                      */
/*                                                                           */
/*****************************************************************************/

/*****************************************************************************/
/*                                                                           */
/*  void KheClearForcedAssts(KHE_SPLIT_SOLVER ss)                            */
/*                                                                           */
/*  Keep making forced assignments, if possible, until all are handled.      */
/*                                                                           */
/*****************************************************************************/

static void KheClearForcedAssts(KHE_SPLIT_SOLVER ss)
{
  KHE_TASK_NODE tn;  KHE_RESOURCE_NODE rn;  bool success;
  while( !KhePriQueueEmpty(ss->forced_task_nodes) )
  {
    /* find the widest unassigned forced task node and its sole rn */
    tn = (KHE_TASK_NODE) KhePriQueueFindMin(ss->forced_task_nodes);
    MAssert(MArraySize(tn->resource_nodes) == 1,
      "KheClearForcedAssts internal error");
    rn = MArrayFirst(tn->resource_nodes);

    /* assign tn if possible, else reduce; either way tn leaves the priqueue */
    success = KheTaskNodeAssign(tn, rn);
    if( !success )
    {
      KheTaskNodeDeleteResourceNode(tn, rn);
      KheResourceNodeDeleteTaskNode(rn, tn);
    }
    if( DEBUG2 )
    {
      fprintf(stderr, "  %s forced asst of %s to durn %d ",
	success ? "successful" : "unsuccessful",
	KheResourceId(rn->resource) == NULL ? "-" : KheResourceId(rn->resource),
	KheTaskDuration(tn->task));
      KheTaskDebug(tn->task, 1, 0, stderr);
    }
  }
}


/*****************************************************************************/
/*                                                                           */
/*  void KheFindSplitResourceAssignments(KHE_TASKING tasking)                */
/*                                                                           */
/*  Find split assignments for the unassigned tasks of tasking.              */
/*                                                                           */
/*****************************************************************************/

void KheFindSplitResourceAssignments(KHE_TASKING tasking)
{
  KHE_SPLIT_SOLVER ss;  bool progressing;  int i, try;  KHE_TASK_SET ts;
  if( DEBUG1 )
    fprintf(stderr, "[ KheFindSplitResourceAssignments(%s) %d tasks\n",
      KheTaskingResourceType(tasking) == NULL ? "~" :
      KheResourceTypeId(KheTaskingResourceType(tasking)) == NULL ? "-" :
      KheResourceTypeId(KheTaskingResourceType(tasking)),
      KheTaskingTaskCount(tasking));

  /* make a split solver; carry out forced assignments and their extensions */
  ss = KheSplitSolverMake(tasking);
  KheClearForcedAssts(ss);
  do
  {
    if( DEBUG1 )
      fprintf(stderr, "  forced asst loop:\n");
    progressing = false;
    MArrayForEach(ss->task_sets, &ts, &i)
      if( KheTaskSetExtendResourceUsage(ts) )
      {
	KheClearForcedAssts(ss);
	progressing = true;
      }
  } while( progressing );

  for( try = 1;  try <= 6;  try++ )
  {
    /* remove completed task sets */
    MArrayForEach(ss->task_sets, &ts, &i)
      if( KheTaskSetComplete(ts) )
      {
	KheTaskSetDelete(ts);
	i--;
      }
    KheClearForcedAssts(ss);

    /* partition the task sets and sort by decreasing total duration */
    MArrayForEach(ss->task_sets, &ts, &i)
      if( KheTaskSetPartition(ts) )
	i--;
    MArraySort(ss->task_sets, &KheTaskSetDecreasingDurationCmp);

    /* remove useless resource nodes from task sets */
    MArrayForEach(ss->task_sets, &ts, &i)
      KheTaskSetDeleteUselessResourceNodes(ts);
    if( DEBUG1 )
      KheSplitSolverDebug(ss, 2, 2, stderr);

    /* make single assignments where possible */
    MArrayForEach(ss->task_sets, &ts, &i)
      if( KheTaskSetTrySingle(ts) )
      {
	KheTaskSetDelete(ts);
	i--;
	KheClearForcedAssts(ss);
      }

    /* make greedy double assignments where possible */
    MArrayForEach(ss->task_sets, &ts, &i)
      if( KheTaskSetTryDouble(ts) )
      {
	KheTaskSetDelete(ts);
	i--;
	KheClearForcedAssts(ss);
      }
    if( DEBUG1 )
    {
      fprintf(stderr, "  after doubles (at end of try %d):\n", try);
      KheSplitSolverDebug(ss, 2, 2, stderr);
    }
  }

  /* delete the split solver */
  KheSplitSolverDelete(ss);
  if( DEBUG1 )
    fprintf(stderr, "] KheFindSplitResourceAssignments returning\n");
}
