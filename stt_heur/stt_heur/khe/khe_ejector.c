
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
/*  FILE:         khe_eject.c                                                */
/*  DESCRIPTION:  Ejection chains                                            */
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
#define DEBUG8 0

/*****************************************************************************/
/*                                                                           */
/*  Submodule "ejection chains framework"                                    */
/*                                                                           */
/*****************************************************************************/

/*****************************************************************************/
/*                                                                           */
/*  KHE_EJECTOR_SCHEDULE - a schedule for solving                            */
/*                                                                           */
/*****************************************************************************/

struct khe_ejector_schedule_rec {
  int			max_depth;		/* max depth                 */
  int			max_disruption;		/* max disruption            */
  bool			may_revisit;		/* allow revisiting          */
};

typedef MARRAY(KHE_EJECTOR_SCHEDULE) ARRAY_KHE_EJECTOR_SCHEDULE;


/*****************************************************************************/
/*                                                                           */
/*  KHE_EJECTOR                                                              */
/*                                                                           */
/*****************************************************************************/

typedef MARRAY(KHE_MONITOR) ARRAY_KHE_MONITOR;

struct khe_ejector_rec
{
  /* defined always */
  KHE_SOLN		soln;			/* enclosing solution        */
  ARRAY_KHE_EJECTOR_SCHEDULE schedules;		/* the schedules             */
  ARRAY_KHE_MONITOR	cost_limit_monitors;	/* limit cost of these       */
  ARRAY_INT64		cost_limit_costs;	/* corresponding cost limits */
  KHE_EJECTOR_AUGMENT_FN nongroup_augment[KHE_MONITOR_TAG_COUNT];
  KHE_EJECTOR_AUGMENT_FN group_augment[MAX_GROUP_AUGMENT];

  /* defined only while solving */
  KHE_EJECTOR_SOLVE_TYPE solve_type;		/* type of solve             */
  KHE_GROUP_MONITOR	group_monitor;		/* monitor this              */
  KHE_COST		target_cost;		/* must improve on this      */
  KHE_EJECTOR_SCHEDULE	curr_schedule;		/* current schedule          */
  KHE_TRANSACTION	curr_transaction;	/* current chain             */
  int			curr_depth;		/* current depth             */
  int			curr_disruption;	/* current disruption        */

  /* for recording the best chain so far while solving */
  KHE_TRANSACTION	best_transaction;	/* best chain, if any        */
  int			best_depth;		/* best depth                */
  int			best_disruption;	/* best disruption           */
  KHE_COST		best_cost;		/* best cost                 */
};


/*****************************************************************************/
/*                                                                           */
/*  Submodule "ejector schedules"                                            */
/*                                                                           */
/*****************************************************************************/

/*****************************************************************************/
/*                                                                           */
/*  KHE_EJECTOR_SCHEDULE KheEjectorScheduleMake(int max_depth,               */
/*    int max_disruption, bool may_revisit)                                  */
/*                                                                           */
/*  Make a new ejector schedule with these attributes.                       */
/*                                                                           */
/*****************************************************************************/

static KHE_EJECTOR_SCHEDULE KheEjectorScheduleMake(int max_depth,
  int max_disruption, bool may_revisit)
{
  KHE_EJECTOR_SCHEDULE res;
  MAssert(max_depth > 0, "KheEjectorScheduleMake: max_depth (%d) out of range",
    max_depth);
  MAssert(max_disruption >= 0,
    "KheEjectorScheduleMake: max_disruption (%d) out of range", max_disruption);
  MMake(res);
  res->max_depth = max_depth;
  res->max_disruption = max_disruption;
  res->may_revisit = may_revisit;
  return res;
}


/*****************************************************************************/
/*                                                                           */
/*  void KheEjectorScheduleDelete(KHE_EJECTOR_SCHEDULE ejs)                  */
/*                                                                           */
/*  Free ejector schedule ejs.                                               */
/*                                                                           */
/*****************************************************************************/

static void KheEjectorScheduleDelete(KHE_EJECTOR_SCHEDULE ejs)
{
  MFree(ejs);
}


/*****************************************************************************/
/*                                                                           */
/*  int KheEjectorScheduleMaxDepth(KHE_EJECTOR_SCHEDULE ejs)                 */
/*                                                                           */
/*  Return the max_depth attribute of ejs.                                   */
/*                                                                           */
/*****************************************************************************/

