
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
/*  FILE:         khe_zone_repair.c                                          */
/*  DESCRIPTION:  Time assignment repair using zones                         */
/*                                                                           */
/*****************************************************************************/
#include "khe.h"
#include "m.h"
#include <limits.h>

#define DEBUG1 1


/*****************************************************************************/
/*                                                                           */
/*  KHE_MEET_GROUP - a group of meets that share a zone and duration         */
/*                                                                           */
/*****************************************************************************/

typedef MARRAY(KHE_MEET) ARRAY_KHE_MEET;

typedef struct khe_meet_group_rec {
  KHE_ZONE		zone;			/* zone of parent node       */
  int			duration;		/* common duration           */
  ARRAY_KHE_MEET	meets;			/* the meets                 */
} *KHE_MEET_GROUP;


/*****************************************************************************/
/*                                                                           */
/*  KHE_MEET_GROUP_SOLVER - a solver for meet groups                         */
/*                                                                           */
/*****************************************************************************/

typedef MARRAY(KHE_MEET_GROUP) ARRAY_KHE_MEET_GROUP;

typedef struct khe_meet_group_solver_rec {
  KHE_LAYER		layer;			/* the layer to repair       */
  ARRAY_KHE_MEET_GROUP	meet_groups;		/* the meet groups           */
} *KHE_MEET_GROUP_SOLVER;


/*****************************************************************************/
/*                                                                           */
/*  Submodule "meet groups"                                                  */
/*                                                                           */
/*****************************************************************************/

/*****************************************************************************/
/*                                                                           */
/*  KHE_MEET_GROUP KheMeetGroupMake(KHE_ZONE zone, int duration)             */
/*                                                                           */
/*  Make a new meet group with these attributes and no meets.                */
/*                                                                           */
/*****************************************************************************/

static KHE_MEET_GROUP KheMeetGroupMake(KHE_ZONE zone, int duration)
{
  KHE_MEET_GROUP res;
  MMake(res);
  res->zone = zone;
  res->duration = duration;
  MArrayInit(res->meets);
  return res;
}


/*****************************************************************************/
/*                                                                           */
/*  bool KheMeetGroupIsTrivial(KHE_MEET_GROUP mg)                            */
/*                                                                           */
/*  Return true if mg is trivial, because all its meets come from the        */
/*  same node.                                                               */
/*                                                                           */
/*****************************************************************************/

static bool KheMeetGroupIsTrivial(KHE_MEET_GROUP mg)
{
  KHE_NODE node;  int i;
  MAssert(MArraySize(mg->meets) > 0, "KheMeetGroupIsTrivial internal error");
  node = KheMeetNode(MArrayFirst(mg->meets));
  for( i = 1;  i < MArraySize(mg->meets);  i++ )
    if( KheMeetNode(MArrayGet(mg->meets, i)) != node )
      return false;
  return true;
}


/*****************************************************************************/
/*                                                                           */
/*  bool KheMeetTrySwap(KHE_MEET meet1, KHE_MEET meet2)                      */
/*                                                                           */
/*  Try swapping the assignments of meet1 and meet2.  If it reduces the      */
/*  cost of the solution, leave it in and return true.  Otherwise take       */
/*  it back and return false.                                                */
/*                                                                           */
/*****************************************************************************/

static bool KheMeetTrySwap(KHE_MEET meet1, KHE_MEET meet2)
{
  KHE_SOLN soln;  KHE_COST cost;
  soln = KheMeetSoln(meet1);
  cost = KheSolnCost(soln);
  if( KheMeetSwap(meet1, meet2) )
  {
    if( DEBUG1 )
    {
      fprintf(stderr, "  ZoneRepair swapping ");
      KheMeetDebug(meet1, 1, -1, stderr);
      fprintf(stderr, " with ");
      KheMeetDebug(meet2, 1, -1, stderr);
      fprintf(stderr, " (%.4f -> %.4f)%s\n",
	KheCostShow(cost), KheCostShow(KheSolnCost(soln)),
	KheSolnCost(soln) < cost ? " new best" : "");
    }
    if( KheSolnCost(soln) < cost )
    {
      return true;
    }
    KheMeetSwap(meet1, meet2);
  }
  return false;
}


