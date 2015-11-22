
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
/*  FILE:         khe_event_in_soln.c                                        */
/*  DESCRIPTION:  A soln object that monitors everything about one event     */
/*                                                                           */
/*****************************************************************************/
#include "khe_interns.h"
#define DEBUG1 0
#define DEBUG2 0


/*****************************************************************************/
/*                                                                           */
/*  KHE_EVENT_IN_SOLN                                                        */
/*                                                                           */
/*****************************************************************************/

struct khe_event_in_soln_rec {
  KHE_SOLN			soln;			/* encl solution     */
  KHE_EVENT			event;			/* original event    */
  ARRAY_KHE_MEET		meets;			/* its meets   */
  ARRAY_KHE_EVENT_RESOURCE_IN_SOLN event_resources_in_soln;
  KHE_TIMETABLE_MONITOR		timetable_monitor;	/* timetable         */
  ARRAY_KHE_MONITOR		all_monitors;		/* all monitors      */
  ARRAY_KHE_MONITOR		attached_monitors;	/* attached monitors */
  ARRAY_KHE_MONITOR	attached_time_asst_monitors;	/* time asst mon's   */
  KHE_EVENT_IN_SOLN		copy;			/* used when copying */
};


/*****************************************************************************/
/*                                                                           */
/*  KHE_EVENT_IN_SOLN KheEventInSolnMake(KHE_SOLN soln, KHE_EVENT e)         */
/*                                                                           */
/*  Make a new event monitor for soln, corresponding to e, initially with    */
/*  no meets, but with one event_resource_in_soln object for each event      */
/*  resource of e.                                                           */
/*                                                                           */
/*****************************************************************************/

KHE_EVENT_IN_SOLN KheEventInSolnMake(KHE_SOLN soln, KHE_EVENT e)
{
  KHE_EVENT_IN_SOLN res;  int i;
  KHE_EVENT_RESOURCE_IN_SOLN ers;
  MMake(res);
  MAssert(e != NULL, "KheEventInSolnMake: internal error (e is NULL)");
  res->soln = soln;
  res->event = e;
  MArrayInit(res->meets);
  MArrayInit(res->event_resources_in_soln);
  for( i = 0;  i < KheEventResourceCount(e);  i++ )
  {
    ers = KheEventResourceInSolnMake(res, KheEventResource(e, i));
    MArrayAddLast(res->event_resources_in_soln, ers);
  }
  res->timetable_monitor = KheTimetableMonitorMake(soln, NULL, res);
  MArrayInit(res->all_monitors);
  MArrayInit(res->attached_monitors);
  MArrayInit(res->attached_time_asst_monitors);
  res->copy = NULL;
  return res;
}


/*****************************************************************************/
/*                                                                           */
/*  int KheEventInSolnTotalDuration(KHE_EVENT_IN_SOLN es)                    */
/*                                                                           */
/*  Return the total duration of the meets of es.                            */
/*                                                                           */
/*****************************************************************************/

static int KheEventInSolnTotalDuration(KHE_EVENT_IN_SOLN es)
{
  int res, i;  KHE_MEET meet;
  res = 0;
  MArrayForEach(es->meets, &meet, &i)
    res += KheMeetDuration(meet);
  return res;
}


/*****************************************************************************/
/*                                                                           */
/*  bool KheEventInSolnMakeCompleteRepresentation(KHE_EVENT_IN_SOLN es,      */
/*    KHE_EVENT *problem_event)                                              */
/*                                                                           */
/*  Carry out the specification of KheSolnMakeCompleteRepresentation on es.  */
/*                                                                           */
/*****************************************************************************/

