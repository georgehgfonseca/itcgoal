
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
/*  FILE:         khe_assign_resource.c                                      */
/*  DESCRIPTION:  Miscellaneous resource assignment solvers                  */
/*                                                                           */
/*****************************************************************************/
#include "khe.h"
#include "m.h"
#include "khe_priqueue.h"
#include <limits.h>

#define DEBUG3 0
#define DEBUG6 0
#define DEBUG7 0
#define DEBUG9 0
#define DEBUG12 0

typedef MARRAY(KHE_TASK) ARRAY_KHE_TASK;


/*****************************************************************************/
/*                                                                           */
/*  Submodule "event resource defects"                                       */
/*                                                                           */
/*****************************************************************************/

/*****************************************************************************/
/*                                                                           */
/*  bool KheAssignResourceAugment(KHE_EJECTOR ej,                            */
/*    KHE_ASSIGN_RESOURCE_MONITOR arm)                                       */
/*                                                                           */
/*  Try to fix the tasks monitored by arm.                                   */
/*                                                                           */
/*****************************************************************************/

static bool KheAssignResourceAugment(KHE_EJECTOR ej,
  KHE_ASSIGN_RESOURCE_MONITOR arm)
{
  KHE_SOLN soln;  KHE_EVENT_RESOURCE er;  KHE_TASK task;  int i, j;
  KHE_RESOURCE_GROUP rg;  KHE_RESOURCE r;  KHE_TRACE tc;
  KHE_TRANSACTION tn;  bool res;  KHE_COST save_cost;
  if( DEBUG7 )
    fprintf(stderr, "%*s[ AssignResourceAugment\n",
      4*KheEjectorCurrDepth(ej) + 2, "");
  soln = KheMonitorSoln((KHE_MONITOR) KheEjectorGroupMonitor(ej));
  save_cost = KheSolnCost(soln);
  er = KheAssignResourceMonitorEventResource(arm);
  tc = KheTraceMake(KheEjectorGroupMonitor(ej));
  tn = KheTransactionMake(KheEjectorSoln(ej));
  for( i = 0;  i < KheEventResourceTaskCount(soln, er);  i++ )
  {
    task = KheTaskLeader(KheEventResourceTask(soln, er, i));
    if( KheTaskAsst(task) == NULL && !KheTaskVisited(task, 0) )
    {
      if( DEBUG7 )
      {
	fprintf(stderr, "%*s[ assigning ", 4*KheEjectorCurrDepth(ej) + 4, "");
	KheTaskDebug(task, 1, 0, stderr);
      }
      KheTaskVisit(task);
      rg = KheTaskDomain(task);
      for( j = 0;  j < KheResourceGroupResourceCount(rg);  j++ )
      {
	r = KheResourceGroupResource(rg, j);
	MAssert(KheSolnCost(soln) == save_cost,
	  "internal error in KheAssignResourceAugment 1: %.4f != %.4f",
	  KheCostShow(KheSolnCost(soln)), KheCostShow(save_cost));
	KheTransactionBegin(tn);
	KheTraceBegin(tc);
	res = KheTaskKempeAssignResource(task, r);
	if( DEBUG7 )
	  fprintf(stderr, "%*s  trying %s%s\n", 4*KheEjectorCurrDepth(ej) + 4,
	    "", KheResourceId(r) == NULL ? "-" : KheResourceId(r),
	    res ? "" : " (Kempe resource move failed)");
	KheTraceEnd(tc);
	KheTransactionEnd(tn);
	if( res && KheEjectorSuccess(ej, tc, KheTaskDuration(task)) )
	{
	  KheTraceDelete(tc);
	  KheTransactionDelete(tn);
	  if( DEBUG7 )
	  {
	    fprintf(stderr, "%*s] success\n", 4*KheEjectorCurrDepth(ej)+4, "");
	    fprintf(stderr, "%*s] AssignResourceAugment success\n",
	      4*KheEjectorCurrDepth(ej) + 2, "");
	  }
	  return true;
	}
	KheTransactionUndo(tn);
	if( DEBUG3 && KheSolnCost(soln) != save_cost )
	{
	  int k;  KHE_MONITOR m;  KHE_LIMIT_WORKLOAD_MONITOR lwm;
          KHE_LIMIT_WORKLOAD_CONSTRAINT lwc;
	  fprintf(stderr, "  KheAssignResourceAugment failing on %s\n",
	    KheResourceId(r) == NULL ? "-" : KheResourceId(r));
	  for( k = 0;  k < KheResourceMonitorCount(soln, r);  k++ )
	  {
	    m = KheResourceMonitor(soln, r, k);
	    if( KheMonitorTag(m) == KHE_LIMIT_WORKLOAD_MONITOR_TAG )
	    {
	      lwm = (KHE_LIMIT_WORKLOAD_MONITOR) m;
	      lwc = KheLimitWorkloadMonitorConstraint(lwm);
	      fprintf(stderr, "    workload %.4f, lim %d..%d\n",
		KheLimitWorkloadMonitorWorkload(lwm),
		KheLimitWorkloadConstraintMinimum(lwc),
		KheLimitWorkloadConstraintMaximum(lwc));
	    }
	  }
	}
	MAssert(KheSolnCost(soln) == save_cost,
	  "internal error in KheAssignResourceAugment 2: %.4f != %.4f",
	  KheCostShow(KheSolnCost(soln)), KheCostShow(save_cost));
      }
      if( KheEjectorScheduleMayRevisit(KheEjectorCurrSchedule(ej)) )
	KheTaskUnVisit(task);
      if( DEBUG7 )
	fprintf(stderr, "%*s] fail\n", 4*KheEjectorCurrDepth(ej) + 4, "");
    }
  }
  KheTraceDelete(tc);
  KheTransactionDelete(tn);
  if( DEBUG7 )
    fprintf(stderr, "%*s] AssignResourceAugment fail\n",
      4*KheEjectorCurrDepth(ej) + 2, "");
  MAssert(KheSolnCost(soln) == save_cost,
    "internal error in KheAssignResourceAugment 3: %.4f != %.4f",
    KheCostShow(KheSolnCost(soln)), KheCostShow(save_cost));
  return false;
}


