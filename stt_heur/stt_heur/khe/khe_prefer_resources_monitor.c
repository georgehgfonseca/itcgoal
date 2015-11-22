
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
/*  FILE:         khe_prefer_resources_monitor.c                             */
/*  DESCRIPTION:  An prefer resources monitor                                */
/*                                                                           */
/*****************************************************************************/
#include "khe_interns.h"


/*****************************************************************************/
/*                                                                           */
/*  KHE_PREFER_RESOURCES_MONITOR - an prefer resources monitor               */
/*                                                                           */
/*****************************************************************************/

struct khe_prefer_resources_monitor_rec {

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

  /* specific to KHE_PREFER_RESOURCES_MONITOR */
  int				deviation;		/* deviation         */
  KHE_EVENT_RESOURCE_IN_SOLN	event_resource_in_soln;	/* enclosing ers     */
  KHE_PREFER_RESOURCES_CONSTRAINT constraint;		/* constraint        */
  KHE_PREFER_RESOURCES_MONITOR	copy;			/* used when copying */
};


/*****************************************************************************/
/*                                                                           */
/*  Submodule "construction and query"                                       */
/*                                                                           */
/*****************************************************************************/

/*****************************************************************************/
/*                                                                           */
/*  KHE_PREFER_RESOURCES_MONITOR KhePreferResourcesMonitorMake(              */
/*    KHE_EVENT_RESOURCE_IN_SOLN ers, KHE_PREFER_RESOURCES_CONSTRAINT c)     */
/*                                                                           */
/*  Make a new prefer resources monitor with these attributes.               */
/*                                                                           */
/*****************************************************************************/

KHE_PREFER_RESOURCES_MONITOR KhePreferResourcesMonitorMake(
  KHE_EVENT_RESOURCE_IN_SOLN ers, KHE_PREFER_RESOURCES_CONSTRAINT c)
{
  KHE_PREFER_RESOURCES_MONITOR res;  KHE_SOLN soln;
  soln = KheEventInSolnSoln(KheEventResourceInSolnEventInSoln(ers));
  MMake(res);
  KheMonitorInitCommonFields((KHE_MONITOR) res, soln,
    KHE_PREFER_RESOURCES_MONITOR_TAG);
  res->deviation = 0;
  res->event_resource_in_soln = ers;
  res->constraint = c;
  res->copy = NULL;
  KheEventResourceInSolnAddMonitor(ers, (KHE_MONITOR) res);
  return res;
}


/*****************************************************************************/
/*                                                                           */
/*  KHE_PREFER_RESOURCES_MONITOR KhePreferResourcesMonitorCopyPhase1(        */
/*    KHE_PREFER_RESOURCES_MONITOR m)                                        */
/*                                                                           */
/*  Carry out Phase 1 of copying m.                                          */
/*                                                                           */
/*****************************************************************************/

KHE_PREFER_RESOURCES_MONITOR KhePreferResourcesMonitorCopyPhase1(
  KHE_PREFER_RESOURCES_MONITOR m)
{
  KHE_PREFER_RESOURCES_MONITOR copy;
  if( m->copy == NULL )
  {
    MMake(copy);
    m->copy = copy;
    KheMonitorCopyCommonFields((KHE_MONITOR) copy, (KHE_MONITOR) m);
    copy->deviation = 0;
    copy->event_resource_in_soln =
      KheEventResourceInSolnCopyPhase1(m->event_resource_in_soln);
    copy->constraint = m->constraint;
    copy->copy = NULL;
  }
  return m->copy;
}


/*****************************************************************************/
/*                                                                           */
/*  void KhePreferResourcesMonitorCopyPhase2(KHE_PREFER_RESOURCES_MONITOR m) */
/*                                                                           */
/*  Carry out Phase 2 of copying m.                                          */
/*                                                                           */
/*****************************************************************************/

void KhePreferResourcesMonitorCopyPhase2(KHE_PREFER_RESOURCES_MONITOR m)
{
  if( m->copy != NULL )
  {
    m->copy = NULL;
  }
}


/*****************************************************************************/
/*                                                                           */
/*  void KhePreferResourcesMonitorDelete(KHE_PREFER_RESOURCES_MONITOR m)     */
/*                                                                           */
/*  Delete m.                                                                */
/*                                                                           */
/*****************************************************************************/

void KhePreferResourcesMonitorDelete(KHE_PREFER_RESOURCES_MONITOR m)
{
  if( m->attached )
    KhePreferResourcesMonitorDetachFromSoln(m);
  if( m->parent_monitor != NULL )
    KheGroupMonitorDeleteChildMonitor(m->parent_monitor, (KHE_MONITOR) m);
  KheEventResourceInSolnDeleteMonitor(m->event_resource_in_soln,
    (KHE_MONITOR) m);
  KheSolnDeleteMonitor(m->soln, (KHE_MONITOR) m);
  MFree(m);
}


