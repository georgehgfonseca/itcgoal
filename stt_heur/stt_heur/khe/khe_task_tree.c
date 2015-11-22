
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
/*  FILE:         khe_task_tree.c                                           */
/*  DESCRIPTION:  Operations for constructing and reorganizing task trees    */
/*                                                                           */
/*****************************************************************************/
#include "khe.h"
#include "m.h"

#define DEBUG1 0
#define DEBUG2 0
#define DEBUG3 0
#define DEBUG4 0
#define DEBUG5 0

typedef MARRAY(KHE_RESOURCE_TYPE) ARRAY_KHE_RESOURCE_TYPE;
typedef MARRAY(KHE_CONSTRAINT) ARRAY_KHE_CONSTRAINT;
typedef MARRAY(KHE_TASK) ARRAY_KHE_TASK;


/*****************************************************************************/
/*                                                                           */
/*  KHE_PARTITION - one resource partition                                   */
/*                                                                           */
/*****************************************************************************/

typedef struct khe_partition_rec {
  KHE_RESOURCE_GROUP		resource_group;		/* the resource grp  */
  int				duration_supply;	/* duration supply   */
  int				duration_demand;	/* duration demand   */
} *KHE_PARTITION;

typedef MARRAY(KHE_PARTITION) ARRAY_KHE_PARTITION;


/*****************************************************************************/
/*                                                                           */
/*  KHE_MULTI_TASK - task assignable resources from more than one partition  */
/*                                                                           */
/*****************************************************************************/

typedef struct khe_multi_task_rec {
  KHE_TASK			task;			/* the task          */
  ARRAY_KHE_PARTITION		partitions;		/* its partitions    */
  KHE_PARTITION			dominant_partition;	/* dominates         */
} *KHE_MULTI_TASK;

typedef MARRAY(KHE_MULTI_TASK) ARRAY_KHE_MULTI_TASK;


/*****************************************************************************/
/*                                                                           */
/*  KHE_PARTITION_SOLVER - a partition solver.                               */
/*                                                                           */
/*****************************************************************************/

typedef struct khe_partition_solver_rec {
  KHE_TASKING			tasking;		/* the tasking       */
  bool				preserve_invariant;	/* flag from caller  */
  ARRAY_KHE_PARTITION		partitions;		/* the partitions    */
  ARRAY_KHE_MULTI_TASK		multi_tasks;		/* multi-partn tasks */
} *KHE_PARTITION_SOLVER;


/*****************************************************************************/
/*                                                                           */
/*  Submodule "partitions" (private)                                         */
/*                                                                           */
/*****************************************************************************/

/*****************************************************************************/
/*                                                                           */
/*  int KheResourceGroupDurationSupply(KHE_RESOURCE_GROUP rg, KHE_SOLN soln) */
/*                                                                           */
/*  Return the duration supply of rg.                                        */
/*                                                                           */
/*****************************************************************************/

static int KheResourceGroupDurationSupply(KHE_RESOURCE_GROUP rg, KHE_SOLN soln)
{
  int i, j, cycle_length, r_supply, res;  KHE_RESOURCE r;  KHE_MONITOR m;
  if( DEBUG2 )
  {
    fprintf(stderr, "[ KheResourceGroupDurationSupply(");
    KheResourceGroupDebug(rg, 1, -1, stderr);
    fprintf(stderr, ", soln)\n");
  }

  cycle_length = KheInstanceTimeCount(KheSolnInstance(soln));
  if( DEBUG2 )
    fprintf(stderr, "  cycle_length %d\n", cycle_length);
  res = 0;
  for( i = 0;  i < KheResourceGroupResourceCount(rg);  i++ )
  {
    r = KheResourceGroupResource(rg, i);
    r_supply = cycle_length;
    for( j = 0;  j < KheResourceMonitorCount(soln, r);  j++ )
    {
      m = KheResourceMonitor(soln, r, j);
      if( KheMonitorTag(m) == KHE_WORKLOAD_DEMAND_MONITOR_TAG )
	r_supply--;
    }
    if( DEBUG2 )
      fprintf(stderr, "  supply %2d from %s\n", r_supply,
	KheResourceId(r) == NULL ? "-" : KheResourceId(r));
    res += r_supply;
  }
  if( DEBUG2 )
    fprintf(stderr, "] KheResourceGroupDurationSupply returning %d\n", res);
  return res;
}


/*****************************************************************************/
/*                                                                           */
/*  KHE_PARTITION KhePartitionMake(KHE_RESOURCE_GROUP rg, KHE_SOLN soln)     */
/*                                                                           */
/*  Make a partition object for this resource group, initially with the      */
/*  correct duration supply, but with 0.0 for the duration demand.           */
/*                                                                           */
/*****************************************************************************/

static KHE_PARTITION KhePartitionMake(KHE_RESOURCE_GROUP rg, KHE_SOLN soln)
{
  KHE_PARTITION res;
  MMake(res);
  res->resource_group = rg;
  res->duration_supply = KheResourceGroupDurationSupply(rg, soln);
  res->duration_demand = 0;
  return res;
}


/*****************************************************************************/
/*                                                                           */
/*  float KhePartitionAverageAvailableDuration(KHE_PARTITION p)              */
/*                                                                           */
/*  Return the average_available duration of p.                              */
/*                                                                           */
/*****************************************************************************/

static float KhePartitionAverageAvailableDuration(KHE_PARTITION p)
{
  int num = KheResourceGroupResourceCount(p->resource_group);
  return num == 0 ? 0.0 :
    (float) (p->duration_supply - p->duration_demand) / num;
}


/*****************************************************************************/
/*                                                                           */
/*  void KhePartitionDecreasingAvailCmp(const void *t1, const void *t2)      */
/*                                                                           */
/*  Comparison function for sorting an array of partition objects by         */
/*  decreasing average availability.                                         */
/*                                                                           */
/*****************************************************************************/

static int KhePartitionDecreasingAvailCmp(const void *t1, const void *t2)
{
  KHE_PARTITION p1 = * (KHE_PARTITION *) t1;
  KHE_PARTITION p2 = * (KHE_PARTITION *) t2;
  int count1 = KheResourceGroupResourceCount(p1->resource_group);
  int count2 = KheResourceGroupResourceCount(p2->resource_group);
  float cmp;
  cmp = KhePartitionAverageAvailableDuration(p2) -
    KhePartitionAverageAvailableDuration(p1);
  if( cmp < 0 )
    return -1;
  else if( cmp > 0 )
    return 1;
  else
    return count1 - count2;
}


