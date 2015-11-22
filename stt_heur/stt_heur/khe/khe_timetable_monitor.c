
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
/*  FILE:         khe_timetable_monitor.c                                    */
/*  DESCRIPTION:  A timetable monitor                                        */
/*                                                                           */
/*****************************************************************************/
#include "khe_interns.h"

#define DEBUG1 0
#define DEBUG2 0
#define DEBUG3 0
#define DEBUG4 0
#define DEBUG4_RESOURCE "Year12"

/*****************************************************************************/
/*                                                                           */
/*  KHE_TIME_CELL - one time of a timetable                                  */
/*                                                                           */
/*****************************************************************************/

typedef struct khe_time_cell_rec *KHE_TIME_CELL;
typedef MARRAY(KHE_TIME_CELL) ARRAY_KHE_TIME_CELL;

struct khe_time_cell_rec {
  KHE_TIME			time;			/* monitored time    */
  ARRAY_KHE_MEET		meets;			/* incidences        */
  ARRAY_KHE_MONITOR		monitors;		/* monitors          */
  KHE_TIME_CELL			copy;			/* used when copying */
};


/*****************************************************************************/
/*                                                                           */
/*  KHE_TIMETABLE_MONITOR - a timetable monitor                              */
/*                                                                           */
/*****************************************************************************/

struct khe_timetable_monitor_rec {

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

  /* specific to KHE_TIMETABLE_MONITOR */
  KHE_RESOURCE_IN_SOLN 		resource_in_soln;	/* resource or...    */
  KHE_EVENT_IN_SOLN		event_in_soln;		/* event             */
  ARRAY_KHE_TIME_CELL		time_cells;		/* time cells        */
  ARRAY_KHE_AVOID_CLASHES_MONITOR avoid_clashes_monitors; /* some monitors   */
  ARRAY_KHE_MONITOR		other_monitors;		/* other monitors    */
  KHE_TIMETABLE_MONITOR		copy;			/* used when copying */
};


/*****************************************************************************/
/*                                                                           */
/*  Submodule "time cells" (private)                                         */
/*                                                                           */
/*****************************************************************************/

/*****************************************************************************/
/*                                                                           */
/*  KHE_TIME_CELL KheTimeCellMake(KHE_TIME t)                                */
/*                                                                           */
/*  Make and initialize a new time cell object.                              */
/*                                                                           */
/*****************************************************************************/

static KHE_TIME_CELL KheTimeCellMake(KHE_TIME t)
{
  KHE_TIME_CELL res;
  MMake(res);
  res->time = t;
  MArrayInit(res->meets);
  MArrayInit(res->monitors);
  res->copy = NULL;
  return res;
}


/*****************************************************************************/
/*                                                                           */
/*  KHE_TIME_CELL KheTimeCellCopyPhase1(KHE_TIME_CELL tc)                    */
/*                                                                           */
/*  Carry out Phase 1 of copying tc.                                         */
/*                                                                           */
/*****************************************************************************/

static KHE_TIME_CELL KheTimeCellCopyPhase1(KHE_TIME_CELL tc)
{
  KHE_TIME_CELL copy;  KHE_MEET meet;  KHE_MONITOR m;  int i;
  if( tc->copy == NULL )
  {
    MMake(copy);
    tc->copy = copy;
    copy->time = tc->time;
    MArrayInit(copy->meets);
    MArrayForEach(tc->meets, &meet, &i)
      MArrayAddLast(copy->meets, KheMeetCopyPhase1(meet));
    MArrayInit(copy->monitors);
    MArrayForEach(tc->monitors, &m, &i)
      MArrayAddLast(copy->monitors, KheMonitorCopyPhase1(m));
    copy->copy = NULL;
  }
  return tc->copy;
}


/*****************************************************************************/
/*                                                                           */
/*  void KheTimeCellCopyPhase2(KHE_TIME_CELL tc)                             */
/*                                                                           */
/*  Carry out Phase 2 of copying tc.                                         */
/*                                                                           */
/*****************************************************************************/

static void KheTimeCellCopyPhase2(KHE_TIME_CELL tc)
{
  KHE_MEET meet;  KHE_MONITOR m;  int i;
  if( tc->copy != NULL )
  {
    tc->copy = NULL;
    MArrayForEach(tc->meets, &meet, &i)
      KheMeetCopyPhase2(meet);
    MArrayForEach(tc->monitors, &m, &i)
      KheMonitorCopyPhase2(m);
  }
}


/*****************************************************************************/
/*                                                                           */
/*  void KheTimeCellDelete(KHE_TIME_CELL tc)                                 */
/*                                                                           */
/*  Delete tc.  The elements of the arrays don't have to be deleted.         */
/*                                                                           */
/*****************************************************************************/

static void KheTimeCellDelete(KHE_TIME_CELL tc)
{
  MArrayFree(tc->meets);
  MArrayFree(tc->monitors);
  MFree(tc);
}


/*****************************************************************************/
/*                                                                           */
/*  void KheTimeCellDebug(KHE_TIME_CELL tc, int verbosity, int indent,       */
/*    FILE *fp)                                                              */
/*                                                                           */
/*  Debug print of tc onto fp with the given verbosity and indent.           */
/*                                                                           */
/*****************************************************************************/

static void KheTimeCellDebug(KHE_TIME_CELL tc, int verbosity,
  int indent, FILE *fp)
{
  KHE_MEET meet;  KHE_MONITOR m;  int i;
  if( verbosity >= 1 )
  {
    if( indent >= 0 )
      fprintf(fp, "%*s", indent, "");
    fprintf(fp, "[ %s", KheTimeId(tc->time) == NULL ? "-" :
      KheTimeId(tc->time));
    MArrayForEach(tc->meets, &meet, &i)
    {
      fprintf(fp, i == 0 ? ": " : " ");
      KheMeetDebug(meet, 1, -1, fp);
    }
    if( indent >= 0 && MArraySize(tc->monitors) > 0 )
    {
      fprintf(fp, "\n");
      if( verbosity >= 2 )
      {
	MArrayForEach(tc->monitors, &m, &i)
	  KheMonitorDebug(m, 1, indent + 2, fp);
      }
      fprintf(fp, "%*s]", indent, "");
    }
    else
      fprintf(fp, " ]");
    if( indent >= 0 )
      fprintf(fp, "\n");
  }
}


/*****************************************************************************/
/*                                                                           */
/*  Submodule "construction and query"                                       */
/*                                                                           */
/*****************************************************************************/

/*****************************************************************************/
/*                                                                           */
/*  KHE_TIMETABLE_MONITOR KheTimetableMonitorMake(KHE_SOLN soln,             */
/*    KHE_RESOURCE_IN_SOLN rs, KHE_EVENT_IN_SOLN es)                         */
/*                                                                           */
/*  Make a new timetable monitor for soln, monitoring either rs or es.       */
/*                                                                           */
/*****************************************************************************/

KHE_TIMETABLE_MONITOR KheTimetableMonitorMake(KHE_SOLN soln,
  KHE_RESOURCE_IN_SOLN rs, KHE_EVENT_IN_SOLN es)
{
  KHE_TIMETABLE_MONITOR res;  KHE_INSTANCE ins;  int i;  KHE_TIME_CELL tc;
  if( DEBUG4 && rs != NULL &&
      strcmp(KheResourceInSolnId(rs), DEBUG4_RESOURCE) == 0 )
    fprintf(stderr, "KheTimetableMonitorMake(soln, %s, -)\n", DEBUG4_RESOURCE);
  ins = KheSolnInstance(soln);
  MMake(res);
  KheMonitorInitCommonFields((KHE_MONITOR) res, soln,
    KHE_TIMETABLE_MONITOR_TAG);
  res->resource_in_soln = rs;
  res->event_in_soln = es;
  MArrayInit(res->time_cells);
  for( i = 0;  i < KheInstanceTimeCount(ins);  i++ )
  {
    tc = KheTimeCellMake(KheInstanceTime(ins, i));
    MArrayAddLast(res->time_cells, tc);
  }
  MArrayInit(res->avoid_clashes_monitors);
  MArrayInit(res->other_monitors);
  res->copy = NULL;
  /* KheGroupMonitorAddMonitor((KHE_GROUP_MONITOR) soln, (KHE_MONITOR) res); */
  return res;
}


/*****************************************************************************/
/*                                                                           */
/*  KHE_TIMETABLE_MONITOR KheTimetableMonitorCopyPhase1(                     */
/*    KHE_TIMETABLE_MONITOR tm)                                              */
/*                                                                           */
/*  Carry out Phase 1 of copying tm.                                         */
/*                                                                           */
/*****************************************************************************/

