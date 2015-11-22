
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
/*  FILE:         khe_layer_solve.c                                          */
/*  DESCRIPTION:  Various layer solvers                                      */
/*                                                                           */
/*****************************************************************************/
#include "khe.h"
#include "m.h"

#define DEBUG1 0
#define DEBUG2 0
#define DEBUG3 0
#define DEBUG4 0
#define DEBUG5 0
#define DEBUG6 0


/*****************************************************************************/
/*                                                                           */
/*  Submodule "layer construction"                                           */
/*                                                                           */
/*****************************************************************************/

/*****************************************************************************/
/*                                                                           */
/*  KHE_NODE KheEnclosingNode(KHE_MEET meet, KHE_NODE parent_node)           */
/*                                                                           */
/*  Return the child node of parent_node that meet lies within or below,     */
/*  or NULL if none.                                                         */
/*                                                                           */
/*****************************************************************************/

static KHE_NODE KheEnclosingNode(KHE_MEET meet, KHE_NODE parent_node)
{
  KHE_NODE res;
  while( KheMeetNode(meet) == NULL && KheMeetAsst(meet) != NULL )
    meet = KheMeetAsst(meet);
  if( KheMeetNode(meet) == NULL )
    return NULL;
  res = KheMeetNode(meet);
  while( KheNodeParent(res) != NULL && KheNodeParent(res) != parent_node )
    res = KheNodeParent(res);
  return KheNodeParent(res) == parent_node ? res : NULL;
}


/*****************************************************************************/
/*                                                                           */
/*  KHE_LAYER KheLayerMakeInternal(KHE_NODE prnt, KHE_LAYERING slg,          */
/*    KHE_RESOURCE r)                                                        */
/*                                                                           */
/*  Make a new layer.  Both slg and r are optional.  If r is present, the    */
/*  appropriate children of prnt are added to the layer.                     */
/*                                                                           */
/*****************************************************************************/

KHE_LAYER KheLayerMakeFromResource(KHE_NODE parent_node, KHE_RESOURCE r)
{
  KHE_LAYER res;  KHE_SOLN soln;  KHE_EVENT e;  KHE_MEET meet;
  KHE_NODE node;  int i, j;
  MAssert(r != NULL, "KheLayerMakeFromResource: r is NULL");
  res = KheLayerMake(parent_node);
  KheLayerAddResource(res, r);
  soln = KheNodeSoln(parent_node);
  for( i = 0;  i < KheResourceLayerEventCount(r);  i++ )
  {
    e = KheResourceLayerEvent(r, i);
    for( j = 0;  j < KheEventMeetCount(soln, e);  j++ )
    {
      meet = KheEventMeet(soln, e, j);
      node = KheEnclosingNode(meet, parent_node);
      if( node != NULL && !KheLayerContains(res, node) )
	KheLayerAddChildNode(res, node);
    }
  }
  return res;
}


/*****************************************************************************/
/*                                                                           */
/*  bool KheLayerIsSubset(KHE_NODE parent_node, KHE_LAYER layer,             */
/*    KHE_LAYER *layer2)                                                     */
/*                                                                           */
/*  If layer is a subset of another layer of parent_node, set *layer2 to     */
/*  the first such layer and return true, otherwise return false.            */
/*                                                                           */
/*****************************************************************************/

static bool KheLayerIsSubset(KHE_NODE parent_node, KHE_LAYER layer,
  KHE_LAYER *layer2)
{
  int i;  KHE_LAYER l2;
  for( i = 0;  i < KheNodeChildLayerCount(parent_node);  i++ )
  {
    l2 = KheNodeChildLayer(parent_node, i);
    if( l2 != layer && KheLayerSubset(layer, l2) )
    {
      *layer2 = l2;
      return true;
    }
  }
  *layer2 = NULL;
  return false;
}


/*****************************************************************************/
/*                                                                           */
/*  void KheLayerPrune(KHE_NODE parent_node, KHE_LAYER *layer)               */
/*                                                                           */
/*  Merge into *layer any layers of parent_node that are subsets of *layer.  */
/*                                                                           */
/*****************************************************************************/

static void KheLayerPrune(KHE_NODE parent_node, KHE_LAYER *layer)
{
  KHE_LAYER layer2;  int i;
  for( i = 0;  i < KheNodeChildLayerCount(parent_node);  i++ )
  {
    layer2 = KheNodeChildLayer(parent_node, i);
    if( layer2 != *layer && KheLayerSubset(layer2, *layer) )
    {
      KheLayerMerge(*layer, layer2, layer);
      i--;
    }
  }
}


/*****************************************************************************/
/*                                                                           */
/*  int KheLayerDefaultCmp(const void *t1, const void *t2)                   */
/*                                                                           */
/*  Comparison functions for sorting an array of layers by decreasing size.  */
/*                                                                           */
/*****************************************************************************/

int KheLayerDefaultCmp(const void *t1, const void *t2)
{
  KHE_LAYER layer1 = * (KHE_LAYER *) t1;
  KHE_LAYER layer2 = * (KHE_LAYER *) t2;
  if( KheLayerDuration(layer1) != KheLayerDuration(layer2) )
    return KheLayerDuration(layer2) - KheLayerDuration(layer1);
  else if( KheLayerChildNodeCount(layer1) != KheLayerChildNodeCount(layer2) )
    return KheLayerChildNodeCount(layer1) - KheLayerChildNodeCount(layer2);
  else
    return KheLayerIndex(layer1) - KheLayerIndex(layer2);
}


/*****************************************************************************/
/*                                                                           */
/*  void KheNodeChildLayersMake(KHE_NODE parent_node)                        */
/*                                                                           */
/*  Make a the usual child layers for this parent_node.                      */
/*                                                                           */
/*****************************************************************************/

void KheNodeChildLayersMake(KHE_NODE parent_node)
{
  KHE_SOLN soln;  KHE_INSTANCE ins;  int i;
  KHE_RESOURCE r;  KHE_LAYER layer, layer2;  KHE_NODE child_node;

  /* delete all existing child layers */
  KheNodeChildLayersDelete(parent_node);

  /* add layers based on all resources */
  soln = KheNodeSoln(parent_node);
  ins = KheSolnInstance(soln);
  for( i = 0;  i < KheInstanceResourceCount(ins);  i++ )
  {
    r = KheInstanceResource(ins, i);
    if( KheResourceLayerEventCount(r) > 0 )
    {
      layer = KheLayerMakeFromResource(parent_node, r);
      if( KheLayerChildNodeCount(layer) == 0 )
	KheLayerDelete(layer);
      else if( KheLayerIsSubset(parent_node, layer, &layer2) )
	KheLayerMerge(layer2, layer, &layer2);
      else
      {
	/* remove any existing layers that are subsets of layer */
        KheLayerPrune(parent_node, &layer);
      }
    }
  }

  /* add one layer for each child node not otherwise included */
  for( i = 0;  i < KheNodeChildCount(parent_node);  i++ )
  {
    child_node = KheNodeChild(parent_node, i);
    if( KheNodeParentLayerCount(child_node) == 0 )
    {
      /* child_node missed out, so add one layer just for it */
      layer = KheLayerMake(parent_node);
      KheLayerAddChildNode(layer, child_node);
    }
  }

  /* sort by decreasing duration and increasing number of nodes */
  KheNodeChildLayersSort(parent_node, &KheLayerDefaultCmp);
}


/*****************************************************************************/
/*                                                                           */
/*  Submodule "layer duration reduction" (private)                           */
/*                                                                           */
/*****************************************************************************/

/*****************************************************************************/
/*                                                                           */
/*  KHE_NODE_REDUCE - a node plus info about its node durations              */
/*                                                                           */
/*****************************************************************************/

typedef struct khe_node_reduce_rec {
  KHE_NODE			node;			/* the node          */
  ARRAY_INT			node_durations;		/* node durations    */
  int				total_node_duration;	/* sum of above      */
} *KHE_NODE_REDUCE;

typedef MARRAY(KHE_NODE_REDUCE) ARRAY_KHE_NODE_REDUCE;


/*****************************************************************************/
/*                                                                           */
/*  KHE_NODE_REDUCE KheNodeReduceMake(KHE_NODE node)                         */
/*                                                                           */
/*  Make a node reduce object, initially with no durations.                  */
/*                                                                           */
/*****************************************************************************/

static KHE_NODE_REDUCE KheNodeReduceMake(KHE_NODE node)
{
  KHE_NODE_REDUCE res;
  MMake(res);
  res->node = node;
  MArrayInit(res->node_durations);
  res->total_node_duration = 0;
  return res;
}


/*****************************************************************************/
/*                                                                           */
/*  void KheNodeReduceDelete(KHE_NODE_REDUCE nr)                             */
/*                                                                           */
/*  Free nr.                                                                 */
/*                                                                           */
/*****************************************************************************/

static void KheNodeReduceDelete(KHE_NODE_REDUCE nr)
{
  MArrayFree(nr->node_durations);
  MFree(nr);
}


/*****************************************************************************/
/*                                                                           */
/*  void KheNodeReduceAddDuration(KHE_NODE_REDUCE nr, int durn)              */
/*                                                                           */
/*  Add one duration to nr.                                                  */
/*                                                                           */
/*****************************************************************************/

static void KheNodeReduceAddDuration(KHE_NODE_REDUCE nr, int durn)
{
  MArrayAddLast(nr->node_durations, durn);
  nr->total_node_duration += durn;
}


