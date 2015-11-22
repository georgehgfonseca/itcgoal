
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
/*  FILE:         khe_merge_meets.                                           */
/*  DESCRIPTION:  KheMergeMeets()                                            */
/*                                                                           */
/*****************************************************************************/
#include "khe.h"
#include "m.h"

#define DEBUG1 0
#define DEBUG2 0

typedef MARRAY(KHE_MEET) ARRAY_KHE_MEET;

/*****************************************************************************/
/*                                                                           */
/*  int KheMeetIncreasingTimeAsstCmp(const void *p1, const void *p2)         */
/*                                                                           */
/*  Comparison function for sorting an array of meets with assigned times    */
/*  by increasing chronological order.                                       */
/*                                                                           */
/*****************************************************************************/

static int KheMeetIncreasingTimeAsstCmp(const void *p1, const void *p2)
{
  KHE_MEET meet1 = * (KHE_MEET *) p1;
  KHE_MEET meet2 = * (KHE_MEET *) p2;
  return KheTimeIndex(KheMeetAsstTime(meet1)) -
    KheTimeIndex(KheMeetAsstTime(meet2));
}


/*****************************************************************************/
/*                                                                           */
/*  bool KheMeetsMerged(KHE_MEET meet1, KHE_MEET meet2,                      */
/*    KHE_MEET *rmeet1, KHE_MEET *rmeet2)                                    */
/*                                                                           */
/*  If meet1 and meet2 can be merged and doing so reduces the cost of the    */
/*  solution, then do the merge and return true.  Otherwise don't, but       */
/*  return replacement meets for meet1 and meet2 in *rmeet1 and *rmeet2.     */
/*                                                                           */
/*****************************************************************************/

static bool KheMeetsMerged(KHE_MEET meet1, KHE_MEET meet2,
  KHE_MEET *rmeet1, KHE_MEET *rmeet2)
{
  int duration1;  KHE_COST cost_before;  KHE_SOLN soln;
  if( DEBUG2 )
  {
    fprintf(stderr, "  [ KheMeetsMerged(");
    KheMeetDebug(meet1, 1, -1, stderr);
    fprintf(stderr, ", ");
    KheMeetDebug(meet2, 1, -1, stderr);
    fprintf(stderr, ")\n");
  }
  duration1 = KheMeetDuration(meet1);
  soln = KheMeetSoln(meet1);
  cost_before = KheSolnCost(soln);
  if( !KheMeetMerge(meet1, meet2, rmeet1) )
  {
    /* merge failed, nothing has changed */
    *rmeet1 = meet1;
    *rmeet2 = meet2;
    if( DEBUG2 )
      fprintf(stderr, "  ] KheMeetsMerged returning false (no merge)\n");
    return false;
  }
  else if( KheSolnCost(soln) >= cost_before )
  {
    /* merge succeeded but cost failed, so undo the merge */
    if( !KheMeetSplit(*rmeet1, duration1, false, rmeet1, rmeet2) )
      MAssert(false, "KheMeetsMerged internal error");
    if( DEBUG2 )
      fprintf(stderr, "  ] KheMeetsMerged returning false (%.4f -> %.4f)\n",
	KheCostShow(cost_before), KheCostShow(KheSolnCost(soln)));
    return false;
  }
  else
  {
    /* success, return true (*rmeet2 is undefined) */
    if( DEBUG2 )
      fprintf(stderr, "  ] KheMeetsMerged returning true (%.4f -> %.4f)\n",
	KheCostShow(cost_before), KheCostShow(KheSolnCost(soln)));
    *rmeet2 = NULL;
    return true;
  }
}


/*****************************************************************************/
/*                                                                           */
/*  void KheMergeMeets(KHE_SOLN soln)                                        */
/*                                                                           */
/*  Merge meets of soln where it's possible and reduces cost.                */
/*  NB for this function to work correctly, split events and distribute      */
/*  split events monitors must be attached.                                  */
/*                                                                           */
/*****************************************************************************/

void KheMergeMeets(KHE_SOLN soln)
{
  ARRAY_KHE_MEET meets;  KHE_MEET meet1, meet2, rmeet1, rmeet2;  KHE_EVENT e;
  KHE_INSTANCE ins;  int i, j;
  if( DEBUG1 )
    fprintf(stderr, "[ KheMergeMeets(soln)\n");
  MArrayInit(meets);
  ins = KheSolnInstance(soln);
  for( i = 0;  i < KheInstanceEventCount(ins);  i++ )
  {
    /* find the assigned meets of e and sort them chronologically */
    e = KheInstanceEvent(ins, i);
    MArrayClear(meets);
    for( j = 0;  j < KheEventMeetCount(soln, e);  j++ )
    {
      meet1 = KheEventMeet(soln, e, j);
      if( KheMeetAsstTime(meet1) != NULL )
	MArrayAddLast(meets, meet1);
    }
    MArraySort(meets, &KheMeetIncreasingTimeAsstCmp);

    /* try merging adjacent pairs */
    for( j = 1;  j < MArraySize(meets);  j++ )
    {
      meet1 = MArrayGet(meets, j - 1);
      meet2 = MArrayGet(meets, j);
      if( KheMeetsMerged(meet1, meet2, &rmeet1, &rmeet2) )
      {
	/* meets merged, so meet2 no longer exists */
	MArrayPut(meets, j - 1, rmeet1);
	MArrayRemove(meets, j);
	j--;
      }
      else
      {
	/* meets did not merge, but replace them by rmeet1 and rmeet2 */
	MArrayPut(meets, j - 1, rmeet1);
	MArrayPut(meets, j, rmeet2);
      }
    }
  }
  MArrayFree(meets);
  if( DEBUG1 )
    fprintf(stderr, "] KheMergeMeets returning\n");
}
