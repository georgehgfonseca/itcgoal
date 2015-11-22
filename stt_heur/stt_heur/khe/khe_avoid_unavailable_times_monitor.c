
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
/*  FILE:         khe_avoid_unavailable_times_monitor.c                      */
/*  DESCRIPTION:  An avoid unavailable times monitor                         */
/*                                                                           */
/*****************************************************************************/
#include "khe_interns.h"


/*****************************************************************************/
/*                                                                           */
/*  KHE_AVOID_UNAVAILABLE_TIMES_MONITOR                                      */
/*                                                                           */
/*****************************************************************************/

struct khe_avoid_unavailable_times_monitor_rec {

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

  /* specific to KHE_AVOID_UNAVAILABLE_TIMES_MONITOR */
  int				deviation;		/* deviation         */
  KHE_RESOURCE_IN_SOLN		resource_in_soln;	/* enclosing rs      */
  KHE_AVOID_UNAVAILABLE_TIMES_CONSTRAINT constraint;	/* monitoring this   */
  KHE_AVOID_UNAVAILABLE_TIMES_MONITOR copy;		/* used when copying */
};


/*****************************************************************************/
/*                                                                           */
/*  Submodule "construction and query"                                       */
/*                                                                           */
/*****************************************************************************/

/*****************************************************************************/
/*                                                                           */
/*  KHE_AVOID_UNAVAILABLE_TIMES_MONITOR KheAvoidUnavailableTimesMonitorMake( */
/*    KHE_RESOURCE_IN_SOLN rs, KHE_AVOID_UNAVAILABLE_TIMES_CONSTRAINT c)     */
/*                                                                           */
/*  Make a new avoid unavailable times monitor object with these attributes. */
/*                                                                           */
/*****************************************************************************/

KHE_AVOID_UNAVAILABLE_TIMES_MONITOR KheAvoidUnavailableTimesMonitorMake(
  KHE_RESOURCE_IN_SOLN rs, KHE_AVOID_UNAVAILABLE_TIMES_CONSTRAINT c)
{
  KHE_AVOID_UNAVAILABLE_TIMES_MONITOR res;  KHE_SOLN soln;
  soln = KheResourceInSolnSoln(rs);
  MMake(res);
  KheMonitorInitCommonFields((KHE_MONITOR) res, soln,
    KHE_AVOID_UNAVAILABLE_TIMES_MONITOR_TAG);
  res->deviation = 0;
  res->resource_in_soln = rs;
  res->constraint = c;
  res->copy = NULL;
  KheResourceInSolnAddMonitor(rs, (KHE_MONITOR) res);
  /* KheGroupMonitorAddMonitor((KHE_GROUP_MONITOR) soln, (KHE_MONITOR) res); */
  return res;
}


/*****************************************************************************/
/*                                                                           */
/*  KHE_AVOID_UNAVAILABLE_TIMES_MONITOR                                      */
/*    KheAvoidUnavailableTimesMonitorCopyPhase1(                             */
/*    KHE_AVOID_UNAVAILABLE_TIMES_MONITOR m)                                 */
/*                                                                           */
/*  Carry out Phase 1 of copying m.                                          */
/*                                                                           */
/*****************************************************************************/

KHE_AVOID_UNAVAILABLE_TIMES_MONITOR KheAvoidUnavailableTimesMonitorCopyPhase1(
  KHE_AVOID_UNAVAILABLE_TIMES_MONITOR m)
{
  KHE_AVOID_UNAVAILABLE_TIMES_MONITOR copy;
  if( m->copy == NULL )
  {
    MMake(copy);
    m->copy = copy;
    KheMonitorCopyCommonFields((KHE_MONITOR) copy, (KHE_MONITOR) m);
    copy->deviation = m->deviation;
    copy->resource_in_soln =
      KheResourceInSolnCopyPhase1(m->resource_in_soln);
    copy->constraint = m->constraint;
    copy->copy = NULL;
  }
  return m->copy;
}


/*****************************************************************************/
/*                                                                           */
/*  void KheAvoidUnavailableTimesMonitorCopyPhase2(                          */
/*    KHE_AVOID_UNAVAILABLE_TIMES_MONITOR m)                                 */
/*                                                                           */
/*  Carry out Phase 2 of copying m.                                          */
/*                                                                           */
/*****************************************************************************/

void KheAvoidUnavailableTimesMonitorCopyPhase2(
  KHE_AVOID_UNAVAILABLE_TIMES_MONITOR m)
{
  if( m->copy != NULL )
  {
    m->copy = NULL;
    KheResourceInSolnCopyPhase2(m->resource_in_soln);
  }
}


/*****************************************************************************/
/*                                                                           */
/*  void KheAvoidUnavailableTimesMonitorDelete(                              */
/*    KHE_AVOID_UNAVAILABLE_TIMES_MONITOR m)                                 */
/*                                                                           */
/*  Delete m.                                                                */
/*                                                                           */
/*****************************************************************************/

