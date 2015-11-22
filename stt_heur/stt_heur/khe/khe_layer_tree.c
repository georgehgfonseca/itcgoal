
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
/*  FILE:         khe_split_link_and_layer.c                                 */
/*  DESCRIPTION:  KheSplitLinkAndLayer() algorithm                           */
/*                                                                           */
/*****************************************************************************/
#include "khe_layer_tree.h"
#include <sys/time.h>

#define DEBUG1 0
#define DEBUG2 0
#define DEBUG3 0


/*****************************************************************************/
/*                                                                           */
/*  void KheSplitAddConstraintTasks(KHE_SOLN soln,                           */
/*    ARRAY_KHE_SPLIT_TASK *split_tasks)                                     */
/*                                                                           */
/*  Add to *split_tasks those tasks of ins that derived from constraints.    */
/*                                                                           */
/*****************************************************************************/

static void KheSplitAddConstraintTasks(KHE_SOLN soln,
  ARRAY_KHE_SPLIT_TASK *split_tasks)
{
  KHE_INSTANCE ins;  KHE_CONSTRAINT c;  int i;
  ins = KheSolnInstance(soln);
  for( i = 0;  i < KheInstanceConstraintCount(ins);  i++ )
  {
    c = KheInstanceConstraint(ins, i);
    switch( KheConstraintTag(c) )
    {
      case KHE_SPLIT_EVENTS_CONSTRAINT_TAG:

	MArrayAddLast(*split_tasks, (KHE_SPLIT_TASK)
	  KheSplitSplitTaskMake((KHE_SPLIT_EVENTS_CONSTRAINT) c));
	break;

      case KHE_DISTRIBUTE_SPLIT_EVENTS_CONSTRAINT_TAG:

	MArrayAddLast(*split_tasks, (KHE_SPLIT_TASK)
	  KheDistributeSplitTaskMake(
	    (KHE_DISTRIBUTE_SPLIT_EVENTS_CONSTRAINT) c));
	break;

      case KHE_PREFER_TIMES_CONSTRAINT_TAG:

	MArrayAddLast(*split_tasks, (KHE_SPLIT_TASK)
	  KheDomainSplitTaskMake((KHE_PREFER_TIMES_CONSTRAINT) c));
	break;

      case KHE_SPREAD_EVENTS_CONSTRAINT_TAG:

	MArrayAddLast(*split_tasks, (KHE_SPLIT_TASK)
	  KheSpreadSplitTaskMake((KHE_SPREAD_EVENTS_CONSTRAINT) c));
	break;

      case KHE_LINK_EVENTS_CONSTRAINT_TAG:

	MArrayAddLast(*split_tasks, (KHE_SPLIT_TASK)
	  KheLinkSplitTaskMake((KHE_LINK_EVENTS_CONSTRAINT) c));
	break;

      case KHE_AVOID_CLASHES_CONSTRAINT_TAG:

	MArrayAddLast(*split_tasks, (KHE_SPLIT_TASK)
	  KheAvoidClashesSplitTaskMake((KHE_AVOID_CLASHES_CONSTRAINT) c));
	break;

      case KHE_ASSIGN_RESOURCE_CONSTRAINT_TAG:
      case KHE_ASSIGN_TIME_CONSTRAINT_TAG:
      case KHE_PREFER_RESOURCES_CONSTRAINT_TAG:
      case KHE_AVOID_SPLIT_ASSIGNMENTS_CONSTRAINT_TAG:
      case KHE_AVOID_UNAVAILABLE_TIMES_CONSTRAINT_TAG:
      case KHE_LIMIT_IDLE_TIMES_CONSTRAINT_TAG:
      case KHE_CLUSTER_BUSY_TIMES_CONSTRAINT_TAG:
      case KHE_LIMIT_BUSY_TIMES_CONSTRAINT_TAG:
      case KHE_LIMIT_WORKLOAD_CONSTRAINT_TAG:

	/* nothing to do for these constraints */
	break;

      default:

	MAssert(false,
	  "KheSplitAddConstraintTasks given unknown constraint type");
    }
  }
}


