
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
/*  FILE:         khe_monitor.c                                              */
/*  DESCRIPTION:  A monitor (abstract supertype)                             */
/*                                                                           */
/*****************************************************************************/
#include "khe_interns.h"

#define DEBUG1 0
#define DEBUG2 0
#define DEBUG3 0


/*****************************************************************************/
/*                                                                           */
/*  KHE_MONITOR - monitors one point of application of one constraint        */
/*                                                                           */
/*  Parent monitors                                                          */
/*  ---------------                                                          */
/*                                                                           */
/*  In every monitor m except those of type KHE_SOLN (which is a subtype of  */
/*  KHE_GROUP_MONITOR), the m->parent_monitor is non-NULL and points to the  */
/*  parent monitor, and m->parent_index is the index of m in the parent      */
/*  monitor's list of child monitors.  In KHE_SOLN objects, parent_monitor   */
/*  is NULL and parent_index is -1.                                          */
/*                                                                           */
/*  Tracing                                                                  */
/*  -------                                                                  */
/*                                                                           */
/*  In every monitor m, m->trace_num is the visit number of the most recent  */
/*  trace during which the cost of the monitor changed, and m->trace_cost    */
/*  is the cost of m at the start of that trace.                             */
/*                                                                           */
/*****************************************************************************/

struct khe_monitor_rec {
  KHE_SOLN			soln;			/* encl. solution    */
  int				index_in_soln;		/* index in soln     */
  unsigned char			tag;			/* tag field         */
  bool				attached;		/* true if attached  */
  void				*back;			/* back pointer      */
  KHE_GROUP_MONITOR		parent_monitor;		/* parent monitor    */
  int				parent_index;		/* index in parent   */
  int				defect_index;		/* defect index      */
  int				trace_num;		/* trace visit num   */
  KHE_COST			trace_cost;		/* at start of trace */
  KHE_COST			cost;			/* current cost      */
};


/*****************************************************************************/
/*                                                                           */
/*  Submodule "construction and query"                                       */
/*                                                                           */
/*****************************************************************************/

/*****************************************************************************/
/*                                                                           */
/*  void KheMonitorCheck(KHE_MONITOR m)                                      */
/*                                                                           */
/*  Check and invariant of m.                                                */
/*                                                                           */
/*****************************************************************************/

/* ***
void KheMonitorCheck(KHE_MONITOR m)
{
  int i;  KHE_MONITOR defect;
  if( DEBUG2 )
  {
    MAssert(m->cost >= 0, "KheMonitorCheck: negative cost");
    if( m->parent_monitor != NULL )
      MAssert((m->defect_index == -1) == (m->cost == 0),
	"KheMonitorCheck: defect index problem 1: (prnt, index %d, cost %.4f)",
	m->defect_index, KheCostShow(m->cost));
    else
      MAssert(m->defect_index == -1,
	"KheMonitorCheck: defect index problem 2: (no prnt, index %d)",
	m->defect_index);
    ** can't call KheGroupMonitorDefectCount in this function! ***
    if( m->cost > 0 && m->parent_monitor != NULL )
    {
      for( i = 0;  i < KheGroupMonitorDefectCount(m->parent_monitor);  i++ )
      {
	defect = KheGroupMonitorDefect(m->parent_monitor, i);
	if( defect == m )
	{
	  MAssert(i == m->defect_index,
	    "KheMonitorCheck: defect index problem 2");
	  break;
	}
      }
      MAssert(i < KheGroupMonitorDefectCount(m->parent_monitor),
        "KheMonitorCheck: defect index problem 2");
    }
    *** **
  }
}
*** */


/*****************************************************************************/
/*                                                                           */
/*  void KheMonitorSetBack(KHE_MONITOR m, void *back)                        */
/*                                                                           */
/*  Set the back pointer of m.                                               */
/*                                                                           */
/*****************************************************************************/

void KheMonitorSetBack(KHE_MONITOR m, void *back)
{
  m->back = back;
}


/*****************************************************************************/
/*                                                                           */
/*  void *KheMonitorBack(KHE_MONITOR m)                                      */
/*                                                                           */
/*  Return the back pointer of m.                                            */
/*                                                                           */
/*****************************************************************************/

void *KheMonitorBack(KHE_MONITOR m)
{
  return m->back;
}


/*****************************************************************************/
/*                                                                           */
/*  KHE_SOLN KheMonitorSoln(KHE_MONITOR m)                                   */
/*                                                                           */
/*  Return the solution containing m.                                        */
/*                                                                           */
/*****************************************************************************/

KHE_SOLN KheMonitorSoln(KHE_MONITOR m)
{
  return m->soln;
}


/*****************************************************************************/
/*                                                                           */
/*  KHE_COST KheMonitorCost(KHE_MONITOR m)                                   */
/*                                                                           */
/*  Return the cost of m.                                                    */
/*                                                                           */
/*****************************************************************************/

KHE_COST KheMonitorCost(KHE_MONITOR m)
{
  if( m->tag == KHE_GROUP_MONITOR_TAG ||
      m->tag == KHE_ORDINARY_DEMAND_MONITOR_TAG ||
      m->tag == KHE_WORKLOAD_DEMAND_MONITOR_TAG )
    KheSolnMatchingUpdate(KheMonitorSoln(m));
  return m->cost;
}


/*****************************************************************************/
/*                                                                           */
/*  KHE_MONITOR_TAG KheMonitorTag(KHE_MONITOR m)                             */
/*                                                                           */
/*  Return the tag attribute of m.                                           */
/*                                                                           */
/*****************************************************************************/

KHE_MONITOR_TAG KheMonitorTag(KHE_MONITOR m)
{
  return m->tag;
}


/*****************************************************************************/
/*                                                                           */
/*  bool KheMonitorAttachedToSoln(KHE_MONITOR m)                             */
/*                                                                           */
/*  Return true if m is currently attached.                                  */
/*                                                                           */
/*****************************************************************************/

bool KheMonitorAttachedToSoln(KHE_MONITOR m)
{
  MAssert(m->tag != KHE_GROUP_MONITOR_TAG,
    "KheMonitorAttachedToSoln: m is a group monitor");
  return m->attached;
}


/*****************************************************************************/
/*                                                                           */
/*  void KheSolnEnsureOfficialCost(KHE_SOLN soln)                            */
/*                                                                           */
/*  Ensure that KheSolnCost(soln) is the official cost, by ensuring that     */
/*  all constraint monitors are both attached to the solution and reporting  */
/*  their cost to the solution, directly or indirectly, and that all         */
/*  non-constraint non-group monitors are detached from the solution.        */
/*                                                                           */
/*****************************************************************************/

void KheSolnEnsureOfficialCost(KHE_SOLN soln)
{
  int i;  KHE_MONITOR m;
  for( i = 0;  i < KheSolnMonitorCount(soln);  i++ )
  {
    m = KheSolnMonitor(soln, i);
    switch( m->tag )
    {
      case KHE_ASSIGN_RESOURCE_MONITOR_TAG:
      case KHE_ASSIGN_TIME_MONITOR_TAG:
      case KHE_SPLIT_EVENTS_MONITOR_TAG:
      case KHE_DISTRIBUTE_SPLIT_EVENTS_MONITOR_TAG:
      case KHE_PREFER_RESOURCES_MONITOR_TAG:
      case KHE_PREFER_TIMES_MONITOR_TAG:
      case KHE_AVOID_SPLIT_ASSIGNMENTS_MONITOR_TAG:
      case KHE_SPREAD_EVENTS_MONITOR_TAG:
      case KHE_LINK_EVENTS_MONITOR_TAG:
      case KHE_AVOID_CLASHES_MONITOR_TAG:
      case KHE_AVOID_UNAVAILABLE_TIMES_MONITOR_TAG:
      case KHE_LIMIT_IDLE_TIMES_MONITOR_TAG:
      case KHE_CLUSTER_BUSY_TIMES_MONITOR_TAG:
      case KHE_LIMIT_BUSY_TIMES_MONITOR_TAG:
      case KHE_LIMIT_WORKLOAD_MONITOR_TAG:

	/* ensure attached to soln */
	if( !KheMonitorAttachedToSoln(m) )
	  KheMonitorAttachToSoln(m);

	/* ensure linked to soln */
	while( KheMonitorParentMonitor(m) != NULL )
	  m = (KHE_MONITOR) KheMonitorParentMonitor(m);
	if( m != (KHE_MONITOR) soln )
	  KheGroupMonitorAddChildMonitor((KHE_GROUP_MONITOR) soln, m);
	break;

      case KHE_TIMETABLE_MONITOR_TAG:
      case KHE_TIME_GROUP_MONITOR_TAG:
      case KHE_GROUP_MONITOR_TAG:

	/* do nothing with these ones */
	break;

      case KHE_ORDINARY_DEMAND_MONITOR_TAG:
      case KHE_WORKLOAD_DEMAND_MONITOR_TAG:
      case KHE_EVENNESS_MONITOR_TAG:

	/* ensure detached from soln */
	if( KheMonitorAttachedToSoln(m) )
	  KheMonitorDetachFromSoln(m);
	break;

      default:

	MAssert(false, "KheSolnEnsureOfficialCost internal error");
	break;
    }
  }
}


/*****************************************************************************/
/*                                                                           */
/*  void KheMonitorChangeCost(KHE_MONITOR m, KHE_COST new_cost)              */
/*                                                                           */
/*  Change the cost of m from m->cost to new_cost, including updating        */
/*  all ancestors and handling tracing.  This function is called only by     */
/*  non-group monitors, and so m->parent_monitor must be non-NULL.           */
/*                                                                           */
/*****************************************************************************/

void KheMonitorChangeCost(KHE_MONITOR m, KHE_COST new_cost)
{
  //I COMMENTED THIS CODE!!!
  //MAssert(new_cost >= 0, "KheMonitorChangeCost internal error");
  if( new_cost != m->cost )
  {
    /* KheMonitorCheck(m); */
    if( m->parent_monitor != NULL )
      KheGroupMonitorChangeCost(m->parent_monitor, m, m->cost, new_cost);
    m->cost = new_cost;
    /* KheMonitorCheck(m); */
  }
}


/*****************************************************************************/
/*                                                                           */
/*  void KheMonitorInitCommonFields(KHE_MONITOR m, KHE_SOLN soln,            */
/*    KHE_MONITOR_TAG tag)                                                   */
/*                                                                           */
/*  Initialize the common fields of m.                                       */
/*                                                                           */
/*****************************************************************************/

void KheMonitorInitCommonFields(KHE_MONITOR m, KHE_SOLN soln,
  KHE_MONITOR_TAG tag)
{
  m->soln = soln;
  KheSolnAddMonitor(soln, m, &m->index_in_soln);
  m->tag = tag;
  m->attached = false;
  m->back = NULL;
  m->parent_monitor = NULL;
  m->parent_index = -1;
  m->defect_index = -1;
  m->trace_num = 0;
  m->trace_cost = 0;
  m->cost = 0;
}


/*****************************************************************************/
/*                                                                           */
/*  void KheMonitorCopyCommonFields(KHE_MONITOR copy, KHE_MONITOR orig)      */
/*                                                                           */
/*  Copy the common fields of orig to copy.                                  */
/*                                                                           */
/*****************************************************************************/

void KheMonitorCopyCommonFields(KHE_MONITOR copy, KHE_MONITOR orig)
{
  copy->soln = KheSolnCopyPhase1(orig->soln);
  copy->index_in_soln = orig->index_in_soln;
  copy->tag = orig->tag;
  copy->attached = orig->attached;
  copy->back = orig->back;
  copy->parent_monitor = orig->parent_monitor == NULL ? NULL :
    KheGroupMonitorCopyPhase1(orig->parent_monitor);  /* does soln correctly */
  copy->parent_index = orig->parent_index;
  copy->defect_index = orig->defect_index;
  copy->trace_num = orig->trace_num;
  copy->trace_cost = orig->trace_cost;
  copy->cost = orig->cost;
}


/*****************************************************************************/
/*                                                                           */
/*  int KheMonitorIndexInSoln(KHE_MONITOR m)                                 */
/*                                                                           */
/*  Return the index of m in soln.                                           */
/*                                                                           */
/*****************************************************************************/

int KheMonitorIndexInSoln(KHE_MONITOR m)
{
  return m->index_in_soln;
}


/*****************************************************************************/
/*                                                                           */
/*  void KheMonitorSetIndexInSoln(KHE_MONITOR m, int val)                    */
/*                                                                           */
/*  Set the index of m in soln to val.                                       */
/*                                                                           */
/*****************************************************************************/

void KheMonitorSetIndexInSoln(KHE_MONITOR m, int val)
{
  m->index_in_soln = val;
}


/*****************************************************************************/
/*                                                                           */
/*  KHE_CONSTRAINT KheMonitorConstraint(KHE_MONITOR m)                       */
/*                                                                           */
/*  Return the constraint that m monitors one point of application of,       */
/*  or NULL if none.                                                         */
/*                                                                           */
/*****************************************************************************/

KHE_CONSTRAINT KheMonitorConstraint(KHE_MONITOR m)
{
  switch( m->tag )
  {
    case KHE_ASSIGN_RESOURCE_MONITOR_TAG:

      return (KHE_CONSTRAINT) KheAssignResourceMonitorConstraint(
	(KHE_ASSIGN_RESOURCE_MONITOR) m);

    case KHE_ASSIGN_TIME_MONITOR_TAG:

      return (KHE_CONSTRAINT) KheAssignTimeMonitorConstraint(
	(KHE_ASSIGN_TIME_MONITOR) m);

    case KHE_SPLIT_EVENTS_MONITOR_TAG:

      return (KHE_CONSTRAINT) KheSplitEventsMonitorConstraint(
	(KHE_SPLIT_EVENTS_MONITOR) m);

    case KHE_DISTRIBUTE_SPLIT_EVENTS_MONITOR_TAG:

      return (KHE_CONSTRAINT) KheDistributeSplitEventsMonitorConstraint(
	(KHE_DISTRIBUTE_SPLIT_EVENTS_MONITOR) m);

    case KHE_PREFER_RESOURCES_MONITOR_TAG:

      return (KHE_CONSTRAINT) KhePreferResourcesMonitorConstraint(
	(KHE_PREFER_RESOURCES_MONITOR) m);

    case KHE_PREFER_TIMES_MONITOR_TAG:

      return (KHE_CONSTRAINT) KhePreferTimesMonitorConstraint(
	(KHE_PREFER_TIMES_MONITOR) m);

    case KHE_AVOID_SPLIT_ASSIGNMENTS_MONITOR_TAG:

      return (KHE_CONSTRAINT) KheAvoidSplitAssignmentsMonitorConstraint(
	(KHE_AVOID_SPLIT_ASSIGNMENTS_MONITOR) m);

    case KHE_SPREAD_EVENTS_MONITOR_TAG:

      return (KHE_CONSTRAINT) KheSpreadEventsMonitorConstraint(
	(KHE_SPREAD_EVENTS_MONITOR) m);

    case KHE_LINK_EVENTS_MONITOR_TAG:

      return (KHE_CONSTRAINT) KheLinkEventsMonitorConstraint(
	(KHE_LINK_EVENTS_MONITOR) m);

    case KHE_AVOID_CLASHES_MONITOR_TAG:

      return (KHE_CONSTRAINT) KheAvoidClashesMonitorConstraint(
	(KHE_AVOID_CLASHES_MONITOR) m);

    case KHE_AVOID_UNAVAILABLE_TIMES_MONITOR_TAG:

      return (KHE_CONSTRAINT) KheAvoidUnavailableTimesMonitorConstraint(
	(KHE_AVOID_UNAVAILABLE_TIMES_MONITOR) m);

    case KHE_LIMIT_IDLE_TIMES_MONITOR_TAG:

      return (KHE_CONSTRAINT) KheLimitIdleTimesMonitorConstraint(
	(KHE_LIMIT_IDLE_TIMES_MONITOR) m);

    case KHE_CLUSTER_BUSY_TIMES_MONITOR_TAG:

      return (KHE_CONSTRAINT) KheClusterBusyTimesMonitorConstraint(
	(KHE_CLUSTER_BUSY_TIMES_MONITOR) m);

    case KHE_LIMIT_BUSY_TIMES_MONITOR_TAG:

      return (KHE_CONSTRAINT) KheLimitBusyTimesMonitorConstraint(
	(KHE_LIMIT_BUSY_TIMES_MONITOR) m);

    case KHE_LIMIT_WORKLOAD_MONITOR_TAG:

      return (KHE_CONSTRAINT) KheLimitWorkloadMonitorConstraint(
	(KHE_LIMIT_WORKLOAD_MONITOR) m);

    case KHE_TIMETABLE_MONITOR_TAG:
    case KHE_TIME_GROUP_MONITOR_TAG:
    case KHE_GROUP_MONITOR_TAG:
    case KHE_ORDINARY_DEMAND_MONITOR_TAG:
    case KHE_WORKLOAD_DEMAND_MONITOR_TAG:
    case KHE_EVENNESS_MONITOR_TAG:

      /* these types do not monitor a constraint, so we return NULL */
      return NULL;

    default:

      MAssert(false, "KheMonitorConstraint: invalid monitor type tag");
      return NULL;  /* keep compiler happy */
  }
}


/*****************************************************************************/
/*                                                                           */
/*  char *KheMonitorAppliesToName(KHE_MONITOR m)                             */
/*                                                                           */
/*  Return the name of the point of application of m, or NULL if none.       */
/*                                                                           */
/*****************************************************************************/

