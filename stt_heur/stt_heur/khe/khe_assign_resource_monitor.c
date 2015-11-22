
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
/*  FILE:         khe_assign_resource_monitor.c                              */
/*  DESCRIPTION:  An assign resource monitor                                 */
/*                                                                           */
/*****************************************************************************/
#include "khe_interns.h"


/*****************************************************************************/
/*                                                                           */
/*  KHE_ASSIGN_RESOURCE_MONITOR - an assign resource monitor                 */
/*                                                                           */
/*****************************************************************************/

struct khe_assign_resource_monitor_rec {

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

  /* specific to KHE_ASSIGN_RESOURCE_MONITOR */
  int				deviation;		/* deviation         */
  KHE_EVENT_RESOURCE_IN_SOLN	event_resource_in_soln;	/* enclosing ers     */
  KHE_ASSIGN_RESOURCE_CONSTRAINT constraint;		/* constraint        */
  KHE_ASSIGN_RESOURCE_MONITOR	copy;			/* used when copying */
};


/*****************************************************************************/
/*                                                                           */
/*  Submodule "construction and query"                                       */
/*                                                                           */
/*****************************************************************************/

/*****************************************************************************/
/*                                                                           */
/*  KHE_ASSIGN_RESOURCE_MONITOR KheAssignResourceMonitorMake(                */
/*    KHE_EVENT_RESOURCE_IN_SOLN ers, KHE_ASSIGN_RESOURCE_CONSTRAINT c)      */
/*                                                                           */
/*  Make a new assign resource monitor with these attributes.                */
/*                                                                           */
/*****************************************************************************/

KHE_ASSIGN_RESOURCE_MONITOR KheAssignResourceMonitorMake(
  KHE_EVENT_RESOURCE_IN_SOLN ers, KHE_ASSIGN_RESOURCE_CONSTRAINT c)
{
  KHE_ASSIGN_RESOURCE_MONITOR res;  KHE_SOLN soln;
  soln = KheEventInSolnSoln(KheEventResourceInSolnEventInSoln(ers));
  MMake(res);
  KheMonitorInitCommonFields((KHE_MONITOR) res, soln,
    KHE_ASSIGN_RESOURCE_MONITOR_TAG);
  res->deviation = 0;
  res->event_resource_in_soln = ers;
  res->constraint = c;
  res->copy = NULL;
  KheEventResourceInSolnAddMonitor(ers, (KHE_MONITOR) res);
  return res;
}


/*****************************************************************************/
/*                                                                           */
/*  KHE_ASSIGN_RESOURCE_MONITOR KheAssignResourceMonitorCopyPhase1(          */
/*    KHE_ASSIGN_RESOURCE_MONITOR m)                                         */
/*                                                                           */
/*  Carry out Phase 1 of copying m.                                          */
/*                                                                           */
/*****************************************************************************/

KHE_ASSIGN_RESOURCE_MONITOR KheAssignResourceMonitorCopyPhase1(
  KHE_ASSIGN_RESOURCE_MONITOR m)
{
  KHE_ASSIGN_RESOURCE_MONITOR copy;
  if( m->copy == NULL )
  {
    MMake(copy);
    m->copy = copy;
    KheMonitorCopyCommonFields((KHE_MONITOR) copy, (KHE_MONITOR) m);
    copy->deviation = m->deviation;
    copy->event_resource_in_soln =
      KheEventResourceInSolnCopyPhase1(m->event_resource_in_soln);
    copy->constraint = m->constraint;
    copy->copy = NULL;
  }
  return m->copy;
}


/*****************************************************************************/
/*                                                                           */
/*  void KheAssignResourceMonitorCopyPhase2(KHE_ASSIGN_RESOURCE_MONITOR m)   */
/*                                                                           */
/*  Carry out Phase 2 of copying m.                                          */
/*                                                                           */
/*****************************************************************************/

void KheAssignResourceMonitorCopyPhase2(KHE_ASSIGN_RESOURCE_MONITOR m)
{
  if( m->copy != NULL )
  {
    m->copy = NULL;
  }
}


/*****************************************************************************/
/*                                                                           */
/*  void KheAssignResourceMonitorDelete(KHE_ASSIGN_RESOURCE_MONITOR m)       */
/*                                                                           */
/*  Delete m.                                                                */
/*                                                                           */
/*****************************************************************************/

void KheAssignResourceMonitorDelete(KHE_ASSIGN_RESOURCE_MONITOR m)
{
  if( m->attached )
    KheAssignResourceMonitorDetachFromSoln(m);
  if( m->parent_monitor != NULL )
    KheGroupMonitorDeleteChildMonitor(m->parent_monitor, (KHE_MONITOR) m);
  KheEventResourceInSolnDeleteMonitor(m->event_resource_in_soln,
    (KHE_MONITOR) m);
  KheSolnDeleteMonitor(m->soln, (KHE_MONITOR) m);
  MFree(m);
}


