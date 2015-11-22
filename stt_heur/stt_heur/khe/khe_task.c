
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
/*  FILE:         khe_task.c                                                 */
/*  DESCRIPTION:  A task                                                     */
/*                                                                           */
/*****************************************************************************/
#include "khe_interns.h"

#define DEBUG1 0

/*****************************************************************************/
/*                                                                           */
/*  KHE_TASK - a task                                                        */
/*                                                                           */
/*****************************************************************************/

struct khe_task_rec {
  void				*back;			/* back pointer      */
  KHE_SOLN			soln;			/* enclosing soln    */
  int				index_in_soln;		/* index in soln     */
  KHE_MEET			meet;			/* optional meet     */
  int				index_in_meet;		/* index in meet     */
  KHE_RESOURCE_GROUP		domain;			/* resource domain   */
  KHE_TASKING			tasking;		/* optional tasking  */
  int				index_in_tasking;	/* index in tasking  */
  KHE_TASK			target_task;		/* task assigned to  */
  ARRAY_KHE_TASK		assigned_tasks;		/* assigned to this  */
  int				visit_num;		/* visit number      */
  ARRAY_KHE_ORDINARY_DEMAND_MONITOR all_monitors;	/* all demand mon's  */
  ARRAY_KHE_ORDINARY_DEMAND_MONITOR attached_monitors;	/* attached monitors */
  KHE_RESOURCE_IN_SOLN		assigned_rs;		/* assigned resource */
  KHE_EVENT_RESOURCE_IN_SOLN	event_resource_in_soln;	/* optional er in s  */
  KHE_TASK			copy;			/* used when copying */
};


/*****************************************************************************/
/*                                                                           */
/*  Submodule "construction and query"                                       */
/*                                                                           */
/*****************************************************************************/

/*****************************************************************************/
/*                                                                           */
/*  KHE_TASK KheTaskMake(KHE_SOLN soln, KHE_RESOURCE_TYPE rt, KHE_MEET meet, */
/*    KHE_EVENT_RESOURCE er)                                                 */
/*                                                                           */
/*  Make and return a new task with these attributes.  The first two are     */
/*  compulsory.  If meet is absent, then er must be absent too.              */
/*                                                                           */
/*****************************************************************************/

KHE_TASK KheTaskMake(KHE_SOLN soln, KHE_RESOURCE_TYPE rt, KHE_MEET meet,
  KHE_EVENT_RESOURCE er)
{
  KHE_TASK res;  int i;  KHE_EVENT_IN_SOLN es;
  KHE_MATCHING_DEMAND_CHUNK dc;  KHE_ORDINARY_DEMAND_MONITOR m; 

  /* ensure parameters are legal and consistent */
  if( meet == NULL )
    MAssert(er == NULL, "KheTaskMake: meet == NULL && er != NULL");
  if( er != NULL )
    MAssert(KheMeetEvent(meet) == KheEventResourceEvent(er),
      "KheTaskMake: KheMeetEvent(meet) != KheEventResourceEvent(er)");

  /* get a new task object and ensure that its arrays are initialized */
  res = KheSolnGetTaskFromFreeList(soln);
  if( res != NULL )
  {
    MArrayClear(res->assigned_tasks);
    MArrayClear(res->all_monitors);
    MArrayClear(res->attached_monitors);
  }
  else
  {
    MMake(res);
    MArrayInit(res->assigned_tasks);
    MArrayInit(res->all_monitors);
    MArrayInit(res->attached_monitors);
  }

  /* add res to soln */
  res->back = NULL;
  res->soln = soln;
  KheSolnAddTask(soln, res, &res->index_in_soln);

  /* add res to meet, if any */
  res->meet = meet;
  if( meet != NULL )
    KheMeetAddTask(meet, res, &res->index_in_meet);
  else
    res->index_in_meet = -1;

  /* initialize res's domain */
  if( er != NULL && KheEventResourcePreassignedResource(er) != NULL )
    res->domain = KheResourceSingletonResourceGroup(
      KheEventResourcePreassignedResource(er));
  else
    res->domain = KheResourceTypeFullResourceGroup(rt);

  /* initially res lies in no tasking, and is unassigned */
  res->tasking = NULL;
  res->index_in_tasking = -1;
  res->target_task = NULL;
  res->visit_num = 0;

  /* add demand monitors to res, but only if it lies in a meet */
  if( meet != NULL )
    for( i = 0;  i < KheMeetDuration(meet);  i++ )
    {
      dc = KheMeetDemandChunk(meet, i);
      m = KheOrdinaryDemandMonitorMake(soln, dc, res, i);
      KheGroupMonitorAddChildMonitor((KHE_GROUP_MONITOR) soln, (KHE_MONITOR) m);
    }

  /* initially res is not assigned a resource */
  res->assigned_rs = NULL;

  /* find and inform the event resource in soln, if any */
  if( er != NULL )
  {
    es = KheSolnEventInSoln(soln, KheEventIndex(KheEventResourceEvent(er)));
    res->event_resource_in_soln =
      KheEventInSolnEventResourceInSoln(es, KheEventResourceIndexInEvent(er));
    KheEventResourceInSolnAddTask(res->event_resource_in_soln, res);
  }
  else
    res->event_resource_in_soln = NULL;

  /* initialize copy, and inform soln that this operation has occurred */
  res->copy = NULL;
  KheSolnOpTaskMake(soln, res);

  return res;
}


/*****************************************************************************/
/*                                                                           */
/*  KHE_TASK KheCycleTaskMake(KHE_SOLN soln, KHE_RESOURCE r)                 */
/*                                                                           */
/*  Make and return a cycle task for resource r.                             */
/*                                                                           */
/*****************************************************************************/

KHE_TASK KheCycleTaskMake(KHE_SOLN soln, KHE_RESOURCE r)
{
  KHE_TASK res;
  res = KheTaskMake(soln, KheResourceResourceType(r), NULL, NULL);
  res->assigned_rs = KheSolnResourceInSoln(soln, KheResourceIndexInInstance(r));
  res->domain = KheResourceSingletonResourceGroup(r);
  return res;
}


/*****************************************************************************/
/*                                                                           */
/*  KHE_TASK KheTaskCopyPhase1(KHE_TASK task)                                */
/*                                                                           */
/*  Carry out Phase 1 of the operation of copying task.                      */
/*                                                                           */
/*****************************************************************************/

KHE_TASK KheTaskCopyPhase1(KHE_TASK task)
{
  KHE_TASK copy, child_task;  KHE_ORDINARY_DEMAND_MONITOR m;  int i;
  if( task->copy == NULL )
  {
    MMake(copy);
    task->copy = copy;
    copy->back = task->back;
    copy->soln = KheSolnCopyPhase1(task->soln);
    copy->index_in_soln = task->index_in_soln;
    copy->meet = task->meet == NULL ? NULL : KheMeetCopyPhase1(task->meet);
    copy->index_in_meet = task->index_in_meet;
    copy->domain = task->domain;
    copy->tasking = task->tasking == NULL ? NULL :
      KheTaskingCopyPhase1(task->tasking);
    copy->index_in_tasking = task->index_in_tasking;
    copy->target_task = task->target_task == NULL ? NULL :
      KheTaskCopyPhase1(task->target_task);
    MArrayInit(copy->assigned_tasks);
    MArrayForEach(task->assigned_tasks, &child_task, &i)
      MArrayAddLast(copy->assigned_tasks, KheTaskCopyPhase1(child_task));
    copy->visit_num = task->visit_num;
    MArrayInit(copy->all_monitors);
    MArrayForEach(task->all_monitors, &m, &i)
      MArrayAddLast(copy->all_monitors, KheOrdinaryDemandMonitorCopyPhase1(m));
    MArrayInit(copy->attached_monitors);
    MArrayForEach(task->attached_monitors, &m, &i)
      MArrayAddLast(copy->attached_monitors,
	KheOrdinaryDemandMonitorCopyPhase1(m));
    copy->assigned_rs = task->assigned_rs == NULL ? NULL :
      KheResourceInSolnCopyPhase1(task->assigned_rs);
    copy->event_resource_in_soln = task->event_resource_in_soln == NULL ? NULL :
      KheEventResourceInSolnCopyPhase1(task->event_resource_in_soln);
    copy->copy = NULL;
  }
  return task->copy;
}


/*****************************************************************************/
/*                                                                           */
/*  void KheTaskCopyPhase2(KHE_TASK task)                                    */
/*                                                                           */
/*  Carry out Phase 2 of copying task.                                       */
/*                                                                           */
/*****************************************************************************/

void KheTaskCopyPhase2(KHE_TASK task)
{
  KHE_ORDINARY_DEMAND_MONITOR m;  int i;
  if( task->copy != NULL )
  {
    task->copy = NULL;
    MArrayForEach(task->all_monitors, &m, &i)
      KheOrdinaryDemandMonitorCopyPhase2(m);
  }
}


/*****************************************************************************/
/*                                                                           */
/*  void KheTaskFree(KHE_TASK task)                                          */
/*                                                                           */
/*  Free the memory occupied by task.                                        */
/*                                                                           */
/*****************************************************************************/

void KheTaskFree(KHE_TASK task)
{
  MArrayFree(task->assigned_tasks);
  MArrayFree(task->all_monitors);
  MArrayFree(task->attached_monitors);
  MFree(task);
}


/*****************************************************************************/
/*                                                                           */
/*  void KheTaskDelete(KHE_TASK task)                                        */
/*                                                                           */
/*  Delete task.  Its memory goes on soln's task free list.                  */
/*                                                                           */
/*****************************************************************************/

