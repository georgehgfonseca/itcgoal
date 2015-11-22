
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
/*  FILE:         khe_augment.c                                              */
/*  DESCRIPTION:  Augment functions for ejection chains                      */
/*                                                                           */
/*****************************************************************************/
#include "khe.h"
#include "m.h"
#include <limits.h>

#define MAX_GROUP_AUGMENT 20

#define DEBUG1 0
#define DEBUG2 0
#define DEBUG3 0
#define DEBUG4 0
#define DEBUG5 0
#define DEBUG6 0
#define DEBUG7 0
#define DEBUG9 0
#define DEBUG10 0
#define DEBUG11 0
#define DEBUG12 0

typedef MARRAY(KHE_MEET) ARRAY_KHE_MEET;
typedef MARRAY(KHE_NODE) ARRAY_KHE_NODE;
typedef MARRAY(KHE_TASK) ARRAY_KHE_TASK;


/*****************************************************************************/
/*                                                                           */
/*  Submodule "event defects"                                                */
/*                                                                           */
/*****************************************************************************/

/*****************************************************************************/
/*                                                                           */
/*  KHE_MEET KheUnAssignedMeetAncestor(KHE_MEET meet)                        */
/*                                                                           */
/*  Assuming meet has no time assignment, get the ancestor we would need     */
/*  to assign to in order to make a time assignment.                         */
/*                                                                           */
/*****************************************************************************/

/* *** no longer used
static KHE_MEET KheUnAssignedMeetAncestor(KHE_MEET meet)
{
  while( KheMeetAsst(meet) != NULL )
    meet = KheMeetAsst(meet);
  MAssert(!KheMeetIsCycleMeet(meet),
    "KheUnAssignedMeetAncestor internal error");
  return meet;
}
*** */


/*****************************************************************************/
/*                                                                           */
/*  KHE_MEET KheAssignedMeetAncestor(KHE_MEET meet)                          */
/*                                                                           */
/*  Assuming meet has a time assignment, get the ancestor we would need      */
/*  to move in order to change that time assignment.                         */
/*                                                                           */
/*****************************************************************************/

/* *** no longer used
static KHE_MEET KheAssignedMeetAncestor(KHE_MEET meet)
{
  while( KheMeetAsst(KheMeetAsst(meet)) != NULL )
    meet = KheMeetAsst(meet);
  MAssert(KheMeetAsst(meet) != NULL &&
      KheMeetIsCycleMeet(KheMeetAsst(meet)),
    "KheAssignedMeetAncestor internal error");
  return meet;
}
*** */


/*****************************************************************************/
/*                                                                           */
/*  Submodule "assign time monitor augment function" (private)               */
/*                                                                           */
/*****************************************************************************/

/*****************************************************************************/
/*                                                                           */
/*  bool KheMeetHasAssignableAncestor(KHE_MEET meet, KHE_MEET *anc_meet)     */
/*                                                                           */
/*  If meet is not currently assigned a time and has an ancestor meet        */
/*  that is assignable (because it is currently unassigned and lies in       */
/*  a node that possesses a parent node) then return true with *anc_meet     */
/*  set to the ancestor meet.  Otherwise return false.                       */
/*                                                                           */
/*  It is safe for meet and *anc_meet to be the same variable.               */
/*                                                                           */
/*****************************************************************************/

static bool KheMeetHasAssignableAncestor(KHE_MEET meet, KHE_MEET *anc_meet)
{
  if( KheMeetAsstTime(meet) != NULL )
    return false;
  while( KheMeetAsst(meet) != NULL )
    meet = KheMeetAsst(meet);
  if( KheMeetNode(meet) != NULL && KheNodeParent(KheMeetNode(meet)) != NULL )
  {
    *anc_meet = meet;
    return true;
  }
  else
    return false;
}


/*****************************************************************************/
/*                                                                           */
/*  bool KheAssignTimeAugment(KHE_EJECTOR ej, KHE_MEET meet,                 */
/*    KHE_MEET target_meet, KHE_TRACE tc)                                    */
/*                                                                           */
/*  Try assigning meet (which is unvisited) to target_meet at each offset.   */
/*  Parameter tc is a scratch trace object.                                  */
/*                                                                           */
/*****************************************************************************/

static bool KheAssignTimeAugment(KHE_EJECTOR ej, KHE_MEET meet,
  KHE_MEET target_meet, KHE_TRACE tc)
{
  bool success;  int i;
  for( i=0;  i <= KheMeetDuration(target_meet) - KheMeetDuration(meet);  i++ )
  {
    KheTraceBegin(tc);
    success = KheMeetAssign(meet, target_meet, i);
    KheTraceEnd(tc);
    if( success )
    {
      if( KheEjectorSuccess(ej, tc, KheMeetDuration(meet)) )
	return true;
      KheMeetUnAssign(meet);
    }
  }
  return false;
}


/*****************************************************************************/
/*                                                                           */
/*  bool KheAssignTimeMonitorAugment(KHE_EJECTOR ej,                         */
/*    KHE_ASSIGN_TIME_MONITOR atm)                                           */
/*                                                                           */
/*  Try to repair atm, which has non-zero cost, without touching any         */
/*  visited meets.                                                           */
/*                                                                           */
/*****************************************************************************/

static bool KheAssignTimeMonitorAugment(KHE_EJECTOR ej,
  KHE_ASSIGN_TIME_MONITOR atm)
{
  KHE_SOLN soln;  KHE_EVENT e;  KHE_MEET meet, target_meet;  int i, j;
  KHE_NODE node;  KHE_TRACE tc;
  tc = KheTraceMake(KheEjectorGroupMonitor(ej));
  soln = KheEjectorSoln(ej);
  e = KheAssignTimeMonitorEvent(atm);
  for( i = 0;  i < KheEventMeetCount(soln, e);  i++ )
  {
    meet = KheEventMeet(soln, e, i);
    if( KheMeetHasAssignableAncestor(meet, &meet) && !KheMeetVisited(meet, 0) )
    {
      KheMeetVisit(meet);
      node = KheNodeParent(KheMeetNode(meet));
      for( j = 0;  j < KheNodeMeetCount(node);  j++ )
      {
	target_meet = KheNodeMeet(node, j);
	if( KheAssignTimeAugment(ej, meet, target_meet, tc) )
	{
	  KheTraceDelete(tc);
	  return true;
	}
      }
      if( KheEjectorScheduleMayRevisit(KheEjectorCurrSchedule(ej)) )
	KheMeetUnVisit(meet);
    }
  }
  KheTraceDelete(tc);
  return false;
}


/*****************************************************************************/
/*                                                                           */
/*  Submodule "prefer times monitor augment function" (private)              */
/*                                                                           */
/*****************************************************************************/

/*****************************************************************************/
/*                                                                           */
/*  bool KheEjectorDoKempeMoveAugment(KHE_EJECTOR ej, KHE_MEET meet,         */
/*    KHE_MEET target_meet, KHE_TRACE tc, KHE_TRANSACTION tn)                */
/*                                                                           */
/*  Try Kempe-moving meet to target_meet at each legal offset.               */
/*  Parameters tc and tn are scratch trace and transaction objects.          */
/*                                                                           */
/*  Tailoring this further so that it tries only moves that put meet into    */
/*  the desired domain is still to do.                                       */
/*                                                                           */
/*****************************************************************************/

/* ***
static bool KheEjectorDoKempeMoveAugment(KHE_EJECTOR ej, KHE_MEET meet,
  KHE_MEET target_meet, KHE_TRACE tc, KHE_TRANSACTION tn)
{
  int i;  bool success;
  ** tn = KheTransactionMake(KheEjectorSoln(ej)); **
  ** tc = KheTraceMake(KheEjectorGroupMonitor(ej)); **
  for( i=0;  i <= KheMeetDuration(target_meet) - KheMeetDuration(meet);  i++ )
  {
    KheTraceBegin(tc);
    KheTransactionBegin(tn);
    if( DEBUG5 )
    {
      fprintf(stderr, "%*s[ Repair Kempe ",
	4*KheEjectorCurrDepth(ej) + 4, "");
      KheMeetDebug(meet, 1, -1, stderr);
      fprintf(stderr, " -> ");
      KheMeetDebug(target_meet, 1, -1, stderr);
      fprintf(stderr, "+%d\n", i);
    }
    success = KheMeetKempeMove(meet, target_meet, i);
    KheTransactionEnd(tn);
    KheTraceEnd(tc);
    if( success && KheEjectorSuccess(ej, tc) )
    {
      ** KheTraceDelete(tc); **
      ** KheTransactionDelete(tn); **
      if( DEBUG5 )
	fprintf(stderr, "%*s] success\n",
	  4*KheEjectorCurrDepth(ej) + 4, "");
      return true;
    }
    if( DEBUG5 )
      fprintf(stderr, "%*s] failure\n",
	4*KheEjectorCurrDepth(ej) + 4, "");
    KheTransactionUndo(tn);
  }
  ** KheTraceDelete(tc); **
  ** KheTransactionDelete(tn); **
  return false;
}
*** */


/*****************************************************************************/
/*                                                                           */
/*  bool KheEjectorDoKempeMoveTestAugment(KHE_EJECTOR ej, KHE_MEET meet,     */
/*    KHE_MEET target_meet, int *best_offset, KHE_COST *best_cost)           */
/*                                                                           */
/*  Try Kempe-moving meet to target_meet at each legal offset.               */
/*                                                                           */
/*  Tailoring this further so that it tries only moves that put meet into    */
/*  the desired domain is still to do.                                       */
/*                                                                           */
/*****************************************************************************/

/* ***
static bool KheEjectorDoKempeMoveTestAugment(KHE_EJECTOR ej, KHE_MEET meet,
  KHE_MEET target_meet, int *best_offset, KHE_COST *best_cost)
{
  KHE_TRANSACTION tn;  int i;  KHE_COST cost, save_cost;
  tn = KheTransactionMake(KheEjectorSoln(ej));
  *best_offset = -1;
  *best_cost = KheCostMax;
  for( i=0; i <= KheMeetDuration(target_meet)-KheMeetDuration(meet); i++)
  {
    save_cost = KheSolnCost(KheMeetSoln(meet));
    KheTransactionBegin(tn);
    if( KheMeetKempeMove(meet, target_meet, i) )
    {
      cost = KheSolnCost(KheMeetSoln(meet));
      if( cost < *best_cost )
      {
	*best_offset = i;
	*best_cost = cost;
      }
    }
    KheTransactionEnd(tn);
    KheTransactionUndo(tn);
    MAssert(save_cost == KheSolnCost(KheMeetSoln(meet)),
      "KheEjectorDoKempeMoveTestAugment internal error");
  }
  KheTransactionDelete(tn);
  return *best_cost < KheCostMax;
}
*** */


/*****************************************************************************/
/*                                                                           */
/*  KHE_KEMPE_OPTION - one option for a successful Kempe time move           */
/*                                                                           */
/*****************************************************************************/

/* ***
typedef struct khe_kempe_option_rec {
  KHE_MEET		target_meet;
  int			target_offset;
  KHE_COST		cost;
} KHE_KEMPE_OPTION;
*** */


