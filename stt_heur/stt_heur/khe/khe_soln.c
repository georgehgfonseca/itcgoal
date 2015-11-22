
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
/*  FILE:         khe_soln.c                                                 */
/*  DESCRIPTION:  A solution                                                 */
/*                                                                           */
/*****************************************************************************/
#include "khe_interns.h"
#define DEBUG1 0
#define DEBUG2 0
#define DEBUG3 0
#define DEBUG4 0
#define DEBUG5 0
#define DEBUG6 0
#define DEBUG7 0
#define DEBUG8 0
#define DEBUG9 0
#define DEBUG10 0
#define DEBUG11 0
#define DEBUG12 0
#define DEBUG13 0
#define DEBUG14 0
#define DEBUG15 0


/*****************************************************************************/
/*                                                                           */
/*  KHE_SOLN - a solution                                                    */
/*                                                                           */
/*  Attributes all_time_groups, all_resource_groups, all_event_groups,       */
/*  all_time_domains, and free_supply_chunks are private and must remain     */
/*  so, because they denote things that are created specifically within      */
/*  this solution and are not to be copied.                                  */
/*                                                                           */
/*****************************************************************************/

struct khe_soln_rec {

  /* inherited from KHE_GROUP_MONITOR */
  KHE_SOLN			soln;			/* encl. solution    */
  int				index_in_soln;		/* not used here     */
  unsigned char			tag;			/* tag field         */
  bool				attached;		/* true if attached  */
  void				*back;			/* back pointer      */
  KHE_GROUP_MONITOR		parent_monitor;		/* parent monitor    */
  int				parent_index;		/* index in parent   */
  int				defect_index;		/* defect index      */
  int				trace_num;		/* trace visit num   */
  KHE_COST			trace_cost;		/* at start of trace */
  KHE_COST			cost;			/* current cost      */
  ARRAY_KHE_MONITOR		child_monitors;		/* child monitors    */
  ARRAY_KHE_MONITOR		defects;		/* defects           */
  ARRAY_KHE_MONITOR		defects_copy;		/* copy of defects   */
  ARRAY_KHE_TRACE		traces;			/* traces            */
  int				sub_tag;		/* sub tag           */
  char				*sub_tag_label;		/* sub tag label     */

  /* specific to KHE_SOLN */
  KHE_INSTANCE			instance;		/* instance solved   */
  KHE_SOLN_GROUP		soln_group;		/* optional soln grp */
  KHE_EVENNESS_HANDLER		evenness_handler;	/* evenness handler  */
  ARRAY_KHE_TRACE		free_traces;		/* free list         */
  ARRAY_KHE_TRANSACTION		free_transactions;	/* free list of t's  */
  ARRAY_KHE_TRANSACTION		curr_transactions;	/* current trans's   */
  KHE_TIME_GROUP		curr_time_group;	/* temp variable     */
  KHE_RESOURCE_GROUP		curr_resource_group;	/* temp variable     */
  KHE_EVENT_GROUP		curr_event_group;	/* temp variable     */
  ARRAY_KHE_TIME_GROUP		all_time_groups;	/* all created ones  */
  ARRAY_KHE_RESOURCE_GROUP	all_resource_groups;	/* all created ones  */
  ARRAY_KHE_EVENT_GROUP		all_event_groups;	/* all created ones  */
  ARRAY_KHE_RESOURCE_IN_SOLN	resources_in_soln;	/* res. monitors     */
  ARRAY_KHE_EVENT_IN_SOLN	events_in_soln;		/* event monitors    */
  ARRAY_KHE_MONITOR		monitors;		/* all monitors      */
  ARRAY_KHE_MEET		meets;			/* meets             */
  ARRAY_KHE_MEET		time_to_cycle_meet;	/* maps time to c.m. */
  ARRAY_INT			time_to_cycle_offset;	/* maps time to c.o. */
  ARRAY_KHE_TIME_GROUP		packing_time_groups;	/* packing time grps */
  ARRAY_KHE_NODE		nodes;			/* layer tree nodes  */
  ARRAY_KHE_TASK		tasks;			/* tasks             */
  ARRAY_KHE_TASK		free_tasks;		/* free tasks        */
  ARRAY_KHE_TASKING		taskings;		/* taskings          */
  ARRAY_KHE_TASKING		free_taskings;		/* free taskings     */
  KHE_MATCHING_TYPE		matching_type;		/* matching type     */
  KHE_COST			matching_weight;	/* weight of matching*/
  KHE_MATCHING			matching;		/* the matching      */
  ARRAY_KHE_MATCHING_SUPPLY_CHUNK matching_free_supply_chunks;	/* free list */
  ARRAY_SHORT			matching_zero_domain;	/* domain { 0 }      */
  int				diversifier;		/* diversifier       */
  int				visit_num;		/* visit number      */
  KHE_SOLN			copy;			/* used when copying */
};


/*****************************************************************************/
/*                                                                           */
/*  Submodule "construction and query"                                       */
/*                                                                           */
/*****************************************************************************/

/*****************************************************************************/
/*                                                                           */
/*  void KheSolnAddInitialCycleMeet(KHE_SOLN soln)                           */
/*                                                                           */
/*  Add the initial cycle meet to soln, that is, if there is at least one    */
/*  time in the instance.                                                    */
/*                                                                           */
/*****************************************************************************/

static void KheSolnAddInitialCycleMeet(KHE_SOLN soln)
{
  KHE_INSTANCE ins;  KHE_TIME t;  KHE_MEET meet;  int i;
  ins = KheSolnInstance(soln);
  if( KheInstanceTimeCount(ins) > 0 )
  {
    /* add the meet, setting various fields appropriately for a cycle meet */
    meet = KheMeetMake(soln, KheInstanceTimeCount(ins), NULL);
    t = KheInstanceTime(ins, 0);
    KheMeetSetDomain(meet, KheTimeSingletonTimeGroup(t));
    KheMeetSetAssignedTimeIndex(meet, KheTimeIndex(t));

    /* initialize time_to_cycle_meet and time_to_cycle_offset */
    for( i = 0;  i < KheMeetDuration(meet);  i++ )
    {
      MArrayAddLast(soln->time_to_cycle_meet, meet);
      MArrayAddLast(soln->time_to_cycle_offset, i);
    }
  }
}


/*****************************************************************************/
/*                                                                           */
/*  void KheSolnMakeCycleMeet(KHE_SOLN soln, KHE_TIME start_time,            */
/*    KHE_TIME end_time)                                                     */
/*                                                                           */
/*  Make a cycle meet covering the times from start_time to end_time.        */
/*                                                                           */
/*****************************************************************************/

/* ***
static void KheSolnMakeCycleMeet(KHE_SOLN soln, KHE_TIME start_time,
  KHE_TIME end_time)
{
  KHE_MEET meet;
  if( DEBUG4 )
    fprintf(stderr, "[ KheSolnMakeCycleMeet(soln %p, %s-%s)\n", (void *) soln,
      KheTimeId(start_time) != NULL ? KheTimeId(start_time) : "-",
      KheTimeId(end_time) != NULL ? KheTimeId(end_time) : "-");
  meet = KheMeetMake(soln,
    KheTimeIndex(end_time) - KheTimeIndex(start_time) + 1, NULL);
  KheMeetSetDomain(meet, KheTimeSingletonTimeGroup(start_time));
  KheMeetSetAssignedTimeIndex(meet, KheTimeIndex(start_time));
  if( DEBUG4 )
    fprintf(stderr, "] KheSolnMakeCycleMeet returning\n");
}
*** */


/*****************************************************************************/
/*                                                                           */
/*  void KheSolnAddCycleMeets(KHE_SOLN soln)                                 */
/*                                                                           */
/*  Add cycle meets to soln.                                                 */
/*                                                                           */
/*****************************************************************************/

/* ***
static void KheSolnAddCycleMeets(KHE_SOLN soln)
{
  int i;  KHE_INSTANCE ins;  KHE_TIME t, start_time;
  if( DEBUG4 )
    fprintf(stderr, "[ KheSolnAddCycleMeets(soln %p)\n", (void *) soln);
  ins = KheSolnInstance(soln);
  start_time = NULL;
  for( i = 0;  i < KheInstanceTimeCount(ins);  i++ )
  {
    t = KheInstanceTime(ins, i);
    if( DEBUG4 )
      fprintf(stderr, "  time %s%s\n",
	KheTimeId(t) != NULL ? KheTimeId(t) : "-",
	KheTimeBreakAfter(t) ? " (break_after)" : "");
    if( start_time == NULL )
      start_time = t;
    if( KheTimeBreakAfter(t) || i == KheInstanceTimeCount(ins) - 1 )
    {
      KheSolnMakeCycleMeet(soln, start_time, t);
      start_time = NULL;
    }
  }
  if( DEBUG4 )
    fprintf(stderr, "] KheSolnAddCycleMeets returning\n");
}
*** */


/*****************************************************************************/
/*                                                                           */
/*  void KheSolnAddCycleTasks(KHE_SOLN soln)                                 */
/*                                                                           */
/*  Add the cycle tasks to the soln.                                         */
/*                                                                           */
/*****************************************************************************/

static void KheSolnAddCycleTasks(KHE_SOLN soln)
{
  int i;  KHE_RESOURCE r;
  for( i = 0;  i < KheInstanceResourceCount(soln->instance);  i++ )
  {
    r = KheInstanceResource(soln->instance, i);
    KheCycleTaskMake(soln, r);
  }
}


/*****************************************************************************/
/*                                                                           */
/*  void KheSolnMakeAndAttachAssignResourceConstraintMonitors(               */
/*    KHE_SOLN soln, KHE_ASSIGN_RESOURCE_CONSTRAINT c)                       */
/*                                                                           */
/*  Make and attach the monitors for this constraint.                        */
/*                                                                           */
/*****************************************************************************/

static void KheSolnMakeAndAttachAssignResourceConstraintMonitors(
  KHE_SOLN soln, KHE_ASSIGN_RESOURCE_CONSTRAINT c)
{
  int i;  KHE_EVENT_RESOURCE er;  KHE_EVENT_IN_SOLN es;
  KHE_EVENT_RESOURCE_IN_SOLN ers;  KHE_ASSIGN_RESOURCE_MONITOR m;
  for( i = 0;  i < KheAssignResourceConstraintEventResourceCount(c);  i++ )
  {
    er = KheAssignResourceConstraintEventResource(c, i);
    es = MArrayGet(soln->events_in_soln,
      KheEventIndex(KheEventResourceEvent(er)));
    ers = KheEventInSolnEventResourceInSoln(es,
      KheEventResourceIndexInEvent(er));
    m = KheAssignResourceMonitorMake(ers, c);
    if( DEBUG11 )
      fprintf(stderr, "new KheAssignResourceMonitor %p (attached %d)\n",
	(void *) m, (int) KheMonitorAttachedToSoln((KHE_MONITOR) m));
    KheMonitorAttachToSoln((KHE_MONITOR) m);
    KheGroupMonitorAddChildMonitor((KHE_GROUP_MONITOR) soln, (KHE_MONITOR) m);
  }
}


/*****************************************************************************/
/*                                                                           */
/*  void KheSolnMakeAndAttachAssignTimeConstraintMonitors(                   */
/*    KHE_SOLN soln, KHE_ASSIGN_TIME_CONSTRAINT c)                           */
/*                                                                           */
/*  Make and attach the monitors for this constraint.                        */
/*                                                                           */
/*****************************************************************************/

static void KheSolnMakeAndAttachAssignTimeConstraintMonitors(
  KHE_SOLN soln, KHE_ASSIGN_TIME_CONSTRAINT c)
{
  int i, j;  KHE_EVENT_GROUP eg;  KHE_EVENT e;
  KHE_EVENT_IN_SOLN es;  KHE_ASSIGN_TIME_MONITOR m;
  for( i = 0;  i < KheAssignTimeConstraintEventCount(c);  i++ )
  {
    e = KheAssignTimeConstraintEvent(c, i);
    es = MArrayGet(soln->events_in_soln, KheEventIndex(e));
    m = KheAssignTimeMonitorMake(es, c);
    KheMonitorAttachToSoln((KHE_MONITOR) m);
    KheGroupMonitorAddChildMonitor((KHE_GROUP_MONITOR) soln, (KHE_MONITOR) m);
  }
  for( i = 0;  i < KheAssignTimeConstraintEventGroupCount(c);  i++ )
  {
    eg = KheAssignTimeConstraintEventGroup(c, i);
    for( j = 0;  j < KheEventGroupEventCount(eg);  j++ )
    {
      e = KheEventGroupEvent(eg, j);
      es = MArrayGet(soln->events_in_soln, KheEventIndex(e));
      m = KheAssignTimeMonitorMake(es, c);
      KheMonitorAttachToSoln((KHE_MONITOR) m);
      KheGroupMonitorAddChildMonitor((KHE_GROUP_MONITOR) soln, (KHE_MONITOR) m);
    }
  }
}


/*****************************************************************************/
/*                                                                           */
/*  void KheSolnMakeAndAttachSplitEventsConstraintMonitors(                  */
/*    KHE_SOLN soln, KHE_SPLIT_EVENTS_CONSTRAINT c)                          */
/*                                                                           */
/*  Make and attach the monitors for this constraint.                        */
/*                                                                           */
/*****************************************************************************/

static void KheSolnMakeAndAttachSplitEventsConstraintMonitors(
  KHE_SOLN soln, KHE_SPLIT_EVENTS_CONSTRAINT c)
{
  int i, j;  KHE_EVENT_GROUP eg;  KHE_EVENT e;
  KHE_EVENT_IN_SOLN es;  KHE_SPLIT_EVENTS_MONITOR m;
  if( DEBUG12 )
    fprintf(stderr, "[ KheSolnMakeAndAttachSplitEventsConstraintMonitors()\n");
  for( i = 0;  i < KheSplitEventsConstraintEventCount(c);  i++ )
  {
    e = KheSplitEventsConstraintEvent(c, i);
    if( DEBUG12 )
    {
      fprintf(stderr, "  event %d ", KheEventIndex(e));
      KheEventDebug(e, 1, 0, stderr);
    }
    es = MArrayGet(soln->events_in_soln, KheEventIndex(e));
    m = KheSplitEventsMonitorMake(es, c);
    KheMonitorAttachToSoln((KHE_MONITOR) m);
    KheGroupMonitorAddChildMonitor((KHE_GROUP_MONITOR) soln, (KHE_MONITOR) m);
  }
  for( i = 0;  i < KheSplitEventsConstraintEventGroupCount(c);  i++ )
  {
    eg = KheSplitEventsConstraintEventGroup(c, i);
    /* ***
    if( DEBUG12 )
    {
      fprintf(stderr, "  event group ");
      KheEventGroupDebug(e, 1, 0, stderr);
    }
    *** */
    for( j = 0;  j < KheEventGroupEventCount(eg);  j++ )
    {
      e = KheEventGroupEvent(eg, j);
      if( DEBUG12 )
      {
	fprintf(stderr, "    event group event %d ", KheEventIndex(e));
	KheEventDebug(e, 1, 0, stderr);
      }
      es = MArrayGet(soln->events_in_soln, KheEventIndex(e));
      m = KheSplitEventsMonitorMake(es, c);
      KheMonitorAttachToSoln((KHE_MONITOR) m);
      KheGroupMonitorAddChildMonitor((KHE_GROUP_MONITOR) soln, (KHE_MONITOR) m);
    }
  }
  if( DEBUG12 )
    fprintf(stderr, "] KheSolnMakeAndAttachSplitEventsConstraintMonitors()\n");
}


/*****************************************************************************/
/*                                                                           */
/*  void KheSolnMakeAndAttachDistributeSplitEventsConstraintMonitors(        */
/*    KHE_SOLN soln, KHE_DISTRIBUTE_SPLIT_EVENTS_CONSTRAINT c)               */
/*                                                                           */
/*  Make and attach the monitors for this constraint.                        */
/*                                                                           */
/*****************************************************************************/

static void KheSolnMakeAndAttachDistributeSplitEventsConstraintMonitors(
  KHE_SOLN soln, KHE_DISTRIBUTE_SPLIT_EVENTS_CONSTRAINT c)
{
  int i, j;  KHE_EVENT_GROUP eg;  KHE_EVENT e;
  KHE_EVENT_IN_SOLN es;  KHE_DISTRIBUTE_SPLIT_EVENTS_MONITOR m;
  for( i = 0;  i < KheDistributeSplitEventsConstraintEventCount(c);  i++ )
  {
    e = KheDistributeSplitEventsConstraintEvent(c, i);
    es = MArrayGet(soln->events_in_soln, KheEventIndex(e));
    m = KheDistributeSplitEventsMonitorMake(es, c);
    KheMonitorAttachToSoln((KHE_MONITOR) m);
    KheGroupMonitorAddChildMonitor((KHE_GROUP_MONITOR) soln, (KHE_MONITOR) m);
  }
  for( i = 0;  i < KheDistributeSplitEventsConstraintEventGroupCount(c);  i++ )
  {
    eg = KheDistributeSplitEventsConstraintEventGroup(c, i);
    for( j = 0;  j < KheEventGroupEventCount(eg);  j++ )
    {
      e = KheEventGroupEvent(eg, j);
      es = MArrayGet(soln->events_in_soln, KheEventIndex(e));
      m = KheDistributeSplitEventsMonitorMake(es, c);
      KheMonitorAttachToSoln((KHE_MONITOR) m);
      KheGroupMonitorAddChildMonitor((KHE_GROUP_MONITOR) soln, (KHE_MONITOR) m);
    }
  }
}


/*****************************************************************************/
/*                                                                           */
/*  void KheSolnMakeAndAttachPreferResourcesConstraintMonitors(              */
/*    KHE_SOLN soln, KHE_PREFER_RESOURCES_CONSTRAINT c)                      */
/*                                                                           */
/*  Make and attach the monitors for this constraint.                        */
/*                                                                           */
/*****************************************************************************/

static void KheSolnMakeAndAttachPreferResourcesConstraintMonitors(
  KHE_SOLN soln, KHE_PREFER_RESOURCES_CONSTRAINT c)
{
  int i;  KHE_EVENT_RESOURCE er;
  KHE_EVENT_IN_SOLN es;  KHE_EVENT_RESOURCE_IN_SOLN ers;
  KHE_PREFER_RESOURCES_MONITOR m;
  for( i = 0;  i < KhePreferResourcesConstraintEventResourceCount(c);  i++ )
  {
    er = KhePreferResourcesConstraintEventResource(c, i);
    es = MArrayGet(soln->events_in_soln,
      KheEventIndex(KheEventResourceEvent(er)));
    ers = KheEventInSolnEventResourceInSoln(es,
      KheEventResourceIndexInEvent(er));
    m = KhePreferResourcesMonitorMake(ers, c);
    KheMonitorAttachToSoln((KHE_MONITOR) m);
    KheGroupMonitorAddChildMonitor((KHE_GROUP_MONITOR) soln, (KHE_MONITOR) m);
  }
}


/*****************************************************************************/
/*                                                                           */
/*  void KheSolnMakeAndAttachPreferTimesConstraintMonitors(                  */
/*    KHE_SOLN soln, KHE_PREFER_TIMES_CONSTRAINT c)                          */
/*                                                                           */
/*  Make and attach the monitors for this constraint.                        */
/*                                                                           */
/*****************************************************************************/

static void KheSolnMakeAndAttachPreferTimesConstraintMonitors(
  KHE_SOLN soln, KHE_PREFER_TIMES_CONSTRAINT c)
{
  int i, j;  KHE_EVENT_GROUP eg;  KHE_EVENT e;
  KHE_EVENT_IN_SOLN es;  KHE_PREFER_TIMES_MONITOR m;
  for( i = 0;  i < KhePreferTimesConstraintEventCount(c);  i++ )
  {
    e = KhePreferTimesConstraintEvent(c, i);
    es = MArrayGet(soln->events_in_soln, KheEventIndex(e));
    m = KhePreferTimesMonitorMake(es, c);
    KheMonitorAttachToSoln((KHE_MONITOR) m);
    KheGroupMonitorAddChildMonitor((KHE_GROUP_MONITOR) soln, (KHE_MONITOR) m);
  }
  for( i = 0;  i < KhePreferTimesConstraintEventGroupCount(c);  i++ )
  {
    eg = KhePreferTimesConstraintEventGroup(c, i);
    for( j = 0;  j < KheEventGroupEventCount(eg);  j++ )
    {
      e = KheEventGroupEvent(eg, j);
      es = MArrayGet(soln->events_in_soln, KheEventIndex(e));
      m = KhePreferTimesMonitorMake(es, c);
      KheMonitorAttachToSoln((KHE_MONITOR) m);
      KheGroupMonitorAddChildMonitor((KHE_GROUP_MONITOR) soln, (KHE_MONITOR) m);
    }
  }
}


/*****************************************************************************/
/*                                                                           */
/*  void KheSolnMakeAndAttachAvoidSplitAssignmentsConstraintMonitors(        */
/*    KHE_SOLN soln, KHE_AVOID_SPLIT_ASSIGNMENTS_CONSTRAINT c)               */
/*                                                                           */
/*  Make and attach the monitors for this constraint.                        */
/*                                                                           */
/*****************************************************************************/

static void KheSolnMakeAndAttachAvoidSplitAssignmentsConstraintMonitors(
  KHE_SOLN soln, KHE_AVOID_SPLIT_ASSIGNMENTS_CONSTRAINT c)
{
  int i;  KHE_AVOID_SPLIT_ASSIGNMENTS_MONITOR m;
  for( i = 0;  i < KheAvoidSplitAssignmentsConstraintEventGroupCount(c);  i++ )
  {
    m = KheAvoidSplitAssignmentsMonitorMake(soln, c, i);
    KheMonitorAttachToSoln((KHE_MONITOR) m);
    KheGroupMonitorAddChildMonitor((KHE_GROUP_MONITOR) soln, (KHE_MONITOR) m);
  }
}


/*****************************************************************************/
/*                                                                           */
/*  void KheSolnMakeAndAttachSpreadEventsConstraintMonitors(                 */
/*    KHE_SOLN soln, KHE_SPREAD_EVENTS_CONSTRAINT c)                         */
/*                                                                           */
/*  Make and attach the monitors for this constraint.                        */
/*                                                                           */
/*****************************************************************************/