void KheTaskDelete(KHE_TASK task)
{
  /* remove task from its tasking, if any */
  if( task->tasking != NULL )
    KheTaskingDeleteTask(task->tasking, task);

  /* unassign task, if assigned */
  if( task->target_task != NULL )
    KheTaskUnAssign(task);

  /* unassign its child tasks */
  while( MArraySize(task->assigned_tasks) > 0 )
    KheTaskUnAssign(MArrayLast(task->assigned_tasks));

  /* inform task's event resource in soln */
  if( task->event_resource_in_soln != NULL )
    KheEventResourceInSolnDeleteTask(task->event_resource_in_soln, task);

  /* inform task's meet */
  if( task->meet != NULL )
    KheMeetDeleteTask(task->meet, task->index_in_meet);

  /* remove task's demand monitors */
  while( MArraySize(task->all_monitors) > 0 )
    KheOrdinaryDemandMonitorDelete(MArrayLast(task->all_monitors));
  MAssert(MArraySize(task->attached_monitors) == 0,
    "KheTaskDelete internal error");

  /* delete from soln, inform soln that operation has occured, and free */
  KheSolnDeleteTask(task->soln, task);
  KheSolnOpTaskDelete(task->soln);
  KheSolnAddTaskToFreeList(task->soln, task);
}


/*****************************************************************************/
/*                                                                           */
/*  KHE_RESOURCE_TYPE KheTaskResourceType(KHE_TASK task)                     */
/*                                                                           */
/*  Return the resource type attribute of task.                              */
/*                                                                           */
/*****************************************************************************/

KHE_RESOURCE_TYPE KheTaskResourceType(KHE_TASK task)
{
  return KheResourceGroupResourceType(task->domain);
}


/*****************************************************************************/
/*                                                                           */
/*  KHE_EVENT_RESOURCE KheTaskEventResource(KHE_TASK task)                   */
/*                                                                           */
/*  Return the optional event resource of task.                              */
/*                                                                           */
/*****************************************************************************/

KHE_EVENT_RESOURCE KheTaskEventResource(KHE_TASK task)
{
  return task->event_resource_in_soln == NULL ? NULL :
    KheEventResourceInSolnEventResource(task->event_resource_in_soln);
}


/*****************************************************************************/
/*                                                                           */
/*  Submodule "back pointers"                                                */
/*                                                                           */
/*****************************************************************************/

/*****************************************************************************/
/*                                                                           */
/*  void KheTaskSetBack(KHE_TASK task, void *back)                           */
/*                                                                           */
/*  Set the back pointer of task.                                            */
/*                                                                           */
/*****************************************************************************/

void KheTaskSetBack(KHE_TASK task, void *back)
{
  task->back = back;
}


/*****************************************************************************/
/*                                                                           */
/*  void *KheTaskBack(KHE_TASK task)                                         */
/*                                                                           */
/*  Return the back pointer of task.                                         */
/*                                                                           */
/*****************************************************************************/

void *KheTaskBack(KHE_TASK task)
{
  return task->back;
}


/*****************************************************************************/
/*                                                                           */
/*  Submodule "visit numbers"                                                */
/*                                                                           */
/*****************************************************************************/

/*****************************************************************************/
/*                                                                           */
/*  void KheTaskSetVisitNum(KHE_TASK task, int num)                          */
/*                                                                           */
/*  Set the visit number of task.                                            */
/*                                                                           */
/*****************************************************************************/

void KheTaskSetVisitNum(KHE_TASK task, int num)
{
  task->visit_num = num;
}


/*****************************************************************************/
/*                                                                           */
/*  int KheTaskVisitNum(KHE_TASK task)                                       */
/*                                                                           */
/*  Return the visit number of task.                                         */
/*                                                                           */
/*****************************************************************************/

int KheTaskVisitNum(KHE_TASK task)
{
  return task->visit_num;
}


/*****************************************************************************/
/*                                                                           */
/*  bool KheTaskVisited(KHE_TASK task, int slack)                            */
/*                                                                           */
/*  Return true if task has been visited recently.                           */
/*                                                                           */
/*****************************************************************************/

bool KheTaskVisited(KHE_TASK task, int slack)
{
  return KheSolnVisitNum(KheTaskSoln(task)) - task->visit_num <= slack;
}


/*****************************************************************************/
/*                                                                           */
/*  void KheTaskVisit(KHE_TASK task)                                         */
/*                                                                           */
/*  Visit task.                                                              */
/*                                                                           */
/*****************************************************************************/

void KheTaskVisit(KHE_TASK task)
{
  task->visit_num = KheSolnVisitNum(KheTaskSoln(task));
}


/*****************************************************************************/
/*                                                                           */
/*  void KheTaskUnVisit(KHE_TASK task)                                       */
/*                                                                           */
/*  Unvisit task.                                                            */
/*                                                                           */
/*****************************************************************************/

void KheTaskUnVisit(KHE_TASK task)
{
  task->visit_num = KheSolnVisitNum(KheTaskSoln(task)) - 1;
}


/*****************************************************************************/
/*                                                                           */
/*  Submodule "relation with enclosing soln"                                 */
/*                                                                           */
/*****************************************************************************/

/*****************************************************************************/
/*                                                                           */
/*  KHE_SOLN KheTaskSoln(KHE_TASK task)                                      */
/*                                                                           */
/*  Return the soln attribute of task.                                       */
/*                                                                           */
/*****************************************************************************/

KHE_SOLN KheTaskSoln(KHE_TASK task)
{
  return task->soln;
}


/*****************************************************************************/
/*                                                                           */
/*  int KheTaskIndexInSoln(KHE_TASK task)                                    */
/*                                                                           */
/*  Return the index number of task in its soln.                             */
/*                                                                           */
/*****************************************************************************/

int KheTaskIndexInSoln(KHE_TASK task)
{
  return task->index_in_soln;
}


/*****************************************************************************/
/*                                                                           */
/*  void KheTaskSetIndexInSoln(KHE_TASK task, int num)                       */
/*                                                                           */
/*  Set the index number of task in its soln to num.                         */
/*                                                                           */
/*****************************************************************************/

void KheTaskSetIndexInSoln(KHE_TASK task, int num)
{
  task->index_in_soln = num;
}


/*****************************************************************************/
/*                                                                           */
/*  Submodule "relation with enclosing meet"                                 */
/*                                                                           */
/*****************************************************************************/

/*****************************************************************************/
/*                                                                           */
/*  KHE_MEET KheTaskMeet(KHE_TASK task)                                      */
/*                                                                           */
/*  Return the soln attribute of task.                                       */
/*                                                                           */
/*****************************************************************************/

KHE_MEET KheTaskMeet(KHE_TASK task)
{
  return task->meet;
}


/*****************************************************************************/
/*                                                                           */
/*  int KheTaskDuration(KHE_TASK task)                                       */
/*                                                                           */
/*  Return the duration of the meet containing task, or 0 if none.           */
/*                                                                           */
/*****************************************************************************/

int KheTaskDuration(KHE_TASK task)
{
  return task->meet == NULL ? 0 : KheMeetDuration(task->meet);
}


/*****************************************************************************/
/*                                                                           */
/*  float KheTaskWorkload(KHE_TASK task)                                     */
/*                                                                           */
/*  Return the workload of task, according to the formula                    */
/*                                                                           */
/*    Workload(task) = Duration(meet) * Workload(er) / Duration(e)           */
/*                                                                           */
/*  where meet is its meet, er is its event resource, and e is the           */
/*  enclosing event.                                                         */
/*                                                                           */
/*****************************************************************************/

float KheTaskWorkload(KHE_TASK task)
{
  KHE_EVENT_RESOURCE er;
  if( task->event_resource_in_soln == NULL )
    return 0.0;
  else
  {
    MAssert(task->meet != NULL, "KheTaskWorkload internal error");
    er = KheEventResourceInSolnEventResource(task->event_resource_in_soln);
    return (float) KheMeetDuration(task->meet) * KheEventResourceWorkload(er) /
      (float) KheEventDuration(KheEventResourceEvent(er));
  }
}


/*****************************************************************************/
/*                                                                           */
/*  int KheTaskIndexInMeet(KHE_TASK task)                                    */
/*                                                                           */
/*  Return the index number of task in its meet.                             */
/*                                                                           */
/*****************************************************************************/

int KheTaskIndexInMeet(KHE_TASK task)
{
  return task->index_in_meet;
}


/*****************************************************************************/
/*                                                                           */
/*  void KheTaskSetIndexInMeet(KHE_TASK task, int num)                       */
/*                                                                           */
/*  Set the index number of task in its meet to num.                         */
/*                                                                           */
/*****************************************************************************/

void KheTaskSetIndexInMeet(KHE_TASK task, int num)
{
  task->index_in_meet = num;
}


/*****************************************************************************/
/*                                                                           */
/*  Submodule "relation with enclosing tasking"                              */
/*                                                                           */
/*****************************************************************************/

/*****************************************************************************/
/*                                                                           */
/*  KHE_TASKING KheTaskTasking(KHE_TASK task)                                */
/*                                                                           */
/*  Return the tasking containing task, or NULL if none.                     */
/*                                                                           */
/*****************************************************************************/

KHE_TASKING KheTaskTasking(KHE_TASK task)
{
  return task->tasking;
}


