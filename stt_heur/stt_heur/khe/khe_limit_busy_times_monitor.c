
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
/*  FILE:         khe_limit_busy_times_monitor.c                             */
/*  DESCRIPTION:  A time group monitor                                       */
/*                                                                           */
/*****************************************************************************/
#include "khe_interns.h"
#define DEBUG1 1
//I COMMENTED THIS CODE!!!
#define DEBUG1_RESOURCE ""
//#define DEBUG1_RESOURCE "T07"
#define DEBUG1_R(rs) ( DEBUG1 &&					\
  strcmp(KheResourceId(KheResourceInSolnResource(rs)), DEBUG1_RESOURCE)==0 )


/*****************************************************************************/
/*                                                                           */
/*  KHE_LIMIT_BUSY_TIMES_MONITOR - monitors busy times                       */
/*                                                                           */
/*****************************************************************************/

struct khe_limit_busy_times_monitor_rec {

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

  /* specific to KHE_LIMIT_BUSY_TIMES_MONITOR */
  KHE_RESOURCE_IN_SOLN		resource_in_soln;	/* enclosing rs      */
  KHE_LIMIT_BUSY_TIMES_CONSTRAINT constraint;		/* monitoring this   */
  int				minimum;		/* from constraint   */
  int				maximum;		/* from constraint   */
  bool				separate;		/* separate          */
  KHE_DEV_MONITOR		separate_dev_monitor;	/* separate devs     */
  int				total_devs;		/* total devs        */
  int				new_total_devs;		/* total devs        */
  KHE_LIMIT_BUSY_TIMES_MONITOR	copy;			/* used when copying */
};


/*****************************************************************************/
/*                                                                           */
/*  Submodule "construction and query"                                       */
/*                                                                           */
/*****************************************************************************/

/*****************************************************************************/
/*                                                                           */
/*  void KheLimitBusyTimesMonitorDebugCost(KHE_LIMIT_BUSY_TIMES_MONITOR m,   */
/*    FILE *fp)                                                              */
/*                                                                           */
/*  Debug print the cost of m onto fp.                                       */
/*                                                                           */
/*****************************************************************************/

static void KheLimitBusyTimesMonitorDebugCost(KHE_LIMIT_BUSY_TIMES_MONITOR m,
  FILE *fp)
{
  fprintf(fp, "cost (");
  if( m->separate )
    KheDevMonitorDebug(&m->separate_dev_monitor, fp);
  else
    fprintf(fp, "%d", m->total_devs);
  fprintf(fp, ") %.4f\n", KheCostShow(m->cost));
}


/*****************************************************************************/
/*                                                                           */
/*  KHE_LIMIT_BUSY_TIMES_MONITOR KheLimitBusyTimesMonitorMake(               */
/*    KHE_RESOURCE_IN_SOLN rs, KHE_LIMIT_BUSY_TIMES_CONSTRAINT c)            */
/*                                                                           */
/*  Make a new limit busy times monitor for rs.                              */
/*                                                                           */
/*****************************************************************************/

KHE_LIMIT_BUSY_TIMES_MONITOR KheLimitBusyTimesMonitorMake(
  KHE_RESOURCE_IN_SOLN rs, KHE_LIMIT_BUSY_TIMES_CONSTRAINT c)
{
  KHE_LIMIT_BUSY_TIMES_MONITOR res;  KHE_SOLN soln;
  soln = KheResourceInSolnSoln(rs);
  if( DEBUG1_R(rs) )
    fprintf(stderr, "[ KheLimitBusyTimesMonitorMake(%s, %s)\n",
      KheResourceId(KheResourceInSolnResource(rs)),
      KheConstraintId( (KHE_CONSTRAINT) c));
  MMake(res);
  KheMonitorInitCommonFields((KHE_MONITOR) res, soln,
    KHE_LIMIT_BUSY_TIMES_MONITOR_TAG);
  res->resource_in_soln = rs;
  res->constraint = c;
  res->minimum = KheLimitBusyTimesConstraintMinimum(c);
  res->maximum = KheLimitBusyTimesConstraintMaximum(c);
  res->separate =
    (KheConstraintCostFunction((KHE_CONSTRAINT) c) != KHE_SUM_COST_FUNCTION);
  KheDevMonitorInit(&res->separate_dev_monitor);
  res->total_devs = 0;
  res->new_total_devs = 0;
  res->copy = NULL;
  KheResourceInSolnAddMonitor(rs, (KHE_MONITOR) res);
  /* KheGroupMonitorAddMonitor((KHE_GROUP_MONITOR) soln, (KHE_MONITOR) res); */
  if( DEBUG1_R(rs) )
  {
    fprintf(stderr, "] KheLimitBusyTimesMonitorMake returning (%sseparate), ",
      res->separate ? "" : "!");
    KheLimitBusyTimesMonitorDebugCost(res, stderr);
  }
  return res;
}


