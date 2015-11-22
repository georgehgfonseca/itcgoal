
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
/*  FILE:         khe_link_events_monitor.c                                  */
/*  DESCRIPTION:  An link events monitor                                     */
/*                                                                           */
/*****************************************************************************/
#include <stdarg.h>
#include "khe_interns.h"
#define DEBUG1 0


/*****************************************************************************/
/*                                                                           */
/*  KHE_LINK_EVENTS_MONITOR - a link events monitor                          */
/*                                                                           */
/*****************************************************************************/

struct khe_link_events_monitor_rec {

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

  /* specific to KHE_LINK_EVENTS_MONITOR */
  KHE_LINK_EVENTS_CONSTRAINT	constraint;		/* constraint        */
  KHE_EVENT_GROUP		event_group;		/* event group       */
  int				wanted_multiplicity;	/* want this many    */
  ARRAY_INT			multiplicities;		/* indexed by time   */
  int				deviation;		/* deviation        */
  int				new_deviation;		/* new deviation    */
  KHE_LINK_EVENTS_MONITOR	copy;			/* used when copying */
};


/*****************************************************************************/
/*                                                                           */
/*  Submodule "construction and query"                                       */
/*                                                                           */
/*****************************************************************************/

/*****************************************************************************/
/*                                                                           */
/*  KHE_LINK_EVENTS_MONITOR KheLinkEventsMonitorMake(KHE_SOLN soln,          */
/*    KHE_LINK_EVENTS_CONSTRAINT c, KHE_EVENT_GROUP eg)                      */
/*                                                                           */
/*  Make a new link events monitor with these attributes.                    */
/*                                                                           */
/*****************************************************************************/

KHE_LINK_EVENTS_MONITOR KheLinkEventsMonitorMake(KHE_SOLN soln,
  KHE_LINK_EVENTS_CONSTRAINT c, KHE_EVENT_GROUP eg)
{
  KHE_LINK_EVENTS_MONITOR res;  KHE_INSTANCE ins;
  KHE_EVENT e;  KHE_EVENT_IN_SOLN es;  int i;

  /* make the monitor */
  MMake(res);
  KheMonitorInitCommonFields((KHE_MONITOR) res, soln,
    KHE_LINK_EVENTS_MONITOR_TAG);
  res->constraint = c;
  res->event_group = eg;
  res->wanted_multiplicity = KheEventGroupEventCount(eg);
  ins = KheSolnInstance(soln);
  MArrayInit(res->multiplicities);
  MArrayFill(res->multiplicities, KheInstanceTimeCount(ins), 0);
  res->deviation = 0;
  res->new_deviation = 0;
  res->copy = NULL;

  /* add (but don't attach) to the monitored event in soln objects */
  for( i = 0;  i < KheEventGroupEventCount(res->event_group);  i++ )
  {
    e = KheEventGroupEvent(res->event_group, i);
    es = KheSolnEventInSoln(soln, KheEventIndex(e));
    KheEventInSolnAddMonitor(es, (KHE_MONITOR) res);
  }
  return res;
}


/*****************************************************************************/
/*                                                                           */
/*  KHE_LINK_EVENTS_CONSTRAINT KheLinkEventsMonitorConstraint(               */
/*    KHE_LINK_EVENTS_MONITOR m)                                             */
/*                                                                           */
/*  Return the constraint that m was derived from.                           */
/*                                                                           */
/*****************************************************************************/

KHE_LINK_EVENTS_CONSTRAINT KheLinkEventsMonitorConstraint(
  KHE_LINK_EVENTS_MONITOR m)
{
  return m->constraint;
}


/*****************************************************************************/
/*                                                                           */
/*  int KheLinkEventsMonitorEventGroupIndex(KHE_LINK_EVENTS_MONITOR m)       */
/*                                                                           */
/*  Return the event group within m's constraint that m monitors.            */
/*                                                                           */
/*****************************************************************************/

KHE_EVENT_GROUP KheLinkEventsMonitorEventGroup(KHE_LINK_EVENTS_MONITOR m)
{
  return m->event_group;
}


/*****************************************************************************/
/*                                                                           */
/*  KHE_LINK_EVENTS_MONITOR KheLinkEventsMonitorCopyPhase1(                  */
/*    KHE_LINK_EVENTS_MONITOR m)                                             */
/*                                                                           */
/*  Carry out Phase 1 of the copying of m.                                   */
/*                                                                           */
/*****************************************************************************/

