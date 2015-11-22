
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
/*  FILE:         khe_pack_split_task.c                                      */
/*  DESCRIPTION:  One pack split task, used by KheSplitAndLink()             */
/*                                                                           */
/*****************************************************************************/
#include "khe_layer_tree.h"

#define DEBUG1 0

/*****************************************************************************/
/*                                                                           */
/*   KHE_PACK_SPLIT_TASK - one pack split task for KheSplitAndLink           */
/*                                                                           */
/*****************************************************************************/

struct khe_pack_split_task_rec {
  KHE_SPLIT_TASK_TAG		tag;		/* task type tag             */
  int				priority;	/* for sorting               */
  KHE_EVENT			event;		/* the already-split event   */
  KHE_PARTITION			partition;	/* partition of sub-events   */
};


/*****************************************************************************/
/*                                                                           */
/*  KHE_PACK_SPLIT_TASK KhePackSplitTaskMake(KHE_EVENT e, KHE_PARTITION p)   */
/*                                                                           */
/*  Make a pack split task with these attributes.  Parameter e may be NULL,  */
/*  in which case the task is to be applied to every event of the instance,  */
/*  because p describes the cycle layer, that all events eventually enter.   */
/*                                                                           */
/*****************************************************************************/

KHE_PACK_SPLIT_TASK KhePackSplitTaskMake(KHE_EVENT e, KHE_PARTITION p)
{
  KHE_PACK_SPLIT_TASK res;
  MMake(res);
  res->tag = KHE_PACK_SPLIT_TASK_TAG;
  res->priority = INT_MAX;
  res->event = e;
  res->partition = p;
  return res;
}


/*****************************************************************************/
/*                                                                           */
/*  void KhePackSplitTaskFree(KHE_PACK_SPLIT_TASK st)                        */
/*                                                                           */
/*  Free st.  Don't free its partition though.                               */
/*                                                                           */
/*****************************************************************************/

void KhePackSplitTaskFree(KHE_PACK_SPLIT_TASK st)
{
  MFree(st);
}


/*****************************************************************************/
/*                                                                           */
/*  void KhePackSplitTaskTryEvent(KHE_PACK_SPLIT_TASK st, KHE_EVENT e,       */
/*    KHE_SPLIT_FOREST sf)                                                   */
/*                                                                           */
/*  Try st on e in sf.                                                       */
/*                                                                           */
/*****************************************************************************/

static void KhePackSplitTaskTryEvent(KHE_PACK_SPLIT_TASK st, KHE_EVENT e,
  KHE_SPLIT_FOREST sf)
{
  if( DEBUG1 )
    fprintf(stderr, "[ KhePackSplitTaskTryEvent(st, %s) %s\n",
      KheEventId(e) != NULL ? KheEventId(e) : "-",
      KhePartitionShow(st->partition));
  KheSplitForestTryPackableInto(sf, e, st->partition);
  if( DEBUG1 )
    fprintf(stderr, "] KhePackSplitTaskTryEvent returning\n");
}


/*****************************************************************************/
/*                                                                           */
/*  void KhePackSplitTaskTry(KHE_PACK_SPLIT_TASK st, KHE_SPLIT_FOREST sf)    */
/*                                                                           */
/*  Try st on sf.                                                            */
/*                                                                           */
/*****************************************************************************/

void KhePackSplitTaskTry(KHE_PACK_SPLIT_TASK st, KHE_SPLIT_FOREST sf)
{
  KHE_INSTANCE ins;  KHE_EVENT e;  int i;
  if( st->event != NULL )
    KhePackSplitTaskTryEvent(st, st->event, sf);
  else
  {
    ins = KheSolnInstance(KheSplitForestSoln(sf));
    for( i = 0;  i < KheInstanceEventCount(ins);  i++ )
    {
      e = KheInstanceEvent(ins, i);
      KhePackSplitTaskTryEvent(st, e, sf);
    }
  }
}


/*****************************************************************************/
/*                                                                           */
/*  void KhePackSplitTaskDebug(KHE_PACK_SPLIT_TASK st, int indent, FILE *fp) */
/*                                                                           */
/*  Debug print of task st onto fp with the given indent.                    */
/*                                                                           */
/*****************************************************************************/

void KhePackSplitTaskDebug(KHE_PACK_SPLIT_TASK st, int indent, FILE *fp)
{
  fprintf(fp, "%*s[ %d Pack Split Task %s %s ]\n", indent, "",
    st->priority, st->event == NULL ? "(all)" :
    KheEventId(st->event) != NULL ? KheEventId(st->event) : "-",
    KhePartitionShow(st->partition));
}