/*****************************************************************************/
/*                                                                           */
/*  KHE_LIMIT_BUSY_TIMES_MONITOR KheLimitBusyTimesMonitorCopyPhase1(         */
/*    KHE_LIMIT_BUSY_TIMES_MONITOR m)                                        */
/*                                                                           */
/*  Carry out Phase 1 of the copying of m.                                   */
/*                                                                           */
/*****************************************************************************/

KHE_LIMIT_BUSY_TIMES_MONITOR KheLimitBusyTimesMonitorCopyPhase1(
  KHE_LIMIT_BUSY_TIMES_MONITOR m)
{
  KHE_LIMIT_BUSY_TIMES_MONITOR copy;
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
    copy->separate = m->separate;
    KheDevMonitorCopy(&copy->separate_dev_monitor, &m->separate_dev_monitor);
    copy->total_devs = m->total_devs;
    copy->new_total_devs = m->new_total_devs;
    copy->copy = NULL;
  }
  return m->copy;
}


/*****************************************************************************/
/*                                                                           */
/* void KheLimitBusyTimesMonitorCopyPhase2(KHE_LIMIT_BUSY_TIMES_MONITOR m)   */
/*                                                                           */
/*  Carry out Phase 2 of the copying of m.                                   */
/*                                                                           */
/*****************************************************************************/

void KheLimitBusyTimesMonitorCopyPhase2(KHE_LIMIT_BUSY_TIMES_MONITOR m)
{
  if( m->copy != NULL )
  {
    m->copy = NULL;
    KheResourceInSolnCopyPhase2(m->resource_in_soln);
  }
}


/*****************************************************************************/
/*                                                                           */
/*  void KheLimitBusyTimesMonitorDelete(KHE_LIMIT_BUSY_TIMES_MONITOR m)      */
/*                                                                           */
/*  Delete m.                                                                */
/*                                                                           */
/*****************************************************************************/

void KheLimitBusyTimesMonitorDelete(KHE_LIMIT_BUSY_TIMES_MONITOR m)
{
  if( DEBUG1_R(m->resource_in_soln) )
  {
    fprintf(stderr, "  KheLimitBusyTimesMonitorDelete(m) ");
    KheLimitBusyTimesMonitorDebugCost(m, stderr);
  }
  if( m->attached )
    KheLimitBusyTimesMonitorDetachFromSoln(m);
  if( m->parent_monitor != NULL )
    KheGroupMonitorDeleteChildMonitor(m->parent_monitor, (KHE_MONITOR) m);
  KheResourceInSolnDeleteMonitor(m->resource_in_soln, (KHE_MONITOR) m);
  KheSolnDeleteMonitor(m->soln, (KHE_MONITOR) m);
  KheDevMonitorFree(&m->separate_dev_monitor);
  MFree(m);
}


/*****************************************************************************/
/*                                                                           */
/*  KHE_RESOURCE_IN_SOLN KheLimitBusyTimesMonitorResourceInSoln(             */
/*    KHE_LIMIT_BUSY_TIMES_MONITOR m)                                        */
/*                                                                           */
/*  Return the resource monitor holding m.                                   */
/*                                                                           */
/*****************************************************************************/

KHE_RESOURCE_IN_SOLN KheLimitBusyTimesMonitorResourceInSoln(
  KHE_LIMIT_BUSY_TIMES_MONITOR m)
{
  return m->resource_in_soln;
}


/*****************************************************************************/
/*                                                                           */
/*  KHE_LIMIT_BUSY_TIMES_CONSTRAINT KheLimitBusyTimesMonitorConstraint(      */
/*    KHE_LIMIT_BUSY_TIMES_MONITOR m)                                        */
/*                                                                           */
/*  Return the contraint that m is monitoring.                               */
/*                                                                           */
/*****************************************************************************/

KHE_LIMIT_BUSY_TIMES_CONSTRAINT KheLimitBusyTimesMonitorConstraint(
  KHE_LIMIT_BUSY_TIMES_MONITOR m)
{
  return m->constraint;
}


/*****************************************************************************/
/*                                                                           */
/*  KHE_RESOURCE KheLimitBusyTimesMonitorResource(                           */
/*    KHE_LIMIT_BUSY_TIMES_MONITOR m)                                        */
/*                                                                           */
/*  Return the resource that m is monitoring.                                */
/*                                                                           */
/*****************************************************************************/