/*****************************************************************************/
/*                                                                           */
/*  int KheNodeReduceCmp(const void *t1, const void *t2)                     */
/*                                                                           */
/*  Comparison function for sorting node reduce objects into increasing      */
/*  total duration order.                                                    */
/*                                                                           */
/*****************************************************************************/

static int KheNodeReduceCmp(const void *t1, const void *t2)
{
  KHE_NODE_REDUCE nr1 = * (KHE_NODE_REDUCE *) t1;
  KHE_NODE_REDUCE nr2 = * (KHE_NODE_REDUCE *) t2;
  if( nr1->total_node_duration != nr2->total_node_duration )
    return nr1->total_node_duration - nr2->total_node_duration;
  else
    return KheNodeIndex(nr1->node) - KheNodeIndex(nr2->node);
}


/*****************************************************************************/
/*                                                                           */
/*  bool KheNodeReduceCanGoUnder(KHE_NODE_REDUCE nr1, KHE_NODE_REDUCE nr2)   */
/*                                                                           */
/*  Return true if nr1 can go under nr2.                                     */
/*                                                                           */
/*****************************************************************************/

static bool KheNodeReduceCanGoUnder(KHE_NODE_REDUCE nr1, KHE_NODE_REDUCE nr2)
{
  int i, durn1, durn2;
  MAssert(MArraySize(nr1->node_durations) == MArraySize(nr2->node_durations),
    "KheNodeReduceCanGoUnder internal error");
  for( i = 0;  i < MArraySize(nr1->node_durations);  i++ )
  {
    durn1 = MArrayGet(nr1->node_durations, i);
    durn2 = MArrayGet(nr2->node_durations, i);
    if( durn1 + durn2 > KheNodeDuration(nr2->node) )
      return false;
  }
  return true;
}


/*****************************************************************************/
/*                                                                           */
/*  bool KheNodeCanBeSubsumed(ARRAY_KHE_NODE_REDUCE *node_reduces,           */
/*    KHE_NODE_REDUCE nr, KHE_NODE_REDUCE *nr2)                              */
/*                                                                           */
/*  Search node_reduces for the first node that nr can go under, and         */
/*  return true, setting *nr2 to that node, if found.                        */
/*                                                                           */
/*****************************************************************************/

static bool KheNodeCanBeSubsumed(ARRAY_KHE_NODE_REDUCE *node_reduces,
  KHE_NODE_REDUCE nr, KHE_NODE_REDUCE *nr2)
{
  int i;  KHE_NODE_REDUCE nr1;
  MArrayForEach(*node_reduces, &nr1, &i)
    if( nr != nr1 && KheNodeReduceCanGoUnder(nr, nr1) )
    {
      *nr2 = nr1;
      return true;
    }
  return false;
}


/*****************************************************************************/
/*                                                                           */
/*  void KheNodeReduceMergeInto(KHE_NODE_REDUCE nr1, KHE_NODE_REDUCE nr2)    */
/*                                                                           */
/*  Merge nr1 into nr2, removing all trace of nr1.                           */
/*                                                                           */
/*****************************************************************************/

static void KheNodeReduceMergeInto(KHE_NODE_REDUCE nr1, KHE_NODE_REDUCE nr2)
{
  int i, durn1, durn2;
  for( i = 0;  i < MArraySize(nr1->node_durations);  i++ )
  {
    durn1 = MArrayGet(nr1->node_durations, i);
    durn2 = MArrayGet(nr2->node_durations, i);
    MArrayPut(nr2->node_durations, i, durn1 + durn2);
  }
  nr2->total_node_duration += nr1->total_node_duration;
  KheNodeMove(nr1->node, nr2->node);
  KheNodeReduceDelete(nr1);
  if( DEBUG5 )
    KheNodeDebug(nr2->node, 3, 4, stderr);
}


/*****************************************************************************/
/*                                                                           */
/*  void KheNodeReduceDebug(KHE_NODE_REDUCE nr, int indent, FILE *fp)        */
/*                                                                           */
/*  One-line debug print of nr.                                              */
/*                                                                           */
/*****************************************************************************/

static void KheNodeReduceDebug(KHE_NODE_REDUCE nr, int indent, FILE *fp)
{
  int durn, i;
  fprintf(stderr, "%*sNode %3d (duration %d, resource durn %2d:", indent, "",
    KheNodeIndex(nr->node), KheNodeDuration(nr->node),
    nr->total_node_duration);
  MArrayForEach(nr->node_durations, &durn, &i)
    fprintf(stderr, " %d", durn);
  fprintf(stderr, ")\n");
}


/*****************************************************************************/
/*                                                                           */
/*  bool KheCoordinateReduceLayerDuration(KHE_LAYER layer, int over)         */
/*                                                                           */
/*  Try to reduce the duration of layer.  It currently exceeds the duration  */
/*  of its parent node by over.  Return true if anything changed.            */
/*                                                                           */
/*****************************************************************************/

static bool KheCoordinateReduceLayerDuration(KHE_LAYER layer, int over)
{
  KHE_NODE parent_node, node;  KHE_RESOURCE r;  int i, j;  bool res;
  ARRAY_KHE_NODE_REDUCE node_reduces;  KHE_NODE_REDUCE nr1, nr2;
  if( DEBUG5 )
  {
    fprintf(stderr, "[ KheCoordinateReduceLayerDuration(layer, %d):\n", over);
    KheLayerDebug(layer, 2, 2, stderr);
  }

  /* build and sort the node reduce objects */
  parent_node = KheLayerParentNode(layer);
  MArrayInit(node_reduces);
  for( i = 0;  i < KheLayerChildNodeCount(layer);  i++ )
  {
    node = KheLayerChildNode(layer, i);
    nr1 = KheNodeReduceMake(node);
    for( j = 0;  j < KheLayerResourceCount(layer);  j++ )
    {
      r = KheLayerResource(layer, j);
      KheNodeReduceAddDuration(nr1, KheNodeResourceDuration(node, r));
    }
    MArrayAddLast(node_reduces, nr1);
  }
  MArraySort(node_reduces, &KheNodeReduceCmp);
  if( DEBUG5 )
    MArrayForEach(node_reduces, &nr1, &i)
      KheNodeReduceDebug(nr1, 2, stderr);

  /* try each node in turn until all tried or overload fixed */
  res = false;
  for( i = 0;  i < MArraySize(node_reduces) && over > 0;  i++ )
  {
    nr1 = MArrayGet(node_reduces, i);
    if( KheNodeCanBeSubsumed(&node_reduces, nr1, &nr2) )
    {
      if( DEBUG5 )
      {
	fprintf(stderr, "    moving node:\n");
	KheNodeReduceDebug(nr1, 6, stderr);
	KheNodeReduceDebug(nr2, 6, stderr);
      }
      over -= KheNodeDuration(nr1->node);
      KheNodeReduceMergeInto(nr1, nr2);
      MArrayRemove(node_reduces, i);
      i--;
      res = true;
    }
  }

  /* clean up and return */
  MArrayForEach(node_reduces, &nr1, &i)
    KheNodeReduceDelete(nr1);
  MArrayFree(node_reduces);
  if( DEBUG5 )
    fprintf(stderr, "] KheCoordinateReduceLayerDuration returning %s\n",
      res ? "true" : "false");
  return res;
}


/*****************************************************************************/
/*                                                                           */
/*  Submodule "layer coordination"                                           */
/*                                                                           */
/*****************************************************************************/

/*****************************************************************************/
/*                                                                           */
/*  bool LayerIsOfInterest(KHE_LAYER layer)                                  */
/*                                                                           */
/*  Return true of layer is of interest (is a candidate for coordinating).   */
/*                                                                           */
/*****************************************************************************/

static bool LayerIsOfInterest(KHE_LAYER layer)
{
  return KheResourcePartition(KheLayerResource(layer, 0)) != NULL &&
    KheLayerDuration(layer) >= 0.9*KheNodeDuration(KheLayerParentNode(layer));
}


/*****************************************************************************/
/*                                                                           */
/*  bool KheLayerDominates(KHE_LAYER layer1, KHE_LAYER layer2,               */
/*    int similar_count)                                                     */
/*                                                                           */
/*  When this function is called, node layers layer1 and layer2 have been    */
/*  subjected to KheLayerSimilar, which returned false with the given        */
/*  similar_count.  That being the case, return true if layer1 dominates     */
/*  layer2.                                                                  */
/*                                                                           */
/*  Soln layer layer1 dominates soln layer layer2 if the duration of         */
/*  layer1 is equal to the duration of the parent node, and all but one of   */
/*  the nodes of layer1 is counted in similar_count.  In that case, the      */
/*  leftover nodes of layer2 might as well be moved to below the one         */
/*  leftover node of layer1.                                                 */
/*                                                                           */
/*****************************************************************************/

static bool KheLayerDominates(KHE_LAYER layer1, KHE_LAYER layer2,
  int similar_count)
{
  return KheLayerDuration(layer1)==KheNodeDuration(KheLayerParentNode(layer1))
    && similar_count == KheLayerChildNodeCount(layer1) - 1;
}


/*****************************************************************************/
/*                                                                           */
/*  void KheCoordinateTwoLayers(KHE_LAYER layer1, KHE_LAYER layer2,          */
/*    int similar_count)                                                     */
/*                                                                           */
/*  Coordinate layer1 and layer2, whose first similar_count nodes are known  */
/*  to be similar; the remaining layer2 nodes are to be made children of     */
/*  the last layer1 node.                                                    */
/*                                                                           */
/*****************************************************************************/

