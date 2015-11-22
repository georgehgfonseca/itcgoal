
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
/*  FILE:         khe_preassigned_split_task.c                               */
/*  DESCRIPTION:  One preassigned split task, used by KheSplitAndLink()      */
/*                                                                           */
/*****************************************************************************/
#include "khe_layer_tree.h"

/*****************************************************************************/
/*                                                                           */
/*   KHE_PREASSIGNED_SPLIT_TASK - one preassigned task for KheSplitAndLink   */
/*                                                                           */
/*****************************************************************************/

struct khe_preassigned_split_task_rec {
  KHE_SPLIT_TASK_TAG		tag;		/* task type tag             */
  int				priority;	/* for sorting               */
  KHE_EVENT			event;		/* the preassigned event     */
};


/*****************************************************************************/
/*                                                                           */
/*  KHE_PREASSIGNED_SPLIT_TASK KhePreassignedSplitTaskMake(KHE_EVENT e)      */
/*                                                                           */
/*  Make a preassigned split task with these attributes.                     */
/*                                                                           */
/*****************************************************************************/

KHE_PREASSIGNED_SPLIT_TASK KhePreassignedSplitTaskMake(KHE_EVENT e)
{
  KHE_PREASSIGNED_SPLIT_TASK res;
  MMake(res);
  res->tag = KHE_PREASSIGNED_SPLIT_TASK_TAG;
  res->priority = INT_MAX - 1;
  res->event = e;
  return res;
}


/*****************************************************************************/
/*                                                                           */
/*  void KhePreassignedSplitTaskFree(KHE_PREASSIGNED_SPLIT_TASK st)          */
/*                                                                           */
/*  Free st.                                                                 */
/*                                                                           */
/*****************************************************************************/

void KhePreassignedSplitTaskFree(KHE_PREASSIGNED_SPLIT_TASK st)
{
  MFree(st);
}


/*****************************************************************************/
/*                                                                           */
/*  void KhePreassignedSplitTaskTry(KHE_PREASSIGNED_SPLIT_TASK st)           */
/*                                                                           */
/*  Try st.                                                                  */
/*                                                                           */
/*****************************************************************************/

void KhePreassignedSplitTaskTry(KHE_PREASSIGNED_SPLIT_TASK st,
  KHE_SPLIT_FOREST sf)
{
  /* ***
  KHE_SPLIT_WEAK sw;  bool success;
  sw = KheSplitInfoSplitWeak(st->split_info, st->event);
  KheSplitWeakPreassigned(sw, KheEventPreassignedTime(st->event));
  *** */
  KheSplitForestTryPreassignedTime(sf, st->event,
    KheEventPreassignedTime(st->event));
}


/*****************************************************************************/
/*                                                                           */
/*  void KhePreassignedSplitTaskDebug(KHE_PREASSIGNED_SPLIT_TASK st,         */
/*    int indent, FILE *fp)                                                  */
/*                                                                           */
/*  Debug print of task st onto fp with the given indent.                    */
/*                                                                           */
/*****************************************************************************/

void KhePreassignedSplitTaskDebug(KHE_PREASSIGNED_SPLIT_TASK st,
  int indent, FILE *fp)
{
  fprintf(fp, "%*s[ %d Preassigned Split Task %s ]\n", indent, "",
    st->priority, KheEventId(st->event) != NULL ? KheEventId(st->event) : "-");
}