char *KheMonitorAppliesToName(KHE_MONITOR m)
{
  KHE_EVENT_RESOURCE er;  KHE_EVENT e;  int egi;  KHE_EVENT_GROUP eg;
  KHE_AVOID_SPLIT_ASSIGNMENTS_CONSTRAINT asac;  KHE_RESOURCE r;
  switch( m->tag )
  {
    case KHE_ASSIGN_RESOURCE_MONITOR_TAG:

      er =KheAssignResourceMonitorEventResource((KHE_ASSIGN_RESOURCE_MONITOR)m);
      return KheEventName(KheEventResourceEvent(er));

    case KHE_ASSIGN_TIME_MONITOR_TAG:

      e = KheAssignTimeMonitorEvent((KHE_ASSIGN_TIME_MONITOR) m);
      return KheEventName(e);

    case KHE_SPLIT_EVENTS_MONITOR_TAG:

      e = KheSplitEventsMonitorEvent((KHE_SPLIT_EVENTS_MONITOR) m);
      return KheEventName(e);

    case KHE_DISTRIBUTE_SPLIT_EVENTS_MONITOR_TAG:

      e = KheDistributeSplitEventsMonitorEvent(
	(KHE_DISTRIBUTE_SPLIT_EVENTS_MONITOR) m);
      return KheEventName(e);

    case KHE_PREFER_RESOURCES_MONITOR_TAG:

      er = KhePreferResourcesMonitorEventResource(
	(KHE_PREFER_RESOURCES_MONITOR) m);
      return KheEventName(KheEventResourceEvent(er));

    case KHE_PREFER_TIMES_MONITOR_TAG:

      e = KhePreferTimesMonitorEvent((KHE_PREFER_TIMES_MONITOR) m);
      return KheEventName(e);

    case KHE_AVOID_SPLIT_ASSIGNMENTS_MONITOR_TAG:

      egi = KheAvoidSplitAssignmentsMonitorEventGroupIndex(
	(KHE_AVOID_SPLIT_ASSIGNMENTS_MONITOR) m);
      asac = KheAvoidSplitAssignmentsMonitorConstraint(
	(KHE_AVOID_SPLIT_ASSIGNMENTS_MONITOR) m);
      eg = KheAvoidSplitAssignmentsConstraintEventGroup(asac, egi);
      return KheEventGroupName(eg);

    case KHE_SPREAD_EVENTS_MONITOR_TAG:

      eg = KheSpreadEventsMonitorEventGroup((KHE_SPREAD_EVENTS_MONITOR) m);
      return KheEventGroupName(eg);

    case KHE_LINK_EVENTS_MONITOR_TAG:

      eg = KheLinkEventsMonitorEventGroup((KHE_LINK_EVENTS_MONITOR) m);
      return KheEventGroupName(eg);

    case KHE_AVOID_CLASHES_MONITOR_TAG:

      r = KheAvoidClashesMonitorResource((KHE_AVOID_CLASHES_MONITOR) m);
      return KheResourceName(r);

    case KHE_AVOID_UNAVAILABLE_TIMES_MONITOR_TAG:

      r = KheAvoidUnavailableTimesMonitorResource(
	(KHE_AVOID_UNAVAILABLE_TIMES_MONITOR) m);
      return KheResourceName(r);

    case KHE_LIMIT_IDLE_TIMES_MONITOR_TAG:

      r = KheLimitIdleTimesMonitorResource((KHE_LIMIT_IDLE_TIMES_MONITOR) m);
      return KheResourceName(r);

    case KHE_CLUSTER_BUSY_TIMES_MONITOR_TAG:

      r = KheClusterBusyTimesMonitorResource((KHE_CLUSTER_BUSY_TIMES_MONITOR)m);
      return KheResourceName(r);

    case KHE_LIMIT_BUSY_TIMES_MONITOR_TAG:

      r = KheLimitBusyTimesMonitorResource((KHE_LIMIT_BUSY_TIMES_MONITOR) m);
      return KheResourceName(r);

    case KHE_LIMIT_WORKLOAD_MONITOR_TAG:

      r = KheLimitWorkloadMonitorResource((KHE_LIMIT_WORKLOAD_MONITOR) m);
      return KheResourceName(r);

    case KHE_TIMETABLE_MONITOR_TAG:
    case KHE_TIME_GROUP_MONITOR_TAG:
    case KHE_GROUP_MONITOR_TAG:
    case KHE_ORDINARY_DEMAND_MONITOR_TAG:
    case KHE_WORKLOAD_DEMAND_MONITOR_TAG:
    case KHE_EVENNESS_MONITOR_TAG:

      /* these types do not monitor a constraint, so we return NULL */
      return NULL;

    default:

      MAssert(false, "KheMonitorConstraint: invalid monitor type tag");
      return NULL;  /* keep compiler happy */
  }
}


/*****************************************************************************/
/*                                                                           */
/*  KHE_MONITOR KheMonitorCopyPhase1(KHE_MONITOR m)                          */
/*                                                                           */
/*  Carry out Phase 1 of copying m.                                          */
/*                                                                           */
/*****************************************************************************/

KHE_MONITOR KheMonitorCopyPhase1(KHE_MONITOR m)
{
  switch( m->tag )
  {
    case KHE_ASSIGN_RESOURCE_MONITOR_TAG:

      return (KHE_MONITOR) KheAssignResourceMonitorCopyPhase1(
	(KHE_ASSIGN_RESOURCE_MONITOR) m);

    case KHE_ASSIGN_TIME_MONITOR_TAG:

      return (KHE_MONITOR) KheAssignTimeMonitorCopyPhase1(
	(KHE_ASSIGN_TIME_MONITOR) m);

    case KHE_SPLIT_EVENTS_MONITOR_TAG:

      return (KHE_MONITOR) KheSplitEventsMonitorCopyPhase1(
	(KHE_SPLIT_EVENTS_MONITOR) m);

    case KHE_DISTRIBUTE_SPLIT_EVENTS_MONITOR_TAG:

      return (KHE_MONITOR) KheDistributeSplitEventsMonitorCopyPhase1(
	(KHE_DISTRIBUTE_SPLIT_EVENTS_MONITOR) m);

    case KHE_PREFER_RESOURCES_MONITOR_TAG:

      return (KHE_MONITOR) KhePreferResourcesMonitorCopyPhase1(
	(KHE_PREFER_RESOURCES_MONITOR) m);

    case KHE_PREFER_TIMES_MONITOR_TAG:

      return (KHE_MONITOR) KhePreferTimesMonitorCopyPhase1(
	(KHE_PREFER_TIMES_MONITOR) m);

    case KHE_AVOID_SPLIT_ASSIGNMENTS_MONITOR_TAG:

      return (KHE_MONITOR) KheAvoidSplitAssignmentsMonitorCopyPhase1(
	(KHE_AVOID_SPLIT_ASSIGNMENTS_MONITOR) m);

    case KHE_SPREAD_EVENTS_MONITOR_TAG:

      return (KHE_MONITOR) KheSpreadEventsMonitorCopyPhase1(
	(KHE_SPREAD_EVENTS_MONITOR) m);

    case KHE_LINK_EVENTS_MONITOR_TAG:

      return (KHE_MONITOR) KheLinkEventsMonitorCopyPhase1(
	(KHE_LINK_EVENTS_MONITOR) m);

    case KHE_AVOID_CLASHES_MONITOR_TAG:

      return (KHE_MONITOR) KheAvoidClashesMonitorCopyPhase1(
	(KHE_AVOID_CLASHES_MONITOR) m);

    case KHE_AVOID_UNAVAILABLE_TIMES_MONITOR_TAG:

      return (KHE_MONITOR) KheAvoidUnavailableTimesMonitorCopyPhase1(
	(KHE_AVOID_UNAVAILABLE_TIMES_MONITOR) m);

    case KHE_LIMIT_IDLE_TIMES_MONITOR_TAG:

      return (KHE_MONITOR) KheLimitIdleTimesMonitorCopyPhase1(
	(KHE_LIMIT_IDLE_TIMES_MONITOR) m);

    case KHE_CLUSTER_BUSY_TIMES_MONITOR_TAG:

      return (KHE_MONITOR) KheClusterBusyTimesMonitorCopyPhase1(
	(KHE_CLUSTER_BUSY_TIMES_MONITOR) m);

    case KHE_LIMIT_BUSY_TIMES_MONITOR_TAG:

      return (KHE_MONITOR) KheLimitBusyTimesMonitorCopyPhase1(
	(KHE_LIMIT_BUSY_TIMES_MONITOR) m);

    case KHE_LIMIT_WORKLOAD_MONITOR_TAG:

      return (KHE_MONITOR) KheLimitWorkloadMonitorCopyPhase1(
	(KHE_LIMIT_WORKLOAD_MONITOR) m);

    case KHE_TIMETABLE_MONITOR_TAG:

      return (KHE_MONITOR) KheTimetableMonitorCopyPhase1(
	(KHE_TIMETABLE_MONITOR) m);

    case KHE_TIME_GROUP_MONITOR_TAG:

      return (KHE_MONITOR) KheTimeGroupMonitorCopyPhase1(
	(KHE_TIME_GROUP_MONITOR) m);

    case KHE_GROUP_MONITOR_TAG:

      return (KHE_MONITOR) KheGroupMonitorCopyPhase1(
	(KHE_GROUP_MONITOR) m);

    case KHE_ORDINARY_DEMAND_MONITOR_TAG:

      return (KHE_MONITOR) KheOrdinaryDemandMonitorCopyPhase1(
	(KHE_ORDINARY_DEMAND_MONITOR) m);

    case KHE_WORKLOAD_DEMAND_MONITOR_TAG:

      return (KHE_MONITOR) KheWorkloadDemandMonitorCopyPhase1(
	(KHE_WORKLOAD_DEMAND_MONITOR) m);

    case KHE_EVENNESS_MONITOR_TAG:

      return (KHE_MONITOR) KheEvennessMonitorCopyPhase1(
	(KHE_EVENNESS_MONITOR) m);

    default:

      MAssert(false, "KheMonitorCopyPhase1: invalid monitor type tag");
      return NULL;  /* keep compiler happy */
  }
}


/*****************************************************************************/
/*                                                                           */
/*  void KheMonitorCopyPhase2(KHE_MONITOR m)                                 */
/*                                                                           */
/*  Carry out Phase 2 of copying m.                                          */
/*                                                                           */
/*****************************************************************************/

void KheMonitorCopyPhase2(KHE_MONITOR m)
{
  switch( m->tag )
  {
    case KHE_ASSIGN_RESOURCE_MONITOR_TAG:

      KheAssignResourceMonitorCopyPhase2((KHE_ASSIGN_RESOURCE_MONITOR) m);
      break;

    case KHE_ASSIGN_TIME_MONITOR_TAG:

      KheAssignTimeMonitorCopyPhase2((KHE_ASSIGN_TIME_MONITOR) m);
      break;

    case KHE_SPLIT_EVENTS_MONITOR_TAG:

      KheSplitEventsMonitorCopyPhase2((KHE_SPLIT_EVENTS_MONITOR) m);
      break;

    case KHE_DISTRIBUTE_SPLIT_EVENTS_MONITOR_TAG:

      KheDistributeSplitEventsMonitorCopyPhase2(
	(KHE_DISTRIBUTE_SPLIT_EVENTS_MONITOR) m);
      break;

    case KHE_PREFER_RESOURCES_MONITOR_TAG:

      KhePreferResourcesMonitorCopyPhase2((KHE_PREFER_RESOURCES_MONITOR) m);
      break;

    case KHE_PREFER_TIMES_MONITOR_TAG:

      KhePreferTimesMonitorCopyPhase2((KHE_PREFER_TIMES_MONITOR) m);
      break;

    case KHE_AVOID_SPLIT_ASSIGNMENTS_MONITOR_TAG:

      KheAvoidSplitAssignmentsMonitorCopyPhase2(
	(KHE_AVOID_SPLIT_ASSIGNMENTS_MONITOR) m);
      break;

    case KHE_SPREAD_EVENTS_MONITOR_TAG:

      KheSpreadEventsMonitorCopyPhase2((KHE_SPREAD_EVENTS_MONITOR) m);
      break;

    case KHE_LINK_EVENTS_MONITOR_TAG:

      KheLinkEventsMonitorCopyPhase2((KHE_LINK_EVENTS_MONITOR) m);
      break;

    case KHE_AVOID_CLASHES_MONITOR_TAG:

      KheAvoidClashesMonitorCopyPhase2((KHE_AVOID_CLASHES_MONITOR) m);
      break;

    case KHE_AVOID_UNAVAILABLE_TIMES_MONITOR_TAG:

      KheAvoidUnavailableTimesMonitorCopyPhase2(
	(KHE_AVOID_UNAVAILABLE_TIMES_MONITOR) m);
      break;

    case KHE_LIMIT_IDLE_TIMES_MONITOR_TAG:

      KheLimitIdleTimesMonitorCopyPhase2((KHE_LIMIT_IDLE_TIMES_MONITOR) m);
      break;

    case KHE_CLUSTER_BUSY_TIMES_MONITOR_TAG:

      KheClusterBusyTimesMonitorCopyPhase2((KHE_CLUSTER_BUSY_TIMES_MONITOR) m);
      break;

    case KHE_LIMIT_BUSY_TIMES_MONITOR_TAG:

      KheLimitBusyTimesMonitorCopyPhase2((KHE_LIMIT_BUSY_TIMES_MONITOR) m);
      break;

    case KHE_LIMIT_WORKLOAD_MONITOR_TAG:

      KheLimitWorkloadMonitorCopyPhase2((KHE_LIMIT_WORKLOAD_MONITOR) m);
      break;

    case KHE_TIMETABLE_MONITOR_TAG:

      KheTimetableMonitorCopyPhase2((KHE_TIMETABLE_MONITOR) m);
      break;

    case KHE_TIME_GROUP_MONITOR_TAG:

      KheTimeGroupMonitorCopyPhase2((KHE_TIME_GROUP_MONITOR) m);
      break;

    case KHE_GROUP_MONITOR_TAG:

      KheGroupMonitorCopyPhase2((KHE_GROUP_MONITOR) m);
      break;

    case KHE_ORDINARY_DEMAND_MONITOR_TAG:

      KheOrdinaryDemandMonitorCopyPhase2((KHE_ORDINARY_DEMAND_MONITOR) m);
      break;

    case KHE_WORKLOAD_DEMAND_MONITOR_TAG:

      KheWorkloadDemandMonitorCopyPhase2((KHE_WORKLOAD_DEMAND_MONITOR) m);
      break;

    case KHE_EVENNESS_MONITOR_TAG:

      KheEvennessMonitorCopyPhase2((KHE_EVENNESS_MONITOR) m);
      break;

    default:

      MAssert(false, "KheMonitorCopyPhase2: invalid monitor type tag");
  }
}


/*****************************************************************************/
/*                                                                           */
/*  void KheMonitorDelete(KHE_MONITOR m)                                     */
/*                                                                           */
/*  Delete m.                                                                */
/*                                                                           */
/*****************************************************************************/

void KheMonitorDelete(KHE_MONITOR m)
{
  switch( m->tag )
  {
    case KHE_ASSIGN_RESOURCE_MONITOR_TAG:

      KheAssignResourceMonitorDelete((KHE_ASSIGN_RESOURCE_MONITOR) m);
      break;

    case KHE_ASSIGN_TIME_MONITOR_TAG:

      KheAssignTimeMonitorDelete((KHE_ASSIGN_TIME_MONITOR) m);
      break;

    case KHE_SPLIT_EVENTS_MONITOR_TAG:

      KheSplitEventsMonitorDelete((KHE_SPLIT_EVENTS_MONITOR) m);
      break;

    case KHE_DISTRIBUTE_SPLIT_EVENTS_MONITOR_TAG:

      KheDistributeSplitEventsMonitorDelete(
	(KHE_DISTRIBUTE_SPLIT_EVENTS_MONITOR) m);
      break;

    case KHE_PREFER_RESOURCES_MONITOR_TAG:

      KhePreferResourcesMonitorDelete((KHE_PREFER_RESOURCES_MONITOR) m);
      break;

    case KHE_PREFER_TIMES_MONITOR_TAG:

      KhePreferTimesMonitorDelete((KHE_PREFER_TIMES_MONITOR) m);
      break;

    case KHE_AVOID_SPLIT_ASSIGNMENTS_MONITOR_TAG:

      KheAvoidSplitAssignmentsMonitorDelete(
	(KHE_AVOID_SPLIT_ASSIGNMENTS_MONITOR) m);
      break;

    case KHE_SPREAD_EVENTS_MONITOR_TAG:

      KheSpreadEventsMonitorDelete((KHE_SPREAD_EVENTS_MONITOR) m);
      break;

    case KHE_LINK_EVENTS_MONITOR_TAG:

      KheLinkEventsMonitorDelete((KHE_LINK_EVENTS_MONITOR) m);
      break;

    case KHE_AVOID_CLASHES_MONITOR_TAG:

      KheAvoidClashesMonitorDelete((KHE_AVOID_CLASHES_MONITOR) m);
      break;

    case KHE_AVOID_UNAVAILABLE_TIMES_MONITOR_TAG:

      KheAvoidUnavailableTimesMonitorDelete(
	(KHE_AVOID_UNAVAILABLE_TIMES_MONITOR) m);
      break;

    case KHE_LIMIT_IDLE_TIMES_MONITOR_TAG:

      KheLimitIdleTimesMonitorDelete((KHE_LIMIT_IDLE_TIMES_MONITOR) m);
      break;

    case KHE_CLUSTER_BUSY_TIMES_MONITOR_TAG:

      KheClusterBusyTimesMonitorDelete((KHE_CLUSTER_BUSY_TIMES_MONITOR) m);
      break;

    case KHE_LIMIT_BUSY_TIMES_MONITOR_TAG:

      KheLimitBusyTimesMonitorDelete((KHE_LIMIT_BUSY_TIMES_MONITOR) m);
      break;

    case KHE_LIMIT_WORKLOAD_MONITOR_TAG:

      KheLimitWorkloadMonitorDelete((KHE_LIMIT_WORKLOAD_MONITOR) m);
      break;

    case KHE_TIMETABLE_MONITOR_TAG:

      KheTimetableMonitorDelete((KHE_TIMETABLE_MONITOR) m);
      break;

    case KHE_TIME_GROUP_MONITOR_TAG:

      KheTimeGroupMonitorDelete((KHE_TIME_GROUP_MONITOR) m);
      break;

    case KHE_GROUP_MONITOR_TAG:

      KheGroupMonitorDelete((KHE_GROUP_MONITOR) m);
      break;

    case KHE_ORDINARY_DEMAND_MONITOR_TAG:

      KheOrdinaryDemandMonitorDelete((KHE_ORDINARY_DEMAND_MONITOR) m);
      break;

    case KHE_WORKLOAD_DEMAND_MONITOR_TAG:

      KheWorkloadDemandMonitorDelete((KHE_WORKLOAD_DEMAND_MONITOR) m);
      break;

    case KHE_EVENNESS_MONITOR_TAG:

      KheEvennessMonitorDelete((KHE_EVENNESS_MONITOR) m);
      break;

    default:

      MAssert(false, "KheMonitorDelete: invalid monitor type tag");
  }
}


