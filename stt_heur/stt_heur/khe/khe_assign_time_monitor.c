
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
/*  FILE:         khe_assign_time_monitor.c                                  */
/*  DESCRIPTION:  An assign time monitor                                     */
/*                                                                           */
/*****************************************************************************/
#include "khe_interns.h"


/*****************************************************************************/
/*                                                                           */
/*  KHE_ASSIGN_TIME_MONITOR - an assign time monitor                         */
/*                                                                           */
/*****************************************************************************/

struct khe_assign_time_monitor_rec {

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

  /* specific to KHE_ASSIGN_TIME_MONITOR */
  int				deviation;		/* unassigned durn   */
  KHE_EVENT_IN_SOLN		event_in_soln;		/* enclosing es      */
  KHE_ASSIGN_TIME_CONSTRAINT	constraint;		/* constraint        */
  KHE_ASSIGN_TIME_MONITOR	copy;			/* used when copying */
};


/*****************************************************************************/
/*                                                                           */
/*  Submodule "construction and query"                                       */
/*                                                                           */
/*****************************************************************************/

/*****************************************************************************/
/*                                                                           */
/*  KHE_ASSIGN_TIME_MONITOR KheAssignTimeMonitorMake(KHE_EVENT_IN_SOLN es,   */
/*    KHE_ASSIGN_TIME_CONSTRAINT c)                                          */
/*                                                                           */
/*  Make a new assign time monitor with these attributes and add it to es.   */
/*                                                                           */
/*****************************************************************************/

KHE_ASSIGN_TIME_MONITOR KheAssignTimeMonitorMake(KHE_EVENT_IN_SOLN es,
  KHE_ASSIGN_TIME_CONSTRAINT c)
{
  KHE_ASSIGN_TIME_MONITOR res;  KHE_SOLN soln;
  soln = KheEventInSolnSoln(es);
  MMake(res);
  KheMonitorInitCommonFields((KHE_MONITOR) res, soln,
    KHE_ASSIGN_TIME_MONITOR_TAG);
  res->deviation = 0;
  res->event_in_soln = es;
  res->constraint = c;
  res->copy = NULL;
  KheEventInSolnAddMonitor(es, (KHE_MONITOR) res);
  return res;
}


/*****************************************************************************/
/*                                                                           */
/*  KHE_ASSIGN_TIME_MONITOR KheAssignTimeMonitorCopyPhase1(                  */
/*    KHE_ASSIGN_TIME_MONITOR m)                                             */
/*                                                                           */
/*  Carry out Phase 1 of copying m.                                          */
/*                                                                           */
/*****************************************************************************/

KHE_ASSIGN_TIME_MONITOR KheAssignTimeMonitorCopyPhase1(
  KHE_ASSIGN_TIME_MONITOR m)
{
  KHE_ASSIGN_TIME_MONITOR copy;
  if( m->copy == NULL )
  {
    MMake(copy);
    m->copy = copy;
    KheMonitorCopyCommonFields((KHE_MONITOR) copy, (KHE_MONITOR) m);
    copy->deviation = m->deviation;
    copy->event_in_soln = KheEventInSolnCopyPhase1(m->event_in_soln);
    copy->constraint = m->constraint;
    copy->copy = NULL;
  }
  return m->copy;
}


/*****************************************************************************/
/*                                                                           */
/*  void KheAssignTimeMonitorCopyPhase2(KHE_ASSIGN_TIME_MONITOR m)           */
/*                                                                           */
/*  Carry out Phase 2 of copying m.                                          */
/*                                                                           */
/*****************************************************************************/

void KheAssignTimeMonitorCopyPhase2(KHE_ASSIGN_TIME_MONITOR m)
{
  if( m->copy != NULL )
  {
    m->copy = NULL;
  }
}


/*****************************************************************************/
/*                                                                           */
/*  void KheAssignTimeMonitorDelete(KHE_ASSIGN_TIME_MONITOR m)               */
/*                                                                           */
/*  Delete m.                                                                */
/*                                                                           */
/*****************************************************************************/

void KheAssignTimeMonitorDelete(KHE_ASSIGN_TIME_MONITOR m)
{
  if( m->attached )
    KheAssignTimeMonitorDetachFromSoln(m);
  if( m->parent_monitor != NULL )
    KheGroupMonitorDeleteChildMonitor(m->parent_monitor, (KHE_MONITOR) m);
  KheEventInSolnDeleteMonitor(m->event_in_soln, (KHE_MONITOR) m);
  KheSolnDeleteMonitor(m->soln, (KHE_MONITOR) m);
  MFree(m);
}