bool KheEventInSolnMakeCompleteRepresentation(KHE_EVENT_IN_SOLN es,
  KHE_EVENT *problem_event)
{
  int j, k, total_durn;  KHE_MEET meet;  KHE_TASK task;
  KHE_EVENT_RESOURCE er;
  if( DEBUG2 )
  {
    fprintf(stderr, "[ KheEventInSolnMakeCompleteRepresentation(%s)\n",
      KheEventId(es->event) != NULL ? KheEventId(es->event) : "-");
    fprintf(stderr, "  event: duration %d; es: %d sub-events, %d duration\n",
      KheEventDuration(es->event), MArraySize(es->meets),
      KheEventInSolnTotalDuration(es));
    KheEventInSolnDebug(es, 2, 2, stderr);
  }

  /* make sure es has the correct total duration */
  total_durn = KheEventInSolnTotalDuration(es);
  if( total_durn > KheEventDuration(es->event) )
  {
    *problem_event = es->event;
    return false;
  }
  else if( total_durn < KheEventDuration(es->event) )
    KheMeetMake(es->soln, KheEventDuration(es->event) - total_durn, es->event);

  /* make sure each meet has the required tasks */
  MArrayForEach(es->meets, &meet, &j)
  {
    MAssert(KheMeetSoln(meet) == es->soln,
      "KheEventInSolnMakeCompleteRepresentation internal error (%p != %p)",
      (void *) KheMeetSoln(meet), (void *) es->soln);
    for( k = 0;  k < KheEventResourceCount(es->event);  k++ )
    {
      er = KheEventResource(es->event, k);
      if( !KheMeetFindTask(meet, er, &task) )
      {
	/* must add a task corresponding to er */
	task = KheTaskMake(KheMeetSoln(meet),
	  KheEventResourceResourceType(er), meet, er);
	KheTaskSetDomain(task, KheEventResourceHardDomain(er), false);
	/* third parameter is don't care here, since no child tasks */
      }
    }
  }
  if( DEBUG2 )
  {
    fprintf(stderr, "] KheEventInSolnMakeCompleteRepresentation ret. true\n");
    KheEventInSolnDebug(es, 2, 2, stderr);
  }
  return true;
}


/*****************************************************************************/
/*                                                                           */
/*  KHE_SOLN KheEventInSolnSoln(KHE_EVENT_IN_SOLN es)                        */
/*                                                                           */
/*  Return the soln attribute of es.                                         */
/*                                                                           */
/*****************************************************************************/

KHE_SOLN KheEventInSolnSoln(KHE_EVENT_IN_SOLN es)
{
  return es->soln;
}


/*****************************************************************************/
/*                                                                           */
/*  KHE_EVENT KheEventInSolnEvent(KHE_EVENT_IN_SOLN es)                      */
/*                                                                           */
/*  Return the event attribute of es.                                        */
/*                                                                           */
/*****************************************************************************/

KHE_EVENT KheEventInSolnEvent(KHE_EVENT_IN_SOLN es)
{
  return es->event;
}


/*****************************************************************************/
/*                                                                           */
/*  char *KheEventInSolnId(KHE_EVENT_IN_SOLN es)                             */
/*                                                                           */
/*  Return the Id of the event that es is for, or "-" if none.               */
/*                                                                           */
/*****************************************************************************/

char *KheEventInSolnId(KHE_EVENT_IN_SOLN es)
{
  return KheEventId(es->event) == NULL ? "-" : KheEventId(es->event);
}


/*****************************************************************************/
/*                                                                           */
/*  void KheEventInSolnDelete(KHE_EVENT_IN_SOLN es)                          */
/*                                                                           */
/*  Delete es and everything below it.                                       */
/*                                                                           */
/*****************************************************************************/

void KheEventInSolnDelete(KHE_EVENT_IN_SOLN es)
{
  KHE_EVENT_RESOURCE_IN_SOLN ers;  int i;

  /* meets should be already deleted */
  MAssert(MArraySize(es->meets) == 0,
    "KheEventInSolnDelete internal error");
  MArrayFree(es->meets);

  /* delete event resource in soln objects */
  MArrayForEach(es->event_resources_in_soln, &ers, &i)
    KheEventResourceInSolnDelete(ers);

  /* timetables and other monitors deleted separately; but free arrays now */
  MArrayFree(es->all_monitors);
  MArrayFree(es->attached_monitors);
  MArrayFree(es->attached_time_asst_monitors);

  MFree(es);
}


