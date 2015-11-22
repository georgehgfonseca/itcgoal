
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
/*  FILE:         khe_distribute_split_events_monitor.c                      */
/*  DESCRIPTION:  A distribute split events monitor                          */
/*                                                                           */
/*****************************************************************************/
#include "khe_interns.h"


/*****************************************************************************/
/*                                                                           */
/*  KHE_DISTRIBUTE_SPLIT_EVENTS_MONITOR - an distribute split events monitor */
/*                                                                           */
/*****************************************************************************/

struct khe_distribute_split_events_monitor_rec {

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

  /* specific to KHE_DISTRIBUTE_SPLIT_EVENTS_MONITOR */
  KHE_EVENT_IN_SOLN		event_in_soln;		/* enclosing es      */
  KHE_DISTRIBUTE_SPLIT_EVENTS_CONSTRAINT constraint;	/* constraint        */
  int				duration;		/* Duration          */
  int				minimum;		/* Minimum           */
  int				maximum;		/* Maximum           */
  int				meet_count;		/* with duration     */
  int				deviation;		/* deviation         */
  KHE_DISTRIBUTE_SPLIT_EVENTS_MONITOR copy;		/* used when copying */
};


/*****************************************************************************/
/*                                                                           */
/*  Submodule "construction and query"                                       */
/*                                                                           */
/*****************************************************************************/

/*****************************************************************************/
/*                                                                           */
/*  KHE_DISTRIBUTE_SPLIT_EVENTS_MONITOR KheDistributeSplitEventsMonitorMake( */
/*    KHE_EVENT_IN_SOLN es, KHE_DISTRIBUTE_SPLIT_EVENTS_CONSTRAINT c)        */
/*                                                                           */
/*  Make a new distribute split events monitor with these attributes.        */
/*                                                                           */
/*****************************************************************************/

KHE_DISTRIBUTE_SPLIT_EVENTS_MONITOR KheDistributeSplitEventsMonitorMake(
  KHE_EVENT_IN_SOLN es, KHE_DISTRIBUTE_SPLIT_EVENTS_CONSTRAINT c)
{
  KHE_DISTRIBUTE_SPLIT_EVENTS_MONITOR res;  KHE_SOLN soln;
  MMake(res);
  soln = KheEventInSolnSoln(es);
  KheMonitorInitCommonFields((KHE_MONITOR) res, soln,
    KHE_DISTRIBUTE_SPLIT_EVENTS_MONITOR_TAG);
  res->event_in_soln = es;
  res->constraint = c;
  res->duration = KheDistributeSplitEventsConstraintDuration(c);
  res->minimum = KheDistributeSplitEventsConstraintMinimum(c);
  res->maximum = KheDistributeSplitEventsConstraintMaximum(c);
  res->meet_count = 0;
  res->deviation = 0;
  res->copy = NULL;
  KheEventInSolnAddMonitor(es, (KHE_MONITOR) res);
  return res;
}


/*****************************************************************************/
/*                                                                           */
/*  KHE_DISTRIBUTE_SPLIT_EVENTS_MONITOR                                      */
/*    KheDistributeSplitEventsMonitorCopyPhase1(                             */
/*    KHE_DISTRIBUTE_SPLIT_EVENTS_MONITOR m)                                 */
/*                                                                           */
/*  Carry out Phase 1 of copying m.                                          */
/*                                                                           */
/*****************************************************************************/

KHE_DISTRIBUTE_SPLIT_EVENTS_MONITOR KheDistributeSplitEventsMonitorCopyPhase1(
  KHE_DISTRIBUTE_SPLIT_EVENTS_MONITOR m)
{
  KHE_DISTRIBUTE_SPLIT_EVENTS_MONITOR copy;
  if( m->copy == NULL )
  {
    MMake(copy);
    m->copy = copy;
    KheMonitorCopyCommonFields((KHE_MONITOR) copy, (KHE_MONITOR) m);
    copy->event_in_soln = KheEventInSolnCopyPhase1(m->event_in_soln);
    copy->constraint = m->constraint;
    copy->duration = m->duration;
    copy->minimum = m->minimum;
    copy->maximum = m->maximum;
    copy->meet_count = m->meet_count;
    copy->deviation = m->deviation;
    copy->copy = NULL;
  }
  return m->copy;
}


