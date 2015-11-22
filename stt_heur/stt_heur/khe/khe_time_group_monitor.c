
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
/*  FILE:         khe_time_group_monitor.c                                   */
/*  DESCRIPTION:  A time group monitor                                       */
/*                                                                           */
/*****************************************************************************/
#include "khe_interns.h"


/*****************************************************************************/
/*                                                                           */
/*  KHE_TIME_GROUP_MONITOR - monitors incidence on one time group.           */
/*                                                                           */
/*****************************************************************************/

struct khe_time_group_monitor_rec {

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

  /* specific to KHE_TIME_GROUP_MONITOR */
  KHE_TIMETABLE_MONITOR		timetable_monitor;	/* enclosing timetab */
  KHE_TIME_GROUP		time_group;		/* being monitored   */
  ARRAY_KHE_MONITOR		monitors;		/* monitors          */
  int				old_busy_count;		/* old busy times    */
  int				new_busy_count;		/* new busy times    */
  int				old_idle_count;		/* old idle count    */
  LSET				new_busy_lset;		/* new set of times  */
  KHE_TIME_GROUP_MONITOR	copy;			/* used when copying */
};


/*****************************************************************************/
/*                                                                           */
/*  Submodule "construction and query"                                       */
/*                                                                           */
/*****************************************************************************/

/*****************************************************************************/
/*                                                                           */
/*  KHE_TIME_GROUP_MONITOR KheTimeGroupMonitorMake(KHE_TIMETABLE_MONITOR tm, */
/*    KHE_TIME_GROUP tg)                                                     */
/*                                                                           */
/*  Make a new time group monitor for tm, monitoring tg.                     */
/*                                                                           */
/*****************************************************************************/

KHE_TIME_GROUP_MONITOR KheTimeGroupMonitorMake(KHE_TIMETABLE_MONITOR tm,
  KHE_TIME_GROUP tg)
{
  KHE_TIME_GROUP_MONITOR res;  KHE_SOLN soln;
  soln = KheMonitorSoln((KHE_MONITOR) tm);
  MMake(res);
  KheMonitorInitCommonFields((KHE_MONITOR) res, soln,
    KHE_TIME_GROUP_MONITOR_TAG);
  res->timetable_monitor = tm;
  res->time_group = tg;
  MArrayInit(res->monitors);
  res->old_busy_count = 0;
  res->new_busy_count = 0;
  res->old_idle_count = 0;
  res->new_busy_lset = NULL;  /* signals no idle time monitoring yet */
  /* KheGroupMonitorAddMonitor((KHE_GROUP_MONITOR) soln, (KHE_MONITOR) res); */
  res->copy = NULL;
  return res;
}


/*****************************************************************************/
/*                                                                           */
/*  KHE_TIME_GROUP_MONITOR KheTimeGroupMonitorCopyPhase1(                    */
/*    KHE_TIME_GROUP_MONITOR tgm)                                            */
/*                                                                           */
/*  Carry out Phase 1 of the copying of tgm.                                 */
/*                                                                           */
/*****************************************************************************/

KHE_TIME_GROUP_MONITOR KheTimeGroupMonitorCopyPhase1(KHE_TIME_GROUP_MONITOR tgm)
{
  KHE_TIME_GROUP_MONITOR copy;  int i;  KHE_MONITOR m;
  if( tgm->copy == NULL )
  {
    MMake(copy);
    tgm->copy = copy;
    KheMonitorCopyCommonFields((KHE_MONITOR) copy, (KHE_MONITOR) tgm);
    copy->timetable_monitor =
      KheTimetableMonitorCopyPhase1(tgm->timetable_monitor);
    copy->time_group = tgm->time_group;
    MArrayInit(copy->monitors);
    MArrayForEach(tgm->monitors, &m, &i)
      MArrayAddLast(copy->monitors, KheMonitorCopyPhase1(m));
    copy->old_busy_count = tgm->old_busy_count;
    copy->new_busy_count = tgm->new_busy_count;
    copy->old_idle_count = tgm->old_idle_count;
    copy->new_busy_lset = tgm->new_busy_lset == NULL ? NULL :
      LSetCopy(tgm->new_busy_lset);
    copy->copy = NULL;
  }
  return tgm->copy;
}


