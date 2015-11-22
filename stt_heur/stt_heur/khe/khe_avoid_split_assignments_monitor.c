
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
/*  FILE:         khe_avoid_split_assignments_monitor.c                      */
/*  DESCRIPTION:  An avoid split assignments monitor                         */
/*                                                                           */
/*****************************************************************************/
#include "khe_interns.h"

#define DEBUG1 0

/*****************************************************************************/
/*                                                                           */
/*  KHE_AVOID_SPLIT_ASSIGNMENTS_MONITOR - an avoid split assignments monitor */
/*                                                                           */
/*  The deviation is MArraySize(m->multiplicities) - 1, or 0 if that number  */
/*  is negative.                                                             */
/*                                                                           */
/*****************************************************************************/

struct khe_avoid_split_assignments_monitor_rec {

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

  /* specific to KHE_AVOID_SPLIT_ASSIGNMENTS_MONITOR */
  KHE_AVOID_SPLIT_ASSIGNMENTS_CONSTRAINT constraint;	/* the constraint    */
  int					eg_index;       /* event group index */
  ARRAY_KHE_RESOURCE			resources;	/* assigned now      */
  ARRAY_INT				multiplicities;	/* multiplicities    */
  KHE_AVOID_SPLIT_ASSIGNMENTS_MONITOR	copy;		/* used when copying */
};


/*****************************************************************************/
/*                                                                           */
/*  Submodule "construction and query"                                       */
/*                                                                           */
/*****************************************************************************/

/*****************************************************************************/
/*                                                                           */
/*  KHE_AVOID_SPLIT_ASSIGNMENTS_MONITOR KheAvoidSplitAssignmentsMonitorMake( */
/*    KHE_SOLN soln, KHE_AVOID_SPLIT_ASSIGNMENTS_CONSTRAINT c, int eg_index) */
/*                                                                           */
/*  Make a new avoid split assignments monitor with these attributes.        */
/*                                                                           */
/*****************************************************************************/

KHE_AVOID_SPLIT_ASSIGNMENTS_MONITOR KheAvoidSplitAssignmentsMonitorMake(
  KHE_SOLN soln, KHE_AVOID_SPLIT_ASSIGNMENTS_CONSTRAINT c, int eg_index)
{
  KHE_AVOID_SPLIT_ASSIGNMENTS_MONITOR res;  KHE_EVENT_RESOURCE_IN_SOLN ers;
  KHE_EVENT_IN_SOLN es;  KHE_EVENT_RESOURCE er;  int i, count;
  if( DEBUG1 )
    fprintf(stderr, "[ KheAvoidSplitAssignmentsMonitorMake(%s.%d)\n",
      KheConstraintId((KHE_CONSTRAINT) c) == NULL ? "-" :
      KheConstraintId((KHE_CONSTRAINT) c), eg_index);
  MMake(res);
  KheMonitorInitCommonFields((KHE_MONITOR) res, soln,
    KHE_AVOID_SPLIT_ASSIGNMENTS_MONITOR_TAG);
  res->constraint = c;
  res->eg_index = eg_index;
  MArrayInit(res->resources);
  MArrayInit(res->multiplicities);
  res->copy = NULL;

  /* add to (but not attach to) all the applicable event resources */
  count = KheAvoidSplitAssignmentsConstraintEventResourceCount(c,res->eg_index);
  for( i = 0;  i < count;  i++ )
  {
    er = KheAvoidSplitAssignmentsConstraintEventResource(c, res->eg_index, i);
    es = KheSolnEventInSoln(soln, KheEventIndex(KheEventResourceEvent(er)));
    ers = KheEventInSolnEventResourceInSoln(es,
      KheEventResourceIndexInEvent(er));
    KheEventResourceInSolnAddMonitor(ers, (KHE_MONITOR) res);
  }
  /* KheGroupMonitorAddMonitor((KHE_GROUP_MONITOR) soln, (KHE_MONITOR) res); */
  if( DEBUG1 )
    fprintf(stderr, "] KheAvoidSplitAssignmentsMonitorMake\n");
  return res;
}


/*****************************************************************************/
/*                                                                           */
/*  KHE_AVOID_SPLIT_ASSIGNMENTS_MONITOR                                      */
/*    KheAvoidSplitAssignmentsMonitorCopyPhase1(                             */
/*    KHE_AVOID_SPLIT_ASSIGNMENTS_MONITOR m)                                 */
/*                                                                           */
/*  Carry out Phase 1 of copying m.                                          */
/*                                                                           */
/*****************************************************************************/