/*****************************************************************************/
/*                                                                           */
/*  void KhePartitionDelete(KHE_PARTITION p)                                 */
/*                                                                           */
/*  Delete p.                                                                */
/*                                                                           */
/*****************************************************************************/

static void KhePartitionDelete(KHE_PARTITION p)
{
  MFree(p);
}


/*****************************************************************************/
/*                                                                           */
/*  bool KhePartitionAcceptsTask(KHE_PARTITION p, KHE_TASK task,             */
/*    bool preserve_invariant)                                               */
/*                                                                           */
/*  If p accepts task (and, if preserve_invariant is true, does so without   */
/*  increasing the number of unmatchable demand nodes), then reduce the      */
/*  domain of task to p and return true, otherwise change nothing and        */
/*  return false.                                                            */
/*                                                                           */
/*****************************************************************************/

static bool KhePartitionAcceptsTask(KHE_PARTITION p, KHE_TASK task,
  bool preserve_invariant, KHE_TRANSACTION t)
{
  int init_count;  bool success;
  KheInvariantTransactionBegin(t, &init_count, preserve_invariant);
  success = KheTaskTightenDomain(task, p->resource_group, true);
  return KheInvariantTransactionEnd(t, &init_count, preserve_invariant,
    success);
}


/*****************************************************************************/
/*                                                                           */
/*  void KhePartitionDebug(KHE_PARTITION p, int verbosity,                   */
/*    int indent, FILE *fp)                                                  */
/*                                                                           */
/*  Debug print of p onto fp.                                                */
/*                                                                           */
/*****************************************************************************/

static void KhePartitionDebug(KHE_PARTITION p, int verbosity,
  int indent, FILE *fp)
{
  if( verbosity >= 1 )
  {
    if( indent >= 0 )
      fprintf(fp, "%*s", indent, "");
    fprintf(fp, "[ Partition %5.2f ", KhePartitionAverageAvailableDuration(p));
    KheResourceGroupDebug(p->resource_group, 1, -1, fp);
    fprintf(fp, " (supply %d, demand %d, resources %d) ]",
      p->duration_supply, p->duration_demand,
      KheResourceGroupResourceCount(p->resource_group));
    if( indent >= 0 )
      fprintf(fp, "\n");
  }
}


/*****************************************************************************/
/*                                                                           */
/*  Submodule "multi-tasks" (private)                                        */
/*                                                                           */
/*****************************************************************************/

/*****************************************************************************/
/*                                                                           */
/*  KHE_MULTI_TASK KheMultiTaskMake(KHE_TASK task)                           */
/*                                                                           */
/*  Make a multi-task, initially with no partitions.                         */
/*                                                                           */
/*****************************************************************************/

static KHE_MULTI_TASK KheMultiTaskMake(KHE_TASK task)
{
  KHE_MULTI_TASK res;
  MMake(res);
  res->task = task;
  MArrayInit(res->partitions);
  res->dominant_partition = NULL;
  return res;
}


/*****************************************************************************/
/*                                                                           */
/*  void KheMultiTaskDelete(KHE_MULTI_TASK mt)                               */
/*                                                                           */
/*  Delete mt, freeing its memory.                                           */
/*                                                                           */
/*****************************************************************************/

static void KheMultiTaskDelete(KHE_MULTI_TASK mt)
{
  MArrayFree(mt->partitions);
  MFree(mt);
}


/*****************************************************************************/
/*                                                                           */
/*  void KheMultiTaskAddPartition(KHE_MULTI_TASK mt, KHE_PARTITION p)        */
/*                                                                           */
/*  Add p to mt, and if mt does not yet have a dominant partition, see       */
/*  whether p is it.                                                         */
/*                                                                           */
/*****************************************************************************/

static void KheMultiTaskAddPartition(KHE_MULTI_TASK mt, KHE_PARTITION p)
{
  int i, count;  KHE_RESOURCE r;  KHE_RESOURCE_GROUP rg;
  MArrayAddLast(mt->partitions, p);
  if( mt->dominant_partition == NULL )
  {
    count = 0;
    rg = KheTaskDomain(mt->task);
    for( i = 0;  i < KheResourceGroupResourceCount(p->resource_group);  i++ )
    {
      r = KheResourceGroupResource(p->resource_group, i);
      if( KheResourceGroupContains(rg, r) )
	count++;
    }
    if( 4 * count >= 3 * KheResourceGroupResourceCount(rg) )
      mt->dominant_partition = p;
  }
}


/*****************************************************************************/
/*                                                                           */
/*  void KheMultiTaskCmp(const void *t1, const void *t2)                     */
/*                                                                           */
/*  Comparison function for sorting an array of multi-tasks into decreasing  */
/*  total workload order, except tasks with dominant partitions come first.  */
/*                                                                           */
/*****************************************************************************/

static int KheMultiTaskCmp(const void *t1, const void *t2)
{
  KHE_MULTI_TASK mt1 = * (KHE_MULTI_TASK *) t1;
  KHE_MULTI_TASK mt2 = * (KHE_MULTI_TASK *) t2;
  float workload1 = KheTaskTotalWorkload(mt1->task);
  float workload2 = KheTaskTotalWorkload(mt2->task);
  if( (mt1->dominant_partition != NULL) != (mt2->dominant_partition != NULL) )
    return (mt2->dominant_partition!=NULL) - (mt1->dominant_partition!=NULL);
  else if( workload1 < workload2 )
    return 1;
  else if( workload1 > workload2 )
    return -1;
  else
    return KheTaskIndexInSoln(mt1->task) - KheTaskIndexInSoln(mt2->task);
}


/*****************************************************************************/
/*                                                                           */
/*  void KheMultiTaskDebug(KHE_MULTI_TASK mt, int verbosity,                 */
/*    int indent, FILE *fp)                                                  */
/*                                                                           */
/*  Debug print of mt onto fp with the given verbosity and indent.           */
/*                                                                           */
/*****************************************************************************/