/*****************************************************************************/
/*                                                                           */
/*  void KheSplitAddPreExistingSplitTasks(KHE_SOLN soln,                     */
/*    ARRAY_KHE_SPLIT_TASK *split_tasks)                                     */
/*                                                                           */
/*  Add pre-existing split tasks to *split_tasks.                            */
/*                                                                           */
/*****************************************************************************/

static void KheSplitAddPreExistingSplitTasks(KHE_SOLN soln,
  ARRAY_KHE_SPLIT_TASK *split_tasks)
{
  KHE_INSTANCE ins;  KHE_EVENT e;  int i, j, durn;  KHE_PARTITION p;
  KHE_MEET meet;
  ins = KheSolnInstance(soln);
  for( i = 0;  i < KheInstanceEventCount(ins);  i++ )
  {
    e = KheInstanceEvent(ins, i);
    durn = KheEventDuration(e);
    if( KheEventMeetCount(soln, e) != 1 ||
	KheMeetDuration(KheEventMeet(soln, e, 0)) != durn )
    {
      p = KhePartitionMake();
      for( j = 0;  i < KheEventMeetCount(soln, e);  j++ )
      {
	meet = KheEventMeet(soln, e, j);
	KhePartitionAdd(p, KheMeetDuration(meet));
      }
      MArrayAddLast(*split_tasks, (KHE_SPLIT_TASK) KhePackSplitTaskMake(e, p));
    }
  }
}


/*****************************************************************************/
/*                                                                           */
/*  void KheSplitAddPreassignedSplitTasks(KHE_SOLN soln,                     */
/*    ARRAY_KHE_SPLIT_TASK *split_tasks)                                     */
/*                                                                           */
/*  Add tasks to *split_tasks installing preassigned times.                  */
/*                                                                           */
/*****************************************************************************/

static void KheSplitAddPreassignedSplitTasks(KHE_SOLN soln,
  ARRAY_KHE_SPLIT_TASK *split_tasks)
{
  KHE_INSTANCE ins;  KHE_EVENT e;  int i;
  ins = KheSolnInstance(soln);
  for( i = 0;  i < KheInstanceEventCount(ins);  i++ )
  {
    e = KheInstanceEvent(ins, i);
    if( KheEventPreassignedTime(e) != NULL )
      MArrayAddLast(*split_tasks,
	(KHE_SPLIT_TASK) KhePreassignedSplitTaskMake(e));
  }
}


/*****************************************************************************/
/*                                                                           */
/*  void KheSplitAddLayerTasks(KHE_SOLN soln,                                */
/*    ARRAY_KHE_SPLIT_TASK *split_tasks)                                     */
/*                                                                           */
/*  Add one task to *split_tasks for each pre-existing layer.                */
/*                                                                           */
/*****************************************************************************/

/* ***
static void KheSplitAddLayerTasks(KHE_SOLN soln,
  ARRAY_KHE_SPLIT_TASK *split_tasks)
{
  int i;  KHE_LAYER layer;
  for( i = 0;  i < KheSolnLayerCount(soln);  i++ )
  {
    layer = KheSolnLayer(soln, i);
    MArrayAddLast(*split_tasks, (KHE_SPLIT_TASK) KheLayerSplitTaskMake(layer));
  }
}
*** */


/*****************************************************************************/
/*                                                                           */
/*  void KheSplitAddAssignedSplitTasks(KHE_SOLN soln,                        */
/*    ARRAY_KHE_SPLIT_TASK *split_tasks)                                     */
/*                                                                           */
/*  Add tasks for installing existing assignments.                           */
/*                                                                           */
/*****************************************************************************/