/*****************************************************************************/
/*                                                                           */
/*  void KheTimeGroupMonitorCopyPhase2(KHE_TIME_GROUP_MONITOR tgm)           */
/*                                                                           */
/*  Carry out Phase 2 of the copying of tgm.                                 */
/*                                                                           */
/*****************************************************************************/

void KheTimeGroupMonitorCopyPhase2(KHE_TIME_GROUP_MONITOR tgm)
{
  int i;  KHE_MONITOR m;
  if( tgm->copy != NULL )
  {
    tgm->copy = NULL;
    MArrayForEach(tgm->monitors, &m, &i)
      KheMonitorCopyPhase2(m);
  }
}


/*****************************************************************************/
/*                                                                           */
/*  void KheTimeGroupMonitorDelete(KHE_TIME_GROUP_MONITOR tgm)               */
/*                                                                           */
/*  Delete tgm (but not the monitors attached to it).                        */
/*                                                                           */
/*****************************************************************************/

void KheTimeGroupMonitorDelete(KHE_TIME_GROUP_MONITOR tgm)
{
  if( tgm->attached )
    KheTimeGroupMonitorDetachFromSoln(tgm);
  if( tgm->parent_monitor != NULL )
    KheGroupMonitorDeleteChildMonitor(tgm->parent_monitor, (KHE_MONITOR) tgm);
  KheSolnDeleteMonitor(tgm->soln, (KHE_MONITOR) tgm);
  MArrayFree(tgm->monitors);
  MFree(tgm);
}


/*****************************************************************************/
/*                                                                           */
/*  KHE_TIMETABLE_MONITOR KheTimeGroupMonitorTimetableMonitor(               */
/*    KHE_TIME_GROUP_MONITOR tgm)                                            */
/*                                                                           */
/*  Return the timetable holding tgm.                                        */
/*                                                                           */
/*****************************************************************************/

KHE_TIMETABLE_MONITOR KheTimeGroupMonitorTimetableMonitor(
  KHE_TIME_GROUP_MONITOR tgm)
{
  return tgm->timetable_monitor;
}


/*****************************************************************************/
/*                                                                           */
/*  KHE_TIME_GROUP KheTimeGroupMonitorTimeGroup(KHE_TIME_GROUP_MONITOR tgm)  */
/*                                                                           */
/*  Return the time group that tgm is monitoring.                            */
/*                                                                           */
/*****************************************************************************/

KHE_TIME_GROUP KheTimeGroupMonitorTimeGroup(KHE_TIME_GROUP_MONITOR tgm)
{
  return tgm->time_group;
}


/*****************************************************************************/
/*                                                                           */
/*  Submodule "idle times" (private)                                         */
/*                                                                           */
/*****************************************************************************/

/*****************************************************************************/
/*                                                                           */
/*  void KheTimeGroupMonitorBeginIdleTimes(KHE_TIME_GROUP_MONITOR tgm)       */
/*                                                                           */
/*  Set up tgm for monitoring idle times.                                    */
/*                                                                           */
/*****************************************************************************/

static void KheTimeGroupMonitorBeginIdleTimes(KHE_TIME_GROUP_MONITOR tgm)
{
  int i;  KHE_TIME t;
  MAssert(tgm->old_idle_count == 0,
    "KheTimeGroupMonitorBeginIdleTimes internal error 1");
  MAssert(tgm->new_busy_lset == NULL,
    "KheTimeGroupMonitorBeginIdleTimes internal error 2");
  tgm->new_busy_lset = LSetNew();
  for( i = 0;  i < KheTimeGroupTimeCount(tgm->time_group);  i++ )
  {
    t = KheTimeGroupTime(tgm->time_group, i);
    if( KheTimetableMonitorTimeMeetCount(tgm->timetable_monitor, t) > 0 )
      LSetInsert(&tgm->new_busy_lset, i);  /* NB *position* in time group */
  }
}


/*****************************************************************************/
/*                                                                           */
/*  void KheTimeGroupMonitorEndIdleTimes(KHE_TIME_GROUP_MONITOR tgm)         */
/*                                                                           */
/*  Finish monitoring idle times in tgm.                                     */
/*                                                                           */
/*****************************************************************************/