/*****************************************************************************/
/*                                                                           */
/*  int KheTaskIndexInTasking(KHE_TASK task)                                 */
/*                                                                           */
/*  Return task's index in the enclosing soln tasking, assumed to exist.     */
/*                                                                           */
/*****************************************************************************/

int KheTaskIndexInTasking(KHE_TASK task)
{
  return task->index_in_tasking;
}


/*****************************************************************************/
/*                                                                           */
/*  void KheTaskSetTaskingAndIndex(KHE_TASK task, KHE_TASKING tasking,       */
/*    int index_in_tasking)                                                  */
/*                                                                           */
/*  Set the tasking and index_in_tasking attributes of task.                 */
/*                                                                           */
/*****************************************************************************/

void KheTaskSetTaskingAndIndex(KHE_TASK task, KHE_TASKING tasking,
  int index_in_tasking)
{
  task->tasking = tasking;
  task->index_in_tasking = index_in_tasking;
}


/*****************************************************************************/
/*                                                                           */
/*  Submodule "demand monitor domains"                                       */
/*                                                                           */
/*  Implementation note.  The domains of the ordinary demand monitors of     */
/*  a task depend on the matching type, the domain of the task, the chain    */
/*  of assignments out of the task (defining its root task), and the         */
/*  domain of the root task.                                                 */
/*                                                                           */
/*  The precise dependence is given by KheTaskMatchingDomain immediately     */
/*  below.  The other functions in this submodule implement the changes      */
/*  required when one of the things that the domains depend on changes.      */
/*                                                                           */
/*****************************************************************************/

/*****************************************************************************/
/*                                                                           */
/*  KHE_RESOURCE_GROUP KheTaskMatchingDomain(KHE_TASK task)                  */
/*                                                                           */
/*  Return a suitable domain for the matching demand nodes of task.          */
/*                                                                           */
/*****************************************************************************/

KHE_RESOURCE_GROUP KheTaskMatchingDomain(KHE_TASK task)
{
  switch( KheSolnMatchingType(KheTaskSoln(task)) )
  {
    case KHE_MATCHING_TYPE_EVAL_INITIAL:
    case KHE_MATCHING_TYPE_EVAL_TIMES:

      return KheTaskDomain(task);

    case KHE_MATCHING_TYPE_SOLVE:
    case KHE_MATCHING_TYPE_EVAL_RESOURCES:

      return KheTaskDomain(KheTaskRoot(task));

    default:

      MAssert(false, "KheTaskMatchingDomain internal error");
      return NULL;  /* keep compiler happy */
  }
}


/*****************************************************************************/
/*                                                                           */
/*  void KheTaskMatchingReset(KHE_TASK task)                                 */
/*                                                                           */
/*  Reset the matching within task.                                          */
/*                                                                           */
/*  This function makes no attempt to move gracefully from one state to      */
/*  another; rather, it starts again from scratch.  This is useful for       */
/*  initializing, and also when the matching type changes, since in those    */
/*  cases a reset is the sensible way forward.                               */
/*                                                                           */
/*  This function is called separately for every task, so there is no need   */
/*  to worry about making recursive calls.                                   */
/*                                                                           */
/*****************************************************************************/

void KheTaskMatchingReset(KHE_TASK task)
{
  KHE_RESOURCE_GROUP rg;  int i;  KHE_ORDINARY_DEMAND_MONITOR m;
  if( MArraySize(task->attached_monitors) > 0 )
  {
    rg = KheTaskMatchingDomain(task);
    MArrayForEach(task->attached_monitors, &m, &i)
      KheOrdinaryDemandMonitorSetDomain(m, rg,
	KHE_MATCHING_DOMAIN_CHANGE_TO_OTHER);
  }
}


/*****************************************************************************/
/*                                                                           */
/*  void KheTaskMatchingDoSetDomain(KHE_TASK task, KHE_RESOURCE_GROUP rg,    */
/*    KHE_MATCHING_DOMAIN_CHANGE_TYPE change_type)                           */
/*                                                                           */
/*  Recursively set the domains of the demand monitors of task and its       */
/*  descendants to rg, with change_type saying what kind of change this is.  */
/*                                                                           */
/*****************************************************************************/

static void KheTaskMatchingDoSetDomain(KHE_TASK task, KHE_RESOURCE_GROUP rg,
  KHE_MATCHING_DOMAIN_CHANGE_TYPE change_type)
{
  KHE_ORDINARY_DEMAND_MONITOR m;  KHE_TASK child_task;  int i;

  /* change the domains within task itself */
  MArrayForEach(task->attached_monitors, &m, &i)
    KheOrdinaryDemandMonitorSetDomain(m, rg, change_type);

  /* change the domains within the followers */
  MArrayForEach(task->assigned_tasks, &child_task, &i)
    KheTaskMatchingDoSetDomain(child_task, rg, change_type);
}


/*****************************************************************************/
/*                                                                           */
/*  KHE_MATCHING_DOMAIN_CHANGE_TYPE KheTaskChangeType(                       */
/*    KHE_RESOURCE_GROUP old_rg, KHE_RESOURCE_GROUP new_rg)                  */
/*                                                                           */
/*  Return the type of change when moving from old_rg to new_rg.             */
/*                                                                           */
/*****************************************************************************/

static KHE_MATCHING_DOMAIN_CHANGE_TYPE KheTaskChangeType(
  KHE_RESOURCE_GROUP old_rg, KHE_RESOURCE_GROUP new_rg)
{
    if( KheResourceGroupSubset(new_rg, old_rg) )
      return KHE_MATCHING_DOMAIN_CHANGE_TO_SUBSET;
    else if( KheResourceGroupSubset(old_rg, new_rg) )
      return KHE_MATCHING_DOMAIN_CHANGE_TO_SUPERSET;
    else
      return KHE_MATCHING_DOMAIN_CHANGE_TO_OTHER;
}


/*****************************************************************************/
/*                                                                           */
/*  void KheTaskMatchingSetDomain(KHE_TASK task, KHE_RESOURCE_GROUP rg)      */
/*                                                                           */
/*  Given that the domain of task is about to change to rg, update the       */
/*  matching in task appropriately, including updating the descendants       */
/*  of task if appropriate.                                                  */
/*                                                                           */
/*****************************************************************************/

static void KheTaskMatchingSetDomain(KHE_TASK task, KHE_RESOURCE_GROUP rg)
{
  int i;  KHE_ORDINARY_DEMAND_MONITOR m;
  KHE_MATCHING_DOMAIN_CHANGE_TYPE change_type;
  switch( KheSolnMatchingType(KheTaskSoln(task)) )
  {
    case KHE_MATCHING_TYPE_EVAL_INITIAL:
    case KHE_MATCHING_TYPE_EVAL_TIMES:

      /* set the domains to rg within task only */
      if( MArraySize(task->attached_monitors) > 0 )
      {
	change_type = KheTaskChangeType(task->domain, rg);
	MArrayForEach(task->attached_monitors, &m, &i)
	  KheOrdinaryDemandMonitorSetDomain(m, rg, change_type);
      }
      break;

    case KHE_MATCHING_TYPE_SOLVE:
    case KHE_MATCHING_TYPE_EVAL_RESOURCES:

      /* set the domains to rg, but only if unassigned root task; */
      /* and in that case, set them in all the followers too      */
      if( task->target_task == NULL )
      {
	change_type = KheTaskChangeType(task->domain, rg);
	KheTaskMatchingDoSetDomain(task, rg, change_type);
      }
      break;

    default:

      MAssert(false, "KheTaskSetDomain internal error 4");
      break;
  }
}


/*****************************************************************************/
/*                                                                           */
/*  void KheTaskMatchingAssign(KHE_TASK task, KHE_TASK target_task)          */
/*                                                                           */
/*  Given that task is about to be assigned to target_task, update the       */
/*  matching appropriately, including the descendants if appropriate.        */
/*                                                                           */
/*****************************************************************************/

static void KheTaskMatchingAssign(KHE_TASK task, KHE_TASK target_task)
{
  KHE_RESOURCE_GROUP new_rg;
  switch( KheSolnMatchingType(KheTaskSoln(task)) )
  {
    case KHE_MATCHING_TYPE_EVAL_INITIAL:
    case KHE_MATCHING_TYPE_EVAL_TIMES:

      /* assignments don't affect matchings of these types */
      break;

    case KHE_MATCHING_TYPE_SOLVE:
    case KHE_MATCHING_TYPE_EVAL_RESOURCES:

      /* domain is tightening from task->domain to new root's domain */
      new_rg = KheTaskDomain(KheTaskRoot(target_task));
      if( !KheResourceGroupEqual(task->domain, new_rg) )
	KheTaskMatchingDoSetDomain(task, new_rg,
	  KHE_MATCHING_DOMAIN_CHANGE_TO_SUBSET);
      break;

    default:

      MAssert(false, "KheTaskMatchingAssign internal error");
      break;
  }
}


/*****************************************************************************/
/*                                                                           */
/*  void KheTaskMatchingUnAssign(KHE_TASK task)                              */
/*                                                                           */
/*  Given that task is about to be unassigned, update the matching           */
/*  appropriately, including the descendants if appropriate.                 */
/*                                                                           */
/*****************************************************************************/