/*****************************************************************************/
/*                                                                           */
/*  Submodule "parent monitors"                                              */
/*                                                                           */
/*****************************************************************************/

/*****************************************************************************/
/*                                                                           */
/*  KHE_GROUP_MONITOR KheMonitorParentMonitor(KHE_MONITOR m)                 */
/*                                                                           */
/*  Return the parent monitor of m, or NULL if none.                         */
/*                                                                           */
/*****************************************************************************/

KHE_GROUP_MONITOR KheMonitorParentMonitor(KHE_MONITOR m)
{
  return m->parent_monitor;
}


/*****************************************************************************/
/*                                                                           */
/*  bool KheMonitorDescendant(KHE_MONITOR m1, KHE_MONITOR m2)                */
/*                                                                           */
/*  Return true if m1 is a descendant of m2.                                 */
/*                                                                           */
/*****************************************************************************/

bool KheMonitorDescendant(KHE_MONITOR m1, KHE_MONITOR m2)
{
  KHE_GROUP_MONITOR gm1, gm2;
  if( m1 == m2 )
    return true;
  else if( KheMonitorTag(m2) != KHE_GROUP_MONITOR_TAG )
    return false;
  else
  {
    gm2 = (KHE_GROUP_MONITOR) m2;
    gm1 = m1->parent_monitor;
    while( gm1 != NULL )
    {
      if( gm1 == gm2 )
	return true;
      gm1 = KheMonitorParentMonitor((KHE_MONITOR) gm1);
    }
    return false;
  }
}


/*****************************************************************************/
/*                                                                           */
/*  int KheMonitorParentIndex(KHE_MONITOR m)                                 */
/*                                                                           */
/*  Return the parent_index attribute of m.                                  */
/*                                                                           */
/*****************************************************************************/

int KheMonitorParentIndex(KHE_MONITOR m)
{
  return m->parent_index;
}


/*****************************************************************************/
/*                                                                           */
/*  void KheMonitorSetParentMonitorAndIndex(KHE_MONITOR m,                   */
/*    KHE_GROUP_MONITOR parent_monitor, int parent_index)                    */
/*                                                                           */
/*  Set the parent_monitor and parent_index attributes of m.                 */
/*                                                                           */
/*****************************************************************************/

void KheMonitorSetParentMonitorAndIndex(KHE_MONITOR m,
  KHE_GROUP_MONITOR parent_monitor, int parent_index)
{
  m->parent_monitor = parent_monitor;
  m->parent_index = parent_index;
}


/*****************************************************************************/
/*                                                                           */
/*  int KheMonitorDefectIndex(KHE_MONITOR m)                                 */
/*                                                                           */
/*  Return m's index in its parent monitor's list of defects.                */
/*                                                                           */
/*****************************************************************************/

int KheMonitorDefectIndex(KHE_MONITOR m)
{
  return m->defect_index;
}


/*****************************************************************************/
/*                                                                           */
/*  void KheMonitorSetDefectIndex(KHE_MONITOR m, int index)                  */
/*                                                                           */
/*  Set m's defect index.                                                    */
/*                                                                           */
/*****************************************************************************/

void KheMonitorSetDefectIndex(KHE_MONITOR m, int index)
{
  m->defect_index = index;
}


/*****************************************************************************/
/*                                                                           */
/*  Submodule "attach and detach"                                            */
/*                                                                           */
/*****************************************************************************/

/*****************************************************************************/
/*                                                                           */
/*  void KheMonitorAttachToSoln(KHE_MONITOR m)                               */
/*                                                                           */
/*  Attach m to soln.  Make sure first that it is detached.                  */
/*                                                                           */
/*****************************************************************************/

void KheMonitorAttachToSoln(KHE_MONITOR m)
{
  if( DEBUG1 )
    fprintf(stderr, "[ KheMonitorAttachToSoln(m %p %s) attached %d\n",
      (void *) m, KheMonitorTagShow(m->tag), (int) m->attached);
  MAssert(!m->attached, "KheMonitorAttachToSoln: monitor is already attached");
  MAssert(m->cost == 0, "KheMonitorAttachToSoln internal error");
  /* ***
  if( m->parent_monitor != NULL )
  {
    if( DEBUG1 )
    {
      fprintf(stderr, "  prnt %p\n", (void *) m->parent_monitor);
      fprintf(stderr, "  prnt %s attached %d\n",
	KheMonitorTagShow(KheMonitorTag((KHE_MONITOR) m->parent_monitor)),
	(int) ((KHE_MONITOR) m->parent_monitor)->attached);
    }
    if( !KheMonitorAttached((KHE_MONITOR) m->parent_monitor) )
      KheMonitorAttach((KHE_MONITOR) m->parent_monitor);
  }
  *** */
  switch( m->tag )
  {
    case KHE_ASSIGN_RESOURCE_MONITOR_TAG:

      KheAssignResourceMonitorAttachToSoln((KHE_ASSIGN_RESOURCE_MONITOR) m);
      break;

    case KHE_ASSIGN_TIME_MONITOR_TAG:

      KheAssignTimeMonitorAttachToSoln((KHE_ASSIGN_TIME_MONITOR) m);
      break;

    case KHE_SPLIT_EVENTS_MONITOR_TAG:

      KheSplitEventsMonitorAttachToSoln((KHE_SPLIT_EVENTS_MONITOR) m);
      break;

    case KHE_DISTRIBUTE_SPLIT_EVENTS_MONITOR_TAG:

      KheDistributeSplitEventsMonitorAttachToSoln(
	(KHE_DISTRIBUTE_SPLIT_EVENTS_MONITOR) m);
      break;

    case KHE_PREFER_RESOURCES_MONITOR_TAG:

      KhePreferResourcesMonitorAttachToSoln((KHE_PREFER_RESOURCES_MONITOR) m);
      break;

    case KHE_PREFER_TIMES_MONITOR_TAG:

      KhePreferTimesMonitorAttachToSoln((KHE_PREFER_TIMES_MONITOR) m);
      break;

    case KHE_AVOID_SPLIT_ASSIGNMENTS_MONITOR_TAG:

      KheAvoidSplitAssignmentsMonitorAttachToSoln(
	(KHE_AVOID_SPLIT_ASSIGNMENTS_MONITOR) m);
      break;

    case KHE_SPREAD_EVENTS_MONITOR_TAG:

      KheSpreadEventsMonitorAttachToSoln((KHE_SPREAD_EVENTS_MONITOR) m);
      break;

    case KHE_LINK_EVENTS_MONITOR_TAG:

      KheLinkEventsMonitorAttachToSoln((KHE_LINK_EVENTS_MONITOR) m);
      break;

    case KHE_AVOID_CLASHES_MONITOR_TAG:

      KheAvoidClashesMonitorAttachToSoln((KHE_AVOID_CLASHES_MONITOR) m);
      break;

    case KHE_AVOID_UNAVAILABLE_TIMES_MONITOR_TAG:

      KheAvoidUnavailableTimesMonitorAttachToSoln(
	(KHE_AVOID_UNAVAILABLE_TIMES_MONITOR) m);
      break;

    case KHE_LIMIT_IDLE_TIMES_MONITOR_TAG:

      KheLimitIdleTimesMonitorAttachToSoln((KHE_LIMIT_IDLE_TIMES_MONITOR) m);
      break;

    case KHE_CLUSTER_BUSY_TIMES_MONITOR_TAG:

      KheClusterBusyTimesMonitorAttachToSoln((KHE_CLUSTER_BUSY_TIMES_MONITOR) m);
      break;

    case KHE_LIMIT_BUSY_TIMES_MONITOR_TAG:

      KheLimitBusyTimesMonitorAttachToSoln((KHE_LIMIT_BUSY_TIMES_MONITOR) m);
      break;

    case KHE_LIMIT_WORKLOAD_MONITOR_TAG:

      KheLimitWorkloadMonitorAttachToSoln((KHE_LIMIT_WORKLOAD_MONITOR) m);
      break;

    case KHE_TIMETABLE_MONITOR_TAG:

      KheTimetableMonitorAttachToSoln((KHE_TIMETABLE_MONITOR) m);
      break;

    case KHE_TIME_GROUP_MONITOR_TAG:

      KheTimeGroupMonitorAttachToSoln((KHE_TIME_GROUP_MONITOR) m);
      break;

    case KHE_GROUP_MONITOR_TAG:

      MAssert(false, "KheMonitorAttachToSoln: m is a group monitor");
      break;

    case KHE_ORDINARY_DEMAND_MONITOR_TAG:

      KheOrdinaryDemandMonitorAttachToSoln((KHE_ORDINARY_DEMAND_MONITOR) m);
      break;

    case KHE_WORKLOAD_DEMAND_MONITOR_TAG:

      KheWorkloadDemandMonitorAttachToSoln((KHE_WORKLOAD_DEMAND_MONITOR) m);
      break;

    case KHE_EVENNESS_MONITOR_TAG:

      KheEvennessMonitorAttachToSoln((KHE_EVENNESS_MONITOR) m);
      break;

    default:

      MAssert(false, "KheMonitorAttachToSoln: invalid monitor type tag");
  }
  if( DEBUG1 )
    fprintf(stderr, "] KheMonitorAttachToSoln(m %p) attached %d returning\n",
      (void *) m, (int) m->attached);
}


/*****************************************************************************/
/*                                                                           */
/*  void KheMonitorDetachFromSoln(KHE_MONITOR m)                             */
/*                                                                           */
/*  Detach m.  Check first that it is currently attached.                    */
/*                                                                           */
/*****************************************************************************/

void KheMonitorDetachFromSoln(KHE_MONITOR m)
{
  MAssert(m->attached, "KheMonitorDetachFromSoln: monitor is already detached");
  switch( m->tag )
  {
    case KHE_ASSIGN_RESOURCE_MONITOR_TAG:

      KheAssignResourceMonitorDetachFromSoln((KHE_ASSIGN_RESOURCE_MONITOR) m);
      break;

    case KHE_ASSIGN_TIME_MONITOR_TAG:

      KheAssignTimeMonitorDetachFromSoln((KHE_ASSIGN_TIME_MONITOR) m);
      break;

    case KHE_SPLIT_EVENTS_MONITOR_TAG:

      KheSplitEventsMonitorDetachFromSoln((KHE_SPLIT_EVENTS_MONITOR) m);
      break;

    case KHE_DISTRIBUTE_SPLIT_EVENTS_MONITOR_TAG:

      KheDistributeSplitEventsMonitorDetachFromSoln(
	(KHE_DISTRIBUTE_SPLIT_EVENTS_MONITOR) m);
      break;

    case KHE_PREFER_RESOURCES_MONITOR_TAG:

      KhePreferResourcesMonitorDetachFromSoln((KHE_PREFER_RESOURCES_MONITOR) m);
      break;

    case KHE_PREFER_TIMES_MONITOR_TAG:

      KhePreferTimesMonitorDetachFromSoln((KHE_PREFER_TIMES_MONITOR) m);
      break;

    case KHE_AVOID_SPLIT_ASSIGNMENTS_MONITOR_TAG:

      KheAvoidSplitAssignmentsMonitorDetachFromSoln(
	(KHE_AVOID_SPLIT_ASSIGNMENTS_MONITOR) m);
      break;

    case KHE_SPREAD_EVENTS_MONITOR_TAG:

      KheSpreadEventsMonitorDetachFromSoln((KHE_SPREAD_EVENTS_MONITOR) m);
      break;

    case KHE_LINK_EVENTS_MONITOR_TAG:

      KheLinkEventsMonitorDetachFromSoln((KHE_LINK_EVENTS_MONITOR) m);
      break;

    case KHE_AVOID_CLASHES_MONITOR_TAG:

      KheAvoidClashesMonitorDetachFromSoln((KHE_AVOID_CLASHES_MONITOR) m);
      break;

    case KHE_AVOID_UNAVAILABLE_TIMES_MONITOR_TAG:

      KheAvoidUnavailableTimesMonitorDetachFromSoln(
	(KHE_AVOID_UNAVAILABLE_TIMES_MONITOR) m);
      break;

    case KHE_LIMIT_IDLE_TIMES_MONITOR_TAG:

      KheLimitIdleTimesMonitorDetachFromSoln((KHE_LIMIT_IDLE_TIMES_MONITOR) m);
      break;

    case KHE_CLUSTER_BUSY_TIMES_MONITOR_TAG:

      KheClusterBusyTimesMonitorDetachFromSoln(
	(KHE_CLUSTER_BUSY_TIMES_MONITOR) m);
      break;

    case KHE_LIMIT_BUSY_TIMES_MONITOR_TAG:

      KheLimitBusyTimesMonitorDetachFromSoln((KHE_LIMIT_BUSY_TIMES_MONITOR) m);
      break;

    case KHE_LIMIT_WORKLOAD_MONITOR_TAG:

      KheLimitWorkloadMonitorDetachFromSoln((KHE_LIMIT_WORKLOAD_MONITOR) m);
      break;

    case KHE_TIMETABLE_MONITOR_TAG:

      KheTimetableMonitorDetachFromSoln((KHE_TIMETABLE_MONITOR) m);
      break;

    case KHE_TIME_GROUP_MONITOR_TAG:

      KheTimeGroupMonitorDetachFromSoln((KHE_TIME_GROUP_MONITOR) m);
      break;

    case KHE_GROUP_MONITOR_TAG:

      MAssert(false, "KheMonitorDetachFromSoln: m is a group monitor");
      break;

    case KHE_ORDINARY_DEMAND_MONITOR_TAG:

      KheOrdinaryDemandMonitorDetachFromSoln((KHE_ORDINARY_DEMAND_MONITOR) m);
      break;

    case KHE_WORKLOAD_DEMAND_MONITOR_TAG:

      KheWorkloadDemandMonitorDetachFromSoln((KHE_WORKLOAD_DEMAND_MONITOR) m);
      break;

    case KHE_EVENNESS_MONITOR_TAG:

      KheEvennessMonitorDetachFromSoln((KHE_EVENNESS_MONITOR) m);
      break;

    default:

      MAssert(false, "KheMonitorDetachFromSoln: invalid monitor type tag");
  }
  MAssert(m->cost == 0, "KheMonitorDetachFromSoln internal error (final cost)");
}


/*****************************************************************************/
/*                                                                           */
/*  void KheMonitorAttachCheck(KHE_MONITOR m)                                */
/*                                                                           */
/*  Check whether m needs to be attached to the soln, and make sure it is    */
/*  attached or detached as appropriate.                                     */
/*                                                                           */
/*****************************************************************************/