/*****************************************************************************/
/*                                                                           */
/*  KHE_EVENT_IN_SOLN KheEventInSolnCopyPhase1(KHE_EVENT_IN_SOLN es)         */
/*                                                                           */
/*  Carry out Phase 1 of the operation of copying es.                        */
/*                                                                           */
/*****************************************************************************/

KHE_EVENT_IN_SOLN KheEventInSolnCopyPhase1(KHE_EVENT_IN_SOLN es)
{
  KHE_EVENT_IN_SOLN copy;  KHE_MEET meet;  int i;
  KHE_EVENT_RESOURCE_IN_SOLN ers;  KHE_MONITOR m;
  if( es->copy == NULL )
  {
    MMake(copy);
    es->copy = copy;
    copy->soln = KheSolnCopyPhase1(es->soln);
    copy->event = es->event;
    MArrayInit(copy->meets);
    MArrayForEach(es->meets, &meet, &i)
      MArrayAddLast(copy->meets, KheMeetCopyPhase1(meet));
    MArrayInit(copy->event_resources_in_soln);
    MArrayForEach(es->event_resources_in_soln, &ers, &i)
      MArrayAddLast(copy->event_resources_in_soln,
	KheEventResourceInSolnCopyPhase1(ers));
    copy->timetable_monitor =
      KheTimetableMonitorCopyPhase1(es->timetable_monitor);
    MArrayInit(copy->all_monitors);
    MArrayForEach(es->all_monitors, &m, &i)
      MArrayAddLast(copy->all_monitors, KheMonitorCopyPhase1(m));
    MArrayInit(copy->attached_monitors);
    MArrayForEach(es->attached_monitors, &m, &i)
      MArrayAddLast(copy->attached_monitors, KheMonitorCopyPhase1(m));
    MArrayInit(copy->attached_time_asst_monitors);
    MArrayForEach(es->attached_time_asst_monitors, &m, &i)
      MArrayAddLast(copy->attached_time_asst_monitors, KheMonitorCopyPhase1(m));
    copy->copy = NULL;
  }
  return es->copy;
}


/*****************************************************************************/
/*                                                                           */
/*  void KheEventInSolnCopyPhase2(KHE_EVENT_IN_SOLN es)                      */
/*                                                                           */
/*  Carry out Phase 2 of the operation of copying es.                        */
/*                                                                           */
/*****************************************************************************/

void KheEventInSolnCopyPhase2(KHE_EVENT_IN_SOLN es)
{
  int i;  KHE_EVENT_RESOURCE_IN_SOLN ers;  KHE_MONITOR m;
  if( es->copy != NULL )
  {
    es->copy = NULL;
    MArrayForEach(es->event_resources_in_soln, &ers, &i)
      KheEventResourceInSolnCopyPhase2(ers);
    KheTimetableMonitorCopyPhase2(es->timetable_monitor);
    MArrayForEach(es->all_monitors, &m, &i)
      KheMonitorCopyPhase2(m);
    MArrayForEach(es->attached_monitors, &m, &i)
      KheMonitorCopyPhase2(m);
    MArrayForEach(es->attached_time_asst_monitors, &m, &i)
      KheMonitorCopyPhase2(m);
  }
}


/*****************************************************************************/
/*                                                                           */
/*  Submodule "consistency check"                                            */
/*                                                                           */
/*****************************************************************************/

/*****************************************************************************/
/*                                                                           */
/*  void KheEventInSolnConsistencyCheck(KHE_EVENT_IN_SOLN es)                */
/*                                                                           */
/*  Consistency check of es, written to track down a memory bug.             */
/*                                                                           */
/*****************************************************************************/

/* ***
void KheEventInSolnConsistencyCheck(KHE_EVENT_IN_SOLN es)
{
  KHE_MEET se;  int i;
  MArrayForEach(es->meets, &se, &i)
    MAssert(KheSolnEventEventInSoln(se) == es,
      "KheEventInSolnConsistencyCheck internal error");
}
*** */


/*****************************************************************************/
/*                                                                           */
/*  Submodule "meets"                                                        */
/*                                                                           */
/*****************************************************************************/

