
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
/*  FILE:         khe_cluster_busy_times_monitor.c                           */
/*  DESCRIPTION:  A cluster busy times monitor                               */
/*                                                                           */
/*****************************************************************************/
#include "khe_interns.h"


/*****************************************************************************/
/*                                                                           */
/*  KHE_CLUSTER_BUSY_TIMES_MONITOR - monitors busy times                     */
/*                                                                           */
/*****************************************************************************/

struct khe_cluster_busy_times_monitor_rec {

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

  /* specific to KHE_SPLIT_EVENTS_MONITOR */
  KHE_RESOURCE_IN_SOLN		resource_in_soln;	/* enclosing rs      */
  KHE_CLUSTER_BUSY_TIMES_CONSTRAINT constraint;		/* monitoring this   */
  int				minimum;		/* minimum           */
  int				maximum;		/* maximum           */
  int				deviation;		/* last reported     */
  int				busy_group_count;	/* busy groups       */
  KHE_CLUSTER_BUSY_TIMES_MONITOR copy;			/* used when copying */
};


/*****************************************************************************/
/*                                                                           */
/*  Submodule "construction and query"                                       */
/*                                                                           */
/*****************************************************************************/

/*****************************************************************************/
/*                                                                           */
/*  KHE_CLUSTER_BUSY_TIMES_MONITOR KheClusterBusyTimesMonitorMake(           */
/*    KHE_RESOURCE_IN_SOLN rs, KHE_CLUSTER_BUSY_TIMES_CONSTRAINT c)          */
/*                                                                           */
/*  Make a new cluster busy times monitor for rs, monitoring c.              */
/*                                                                           */
/*****************************************************************************/

KHE_CLUSTER_BUSY_TIMES_MONITOR KheClusterBusyTimesMonitorMake(
  KHE_RESOURCE_IN_SOLN rs, KHE_CLUSTER_BUSY_TIMES_CONSTRAINT c)
{
  KHE_CLUSTER_BUSY_TIMES_MONITOR res;   KHE_SOLN soln;
  MMake(res);
  soln = KheResourceInSolnSoln(rs);
  KheMonitorInitCommonFields((KHE_MONITOR) res, soln,
    KHE_CLUSTER_BUSY_TIMES_MONITOR_TAG);
  res->resource_in_soln = rs;
  res->constraint = c;
  res->minimum = KheClusterBusyTimesConstraintMinimum(c);
  res->maximum = KheClusterBusyTimesConstraintMaximum(c);
  res->deviation = res->minimum;
  res->busy_group_count = 0;
  res->copy = NULL;
  KheResourceInSolnAddMonitor(rs, (KHE_MONITOR) res);
  return res;
}


/*****************************************************************************/
/*                                                                           */
/*  KHE_CLUSTER_BUSY_TIMES_MONITOR KheClusterBusyTimesMonitorCopyPhase1(     */
/*    KHE_CLUSTER_BUSY_TIMES_MONITOR m)                                      */
/*                                                                           */
/*  Carry out Phase 1 of the copying of m.                                   */
/*                                                                           */
/*****************************************************************************/

KHE_CLUSTER_BUSY_TIMES_MONITOR KheClusterBusyTimesMonitorCopyPhase1(
  KHE_CLUSTER_BUSY_TIMES_MONITOR m)
{
  KHE_CLUSTER_BUSY_TIMES_MONITOR copy;
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
    copy->deviation = m->deviation;
    copy->busy_group_count = m->busy_group_count;
    copy->copy = NULL;
  }
  return m->copy;
}


/*****************************************************************************/
/*                                                                           */
/*  void KheClusterBusyTimesMonitorCopyPhase2(                               */
/*    KHE_CLUSTER_BUSY_TIMES_MONITOR m)                                      */
/*                                                                           */
/*  Carry out Phase 2 of the copying of m.                                   */
/*                                                                           */
/*****************************************************************************/