/*****************************************************************************/
/*                                                                           */
/*  bool KhePreferResourcesAugment(KHE_EJECTOR ej,                           */
/*    KHE_PREFER_RESOURCES_MONITOR prm)                                      */
/*                                                                           */
/*  Try to fix the solution resources monitored by prm.                      */
/*                                                                           */
/*****************************************************************************/

static bool KhePreferResourcesAugment(KHE_EJECTOR ej,
  KHE_PREFER_RESOURCES_MONITOR prm)
{
  KHE_SOLN soln;  KHE_EVENT_RESOURCE er;  KHE_TASK task;  int i, j;
  KHE_RESOURCE_GROUP rg;  KHE_RESOURCE r, old_r;  KHE_TRACE tc;  bool success;
  KHE_PREFER_RESOURCES_CONSTRAINT prc;  KHE_TRANSACTION tn;
  soln = KheMonitorSoln((KHE_MONITOR) KheEjectorGroupMonitor(ej));
  er = KhePreferResourcesMonitorEventResource(prm);
  prc = KhePreferResourcesMonitorConstraint(prm);
  rg = KhePreferResourcesConstraintDomain(prc);
  tn = KheTransactionMake(soln);
  tc = KheTraceMake(KheEjectorGroupMonitor(ej));
  for( i = 0;  i < KheEventResourceTaskCount(soln, er);  i++ )
  {
    task = KheTaskLeader(KheEventResourceTask(soln, er, i));
    if( KheTaskAsst(task) != NULL )
    {
      old_r = KheTaskAsstResource(task);
      MAssert(old_r != NULL, "KhePreferResourcesAugment internal error");
      if( !KheTaskVisited(task, 0) && !KheResourceGroupContains(rg, old_r) )
      {
	KheTaskVisit(task);
	for( j = 0;  j < KheResourceGroupResourceCount(rg);  j++ )
	{
	  r = KheResourceGroupResource(rg, j);
	  KheTransactionBegin(tn);
	  KheTraceBegin(tc);
	  KheTaskUnAssignResource(task);
	  success = KheTaskKempeAssignResource(task, r);
	  KheTraceEnd(tc);
	  KheTransactionEnd(tn);
	  if( success && KheEjectorSuccess(ej, tc, KheTaskDuration(task)) )
	  {
	    KheTraceDelete(tc);
	    KheTransactionDelete(tn);
	    return true;
	  }
	  KheTransactionUndo(tn);
	}
	if( KheEjectorScheduleMayRevisit(KheEjectorCurrSchedule(ej)) )
	  KheTaskUnVisit(task);
      }
    }
  }
  KheTraceDelete(tc);
  KheTransactionDelete(tn);
  return false;
}


/*****************************************************************************/
/*                                                                           */
/*  void KheVisitAllTasks(KHE_AVOID_SPLIT_ASSIGNMENTS_MONITOR asam)          */
/*                                                                           */
/*  Mark all tasks monitored by asam unvisited.                              */
/*                                                                           */
/*****************************************************************************/

