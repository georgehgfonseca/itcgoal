
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
/*  FILE:         khe_limit_idle_times_monitor.c                             */
/*  DESCRIPTION:  A limit idle times monitor                                 */
/*                                                                           */
/*****************************************************************************/
#include "khe_interns.h"


/*****************************************************************************/
/*                                                                           */
/*  KHE_LIMIT_IDLE_TIMES_MONITOR - monitors idle times                       */
/*                                                                           */
/*****************************************************************************/

struct khe_limit_idle_times_monitor_rec {

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

  /* specific to KHE_LIMIT_IDLE_TIMES_MONITOR */
  KHE_RESOURCE_IN_SOLN		resource_in_soln;	/* enclosing rs      */
  KHE_LIMIT_IDLE_TIMES_CONSTRAINT constraint;		/* monitoring this   */
  int				minimum;		/* from constraint   */
  int				maximum;		/* from constraint   */
  int				total_idle_count;	/* total idle times  */
  int				new_total_idle_count;	/* new total idle    */
  KHE_LIMIT_IDLE_TIMES_MONITOR	copy;
};


/*****************************************************************************/
/*                                                                           */
/*  Submodule "construction and query"                                       */
/*                                                                           */
/*****************************************************************************/

/*****************************************************************************/
/*                                                                           */
/*  KHE_LIMIT_IDLE_TIMES_MONITOR KheLimitIdleTimesMonitorMake(               */
/*    KHE_RESOURCE_IN_SOLN rs, KHE_LIMIT_IDLE_TIMES_CONSTRAINT c)            */
/*                                                                           */
/*  Make a new limit idle times monitor for rs.                              */
/*                                                                           */
/*****************************************************************************/

KHE_LIMIT_IDLE_TIMES_MONITOR KheLimitIdleTimesMonitorMake(
  KHE_RESOURCE_IN_SOLN rs, KHE_LIMIT_IDLE_TIMES_CONSTRAINT c)
{
  KHE_LIMIT_IDLE_TIMES_MONITOR res;  KHE_SOLN soln;
  soln = KheResourceInSolnSoln(rs);
  MMake(res);
  KheMonitorInitCommonFields((KHE_MONITOR) res, soln,
    KHE_LIMIT_IDLE_TIMES_MONITOR_TAG);
  res->resource_in_soln = rs;
  res->constraint = c;
  res->minimum = KheLimitIdleTimesConstraintMinimum(c);
  res->maximum = KheLimitIdleTimesConstraintMaximum(c);
  res->total_idle_count = 0;
  res->new_total_idle_count = 0;
  res->copy = NULL;
  KheResourceInSolnAddMonitor(rs, (KHE_MONITOR) res);
  /* KheGroupMonitorAddMonitor((KHE_GROUP_MONITOR) soln, (KHE_MONITOR) res); */
  return res;
}


/*****************************************************************************/
/*                                                                           */
/*  KHE_LIMIT_IDLE_TIMES_MONITOR KheLimitIdleTimesMonitorCopyPhase1(         */
/*    KHE_LIMIT_IDLE_TIMES_MONITOR m)                                        */
/*                                                                           */
/*  Carry out Phase 1 of the copying of m.                                   */
/*                                                                           */
/*****************************************************************************/

KHE_LIMIT_IDLE_TIMES_MONITOR KheLimitIdleTimesMonitorCopyPhase1(
  KHE_LIMIT_IDLE_TIMES_MONITOR m)
{
  KHE_LIMIT_IDLE_TIMES_MONITOR copy;
  if( m->copy == NULL )
  {
    MMake(copy);
    m->copy = copy;
    KheMonitorCopyCommonFields((KHE_MONITOR) copy, (KHE_MONITOR) m);
    copy->resource_in_soln =
      KheResourceInSolnCopyPhase1(m->resource_in_soln);
    copy->constraint = m->constraint;
    copy->minimum = m->minimum;
    copy->maximum = m->maximum;
    copy->total_idle_count = m->total_idle_count;
    copy->new_total_idle_count = m->new_total_idle_count;
    copy->copy = NULL;
  }
  return m->copy;
}


