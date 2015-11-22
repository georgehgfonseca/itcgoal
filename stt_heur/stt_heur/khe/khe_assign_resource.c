
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

#define DEBUG1 0
#define DEBUG2 0
#define DEBUG3 0
#define DEBUG4 0
#define DEBUG5 0
#define DEBUG6 0
#define DEBUG7 0
#define DEBUG8 0

typedef MARRAY(KHE_TASK) ARRAY_KHE_TASK;

/*****************************************************************************/
/*                                                                           */
/*  Submodule "preserving the resource assignment invariant"                 */
/*                                                                           */
/*****************************************************************************/

/*****************************************************************************/
/*                                                                           */
/*  void KheInvariantTransactionBegin(KHE_TRANSACTION t, int *init_count,    */
/*    bool preserve_invariant)                                               */
/*                                                                           */
/*  Begin a transaction that has to not increase the matching cost if        */
/*  preserve_invariant is true.                                              */
/*                                                                           */
/*****************************************************************************/

void KheInvariantTransactionBegin(KHE_TRANSACTION t, int *init_count,
  bool preserve_invariant)
{
  KHE_SOLN soln;
  if( preserve_invariant )
  {
    soln = KheTransactionSoln(t);
    *init_count = KheSolnMatchingDefectCount(soln);
    KheSolnMatchingMarkBegin(soln);
    KheTransactionBegin(t);
  }
}


/*****************************************************************************/
/*                                                                           */
/*  bool KheInvariantTransactionEnd(KHE_TRANSACTION t,                       */
/*    int *init_count, bool preserve_invariant, bool success)                */
/*                                                                           */
/*  End a transaction that has to not increase the matching cost if          */
/*  preserve_invariant is true.                                              */
/*                                                                           */
/*****************************************************************************/

bool KheInvariantTransactionEnd(KHE_TRANSACTION t,
  int *init_count, bool preserve_invariant, bool success)
{
  KHE_SOLN soln;
  if( preserve_invariant )
  {
    soln = KheTransactionSoln(t);
    KheTransactionEnd(t);
    if( KheSolnMatchingDefectCount(soln) > *init_count )
      success = false;
    if( !success )
      KheTransactionUndo(t);
    KheSolnMatchingMarkEnd(soln, !success);
  }
  return success;
}


/*****************************************************************************/
/*                                                                           */
/*  Submodule "Kempe resource assignments"                                   */
/*                                                                           */
/*****************************************************************************/

/*****************************************************************************/
/*                                                                           */
/*  bool KheTaskAssignedTimesIntersect(KHE_TASK task, KHE_TIME_GROUP tg)     */
/*                                                                           */
/*  Return true if task is running at any of the times of tg.                */
/*                                                                           */
/*****************************************************************************/

static bool KheTaskAssignedTimesIntersect(KHE_TASK task, KHE_TIME_GROUP tg)
{
  int i;  KHE_TASK child_task;  KHE_MEET meet;  KHE_TIME t;

  /* check task itself */
  meet = KheTaskMeet(task);
  if( meet != NULL && KheMeetAsstTime(meet) != NULL )
  {
    t = KheMeetAsstTime(meet);
    for( i = 0;  i < KheMeetDuration(meet);  i++ )
      if( KheTimeGroupContains(tg, KheTimeNeighbour(t, i)) )
	return true;
  }

  /* check the tasks assigned to task */
  for( i = 0;  i < KheTaskAssignedToCount(task);  i++ )
  {
    child_task = KheTaskAssignedTo(task, i);
    if( KheTaskAssignedTimesIntersect(child_task, tg) )
      return true;
  }

  /* return false if found nothing */
  return false;
}


/*****************************************************************************/
/*                                                                           */
/*  bool KheTaskUnAssignClashing(KHE_TASK task, KHE_RESOURCE r,              */
/*    KHE_TIMETABLE_MONITOR tm)                                              */
/*                                                                           */
/*  Unassign every task in r's timetable tm that would clash with task, and  */
/*  return true; or if one of those tasks is preassigned r, return false.    */
/*                                                                           */
/*****************************************************************************/

