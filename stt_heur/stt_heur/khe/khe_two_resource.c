
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
/*  FILE:         khe_two_resource.c                                         */
/*  DESCRIPTION:  Two-colouring resource repair                              */
/*                                                                           */
/*****************************************************************************/
#include "khe.h"
#include "m.h"
#include "khe_lset.h"
#include <limits.h>

#define DEBUG1 0
#define DEBUG2 0
#define DEBUG3 0
#define DEBUG4 0
#define DEBUG5 0
#define DEBUG6 0
#define DEBUG7 0

#define KHE_TWO_MAX_NODES 256

typedef MARRAY(KHE_RESOURCE) ARRAY_KHE_RESOURCE;
typedef MARRAY(KHE_TASK) ARRAY_KHE_TASK;
typedef MARRAY(KHE_AVOID_SPLIT_ASSIGNMENTS_MONITOR)
  ARRAY_KHE_AVOID_SPLIT_ASSIGNMENTS_MONITOR;

typedef struct khe_two_node_rec *KHE_TWO_NODE;
typedef MARRAY(KHE_TWO_NODE) ARRAY_KHE_TWO_NODE;

typedef struct khe_two_component_rec *KHE_TWO_COMPONENT;
typedef MARRAY(KHE_TWO_COMPONENT) ARRAY_KHE_TWO_COMPONENT;

typedef struct khe_two_graph_rec *KHE_TWO_GRAPH;
typedef MARRAY(KHE_TWO_GRAPH) ARRAY_KHE_TWO_GRAPH;


/*****************************************************************************/
/*                                                                           */
/*  KHE_TWO_NODE - a node in the two-colouring clash graph                   */
/*                                                                           */
/*****************************************************************************/

struct khe_two_node_rec {
  ARRAY_KHE_TASK	tasks;			/* the tasks                 */
  LSET			assigned_times;		/* their assigned times      */
  float			workload;		/* their total workload      */
  KHE_RESOURCE		resource;		/* assigned resource         */
};


/*****************************************************************************/
/*                                                                           */
/*  KHE_TWO_COMPONENT - a component of the clash graph                       */
/*                                                                           */
/*****************************************************************************/

struct khe_two_component_rec {
  KHE_TWO_GRAPH		graph;			/* enclosing graph           */
  ARRAY_KHE_TWO_NODE	nodes;			/* nodes of this component   */
  int			second_start;		/* if two-coloured           */
  LSET			assigned_times;		/* assigned times of nodes   */
  LSET			assigned_times1;	/* times of first colour     */
  LSET			assigned_times2;	/* times of second colour    */
  float			assigned_workload1;	/* workload of first colour  */
  float			assigned_workload2;     /* workload of second colour */
  ARRAY_KHE_RESOURCE	avail_assts1;		/* workable pairs of rsrcs   */
  ARRAY_KHE_RESOURCE	avail_assts2;		/* workable pairs of rsrcs   */
  int			best_asst_index;	/* index of best asst, or -1 */
};


/*****************************************************************************/
/*                                                                           */
/*  KHE_TWO_GRAPH - a clash graph for two-colouring                          */
/*                                                                           */
/*****************************************************************************/

struct khe_two_graph_rec {
  KHE_SOLN		soln;			/* enclosing solution        */
  KHE_RESOURCE		resource1;		/* first resource            */
  KHE_RESOURCE		resource2;		/* second resource           */
  bool			preserve_invariant;	/* true if want to do this   */
  ARRAY_KHE_TWO_NODE	nodes;			/* nodes of the clash graph  */
  ARRAY_KHE_TWO_COMPONENT components;		/* components of clash graph */

  /* used when searching */
  KHE_TRANSACTION	curr_t;			/* holds current assignments */
  KHE_TRANSACTION	best_t;			/* holds best assignments    */
  KHE_COST		best_cost;		/* best cost when searching  */
  int			node_count;		/* node count while searching*/
};


/*****************************************************************************/
/*                                                                           */
/*  Submodule "nodes"                                                        */
/*                                                                           */
/*****************************************************************************/

/*****************************************************************************/
/*                                                                           */
/*  void KheTwoNodeAddTask(KHE_TWO_NODE tn, KHE_TASK task)                   */
/*                                                                           */
/*  Add task to tn.                                                          */
/*                                                                           */
/*****************************************************************************/

static void KheTwoNodeAddTask(KHE_TWO_NODE tn, KHE_TASK task)
{
  KHE_MEET meet;  int index, durn, i;
  MArrayAddLast(tn->tasks, task);
  meet = KheTaskMeet(task);
  if( meet != NULL && KheMeetAsstTime(meet) != NULL )
  {
    durn = KheMeetDuration(meet);
    index = KheTimeIndex(KheMeetAsstTime(meet));
    for( i = 0;  i < durn;  i++ )
      LSetInsert(&tn->assigned_times, index + i);
  }
  tn->workload += KheTaskWorkload(task);
}


