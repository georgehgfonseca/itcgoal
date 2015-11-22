
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
/*  FILE:         khe_avoid_clashes_monitor.c                                */
/*  DESCRIPTION:  An avoid clashes monitor                                   */
/*                                                                           */
/*****************************************************************************/
#include "khe_interns.h"

#define DEBUG1 0

/*****************************************************************************/
/*                                                                           */
/*  KHE_AVOID_CLASHES_MONITOR                                                */
/*                                                                           */
/*****************************************************************************/

struct khe_avoid_clashes_monitor_rec {

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

  /* specific to KHE_AVOID_CLASHES_MONITOR */
  KHE_RESOURCE_IN_SOLN		resource_in_soln;	/* monitored resource*/
  KHE_AVOID_CLASHES_CONSTRAINT	constraint;		/* constraint        */
  bool				separate;		/* separate          */
  KHE_DEV_MONITOR		separate_dev_monitor;	/* separate devs     */
  int				total_devs;		/* total devs        */
  int				new_total_devs;		/* total devs        */
  KHE_AVOID_CLASHES_MONITOR	copy;			/* used when copying */
};


/*****************************************************************************/
/*                                                                           */
/*  Submodule "construction and query"                                       */
/*                                                                           */
/*****************************************************************************/

/*****************************************************************************/
/*                                                                           */
/*  KHE_AVOID_CLASHES_MONITOR KheAvoidClashesMonitorMake(                    */
/*    KHE_RESOURCE_IN_SOLN rs, KHE_AVOID_CLASHES_CONSTRAINT c)               */
/*                                                                           */
/*  Make a new avoid clashes monitor with these attributes.                  */
/*                                                                           */
/*****************************************************************************/

KHE_AVOID_CLASHES_MONITOR KheAvoidClashesMonitorMake(
  KHE_RESOURCE_IN_SOLN rs, KHE_AVOID_CLASHES_CONSTRAINT c)
{
  KHE_AVOID_CLASHES_MONITOR res;  KHE_SOLN soln;
  soln = KheResourceInSolnSoln(rs);
  MMake(res);
  KheMonitorInitCommonFields((KHE_MONITOR) res, soln,
    KHE_AVOID_CLASHES_MONITOR_TAG);
  res->resource_in_soln = rs;
  res->constraint = c;
  res->separate =
    (KheConstraintCostFunction((KHE_CONSTRAINT) c) != KHE_SUM_COST_FUNCTION);
  KheDevMonitorInit(&res->separate_dev_monitor);
  res->total_devs = 0;
  res->new_total_devs = 0;
  res->copy = NULL;
  KheResourceInSolnAddMonitor(rs, (KHE_MONITOR) res);
  /* KheGroupMonitorAddMonitor((KHE_GROUP_MONITOR) soln, (KHE_MONITOR) res); */
  return res;
}


/*****************************************************************************/
/*                                                                           */
/*  KHE_AVOID_CLASHES_MONITOR KheAvoidClashesMonitorCopyPhase1(              */
/*    KHE_AVOID_CLASHES_MONITOR m)                                           */
/*                                                                           */
/*  Carry out Phase 1 of copying m.                                          */
/*                                                                           */
/*****************************************************************************/

