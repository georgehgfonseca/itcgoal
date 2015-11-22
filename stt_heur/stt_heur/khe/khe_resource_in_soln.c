
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
/*  FILE:         khe_resource_in_soln.c                                     */
/*  DESCRIPTION:  A resource as viewed from within a solution                */
/*                                                                           */
/*****************************************************************************/
#include "khe_interns.h"

#define DEBUG1 0
#define DEBUG2 0


/*****************************************************************************/
/*                                                                           */
/*  KHE_WORKLOAD_REQUIREMENT - one workload requirement                      */
/*                                                                           */
/*****************************************************************************/

typedef struct khe_workload_requirement_rec *KHE_WORKLOAD_REQUIREMENT;
typedef MARRAY(KHE_WORKLOAD_REQUIREMENT) ARRAY_KHE_WORKLOAD_REQUIREMENT;

struct khe_workload_requirement_rec {
  int				count;		/* number required           */
  KHE_TIME_GROUP		time_group;	/* time group required       */
  ARRAY_KHE_WORKLOAD_REQUIREMENT children;	/* child requirements        */
};


/*****************************************************************************/
/*                                                                           */
/*  KHE_RESOURCE_IN_SOLN - a resource monitor                                */
/*                                                                           */
/*  Implementation note.  All monitors, attached and unattached, and         */
/*  including workload demand monitors, go on the all_monitors list.         */
/*  But only attached non-workload monitors go on the attached_monitors      */
/*  list.  Attached workload demand monitors go only into their chunks.      */
/*                                                                           */
/*****************************************************************************/

struct khe_resource_in_soln_rec {
  KHE_SOLN			soln;			/* encl. solution    */
  KHE_RESOURCE			resource;		/* monitored resource*/
  ARRAY_KHE_TASK		tasks;			/* assigned resource */
  KHE_TIMETABLE_MONITOR		timetable_monitor;	/* its timetable     */
  ARRAY_KHE_MONITOR		all_monitors;		/* all monitors      */
  ARRAY_KHE_MONITOR		attached_monitors;	/* attached monitors */
  ARRAY_KHE_WORKLOAD_REQUIREMENT	workload_requirements;
  ARRAY_KHE_MATCHING_DEMAND_CHUNK	workload_demand_chunks;
  bool				workload_requirements_active;
  KHE_RESOURCE_IN_SOLN		copy;
};


/*****************************************************************************/
/*                                                                           */
/*  Submodule "construction and query"                                       */
/*                                                                           */
/*****************************************************************************/

/*****************************************************************************/
/*                                                                           */
/* KHE_RESOURCE_IN_SOLN KheResourceInSolnMake(KHE_SOLN soln, KHE_RESOURCE r) */
/*                                                                           */
/*  Make a new resource in soln for soln that monitors r, initially with     */
/*  no monitors except the timetable, and that is unattached anyway.         */
/*                                                                           */
/*****************************************************************************/

KHE_RESOURCE_IN_SOLN KheResourceInSolnMake(KHE_SOLN soln, KHE_RESOURCE r)
{
  KHE_RESOURCE_IN_SOLN res;
  MMake(res);
  res->soln = soln;
  res->resource = r;
  MArrayInit(res->tasks);
  res->timetable_monitor = KheTimetableMonitorMake(soln, res, NULL);
  MArrayInit(res->all_monitors);
  MArrayInit(res->attached_monitors);
  MArrayInit(res->workload_requirements);
  MArrayInit(res->workload_demand_chunks);
  res->workload_requirements_active = false;;
  res->copy = NULL;
  return res;
}


/*****************************************************************************/
/*                                                                           */
/* KHE_RESOURCE_IN_SOLN KheResourceInSolnCopyPhase1(KHE_RESOURCE_IN_SOLN rs) */
/*                                                                           */
/*  Carry out Phase 1 of copying rs.                                         */
/*                                                                           */
/*****************************************************************************/

static KHE_WORKLOAD_REQUIREMENT KheWorkloadRequirementCopy(
  KHE_WORKLOAD_REQUIREMENT wr)
{
  KHE_WORKLOAD_REQUIREMENT res;
  MMake(res);
  res->count = wr->count;
  res->time_group = wr->time_group;
  MArrayInit(res->children);
  return res;
}

KHE_RESOURCE_IN_SOLN KheResourceInSolnCopyPhase1(KHE_RESOURCE_IN_SOLN rs)
{
  KHE_RESOURCE_IN_SOLN copy;  KHE_MONITOR m;  KHE_TASK task;  int i;
  KHE_WORKLOAD_REQUIREMENT wr;  KHE_MATCHING_DEMAND_CHUNK dc;
  if( rs->copy == NULL )
  {
    MMake(copy);
    rs->copy = copy;
    copy->soln = KheSolnCopyPhase1(rs->soln);
    copy->resource = rs->resource;
    MArrayInit(copy->tasks);
    MArrayForEach(rs->tasks, &task, &i)
      MArrayAddLast(copy->tasks, KheTaskCopyPhase1(task));
    copy->timetable_monitor =
      KheTimetableMonitorCopyPhase1(rs->timetable_monitor);
    MArrayInit(copy->all_monitors);
    MArrayForEach(rs->all_monitors, &m, &i)
      MArrayAddLast(copy->all_monitors, KheMonitorCopyPhase1(m));
    MArrayInit(copy->attached_monitors);
    MArrayForEach(rs->attached_monitors, &m, &i)
      MArrayAddLast(copy->attached_monitors, KheMonitorCopyPhase1(m));
    MArrayInit(copy->workload_requirements);
    MArrayForEach(rs->workload_requirements, &wr, &i)
      MArrayAddLast(copy->workload_requirements,
	KheWorkloadRequirementCopy(wr));
    MArrayInit(copy->workload_demand_chunks);
    MArrayForEach(rs->workload_demand_chunks, &dc, &i)
      MArrayAddLast(copy->workload_demand_chunks,
	KheMatchingDemandChunkCopyPhase1(dc));
    MAssert(!rs->workload_requirements_active,
      "KheSolnCopy called when currently adding workload requirements");
    copy->workload_requirements_active = false;
    copy->copy = NULL;
  }
  return rs->copy;
}