static void KheVisitAllTasks(KHE_AVOID_SPLIT_ASSIGNMENTS_MONITOR asam)
{
  KHE_AVOID_SPLIT_ASSIGNMENTS_CONSTRAINT c;  int egi, count, i, j;
  KHE_EVENT_RESOURCE er;  KHE_SOLN soln;  KHE_TASK task;
  c = KheAvoidSplitAssignmentsMonitorConstraint(asam);
  egi = KheAvoidSplitAssignmentsMonitorEventGroupIndex(asam);
  count = KheAvoidSplitAssignmentsConstraintEventResourceCount(c, egi);
  soln = KheMonitorSoln((KHE_MONITOR) asam);
  for( i = 0;  i < count;  i++ )
  {
    er = KheAvoidSplitAssignmentsConstraintEventResource(c, egi, i);
    for( j = 0;  j < KheEventResourceTaskCount(soln, er);  j++ )
    {
      task = KheTaskLeader(KheEventResourceTask(soln, er, j));
      KheTaskVisit(task);
    }
  }
}


/*****************************************************************************/
/*                                                                           */
/*  void KheUnVisitAllTasks(KHE_AVOID_SPLIT_ASSIGNMENTS_MONITOR asam)        */
/*                                                                           */
/*  Mark all tasks monitored by asam unvisited.                              */
/*                                                                           */
/*****************************************************************************/

static void KheUnVisitAllTasks(KHE_AVOID_SPLIT_ASSIGNMENTS_MONITOR asam)
{
  KHE_AVOID_SPLIT_ASSIGNMENTS_CONSTRAINT c;  int egi, count, i, j;
  KHE_EVENT_RESOURCE er;  KHE_SOLN soln;  KHE_TASK task;
  c = KheAvoidSplitAssignmentsMonitorConstraint(asam);
  egi = KheAvoidSplitAssignmentsMonitorEventGroupIndex(asam);
  count = KheAvoidSplitAssignmentsConstraintEventResourceCount(c, egi);
  soln = KheMonitorSoln((KHE_MONITOR) asam);
  for( i = 0;  i < count;  i++ )
  {
    er = KheAvoidSplitAssignmentsConstraintEventResource(c, egi, i);
    for( j = 0;  j < KheEventResourceTaskCount(soln, er);  j++ )
    {
      task = KheTaskLeader(KheEventResourceTask(soln, er, j));
      KheTaskUnVisit(task);
    }
  }
}


/*****************************************************************************/
/*                                                                           */
/*  bool KheAvoidSplitAssignmentsAugment(KHE_EJECTOR ej,                     */
/*    KHE_AVOID_SPLIT_ASSIGNMENTS_MONITOR asam)                              */
/*                                                                           */
/*  Try to fix the tasks monitored by asam.                                  */
/*                                                                           */
/*****************************************************************************/