static void KheSolnMakeAndAttachSpreadEventsConstraintMonitors(
  KHE_SOLN soln, KHE_SPREAD_EVENTS_CONSTRAINT c)
{
  int i;  KHE_EVENT_GROUP eg;  KHE_SPREAD_EVENTS_MONITOR m;
  for( i = 0;  i < KheSpreadEventsConstraintEventGroupCount(c);  i++ )
  {
    eg = KheSpreadEventsConstraintEventGroup(c, i);
    m = KheSpreadEventsMonitorMake(soln, c, eg);
    KheMonitorAttachToSoln((KHE_MONITOR) m);
    KheGroupMonitorAddChildMonitor((KHE_GROUP_MONITOR) soln, (KHE_MONITOR) m);
  }
}


/*****************************************************************************/
/*                                                                           */
/*  void KheSolnMakeAndAttachLinkEventsConstraintMonitors(                   */
/*    KHE_SOLN soln, KHE_LINK_EVENTS_CONSTRAINT c)                           */
/*                                                                           */
/*  Make and attach the monitors for this constraint.                        */
/*                                                                           */
/*****************************************************************************/

static void KheSolnMakeAndAttachLinkEventsConstraintMonitors(
  KHE_SOLN soln, KHE_LINK_EVENTS_CONSTRAINT c)
{
  int i;  KHE_EVENT_GROUP eg;  KHE_LINK_EVENTS_MONITOR m;
  for( i = 0;  i < KheLinkEventsConstraintEventGroupCount(c);  i++ )
  {
    eg = KheLinkEventsConstraintEventGroup(c, i);
    m = KheLinkEventsMonitorMake(soln, c, eg);
    KheMonitorAttachToSoln((KHE_MONITOR) m);
    KheGroupMonitorAddChildMonitor((KHE_GROUP_MONITOR) soln, (KHE_MONITOR) m);
  }
}


/*****************************************************************************/
/*                                                                           */
/*  void KheSolnMakeAndAttachAvoidClashesConstraintMonitors(                 */
/*    KHE_SOLN soln, KHE_AVOID_CLASHES_CONSTRAINT c)                         */
/*                                                                           */
/*  Make and attach the monitors for this constraint.                        */
/*                                                                           */
/*****************************************************************************/

static void KheSolnMakeAndAttachAvoidClashesConstraintMonitors(
  KHE_SOLN soln, KHE_AVOID_CLASHES_CONSTRAINT c)
{
  int i, j;  KHE_RESOURCE_GROUP rg;  KHE_RESOURCE r;  KHE_RESOURCE_IN_SOLN rs;
  KHE_AVOID_CLASHES_MONITOR m;
  for( i = 0;  i < KheAvoidClashesConstraintResourceGroupCount(c);  i++ )
  {
    rg = KheAvoidClashesConstraintResourceGroup(c, i);
    for( j = 0;  j < KheResourceGroupResourceCount(rg);  j++ )
    {
      r = KheResourceGroupResource(rg, j);
      rs = KheSolnResourceInSoln(soln, KheResourceIndexInInstance(r));
      m = KheAvoidClashesMonitorMake(rs, c);
      KheMonitorAttachToSoln((KHE_MONITOR) m);
      KheGroupMonitorAddChildMonitor((KHE_GROUP_MONITOR) soln, (KHE_MONITOR) m);
    }
  }
  for( i = 0;  i < KheAvoidClashesConstraintResourceCount(c);  i++ )
  {
    r = KheAvoidClashesConstraintResource(c, i);
    rs = KheSolnResourceInSoln(soln, KheResourceIndexInInstance(r));
    m = KheAvoidClashesMonitorMake(rs, c);
    KheMonitorAttachToSoln((KHE_MONITOR) m);
    KheGroupMonitorAddChildMonitor((KHE_GROUP_MONITOR) soln, (KHE_MONITOR) m);
  }
}


/*****************************************************************************/
/*                                                                           */
/*  void KheSolnMakeAndAttachAvoidUnavailableTimesConstraintMonitors(        */
/*    KHE_SOLN soln, KHE_AVOID_UNAVAILABLE_TIMES_CONSTRAINT c)               */
/*                                                                           */
/*  Make and attach the monitors for this constraint.                        */
/*                                                                           */
/*****************************************************************************/

static void KheSolnMakeAndAttachAvoidUnavailableTimesConstraintMonitors(
  KHE_SOLN soln, KHE_AVOID_UNAVAILABLE_TIMES_CONSTRAINT c)
{
  int i, j;  KHE_RESOURCE_GROUP rg;  KHE_RESOURCE r;  KHE_RESOURCE_IN_SOLN rs;
  KHE_AVOID_UNAVAILABLE_TIMES_MONITOR m;
  for( i=0; i < KheAvoidUnavailableTimesConstraintResourceGroupCount(c); i++ )
  {
    rg = KheAvoidUnavailableTimesConstraintResourceGroup(c, i);
    for( j = 0;  j < KheResourceGroupResourceCount(rg);  j++ )
    {
      r = KheResourceGroupResource(rg, j);
      rs = KheSolnResourceInSoln(soln, KheResourceIndexInInstance(r));
      m = KheAvoidUnavailableTimesMonitorMake(rs, c);
      KheMonitorAttachToSoln((KHE_MONITOR) m);
      KheGroupMonitorAddChildMonitor((KHE_GROUP_MONITOR) soln, (KHE_MONITOR) m);
    }
  }
  for( i = 0;  i < KheAvoidUnavailableTimesConstraintResourceCount(c);  i++ )
  {
    r = KheAvoidUnavailableTimesConstraintResource(c, i);
    rs = KheSolnResourceInSoln(soln, KheResourceIndexInInstance(r));
    m = KheAvoidUnavailableTimesMonitorMake(rs, c);
    KheMonitorAttachToSoln((KHE_MONITOR) m);
    KheGroupMonitorAddChildMonitor((KHE_GROUP_MONITOR) soln, (KHE_MONITOR) m);
  }
}


/*****************************************************************************/
/*                                                                           */
/*  void KheSolnMakeAndAttachLimitIdleTimesConstraintMonitors(               */
/*    KHE_SOLN soln, KHE_LIMIT_IDLE_TIMES_CONSTRAINT c)                      */
/*                                                                           */
/*  Make and attach the monitors for this constraint.                        */
/*                                                                           */
/*****************************************************************************/

static void KheSolnMakeAndAttachLimitIdleTimesConstraintMonitors(
  KHE_SOLN soln, KHE_LIMIT_IDLE_TIMES_CONSTRAINT c)
{
  int i, j;  KHE_RESOURCE_GROUP rg;  KHE_RESOURCE r;  KHE_RESOURCE_IN_SOLN rs;
  KHE_LIMIT_IDLE_TIMES_MONITOR m;
  for( i = 0;  i < KheLimitIdleTimesConstraintResourceGroupCount(c);  i++ )
  {
    rg = KheLimitIdleTimesConstraintResourceGroup(c, i);
    for( j = 0;  j < KheResourceGroupResourceCount(rg);  j++ )
    {
      r = KheResourceGroupResource(rg, j);
      rs = KheSolnResourceInSoln(soln, KheResourceIndexInInstance(r));
      m = KheLimitIdleTimesMonitorMake(rs, c);
      KheMonitorAttachToSoln((KHE_MONITOR) m);
      KheGroupMonitorAddChildMonitor((KHE_GROUP_MONITOR) soln, (KHE_MONITOR) m);
    }
  }
  for( i = 0;  i < KheLimitIdleTimesConstraintResourceCount(c);  i++ )
  {
    r = KheLimitIdleTimesConstraintResource(c, i);
    rs = KheSolnResourceInSoln(soln, KheResourceIndexInInstance(r));
    m = KheLimitIdleTimesMonitorMake(rs, c);
    KheMonitorAttachToSoln((KHE_MONITOR) m);
    KheGroupMonitorAddChildMonitor((KHE_GROUP_MONITOR) soln, (KHE_MONITOR) m);
  }
}


/*****************************************************************************/
/*                                                                           */
/*  void KheSolnMakeAndAttachClusterBusyTimesConstraintMonitors(             */
/*    KHE_SOLN soln, KHE_CLUSTER_BUSY_TIMES_CONSTRAINT c)                    */
/*                                                                           */
/*  Make and attach the monitors for this constraint.                        */
/*                                                                           */
/*****************************************************************************/

static void KheSolnMakeAndAttachClusterBusyTimesConstraintMonitors(
  KHE_SOLN soln, KHE_CLUSTER_BUSY_TIMES_CONSTRAINT c)
{
  int i, j;  KHE_RESOURCE_GROUP rg;  KHE_RESOURCE r;  KHE_RESOURCE_IN_SOLN rs;
  KHE_CLUSTER_BUSY_TIMES_MONITOR m;
  for( i = 0;  i < KheClusterBusyTimesConstraintResourceGroupCount(c);  i++ )
  {
    rg = KheClusterBusyTimesConstraintResourceGroup(c, i);
    for( j = 0;  j < KheResourceGroupResourceCount(rg);  j++ )
    {
      r = KheResourceGroupResource(rg, j);
      rs = KheSolnResourceInSoln(soln, KheResourceIndexInInstance(r));
      m = KheClusterBusyTimesMonitorMake(rs, c);
      KheMonitorAttachToSoln((KHE_MONITOR) m);
      KheGroupMonitorAddChildMonitor((KHE_GROUP_MONITOR) soln, (KHE_MONITOR) m);
    }
  }
  for( i = 0;  i < KheClusterBusyTimesConstraintResourceCount(c);  i++ )
  {
    r = KheClusterBusyTimesConstraintResource(c, i);
    rs = KheSolnResourceInSoln(soln, KheResourceIndexInInstance(r));
    m = KheClusterBusyTimesMonitorMake(rs, c);
    KheMonitorAttachToSoln((KHE_MONITOR) m);
    KheGroupMonitorAddChildMonitor((KHE_GROUP_MONITOR) soln, (KHE_MONITOR) m);
  }
}


/*****************************************************************************/
/*                                                                           */
/*  void KheSolnMakeAndAttachLimitBusyTimesConstraintMonitors(               */
/*    KHE_SOLN soln, KHE_LIMIT_BUSY_TIMES_CONSTRAINT c)                      */
/*                                                                           */
/*  Make and attach the monitors for this constraint.                        */
/*                                                                           */
/*****************************************************************************/

static void KheSolnMakeAndAttachLimitBusyTimesConstraintMonitors(
  KHE_SOLN soln, KHE_LIMIT_BUSY_TIMES_CONSTRAINT c)
{
  int i, j;  KHE_RESOURCE_GROUP rg;  KHE_RESOURCE r;  KHE_RESOURCE_IN_SOLN rs;
  KHE_LIMIT_BUSY_TIMES_MONITOR m;
  for( i = 0;  i < KheLimitBusyTimesConstraintResourceGroupCount(c);  i++ )
  {
    rg = KheLimitBusyTimesConstraintResourceGroup(c, i);
    for( j = 0;  j < KheResourceGroupResourceCount(rg);  j++ )
    {
      r = KheResourceGroupResource(rg, j);
      rs = KheSolnResourceInSoln(soln, KheResourceIndexInInstance(r));
      m = KheLimitBusyTimesMonitorMake(rs, c);
      KheMonitorAttachToSoln((KHE_MONITOR) m);
      KheGroupMonitorAddChildMonitor((KHE_GROUP_MONITOR) soln, (KHE_MONITOR) m);
    }
  }
  for( i = 0;  i < KheLimitBusyTimesConstraintResourceCount(c);  i++ )
  {
    r = KheLimitBusyTimesConstraintResource(c, i);
    rs = KheSolnResourceInSoln(soln, KheResourceIndexInInstance(r));
    m = KheLimitBusyTimesMonitorMake(rs, c);
    KheMonitorAttachToSoln((KHE_MONITOR) m);
    KheGroupMonitorAddChildMonitor((KHE_GROUP_MONITOR) soln, (KHE_MONITOR) m);
  }
}


/*****************************************************************************/
/*                                                                           */
/*  void KheSolnMakeAndAttachLimitWorkloadConstraintMonitors(                */
/*    KHE_SOLN soln, KHE_LIMIT_WORKLOAD_CONSTRAINT c)                        */
/*                                                                           */
/*  Make and attach the monitors for this constraint.                        */
/*                                                                           */
/*****************************************************************************/

static void KheSolnMakeAndAttachLimitWorkloadConstraintMonitors(
  KHE_SOLN soln, KHE_LIMIT_WORKLOAD_CONSTRAINT c)
{
  int i, j;  KHE_RESOURCE_GROUP rg;  KHE_RESOURCE r;  KHE_RESOURCE_IN_SOLN rs;
  KHE_LIMIT_WORKLOAD_MONITOR m;
  for( i = 0;  i < KheLimitWorkloadConstraintResourceGroupCount(c);  i++ )
  {
    rg = KheLimitWorkloadConstraintResourceGroup(c, i);
    for( j = 0;  j < KheResourceGroupResourceCount(rg);  j++ )
    {
      r = KheResourceGroupResource(rg, j);
      rs = KheSolnResourceInSoln(soln, KheResourceIndexInInstance(r));
      m = KheLimitWorkloadMonitorMake(rs, c);
      KheMonitorAttachToSoln((KHE_MONITOR) m);
      KheGroupMonitorAddChildMonitor((KHE_GROUP_MONITOR) soln, (KHE_MONITOR) m);
    }
  }
  for( i = 0;  i < KheLimitWorkloadConstraintResourceCount(c);  i++ )
  {
    r = KheLimitWorkloadConstraintResource(c, i);
    rs = KheSolnResourceInSoln(soln, KheResourceIndexInInstance(r));
    m = KheLimitWorkloadMonitorMake(rs, c);
    KheMonitorAttachToSoln((KHE_MONITOR) m);
    KheGroupMonitorAddChildMonitor((KHE_GROUP_MONITOR) soln, (KHE_MONITOR) m);
  }
}


/*****************************************************************************/
/*                                                                           */
/*  void KheSolnMakeAndAttachConstraintMonitors(KHE_SOLN soln)               */
/*                                                                           */
/*  Make and attach the constraint monitors of soln.                         */
/*                                                                           */
/*****************************************************************************/

static void KheSolnMakeAndAttachConstraintMonitors(KHE_SOLN soln)
{
  int i;  KHE_CONSTRAINT c;  KHE_INSTANCE ins;
  ins = KheSolnInstance(soln);
  for( i = 0;  i < KheInstanceConstraintCount(ins);  i++ )
  {
    c = KheInstanceConstraint(ins, i);
    switch( KheConstraintTag(c) )
    {
      case KHE_ASSIGN_RESOURCE_CONSTRAINT_TAG:

	KheSolnMakeAndAttachAssignResourceConstraintMonitors(soln,
	  (KHE_ASSIGN_RESOURCE_CONSTRAINT) c);
	break;

      case KHE_ASSIGN_TIME_CONSTRAINT_TAG:

	KheSolnMakeAndAttachAssignTimeConstraintMonitors(soln,
	  (KHE_ASSIGN_TIME_CONSTRAINT) c);
	break;

      case KHE_SPLIT_EVENTS_CONSTRAINT_TAG:

	KheSolnMakeAndAttachSplitEventsConstraintMonitors(soln,
	  (KHE_SPLIT_EVENTS_CONSTRAINT) c);
	break;

      case KHE_DISTRIBUTE_SPLIT_EVENTS_CONSTRAINT_TAG:

	KheSolnMakeAndAttachDistributeSplitEventsConstraintMonitors(soln,
	  (KHE_DISTRIBUTE_SPLIT_EVENTS_CONSTRAINT) c);
	break;

      case KHE_PREFER_RESOURCES_CONSTRAINT_TAG:

	KheSolnMakeAndAttachPreferResourcesConstraintMonitors(soln,
	  (KHE_PREFER_RESOURCES_CONSTRAINT) c);
	break;

      case KHE_PREFER_TIMES_CONSTRAINT_TAG:

	KheSolnMakeAndAttachPreferTimesConstraintMonitors(soln,
	  (KHE_PREFER_TIMES_CONSTRAINT) c);
	break;

      case KHE_AVOID_SPLIT_ASSIGNMENTS_CONSTRAINT_TAG:

	KheSolnMakeAndAttachAvoidSplitAssignmentsConstraintMonitors(soln,
	  (KHE_AVOID_SPLIT_ASSIGNMENTS_CONSTRAINT) c);
	break;

      case KHE_SPREAD_EVENTS_CONSTRAINT_TAG:

	KheSolnMakeAndAttachSpreadEventsConstraintMonitors(soln,
	  (KHE_SPREAD_EVENTS_CONSTRAINT) c);
	break;

      case KHE_LINK_EVENTS_CONSTRAINT_TAG:

	KheSolnMakeAndAttachLinkEventsConstraintMonitors(soln,
	  (KHE_LINK_EVENTS_CONSTRAINT) c);
	break;

      case KHE_AVOID_CLASHES_CONSTRAINT_TAG:

	KheSolnMakeAndAttachAvoidClashesConstraintMonitors(soln,
	  (KHE_AVOID_CLASHES_CONSTRAINT) c);
	break;

      case KHE_AVOID_UNAVAILABLE_TIMES_CONSTRAINT_TAG:

	KheSolnMakeAndAttachAvoidUnavailableTimesConstraintMonitors(soln,
	  (KHE_AVOID_UNAVAILABLE_TIMES_CONSTRAINT) c);
	break;

      case KHE_LIMIT_IDLE_TIMES_CONSTRAINT_TAG:

	KheSolnMakeAndAttachLimitIdleTimesConstraintMonitors(soln,
	  (KHE_LIMIT_IDLE_TIMES_CONSTRAINT) c);
	break;

      case KHE_CLUSTER_BUSY_TIMES_CONSTRAINT_TAG:

	KheSolnMakeAndAttachClusterBusyTimesConstraintMonitors(soln,
	  (KHE_CLUSTER_BUSY_TIMES_CONSTRAINT) c);
	break;

      case KHE_LIMIT_BUSY_TIMES_CONSTRAINT_TAG:

	KheSolnMakeAndAttachLimitBusyTimesConstraintMonitors(soln,
	  (KHE_LIMIT_BUSY_TIMES_CONSTRAINT) c);
	break;

      case KHE_LIMIT_WORKLOAD_CONSTRAINT_TAG:

	KheSolnMakeAndAttachLimitWorkloadConstraintMonitors(soln,
	  (KHE_LIMIT_WORKLOAD_CONSTRAINT) c);
	break;

      default:

	MAssert(false, 
	  "KheSolnMakeAndAttachConstraintMonitors illegal constraint tag");
	break;
    }
  }
}


/*****************************************************************************/
/*                                                                           */
/*  void KheSolnSetSolnGroup(KHE_SOLN soln, KHE_SOLN_GROUP soln_group)       */
/*                                                                           */
/*  Set the soln_group attribute of soln (internal use only).                */
/*                                                                           */
/*****************************************************************************/

void KheSolnSetSolnGroup(KHE_SOLN soln, KHE_SOLN_GROUP soln_group)
{
  soln->soln_group = soln_group;
}


/*****************************************************************************/
/*                                                                           */
/*  KHE_SOLN KheSolnMake(KHE_INSTANCE ins, KHE_SOLN_GROUP soln_group)        */
/*                                                                           */
/*  Make and return a new soln of instance, with no meets, no nodes, and     */
/*  no layers.  Parameter soln_group is optional.                            */
/*                                                                           */
/*****************************************************************************/

KHE_SOLN KheSolnMake(KHE_INSTANCE ins, KHE_SOLN_GROUP soln_group)
{
  KHE_SOLN res;  int i;  KHE_EVENT e;  KHE_RESOURCE r;
  KHE_EVENT_IN_SOLN es;  KHE_RESOURCE_IN_SOLN rs;

  if( DEBUG1 )
    fprintf(stderr, "[ KheSolnMake(%s)\n",
      KheInstanceId(ins) != NULL ? KheInstanceId(ins) : "-");
  MAssert(KheInstanceComplete(ins),
    "KheSolnMake called before KheInstanceMakeEnd");
  /* ***
  MAssert(sizeof(int) < sizeof(lo ng),
    "KheSolnMake: no difference between sizeof(int) and sizeof(lo ng)");
  *** */

  /* attributes inherited from KHE_GROUP_MONITOR */
  MMake(res);
  MArrayInit(res->monitors); /* must put this early! */
  KheMonitorInitCommonFields((KHE_MONITOR) res, res, KHE_GROUP_MONITOR_TAG);
  MArrayInit(res->child_monitors);
  MArrayInit(res->defects);
  MArrayInit(res->defects_copy);
  MArrayInit(res->traces);
  res->sub_tag = -1;
  res->sub_tag_label = "Soln";

  /* instance and solution group */
  res->instance = ins;
  if( soln_group != NULL )
    KheSolnGroupAddSoln(soln_group, res);
  else
    res->soln_group = NULL;

  /* evenness handler */
  res->evenness_handler = KheEvennessHandlerMake(res);

  /* traces and transactions */
  MArrayInit(res->free_traces);
  MArrayInit(res->free_transactions);
  MArrayInit(res->curr_transactions);

  /* group and domain construction variables */
  res->curr_time_group = NULL;
  res->curr_resource_group = NULL;
  res->curr_event_group = NULL;
  /* res->curr_time_domain = NULL; */
  MArrayInit(res->all_time_groups);
  MArrayInit(res->all_resource_groups);
  MArrayInit(res->all_event_groups);

  /* resource in soln objects, one per resource */
  MArrayInit(res->resources_in_soln);
  for( i = 0;  i < KheInstanceResourceCount(ins);  i++ )
  {
    r = KheInstanceResource(ins, i);
    rs = KheResourceInSolnMake(res, r);
    MArrayAddLast(res->resources_in_soln, rs);
  }

  /* event in soln objects, one per instance event */
  MArrayInit(res->events_in_soln);
  for( i = 0;  i < KheInstanceEventCount(ins);  i++ )
  {
    e = KheInstanceEvent(ins, i);
    es = KheEventInSolnMake(res, KheInstanceEvent(ins, i));
    MArrayAddLast(res->events_in_soln, es);
  }

  /* meets */
  MArrayInit(res->meets);

  /* cycle meet stuff */
  MArrayInit(res->time_to_cycle_meet);
  MArrayInit(res->time_to_cycle_offset);
  MArrayInit(res->packing_time_groups);

  /* nodes */
  MArrayInit(res->nodes);

  /* tasks and taskings */
  MArrayInit(res->tasks);
  MArrayInit(res->free_tasks);
  MArrayInit(res->taskings);
  MArrayInit(res->free_taskings);

  /* matching */
  /*  res->matching_monitor = KheMatchingMonitorMake(res); */
  res->matching_type = KHE_MATCHING_TYPE_SOLVE;
  res->matching_weight = 0;
  res->matching = KheMatchingMake(res);
  MArrayInit(res->matching_free_supply_chunks);
  MArrayInit(res->matching_zero_domain);
  MArrayAddLast(res->matching_zero_domain, 0);

  /* cycle meets and tasks */
  /* KheSolnAddCycleMeets(res); */
  KheSolnAddInitialCycleMeet(res);
  KheSolnAddCycleTasks(res);

  /* diversifier, visit_num and copy */
  res->diversifier = 0;
  res->visit_num = 0;
  res->copy = NULL;

  /* make and attach constraint monitors */
  KheSolnMakeAndAttachConstraintMonitors(res);

  if( DEBUG1 )
    fprintf(stderr, "] KheSolnMake returning\n");
  MAssert(res->instance != NULL, "KheSolnMake internal error");
  return res;
}