/*****************************************************************************/
/*                                                                           */
/*  void KheResourceInSolnCopyPhase2(KHE_RESOURCE_IN_SOLN rs)                */
/*                                                                           */
/*  Carry out Phase 2 of copying rs.                                         */
/*                                                                           */
/*****************************************************************************/

void KheResourceInSolnCopyPhase2(KHE_RESOURCE_IN_SOLN rs)
{
  KHE_TASK task;  KHE_MONITOR m;  int i;  KHE_MATCHING_DEMAND_CHUNK dc;
  if( rs->copy != NULL )
  {
    rs->copy = NULL;
    MArrayForEach(rs->tasks, &task, &i)
      KheTaskCopyPhase2(task);
    KheTimetableMonitorCopyPhase2(rs->timetable_monitor);
    MArrayForEach(rs->all_monitors, &m, &i)
      KheMonitorCopyPhase2(m);
    MArrayForEach(rs->attached_monitors, &m, &i)
      KheMonitorCopyPhase2(m);
    MArrayForEach(rs->workload_demand_chunks, &dc, &i)
      KheMatchingDemandChunkCopyPhase2(dc);
  }
}


/*****************************************************************************/
/*                                                                           */
/*  KHE_SOLN KheResourceInSolnSoln(KHE_RESOURCE_IN_SOLN rs)                  */
/*                                                                           */
/*  Return the solution that rs is a part of.                                */
/*                                                                           */
/*****************************************************************************/

KHE_SOLN KheResourceInSolnSoln(KHE_RESOURCE_IN_SOLN rs)
{
  return rs->soln;
}


/*****************************************************************************/
/*                                                                           */
/*  KHE_RESOURCE KheResourceInSolnResource(KHE_RESOURCE_IN_SOLN rs)          */
/*                                                                           */
/*  Return the resource that rs is monitoring.                               */
/*                                                                           */
/*****************************************************************************/

KHE_RESOURCE KheResourceInSolnResource(KHE_RESOURCE_IN_SOLN rs)
{
  return rs->resource;
}


/*****************************************************************************/
/*                                                                           */
/*  char *KheResourceInSolnId(KHE_RESOURCE_IN_SOLN rs)                       */
/*                                                                           */
/*  Return the Id of rs's resource, or "-" if none.                          */
/*                                                                           */
/*****************************************************************************/

char *KheResourceInSolnId(KHE_RESOURCE_IN_SOLN rs)
{
  return KheResourceId(rs->resource)==NULL ? "-" : KheResourceId(rs->resource);
}


/*****************************************************************************/
/*                                                                           */
/*  void KheResourceInSolnDelete(KHE_RESOURCE_IN_SOLN rs)                    */
/*                                                                           */
/*  Delete rs.                                                               */
/*                                                                           */
/*****************************************************************************/

static void KheResourceInSolnWorkloadRequirementDelete(
  KHE_WORKLOAD_REQUIREMENT wr);

void KheResourceInSolnDelete(KHE_RESOURCE_IN_SOLN rs)
{
  int i;  KHE_MATCHING_DEMAND_CHUNK dc;

  /* tasks should be deleted by now */
  MAssert(MArraySize(rs->tasks) == 0,
    "KheResourceInSolnDelete internal error");
  MArrayFree(rs->tasks);

  /* monitors deleted separately; just free the arrays here */
  MArrayFree(rs->all_monitors);
  MArrayFree(rs->attached_monitors);

  /* free the workload requirements */
  while( MArraySize(rs->workload_requirements) > 0 )
    KheResourceInSolnWorkloadRequirementDelete(
      MArrayRemoveLast(rs->workload_requirements));

  /* free the workload demand chunks (each should be empty by now) */
  MArrayForEach(rs->workload_demand_chunks, &dc, &i)
    KheMatchingDemandChunkDelete(dc);
  MArrayFree(rs->workload_demand_chunks);

  MFree(rs);
}


/*****************************************************************************/
/*                                                                           */
/*  Submodule "monitors"                                                     */
/*                                                                           */
/*****************************************************************************/

/*****************************************************************************/
/*                                                                           */
/*  void KheResourceInSolnAttachMonitor(KHE_RESOURCE_IN_SOLN rs,             */
/*    KHE_MONITOR m)                                                         */
/*                                                                           */
/*  Attach m (not a workload demand monitor) to rs.                          */
/*                                                                           */
/*****************************************************************************/

