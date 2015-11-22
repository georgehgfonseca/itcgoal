
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
/*  FILE:         khe_split_split_task.c                                     */
/*  DESCRIPTION:  One split split task, used by KheSplitAndLink()            */
/*                                                                           */
/*****************************************************************************/
#include "khe_layer_tree.h"

#define DEBUG1 0

/*****************************************************************************/
/*                                                                           */
/*   KHE_SPLIT_SPLIT_TASK - one split task for KheSplitAndLink               */
/*                                                                           */
/*****************************************************************************/

struct khe_split_split_task_rec {
  KHE_SPLIT_TASK_TAG		tag;		/* task type tag             */
  int				priority;	/* for sorting               */
  KHE_SPLIT_EVENTS_CONSTRAINT	constraint;	/* the constraint to apply   */
};


/*****************************************************************************/
/*                                                                           */
/*  KHE_SPLIT_SPLIT_TASK KheSplitSplitTaskMake(KHE_SPLIT_EVENTS_CONSTRAINT c)*/
/*                                                                           */
/*  Make a split split task with these attributes.                           */
/*                                                                           */
/*****************************************************************************/

KHE_SPLIT_SPLIT_TASK KheSplitSplitTaskMake(KHE_SPLIT_EVENTS_CONSTRAINT c)
{
  KHE_SPLIT_SPLIT_TASK res;
  MMake(res);
  res->tag = KHE_SPLIT_SPLIT_TASK_TAG;
  res->priority = KheConstraintCombinedWeight((KHE_CONSTRAINT) c);
  res->constraint = c;
  return res;
}


/*****************************************************************************/
/*                                                                           */
/*  void KheSplitSplitTaskFree(KHE_SPLIT_SPLIT_TASK st)                      */
/*                                                                           */
/*  Free st.                                                                 */
/*                                                                           */
/*****************************************************************************/

void KheSplitSplitTaskFree(KHE_SPLIT_SPLIT_TASK st)
{
  MFree(st);
}


/*****************************************************************************/
/*                                                                           */
/*  void KheSplitSplitTaskTryEvent(KHE_SPLIT_SPLIT_TASK st, KHE_EVENT e,     */
/*    KHE_SPLIT_FOREST sf)                                                   */
/*                                                                           */
/*  Try st on e in sf.                                                       */
/*                                                                           */
/*****************************************************************************/

static void KheSplitSplitTaskTryEvent(KHE_SPLIT_SPLIT_TASK st, KHE_EVENT e,
  KHE_SPLIT_FOREST sf)
{
  int d, durn, min_durn, max_durn, min_amount, max_amount;
  /* KHE_SPLIT_STRONG ss; */  /* bool success; */
  durn = KheEventDuration(e);
  min_durn = KheSplitEventsConstraintMinDuration(st->constraint);
  max_durn = KheSplitEventsConstraintMaxDuration(st->constraint);
  min_amount = KheSplitEventsConstraintMinAmount(st->constraint);
  max_amount = KheSplitEventsConstraintMaxAmount(st->constraint);
  if( DEBUG1 )
  {
    fprintf(stderr, "  [ KheSplitSplitTaskTryEvent(st, %s)\n",
      KheEventId(e) != NULL ? KheEventId(e) : "-");
    fprintf(stderr,
      "    min_durn %d, max_durn %d, min_amount %d, max_amount %d\n",
      min_durn, max_durn, min_amount, max_amount);
  }

  /* try everything */
  KheSplitForestTryTotalAmount(sf, e, min_amount, max_amount);
  for( d = 1;  d < min_durn;  d++ )
    KheSplitForestTryDurnAmount(sf, e, d, 0, 0);
  for( d = max_durn + 1;  d <= durn;  d++ )
    KheSplitForestTryDurnAmount(sf, e, d, 0, 0);

  /* ***
  ss = KheSplitInfoSplitStrong(st->split_info, e);
  KheSplitStrongTotalAmount(ss, min_amount, max_amount);
  for( d = 1;  d < min_durn;  d++ )
    KheSplitStrongDurnAmount(ss, d, 0, 0);
  for( d = max_durn + 1;  d <= durn;  d++ )
    KheSplitStrongDurnAmount(ss, d, 0, 0);
  *** */

  if( DEBUG1 )
    fprintf(stderr, "  ] KheSplitSplitTaskTryEvent returning\n");
}


/*****************************************************************************/
/*                                                                           */
/*  void KheSplitSplitTaskTry(KHE_SPLIT_SPLIT_TASK st, KHE_SPLIT_FOREST sf)  */
/*                                                                           */
/*  Try st on each event of its constraint.                                  */
/*                                                                           */
/*****************************************************************************/

void KheSplitSplitTaskTry(KHE_SPLIT_SPLIT_TASK st, KHE_SPLIT_FOREST sf)
{
  KHE_EVENT_GROUP eg;  KHE_EVENT e;  int i, j;
  for( i = 0;  i < KheSplitEventsConstraintEventCount(st->constraint);  i++ )
  {
    e = KheSplitEventsConstraintEvent(st->constraint, i);
    KheSplitSplitTaskTryEvent(st, e, sf);
  }
  for( i=0; i < KheSplitEventsConstraintEventGroupCount(st->constraint); i++ )
  {
    eg = KheSplitEventsConstraintEventGroup(st->constraint, i);
    for( j = 0;  j < KheEventGroupEventCount(eg);  j++ )
    {
      e = KheEventGroupEvent(eg, j);
      KheSplitSplitTaskTryEvent(st, e, sf);
    }
  }
}


/*****************************************************************************/
/*                                                                           */
/* void KheSplitSplitTaskDebug(KHE_SPLIT_SPLIT_TASK st, int indent, FILE *fp)*/
/*                                                                           */
/*  Debug print of task st onto fp with the given indent.                    */
/*                                                                           */
/*****************************************************************************/

void KheSplitSplitTaskDebug(KHE_SPLIT_SPLIT_TASK st, int indent, FILE *fp)
{
  fprintf(fp, "%*s[ %d Split Split Task %s ]\n", indent, "", st->priority,
    KheConstraintId((KHE_CONSTRAINT) st->constraint));
}
