
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
/*  FILE:         khe_spread_split_task.c                                    */
/*  DESCRIPTION:  One spread split task, used by KheSplitAndLink()           */
/*                                                                           */
/*****************************************************************************/
#include "khe_layer_tree.h"

#define DEBUG1 0

/*****************************************************************************/
/*                                                                           */
/*   KHE_SPREAD_SPLIT_TASK - one spread task for KheSplitAndLink             */
/*                                                                           */
/*****************************************************************************/

struct khe_spread_split_task_rec {
  KHE_SPLIT_TASK_TAG		tag;		/* task type tag             */
  int				priority;	/* for sorting               */
  KHE_SPREAD_EVENTS_CONSTRAINT	constraint;	/* the constraint            */
};


/*****************************************************************************/
/*                                                                           */
/*  KHE_SPREAD_SPLIT_TASK KheSpreadSplitTaskMake(                            */
/*    KHE_SPREAD_EVENTS_CONSTRAINT c)                                        */
/*                                                                           */
/*  Make a spread split task with these attributes.                          */
/*                                                                           */
/*****************************************************************************/

KHE_SPREAD_SPLIT_TASK KheSpreadSplitTaskMake(
  KHE_SPREAD_EVENTS_CONSTRAINT c)
{
  KHE_SPREAD_SPLIT_TASK res;
  MMake(res);
  res->tag = KHE_SPREAD_SPLIT_TASK_TAG;
  res->priority = KheConstraintCombinedWeight((KHE_CONSTRAINT) c);
  res->constraint = c;
  return res;
}


/*****************************************************************************/
/*                                                                           */
/*  void KheSpreadSplitTaskFree(KHE_SPREAD_SPLIT_TASK st)                    */
/*                                                                           */
/*  Free st.                                                                 */
/*                                                                           */
/*****************************************************************************/

void KheSpreadSplitTaskFree(KHE_SPREAD_SPLIT_TASK st)
{
  MFree(st);
}


/*****************************************************************************/
/*                                                                           */
/*  void KheSpreadSplitTaskTryEventGroup(KHE_SPREAD_SPLIT_TASK st,           */
/*    int min_amount, int max_amount, KHE_EVENT_GROUP eg)                    */
/*                                                                           */
/*  Try st on eg.                                                            */
/*                                                                           */
/*****************************************************************************/

static void KheSpreadSplitTaskTryEventGroup(KHE_SPREAD_SPLIT_TASK st,
  int min_amount, int max_amount, KHE_EVENT_GROUP eg, KHE_SPLIT_FOREST sf)
{
  if( DEBUG1 )
    fprintf(stderr, "[ KheSpreadSplitTaskTryEventGroup(st, %d-%d, %s)\n",
      min_amount, max_amount,
      KheEventGroupId(eg) != NULL ? KheEventGroupId(eg) : "-");
  KheSplitForestTrySpread(sf, eg, min_amount, max_amount);
  if( DEBUG1 )
    fprintf(stderr, "] KheSpreadSplitTaskTryEventGroup\n");
}


/*****************************************************************************/
/*                                                                           */
/*  void KheSpreadSplitTaskTry(KHE_SPREAD_SPLIT_TASK st, KHE_SPLIT_FOREST sf) */
/*                                                                           */
/*  Try st on sf.                                                            */
/*                                                                           */
/*****************************************************************************/

void KheSpreadSplitTaskTry(KHE_SPREAD_SPLIT_TASK st, KHE_SPLIT_FOREST sf)
{
  KHE_TIME_SPREAD ts;  KHE_LIMITED_TIME_GROUP ltg;  int i;
  int min_amount, max_amount;  KHE_EVENT_GROUP eg;

  /* work out the minimum and maximum number of parts */
  min_amount = max_amount = 0;
  ts = KheSpreadEventsConstraintTimeSpread(st->constraint);
  for( i = 0;  i < KheTimeSpreadLimitedTimeGroupCount(ts);  i++ )
  {
    ltg = KheTimeSpreadLimitedTimeGroup(ts, i);
    min_amount += KheLimitedTimeGroupMinimum(ltg);
    max_amount += KheLimitedTimeGroupMaximum(ltg);
  }

  /* but a minimum only makes sense if the time groups are disjoint */
  if( !KheTimeSpreadTimeGroupsDisjoint(ts) )
    min_amount = 0;

  /* and a maximum only makes sense if they cover the whole cycle */
  if( !KheTimeSpreadCoversWholeCycle(ts) )
    max_amount = INT_MAX;

  /* apply task to each event group, but only if it might change something */
  if( min_amount > 0 || max_amount < INT_MAX )
    for(i=0; i < KheSpreadEventsConstraintEventGroupCount(st->constraint); i++)
    {
      eg = KheSpreadEventsConstraintEventGroup(st->constraint, i);
      KheSpreadSplitTaskTryEventGroup(st, min_amount, max_amount, eg, sf);
    }
}


/*****************************************************************************/
/*                                                                           */
/*  void KheSpreadSplitTaskDebug(KHE_SPREAD_SPLIT_TASK st,                   */
/*    int indent, FILE *fp)                                                  */
/*                                                                           */
/*  Debug print of task st onto fp with the given indent.                    */
/*                                                                           */
/*****************************************************************************/

void KheSpreadSplitTaskDebug(KHE_SPREAD_SPLIT_TASK st, int indent, FILE *fp)
{
  fprintf(fp, "%*s[ %d Spread Split Task %s ]\n", indent, "", st->priority,
    KheConstraintId((KHE_CONSTRAINT) st->constraint));
}