void KheResourceInSolnAttachMonitor(KHE_RESOURCE_IN_SOLN rs, KHE_MONITOR m)
{
  KHE_TASK task;  int i;
  switch( KheMonitorTag(m) )
  {
    case KHE_LIMIT_WORKLOAD_MONITOR_TAG:
    case KHE_TIMETABLE_MONITOR_TAG:

      /* attach directly to rs */
      MArrayAddLast(rs->attached_monitors, m);
      MArrayForEach(rs->tasks, &task, &i)
	KheMonitorAssignResource(m, task, rs->resource);
      break;

    case KHE_AVOID_CLASHES_MONITOR_TAG:
    case KHE_AVOID_UNAVAILABLE_TIMES_MONITOR_TAG:
    case KHE_LIMIT_IDLE_TIMES_MONITOR_TAG:
    case KHE_CLUSTER_BUSY_TIMES_MONITOR_TAG:
    case KHE_LIMIT_BUSY_TIMES_MONITOR_TAG:

      /* attach to timetable, which much itself be attached first */
      if( !KheMonitorAttachedToSoln((KHE_MONITOR) rs->timetable_monitor) )
        KheMonitorAttachToSoln((KHE_MONITOR) rs->timetable_monitor);
      KheTimetableMonitorAttachMonitor(rs->timetable_monitor, m);
      break;

    case KHE_ASSIGN_RESOURCE_MONITOR_TAG:
    case KHE_ASSIGN_TIME_MONITOR_TAG:
    case KHE_SPLIT_EVENTS_MONITOR_TAG:
    case KHE_DISTRIBUTE_SPLIT_EVENTS_MONITOR_TAG:
    case KHE_PREFER_RESOURCES_MONITOR_TAG:
    case KHE_PREFER_TIMES_MONITOR_TAG:
    case KHE_AVOID_SPLIT_ASSIGNMENTS_MONITOR_TAG:
    case KHE_SPREAD_EVENTS_MONITOR_TAG:
    case KHE_LINK_EVENTS_MONITOR_TAG:
    case KHE_TIME_GROUP_MONITOR_TAG:
    /* case KHE_MATCHING_MONITOR_TAG: */
    case KHE_ORDINARY_DEMAND_MONITOR_TAG:
    case KHE_WORKLOAD_DEMAND_MONITOR_TAG:
    default:

      /* unexpected monitor type */
      MAssert(false, "KheResourceInSolnAttachMonitor unexpected monitor tag");
  }
}


/*****************************************************************************/
/*                                                                           */
/*  void KheResourceInSolnDetachMonitor(KHE_RESOURCE_IN_SOLN rs,             */
/*    KHE_MONITOR m)                                                         */
/*                                                                           */
/*  Detach m (not a workload demand monitor) from rs.                        */
/*                                                                           */
/*****************************************************************************/

void KheResourceInSolnDetachMonitor(KHE_RESOURCE_IN_SOLN rs, KHE_MONITOR m)
{
  KHE_TASK task;  int i, pos;
  switch( KheMonitorTag(m) )
  {
    case KHE_LIMIT_WORKLOAD_MONITOR_TAG:
    case KHE_TIMETABLE_MONITOR_TAG:

      /* detach directly from rs */
      if( !MArrayContains(rs->attached_monitors, m, &pos) )
	MAssert(false, "KheResourceInSolnDetachMonitor internal error");
      MArrayRemove(rs->attached_monitors, pos);
      MArrayForEach(rs->tasks, &task, &i)
	KheMonitorUnAssignResource(m, task, rs->resource);
      break;

    case KHE_AVOID_CLASHES_MONITOR_TAG:
    case KHE_AVOID_UNAVAILABLE_TIMES_MONITOR_TAG:
    case KHE_LIMIT_IDLE_TIMES_MONITOR_TAG:
    case KHE_CLUSTER_BUSY_TIMES_MONITOR_TAG:
    case KHE_LIMIT_BUSY_TIMES_MONITOR_TAG:

      /* detach from timetable monitor */
      KheTimetableMonitorDetachMonitor(rs->timetable_monitor, m);
      break;

    case KHE_ASSIGN_RESOURCE_MONITOR_TAG:
    case KHE_ASSIGN_TIME_MONITOR_TAG:
    case KHE_SPLIT_EVENTS_MONITOR_TAG:
    case KHE_DISTRIBUTE_SPLIT_EVENTS_MONITOR_TAG:
    case KHE_PREFER_RESOURCES_MONITOR_TAG:
    case KHE_PREFER_TIMES_MONITOR_TAG:
    case KHE_AVOID_SPLIT_ASSIGNMENTS_MONITOR_TAG:
    case KHE_SPREAD_EVENTS_MONITOR_TAG:
    case KHE_LINK_EVENTS_MONITOR_TAG:
    case KHE_TIME_GROUP_MONITOR_TAG:
    /* case KHE_MATCHING_MONITOR_TAG: */
    case KHE_ORDINARY_DEMAND_MONITOR_TAG:
    case KHE_WORKLOAD_DEMAND_MONITOR_TAG:
    default:

      MAssert(false, "KheResourceInSolnAttachMonitor unexpected monitor tag");
  }
}


/*****************************************************************************/
/*                                                                           */
/*  Submodule "monitoring calls"                                             */
/*                                                                           */
/*****************************************************************************/

/*****************************************************************************/
/*                                                                           */
/*  void KheResourceInSolnSplitTask(KHE_RESOURCE_IN_SOLN rs,                 */
/*    KHE_TASK task1, KHE_TASK task2)                                        */
/*                                                                           */
/*  Inform rs that task1 and task2 (which rs's resource is assigned to) are  */
/*  splitting.                                                               */
/*                                                                           */
/*****************************************************************************/

void KheResourceInSolnSplitTask(KHE_RESOURCE_IN_SOLN rs,
  KHE_TASK task1, KHE_TASK task2)
{
  KHE_MONITOR m;  int i;
  if( DEBUG1 )
    fprintf(stderr, "[ KheResourceInSolnSplitTask(%s)\n",
      KheResourceId(rs->resource) != NULL ? KheResourceId(rs->resource) : "-");
  MArrayAddLast(rs->tasks, task2);
  MArrayForEach(rs->attached_monitors, &m, &i)
    KheMonitorSplitTask(m, task1, task2);
  if( DEBUG1 )
    fprintf(stderr, "] KheResourceInSolnSplitTask\n");
}