KHE_TIMETABLE_MONITOR KheTimetableMonitorCopyPhase1(KHE_TIMETABLE_MONITOR tm)
{
  KHE_TIME_CELL tc;  KHE_AVOID_CLASHES_MONITOR acm;  KHE_MONITOR m;  int i;
  KHE_TIMETABLE_MONITOR copy;
  if( tm->copy == NULL )
  {
    MMake(copy);
    tm->copy = copy;
    KheMonitorCopyCommonFields((KHE_MONITOR) copy, (KHE_MONITOR) tm);
    copy->resource_in_soln = tm->resource_in_soln == NULL ? NULL :
      KheResourceInSolnCopyPhase1(tm->resource_in_soln);
    copy->event_in_soln = tm->event_in_soln == NULL ? NULL :
      KheEventInSolnCopyPhase1(tm->event_in_soln);
    MArrayInit(copy->time_cells);
    MArrayForEach(tm->time_cells, &tc, &i)
      MArrayAddLast(copy->time_cells, KheTimeCellCopyPhase1(tc));
    MArrayInit(copy->avoid_clashes_monitors);
    MArrayForEach(tm->avoid_clashes_monitors, &acm, &i)
      MArrayAddLast(copy->avoid_clashes_monitors,
	KheAvoidClashesMonitorCopyPhase1(acm));
    MArrayInit(copy->other_monitors);
    MArrayForEach(tm->other_monitors, &m, &i)
      MArrayAddLast(copy->other_monitors, KheMonitorCopyPhase1(m));
    copy->copy = NULL;
  }
  return tm->copy;
}


/*****************************************************************************/
/*                                                                           */
/*  void KheTimetableMonitorCopyPhase2(KHE_TIMETABLE_MONITOR tm)             */
/*                                                                           */
/*  Carry out Phase 2 of copying tm.                                         */
/*                                                                           */
/*****************************************************************************/

void KheTimetableMonitorCopyPhase2(KHE_TIMETABLE_MONITOR tm)
{
  KHE_TIME_CELL tc;  KHE_AVOID_CLASHES_MONITOR acm;  KHE_MONITOR m;  int i;
  if( tm->copy != NULL )
  {
    tm->copy = NULL;
    MArrayForEach(tm->time_cells, &tc, &i)
      KheTimeCellCopyPhase2(tc);
    MArrayForEach(tm->avoid_clashes_monitors, &acm, &i)
      KheAvoidClashesMonitorCopyPhase2(acm);
    MArrayForEach(tm->other_monitors, &m, &i)
      KheMonitorCopyPhase2(m);
  }
}


/*****************************************************************************/
/*                                                                           */
/*  void KheTimetableMonitorDelete(KHE_TIMETABLE_MONITOR tm)                 */
/*                                                                           */
/*  Delete tm.  The monitors attached to it will be deleted separately.      */
/*                                                                           */
/*****************************************************************************/

void KheTimetableMonitorDelete(KHE_TIMETABLE_MONITOR tm)
{
  KHE_TIME_CELL tc;  int i;
  if( tm->attached )
    KheTimetableMonitorDetachFromSoln(tm);
  if( tm->parent_monitor != NULL )
    KheGroupMonitorDeleteChildMonitor(tm->parent_monitor, (KHE_MONITOR) tm);
  KheSolnDeleteMonitor(tm->soln, (KHE_MONITOR) tm);
  MArrayForEach(tm->time_cells, &tc, &i)
    KheTimeCellDelete(tc);
  MArrayFree(tm->time_cells);
  MArrayFree(tm->avoid_clashes_monitors);
  MArrayFree(tm->other_monitors);
  MFree(tm);
}


/*****************************************************************************/
/*                                                                           */
/*  int KheTimetableMonitorTimeMeetCount(KHE_TIMETABLE_MONITOR tm,           */
/*    KHE_TIME time)                                                         */
/*                                                                           */
/*  Return the number of meets running at time.                              */
/*                                                                           */
/*****************************************************************************/

int KheTimetableMonitorTimeMeetCount(KHE_TIMETABLE_MONITOR tm, KHE_TIME time)
{
  KHE_TIME_CELL tc;
  tc = MArrayGet(tm->time_cells, KheTimeIndex(time));
  return MArraySize(tc->meets);
}


/*****************************************************************************/
/*                                                                           */
/*  KHE_MEET KheTimetableMonitorTimeMeet(KHE_TIMETABLE_MONITOR tm,           */
/*    KHE_TIME time, int i)                                                  */
/*                                                                           */
/*  Return the i'th meet running at time.                                    */
/*                                                                           */
/*****************************************************************************/

KHE_MEET KheTimetableMonitorTimeMeet(KHE_TIMETABLE_MONITOR tm,
  KHE_TIME time, int i)
{
  KHE_TIME_CELL tc;
  tc = MArrayGet(tm->time_cells, KheTimeIndex(time));
  return MArrayGet(tc->meets, i);
}


/*****************************************************************************/
/*                                                                           */
/*  Submodule "monitors"                                                     */
/*                                                                           */
/*****************************************************************************/

/*****************************************************************************/
/*                                                                           */
/*  void KheTimetableMonitorAttachAvoidClashesMonitor(                       */
/*    KHE_TIMETABLE_MONITOR tm, KHE_AVOID_CLASHES_MONITOR m)                 */
/*                                                                           */
/*  Attach avoid clashes monitor m to tm.                                    */
/*                                                                           */
/*****************************************************************************/

#define DO_DEBUG4 (DEBUG4 && tm->resource_in_soln != NULL &&		\
  strcmp(KheResourceInSolnId(tm->resource_in_soln), DEBUG4_RESOURCE) == 0)

static void KheTimetableMonitorAttachAvoidClashesMonitor(
  KHE_TIMETABLE_MONITOR tm, KHE_AVOID_CLASHES_MONITOR m)
{
  int i;  KHE_TIME_CELL tc;
  if( DO_DEBUG4 )
    fprintf(stderr, "KheTimetableMonitorAttachAvoidClashesMonitor()\n");
  MArrayAddLast(tm->avoid_clashes_monitors, m);
  MArrayForEach(tm->time_cells, &tc, &i)
    if( MArraySize(tc->meets) >= 2 )
      KheAvoidClashesMonitorChangeClashCount(m, 0,
	MArraySize(tc->meets) - 1);
  KheAvoidClashesMonitorFlush(m);
}


/*****************************************************************************/
/*                                                                           */
/*  void KheTimetableMonitorDetachAvoidClashesMonitor(                       */
/*    KHE_TIMETABLE_MONITOR tm, KHE_AVOID_CLASHES_MONITOR m)                 */
/*                                                                           */
/*  Detach avoid clashes monitor m from tm.                                  */
/*                                                                           */
/*****************************************************************************/

static void KheTimetableMonitorDetachAvoidClashesMonitor(
  KHE_TIMETABLE_MONITOR tm, KHE_AVOID_CLASHES_MONITOR m)
{
  int i, pos;  KHE_TIME_CELL tc;
  if( DO_DEBUG4 )
    fprintf(stderr, "KheTimetableMonitorDetachAvoidClashesMonitor()\n");
  MArrayForEach(tm->time_cells, &tc, &i)
    if( MArraySize(tc->meets) >= 2 )
      KheAvoidClashesMonitorChangeClashCount(m,
	MArraySize(tc->meets) - 1, 0);
  KheAvoidClashesMonitorFlush(m);
  if( !MArrayContains(tm->avoid_clashes_monitors, m, &pos) )
    MAssert(false,
      "KheTimetableMonitorDetachAvoidClashesMonitor internal error");
  MArrayRemove(tm->avoid_clashes_monitors, pos);
}


/*****************************************************************************/
/*                                                                           */
/*  void KheTimetableMonitorAttachLinkEventsMonitor(                         */
/*    KHE_TIMETABLE_MONITOR tm, KHE_LINK_EVENTS_MONITOR m)                   */
/*                                                                           */
/*  Attach link events monitor m to tm.                                      */
/*                                                                           */
/*****************************************************************************/

static void KheTimetableMonitorAttachLinkEventsMonitor(
  KHE_TIMETABLE_MONITOR tm, KHE_LINK_EVENTS_MONITOR m)
{
  int i;  KHE_TIME_CELL tc;
  MArrayAddLast(tm->other_monitors, (KHE_MONITOR) m);
  for( i = 0;  i < MArraySize(tm->time_cells);  i++ )
  {
    tc = MArrayGet(tm->time_cells, i);
    MArrayAddLast(tc->monitors, (KHE_MONITOR) m);
    if( MArraySize(tc->meets) > 0 )
      KheLinkEventsMonitorAssignNonClash(m, i);
  }
  KheLinkEventsMonitorFlush(m);
}


/*****************************************************************************/
/*                                                                           */
/*  void KheTimetableMonitorDetachLinkEventsMonitor(                         */
/*    KHE_TIMETABLE_MONITOR tm, KHE_LINK_EVENTS_MONITOR m)                   */
/*                                                                           */
/*  Detach link events monitor m from tm.                                    */
/*                                                                           */
/*****************************************************************************/

