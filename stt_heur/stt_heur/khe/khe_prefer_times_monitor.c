
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
/*  FILE:         khe_prefer_times_monitor.c                                 */
/*  DESCRIPTION:  An prefer times monitor                                    */
/*                                                                           */
/*****************************************************************************/
#include "khe_interns.h"


/*****************************************************************************/
/*                                                                           */
/*  KHE_PREFER_TIMES_MONITOR - an prefer times monitor                       */
/*                                                                           */
/*****************************************************************************/

struct khe_prefer_times_monitor_rec {

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

  /* specific to KHE_PREFER_TIMES_MONITOR */
  int				deviation;		/* deviation         */
  KHE_EVENT_IN_SOLN		event_in_soln;		/* enclosing es      */
  KHE_PREFER_TIMES_CONSTRAINT	constraint;		/* constraint        */
  KHE_PREFER_TIMES_MONITOR	copy;			/* used when copying */
};


/*****************************************************************************/
/*                                                                           */
/*  Submodule "construction and query"                                       */
/*                                                                           */
/*****************************************************************************/

/*****************************************************************************/
/*                                                                           */
/*  KHE_PREFER_TIMES_MONITOR KhePreferTimesMonitorMake(KHE_EVENT_IN_SOLN es, */
/*    KHE_PREFER_TIMES_CONSTRAINT c)                                         */
/*                                                                           */
/*  Make a new prefer times monitor with these attributes.                   */
/*                                                                           */
/*****************************************************************************/

KHE_PREFER_TIMES_MONITOR KhePreferTimesMonitorMake(KHE_EVENT_IN_SOLN es,
  KHE_PREFER_TIMES_CONSTRAINT c)
{
  KHE_PREFER_TIMES_MONITOR res;  KHE_SOLN soln;
  soln = KheEventInSolnSoln(es);
  MMake(res);
  KheMonitorInitCommonFields((KHE_MONITOR) res, soln,
    KHE_PREFER_TIMES_MONITOR_TAG);
  res->deviation = 0;
  res->event_in_soln = es;
  res->constraint = c;
  res->copy = NULL;
  KheEventInSolnAddMonitor(es, (KHE_MONITOR) res);
  return res;
}


/*****************************************************************************/
/*                                                                           */
/*  KHE_PREFER_TIMES_MONITOR KhePreferTimesMonitorCopyPhase1(                */
/*    KHE_PREFER_TIMES_MONITOR m)                                            */
/*                                                                           */
/*  Carry out Phase 1 of copying m.                                          */
/*                                                                           */
/*****************************************************************************/