static void KheMultiTaskDebug(KHE_MULTI_TASK mt, int verbosity,
  int indent, FILE *fp)
{
  KHE_PARTITION p;  int i;
  if( verbosity >= 1 )
  {
    if( indent >= 0 )
      fprintf(fp, "%*s", indent, "");
    fprintf(fp, "[ MultiTask ");
    KheTaskDebug(mt->task, 1, -1, fp);
    fprintf(fp, " (%d partitions", MArraySize(mt->partitions));
    if( mt->dominant_partition != NULL )
    {
      fprintf(fp, ", dominated by ");
      KheResourceGroupDebug(mt->dominant_partition->resource_group, 1, -1, fp);
    }
    fprintf(fp, ")");
    if( verbosity >= 2 )
    {
      fprintf(fp, "\n%*s  ", indent, "");
      MArrayForEach(mt->partitions, &p, &i)
      {
	if( i > 0 )
	  fprintf(fp, ", ");
	KheResourceGroupDebug(p->resource_group, 1, -1, fp);
      }
      fprintf(fp, "\n%*s]", indent, "");
    }
    else
      fprintf(fp, " ]");
    if( indent >= 0 )
      fprintf(fp, "\n");
  }
}


/*****************************************************************************/
/*                                                                           */
/*  Submodule "partition solvers" (private)                                  */
/*                                                                           */
/*****************************************************************************/

/*****************************************************************************/
/*                                                                           */
/*  bool KhePartitionSolverRetrieve(KHE_PARTITION_SOLVER ps,                 */
/*    KHE_RESOURCE_GROUP rg, KHE_PARTITION *p)                               */
/*                                                                           */
/*  If ps contains a partition for resource group rg, set *p to that         */
/*  partition and return true.  Otherwise return false.                      */
/*                                                                           */
/*****************************************************************************/

static bool KhePartitionSolverRetrieve(KHE_PARTITION_SOLVER ps,
  KHE_RESOURCE_GROUP rg, KHE_PARTITION *p)
{
  int i;
  MArrayForEach(ps->partitions, p, &i)
    if( (*p)->resource_group == rg )
      return true;
  return false;
}


/*****************************************************************************/
/*                                                                           */
/*  KHE_PARTITION_SOLVER KhePartitionSolverMake(KHE_TASKING tasking,         */
/*    bool preserve_invariant)                                               */
/*                                                                           */
/*  Make a partition solver for with these attributes.  The tasking must     */
/*  have a resource type.                                                    */
/*                                                                           */
/*****************************************************************************/

static KHE_PARTITION_SOLVER KhePartitionSolverMake(KHE_TASKING tasking,
  bool preserve_invariant)
{
  int i;  KHE_RESOURCE_GROUP rg;  KHE_PARTITION_SOLVER res;
  KHE_RESOURCE_TYPE rt;

  /* do the basic initialization */
  rt = KheTaskingResourceType(tasking);
  MAssert(rt != NULL, "KhePartitionSolverMake internal error");
  MMake(res);
  res->tasking = tasking;
  res->preserve_invariant = preserve_invariant;
  MArrayInit(res->partitions);
  MArrayInit(res->multi_tasks);

  /* add one partition for each partition of rt */
  for( i = 0;  i < KheResourceTypePartitionCount(rt);  i++ )
  {
    rg = KheResourceTypePartition(rt, i);
    MArrayAddLast(res->partitions,
      KhePartitionMake(rg, KheTaskingSoln(tasking)));
  }
  return res;
}


/*****************************************************************************/
/*                                                                           */
/*  void KhePartitionSolverClassifyTask(KHE_PARTITION_SOLVER ps,             */
/*    KHE_TASK task, KHE_PARTITION *p, KHE_MULTI_TASK *mt)                   */
/*                                                                           */
/*  Classify task into one of three categories:                              */
/*                                                                           */
/*    * Task is assignable to resources of no partitions.  Return NULL       */
/*      in both *p and *mt.                                                  */
/*                                                                           */
/*    * Task is assignable to resources of one partition (possibly because   */
/*      it is assigned).  Return that partition in *p and NULL in *mt.       */
/*                                                                           */
/*    * Task is assignable to resources of two or more partitions.  Return   */
/*      NULL in *p and a new multi-task *mt holding the task and partitions. */
/*                                                                           */
/*****************************************************************************/

static void KhePartitionSolverClassifyTask(KHE_PARTITION_SOLVER ps,
  KHE_TASK task, KHE_PARTITION *p, KHE_MULTI_TASK *mt)
{
  KHE_PARTITION p2;  int i;  KHE_RESOURCE r;

  /* do the classifying */
  *p = NULL;  *mt = NULL;
  r = KheTaskAsstResource(task);
  if( r != NULL )
  {
    if( !KhePartitionSolverRetrieve(ps, KheResourcePartition(r), p) )
      MAssert(false, "KhePartitionSolverClassifyTask internal error");
  }
  else
  {
    MArrayForEach(ps->partitions, &p2, &i)
      if( !KheResourceGroupDisjoint(KheTaskDomain(task), p2->resource_group) )
      {
	if( *mt != NULL )
          KheMultiTaskAddPartition(*mt, p2);
	else if( *p != NULL )
	{
	  *mt = KheMultiTaskMake(task);
          KheMultiTaskAddPartition(*mt, *p);
          KheMultiTaskAddPartition(*mt, p2);
	  *p = NULL;
	}
	else
	  *p = p2;
      }
  }
}


/*****************************************************************************/
/*                                                                           */
/*  void KhePartitionSolverSetDemandsAndTasks(KHE_PARTITION_SOLVER ps)       */
/*                                                                           */
/*  Work out the demands and tasks for ps.                                   */
/*                                                                           */
/*****************************************************************************/

static void KhePartitionSolverSetDemandsAndTasks(KHE_PARTITION_SOLVER ps)
{
  int i;  KHE_TASK task;  KHE_PARTITION p;  KHE_MULTI_TASK mt;
  for( i = 0;  i < KheTaskingTaskCount(ps->tasking);  i++ )
  {
    task = KheTaskingTask(ps->tasking, i);
    MAssert(KheTaskIsLeader(task),
      "KhePartitionSolverSetDemandsAndTasks internal error");
    KhePartitionSolverClassifyTask(ps, task, &p, &mt);
    if( mt != NULL )
      MArrayAddLast(ps->multi_tasks, mt);
    else if( p != NULL )
      p->duration_demand += KheTaskTotalDuration(task);
    else
    {
      /* task is unassignable; ignore it */
      if( DEBUG1 )
      {
	fprintf(stderr, "  unclassified task ");
	KheTaskDebug(task, 1, 0, stderr);
      }
    }
  }
}


/*****************************************************************************/
/*                                                                           */
/*  void KhePartitionSolverDelete(KHE_PARTITION_SOLVER ps)                   */
/*                                                                           */
/*  Delete ps, reclaiming its memory.                                        */
/*                                                                           */
/*****************************************************************************/

