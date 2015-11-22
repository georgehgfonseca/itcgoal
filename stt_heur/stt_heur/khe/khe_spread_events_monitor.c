
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
/*  FILE:         khe_spread_events_monitor.c                                */
/*  DESCRIPTION:  A spread events monitor                                    */
/*                                                                           */
/*****************************************************************************/
#include "khe_interns.h"
#define DEBUG1 0


/*****************************************************************************/
/*                                                                           */
/*  KHE_SPREAD_TIME_GROUP                                                    */
/*                                                                           */
/*  One time group being monitored.                                          */
/*                                                                           */
/*****************************************************************************/

typedef struct khe_spread_time_group_rec *KHE_SPREAD_TIME_GROUP;

struct khe_spread_time_group_rec {
  KHE_TIME_GROUP	time_group;			/* time group        */
  int			minimum;			/* min incidences    */
  int			maximum;			/* max incidences    */
  int			incidences;			/* curr. incidences  */
  KHE_SPREAD_TIME_GROUP	copy;				/* used when copying */
};

typedef MARRAY(KHE_SPREAD_TIME_GROUP) ARRAY_KHE_SPREAD_TIME_GROUP;


/*****************************************************************************/
/*                                                                           */
/*  KHE_SPREAD_TIME - one time of an event set's timetable                   */
/*                                                                           */
/*****************************************************************************/

typedef struct khe_spread_time_rec *KHE_SPREAD_TIME;

struct khe_spread_time_rec {
  KHE_TIME			time;			/* monitored time    */
  ARRAY_KHE_SPREAD_TIME_GROUP	spread_time_groups; 	/* time groups       */
  KHE_SPREAD_TIME		copy;			/* used when copying */
};

typedef MARRAY(KHE_SPREAD_TIME) ARRAY_KHE_SPREAD_TIME;


/*****************************************************************************/
/*                                                                           */
/*  KHE_SPREAD_EVENTS_MONITOR - a spread events monitor                      */
/*                                                                           */
/*****************************************************************************/

struct khe_spread_events_monitor_rec {

  /* inherited from KHE_MONITOR */
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

  /* specific to KHE_SPREAD_EVENTS_MONITOR */
  KHE_SPREAD_EVENTS_CONSTRAINT	constraint;		/* constraint        */
  KHE_EVENT_GROUP		event_group;		/* event group       */
  ARRAY_KHE_SPREAD_TIME		spread_times;		/* one for each time */
  ARRAY_KHE_SPREAD_TIME_GROUP	spread_time_groups;	/* time groups       */
  bool				separate;		/* separate needed   */
  int				total_devs;		/* if not separate   */
  KHE_DEV_MONITOR		dev_monitor;		/* if separate       */
  KHE_SPREAD_EVENTS_MONITOR	copy;			/* used when copying */
};


/*****************************************************************************/
/*                                                                           */
/*  Submodule "spread time groups" (private)                                 */
/*                                                                           */
/*****************************************************************************/

/*****************************************************************************/
/*                                                                           */
/*  KHE_SPREAD_TIME_GROUP KheSpreadTimeGroupMake(KHE_LIMITED_TIME_GROUP ltg) */
/*                                                                           */
/*  Make a new spread time group based on ltg.                               */
/*                                                                           */
/*****************************************************************************/

static KHE_SPREAD_TIME_GROUP KheSpreadTimeGroupMake(KHE_LIMITED_TIME_GROUP ltg)
{
  KHE_SPREAD_TIME_GROUP res;
  MMake(res);
  res->time_group = KheLimitedTimeGroupTimeGroup(ltg);
  res->minimum = KheLimitedTimeGroupMinimum(ltg);
  res->maximum = KheLimitedTimeGroupMaximum(ltg);
  res->incidences = 0;
  res->copy = NULL;
  return res;
}


/*****************************************************************************/
/*                                                                           */
/*  KHE_SPREAD_TIME_GROUP KheSpreadTimeGroupCopyPhase1(                      */
/*    KHE_SPREAD_TIME_GROUP stg)                                             */
/*                                                                           */
/*  Carry out Phase 1 of the copying of stg.                                 */
/*                                                                           */
/*****************************************************************************/

