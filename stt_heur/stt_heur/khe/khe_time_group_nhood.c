
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
/*  FILE:         khe_time_group_nhood.c                                     */
/*  DESCRIPTION:  A sequence of time groups neighbouring some time group.    */
/*                                                                           */
/*****************************************************************************/
#include "khe_interns.h"

#define DEBUG1 0


/*****************************************************************************/
/*                                                                           */
/*  KHE_TIME_GROUP_NHOOD                                                     */
/*                                                                           */
/*****************************************************************************/

struct khe_time_group_nhood_rec {
  ARRAY_KHE_TIME_GROUP		time_groups;		/* the neighbours    */
};


/*****************************************************************************/
/*                                                                           */
/*  KHE_TIME_GROUP_NHOOD KheTimeGroupNhoodMake(KHE_TIME_GROUP tg,            */
/*    int *index_in_nhood)                                                   */
/*                                                                           */
/*  Make a time group for, and including, tg.  Return it and also set        */
/*  *index_in_nhood to tg's index in the neighbourhood.                      */
/*                                                                           */
/*****************************************************************************/

KHE_TIME_GROUP_NHOOD KheTimeGroupNhoodMake(KHE_TIME_GROUP tg,
  int *index_in_nhood)
{
  KHE_TIME_GROUP_NHOOD res;  int a, b, count, max_count, delta;
  LSET tg_lset, shifted_lset;  KHE_TIME_GROUP shifted_tg;  KHE_INSTANCE ins;
  KHE_TIME ta, tb;
  if( DEBUG1 )
  {
    fprintf(stderr, "[ KheTimeGroupNhoodMake(");
    KheTimeGroupDebug(tg, 1, -1, stderr);
    fprintf(stderr, ")\n");
  }
  MMake(res);
  MArrayInit(res->time_groups);
  count = KheTimeGroupTimeCount(tg);
  ins = KheTimeGroupInstance(tg);
  max_count = KheInstanceTimeCount(ins);
  if( count == 0 )
  {
    /* empty time group can make do with an almost empty neighbourhood */
    *index_in_nhood = 0;
    MArrayAddLast(res->time_groups, tg);
    if( DEBUG1 )
      fprintf(stderr, "  empty case\n");
  }
  else
  {
    /* for each delta that gives a non-empty neighbour, add the neighbour */
    ta = KheTimeGroupTime(tg, count - 1);
    a = KheTimeIndex(ta);
    tb = KheTimeGroupTime(tg, 0);
    b = max_count - KheTimeIndex(tb) - 1;
    tg_lset = KheTimeGroupTimeSet(tg);
    if( DEBUG1 )
      fprintf(stderr, "  %s: a = %d;  %s: b = %d\n",
	KheTimeId(ta) == NULL ? "-" : KheTimeId(ta), a,
	KheTimeId(tb) == NULL ? "-" : KheTimeId(tb), b);
    for( delta = -a;  delta <= b;  delta++ )
    {
      /* find the shifted time group and add it to res->time_groups */
      if( DEBUG1 )
	fprintf(stderr, "  delta = %d:\n", delta);
      if( delta == 0 )
      {
	/* time group tg itself goes here */
	*index_in_nhood = MArraySize(res->time_groups);
	shifted_tg = tg;
      }
      else
      {
	/* have to make the lset and shifted_tg */
	shifted_lset = LSetNew();
	LSetShift(tg_lset, &shifted_lset, delta, max_count);
	shifted_tg = KheTimeGroupMakeInternal(KHE_TIME_GROUP_TYPE_NEIGHBOUR,
	  ins, KHE_TIME_GROUP_KIND_ORDINARY, NULL, NULL, shifted_lset);
	KheTimeGroupFinalize(shifted_tg, res, MArraySize(res->time_groups));
      }
      MArrayAddLast(res->time_groups, shifted_tg);
    }
  }
  if( DEBUG1 )
    fprintf(stderr, "] KheTimeGroupNhoodMake returning.\n");
  return res;
}


/*****************************************************************************/
/*                                                                           */
/*  KHE_TIME_GROUP_NHOOD KheTimeGroupNHoodMakeEmpty(int count)               */
/*                                                                           */
/*  Return a time group nhood with count entries, all set to NULL.  This     */
/*  will become the unique singleton time group neighbourhood.               */
/*                                                                           */
/*****************************************************************************/

KHE_TIME_GROUP_NHOOD KheTimeGroupNHoodMakeEmpty(int count)
{
  KHE_TIME_GROUP_NHOOD res;
  MMake(res);
  MArrayInit(res->time_groups);
  MArrayFill(res->time_groups, count, NULL);
  return res;
}


/*****************************************************************************/
/*                                                                           */
/*  void KheTimeGroupNHoodSetTimeGroup(KHE_TIME_GROUP_NHOOD tgn,             */
/*    int pos, KHE_TIME_GROUP tg)                                            */
/*                                                                           */
/*  Set the time group at position pos of tgn to tg.  This is used only      */
/*  to initialize the unique singleton time group neighbourhood.             */
/*                                                                           */
/*****************************************************************************/

void KheTimeGroupNHoodSetTimeGroup(KHE_TIME_GROUP_NHOOD tgn,
  int pos, KHE_TIME_GROUP tg)
{
  MAssert(pos >= 0 && pos < MArraySize(tgn->time_groups),
    "KheTimeGroupNHoodSetTimeGroup internal error (pos %d, size %d)",
     pos, MArraySize(tgn->time_groups));
  MArrayPut(tgn->time_groups, pos, tg);
}


/*****************************************************************************/
/*                                                                           */
/*  void KheTimeGroupNHoodDelete(KHE_TIME_GROUP_NHOOD tgn, int pos)          */
/*                                                                           */
/*  Delete tgn.  If pos == -1, do not delete any of its time groups.         */
/*  If pos >= 0, delete all of its time groups except the one at pos.        */
/*                                                                           */
/*****************************************************************************/

void KheTimeGroupNHoodDelete(KHE_TIME_GROUP_NHOOD tgn, int pos)
{
  int i;
  MAssert(false, "KheTimeGroupNHoodDelete should not be called (?)");
  if( pos >= 0 )
  {
    for( i = 0;  i < MArraySize(tgn->time_groups);  i++ )
      if( i != pos )
	KheTimeGroupDelete(MArrayGet(tgn->time_groups, i));
  }
  MArrayFree(tgn->time_groups);
  MFree(tgn);
}


/*****************************************************************************/
/*                                                                           */
/*  KHE_TIME_GROUP KheTimeGroupNHoodNeighbour(KHE_TIME_GROUP_NHOOD tgn,      */
/*    int pos)                                                               */
/*                                                                           */
/*  Return the time group at this position in pos if there is one, or        */
/*  else the empty time group.                                               */
/*                                                                           */
/*****************************************************************************/

KHE_TIME_GROUP KheTimeGroupNHoodNeighbour(KHE_TIME_GROUP_NHOOD tgn, int pos)
{
  if( pos >= 0 && pos < MArraySize(tgn->time_groups) )
    return MArrayGet(tgn->time_groups, pos);
  else
    return KheInstanceEmptyTimeGroup(KheTimeGroupInstance(
      MArrayFirst(tgn->time_groups)));
}