/*****************************************************************************/
/*                                                                           */
/*  void KheEventInSolnAddMeet(KHE_EVENT_IN_SOLN es, KHE_MEET meet)          */
/*                                                                           */
/*  Add meet to es.                                                          */
/*                                                                           */
/*****************************************************************************/

void KheEventInSolnAddMeet(KHE_EVENT_IN_SOLN es, KHE_MEET meet)
{
  int i;  KHE_MONITOR m;
  if( DEBUG1 )
    fprintf(stderr, "[ KheEventInSolnAddMeet(es, \"%s\" d%d, meet d%d\n",
      KheEventId(es->event), KheEventDuration(es->event),
      KheMeetDuration(meet));
  MAssert(KheMeetDuration(meet) > 0,
    "KheEventInSolnAddMeet internal error");
  MArrayAddLast(es->meets, meet);
  MArrayForEach(es->attached_monitors, &m, &i)
    KheMonitorAddMeet(m, meet);
  if( DEBUG1 )
  {
    KheEventInSolnDebug(es, 2, 2, stderr);
    fprintf(stderr, "] KheEventInSolnAddMeet returning\n");
  }
}


/*****************************************************************************/
/*                                                                           */
/*  void KheEventInSolnDeleteMeet(KHE_EVENT_IN_SOLN es, KHE_MEET meet)       */
/*                                                                           */
/*  Delete meet from es.                                                     */
/*                                                                           */
/*****************************************************************************/

void KheEventInSolnDeleteMeet(KHE_EVENT_IN_SOLN es, KHE_MEET meet)
{
  KHE_MEET x;  int i;  KHE_MONITOR m;
  MArrayForEach(es->attached_monitors, &m, &i)
    KheMonitorDeleteMeet(m, meet);
  x = MArrayRemoveLast(es->meets);
  if( x != meet )
  {
    if( !MArrayContains(es->meets, meet, &i) )
      MAssert(false, "KheEventInSolnDeleteMeet internal error");
    MArrayPut(es->meets, i, x);
  }
}


/*****************************************************************************/
/*                                                                           */
/*  void KheEventInSolnSplitMeet(KHE_EVENT_IN_SOLN es, KHE_MEET meet1,       */
/*    KHE_MEET meet2)                                                        */
/*                                                                           */
/*  Inform es that its meet meet1 is splitting into meet1 and meet2.  The    */
/*  new durations are already installed.                                     */
/*                                                                           */
/*****************************************************************************/

void KheEventInSolnSplitMeet(KHE_EVENT_IN_SOLN es, KHE_MEET meet1,
  KHE_MEET meet2)
{
  int i;  KHE_MONITOR m;
  MAssert(KheMeetDuration(meet1) > 0,
    "KheEventInSolnSplitMeet internal error 1");
  MAssert(KheMeetDuration(meet2) > 0,
    "KheEventInSolnSplitMeet internal error 2");
  MArrayAddLast(es->meets, meet2);
  MArrayForEach(es->attached_monitors, &m, &i)
    KheMonitorSplitMeet(m, meet1, meet2);
}


/*****************************************************************************/
/*                                                                           */
/*  void KheEventInSolnMergeMeet(KHE_EVENT_IN_SOLN es, KHE_MEET meet1,       */
/*    KHE_MEET meet2)                                                        */
/*                                                                           */
/*  Inform es that its meets meet1 and meet2 are merging.  They currently    */
/*  have their pre-merging durations; meet2 is the one going.                */
/*                                                                           */
/*****************************************************************************/

void KheEventInSolnMergeMeet(KHE_EVENT_IN_SOLN es, KHE_MEET meet1,
  KHE_MEET meet2)
{
  KHE_MEET x;  int i;  KHE_MONITOR m;
  MArrayForEach(es->attached_monitors, &m, &i)
    KheMonitorMergeMeet(m, meet1, meet2);
  x = MArrayRemoveLast(es->meets);
  if( x != meet2 )
  {
    if( !MArrayContains(es->meets, meet2, &i) )
      MAssert(false, "KheEventInSolnMergeMeet internal error");
    MArrayPut(es->meets, i, x);
  }
}