void KheAvoidUnavailableTimesMonitorDelete(
  KHE_AVOID_UNAVAILABLE_TIMES_MONITOR m)
{
  if( m->attached )
    KheAvoidUnavailableTimesMonitorDetachFromSoln(m);
  if( m->parent_monitor != NULL )
    KheGroupMonitorDeleteChildMonitor(m->parent_monitor, (KHE_MONITOR) m);
  KheResourceInSolnDeleteMonitor(m->resource_in_soln, (KHE_MONITOR) m);
  KheSolnDeleteMonitor(m->soln, (KHE_MONITOR) m);
  MFree(m);
}


/*****************************************************************************/
/*                                                                           */
/*  KHE_AVOID_UNAVAILABLE_TIMES_CONSTRAINT                                   */
/*    KheAvoidUnavailableTimesMonitorConstraint(                             */
/*    KHE_AVOID_UNAVAILABLE_TIMES_MONITOR m)                                 */
/*                                                                           */
/*  Return the constraint monitored by m.                                    */
/*                                                                           */
/*****************************************************************************/

KHE_AVOID_UNAVAILABLE_TIMES_CONSTRAINT
  KheAvoidUnavailableTimesMonitorConstraint(
  KHE_AVOID_UNAVAILABLE_TIMES_MONITOR m)
{
  return m->constraint;
}


/*****************************************************************************/
/*                                                                           */
/*  KHE_RESOURCE KheAvoidUnavailableTimesMonitorResource(                    */
/*    KHE_AVOID_UNAVAILABLE_TIMES_MONITOR m)                                 */
/*                                                                           */
/*  Return the resource monitored by m.                                      */
/*                                                                           */
/*****************************************************************************/

KHE_RESOURCE KheAvoidUnavailableTimesMonitorResource(
  KHE_AVOID_UNAVAILABLE_TIMES_MONITOR m)
{
  return KheResourceInSolnResource(m->resource_in_soln);
}


/*****************************************************************************/
/*                                                                           */
/*  Submodule "attach and detach"                                            */
/*                                                                           */
/*****************************************************************************/

/*****************************************************************************/
/*                                                                           */
/*  void KheAvoidUnavailableTimesMonitorAttachToSoln(                        */
/*    KHE_AVOID_UNAVAILABLE_TIMES_MONITOR m)                                 */
/*                                                                           */
/*  Attach m.  It is known to be currently detached with cost 0.             */
/*                                                                           */
/*****************************************************************************/

void KheAvoidUnavailableTimesMonitorAttachToSoln(
  KHE_AVOID_UNAVAILABLE_TIMES_MONITOR m)
{
  m->attached = true;
  KheResourceInSolnAttachMonitor(m->resource_in_soln, (KHE_MONITOR) m);
}


/*****************************************************************************/
/*                                                                           */
/*  void KheAvoidUnavailableTimesMonitorDetachFromSoln(                      */
/*    KHE_AVOID_UNAVAILABLE_TIMES_MONITOR m)                                 */
/*                                                                           */
/*  Detach m.  It is known to be currently attached.                         */
/*                                                                           */
/*****************************************************************************/

void KheAvoidUnavailableTimesMonitorDetachFromSoln(
  KHE_AVOID_UNAVAILABLE_TIMES_MONITOR m)
{
  KheResourceInSolnDetachMonitor(m->resource_in_soln, (KHE_MONITOR) m);
  m->attached = false;
}


/*****************************************************************************/
/*                                                                           */
/*  void KheAvoidUnavailableTimesMonitorAttachCheck(                         */
/*    KHE_AVOID_UNAVAILABLE_TIMES_MONITOR m)                                 */
/*                                                                           */
/*  Check the attachment of m.                                               */
/*                                                                           */
/*  There is actually a way to be certain that the solution is structured    */
/*  so that this constraint cannot be violated:  check every task whose      */
/*  domain includes the resource that m is monitoring, to see whether its    */
/*  time domain is disjoint from m's domain.  But that is very slow and      */
/*  likely to fail anyway, so we don't bother.                               */
/*                                                                           */
/*****************************************************************************/

void KheAvoidUnavailableTimesMonitorAttachCheck(
  KHE_AVOID_UNAVAILABLE_TIMES_MONITOR m)
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
/*  void KheAvoidUnavailableTimesMonitorAddBusyAndIdle(                      */
/*    KHE_AVOID_UNAVAILABLE_TIMES_MONITOR m, int busy_count, int idle_count) */
/*                                                                           */
/*  Receive a report of a new lot of busy times, and pass it on.  Idle       */
/*  times are not used by this monitor.                                      */
/*                                                                           */
/*****************************************************************************/

void KheAvoidUnavailableTimesMonitorAddBusyAndIdle(
  KHE_AVOID_UNAVAILABLE_TIMES_MONITOR m, int busy_count, int idle_count)
{
  MAssert(m->deviation == 0,
    "KheAvoidUnavailableTimesMonitorAddBusyAndIdle internal error");
  m->deviation = busy_count;
  KheMonitorChangeCost((KHE_MONITOR) m,
    KheConstraintCost((KHE_CONSTRAINT) m->constraint, m->deviation));
}


