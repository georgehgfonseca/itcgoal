
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
/*  FILE:         khe_meet.c                                                 */
/*  DESCRIPTION:  A meet                                                     */
/*                                                                           */
/*****************************************************************************/
#include "khe_interns.h"
#define DEBUG1 0
#define DEBUG2 0
#define DEBUG3 0
#define DEBUG4 0
#define DEBUG5 0
#define DEBUG6 0
#define DEBUG7 0
#define DEBUG8 0
#define DEBUG9 0
#define DEBUG10 0
#define DEBUG12 0
#define DEBUG13 0


/*****************************************************************************/
/*                                                                           */
/*  KHE_MEET - a meet                                                        */
/*                                                                           */
/*****************************************************************************/

struct khe_meet_rec {
  void				*back;			/* back pointer      */
  KHE_SOLN			soln;			/* enclosing soln    */
  int				index;			/* in soln list      */
  int				duration;		/* duration          */
  KHE_TIME_GROUP		time_domain;		/* time domain       */
  ARRAY_KHE_TASK		tasks;			/* tasks             */
  KHE_MEET			target_meet;		/* assigned to       */
  int				target_offset;		/* offset in target  */
  int				target_index;		/* index in target   */
  int				assigned_time_index;	/* if time assigned  */
  ARRAY_KHE_MEET		assigned_meets;		/* assigned to here  */
  KHE_NODE			node;			/* optional node     */
  int				all_demand;		/* demand            */
  int				visit_num;		/* visit number      */
  KHE_EVENT_IN_SOLN		event_in_soln;		/* encl em           */
  ARRAY_KHE_ZONE		zones;			/* zones             */
  ARRAY_KHE_MATCHING_SUPPLY_CHUNK supply_chunks;	/* when matching     */
  ARRAY_KHE_MATCHING_DEMAND_CHUNK demand_chunks;	/* when matching     */
  KHE_MEET			copy;			/* used when copying */
};


/*****************************************************************************/
/*                                                                           */
/*  Submodule "construction and query"                                       */
/*                                                                           */
/*****************************************************************************/

/*****************************************************************************/
/*                                                                           */
/*  void KheMeetCheckAsstInvt(KHE_MEET meet)                                 */
/*                                                                           */
/*  Check the asst invariant of meet and fail if it's violated.              */
/*                                                                           */
/*****************************************************************************/

/* *** used for debugging only
static void KheMeetCheckAsstInvt(KHE_MEET meet)
{
  if( meet->target_meet != NULL )
  {
    if( meet->target_offset < 0 ||
        meet->target_offset + meet->duration > meet->target_meet->duration )
    {
      if( DEBUG11 )
      {
	fprintf(stderr, "KheMeetCheckAsstInvt(");
	KheMeetDebug(meet, 1, -1, stderr);
	fprintf(stderr, " := ");
	KheMeetDebug(meet->target_meet, 1, -1, stderr);
	fprintf(stderr, "+%d) failing\n", meet->target_offset);
      }
      MAssert(false, "KheMeetCheckAsstInvt internal error");
    }
  }
}
*** */


/*****************************************************************************/
/*                                                                           */
/*  KHE_MEET KheMeetMake(KHE_SOLN soln, int duration, KHE_EVENT e)           */
/*                                                                           */
/*  Make a meet and add it to soln.  No tasks are added.                     */
/*                                                                           */
/*****************************************************************************/

KHE_MEET KheMeetMake(KHE_SOLN soln, int duration, KHE_EVENT e)
{
  KHE_MEET res;  int i;
  if( DEBUG4 )
    fprintf(stderr, "[ KheMeetMake(soln, %d, \"%s\")\n",
      duration, e == NULL || KheEventId(e) == NULL ? "-" : KheEventId(e));
  MAssert(duration > 0, "KheMeetMake: invalid duration (%d)", duration);
  MAssert(soln != NULL, "KheMeetMake: soln parameter is null");
  MMake(res);
  res->back = NULL;
  res->soln = soln;
  KheSolnAddMeet(soln, res, &res->index);
  res->duration = duration;
  if( e != NULL && KheEventPreassignedTime(e) != NULL )
    res->time_domain = KheTimeSingletonTimeGroup(KheEventPreassignedTime(e));
  else
    res->time_domain = KheInstanceFullTimeGroup(KheSolnInstance(soln));
  MArrayInit(res->tasks);
  res->target_meet = NULL;
  res->target_offset = -1;
  res->target_index = -1;
  res->assigned_time_index = -1;
  MArrayInit(res->assigned_meets);
  res->node = NULL;
  res->all_demand = 0;
  res->visit_num = 0;
  if( e != NULL )
  {
    res->event_in_soln = KheSolnEventInSoln(soln, KheEventIndex(e));
    KheEventInSolnAddMeet(res->event_in_soln, res);
  }
  else
    res->event_in_soln = NULL;

  /* zones */
  MArrayInit(res->zones);
  MArrayFill(res->zones, res->duration, NULL);

  /* matching supply and demand chunks */
  MArrayInit(res->supply_chunks);
  for( i = 0;  i < res->duration;  i++ )
    MArrayAddLast(res->supply_chunks,
      KheSolnMatchingMakeOrdinarySupplyChunk(soln, res));
  MArrayInit(res->demand_chunks);
  for( i = 0;  i < res->duration;  i++ )
    MArrayAddLast(res->demand_chunks, KheMatchingDemandChunkMake(
      KheSolnMatching(soln), NULL, 0, 0, KheSolnMatchingZeroDomain(soln)));
  KheMeetMatchingReset(res);

  res->copy = NULL;

  if( DEBUG4 )
  {
    fprintf(stderr, "  time domain: ");
    KheTimeGroupDebug(res->time_domain, 1, -1, stderr);
    fprintf(stderr, "\n] KheMeetMake\n");
  }

  /* inform soln of this operation and return */
  KheSolnOpMeetMake(soln, res);
  /* KheMeetCheckAsstInvt(res); */
  return res;
}


/*****************************************************************************/
/*                                                                           */
/*  void KheMeetFree(KHE_MEET meet)                                          */
/*                                                                           */
/*  Free the memory occupied by meet.                                        */
/*                                                                           */
/*****************************************************************************/

static void KheMeetFree(KHE_MEET meet)
{
  MArrayFree(meet->tasks);
  MArrayFree(meet->assigned_meets);
  MArrayFree(meet->zones);
  MArrayFree(meet->supply_chunks);
  MArrayFree(meet->demand_chunks);
  MFree(meet);
}


/*****************************************************************************/
/*                                                                           */
/*  void KheMeetDelete(KHE_MEET meet)                                        */
/*                                                                           */
/*  Delete meet from soln.                                                   */
/*                                                                           */
/*****************************************************************************/

void KheMeetDelete(KHE_MEET meet)
{
  /* KheMeetCheckAsstInvt(meet); */
  /* can't delete a cycle meet */
  /* ***
  MAssert(!KheMeetIsCycleMeet(meet),
    "KheMeetDelete: meet is a cycle meet");
  *** */

  /* inform soln that this operation is occurring */
  MAssert(meet->soln != NULL, "KheMeetDelete internal error");
  KheSolnOpMeetDelete(meet->soln);

  /* remove the tasks of meet */
  while( MArraySize(meet->tasks) > 0 )
    KheTaskDelete(MArrayLast(meet->tasks));

  /* unassign meet, if assigned */
  if( meet->target_meet != NULL )
    KheMeetUnAssign(meet);

  /* unassign everything assigned to meet */
  while( MArraySize(meet->assigned_meets) > 0 )
    KheMeetUnAssign(MArrayLast(meet->assigned_meets));

  /* remove meet from its node, if any (will also clear zones) */
  if( meet->node != NULL )
    KheNodeDeleteMeet(meet->node, meet);
  /* ***
  while( MArraySize(meet->own_layers) > 0 )
    KheLayerDeleteMeet(MArrayLast(meet->own_layers), meet);
  *** */

  /* remove meet from its event monitor */
  if( meet->event_in_soln != NULL )
    KheEventInSolnDeleteMeet(meet->event_in_soln, meet);

  /* remove the demand chunks of meet (each will be empty by now) */
  while( MArraySize(meet->demand_chunks) > 0 )
    KheMatchingDemandChunkDelete(MArrayRemoveLast(meet->demand_chunks));

  /* remove the supply chunks and save for reuse later */
  while( MArraySize(meet->supply_chunks) > 0 )
    KheSolnMatchingAddOrdinarySupplyChunkToFreeList(
      meet->soln, MArrayRemoveLast(meet->supply_chunks));
    /* ***
    KheMatchingMonitorAddOrdinarySupplyChunkToFreeList(
      KheSolnMatchingMonitor(meet->soln),
      MArrayRemoveLast(meet->supply_chunks));
    *** */

  /* get rid of meet from soln and free it */
  KheSolnDeleteMeet(meet->soln, meet);
  KheMeetFree(meet);
}


/*****************************************************************************/
/*                                                                           */
/*  void KheMeetSetBack(KHE_MEET meet, void *back)                           */
/*                                                                           */
/*  Set the back pointer of meet.                                            */
/*                                                                           */
/*****************************************************************************/

void KheMeetSetBack(KHE_MEET meet, void *back)
{
  meet->back = back;
}


/*****************************************************************************/
/*                                                                           */
/*  void *KheMeetBack(KHE_MEET meet)                                         */
/*                                                                           */
/*  Return the back pointer of meet.                                         */
/*                                                                           */
/*****************************************************************************/

void *KheMeetBack(KHE_MEET meet)
{
  return meet->back;
}


/*****************************************************************************/
/*                                                                           */
/*  int KheMeetIndex(KHE_MEET meet)                                          */
/*                                                                           */
/*  Return the index attribute of meet.                                      */
/*                                                                           */
/*****************************************************************************/

int KheMeetIndex(KHE_MEET meet)
{
  return meet->index;
}


/*****************************************************************************/
/*                                                                           */
/*  void KheMeetSetIndex(KHE_MEET meet, int index)                           */
/*                                                                           */
/*  Set the index attribute of meet.                                         */
/*                                                                           */
/*****************************************************************************/

void KheMeetSetIndex(KHE_MEET meet, int index)
{
  meet->index = index;
}


/*****************************************************************************/
/*                                                                           */
/*  void KheMeetSetAssignedTimeIndex(KHE_MEET meet, int assigned_time_index) */
/*                                                                           */
/*  Set the assigned time index attribute of meet.  This is called only      */
/*  when initializing the cycle layer.                                       */
/*                                                                           */
/*****************************************************************************/

void KheMeetSetAssignedTimeIndex(KHE_MEET meet, int assigned_time_index)
{
  meet->assigned_time_index = assigned_time_index;
}


/*****************************************************************************/
/*                                                                           */
/*  int KheMeetAssignedTimeIndex(KHE_MEET meet)                              */
/*                                                                           */
/*  Return the assigned time index of meet, or -1 if not assigned a time.    */
/*                                                                           */
/*****************************************************************************/

int KheMeetAssignedTimeIndex(KHE_MEET meet)
{
  return meet->assigned_time_index;
}


/*****************************************************************************/
/*                                                                           */
/*  KHE_MEET KheMeetCopyPhase1(KHE_MEET meet)                                */
/*                                                                           */
/*  Carry out Phase 1 of copying meet.                                       */
/*                                                                           */
/*****************************************************************************/

KHE_MEET KheMeetCopyPhase1(KHE_MEET meet)
{
  KHE_MEET copy, ameet;  KHE_TASK task;  int i;  KHE_ZONE zone;
  KHE_MATCHING_SUPPLY_CHUNK sc;  KHE_MATCHING_DEMAND_CHUNK dc, dc2;
  /* KHE_LAYER layer;  LSET lset; */
  if( meet->copy == NULL )
  {
    MMake(copy);
    meet->copy = copy;
    copy->back = meet->back;
    copy->soln = KheSolnCopyPhase1(meet->soln);
    copy->index = meet->index;
    copy->duration = meet->duration;
    copy->time_domain = meet->time_domain;
    /* copy->curr_time_domain = meet->curr_time_domain; */
    MArrayInit(copy->tasks);
    MArrayForEach(meet->tasks, &task, &i)
      MArrayAddLast(copy->tasks, KheTaskCopyPhase1(task));
    copy->target_meet = (meet->target_meet == NULL ? NULL :
      KheMeetCopyPhase1(meet->target_meet));
    copy->target_offset = meet->target_offset;
    copy->target_index = meet->target_index;
    copy->assigned_time_index = meet->assigned_time_index;
    MArrayInit(copy->assigned_meets);
    MArrayForEach(meet->assigned_meets, &ameet, &i)
      MArrayAddLast(copy->assigned_meets, KheMeetCopyPhase1(ameet));
    copy->node = (meet->node == NULL ? NULL : KheNodeCopyPhase1(meet->node));
    copy->all_demand = meet->all_demand;
    copy->visit_num = meet->visit_num;
    copy->event_in_soln = (meet->event_in_soln == NULL ? NULL :
      KheEventInSolnCopyPhase1(meet->event_in_soln));
    MArrayInit(copy->zones);
    MArrayForEach(meet->zones, &zone, &i)
      MArrayAddLast(copy->zones, zone == NULL ? NULL : KheZoneCopyPhase1(zone));
    MArrayInit(copy->supply_chunks);
    MArrayForEach(meet->supply_chunks, &sc, &i)
      MArrayAddLast(copy->supply_chunks, KheMatchingSupplyChunkCopyPhase1(sc));
    MArrayInit(copy->demand_chunks);
    MArrayForEach(meet->demand_chunks, &dc, &i)
    {
      MAssert((KHE_SOLN) KheMatchingImpl(KheMatchingDemandChunkMatching(dc))
	== meet->soln, "KheMeetCopyPhase1 internal error 1 (%p != %p)\n",
        KheMatchingImpl(KheMatchingDemandChunkMatching(dc)),
	(void *) copy->soln);
      dc2 = KheMatchingDemandChunkCopyPhase1(dc);
      MAssert((KHE_SOLN) KheMatchingImpl(KheMatchingDemandChunkMatching(dc2))
	== copy->soln, "KheMeetCopyPhase1 internal error 2 (%p != %p)\n",
        KheMatchingImpl(KheMatchingDemandChunkMatching(dc2)),
	(void *) copy->soln);
      MArrayAddLast(copy->demand_chunks, dc2);
    }
    copy->copy = NULL;
    /* KheMeetCheckAsstInvt(meet); */
    /* KheMeetCheckAsstInvt(copy); */
  }
  return meet->copy;
}


/*****************************************************************************/
/*                                                                           */
/*  void KheMeetCopyPhase2(KHE_MEET meet)                                    */
/*                                                                           */
/*  Carry out Phase 2 of copying meet.                                       */
/*                                                                           */
/*****************************************************************************/

void KheMeetCopyPhase2(KHE_MEET meet)
{
  int i;  KHE_TASK task;  KHE_ZONE zone;
  KHE_MATCHING_SUPPLY_CHUNK sc;  KHE_MATCHING_DEMAND_CHUNK dc;
  if( meet->copy != NULL )
  {
    meet->copy = NULL;
    MArrayForEach(meet->tasks, &task, &i)
      KheTaskCopyPhase2(task);
    MArrayForEach(meet->zones, &zone, &i)
      if( zone != NULL )
	KheZoneCopyPhase2(zone);
    MArrayForEach(meet->supply_chunks, &sc, &i)
      KheMatchingSupplyChunkCopyPhase2(sc);
    MArrayForEach(meet->demand_chunks, &dc, &i)
      KheMatchingDemandChunkCopyPhase2(dc);
    /* *** omit these, since directly reachable from the soln object itself
    if( meet->event_in_soln != NULL )
      KheEventInSolnCopyPhase2(meet->event_in_soln);
    if( meet->target_meet != NULL )
      KheMeetCopyPhase2(meet->target_meet);
    MArrayForEach(meet->assigned_meets, &ameet, &i)
      KheMeetCopyPhase2(ameet);
    MArrayForEach(meet->own_layers, &layer, &i)
      KheLayerCopyPhase2(layer);
    *** */
  }
}