void KheMonitorAttachCheck(KHE_MONITOR m)
{
  if( DEBUG1 )
    fprintf(stderr, "[ KheMonitorAttachCheck(m %p %s) attached %d\n",
      (void *) m, KheMonitorTagShow(m->tag), (int) m->attached);
  switch( m->tag )
  {
    case KHE_ASSIGN_RESOURCE_MONITOR_TAG:

      KheAssignResourceMonitorAttachCheck((KHE_ASSIGN_RESOURCE_MONITOR) m);
      break;

    case KHE_ASSIGN_TIME_MONITOR_TAG:

      KheAssignTimeMonitorAttachCheck((KHE_ASSIGN_TIME_MONITOR) m);
      break;

    case KHE_SPLIT_EVENTS_MONITOR_TAG:

      KheSplitEventsMonitorAttachCheck((KHE_SPLIT_EVENTS_MONITOR) m);
      break;

    case KHE_DISTRIBUTE_SPLIT_EVENTS_MONITOR_TAG:

      KheDistributeSplitEventsMonitorAttachCheck(
	(KHE_DISTRIBUTE_SPLIT_EVENTS_MONITOR) m);
      break;

    case KHE_PREFER_RESOURCES_MONITOR_TAG:

      KhePreferResourcesMonitorAttachCheck((KHE_PREFER_RESOURCES_MONITOR) m);
      break;

    case KHE_PREFER_TIMES_MONITOR_TAG:

      KhePreferTimesMonitorAttachCheck((KHE_PREFER_TIMES_MONITOR) m);
      break;

    case KHE_AVOID_SPLIT_ASSIGNMENTS_MONITOR_TAG:

      KheAvoidSplitAssignmentsMonitorAttachCheck(
	(KHE_AVOID_SPLIT_ASSIGNMENTS_MONITOR) m);
      break;

    case KHE_SPREAD_EVENTS_MONITOR_TAG:

      KheSpreadEventsMonitorAttachCheck((KHE_SPREAD_EVENTS_MONITOR) m);
      break;

    case KHE_LINK_EVENTS_MONITOR_TAG:

      KheLinkEventsMonitorAttachCheck((KHE_LINK_EVENTS_MONITOR) m);
      break;

    case KHE_AVOID_CLASHES_MONITOR_TAG:

      KheAvoidClashesMonitorAttachCheck((KHE_AVOID_CLASHES_MONITOR) m);
      break;

    case KHE_AVOID_UNAVAILABLE_TIMES_MONITOR_TAG:

      KheAvoidUnavailableTimesMonitorAttachCheck(
	(KHE_AVOID_UNAVAILABLE_TIMES_MONITOR) m);
      break;

    case KHE_LIMIT_IDLE_TIMES_MONITOR_TAG:

      KheLimitIdleTimesMonitorAttachCheck((KHE_LIMIT_IDLE_TIMES_MONITOR) m);
      break;

    case KHE_CLUSTER_BUSY_TIMES_MONITOR_TAG:

      KheClusterBusyTimesMonitorAttachCheck((KHE_CLUSTER_BUSY_TIMES_MONITOR) m);
      break;

    case KHE_LIMIT_BUSY_TIMES_MONITOR_TAG:

      KheLimitBusyTimesMonitorAttachCheck((KHE_LIMIT_BUSY_TIMES_MONITOR) m);
      break;

    case KHE_LIMIT_WORKLOAD_MONITOR_TAG:

      KheLimitWorkloadMonitorAttachCheck((KHE_LIMIT_WORKLOAD_MONITOR) m);
      break;

    case KHE_TIMETABLE_MONITOR_TAG:

      KheTimetableMonitorAttachCheck((KHE_TIMETABLE_MONITOR) m);
      break;

    case KHE_TIME_GROUP_MONITOR_TAG:

      KheTimeGroupMonitorAttachCheck((KHE_TIME_GROUP_MONITOR) m);
      break;

    case KHE_GROUP_MONITOR_TAG:

      MAssert(false, "KheMonitorAttachCheck: m is a group monitor");
      break;

    case KHE_ORDINARY_DEMAND_MONITOR_TAG:

      KheOrdinaryDemandMonitorAttachCheck((KHE_ORDINARY_DEMAND_MONITOR) m);
      break;

    case KHE_WORKLOAD_DEMAND_MONITOR_TAG:

      KheWorkloadDemandMonitorAttachCheck((KHE_WORKLOAD_DEMAND_MONITOR) m);
      break;

    case KHE_EVENNESS_MONITOR_TAG:

      KheEvennessMonitorAttachCheck((KHE_EVENNESS_MONITOR) m);
      break;

    default:

      MAssert(false, "KheMonitorAttachCheck: invalid monitor type tag");
  }
  if( DEBUG1 )
    fprintf(stderr, "] KheMonitorAttachCheck(m %p) attached %d returning\n",
      (void *) m, (int) m->attached);
}


/*****************************************************************************/
/*                                                                           */
/*  Submodule "calls emanating from KHE_EVENT_IN_SOLN objects"               */
/*                                                                           */
/*****************************************************************************/

/*****************************************************************************/
/*                                                                           */
/*  void KheMonitorAddMeet(KHE_MONITOR m, KHE_MEET meet)                     */
/*                                                                           */
/*  Inform m that meet has been added to its purview.                        */
/*                                                                           */
/*****************************************************************************/

void KheMonitorAddMeet(KHE_MONITOR m, KHE_MEET meet)
{
  switch( m->tag )
  {
    case KHE_ASSIGN_TIME_MONITOR_TAG:

      KheAssignTimeMonitorAddMeet((KHE_ASSIGN_TIME_MONITOR) m, meet);
      break;

    case KHE_PREFER_TIMES_MONITOR_TAG:

      KhePreferTimesMonitorAddMeet((KHE_PREFER_TIMES_MONITOR) m, meet);
      break;

    case KHE_SPLIT_EVENTS_MONITOR_TAG:

      KheSplitEventsMonitorAddMeet((KHE_SPLIT_EVENTS_MONITOR) m, meet);
      break;

    case KHE_DISTRIBUTE_SPLIT_EVENTS_MONITOR_TAG:

      KheDistributeSplitEventsMonitorAddMeet(
	(KHE_DISTRIBUTE_SPLIT_EVENTS_MONITOR) m, meet);
      break;

    case KHE_TIMETABLE_MONITOR_TAG:

      KheTimetableMonitorAddMeet((KHE_TIMETABLE_MONITOR) m, meet);
      break;

    case KHE_SPREAD_EVENTS_MONITOR_TAG:

      KheSpreadEventsMonitorAddMeet((KHE_SPREAD_EVENTS_MONITOR) m, meet);
      break;


    case KHE_LINK_EVENTS_MONITOR_TAG:
    case KHE_ASSIGN_RESOURCE_MONITOR_TAG:
    case KHE_PREFER_RESOURCES_MONITOR_TAG:
    case KHE_AVOID_SPLIT_ASSIGNMENTS_MONITOR_TAG:
    case KHE_AVOID_CLASHES_MONITOR_TAG:
    case KHE_AVOID_UNAVAILABLE_TIMES_MONITOR_TAG:
    case KHE_LIMIT_IDLE_TIMES_MONITOR_TAG:
    case KHE_CLUSTER_BUSY_TIMES_MONITOR_TAG:
    case KHE_LIMIT_BUSY_TIMES_MONITOR_TAG:
    case KHE_LIMIT_WORKLOAD_MONITOR_TAG:
    case KHE_TIME_GROUP_MONITOR_TAG:
    case KHE_GROUP_MONITOR_TAG:
    case KHE_ORDINARY_DEMAND_MONITOR_TAG:
    case KHE_WORKLOAD_DEMAND_MONITOR_TAG:
    case KHE_EVENNESS_MONITOR_TAG:
    default:

      MAssert(false, "KheMonitorAddMeet: invalid monitor type tag");
  }
}


/*****************************************************************************/
/*                                                                           */
/*  void KheMonitorDeleteMeet(KHE_MONITOR m, KHE_MEET meet)                  */
/*                                                                           */
/*  Inform m that meet has been deleted.                                     */
/*                                                                           */
/*****************************************************************************/

void KheMonitorDeleteMeet(KHE_MONITOR m, KHE_MEET meet)
{
  switch( m->tag )
  {
    case KHE_ASSIGN_TIME_MONITOR_TAG:

      KheAssignTimeMonitorDeleteMeet((KHE_ASSIGN_TIME_MONITOR) m, meet);
      break;

    case KHE_PREFER_TIMES_MONITOR_TAG:

      KhePreferTimesMonitorDeleteMeet((KHE_PREFER_TIMES_MONITOR) m, meet);
      break;

    case KHE_SPLIT_EVENTS_MONITOR_TAG:

      KheSplitEventsMonitorDeleteMeet((KHE_SPLIT_EVENTS_MONITOR) m, meet);
      break;

    case KHE_DISTRIBUTE_SPLIT_EVENTS_MONITOR_TAG:

      KheDistributeSplitEventsMonitorDeleteMeet(
	(KHE_DISTRIBUTE_SPLIT_EVENTS_MONITOR) m, meet);
      break;

    case KHE_TIMETABLE_MONITOR_TAG:

      KheTimetableMonitorDeleteMeet((KHE_TIMETABLE_MONITOR) m, meet);

    case KHE_SPREAD_EVENTS_MONITOR_TAG:

      KheSpreadEventsMonitorDeleteMeet((KHE_SPREAD_EVENTS_MONITOR) m, meet);
      break;

    case KHE_LINK_EVENTS_MONITOR_TAG:
    case KHE_ASSIGN_RESOURCE_MONITOR_TAG:
    case KHE_PREFER_RESOURCES_MONITOR_TAG:
    case KHE_AVOID_SPLIT_ASSIGNMENTS_MONITOR_TAG:
    case KHE_AVOID_CLASHES_MONITOR_TAG:
    case KHE_AVOID_UNAVAILABLE_TIMES_MONITOR_TAG:
    case KHE_LIMIT_IDLE_TIMES_MONITOR_TAG:
    case KHE_CLUSTER_BUSY_TIMES_MONITOR_TAG:
    case KHE_LIMIT_BUSY_TIMES_MONITOR_TAG:
    case KHE_LIMIT_WORKLOAD_MONITOR_TAG:
    case KHE_TIME_GROUP_MONITOR_TAG:
    case KHE_GROUP_MONITOR_TAG:
    case KHE_ORDINARY_DEMAND_MONITOR_TAG:
    case KHE_WORKLOAD_DEMAND_MONITOR_TAG:
    case KHE_EVENNESS_MONITOR_TAG:
    default:

      MAssert(false, "KheMonitorDeleteMeet: invalid monitor type tag");
  }
}


/*****************************************************************************/
/*                                                                           */
/*  void KheMonitorSplitMeet(KHE_MONITOR m, KHE_MEET meet1, KHE_MEET meet2)  */
/*                                                                           */
/*  Inform m that a split into meet1 and meet2 has occurred.                 */
/*                                                                           */
/*****************************************************************************/

void KheMonitorSplitMeet(KHE_MONITOR m, KHE_MEET meet1, KHE_MEET meet2)
{
  switch( m->tag )
  {
    case KHE_ASSIGN_TIME_MONITOR_TAG:

      KheAssignTimeMonitorSplitMeet((KHE_ASSIGN_TIME_MONITOR) m, meet1, meet2);
      break;

    case KHE_PREFER_TIMES_MONITOR_TAG:

      KhePreferTimesMonitorSplitMeet((KHE_PREFER_TIMES_MONITOR) m,
	meet1, meet2);
      break;

    case KHE_SPLIT_EVENTS_MONITOR_TAG:

      KheSplitEventsMonitorSplitMeet((KHE_SPLIT_EVENTS_MONITOR) m,
	meet1, meet2);
      break;

    case KHE_DISTRIBUTE_SPLIT_EVENTS_MONITOR_TAG:

      KheDistributeSplitEventsMonitorSplitMeet(
	(KHE_DISTRIBUTE_SPLIT_EVENTS_MONITOR) m, meet1, meet2);
      break;

    case KHE_TIMETABLE_MONITOR_TAG:

      KheTimetableMonitorSplitMeet((KHE_TIMETABLE_MONITOR) m, meet1, meet2);
      break;

    case KHE_SPREAD_EVENTS_MONITOR_TAG:

      KheSpreadEventsMonitorSplitMeet((KHE_SPREAD_EVENTS_MONITOR) m,
	meet1, meet2);
      break;

    case KHE_LINK_EVENTS_MONITOR_TAG:
    case KHE_ASSIGN_RESOURCE_MONITOR_TAG:
    case KHE_PREFER_RESOURCES_MONITOR_TAG:
    case KHE_AVOID_SPLIT_ASSIGNMENTS_MONITOR_TAG:
    case KHE_AVOID_CLASHES_MONITOR_TAG:
    case KHE_AVOID_UNAVAILABLE_TIMES_MONITOR_TAG:
    case KHE_LIMIT_IDLE_TIMES_MONITOR_TAG:
    case KHE_CLUSTER_BUSY_TIMES_MONITOR_TAG:
    case KHE_LIMIT_WORKLOAD_MONITOR_TAG:
    case KHE_LIMIT_BUSY_TIMES_MONITOR_TAG:
    case KHE_TIME_GROUP_MONITOR_TAG:
    case KHE_GROUP_MONITOR_TAG:
    case KHE_ORDINARY_DEMAND_MONITOR_TAG:
    case KHE_WORKLOAD_DEMAND_MONITOR_TAG:
    case KHE_EVENNESS_MONITOR_TAG:
    default:

      MAssert(false, "KheMonitorSplitMeet: invalid monitor type tag");
  }
}


/*****************************************************************************/
/*                                                                           */
/*  void KheMonitorMergeMeet(KHE_MONITOR m, KHE_MEET meet1, KHE_MEET meet2)  */
/*                                                                           */
/*  Inform m that a merge of meet1 and meet2 is occurring.                   */
/*                                                                           */
/*****************************************************************************/

void KheMonitorMergeMeet(KHE_MONITOR m, KHE_MEET meet1, KHE_MEET meet2)
{
  switch( m->tag )
  {
    case KHE_ASSIGN_TIME_MONITOR_TAG:

      KheAssignTimeMonitorMergeMeet((KHE_ASSIGN_TIME_MONITOR) m, meet1, meet2);
      break;

    case KHE_PREFER_TIMES_MONITOR_TAG:

      KhePreferTimesMonitorMergeMeet((KHE_PREFER_TIMES_MONITOR) m,
	meet1, meet2);
      break;

    case KHE_SPLIT_EVENTS_MONITOR_TAG:

      KheSplitEventsMonitorMergeMeet((KHE_SPLIT_EVENTS_MONITOR) m,
	meet1, meet2);
      break;

    case KHE_DISTRIBUTE_SPLIT_EVENTS_MONITOR_TAG:

      KheDistributeSplitEventsMonitorMergeMeet(
	(KHE_DISTRIBUTE_SPLIT_EVENTS_MONITOR) m, meet1, meet2);
      break;

    case KHE_TIMETABLE_MONITOR_TAG:

      KheTimetableMonitorMergeMeet((KHE_TIMETABLE_MONITOR) m, meet1, meet2);
      break;

    case KHE_SPREAD_EVENTS_MONITOR_TAG:

      KheSpreadEventsMonitorMergeMeet((KHE_SPREAD_EVENTS_MONITOR) m,
	meet1, meet2);
      break;

    case KHE_LINK_EVENTS_MONITOR_TAG:
    case KHE_ASSIGN_RESOURCE_MONITOR_TAG:
    case KHE_PREFER_RESOURCES_MONITOR_TAG:
    case KHE_AVOID_SPLIT_ASSIGNMENTS_MONITOR_TAG:
    case KHE_AVOID_CLASHES_MONITOR_TAG:
    case KHE_AVOID_UNAVAILABLE_TIMES_MONITOR_TAG:
    case KHE_LIMIT_IDLE_TIMES_MONITOR_TAG:
    case KHE_CLUSTER_BUSY_TIMES_MONITOR_TAG:
    case KHE_LIMIT_BUSY_TIMES_MONITOR_TAG:
    case KHE_LIMIT_WORKLOAD_MONITOR_TAG:
    case KHE_TIME_GROUP_MONITOR_TAG:
    case KHE_GROUP_MONITOR_TAG:
    case KHE_ORDINARY_DEMAND_MONITOR_TAG:
    case KHE_WORKLOAD_DEMAND_MONITOR_TAG:
    case KHE_EVENNESS_MONITOR_TAG:
    default:

      MAssert(false, "KheMonitorMergeMeet: invalid monitor type tag");
  }
}


/*****************************************************************************/
/*                                                                           */
/*  void KheMonitorAssignTime(KHE_MONITOR m, KHE_MEET meet,                  */
/*    int assigned_time_index)                                               */
/*                                                                           */
/*  Inform m that a time assignment of meet is occurring.                    */
/*                                                                           */
/*****************************************************************************/

void KheMonitorAssignTime(KHE_MONITOR m, KHE_MEET meet,
  int assigned_time_index)
{
  switch( m->tag )
  {
    case KHE_ASSIGN_TIME_MONITOR_TAG:

      KheAssignTimeMonitorAssignTime((KHE_ASSIGN_TIME_MONITOR) m,
	meet, assigned_time_index);
      break;

    case KHE_PREFER_TIMES_MONITOR_TAG:

      KhePreferTimesMonitorAssignTime((KHE_PREFER_TIMES_MONITOR) m,
	meet, assigned_time_index);
      break;

    case KHE_TIMETABLE_MONITOR_TAG:

      KheTimetableMonitorAssignTime((KHE_TIMETABLE_MONITOR) m,
	meet, assigned_time_index);
      break;

    case KHE_SPREAD_EVENTS_MONITOR_TAG:

      KheSpreadEventsMonitorAssignTime((KHE_SPREAD_EVENTS_MONITOR) m,
	meet, assigned_time_index);
      break;

    /* these two don't need this call and we optimize by omitting them */
    case KHE_SPLIT_EVENTS_MONITOR_TAG:
    case KHE_DISTRIBUTE_SPLIT_EVENTS_MONITOR_TAG:

    case KHE_LINK_EVENTS_MONITOR_TAG:
    case KHE_ASSIGN_RESOURCE_MONITOR_TAG:
    case KHE_PREFER_RESOURCES_MONITOR_TAG:
    case KHE_AVOID_SPLIT_ASSIGNMENTS_MONITOR_TAG:
    case KHE_AVOID_CLASHES_MONITOR_TAG:
    case KHE_AVOID_UNAVAILABLE_TIMES_MONITOR_TAG:
    case KHE_LIMIT_IDLE_TIMES_MONITOR_TAG:
    case KHE_CLUSTER_BUSY_TIMES_MONITOR_TAG:
    case KHE_LIMIT_BUSY_TIMES_MONITOR_TAG:
    case KHE_LIMIT_WORKLOAD_MONITOR_TAG:
    case KHE_TIME_GROUP_MONITOR_TAG:
    case KHE_GROUP_MONITOR_TAG:
    case KHE_ORDINARY_DEMAND_MONITOR_TAG:
    case KHE_WORKLOAD_DEMAND_MONITOR_TAG:
    case KHE_EVENNESS_MONITOR_TAG:
    default:

      MAssert(false, "KheMonitorAssignTime: invalid monitor type tag");
  }
}