/*****************************************************************************/
/*                                                                           */
/*  void KheDistributeSplitEventsMonitorCopyPhase2(                          */
/*    KHE_DISTRIBUTE_SPLIT_EVENTS_MONITOR m)                                 */
/*                                                                           */
/*  Carry out Phase 2 of copying m.                                          */
/*                                                                           */
/*****************************************************************************/

void KheDistributeSplitEventsMonitorCopyPhase2(
  KHE_DISTRIBUTE_SPLIT_EVENTS_MONITOR m)
{
  if( m->copy != NULL )
  {
    m->copy = NULL;
  }
}


/*****************************************************************************/
/*                                                                           */
/*  void KheDistributeSplitEventsMonitorDelete(                              */
/*    KHE_DISTRIBUTE_SPLIT_EVENTS_MONITOR m)                                 */
/*                                                                           */
/*  Delete m.                                                                */
/*                                                                           */
/*****************************************************************************/

void KheDistributeSplitEventsMonitorDelete(
  KHE_DISTRIBUTE_SPLIT_EVENTS_MONITOR m)
{
  if( m->attached )
    KheDistributeSplitEventsMonitorDetachFromSoln(m);
  if( m->parent_monitor != NULL )
    KheGroupMonitorDeleteChildMonitor(m->parent_monitor, (KHE_MONITOR) m);
  KheEventInSolnDeleteMonitor(m->event_in_soln, (KHE_MONITOR) m);
  KheSolnDeleteMonitor(m->soln, (KHE_MONITOR) m);
  MFree(m);
}


/*****************************************************************************/
/*                                                                           */
/*  KHE_DISTRIBUTE_SPLIT_EVENTS_CONSTRAINT                                   */
/*    KheDistributeSplitEventsMonitorConstraint(                             */
/*    KHE_DISTRIBUTE_SPLIT_EVENTS_MONITOR m)                                 */
/*                                                                           */
/*  Return the constraint that m is monitoring.                              */
/*                                                                           */
/*****************************************************************************/

KHE_DISTRIBUTE_SPLIT_EVENTS_CONSTRAINT
  KheDistributeSplitEventsMonitorConstraint(
  KHE_DISTRIBUTE_SPLIT_EVENTS_MONITOR m)
{
  return m->constraint;
}


/*****************************************************************************/
/*                                                                           */
/*  KHE_EVENT KheDistributeSplitEventsMonitorEvent(                          */
/*    KHE_DISTRIBUTE_SPLIT_EVENTS_MONITOR m)                                 */
/*                                                                           */
/*  Return the event that m is monitoring.                                   */
/*                                                                           */
/*****************************************************************************/

KHE_EVENT KheDistributeSplitEventsMonitorEvent(
  KHE_DISTRIBUTE_SPLIT_EVENTS_MONITOR m)
{
  return KheEventInSolnEvent(m->event_in_soln);
}


/*****************************************************************************/
/*                                                                           */
/*  Submodule "attach and detach"                                            */
/*                                                                           */
/*****************************************************************************/

/*****************************************************************************/
/*                                                                           */
/*  void KheDistributeSplitEventsMonitorAttachToSoln(                        */
/*    KHE_DISTRIBUTE_SPLIT_EVENTS_MONITOR m)                                 */
/*                                                                           */
/*  Attach m.  It is known to be currently detached with cost 0.             */
/*                                                                           */
/*****************************************************************************/

void KheDistributeSplitEventsMonitorAttachToSoln(
  KHE_DISTRIBUTE_SPLIT_EVENTS_MONITOR m)
{
  m->attached = true;
  m->deviation = m->minimum;
  KheMonitorChangeCost((KHE_MONITOR) m,
    KheConstraintCost((KHE_CONSTRAINT) m->constraint, m->deviation));
  KheEventInSolnAttachMonitor(m->event_in_soln, (KHE_MONITOR) m);
}