/*****************************************************************************/
/*                                                                           */
/*  bool KheMeetGroupRepair(KHE_MEET_GROUP mg)                               */
/*                                                                           */
/*  Try to repair mg and return true if anything worked.                     */
/*                                                                           */
/*****************************************************************************/

static bool KheMeetGroupRepair(KHE_MEET_GROUP mg)
{
  int i, j;  KHE_MEET meet1, meet2;
  for( i = 0;  i < MArraySize(mg->meets);  i++ )
  {
    meet1 = MArrayGet(mg->meets, i);
    for( j = i + 1;  j < MArraySize(mg->meets);  j++ )
    {
      meet2 = MArrayGet(mg->meets, j);
      if( KheMeetNode(meet1) != KheMeetNode(meet2) &&
	  KheMeetTrySwap(meet1, meet2) )
	return true;
    }
  }
  return false;
}


/*****************************************************************************/
/*                                                                           */
/*  void KheMeetGroupDelete(KHE_MEET_GROUP mg)                               */
/*                                                                           */
/*  Delete mg.                                                               */
/*                                                                           */
/*****************************************************************************/

static void KheMeetGroupDelete(KHE_MEET_GROUP mg)
{
  MArrayFree(mg->meets);
  MFree(mg);
}


/*****************************************************************************/
/*                                                                           */
/*  void KheMeetGroupDebug(KHE_MEET_GROUP mg, int verbosity,                 */
/*    int indent, FILE *fp)                                                  */
/*                                                                           */
/*  Debug print mg onto fp with the given verbosity and indent.              */
/*                                                                           */
/*****************************************************************************/

static void KheMeetGroupDebug(KHE_MEET_GROUP mg, int verbosity,
  int indent, FILE *fp)
{
  KHE_MEET meet;  int i;
  if( verbosity >= 1 )
  {
    if( indent >= 0 )
      fprintf(fp, "%*s", indent, "");
    fprintf(fp, "MeetGroup(");
    KheZoneDebug(mg->zone, 1, -1, fp);
    fprintf(fp, ", durn %d: ", mg->duration);
    MArrayForEach(mg->meets, &meet, &i)
    {
      if( i > 0 )
	fprintf(fp, ", ");
      KheMeetDebug(meet, 1, -1, fp);
    }
    fprintf(fp, ")");
    if( indent >= 0 )
      fprintf(fp, "\n");
  }
}


/*****************************************************************************/
/*                                                                           */
/*  Submodule "meet group solvers"                                           */
/*                                                                           */
/*****************************************************************************/

/*****************************************************************************/
/*                                                                           */
/*  KHE_MEET_GROUP_SOLVER KheMeetGroupSolverMake(KHE_LAYER layer,            */
/*    KHE_LAYER template_layer)                                              */
/*                                                                           */
/*  Make a meet group solver with these attributes.                          */
/*                                                                           */
/*****************************************************************************/

static KHE_MEET_GROUP_SOLVER KheMeetGroupSolverMake(KHE_LAYER layer)
{
  KHE_MEET_GROUP_SOLVER res;
  MMake(res);
  res->layer = layer;
  MArrayInit(res->meet_groups);
  return res;
}


/*****************************************************************************/
/*                                                                           */
/*  void KheMeetGroupSolverDelete(KHE_MEET_GROUP_SOLVER mgs)                 */
/*                                                                           */
/*  Delete mgs and its meet groups.                                          */
/*                                                                           */
/*****************************************************************************/

static void KheMeetGroupSolverDelete(KHE_MEET_GROUP_SOLVER mgs)
{
  while( MArraySize(mgs->meet_groups) > 0 )
    KheMeetGroupDelete(MArrayRemoveLast(mgs->meet_groups));
  MArrayFree(mgs->meet_groups);
  MFree(mgs);
}