static void KheTimetableMonitorDetachLinkEventsMonitor(
  KHE_TIMETABLE_MONITOR tm, KHE_LINK_EVENTS_MONITOR m)
{
  int i, pos;  KHE_TIME_CELL tc;
  if( !MArrayContains(tm->other_monitors, (KHE_MONITOR) m, &pos) )
    MAssert(false,
      "KheTimetableMonitorDetachLinkEventsMonitor internal error 1");
  MArrayRemove(tm->other_monitors, pos);
  for( i = 0;  i < MArraySize(tm->time_cells);  i++ )
  {
    tc = MArrayGet(tm->time_cells, i);
    if( !MArrayContains(tc->monitors, (KHE_MONITOR) m, &pos) )
      MAssert(false,
	"KheTimetableMonitorDetachLinkEventsMonitor internal error 2");
    MArrayRemove(tc->monitors, pos);
    if( MArraySize(tc->meets) > 0 )
      KheLinkEventsMonitorUnAssignNonClash(m, i);
  }
  KheLinkEventsMonitorFlush(m);
}


/*****************************************************************************/
/*                                                                           */
/* bool KheTimetableMonitorContainsTimeGroupMonitor(KHE_TIMETABLE_MONITOR tm,*/
/*    KHE_TIME_GROUP tg, KHE_TIME_GROUP_MONITOR *tgm)                        */
/*                                                                           */
/*  If tm contains a time group monitor monitoring tg, set *tgm to that      */
/*  monitor and return true.  Otherwise return false.                        */
/*                                                                           */
/*****************************************************************************/

bool KheTimetableMonitorContainsTimeGroupMonitor(
  KHE_TIMETABLE_MONITOR tm, KHE_TIME_GROUP tg, KHE_TIME_GROUP_MONITOR *tgm)
{
  KHE_MONITOR m;  int i;
  MArrayForEach(tm->other_monitors, &m, &i)
    if( KheMonitorTag(m) == KHE_TIME_GROUP_MONITOR_TAG )
    {
      *tgm = (KHE_TIME_GROUP_MONITOR) m;
      if( KheTimeGroupEqual(KheTimeGroupMonitorTimeGroup(*tgm), tg) )
	return true;
    }
  *tgm = NULL;
  return false;
}


/*****************************************************************************/
/*                                                                           */
/*  void KheTimetableMonitorAttachTimeGroupMonitor(KHE_TIMETABLE_MONITOR tm, */
/*    KHE_TIME_GROUP_MONITOR tgm)                                            */
/*                                                                           */
/*  Attach tgm to tg.  Add it to tm's "other" monitors at the front, to      */
/*  get the right topological ordering.                                      */
/*                                                                           */
/*****************************************************************************/

static void KheTimetableMonitorAttachTimeGroupMonitor(KHE_TIMETABLE_MONITOR tm,
  KHE_TIME_GROUP_MONITOR tgm)
{
  KHE_TIME_GROUP tg;  int i;  KHE_TIME t;  KHE_TIME_CELL tc;
  if( DEBUG3 )
  {
    fprintf(stderr, "[ KheTimetableMonitorAttachTimeGroupMonitor(tm, tgm)\n");
    KheTimetableMonitorDebug(tm, 2, 2, stderr);
  }
  MArrayAddFirst(tm->other_monitors, (KHE_MONITOR) tgm);
  tg = KheTimeGroupMonitorTimeGroup(tgm);
  for( i = 0;  i < KheTimeGroupTimeCount(tg);  i++ )
  {
    t = KheTimeGroupTime(tg, i);
    tc = MArrayGet(tm->time_cells, KheTimeIndex(t));
    MArrayAddLast(tc->monitors, (KHE_MONITOR) tgm);
    if( MArraySize(tc->meets) > 0 )
      KheTimeGroupMonitorAssignNonClash(tgm, KheTimeIndex(t));
  }
  KheTimeGroupMonitorFlush(tgm);
  if( DEBUG3 )
    fprintf(stderr, "] KheTimetableMonitorAttachTimeGroupMonitor returning\n");
}


/*****************************************************************************/
/*                                                                           */
/*  void KheTimetableMonitorDetachTimeGroupMonitor(KHE_TIMETABLE_MONITOR tm, */
/*    KHE_TIME_GROUP_MONITOR tgm)                                            */
/*                                                                           */
/*  Detach tgm from tm.                                                      */
/*                                                                           */
/*****************************************************************************/

static void KheTimetableMonitorDetachTimeGroupMonitor(KHE_TIMETABLE_MONITOR tm,
  KHE_TIME_GROUP_MONITOR tgm)
{
  int i, pos;  KHE_TIME t;  KHE_TIME_CELL tc;  KHE_TIME_GROUP tg;
  if( !MArrayContains(tm->other_monitors, (KHE_MONITOR) tgm, &pos) )
    MAssert(false, "KheTimetableMonitorDetachTimeGroupMonitor internal err 1");
  MArrayRemove(tm->other_monitors, pos);
  tg = KheTimeGroupMonitorTimeGroup(tgm);
  for( i = 0;  i < KheTimeGroupTimeCount(tg);  i++ )
  {
    t = KheTimeGroupTime(tg, i);
    tc = MArrayGet(tm->time_cells, KheTimeIndex(t));
    if( !MArrayContains(tc->monitors, (KHE_MONITOR) tgm, &pos) )
      MAssert(false,"KheTimetableMonitorDetachTimeGroupMonitor internal err 2");
    MArrayRemove(tc->monitors, pos);
    if( MArraySize(tc->meets) > 0 )
      KheTimeGroupMonitorUnAssignNonClash(tgm, KheTimeIndex(t));
  }
  KheTimeGroupMonitorFlush(tgm);
}


/*****************************************************************************/
/*                                                                           */
/*  void KheTimetableMonitorAttachAvoidUnavailableTimesMonitor(              */
/*    KHE_TIMETABLE_MONITOR tm, KHE_AVOID_UNAVAILABLE_TIMES_MONITOR m)       */
/*                                                                           */
/*  Attach avoid unavailable times monitor m to tm.                          */
/*                                                                           */
/*****************************************************************************/

static void KheTimetableMonitorAttachAvoidUnavailableTimesMonitor(
  KHE_TIMETABLE_MONITOR tm, KHE_AVOID_UNAVAILABLE_TIMES_MONITOR m)
{
  KHE_AVOID_UNAVAILABLE_TIMES_CONSTRAINT c;
  KHE_TIME_GROUP tg;  KHE_TIME_GROUP_MONITOR tgm;
  c = KheAvoidUnavailableTimesMonitorConstraint(m);
  tg = KheAvoidUnavailableTimesConstraintDomain(c);
  if( !KheTimetableMonitorContainsTimeGroupMonitor(tm, tg, &tgm) )
  {
    tgm = KheTimeGroupMonitorMake(tm, tg);
    KheTimeGroupMonitorAttachToSoln(tgm);
  }
  KheTimeGroupMonitorAttachMonitor(tgm, (KHE_MONITOR) m);
}


/*****************************************************************************/
/*                                                                           */
/*  void KheTimetableMonitorDetachAvoidUnavailableTimesMonitor(              */
/*    KHE_TIMETABLE_MONITOR tm, KHE_AVOID_UNAVAILABLE_TIMES_MONITOR m)       */
/*                                                                           */
/*  Detach avoid unavailable times monitor m from tm.                        */
/*                                                                           */
/*  Implementation note.  This function is called, among other things,       */
/*  when deleting m.  At that point, the time group monitor it relies        */
/*  on may already be deleted.  So we don't assume that it is present.       */
/*                                                                           */
/*****************************************************************************/

static void KheTimetableMonitorDetachAvoidUnavailableTimesMonitor(
  KHE_TIMETABLE_MONITOR tm, KHE_AVOID_UNAVAILABLE_TIMES_MONITOR m)
{
  KHE_AVOID_UNAVAILABLE_TIMES_CONSTRAINT c;
  KHE_TIME_GROUP tg;  KHE_TIME_GROUP_MONITOR tgm;
  c = KheAvoidUnavailableTimesMonitorConstraint(m);
  tg = KheAvoidUnavailableTimesConstraintDomain(c);
  if( KheTimetableMonitorContainsTimeGroupMonitor(tm, tg, &tgm) )
    KheTimeGroupMonitorDetachMonitor(tgm, (KHE_MONITOR) m);
}


/*****************************************************************************/
/*                                                                           */
/*  void KheTimetableMonitorAttachLimitIdleTimesMonitor(                     */
/*    KHE_TIMETABLE_MONITOR tm, KHE_LIMIT_IDLE_TIMES_MONITOR m)              */
/*                                                                           */
/*  Attach limit idle times monitor m to tm.                                 */
/*                                                                           */
/*****************************************************************************/