/*****************************************************************************/
/*                                                                           */
/*  KHE_TWO_NODE KheTwoNodeMake(KHE_TASK task)                               */
/*                                                                           */
/*  Make a node of the clash graph containing task as its first task.        */
/*                                                                           */
/*****************************************************************************/

static KHE_TWO_NODE KheTwoNodeMake(KHE_TASK task)
{
  KHE_TWO_NODE res;
  MMake(res);
  MArrayInit(res->tasks);
  res->assigned_times = LSetNew();
  res->workload = 0.0;
  res->resource = NULL;
  KheTwoNodeAddTask(res, task);
  return res;
}


/*****************************************************************************/
/*                                                                           */
/*  void KheTwoNodeDelete(KHE_TWO_NODE tn)                                   */
/*                                                                           */
/*  Delete tn, freeing its memory.                                           */
/*                                                                           */
/*****************************************************************************/

static void KheTwoNodeDelete(KHE_TWO_NODE tn)
{
  MArrayFree(tn->tasks);
  LSetFree(tn->assigned_times);
  MFree(tn);
}


/*****************************************************************************/
/*                                                                           */
/*  bool KheTasksShareAvoidSplitMonitor(KHE_TASK task1, KHE_TASK task2)      */
/*                                                                           */
/*  Return true if task1 and task2 have an avoid split assignments monitor   */
/*  in common.                                                               */
/*                                                                           */
/*****************************************************************************/

static bool KheTasksShareAvoidSplitMonitor(KHE_TASK task1, KHE_TASK task2)
{
  KHE_EVENT_RESOURCE er1, er2;  KHE_SOLN soln;  KHE_MONITOR m1, m2;  int i, j;
  er1 = KheTaskEventResource(task1);
  er2 = KheTaskEventResource(task2);
  if( er1 != NULL && er2 != NULL )
  {
    soln = KheTaskSoln(task1);
    for( i = 0;  i < KheEventResourceMonitorCount(soln, er1);  i++ )
    {
      m1 = KheEventResourceMonitor(soln, er1, i);
      if( KheMonitorTag(m1) == KHE_AVOID_SPLIT_ASSIGNMENTS_MONITOR_TAG &&
	  KheConstraintWeight(KheMonitorConstraint(m1)) > 0 )
	for( j = 0;  j < KheEventResourceMonitorCount(soln, er2);  j++ )
	{
	  m2 = KheEventResourceMonitor(soln, er2, j);
	  if( m1 == m2 )
	    return true;
	}
    }
  }
  return false;
}


/*****************************************************************************/
/*                                                                           */
/*  bool KheTwoNodeAcceptsTask(KHE_TWO_NODE tn, KHE_TASK task)               */
/*                                                                           */
/*  Return true if tn accepts task, either because task shares a leader      */
/*  task with a task of tn, or because it shares an avoid split assignments  */
/*  monitor with a task of tn.                                               */
/*                                                                           */
/*****************************************************************************/

static bool KheTwoNodeAcceptsTask(KHE_TWO_NODE tn, KHE_TASK task)
{
  KHE_TASK task2;  int i;
  MArrayForEach(tn->tasks, &task2, &i)
    if( KheTaskLeader(task2) == KheTaskLeader(task) ||
	KheTasksShareAvoidSplitMonitor(task2, task) )
      return true;
  return false;
}


/*****************************************************************************/
/*                                                                           */
/*  bool KheTwoNodeAssign(KHE_TWO_NODE tn, KHE_RESOURCE r)                   */
/*                                                                           */
/*  If all of tn's leader tasks can be assigned r, assign them all and       */
/*  return true.  Otherwise return false, leaving tn in an indeterminate     */
/*  state that will have to be cleaned up using a transaction.               */
/*                                                                           */
/*  This function assumes that the leader tasks are unassigned.              */
/*                                                                           */
/*****************************************************************************/

static bool KheTwoNodeAssign(KHE_TWO_NODE tn, KHE_RESOURCE r)
{
  KHE_TASK task;  int i;
  MArrayForEach(tn->tasks, &task, &i)
    if( KheTaskIsLeader(task) && !KheTaskAssignResource(task, r) )
      return false;
  return true;
}


/*****************************************************************************/
/*                                                                           */
/*  void KheTwoNodeUnAssign(KHE_TWO_NODE tn)                                 */
/*                                                                           */
/*  Ensure that tn's leader tasks are all unassigned.                        */
/*                                                                           */
/*  NB it is vital to deassign in reverse order, so that transactions can    */
/*  see that previous assignments are being undone.                          */
/*                                                                           */
/*****************************************************************************/