static void KheCoordinateTwoLayers(KHE_LAYER layer1, KHE_LAYER layer2,
  int similar_count)
{
  KHE_NODE node1, node2;  int i1, i2;
  if( DEBUG2 )
    fprintf(stderr, "  [ KheCoordinateTwoLayers(%s:%d, %s:%d, %d):\n",
      KheResourceId(KheLayerResource(layer1, 0)) != NULL ?
	KheResourceId(KheLayerResource(layer1, 0)) : "-",
      KheLayerChildNodeCount(layer1),
      KheResourceId(KheLayerResource(layer2, 0)) != NULL ?
        KheResourceId(KheLayerResource(layer2, 0)) : "-",
      KheLayerChildNodeCount(layer2), similar_count);

  /* move the first similar_count layer2 nodes to be children of layer1 nodes */
  for( i1 = i2 = 0;  i1 < similar_count;  i1++, i2++ )
  {
    node1 = KheLayerChildNode(layer1, i1);
    node2 = KheLayerChildNode(layer2, i2);
    if( node1 != node2 )
    {
      if( DEBUG2 )
      {
	fprintf(stderr, "      moving Node %d to child of Node %d\n",
	  KheNodeIndex(node2), KheNodeIndex(node1));
	KheNodeDebug(node2, 2, 6, stderr);
	KheNodeDebug(node1, 2, 6, stderr);
      }
      if( !KheNodeMove(node2, node1) )
	MAssert(false, "KheCoordinateTwoLayers internal error 3");
      i2--;
    }
  }

  /* move the remaining layer2 nodes to be children of the last layer1 node */
  if( similar_count < KheLayerChildNodeCount(layer1) )
  {
    node1 = KheLayerChildNode(layer1, similar_count);
    for( ;  i2 < KheLayerChildNodeCount(layer2);  i2++ )
    {
      node2 = KheLayerChildNode(layer2, i2);
      MAssert(node1 != node2, "KheCoordinateTwoLayers internal error 4");
      if( DEBUG2 )
      {
	fprintf(stderr, "      moving extra Node %d to child of Node %d\n",
	  KheNodeIndex(node2), KheNodeIndex(node1));
	KheNodeDebug(node2, 2, 6, stderr);
	KheNodeDebug(node1, 2, 6, stderr);
      }
      if( !KheNodeMove(node2, node1) )
	MAssert(false, "KheCoordinateTwoLayers internal error 5");
      i2--;
    }
  }
  if( DEBUG2 )
    fprintf(stderr, "  ] KheCoordinateTwoLayers returning\n");
}


/*****************************************************************************/
/*                                                                           */
/*  void KheCoordinateResources(KHE_RESOURCE r1, KHE_RESOURCE r2,            */
/*    KHE_NODE parent_node, bool with_domination)                            */
/*                                                                           */
/*  Coordinate resources r1 and r2.  These are known to lie in the same      */
/*  resource partition, and the duration of their preassigned events is      */
/*  enough to suggest that they might be of interest.                        */
/*                                                                           */
/*****************************************************************************/

static void KheCoordinateResources(KHE_RESOURCE r1, KHE_RESOURCE r2,
  KHE_NODE parent_node, bool with_domination)
{
  KHE_LAYER layer1, layer2;  int similar_count;
  if( DEBUG1 )
  {
    fprintf(stderr, "  [ KheCoordinateResources(");
    KheResourceDebug(r1, 1, -1, stderr);
    fprintf(stderr, ", ");
    KheResourceDebug(r2, 1, -1, stderr);
    fprintf(stderr, ", Node %d, %s)\n", KheNodeIndex(parent_node),
      with_domination ? "true" : "false");
  }

  /* make the two soln layers and return if they are not interesting */
  layer1 = KheLayerMakeFromResource(parent_node, r1);
  if( !LayerIsOfInterest(layer1) )
  {
    KheLayerDelete(layer1);
    if( DEBUG1 )
      fprintf(stderr, "  ] KheCoordinateResources returning (r1 no use)\n");
    return;
  }
  layer2 = KheLayerMakeFromResource(parent_node, r2);
  if( !LayerIsOfInterest(layer2) )
  {
    KheLayerDelete(layer2);
    if( DEBUG1 )
      fprintf(stderr, "  ] KheCoordinateResources returning (r2 no use)\n");
    return;
  }

  /* try to match up the nodes of the two layers */
  if( KheLayerSimilar(layer1, layer2, &similar_count) )
    KheCoordinateTwoLayers(layer1, layer2, similar_count);
  else if( with_domination && KheLayerDominates(layer1, layer2, similar_count) )
    KheCoordinateTwoLayers(layer1, layer2, similar_count);
  else if( with_domination && KheLayerDominates(layer2, layer1, similar_count) )
    KheCoordinateTwoLayers(layer2, layer1, similar_count);

  /* finished with the node layer objects */
  KheLayerDelete(layer1);
  KheLayerDelete(layer2);
  if( DEBUG1 )
    fprintf(stderr, "  ] KheCoordinateResources returning\n");
}


/*****************************************************************************/
/*                                                                           */
/*  void KheCoordinateResourcePartition(KHE_RESOURCE_GROUP rg,               */
/*    KHE_NODE parent_node, bool with_domination)                            */
/*                                                                           */
/*  Coordinate the resources of partition rg.                                */
/*                                                                           */
/*****************************************************************************/

static void KheCoordinateResourcePartition(KHE_RESOURCE_GROUP rg,
  KHE_NODE parent_node, bool with_domination)
{
  int i1, i2;  KHE_RESOURCE r1, r2;
  if( DEBUG1 )
  {
    fprintf(stderr, "[ KheCoordinateResourcePartition(");
    KheResourceGroupDebug(rg, 1, -1, stderr);
    fprintf(stderr, ", Node %d, %s)\n", KheNodeIndex(parent_node),
      with_domination ? "true" : "false");
  }
  for( i1 = 0;  i1 < KheResourceGroupResourceCount(rg);  i1++ )
  {
    r1 = KheResourceGroupResource(rg, i1);
    if( KheResourceLayerDuration(r1) >= 0.9 * KheNodeDuration(parent_node) )
    {
      for( i2 = i1 + 1;  i2 < KheResourceGroupResourceCount(rg);  i2++ )
      {
	r2 = KheResourceGroupResource(rg, i2);
	if( KheResourceLayerDuration(r2) >= 0.9 * KheNodeDuration(parent_node) )
	  KheCoordinateResources(r1, r2, parent_node, with_domination);
      }
    }
  }
  if( DEBUG1 )
    fprintf(stderr, "] KheCoordinateResourcePartition returning\n");
}


/*****************************************************************************/
/*                                                                           */
/*  void KheCoordinateLayers(KHE_NODE parent_node, bool with_domination)     */
/*                                                                           */
/*  Solver for coordinating the soln layers of parent_node.                  */
/*                                                                           */
/*****************************************************************************/

void KheCoordinateLayers(KHE_NODE parent_node, bool with_domination)
{
  int i, j;  KHE_RESOURCE_TYPE rt;  KHE_RESOURCE_GROUP rg;  KHE_INSTANCE ins;
  KHE_LAYER layer;
  if( DEBUG1 )
    fprintf(stderr, "[ KheCoordinateLayers(Node %d)\n",
      KheNodeIndex(parent_node));

  /* do the coordinating proper */
  ins = KheSolnInstance(KheNodeSoln(parent_node));
  for( i = 0;  i < KheInstanceResourceTypeCount(ins);  i++ )
  {
    rt = KheInstanceResourceType(ins, i);
    if( KheResourceTypeHasPartitions(rt) )
      for( j = 0;  j < KheResourceTypeResourceGroupCount(rt);  j++ )
      {
	rg = KheResourceTypeResourceGroup(rt, j);
	if( KheResourceGroupIsPartition(rg) )
	  KheCoordinateResourcePartition(rg, parent_node, with_domination);
      }
  }

  /* try to reduce the duration of oversize layers */
  KheNodeChildLayersMake(parent_node);
  for( i = 0;  i < KheNodeChildLayerCount(parent_node);  i++ )
  {
    layer = KheNodeChildLayer(parent_node, i);
    if( KheLayerDuration(layer) > KheNodeDuration(parent_node) )
    {
      if( KheCoordinateReduceLayerDuration(layer,
	  KheLayerDuration(layer) - KheNodeDuration(parent_node)) )
      {
	/* if successful, the layers change and need reconstructing */
	KheNodeChildLayersMake(parent_node);
      }
    }
  }
  KheNodeChildLayersDelete(parent_node);

  if( DEBUG1 )
  {
    fprintf(stderr, "  at end:\n");
    KheNodeDebug(parent_node, 3, 2, stderr);
    fprintf(stderr, "] KheCoordinateLayers returning\n");
  }
}


/*****************************************************************************/
/*                                                                           */
/*  Submodule "minimum runaround duration"                                   */
/*                                                                           */
/*****************************************************************************/

/*****************************************************************************/
/*                                                                           */
/*  void KheReset(KHE_NODE parent_node, int init_count)                      */
/*                                                                           */
/*  Reset the tree rooted at parent_node to its initial state, including     */
/*  unassigning all child meets, and removing all but the first init_count   */
/*  meets of parent_node.                                                    */
/*                                                                           */
/*****************************************************************************/

