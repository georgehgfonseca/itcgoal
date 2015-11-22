
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
/*  FILE:         khe_vizier.c                                               */
/*  DESCRIPTION:  Vizier nodes                                               */
/*                                                                           */
/*****************************************************************************/
#include "khe.h"
#include "m.h"

#define DEBUG1 0
#define DEBUG2 0

typedef MARRAY(KHE_MEET) ARRAY_KHE_MEET;


/*****************************************************************************/
/*                                                                           */
/*  KHE_MEET KheFindVizierTarget(KHE_MEET meet, KHE_NODE vizier_node)        */
/*                                                                           */
/*  Find the meet of vizier_node such that assigning meet to it leaves       */
/*  meet's assignment in the parent node unchanged.                          */
/*                                                                           */
/*****************************************************************************/

static KHE_MEET KheFindVizierTarget(KHE_MEET meet, KHE_NODE vizier_node)
{
  int i;  KHE_MEET vizier_meet;
  if( KheMeetAsst(meet) == NULL )
    return NULL;
  for( i = 0;  i < KheNodeMeetCount(vizier_node);  i++ )
  {
    vizier_meet = KheNodeMeet(vizier_node, i);
    if( KheMeetAsst(vizier_meet) == KheMeetAsst(meet) )
      return vizier_meet;
  }
  MAssert(false, "KheFindVizierTarget internal error");
  return NULL;  /* keep compiler happy */
}


/*****************************************************************************/
/*                                                                           */
/*  void KheMoveChildNode(KHE_NODE child_node, KHE_NODE vizier_node)         */
/*                                                                           */
/*  Move child_node to be a child of vizier_node.  This includes saving      */
/*  the assignments of its meets, and bringing them back after the move.     */
/*                                                                           */
/*****************************************************************************/

static void KheMoveChildNode(KHE_NODE child_node, KHE_NODE vizier_node)
{
  int i, vizier_offset;  KHE_MEET child_meet, vizier_meet;
  ARRAY_KHE_MEET vizier_meets;  ARRAY_INT vizier_offsets;
  if( DEBUG2 )
  {
    fprintf(stderr, "[ KheMoveChildNode(");
    KheNodeDebug(child_node, 1, -1, stderr);
    fprintf(stderr, ", ");
    KheNodeDebug(vizier_node, 1, -1, stderr);
    fprintf(stderr, ")\n");
  }

  /* store the current assignments and ensure all meets are unassigned */
  MArrayInit(vizier_meets);
  MArrayInit(vizier_offsets);
  for( i = 0;  i < KheNodeMeetCount(child_node);  i++ )
  {
    child_meet = KheNodeMeet(child_node, i);
    vizier_meet = KheFindVizierTarget(child_meet, vizier_node);
    MArrayAddLast(vizier_meets, vizier_meet);
    MArrayAddLast(vizier_offsets, KheMeetAsstOffset(child_meet));
    if( DEBUG2 )
    {
      fprintf(stderr, "  saving child_meet: ");
      KheMeetDebug(child_meet, 3, 0, stderr);
      fprintf(stderr, "  vizier_meet: ");
      if( vizier_meet == NULL )
	fprintf(stderr, "null\n");
      else
	KheMeetDebug(vizier_meet, 3, 0, stderr);
    }
    if( vizier_meet != NULL )
      KheMeetUnAssign(child_meet);
  }

  /* move the node */
  if( !KheNodeDeleteParent(child_node) )
    MAssert(false, "KheMoveChildNode internal error 1");
  if( !KheNodeAddParent(child_node, vizier_node) )
    MAssert(false, "KheMoveChildNode internal error 2");

  /* assign the meets as recorded */
  for( i = 0;  i < KheNodeMeetCount(child_node);  i++ )
  {
    child_meet = KheNodeMeet(child_node, i);
    vizier_meet = MArrayGet(vizier_meets, i);
    vizier_offset = MArrayGet(vizier_offsets, i);
    if( DEBUG2 )
    {
      fprintf(stderr, "  restoring child_meet: ");
      KheMeetDebug(child_meet, 3, 0, stderr);
      fprintf(stderr, "  vizier_meet: ");
      if( vizier_meet == NULL )
	fprintf(stderr, "null\n");
      else
	KheMeetDebug(vizier_meet, 3, 0, stderr);
    }
    if( vizier_meet != NULL &&
	!KheMeetAssign(child_meet, vizier_meet, vizier_offset) )
      MAssert(false, "KheMoveChildNode internal error 3");
  }
  MArrayFree(vizier_meets);
  MArrayFree(vizier_offsets);
  if( DEBUG2 )
    fprintf(stderr, "] KheMoveChildNode returning\n");
}


/*****************************************************************************/
/*                                                                           */
/*  KHE_NODE KheNodeInsertVizierNode(KHE_NODE parent_node)                   */
/*                                                                           */
/*  Insert a vizier node directly below parent_node; return the new node.    */
/*                                                                           */
/*****************************************************************************/