KHE_RESOURCE KheLimitBusyTimesMonitorResource(KHE_LIMIT_BUSY_TIMES_MONITOR m)
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
/*  void KheLimitBusyTimesMonitorAttachToSoln(KHE_LIMIT_BUSY_TIMES_MONITOR m)*/
/*                                                                           */
/*  Attach m.  It is known to be currently detached with cost 0.             */
/*                                                                           */
/*  Note: for cost 0, the number of busy times in each time group has        */
/*  to be either 0 or lie between m->minimum and m->maximum.  The first      */
/*  part of this condition means that the initial cost is always 0.          */
/*                                                                           */
/*****************************************************************************/

void KheLimitBusyTimesMonitorAttachToSoln(KHE_LIMIT_BUSY_TIMES_MONITOR m)
{
  if( DEBUG1_R(m->resource_in_soln) )
  {
    fprintf(stderr, "  KheLimitBusyTimesMonitorAttachToSoln(m) ");
    KheLimitBusyTimesMonitorDebugCost(m, stderr);
  }
  m->attached = true;
  KheResourceInSolnAttachMonitor(m->resource_in_soln, (KHE_MONITOR) m);
}


/*****************************************************************************/
/*                                                                           */
/*  void KheLimitBusyTimesMonitorDetachFromSoln(                             */
/*    KHE_LIMIT_BUSY_TIMES_MONITOR m)                                        */
/*                                                                           */
/*  Detach m.  It is known to be currently attached.                         */
/*                                                                           */
/*****************************************************************************/

void KheLimitBusyTimesMonitorDetachFromSoln(KHE_LIMIT_BUSY_TIMES_MONITOR m)
{
  KheResourceInSolnDetachMonitor(m->resource_in_soln, (KHE_MONITOR) m);
  m->attached = false;
  if( DEBUG1_R(m->resource_in_soln) )
  {
    fprintf(stderr, "  KheLimitBusyTimesMonitorDetachFromSoln(m) ");
    KheLimitBusyTimesMonitorDebugCost(m, stderr);
  }
}


/*****************************************************************************/
/*                                                                           */
/*  void KheLimitBusyTimesMonitorAttachCheck(KHE_LIMIT_BUSY_TIMES_MONITOR m) */
/*                                                                           */
/*  Check the attachment of m.                                               */
/*                                                                           */
/*****************************************************************************/

void KheLimitBusyTimesMonitorAttachCheck(KHE_LIMIT_BUSY_TIMES_MONITOR m)
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
/*  int KheLimitBusyTimesMonitorDev(                                         */
/*    KHE_LIMIT_BUSY_TIMES_MONITOR m, int busy_count)                        */
/*                                                                           */
/*  Work out the deviations caused by this busy_count.                       */
/*                                                                           */
/*****************************************************************************/

static int KheLimitBusyTimesMonitorDev(
  KHE_LIMIT_BUSY_TIMES_MONITOR m, int busy_count)
{
  if( busy_count == 0 )
    return 0;
  else if( busy_count < m->minimum )
    return m->minimum - busy_count;
  else if( busy_count > m->maximum )
    return busy_count - m->maximum;
  else
    return 0;
}


/*****************************************************************************/
/*                                                                           */
/*  void KheLimitBusyTimesMonitorFlush(KHE_LIMIT_BUSY_TIMES_MONITOR m)       */
/*                                                                           */
/*  Flush m, assuming that there has been a change in devs.                  */
/*                                                                           */
/*****************************************************************************/

static void KheLimitBusyTimesMonitorFlush(KHE_LIMIT_BUSY_TIMES_MONITOR m)
{
  if( DEBUG1_R(m->resource_in_soln) )
  {
    fprintf(stderr, "[ KheLimitBusyTimesMonitorFlush(m), init ");
    KheLimitBusyTimesMonitorDebugCost(m, stderr);
  }
  if( m->separate )
  {
    KheMonitorChangeCost((KHE_MONITOR) m,
      KheConstraintCostMulti((KHE_CONSTRAINT) m->constraint,
        KheDevMonitorDevs(&m->separate_dev_monitor)));
    KheDevMonitorFlush(&m->separate_dev_monitor);
  }
  else
  {
    KheMonitorChangeCost((KHE_MONITOR) m,
      KheConstraintCost((KHE_CONSTRAINT) m->constraint, m->new_total_devs));
    m->total_devs = m->new_total_devs;
  }
  if( DEBUG1_R(m->resource_in_soln) )
  {
    fprintf(stderr, "] KheLimitBusyTimesMonitorFlush(m), final ");
    KheLimitBusyTimesMonitorDebugCost(m, stderr);
  }
}