int KheEjectorScheduleMaxDepth(KHE_EJECTOR_SCHEDULE ejs)
{
  return ejs->max_depth;
}


/*****************************************************************************/
/*                                                                           */
/*  int KheEjectorScheduleMaxDisruption(KHE_EJECTOR_SCHEDULE ejs)            */
/*                                                                           */
/*  Return the max_disruption attribute of ejs.                              */
/*                                                                           */
/*****************************************************************************/

int KheEjectorScheduleMaxDisruption(KHE_EJECTOR_SCHEDULE ejs)
{
  return ejs->max_disruption;
}


/*****************************************************************************/
/*                                                                           */
/*  bool KheEjectorScheduleMayRevisit(KHE_EJECTOR_SCHEDULE ejs)              */
/*                                                                           */
/*  Return the may_revisit attribute of ejs.                                 */
/*                                                                           */
/*****************************************************************************/

bool KheEjectorScheduleMayRevisit(KHE_EJECTOR_SCHEDULE ejs)
{
  return ejs->may_revisit;
}


/*****************************************************************************/
/*                                                                           */
/*  Submodule "ejectors - construction and query"                            */
/*                                                                           */
/*****************************************************************************/

/*****************************************************************************/
/*                                                                           */
/*  KHE_EJECTOR KheEjectorMake(KHE_SOLN soln)                                */
/*                                                                           */
/*  Make a new ejector with these attributes.                                */
/*                                                                           */
/*****************************************************************************/

KHE_EJECTOR KheEjectorMake(KHE_SOLN soln)
{
  KHE_EJECTOR res;  int i;

  /* defined always */
  MMake(res);
  res->soln = soln;
  MArrayInit(res->schedules);
  MArrayInit(res->cost_limit_monitors);
  MArrayInit(res->cost_limit_costs);
  for( i = 0;  i < KHE_MONITOR_TAG_COUNT;  i++ )
    res->nongroup_augment[i] = NULL;
  for( i = 0;  i < MAX_GROUP_AUGMENT;  i++ )
    res->group_augment[i] = NULL;

  /* defined only while solving, but best to initialize */
  res->solve_type = KHE_EJECTOR_NONE;
  res->group_monitor = NULL;
  res->target_cost = -1;
  res->curr_schedule = NULL;
  res->curr_transaction = KheTransactionMake(soln);
  res->curr_depth = -1;
  res->curr_disruption = -1;
  res->best_transaction = KheTransactionMake(soln);
  res->best_depth = -1;
  res->best_disruption = -1;
  res->best_cost = -1;
  return res;
}


/*****************************************************************************/
/*                                                                           */
/*  KHE_SOLN KheEjectorSoln(KHE_EJECTOR ej)                                  */
/*                                                                           */
/*  Return the soln of ej, obtained via its group monitor.                   */
/*                                                                           */
/*****************************************************************************/

KHE_SOLN KheEjectorSoln(KHE_EJECTOR ej)
{
  return KheMonitorSoln((KHE_MONITOR) ej->group_monitor);
}


/*****************************************************************************/
/*                                                                           */
/*  void KheEjectorDelete(KHE_EJECTOR ej)                                    */
/*                                                                           */
/*  Delete ej.                                                               */
/*                                                                           */
/*****************************************************************************/

void KheEjectorDelete(KHE_EJECTOR ej)
{
  while( MArraySize(ej->schedules) > 0 )
    KheEjectorScheduleDelete(MArrayRemoveLast(ej->schedules));
  MArrayFree(ej->schedules);
  MArrayFree(ej->cost_limit_monitors);
  MArrayFree(ej->cost_limit_costs);
  KheTransactionDelete(ej->curr_transaction);
  KheTransactionDelete(ej->best_transaction);
  MFree(ej);
}


/*****************************************************************************/
/*                                                                           */
/*  void KheEjectorAddSchedule(KHE_EJECTOR ej, int max_depth,                */
/*    int max_disruption, bool may_revisit)                                  */
/*                                                                           */
/*  Add a schedule with these attributes to ej.                              */
/*                                                                           */
/*****************************************************************************/

void KheEjectorAddSchedule(KHE_EJECTOR ej, int max_depth,
  int max_disruption, bool may_revisit)
{
  MArrayAddLast(ej->schedules,
    KheEjectorScheduleMake(max_depth, max_disruption, may_revisit));
}


