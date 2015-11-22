
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
/*  FILE:         khe_layer_time.c                                           */
/*  DESCRIPTION:  Solvers for assigning times to layers                      */
/*                                                                           */
/*****************************************************************************/
#include <limits.h>
#include "khe.h"
#include "m.h"
#include "khe_wmatch.h"

#define DEBUG1 0
#define DEBUG2 0
#define DEBUG3 0
#define DEBUG4 0
#define DEBUG5 0
#define DEBUG6 0
#define DEBUG7 0
#define DEBUG8 0

/* these are for producing statistical graphs of performance */
#define DEBUG_GRAPH 1
#define DEBUG_GRAPH_HARD 0
#define DEBUG_GRAPH_REPAIR_ALL_LAYERS 1
#define DEBUG_GRAPH_REPAIR_LAYER 7

typedef MARRAY(KHE_MEET) ARRAY_KHE_MEET;
typedef MARRAY(KHE_NODE) ARRAY_KHE_NODE;

typedef struct khe_layer_info_rec *KHE_LAYER_INFO;
typedef MARRAY(KHE_LAYER_INFO) ARRAY_KHE_LAYER_INFO;


/*****************************************************************************/
/*                                                                           */
/*  KHE_LAYER_INFO - information about one soln layer.                       */
/*                                                                           */
/*****************************************************************************/

struct khe_layer_info_rec {
  KHE_LAYER		layer;			/* the soln layer            */
  int			assigned_count;		/* no of assigned meets      */
};


/*****************************************************************************/
/*                                                                           */
/*  Submodule "paralleling preassignments" (private)                         */
/*                                                                           */
/*****************************************************************************/

/*****************************************************************************/
/*                                                                           */
/*  int KheNodeAssignedMeetCount(KHE_NODE node)                              */
/*                                                                           */
/*  Return the number of assigned meets in node.                             */
/*                                                                           */
/*****************************************************************************/

static int KheNodeAssignedMeetCount(KHE_NODE node)
{
  int i, res;  KHE_MEET meet;
  res = 0;
  for( i = 0;  i < KheNodeMeetCount(node);  i++ )
  {
    meet = KheNodeMeet(node, i);
    if( KheMeetAsst(meet) != NULL )
      res++;
  }
  return res;
}


/*****************************************************************************/
/*                                                                           */
/*  int KheLayerAssignedMeetCount(KHE_LAYER layer)                           */
/*                                                                           */
/*  Return the no. of assigned meets in layer.                               */
/*                                                                           */
/*****************************************************************************/

static int KheLayerAssignedMeetCount(KHE_LAYER layer)
{
  int i, res;  KHE_NODE child_node;
  res = 0;
  for( i = 0;  i < KheLayerChildNodeCount(layer);  i++ )
  {
    child_node = KheLayerChildNode(layer, i);
    res += KheNodeAssignedMeetCount(child_node);
  }
  return res;
}


/*****************************************************************************/
/*                                                                           */
/*  KHE_LAYER_INFO KheLayerInfoMake(KHE_LAYER layer)                         */
/*                                                                           */
/*  Make a new soln layer info object for layer.                             */
/*                                                                           */
/*****************************************************************************/

static KHE_LAYER_INFO KheLayerInfoMake(KHE_LAYER layer)
{
  KHE_LAYER_INFO res;
  MMake(res);
  res->layer = layer;
  res->assigned_count = KheLayerAssignedMeetCount(layer);
  return res;
}


/*****************************************************************************/
/*                                                                           */
/*  int KheLayerInfoCmp(const void *t1, const void *t2)                      */
/*                                                                           */
/*  Comparison function for sorting soln layer info records into order of    */
/*  decreasing number of assigned meets.                                     */
/*                                                                           */
/*****************************************************************************/

static int KheLayerInfoCmp(const void *t1, const void *t2)
{
  KHE_LAYER_INFO si1 = * (KHE_LAYER_INFO *) t1;
  KHE_LAYER_INFO si2 = * (KHE_LAYER_INFO *) t2;
  if( si2->assigned_count != si1->assigned_count )
    return si2->assigned_count - si1->assigned_count;
  else
    return KheLayerIndex(si1->layer) - KheLayerIndex(si2->layer);
}


/*****************************************************************************/
/*                                                                           */
/*  bool KheOverlaps(int offset1, int duration1, int offset2, int duration2) */
/*                                                                           */
/*  Return true if two meets with these offsets and durations, assigned to   */
/*  the same meet, would overlap in time.                                    */
/*                                                                           */
/*****************************************************************************/

