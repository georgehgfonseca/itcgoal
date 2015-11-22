
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
/*  FILE:         khe_trace.c                                                */
/*  DESCRIPTION:  A trace                                                    */
/*                                                                           */
/*****************************************************************************/
#include "khe_interns.h"

/*****************************************************************************/
/*                                                                           */
/*  KHE_TRACE - a trace                                                      */
/*                                                                           */
/*****************************************************************************/

struct khe_trace_rec {
  KHE_GROUP_MONITOR		group_monitor;		/* tracing this      */
  KHE_COST			gm_init_cost;		/* init cost of gm   */
  bool				active;			/* tracing now       */
  ARRAY_KHE_MONITOR		monitors;		/* changed ones      */
  ARRAY_INT64			init_costs;		/* init costs        */
};


/*****************************************************************************/
/*                                                                           */
/*  Submodule "construction and query"                                       */
/*                                                                           */
/*****************************************************************************/

/*****************************************************************************/
/*                                                                           */
/*  KHE_TRACE KheTraceMake(KHE_GROUP_MONITOR gm)                             */
/*                                                                           */
/*  Create a new trace for gm.                                               */
/*                                                                           */
/*****************************************************************************/

KHE_TRACE KheTraceMake(KHE_GROUP_MONITOR gm)
{
  KHE_TRACE res;
  KheSolnMatchingUpdate(KheMonitorSoln((KHE_MONITOR) gm));
  res = KheSolnGetTraceFromFreeList(KheMonitorSoln((KHE_MONITOR) gm));
  if( res == NULL )
  {
    MMake(res);
    MArrayInit(res->monitors);
    MArrayInit(res->init_costs);
  }
  res->group_monitor = gm;
  res->gm_init_cost = 0;
  res->active = false;
  MArrayClear(res->monitors);
  MArrayClear(res->init_costs);
  return res;
}


/*****************************************************************************/
/*                                                                           */
/*  void KheTraceDelete(KHE_TRACE t)                                         */
/*                                                                           */
/*  Delete t.                                                                */
/*                                                                           */
/*****************************************************************************/

void KheTraceDelete(KHE_TRACE t)
{
  KHE_SOLN soln;
  soln = KheMonitorSoln((KHE_MONITOR) t->group_monitor);
  MAssert(!t->active, "KheTraceDelete called before KheTraceEnd");
  KheSolnAddTraceToFreeList(soln, t);
}


/*****************************************************************************/
/*                                                                           */
/*  void KheTraceBegin(KHE_TRACE t)                                          */
/*                                                                           */
/*  Begin tracing with t.                                                    */
/*                                                                           */
/*****************************************************************************/

void KheTraceBegin(KHE_TRACE t)
{
  MAssert(!t->active,
    "KheTraceBegin called twice with no intervening KheTraceEnd");
  t->gm_init_cost = KheMonitorCost((KHE_MONITOR) t->group_monitor);
  t->active = true;
  MArrayClear(t->monitors);
  MArrayClear(t->init_costs);
  KheGroupMonitorBeginTrace(t->group_monitor, t);
}


/*****************************************************************************/
/*                                                                           */
/*  void KheTraceMakeEnd(KHE_TRACE t)                                        */
/*                                                                           */
/*  End tracing with t.                                                      */
/*                                                                           */
/*****************************************************************************/

void KheTraceEnd(KHE_TRACE t)
{
  MAssert(t->active, "KheTraceEnd with no matching call to KheTraceBegin");
  t->active = false;
  KheGroupMonitorEndTrace(t->group_monitor, t);
}


/*****************************************************************************/
/*                                                                           */
/*  void KheTraceFree(KHE_TRACE t)                                           */
/*                                                                           */
/*  Reclaim the memory used by t (internal use only).                        */
/*                                                                           */
/*****************************************************************************/

void KheTraceFree(KHE_TRACE t)
{
  MArrayFree(t->monitors);
  MArrayFree(t->init_costs);
  MFree(t);
}


/*****************************************************************************/
/*                                                                           */
/*  void KheTraceChangeCost(KHE_TRACE t, KHE_MONITOR m, KHE_COST old_cost)   */
/*                                                                           */
/*  The cost of m has changed.  Trace it.                                    */
/*                                                                           */
/*****************************************************************************/

void KheTraceChangeCost(KHE_TRACE t, KHE_MONITOR m, KHE_COST old_cost)
{
  int pos;
  if( !MArrayContains(t->monitors, m, &pos) )
  {
    MArrayAddLast(t->monitors, m);
    MArrayAddLast(t->init_costs, old_cost);
  }
}


/*****************************************************************************/
/*                                                                           */
/*  KHE_COST KheTraceInitCost(KHE_TRACE t)                                   */
/*                                                                           */
/*  Return the cost of t's group monitor at the time tracing started.        */
/*                                                                           */
/*****************************************************************************/

KHE_COST KheTraceInitCost(KHE_TRACE t)
{
  return t->gm_init_cost;
}


/*****************************************************************************/
/*                                                                           */
/*  int KheTraceMonitorCount(KHE_TRACE t)                                    */
/*                                                                           */
/*  Return the number of child monitors of t's group monitor that changed    */
/*  their cost during the most recent trace.                                 */
/*                                                                           */
/*****************************************************************************/

int KheTraceMonitorCount(KHE_TRACE t)
{
  KheSolnMatchingUpdate(KheMonitorSoln((KHE_MONITOR) t->group_monitor));
  return MArraySize(t->monitors);
}


/*****************************************************************************/
/*                                                                           */
/*  KHE_MONITOR KheTraceMonitor(KHE_TRACE t, int i)                          */
/*                                                                           */
/*  Return the i'th monitor that changed its cost.                           */
/*                                                                           */
/*****************************************************************************/

KHE_MONITOR KheTraceMonitor(KHE_TRACE t, int i)
{
  KheSolnMatchingUpdate(KheMonitorSoln((KHE_MONITOR) t->group_monitor));
  return MArrayGet(t->monitors, i);
}


/*****************************************************************************/
/*                                                                           */
/*  KHE_COST KheTraceMonitorInitCost(KHE_TRACE t, int i)                     */
/*                                                                           */
/*  Return the initial cost of the i'th monitor.                             */
/*                                                                           */
/*****************************************************************************/

KHE_COST KheTraceMonitorInitCost(KHE_TRACE t, int i)
{
  return MArrayGet(t->init_costs, i);
}