/*****************************************************************************/
/*                                                                           */
/*  void KheSolnConsistencyCheck(KHE_SOLN soln)                              */
/*                                                                           */
/*  Consistency check, written specifically to track down a memory bug.      */
/*                                                                           */
/*****************************************************************************/

/* ***
void KheSolnConsistencyCheck(KHE_SOLN soln)
{
  KHE_EVENT_IN_SOLN es;  int i;
  MArrayForEach(soln->events_in_soln, &es, &i)
    KheEventInSolnConsistencyCheck(es);
}
*** */


/*****************************************************************************/
/*                                                                           */
/*  void KheSolnDelete(KHE_SOLN soln)                                        */
/*                                                                           */
/*  Delete soln.                                                             */
/*                                                                           */
/*  Implementation note.  Deletion requires a plan.  We must ensure that     */
/*  every object to be deleted is deleted exactly once, and we must do it    */
/*  in a way that re-uses user-accessible operations for deleting parts of   */
/*  a solution.  This plan shows which deletion functions delete what.       */
/*                                                                           */
/*    KheSolnDelete (public)                                                 */
/*      KheLayerDelete (public, deletes only the layer)                      */
/*      KheMeetDelete (public)                                          */
/*        KheSolnResourceDelete (public)                                     */
/*          KheOrdinaryDemandMonitorDelete (private)                         */
/*        KheMatchingDemandChunkDelete (private, only when no nodes)         */
/*      KheNodeDelete (public, deletes only the node)                        */
/*      KheTimeDomainCacheDelete (private)                                   */
/*      KheEventInSolnDelete (private, does not free any monitors)           */
/*        KheEventResourceInSolnDelete (private, doesn't free mons/chunks)   */
/*      KheResourceInSolnDelete (private, doesn't free monitors or chunks)   */
/*        KheResourceInSolnWorkloadRequirementDelete (private)               */
/*      KheTimeDomainFree (private, doesn't free any elements)               */
/*      KheEventGroupDelete (private, doesn't free any elements)             */
/*      KheResourceGroupDelete (private, doesn't free any elements)          */
/*      KheTimeGroupDelete (private, doesn't free any elements)              */
/*      KheMonitorDelete (private) + its many redefs in child classes        */
/*      KheOrdinaryDemandMonitorDelete (private)                             */
/*      KheWorkloadDemandMonitorDelete (private)                             */
/*      KheTransactionFree (deletes free transactions, user deletes others)  */
/*      KheGroupMonitorDelete (public, deletes just the one monitor)         */
/*                                                                           */
/*  NB the following functions are not deletions in the present sense,       */
/*  because they move their objects to free lists rather than freeing them.  */
/*                                                                           */
/*      KheTransactionDelete (public)                                        */
/*      KheSolnMatchingAddOrdinarySupplyChunkToFreeList                      */
/*                                                                           */
/*****************************************************************************/

void KheSolnDelete(KHE_SOLN soln)
{
  KHE_MONITOR m;  KHE_TASKING tasking;  KHE_TASK task;  KHE_MEET meet;
  KHE_NODE node;
  if( DEBUG9 )
  {
    fprintf(stderr, "[ KheSolnDelete(");
    KheSolnDebug(soln, 1, -1, stderr);
    fprintf(stderr, ")\n");
  }

  /***********************************************************************/
  /*                                                                     */
  /*  First phase, in which the normal solution invariant is maintained  */
  /*                                                                     */
  /***********************************************************************/

  /* delete from enclosing soln group, if any */
  if( soln->soln_group != NULL )
    KheSolnGroupDeleteSoln(soln->soln_group, soln);

  /* delete taskings and tasks */
  while( MArraySize(soln->taskings) > 0 )
  {
    tasking = MArrayLast(soln->taskings);
    MAssert(KheTaskingSoln(tasking) == soln, "KheSolnDelete internal error 1");
    KheTaskingDelete(tasking);
  }
  while( MArraySize(soln->tasks) > 0 )
  {
    task = MArrayLast(soln->tasks);
    MAssert(KheTaskSoln(task) == soln, "KheSolnDelete internal error 2");
    KheTaskDelete(task);
  }

  /* delete meets */
  while( MArraySize(soln->meets) > 0 )
  {
    meet = MArrayLast(soln->meets);
    MAssert(KheMeetSoln(meet) == soln, "KheSolnDelete internal error 3");
    KheMeetDelete(meet);
  }

  /* delete nodes (NB meets must be deleted before nodes, else nodes fail) */
  while( MArraySize(soln->nodes) > 0 )
  {
    node = MArrayLast(soln->nodes);
    MAssert(KheNodeSoln(node) == soln, "KheSolnDelete internal error 4");
    if( !KheNodeDelete(node) )
      MAssert(false, "KheSolnDelete internal error 5");
  }

  /* delete monitors (these will remove themselves from the array) */
  while( MArraySize(soln->monitors) > 1 )  /* 1 for soln itself */
  {
    m = MArrayLast(soln->monitors);
    MAssert(KheMonitorSoln(m) == soln, "KheSolnDelete internal error 6");
    if( m != (KHE_MONITOR) soln )
      KheMonitorDelete(m);
  }


  /***********************************************************************/
  /*                                                                     */
  /*  Second phase, in which the solution invariant is not maintained    */
  /*  (working up the object's list of attributes)                       */
  /*                                                                     */
  /***********************************************************************/

  /* free tasks and taskings */
  while( MArraySize(soln->free_taskings) > 0 )
    KheTaskingFree(MArrayRemoveLast(soln->free_taskings));
  MArrayFree(soln->free_taskings);
  MArrayFree(soln->taskings);
  while( MArraySize(soln->free_tasks) > 0 )
    KheTaskFree(MArrayRemoveLast(soln->free_tasks));
  MArrayFree(soln->free_tasks);
  MArrayFree(soln->tasks);

  /* free the layer, node, and meet arrays (elements done separately) */
  MArrayFree(soln->nodes);
  MArrayFree(soln->meets);
  MArrayFree(soln->time_to_cycle_meet);
  MArrayFree(soln->time_to_cycle_offset);
  MArrayFree(soln->packing_time_groups);
  MArrayFree(soln->monitors);

  /* delete event in soln objects */
  while( MArraySize(soln->events_in_soln) > 0 )
    KheEventInSolnDelete(MArrayRemoveLast(soln->events_in_soln));
  MArrayFree(soln->events_in_soln);

  /* delete resource in soln objects */
  while( MArraySize(soln->resources_in_soln) > 0 )
    KheResourceInSolnDelete(MArrayRemoveLast(soln->resources_in_soln));
  MArrayFree(soln->resources_in_soln);

  /* delete event groups */
  while( MArraySize(soln->all_event_groups) > 0 )
    KheEventGroupDelete(MArrayRemoveLast(soln->all_event_groups));
  MArrayFree(soln->all_event_groups);

  /* delete resource groups */
  while( MArraySize(soln->all_resource_groups) > 0 )
    KheResourceGroupDelete(MArrayRemoveLast(soln->all_resource_groups));
  MArrayFree(soln->all_resource_groups);

  /* delete time groups */
  while( MArraySize(soln->all_time_groups) > 0 )
    KheTimeGroupDelete(MArrayRemoveLast(soln->all_time_groups));
  MArrayFree(soln->all_time_groups);

  /* delete free traces (user must delete others) */
  while( MArraySize(soln->free_traces) > 0 )
    KheTraceFree(MArrayRemoveLast(soln->free_traces));
  MArrayFree(soln->free_traces);

  /* delete free transactions (user must delete others) */
  while( MArraySize(soln->free_transactions) > 0 )
    KheTransactionFree(MArrayRemoveLast(soln->free_transactions));
  MArrayFree(soln->free_transactions);
  MArrayFree(soln->curr_transactions);

  /* delete monitors */
  if( DEBUG14 && MArraySize(soln->child_monitors) > 0 )
  {
    int i;
    MArrayForEach(soln->child_monitors, &m, &i)
      KheMonitorDebug(m, 1, 2, stderr);
  }
  MAssert(MArraySize(soln->child_monitors) == 0,
    "KheSolnDelete internal error 7 (%d child monitors)",
    MArraySize(soln->child_monitors));
  MArrayFree(soln->child_monitors);

  /* check traces */
  MAssert(MArraySize(soln->traces) == 0,
    "KheSolnDelete:  soln is currently being traced");
  MArrayFree(soln->traces);

  /* delete the matching (should be no demand nodes or chunks left by now) */
  KheMatchingDelete(soln->matching);
  MArrayFree(soln->matching_free_supply_chunks);

  /* delete evenness handler */
  KheEvennessHandlerDelete(soln->evenness_handler);

  MFree(soln);

  if( DEBUG9 )
    fprintf(stderr, "] KheSolnDelete returning\n");
}


/*****************************************************************************/
/*                                                                           */
/*  void KheSolnSetBack(KHE_SOLN soln, void *back)                           */
/*                                                                           */
/*  Set the back pointer of soln.                                            */
/*                                                                           */
/*****************************************************************************/

void KheSolnSetBack(KHE_SOLN soln, void *back)
{
  soln->back = back;
}


/*****************************************************************************/
/*                                                                           */
/*  void *KheSolnBack(KHE_SOLN soln)                                         */
/*                                                                           */
/*  Return the back pointer of soln.                                         */
/*                                                                           */
/*****************************************************************************/

void *KheSolnBack(KHE_SOLN soln)
{
  return soln->back;
}


/*****************************************************************************/
/*                                                                           */
/*  KHE_SOLN KheSolnCopyPhase1(KHE_SOLN soln)                                */
/*                                                                           */
/*  Carry out Phase 1 of the copying of soln.                                */
/*                                                                           */
/*****************************************************************************/

KHE_SOLN KheSolnCopyPhase1(KHE_SOLN soln)
{
  int i;  KHE_SOLN copy;  KHE_EVENT_IN_SOLN es;  KHE_RESOURCE_IN_SOLN rs;
  KHE_MEET meet;  KHE_NODE node;  KHE_MONITOR m;
  KHE_MATCHING_SUPPLY_CHUNK sc;  KHE_TASK task;  KHE_TASKING tasking;
  if( soln->copy == NULL )
  {
    /* inherited from KHE_GROUP_MONITOR */
    MAssert(MArraySize(soln->traces) == 0,
      "KheSolnCopy cannot copy:  soln is currently being traced");
    MMake(copy);
    if( DEBUG13 )
  
        fprintf(stderr, "[ %p = KheSolnCopyPhase1(%p)\n", (void *) copy,
	(void *) soln);
    soln->copy = copy;
    KheMonitorCopyCommonFields((KHE_MONITOR) copy, (KHE_MONITOR) soln);
    MArrayInit(copy->child_monitors);
    MArrayForEach(soln->child_monitors, &m, &i)
      MArrayAddLast(copy->child_monitors, KheMonitorCopyPhase1(m));
    MArrayInit(copy->defects);
    MArrayForEach(soln->defects, &m, &i)
      MArrayAddLast(copy->defects, KheMonitorCopyPhase1(m));
    MArrayInit(copy->defects_copy);
    MArrayForEach(soln->defects_copy, &m, &i)
      MArrayAddLast(copy->defects_copy, KheMonitorCopyPhase1(m));
    MArrayInit(copy->traces);
    copy->sub_tag = soln->sub_tag;
    copy->sub_tag_label = soln->sub_tag_label;

    /* specific to KHE_SOLN */
    copy->instance = soln->instance;
    copy->soln_group = NULL;
    copy->evenness_handler =
      KheEvennessHandlerCopyPhase1(soln->evenness_handler);
    MArrayInit(copy->free_traces);
    MArrayInit(copy->free_transactions);
    MArrayInit(copy->curr_transactions);
    copy->curr_time_group = NULL;
    copy->curr_resource_group = NULL;
    copy->curr_event_group = NULL;
    MArrayInit(copy->all_time_groups);      /* don't copy these ones! */
    MArrayInit(copy->all_resource_groups);  /* don't copy these ones! */
    MArrayInit(copy->all_event_groups);     /* don't copy these ones! */
    MArrayInit(copy->resources_in_soln);
    MArrayForEach(soln->resources_in_soln, &rs, &i)
      MArrayAddLast(copy->resources_in_soln, KheResourceInSolnCopyPhase1(rs));
    MArrayInit(copy->events_in_soln);
    MArrayForEach(soln->events_in_soln, &es, &i)
      MArrayAddLast(copy->events_in_soln, KheEventInSolnCopyPhase1(es));
    MArrayInit(copy->monitors);
    MArrayForEach(soln->monitors, &m, &i)
      MArrayAddLast(copy->monitors, KheMonitorCopyPhase1(m));
    MArrayInit(copy->meets);
    MArrayForEach(soln->meets, &meet, &i)
      MArrayAddLast(copy->meets, KheMeetCopyPhase1(meet));
    MArrayInit(copy->time_to_cycle_meet);
    MArrayForEach(soln->time_to_cycle_meet, &meet, &i)
      MArrayAddLast(copy->time_to_cycle_meet, KheMeetCopyPhase1(meet));
    MArrayInit(copy->time_to_cycle_offset);
    MArrayAppend(copy->time_to_cycle_offset, soln->time_to_cycle_offset, i);
    MArrayInit(copy->packing_time_groups);
    MArrayAppend(copy->packing_time_groups, soln->packing_time_groups, i);
    MArrayInit(copy->nodes);
    MArrayForEach(soln->nodes, &node, &i)
      MArrayAddLast(copy->nodes, KheNodeCopyPhase1(node));
    MArrayInit(copy->tasks);
    MArrayForEach(soln->tasks, &task, &i)
      MArrayAddLast(copy->tasks, KheTaskCopyPhase1(task));
    MArrayInit(copy->free_tasks);
    MArrayInit(copy->taskings);
    MArrayForEach(soln->taskings, &tasking, &i)
      MArrayAddLast(copy->taskings, KheTaskingCopyPhase1(tasking));
    MArrayInit(copy->free_taskings);
    copy->matching_type = soln->matching_type;
    copy->matching_weight = soln->matching_weight;
    copy->matching = KheMatchingCopyPhase1(soln->matching);
    MArrayInit(copy->matching_free_supply_chunks);
    MArrayForEach(soln->matching_free_supply_chunks, &sc, &i)
      MArrayAddLast(copy->matching_free_supply_chunks,
	KheMatchingSupplyChunkCopyPhase1(sc));
    MArrayInit(copy->matching_zero_domain);
    MArrayAddLast(copy->matching_zero_domain, 0);
    copy->diversifier = soln->diversifier;
    copy->visit_num = soln->visit_num;
    copy->copy = NULL;
    if( DEBUG13 )
      fprintf(stderr, "] KheSolnCopyPhase1 returning\n");
  }
  return soln->copy;
}


/*****************************************************************************/
/*                                                                           */
/*  void KheSolnCopyPhase2(KHE_SOLN soln)                                    */
/*                                                                           */
/*  Carry out Phase 2 of the copying of soln.                                */
/*                                                                           */
/*****************************************************************************/

void KheSolnCopyPhase2(KHE_SOLN soln)
{
  int i;  KHE_EVENT_IN_SOLN es;  KHE_RESOURCE_IN_SOLN rs;  KHE_MEET meet;
  KHE_NODE node;  KHE_MONITOR m;  KHE_TASK task;  KHE_TASKING tasking;
  KHE_MATCHING_SUPPLY_CHUNK sc;
  if( soln->copy != NULL )
  {
    soln->copy = NULL;
    MArrayForEach(soln->child_monitors, &m, &i)
      KheMonitorCopyPhase2(m);
    KheEvennessHandlerCopyPhase2(soln->evenness_handler);
    MArrayForEach(soln->resources_in_soln, &rs, &i)
      KheResourceInSolnCopyPhase2(rs);
    MArrayForEach(soln->events_in_soln, &es, &i)
      KheEventInSolnCopyPhase2(es);
    MArrayForEach(soln->monitors, &m, &i)
      KheMonitorCopyPhase2(m);
    MArrayForEach(soln->meets, &meet, &i)
      KheMeetCopyPhase2(meet);
    MArrayForEach(soln->nodes, &node, &i)
      KheNodeCopyPhase2(node);
    MArrayForEach(soln->tasks, &task, &i)
      KheTaskCopyPhase2(task);
    MArrayForEach(soln->taskings, &tasking, &i)
      KheTaskingCopyPhase2(tasking);
    MArrayForEach(soln->matching_free_supply_chunks, &sc, &i)
      KheMatchingSupplyChunkCopyPhase2(sc);
  }
}


/*****************************************************************************/
/*                                                                           */
/*  KHE_SOLN KheSolnCopy(KHE_SOLN soln)                                      */
/*                                                                           */
/*  Make a deep copy of soln.                                                */
/*                                                                           */
/*****************************************************************************/

KHE_SOLN KheSolnCopy(KHE_SOLN soln)
{
  KHE_SOLN copy;

  /* probabilistic check for re-entrant call */
  MAssert(soln->copy == NULL, "re-entrant call on KheSolnCopy");

  /* check temporary group construction variables */
  MAssert(soln->curr_time_group == NULL,
    "KheSolnCopy called while time group under construction");
  MAssert(soln->curr_resource_group == NULL,
    "KheSolnCopy called while resource group under construction");
  MAssert(soln->curr_event_group == NULL,
    "KheSolnCopy called while event group under construction");
  MAssert(MArraySize(soln->curr_transactions) == 0,
    "KheSolnCopy called after unmatched KheTransactionBegin");

  KheSolnMatchingUpdate(soln);
  copy = KheSolnCopyPhase1(soln);
  KheSolnCopyPhase2(soln);
  return copy;
}


/*****************************************************************************/
/*                                                                           */
/*  KHE_INSTANCE KheSolnInstance(KHE_SOLN soln)                              */
/*                                                                           */
/*  Return the instance of soln.                                             */
/*                                                                           */
/*****************************************************************************/

KHE_INSTANCE KheSolnInstance(KHE_SOLN soln)
{
  return soln->instance;
}


/*****************************************************************************/
/*                                                                           */
/*  KHE_SOLN_GROUP KheSolnSolnGroup(KHE_SOLN soln)                           */
/*                                                                           */
/*  Return the solution group containing soln, or NULL if none.              */
/*                                                                           */
/*****************************************************************************/

KHE_SOLN_GROUP KheSolnSolnGroup(KHE_SOLN soln)
{
  return soln->soln_group;
}


/*****************************************************************************/
/*                                                                           */
/*  Submodule "evenness handling"                                            */
/*                                                                           */
/*****************************************************************************/

/*****************************************************************************/
/*                                                                           */
/*  KHE_EVENNESS_HANDLER KheSolnEvennessHandler(KHE_SOLN soln)               */
/*                                                                           */
/*  Return soln's evenness handler.                                          */
/*                                                                           */
/*****************************************************************************/

KHE_EVENNESS_HANDLER KheSolnEvennessHandler(KHE_SOLN soln)
{
  return soln->evenness_handler;
}


/*****************************************************************************/
/*                                                                           */
/*  void KheSolnAttachAllEvennessMonitors(KHE_SOLN soln)                     */
/*                                                                           */
/*  Ensure that all evenness monitors are attached.                          */
/*                                                                           */
/*****************************************************************************/

void KheSolnAttachAllEvennessMonitors(KHE_SOLN soln)
{
  KheEvennessHandlerAttachAllEvennessMonitors(soln->evenness_handler);
}


/*****************************************************************************/
/*                                                                           */
/*  void KheSolnDetachAllEvennessMonitors(KHE_SOLN soln)                     */
/*                                                                           */
/*  Ensure that all evenness monitors are detached.                          */
/*                                                                           */
/*****************************************************************************/

void KheSolnDetachAllEvennessMonitors(KHE_SOLN soln)
{
  KheEvennessHandlerDetachAllEvennessMonitors(soln->evenness_handler);
}


/*****************************************************************************/
/*                                                                           */
/*  void KheSolnSetAllEvennessMonitorWeights(KHE_SOLN soln, KHE_COST weight) */
/*                                                                           */
/*  Set the weight of all evenness monitors.                                 */
/*                                                                           */
/*****************************************************************************/

void KheSolnSetAllEvennessMonitorWeights(KHE_SOLN soln, KHE_COST weight)
{
  KheEvennessHandleSetAllEvennessMonitorWeights(soln->evenness_handler, weight);
}


/*****************************************************************************/
/*                                                                           */
/*  Submodule "cost and monitors"                                            */
/*                                                                           */
/*****************************************************************************/

/*****************************************************************************/
/*                                                                           */
/*  KHE_COST KheCost(int hard_cost, int soft_cost)                           */
/*                                                                           */
/*  Combine hard_cost and soft_cost into a single cost.                      */
/*                                                                           */
/*****************************************************************************/
#define KHE_HARD_COST_WEIGHT ((KHE_COST) 1 << (KHE_COST) 32)

KHE_COST KheCost(int hard_cost, int soft_cost)
{
  return (KHE_COST) hard_cost * KHE_HARD_COST_WEIGHT + (KHE_COST) soft_cost;
}