/*****************************************************************************/
/*                                                                           */
/*  void KheLimitBusyTimesMonitorAddBusyAndIdle(                             */
/*    KHE_LIMIT_BUSY_TIMES_MONITOR m, int busy_count, int idle_count)        */
/*                                                                           */
/*  Add an idle value for one monitored time group.                          */
/*                                                                           */
/*****************************************************************************/

void KheLimitBusyTimesMonitorAddBusyAndIdle(
  KHE_LIMIT_BUSY_TIMES_MONITOR m, int busy_count, int idle_count)
{
  int devs;
  if( DEBUG1_R(m->resource_in_soln) )
  {
    fprintf(stderr, "[ KheLimitBusyTimesMonitorAddBusyAndIdle(m, %d, %d), ",
      busy_count, idle_count);
    KheLimitBusyTimesMonitorDebugCost(m, stderr);
  }
  devs = KheLimitBusyTimesMonitorDev(m, busy_count);
  if( devs != 0 )
  {
    if( m->separate )
      KheDevMonitorAttach(&m->separate_dev_monitor, devs);
    else
      m->new_total_devs += devs;
    KheLimitBusyTimesMonitorFlush(m);
  }
  if( DEBUG1_R(m->resource_in_soln) )
  {
    fprintf(stderr, "] KheLimitBusyTimesMonitorAddBusyAndIdle returning, ");
    KheLimitBusyTimesMonitorDebugCost(m, stderr);
  }
}


/*****************************************************************************/
/*                                                                           */
/*  void KheLimitBusyTimesMonitorDeleteBusyAndIdle(                          */
/*    KHE_LIMIT_BUSY_TIMES_MONITOR m, int busy_count, int idle_count)        */
/*                                                                           */
/*  Remove the idle value of one monitored time group.                       */
/*                                                                           */
/*****************************************************************************/

void KheLimitBusyTimesMonitorDeleteBusyAndIdle(
  KHE_LIMIT_BUSY_TIMES_MONITOR m, int busy_count, int idle_count)
{
  int devs;
  if( DEBUG1_R(m->resource_in_soln) )
  {
    fprintf(stderr, "[ KheLimitBusyTimesMonitorDeleteBusyAndIdle(m, %d, %d) ",
      busy_count, idle_count);
    KheLimitBusyTimesMonitorDebugCost(m, stderr);
  }
  devs = KheLimitBusyTimesMonitorDev(m, busy_count);
  if( devs != 0 )
  {
    if( m->separate )
      KheDevMonitorDetach(&m->separate_dev_monitor, devs);
    else
      m->new_total_devs -= devs;
    KheLimitBusyTimesMonitorFlush(m);
  }
  if( DEBUG1_R(m->resource_in_soln) )
  {
    fprintf(stderr, "] KheLimitBusyTimesMonitorDeleteBusyAndIdle returning, ");
    KheLimitBusyTimesMonitorDebugCost(m, stderr);
  }
}


/*****************************************************************************/
/*                                                                           */
/*  void KheLimitBusyTimesMonitorChangeBusyAndIdle(                          */
/*    KHE_LIMIT_BUSY_TIMES_MONITOR m, int old_busy_count,                    */
/*    int new_busy_count, int old_idle_count, int new_idle_count)            */
/*                                                                           */
/*  Change the idle value of one monitored time group.                       */
/*                                                                           */
/*****************************************************************************/

void KheLimitBusyTimesMonitorChangeBusyAndIdle(
  KHE_LIMIT_BUSY_TIMES_MONITOR m, int old_busy_count,
  int new_busy_count, int old_idle_count, int new_idle_count)
{
  int old_devs, new_devs;
  if( DEBUG1_R(m->resource_in_soln) )
  {
    fprintf(stderr, "[ %s(m, %d, %d, %d, %d), ",
      "KheLimitBusyTimesMonitorChangeBusyAndIdle", old_busy_count,
      new_busy_count, old_idle_count, new_idle_count);
    KheLimitBusyTimesMonitorDebugCost(m, stderr);
  }
  old_devs = KheLimitBusyTimesMonitorDev(m, old_busy_count);
  new_devs = KheLimitBusyTimesMonitorDev(m, new_busy_count);
  if( old_devs != new_devs )
  {
    if( m->separate )
    {
      if( old_devs == 0 )
	KheDevMonitorAttach(&m->separate_dev_monitor, new_devs);
      else if( new_devs == 0 )
	KheDevMonitorDetach(&m->separate_dev_monitor, old_devs);
      else
	KheDevMonitorUpdate(&m->separate_dev_monitor, old_devs, new_devs);
    }
    else
      m->new_total_devs += (new_devs - old_devs);
    KheLimitBusyTimesMonitorFlush(m);
  }
  if( DEBUG1_R(m->resource_in_soln) )
  {
    fprintf(stderr, "] KheLimitBusyTimesMonitorChangeBusyAndIdle returning, ");
    KheLimitBusyTimesMonitorDebugCost(m, stderr);
  }
}