static KHE_SPREAD_TIME_GROUP KheSpreadTimeGroupCopyPhase1(
  KHE_SPREAD_TIME_GROUP stg)
{
  KHE_SPREAD_TIME_GROUP copy;
  if( stg->copy == NULL )
  {
    MMake(copy);
    stg->copy = copy;
    copy->time_group = stg->time_group;
    copy->minimum = stg->minimum;
    copy->maximum = stg->maximum;
    copy->incidences = stg->incidences;
    copy->copy = NULL;
  }
  return stg->copy;
}


/*****************************************************************************/
/*                                                                           */
/*  void KheSpreadTimeGroupCopyPhase2(KHE_SPREAD_TIME_GROUP stg)             */
/*                                                                           */
/*  Carry out Phase 2 of the copying of stg.                                 */
/*                                                                           */
/*****************************************************************************/

static void KheSpreadTimeGroupCopyPhase2(KHE_SPREAD_TIME_GROUP stg)
{
  if( stg->copy != NULL )
  {
    stg->copy = NULL;
  }
}


/*****************************************************************************/
/*                                                                           */
/*  Submodule "spread times" (private)                                       */
/*                                                                           */
/*****************************************************************************/

/*****************************************************************************/
/*                                                                           */
/*  KHE_SPREAD_TIME KheSpreadTimeMake(KHE_TIME time)                         */
/*                                                                           */
/*  Make a new spread time object with these attributes.                     */
/*                                                                           */
/*****************************************************************************/

static KHE_SPREAD_TIME KheSpreadTimeMake(KHE_TIME time)
{
  KHE_SPREAD_TIME res;
  MMake(res);
  res->time = time;
  MArrayInit(res->spread_time_groups);
  res->copy = NULL;
  return res;
}


/*****************************************************************************/
/*                                                                           */
/*  KHE_SPREAD_TIME KheSpreadTimeCopyPhase1(KHE_SPREAD_TIME st)              */
/*                                                                           */
/*  Carry out Phase 1 of the copying of st.                                  */
/*                                                                           */
/*****************************************************************************/

static KHE_SPREAD_TIME KheSpreadTimeCopyPhase1(KHE_SPREAD_TIME st)
{
  KHE_SPREAD_TIME copy;  KHE_SPREAD_TIME_GROUP stg;  int i;
  if( st->copy == NULL )
  {
    MMake(copy);
    st->copy = copy;
    copy->time = st->time;
    MArrayInit(copy->spread_time_groups);
    MArrayForEach(st->spread_time_groups, &stg, &i)
      MArrayAddLast(copy->spread_time_groups,
	KheSpreadTimeGroupCopyPhase1(stg));
    copy->copy = NULL;
  }
  return st->copy;
}


/*****************************************************************************/
/*                                                                           */
/*  void KheSpreadTimeCopyPhase2(KHE_SPREAD_TIME st)                         */
/*                                                                           */
/*  Carry out Phase 2 of the copying of st.                                  */
/*                                                                           */
/*****************************************************************************/

static void KheSpreadTimeCopyPhase2(KHE_SPREAD_TIME st)
{
  KHE_SPREAD_TIME_GROUP stg;  int i;
  if( st->copy != NULL )
  {
    st->copy = NULL;
    MArrayForEach(st->spread_time_groups, &stg, &i)
      KheSpreadTimeGroupCopyPhase2(stg);
  }
}


/*****************************************************************************/
/*                                                                           */
/*  Submodule "construction and query"                                       */
/*                                                                           */
/*****************************************************************************/

/*****************************************************************************/
/*                                                                           */
/*  KHE_SPREAD_EVENTS_MONITOR KheSpreadEventsMonitorMake(KHE_SOLN soln,      */
/*    KHE_SPREAD_EVENTS_CONSTRAINT c, KHE_EVENT_GROUP eg)                    */
/*                                                                           */
/*  Make a spread events monitor with these attributes.                      */
/*                                                                           */
/*****************************************************************************/