static bool KheAvoidSplitAssignmentsAugment(KHE_EJECTOR ej,
  KHE_AVOID_SPLIT_ASSIGNMENTS_MONITOR asam)
{
  KHE_AVOID_SPLIT_ASSIGNMENTS_CONSTRAINT c;  int egi, count, i, j, k, durn;
  KHE_RESOURCE r;  KHE_TRACE tc;  KHE_TRANSACTION tn;  KHE_EVENT_RESOURCE er;
  KHE_SOLN soln;  KHE_TASK task;  KHE_RESOURCE_GROUP rg;  bool success;
  if( DEBUG12 )
  {
    fprintf(stderr, "[ KheAvoidSplitAssignmentsAugment:\n");
    KheMonitorDebug((KHE_MONITOR) asam, 2, 2, stderr);
  }

  /* if there are distinct resources assigned to visited tasks, fail */
  c = KheAvoidSplitAssignmentsMonitorConstraint(asam);
  egi = KheAvoidSplitAssignmentsMonitorEventGroupIndex(asam);
  count = KheAvoidSplitAssignmentsConstraintEventResourceCount(c, egi);
  soln = KheEjectorSoln(ej);
  r = NULL;
  task = NULL;
  for( i = 0;  i < count;  i++ )
  {
    er = KheAvoidSplitAssignmentsConstraintEventResource(c, egi, i);
    for( j = 0;  j < KheEventResourceTaskCount(soln, er);  j++ )
    {
      task = KheTaskLeader(KheEventResourceTask(soln, er, j));
      if( KheTaskVisited(task, 0) && KheTaskAsstResource(task) != NULL )
      {
	if( r != NULL && KheTaskAsstResource(task) != r )
	{
	  if( DEBUG12 )
	    fprintf(stderr,
	      "] KheAvoidSplitAssignmentsAugment returning false (visited)\n");
	  return false;
	}
	else
	  r = KheTaskAsstResource(task);
      }
    }
  }
  MAssert(task != NULL,"KheAvoidSplitAssignmentsAugment internal error");

  /* visit all tasks */
  KheVisitAllTasks(asam);

  /* the possible resources are just r if non-NULL, else a task's domain */
  rg = (r != NULL ? KheResourceSingletonResourceGroup(r) : KheTaskDomain(task));
  tc = KheTraceMake(KheEjectorGroupMonitor(ej));
  tn = KheTransactionMake(soln);
  for( i = 0;  i < KheResourceGroupResourceCount(rg);  i++ )
  {
    r = KheResourceGroupResource(rg, i);
    if( DEBUG12 )
      fprintf(stderr, "  trying %s\n", KheResourceId(r) == NULL ? "-" :
        KheResourceId(r));

    /* trace a transaction which ensures that r is assigned to every task */
    KheTransactionBegin(tn);
    KheTraceBegin(tc);
    success = true;
    durn = 0;
    for( j = 0;  success && j < count;  j++ )
    {
      er = KheAvoidSplitAssignmentsConstraintEventResource(c, egi, j);
      for( k = 0;  success && k < KheEventResourceTaskCount(soln, er);  k++ )
      {
	task = KheTaskLeader(KheEventResourceTask(soln, er, k));
	if( KheTaskAsstResource(task) != r )
	{
	  if( KheTaskAsstResource(task) != NULL )
	    KheTaskUnAssignResource(task);
	  success = success && KheTaskKempeAssignResource(task, r);
	  durn += KheTaskDuration(task);
	}
      }
    }
    KheTraceEnd(tc);
    KheTransactionEnd(tn);

    /* check for success and undo if not */
    if( success && KheEjectorSuccess(ej, tc, durn) )
    {
      KheTraceDelete(tc);
      KheTransactionDelete(tn);
      if( DEBUG12 )
	fprintf(stderr, "] KheAvoidSplitAssignmentsAugment returning true\n");
      return true;
    }
    KheTransactionUndo(tn);
  }
  KheTraceDelete(tc);
  KheTransactionDelete(tn);
  if( KheEjectorScheduleMayRevisit(KheEjectorCurrSchedule(ej)) )
    KheUnVisitAllTasks(asam);
  if( DEBUG12 )
    fprintf(stderr, "] KheAvoidSplitAssignmentsAugment returning false\n");
  return false;
}


/*****************************************************************************/
/*                                                                           */
/*  Submodule "resource defects"                                             */
/*                                                                           */
/*****************************************************************************/

/*****************************************************************************/
/*                                                                           */
/*  bool KheAvoidClashesAugment(KHE_EJECTOR ej,                              */
/*    KHE_AVOID_CLASHES_MONITOR acm)                                         */
/*                                                                           */
/*  Try to fix the resource timetable monitored by acm.                      */
/*                                                                           */
/*****************************************************************************/

static bool KheAvoidClashesAugment(KHE_EJECTOR ej,
  KHE_AVOID_CLASHES_MONITOR acm)
{
  KHE_RESOURCE r;
  r = KheAvoidClashesMonitorResource(acm);
  if( DEBUG9 )
    fprintf(stderr, "%*s[ AvoidClashesAugment (%s)\n",
      4*KheEjectorCurrDepth(ej) + 2, "",
      KheResourceId(r) != NULL ? KheResourceId(r) : "-");
  /* still to do */
          if( DEBUG9 )
    fprintf(stderr, "%*s] AvoidClashesAugment returning false (still to do)\n",
      4*KheEjectorCurrDepth(ej) + 2, "");
  return false;
}


/*****************************************************************************/
/*                                                                           */
/*  bool KheAvoidUnavailableTimesAugment(KHE_EJECTOR ej,                     */
/*    KHE_AVOID_UNAVAILABLE_TIMES_MONITOR autm)                              */
/*                                                                           */
/*  Try to fix the resource timetable monitored by autm.                     */
/*                                                                           */
/*****************************************************************************/