/*****************************************************************************/
/*                                                                           */
/*  int KheEjectorScheduleCount(KHE_EJECTOR ej)                              */
/*                                                                           */
/*  Return the number of schedules of ej.                                    */
/*                                                                           */
/*****************************************************************************/

int KheEjectorScheduleCount(KHE_EJECTOR ej)
{
  return MArraySize(ej->schedules);
}


/*****************************************************************************/
/*                                                                           */
/*  KHE_EJECTOR_SCHEDULE KheEjectorSchedule(KHE_EJECTOR ej, int i)           */
/*                                                                           */
/*  Return the i'th schedule of ej.                                          */
/*                                                                           */
/*****************************************************************************/

KHE_EJECTOR_SCHEDULE KheEjectorSchedule(KHE_EJECTOR ej, int i)
{
  return MArrayGet(ej->schedules, i);
}


/*****************************************************************************/
/*                                                                           */
/*  void KheEjectorAddAugment(KHE_EJECTOR ej,                                */
/*    KHE_MONITOR_TAG tag, KHE_EJECTOR_AUGMENT_FN augment_fn)                */
/*                                                                           */
/*  Set the augment function for non-group monitors with this tag.           */
/*                                                                           */
/*****************************************************************************/

void KheEjectorAddAugment(KHE_EJECTOR ej,
  KHE_MONITOR_TAG tag, KHE_EJECTOR_AUGMENT_FN augment_fn)
{
  MAssert(tag >= 0 && tag < KHE_MONITOR_TAG_COUNT,
    "KheEjectorAddAugment: tag (%d) out of range", tag);
  MAssert(tag != KHE_GROUP_MONITOR_TAG,
    "KheEjectorAddAugment: tag is KHE_GROUP_MONITOR_TAG");
  ej->nongroup_augment[tag] = augment_fn;
}


/*****************************************************************************/
/*                                                                           */
/*  void KheEjectorAddGroupAugment(KHE_EJECTOR ej,                           */
/*    int sub_tag, KHE_EJECTOR_AUGMENT_FN augment_fn)                        */
/*                                                                           */
/*  Set the augment function for group monitors with this sub-tag.           */
/*                                                                           */
/*****************************************************************************/

void KheEjectorAddGroupAugment(KHE_EJECTOR ej,
  int sub_tag, KHE_EJECTOR_AUGMENT_FN augment_fn)
{
  MAssert(sub_tag >= 0 && sub_tag < MAX_GROUP_AUGMENT,
    "KheEjectorAddGroupAugment: sub_tag (%d) out of range", sub_tag);
  ej->group_augment[sub_tag] = augment_fn;
}


/*****************************************************************************/
/*                                                                           */
/*  void KheEjectorAddMonitorCostLimit(KHE_EJECTOR ej, KHE_MONITOR m,        */
/*    KHE_COST cost_limit)                                                       */
/*                                                                           */
/*  Add a cost limit to ej.                                                  */
/*                                                                           */
/*****************************************************************************/

void KheEjectorAddMonitorCostLimit(KHE_EJECTOR ej, KHE_MONITOR m,
  KHE_COST cost_limit)
{
  if( DEBUG3 )
  {
    fprintf(stderr, "[ KheEjectorAddMonitorCostLimit(ej, m, %.4f)\n",
      KheCostShow(cost_limit));
    if( KheMonitorTag(m) == KHE_GROUP_MONITOR_TAG )
      KheGroupMonitorDefectDebug((KHE_GROUP_MONITOR) m, 2, 2, stderr);
    else
      KheMonitorDebug(m, 2, 2, stderr);
    fprintf(stderr, "]\n");
  }
  MArrayAddLast(ej->cost_limit_monitors, m);
  MArrayAddLast(ej->cost_limit_costs, cost_limit);
}


/*****************************************************************************/
/*                                                                           */
/*  int KheEjectorMonitorCostLimitCount(KHE_EJECTOR ej)                      */
/*                                                                           */
/*  Return the number of cost limits in ej.                                  */
/*                                                                           */
/*****************************************************************************/

int KheEjectorMonitorCostLimitCount(KHE_EJECTOR ej)
{
  return MArraySize(ej->cost_limit_monitors);
}


/*****************************************************************************/
/*                                                                           */
/*  void KheEjectorMonitorCostLimit(KHE_EJECTOR ej, int i,                   */
/*    KHE_MONITOR *m, KHE_COST *cost_limit)                                      */
/*                                                                           */
/*  Return the i'th cost limit of ej.                                        */
/*                                                                           */
/*****************************************************************************/