/*****************************************************************************/
/*                                                                           */
/*  void KheEventInSolnAssignTime(KHE_EVENT_IN_SOLN es,                      */
/*    KHE_MEET meet, int assigned_time_index)                                */
/*                                                                           */
/*  Inform es that meet has been assigned this time.                         */
/*                                                                           */
/*****************************************************************************/

void KheEventInSolnAssignTime(KHE_EVENT_IN_SOLN es,
  KHE_MEET meet, int assigned_time_index)
{
  KHE_MONITOR m;  int i;
  MArrayForEach(es->attached_time_asst_monitors, &m, &i)
    KheMonitorAssignTime(m, meet, assigned_time_index);
}


/*****************************************************************************/
/*                                                                           */
/*  void KheEventInSolnUnAssignTime(KHE_EVENT_IN_SOLN es,                    */
/*    KHE_MEET meet, int assigned_time_index)                                */
/*                                                                           */
/*  Inform es that meet has been unassigned this time.                       */
/*                                                                           */
/*****************************************************************************/

void KheEventInSolnUnAssignTime(KHE_EVENT_IN_SOLN es,
  KHE_MEET meet, int assigned_time_index)
{
  KHE_MONITOR m;  int i;
  MArrayForEach(es->attached_time_asst_monitors, &m, &i)
    KheMonitorUnAssignTime(m, meet, assigned_time_index);
}


/*****************************************************************************/
/*                                                                           */
/*  int KheEventInSolnMeetCount(KHE_EVENT_IN_SOLN es)                        */
/*                                                                           */
/*  Return the number of meets in es.                                        */
/*                                                                           */
/*****************************************************************************/

int KheEventInSolnMeetCount(KHE_EVENT_IN_SOLN es)
{
  return MArraySize(es->meets);
}


/*****************************************************************************/
/*                                                                           */
/*  KHE_MEET KheEventInSolnMeet(KHE_EVENT_IN_SOLN es, int i)                 */
/*                                                                           */
/*  Return the i'th meet of es.                                              */
/*                                                                           */
/*****************************************************************************/

KHE_MEET KheEventInSolnMeet(KHE_EVENT_IN_SOLN es, int i)
{
  return MArrayGet(es->meets, i);
}


/*****************************************************************************/
/*                                                                           */
/*  Submodule "event resources in soln"                                      */
/*                                                                           */
/*****************************************************************************/

/*****************************************************************************/
/*                                                                           */
/*  int KheEventInSolnEventResourceInSolnCount(KHE_EVENT_IN_SOLN es)         */
/*                                                                           */
/*  Return the number of event resources in soln containing in es.  This     */
/*  will be the same as the number of event resources in es's event.         */
/*                                                                           */
/*****************************************************************************/

int KheEventInSolnEventResourceInSolnCount(KHE_EVENT_IN_SOLN es)
{
  return MArraySize(es->event_resources_in_soln);
}


/*****************************************************************************/
/*                                                                           */
/*  KHE_EVENT_RESOURCE_IN_SOLN KheEventInSolnEventResourceInSoln(            */
/*    KHE_EVENT_IN_SOLN es, int i)                                           */
/*                                                                           */
/*  Return the i'th event resource in soln of es.                            */
/*                                                                           */
/*****************************************************************************/

KHE_EVENT_RESOURCE_IN_SOLN KheEventInSolnEventResourceInSoln(
  KHE_EVENT_IN_SOLN es, int i)
{
  return MArrayGet(es->event_resources_in_soln, i);
}


/*****************************************************************************/
/*                                                                           */
/*  Submodule "monitors"                                                     */
/*                                                                           */
/*****************************************************************************/

/*****************************************************************************/
/*                                                                           */
/*  void KheEventInSolnAttachMonitor(KHE_EVENT_IN_SOLN es, KHE_MONITOR m)    */
/*                                                                           */
/*  Attach m to es.                                                          */
/*                                                                           */
/*****************************************************************************/