/*****************************************************************************/
/*                                                                           */
/*  int KheHardCost(KHE_COST combined_cost)                                  */
/*                                                                           */
/*  Return the hard cost component of combined_cost.                         */
/*                                                                           */
/*****************************************************************************/

int KheHardCost(KHE_COST combined_cost)
{
  return (int) (combined_cost / KHE_HARD_COST_WEIGHT);
}


/*****************************************************************************/
/*                                                                           */
/*  int KheSoftCost(KHE_COST combined_cost)                                  */
/*                                                                           */
/*  Return the soft cost component of combined_cost.                         */
/*                                                                           */
/*****************************************************************************/

int KheSoftCost(KHE_COST combined_cost)
{
  return (int) (combined_cost % KHE_HARD_COST_WEIGHT);
}


/*****************************************************************************/
/*                                                                           */
/*  int KheCostCmp(KHE_COST cost1, KHE_COST cost2)                           */
/*                                                                           */
/*  Return an int which is less than, equal to, or greater than zero if      */
/*  the first argument is respectively less than, equal to, or greater       */
/*  than the second.                                                         */
/*                                                                           */
/*****************************************************************************/

int KheCostCmp(KHE_COST cost1, KHE_COST cost2)
{
  if( KheHardCost(cost1) != KheHardCost(cost2) )
    return KheHardCost(cost1) - KheHardCost(cost2);
  else
    return KheSoftCost(cost1) - KheSoftCost(cost2);
}


/*****************************************************************************/
/*                                                                           */
/*  double KheCostShow(KHE_COST combined_cost)                               */
/*                                                                           */
/*  Return a floating point value suitable for displaying combined_cost.     */
/*                                                                           */
/*****************************************************************************/

double KheCostShow(KHE_COST combined_cost)
{
  int soft_cost;
  soft_cost =
    KheSoftCost(combined_cost) <= 9999 ? KheSoftCost(combined_cost) : 9999;
  return (double) KheHardCost(combined_cost) + (double) soft_cost / 10000;
}


/*****************************************************************************/
/*                                                                           */
/*  KHE_COST KheSolnCost(KHE_SOLN soln)                                      */
/*                                                                           */
/*  Return the total cost of soln.                                           */
/*                                                                           */
/*****************************************************************************/

KHE_COST KheSolnCost(KHE_SOLN soln)
{
  KheMatchingUnmatchedDemandNodeCount(soln->matching);
  return soln->cost;
}


/*****************************************************************************/
/*                                                                           */
/*  int KheSolnChildMonitorCount(KHE_SOLN soln)                              */
/*                                                                           */
/*  Return the number of child monitors of soln.                             */
/*                                                                           */
/*****************************************************************************/

int KheSolnChildMonitorCount(KHE_SOLN soln)
{
  return MArraySize(soln->child_monitors);
}


/*****************************************************************************/
/*                                                                           */
/*  KHE_MONITOR KheSolnChildMonitor(KHE_SOLN soln, int i)                    */
/*                                                                           */
/*  Return the i'th child monitor of soln.                                   */
/*                                                                           */
/*****************************************************************************/

KHE_MONITOR KheSolnChildMonitor(KHE_SOLN soln, int i)
{
  return MArrayGet(soln->child_monitors, i);
}


/*****************************************************************************/
/*                                                                           */
/*  int KheSolnDefectCount(KHE_SOLN soln)                                    */
/*                                                                           */
/*  Return the number of defects (child monitors of non-zero cost) of soln.  */
/*                                                                           */
/*****************************************************************************/

int KheSolnDefectCount(KHE_SOLN soln)
{
  return KheGroupMonitorDefectCount((KHE_GROUP_MONITOR) soln);
}


/*****************************************************************************/
/*                                                                           */
/*  KHE_MONITOR KheSolnDefect(KHE_SOLN soln, int i)                          */
/*                                                                           */
/*  Return the i'th defect (child monitor of non-zero cost) of soln.         */
/*                                                                           */
/*****************************************************************************/

KHE_MONITOR KheSolnDefect(KHE_SOLN soln, int i)
{
  return KheGroupMonitorDefect((KHE_GROUP_MONITOR) soln, i);
}


/*****************************************************************************/
/*                                                                           */
/*  void KheSolnDefectSort(KHE_SOLN soln)                                    */
/*                                                                           */
/*  Sort the defects of soln by decreasing cost.                             */
/*                                                                           */
/*****************************************************************************/

void KheSolnDefectSort(KHE_SOLN soln)
{
  KheGroupMonitorDefectSort((KHE_GROUP_MONITOR) soln);
}


/*****************************************************************************/
/*                                                                           */
/*  void KheSolnCopyDefects(KHE_SOLN soln)                                   */
/*                                                                           */
/*  Initialize the copied defect list of soln.                               */
/*                                                                           */
/*****************************************************************************/

void KheSolnCopyDefects(KHE_SOLN soln)
{
  KheGroupMonitorCopyDefects((KHE_GROUP_MONITOR) soln);
}


/*****************************************************************************/
/*                                                                           */
/*  int KheSolnDefectCopyCount(KHE_SOLN soln)                                */
/*                                                                           */
/*  Return the number of elements on soln's copied defect list.              */
/*                                                                           */
/*****************************************************************************/

int KheSolnDefectCopyCount(KHE_SOLN soln)
{
  return KheGroupMonitorDefectCopyCount((KHE_GROUP_MONITOR) soln);
}


/*****************************************************************************/
/*                                                                           */
/*  KHE_MONITOR KheSolnDefectCopy(KHE_SOLN soln, int i)                      */
/*                                                                           */
/*  Return the i'th element of soln's copied defect list.                    */
/*                                                                           */
/*****************************************************************************/

KHE_MONITOR KheSolnDefectCopy(KHE_SOLN soln, int i)
{
  return KheGroupMonitorDefectCopy((KHE_GROUP_MONITOR) soln, i);
}


/*****************************************************************************/
/*                                                                           */
/*  Submodule "event monitors and cost"                                      */
/*                                                                           */
/*****************************************************************************/

/*****************************************************************************/
/*                                                                           */
/*  int KheEventMonitorCount(KHE_SOLN soln, KHE_EVENT e)                     */
/*                                                                           */
/*  Return the number of event monitors of e in soln.                        */
/*                                                                           */
/*****************************************************************************/

int KheEventMonitorCount(KHE_SOLN soln, KHE_EVENT e)
{
  KHE_EVENT_IN_SOLN es;
  es = MArrayGet(soln->events_in_soln, KheEventIndex(e));
  return KheEventInSolnMonitorCount(es);
}


/*****************************************************************************/
/*                                                                           */
/*  KHE_MONITOR KheEventMonitor(KHE_SOLN soln, KHE_EVENT e, int i)           */
/*                                                                           */
/*  Return the i'th event monitor of e in soln.                              */
/*                                                                           */
/*****************************************************************************/

KHE_MONITOR KheEventMonitor(KHE_SOLN soln, KHE_EVENT e, int i)
{
  KHE_EVENT_IN_SOLN es;
  es = MArrayGet(soln->events_in_soln, KheEventIndex(e));
  return KheEventInSolnMonitor(es, i);
}


/*****************************************************************************/
/*                                                                           */
/*  KHE_COST KheEventCost(KHE_SOLN soln, KHE_EVENT e)                        */
/*                                                                           */
/*  Return the total cost of monitors applicable to e.                       */
/*                                                                           */
/*****************************************************************************/

KHE_COST KheEventCost(KHE_SOLN soln, KHE_EVENT e)
{
  KHE_EVENT_IN_SOLN es;
  es = MArrayGet(soln->events_in_soln, KheEventIndex(e));
  return KheEventInSolnCost(es);
}


/*****************************************************************************/
/*                                                                           */
/*  KHE_COST KheEventMonitorCost(KHE_SOLN soln, KHE_EVENT e,                 */
/*    KHE_MONITOR_TAG tag)                                                   */
/*                                                                           */
/*  Return the total cost of monitors of type tag applicable to e.           */
/*                                                                           */
/*****************************************************************************/

KHE_COST KheEventMonitorCost(KHE_SOLN soln, KHE_EVENT e, KHE_MONITOR_TAG tag)
{
  KHE_EVENT_IN_SOLN es;
  es = MArrayGet(soln->events_in_soln, KheEventIndex(e));
  return KheEventInSolnMonitorCost(es, tag);
}


/*****************************************************************************/
/*                                                                           */
/*  KHE_TIMETABLE_MONITOR KheEventTimetableMonitor(KHE_SOLN soln,KHE_EVENT e)*/
/*                                                                           */
/*  Return the timetable of e in soln.                                       */
/*                                                                           */
/*****************************************************************************/

KHE_TIMETABLE_MONITOR KheEventTimetableMonitor(KHE_SOLN soln, KHE_EVENT e)
{
  KHE_EVENT_IN_SOLN es;
  es = MArrayGet(soln->events_in_soln, KheEventIndex(e));
  return KheEventInSolnTimetableMonitor(es);
}


/*****************************************************************************/
/*                                                                           */
/*  Submodule "event resource monitors and cost"                             */
/*                                                                           */
/*****************************************************************************/

/*****************************************************************************/
/*                                                                           */
/*  int KheEventResourceMonitorCount(KHE_SOLN soln, KHE_EVENT_RESOURCE er)   */
/*                                                                           */
/*  Return the number of event resource monitors of er in soln.              */
/*                                                                           */
/*****************************************************************************/

int KheEventResourceMonitorCount(KHE_SOLN soln, KHE_EVENT_RESOURCE er)
{
  KHE_EVENT_IN_SOLN es;  KHE_EVENT_RESOURCE_IN_SOLN ers;
  es = MArrayGet(soln->events_in_soln,
    KheEventIndex(KheEventResourceEvent(er)));
  ers = KheEventInSolnEventResourceInSoln(es,
    KheEventResourceIndexInEvent(er));
  return KheEventResourceInSolnMonitorCount(ers);
}


/*****************************************************************************/
/*                                                                           */
/*  KHE_MONITOR KheEventResourceMonitor(KHE_SOLN soln,                       */
/*    KHE_EVENT_RESOURCE er, int i)                                          */
/*                                                                           */
/*  Return the i'th event resource monitor of er in soln.                    */
/*                                                                           */
/*****************************************************************************/

KHE_MONITOR KheEventResourceMonitor(KHE_SOLN soln,
  KHE_EVENT_RESOURCE er, int i)
{
  KHE_EVENT_IN_SOLN es;  KHE_EVENT_RESOURCE_IN_SOLN ers;
  es = MArrayGet(soln->events_in_soln,
    KheEventIndex(KheEventResourceEvent(er)));
  ers = KheEventInSolnEventResourceInSoln(es,
    KheEventResourceIndexInEvent(er));
  return KheEventResourceInSolnMonitor(ers, i);
}


/*****************************************************************************/
/*                                                                           */
/*  KHE_COST KheEventResourceCost(KHE_SOLN soln, KHE_EVENT_RESOURCE er)      */
/*                                                                           */
/*  Return the total cost of monitors applicable to er.                      */
/*                                                                           */
/*****************************************************************************/

KHE_COST KheEventResourceCost(KHE_SOLN soln, KHE_EVENT_RESOURCE er)
{
  KHE_EVENT_IN_SOLN es;  KHE_EVENT_RESOURCE_IN_SOLN ers;
  es = MArrayGet(soln->events_in_soln,
    KheEventIndex(KheEventResourceEvent(er)));
  ers = KheEventInSolnEventResourceInSoln(es,
    KheEventResourceIndexInEvent(er));
  return KheEventResourceInSolnCost(ers);
}


/*****************************************************************************/
/*                                                                           */
/*  KHE_COST KheEventResourceMonitorCost(KHE_SOLN soln,                      */
/*    KHE_EVENT_RESOURCE er, KHE_MONITOR_TAG tag)                            */
/*                                                                           */
/*  Return the total cost of monitors of type tag applicable to er.          */
/*                                                                           */
/*****************************************************************************/

KHE_COST KheEventResourceMonitorCost(KHE_SOLN soln, KHE_EVENT_RESOURCE er,
  KHE_MONITOR_TAG tag)
{
  KHE_EVENT_IN_SOLN es;  KHE_EVENT_RESOURCE_IN_SOLN ers;
  es = MArrayGet(soln->events_in_soln,
    KheEventIndex(KheEventResourceEvent(er)));
  ers = KheEventInSolnEventResourceInSoln(es,
    KheEventResourceIndexInEvent(er));
  return KheEventResourceInSolnMonitorCost(ers, tag);
}


/*****************************************************************************/
/*                                                                           */
/*  Submodule "tasks assigned to resources"                                  */
/*                                                                           */
/*****************************************************************************/

/*****************************************************************************/
/*                                                                           */
/*  int KheResourceAssignedTaskCount(KHE_SOLN soln, KHE_RESOURCE r)          */
/*                                                                           */
/*  Return the number of tasks assigned r in soln.                           */
/*                                                                           */
/*****************************************************************************/

int KheResourceAssignedTaskCount(KHE_SOLN soln, KHE_RESOURCE r)
{
  KHE_RESOURCE_IN_SOLN rs;
  rs = MArrayGet(soln->resources_in_soln, KheResourceIndexInInstance(r));
  return KheResourceInSolnAssignedTaskCount(rs);
}


/*****************************************************************************/
/*                                                                           */
/*  KHE_TASK KheResourceAssignedTask(KHE_SOLN soln, KHE_RESOURCE r, int i)   */
/*                                                                           */
/*  Return the i'th task assigned to r in soln.                              */
/*                                                                           */
/*****************************************************************************/

KHE_TASK KheResourceAssignedTask(KHE_SOLN soln, KHE_RESOURCE r, int i)
{
  KHE_RESOURCE_IN_SOLN rs;
  rs = MArrayGet(soln->resources_in_soln, KheResourceIndexInInstance(r));
  return KheResourceInSolnAssignedTask(rs, i);
}


/*****************************************************************************/
/*                                                                           */
/*  Submodule "resource monitors and cost"                                   */
/*                                                                           */
/*****************************************************************************/

/*****************************************************************************/
/*                                                                           */
/*  int KheResourceMonitorCount(KHE_SOLN soln, KHE_RESOURCE r)               */
/*                                                                           */
/*  Return the number of resource monitors of r in soln.                     */
/*                                                                           */
/*****************************************************************************/

int KheResourceMonitorCount(KHE_SOLN soln, KHE_RESOURCE r)
{
  KHE_RESOURCE_IN_SOLN rs;
  rs = MArrayGet(soln->resources_in_soln, KheResourceIndexInInstance(r));
  return KheResourceInSolnMonitorCount(rs);
}


/*****************************************************************************/
/*                                                                           */
/*  KHE_MONITOR KheResourceMonitor(KHE_SOLN soln, KHE_RESOURCE r, int i)     */
/*                                                                           */
/*  Return the i'th resource monitor of r in soln.                           */
/*                                                                           */
/*****************************************************************************/

KHE_MONITOR KheResourceMonitor(KHE_SOLN soln, KHE_RESOURCE r, int i)
{
  KHE_RESOURCE_IN_SOLN rs;
  rs = MArrayGet(soln->resources_in_soln, KheResourceIndexInInstance(r));
  return KheResourceInSolnMonitor(rs, i);
}


/*****************************************************************************/
/*                                                                           */
/*  KHE_COST KheResourceCost(KHE_SOLN soln, KHE_RESOURCE r)                  */
/*                                                                           */
/*  Return the total cost of monitors applicable to r.                       */
/*                                                                           */
/*****************************************************************************/

KHE_COST KheResourceCost(KHE_SOLN soln, KHE_RESOURCE r)
{
  KHE_RESOURCE_IN_SOLN rs;
  rs = MArrayGet(soln->resources_in_soln, KheResourceIndexInInstance(r));
  return KheResourceInSolnCost(rs);
}


/*****************************************************************************/
/*                                                                           */
/*  KHE_COST KheResourceMonitorCost(KHE_SOLN soln, KHE_RESOURCE r,           */
/*    KHE_MONITOR_TAG tag)                                                   */
/*                                                                           */
/*  Return the total cost of constraints of type tag applicable to r.        */
/*                                                                           */
/*****************************************************************************/

KHE_COST KheResourceMonitorCost(KHE_SOLN soln, KHE_RESOURCE r,
  KHE_MONITOR_TAG tag)
{
  KHE_RESOURCE_IN_SOLN rs;
  rs = MArrayGet(soln->resources_in_soln, KheResourceIndexInInstance(r));
  return KheResourceInSolnMonitorCost(rs, tag);
}


/*****************************************************************************/
/*                                                                           */
/*  KHE_TIMETABLE_MONITOR KheResourceTimetableMonitor(KHE_SOLN soln,         */
/*    KHE_RESOURCE r)                                                        */
/*                                                                           */
/*  Return the timetable of r in soln.                                       */
/*                                                                           */
/*****************************************************************************/

KHE_TIMETABLE_MONITOR KheResourceTimetableMonitor(KHE_SOLN soln, KHE_RESOURCE r)
{
  KHE_RESOURCE_IN_SOLN rs;
  rs = MArrayGet(soln->resources_in_soln, KheResourceIndexInInstance(r));
  return KheResourceInSolnTimetableMonitor(rs);
}


/*****************************************************************************/
/*                                                                           */
/*  KHE_RESOURCE_IN_SOLN KheSolnResourceInSoln(KHE_SOLN soln, int i)         */
/*                                                                           */
/*  Return the i'th resource monitor of soln.                                */
/*                                                                           */
/*****************************************************************************/

KHE_RESOURCE_IN_SOLN KheSolnResourceInSoln(KHE_SOLN soln, int i)
{
  return MArrayGet(soln->resources_in_soln, i);
}


/*****************************************************************************/
/*                                                                           */
/*  Submodule "monitors"                                                     */
/*                                                                           */
/*****************************************************************************/

/*****************************************************************************/
/*                                                                           */
/*  void KheSolnAddMonitor(KHE_SOLN soln, KHE_MONITOR m, int *index_in_soln) */
/*                                                                           */
/*  Add m to soln, returning its index.                                      */
/*                                                                           */
/*****************************************************************************/

void KheSolnAddMonitor(KHE_SOLN soln, KHE_MONITOR m, int *index_in_soln)
{
  *index_in_soln = MArraySize(soln->monitors);
  MArrayAddLast(soln->monitors, m);
}


/*****************************************************************************/
/*                                                                           */
/*  void KheSolnDeleteMonitor(KHE_SOLN soln, KHE_MONITOR m)                  */
/*                                                                           */
/*  Delete m from soln.                                                      */
/*                                                                           */
/*****************************************************************************/

void KheSolnDeleteMonitor(KHE_SOLN soln, KHE_MONITOR m)
{
  KHE_MONITOR tmp;
  tmp = MArrayRemoveAndPlug(soln->monitors, KheMonitorIndexInSoln(m));
  KheMonitorSetIndexInSoln(tmp, KheMonitorIndexInSoln(m));
}


/*****************************************************************************/
/*                                                                           */
/*  int KheSolnMonitorCount(KHE_SOLN soln)                                   */
/*                                                                           */
/*  Return the number of monitors of soln.                                   */
/*                                                                           */
/*****************************************************************************/

int KheSolnMonitorCount(KHE_SOLN soln)
{
  return MArraySize(soln->monitors);
}


/*****************************************************************************/
/*                                                                           */
/*  KHE_MONITOR KheSolnMonitor(KHE_SOLN soln, int i)                         */
/*                                                                           */
/*  Return the i'th monitor of soln.                                         */
/*                                                                           */
/*****************************************************************************/

KHE_MONITOR KheSolnMonitor(KHE_SOLN soln, int i)
{
  return MArrayGet(soln->monitors, i);
}


/*****************************************************************************/
/*                                                                           */
/*  Submodule "meets"                                                        */
/*                                                                           */
/*****************************************************************************/

/*****************************************************************************/
/*                                                                           */
/*  void KheSolnAddMeet(KHE_SOLN soln, KHE_MEET meet, int *index_in_soln)    */
/*                                                                           */
/*  Add meet to soln, returning its index.                                   */
/*                                                                           */
/*****************************************************************************/

void KheSolnAddMeet(KHE_SOLN soln, KHE_MEET meet, int *index_in_soln)
{
  *index_in_soln = MArraySize(soln->meets);
  MArrayAddLast(soln->meets, meet);
}


/*****************************************************************************/
/*                                                                           */
/*  void KheSolnDeleteMeet(KHE_SOLN soln, KHE_MEET meet)                     */
/*                                                                           */
/*  Delete meet from soln.                                                   */
/*                                                                           */
/*****************************************************************************/

void KheSolnDeleteMeet(KHE_SOLN soln, KHE_MEET meet)
{
  /* KHE_EVENT event;  KHE_EVENT_IN_SOLN es; */
  KHE_MEET tmp;

  /* remove from meets */
  tmp = MArrayRemoveAndPlug(soln->meets, KheMeetIndex(meet));
  KheMeetSetIndex(tmp, KheMeetIndex(meet));
}


/*****************************************************************************/
/*                                                                           */
/*  bool KheSolnMakeCompleteRepresentation(KHE_SOLN soln,                    */
/*    KHE_EVENT *problem_event)                                              */
/*                                                                           */
/*  Ensure that soln is a complete representation of its instance, by        */
/*  adding one meet to each meet set whose total duration is less than       */
/*  the duration of its event, and adding tasks to all meets as required.    */
/*                                                                           */
/*  Return true if successful, or, if prevented by the presence of an        */
/*  event whose meets' durations are already too great, set *problem_event   */
/*  to the first such event and return false.                                */
/*                                                                           */
/*****************************************************************************/

bool KheSolnMakeCompleteRepresentation(KHE_SOLN soln,
  KHE_EVENT *problem_event)
{
  int i;  KHE_EVENT_IN_SOLN es;
  MArrayForEach(soln->events_in_soln, &es, &i)
    if( !KheEventInSolnMakeCompleteRepresentation(es, problem_event) )
      return false;
  return true;
}


/*****************************************************************************/
/*                                                                           */
/*  void KheSolnAssignPreassignedTimes(KHE_SOLN soln, bool as_in_event)      */
/*                                                                           */
/*  Assign preassigned times to all the meets of soln that have them and     */
/*  are not already assigned.                                                */
/*                                                                           */
/*  If as_in_event is true, a meet is considered to have a preassigned       */
/*  time if it is derived from an event and that event has a preassigned     */
/*  time.  If as_in_event is false, a meet is considered to have a           */
/*  preassigned time if its domain contains exactly one element.             */
/*                                                                           */
/*****************************************************************************/