/*****************************************************************************/
/*                                                                           */
/*  KHE_PREFER_RESOURCES_CONSTRAINT KhePreferResourcesMonitorConstraint(     */
/*    KHE_PREFER_RESOURCES_MONITOR m)                                        */
/*                                                                           */
/*  Return the constraint that m is monitoring.                              */
/*                                                                           */
/*****************************************************************************/

KHE_PREFER_RESOURCES_CONSTRAINT KhePreferResourcesMonitorConstraint(
  KHE_PREFER_RESOURCES_MONITOR m)
{
  return m->constraint;
}


/*****************************************************************************/
/*                                                                           */
/*  KHE_EVENT KhePreferResourcesMonitorEvent(KHE_PREFER_RESOURCES_MONITOR m) */
/*                                                                           */
/*  Return the event that m is monitoring.                                   */
/*                                                                           */
/*****************************************************************************/

KHE_EVENT_RESOURCE KhePreferResourcesMonitorEventResource(
  KHE_PREFER_RESOURCES_MONITOR m)
{
  return KheEventResourceInSolnEventResource(m->event_resource_in_soln);
}


/*****************************************************************************/
/*                                                                           */
/*  Submodule "attach and detach"                                            */
/*                                                                           */
/*****************************************************************************/

/*****************************************************************************/
/*                                                                           */
/* void KhePreferResourcesMonitorAttachToSoln(KHE_PREFER_RESOURCES_MONITOR m)*/
/*                                                                           */
/*  Attach m.  It is known to be currently detached with cost 0.             */
/*                                                                           */
/*****************************************************************************/

void KhePreferResourcesMonitorAttachToSoln(KHE_PREFER_RESOURCES_MONITOR m)
{
  m->attached = true;
  KheEventResourceInSolnAttachMonitor(m->event_resource_in_soln,
    (KHE_MONITOR) m);
}


/*****************************************************************************/
/*                                                                           */
/*  void KhePreferResourcesMonitorDetachFromSoln(                            */
/*    KHE_PREFER_RESOURCES_MONITOR m)                                        */
/*                                                                           */
/*  Detach m.  It is known to be currently attached.                         */
/*                                                                           */
/*****************************************************************************/

void KhePreferResourcesMonitorDetachFromSoln(KHE_PREFER_RESOURCES_MONITOR m)
{
  KheEventResourceInSolnDetachMonitor(m->event_resource_in_soln,
    (KHE_MONITOR) m);
  m->attached = false;
}


/*****************************************************************************/
/*                                                                           */
/*  void KhePreferResourcesMonitorAttachCheck(KHE_PREFER_RESOURCES_MONITOR m)*/
/*                                                                           */
/*  Check the attachment of m.                                               */
/*                                                                           */
/*  Implementation note.  This is one of the interesting cases.  If the      */
/*  domains of the leaders of the tasks monitored by m are subsets of m's    */
/*  constraint's domain, then m cannot be violated and may be detached.      */
/*                                                                           */
/*****************************************************************************/

void KhePreferResourcesMonitorAttachCheck(KHE_PREFER_RESOURCES_MONITOR m)
{
  KHE_RESOURCE_GROUP domain;  KHE_EVENT_RESOURCE er;  KHE_TASK task;  int i;

  /* find the domain of m's constraint */
  domain = KhePreferResourcesConstraintDomain(m->constraint);

  /* compare it with the domains of the leaders of the tasks that m monitors */
  er = KhePreferResourcesMonitorEventResource(m);
  for( i = 0;  i < KheEventResourceTaskCount(m->soln, er);  i++ )
  {
    task = KheTaskLeader(KheEventResourceTask(m->soln, er, i));
    if( !KheResourceGroupSubset(KheTaskDomain(task), domain) )
    {
      /* task has a larger domain than domain, so want to be attached */
      if( !KheMonitorAttachedToSoln((KHE_MONITOR) m) )
	KheMonitorAttachToSoln((KHE_MONITOR) m);
      return;
    }
  }

  /* no task has a larger domain than domain, so want to be detached */
  if( KheMonitorAttachedToSoln((KHE_MONITOR) m) )
    KheMonitorDetachFromSoln((KHE_MONITOR) m);
}


/*****************************************************************************/
/*                                                                           */
/*  Submodule "monitoring calls"                                             */
/*                                                                           */
/*****************************************************************************/

/*****************************************************************************/
/*                                                                           */
/*  bool KhePreferResourcesMonitorWrongResource(                             */
/*    KHE_PREFER_RESOURCES_MONITOR m, KHE_RESOURCE r)                        */
/*                                                                           */
/*  Return true when r should attract a cost.                                */
/*                                                                           */
/*****************************************************************************/