void KheClusterBusyTimesMonitorCopyPhase2(KHE_CLUSTER_BUSY_TIMES_MONITOR m)
{
  if( m->copy != NULL )
  {
    m->copy = NULL;
    KheResourceInSolnCopyPhase2(m->resource_in_soln);
  }
}


/*****************************************************************************/
/*                                                                           */
/*  void KheClusterBusyTimesMonitorDelete(KHE_CLUSTER_BUSY_TIMES_MONITOR m)  */
/*                                                                           */
/*  Delete m.                                                                */
/*                                                                           */
/*****************************************************************************/

void KheClusterBusyTimesMonitorDelete(KHE_CLUSTER_BUSY_TIMES_MONITOR m)
{
  if( m->attached )
    KheClusterBusyTimesMonitorDetachFromSoln(m);
  if( m->parent_monitor != NULL )
    KheGroupMonitorDeleteChildMonitor(m->parent_monitor, (KHE_MONITOR) m);
  KheResourceInSolnDeleteMonitor(m->resource_in_soln, (KHE_MONITOR) m);
  KheSolnDeleteMonitor(m->soln, (KHE_MONITOR) m);
  MFree(m);
}


/*****************************************************************************/
/*                                                                           */
/*  KHE_RESOURCE_IN_SOLN KheClusterBusyTimesMonitorResourceInSoln(           */
/*    KHE_CLUSTER_BUSY_TIMES_MONITOR m)                                      */
/*                                                                           */
/*  Return the resource monitor holding m.                                   */
/*                                                                           */
/*****************************************************************************/

KHE_RESOURCE_IN_SOLN KheClusterBusyTimesMonitorResourceInSoln(
  KHE_CLUSTER_BUSY_TIMES_MONITOR m)
{
  return m->resource_in_soln;
}


/*****************************************************************************/
/*                                                                           */
/*  KHE_CLUSTER_BUSY_TIMES_CONSTRAINT KheClusterBusyTimesMonitorConstraint(  */
/*    KHE_CLUSTER_BUSY_TIMES_MONITOR m)                                      */
/*                                                                           */
/*  Return the contraint that m is monitoring.                               */
/*                                                                           */
/*****************************************************************************/

KHE_CLUSTER_BUSY_TIMES_CONSTRAINT KheClusterBusyTimesMonitorConstraint(
  KHE_CLUSTER_BUSY_TIMES_MONITOR m)
{
  return m->constraint;
}


/*****************************************************************************/
/*                                                                           */
/*  KHE_RESOURCE KheClusterBusyTimesMonitorResource(                         */
/*    KHE_CLUSTER_BUSY_TIMES_MONITOR m)                                      */
/*                                                                           */
/*  Return the resource that m is monitoring.                                */
/*                                                                           */
/*****************************************************************************/

KHE_RESOURCE KheClusterBusyTimesMonitorResource(
  KHE_CLUSTER_BUSY_TIMES_MONITOR m)
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
/*  void KheClusterBusyTimesMonitorAttachToSoln(                             */
/*    KHE_CLUSTER_BUSY_TIMES_MONITOR m)                                      */
/*                                                                           */
/*  Attach m.  It is known to be currently detached with cost 0.             */
/*                                                                           */
/*****************************************************************************/

void KheClusterBusyTimesMonitorAttachToSoln(KHE_CLUSTER_BUSY_TIMES_MONITOR m)
{
  m->attached = true;
  MAssert(m->deviation == m->minimum,
    "KheClusterBusyTimesMonitorAttachToSoln internal error 1");
  MAssert(m->busy_group_count == 0,
    "KheClusterBusyTimesMonitorAttachToSoln internal error 2");
  if( m->deviation != 0 )
    KheMonitorChangeCost((KHE_MONITOR) m,
      KheConstraintCost((KHE_CONSTRAINT) m->constraint, m->deviation));
  KheResourceInSolnAttachMonitor(m->resource_in_soln, (KHE_MONITOR) m);
}