static void KheTaskMatchingUnAssign(KHE_TASK task)
{
  KHE_RESOURCE_GROUP old_rg;
  switch( KheSolnMatchingType(KheTaskSoln(task)) )
  {
    case KHE_MATCHING_TYPE_EVAL_INITIAL:
    case KHE_MATCHING_TYPE_EVAL_TIMES:

      /* assignments don't affect matchings of these types */
      break;

    case KHE_MATCHING_TYPE_SOLVE:
    case KHE_MATCHING_TYPE_EVAL_RESOURCES:

      /* domain is loosening from old_rg to task->domain */
      old_rg = KheTaskDomain(KheTaskRoot(task));
      if( !KheResourceGroupEqual(task->domain, old_rg) )
	KheTaskMatchingDoSetDomain(task, task->domain,
	  KHE_MATCHING_DOMAIN_CHANGE_TO_SUPERSET);
      break;

    default:

      MAssert(false, "KheTaskMatchingAssign internal error");
      break;
  }
}


/*****************************************************************************/
/*                                                                           */
/*  Submodule "splitting and merging etc."                                   */
/*                                                                           */
/*****************************************************************************/

/*****************************************************************************/
/*                                                                           */
/*  void KheTaskAssignTime(KHE_TASK task, int assigned_time_index)           */
/*                                                                           */
/*  Inform task that the enclosing meet has been assigned this time.         */
/*                                                                           */
/*****************************************************************************/

void KheTaskAssignTime(KHE_TASK task, int assigned_time_index)
{
  if( task->assigned_rs != NULL )
    KheResourceInSolnAssignTime(task->assigned_rs, task, assigned_time_index);
  KheEvennessHandlerAddTask(KheSolnEvennessHandler(task->soln), task,
    assigned_time_index);
}


/*****************************************************************************/
/*                                                                           */
/*  void KheTaskUnAssignTime(KHE_TASK task, int assigned_time_index)         */
/*                                                                           */
/*  Inform task that the enclosing meet has been unassigned this time.       */
/*                                                                           */
/*****************************************************************************/

void KheTaskUnAssignTime(KHE_TASK task, int assigned_time_index)
{
  if( task->assigned_rs != NULL )
    KheResourceInSolnUnAssignTime(task->assigned_rs, task, assigned_time_index);
  KheEvennessHandlerDeleteTask(KheSolnEvennessHandler(task->soln), task,
    assigned_time_index);
}


/*****************************************************************************/
/*                                                                           */
/*  KHE_TASK KheTaskSplit(KHE_TASK task, int duration1, KHE_MEET meet2,      */
/*    int index_in_meet2)                                                    */
/*                                                                           */
/*  Split task at duration1 and return the new task.  It lies in meet2 at    */
/*  position index_in_meet2.                                                 */
/*                                                                           */
/*****************************************************************************/

KHE_TASK KheTaskSplit(KHE_TASK task, int duration1, KHE_MEET meet2,
  int index_in_meet2)
{
  int i, duration2;  KHE_TASK res;  KHE_ORDINARY_DEMAND_MONITOR m;

  /* initialize the new task */
  res = KheSolnGetTaskFromFreeList(task->soln);
  if( res != NULL )
  {
    MArrayClear(res->assigned_tasks);
    MArrayClear(res->all_monitors);
    MArrayClear(res->attached_monitors);
  }
  else
  {
    MMake(res);
    MArrayInit(res->assigned_tasks);
    MArrayInit(res->all_monitors);
    MArrayInit(res->attached_monitors);
  }
  res->back = task->back;

  /* add task to soln */
  res->soln = task->soln;
  KheSolnAddTask(task->soln, res, &res->index_in_soln);

  /* record its meet and index in meet (will be added there on return) */
  MAssert(meet2 != NULL, "KheTaskSplit internal error");
  res->meet = meet2;
  res->index_in_meet = index_in_meet2;

  /* set domain, and tasking if any */
  res->domain = task->domain;  /* must precede KheTaskingAddTask */
  res->tasking = NULL;
  if( task->tasking != NULL )
    KheTaskingAddTask(task->tasking, res);

  /* same target task as task */
  res->target_task = task->target_task;
  if( res->target_task != NULL )
    MArrayAddLast(task->target_task->assigned_tasks, res);

  /* assigned tasks stay with task */
  res->visit_num = task->visit_num;
  res->copy = NULL;

  /* split the monitors between task and res */
  duration2 = MArraySize(task->all_monitors) - duration1;
  MAssert(duration2 >= 1, "KheTaskSplit internal error");
  for( i = duration1;  i < MArraySize(task->all_monitors);  i++ )
  {
    m = MArrayGet(task->all_monitors, i);
    KheOrdinaryDemandMonitorSetTaskAndOffset(m, res, i - duration1);
    MArrayAddLast(res->all_monitors, m);
  }
  MArrayDropFromEnd(task->all_monitors, duration2);
  MArrayForEach(task->attached_monitors, &m, &i)
    if( KheOrdinaryDemandMonitorTask(m) == res )
    {
      MArrayRemove(task->attached_monitors, i);
      MArrayAddLast(res->attached_monitors, m);
      i--;
    }

  /* initialize assigned_rs and inform it of what is happening */
  res->assigned_rs = task->assigned_rs;
  if( task->assigned_rs != NULL )
    KheResourceInSolnSplitTask(task->assigned_rs, task, res);

  /* initialize event_resource_in_soln and inform it of what is happening */
  res->event_resource_in_soln = task->event_resource_in_soln;
  if( res->event_resource_in_soln != NULL )
    KheEventResourceInSolnSplitTask(res->event_resource_in_soln, task, res);
  return res;
}


/*****************************************************************************/
/*                                                                           */
/*  bool KheTaskMergeCheck(KHE_TASK task1, KHE_TASK task2)                   */
/*                                                                           */
/*  Check that it is safe to merge these two tasks.                          */
/*                                                                           */
/*****************************************************************************/

bool KheTaskMergeCheck(KHE_TASK task1, KHE_TASK task2)
{
  return task1 != task2 &&
    task1->back == task2->back &&
    task1->tasking == task2->tasking &&
    task1->target_task == task2->target_task &&
    task1->assigned_rs == task2->assigned_rs &&
    KheResourceGroupEqual(task1->domain, task2->domain);
}


/*****************************************************************************/
/*                                                                           */
/*  void KheTaskMerge(KHE_TASK task1, KHE_TASK task2)                        */
/*                                                                           */
/*  Merge task2 into task1, freeing task2.                                   */
/*                                                                           */
/*****************************************************************************/

void KheTaskMerge(KHE_TASK task1, KHE_TASK task2)
{
  int i, pos;  KHE_ORDINARY_DEMAND_MONITOR m;  KHE_TASK child_task;

  /* the enclosing meet will remove task2 itself */

  /* remove task2 from its tasking, if any */
  if( task2->tasking != NULL )
    KheTaskingDeleteTask(task2->tasking, task2);

  /* remove task2 from its the target task, if any */
  if( task1->target_task != NULL )
  {
    if( !MArrayContains(task1->target_task->assigned_tasks, task2, &pos) )
      MAssert(false, "KheTaskMerge internal error");
    MArrayRemove(task1->target_task->assigned_tasks, pos);
  }

  /* move task2's assigned tasks to task1 */
  while( MArraySize(task2->assigned_tasks) > 0 )
  {
    child_task = MArrayRemoveLast(task2->assigned_tasks);
    child_task->target_task = task1;
    MArrayAddLast(task1->assigned_tasks, child_task);
  }

  /* move the demand monitors */
  MArrayForEach(task2->all_monitors, &m, &i)
  {
    KheOrdinaryDemandMonitorSetTaskAndOffset(m, task1,
      MArraySize(task1->all_monitors));
    MArrayAddLast(task1->all_monitors, m);
  }
  MArrayClear(task2->all_monitors);
  MArrayAppend(task1->attached_monitors, task2->attached_monitors, i);
  MArrayClear(task2->attached_monitors);

  /* inform the assigned_rs, if any */
  if( task1->assigned_rs != NULL )
    KheResourceInSolnMergeTask(task1->assigned_rs, task1, task2);

  /* inform the event resource in soln, if any */
  if( task1->event_resource_in_soln != NULL )
    KheEventResourceInSolnMergeTask(task1->event_resource_in_soln,
      task1, task2);

  /* delete the task from soln and add it to the free list */
  KheSolnDeleteTask(task1->soln, task2);
  KheSolnAddTaskToFreeList(task1->soln, task2);
}


/*****************************************************************************/
/*                                                                           */
/*  Submodule "assignment"                                                   */
/*                                                                           */
/*****************************************************************************/

/*****************************************************************************/
/*                                                                           */
/*  bool KheTaskAssignResourceCheck(KHE_TASK task, KHE_RESOURCE r)           */
/*                                                                           */
/*  Return true if r can be assigned to task.                                */
/*                                                                           */
/*****************************************************************************/

bool KheTaskAssignResourceCheck(KHE_TASK task, KHE_RESOURCE r)
{
  KHE_TASK target_task;
  target_task = KheSolnTask(task->soln, KheResourceIndexInInstance(r));
  return KheTaskAssignCheck(task, target_task);
}


/*****************************************************************************/
/*                                                                           */
/*  bool KheTaskAssignResource(KHE_TASK task, KHE_RESOURCE r)                */
/*                                                                           */
/*  Assign r to task if possible.                                            */
/*                                                                           */
/*****************************************************************************/

bool KheTaskAssignResource(KHE_TASK task, KHE_RESOURCE r)
{
  KHE_TASK target_task;
  target_task = KheSolnTask(task->soln, KheResourceIndexInInstance(r));
  return KheTaskAssign(task, target_task);
}


/*****************************************************************************/
/*                                                                           */
/*  void KheTaskUnAssignResource(KHE_TASK task)                              */
/*                                                                           */
/*  Unassign task.                                                           */
/*                                                                           */
/*****************************************************************************/