/*****************************************************************************/
/*                                                                           */
/*  int KheKempeOptionCmp(const void *t1, const void *t2)                    */
/*                                                                           */
/*  Comparison function for sorting Kempe options.                           */
/*                                                                           */
/*****************************************************************************/

/* ***
static int KheKempeOptionCmp(const void *t1, const void *t2)
{
  KHE_KEMPE_OPTION *ko1 = (KHE_KEMPE_OPTION *) t1;
  KHE_KEMPE_OPTION *ko2 = (KHE_KEMPE_OPTION *) t2;
  int cost_cmp = KheCostCmp(ko1->cost, ko2->cost);
  if( cost_cmp != 0 )
    return cost_cmp;
  else if( KheMeetIndex(ko1->target_meet) != KheMeetIndex(ko2->target_meet) )
    return KheMeetIndex(ko1->target_meet) - KheMeetIndex(ko2->target_meet);
  else
    return ko1->target_offset - ko2->target_offset;
}
*** */


/*****************************************************************************/
/*                                                                           */
/*  bool KheEjectorKempeMoveAugment(KHE_EJECTOR ej, KHE_MEET meet)           */
/*                                                                           */
/*  Try all Kempe moves of meet to other places, handling the visited flag   */
/*  of meet as well, to make sure we don't revisit when we don't want to.    */
/*                                                                           */
/*  Tailoring this further so that it tries only moves that put meet into    */
/*  the desired domain (and also to nearby times) is still to do.            */
/*                                                                           */
/*****************************************************************************/

/* ***
static bool KheEjectorKempeMoveAugment(KHE_EJECTOR ej, KHE_MEET meet)
{
  KHE_NODE node;  int j, offset, ocount;  KHE_MEET meet2;
  KHE_SOLN soln;  KHE_TRACE tc;  KHE_TRANSACTION tn;  bool success;
  KHE_KEMPE_OPTION options[50];  KHE_COST cost;
  soln = KheEjectorSoln(ej);
  if( !KheMeetVisited(meet, 0) )
  {
    KheMeetVisit(meet);
    if( KheMeetNode(meet) != NULL )
    {
      ** choices are the meets of the parent node of meet's node **
      node = KheNodeParent(KheMeetNode(meet));
      if( node != NULL )
      {
	** find and sort the Kempe time move options **
	ocount = 0;
	for( j = 0;  j < KheNodeMeetCount(node);  j++ )
	{
	  meet2 = KheNodeMeet(node, j);
	  if( ocount < 50 &&
	      KheMeetDuration(meet) <= KheMeetDuration(meet2) &&
              KheEjectorDoKempeMoveTestAugment(ej, meet, meet2, &offset,&cost) )
	  {
	    options[ocount].target_meet = meet2;
	    options[ocount].target_offset = offset;
	    options[ocount].cost = cost;
	    if( DEBUG5 )
	    {
	      fprintf(stderr, "%*s  found Repair2 ",
		4*KheEjectorCurrDepth(ej) + 4, "");
	      KheMeetDebug(meet, 1, -1, stderr);
	      fprintf(stderr, " -> ");
	      KheMeetDebug(options[ocount].target_meet, 1, -1, stderr);
	      fprintf(stderr, "+%d cost %.4f\n", options[ocount].target_offset,
		KheCostShow(options[ocount].cost));
	    }
	    ocount++;
	  }
	}
	qsort(options, ocount, sizeof(KHE_KEMPE_OPTION), &KheKempeOptionCmp);

	** try each of these options **
	tn = KheTransactionMake(soln);
	tc = KheTraceMake(KheEjectorGroupMonitor(ej));
	for( j = 0;  j < ocount;  j++ )
	{
	  KheTraceBegin(tc);
	  KheTransactionBegin(tn);
	  if( DEBUG5 )
	  {
	    fprintf(stderr, "%*s[ Repair2 Kempe ",
	      4*KheEjectorCurrDepth(ej) + 4, "");
	    KheMeetDebug(meet, 1, -1, stderr);
	    fprintf(stderr, " -> ");
	    KheMeetDebug(options[j].target_meet, 1, -1, stderr);
	    fprintf(stderr, "+%d\n", options[j].target_offset);
	  }
	  success = KheMeetKempeMove(meet, options[j].target_meet,
	    options[j].target_offset);
	  KheTransactionEnd(tn);
	  KheTraceEnd(tc);
	  if( success && KheEjectorSuccess(ej, tc) )
	  {
	    KheTraceDelete(tc);
	    KheTransactionDelete(tn);
	    if( DEBUG5 )
	      fprintf(stderr, "%*s] success %.4f\n",
		4*KheEjectorCurrDepth(ej) + 4, "",
	        KheCostShow(KheSolnCost(soln)));
	    return true;
	  }
	  if( DEBUG5 )
	    fprintf(stderr, "%*s] failure\n",
	      4*KheEjectorCurrDepth(ej) + 4, "");
	  KheTransactionUndo(tn);
	}
	KheTraceDelete(tc);
	KheTransactionDelete(tn);
      }
    }
    else
    {
      ** choices are the cycle meets **
      for( j = 0;  j < KheSolnMeetCount(soln);  j++ )
      {
	meet2 = KheSolnMeet(soln, j);
	if( !KheMeetIsCycleMeet(meet) )
	  break;
	if( KheMeetDuration(meet) <= KheMeetDuration(meet2) &&
	    KheEjectorDoKempeMoveAugment(ej, meet, meet2) )
	  return true;
      }
    }
    if( KheEjectorMayRevisit(ej) )
      KheMeetUnVisit(meet);
  }
  return false;
}
*** */


/*****************************************************************************/
/*                                                                           */
/*  bool KheKempeTimeMoveAugment(KHE_EJECTOR ej, KHE_MEET meet,              */
/*    KHE_MEET target_meet, int target_offset, KHE_TRANSACTION tn,           */
/*    KHE_TRACE tc)                                                          */
/*                                                                           */
/*  Try Kempe-moving meet to target_meet at target_offset.                   */
/*  Parameters tn and tc are scratch transaction and trace objects.          */
/*                                                                           */
/*****************************************************************************/

static bool KheKempeTimeMoveAugment(KHE_EJECTOR ej, KHE_MEET meet,
  KHE_MEET target_meet, int target_offset, KHE_TRANSACTION tn, KHE_TRACE tc)
{
  bool success;  int d;
  KheTransactionBegin(tn);
  KheTraceBegin(tc);
  success = KheKempeMeetMove(meet, target_meet, target_offset, &d);
  KheTraceEnd(tc);
  KheTransactionEnd(tn);
  if( success && KheEjectorSuccess(ej, tc, d) )
    return true;
  KheTransactionUndo(tn);
  return false;
}


/*****************************************************************************/
/*                                                                           */
/*  bool KheKempeDomainAugment(KHE_EJECTOR ej, KHE_MEET meet,                */
/*    KHE_TIME_GROUP domain, int base, KHE_TRANSACTION tn, KHE_TRACE tc)     */
/*                                                                           */
/*  Try moving meet so that its original meet goes into domain-base.         */
/*  Assume that the meet can be moved and has not been visited.              */
/*                                                                           */
/*****************************************************************************/

bool KheKempeDomainAugment(KHE_EJECTOR ej, KHE_MEET meet,
  KHE_TIME_GROUP domain, int base, KHE_TRANSACTION tn, KHE_TRACE tc)
{
  KHE_MEET target_meet;  KHE_TIME t;  int i, ti, max_offset, target_offset;
  KHE_NODE parent_node;  KHE_INSTANCE ins;
  parent_node = KheNodeParent(KheMeetNode(meet));
  ins = KheSolnInstance(KheEjectorSoln(ej));
  for( i = 0;  i < KheNodeMeetCount(parent_node);  i++ )
  {
    target_meet = KheNodeMeet(parent_node, i);
    if( KheMeetAsstTime(target_meet) == NULL )
      continue;  /* not interested if we don't assign a time */
    ti = KheTimeIndex(KheMeetAsstTime(target_meet));
    max_offset = KheMeetDuration(target_meet) - KheMeetDuration(meet);
    for( target_offset = 0;  target_offset <= max_offset;  target_offset++ )
    {
      t = KheInstanceTime(ins, ti + base + target_offset);
      if( KheTimeGroupContains(domain, t) &&
	  KheKempeTimeMoveAugment(ej, meet, target_meet,
	      target_offset, tn, tc) )
	return true;
    }
  }
  return false;
}


/*****************************************************************************/
/*                                                                           */
/*  bool KhePreferTimesKempeMoveAugment(KHE_EJECTOR ej, KHE_MEET meet,       */
/*    KHE_TIME_GROUP domain, int base, KHE_TRANSACTION tn, KHE_TRACE tc)     */
/*                                                                           */
/*  Try Kempe time moves on meet (assumed assigned), tailored to put the     */
/*  original meet into domain, but only if meet is unvisited and suitable.   */
/*  Parameters tn and tc are scratch transaction and trace objects.          */
/*                                                                           */
/*****************************************************************************/

static bool KhePreferTimesKempeMoveAugment(KHE_EJECTOR ej, KHE_MEET meet,
  KHE_TIME_GROUP domain, int base, KHE_TRANSACTION tn, KHE_TRACE tc)
{
  if( KheMeetNode(meet) != NULL && KheNodeParent(KheMeetNode(meet)) != NULL &&
      !KheMeetVisited(meet, 0) )
  {
    KheMeetVisit(meet);
    if( KheKempeDomainAugment(ej, meet, domain, base, tn, tc) )
      return true;
    if( KheEjectorScheduleMayRevisit(KheEjectorCurrSchedule(ej)) )
      KheMeetUnVisit(meet);
  }
  return false;
}


/*****************************************************************************/
/*                                                                           */
/*  bool KheKempeMoveToDomainAugment(KHE_EJECTOR ej, KHE_MEET meet,          */
/*    KHE_TIME_GROUP domain, KHE_TRANSACTION tn, KHE_TRACE tc)               */
/*                                                                           */
/*  Assuming that meet has an assigned time, try to move any one of its      */
/*  unvisited and acceptable ancestors to make meet's assigned time lie in   */
/*  domain.  Parameters tn and tc are scratch transaction and trace objects. */
/*                                                                           */
/*****************************************************************************/

static bool KheKempeMoveToDomainAugment(KHE_EJECTOR ej, KHE_MEET meet,
  KHE_TIME_GROUP domain, KHE_TRANSACTION tn, KHE_TRACE tc)
{
  int base;
  base = 0;
  while( KheMeetAsst(meet) != NULL )
  {
    if( KhePreferTimesKempeMoveAugment(ej, meet, domain, base, tn, tc) )
      return true;
    base += KheMeetAsstOffset(meet);
    meet = KheMeetAsst(meet);
  }
  return false;
}


/*****************************************************************************/
/*                                                                           */
/*  bool KhePreferTimesAugment(KHE_EJECTOR ej, KHE_PREFER_TIMES_MONITOR ptm) */
/*                                                                           */
/*  Try to repair ptm.                                                       */
/*                                                                           */
/*****************************************************************************/