KHE_AVOID_SPLIT_ASSIGNMENTS_MONITOR KheAvoidSplitAssignmentsMonitorCopyPhase1(
  KHE_AVOID_SPLIT_ASSIGNMENTS_MONITOR m)
{
  KHE_AVOID_SPLIT_ASSIGNMENTS_MONITOR copy;  int i;
  if( m->copy == NULL )
  {
    MMake(copy);
    m->copy = copy;
    KheMonitorCopyCommonFields((KHE_MONITOR) copy, (KHE_MONITOR) m);
    copy->constraint = m->constraint;
    copy->eg_index = m->eg_index;
    MArrayInit(copy->resources);
    MArrayAppend(copy->resources, m->resources, i);
    MArrayInit(copy->multiplicities);
    MArrayAppend(copy->multiplicities, m->multiplicities, i);
    copy->copy = NULL;
  }
  return m->copy;
}


/*****************************************************************************/
/*                                                                           */
/*  void KheAvoidSplitAssignmentsMonitorCopyPhase2(                          */
/*    KHE_AVOID_SPLIT_ASSIGNMENTS_MONITOR m)                                 */
/*                                                                           */
/*  Carry out Phase 2 of copying m.                                          */
/*                                                                           */
/*****************************************************************************/

void KheAvoidSplitAssignmentsMonitorCopyPhase2(
  KHE_AVOID_SPLIT_ASSIGNMENTS_MONITOR m)
{
  if( m->copy != NULL )
  {
    m->copy = NULL;
  }
}


/*****************************************************************************/
/*                                                                           */
/*  void KheAvoidSplitAssignmentsMonitorDelete(                              */
/*    KHE_AVOID_SPLIT_ASSIGNMENTS_MONITOR m)                                 */
/*                                                                           */
/*  Delete m.                                                                */
/*                                                                           */
/*****************************************************************************/

void KheAvoidSplitAssignmentsMonitorDelete(
  KHE_AVOID_SPLIT_ASSIGNMENTS_MONITOR m)
{
  int i, count;  KHE_EVENT_RESOURCE_IN_SOLN ers;  KHE_EVENT_IN_SOLN es;
  KHE_EVENT_RESOURCE er;  KHE_AVOID_SPLIT_ASSIGNMENTS_CONSTRAINT c;
  if( m->attached )
    KheAvoidSplitAssignmentsMonitorDetachFromSoln(m);
  if( m->parent_monitor != NULL )
    KheGroupMonitorDeleteChildMonitor(m->parent_monitor, (KHE_MONITOR) m);
  c = m->constraint;
  count = KheAvoidSplitAssignmentsConstraintEventResourceCount(c, m->eg_index);
  for( i = 0;  i < count;  i++ )
  {
    er = KheAvoidSplitAssignmentsConstraintEventResource(c, m->eg_index, i);
    es = KheSolnEventInSoln(m->soln, KheEventIndex(KheEventResourceEvent(er)));
    ers = KheEventInSolnEventResourceInSoln(es,
      KheEventResourceIndexInEvent(er));
    KheEventResourceInSolnDeleteMonitor(ers, (KHE_MONITOR) m);
  }
  KheSolnDeleteMonitor(m->soln, (KHE_MONITOR) m);
  MArrayFree(m->resources);
  MArrayFree(m->multiplicities);
  MFree(m);
}


/*****************************************************************************/
/*                                                                           */
/*  KHE_AVOID_SPLIT_ASSIGNMENTS_CONSTRAINT                                   */
/*    KheAvoidSplitAssignmentsMonitorConstraint(                             */
/*    KHE_AVOID_SPLIT_ASSIGNMENTS_MONITOR m)                                 */
/*                                                                           */
/*  Return the constraint that m is monitoring.                              */
/*                                                                           */
/*****************************************************************************/

KHE_AVOID_SPLIT_ASSIGNMENTS_CONSTRAINT
  KheAvoidSplitAssignmentsMonitorConstraint(
  KHE_AVOID_SPLIT_ASSIGNMENTS_MONITOR m)
{
  return m->constraint;
}


/*****************************************************************************/
/*                                                                           */
/*  int KheAvoidSplitAssignmentsMonitorEventGroupIndex(                      */
/*    KHE_AVOID_SPLIT_ASSIGNMENTS_MONITOR m)                                 */
/*                                                                           */
/*  Return the event group index of m.                                       */
/*                                                                           */
/*****************************************************************************/