/*****************************************************************************/
/*                                                                           */
/*  void KheResourceInSolnMergeTask(KHE_RESOURCE_IN_SOLN rs,                 */
/*    KHE_TASK task1, KHE_TASK task2)                                        */
/*                                                                           */
/*  Inform rs that task1 and task2 (which rs's resource is assigned to) are  */
/*  merging.                                                                 */
/*                                                                           */
/*****************************************************************************/

void KheResourceInSolnMergeTask(KHE_RESOURCE_IN_SOLN rs,
  KHE_TASK task1, KHE_TASK task2)
{
  KHE_MONITOR m;  int i, pos;
  if( DEBUG1 )
    fprintf(stderr, "[ KheResourceInSolnMergeTask(%s)\n",
      KheResourceId(rs->resource) != NULL ? KheResourceId(rs->resource) : "-");
  if( !MArrayContains(rs->tasks, task2, &pos) )
    MAssert(false, "KheResourceInSolnMergeTask internal error");
  MArrayRemove(rs->tasks, pos);
  MArrayForEach(rs->attached_monitors, &m, &i)
    KheMonitorMergeTask(m, task1, task2);
  if( DEBUG1 )
    fprintf(stderr, "] KheResourceInSolnMergeTask\n");
}


/*****************************************************************************/
/*                                                                           */
/*  void KheResourceInSolnAssignResource(KHE_RESOURCE_IN_SOLN rs,            */
/*    KHE_TASK task)                                                         */
/*                                                                           */
/*  Inform rs that its resource has just been assigned to task.              */
/*                                                                           */
/*****************************************************************************/

void KheResourceInSolnAssignResource(KHE_RESOURCE_IN_SOLN rs, KHE_TASK task)
{
  KHE_MONITOR m;  int i;
  if( DEBUG1 )
    fprintf(stderr, "[ KheResourceInSolnAssignResource(%s)\n",
      KheResourceId(rs->resource) != NULL ? KheResourceId(rs->resource) : "-");
  MArrayAddLast(rs->tasks, task);
  MArrayForEach(rs->attached_monitors, &m, &i)
    KheMonitorAssignResource(m, task, rs->resource);
  if( DEBUG1 )
    fprintf(stderr, "] KheResourceInSolnAssignResource\n");
}


/*****************************************************************************/
/*                                                                           */
/*  void KheResourceInSolnUnAssignResource(KHE_RESOURCE_IN_SOLN rs,          */
/*    KHE_TASK task)                                                         */
/*                                                                           */
/*  Inform rs that its resource has just been unassigned from task.          */
/*                                                                           */
/*****************************************************************************/

void KheResourceInSolnUnAssignResource(KHE_RESOURCE_IN_SOLN rs, KHE_TASK task)
{
  KHE_MONITOR m;  int i;  KHE_TASK task2;
  if( DEBUG1 )
    fprintf(stderr, "[ KheResourceInSolnUnAssignResource(%s)\n",
      KheResourceId(rs->resource) != NULL ? KheResourceId(rs->resource) : "-");
  MArrayForEach(rs->attached_monitors, &m, &i)
    KheMonitorUnAssignResource(m, task, rs->resource);
  MArrayForEachReverse(rs->tasks, &task2, &i)
    if( task2 == task )
      break;
  MAssert(i >= 0, "KheResourceInSolnUnAssignResource internal error");
  task2 = MArrayRemoveAndPlug(rs->tasks, i);
  if( DEBUG1 )
    fprintf(stderr, "] KheResourceInSolnUnssignResource\n");
}


/*****************************************************************************/
/*                                                                           */
/*  void KheResourceInSolnAssignTime(KHE_RESOURCE_IN_SOLN rs,                */
/*    KHE_TASK task, int assigned_time_index)                                */
/*                                                                           */
/*  Inform rs that the solution event above task has just been assigned the  */
/*  time with this index, while task is currently assigned rs's resource.    */
/*                                                                           */
/*****************************************************************************/

void KheResourceInSolnAssignTime(KHE_RESOURCE_IN_SOLN rs,
  KHE_TASK task, int assigned_time_index)
{
  KHE_MONITOR m;  int i;
  if( DEBUG1 )
    fprintf(stderr, "[ KheResourceInSolnAssignTime(%s, task, %d)\n",
      KheResourceId(rs->resource) != NULL ? KheResourceId(rs->resource) : "-",
      assigned_time_index);
  MArrayForEach(rs->attached_monitors, &m, &i)
    KheMonitorTaskAssignTime(m, task, assigned_time_index);
  if( DEBUG1 )
    fprintf(stderr, "] KheResourceInSolnAssignTime\n");
}


/*****************************************************************************/
/*                                                                           */
/*  void KheResourceInSolnUnAssignTime(KHE_RESOURCE_IN_SOLN rs,              */
/*    KHE_TASK task, int assigned_time_index)                                */
/*                                                                           */
/*  Inform rs that the solution event above task has just been unassigned    */
/*  the time with this index, while task is currently assigned rs's resource.*/
/*                                                                           */
/*****************************************************************************/

void KheResourceInSolnUnAssignTime(KHE_RESOURCE_IN_SOLN rs,
  KHE_TASK task, int assigned_time_index)
{
  KHE_MONITOR m;  int i;
  if( DEBUG1 )
    fprintf(stderr, "[ KheResourceInSolnUnAssignTime(%s, task, %d)\n",
      KheResourceId(rs->resource) != NULL ? KheResourceId(rs->resource) : "-",
      assigned_time_index);
  MArrayForEach(rs->attached_monitors, &m, &i)
    KheMonitorTaskUnAssignTime(m, task, assigned_time_index);
  if( DEBUG1 )
    fprintf(stderr, "] KheResourceInSolnUnAssignTime\n");
}