static bool KheOverlaps(int offset1, int duration1, int offset2, int duration2)
{
  if( offset1 + duration1 <= offset2 )
    return false;
  if( offset2 + duration2 <= offset1 )
    return false;
  return true;
}


/*****************************************************************************/
/*                                                                           */
/*  bool KheMeetsMatch(KHE_MEET meet1, KHE_MEET meet2)                       */
/*                                                                           */
/*  Assuming that meet1 and meet2 are both assigned, return true if they     */
/*  have the same duration and are assigned to the same meet at the same     */
/*  offset.                                                                  */
/*                                                                           */
/*****************************************************************************/

static bool KheMeetsMatch(KHE_MEET meet1, KHE_MEET meet2)
{
  return KheMeetAsst(meet1) == KheMeetAsst(meet2) &&
    KheMeetAsstOffset(meet1) == KheMeetAsstOffset(meet2) &&
    KheMeetDuration(meet1) == KheMeetDuration(meet2);
}


/*****************************************************************************/
/*                                                                           */
/*  bool KheNodeMatches(KHE_NODE layer_node, KHE_NODE other_node,            */
/*    ARRAY_KHE_MEET *match)                                                 */
/*                                                                           */
/*  Return true if layer_node matches other_node, that is, if the two        */
/*  nodes are distinct, have the same duration, and the assigned meets of    */
/*  other_node can be matched up with meets of layer_node, preferably with   */
/*  meets with the same assignment, but otherwise with unassigned meets.     */
/*  If successful, set *match to the match.                                  */
/*                                                                           */
/*****************************************************************************/

static bool KheNodeMatches(KHE_NODE layer_node, KHE_NODE other_node,
  ARRAY_KHE_MEET *match)
{
  int i, j, pos;  KHE_MEET other_meet, layer_meet;  bool done;
  if( DEBUG5 )
    fprintf(stderr, "  [ KheNodeMatches(Node %d, Node %d)\n",
      KheNodeIndex(layer_node), KheNodeIndex(other_node));

  /* make sure the two nodes are distinct with equal durations */
  if( other_node == layer_node )
  {
    if( DEBUG5 )
      fprintf(stderr, "  ] KheNodeMatches returning false (identical nodes)\n");
    return false;
  }
  if( KheNodeDuration(other_node) != KheNodeDuration(layer_node) )
  {
    if( DEBUG5 )
      fprintf(stderr, "  ] KheNodeMatches returning false (node durations)\n");
    return false;
  }

  /* match each assigned meet of other_node with a meet of layer_node */
  MArrayClear(*match);
  for( i = 0;  i < KheNodeMeetCount(other_node);  i++ )
  {
    other_meet = KheNodeMeet(other_node, i);
    if( KheMeetAsst(other_meet) != NULL )
    {
      /* search for an unused meet of layer_node assigned like other_meet */
      done = false;
      for( j = 0;  !done && j < KheNodeMeetCount(layer_node);  j++ )
      {
	layer_meet = KheNodeMeet(layer_node, j);
	if( KheMeetAsst(layer_meet) != NULL &&
	    KheMeetsMatch(layer_meet, other_meet) &&
	    !MArrayContains(*match, layer_meet, &pos) )
	{
	  MArrayAddLast(*match, layer_meet);
	  done = true;
	}
      }

      /* else, search for an unused assignable meet of layer_node */
      for( j = 0;  !done && j < KheNodeMeetCount(layer_node);  j++ )
      {
	layer_meet = KheNodeMeet(layer_node, j);
	if( KheMeetAsst(layer_meet) == NULL &&
	    KheMeetDuration(layer_meet) == KheMeetDuration(other_meet) &&
	    KheMeetAssignCheck(layer_meet, KheMeetAsst(other_meet),
	      KheMeetAsstOffset(other_meet)) &&
	    !MArrayContains(*match, layer_meet, &pos) )
	{
	  MArrayAddLast(*match, layer_meet);
	  done = true;
	}
      }

      /* else fail now */
      if( !done )
      {
	if( DEBUG5 )
	{
	  fprintf(stderr, "  ] KheNodeMatches returning false at ");
	  KheMeetDebug(other_meet, 1, -1, stderr);
	  fprintf(stderr, "\n");
	}
	return false;
      }
    }
  }
  if( DEBUG5 )
  {
    MArrayForEach(*match, &layer_meet, &i)
      KheMeetDebug(layer_meet, 2, 4, stderr);
    fprintf(stderr, "  ] KheNodeMatches returning true\n");
  }
  return true;
}