static void KheTimetableMonitorAttachLimitIdleTimesMonitor(
  KHE_TIMETABLE_MONITOR tm, KHE_LIMIT_IDLE_TIMES_MONITOR m)
{
  KHE_TIME_GROUP_MONITOR tgm;  KHE_TIME_GROUP tg;  int i;
  KHE_LIMIT_IDLE_TIMES_CONSTRAINT c;
  c = KheLimitIdleTimesMonitorConstraint(m);
  for( i = 0;  i < KheLimitIdleTimesConstraintTimeGroupCount(c);  i++ )
  {
    tg = KheLimitIdleTimesConstraintTimeGroup(c, i);
    if( !KheTimetableMonitorContainsTimeGroupMonitor(tm, tg, &tgm) )
    {
      tgm = KheTimeGroupMonitorMake(tm, tg);
      KheTimeGroupMonitorAttachToSoln(tgm);
    }
    KheTimeGroupMonitorAttachMonitor(tgm, (KHE_MONITOR) m);
  }
}


/*****************************************************************************/
/*                                                                           */
/*  void KheTimetableMonitorDetachLimitIdleTimesMonitor(                     */
/*    KHE_TIMETABLE_MONITOR tm, KHE_LIMIT_IDLE_TIMES_MONITOR m)              */
/*                                                                           */
/*  Detach limit idle times monitor m from tm.                               */
/*                                                                           */
/*  Implementation note.  This function is called, among other things,       */
/*  when deleting m.  At that point, the time group monitors it relies       */
/*  on may already be deleted.  So we don't assume that they are present.    */
/*                                                                           */
/*****************************************************************************/

static void KheTimetableMonitorDetachLimitIdleTimesMonitor(
  KHE_TIMETABLE_MONITOR tm, KHE_LIMIT_IDLE_TIMES_MONITOR m)
{
  KHE_TIME_GROUP_MONITOR tgm;  KHE_TIME_GROUP tg;  int i;
  KHE_LIMIT_IDLE_TIMES_CONSTRAINT c;
  c = KheLimitIdleTimesMonitorConstraint(m);
  for( i = 0;  i < KheLimitIdleTimesConstraintTimeGroupCount(c);  i++ )
  {
    tg = KheLimitIdleTimesConstraintTimeGroup(c, i);
    if( KheTimetableMonitorContainsTimeGroupMonitor(tm, tg, &tgm) )
      KheTimeGroupMonitorDetachMonitor(tgm, (KHE_MONITOR) m);
  }
}


/*****************************************************************************/
/*                                                                           */
/*  void KheTimetableMonitorAttachClusterBusyTimesMonitor(                   */
/*    KHE_TIMETABLE_MONITOR tm, KHE_CLUSTER_BUSY_TIMES_MONITOR m)            */
/*                                                                           */
/*  Attach cluster busy times monitor m to tm.                               */
/*                                                                           */
/*****************************************************************************/

static void KheTimetableMonitorAttachClusterBusyTimesMonitor(
  KHE_TIMETABLE_MONITOR tm, KHE_CLUSTER_BUSY_TIMES_MONITOR m)
{
  KHE_TIME_GROUP_MONITOR tgm;  KHE_TIME_GROUP tg;
  KHE_CLUSTER_BUSY_TIMES_CONSTRAINT c;  int i;
  c = KheClusterBusyTimesMonitorConstraint(m);
  for( i = 0;  i < KheClusterBusyTimesConstraintTimeGroupCount(c);  i++ )
  {
    tg = KheClusterBusyTimesConstraintTimeGroup(c, i);
    if( !KheTimetableMonitorContainsTimeGroupMonitor(tm, tg, &tgm) )
    {
      tgm = KheTimeGroupMonitorMake(tm, tg);
      KheTimeGroupMonitorAttachToSoln(tgm);
    }
    KheTimeGroupMonitorAttachMonitor(tgm, (KHE_MONITOR) m);
  }
}


/*****************************************************************************/
/*                                                                           */
/*  void KheTimetableMonitorDetachClusterBusyTimesMonitor(                   */
/*    KHE_TIMETABLE_MONITOR tm, KHE_CLUSTER_BUSY_TIMES_MONITOR m)            */
/*                                                                           */
/*  Detach cluster busy times monitor m from tm.                             */
/*                                                                           */
/*  Implementation note.  This function is called, among other things,       */
/*  when deleting m.  At that point, the time group monitors it relies       */
/*  on may already be deleted.  So we don't assume that they are present.    */
/*                                                                           */
/*****************************************************************************/

static void KheTimetableMonitorDetachClusterBusyTimesMonitor(
  KHE_TIMETABLE_MONITOR tm, KHE_CLUSTER_BUSY_TIMES_MONITOR m)
{
  KHE_TIME_GROUP_MONITOR tgm;  KHE_TIME_GROUP tg;
  KHE_CLUSTER_BUSY_TIMES_CONSTRAINT c;  int i;
  c = KheClusterBusyTimesMonitorConstraint(m);
  for( i = 0;  i < KheClusterBusyTimesConstraintTimeGroupCount(c);  i++ )
  {
    tg = KheClusterBusyTimesConstraintTimeGroup(c, i);
    if( KheTimetableMonitorContainsTimeGroupMonitor(tm, tg, &tgm) )
      KheTimeGroupMonitorDetachMonitor(tgm, (KHE_MONITOR) m);
  }
}


/*****************************************************************************/
/*                                                                           */
/*  void KheTimetableMonitorAttachLimitBusyTimesMonitor(                     */
/*    KHE_TIMETABLE_MONITOR tm, KHE_LIMIT_BUSY_TIMES_MONITOR m)              */
/*                                                                           */
/*  Attach limit busy times monitor m to tm.                                 */
/*                                                                           */
/*****************************************************************************/

static void KheTimetableMonitorAttachLimitBusyTimesMonitor(
  KHE_TIMETABLE_MONITOR tm, KHE_LIMIT_BUSY_TIMES_MONITOR m)
{
  KHE_TIME_GROUP_MONITOR tgm;  KHE_TIME_GROUP tg;
  KHE_LIMIT_BUSY_TIMES_CONSTRAINT c;  int i;
  /* MArrayAddLast(tm->other_monitors, (KHE_MONITOR) m); */
  if( DEBUG1 )
    fprintf(stderr, "[ KheTimetableMonitorAttachLimitBusyTimesMonitor()\n");
  c = KheLimitBusyTimesMonitorConstraint(m);
  for( i = 0;  i < KheLimitBusyTimesConstraintTimeGroupCount(c);  i++ )
  {
    tg = KheLimitBusyTimesConstraintTimeGroup(c, i);
    if( !KheTimetableMonitorContainsTimeGroupMonitor(tm, tg, &tgm) )
    {
      tgm = KheTimeGroupMonitorMake(tm, tg);
      KheTimeGroupMonitorAttachToSoln(tgm);
    }
    KheTimeGroupMonitorAttachMonitor(tgm, (KHE_MONITOR) m);
  }
  /* KheLimitBusyTimesMonitorFlush(m); */
  if( DEBUG1 )
    fprintf(stderr, "] KheTimetableMonitorAttachLimitBusyTimesMonitor\n");
}


/*****************************************************************************/
/*                                                                           */
/*  void KheTimetableMonitorDetachLimitBusyTimesMonitor(                     */
/*    KHE_TIMETABLE_MONITOR tm, KHE_LIMIT_BUSY_TIMES_MONITOR m)              */
/*                                                                           */
/*  Detach limit busy times monitor m from tm.                               */
/*                                                                           */
/*  Implementation note.  This function is called, among other things,       */
/*  when deleting m.  At that point, the time group monitors it relies       */
/*  on may already be deleted.  So we don't assume that they are present.    */
/*                                                                           */
/*****************************************************************************/

static void KheTimetableMonitorDetachLimitBusyTimesMonitor(
  KHE_TIMETABLE_MONITOR tm, KHE_LIMIT_BUSY_TIMES_MONITOR m)
{
  KHE_TIME_GROUP_MONITOR tgm;  KHE_TIME_GROUP tg;
  KHE_LIMIT_BUSY_TIMES_CONSTRAINT c;  int i;
  if( DEBUG1 )
    fprintf(stderr, "[ KheTimetableMonitorDetachLimitBusyTimesMonitor()\n");
  c = KheLimitBusyTimesMonitorConstraint(m);
  for( i = 0;  i < KheLimitBusyTimesConstraintTimeGroupCount(c);  i++ )
  {
    tg = KheLimitBusyTimesConstraintTimeGroup(c, i);
    if( KheTimetableMonitorContainsTimeGroupMonitor(tm, tg, &tgm) )
      KheTimeGroupMonitorDetachMonitor(tgm, (KHE_MONITOR) m);
  }
  if( DEBUG1 )
    fprintf(stderr, "] KheTimetableMonitorDetachLimitBusyTimesConstraint\n");
}