static void KheTimeGroupMonitorEndIdleTimes(KHE_TIME_GROUP_MONITOR tgm)
{
  MAssert(tgm->new_busy_lset != NULL,
    "KheTimeGroupMonitorEndIdleTimes internal error");
  LSetFree(tgm->new_busy_lset);
  tgm->new_busy_lset = NULL;
}


/*****************************************************************************/
/*                                                                           */
/*  int KheIdleTimes(KHE_TIME_GROUP_MONITOR tgm)                             */
/*                                                                           */
/*  Return the new number of idle times.                                     */
/*                                                                           */
/*****************************************************************************/

static int KheIdleTimes(KHE_TIME_GROUP_MONITOR tgm)
{
  return tgm->new_busy_lset == NULL || tgm->new_busy_count <= 1 ? 0 :
    LSetMax(tgm->new_busy_lset) - LSetMin(tgm->new_busy_lset) + 1 -
      tgm->new_busy_count;
}


/*****************************************************************************/
/*                                                                           */
/*  Submodule "attach and detach"                                            */
/*                                                                           */
/*****************************************************************************/

/*****************************************************************************/
/*                                                                           */
/*  void KheTimeGroupMonitorAttachToSoln(KHE_TIME_GROUP_MONITOR tgm)         */
/*                                                                           */
/*  Attach tgm.                                                              */
/*                                                                           */
/*****************************************************************************/

void KheTimeGroupMonitorAttachToSoln(KHE_TIME_GROUP_MONITOR tgm)
{
  tgm->attached = true;
  KheTimetableMonitorAttachMonitor(tgm->timetable_monitor, (KHE_MONITOR) tgm);
}


/*****************************************************************************/
/*                                                                           */
/*  void KheTimeGroupMonitorDetachFromSoln(KHE_TIME_GROUP_MONITOR tgm)       */
/*                                                                           */
/*  Detach tgm.                                                              */
/*                                                                           */
/*****************************************************************************/

void KheTimeGroupMonitorDetachFromSoln(KHE_TIME_GROUP_MONITOR tgm)
{
  KheTimetableMonitorDetachMonitor(tgm->timetable_monitor, (KHE_MONITOR) tgm);
  tgm->attached = false;
}


/*****************************************************************************/
/*                                                                           */
/*  void KheTimeGroupMonitorAttachCheck(KHE_TIME_GROUP_MONITOR m)            */
/*                                                                           */
/*  Check the attachment of m.                                               */
/*                                                                           */
/*****************************************************************************/

void KheTimeGroupMonitorAttachCheck(KHE_TIME_GROUP_MONITOR m)
{
  /* not sure about this! but it is the default behaviour */
  if( !KheMonitorAttachedToSoln((KHE_MONITOR) m) )
    KheMonitorAttachToSoln((KHE_MONITOR) m);
}


/*****************************************************************************/
/*                                                                           */
/*  Submodule "monitors"                                                     */
/*                                                                           */
/*****************************************************************************/

/*****************************************************************************/
/*                                                                           */
/*  void KheTimeGroupMonitorAttachMonitor(KHE_TIME_GROUP_MONITOR tgm,        */
/*    KHE_MONITOR m)                                                         */
/*                                                                           */
/*  Attach m to tgm.  Someone else will flush it.                            */
/*                                                                           */
/*****************************************************************************/

void KheTimeGroupMonitorAttachMonitor(KHE_TIME_GROUP_MONITOR tgm,
  KHE_MONITOR m)
{
  MArrayAddLast(tgm->monitors, m);
  if( KheMonitorTag(m) == KHE_LIMIT_IDLE_TIMES_MONITOR_TAG && 
      tgm->new_busy_lset == NULL )
    KheTimeGroupMonitorBeginIdleTimes(tgm);
  tgm->old_idle_count = KheIdleTimes(tgm);
  KheMonitorAddBusyAndIdle(m, tgm->old_busy_count, tgm->old_idle_count);
}


/*****************************************************************************/
/*                                                                           */
/*  bool KheTimeGroupMonitorHasIdleTimesMonitor(KHE_TIME_GROUP_MONITOR tgm)  */
/*                                                                           */
/*  Return true when at least one of the monitors of tgm is a limit idle     */
/*  times monitor.                                                           */
/*                                                                           */
/*****************************************************************************/