void KheTaskUnAssignResource(KHE_TASK task)
{
  KheTaskUnAssign(task);
}


/*****************************************************************************/
/*                                                                           */
/*  KHE_RESOURCE KheTaskAsstResource(KHE_TASK task)                          */
/*                                                                           */
/*  Return the resource that task is assigned to, or NULL if none.           */
/*                                                                           */
/*****************************************************************************/

KHE_RESOURCE KheTaskAsstResource(KHE_TASK task)
{
  return task->assigned_rs == NULL ? NULL :
    KheResourceInSolnResource(task->assigned_rs);
}


/*****************************************************************************/
/*                                                                           */
/*  bool KheTaskAssignCheck(KHE_TASK task, KHE_TASK target_task)             */
/*                                                                           */
/*  Return true if task can be assigned to target_task.                      */
/*                                                                           */
/*****************************************************************************/

bool KheTaskAssignCheck(KHE_TASK task, KHE_TASK target_task)
{
  /* abor if task is assigned, or is a cycle task */
  MAssert(task->target_task == NULL, "KheTaskAssignCheck: task is assigned");
  MAssert(task->assigned_rs == NULL, "KheTaskAssignCheck: task is cycle task");

  /* resource domains must match */
  return KheResourceGroupSubset(target_task->domain, task->domain);
}


/*****************************************************************************/
/*                                                                           */
/*  void KheTaskDoAssignResource(KHE_TASK task, KHE_RESOURCE r)              */
/*                                                                           */
/*  Task has just been assigned to a task that has a resource, so inform     */
/*  it and its descendants of that fact.                                     */
/*                                                                           */
/*****************************************************************************/

static void KheTaskDoAssignResource(KHE_TASK task, KHE_RESOURCE r)
{
  KHE_TASK child_task;  int i;
  task->assigned_rs = task->target_task->assigned_rs;
  /* KheTaskMatchingAssign(task, r); nothing to do with this fn now */
  KheResourceInSolnAssignResource(task->assigned_rs, task);
  if( task->event_resource_in_soln != NULL )
    KheEventResourceInSolnAssignResource(task->event_resource_in_soln, task, r);
  MArrayForEach(task->assigned_tasks, &child_task, &i)
    KheTaskDoAssignResource(child_task, r);
}


/*****************************************************************************/
/*                                                                           */
/*  void KheTaskDoUnAssignResource(KHE_TASK task, KHE_RESOURCE r)            */
/*                                                                           */
/*  Task has just been unassigned from a task that has a resource, so        */
/*  inform it and its descendants of that fact.                              */
/*                                                                           */
/*****************************************************************************/

static void KheTaskDoUnAssignResource(KHE_TASK task, KHE_RESOURCE r)
{
  KHE_TASK child_task;  int i;
  /* KheTaskMatchingUnAssign(task); nothing to do with this fn now */
  KheResourceInSolnUnAssignResource(task->assigned_rs, task);
  if( task->event_resource_in_soln != NULL )
    KheEventResourceInSolnUnAssignResource(task->event_resource_in_soln,
      task, r);
  MArrayForEach(task->assigned_tasks, &child_task, &i)
    KheTaskDoUnAssignResource(child_task, r);
  task->assigned_rs = NULL;
}


/*****************************************************************************/
/*                                                                           */
/*  bool KheTaskAssign(KHE_TASK task, KHE_TASK target_task)                  */
/*                                                                           */
/*  Assign task to target_task.                                              */
/*                                                                           */
/*****************************************************************************/

bool KheTaskAssign(KHE_TASK task, KHE_TASK target_task)
{
  /* check safe to proceed */
  if( !KheTaskAssignCheck(task, target_task) )
    return false;

  /* inform soln that this is happening */
  KheSolnOpTaskAssign(task->soln, task, target_task);

  /* update the matching */
  KheTaskMatchingAssign(task, target_task);

  /* record the assignment */
  task->target_task = target_task;
  MArrayAddLast(target_task->assigned_tasks, task);

  /* if assigning a resource, inform descendants of task */
  if( target_task->assigned_rs != NULL )
    KheTaskDoAssignResource(task,
      KheResourceInSolnResource(target_task->assigned_rs));
  return true;
}


/*****************************************************************************/
/*                                                                           */
/*  void KheTaskUnAssign(KHE_TASK task)                                      */
/*                                                                           */
/*  Unassign task.                                                           */
/*                                                                           */
/*****************************************************************************/

void KheTaskUnAssign(KHE_TASK task)
{
  int pos;

  /* task must be assigned */
  MAssert(task->target_task != NULL, "KheTaskUnAssign: task not assigned");

  /* inform soln that this is happening */
  KheSolnOpTaskUnAssign(task->soln, task, task->target_task);

  /* update the matching */
  KheTaskMatchingUnAssign(task);

  /* if unassigning a resource, inform descendants of task */
  if( task->target_task->assigned_rs != NULL )
    KheTaskDoUnAssignResource(task,
      KheResourceInSolnResource(task->target_task->assigned_rs));

  /* record the unassignment */
  if( !MArrayContains(task->target_task->assigned_tasks, task, &pos) )
    MAssert(false, "KheTaskUnAssign internal error");
  MArrayRemove(task->target_task->assigned_tasks, pos);
  task->target_task = NULL;
}


/*****************************************************************************/
/*                                                                           */
/*  KHE_TASK KheTaskAsst(KHE_TASK task)                                      */
/*                                                                           */
/*  Return the task that task is currently assigned to, or NULL if none.     */
/*                                                                           */
/*****************************************************************************/

KHE_TASK KheTaskAsst(KHE_TASK task)
{
  return task->target_task;
}


/*****************************************************************************/
/*                                                                           */
/*  int KheTaskAssignedToCount(KHE_TASK target_task)                         */
/*                                                                           */
/*  Return the number of tasks assigned to target_task.                      */
/*                                                                           */
/*****************************************************************************/

int KheTaskAssignedToCount(KHE_TASK target_task)
{
  return MArraySize(target_task->assigned_tasks);
}


/*****************************************************************************/
/*                                                                           */
/*  KHE_TASK KheTaskAssignedTo(KHE_TASK target_task, int i)                  */
/*                                                                           */
/*  Return the i'th task assigned to target_task.                            */
/*                                                                           */
/*****************************************************************************/

KHE_TASK KheTaskAssignedTo(KHE_TASK target_task, int i)
{
  return MArrayGet(target_task->assigned_tasks, i);
}


/*****************************************************************************/
/*                                                                           */
/*  int KheTaskTotalDuration(KHE_TASK task)                                  */
/*                                                                           */
/*  Return the total duration of task and the tasks that are assigned to     */
/*  it, directly and indirectly.                                             */
/*                                                                           */
/*****************************************************************************/

int KheTaskTotalDuration(KHE_TASK task)
{
  int res, i;  KHE_TASK child_task;
  res = KheTaskDuration(task);
  MArrayForEach(task->assigned_tasks, &child_task, &i)
    res += KheTaskTotalDuration(child_task);
  return res;
}


/*****************************************************************************/
/*                                                                           */
/*  float KheTaskTotalWorkload(KHE_TASK task)                                */
/*                                                                           */
/*  Return the total workload of task and the tasks that are assigned to     */
/*  it, directly and indirectly.                                             */
/*                                                                           */
/*****************************************************************************/

float KheTaskTotalWorkload(KHE_TASK task)
{
  float res;  int i;  KHE_TASK child_task;
  res = KheTaskWorkload(task);
  MArrayForEach(task->assigned_tasks, &child_task, &i)
    res += KheTaskTotalWorkload(child_task);
  return res;
}


/*****************************************************************************/
/*                                                                           */
/*  bool KheTaskIsCycle(KHE_TASK task)                                       */
/*                                                                           */
/*  Return true if task is a cycle task.                                     */
/*                                                                           */
/*  Implementation note.  To save a boolean field, we don't store this       */
/*  condition explicitly in task.  Instead, we use the fact that the only    */
/*  tasks that have an assigned resource without being assigned to another   */
/*  task are cycle tasks.                                                    */
/*                                                                           */
/*****************************************************************************/

bool KheTaskIsCycle(KHE_TASK task)
{
  return task->target_task == NULL && task->assigned_rs != NULL;
}


/*****************************************************************************/
/*                                                                           */
/*  bool KheTaskIsLeader(KHE_TASK task)                                      */
/*                                                                           */
/*  Return true if task is a leader task.                                    */
/*                                                                           */
/*  Implementation note.  Every task is either a cycle task, a leader        */
/*  task, or a follower task.  It turns out that the implementation here,    */
/*  which just rules out the other two possibilities, is as quick as any.    */
/*                                                                           */
/*****************************************************************************/

bool KheTaskIsLeader(KHE_TASK task)
{
  return !KheTaskIsCycle(task) && !KheTaskIsFollower(task);
}


/*****************************************************************************/
/*                                                                           */
/*  bool KheTaskIsFollower(KHE_TASK task)                                    */
/*                                                                           */
/*  Return true if task is a follower task.                                  */
/*                                                                           */
/*  Implementation note.  By definition, a follower task is one that is      */
/*  assigned to another task which is not a cycle task.  The implementation  */
/*  simply follows the definition.                                           */
/*                                                                           */
/*****************************************************************************/

bool KheTaskIsFollower(KHE_TASK task)
{
  return task->target_task != NULL && !KheTaskIsCycle(task->target_task);
}