int KheAvoidSplitAssignmentsMonitorEventGroupIndex(
  KHE_AVOID_SPLIT_ASSIGNMENTS_MONITOR m)
{
  return m->eg_index;
}


/*****************************************************************************/
/*                                                                           */
/*  int KheAvoidSplitAssignmentsMonitorResourceCount(                        */
/*    KHE_AVOID_SPLIT_ASSIGNMENTS_MONITOR m)                                 */
/*                                                                           */
/*  Return the number of distinct resources currently assigned to the tasks  */
/*  monitored by m.                                                          */
/*                                                                           */
/*****************************************************************************/

int KheAvoidSplitAssignmentsMonitorResourceCount(
  KHE_AVOID_SPLIT_ASSIGNMENTS_MONITOR m)
{
  return MArraySize(m->resources);
}


/*****************************************************************************/
/*                                                                           */
/*  KHE_RESOURCE KheAvoidSplitAssignmentsMonitorResource(                    */
/*    KHE_AVOID_SPLIT_ASSIGNMENTS_MONITOR m, int i)                          */
/*                                                                           */
/*  Return the i'th distinct resource current assigned to the tasks          */
/*  monitored by m.                                                          */
/*                                                                           */
/*****************************************************************************/

KHE_RESOURCE KheAvoidSplitAssignmentsMonitorResource(
  KHE_AVOID_SPLIT_ASSIGNMENTS_MONITOR m, int i)
{
  return MArrayGet(m->resources, i);
}


/*****************************************************************************/
/*                                                                           */
/*  int KheAvoidSplitAssignmentsMonitorResourceMultiplicity(                 */
/*    KHE_AVOID_SPLIT_ASSIGNMENTS_MONITOR m, int i)                          */
/*                                                                           */
/*  Return the number of tasks that the i'th distinct resource is assigned.  */
/*                                                                           */
/*****************************************************************************/

int KheAvoidSplitAssignmentsMonitorResourceMultiplicity(
  KHE_AVOID_SPLIT_ASSIGNMENTS_MONITOR m, int i)
{
  return MArrayGet(m->multiplicities, i);
}


/*****************************************************************************/
/*                                                                           */
/*  Submodule "attach and detach"                                            */
/*                                                                           */
/*****************************************************************************/

/*****************************************************************************/
/*                                                                           */
/*  void KheAvoidSplitAssignmentsMonitorAttachToSoln(                        */
/*    KHE_AVOID_SPLIT_ASSIGNMENTS_MONITOR m)                                 */
/*                                                                           */
/*  Attach m.  It is known to be currently detached with cost 0.             */
/*                                                                           */
/*****************************************************************************/

void KheAvoidSplitAssignmentsMonitorAttachToSoln(
  KHE_AVOID_SPLIT_ASSIGNMENTS_MONITOR m)
{
  int i, count;  KHE_EVENT_RESOURCE er;  KHE_EVENT_RESOURCE_IN_SOLN ers;
  KHE_EVENT_IN_SOLN es;
  m->attached = true;
  count = KheAvoidSplitAssignmentsConstraintEventResourceCount(m->constraint,
    m->eg_index);
  for( i = 0;  i < count;  i++ )
  {
    er = KheAvoidSplitAssignmentsConstraintEventResource(m->constraint,
      m->eg_index, i);
    es = KheSolnEventInSoln(KheMonitorSoln((KHE_MONITOR) m),
      KheEventIndex(KheEventResourceEvent(er)));
    ers = KheEventInSolnEventResourceInSoln(es,
      KheEventResourceIndexInEvent(er));
    KheEventResourceInSolnAttachMonitor(ers, (KHE_MONITOR) m);
  }
}


/*****************************************************************************/
/*                                                                           */
/*  void KheAvoidSplitAssignmentsMonitorDetachFromSoln(                      */
/*    KHE_AVOID_SPLIT_ASSIGNMENTS_MONITOR m)                                 */
/*                                                                           */
/*  Detach m.  It is known to be currently attached.                         */
/*                                                                           */
/*****************************************************************************/