KHE_LINK_EVENTS_MONITOR KheLinkEventsMonitorCopyPhase1(
  KHE_LINK_EVENTS_MONITOR m)
{
  KHE_LINK_EVENTS_MONITOR copy;  int i;
  if( m->copy == NULL )
  {
    MMake(copy);
    m->copy = copy;
    KheMonitorCopyCommonFields((KHE_MONITOR) copy, (KHE_MONITOR) m);
    copy->constraint = m->constraint;
    copy->event_group = m->event_group;
    copy->wanted_multiplicity = m->wanted_multiplicity;
    MArrayInit(copy->multiplicities);
    MArrayAppend(copy->multiplicities, m->multiplicities, i);
    copy->deviation = m->deviation;
    copy->new_deviation = m->new_deviation;
    copy->copy = NULL;
  }
  return m->copy;
}


/*****************************************************************************/
/*                                                                           */
/*  void KheLinkEventsMonitorCopyPhase2(KHE_LINK_EVENTS_MONITOR m)           */
/*                                                                           */
/*  Carry out Phase 2 of the copying of m.                                   */
/*                                                                           */
/*****************************************************************************/

void KheLinkEventsMonitorCopyPhase2(KHE_LINK_EVENTS_MONITOR m)
{
  if( m->copy != NULL )
  {
    m->copy = NULL;
  }
}


/*****************************************************************************/
/*                                                                           */
/*  void KheLinkEventsMonitorDelete(KHE_LINK_EVENTS_MONITOR m)               */
/*                                                                           */
/*  Delete m.                                                                */
/*                                                                           */
/*****************************************************************************/

void KheLinkEventsMonitorDelete(KHE_LINK_EVENTS_MONITOR m)
{
  int i;  KHE_EVENT e;  KHE_EVENT_IN_SOLN es;
  if( m->attached )
    KheLinkEventsMonitorDetachFromSoln(m);
  if( m->parent_monitor != NULL )
    KheGroupMonitorDeleteChildMonitor(m->parent_monitor, (KHE_MONITOR) m);
  for( i = 0;  i < KheEventGroupEventCount(m->event_group);  i++ )
  {
    e = KheEventGroupEvent(m->event_group, i);
    es = KheSolnEventInSoln(m->soln, KheEventIndex(e));
    KheEventInSolnDeleteMonitor(es, (KHE_MONITOR) m);
  }
  KheSolnDeleteMonitor(m->soln, (KHE_MONITOR) m);
  MArrayFree(m->multiplicities);
  MFree(m);
}


/*****************************************************************************/
/*                                                                           */
/*  Submodule "attach and detach"                                            */
/*                                                                           */
/*****************************************************************************/

/*****************************************************************************/
/*                                                                           */
/*  void KheLinkEventsMonitorAttachToSoln(KHE_LINK_EVENTS_MONITOR m)         */
/*                                                                           */
/*  Attach m.  It is known to be currently detached with cost 0.             */
/*                                                                           */
/*****************************************************************************/

void KheLinkEventsMonitorAttachToSoln(KHE_LINK_EVENTS_MONITOR m)
{
  int i;  KHE_EVENT e;  KHE_EVENT_IN_SOLN es;
  m->attached = true;
  for( i = 0;  i < KheEventGroupEventCount(m->event_group);  i++ )
  {
    e = KheEventGroupEvent(m->event_group, i);
    es = KheSolnEventInSoln(m->soln, KheEventIndex(e));
    KheEventInSolnAttachMonitor(es, (KHE_MONITOR) m);
  }
}


/*****************************************************************************/
/*                                                                           */
/*  void KheLinkEventsMonitorDetachFromSoln(KHE_LINK_EVENTS_MONITOR m)       */
/*                                                                           */
/*  Detach m.  It is known to be currently attached.                         */
/*                                                                           */
/*****************************************************************************/

void KheLinkEventsMonitorDetachFromSoln(KHE_LINK_EVENTS_MONITOR m)
{
  int i;  KHE_EVENT e;  KHE_EVENT_IN_SOLN es;
  for( i = 0;  i < KheEventGroupEventCount(m->event_group);  i++ )
  {
    e = KheEventGroupEvent(m->event_group, i);
    es = KheSolnEventInSoln(m->soln, KheEventIndex(e));
    KheEventInSolnDetachMonitor(es, (KHE_MONITOR) m);
  }
  m->attached = false;
}


/*****************************************************************************/
/*                                                                           */
/*  bool KheLinkEventsMonitorMustHaveZeroCost(KHE_LINK_EVENTS_MONITOR m)     */
/*                                                                           */
/*  Return true if m must have zero cost, because its events' meets have     */
/*  equal durations, leader meets, and offsets into leader meets.            */
/*                                                                           */
/*****************************************************************************/