/*****************************************************************************/
/*                                                                           */
/*  void KheDistributeSplitEventsMonitorDetachFromSoln(                      */
/*    KHE_DISTRIBUTE_SPLIT_EVENTS_MONITOR m)                                 */
/*                                                                           */
/*  Detach m.  It is known to be currently attached.                         */
/*                                                                           */
/*****************************************************************************/

void KheDistributeSplitEventsMonitorDetachFromSoln(
  KHE_DISTRIBUTE_SPLIT_EVENTS_MONITOR m)
{
  KheEventInSolnDetachMonitor(m->event_in_soln, (KHE_MONITOR) m);
  MAssert(m->deviation == m->minimum,
    "KheDistributeSplitEventsMonitorDetach internal error");
  m->deviation = 0;
  KheMonitorChangeCost((KHE_MONITOR) m, 0);
  m->attached = false;
}


/*****************************************************************************/
/*                                                                           */
/*  void KheDistributeSplitEventsMonitorAttachCheck(                         */
/*    KHE_DISTRIBUTE_SPLIT_EVENTS_MONITOR m)                                 */
/*                                                                           */
/*  Check the attachment of m.  Since event splitting is considered to be    */
/*  structural, the way to do this is to ensure it is attached, and then     */
/*  detach it if its cost is 0.                                              */
/*                                                                           */
/*****************************************************************************/

void KheDistributeSplitEventsMonitorAttachCheck(
  KHE_DISTRIBUTE_SPLIT_EVENTS_MONITOR m)
{
  if( !KheMonitorAttachedToSoln((KHE_MONITOR) m) )
    KheMonitorAttachToSoln((KHE_MONITOR) m);
  if( m->cost == 0 )
    KheMonitorDetachFromSoln((KHE_MONITOR) m);
}


/*****************************************************************************/
/*                                                                           */
/*  Submodule "monitoring calls"                                             */
/*                                                                           */
/*****************************************************************************/

/*****************************************************************************/
/*                                                                           */
/*  int KheDistributeSplitEventsMonitorAddOneMeet(                           */
/*    KHE_DISTRIBUTE_SPLIT_EVENTS_MONITOR m)                                 */
/*                                                                           */
/*  Do what's required to add one meet of the correct duration, and return   */
/*  the change in deviations.                                                */
/*                                                                           */
/*****************************************************************************/

static int KheDistributeSplitEventsMonitorAddOneMeet(
  KHE_DISTRIBUTE_SPLIT_EVENTS_MONITOR m)
{
  m->meet_count++;
  if( m->meet_count <= m->minimum )
    return -1;
  else if( m->meet_count > m->maximum )
    return 1;
  else
    return 0;
}


/*****************************************************************************/
/*                                                                           */
/*  int KheDistributeSplitEventsMonitorDeleteOneMeet(                        */
/*    KHE_DISTRIBUTE_SPLIT_EVENTS_MONITOR m)                                 */
/*                                                                           */
/*  Do what's required to delete one meet of the correct duration, and       */
/*  return the change in deviations.                                         */
/*                                                                           */
/*****************************************************************************/

static int KheDistributeSplitEventsMonitorDeleteOneMeet(
  KHE_DISTRIBUTE_SPLIT_EVENTS_MONITOR m)
{
  m->meet_count--;
  if( m->meet_count < m->minimum )
    return 1;
  else if( m->meet_count >= m->maximum )
    return -1;
  else
    return 0;
}


/*****************************************************************************/
/*                                                                           */
/*  void KheDistributeSplitEventsMonitorAddMeet(                             */
/*    KHE_DISTRIBUTE_SPLIT_EVENTS_MONITOR m, KHE_MEET meet)                  */
/*                                                                           */
/*  Monitor the effect of adding meet.s                                      */
/*                                                                           */
/*****************************************************************************/

void KheDistributeSplitEventsMonitorAddMeet(
  KHE_DISTRIBUTE_SPLIT_EVENTS_MONITOR m, KHE_MEET meet)
{
  int delta_deviation;
  if( KheMeetDuration(meet) == m->duration )
  {
    /* update meet_count and find the change in deviation */
    delta_deviation = KheDistributeSplitEventsMonitorAddOneMeet(m);

    /* do the update if deviations have changed */
    if( delta_deviation != 0 )
    {
      m->deviation += delta_deviation;
      KheMonitorChangeCost((KHE_MONITOR) m,
	KheConstraintCost((KHE_CONSTRAINT) m->constraint, m->deviation));
    }
  }
}