/*****************************************************************************/
/*                                                                           */
/*  bool KheNodeMatch(KHE_NODE layer_node, KHE_NODE other_node,              */
/*    ARRAY_KHE_MEET *match)                                                 */
/*                                                                           */
/*  Assuming KheNodeMatches above returned true for these parameters,        */
/*  carry out the assignments that give layer_node the assignments           */
/*  of other_node, using *match as a guide, and return true.                 */
/*                                                                           */
/*****************************************************************************/

static bool KheNodeMatch(KHE_NODE layer_node, KHE_NODE other_node,
  ARRAY_KHE_MEET *match)
{
  int i, j;  KHE_MEET layer_meet, other_meet;
  j = 0;
  for( i = 0;  i < KheNodeMeetCount(other_node);  i++ )
  {
    other_meet = KheNodeMeet(other_node, i);
    if( KheMeetAsst(other_meet) != NULL )
    {
      layer_meet = MArrayGet(*match, j);
      if( KheMeetAsst(layer_meet) == NULL )
      {
	if( DEBUG2 )
	{
	  fprintf(stderr, "  calling KheMeetAssign(");
	  KheMeetDebug(layer_meet, 1, -1, stderr);
	  fprintf(stderr, ", ");
	  KheMeetDebug(KheMeetAsst(other_meet), 1, -1, stderr);
	  fprintf(stderr, ", %d)\n", KheMeetAsstOffset(other_meet));
	}
	if( !KheMeetAssign(layer_meet, KheMeetAsst(other_meet),
	      KheMeetAsstOffset(other_meet)) )
	  MAssert(false, "KheNodeMatch internal error");
      }
      j++;
    }
  }
  return true;
}


/*****************************************************************************/
/*                                                                           */
/*  bool KheMeetsOverlap(KHE_MEET meet1, KHE_MEET meet2)                     */
/*                                                                           */
/*  Assuming that meet1 and meet2 are both assigned, return true if they     */
/*  are assigned to the same meet in such a way as to overlap in time.       */
/*                                                                           */
/*****************************************************************************/

static bool KheMeetsOverlap(KHE_MEET meet1, KHE_MEET meet2)
{
  return KheMeetAsst(meet1) == KheMeetAsst(meet2) &&
    KheOverlaps(KheMeetAsstOffset(meet1), KheMeetDuration(meet1),
      KheMeetAsstOffset(meet2), KheMeetDuration(meet2));
}


/*****************************************************************************/
/*                                                                           */
/*  bool KheNodesOverlap(KHE_NODE node1, KHE_NODE node2)                     */
/*                                                                           */
/*  Return true if node1 and node2 contain meets that overlap in time.       */
/*                                                                           */
/*****************************************************************************/

static bool KheNodesOverlap(KHE_NODE node1, KHE_NODE node2)
{
  KHE_MEET meet1, meet2;  int i, j;
  for( i = 0;  i < KheNodeMeetCount(node1);  i++ )
  {
    meet1 = KheNodeMeet(node1, i);
    if( KheMeetAsst(meet1) != NULL )
      for( j = 0;  j < KheNodeMeetCount(node2);  j++ )
      {
	meet2 = KheNodeMeet(node2, j);
	if( KheMeetAsst(meet2) != NULL && KheMeetsOverlap(meet1, meet2) )
	  return true;
      }
  }
  return false;
}


/*****************************************************************************/
/*                                                                           */
/*  bool KheNodeParallelAssignTimes(KHE_NODE child_node, KHE_LAYER layer)    */
/*                                                                           */
/*  Here child_node is known to have at least one preassigned meet.  Try     */
/*  to match up the node with a node of layer, assigning some meets of the   */
/*  node of layer the preassigned times.                                     */
/*                                                                           */
/*****************************************************************************/