void KheSolnAssignPreassignedTimes(KHE_SOLN soln, bool as_in_event)
{
  KHE_MEET meet;  int i;  KHE_TIME preassigned_time;
  if( DEBUG15 )
    fprintf(stderr, "[ KheSolnAssignPreassignedTimes(soln)\n");
  MArrayForEach(soln->meets, &meet, &i)
    if( KheMeetAsst(meet) == NULL && KheMeetEvent(meet) != NULL )
    {
      preassigned_time = KheEventPreassignedTime(KheMeetEvent(meet));
      if( preassigned_time != NULL )
      {
	if( !KheMeetAssignTime(meet, preassigned_time) )
	  MAssert(false, "KheSolnAssignPreassignedTimes failed to assign");
      }
    }
  if( DEBUG15 )
    fprintf(stderr, "] KheSolnAssignPreassignedTimes returning\n");
}


/*****************************************************************************/
/*                                                                           */
/*  int KheSolnMeetCount(KHE_SOLN soln)                                      */
/*                                                                           */
/*  Return the number of meets in soln.                                      */
/*                                                                           */
/*****************************************************************************/

int KheSolnMeetCount(KHE_SOLN soln)
{
  return MArraySize(soln->meets);
}


/*****************************************************************************/
/*                                                                           */
/*  KHE_MEET KheSolnMeet(KHE_SOLN soln, int i)                               */
/*                                                                           */
/*  Return the i'th meet of soln.                                            */
/*                                                                           */
/*****************************************************************************/

KHE_MEET KheSolnMeet(KHE_SOLN soln, int i)
{
  return MArrayGet(soln->meets, i);
}


/*****************************************************************************/
/*                                                                           */
/*  KHE_EVENT_IN_SOLN KheSolnEventInSoln(KHE_SOLN soln, int i)               */
/*                                                                           */
/*  Return the i'th event in soln of soln.                                   */
/*                                                                           */
/*****************************************************************************/

KHE_EVENT_IN_SOLN KheSolnEventInSoln(KHE_SOLN soln, int i)
{
  return MArrayGet(soln->events_in_soln, i);
}


/*****************************************************************************/
/*                                                                           */
/*  int KheEventMeetCount(KHE_SOLN soln, KHE_EVENT e)                        */
/*                                                                           */
/*  Return the number of meets for e in soln.                                */
/*                                                                           */
/*****************************************************************************/

int KheEventMeetCount(KHE_SOLN soln, KHE_EVENT e)
{
  KHE_EVENT_IN_SOLN es;
  es = MArrayGet(soln->events_in_soln, KheEventIndex(e));
  return KheEventInSolnMeetCount(es);
}


/*****************************************************************************/
/*                                                                           */
/*  KHE_MEET KheEventMeet(KHE_SOLN soln, KHE_EVENT e, int i)                 */
/*                                                                           */
/*  Return the i'th meet of event e in soln.                                 */
/*                                                                           */
/*****************************************************************************/

KHE_MEET KheEventMeet(KHE_SOLN soln, KHE_EVENT e, int i)
{
  KHE_EVENT_IN_SOLN es;
  es = MArrayGet(soln->events_in_soln, KheEventIndex(e));
  return KheEventInSolnMeet(es, i);
}


/*****************************************************************************/
/*                                                                           */
/*  Submodule "cycle meets"                                                  */
/*                                                                           */
/*****************************************************************************/

/*****************************************************************************/
/*                                                                           */
/*  KHE_MEET KheSolnTimeCycleMeet(KHE_SOLN soln, KHE_TIME t)                 */
/*                                                                           */
/*  Return the cycle meet covering t.                                        */
/*                                                                           */
/*****************************************************************************/

KHE_MEET KheSolnTimeCycleMeet(KHE_SOLN soln, KHE_TIME t)
{
  return MArrayGet(soln->time_to_cycle_meet, KheTimeIndex(t));
}


/*****************************************************************************/
/*                                                                           */
/*  int KheSolnTimeCycleMeetOffset(KHE_SOLN soln, KHE_TIME t)                */
/*                                                                           */
/*  Return the offset of t in its cycle meet.                                */
/*                                                                           */
/*****************************************************************************/

int KheSolnTimeCycleMeetOffset(KHE_SOLN soln, KHE_TIME t)
{
  return MArrayGet(soln->time_to_cycle_offset, KheTimeIndex(t));
}


/*****************************************************************************/
/*                                                                           */
/*  bool KheSolnTimeMayBeginBlock(KHE_SOLN soln, KHE_TIME t, int durn)       */
/*                                                                           */
/*  Return true if a meet of this durn may start at time t.                  */
/*                                                                           */
/*****************************************************************************/

static bool KheSolnTimeMayBeginBlock(KHE_SOLN soln, KHE_TIME t, int durn)
{
  KHE_MEET cycle_meet;  int cycle_offset;
  cycle_meet = KheSolnTimeCycleMeet(soln, t);
  cycle_offset = KheSolnTimeCycleMeetOffset(soln, t);
  return cycle_offset + durn <= KheMeetDuration(cycle_meet);
}


/*****************************************************************************/
/*                                                                           */
/*  KHE_TIME_GROUP KhePackingTimeGroupMake(KHE_SOLN soln, int duration)      */
/*                                                                           */
/*  Make the packing time group for meets of soln of this duration.          */
/*                                                                           */
/*****************************************************************************/

static KHE_TIME_GROUP KhePackingTimeGroupMake(KHE_SOLN soln, int duration)
{
  KHE_TIME t;  int i;
  KheSolnTimeGroupBegin(soln);
  for( i = 0;  i < KheInstanceTimeCount(soln->instance);  i++ )
  {
    t = KheInstanceTime(soln->instance, i);
    if( KheSolnTimeMayBeginBlock(soln, t, duration) )
      KheSolnTimeGroupAddTime(soln, t);
  }
  return KheSolnTimeGroupEnd(soln);
}


/*****************************************************************************/
/*                                                                           */
/*  KHE_TIME_GROUP KheSolnPackingTimeGroup(KHE_SOLN soln, int duration)      */
/*                                                                           */
/*  Return the time group suitable for use as the domain of meets of         */
/*  the given duration.                                                      */
/*                                                                           */
/*****************************************************************************/

KHE_TIME_GROUP KheSolnPackingTimeGroup(KHE_SOLN soln, int duration)
{
  MArrayFill(soln->packing_time_groups, duration, NULL);
  if( MArrayGet(soln->packing_time_groups, duration - 1) == NULL )
    MArrayPut(soln->packing_time_groups, duration - 1,
      KhePackingTimeGroupMake(soln, duration));
  return MArrayGet(soln->packing_time_groups, duration - 1);
}


/*****************************************************************************/
/*                                                                           */
/*  void KheSolnCycleMeetSplit(KHE_SOLN soln, KHE_MEET meet1, KHE_MEET meet2)*/
/*                                                                           */
/*  Record the fact that a cycle meet has just split into meet1 and meet2.   */
/*                                                                           */
/*****************************************************************************/

void KheSolnCycleMeetSplit(KHE_SOLN soln, KHE_MEET meet1, KHE_MEET meet2)
{
  int i, ti;

  /* packing time groups are now out of date */
  MArrayClear(soln->packing_time_groups);

  /* reset time_to_cycle_meet and time_to_cycle_offset */
  for( i = 0;  i < KheMeetDuration(meet2);  i++ )
  {
    ti = i + KheMeetAssignedTimeIndex(meet2);  
    MArrayPut(soln->time_to_cycle_meet, ti, meet2);
    MArrayPut(soln->time_to_cycle_offset, ti, i);
  }
}


/*****************************************************************************/
/*                                                                           */
/*  void KheSolnCycleMeetMerge(KHE_SOLN soln, KHE_MEET meet1, KHE_MEET meet2)*/
/*                                                                           */
/*  Record the fact that these cycle meets are merging.                      */
/*                                                                           */
/*****************************************************************************/

void KheSolnCycleMeetMerge(KHE_SOLN soln, KHE_MEET meet1, KHE_MEET meet2)
{
  int i, ti;

  /* packing time groups are now out of date */
  MArrayClear(soln->packing_time_groups);

  /* reset time_to_cycle_meet and time_to_cycle_offset */
  for( i = 0;  i < KheMeetDuration(meet2);  i++ )
  {
    ti = i + KheMeetAssignedTimeIndex(meet2);  
    MArrayPut(soln->time_to_cycle_meet, ti, meet1);
    MArrayPut(soln->time_to_cycle_offset, ti, i + KheMeetDuration(meet1));
  }
}


/*****************************************************************************/
/*                                                                           */
/*  Submodule "tasks"                                                        */
/*                                                                           */
/*****************************************************************************/

/*****************************************************************************/
/*                                                                           */
/*  int KheEventResourceTaskCount(KHE_SOLN soln, KHE_EVENT_RESOURCE er)      */
/*                                                                           */
/*  Return the number of soln resources in soln corresponding to er.         */
/*                                                                           */
/*****************************************************************************/

int KheEventResourceTaskCount(KHE_SOLN soln, KHE_EVENT_RESOURCE er)
{
  KHE_EVENT_IN_SOLN es;  KHE_EVENT_RESOURCE_IN_SOLN ers;
  es = MArrayGet(soln->events_in_soln,
    KheEventIndex(KheEventResourceEvent(er)));
  ers = KheEventInSolnEventResourceInSoln(es,
    KheEventResourceIndexInEvent(er));
  return KheEventResourceInSolnTaskCount(ers);
}


/*****************************************************************************/
/*                                                                           */
/* KHE_TASK KheEventResourceTask(KHE_SOLN soln, KHE_EVENT_RESOURCE er, int i)*/
/*                                                                           */
/*  Return the i'th solution resource corresponding to er in soln.           */
/*                                                                           */
/*****************************************************************************/

KHE_TASK KheEventResourceTask(KHE_SOLN soln, KHE_EVENT_RESOURCE er, int i)
{
  KHE_EVENT_IN_SOLN es;  KHE_EVENT_RESOURCE_IN_SOLN ers;
  es = MArrayGet(soln->events_in_soln,
    KheEventIndex(KheEventResourceEvent(er)));
  ers = KheEventInSolnEventResourceInSoln(es,
    KheEventResourceIndexInEvent(er));
  return KheEventResourceInSolnTask(ers, i);
}


/*****************************************************************************/
/*                                                                           */
/*  void KheSolnAssignPreassignedResources(KHE_SOLN soln,                    */
/*    KHE_RESOURCE_TYPE rt, bool as_in_event_resource)                       */
/*                                                                           */
/*  Assign preassigned resources to those tasks that are not already         */
/*  assigned and have them.  If rt != NULL, this applies only to tasks       */
/*  of resource type rt.                                                     */
/*                                                                           */
/*  If as_in_event_resource is true, a task is considered to have a          */
/*  preassigned resource if it is derived from an event resource with        */
/*  a preassigned resource.  If as_in_event_resource is false, a task        */
/*  is considered to have a preassigned resource if its domain contains      */
/*  exactly one element.                                                     */
/*                                                                           */
/*****************************************************************************/

void KheSolnAssignPreassignedResources(KHE_SOLN soln, KHE_RESOURCE_TYPE rt,
  bool as_in_event_resource)
{
  KHE_MEET meet;  int i;
  MArrayForEach(soln->meets, &meet, &i)
    KheMeetAssignPreassignedResources(meet, rt, as_in_event_resource);
}


/*****************************************************************************/
/*                                                                           */
/*  Submodule "nodes"                                                        */
/*                                                                           */
/*****************************************************************************/

/*****************************************************************************/
/*                                                                           */
/*  void KheSolnAddNode(KHE_SOLN soln, KHE_NODE node, int *index)            */
/*                                                                           */
/*  Add node to soln, and set *index to the node's index.                    */
/*                                                                           */
/*****************************************************************************/

void KheSolnAddNode(KHE_SOLN soln, KHE_NODE node, int *index)
{
  *index = MArraySize(soln->nodes);
  MArrayAddLast(soln->nodes, node);
  if( DEBUG3 )
    fprintf(stderr, "[ KheSolnAddNode(soln %p, node %p, %d) ]\n",
      (void *) soln, (void *) node, *index);
}


/*****************************************************************************/
/*                                                                           */
/*  void KheSolnDeleteNode(KHE_SOLN soln, KHE_NODE node)                     */
/*                                                                           */
/*  Delete node from soln.                                                   */
/*                                                                           */
/*****************************************************************************/

void KheSolnDeleteNode(KHE_SOLN soln, KHE_NODE node)
{
  KHE_NODE tmp;
  tmp = MArrayRemoveLast(soln->nodes);
  if( tmp != node )
  {
    MArrayPut(soln->nodes, KheNodeIndex(node), tmp);
    KheNodeSetIndex(tmp, KheNodeIndex(node));
  }
}


/*****************************************************************************/
/*                                                                           */
/*  int KheSolnNodeCount(KHE_SOLN soln)                                      */
/*                                                                           */
/*  Return the number of nodes in soln.                                      */
/*                                                                           */
/*****************************************************************************/

int KheSolnNodeCount(KHE_SOLN soln)
{
  return MArraySize(soln->nodes);
}


/*****************************************************************************/
/*                                                                           */
/*  KHE_NODE KheSolnNode(KHE_SOLN soln, int i)                               */
/*                                                                           */
/*  Return the i'th solution node of soln.                                   */
/*                                                                           */
/*****************************************************************************/

KHE_NODE KheSolnNode(KHE_SOLN soln, int i)
{
  return MArrayGet(soln->nodes, i);
}


/*****************************************************************************/
/*                                                                           */
/*  Submodule "tasks"                                                        */
/*                                                                           */
/*****************************************************************************/

/*****************************************************************************/
/*                                                                           */
/*  KHE_TASK KheSolnGetTaskFromFreeList(KHE_SOLN soln)                       */
/*                                                                           */
/*  Return a new task from soln's free list, or NULL if none.                */
/*                                                                           */
/*****************************************************************************/

KHE_TASK KheSolnGetTaskFromFreeList(KHE_SOLN soln)
{
  if( MArraySize(soln->free_tasks) > 0 )
    return MArrayRemoveLast(soln->free_tasks);
  else
    return NULL;
}


/*****************************************************************************/
/*                                                                           */
/*  void KheSolnAddTaskToFreeList(KHE_SOLN soln, KHE_TASK task)              */
/*                                                                           */
/*  Add task to soln's free list of tasks.                                   */
/*                                                                           */
/*****************************************************************************/

void KheSolnAddTaskToFreeList(KHE_SOLN soln, KHE_TASK task)
{
  MArrayAddLast(soln->free_tasks, task);
}


/*****************************************************************************/
/*                                                                           */
/*  void KheSolnAddTask(KHE_SOLN soln, KHE_TASK task, int *index_in_soln)    */
/*                                                                           */
/*  Add task to soln, returning its index.                                   */
/*                                                                           */
/*****************************************************************************/

void KheSolnAddTask(KHE_SOLN soln, KHE_TASK task, int *index_in_soln)
{
  *index_in_soln = MArraySize(soln->tasks);
  MArrayAddLast(soln->tasks, task);
}


/*****************************************************************************/
/*                                                                           */
/*  void KheSolnDeleteTask(KHE_SOLN soln, KHE_TASK task)                     */
/*                                                                           */
/*  Delete task from soln.                                                   */
/*                                                                           */
/*****************************************************************************/

void KheSolnDeleteTask(KHE_SOLN soln, KHE_TASK task)
{
  KHE_TASK tmp;
  tmp = MArrayRemoveAndPlug(soln->tasks, KheTaskIndexInSoln(task));
  KheTaskSetIndexInSoln(tmp, KheTaskIndexInSoln(task));
}


/*****************************************************************************/
/*                                                                           */
/*  int KheSolnTaskCount(KHE_SOLN soln)                                      */
/*                                                                           */
/*  Return the number of tasks in soln.                                      */
/*                                                                           */
/*****************************************************************************/

int KheSolnTaskCount(KHE_SOLN soln)
{
  return MArraySize(soln->tasks);
}


/*****************************************************************************/
/*                                                                           */
/*  KHE_TASK KheSolnTask(KHE_SOLN soln, int i)                               */
/*                                                                           */
/*  Return the i'th task of soln.                                            */
/*                                                                           */
/*****************************************************************************/

KHE_TASK KheSolnTask(KHE_SOLN soln, int i)
{
  return MArrayGet(soln->tasks, i);
}


/*****************************************************************************/
/*                                                                           */
/*  Submodule "taskings"                                                     */
/*                                                                           */
/*****************************************************************************/

/*****************************************************************************/
/*                                                                           */
/*  KHE_TASKING KheSolnGetTaskingFromFreeList(KHE_SOLN soln)                 */
/*                                                                           */
/*  Return a new tasking from soln's free list, or NULL if none.             */
/*                                                                           */
/*****************************************************************************/

KHE_TASKING KheSolnGetTaskingFromFreeList(KHE_SOLN soln)
{
  if( MArraySize(soln->free_taskings) > 0 )
    return MArrayRemoveLast(soln->free_taskings);
  else
    return NULL;
}


/*****************************************************************************/
/*                                                                           */
/*  void KheSolnAddTaskingToFreeList(KHE_SOLN soln, KHE_TASKING tasking)     */
/*                                                                           */
/*  Add tasking to soln's free list of taskings.                             */
/*                                                                           */
/*****************************************************************************/

void KheSolnAddTaskingToFreeList(KHE_SOLN soln, KHE_TASKING tasking)
{
  MArrayAddLast(soln->free_taskings, tasking);
}


/*****************************************************************************/
/*                                                                           */
/*  void KheSolnAddTasking(KHE_SOLN soln, KHE_TASKING tasking,               */
/*    int *index_in_soln)                                                    */
/*                                                                           */
/*  Add tasking to soln, returning its index.                                */
/*                                                                           */
/*****************************************************************************/

void KheSolnAddTasking(KHE_SOLN soln, KHE_TASKING tasking, int *index_in_soln)
{
  *index_in_soln = MArraySize(soln->taskings);
  MArrayAddLast(soln->taskings, tasking);
}


/*****************************************************************************/
/*                                                                           */
/*  void KheSolnDeleteTasking(KHE_SOLN soln, KHE_TASKING tasking)            */
/*                                                                           */
/*  Delete tasking from soln.                                                */
/*                                                                           */
/*****************************************************************************/

void KheSolnDeleteTasking(KHE_SOLN soln, KHE_TASKING tasking)
{
  KHE_TASKING tmp;
  tmp = MArrayRemoveAndPlug(soln->taskings, KheTaskingIndexInSoln(tasking));
  KheTaskingSetIndexInSoln(tmp, KheTaskingIndexInSoln(tasking));
}


/*****************************************************************************/
/*                                                                           */
/*  int KheSolnTaskingCount(KHE_SOLN soln)                                   */
/*                                                                           */
/*  Return the number of taskings in soln.                                   */
/*                                                                           */
/*****************************************************************************/

int KheSolnTaskingCount(KHE_SOLN soln)
{
  return MArraySize(soln->taskings);
}


/*****************************************************************************/
/*                                                                           */
/*  KHE_TASKING KheSolnTasking(KHE_SOLN soln, int i)                         */
/*                                                                           */
/*  Return the i'th tasking of soln.                                         */
/*                                                                           */
/*****************************************************************************/

KHE_TASKING KheSolnTasking(KHE_SOLN soln, int i)
{
  return MArrayGet(soln->taskings, i);
}


/*****************************************************************************/
/*                                                                           */
/*  Submodule "solution time groups"                                         */
/*                                                                           */
/*****************************************************************************/

/*****************************************************************************/
/*                                                                           */
/*  void KheSolnTimeGroupBegin(KHE_SOLN soln)                                */
/*                                                                           */
/*  Begin the construction of a soln time group.                             */
/*                                                                           */
/*****************************************************************************/

void KheSolnTimeGroupBegin(KHE_SOLN soln)
{
  MAssert(soln->curr_time_group == NULL,
    "KheSolnTimeGroupBegin: time group already under construction");
  soln->curr_time_group = KheTimeGroupMakeInternal(KHE_TIME_GROUP_TYPE_SOLN,
    soln->instance, KHE_TIME_GROUP_KIND_ORDINARY, NULL, NULL, LSetNew());
}


/*****************************************************************************/
/*                                                                           */
/*  void KheSolnTimeGroupAddTime(KHE_SOLN soln, KHE_TIME time)               */
/*                                                                           */
/*  Add a time to the time group currently being constructed.                */
/*                                                                           */
/*****************************************************************************/

void KheSolnTimeGroupAddTime(KHE_SOLN soln, KHE_TIME time)
{
  MAssert(soln->curr_time_group != NULL,
    "KheSolnTimeGroupAddTime: time group not under construction");
  KheTimeGroupUnionInternal(soln->curr_time_group,
    KheTimeSingletonTimeGroup(time));
}


/*****************************************************************************/
/*                                                                           */
/*  void KheSolnTimeGroupSubTime(KHE_SOLN soln, KHE_TIME time)               */
/*                                                                           */
/*  Take away time from the time group currently being constructed.          */
/*                                                                           */
/*****************************************************************************/

void KheSolnTimeGroupSubTime(KHE_SOLN soln, KHE_TIME time)
{
  MAssert(soln->curr_time_group != NULL,
    "KheSolnTimeGroupSubTime: time group not under construction");
  KheTimeGroupDifferenceInternal(soln->curr_time_group,
    KheTimeSingletonTimeGroup(time));
}


/*****************************************************************************/
/*                                                                           */
/*  void KheSolnTimeGroupUnion(KHE_SOLN soln, KHE_TIME_GROUP tg2)            */
/*                                                                           */
/*  Union a time group to the time group currently being constructed.        */
/*                                                                           */
/*****************************************************************************/

void KheSolnTimeGroupUnion(KHE_SOLN soln, KHE_TIME_GROUP tg2)
{
  MAssert(soln->curr_time_group != NULL,
    "KheSolnTimeGroupUnionTimeGroup: time group not under construction");
  KheTimeGroupUnionInternal(soln->curr_time_group, tg2);
}


/*****************************************************************************/
/*                                                                           */
/*  void KheSolnTimeGroupIntersect(KHE_SOLN soln,KHE_TIME_GROUP tg2)         */
/*                                                                           */
/*  Intersect a time group to the time group currently being constructed.    */
/*                                                                           */
/*****************************************************************************/