static bool KhePreferResourcesMonitorWrongResource(
  KHE_PREFER_RESOURCES_MONITOR m, KHE_RESOURCE r)
{
  return r != NULL && !KheResourceGroupContains(
    KhePreferResourcesConstraintDomain(m->constraint), r);
}


/*****************************************************************************/
/*                                                                           */
/*  void KhePreferResourcesMonitorAddTask(KHE_PREFER_RESOURCES_MONITOR m,    */
/*    KHE_TASK task)                                                         */
/*                                                                           */
/*  Monitor the effect of adding task.                                       */
/*                                                                           */
/*****************************************************************************/

void KhePreferResourcesMonitorAddTask(KHE_PREFER_RESOURCES_MONITOR m,
  KHE_TASK task)
{
  if( KhePreferResourcesMonitorWrongResource(m, KheTaskAsstResource(task)) )
  {
    m->deviation += KheTaskDuration(task);
    KheMonitorChangeCost((KHE_MONITOR) m,
      KheConstraintCost((KHE_CONSTRAINT) m->constraint, m->deviation));
  }
}


/*****************************************************************************/
/*                                                                           */
/*  void KhePreferResourcesMonitorDeleteTask(KHE_PREFER_RESOURCES_MONITOR m, */
/*    KHE_TASK task)                                                         */
/*                                                                           */
/*  Monitor the effect of deleting task.                                     */
/*                                                                           */
/*****************************************************************************/

void KhePreferResourcesMonitorDeleteTask(KHE_PREFER_RESOURCES_MONITOR m,
  KHE_TASK task)
{
  if( KhePreferResourcesMonitorWrongResource(m, KheTaskAsstResource(task)) )
  {
    m->deviation -= KheTaskDuration(task);
    MAssert(m->deviation >= 0,
      "KhePreferResourcesMonitorDeleteTask internal error");
    KheMonitorChangeCost((KHE_MONITOR) m,
      KheConstraintCost((KHE_CONSTRAINT) m->constraint, m->deviation));
  }
}


/*****************************************************************************/
/*                                                                           */
/*  void KhePreferResourcesMonitorSplitTask(KHE_PREFER_RESOURCES_MONITOR m,  */
/*    KHE_TASK task1, KHE_TASK task2)                                        */
/*                                                                           */
/*  Let m know that a task has just split into task1 and task2.              */
/*  Either both tasks are assigned the same resource, or                     */
/*  else they aren't assigned at all; so no change in cost is possible.      */
/*                                                                           */
/*****************************************************************************/

void KhePreferResourcesMonitorSplitTask(KHE_PREFER_RESOURCES_MONITOR m,
  KHE_TASK task1, KHE_TASK task2)
{
  /* no change in cost is possible */
  /* ***
  KHE_COST new_cost;  int durn1, durn2;
  if( KheConstraintCostFunction((KHE_CONSTRAINT) m->constraint) !=
        KHE_SUM_COST_FUNCTION &&
      KhePreferResourcesMonitorWrongResource(m, KheTaskAsstResource(task1)) )
  {
    ** delete 1 old, add 2 new **
    durn1 = KheTaskDuration(task1);
    durn2 = KheTaskDuration(task2);
    new_cost = m->cost
      + KheConstraintCost((KHE_CONSTRAINT) m->constraint, durn1)
      + KheConstraintCost((KHE_CONSTRAINT) m->constraint, durn2)
      - KheConstraintCost((KHE_CONSTRAINT) m->constraint, durn1 + durn2);

    ** report any change in cost **
    KheMonitorChangeCost((KHE_MONITOR) m, new_cost);
  }
  *** */
}


/*****************************************************************************/
/*                                                                           */
/*  void KhePreferResourcesMonitorMergeTask(KHE_PREFER_RESOURCES_MONITOR m,  */
/*    KHE_TASK task1, KHE_TASK task2)                                        */
/*                                                                           */
/*  Let m know that task1 and task2 are just about to be merged.             */
/*                                                                           */
/*  It might seem that there is a problem here if task1 is assigned and      */
/*  task2 is not, or vice versa.  However, this code is called only by       */
/*  KheTaskMerge, and it is a precondition of that function that task1 and   */
/*  task2 must either both be unassigned or assigned to the same resource.   */
/*                                                                           */
/*****************************************************************************/