void KheEventInSolnAttachMonitor(KHE_EVENT_IN_SOLN es, KHE_MONITOR m)
{
  KHE_MEET meet;  int i;
  switch( KheMonitorTag(m) )
  {
    case KHE_ASSIGN_TIME_MONITOR_TAG:
    case KHE_PREFER_TIMES_MONITOR_TAG:
    case KHE_TIMETABLE_MONITOR_TAG:
    case KHE_SPREAD_EVENTS_MONITOR_TAG:

      /* first add to es's time asst monitor list */
      MArrayAddLast(es->attached_time_asst_monitors, m);
      /* NB NO BREAK */

    case KHE_SPLIT_EVENTS_MONITOR_TAG:
    case KHE_DISTRIBUTE_SPLIT_EVENTS_MONITOR_TAG:

      /* attach directly to es */
      MArrayAddLast(es->attached_monitors, m);
      MArrayForEach(es->meets, &meet, &i)
	KheMonitorAddMeet(m, meet);
      break;

    case KHE_LINK_EVENTS_MONITOR_TAG:

      /* attach to timetable, which much itself be attached first */
      if( !KheMonitorAttachedToSoln((KHE_MONITOR) es->timetable_monitor) )
        KheMonitorAttachToSoln((KHE_MONITOR) es->timetable_monitor);
      KheTimetableMonitorAttachMonitor(es->timetable_monitor, m);
      break;

    case KHE_AVOID_CLASHES_MONITOR_TAG:
    case KHE_AVOID_UNAVAILABLE_TIMES_MONITOR_TAG:
    case KHE_LIMIT_IDLE_TIMES_MONITOR_TAG:
    case KHE_CLUSTER_BUSY_TIMES_MONITOR_TAG:
    case KHE_LIMIT_BUSY_TIMES_MONITOR_TAG:
    case KHE_PREFER_RESOURCES_MONITOR_TAG:
    case KHE_AVOID_SPLIT_ASSIGNMENTS_MONITOR_TAG:
    case KHE_LIMIT_WORKLOAD_MONITOR_TAG:
    case KHE_ASSIGN_RESOURCE_MONITOR_TAG:
    case KHE_TIME_GROUP_MONITOR_TAG:
    case KHE_ORDINARY_DEMAND_MONITOR_TAG:
    case KHE_WORKLOAD_DEMAND_MONITOR_TAG:
    /* case KHE_MATCHING_MONITOR_TAG: */
    default:

      /* unexpected monitor type */
      MAssert(false, "KheEventInSolnAttachMonitor internal error");
  }
}


/*****************************************************************************/
/*                                                                           */
/*  void KheEventInSolnDetachMonitor(KHE_EVENT_IN_SOLN es, KHE_MONITOR m)    */
/*                                                                           */
/*  Detach m from es.                                                        */
/*                                                                           */
/*****************************************************************************/