KHE_PREFER_TIMES_MONITOR KhePreferTimesMonitorCopyPhase1(
  KHE_PREFER_TIMES_MONITOR m)
{
  KHE_PREFER_TIMES_MONITOR copy;
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
/*  void KhePreferTimesMonitorCopyPhase2(KHE_PREFER_TIMES_MONITOR m)         */
/*                                                                           */
/*  Carry out Phase 2 of copying m.                                          */
/*                                                                           */
/*****************************************************************************/

void KhePreferTimesMonitorCopyPhase2(KHE_PREFER_TIMES_MONITOR m)
{
  if( m->copy != NULL )
  {
    m->copy = NULL;
  }
}


/*****************************************************************************/
/*                                                                           */
/*  void KhePreferTimesMonitorDelete(KHE_PREFER_TIMES_MONITOR m)             */
/*                                                                           */
/*  Free m.                                                                  */
/*                                                                           */
/*****************************************************************************/

void KhePreferTimesMonitorDelete(KHE_PREFER_TIMES_MONITOR m)
{
  if( m->attached )
    KhePreferTimesMonitorDetachFromSoln(m);
  if( m->parent_monitor != NULL )
    KheGroupMonitorDeleteChildMonitor(m->parent_monitor, (KHE_MONITOR) m);
  KheEventInSolnDeleteMonitor(m->event_in_soln, (KHE_MONITOR) m);
  KheSolnDeleteMonitor(m->soln, (KHE_MONITOR) m);
  MFree(m);
}


/*****************************************************************************/
/*                                                                           */
/*  KHE_PREFER_TIMES_CONSTRAINT KhePreferTimesMonitorConstraint(             */
/*    KHE_PREFER_TIMES_MONITOR m)                                            */
/*                                                                           */
/*  Return the constraint that m is monitoring.                              */
/*                                                                           */
/*****************************************************************************/

KHE_PREFER_TIMES_CONSTRAINT KhePreferTimesMonitorConstraint(
  KHE_PREFER_TIMES_MONITOR m)
{
  return m->constraint;
}


/*****************************************************************************/
/*                                                                           */
/*  KHE_EVENT KhePreferTimesMonitorEvent(KHE_PREFER_TIMES_MONITOR m)         */
/*                                                                           */
/*  Return the event that m is monitoring.                                   */
/*                                                                           */
/*****************************************************************************/

KHE_EVENT KhePreferTimesMonitorEvent(KHE_PREFER_TIMES_MONITOR m)
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
/*  void KhePreferTimesMonitorAttachToSoln(KHE_PREFER_TIMES_MONITOR m)       */
/*                                                                           */
/*  Attach m.  It is known to be currently detached with cost 0.             */
/*                                                                           */
/*****************************************************************************/

void KhePreferTimesMonitorAttachToSoln(KHE_PREFER_TIMES_MONITOR m)
{
  m->attached = true;
  KheEventInSolnAttachMonitor(m->event_in_soln, (KHE_MONITOR) m);
}


/*****************************************************************************/
/*                                                                           */
/*  void KhePreferTimesMonitorDetachFromSoln(KHE_PREFER_TIMES_MONITOR m)     */
/*                                                                           */
/*  Detach m.  It is known to be currently attached.                         */
/*                                                                           */
/*****************************************************************************/

void KhePreferTimesMonitorDetachFromSoln(KHE_PREFER_TIMES_MONITOR m)
{
  KheEventInSolnDetachMonitor(m->event_in_soln, (KHE_MONITOR) m);
  m->attached = false;
}


/*****************************************************************************/
/*                                                                           */
/*  bool PreferTimesMonitorMustHaveZeroCost(KHE_PREFER_TIMES_MONITOR m)      */
/*                                                                           */
/*  Return true if a comparison of the domains of the meets monitored by     */
/*  m with its constraint shows that this monitor must have cost 0.          */
/*                                                                           */
/*****************************************************************************/

static bool KhePreferTimesMonitorMustHaveZeroCost(KHE_PREFER_TIMES_MONITOR m)
{
  KHE_MEET meet;  KHE_TIME_GROUP tg;  int i;  KHE_EVENT e;
  KHE_PREFER_TIMES_CONSTRAINT c;
  c = KhePreferTimesMonitorConstraint(m);
  tg = KhePreferTimesConstraintDomain(c);
  e = KhePreferTimesMonitorEvent(m);
  for( i = 0;  i < KheEventMeetCount(m->soln, e);  i++ )
  {
    meet = KheEventMeet(m->soln, e, i);
    if( KhePreferTimesConstraintDuration(c) == KHE_NO_DURATION ||
	KhePreferTimesConstraintDuration(c) == KheMeetDuration(meet) )
    {
      /* m is applicable to meet; nb domain cannot be NULL here */
      if( !KheTimeGroupSubset(KheMeetDomain(meet), tg) )
	return false;
    }
  }
  return true;
}


/*****************************************************************************/
/*                                                                           */
/*  void KhePreferTimesMonitorAttachCheck(KHE_PREFER_TIMES_MONITOR m)        */
/*                                                                           */
/*  Check the attachment of m.                                               */
/*                                                                           */
/*****************************************************************************/

void KhePreferTimesMonitorAttachCheck(KHE_PREFER_TIMES_MONITOR m)
{
  if( KhePreferTimesMonitorMustHaveZeroCost(m) )
  {
    if( KheMonitorAttachedToSoln((KHE_MONITOR) m) )
      KheMonitorDetachFromSoln((KHE_MONITOR) m);
  }
  else
  {
    if( !KheMonitorAttachedToSoln((KHE_MONITOR) m) )
      KheMonitorAttachToSoln((KHE_MONITOR) m);
  }
}


/*****************************************************************************/
/*                                                                           */
/*  Submodule "monitoring calls"                                             */
/*                                                                           */
/*****************************************************************************/

/*****************************************************************************/
/*                                                                           */
/*  bool KhePreferTimesMonitorApplies(KHE_PREFER_TIMES_MONITOR m, int durn)  */
/*                                                                           */
/*  Return true if m is interested in meets of duration durn.                */
/*                                                                           */
/*****************************************************************************/

static bool KhePreferTimesMonitorApplies(KHE_PREFER_TIMES_MONITOR m, int durn)
{
  int d;
  d = KhePreferTimesConstraintDuration(m->constraint);
  return d == durn || d == KHE_NO_DURATION;
}


/*****************************************************************************/
/*                                                                           */
/*  bool KhePreferTimesMonitorWrongTime(KHE_PREFER_TIMES_MONITOR m,          */
/*    KHE_TIME t)                                                            */
/*                                                                           */
/*  Return true when assigning t should attract a cost.                      */
/*                                                                           */
/*****************************************************************************/

static bool KhePreferTimesMonitorWrongTime(KHE_PREFER_TIMES_MONITOR m,
  KHE_TIME t)
{
  return t != NULL && !KheTimeGroupContains(
    KhePreferTimesConstraintDomain(m->constraint), t);
}


/*****************************************************************************/
/*                                                                           */
/*  bool KhePreferTimesMonitorWrongTimeIndex(KHE_PREFER_TIMES_MONITOR m,     */
/*    int assigned_time_index)                                               */
/*                                                                           */
/*  Like KhePreferTimesMonitorWrongTime except we are sure that there is     */
/*  a time, and we are given its index.                                      */
/*                                                                           */
/*****************************************************************************/

static bool KhePreferTimesMonitorWrongTimeIndex(KHE_PREFER_TIMES_MONITOR m,
  int assigned_time_index)
{
  return !KheTimeGroupContainsIndex(
    KhePreferTimesConstraintDomain(m->constraint), assigned_time_index);
}


/*****************************************************************************/
/*                                                                           */
/*  void KhePreferTimesMonitorAddMeet(KHE_PREFER_TIMES_MONITOR m,            */
/*    KHE_MEET meet)                                                         */
/*                                                                           */
/*  Monitor the effect of adding meet.                                       */
/*                                                                           */
/*****************************************************************************/

void KhePreferTimesMonitorAddMeet(KHE_PREFER_TIMES_MONITOR m, KHE_MEET meet)
{
  int durn;
  durn = KheMeetDuration(meet);
  if( KhePreferTimesMonitorApplies(m, durn) &&
      KhePreferTimesMonitorWrongTime(m, KheMeetAsstTime(meet)) )
  {
    m->deviation += durn;
    KheMonitorChangeCost((KHE_MONITOR) m,
      KheConstraintCost((KHE_CONSTRAINT) m->constraint, m->deviation));
  }
}


/*****************************************************************************/
/*                                                                           */
/*  void KhePreferTimesMonitorDeleteMeet(KHE_PREFER_TIMES_MONITOR m,         */
/*    KHE_MEET meet)                                                         */
/*                                                                           */
/*  Monitor the effect of deleting meet.                                     */
/*                                                                           */
/*****************************************************************************/

void KhePreferTimesMonitorDeleteMeet(KHE_PREFER_TIMES_MONITOR m, KHE_MEET meet)
{
  int durn;
  durn = KheMeetDuration(meet);
  if( KhePreferTimesMonitorApplies(m, durn) &&
      KhePreferTimesMonitorWrongTime(m, KheMeetAsstTime(meet)) )
  {
    m->deviation -= durn;
    MAssert(m->deviation >= 0,
      "KhePreferTimesMonitorDeleteMeet internal error");
    KheMonitorChangeCost((KHE_MONITOR) m,
      KheConstraintCost((KHE_CONSTRAINT) m->constraint, m->deviation));
  }
}


/*****************************************************************************/
/*                                                                           */
/*  void KhePreferTimesMonitorSplitMeet(KHE_PREFER_TIMES_MONITOR m,          */
/*    KHE_MEET meet1, KHE_MEET meet2)                                        */
/*                                                                           */
/*  Let m know that a meet has just split into meet1 and meet2.              */
/*  Either both meets are assigned times, or they aren't.                    */
/*                                                                           */
/*****************************************************************************/

void KhePreferTimesMonitorSplitMeet(KHE_PREFER_TIMES_MONITOR m,
  KHE_MEET meet1, KHE_MEET meet2)
{
  int durn1, durn2;
  if( KheMeetAsstTime(meet1) != NULL )
  {
    durn1 = KheMeetDuration(meet1);
    durn2 = KheMeetDuration(meet2);

    /* remove old deviation */
    if( KhePreferTimesMonitorApplies(m, durn1 + durn2) &&
	KhePreferTimesMonitorWrongTime(m, KheMeetAsstTime(meet1)) )
      m->deviation -= durn1 + durn2;

    /* add meet1 deviation */
    if( KhePreferTimesMonitorApplies(m, durn1) &&
	KhePreferTimesMonitorWrongTime(m, KheMeetAsstTime(meet1)) )
      m->deviation += durn1;

    /* add meet2 deviation */
    if( KhePreferTimesMonitorApplies(m, durn2) &&
	KhePreferTimesMonitorWrongTime(m, KheMeetAsstTime(meet2)) )
      m->deviation += durn2;

    /* report any change in cost */
    MAssert(m->deviation >= 0,
      "KhePreferTimesMonitorSplitMeet internal error");
    KheMonitorChangeCost((KHE_MONITOR) m,
      KheConstraintCost((KHE_CONSTRAINT) m->constraint, m->deviation));
  }
}


/*****************************************************************************/
/*                                                                           */
/*  void KhePreferTimesMonitorMergeMeet(KHE_PREFER_TIMES_MONITOR m,          */
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

void KhePreferTimesMonitorMergeMeet(KHE_PREFER_TIMES_MONITOR m,
  KHE_MEET meet1, KHE_MEET meet2)
{
  int durn1, durn2;
  if( KheMeetAsstTime(meet1) != NULL )
  {
    durn1 = KheMeetDuration(meet1);
    durn2 = KheMeetDuration(meet2);

    /* remove meet1 cost */
    if( KhePreferTimesMonitorApplies(m, durn1) &&
	KhePreferTimesMonitorWrongTime(m, KheMeetAsstTime(meet1)) )
      m->deviation -= durn1;

    /* remove meet2 cost */
    if( KhePreferTimesMonitorApplies(m, durn2) &&
	KhePreferTimesMonitorWrongTime(m, KheMeetAsstTime(meet2)) )
      m->deviation -= durn2;

    /* add merged cost */
    if( KhePreferTimesMonitorApplies(m, durn1 + durn2) &&
	KhePreferTimesMonitorWrongTime(m, KheMeetAsstTime(meet1)) )
      m->deviation += durn1 + durn2;

    /* report any change in cost */
    MAssert(m->deviation >= 0,
      "KhePreferTimesMonitorMergeMeet internal error");
    KheMonitorChangeCost((KHE_MONITOR) m,
      KheConstraintCost((KHE_CONSTRAINT) m->constraint, m->deviation));
  }
}


/*****************************************************************************/
/*                                                                           */
/*  void KhePreferTimesMonitorAssignTime(KHE_PREFER_TIMES_MONITOR m,         */
/*    KHE_MEET meet, int assigned_time_index)                                */
/*                                                                           */
/*  Let m know that meet has just been assigned the time with this index.    */
/*                                                                           */
/*****************************************************************************/

void KhePreferTimesMonitorAssignTime(KHE_PREFER_TIMES_MONITOR m,
  KHE_MEET meet, int assigned_time_index)
{
  int durn;
  durn = KheMeetDuration(meet);
  if( KhePreferTimesMonitorApplies(m, durn) &&
      KhePreferTimesMonitorWrongTimeIndex(m, assigned_time_index) )
  {
    m->deviation += durn;
    KheMonitorChangeCost((KHE_MONITOR) m,
      KheConstraintCost((KHE_CONSTRAINT) m->constraint, m->deviation));
  }
}


/*****************************************************************************/
/*                                                                           */
/*  void KhePreferTimesMonitorUnAssignTime(KHE_PREFER_TIMES_MONITOR m,       */
/*    KHE_MEET meet, int assigned_time_index)                                */
/*                                                                           */
/*  Let m know that meet is about to be unassigned the time with this index. */
/*                                                                           */
/*****************************************************************************/

void KhePreferTimesMonitorUnAssignTime(KHE_PREFER_TIMES_MONITOR m,
  KHE_MEET meet, int assigned_time_index)
{
  int durn;
  durn = KheMeetDuration(meet);
  if( KhePreferTimesMonitorApplies(m, durn) &&
      KhePreferTimesMonitorWrongTimeIndex(m, assigned_time_index) )
  {
    m->deviation -= durn;
    MAssert(m->deviation >= 0,
      "KhePreferTimesMonitorUnAssignTime internal error");
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
/*  int KhePreferTimesMonitorDeviationCount(KHE_PREFER_TIMES_MONITOR m)      */
/*                                                                           */
/*  Return the number of deviations of m.                                    */
/*                                                                           */
/*****************************************************************************/

int KhePreferTimesMonitorDeviationCount(KHE_PREFER_TIMES_MONITOR m)
{
  return 1;
}


/*****************************************************************************/
/*                                                                           */
/*  int KhePreferTimesMonitorDeviation(KHE_PREFER_TIMES_MONITOR m, int i)    */
/*                                                                           */
/*  Return the i'th deviation of m.                                          */
/*                                                                           */
/*****************************************************************************/

int KhePreferTimesMonitorDeviation(KHE_PREFER_TIMES_MONITOR m, int i)
{
  MAssert(i == 0, "KhePreferTimesMonitorDeviation: i out of range");
  return m->deviation;
}


/*****************************************************************************/
/*                                                                           */
/*  char *KhePreferTimesMonitorDeviationDescription(                         */
/*    KHE_PREFER_TIMES_MONITOR m, int i)                                     */
/*                                                                           */
/*  Return a description of the i'th deviation of m.                         */
/*                                                                           */
/*****************************************************************************/

char *KhePreferTimesMonitorDeviationDescription(
  KHE_PREFER_TIMES_MONITOR m, int i)
{
  MAssert(i == 0, "KhePreferTimesMonitorDeviationDescription: i out of range");
  return NULL;
}


/*****************************************************************************/
/*                                                                           */
/*  Submodule "debug"                                                        */
/*                                                                           */
/*****************************************************************************/

/*****************************************************************************/
/*                                                                           */
/*  void KhePreferTimesMonitorDebug(KHE_PREFER_TIMES_MONITOR m,              */
/*    int verbosity, int indent, FILE *fp)                                   */
/*                                                                           */
/*  Debug print of m onto fp with the given verbosity and indent.            */
/*                                                                           */
/*****************************************************************************/

void KhePreferTimesMonitorDebug(KHE_PREFER_TIMES_MONITOR m,
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
