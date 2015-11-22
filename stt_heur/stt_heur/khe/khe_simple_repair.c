
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
/*  FILE:         khe_simple_repair.c                                        */
/*  DESCRIPTION:  A simple time assignment repair algorithm                  */
/*                                                                           */
/*****************************************************************************/
#include "khe.h"
#include "m.h"

#define DEBUG1 0
#define DEBUG2 0
#define DEBUG3 0
#define DEBUG4 0

typedef MARRAY(KHE_MEET) ARRAY_KHE_MEET;


/*****************************************************************************/
/*                                                                           */
/*  Submodule "meet swapping"                                                */
/*                                                                           */
/*****************************************************************************/

/*****************************************************************************/
/*                                                                           */
/*  bool KheTrySwap(KHE_MEET meet1, KHE_MEET meet2, KHE_COST *cost)          */
/*                                                                           */
/*  If meet1 and meet2 can be swapped, return true and set *cost to the      */
/*  cost of the solution afterwards.  Otherwise return false.                */
/*                                                                           */
/*****************************************************************************/

static bool KheTrySwap(KHE_MEET meet1, KHE_MEET meet2, KHE_COST *cost)
{
  if( KheMeetDuration(meet1) != KheMeetDuration(meet2) )
    return false;
  if( !KheMeetSwap(meet1, meet2) )
    return false;
  *cost = KheSolnCost(KheMeetSoln(meet1));
  KheMeetSwap(meet1, meet2);
  return true;
}


/*****************************************************************************/
/*                                                                           */
/*  bool KheTryMeetSwaps(ARRAY_KHE_MEET *meets, KHE_SOLN soln, char *str,    */
/*    KHE_COST min_cost_improvement)                                         */
/*                                                                           */
/*  These meets are involved in defects in some way.  Try all swaps of       */
/*  these meets with suitable other meets (no attempt is made to tailor      */
/*  the repairs, but the meets themselves are carefully targeted).           */
/*                                                                           */
/*  This functions assumes that all the meets it is passed lie in nodes.     */
/*                                                                           */
/*****************************************************************************/

static bool KheTryMeetSwaps(ARRAY_KHE_MEET *meets, KHE_SOLN soln, char *str,
  KHE_COST min_cost_improvement)
{
  int i, j, k, demand, meet1_demand, best_demand;
  KHE_MEET meet1, meet2, best_meet1, best_meet2;
  KHE_COST cost, init_cost, best_cost;  KHE_LAYER layer;  KHE_NODE node;

  if( DEBUG3 )
  {
    fprintf(stderr, "  [ KheTryMeetSwaps(meets, soln) (cost %.4f)\n",
      KheCostShow(KheSolnCost(soln)));
    fprintf(stderr, "    meets: ");
    MArrayForEach(*meets, &meet1, &i)
    {
      if( i > 0 )
      {
	if( i % 5 == 0 )
	  fprintf(stderr, ",\n    ");
	else
	  fprintf(stderr, ", ");
      }
      KheMeetDebug(meet1, 1, -1, stderr);
    }
    fprintf(stderr, "\n");
  }

  /* find the best swap, one with minimum (cost, demand) */
  init_cost = KheSolnCost(soln);
  best_cost = init_cost;  best_demand = 0;  /* have to beat this */
  best_meet1 = best_meet2 = NULL;
  MArrayForEach(*meets, &meet1, &i)
  {
    meet1_demand = KheNodeDemand(KheMeetNode(meet1));
    node = KheMeetNode(meet1);
    if( node != NULL && KheNodeParentLayerCount(node) > 0 )
    {
      layer = KheNodeParentLayer(node, 0);
      for( j = 0;  j < KheLayerChildNodeCount(layer);  j++ )
      {
	node = KheLayerChildNode(layer, j);
	for( k = 0;  k < KheNodeMeetCount(node);  k++ )
	{
	  meet2 = KheNodeMeet(node, k);
	  if( KheTrySwap(meet1, meet2, &cost) )
	  {
	    demand = meet1_demand + KheNodeDemand(KheMeetNode(meet2));
	    if( DEBUG3 )
	    {
	      fprintf(stderr, "    swapping ");
	      KheMeetDebug(meet1, 1, -1, stderr);
	      fprintf(stderr, " with ");
	      KheMeetDebug(meet2, 1, -1, stderr);
	      fprintf(stderr, " (cost %.4f, demand %d)",
		KheCostShow(cost), demand);
	    }
	    if( cost < best_cost || (cost==best_cost && demand < best_demand) )
	    {
	      best_meet1 = meet1;
	      best_meet2 = meet2;
	      best_cost = cost;
	      best_demand = demand;
	      if( DEBUG3 )
		fprintf(stderr, " new best");
	    }
	    if( DEBUG3 )
	      fprintf(stderr, "\n");
	  }
	}
      }
    }
  }
  MAssert(KheSolnCost(soln) == init_cost, "KheTryMeetSwaps internal error 1");
  if( DEBUG1 || DEBUG3 )
  {
    if( best_cost < init_cost - min_cost_improvement )
    {
      fprintf(stderr, "  %s swap ", str);
      KheMeetDebug(best_meet1, 1, -1, stderr);
      fprintf(stderr, "  <--> ");
      KheMeetDebug(best_meet2, 1, -1, stderr);
      fprintf(stderr, " (demand %d, cost %.4f --> %.4f)\n",
	best_demand, KheCostShow(init_cost), KheCostShow(best_cost));
    }
    else if( DEBUG3 )
      fprintf(stderr, "    no best swap\n");
  }

  if( best_cost < init_cost - min_cost_improvement )
  {
    if( !KheMeetSwap(best_meet1, best_meet2) )
      MAssert(false, "KheTryMeetSwaps internal error 2");
    MAssert(KheSolnCost(soln) == best_cost, "KheTryMeetSwaps internal error 3");
  }
  if( DEBUG3 )
    fprintf(stderr, "  ] KheTryMeetSwaps returning %s (cost %.4f)\n",
      best_cost < init_cost - min_cost_improvement ? "true" : "false",
      KheCostShow(KheSolnCost(soln)));
  return best_cost < init_cost - min_cost_improvement;
}


