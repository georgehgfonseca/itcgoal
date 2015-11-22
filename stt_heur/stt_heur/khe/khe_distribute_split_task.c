
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
/*  FILE:         khe_distribute_split_task.c                                */
/*  DESCRIPTION:  One distribute split task, used by KheSplitAndLink()       */
/*                                                                           */
/*****************************************************************************/
#include "khe_layer_tree.h"

#define DEBUG1 0

/*****************************************************************************/
/*                                                                           */
/*   KHE_DISTRIBUTE_SPLIT_TASK - one distribute task for KheSplitAndLink     */
/*                                                                           */
/*****************************************************************************/

struct khe_distribute_split_task_rec {
  KHE_SPLIT_TASK_TAG		tag;		/* task type tag             */
  int				priority;	/* for sorting               */
  KHE_DISTRIBUTE_SPLIT_EVENTS_CONSTRAINT constraint;  /* the constraint      */
};


/*****************************************************************************/
/*                                                                           */
/*  KHE_DISTRIBUTE_SPLIT_TASK KheDistributeSplitTaskMake(                    */
/*    KHE_DISTRIBUTE_SPLIT_EVENTS_CONSTRAINT c)                              */
/*                                                                           */
/*  Make a new distribute split task with these attributes.                  */
/*                                                                           */
/*****************************************************************************/

KHE_DISTRIBUTE_SPLIT_TASK KheDistributeSplitTaskMake(
  KHE_DISTRIBUTE_SPLIT_EVENTS_CONSTRAINT c)
{
  KHE_DISTRIBUTE_SPLIT_TASK res;
  MMake(res);
  res->tag = KHE_DISTRIBUTE_SPLIT_TASK_TAG;
  res->priority = KheConstraintCombinedWeight((KHE_CONSTRAINT) c);
  res->constraint = c;
  return res;
}


/*****************************************************************************/
/*                                                                           */
/*  void KheDistributeSplitTaskFree(KHE_DISTRIBUTE_SPLIT_TASK st)            */
/*                                                                           */
/*  Free st.                                                                 */
/*                                                                           */
/*****************************************************************************/

void KheDistributeSplitTaskFree(KHE_DISTRIBUTE_SPLIT_TASK st)
{
  MFree(st);
}


/*****************************************************************************/
/*                                                                           */
/*  void KheDistributeSplitTaskTryEvent(KHE_DISTRIBUTE_SPLIT_TASK st,        */
/*    KHE_EVENT e, KHE_SPLIT_FOREST sf)                                      */
/*                                                                           */
/*  Try to apply st to e in sf.                                              */
/*                                                                           */
/*****************************************************************************/

static void KheDistributeSplitTaskTryEvent(KHE_DISTRIBUTE_SPLIT_TASK st,
  KHE_EVENT e, KHE_SPLIT_FOREST sf)
{
  int durn, minimum, maximum;  /* KHE_SPLIT_STRONG ss; */
  durn = KheDistributeSplitEventsConstraintDuration(st->constraint);
  minimum = KheDistributeSplitEventsConstraintMinimum(st->constraint);
  maximum = KheDistributeSplitEventsConstraintMaximum(st->constraint);
  if( DEBUG1 )
  {
    fprintf(stderr, "  [ KheDistributeSplitTaskTryEvent(st, %s)\n",
      KheEventId(e) != NULL ? KheEventId(e) : "-");
    fprintf(stderr, "    durn %d, minimum %d, maximum %d\n",
      durn, minimum, maximum);
  }
  KheSplitForestTryDurnAmount(sf, e, durn, minimum, maximum);

  /* ***
  ss = KheSplitInfoSplitStrong(st->split_info, e);
  KheSplitStrongDurnAmount(ss, durn, minimum, maximum);
  *** */
  if( DEBUG1 )
    fprintf(stderr, "  ] KheDistributeSplitTaskTryEvent returning\n");
}


/*****************************************************************************/
/*                                                                           */
/*  void KheDistributeSplitTaskTry(KHE_DISTRIBUTE_SPLIT_TASK st,             */
/*    KHE_SPLIT_FOREST sf)                                                   */
/*                                                                           */
/*  Try st on sf.                                                            */
/*                                                                           */
/*****************************************************************************/

void KheDistributeSplitTaskTry(KHE_DISTRIBUTE_SPLIT_TASK st,
  KHE_SPLIT_FOREST sf)
{
  KHE_EVENT_GROUP eg;  KHE_EVENT e;  int i, j;
  for( i = 0;
       i < KheDistributeSplitEventsConstraintEventCount(st->constraint);  i++ )
  {
    e = KheDistributeSplitEventsConstraintEvent(st->constraint, i);
    KheDistributeSplitTaskTryEvent(st, e, sf);
  }
  for( i = 0;
       i < KheDistributeSplitEventsConstraintEventGroupCount(st->constraint);
       i++ )
  {
    eg = KheDistributeSplitEventsConstraintEventGroup(st->constraint, i);
    for( j = 0;  j < KheEventGroupEventCount(eg);  j++ )
    {
      e = KheEventGroupEvent(eg, j);
      KheDistributeSplitTaskTryEvent(st, e, sf);
    }
  }
}


/*****************************************************************************/
/*                                                                           */
/*  void KheDistributeSplitTaskDebug(KHE_DISTRIBUTE_SPLIT_TASK st,           */
/*    int indent, FILE *fp)                                                  */
/*                                                                           */
/*  Debug print of task st onto fp with the given indent.                    */
/*                                                                           */
/*****************************************************************************/

void KheDistributeSplitTaskDebug(KHE_DISTRIBUTE_SPLIT_TASK st,
  int indent, FILE *fp)
{
  fprintf(fp, "%*s[ %d Distribute Split Task %s ]\n", indent, "", st->priority,
    KheConstraintId((KHE_CONSTRAINT) st->constraint));
}