static bool KhePreferTimesAugment(KHE_EJECTOR ej, KHE_PREFER_TIMES_MONITOR ptm)
{
  KHE_SOLN soln;  KHE_EVENT e;  KHE_MEET meet;  int i, durn;
  KHE_PREFER_TIMES_CONSTRAINT ptc;  KHE_TIME_GROUP domain;
  KHE_TIME t;  KHE_TRANSACTION tn;  KHE_TRACE tc;
  soln = KheEjectorSoln(ej);
  e = KhePreferTimesMonitorEvent(ptm);
  ptc = KhePreferTimesMonitorConstraint(ptm);
  domain = KhePreferTimesConstraintDomain(ptc);
  durn = KhePreferTimesConstraintDuration(ptc);
  tn = KheTransactionMake(soln);
  tc = KheTraceMake(KheEjectorGroupMonitor(ej));
  for( i = 0;  i < KheEventMeetCount(soln, e);  i++ )
  {
    meet = KheEventMeet(soln, e, i);
    t = KheMeetAsstTime(meet);
    if( (durn == KHE_NO_DURATION || KheMeetDuration(meet) == durn) &&
	t != NULL && !KheTimeGroupContains(domain, t) &&
        KheKempeMoveToDomainAugment(ej, meet, domain, tn, tc) )
    {
      KheTraceDelete(tc);
      KheTransactionDelete(tn);
      return true;
    }
  }
  KheTraceDelete(tc);
  KheTransactionDelete(tn);
  return false;
}


/* *** old version
static bool KheEjectorPreferTimesAugment(KHE_EJECTOR ej,
  KHE_PREFER_TIMES_MONITOR ptm)
{
  KHE_SOLN soln;  KHE_EVENT e;  KHE_MEET meet;  int i, durn;
  KHE_PREFER_TIMES_CONSTRAINT ptc;  KHE_TIME_GROUP domain;
  KHE_TIME t;
  soln = KheEjectorSoln(ej);
  e = KhePreferTimesMonitorEvent(ptm);
  ptc = KhePreferTimesMonitorConstraint(ptm);
  domain = KhePreferTimesConstraintDomain(ptc);
  durn = KhePreferTimesConstraintDuration(ptc);
  for( i = 0;  i < KheEventMeetCount(soln, e);  i++ )
  {
    meet = KheEventMeet(soln, e, i);
    t = KheMeetAsstTime(meet);
    if( (durn == KHE_NO_DURATION || KheMeetDuration(meet) == durn) &&
	t != NULL && !KheTimeGroupContains(domain, t) )
    {
      ** meet is broken; try Kempe moves on the appropriate ancestor **
      meet = KheAssignedMeetAncestor(meet);
      if( KheEjectorKempeMoveAugment(ej, meet) )
	return true;
    }
  }
  return false;
}
*** */


/*****************************************************************************/
/*                                                                           */
/*  Submodule "spread events monitor augment function" (private)             */
/*                                                                           */
/*****************************************************************************/

/*****************************************************************************/
/*                                                                           */
/*  bool KheSpreadEventsMonitorContainsHighTimeGroup(                        */
/*    KHE_SPREAD_EVENTS_MONITOR sem)                                         */
/*                                                                           */
/*  Return true if sem contains a high time group.                           */
/*                                                                           */
/*****************************************************************************/

static bool KheSpreadEventsMonitorContainsHighTimeGroup(
  KHE_SPREAD_EVENTS_MONITOR sem)
{
  int i, minimum, maximum, inc;  KHE_TIME_GROUP tg;
  for( i = 0;  i < KheSpreadEventsMonitorTimeGroupCount(sem);  i++ )
  {
    KheSpreadEventsMonitorTimeGroup(sem, i, &tg, &minimum, &maximum, &inc);
    if( inc > maximum )
      return true;
  }
  return false;
}


/*****************************************************************************/
/*                                                                           */
/*  bool KheSpreadEventsMonitorContainsLowTimeGroup(                         */
/*    KHE_SPREAD_EVENTS_MONITOR sem)                                         */
/*                                                                           */
/*  Return true if sem contains a low time group.                            */
/*                                                                           */
/*****************************************************************************/

static bool KheSpreadEventsMonitorContainsLowTimeGroup(
  KHE_SPREAD_EVENTS_MONITOR sem)
{
  int i, minimum, maximum, inc;  KHE_TIME_GROUP tg;
  for( i = 0;  i < KheSpreadEventsMonitorTimeGroupCount(sem);  i++ )
  {
    KheSpreadEventsMonitorTimeGroup(sem, i, &tg, &minimum, &maximum, &inc);
    if( inc < minimum )
      return true;
  }
  return false;
}


/*****************************************************************************/
/*                                                                           */
/*  bool KheMeetIsHigh(KHE_MEET meet, KHE_SPREAD_EVENTS_MONITOR sem)         */
/*                                                                           */
/*  Return true if meet is a high meet.                                      */
/*                                                                           */
/*****************************************************************************/

static bool KheMeetIsHigh(KHE_MEET meet, KHE_SPREAD_EVENTS_MONITOR sem)
{
  int i, minimum, maximum, inc;  KHE_TIME_GROUP tg;  KHE_TIME t;

  /* meet must be assigned a time */
  t = KheMeetAsstTime(meet);
  if( t == NULL )
    return false;

  /* find the first time group of sem containing meet's assigned time */
  for( i = 0;  i < KheSpreadEventsMonitorTimeGroupCount(sem);  i++ )
  {
    KheSpreadEventsMonitorTimeGroup(sem, i, &tg, &minimum, &maximum, &inc);
    if( KheTimeGroupContains(tg, t) )
      return inc > maximum;
  }
  return false;
}


/*****************************************************************************/
/*                                                                           */
/*  bool KheMeetIsVeryMiddle(KHE_MEET meet, KHE_SPREAD_EVENTS_MONITOR sem)   */
/*                                                                           */
/*  Return true if meet is a very middle meet (it lies in a middle time      */
/*  group which would remain middle if meet was removed from it).            */
/*                                                                           */
/*****************************************************************************/

static bool KheMeetIsVeryMiddle(KHE_MEET meet, KHE_SPREAD_EVENTS_MONITOR sem)
{
  int i, minimum, maximum, inc;  KHE_TIME_GROUP tg;  KHE_TIME t;

  /* meet must be assigned a time */
  t = KheMeetAsstTime(meet);
  if( t == NULL )
    return false;

  /* find the first time group of sem containing meet's assigned time */
  for( i = 0;  i < KheSpreadEventsMonitorTimeGroupCount(sem);  i++ )
  {
    KheSpreadEventsMonitorTimeGroup(sem, i, &tg, &minimum, &maximum, &inc);
    if( KheTimeGroupContains(tg, t) )
      return inc <= maximum && inc > minimum;  /* NB ">" implements "very" */
  }
  return false;
}


/*****************************************************************************/
/*                                                                           */
/*  bool KheTimeGroupIsLow(KHE_SPREAD_EVENTS_MONITOR sem, int i,             */
/*    KHE_TIME_GROUP *tg)                                                    */
/*                                                                           */
/*  If the i'th time group of sem is low, return true and set *tg to the     */
/*  time group, otherwise return false.                                      */
/*                                                                           */
/*****************************************************************************/

static bool KheTimeGroupIsLow(KHE_SPREAD_EVENTS_MONITOR sem, int i,
  KHE_TIME_GROUP *tg)
{
  int minimum, maximum, inc;
  KheSpreadEventsMonitorTimeGroup(sem, i, tg, &minimum, &maximum, &inc);
  return inc < minimum;
}


/*****************************************************************************/
/*                                                                           */
/*  bool KheTimeGroupIsVeryMiddle(KHE_SPREAD_EVENTS_MONITOR sem, int i,      */
/*    KHE_TIME_GROUP *tg)                                                    */
/*                                                                           */
/*  If the i'th time group of sem is very middle (if it is a middle time     */
/*  group and would remain so if another meet was added to it), return       */
/*  true and set *tg to the time group, otherwise return false.              */
/*                                                                           */
/*****************************************************************************/

static bool KheTimeGroupIsVeryMiddle(KHE_SPREAD_EVENTS_MONITOR sem, int i,
  KHE_TIME_GROUP *tg)
{
  int minimum, maximum, inc;
  KheSpreadEventsMonitorTimeGroup(sem, i, tg, &minimum, &maximum, &inc);
  return inc >= minimum && inc < maximum; /* NB "<" implements "very" */
}


/*****************************************************************************/
/*                                                                           */
/*  bool KheMoveToLowOrVeryMiddleAugment(KHE_EJECTOR ej, bool to_middle,     */
/*    KHE_SPREAD_EVENTS_MONITOR sem, KHE_MEET meet, KHE_TRANSACTION tn,      */
/*    KHE_TRACE tc)                                                          */
/*                                                                           */
/*  Try to augment by moving one of meet's ancestors so that meet lies in    */
/*  a low time group, or (but only if to_middle is true) a very middle time  */
/*  group.                                                                   */
/*                                                                           */
/*****************************************************************************/

static bool KheMoveToLowOrVeryMiddleAugment(KHE_EJECTOR ej, bool to_middle,
  KHE_SPREAD_EVENTS_MONITOR sem, KHE_MEET meet, KHE_TRANSACTION tn,
  KHE_TRACE tc)
{
  int base, i;  KHE_TIME_GROUP tg;

  /* visit every ancestor of meet */
  base = 0;
  while( KheMeetAsst(meet) != NULL )
  {
    if( KheMeetNode(meet) != NULL && KheNodeParent(KheMeetNode(meet)) != NULL
	&& !KheMeetVisited(meet, 0) )
    {
      KheMeetVisit(meet);

      /* try assigning meet to each low time group */
      for( i = 0;  i < KheSpreadEventsMonitorTimeGroupCount(sem);  i++ )
        if( KheTimeGroupIsLow(sem, i, &tg) &&
	    KheKempeDomainAugment(ej, meet, tg, base, tn, tc) )
	  return true;

      /* optionally try assigning meet to each very middle time group */
      if( to_middle )
	for( i = 0;  i < KheSpreadEventsMonitorTimeGroupCount(sem);  i++ )
	  if( KheTimeGroupIsVeryMiddle(sem, i, &tg) &&
	      KheKempeDomainAugment(ej, meet, tg, base, tn, tc) )
	    return true;

      if( KheEjectorScheduleMayRevisit(KheEjectorCurrSchedule(ej)) )
	KheMeetUnVisit(meet);
    }
    base += KheMeetAsstOffset(meet);
    meet = KheMeetAsst(meet);
  }
  return false;
}


/*****************************************************************************/
/*                                                                           */
/*  bool KheSpreadEventsAugment(KHE_EJECTOR ej,                              */
/*    KHE_SPREAD_EVENTS_MONITOR sem)                                         */
/*                                                                           */
/*  Try to repair sem by moving an ancestor of one of the assigned meets it  */
/*  monitors.                                                                */
/*                                                                           */
/*****************************************************************************/