KHE_SPREAD_EVENTS_MONITOR KheSpreadEventsMonitorMake(KHE_SOLN soln,
  KHE_SPREAD_EVENTS_CONSTRAINT c, KHE_EVENT_GROUP eg)
{
  KHE_SPREAD_EVENTS_MONITOR res;  KHE_INSTANCE ins;  KHE_TIME t;
  KHE_SPREAD_TIME st;  KHE_SPREAD_TIME_GROUP stg;  KHE_TIME_SPREAD ts;
  KHE_LIMITED_TIME_GROUP ltg;  int i, j;
  KHE_EVENT e;  KHE_EVENT_IN_SOLN es;

  /* make the monitor */
  MMake(res);
  KheMonitorInitCommonFields((KHE_MONITOR) res, soln,
    KHE_SPREAD_EVENTS_MONITOR_TAG);
  res->constraint = c;
  res->event_group = eg;
  res->separate =
    (KheConstraintCostFunction((KHE_CONSTRAINT) c) != KHE_SUM_COST_FUNCTION);
  res->total_devs = 0;
  KheDevMonitorInit(&res->dev_monitor);
  res->copy = NULL;

  /* add one spread time for each time of the instance */
  MArrayInit(res->spread_times);
  ins = KheSolnInstance(soln);
  for( i = 0;  i < KheInstanceTimeCount(ins);  i++ )
    MArrayAddLast(res->spread_times, KheSpreadTimeMake(KheInstanceTime(ins,i)));

  /* add one spread time group for each limited time group of the constraint */
  /* linked to the spread times corresponding to the times of the time group */
  MArrayInit(res->spread_time_groups);
  ts = KheSpreadEventsConstraintTimeSpread(c);
  for( i = 0;  i < KheTimeSpreadLimitedTimeGroupCount(ts);  i++ )
  {
    ltg = KheTimeSpreadLimitedTimeGroup(ts, i);
    stg = KheSpreadTimeGroupMake(ltg);
    MArrayAddLast(res->spread_time_groups, stg);
    for( j = 0;  j < KheTimeGroupTimeCount(stg->time_group);  j++ )
    {
      t = KheTimeGroupTime(stg->time_group, j);
      st = MArrayGet(res->spread_times, KheTimeIndex(t));
      MArrayAddLast(st->spread_time_groups, stg);
    }
  }

  /* add (but don't attach) to the monitored event in soln objects */
  for( i = 0;  i < KheEventGroupEventCount(res->event_group);  i++ )
  {
    e = KheEventGroupEvent(res->event_group, i);
    es = KheSolnEventInSoln(soln, KheEventIndex(e));
    KheEventInSolnAddMonitor(es, (KHE_MONITOR) res);
  }
  /* KheGroupMonitorAddMonitor((KHE_GROUP_MONITOR) soln, (KHE_MONITOR) res); */
  return res;
}


/*****************************************************************************/
/*                                                                           */
/*  KHE_SPREAD_EVENTS_MONITOR KheSpreadEventsMonitorCopyPhase1(              */
/*    KHE_SPREAD_EVENTS_MONITOR m)                                           */
/*                                                                           */
/*  Carry out Phase 1 of the copying of m.                                   */
/*                                                                           */
/*****************************************************************************/

KHE_SPREAD_EVENTS_MONITOR KheSpreadEventsMonitorCopyPhase1(
  KHE_SPREAD_EVENTS_MONITOR m)
{
  KHE_SPREAD_EVENTS_MONITOR copy;  int i;
  KHE_SPREAD_TIME st;  KHE_SPREAD_TIME_GROUP stg;
  if( m->copy == NULL )
  {
    MMake(copy);
    m->copy = copy;
    KheMonitorCopyCommonFields((KHE_MONITOR) copy, (KHE_MONITOR) m);
    copy->constraint = m->constraint;
    copy->event_group = m->event_group;
    MArrayInit(copy->spread_times);
    MArrayForEach(m->spread_times, &st, &i)
      MArrayAddLast(copy->spread_times, KheSpreadTimeCopyPhase1(st));
    MArrayInit(copy->spread_time_groups);
    MArrayForEach(m->spread_time_groups, &stg, &i)
      MArrayAddLast(copy->spread_time_groups,
	KheSpreadTimeGroupCopyPhase1(stg));
    copy->separate = m->separate;
    copy->total_devs = m->total_devs;
    KheDevMonitorCopy(&copy->dev_monitor, &m->dev_monitor);
    copy->copy = NULL;
  }
  return m->copy;
}