/*****************************************************************************/
/*                                                                           */
/*  bool KheOverlaps(int offset1, int duration1, int offset2, int duration2) */
/*                                                                           */
/*  Return true if two meets with these offsets and durations, assigned to   */
/*  the same meet, would overlap in time.                                    */
/*                                                                           */
/*****************************************************************************/

/* ***
static bool KheOverlaps(int offset1, int duration1, int offset2, int duration2)
{
  if( offset1 + duration1 <= offset2 )
    return false;
  if( offset2 + duration2 <= offset1 )
    return false;
  return true;
}
*** */


/*****************************************************************************/
/*                                                                           */
/*  bool KheMeetsOverlap(KHE_MEET meet1, KHE_MEET meet2)                     */
/*                                                                           */
/*  Return true if these meets overlap in time.                              */
/*                                                                           */
/*****************************************************************************/

/* ***
static bool KheMeetsOverlap(KHE_MEET meet1, KHE_MEET meet2)
{
  MAssert(KheMeetAsst(meet1) == KheMeetAsst(meet2),
    "KheMeetsOverlap internal error");
  return KheOverlaps(KheMeetAsstOffset(meet1), KheMeetDuration(meet1),
    KheMeetAsstOffset(meet2), KheMeetDuration(meet2));
}
*** */


/*****************************************************************************/
/*                                                                           */
/*  bool KheMeetHasZone(KHE_MEET meet, KHE_ZONE *zone)                       */
/*                                                                           */
/*  If meet is assigned entirely within one zone of its node's parent_node,  */
/*  return true and set *zone to that zone.  Otherwise return false.         */
/*                                                                           */
/*****************************************************************************/

static bool KheMeetHasZone(KHE_MEET meet, KHE_ZONE *zone)
{
  KHE_MEET asst;  int offset, i;  KHE_ZONE zn;
  asst = KheMeetAsst(meet);
  if( asst == NULL )
    return false;
  offset = KheMeetAsstOffset(meet);
  *zone = NULL;
  for( i = 0;  i < KheMeetDuration(meet);  i++ )
  {
    zn = KheMeetOffsetZone(asst, offset + i);
    if( zn == NULL )
      return false;
    else if( *zone == NULL )
      *zone = zn;
    else if( zn != *zone )
      return false;
  }
  return true;
}


/*****************************************************************************/
/*                                                                           */
/*  bool KheMeetGroupSolverFindMeetGroup(KHE_MEET_GROUP_SOLVER mgs,          */
/*    KHE_ZONE zone, int duration, KHE_MEET_GROUP *mg)                       */
/*                                                                           */
/*  If mgs contains a meet group with the given zone duration, set *mg to    */
/*  that meet group and return true.  Otherwise return false.                */
/*                                                                           */
/*****************************************************************************/

static bool KheMeetGroupSolverFindMeetGroup(KHE_MEET_GROUP_SOLVER mgs,
  KHE_ZONE zone, int duration, KHE_MEET_GROUP *mg)
{
  KHE_MEET_GROUP mg2;  int i;
  MArrayForEach(mgs->meet_groups, &mg2, &i)
    if( mg2->zone == zone && mg2->duration == duration )
    {
      *mg = mg2;
      return true;
    }
  *mg = NULL;
  return false;
}


/*****************************************************************************/
/*                                                                           */
/*  KHE_MEET_GROUP_SOLVER KheMeetGroupSolverBuild(KHE_LAYER layer)           */
/*                                                                           */
/*  Build a meet group solver with these attributes.                         */
/*                                                                           */
/*****************************************************************************/

