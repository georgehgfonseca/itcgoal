
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
/*  FILE:         khe_assigned_split_task.c                                  */
/*  DESCRIPTION:  One assigned split task, used by KheSplitAndLink()         */
/*                                                                           */
/*****************************************************************************/
#include "khe_layer_tree.h"

#define DEBUG1 0

/*****************************************************************************/
/*                                                                           */
/*   KHE_ASSIGNED_SPLIT_TASK - one assigned task for KheSplitAndLink         */
/*                                                                           */
/*****************************************************************************/

struct khe_assigned_split_task_rec {
  KHE_SPLIT_TASK_TAG		tag;		/* task type tag             */
  int				priority;	/* for sorting               */
  KHE_EVENT			child_event;	/* the assigned event        */
  KHE_EVENT			prnt_event;	/* the event assigned to     */
};


/*****************************************************************************/
/*                                                                           */
/*  KHE_ASSIGNED_SPLIT_TASK KheAssignedSplitTaskMake(KHE_EVENT e)            */
/*                                                                           */
/*  Return an assigned split task with these attributes.                     */
/*                                                                           */
/*****************************************************************************/

KHE_ASSIGNED_SPLIT_TASK KheAssignedSplitTaskMake(KHE_EVENT child_event,
  KHE_EVENT prnt_event)
{
  KHE_ASSIGNED_SPLIT_TASK res;
  MMake(res);
  res->tag = KHE_ASSIGNED_SPLIT_TASK_TAG;
  res->priority = INT_MAX - 2;
  res->child_event = child_event;
  res->prnt_event = prnt_event;
  return res;
}


/*****************************************************************************/
/*                                                                           */
/*  void KheAssignedSplitTaskFree(KHE_ASSIGNED_SPLIT_TASK st)                */
/*                                                                           */
/*  Free st.                                                                 */
/*                                                                           */
/*****************************************************************************/

void KheAssignedSplitTaskFree(KHE_ASSIGNED_SPLIT_TASK st)
{
  MFree(st);
}


/*****************************************************************************/
/*                                                                           */
/*  void KheAssignedSplitTaskTry(KHE_ASSIGNED_SPLIT_TASK st,                 */
/*    KHE_SPLIT_FOREST sf)                                                   */
/*                                                                           */
/*  Try st on sf.                                                            */
/*                                                                           */
/*****************************************************************************/

void KheAssignedSplitTaskTry(KHE_ASSIGNED_SPLIT_TASK st, KHE_SPLIT_FOREST sf)
{
  /* int i;  KHE_SOLN_EVENT se;  KHE_SOLN soln; */
  if( DEBUG1 )
    fprintf(stderr, "[ KheAssignedSplitTaskTry(st, %s -> %s)\n",
      KheEventId(st->child_event) != NULL ? KheEventId(st->child_event) : "-",
      KheEventId(st->prnt_event) != NULL ? KheEventId(st->prnt_event) : "-");
  KheSplitForestTryEventAssign(sf, st->child_event, st->prnt_event);
  if( DEBUG1 )
    fprintf(stderr, "] KheAssignedSplitTaskTry returning\n");
}


/*****************************************************************************/
/*                                                                           */
/*  void KheAssignedSplitTaskDebug(KHE_ASSIGNED_SPLIT_TASK st,               */
/*    int indent, FILE *fp)                                                  */
/*                                                                           */
/*  Debug print of task st onto fp with the given indent.                    */
/*                                                                           */
/*****************************************************************************/

void KheAssignedSplitTaskDebug(KHE_ASSIGNED_SPLIT_TASK st,
  int indent, FILE *fp)
{
  fprintf(fp, "%*s[ %d Assigned Split Task %s -> %s ]\n", indent, "",
    st->priority,
    KheEventId(st->child_event) != NULL ? KheEventId(st->child_event) : "-",
    KheEventId(st->prnt_event) != NULL ? KheEventId(st->prnt_event) : "-");
}