static bool KheTaskUnAssignClashing(KHE_TASK task, KHE_RESOURCE r,
  KHE_TIMETABLE_MONITOR tm)
{
  int i;  KHE_TASK child_task, task2;  KHE_MEET meet, meet2;
  KHE_TIME t, time;  KHE_RESOURCE junk;

  /* unassign tasks that clash with task itself */
  meet = KheTaskMeet(task);
  if( meet != NULL && KheMeetAsstTime(meet) != NULL )
  {
    time = KheMeetAsstTime(meet);
    for( i = 0;  i < KheMeetDuration(meet);  i++ )
    {
      t = KheTimeNeighbour(time, i);
      while( KheTimetableMonitorTimeMeetCount(tm, t) > 0 )
      {
	meet2 = KheTimetableMonitorTimeMeet(tm, t, 0);
	if( !KheMeetContainsResourceAssignment(meet2, r, &task2) )
	  MAssert(false, "KheTaskKempeAssignResource internal error");
	task2 = KheTaskLeader(task2);
	if( KheTaskIsPreassigned(task2, false, &junk) )
	  return false;
	KheTaskUnAssignResource(task2);
      }
    }
  }

  /* unassign tasks that clash with task's child tasks */
  for( i = 0;  i < KheTaskAssignedToCount(task);  i++ )
  {
    child_task = KheTaskAssignedTo(task, i);
    if( !KheTaskUnAssignClashing(child_task, r, tm) )
      return false;
  }

  /* return true since no problems arose */
  return true;
}


/*****************************************************************************/
/*                                                                           */
/*  bool KheTaskKempeAssignResource(KHE_TASK task, KHE_RESOURCE r)           */
/*                                                                           */
/*  Carry out a Kempe resource assignment of r to task.  This version        */
/*  does not worry about the matching, it merely fails on preassigned        */
/*  times and deassigns clashing tasks.                                      */
/*                                                                           */
/*****************************************************************************/

bool KheTaskKempeAssignResource(KHE_TASK task, KHE_RESOURCE r)
{
  KHE_TIME_GROUP unavail_times;  bool res;  KHE_TIMETABLE_MONITOR tm;
  if( DEBUG5 )
  {
    fprintf(stderr, "[ KheTaskKempeAssignResource(");
    KheTaskDebug(task, 1, -1, stderr);
    fprintf(stderr, ", ");
    KheResourceDebug(r, 1, -1, stderr);
    fprintf(stderr, ")\n");
  }
  MAssert(KheTaskIsLeader(task), "KheTaskKempeAssignResource: task not leader");

  /* fail if r is unavailable at any of task's assigned times */
  unavail_times = KheResourceHardUnavailableTimeGroup(r);
  if( KheTaskAssignedTimesIntersect(task, unavail_times) )
  {
    if( DEBUG5 )
      fprintf(stderr, "] KheTaskKempeAssignResource ret. false (unavail)\n");
    return false;
  }

  /* unassign everything that will clash with task */
  tm = KheResourceTimetableMonitor(KheTaskSoln(task), r);
  if( !KheMonitorAttachedToSoln((KHE_MONITOR) tm) )
    KheMonitorAttachToSoln((KHE_MONITOR) tm);
  if( !KheTaskUnAssignClashing(task, r, tm) )
  {
    if( DEBUG5 )
      fprintf(stderr, "] KheTaskKempeAssignResource ret. false (preass)\n");
    return false;
  }

  /* make the assignment and fail if it fails */
  res = KheTaskAssignResource(task, r);
  if( DEBUG5 )
    fprintf(stderr, "] KheTaskKempeAssignResource returning %s\n",
      res ? "true" : "false");
  return res;
}