static void KheReset(KHE_NODE parent_node, int init_count)
{
  KheNodeUnAssignTimes(parent_node);
  while( KheNodeMeetCount(parent_node) > init_count )
    KheMeetDelete(KheNodeMeet(parent_node, init_count));
}


/*****************************************************************************/
/*                                                                           */
/*  bool KheMinimumRunaroundDuration(KHE_NODE parent_node,                   */
/*    KHE_NODE_TIME_SOLVER time_solver, int *duration)                       */
/*                                                                           */
/*  Set *duration to the minimum runaround duration of parent_node and       */
/*  return true, unless there is a problem, in which case return false       */
/*  with *duration undefined.  Use time_solver to carry out the time         */
/*  assignment trials required by this function.                             */
/*                                                                           */
/*  There is a problem if time_solver ever returns false, and if one         */
/*  trial produces a solution whose cost is not less than the cost of        */
/*  the solution produced by the previous trial.                             */
/*                                                                           */
/*****************************************************************************/

bool KheMinimumRunaroundDuration(KHE_NODE parent_node,
  KHE_NODE_TIME_SOLVER time_solver, int *duration)
{
  KHE_COST init_cost, cost;  int init_count, i, j;  KHE_SOLN soln;
  KHE_MEET orig_meet, new_meet;
  if( DEBUG3 )
    fprintf(stderr, "[ KheMinimumRunaroundDuration(Node %d)\n",
      KheNodeIndex(parent_node));

  /* deassign the children and record the cost */
  soln = KheNodeSoln(parent_node);
  KheNodeUnAssignTimes(parent_node);
  init_count = KheNodeMeetCount(parent_node);
  init_cost = KheSolnCost(soln);
  if( DEBUG3 )
    fprintf(stderr, "  child_count %d, init_cost %.4f\n",
      KheNodeChildCount(parent_node), KheCostShow(init_cost));

  /* repeatedly assign, compare costs, and add more meets if required */
  for( i = 0;  i <= KheNodeChildCount(parent_node);  i++ )
  {
    /* finish now if assigned and the cost is minimal */
    if( time_solver(parent_node) )
    {
      cost = KheSolnCost(soln);
      if( DEBUG3 )
      {
	fprintf(stderr, "  iteration %d assigned all times, cost %.4f:\n",
	  i, KheCostShow(cost));
	KheNodeDebug(parent_node, 2, 4, stderr);
	/* KheNodeMatchingDebug(parent_node, 3, 4, stderr); */
      }
      if( cost <= init_cost )
      {
	*duration = KheNodeDuration(parent_node);
	KheReset(parent_node, init_count);
	if( DEBUG3 )
	{
	  KheNodeDebug(parent_node, 2, 2, stderr);
	  fprintf(stderr, "] KheMinimumRunaroundDuration returning true (%d)\n",
	    *duration);
	}
	return true;
      }
    }

    /* no success yet, so widen parent_node, unassign, and try again */
    for( j = 0;  j < init_count;  j++ )
    {
      orig_meet = KheNodeMeet(parent_node, j);
      new_meet = KheMeetMake(soln, KheMeetDuration(orig_meet), NULL);
      if( !KheMeetSetDomain(new_meet, KheMeetDomain(orig_meet)) )
	MAssert(false, "KheMinimumRunaroundDuration internal error 1");
      if( !KheNodeAddMeet(parent_node, new_meet) )
	MAssert(false, "KheMinimumRunaroundDuration internal error 2");
    }
    KheNodeUnAssignTimes(parent_node);
  }

  /* reach here only on failure */
  if( DEBUG3 )
    fprintf(stderr, "] KheMinimumRunaroundDuration returning false\n");
  KheReset(parent_node, init_count);
  return false;
}


/*****************************************************************************/
/*                                                                           */
/*  Submodule "node templates"                                               */
/*                                                                           */
/*****************************************************************************/

/*****************************************************************************/
/*                                                                           */
/*  KHE_MEET_TEMPLATE - a template for a meet                                */
/*                                                                           */
/*****************************************************************************/

typedef struct khe_meet_template_rec {
  int			  duration;		/* duration of meet          */
  KHE_TIME_GROUP	  time_domain;		/* domain of meet            */
} *KHE_MEET_TEMPLATE;

typedef MARRAY(KHE_MEET_TEMPLATE) ARRAY_KHE_MEET_TEMPLATE;


/*****************************************************************************/
/*                                                                           */
/*  KHE_NODE_TEMPLATE - a template for a node                                */
/*                                                                           */
/*****************************************************************************/

typedef struct khe_node_template_rec {
  int			  frequency;		/* how often this occurs     */
  int			  duration;		/* duration of node          */
  ARRAY_KHE_MEET_TEMPLATE meet_templates;	/* the meet templates        */
} *KHE_NODE_TEMPLATE;

typedef MARRAY(KHE_NODE_TEMPLATE) ARRAY_KHE_NODE_TEMPLATE;
typedef MARRAY(ARRAY_KHE_NODE_TEMPLATE) ARRAY_ARRAY_KHE_NODE_TEMPLATE;


/*****************************************************************************/
/*                                                                           */
/*  KHE_NODE_TEMPLATES - an array of node templates of equal duration        */
/*                                                                           */
/*****************************************************************************/

typedef struct khe_node_templates_rec {
  int			  duration;		/* the shared duration       */
  ARRAY_KHE_NODE_TEMPLATE node_templates;	/* the templates             */
} *KHE_NODE_TEMPLATES;

typedef MARRAY(KHE_NODE_TEMPLATES) ARRAY_KHE_NODE_TEMPLATES;


/*****************************************************************************/
/*                                                                           */
/*  KHE_NODE_TEMPLATE_SET - a set of node templates, indexed by duration     */
/*                                                                           */
/*****************************************************************************/

typedef struct khe_node_template_set_rec {
  ARRAY_KHE_NODE_TEMPLATES   node_templates;	/* the templates */
} *KHE_NODE_TEMPLATE_SET;


/*****************************************************************************/
/*                                                                           */
/*  KHE_NODE_TEMPLATE KheNodeTemplateMake(void)                              */
/*                                                                           */
/*  Make a new, empty node template.                                         */
/*                                                                           */
/*****************************************************************************/

static KHE_NODE_TEMPLATE KheNodeTemplateMake(void)
{
  KHE_NODE_TEMPLATE res;
  MMake(res);
  res->frequency = 0;
  res->duration = 0;
  MArrayInit(res->meet_templates);
  return res;
}


/*****************************************************************************/
/*                                                                           */
/*  void KheNodeTemplateAddMeetTemplate(KHE_NODE_TEMPLATE nt,                */
/*    int duration, KHE_TIME_GROUP time_domain)                              */
/*                                                                           */
/*  Add (duration, time_domain) as a meet template to nt.  Make sure the     */
/*  meet templates are in decreasing duration order.                         */
/*                                                                           */
/*****************************************************************************/

static void KheNodeTemplateAddMeetTemplate(KHE_NODE_TEMPLATE nt,
  int duration, KHE_TIME_GROUP time_domain)
{
  KHE_MEET_TEMPLATE mt;  int i;
  MMake(mt);
  mt->duration = duration;
  mt->time_domain = time_domain;
  MArrayAddLast(nt->meet_templates, NULL);
  for( i = MArraySize(nt->meet_templates) - 2;
    i >= 0 && mt->duration > MArrayGet(nt->meet_templates, i)->duration;  i-- )
    MArrayPut(nt->meet_templates, i + 1, MArrayGet(nt->meet_templates, i));
  MArrayPut(nt->meet_templates, i + 1, mt);
  nt->duration += mt->duration;
}


/*****************************************************************************/
/*                                                                           */
/*  void KheNodeTemplateAddNode(KHE_NODE_TEMPLATE nt, KHE_NODE node)         */
/*                                                                           */
/*  Add templates for the meets of node to nt.                               */
/*                                                                           */
/*****************************************************************************/

static void KheNodeTemplateAddNode(KHE_NODE_TEMPLATE nt, KHE_NODE node)
{
  KHE_MEET meet;  int i;
  for( i = 0;  i < KheNodeMeetCount(node);  i++ )
  {
    meet = KheNodeMeet(node, i);
    KheNodeTemplateAddMeetTemplate(nt, KheMeetDuration(meet),
      KheMeetDomain(meet));
  }
}


/*****************************************************************************/
/*                                                                           */
/*  KHE_NODE KheNodeMakeFromTemplate(KHE_NODE_TEMPLATE nt, KHE_SOLN soln)    */
/*                                                                           */
/*  Make a node with no children, and meets based on nt.                     */
/*                                                                           */
/*****************************************************************************/

static KHE_NODE KheNodeMakeFromTemplate(KHE_NODE_TEMPLATE nt, KHE_SOLN soln)
{
  KHE_NODE res;  KHE_MEET_TEMPLATE mt;  int i;  KHE_MEET meet;
  res = KheNodeMake(soln);
  MArrayForEach(nt->meet_templates, &mt, &i)
  {
    meet = KheMeetMake(soln, mt->duration, NULL);
    if( !KheMeetSetDomain(meet, mt->time_domain) )
      MAssert(false, "KheNodeMakeFromTemplate internal error 1");
    if( !KheNodeAddMeet(res, meet) )
      MAssert(false, "KheNodeMakeFromTemplate internal error 2");
  }
  return res;
}