/*****************************************************************************/
/*                                                                           */
/*  Submodule "spread events defects repair"                                 */
/*                                                                           */
/*****************************************************************************/

/*****************************************************************************/
/*                                                                           */
/*  bool KheSimpleRepairSpreadEventsDefect(KHE_SPREAD_EVENTS_MONITOR sem,    */
/*    KHE_COST min_cost_improvement)                                         */
/*                                                                           */
/*  Try to repair m, return true if changed anything.                        */
/*                                                                           */
/*****************************************************************************/

static bool KheSimpleRepairSpreadEventsDefect(KHE_SPREAD_EVENTS_MONITOR sem,
  KHE_COST min_cost_improvement)
{
  KHE_EVENT_GROUP eg;  KHE_EVENT e;  int i, j;  KHE_SOLN soln;
  ARRAY_KHE_MEET meets;  KHE_MEET meet;  bool res;
  soln = KheMonitorSoln((KHE_MONITOR) sem);
  if( DEBUG4 )
  {
    fprintf(stderr, "[ KheSimpleRepairSpreadEventsDefect(sem) (cost %.4f)\n",
      KheCostShow(KheSolnCost(soln)));
    KheMonitorDebug((KHE_MONITOR) sem, 1, 2, stderr);
  }

  /* build an array of the monitored meets, if they lie in nodes */
  MArrayInit(meets);
  eg = KheSpreadEventsMonitorEventGroup(sem);
  for( i = 0;  i < KheEventGroupEventCount(eg);  i++ )
  {
    e = KheEventGroupEvent(eg, i);
    for( j = 0;  j < KheEventMeetCount(soln, e);  j++ )
    {
      meet = KheEventMeet(soln, e, j);
      if( KheMeetNode(meet) != NULL )
        MArrayAddLast(meets, meet);
    }
  }

  /* try swaps of those meets with other meets in the same layer */
  res = KheTryMeetSwaps(&meets, soln, "spread", min_cost_improvement);
  MArrayFree(meets);
  if( DEBUG4 )
    fprintf(stderr, "] KheSimpleRepairSpreadEventsDefect ret %s (cost %.4f)\n",
      res ? "true" : "false", KheCostShow(KheSolnCost(soln)));
  return res;
}