/*****************************************************************************/
/*                                                                           */
/*  void KheMonitorUnAssignTime(KHE_MONITOR m, KHE_MEET meet,                */
/*    int assigned_time_index)                                               */
/*                                                                           */
/*  Inform m that a time unassignment of meet is occurring.                  */
/*                                                                           */
/*****************************************************************************/

void KheMonitorUnAssignTime(KHE_MONITOR m, KHE_MEET meet,
  int assigned_time_index)
{
  switch( m->tag )
  {
    case KHE_ASSIGN_TIME_MONITOR_TAG:

      KheAssignTimeMonitorUnAssignTime((KHE_ASSIGN_TIME_MONITOR) m,
	meet, assigned_time_index);
      break;

    case KHE_PREFER_TIMES_MONITOR_TAG:

      KhePreferTimesMonitorUnAssignTime((KHE_PREFER_TIMES_MONITOR) m,
	meet, assigned_time_index);
      break;

    case KHE_TIMETABLE_MONITOR_TAG:

      KheTimetableMonitorUnAssignTime((KHE_TIMETABLE_MONITOR) m,
	meet, assigned_time_index);
      break;

    case KHE_SPREAD_EVENTS_MONITOR_TAG:

      KheSpreadEventsMonitorUnAssignTime((KHE_SPREAD_EVENTS_MONITOR) m,
	meet, assigned_time_index);
      break;

    /* these two don't need this call and we optimize by omitting them */
    case KHE_SPLIT_EVENTS_MONITOR_TAG:
    case KHE_DISTRIBUTE_SPLIT_EVENTS_MONITOR_TAG:

    case KHE_LINK_EVENTS_MONITOR_TAG:
    case KHE_ASSIGN_RESOURCE_MONITOR_TAG:
    case KHE_PREFER_RESOURCES_MONITOR_TAG:
    case KHE_AVOID_SPLIT_ASSIGNMENTS_MONITOR_TAG:
    case KHE_AVOID_CLASHES_MONITOR_TAG:
    case KHE_AVOID_UNAVAILABLE_TIMES_MONITOR_TAG:
    case KHE_LIMIT_IDLE_TIMES_MONITOR_TAG:
    case KHE_CLUSTER_BUSY_TIMES_MONITOR_TAG:
    case KHE_LIMIT_BUSY_TIMES_MONITOR_TAG:
    case KHE_LIMIT_WORKLOAD_MONITOR_TAG:
    case KHE_GROUP_MONITOR_TAG:
    case KHE_ORDINARY_DEMAND_MONITOR_TAG:
    case KHE_WORKLOAD_DEMAND_MONITOR_TAG:
    case KHE_EVENNESS_MONITOR_TAG:
    default:

      MAssert(false, "KheMonitorUnAssignTime: invalid monitor type tag");
  }
}


/*****************************************************************************/
/*                                                                           */
/*  Submodule "calls emanating from KHE_EVENT_RESOURCE_IN_SOLN objects"      */
/*                                                                           */
/*****************************************************************************/

/*****************************************************************************/
/*                                                                           */
/*  void KheMonitorAddTask(KHE_MONITOR m, KHE_TASK task)                     */
/*                                                                           */
/*  Monitor the effect of adding task.                                       */
/*                                                                           */
/*****************************************************************************/

void KheMonitorAddTask(KHE_MONITOR m, KHE_TASK task)
{
  switch( m->tag )
  {
    case KHE_ASSIGN_RESOURCE_MONITOR_TAG:

      KheAssignResourceMonitorAddTask(
        (KHE_ASSIGN_RESOURCE_MONITOR) m, task);
      break;

    case KHE_PREFER_RESOURCES_MONITOR_TAG:

      KhePreferResourcesMonitorAddTask(
        (KHE_PREFER_RESOURCES_MONITOR) m, task);
      break;

    case KHE_AVOID_SPLIT_ASSIGNMENTS_MONITOR_TAG:

      KheAvoidSplitAssignmentsMonitorAddTask(
        (KHE_AVOID_SPLIT_ASSIGNMENTS_MONITOR) m, task);
      break;

    case KHE_LIMIT_IDLE_TIMES_MONITOR_TAG:
    case KHE_CLUSTER_BUSY_TIMES_MONITOR_TAG:
    case KHE_LIMIT_BUSY_TIMES_MONITOR_TAG:
    case KHE_ASSIGN_TIME_MONITOR_TAG:
    case KHE_PREFER_TIMES_MONITOR_TAG:
    case KHE_SPREAD_EVENTS_MONITOR_TAG:
    case KHE_LINK_EVENTS_MONITOR_TAG:
    case KHE_SPLIT_EVENTS_MONITOR_TAG:
    case KHE_DISTRIBUTE_SPLIT_EVENTS_MONITOR_TAG:
    case KHE_AVOID_CLASHES_MONITOR_TAG:
    case KHE_AVOID_UNAVAILABLE_TIMES_MONITOR_TAG:
    case KHE_LIMIT_WORKLOAD_MONITOR_TAG:
    case KHE_TIMETABLE_MONITOR_TAG:
    case KHE_TIME_GROUP_MONITOR_TAG:
    case KHE_GROUP_MONITOR_TAG:
    case KHE_ORDINARY_DEMAND_MONITOR_TAG:
    case KHE_WORKLOAD_DEMAND_MONITOR_TAG:
    case KHE_EVENNESS_MONITOR_TAG:
    default:

      MAssert(false, "KheMonitorAddTask: invalid monitor type tag");
  }
}


/*****************************************************************************/
/*                                                                           */
/*  void KheMonitorDeleteTask(KHE_MONITOR m, KHE_TASK task)                  */
/*                                                                           */
/*  Monitor the effect of deleting task.                                     */
/*                                                                           */
/*****************************************************************************/

void KheMonitorDeleteTask(KHE_MONITOR m, KHE_TASK task)
{
  switch( m->tag )
  {
    case KHE_ASSIGN_RESOURCE_MONITOR_TAG:

      KheAssignResourceMonitorDeleteTask(
        (KHE_ASSIGN_RESOURCE_MONITOR) m, task);
      break;

    case KHE_PREFER_RESOURCES_MONITOR_TAG:

      KhePreferResourcesMonitorDeleteTask(
        (KHE_PREFER_RESOURCES_MONITOR) m, task);
      break;

    case KHE_AVOID_SPLIT_ASSIGNMENTS_MONITOR_TAG:

      KheAvoidSplitAssignmentsMonitorDeleteTask(
        (KHE_AVOID_SPLIT_ASSIGNMENTS_MONITOR) m, task);
      break;

    case KHE_LIMIT_IDLE_TIMES_MONITOR_TAG:
    case KHE_CLUSTER_BUSY_TIMES_MONITOR_TAG:
    case KHE_LIMIT_BUSY_TIMES_MONITOR_TAG:
    case KHE_ASSIGN_TIME_MONITOR_TAG:
    case KHE_PREFER_TIMES_MONITOR_TAG:
    case KHE_SPREAD_EVENTS_MONITOR_TAG:
    case KHE_LINK_EVENTS_MONITOR_TAG:
    case KHE_SPLIT_EVENTS_MONITOR_TAG:
    case KHE_DISTRIBUTE_SPLIT_EVENTS_MONITOR_TAG:
    case KHE_AVOID_CLASHES_MONITOR_TAG:
    case KHE_AVOID_UNAVAILABLE_TIMES_MONITOR_TAG:
    case KHE_LIMIT_WORKLOAD_MONITOR_TAG:
    case KHE_TIMETABLE_MONITOR_TAG:
    case KHE_TIME_GROUP_MONITOR_TAG:
    case KHE_GROUP_MONITOR_TAG:
    case KHE_ORDINARY_DEMAND_MONITOR_TAG:
    case KHE_WORKLOAD_DEMAND_MONITOR_TAG:
    case KHE_EVENNESS_MONITOR_TAG:
    default:

      MAssert(false, "KheMonitorDeleteTask: invalid monitor type tag");
  }
}


/*****************************************************************************/
/*                                                                           */
/*  void KheMonitorSplitTask(KHE_MONITOR m, KHE_TASK task1, KHE_TASK task2)  */
/*                                                                           */
/*  Let m know that a task has just split into task1 and task2.              */
/*  Either both tasks are assigned, or they aren't.  This                    */
/*  call emanates from KHE_RESOURCE_IN_SOLN as well.                         */
/*                                                                           */
/*****************************************************************************/

void KheMonitorSplitTask(KHE_MONITOR m, KHE_TASK task1, KHE_TASK task2)
{
  switch( m->tag )
  {
    case KHE_ASSIGN_RESOURCE_MONITOR_TAG:

      KheAssignResourceMonitorSplitTask(
        (KHE_ASSIGN_RESOURCE_MONITOR) m, task1, task2);
      break;

    case KHE_PREFER_RESOURCES_MONITOR_TAG:

      KhePreferResourcesMonitorSplitTask(
        (KHE_PREFER_RESOURCES_MONITOR) m, task1, task2);
      break;

    case KHE_AVOID_SPLIT_ASSIGNMENTS_MONITOR_TAG:

      KheAvoidSplitAssignmentsMonitorSplitTask(
        (KHE_AVOID_SPLIT_ASSIGNMENTS_MONITOR) m, task1, task2);
      break;

    case KHE_LIMIT_WORKLOAD_MONITOR_TAG:

      /* called but nothing to do in this case */
      break;

    case KHE_TIMETABLE_MONITOR_TAG:

      KheTimetableMonitorSplitTask((KHE_TIMETABLE_MONITOR) m, task1, task2);
      break;

    case KHE_LIMIT_IDLE_TIMES_MONITOR_TAG:
    case KHE_CLUSTER_BUSY_TIMES_MONITOR_TAG:
    case KHE_LIMIT_BUSY_TIMES_MONITOR_TAG:
    case KHE_ASSIGN_TIME_MONITOR_TAG:
    case KHE_PREFER_TIMES_MONITOR_TAG:
    case KHE_SPREAD_EVENTS_MONITOR_TAG:
    case KHE_LINK_EVENTS_MONITOR_TAG:
    case KHE_SPLIT_EVENTS_MONITOR_TAG:
    case KHE_DISTRIBUTE_SPLIT_EVENTS_MONITOR_TAG:
    case KHE_AVOID_CLASHES_MONITOR_TAG:
    case KHE_AVOID_UNAVAILABLE_TIMES_MONITOR_TAG:
    case KHE_TIME_GROUP_MONITOR_TAG:
    case KHE_GROUP_MONITOR_TAG:
    case KHE_ORDINARY_DEMAND_MONITOR_TAG:
    case KHE_WORKLOAD_DEMAND_MONITOR_TAG:
    case KHE_EVENNESS_MONITOR_TAG:
    default:

      MAssert(false, "KheMonitorSplitTask: invalid monitor type tag");
  }
}


/*****************************************************************************/
/*                                                                           */
/*  void KheMonitorMergeTask(KHE_MONITOR m, KHE_TASK task1, KHE_TASK task2)  */
/*                                                                           */
/*  Let m know that task1 and task2 are just about to be merged.             */
/*                                                                           */
/*****************************************************************************/

void KheMonitorMergeTask(KHE_MONITOR m, KHE_TASK task1, KHE_TASK task2)
{
  switch( m->tag )
  {
    case KHE_ASSIGN_RESOURCE_MONITOR_TAG:

      KheAssignResourceMonitorMergeTask(
        (KHE_ASSIGN_RESOURCE_MONITOR) m, task1, task2);
      break;

    case KHE_PREFER_RESOURCES_MONITOR_TAG:

      KhePreferResourcesMonitorMergeTask(
        (KHE_PREFER_RESOURCES_MONITOR) m, task1, task2);
      break;

    case KHE_AVOID_SPLIT_ASSIGNMENTS_MONITOR_TAG:

      KheAvoidSplitAssignmentsMonitorMergeTask(
        (KHE_AVOID_SPLIT_ASSIGNMENTS_MONITOR) m, task1, task2);
      break;

    case KHE_LIMIT_WORKLOAD_MONITOR_TAG:

      /* called but nothing to do in this case */
      break;

    case KHE_TIMETABLE_MONITOR_TAG:

      KheTimetableMonitorMergeTask((KHE_TIMETABLE_MONITOR) m, task1, task2);
      break;

    case KHE_LIMIT_IDLE_TIMES_MONITOR_TAG:
    case KHE_CLUSTER_BUSY_TIMES_MONITOR_TAG:
    case KHE_LIMIT_BUSY_TIMES_MONITOR_TAG:
    case KHE_ASSIGN_TIME_MONITOR_TAG:
    case KHE_PREFER_TIMES_MONITOR_TAG:
    case KHE_SPREAD_EVENTS_MONITOR_TAG:
    case KHE_LINK_EVENTS_MONITOR_TAG:
    case KHE_SPLIT_EVENTS_MONITOR_TAG:
    case KHE_DISTRIBUTE_SPLIT_EVENTS_MONITOR_TAG:
    case KHE_AVOID_CLASHES_MONITOR_TAG:
    case KHE_AVOID_UNAVAILABLE_TIMES_MONITOR_TAG:
    case KHE_TIME_GROUP_MONITOR_TAG:
    case KHE_GROUP_MONITOR_TAG:
    case KHE_ORDINARY_DEMAND_MONITOR_TAG:
    case KHE_WORKLOAD_DEMAND_MONITOR_TAG:
    case KHE_EVENNESS_MONITOR_TAG:
    default:

      MAssert(false, "KheMonitorMergeTask: invalid monitor type tag");
  }
}


/*****************************************************************************/
/*                                                                           */
/*  void KheMonitorAssignResource(KHE_MONITOR m, KHE_TASK task,              */
/*    KHE_RESOURCE r)                                                        */
/*                                                                           */
/*  Let m know that task has just been assigned resource r.                  */
/*                                                                           */
/*****************************************************************************/

void KheMonitorAssignResource(KHE_MONITOR m, KHE_TASK task, KHE_RESOURCE r)
{
  switch( m->tag )
  {
    case KHE_ASSIGN_RESOURCE_MONITOR_TAG:

      KheAssignResourceMonitorAssignResource(
        (KHE_ASSIGN_RESOURCE_MONITOR) m, task, r);
      break;

    case KHE_PREFER_RESOURCES_MONITOR_TAG:

      KhePreferResourcesMonitorAssignResource(
        (KHE_PREFER_RESOURCES_MONITOR) m, task, r);
      break;

    case KHE_AVOID_SPLIT_ASSIGNMENTS_MONITOR_TAG:

      KheAvoidSplitAssignmentsMonitorAssignResource(
        (KHE_AVOID_SPLIT_ASSIGNMENTS_MONITOR) m, task, r);
      break;

    case KHE_LIMIT_WORKLOAD_MONITOR_TAG:

      KheLimitWorkloadMonitorAssignResource(
	(KHE_LIMIT_WORKLOAD_MONITOR) m, task, r);
      break;

    case KHE_TIMETABLE_MONITOR_TAG:

      KheTimetableMonitorAssignResource((KHE_TIMETABLE_MONITOR) m, task, r);
      break;

    case KHE_LIMIT_IDLE_TIMES_MONITOR_TAG:
    case KHE_CLUSTER_BUSY_TIMES_MONITOR_TAG:
    case KHE_LIMIT_BUSY_TIMES_MONITOR_TAG:
    case KHE_ASSIGN_TIME_MONITOR_TAG:
    case KHE_PREFER_TIMES_MONITOR_TAG:
    case KHE_SPREAD_EVENTS_MONITOR_TAG:
    case KHE_LINK_EVENTS_MONITOR_TAG:
    case KHE_SPLIT_EVENTS_MONITOR_TAG:
    case KHE_DISTRIBUTE_SPLIT_EVENTS_MONITOR_TAG:
    case KHE_AVOID_CLASHES_MONITOR_TAG:
    case KHE_AVOID_UNAVAILABLE_TIMES_MONITOR_TAG:
    case KHE_TIME_GROUP_MONITOR_TAG:
    case KHE_GROUP_MONITOR_TAG:
    case KHE_ORDINARY_DEMAND_MONITOR_TAG:
    case KHE_WORKLOAD_DEMAND_MONITOR_TAG:
    case KHE_EVENNESS_MONITOR_TAG:
    default:

      MAssert(false, "KheMonitorAssignResource: invalid monitor type tag");
  }
}