static void KheSplitAddAssignedSplitTasks(KHE_SOLN soln,
  ARRAY_KHE_SPLIT_TASK *split_tasks)
{
  KHE_INSTANCE ins;  KHE_EVENT e;  int i, j;  KHE_MEET meet, meet2;
  ins = KheSolnInstance(soln);
  for( i = 0;  i < KheInstanceEventCount(ins);  i++ )
  {
    e = KheInstanceEvent(ins, i);
    for( j = 0;  j < KheEventMeetCount(soln, e);  j++ )
    {
      meet = KheEventMeet(soln, e, j);
      meet2 = KheMeetAsst(meet);
      if( meet2 != NULL && KheMeetEvent(meet2) != NULL )
      {
	MArrayAddLast(*split_tasks, (KHE_SPLIT_TASK)
	  KheAssignedSplitTaskMake(e, KheMeetEvent(meet2)));
	break;
      }
    }
  }
}


/*****************************************************************************/
/*                                                                           */
/*  void KheSplitEventHomogenizeAssignments(KHE_SOLN soln, KHE_EVENT e)      */
/*                                                                           */
/*  Homogenize the assignments of e, so that they are all either to          */
/*  meets of the same event, or to cycle meets.                              */
/*                                                                           */
/*****************************************************************************/

static void KheSplitEventHomogenizeAssignments(KHE_SOLN soln, KHE_EVENT e)
{
  int i;  KHE_MEET meet, first_asst;
  first_asst = NULL;
  for( i = 0;  i < KheEventMeetCount(soln, e);  i++ )
  {
    meet = KheEventMeet(soln, e, i);
    if( KheMeetAsst(meet) != NULL )
    {
      if( first_asst == NULL )
	first_asst = KheMeetAsst(meet);
      else if( KheMeetIsCycleMeet(first_asst) )
      {
	if( !KheMeetIsCycleMeet(KheMeetAsst(meet)) )
	{
	  /* first_asst is a cycle meet, but this isn't, so deassign */
	  if( DEBUG1 )
	    fprintf(stderr, "  unassigning meet of %s (not cycle)\n",
	      KheEventId(e) != NULL ? KheEventId(e) : "-");
	  KheMeetUnAssign(meet);
	}
      }
      else if( KheMeetEvent(first_asst) != NULL )
      {
	if( KheMeetEvent(KheMeetAsst(meet)) !=
	    KheMeetEvent(first_asst) )
	{
	  /* first_asst is to event, but this asst isn't, so deassign */
	  if( DEBUG1 )
	    fprintf(stderr, "  unassigning meet of %s (not event)\n",
	      KheEventId(e) != NULL ? KheEventId(e) : "-");
	  KheMeetUnAssign(meet);
	}
      }
    }
  }
}


/*****************************************************************************/
/*                                                                           */
/*  bool KheSplitLayerIsFullyAssigned(KHE_LAYER layer)                       */
/*                                                                           */
/*  Return true if every meet of layer is assigned.                          */
/*                                                                           */
/*****************************************************************************/

/* ***
static bool KheSplitLayerIsFullyAssigned(KHE_LAYER layer)
{
  int i;  KHE_MEET meet;
  for( i = 0;  i < KheLayerMeetCount(layer);  i++ )
  {
    meet = KheLayerMeet(layer, i);
    if( KheMeetAsst(meet) == NULL )
      return false;
  }
  return true;
}
*** */


/*****************************************************************************/
/*                                                                           */
/*  See khe_test.c for these                                                 */
/*                                                                           */
/*****************************************************************************/

static void KheTestTimeOfDayBegin(struct timeval *tv)
{
  gettimeofday(tv, NULL);
}

static float KheTestTimeOfDayEnd(struct timeval *tv)
{
  struct timeval end_tv;
  gettimeofday(&end_tv, NULL);
  return (float) (end_tv.tv_sec - tv->tv_sec) +
    (float) (end_tv.tv_usec - tv->tv_usec) / 1000000.0;
}