static bool KheSpreadEventsAugment(KHE_EJECTOR ej,
  KHE_SPREAD_EVENTS_MONITOR sem)
{
  KHE_SOLN soln;  KHE_EVENT_GROUP eg;  KHE_EVENT e;  KHE_MEET meet;
  KHE_TRANSACTION tn;  KHE_TRACE tc;  int i, j;
  soln = KheEjectorSoln(ej);
  eg = KheSpreadEventsMonitorEventGroup(sem);
  tn = KheTransactionMake(soln);
  tc = KheTraceMake(KheEjectorGroupMonitor(ej));

  /* try to move a high meet to a low or very middle time group */
  if( KheSpreadEventsMonitorContainsHighTimeGroup(sem) )
    for( i = 0;  i < KheEventGroupEventCount(eg);  i++ )
    {
      e = KheEventGroupEvent(eg, i);
      for( j = 0;  j < KheEventMeetCount(soln, e);  j++ )
      {
	meet = KheEventMeet(soln, e, j);
	if( KheMeetIsHigh(meet, sem) &&
            KheMoveToLowOrVeryMiddleAugment(ej, true, sem, meet, tn, tc) )
	{
	  KheTraceDelete(tc);
	  KheTransactionDelete(tn);
	  return true;
	}
      }
    }

  /* try to move a very middle meet to a low time group */
  if( KheSpreadEventsMonitorContainsLowTimeGroup(sem) )
    for( i = 0;  i < KheEventGroupEventCount(eg);  i++ )
    {
      e = KheEventGroupEvent(eg, i);
      for( j = 0;  j < KheEventMeetCount(soln, e);  j++ )
      {
	meet = KheEventMeet(soln, e, j);
	if( KheMeetIsVeryMiddle(meet, sem) &&
            KheMoveToLowOrVeryMiddleAugment(ej, false, sem, meet, tn, tc) )
	{
	  KheTraceDelete(tc);
	  KheTransactionDelete(tn);
	  return true;
	}
      }
    }
  return false;
}


/*****************************************************************************/
/*                                                                           */
/*  Submodule "event monitor augment function"                               */
/*                                                                           */
/*****************************************************************************/

/*****************************************************************************/
/*                                                                           */
/*  bool KheEventAugment(KHE_EJECTOR ej, KHE_MONITOR d)                      */
/*                                                                           */
/*  Augment function for repairing time value defects.                       */
/*                                                                           */
/*****************************************************************************/

bool KheEventAugment(KHE_EJECTOR ej, KHE_MONITOR d)
{
  KHE_GROUP_MONITOR dgm;  int i;
  MAssert(KheMonitorCost(d) > 0,
    "KheEventAugment: d does not have non-zero cost");
  switch( KheMonitorTag(d) )
  {
    case KHE_ASSIGN_TIME_MONITOR_TAG:

      return KheAssignTimeMonitorAugment(ej, (KHE_ASSIGN_TIME_MONITOR)d);

    case KHE_PREFER_TIMES_MONITOR_TAG:

      return KhePreferTimesAugment(ej, (KHE_PREFER_TIMES_MONITOR) d);

    case KHE_SPREAD_EVENTS_MONITOR_TAG:

      return KheSpreadEventsAugment(ej, (KHE_SPREAD_EVENTS_MONITOR) d);

    case KHE_SPLIT_EVENTS_MONITOR_TAG:
    case KHE_DISTRIBUTE_SPLIT_EVENTS_MONITOR_TAG:
    case KHE_LINK_EVENTS_MONITOR_TAG:

      return false;

    case KHE_GROUP_MONITOR_TAG:

      dgm = (KHE_GROUP_MONITOR) d;
      KheGroupMonitorCopyDefects(dgm);
      for( i = 0;  i < KheGroupMonitorDefectCopyCount(dgm);  i++ )
      {
	d = KheGroupMonitorDefectCopy(dgm, i);
	if( KheMonitorCost(d) > 0 ) switch( KheMonitorTag(d) )
	{
	  case KHE_ASSIGN_TIME_MONITOR_TAG:

	    if( KheAssignTimeMonitorAugment(ej, (KHE_ASSIGN_TIME_MONITOR) d) )
	      return true;
	    break;

	  case KHE_PREFER_TIMES_MONITOR_TAG:

	    if( KhePreferTimesAugment(ej, (KHE_PREFER_TIMES_MONITOR) d) )
	      return true;
	    break;

	  case KHE_SPREAD_EVENTS_MONITOR_TAG:

	    if( KheSpreadEventsAugment(ej, (KHE_SPREAD_EVENTS_MONITOR) d) )
	      return true;
	    break;

	  case KHE_SPLIT_EVENTS_MONITOR_TAG:
	  case KHE_DISTRIBUTE_SPLIT_EVENTS_MONITOR_TAG:
	  case KHE_LINK_EVENTS_MONITOR_TAG:

	    break;

	  default:

	    MAssert(false, "KheEventAugment: "
	      "d has a child which is not a time value monitor");
	    break;
	}
      }
      break;

    default:

      MAssert(false, "KheEventAugment: d is not a time value monitor");
      break;
  }
  return false;
}


/*****************************************************************************/
/*                                                                           */
/*  Submodule "event resource defects"                                       */
/*                                                                           */
/*****************************************************************************/

/*****************************************************************************/
/*                                                                           */
/*  bool KheAssignResourceAugment(KHE_EJECTOR ej,                            */
/*    KHE_ASSIGN_RESOURCE_MONITOR arm)                                       */
/*                                                                           */
/*  Try to fix the tasks monitored by arm.                                   */
/*                                                                           */
/*****************************************************************************/

static bool KheAssignResourceAugment(KHE_EJECTOR ej,
  KHE_ASSIGN_RESOURCE_MONITOR arm)
{
  KHE_SOLN soln;  KHE_EVENT_RESOURCE er;  KHE_TASK task;  int i, j;
  KHE_RESOURCE_GROUP rg;  KHE_RESOURCE r;  KHE_TRACE tc;
  KHE_TRANSACTION tn;  bool res;  KHE_COST save_cost;
  if( DEBUG7 )
    fprintf(stderr, "%*s[ AssignResourceAugment\n",
      4*KheEjectorCurrDepth(ej) + 2, "");
  soln = KheMonitorSoln((KHE_MONITOR) KheEjectorGroupMonitor(ej));
  save_cost = KheSolnCost(soln);
  er = KheAssignResourceMonitorEventResource(arm);
  tc = KheTraceMake(KheEjectorGroupMonitor(ej));
  tn = KheTransactionMake(KheEjectorSoln(ej));
  for( i = 0;  i < KheEventResourceTaskCount(soln, er);  i++ )
  {
    task = KheTaskLeader(KheEventResourceTask(soln, er, i));
    if( KheTaskAsst(task) == NULL && !KheTaskVisited(task, 0) )
    {
      if( DEBUG7 )
      {
	fprintf(stderr, "%*s[ assigning ", 4*KheEjectorCurrDepth(ej) + 4, "");
	KheTaskDebug(task, 1, 0, stderr);
      }
      KheTaskVisit(task);
      rg = KheTaskDomain(task);
      for( j = 0;  j < KheResourceGroupResourceCount(rg);  j++ )
      {
	r = KheResourceGroupResource(rg, j);
	MAssert(KheSolnCost(soln) == save_cost,
	  "internal error in KheAssignResourceAugment 1: %.4f != %.4f",
	  KheCostShow(KheSolnCost(soln)), KheCostShow(save_cost));
	KheTransactionBegin(tn);
	KheTraceBegin(tc);
	res = KheTaskKempeAssignResource(task, r);
	if( DEBUG7 )
	  fprintf(stderr, "%*s  trying %s%s\n", 4*KheEjectorCurrDepth(ej) + 4,
	    "", KheResourceId(r) == NULL ? "-" : KheResourceId(r),
	    res ? "" : " (Kempe resource move failed)");
	KheTraceEnd(tc);
	KheTransactionEnd(tn);
	if( res && KheEjectorSuccess(ej, tc, KheTaskDuration(task)) )
	{
	  KheTraceDelete(tc);
	  KheTransactionDelete(tn);
	  if( DEBUG7 )
	  {
	    fprintf(stderr, "%*s] success\n", 4*KheEjectorCurrDepth(ej)+4, "");
	    fprintf(stderr, "%*s] AssignResourceAugment success\n",
	      4*KheEjectorCurrDepth(ej) + 2, "");
	  }
	  return true;
	}
	KheTransactionUndo(tn);
	if( DEBUG3 && KheSolnCost(soln) != save_cost )
	{
	  int k;  KHE_MONITOR m;  KHE_LIMIT_WORKLOAD_MONITOR lwm;
          KHE_LIMIT_WORKLOAD_CONSTRAINT lwc;
	  fprintf(stderr, "  KheAssignResourceAugment failing on %s\n",
	    KheResourceId(r) == NULL ? "-" : KheResourceId(r));
	  for( k = 0;  k < KheResourceMonitorCount(soln, r);  k++ )
	  {
	    m = KheResourceMonitor(soln, r, k);
	    if( KheMonitorTag(m) == KHE_LIMIT_WORKLOAD_MONITOR_TAG )
	    {
	      lwm = (KHE_LIMIT_WORKLOAD_MONITOR) m;
	      lwc = KheLimitWorkloadMonitorConstraint(lwm);
	      fprintf(stderr, "    workload %.4f, lim %d..%d\n",
		KheLimitWorkloadMonitorWorkload(lwm),
		KheLimitWorkloadConstraintMinimum(lwc),
		KheLimitWorkloadConstraintMaximum(lwc));
	    }
	  }
	}
	MAssert(KheSolnCost(soln) == save_cost,
	  "internal error in KheAssignResourceAugment 2: %.4f != %.4f",
	  KheCostShow(KheSolnCost(soln)), KheCostShow(save_cost));
      }
      if( KheEjectorScheduleMayRevisit(KheEjectorCurrSchedule(ej)) )
	KheTaskUnVisit(task);
      if( DEBUG7 )
	fprintf(stderr, "%*s] fail\n", 4*KheEjectorCurrDepth(ej) + 4, "");
    }
  }
  KheTraceDelete(tc);
  KheTransactionDelete(tn);
  if( DEBUG7 )
    fprintf(stderr, "%*s] AssignResourceAugment fail\n",
      4*KheEjectorCurrDepth(ej) + 2, "");
  MAssert(KheSolnCost(soln) == save_cost,
    "internal error in KheAssignResourceAugment 3: %.4f != %.4f",
    KheCostShow(KheSolnCost(soln)), KheCostShow(save_cost));
  return false;
}


/*****************************************************************************/
/*                                                                           */
/*  bool KhePreferResourcesAugment(KHE_EJECTOR ej,                           */
/*    KHE_PREFER_RESOURCES_MONITOR prm)                                      */
/*                                                                           */
/*  Try to fix the solution resources monitored by prm.                      */
/*                                                                           */
/*****************************************************************************/