/*****************************************************************************/
/*                                                                           */
/*  void KheMonitorUnAssignResource(KHE_MONITOR m, KHE_TASK task,            */
/*    KHE_RESOURCE r)                                                        */
/*                                                                           */
/*  Let m know that task has just been unassigned resource r.                */
/*                                                                           */
/*****************************************************************************/

void KheMonitorUnAssignResource(KHE_MONITOR m, KHE_TASK task,
  KHE_RESOURCE r)
{
  switch( m->tag )
  {
    case KHE_ASSIGN_RESOURCE_MONITOR_TAG:

      KheAssignResourceMonitorUnAssignResource(
        (KHE_ASSIGN_RESOURCE_MONITOR) m, task, r);
      break;

    case KHE_PREFER_RESOURCES_MONITOR_TAG:

      KhePreferResourcesMonitorUnAssignResource(
        (KHE_PREFER_RESOURCES_MONITOR) m, task, r);
      break;

    case KHE_AVOID_SPLIT_ASSIGNMENTS_MONITOR_TAG:

      KheAvoidSplitAssignmentsMonitorUnAssignResource(
        (KHE_AVOID_SPLIT_ASSIGNMENTS_MONITOR) m, task, r);
      break;

    case KHE_LIMIT_WORKLOAD_MONITOR_TAG:

      KheLimitWorkloadMonitorUnAssignResource(
        (KHE_LIMIT_WORKLOAD_MONITOR) m, task, r);
      break;

    case KHE_TIMETABLE_MONITOR_TAG:

      KheTimetableMonitorUnAssignResource(
        (KHE_TIMETABLE_MONITOR) m, task, r);
      break;

    case KHE_LIMIT_IDLE_TIMES_MONITOR_TAG:
    case KHE_CLUSTER_BUSY_TIMES_MONITOR_TAG:
    case KHE_LIMIT_BUSY_TIMES_MONITOR_TAG:
    case KHE_ASSIGN_TIME_MONITOR_TAG:
    case KHE_PREFER_TIMES_MONITOR_TAG:
    case KHE_SPREAD_EVENTS_MONITOR_TAG:
    case KHE_LINK_EVENTS_MONITOR_TAG:
    case KHE_SPLIT_EVENTS_MONITOR_TAG:
    case KHE_DISTRIBUTE_SPLIT_EVENTS_MONITOR_TAG:
    case KHE_AVOID_CLASHES_MONITOR_TAG:
    case KHE_AVOID_UNAVAILABLE_TIMES_MONITOR_TAG:
    case KHE_TIME_GROUP_MONITOR_TAG:
    case KHE_GROUP_MONITOR_TAG:
    case KHE_ORDINARY_DEMAND_MONITOR_TAG:
    case KHE_WORKLOAD_DEMAND_MONITOR_TAG:
    case KHE_EVENNESS_MONITOR_TAG:
    default:

      MAssert(false, "KheMonitorUnAssignResource: invalid monitor type tag");
  }
}


/*****************************************************************************/
/*                                                                           */
/*  Submodule "calls emanating from KHE_RESOURCE_IN_SOLN objects"            */
/*                                                                           */
/*  KheMonitorSplitTask, KheMonitorMergeTask,                                */
/*  KheMonitorAssignResource and KheMonitorUnAssignResource also emanate     */
/*  from KHE_EVENT_RESOURCE_IN_SOLN objects, and appear above.               */
/*                                                                           */
/*****************************************************************************/

/*****************************************************************************/
/*                                                                           */
/*  void KheMonitorTaskAssignTime(KHE_MONITOR m, KHE_TASK task,              */
/*    int assigned_time_index)                                               */
/*                                                                           */
/*  Let m know that task's meet has been assigned this time.                 */
/*                                                                           */
/*****************************************************************************/

void KheMonitorTaskAssignTime(KHE_MONITOR m, KHE_TASK task,
  int assigned_time_index)
{
  switch( m->tag )
  {
    case KHE_LIMIT_WORKLOAD_MONITOR_TAG:
    case KHE_WORKLOAD_DEMAND_MONITOR_TAG:

      /* will be called but there is nothing to do */
      break;

    case KHE_TIMETABLE_MONITOR_TAG:

      KheTimetableMonitorTaskAssignTime((KHE_TIMETABLE_MONITOR) m,
	task, assigned_time_index);
      break;

    case KHE_ASSIGN_RESOURCE_MONITOR_TAG:
    case KHE_PREFER_RESOURCES_MONITOR_TAG:
    case KHE_AVOID_SPLIT_ASSIGNMENTS_MONITOR_TAG:
    case KHE_LIMIT_IDLE_TIMES_MONITOR_TAG:
    case KHE_CLUSTER_BUSY_TIMES_MONITOR_TAG:
    case KHE_LIMIT_BUSY_TIMES_MONITOR_TAG:
    case KHE_ASSIGN_TIME_MONITOR_TAG:
    case KHE_PREFER_TIMES_MONITOR_TAG:
    case KHE_SPREAD_EVENTS_MONITOR_TAG:
    case KHE_LINK_EVENTS_MONITOR_TAG:
    case KHE_SPLIT_EVENTS_MONITOR_TAG:
    case KHE_DISTRIBUTE_SPLIT_EVENTS_MONITOR_TAG:
    case KHE_AVOID_CLASHES_MONITOR_TAG:
    case KHE_AVOID_UNAVAILABLE_TIMES_MONITOR_TAG:
    case KHE_TIME_GROUP_MONITOR_TAG:
    case KHE_GROUP_MONITOR_TAG:
    case KHE_ORDINARY_DEMAND_MONITOR_TAG:
    case KHE_EVENNESS_MONITOR_TAG:
    default:

      MAssert(false,
	"KheMonitorTaskAssignTime: invalid monitor type tag");
  }
}


/*****************************************************************************/
/*                                                                           */
/*  void KheMonitorTaskUnAssignTime(KHE_MONITOR m, KHE_TASK task,            */
/*    int assigned_time_index)                                               */
/*                                                                           */
/*  Let m know that task's meet has been unassigned this time.               */
/*                                                                           */
/*****************************************************************************/

void KheMonitorTaskUnAssignTime(KHE_MONITOR m, KHE_TASK task,
  int assigned_time_index)
{
  switch( m->tag )
  {
    case KHE_LIMIT_WORKLOAD_MONITOR_TAG:
    case KHE_WORKLOAD_DEMAND_MONITOR_TAG:

      /* will be called but there is nothing to do */
      break;

    case KHE_TIMETABLE_MONITOR_TAG:

      KheTimetableMonitorTaskUnAssignTime((KHE_TIMETABLE_MONITOR) m,
	task, assigned_time_index);
      break;

    case KHE_ASSIGN_RESOURCE_MONITOR_TAG:
    case KHE_PREFER_RESOURCES_MONITOR_TAG:
    case KHE_AVOID_SPLIT_ASSIGNMENTS_MONITOR_TAG:
    case KHE_LIMIT_IDLE_TIMES_MONITOR_TAG:
    case KHE_CLUSTER_BUSY_TIMES_MONITOR_TAG:
    case KHE_LIMIT_BUSY_TIMES_MONITOR_TAG:
    case KHE_ASSIGN_TIME_MONITOR_TAG:
    case KHE_PREFER_TIMES_MONITOR_TAG:
    case KHE_SPREAD_EVENTS_MONITOR_TAG:
    case KHE_LINK_EVENTS_MONITOR_TAG:
    case KHE_SPLIT_EVENTS_MONITOR_TAG:
    case KHE_DISTRIBUTE_SPLIT_EVENTS_MONITOR_TAG:
    case KHE_AVOID_CLASHES_MONITOR_TAG:
    case KHE_AVOID_UNAVAILABLE_TIMES_MONITOR_TAG:
    case KHE_TIME_GROUP_MONITOR_TAG:
    case KHE_GROUP_MONITOR_TAG:
    case KHE_ORDINARY_DEMAND_MONITOR_TAG:
    case KHE_EVENNESS_MONITOR_TAG:
    default:

      MAssert(false,
	"KheMonitorTaskUnAssignTime: invalid monitor type tag");
  }
}


/*****************************************************************************/
/*                                                                           */
/*  Submodule "calls emanating from KHE_TIMETABLE_MONITOR objects"           */
/*                                                                           */
/*****************************************************************************/

/*****************************************************************************/
/*                                                                           */
/*  void KheMonitorAssignNonClash(KHE_MONITOR m, int assigned_time_index)    */
/*                                                                           */
/*  This time is changing from unassigned to assigned in the timetable.      */
/*                                                                           */
/*****************************************************************************/

void KheMonitorAssignNonClash(KHE_MONITOR m, int assigned_time_index)
{
  switch( m->tag )
  {
    case KHE_LINK_EVENTS_MONITOR_TAG:

      KheLinkEventsMonitorAssignNonClash(
	(KHE_LINK_EVENTS_MONITOR) m, assigned_time_index);
      break;

    case KHE_TIME_GROUP_MONITOR_TAG:

      KheTimeGroupMonitorAssignNonClash(
	(KHE_TIME_GROUP_MONITOR) m, assigned_time_index);
      break;

    case KHE_AVOID_UNAVAILABLE_TIMES_MONITOR_TAG:
    case KHE_LIMIT_IDLE_TIMES_MONITOR_TAG:
    case KHE_CLUSTER_BUSY_TIMES_MONITOR_TAG:
    case KHE_LIMIT_BUSY_TIMES_MONITOR_TAG:
    case KHE_SPREAD_EVENTS_MONITOR_TAG:
    case KHE_ASSIGN_TIME_MONITOR_TAG:
    case KHE_PREFER_TIMES_MONITOR_TAG:
    case KHE_SPLIT_EVENTS_MONITOR_TAG:
    case KHE_DISTRIBUTE_SPLIT_EVENTS_MONITOR_TAG:
    case KHE_ASSIGN_RESOURCE_MONITOR_TAG:
    case KHE_PREFER_RESOURCES_MONITOR_TAG:
    case KHE_AVOID_SPLIT_ASSIGNMENTS_MONITOR_TAG:
    case KHE_AVOID_CLASHES_MONITOR_TAG:
    case KHE_LIMIT_WORKLOAD_MONITOR_TAG:
    case KHE_TIMETABLE_MONITOR_TAG:
    case KHE_GROUP_MONITOR_TAG:
    case KHE_ORDINARY_DEMAND_MONITOR_TAG:
    case KHE_WORKLOAD_DEMAND_MONITOR_TAG:
    case KHE_EVENNESS_MONITOR_TAG:
    default:

      MAssert(false, "KheMonitorAssignNonClash: invalid monitor type tag");
  }
}


/*****************************************************************************/
/*                                                                           */
/*  void KheMonitorUnAssignNonClash(KHE_MONITOR m, int assigned_time_index)  */
/*                                                                           */
/*  This time is changing from assigned to unassigned in the timetable.      */
/*                                                                           */
/*****************************************************************************/

void KheMonitorUnAssignNonClash(KHE_MONITOR m, int assigned_time_index)
{
  switch( m->tag )
  {
    case KHE_LINK_EVENTS_MONITOR_TAG:

      KheLinkEventsMonitorUnAssignNonClash(
	(KHE_LINK_EVENTS_MONITOR) m, assigned_time_index);
      break;

    case KHE_TIME_GROUP_MONITOR_TAG:

      KheTimeGroupMonitorUnAssignNonClash(
	(KHE_TIME_GROUP_MONITOR) m, assigned_time_index);
      break;

    case KHE_AVOID_UNAVAILABLE_TIMES_MONITOR_TAG:
    case KHE_LIMIT_IDLE_TIMES_MONITOR_TAG:
    case KHE_CLUSTER_BUSY_TIMES_MONITOR_TAG:
    case KHE_LIMIT_BUSY_TIMES_MONITOR_TAG:
    case KHE_SPREAD_EVENTS_MONITOR_TAG:
    case KHE_ASSIGN_TIME_MONITOR_TAG:
    case KHE_PREFER_TIMES_MONITOR_TAG:
    case KHE_SPLIT_EVENTS_MONITOR_TAG:
    case KHE_DISTRIBUTE_SPLIT_EVENTS_MONITOR_TAG:
    case KHE_ASSIGN_RESOURCE_MONITOR_TAG:
    case KHE_PREFER_RESOURCES_MONITOR_TAG:
    case KHE_AVOID_SPLIT_ASSIGNMENTS_MONITOR_TAG:
    case KHE_AVOID_CLASHES_MONITOR_TAG:
    case KHE_LIMIT_WORKLOAD_MONITOR_TAG:
    case KHE_TIMETABLE_MONITOR_TAG:
    case KHE_GROUP_MONITOR_TAG:
    case KHE_ORDINARY_DEMAND_MONITOR_TAG:
    case KHE_WORKLOAD_DEMAND_MONITOR_TAG:
    case KHE_EVENNESS_MONITOR_TAG:
    default:

      MAssert(false, "KheMonitorUnAssignNonClash: invalid monitor type tag");
  }
}


/*****************************************************************************/
/*                                                                           */
/*  void KheMonitorFlush(KHE_MONITOR m)                                      */
/*                                                                           */
/*  Flush m.                                                                 */
/*                                                                           */
/*****************************************************************************/

void KheMonitorFlush(KHE_MONITOR m)
{
  switch( m->tag )
  {
    case KHE_AVOID_CLASHES_MONITOR_TAG:

      KheAvoidClashesMonitorFlush((KHE_AVOID_CLASHES_MONITOR) m);
      break;

    case KHE_LINK_EVENTS_MONITOR_TAG:

      KheLinkEventsMonitorFlush((KHE_LINK_EVENTS_MONITOR) m);
      break;

    case KHE_TIME_GROUP_MONITOR_TAG:

      KheTimeGroupMonitorFlush((KHE_TIME_GROUP_MONITOR) m);
      break;

    case KHE_AVOID_UNAVAILABLE_TIMES_MONITOR_TAG:
    case KHE_ASSIGN_TIME_MONITOR_TAG:
    case KHE_PREFER_TIMES_MONITOR_TAG:
    case KHE_SPLIT_EVENTS_MONITOR_TAG:
    case KHE_DISTRIBUTE_SPLIT_EVENTS_MONITOR_TAG:
    case KHE_SPREAD_EVENTS_MONITOR_TAG:
    case KHE_ASSIGN_RESOURCE_MONITOR_TAG:
    case KHE_PREFER_RESOURCES_MONITOR_TAG:
    case KHE_AVOID_SPLIT_ASSIGNMENTS_MONITOR_TAG:
    case KHE_LIMIT_IDLE_TIMES_MONITOR_TAG:
    case KHE_CLUSTER_BUSY_TIMES_MONITOR_TAG:
    case KHE_LIMIT_BUSY_TIMES_MONITOR_TAG:
    case KHE_LIMIT_WORKLOAD_MONITOR_TAG:
    case KHE_TIMETABLE_MONITOR_TAG:
    case KHE_GROUP_MONITOR_TAG:
    case KHE_ORDINARY_DEMAND_MONITOR_TAG:
    case KHE_WORKLOAD_DEMAND_MONITOR_TAG:
    case KHE_EVENNESS_MONITOR_TAG:
    default:

      MAssert(false, "KheMonitorFlush: invalid monitor type tag");
  }
}


/*****************************************************************************/
/*                                                                           */
/*  Submodule "calls emanating from KHE_TIME_GROUP_MONITOR objects"          */
/*                                                                           */
/*****************************************************************************/

/*****************************************************************************/
/*                                                                           */
/*  void KheMonitorAddBusyAndIdle(KHE_MONITOR m, int busy_count,             */
/*    int idle_count)                                                        */
/*                                                                           */
/*  Add a link coming in to m with these values.                             */
/*                                                                           */
/*****************************************************************************/

void KheMonitorAddBusyAndIdle(KHE_MONITOR m, int busy_count,
  int idle_count)
{
  switch( m->tag )
  {
    case KHE_AVOID_UNAVAILABLE_TIMES_MONITOR_TAG:

      KheAvoidUnavailableTimesMonitorAddBusyAndIdle(
	(KHE_AVOID_UNAVAILABLE_TIMES_MONITOR) m, busy_count, idle_count);
      break;

    case KHE_LIMIT_IDLE_TIMES_MONITOR_TAG:

      KheLimitIdleTimesMonitorAddBusyAndIdle(
	(KHE_LIMIT_IDLE_TIMES_MONITOR) m, busy_count, idle_count);
      break;

    case KHE_CLUSTER_BUSY_TIMES_MONITOR_TAG:

      KheClusterBusyTimesMonitorAddBusyAndIdle(
	(KHE_CLUSTER_BUSY_TIMES_MONITOR) m, busy_count, idle_count);
      break;

    case KHE_LIMIT_BUSY_TIMES_MONITOR_TAG:

      KheLimitBusyTimesMonitorAddBusyAndIdle(
	(KHE_LIMIT_BUSY_TIMES_MONITOR) m, busy_count, idle_count);
      break;

    case KHE_ASSIGN_TIME_MONITOR_TAG:
    case KHE_PREFER_TIMES_MONITOR_TAG:
    case KHE_LINK_EVENTS_MONITOR_TAG:
    case KHE_SPREAD_EVENTS_MONITOR_TAG:
    case KHE_SPLIT_EVENTS_MONITOR_TAG:
    case KHE_DISTRIBUTE_SPLIT_EVENTS_MONITOR_TAG:
    case KHE_ASSIGN_RESOURCE_MONITOR_TAG:
    case KHE_PREFER_RESOURCES_MONITOR_TAG:
    case KHE_AVOID_SPLIT_ASSIGNMENTS_MONITOR_TAG:
    case KHE_AVOID_CLASHES_MONITOR_TAG:
    case KHE_LIMIT_WORKLOAD_MONITOR_TAG:
    case KHE_TIMETABLE_MONITOR_TAG:
    case KHE_TIME_GROUP_MONITOR_TAG:
    case KHE_GROUP_MONITOR_TAG:
    case KHE_ORDINARY_DEMAND_MONITOR_TAG:
    case KHE_WORKLOAD_DEMAND_MONITOR_TAG:
    case KHE_EVENNESS_MONITOR_TAG:
    default:

      MAssert(false, "KheMonitorAddBusyAndIdle: invalid monitor type tag");
  }
}