/*****************************************************************************/
/*                                                                           */
/*  bool KheNodeTemplateEquivalent(KHE_NODE_TEMPLATE nt1,                    */
/*    KHE_NODE_TEMPLATE nt2)                                                 */
/*                                                                           */
/*  Return true if nt1 and nt2 are equivalent, in the sense of having        */
/*  meets of the same durations with the same time domains.                  */
/*                                                                           */
/*****************************************************************************/

static bool KheNodeTemplateEquivalent(KHE_NODE_TEMPLATE nt1,
  KHE_NODE_TEMPLATE nt2)
{
  int i;  KHE_MEET_TEMPLATE mt1, mt2;
  if( MArraySize(nt1->meet_templates) != MArraySize(nt2->meet_templates) )
    return false;
  for( i = 0;  i < MArraySize(nt1->meet_templates);  i++ )
  {
    mt1 = MArrayGet(nt1->meet_templates, i);
    mt2 = MArrayGet(nt2->meet_templates, i);
    if( mt1->duration != mt2->duration || mt1->time_domain != mt2->time_domain )
      return false;
  }
  return true;
}


/*****************************************************************************/
/*                                                                           */
/*  void KheNodeTemplateDelete(KHE_NODE_TEMPLATE nt)                         */
/*                                                                           */
/*  Delete nt and the meet templates it contains.                            */
/*                                                                           */
/*****************************************************************************/

static void KheNodeTemplateDelete(KHE_NODE_TEMPLATE nt)
{
  while( MArraySize(nt->meet_templates) > 0 )
    MFree(MArrayRemoveLast(nt->meet_templates));
  MFree(nt);
}


/*****************************************************************************/
/*                                                                           */
/*  void KheNodeTemplateDebug(KHE_NODE_TEMPLATE nt, int verbosity,           */
/*    int indent, FILE *fp)                                                  */
/*                                                                           */
/*  Debug print of node template nt.                                         */
/*                                                                           */
/*****************************************************************************/

void KheNodeTemplateDebug(KHE_NODE_TEMPLATE nt, int verbosity,
  int indent, FILE *fp)
{
  KHE_MEET_TEMPLATE mt;  int i;
  if( verbosity >= 1 && indent >= 0 )
  {
    fprintf(fp, "%*s[ Node Template (freq %d): ", indent, "", nt->frequency);
    MArrayForEach(nt->meet_templates, &mt, &i)
    {
      if( i > 0 )
	fprintf(fp, ", ");
      fprintf(fp, "%d ", mt->duration);
      if( mt->time_domain == NULL )
	fprintf(fp, "auto");
      else
	KheTimeGroupDebug(mt->time_domain, 1, -1, fp);
    }
    fprintf(fp, " ]\n");
  }
}


/*****************************************************************************/
/*                                                                           */
/*  KHE_NODE_TEMPLATE_SET KheNodeTemplateSetMake(void)                       */
/*                                                                           */
/*  Make a new, empty node template set.                                     */
/*                                                                           */
/*****************************************************************************/

static KHE_NODE_TEMPLATE_SET KheNodeTemplateSetMake(void)
{
  KHE_NODE_TEMPLATE_SET res;
  MMake(res);
  MArrayInit(res->node_templates);
  return res;
}


/*****************************************************************************/
/*                                                                           */
/*  KHE_NODE_TEMPLATE KheNodeTemplateSetAddNodeTemplate(                     */
/*    KHE_NODE_TEMPLATE_SET nts, KHE_NODE_TEMPLATE nt)                       */
/*                                                                           */
/*  Add nt to nts.  If it proves to be the same as an existing node          */
/*  template, delete it, increment the frequency of the other one, and       */
/*  return it as a replacement for the nt passed in.                         */
/*                                                                           */
/*****************************************************************************/

static KHE_NODE_TEMPLATE KheNodeTemplateSetAddNodeTemplate(
  KHE_NODE_TEMPLATE_SET nts, KHE_NODE_TEMPLATE nt)
{
  KHE_NODE_TEMPLATES tps;  KHE_NODE_TEMPLATE nt2;  int i, j;

  /* make sure that nt has a templates array for nt's duration */
  MArrayFill(nts->node_templates, nt->duration + 1, NULL);
  if( MArrayGet(nts->node_templates, nt->duration) == NULL )
  {
    MMake(tps);
    tps->duration = nt->duration;
    MArrayInit(tps->node_templates);
    MArrayPut(nts->node_templates, nt->duration, tps);
  }

  /* check whether something equivalent to nt is already in the array */
  tps = MArrayGet(nts->node_templates, nt->duration);
  MArrayForEach(tps->node_templates, &nt2, &i)
    if( KheNodeTemplateEquivalent(nt, nt2) )
    {
      nt2->frequency++;
      for( j = i - 1; j >= 0 &&
	  MArrayGet(tps->node_templates, j)->frequency < nt2->frequency;  j-- )
	MArrayPut(tps->node_templates, j+1, MArrayGet(tps->node_templates, j));
      MArrayPut(tps->node_templates, j+1, nt2);
      KheNodeTemplateDelete(nt);
      return nt2;
    }

  /* not present, so add it */
  MArrayAddLast(tps->node_templates, nt);
  nt->frequency = 1;
  return nt;
}


/*****************************************************************************/
/*                                                                           */
/*  void KheNodeTemplateSetDelete(KHE_NODE_TEMPLATE_SET nts)                 */
/*                                                                           */
/*  Delete nts.                                                              */
/*                                                                           */
/*****************************************************************************/

static void KheNodeTemplateSetDelete(KHE_NODE_TEMPLATE_SET nts)
{
  KHE_NODE_TEMPLATES tps;
  while( MArraySize(nts->node_templates) > 0 )
  {
    tps = MArrayRemoveLast(nts->node_templates);
    if( tps != NULL )
    {
      while( MArraySize(tps->node_templates) > 0 )
	KheNodeTemplateDelete(MArrayRemoveLast(tps->node_templates));
      MFree(tps);
    }
  }
}


/*****************************************************************************/
/*                                                                           */
/* KHE_NODE_TEMPLATE_SET KheNodeTemplateSetMakeFromNode(KHE_NODE parent_node)*/
/*                                                                           */
/*  Make a node template set from the child nodes of parent_node.            */
/*                                                                           */
/*****************************************************************************/

static KHE_NODE_TEMPLATE_SET KheNodeTemplateSetMakeFromNode(
  KHE_NODE parent_node)
{
  int i;  KHE_NODE child_node;  KHE_NODE_TEMPLATE_SET res;
  KHE_NODE_TEMPLATE nt;
  res = KheNodeTemplateSetMake();
  for( i = 0;  i < KheNodeChildCount(parent_node);  i++ )
  {
    child_node = KheNodeChild(parent_node, i);
    nt = KheNodeTemplateMake();
    KheNodeTemplateAddNode(nt, child_node);
    nt = KheNodeTemplateSetAddNodeTemplate(res, nt);
  }
  return res;
}


/*****************************************************************************/
/*                                                                           */
/*  void KheNodeTemplateSetDebug(KHE_NODE_TEMPLATE_SET nts, int verbosity,   */
/*    int indent, FILE *fp)                                                  */
/*                                                                           */
/*  Debug print of nts onto fp with the given verbosity and indent.          */
/*                                                                           */
/*****************************************************************************/

static void KheNodeTemplateSetDebug(KHE_NODE_TEMPLATE_SET nts, int verbosity,
  int indent, FILE *fp)
{
  int i, j;  KHE_NODE_TEMPLATES tps;  KHE_NODE_TEMPLATE nt;
  if( verbosity >= 1 && indent >= 0 )
  {
    fprintf(fp, "%*s[ NodeTemplateSet\n", indent, "");
    MArrayForEach(nts->node_templates, &tps, &i)
      if( tps != NULL )
      {
	fprintf(fp, "%*s  duration %d:\n", indent, "", tps->duration);
	MArrayForEach(tps->node_templates, &nt, &j)
	  KheNodeTemplateDebug(nt, verbosity, indent + 4, fp);
      }
    fprintf(fp, "%*s]\n", indent, "");
  }
}


/*****************************************************************************/
/*                                                                           */
/*  Submodule "runarounds"                                                   */
/*                                                                           */
/*****************************************************************************/

/*****************************************************************************/
/*                                                                           */
/*  KHE_RUNAROUND_NODE - one node being processed by runaround code          */
/*                                                                           */
/*  NB we don't represent fixed nodes, they are useless so are left out.     */
/*                                                                           */
/*****************************************************************************/

typedef MARRAY(KHE_RESOURCE) ARRAY_KHE_RESOURCE;

typedef struct khe_runaround_node_rec {
  KHE_NODE		node;			/* the node                  */
  ARRAY_KHE_RESOURCE	layer_resources;	/* that the node holds       */
  int			meet_count;		/* its no. of meets          */
  int			child_count;		/* its no. of child nodes    */
  int			mrd;			/* its min runaround durn    */
  bool			problem;		/* true if problem node      */
} *KHE_RUNAROUND_NODE;

typedef MARRAY(KHE_RUNAROUND_NODE) ARRAY_KHE_RUNAROUND_NODE;