/*****************************************************************************/
/*                                                                           */
/*  void KheSpreadEventsMonitorCopyPhase2(KHE_SPREAD_EVENTS_MONITOR m)       */
/*                                                                           */
/*  Carry out Phase 2 of the copying of m.                                   */
/*                                                                           */
/*****************************************************************************/

void KheSpreadEventsMonitorCopyPhase2(KHE_SPREAD_EVENTS_MONITOR m)
{
  KHE_SPREAD_TIME st;  KHE_SPREAD_TIME_GROUP stg;  int i;
  if( m->copy != NULL )
  {
    MArrayForEach(m->spread_times, &st, &i)
      KheSpreadTimeCopyPhase2(st);
    MArrayForEach(m->spread_time_groups, &stg, &i)
      KheSpreadTimeGroupCopyPhase2(stg);
    m->copy = NULL;
  }
}


/*****************************************************************************/
/*                                                                           */
/*  void KheSpreadEventsMonitorDelete(KHE_SPREAD_EVENTS_MONITOR m)           */
/*                                                                           */
/*  Delete m.                                                                */
/*                                                                           */
/*****************************************************************************/

void KheSpreadEventsMonitorDelete(KHE_SPREAD_EVENTS_MONITOR m)
{
  KHE_EVENT e;  KHE_EVENT_IN_SOLN es;
  KHE_SPREAD_TIME st;  KHE_SPREAD_TIME_GROUP stg;  int i;
  if( m->attached )
    KheSpreadEventsMonitorDetachFromSoln(m);
  if( m->parent_monitor != NULL )
    KheGroupMonitorDeleteChildMonitor(m->parent_monitor, (KHE_MONITOR) m);
  for( i = 0;  i < KheEventGroupEventCount(m->event_group);  i++ )
  {
    e = KheEventGroupEvent(m->event_group, i);
    es = KheSolnEventInSoln(m->soln, KheEventIndex(e));
    KheEventInSolnDeleteMonitor(es, (KHE_MONITOR) m);
  }
  KheSolnDeleteMonitor(m->soln, (KHE_MONITOR) m);
  MArrayForEach(m->spread_times, &st, &i)
  {
    MArrayFree(st->spread_time_groups);
    MFree(st);
  }
  MArrayFree(m->spread_times);
  MArrayForEach(m->spread_time_groups, &stg, &i)
    MFree(stg);
  MArrayFree(m->spread_time_groups);
  KheDevMonitorFree(&m->dev_monitor);
  MFree(m);
}


/*****************************************************************************/
/*                                                                           */
/*  KHE_SPREAD_EVENTS_CONSTRAINT KheSpreadEventsMonitorConstraint(           */
/*    KHE_SPREAD_EVENTS_MONITOR m)                                           */
/*                                                                           */
/*  Return the constraint being monitored by m.                              */
/*                                                                           */
/*****************************************************************************/

KHE_SPREAD_EVENTS_CONSTRAINT KheSpreadEventsMonitorConstraint(
  KHE_SPREAD_EVENTS_MONITOR m)
{
  return m->constraint;
}


/*****************************************************************************/
/*                                                                           */
/*  KHE_EVENT_GROUP KheSpreadEventsMonitorEventGroup(                        */
/*    KHE_SPREAD_EVENTS_MONITOR m)                                           */
/*                                                                           */
/*  Return the event group being monitored by m.                             */
/*                                                                           */
/*****************************************************************************/

KHE_EVENT_GROUP KheSpreadEventsMonitorEventGroup(KHE_SPREAD_EVENTS_MONITOR m)
{
  return m->event_group;
}


/*****************************************************************************/
/*                                                                           */
/*  int KheSpreadEventsMonitorTimeGroupCount(KHE_SPREAD_EVENTS_MONITOR m)    */
/*                                                                           */
/*  Return the number of time groups being monitored.                        */
/*                                                                           */
/*****************************************************************************/