static void KhePartitionSolverDelete(KHE_PARTITION_SOLVER ps)
{
  while( MArraySize(ps->partitions) > 0 )
    KhePartitionDelete(MArrayRemoveLast(ps->partitions));
  MArrayFree(ps->partitions);
  while( MArraySize(ps->multi_tasks) > 0 )
    KheMultiTaskDelete(MArrayRemoveLast(ps->multi_tasks));
  MArrayFree(ps->multi_tasks);
  MFree(ps);
}


/*****************************************************************************/
/*                                                                           */
/*  void KhePartitionSolverDebug(KHE_PARTITION_SOLVER ps, int verbosity,     */
/*    int indent, FILE *fp)                                                  */
/*                                                                           */
/*  Debug printf of ps onto fp with the given verbosity and indent.          */
/*                                                                           */
/*****************************************************************************/

static void KhePartitionSolverDebug(KHE_PARTITION_SOLVER ps, int verbosity,
  int indent, FILE *fp)
{
  KHE_PARTITION p;  KHE_MULTI_TASK mt;  int i;  KHE_RESOURCE_TYPE rt;
  if( verbosity >= 1 && indent >= 0 )
  {
    rt = KheTaskingResourceType(ps->tasking);
    fprintf(fp, "%*s[ PartitionSolver (type %s)\n", indent, "",
      KheResourceTypeId(rt) == NULL ? "-" : KheResourceTypeId(rt));
    MArrayForEach(ps->partitions, &p, &i)
      KhePartitionDebug(p, verbosity, indent + 2, fp);
    MArrayForEach(ps->multi_tasks, &mt, &i)
      KheMultiTaskDebug(mt, verbosity, indent + 2, fp);
    fprintf(fp, "%*s]\n", indent, "");
  }
}


/*****************************************************************************/
/*                                                                           */
/*  Submodule "partition job"                                                */
/*                                                                           */
/*****************************************************************************/

/*****************************************************************************/
/*                                                                           */
/*  void KheDoPartitionJob(KHE_TASKING tasking, bool preserve_invariant)     */
/*                                                                           */
/*  Do the partition job on tasking.                                         */
/*                                                                           */
/*****************************************************************************/

static void KheDoPartitionJob(KHE_TASKING tasking, bool preserve_invariant)
{
  KHE_RESOURCE_TYPE rt;  KHE_TRANSACTION t;
  int i, j;  KHE_PARTITION_SOLVER ps;  KHE_PARTITION p;  KHE_MULTI_TASK mt;

  if( DEBUG1 )
    fprintf(stderr, "[ KheDoPartitionJob(tasking, %s)\n",
      preserve_invariant ? "true" : "false");

  /* quit if tasking has no resource type, rt */
  rt = KheTaskingResourceType(tasking);
  if( rt == NULL )
  {
    if( DEBUG1 )
      fprintf(stderr, "] KheDoPartitionJob returning (no resource type)\n");
    return;
  }

  /* quit if rt is all preassigned */
  if( KheResourceTypeDemandIsAllPreassigned(rt) )
  {
    if( DEBUG1 )
      fprintf(stderr, "] KheDoPartitionJob returning (all preassigned)\n");
    return;
  }

  /* quit if rt does not have partitions */
  if( !KheResourceTypeHasPartitions(rt) )
  {
    if( DEBUG1 )
      fprintf(stderr, "] KheDoPartitionJob returning (no partitions)\n");
    return;
  }

  /* quit if rt has too few partitions */
  if( KheResourceTypePartitionCount(rt) < 4 )
  {
    if( DEBUG1 )
      fprintf(stderr, "] KheDoPartitionJob returning (too few partitions)\n");
    return;
  }

  /* quit if rt has too many partitions */
  if( 3*KheResourceTypePartitionCount(rt) > KheResourceTypeResourceCount(rt) )
  {
    if( DEBUG1 )
      fprintf(stderr,"] KheDoPartitionJob returning (too many partitions)\n");
    return;
  }

  /* initialize a partition solver */
  ps = KhePartitionSolverMake(tasking, preserve_invariant);

  /* set duration demand and accumulate general tasks */
  KhePartitionSolverSetDemandsAndTasks(ps);

  /* sort and debug */
  MArraySort(ps->multi_tasks, &KheMultiTaskCmp);
  if( DEBUG4 )
    KhePartitionSolverDebug(ps, 2, 2, stderr);
  
  /* try to reduce the domain of each multi-task in turn */
  t = KheTransactionMake(KheTaskingSoln(tasking));
  MArrayForEach(ps->multi_tasks, &mt, &i)
  {
    if( mt->dominant_partition != NULL )
    {
      /* if there is a dominant partition, try it only */
      p = mt->dominant_partition;
      if( KhePartitionAcceptsTask(p, mt->task, ps->preserve_invariant, t) )
      {
	p->duration_demand += KheTaskTotalDuration(mt->task);
	if( DEBUG1 )
	{
	  fprintf(stderr, "  reduced dominated task ");
	  KheTaskDebug(mt->task, 2, 0, stderr);
	}
      }
      else
      {
	if( DEBUG1 )
	{
	  fprintf(stderr, "  unreduced dominated task ");
	  KheTaskDebug(mt->task, 2, 0, stderr);
	}
      }
    }
    else
    {
      /* if there is no dominant partition, try them in avail duration order */
      MArraySort(mt->partitions, &KhePartitionDecreasingAvailCmp);
      if( DEBUG5 )
      {
	fprintf(stderr, "  about to choose from:\n");
	MArrayForEach(mt->partitions, &p, &j)
	  KhePartitionDebug(p, 2, 4, stderr);
      }
      MArrayForEach(mt->partitions, &p, &j)
	if( KhePartitionAcceptsTask(p, mt->task, ps->preserve_invariant, t) )
	{
	  p->duration_demand += KheTaskTotalDuration(mt->task);
	  if( DEBUG1 )
	  {
	    fprintf(stderr, "  reduced undominated task ");
	    KheTaskDebug(mt->task, 2, 0, stderr);
	  }
	  break;
	}
      if( DEBUG1 && j >= MArraySize(ps->partitions) )
      {
	fprintf(stderr, "  unreduced undominated task ");
	KheTaskDebug(mt->task, 2, 0, stderr);
      }
    }
  }
  KheTransactionDelete(t);

  /* reclaim memory and quit */
  KhePartitionSolverDelete(ps);
  if( DEBUG1 )
    fprintf(stderr, "] KheDoPartitionJob returning (all done)\n");
}