static bool KhePreferResourcesAugment(KHE_EJECTOR ej,
  KHE_PREFER_RESOURCES_MONITOR prm)
{
  KHE_SOLN soln;  KHE_EVENT_RESOURCE er;  KHE_TASK task;  int i, j;
  KHE_RESOURCE_GROUP rg;  KHE_RESOURCE r, old_r;  KHE_TRACE tc;  bool success;
  KHE_PREFER_RESOURCES_CONSTRAINT prc;  KHE_TRANSACTION tn;
  soln = KheMonitorSoln((KHE_MONITOR) KheEjectorGroupMonitor(ej));
  er = KhePreferResourcesMonitorEventResource(prm);
  prc = KhePreferResourcesMonitorConstraint(prm);
  rg = KhePreferResourcesConstraintDomain(prc);
  tn = KheTransactionMake(soln);
  tc = KheTraceMake(KheEjectorGroupMonitor(ej));
  for( i = 0;  i < KheEventResourceTaskCount(soln, er);  i++ )
  {
    task = KheTaskLeader(KheEventResourceTask(soln, er, i));
    if( KheTaskAsst(task) != NULL )
    {
      old_r = KheTaskAsstResource(task);
      MAssert(old_r != NULL, "KhePreferResourcesAugment internal error");
      if( !KheTaskVisited(task, 0) && !KheResourceGroupContains(rg, old_r) )
      {
	KheTaskVisit(task);
	for( j = 0;  j < KheResourceGroupResourceCount(rg);  j++ )
	{
	  r = KheResourceGroupResource(rg, j);
	  KheTransactionBegin(tn);
	  KheTraceBegin(tc);
	  KheTaskUnAssignResource(task);
	  success = KheTaskKempeAssignResource(task, r);
	  KheTraceEnd(tc);
	  KheTransactionEnd(tn);
	  if( success && KheEjectorSuccess(ej, tc, KheTaskDuration(task)) )
	  {
	    KheTraceDelete(tc);
	    KheTransactionDelete(tn);
	    return true;
	  }
	  KheTransactionUndo(tn);
	}
	if( KheEjectorScheduleMayRevisit(KheEjectorCurrSchedule(ej)) )
	  KheTaskUnVisit(task);
      }
    }
  }
  KheTraceDelete(tc);
  KheTransactionDelete(tn);
  return false;
}


/*****************************************************************************/
/*                                                                           */
/*  void KheVisitAllTasks(KHE_AVOID_SPLIT_ASSIGNMENTS_MONITOR asam)          */
/*                                                                           */
/*  Mark all tasks monitored by asam unvisited.                              */
/*                                                                           */
/*****************************************************************************/

static void KheVisitAllTasks(KHE_AVOID_SPLIT_ASSIGNMENTS_MONITOR asam)
{
  KHE_AVOID_SPLIT_ASSIGNMENTS_CONSTRAINT c;  int egi, count, i, j;
  KHE_EVENT_RESOURCE er;  KHE_SOLN soln;  KHE_TASK task;
  c = KheAvoidSplitAssignmentsMonitorConstraint(asam);
  egi = KheAvoidSplitAssignmentsMonitorEventGroupIndex(asam);
  count = KheAvoidSplitAssignmentsConstraintEventResourceCount(c, egi);
  soln = KheMonitorSoln((KHE_MONITOR) asam);
  for( i = 0;  i < count;  i++ )
  {
    er = KheAvoidSplitAssignmentsConstraintEventResource(c, egi, i);
    for( j = 0;  j < KheEventResourceTaskCount(soln, er);  j++ )
    {
      task = KheTaskLeader(KheEventResourceTask(soln, er, j));
      KheTaskVisit(task);
    }
  }
}


/*****************************************************************************/
/*                                                                           */
/*  void KheUnVisitAllTasks(KHE_AVOID_SPLIT_ASSIGNMENTS_MONITOR asam)        */
/*                                                                           */
/*  Mark all tasks monitored by asam unvisited.                              */
/*                                                                           */
/*****************************************************************************/

static void KheUnVisitAllTasks(KHE_AVOID_SPLIT_ASSIGNMENTS_MONITOR asam)
{
  KHE_AVOID_SPLIT_ASSIGNMENTS_CONSTRAINT c;  int egi, count, i, j;
  KHE_EVENT_RESOURCE er;  KHE_SOLN soln;  KHE_TASK task;
  c = KheAvoidSplitAssignmentsMonitorConstraint(asam);
  egi = KheAvoidSplitAssignmentsMonitorEventGroupIndex(asam);
  count = KheAvoidSplitAssignmentsConstraintEventResourceCount(c, egi);
  soln = KheMonitorSoln((KHE_MONITOR) asam);
  for( i = 0;  i < count;  i++ )
  {
    er = KheAvoidSplitAssignmentsConstraintEventResource(c, egi, i);
    for( j = 0;  j < KheEventResourceTaskCount(soln, er);  j++ )
    {
      task = KheTaskLeader(KheEventResourceTask(soln, er, j));
      KheTaskUnVisit(task);
    }
  }
}


/*****************************************************************************/
/*                                                                           */
/*  bool KheAvoidSplitAssignmentsAugment(KHE_EJECTOR ej,                     */
/*    KHE_AVOID_SPLIT_ASSIGNMENTS_MONITOR asam)                              */
/*                                                                           */
/*  Try to fix the tasks monitored by asam.                                  */
/*                                                                           */
/*****************************************************************************/

static bool KheAvoidSplitAssignmentsAugment(KHE_EJECTOR ej,
  KHE_AVOID_SPLIT_ASSIGNMENTS_MONITOR asam)
{
  KHE_AVOID_SPLIT_ASSIGNMENTS_CONSTRAINT c;  int egi, count, i, j, k, durn;
  KHE_RESOURCE r;  KHE_TRACE tc;  KHE_TRANSACTION tn;  KHE_EVENT_RESOURCE er;
  KHE_SOLN soln;  KHE_TASK task;  KHE_RESOURCE_GROUP rg;  bool success;
  if( DEBUG12 )
  {
    fprintf(stderr, "[ KheAvoidSplitAssignmentsAugment:\n");
    KheMonitorDebug((KHE_MONITOR) asam, 2, 2, stderr);
  }

  /* if there are distinct resources assigned to visited tasks, fail */
  c = KheAvoidSplitAssignmentsMonitorConstraint(asam);
  egi = KheAvoidSplitAssignmentsMonitorEventGroupIndex(asam);
  count = KheAvoidSplitAssignmentsConstraintEventResourceCount(c, egi);
  soln = KheEjectorSoln(ej);
  r = NULL;
  task = NULL;
  for( i = 0;  i < count;  i++ )
  {
    er = KheAvoidSplitAssignmentsConstraintEventResource(c, egi, i);
    for( j = 0;  j < KheEventResourceTaskCount(soln, er);  j++ )
    {
      task = KheTaskLeader(KheEventResourceTask(soln, er, j));
      if( KheTaskVisited(task, 0) && KheTaskAsstResource(task) != NULL )
      {
	if( r != NULL && KheTaskAsstResource(task) != r )
	{
	  if( DEBUG12 )
	    fprintf(stderr,
	      "] KheAvoidSplitAssignmentsAugment returning false (visited)\n");
	  return false;
	}
	else
	  r = KheTaskAsstResource(task);
      }
    }
  }
  MAssert(task != NULL,"KheAvoidSplitAssignmentsAugment internal error");

  /* visit all tasks */
  KheVisitAllTasks(asam);

  /* the possible resources are just r if non-NULL, else a task's domain */
  rg = (r != NULL ? KheResourceSingletonResourceGroup(r) : KheTaskDomain(task));
  tc = KheTraceMake(KheEjectorGroupMonitor(ej));
  tn = KheTransactionMake(soln);
  for( i = 0;  i < KheResourceGroupResourceCount(rg);  i++ )
  {
    r = KheResourceGroupResource(rg, i);
    if( DEBUG12 )
      fprintf(stderr, "  trying %s\n", KheResourceId(r) == NULL ? "-" :
        KheResourceId(r));

    /* trace a transaction which ensures that r is assigned to every task */
    KheTransactionBegin(tn);
    KheTraceBegin(tc);
    success = true;
    durn = 0;
    for( j = 0;  success && j < count;  j++ )
    {
      er = KheAvoidSplitAssignmentsConstraintEventResource(c, egi, j);
      for( k = 0;  success && k < KheEventResourceTaskCount(soln, er);  k++ )
      {
	task = KheTaskLeader(KheEventResourceTask(soln, er, k));
	if( KheTaskAsstResource(task) != r )
	{
	  if( KheTaskAsstResource(task) != NULL )
	    KheTaskUnAssignResource(task);
	  success = success && KheTaskKempeAssignResource(task, r);
	  durn += KheTaskDuration(task);
	}
      }
    }
    KheTraceEnd(tc);
    KheTransactionEnd(tn);

    /* check for success and undo if not */
    if( success && KheEjectorSuccess(ej, tc, durn) )
    {
      KheTraceDelete(tc);
      KheTransactionDelete(tn);
      if( DEBUG12 )
	fprintf(stderr, "] KheAvoidSplitAssignmentsAugment returning true\n");
      return true;
    }
    KheTransactionUndo(tn);
  }
  KheTraceDelete(tc);
  KheTransactionDelete(tn);
  if( KheEjectorScheduleMayRevisit(KheEjectorCurrSchedule(ej)) )
    KheUnVisitAllTasks(asam);
  if( DEBUG12 )
    fprintf(stderr, "] KheAvoidSplitAssignmentsAugment returning false\n");
  return false;
}


/*****************************************************************************/
/*                                                                           */
/*  bool KheEventResourceAugment(KHE_EJECTOR ej, KHE_MONITOR d)              */
/*                                                                           */
/*  Augment function for repairing event resource defects.                   */
/*                                                                           */
/*****************************************************************************/

bool KheEventResourceAugment(KHE_EJECTOR ej, KHE_MONITOR d)
{
  KHE_GROUP_MONITOR dgm;  int i;
  MAssert(KheMonitorCost(d) > 0,
    "KheEventResourceAugment: d does not have non-zero cost");
  switch( KheMonitorTag(d) )
  {
    case KHE_ASSIGN_RESOURCE_MONITOR_TAG:

      return KheAssignResourceAugment(ej, (KHE_ASSIGN_RESOURCE_MONITOR) d);

    case KHE_PREFER_RESOURCES_MONITOR_TAG:

      return KhePreferResourcesAugment(ej, (KHE_PREFER_RESOURCES_MONITOR) d);

    case KHE_AVOID_SPLIT_ASSIGNMENTS_MONITOR_TAG:

      return KheAvoidSplitAssignmentsAugment(ej,
	(KHE_AVOID_SPLIT_ASSIGNMENTS_MONITOR) d);

    case KHE_GROUP_MONITOR_TAG:

      dgm = (KHE_GROUP_MONITOR) d;
      KheGroupMonitorCopyDefects(dgm);
      for( i = 0;  i < KheGroupMonitorDefectCopyCount(dgm);  i++ )
      {
	d = KheGroupMonitorDefectCopy(dgm, i);
	if( KheMonitorCost(d) > 0 )  switch( KheMonitorTag(d) )
	{
	  case KHE_ASSIGN_RESOURCE_MONITOR_TAG:

	    if( KheAssignResourceAugment(ej, (KHE_ASSIGN_RESOURCE_MONITOR) d) )
	      return true;
	    break;

	  case KHE_PREFER_RESOURCES_MONITOR_TAG:

	    if( KhePreferResourcesAugment(ej, (KHE_PREFER_RESOURCES_MONITOR)d) )
	      return true;
	    break;

	  case KHE_AVOID_SPLIT_ASSIGNMENTS_MONITOR_TAG:

	    if( KheAvoidSplitAssignmentsAugment(ej,
		  (KHE_AVOID_SPLIT_ASSIGNMENTS_MONITOR) d) )
	      return true;
	    break;

	  default:

	    MAssert(false, "KheEventResourceAugment: "
	      "d has a child which is not a resource value monitor");
	    break;
	}
      }
      break;

    default:

      MAssert(false,
	"KheEventResourceAugment: d is not a resource value monitor");
      break;
  }
  return false;
}