static bool KheAvoidUnavailableTimesAugment(KHE_EJECTOR ej,
  KHE_AVOID_UNAVAILABLE_TIMES_MONITOR autm)
{
  KHE_RESOURCE r;
  r = KheAvoidUnavailableTimesMonitorResource(autm);
  if( DEBUG9 )
    fprintf(stderr, "%*s[ AvoidUnavailableTimesAugment (%s)\n",
      4*KheEjectorCurrDepth(ej) + 2, "",
      KheResourceId(r) != NULL ? KheResourceId(r) : "-");
  /* still to do */
  if( DEBUG9 )
    fprintf(stderr,
      "%*s] AvoidUnavailableTimesAugment returning false (still to do)\n",
      4*KheEjectorCurrDepth(ej) + 2, "");
  return false;
}


/*****************************************************************************/
/*                                                                           */
/*  bool KheLimitIdleTimesAugment(KHE_EJECTOR ej,                            */
/*    KHE_LIMIT_IDLE_TIMES_MONITOR litm)                                     */
/*                                                                           */
/*  Try to fix the resource timetable monitored by litm.                     */
/*                                                                           */
/*****************************************************************************/

static bool KheLimitIdleTimesAugment(KHE_EJECTOR ej,
  KHE_LIMIT_IDLE_TIMES_MONITOR litm)
{
  /* still to do */
  //I COMMENTED THIS CODE!!!
  //MAssert(false, "KheLimitIdleTimesAugment still to do");
  return false;
}


/*****************************************************************************/
/*                                                                           */
/*  bool KheClusterBusyTimesAugment(KHE_EJECTOR ej,                          */
/*    KHE_CLUSTER_BUSY_TIMES_MONITOR cbtm)                                   */
/*                                                                           */
/*  Try to fix the resource timetable monitored by cbtm.                     */
/*                                                                           */
/*****************************************************************************/

static bool KheClusterBusyTimesAugment(KHE_EJECTOR ej,
  KHE_CLUSTER_BUSY_TIMES_MONITOR cbtm)
{
  /* still to do */
  //I COMMENTED THIS CODE
  //MAssert(false, "KheClusterBusyTimesAugment still to do");
  return false;
}


/*****************************************************************************/
/*                                                                           */
/*  bool KheBusyTimesOrWorkloadAugment(KHE_EJECTOR ej,                       */
/*    KHE_RESOURCE r)                                                        */
/*                                                                           */
/*  Repair a busy times or workload defect by deassigning one task.          */
/*                                                                           */
/*****************************************************************************/

static bool KheBusyTimesOrWorkloadAugment(KHE_EJECTOR ej,
  KHE_RESOURCE r)
{
  ARRAY_KHE_TASK tasks;  KHE_TASK task;  KHE_SOLN soln;  int i;
  KHE_TRACE tc;  KHE_RESOURCE junk;  KHE_COST save_cost;

  /* gather the unvisited tasks assigned directly to the cycle task */
  soln = KheEjectorSoln(ej);
  save_cost = KheSolnCost(soln);
  if( DEBUG6 )
    fprintf(stderr, "%*s[ LimitWorkloadAugment (%s)\n",
      4*KheEjectorCurrDepth(ej) + 2, "",
      KheResourceId(r) != NULL ? KheResourceId(r) : "-");
  MArrayInit(tasks);
  for( i = 0;  i < KheResourceAssignedTaskCount(soln, r);  i++ )
  {
    task = KheResourceAssignedTask(soln, r, i);
    MAssert(KheTaskAsst(task) != NULL,
      "KheLimitWorkloadAugment internal error");
    if( KheTaskIsLeader(task) && !KheTaskVisited(task, 0) &&
	!KheTaskIsPreassigned(task, false, &junk) )
      MArrayAddLast(tasks, task);
  }

  /* one repair deassigns one task (still unvisited and assigned r)  */
  tc = KheTraceMake(KheEjectorGroupMonitor(ej));
  MArrayForEach(tasks, &task, &i)
    if( !KheTaskVisited(task, 0) && KheTaskAsstResource(task) == r )
    {
      /* KheTaskVisit(task); don't consider this a visit! */
      KheTraceBegin(tc);
      if( DEBUG6 )
      {
	fprintf(stderr, "%*s[ deassign %s from ",
	  4*KheEjectorCurrDepth(ej) + 4, "",
          KheResourceId(r) != NULL ? KheResourceId(r) : "-");
	KheTaskDebug(task, 1, 0, stderr);
      }
      KheTaskUnAssignResource(task);
      KheTraceEnd(tc);
      if( KheEjectorSuccess(ej, tc, KheTaskDuration(task)) )
      {
	KheTraceDelete(tc);
	MArrayFree(tasks);
	if( DEBUG6 )
	{
	  fprintf(stderr, "%*s] success\n", 4*KheEjectorCurrDepth(ej) + 4, "");
	  fprintf(stderr, "%*s] LimitWorkloadAugment success\n",
	    4*KheEjectorCurrDepth(ej) + 2, "");
	}
	return true;
      }
      if( DEBUG6 )
	fprintf(stderr, "%*s] fail\n", 4*KheEjectorCurrDepth(ej) + 4, "");
      if( !KheTaskAssignResource(task, r) )
	MAssert(false, "KheEjectorLimitWorkloadAugment internal error");
      /* *** don't consider this a visit
      if( KheEjectorMayRevisit(ej) )
	KheTaskUnVisit(task);
      *** */
    }
  KheTraceDelete(tc);
  MArrayFree(tasks);
  if( DEBUG6 )
    fprintf(stderr, "%*s] LimitWorkloadAugment fail\n",
      4*KheEjectorCurrDepth(ej) + 2, "");
  MAssert(KheSolnCost(soln) == save_cost,
    "internal error in KheBusyTimesOrWorkloadAugment: %.4f != %.4f",
    KheCostShow(KheSolnCost(soln)), KheCostShow(save_cost));
  return false;
}