static void KheTwoNodeUnAssign(KHE_TWO_NODE tn)
{
  KHE_TASK task;  int i;
  MArrayForEachReverse(tn->tasks, &task, &i)
    if( KheTaskIsLeader(task) && KheTaskAsst(task) != NULL )
      KheTaskUnAssign(task);
}


/*****************************************************************************/
/*                                                                           */
/* void KheTwoNodeDebug(KHE_TWO_NODE tn, int verbosity, int indent, FILE *fp)*/
/*                                                                           */
/*  Debug print of tn onto fp with the given verbosity and indent.           */
/*                                                                           */
/*****************************************************************************/

static void KheTwoNodeDebug(KHE_TWO_NODE tn, int verbosity,
  int indent, FILE *fp)
{
  KHE_TASK task;  int i;
  if( verbosity >= 1 && indent >= 0 )
  {
    fprintf(fp, "%*s[ TwoNode wk%.1f %s", indent, "", tn->workload,
      LSetShow(tn->assigned_times));
    if( tn->resource != NULL )
      fprintf(fp, " := %s", KheResourceId(tn->resource) == NULL ? "-" :
	KheResourceId(tn->resource));
    fprintf(fp, "\n");
    if( verbosity >= 2 )
      MArrayForEach(tn->tasks, &task, &i)
	KheTaskDebug(task, 1, indent + 2, fp);
    fprintf(fp, "%*s]\n", indent, "");
  }
}


/*****************************************************************************/
/*                                                                           */
/*  Submodule "components"                                                   */
/*                                                                           */
/*****************************************************************************/

/*****************************************************************************/
/*                                                                           */
/*  void KheTwoComponentAddTwoNode(KHE_TWO_COMPONENT tc, KHE_TWO_NODE tn)    */
/*                                                                           */
/*  Add tn to tc.                                                            */
/*                                                                           */
/*****************************************************************************/

static void KheTwoComponentAddTwoNode(KHE_TWO_COMPONENT tc, KHE_TWO_NODE tn)
{
  MArrayAddLast(tc->nodes, tn);
  LSetUnion(&tc->assigned_times, tn->assigned_times);
}


/*****************************************************************************/
/*                                                                           */
/*  KHE_TWO_COMPONENT KheTwoComponentMake(KHE_TWO_GRAPH tg, KHE_TWO_NODE tn) */
/*                                                                           */
/*  Make a new component of the clash graph, with this node for its first    */
/*  element (a component always has at least one node).                      */
/*                                                                           */
/*****************************************************************************/

static KHE_TWO_COMPONENT KheTwoComponentMake(KHE_TWO_GRAPH tg, KHE_TWO_NODE tn)
{
  KHE_TWO_COMPONENT res;
  MMake(res);
  res->graph = tg;
  MArrayInit(res->nodes);
  res->second_start = -1;
  res->assigned_times = LSetNew();
  res->assigned_times1 = LSetNew();
  res->assigned_times2 = LSetNew();
  res->assigned_workload1 = 0.0;
  res->assigned_workload2 = 0.0;
  MArrayInit(res->avail_assts1);
  MArrayInit(res->avail_assts2);
  res->best_asst_index = -1;
  KheTwoComponentAddTwoNode(res, tn);
  return res;
}


/*****************************************************************************/
/*                                                                           */
/*  void KheTwoComponentDelete(KHE_TWO_COMPONENT tc)                         */
/*                                                                           */
/*  Delete tc.                                                               */
/*                                                                           */
/*****************************************************************************/

static void KheTwoComponentDelete(KHE_TWO_COMPONENT tc)
{
  /* nodes are deleted separately */
  MArrayFree(tc->nodes);
  LSetFree(tc->assigned_times);
  MArrayFree(tc->avail_assts1);
  MArrayFree(tc->avail_assts2);
  MFree(tc);
}


/*****************************************************************************/
/*                                                                           */
/*  bool KheTwoComponentAcceptsTwoNode(KHE_TWO_COMPONENT tc, KHE_TWO_NODE tn)*/
/*                                                                           */
/*  Return true if tc accepts tn, because tn overlaps in time with a node    */
/*  of tc.                                                                   */
/*                                                                           */
/*****************************************************************************/

static bool KheTwoComponentAcceptsTwoNode(KHE_TWO_COMPONENT tc, KHE_TWO_NODE tn)
{
  return !LSetDisjoint(tc->assigned_times, tn->assigned_times);
}