static bool KheLinkEventsMonitorMustHaveZeroCost(KHE_LINK_EVENTS_MONITOR m)
{
  ARRAY_KHE_MEET leader_meets;  KHE_MEET leader_meet, meet;  KHE_EVENT e;
  ARRAY_INT offsets, durations, multiplicities;  int i, j, k, offset;

  /* if there are no events, then there is no cost */
  if( KheEventGroupEventCount(m->event_group) == 0 )
    return true;

  /* find the leader meets, durations, and offsets of the first event */
  MArrayInit(durations);
  MArrayInit(leader_meets);
  MArrayInit(offsets);
  MArrayInit(multiplicities);
  e = KheEventGroupEvent(m->event_group, 0);
  for( j = 0;  j < KheEventMeetCount(m->soln, e);  j++ )
  {
    meet = KheEventMeet(m->soln, e, j);
    leader_meet = KheMeetLeader(meet, &offset);
    if( leader_meet == NULL )
    {
      /* fail if meet has no leader meet */
      MArrayFree(durations);
      MArrayFree(leader_meets);
      MArrayFree(offsets);
      MArrayFree(multiplicities);
      return false;
    }
    MArrayAddLast(durations, KheMeetDuration(meet));
    MArrayAddLast(leader_meets, leader_meet);
    MArrayAddLast(offsets, offset);
    MArrayAddLast(multiplicities, 1);
  }

  /* check consistency of the leader meets and offsets of the other events */
  for( i = 1;  i < KheEventGroupEventCount(m->event_group);  i++ )
  {
    e = KheEventGroupEvent(m->event_group, i);

    /* fail if e has a different number of meets from the the first e */
    if( KheEventMeetCount(m->soln, e) != MArraySize(leader_meets) )
    {
      MArrayFree(durations);
      MArrayFree(leader_meets);
      MArrayFree(offsets);
      MArrayFree(multiplicities);
      return false;
    }

    /* fail if any of e's meets don't match up, including having no leader */
    for( j = 0;  j < KheEventMeetCount(m->soln, e);  j++ )
    {
      meet = KheEventMeet(m->soln, e, j);

      /* search for a leader meet that meet matches with, not already nabbed */
      leader_meet = KheMeetRoot(meet, &offset);
      for( k = 0;  k < MArraySize(leader_meets);  k++ )
	if( KheMeetDuration(meet) == MArrayGet(durations, k) &&
	    leader_meet == MArrayGet(leader_meets, k) &&
	    offset == MArrayGet(offsets, k) &&
	    i == MArrayGet(multiplicities, k) )
	  break;

      /* if no luck, clean up and exit with failure */
      if( k >= MArraySize(leader_meets) )
      {
	MArrayFree(durations);
	MArrayFree(leader_meets);
	MArrayFree(offsets);
	MArrayFree(multiplicities);
	return false;
      }

      /* success, but nab index k so it isn't matched twice by e's meets */
      MArrayInc(multiplicities, k);
    }
  }

  /* all correct */
  MArrayFree(durations);
  MArrayFree(leader_meets);
  MArrayFree(offsets);
  MArrayFree(multiplicities);
  return true;
}


/*****************************************************************************/
/*                                                                           */
/*  void KheLinkEventsMonitorAttachCheck(KHE_LINK_EVENTS_MONITOR m)          */
/*                                                                           */
/*  Check the attachment of m.                                               */
/*                                                                           */
/*****************************************************************************/

void KheLinkEventsMonitorAttachCheck(KHE_LINK_EVENTS_MONITOR m)
{
  if( KheLinkEventsMonitorMustHaveZeroCost(m) )
  {
    if( KheMonitorAttachedToSoln((KHE_MONITOR) m) )
      KheMonitorDetachFromSoln((KHE_MONITOR) m);
  }
  else
  {
    if( !KheMonitorAttachedToSoln((KHE_MONITOR) m) )
      KheMonitorAttachToSoln((KHE_MONITOR) m);
  }
}


/*****************************************************************************/
/*                                                                           */
/*  Submodule "monitoring calls"                                             */
/*                                                                           */
/*****************************************************************************/

/*****************************************************************************/
/*                                                                           */
/*  int ValDeviations(KHE_LINK_EVENTS_MONITOR m, int val)                    */
/*                                                                           */
/*  Return the number of deviations associated with val.  This will be 0     */
/*  if val is 0 or the wanted multiplicity, and 1 otherwise.                 */
/*                                                                           */
/*****************************************************************************/

static int ValDeviations(KHE_LINK_EVENTS_MONITOR m, int val)
{
  return val != 0 && val != m->wanted_multiplicity ? 1 : 0;
}


/*****************************************************************************/
/*                                                                           */
/*  void KheLinkEventsMonitorAssignNonClash(KHE_LINK_EVENTS_MONITOR m,       */
/*    int assigned_time_index)                                               */
/*                                                                           */
/*  Inform m that one of the events it is monitoring has been assigned       */
/*  this time for the first time (i.e. it's not a clash).                    */
/*                                                                           */
/*****************************************************************************/