/*****************************************************************************/
/*                                                                           */
/*  Submodule "constraint jobs"                                              */
/*                                                                           */
/*****************************************************************************/

/*****************************************************************************/
/*                                                                           */
/*  void KheTaskingDoPreferResourcesConstraintJob(KHE_TASKING tasking,       */
/*    KHE_PREFER_RESOURCES_CONSTRAINT c, bool preserve_invariant)            */
/*                                                                           */
/*  Apply prefer resources constraint c to tasking.                          */
/*                                                                           */
/*****************************************************************************/

static void KheTaskingDoPreferResourcesConstraintJob(KHE_TASKING tasking,
  KHE_PREFER_RESOURCES_CONSTRAINT c, bool preserve_invariant)
{
  KHE_TASK task;  KHE_RESOURCE_GROUP domain;  int i, j, init_count;
  KHE_EVENT_RESOURCE er;  KHE_SOLN soln;  KHE_TRANSACTION t;  bool success;
  domain = KhePreferResourcesConstraintDomain(c);
  soln = KheTaskingSoln(tasking);
  t = KheTransactionMake(soln);
  for( i = 0;  i < KhePreferResourcesConstraintEventResourceCount(c);  i++ )
  {
    er = KhePreferResourcesConstraintEventResource(c, i);
    for( j = 0;  j < KheEventResourceTaskCount(soln, er);  j++ )
    {
      task = KheTaskLeader(KheEventResourceTask(soln, er, j));
      if( KheTaskTasking(task) == tasking &&
	  !KheResourceGroupDisjoint(KheTaskDomain(task), domain) )
      {
	KheInvariantTransactionBegin(t, &init_count, preserve_invariant);
	success = KheTaskTightenDomain(task, domain, true);
	KheInvariantTransactionEnd(t, &init_count, preserve_invariant, success);
      }
    }
  }
  KheTransactionDelete(t);
}


/*****************************************************************************/
/*                                                                           */
/*  int KheTaskIncreasingDomainSizeCmp(const void *t1, const void *t2)       */
/*                                                                           */
/*  Comparison function for sorting an array of tasks by increasing          */
/*  domain size.                                                             */
/*                                                                           */
/*****************************************************************************/

static int KheTaskIncreasingDomainSizeCmp(const void *t1, const void *t2)
{
  KHE_TASK task1 = * (KHE_TASK *) t1;
  KHE_TASK task2 = * (KHE_TASK *) t2;
  KHE_RESOURCE_GROUP rg1 = KheTaskDomain(task1);
  KHE_RESOURCE_GROUP rg2 = KheTaskDomain(task2);
  if( KheResourceGroupResourceCount(rg1) != KheResourceGroupResourceCount(rg2) )
   return KheResourceGroupResourceCount(rg1)-KheResourceGroupResourceCount(rg2);
  else
    return KheTaskIndexInSoln(task1) - KheTaskIndexInSoln(task2);
}


/*****************************************************************************/
/*                                                                           */
/*  void KheTaskingDoAvoidSplitAssignmentsConstraintJob(KHE_TASKING tasking, */
/*    KHE_AVOID_SPLIT_ASSIGNMENTS_CONSTRAINT c, bool preserve_invariant)     */
/*                                                                           */
/*  Apply avoid split assignments constraint c to tasking.                   */
/*                                                                           */
/*  Implementation note.  After gathering the set of all tasks that need     */
/*  to be merged, the tasks are sorted into increasing domain size order.    */
/*  The point of doing this is that we need then only try merging each       */
/*  task into its preceding tasks, and only then if it is not assigned.      */
/*                                                                           */
/*****************************************************************************/

static void KheTaskingDoAvoidSplitAssignmentsConstraintJob(KHE_TASKING tasking,
  KHE_AVOID_SPLIT_ASSIGNMENTS_CONSTRAINT c, bool preserve_invariant)
{
  ARRAY_KHE_TASK tasks;  KHE_TASK task, task2;  int i, j, k, pos, init_count;
  KHE_EVENT_RESOURCE er;  KHE_RESOURCE_GROUP domain;  KHE_SOLN soln;
  KHE_TRANSACTION t;  bool success;

  MArrayInit(tasks);
  soln = KheTaskingSoln(tasking);
  t = KheTransactionMake(soln);
  for( i = 0;  i < KheAvoidSplitAssignmentsConstraintEventGroupCount(c);  i++ )
  {
    /* gather the distinct root tasks for event group i that lie in tasking */
    MArrayClear(tasks);
    for( j=0; j<KheAvoidSplitAssignmentsConstraintEventResourceCount(c,i); j++ )
    {
      er = KheAvoidSplitAssignmentsConstraintEventResource(c, i, j);
      for( k = 0;  k < KheEventResourceTaskCount(soln, er);  k++ )
      {
	task = KheTaskLeader(KheEventResourceTask(soln, er, k));
	if( KheTaskTasking(task)==tasking && !MArrayContains(tasks,task,&pos) )
	  MArrayAddLast(tasks, task);
      }
    }

    /* sort the tasks into increasing domain size order */
    MArraySort(tasks, &KheTaskIncreasingDomainSizeCmp);

    /* try to merge each unassigned task into a preceding task */
    MArrayForEach(tasks, &task, &j)
      if( KheTaskAsst(task) == NULL )
      {
	/* look for a merge that involves no change to task's domain */
	KheInvariantTransactionBegin(t, &init_count, preserve_invariant);
	success = false;
	for( k = j - 1;  !success && k >= 0;  k-- )
	{
          task2 = MArrayGet(tasks, k);
	  if( KheTaskAsst(task2) != NULL )
	    task2 = KheTaskAsst(task);
	  if( KheTaskAssign(task, task2) )
	  {
	    KheTaskingDeleteTask(tasking, task);
	    MArrayRemove(tasks, j);
	    j--;
	    success = true;
	  }
	}

	/* look for a merge that involves tightening task's domain */
	domain = KheTaskDomain(task);
	for( k = j - 1;  !success && k >= 0;  k-- )
	{
          task2 = MArrayGet(tasks, k);
	  if( KheTaskAsst(task2) == NULL &&
	      !KheResourceGroupDisjoint(domain, KheTaskDomain(task2)) )
	  {
	    /* reduce task's domain and assign task to task2 */
	    if( !KheTaskTightenDomain(task, KheTaskDomain(task2), true) )
	      MAssert(false, "KheTaskingDoAvoidSplitAssignmentsConstraintJob"
		"internal error 1");
	    if( !KheTaskAssign(task, task2) )
	      MAssert(false, "KheTaskingDoAvoidSplitAssignmentsConstraintJob"
		"internal error 2");
	    KheTaskingDeleteTask(tasking, task);
	    MArrayRemove(tasks, j);
	    j--;
	    success = true;
	  }
	}
	KheInvariantTransactionEnd(t, &init_count, preserve_invariant, success);
      }
  }
  KheTransactionDelete(t);
  MArrayFree(tasks);
}