/*****************************************************************************/
/*                                                                           */
/*  KHE_SOLN KheMeetSoln(KHE_MEET meet)                                      */
/*                                                                           */
/*  Return the enclosing solution of meet.                                   */
/*                                                                           */
/*****************************************************************************/

KHE_SOLN KheMeetSoln(KHE_MEET meet)
{
  return meet->soln;
}


/*****************************************************************************/
/*                                                                           */
/*  KHE_EVENT KheMeetEvent(KHE_MEET meet)                                    */
/*                                                                           */
/*  Return the event that meet is derived from, or NULL if none.             */
/*                                                                           */
/*****************************************************************************/

KHE_EVENT KheMeetEvent(KHE_MEET meet)
{
  if( meet->event_in_soln == NULL )
    return NULL;
  else
    return KheEventInSolnEvent(meet->event_in_soln);
}


/*****************************************************************************/
/*                                                                           */
/*  KHE_EVENT_IN_SOLN KheMeetEventInSoln(KHE_MEET meet)                      */
/*                                                                           */
/*  Return meet's event in soln.                                             */
/*                                                                           */
/*****************************************************************************/

KHE_EVENT_IN_SOLN KheMeetEventInSoln(KHE_MEET meet)
{
  return meet->event_in_soln;
}


/*****************************************************************************/
/*                                                                           */
/*  int KheMeetDuration(KHE_MEET meet)                                       */
/*                                                                           */
/*  Return the duration of meet.                                             */
/*                                                                           */
/*****************************************************************************/

int KheMeetDuration(KHE_MEET meet)
{
  return meet->duration;
}


/*****************************************************************************/
/*                                                                           */
/*  int KheMeetAssignedDuration(KHE_MEET meet)                               */
/*                                                                           */
/*  Return the assigned duration of meet:  its duration if assigned, and     */
/*  0 otherwise.                                                             */
/*                                                                           */
/*****************************************************************************/

int KheMeetAssignedDuration(KHE_MEET meet)
{
  return meet->target_meet != NULL ? meet->duration : 0;
}


/*****************************************************************************/
/*                                                                           */
/*  int KheMeetDemand(KHE_MEET meet)                                         */
/*                                                                           */
/*  Return the demand of meet.                                               */
/*                                                                           */
/*****************************************************************************/

int KheMeetDemand(KHE_MEET meet)
{
  return meet->all_demand;
  /* ***
  int i, res;  KHE_MEET child_meet;
  res = meet->duration * MArraySize(meet->tasks);
  MArrayForEach(meet->assigned_meets, &child_meet, &i)
    res += KheMeetDemand(child_meet);
  return res;
  *** */
}


/*****************************************************************************/
/*                                                                           */
/*  Submodule "nodes"                                                        */
/*                                                                           */
/*****************************************************************************/

/*****************************************************************************/
/*                                                                           */
/*  KHE_NODE KheMeetNode(KHE_MEET meet)                                      */
/*                                                                           */
/*  Return the node containing meet, or NULL if none.                        */
/*                                                                           */
/*****************************************************************************/

KHE_NODE KheMeetNode(KHE_MEET meet)
{
  return meet->node;
}


/*****************************************************************************/
/*                                                                           */
/*  bool KheMeetAddNodeCheck(KHE_MEET meet, KHE_NODE node)                   */
/*                                                                           */
/*  Check whether adding node to meet is safe.                               */
/*                                                                           */
/*****************************************************************************/

bool KheMeetAddNodeCheck(KHE_MEET meet, KHE_NODE node)
{
  /* meet must not be already assigned to a node */
  MAssert(meet->node == NULL,
    "KheNodeAddMeetCheck: meet already lies in a node");

  /* check the node rule at meet */
  if( meet->target_meet != NULL )
  {
    /* it will become true that "meet meet lies in node and is  */
    /* assigned to target_meet", therefore it must be true */
    /* that "node has a parent and target_meet lies in that parent." */
    if( KheNodeParent(node) == NULL ||
	KheMeetNode(meet->target_meet) != KheNodeParent(node) )
      return false;
  }
  return true;
}


/*****************************************************************************/
/*                                                                           */
/*  void KheMeetSetNodeInternal(KHE_MEET meet, KHE_NODE node)                */
/*                                                                           */
/*  Set the node attribute of meet.  This is used internally, unchecked.     */
/*  NB changing the node means that all zones are lost.                      */
/*                                                                           */
/*****************************************************************************/

void KheMeetSetNodeInternal(KHE_MEET meet, KHE_NODE node)
{
  int offset;  KHE_ZONE zone;
  if( node != meet->node )
    MArrayForEach(meet->zones, &zone, &offset)
      if( zone != NULL )
        KheMeetOffsetDeleteZone(meet, offset);
  meet->node = node;
}


/*****************************************************************************/
/*                                                                           */
/*  bool KheMeetDeleteNodeCheck(KHE_MEET meet, KHE_NODE node)                */
/*                                                                           */
/*  Chceck whether it is safe to delete meet from node.                      */
/*                                                                           */
/*****************************************************************************/

bool KheMeetDeleteNodeCheck(KHE_MEET meet, KHE_NODE node)
{
  KHE_MEET child_meet;  int i;

  /* meet must be already assigned to node */
  if( meet->node != node )
    MAssert(false, "KheNodeDeleteMeetCheck: meet does not lie in node");

  /* check the node rule at each child of meet */
  MArrayForEach(meet->assigned_meets, &child_meet, &i)
    if( child_meet->node != NULL )
    {
      /* at this point we have "meet child_meet lies in node and  */
      /* is assigned to meet meet", therefore it must be true     */
      /* that "node has a parent" - but we are trying to delete that node */
      return false;
    }

  return true;
}


/*****************************************************************************/
/*                                                                           */
/*  void KheMeetFindNodeTarget(KHE_MEET meet, KHE_NODE node,                 */
/*    KHE_MEET *target_meet, int *target_offset)                             */
/*                                                                           */
/*  If meet is assigned, directly or indirectly, to a meet lying             */
/*  in node, then set *target_meet to that meet and *target_offset           */
/*  to meet's offset in *target_meet; else set them to NULL and -1.          */
/*                                                                           */
/*****************************************************************************/

void KheMeetFindNodeTarget(KHE_MEET meet, KHE_NODE node,
  KHE_MEET *target_meet, int *target_offset)
{
  int offset;
  offset = 0;
  while( meet->node != node && meet->target_meet != NULL )
  {
    offset += meet->target_offset;
    meet = meet->target_meet;
  }
  if( meet->node == node )
  {
    *target_meet = meet;
    *target_offset = offset;
  }
  else
  {
    *target_meet = NULL;
    *target_offset = -1;
  }
}


/*****************************************************************************/
/*                                                                           */
/*  Submodule "zones"                                                        */
/*                                                                           */
/*****************************************************************************/

/*****************************************************************************/
/*                                                                           */
/*  void KheMeetOffsetAddZone(KHE_MEET meet, int offset, KHE_ZONE zone)      */
/*                                                                           */
/*  Add zone to meet at offset.                                              */
/*                                                                           */
/*****************************************************************************/

void KheMeetOffsetAddZone(KHE_MEET meet, int offset, KHE_ZONE zone)
{
  MAssert(offset >= 0 && offset < KheMeetDuration(meet),
    "KheMeetOffsetAddZone: offset (%d) out of range (0 .. %d)",
    offset, KheMeetDuration(meet) - 1);
  MAssert(MArrayGet(meet->zones, offset) == NULL,
    "KheMeetOffsetAddZone: (meet, %d) already has a zone", offset);
  MAssert(meet->node != NULL, "KheMeetOffsetAddZone: meet not in node");
  MAssert(KheZoneNode(zone) == meet->node,
    "KheMeetOffsetAddZone: meet's node and zone's node are different");
  MArrayPut(meet->zones, offset, zone);
  KheZoneAddMeetOffset(zone, meet, offset);
}


/*****************************************************************************/
/*                                                                           */
/*  void KheMeetOffsetDeleteZone(KHE_MEET meet, int offset)                  */
/*                                                                           */
/*  Delete the zone of meet at offset.                                       */
/*                                                                           */
/*****************************************************************************/

void KheMeetOffsetDeleteZone(KHE_MEET meet, int offset)
{
  KHE_ZONE zone;
  MAssert(offset >= 0 && offset < KheMeetDuration(meet),
    "KheMeetOffsetDeleteZone: offset (%d) out of range (0 .. %d)",
    offset, KheMeetDuration(meet) - 1);
  zone = MArrayGet(meet->zones, offset);
  MAssert(zone != NULL,
    "KheMeetOffsetDeleteZone: (meet, %d) has no zone", offset);
  MArrayPut(meet->zones, offset, NULL);
  KheZoneDeleteMeetOffset(zone, meet, offset);
}


/*****************************************************************************/
/*                                                                           */
/*  KHE_ZONE KheMeetOffsetZone(KHE_MEET meet, int offset)                    */
/*                                                                           */
/*  Return the zone of meet at offset (possibly NULL).                       */
/*                                                                           */
/*****************************************************************************/

KHE_ZONE KheMeetOffsetZone(KHE_MEET meet, int offset)
{
  MAssert(offset >= 0 && offset < KheMeetDuration(meet),
    "KheMeetOffsetZone: offset (%d) out of range (0 .. %d)",
    offset, KheMeetDuration(meet) - 1);
  return MArrayGet(meet->zones, offset);
}


/*****************************************************************************/
/*                                                                           */
/*  Submodule "tasks"                                                        */
/*                                                                           */
/*****************************************************************************/

/*****************************************************************************/
/*                                                                           */
/*  void KheMeetAssignPreassignedResources(KHE_MEET meet,                    */
/*    KHE_RESOURCE_TYPE rt, bool as_in_event_resource)                       */
/*                                                                           */
/*  Assign preassigned resources to those tasks of meet that are not         */
/*  already assigned and have them.  If rt != NULL, this applies only to     */
/*  tasks of that resource type.                                             */
/*                                                                           */
/*  If as_in_event_resource is true, a task is considered to have a          */
/*  preassigned resource if it is derived from an event resource with        */
/*  a preassigned resource.  If as_in_event_resource is false, a task        */
/*  is considered to have a preassigned resource if its domain contains      */
/*  exactly one element.                                                     */
/*                                                                           */
/*****************************************************************************/

void KheMeetAssignPreassignedResources(KHE_MEET meet,
  KHE_RESOURCE_TYPE rt, bool as_in_event_resource)
{
  KHE_TASK task;  KHE_RESOURCE r;  int i;
  MArrayForEach(meet->tasks, &task, &i)
    if( (rt==NULL || KheTaskResourceType(task)==rt) && KheTaskAsst(task)==NULL
	&& KheTaskIsPreassigned(task, as_in_event_resource, &r) )
    {
      if( !KheTaskAssignResource(task, r) )
	MAssert(false,"KheMeetAssignPreassignedResources internal error");
    }
}


/*****************************************************************************/
/*                                                                           */
/*  void KheMeetAddTask(KHE_MEET meet, KHE_TASK task, int *index_in_meet)    */
/*                                                                           */
/*  Add task to meet, setting *index_in_meet to its index.                   */
/*                                                                           */
/*  This function is not called when splitting or merging; it represents     */
/*  a genuine addition, so update the demand in meet and its ancestors.      */
/*                                                                           */
/*****************************************************************************/

void KheMeetAddTask(KHE_MEET meet, KHE_TASK task, int *index_in_meet)
{
  KHE_MEET prnt;
  MAssert(meet->time_domain != NULL,
    "KheMeetAddTask: meet has automatic domain");
  *index_in_meet = MArraySize(meet->tasks);
  MArrayAddLast(meet->tasks, task);
  for( prnt = meet;  prnt != NULL;  prnt = prnt->target_meet )
    prnt->all_demand += meet->duration;
}


/*****************************************************************************/
/*                                                                           */
/*  void KheMeetDeleteTask(KHE_MEET meet, int task_index)                    */
/*                                                                           */
/*  Delete from meet the soln resource at task_index.                        */
/*                                                                           */
/*  This function is not called when splitting or merging; it represents     */
/*  a genuine deletion, so update the demand in meet and its ancestors.      */
/*                                                                           */
/*****************************************************************************/

void KheMeetDeleteTask(KHE_MEET meet, int task_index)
{
  KHE_MEET prnt;  KHE_TASK task;  int i;
  for( i = task_index;  i < MArraySize(meet->tasks) - 1;  i++ )
  {
    task = MArrayGet(meet->tasks, i + 1);
    KheTaskSetIndexInMeet(task, i);
    MArrayPut(meet->tasks, i, task);
  }
  MArrayDropLast(meet->tasks);
  for( prnt = meet;  prnt != NULL;  prnt = prnt->target_meet )
    prnt->all_demand -= meet->duration;
}


/*****************************************************************************/
/*                                                                           */
/*  int KheMeetTaskCount(KHE_MEET meet)                                      */
/*                                                                           */
/*  Return the number of solution resources in meet.                         */
/*                                                                           */
/*****************************************************************************/

int KheMeetTaskCount(KHE_MEET meet)
{
  return MArraySize(meet->tasks);
}


/*****************************************************************************/
/*                                                                           */
/*  KHE_TASK KheMeetTask(KHE_MEET meet, int i)                               */
/*                                                                           */
/*  Return the i'th solution resource of meet.                               */
/*                                                                           */
/*****************************************************************************/

KHE_TASK KheMeetTask(KHE_MEET meet, int i)
{
  return MArrayGet(meet->tasks, i);
}


/*****************************************************************************/
/*                                                                           */
/*  bool KheMeetRetrieveTask(KHE_MEET meet, char *role, KHE_TASK *task)      */
/*                                                                           */
/*  Retrieve the first solution resource of meet whose event resource exists */
/*  and has the given role.                                                  */
/*                                                                           */
/*****************************************************************************/

bool KheMeetRetrieveTask(KHE_MEET meet, char *role, KHE_TASK *task)
{
  KHE_TASK task1;  KHE_EVENT_RESOURCE er;  int i;
  MArrayForEach(meet->tasks, &task1, &i)
  {
    er = KheTaskEventResource(task1);
    if( er != NULL && KheEventResourceRole(er) != NULL &&
	strcmp(KheEventResourceRole(er), role) == 0 )
    {
      *task = task1;
      return true;
    }
  }
  return false;
}


/*****************************************************************************/
/*                                                                           */
/* bool KheMeetFindTask(KHE_MEET meet, KHE_EVENT_RESOURCE er, KHE_TASK *task)*/
/*                                                                           */
/*  Similar to KheMeetRetrieveResource, but it searches for a soln           */
/*  resource with the given event resource, rather than for an event         */
/*  resource with the given role.                                            */
/*                                                                           */
/*****************************************************************************/

bool KheMeetFindTask(KHE_MEET meet, KHE_EVENT_RESOURCE er, KHE_TASK *task)
{
  KHE_TASK task1;  KHE_EVENT_RESOURCE er1;  int i;
  MArrayForEach(meet->tasks, &task1, &i)
  {
    er1 = KheTaskEventResource(task1);
    if( er1 == er )
    {
      *task = task1;
      return true;
    }
  }
  return false;
}


/*****************************************************************************/
/*                                                                           */
/*  bool KheMeetContainsResourcePreassignment(KHE_MEET meet, KHE_RESOURCE r, */
/*    bool as_in_event_resource, KHE_TASK *task)                             */
/*                                                                           */
/*  If meet contains a task preassigned r, return true and set *task to      */
/*  that task.                                                               */
/*                                                                           */
/*****************************************************************************/

bool KheMeetContainsResourcePreassignment(KHE_MEET meet, KHE_RESOURCE r,
  bool as_in_event_resource, KHE_TASK *task)
{
  KHE_TASK task1;  KHE_RESOURCE r1;  int i;
  MArrayForEach(meet->tasks, &task1, &i)
    if( KheTaskIsPreassigned(task1, as_in_event_resource, &r1) && r1 == r )
    {
      *task = task1;
      return true;
    }
  return false;
}