/*****************************************************************************/
/*                                                                           */
/*  bool KheLimitBusyTimesAugment(KHE_EJECTOR ej,                            */
/*    KHE_LIMIT_BUSY_TIMES_MONITOR lbtm)                                     */
/*                                                                           */
/*  Try to fix the resource timetable monitored by lbtm.                     */
/*                                                                           */
/*  Limiting this call to overload cases is still to do.                     */
/*                                                                           */
/*****************************************************************************/

static bool KheLimitBusyTimesAugment(KHE_EJECTOR ej,
  KHE_LIMIT_BUSY_TIMES_MONITOR lbtm)
{
  /* handling underloads here is still to do */
  return KheBusyTimesOrWorkloadAugment(ej,
    KheLimitBusyTimesMonitorResource(lbtm));
}


/*****************************************************************************/
/*                                                                           */
/*  bool KheLimitWorkloadAugment(KHE_EJECTOR ej,                             */
/*    KHE_LIMIT_WORKLOAD_MONITOR lwm)                                        */
/*                                                                           */
/*  Try to fix the resource timetable monitored by lwm.                      */
/*                                                                           */
/*****************************************************************************/

static bool KheLimitWorkloadAugment(KHE_EJECTOR ej,
  KHE_LIMIT_WORKLOAD_MONITOR lwm)
{
  KHE_LIMIT_WORKLOAD_CONSTRAINT lwc;  float workload;
  lwc = KheLimitWorkloadMonitorConstraint(lwm);
  workload = KheLimitWorkloadMonitorWorkload(lwm);
  if( workload < KheLimitWorkloadConstraintMinimum(lwc) )
  {
    /* resource is underloaded, repairing this is still to do */
    return false;
  }
  else if( workload > KheLimitWorkloadConstraintMaximum(lwc) )
  {
    /* resource is overloaded */
    return KheBusyTimesOrWorkloadAugment(ej,
      KheLimitWorkloadMonitorResource(lwm));
  }
  else
  {
    MAssert(false, "KheLimitWorkloadAugment internal error");
    return false;  /* keep compiler happy */
  }
}


/*****************************************************************************/
/*                                                                           */
/*  Submodule "main function and its augment functions"                      */
/*                                                                           */
/*****************************************************************************/

/*****************************************************************************/
/*                                                                           */
/*  bool KheResourceRepairEventResourceMonitorAugment(KHE_EJECTOR ej,        */
/*    KHE_MONITOR d)                                                         */
/*                                                                           */
/*  Augment function for repairing event resource defects.                   */
/*                                                                           */
/*****************************************************************************/