/*****************************************************************************/
/*                                                                           */
/*  void KheLimitIdleTimesMonitorCopyPhase2(KHE_LIMIT_IDLE_TIMES_MONITOR m)  */
/*                                                                           */
/*  Carry out Phase 2 of the copying of m.                                   */
/*                                                                           */
/*****************************************************************************/

void KheLimitIdleTimesMonitorCopyPhase2(KHE_LIMIT_IDLE_TIMES_MONITOR m)
{
  if( m->copy != NULL )
  {
    m->copy = NULL;
    KheResourceInSolnCopyPhase2(m->resource_in_soln);
  }
}


/*****************************************************************************/
/*                                                                           */
/*  void KheLimitIdleTimesMonitorDelete(KHE_LIMIT_IDLE_TIMES_MONITOR m)      */
/*                                                                           */
/*  Delete m.                                                                */
/*                                                                           */
/*****************************************************************************/

void KheLimitIdleTimesMonitorDelete(KHE_LIMIT_IDLE_TIMES_MONITOR m)
{
  if( m->attached )
    KheLimitIdleTimesMonitorDetachFromSoln(m);
  if( m->parent_monitor != NULL )
    KheGroupMonitorDeleteChildMonitor(m->parent_monitor, (KHE_MONITOR) m);
  KheResourceInSolnDeleteMonitor(m->resource_in_soln, (KHE_MONITOR) m);
  KheSolnDeleteMonitor(m->soln, (KHE_MONITOR) m);
  MFree(m);
}


/*****************************************************************************/
/*                                                                           */
/*  KHE_RESOURCE_IN_SOLN KheLimitIdleTimesMonitorResourceInSoln(             */
/*    KHE_LIMIT_IDLE_TIMES_MONITOR m)                                        */
/*                                                                           */
/*  Return the resource monitor holding m.                                   */
/*                                                                           */
/*****************************************************************************/

KHE_RESOURCE_IN_SOLN KheLimitIdleTimesMonitorResourceInSoln(
  KHE_LIMIT_IDLE_TIMES_MONITOR m)
{
  return m->resource_in_soln;
}


/*****************************************************************************/
/*                                                                           */
/*  KHE_LIMIT_IDLE_TIMES_CONSTRAINT KheLimitIdleTimesMonitorConstraint(      */
/*    KHE_LIMIT_IDLE_TIMES_MONITOR m)                                        */
/*                                                                           */
/*  Return the contraint that m is monitoring.                               */
/*                                                                           */
/*****************************************************************************/

KHE_LIMIT_IDLE_TIMES_CONSTRAINT KheLimitIdleTimesMonitorConstraint(
  KHE_LIMIT_IDLE_TIMES_MONITOR m)
{
  return m->constraint;
}


/*****************************************************************************/
/*                                                                           */
/*  KHE_RESOURCE KheLimitIdleTimesMonitorResource(                           */
/*    KHE_LIMIT_IDLE_TIMES_MONITOR m)                                        */
/*                                                                           */
/*  Return the resource that m is monitoring.                                */
/*                                                                           */
/*****************************************************************************/

KHE_RESOURCE KheLimitIdleTimesMonitorResource(KHE_LIMIT_IDLE_TIMES_MONITOR m)
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
/*  void KheLimitIdleTimesMonitorAttachToSoln(KHE_LIMIT_IDLE_TIMES_MONITOR m)*/
/*                                                                           */
/*  Attach m.  It is known to be currently detached with cost 0.             */
/*                                                                           */
/*****************************************************************************/

void KheLimitIdleTimesMonitorAttachToSoln(KHE_LIMIT_IDLE_TIMES_MONITOR m)
{
  m->attached = true;
  KheResourceInSolnAttachMonitor(m->resource_in_soln, (KHE_MONITOR) m);
}