int KheSpreadEventsMonitorTimeGroupCount(KHE_SPREAD_EVENTS_MONITOR m)
{
  return MArraySize(m->spread_time_groups);
}


/*****************************************************************************/
/*                                                                           */
/*  void KheSpreadEventsMonitorTimeGroup(KHE_SPREAD_EVENTS_MONITOR m, int i, */
/*   KHE_TIME_GROUP *time_group, int *minimum, int *maximum, int *incidences)*/
/*                                                                           */
/*  Return the i'th time group being monitored, and its limits and the       */
/*  current number of incidences.                                            */
/*                                                                           */
/*****************************************************************************/

void KheSpreadEventsMonitorTimeGroup(KHE_SPREAD_EVENTS_MONITOR m, int i,
  KHE_TIME_GROUP *time_group, int *minimum, int *maximum, int *incidences)
{
  KHE_SPREAD_TIME_GROUP stg;
  stg = MArrayGet(m->spread_time_groups, i);
  *time_group = stg->time_group;
  *minimum = stg->minimum;
  *maximum = stg->maximum;
  *incidences = stg->incidences;
}


/*****************************************************************************/
/*                                                                           */
/*  Submodule "attach and detach"                                            */
/*                                                                           */
/*****************************************************************************/

/*****************************************************************************/
/*                                                                           */
/*  void KheSpreadEventsMonitorAttachToSoln(KHE_SPREAD_EVENTS_MONITOR m)     */
/*                                                                           */
/*  Attach m.  It is known to be currently detached with cost 0.             */
/*                                                                           */
/*****************************************************************************/

void KheSpreadEventsMonitorAttachToSoln(KHE_SPREAD_EVENTS_MONITOR m)
{
  KHE_EVENT e;  KHE_EVENT_IN_SOLN es;  KHE_SPREAD_TIME_GROUP stg;  int i;

  /* work out initial cost and inform soln */
  m->attached = true;
  if( m->separate )
  {
    KheDevMonitorInit(&m->dev_monitor);
    MArrayForEach(m->spread_time_groups, &stg, &i)
    {
      stg->incidences = 0;
      if( stg->incidences < stg->minimum )
	KheDevMonitorAttach(&m->dev_monitor, stg->minimum - stg->incidences);
    }
    if( KheDevMonitorHasChanged(&m->dev_monitor) )
    {
      KheMonitorChangeCost((KHE_MONITOR) m,
	KheConstraintCostMulti((KHE_CONSTRAINT) m->constraint,
	  KheDevMonitorDevs(&m->dev_monitor)));
      KheDevMonitorFlush(&m->dev_monitor);
    }
  }
  else
  {
    m->total_devs = 0;
    MArrayForEach(m->spread_time_groups, &stg, &i)
    {
      stg->incidences = 0;
      if( stg->incidences < stg->minimum )
	m->total_devs += (stg->minimum - stg->incidences);
    }
    if( m->total_devs != 0 )
    {
      KheMonitorChangeCost((KHE_MONITOR) m,
        KheConstraintCost((KHE_CONSTRAINT) m->constraint, m->total_devs));
    }
  }

  /* attach */
  for( i = 0;  i < KheEventGroupEventCount(m->event_group);  i++ )
  {
    e = KheEventGroupEvent(m->event_group, i);
    es = KheSolnEventInSoln(m->soln, KheEventIndex(e));
    KheEventInSolnAttachMonitor(es, (KHE_MONITOR) m);
  }
}


/*****************************************************************************/
/*                                                                           */
/*  void KheSpreadEventsMonitorDetachFromSoln(KHE_SPREAD_EVENTS_MONITOR m)   */
/*                                                                           */
/*  Detach m.  It is known to be currently attached.                         */
/*                                                                           */
/*****************************************************************************/