/*****************************************************************************/
/*                                                                           */
/*  void KheDistributeSplitEventsMonitorDeleteMeet(                          */
/*    KHE_DISTRIBUTE_SPLIT_EVENTS_MONITOR m, KHE_MEET meet)                  */
/*                                                                           */
/*  Monitor the effect of deleting meet.                                     */
/*                                                                           */
/*****************************************************************************/

void KheDistributeSplitEventsMonitorDeleteMeet(
  KHE_DISTRIBUTE_SPLIT_EVENTS_MONITOR m, KHE_MEET meet)
{
  int delta_deviation;
  if( KheMeetDuration(meet) == m->duration )
  {
    /* update meet_count and find the change in deviation */
    delta_deviation = KheDistributeSplitEventsMonitorDeleteOneMeet(m);

    /* do the update if deviations have changed */
    if( delta_deviation != 0 )
    {
      m->deviation += delta_deviation;
      KheMonitorChangeCost((KHE_MONITOR) m,
	KheConstraintCost((KHE_CONSTRAINT) m->constraint, m->deviation));
    }
  }
}


/*****************************************************************************/
/*                                                                           */
/*  void KheDistributeSplitEventsMonitorSplitMeet(                           */
/*    KHE_DISTRIBUTE_SPLIT_EVENTS_MONITOR m, KHE_MEET meet1, KHE_MEET meet2) */
/*                                                                           */
/*  Let m know that a meet has just split into meet1 and meet2.              */
/*  Either both meets are assigned times, or they aren't.                    */
/*                                                                           */
/*****************************************************************************/

void KheDistributeSplitEventsMonitorSplitMeet(
  KHE_DISTRIBUTE_SPLIT_EVENTS_MONITOR m, KHE_MEET meet1, KHE_MEET meet2)
{
  int durn1, durn2, delta_deviation;

  /* update meet_count and find the change in deviation */
  durn1 = KheMeetDuration(meet1);
  durn2 = KheMeetDuration(meet2);
  delta_deviation = 0;
  if( durn1 + durn2 == m->duration )
    delta_deviation += KheDistributeSplitEventsMonitorDeleteOneMeet(m);
  if( durn1 == m->duration )
    delta_deviation += KheDistributeSplitEventsMonitorAddOneMeet(m);
  if( durn2 == m->duration )
    delta_deviation += KheDistributeSplitEventsMonitorAddOneMeet(m);

  /* do the update if deviations have changed */
  if( delta_deviation != 0 )
  {
    m->deviation += delta_deviation;
    KheMonitorChangeCost((KHE_MONITOR) m,
      KheConstraintCost((KHE_CONSTRAINT) m->constraint, m->deviation));
  }
}


/*****************************************************************************/
/*                                                                           */
/*  void KheDistributeSplitEventsMonitorMergeMeet(                           */
/*    KHE_DISTRIBUTE_SPLIT_EVENTS_MONITOR m, KHE_MEET meet1, KHE_MEET meet2) */
/*                                                                           */
/*  Let m know that meet1 and meet2 are just about to be merged.             */
/*                                                                           */
/*  It might seem that there is a problem with this code of meet is assigned */
/*  a time and merge_meet is not.  However, this code is called only by      */
/*  KheMeetMerge, and it is a precondition of that function that             */
/*  meet and merge_meet must either both be unassigned (and hence both not   */
/*  assigned a time) or else assigned to the same meet (and hence            */
/*  both either assigned a time or not).                                     */
/*                                                                           */
/*  KheMeetMerge could allow meet to be assigned and merge_meet to not be,   */
/*  at least when the combined duration is acceptable; but it doesn't.       */
/*                                                                           */
/*****************************************************************************/