/*****************************************************************************/
/*                                                                           */
/*  bool KheEventMeetsLieInNode(KHE_SOLN soln, KHE_EVENT e)                  */
/*                                                                           */
/*  Return true if the meets of e all lie in the same node.                  */
/*                                                                           */
/*****************************************************************************/

/* *** no longer used
static bool KheEventMeetsLieInNode(KHE_SOLN soln, KHE_EVENT e)
{
  KHE_MEET meet;  int i;  KHE_NODE node;
  MAssert(KheEventMeetCount(soln, e) >= 1,
    "KheEventMeetsLieInNode internal error");
  node = KheMeetNode(KheEventMeet(soln, e, 0));
  if( node == NULL )
    return false;
  for( i = 1;  i < KheEventMeetCount(soln, e);  i++ )
  {
    meet = KheEventMeet(soln, e, i);
    if( KheMeetNode(meet) != node )
      return false;
  }
  return true;
}
*** */


/*****************************************************************************/
/*                                                                           */
/*  bool KheEventSolEventsLieOutsideNodes(KHE_SOLN soln, KHE_EVENT e)        */
/*                                                                           */
/*  Return true if the meets of e all lie outside nodes.                     */
/*                                                                           */
/*****************************************************************************/

/* *** no longer used
static bool KheEventSolEventsLieOutsideNodes(KHE_SOLN soln, KHE_EVENT e)
{
  KHE_MEET meet;  int i;
  for( i = 0;  i < KheEventMeetCount(soln, e);  i++ )
  {
    meet = KheEventMeet(soln, e, i);
    if( KheMeetNode(meet) != NULL )
      return false;
  }
  return true;
}
*** */


/*****************************************************************************/
/*                                                                           */
/*  bool KheEventMeetsAssignedTo(KHE_SOLN soln, KHE_EVENT e,                 */
/*    KHE_EVENT leader_event)                                                */
/*                                                                           */
/*  Return true if all the meets of e are assigned to meets                  */
/*  of leader_event of the same duration as themselves.                      */
/*                                                                           */
/*****************************************************************************/

/* *** no longer used
static bool KheEventMeetsAssignedTo(KHE_SOLN soln, KHE_EVENT e,
  KHE_EVENT leader_event)
{
  KHE_MEET meet;  int i;
  for( i = 0;  i < KheEventMeetCount(soln, e);  i++ )
  {
    meet = KheEventMeet(soln, e, i);
    if( KheMeetAsst(meet) == NULL ||
	KheMeetEvent(KheMeetAsst(meet)) != leader_event ||
	KheMeetDuration(meet) != KheMeetDuration(KheMeetAsst(meet)) )
      return false;
  }
  return true;
}
*** */


/*****************************************************************************/
/*                                                                           */
/*  bool KheEventMeetsAssignedDistinctly(KHE_SOLN soln, KHE_EVENT e)         */
/*                                                                           */
/*  Return true if the meets of e are assigned to distinct meets.            */
/*                                                                           */
/*****************************************************************************/

/* *** no longer used
static bool KheEventMeetsAssignedDistinctly(KHE_SOLN soln, KHE_EVENT e)
{
  KHE_MEET meet, meet2;  int i, j;
  for( i = 1;  i < KheEventMeetCount(soln, e);  i++ )
  {
    meet = KheEventMeet(soln, e, i);
    for( j = 0;  j < i;  j++ )
    {
      meet2 = KheEventMeet(soln, e, j);
      if( KheMeetAsst(meet) == KheMeetAsst(meet2) )
	return false;
    }
  }
  return true;
}
*** */


/*****************************************************************************/
/*                                                                           */
/*  bool KheLayerTreeLinkEventsMonitorMustHaveZeroCost(KHE_SOLN soln,        */
/*    KHE_EVENT e, KHE_LINK_EVENTS_MONITOR m)                                */
/*                                                                           */
/*  Return true if m must have zero cost, taking e as the leader event.      */
/*                                                                           */
/*****************************************************************************/