/*****************************************************************************/
/*                                                                           */
/*  void KheLimitIdleTimesMonitorDetachFromSoln(                             */
/*    KHE_LIMIT_IDLE_TIMES_MONITOR m)                                        */
/*                                                                           */
/*  Detach m.  It is known to be currently attached.                         */
/*                                                                           */
/*****************************************************************************/

void KheLimitIdleTimesMonitorDetachFromSoln(KHE_LIMIT_IDLE_TIMES_MONITOR m)
{
  KheResourceInSolnDetachMonitor(m->resource_in_soln, (KHE_MONITOR) m);
  m->attached = false;
}


/*****************************************************************************/
/*                                                                           */
/*  void KheLimitIdleTimesMonitorAttachCheck(KHE_LIMIT_IDLE_TIMES_MONITOR m) */
/*                                                                           */
/*  Check the attachment of m.                                               */
/*                                                                           */
/*****************************************************************************/

void KheLimitIdleTimesMonitorAttachCheck(KHE_LIMIT_IDLE_TIMES_MONITOR m)
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
/*  int KheLimitIdleTimesMonitorDev(KHE_LIMIT_IDLE_TIMES_MONITOR m,          */
/*      int idle_count)                                                      */
/*                                                                           */
/*  Work out the deviations caused by this many idle times.                  */
/*                                                                           */
/*****************************************************************************/

static int KheLimitIdleTimesMonitorDev(KHE_LIMIT_IDLE_TIMES_MONITOR m,
    int idle_count)
{
  if( idle_count < m->minimum )
    return m->minimum - idle_count;
  else if( idle_count > m->maximum )
    return idle_count - m->maximum;
  else
    return 0;
}


/*****************************************************************************/
/*                                                                           */
/*  void KheLimitIdleTimesMonitorFlush(KHE_LIMIT_IDLE_TIMES_MONITOR m)       */
/*                                                                           */
/*  Flush m.                                                                 */
/*                                                                           */
/*****************************************************************************/

static void KheLimitIdleTimesMonitorFlush(KHE_LIMIT_IDLE_TIMES_MONITOR m)
{
  int old_devs, new_devs;
  if( m->new_total_idle_count != m->total_idle_count )
  {
    old_devs = KheLimitIdleTimesMonitorDev(m, m->total_idle_count);
    new_devs = KheLimitIdleTimesMonitorDev(m, m->new_total_idle_count);
    if( old_devs != new_devs )
      KheMonitorChangeCost((KHE_MONITOR) m,
        KheConstraintCost((KHE_CONSTRAINT) m->constraint, new_devs));
    m->total_idle_count = m->new_total_idle_count;
  }
}


/*****************************************************************************/
/*                                                                           */
/*  void KheLimitIdleTimesMonitorAddBusyAndIdle(                             */
/*    KHE_LIMIT_IDLE_TIMES_MONITOR m, int busy_count, int idle_count)        */
/*                                                                           */
/*  Add an idle value for one monitored time group.                          */
/*                                                                           */
/*****************************************************************************/

void KheLimitIdleTimesMonitorAddBusyAndIdle(
  KHE_LIMIT_IDLE_TIMES_MONITOR m, int busy_count, int idle_count)
{
  if( idle_count != 0 )
  {
    m->new_total_idle_count += idle_count;
    KheLimitIdleTimesMonitorFlush(m);
  }
}


/*****************************************************************************/
/*                                                                           */
/*  void KheLimitIdleTimesMonitorDeleteBusyAndIdle(                          */
/*    KHE_LIMIT_IDLE_TIMES_MONITOR m, int busy_count, int idle_count)        */
/*                                                                           */
/*  Remove the idle value of one monitored time group.                       */
/*                                                                           */
/*****************************************************************************/

void KheLimitIdleTimesMonitorDeleteBusyAndIdle(
  KHE_LIMIT_IDLE_TIMES_MONITOR m, int busy_count, int idle_count)
{
  if( idle_count != 0 )
  {
    m->new_total_idle_count -= idle_count;
    KheLimitIdleTimesMonitorFlush(m);
  }
}