/*****************************************************************************/
/*                                                                           */
/*  bool KheMeetContainsResourceAssignment(KHE_MEET meet, KHE_RESOURCE r,    */
/*    KHE_TASK *task)                                                        */
/*                                                                           */
/*  If meet contains a task assigned r, return true and set *task to         */
/*  that task.                                                               */
/*                                                                           */
/*****************************************************************************/

bool KheMeetContainsResourceAssignment(KHE_MEET meet, KHE_RESOURCE r,
  KHE_TASK *task)
{
  KHE_TASK task1;  int i;
  MArrayForEach(meet->tasks, &task1, &i)
    if( KheTaskAsstResource(task1) == r )
    {
      *task = task1;
      return true;
    }
  return false;
}


/*****************************************************************************/
/*                                                                           */
/*  void KheMeetPartitionTaskCount(KHE_MEET meet, int offset,                */
/*    KHE_RESOURCE_GROUP partition, int *count)                              */
/*                                                                           */
/*  For each task running at the given offset in meet and in the meets       */
/*  assigned to it, directly or indirectly, whose domains lie in partition,  */
/*  add 1 to *count.                                                         */
/*                                                                           */
/*****************************************************************************/

void KheMeetPartitionTaskCount(KHE_MEET meet, int offset,
  KHE_RESOURCE_GROUP partition, int *count)
{
  KHE_TASK task;  int i, child_offset;  KHE_MEET child_meet;

  /* do the job for the tasks of meet itself */
  MArrayForEach(meet->tasks, &task, &i)
    if( KheResourceGroupPartition(KheTaskDomain(task)) == partition )
      (*count)++;

  /* do the job for the tasks of the overlapping meets assigned to meet */
  MArrayForEach(meet->assigned_meets, &child_meet, &i)
  {
    child_offset = KheMeetAsstOffset(child_meet);
    if( child_offset <= offset &&
        child_offset + KheMeetDuration(child_meet) > offset )
      KheMeetPartitionTaskCount(child_meet, offset - child_offset,
	partition, count);
  }
}


/*****************************************************************************/
/*                                                                           */
/*  void KheMeetPartitionTaskCount(KHE_MEET meet, int offset,                */
/*    KHE_RESOURCE_GROUP partition, int *count)                              */
/*                                                                           */
/*  For each task running at the given offset in meet and in the meets       */
/*  assigned to it, directly or indirectly, whose domains lie in partition,  */
/*  add 1 to *count.                                                         */
/*                                                                           */
/*****************************************************************************/

void KheMeetPartitionTaskDebug(KHE_MEET meet, int offset,
  KHE_RESOURCE_GROUP partition, int verbosity, int indent, FILE *fp)
{
  KHE_TASK task;  int i, child_offset;  KHE_MEET child_meet;

  /* do the job for the tasks of meet itself */
  MArrayForEach(meet->tasks, &task, &i)
    if( KheResourceGroupPartition(KheTaskDomain(task)) == partition )
      KheMeetDebug(meet, 1, indent, fp);

  /* do the job for the tasks of the overlapping meets assigned to meet */
  MArrayForEach(meet->assigned_meets, &child_meet, &i)
  {
    child_offset = KheMeetAsstOffset(child_meet);
    if( child_offset <= offset &&
        child_offset + KheMeetDuration(child_meet) > offset )
      KheMeetPartitionTaskDebug(child_meet, offset - child_offset,
	partition, verbosity, indent, fp);
  }
}


/*****************************************************************************/
/*                                                                           */
/*  Submodule "splitting and merging"                                        */
/*                                                                           */
/*****************************************************************************/

/*****************************************************************************/
/*                                                                           */
/*  bool KheMeetChildrenAllowSplit(KHE_MEET meet, int duration1)             */
/*                                                                           */
/*  Return true when there are no problems with the children if meet is      */
/*  split at duration1.                                                      */
/*                                                                           */
/*****************************************************************************/

static bool KheMeetChildrenAllowSplit(KHE_MEET meet, int duration1)
{
  KHE_MEET child_meet;  int i;
  if( DEBUG10 )
  {
    fprintf(stderr, "  [ KheMeetChildrenAllowSplit(");
    KheMeetDebug(meet, 1, -1, stderr);
    fprintf(stderr, ", %d)\n", duration1);
  }
  MArrayForEach(meet->assigned_meets, &child_meet, &i)
    if( child_meet->target_offset < duration1 &&
        child_meet->target_offset + child_meet->duration > duration1 )
    {
      if( DEBUG10 )
      {
	fprintf(stderr, "  ] KheMeetChildrenAllowSplit returning false: ");
	KheMeetDebug(meet, 1, 0, stderr);
      }
      return false;
    }
  if( DEBUG10 )
    fprintf(stderr, "  ] KheMeetChildrenAllowSplit returning true\n");
  return true;
}


/*****************************************************************************/
/*                                                                           */
/*  bool KheMeetSplitCheck(KHE_MEET meet, int duration1)                     */
/*                                                                           */
/*  Check whether meet can be split at duration1.                            */
/*                                                                           */
/*****************************************************************************/

bool KheMeetSplitCheck(KHE_MEET meet, int duration1, bool recursive)
{
  if( DEBUG10 )
  {
    fprintf(stderr, "[ KheMeetSplitCheck(");
    KheMeetDebug(meet, 1, -1, stderr);
    fprintf(stderr, ", %d)\n", duration1);
  }

  /* check duration */
  MAssert(duration1 > 0 && duration1 < meet->duration,
    "KheMeetSplitCheck: invalid duration1 value %d (meet duration is %d)",
    duration1, meet->duration);

  /* check children */
  if( !recursive && !KheMeetChildrenAllowSplit(meet, duration1) )
  {
    if( DEBUG10 )
      fprintf(stderr, "] KheMeetSplitCheck returning false (children)\n");
    return false;
  }

  /* all safe to split */
  if( DEBUG10 )
    fprintf(stderr, "] KheMeetSplitCheck returning true\n");
  return true;
}


/*****************************************************************************/
/*                                                                           */
/*  void KheChildMeetsDebug(KHE_MEET meet, int indent, FILE *fp)             */
/*                                                                           */
/*  Debug print of the child meets of meet.                                  */
/*                                                                           */
/*****************************************************************************/

/* ***
static void KheChildMeetsDebug(KHE_MEET meet, int indent, FILE *fp)
{
  KHE_MEET child_meet;  int i;
  MArrayForEach(meet->assigned_meets, &child_meet, &i)
  {
    fprintf(stderr, "%*s", indent, "");
    KheMeetDebug(child_meet, 1, -1, stderr);
    if( child_meet->target_meet != meet )
    {
      fprintf(stderr, " in unexpected meet ");
      if( child_meet->target_meet != NULL )
	KheMeetDebug(child_meet->target_meet, 1, -1, stderr);
      else
	fprintf(stderr, "-");
    }
    fprintf(stderr, " at offset %d\n", child_meet->target_offset);
    MAssert(child_meet->target_meet == meet,
      "KheChildMeetsDebug internal error");
  }
}
*** */


/*****************************************************************************/
/*                                                                           */
/*  void KheMeetSplitUnchecked(KHE_MEET meet, int duration1, bool recursive, */
/*    KHE_MEET *meet1, KHE_MEET *meet2)                                      */
/*                                                                           */
/*  Like KheMeetSplit below but don't check first; just do it.               */
/*                                                                           */
/*****************************************************************************/

static void KheMeetSplitUnchecked(KHE_MEET meet, int duration1, bool recursive,
  KHE_MEET *meet1, KHE_MEET *meet2)
{
  KHE_MEET res1, res2, child_meet, tmp;  int i;  KHE_TASK task, task2;
  KHE_MATCHING_SUPPLY_CHUNK sc;  KHE_ZONE zone;

  if( DEBUG9 )
  {
    fprintf(stderr, "[ KheMeetSplitUnchecked(");
    KheMeetDebug(meet, 1, -1, stderr);
    fprintf(stderr, "(durn %d), %d, *meet1, *meet2)\n", KheMeetDuration(meet),
      duration1);
    /* ***
    fprintf(stderr, "  child meets before split:\n");
    KheChildMeetsDebug(meet, 4, stderr);
    *** */
  }
  /* KheMeetCheckAsstInvt(meet); */

  /* if recursive, split any child meets that straddle the gap */
  if( recursive )
    MArrayForEach(meet->assigned_meets, &child_meet, &i)
      if( child_meet->target_offset < duration1 &&
	  child_meet->target_offset + child_meet->duration > duration1 )
	KheMeetSplitUnchecked(child_meet, duration1 - child_meet->target_offset,
	  recursive, &res1, &res2);

  /* make res1 (recycle meet) and res2 (new) */
  res1 = meet;
  MMake(res2);
  res2->back = meet->back;
  res2->soln = meet->soln;
  MAssert(meet->soln != NULL, "KheMeetSplitUnchecked internal error 1");
  KheSolnAddMeet(meet->soln, res2, &res2->index);
  if( DEBUG9 )
    fprintf(stderr, "  res1 #%d#, res2 #%d#\n", res1->index, res2->index);

  /* duration and time domain (res1's time domain remains unchanged) */
  res2->duration = res1->duration - duration1;
  res1->duration = duration1;
  res2->time_domain = res1->time_domain == NULL ? NULL :
    KheTimeGroupNeighbour(res1->time_domain, duration1);

  /* assigned time index */
  if( res1->assigned_time_index != -1 )
    res2->assigned_time_index = res1->assigned_time_index + duration1;
  else
    res2->assigned_time_index = -1;

  /* cycle meets */
  if( KheMeetIsCycleMeet(res1) )
    KheSolnCycleMeetSplit(res1->soln, res1, res2);

  /* assignment */
  if( res1->target_meet != NULL )
  {
    res2->target_meet = res1->target_meet;
    res2->target_offset = res1->target_offset + duration1;
    res2->target_index = MArraySize(res2->target_meet->assigned_meets);
    MArrayAddLast(res2->target_meet->assigned_meets, res2);
  }
  else
  {
    res2->target_meet = NULL;
    res2->target_offset = -1;
    res2->target_index = -1;
  }

  /* split the tasks (must come later than assignment!) */
  MArrayInit(res2->tasks);
  MArrayForEach(meet->tasks, &task, &i)
  {
    task2 = KheTaskSplit(task, duration1, res2, MArraySize(res2->tasks));
    MArrayAddLast(res2->tasks, task2);
  }

  /* distribute meet's assigned meets between res1 and res2 */
  /* this code also recalculates all_demand from scratch (important!) */
  res1->all_demand = res1->duration * MArraySize(res1->tasks);
  res2->all_demand = res2->duration * MArraySize(res2->tasks);
  MArrayInit(res2->assigned_meets);
  MArrayForEach(res1->assigned_meets, &child_meet, &i)
  {
    if( child_meet->target_offset + child_meet->duration <= duration1 )
    {
      /* child_meet is targeted entirely at the first fragment of meet */
      res1->all_demand += child_meet->all_demand;
    }
    else if( child_meet->target_offset >= duration1 )
    {
      /* child_meet is targeted entirely at the second fragment of meet */
      /* retarget child_meet to res2 */
      child_meet->target_meet = res2;
      child_meet->target_offset -= duration1;
      child_meet->target_index = MArraySize(res2->assigned_meets);
      MArrayAddLast(res2->assigned_meets, child_meet);
      res2->all_demand += child_meet->all_demand;
      tmp = MArrayRemoveLast(res1->assigned_meets);
      if( tmp != child_meet )
      {
	/* fill the hole left behind by child_meet */
	tmp->target_index = i;
	MArrayPut(res1->assigned_meets, i, tmp);
      }
      i--;
    }
    else
    {
      /* child_meet straddles both fragments of meet */
      /* this case cannot occur because of earlier recursive splits */
      MAssert(false, "KheMeetSplit internal error");
    }
    /* KheMeetCheckAsstInvt(child_meet); */
  }

  /* node and visit num */
  res2->node = res1->node;
  if( res2->node != NULL )
    KheNodeAddSplitMeet(res2->node, res2);
  res2->visit_num = res1->visit_num;

  /* event_in_soln */
  res2->event_in_soln = res1->event_in_soln;
  if( res1->event_in_soln != NULL )
    KheEventInSolnSplitMeet(res1->event_in_soln, res1, res2);
  res2->copy = NULL;

  /* zones */
  MArrayInit(res2->zones);
  for( i = duration1;  i < MArraySize(res1->zones);  i++ )
  {
    zone = MArrayGet(res1->zones, i);
    MArrayAddLast(res2->zones, zone);
    if( zone != NULL )
      KheZoneUpdateMeetOffset(zone, res1, i, res2, i - duration1);
  }
  MArrayDropFromEnd(res1->zones, MArraySize(res1->zones) - duration1);

  /* supply and demand chunks */
  MArrayInit(res2->supply_chunks);
  MArrayInit(res2->demand_chunks);
  MAssert(MArraySize(res1->supply_chunks) == MArraySize(res1->demand_chunks),
    "KheMeetSplitUnchecked internal error");
  for( i = duration1;  i < MArraySize(res1->supply_chunks);  i++ )
  {
    sc = MArrayGet(res1->supply_chunks, i);
    KheMatchingSupplyChunkSetImpl(sc, (void *) res2);
    MArrayAddLast(res2->supply_chunks, sc);
    MArrayAddLast(res2->demand_chunks, MArrayGet(res1->demand_chunks, i));
  }
  MArrayDropFromEnd(res1->supply_chunks,
    MArraySize(res1->supply_chunks) - duration1);
  MArrayDropFromEnd(res1->demand_chunks,
    MArraySize(res1->demand_chunks) - duration1);

  /* set result variables, inform soln that this has happened, and return */
  /* KheMeetCheckAsstInvt(res1); */
  /* KheMeetCheckAsstInvt(res2); */
  *meet1 = res1;
  *meet2 = res2;
  KheSolnOpMeetSplit(res1->soln, res1, res2);

  if( DEBUG9 )
  {
    /* ***
    fprintf(stderr, "  child meets of *meet1 after split:\n");
    KheChildMeetsDebug(*meet1, 4, stderr);
    fprintf(stderr, "  child meets of *meet2 after split:\n");
    KheChildMeetsDebug(*meet2, 4, stderr);
    *** */
    fprintf(stderr, "] returning (meet1 durn %d, meet2 durn %d)\n",
      KheMeetDuration(*meet1), KheMeetDuration(*meet2));
  }
}


/*****************************************************************************/
/*                                                                           */
/*  bool KheMeetSplit(KHE_MEET meet, int duration1, bool recursive,          */
/*    KHE_MEET *meet1, KHE_MEET *meet2)                                      */
/*                                                                           */
/*  Split meet into *meet1 and *meet2 at duration1.                          */
/*                                                                           */
/*****************************************************************************/

bool KheMeetSplit(KHE_MEET meet, int duration1, bool recursive,
  KHE_MEET *meet1, KHE_MEET *meet2)
{
  if( !KheMeetSplitCheck(meet, duration1, recursive) )
    return false;
  KheMeetSplitUnchecked(meet, duration1, recursive, meet1, meet2);
  return true;
}


/*****************************************************************************/
/*                                                                           */
/*  bool KheMeetMergeCheck(KHE_MEET meet1, KHE_MEET meet2)                   */
/*                                                                           */
/*  Check whether meet1 and meet2 can be merged.                             */
/*                                                                           */
/*****************************************************************************/