void KheEjectorMonitorCostLimit(KHE_EJECTOR ej, int i,
  KHE_MONITOR *m, KHE_COST *cost_limit)
{
  *m = MArrayGet(ej->cost_limit_monitors, i);
  *cost_limit = MArrayGet(ej->cost_limit_costs, i);
}


/*****************************************************************************/
/*                                                                           */
/*  Ejectors - solving                                                       */
/*                                                                           */
/*****************************************************************************/

/*****************************************************************************/
/*                                                                           */
/*  bool KheEjectorPolyAugment(KHE_EJECTOR ej, KHE_MONITOR d)                */
/*                                                                           */
/*  Make a polymorphic augment.                                              */
/*                                                                           */
/*****************************************************************************/

static bool KheEjectorPolyAugment(KHE_EJECTOR ej, KHE_MONITOR d)
{
  KHE_EJECTOR_AUGMENT_FN augment_fn;  KHE_GROUP_MONITOR gm2;  bool res;
  int sub_tag;
  if( DEBUG4 )
    fprintf(stderr, "%*s[ Augment (soln %.4f, c %.4f, depth %d%s) %s %.4f\n",
      4*KheEjectorCurrDepth(ej) - 2, "",
      KheCostShow(KheSolnCost(KheMonitorSoln(d))),
      KheCostShow(ej->target_cost), KheEjectorCurrDepth(ej),
      KheEjectorCurrDepth(ej) == ej->curr_schedule->max_depth - 1 ?
      " (limit)" : "", KheMonitorLabel(d), KheCostShow(KheMonitorCost(d)));
  MAssert(KheMonitorCost(d) > 0, "KheEjectorPolyAugment internal error");
  if( KheMonitorTag(d) == KHE_GROUP_MONITOR_TAG )
  {
    gm2 = (KHE_GROUP_MONITOR) d;
    sub_tag = KheGroupMonitorSubTag(gm2);
    MAssert(sub_tag >= 0 && sub_tag < MAX_GROUP_AUGMENT,
      "KheEjectorPolyAugment: sub_tag (%d) out of range", sub_tag);
    augment_fn = ej->group_augment[sub_tag];
  }
  else
    augment_fn = ej->nongroup_augment[KheMonitorTag(d)];
  res = (augment_fn == NULL ? false : augment_fn(ej, d));
  if( DEBUG4 )
    fprintf(stderr, "%*s] Augment returning %s\n",
      4 * KheEjectorCurrDepth(ej) - 2, "", res ? "true" : "false");
  return res;
}


/*****************************************************************************/
/*                                                                           */
/*  bool KheEjectorAugment(KHE_EJECTOR ej, KHE_MONITOR d)                    */
/*                                                                           */
/*  Augment from d.                                                          */
/*                                                                           */
/*  Implementation note.  It is possible for the cost of d to change to      */
/*  0 even when the algorithm does not succeed, when d is a resource         */
/*  demand monitor.  Hence the test at the start of each iteration.          */
/*                                                                           */
/*****************************************************************************/

static bool KheEjectorAugment(KHE_EJECTOR ej, KHE_MONITOR d)
{
  KHE_COST save_cost;  int i;
  MAssert(KheMonitorCost(d) > 0, "KheEjectorAugment internal error 1");

  /* initialize solve fields of ej */
  ej->target_cost = KheSolnCost(ej->soln);
  ej->curr_depth = 0;
  ej->curr_disruption = 0;
  ej->best_depth = INT_MAX;
  ej->best_disruption = INT_MAX;
  ej->best_cost = KheCostMax;
  if( ej->solve_type != KHE_EJECTOR_FIRST_SUCCESS )
    KheTransactionBegin(ej->curr_transaction);

  /* run each schedule */
  save_cost = KheSolnCost(ej->soln);
  for( i = 0;  KheMonitorCost(d) > 0 && i < MArraySize(ej->schedules);  i++ )
  {
    ej->curr_schedule = MArrayGet(ej->schedules, i);
    KheSolnNewVisit(ej->soln);
    MAssert(ej->curr_depth == 0, "KheEjectorAugment internal error 2");
    MAssert(ej->curr_disruption == 0, "KheEjectorAugment internal error 3");
    if( KheEjectorPolyAugment(ej, d) )
      return true;
    if( DEBUG1 && KheSolnCost(ej->soln) != save_cost )
      KheSolnCostByTypeDebug(ej->soln, 2, 2, stderr);
    MAssert(KheSolnCost(ej->soln) == save_cost,
      "KheEjectorAugment internal error 4: %.4f != %.4f",
      KheCostShow(KheSolnCost(ej->soln)), KheCostShow(save_cost));
  }
  if( ej->solve_type != KHE_EJECTOR_FIRST_SUCCESS )
  {
    KheTransactionEnd(ej->curr_transaction);
    if( ej->best_cost < KheCostMax )
    {
      KheTransactionRedo(ej->best_transaction);
      MAssert(KheSolnCost(ej->soln) == ej->best_cost,
	"KheEjectorAugment internal error 5");
      return true;
    }
    else
      return false;
  }
  else
    return false;
}