/*****************************************************************************/
/*                                                                           */
/*  bool KheTwoComponentColour(KHE_TWO_COMPONENT tc)                         */
/*                                                                           */
/*  Find a two-colouring of tc.  There is at most one unique way to do       */
/*  this, since the component is connected.  If there is no way to do        */
/*  it (because the component has an odd cycle), return false.               */
/*                                                                           */
/*  The colouring is recorded by placing the nodes coloured with the first   */
/*  colour at the left, and the nodes coloured with the second colour to the */
/*  right; tc->second_start is the index of the first second colour node.    */
/*                                                                           */
/*****************************************************************************/

static bool KheTwoComponentColour(KHE_TWO_COMPONENT tc)
{
  KHE_TWO_NODE tn, tmp;  int i, j, k;  bool progressing;

  i = 1;
  MAssert(MArraySize(tc->nodes) >= 1, "KheTwoComponentColour internal error");
  tn = MArrayFirst(tc->nodes);
  LSetUnion(&tc->assigned_times1, tn->assigned_times);
  tc->assigned_workload1 = tn->workload;
  j = MArraySize(tc->nodes) - 1;

  /* invariant: nodes[0..i-1] have colour 1 and assigned_times1; nodes[j+1..] */
  /* have colour 2 and assigned_times2; nodes[i..j] are uncoloured */
  while( i <= j )
  {
    progressing = false;
    for( k = i;  k <= j;  k++ )
    {
      tn = MArrayGet(tc->nodes, k);
      if( !LSetDisjoint(tc->assigned_times1, tn->assigned_times) )
      {
	if( !LSetDisjoint(tc->assigned_times2, tn->assigned_times) )
	{
	  if( DEBUG7 )
	  {
	    fprintf(stderr, "  KheTwoComponentColour fail at [%d, %d, %d]:\n",
	      i, k, j);
	    KheTwoNodeDebug(tn, 4, 4, stderr);
	    fprintf(stderr, "    left:  %s\n", LSetShow(tc->assigned_times1));
	    fprintf(stderr, "    right: %s\n", LSetShow(tc->assigned_times2));
	  }
	  return false;
	}
	else
	{
	  MArraySwap(tc->nodes, k, j, tmp);
	  LSetUnion(&tc->assigned_times2, tn->assigned_times);
	  tc->assigned_workload2 += tn->workload;
	  j--;
	  progressing = true;
	}
      }
      else if( !LSetDisjoint(tc->assigned_times2, tn->assigned_times) )
      {
	MArraySwap(tc->nodes, k, i, tmp);
	LSetUnion(&tc->assigned_times1, tn->assigned_times);
	tc->assigned_workload1 += tn->workload;
	i++;
	progressing = true;
      }
    }
    MAssert(progressing, "KheTwoComponentColour internal error");
  }
  tc->second_start = i;
  return true;
}


/*****************************************************************************/
/*                                                                           */
/*  bool KheComponentDistinct(KHE_TWO_COMPONENT tc)                          */
/*                                                                           */
/*  Return true if the two halves of tc have distinct times or distinct      */
/*  workloads.                                                               */
/*                                                                           */
/*****************************************************************************/

static bool KheComponentDistinct(KHE_TWO_COMPONENT tc)
{
  return tc->assigned_workload1 != tc->assigned_workload2 ||
    !LSetEqual(tc->assigned_times1, tc->assigned_times2);
}


/*****************************************************************************/
/*                                                                           */
/*  bool KheTwoComponentAssign(KHE_TWO_COMPONENT tc, KHE_RESOURCE r1,        */
/*    KHE_RESOURCE r2, KHE_TRANSACTION t)                                    */
/*                                                                           */
/*  Try assigning r1 to the first component and r2 to the second.  If        */
/*  successful, leave the assignments as they are and return true.           */
/*  Otherwise return false, leaving the component unassigned.                */
/*                                                                           */
/*  Parameter t is a scratch transaction object.                             */
/*                                                                           */
/*****************************************************************************/

static bool KheTwoComponentAssign(KHE_TWO_COMPONENT tc, KHE_RESOURCE r1,
  KHE_RESOURCE r2, KHE_TRANSACTION t)
{
  KHE_TWO_NODE tn;  int i, init_count;  bool invt;
  invt = tc->graph->preserve_invariant;
  KheInvariantTransactionBegin(t, &init_count, invt);
  MArrayForEach(tc->nodes, &tn, &i)
    if( !KheTwoNodeAssign(tn, i < tc->second_start ? r1 : r2) )
      return KheInvariantTransactionEnd(t, &init_count, invt, false);
  return KheInvariantTransactionEnd(t, &init_count, invt, true);
}


/*****************************************************************************/
/*                                                                           */
/*  void KheTwoComponentUnAssign(KHE_TWO_COMPONENT tc)                       */
/*                                                                           */
/*  Ensure that all the tasks of all the nodes of tc are unassigned.         */
/*                                                                           */
/*  NB it is vital to deassign in reverse order, so that transactions can    */
/*  see that previous assignments are being undone.                          */
/*                                                                           */
/*****************************************************************************/