void KheAvoidSplitAssignmentsMonitorDetachFromSoln(
  KHE_AVOID_SPLIT_ASSIGNMENTS_MONITOR m)
{
  int i, count;  KHE_EVENT_RESOURCE er;  KHE_EVENT_RESOURCE_IN_SOLN ers;
  KHE_EVENT_IN_SOLN es;
  count = KheAvoidSplitAssignmentsConstraintEventResourceCount(m->constraint,
    m->eg_index);
  for( i = 0; i < count; i++ )
  {
    er = KheAvoidSplitAssignmentsConstraintEventResource(m->constraint,
      m->eg_index, i);
    es = KheSolnEventInSoln(KheMonitorSoln((KHE_MONITOR) m),
      KheEventIndex(KheEventResourceEvent(er)));
    ers = KheEventInSolnEventResourceInSoln(es,
      KheEventResourceIndexInEvent(er));
    KheEventResourceInSolnDetachMonitor(ers, (KHE_MONITOR) m);
  }
  m->attached = false;
}


/*****************************************************************************/
/*                                                                           */
/*  void KheAvoidSplitAssignmentsMonitorAttachCheck(                         */
/*    KHE_AVOID_SPLIT_ASSIGNMENTS_MONITOR m)                                 */
/*                                                                           */
/*  Check the attachment of m.                                               */
/*                                                                           */
/*  This will ensure that m is detached from the solution if and only if     */
/*  every task that it monitors is assigned, directly or indirectly, to      */
/*  the same leader task.                                                    */
/*                                                                           */
/*****************************************************************************/