void KheSolnTimeGroupIntersect(KHE_SOLN soln, KHE_TIME_GROUP tg2)
{
  MAssert(soln->curr_time_group != NULL,
    "KheSolnTimeGroupIntersect: time group not under construction");
  KheTimeGroupIntersectInternal(soln->curr_time_group, tg2);
}


/*****************************************************************************/
/*                                                                           */
/*  void KheSolnTimeGroupDifferenceTimeGroup(KHE_SOLN soln,                  */
/*    KHE_TIME_GROUP tg2)                                                    */
/*                                                                           */
/*  Take away tg2 from the time group currently being constructed.           */
/*                                                                           */
/*****************************************************************************/

void KheSolnTimeGroupDifference(KHE_SOLN soln, KHE_TIME_GROUP tg2)
{
  MAssert(soln->curr_time_group != NULL,
    "KheSolnTimeGroupDifference: time group not under construction");
  KheTimeGroupDifferenceInternal(soln->curr_time_group, tg2);
}


/*****************************************************************************/
/*                                                                           */
/*  KHE_TIME_GROUP KheSolnTimeGroupEnd(KHE_SOLN soln)                        */
/*                                                                           */
/*  End the construction of a time group and return it.                      */
/*                                                                           */
/*****************************************************************************/

KHE_TIME_GROUP KheSolnTimeGroupEnd(KHE_SOLN soln)
{
  KHE_TIME_GROUP res;
  MAssert(soln->curr_time_group != NULL,
    "KheSolnTimeGroupSubTime: time group not under construction");
  res = soln->curr_time_group;
  KheTimeGroupFinalize(res, NULL, -1);
  soln->curr_time_group = NULL;
  return res;
}


/*****************************************************************************/
/*                                                                           */
/*  Submodule "solution resource groups"                                     */
/*                                                                           */
/*****************************************************************************/

/*****************************************************************************/
/*                                                                           */
/*  void KheSolnResourceGroupBegin(KHE_SOLN soln, KHE_RESOURCE_TYPE rt,      */
/*    void *impl)                                                            */
/*                                                                           */
/*  Begin the construction of a soln resource group.                         */
/*                                                                           */
/*****************************************************************************/

void KheSolnResourceGroupBegin(KHE_SOLN soln, KHE_RESOURCE_TYPE rt)
{
  MAssert(rt != NULL, "KheSolnResourceGroupBegin: rt is NULL");
  MAssert(soln->curr_resource_group == NULL,
    "KheSolnResourceGroupBegin: resource group already under construction");
  soln->curr_resource_group = KheResourceGroupMakeInternal(
    KHE_RESOURCE_GROUP_TYPE_SOLN, rt, NULL, NULL);
}


/*****************************************************************************/
/*                                                                           */
/*  void KheSolnResourceGroupAddResource(KHE_SOLN soln, KHE_RESOURCE r)      */
/*                                                                           */
/*  Add a r to the resource group currently being constructed.               */
/*                                                                           */
/*****************************************************************************/

void KheSolnResourceGroupAddResource(KHE_SOLN soln, KHE_RESOURCE r)
{
  MAssert(soln->curr_resource_group != NULL,
    "KheSolnResourceGroupAddResource: no resource group under construction");
  KheResourceGroupUnionInternal(soln->curr_resource_group,
    KheResourceSingletonResourceGroup(r));
}


/*****************************************************************************/
/*                                                                           */
/*  void KheSolnResourceGroupSubResource(KHE_SOLN soln, KHE_RESOURCE r)      */
/*                                                                           */
/*  Take away r from the resource group currently being constructed.         */
/*                                                                           */
/*****************************************************************************/

void KheSolnResourceGroupSubResource(KHE_SOLN soln, KHE_RESOURCE r)
{
  MAssert(soln->curr_resource_group != NULL,
    "KheSolnResourceGroupSubResource: no resource group under construction");
  KheResourceGroupDifferenceInternal(soln->curr_resource_group,
    KheResourceSingletonResourceGroup(r));
}


/*****************************************************************************/
/*                                                                           */
/*  void KheSolnResourceGroupUnion(KHE_SOLN soln, KHE_RESOURCE_GROUP rg2)    */
/*                                                                           */
/*  Add a resource group to the resource group currently being constructed.  */
/*                                                                           */
/*****************************************************************************/

void KheSolnResourceGroupUnion(KHE_SOLN soln, KHE_RESOURCE_GROUP rg2)
{
  MAssert(soln->curr_resource_group != NULL,
    "KheSolnResourceGroupUnion:  no resource group under construction");
  KheResourceGroupUnionInternal(soln->curr_resource_group, rg2);
}


/*****************************************************************************/
/*                                                                           */
/*  void KheSolnResourceGroupIntersect(KHE_SOLN soln, KHE_RESOURCE_GROUP rg2)*/
/*                                                                           */
/*  Intersect a resource group to the resource group currently being         */
/*  constructed.                                                             */
/*                                                                           */
/*****************************************************************************/

void KheSolnResourceGroupIntersect(KHE_SOLN soln, KHE_RESOURCE_GROUP rg2)
{
  MAssert(soln->curr_resource_group != NULL,
    "KheSolnResourceGroupIntersect: no resource group under construction");
  KheResourceGroupIntersectInternal(soln->curr_resource_group, rg2);
}


/*****************************************************************************/
/*                                                                           */
/* void KheSolnResourceGroupDifference(KHE_SOLN soln, KHE_RESOURCE_GROUP rg2)*/
/*                                                                           */
/*  Take away rg2 from the resource group currently being constructed.       */
/*                                                                           */
/*****************************************************************************/

void KheSolnResourceGroupDifference(KHE_SOLN soln, KHE_RESOURCE_GROUP rg2)
{
  MAssert(soln->curr_resource_group != NULL,
    "KheSolnResourceGroupDifference: no resource group under construction");
  KheResourceGroupDifferenceInternal(soln->curr_resource_group, rg2);
}


/*****************************************************************************/
/*                                                                           */
/*  KHE_RESOURCE_GROUP KheSolnResourceGroupEnd(KHE_SOLN soln)                */
/*                                                                           */
/*  End the construction of a resource group and return it.                  */
/*                                                                           */
/*****************************************************************************/

KHE_RESOURCE_GROUP KheSolnResourceGroupEnd(KHE_SOLN soln)
{
  KHE_RESOURCE_GROUP res;
  MAssert(soln->curr_resource_group != NULL,
    "KheSolnResourceGroupSubResource: no resource group under construction");
  res = soln->curr_resource_group;
  KheResourceGroupSetResourcesArrayInternal(res);
  soln->curr_resource_group = NULL;
  return res;
}


/*****************************************************************************/
/*                                                                           */
/*  Submodule "solution event groups"                                        */
/*                                                                           */
/*****************************************************************************/

/*****************************************************************************/
/*                                                                           */
/*  void KheSolnEventGroupBegin(KHE_SOLN soln)                               */
/*                                                                           */
/*  Begin the construction of a soln event group.                            */
/*                                                                           */
/*****************************************************************************/

void KheSolnEventGroupBegin(KHE_SOLN soln)
{
  MAssert(soln->curr_event_group == NULL,
    "KheSolnEventGroupBegin: event group already under construction");
  soln->curr_event_group = KheEventGroupMakeInternal(KHE_EVENT_GROUP_TYPE_SOLN,
    soln->instance, KHE_EVENT_GROUP_KIND_ORDINARY, NULL, NULL);
}


/*****************************************************************************/
/*                                                                           */
/*  void KheSolnEventGroupAddEvent(KHE_SOLN soln, KHE_EVENT e)               */
/*                                                                           */
/*  Add a r to the event group currently being constructed.                  */
/*                                                                           */
/*****************************************************************************/

void KheSolnEventGroupAddEvent(KHE_SOLN soln, KHE_EVENT e)
{
  MAssert(soln->curr_event_group != NULL,
    "KheSolnEventGroupAddEvent: no event group under construction");
  KheEventGroupUnionInternal(soln->curr_event_group,
    KheEventSingletonEventGroup(e));
}


/*****************************************************************************/
/*                                                                           */
/*  void KheSolnEventGroupSubEvent(KHE_SOLN soln, KHE_EVENT e)               */
/*                                                                           */
/*  Take away r from the event group currently being constructed.            */
/*                                                                           */
/*****************************************************************************/

void KheSolnEventGroupSubEvent(KHE_SOLN soln, KHE_EVENT e)
{
  MAssert(soln->curr_event_group != NULL,
    "KheSolnEventGroupSubEvent: no event group under construction");
  KheEventGroupDifferenceInternal(soln->curr_event_group,
    KheEventSingletonEventGroup(e));
}


/*****************************************************************************/
/*                                                                           */
/*  void KheSolnEventGroupUnion(KHE_SOLN soln, KHE_EVENT_GROUP eg2)          */
/*                                                                           */
/*  Add a event group to the event group currently being constructed.        */
/*                                                                           */
/*****************************************************************************/

void KheSolnEventGroupUnion(KHE_SOLN soln, KHE_EVENT_GROUP eg2)
{
  MAssert(soln->curr_event_group != NULL,
    "KheSolnEventGroupUnion: no event group under construction");
  KheEventGroupUnionInternal(soln->curr_event_group, eg2);
}


/*****************************************************************************/
/*                                                                           */
/*  void KheSolnEventGroupIntersect(KHE_SOLN soln, KHE_EVENT_GROUP eg2)      */
/*                                                                           */
/*  Intersect a event group to the event group currently being constructed.  */
/*                                                                           */
/*****************************************************************************/

void KheSolnEventGroupIntersect(KHE_SOLN soln, KHE_EVENT_GROUP eg2)
{
  MAssert(soln->curr_event_group != NULL,
    "KheSolnEventGroupIntersect: no event group under construction");
  KheEventGroupIntersectInternal(soln->curr_event_group, eg2);
}


/*****************************************************************************/
/*                                                                           */
/*  void KheSolnEventGroupDifference(KHE_SOLN soln, KHE_EVENT_GROUP eg2)     */
/*                                                                           */
/*  Take away eg2 from the event group currently being constructed.          */
/*                                                                           */
/*****************************************************************************/

void KheSolnEventGroupDifference(KHE_SOLN soln, KHE_EVENT_GROUP eg2)
{
  MAssert(soln->curr_event_group != NULL,
   "KheSolnEventGroupDifference: no event group under construction");
  KheEventGroupDifferenceInternal(soln->curr_event_group, eg2);
}


/*****************************************************************************/
/*                                                                           */
/*  KHE_EVENT_GROUP KheSolnEventGroupEnd(KHE_SOLN soln)                      */
/*                                                                           */
/*  End the construction of a event group and return it.                     */
/*                                                                           */
/*****************************************************************************/

KHE_EVENT_GROUP KheSolnEventGroupEnd(KHE_SOLN soln)
{
  KHE_EVENT_GROUP res;
  MAssert(soln->curr_event_group != NULL,
    "KheSolnEventGroupSubEvent: no event group under construction");
  res = soln->curr_event_group;
  KheEventGroupSetEventsArrayInternal(res);
  soln->curr_event_group = NULL;
  return res;
}


/*****************************************************************************/
/*                                                                           */
/*  Submodule "traces"                                                       */
/*                                                                           */
/*****************************************************************************/

/*****************************************************************************/
/*                                                                           */
/*  KHE_TRACE KheSolnGetTraceFromFreeList(KHE_SOLN soln)                     */
/*                                                                           */
/*  Return a new trace from soln's free list, or NULL if none.               */
/*                                                                           */
/*****************************************************************************/

KHE_TRACE KheSolnGetTraceFromFreeList(KHE_SOLN soln)
{
  if( MArraySize(soln->free_traces) > 0 )
    return MArrayRemoveLast(soln->free_traces);
  else
    return NULL;
}


/*****************************************************************************/
/*                                                                           */
/*  void KheSolnAddTraceToFreeList(KHE_SOLN soln, KHE_TRACE t)               */
/*                                                                           */
/*  Add t to soln's free list of traces.                                     */
/*                                                                           */
/*****************************************************************************/

void KheSolnAddTraceToFreeList(KHE_SOLN soln, KHE_TRACE t)
{
  MArrayAddLast(soln->free_traces, t);
}


/*****************************************************************************/
/*                                                                           */
/*  Submodule "transactions"                                                 */
/*                                                                           */
/*****************************************************************************/

/*****************************************************************************/
/*                                                                           */
/*  KHE_TRANSACTION KheSolnGetTransactionFromFreeList(KHE_SOLN soln)         */
/*                                                                           */
/*  Return a new transaction from soln's free list, or NULL if none.         */
/*                                                                           */
/*****************************************************************************/

KHE_TRANSACTION KheSolnGetTransactionFromFreeList(KHE_SOLN soln)
{
  if( MArraySize(soln->free_transactions) > 0 )
    return MArrayRemoveLast(soln->free_transactions);
  else
    return NULL;
}


/*****************************************************************************/
/*                                                                           */
/*  void KheSolnAddTransactionToFreeList(KHE_SOLN soln, KHE_TRANSACTION t)   */
/*                                                                           */
/*  Add t to soln's free list of transactions.                               */
/*                                                                           */
/*****************************************************************************/

void KheSolnAddTransactionToFreeList(KHE_SOLN soln, KHE_TRANSACTION t)
{
  int pos;
  MAssert(!MArrayContains(soln->curr_transactions, t, &pos),
    "KheTransactionDelete called before KheTransactionMakeEnd");
  MArrayAddLast(soln->free_transactions, t);
}


/*****************************************************************************/
/*                                                                           */
/*  void KheSolnBeginTransaction(KHE_SOLN soln, KHE_TRANSACTION t)           */
/*                                                                           */
/*  Add t to soln's list of current transactions.                            */
/*                                                                           */
/*****************************************************************************/

void KheSolnBeginTransaction(KHE_SOLN soln, KHE_TRANSACTION t)
{
  MArrayAddLast(soln->curr_transactions, t);
}


/*****************************************************************************/
/*                                                                           */
/*  void KheSolnEndTransaction(KHE_SOLN soln, KHE_TRANSACTION t)             */
/*                                                                           */
/*  Delete t from soln's list of current transactions.  It must be present.  */
/*                                                                           */
/*****************************************************************************/

void KheSolnEndTransaction(KHE_SOLN soln, KHE_TRANSACTION t)
{
  int pos;
  if( !MArrayContains(soln->curr_transactions, t, &pos) )
    MAssert(false, "KheSolnEndTransaction internal error");
  MArrayRemove(soln->curr_transactions, pos);
}


/*****************************************************************************/
/*                                                                           */
/*  Submodule "transaction operation loading"                                */
/*                                                                           */
/*****************************************************************************/

/*****************************************************************************/
/*                                                                           */
/*  void KheSolnOpMeetMake(KHE_SOLN soln, KHE_MEET res)                      */
/*                                                                           */
/*  Inform soln that a call to KheMeetMake has occurred.                     */
/*                                                                           */
/*****************************************************************************/

void KheSolnOpMeetMake(KHE_SOLN soln, KHE_MEET res)
{
  KHE_TRANSACTION t;  int i;
  MArrayForEach(soln->curr_transactions, &t, &i)
    KheTransactionOpMeetMake(t, res);
}


/*****************************************************************************/
/*                                                                           */
/*  void KheSolnOpMeetDelete(KHE_SOLN soln)                                  */
/*                                                                           */
/*  Inform soln that a call to KheMeetDelete has occurred.                   */
/*                                                                           */
/*****************************************************************************/

void KheSolnOpMeetDelete(KHE_SOLN soln)
{
  KHE_TRANSACTION t;  int i;
  MArrayForEach(soln->curr_transactions, &t, &i)
    KheTransactionOpMeetDelete(t);
}


/*****************************************************************************/
/*                                                                           */
/*  void KheSolnOpMeetSplit(KHE_SOLN soln, KHE_MEET meet1, KHE_MEET meet2)   */
/*                                                                           */
/*  Inform soln that a call to KheMeetSplit has occurred.                    */
/*                                                                           */
/*****************************************************************************/

void KheSolnOpMeetSplit(KHE_SOLN soln, KHE_MEET meet1, KHE_MEET meet2)
{
  KHE_TRANSACTION t;  int i;
  MArrayForEach(soln->curr_transactions, &t, &i)
    KheTransactionOpMeetSplit(t, meet1, meet2);
}


/*****************************************************************************/
/*                                                                           */
/*  void KheSolnOpMeetMerge(KHE_SOLN soln)                                   */
/*                                                                           */
/*  Inform soln that a call to KheMeetMerge has occurred.                    */
/*                                                                           */
/*****************************************************************************/

void KheSolnOpMeetMerge(KHE_SOLN soln)
{
  KHE_TRANSACTION t;  int i;
  MArrayForEach(soln->curr_transactions, &t, &i)
    KheTransactionOpMeetMerge(t);
}


/*****************************************************************************/
/*                                                                           */
/*  void KheSolnOpMeetAssign(KHE_SOLN soln, KHE_MEET meet,                   */
/*    KHE_MEET target_meet, int target_offset)                               */
/*                                                                           */
/*  Inform soln that a call to KheMeetAssign has occurred.                   */
/*                                                                           */
/*****************************************************************************/

void KheSolnOpMeetAssign(KHE_SOLN soln, KHE_MEET meet,
  KHE_MEET target_meet, int target_offset)
{
  KHE_TRANSACTION t;  int i;
  MArrayForEach(soln->curr_transactions, &t, &i)
    KheTransactionOpMeetAssign(t, meet, target_meet, target_offset);
}


/*****************************************************************************/
/*                                                                           */
/*  void KheSolnOpMeetUnAssign(KHE_SOLN soln, KHE_MEET meet,                 */
/*    KHE_MEET target_meet, int target_offset)                               */
/*                                                                           */
/*  Inform soln that a call to KheMeetUnAssign has occurred.                 */
/*                                                                           */
/*****************************************************************************/

void KheSolnOpMeetUnAssign(KHE_SOLN soln, KHE_MEET meet,
  KHE_MEET target_meet, int target_offset)
{
  KHE_TRANSACTION t;  int i;
  MArrayForEach(soln->curr_transactions, &t, &i)
    KheTransactionOpMeetUnAssign(t, meet, target_meet, target_offset);
}


/*****************************************************************************/
/*                                                                           */
/*  void KheSolnOpMeetSetDomain(KHE_SOLN soln, KHE_MEET meet,                */
/*    KHE_TIME_GROUP old_tg, KHE_TIME_GROUP new_tg)                          */
/*                                                                           */
/*  Inform soln that a call to KheMeetSetDomain has occurred.                */
/*                                                                           */
/*****************************************************************************/

void KheSolnOpMeetSetDomain(KHE_SOLN soln, KHE_MEET meet,
  KHE_TIME_GROUP old_tg, KHE_TIME_GROUP new_tg)
{
  KHE_TRANSACTION t;  int i;
  MArrayForEach(soln->curr_transactions, &t, &i)
    KheTransactionOpMeetSetDomain(t, meet, old_tg, new_tg);
}


/*****************************************************************************/
/*                                                                           */
/*  void KheSolnOpTaskMake(KHE_SOLN soln, KHE_TASK res)                      */
/*                                                                           */
/*  Inform soln that a call to KheTaskMake has occurred.                     */
/*                                                                           */
/*****************************************************************************/

void KheSolnOpTaskMake(KHE_SOLN soln, KHE_TASK res)
{
  KHE_TRANSACTION t;  int i;
  MArrayForEach(soln->curr_transactions, &t, &i)
    KheTransactionOpTaskMake(t, res);
}


/*****************************************************************************/
/*                                                                           */
/*  void KheSolnOpTaskDelete(KHE_SOLN soln)                                  */
/*                                                                           */
/*  Inform soln that a call to KheTaskDelete has occurred.                   */
/*                                                                           */
/*****************************************************************************/

void KheSolnOpTaskDelete(KHE_SOLN soln)
{
  KHE_TRANSACTION t;  int i;
  MArrayForEach(soln->curr_transactions, &t, &i)
    KheTransactionOpTaskDelete(t);
}


/*****************************************************************************/
/*                                                                           */
/*  void KheSolnOpTaskAssign(KHE_SOLN soln, KHE_TASK task,                   */
/*    KHE_TASK target_task)                                                  */
/*                                                                           */
/*  Inform soln that a call to KheTaskAssign has occurred.                   */
/*                                                                           */
/*****************************************************************************/

void KheSolnOpTaskAssign(KHE_SOLN soln, KHE_TASK task, KHE_TASK target_task)
{
  KHE_TRANSACTION t;  int i;
  MArrayForEach(soln->curr_transactions, &t, &i)
    KheTransactionOpTaskAssign(t, task, target_task);
}


/*****************************************************************************/
/*                                                                           */
/*  void KheSolnOpTaskUnAssign(KHE_SOLN soln, KHE_TASK task,                 */
/*    KHE_TASK target_task)                                                  */
/*                                                                           */
/*  Inform soln that a call to KheTaskUnAssign has occurred.                 */
/*                                                                           */
/*****************************************************************************/

void KheSolnOpTaskUnAssign(KHE_SOLN soln, KHE_TASK task, KHE_TASK target_task)
{
  KHE_TRANSACTION t;  int i;
  MArrayForEach(soln->curr_transactions, &t, &i)
    KheTransactionOpTaskUnAssign(t, task, target_task);
}


/*****************************************************************************/
/*                                                                           */
/*  void KheSolnOpTaskSetDomain(KHE_SOLN soln, KHE_TASK task,                */
/*    KHE_RESOURCE_GROUP old_rg, KHE_RESOURCE_GROUP new_rg)                  */
/*                                                                           */
/*  Inform soln that a call to KheTaskSetDomain has occurred.                */
/*                                                                           */
/*****************************************************************************/

void KheSolnOpTaskSetDomain(KHE_SOLN soln, KHE_TASK task,
  KHE_RESOURCE_GROUP old_rg, KHE_RESOURCE_GROUP new_rg)
{
  KHE_TRANSACTION t;  int i;
  MArrayForEach(soln->curr_transactions, &t, &i)
    KheTransactionOpTaskSetDomain(t, task, old_rg, new_rg);
}