/*****************************************************************************/
/*                                                                           */
/*  void KheMonitorDeleteBusyAndIdle(KHE_MONITOR m, int busy_count,          */
/*    int idle_count)                                                        */
/*                                                                           */
/*  Remove a link coming in to m with these values.                          */
/*                                                                           */
/*****************************************************************************/

void KheMonitorDeleteBusyAndIdle(KHE_MONITOR m, int busy_count,
  int idle_count)
{
  switch( m->tag )
  {
    case KHE_AVOID_UNAVAILABLE_TIMES_MONITOR_TAG:

      KheAvoidUnavailableTimesMonitorDeleteBusyAndIdle(
	(KHE_AVOID_UNAVAILABLE_TIMES_MONITOR) m, busy_count, idle_count);
      break;

    case KHE_LIMIT_IDLE_TIMES_MONITOR_TAG:

      KheLimitIdleTimesMonitorDeleteBusyAndIdle(
	(KHE_LIMIT_IDLE_TIMES_MONITOR) m, busy_count, idle_count);
      break;

    case KHE_CLUSTER_BUSY_TIMES_MONITOR_TAG:

      KheClusterBusyTimesMonitorDeleteBusyAndIdle(
	(KHE_CLUSTER_BUSY_TIMES_MONITOR) m, busy_count, idle_count);
      break;

    case KHE_LIMIT_BUSY_TIMES_MONITOR_TAG:

      KheLimitBusyTimesMonitorDeleteBusyAndIdle(
	(KHE_LIMIT_BUSY_TIMES_MONITOR) m, busy_count, idle_count);
      break;


    case KHE_ASSIGN_TIME_MONITOR_TAG:
    case KHE_PREFER_TIMES_MONITOR_TAG:
    case KHE_LINK_EVENTS_MONITOR_TAG:
    case KHE_SPREAD_EVENTS_MONITOR_TAG:
    case KHE_SPLIT_EVENTS_MONITOR_TAG:
    case KHE_DISTRIBUTE_SPLIT_EVENTS_MONITOR_TAG:
    case KHE_ASSIGN_RESOURCE_MONITOR_TAG:
    case KHE_PREFER_RESOURCES_MONITOR_TAG:
    case KHE_AVOID_SPLIT_ASSIGNMENTS_MONITOR_TAG:
    case KHE_AVOID_CLASHES_MONITOR_TAG:
    case KHE_LIMIT_WORKLOAD_MONITOR_TAG:
    case KHE_TIMETABLE_MONITOR_TAG:
    case KHE_TIME_GROUP_MONITOR_TAG:
    case KHE_GROUP_MONITOR_TAG:
    case KHE_ORDINARY_DEMAND_MONITOR_TAG:
    case KHE_WORKLOAD_DEMAND_MONITOR_TAG:
    case KHE_EVENNESS_MONITOR_TAG:
    default:

      MAssert(false, "KheMonitorDeleteBusyAndIdle: invalid monitor type tag");
  }
}


/*****************************************************************************/
/*                                                                           */
/*  void KheMonitorChangeBusyAndIdle(KHE_MONITOR m, int busy_count,          */
/*    int idle_count)                                                        */
/*                                                                           */
/*  Change a link coming in to m from these old to new values.               */
/*                                                                           */
/*****************************************************************************/

void KheMonitorChangeBusyAndIdle(KHE_MONITOR m, int old_busy_count,
  int new_busy_count, int old_idle_count, int new_idle_count)
{
  switch( m->tag )
  {
    case KHE_AVOID_UNAVAILABLE_TIMES_MONITOR_TAG:

      KheAvoidUnavailableTimesMonitorChangeBusyAndIdle(
	(KHE_AVOID_UNAVAILABLE_TIMES_MONITOR) m,
	old_busy_count, new_busy_count, old_idle_count, new_idle_count);
      break;

    case KHE_LIMIT_IDLE_TIMES_MONITOR_TAG:

      KheLimitIdleTimesMonitorChangeBusyAndIdle(
	(KHE_LIMIT_IDLE_TIMES_MONITOR) m,
	old_busy_count, new_busy_count, old_idle_count, new_idle_count);
      break;

    case KHE_CLUSTER_BUSY_TIMES_MONITOR_TAG:

      KheClusterBusyTimesMonitorChangeBusyAndIdle(
	(KHE_CLUSTER_BUSY_TIMES_MONITOR) m,
	old_busy_count, new_busy_count, old_idle_count, new_idle_count);
      break;

    case KHE_LIMIT_BUSY_TIMES_MONITOR_TAG:

      KheLimitBusyTimesMonitorChangeBusyAndIdle(
	(KHE_LIMIT_BUSY_TIMES_MONITOR) m,
	old_busy_count, new_busy_count, old_idle_count, new_idle_count);
      break;


    case KHE_ASSIGN_TIME_MONITOR_TAG:
    case KHE_PREFER_TIMES_MONITOR_TAG:
    case KHE_LINK_EVENTS_MONITOR_TAG:
    case KHE_SPREAD_EVENTS_MONITOR_TAG:
    case KHE_SPLIT_EVENTS_MONITOR_TAG:
    case KHE_DISTRIBUTE_SPLIT_EVENTS_MONITOR_TAG:
    case KHE_ASSIGN_RESOURCE_MONITOR_TAG:
    case KHE_PREFER_RESOURCES_MONITOR_TAG:
    case KHE_AVOID_SPLIT_ASSIGNMENTS_MONITOR_TAG:
    case KHE_AVOID_CLASHES_MONITOR_TAG:
    case KHE_LIMIT_WORKLOAD_MONITOR_TAG:
    case KHE_TIMETABLE_MONITOR_TAG:
    case KHE_TIME_GROUP_MONITOR_TAG:
    case KHE_GROUP_MONITOR_TAG:
    case KHE_ORDINARY_DEMAND_MONITOR_TAG:
    case KHE_WORKLOAD_DEMAND_MONITOR_TAG:
    case KHE_EVENNESS_MONITOR_TAG:
    default:

      MAssert(false, "KheMonitorChangeBusyAndIdle: invalid monitor type tag");
  }
}


/*****************************************************************************/
/*                                                                           */
/*  Submodule "operations on demand monitors"                                */
/*                                                                           */
/*****************************************************************************/

/*****************************************************************************/
/*                                                                           */
/*  KHE_MONITOR KheMonitorFirstCompetitor(KHE_MONITOR m)                     */
/*                                                                           */
/*  Return the first competitor of demand monitor m.                         */
/*                                                                           */
/*****************************************************************************/

KHE_MONITOR KheMonitorFirstCompetitor(KHE_MONITOR m)
{
  MAssert(m->tag == KHE_ORDINARY_DEMAND_MONITOR_TAG ||
    m->tag == KHE_WORKLOAD_DEMAND_MONITOR_TAG,
    "KheMonitorFirstCompetitor: m is not a demand monitor");
  MAssert(m->attached, "KheMonitorFirstCompetitor: m is not attached");
  return (KHE_MONITOR) KheMatchingFirstCompetitor(KheSolnMatching(m->soln),
    (KHE_MATCHING_DEMAND_NODE) m);
}


/*****************************************************************************/
/*                                                                           */
/*  KHE_MONITOR KheMonitorNextCompetitor(KHE_MONITOR m)                      */
/*                                                                           */
/*  Return the next competitor of demand monitor m.                          */
/*                                                                           */
/*****************************************************************************/

KHE_MONITOR KheMonitorNextCompetitor(KHE_MONITOR m)
{
  MAssert(m->tag == KHE_ORDINARY_DEMAND_MONITOR_TAG ||
    m->tag == KHE_WORKLOAD_DEMAND_MONITOR_TAG,
    "KheMonitorNextCompetitor: m is not a demand monitor");
  MAssert(m->attached, "KheMonitorNextCompetitor: m is not attached");
  return (KHE_MONITOR) KheMatchingNextCompetitor(KheSolnMatching(m->soln));
}


/*****************************************************************************/
/*                                                                           */
/*  Submodule "deviations"                                                   */
/*                                                                           */
/*****************************************************************************/

/*****************************************************************************/
/*                                                                           */
/*  int KheMonitorDeviationCount(KHE_MONITOR m)                              */
/*                                                                           */
/*  Return the number of deviations of m.                                    */
/*                                                                           */
/*****************************************************************************/

int KheMonitorDeviationCount(KHE_MONITOR m)
{
  switch( m->tag )
  {
    case KHE_ASSIGN_RESOURCE_MONITOR_TAG:

      return KheAssignResourceMonitorDeviationCount(
	(KHE_ASSIGN_RESOURCE_MONITOR) m);

    case KHE_ASSIGN_TIME_MONITOR_TAG:

      return KheAssignTimeMonitorDeviationCount(
	(KHE_ASSIGN_TIME_MONITOR) m);

    case KHE_SPLIT_EVENTS_MONITOR_TAG:

      return KheSplitEventsMonitorDeviationCount(
	(KHE_SPLIT_EVENTS_MONITOR) m);

    case KHE_DISTRIBUTE_SPLIT_EVENTS_MONITOR_TAG:

      return KheDistributeSplitEventsMonitorDeviationCount(
	(KHE_DISTRIBUTE_SPLIT_EVENTS_MONITOR) m);

    case KHE_PREFER_RESOURCES_MONITOR_TAG:

      return KhePreferResourcesMonitorDeviationCount(
	(KHE_PREFER_RESOURCES_MONITOR) m);

    case KHE_PREFER_TIMES_MONITOR_TAG:

      return KhePreferTimesMonitorDeviationCount(
	(KHE_PREFER_TIMES_MONITOR) m);

    case KHE_AVOID_SPLIT_ASSIGNMENTS_MONITOR_TAG:

      return KheAvoidSplitAssignmentsMonitorDeviationCount(
	(KHE_AVOID_SPLIT_ASSIGNMENTS_MONITOR) m);

    case KHE_SPREAD_EVENTS_MONITOR_TAG:

      return KheSpreadEventsMonitorDeviationCount(
	(KHE_SPREAD_EVENTS_MONITOR) m);

    case KHE_LINK_EVENTS_MONITOR_TAG:

      return KheLinkEventsMonitorDeviationCount(
	(KHE_LINK_EVENTS_MONITOR) m);

    case KHE_AVOID_CLASHES_MONITOR_TAG:

      return KheAvoidClashesMonitorDeviationCount(
	(KHE_AVOID_CLASHES_MONITOR) m);

    case KHE_AVOID_UNAVAILABLE_TIMES_MONITOR_TAG:

      return KheAvoidUnavailableTimesMonitorDeviationCount(
	(KHE_AVOID_UNAVAILABLE_TIMES_MONITOR) m);

    case KHE_LIMIT_IDLE_TIMES_MONITOR_TAG:

      return KheLimitIdleTimesMonitorDeviationCount(
	(KHE_LIMIT_IDLE_TIMES_MONITOR) m);

    case KHE_CLUSTER_BUSY_TIMES_MONITOR_TAG:

      return KheClusterBusyTimesMonitorDeviationCount(
	(KHE_CLUSTER_BUSY_TIMES_MONITOR) m);

    case KHE_LIMIT_BUSY_TIMES_MONITOR_TAG:

      return KheLimitBusyTimesMonitorDeviationCount(
	(KHE_LIMIT_BUSY_TIMES_MONITOR) m);

    case KHE_LIMIT_WORKLOAD_MONITOR_TAG:

      return KheLimitWorkloadMonitorDeviationCount(
	(KHE_LIMIT_WORKLOAD_MONITOR) m);

    case KHE_TIMETABLE_MONITOR_TAG:

      return KheTimetableMonitorDeviationCount(
	(KHE_TIMETABLE_MONITOR) m);

    case KHE_TIME_GROUP_MONITOR_TAG:

      return KheTimeGroupMonitorDeviationCount(
	(KHE_TIME_GROUP_MONITOR) m);

    case KHE_GROUP_MONITOR_TAG:

      return KheGroupMonitorDeviationCount(
	(KHE_GROUP_MONITOR) m);

    case KHE_ORDINARY_DEMAND_MONITOR_TAG:

      return KheOrdinaryDemandMonitorDeviationCount(
	(KHE_ORDINARY_DEMAND_MONITOR) m);

    case KHE_WORKLOAD_DEMAND_MONITOR_TAG:

      return KheWorkloadDemandMonitorDeviationCount(
	(KHE_WORKLOAD_DEMAND_MONITOR) m);

    case KHE_EVENNESS_MONITOR_TAG:

      return KheEvennessMonitorDeviationCount(
	(KHE_EVENNESS_MONITOR) m);

    default:

      MAssert(false, "KheMonitorDeviationCount: invalid monitor type tag");
      return 0;  /* keep compiler happy */
  }
}


/*****************************************************************************/
/*                                                                           */
/*  int KheMonitorDeviation(KHE_MONITOR m, int i)                            */
/*                                                                           */
/*  Return the value of the i'th deviation of m.                             */
/*                                                                           */
/*****************************************************************************/

int KheMonitorDeviation(KHE_MONITOR m, int i)
{
  switch( m->tag )
  {
    case KHE_ASSIGN_RESOURCE_MONITOR_TAG:

      return KheAssignResourceMonitorDeviation(
	(KHE_ASSIGN_RESOURCE_MONITOR) m, i);

    case KHE_ASSIGN_TIME_MONITOR_TAG:

      return KheAssignTimeMonitorDeviation(
	(KHE_ASSIGN_TIME_MONITOR) m, i);

    case KHE_SPLIT_EVENTS_MONITOR_TAG:

      return KheSplitEventsMonitorDeviation(
	(KHE_SPLIT_EVENTS_MONITOR) m, i);

    case KHE_DISTRIBUTE_SPLIT_EVENTS_MONITOR_TAG:

      return KheDistributeSplitEventsMonitorDeviation(
	(KHE_DISTRIBUTE_SPLIT_EVENTS_MONITOR) m, i);

    case KHE_PREFER_RESOURCES_MONITOR_TAG:

      return KhePreferResourcesMonitorDeviation(
	(KHE_PREFER_RESOURCES_MONITOR) m, i);

    case KHE_PREFER_TIMES_MONITOR_TAG:

      return KhePreferTimesMonitorDeviation(
	(KHE_PREFER_TIMES_MONITOR) m, i);

    case KHE_AVOID_SPLIT_ASSIGNMENTS_MONITOR_TAG:

      return KheAvoidSplitAssignmentsMonitorDeviation(
	(KHE_AVOID_SPLIT_ASSIGNMENTS_MONITOR) m, i);

    case KHE_SPREAD_EVENTS_MONITOR_TAG:

      return KheSpreadEventsMonitorDeviation(
	(KHE_SPREAD_EVENTS_MONITOR) m, i);

    case KHE_LINK_EVENTS_MONITOR_TAG:

      return KheLinkEventsMonitorDeviation(
	(KHE_LINK_EVENTS_MONITOR) m, i);

    case KHE_AVOID_CLASHES_MONITOR_TAG:

      return KheAvoidClashesMonitorDeviation(
	(KHE_AVOID_CLASHES_MONITOR) m, i);

    case KHE_AVOID_UNAVAILABLE_TIMES_MONITOR_TAG:

      return KheAvoidUnavailableTimesMonitorDeviation(
	(KHE_AVOID_UNAVAILABLE_TIMES_MONITOR) m, i);

    case KHE_LIMIT_IDLE_TIMES_MONITOR_TAG:

      return KheLimitIdleTimesMonitorDeviation(
	(KHE_LIMIT_IDLE_TIMES_MONITOR) m, i);

    case KHE_CLUSTER_BUSY_TIMES_MONITOR_TAG:

      return KheClusterBusyTimesMonitorDeviation(
	(KHE_CLUSTER_BUSY_TIMES_MONITOR) m, i);

    case KHE_LIMIT_BUSY_TIMES_MONITOR_TAG:

      return KheLimitBusyTimesMonitorDeviation(
	(KHE_LIMIT_BUSY_TIMES_MONITOR) m, i);

    case KHE_LIMIT_WORKLOAD_MONITOR_TAG:

      return KheLimitWorkloadMonitorDeviation(
	(KHE_LIMIT_WORKLOAD_MONITOR) m, i);

    case KHE_TIMETABLE_MONITOR_TAG:

      return KheTimetableMonitorDeviation(
	(KHE_TIMETABLE_MONITOR) m, i);

    case KHE_TIME_GROUP_MONITOR_TAG:

      return KheTimeGroupMonitorDeviation(
	(KHE_TIME_GROUP_MONITOR) m, i);

    case KHE_GROUP_MONITOR_TAG:

      return KheGroupMonitorDeviation(
	(KHE_GROUP_MONITOR) m, i);

    case KHE_ORDINARY_DEMAND_MONITOR_TAG:

      return KheOrdinaryDemandMonitorDeviation(
	(KHE_ORDINARY_DEMAND_MONITOR) m, i);

    case KHE_WORKLOAD_DEMAND_MONITOR_TAG:

      return KheWorkloadDemandMonitorDeviation(
	(KHE_WORKLOAD_DEMAND_MONITOR) m, i);

    case KHE_EVENNESS_MONITOR_TAG:

      return KheEvennessMonitorDeviation(
	(KHE_EVENNESS_MONITOR) m, i);

    default:

      MAssert(false, "KheMonitorDeviation: invalid monitor type tag");
      return 0;  /* keep compiler happy */
  }
}