/*****************************************************************************/
/*                                                                           */
/*  KHE_RUNAROUND_NODE KheRunaroundNodeMake(KHE_NODE node)                   */
/*                                                                           */
/*  Make a runaround node object about node, which may not be a fixed node.  */
/*  Leave its layer_resources array empty for now.                           */
/*                                                                           */
/*****************************************************************************/

static KHE_RUNAROUND_NODE KheRunaroundNodeMake(KHE_NODE node, int mrd)
{
  KHE_RUNAROUND_NODE res;
  MAssert(KheNodeChildCount(node) >= 1, "KheRunaroundNodeMake internal error");
  MMake(res);
  res->node = node;
  MArrayInit(res->layer_resources);
  res->meet_count = KheNodeMeetCount(node);
  res->child_count = KheNodeChildCount(node);
  res->mrd = mrd;
  res->problem = (res->mrd > KheNodeDuration(node));
  return res;
}


/*****************************************************************************/
/*                                                                           */
/*  void KheRunaroundNodeFree(KHE_RUNAROUND_NODE in)                         */
/*                                                                           */
/*  Free a runaround node.                                                   */
/*                                                                           */
/*****************************************************************************/

static void KheRunaroundNodeFree(KHE_RUNAROUND_NODE in)
{
  MArrayFree(in->layer_resources);
  MFree(in);
}


/*****************************************************************************/
/*                                                                           */
/*  int KheRunaroundNodeCmp(const void *t1, const void *t2)                  */
/*                                                                           */
/*  Comparison function for sorting an array of runaround nodes              */
/*  into the order used by the runaround construction algorithm.             */
/*                                                                           */
/*****************************************************************************/

static int KheRunaroundNodeCmp(const void *t1, const void *t2)
{
  KHE_RUNAROUND_NODE in1 = * (KHE_RUNAROUND_NODE *) t1;
  KHE_RUNAROUND_NODE in2 = * (KHE_RUNAROUND_NODE *) t2;
  if( in1->problem )
  {
    if( in2->problem )
    {
      /* problem nodes are sorted by increasing mrd */
      if( in1->mrd != in2->mrd )
	return in1->mrd - in2->mrd;
      else
	return KheNodeIndex(in1->node) - KheNodeIndex(in2->node);
    }
    else
    {
      /* free node in2 precedes problem node in1 */
      return 1;
    }
  }
  else
  {
    if( in2->problem )
    {
      /* free node in1 precedes problem node in2 */
      return -1;
    }
    else
    {
      /* free nodes are sorted by decreasing duration */
      if( KheNodeDuration(in2->node) != KheNodeDuration(in1->node) )
	return KheNodeDuration(in2->node) - KheNodeDuration(in1->node);
      else
	return KheNodeIndex(in1->node) - KheNodeIndex(in2->node);
    }
  }
}


/*****************************************************************************/
/*                                                                           */
/*  void KheRunaroundNodeDebug(KHE_RUNAROUND_NODE rn, int indent, FILE *fp)  */
/*                                                                           */
/*  Print in onto fp with the given indent.                                  */
/*                                                                           */
/*****************************************************************************/

static void KheRunaroundNodeDebug(KHE_RUNAROUND_NODE rn, int indent, FILE *fp)
{
  int i;  KHE_RESOURCE r;
  fprintf(stderr, "%*s[ Node %d {", indent, "", KheNodeIndex(rn->node));
  MArrayForEach(rn->layer_resources, &r, &i)
    fprintf(stderr, "%s%s", i > 0 ? ", " : "",
      KheResourceId(r) == NULL ? "-" : KheResourceId(r));
  fprintf(stderr, "}: %d meets, %d children, %d mrd, %s ]\n",
    rn->meet_count, rn->child_count, rn->mrd, rn->problem ? "problem" : "free");
}


/*****************************************************************************/
/*                                                                           */
/*  bool KheRunaroundNodeCompatible(KHE_RUNAROUND_NODE rn1,                  */
/*    KHE_RUNAROUND_NODE rn2)                                                */
/*                                                                           */
/*  Return true if rn1 and rn2 are compatible in the sense of lying in the   */
/*  same segments.                                                           */
/*                                                                           */
/*  Implementation note.  By the way that the layer resource arrays of rn1   */
/*  and rn2 were constructed, if they have the same resources they appear    */
/*  in the same order.                                                       */
/*                                                                           */
/*****************************************************************************/

/* *** replaced by comparison function used when sorting
static bool KheRunaroundNodeCompatible(KHE_RUNAROUND_NODE rn1,
  KHE_RUNAROUND_NODE rn2)
{
  int i;  KHE_RESOURCE r1, r2;
  if( MArraySize(rn1->layer_resources) != MArraySize(rn2->layer_resources) )
    return false;
  for( i = 0;  i < MArraySize(rn1->layer_resources);  i++ )
  {
    r1 = MArrayGet(rn1->layer_resources, i);
    r2 = MArrayGet(rn2->layer_resources, i);
    if( r1 != r2 )
      return false;
  }
  return true;
}
*** */


/*****************************************************************************/
/*                                                                           */
/*  void KheMoveNodeAndChildren(KHE_NODE node, KHE_NODE run_root_node)       */
/*                                                                           */
/*  Move node and its children to below run_root_node.                       */
/*                                                                           */
/*****************************************************************************/

static void KheMoveNodeAndChildren(KHE_NODE node, KHE_NODE run_root_node)
{
  while( KheNodeChildCount(node) > 0 )
    if( !KheNodeMove(KheNodeChild(node, 0), run_root_node) )
      MAssert(false, "KheMoveNodeAndChildren internal error 1");
  if( !KheNodeMove(node, run_root_node) )
    MAssert(false, "KheMoveNodeAndChildren internal error 2");
}


/*****************************************************************************/
/*                                                                           */
/*  bool KheTryRunaround(KHE_RUNAROUND_NODE n,                               */
/*    ARRAY_KHE_RUNAROUND_NODE *nodes, int first, int last,                  */
/*    KHE_NODE_TIME_SOLVER time_solver, KHE_NODE_TEMPLATE nt)                */
/*                                                                           */
/*  Try to make a runaround by merging n with nodes[first..last].  If        */
/*  successful, n's node will be the merged node, nodes[first..last] is      */
/*  deleted from nodes, and true is returned.  Otherwise everything is       */
/*  returned to how it was at the start of the call, and false is returned.  */
/*                                                                           */
/*****************************************************************************/

static bool KheTryRunaround(KHE_RUNAROUND_NODE n,
  ARRAY_KHE_RUNAROUND_NODE *nodes, int first, int last,
  KHE_NODE_TIME_SOLVER time_solver, KHE_NODE_TEMPLATE nt)
{
  KHE_RUNAROUND_NODE x;  int i;  KHE_COST cost_before;  KHE_SOLN soln;
  KHE_NODE parent_node, run_root_node;  KHE_TRANSACTION t;
  if( DEBUG6 )
  {
    fprintf(stderr, "  [ KheTryRunaround(n, nodes[%d .. %d]):\n", first, last);
    KheRunaroundNodeDebug(n, 4, stderr);
    for( i = first;  i <= last;  i++ )
    {
      x = MArrayGet(*nodes, i);
      KheRunaroundNodeDebug(x, 4, stderr);
    }
    if( nt != NULL )
      KheNodeTemplateDebug(nt, 2, 4, stderr);
  }

  /* boilerplate */
  parent_node = KheNodeParent(n->node);
  MAssert(parent_node != NULL, "KheTryRunaround: internal error 1");
  soln = KheNodeSoln(n->node);

  /* make a root node for the runaround based on nt, or on a new nt if none */
  if( nt != NULL )
    run_root_node = KheNodeMakeFromTemplate(nt, soln);
  else
  {
    nt = KheNodeTemplateMake();
    KheNodeTemplateAddNode(nt, n->node);
    for( i = first;  i <= last;  i++ )
    {
      x = MArrayGet(*nodes, i);
      KheNodeTemplateAddNode(nt, x->node);
    }
    run_root_node = KheNodeMakeFromTemplate(nt, soln);
    KheNodeTemplateDelete(nt);
  }

  /* move the candidates nodes and their children to under run_root_node */
  t = KheTransactionMake(soln);
  KheTransactionBegin(t);
  KheMoveNodeAndChildren(n->node, run_root_node);
  for( i = first;  i <= last;  i++ )
  {
    x = MArrayGet(*nodes, i);
    KheMoveNodeAndChildren(x->node, run_root_node);
  }
  KheTransactionEnd(t);
  if( DEBUG6 )
  {
    fprintf(stderr, "    run root node:\n");
    KheNodeDebug(run_root_node, 3, 4, stderr);
  }

  /* timetable the run root node */
  cost_before = KheSolnCost(soln);
  if( time_solver(run_root_node) && KheSolnCost(soln) <= cost_before )
  {
    /* success; unassign, and remove the merged nodes from nodes */
    KheNodeUnAssignTimes(run_root_node);
    for( i = last;  i >= first;  i-- )
    {
      x = MArrayGet(*nodes, i);
      MArrayRemove(*nodes, i);
      /* KheRunaroundNodeFree(x); don't free x! done later by main prog */
    }
    n->node = run_root_node;  /* probably not necessary */
    if( !KheNodeAddParent(run_root_node, parent_node) )
      MAssert(false, "KheTryRunaround internal error 2");
    KheTransactionDelete(t);
    if( DEBUG6 )
      fprintf(stderr, "  ] KheTryRunaround returning true\n");
    return true;
  }
  else
  {
    /* failure; restore the original node linkages */
    KheNodeUnAssignTimes(run_root_node);
    KheTransactionUndo(t);
    KheTransactionDelete(t);
    while( KheNodeMeetCount(run_root_node) > 0 )
      KheMeetDelete(KheNodeMeet(run_root_node, 0));
    if( !KheNodeDelete(run_root_node) )
      MAssert(false, "KheTryRunaround internal error 3");
    if( DEBUG6 )
      fprintf(stderr, "  ] KheTryRunaround returning false\n");
    return false;
  }
}