/*****************************************************************************/
/*                                                                           */
/*  Submodule "resource defects"                                             */
/*                                                                           */
/*****************************************************************************/

/*****************************************************************************/
/*                                                                           */
/*  bool KheAvoidClashesAugment(KHE_EJECTOR ej,                              */
/*    KHE_AVOID_CLASHES_MONITOR acm)                                         */
/*                                                                           */
/*  Try to fix the resource timetable monitored by acm.                      */
/*                                                                           */
/*****************************************************************************/

static bool KheAvoidClashesAugment(KHE_EJECTOR ej,
  KHE_AVOID_CLASHES_MONITOR acm)
{
  KHE_RESOURCE r;
  r = KheAvoidClashesMonitorResource(acm);
  if( DEBUG9 )
    fprintf(stderr, "%*s[ AvoidClashesAugment (%s)\n",
      4*KheEjectorCurrDepth(ej) + 2, "",
      KheResourceId(r) != NULL ? KheResourceId(r) : "-");
  /* still to do */
  if( DEBUG9 )
    fprintf(stderr, "%*s] AvoidClashesAugment returning false (still to do)\n",
      4*KheEjectorCurrDepth(ej) + 2, "");
  return false;
}


/*****************************************************************************/
/*                                                                           */
/*  bool KheAvoidUnavailableTimesAugment(KHE_EJECTOR ej,                     */
/*    KHE_AVOID_UNAVAILABLE_TIMES_MONITOR autm)                              */
/*                                                                           */
/*  Try to fix the resource timetable monitored by autm.                     */
/*                                                                           */
/*****************************************************************************/

static bool KheAvoidUnavailableTimesAugment(KHE_EJECTOR ej,
  KHE_AVOID_UNAVAILABLE_TIMES_MONITOR autm)
{
  KHE_RESOURCE r;
  r = KheAvoidUnavailableTimesMonitorResource(autm);
  if( DEBUG9 )
    fprintf(stderr, "%*s[ AvoidUnavailableTimesAugment (%s)\n",
      4*KheEjectorCurrDepth(ej) + 2, "",
      KheResourceId(r) != NULL ? KheResourceId(r) : "-");
  /* still to do */
  if( DEBUG9 )
    fprintf(stderr,
      "%*s] AvoidUnavailableTimesAugment returning false (still to do)\n",
      4*KheEjectorCurrDepth(ej) + 2, "");
  return false;
}


/*****************************************************************************/
/*                                                                           */
/*  bool KheLimitIdleTimesAugment(KHE_EJECTOR ej,                            */
/*    KHE_LIMIT_IDLE_TIMES_MONITOR litm)                                     */
/*                                                                           */
/*  Try to fix the resource timetable monitored by litm.                     */
/*                                                                           */
/*****************************************************************************/

static bool KheLimitIdleTimesAugment(KHE_EJECTOR ej,
  KHE_LIMIT_IDLE_TIMES_MONITOR litm)
{
  /* still to do */
  //I COMMENTED THIS CODE!!!
  //MAssert(false, "KheLimitIdleTimesAugment still to do");
  return false;
}


/*****************************************************************************/
/*                                                                           */
/*  bool KheClusterBusyTimesAugment(KHE_EJECTOR ej,                          */
/*    KHE_CLUSTER_BUSY_TIMES_MONITOR cbtm)                                   */
/*                                                                           */
/*  Try to fix the resource timetable monitored by cbtm.                     */
/*                                                                           */
/*****************************************************************************/

static bool KheClusterBusyTimesAugment(KHE_EJECTOR ej,
  KHE_CLUSTER_BUSY_TIMES_MONITOR cbtm)
{
  /* still to do */
  //I COMMENTED THIS CODE
  //MAssert(false, "KheClusterBusyTimesAugment still to do");
  return false;
}


/*****************************************************************************/
/*                                                                           */
/*  bool KheBusyTimesOrWorkloadAugment(KHE_EJECTOR ej,                       */
/*    KHE_RESOURCE r)                                                        */
/*                                                                           */
/*  Repair a busy times or workload defect by deassigning one task.          */
/*                                                                           */
/*****************************************************************************/

static bool KheBusyTimesOrWorkloadAugment(KHE_EJECTOR ej,
  KHE_RESOURCE r)
{
  ARRAY_KHE_TASK tasks;  KHE_TASK task;  KHE_SOLN soln;  int i;
  KHE_TRACE tc;  KHE_RESOURCE junk;  KHE_COST save_cost;

  /* gather the unvisited tasks assigned directly to the cycle task */
  soln = KheEjectorSoln(ej);
  save_cost = KheSolnCost(soln);
  if( DEBUG6 )
    fprintf(stderr, "%*s[ LimitWorkloadAugment (%s)\n",
      4*KheEjectorCurrDepth(ej) + 2, "",
      KheResourceId(r) != NULL ? KheResourceId(r) : "-");
  MArrayInit(tasks);
  for( i = 0;  i < KheResourceAssignedTaskCount(soln, r);  i++ )
  {
    task = KheResourceAssignedTask(soln, r, i);
    MAssert(KheTaskAsst(task) != NULL,
      "KheLimitWorkloadAugment internal error");
    if( KheTaskIsLeader(task) && !KheTaskVisited(task, 0) &&
	!KheTaskIsPreassigned(task, false, &junk) )
      MArrayAddLast(tasks, task);
  }

  /* one repair deassigns one task (still unvisited and assigned r)  */
  tc = KheTraceMake(KheEjectorGroupMonitor(ej));
  MArrayForEach(tasks, &task, &i)
    if( !KheTaskVisited(task, 0) && KheTaskAsstResource(task) == r )
    {
      /* KheTaskVisit(task); don't consider this a visit! */
      KheTraceBegin(tc);
      if( DEBUG6 )
      {
	fprintf(stderr, "%*s[ deassign %s from ",
	  4*KheEjectorCurrDepth(ej) + 4, "",
          KheResourceId(r) != NULL ? KheResourceId(r) : "-");
	KheTaskDebug(task, 1, 0, stderr);
      }
      KheTaskUnAssignResource(task);
      KheTraceEnd(tc);
      if( KheEjectorSuccess(ej, tc, KheTaskDuration(task)) )
      {
	KheTraceDelete(tc);
	MArrayFree(tasks);
	if( DEBUG6 )
	{
	  fprintf(stderr, "%*s] success\n", 4*KheEjectorCurrDepth(ej) + 4, "");
	  fprintf(stderr, "%*s] LimitWorkloadAugment success\n",
	    4*KheEjectorCurrDepth(ej) + 2, "");
	}
	return true;
      }
      if( DEBUG6 )
	fprintf(stderr, "%*s] fail\n", 4*KheEjectorCurrDepth(ej) + 4, "");
      if( !KheTaskAssignResource(task, r) )
	MAssert(false, "KheEjectorLimitWorkloadAugment internal error");
      /* *** don't consider this a visit
      if( KheEjectorMayRevisit(ej) )
	KheTaskUnVisit(task);
      *** */
    }
  KheTraceDelete(tc);
  MArrayFree(tasks);
  if( DEBUG6 )
    fprintf(stderr, "%*s] LimitWorkloadAugment fail\n",
      4*KheEjectorCurrDepth(ej) + 2, "");
  MAssert(KheSolnCost(soln) == save_cost,
    "internal error in KheBusyTimesOrWorkloadAugment: %.4f != %.4f",
    KheCostShow(KheSolnCost(soln)), KheCostShow(save_cost));
  return false;
}


/*****************************************************************************/
/*                                                                           */
/*  bool KheLimitBusyTimesAugment(KHE_EJECTOR ej,                            */
/*    KHE_LIMIT_BUSY_TIMES_MONITOR lbtm)                                     */
/*                                                                           */
/*  Try to fix the resource timetable monitored by lbtm.                     */
/*                                                                           */
/*  Limiting this call to overload cases is still to do.                     */
/*                                                                           */
/*****************************************************************************/

static bool KheLimitBusyTimesAugment(KHE_EJECTOR ej,
  KHE_LIMIT_BUSY_TIMES_MONITOR lbtm)
{
  /* handling underloads here is still to do */
  return KheBusyTimesOrWorkloadAugment(ej,
    KheLimitBusyTimesMonitorResource(lbtm));
}


/*****************************************************************************/
/*                                                                           */
/*  bool KheLimitWorkloadAugment(KHE_EJECTOR ej,                             */
/*    KHE_LIMIT_WORKLOAD_MONITOR lwm)                                        */
/*                                                                           */
/*  Try to fix the resource timetable monitored by lwm.                      */
/*                                                                           */
/*****************************************************************************/

static bool KheLimitWorkloadAugment(KHE_EJECTOR ej,
  KHE_LIMIT_WORKLOAD_MONITOR lwm)
{
  KHE_LIMIT_WORKLOAD_CONSTRAINT lwc;  float workload;
  lwc = KheLimitWorkloadMonitorConstraint(lwm);
  workload = KheLimitWorkloadMonitorWorkload(lwm);
  if( workload < KheLimitWorkloadConstraintMinimum(lwc) )
  {
    /* resource is underloaded, repairing this is still to do */
    return false;
  }
  else if( workload > KheLimitWorkloadConstraintMaximum(lwc) )
  {
    /* resource is overloaded */
    return KheBusyTimesOrWorkloadAugment(ej,
      KheLimitWorkloadMonitorResource(lwm));
  }
  else
  {
    MAssert(false, "KheLimitWorkloadAugment internal error");
    return false;  /* keep compiler happy */
  }
}


/*****************************************************************************/
/*                                                                           */
/*  bool KheResourceAugment(KHE_EJECTOR ej, KHE_MONITOR d)                   */
/*                                                                           */
/*  Augment function for repairing resource defects.                         */
/*                                                                           */
/*  Limiting this call to overload cases is still to do.                     */
/*                                                                           */
/*****************************************************************************/