void KheSpreadEventsMonitorDetachFromSoln(KHE_SPREAD_EVENTS_MONITOR m)
{
  int i;  KHE_EVENT e;  KHE_EVENT_IN_SOLN es;
  for( i = 0;  i < KheEventGroupEventCount(m->event_group);  i++ )
  {
    e = KheEventGroupEvent(m->event_group, i);
    es = KheSolnEventInSoln(m->soln, KheEventIndex(e));
    KheEventInSolnDetachMonitor(es, (KHE_MONITOR) m);
  }
  KheMonitorChangeCost((KHE_MONITOR) m, 0);
  m->attached = false;
}


/*****************************************************************************/
/*                                                                           */
/*  void KheSpreadEventsMonitorAttachCheck(KHE_SPREAD_EVENTS_MONITOR m)      */
/*                                                                           */
/*  Check the attachment of m.                                               */
/*                                                                           */
/*****************************************************************************/

void KheSpreadEventsMonitorAttachCheck(KHE_SPREAD_EVENTS_MONITOR m)
{
  if( !KheMonitorAttachedToSoln((KHE_MONITOR) m) )
    KheMonitorAttachToSoln((KHE_MONITOR) m);
}


/*****************************************************************************/
/*                                                                           */
/*  Submodule "monitoring calls"                                             */
/*                                                                           */
/*****************************************************************************/

/*****************************************************************************/
/*                                                                           */
/*  void KheSpreadEventsMonitorAddMeet(KHE_SPREAD_EVENTS_MONITOR m,          */
/*    KHE_MEET meet)                                                         */
/*                                                                           */
/*  Inform m that meet is being added.                                       */
/*                                                                           */
/*****************************************************************************/

void KheSpreadEventsMonitorAddMeet(KHE_SPREAD_EVENTS_MONITOR m, KHE_MEET meet)
{
  if( KheMeetAssignedTimeIndex(meet) != -1 )
    KheSpreadEventsMonitorAssignTime(m, meet, KheMeetAssignedTimeIndex(meet));
}


/*****************************************************************************/
/*                                                                           */
/*  void KheSpreadEventsMonitorDeleteMeet(KHE_SPREAD_EVENTS_MONITOR m,       */
/*    KHE_MEET meet)                                                         */
/*                                                                           */
/*  Inform m that meet is being deleted.                                     */
/*                                                                           */
/*****************************************************************************/

void KheSpreadEventsMonitorDeleteMeet(KHE_SPREAD_EVENTS_MONITOR m,
  KHE_MEET meet)
{
  if( KheMeetAssignedTimeIndex(meet) != -1 )
    KheSpreadEventsMonitorUnAssignTime(m, meet,KheMeetAssignedTimeIndex(meet));
}


/*****************************************************************************/
/*                                                                           */
/*  void KheSpreadEventsMonitorSplitMeet(KHE_SPREAD_EVENTS_MONITOR m,        */
/*    KHE_MEET meet1, KHE_MEET meet2)                                        */
/*                                                                           */
/*  Inform m that a split into meet1 and meet2 is occurring.                 */
/*                                                                           */
/*****************************************************************************/

void KheSpreadEventsMonitorSplitMeet(KHE_SPREAD_EVENTS_MONITOR m,
  KHE_MEET meet1, KHE_MEET meet2)
{
  KheSpreadEventsMonitorAddMeet(m, meet2);
}


/*****************************************************************************/
/*                                                                           */
/*  void KheSpreadEventsMonitorMergeMeet(KHE_SPREAD_EVENTS_MONITOR m,        */
/*    KHE_MEET meet1, KHE_MEET meet2)                                        */
/*                                                                           */
/*  Inform m that a merge of meet1 and meet2 is occurring.                   */
/*                                                                           */
/*****************************************************************************/

void KheSpreadEventsMonitorMergeMeet(KHE_SPREAD_EVENTS_MONITOR m,
  KHE_MEET meet1, KHE_MEET meet2)
{
  KheSpreadEventsMonitorDeleteMeet(m, meet2);
}


/*****************************************************************************/
/*                                                                           */
/*  void KheSpreadEventsMonitorAssignTime(KHE_SPREAD_EVENTS_MONITOR m,       */
/*    KHE_MEET meet, int assigned_time_index)                                */
/*                                                                           */
/*  Inform m that a time assignment of meet is occurring.                    */
/*                                                                           */
/*****************************************************************************/