/* *** old version
static bool KheTryRunaround(KHE_RUNAROUND_NODE n,
  ARRAY_KHE_RUNAROUND_NODE *nodes, int first, int last,
  KHE_NODE_TIME_SOLVER time_solver, KHE_NODE_TEMPLATE nt)
{
  KHE_RUNAROUND_NODE x;  int i;  KHE_COST cost_before;
  if( DEBUG6 )
  {
    fprintf(stderr, "  [ KheTryRunaround(n, nodes[%d .. %d]):\n", first, last);
    KheRunaroundNodeDebug(n, 4, stderr);
    for( i = first;  i <= last;  i++ )
    {
      x = MArrayGet(*nodes, i);
      KheRunaroundNodeDebug(x, 4, stderr);
    }
    if( nt != NULL )
      KheNodeTemplateDebug(nt, 2, 4, stderr);
  }

  ** merge the nodes **
  for( i = first;  i <= last;  i++ )
  {
    x = MArrayGet(*nodes, i);
    KheNodeMerge(n->node, x->node, &n->node);
    x->node = NULL;  ** it's undefined, so we make that clear **
  }
  if( DEBUG6 )
  {
    fprintf(stderr, "    merged node:\n");
    KheNodeDebug(n->node, 3, 4, stderr);
  }

  ** timetable the merged node **
  cost_before = KheSolnCost(KheNodeSoln(n->node));
  if( time_solver(n->node) && KheSolnCost(KheNodeSoln(n->node)) <= cost_before )
  {
    ** success; remove the merged nodes from nodes **
    KheNodeUnAssignTimes(n->node);
    for( i = last;  i >= first;  i-- )
    {
      x = MArrayGet(*nodes, i);
      MArrayRemove(*nodes, i);
      ** KheRunaroundNodeFree(x); don't free x! done later by main prog **
    }
    if( DEBUG6 )
      fprintf(stderr, "  ] KheTryRunaround returning true\n");
    return true;
  }
  else
  {
    ** failure; split the merged node back to the original state **
    KheNodeUnAssignTimes(n->node);
    for( i = last;  i >= first;  i-- )
    {
      x = MArrayGet(*nodes, i);
      if( !KheNodeSplit(n->node, KheNodeMeetCount(n->node) - x->meet_count,
	  KheNodeChildCount(n->node) - x->child_count, &n->node, &x->node) )
	MAssert(false, "KheTryRunaround internal error");
      if( DEBUG6 )
      {
	fprintf(stderr, "    split: ");
	KheRunaroundNodeDebug(x, 0, stderr);
	KheNodeDebug(x->node, 3, 4, stderr);
      }
    }
    if( DEBUG6 )
      fprintf(stderr, "  ] KheTryRunaround returning false\n");
    return false;
  }
}
*** */


/*****************************************************************************/
/*                                                                           */
/*  bool KheTryRunarounds(KHE_RUNAROUND_NODE n,                              */
/*    ARRAY_KHE_RUNAROUND_NODE *nodes, int first, int last,                  */
/*    KHE_NODE_TIME_SOLVER time_solver, int durn, KHE_NODE_TEMPLATE_SET nts) */
/*                                                                           */
/*  Try runarounds for n + nodes[first..last].  Their total duration is      */
/*  durn, so try with template nodes from nts of this duration; but, as      */
/*  a last resort, also try without a template node.                         */
/*                                                                           */
/*****************************************************************************/

static bool KheTryRunarounds(KHE_RUNAROUND_NODE n,
  ARRAY_KHE_RUNAROUND_NODE *nodes, int first, int last,
  KHE_NODE_TIME_SOLVER time_solver, int durn, KHE_NODE_TEMPLATE_SET nts)
{
  KHE_NODE_TEMPLATES tps;  KHE_NODE_TEMPLATE nt;  int i;
  if( MArraySize(nts->node_templates) > durn )
  {
    tps = MArrayGet(nts->node_templates, durn);
    if( tps != NULL )
      MArrayForEach(tps->node_templates, &nt, &i)
        if( KheTryRunaround(n, nodes, first, last, time_solver, nt) )
	  return true;
  }
  return KheTryRunaround(n, nodes, first, last, time_solver, NULL);
}


/*****************************************************************************/
/*                                                                           */
/*  int KheRunaroundNodeLexCompar(KHE_RUNAROUND_NODE rn1,                    */
/*    KHE_RUNAROUND_NODE rn2)                                                */
/*                                                                           */
/*  Return an integer less than, equal to, or greater than zero if the       */
/*  resources of rn1 are lexically less than, equal to, or greater than      */
/*  the resources of rn2.                                                    */
/*                                                                           */
/*****************************************************************************/
#define min(x, y) ((x) < (y) ? (x) : (y))

static int KheRunaroundNodeLexCompar(KHE_RUNAROUND_NODE rn1,
  KHE_RUNAROUND_NODE rn2)
{
  int i, lim;  KHE_RESOURCE r1, r2;
  lim = min(MArraySize(rn1->layer_resources), MArraySize(rn2->layer_resources));
  for( i = 0;  i < lim;  i++ )
  {
    r1 = MArrayGet(rn1->layer_resources, i);
    r2 = MArrayGet(rn2->layer_resources, i);
    if( KheResourceIndexInInstance(r1) != KheResourceIndexInInstance(r2) )
      return
	KheResourceIndexInInstance(r1) - KheResourceIndexInInstance(r2);
  }
  return MArraySize(rn1->layer_resources) - MArraySize(rn2->layer_resources);
}


/*****************************************************************************/
/*                                                                           */
/*  bool KheRunaroundNodesHaveSameResources(KHE_RUNAROUND_NODE rn1,          */
/*    KHE_RUNAROUND_NODE rn2)                                                */
/*                                                                           */
/*  Return true if these two nodes have the same resources.                  */
/*                                                                           */
/*****************************************************************************/

static bool KheRunaroundNodesHaveSameResources(KHE_RUNAROUND_NODE rn1,
  KHE_RUNAROUND_NODE rn2)
{
  return KheRunaroundNodeLexCompar(rn1, rn2) == 0;
}


/*****************************************************************************/
/*                                                                           */
/*  int KheRunaroundNodeLexCmp(const void *t1, const void *t2)               */
/*                                                                           */
/*  Comparison function for sorting an array of runaround nodes so that      */
/*  runaround nodes with the same resources appear together, and null        */
/*  array entries go on the end.                                             */
/*                                                                           */
/*****************************************************************************/

static int KheRunaroundNodeLexCmp(const void *t1, const void *t2)
{
  KHE_RUNAROUND_NODE rn1 = * (KHE_RUNAROUND_NODE *) t1;
  KHE_RUNAROUND_NODE rn2 = * (KHE_RUNAROUND_NODE *) t2;
  int cmp;
  if( rn1 == NULL )
    return rn2 == NULL ? 0 : 1;
  else if( rn2 == NULL )
    return -1;
  else
  {
    cmp = KheRunaroundNodeLexCompar(rn1, rn2);
    if( cmp != 0 )
      return cmp;
    else
      return KheNodeIndex(rn1->node) - KheNodeIndex(rn2->node);
  }
}


/*****************************************************************************/
/*                                                                           */
/*  void HandleOneMergeableSet(ARRAY_KHE_RUNAROUND_NODE *nodes,              */
/*    int u, int v, KHE_NODE_TIME_SOLVER runaround_solver,                   */
/*    KHE_NODE_TEMPLATE_SET nts)                                             */
/*                                                                           */
/*  Handle one set of mergeable nodes.  This is the algorithm documented in  */
/*  the User's Guide, except for nts.  Fixed nodes have already been         */
/*  omitted from *nodes.                                                     */
/*                                                                           */
/*****************************************************************************/

static void HandleOneMergeableSet(ARRAY_KHE_RUNAROUND_NODE *nodes,
  int u, int v, KHE_NODE_TIME_SOLVER runaround_solver,
  KHE_NODE_TEMPLATE_SET nts)
{
  KHE_RUNAROUND_NODE n, x;  int i, durn;  bool done;
  MArraySort(*nodes, &KheRunaroundNodeCmp);
  if( DEBUG4 )
  {
    fprintf(stderr, "  [ HandleOneMergeableSet(nodes, u %d, v %d):\n", u, v);
    MArrayForEach(*nodes, &n, &i)
      KheRunaroundNodeDebug(n, 4, stderr);
  }
  while( MArraySize(*nodes) > 0 )
  {
    n = MArrayRemoveLast(*nodes);
    if( n->problem && n->mrd < v )
    {
      /* Case 1 */
      done = false;
      for( i = MArraySize(*nodes) - 1;  i >= 0 && !done;  i-- )
      {
	x = MArrayGet(*nodes, i);
	durn = KheNodeDuration(n->node) + KheNodeDuration(x->node);
	if( n->mrd <= durn && durn == u && durn <= v &&
	    KheTryRunarounds(n, nodes, i, i, runaround_solver, durn, nts) )
	  done = true;
      }

      /* Case 2 */
      durn = KheNodeDuration(n->node);
      for( i = MArraySize(*nodes) - 1;  i >= 0 && !done;  i-- )
      {
	x = MArrayGet(*nodes, i);
	durn += KheNodeDuration(x->node);
	if( durn > v )
	  break;
	if( n->mrd <= durn && KheTryRunarounds(n, nodes, i,
	    MArraySize(*nodes) - 1, runaround_solver, durn, nts) )
	  done = true;
      }
    }
  }
  if( DEBUG4 )
    fprintf(stderr, "  ] HandleOneMergeableSet returning\n");
}