void KheDistributeSplitEventsMonitorMergeMeet(
  KHE_DISTRIBUTE_SPLIT_EVENTS_MONITOR m, KHE_MEET meet1, KHE_MEET meet2)
{
  int durn1, durn2, delta_deviation;

  /* update meet_count and find the change in deviation */
  durn1 = KheMeetDuration(meet1);
  durn2 = KheMeetDuration(meet2);
  delta_deviation = 0;
  if( durn1 == m->duration )
    delta_deviation += KheDistributeSplitEventsMonitorDeleteOneMeet(m);
  if( durn2 == m->duration )
    delta_deviation += KheDistributeSplitEventsMonitorDeleteOneMeet(m);
  if( durn1 + durn2 == m->duration )
    delta_deviation += KheDistributeSplitEventsMonitorAddOneMeet(m);

  /* do the update if deviations have changed */
  if( delta_deviation != 0 )
  {
    m->deviation += delta_deviation;
    KheMonitorChangeCost((KHE_MONITOR) m,
      KheConstraintCost((KHE_CONSTRAINT) m->constraint, m->deviation));
  }
}


/*****************************************************************************/
/*                                                                           */
/*  Submodule "deviations"                                                   */
/*                                                                           */
/*****************************************************************************/

/*****************************************************************************/
/*                                                                           */
/*  int KheDistributeSplitEventsMonitorDeviationCount(                       */
/*    KHE_DISTRIBUTE_SPLIT_EVENTS_MONITOR m)                                 */
/*                                                                           */
/*  Return the number of deviations of m.                                    */
/*                                                                           */
/*****************************************************************************/

int KheDistributeSplitEventsMonitorDeviationCount(
  KHE_DISTRIBUTE_SPLIT_EVENTS_MONITOR m)
{
  return 1;
}


/*****************************************************************************/
/*                                                                           */
/*  int KheDistributeSplitEventsMonitorDeviation(                            */
/*    KHE_DISTRIBUTE_SPLIT_EVENTS_MONITOR m, int i)                          */
/*                                                                           */
/*  Return the i'th deviation of m.                                          */
/*                                                                           */
/*****************************************************************************/

int KheDistributeSplitEventsMonitorDeviation(
  KHE_DISTRIBUTE_SPLIT_EVENTS_MONITOR m, int i)
{
  MAssert(i == 0, "KheDistributeSplitEventsMonitorDeviation: i out of range");
  return m->deviation;
}


/*****************************************************************************/
/*                                                                           */
/*  char *KheDistributeSplitEventsMonitorDeviationDescription(               */
/*    KHE_DISTRIBUTE_SPLIT_EVENTS_MONITOR m, int i)                          */
/*                                                                           */
/*  Return a description of the i'th deviation of m.                         */
/*                                                                           */
/*****************************************************************************/

char *KheDistributeSplitEventsMonitorDeviationDescription(
  KHE_DISTRIBUTE_SPLIT_EVENTS_MONITOR m, int i)
{
  MAssert(i == 0,
    "KheDistributeSplitEventsMonitorDeviationDescription: i out of range");
  return NULL;
}


/*****************************************************************************/
/*                                                                           */
/*  Submodule "debug"                                                        */
/*                                                                           */
/*****************************************************************************/

/*****************************************************************************/
/*                                                                           */
/*  void KheDistributeSplitEventsMonitorDebug(                               */
/*    KHE_DISTRIBUTE_SPLIT_EVENTS_MONITOR m, int verbosity,                  */
/*    int indent, FILE *fp)                                                  */
/*                                                                           */
/*  Debug print of m onto fp with the given verbosity and indent.            */
/*                                                                           */
/*****************************************************************************/

void KheDistributeSplitEventsMonitorDebug(
  KHE_DISTRIBUTE_SPLIT_EVENTS_MONITOR m, int verbosity, int indent, FILE *fp)
{
  if( verbosity >= 1 )
  {
    KheMonitorDebugBegin((KHE_MONITOR) m, indent, fp);
    fprintf(fp, " ");
    KheEventInSolnDebug(m->event_in_soln, 1, -1, fp);
    KheMonitorDebugEnd((KHE_MONITOR) m, true, indent, fp);
  }
}