static bool KheTimeGroupMonitorHasIdleTimesMonitor(KHE_TIME_GROUP_MONITOR tgm)
{
  KHE_MONITOR m;  int i;
  MArrayForEach(tgm->monitors, &m, &i)
    if( KheMonitorTag(m) == KHE_LIMIT_IDLE_TIMES_MONITOR_TAG )
      return true;
  return false;
}


/*****************************************************************************/
/*                                                                           */
/*  void KheTimeGroupMonitorDetachMonitor(KHE_TIME_GROUP_MONITOR tgm,        */
/*    KHE_MONITOR m)                                                         */
/*                                                                           */
/*  Detach m from tgm.  Someone else will flush it.                          */
/*                                                                           */
/*****************************************************************************/

void KheTimeGroupMonitorDetachMonitor(KHE_TIME_GROUP_MONITOR tgm,
  KHE_MONITOR m)
{
  int pos;
  if( !MArrayContains(tgm->monitors, m, &pos) )
    MAssert(false, "KheTimeGroupMonitorDetachMonitor internal error");
  MArrayRemove(tgm->monitors, pos);
  KheMonitorDeleteBusyAndIdle(m, tgm->old_busy_count, tgm->old_idle_count);
  if( tgm->new_busy_lset!=NULL && !KheTimeGroupMonitorHasIdleTimesMonitor(tgm) )
    KheTimeGroupMonitorEndIdleTimes(tgm);
}


/*****************************************************************************/
/*                                                                           */
/*  Submodule "monitoring calls"                                             */
/*                                                                           */
/*****************************************************************************/

/*****************************************************************************/
/*                                                                           */
/*  void KheTimeGroupMonitorAssignNonClash(KHE_TIME_GROUP_MONITOR tgm,       */
/*    int assigned_time_index)                                               */
/*                                                                           */
/*  Inform tgm that in the enclosing timetable, the time with this index     */
/*  number (which lies in tgm's time group) has changed from being           */
/*  unoccupied to being occupied.                                            */
/*                                                                           */
/*****************************************************************************/

void KheTimeGroupMonitorAssignNonClash(KHE_TIME_GROUP_MONITOR tgm,
  int assigned_time_index)
{
  tgm->new_busy_count++;
  if( tgm->new_busy_lset != NULL )
    LSetInsert(&tgm->new_busy_lset,
      KheTimeGroupTimePos(tgm->time_group, assigned_time_index));
}


/*****************************************************************************/
/*                                                                           */
/*  void KheTimeGroupMonitorUnAssignNonClash(KHE_TIME_GROUP_MONITOR tgm,     */
/*    int assigned_time_index)                                               */
/*                                                                           */
/*  Inform tgm that in the enclosing timetable, the time with this index     */
/*  number (which lies in tgm's time group) has changed from being           */
/*  occupied to being unoccupied.                                            */
/*                                                                           */
/*****************************************************************************/

void KheTimeGroupMonitorUnAssignNonClash(KHE_TIME_GROUP_MONITOR tgm,
  int assigned_time_index)
{
  MAssert(tgm->new_busy_count > 0,
    "KheTimeGroupMonitorUnAssignNonClash internal error");
  tgm->new_busy_count--;
  if( tgm->new_busy_lset != NULL )
    LSetDelete(tgm->new_busy_lset,
      KheTimeGroupTimePos(tgm->time_group, assigned_time_index));
}


/*****************************************************************************/
/*                                                                           */
/*  void KheTimeGroupMonitorFlush(KHE_TIME_GROUP_MONITOR tgm)                */
/*                                                                           */
/*  Flush out tgm:  report its incidences to whoever wants to know (but      */
/*  only if they have changed), and reset the old ones to the new ones.      */
/*                                                                           */
/*****************************************************************************/

void KheTimeGroupMonitorFlush(KHE_TIME_GROUP_MONITOR tgm)
{
  KHE_MONITOR m;  int i, new_idle_count;
  new_idle_count = KheIdleTimes(tgm);
  if( tgm->old_busy_count != tgm->new_busy_count ||
      tgm->old_idle_count != new_idle_count )
  {
    MArrayForEach(tgm->monitors, &m, &i)
      KheMonitorChangeBusyAndIdle(m, tgm->old_busy_count, tgm->new_busy_count,
	  tgm->old_idle_count, new_idle_count);
    tgm->old_busy_count = tgm->new_busy_count;
    tgm->old_idle_count = new_idle_count;
  }
}


