
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
/*  FILE:         khe_kempe_move.c                                           */
/*  DESCRIPTION:  Kempe meet moves                                           */
/*                                                                           */
/*****************************************************************************/
#include "khe.h"
#include "m.h"
#include <limits.h>

#define DEBUG1 0

typedef MARRAY(KHE_MEET) ARRAY_KHE_MEET;


/*****************************************************************************/
/*                                                                           */
/*  bool KheMeetHasPreassignedTask(KHE_MEET meet, KHE_TASK *task)            */
/*                                                                           */
/*  If meet, or any meet assigned to meet directly or indirectly, contains   */
/*  a preassigned task, then set *task to one such task and return true,     */
/*  otherwise return false.                                                  */
/*                                                                           */
/*****************************************************************************/

static bool KheMeetHasPreassignedTask(KHE_MEET meet, KHE_TASK *task)
{
  int i;  KHE_RESOURCE junk;

  /* try meet's own tasks */
  for( i = 0;  i < KheMeetTaskCount(meet);  i++ )
  {
    *task = KheMeetTask(meet, i);
    if( KheTaskIsPreassigned(*task, false, &junk) )
      return true;
  }

  /* try meets assigned to meet */
  for( i = 0;  i < KheMeetAssignedToCount(meet);  i++ )
    if( KheMeetHasPreassignedTask(KheMeetAssignedTo(meet, i), task) )
      return true;
  return false;
}


/*****************************************************************************/
/*                                                                           */
/*  bool KheKempeFail(ARRAY_KHE_MEET *meets, KHE_TRACE t)                    */
/*                                                                           */
/*  Convenience function for KheKempeMeetMove to call when failing.          */
/*                                                                           */
/*****************************************************************************/

static bool KheKempeFail(ARRAY_KHE_MEET *meets, KHE_TRACE t,
  char *message, KHE_MEET meet)
{
  MArrayFree(*meets);
  KheTraceEnd(t);
  KheTraceDelete(t);
  if( DEBUG1 )
  {
    fprintf(stderr, "  ] KheKempeMeetMove returning false (%s", message);
    if( meet != NULL )
    {
      fprintf(stderr, " ");
      KheMeetDebug(meet, 1, -1, stderr);
    }
    fprintf(stderr, ")\n");
  }
  return false;
}


/*****************************************************************************/
/*                                                                           */
/*  bool KheMonitorMonitorsPreassignedTask(KHE_MONITOR m, KHE_TASK *task)    */
/*                                                                           */
/*  If m is an ordinary demand monitor and the task it monitors is           */
/*  preassigned, set *task to that task and return true, else return false.  */
/*                                                                           */
/*****************************************************************************/

static bool KheMonitorMonitorsPreassignedTask(KHE_MONITOR m, KHE_TASK *task)
{
  KHE_RESOURCE junk;
  if( KheMonitorTag(m) != KHE_ORDINARY_DEMAND_MONITOR_TAG )
    return false;
  *task = KheOrdinaryDemandMonitorTask((KHE_ORDINARY_DEMAND_MONITOR) m);
  return KheTaskIsPreassigned(*task, false, &junk);
}


/*****************************************************************************/
/*                                                                           */
/*  bool KheKempeMeetMove(KHE_MEET meet, KHE_MEET target_meet,               */
/*    int target_offset, int *demand)                                        */
/*                                                                           */
/*  Make a Kempe meet move of meet from wherever it is now to target_meet    */
/*  at target_offset, and set *demand to the total demand of the nodes of    */
/*  the meets that were moved.                                               */
/*                                                                           */
/*  Implementation note.  This function uses the fact that when a change     */
/*  is made, any new unmatched nodes in the matching lie in the part that    */
/*  changed, not in the part that didn't.                                    */
/*                                                                           */
/*****************************************************************************/