static void KheTwoComponentUnAssign(KHE_TWO_COMPONENT tc)
{
  KHE_TWO_NODE tn;  int i;
  MArrayForEachReverse(tc->nodes, &tn, &i)
    KheTwoNodeUnAssign(tn);
}


/*****************************************************************************/
/*                                                                           */
/*  void KheTwoComponentAddAsstOption(KHE_TWO_COMPONENT tc,                  */
/*    KHE_RESOURCE r1, KHE_RESOURCE r2)                                      */
/*                                                                           */
/*  Record the fact that KheTwoComponentAssign(tc, r1, r2, t) is worth a    */
/*  try when searching for an optimal assignment of all components.          */
/*                                                                           */
/*****************************************************************************/

static void KheTwoComponentAddAsstOption(KHE_TWO_COMPONENT tc,
  KHE_RESOURCE r1, KHE_RESOURCE r2)
{
  MArrayAddLast(tc->avail_assts1, r1);
  MArrayAddLast(tc->avail_assts2, r2);
}


/*****************************************************************************/
/*                                                                           */
/*  int KheTwoComponentAvailAsstsCmp(const void *t1, const void *t2)         */
/*                                                                           */
/*  Comparison function for sorting components by increasing number of       */
/*  available assignments.                                                   */
/*                                                                           */
/*****************************************************************************/

static int KheTwoComponentAvailAsstsCmp(const void *t1, const void *t2)
{
  KHE_TWO_COMPONENT tc1 = * (KHE_TWO_COMPONENT *) t1;
  KHE_TWO_COMPONENT tc2 = * (KHE_TWO_COMPONENT *) t2;
  return MArraySize(tc1->avail_assts1) - MArraySize(tc2->avail_assts1);
}


/*****************************************************************************/
/*                                                                           */
/*  void KheTwoComponentDebug(KHE_TWO_COMPONENT tc, int verbosity,           */
/*    int indent, FILE *fp)                                                  */
/*                                                                           */
/*  Debug print of tc onto fp with the given verbosity and indent.           */
/*                                                                           */
/*****************************************************************************/

static void KheTwoComponentDebug(KHE_TWO_COMPONENT tc, int verbosity,
  int indent, FILE *fp)
{
  KHE_TWO_NODE tn;  int i;  KHE_RESOURCE r1, r2;
  if( verbosity >= 1 && indent >= 0 )
  {
    fprintf(fp, "%*s[ Component %s\n", indent, "",
      LSetShow(tc->assigned_times));
    if( MArraySize(tc->avail_assts1) > 0 )
    {
      fprintf(fp, "%*s  avail assts: ", indent, "");
      for( i = 0;  i < MArraySize(tc->avail_assts1);  i++ )
      {
	if( i > 0 )
	  fprintf(fp, ", ");
	r1 = MArrayGet(tc->avail_assts1, i);
	r2 = MArrayGet(tc->avail_assts2, i);
	fprintf(fp, "(%s, %s)",
	  KheResourceId(r1) == NULL ? "-" : KheResourceId(r1),
	  KheResourceId(r2) == NULL ? "-" : KheResourceId(r2));
      }
      fprintf(fp, "\n");
    }
    if( tc->second_start != -1 )
    {
      fprintf(stderr, "%*s  left:  wk%.1f %s\n", indent, "",
	tc->assigned_workload1, LSetShow(tc->assigned_times1));
      fprintf(stderr, "%*s  right: wk%.1f %s\n", indent, "",
	tc->assigned_workload2, LSetShow(tc->assigned_times2));
    }
    MArrayForEach(tc->nodes, &tn, &i)
    {
      if( tc->second_start == i )
	fprintf(fp, "%*s  -----\n", indent, "");
      KheTwoNodeDebug(tn, verbosity, indent + 2, fp);
    }
    fprintf(fp, "%*s]\n", indent, "");
  }
}


/*****************************************************************************/
/*                                                                           */
/*  Submodule "graph"                                                        */
/*                                                                           */
/*****************************************************************************/

/*****************************************************************************/
/*                                                                           */
/*  void KheTwoGraphDelete(KHE_TWO_GRAPH tg)                                 */
/*                                                                           */
/*  Delete tg and everything in it.                                          */
/*                                                                           */
/*****************************************************************************/

static void KheTwoGraphDelete(KHE_TWO_GRAPH tg)
{
  while( MArraySize(tg->nodes) > 0 )
    KheTwoNodeDelete(MArrayRemoveLast(tg->nodes));
  MArrayFree(tg->nodes);
  while( MArraySize(tg->components) > 0 )
    KheTwoComponentDelete(MArrayRemoveLast(tg->components));
  MArrayFree(tg->components);
  KheTransactionDelete(tg->curr_t);
  KheTransactionDelete(tg->best_t);
  MFree(tg);
}