/*****************************************************************************/
/*                                                                           */
/*  void KheLimitIdleTimesMonitorChangeBusyAndIdle(                          */
/*    KHE_LIMIT_IDLE_TIMES_MONITOR m, int old_busy_count,                    */
/*    int new_busy_count, int old_idle_count, int new_idle_count)            */
/*                                                                           */
/*  Change the idle value of one monitored time group.                       */
/*                                                                           */
/*****************************************************************************/

void KheLimitIdleTimesMonitorChangeBusyAndIdle(
  KHE_LIMIT_IDLE_TIMES_MONITOR m, int old_busy_count,
  int new_busy_count, int old_idle_count, int new_idle_count)
{
  if( new_idle_count != old_idle_count )
  {
    m->new_total_idle_count += (new_idle_count - old_idle_count);
    KheLimitIdleTimesMonitorFlush(m);
  }
}


/*****************************************************************************/
/*                                                                           */
/*  Submodule "deviations"                                                   */
/*                                                                           */
/*****************************************************************************/

/*****************************************************************************/
/*                                                                           */
/* int KheLimitIdleTimesMonitorDeviationCount(KHE_LIMIT_IDLE_TIMES_MONITOR m)*/
/*                                                                           */
/*  Return the number of deviations of m.                                    */
/*                                                                           */
/*****************************************************************************/

int KheLimitIdleTimesMonitorDeviationCount(KHE_LIMIT_IDLE_TIMES_MONITOR m)
{
  return 1;
}


/*****************************************************************************/
/*                                                                           */
/*  int KheLimitIdleTimesMonitorDeviation(KHE_LIMIT_IDLE_TIMES_MONITOR m,    */
/*    int i)                                                                 */
/*                                                                           */
/*  Return the i'th deviation of m.                                          */
/*                                                                           */
/*****************************************************************************/

int KheLimitIdleTimesMonitorDeviation(KHE_LIMIT_IDLE_TIMES_MONITOR m, int i)
{
  MAssert(i == 0, "KheLimitIdleTimesMonitorDeviation: i out of range");
  return KheLimitIdleTimesMonitorDev(m, m->total_idle_count);
}


/*****************************************************************************/
/*                                                                           */
/*  char *KheLimitIdleTimesMonitorDeviationDescription(                      */
/*    KHE_LIMIT_IDLE_TIMES_MONITOR m, int i)                                 */
/*                                                                           */
/*  Return a description of the i'th deviation of m.                         */
/*                                                                           */
/*****************************************************************************/

char *KheLimitIdleTimesMonitorDeviationDescription(
  KHE_LIMIT_IDLE_TIMES_MONITOR m, int i)
{
  MAssert(i == 0,
    "KheLimitIdleTimesMonitorDeviationDescription: i out of range");
  return NULL;
}


/*****************************************************************************/
/*                                                                           */
/*  Submodule "debug"                                                        */
/*                                                                           */
/*****************************************************************************/

/*****************************************************************************/
/*                                                                           */
/*  void KheLimitIdleTimesMonitorDebug(KHE_LIMIT_IDLE_TIMES_MONITOR m,       */
/*    int verbosity, int indent, FILE *fp)                                   */
/*                                                                           */
/*  Debug print of m onto fp at the given indent.                            */
/*                                                                           */
/*****************************************************************************/

void KheLimitIdleTimesMonitorDebug(KHE_LIMIT_IDLE_TIMES_MONITOR m,
  int verbosity, int indent, FILE *fp)
{
  if( verbosity >= 1 )
  {
    KheMonitorDebugBegin((KHE_MONITOR) m, indent, fp);
    fprintf(fp, " (min %d, max %d, idle %d)", m->minimum, m->maximum,
      m->total_idle_count);
    KheMonitorDebugEnd((KHE_MONITOR) m, true, indent, fp);
  }
}