/*****************************************************************************/
/*                                                                           */
/*  int KheConstraintDecreasingCombinedWeightCmp(const void *t1,             */
/*    const void *t2)                                                        */
/*                                                                           */
/*  Comparison function for sorting an array of constraints by decreasing    */
/*  combined weight.                                                         */
/*                                                                           */
/*****************************************************************************/

static int KheConstraintDecreasingCombinedWeightCmp(const void *t1,
  const void *t2)
{
  KHE_CONSTRAINT c1 = * (KHE_CONSTRAINT *) t1;
  KHE_CONSTRAINT c2 = * (KHE_CONSTRAINT *) t2;
  return KheCostCmp(KheConstraintCombinedWeight(c2),
    KheConstraintCombinedWeight(c1));
}


/*****************************************************************************/
/*                                                                           */
/*  void KheTaskingDoConstraintJobs(KHE_TASKING tasking,                     */
/*    KHE_TASK_JOB_TYPE tjt, bool preserve_invariant)                        */
/*                                                                           */
/*  Apply to the tasks of tasking the constraint jobs requested by tjt.      */
/*                                                                           */
/*****************************************************************************/

static void KheTaskingDoConstraintJobs(KHE_TASKING tasking,
  KHE_TASK_JOB_TYPE tjt, bool preserve_invariant)
{
  KHE_INSTANCE ins;  ARRAY_KHE_CONSTRAINT constraints;  KHE_CONSTRAINT c;
  KHE_RESOURCE_TYPE rt;  KHE_PREFER_RESOURCES_CONSTRAINT prc;  int i;

  /* gather the constraints required */
  ins = KheSolnInstance(KheTaskingSoln(tasking));
  rt = KheTaskingResourceType(tasking);
  MArrayInit(constraints);
  for( i = 0;  i < KheInstanceConstraintCount(ins);  i++ )
  {
    c = KheInstanceConstraint(ins, i);
    if( KheConstraintCombinedWeight(c) > 0 ) switch( KheConstraintTag(c) )
    {
      case KHE_PREFER_RESOURCES_CONSTRAINT_TAG:

	prc = (KHE_PREFER_RESOURCES_CONSTRAINT) c;
	if( rt == NULL || rt == KheResourceGroupResourceType(
	    KhePreferResourcesConstraintDomain(prc)) )
	{
	  if( (KheConstraintRequired(c) && (tjt & KHE_TASK_JOB_HARD_PRC)) ||
	      (!KheConstraintRequired(c) && (tjt & KHE_TASK_JOB_SOFT_PRC)) )
	    MArrayAddLast(constraints, c);
	}
	break;

      case KHE_AVOID_SPLIT_ASSIGNMENTS_CONSTRAINT_TAG:

	if( (KheConstraintRequired(c) && (tjt & KHE_TASK_JOB_HARD_ASAC)) ||
	    (!KheConstraintRequired(c) && (tjt & KHE_TASK_JOB_SOFT_ASAC)) )
	  MArrayAddLast(constraints, c);
	break;

      default:

	/* constraint of other types are not relevant */
	break;
    }
  }

  /* handle each constraint in turn, in decreasing combined weight order */
  MArraySort(constraints, &KheConstraintDecreasingCombinedWeightCmp);
  MArrayForEach(constraints, &c, &i)
    switch( KheConstraintTag(c) )
    {
      case KHE_PREFER_RESOURCES_CONSTRAINT_TAG:

        KheTaskingDoPreferResourcesConstraintJob(tasking,
          (KHE_PREFER_RESOURCES_CONSTRAINT) c, preserve_invariant);
	break;

      case KHE_AVOID_SPLIT_ASSIGNMENTS_CONSTRAINT_TAG:

        KheTaskingDoAvoidSplitAssignmentsConstraintJob(tasking,
          (KHE_AVOID_SPLIT_ASSIGNMENTS_CONSTRAINT) c, preserve_invariant);
	break;

      default:

	MAssert(false, "KheTaskingDoConstraintJobs internal error");
	break;
    }

  /* clean up and quit */
  MArrayFree(constraints);
}


/*****************************************************************************/
/*                                                                           */
/*  Submodule "task tree construction"                                       */
/*                                                                           */
/*****************************************************************************/

/*****************************************************************************/
/*                                                                           */
/*  int KheResourceTypeCmp(const void *t1, const void *t2)                   */
/*                                                                           */
/*  Comparison function for sorting an array of resource types.  Resource    */
/*  types whose event resources are all preassigned come first, followed     */
/*  by the others in decreasing order of number of points of application     */
/*  of avoid split assignments constraints.                                  */
/*                                                                           */
/*****************************************************************************/

static int KheResourceTypeCmp(const void *t1, const void *t2)
{
  KHE_RESOURCE_TYPE rt1 = * (KHE_RESOURCE_TYPE *) t1;
  KHE_RESOURCE_TYPE rt2 = * (KHE_RESOURCE_TYPE *) t2;
  bool all_preassigned1 = KheResourceTypeDemandIsAllPreassigned(rt1);
  bool all_preassigned2 = KheResourceTypeDemandIsAllPreassigned(rt2);
  int asa_count1 = KheResourceTypeAvoidSplitAssignmentsCount(rt1);
  int asa_count2 = KheResourceTypeAvoidSplitAssignmentsCount(rt2);
  if( all_preassigned1 != all_preassigned2 )
    return all_preassigned2 - all_preassigned1;
  else if( asa_count1 != asa_count2 )
    return asa_count2 - asa_count1;
  else
    return KheResourceTypeIndex(rt1) - KheResourceTypeIndex(rt2);
}


/*****************************************************************************/
/*                                                                           */
/*  void KheTaskTreeMake(KHE_SOLN soln, KHE_TASK_JOB_TYPE tjt,               */
/*    bool preserve_invariant, bool check_prefer_resources_monitors,         */
/*    bool check_avoid_split_assignments_monitors)                           */
/*                                                                           */
/*  Make a task tree from the tasks of soln.                                 */
/*                                                                           */
/*****************************************************************************/