/*****************************************************************************/
/*                                                                           */
/*  int KheTimeGroupMonitorBusyTimes(KHE_TIME_GROUP_MONITOR tgm)             */
/*                                                                           */
/*  Return the number of busy times in tgm.                                  */
/*                                                                           */
/*****************************************************************************/

int KheTimeGroupMonitorBusyTimes(KHE_TIME_GROUP_MONITOR tgm)
{
  return tgm->old_busy_count;
}


/*****************************************************************************/
/*                                                                           */
/*  int KheTimeGroupMonitorIdleTimes(KHE_TIME_GROUP_MONITOR tgm)             */
/*                                                                           */
/*  Return the number of idle times in tgm.                                  */
/*                                                                           */
/*****************************************************************************/

int KheTimeGroupMonitorIdleTimes(KHE_TIME_GROUP_MONITOR tgm)
{
  return tgm->old_idle_count;
}


/*****************************************************************************/
/*                                                                           */
/*  Submodule "deviations"                                                   */
/*                                                                           */
/*****************************************************************************/

/*****************************************************************************/
/*                                                                           */
/*  int KheTimeGroupMonitorDeviationCount(KHE_TIME_GROUP_MONITOR m)          */
/*                                                                           */
/*  Return the deviations of m (0 in this case).                             */
/*                                                                           */
/*****************************************************************************/

int KheTimeGroupMonitorDeviationCount(KHE_TIME_GROUP_MONITOR m)
{
  return 0;
}


/*****************************************************************************/
/*                                                                           */
/*  int KheTimeGroupMonitorDeviation(KHE_TIME_GROUP_MONITOR m, int i)        */
/*                                                                           */
/*  Return the i'th deviation of m.  There are none it's an error.           */
/*                                                                           */
/*****************************************************************************/

int KheTimeGroupMonitorDeviation(KHE_TIME_GROUP_MONITOR m, int i)
{
  MAssert(false, "KheTimeGroupMonitorDeviation: i out of range");
  return 0;  /* keep compiler happy */
}


/*****************************************************************************/
/*                                                                           */
/*  char *KheTimeGroupMonitorDeviationDescription(                           */
/*    KHE_TIME_GROUP_MONITOR m, int i)                                       */
/*                                                                           */
/*  Return a description of the i'th deviation of m.  There are no           */
/*  deviations so it's an error.                                             */
/*                                                                           */
/*****************************************************************************/

char *KheTimeGroupMonitorDeviationDescription(
  KHE_TIME_GROUP_MONITOR m, int i)
{
  MAssert(false, "KheTimeGroupMonitorDeviationDescription: i out of range");
  return NULL;  /* keep compiler happy */
}


/*****************************************************************************/
/*                                                                           */
/*  Submodule "debug"                                                        */
/*                                                                           */
/*****************************************************************************/

/*****************************************************************************/
/*                                                                           */
/*  void KheTimeGroupMonitorDebug(KHE_TIME_GROUP_MONITOR tgm,                */
/*    int verbosity, int indent, FILE *fp)                                   */
/*                                                                           */
/*  Debug print of tgm onto fp with the given verbosity and indent.          */
/*                                                                           */
/*****************************************************************************/

void KheTimeGroupMonitorDebug(KHE_TIME_GROUP_MONITOR tgm,
  int verbosity, int indent, FILE *fp)
{
  if( verbosity >= 1 )
  {
    KheMonitorDebugBegin((KHE_MONITOR) tgm, indent, fp);
    fprintf(fp, " ");
    KheTimeGroupDebug(tgm->time_group, 1, -1, fp);
    fprintf(fp, " (%d busy", tgm->old_busy_count);
    if( tgm->new_busy_lset != NULL )
      fprintf(fp, ", %d idle", tgm->old_idle_count);
    fprintf(fp, ")");
    KheMonitorDebugEnd((KHE_MONITOR) tgm, true, indent, fp);
  }
}