/*****************************************************************************/
/*                                                                           */
/*  Submodule "ordinary demand defects repair"                               */
/*                                                                           */
/*****************************************************************************/

/*****************************************************************************/
/*                                                                           */
/*  bool KheTailHasRepeats(ARRAY_KHE_MEET *meets, int count)                 */
/*                                                                           */
/*  Return true if the last count elements of *meets (which must exist)      */
/*  are all equal.                                                           */
/*                                                                           */
/*****************************************************************************/

static bool KheTailHasRepeats(ARRAY_KHE_MEET *meets, int count)
{
  KHE_MEET meet;  int i;
  meet = MArrayLast(*meets);
  for( i = 2;  i <= count;  i++ )
    if( MArrayGet(*meets, MArraySize(*meets) - i) != meet )
      return false;
  return true;
}


/*****************************************************************************/
/*                                                                           */
/*  void KheAddMeets(ARRAY_KHE_MEET *meets,                                  */
/*    KHE_ORDINARY_DEMAND_MONITOR odm, int *count)                           */
/*                                                                           */
/*  Add the ancestors of odm that lie in nodes to *meets, and                */
/*  increment *count if added anything at all.                               */
/*                                                                           */
/*****************************************************************************/

static void KheAddMeets(ARRAY_KHE_MEET *meets,
  KHE_ORDINARY_DEMAND_MONITOR odm, int *count)
{
  KHE_MEET meet;  bool contributed;  KHE_TIME t;
  meet = KheTaskMeet(KheOrdinaryDemandMonitorTask(odm));
  if( !KheMeetIsPreassigned(meet, false, &t) )
  {
    contributed = false;
    while( KheMeetAsst(meet) != NULL )
    {
      if( KheMeetNode(meet) != NULL )
      {
	MAssert(KheNodeParent(KheMeetNode(meet)) != NULL,
	  "KheAddMeets internal error");
	MArrayAddLast(*meets, meet);
	contributed = true;
      }
      meet = KheMeetAsst(meet);
    }
    if( contributed )
      (*count)++;
  }
}


/*****************************************************************************/
/*                                                                           */
/*  int KheMeetIncreasingDemandCmp(const void *t1, const void *t2)           */
/*                                                                           */
/*  Comparison function for sorting meets by increasing demand.              */
/*                                                                           */
/*****************************************************************************/

static int KheMeetIncreasingDemandCmp(const void *t1, const void *t2)
{
  KHE_MEET meet1 = * (KHE_MEET *) t1;
  KHE_MEET meet2 = * (KHE_MEET *) t2;
  if( KheMeetDemand(meet1) != KheMeetDemand(meet2) )
    return KheMeetDemand(meet1) - KheMeetDemand(meet2);
  else
    return KheMeetIndex(meet1) - KheMeetIndex(meet2);
}


/*****************************************************************************/
/*                                                                           */
/*  bool KheSimpleRepairOrdinaryDemandDefect(KHE_ORDINARY_DEMAND_MONITOR odm,*/
/*    KHE_COST min_cost_improvement)                                         */
/*                                                                           */
/*  Try to repair m, return true if changed anything.                        */
/*                                                                           */
/*****************************************************************************/

static bool KheSimpleRepairOrdinaryDemandDefect(KHE_ORDINARY_DEMAND_MONITOR odm,
  KHE_COST min_cost_improvement)
{
  int i, count;  bool res;  ARRAY_KHE_MEET meets; KHE_SOLN soln; KHE_MONITOR m;
  soln = KheMonitorSoln((KHE_MONITOR) odm);
  if( DEBUG2 )
  {
    fprintf(stderr, "[ KheSimpleRepairOrdinaryDemandDefect(odm) (cost %.4f)\n",
      KheCostShow(KheSolnCost(soln)));
    KheMonitorDebug((KHE_MONITOR) odm, 1, 2, stderr);
  }

  /* build a single array of all competitor meets that may move */
  MArrayInit(meets);
  count = 0;
  KheAddMeets(&meets, odm, &count);
  for( m = KheMonitorFirstCompetitor((KHE_MONITOR) odm);  m != NULL; 
       m = KheMonitorNextCompetitor((KHE_MONITOR) odm) )
    if( KheMonitorTag(m) == KHE_ORDINARY_DEMAND_MONITOR_TAG )
      KheAddMeets(&meets, (KHE_ORDINARY_DEMAND_MONITOR) m, &count);

  /* sort the meets by increasing demand, and bring identical meets together */
  MArraySort(meets, &KheMeetIncreasingDemandCmp);

  /* from the right, remove blocks of count identical meets; moving */
  /* these ones would just shift the whole problem, so we omit them */
  while( MArraySize(meets) >= count && KheTailHasRepeats(&meets, count) )
    MArrayDropFromEnd(meets, count);

  /* uniqueify the array */
  for( i = 1;  i < MArraySize(meets);  i++ )
    if( MArrayGet(meets, i-1) == MArrayGet(meets, i) )
    {
      MArrayRemove(meets, i);
      i--;
    }

  res = KheTryMeetSwaps(&meets, soln, "demand", min_cost_improvement);
  MArrayFree(meets);
  if( DEBUG2 )
    fprintf(stderr,"] KheSimpleRepairOrdinaryDemandDefect ret %s (cost %.4f)\n",
      res ? "true" : "false", KheCostShow(KheSolnCost(soln)));
  return res;
}