bool KheMeetMergeCheck(KHE_MEET meet1, KHE_MEET meet2)
{
  int i, j;  KHE_TASK task1, task2, tmp;

  /* meets must be distinct */
  if( meet1 == meet2 )
    return false;

  /* same back pointers */
  if( meet1->back != meet2->back )
    return false;

  /* must both be cycle meets, or both not be cycle meets */
  if( KheMeetIsCycleMeet(meet1) != KheMeetIsCycleMeet(meet2) )
    return false;

  /* must be derived from the same instance event, or both from none */
  if( meet1->event_in_soln != meet2->event_in_soln )
    return false;

  /* assignments must be compatible */
  if( KheMeetIsCycleMeet(meet1) )
  {
    /* cycle meets must have compatible assigned time indexes */
    if( meet1->assigned_time_index + meet1->duration !=
	meet2->assigned_time_index &&
        meet2->assigned_time_index + meet2->duration !=
	meet1->assigned_time_index )
      return false;
  }
  else
  {
    /* non-cycle meets must have compatible target meets and offsets */
    if( meet1->target_meet != meet2->target_meet )
      return false;
    if( meet1->target_meet != NULL )
    {
      if( meet1->target_offset <= meet2->target_offset )
      {
	if( meet1->target_offset + meet1->duration != meet2->target_offset )
	  return false;
      }
      else
      {
	if( meet2->target_offset + meet2->duration != meet1->target_offset )
	  return false;
      }
    }
  }

  /* same node */
  if( meet1->node != meet2->node )
    return false;

  /* tasks must be mergeable, when meet2's are reordered */
  if( MArraySize(meet1->tasks) != MArraySize(meet2->tasks) )
    return false;
  MArrayForEach(meet1->tasks, &task1, &i)
  {
    for( j = i;  j < MArraySize(meet2->tasks);  j++ )
    {
      task2 = MArrayGet(meet2->tasks, j);
      if( KheTaskMergeCheck(task1, task2) )
      {
	if( i != j )
	{
	  MArraySwap(meet2->tasks, i, j, tmp);
	  KheTaskSetIndexInMeet(MArrayGet(meet2->tasks, i), i);
	  KheTaskSetIndexInMeet(MArrayGet(meet2->tasks, j), j);
	}
	break;
      }
    }
    if( j == MArraySize(meet2->tasks) )
      return false;
  }

  /* no problems, so they are mergeable */
  return true;
}


/*****************************************************************************/
/*                                                                           */
/*  bool KheMeetMerge(KHE_MEET meet1, KHE_MEET meet2, KHE_MEET *meet)        */
/*                                                                           */
/*  Merge meet1 and meet2 into *meet.                                        */
/*                                                                           */
/*****************************************************************************/

bool KheMeetMerge(KHE_MEET meet1, KHE_MEET meet2, KHE_MEET *meet)
{
  KHE_MEET tmp, child_meet;  int i, init_meet1_durn;  KHE_ZONE zone;
  KHE_TASK task1, task2;  KHE_MATCHING_SUPPLY_CHUNK sc;

  /* check mergable and make sure meet1 is the first one */
  /* KheMeetCheckAsstInvt(meet1); */
  /* KheMeetCheckAsstInvt(meet2); */
  if( !KheMeetMergeCheck(meet1, meet2) )
    return false;
  if( KheMeetIsCycleMeet(meet1) )
  {
    /* cycle meet: swap if required, *then* inform soln */
    if( meet1->assigned_time_index > meet2->assigned_time_index )
      tmp = meet1, meet1 = meet2, meet2 = tmp;
    KheSolnCycleMeetMerge(meet1->soln, meet1, meet2);
  }
  else
  {
    /* non-cycle meet:  swap if required */
    if( meet1->target_offset > meet2->target_offset )  /* works even if -1 */
      tmp = meet1, meet1 = meet2, meet2 = tmp;
  }

  /* inform soln that this is happening */
  KheSolnOpMeetMerge(meet1->soln);

  /* merge the tasks of meet2 (already reordered) into those of meet1 */
  MArrayForEach(meet1->tasks, &task1, &i)
  {
    task2 = MArrayGet(meet2->tasks, i);
    KheTaskMerge(task1, task2);
  }
  MArrayClear(meet2->tasks);

  /* inform the event monitor about the merge */
  if( meet2->event_in_soln != NULL )
    KheEventInSolnMergeMeet(meet1->event_in_soln, meet1, meet2);

  /* fix meet1's duration, current time domain, and workload */
  init_meet1_durn = meet1->duration;
  meet1->duration += meet2->duration;

  /* retarget everything assigned to meet2 to be assigned to meet1 */
  MArrayForEach(meet2->assigned_meets, &child_meet, &i)
  {
    child_meet->target_meet = meet1;
    child_meet->target_offset += init_meet1_durn;
    child_meet->target_index = MArraySize(meet1->assigned_meets);
    MArrayAddLast(meet1->assigned_meets, child_meet);
    /* KheMeetCheckAsstInvt(child_meet); */
  }

  /* demand */
  meet1->all_demand += meet2->all_demand;

  /* move meet2's zones to meet1 */
  MArrayForEach(meet2->zones, &zone, &i)
  {
    MArrayAddLast(meet1->zones, zone);
    if( zone != NULL )
      KheZoneUpdateMeetOffset(zone, meet2, i, meet1, init_meet1_durn + i);
  }
  MArrayClear(meet2->zones);

  /* move meet2's supply and demand chunks to meet1 */
  MArrayForEach(meet2->supply_chunks, &sc, &i)
  {
    KheMatchingSupplyChunkSetImpl(sc, (void *) meet1);
    MArrayAddLast(meet1->supply_chunks, sc);
  }
  MArrayClear(meet2->supply_chunks);
  MArrayAppend(meet1->demand_chunks, meet2->demand_chunks, i);
  MArrayClear(meet2->demand_chunks);

  /* remove meet2 from its node, if any */
  if( meet2->node != NULL )
    KheNodeDeleteSplitMeet(meet2->node, meet2);

  /* remove meet from its parent and fill any hole left behind */
  if( meet2->target_meet != NULL )
  {
    tmp = MArrayRemoveLast(meet2->target_meet->assigned_meets);
    if( meet2 != tmp )
    {
      tmp->target_index = meet2->target_index;
      MArrayPut(meet2->target_meet->assigned_meets, meet2->target_index, tmp);
    }
  }

  /* get rid of meet2 */
  KheSolnDeleteMeet(meet1->soln, meet2);
  KheMeetFree(meet2);

  /* return meet1 */
  /* KheMeetCheckAsstInvt(meet1); */
  *meet = meet1;
  return true;
}


/*****************************************************************************/
/*                                                                           */
/*  Submodule "time assignment"                                              */
/*                                                                           */
/*****************************************************************************/

/*****************************************************************************/
/*                                                                           */
/*  bool KheMeetAssignTimeCheck(KHE_MEET meet, KHE_TIME time)                */
/*                                                                           */
/*  Check whether meet can be assigned to time.                              */
/*                                                                           */
/*****************************************************************************/

bool KheMeetAssignTimeCheck(KHE_MEET meet, KHE_TIME time)
{
  KHE_MEET target_meet;  int target_offset;
  target_meet = KheSolnTimeCycleMeet(meet->soln, time);
  target_offset = KheSolnTimeCycleMeetOffset(meet->soln, time);
  return KheMeetAssignCheck(meet, target_meet, target_offset);
  /* ***
  target_meet = KheSolnMeet(meet->soln, KheTimeCycleMeetIndex(time));
  return KheMeetAssignCheck(meet, target_meet, KheTimeCycleMeetOffset(time));
  *** */
}


/*****************************************************************************/
/*                                                                           */
/*  bool KheMeetAssignTime(KHE_MEET meet, KHE_TIME time)                     */
/*                                                                           */
/*  Assign meet to time.                                                     */
/*                                                                           */
/*****************************************************************************/

bool KheMeetAssignTime(KHE_MEET meet, KHE_TIME time)
{
  KHE_MEET target_meet;  int target_offset;  bool res;
  if( DEBUG2 )
  {
    KHE_EVENT e = KheMeetEvent(meet);
    fprintf(stderr, "[ KheMeetAssignTime(%s, %s)\n",
      e != NULL && KheEventId(e) != NULL ? KheEventId(e) : "-",
      KheTimeId(time) != NULL ? KheTimeId(time) : "-");
  }
  /* ***
  target_meet = KheSolnMeet(meet->soln, KheTimeCycleMeetIndex(time));
  res = KheMeetAssign(meet, target_meet, KheTimeCycleMeetOffset(time));
  *** */
  target_meet = KheSolnTimeCycleMeet(meet->soln, time);
  target_offset = KheSolnTimeCycleMeetOffset(meet->soln, time);
  res = KheMeetAssign(meet, target_meet, target_offset);
  if( DEBUG2 )
    fprintf(stderr, "] KheMeetAssignTime returning %s\n",
      res ? "true" : "false");
  return res;
}


/*****************************************************************************/
/*                                                                           */
/*  void KheMeetUnAssignTime(KHE_MEET meet)                                  */
/*                                                                           */
/*  Unassign meet from whatever time it is assigned to.                      */
/*                                                                           */
/*****************************************************************************/

void KheMeetUnAssignTime(KHE_MEET meet)
{
  KheMeetUnAssign(meet);
}


/*****************************************************************************/
/*                                                                           */
/*  KHE_TIME KheMeetAsstTime(KHE_MEET meet)                                  */
/*                                                                           */
/*  Return the time that meet is assigned to, or NULL if none.               */
/*                                                                           */
/*****************************************************************************/

KHE_TIME KheMeetAsstTime(KHE_MEET meet)
{
  if( meet->assigned_time_index == -1 )
    return NULL;
  else
    return KheInstanceTime(KheSolnInstance(meet->soln), meet->assigned_time_index);
}


/*****************************************************************************/
/*                                                                           */
/*  Submodule "general assignment"                                           */
/*                                                                           */
/*****************************************************************************/

/*****************************************************************************/
/*                                                                           */
/*  bool KheMeetAssignCheck(KHE_MEET meet, KHE_MEET target_meet,             */
/*    int target_offset)                                                     */
/*                                                                           */
/*  Check whether meet can be assigned to target_meet at target_offset.      */
/*                                                                           */
/*****************************************************************************/
static bool KheMeetDomainAllowsAssignment(KHE_MEET meet,
  KHE_TIME_GROUP target_domain, int target_offset);

bool KheMeetAssignCheck(KHE_MEET meet, KHE_MEET target_meet, int target_offset)
{
  KHE_MEET anc;  int anc_offset;
  if( DEBUG1 )
    fprintf(stderr, "[ KheMeetAssignCheck(meet d%d, target_meet d%d, o%d)\n",
      meet->duration, target_meet->duration, target_offset);

  /* meet must not be currently assigned */
  MAssert(meet->target_meet == NULL, "KheMeetAssignCheck: meet is assigned");

  /* meet must not be a cycle meet */
  if( meet->assigned_time_index != -1 )
  {
    if( DEBUG1 )
      fprintf(stderr, "] KheMeetAssignCheck false (cycle meet)\n");
    return false;
  }

  /* target_offset must be in range */
  if( target_offset < 0 || target_offset + meet->duration > target_meet->duration )
  {
    if( DEBUG1 )
      fprintf(stderr, "] KheMeetAssignCheck false (offset)\n");
    return false;
  }

  /* node rule must not be violated */
  if( meet->node != NULL )
  {
    if( KheNodeParent(meet->node) == NULL ||
	KheNodeParent(meet->node) != target_meet->node )
    {
      if( DEBUG1 )
	fprintf(stderr, "] KheMeetAssignCheck false (node rule)\n");
      return false;
    }
  }

  /* time domains must match */
  anc = target_meet;  anc_offset = target_offset;
  while( anc != NULL && anc->time_domain == NULL )
  {
    anc_offset += anc->target_offset;
    anc = anc->target_meet;
  };
  if( anc != NULL &&
      !KheMeetDomainAllowsAssignment(meet, anc->time_domain, anc_offset) )
  {
    if( DEBUG1 )
    {
      fprintf(stderr, "] KheMeetAssignCheck false (domains)\n");
      fprintf(stderr, "  meet->time_domain: ");
      if( meet->time_domain == NULL )
	fprintf(stderr, "NULL");
      else
	KheTimeGroupDebug(meet->time_domain, 3, 0, stderr);
      fprintf(stderr, "  anc->time_domain: ");
      KheTimeGroupDebug(anc->time_domain, 3, 0, stderr);
      fprintf(stderr, "  anc_offset %d\n", anc_offset);
    }
    return false;
  }

  /* no problems, allow the assignment */
  if( DEBUG1 )
    fprintf(stderr, "] KheMeetAssignCheck returning true\n");
  return true;
}


/*****************************************************************************/
/*                                                                           */
/*  void KheMeetDoAssignTime(KHE_MEET meet)                                  */
/*                                                                           */
/*  Inform meet and all its descendants and their constraints that meet's    */
/*  parent has been assigned a time.                                         */
/*                                                                           */
/*****************************************************************************/

static void KheMeetDoAssignTime(KHE_MEET meet)
{
  KHE_MEET child_meet;  int i;
  KHE_TASK task;  /* KHE_RESOURCE r;  KHE_RESOURCE_IN_SOLN rs; */
  meet->assigned_time_index =
    meet->target_meet->assigned_time_index + meet->target_offset;
  if( DEBUG6 )
  {
    KHE_TIME t;
    t = KheInstanceTime(KheSolnInstance(meet->soln), meet->assigned_time_index);
    fprintf(stderr, "     [ KheMeetDoAssignTime(");
    KheMeetDebug(meet, 1, -1, stderr);
    fprintf(stderr, ", %s)\n", KheTimeId(t) == NULL ? "-" : KheTimeId(t));
  }
  if( meet->event_in_soln != NULL )
    KheEventInSolnAssignTime(meet->event_in_soln, meet,
      meet->assigned_time_index);
  MArrayForEach(meet->tasks, &task, &i)
    KheTaskAssignTime(task, meet->assigned_time_index);
  MArrayForEach(meet->assigned_meets, &child_meet, &i)
    KheMeetDoAssignTime(child_meet);
  if( DEBUG6 )
    fprintf(stderr, "     ] KheMeetDoAssignTime returning\n");
}


/*****************************************************************************/
/*                                                                           */
/*  void KheMeetDoUnAssignTime(KHE_MEET meet)                                */
/*                                                                           */
/*  Inform meet and all its descendants and their constraints that meet's    */
/*  time assignment is being removed.                                        */
/*                                                                           */
/*****************************************************************************/

static void KheMeetDoUnAssignTime(KHE_MEET meet)
{
  KHE_MEET child_meet;  int i;
  KHE_TASK task;  /* KHE_RESOURCE r; KHE_RESOURCE_IN_SOLN rs; */
  if( meet->event_in_soln != NULL )
    KheEventInSolnUnAssignTime(meet->event_in_soln, meet,
      meet->assigned_time_index);
  MArrayForEach(meet->tasks, &task, &i)
    KheTaskUnAssignTime(task, meet->assigned_time_index);
  MArrayForEach(meet->assigned_meets, &child_meet, &i)
    KheMeetDoUnAssignTime(child_meet);
  meet->assigned_time_index = -1;
}


/*****************************************************************************/
/*                                                                           */
/*  void KheMeetChangeDemandChunkBases(KHE_MEET meet,                        */
/*    KHE_MEET ancestor_meet, int ancestor_offset)                           */
/*                                                                           */
/*  Change the bases of the demand chunks of meet so that they are linked    */
/*  to the supply chunks of ancestor_meet, starting at ancestor_offset.  Do  */
/*  this for all meets assigned to meet, recursively, as well.               */
/*                                                                           */
/*****************************************************************************/

static void KheMeetChangeDemandChunkBases(KHE_MEET meet,
  KHE_MEET ancestor_meet, int ancestor_offset)
{
  KHE_MATCHING_DEMAND_CHUNK dc;  int i;
  KHE_MATCHING_SUPPLY_CHUNK sc;  KHE_MEET child_meet;
  MArrayForEach(meet->demand_chunks, &dc, &i)
  {
    sc = MArrayGet(ancestor_meet->supply_chunks, ancestor_offset + i);
    KheMatchingDemandChunkSetBase(dc, KheMatchingSupplyChunkBase(sc));
  }
  MArrayForEach(meet->assigned_meets, &child_meet, &i)
    KheMeetChangeDemandChunkBases(child_meet,
      ancestor_meet, ancestor_offset + child_meet->target_offset);
}


/*****************************************************************************/
/*                                                                           */
/* bool KheMeetAssign(KHE_MEET meet, KHE_MEET target_meet, int target_offset)*/
/*                                                                           */
/*  Assign meet to target_meet at target_offset.                             */
/*                                                                           */
/*****************************************************************************/

