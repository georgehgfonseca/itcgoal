
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
/*  FILE:         khe_split_task.c                                           */
/*  DESCRIPTION:  One split task, used by KheSplitAndLink()                  */
/*                                                                           */
/*****************************************************************************/
#include "khe_layer_tree.h"

/*****************************************************************************/
/*                                                                           */
/*   KHE_SPLIT_TASK - one task to be carried out by KheSplitAndLink          */
/*                                                                           */
/*****************************************************************************/

struct khe_split_task_rec {
  KHE_SPLIT_TASK_TAG		tag;		/* task type tag             */
  int				priority;	/* for sorting               */
};


/*****************************************************************************/
/*                                                                           */
/*  KHE_SPLIT_TASK_TAG KheSplitTaskTag(KHE_SPLIT_TASK st)                    */
/*                                                                           */
/*  Return the type tag of st.                                               */
/*                                                                           */
/*****************************************************************************/

KHE_SPLIT_TASK_TAG KheSplitTaskTag(KHE_SPLIT_TASK st)
{
  return st->tag;
}


/*****************************************************************************/
/*                                                                           */
/*  int KheSplitTaskPriority(KHE_SPLIT_TASK st)                              */
/*                                                                           */
/*  Return the priority of st.                                               */
/*                                                                           */
/*****************************************************************************/

int KheSplitTaskPriority(KHE_SPLIT_TASK st)
{
  return st->priority;
}


/*****************************************************************************/
/*                                                                           */
/*  int KheSplitTaskDecreasingPriorityCmp(const void *t1, const void *t2)    */
/*                                                                           */
/*  Comparison function for sorting split tasks by decreasing priority.      */
/*                                                                           */
/*****************************************************************************/

int KheSplitTaskDecreasingPriorityCmp(const void *t1, const void *t2)
{
  KHE_SPLIT_TASK st1 = * (KHE_SPLIT_TASK *) t1;
  KHE_SPLIT_TASK st2 = * (KHE_SPLIT_TASK *) t2;
  if( st2->priority != st1->priority )
    return st2->priority - st1->priority;
  else
    return st2->tag - st1->tag;
}


/*****************************************************************************/
/*                                                                           */
/*  void KheSplitTaskTry(KHE_SPLIT_TASK st)                                  */
/*                                                                           */
/*  Try st.                                                                  */
/*                                                                           */
/*****************************************************************************/

void KheSplitTaskTry(KHE_SPLIT_TASK st, KHE_SPLIT_FOREST sf)
{
  switch( st->tag )
  {
    /* ***
    case KHE_LAYER_SPLIT_TASK_TAG:

      KheLayerSplitTaskTry((KHE_LAYER_SPLIT_TASK) st, sf);
      break;
    *** */

    case KHE_AVOID_CLASHES_SPLIT_TASK_TAG:

      KheAvoidClashesSplitTaskTry((KHE_AVOID_CLASHES_SPLIT_TASK) st, sf);
      break;

    case KHE_PACK_SPLIT_TASK_TAG:

      KhePackSplitTaskTry((KHE_PACK_SPLIT_TASK) st, sf);
      break;

    case KHE_PREASSIGNED_SPLIT_TASK_TAG:

      KhePreassignedSplitTaskTry((KHE_PREASSIGNED_SPLIT_TASK) st, sf);
      break;

    case KHE_ASSIGNED_SPLIT_TASK_TAG:

      KheAssignedSplitTaskTry((KHE_ASSIGNED_SPLIT_TASK) st, sf);
      break;

    case KHE_LINK_SPLIT_TASK_TAG:

      KheLinkSplitTaskTry((KHE_LINK_SPLIT_TASK) st, sf);
      break;

    case KHE_SPLIT_SPLIT_TASK_TAG:

      KheSplitSplitTaskTry((KHE_SPLIT_SPLIT_TASK) st, sf);
      break;

    case KHE_DISTRIBUTE_SPLIT_TASK_TAG:

      KheDistributeSplitTaskTry((KHE_DISTRIBUTE_SPLIT_TASK) st, sf);
      break;

    case KHE_SPREAD_SPLIT_TASK_TAG:

      KheSpreadSplitTaskTry((KHE_SPREAD_SPLIT_TASK) st, sf);
      break;

    case KHE_DOMAIN_SPLIT_TASK_TAG:

      KheDomainSplitTaskTry((KHE_DOMAIN_SPLIT_TASK) st, sf);
      break;

    default:

      MAssert(false, "KheSplitTaskTry given task of unknown type");
  }
}


/*****************************************************************************/
/*                                                                           */
/*  void KheSplitTaskFree(KHE_SPLIT_TASK st)                                 */
/*                                                                           */
/*  Free st.                                                                 */
/*                                                                           */
/*****************************************************************************/

