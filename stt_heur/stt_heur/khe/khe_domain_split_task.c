
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
/*  FILE:         khe_domain_split_task.c                                    */
/*  DESCRIPTION:  One domain split task, used by KheSplitAndLink()           */
/*                                                                           */
/*****************************************************************************/
#include "khe_layer_tree.h"

#define DEBUG1 0

/*****************************************************************************/
/*                                                                           */
/*   KHE_DOMAIN_SPLIT_TASK - one domain task for KheSplitAndLink             */
/*                                                                           */
/*****************************************************************************/

struct khe_domain_split_task_rec {
  KHE_SPLIT_TASK_TAG		tag;		/* task type tag             */
  int				priority;	/* for sorting               */
  KHE_PREFER_TIMES_CONSTRAINT	constraint;	/* the constraint            */
};


/*****************************************************************************/
/*                                                                           */
/*  KHE_DOMAIN_SPLIT_TASK KheDomainSplitTaskMake(                            */
/*    KHE_PREFER_TIMES_CONSTRAINT c)                                         */
/*                                                                           */
/*  Make a domain split task with these attributes.                          */
/*                                                                           */
/*****************************************************************************/

KHE_DOMAIN_SPLIT_TASK KheDomainSplitTaskMake(KHE_PREFER_TIMES_CONSTRAINT c)
{
  KHE_DOMAIN_SPLIT_TASK res;
  MMake(res);
  res->tag = KHE_DOMAIN_SPLIT_TASK_TAG;
  res->priority = KheConstraintCombinedWeight((KHE_CONSTRAINT) c);
  res->constraint = c;
  return res;
}


/*****************************************************************************/
/*                                                                           */
/*  void KheDomainSplitTaskFree(KHE_DOMAIN_SPLIT_TASK st)                    */
/*                                                                           */
/*  Free st.                                                                 */
/*                                                                           */
/*****************************************************************************/

void KheDomainSplitTaskFree(KHE_DOMAIN_SPLIT_TASK st)
{
  MFree(st);
}


/*****************************************************************************/
/*                                                                           */
/*  void KheDomainSplitTaskTryEvent(KHE_DOMAIN_SPLIT_TASK st, KHE_EVENT e)   */
/*                                                                           */
/*  Try st on e.                                                             */
/*                                                                           */
/*****************************************************************************/

void KheDomainSplitTaskTryEvent(KHE_DOMAIN_SPLIT_TASK st, KHE_EVENT e,
  KHE_SPLIT_FOREST sf)
{
  KHE_TIME_GROUP tg;  int durn, d;  /* bool success; */  /* KHE_SPLIT_WEAK sw; */
  if( DEBUG1 )
    fprintf(stderr, "[ KheDomainSplitTaskTryEvent(st, %s)\n",
      KheEventId(e) != NULL ? KheEventId(e) : "-");
  tg = KhePreferTimesConstraintDomain(st->constraint);
  durn = KhePreferTimesConstraintDuration(st->constraint);
  if( durn == KHE_NO_DURATION )
    for( d = 1;  d <= KheEventDuration(e);  d++ )
      KheSplitForestTryEventDomain(sf, e, tg, d);
  else
    KheSplitForestTryEventDomain(sf, e, tg, durn);

  /* ***
  sw = KheSplitInfoSplitWeak(st->split_info, e);
  if( durn == KHE_NO_DURATION )
    for( d = 1;  d <= KheEventDuration(e);  d++ )
      KheSplitWeakDomain(sw, tg, d);
  else
    KheSplitWeakDomain(sw, tg, durn);
  *** */
  if( DEBUG1 )
    fprintf(stderr, "] KheDomainSplitTaskTryEvent returning\n");
}


/*****************************************************************************/
/*                                                                           */
/*  void KheDomainSplitTaskTry(KHE_DOMAIN_SPLIT_TASK st, KHE_SPLIT_FOREST sf)*/
/*                                                                           */
/*  Try st on sf.                                                            */
/*                                                                           */
/*****************************************************************************/

void KheDomainSplitTaskTry(KHE_DOMAIN_SPLIT_TASK st, KHE_SPLIT_FOREST sf)
{
  KHE_EVENT_GROUP eg;  KHE_EVENT e;  int i, j;
  for( i = 0;  i < KhePreferTimesConstraintEventCount(st->constraint);  i++ )
  {
    e = KhePreferTimesConstraintEvent(st->constraint, i);
    KheDomainSplitTaskTryEvent(st, e, sf);
  }
  for( i=0; i < KhePreferTimesConstraintEventGroupCount(st->constraint); i++ )
  {
    eg = KhePreferTimesConstraintEventGroup(st->constraint, i);
    for( j = 0;  j < KheEventGroupEventCount(eg);  j++ )
    {
      e = KheEventGroupEvent(eg, j);
      KheDomainSplitTaskTryEvent(st, e, sf);
    }
  }
}


/*****************************************************************************/
/*                                                                           */
/*  void KheDomainSplitTaskDebug(KHE_DOMAIN_SPLIT_TASK st,                   */
/*    int indent, FILE *fp)                                                  */
/*                                                                           */
/*  Debug print of task st onto fp with the given indent.                    */
/*                                                                           */
/*****************************************************************************/

void KheDomainSplitTaskDebug(KHE_DOMAIN_SPLIT_TASK st, int indent, FILE *fp)
{
  fprintf(fp, "%*s[ %d Domain Split Task %s ]\n", indent, "", st->priority,
    KheConstraintId((KHE_CONSTRAINT) st->constraint));
}