/*****************************************************************************/
/*                                                                           */
/*  void KheEjectorSolve(KHE_EJECTOR ej, KHE_EJECTOR_SOLVE_TYPE solve_type,  */
/*    KHE_GROUP_MONITOR gm)                                                  */
/*                                                                           */
/*  Run a solve using ej.                                                    */
/*                                                                           */
/*****************************************************************************/

void KheEjectorSolve(KHE_EJECTOR ej, KHE_EJECTOR_SOLVE_TYPE solve_type,
  KHE_GROUP_MONITOR gm)
{
  int i;  KHE_MONITOR d;  bool progressing;
  ej->solve_type = solve_type;
  ej->group_monitor = gm;
  KheGroupMonitorDefectSort(gm);
  if( DEBUG3 )
  {
    fprintf(stderr, "[ KheEjectorSolve(ej)\n");
    for( i = 0;  i < KheGroupMonitorDefectCount(gm);  i++ )
    {
      d = KheGroupMonitorDefect(gm, i);
      fprintf(stderr, "  init ");
      KheMonitorDebug(d, 2, 2, stderr);
    }
  }
  do
  {
    progressing = false;
    KheGroupMonitorCopyDefects(gm);
    for( i = 0;  i < KheGroupMonitorDefectCopyCount(gm);  i++ )
    {
      d = KheGroupMonitorDefectCopy(gm, i);
      if( KheMonitorCost(d) > 0 )
      {
	if( DEBUG3 )
	{
	  fprintf(stderr, "  augment ");
	  KheMonitorDebug(d, 2, 2, stderr);
	}
	if( KheEjectorAugment(ej, d) )
	{
	  progressing = true;
	  if( DEBUG3 )
	    fprintf(stderr, "  after success, soln cost is %.4f\n",
	      KheCostShow(KheSolnCost(ej->soln)));
	}
      }
    }
  } while( progressing );
  if( DEBUG3 )
  {
    for( i = 0;  i < KheGroupMonitorDefectCount(gm);  i++ )
    {
      d = KheGroupMonitorDefect(gm, i);
      fprintf(stderr, "  final ");
      KheMonitorDebug(d, 2, 2, stderr);
    }
    fprintf(stderr, "] KheEjectorSolve returning\n");
  }
}

/* *** old version from before defect copying
void KheEjectorSolve(KHE_EJECTOR ej, KHE_EJECTOR_SOLVE_TYPE solve_type,
  KHE_GROUP_MONITOR gm)
{
  int i, fail_count;  KHE_MONITOR d;
  ej->solve_type = solve_type;
  ej->group_monitor = gm;
  KheGroupMonitorDefectSort(gm);
  if( DEBUG3 )
  {
    fprintf(stderr, "[ KheEjectorSolve(ej)\n");
    for( i = 0;  i < KheGroupMonitorDefectCount(gm);  i++ )
    {
      d = KheGroupMonitorDefect(gm, i);
      fprintf(stderr, "  init ");
      KheMonitorDebug(d, 2, 2, stderr);
    }
  }
  i = fail_count = 0;
  while( fail_count < KheGroupMonitorDefectCount(gm) )
  {
    i = (i + 1) % KheGroupMonitorDefectCount(gm);
    d = KheGroupMonitorDefect(gm, i);
    if( DEBUG3 )
    {
      fprintf(stderr, "  augment ");
      KheMonitorDebug(d, 2, 2, stderr);
    }
    if( KheEjectorAugment(ej, d) )
    {
      fail_count = 0;
      if( DEBUG3 )
        fprintf(stderr, "  after success, soln cost is %.4f\n",
	  KheCostShow(KheSolnCost(ej->soln)));
    }
    else
      fail_count++;
  }
  if( DEBUG3 )
  {
    for( i = 0;  i < KheGroupMonitorDefectCount(gm);  i++ )
    {
      d = KheGroupMonitorDefect(gm, i);
      fprintf(stderr, "  final ");
      KheMonitorDebug(d, 2, 2, stderr);
    }
    fprintf(stderr, "] KheEjectorSolve returning\n");
  }
}
*** */