static bool KheNodeParallelAssignTimes(KHE_NODE child_node, KHE_LAYER layer)
{
  int i, j;  bool res;  ARRAY_KHE_MEET match;
  ARRAY_KHE_NODE nodes;  KHE_NODE best_node, node;
  if( DEBUG2 )
  {
    fprintf(stderr, "[ KheNodeParallelAssignTimes(Node %d)\n",
      KheNodeIndex(child_node));
    KheNodeDebug(child_node, 2, 2, stderr);
  }

  /* find the nodes of layer that overlap child_node */
  MArrayInit(nodes);
  for( i = 0;  i < KheLayerChildNodeCount(layer);  i++ )
  {
    node = KheLayerChildNode(layer, i);
    if( !MArrayContains(nodes, node, &j) && KheNodesOverlap(node, child_node) )
      MArrayAddLast(nodes, node);
  }
  if( DEBUG2 )
  {
    fprintf(stderr, "  overlaps %d nodes", MArraySize(nodes));
    MArrayForEach(nodes, &node, &i)
      fprintf(stderr, "%s Node %d", i == 0 ? ":" : ",", KheNodeIndex(node));
    fprintf(stderr, "\n");
  }

  res = false;
  if( MArraySize(nodes) == 0 )
  {
    /* no node overlaps at present, so any matching node will do */
    best_node = NULL;
    MArrayInit(match);
    for( i = 0;  i < KheLayerChildNodeCount(layer);  i++ )
    {
      node = KheLayerChildNode(layer, j);
      if( KheNodeMatches(node, child_node, &match) )
      {
	if( best_node == NULL ||
	    KheNodeDuration(node) < KheNodeDuration(best_node) ||
	    (KheNodeDuration(node) == KheNodeDuration(best_node) &&
	     KheNodeAssignedMeetCount(node) < 
	     KheNodeAssignedMeetCount(best_node)) )
	  best_node = node;
      }
    }

    /* match the best node if there is one */
    if( best_node != NULL )
    {
      if( !KheNodeMatches(best_node, child_node, &match) )
	MAssert(false, "KheNodeParallelAssignTimes internal error");
      res = KheNodeMatch(best_node, child_node, &match);
    }
    MArrayFree(match);
  }
  else if( MArraySize(nodes) == 1 )
  {
    /* try to match child_node with this one node */
    MArrayInit(match);
    best_node = MArrayFirst(nodes);
    if( KheNodeMatches(best_node, child_node, &match) )
      res = KheNodeMatch(best_node, child_node, &match);
    MArrayFree(match);
  }
  else
  {
    /* preassignments already span two nodes, so abandon paralleling here */
  }

  MArrayFree(nodes);
  if( DEBUG2 )
    fprintf(stderr, "] KheNodeParallelAssignTimes returning %s\n",
      res ? "true" : "false");
  return res;
}



/*****************************************************************************/
/*                                                                           */
/*  bool KheLayerParallelAssignTimes(KHE_LAYER layer)                        */
/*                                                                           */
/*  Make assignments to the meets of layer which parallel preassignments     */
/*  layer's sibling layers, as far as possible.  Return true if every        */
/*  assigned meet in every sibling layer has a parallel meet in layer.       */
/*                                                                           */
/*****************************************************************************/

bool KheLayerParallelAssignTimes(KHE_LAYER layer)
{
  ARRAY_KHE_LAYER_INFO layer_infos;  KHE_LAYER_INFO layer_info;
  KHE_LAYER layer2;  KHE_NODE parent_node, child_node;  int i, j;
  bool res;
  if( DEBUG2 )
    fprintf(stderr, "[ KheLayerParallelAssignTimes(layer)\n");

  /* get the layer_infos and sort them by decreasing no of preassigned meets */
  MArrayInit(layer_infos);
  parent_node = KheLayerParentNode(layer);
  for( i = 0;  i < KheNodeChildLayerCount(parent_node);  i++ )
  {
    layer2 = KheNodeChildLayer(parent_node, i);
    if( layer2 != layer )
      MArrayAddLast(layer_infos, KheLayerInfoMake(layer2));
  }
  MArraySort(layer_infos, &KheLayerInfoCmp);

  /* make parallel assignments for each layer_info in turn */
  res = true;
  MArrayForEach(layer_infos, &layer_info, &i)
    for( j = 0;  j < KheLayerChildNodeCount(layer_info->layer);  j++ )
    {
      child_node = KheLayerChildNode(layer_info->layer, j);
      if( KheNodeAssignedMeetCount(child_node) > 0 )
	res &= KheNodeParallelAssignTimes(child_node, layer);
    }

  /* free the array of soln layer info objects and the objects themselves */
  MArrayForEach(layer_infos, &layer_info, &j)
    MFree(layer_info);
  MArrayFree(layer_infos);

  if( DEBUG2 )
    fprintf(stderr, "] KheLayerParallelAssignTimes returning %s\n",
      res ? "true" : "false");
  return res;
}


/*****************************************************************************/
/*                                                                           */
/*  Submodule "repairing one layer" (private)                                */
/*                                                                           */
/*****************************************************************************/

/*****************************************************************************/
/*                                                                           */
/*  bool KheEjectionChainLayerRepairTimesOld(KHE_LAYER layer)                */
/*                                                                           */
/*  Carry out a local search which changes the time assignments of layer     */
/*  with the aim of reducing the total cost of the solution.  Return true    */
/*  if all layer's meets are assigned on return.                             */
/*                                                                           */
/*****************************************************************************/