/*****************************************************************************/
/*                                                                           */
/*  void KheTimetableMonitorAttachMonitor(KHE_TIMETABLE_MONITOR tm,          */
/*    KHE_MONITOR m)                                                         */
/*                                                                           */
/*  Attach m to tm.  As far as callers are concerned, all monitors are       */
/*  the same; but tm knows that they need individual handling.               */
/*                                                                           */
/*****************************************************************************/

void KheTimetableMonitorAttachMonitor(KHE_TIMETABLE_MONITOR tm, KHE_MONITOR m)
{
  /* make sure tm is attached */
  MAssert(tm->tag == KHE_TIMETABLE_MONITOR_TAG,
    "KheTimetableMonitorAttachMonitor internal error");
  if( !KheMonitorAttachedToSoln((KHE_MONITOR) tm) )
    KheTimetableMonitorAttachToSoln(tm);

  /* attach m to tm */
  switch( KheMonitorTag(m) )
  {
    /* ***
    case KHE_SPREAD_EVENTS_MONITOR_TAG:

      KheTimetableMonitorAttachSpreadEventsMonitor(tm,
	(KHE_SPREAD_EVENTS_MONITOR) m);
      break;
    *** */

    case KHE_LINK_EVENTS_MONITOR_TAG:

      KheTimetableMonitorAttachLinkEventsMonitor(tm,
	(KHE_LINK_EVENTS_MONITOR) m);
      break;

    case KHE_AVOID_CLASHES_MONITOR_TAG:

      KheTimetableMonitorAttachAvoidClashesMonitor(tm,
	(KHE_AVOID_CLASHES_MONITOR) m);
      break;

    case KHE_AVOID_UNAVAILABLE_TIMES_MONITOR_TAG:

      KheTimetableMonitorAttachAvoidUnavailableTimesMonitor(tm,
	(KHE_AVOID_UNAVAILABLE_TIMES_MONITOR) m);
      break;

    case KHE_LIMIT_IDLE_TIMES_MONITOR_TAG:

      KheTimetableMonitorAttachLimitIdleTimesMonitor(tm,
	(KHE_LIMIT_IDLE_TIMES_MONITOR) m);
      break;

    case KHE_CLUSTER_BUSY_TIMES_MONITOR_TAG:

      KheTimetableMonitorAttachClusterBusyTimesMonitor(tm,
	(KHE_CLUSTER_BUSY_TIMES_MONITOR) m);
      break;

    case KHE_LIMIT_BUSY_TIMES_MONITOR_TAG:

      KheTimetableMonitorAttachLimitBusyTimesMonitor(tm,
	(KHE_LIMIT_BUSY_TIMES_MONITOR) m);
      break;

    case KHE_TIME_GROUP_MONITOR_TAG:

      KheTimetableMonitorAttachTimeGroupMonitor(tm, (KHE_TIME_GROUP_MONITOR) m);
      break;

    case KHE_SPREAD_EVENTS_MONITOR_TAG:
    case KHE_ASSIGN_RESOURCE_MONITOR_TAG:
    case KHE_ASSIGN_TIME_MONITOR_TAG:
    case KHE_SPLIT_EVENTS_MONITOR_TAG:
    case KHE_DISTRIBUTE_SPLIT_EVENTS_MONITOR_TAG:
    case KHE_PREFER_RESOURCES_MONITOR_TAG:
    case KHE_PREFER_TIMES_MONITOR_TAG:
    case KHE_AVOID_SPLIT_ASSIGNMENTS_MONITOR_TAG:
    case KHE_LIMIT_WORKLOAD_MONITOR_TAG:
    case KHE_TIMETABLE_MONITOR_TAG:
    default:

      MAssert(false,
	"KheTimetableMonitorAttachMonitor given illegal monitor type");
      break;
  }
}


/*****************************************************************************/
/*                                                                           */
/*  void KheTimetableMonitorDetachMonitor(KHE_TIMETABLE_MONITOR tm,          */
/*    KHE_MONITOR m)                                                         */
/*                                                                           */
/*  Detach m from tm.                                                        */
/*                                                                           */
/*****************************************************************************/

void KheTimetableMonitorDetachMonitor(KHE_TIMETABLE_MONITOR tm, KHE_MONITOR m)
{
  switch( KheMonitorTag(m) )
  {
    /* ***
    case KHE_SPREAD_EVENTS_MONITOR_TAG:

      KheTimetableMonitorDetachSpreadEventsMonitor(tm,
	(KHE_SPREAD_EVENTS_MONITOR) m);
      break;
    *** */

    case KHE_LINK_EVENTS_MONITOR_TAG:

      KheTimetableMonitorDetachLinkEventsMonitor(tm,
	(KHE_LINK_EVENTS_MONITOR) m);
      break;

    case KHE_AVOID_CLASHES_MONITOR_TAG:

      KheTimetableMonitorDetachAvoidClashesMonitor(tm,
	(KHE_AVOID_CLASHES_MONITOR) m);
      break;

    case KHE_AVOID_UNAVAILABLE_TIMES_MONITOR_TAG:

      KheTimetableMonitorDetachAvoidUnavailableTimesMonitor(tm,
	(KHE_AVOID_UNAVAILABLE_TIMES_MONITOR) m);
      break;

    case KHE_LIMIT_IDLE_TIMES_MONITOR_TAG:

      KheTimetableMonitorDetachLimitIdleTimesMonitor(tm,
	(KHE_LIMIT_IDLE_TIMES_MONITOR) m);
      break;

    case KHE_CLUSTER_BUSY_TIMES_MONITOR_TAG:

      KheTimetableMonitorDetachClusterBusyTimesMonitor(tm,
	(KHE_CLUSTER_BUSY_TIMES_MONITOR) m);
      break;

    case KHE_LIMIT_BUSY_TIMES_MONITOR_TAG:

      KheTimetableMonitorDetachLimitBusyTimesMonitor(tm,
	(KHE_LIMIT_BUSY_TIMES_MONITOR) m);
      break;

    case KHE_TIME_GROUP_MONITOR_TAG:

      KheTimetableMonitorDetachTimeGroupMonitor(tm, (KHE_TIME_GROUP_MONITOR) m);
      break;

    case KHE_SPREAD_EVENTS_MONITOR_TAG:
    case KHE_ASSIGN_RESOURCE_MONITOR_TAG:
    case KHE_ASSIGN_TIME_MONITOR_TAG:
    case KHE_SPLIT_EVENTS_MONITOR_TAG:
    case KHE_DISTRIBUTE_SPLIT_EVENTS_MONITOR_TAG:
    case KHE_PREFER_RESOURCES_MONITOR_TAG:
    case KHE_PREFER_TIMES_MONITOR_TAG:
    case KHE_AVOID_SPLIT_ASSIGNMENTS_MONITOR_TAG:
    case KHE_LIMIT_WORKLOAD_MONITOR_TAG:
    case KHE_TIMETABLE_MONITOR_TAG:
    default:

      MAssert(false, "KheTimetableMonitorDetachMonitor: illegal monitor type");
      break;
  }
}


/*****************************************************************************/
/*                                                                           */
/*  Submodule "attach and detach"                                            */
/*                                                                           */
/*****************************************************************************/

/*****************************************************************************/
/*                                                                           */
/*  void KheTimetableMonitorAttachToSoln(KHE_TIMETABLE_MONITOR tm)           */
/*                                                                           */
/*  Attach tm.  It is known to be currently detached.                        */
/*                                                                           */
/*****************************************************************************/

void KheTimetableMonitorAttachToSoln(KHE_TIMETABLE_MONITOR tm)
{
  if( DO_DEBUG4 )
    fprintf(stderr, "KheTimetableMonitorAttachToSoln()\n");
  tm->attached = true;
  if( tm->resource_in_soln != NULL )
    KheResourceInSolnAttachMonitor(tm->resource_in_soln, (KHE_MONITOR) tm);
  else
    KheEventInSolnAttachMonitor(tm->event_in_soln, (KHE_MONITOR) tm);
}


/*****************************************************************************/
/*                                                                           */
/*  void KheTimetableMonitorDetachFromSoln(KHE_TIMETABLE_MONITOR tm)         */
/*                                                                           */
/*  Detach tm.  It is known to be currently attached.                        */
/*                                                                           */
/*****************************************************************************/