/*****************************************************************************/
/*                                                                           */
/*  KHE_EJECTOR_SOLVE_TYPE KheEjectorSolveType(KHE_EJECTOR ej)               */
/*                                                                           */
/*  Return the type of the current solve.                                    */
/*                                                                           */
/*****************************************************************************/

KHE_EJECTOR_SOLVE_TYPE KheEjectorSolveType(KHE_EJECTOR ej)
{
  return ej->solve_type;
}


/*****************************************************************************/
/*                                                                           */
/*  KHE_GROUP_MONITOR KheEjectorGroupMonitor(KHE_EJECTOR ej)                 */
/*                                                                           */
/*  Return the group monitor whose defects ej is repairing.                  */
/*                                                                           */
/*****************************************************************************/

KHE_GROUP_MONITOR KheEjectorGroupMonitor(KHE_EJECTOR ej)
{
  return ej->group_monitor;
}


/*****************************************************************************/
/*                                                                           */
/*  KHE_COST KheEjectorTargetCost(KHE_EJECTOR ej)                                */
/*                                                                           */
/*  Return the target cost of ej.                                            */
/*                                                                           */
/*****************************************************************************/

KHE_COST KheEjectorTargetCost(KHE_EJECTOR ej)
{
  return ej->target_cost;
}


/*****************************************************************************/
/*                                                                           */
/*  KHE_EJECTOR_SCHEDULE KheEjectorCurrSchedule(KHE_EJECTOR ej)              */
/*                                                                           */
/*  Return the current schedule of ej.                                       */
/*                                                                           */
/*****************************************************************************/

KHE_EJECTOR_SCHEDULE KheEjectorCurrSchedule(KHE_EJECTOR ej)
{
  return ej->curr_schedule;
}


/*****************************************************************************/
/*                                                                           */
/*  int KheEjectorCurrDepth(KHE_EJECTOR ej)                                  */
/*                                                                           */
/*  Return the depth of the current chain.                                   */
/*                                                                           */
/*****************************************************************************/

int KheEjectorCurrDepth(KHE_EJECTOR ej)
{
  return ej->curr_depth;
}


/*****************************************************************************/
/*                                                                           */
/*  int KheEjectorCurrDisruption(KHE_EJECTOR ej)                             */
/*                                                                           */
/*  Return the disruption of the current chain.                              */
/*                                                                           */
/*****************************************************************************/

int KheEjectorCurrDisruption(KHE_EJECTOR ej)
{
  return ej->curr_disruption;
}


/*****************************************************************************/
/*                                                                           */
/*  bool KheCostAndGroupMonitorLimitsSatisfied(KHE_EJECTOR ej)               */
/*                                                                           */
/*  Return true if the current solution cost is below the target and         */
/*  all the group monitor limits are satisfied.                              */
/*                                                                           */
/*****************************************************************************/

static bool KheCostAndGroupMonitorLimitsSatisfied(KHE_EJECTOR ej)
{
  KHE_MONITOR m;  int i;
  if( KheSolnCost(ej->soln) >= ej->target_cost )
    return false;
  MArrayForEach(ej->cost_limit_monitors, &m, &i)
    if( KheMonitorCost(m) > MArrayGet(ej->cost_limit_costs, i) )
      return false;
  return true;
}


/*****************************************************************************/
/*                                                                           */
/*  bool KheEjectorSuccess(KHE_EJECTOR ej, KHE_TRACE tc, int disruption)     */
/*                                                                           */
/*  Do the success testing part of the ejection chain algorithm, including   */
/*  the recursive call.                                                      */
/*                                                                           */
/*****************************************************************************/