bool KheResourceRepairEventResourceMonitorAugment(KHE_EJECTOR ej, KHE_MONITOR d)
{
  KHE_GROUP_MONITOR dgm;  int i;
  MAssert(KheMonitorCost(d) > 0, "KheResourceRepairEventResourceMonitorAugment:"
    " d does not have non-zero cost");
  switch( KheMonitorTag(d) )
  {
    case KHE_ASSIGN_RESOURCE_MONITOR_TAG:

      return KheAssignResourceAugment(ej, (KHE_ASSIGN_RESOURCE_MONITOR) d);

    case KHE_PREFER_RESOURCES_MONITOR_TAG:

      return KhePreferResourcesAugment(ej, (KHE_PREFER_RESOURCES_MONITOR) d);

    case KHE_AVOID_SPLIT_ASSIGNMENTS_MONITOR_TAG:

      return KheAvoidSplitAssignmentsAugment(ej,
	(KHE_AVOID_SPLIT_ASSIGNMENTS_MONITOR) d);

    case KHE_GROUP_MONITOR_TAG:

      dgm = (KHE_GROUP_MONITOR) d;
      KheGroupMonitorCopyDefects(dgm);
      for( i = 0;  i < KheGroupMonitorDefectCopyCount(dgm);  i++ )
      {
	d = KheGroupMonitorDefectCopy(dgm, i);
	if( KheMonitorCost(d) > 0 )  switch( KheMonitorTag(d) )
	{
	  case KHE_ASSIGN_RESOURCE_MONITOR_TAG:

	    if( KheAssignResourceAugment(ej, (KHE_ASSIGN_RESOURCE_MONITOR) d) )
	      return true;
	    break;

	  case KHE_PREFER_RESOURCES_MONITOR_TAG:

	    if( KhePreferResourcesAugment(ej, (KHE_PREFER_RESOURCES_MONITOR)d) )
	      return true;
	    break;

	  case KHE_AVOID_SPLIT_ASSIGNMENTS_MONITOR_TAG:

	    if( KheAvoidSplitAssignmentsAugment(ej,
		  (KHE_AVOID_SPLIT_ASSIGNMENTS_MONITOR) d) )
	      return true;
	    break;

	  default:

	    MAssert(false, "KheResourceRepairEventResourceMonitorAugment: "
	      "d has a child which is not a resource value monitor");
	    break;
	}
      }
      break;

    default:

      MAssert(false, "KheResourceRepairEventResourceMonitorAugment: "
	"d is not a resource value monitor");
      break;
  }
  return false;
}


/*****************************************************************************/
/*                                                                           */
/*  bool KheResourceRepairResourceMonitorAugment(KHE_EJECTOR ej,             */
/*    KHE_MONITOR d)                                                         */
/*                                                                           */
/*  Augment function for repairing resource defects.                         */
/*                                                                           */
/*  Limiting this call to overload cases is still to do.                     */
/*                                                                           */
/*****************************************************************************/

bool KheResourceRepairResourceMonitorAugment(KHE_EJECTOR ej, KHE_MONITOR d)
{
  KHE_GROUP_MONITOR dgm;  int i;
  MAssert(KheMonitorCost(d) > 0,
    "KheResourceRepairResourceMonitorAugment: d does not have non-zero cost");
  switch( KheMonitorTag(d) )
  {
    case KHE_AVOID_CLASHES_MONITOR_TAG:

      return KheAvoidClashesAugment(ej, (KHE_AVOID_CLASHES_MONITOR) d);

    case KHE_AVOID_UNAVAILABLE_TIMES_MONITOR_TAG:

      return KheAvoidUnavailableTimesAugment(ej,
	(KHE_AVOID_UNAVAILABLE_TIMES_MONITOR) d);

    case KHE_LIMIT_IDLE_TIMES_MONITOR_TAG:

      return KheLimitIdleTimesAugment(ej, (KHE_LIMIT_IDLE_TIMES_MONITOR) d);

    case KHE_CLUSTER_BUSY_TIMES_MONITOR_TAG:

      return KheClusterBusyTimesAugment(ej, (KHE_CLUSTER_BUSY_TIMES_MONITOR) d);

    case KHE_LIMIT_BUSY_TIMES_MONITOR_TAG:

      return KheLimitBusyTimesAugment(ej, (KHE_LIMIT_BUSY_TIMES_MONITOR) d);

    case KHE_LIMIT_WORKLOAD_MONITOR_TAG:

      return KheLimitWorkloadAugment(ej, (KHE_LIMIT_WORKLOAD_MONITOR) d);

    case KHE_GROUP_MONITOR_TAG:

      dgm = (KHE_GROUP_MONITOR) d;
      KheGroupMonitorCopyDefects(dgm);
      for( i = 0;  i < KheGroupMonitorDefectCopyCount(dgm);  i++ )
      {
	d = KheGroupMonitorDefect(dgm, i);
	if( KheMonitorCost(d) > 0 ) switch( KheMonitorTag(d) )
	{
	  case KHE_AVOID_CLASHES_MONITOR_TAG:

	    if( KheAvoidClashesAugment(ej, (KHE_AVOID_CLASHES_MONITOR) d) )
	      return true;
	    break;

	  case KHE_AVOID_UNAVAILABLE_TIMES_MONITOR_TAG:

	    if( KheAvoidUnavailableTimesAugment(ej,
		(KHE_AVOID_UNAVAILABLE_TIMES_MONITOR) d) )
	      return true;
	    break;

	  case KHE_LIMIT_IDLE_TIMES_MONITOR_TAG:

	    if( KheLimitIdleTimesAugment(ej, (KHE_LIMIT_IDLE_TIMES_MONITOR) d) )
	      return true;
	    break;

	  case KHE_CLUSTER_BUSY_TIMES_MONITOR_TAG:

	    if( KheClusterBusyTimesAugment(ej,
		  (KHE_CLUSTER_BUSY_TIMES_MONITOR) d) )
	      return true;
	    break;

	  case KHE_LIMIT_BUSY_TIMES_MONITOR_TAG:

	    if( KheLimitBusyTimesAugment(ej, (KHE_LIMIT_BUSY_TIMES_MONITOR) d) )
	      return true;
	    break;

	  case KHE_LIMIT_WORKLOAD_MONITOR_TAG:

	    if( KheLimitWorkloadAugment(ej, (KHE_LIMIT_WORKLOAD_MONITOR) d) )
	      return true;
	    break;

	  default:

	    MAssert(false, "KheResourceRepairResourceMonitorAugment: "
	      "d has a child which is not a resource monitor");
	    break;
	}
      }
      break;

    default:

      MAssert(false, "KheResourceRepairResourceMonitorAugment: "
	"d is not a resource monitor");
      break;
  }
  return false;
}