bool KheResourceAugment(KHE_EJECTOR ej, KHE_MONITOR d)
{
  KHE_GROUP_MONITOR dgm;  int i;
  MAssert(KheMonitorCost(d) > 0,
    "KheResourceAugment: d does not have non-zero cost");
  switch( KheMonitorTag(d) )
  {
    case KHE_AVOID_CLASHES_MONITOR_TAG:

      return KheAvoidClashesAugment(ej, (KHE_AVOID_CLASHES_MONITOR) d);

    case KHE_AVOID_UNAVAILABLE_TIMES_MONITOR_TAG:

      return KheAvoidUnavailableTimesAugment(ej,
	(KHE_AVOID_UNAVAILABLE_TIMES_MONITOR) d);

    case KHE_LIMIT_IDLE_TIMES_MONITOR_TAG:

      return KheLimitIdleTimesAugment(ej, (KHE_LIMIT_IDLE_TIMES_MONITOR) d);

    case KHE_CLUSTER_BUSY_TIMES_MONITOR_TAG:

      return KheClusterBusyTimesAugment(ej, (KHE_CLUSTER_BUSY_TIMES_MONITOR) d);

    case KHE_LIMIT_BUSY_TIMES_MONITOR_TAG:

      return KheLimitBusyTimesAugment(ej, (KHE_LIMIT_BUSY_TIMES_MONITOR) d);

    case KHE_LIMIT_WORKLOAD_MONITOR_TAG:

      return KheLimitWorkloadAugment(ej, (KHE_LIMIT_WORKLOAD_MONITOR) d);

    case KHE_GROUP_MONITOR_TAG:

      dgm = (KHE_GROUP_MONITOR) d;
      KheGroupMonitorCopyDefects(dgm);
      for( i = 0;  i < KheGroupMonitorDefectCopyCount(dgm);  i++ )
      {
	d = KheGroupMonitorDefect(dgm, i);
	if( KheMonitorCost(d) > 0 ) switch( KheMonitorTag(d) )
	{
	  case KHE_AVOID_CLASHES_MONITOR_TAG:

	    if( KheAvoidClashesAugment(ej, (KHE_AVOID_CLASHES_MONITOR) d) )
	      return true;
	    break;

	  case KHE_AVOID_UNAVAILABLE_TIMES_MONITOR_TAG:

	    if( KheAvoidUnavailableTimesAugment(ej,
		(KHE_AVOID_UNAVAILABLE_TIMES_MONITOR) d) )
	      return true;
	    break;

	  case KHE_LIMIT_IDLE_TIMES_MONITOR_TAG:

	    if( KheLimitIdleTimesAugment(ej, (KHE_LIMIT_IDLE_TIMES_MONITOR) d) )
	      return true;
	    break;

	  case KHE_CLUSTER_BUSY_TIMES_MONITOR_TAG:

	    if( KheClusterBusyTimesAugment(ej,
		  (KHE_CLUSTER_BUSY_TIMES_MONITOR) d) )
	      return true;
	    break;

	  case KHE_LIMIT_BUSY_TIMES_MONITOR_TAG:

	    if( KheLimitBusyTimesAugment(ej, (KHE_LIMIT_BUSY_TIMES_MONITOR) d) )
	      return true;
	    break;

	  case KHE_LIMIT_WORKLOAD_MONITOR_TAG:

	    if( KheLimitWorkloadAugment(ej, (KHE_LIMIT_WORKLOAD_MONITOR) d) )
	      return true;
	    break;

	  default:

	    MAssert(false, "KheResourceAugment: "
	      "d has a child which is not a resource monitor");
	    break;
	}
      }
      break;

    default:

      MAssert(false, "KheResourceAugment: d is not a resource monitor");
      break;
  }
  return false;
}


/*****************************************************************************/
/*                                                                           */
/*  Submodule "demand defects (meet moves)"                                  */
/*                                                                           */
/*****************************************************************************/

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
  KHE_MEET meet;  bool contributed;
  meet = KheTaskMeet(KheOrdinaryDemandMonitorTask(odm));
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

/* *** hmmm - this version assumes that times have been assigned!
static void KheAddMeets(ARRAY_KHE_MEET *meets,
  KHE_ORDINARY_DEMAND_MONITOR odm, int *count)
{
  KHE_MEET meet;  bool contributed;
  meet = KheTaskMeet(KheOrdinaryDemandMonitorTask(odm));
  contributed = false;
  if( KheMeetAsstTime(meet) != NULL )
    while( !KheMeetIsCycleMeet(meet) )
    {
      MAssert(KheMeetAsst(meet) != NULL,
	"KheOrdinaryDemandAugment internal error");
      if( KheMeetNode(meet) != NULL && KheNodeParent(KheMeetNode(meet))!=NULL )
      {
	MArrayAddLast(*meets, meet);
	contributed = true;
      }
      meet = KheMeetAsst(meet);
    }
  if( contributed )
    (*count)++;
}
*** */


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
/*  bool KheTryAllOffsets(KHE_EJECTOR ej, KHE_MEET meet,                     */
/*    KHE_MEET target_meet, KHE_TRANSACTION tn, KHE_TRACE tc)                */
/*                                                                           */
/*  Try a Kempe time move to each legal offset of target_meet.               */
/*                                                                           */
/*****************************************************************************/

static bool KheTryAllOffsets(KHE_EJECTOR ej, KHE_MEET meet,
  KHE_MEET target_meet, KHE_TRANSACTION tn, KHE_TRACE tc)
{
  int max_offset, offset;
  max_offset = KheMeetDuration(target_meet) - KheMeetDuration(meet);
  for( offset = 0;  offset <= max_offset;  offset++ )
    if( KheKempeTimeMoveAugment(ej, meet, target_meet, offset, tn, tc) )
      return true;
  return false;
}


/*****************************************************************************/
/*                                                                           */
/*  bool KheKempeMoveAugment(KHE_EJECTOR ej, KHE_MEET meet,                  */
/*    KHE_TRANSACTION tn, KHE_TRACE tc)                                      */
/*                                                                           */
/*  Here meet is movable according to the meet rules, but it may already     */
/*  be visited.  If not, move it to all possible target meets and offsets,   */
/*  closest first.                                                           */
/*                                                                           */
/*  Parameters tn and tc are scratch transaction and trace monitors.         */
/*                                                                           */
/*****************************************************************************/

static bool KheKempeMoveAugment(KHE_EJECTOR ej, KHE_MEET meet,
  KHE_TRANSACTION tn, KHE_TRACE tc)
{
  KHE_NODE parent_node;  int pos, i, target_offset, max_offset;
  KHE_MEET target_meet;
  MAssert(KheMeetAsst(meet) != NULL,
    "KheKempeMoveAugment internal error 1");
  MAssert(KheMeetNode(meet) != NULL && KheNodeParent(KheMeetNode(meet)) != NULL,
    "KheKempeMoveAugment internal error 2");
  if( !KheMeetVisited(meet, 0) )
  {
    KheMeetVisit(meet);

    /* try other offsets in the current target meet, closest first */
    target_meet = KheMeetAsst(meet);
    target_offset = KheMeetAsstOffset(meet);
    max_offset = KheMeetDuration(target_meet) - KheMeetDuration(meet);
    for( i = 1;  i < KheMeetDuration(target_meet);  i++ )
    {
      if( target_offset - i >= 0 && KheKempeTimeMoveAugment(ej, meet,
	  target_meet, target_offset - i, tn, tc) )
	return true;
      if( target_offset + i <= max_offset && KheKempeTimeMoveAugment(ej,
	  meet, target_meet, target_offset - i, tn, tc) )
	return true;
    }

    /* find pos, the position of meet's assignment in the parent node */
    parent_node = KheNodeParent(KheMeetNode(meet));
    for( pos = 0;  pos < KheNodeMeetCount(parent_node);  pos++ )
      if( KheNodeMeet(parent_node, pos) == KheMeetAsst(meet) )
	break;
    MAssert(pos < KheNodeMeetCount(parent_node),
      "KheKempeMoveAugment internal error 3");

    /* try other meets in the parent node, closest to pos first */
    /* (this helps preserve spread when parent_node is the cycle node) */
    for( i = 1;  i < KheNodeMeetCount(parent_node);  i++ )
    {
      if( pos - i >= 0 &&
	  KheTryAllOffsets(ej, meet, KheNodeMeet(parent_node, pos-i), tn, tc) )
	return true;
      if( pos + i < KheNodeMeetCount(parent_node) &&
	  KheTryAllOffsets(ej, meet, KheNodeMeet(parent_node, pos+i), tn, tc) )
	return true;
    }

    if( KheEjectorScheduleMayRevisit(KheEjectorCurrSchedule(ej)) )
      KheMeetUnVisit(meet);
  }
  return false;
}


/*****************************************************************************/
/*                                                                           */
/*  bool KheOrdinaryDemandAugment(KHE_EJECTOR ej,                            */
/*    KHE_ORDINARY_DEMAND_MONITOR odm)                                       */
/*                                                                           */
/*  Try to fix the ordinary demand problem reported by odm, by moving        */
/*  its meet or one of its competitors' meets.                               */
/*                                                                           */
/*****************************************************************************/

static bool KheOrdinaryDemandAugment(KHE_EJECTOR ej,
  KHE_ORDINARY_DEMAND_MONITOR odm)
{
  KHE_MONITOR m;  int i, count;  ARRAY_KHE_MEET meets;  KHE_MEET meet;
  KHE_TRANSACTION tn;  KHE_TRACE tc;

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

  /* try a Kempe move of each element of meets */
  if( MArraySize(meets) > 0 )
  {
    tn = KheTransactionMake(KheEjectorSoln(ej));
    tc = KheTraceMake(KheEjectorGroupMonitor(ej));
    MArrayForEach(meets, &meet, &i)
      if( KheKempeMoveAugment(ej, meet, tn, tc) )
      {
	MArrayFree(meets);
	KheTransactionDelete(tn);
	KheTraceDelete(tc);
	return true;
      }
    MArrayFree(meets);
    KheTransactionDelete(tn);
    KheTraceDelete(tc);
  }
  return false;
}


/*****************************************************************************/
/*                                                                           */
/*  bool KheWorkloadDemandAugment(KHE_EJECTOR ej,                            */
/*    KHE_WORKLOAD_DEMAND_MONITOR wdm)                                       */
/*                                                                           */
/*  Try to fix the workload demand problem reported by odm, by moving        */
/*  its competitors' meets.                                                  */
/*                                                                           */
/*  At present this function is just a stub, and it will probably remain     */
/*  that way.  Workload demands are inserted first and never removed, and    */
/*  this means that they get first bite at the available supply and so in    */
/*  practice are never unmatched.  So it is hardly worthwhile to implement   */
/*  this function properly.                                                  */
/*                                                                           */
/*****************************************************************************/

static bool KheWorkloadDemandAugment(KHE_EJECTOR ej,
  KHE_WORKLOAD_DEMAND_MONITOR wdm)
{
  return false;
}


/*****************************************************************************/
/*                                                                           */
/*  bool KheDemandAugment(KHE_EJECTOR ej, KHE_MONITOR d)                     */
/*                                                                           */
/*  Augment function for repairing demand defects.                           */
/*                                                                           */
/*****************************************************************************/