/*****************************************************************************/
/*                                                                           */
/*  char *KheMonitorDeviationDescription(KHE_MONITOR m, int i)               */
/*                                                                           */
/*  Return a description of the i'th deviation of m.                         */
/*                                                                           */
/*****************************************************************************/

char *KheMonitorDeviationDescription(KHE_MONITOR m, int i)
{
  switch( m->tag )
  {
    case KHE_ASSIGN_RESOURCE_MONITOR_TAG:

      return KheAssignResourceMonitorDeviationDescription(
	(KHE_ASSIGN_RESOURCE_MONITOR) m, i);

    case KHE_ASSIGN_TIME_MONITOR_TAG:

      return KheAssignTimeMonitorDeviationDescription(
	(KHE_ASSIGN_TIME_MONITOR) m, i);

    case KHE_SPLIT_EVENTS_MONITOR_TAG:

      return KheSplitEventsMonitorDeviationDescription(
	(KHE_SPLIT_EVENTS_MONITOR) m, i);

    case KHE_DISTRIBUTE_SPLIT_EVENTS_MONITOR_TAG:

      return KheDistributeSplitEventsMonitorDeviationDescription(
	(KHE_DISTRIBUTE_SPLIT_EVENTS_MONITOR) m, i);

    case KHE_PREFER_RESOURCES_MONITOR_TAG:

      return KhePreferResourcesMonitorDeviationDescription(
	(KHE_PREFER_RESOURCES_MONITOR) m, i);

    case KHE_PREFER_TIMES_MONITOR_TAG:

      return KhePreferTimesMonitorDeviationDescription(
	(KHE_PREFER_TIMES_MONITOR) m, i);

    case KHE_AVOID_SPLIT_ASSIGNMENTS_MONITOR_TAG:

      return KheAvoidSplitAssignmentsMonitorDeviationDescription(
	(KHE_AVOID_SPLIT_ASSIGNMENTS_MONITOR) m, i);

    case KHE_SPREAD_EVENTS_MONITOR_TAG:

      return KheSpreadEventsMonitorDeviationDescription(
	(KHE_SPREAD_EVENTS_MONITOR) m, i);

    case KHE_LINK_EVENTS_MONITOR_TAG:

      return KheLinkEventsMonitorDeviationDescription(
	(KHE_LINK_EVENTS_MONITOR) m, i);

    case KHE_AVOID_CLASHES_MONITOR_TAG:

      return KheAvoidClashesMonitorDeviationDescription(
	(KHE_AVOID_CLASHES_MONITOR) m, i);

    case KHE_AVOID_UNAVAILABLE_TIMES_MONITOR_TAG:

      return KheAvoidUnavailableTimesMonitorDeviationDescription(
	(KHE_AVOID_UNAVAILABLE_TIMES_MONITOR) m, i);

    case KHE_LIMIT_IDLE_TIMES_MONITOR_TAG:

      return KheLimitIdleTimesMonitorDeviationDescription(
	(KHE_LIMIT_IDLE_TIMES_MONITOR) m, i);

    case KHE_CLUSTER_BUSY_TIMES_MONITOR_TAG:

      return KheClusterBusyTimesMonitorDeviationDescription(
	(KHE_CLUSTER_BUSY_TIMES_MONITOR) m, i);

    case KHE_LIMIT_BUSY_TIMES_MONITOR_TAG:

      return KheLimitBusyTimesMonitorDeviationDescription(
	(KHE_LIMIT_BUSY_TIMES_MONITOR) m, i);

    case KHE_LIMIT_WORKLOAD_MONITOR_TAG:

      return KheLimitWorkloadMonitorDeviationDescription(
	(KHE_LIMIT_WORKLOAD_MONITOR) m, i);

    case KHE_TIMETABLE_MONITOR_TAG:

      return KheTimetableMonitorDeviationDescription(
	(KHE_TIMETABLE_MONITOR) m, i);

    case KHE_TIME_GROUP_MONITOR_TAG:

      return KheTimeGroupMonitorDeviationDescription(
	(KHE_TIME_GROUP_MONITOR) m, i);

    case KHE_GROUP_MONITOR_TAG:

      return KheGroupMonitorDeviationDescription(
	(KHE_GROUP_MONITOR) m, i);

    case KHE_ORDINARY_DEMAND_MONITOR_TAG:

      return KheOrdinaryDemandMonitorDeviationDescription(
	(KHE_ORDINARY_DEMAND_MONITOR) m, i);

    case KHE_WORKLOAD_DEMAND_MONITOR_TAG:

      return KheWorkloadDemandMonitorDeviationDescription(
	(KHE_WORKLOAD_DEMAND_MONITOR) m, i);

    case KHE_EVENNESS_MONITOR_TAG:

      return KheEvennessMonitorDeviationDescription(
	(KHE_EVENNESS_MONITOR) m, i);

    default:

      MAssert(false,"KheMonitorDeviationDescription: invalid monitor type tag");
      return NULL;  /* keep compiler happy */
  }
}


/*****************************************************************************/
/*                                                                           */
/*  Submodule "debug"                                                        */
/*                                                                           */
/*****************************************************************************/

/*****************************************************************************/
/*                                                                           */
/*  void KheMonitorDebug(KHE_MONITOR m, int verbosity, int indent, FILE *fp) */
/*                                                                           */
/*  Debug print of m onto fp with the given verbosity and indent.            */
/*                                                                           */
/*****************************************************************************/

void KheMonitorDebug(KHE_MONITOR m, int verbosity, int indent, FILE *fp)
{
  switch( m->tag )
  {
    case KHE_ASSIGN_RESOURCE_MONITOR_TAG:

      KheAssignResourceMonitorDebug((KHE_ASSIGN_RESOURCE_MONITOR) m,
	verbosity, indent, fp);
      break;

    case KHE_ASSIGN_TIME_MONITOR_TAG:

      KheAssignTimeMonitorDebug((KHE_ASSIGN_TIME_MONITOR) m,
	verbosity, indent, fp);
      break;

    case KHE_SPLIT_EVENTS_MONITOR_TAG:

      KheSplitEventsMonitorDebug((KHE_SPLIT_EVENTS_MONITOR) m,
	verbosity, indent, fp);
      break;

    case KHE_DISTRIBUTE_SPLIT_EVENTS_MONITOR_TAG:

      KheDistributeSplitEventsMonitorDebug(
	(KHE_DISTRIBUTE_SPLIT_EVENTS_MONITOR) m, verbosity, indent, fp);
      break;

    case KHE_PREFER_RESOURCES_MONITOR_TAG:

      KhePreferResourcesMonitorDebug((KHE_PREFER_RESOURCES_MONITOR) m,
	verbosity, indent, fp);
      break;

    case KHE_PREFER_TIMES_MONITOR_TAG:

      KhePreferTimesMonitorDebug((KHE_PREFER_TIMES_MONITOR) m,
	verbosity, indent, fp);
      break;

    case KHE_AVOID_SPLIT_ASSIGNMENTS_MONITOR_TAG:

      KheAvoidSplitAssignmentsMonitorDebug(
	(KHE_AVOID_SPLIT_ASSIGNMENTS_MONITOR) m, verbosity, indent, fp);
      break;

    case KHE_SPREAD_EVENTS_MONITOR_TAG:

      KheSpreadEventsMonitorDebug((KHE_SPREAD_EVENTS_MONITOR) m,
	verbosity, indent, fp);
      break;

    case KHE_LINK_EVENTS_MONITOR_TAG:

      KheLinkEventsMonitorDebug((KHE_LINK_EVENTS_MONITOR) m,
	verbosity, indent, fp);
      break;

    case KHE_AVOID_CLASHES_MONITOR_TAG:

      KheAvoidClashesMonitorDebug((KHE_AVOID_CLASHES_MONITOR) m,
	verbosity, indent, fp);
      break;

    case KHE_AVOID_UNAVAILABLE_TIMES_MONITOR_TAG:

      KheAvoidUnavailableTimesMonitorDebug(
	(KHE_AVOID_UNAVAILABLE_TIMES_MONITOR) m, verbosity, indent, fp);
      break;

    case KHE_LIMIT_IDLE_TIMES_MONITOR_TAG:

      KheLimitIdleTimesMonitorDebug((KHE_LIMIT_IDLE_TIMES_MONITOR) m,
	verbosity, indent, fp);
      break;

    case KHE_CLUSTER_BUSY_TIMES_MONITOR_TAG:

      KheClusterBusyTimesMonitorDebug((KHE_CLUSTER_BUSY_TIMES_MONITOR) m,
	verbosity, indent, fp);
      break;

    case KHE_LIMIT_BUSY_TIMES_MONITOR_TAG:

      KheLimitBusyTimesMonitorDebug((KHE_LIMIT_BUSY_TIMES_MONITOR) m,
	verbosity, indent, fp);
      break;

    case KHE_LIMIT_WORKLOAD_MONITOR_TAG:

      KheLimitWorkloadMonitorDebug((KHE_LIMIT_WORKLOAD_MONITOR) m,
	verbosity, indent, fp);
      break;

    case KHE_TIMETABLE_MONITOR_TAG:

      KheTimetableMonitorDebug((KHE_TIMETABLE_MONITOR) m, verbosity,indent,fp);
      break;

    case KHE_TIME_GROUP_MONITOR_TAG:

      KheTimeGroupMonitorDebug((KHE_TIME_GROUP_MONITOR) m,
	verbosity, indent, fp);
      break;

    case KHE_GROUP_MONITOR_TAG:

      KheGroupMonitorDebug((KHE_GROUP_MONITOR) m,
	verbosity, indent, fp);
      break;

    case KHE_ORDINARY_DEMAND_MONITOR_TAG:

      KheOrdinaryDemandMonitorDebug((KHE_ORDINARY_DEMAND_MONITOR) m,
	verbosity, indent, fp);
      break;

    case KHE_WORKLOAD_DEMAND_MONITOR_TAG:

      KheWorkloadDemandMonitorDebug((KHE_WORKLOAD_DEMAND_MONITOR) m,
	verbosity, indent, fp);
      break;

    case KHE_EVENNESS_MONITOR_TAG:

      KheEvennessMonitorDebug((KHE_EVENNESS_MONITOR) m,
	verbosity, indent, fp);
      break;

    default:

      MAssert(false, "KheMonitorDebug: invalid monitor type tag");
  }
}


/*****************************************************************************/
/*                                                                           */
/*  char *KheMonitorTagShow(KHE_MONITOR_TAG tag)                             */
/*                                                                           */
/*  Return a string version of tag.                                          */
/*                                                                           */
/*****************************************************************************/

char *KheMonitorTagShow(KHE_MONITOR_TAG tag)
{
  switch( tag )
  {
    case KHE_ASSIGN_RESOURCE_MONITOR_TAG:

      return "AssignResourceMonitor";

    case KHE_ASSIGN_TIME_MONITOR_TAG:

      return "AssignTimeMonitor";

    case KHE_SPLIT_EVENTS_MONITOR_TAG:

      return "SplitEventsMonitor";

    case KHE_DISTRIBUTE_SPLIT_EVENTS_MONITOR_TAG:

      return "DistributeSplitEventsMonitor";

    case KHE_PREFER_RESOURCES_MONITOR_TAG:

      return "PreferResourcesMonitor";

    case KHE_PREFER_TIMES_MONITOR_TAG:

      return "PreferTimesMonitor";

    case KHE_AVOID_SPLIT_ASSIGNMENTS_MONITOR_TAG:

      return "AvoidSplitAssignmentsMonitor";

    case KHE_SPREAD_EVENTS_MONITOR_TAG:

      return "SpreadEventsMonitor";

    case KHE_LINK_EVENTS_MONITOR_TAG:

      return "LinkEventsMonitor";

    case KHE_AVOID_CLASHES_MONITOR_TAG:

      return "AvoidClashesMonitor";

    case KHE_AVOID_UNAVAILABLE_TIMES_MONITOR_TAG:

      return "AvoidUnavailableTimesMonitor";

    case KHE_LIMIT_IDLE_TIMES_MONITOR_TAG:

      return "LimitIdleTimesMonitor";

    case KHE_CLUSTER_BUSY_TIMES_MONITOR_TAG:

      return "ClusterBusyTimesMonitor";

    case KHE_LIMIT_BUSY_TIMES_MONITOR_TAG:

      return "LimitBusyTimesMonitor";

    case KHE_LIMIT_WORKLOAD_MONITOR_TAG:

      return "LimitWorkloadMonitor";

    case KHE_TIMETABLE_MONITOR_TAG:

      return "TimetableMonitor";

    case KHE_TIME_GROUP_MONITOR_TAG:

      return "TimeGroupMonitor";

    case KHE_GROUP_MONITOR_TAG:

      return "GroupMonitor";

    case KHE_ORDINARY_DEMAND_MONITOR_TAG:

      return "OrdinaryDemandMonitor";

    case KHE_WORKLOAD_DEMAND_MONITOR_TAG:

      return "WorkloadDemandMonitor";

    case KHE_EVENNESS_MONITOR_TAG:

      return "EvennessMonitor";

    default:

      MAssert(false, "KheMonitorTagShow: invalid monitor type tag %d", tag);
      return "??";  /* keep compiler happy */
  }
}


/*****************************************************************************/
/*                                                                           */
/*  char *KheMonitorLabel(KHE_MONITOR m)                                     */
/*                                                                           */
/*  Return an identifying label for m:  its tag if it is not a group         */
/*  monitor, and its subtag label if it is.                                  */
/*                                                                           */
/*****************************************************************************/

char *KheMonitorLabel(KHE_MONITOR m)
{
  if( m->tag == KHE_GROUP_MONITOR_TAG )
    return KheGroupMonitorSubTagLabel((KHE_GROUP_MONITOR) m);
  else
    return KheMonitorTagShow(KheMonitorTag(m));
}


/*****************************************************************************/
/*                                                                           */
/*  bool KheMonitorIsLinkedToSoln(KHE_MONITOR m)                             */
/*                                                                           */
/*  Return true if m is linked, directly or indirectly, to its soln.         */
/*                                                                           */
/*****************************************************************************/

static bool KheMonitorIsLinkedToSoln(KHE_MONITOR m)
{
  KHE_SOLN soln;
  soln = m->soln;
  while( m->parent_monitor != NULL )
    m = (KHE_MONITOR) m->parent_monitor;
  return m == (KHE_MONITOR) soln;
}


/*****************************************************************************/
/*                                                                           */
/*  void KheMonitorDebugWithTagBegin(KHE_MONITOR m, char *tag, int indent,   */
/*    FILE *fp)                                                              */
/*  void KheMonitorDebugBegin(KHE_MONITOR m, int indent, FILE *fp)           */
/*                                                                           */
/*  Begin a debug print of m onto fp.   KheMonitorDebugWithTagBegin allows   */
/*  the caller to specify the tag at the start; KheMonitorDebugBegin uses    */
/*  a string value of the monitor's tag to supply this tag.                  */
/*                                                                           */
/*****************************************************************************/

void KheMonitorDebugWithTagBegin(KHE_MONITOR m, char *tag, int indent, FILE *fp)
{
  if( indent >= 0 )
    fprintf(fp, "%*s", indent, "");
  fprintf(fp, "[ %c%c %-35s %10.4f",
    KheMonitorTag(m) == KHE_GROUP_MONITOR_TAG ?
    'G' : KheMonitorAttachedToSoln(m) ? 'A' : 'D',
    m->parent_monitor == NULL ? '_' : KheMonitorIsLinkedToSoln(m) ? 'S' : 'U',
    tag, KheCostShow(m->cost));
}

void KheMonitorDebugBegin(KHE_MONITOR m, int indent, FILE *fp)
{
  KheMonitorDebugWithTagBegin(m, KheMonitorTagShow(m->tag), indent, fp);
}


/*****************************************************************************/
/*                                                                           */
/*  void KheMonitorDebugEnd(KHE_MONITOR m, bool single_line,                 */
/*    int indent, FILE *fp)                                                  */
/*                                                                           */
/*  End a debug print of m onto fp.  If single_line is true, the entire      */
/*  debug has been on a single line, so the concluding ] can go on that      */
/*  same line.                                                               */
/*                                                                           */
/*****************************************************************************/

void KheMonitorDebugEnd(KHE_MONITOR m, bool single_line,
  int indent, FILE *fp)
{
  if( indent < 0 )
    fprintf(fp, " ]");
  else if ( single_line )
    fprintf(fp, " ]\n");
  else
    fprintf(fp, "%*s]\n", indent, "");
}