void KheSplitTaskFree(KHE_SPLIT_TASK st)
{
  switch( st->tag )
  {
    /* ***
    case KHE_LAYER_SPLIT_TASK_TAG:

      KheLayerSplitTaskFree((KHE_LAYER_SPLIT_TASK) st);
      break;
    *** */

    case KHE_AVOID_CLASHES_SPLIT_TASK_TAG:

      KheAvoidClashesSplitTaskFree((KHE_AVOID_CLASHES_SPLIT_TASK) st);
      break;

    case KHE_PACK_SPLIT_TASK_TAG:

      KhePackSplitTaskFree((KHE_PACK_SPLIT_TASK) st);
      break;

    case KHE_PREASSIGNED_SPLIT_TASK_TAG:

      KhePreassignedSplitTaskFree((KHE_PREASSIGNED_SPLIT_TASK) st);
      break;

    case KHE_ASSIGNED_SPLIT_TASK_TAG:

      KheAssignedSplitTaskFree((KHE_ASSIGNED_SPLIT_TASK) st);
      break;

    case KHE_LINK_SPLIT_TASK_TAG:

      KheLinkSplitTaskFree((KHE_LINK_SPLIT_TASK) st);
      break;

    case KHE_SPLIT_SPLIT_TASK_TAG:

      KheSplitSplitTaskFree((KHE_SPLIT_SPLIT_TASK) st);
      break;

    case KHE_DISTRIBUTE_SPLIT_TASK_TAG:

      KheDistributeSplitTaskFree((KHE_DISTRIBUTE_SPLIT_TASK) st);
      break;

    case KHE_SPREAD_SPLIT_TASK_TAG:

      KheSpreadSplitTaskFree((KHE_SPREAD_SPLIT_TASK) st);
      break;

    case KHE_DOMAIN_SPLIT_TASK_TAG:

      KheDomainSplitTaskFree((KHE_DOMAIN_SPLIT_TASK) st);
      break;

    default:

      MAssert(false, "KheSplitTaskFree given task of unknown type");
  }
}


/*****************************************************************************/
/*                                                                           */
/*  void KheSplitTaskDebug(KHE_SPLIT_TASK st, int indent, FILE *fp)          */
/*                                                                           */
/*  Debug print of task st onto fp with the given indent.                    */
/*                                                                           */
/*****************************************************************************/

void KheSplitTaskDebug(KHE_SPLIT_TASK st, int indent, FILE *fp)
{
  switch( st->tag )
  {
    /* ***
    case KHE_LAYER_SPLIT_TASK_TAG:

      KheLayerSplitTaskDebug((KHE_LAYER_SPLIT_TASK) st, indent, fp);
      break;
    *** */

    case KHE_AVOID_CLASHES_SPLIT_TASK_TAG:

      KheAvoidClashesSplitTaskDebug((KHE_AVOID_CLASHES_SPLIT_TASK) st,
	indent, fp);
      break;

    case KHE_PACK_SPLIT_TASK_TAG:

      KhePackSplitTaskDebug((KHE_PACK_SPLIT_TASK) st, indent, fp);
      break;

    case KHE_PREASSIGNED_SPLIT_TASK_TAG:

      KhePreassignedSplitTaskDebug((KHE_PREASSIGNED_SPLIT_TASK) st, indent, fp);
      break;

    case KHE_ASSIGNED_SPLIT_TASK_TAG:

      KheAssignedSplitTaskDebug((KHE_ASSIGNED_SPLIT_TASK) st, indent, fp);
      break;

    case KHE_LINK_SPLIT_TASK_TAG:

      KheLinkSplitTaskDebug((KHE_LINK_SPLIT_TASK) st, indent, fp);
      break;

    case KHE_SPLIT_SPLIT_TASK_TAG:

      KheSplitSplitTaskDebug((KHE_SPLIT_SPLIT_TASK) st, indent, fp);
      break;

    case KHE_DISTRIBUTE_SPLIT_TASK_TAG:

      KheDistributeSplitTaskDebug((KHE_DISTRIBUTE_SPLIT_TASK) st, indent, fp);
      break;

    case KHE_SPREAD_SPLIT_TASK_TAG:

      KheSpreadSplitTaskDebug((KHE_SPREAD_SPLIT_TASK) st, indent, fp);
      break;

    case KHE_DOMAIN_SPLIT_TASK_TAG:

      KheDomainSplitTaskDebug((KHE_DOMAIN_SPLIT_TASK) st, indent, fp);
      break;

    default:

      MAssert(false, "KheSplitTaskDebug given task of unknown type");
  }
}