/*****************************************************************************/
/*                                                                           */
/*  KHE_TASK KheTaskRoot(KHE_TASK task)                                      */
/*                                                                           */
/*  Return the root of the task's assignment chain.                          */
/*                                                                           */
/*****************************************************************************/

KHE_TASK KheTaskRoot(KHE_TASK task)
{
  while( task->target_task != NULL )
    task = task->target_task;
  return task;
}


/*****************************************************************************/
/*                                                                           */
/*  KHE_TASK KheTaskLeader(KHE_TASK task)                                    */
/*                                                                           */
/*  Return the leader of task.                                               */
/*                                                                           */
/*****************************************************************************/

KHE_TASK KheTaskLeader(KHE_TASK task)
{
  MAssert(!KheTaskIsCycle(task), "KheTaskLeader: task is cycle task");
  while( KheTaskIsFollower(task) )
    task = task->target_task;
  return task;
}


/*****************************************************************************/
/*                                                                           */
/*  Submodule "moving and swapping"                                          */
/*                                                                           */
/*****************************************************************************/

/*****************************************************************************/
/*                                                                           */
/*  bool KheTaskMoveCheck(KHE_TASK task, KHE_TASK target_task)              */
/*                                                                           */
/*  Check whether task can be moved to target_task.                          */
/*                                                                           */
/*****************************************************************************/

bool KheTaskMoveCheck(KHE_TASK task, KHE_TASK target_task)
{
  KHE_TASK task_target_task;  bool res;
  if( target_task == NULL )
    return task->target_task != NULL;
  else if( task->target_task == NULL )
    return KheTaskAssignCheck(task, target_task);
  else
  {
    task_target_task = task->target_task;
    KheTaskUnAssign(task);
    res = KheTaskAssignCheck(task, target_task);
    KheTaskAssign(task, task_target_task);
    return res;
  }
}


/*****************************************************************************/
/*                                                                           */
/*  bool KheTaskMove(KHE_TASK task, KHE_TASK target_task)                    */
/*                                                                           */
/*  Move task to target_task.                                                */
/*                                                                           */
/*****************************************************************************/

bool KheTaskMove(KHE_TASK task, KHE_TASK target_task)
{
  if( !KheTaskMoveCheck(task, target_task) )
    return false;
  if( task->target_task != NULL )
    KheTaskUnAssign(task);
  if( target_task != NULL && !KheTaskAssign(task, target_task) )
    MAssert(false, "KheTaskMove internal error");
  return true;
}


/*****************************************************************************/
/*                                                                           */
/*  bool KheTaskMoveResourceCheck(KHE_TASK task, KHE_RESOURCE r)             */
/*                                                                           */
/*  Check whether it is possible to move task from wherever it is now to r.  */
/*                                                                           */
/*****************************************************************************/

bool KheTaskMoveResourceCheck(KHE_TASK task, KHE_RESOURCE r)
{
  KHE_TASK target_task;
  target_task = r == NULL ? NULL :
    KheSolnTask(task->soln, KheResourceIndexInInstance(r));
  return KheTaskMoveCheck(task, target_task);
}


/*****************************************************************************/
/*                                                                           */
/*  bool KheTaskMoveResource(KHE_TASK task, KHE_RESOURCE r)                  */
/*                                                                           */
/*  Move task from wherever it is now to r.                                  */
/*                                                                           */
/*****************************************************************************/

bool KheTaskMoveResource(KHE_TASK task, KHE_RESOURCE r)
{
  KHE_TASK target_task;
  target_task = r == NULL ? NULL :
    KheSolnTask(task->soln, KheResourceIndexInInstance(r));
  return KheTaskMove(task, target_task);
}


/*****************************************************************************/
/*                                                                           */
/*  bool KheTaskSwapCheck(KHE_TASK task1, KHE_TASK task2)                    */
/*                                                                           */
/*  Check whether swapping task1 and task2 would succeed.                    */
/*                                                                           */
/*  Implementation note.  Correct, but optimization still to do.             */
/*                                                                           */
/*****************************************************************************/

bool KheTaskSwapCheck(KHE_TASK task1, KHE_TASK task2)
{
  if( task1->target_task == task2->target_task )
    return false;
  else if( task1->target_task == NULL )
    return KheTaskMoveCheck(task1, task2->target_task);
  else if( task2->target_task == NULL )
    return KheTaskMoveCheck(task2, task1->target_task);
  else
    return KheTaskMoveCheck(task1, task2->target_task) &&
      KheTaskMoveCheck(task2, task1->target_task);
}


/*****************************************************************************/
/*                                                                           */
/*  bool KheTaskSwap(KHE_TASK task1, KHE_TASK task2)                         */
/*                                                                           */
/*  Swap task1 and task1.                                                    */
/*                                                                           */
/*****************************************************************************/

bool KheTaskSwap(KHE_TASK task1, KHE_TASK task2)
{
  KHE_TASK target_task1, target_task2;
  if( !KheTaskSwapCheck(task1, task2) )
    return false;
  if( task1->target_task == NULL )
  {
    target_task2 = task2->target_task;
    KheTaskUnAssign(task2);
    KheTaskAssign(task1, target_task2);
  }
  else if( task2->target_task == NULL )
  {
    target_task1 = task1->target_task;
    KheTaskUnAssign(task1);
    KheTaskAssign(task2, target_task1);
  }
  else
  {
    target_task1 = task1->target_task;
    target_task2 = task2->target_task;
    KheTaskUnAssign(task1);
    KheTaskUnAssign(task2);
    KheTaskAssign(task1, target_task2);
    KheTaskAssign(task2, target_task1);
  }
  return true;
}


/*****************************************************************************/
/*                                                                           */
/*  Submodule "resource preassignment and domains"                           */
/*                                                                           */
/*****************************************************************************/

/*****************************************************************************/
/*                                                                           */
/*  bool KheTaskPreassignCheck(KHE_TASK task, KHE_RESOURCE r)                */
/*                                                                           */
/*  Check whether task can be preassigned r.                                 */
/*                                                                           */
/*****************************************************************************/

bool KheTaskPreassignCheck(KHE_TASK task, KHE_RESOURCE r)
{
  return KheTaskSetDomainCheck(task, KheResourceSingletonResourceGroup(r),
    false);
}


/*****************************************************************************/
/*                                                                           */
/*  bool KheTaskPreassign(KHE_TASK task, KHE_RESOURCE r)                     */
/*                                                                           */
/*  Preassign r to task if possible.                                         */
/*                                                                           */
/*****************************************************************************/

bool KheTaskPreassign(KHE_TASK task, KHE_RESOURCE r)
{
  return KheTaskSetDomain(task, KheResourceSingletonResourceGroup(r),
    false);
}


/*****************************************************************************/
/*                                                                           */
/*  bool KheTaskIsPreassigned(KHE_TASK task, bool as_in_event_resource,      */
/*    KHE_RESOURCE *r)                                                       */
/*                                                                           */
/*  If task is preassigned, set *r to the preassigned resource and           */
/*  return true.  Otherwise, return false leaving *r untouched.              */
/*                                                                           */
/*  If as_in_event_resource is true, a task is considered to be              */
/*  preassigned if it is derived from an event resource and that             */
/*  event resource is preassigned.  If as_in_event_resource is               */
/*  false, a task is considered to be preassigned if its domain              */
/*  contains exactly one element.                                            */
/*                                                                           */
/*****************************************************************************/

bool KheTaskIsPreassigned(KHE_TASK task, bool as_in_event_resource,
  KHE_RESOURCE *r)
{
  KHE_EVENT_RESOURCE er;
  if( as_in_event_resource )
  {
    er = KheTaskEventResource(task);
    if( er == NULL )
      return false;
    else if( KheEventResourcePreassignedResource(er) != NULL )
    {
      *r = KheEventResourcePreassignedResource(er);
      return true;
    }
    else
      return false;
  }
  else
  {
    if( KheResourceGroupResourceCount(task->domain) == 1 )
    {
      *r = KheResourceGroupResource(task->domain, 0);
      return true;
    }
    else
      return false;
  }
}


/*****************************************************************************/
/*                                                                           */
/*  bool KheTaskSetDomainCheck(KHE_TASK task, KHE_RESOURCE_GROUP rg,         */
/*    bool recursive)                                                        */
/*                                                                           */
/*  Check whether operation KheSetDomain(task, rg, recursive), described     */
/*  below, can be carried out without violating the solution invariant.      */
/*                                                                           */
/*****************************************************************************/

bool KheTaskSetDomainCheck(KHE_TASK task, KHE_RESOURCE_GROUP rg,
  bool recursive)
{
  KHE_TASK child_task;  int i;

  /* task may not be a cycle task, and rg must have the right type */
  MAssert(!KheTaskIsCycle(task), "KheTaskSetDomainCheck: task is a cycle task");
  MAssert(KheResourceGroupResourceType(task->domain) ==
    KheResourceGroupResourceType(rg),
    "KheTaskSetDomainCheck: rg has wrong type");

  /* rg must be a superset of any target task's domain; this also */
  /* proves that rg contains any assigned resource */
  if( task->target_task != NULL &&
      !KheResourceGroupSubset(task->target_task->domain, rg) )
    return false;

  /* rg must be a subset of each child task's domain, unless recursive */
  if( !recursive )
    MArrayForEach(task->assigned_tasks, &child_task, &i)
      if( !KheResourceGroupSubset(rg, child_task->domain) )
	return false;
  return true;
}