void KheSpreadEventsMonitorAssignTime(KHE_SPREAD_EVENTS_MONITOR m,
  KHE_MEET meet, int assigned_time_index)
{
  KHE_SPREAD_TIME st;  KHE_SPREAD_TIME_GROUP stg;
  int i, old_incidences, delta_devs;
  if( DEBUG1 )
    fprintf(stderr, "[ KheSpreadEventsMonitorAssignTime(m, %d)\n",
      assigned_time_index);
  st = MArrayGet(m->spread_times, assigned_time_index);
  if( m->separate )
  {
    MArrayForEach(st->spread_time_groups, &stg, &i)
    {
      old_incidences = stg->incidences++;
      if( stg->incidences <= stg->minimum )
      {
	if( stg->incidences == stg->minimum )
	  KheDevMonitorDetach(&m->dev_monitor, 1);
	else
	  KheDevMonitorUpdate(&m->dev_monitor,
	    stg->minimum - old_incidences, stg->minimum - stg->incidences);
      }
      else if( stg->incidences > stg->maximum )
      {
	if( old_incidences == stg->maximum )
	  KheDevMonitorAttach(&m->dev_monitor, 1);
	else
	  KheDevMonitorUpdate(&m->dev_monitor,
	    old_incidences - stg->maximum, stg->incidences - stg->maximum);
      }
    }
    if( KheDevMonitorHasChanged(&m->dev_monitor) )
    {
      KheMonitorChangeCost((KHE_MONITOR) m,
        KheConstraintCostMulti((KHE_CONSTRAINT) m->constraint,
	  KheDevMonitorDevs(&m->dev_monitor)));
      KheDevMonitorFlush(&m->dev_monitor);
    }
  }
  else
  {
    delta_devs = 0;
    MArrayForEach(st->spread_time_groups, &stg, &i)
    {
      stg->incidences++;
      if( stg->incidences <= stg->minimum )
	delta_devs--;
      else if( stg->incidences > stg->maximum )
	delta_devs++;
    }
    if( delta_devs != 0 )
    {
      m->total_devs += delta_devs;
      KheMonitorChangeCost((KHE_MONITOR) m,
        KheConstraintCost((KHE_CONSTRAINT) m->constraint, m->total_devs));
    }
  }
  if( DEBUG1 )
    fprintf(stderr, "] KheSpreadEventsMonitorAssignTime returning\n");
}


/*****************************************************************************/
/*                                                                           */
/*  void KheSpreadEventsMonitorUnAssignTime(KHE_SPREAD_EVENTS_MONITOR m,     */
/*    KHE_MEET meet, int assigned_time_index)                                */
/*                                                                           */
/*  Inform m that a time unassignment of meet is occurring.                  */
/*                                                                           */
/*****************************************************************************/

void KheSpreadEventsMonitorUnAssignTime(KHE_SPREAD_EVENTS_MONITOR m,
  KHE_MEET meet, int assigned_time_index)
{
  KHE_SPREAD_TIME st;  KHE_SPREAD_TIME_GROUP stg;
  int i, old_incidences, delta_devs;
  st = MArrayGet(m->spread_times, assigned_time_index);
  if( m->separate )
  {
    MArrayForEach(st->spread_time_groups, &stg, &i)
    {
      old_incidences = stg->incidences--;
      if( old_incidences <= stg->minimum )
      {
	if( old_incidences == stg->minimum )
	  KheDevMonitorAttach(&m->dev_monitor, 1);
	else
	  KheDevMonitorUpdate(&m->dev_monitor,
	    stg->minimum - old_incidences, stg->minimum - stg->incidences);
      }
      else if( stg->incidences >= stg->maximum )
      {
	if( stg->incidences == stg->maximum )
	  KheDevMonitorDetach(&m->dev_monitor, 1);
	else
	  KheDevMonitorUpdate(&m->dev_monitor,
	    old_incidences - stg->maximum, stg->incidences - stg->maximum);
      }
    }
    if( KheDevMonitorHasChanged(&m->dev_monitor) )
    {
      KheMonitorChangeCost((KHE_MONITOR) m,
	KheConstraintCostMulti((KHE_CONSTRAINT) m->constraint,
	  KheDevMonitorDevs(&m->dev_monitor)));
      KheDevMonitorFlush(&m->dev_monitor);
    }
  }
  else
  {
    delta_devs = 0;
    MArrayForEach(st->spread_time_groups, &stg, &i)
    {
      stg->incidences--;
      if( stg->incidences < stg->minimum )
	delta_devs++;
      else if( stg->incidences >= stg->maximum )
	delta_devs--;
    }
    if( delta_devs != 0 )
    {
      m->total_devs += delta_devs;
      KheMonitorChangeCost((KHE_MONITOR) m,
        KheConstraintCost((KHE_CONSTRAINT) m->constraint, m->total_devs));
    }
  }
}