/*****************************************************************************/
/*                                                                           */
/*  void KheEjectionChainRepairResources(KHE_TASKING tasking,                */
/*    bool preserve_invariant)                                               */
/*                                                                           */
/*  Ejection chain local search for improving resource assts.  If            */
/*  preserve_invariant is true, preserve the resource assignment invariant.  */
/*                                                                           */
/*****************************************************************************/

void KheEjectionChainRepairResources(KHE_TASKING tasking,
  bool preserve_invariant)
{
  KHE_EJECTOR ej;  KHE_GROUP_MONITOR gm, dgm;  KHE_SOLN soln;

  /* build the group monitors required by this algorithm */
  soln = KheTaskingSoln(tasking);
  gm = KheTaskingGroupMonitorsForTaskRepair(tasking, NULL,
    KHE_SUBTAG_EVENT_RESOURCE, "EventResourceGroupMonitor",
    KHE_SUBTAG_RESOURCE, "ResourceGroupMonitor",
    KHE_SUBTAG_TASKING, "TaskingMonitor", (KHE_GROUP_MONITOR) soln);
  dgm = KheSolnGroupDemandMonitors(soln, NULL, true, true, true,
    KHE_SUBTAG_DEMAND, "DemandGroupMonitor", (KHE_GROUP_MONITOR) soln);
  if( DEBUG3 )
  {
    fprintf(stderr, "[ KheEjectionChainRepairResources(tasking)\n");
    KheGroupMonitorDefectDebug(gm, 2, 4, stderr);
  }

  /* set up the ejector and run the algorithm */
  ej = KheEjectorMake(soln);
  KheEjectorAddSchedule(ej, 1, INT_MAX, false);
  KheEjectorAddSchedule(ej, 2, INT_MAX, false);
  KheEjectorAddSchedule(ej, 3, INT_MAX, false);
  KheEjectorAddSchedule(ej, INT_MAX, INT_MAX, false);
  if( preserve_invariant )
    KheEjectorAddMonitorCostLimit(ej, (KHE_MONITOR) dgm,
      KheMonitorCost((KHE_MONITOR) dgm));
  KheEjectorAddGroupAugment(ej, KHE_SUBTAG_EVENT_RESOURCE,
    &KheResourceRepairEventResourceMonitorAugment);
  KheEjectorAddGroupAugment(ej, KHE_SUBTAG_RESOURCE,
    &KheResourceRepairResourceMonitorAugment);
  KheEjectorSolve(ej, KHE_EJECTOR_FIRST_SUCCESS, gm);
  KheEjectorDelete(ej);

  /* remove group monitors and exit */
  if( DEBUG3 )
  {
    fprintf(stderr, "  final defects:\n");
    KheGroupMonitorDefectDebug(gm, 2, 4, stderr);
    fprintf(stderr, "] KheEjectionChainRepairResources returning\n");
  }
  KheUnGroupMonitors(gm);
  KheUnGroupMonitors(dgm);
}