bool KheMeetAssign(KHE_MEET meet, KHE_MEET target_meet, int target_offset)
{
  KHE_MEET ancestor_meet;  int ancestor_offset;  /* int i; */
  KHE_MATCHING_TYPE mt;

  /* check safe to proceed */
  if( !KheMeetAssignCheck(meet, target_meet, target_offset) )
    return false;

  /* inform soln that this is happening */
  KheSolnOpMeetAssign(meet->soln, meet, target_meet, target_offset);

  /* add meet's layers and demand to all its new ancestors at all offsets */
  ancestor_offset = target_offset;
  ancestor_meet = target_meet;
  /* ***
  for( i = 0;  i < meet->duration;  i++ )
    LSetUnion(&MArrayGet(ancestor_meet->all_layers, ancestor_offset + i),
      MArrayGet(meet->all_layers, i));
  *** */
  ancestor_meet->all_demand += meet->all_demand;
  while( ancestor_meet->target_meet != NULL )
  {
    ancestor_offset += ancestor_meet->target_offset;
    ancestor_meet = ancestor_meet->target_meet;
    /* ***
    for( i = 0;  i < meet->duration;  i++ )
      LSetUnion(&MArrayGet(ancestor_meet->all_layers, ancestor_offset + i),
	MArrayGet(meet->all_layers, i));
    *** */
    ancestor_meet->all_demand += meet->all_demand;
  }

  /* record the assignment */
  meet->target_meet = target_meet;
  meet->target_offset = target_offset;
  meet->target_index = MArraySize(target_meet->assigned_meets);
  MArrayAddLast(target_meet->assigned_meets, meet);

  /* if required, change the bases of all descendants' demand chunks */
  mt = KheSolnMatchingType(meet->soln);
  if( mt == KHE_MATCHING_TYPE_SOLVE || mt == KHE_MATCHING_TYPE_EVAL_TIMES )
    KheMeetChangeDemandChunkBases(meet, ancestor_meet, ancestor_offset);

  /* if assigning a time, inform descendants of meet */
  if( target_meet->assigned_time_index != -1 )
    KheMeetDoAssignTime(meet);
  if( DEBUG8 )
  {
    fprintf(stderr, "  KheMeetAssign(");
    KheMeetDebug(meet, 1, -1, stderr);
    fprintf(stderr, ", ");
    KheMeetDebug(target_meet, 1, -1, stderr);
    fprintf(stderr, ", %d)\n", target_offset);
  }
  /* KheMeetCheckAsstInvt(meet); */
  return true;
}


/*****************************************************************************/
/*                                                                           */
/*  void KheMeetUnAssign(KHE_MEET meet)                                      */
/*                                                                           */
/*  Unassign meet from whatever it is assigned to, if anything.              */
/*                                                                           */
/*****************************************************************************/

void KheMeetUnAssign(KHE_MEET meet)
{
  KHE_MEET tmp, ancestor_meet;  int ancestor_offset;  /* int i; */
  KHE_MATCHING_TYPE mt;

  if( DEBUG8 )
  {
    fprintf(stderr, "  KheMeetUnAssign(");
    KheMeetDebug(meet, 1, -1, stderr);
    fprintf(stderr, ")\n");
  }
  /* KheMeetCheckAsstInvt(meet); */

  MAssert(meet->target_meet != NULL, "KheMeetUnAssign: meet is not assigned");

  /* inform soln that this is happening */
  KheSolnOpMeetUnAssign(meet->soln, meet, meet->target_meet,
    meet->target_offset);

  /* if unassigning a time, inform descendants of meet */
  if( meet->target_meet->assigned_time_index != -1 )
    KheMeetDoUnAssignTime(meet);

  /* if required, change the bases of all descendants' demand chunks */
  mt = KheSolnMatchingType(meet->soln);
  if( mt == KHE_MATCHING_TYPE_SOLVE || mt == KHE_MATCHING_TYPE_EVAL_TIMES )
    KheMeetChangeDemandChunkBases(meet, meet, 0);

  /* remove meet's layers and demand from all its ancestors at all offsets */
  ancestor_offset = meet->target_offset;
  ancestor_meet = meet->target_meet;
  /* ***
  for( i = 0;  i < meet->duration;  i++ )
    LSetDifference(MArrayGet(ancestor_meet->all_layers, ancestor_offset + i),
      MArrayGet(meet->all_layers, i));
  *** */
  ancestor_meet->all_demand -= meet->all_demand;
  while( ancestor_meet->target_meet != NULL )
  {
    ancestor_offset += ancestor_meet->target_offset;
    ancestor_meet = ancestor_meet->target_meet;
    /* ***
    for( i = 0;  i < meet->duration;  i++ )
      LSetDifference(MArrayGet(ancestor_meet->all_layers, ancestor_offset + i),
	MArrayGet(meet->all_layers, i));
    *** */
    ancestor_meet->all_demand -= meet->all_demand;
  }

  /* remove meet from its parent and fill any hole left behind */
  MAssert(MArrayGet(meet->target_meet->assigned_meets,meet->target_index)==meet,
    "KheMeetUnAssign internal error");
  tmp = MArrayRemoveLast(meet->target_meet->assigned_meets);
  if( meet != tmp )
  {
    tmp->target_index = meet->target_index;
    MArrayPut(meet->target_meet->assigned_meets, meet->target_index, tmp);
  }
  /* KheMeetCheckAsstInvt(meet->target_meet); */
  meet->target_meet = NULL;
  meet->target_offset = -1;
  meet->target_index = -1;
  /* KheMeetCheckAsstInvt(meet); */
}


/*****************************************************************************/
/*                                                                           */
/*  KHE_MEET KheMeetAsst(KHE_MEET meet)                                      */
/*                                                                           */
/*  Return the assignment of meet, or NULL if none.                          */
/*                                                                           */
/*****************************************************************************/

KHE_MEET KheMeetAsst(KHE_MEET meet)
{
  return meet->target_meet;
}


/*****************************************************************************/
/*                                                                           */
/*  int KheMeetAsstOffset(KHE_MEET meet)                                     */
/*                                                                           */
/*  Return the assignment offset of meet, or NULL if none.                   */
/*                                                                           */
/*****************************************************************************/

int KheMeetAsstOffset(KHE_MEET meet)
{
  return meet->target_offset;
}


/*****************************************************************************/
/*                                                                           */
/*  int KheMeetAssignedToCount(KHE_MEET target_meet)                         */
/*                                                                           */
/*  Return the number of meets assigned to target_meet.                      */
/*                                                                           */
/*****************************************************************************/

int KheMeetAssignedToCount(KHE_MEET target_meet)
{
  return MArraySize(target_meet->assigned_meets);
}


/*****************************************************************************/
/*                                                                           */
/*  KHE_MEET KheMeetAssignedTo(KHE_MEET target_meet, int i)                  */
/*                                                                           */
/*  Return the i'th meet assigned to target_meet.                            */
/*                                                                           */
/*****************************************************************************/

KHE_MEET KheMeetAssignedTo(KHE_MEET target_meet, int i)
{
  return MArrayGet(target_meet->assigned_meets, i);
}


/*****************************************************************************/
/*                                                                           */
/*  bool KheMeetIsCycleMeet(KHE_MEET meet)                                   */
/*                                                                           */
/*  Return true if meet is a cycle meet.                                     */
/*                                                                           */
/*  Implementation note.  To save a boolean field, we don't store this       */
/*  condition explicitly in meet.  Instead, we use the fact that the only    */
/*  meets that can have an assigned time index without being                 */
/*  assigned to something else are cycle meets.                              */
/*                                                                           */
/*****************************************************************************/

bool KheMeetIsCycleMeet(KHE_MEET meet)
{
  return meet->target_meet == NULL && meet->assigned_time_index != -1;
}


/*****************************************************************************/
/*                                                                           */
/*  KHE_MEET KheMeetRoot(KHE_MEET meet, int *offset_in_root)                 */
/*                                                                           */
/*  Return the root of meet, setting *offset_in_root to its offset.          */
/*                                                                           */
/*****************************************************************************/

KHE_MEET KheMeetRoot(KHE_MEET meet, int *offset_in_root)
{
  *offset_in_root = 0;
  while( meet->target_meet != NULL )
  {
    *offset_in_root += meet->target_offset;
    meet = meet->target_meet;
  }
  return meet;
}


/*****************************************************************************/
/*                                                                           */
/*  bool KheMeetOverlap(KHE_MEET meet1, KHE_MEET meet2)                      */
/*                                                                           */
/*  Return true if meet1 and meet2 overlap in time.                          */
/*                                                                           */
/*****************************************************************************/

bool KheMeetOverlap(KHE_MEET meet1, KHE_MEET meet2)
{
  KHE_MEET root1, root2;  int offset1, offset2;
  root1 = KheMeetRoot(meet1, &offset1);
  root2 = KheMeetRoot(meet2, &offset2);
  if( root1 != root2 )
    return false;
  if( offset1 + meet1->duration <= offset2 )
    return false;
  if( offset2 + meet2->duration <= offset1 )
    return false;
  return true;
}


/*****************************************************************************/
/*                                                                           */
/*  KHE_MEET KheMeetLeader(KHE_MEET meet, int *offset_in_leader)             */
/*                                                                           */
/*  Return the leader of meet, setting *offset_in_leader to its offset.      */
/*  The leader meet is the first non-cycle meet on the chain of assignments  */
/*  leading out of meet which lies in a node.                                */
/*                                                                           */
/*****************************************************************************/

KHE_MEET KheMeetLeader(KHE_MEET meet, int *offset_in_leader)
{
  *offset_in_leader = 0;
  while( meet->node == NULL && meet->target_meet != NULL )
  {
    *offset_in_leader += meet->target_offset;
    meet = meet->target_meet;
  }
  return meet->node != NULL && !KheMeetIsCycleMeet(meet) ? meet : NULL;
}


/*****************************************************************************/
/*                                                                           */
/*  Submodule "moving and swapping"                                          */
/*                                                                           */
/*****************************************************************************/

/*****************************************************************************/
/*                                                                           */
/*  void KheMeetAssignDebug(char *op, KHE_MEET meet, KHE_MEET target_meet,   */
/*    int target_offset)                                                     */
/*                                                                           */
/*  Generate a debug print of op(meet, target_meet, target_offset).          */
/*                                                                           */
/*****************************************************************************/

static void KheMeetAssignDebug(char *op, KHE_MEET meet, KHE_MEET target_meet,
  int target_offset)
{
  fprintf(stderr, "%s(", op);
  KheMeetDebug(meet, 1, -1, stderr);
  fprintf(stderr, ", ");
  if( target_meet != NULL )
  {
    KheMeetDebug(target_meet, 1, -1, stderr);
    fprintf(stderr, ", %d)", target_offset);
  }
  else
    fprintf(stderr, "-, -");
}


/*****************************************************************************/
/*                                                                           */
/*  bool KheMeetMoveCheck(KHE_MEET meet, KHE_MEET target_meet,               */
/*    int target_offset)                                                     */
/*                                                                           */
/*  Check whether moving meet to target_meet at target_offset is possible.   */
/*                                                                           */
/*****************************************************************************/

bool KheMeetMoveCheck(KHE_MEET meet, KHE_MEET target_meet, int target_offset)
{
  KHE_MEET meet_target_meet;  int meet_target_offset;  bool res;
  if( DEBUG13 )
    KheMeetAssignDebug("[ KheMeetMoveCheck", meet, target_meet, target_offset);
  if( target_meet == NULL )
    res = (meet->target_meet != NULL);
  else if( meet->target_meet == NULL )
    res = KheMeetAssignCheck(meet, target_meet, target_offset);
  else if(meet->target_meet==target_meet && meet->target_offset==target_offset)
    res = false;
  else
  {
    meet_target_meet = meet->target_meet;
    meet_target_offset = meet->target_offset;
    KheMeetUnAssign(meet);
    res = KheMeetAssignCheck(meet, target_meet, target_offset);
    KheMeetAssign(meet, meet_target_meet, meet_target_offset);
  }
  if( DEBUG13 )
    fprintf(stderr, "] KheMeetMoveCheck returning %s\n", res ? "true":"false");
  return res;
}


/*****************************************************************************/
/*                                                                           */
/*  bool KheMeetMove(KHE_MEET meet, KHE_MEET target_meet, int target_offset) */
/*                                                                           */
/*  If possible, move meet to target_meet at target_offset.                  */
/*                                                                           */
/*****************************************************************************/

bool KheMeetMove(KHE_MEET meet, KHE_MEET target_meet, int target_offset)
{
  KHE_MEET meet_target_meet;  int meet_target_offset;  bool res;
  if( DEBUG13 )
    KheMeetAssignDebug("[ KheMeetMove", meet, target_meet, target_offset);
  if( target_meet == NULL )
  {
    if( meet->target_meet == NULL )
      res = false;
    else
    {
      KheMeetUnAssign(meet);
      res = true;
    }
  }
  else if( meet->target_meet == NULL )
    res = KheMeetAssign(meet, target_meet, target_offset);
  else if(target_meet==meet->target_meet && target_offset==meet->target_offset)
    res = false;
  else
  {
    meet_target_meet = meet->target_meet;
    meet_target_offset = meet->target_offset;
    KheMeetUnAssign(meet);
    if( KheMeetAssign(meet, target_meet, target_offset) )
      res = true;
    else
    {
      KheMeetAssign(meet, meet_target_meet, meet_target_offset);
      res = false;
    }
  }
  if( DEBUG13 )
    fprintf(stderr, "] KheMeetMove returning %s\n", res ? "true" : "false");
  return res;
}


/*****************************************************************************/
/*                                                                           */
/*  bool KheMeetMoveTimeCheck(KHE_MEET meet, KHE_TIME t)                     */
/*                                                                           */
/*  Check whether meet can be moved to t.                                    */
/*                                                                           */
/*****************************************************************************/

bool KheMeetMoveTimeCheck(KHE_MEET meet, KHE_TIME t)
{
  KHE_MEET target_meet;  int target_offset;
  target_meet = KheSolnTimeCycleMeet(meet->soln, t);
  target_offset = KheSolnTimeCycleMeetOffset(meet->soln, t);
  return KheMeetMoveCheck(meet, target_meet, target_offset);
  /* ***
  target_meet = KheSolnMeet(meet->soln, KheTimeCycleMeetIndex(t));
  return KheMeetMoveCheck(meet, target_meet, KheTimeCycleMeetOffset(t));
  *** */
}


/*****************************************************************************/
/*                                                                           */
/*  bool KheMeetMoveTime(KHE_MEET meet, KHE_TIME t)                          */
/*                                                                           */
/*  Move meet to t.                                                          */
/*                                                                           */
/*****************************************************************************/

bool KheMeetMoveTime(KHE_MEET meet, KHE_TIME t)
{
  KHE_MEET target_meet;  int target_offset;
  target_meet = KheSolnTimeCycleMeet(meet->soln, t);
  target_offset = KheSolnTimeCycleMeetOffset(meet->soln, t);
  return KheMeetMove(meet, target_meet, target_offset);
  /* ***
  KHE_MEET target_meet;
  target_meet = KheSolnMeet(meet->soln, KheTimeCycleMeetIndex(t));
  return KheMeetMove(meet, target_meet, KheTimeCycleMeetOffset(t));
  *** */
}


/*****************************************************************************/
/*                                                                           */
/*  void KheMeetSwapDebug(char *op, KHE_MEET meet1, KHE_MEET meet2)          */
/*                                                                           */
/*  Generate a debug print of op(meet1, meet2).                              */
/*                                                                           */
/*****************************************************************************/

static void KheMeetSwapDebug(char *op, KHE_MEET meet1, KHE_MEET meet2)
{
  fprintf(stderr, "%s(", op);
  KheMeetDebug(meet1, 1, -1, stderr);
  fprintf(stderr, ", ");
  KheMeetDebug(meet2, 1, -1, stderr);
  fprintf(stderr, ")\n");
}


/*****************************************************************************/
/*                                                                           */
/*  bool KheMeetSwapCheck(KHE_MEET meet1, KHE_MEET meet2)                    */
/*                                                                           */
/*  Return true if a swap of the assignments of meet1 and meet2 is possible. */
/*                                                                           */
/*****************************************************************************/