/*****************************************************************************/
/*                                                                           */
/*  void KheTwoGraphAddTask(KHE_TWO_GRAPH tg, KHE_TASK task)                 */
/*                                                                           */
/*  Add task to tg, by either adding it to an existing node, or by making    */
/*  a new node.                                                              */
/*                                                                           */
/*****************************************************************************/

static void KheTwoGraphAddTask(KHE_TWO_GRAPH tg, KHE_TASK task)
{
  KHE_TWO_NODE tn;  int i;

  /* add task to a node that will accept it, if any */
  MArrayForEach(tg->nodes, &tn, &i)
    if( KheTwoNodeAcceptsTask(tn, task) )
    {
      KheTwoNodeAddTask(tn, task);
      return;
    }

  /* alternatively, make a new node containing task */
  tn = KheTwoNodeMake(task);
  MArrayAddLast(tg->nodes, tn);
}


/*****************************************************************************/
/*                                                                           */
/*  void KheTwoGraphAddNodeToComponent(KHE_TWO_GRAPH tg, KHE_TWO_NODE tn)    */
/*                                                                           */
/*  Add tn to a component, by either adding it to an existing component,     */
/*  or by making a new component.                                            */
/*                                                                           */
/*****************************************************************************/

static void KheTwoGraphAddNodeToComponent(KHE_TWO_GRAPH tg, KHE_TWO_NODE tn)
{
  KHE_TWO_COMPONENT tc;  int i;

  /* add node to a component that will accept it, if any */
  MArrayForEach(tg->components, &tc, &i)
    if( KheTwoComponentAcceptsTwoNode(tc, tn) )
    {
      KheTwoComponentAddTwoNode(tc, tn);
      return;
    }

  /* alternatively, make a new node containing task */
  tc = KheTwoComponentMake(tg, tn);
  MArrayAddLast(tg->components, tc);
}


/*****************************************************************************/
/*                                                                           */
/*  KHE_TWO_GRAPH KheTwoGraphMake(KHE_SOLN soln, KHE_RESOURCE r1,            */
/*    KHE_RESOURCE r2, bool preserve_invariant)                              */
/*                                                                           */
/*  Make a two-graph with these attributes.                                  */
/*                                                                           */
/*****************************************************************************/

static KHE_TWO_GRAPH KheTwoGraphMake(KHE_SOLN soln, KHE_RESOURCE r1,
  KHE_RESOURCE r2, bool preserve_invariant)
{
  KHE_TWO_GRAPH res;  int i;  KHE_TWO_NODE tn;
  MAssert(r1 != r2, "KheTwoGraphMake: r1 == r2");

  /* make the basic object */
  MMake(res);
  res->soln = soln;
  res->resource1 = r1;
  res->resource2 = r2;
  res->preserve_invariant = preserve_invariant;
  MArrayInit(res->nodes);
  MArrayInit(res->components);
  res->curr_t = KheTransactionMake(soln);
  res->best_t = KheTransactionMake(soln);
  res->best_cost = KheSolnCost(soln);  /* must do better than this */
  res->node_count = 0;

  /* add nodes to res containing all tasks assigned to r1 and r2 */
  for( i = 0;  i < KheResourceAssignedTaskCount(soln, r1);  i++ )
    KheTwoGraphAddTask(res, KheResourceAssignedTask(soln, r1, i));
  for( i = 0;  i < KheResourceAssignedTaskCount(soln, r2);  i++ )
    KheTwoGraphAddTask(res, KheResourceAssignedTask(soln, r2, i));

  /* add components to res */
  MArrayForEach(res->nodes, &tn, &i)
    KheTwoGraphAddNodeToComponent(res, tn);
  return res;
}


/*****************************************************************************/
/*                                                                           */
/*  void KheTwoGraphDebug(KHE_TWO_GRAPH tg, int verbosity, int indent,       */
/*    FILE *fp)                                                              */
/*                                                                           */
/*  Debug print of tg onto fp with the given verbosity and indent.           */
/*                                                                           */
/*****************************************************************************/

void KheTwoGraphDebug(KHE_TWO_GRAPH tg, int verbosity, int indent, FILE *fp)
{
  KHE_TWO_COMPONENT tc;  int i;
  if( verbosity >= 1 && indent >= 0 )
  {
    fprintf(fp, "%*s[ TwoGraph(%s, %s, %s)\n", indent, "",
      KheResourceId(tg->resource1)==NULL ? "-" : KheResourceId(tg->resource1),
      KheResourceId(tg->resource2)==NULL ? "-" : KheResourceId(tg->resource2),
      tg->preserve_invariant ? "true" : "false");
    MArrayForEach(tg->components, &tc, &i)
      KheTwoComponentDebug(tc, verbosity, indent + 2, fp);
    fprintf(fp, "%*s]\n", indent, "");
  }
}