void KheTimetableMonitorDetachFromSoln(KHE_TIMETABLE_MONITOR tm)
{
  if( DO_DEBUG4 )
    fprintf(stderr, "KheTimetableMonitorDetachFromSoln()\n");
  MAssert(MArraySize(tm->avoid_clashes_monitors) == 0 &&
      MArraySize(tm->other_monitors) == 0,
    "KheTimetableMonitorDetachFromSoln: dependent monitors attached");
  if( tm->resource_in_soln != NULL )
    KheResourceInSolnDetachMonitor(tm->resource_in_soln, (KHE_MONITOR) tm);
  else
    KheEventInSolnDetachMonitor(tm->event_in_soln, (KHE_MONITOR) tm);
  tm->attached = false;
}


/*****************************************************************************/
/*                                                                           */
/*  void KheTimetableMonitorAttachCheck(KHE_TIMETABLE_MONITOR tm)            */
/*                                                                           */
/*  Check the attachment of tm.                                              */
/*                                                                           */
/*****************************************************************************/

void KheTimetableMonitorAttachCheck(KHE_TIMETABLE_MONITOR tm)
{
  /* not sure about this! but it is the default behaviour */
  if( !KheMonitorAttachedToSoln((KHE_MONITOR) tm) )
    KheMonitorAttachToSoln((KHE_MONITOR) tm);
}


/*****************************************************************************/
/*                                                                           */
/*  Submodule "monitoring calls from KHE_EVENT_IN_SOLN"                      */
/*                                                                           */
/*****************************************************************************/

/*****************************************************************************/
/*                                                                           */
/*  void KheTimetableMonitorAddMeet(KHE_TIMETABLE_MONITOR tm, KHE_MEET meet) */
/*                                                                           */
/*  Add meet to tm, if assigned.                                             */
/*                                                                           */
/*****************************************************************************/

void KheTimetableMonitorAddMeet(KHE_TIMETABLE_MONITOR tm, KHE_MEET meet)
{
  if( KheMeetAssignedTimeIndex(meet) != -1 )
    KheTimetableMonitorAssignTime(tm, meet, KheMeetAssignedTimeIndex(meet));
}


/*****************************************************************************/
/*                                                                           */
/*  void KheTimetableMonitorDeleteMeet(KHE_TIMETABLE_MONITOR tm,             */
/*    KHE_MEET meet)                                                         */
/*                                                                           */
/*  Delete meet from tm, if assigned.                                        */
/*                                                                           */
/*****************************************************************************/

void KheTimetableMonitorDeleteMeet(KHE_TIMETABLE_MONITOR tm,
  KHE_MEET meet)
{
  if( KheMeetAssignedTimeIndex(meet) != -1 )
    KheTimetableMonitorUnAssignTime(tm, meet, KheMeetAssignedTimeIndex(meet));
}


/*****************************************************************************/
/*                                                                           */
/*  void KheTimetableMonitorSplitMeet(KHE_TIMETABLE_MONITOR tm,              */
/*    KHE_MEET meet1, KHE_MEET meet2)                                        */
/*                                                                           */
/*  Inform tm that meet1 and meet2 are splitting.                            */
/*                                                                           */
/*  Implementation note.  For the timetable, this means that some            */
/*  occurrences of meet1 have to be replaced by meet2.  Since this does not  */
/*  change the number of meets at any time, no propagation of these changes  */
/*  to the monitors attached to tm is needed.                                */
/*                                                                           */
/*****************************************************************************/

void KheTimetableMonitorSplitMeet(KHE_TIMETABLE_MONITOR tm,
  KHE_MEET meet1, KHE_MEET meet2)
{
  int i, pos;  KHE_TIME_CELL tc;
  if( KheMeetAssignedTimeIndex(meet2) != -1 )
    for( i = 0;  i < KheMeetDuration(meet2);  i++ )
    {
      tc = MArrayGet(tm->time_cells, KheMeetAssignedTimeIndex(meet2) + i);
      if( !MArrayContains(tc->meets, meet1, &pos) )
	MAssert(false, "KheTimetableMonitorSplitMeet internal error");
      MArrayPut(tc->meets, pos, meet2);
    }
}


/*****************************************************************************/
/*                                                                           */
/*  void KheTimetableMonitorMergeMeet(KHE_TIMETABLE_MONITOR tm,              */
/*    KHE_MEET meet1, KHE_MEET meet2)                                        */
/*                                                                           */
/*  Inform tm that meet1 and meet2 are merging.                              */
/*                                                                           */
/*  Implementation note.  Similarly to splitting, this means that some       */
/*  occurrences of meet2 have to be replaced by meet1, without propagation.  */
/*                                                                           */
/*****************************************************************************/

void KheTimetableMonitorMergeMeet(KHE_TIMETABLE_MONITOR tm,
  KHE_MEET meet1, KHE_MEET meet2)
{
  int i, pos;  KHE_TIME_CELL tc;
  if( KheMeetAssignedTimeIndex(meet2) != -1 )
    for( i = 0;  i < KheMeetDuration(meet2);  i++ )
    {
      tc = MArrayGet(tm->time_cells, KheMeetAssignedTimeIndex(meet2) + i);
      if( !MArrayContains(tc->meets, meet2, &pos) )
	MAssert(false, "KheTimetableMonitorMergeMeet internal error");
      MArrayPut(tc->meets, pos, meet1);
    }
}


/*****************************************************************************/
/*                                                                           */
/*  void KheTimetableMonitorAssignTime(KHE_TIMETABLE_MONITOR tm,             */
/*    KHE_MEET meet, int assigned_time_index)                                */
/*                                                                           */
/*  Update tm to add meet at assigned_time_index and on for its duration.    */
/*                                                                           */
/*  Implementation note.  As it happens, at each time we either inform       */
/*  the monitors attached to that time that the resource has become busy     */
/*  then, or else we inform all avoid clashes monitors that the resoure      */
/*  resource has a clash then.                                               */
/*                                                                           */
/*****************************************************************************/

void KheTimetableMonitorAssignTime(KHE_TIMETABLE_MONITOR tm,
  KHE_MEET meet, int assigned_time_index)
{
  int i, j;  KHE_TIME_CELL tc;  KHE_MONITOR m;  KHE_AVOID_CLASHES_MONITOR acm;
  if( DO_DEBUG4 )
    fprintf(stderr, "KheTimetableMonitorAssignTime(tm, meet, %d)\n",
      assigned_time_index);
  for( i = 0;  i < KheMeetDuration(meet);  i++ )
  {
    MAssert(0 <= assigned_time_index + i,
      "KheTimetableMonitorAssignTime internal error 1");
    MAssert(assigned_time_index + i < MArraySize(tm->time_cells),
      "KheTimetableMonitorAssignTime internal error 2");
    tc = MArrayGet(tm->time_cells, assigned_time_index + i);
    if( MArraySize(tc->meets) == 0 )
      MArrayForEach(tc->monitors, &m, &j)
	KheMonitorAssignNonClash(m, assigned_time_index + i);
    else
      MArrayForEach(tm->avoid_clashes_monitors, &acm, &j)
	KheAvoidClashesMonitorChangeClashCount(acm,
	  MArraySize(tc->meets) - 1, MArraySize(tc->meets));
    MArrayAddLast(tc->meets, meet);
  }
  MArrayForEach(tm->avoid_clashes_monitors, &acm, &i)
    KheAvoidClashesMonitorFlush(acm);
  MArrayForEach(tm->other_monitors, &m, &i)
    KheMonitorFlush(m);
}


/*****************************************************************************/
/*                                                                           */
/*  void KheTimetableMonitorUnAssignTime(KHE_TIMETABLE_MONITOR tm,           */
/*    KHE_MEET meet, int assigned_time_index)                                */
/*                                                                           */
/*  Update tm to delete meet at assigned_time_index and on for its duration. */
/*                                                                           */
/*****************************************************************************/

void KheTimetableMonitorUnAssignTime(KHE_TIMETABLE_MONITOR tm,
  KHE_MEET meet, int assigned_time_index)
{
  int i, j, pos;  KHE_TIME_CELL tc;  KHE_MONITOR m;
  KHE_AVOID_CLASHES_MONITOR acm;
  if( DO_DEBUG4 )
    fprintf(stderr, "KheTimetableMonitorUnAssignTime(tm, meet, %d)\n",
      assigned_time_index);
  for( i = 0;  i < KheMeetDuration(meet);  i++ )
  {
    tc = MArrayGet(tm->time_cells, assigned_time_index + i);
    if( !MArrayContains(tc->meets, meet, &pos) )
      MAssert(false, "KheTimetableMonitorUnAssignTime internal error)");
    MArrayRemove(tc->meets, pos);
    if( MArraySize(tc->meets) == 0 )
      MArrayForEach(tc->monitors, &m, &j)
	KheMonitorUnAssignNonClash(m, assigned_time_index + i);
    else
      MArrayForEach(tm->avoid_clashes_monitors, &acm, &j)
	KheAvoidClashesMonitorChangeClashCount(acm,
	  MArraySize(tc->meets), MArraySize(tc->meets) - 1);
  }
  MArrayForEach(tm->avoid_clashes_monitors, &acm, &i)
    KheAvoidClashesMonitorFlush(acm);
  MArrayForEach(tm->other_monitors, &m, &i)
    KheMonitorFlush(m);
}