void KheLinkEventsMonitorAssignNonClash(KHE_LINK_EVENTS_MONITOR m,
  int assigned_time_index)
{
  int old_val, new_val;
  old_val = MArrayGet(m->multiplicities, assigned_time_index);
  new_val = old_val + 1;
  MAssert(new_val <= m->wanted_multiplicity,
    "KheLinkEventsMonitorAssignNonClash internal error");
  MArrayPut(m->multiplicities, assigned_time_index, new_val);
  m->new_deviation += (ValDeviations(m, new_val) - ValDeviations(m, old_val));
}


/*****************************************************************************/
/*                                                                           */
/*  void KheLinkEventsMonitorUnAssignNonClash(KHE_LINK_EVENTS_MONITOR m,     */
/*    int assigned_time_index)                                               */
/*                                                                           */
/*  Inform m that one of the events it is monitoring has been unassigned     */
/*  this time for the first time (i.e. it's not a clash).                    */
/*                                                                           */
/*****************************************************************************/

void KheLinkEventsMonitorUnAssignNonClash(KHE_LINK_EVENTS_MONITOR m,
  int assigned_time_index)
{
  int old_val, new_val;
  old_val = MArrayGet(m->multiplicities, assigned_time_index);
  new_val = old_val - 1;
  MAssert(new_val >= 0, "KheLinkEventsMonitorUnAssignNonClash internal error");
  MArrayPut(m->multiplicities, assigned_time_index, new_val);
  m->new_deviation += (ValDeviations(m, new_val) - ValDeviations(m, old_val));
}


/*****************************************************************************/
/*                                                                           */
/*  void KheLinkEventsMonitorFlush(KHE_LINK_EVENTS_MONITOR m)                */
/*                                                                           */
/*  Flush m.                                                                 */
/*                                                                           */
/*****************************************************************************/

void KheLinkEventsMonitorFlush(KHE_LINK_EVENTS_MONITOR m)
{
  if( m->new_deviation != m->deviation )
  {
    KheMonitorChangeCost((KHE_MONITOR) m,
      KheConstraintCost((KHE_CONSTRAINT) m->constraint, m->new_deviation));
    m->deviation = m->new_deviation;
  }
}


/*****************************************************************************/
/*                                                                           */
/*  Submodule "deviations"                                                   */
/*                                                                           */
/*****************************************************************************/

/*****************************************************************************/
/*                                                                           */
/*  int KheLinkEventsMonitorDeviationCount(KHE_LINK_EVENTS_MONITOR m)        */
/*                                                                           */
/*  Return the number of deviations of m.                                    */
/*                                                                           */
/*****************************************************************************/

int KheLinkEventsMonitorDeviationCount(KHE_LINK_EVENTS_MONITOR m)
{
  return 1;
}


/*****************************************************************************/
/*                                                                           */
/*  int KheLinkEventsMonitorDeviation(KHE_LINK_EVENTS_MONITOR m, int i)      */
/*                                                                           */
/*  Return the i'th deviation.                                               */
/*                                                                           */
/*****************************************************************************/

int KheLinkEventsMonitorDeviation(KHE_LINK_EVENTS_MONITOR m, int i)
{
  MAssert(i == 0, "KheLinkEventsMonitorDeviation: i out of range");
  return m->deviation;
}


/*****************************************************************************/
/*                                                                           */
/*  char *KheLinkEventsMonitorDeviationDescription(                          */
/*    KHE_LINK_EVENTS_MONITOR m, int i)                                      */
/*                                                                           */
/*  Return a description of the i'th deviation.                              */
/*                                                                           */
/*****************************************************************************/

char *KheLinkEventsMonitorDeviationDescription(
  KHE_LINK_EVENTS_MONITOR m, int i)
{
  MAssert(i == 0, "KheLinkEventsMonitorDeviationDescription: i out of range");
  return NULL;
}


/*****************************************************************************/
/*                                                                           */
/*  Submodule "debug"                                                        */
/*                                                                           */
/*****************************************************************************/

/*****************************************************************************/
/*                                                                           */
/*  void KheLinkEventsMonitorDebug(KHE_LINK_EVENTS_MONITOR m,                */
/*    int verbosity, int indent, FILE *fp)                                   */
/*                                                                           */
/*  Debug print of m onto fp with the given verbosity and indent.            */
/*                                                                           */
/*****************************************************************************/

void KheLinkEventsMonitorDebug(KHE_LINK_EVENTS_MONITOR m,
  int verbosity, int indent, FILE *fp)
{
  if( verbosity >= 1 )
  {
    KheMonitorDebugBegin((KHE_MONITOR) m, indent, fp);
    fprintf(fp, " %s",
      KheConstraintId((KHE_CONSTRAINT) m->constraint) == NULL ? "-" :
      KheConstraintId((KHE_CONSTRAINT) m->constraint));
    KheMonitorDebugEnd((KHE_MONITOR) m, true, indent, fp);
  }
}