/* *** replaced by KheMonitorAttachCheck
static bool KheLayerTreeLinkEventsMonitorMustHaveZeroCost(KHE_SOLN soln,
  KHE_EVENT e, KHE_LINK_EVENTS_MONITOR m)
{
  KHE_EVENT_GROUP eg;  KHE_EVENT e2;  int i;

  ** return false if e's meets don't lie in a common node **
  if( !KheEventMeetsLieInNode(soln, e) )
    return false;

  ** check the other events of the monitor's event group **
  eg = KheLinkEventsMonitorEventGroup(m);
  for( i = 0;  i < KheEventGroupEventCount(eg);  i++ )
  {
    e2 = KheEventGroupEvent(eg, i);
    if( e2 != e )
    {
      ** return false if e2's meets lie in nodes **
      if( !KheEventSolEventsLieOutsideNodes(soln, e2) )
	return false;

      ** return false if e2's meets are not assigned to soln **
      ** events of e of the same duration as themselves **
      if( !KheEventMeetsAssignedTo(soln, e2, e) )
	return false;

      ** return false if e2's meets are not assigned distinctly **
      if( !KheEventMeetsAssignedDistinctly(soln, e2) )
	return false;
    }
  }

  ** all tests passed **
  return true;
}
*** */


/*****************************************************************************/
/*                                                                           */
/*  bool KheLayerTreePreferTimesMonitorMustHaveZeroCost(KHE_SOLN soln,       */
/*    KHE_EVENT e, KHE_PREFER_TIMES_MONITOR m)                               */
/*                                                                           */
/*  Return true if this monitor must have cost 0, given the domains of the   */
/*  meets of e.                                                              */
/*                                                                           */
/*****************************************************************************/

/* *** replaced by KheMonitorAttachCheck
static bool KheLayerTreePreferTimesMonitorMustHaveZeroCost(KHE_SOLN soln,
  KHE_EVENT e, KHE_PREFER_TIMES_MONITOR m)
{
  KHE_MEET meet;  KHE_TIME_GROUP tg;  int i;
  KHE_PREFER_TIMES_CONSTRAINT c;
  c = KhePreferTimesMonitorConstraint(m);
  tg = KhePreferTimesConstraintDomain(c);
  for( i = 0;  i < KheEventMeetCount(soln, e);  i++ )
  {
    meet = KheEventMeet(soln, e, i);
    if( KhePreferTimesConstraintDuration(c) == KHE_NO_DURATION ||
	KhePreferTimesConstraintDuration(c) == KheMeetDuration(meet) )
    {
      ** m is applicable to meet **
      if( !KheTimeGroupSubset(KheMeetCurrentDomain(meet), tg) )
	return false;
    }
  }
  return true;
}
*** */


/*****************************************************************************/
/*                                                                           */
/*  bool KheLayerTreePreferResourcesMonitorMustHaveZeroCost(KHE_SOLN soln,   */
/*    KHE_EVENT_RESOURCE er, KHE_PREFER_RESOURCES_MONITOR m)                 */
/*                                                                           */
/*  Return true if m must have cost 0, given the domains of the soln         */
/*  resources it is monitoring.                                              */
/*                                                                           */
/*****************************************************************************/

/* *** replace by KheMonitorAttachCheck (out of place in layer trees anyway)
static bool KheLayerTreePreferResourcesMonitorMustHaveZeroCost(KHE_SOLN soln,
  KHE_EVENT_RESOURCE er, KHE_PREFER_RESOURCES_MONITOR m)
{
  int i;  KHE_TASK task;  KHE_RESOURCE_GROUP rg;
  rg =
    KhePreferResourcesConstraintDomain(KhePreferResourcesMonitorConstraint(m));
  for( i = 0;  i < KheEventResourceTaskCount(soln, er);  i++ )
  {
    task = KheEventResourceTask(soln, er, i);
    if( !KheResourceGroupSubset(KheTaskDomain(task), rg) )
      return false;
  }
  return true;
}
*** */