/*****************************************************************************/
/*                                                                           */
/*  void KheBuildRunarounds(KHE_NODE parent_node,                            */
/*    KHE_NODE_TIME_SOLVER mrd_solver, KHE_NODE_TIME_SOLVER runaround_solver)*/
/*                                                                           */
/*  Build runarounds for the problem child nodes of parent_node.             */
/*                                                                           */
/*****************************************************************************/

void KheBuildRunarounds(KHE_NODE parent_node,
  KHE_NODE_TIME_SOLVER mrd_solver, KHE_NODE_TIME_SOLVER runaround_solver)
{
  ARRAY_INT frequencies;  int i, j, u, v, freq, u_freq, mrd;  bool has_problem;
  KHE_RESOURCE r;  KHE_INSTANCE ins;  KHE_NODE child_node;  KHE_LAYER layer;
  ARRAY_KHE_RUNAROUND_NODE runaround_nodes, nodes;  KHE_RUNAROUND_NODE rn;
  KHE_NODE_TEMPLATE_SET nts;

  if( DEBUG4 )
    fprintf(stderr, "[ KheBuildRunarounds(Node %d)\n",
      KheNodeIndex(parent_node));

  /* sort out node templates */
  nts = KheNodeTemplateSetMakeFromNode(parent_node);
  if( DEBUG4 )
    KheNodeTemplateSetDebug(nts, 2, 2, stderr);

  /* use a frequency table to choose a suitable value of u */
  MArrayInit(frequencies);
  for( i = 0;  i < KheNodeChildCount(parent_node);  i++ )
  {
    child_node = KheNodeChild(parent_node, i);
    MArrayFill(frequencies, KheNodeDuration(child_node) + 1, 0);
    MArrayInc(frequencies, KheNodeDuration(child_node));
  }
  u = u_freq = 0;
  MArrayForEach(frequencies, &freq, &i)
    if( freq * i > u_freq * u )
      u = i, u_freq = freq;
  MArrayFree(frequencies);

  /* use one-fifth of the duration of parent_node as v */
  v = KheNodeDuration(parent_node) / 5;
  if( DEBUG4 )
    fprintf(stderr, "  u = %d, v = %d\n", u, v);

  /* build runaround nodes for useful child nodes, indexed by node index */
  MArrayInit(runaround_nodes);
  for( i = 0;  i < KheNodeChildCount(parent_node);  i++ )
  {
    child_node = KheNodeChild(parent_node, i);
    if( DEBUG4 )
    {
      fprintf(stderr, "  examining child_node (%d children):\n",
	KheNodeChildCount(child_node));
      KheNodeDebug(child_node, 2, 2, stderr);
    }
    if( KheNodeChildCount(child_node) >= 1 &&
	KheMinimumRunaroundDuration(child_node, mrd_solver, &mrd) && mrd <= v )
    {
      rn = KheRunaroundNodeMake(child_node, mrd);
      MArrayFill(runaround_nodes, KheNodeIndex(child_node) + 1, NULL);
      MArrayPut(runaround_nodes, KheNodeIndex(child_node), rn);
    }
  }

  /* distribute layer resources to runaround nodes */
  ins = KheSolnInstance(KheNodeSoln(parent_node));
  for( i = 0;  i < KheInstanceResourceCount(ins);  i++ )
  {
    r = KheInstanceResource(ins, i);
    if( KheResourceLayerDuration(r) >= 0.9 * KheNodeDuration(parent_node) )
    {
      layer = KheLayerMakeFromResource(parent_node, r);
      if( DEBUG4 )
      {
	fprintf(stderr, "  resource %s (layer durn %d) %sof interest:\n",
	  KheResourceId(r) == NULL ? "-" : KheResourceId(r),
	  KheResourceLayerDuration(r), LayerIsOfInterest(layer) ? "" : "not ");
	KheLayerDebug(layer, 2, 2, stderr);
      }
      if( LayerIsOfInterest(layer) )
      {
	for( j = 0;  j < KheLayerChildNodeCount(layer);  j++ )
	{
	  child_node = KheLayerChildNode(layer, j);
	  MArrayFill(runaround_nodes, KheNodeIndex(child_node) + 1, NULL);
	  rn = MArrayGet(runaround_nodes, KheNodeIndex(child_node));
	  if( rn != NULL )
	    MArrayAddLast(rn->layer_resources, r);
	}
      }
      KheLayerDelete(layer);
    }
  }

  /* bring nodes with the same layers together, and remove null entries */
  MArraySort(runaround_nodes, &KheRunaroundNodeLexCmp);
  while( MArraySize(runaround_nodes) > 0 && MArrayLast(runaround_nodes)==NULL )
    MArrayDropLast(runaround_nodes);
  if( DEBUG4 )
  {
    fprintf(stderr, "  [ runaround nodes:\n");
    MArrayForEach(runaround_nodes, &rn, &i)
    {
      fprintf(stderr, "    %d: ", i);
      KheRunaroundNodeDebug(rn, 4, stderr);
    }
    fprintf(stderr, "  ]\n");
  }

  /* handle each set of mergeable nodes */
  MArrayInit(nodes);
  for( i = 0;  i < MArraySize(runaround_nodes);  i = j )
  {
    /* build one set of mergeable nodes */
    MArrayClear(nodes);
    rn = MArrayGet(runaround_nodes, i);
    has_problem = rn->problem;
    MArrayAddLast(nodes, rn);
    for( j = i + 1;  j < MArraySize(runaround_nodes);  j++ )
    {
      rn = MArrayGet(runaround_nodes, j);
      if( !KheRunaroundNodesHaveSameResources(rn, MArrayFirst(nodes)) )
	break;
      if( rn->problem )
	has_problem = true;
      MArrayAddLast(nodes, rn);
    }

    /* if this set has a problem, handle it */
    if( has_problem )
      HandleOneMergeableSet(&nodes, u, v, runaround_solver, nts);
  }
  MArrayFree(nodes);

  /* free runaround nodes memory */
  MArrayForEach(runaround_nodes, &rn, &i)
    KheRunaroundNodeFree(rn);
  MArrayFree(runaround_nodes);

  /* free template nodes memory */
  KheNodeTemplateSetDelete(nts);

  if( DEBUG4 )
  {
    fprintf(stderr, "  at end of KheBuildRunarounds:\n");
    KheNodeDebug(parent_node, 3, 2, stderr);
    fprintf(stderr, "] KheBuildRunarounds returning\n");
  }
}


/*****************************************************************************/
/*                                                                           */
/*  Submodule "flattening"                                                   */
/*                                                                           */
/*****************************************************************************/

/*****************************************************************************/
/*                                                                           */
/*  void KheNodeDoBypass(KHE_NODE node, KHE_NODE target_node)                */
/*                                                                           */
/*  Helper function for KheNodeBypass below.  Move the proper                */
/*  descendants of node to be children of target_node.                       */
/*                                                                           */
/*****************************************************************************/

/* ***
static void KheNodeDoBypass(KHE_NODE node, KHE_NODE target_node)
{
  KHE_NODE child_node;
  while( KheNodeChildCount(node) > 0 )
  {
    child_node = KheNodeChild(node, 0);
    KheNodeDoBypass(child_node, target_node);
    if( !KheNodeMove(child_node, target_node) )
      MAssert(false, "KheNodeDoBypass internal error");
  }
}
*** */


/*****************************************************************************/
/*                                                                           */
/*  void KheNodeBypass(KHE_NODE node)                                        */
/*                                                                           */
/*  Move the children of node to be children of its parent.                  */
/*                                                                           */
/*****************************************************************************/

void KheNodeBypass(KHE_NODE node)
{
  MAssert(KheNodeParent(node) != NULL, "KheNodeBypass: node has no parent");
  while( KheNodeChildCount(node) > 0 )
    if( !KheNodeMove(KheNodeChild(node, 0), KheNodeParent(node)) )
      MAssert(false, "KheNodeBypass internal error");
}


/*****************************************************************************/
/*                                                                           */
/*  void KheNodeFlatten(KHE_NODE parent_node)                                */
/*                                                                           */
/*  Flatten the subtree rooted at parent_node.                               */
/*                                                                           */
/*  Implementation note.  The implementation assumes that new children       */
/*  will go on the end.  The loop invariant is "all nodes at indexes         */
/*  less than i have no children, and all proper descendants are still       */
/*  descendants."                                                            */
/*                                                                           */
/*****************************************************************************/

void KheNodeFlatten(KHE_NODE parent_node)
{
  int i;
  for( i = 0;  i < KheNodeChildCount(parent_node);  i++ )
    KheNodeBypass(KheNodeChild(parent_node, i));
}