/*****************************************************************************/
/*                                                                           */
/*  void KheTaskSetDomainUnchecked(KHE_TASK task, KHE_RESOURCE_GROUP rg,     */
/*    bool recursive)                                                        */
/*                                                                           */
/*  Set the domain of task to rg, possibly recursively, without checking     */
/*  whether or not this preserves the solution invariant.                    */
/*                                                                           */
/*  All operations which change domains, whether setting or tightening,      */
/*  recursive or non-recursive, ultimately call this private function to     */
/*  actually change domains.  And it is calls to this function (including    */
/*  recursive calls), not to others, that are recorded in transactions.      */
/*                                                                           */
/*****************************************************************************/

void KheTaskSetDomainUnchecked(KHE_TASK task, KHE_RESOURCE_GROUP rg,
  bool recursive)
{
  KHE_TASK child_task;  int i, assigned_time_index;

  /* make the changes necessary to update the domain of one task */
  KheSolnOpTaskSetDomain(KheTaskSoln(task), task, task->domain, rg);
  KheTaskMatchingSetDomain(task, rg);
  assigned_time_index = KheMeetAssignedTimeIndex(task->meet);
  if( assigned_time_index != -1 )
  {
    KheEvennessHandlerDeleteTask(KheSolnEvennessHandler(task->soln), task,
      assigned_time_index);
    task->domain = rg;
    KheEvennessHandlerAddTask(KheSolnEvennessHandler(task->soln), task,
      assigned_time_index);
  }
  else
    task->domain = rg;

  /* make these changes recursively if required */
  if( recursive )
    MArrayForEach(task->assigned_tasks, &child_task, &i)
      KheTaskSetDomainUnchecked(task, rg, recursive);
}


/*****************************************************************************/
/*                                                                           */
/*  bool KheTaskSetDomain(KHE_TASK task, KHE_RESOURCE_GROUP rg,              */
/*    bool recursive)                                                        */
/*                                                                           */
/*  Set the domain of task to rg.  If recursive, do this for all tasks       */
/*  assigned to task, directly or indirectly, as well.                       */
/*                                                                           */
/*****************************************************************************/

bool KheTaskSetDomain(KHE_TASK task, KHE_RESOURCE_GROUP rg, bool recursive)
{
  if( recursive || !KheResourceGroupEqual(rg, task->domain) )
  {
    if( !KheTaskSetDomainCheck(task, rg, recursive) )
      return false;
    KheTaskSetDomainUnchecked(task, rg, recursive);
  }
  return true;
}


/*****************************************************************************/
/*                                                                           */
/*  KHE_RESOURCE_GROUP KheTaskDomain(KHE_TASK task)                          */
/*                                                                           */
/*  Return the domain of task.                                               */
/*                                                                           */
/*****************************************************************************/

KHE_RESOURCE_GROUP KheTaskDomain(KHE_TASK task)
{
  return task->domain;
}


/*****************************************************************************/
/*                                                                           */
/*  bool KheTaskTightenDomainCheck(KHE_TASK task, KHE_RESOURCE_GROUP rg,     */
/*    bool recursive)                                                        */
/*                                                                           */
/*  Check whether operation KheTaskTightenDomain(task, rg, recursive),       */
/*  described below, can be done without violating the solution invariant.   */
/*                                                                           */
/*  Implementation note.  Coincidentally, this turns out to be the same      */
/*  as asking whether setting the domain to rg recursively is safe.          */
/*                                                                           */
/*****************************************************************************/

bool KheTaskTightenDomainCheck(KHE_TASK task, KHE_RESOURCE_GROUP rg,
  bool recursive)
{
  return KheTaskSetDomainCheck(task, rg, true);
}


/*****************************************************************************/
/*                                                                           */
/*  KHE_RESOURCE_GROUP KheTaskFindTightening(KHE_TASK task,                  */
/*    KHE_RESOURCE_GROUP rg)                                                 */
/*                                                                           */
/*  Return the resource group for task tightened by rg.                      */
/*                                                                           */
/*****************************************************************************/

static KHE_RESOURCE_GROUP KheTaskFindTightening(KHE_TASK task,
  KHE_RESOURCE_GROUP rg)
{
  if( KheResourceGroupSubset(task->domain, rg) )
    return task->domain;
  else if( KheResourceGroupSubset(rg, task->domain) )
    return rg;
  else
  {
    KheSolnResourceGroupBegin(task->soln, KheTaskResourceType(task));
    KheSolnResourceGroupUnion(task->soln, task->domain);
    KheSolnResourceGroupIntersect(task->soln, rg);
    return KheSolnResourceGroupEnd(task->soln);
  }
}


/*****************************************************************************/
/*                                                                           */
/*  bool KheTaskTightenDomain(KHE_TASK task, KHE_RESOURCE_GROUP rg,          */
/*    bool recursive)                                                        */
/*                                                                           */
/*  Tighten the domain of task to its intersecton with rg.  If recursive     */
/*  is true, set the domains of all tasks assigned to task, directly or      */
/*  indirectly, to this same tightened value.                                */
/*                                                                           */
/*****************************************************************************/

bool KheTaskTightenDomain(KHE_TASK task, KHE_RESOURCE_GROUP rg, bool recursive)
{
  if( !KheTaskTightenDomainCheck(task, rg, recursive) )
    return false;
  KheTaskSetDomainUnchecked(task, KheTaskFindTightening(task, rg), recursive);
  return true;
}


/*****************************************************************************/
/*                                                                           */
/*  Submodule "demand monitors"                                              */
/*                                                                           */
/*****************************************************************************/

/*****************************************************************************/
/*                                                                           */
/*  void KheTaskMatchingAttachAllOrdinaryDemandMonitors(KHE_TASK task)       */
/*                                                                           */
/*  Ensure that all the ordinary demand monitors of task are attached.       */
/*                                                                           */
/*****************************************************************************/

void KheTaskMatchingAttachAllOrdinaryDemandMonitors(KHE_TASK task)
{
  KHE_ORDINARY_DEMAND_MONITOR m;  int i;
  MArrayForEach(task->all_monitors, &m, &i)
    if( !KheMonitorAttachedToSoln((KHE_MONITOR) m) )
      KheMonitorAttachToSoln((KHE_MONITOR) m);
}


/*****************************************************************************/
/*                                                                           */
/*  void KheTaskMatchingDetachAllOrdinaryDemandMonitors(KHE_TASK task)       */
/*                                                                           */
/*  Ensure that all the ordinary demand monitors of task are detached.       */
/*                                                                           */
/*****************************************************************************/

void KheTaskMatchingDetachAllOrdinaryDemandMonitors(KHE_TASK task)
{
  while( MArraySize(task->attached_monitors) > 0 )
    KheMonitorDetachFromSoln((KHE_MONITOR) MArrayLast(task->attached_monitors));
}


/*****************************************************************************/
/*                                                                           */
/*  void KheTaskMatchingSetWeight(KHE_TASK task, KHE_COST new_weight)        */
/*                                                                           */
/*  Change the weight of all the attached monitors of task.                  */
/*  NB it would actually be wrong to do this for all monitors.               */
/*                                                                           */
/*****************************************************************************/

void KheTaskMatchingSetWeight(KHE_TASK task, KHE_COST new_weight)
{
  KHE_ORDINARY_DEMAND_MONITOR m;  int i;
  MArrayForEach(task->attached_monitors, &m, &i)
    KheOrdinaryDemandMonitorSetWeight(m, new_weight);
}


/*****************************************************************************/
/*                                                                           */
/*  int KheTaskDemandMonitorCount(KHE_TASK task)                             */
/*                                                                           */
/*  Return the number of demand monitors of task.                            */
/*                                                                           */
/*****************************************************************************/

int KheTaskDemandMonitorCount(KHE_TASK task)
{
  return MArraySize(task->all_monitors);
}


/*****************************************************************************/
/*                                                                           */
/*  KHE_ORDINARY_DEMAND_MONITOR KheTaskDemandMonitor(KHE_TASK task, int i)   */
/*                                                                           */
/*  Return the i'th demand monitor of task.                                  */
/*                                                                           */
/*****************************************************************************/

KHE_ORDINARY_DEMAND_MONITOR KheTaskDemandMonitor(KHE_TASK task, int i)
{
  return MArrayGet(task->all_monitors, i);
}


/*****************************************************************************/
/*                                                                           */
/* void KheTaskAddDemandMonitor(KHE_TASK task, KHE_ORDINARY_DEMAND_MONITOR m)*/
/*                                                                           */
/*  Add m to task.                                                           */
/*                                                                           */
/*****************************************************************************/

void KheTaskAddDemandMonitor(KHE_TASK task, KHE_ORDINARY_DEMAND_MONITOR m)
{
  MArrayAddLast(task->all_monitors, m);
}


/*****************************************************************************/
/*                                                                           */
/*  void KheTaskDeleteDemandMonitor(KHE_TASK task,                           */
/*    KHE_ORDINARY_DEMAND_MONITOR m)                                         */
/*                                                                           */
/*  Remove m from task.                                                      */
/*                                                                           */
/*****************************************************************************/

void KheTaskDeleteDemandMonitor(KHE_TASK task, KHE_ORDINARY_DEMAND_MONITOR m)
{
  int pos;
  if( !MArrayContains(task->all_monitors, m, &pos) )
    MAssert(false, "KheTaskDeleteDemandMonitor internal error");
  MArrayRemove(task->all_monitors, pos);
}