/*****************************************************************************/
/*                                                                           */
/*  KHE_ASSIGN_TIME_CONSTRAINT KheAssignTimeMonitorConstraint(               */
/*    KHE_ASSIGN_TIME_MONITOR m)                                             */
/*                                                                           */
/*  Return the constraint that m is monitoring.                              */
/*                                                                           */
/*****************************************************************************/

KHE_ASSIGN_TIME_CONSTRAINT KheAssignTimeMonitorConstraint(
  KHE_ASSIGN_TIME_MONITOR m)
{
  return m->constraint;
}


/*****************************************************************************/
/*                                                                           */
/*  KHE_EVENT KheAssignTimeMonitorEvent(KHE_ASSIGN_TIME_MONITOR m)           */
/*                                                                           */
/*  Return the event that m is monitoring.                                   */
/*                                                                           */
/*****************************************************************************/

KHE_EVENT KheAssignTimeMonitorEvent(KHE_ASSIGN_TIME_MONITOR m)
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
/*  void KheAssignTimeMonitorAttachToSoln(KHE_ASSIGN_TIME_MONITOR m)         */
/*                                                                           */
/*  Attach m.  It is known to be currently detached with cost 0.             */
/*                                                                           */
/*****************************************************************************/

void KheAssignTimeMonitorAttachToSoln(KHE_ASSIGN_TIME_MONITOR m)
{
  m->attached = true;
  KheEventInSolnAttachMonitor(m->event_in_soln, (KHE_MONITOR) m);
}


/*****************************************************************************/
/*                                                                           */
/*  void KheAssignTimeMonitorDetachFromSoln(KHE_ASSIGN_TIME_MONITOR m)       */
/*                                                                           */
/*  Detach m.  It is known to be currently attached.                         */
/*                                                                           */
/*****************************************************************************/

void KheAssignTimeMonitorDetachFromSoln(KHE_ASSIGN_TIME_MONITOR m)
{
  KheEventInSolnDetachMonitor(m->event_in_soln, (KHE_MONITOR) m);
  m->attached = false;
}


/*****************************************************************************/
/*                                                                           */
/*  void KheAssignTimeMonitorAttachCheck(KHE_ASSIGN_TIME_MONITOR m)          */
/*                                                                           */
/*  Check the attachment of m.                                               */
/*                                                                           */
/*****************************************************************************/

void KheAssignTimeMonitorAttachCheck(KHE_ASSIGN_TIME_MONITOR m)
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
/*  void KheAssignTimeMonitorAddMeet(KHE_ASSIGN_TIME_MONITOR m,              */
/*    KHE_MEET meet)                                                         */
/*                                                                           */
/*  Monitor the effect of adding meet.                                       */
/*                                                                           */
/*****************************************************************************/

void KheAssignTimeMonitorAddMeet(KHE_ASSIGN_TIME_MONITOR m, KHE_MEET meet)
{
  if( KheMeetAssignedTimeIndex(meet) == -1 )
  {
    m->deviation += KheMeetDuration(meet);
    KheMonitorChangeCost((KHE_MONITOR) m,
      KheConstraintCost((KHE_CONSTRAINT) m->constraint, m->deviation));
  }
}


/*****************************************************************************/
/*                                                                           */
/*  void KheAssignTimeMonitorDeleteMeet(KHE_ASSIGN_TIME_MONITOR m,           */
/*    KHE_MEET meet)                                                         */
/*                                                                           */
/*  Monitor the effect of deleting meet.                                     */
/*                                                                           */
/*****************************************************************************/

void KheAssignTimeMonitorDeleteMeet(KHE_ASSIGN_TIME_MONITOR m, KHE_MEET meet)
{
  if( KheMeetAssignedTimeIndex(meet) == -1 )
  {
    m->deviation -= KheMeetDuration(meet);
    MAssert(m->deviation >= 0,
      "KheAssignTimeMonitorDeleteMeet internal error");
    KheMonitorChangeCost((KHE_MONITOR) m,
      KheConstraintCost((KHE_CONSTRAINT) m->constraint, m->deviation));
  }
}


/*****************************************************************************/
/*                                                                           */
/*  void KheAssignTimeMonitorSplitMeet(KHE_ASSIGN_TIME_MONITOR m,            */
/*    KHE_MEET meet1, KHE_MEET meet2)                                        */
/*                                                                           */
/*  Let m know that a meet has just split into meet1 and meet2.              */
/*  Either both meets are assigned times, or they aren't.                    */
/*                                                                           */
/*****************************************************************************/

