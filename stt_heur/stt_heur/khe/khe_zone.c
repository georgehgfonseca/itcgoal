
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
/*  FILE:         khe_zone.c                                                 */
/*  DESCRIPTION:  A zone of a node                                           */
/*                                                                           */
/*****************************************************************************/
#include "khe_interns.h"

#define DEBUG1 0
#define DEBUG2 0

/*****************************************************************************/
/*                                                                           */
/*  KHE_ZONE                                                                 */
/*                                                                           */
/*****************************************************************************/

struct khe_zone_rec {
  void				*back;		/* back pointer              */
  int				index;		/* index in node             */
  int				visit_num;	/* visit number              */
  KHE_NODE			node;		/* the node                  */
  ARRAY_KHE_MEET		meets;		/* meets                     */
  ARRAY_INT			offsets;	/* offsets                   */
  KHE_ZONE			copy;		/* used when copying         */
};


/*****************************************************************************/
/*                                                                           */
/*  Submodule "construction and query"                                       */
/*                                                                           */
/*****************************************************************************/

/*****************************************************************************/
/*                                                                           */
/*  KHE_ZONE KheZoneMake(KHE_NODE node)                                      */
/*                                                                           */
/*  Make a new zone within node.                                             */
/*                                                                           */
/*****************************************************************************/

KHE_ZONE KheZoneMake(KHE_NODE node)
{
  KHE_ZONE res;
  MMake(res);
  res->back = NULL;
  KheNodeAddZone(node, res, &res->index);
  res->node = node;
  res->visit_num = 0;
  MArrayInit(res->meets);
  MArrayInit(res->offsets);
  res->copy = NULL;
  return res;
}


/*****************************************************************************/
/*                                                                           */
/*  KHE_NODE KheZoneNode(KHE_ZONE zone)                                      */
/*                                                                           */
/*  Return the node of zone.                                                 */
/*                                                                           */
/*****************************************************************************/

KHE_NODE KheZoneNode(KHE_ZONE zone)
{
  return zone->node;
}


/*****************************************************************************/
/*                                                                           */
/*  int KheZoneIndex(KHE_ZONE zone)                                          */
/*                                                                           */
/*  Return the index of zone in its node.                                    */
/*                                                                           */
/*****************************************************************************/

int KheZoneIndex(KHE_ZONE zone)
{
  return zone->index;
}


/*****************************************************************************/
/*                                                                           */
/*  void KheZoneSetIndex(KHE_ZONE zone, int index)                           */
/*                                                                           */
/*  Set the index of zone.                                                   */
/*                                                                           */
/*****************************************************************************/

void KheZoneSetIndex(KHE_ZONE zone, int index)
{
  zone->index = index;
}


/*****************************************************************************/
/*                                                                           */
/*  KHE_ZONE KheZoneCopyPhase1(KHE_ZONE zone)                                */
/*                                                                           */
/*  Carry out Phase 1 of the operation of copying zone.                      */
/*                                                                           */
/*****************************************************************************/

KHE_ZONE KheZoneCopyPhase1(KHE_ZONE zone)
{
  KHE_ZONE copy;  int i;  KHE_MEET meet;
  if( zone->copy == NULL )
  {
    MMake(copy);
    zone->copy = copy;
    copy->back = zone->back;
    copy->index = zone->index;
    copy->visit_num = zone->visit_num;
    copy->node = KheNodeCopyPhase1(zone->node);
    MArrayInit(copy->meets);
    MArrayForEach(zone->meets, &meet, &i)
      MArrayAddLast(copy->meets, KheMeetCopyPhase1(meet));
    MArrayInit(copy->offsets);
    MArrayAppend(copy->offsets, zone->offsets, i);
    copy->copy = NULL;
  }
  return zone->copy;
}


/*****************************************************************************/
/*                                                                           */
/*  void KheZoneCopyPhase2(KHE_ZONE zone)                                    */
/*                                                                           */
/*  Carry out Phase 2 of the operation of copying zone.                      */
/*                                                                           */
/*****************************************************************************/

void KheZoneCopyPhase2(KHE_ZONE zone)
{
  KHE_MEET meet;  int i;
  if( zone->copy != NULL )
  {
    zone->copy = NULL;
    KheNodeCopyPhase2(zone->node);
    MArrayForEach(zone->meets, &meet, &i)
      KheMeetCopyPhase2(meet);
  }
}


/*****************************************************************************/
/*                                                                           */
/*  void KheZoneDelete(KHE_ZONE zone)                                        */
/*                                                                           */
/*  Delete zone.                                                             */
/*                                                                           */
/*****************************************************************************/

void KheZoneDelete(KHE_ZONE zone)
{
  KHE_MEET meet;  int offset;

  /* delete the meet offsets of zone */
  while( MArraySize(zone->meets) > 0 )
  {
    meet = MArrayLast(zone->meets);
    offset = MArrayLast(zone->offsets);
    MAssert(KheMeetOffsetZone(meet, offset) == zone,
      "KheZoneDelete internal error");
    KheMeetOffsetDeleteZone(meet, offset);
  }

  /* delete zone from node */
  KheNodeDeleteZone(zone->node, zone, zone->index);

  /* free memory */
  MArrayFree(zone->meets);
  MArrayFree(zone->offsets);
  MFree(zone);
}