bool KheMeetSwapCheck(KHE_MEET meet1, KHE_MEET meet2)
{
  KHE_MEET meet1_target_meet, meet2_target_meet;
  int meet1_target_offset, meet2_target_offset;  bool res;
  if( DEBUG13 )
    KheMeetSwapDebug("[ KheMeetSwapCheck", meet1, meet2);

  /* make sure the swap would change something */
  meet1_target_meet = meet1->target_meet;
  meet2_target_meet = meet2->target_meet;
  meet1_target_offset = meet1->target_offset;
  meet2_target_offset = meet2->target_offset;
  if( meet1_target_meet == meet2_target_meet &&
      meet1_target_offset == meet2_target_offset )
    res = false;
  else if( meet1->target_meet == NULL )
  {
    /* unassign meet2 and assign meet1 */
    KheMeetUnAssign(meet2);
    res = KheMeetAssignCheck(meet1, meet2_target_meet, meet2_target_offset);
    KheMeetAssign(meet2, meet2_target_meet, meet2_target_offset);
  }
  else if( meet2->target_meet == NULL )
  {
    /* unassign meet1 and assign meet2 */
    KheMeetUnAssign(meet1);
    res = KheMeetAssignCheck(meet2, meet1_target_meet, meet1_target_offset);
    KheMeetAssign(meet1, meet1_target_meet, meet1_target_offset);
  }
  else
  {
    /* a full swap of meet1 and meet2 */
    KheMeetUnAssign(meet1);
    KheMeetUnAssign(meet2);
    if( KheMeetAssign(meet1, meet2_target_meet, meet2_target_offset) )
    {
      res = KheMeetAssignCheck(meet2, meet1_target_meet, meet1_target_offset);
      KheMeetUnAssign(meet1);
    }
    else
      res = false;
    KheMeetAssign(meet1, meet1_target_meet, meet1_target_offset);
    KheMeetAssign(meet2, meet2_target_meet, meet2_target_offset);
  }
  if( DEBUG13 )
    fprintf(stderr, "] KheMeetSwapCheck returning %s\n", res ? "true":"false");
  return res;
}


/*****************************************************************************/
/*                                                                           */
/*  bool KheMeetSwap(KHE_MEET meet1, KHE_MEET meet2)                         */
/*                                                                           */
/*  Either swap the assignments of meet1 and meet2 and return true, or, if   */
/*  that is not possible, change nothing and return false.                   */
/*                                                                           */
/*****************************************************************************/

bool KheMeetSwap(KHE_MEET meet1, KHE_MEET meet2)
{
  KHE_MEET meet1_target_meet, meet2_target_meet;  bool res;
  int meet1_target_offset, meet2_target_offset;
  if( DEBUG13 )
    KheMeetSwapDebug("[ KheMeetSwap", meet1, meet2);

  /* make sure the swap would change something */
  meet1_target_meet = meet1->target_meet;
  meet2_target_meet = meet2->target_meet;
  meet1_target_offset = meet1->target_offset;
  meet2_target_offset = meet2->target_offset;
  if( meet1_target_meet == meet2_target_meet &&
      meet1_target_offset == meet2_target_offset )
    res = false;
  else if( meet1->target_meet == NULL )
  {
    /* unassign meet2 and assign meet1 */
    KheMeetUnAssign(meet2);
    if( !KheMeetAssign(meet1, meet2_target_meet, meet2_target_offset) )
    {
      KheMeetAssign(meet2, meet2_target_meet, meet2_target_offset);
      res = false;
    }
    else
      res = true;
  }
  else if( meet2->target_meet == NULL )
  {
    /* unassign meet1 and assign meet2 */
    KheMeetUnAssign(meet1);
    if( !KheMeetAssign(meet2, meet1_target_meet, meet1_target_offset) )
    {
      KheMeetAssign(meet1, meet1_target_meet, meet1_target_offset);
      res = false;
    }
    else
      res = true;
  }
  else
  {
    /* a full swap of meet1 and meet2 */
    KheMeetUnAssign(meet1);
    KheMeetUnAssign(meet2);
    if( !KheMeetAssign(meet1, meet2_target_meet, meet2_target_offset) )
    {
      KheMeetAssign(meet1, meet1_target_meet, meet1_target_offset);
      KheMeetAssign(meet2, meet2_target_meet, meet2_target_offset);
      res = false;
    }
    else if( !KheMeetAssign(meet2, meet1_target_meet, meet1_target_offset) )
    {
      KheMeetUnAssign(meet1);
      KheMeetAssign(meet1, meet1_target_meet, meet1_target_offset);
      KheMeetAssign(meet2, meet2_target_meet, meet2_target_offset);
      res = false;
    }
    else
      res = true;
  }
  if( DEBUG13 )
    fprintf(stderr, "] KheMeetSwap returning %s\n", res ? "true" : "false");
  return res;
}


/*****************************************************************************/
/*                                                                           */
/*  void GetBlockSwapOffsets(KHE_MEET meet1, KHE_MEET meet2,                 */
/*    int *meet1_block_offset, int *meet2_block_offset)                      */
/*                                                                           */
/*  Helper function for use when block swapping.  Here meet1 and meet2 are   */
/*  in their initial state and we are interested in block swapping them.     */
/*  With an ordinary swap, meet1's target offset is used for meet2, and      */
/*  meet2's target offset is used for meet1.  With a block swap, one of      */
/*  these could be different.  This function works them both out.            */
/*                                                                           */
/*  It is a precondition that both meet1 and meet2 are assigned, but         */
/*  possibly to different meets.                                             */
/*                                                                           */
/*****************************************************************************/

static void GetBlockSwapOffsets(KHE_MEET meet1, KHE_MEET meet2,
  int *meet1_block_offset, int *meet2_block_offset)
{
  if( meet1->target_meet != meet2->target_meet )
  {
    /* assigned to different meets, so not a block swap */
    *meet1_block_offset = meet1->target_offset;
    *meet2_block_offset = meet2->target_offset;
  }
  else if( meet1->target_offset + meet1->duration == meet2->target_offset )
  {
    /* block swap with meet2 immediately following meet1 */
    *meet1_block_offset = meet1->target_offset;             /* used by meet2 */
    *meet2_block_offset = meet1->target_offset + meet2->duration; /* by meet1*/
  }
  else if( meet2->target_offset + meet2->duration == meet1->target_offset )
  {
    /* block swap with meet1 immediately following meet2 */
    *meet2_block_offset = meet2->target_offset;             /* used by meet1 */
    *meet1_block_offset = meet2->target_offset + meet1->duration; /* by meet2*/
  }
  else
  {
    /* assigned to same meet but not adjacent, so not a block swap */
    *meet1_block_offset = meet1->target_offset;
    *meet2_block_offset = meet2->target_offset;
  }
}


/*****************************************************************************/
/*                                                                           */
/*  bool KheMeetBlockSwapCheck(KHE_MEET meet1, KHE_MEET meet2)               */
/*                                                                           */
/*  Return true if a block swap of the assignments of meet1 and meet2 is     */
/*  possible.                                                                */
/*                                                                           */
/*****************************************************************************/

bool KheMeetBlockSwapCheck(KHE_MEET meet1, KHE_MEET meet2)
{
  KHE_MEET meet1_target_meet, meet2_target_meet;
  int meet1_target_offset, meet2_target_offset;  bool res;
  int meet1_block_offset, meet2_block_offset;
  if( DEBUG13 )
    KheMeetSwapDebug("[ KheMeetBlockSwapCheck", meet1, meet2);

  /* make sure the swap would change something */
  meet1_target_meet = meet1->target_meet;
  meet2_target_meet = meet2->target_meet;
  meet1_target_offset = meet1->target_offset;
  meet2_target_offset = meet2->target_offset;
  if( meet1_target_meet == meet2_target_meet &&
      meet1_target_offset == meet2_target_offset )
    res = false;
  else if( meet1->target_meet == NULL )
  {
    /* unassign meet2 and assign meet1 */
    KheMeetUnAssign(meet2);
    res = KheMeetAssignCheck(meet1, meet2_target_meet, meet2_target_offset);
    KheMeetAssign(meet2, meet2_target_meet, meet2_target_offset);
  }
  else if( meet2->target_meet == NULL )
  {
    /* unassign meet1 and assign meet2 */
    KheMeetUnAssign(meet1);
    res = KheMeetAssignCheck(meet2, meet1_target_meet, meet1_target_offset);
    KheMeetAssign(meet1, meet1_target_meet, meet1_target_offset);
  }
  else
  {
    /* a full swap of meet1 and meet2, possibly a block swap */
    GetBlockSwapOffsets(meet1, meet2, &meet1_block_offset, &meet2_block_offset);
    KheMeetUnAssign(meet1);
    KheMeetUnAssign(meet2);
    if( KheMeetAssign(meet1, meet2_target_meet, meet2_block_offset) )
    {
      res = KheMeetAssignCheck(meet2, meet1_target_meet, meet1_block_offset);
      KheMeetUnAssign(meet1);
    }
    else
      res = false;
    KheMeetAssign(meet1, meet1_target_meet, meet1_target_offset);
    KheMeetAssign(meet2, meet2_target_meet, meet2_target_offset);
  }
  if( DEBUG13 )
    fprintf(stderr, "] KheMeetBlockSwapCheck ret %s\n", res ? "true" : "false");
  return res;
}


/*****************************************************************************/
/*                                                                           */
/*  bool KheMeetBlockSwap(KHE_MEET meet1, KHE_MEET meet2)                    */
/*                                                                           */
/*  Either block swap the assignments of meet1 and meet2 and return true,    */
/*  or, if that is not possible, change nothing and return false.            */
/*                                                                           */
/*****************************************************************************/

bool KheMeetBlockSwap(KHE_MEET meet1, KHE_MEET meet2)
{
  KHE_MEET meet1_target_meet, meet2_target_meet;  bool res;
  int meet1_target_offset, meet2_target_offset;
  int meet1_block_offset, meet2_block_offset;
  if( DEBUG13 )
    KheMeetSwapDebug("[ KheMeetBlockSwap", meet1, meet2);

  /* make sure the swap would change something */
  meet1_target_meet = meet1->target_meet;
  meet2_target_meet = meet2->target_meet;
  meet1_target_offset = meet1->target_offset;
  meet2_target_offset = meet2->target_offset;
  if( meet1_target_meet == meet2_target_meet &&
      meet1_target_offset == meet2_target_offset )
    res = false;
  else if( meet1->target_meet == NULL )
  {
    /* unassign meet2 and assign meet1 */
    KheMeetUnAssign(meet2);
    if( !KheMeetAssign(meet1, meet2_target_meet, meet2_target_offset) )
    {
      KheMeetAssign(meet2, meet2_target_meet, meet2_target_offset);
      res = false;
    }
    else
      res = true;
  }
  else if( meet2->target_meet == NULL )
  {
    /* unassign meet1 and assign meet2 */
    KheMeetUnAssign(meet1);
    if( !KheMeetAssign(meet2, meet1_target_meet, meet1_target_offset) )
    {
      KheMeetAssign(meet1, meet1_target_meet, meet1_target_offset);
      res = false;
    }
    else
      res = true;
  }
  else
  {
    /* a full swap of meet1 and meet2, possibly a block swap */
    GetBlockSwapOffsets(meet1, meet2, &meet1_block_offset, &meet2_block_offset);
    KheMeetUnAssign(meet1);
    KheMeetUnAssign(meet2);
    if( !KheMeetAssign(meet1, meet2_target_meet, meet2_block_offset) )
    {
      KheMeetAssign(meet1, meet1_target_meet, meet1_target_offset);
      KheMeetAssign(meet2, meet2_target_meet, meet2_target_offset);
      res = false;
    }
    else if( !KheMeetAssign(meet2, meet1_target_meet, meet1_block_offset) )
    {
      KheMeetUnAssign(meet1);
      KheMeetAssign(meet1, meet1_target_meet, meet1_target_offset);
      KheMeetAssign(meet2, meet2_target_meet, meet2_target_offset);
      res = false;
    }
    else
      res = true;
  }
  if( DEBUG13 )
    fprintf(stderr, "] KheMeetBlockSwap ret %s\n", res ? "true" : "false");
  return res;
}


/*****************************************************************************/
/*                                                                           */
/*  Submodule "time preassignment"                                           */
/*                                                                           */
/*****************************************************************************/

/*****************************************************************************/
/*                                                                           */
/*  bool KheMeetIsPreassigned(KHE_MEET meet, bool as_in_event,KHE_TIME *time)*/
/*                                                                           */
/*  If meet is preassigned to a specific time, return true and set *time to  */
/*  that time, else return false.                                            */
/*                                                                           */
/*  If as_in_event is true, a meet is considered to have a preassigned time  */
/*  if it is derived from an event and that event has a preassigned time.    */
/*  If as_in_event is false, a meet is considered to have a preassigned      */
/*  time if it has a time domain containing exactly one element.             */
/*                                                                           */
/*****************************************************************************/

bool KheMeetIsPreassigned(KHE_MEET meet, bool as_in_event, KHE_TIME *time)
{
  KHE_EVENT e;
  if( as_in_event )
  {
    /* take preassignment from event */
    e = KheMeetEvent(meet);
    if( e == NULL || KheEventPreassignedTime(e) == NULL )
      return false;
    else
    {
      *time = KheEventPreassignedTime(e);
      return true;
    }
  }
  else
  {
    /* take preassignment from domain, if any */
    if( meet->time_domain == NULL )
      return false;
    else if( KheTimeGroupTimeCount(meet->time_domain) == 1 )
    {
      *time = KheTimeGroupTime(meet->time_domain, 0);
      return true;
    }
    else
      return false;
  }
}


/*****************************************************************************/
/*                                                                           */
/*  bool KheMeetHasPreassignedDescendant(KHE_MEET meet, bool as_in_event,    */
/*    KHE_TIME *time)                                                        */
/*                                                                           */
/*  Return true if any descendant of meet (including meet itself) is         */
/*  preassigned.  Parameter as_in_event is as for KheMeetIsPreassigned.      */
/*                                                                           */
/*****************************************************************************/

static bool KheMeetHasPreassignedDescendant(KHE_MEET meet, bool as_in_event,
  KHE_TIME *time)
{
  KHE_MEET child_meet;  int i;
  if( KheMeetIsPreassigned(meet, as_in_event, time) )
    return true;
  MArrayForEach(meet->assigned_meets, &child_meet, &i)
    if( KheMeetHasPreassignedDescendant(child_meet, as_in_event, time) )
      return true;
  return false;
}


/*****************************************************************************/
/*                                                                           */
/*  bool KheMeetIsAssignedPreassigned(KHE_MEET meet, KHE_TIME *t)            */
/*                                                                           */
/*  Return true if meet is, or is assigned, a preassigned meet,              */
/*  and set *t to the time that meet needs to be assigned in order to make   */
/*  sure that this meet gets the assignment it needs.                        */
/*                                                                           */
/*****************************************************************************/

bool KheMeetIsAssignedPreassigned(KHE_MEET meet, bool as_in_event,
  KHE_TIME *t)
{
  if( meet->time_domain == NULL )
    return KheMeetHasPreassignedDescendant(meet, as_in_event, t);
  else if( KheTimeGroupTimeCount(meet->time_domain) == 1 &&
    KheMeetHasPreassignedDescendant(meet, as_in_event, t) )
      return true;
  else
    return false;
}


/*****************************************************************************/
/*                                                                           */
/*  Submodule "time domains"                                                 */
/*                                                                           */
/*****************************************************************************/

/*****************************************************************************/
/*                                                                           */
/*  bool KheMeetDomainAllowsAssignment(KHE_MEET meet,                        */
/*    KHE_TIME_GROUP target_domain, int target_offset)                       */
/*                                                                           */
/*  Return true if the domain of meet allows its assignment to a meet        */
/*  with domain target_domain at offset target_offset.                       */
/*                                                                           */
/*****************************************************************************/