/*****************************************************************************/
/*                                                                           */
/*  Submodule "exclude preassignments flag (for matchings)"                  */
/*                                                                           */
/*****************************************************************************/

/*****************************************************************************/
/*                                                                           */
/*  void KheResourceInSolnMatchingSetExcludePreassignments(                  */
/*    KHE_RESOURCE_IN_SOLN rs, bool value)                                   */
/*                                                                           */
/*  Set the exclude preassignments flag of rs to value.                      */
/*                                                                           */
/*****************************************************************************/

/* ***
void KheResourceInSolnMatchingSetExcludePreassignments(
  KHE_RESOURCE_IN_SOLN rs, bool value)
{
  rs->exclude_preassignments = value;
}
*** */


/*****************************************************************************/
/*                                                                           */
/*  bool KheResourceInSolnMatchingExcludePreassignments(                     */
/*    KHE_RESOURCE_IN_SOLN rs)                                               */
/*                                                                           */
/*  Return the exclude preassignments flag of rs.                            */
/*                                                                           */
/*****************************************************************************/

/* ***
bool KheResourceInSolnMatchingExcludePreassignments(
  KHE_RESOURCE_IN_SOLN rs)
{
  return rs->exclude_preassignments;
}
*** */


/*****************************************************************************/
/*                                                                           */
/*  Submodule "workload requirements (for matchings)"                        */
/*                                                                           */
/*****************************************************************************/

/*****************************************************************************/
/*                                                                           */
/*  int KheResourceInSolnWorkloadRequirementCount(KHE_RESOURCE_IN_SOLN rs)   */
/*                                                                           */
/*  Return the number of workload requirements stored in rs.                 */
/*                                                                           */
/*****************************************************************************/

int KheResourceInSolnWorkloadRequirementCount(KHE_RESOURCE_IN_SOLN rs)
{
  return MArraySize(rs->workload_requirements);
}


/*****************************************************************************/
/*                                                                           */
/*  void KheResourceInSolnWorkloadRequirement(KHE_RESOURCE_IN_SOLN rs,       */
/*    int i, int *num, KHE_TIME_GROUP *tg)                                   */
/*                                                                           */
/*  Return the i'th workload requirement stored in rs.                       */
/*                                                                           */
/*****************************************************************************/

void KheResourceInSolnWorkloadRequirement(KHE_RESOURCE_IN_SOLN rs,
  int i, int *num, KHE_TIME_GROUP *tg)
{
  KHE_WORKLOAD_REQUIREMENT wr;
  wr = MArrayGet(rs->workload_requirements, i);
  *num = wr->count;
  *tg = wr->time_group;
}


/*****************************************************************************/
/*                                                                           */
/*  void KheResourceInSolnWorkloadRequirementDelete(                         */
/*    KHE_WORKLOAD_REQUIREMENT wr)                                           */
/*                                                                           */
/*  Delete wr.                                                               */
/*                                                                           */
/*****************************************************************************/

static void KheResourceInSolnWorkloadRequirementDelete(
  KHE_WORKLOAD_REQUIREMENT wr)
{
  MArrayFree(wr->children);
  MFree(wr);
}


/*****************************************************************************/
/*                                                                           */
/*  void KheResourceInSolnBeginWorkloadRequirements(KHE_RESOURCE_IN_SOLN rs) */
/*                                                                           */
/*  Begin adding workload requirements to rs.                                */
/*                                                                           */
/*****************************************************************************/

void KheResourceInSolnBeginWorkloadRequirements(KHE_RESOURCE_IN_SOLN rs)
{
  MAssert(!rs->workload_requirements_active,
    "KheSolnMatchingBeginWorkloadRequirements called out of order");
  while( MArraySize(rs->workload_requirements) > 0 )
    KheResourceInSolnWorkloadRequirementDelete(
      MArrayRemoveLast(rs->workload_requirements));
  rs->workload_requirements_active = true;
}


/*****************************************************************************/
/*                                                                           */
/*  void KheResourceInSolnAddWorkloadRequirement(KHE_RESOURCE_IN_SOLN rs,    */
/*    int num, KHE_TIME_GROUP tg)                                            */
/*                                                                           */
/*  Append a workload requirement to the workload requirements stored in rs. */
/*                                                                           */
/*****************************************************************************/

void KheResourceInSolnAddWorkloadRequirement(KHE_RESOURCE_IN_SOLN rs,
  int num, KHE_TIME_GROUP tg)
{
  KHE_WORKLOAD_REQUIREMENT wr;
  MAssert(rs->workload_requirements_active,
    "KheSolnMatchingAddWorkloadRequirement called out of order");
  MAssert(num >= 0 && num <= KheTimeGroupTimeCount(tg),
    "KheSolnMatchingAddWorkloadRequirement invalid num");
  MMake(wr);
  wr->count = num;
  wr->time_group = tg;
  MArrayInit(wr->children);
  MArrayAddLast(rs->workload_requirements, wr);
}


/*****************************************************************************/
/*                                                                           */
/*  void KheWorkloadRequirementInsertIntoForest(                             */
/*    ARRAY_KHE_WORKLOAD_REQUIREMENT *roots, KHE_WORKLOAD_REQUIREMENT wr)    */
/*                                                                           */
/*  Insert wr into roots.                                                    */
/*                                                                           */
/*****************************************************************************/