/*****************************************************************************/
/*                                                                           */
/*  void KheClusterBusyTimesMonitorDetachFromSoln(                           */
/*    KHE_CLUSTER_BUSY_TIMES_MONITOR m)                                      */
/*                                                                           */
/*  Detach m.  It is known to be currently attached.                         */
/*                                                                           */
/*****************************************************************************/

void KheClusterBusyTimesMonitorDetachFromSoln(KHE_CLUSTER_BUSY_TIMES_MONITOR m)
{
  KheResourceInSolnDetachMonitor(m->resource_in_soln, (KHE_MONITOR) m);
  MAssert(m->deviation == m->minimum,
    "KheClusterBusyTimesMonitorDetach internal error 1");
  MAssert(m->busy_group_count == 0,
    "KheClusterBusyTimesMonitorDetach internal error 2");
  KheMonitorChangeCost((KHE_MONITOR) m, 0);
  m->attached = false;
}


/*****************************************************************************/
/*                                                                           */
/*  void KheClusterBusyTimesMonitorAttachCheck(                              */
/*    KHE_CLUSTER_BUSY_TIMES_MONITOR m)                                      */
/*                                                                           */
/*  Check the attachment of m.                                               */
/*                                                                           */
/*****************************************************************************/

void KheClusterBusyTimesMonitorAttachCheck(KHE_CLUSTER_BUSY_TIMES_MONITOR m)
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
/*  int KheClusterBusyTimesDev(KHE_CLUSTER_BUSY_TIMES_MONITOR m,             */
/*    int busy_groups)                                                       */
/*                                                                           */
/*  Convert a number of busy groups into a number of deviations.             */
/*                                                                           */
/*****************************************************************************/

int KheClusterBusyTimesDev(KHE_CLUSTER_BUSY_TIMES_MONITOR m,
  int busy_groups)
{
  if( busy_groups < m->minimum )
    return m->minimum - busy_groups;
  else if( busy_groups > m->maximum )
    return busy_groups - m->maximum;
  else
    return 0;
}


/*****************************************************************************/
/*                                                                           */
/*  void KheClusterBusyTimesMonitorFlush(KHE_CLUSTER_BUSY_TIMES_MONITOR m)   */
/*                                                                           */
/*  Flush m.                                                                 */
/*                                                                           */
/*****************************************************************************/

static void KheClusterBusyTimesMonitorFlush(KHE_CLUSTER_BUSY_TIMES_MONITOR m)
{
  int new_deviation;
  new_deviation = KheClusterBusyTimesDev(m, m->busy_group_count);
  if( m->deviation != new_deviation )
  {
    KheMonitorChangeCost((KHE_MONITOR) m,
      KheConstraintCost((KHE_CONSTRAINT) m->constraint, new_deviation));
    m->deviation = new_deviation;
  }
}


/*****************************************************************************/
/*                                                                           */
/*  void KheClusterBusyTimesMonitorAddBusyAndIdle(                           */
/*    KHE_CLUSTER_BUSY_TIMES_MONITOR m, int busy_count, int idle_count)      */
/*                                                                           */
/*  Add a busy value for one monitored time group.                           */
/*                                                                           */
/*****************************************************************************/

void KheClusterBusyTimesMonitorAddBusyAndIdle(
  KHE_CLUSTER_BUSY_TIMES_MONITOR m, int busy_count, int idle_count)
{
  if( busy_count > 0 )
  {
    m->busy_group_count++;
    KheClusterBusyTimesMonitorFlush(m);
  }
}


/*****************************************************************************/
/*                                                                           */
/*  void KheClusterBusyTimesMonitorDeleteBusyAndIdle(                        */
/*    KHE_CLUSTER_BUSY_TIMES_MONITOR m, int busy_count, int idle_count)      */
/*                                                                           */
/*  Remove the idle value of one monitored time group.                       */
/*                                                                           */
/*****************************************************************************/

void KheClusterBusyTimesMonitorDeleteBusyAndIdle(
  KHE_CLUSTER_BUSY_TIMES_MONITOR m, int busy_count, int idle_count)
{
  if( busy_count > 0 )
  {
    m->busy_group_count--;
    KheClusterBusyTimesMonitorFlush(m);
  }
}