static bool KheMeetDomainAllowsAssignment(KHE_MEET meet,
  KHE_TIME_GROUP target_domain, int target_offset)
{
  KHE_MEET child_meet;  int i;
  if( meet->time_domain != NULL )
    return KheTimeGroupDomainsAllowAssignment(meet->time_domain,
      target_domain, target_offset);
  else
  {
    /* automatic domain, have to consult meet's children */
    MArrayForEach(meet->assigned_meets, &child_meet, &i)
      if( !KheMeetDomainAllowsAssignment(child_meet, target_domain,
	    target_offset + KheMeetAsstOffset(child_meet)) )
	return false;
    return true;
  }
}


/*****************************************************************************/
/*                                                                           */
/*  bool KheMeetDomainPreassignmentCheck(KHE_MEET meet, bool as_in_event,    */
/*    KHE_TIME_GROUP tg)                                                     */
/*                                                                           */
/*  Check that current domain tg is compatible with any preassignment of     */
/*  meet.                                                                    */
/*                                                                           */
/*****************************************************************************/

static bool KheMeetDomainPreassignmentCheck(KHE_MEET meet, bool as_in_event,
  KHE_TIME_GROUP tg)
{
  KHE_TIME t;
  return !KheMeetIsPreassigned(meet, as_in_event, &t) ||
    KheTimeGroupSubset(tg, KheTimeSingletonTimeGroup(t));
}


/*****************************************************************************/
/*                                                                           */
/*  bool KheMeetSetDomainCheck(KHE_MEET meet, KHE_TIME_GROUP tg)             */
/*                                                                           */
/*  Check whether the domain of meet can be set to td.                       */
/*                                                                           */
/*****************************************************************************/

bool KheMeetSetDomainCheck(KHE_MEET meet, KHE_TIME_GROUP tg)
{
  KHE_MEET child_meet, anc;  int i, anc_offset;

  /* meet must not be a cycle meet */
  if( KheMeetIsCycleMeet(meet) )
  {
    if( DEBUG12 )
    {
      fprintf(stderr, "KheMeetSetDomainCheck(");
      KheMeetDebug(meet, 1, -1, stderr);
      fprintf(stderr, ", ");
      if( tg == NULL )
	fprintf(stderr, "NULL");
      else
	KheTimeGroupDebug(tg, 1, -1, stderr);
      fprintf(stderr, ") = false (cycle meet)\n");
    }
    return false;
  }

  /* if tg is NULL, meet must not be derived from an event or have tasks; */
  /* but the assignment is otherwise permitted */
  if( tg == NULL )
  {
    if( DEBUG12 )
    {
      fprintf(stderr, "KheMeetSetDomainCheck(");
      KheMeetDebug(meet, 1, -1, stderr);
      fprintf(stderr, ", NULL) = %s\n", meet->event_in_soln == NULL &&
	MArraySize(meet->tasks) == 0 ? "true" : "false");
    }
    return meet->event_in_soln == NULL && MArraySize(meet->tasks) == 0;
  }

  /* now tg != NULL; check compatibility with any preassignment */
  if( !KheMeetDomainPreassignmentCheck(meet, true, tg) )
  {
    if( DEBUG12 )
    {
      fprintf(stderr, "KheMeetSetDomainCheck(");
      KheMeetDebug(meet, 1, -1, stderr);
      fprintf(stderr, ", ");
      KheTimeGroupDebug(tg, 1, -1, stderr);
      fprintf(stderr, ") = false (preasst)\n");
    }
    return false;
  }

  /* check compatibility of tg with the closest ancestor domain, if any */
  anc = meet->target_meet;  anc_offset = meet->target_offset;
  while( anc != NULL && anc->time_domain == NULL )
  {
    anc_offset += anc->target_offset;
    anc = anc->target_meet;
  };
  if( anc != NULL &&
      !KheTimeGroupDomainsAllowAssignment(tg, anc->time_domain, anc_offset) )
  {
    if( DEBUG12 )
    {
      fprintf(stderr, "KheMeetSetDomainCheck(");
      KheMeetDebug(meet, 1, -1, stderr);
      fprintf(stderr, ", ");
      KheTimeGroupDebug(tg, 1, -1, stderr);
      fprintf(stderr, ") = false (ancestor meet ");
      KheMeetDebug(anc, 1, -1, stderr);
      fprintf(stderr, ")\n");
    }
    return false;
  }

  /* check compatibility of tg with the childrens' domains */
  MArrayForEach(meet->assigned_meets, &child_meet, &i)
    if( !KheMeetDomainAllowsAssignment(child_meet, tg,
	  child_meet->target_offset) )
    {
      if( DEBUG12 )
      {
	fprintf(stderr, "KheMeetSetDomainCheck(");
	KheMeetDebug(meet, 1, -1, stderr);
	fprintf(stderr, ", ");
	KheTimeGroupDebug(tg, 1, -1, stderr);
	fprintf(stderr, ") = false (child meet ");
	KheMeetDebug(child_meet, 1, -1, stderr);
	fprintf(stderr, ")\n");
      }
      return false;
    }

  /* all in order */
  return true;
}


/*****************************************************************************/
/*                                                                           */
/*  bool KheMeetSetDomain(KHE_MEET meet, KHE_TIME_GROUP tg)                  */
/*                                                                           */
/*  Set the domain of meet to tg.                                            */
/*                                                                           */
/*****************************************************************************/

bool KheMeetSetDomain(KHE_MEET meet, KHE_TIME_GROUP tg)
{
  KHE_MATCHING_DEMAND_CHUNK dc;  int i;  KHE_MATCHING_TYPE t;

  /* check safe to do this */
  if( !KheMeetSetDomainCheck(meet, tg) )
    return false;

  /* inform soln that this is happening */
  KheSolnOpMeetSetDomain(meet->soln, meet, meet->time_domain, tg);

  /* do it */
  meet->time_domain = tg;
  if( tg != NULL )
  {
    t = KheSolnMatchingType(meet->soln);
    if(t==KHE_MATCHING_TYPE_EVAL_INITIAL || t==KHE_MATCHING_TYPE_EVAL_RESOURCES)
      MArrayForEach(meet->demand_chunks, &dc, &i)
	KheMatchingDemandChunkSetDomain(dc, KheTimeGroupTimeIndexes(tg),
	  KHE_MATCHING_DOMAIN_CHANGE_TO_OTHER);
  }
  if( DEBUG4 )
  {
    fprintf(stderr, "  KheMeetSetDomain(");
    KheMeetDebug(meet, 1, -1, stderr);
    fprintf(stderr, ", ");
    if( tg != NULL )
      KheTimeGroupDebug(tg, 1, -1, stderr);
    else
      fprintf(stderr, "NULL");
    fprintf(stderr, ")\n");
  }
  return true;
}


/*****************************************************************************/
/*                                                                           */
/*  KHE_TIME_GROUP KheMeetDomain(KHE_MEET meet)                              */
/*                                                                           */
/*  Return the domain of meet.  This is NULL if the domain is automatic.     */
/*                                                                           */
/*****************************************************************************/

KHE_TIME_GROUP KheMeetDomain(KHE_MEET meet)
{
  return meet->time_domain;
}


/*****************************************************************************/
/*                                                                           */
/*  void KheIntesectMeetDescendants(KHE_MEET meet, int offset,               */
/*    KHE_DOMAIN_STATE *state, KHE_TIME_GROUP *domain)                       */
/*                                                                           */
/*  Accumulate the intersection of the domains of the descendants of meet,   */
/*  adjusted by offset, in *domain, depending on *state as follows:          */
/*                                                                           */
/*    KHE_DOMAIN_INIT: no domains have been encountered yet.                 */
/*                                                                           */
/*    KHE_DOMAIN_SINGLE: at least one domain has been encounted, and one     */
/*    of those domains is a subset of all the others; *domain is that one.   */
/*                                                                           */
/*    KHE_DOMAIN_MULTI: neither of the above cases applies.  A time group    */
/*    is under construction by KheSolnTimeGroupBegin etc.                    */
/*                                                                           */
/*  Parameter *domain is really undefined if *state != KHE_DOMAIN_SINGLE;    */
/*  but for sanity its value is set to NULL in that case.                    */
/*                                                                           */
/*****************************************************************************/

typedef enum {
  KHE_DOMAIN_INIT,
  KHE_DOMAIN_SINGLE,
  KHE_DOMAIN_MULTI
} KHE_DOMAIN_STATE;

static void KheIntesectMeetDescendants(KHE_MEET meet, int offset,
  KHE_DOMAIN_STATE *state, KHE_TIME_GROUP *domain)
{
  int i;  KHE_MEET child_meet;  KHE_TIME_GROUP tg;
  if( meet->time_domain == NULL )
  {
    MArrayForEach(meet->assigned_meets, &child_meet, &i)
      KheIntesectMeetDescendants(child_meet,
	offset - child_meet->target_offset, state, domain);
  }
  else
  {
    tg = KheTimeGroupNeighbour(meet->time_domain, offset);
    switch( *state )
    {
      case KHE_DOMAIN_INIT:

	*domain = tg;
	*state = KHE_DOMAIN_SINGLE;
	break;

      case KHE_DOMAIN_SINGLE:

	if( KheTimeGroupSubset(tg, *domain) )
	  *domain = tg;
	else if( !KheTimeGroupSubset(*domain, tg) )
	{
	  KheSolnTimeGroupBegin(meet->soln);
	  KheSolnTimeGroupUnion(meet->soln, *domain);
	  KheSolnTimeGroupIntersect(meet->soln, tg);
	  *state = KHE_DOMAIN_MULTI;
	  *domain = NULL;
	}
	break;

      case KHE_DOMAIN_MULTI:

	KheSolnTimeGroupIntersect(meet->soln, tg);
	break;
      
      default:
	MAssert(false, "KheIntesectMeetDescendants internal error");
    }
  }
}


/*****************************************************************************/
/*                                                                           */
/*  KHE_TIME_GROUP KheMeetDescendantsDomain(KHE_MEET meet)                   */
/*                                                                           */
/*  Return the time group which is the intersection of the domains of the    */
/*  meets assigned to meet, or the full group if none.                       */
/*                                                                           */
/*****************************************************************************/

KHE_TIME_GROUP KheMeetDescendantsDomain(KHE_MEET meet)
{
  KHE_DOMAIN_STATE state;  KHE_TIME_GROUP domain;
  state = KHE_DOMAIN_INIT;
  domain = NULL;
  KheIntesectMeetDescendants(meet, 0, &state, &domain);
  switch( state )
  {
    case KHE_DOMAIN_INIT:

      return KheInstanceFullTimeGroup(KheSolnInstance(meet->soln));

    case KHE_DOMAIN_SINGLE:

      return domain;

    case KHE_DOMAIN_MULTI:

      return KheSolnTimeGroupEnd(meet->soln);
    
    default:

      MAssert(false, "KheMeetDescendantsDomain internal error");
      return NULL;  /* keep compiler happy */
  }
}


/*****************************************************************************/
/*                                                                           */
/*  Submodule "visit numbers"                                                */
/*                                                                           */
/*****************************************************************************/

/*****************************************************************************/
/*                                                                           */
/*  void KheMeetSetVisitNum(KHE_MEET meet, int num)                          */
/*                                                                           */
/*  Set the visit number of meet.                                            */
/*                                                                           */
/*****************************************************************************/

void KheMeetSetVisitNum(KHE_MEET meet, int num)
{
  meet->visit_num = num;
}


/*****************************************************************************/
/*                                                                           */
/*  int KheMeetVisitNum(KHE_MEET meet)                                       */
/*                                                                           */
/*  Return the visit number of meet.                                         */
/*                                                                           */
/*****************************************************************************/

int KheMeetVisitNum(KHE_MEET meet)
{
  return meet->visit_num;
}


/*****************************************************************************/
/*                                                                           */
/*  bool KheMeetVisited(KHE_MEET meet, int slack)                            */
/*                                                                           */
/*  Return true if meet has been visited recently.                           */
/*                                                                           */
/*****************************************************************************/

bool KheMeetVisited(KHE_MEET meet, int slack)
{
  return KheSolnVisitNum(meet->soln) - meet->visit_num <= slack;
}


/*****************************************************************************/
/*                                                                           */
/*  void KheMeetVisit(KHE_MEET meet)                                         */
/*                                                                           */
/*  Visit meet.                                                              */
/*                                                                           */
/*****************************************************************************/

void KheMeetVisit(KHE_MEET meet)
{
  meet->visit_num = KheSolnVisitNum(meet->soln);
}


/*****************************************************************************/
/*                                                                           */
/*  void KheMeetUnVisit(KHE_MEET meet)                                       */
/*                                                                           */
/*  Unvisit meet.                                                            */
/*                                                                           */
/*****************************************************************************/

void KheMeetUnVisit(KHE_MEET meet)
{
  meet->visit_num = KheSolnVisitNum(meet->soln) - 1;
}


/*****************************************************************************/
/*                                                                           */
/*  Submodule "matchings"                                                    */
/*                                                                           */
/*****************************************************************************/

/*****************************************************************************/
/*                                                                           */
/*  void KheMeetMatchingReset(KHE_MEET meet)                                 */
/*                                                                           */
/*  Reset the matching within meet (but not within its tasks).               */
/*                                                                           */
/*  This function makes no attempt to move gracefully from one state to      */
/*  another; rather, it starts again from scratch, querying the current      */
/*  solution to see what is required.  This is useful for initializing,      */
/*  and also when the matching type changes, since in those cases a reset    */
/*  is the sensible way forward.                                             */
/*                                                                           */
/*  This function will be called separately for every meet, so there is      */
/*  no need to worry about making recursive calls, or calls on tasks.        */
/*                                                                           */
/*****************************************************************************/

void KheMeetMatchingReset(KHE_MEET meet)
{
  int i, resource_count;
  KHE_MATCHING_DEMAND_CHUNK dc; KHE_MATCHING_SUPPLY_CHUNK sc;
  KHE_MEET ancestor_meet;  int ancestor_offset;
  if( meet->time_domain != NULL )
  {
    resource_count = KheInstanceResourceCount(KheSolnInstance(meet->soln));
    switch( KheSolnMatchingType(meet->soln) )
    {
      case KHE_MATCHING_TYPE_EVAL_INITIAL:
      case KHE_MATCHING_TYPE_EVAL_RESOURCES:

	/* set the base and domain to reflect the current time domain */
	MArrayForEach(meet->demand_chunks, &dc, &i)
	{
	  KheMatchingDemandChunkSetBase(dc, i * resource_count);
	  KheMatchingDemandChunkSetIncrement(dc, resource_count);
	  KheMatchingDemandChunkSetDomain(dc,
	    KheTimeGroupTimeIndexes(meet->time_domain),
	    KHE_MATCHING_DOMAIN_CHANGE_TO_OTHER);
	}
	break;

      case KHE_MATCHING_TYPE_EVAL_TIMES:
      case KHE_MATCHING_TYPE_SOLVE:

	/* set the base and domain to the corresponding supply set of meet */
	ancestor_meet = meet;
	ancestor_offset = 0;
	while( ancestor_meet->target_meet != NULL )
	{
	  ancestor_offset += ancestor_meet->target_offset;
	  ancestor_meet = ancestor_meet->target_meet;
	}
	MArrayForEach(meet->demand_chunks, &dc, &i)
	{
	  sc = MArrayGet(ancestor_meet->supply_chunks, ancestor_offset + i);
	  KheMatchingDemandChunkSetBase(dc, KheMatchingSupplyChunkBase(sc));
	  KheMatchingDemandChunkSetIncrement(dc, resource_count);
	  KheMatchingDemandChunkSetDomain(dc,
	    KheSolnMatchingZeroDomain(meet->soln),
	    KHE_MATCHING_DOMAIN_CHANGE_TO_OTHER);
	}
	break;

      default:

	MAssert(false, "KheMeetMatchingReset internal error");
	break;
    }
  }
}


/*****************************************************************************/
/*                                                                           */
/*  KHE_MATCHING_DEMAND_CHUNK KheMeetDemandChunk(KHE_MEET meet, int offset)  */
/*                                                                           */
/*  Return meet's demand chunk at this offset.                               */
/*                                                                           */
/*****************************************************************************/