void KheEventInSolnDetachMonitor(KHE_EVENT_IN_SOLN es, KHE_MONITOR m)
{
  KHE_MEET meet;  int i, pos;
  switch( KheMonitorTag(m) )
  {
    case KHE_ASSIGN_TIME_MONITOR_TAG:
    case KHE_PREFER_TIMES_MONITOR_TAG:
    case KHE_TIMETABLE_MONITOR_TAG:
    case KHE_SPREAD_EVENTS_MONITOR_TAG:

      /* first remove from es's time asst monitor list */
      if( !MArrayContains(es->attached_time_asst_monitors, m, &pos) )
	MAssert(false, "KheEventInSolnDetachMonitor internal error 1");
      MArrayRemove(es->attached_time_asst_monitors, pos);
      /* NB NO BREAK */

    case KHE_SPLIT_EVENTS_MONITOR_TAG:
    case KHE_DISTRIBUTE_SPLIT_EVENTS_MONITOR_TAG:

      /* detach directly from es */
      MArrayForEach(es->meets, &meet, &i)
	KheMonitorDeleteMeet(m, meet);
      if( !MArrayContains(es->attached_monitors, m, &pos) )
	MAssert(false, "KheEventInSolnDetachMonitor internal error 2");
      MArrayRemove(es->attached_monitors, pos);
      break;

    case KHE_LINK_EVENTS_MONITOR_TAG:

      /* detach from timetable */
      KheTimetableMonitorDetachMonitor(es->timetable_monitor, m);
      break;

    case KHE_AVOID_CLASHES_MONITOR_TAG:
    case KHE_AVOID_UNAVAILABLE_TIMES_MONITOR_TAG:
    case KHE_LIMIT_IDLE_TIMES_MONITOR_TAG:
    case KHE_CLUSTER_BUSY_TIMES_MONITOR_TAG:
    case KHE_LIMIT_BUSY_TIMES_MONITOR_TAG:
    case KHE_PREFER_RESOURCES_MONITOR_TAG:
    case KHE_AVOID_SPLIT_ASSIGNMENTS_MONITOR_TAG:
    case KHE_LIMIT_WORKLOAD_MONITOR_TAG:
    case KHE_ASSIGN_RESOURCE_MONITOR_TAG:
    case KHE_TIME_GROUP_MONITOR_TAG:
    /* case KHE_MATCHING_MONITOR_TAG: */
    case KHE_ORDINARY_DEMAND_MONITOR_TAG:
    case KHE_WORKLOAD_DEMAND_MONITOR_TAG:
    default:

      /* unexpected monitor type */
      MAssert(false, "KheEventInSolnDetachMonitor internal error");
  }
}


/*****************************************************************************/
/*                                                                           */
/*  Submodule "user monitors and cost"                                       */
/*                                                                           */
/*****************************************************************************/

/*****************************************************************************/
/*                                                                           */
/*  void KheEventInSolnAddMonitor(KHE_EVENT_IN_SOLN es, KHE_MONITOR m)       */
/*                                                                           */
/*  Add m to the user-accessible list of monitors of es.                     */
/*                                                                           */
/*****************************************************************************/

void KheEventInSolnAddMonitor(KHE_EVENT_IN_SOLN es, KHE_MONITOR m)
{
  MArrayAddLast(es->all_monitors, m);
}


/*****************************************************************************/
/*                                                                           */
/*  void KheEventInSolnDeleteMonitor(KHE_EVENT_IN_SOLN es, KHE_MONITOR m)    */
/*                                                                           */
/*  Delete m from the user-accessible list of monitors of es.                */
/*                                                                           */
/*****************************************************************************/

void KheEventInSolnDeleteMonitor(KHE_EVENT_IN_SOLN es, KHE_MONITOR m)
{
  int pos;
  if( !MArrayContains(es->all_monitors, m, &pos) )
    MAssert(false, "KheEventInSolnDeleteMonitor internal error");
  MArrayRemove(es->all_monitors, pos);
}


/*****************************************************************************/
/*                                                                           */
/*  int KheEventInSolnMonitorCount(KHE_EVENT_IN_SOLN es)                     */
/*                                                                           */
/*  Return the number of user-accessible monitors in es.                     */
/*                                                                           */
/*****************************************************************************/

int KheEventInSolnMonitorCount(KHE_EVENT_IN_SOLN es)
{
  return MArraySize(es->all_monitors);
}


/*****************************************************************************/
/*                                                                           */
/*  KHE_MONITOR KheEventInSolnMonitor(KHE_EVENT_IN_SOLN es, int i)           */
/*                                                                           */
/*  Return the i'th user-accessible monitor of es.                           */
/*                                                                           */
/*****************************************************************************/

KHE_MONITOR KheEventInSolnMonitor(KHE_EVENT_IN_SOLN es, int i)
{
  return MArrayGet(es->all_monitors, i);
}


/*****************************************************************************/
/*                                                                           */
/*  KHE_COST KheEventInSolnCost(KHE_EVENT_IN_SOLN es)                        */
/*                                                                           */
/*  Return the total cost of all monitors.                                   */
/*                                                                           */
/*****************************************************************************/