/*****************************************************************************/
/*                                                                           */
/*  Submodule "deviations"                                                   */
/*                                                                           */
/*****************************************************************************/

/*****************************************************************************/
/*                                                                           */
/*  int KheSpreadEventsMonitorDeviationCount(KHE_SPREAD_EVENTS_MONITOR m)    */
/*                                                                           */
/*  Return the number of deviations of m.                                    */
/*                                                                           */
/*****************************************************************************/

int KheSpreadEventsMonitorDeviationCount(KHE_SPREAD_EVENTS_MONITOR m)
{
  return MArraySize(m->spread_time_groups);
}


/*****************************************************************************/
/*                                                                           */
/*  int KheSpreadEventsMonitorDeviation(KHE_SPREAD_EVENTS_MONITOR m, int i)  */
/*                                                                           */
/*  Return the i'th deviation of m.                                          */
/*                                                                           */
/*****************************************************************************/

int KheSpreadEventsMonitorDeviation(KHE_SPREAD_EVENTS_MONITOR m, int i)
{
  KHE_SPREAD_TIME_GROUP stg;
  stg = MArrayGet(m->spread_time_groups, i);
  return stg->incidences < stg->minimum ? stg->minimum - stg->incidences :
    stg->incidences > stg->maximum ? stg->incidences - stg->maximum : 0;
}


/*****************************************************************************/
/*                                                                           */
/*  char *KheSpreadEventsMonitorDeviationDescription(                        */
/*    KHE_SPREAD_EVENTS_MONITOR m, int i)                                    */
/*                                                                           */
/*  Return a description of the i'th deviation of m.                         */
/*                                                                           */
/*****************************************************************************/

char *KheSpreadEventsMonitorDeviationDescription(
  KHE_SPREAD_EVENTS_MONITOR m, int i)
{
  KHE_SPREAD_TIME_GROUP stg;
  stg = MArrayGet(m->spread_time_groups, i);
  return KheTimeGroupName(stg->time_group);
}


/*****************************************************************************/
/*                                                                           */
/*  Submodule "debug"                                                        */
/*                                                                           */
/*****************************************************************************/

/*****************************************************************************/
/*                                                                           */
/*  void KheSpreadEventsMonitorDebug(KHE_SPREAD_EVENTS_MONITOR m,            */
/*    int verbosity, int indent, FILE *fp)                                   */
/*                                                                           */
/*  Debug print of m onto fp with the given verbosity and indent.            */
/*                                                                           */
/*****************************************************************************/

void KheSpreadEventsMonitorDebug(KHE_SPREAD_EVENTS_MONITOR m,
  int verbosity, int indent, FILE *fp)
{
  if( verbosity >= 1 )
  {
    KheMonitorDebugBegin((KHE_MONITOR) m, indent, fp);
    fprintf(fp, " %s", KheEventGroupId(m->event_group) == NULL ? "-" :
      KheEventGroupId(m->event_group));
    /* ***
    fprintf(fp, " %s",
      KheConstraintId((KHE_CONSTRAINT) m->constraint) == NULL ? "-" :
      KheConstraintId((KHE_CONSTRAINT) m->constraint));
    *** */
    KheMonitorDebugEnd((KHE_MONITOR) m, true, indent, fp);
  }
}