void KheAvoidSplitAssignmentsMonitorAttachCheck(
  KHE_AVOID_SPLIT_ASSIGNMENTS_MONITOR m)
{
  KHE_TASK task, leader_task;  int count, i, j;  KHE_EVENT_RESOURCE er;
  leader_task = NULL;
  count = KheAvoidSplitAssignmentsConstraintEventResourceCount(m->constraint,
    m->eg_index);
  for( i = 0; i < count; i++ )
  {
    er = KheAvoidSplitAssignmentsConstraintEventResource(m->constraint,
      m->eg_index, i);
    for( j = 0;  j < KheEventResourceTaskCount(m->soln, er);  j++ )
    {
      task = KheTaskLeader(KheEventResourceTask(m->soln, er, j));
      if( leader_task == NULL )
	leader_task = task;
      else if( leader_task != task )
      {
	/* distinct leaders; make sure m is attached, and exit */
	if( !KheMonitorAttachedToSoln((KHE_MONITOR) m) )
	  KheMonitorAttachToSoln((KHE_MONITOR) m);
	return;
      }
    }
  }

  /* no distinct leaders; make sure m is detached, and exit */
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
/*  void KheAvoidSplitAssignmentsMonitorAddTask(                             */
/*    KHE_AVOID_SPLIT_ASSIGNMENTS_MONITOR m, KHE_TASK task)                  */
/*                                                                           */
/*  Monitor the effect of adding task.                                       */
/*                                                                           */
/*****************************************************************************/

void KheAvoidSplitAssignmentsMonitorAddTask(
  KHE_AVOID_SPLIT_ASSIGNMENTS_MONITOR m, KHE_TASK task)
{
  KHE_RESOURCE r;
  r = KheTaskAsstResource(task);
  if( r != NULL )
  {
    /* multiplicy increases, just like assigning a resource */
    KheAvoidSplitAssignmentsMonitorAssignResource(m, task, r);
  }
}


/*****************************************************************************/
/*                                                                           */
/*  void KheAvoidSplitAssignmentsMonitorDeleteTask(                          */
/*    KHE_AVOID_SPLIT_ASSIGNMENTS_MONITOR m, KHE_TASK task)                  */
/*                                                                           */
/*  Monitor the effect of deleting task.                                     */
/*                                                                           */
/*****************************************************************************/

void KheAvoidSplitAssignmentsMonitorDeleteTask(
  KHE_AVOID_SPLIT_ASSIGNMENTS_MONITOR m, KHE_TASK task)
{
  KHE_RESOURCE r;
  r = KheTaskAsstResource(task);
  if( r != NULL )
  {
    /* multiplicy decreases, just like deleting a resource */
    KheAvoidSplitAssignmentsMonitorUnAssignResource(m, task, r);
  }
}


/*****************************************************************************/
/*                                                                           */
/*  void KheAvoidSplitAssignmentsMonitorSplitTask(                           */
/*    KHE_AVOID_SPLIT_ASSIGNMENTS_MONITOR m,                                 */
/*    KHE_TASK task1, KHE_TASK task2)                                        */
/*                                                                           */
/*  Let m know that a solution resource has just split into task1 and task2. */
/*  Either both solution resources are assigned the same resource, or        */
/*  they are both unassigned.                                                */
/*                                                                           */
/*****************************************************************************/

void KheAvoidSplitAssignmentsMonitorSplitTask(
  KHE_AVOID_SPLIT_ASSIGNMENTS_MONITOR m,
  KHE_TASK task1, KHE_TASK task2)
{
  KHE_RESOURCE r;
  r = KheTaskAsstResource(task2);
  if( r != NULL )
  {
    /* multiplicity increases, just like assigning a resource */
    KheAvoidSplitAssignmentsMonitorAssignResource(m, task2, r);
  }
}


/*****************************************************************************/
/*                                                                           */
/*  void KheAvoidSplitAssignmentsMonitorMergeTask(                           */
/*    KHE_AVOID_SPLIT_ASSIGNMENTS_MONITOR m,                                 */
/*    KHE_TASK task1, KHE_TASK task2)                                        */
/*                                                                           */
/*  Let m know that task1 and task2 are just about to be merged.             */
/*  Either both solution resources are assigned the same resource, or        */
/*  they are both unassigned.                                                */
/*                                                                           */
/*****************************************************************************/

void KheAvoidSplitAssignmentsMonitorMergeTask(
  KHE_AVOID_SPLIT_ASSIGNMENTS_MONITOR m,
  KHE_TASK task1, KHE_TASK task2)
{
  KHE_RESOURCE r;
  r = KheTaskAsstResource(task2);
  if( r != NULL )
  {
    /* multiplicy decreases, just like deleting a resource */
    KheAvoidSplitAssignmentsMonitorUnAssignResource(m, task2, r);
  }
}


/*****************************************************************************/
/*                                                                           */
/*  void KheAvoidSplitAssignmentsMonitorAssignResource(                      */
/*    KHE_AVOID_SPLIT_ASSIGNMENTS_MONITOR m,                                 */
/*    KHE_TASK task, KHE_RESOURCE r)                                         */
/*                                                                           */
/*  Let m know that task has just been assigned resource r.                  */
/*                                                                           */
/*****************************************************************************/

void KheAvoidSplitAssignmentsMonitorAssignResource(
  KHE_AVOID_SPLIT_ASSIGNMENTS_MONITOR m,
  KHE_TASK task, KHE_RESOURCE r)
{
  int pos;
  if( MArrayContains(m->resources, r, &pos) )
    MArrayInc(m->multiplicities, pos);
  else
  {
    MArrayAddLast(m->resources, r);
    MArrayAddLast(m->multiplicities, 1);
    if( MArraySize(m->resources) >= 2 )
      KheMonitorChangeCost((KHE_MONITOR) m,
	KheConstraintCost((KHE_CONSTRAINT) m->constraint,
	MArraySize(m->resources) - 1));
  }
}


/*****************************************************************************/
/*                                                                           */
/*  void KheAvoidSplitAssignmentsMonitorUnAssignResource(                    */
/*    KHE_AVOID_SPLIT_ASSIGNMENTS_MONITOR m,                                 */
/*    KHE_TASK task, KHE_RESOURCE r)                                         */
/*                                                                           */
/*  Let m know that task has just been unassigned resource r.                */
/*                                                                           */
/*****************************************************************************/

void KheAvoidSplitAssignmentsMonitorUnAssignResource(
  KHE_AVOID_SPLIT_ASSIGNMENTS_MONITOR m,
  KHE_TASK task, KHE_RESOURCE r)
{
  int pos, tmpm;  KHE_RESOURCE tmpr;
  if( !MArrayContains(m->resources, r, &pos) )
    MAssert(false,
      "KheAvoidSplitAssignmentsMonitorUnAssignResource internal error");
  if( MArrayGet(m->multiplicities, pos) >= 2 )
    MArrayDec(m->multiplicities, pos);
  else
  {
    tmpr = MArrayRemoveLast(m->resources);
    tmpm = MArrayRemoveLast(m->multiplicities);
    if( tmpr != r )
    {
      MArrayPut(m->resources, pos, tmpr);
      MArrayPut(m->multiplicities, pos, tmpm);
    }
    if( MArraySize(m->resources) >= 1 )
      KheMonitorChangeCost((KHE_MONITOR) m,
	KheConstraintCost((KHE_CONSTRAINT) m->constraint,
	MArraySize(m->resources) - 1));
  }
}


/*****************************************************************************/
/*                                                                           */
/*  Submodule "deviations"                                                   */
/*                                                                           */
/*****************************************************************************/

/*****************************************************************************/
/*                                                                           */
/*  int KheAvoidSplitAssignmentsMonitorDeviationCount(                       */
/*    KHE_AVOID_SPLIT_ASSIGNMENTS_MONITOR m)                                 */
/*                                                                           */
/*  Return the number of deviations of m.                                    */
/*                                                                           */
/*****************************************************************************/

int KheAvoidSplitAssignmentsMonitorDeviationCount(
  KHE_AVOID_SPLIT_ASSIGNMENTS_MONITOR m)
{
  return 1;
}


/*****************************************************************************/
/*                                                                           */
/*  int KheAvoidSplitAssignmentsMonitorDeviation(                            */
/*    KHE_AVOID_SPLIT_ASSIGNMENTS_MONITOR m, int i)                          */
/*                                                                           */
/*  Return the i'th deviation of m.                                          */
/*                                                                           */
/*****************************************************************************/

int KheAvoidSplitAssignmentsMonitorDeviation(
  KHE_AVOID_SPLIT_ASSIGNMENTS_MONITOR m, int i)
{
  MAssert(i == 0, "KheAvoidSplitAssignmentsMonitorDeviation: i out of range");
  return MArraySize(m->resources) >= 2 ? MArraySize(m->resources) - 1 : 0;
}


/*****************************************************************************/
/*                                                                           */
/*  char *KheAvoidSplitAssignmentsMonitorDeviationDescription(               */
/*    KHE_AVOID_SPLIT_ASSIGNMENTS_MONITOR m, int i)                          */
/*                                                                           */
/*  Return a description of the i'th deviation of m.                         */
/*                                                                           */
/*****************************************************************************/

char *KheAvoidSplitAssignmentsMonitorDeviationDescription(
  KHE_AVOID_SPLIT_ASSIGNMENTS_MONITOR m, int i)
{
  MAssert(i == 0,
    "KheAvoidSplitAssignmentsMonitorDeviationDescription: i out of range");
  return NULL;
}


/*****************************************************************************/
/*                                                                           */
/*  Submodule "debug"                                                        */
/*                                                                           */
/*****************************************************************************/

/*****************************************************************************/
/*                                                                           */
/*  void KheAvoidSplitAssignmentsMonitorDebug(                               */
/*    KHE_AVOID_SPLIT_ASSIGNMENTS_MONITOR m, int verbosity,                  */
/*    int indent, FILE *fp)                                                  */
/*                                                                           */
/*  Debug print of m onto fp with the given verbosity and indent.            */
/*                                                                           */
/*****************************************************************************/

void KheAvoidSplitAssignmentsMonitorDebug(
  KHE_AVOID_SPLIT_ASSIGNMENTS_MONITOR m, int verbosity, int indent, FILE *fp)
{
  char *id;  KHE_RESOURCE r;  int i, count;  KHE_EVENT_RESOURCE er;
  if( verbosity >= 1 )
  {
    KheMonitorDebugBegin((KHE_MONITOR) m, indent, fp);
    if( verbosity >= 3 )
    {
      id = KheConstraintId((KHE_CONSTRAINT) m->constraint);
      fprintf(fp, " %s.%d", id != NULL ? id : "-", m->eg_index);
    }
    count = KheAvoidSplitAssignmentsConstraintEventResourceCount(m->constraint,
      m->eg_index);
    MAssert(count >= 1, "KheAvoidSplitAssignmentsMonitorDebug internal error");
    er = KheAvoidSplitAssignmentsConstraintEventResource(m->constraint,
      m->eg_index, 0);
    fprintf(fp, " ");
    KheEventResourceDebug(er, 1, -1, fp);
    fprintf(fp, " := (");
    for( i = 0;  i < MArraySize(m->resources);  i++ )
    {
      if( i > 0 )
	fprintf(fp, ", ");
      r = MArrayGet(m->resources, i);
      fprintf(fp, "%d %s", MArrayGet(m->multiplicities, i),
	KheResourceId(r) != NULL ? KheResourceId(r) : "-");
    }
    fprintf(fp, ")");
    KheMonitorDebugEnd((KHE_MONITOR) m, true, indent, fp);
  }
}