void KheAssignTimeMonitorSplitMeet(KHE_ASSIGN_TIME_MONITOR m,
  KHE_MEET meet1, KHE_MEET meet2)
{
  /* no change in cost is possible */
}


/*****************************************************************************/
/*                                                                           */
/*  void KheAssignTimeMonitorMergeMeet(KHE_ASSIGN_TIME_MONITOR m,            */
/*    KHE_MEET meet1, KHE_MEET meet2)                                        */
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

void KheAssignTimeMonitorMergeMeet(KHE_ASSIGN_TIME_MONITOR m,
  KHE_MEET meet1, KHE_MEET meet2)
{
  /* no change in cost is possible */
}


/*****************************************************************************/
/*                                                                           */
/*  void KheAssignTimeMonitorAssignTime(KHE_ASSIGN_TIME_MONITOR m,           */
/*    KHE_MEET meet, int assigned_time_index)                                */
/*                                                                           */
/*  Let m know that meet has just been assigned the time with this index.    */
/*                                                                           */
/*****************************************************************************/

void KheAssignTimeMonitorAssignTime(KHE_ASSIGN_TIME_MONITOR m,
  KHE_MEET meet, int assigned_time_index)
{
  m->deviation -= KheMeetDuration(meet);
  MAssert(m->deviation >= 0,
    "KheAssignTimeMonitorAssignTime internal error");
  KheMonitorChangeCost((KHE_MONITOR) m,
    KheConstraintCost((KHE_CONSTRAINT) m->constraint, m->deviation));
}


/*****************************************************************************/
/*                                                                           */
/*  void KheAssignTimeMonitorUnAssignTime(KHE_ASSIGN_TIME_MONITOR m,         */
/*    KHE_MEET meet, int assigned_time_index)                                */
/*                                                                           */
/*  Let m know that meet is about to be unassigned the time with this index. */
/*                                                                           */
/*****************************************************************************/

void KheAssignTimeMonitorUnAssignTime(KHE_ASSIGN_TIME_MONITOR m,
  KHE_MEET meet, int assigned_time_index)
{
  m->deviation += KheMeetDuration(meet);
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
/*  int KheAssignTimeMonitorDeviationCount(KHE_ASSIGN_TIME_MONITOR m)        */
/*                                                                           */
/*  Return the number of deviations of m.                                    */
/*                                                                           */
/*****************************************************************************/

int KheAssignTimeMonitorDeviationCount(KHE_ASSIGN_TIME_MONITOR m)
{
  return 1;
}


/*****************************************************************************/
/*                                                                           */
/*  int KheAssignTimeMonitorDeviation(KHE_ASSIGN_TIME_MONITOR m, int i)      */
/*                                                                           */
/*  Return the i'th deviation of m.                                          */
/*                                                                           */
/*****************************************************************************/

int KheAssignTimeMonitorDeviation(KHE_ASSIGN_TIME_MONITOR m, int i)
{
  MAssert(i == 0, "KheAssignTimeMonitorDeviation: i out of range");
  return m->deviation;
}


/*****************************************************************************/
/*                                                                           */
/*  char *KheAssignTimeMonitorDeviationDescription(KHE_ASSIGN_TIME_MONITOR m,*/
/*    int i)                                                                 */
/*                                                                           */
/*  Return a description of the i'th deviation.                              */
/*                                                                           */
/*****************************************************************************/

char *KheAssignTimeMonitorDeviationDescription(KHE_ASSIGN_TIME_MONITOR m,
  int i)
{
  MAssert(i == 0, "KheAssignTimeMonitorDeviationDescription: i out of range");
  return NULL;
}


/*****************************************************************************/
/*                                                                           */
/*  Submodule "debug"                                                        */
/*                                                                           */
/*****************************************************************************/

/*****************************************************************************/
/*                                                                           */
/*  void KheAssignTimeMonitorDebug(KHE_ASSIGN_TIME_MONITOR m,                */
/*    int verbosity, int indent, FILE *fp)                                   */
/*                                                                           */
/*  Debug print of m onto fp with the given verbosity and indent.            */
/*                                                                           */
/*****************************************************************************/

void KheAssignTimeMonitorDebug(KHE_ASSIGN_TIME_MONITOR m,
  int verbosity, int indent, FILE *fp)
{
  if( verbosity >= 1 )
  {
    KheMonitorDebugBegin((KHE_MONITOR) m, indent, fp);
    fprintf(fp, " ");
    KheEventInSolnDebug(m->event_in_soln, 1, -1, fp);
    KheMonitorDebugEnd((KHE_MONITOR) m, true, indent, fp);
  }
}