/* ** correct, but no longer used
static bool KheEjectionChainLayerRepairTimesOld(KHE_LAYER layer)
{
  KHE_EJECTOR ej;  KHE_GROUP_MONITOR gm;  bool res, child_of_root;
  KHE_SOLN soln;  KHE_NODE parent_node;
  soln = KheLayerSoln(layer);
  if( DEBUG6 )
  {
    fprintf(stderr, "[ KheEjectionChainLayerRepairTimesOld(layer) cost %.4f\n",
      KheCostShow(KheSolnCost(KheLayerSoln(layer))));
    KheSolnCostByTypeDebug(soln, 2, 2, stderr);
  }

  ** work out whether layer is a child layer of the root layer **
  parent_node = KheLayerParentNode(layer);
  child_of_root = KheNodeMeetCount(parent_node) > 0 &&
    KheMeetIsCycleMeet(KheNodeMeet(parent_node, 0));

  ** build the group monitors required by this algorithm **
  gm = KheLayerGroupMonitorsForMeetRepair(layer,
    KHE_SUBTAG_EVENT, "EventGroupMonitor",
    KHE_SUBTAG_PREASSIGNED_DEMAND, "PreassignedDemandGroupMonitor",
    KHE_SUBTAG_UNPREASSIGNED_DEMAND, "UnpreassignedDemandGroupMonitor",
    KHE_SUBTAG_LAYER, "LayerGroupMonitor", (KHE_GROUP_MONITOR) soln);

  ** set up the ejector and run the algorithm **
  ej = KheEjectorMake(soln);
  KheEjectorAddSchedule(ej, 1, INT_MAX, true);
  KheEjectorAddSchedule(ej, 2, INT_MAX, true);
  KheEjectorAddSchedule(ej, 3, INT_MAX, true);
  KheEjectorAddSchedule(ej, INT_MAX, INT_MAX, false);
  if( child_of_root )
    KheEjectorAddGroupAugment(ej, KHE_SUBTAG_EVENT, &KheEventAugment);
  KheEjectorAddGroupAugment(ej, KHE_SUBTAG_PREASSIGNED_DEMAND,
    &KheDemandAugment);
  KheEjectorAddGroupAugment(ej, KHE_SUBTAG_UNPREASSIGNED_DEMAND,
    &KheDemandAugment);
  KheEjectorSolve(ej, KHE_EJECTOR_FIRST_SUCCESS, gm);
  KheEjectorDelete(ej);

  ** remove group monitors and exit **
  KheUnGroupMonitors(gm);
  res = KheLayerAllChildMeetsAssigned(layer);
  if( DEBUG6 )
  {
    fprintf(stderr, "  cost after repairing:\n");
    KheSolnCostByTypeDebug(soln, 2, 2, stderr);
    fprintf(stderr, "] KheEjectionChainLayerRepairTimesOld ret %s, cost %.4f\n",
      res ? "true" : "false", KheCostShow(KheSolnCost(KheLayerSoln(layer))));
  }
  return res;
}
*** */


/*****************************************************************************/
/*                                                                           */
/*  bool TripleGt(int a1, int a2, int a3, int b1, int b2, int b3)            */
/*                                                                           */
/*  Return true if (a1, a2, a3) > (b1, b2, b3) lexicographically.            */
/*                                                                           */
/*****************************************************************************/

/* ***
static bool TripleGt(int a1, int a2, int a3, int b1, int b2, int b3)
{
  return a1 != b1 ? a1 > b1 : a2 != b2 ? a2 > b2 : a3 > b3;
}
*** */


/*****************************************************************************/
/*                                                                           */
/*  KHE_LAYER KheFindTemplateLayer(KHE_LAYER layer)                          */
/*                                                                           */
/*  Find an already visited layer of layer's layering that makes the best    */
/*  template for layer.  The best is the one with the lexicographically      */
/*  maximum value of the triple                                              */
/*                                                                           */
/*     (duration, sameness, regularity)                                      */
/*                                                                           */
/*  where duration is the duration of the layer, sameness is the number of   */
/*  nodes in common with layer, and regularity is the regularity with layer. */
/*                                                                           */
/*****************************************************************************/