void KheTaskTreeMake(KHE_SOLN soln, KHE_TASK_JOB_TYPE tjt,
  bool preserve_invariant, bool check_prefer_resources_monitors,
  bool check_avoid_split_assignments_monitors)
{
  ARRAY_KHE_RESOURCE_TYPE resource_types;  KHE_RESOURCE_TYPE rt;
  KHE_INSTANCE ins;  int i;  KHE_TASKING tasking;

  /* get the resource types of soln's instance, in the required order */
  ins = KheSolnInstance(soln);
  MArrayInit(resource_types);
  for( i = 0;  i < KheInstanceResourceTypeCount(ins);  i++ )
    MArrayAddLast(resource_types, KheInstanceResourceType(ins, i));
  MArraySort(resource_types, &KheResourceTypeCmp);

  /* build one tasking for each type, and apply KheTaskingMakeTaskTree to it */
  MAssert(KheSolnTaskingCount(soln) == 0, "KheTaskTreeMake: soln has tasking");
  MArrayForEach(resource_types, &rt, &i)
  {
    tasking = KheTaskingMakeFromResourceType(soln, rt);
    KheTaskingMakeTaskTree(tasking, tjt, preserve_invariant,
      check_prefer_resources_monitors, check_avoid_split_assignments_monitors);
  }

  /* clean up and exit */
  MArrayFree(resource_types);
}


/*****************************************************************************/
/*                                                                           */
/*  KHE_TASKING KheTaskingMakeFromResourceType(KHE_SOLN soln,                */
/*    KHE_RESOURCE_TYPE rt)                                                  */
/*                                                                           */
/*  Make a tasking holding the leader tasks of soln of resource type rt.     */
/*                                                                           */
/*****************************************************************************/

KHE_TASKING KheTaskingMakeFromResourceType(KHE_SOLN soln, KHE_RESOURCE_TYPE rt)
{
  int i;  KHE_TASK task;  KHE_TASKING res;
  res = KheTaskingMake(soln, rt);
  for( i = 0;  i < KheSolnTaskCount(soln);  i++ )
  {
    task = KheSolnTask(soln, i);
    if( KheTaskIsLeader(task) && (rt==NULL || KheTaskResourceType(task)==rt) )
      KheTaskingAddTask(res, task);
  }
  return res;
}


/*****************************************************************************/
/*                                                                           */
/*  void KheTaskingMakeTaskTree(KHE_TASKING tasking, KHE_TASK_JOB_TYPE tjt,  */
/*    bool preserve_invariant, bool check_prefer_resources_monitors,         */
/*    bool check_avoid_split_assignments_monitors)                           */
/*                                                                           */
/*  Make a task tree from the tasks of tasking.                              */
/*                                                                           */
/*  Since this can change parts of the solution that rarely change,          */
/*  namely task domains and the assignments of follower tasks, this          */
/*  function offers the option of calling KheMonitorAttachCheck on           */
/*  the affected prefer resources and avoid split assignments monitors.      */
/*                                                                           */
/*****************************************************************************/

void KheTaskingMakeTaskTree(KHE_TASKING tasking, KHE_TASK_JOB_TYPE tjt,
  bool preserve_invariant, bool check_prefer_resources_monitors,
  bool check_avoid_split_assignments_monitors)
{
  int i;  KHE_TASK task;  KHE_EVENT_RESOURCE er;

  if( DEBUG3 )
  {
    fprintf(stderr, "[ KheTaskingMakeTaskTree(%s,",
      KheTaskingResourceType(tasking) == NULL ? "~" :
      KheResourceTypeId(KheTaskingResourceType(tasking)) == NULL ? "-" :
      KheResourceTypeId(KheTaskingResourceType(tasking)));
    if( tjt & KHE_TASK_JOB_HARD_PRC )
      fprintf(stderr, " HARD_PRC");
    if( tjt & KHE_TASK_JOB_SOFT_PRC )
      fprintf(stderr, " SOFT_PRC");
    if( tjt & KHE_TASK_JOB_HARD_ASAC )
      fprintf(stderr, " HARD_ASAC");
    if( tjt & KHE_TASK_JOB_SOFT_ASAC )
      fprintf(stderr, " SOFT_ASAC");
    if( tjt & KHE_TASK_JOB_PARTITION )
      fprintf(stderr, " PART");
    fprintf(stderr, ", %s, %s, %s)\n",
      preserve_invariant ? "true" : "false",
      check_prefer_resources_monitors ? "true" : "false",
      check_avoid_split_assignments_monitors ? "true" : "false");
  }

  /* jobs to install existing domains and assignments */
  /* nothing to do here */

  /* job to assign preassigned tasks (some may fail, ignore that) */
  for( i = 0;  i < KheTaskingTaskCount(tasking);  i++ )
  {
    task = KheTaskingTask(tasking, i);
    er = KheTaskEventResource(task);
    if( er != NULL && KheEventResourcePreassignedResource(er) != NULL &&
	KheTaskAsst(task) == NULL )
      KheTaskAssignResource(task, KheEventResourcePreassignedResource(er));
  }

  /* jobs for prefer resources and avoid split assignments constraints */
  if( tjt & (KHE_TASK_JOB_HARD_PRC | KHE_TASK_JOB_SOFT_PRC |
	     KHE_TASK_JOB_HARD_ASAC | KHE_TASK_JOB_SOFT_ASAC) )
    KheTaskingDoConstraintJobs(tasking, tjt, preserve_invariant);

  /* partition jobs */
  if( tjt & KHE_TASK_JOB_PARTITION )
    KheDoPartitionJob(tasking, preserve_invariant);

  /* check prefer resources monitors */
  if( check_prefer_resources_monitors )
    KheTaskingMonitorAttachCheck(tasking, KHE_PREFER_RESOURCES_MONITOR_TAG,
      false);

  /* check avoid split assignments monitors */
  if( check_avoid_split_assignments_monitors )
    KheTaskingMonitorAttachCheck(tasking,
      KHE_AVOID_SPLIT_ASSIGNMENTS_MONITOR_TAG, false);
  if( DEBUG3 )
    fprintf(stderr, "] KheTaskingMakeTaskTree returning\n");
}


/*****************************************************************************/
/*                                                                           */
/*  Submodule "task tree reconstruction"                                     */
/*                                                                           */
/*****************************************************************************/