bool KheKempeMeetMove(KHE_MEET meet, KHE_MEET target_meet,
  int target_offset, int *demand)
{
  KHE_MEET from_target_meet[2], to_target_meet[2];
  int from_target_offset[2], to_target_offset[2], duration[2];
  int parity, i, j, first_unmoved, pos;
  ARRAY_KHE_MEET meets;  KHE_TRACE t;  bool res;
  KHE_MONITOR m, c;  KHE_GROUP_MONITOR gm;  KHE_TASK task;
  if( DEBUG1 )
  {
    fprintf(stderr, "  [ KheKempeMeetMove(");
    KheMeetDebug(meet, 1, -1, stderr);
    fprintf(stderr, ", ");
    KheMeetDebug(target_meet, 1, -1, stderr);
    fprintf(stderr, ", %d, &d)\n", target_offset);
  }
//printf("i\n"); fflush(stdout);

  /* meet must have a current assignment */
  MAssert(KheMeetAsst(meet) != NULL, "KheKempeMeetMove: meet not assigned");
  *demand = 0;
//printf("j\n"); fflush(stdout);

  /* return false if the move takes us nowhere */
  if( KheMeetAsst(meet)==target_meet && KheMeetAsstOffset(meet)==target_offset )
  {
    if( DEBUG1 )
      fprintf(stderr, "  ] KheKempeMeetMove returning false (goes nowhere)\n");
    return false;
  }
//printf("k\n"); fflush(stdout);
  
  /* if there are no preassigned tasks, do an ordinary move */
  if( !KheMeetHasPreassignedTask(meet, &task) )
  {
    res = KheMeetMove(meet, target_meet, target_offset);
    *demand += KheMeetDemand(meet);
    if( DEBUG1 )
      fprintf(stderr, "  ] KheKempeMeetMove returning %s (trivial)\n",
	res ? "true" : "false");
    return res;
  }

  /* start tracing, or abort if can't */
  MAssert(KheTaskDemandMonitorCount(task) > 0,
    "KheKempeMeetMove internal error 1");
  gm = KheMonitorParentMonitor((KHE_MONITOR) KheTaskDemandMonitor(task, 0));
  if( gm != NULL )
    gm = KheMonitorParentMonitor((KHE_MONITOR) gm);
  MAssert(gm != NULL, "KheKempeMeetMove: incorrect demand monitor grouping");
  t = KheTraceMake(gm);
  KheTraceBegin(t);

  /* initialize frame for odd-numbered steps */
  duration[1] = KheMeetDuration(meet);
  from_target_meet[1] = KheMeetAsst(meet);
  from_target_offset[1] = KheMeetAsstOffset(meet);
  to_target_meet[1] = target_meet;
  to_target_offset[1] = target_offset;

  /* initialize frame for even-numbered steps, but only partially */
  duration[0] = -1;
  from_target_meet[0] = target_meet;
  from_target_offset[0] = -1;
  to_target_meet[0] = KheMeetAsst(meet);
  to_target_offset[0] = -1;

  /* build the initial set of meets to move (just meet) */
  MArrayInit(meets);
  MArrayAddLast(meets, meet);
  first_unmoved = 0;

  /* carry out as many steps as needed to move all meets, unless fail first */
  for( parity = 1;  first_unmoved < MArraySize(meets);  parity = 1 - parity )
  {
    /* move the unmoved meets and fail if any refuse to move */
    for( ; first_unmoved < MArraySize(meets);  first_unmoved++ )
    {
      meet = MArrayGet(meets, first_unmoved);

      /* initialize frame for even-numbered steps, if not done yet */
      if( parity == 0 && duration[0] == -1 )
      {
	if( to_target_meet[1] != from_target_meet[1] )
	{
	  /* target meets differ, so simple case only */
	  duration[0] = duration[1];
	  from_target_offset[0] = to_target_offset[1];
	  to_target_offset[0] = from_target_offset[1];
	}
	else if( KheMeetAsstOffset(meet) == from_target_offset[1]+duration[1] )
	{
	  /* meet follows immediately after where original meet was */
	  duration[0] = KheMeetDuration(meet);
	  from_target_offset[0] = KheMeetAsstOffset(meet);
	  to_target_offset[0] = from_target_offset[1];
	}
	else if( KheMeetAsstOffset(meet) + KheMeetDuration(meet) == 
		 from_target_offset[1] )
	{
	  /* where original meet was follows immediately after meet */
	  duration[0] = KheMeetDuration(meet);
	  from_target_offset[0] = KheMeetAsstOffset(meet);
	  to_target_offset[0] = KheMeetAsstOffset(meet) + duration[1];
	}
	else
	{
	  /* same target meets but not adjacent in time, so simple case only */
	  duration[0] = duration[1];
	  from_target_offset[0] = to_target_offset[1];
	  to_target_offset[0] = from_target_offset[1];
	}
      }

      /* check meet against the frame for this step and move it */
      MAssert(KheMeetAsst(meet) == from_target_meet[parity], 
	"KheKempeMeetMove internal error 2");
      if( KheMeetAsstOffset(meet) != from_target_offset[parity] ||
          KheMeetDuration(meet) != duration[parity] ||
          !KheMeetMove(meet, to_target_meet[parity],to_target_offset[parity]) )
        return KheKempeFail(&meets, t,
	  KheMeetAsstOffset(meet) != from_target_offset[parity] ? "offset":
	  KheMeetDuration(meet) != duration[parity] ? "durn" : "move", meet);
      *demand += KheMeetDemand(meet);
      if( DEBUG1 )
      {
	fprintf(stderr, "    %s KheMeetMove(", parity == 1 ? "->" : "<-");
	KheMeetDebug(meet, 1, -1, stderr);
	fprintf(stderr, " d%d, ", KheMeetDuration(meet));
	KheMeetDebug(to_target_meet[parity], 1, -1, stderr);
	fprintf(stderr, ", %d) (%d trace monitors)\n",
	  to_target_offset[parity], KheTraceMonitorCount(t));
      }
    }

    /* handle all preassigned demand monitors that increased in cost */
    for( i = 0;  i < KheTraceMonitorCount(t);  i++ )
    {
      m = KheTraceMonitor(t, i);
      if( KheMonitorTag(m) == KHE_GROUP_MONITOR_TAG &&
	  KheMonitorCost(m) > KheTraceMonitorInitCost(t, i) )
      {
	gm = (KHE_GROUP_MONITOR) m;
	for( j = 0;  j < KheGroupMonitorDefectCount(gm);  j++ )
	{
	  m = KheGroupMonitorDefect(gm, j);
          if( KheMonitorMonitorsPreassignedTask(m, &task) )
	  {
	    for( c = KheMonitorFirstCompetitor(m);  c != NULL; 
		 c = KheMonitorNextCompetitor(m) )
	    {
	      if( KheMonitorTag(c) == KHE_WORKLOAD_DEMAND_MONITOR_TAG )
		return KheKempeFail(&meets, t, "unavail", NULL);
	      MAssert(KheMonitorTag(c) == KHE_ORDINARY_DEMAND_MONITOR_TAG,
		"KheKempeMeetMove internal error 3");
	      if( KheMonitorMonitorsPreassignedTask(c, &task) )
	      {
		/* find ancestor of meet directly under to_target, or fail */
		meet = KheTaskMeet(task);
		MAssert(meet != NULL, "KheKempeMeetMove internal error 4");
		while( KheMeetAsst(meet) != NULL &&
		    KheMeetAsst(meet) != to_target_meet[parity] )
		  meet = KheMeetAsst(meet);
		if( KheMeetAsst(meet) != to_target_meet[parity] )
		  return KheKempeFail(&meets, t, "scope", meet);

		/* ensure meet is enqueued for the next step, or fail */
		if( !MArrayContains(meets, meet, &pos) )
		{
		  /* meet not seen before, enqueue it for the next step */
		  if( DEBUG1 )
		  {
		    fprintf(stderr, "      adding meet ");
		    KheMeetDebug(meet, 1, -1, stderr);
		    fprintf(stderr, "\n");
		  }
		  MArrayAddLast(meets, meet);
		}
		else if( pos < first_unmoved )
		{
		  /* meet moved previously, can't move it again, so fail */
		  return KheKempeFail(&meets, t, "re-move", meet);
		}
		else
		{
		  /* meet already enqueued for the next step */
		}
	      }
	    }
	  }
	}
      }
    }
  }

  /* everything has moved and no more problems, so success */
  MArrayFree(meets);
  KheTraceEnd(t);
  KheTraceDelete(t);
  if( DEBUG1 )
    fprintf(stderr, "  ] KheKempeMeetMove ret true\n");
  return true;
}


/*****************************************************************************/
/*                                                                           */
/*  bool KheKempeMeetMoveTime(KHE_MEET meet, KHE_TIME t, int *demand)        */
/*                                                                           */
/*  Make a Kempe meet move of meet to the cycle meet and offset that         */
/*  represent t.                                                             */
/*                                                                           */
/*****************************************************************************/

bool KheKempeMeetMoveTime(KHE_MEET meet, KHE_TIME t, int *demand)
{
  KHE_SOLN soln;  KHE_MEET target_meet;  int target_offset;
  soln = KheMeetSoln(meet);
  target_meet = KheSolnTimeCycleMeet(soln, t);
  target_offset = KheSolnTimeCycleMeetOffset(soln, t);
  return KheKempeMeetMove(meet, target_meet, target_offset, demand);
}