/*****************************************************************************/
/*                                                                           */
/*  Submodule "exported functions"                                           */
/*                                                                           */
/*****************************************************************************/

/*****************************************************************************/
/*                                                                           */
/*  void KheTwoGraphTreeSearch(KHE_TWO_GRAPH tg, int pos, KHE_TRANSACTION t) */
/*                                                                           */
/*  Assign tg->components[pos...] in as many different ways as allowed by    */
/*  the node limit, setting tg->best_cost and tg->best_t to the best.        */
/*  Parameter t is a scratch transaction object.                             */
/*                                                                           */
/*****************************************************************************/

static void KheTwoGraphTreeSearch(KHE_TWO_GRAPH tg, int pos, KHE_TRANSACTION t)
{
  KHE_TWO_COMPONENT tc;  int i;  KHE_RESOURCE r1, r2;
  if( pos >= MArraySize(tg->components) )
  {
    /* off the end, see whether we have a new best */
    if( KheSolnCost(tg->soln) < tg->best_cost )
    {
      if( DEBUG6 )
      {
	fprintf(stderr, "  new best (%.4f -> %.4f):\n",
	  KheCostShow(tg->best_cost), KheCostShow(KheSolnCost(tg->soln)));
	KheTransactionDebug(tg->curr_t, 1, 4, stderr);
      }
      KheTransactionCopy(tg->curr_t, tg->best_t);
      tg->best_cost = KheSolnCost(tg->soln);
    }
  }
  else
  {
    /* assign tc in all possible ways, unless restricted by node limit */
    tc = MArrayGet(tg->components, pos);
    for( i = 0;  i < MArraySize(tc->avail_assts1);  i++ )
      if( tg->node_count++ < KHE_TWO_MAX_NODES || i == 0 )
      {
	r1 = MArrayGet(tc->avail_assts1, i);
	r2 = MArrayGet(tc->avail_assts2, i);
        if( KheTwoComponentAssign(tc, r1, r2, t) )
	{
          KheTwoGraphTreeSearch(tg, pos + 1, t);
          KheTwoComponentUnAssign(tc);
	}
      }
  }
}


/*****************************************************************************/
/*                                                                           */
/*  bool KheTwoColourReassign(KHE_SOLN soln, KHE_RESOURCE r1,                */
/*    KHE_RESOURCE r2, bool preserve_invariant)                              */
/*                                                                           */
/*  Redistribute the tasks assigned to r1 and r2 in soln, so as to reduce    */
/*  solution cost if possible.                                               */
/*                                                                           */
/*****************************************************************************/

bool KheTwoColourReassign(KHE_SOLN soln, KHE_RESOURCE r1,
  KHE_RESOURCE r2, bool preserve_invariant)
{
  KHE_COST init_cost;  KHE_TWO_GRAPH tg;  KHE_TWO_COMPONENT tc;  int i;
  KHE_TRANSACTION init_t, t;
  init_cost = KheSolnCost(soln);
  if( DEBUG1 )
    fprintf(stderr, "  [ KheTwoColourReassign(soln, %s, %s, %s) init %.4f\n",
      KheResourceId(r1), KheResourceId(r2),
      preserve_invariant ? "true" : "false", KheCostShow(init_cost));

  /* build the clash graph remove components that can't be two-coloured */
  tg = KheTwoGraphMake(soln, r1, r2, preserve_invariant);
  MArrayForEach(tg->components, &tc, &i)
    if( !KheTwoComponentColour(tc) )
    {
      if( DEBUG1 )
      {
	fprintf(stderr, "    deleting uncolourable component:\n");
	KheTwoComponentDebug(tc, 2, 4, stderr);
      }
      KheTwoComponentDelete(tc);
      MArrayRemove(tg->components, i);
      i--;
    }

  /* remove all assignments, but save them in transaction init_t */
  init_t = KheTransactionMake(soln);
  KheTransactionBegin(init_t);
  MArrayForEach(tg->components, &tc, &i)
    KheTwoComponentUnAssign(tc);
  KheTransactionEnd(init_t);

  /* for each component, record the options for assigning it */
  t = KheTransactionMake(soln);
  MArrayForEach(tg->components, &tc, &i)
  {
    if( KheTwoComponentAssign(tc, r1, r2, t) )
    {
      KheTwoComponentAddAsstOption(tc, r1, r2);
      KheTwoComponentUnAssign(tc);
      if( KheComponentDistinct(tc) && KheTwoComponentAssign(tc, r2, r1, t) )
      {
	KheTwoComponentAddAsstOption(tc, r2, r1);
	KheTwoComponentUnAssign(tc);
      }
    }
    else if( KheTwoComponentAssign(tc, r2, r1, t) )
    {
      KheTwoComponentAddAsstOption(tc, r2, r1);
      KheTwoComponentUnAssign(tc);
    }
  }

  /* sort the components by increasing number of available assignments */
  MArraySort(tg->components, &KheTwoComponentAvailAsstsCmp);
  if( DEBUG1 )
    KheTwoGraphDebug(tg, 2, 4, stderr);

  /* initialize for the tree search, and do it */
  tg->node_count = 0;
  MArrayForEach(tg->components, &tc, &i)
    tc->best_asst_index = -1;
  KheTransactionBegin(tg->curr_t);
  KheTwoGraphTreeSearch(tg, 0, t);
  KheTransactionEnd(tg->curr_t);

  if( tg->best_cost < init_cost )
  {
    /* apply best_t */
    KheTransactionRedo(tg->best_t);
    MAssert(KheSolnCost(soln) == tg->best_cost,
      "KheTwoColourReassign internal error 1");
    if( DEBUG1 )
      fprintf(stderr,"  ] KheTwoColourReassign returning true (%.4f -> %.4f)\n",
	KheCostShow(init_cost), KheCostShow(tg->best_cost));
    KheTwoGraphDelete(tg);
    return true;
  }
  else
  {
    /* return to init_t */
    KheTransactionUndo(init_t);
    MAssert(KheSolnCost(soln) == init_cost,
      "KheTwoColourReassign internal error 2");
    if( DEBUG1 )
      fprintf(stderr,"  ] KheTwoColourReassign returning false\n");
    KheTwoGraphDelete(tg);
    return false;
  }
}