/*****************************************************************************/
/*                                                                           */
/*  Submodule "back pointers and visit numbers"                              */
/*                                                                           */
/*****************************************************************************/

/*****************************************************************************/
/*                                                                           */
/*  void KheZoneSetBack(KHE_ZONE zone, void *back)                           */
/*                                                                           */
/*  Set the back pointer of zone.                                            */
/*                                                                           */
/*****************************************************************************/

void KheZoneSetBack(KHE_ZONE zone, void *back)
{
  zone->back = back;
}


/*****************************************************************************/
/*                                                                           */
/*  void *KheZoneBack(KHE_ZONE zone)                                         */
/*                                                                           */
/*  Return the back pointer of zone.                                         */
/*                                                                           */
/*****************************************************************************/

void *KheZoneBack(KHE_ZONE zone)
{
  return zone->back;
}


/*****************************************************************************/
/*                                                                           */
/*  void KheZoneSetVisitNum(KHE_ZONE zone, int num)                          */
/*                                                                           */
/*  Set the visit number of zone.                                            */
/*                                                                           */
/*****************************************************************************/

void KheZoneSetVisitNum(KHE_ZONE zone, int num)
{
  zone->visit_num = num;
}


/*****************************************************************************/
/*                                                                           */
/*  int KheZoneVisitNum(KHE_ZONE zone)                                       */
/*                                                                           */
/*  Return the visit number of zone.                                         */
/*                                                                           */
/*****************************************************************************/

int KheZoneVisitNum(KHE_ZONE zone)
{
  return zone->visit_num;
}


/*****************************************************************************/
/*                                                                           */
/*  bool KheZoneVisited(KHE_ZONE zone, int slack)                            */
/*                                                                           */
/*  Return true if zone has been visited recently.                           */
/*                                                                           */
/*****************************************************************************/

bool KheZoneVisited(KHE_ZONE zone, int slack)
{
  return KheSolnVisitNum(KheNodeSoln(zone->node)) - zone->visit_num <= slack;
}


/*****************************************************************************/
/*                                                                           */
/*  void KheZoneVisit(KHE_ZONE zone)                                         */
/*                                                                           */
/*  Visit zone.                                                              */
/*                                                                           */
/*****************************************************************************/

void KheZoneVisit(KHE_ZONE zone)
{
  zone->visit_num = KheSolnVisitNum(KheNodeSoln(zone->node));
}


/*****************************************************************************/
/*                                                                           */
/*  void KheZoneUnVisit(KHE_ZONE zone)                                       */
/*                                                                           */
/*  Unvisit zone.                                                            */
/*                                                                           */
/*****************************************************************************/

void KheZoneUnVisit(KHE_ZONE zone)
{
  zone->visit_num = KheSolnVisitNum(KheNodeSoln(zone->node)) - 1;
}


/*****************************************************************************/
/*                                                                           */
/*  Submodule "meet offsets"                                                 */
/*                                                                           */
/*****************************************************************************/

/*****************************************************************************/
/*                                                                           */
/*  int KheZoneMeetOffsetCount(KHE_ZONE zone)                                */
/*                                                                           */
/*  Return the number of meet offsets of zone.                               */
/*                                                                           */
/*****************************************************************************/

int KheZoneMeetOffsetCount(KHE_ZONE zone)
{
  return MArraySize(zone->meets);
}


/*****************************************************************************/
/*                                                                           */
/*  void KheZoneMeetOffset(KHE_ZONE zone, int i, KHE_MEET *meet, int *offset)*/
/*                                                                           */
/*  Return the i'th meet-offset of zone.                                     */
/*                                                                           */
/*****************************************************************************/

void KheZoneMeetOffset(KHE_ZONE zone, int i, KHE_MEET *meet, int *offset)
{
  *meet = MArrayGet(zone->meets, i);
  *offset = MArrayGet(zone->offsets, i);
}


/*****************************************************************************/
/*                                                                           */
/*  void KheZoneAddMeetOffset(KHE_ZONE zone, KHE_MEET meet, int offset)      */
/*                                                                           */
/*  Add (meet, offset) to zone.                                              */
/*                                                                           */
/*****************************************************************************/

void KheZoneAddMeetOffset(KHE_ZONE zone, KHE_MEET meet, int offset)
{
  MArrayAddLast(zone->meets, meet);
  MArrayAddLast(zone->offsets, offset);
}


/*****************************************************************************/
/*                                                                           */
/*  void KheZoneDeleteMeetOffset(KHE_ZONE zone, KHE_MEET meet, int offset)   */
/*                                                                           */
/*  Delete (meet, offset) from zone.                                         */
/*                                                                           */
/*****************************************************************************/

