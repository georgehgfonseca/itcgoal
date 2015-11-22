
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
/*  FILE:         khe_link_split_task.c                                      */
/*  DESCRIPTION:  One link split task, used by KheSplitAndLink()             */
/*                                                                           */
/*****************************************************************************/
#include "khe_layer_tree.h"

#define DEBUG1 0

/*****************************************************************************/
/*                                                                           */
/*   KHE_LINK_SPLIT_TASK - one link task for KheSplitAndLink                 */
/*                                                                           */
/*****************************************************************************/

struct khe_link_split_task_rec {
  KHE_SPLIT_TASK_TAG		tag;		/* task type tag             */
  int				priority;	/* for sorting               */
  KHE_LINK_EVENTS_CONSTRAINT	constraint;	/* the constraint            */
};


/*****************************************************************************/
/*                                                                           */
/*  KHE_LINK_SPLIT_TASK KheLinkSplitTaskMake(KHE_LINK_EVENTS_CONSTRAINT c)   */
/*                                                                           */
/*  Make a link split task with these attributes.                            */
/*                                                                           */
/*****************************************************************************/

KHE_LINK_SPLIT_TASK KheLinkSplitTaskMake(KHE_LINK_EVENTS_CONSTRAINT c)
{
  KHE_LINK_SPLIT_TASK res;
  MMake(res);
  res->tag = KHE_LINK_SPLIT_TASK_TAG;
  res->priority = KheConstraintCombinedWeight((KHE_CONSTRAINT) c);
  res->constraint = c;
  return res;
}


/*****************************************************************************/
/*                                                                           */
/*  void KheLinkSplitTaskFree(KHE_LINK_SPLIT_TASK st)                        */
/*                                                                           */
/*  Free st.                                                                 */
/*                                                                           */
/*****************************************************************************/

void KheLinkSplitTaskFree(KHE_LINK_SPLIT_TASK st)
{
  MFree(st);
}


/*****************************************************************************/
/*                                                                           */
/*  void KheLinkSplitTaskTryEventGroup(KHE_LINK_SPLIT_TASK st,               */
/*    KHE_EVENT_GROUP eg)                                                    */
/*                                                                           */
/*  Try st on eg.                                                            */
/*                                                                           */
/*****************************************************************************/

static void KheLinkSplitTaskTryEventGroup(KHE_LINK_SPLIT_TASK st,
  KHE_EVENT_GROUP eg, KHE_SPLIT_FOREST sf)
{
  int durn, min_duration, max_duration, i, j;  KHE_EVENT ei, ej;
  if( DEBUG1 )
    fprintf(stderr, "[ KheLinkSplitTaskTryEventGroup(st, %s (%d events))\n",
      KheEventGroupId(eg) != NULL ? KheEventGroupId(eg) : "-",
      KheEventGroupEventCount(eg));

  /* find the maximum duration of the events of eg */
  min_duration = INT_MAX;  max_duration = 0;
  for( i = 0;  i < KheEventGroupEventCount(eg);  i++ )
  {
    ei = KheEventGroupEvent(eg, i);
    if( KheEventDuration(ei) > max_duration )
      max_duration = KheEventDuration(ei);
    if( KheEventDuration(ei) < min_duration )
      min_duration = KheEventDuration(ei);
  }

  /* try to link each pair of events of equal duration */
  for( durn = min_duration;  durn <= max_duration;  durn++ )
  {
    for( i = 0;  i < KheEventGroupEventCount(eg);  i++ )
    {
      ei = KheEventGroupEvent(eg, i);
      if( KheEventDuration(ei) == durn )
      {
	for( j = i + 1;  j < KheEventGroupEventCount(eg);  j++ )
	{
	  ej = KheEventGroupEvent(eg, j);
	  if( KheEventDuration(ej) == durn )
            KheSplitForestTryEventMerge(sf, ei, ej);
	}
      }
    }
  }

  /* try to assign all pairs of distinct events regardless of duration */
  if( min_duration != max_duration )
    for( i = 0;  i < KheEventGroupEventCount(eg);  i++ )
    {
      ei = KheEventGroupEvent(eg, i);
      for( j = 0;  j < KheEventGroupEventCount(eg);  j++ )
      {
	ej = KheEventGroupEvent(eg, j);
        if( ei != ej )
          KheSplitForestTryEventAssign(sf, ei, ej);
      }
    }
  if( DEBUG1 )
    fprintf(stderr, "] KheLinkSplitTaskTryEventGroup returning\n");
}


/*****************************************************************************/
/*                                                                           */
/*  void KheLinkSplitTaskTry(KHE_LINK_SPLIT_TASK st, KHE_SPLIT_FOREST sf)    */
/*                                                                           */
/*  Try st on sf.                                                            */
/*                                                                           */
/*****************************************************************************/

void KheLinkSplitTaskTry(KHE_LINK_SPLIT_TASK st, KHE_SPLIT_FOREST sf)
{
  KHE_EVENT_GROUP eg;  int i;
  for( i = 0; i < KheLinkEventsConstraintEventGroupCount(st->constraint); i++ )
  {
    eg = KheLinkEventsConstraintEventGroup(st->constraint, i);
    KheLinkSplitTaskTryEventGroup(st, eg, sf);
  }
}


/*****************************************************************************/
/*                                                                           */
/*  void KheLinkSplitTaskDebug(KHE_LINK_SPLIT_TASK st, int indent, FILE *fp) */
/*                                                                           */
/*  Debug print of task st onto fp with the given indent.                    */
/*                                                                           */
/*****************************************************************************/

void KheLinkSplitTaskDebug(KHE_LINK_SPLIT_TASK st, int indent, FILE *fp)
{
  fprintf(fp, "%*s[ %d Link Split Task %s ]\n", indent, "", st->priority,
    KheConstraintId((KHE_CONSTRAINT) st->constraint));
}