/*****************************************************************************/
/*                                                                           */
/*  void KheTwoColourRepairSplitAssignments(KHE_TASKING tasking,             */
/*    bool preserve_invariant)                                               */
/*                                                                           */
/*  For each pair of resources involved in a split assigning in tasking,     */
/*  call KheTwoColourReassign.                                               */
/*                                                                           */
/*****************************************************************************/

void KheTwoColourRepairSplitAssignments(KHE_TASKING tasking,
  bool preserve_invariant)
{
  int i, j, k, pos;  KHE_TASK task;  KHE_EVENT_RESOURCE er;  KHE_SOLN soln;
  ARRAY_KHE_AVOID_SPLIT_ASSIGNMENTS_MONITOR defects;  KHE_RESOURCE r1, r2;
  KHE_MONITOR m;  KHE_AVOID_SPLIT_ASSIGNMENTS_MONITOR asam;

  if( DEBUG1 )
    fprintf(stderr, "[ KheTwoColourRepairSplitAssignments(tasking, %s)\n",
      preserve_invariant ? "true" : "false");

  /* find the avoid split assignments defects */
  soln = KheTaskingSoln(tasking);
  MArrayInit(defects);
  for( i = 0;  i < KheTaskingTaskCount(tasking);  i++ )
  {
    task = KheTaskingTask(tasking, i);
    er = KheTaskEventResource(task);
    if( er != NULL )
      for( j = 0;  j < KheEventResourceMonitorCount(soln, er);  j++ )
      {
	m = KheEventResourceMonitor(soln, er, j);
	if( KheMonitorTag(m) == KHE_AVOID_SPLIT_ASSIGNMENTS_MONITOR_TAG &&
            KheMonitorCost(m) > 0 )
	{
	  asam = (KHE_AVOID_SPLIT_ASSIGNMENTS_MONITOR) m;
	  if( !MArrayContains(defects, asam, &pos) )
	    MArrayAddLast(defects, asam);
	}
      }
  }

  /* call KheTwoColourReassign on all pairs of resources involved in splits */
  MArrayForEach(defects, &asam, &i)
  {
    RESTART_ASAM:
    if( DEBUG1 )
    {
      fprintf(stderr, "  repairing ");
      KheMonitorDebug((KHE_MONITOR) asam, 1, 0, stderr);
    }
    for( j = 0;  j < KheAvoidSplitAssignmentsMonitorResourceCount(asam);  j++ )
    {
      /* can't assign r1 here, sometimes it gives r1 == r2 owing to changes */
      for( k=j+1; k < KheAvoidSplitAssignmentsMonitorResourceCount(asam); k++ )
      {
	r1 = KheAvoidSplitAssignmentsMonitorResource(asam, j);
	r2 = KheAvoidSplitAssignmentsMonitorResource(asam, k);
	if( KheTwoColourReassign(soln, r1, r2, preserve_invariant) )
	  goto RESTART_ASAM;
      }
    }
  }

  if( DEBUG1 )
    fprintf(stderr, "] KheTwoColourRepairSplitAssignments returning\n");
}