/*****************************************************************************/
/*                                                                           */
/*  void KheAvoidUnavailableTimesMonitorDeleteBusyAndIdle(                   */
/*    KHE_AVOID_UNAVAILABLE_TIMES_MONITOR m, int busy_count, int idle_count) */
/*                                                                           */
/*  Receive a report of a lot of busy times being deleted, and pass it on.   */
/*  Idle times are not used by this monitor.                                 */
/*                                                                           */
/*****************************************************************************/

void KheAvoidUnavailableTimesMonitorDeleteBusyAndIdle(
  KHE_AVOID_UNAVAILABLE_TIMES_MONITOR m, int busy_count, int idle_count)
{
  MAssert(m->cost==KheConstraintCost((KHE_CONSTRAINT)m->constraint, busy_count),
    "KheAvoidUnavailableTimesMonitorDeleteBusyAndIdle internal error");
  m->deviation = 0;
  KheMonitorChangeCost((KHE_MONITOR) m, 0);
}


/*****************************************************************************/
/*                                                                           */
/*  void KheAvoidUnavailableTimesMonitorChangeBusyAndIdle(                   */
/*    KHE_AVOID_UNAVAILABLE_TIMES_MONITOR m, int old_busy_count,             */
/*    int new_busy_count, int old_idle_count, int new_idle_count)            */
/*                                                                           */
/*  Receive a report of a change in the number of busy and idle times,       */
/*  and pass it on.  Idle times are not used by this monitor.                */
/*                                                                           */
/*****************************************************************************/

void KheAvoidUnavailableTimesMonitorChangeBusyAndIdle(
  KHE_AVOID_UNAVAILABLE_TIMES_MONITOR m, int old_busy_count,
  int new_busy_count, int old_idle_count, int new_idle_count)
{
  MAssert(old_busy_count == m->deviation,
    "KheAvoidUnavailableTimesMonitorChangeBusyAndIdle internal error");
  if( old_busy_count != new_busy_count )
  {
    m->deviation = new_busy_count;
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
/*  int KheAvoidUnavailableTimesMonitorDeviationCount(                       */
/*    KHE_AVOID_UNAVAILABLE_TIMES_MONITOR m)                                 */
/*                                                                           */
/*  Return the number of deviations of m.                                    */
/*                                                                           */
/*****************************************************************************/

int KheAvoidUnavailableTimesMonitorDeviationCount(
  KHE_AVOID_UNAVAILABLE_TIMES_MONITOR m)
{
  return 1;
}


/*****************************************************************************/
/*                                                                           */
/*  int KheAvoidUnavailableTimesMonitorDeviation(                            */
/*    KHE_AVOID_UNAVAILABLE_TIMES_MONITOR m, int i)                          */
/*                                                                           */
/*  Return the i'th deviation of m.                                          */
/*                                                                           */
/*****************************************************************************/

int KheAvoidUnavailableTimesMonitorDeviation(
  KHE_AVOID_UNAVAILABLE_TIMES_MONITOR m, int i)
{
  MAssert(i == 0, "KheAvoidUnavailableTimesMonitorDeviation: i out of range");
  return m->deviation;
}


/*****************************************************************************/
/*                                                                           */
/*  char *KheAvoidUnavailableTimesMonitorDeviationDescription(               */
/*    KHE_AVOID_UNAVAILABLE_TIMES_MONITOR m, int i)                          */
/*                                                                           */
/*  Return a description of the i'th deviation of m.                         */
/*                                                                           */
/*****************************************************************************/

char *KheAvoidUnavailableTimesMonitorDeviationDescription(
  KHE_AVOID_UNAVAILABLE_TIMES_MONITOR m, int i)
{
  MAssert(i == 0,
    "KheAvoidUnavailableTimesMonitorDeviationDescription: i out of range");
  return NULL;
}


/*****************************************************************************/
/*                                                                           */
/*  Submodule "debug"                                                        */
/*                                                                           */
/*****************************************************************************/

/*****************************************************************************/
/*                                                                           */
/*  void KheAvoidUnavailableTimesMonitorDebug(                               */
/*    KHE_AVOID_UNAVAILABLE_TIMES_MONITOR m, int verbosity,                  */
/*    int indent, FILE *fp)                                                  */
/*                                                                           */
/*  Debug print of m onto fp with the given indent.                          */
/*                                                                           */
/*****************************************************************************/

void KheAvoidUnavailableTimesMonitorDebug(
  KHE_AVOID_UNAVAILABLE_TIMES_MONITOR m, int verbosity, int indent, FILE *fp)
{
  if( verbosity >= 1 )
  {
    KheMonitorDebugBegin((KHE_MONITOR) m, indent, fp);
    fprintf(fp, " ");
    KheResourceInSolnDebug(m->resource_in_soln, 1, -1, fp);
    KheMonitorDebugEnd((KHE_MONITOR) m, true, indent, fp);
  }
}