static KHE_MEET_GROUP_SOLVER KheMeetGroupSolverBuild(KHE_LAYER layer)
{
  KHE_MEET_GROUP_SOLVER res;  KHE_MEET_GROUP mg;  KHE_MEET meet;
  KHE_NODE node;  int i, j, durn;  KHE_ZONE zone;

  /* build the solver */
  res = KheMeetGroupSolverMake(layer);
  for( i = 0;  i < KheLayerChildNodeCount(layer);  i++ )
  {
    node = KheLayerChildNode(layer, i);
    for( j = 0;  j < KheNodeMeetCount(node);  j++ )
    {
      meet = KheNodeMeet(node, j);
      durn = KheMeetDuration(meet);
      if( KheMeetHasZone(meet, &zone) )
      {
	if( !KheMeetGroupSolverFindMeetGroup(res, zone, durn, &mg) )
	{
          mg = KheMeetGroupMake(zone, durn);
	  MArrayAddLast(res->meet_groups, mg);
	}
	MArrayAddLast(mg->meets, meet);
      }
    }
  }

  /* delete trivial meet groups */
  MArrayForEach(res->meet_groups, &mg, &i)
    if( KheMeetGroupIsTrivial(mg) )
    {
      if( DEBUG1 )
      {
	fprintf(stderr, "  deleting trivial ");
	KheMeetGroupDebug(mg, 1, 0, stderr);
      }
      KheMeetGroupDelete(mg);
      MArrayRemove(res->meet_groups, i);
      i--;
    }

  /* return */
  return res;
}


/*****************************************************************************/
/*                                                                           */
/*  void KheMeetGroupSolverDebug(KHE_MEET_GROUP_SOLVER mgs, int verbosity,   */
/*    int indent, FILE *fp)                                                  */
/*                                                                           */
/*  Debug print of mgs onto fp with the given verbosity and indent.          */
/*                                                                           */
/*****************************************************************************/

static void KheMeetGroupSolverDebug(KHE_MEET_GROUP_SOLVER mgs, int verbosity,
  int indent, FILE *fp)
{
  KHE_MEET_GROUP mg;  int i;
  if( verbosity >= 1 && indent >= 0 )
  {
    fprintf(fp, "%*s[ MeetGroupSolver\n", indent, "");
    fprintf(fp, "%*s  layer: ", indent, "");
    KheLayerDebug(mgs->layer, 1, 0, fp);
    MArrayForEach(mgs->meet_groups, &mg, &i)
      KheMeetGroupDebug(mg, verbosity, indent + 2, fp);
    fprintf(fp, "%*s]\n", indent, "");
  }
}


/*****************************************************************************/
/*                                                                           */
/*  Submodule "the main solver"                                              */
/*                                                                           */
/*****************************************************************************/

/*****************************************************************************/
/*                                                                           */
/*  void KheLayerZoneRepairTimes(KHE_LAYER layer)                            */
/*                                                                           */
/*  Repair layer, preserving node regularity with the zones of the parent    */
/*  node; or do nothing if the parent node has no zones.                     */
/*                                                                           */
/*****************************************************************************/

void KheLayerZoneRepairTimes(KHE_LAYER layer)
{
  KHE_MEET_GROUP_SOLVER mgs;  int i, fail_count;
  if( DEBUG1 )
    fprintf(stderr, "[ KheLayerZoneRepairTimes(layer)\n");
  if( KheNodeZoneCount(KheLayerParentNode(layer)) > 0 )
  {
    /* build a meet group solver */
    mgs = KheMeetGroupSolverBuild(layer);
    if( DEBUG1 )
      KheMeetGroupSolverDebug(mgs, 2, 2, stderr);

    /* keep repairing while making progress */
    i = fail_count = 0;
    while( fail_count < MArraySize(mgs->meet_groups) )
    {
      i = (i + 1) % MArraySize(mgs->meet_groups);
      if( KheMeetGroupRepair(MArrayGet(mgs->meet_groups, i)) )
	fail_count = 0;
      else
	fail_count++;
    }

    /* delete the meet group solver */
    KheMeetGroupSolverDelete(mgs);
  }
  if( DEBUG1 )
    fprintf(stderr, "] KheLayerZoneRepairTimes returning\n");
}