/*****************************************************************************/
/*                                                                           */
/*  void KheLayerTreeCheckMonitors(KHE_SOLN soln,                            */
/*    bool check_prefer_times_monitors, bool check_split_events_monitors,    */
/*    bool check_link_events_monitors)                                       */
/*                                                                           */
/*  Detach monitors, if requested and appropriate.                           */
/*                                                                           */
/*****************************************************************************/

static void KheLayerTreeCheckMonitors(KHE_SOLN soln,
  bool check_prefer_times_monitors, bool check_split_events_monitors,
  bool check_link_events_monitors)
{
  KHE_INSTANCE ins;  KHE_EVENT e;  KHE_MONITOR m;  /* KHE_EVENT_RESOURCE er; */
  int i, j;
  ins = KheSolnInstance(soln);
  if( DEBUG3 )
    fprintf(stderr, "[ KheLayerTreeCheckMonitors(soln, %s, %s, %s):\n",
      check_prefer_times_monitors ? "true" : "false",
      check_split_events_monitors ? "true" : "false",
      check_link_events_monitors ? "true" : "false");
  for( i = 0;  i < KheInstanceEventCount(ins);  i++ )
  {
    /* check event monitors as requested */
    e = KheInstanceEvent(ins, i);
    for( j = 0;  j < KheEventMonitorCount(soln, e);  j++ )
    {
      m = KheEventMonitor(soln, e, j);
      if( KheMonitorAttachedToSoln(m) )
	switch( KheMonitorTag(m) )
	{
	  case KHE_PREFER_TIMES_MONITOR_TAG:

	    if( check_prefer_times_monitors )
	      KheMonitorAttachCheck(m);
	    /* ***
	    if( check_prefer_times_monitors &&
		KheLayerTreePreferTimesMonitorMustHaveZeroCost(soln, e,
		  (KHE_PREFER_TIMES_MONITOR) m) )
	    {
	      KheMonitorDetachFromSoln(m);
	      if( DEBUG3 )
		KheMonitorDebug(m, 2, 2, stderr);
	    }
	    *** */
	    break;


	  case KHE_SPLIT_EVENTS_MONITOR_TAG:
	  case KHE_DISTRIBUTE_SPLIT_EVENTS_MONITOR_TAG:

	    if( check_split_events_monitors )
	      KheMonitorAttachCheck(m);
	    /* ***
	    if( check_split_monitors && KheMonitorCost(m) == 0 )
	    {
	      KheMonitorDetachFromSoln(m);
	      if( DEBUG3 )
		KheMonitorDebug(m, 2, 2, stderr);
	    }
	    *** */
	    break;


	  case KHE_LINK_EVENTS_MONITOR_TAG:

	    if( check_link_events_monitors )
	      KheMonitorAttachCheck(m);
	    /* ***
	    if( check_link_monitors &&
		KheLayerTreeLinkEventsMonitorMustHaveZeroCost(soln, e,
		  (KHE_LINK_EVENTS_MONITOR) m) )
	    {
	      KheMonitorDetachFromSoln(m);
	      if( DEBUG3 )
		KheMonitorDebug(m, 2, 2, stderr);
	    }
	    *** */
	    break;

	  default:

	    /* ignore other monitors */
	    break;
	}
    }

    /* check event resource monitors as requested */
    /* ***
    if( check_prefer_resources_monitors )
    {
      for( j = 0;  j < KheEventResourceCount(e);  j++ )
      {
	er = KheEventResource(e, j);
	for( k = 0;  k < KheEventResourceMonitorCount(soln, er);  k++ )
	{
	  m = KheEventResourceMonitor(soln, er, k);
	  switch( KheMonitorTag(m) )
	  {
	    case KHE_PREFER_RESOURCES_MONITOR_TAG:

	      if( check_prefer_resources_monitors &&
		KheLayerTreePreferResourcesMonitorMustHaveZeroCost(soln, er,
		  (KHE_PREFER_RESOURCES_MONITOR) m) )
	      {
		KheMonitorDetachFromSoln(m);
		if( DEBUG3 )
		  KheMonitorDebug(m, 2, 2, stderr);
	      }
	      break;

	    default:

	      ** ignore other monitors **
	      break;
	  }
	}
      }
    }
    *** */
  }
  if( DEBUG3 )
    fprintf(stderr, "] KheLayerTreeCheckMonitors returning\n");
}