/*****************************************************************************/
/*                                                                           */
/*  void KheSolnOpNodeAddParent(KHE_SOLN soln,                               */
/*    KHE_NODE child_node, KHE_NODE parent_node)                             */
/*                                                                           */
/*  Inform soln that a call to KheNodeAddParent has occurred.                */
/*                                                                           */
/*****************************************************************************/

void KheSolnOpNodeAddParent(KHE_SOLN soln,
  KHE_NODE child_node, KHE_NODE parent_node)
{
  KHE_TRANSACTION t;  int i;
  MArrayForEach(soln->curr_transactions, &t, &i)
    KheTransactionOpNodeAddParent(t, child_node, parent_node);
}


/*****************************************************************************/
/*                                                                           */
/*  void KheSolnOpNodeDeleteParent(KHE_SOLN soln,                            */
/*    KHE_NODE child_node, KHE_NODE parent_node)                             */
/*                                                                           */
/*  Inform soln that a call to KheNodeDeleteParent has occurred.             */
/*                                                                           */
/*****************************************************************************/

void KheSolnOpNodeDeleteParent(KHE_SOLN soln,
  KHE_NODE child_node, KHE_NODE parent_node)
{
  KHE_TRANSACTION t;  int i;
  MArrayForEach(soln->curr_transactions, &t, &i)
    KheTransactionOpNodeDeleteParent(t, child_node, parent_node);
}


/*****************************************************************************/
/*                                                                           */
/*  Submodule "diversifiers"                                                 */
/*                                                                           */
/*****************************************************************************/

/*****************************************************************************/
/*                                                                           */
/*  void KheSolnSetDiversifier(KHE_SOLN soln, int val)                       */
/*                                                                           */
/*  Set the diversifier of soln to val.                                      */
/*                                                                           */
/*****************************************************************************/

void KheSolnSetDiversifier(KHE_SOLN soln, int val)
{
  soln->diversifier = val;
}


/*****************************************************************************/
/*                                                                           */
/*  int KheSolnDiversifier(KHE_SOLN soln)                                    */
/*                                                                           */
/*  Return the diversifier of soln.                                          */
/*                                                                           */
/*****************************************************************************/

int KheSolnDiversifier(KHE_SOLN soln)
{
  return soln->diversifier;
}


/*****************************************************************************/
/*                                                                           */
/*  int KheSolnDiversifierChoose(KHE_SOLN soln, int c)                       */
/*                                                                           */
/*  Choose an integer i in the range 0 <= i < c using soln's diversifier.    */
/*                                                                           */
/*****************************************************************************/

int KheSolnDiversifierChoose(KHE_SOLN soln, int c)
{
  int i, c1f;
  c1f = 1;
  for( i = 1;  i < c && c1f <= soln->diversifier;  i++ )
    c1f *= i;
  return ((soln->diversifier / c1f) + (soln->diversifier % c1f)) % c;
}


/*****************************************************************************/
/*                                                                           */
/*  Submodule "visit numbers"                                                */
/*                                                                           */
/*****************************************************************************/

/*****************************************************************************/
/*                                                                           */
/*  void KheSolnSetVisitNum(KHE_SOLN soln, int num)                          */
/*                                                                           */
/*  Set soln's visit number to num.                                          */
/*                                                                           */
/*****************************************************************************/

void KheSolnSetVisitNum(KHE_SOLN soln, int num)
{
  soln->visit_num = num;
}


/*****************************************************************************/
/*                                                                           */
/*  int KheSolnVisitNum(KHE_SOLN soln)                                       */
/*                                                                           */
/*  Return soln's visit number.                                              */
/*                                                                           */
/*****************************************************************************/

int KheSolnVisitNum(KHE_SOLN soln)
{
  return soln->visit_num;
}


/*****************************************************************************/
/*                                                                           */
/*  void KheSolnNewVisit(KHE_SOLN soln)                                      */
/*                                                                           */
/*  Increment soln's visit number.                                           */
/*                                                                           */
/*****************************************************************************/

void KheSolnNewVisit(KHE_SOLN soln)
{
  soln->visit_num++;
}


/*****************************************************************************/
/*                                                                           */
/*  Submodule "reading and writing"                                          */
/*                                                                           */
/*****************************************************************************/

/*****************************************************************************/
/*                                                                           */
/*  bool KheSolnMakeFromKml(KML_ELT soln_elt, KHE_SOLN_GROUP soln_group,     */
/*    KML_ERROR *ke)                                                         */
/*                                                                           */
/*  Make a solution based on soln_elt and add it to soln_group.              */
/*                                                                           */
/*****************************************************************************/

bool KheSolnMakeFromKml(KML_ELT soln_elt, KHE_SOLN_GROUP soln_group,
  KML_ERROR *ke)
{
  KML_ELT events_elt, event_elt;  KHE_SOLN res;  char *ref;  KHE_EVENT e;
  KHE_INSTANCE ins;  int i;

  /* check soln_elt, find ins, and make res, the result object */
  if( DEBUG2 )
    fprintf(stderr, "[ KheSolnMakeFromKml(soln_elt)\n");
  if( !KmlCheck(soln_elt, "Reference : +Events +Report", ke) )
    return false;
  ref = KmlAttributeValue(soln_elt, 0);
  if( !KheArchiveRetrieveInstance(KheSolnGroupArchive(soln_group), ref, &ins) )
    return KmlErrorMake(ke, KmlLineNum(soln_elt), KmlColNum(soln_elt),
      "<Solution> Reference \"%s\" unknown", ref);
  res = KheSolnMake(ins, soln_group);

  /* Events */
  if( KmlContainsChild(soln_elt, "Events", &events_elt) )
  {
    if( !KmlCheck(events_elt, ": *Event", ke) )
      return false;
    for( i = 0;  i < KmlChildCount(events_elt);  i++ )
    {
      event_elt = KmlChild(events_elt, i);
      if( !KheMeetMakeFromKml(event_elt, res, ke) )
	return false;
    }
  }

  /* make sure event durations are correct, then add any missing ones */
  if( !KheSolnMakeCompleteRepresentation(res, &e) )
    return KmlErrorMake(ke, KmlLineNum(soln_elt), KmlColNum(soln_elt),
      "<Solution> invalid total duration of meets of event \"%s\"",
      KheEventId(e));

  /* convert preassignments into assignments */
  KheSolnAssignPreassignedTimes(res, true);
  KheSolnAssignPreassignedResources(res, NULL, true);

  if( DEBUG2 )
  {
    KheSolnDebug(res, 2, 2, stderr);
    fprintf(stderr, "] KheSolnMakeFromKml returning\n");
  }
  return true;
}


/*****************************************************************************/
/*                                                                           */
/*  void KheSolnResourcesReportWrite(KHE_SOLN soln, KML_FILE kf)             */
/*                                                                           */
/*  Write the Resources part of the report on soln.                          */
/*                                                                           */
/*****************************************************************************/

static void KheSolnResourcesReportWrite(KHE_SOLN soln, KML_FILE kf)
{
  bool section_started, resource_started;  KHE_CONSTRAINT c;
  KHE_INSTANCE ins;  KHE_RESOURCE r;  int i, j, cost;  KHE_MONITOR m;
  section_started = false;
  ins = KheSolnInstance(soln);
  for( i = 0;  i < KheInstanceResourceCount(ins);  i++ )
  {
    r = KheInstanceResource(ins, i);
    resource_started = false;
    for( j = 0;  j < KheResourceMonitorCount(soln, r);  j++ )
    {
      m = KheResourceMonitor(soln, r, j);
      c = KheMonitorConstraint(m);
      if( c != NULL && KheMonitorCost(m) > 0 )
      {
	if( !section_started )
	{
	  KmlBegin(kf, "Resources");
	  section_started = true;
	}
	if( !resource_started )
	{
	  KmlBegin(kf, "Resource");
	  KmlAttribute(kf, "Reference", KheResourceId(r));
	  resource_started = true;
	}
        KmlBegin(kf, "Constraint");
	KmlAttribute(kf, "Reference", KheConstraintId(c));
	cost = KheConstraintRequired(c) ? KheHardCost(KheMonitorCost(m)) :
	  KheSoftCost(KheMonitorCost(m));
	KmlEltPrintf(kf, "Cost", "%d", cost);
	/* no Description at present */
        KmlEnd(kf, "Constraint");
      }
    }
    if( resource_started )
      KmlEnd(kf, "Resource");
  }
  if( section_started )
    KmlEnd(kf, "Resources");
}


/*****************************************************************************/
/*                                                                           */
/*  bool KheMonitorReportWithEvent(KHE_MONITOR_TAG tag)                      */
/*                                                                           */
/*  Return true if tag tags a monitor that is reported in the Events         */
/*  section of a report.                                                     */
/*                                                                           */
/*****************************************************************************/

static bool KheMonitorReportWithEvent(KHE_MONITOR_TAG tag)
{
  return tag == KHE_ASSIGN_RESOURCE_MONITOR_TAG ||
    tag == KHE_ASSIGN_TIME_MONITOR_TAG ||
    tag == KHE_SPLIT_EVENTS_MONITOR_TAG ||
    tag == KHE_DISTRIBUTE_SPLIT_EVENTS_MONITOR_TAG ||
    tag == KHE_PREFER_RESOURCES_MONITOR_TAG ||
    tag == KHE_PREFER_TIMES_MONITOR_TAG;
}


/*****************************************************************************/
/*                                                                           */
/*  void KheHandleEventMonitor(KHE_SOLN soln, KML_FILE kf, KHE_MONITOR m,    */
/*    KHE_EVENT e, bool *section_started, bool *event_started)               */
/*                                                                           */
/*  Handle the reporting of monitor m, if relevant with non-zero cost.       */
/*                                                                           */
/*****************************************************************************/

static void KheHandleEventMonitor(KHE_SOLN soln, KML_FILE kf, KHE_MONITOR m,
  KHE_EVENT e, bool *section_started, bool *event_started)
{
  KHE_CONSTRAINT c;  int cost;
  if( KheMonitorReportWithEvent(KheMonitorTag(m)) && KheMonitorCost(m) > 0 )
  {
    c = KheMonitorConstraint(m);
    if( !*section_started )
    {
      KmlBegin(kf, "Events");
      *section_started = true;
    }
    if( !*event_started )
    {
      KmlBegin(kf, "Event");
      KmlAttribute(kf, "Reference", KheEventId(e));
      *event_started = true;
    }
    KmlBegin(kf, "Constraint");
    KmlAttribute(kf, "Reference", KheConstraintId(c));
    cost = KheConstraintRequired(c) ? KheHardCost(KheMonitorCost(m)) :
      KheSoftCost(KheMonitorCost(m));
    KmlEltPrintf(kf, "Cost", "%d", cost);
    /* no Description at present */
    KmlEnd(kf, "Constraint");
  }
}


/*****************************************************************************/
/*                                                                           */
/*  void KheEventsReportWrite(KHE_SOLN soln, KML_FILE kf)                    */
/*                                                                           */
/*  Write the Events part of the report on soln.                             */
/*                                                                           */
/*****************************************************************************/

static void KheEventsReportWrite(KHE_SOLN soln, KML_FILE kf)
{
  bool section_started, event_started;  KHE_EVENT_RESOURCE er;
  KHE_INSTANCE ins;  KHE_EVENT e;  int i, j, k;  KHE_MONITOR m;
  section_started = false;
  ins = KheSolnInstance(soln);
  for( i = 0;  i < KheInstanceEventCount(ins);  i++ )
  {
    e = KheInstanceEvent(ins, i);
    event_started = false;
    for( j = 0;  j < KheEventMonitorCount(soln, e);  j++ )
    {
      m = KheEventMonitor(soln, e, j);
      KheHandleEventMonitor(soln, kf, m, e, &section_started, &event_started);
    }
    for( j = 0;  j < KheEventResourceCount(e);  j++ )
    {
      er = KheEventResource(e, j);
      for( k = 0;  k < KheEventResourceMonitorCount(soln, er);  k++ )
      {
	m = KheEventResourceMonitor(soln, er, k);
	KheHandleEventMonitor(soln, kf, m, e, &section_started, &event_started);
      }
    }
    if( event_started )
      KmlEnd(kf, "Event");
  }
  if( section_started )
    KmlEnd(kf, "Events");
}


/*****************************************************************************/
/*                                                                           */
/*  bool KheMonitorReportWithEventGroup(KHE_MONITOR m, KHE_EVENT_GROUP eg)   */
/*                                                                           */
/*  Return true if m is to be reported in eg's section of the report.        */
/*                                                                           */
/*****************************************************************************/

static bool KheMonitorReportWithEventGroup(KHE_MONITOR m, KHE_EVENT_GROUP eg)
{
  KHE_SPREAD_EVENTS_MONITOR sem;  KHE_AVOID_SPLIT_ASSIGNMENTS_MONITOR asam;
  KHE_AVOID_SPLIT_ASSIGNMENTS_CONSTRAINT c;  int i;
  switch( KheMonitorTag(m) )
  {
    case KHE_AVOID_SPLIT_ASSIGNMENTS_MONITOR_TAG:

      asam = (KHE_AVOID_SPLIT_ASSIGNMENTS_MONITOR) m;
      c = KheAvoidSplitAssignmentsMonitorConstraint(asam);
      i = KheAvoidSplitAssignmentsMonitorEventGroupIndex(asam);
      return KheAvoidSplitAssignmentsConstraintEventGroup(c, i) == eg;

    case KHE_SPREAD_EVENTS_MONITOR_TAG:

      sem = (KHE_SPREAD_EVENTS_MONITOR) m;
      return KheSpreadEventsMonitorEventGroup(sem) == eg;

    case KHE_LINK_EVENTS_MONITOR_TAG:

      return KheLinkEventsMonitorEventGroup((KHE_LINK_EVENTS_MONITOR) m) == eg;

    default:

      return false;
  }
}


/*****************************************************************************/
/*                                                                           */
/*  void KheEventGroupsReportWrite(KHE_SOLN soln, KML_FILE kf)               */
/*                                                                           */
/*  Write the EventGroups part of the report on soln to ktf.                 */
/*                                                                           */
/*****************************************************************************/

static void KheEventGroupsReportWrite(KHE_SOLN soln, KML_FILE kf)
{
  bool section_started;  KHE_EVENT_GROUP eg;  KHE_CONSTRAINT c;
  KHE_INSTANCE ins;  KHE_EVENT e;  int i, j, k, n, pos, cost;
  ARRAY_KHE_MONITOR eg_monitors;  KHE_MONITOR m;  KHE_EVENT_RESOURCE er;
  section_started = false;
  ins = KheSolnInstance(soln);
  MArrayInit(eg_monitors);
  for( i = 0;  i < KheInstanceEventGroupCount(ins);  i++ )
  {
    eg = KheInstanceEventGroup(ins, i);

    /* find the monitors of event group eg */
    MArrayClear(eg_monitors);
    for( j = 0;  j < KheEventGroupEventCount(eg);  j++ )
    {
      e = KheEventGroupEvent(eg, j);
      for( k = 0;  k < KheEventMonitorCount(soln, e);  k++ )
      {
	m = KheEventMonitor(soln, e, k);
	if( KheMonitorReportWithEventGroup(m, eg) &&
	    KheMonitorCost(m) > 0 && !MArrayContains(eg_monitors, m, &pos) )
	  MArrayAddLast(eg_monitors, m);
      }
      for( k = 0;  k < KheEventResourceCount(e);  k++ )
      {
	er = KheEventResource(e, k);
	for( n = 0;  n < KheEventResourceMonitorCount(soln, er);  n++ )
	{
	  m = KheEventResourceMonitor(soln, er, n);
	  if( KheMonitorReportWithEventGroup(m, eg) &&
	      KheMonitorCost(m) > 0 && !MArrayContains(eg_monitors, m, &pos) )
	    MArrayAddLast(eg_monitors, m);
	}
      }
    }

    /* print the report for event group eg, if any relevant monitors */
    if( MArraySize(eg_monitors) > 0 )
    {
      if( !section_started )
      {
	KmlBegin(kf, "EventGroups");
	section_started = true;
      }
      KmlBegin(kf, "EventGroup");
      KmlAttribute(kf, "Reference", KheEventGroupId(eg));
      MArrayForEach(eg_monitors, &m, &k)
      {
	c = KheMonitorConstraint(m);
	KmlBegin(kf, "Constraint");
	KmlAttribute(kf, "Reference", KheConstraintId(c));
	cost = KheConstraintRequired(c) ? KheHardCost(KheMonitorCost(m)) :
	  KheSoftCost(KheMonitorCost(m));
	KmlEltPrintf(kf, "Cost", "%d", cost);
	/* no Description at present */
	KmlEnd(kf, "Constraint");
      }
      KmlEnd(kf, "EventGroup");
    }
  }
  if( section_started )
    KmlEnd(kf, "EventGroups");
}


/*****************************************************************************/
/*                                                                           */
/*  bool KheSolnWrite(KHE_SOLN soln, bool with_reports, KML_FILE kf)         */
/*                                                                           */
/*  Write soln to kf, with a report if with_reports is true.                 */
/*                                                                           */
/*****************************************************************************/

bool KheSolnWrite(KHE_SOLN soln, bool with_reports, KML_FILE kf)
{
  KHE_EVENT_IN_SOLN es;  int i;  bool started;
  KmlBegin(kf, "Solution");
  KmlAttribute(kf, "Reference", KheInstanceId(soln->instance));
  started = false;
  MArrayForEach(soln->events_in_soln, &es, &i)
    if( KheEventInSolnMeetCount(es) > 0 )
    {
      if( !started )
      {
	KmlBegin(kf, "Events");
	started = true;
      }
      if( !KheEventInSolnWrite(es, kf) )
	return false;
    }
  if( started )
    KmlEnd(kf, "Events");
  if( with_reports )
  {
    KmlBegin(kf, "Report");
    KmlEltPrintf(kf, "InfeasibilityValue","%d",KheHardCost(KheSolnCost(soln)));
    KmlEltPrintf(kf, "ObjectiveValue", "%d", KheSoftCost(KheSolnCost(soln)));
    KheSolnResourcesReportWrite(soln, kf);
    KheEventsReportWrite(soln, kf);
    KheEventGroupsReportWrite(soln, kf);
    KmlEnd(kf, "Report");
  }
  KmlEnd(kf, "Solution");
  return true;
}


/*****************************************************************************/
/*                                                                           */
/*  Submodule "matchings - zero domain"                                      */
/*                                                                           */
/*****************************************************************************/

/*****************************************************************************/
/*                                                                           */
/*  ARRAY_SHORT KheSolnMatchingZeroDomain(KHE_SOLN soln)                     */
/*                                                                           */
/*  Return a domain containing just 0.                                       */
/*                                                                           */
/*****************************************************************************/

ARRAY_SHORT KheSolnMatchingZeroDomain(KHE_SOLN soln)
{
  return soln->matching_zero_domain;
}


/*****************************************************************************/
/*                                                                           */
/*  Submodule "matchings - free supply chunks"                               */
/*                                                                           */
/*****************************************************************************/

/*****************************************************************************/
/*                                                                           */
/*  KHE_MATCHING_SUPPLY_CHUNK KheSolnMatchingMakeOrdinarySupplyChunk(        */
/*    KHE_SOLN soln, KHE_MEET meet)                                          */
/*                                                                           */
/*  Return a fresh ordinary supply chunk, with one node for each resource    */
/*  of the instance, either taken from a free list or made from scratch.     */
/*  Set the impl field of the supply chunk to meet.                          */
/*                                                                           */
/*****************************************************************************/

KHE_MATCHING_SUPPLY_CHUNK KheSolnMatchingMakeOrdinarySupplyChunk(
  KHE_SOLN soln, KHE_MEET meet)
{
  KHE_MATCHING_SUPPLY_CHUNK res;  KHE_INSTANCE ins;  int i;
  if( MArraySize(soln->matching_free_supply_chunks) > 0 )
    res = MArrayRemoveLast(soln->matching_free_supply_chunks);
  else
  {
    ins = KheSolnInstance(soln);
    res = KheMatchingSupplyChunkMake(soln->matching, NULL);
    for( i = 0;  i < KheInstanceResourceCount(ins);  i++ )
      KheMatchingSupplyNodeMake(res, (void *) KheInstanceResource(ins, i));
  }
  KheMatchingSupplyChunkSetImpl(res, meet);
  return res;
}


/*****************************************************************************/
/*                                                                           */
/*  void KheSolnMatchingAddOrdinarySupplyChunkToFreeList(KHE_SOLN soln,      */
/*    KHE_MATCHING_SUPPLY_CHUNK sc)                                          */
/*                                                                           */
/*  Save sc for reuse later.                                                 */
/*                                                                           */
/*****************************************************************************/

void KheSolnMatchingAddOrdinarySupplyChunkToFreeList(KHE_SOLN soln,
  KHE_MATCHING_SUPPLY_CHUNK sc)
{
  MArrayAddLast(soln->matching_free_supply_chunks, sc);
}


/*****************************************************************************/
/*                                                                           */
/*  Submodule "matchings - setting up"                                       */
/*                                                                           */
/*****************************************************************************/

/*****************************************************************************/
/*                                                                           */
/*  KHE_MATCHING KheSolnMatching(KHE_SOLN soln)                              */
/*                                                                           */
/*  Return the matching held by soln.                                        */
/*                                                                           */
/*****************************************************************************/

KHE_MATCHING KheSolnMatching(KHE_SOLN soln)
{
  return soln->matching;
}


/*****************************************************************************/
/*                                                                           */
/*  void KheSolnMatchingUpdate(KHE_SOLN soln)                                */
/*                                                                           */
/*  Bring soln's matching up to date.                                        */
/*                                                                           */
/*****************************************************************************/

void KheSolnMatchingUpdate(KHE_SOLN soln)
{
  KheMatchingUnmatchedDemandNodeCount(soln->matching);
}


/*****************************************************************************/
/*                                                                           */
/*  void KheSolnMatchingSetWeight(KHE_SOLN soln, KHE_COST matching_weight)   */
/*                                                                           */
/*  Set the weight that the matching is to have in the total cost.           */
/*                                                                           */
/*****************************************************************************/

void KheSolnMatchingSetWeight(KHE_SOLN soln, KHE_COST matching_weight)
{
  KHE_RESOURCE_IN_SOLN rs;  KHE_MEET meet;  int i;
  if( matching_weight != soln->matching_weight )
  {
    MArrayForEach(soln->resources_in_soln, &rs, &i)
      KheResourceInSolnMatchingSetWeight(rs, matching_weight);
    MArrayForEach(soln->meets, &meet, &i)
      KheMeetMatchingSetWeight(meet, matching_weight);
    soln->matching_weight = matching_weight;
  }
}