void KhePreferResourcesMonitorMergeTask(KHE_PREFER_RESOURCES_MONITOR m,
  KHE_TASK task1, KHE_TASK task2)
{
  /* no change in cost is possible */
  /* ***
  KHE_COST new_cost;  int durn1, durn2;
  if( KheConstraintCostFunction((KHE_CONSTRAINT) m->constraint)
        != KHE_SUM_COST_FUNCTION &&
      KhePreferResourcesMonitorWrongResource(m, KheTaskAsstResource(task1)) )
  {
    ** time not assigned, so change is possible; delete 2 old, add 1 new **
    durn1 = KheTaskDuration(task1);
    durn2 = KheTaskDuration(task2);
    new_cost = m->cost
      - KheConstraintCost((KHE_CONSTRAINT) m->constraint, durn1)
      - KheConstraintCost((KHE_CONSTRAINT) m->constraint, durn2)
      + KheConstraintCost((KHE_CONSTRAINT) m->constraint, durn1 + durn2);

    ** report any change in cost **
    KheMonitorChangeCost((KHE_MONITOR) m, new_cost);
  }
  *** */
}


/*****************************************************************************/
/*                                                                           */
/*  void KhePreferResourcesMonitorAssignResource(                            */
/*    KHE_PREFER_RESOURCES_MONITOR m, KHE_TASK task, KHE_RESOURCE r)         */
/*                                                                           */
/*  Let m know that task is about to be assigned resource r.                 */
/*                                                                           */
/*****************************************************************************/

void KhePreferResourcesMonitorAssignResource(KHE_PREFER_RESOURCES_MONITOR m,
  KHE_TASK task, KHE_RESOURCE r)
{
  if( KhePreferResourcesMonitorWrongResource(m, r) )
  {
    m->deviation += KheTaskDuration(task);
    KheMonitorChangeCost((KHE_MONITOR) m,
      KheConstraintCost((KHE_CONSTRAINT) m->constraint, m->deviation));
  }
}


/*****************************************************************************/
/*                                                                           */
/*  void KhePreferResourcesMonitorUnAssignResource(                          */
/*    KHE_PREFER_RESOURCES_MONITOR m, KHE_TASK task, KHE_RESOURCE r)         */
/*                                                                           */
/*  Let m know that task is about to be unassigned resource r.               */
/*                                                                           */
/*****************************************************************************/

void KhePreferResourcesMonitorUnAssignResource(KHE_PREFER_RESOURCES_MONITOR m,
  KHE_TASK task, KHE_RESOURCE r)
{
  if( KhePreferResourcesMonitorWrongResource(m, r) )
  {
    m->deviation -= KheTaskDuration(task);
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
/*  int KhePreferResourcesMonitorDeviationCount(                             */
/*    KHE_PREFER_RESOURCES_MONITOR m)                                        */
/*                                                                           */
/*  Return the number of deviations of m.                                    */
/*                                                                           */
/*****************************************************************************/

int KhePreferResourcesMonitorDeviationCount(KHE_PREFER_RESOURCES_MONITOR m)
{
  return 1;
}


/*****************************************************************************/
/*                                                                           */
/*  int KhePreferResourcesMonitorDeviation(                                  */
/*    KHE_PREFER_RESOURCES_MONITOR m, int i)                                 */
/*                                                                           */
/*  Return the i'th deviation of m.                                          */
/*                                                                           */
/*****************************************************************************/

int KhePreferResourcesMonitorDeviation(
  KHE_PREFER_RESOURCES_MONITOR m, int i)
{
  MAssert(i == 0, "KhePreferResourcesMonitorDeviation: i out of range");
  return m->deviation;
}


/*****************************************************************************/
/*                                                                           */
/*  char *KhePreferResourcesMonitorDeviationDescription(                     */
/*    KHE_PREFER_RESOURCES_MONITOR m, int i)                                 */
/*                                                                           */
/*  Return a description of the i'th deviation of m.                         */
/*                                                                           */
/*****************************************************************************/

char *KhePreferResourcesMonitorDeviationDescription(
  KHE_PREFER_RESOURCES_MONITOR m, int i)
{
  MAssert(i == 0,
    "KhePreferResourcesMonitorDeviationDescription: i out of range");
  return NULL;
}


/*****************************************************************************/
/*                                                                           */
/*  Submodule "debug"                                                        */
/*                                                                           */
/*****************************************************************************/

/*****************************************************************************/
/*                                                                           */
/*  void KhePreferResourcesMonitorDebug(KHE_PREFER_RESOURCES_MONITOR m,      */
/*    int verbosity, int indent, FILE *fp)                                   */
/*                                                                           */
/*  Debug print of m onto fp with the given verbosity and indent.            */
/*                                                                           */
/*****************************************************************************/

void KhePreferResourcesMonitorDebug(KHE_PREFER_RESOURCES_MONITOR m,
  int verbosity, int indent, FILE *fp)
{
  if( verbosity >= 1 )
  {
    KheMonitorDebugBegin((KHE_MONITOR) m, indent, fp);
    fprintf(fp, " ");
    KheEventResourceInSolnDebug(m->event_resource_in_soln, 1, -1, fp);
    KheMonitorDebugEnd((KHE_MONITOR) m, true, indent, fp);
  }
}