/* *** old version that erroneously did not work with leader tasks 
bool KheTaskKempeAssignResource(KHE_TASK task, KHE_RESOURCE r)
{
  int i, ti, offset;  KHE_TIME_GROUP unavail_times;  KHE_RESOURCE junk;
  KHE_MEET meet, meet2;  KHE_TIME t;  KHE_INSTANCE ins;
  KHE_TASK task2;  KHE_TIMETABLE_MONITOR tm;
  if( DEBUG5 )
  {
    fprintf(stderr, "[ KheTaskKempeAssignResource(");
    KheTaskDebug(task, 1, -1, stderr);
    fprintf(stderr, ", ");
    KheResourceDebug(r, 1, -1, stderr);
    fprintf(stderr, ")\n");
  }

  meet = KheTaskMeet(task);
  if( meet == NULL || KheMeetAsstTime(meet) == NULL )
  {
    ** the task has no assigned time, so just do it **
    return KheTaskAssignResource(task, r);
  }
  else
  {
    ** fail if r is unavailable at any of the times of task's meet **
    unavail_times = KheResourceHardUnavailableTimeGroup(r);
    ti = KheTimeIndex(KheMeetAsstTime(meet));
    ins = KheResourceInstance(r);
    for( offset = 0;  offset < KheMeetDuration(meet);  offset++ )
    {
      t = KheInstanceTime(ins, ti + offset);
      if( KheTimeGroupContains(unavail_times, t) )
	return false;
    }

    ** make the assignment and fail if it fails **
    if( !KheTaskAssignResource(task, r) )
      return false;

    ** unassign everything that clashes with task now **
    tm = KheResourceTimetableMonitor(KheTaskSoln(task), r);
    for( offset = 0;  offset < KheMeetDuration(meet);  offset++ )
    {
      t = KheInstanceTime(ins, ti + offset);
      for( i = 0;  i < KheTimetableMonitorTimeMeetCount(tm, t);  i++ )
      {
	meet2 = KheTimetableMonitorTimeMeet(tm, t, i);
	if( meet2 != meet )
	{
	  if( !KheMeetContainsResourceAssignment(meet2, r, &task2) )
	    MAssert(false, "KheTaskKempeAssignResource internal error");
	  if( KheTaskIsPreassigned(task2, &junk) )
	    return false;
	  KheTaskUnAssignResource(task2);
	  i--;
	}
      }
    }

    ** all in order **
    return true;
  }
}
*** */


/*****************************************************************************/
/*                                                                           */
/*  Submodule "putting it all together"                                      */
/*                                                                           */
/*****************************************************************************/

/*****************************************************************************/
/*                                                                           */
/*  void KheTaskingAssignResources(KHE_TASKING tasking)                      */
/*                                                                           */
/*  Assign resources to the leader tasks of tasking.                         */
/*                                                                           */
/*****************************************************************************/

void KheTaskingAssignResources(KHE_TASKING tasking)
{
  KHE_TASK_JOB_TYPE tjt;  KHE_RESOURCE_TYPE rt;
  bool with_two_colour = true;
  bool skip_after_packing = false;
  if( DEBUG8 )
    fprintf(stderr, "[ KheTaskingAssignResources(%s)\n",
      KheTaskingResourceType(tasking) == NULL ? "~" :
      KheResourceTypeId(KheTaskingResourceType(tasking)) == NULL ? "-" :
      KheResourceTypeId(KheTaskingResourceType(tasking)));

  /* add soft contstraints to the existing hard constraints */
  tjt = KHE_TASK_JOB_HARD_PRC | KHE_TASK_JOB_SOFT_PRC |
        KHE_TASK_JOB_HARD_ASAC | KHE_TASK_JOB_SOFT_ASAC;
  KheTaskingMakeTaskTree(tasking, tjt, true, true, true);

  /* use resource packing or most-constrained-first, and repair */
  rt = KheTaskingResourceType(tasking);
  if( rt == NULL || KheResourceTypeAvoidSplitAssignmentsCount(rt) > 0 )
    KheResourcePackAssignResources(tasking);
  else
    KheMostConstrainedFirstAssignResources(tasking);
  if( !skip_after_packing )
  {
    KheEjectionChainRepairResources(tasking, true);

    /* find split assignments */
    KheFindSplitResourceAssignments(tasking);
    KheTaskingAllowSplitAssignments(tasking, false);
    if( DEBUG8 )
      fprintf(stderr, "  KheTaskingAssignResources %s repairing splits:\n",
	KheTaskingResourceType(tasking) == NULL ? "~" :
	KheResourceTypeId(KheTaskingResourceType(tasking)) == NULL ? "-" :
	KheResourceTypeId(KheTaskingResourceType(tasking)));
    KheEjectionChainRepairResources(tasking, true);
    if( with_two_colour )
      KheTwoColourRepairSplitAssignments(tasking, true);

    /* make final repairs */
    KheEjectionChainRepairResources(tasking, false);
    KheTaskingEnlargeDomains(tasking, true);
    KheEjectionChainRepairResources(tasking, false);
  }
  if( DEBUG8 )
    fprintf(stderr, "] KheTaskingAssignResources returning\n");
}