/*****************************************************************************/
/*                                                                           */
/*  KHE_ASSIGN_RESOURCE_CONSTRAINT KheAssignResourceMonitorConstraint(       */
/*    KHE_ASSIGN_RESOURCE_MONITOR m)                                         */
/*                                                                           */
/*  Return the constraint that m is monitoring.                              */
/*                                                                           */
/*****************************************************************************/

KHE_ASSIGN_RESOURCE_CONSTRAINT KheAssignResourceMonitorConstraint(
  KHE_ASSIGN_RESOURCE_MONITOR m)
{
  return m->constraint;
}


/*****************************************************************************/
/*                                                                           */
/*  KHE_EVENT KheAssignResourceMonitorEvent(KHE_ASSIGN_RESOURCE_MONITOR m)   */
/*                                                                           */
/*  Return the event that m is monitoring.                                   */
/*                                                                           */
/*****************************************************************************/

KHE_EVENT_RESOURCE KheAssignResourceMonitorEventResource(
  KHE_ASSIGN_RESOURCE_MONITOR m)
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
/*  void KheAssignResourceMonitorAttachToSoln(KHE_ASSIGN_RESOURCE_MONITOR m) */
/*                                                                           */
/*  Attach m.  It is known to be currently detached with cost 0.             */
/*                                                                           */
/*****************************************************************************/

void KheAssignResourceMonitorAttachToSoln(KHE_ASSIGN_RESOURCE_MONITOR m)
{
  m->attached = true;
  KheEventResourceInSolnAttachMonitor(m->event_resource_in_soln,
    (KHE_MONITOR) m);
}


/*****************************************************************************/
/*                                                                           */
/* void KheAssignResourceMonitorDetachFromSoln(KHE_ASSIGN_RESOURCE_MONITOR m)*/
/*                                                                           */
/*  Detach m.  It is known to be currently attached.                         */
/*                                                                           */
/*****************************************************************************/

void KheAssignResourceMonitorDetachFromSoln(KHE_ASSIGN_RESOURCE_MONITOR m)
{
  KheEventResourceInSolnDetachMonitor(m->event_resource_in_soln,
    (KHE_MONITOR) m);
  m->attached = false;
}


/*****************************************************************************/
/*                                                                           */
/*  void KheAssignResourceMonitorAttachCheck(KHE_ASSIGN_RESOURCE_MONITOR m)  */
/*                                                                           */
/*  Check the attachment of m.                                               */
/*                                                                           */
/*****************************************************************************/

void KheAssignResourceMonitorAttachCheck(KHE_ASSIGN_RESOURCE_MONITOR m)
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
/*  void KheAssignResourceMonitorAddTask(KHE_ASSIGN_RESOURCE_MONITOR m,      */
/*    KHE_TASK task)                                                         */
/*                                                                           */
/*  Monitor the effect of adding task.                                       */
/*                                                                           */
/*****************************************************************************/

void KheAssignResourceMonitorAddTask(KHE_ASSIGN_RESOURCE_MONITOR m,
  KHE_TASK task)
{
  if( KheTaskAsst(task) == NULL )
  {
    m->deviation += KheTaskDuration(task);
    KheMonitorChangeCost((KHE_MONITOR) m,
      KheConstraintCost((KHE_CONSTRAINT) m->constraint, m->deviation));
  }
}


/*****************************************************************************/
/*                                                                           */
/*  void KheAssignResourceMonitorDeleteTask(KHE_ASSIGN_RESOURCE_MONITOR m,   */
/*    KHE_TASK task)                                                         */
/*                                                                           */
/*  Monitor the effect of deleting task.                                     */
/*                                                                           */
/*****************************************************************************/

void KheAssignResourceMonitorDeleteTask(KHE_ASSIGN_RESOURCE_MONITOR m,
  KHE_TASK task)
{
  if( KheTaskAsst(task) == NULL )
  {
    m->deviation -= KheTaskDuration(task);
    MAssert(m->deviation >= 0,
      "KheAssignResourceMonitorDeleteTask internal error");
    KheMonitorChangeCost((KHE_MONITOR) m,
      KheConstraintCost((KHE_CONSTRAINT) m->constraint, m->deviation));
  }
}


/*****************************************************************************/
/*                                                                           */
/*  void KheAssignResourceMonitorSplitTask(KHE_ASSIGN_RESOURCE_MONITOR m,    */
/*    KHE_TASK task1, KHE_TASK task2)                                        */
/*                                                                           */
/*  Let m know that a task has just split into task1 and task2.              */
/*  Either both tasks are assigned, or they aren't.                          */
/*                                                                           */
/*****************************************************************************/

void KheAssignResourceMonitorSplitTask(KHE_ASSIGN_RESOURCE_MONITOR m,
  KHE_TASK task1, KHE_TASK task2)
{
  /* no change in cost is possible */
}