/*****************************************************************************/
/*                                                                           */
/*  Submodule "monitoring calls from KHE_RESOURCE_IN_SOLN"                   */
/*                                                                           */
/*****************************************************************************/

/*****************************************************************************/
/*                                                                           */
/*  void KheTimetableMonitorSplitTask(KHE_TIMETABLE_MONITOR tm,              */
/*    KHE_TASK task1, KHE_TASK task2)                                        */
/*                                                                           */
/*  Inform tm that task1 and task2 are splitting.                            */
/*                                                                           */
/*****************************************************************************/

void KheTimetableMonitorSplitTask(KHE_TIMETABLE_MONITOR tm,
  KHE_TASK task1, KHE_TASK task2)
{
  KHE_MEET meet1, meet2;
  meet1 = KheTaskMeet(task1);
  meet2 = KheTaskMeet(task2);
  MAssert((meet1 == NULL) == (meet2 == NULL),
    "KheTimetableMonitorSplitTask internal error");
  if( meet1 != NULL )
    KheTimetableMonitorSplitMeet(tm, meet1, meet2);
}


/*****************************************************************************/
/*                                                                           */
/*  void KheTimetableMonitorMergeTask(KHE_TIMETABLE_MONITOR tm,              */
/*    KHE_TASK task1, KHE_TASK task2)                                        */
/*                                                                           */
/*  Inform tm that task1 and task2 are merging.                              */
/*                                                                           */
/*****************************************************************************/

void KheTimetableMonitorMergeTask(KHE_TIMETABLE_MONITOR tm,
  KHE_TASK task1, KHE_TASK task2)
{
  KHE_MEET meet1, meet2;
  meet1 = KheTaskMeet(task1);
  meet2 = KheTaskMeet(task2);
  MAssert((meet1 == NULL) == (meet2 == NULL),
    "KheTimetableMonitorMergeTask internal error");
  if( meet1 != NULL )
    KheTimetableMonitorMergeMeet(tm, meet1, meet2);
}


/*****************************************************************************/
/*                                                                           */
/*  void KheTimetableMonitorTaskAssignTime(KHE_TIMETABLE_MONITOR tm,         */
/*    KHE_TASK task, int assigned_time_index)                                */
/*                                                                           */
/*  Let m know that task has just been assigned this time.                   */
/*                                                                           */
/*****************************************************************************/

void KheTimetableMonitorTaskAssignTime(KHE_TIMETABLE_MONITOR tm,
  KHE_TASK task, int assigned_time_index)
{
  KHE_MEET meet;
  meet = KheTaskMeet(task);
  MAssert(meet != NULL, "KheTimetableMonitorTaskAssignTime internal error");
  KheTimetableMonitorAssignTime(tm, meet, assigned_time_index);
}


/*****************************************************************************/
/*                                                                           */
/* void KheTimetableMonitorTaskUnAssignTime(KHE_TIMETABLE_MONITOR tm,        */
/*    KHE_TASK task, int assigned_time_index)                                */
/*                                                                           */
/*  Let m know that task has just been unassigned this time.                 */
/*                                                                           */
/*****************************************************************************/

void KheTimetableMonitorTaskUnAssignTime(KHE_TIMETABLE_MONITOR tm,
  KHE_TASK task, int assigned_time_index)
{
  KHE_MEET meet;
  meet = KheTaskMeet(task);
  MAssert(meet != NULL, "KheTimetableMonitorTaskUnAssignTime internal error");
  KheTimetableMonitorUnAssignTime(tm, meet, assigned_time_index);
}


/*****************************************************************************/
/*                                                                           */
/*  void KheTimetableMonitorAssignResource(KHE_TIMETABLE_MONITOR tm,         */
/*    KHE_TASK task, KHE_RESOURCE r)                                         */
/*                                                                           */
/*  Add task to tm, but only if it has an assigned time.                     */
/*                                                                           */
/*****************************************************************************/

void KheTimetableMonitorAssignResource(KHE_TIMETABLE_MONITOR tm,
  KHE_TASK task, KHE_RESOURCE r)
{
  KHE_MEET meet;
  meet = KheTaskMeet(task);
  if( meet != NULL && KheMeetAssignedTimeIndex(meet) != -1 )
    KheTimetableMonitorAssignTime(tm, meet, KheMeetAssignedTimeIndex(meet));
}


/*****************************************************************************/
/*                                                                           */
/*  void KheTimetableMonitorUnAssignResource(KHE_TIMETABLE_MONITOR tm,       */
/*    KHE_TASK task, KHE_RESOURCE r)                                         */
/*                                                                           */
/*  Delete task from tm, but only if it has an assigned time.                */
/*                                                                           */
/*****************************************************************************/

void KheTimetableMonitorUnAssignResource(KHE_TIMETABLE_MONITOR tm,
  KHE_TASK task, KHE_RESOURCE r)
{
  KHE_MEET meet;
  meet = KheTaskMeet(task);
  if( meet != NULL && KheMeetAssignedTimeIndex(meet) != -1 )
    KheTimetableMonitorUnAssignTime(tm, meet, KheMeetAssignedTimeIndex(meet));
}


/*****************************************************************************/
/*                                                                           */
/*  Submodule "deviations"                                                   */
/*                                                                           */
/*****************************************************************************/

/*****************************************************************************/
/*                                                                           */
/*  int KheTimetableMonitorDeviationCount(KHE_TIMETABLE_MONITOR m)           */
/*                                                                           */
/*  Return the number of deviations of m (0 in this case).                   */
/*                                                                           */
/*****************************************************************************/

int KheTimetableMonitorDeviationCount(KHE_TIMETABLE_MONITOR m)
{
  return 0;
}


/*****************************************************************************/
/*                                                                           */
/*  int KheTimetableMonitorDeviation(KHE_TIMETABLE_MONITOR m, int i)         */
/*                                                                           */
/*  Return the i'th deviation of m (there are none, so it's an error).       */
/*                                                                           */
/*****************************************************************************/

int KheTimetableMonitorDeviation(KHE_TIMETABLE_MONITOR m, int i)
{
  MAssert(false, "KheTimetableMonitorDeviation: i out of range");
  return 0;  /* keep compiler happy */
}


/*****************************************************************************/
/*                                                                           */
/*  char *KheTimetableMonitorDeviationDescription(                           */
/*    KHE_TIMETABLE_MONITOR m, int i)                                        */
/*                                                                           */
/*  Return a description of the i'th deviation of m (there are none, so      */
/*  it's an error).                                                          */
/*                                                                           */
/*****************************************************************************/

char *KheTimetableMonitorDeviationDescription(
  KHE_TIMETABLE_MONITOR m, int i)
{
  MAssert(false, "KheTimetableMonitorDeviationDescription: i out of range");
  return NULL;  /* keep compiler happy */
}


/*****************************************************************************/
/*                                                                           */
/*  Submodule "debug"                                                        */
/*                                                                           */
/*****************************************************************************/

typedef struct khe_week_rec {
  KHE_TIME_GROUP	week_tg;		/* optional; defines week    */
  int			max_times;		/* on any one day            */
  ARRAY_KHE_TIME_GROUP	days;			/* days of the week          */
} *KHE_WEEK;

typedef MARRAY(KHE_WEEK) ARRAY_KHE_WEEK;


/*****************************************************************************/
/*                                                                           */
/*  KHE_WEEK KheWeekMake(KHE_TIME_GROUP week_tg)                             */
/*                                                                           */
/*  Make a new week object with these attributes.                            */
/*                                                                           */
/*****************************************************************************/

static KHE_WEEK KheWeekMake(KHE_TIME_GROUP week_tg)
{
  KHE_WEEK res;
  MMake(res);
  res->week_tg = week_tg;
  res->max_times = 0;
  MArrayInit(res->days);
  return res;
}


/*****************************************************************************/
/*                                                                           */
/*  void KheWeekDelete(KHE_WEEK week)                                        */
/*                                                                           */
/*  Delete week.                                                             */
/*                                                                           */
/*****************************************************************************/

static void KheWeekDelete(KHE_WEEK week)
{
  MArrayFree(week->days);
  MFree(week);
}


/*****************************************************************************/
/*                                                                           */
/*  void KhePrint(char *str, int cell_width, FILE *fp)                       */
/*                                                                           */
/*  Print str onto fp, with a margin, taking care not to overrun.            */
/*                                                                           */
/*****************************************************************************/

static void KhePrint(char *str, bool in_cell, int cell_width, FILE *fp)
{
  char buff[100];
  snprintf(buff, cell_width - 3, "%s", str);
  fprintf(fp, "%c %-*s ", in_cell ? '|' : ' ', cell_width - 3, buff);
}