/*****************************************************************************/
/*                                                                           */
/*  KHE_NODE KheLayerTreeMake(KHE_SOLN soln,                                 */
/*    bool check_prefer_times_monitors, bool check_split_events_monitors,    */
/*    bool check_link_events_monitors)                                       */
/*                                                                           */
/*  Build a layer tree for soln.  Optionally check various kinds of          */
/*  monitors whose costs are always going to be 0 while this tree            */
/*  continues in use.                                                        */
/*                                                                           */
/*****************************************************************************/

KHE_NODE KheLayerTreeMake(KHE_SOLN soln,
  bool check_prefer_times_monitors, bool check_split_events_monitors,
  bool check_link_events_monitors)
{
  KHE_INSTANCE ins;  KHE_SPLIT_FOREST sf;  KHE_SPLIT_TASK st;
  int i, max_duration;  KHE_MEET meet, junk;  /* KHE_LAYER layer; */
  ARRAY_KHE_SPLIT_TASK split_tasks;  KHE_NODE res;
  struct timeval tv;

  ins = KheSolnInstance(soln);
  if( DEBUG1 )
  {
    fprintf(stderr, "[ KheLayerTreeMake(soln to %s)\n",
      KheInstanceId(ins) != NULL ? KheInstanceId(ins) : "-");
    KheTestTimeOfDayBegin(&tv);
  }

  /* check precondition: there may be no pre-existing nodes */
  MAssert(KheSolnNodeCount(soln) == 0, "KheLayerTreeMake: nodes already exist");

  /* ensure that all events are assigned to at most one distinct thing */
  for( i = 0;  i < KheInstanceEventCount(ins);  i++ )
    KheSplitEventHomogenizeAssignments(soln, KheInstanceEvent(ins, i));

  /* ensure that all meets have durations within the cycle size */
  max_duration = KheInstanceTimeCount(ins);
  for( i = 0;  i < KheSolnMeetCount(soln);  i++ )
  {
    meet = KheSolnMeet(soln, i);
    if( KheMeetDuration(meet) > max_duration )
      KheMeetSplit(meet, max_duration, false, &meet, &junk);
      /* junk goes on the end and gets examined later in the loop */
  }

  /* make and sort the split tasks */
  if( DEBUG1 )
    fprintf(stderr, "  before creating tasks, run time %.2fs\n",
      KheTestTimeOfDayEnd(&tv));
  MArrayInit(split_tasks);
  KheSplitAddConstraintTasks(soln, &split_tasks);
  KheSplitAddPreExistingSplitTasks(soln, &split_tasks);
  KheSplitAddPreassignedSplitTasks(soln, &split_tasks);
  /* KheSplitAddLayerTasks(soln, &split_tasks); */
  KheSplitAddAssignedSplitTasks(soln, &split_tasks);
  if( DEBUG2 )
  {
    fprintf(stderr, "[ split tasks before sorting (%d tasks):\n",
      MArraySize(split_tasks));
    MArrayForEach(split_tasks, &st, &i)
      KheSplitTaskDebug(st, 2, stderr);
    fprintf(stderr, "]\n");
  }
  if( DEBUG1 )
    fprintf(stderr, "  before sorting tasks, run time %.2fs\n",
      KheTestTimeOfDayEnd(&tv));
  MArraySort(split_tasks, &KheSplitTaskDecreasingPriorityCmp);
  if( DEBUG1 )
    fprintf(stderr, "  after sorting tasks, run time %.2fs\n",
      KheTestTimeOfDayEnd(&tv));
  if( DEBUG2 )
  {
    fprintf(stderr, "[ split tasks after sorting (%d tasks):\n",
      MArraySize(split_tasks));
    MArrayForEach(split_tasks, &st, &i)
      KheSplitTaskDebug(st, 2, stderr);
    fprintf(stderr, "]\n");
  }

  /* make a forest and try the tasks on it */
  sf = KheSplitForestMake(soln);
  MArrayForEach(split_tasks, &st, &i)
  {
    if( DEBUG1 )
    {
      fprintf(stderr, "  [ Task %d:\n", i);
      KheSplitTaskDebug(st, 4, stderr);
    }
    KheSplitTaskTry(st, sf);
    if( DEBUG1 )
      fprintf(stderr, "  ] after Task %d, run time %.2fs\n",
	i, KheTestTimeOfDayEnd(&tv));
  }
  /* ***
  if( DEBUG1 )
    KheSplitForestDebug(sf, 2, stderr);
  *** */

  /* choose partitions, split events into meets, and build tree */
  res = KheSplitForestFinalize(sf);
  if( DEBUG1 )
    fprintf(stderr, "  after finalizing, run time %.2fs\n",
      KheTestTimeOfDayEnd(&tv));

  /* free the split tasks, the array containing them, and the forest */
  MArrayForEach(split_tasks, &st, &i)
    KheSplitTaskFree(st);
  MArrayFree(split_tasks);
  KheSplitForestFree(sf);
  if( DEBUG1 )
    fprintf(stderr, "  after freeing, run time %.2fs\n",
      KheTestTimeOfDayEnd(&tv));

  /* remove redundant layers */
  /* ***
  for( i = 0;  i < KheSolnLayerCount(soln);  i++ )
  {
    layer = KheSolnLayer(soln, i);
    if( KheSplitLayerIsFullyAssigned(layer) )
    {
      KheLayerDelete(layer);
      i--;
    }
  }
  *** */
  /* ***
  for( i = 0;  i < KheSolnLayerCount(soln);  i++ )
  {
    layer = KheSolnLayer(soln, i);
    if( KheLayerIsRedundant(layer) )
    {
      KheLayerDelete(layer);
      i--;
    }
  }
  *** */

  /* ensure that every unassigned meet lies in at least one layer */
  /* ***
  for( i = 0;  i < KheMeetCount(soln);  i++ )
  {
    meet = KheMeet(soln, i);
    if( KheMeetAsst(meet) == NULL && KheMeetLayerCount(meet) == 0 )
    {
      layer = KheLayerMake(soln, NULL);
      if( !KheLayerAddMeet(layer, meet) )
	MAssert(false, "KheLayerTreeMake internal error");
    }
  }
  *** */

  /* check monitors as requested */
  if( check_prefer_times_monitors || check_split_events_monitors ||
      check_link_events_monitors )
    KheLayerTreeCheckMonitors(soln, check_prefer_times_monitors,
      check_split_events_monitors, check_link_events_monitors);

  if( DEBUG1 )
  {
    /* ***
    fprintf(stderr, "  final layers:\n");
    for( i = 0;  i < KheSolnLayerCount(soln);  i++ )
    {
      layer = KheSolnLayer(soln, i);
      KheLayerDebug(layer, 2, 2, stderr);
    }
    *** */
    fprintf(stderr, "  final tree:\n");
    KheNodeDebug(res, 2, 2, stderr);
    fprintf(stderr, "] KheLayerTreeMake (%d nodes, run time %.2fs)\n",
      KheSolnNodeCount(soln), KheTestTimeOfDayEnd(&tv));
  }
  return res;
}