KHE_NODE KheNodeInsertVizierNode(KHE_NODE parent_node)
{
  KHE_MEET parent_meet, vizier_meet;  KHE_NODE vizier_node, child_node;
  KHE_SOLN soln;  int i, offset;  KHE_ZONE parent_zone, vizier_zone;
  if( DEBUG1 )
  {
    fprintf(stderr, "[ KheNodeInsertVizierNode(");
    KheNodeDebug(parent_node, 1, -1, stderr);
    fprintf(stderr, ")\n");
  }

  /* build the new vizier node and its meets */
  soln = KheNodeSoln(parent_node);
  vizier_node = KheNodeMake(soln);
  if( !KheNodeAddParent(vizier_node, parent_node) )
    MAssert(false, "KheNodeInsertVizierNode internal error 1");
  for( i = 0;  i < KheNodeMeetCount(parent_node);  i++ )
  {
    parent_meet = KheNodeMeet(parent_node, i);
    vizier_meet = KheMeetMake(soln, KheMeetDuration(parent_meet), NULL);
    if( !KheMeetSetDomain(vizier_meet, NULL) )
      MAssert(false, "KheNodeInsertVizierNode internal error 2");
    if( !KheNodeAddMeet(vizier_node, vizier_meet) )
      MAssert(false, "KheNodeInsertVizierNode internal error 3");
    if( !KheMeetAssign(vizier_meet, parent_meet, 0) )
      MAssert(false, "KheNodeInsertVizierNode internal error 4");
  }
  if( DEBUG1 )
    KheNodeDebug(vizier_node, 4, 2, stderr);

  /* add parent_node's zones to vizier_node */
  for( i = 0;  i < KheNodeZoneCount(parent_node);  i++ )
    KheZoneMake(vizier_node);
  for( i = 0;  i < KheNodeMeetCount(vizier_node);  i++ )
  {
    vizier_meet = KheNodeMeet(vizier_node, i);
    parent_meet = KheMeetAsst(vizier_meet);
    for( offset = 0;  offset < KheMeetDuration(vizier_meet);  offset++ )
    {
      parent_zone = KheMeetOffsetZone(parent_meet, offset);
      if( parent_zone != NULL )
      {
	vizier_zone = KheNodeZone(vizier_node, KheZoneIndex(parent_zone));
	KheMeetOffsetAddZone(vizier_meet, offset, vizier_zone);
      }
    }
  }

  /* delete parent_node's zones */
  while( KheNodeZoneCount(parent_node) > 0 )
    KheZoneDelete(KheNodeZone(parent_node, 0));

  /* move every child node of parent_node except vizier_node to vizier_node */
  for( i = 0;  i < KheNodeChildCount(parent_node);  i++ )
  {
    child_node = KheNodeChild(parent_node, i);
    if( child_node != vizier_node )
    {
      KheMoveChildNode(child_node, vizier_node);
      i--;
    }
  }
  if( DEBUG1 )
    fprintf(stderr, "] KheNodeInsertVizierNode returning\n");
  return vizier_node;
}


/*****************************************************************************/
/*                                                                           */
/*  void KheNodeRemoveVizierNode(KHE_NODE vizier_node)                       */
/*                                                                           */
/*  Remove vizier_node, including moving its zones to its parent.            */
/*                                                                           */
/*****************************************************************************/

void KheNodeRemoveVizierNode(KHE_NODE vizier_node)
{
  KHE_NODE parent_node;  int i, parent_offset, offset;
  KHE_MEET vizier_meet, parent_meet;  KHE_ZONE vizier_zone, parent_zone;
  if( DEBUG1 )
    fprintf(stderr, "[ KheNodeRemoveVizierNode(Node %d)\n",
      KheNodeIndex(vizier_node));
  parent_node = KheNodeParent(vizier_node);
  MAssert(parent_node != NULL,
    "KheNodeRemoveVizierNode: vizier_node has no parent");

  /* remove parent_node's zones and add vizier_node's zones to parent_node */
  while( KheNodeZoneCount(parent_node) > 0 )
    KheZoneDelete(KheNodeZone(parent_node, 0));
  for( i = 0;  i < KheNodeZoneCount(vizier_node);  i++ )
    KheZoneMake(parent_node);
  for( i = 0;  i < KheNodeMeetCount(vizier_node);  i++ )
  {
    vizier_meet = KheNodeMeet(vizier_node, i);
    parent_meet = KheMeetAsst(vizier_meet);
    if( parent_meet != NULL )
    {
      parent_offset = KheMeetAsstOffset(vizier_meet);
      for( offset = 0;  offset < KheMeetDuration(vizier_meet);  offset++ )
      {
	vizier_zone = KheMeetOffsetZone(vizier_meet, offset);
	if( vizier_zone != NULL )
	{
	  parent_zone = KheNodeZone(parent_node, KheZoneIndex(vizier_zone));
	  KheMeetOffsetAddZone(parent_meet, parent_offset+offset, parent_zone);
	}
      }
    }
  }

  /* bypass vizier_node, delete its meets, and delete it */
  KheNodeBypass(vizier_node);
  while( KheNodeMeetCount(vizier_node) > 0 )
    KheMeetDelete(KheNodeMeet(vizier_node, 0));
  KheNodeDelete(vizier_node);
  if( DEBUG1 )
    fprintf(stderr, "] KheNodeRemoveVizierNode returning\n");
}