static void KheWorkloadRequirementInsertIntoForest(
  ARRAY_KHE_WORKLOAD_REQUIREMENT *roots, KHE_WORKLOAD_REQUIREMENT wr)
{
  int touch_count, superset_count, i;  KHE_WORKLOAD_REQUIREMENT touch_wr, wr2;

  /* find out how many roots wr touches and is a superset of */
  touch_count = 0;
  touch_wr = NULL;
  superset_count = 0;
  MArrayForEach(*roots, &wr2, &i)
    if( !KheTimeGroupDisjoint(wr->time_group, wr2->time_group) )
    {
      touch_count++;
      touch_wr = wr2;
      if( KheTimeGroupSubset(wr2->time_group, wr->time_group) )
	superset_count++;
    }

  if( superset_count == touch_count )
  {
    /* wr is a superset of everything it touches; move them to below wr */
    MArrayForEach(*roots, &wr2, &i)
      if( KheTimeGroupSubset(wr2->time_group, wr->time_group) )
      {
	MArrayAddLast(wr->children, wr2);
	MArrayRemove(*roots, i);
	i--;
      }
    MArrayAddLast(*roots, wr);
  }
  else if( touch_count == 1 &&
      KheTimeGroupSubset(wr->time_group, touch_wr->time_group) )
  {
    /* wr belongs below touch_wr */
    KheWorkloadRequirementInsertIntoForest(&touch_wr->children, wr);
  }
  else
  {
    /* no other legal possibilities, so we abandon wr here */
  }
}


/*****************************************************************************/
/*                                                                           */
/*  int KheWorkloadRequirementToDemandNodes(KHE_WORKLOAD_REQUIREMENT wr,     */
/*    KHE_RESOURCE_IN_SOLN rs)                                               */
/*                                                                           */
/*  Traverse the tree rooted at wr in postorder, adding demand chunks and    */
/*  nodes to rs as required, and returning the number of nodes added by      */
/*  wr and its descendants.                                                  */
/*                                                                           */
/*  Implementation note.  There is one chunk for each workload requirement   */
/*  giving rise to at least one demand node:                                 */
/*                                                                           */
/*     dc->impl       time group of requirement                              */
/*     dc->base       0                                                      */
/*     dc->increment  resource_count                                         */
/*     dc->domain     indexes of time group times                            */
/*                                                                           */
/*     dn->impl       resource                                               */
/*     dn->domain     indexes of resource's singleton resource group         */
/*                                                                           */
/*****************************************************************************/

static int KheWorkloadRequirementToDemandNodes(KHE_WORKLOAD_REQUIREMENT wr,
  KHE_RESOURCE_IN_SOLN rs)
{
  int child_count, root_count, i, resource_count;  KHE_MATCHING m;
  KHE_WORKLOAD_REQUIREMENT child_wr;  KHE_MATCHING_DEMAND_CHUNK dc;
  KHE_WORKLOAD_DEMAND_MONITOR wdm;

  /* build demand nodes in proper descendants and count how many */
  child_count = 0;
  MArrayForEach(wr->children, &child_wr, &i)
    child_count += KheWorkloadRequirementToDemandNodes(child_wr, rs);

  /* find out how many we need here at the root */
  root_count = KheTimeGroupTimeCount(wr->time_group) - wr->count - child_count;
  if( root_count < 0 )
    root_count = 0;

  /* add one chunk with root_count demand nodes, if root_count > 0 */
  if( root_count > 0 )
  {
    m = KheSolnMatching(rs->soln);
    resource_count = KheInstanceResourceCount(KheSolnInstance(rs->soln));
    dc = KheMatchingDemandChunkMake(m, NULL, 0, resource_count,
      KheTimeGroupTimeIndexes(wr->time_group));
    MArrayAddLast(rs->workload_demand_chunks, dc);
    for( i = 0;  i < root_count;  i++ )
    {
      wdm = KheWorkloadDemandMonitorMake(rs->soln, dc, rs, wr->time_group);
      KheGroupMonitorAddChildMonitor((KHE_GROUP_MONITOR) rs->soln,
	(KHE_MONITOR) wdm);
      /* *** done by KheWorkloadDemandMonitorMake
      MArrayAddLast(rs->all_monitors, (KHE_MONITOR) wdm);
      *** */
      KheMonitorAttachToSoln((KHE_MONITOR) wdm);
    }
  }

  /* return the total number of demand nodes added at wr or below it */
  return child_count + root_count;
}


/*****************************************************************************/
/*                                                                           */
/*  void KheResourceInSolnEndWorkloadRequirements(KHE_RESOURCE_IN_SOLN rs)   */
/*                                                                           */
/*  End adding workload requirements to rs.  This is the signal for          */
/*  calculating the demand chunks and nodes.                                 */
/*                                                                           */
/*****************************************************************************/