/*****************************************************************************/
/*                                                                           */
/*  void KheTaskAttachDemandMonitor(KHE_TASK task,                           */
/*    KHE_ORDINARY_DEMAND_MONITOR m)                                         */
/*                                                                           */
/*  Attach m to task.                                                        */
/*                                                                           */
/*****************************************************************************/

void KheTaskAttachDemandMonitor(KHE_TASK task, KHE_ORDINARY_DEMAND_MONITOR m)
{
  MArrayAddLast(task->attached_monitors, m);
}


/*****************************************************************************/
/*                                                                           */
/*  void KheTaskDetachDemandMonitor(KHE_TASK task,                           */
/*    KHE_ORDINARY_DEMAND_MONITOR m)                                         */
/*                                                                           */
/*  Detach m from task.                                                      */
/*                                                                           */
/*****************************************************************************/

void KheTaskDetachDemandMonitor(KHE_TASK task, KHE_ORDINARY_DEMAND_MONITOR m)
{
  int pos;
  if( !MArrayContains(task->attached_monitors, m, &pos) )
    MAssert(false, "KheTaskDetachDemandMonitor internal error");
  MArrayRemove(task->attached_monitors, pos);
}


/*****************************************************************************/
/*                                                                           */
/*  Submodule "reading and writing"                                          */
/*                                                                           */
/*****************************************************************************/

/*****************************************************************************/
/*                                                                           */
/*  bool KheTaskMakeFromKml(KML_ELT task_elt, KHE_MEET meet, KML_ERROR *ke)  */
/*                                                                           */
/*  Make a task based on task_elt and add it to meet.                        */
/*                                                                           */
/*****************************************************************************/

bool KheTaskMakeFromKml(KML_ELT task_elt, KHE_MEET meet, KML_ERROR *ke)
{
  char *role, *ref;  KHE_TASK task;  KHE_INSTANCE ins;
  KHE_EVENT_RESOURCE er;  KHE_EVENT e;
  KHE_RESOURCE resource, preassigned_resource;

  /* check task_elt */
  ins = KheSolnInstance(KheMeetSoln(meet));
  if( !KmlCheck(task_elt, "Reference : $Role", ke) )
    return false;
	  
  /* make sure the role makes sense; find the event resource */
  role = KmlExtractText(KmlChild(task_elt, 0));
  e = KheMeetEvent(meet);
  MAssert(e != NULL, "KheTaskMakeFromKml: no event (internal error)");
  if( !KheEventRetrieveEventResource(e, role, &er) )
    return KmlErrorMake(ke, KmlLineNum(task_elt), KmlColNum(task_elt),
      "<Role> \"%s\" unknown in <Event> \"%s\"", role, KheEventId(e));

  /* make sure that meet does not already have a task for this */
  if( KheMeetRetrieveTask(meet, role, &task) )
    return KmlErrorMake(ke, KmlLineNum(task_elt), KmlColNum(task_elt),
      "<Role> \"%s\" already assigned in <Event> \"%s\"", role, KheEventId(e));

  /* make sure the reference is to a legal resource */
  ref = KmlAttributeValue(task_elt, 0);
  if( !KheInstanceRetrieveResource(ins, ref, &resource) )
    return KmlErrorMake(ke, KmlLineNum(task_elt), KmlColNum(task_elt),
      "<Resource> Reference \"%s\" unknown", ref);

  /* check that resource is compatible with preassignment */
  preassigned_resource = KheEventResourcePreassignedResource(er);
  if( preassigned_resource != NULL && preassigned_resource != resource )
    return KmlErrorMake(ke, KmlLineNum(task_elt), KmlColNum(task_elt),
      "<Resource> \"%s\" conflicts with preassigned resource \"%s\"",
      ref, KheResourceId(preassigned_resource));

  /* check that resource is compatible with ResourceType */
  if( KheResourceResourceType(resource) != KheEventResourceResourceType(er) )
    return KmlErrorMake(ke, KmlLineNum(task_elt), KmlColNum(task_elt),
      "<Resource> of type \"%s\" where type \"%s\" expected",
      KheResourceTypeId(KheResourceResourceType(resource)),
      KheResourceTypeId(KheEventResourceResourceType(er)));

  /* make a task, link it to meet, and assign resource to it */
  task = KheTaskMake(KheMeetSoln(meet), KheEventResourceResourceType(er),
    meet, er);
  if( !KheTaskAssignResource(task, resource) )
    return KmlErrorMake(ke, KmlLineNum(task_elt), KmlColNum(task_elt),
      "<Resource> \"%s\" unassignable here", ref);
  return true;
}


/*****************************************************************************/
/*                                                                           */
/*  bool KheTaskMustWrite(KHE_TASK task, KHE_EVENT_RESOURCE er)              */
/*                                                                           */
/*  Return true if it is necessary to write task, because it is assigned     */
/*  a value which is not a preassigned value.                                */
/*                                                                           */
/*****************************************************************************/

bool KheTaskMustWrite(KHE_TASK task, KHE_EVENT_RESOURCE er)
{
  return task->assigned_rs != NULL &&
    KheResourceInSolnResource(task->assigned_rs) !=
      KheEventResourcePreassignedResource(er);
}


/*****************************************************************************/
/*                                                                           */
/*  bool KheTaskWrite(KHE_TASK task, KHE_EVENT_RESOURCE er, KML_FILE kf)     */
/*                                                                           */
/*  Write task to kf.                                                        */
/*                                                                           */
/*****************************************************************************/

bool KheTaskWrite(KHE_TASK task, KHE_EVENT_RESOURCE er, KML_FILE kf)
{
  if( KheEventResourceRole(er) == NULL )
    return false;
  KmlEltAttributeEltPrintf(kf, "Resource", "Reference",
    KheResourceId(KheResourceInSolnResource(task->assigned_rs)), "Role",
    "%s", KheEventResourceRole(er));
  return true;
}


/*****************************************************************************/
/*                                                                           */
/*  Submodule "debug"                                                        */
/*                                                                           */
/*****************************************************************************/

/*****************************************************************************/
/*                                                                           */
/*  void KheTaskDebug(KHE_TASK task, int verbosity, int indent, FILE *fp)    */
/*                                                                           */
/*  Debug print of task onto fp with the given verbosity and indent.         */
/*                                                                           */
/*****************************************************************************/

void KheTaskDebug(KHE_TASK task, int verbosity, int indent, FILE *fp)
{
  KHE_RESOURCE r;  KHE_TIME t;  KHE_TASK child_task;  int i, j, pos;
  ARRAY_KHE_EVENT_RESOURCE event_resources;  KHE_EVENT_RESOURCE er;
  if( verbosity >= 1 )
  {
    if( indent >= 0 )
      fprintf(fp, "%*s", indent, "");
    if( verbosity > 1 )
      fprintf(fp, "[ ");
    if( KheTaskIsCycle(task) )
    {
      r = KheResourceInSolnResource(task->assigned_rs);
      fprintf(fp, "/%s/", KheResourceId(r) != NULL ? KheResourceId(r) : "-");
    }
    else
    {
      if( task->meet != NULL )
      {
	KheMeetDebug(task->meet, 1, -1, fp);
	fprintf(fp, ".%d", task->index_in_meet);
	t = KheMeetAsstTime(task->meet);
	if( t != NULL )
	{
	  fprintf(stderr, "$%s", KheTimeId(t) == NULL ? "-" : KheTimeId(t));
	  if( KheTaskDuration(task) > 1 )
	  {
	    i = KheTimeIndex(t) + KheTaskDuration(task) - 1;
	    t = KheInstanceTime(KheSolnInstance(KheTaskSoln(task)), i);
	    fprintf(stderr, "-%s", KheTimeId(t) == NULL ? "-" : KheTimeId(t));
	  }
	}
      }
      if( MArraySize(task->assigned_tasks) > 0 )
      {
	MArrayInit(event_resources);
	MArrayForEach(task->assigned_tasks, &child_task, &i)
	{
	  er = KheTaskEventResource(child_task);
	  if( !MArrayContains(event_resources, er, &pos) )
	    MArrayAddLast(event_resources, er); /* may add NULL */
	}
	fprintf(fp, "{");
	MArrayForEach(event_resources, &er, &i)
	{
	  if( i > 0 )
	    fprintf(fp, ", ");
	  fprintf(fp, "%s", er == NULL ? "??" :
	    KheEventId(KheEventResourceEvent(er)) == NULL ?  "-" :
	    KheEventId(KheEventResourceEvent(er)));
	  MArrayForEach(task->assigned_tasks, &child_task, &j)
	    if( KheTaskEventResource(child_task) == er )
	    {
	      if( child_task->meet == NULL )
		fprintf(fp, "+??");
	      else if( KheMeetAsstTime(child_task->meet) == NULL )
		fprintf(fp, "+__");
	      else
	      {
		t = KheMeetAsstTime(child_task->meet);
		fprintf(fp, "+%s", KheTimeId(t) == NULL ? "-" : KheTimeId(t));
	      }
	    }
	}
	fprintf(fp, "}");
	MArrayFree(event_resources);
      }
    }
    if( verbosity >= 2 )
    {
      fprintf(fp, ": ");
      KheResourceGroupDebug(task->domain, 1, -1, fp);
      if( task->assigned_rs != NULL )
      {
	fprintf(fp, " := ");
	KheResourceDebug(KheResourceInSolnResource(task->assigned_rs),
	  1, -1, fp);
      }
    }
    if( verbosity > 1 )
      fprintf(fp, " ]");
    if( indent >= 0 )
      fprintf(fp, "\n");
  }
}