/*****************************************************************************/
/*                                                                           */
/*  void KhePrintRule(int cell_width, FILE *fp)                              */
/*                                                                           */
/*  Print a rule of the given cell_width onto fp.                            */
/*                                                                           */
/*****************************************************************************/

static void KhePrintRule(int cell_width, FILE *fp)
{
  int i;
  fprintf(fp, "+");
  for( i = 0;  i < cell_width - 1;  i++ )
    fprintf(fp, "-");
}


/*****************************************************************************/
/*                                                                           */
/*  void KhePrintRuleLine(int cells, int cell_width, int indent, FILE *fp)   */
/*                                                                           */
/*  Print a full-width rule, for this many cells of this width, onto fp      */
/*  with the given indent.                                                   */
/*                                                                           */
/*****************************************************************************/

static void KhePrintRuleLine(int cells, int cell_width, int indent, FILE *fp)
{
  int i;
  fprintf(fp, "%*s", indent, "");
  for( i = 0;  i < cells;  i++ )
    KhePrintRule(cell_width, fp);
  fprintf(fp, "+\n");
}


/*****************************************************************************/
/*                                                                           */
/*  void KhePrintBlankLine(int cells, int cell_width, int indent, FILE *fp)  */
/*                                                                           */
/*  Print a full-width rule, for this many cells of this width, onto fp      */
/*  with the given indent.                                                   */
/*                                                                           */
/*****************************************************************************/

/* *** currently unused
static void KhePrintBlankLine(int cells, int cell_width, int indent, FILE *fp)
{
  int i;
  fprintf(fp, "%*s", indent, "");
  for( i = 0;  i < cells;  i++ )
    KhePrint("", true, cell_width, fp);
  fprintf(fp, "|\n");
}
*** */


/*****************************************************************************/
/*                                                                           */
/*  void KheTimetableMonitorPrint(KHE_TIMETABLE_MONITOR tm, KHE_WEEK week,   */
/*    int time_index, int indent, FILE *fp)                                  */
/*                                                                           */
/*  Print one row of the timetable, the one for time time_index of week.     */
/*                                                                           */
/*****************************************************************************/

static void KheTimetableMonitorPrint(KHE_TIMETABLE_MONITOR tm, KHE_WEEK week,
  int time_index, int cell_width, int indent, FILE *fp)
{
  int i, j, content_lines;  KHE_TIME_GROUP day;  KHE_TIME t;  char *str;
  KHE_MEET meet;

  /* print the top line and the blank line just under it */
  KhePrintRuleLine(MArraySize(week->days), cell_width, indent, fp);
  /* KhePrintBlankLine(MArraySize(week->days), cell_width, indent, fp); */

  /* find the number of content lines to print */
  content_lines = 0;
  MArrayForEach(week->days, &day, &i)
    if( time_index < KheTimeGroupTimeCount(day) )
    {
      t = KheTimeGroupTime(day, time_index);
      if( content_lines < KheTimetableMonitorTimeMeetCount(tm, t) )
	content_lines = KheTimetableMonitorTimeMeetCount(tm, t);
    }

  /* if there are any content lines, print them plus a blank line */
  if( content_lines > 0 )
  {
    for( j = 0;  j < content_lines;  j++ )
    {
      fprintf(fp, "%*s", indent, "");
      MArrayForEach(week->days, &day, &i)
      {
	/* print something for this day/j, even if just a blank line */
	if( time_index >= KheTimeGroupTimeCount(day) )
	  str = "";
	else
	{
	  t = KheTimeGroupTime(day, time_index);
	  if( j >= KheTimetableMonitorTimeMeetCount(tm, t) )
	    str = "";
	  else
	  {
	    meet = KheTimetableMonitorTimeMeet(tm, t, j);
	    str = KheMeetEvent(meet) == NULL ? "?" :
	      KheEventId(KheMeetEvent(meet)) == NULL ? "-" :
	      KheEventId(KheMeetEvent(meet));
	  }
	}
	KhePrint(str, true, cell_width, fp);
      }
      fprintf(fp, "|\n");
    }
    /* KhePrintBlankLine(MArraySize(week->days), cell_width, indent, fp); */
  }
}


/*****************************************************************************/
/*                                                                           */
/*  void KheTimetableMonitorPrintTimetable(KHE_TIMETABLE_MONITOR tm,         */
/*    int cell_width, int indent, FILE *fp)                                  */
/*                                                                           */
/*  Print tm onto fp with the given indent, in a format that looks like      */
/*  an actual timetable (there must be Days, at least, in the instance).     */
/*                                                                           */
/*****************************************************************************/

void KheTimetableMonitorPrintTimetable(KHE_TIMETABLE_MONITOR tm,
  int cell_width, int indent, FILE *fp)
{
  ARRAY_KHE_WEEK weeks;  KHE_WEEK week;  KHE_INSTANCE ins;  int i, j, count;
  KHE_TIME_GROUP tg;

  /* find the weeks; make just one if there are no weeks */
  ins = KheSolnInstance(tm->soln);
  MArrayInit(weeks);
  for( i = 0;  i < KheInstanceTimeGroupCount(ins);  i++ )
  {
    tg = KheInstanceTimeGroup(ins, i);
    if( KheTimeGroupKind(tg) == KHE_TIME_GROUP_KIND_WEEK )
      MArrayAddLast(weeks, KheWeekMake(tg));
  }
  if( MArraySize(weeks) == 0 )
    MArrayAddLast(weeks, KheWeekMake(NULL));

  /* add the days to the weeks */
  for( i = 0;  i < KheInstanceTimeGroupCount(ins);  i++ )
  {
    tg = KheInstanceTimeGroup(ins, i);
    if( KheTimeGroupKind(tg) == KHE_TIME_GROUP_KIND_DAY )
    {
      count = 0;
      MArrayForEach(weeks, &week, &j)
	if( week->week_tg == NULL || KheTimeGroupSubset(tg, week->week_tg) )
	{
	  MArrayAddLast(week->days, tg);
	  count++;
	  if( week->max_times < KheTimeGroupTimeCount(tg) )
	    week->max_times = KheTimeGroupTimeCount(tg);
	}
      if( count != 1 )
      {
	fprintf(fp, "%*sKheTimetableMonitorPrintTimetable: day ",
	  indent, "");
	KheTimeGroupDebug(tg, 1, -1, fp);
	fprintf(fp, " lies in %d weeks\n", count);
      }
    }
  }

  /* print the timetable, week by week */
  MArrayForEach(weeks, &week, &i)
  {
    /* blank line between weeks */
    if( i > 0 )
      fprintf(fp, "\n");

    /* header line containing the names of the days */
    fprintf(fp, "%*s", indent, "");
    MArrayForEach(week->days, &tg, &j)
      KhePrint(KheTimeGroupId(tg) == NULL ? "-" : KheTimeGroupId(tg),
	false, cell_width, fp);
    fprintf(fp, "\n");

    /* one row for each time of the day */
    for( j = 0;  j < week->max_times;  j++ )
      KheTimetableMonitorPrint(tm, week, j, cell_width, indent, fp);

    /* and a finishing rule */
    KhePrintRuleLine(MArraySize(week->days), cell_width, indent, fp);
  }

  /* delete the memory used */
  while( MArraySize(weeks) > 0 )
    KheWeekDelete(MArrayRemoveLast(weeks));
}


/*****************************************************************************/
/*                                                                           */
/*  void KheTimetableMonitorDebug(KHE_TIMETABLE_MONITOR tm, int verbosity,   */
/*    int indent, FILE *fp)                                                  */
/*                                                                           */
/*  Debug print of timetable tm onto fp with this verbosity and indent.      */
/*                                                                           */
/*****************************************************************************/

void KheTimetableMonitorDebug(KHE_TIMETABLE_MONITOR tm, int verbosity,
  int indent, FILE *fp)
{
  KHE_TIME_CELL tc;  KHE_MONITOR m;  int i;
  if( indent >= 0 && verbosity >= 1 )
  {
    KheMonitorDebugBegin((KHE_MONITOR) tm, indent, fp);
    fprintf(fp, " %s\n", tm->resource_in_soln != NULL ?
      KheResourceInSolnId(tm->resource_in_soln) :
      KheEventInSolnId(tm->event_in_soln));
    MArrayForEach(tm->other_monitors, &m, &i)
      KheMonitorDebug(m, 1, indent + 2, fp);
    MArrayForEach(tm->time_cells, &tc, &i)
      if( verbosity >= 3 || MArraySize(tc->meets) > 0 )
	KheTimeCellDebug(tc, verbosity, indent + 2, fp);
    KheMonitorDebugEnd((KHE_MONITOR) tm, false, indent, fp);
  }
}