/*****************************************************************************/
/*                                                                           */
/*  void KheClusterBusyTimesMonitorChangeBusyAndIdle(                        */
/*    KHE_CLUSTER_BUSY_TIMES_MONITOR m, int old_busy_count,                  */
/*    int new_busy_count, int old_idle_count, int new_idle_count)            */
/*                                                                           */
/*  Change the value of one monitored time group.                            */
/*                                                                           */
/*****************************************************************************/

void KheClusterBusyTimesMonitorChangeBusyAndIdle(
  KHE_CLUSTER_BUSY_TIMES_MONITOR m, int old_busy_count,
  int new_busy_count, int old_idle_count, int new_idle_count)
{
  if( old_busy_count > 0 )
    m->busy_group_count--;
  if( new_busy_count > 0 )
    m->busy_group_count++;
  KheClusterBusyTimesMonitorFlush(m);
}


/*****************************************************************************/
/*                                                                           */
/*  Submodule "deviations"                                                   */
/*                                                                           */
/*****************************************************************************/

/*****************************************************************************/
/*                                                                           */
/*  int KheClusterBusyTimesMonitorDeviationCount(                            */
/*    KHE_CLUSTER_BUSY_TIMES_MONITOR m)                                      */
/*                                                                           */
/*  Return the number of deviations of m.                                    */
/*                                                                           */
/*****************************************************************************/

int KheClusterBusyTimesMonitorDeviationCount(KHE_CLUSTER_BUSY_TIMES_MONITOR m)
{
  return 1;
}


/*****************************************************************************/
/*                                                                           */
/*  int KheClusterBusyTimesMonitorDeviation(                                 */
/*    KHE_CLUSTER_BUSY_TIMES_MONITOR m, int i)                               */
/*                                                                           */
/*  Return the i'th deviation of m.                                          */
/*                                                                           */
/*****************************************************************************/

int KheClusterBusyTimesMonitorDeviation(KHE_CLUSTER_BUSY_TIMES_MONITOR m,
  int i)
{
  MAssert(i == 0, "KheClusterBusyTimesMonitorDeviation: i out of range");
  return m->deviation;
}


/*****************************************************************************/
/*                                                                           */
/*  char *KheClusterBusyTimesMonitorDeviationDescription(                    */
/*    KHE_CLUSTER_BUSY_TIMES_MONITOR m, int i)                               */
/*                                                                           */
/*  Return a description of the i'th deviation of m.                         */
/*                                                                           */
/*****************************************************************************/

char *KheClusterBusyTimesMonitorDeviationDescription(
  KHE_CLUSTER_BUSY_TIMES_MONITOR m, int i)
{
  MAssert(i == 0,
    "KheClusterBusyTimesMonitorDeviationDescription: i out of range");
  return NULL;
}


/*****************************************************************************/
/*                                                                           */
/*  Submodule "debug"                                                        */
/*                                                                           */
/*****************************************************************************/

/*****************************************************************************/
/*                                                                           */
/*  void KheClusterBusyTimesMonitorDebug(KHE_CLUSTER_BUSY_TIMES_MONITOR m,   */
/*    int verbosity, int indent, FILE *fp)                                   */
/*                                                                           */
/*  Debug print of m onto fp with the given verbosity and indent.            */
/*                                                                           */
/*****************************************************************************/

void KheClusterBusyTimesMonitorDebug(KHE_CLUSTER_BUSY_TIMES_MONITOR m,
  int verbosity, int indent, FILE *fp)
{
  if( verbosity >= 1 )
  {
    KheMonitorDebugBegin((KHE_MONITOR) m, indent, fp);
    fprintf(fp, "(min %d, max %d, count %d)", m->minimum, m->maximum,
      m->busy_group_count);
    KheMonitorDebugEnd((KHE_MONITOR) m, true, indent, fp);
  }
}