KHE_MATCHING_DEMAND_CHUNK KheMeetDemandChunk(KHE_MEET meet, int offset)
{
  return MArrayGet(meet->demand_chunks, offset);
}


/*****************************************************************************/
/*                                                                           */
/*  int KheMeetSupplyNodeOffset(KHE_MEET meet, KHE_MATCHING_SUPPLY_NODE sn)  */
/*                                                                           */
/*  Return the offset in meet that sn is for.                                */
/*                                                                           */
/*****************************************************************************/

int KheMeetSupplyNodeOffset(KHE_MEET meet, KHE_MATCHING_SUPPLY_NODE sn)
{
  int index, i;  KHE_MATCHING_SUPPLY_CHUNK sc;
  index = KheMatchingSupplyNodeIndex(sn);
  MArrayForEachReverse(meet->supply_chunks, &sc, &i)
    if( KheMatchingSupplyChunkBase(sc) <= index )
    {
      MAssert(index < KheMatchingSupplyChunkBase(sc) +
	KheMatchingSupplyChunkSupplyNodeCount(sc),
	"KheMeetSupplyNodeOffset internal error 1 (failed %d < %d + %d)",
	  index, KheMatchingSupplyChunkBase(sc),
	  KheMatchingSupplyChunkSupplyNodeCount(sc));
      return i;
    }
  MAssert(false, "KheMeetSupplyNodeOffset internal error 2");
  return 0; /* keep compiler happy */
}


/*****************************************************************************/
/*                                                                           */
/*  void KheMeetMatchingAttachAllOrdinaryDemandMonitors(KHE_MEET meet)       */
/*                                                                           */
/*  Ensure that all the ordinary demand monitors of meet are attached.       */
/*                                                                           */
/*****************************************************************************/

void KheMeetMatchingAttachAllOrdinaryDemandMonitors(KHE_MEET meet)
{
  KHE_TASK task;  int i;
  MArrayForEach(meet->tasks, &task, &i)
    KheTaskMatchingAttachAllOrdinaryDemandMonitors(task);
}


/*****************************************************************************/
/*                                                                           */
/*  void KheMeetMatchingDetachAllOrdinaryDemandMonitors(KHE_MEET meet)       */
/*                                                                           */
/*  Ensure that all the ordinary demand monitors of meet are detached.       */
/*                                                                           */
/*****************************************************************************/

void KheMeetMatchingDetachAllOrdinaryDemandMonitors(KHE_MEET meet)
{
  KHE_TASK task;  int i;
  MArrayForEach(meet->tasks, &task, &i)
    KheTaskMatchingDetachAllOrdinaryDemandMonitors(task);
}


/*****************************************************************************/
/*                                                                           */
/*  void KheMeetMatchingSetWeight(KHE_MEET meet, KHE_COST new_weight)        */
/*                                                                           */
/*  Change the weight of the ordinary demand monitors of meet.               */
/*                                                                           */
/*****************************************************************************/

void KheMeetMatchingSetWeight(KHE_MEET meet, KHE_COST new_weight)
{
  KHE_TASK task;  int i;
  MArrayForEach(meet->tasks, &task, &i)
    KheTaskMatchingSetWeight(task, new_weight);
}


/*****************************************************************************/
/*                                                                           */
/*  Submodule "reading and writing"                                          */
/*                                                                           */
/*****************************************************************************/

/*****************************************************************************/
/*                                                                           */
/*  bool KheMeetMakeFromKml(KML_ELT meet_elt, KHE_SOLN soln, KML_ERROR *ke)  */
/*                                                                           */
/*  Make a meet based on meet_elt and add it to soln.                        */
/*                                                                           */
/*****************************************************************************/

bool KheMeetMakeFromKml(KML_ELT meet_elt, KHE_SOLN soln, KML_ERROR *ke)
{
  KML_ELT duration_elt, time_elt, resources_elt, resource_elt;
  KHE_EVENT event;  KHE_TIME time, preassigned_time;
  int duration, j;  char *ref;  KHE_MEET meet;
  if( !KmlCheck(meet_elt, "Reference : +#Duration +Time +Resources", ke) )
    return false;

  /* reference (must be present in instance) */
  ref = KmlAttributeValue(meet_elt, 0);
  if( DEBUG3 )
    fprintf(stderr, "[ KheMeetMakeFromKml(%s, soln, -)\n", ref);
  if( !KheInstanceRetrieveEvent(KheSolnInstance(soln), ref, &event) )
    return KmlErrorMake(ke, KmlLineNum(meet_elt),
      KmlColNum(meet_elt), "<Event> Reference \"%s\" unknown", ref);

  /* Duration and event duration */
  if( KmlContainsChild(meet_elt, "Duration", &duration_elt) )
    sscanf(KmlText(duration_elt), "%d", &duration);
  else
    duration = KheEventDuration(event);

  /* make and add meet */
  meet = KheMeetMake(soln, duration, event);

  /* Time */
  if( KmlContainsChild(meet_elt, "Time", &time_elt) )
  {
    if( !KmlCheck(time_elt, "Reference", ke) )
      return false;
    ref = KmlAttributeValue(time_elt, 0);
    if( !KheInstanceRetrieveTime(KheSolnInstance(soln), ref, &time) )
      return KmlErrorMake(ke, KmlLineNum(time_elt), KmlColNum(time_elt),
	"<Time> Reference \"%s\" unknown", ref);
    preassigned_time = KheEventPreassignedTime(event);
    if( preassigned_time != NULL && preassigned_time != time )
      return KmlErrorMake(ke, KmlLineNum(time_elt), KmlColNum(time_elt),
	"<Time> \"%s\" conflicts with preassigned time \"%s\"",
	ref, KheTimeId(preassigned_time));
    if( !KheMeetAssignTime(meet, time) )
      return KmlErrorMake(ke, KmlLineNum(time_elt), KmlColNum(time_elt),
	"<Time> \"%s\" not assignable to <Event> \"%s\"",
	KheTimeId(time), KheEventId(event));
    if( DEBUG3 )
      fprintf(stderr, "  assigned time %s to %s\n", KheTimeId(time), ref);
  }

  /* Resources */
  if( KmlContainsChild(meet_elt, "Resources", &resources_elt) )
  {
    if( !KmlCheck(resources_elt, ": *Resource", ke) )
      return false;
    for( j = 0;  j < KmlChildCount(resources_elt);  j++ )
    {
      resource_elt = KmlChild(resources_elt, j);
      if( !KheTaskMakeFromKml(resource_elt, meet, ke) )
	return false;
    }
  }
  if( DEBUG3 )
    fprintf(stderr, "] KheMeetMakeFromKml returning\n");
  return true;
}


/*****************************************************************************/
/*                                                                           */
/*  int KheMeetAssignedTimeCmp(const void *t1, const void *t2)               */
/*                                                                           */
/*  Comparison function for sorting an array of meets by                     */
/*  increasing assigned time.  Unassigned meets go at the end.               */
/*                                                                           */
/*****************************************************************************/

int KheMeetAssignedTimeCmp(const void *t1, const void *t2)
{
  KHE_MEET meet1 = * (KHE_MEET *) t1;
  KHE_MEET meet2 = * (KHE_MEET *) t2;
  KHE_TIME time1 = KheMeetAsstTime(meet1);
  KHE_TIME time2 = KheMeetAsstTime(meet2);
  if( time1 == time2 )
    return KheMeetIndex(meet1) - KheMeetIndex(meet2);
  else
    return time1 == NULL ? 1 : time2 == NULL ? -1 :
      KheTimeIndex(time1) - KheTimeIndex(time2);
}


/*****************************************************************************/
/*                                                                           */
/*  bool KheMeetMustWrite(KHE_MEET meet)                                     */
/*                                                                           */
/*  Return true if it is necessary to write meet, either because its         */
/*  duration differs from the duration of the corresponding instance         */
/*  event, or it is assigned a time which is not a preassigned time, or      */
/*  it is necessary to write at least one of its solution resources.         */
/*                                                                           */
/*****************************************************************************/

bool KheMeetMustWrite(KHE_MEET meet)
{
  KHE_TIME assigned_time;  KHE_TASK task;  int i;
  MAssert(meet->event_in_soln != NULL, "KheMeetMustWrite internal error");
  if( meet->duration != KheEventDuration(KheMeetEvent(meet)) )
    return true;
  assigned_time = KheMeetAsstTime(meet);
  if( assigned_time != NULL &&
      assigned_time != KheEventPreassignedTime(KheMeetEvent(meet)) )
    return true;
  MArrayForEach(meet->tasks, &task, &i)
    if( KheTaskMustWrite(task, KheTaskEventResource(task)) )
      return true;
  return false;
}


/*****************************************************************************/
/*                                                                           */
/*  bool KheMeetWrite(KHE_MEET meet, KML_FILE kf)                            */
/*                                                                           */
/*  Write meet to kf.                                                        */
/*                                                                           */
/*****************************************************************************/

bool KheMeetWrite(KHE_MEET meet, KML_FILE kf)
{
  KHE_TIME assigned_time;  KHE_TASK task;  int i;
  bool started;
  MAssert(meet->event_in_soln != NULL, "KheMeetWrite: internal error");
  /* XMLVerify(event_elt, "Event Reference : +Duration +Time +Resources"); */
  KmlBegin(kf, "Event");
  KmlAttribute(kf, "Reference", KheEventId(KheMeetEvent(meet)));
  if( meet->duration != KheEventDuration(KheMeetEvent(meet)) )
    KmlEltPrintf(kf, "Duration", "%d", meet->duration);
  assigned_time = KheMeetAsstTime(meet);
  if( assigned_time != NULL &&
      assigned_time != KheEventPreassignedTime(KheMeetEvent(meet)) )
    KmlEltAttribute(kf, "Time", "Reference", KheTimeId(assigned_time));
  started = false;
  MArrayForEach(meet->tasks, &task, &i)
    if( KheTaskMustWrite(task, KheTaskEventResource(task)) )
    {
      if( !started )
      {
	KmlBegin(kf, "Resources");
	started = true;
      }
      if( !KheTaskWrite(task, KheTaskEventResource(task), kf) )
	return false;
    }
  if( started )
    KmlEnd(kf, "Resources");
  KmlEnd(kf, "Event");
  return true;
}


/*****************************************************************************/
/*                                                                           */
/*  Submodule "debug"                                                        */
/*                                                                           */
/*****************************************************************************/

/*****************************************************************************/
/*                                                                           */
/*  void KheMeetDebugName(KHE_MEET meet, FILE *fp)                           */
/*                                                                           */
/*  Debug print of the name of meet onto fp.                                 */
/*                                                                           */
/*****************************************************************************/

static void KheMeetDebugName(KHE_MEET meet, FILE *fp)
{
  KHE_EVENT e;  KHE_TIME t;  int j;
  if( KheMeetIsCycleMeet(meet) )
  {
    t = KheMeetAsstTime(meet);
    if( KheTimeId(t) != NULL )
      fprintf(fp, "/%s/", KheTimeId(t));
    else
      fprintf(fp, "/%d/", meet->assigned_time_index);
  }
  else if( meet->event_in_soln != NULL )
  {
    e = KheEventInSolnEvent(meet->event_in_soln);
    for( j = 0;  j < KheEventMeetCount(meet->soln, e);  j++ )
      if( KheEventMeet(meet->soln, e, j) == meet )
	break;
    MAssert(j < KheEventMeetCount(meet->soln, e),
      "KheMeetDebugName internal error");
    fprintf(fp, "\"%s\"", KheEventId(e) != NULL ? KheEventId(e) : "-");
    if( KheEventMeetCount(meet->soln, e) > 1 )
      fprintf(fp, ":%d", j);
  }
  else
    fprintf(fp, "#%d#", meet->index);
}


/*****************************************************************************/
/*                                                                           */
/*  bool KheMeetHasZones(KHE_MEET meet)                                      */
/*                                                                           */
/*  Return true if meet has any non-NULL zones.                              */
/*                                                                           */
/*****************************************************************************/

static bool KheMeetHasZones(KHE_MEET meet)
{
  int i;  KHE_ZONE zone;
  MArrayForEach(meet->zones, &zone, &i)
    if( zone != NULL )
      return true;
  return false;
}


/*****************************************************************************/
/*                                                                           */
/*  void KheMeetDebug(KHE_MEET meet, int verbosity, int indent, FILE *fp)    */
/*                                                                           */
/*  Debug print of meet onto fp with the given verbosity and indent.         */
/*                                                                           */
/*****************************************************************************/

void KheMeetDebug(KHE_MEET meet, int verbosity, int indent, FILE *fp)
{
  KHE_TIME t;  KHE_INSTANCE ins;  KHE_ZONE zone;  int i;
  MAssert(meet->soln != NULL, "KheMeetDebug internal error");
  if( verbosity == 1 )
  {
    /* just the name and duration */
    if( indent >= 0 )
      fprintf(fp, "%*s", indent, "");
    KheMeetDebugName(meet, fp);
    fprintf(fp, "d%d", meet->duration);
    if( indent >= 0 )
      fprintf(fp, "\n");
  }
  else if( verbosity >= 2 )
  {
    if( indent >= 0 )
      fprintf(fp, "%*s", indent, "");
    fprintf(fp, "[ Meet ");
    KheMeetDebugName(meet, fp);
    fprintf(fp, " durn %d ", meet->duration);
    if( meet->time_domain == NULL )
    {
      fprintf(stderr, "auto ");
      KheTimeGroupDebug(KheMeetDescendantsDomain(meet), 1, -1, fp);
    }
    else
      KheTimeGroupDebug(meet->time_domain, 1, -1, fp);
    if( verbosity >= 3 && KheMeetHasZones(meet) )
    {
      /* print zone indexes, if there are any zones */
      fprintf(fp, " ");
      MArrayForEach(meet->zones, &zone, &i)
      {
	if( i > 0 )
	  fprintf(fp, ":");
	if( zone == NULL )
	  fprintf(fp, "_");
	else
	  fprintf(fp, "%d", KheZoneIndex(zone));
      }
    }
    ins = KheSolnInstance(KheMeetSoln(meet));
    while( meet->target_meet != NULL )
    {
      if( KheMeetIsCycleMeet(meet->target_meet) )
      {
	t = KheTimeNeighbour(KheMeetAsstTime(meet->target_meet),
	  meet->target_offset);
	fprintf(fp, " --> ");
	if( KheTimeId(t) != NULL )
	  fprintf(fp, "/%s/", KheTimeId(t));
	else
	  fprintf(fp, "/%d/", KheTimeIndex(t));
      }
      else
      {
	if( meet->target_offset == 0 )
	  fprintf(fp, " --> ");
	else
	  fprintf(fp, " -%d-> ", meet->target_offset);
	KheMeetDebugName(meet->target_meet, fp);
      }
      meet = meet->target_meet;
    }
    fprintf(fp, " ]");
    if( indent >= 0 )
      fprintf(fp, "\n");
  }
}


/*****************************************************************************/
/*                                                                           */
/*  void KheMeetDebugDemandChunks(KHE_MEET meet, int verbosity,              */
/*    int indent, FILE *fp)                                                  */
/*                                                                           */
/*  Debug the demand chunks of meet and the meets assigned to meet.          */
/*                                                                           */
/*****************************************************************************/

/* ***
void KheMeetDebugDemandChunks(KHE_MEET meet, int verbosity,
  int indent, FILE *fp)
{
  KHE_MATCHING_DEMAND_CHUNK dc;  int i;  KHE_MEET child_meet;
  fprintf(fp, "%*s  ", indent, "");
  KheMeetDebug(meet, 1, -1, fp);
  fprintf(fp, " ");
  KheTimeGroupDebug(meet->curr_time_domain, 1, -1, fp);
  fprintf(fp, ":\n");
  MArrayForEach(meet->demand_chunks, &dc, &i)
    KheMatchingDebugDemandChunk(dc, 34, &KheMatchingMonitorSupplyNodeShow,
      &KheMatchingMonitorDemandNodeShow, verbosity, indent + 2, fp);
  MArrayForEach(meet->assigned_meets, &child_meet, &i)
    KheMeetDebugDemandChunks(child_meet, verbosity, indent, fp);
}
*** */