KHE_COST KheEventInSolnCost(KHE_EVENT_IN_SOLN es)
{
  KHE_MONITOR m;  int i;  KHE_COST res;
  res = 0;
  MArrayForEach(es->all_monitors, &m, &i)
    res += KheMonitorCost(m);
  return res;
}


/*****************************************************************************/
/*                                                                           */
/*  KHE_COST KheEventInSolnMonitorCost(KHE_EVENT_IN_SOLN es,                 */
/*    KHE_MONITOR_TAG tag)                                                   */
/*                                                                           */
/*  Return the total cost of monitors with this tag.                         */
/*                                                                           */
/*****************************************************************************/

KHE_COST KheEventInSolnMonitorCost(KHE_EVENT_IN_SOLN es, KHE_MONITOR_TAG tag)
{
  KHE_MONITOR m;  int i;  KHE_COST res;
  res = 0;
  MArrayForEach(es->all_monitors, &m, &i)
    if( KheMonitorTag(m) == tag )
      res += KheMonitorCost(m);
  return res;
}


/*****************************************************************************/
/*                                                                           */
/* KHE_TIMETABLE_MONITOR KheEventInSolnTimetableMonitor(KHE_EVENT_IN_SOLN es)*/
/*                                                                           */
/*  Return the timetable of es.                                              */
/*                                                                           */
/*****************************************************************************/

KHE_TIMETABLE_MONITOR KheEventInSolnTimetableMonitor(KHE_EVENT_IN_SOLN es)
{
  return es->timetable_monitor;
}


/*****************************************************************************/
/*                                                                           */
/*  Submodule "reading and writing"                                          */
/*                                                                           */
/*****************************************************************************/

/*****************************************************************************/
/*                                                                           */
/*  bool KheEventInSolnWrite(KHE_EVENT_IN_SOLN es, KML_FILE kf)              */
/*                                                                           */
/*  Write the meets of es to kf.                                             */
/*                                                                           */
/*****************************************************************************/

bool KheEventInSolnWrite(KHE_EVENT_IN_SOLN es, KML_FILE kf)
{
  KHE_MEET meet;  int i;
  MArraySort(es->meets, &KheMeetAssignedTimeCmp);
  MArrayForEach(es->meets, &meet, &i)
    if( KheMeetMustWrite(meet) && !KheMeetWrite(meet, kf) )
      return false;
  return true;
}


/*****************************************************************************/
/*                                                                           */
/*  Submodule "debug"                                                        */
/*                                                                           */
/*****************************************************************************/

/*****************************************************************************/
/*                                                                           */
/*  void KheEventInSolnDebug(KHE_EVENT_IN_SOLN es, int verbosity,            */
/*    int indent, FILE *fp)                                                  */
/*                                                                           */
/*  Debug print of es onto fp with the given verbosity and indent,           */
/*  including its constraints.                                               */
/*                                                                           */
/*****************************************************************************/

void KheEventInSolnDebug(KHE_EVENT_IN_SOLN es, int verbosity,
  int indent, FILE *fp)
{
  int i;  KHE_MEET meet;  KHE_EVENT_RESOURCE_IN_SOLN ers;
  KHE_MONITOR m;
  if( verbosity >= 1 )
  {
    if( indent >= 0 )
    {
      fprintf(fp, "%*s[ Event Monitor \"%s\"\n", indent, "",
	KheEventId(es->event) != NULL ? KheEventId(es->event) : "-");
      MArrayForEach(es->meets, &meet, &i)
	fprintf(fp, "%*s  meet dur %d %p\n", indent, "",
	  KheMeetDuration(meet), (void *) meet);
      MArrayForEach(es->event_resources_in_soln, &ers, &i)
	KheEventResourceInSolnDebug(ers, verbosity, indent + 2, fp);
      MArrayForEach(es->attached_monitors, &m, &i)
	KheMonitorDebug(m, verbosity, indent + 2, fp);
      fprintf(fp, "%*s]\n", indent, "");
    }
    else
    {
      fprintf(fp, "\"%s\"", KheEventId(es->event) != NULL ?
	KheEventId(es->event) : "-");
    }
  }
}