bool KheDemandAugment(KHE_EJECTOR ej, KHE_MONITOR d)
{
  KHE_GROUP_MONITOR dgm;  int i;
  MAssert(KheMonitorCost(d) > 0,
    "KheDemandAugment: d does not have non-zero cost");
  switch( KheMonitorTag(d) )
  {
    case KHE_ORDINARY_DEMAND_MONITOR_TAG:

      return KheOrdinaryDemandAugment(ej, (KHE_ORDINARY_DEMAND_MONITOR) d);

    case KHE_WORKLOAD_DEMAND_MONITOR_TAG:

      return KheWorkloadDemandAugment(ej, (KHE_WORKLOAD_DEMAND_MONITOR) d);

    case KHE_GROUP_MONITOR_TAG:

      dgm = (KHE_GROUP_MONITOR) d;
      KheGroupMonitorCopyDefects(dgm);
      for( i = 0;  i < KheGroupMonitorDefectCopyCount(dgm);  i++ )
      {
	d = KheGroupMonitorDefectCopy(dgm, i);
	if( KheMonitorCost(d) > 0 )  switch( KheMonitorTag(d) )
	{
	  case KHE_ORDINARY_DEMAND_MONITOR_TAG:

	    if( KheOrdinaryDemandAugment(ej, (KHE_ORDINARY_DEMAND_MONITOR) d) )
	      return true;
	    break;

	  case KHE_WORKLOAD_DEMAND_MONITOR_TAG:

	    if( KheWorkloadDemandAugment(ej, (KHE_WORKLOAD_DEMAND_MONITOR) d) )
	      return true;
	    break;

	  default:

	    MAssert(false, "KheDemandAugment: "
	      "d has a child which is not a demand monitor");
	    break;
	}
      }
      break;

    default:

      MAssert(false,
	"KheDemandAugment: d is not a demand monitor");
      break;
  }
  return false;
}


/*****************************************************************************/
/*                                                                           */
/*  Submodule "demand defects (node swaps)"                                  */
/*                                                                           */
/*****************************************************************************/

/*****************************************************************************/
/*                                                                           */
/*  void KheAddNodes(ARRAY_KHE_NODE *nodes, KHE_ORDINARY_DEMAND_MONITOR odm) */
/*                                                                           */
/*  Add zero or one nodes to nodes, being the node enclosing odm at the      */
/*  level just below the topmost.                                            */
/*                                                                           */
/*****************************************************************************/

static void KheAddNodes(ARRAY_KHE_NODE *nodes, KHE_ORDINARY_DEMAND_MONITOR odm)
{
  KHE_MEET meet;  KHE_NODE node;
  meet = KheTaskMeet(KheOrdinaryDemandMonitorTask(odm));
  if( KheMeetAsst(meet) != NULL )
  {
    while( KheMeetAsst(KheMeetAsst(meet)) != NULL )
      meet = KheMeetAsst(meet);
    node = KheMeetNode(meet);
    if( node != NULL && !KheNodeVisited(node, 0) &&
	KheNodeParentLayerCount(node) > 0 )
      MArrayAddLast(*nodes, node);
  }
}


/*****************************************************************************/
/*                                                                           */
/*  int KheNodeIncreasingDemandCmp(const void *t1, const void *t2)           */
/*                                                                           */
/*  Comparison function for sorting an array of nodes by increasing          */
/*  demand, and bringing identical nodes together.                           */
/*                                                                           */
/*****************************************************************************/

static int KheNodeIncreasingDemandCmp(const void *t1, const void *t2)
{
  KHE_NODE node1 = * (KHE_NODE *) t1;
  KHE_NODE node2 = * (KHE_NODE *) t2;
  int demand1 = KheNodeDemand(node1);
  int demand2 = KheNodeDemand(node2);
  if( demand1 != demand2 )
    return demand1 - demand2;
  else
    return KheNodeIndex(node1) - KheNodeIndex(node2);
}


/*****************************************************************************/
/*                                                                           */
/*  bool KheNodeSwapAugment(KHE_EJECTOR ej, KHE_NODE node,                   */
/*    KHE_TRANSACTION tn, KHE_TRACE tc)                                      */
/*                                                                           */
/*  Augment function that tries to repair node by swapping it with other     */
/*  nodes lying in its first parent layer, which must exist.  Any two        */
/*  nodes swapped must both have times assigned to all their meets.          */
/*                                                                           */
/*  Parameters tn and tc are scratch transaction and trace objects.          */
/*                                                                           */
/*****************************************************************************/

static bool KheNodeSwapAugment(KHE_EJECTOR ej, KHE_NODE node,
  KHE_TRANSACTION tn, KHE_TRACE tc)
{
  KHE_LAYER parent_layer;  KHE_NODE node2;  int i, node_demand;  bool success;
  if( DEBUG4 )
  {
    fprintf(stderr, "[ KheNodeSwapAugment(ej, ");
    KheNodeDebug(node, 1, -1, stderr);
    fprintf(stderr, ", tc)\n");
  }
  if( !KheNodeVisited(node, 0) && KheNodeParentLayerCount(node) > 0 )
  {
    KheNodeVisit(node);
    if( KheNodeAssignedDuration(node) == KheNodeDuration(node) )
    {
      parent_layer = KheNodeParentLayer(node, 0);
      node_demand = KheNodeDemand(node);
      for( i = 0;  i < KheLayerChildNodeCount(parent_layer);  i++ )
      {
	node2 = KheLayerChildNode(parent_layer, i);
	if( KheNodeAssignedDuration(node2) == KheNodeDuration(node2) )
	{
	  KheTransactionBegin(tn);
	  KheTraceBegin(tc);
	  success = KheNodeMeetSwap(node, node2);
	  if( DEBUG4 )
	  {
	    fprintf(stderr, "  %ssuccessful swap of ", success ? "" : "un");
	    KheNodeDebug(node, 1, -1, stderr);
	    fprintf(stderr, " with ");
	    KheNodeDebug(node2, 1, -1, stderr);
	    fprintf(stderr, "\n");
	  }
	  KheTransactionEnd(tn);
	  KheTraceEnd(tc);
	  if( success &&
	      KheEjectorSuccess(ej, tc, node_demand + KheNodeDemand(node2)) )
	  {
	    if( DEBUG4 )
	    {
	      fprintf(stderr, "] KheNodeSwapAugment returning true (");
	      KheNodeDebug(node, 1, -1, stderr);
	      fprintf(stderr, " with ");
	      KheNodeDebug(node2, 1, -1, stderr);
	      fprintf(stderr, ", soln cost now %.4f)\n",
		KheCostShow(KheSolnCost(KheNodeSoln(node))));
	    }
	    return true;
	  }
	  KheTransactionUndo(tn);
	}
      }
    }
    if( KheEjectorScheduleMayRevisit(KheEjectorCurrSchedule(ej)) )
      KheNodeUnVisit(node);
  }
  if( DEBUG4 )
    fprintf(stderr, "] KheNodeSwapAugment returning false\n");
  return false;
}


/*****************************************************************************/
/*                                                                           */
/*  bool KheNodeOrdinaryDemandAugment(KHE_EJECTOR ej,                        */
/*    KHE_ORDINARY_DEMAND_MONITOR odm)                                       */
/*                                                                           */
/*  Try to fix the ordinary demand problem reported by odm, by swapping      */
/*  its node or one of its competitors' nodes.                               */
/*                                                                           */
/*****************************************************************************/

static bool KheNodeOrdinaryDemandAugment(KHE_EJECTOR ej,
  KHE_ORDINARY_DEMAND_MONITOR odm)
{
  KHE_MONITOR m;  int i;  ARRAY_KHE_NODE nodes;  KHE_NODE node;
  KHE_TRANSACTION tn;  KHE_TRACE tc;
  if( DEBUG4 )
  {
    fprintf(stderr, "[ KheNodeOrdinaryDemandAugment(ej, odm)\n");
    KheMonitorDebug((KHE_MONITOR) odm, 1, 2, stderr);
  }

  /* build a single array of all competitor nodes that may move */
  MArrayInit(nodes);
  KheAddNodes(&nodes, odm);
  for( m = KheMonitorFirstCompetitor((KHE_MONITOR) odm);  m != NULL; 
       m = KheMonitorNextCompetitor((KHE_MONITOR) odm) )
    if( KheMonitorTag(m) == KHE_ORDINARY_DEMAND_MONITOR_TAG )
      KheAddNodes(&nodes, (KHE_ORDINARY_DEMAND_MONITOR) m);

  /* sort the nodes by increasing demand, and remove identical nodes */
  MArraySortUnique(nodes, &KheNodeIncreasingDemandCmp);
  if( DEBUG4 )
  {
    fprintf(stderr, "  competitor nodes: ");
    MArrayForEach(nodes, &node, &i)
    {
      if( i > 0 )
	fprintf(stderr, ", ");
      KheNodeDebug(node, 1, -1, stderr);
    }
    fprintf(stderr, "\n");
  }

  /* try node swaps among these nodes */
  if( MArraySize(nodes) > 0 )
  {
    tn = KheTransactionMake(KheEjectorSoln(ej));
    tc = KheTraceMake(KheEjectorGroupMonitor(ej));
    MArrayForEach(nodes, &node, &i)
      if( KheNodeSwapAugment(ej, node, tn, tc) )
      {
	MArrayFree(nodes);
	KheTraceDelete(tc);
	if( DEBUG4 )
	  fprintf(stderr, "] KheNodeOrdinaryDemandAugment returning true\n");
	return true;
      }
    MArrayFree(nodes);
    KheTransactionDelete(tn);
    KheTraceDelete(tc);
  }
  if( DEBUG4 )
    fprintf(stderr, "] KheNodeOrdinaryDemandAugment returning false\n");
  return false;
}


/*****************************************************************************/
/*                                                                           */
/*  bool KheNodeDemandAugment(KHE_EJECTOR ej, KHE_MONITOR d)                 */
/*                                                                           */
/*  Augment function for repairing demand defects with node swaps.           */
/*                                                                           */
/*****************************************************************************/

bool KheNodeDemandAugment(KHE_EJECTOR ej, KHE_MONITOR d)
{
  KHE_GROUP_MONITOR dgm;  int i;
  MAssert(KheMonitorCost(d) > 0,
    "KheNodeDemandAugment: d does not have non-zero cost");
  switch( KheMonitorTag(d) )
  {
    case KHE_ORDINARY_DEMAND_MONITOR_TAG:

      return KheNodeOrdinaryDemandAugment(ej, (KHE_ORDINARY_DEMAND_MONITOR) d);

    case KHE_WORKLOAD_DEMAND_MONITOR_TAG:

      return false;

    case KHE_GROUP_MONITOR_TAG:

      dgm = (KHE_GROUP_MONITOR) d;
      KheGroupMonitorCopyDefects(dgm);
      for( i = 0;  i < KheGroupMonitorDefectCopyCount(dgm);  i++ )
      {
	d = KheGroupMonitorDefectCopy(dgm, i);
	if( KheMonitorCost(d) > 0 )  switch( KheMonitorTag(d) )
	{
	  case KHE_ORDINARY_DEMAND_MONITOR_TAG:

	    if( KheNodeOrdinaryDemandAugment(ej,
		  (KHE_ORDINARY_DEMAND_MONITOR) d) )
	      return true;
	    break;

	  case KHE_WORKLOAD_DEMAND_MONITOR_TAG:

	    /* doing nothing about these */
	    break;

	  default:

	    MAssert(false, "KheNodeDemandAugment: "
	      "d has a child which is not a demand monitor");
	    break;
	}
      }
      break;

    default:

      MAssert(false,
	"KheNodeDemandAugment: d is not a demand monitor");
      break;
  }
  return false;
}