/*****************************************************************************/
/*                                                                           */
/*  Submodule "deviations"                                                   */
/*                                                                           */
/*****************************************************************************/

/*****************************************************************************/
/*                                                                           */
/* int KheLimitBusyTimesMonitorDeviationCount(KHE_LIMIT_BUSY_TIMES_MONITOR m)*/
/*                                                                           */
/*  Return the number of deviations of m.                                    */
/*                                                                           */
/*****************************************************************************/

int KheLimitBusyTimesMonitorDeviationCount(KHE_LIMIT_BUSY_TIMES_MONITOR m)
{
  return KheLimitBusyTimesConstraintTimeGroupCount(m->constraint);
}


/*****************************************************************************/
/*                                                                           */
/*  int KheLimitBusyTimesMonitorDeviation(                                   */
/*    KHE_LIMIT_BUSY_TIMES_MONITOR m, int i)                                 */
/*                                                                           */
/*  Return the i'th deviation of m.                                          */
/*                                                                           */
/*****************************************************************************/

int KheLimitBusyTimesMonitorDeviation(KHE_LIMIT_BUSY_TIMES_MONITOR m, int i)
{
  KHE_TIMETABLE_MONITOR tt;  KHE_RESOURCE r;  KHE_TIME_GROUP tg;
  KHE_TIME_GROUP_MONITOR tgm;  int busy_count;
  r = KheResourceInSolnResource(m->resource_in_soln);
  tt = KheResourceTimetableMonitor(m->soln, r);
  if( !KheMonitorAttachedToSoln((KHE_MONITOR) tt) )
    KheMonitorAttachToSoln((KHE_MONITOR) tt);
  tg = KheLimitBusyTimesConstraintTimeGroup(m->constraint, i);
  if( !KheTimetableMonitorContainsTimeGroupMonitor(tt, tg, &tgm) )
    MAssert(false, "KheLimitBusyTimesMonitorDeviation internal error");
  busy_count = KheTimeGroupMonitorBusyTimes(tgm);
  return KheLimitBusyTimesMonitorDev(m, busy_count);
}


/*****************************************************************************/
/*                                                                           */
/*  char *KheLimitBusyTimesMonitorDeviationDescription(                      */
/*    KHE_LIMIT_BUSY_TIMES_MONITOR m, int i)                                 */
/*                                                                           */
/*  Return a description of the i'th deviation of m.                         */
/*                                                                           */
/*****************************************************************************/

char *KheLimitBusyTimesMonitorDeviationDescription(
  KHE_LIMIT_BUSY_TIMES_MONITOR m, int i)
{
  KHE_TIME_GROUP tg;
  tg = KheLimitBusyTimesConstraintTimeGroup(m->constraint, i);
  return KheTimeGroupName(tg);
}


/*****************************************************************************/
/*                                                                           */
/*  Submodule "debug"                                                        */
/*                                                                           */
/*****************************************************************************/

/*****************************************************************************/
/*                                                                           */
/*  void KheLimitBusyTimesMonitorDebug(KHE_LIMIT_BUSY_TIMES_MONITOR m,       */
/*    int verbosity, int indent, FILE *fp)                                   */
/*                                                                           */
/*  Debug print of m onto fp with the given verbosity and indent.            */
/*                                                                           */
/*****************************************************************************/

void KheLimitBusyTimesMonitorDebug(KHE_LIMIT_BUSY_TIMES_MONITOR m,
  int verbosity, int indent, FILE *fp)
{
  if( verbosity >= 1 )
  {
    KheMonitorDebugBegin((KHE_MONITOR) m, indent, fp);
    fprintf(fp, " %s (min %d, max %d)%s ",
      KheResourceInSolnId(m->resource_in_soln),
      m->minimum, m->maximum, m->separate ? " sep" : "");
    if( m->separate )
      KheDevMonitorDebug(&m->separate_dev_monitor, fp);
    else
      fprintf(fp, "%s%d", m->total_devs != m->new_total_devs ? "*" : "",
	m->total_devs);
    KheMonitorDebugEnd((KHE_MONITOR) m, true, indent, fp);
  }
}