void KheZoneDeleteMeetOffset(KHE_ZONE zone, KHE_MEET meet, int offset)
{
  int i, offs;  KHE_MEET mt;
  for( i = 0;  i < MArraySize(zone->meets);  i++ )
  {
    mt = MArrayGet(zone->meets, i);
    offs = MArrayGet(zone->offsets, i);
    if( mt == meet && offs == offset )
    {
      MArrayRemove(zone->meets, i);
      MArrayRemove(zone->offsets, i);
      return;
    }
  }
  MAssert(false, "KheZoneUpdateMeetOffset internal error");
}


/*****************************************************************************/
/*                                                                           */
/*  void KheZoneUpdateMeetOffset(KHE_ZONE zone, KHE_MEET old_meet,           */
/*    int old_offset, KHE_MEET new_meet, int new_offset)                     */
/*                                                                           */
/*  Replace (old_meet, old_offset) in zone by (new_meet, new_offset).        */
/*                                                                           */
/*****************************************************************************/

void KheZoneUpdateMeetOffset(KHE_ZONE zone, KHE_MEET old_meet,
  int old_offset, KHE_MEET new_meet, int new_offset)
{
  int i, offset;  KHE_MEET meet;
  for( i = 0;  i < MArraySize(zone->meets);  i++ )
  {
    meet = MArrayGet(zone->meets, i);
    offset = MArrayGet(zone->offsets, i);
    if( meet == old_meet && offset == old_offset )
    {
      MArrayPut(zone->meets, i, new_meet);
      MArrayPut(zone->offsets, i, new_offset);
      return;
    }
  }
  MAssert(false, "KheZoneUpdateMeetOffset internal error");
}


/*****************************************************************************/
/*                                                                           */
/*  Submodule "helper functions involving zones"                             */
/*                                                                           */
/*****************************************************************************/

/*****************************************************************************/
/*                                                                           */
/*  bool KheMeetSegmentContainsZone(KHE_MEET meet, int offset, int durn,     */
/*    KHE_ZONE zone)                                                         */
/*                                                                           */
/*  Return true if the segment of meet beginning at offset of duration       */
/*  durn contains zone.                                                      */
/*                                                                           */
/*****************************************************************************/

static bool KheMeetSegmentContainsZone(KHE_MEET meet, int offset, int durn,
  KHE_ZONE zone)
{
  int i;
  for( i = 0;  i < durn;  i++ )
    if( KheMeetOffsetZone(meet, offset + i) == zone )
      return true;
  return false;
}


/*****************************************************************************/
/*                                                                           */
/*  bool KheMeetMovePreservesZones(KHE_MEET meet1, int offset1,              */
/*    KHE_MEET meet2, int offset2, int durn)                                 */
/*                                                                           */
/*  Return true if changing the assignment of a meet of duration durn        */
/*  from meet1 at offset1 to meet2 at offset2 would not change its zones.    */
/*                                                                           */
/*****************************************************************************/

bool KheMeetMovePreservesZones(KHE_MEET meet1, int offset1,
  KHE_MEET meet2, int offset2, int durn)
{
  int i;  KHE_ZONE zone;
  for( i = 0;  i < durn;  i++ )
  {
    zone = KheMeetOffsetZone(meet1, offset1 + i);
    if( !KheMeetSegmentContainsZone(meet2, offset2, durn, zone) )
      return false;
  }
  for( i = 0;  i < durn;  i++ )
  {
    zone = KheMeetOffsetZone(meet2, offset2 + i);
    if( !KheMeetSegmentContainsZone(meet1, offset1, durn, zone) )
      return false;
  }
  return true;
}


/*****************************************************************************/
/*                                                                           */
/*  Submodule "debug"                                                        */
/*                                                                           */
/*****************************************************************************/

/*****************************************************************************/
/*                                                                           */
/*  void KheZoneDebug(KHE_ZONE zone, int verbosity, int indent, FILE *fp)    */
/*                                                                           */
/*  Debug print of zone onto fp with the given indent.                       */
/*                                                                           */
/*****************************************************************************/

void KheZoneDebug(KHE_ZONE zone, int verbosity, int indent, FILE *fp)
{
  int i, j, offset;  KHE_MEET meet, next_meet;
  if( verbosity >= 1 && indent >= 0 )
  {
    fprintf(fp, "%*s[ Zone %d of Node %d: ", indent, "", zone->index,
      KheNodeIndex(zone->node));
    for( i = 0;  i < MArraySize(zone->meets);  i = j )
    {
      if( i > 0 )
	fprintf(fp, ", ");
      meet = MArrayGet(zone->meets, i);
      KheMeetDebug(meet, 1, -1, fp);
      for( j = i;  j < MArraySize(zone->meets);  j++ )
      {
	next_meet = MArrayGet(zone->meets, j);
	offset = MArrayGet(zone->offsets, j);
	if( next_meet != meet )
	  break;
	fprintf(fp, "+%d", offset);
      }
    }
    fprintf(fp, " ]\n");
  }
}