/*****************************************************************************/
/*                                                                           */
/*  Submodule "main algorithm"                                               */
/*                                                                           */
/*****************************************************************************/

/*****************************************************************************/
/*                                                                           */
/*  bool KheSimpleRepairTimes(KHE_NODE parent_node,                          */
/*    KHE_COST min_cost_improvement)                                         */
/*                                                                           */
/*  Simple time repair at root node.  A repair is only accepted if it        */
/*  improves cost by at least min_cost_improvement.                          */
/*                                                                           */
/*****************************************************************************/

bool KheSimpleRepairTimes(KHE_NODE parent_node, KHE_COST min_cost_improvement)
{
  bool res, progressing, need_layers;  KHE_SOLN soln;  KHE_MONITOR m;
  int i, count;
  KHE_ORDINARY_DEMAND_MONITOR odm;  KHE_SPREAD_EVENTS_MONITOR sem;
  soln = KheNodeSoln(parent_node);
  if( DEBUG1 )
  {
    fprintf(stderr, "[ KheSimpleRepairTimes(");
    KheNodeDebug(parent_node, 1, -1, stderr);
    fprintf(stderr, "), soln cost %.4f\n", KheCostShow(KheSolnCost(soln)));
  }

  /* make child layers, if needed */
  need_layers = (KheNodeChildLayerCount(parent_node) == 0);
  if( need_layers )
    KheNodeChildLayersMake(parent_node);

  /* repair defects */
  progressing = true;
  for( count = 1;  progressing;  count++ )
  {
    if( DEBUG1 )
      fprintf(stderr, "  iteration %d:\n", count);
    progressing = false;
    KheSolnDefectSort(soln);
    for( i = 0;  i < KheSolnDefectCount(soln);  i++ )
    {
      m = KheSolnDefect(soln, i);
      switch( KheMonitorTag(m) )
      {
	case KHE_PREFER_TIMES_MONITOR_TAG:

	  /* still to do (if ever) */
	  break;

	case KHE_SPREAD_EVENTS_MONITOR_TAG:

	  sem = (KHE_SPREAD_EVENTS_MONITOR) m;
	  if( KheSimpleRepairSpreadEventsDefect(sem, min_cost_improvement) )
	    progressing = true;
	  break;

	case KHE_AVOID_UNAVAILABLE_TIMES_MONITOR_TAG:

	  /* still to do (if ever) */
	  break;

	case KHE_ORDINARY_DEMAND_MONITOR_TAG:

	  odm = (KHE_ORDINARY_DEMAND_MONITOR) m;
	  if( KheSimpleRepairOrdinaryDemandDefect(odm, min_cost_improvement) )
	    progressing = true;
	  break;

	default:

	  /* ignore other kinds of defects */
	  break;
      }
    }
  }

  /* delete child layers and return */
  if( need_layers )
    KheNodeChildLayersDelete(parent_node);
  res = KheNodeAllChildMeetsAssigned(parent_node);
  if( DEBUG1 )
    fprintf(stderr, "] KheSimpleRepairTimes returning %s, soln cost %.4f\n",
      res ? "true" : "false", KheCostShow(KheSolnCost(soln)));
  return res;
}