/*****************************************************************************/
/*                                                                           */
/*  void KheAssignResourceMonitorMergeTask(KHE_ASSIGN_RESOURCE_MONITOR m,    */
/*    KHE_TASK task1, KHE_TASK task2)                                        */
/*                                                                           */
/*  Let m know that task1 and task2 are just about to be merged.  It might   */
/*  seem that there is a problem with this code if task1 is assigned         */
/*  a resource and task2 is not.  However, this code is called only by       */
/*  KheTaskMerge, and it is a precondition of that function that             */
/*  task1 and task2 must either both be unassigned or else both assigned to  */
/*  the same resource.                                                       */
/*                                                                           */
/*****************************************************************************/

void KheAssignResourceMonitorMergeTask(KHE_ASSIGN_RESOURCE_MONITOR m,
  KHE_TASK task1, KHE_TASK task2)
{
  /* no change in cost is possible */
}


/*****************************************************************************/
/*                                                                           */
/*  void KheAssignResourceMonitorAssignResource(                             */
/*    KHE_ASSIGN_RESOURCE_MONITOR m, KHE_TASK task, KHE_RESOURCE r)          */
/*                                                                           */
/*  Let m know that task has just been assigned resource r.                  */
/*                                                                           */
/*****************************************************************************/

void KheAssignResourceMonitorAssignResource(KHE_ASSIGN_RESOURCE_MONITOR m,
  KHE_TASK task, KHE_RESOURCE r)
{
  m->deviation -= KheTaskDuration(task);
  MAssert(m->deviation >= 0,
    "KheAssignResourceMonitorAssignTime internal error");
  KheMonitorChangeCost((KHE_MONITOR) m,
    KheConstraintCost((KHE_CONSTRAINT) m->constraint, m->deviation));
}


/*****************************************************************************/
/*                                                                           */
/*  void KheAssignResourceMonitorUnAssignResource(                           */
/*    KHE_ASSIGN_RESOURCE_MONITOR m, KHE_TASK task, KHE_RESOURCE r)          */
/*                                                                           */
/*  Let m know that task has just been unassigned resource r.                */
/*                                                                           */
/*****************************************************************************/

void KheAssignResourceMonitorUnAssignResource(KHE_ASSIGN_RESOURCE_MONITOR m,
  KHE_TASK task, KHE_RESOURCE r)
{
  m->deviation += KheTaskDuration(task);
  KheMonitorChangeCost((KHE_MONITOR) m,
    KheConstraintCost((KHE_CONSTRAINT) m->constraint, m->deviation));
}


/*****************************************************************************/
/*                                                                           */
/*  Submodule "deviations"                                                   */
/*                                                                           */
/*****************************************************************************/

/*****************************************************************************/
/*                                                                           */
/*  int KheAssignResourceMonitorDeviationCount(KHE_ASSIGN_RESOURCE_MONITOR m)*/
/*                                                                           */
/*  Return the number of deviations of m.                                    */
/*                                                                           */
/*****************************************************************************/

int KheAssignResourceMonitorDeviationCount(KHE_ASSIGN_RESOURCE_MONITOR m)
{
  return 1;
}


/*****************************************************************************/
/*                                                                           */
/*  int KheAssignResourceMonitorDeviation(                                   */
/*    KHE_ASSIGN_RESOURCE_MONITOR m, int i)                                  */
/*                                                                           */
/*  Return the i'th deviation of m.                                          */
/*                                                                           */
/*****************************************************************************/

int KheAssignResourceMonitorDeviation(KHE_ASSIGN_RESOURCE_MONITOR m, int i)
{
  MAssert(i == 0, "KheAssignResourceMonitorDeviation: i out of range");
  return m->deviation;
}


/*****************************************************************************/
/*                                                                           */
/*  char *KheAssignResourceMonitorDeviationDescription(                      */
/*    KHE_ASSIGN_RESOURCE_MONITOR m, int i)                                  */
/*                                                                           */
/*  Return a description of the i'th deviation of m.                         */
/*                                                                           */
/*****************************************************************************/

char *KheAssignResourceMonitorDeviationDescription(
  KHE_ASSIGN_RESOURCE_MONITOR m, int i)
{
  MAssert(i==0, "KheAssignResourceMonitorDeviationDescription: i out of range");
  return NULL;
}


/*****************************************************************************/
/*                                                                           */
/*  Submodule "debug"                                                        */
/*                                                                           */
/*****************************************************************************/

/*****************************************************************************/
/*                                                                           */
/*  void KheAssignResourceMonitorDebug(KHE_ASSIGN_RESOURCE_MONITOR m,        */
/*    int verbosity, int indent, FILE *fp)                                   */
/*                                                                           */
/*  Debug print of m onto fp with the given verbosity and indent.            */
/*                                                                           */
/*****************************************************************************/

void KheAssignResourceMonitorDebug(KHE_ASSIGN_RESOURCE_MONITOR m,
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