bool KheEjectorSuccess(KHE_EJECTOR ej, KHE_TRACE tc, int disruption)
{
  KHE_MONITOR m;  int i;

  /* depth and disruption have now increased */
  ej->curr_depth++;
  ej->curr_disruption += disruption;

  /* if the current solution is successful, handle it and return */
  if( KheCostAndGroupMonitorLimitsSatisfied(ej) )
  {
    if( DEBUG5 )
      fprintf(stderr, "    Augment %s %.4f (depth %d, target %.4f)\n",
	"succeeding with soln cost", KheCostShow(KheSolnCost(ej->soln)),
	KheEjectorCurrDepth(ej), KheCostShow(ej->target_cost));
    if( DEBUG4 )
      fprintf(stderr, "%*ssucceeding with soln cost %.4f (target was %.4f)\n",
	4 * KheEjectorCurrDepth(ej) + 2, "",
	KheCostShow(KheSolnCost(ej->soln)), KheCostShow(ej->target_cost));
    switch( ej->solve_type )
    {
      case KHE_EJECTOR_NONE:

	MAssert(false, "KheEjectorSuccess called while not solving");
	break;

      case KHE_EJECTOR_FIRST_SUCCESS:

	/* ordinary ejection chain, return true immediately */
	ej->curr_depth--;
	ej->curr_disruption -= disruption;
	return true;

      case KHE_EJECTOR_MIN_COST:

	/* minimize cost, leave best_disruption alone at INT_MAX */
	if( KheSolnCost(ej->soln) < ej->best_cost )
	{
	  KheTransactionCopy(ej->curr_transaction, ej->best_transaction);
	  ej->best_cost = KheSolnCost(ej->soln);
	  MAssert(ej->best_disruption == INT_MAX,
	    "KheEjectorSuccess internal error");
	}
	ej->curr_depth--;
	ej->curr_disruption -= disruption;
	return false;

      case KHE_EJECTOR_MIN_DISRUPTION_THEN_COST:

	/* minimize (disruption, cost) */
	if( ej->curr_disruption < ej->best_disruption ||
	    (ej->curr_disruption == ej->best_disruption &&
	     KheSolnCost(ej->soln) < ej->best_cost) )
	{
	  /* new best, record it and keep going */
	  if( DEBUG2 )
	  {
	    fprintf(stderr, "  KheEjectorSuccess new best (%d, %.4f):\n",
	      ej->curr_disruption, KheCostShow(KheSolnCost(ej->soln)));
	    KheTransactionDebug(ej->curr_transaction, 2, 2, stderr);
	  }
	  KheTransactionCopy(ej->curr_transaction, ej->best_transaction);
	  ej->best_disruption = ej->curr_disruption;
	  ej->best_cost = KheSolnCost(ej->soln);
	}
	ej->curr_depth--;
	ej->curr_disruption -= disruption;
	return false;

      default:

	MAssert(false, "KheEjectorSuccess internal error (solve type)");
	break;
    }
  }

  /* if we are not up against a depth or disruption limit, recurse */
  if( ej->curr_depth < ej->curr_schedule->max_depth &&
      ej->curr_disruption <= ej->curr_schedule->max_disruption &&
      ej->curr_disruption <= ej->best_disruption )
  {
    for( i = 0;  i < KheTraceMonitorCount(tc);  i++ )
    {
      m = KheTraceMonitor(tc, i);
      if( KheMonitorCost(m) > KheTraceMonitorInitCost(tc, i) )
      {
	if( DEBUG8 )
	{
	  fprintf(stderr, "%*strace ", 4 * KheEjectorCurrDepth(ej) + 2, "");
	  KheMonitorDebug(m, 1, 0, stderr);
	}
	if( KheSolnCost(ej->soln) - KheMonitorCost(m) < ej->target_cost &&
	    KheEjectorPolyAugment(ej, m) )
	{
	  ej->curr_depth--;
	  ej->curr_disruption -= disruption;
	  return true;
	}
      }
    }
  }
  if( DEBUG8 )
    fprintf(stderr, "%*sfailing %son cost %.4f not less than %.4f\n",
      4*KheEjectorCurrDepth(ej) + 2, "",
      ej->curr_depth >= ej->curr_schedule->max_depth ? "(max depth)" : "",
      KheCostShow(KheSolnCost(ej->soln)), KheCostShow(ej->target_cost));
  /* ***
  if( DEBUG4 )
    fprintf(stderr, "%*sfailing with soln cost %.4f\n",
      4 * KheEjectorCurrDepth(ej) + 2, "", KheCostShow(KheSolnCost(ej->soln)));
  *** */
  ej->curr_depth--;
  ej->curr_disruption -= disruption;
  return false;
}