/* ***
static KHE_LAYER KheFindTemplateLayer(KHE_LAYER layer)
{
  KHE_LAYER tlayer, best_tlayer;  KHE_NODE parent_node;
  int durn, best_durn, same, best_same, regl, best_regl, i;
  if( DEBUG8 )
  {
    fprintf(stderr, "[ KheFindTemplateLayer(layer):\n");
    KheLayerDebug(layer, 2, 2, stderr);
  }
  best_tlayer = NULL;
  best_durn = best_same = best_regl = -1;
  parent_node = KheLayerParentNode(layer);
  for( i = 0;  i < KheNodeChildLayerCount(parent_node);  i++ )
  {
    tlayer = KheNodeChildLayer(parent_node, i);
    if( tlayer == layer )
      break;
    MAssert(KheLayerVisitNum(tlayer)==1, "KheFindTemplateLayer internal error");
    durn = KheLayerDuration(tlayer);
    KheLayerSame(layer, tlayer, &same);
    KheLayerRegular(layer, tlayer, &regl);
    if( TripleGt(durn, same, regl, best_durn, best_same, best_regl) )
    {
      best_tlayer = tlayer;
      best_durn = durn;
      best_same = same;
      best_regl = regl;
    }
  }
  MAssert(best_tlayer != NULL, "KheFindTemplateLayer internal error 2");
  if( DEBUG8 )
  {
    fprintf(stderr, "  best_tlayer:\n");
    KheLayerDebug(best_tlayer, 2, 2, stderr);
    fprintf(stderr, "] KheFindTemplateLayer returning best_tlayer\n");
  }
  return best_tlayer;
}
*** */


/*****************************************************************************/
/*                                                                           */
/*  Submodule "layered time assignment"                                      */
/*                                                                           */
/*****************************************************************************/

/*****************************************************************************/
/*                                                                           */
/*  int KheLayerSaturationDegreeCmp(const void *t1, const void *t2)          */
/*                                                                           */
/*  Sort soln layers so that visited ones come first, and the rest are       */
/*  sorted according to the saturation degree heuristic.                     */
/*                                                                           */
/*****************************************************************************/

static int KheLayerSaturationDegreeCmp(const void *t1, const void *t2)
{
  KHE_LAYER layer1 = * (KHE_LAYER *) t1;
  KHE_LAYER layer2 = * (KHE_LAYER *) t2;
  int demand1, demand2;
  int assigned_duration1, assigned_duration2;
  if( KheLayerVisitNum(layer1) != KheLayerVisitNum(layer2) )
    return KheLayerVisitNum(layer2) - KheLayerVisitNum(layer1);
  if( KheLayerDuration(layer1) != KheLayerDuration(layer2) )
    return KheLayerDuration(layer2) - KheLayerDuration(layer1);
  assigned_duration1 = KheLayerAssignedDuration(layer1);
  assigned_duration2 = KheLayerAssignedDuration(layer2);
  if( assigned_duration1 != assigned_duration2 )
    return assigned_duration2 - assigned_duration1;
  demand1 = KheLayerDemand(layer1);
  demand2 = KheLayerDemand(layer2);
  if( demand1 != demand2 )
    return demand2 - demand1;
  else
    return KheLayerIndex(layer1) - KheLayerIndex(layer2);
}


/*****************************************************************************/
/*                                                                           */
/*  KHE_SPREAD_EVENTS_CONSTRAINT BestSpreadEventsConstraint(KHE_INSTANCE ins)*/
/*                                                                           */
/*  Return the best spread events constraint of ins, or NULL if none.        */
/*                                                                           */
/*****************************************************************************/

static KHE_SPREAD_EVENTS_CONSTRAINT BestSpreadEventsConstraint(KHE_INSTANCE ins)
{
  KHE_CONSTRAINT c, best_c;  int i;
  best_c = NULL;
  for( i = 0;  i < KheInstanceConstraintCount(ins);  i++ )
  {
    c = KheInstanceConstraint(ins, i);
    if( KheConstraintTag(c) == KHE_SPREAD_EVENTS_CONSTRAINT_TAG &&
	(best_c == NULL ||
	 KheConstraintAppliesToCount(c) > KheConstraintAppliesToCount(best_c)) )
      best_c = c;
  }
  return (KHE_SPREAD_EVENTS_CONSTRAINT) best_c;
}


/*****************************************************************************/
/*                                                                           */
/*  void KheSolnDebugDefectsWithTag(KHE_SOLN soln, KHE_MONITOR_TAG tag)      */
/*                                                                           */
/*  Helper function for printing defects with the given tag.                 */
/*                                                                           */
/*****************************************************************************/

static void KheSolnDebugDefectsWithTag(KHE_SOLN soln, KHE_MONITOR_TAG tag)
{
  KHE_MONITOR m;  int i;
  for( i = 0;  i < KheSolnDefectCount(soln);  i++ )
  {
    m = KheSolnDefect(soln, i);
    if( KheMonitorTag(m) == tag )
      KheMonitorDebug(m, 2, 4, stderr);
  }
}