void KheResourceInSolnEndWorkloadRequirements(KHE_RESOURCE_IN_SOLN rs)
{
  ARRAY_KHE_WORKLOAD_REQUIREMENT roots;  int i;  KHE_WORKLOAD_REQUIREMENT wr;
  KHE_MONITOR m;
  if( DEBUG2 )
  {
    fprintf(stderr, "[ KheResourceInSolnEndWorkloadRequirements(%s)\n",
      KheResourceId(rs->resource) == NULL ? "-" : KheResourceId(rs->resource));
    fprintf(stderr, "  initial monitors:\n");
    MArrayForEach(rs->all_monitors, &m, &i)
      KheMonitorDebug(m, 2, 4, stderr);
    fprintf(stderr, "  initial workload requirements:\n");
    MArrayForEach(rs->workload_requirements, &wr, &i)
    {
      fprintf(stderr, "    %d ", wr->count);
      KheTimeGroupDebug(wr->time_group, 1, -1, stderr);
      fprintf(stderr, "\n");
    }
  }
  MAssert(rs->workload_requirements_active,
    "KheSolnMatchingEndWorkloadRequirements called out of order");
  rs->workload_requirements_active = false;

  /* delete old workload demand monitors */
  MArrayForEach(rs->all_monitors, &m, &i)
    if( KheMonitorTag(m) == KHE_WORKLOAD_DEMAND_MONITOR_TAG )
    {
      KheWorkloadDemandMonitorDelete((KHE_WORKLOAD_DEMAND_MONITOR) m);
      i--;
    }

  /* delete old workload demand chunks */
  while( MArraySize(rs->workload_demand_chunks) > 0 )
    KheMatchingDemandChunkDelete(MArrayRemoveLast(rs->workload_demand_chunks));

  /* build the forest */
  MArrayInit(roots);
  MArrayForEach(rs->workload_requirements, &wr, &i)
  {
    MArrayClear(wr->children);
    KheWorkloadRequirementInsertIntoForest(&roots, wr);
  }

  /* traverse the forest, adding workload demand chunks and nodes */
  MArrayForEach(roots, &wr, &i)
    KheWorkloadRequirementToDemandNodes(wr, rs);
  MArrayFree(roots);
  if( DEBUG2 )
  {
    fprintf(stderr, "  final monitors:\n");
    MArrayForEach(rs->all_monitors, &m, &i)
      KheMonitorDebug(m, 2, 4, stderr);
    fprintf(stderr, "] KheResourceInSolnEndWorkloadRequirements returning\n");
  }
}


/*****************************************************************************/
/*                                                                           */
/*  bool KheTaskInSolnWorkloadActive(KHE_RESOURCE_IN_SOLN rs)                */
/*                                                                           */
/*  Return true if rs is current accepting workload requirements.            */
/*                                                                           */
/*****************************************************************************/

/* ***
bool KheTaskInSolnWorkloadActive(KHE_RESOURCE_IN_SOLN rs)
{
  return rs->workload_requirements_active;
}
*** */


/*****************************************************************************/
/*                                                                           */
/*  void KheResourceInSolnMatchingSetType(KHE_RESOURCE_IN_SOLN rs,           */
/*    KHE_MATCHING_TYPE mt)                                                  */
/*                                                                           */
/*  The type of the current matching is changing to mt.  Change the          */
/*  workload demand nodes of rs accordingly, including creating or           */
/*  deleting them if required.                                               */
/*                                                                           */
/*  Implementation note.  If the matching type is empty, there are no        */
/*  workload demand nodes.  Otherwise, there may be, but they will be        */
/*  the same whatever the type.                                              */
/*                                                                           */
/*****************************************************************************/

/* *** workload demand monitors are unaffected by type
void KheResourceInSolnMatchingSetType(KHE_RESOURCE_IN_SOLN rs,
  KHE_MATCHING_TYPE mt)
{
  ARRAY_KHE_WORKLOAD_REQUIREMENT roots;  int i;  KHE_WORKLOAD_REQUIREMENT wr;

  ** this is wrong, these are constant over all types **
  if( MArraySize(rs->workload_demand_chunks) == 0 )
  {
    ** build the forest **
    MArrayInit(roots);
    MArrayForEach(rs->workload_requirements, &wr, &i)
    {
      MArrayClear(wr->children);
      KheWorkloadRequirementInsertIntoForest(&roots, wr);
    }

    ** traverse the forest, adding workload demand chunks+nodes **
    MArrayForEach(roots, &wr, &i)
      KheWorkloadRequirementToDemandNodes(wr, rs);
    MArrayFree(roots);
  }
}
*** */


/*****************************************************************************/
/*                                                                           */
/*  void KheResourceInSolnMatchingSetWeight(KHE_RESOURCE_IN_SOLN rs,         */
/*    KHE_COST new_weight)                                                   */
/*                                                                           */
/*  Update the attached matching demand monitors of rs to reflect the fact   */
/*  that their weight is changing to new_weight.                             */
/*                                                                           */
/*  Implementation note.  Array all_monitors contains all monitors, but we   */
/*  want just the attached demand monitors, so we traverse their chunks.     */
/*                                                                           */
/*****************************************************************************/

void KheResourceInSolnMatchingSetWeight(KHE_RESOURCE_IN_SOLN rs,
  KHE_COST new_weight)
{
  int i, j;  KHE_MATCHING_DEMAND_CHUNK dc;  KHE_WORKLOAD_DEMAND_MONITOR m;
  MArrayForEach(rs->workload_demand_chunks, &dc, &i)
    for( j = 0;  j < KheMatchingDemandChunkNodeCount(dc);  j++ )
    {
      m = (KHE_WORKLOAD_DEMAND_MONITOR) KheMatchingDemandChunkNode(dc, j);
      KheWorkloadDemandMonitorSetWeight(m, new_weight);
    }
}


/*****************************************************************************/
/*                                                                           */
/*  Submodule "assigned tasks"                                               */
/*                                                                           */
/*****************************************************************************/

/*****************************************************************************/
/*                                                                           */
/*  int KheResourceInSolnAssignedTaskCount(KHE_RESOURCE_IN_SOLN rs)          */
/*                                                                           */
/*  Return the number of assigned tasks in rs.                               */
/*                                                                           */
/*****************************************************************************/

int KheResourceInSolnAssignedTaskCount(KHE_RESOURCE_IN_SOLN rs)
{
  return MArraySize(rs->tasks);
}


/*****************************************************************************/
/*                                                                           */
/*  KHE_TASK KheResourceInSolnAssignedTask(KHE_RESOURCE_IN_SOLN rs, int i)   */
/*                                                                           */
/*  Return the i'th assigned task of rs.                                     */
/*                                                                           */
/*****************************************************************************/