/*****************************************************************************/
/*                                                                           */
/*  KHE_COST KheSolnMatchingWeight(KHE_SOLN soln)                            */
/*                                                                           */
/*  Return the weight of matching in soln.                                   */
/*                                                                           */
/*****************************************************************************/

KHE_COST KheSolnMatchingWeight(KHE_SOLN soln)
{
  return soln->matching_weight;
}


/*****************************************************************************/
/*                                                                           */
/*  KHE_COST KheSolnMinMatchingWeight(KHE_SOLN soln)                         */
/*                                                                           */
/*  Return a suitable value for the matching_weight field of soln, being     */
/*  the minimum of all the following quantities:                             */
/*                                                                           */
/*    * for each resource r, the sum of the combined weights of the avoid    */
/*      clashes constraints applicable to r;                                 */
/*                                                                           */
/*    * for each event resource er, the sum of the combined weights of the   */
/*      assign resource constraints applicable to er.                        */
/*                                                                           */
/*  If there are no such quantities, the value returned is 0.                */
/*                                                                           */
/*****************************************************************************/

KHE_COST KheSolnMinMatchingWeight(KHE_SOLN soln)
{
  int i, j, k;  KHE_COST weight, res;  KHE_EVENT e;  KHE_EVENT_RESOURCE er;
  KHE_RESOURCE r;  KHE_INSTANCE ins;  KHE_CONSTRAINT c;

  /* avoid clashes constraints */
  ins = KheSolnInstance(soln);
  res = KheCostMax;
  for( i = 0;  i < KheInstanceResourceCount(ins);  i++ )
  {
    r = KheInstanceResource(ins, i);
    weight = 0;
    for( j = 0;  j < KheResourceConstraintCount(r);  j++ )
    {
      c = KheResourceConstraint(r, j);
      if( KheConstraintTag(c) == KHE_AVOID_CLASHES_CONSTRAINT_TAG )
	weight += KheConstraintCombinedWeight(c);
    }
    if( weight < res )
      res = weight;
  }

  /* assign resource constraints */
  for( i = 0;  i < KheInstanceEventCount(ins);  i++ )
  {
    e = KheInstanceEvent(ins, i);
    for( j = 0;  j < KheEventResourceCount(e);  j++ )
    {
      er = KheEventResource(e, j);
      weight = 0;
      for( k = 0;  k < KheEventResourceConstraintCount(er);  k++ )
      {
	c = KheEventResourceConstraint(er, k);
	if( KheConstraintTag(c) == KHE_ASSIGN_RESOURCE_CONSTRAINT_TAG )
	  weight += KheConstraintCombinedWeight(c);
      }
      if( weight < res )
	res = weight;
    }
  }

  /* final value is res, or 0 if no cases */
  return res == KheCostMax ? 0 : res;
}


/*****************************************************************************/
/*                                                                           */
/*  KHE_MATCHING_TYPE KheSolnMatchingType(KHE_SOLN soln)                     */
/*                                                                           */
/*  Return the type of soln's matching.                                      */
/*                                                                           */
/*****************************************************************************/

KHE_MATCHING_TYPE KheSolnMatchingType(KHE_SOLN soln)
{
  return soln->matching_type;
}


/*****************************************************************************/
/*                                                                           */
/*  void KheSolnMatchingSetType(KHE_SOLN soln, KHE_MATCHING_TYPE mt)         */
/*                                                                           */
/*  Set the type of soln's matching.                                         */
/*                                                                           */
/*****************************************************************************/

void KheSolnMatchingSetType(KHE_SOLN soln, KHE_MATCHING_TYPE mt)
{
  /* KHE_RESOURCE_IN_SOLN rs; */  KHE_MEET meet;  int i;  KHE_TASK task;
  if( mt != soln->matching_type )
  {
    /* *** workload demand monitors are unaffected by type
    MArrayForEach(soln->resources_in_soln, &rs, &i)
      KheResourceInSolnMatchingSetType(rs, mt);
    *** */
    soln->matching_type = mt;  /* must come first */
    MArrayForEach(soln->meets, &meet, &i)
      KheMeetMatchingReset(meet);
    MArrayForEach(soln->tasks, &task, &i)
      KheTaskMatchingReset(task);
  }
}


/*****************************************************************************/
/*                                                                           */
/*  void KheSolnMatchingMarkBegin(KHE_SOLN soln)                             */
/*                                                                           */
/*  Begin a bracketed section of code that might return the matching to      */
/*  its initial state.                                                       */
/*                                                                           */
/*****************************************************************************/

void KheSolnMatchingMarkBegin(KHE_SOLN soln)
{
  KheMatchingMarkBegin(soln->matching);
}


/*****************************************************************************/
/*                                                                           */
/*  void KheSolnMatchingMarkEnd(KHE_SOLN soln, bool undo)                    */
/*                                                                           */
/*  End a bracketed section of code that might return the matching to        */
/*  its state at the start of the bracketing.  Parameter undo should be      */
/*  true if indeed it did return the matching to that state.                 */
/*                                                                           */
/*****************************************************************************/

void KheSolnMatchingMarkEnd(KHE_SOLN soln, bool undo)
{
  KheMatchingMarkEnd(soln->matching, undo);
}


/*****************************************************************************/
/*                                                                           */
/*  bool KheSolnSupplyNodeIsOrdinary(KHE_MATCHING_SUPPLY_NODE sn,            */
/*    KHE_MEET *meet, int *meet_offset, KHE_RESOURCE *r)                     */
/*                                                                           */
/*  If sn is an ordinary supply node (which at present it always is), set    */
/*  *meet to the meet it lies in, *meet_offset to its offset in that meet,   */
/*  and *r to the resource it represents, and return true.                   */
/*                                                                           */
/*****************************************************************************/

static bool KheSolnSupplyNodeIsOrdinary(KHE_MATCHING_SUPPLY_NODE sn,
  KHE_MEET *meet, int *meet_offset, KHE_RESOURCE *r)
{
  *meet = (KHE_MEET)
    KheMatchingSupplyChunkImpl(KheMatchingSupplyNodeChunk(sn));
  *meet_offset = KheMeetSupplyNodeOffset(*meet, sn);
  *r = (KHE_RESOURCE) KheMatchingSupplyNodeImpl(sn);
  return true;
}


/*****************************************************************************/
/*                                                                           */
/*  void KheSupplyNodeDebug(KHE_MATCHING_SUPPLY_NODE sn, int verbosity,      */
/*    int indent, FILE *fp)                                                  */
/*                                                                           */
/*  Show supply node sn onto fp.                                             */
/*                                                                           */
/*****************************************************************************/

void KheSupplyNodeDebug(KHE_MATCHING_SUPPLY_NODE sn, int verbosity,
  int indent, FILE *fp)
{
  KHE_MEET meet;  int meet_offset;  KHE_RESOURCE r;  KHE_TIME t;
  KHE_INSTANCE ins;  KHE_EVENT e;
  if( !KheSolnSupplyNodeIsOrdinary(sn, &meet, &meet_offset, &r) )
    MAssert(false, "KheSupplyNodeDebug internal error");
  if( verbosity >= 1 )
  {
    if( indent >= 0 )
      fprintf(fp, "%*s", indent, "");
    fprintf(fp, "[ ");
    ins = KheSolnInstance(KheMeetSoln(meet));
    if( KheMeetIsCycleMeet(meet) )
    {
      t = KheInstanceTime(ins, KheMeetAssignedTimeIndex(meet) + meet_offset);
      fprintf(fp, "%s:%s", KheResourceId(r) != NULL ? KheResourceId(r) : "-",
	KheTimeId(t) != NULL ? KheTimeId(t) : "-");
    }
    else
    {
      e = KheMeetEvent(meet);
      fprintf(fp, "%s:%s+%d", KheResourceId(r) != NULL ? KheResourceId(r) : "-",
	e==NULL ? "~" : KheEventId(e)==NULL ? "-" : KheEventId(e), meet_offset);
    }
    fprintf(fp, " ]");
    if( indent >= 0 )
      fprintf(fp, "\n");
  }
}


/*****************************************************************************/
/*                                                                           */
/*  void KheSolnMatchingDebug(KHE_SOLN soln, int verbosity,                  */
/*    int indent, FILE *fp)                                                  */
/*                                                                           */
/*  Debug print of soln's matching onto fp with the given verbosity and      */
/*  indent.                                                                  */
/*                                                                           */
/*****************************************************************************/

static void KheDemandNodeDebug(KHE_MATCHING_DEMAND_NODE dn, int verbosity,
  int indent, FILE *fp)
{
  KheMonitorDebug((KHE_MONITOR) dn, verbosity, indent, fp);
}

void KheSolnMatchingDebug(KHE_SOLN soln, int verbosity, int indent, FILE *fp)
{
  KheSolnMatchingUpdate(soln);
  KheMatchingDebug(soln->matching, &KheSupplyNodeDebug, &KheDemandNodeDebug,
    verbosity, indent, fp);
}


/*****************************************************************************/
/*                                                                           */
/*  Submodule "matchings - ordinary supply and demand nodes"                 */
/*                                                                           */
/*****************************************************************************/

/*****************************************************************************/
/*                                                                           */
/*  void KheSolnMatchingAttachAllOrdinaryDemandMonitors(KHE_SOLN soln)       */
/*                                                                           */
/*  Make sure all the ordinary demand monitors of soln are attached.         */
/*                                                                           */
/*****************************************************************************/

void KheSolnMatchingAttachAllOrdinaryDemandMonitors(KHE_SOLN soln)
{
  KHE_MEET meet;  int i;
  MArrayForEach(soln->meets, &meet, &i)
    KheMeetMatchingAttachAllOrdinaryDemandMonitors(meet);
}


/*****************************************************************************/
/*                                                                           */
/*  void KheSolnMatchingDetachAllOrdinaryDemandMonitors(KHE_SOLN soln)       */
/*                                                                           */
/*  Make sure all the ordinary demand monitors of soln are detached.         */
/*                                                                           */
/*****************************************************************************/

void KheSolnMatchingDetachAllOrdinaryDemandMonitors(KHE_SOLN soln)
{
  KHE_MEET meet;  int i;
  MArrayForEach(soln->meets, &meet, &i)
    KheMeetMatchingDetachAllOrdinaryDemandMonitors(meet);
}


/*****************************************************************************/
/*                                                                           */
/*  Submodule "matchings - workload demand nodes"                            */
/*                                                                           */
/*****************************************************************************/

/*****************************************************************************/
/*                                                                           */
/*  void KheSolnMatchingAddAllWorkloadRequirements(KHE_SOLN soln)            */
/*                                                                           */
/*  Add all workload requirements for soln.                                  */
/*                                                                           */
/*****************************************************************************/

/* see khe_workload.c for the implementation of this function */


/*****************************************************************************/
/*                                                                           */
/*  int KheSolnMatchingWorkloadRequirementCount(KHE_SOLN soln,               */
/*    KHE_RESOURCE r)                                                        */
/*                                                                           */
/*  Return the number of workload requirements associated with r in soln.    */
/*                                                                           */
/*****************************************************************************/

int KheSolnMatchingWorkloadRequirementCount(KHE_SOLN soln, KHE_RESOURCE r)
{
  KHE_RESOURCE_IN_SOLN rs;
  rs = KheSolnResourceInSoln(soln, KheResourceIndexInInstance(r));
  return KheResourceInSolnWorkloadRequirementCount(rs);
}


/*****************************************************************************/
/*                                                                           */
/*  void KheSolnMatchingWorkloadRequirement(KHE_SOLN soln,                   */
/*    KHE_RESOURCE r, int i, int *num, KHE_TIME_GROUP *tg)                   */
/*                                                                           */
/*  Return the i'th workload requirement associated with r in soln.          */
/*                                                                           */
/*****************************************************************************/

void KheSolnMatchingWorkloadRequirement(KHE_SOLN soln,
  KHE_RESOURCE r, int i, int *num, KHE_TIME_GROUP *tg)
{
  KHE_RESOURCE_IN_SOLN rs;
  rs = KheSolnResourceInSoln(soln, KheResourceIndexInInstance(r));
  KheResourceInSolnWorkloadRequirement(rs, i, num, tg);
}


/*****************************************************************************/
/*                                                                           */
/*  void KheSolnMatchingBeginWorkloadRequirements(KHE_SOLN soln,             */
/*    KHE_RESOURCE r)                                                        */
/*                                                                           */
/*  Begin a new set of workload requirements for r in soln.                  */
/*                                                                           */
/*****************************************************************************/

void KheSolnMatchingBeginWorkloadRequirements(KHE_SOLN soln, KHE_RESOURCE r)
{
  KHE_RESOURCE_IN_SOLN rs;
  rs = KheSolnResourceInSoln(soln, KheResourceIndexInInstance(r));
  KheResourceInSolnBeginWorkloadRequirements(rs);
}


/*****************************************************************************/
/*                                                                           */
/*  void KheSolnMatchingAddWorkloadRequirement(KHE_SOLN soln,                */
/*    KHE_RESOURCE r, int num, KHE_TIME_GROUP tg)                            */
/*                                                                           */
/*  Add one workload requirement for r in soln.                              */
/*                                                                           */
/*****************************************************************************/

void KheSolnMatchingAddWorkloadRequirement(KHE_SOLN soln,
  KHE_RESOURCE r, int num, KHE_TIME_GROUP tg)
{
  KHE_RESOURCE_IN_SOLN rs;
  rs = KheSolnResourceInSoln(soln, KheResourceIndexInInstance(r));
  KheResourceInSolnAddWorkloadRequirement(rs, num, tg);
}


/*****************************************************************************/
/*                                                                           */
/*  void KheSolnMatchingEndWorkloadRequirements(KHE_SOLN soln,               */
/*    KHE_RESOURCE r)                                                        */
/*                                                                           */
/*  End a new set of workload requirements for r in soln.                    */
/*                                                                           */
/*****************************************************************************/

void KheSolnMatchingEndWorkloadRequirements(KHE_SOLN soln, KHE_RESOURCE r)
{
  KHE_RESOURCE_IN_SOLN rs;
  rs = KheSolnResourceInSoln(soln, KheResourceIndexInInstance(r));
  KheResourceInSolnEndWorkloadRequirements(rs);
}


/*****************************************************************************/
/*                                                                           */
/*  Submodule "matchings - diagnosing failure to match"                      */
/*                                                                           */
/*****************************************************************************/

/*****************************************************************************/
/*                                                                           */
/*  int KheSolnMatchingDefectCount(KHE_SOLN soln)                            */
/*                                                                           */
/*  Return the number of unmatched demand nodes of soln.                     */
/*                                                                           */
/*****************************************************************************/

int KheSolnMatchingDefectCount(KHE_SOLN soln)
{
  return KheMatchingUnmatchedDemandNodeCount(soln->matching);
}


/*****************************************************************************/
/*                                                                           */
/*  KHE_MONITOR KheSolnMatchingDefect(KHE_SOLN soln, int i)                  */
/*                                                                           */
/*  Return the i'th unmatched demand node of soln.                           */
/*                                                                           */
/*****************************************************************************/

KHE_MONITOR KheSolnMatchingDefect(KHE_SOLN soln, int i)
{
  return (KHE_MONITOR) KheMatchingUnmatchedDemandNode(soln->matching, i);
}


/*****************************************************************************/
/*                                                                           */
/*  int KheSolnMatchingHallSetCount(KHE_SOLN soln)                           */
/*                                                                           */
/*  Return the number of Hall sets.                                          */
/*                                                                           */
/*****************************************************************************/

int KheSolnMatchingHallSetCount(KHE_SOLN soln)
{
  return KheMatchingHallSetCount(soln->matching);
}


/*****************************************************************************/
/*                                                                           */
/*  int KheSolnMatchingHallSetSupplyNodeCount(KHE_SOLN soln, int i)          */
/*                                                                           */
/*  Return the number of supply nodes in the i'th Hall set.                  */
/*                                                                           */
/*****************************************************************************/

int KheSolnMatchingHallSetSupplyNodeCount(KHE_SOLN soln, int i)
{
  KHE_MATCHING_HALL_SET hs;
  hs = KheMatchingHallSet(soln->matching, i);
  return KheMatchingHallSetSupplyNodeCount(hs);
}


/*****************************************************************************/
/*                                                                           */
/*  int KheSolnMatchingHallSetDemandNodeCount(KHE_SOLN soln, int i)          */
/*                                                                           */
/*  Return the number of demand nodes in the i'th Hall set.                  */
/*                                                                           */
/*****************************************************************************/

int KheSolnMatchingHallSetDemandNodeCount(KHE_SOLN soln, int i)
{
  KHE_MATCHING_HALL_SET hs;
  hs = KheMatchingHallSet(soln->matching, i);
  return KheMatchingHallSetDemandNodeCount(hs);
}


/*****************************************************************************/
/*                                                                           */
/*  bool KheSolnMatchingHallSetSupplyNodeIsOrdinary(KHE_SOLN soln,           */
/*    int i, int j, KHE_MEET *meet, int *meet_offset, KHE_RESOURCE *r)       */
/*                                                                           */
/*  Extract information about the j'th supply node of the i'th Hall set.     */
/*                                                                           */
/*****************************************************************************/

bool KheSolnMatchingHallSetSupplyNodeIsOrdinary(KHE_SOLN soln,
  int i, int j, KHE_MEET *meet, int *meet_offset, KHE_RESOURCE *r)
{
  KHE_MATCHING_HALL_SET hs;  KHE_MATCHING_SUPPLY_NODE sn;
  hs = KheMatchingHallSet(soln->matching, i);
  sn = KheMatchingHallSetSupplyNode(hs, j);
  return KheSolnSupplyNodeIsOrdinary(sn, meet, meet_offset, r);
}


/*****************************************************************************/
/*                                                                           */
/*  void KheSolnMatchingHallSetsDebug(KHE_SOLN soln, int verbosity,          */
/*    int indent, FILE *fp)                                                  */
/*                                                                           */
/*  Debug print of the Hall sets of soln onto fp with the given verbosity    */
/*  and indent.                                                              */
/*                                                                           */
/*****************************************************************************/

void KheSolnMatchingHallSetsDebug(KHE_SOLN soln, int verbosity,
  int indent, FILE *fp)
{
  KHE_MATCHING_HALL_SET hs;  int i;
  if( verbosity >= 1 && indent >= 0 )
  {
    fprintf(fp, "%*s[ %d Hall Sets:\n", indent, "",
      KheSolnMatchingHallSetCount(soln));
    for( i = 0;  i < KheSolnMatchingHallSetCount(soln);  i++ )
    {
      hs = KheMatchingHallSet(soln->matching, i);
      KheMatchingHallSetDebug(hs, &KheSupplyNodeDebug, &KheDemandNodeDebug,
	verbosity, indent + 2, fp);
    }
    fprintf(fp, "%*s]\n", indent, "");
  }
}


/*****************************************************************************/
/*                                                                           */
/*  Submodule "debug"                                                        */
/*                                                                           */
/*****************************************************************************/

/*****************************************************************************/
/*                                                                           */
/*  KHE_COST KheSolnCostByType(KHE_SOLN soln, KHE_MONITOR_TAG tag,           */
/*    int *defect_count)                                                     */
/*                                                                           */
/*  Return the cost and number of defects of monitors below soln that        */
/*  have the given tag.                                                      */
/*                                                                           */
/*****************************************************************************/

KHE_COST KheSolnCostByType(KHE_SOLN soln, KHE_MONITOR_TAG tag,
  int *defect_count)
{
  return KheGroupMonitorCostByType((KHE_GROUP_MONITOR) soln, tag,
    defect_count);
}


/*****************************************************************************/
/*                                                                           */
/*  void KheSolnCostByTypeDebug(KHE_SOLN soln, int verbosity,                */
/*    int indent, FILE *fp)                                                  */
/*                                                                           */
/*  Debug print of the cost of soln onto fp with the given indent.           */
/*                                                                           */
/*****************************************************************************/

void KheSolnCostByTypeDebug(KHE_SOLN soln, int verbosity, int indent, FILE *fp)
{
  KheGroupMonitorCostByTypeDebug((KHE_GROUP_MONITOR) soln,
    verbosity, indent, fp);
}


/*****************************************************************************/
/*                                                                           */
/*  void KheSolnDebug(KHE_SOLN soln, int verbosity, int indent, FILE *fp)    */
/*                                                                           */
/*  Debug print of soln onto fp with the given verbosity and indent.         */
/*                                                                           */
/*****************************************************************************/

void KheSolnDebug(KHE_SOLN soln, int verbosity, int indent, FILE *fp)
{
  int i;  KHE_EVENT_IN_SOLN es;  KHE_MEET meet;  KHE_RESOURCE_IN_SOLN rs;
  if( verbosity >= 1 )
  {
    if( indent >= 0 )
      fprintf(fp, "%*s", indent, "");
    fprintf(fp, "[ Soln (instance \"%s\", diversifier %d, cost %.4f)",
      KheInstanceId(soln->instance)!=NULL ? KheInstanceId(soln->instance):"-",
      soln->diversifier, KheCostShow(KheSolnCost(soln)));
    if( indent >= 0 )
    {
      fprintf(fp, "\n");
      if( verbosity >= 2 )
      {
	if( verbosity >= 4 )
	{
	  MArrayForEach(soln->meets, &meet, &i)
	    KheMeetDebug(meet, verbosity, indent + 2, fp);
	  MArrayForEach(soln->events_in_soln, &es, &i)
	    KheEventInSolnDebug(es, verbosity, indent + 2, fp);
	  MArrayForEach(soln->resources_in_soln, &rs, &i)
	    KheResourceInSolnDebug(rs, verbosity, indent + 2, fp);
	}
	if( verbosity >= 3 )
	{
	  fprintf(fp, "%*s  defects:\n", indent, "");
	  KheGroupMonitorDefectDebug((KHE_GROUP_MONITOR) soln, 2, indent+2, fp);
	}
	KheSolnCostByTypeDebug(soln, 2, indent + 2, fp);
      }
      fprintf(fp, "%*s]\n", indent, "");
    }
    else
      fprintf(fp, " ]");
  }
}