/*****************************************************************************/
/*                                                                           */
/*  bool KheNodeLayeredAssignTimes(KHE_NODE parent_node,                     */
/*    bool vizier_splits, bool regular)                                      */
/*                                                                           */
/*  Assign times to the meets of the child nodes of parent_node, layer       */
/*  by layer.  If vizier_splits is true, parent_node is a vizier node and    */
/*  node meet splits should be applied to it before repairing each layer.    */
/*  If regular is true, try for node regularity.                             */
/*                                                                           */
/*****************************************************************************/

bool KheNodeLayeredAssignTimes(KHE_NODE parent_node,
  bool vizier_splits, bool regular)
{
  KHE_LAYER layer;  int i;  KHE_SOLN soln;
  bool added_layers;
  bool ejection_chain_repair = true;
  if( DEBUG7 )
    fprintf(stderr, "[ KheNodeLayeredAssignTimes(Node %d, %s)\n",
      KheNodeIndex(parent_node), regular ? "true" : "false");
  if( DEBUG_GRAPH )
    fprintf(stderr, "  @Data points { plus } pairs { dashed }\n  {\n");
  soln = KheNodeSoln(parent_node);
  added_layers = (KheNodeChildLayerCount(parent_node) == 0);
  if( added_layers )
    KheNodeChildLayersMake(parent_node);
  for( i = 0;  i < KheNodeChildLayerCount(parent_node);  i++ )
  {
    KheNodeChildLayersSort(parent_node, &KheLayerSaturationDegreeCmp);
    layer = KheNodeChildLayer(parent_node, i);
    if( i == 0 )
    {
      if( regular )
	KheLayerParallelAssignTimes(layer);
      KheLayerMatchAssignTimes(layer, !regular ? NULL :
	BestSpreadEventsConstraint(KheSolnInstance(KheNodeSoln(parent_node))));
      if( DEBUG_GRAPH )
	fprintf(stderr, "    %d %d\n", 2 * i + 1, DEBUG_GRAPH_HARD ?
	  KheSolnMatchingDefectCount(soln) : KheSoftCost(KheSolnCost(soln)));
      if( ejection_chain_repair )
      {
	if( vizier_splits )
	  KheNodeMeetSplit(parent_node, false);
	KheEjectionChainRepairTimes(parent_node);
	if( vizier_splits )
	  KheNodeMeetMerge(parent_node);
	if( DEBUG_GRAPH )
	  fprintf(stderr, "    %d %d\n", 2 * i + 2, DEBUG_GRAPH_HARD ?
	    KheSolnMatchingDefectCount(soln) : KheSoftCost(KheSolnCost(soln)));
      }
      if( regular )
	KheLayerInstallZonesInParent(layer);
    }
    else
    {
      KheLayerMatchAssignTimes(layer, NULL);
      if( DEBUG_GRAPH )
	fprintf(stderr, "    %d %d\n", 2 * i + 1, DEBUG_GRAPH_HARD ?
	  KheSolnMatchingDefectCount(soln) : KheSoftCost(KheSolnCost(soln)));
      if( ejection_chain_repair &&
	  (DEBUG_GRAPH_REPAIR_ALL_LAYERS || i == DEBUG_GRAPH_REPAIR_LAYER) )
      {
	if( vizier_splits )
	  KheNodeMeetSplit(parent_node, false);
	KheEjectionChainRepairTimes(parent_node);
	if( vizier_splits )
	  KheNodeMeetMerge(parent_node);
	if( DEBUG_GRAPH )
	  fprintf(stderr, "    %d %d\n", 2 * i + 2, DEBUG_GRAPH_HARD ?
	    KheSolnMatchingDefectCount(soln) : KheSoftCost(KheSolnCost(soln)));
      }
    }
    KheLayerSetVisitNum(layer, 1);
    if( DEBUG4 )
    {
      fprintf(stderr, "  after assigning layer %d: ", i);
      KheLayerDebug(layer, 1, 0, stderr);
      KheSolnCostByTypeDebug(soln, 2, 4, stderr);
      KheSolnDebugDefectsWithTag(soln, KHE_ORDINARY_DEMAND_MONITOR_TAG);
      KheSolnDebugDefectsWithTag(soln, KHE_SPREAD_EVENTS_MONITOR_TAG);
      KheSolnDebugDefectsWithTag(soln, KHE_EVENNESS_MONITOR_TAG);
    }
  }
  if( added_layers )
    KheNodeChildLayersDelete(parent_node);
  if( DEBUG_GRAPH )
    fprintf(stderr, "  }\n");
  if( DEBUG7 )
    fprintf(stderr, "] KheNodeLayeredAssignTimes returning %s\n",
      KheNodeAllChildMeetsAssigned(parent_node) ? "true" : "false");
  return KheNodeAllChildMeetsAssigned(parent_node);
} 