KHE_TASK KheResourceInSolnAssignedTask(KHE_RESOURCE_IN_SOLN rs, int i)
{
  return MArrayGet(rs->tasks, i);
}


/*****************************************************************************/
/*                                                                           */
/*  Submodule "user monitors, cost, and timetables"                          */
/*                                                                           */
/*****************************************************************************/

/*****************************************************************************/
/*                                                                           */
/*  void KheResourceInSolnAddMonitor(KHE_RESOURCE_IN_SOLN rs, KHE_MONITOR m) */
/*                                                                           */
/*  Add m to the user-accessible list of monitors of rs.                     */
/*                                                                           */
/*****************************************************************************/

void KheResourceInSolnAddMonitor(KHE_RESOURCE_IN_SOLN rs, KHE_MONITOR m)
{
  MArrayAddLast(rs->all_monitors, m);
}


/*****************************************************************************/
/*                                                                           */
/*void KheResourceInSolnDeleteMonitor(KHE_RESOURCE_IN_SOLN rs, KHE_MONITOR m)*/
/*                                                                           */
/*  Delete m from rs's all_monitors list.                                    */
/*                                                                           */
/*****************************************************************************/

void KheResourceInSolnDeleteMonitor(KHE_RESOURCE_IN_SOLN rs, KHE_MONITOR m)
{
  int pos;
  if( !MArrayContains(rs->all_monitors, m, &pos) )
    MAssert(false, "KheResourceInSolnDeleteMonitor internal error");
  MArrayRemove(rs->all_monitors, pos);
}


/*****************************************************************************/
/*                                                                           */
/*  int KheResourceInSolnMonitorCount(KHE_RESOURCE_IN_SOLN rs)               */
/*                                                                           */
/*  Return the number of user-accessible monitors in rs.                     */
/*                                                                           */
/*****************************************************************************/

int KheResourceInSolnMonitorCount(KHE_RESOURCE_IN_SOLN rs)
{
  return MArraySize(rs->all_monitors);
}


/*****************************************************************************/
/*                                                                           */
/*  KHE_MONITOR KheResourceInSolnMonitor(KHE_RESOURCE_IN_SOLN rs, int i)     */
/*                                                                           */
/*  Return the i'th user-accessible monitor of rs.                           */
/*                                                                           */
/*****************************************************************************/

KHE_MONITOR KheResourceInSolnMonitor(KHE_RESOURCE_IN_SOLN rs, int i)
{
  return MArrayGet(rs->all_monitors, i);
}


/*****************************************************************************/
/*                                                                           */
/*  KHE_COST KheResourceInSolnCost(KHE_RESOURCE_IN_SOLN rs)                  */
/*                                                                           */
/*  Return the total cost of all monitors.                                   */
/*                                                                           */
/*****************************************************************************/

KHE_COST KheResourceInSolnCost(KHE_RESOURCE_IN_SOLN rs)
{
  KHE_MONITOR m;  int i;  KHE_COST res;
  res = 0;
  MArrayForEach(rs->all_monitors, &m, &i)
    res += KheMonitorCost(m);
  return res;
}


/*****************************************************************************/
/*                                                                           */
/*  KHE_COST KheResourceInSolnMonitorCost(KHE_RESOURCE_IN_SOLN rs,           */
/*    KHE_MONITOR_TAG tag)                                                   */
/*                                                                           */
/*  Return the total cost of monitors with this tag.                         */
/*                                                                           */
/*****************************************************************************/

KHE_COST KheResourceInSolnMonitorCost(KHE_RESOURCE_IN_SOLN rs, KHE_MONITOR_TAG tag)
{
  KHE_MONITOR m;  int i;  KHE_COST res;
  res = 0;
  MArrayForEach(rs->all_monitors, &m, &i)
    if( KheMonitorTag(m) == tag )
      res += KheMonitorCost(m);
  return res;
}


/*****************************************************************************/
/*                                                                           */
/*  KHE_TIMETABLE_MONITOR KheResourceInSolnTimetableMonitor(                 */
/*    KHE_RESOURCE_IN_SOLN rs)                                               */
/*                                                                           */
/*  Return the timetable of rs.                                              */
/*                                                                           */
/*****************************************************************************/

KHE_TIMETABLE_MONITOR KheResourceInSolnTimetableMonitor(KHE_RESOURCE_IN_SOLN rs)
{
  return rs->timetable_monitor;
}


/*****************************************************************************/
/*                                                                           */
/*  Submodule "debug"                                                        */
/*                                                                           */
/*****************************************************************************/

/*****************************************************************************/
/*                                                                           */
/*  void KheResourceInSolnDebug(KHE_RESOURCE_IN_SOLN rs, int verbosity,      */
/*    int indent, FILE *fp)                                                  */
/*                                                                           */
/*  Debug print of resource monitor rs onto fp with the given verbosity      */
/*  and indent.                                                              */
/*                                                                           */
/*****************************************************************************/

void KheResourceInSolnDebug(KHE_RESOURCE_IN_SOLN rs, int verbosity,
  int indent, FILE *fp)
{
  KHE_MONITOR m;  int i;
  if( verbosity >= 1 )
  {
    if( indent >= 0 )
    {
      fprintf(fp, "%*s[ Resource %s\n", indent, "", KheResourceInSolnId(rs));
      MArrayForEach(rs->attached_monitors, &m, &i)
	KheMonitorDebug(m, verbosity, indent + 2, fp);
      fprintf(fp, "%*s]\n", indent, "");
    }
    else
      fprintf(fp, "%s", KheResourceInSolnId(rs));
  }
}