KHE_AVOID_CLASHES_MONITOR KheAvoidClashesMonitorCopyPhase1(
  KHE_AVOID_CLASHES_MONITOR m)
{
  KHE_AVOID_CLASHES_MONITOR copy;
  if( m->copy == NULL )
  {
    MMake(copy);
    m->copy = copy;
    KheMonitorCopyCommonFields((KHE_MONITOR) copy, (KHE_MONITOR) m);
    copy->resource_in_soln = KheResourceInSolnCopyPhase1(m->resource_in_soln);
    copy->constraint = m->constraint;
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
/*  void KheAvoidClashesMonitorCopyPhase2(KHE_AVOID_CLASHES_MONITOR m)       */
/*                                                                           */
/*  Carry out Phase 2 of copying m.                                          */
/*                                                                           */
/*****************************************************************************/

void KheAvoidClashesMonitorCopyPhase2(KHE_AVOID_CLASHES_MONITOR m)
{
  if( m->copy != NULL )
  {
    m->copy = NULL;
    KheResourceInSolnCopyPhase2(m->resource_in_soln);
  }
}


/*****************************************************************************/
/*                                                                           */
/*  void KheAvoidClashesMonitorDelete(KHE_AVOID_CLASHES_MONITOR m)           */
/*                                                                           */
/*  Free m.                                                                  */
/*                                                                           */
/*****************************************************************************/

void KheAvoidClashesMonitorDelete(KHE_AVOID_CLASHES_MONITOR m)
{
  if( m->attached )
    KheAvoidClashesMonitorDetachFromSoln(m);
  if( m->parent_monitor != NULL )
    KheGroupMonitorDeleteChildMonitor(m->parent_monitor, (KHE_MONITOR) m);
  KheResourceInSolnDeleteMonitor(m->resource_in_soln, (KHE_MONITOR) m);
  KheSolnDeleteMonitor(m->soln, (KHE_MONITOR) m);
  KheDevMonitorFree(&m->separate_dev_monitor);
  MFree(m);
}


/*****************************************************************************/
/*                                                                           */
/*  KHE_AVOID_CLASHES_CONSTRAINT KheAvoidClashesMonitorConstraint(           */
/*    KHE_AVOID_CLASHES_MONITOR m)                                           */
/*                                                                           */
/*  Return the contraint that m is monitoring.                               */
/*                                                                           */
/*****************************************************************************/

KHE_AVOID_CLASHES_CONSTRAINT KheAvoidClashesMonitorConstraint(
  KHE_AVOID_CLASHES_MONITOR m)
{
  return m->constraint;
}


/*****************************************************************************/
/*                                                                           */
/*  KHE_RESOURCE KheAvoidClashesMonitorResource(KHE_AVOID_CLASHES_MONITOR m) */
/*                                                                           */
/*  Return the resource that m is monitoring.                                */
/*                                                                           */
/*****************************************************************************/

KHE_RESOURCE KheAvoidClashesMonitorResource(KHE_AVOID_CLASHES_MONITOR m)
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
/*  void KheAvoidClashesMonitorAttachToSoln(KHE_AVOID_CLASHES_MONITOR m)     */
/*                                                                           */
/*  Attach m.  It is known to be currently detached with cost 0.             */
/*                                                                           */
/*****************************************************************************/

void KheAvoidClashesMonitorAttachToSoln(KHE_AVOID_CLASHES_MONITOR m)
{
  m->attached = true;
  KheResourceInSolnAttachMonitor(m->resource_in_soln, (KHE_MONITOR) m);
}


/*****************************************************************************/
/*                                                                           */
/*  void KheAvoidClashesMonitorDetachFromSoln(KHE_AVOID_CLASHES_MONITOR m)   */
/*                                                                           */
/*  Detach m.  It is known to be currently attached.                         */
/*                                                                           */
/*****************************************************************************/

void KheAvoidClashesMonitorDetachFromSoln(KHE_AVOID_CLASHES_MONITOR m)
{
  KheResourceInSolnDetachMonitor(m->resource_in_soln, (KHE_MONITOR) m);
  m->attached = false;
}


/*****************************************************************************/
/*                                                                           */
/*  void KheAvoidClashesMonitorAttachCheck(KHE_AVOID_CLASHES_MONITOR m)      */
/*                                                                           */
/*  Check the attachment of m.                                               */
/*                                                                           */
/*****************************************************************************/

void KheAvoidClashesMonitorAttachCheck(KHE_AVOID_CLASHES_MONITOR m)
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
/*  void KheAvoidClashesMonitorChangeClashCount(KHE_AVOID_CLASHES_MONITOR m, */
/*    int old_clash_count, int new_clash_count)                              */
/*                                                                           */
/*  Change one of the deviation counds held by m from old_clash_count to     */
/*  new_clash_count.  These are assumed to be distinct.                      */
/*                                                                           */
/*****************************************************************************/

void KheAvoidClashesMonitorChangeClashCount(KHE_AVOID_CLASHES_MONITOR m,
  int old_clash_count, int new_clash_count)
{
  if( m->separate )
  {
    if( old_clash_count == 0 )
      KheDevMonitorAttach(&m->separate_dev_monitor, new_clash_count);
    else if( new_clash_count == 0 )
      KheDevMonitorDetach(&m->separate_dev_monitor, old_clash_count);
    else
      KheDevMonitorUpdate(&m->separate_dev_monitor, old_clash_count,
	new_clash_count);
  }
  else
    m->new_total_devs += (new_clash_count - old_clash_count);
}


/*****************************************************************************/
/*                                                                           */
/*  void KheAvoidClashesMonitorFlush(KHE_AVOID_CLASHES_MONITOR m)            */
/*                                                                           */
/*  Flush m.                                                                 */
/*                                                                           */
/*****************************************************************************/

void KheAvoidClashesMonitorFlush(KHE_AVOID_CLASHES_MONITOR m)
{
  if( m->separate )
  {
    if( KheDevMonitorHasChanged(&m->separate_dev_monitor) )
    {
      KheMonitorChangeCost((KHE_MONITOR) m,
        KheConstraintCostMulti((KHE_CONSTRAINT) m->constraint,
	  KheDevMonitorDevs(&m->separate_dev_monitor)));
      KheDevMonitorFlush(&m->separate_dev_monitor);
    }
  }
  else
  {
    if( m->new_total_devs != m->total_devs )
    {
      KheMonitorChangeCost((KHE_MONITOR) m,
        KheConstraintCost((KHE_CONSTRAINT) m->constraint, m->new_total_devs));
      m->total_devs = m->new_total_devs;
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
/*  int KheAvoidClashesMonitorDeviationCount(KHE_AVOID_CLASHES_MONITOR m)    */
/*                                                                           */
/*  Return the number of deviations of m.                                    */
/*                                                                           */
/*****************************************************************************/

int KheAvoidClashesMonitorDeviationCount(KHE_AVOID_CLASHES_MONITOR m)
{
  KHE_TIMETABLE_MONITOR tt;  KHE_INSTANCE ins;  KHE_TIME t;  int i, res;
  ins = KheSolnInstance(m->soln);
  tt = KheResourceTimetableMonitor(m->soln,
    KheResourceInSolnResource(m->resource_in_soln));
  if( !KheMonitorAttachedToSoln((KHE_MONITOR) tt) )
    KheMonitorAttachToSoln((KHE_MONITOR) tt);
  res = 0;
  for( i = 0;  i < KheInstanceTimeCount(ins);  i++ )
  {
    t = KheInstanceTime(ins, i);
    if( KheTimetableMonitorTimeMeetCount(tt, t) > 1 )
      res++;
  }
  if( DEBUG1 )
    fprintf(stderr, "  KheAvoidClashesMonitorDeviationCount(m) returning %d\n",
      res);
  return res;
}


/*****************************************************************************/
/*                                                                           */
/*  bool FindDeviation(KHE_AVOID_CLASHES_MONITOR m, int i,                   */
/*    KHE_TIME *t, int *dev)                                                 */
/*                                                                           */
/*  If m has an i'th deviation, return true with *t set to its time and      */
/*  *dev set to its deviation.  Otherwise return false.                      */
/*                                                                           */
/*****************************************************************************/

static bool FindDeviation(KHE_AVOID_CLASHES_MONITOR m, int i,
  KHE_TIME *t, int *dev)
{
  KHE_TIMETABLE_MONITOR tt;  KHE_INSTANCE ins;  int j, res;
  ins = KheSolnInstance(m->soln);
  tt = KheResourceTimetableMonitor(m->soln,
    KheResourceInSolnResource(m->resource_in_soln));
  if( !KheMonitorAttachedToSoln((KHE_MONITOR) tt) )
    KheMonitorAttachToSoln((KHE_MONITOR) tt);
  res = 0;
  for( j = 0;  j < KheInstanceTimeCount(ins);  j++ )
  {
    *t = KheInstanceTime(ins, j);
    *dev = KheTimetableMonitorTimeMeetCount(tt, *t) - 1;
    if( *dev > 0 )
      res++;
    if( res == i + 1 )
    {
      if( DEBUG1 )
	fprintf(stderr, "  FindDeviation(m, %d) returning true (%s, %d)\n",
	  i, KheTimeName(*t), *dev);
      return true;
    }
  }
  if( DEBUG1 )
    fprintf(stderr, "  FindDeviation(m, %d) returning false\n", i);
  return false;
}


/*****************************************************************************/
/*                                                                           */
/*  int KheAvoidClashesMonitorDeviation(KHE_AVOID_CLASHES_MONITOR m, int i)  */
/*                                                                           */
/*  Return the i'th deviation of m.                                          */
/*                                                                           */
/*****************************************************************************/

int KheAvoidClashesMonitorDeviation(KHE_AVOID_CLASHES_MONITOR m, int i)
{
  KHE_TIME t;  int dev;
  if( !FindDeviation(m, i, &t, &dev) )
    MAssert(false, "KheAvoidClashesMonitorDeviation: i out of range");
  return dev;
}


/*****************************************************************************/
/*                                                                           */
/*  char *KheAvoidClashesMonitorDeviationDescription(                        */
/*    KHE_AVOID_CLASHES_MONITOR m, int i)                                    */
/*                                                                           */
/*  Return a description of the i'th deviation of m.                         */
/*                                                                           */
/*****************************************************************************/

char *KheAvoidClashesMonitorDeviationDescription(
  KHE_AVOID_CLASHES_MONITOR m, int i)
{
  KHE_TIME t;  int dev;
  if( !FindDeviation(m, i, &t, &dev) )
    MAssert(false,
      "KheAvoidClashesMonitorDeviationDescription: i out of range");
  return KheTimeName(t);
}


/*****************************************************************************/
/*                                                                           */
/*  Submodule "debug"                                                        */
/*                                                                           */
/*****************************************************************************/

/*****************************************************************************/
/*                                                                           */
/*  void KheAvoidClashesMonitorDebug(KHE_AVOID_CLASHES_MONITOR m,            */
/*    int verbosity, int indent, FILE *fp)                                   */
/*                                                                           */
/*  Debug print of m onto fp with the given verbosity and indent.            */
/*                                                                           */
/*****************************************************************************/

void KheAvoidClashesMonitorDebug(KHE_AVOID_CLASHES_MONITOR m,
  int verbosity, int indent, FILE *fp)
{
  if( verbosity >= 1 )
  {
    KheMonitorDebugBegin((KHE_MONITOR) m, indent, fp);
    fprintf(fp, " ");
    KheResourceInSolnDebug(m->resource_in_soln, 1, -1, fp);
    fprintf(fp, "%s ", m->separate ? " sep" : "");
    if( m->separate )
      KheDevMonitorDebug(&m->separate_dev_monitor, fp);
    else
      fprintf(fp, "%s%d", m->total_devs != m->new_total_devs ? "*" : "",
	m->total_devs);
    KheMonitorDebugEnd((KHE_MONITOR) m, true, indent, fp);
  }
}