/*****************************************************************************/
/*                                                                           */
/*  bool KheNodeChildLayersAssignTimes(KHE_NODE parent_node)                 */
/*                                                                           */
/*  Assign times by soln layer.                                              */
/*                                                                           */
/*****************************************************************************/

/* ***
bool KheNodeChildLayersAssignTimes(KHE_NODE parent_node)
{
  KHE_LAYER layer;  int i;  KHE_SPREAD_EVENTS_CONSTRAINT sec;

  ** bool zone_repair_times = true; **
  bool layer_node_matching_repair_times = true;
  ** bool node_meet_swap_repair_times = true;  a bit slow; it works, though **
  bool ejection_chain_repair = true;
  bool at_root = KheNodeMeetCount(parent_node) > 0 &&
      KheMeetIsCycleMeet(KheNodeMeet(parent_node, 0));
  if( DEBUG1 || (DEBUG4 && at_root) )
    fprintf(stderr, "[ KheNodeChildLayersAssignTimes(Node %d)\n",
      KheNodeIndex(parent_node));
  MAssert(KheNodeChildLayerCount(parent_node) == 0,
    "KheNodeChildLayersAssignTimes: parent_node already has layers");
  KheNodeChildLayersMake(parent_node);
  for( i = 0;  i < KheNodeChildLayerCount(parent_node);  i++ )
  {
    KheNodeChildLayersSort(parent_node, &KheLayerSaturationDegreeCmp);
    layer = KheNodeChildLayer(parent_node, i);
    if( i == 0 )
    {
      KheLayerParallelAssignTimes(layer);
      sec = !at_root ? NULL :
	BestSpreadEventsConstraint(KheSolnInstance(KheNodeSoln(parent_node)));
      KheLayerMatchAssignTimes(layer, sec);
      if( ejection_chain_repair )
	KheEjectionChainRepairTimes(parent_node);
      ** KheEjectionChainLayerRepairTimesOld(layer); **
      if( at_root )
	KheLayerInstallZonesInParent(layer);
    }
    else
    {
      KheLayerMatchAssignTimes(layer, NULL);
      if( layer_node_matching_repair_times )
	KheLayerNodeMatchingLayerRepairTimes(layer);
      ** ***
      if( zone_repair_times )
	KheLayerZoneRepairTimes(layer);
      if( layer_node_matching_repair_times )
      {
	KheLayerNodeMatchingLayerRepairTimes(layer);
	KheLayerNodeMatchingNodeRepairTimes(parent_node);
      }
      if( zone_repair_times )
	KheLayerZoneRepairTimes(layer);
      if( node_meet_swap_repair_times )
	KheNodeMeetSwapRepairTimes(parent_node);
      if( ejection_chain )
	KheEjectionChainRepairTimes(parent_node);
      *** **
      if( ejection_chain_repair )
	KheEjectionChainRepairTimes(parent_node);
    }
    KheLayerSetVisitNum(layer, 1);
    if( DEBUG4 && at_root )
    {
      KHE_SOLN soln;
      fprintf(stderr, "  after assigning layer %d: ", i);
      KheLayerDebug(layer, 1, 0, stderr);
      soln = KheNodeSoln(parent_node);
      KheSolnCostByTypeDebug(soln, 2, 4, stderr);
      KheSolnDebugDefectsWithTag(soln, KHE_ORDINARY_DEMAND_MONITOR_TAG);
      KheSolnDebugDefectsWithTag(soln, KHE_SPREAD_EVENTS_MONITOR_TAG);
      KheSolnDebugDefectsWithTag(soln, KHE_EVENNESS_MONITOR_TAG);
    }
  }
  KheNodeChildLayersDelete(parent_node);
  if( DEBUG1 || (DEBUG4 && at_root) )
  {
    if( !at_root )
    {
      fprintf(stderr, "  timetable for entire node at end:\n");
      KheNodePrintTimetable(parent_node, 15, 4, stderr);
    }
    fprintf(stderr, "] KheNodeChildLayersAssignTimes returning %s\n",
      KheNodeAllChildMeetsAssigned(parent_node) ? "true" : "false");
  }
  return KheNodeAllChildMeetsAssigned(parent_node);
} 
*** */