/*****************************************************************************/
/*                                                                           */
/*  void KheTaskingTightenToPartition(KHE_TASKING tasking)                   */
/*                                                                           */
/*  Tighten the domains of the tasks of tasking so that they lie entirely    */
/*  within individual domains, as far as possible while preserving the       */
/*  resource assignment invariant.                                           */
/*                                                                           */
/*****************************************************************************/

void KheTaskingTightenToPartition(KHE_TASKING tasking)
{
  KheTaskingMakeTaskTree(tasking, KHE_TASK_JOB_PARTITION,
    true, false, false);
}


/*****************************************************************************/
/*                                                                           */
/*  void KheTaskTreeDelete(KHE_SOLN soln)                                    */
/*                                                                           */
/*  Remove all trace of a task tree from soln.                               */
/*                                                                           */
/*****************************************************************************/

/* ***
void KheTaskTreeDelete(KHE_SOLN soln)
{
}
*** */


/*****************************************************************************/
/*                                                                           */
/*  void KheTaskingAllowSplitAssignments(KHE_TASKING tasking,                */
/*    bool unassigned_only)                                                  */
/*                                                                           */
/*  Allow split assignments by converting the followers of the tasks of      */
/*  tasking into leaders.  If unassigned_only is true, do this only for      */
/*  the unassigned tasks of tasking.                                         */
/*                                                                           */
/*  The followers are made into tasks of tasking, and any assignments to     */
/*  their former leaders are preserved as assignments to them.               */
/*                                                                           */
/*  Implementation note.  At first glance this code appears to visit         */
/*  only the children of each leader task, not its proper descendants.       */
/*  However, each child becomes a leader and gets added to the end of        */
/*  tasking's list of tasks, so it is visited later and its children         */
/*  made into leaders, etc.                                                  */
/*                                                                           */
/*****************************************************************************/

void KheTaskingAllowSplitAssignments(KHE_TASKING tasking, bool unassigned_only)
{
  KHE_TASK task, child_task;  int i;

  /* do the reconstructing */
  for( i = 0;  i < KheTaskingTaskCount(tasking);  i++ )
  {
    task = KheTaskingTask(tasking, i);
    if( !unassigned_only || KheTaskAsst(task) == NULL )
      while( KheTaskAssignedToCount(task) > 0 )
      {
	child_task = KheTaskAssignedTo(task, 0);
	KheTaskUnAssign(child_task);
	KheTaskingAddTask(tasking, child_task);
	if( KheTaskAsst(task) != NULL )
	  if( !KheTaskAssign(child_task, KheTaskAsst(task)) )
	    MAssert(false, "KheTaskingAllowSplitAssignments internal error");
      }
  }

  /* apply KheMonitorAttachCheck to affected avoid split assignments monitors */
  KheTaskingMonitorAttachCheck(tasking,
    KHE_AVOID_SPLIT_ASSIGNMENTS_MONITOR_TAG, unassigned_only);
}


/*****************************************************************************/
/*                                                                           */
/*  void KheTaskingEnlargeDomains(KHE_TASKING tasking, bool unassigned_only) */
/*                                                                           */
/*  Enlarge the domains of the tasks of tasking (or the domains of just the  */
/*  unassigned tasks, if unassigned_only is true) to include all resources   */
/*  of their type.                                                           */
/*                                                                           */
/*****************************************************************************/

void KheTaskingEnlargeDomains(KHE_TASKING tasking, bool unassigned_only)
{
  KHE_TASK task;  int i;  KHE_RESOURCE_GROUP rg;

  /* do the domain enlarging */
  for( i = 0;  i < KheTaskingTaskCount(tasking);  i++ )
  {
    task = KheTaskingTask(tasking, i);
    if( !unassigned_only || KheTaskAsst(task) == NULL )
    {
      rg = KheResourceTypeFullResourceGroup(KheTaskResourceType(task));
      if( !KheTaskSetDomain(task, rg, true) )
	MAssert(false, "KheTaskingEnlargeDomains internal error");
    }
  }

  /* apply KheMonitorAttachCheck to affected prefer resources monitors */
  KheTaskingMonitorAttachCheck(tasking,
    KHE_PREFER_RESOURCES_MONITOR_TAG, unassigned_only);
}


/*****************************************************************************/
/*                                                                           */
/*  void KheTaskCheckMonitors(KHE_TASK task, KHE_MONITOR_TAG tag)            */
/*                                                                           */
/*  Call KheMonitorAttachCheck on monitors of type tag affected by task.     */
/*                                                                           */
/*****************************************************************************/

static void KheTaskCheckMonitors(KHE_TASK task, KHE_MONITOR_TAG tag)
{
  int i;  KHE_EVENT_RESOURCE er;  KHE_SOLN soln;  KHE_MONITOR m;

  /* check the prefer resources monitors affected by task itself */
  soln = KheTaskSoln(task);
  er = KheTaskEventResource(task);
  if( er != NULL )
    for( i = 0;  i < KheEventResourceMonitorCount(soln, er);  i++ )
    {
      m = KheEventResourceMonitor(soln, er, i);
      if( KheMonitorTag(m) == tag )
	KheMonitorAttachCheck(m);
    }

  /* do the same job for the tasks assigned to task */
  for( i = 0;  i < KheTaskAssignedToCount(task);  i++ )
    KheTaskCheckMonitors(KheTaskAssignedTo(task, i), tag);
}


/*****************************************************************************/
/*                                                                           */
/*  void KheTaskingMonitorAttachCheck(KHE_TASKING tasking,                   */
/*    KHE_MONITOR_TAG tag, bool unassigned_only)                             */
/*                                                                           */
/*  Apply KheMonitorAttachCheck to each monitor of each event resource       */
/*  of each task of tasking, including follower tasks.  Only monitors        */
/*  of type tag are checked, and it unassigned_only is true, only            */
/*  unassigned leader tasks and their followers are checked.                 */
/*                                                                           */
/*****************************************************************************/

void KheTaskingMonitorAttachCheck(KHE_TASKING tasking,
  KHE_MONITOR_TAG tag, bool unassigned_only)
{
  KHE_TASK task;  int i;
  for( i = 0;  i < KheTaskingTaskCount(tasking);  i++ )
  {
    task = KheTaskingTask(tasking, i);
    if( !unassigned_only || KheTaskAsst(task) == NULL )
      KheTaskCheckMonitors(task, KHE_PREFER_RESOURCES_MONITOR_TAG);
  }
}
