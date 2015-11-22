
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
/*  FILE:         khe_layer.c                                                */
/*  DESCRIPTION:  Solution layers and layerings                              */
/*                                                                           */
/*****************************************************************************/
#include "khe_interns.h"

#define DEBUG1 0
#define DEBUG2 0
#define DEBUG3 0

/*****************************************************************************/
/*                                                                           */
/*  KHE_LAYER                                                                */
/*                                                                           */
/*****************************************************************************/

struct khe_layer_rec {
  void				*back;		/* back pointer              */
  int				index;		/* index in parent node      */
  int				visit_num;	/* visit number              */
  KHE_NODE			parent_node;	/* the parent node           */
  /* KHE_LAYERING		layering; */	/* optional encl. layering   */
  ARRAY_KHE_NODE		child_nodes;	/* the child nodes           */
  ARRAY_KHE_RESOURCE		resources;	/* resources                 */
  int				duration;	/* duration of nodes         */
  LSET				lset;		/* lset of node indexes      */
  KHE_LAYER			copy;		/* used when copying         */
};


/*****************************************************************************/
/*                                                                           */
/*  KHE_LAYERING                                                             */
/*                                                                           */
/*****************************************************************************/

/* ***
typedef MARRAY(KHE_LAYER) ARRAY_KHE_LAYER;

struct khe_layering_rec {
  KHE_NODE			parent_node;	** the parent node           **
  ARRAY_KHE_LAYER		layers;		** the layers                **
};
*** */


/*****************************************************************************/
/*                                                                           */
/*  Submodule "construction and query"                                       */
/*                                                                           */
/*****************************************************************************/

/*****************************************************************************/
/*                                                                           */
/*  KHE_LAYER KheLayerMake(KHE_NODE parent_node)                             */
/*                                                                           */
/*  Make a new, empty child layer for parent_node.                           */
/*                                                                           */
/*****************************************************************************/

KHE_LAYER KheLayerMake(KHE_NODE parent_node)
{
  KHE_LAYER res;
  MMake(res);
  res->back = NULL;
  res->visit_num = 0;
  res->parent_node = parent_node;
  MArrayInit(res->child_nodes);
  MArrayInit(res->resources);
  res->duration = 0;
  res->lset = LSetNew();
  res->copy = NULL;
  KheNodeAddChildLayer(parent_node, res, &res->index);
  return res;
}


/*****************************************************************************/
/*                                                                           */
/*  int KheLayerIndex(KHE_LAYER layer)                                       */
/*                                                                           */
/*  Return the index number of layer in its parent node.                     */
/*                                                                           */
/*****************************************************************************/

int KheLayerIndex(KHE_LAYER layer)
{
  return layer->index;
}


/*****************************************************************************/
/*                                                                           */
/*  void KheLayerSetIndex(KHE_LAYER layer, int index)                        */
/*                                                                           */
/*  Set the index of layer.                                                  */
/*                                                                           */
/*****************************************************************************/

void KheLayerSetIndex(KHE_LAYER layer, int index)
{
  layer->index = index;
}


/*****************************************************************************/
/*                                                                           */
/*  KHE_NODE KheLayerParentNode(KHE_LAYER layer)                             */
/*                                                                           */
/*  Return the parent node of layer.                                         */
/*                                                                           */
/*****************************************************************************/

KHE_NODE KheLayerParentNode(KHE_LAYER layer)
{
  return layer->parent_node;
}


/*****************************************************************************/
/*                                                                           */
/*  KHE_SOLN KheLayerSoln(KHE_LAYER layer)                                   */
/*                                                                           */
/*  Return the solution of layer's parent node.                              */
/*                                                                           */
/*****************************************************************************/

KHE_SOLN KheLayerSoln(KHE_LAYER layer)
{
  return KheNodeSoln(layer->parent_node);
}


/*****************************************************************************/
/*                                                                           */
/*  void KheLayerChangeNodeIndex(KHE_LAYER layer,                            */
/*    int old_index, int new_index)                                          */
/*                                                                           */
/*  The index number of one of the child nodes of layer has changed from     */
/*  old_index to new_index, so change layer's lset accordingly.              */
/*                                                                           */
/*****************************************************************************/

void KheLayerChangeNodeIndex(KHE_LAYER layer, int old_index, int new_index)
{
  LSetDelete(layer->lset, old_index);
  LSetInsert(&layer->lset, new_index);
}


/*****************************************************************************/
/*                                                                           */
/*  KHE_LAYER KheLayerCopyPhase1(KHE_LAYER layer)                            */
/*                                                                           */
/*  Carry out Phase 1 of copying layer.                                      */
/*                                                                           */
/*****************************************************************************/

KHE_LAYER KheLayerCopyPhase1(KHE_LAYER layer)
{
  KHE_LAYER copy;  KHE_NODE child_node;  int i;
  if( layer->copy == NULL )
  {
    MMake(copy);
    copy->back = layer->back;
    copy->visit_num = layer->visit_num;
    layer->copy = copy;
    copy->parent_node = KheNodeCopyPhase1(layer->parent_node);
    MArrayInit(copy->child_nodes);
    MArrayForEach(layer->child_nodes, &child_node, &i)
      MArrayAddLast(copy->child_nodes, KheNodeCopyPhase1(child_node));
    MArrayInit(copy->resources);
    MArrayAppend(copy->resources, layer->resources, i);
    copy->duration = layer->duration;
    copy->lset = LSetCopy(layer->lset);
  }
  return layer->copy;
}


/*****************************************************************************/
/*                                                                           */
/*  void KheLayerCopyPhase2(KHE_LAYER layer)                                 */
/*                                                                           */
/*  Carry out Phase 2 of copying layer.                                      */
/*                                                                           */
/*****************************************************************************/

void KheLayerCopyPhase2(KHE_LAYER layer)
{
  if( layer->copy != NULL )
    layer->copy = NULL;
}


/*****************************************************************************/
/*                                                                           */
/*  void KheLayerAddDuration(KHE_LAYER layer, int durn)                      */
/*                                                                           */
/*  Add durn to the duration of layer.                                       */
/*                                                                           */
/*****************************************************************************/

void KheLayerAddDuration(KHE_LAYER layer, int durn)
{
  layer->duration += durn;
}


/*****************************************************************************/
/*                                                                           */
/*  void KheLayerSubtractDuration(KHE_LAYER layer, int durn)                 */
/*                                                                           */
/*  Subtract durn from the duration of layer.                                */
/*                                                                           */
/*****************************************************************************/

void KheLayerSubtractDuration(KHE_LAYER layer, int durn)
{
  layer->duration -= durn;
  MAssert(layer->duration >= 0, "KheLayerSubtractDuration internal error");
}


/*****************************************************************************/
/*                                                                           */
/*  void KheLayerDelete(KHE_LAYER layer)                                     */
/*                                                                           */
/*  Delete and free layer.                                                   */
/*                                                                           */
/*****************************************************************************/

void KheLayerDelete(KHE_LAYER layer)
{
  /* detach layer from child nodes and parent node */
  while( MArraySize(layer->child_nodes) > 0 )
    KheLayerDeleteChildNode(layer, MArrayLast(layer->child_nodes));
  KheNodeDeleteChildLayer(layer->parent_node, layer);

  /* free the memory it used */
  MArrayFree(layer->child_nodes);
  MArrayFree(layer->resources);
  LSetFree(layer->lset);
  MFree(layer);
}


/*****************************************************************************/
/*                                                                           */
/*  void KheLayerAddChildNode(KHE_LAYER layer, KHE_NODE node)                */
/*                                                                           */
/*  Add node to layer.                                                       */
/*                                                                           */
/*****************************************************************************/

void KheLayerAddChildNode(KHE_LAYER layer, KHE_NODE node)
{
  MArrayAddLast(layer->child_nodes, node);
  layer->duration += KheNodeDuration(node);
  LSetInsert(&layer->lset, KheNodeIndex(node));
  KheNodeAddParentLayer(node, layer);
}


/*****************************************************************************/
/*                                                                           */
/*  void KheLayerDeleteChildNode(KHE_LAYER layer, KHE_NODE n)                */
/*                                                                           */
/*  Delete n from layer.                                                     */
/*                                                                           */
/*****************************************************************************/

void KheLayerDeleteChildNode(KHE_LAYER layer, KHE_NODE n)
{
  int pos;
  if( !MArrayContains(layer->child_nodes, n, &pos) )
    MAssert(false, "KheLayerDeleteChildNode: layer does not contain n");
  KheNodeDeleteParentLayer(n, layer);
  LSetDelete(layer->lset, KheNodeIndex(n));
  layer->duration -= KheNodeDuration(n);
  MArrayRemove(layer->child_nodes, pos);
}


/*****************************************************************************/
/*                                                                           */
/*  Submodule "resources"                                                    */
/*                                                                           */
/*****************************************************************************/

/*****************************************************************************/
/*                                                                           */
/*  void KheLayerAddResource(KHE_LAYER layer, KHE_RESOURCE r)                */
/*                                                                           */
/*  Add r to layer.                                                          */
/*                                                                           */
/*****************************************************************************/

void KheLayerAddResource(KHE_LAYER layer, KHE_RESOURCE r)
{
  MArrayAddLast(layer->resources, r);
}


/*****************************************************************************/
/*                                                                           */
/*  void KheLayerDeleteResource(KHE_LAYER layer, KHE_RESOURCE r)             */
/*                                                                           */
/*  Delete r from layer.                                                     */
/*                                                                           */
/*****************************************************************************/

void KheLayerDeleteResource(KHE_LAYER layer, KHE_RESOURCE r)
{
  int pos;
  if( !MArrayContains(layer->resources, r, &pos) )
    MAssert(false, "KheLayerDeleteResource: layer does not contain r");
  MArrayRemove(layer->resources, pos);
}


/*****************************************************************************/
/*                                                                           */
/*  int KheLayerResourceCount(KHE_LAYER layer)                               */
/*                                                                           */
/*  Return the number of resources in layer.                                 */
/*                                                                           */
/*****************************************************************************/

int KheLayerResourceCount(KHE_LAYER layer)
{
  return MArraySize(layer->resources);
}


/*****************************************************************************/
/*                                                                           */
/*  KHE_RESOURCE KheLayerResource(KHE_LAYER layer, int i)                    */
/*                                                                           */
/*  Return the i'th resource of layer.                                       */
/*                                                                           */
/*****************************************************************************/

KHE_RESOURCE KheLayerResource(KHE_LAYER layer, int i)
{
  return MArrayGet(layer->resources, i);
}


/*****************************************************************************/
/*                                                                           */
/*  KHE_LAYERING KheLayerLayering(KHE_LAYER layer)                           */
/*                                                                           */
/*  Return the enclosing soln layering of layer, or NULL if none.            */
/*                                                                           */
/*****************************************************************************/

/* ***
KHE_LAYERING KheLayerLayering(KHE_LAYER layer)
{
  return layer->layering;
}
*** */


/*****************************************************************************/
/*                                                                           */
/*  Submodule "duration etc."                                                */
/*                                                                           */
/*****************************************************************************/

/*****************************************************************************/
/*                                                                           */
/*  int KheLayerDuration(KHE_LAYER layer)                                    */
/*                                                                           */
/*  Return the total duration of the nodes of layer.                         */
/*                                                                           */
/*****************************************************************************/

int KheLayerDuration(KHE_LAYER layer)
{
  return layer->duration;
}


/*****************************************************************************/
/*                                                                           */
/*  int KheLayerChildNodeCount(KHE_LAYER layer)                              */
/*                                                                           */
/*  Return the number of child nodes in layer.                               */
/*                                                                           */
/*****************************************************************************/

int KheLayerChildNodeCount(KHE_LAYER layer)
{
  return MArraySize(layer->child_nodes);
}


/*****************************************************************************/
/*                                                                           */
/*  KHE_NODE KheLayerChildNode(KHE_LAYER layer, int i)                       */
/*                                                                           */
/*  Return the ith child node of layer.                                      */
/*                                                                           */
/*****************************************************************************/

KHE_NODE KheLayerChildNode(KHE_LAYER layer, int i)
{
  return MArrayGet(layer->child_nodes, i);
}


/*****************************************************************************/
/*                                                                           */
/*  int KheLayerAssignedDuration(KHE_LAYER layer)                            */
/*                                                                           */
/*  Return the total assigned duration of the nodes of layer.                */
/*                                                                           */
/*****************************************************************************/

int KheLayerAssignedDuration(KHE_LAYER layer)
{
  int i, res;  KHE_NODE n;
  res = 0;
  MArrayForEach(layer->child_nodes, &n, &i)
    res += KheNodeAssignedDuration(n);
  return res;
}


/*****************************************************************************/
/*                                                                           */
/*  int KheLayerDemand(KHE_LAYER layer)                                      */
/*                                                                           */
/*  Return the total demand of the nodes of layer.                           */
/*                                                                           */
/*****************************************************************************/

int KheLayerDemand(KHE_LAYER layer)
{
  int i, res;  KHE_NODE n;
  res = 0;
  MArrayForEach(layer->child_nodes, &n, &i)
    res += KheNodeDemand(n);
  return res;
}


/*****************************************************************************/
/*                                                                           */
/*  Submodule "back pointers"                                                */
/*                                                                           */
/*****************************************************************************/

/*****************************************************************************/
/*                                                                           */
/*  void KheLayerSetBack(KHE_LAYER layer, void *back)                        */
/*                                                                           */
/*  Set the back pointer of layer.                                           */
/*                                                                           */
/*****************************************************************************/

void KheLayerSetBack(KHE_LAYER layer, void *back)
{
  layer->back = back;
}


/*****************************************************************************/
/*                                                                           */
/*  void *KheLayerBack(KHE_LAYER layer)                                      */
/*                                                                           */
/*  Return the back pointer of layer.                                        */
/*                                                                           */
/*****************************************************************************/

void *KheLayerBack(KHE_LAYER layer)
{
  return layer->back;
}


/*****************************************************************************/
/*                                                                           */
/*  Submodule "visit numbers"                                                */
/*                                                                           */
/*****************************************************************************/

/*****************************************************************************/
/*                                                                           */
/*  void KheLayerSetVisitNum(KHE_LAYER layer, int num)                       */
/*                                                                           */
/*  Set the visit number of layer.                                           */
/*                                                                           */
/*****************************************************************************/

void KheLayerSetVisitNum(KHE_LAYER layer, int num)
{
  layer->visit_num = num;
}


/*****************************************************************************/
/*                                                                           */
/*  int KheLayerVisitNum(KHE_LAYER layer)                                    */
/*                                                                           */
/*  Return the visit number of layer.                                        */
/*                                                                           */
/*****************************************************************************/

int KheLayerVisitNum(KHE_LAYER layer)
{
  return layer->visit_num;
}


/*****************************************************************************/
/*                                                                           */
/*  bool KheLayerVisited(KHE_LAYER layer, int slack)                         */
/*                                                                           */
/*  Return true if layer has been visited recently.                          */
/*                                                                           */
/*****************************************************************************/

bool KheLayerVisited(KHE_LAYER layer, int slack)
{
  return KheSolnVisitNum(KheLayerSoln(layer)) - layer->visit_num <= slack;
}


/*****************************************************************************/
/*                                                                           */
/*  void KheLayerVisit(KHE_LAYER layer)                                      */
/*                                                                           */
/*  Visit layer.                                                             */
/*                                                                           */
/*****************************************************************************/

void KheLayerVisit(KHE_LAYER layer)
{
  layer->visit_num = KheSolnVisitNum(KheLayerSoln(layer));
}


/*****************************************************************************/
/*                                                                           */
/*  void KheLayerUnVisit(KHE_LAYER layer)                                    */
/*                                                                           */
/*  Unvisit layer.                                                           */
/*                                                                           */
/*****************************************************************************/

void KheLayerUnVisit(KHE_LAYER layer)
{
  layer->visit_num = KheSolnVisitNum(KheLayerSoln(layer)) - 1;
}


/*****************************************************************************/
/*                                                                           */
/*  Submodule "set operations on soln layers"                                */
/*                                                                           */
/*****************************************************************************/

/*****************************************************************************/
/*                                                                           */
/*  bool KheLayerEqual(KHE_LAYER layer1, KHE_LAYER layer2)                   */
/*                                                                           */
/*  Return true if layer1 and layer2 contain the same nodes.                 */
/*                                                                           */
/*****************************************************************************/

bool KheLayerEqual(KHE_LAYER layer1, KHE_LAYER layer2)
{
  return LSetEqual(layer1->lset, layer2->lset);
}


/*****************************************************************************/
/*                                                                           */
/*  bool KheLayerSubset(KHE_LAYER layer1, KHE_LAYER layer2)                  */
/*                                                                           */
/*  Return true of all of layer1's nodes are nodes of layer2.                */
/*                                                                           */
/*****************************************************************************/

bool KheLayerSubset(KHE_LAYER layer1, KHE_LAYER layer2)
{
  return LSetSubset(layer1->lset, layer2->lset);
}


/*****************************************************************************/
/*                                                                           */
/*  bool KheLayerDisjoint(KHE_LAYER layer1, KHE_LAYER layer2)                */
/*                                                                           */
/*  Return true if layer1 and layer2 have no nodes in common.                */
/*                                                                           */
/*****************************************************************************/

bool KheLayerDisjoint(KHE_LAYER layer1, KHE_LAYER layer2)
{
  return LSetDisjoint(layer1->lset, layer2->lset);
}


/*****************************************************************************/
/*                                                                           */
/*  bool KheLayerContains(KHE_LAYER layer1, KHE_NODE n)                      */
/*                                                                           */
/*  Return true if layer1 contains n.                                        */
/*                                                                           */
/*****************************************************************************/

bool KheLayerContains(KHE_LAYER layer1, KHE_NODE n)
{
  return LSetContains(layer1->lset, KheNodeIndex(n));
}


/*****************************************************************************/
/*                                                                           */
/*  int KheNodeDecreasingDurationCmp(const void *t1, const void *t2)         */
/*                                                                           */
/*  Comparison function for sorting nodes into decreasing duration order,    */
/*  with ties broken by increasing number of meets.                          */
/*                                                                           */
/*****************************************************************************/

static int KheNodeDecreasingDurationCmp(const void *t1, const void *t2)
{
  KHE_NODE node1 = * (KHE_NODE *) t1;
  KHE_NODE node2 = * (KHE_NODE *) t2;
  if( KheNodeDuration(node1) != KheNodeDuration(node2) )
    return KheNodeDuration(node2) - KheNodeDuration(node1);
  else
    return KheNodeMeetCount(node1) - KheNodeMeetCount(node2);
}


/*****************************************************************************/
/*                                                                           */
/*  bool KheLayerAlign(KHE_LAYER layer1, KHE_LAYER layer2,                   */
/*    bool (*node_equiv)(KHE_NODE node1, KHE_NODE node2), int *count)        */
/*                                                                           */
/*  Align equivalent nodes of layer1 and layer2, defined according to        */
/*  node_equiv, and setting *count to the number that aligned in this way.   */
/*                                                                           */
/*****************************************************************************/

bool KheLayerAlign(KHE_LAYER layer1, KHE_LAYER layer2,
  bool (*node_equiv)(KHE_NODE node1, KHE_NODE node2), int *count)
{
  int i, j;  KHE_NODE node1, node2, tmp;
  *count = MArraySize(layer1->child_nodes);
  if( layer1 == layer2 )
    return true;
  MArraySort(layer1->child_nodes, &KheNodeDecreasingDurationCmp);
  MArraySort(layer2->child_nodes, &KheNodeDecreasingDurationCmp);
  for( i = 0;  i < *count;  i++ )
  {
    node1 = MArrayGet(layer1->child_nodes, i);
    for( j = i;  j < MArraySize(layer2->child_nodes);  j++ )
    {
      node2 = MArrayGet(layer2->child_nodes, j);
      if( node_equiv(node1, node2) )
	break;
    }
    if( j < MArraySize(layer2->child_nodes) )
    {
      /* node1 and node2 are similar */
      MArraySwap(layer2->child_nodes, i, j, tmp);
    }
    else
    {
      /* node1 is not similar to any node of layer2 */
      (*count)--;
      MArraySwap(layer1->child_nodes, *count, i, tmp);
      i--;
    }
  }
  return MArraySize(layer1->child_nodes) == MArraySize(layer2->child_nodes) &&
    MArraySize(layer1->child_nodes) == *count;
}


/*****************************************************************************/
/*                                                                           */
/*  bool KheLayerSame(KHE_LAYER layer1, KHE_LAYER layer2, int *same_count)   */
/*                                                                           */
/*  Align nodes of layer1 and layer2 that are the exact same node.           */
/*                                                                           */
/*****************************************************************************/

static bool KheDoNodeSame(KHE_NODE node1, KHE_NODE node2)
{
  return node1 == node2;
}

bool KheLayerSame(KHE_LAYER layer1, KHE_LAYER layer2, int *same_count)
{
  return KheLayerAlign(layer1, layer2, &KheDoNodeSame, same_count);
}


/*****************************************************************************/
/*                                                                           */
/*  bool KheLayerSimilar(KHE_LAYER layer1, KHE_LAYER layer2,                 */
/*    int *similar_count)                                                    */
/*                                                                           */
/*  Compare layer1 and layer2 for similarity, reordering the nodes so that   */
/*  the similar ones come first in both node layers, and setting             */
/*  *similar_count to the number of similarities.                            */
/*                                                                           */
/*****************************************************************************/

bool KheLayerSimilar(KHE_LAYER layer1, KHE_LAYER layer2, int *similar_count)
{
  return KheLayerAlign(layer1, layer2, &KheNodeSimilar, similar_count);
}


/*****************************************************************************/
/*                                                                           */
/*  bool KheLayerRegular(KHE_LAYER layer1, KHE_LAYER layer2,                 */
/*    int *regular_count)                                                    */
/*                                                                           */
/*  Return true when layer1 and layer2 are regular, setting *regular_count   */
/*  to the number of pairs of regular nodes.                                 */
/*                                                                           */
/*****************************************************************************/

static bool KheDoNodeRegular(KHE_NODE node1, KHE_NODE node2)
{
  int junk;
  return KheNodeRegular(node1, node2, &junk);
}

bool KheLayerRegular(KHE_LAYER layer1, KHE_LAYER layer2, int *regular_count)
{
  return KheLayerAlign(layer1, layer2, &KheDoNodeRegular, regular_count);
}


/*****************************************************************************/
/*                                                                           */
/*  void KheLayerMerge(KHE_LAYER layer1, KHE_LAYER layer2, KHE_LAYER *res)   */
/*                                                                           */
/*  Merge layer1 and layer2, producing *res.                                 */
/*                                                                           */
/*****************************************************************************/

void KheLayerMerge(KHE_LAYER layer1, KHE_LAYER layer2, KHE_LAYER *res)
{
  KHE_NODE child_node;  KHE_RESOURCE r;  int i, pos;
  MAssert(layer1 != layer2,
    "KheLayerMerge: layer1 and layer2 are the same layer");
  MAssert(layer1->parent_node == layer2->parent_node,
    "KheLayerMerge: layer1 and layer2 have different parent nodes");
  MArrayForEach(layer2->child_nodes, &child_node, &i)
    if( !KheLayerContains(layer1, child_node) )
      KheLayerAddChildNode(layer1, child_node);
  MArrayForEach(layer2->resources, &r, &i)
    if( !MArrayContains(layer1->resources, r, &pos) )
      MArrayAddLast(layer1->resources, r);
  KheLayerDelete(layer2);
  *res = layer1;
}


/*****************************************************************************/
/*                                                                           */
/*  void KheLayerDebug(KHE_LAYER layer, int verbosity, int indent, FILE *fp) */
/*                                                                           */
/*  Debug print of layer onto fp.                                            */
/*                                                                           */
/*****************************************************************************/

void KheLayerDebug(KHE_LAYER layer, int verbosity, int indent, FILE *fp)
{
  KHE_RESOURCE r;  KHE_NODE node;  int i;
  if( verbosity > 0 )
  {
    if( indent >= 0 )
      fprintf(fp, "%*s", indent, "");
    fprintf(fp, "[ Layer: %d nodes, durn %d %s(",
      MArraySize(layer->child_nodes), layer->duration,
      layer->duration > KheNodeDuration(layer->parent_node) ?
        "[OVERSIZE] " : "");
    if( verbosity <= 2 && MArraySize(layer->resources) > 5 )
    {
      for( i = 0;  i < 4;  i++ )
      {
	r = MArrayGet(layer->resources, i);
	fprintf(fp, "%s%s", i == 0 ? "" : ", ",
	KheResourceId(r) == NULL ? "-" : KheResourceId(r));
      }
      r = MArrayLast(layer->resources);
      fprintf(fp, ", ... , %s",
	KheResourceId(r) == NULL ? "-" : KheResourceId(r));
    }
    else
    {
      MArrayForEach(layer->resources, &r, &i)
	fprintf(fp, "%s%s", i == 0 ? "" : ", ",
	KheResourceId(r) == NULL ? "-" : KheResourceId(r));
    }
    fprintf(fp, ")");
    if( indent >= 0 && verbosity > 2 )
    {
      fprintf(fp, "\n");
      MArrayForEach(layer->child_nodes, &node, &i)
	KheNodeDebug(node, verbosity, indent + 2, fp);
      fprintf(fp, "%*s]\n", indent, "");
    }
    else
    {
      fprintf(fp, " ]");
      if( indent >= 0 )
	fprintf(fp, "\n");
    }
  }
}


/*****************************************************************************/
/*                                                                           */
/*  void KheLayerInstallZonesInParent(KHE_LAYER layer)                       */
/*                                                                           */
/*  Install zones in layer's parent corresponding to how the meets           */
/*  of the nodes of layer are assigned.                                      */
/*                                                                           */
/*  NB despite its location in this file, this function is really a          */
/*  solver; it depends on khe.h but not khe_interns.h.                       */
/*                                                                           */
/*****************************************************************************/

void KheLayerInstallZonesInParent(KHE_LAYER layer)
{
  KHE_NODE parent_node, child_node;  KHE_ZONE zone;
  KHE_MEET child_meet, parent_meet;  int i, j, k, parent_offset;
  if( DEBUG3 )
  {
    fprintf(stderr, "[ KheLayerInstallZonesInParent(");
    KheLayerDebug(layer, 1, -1, stderr);
    fprintf(stderr, ")\n");
  }

  /* remove existing zones from the parent of layer */
  parent_node = KheLayerParentNode(layer);
  while( KheNodeZoneCount(parent_node) > 0 )
    KheZoneDelete(KheNodeZone(parent_node, 0));

  /* make one zone for each child node of layer that has assigned meets */
  for( i = 0;  i < KheLayerChildNodeCount(layer);  i++ )
  {
    child_node = KheLayerChildNode(layer, i);
    if( KheNodeAssignedDuration(child_node) > 0 )
    {
      zone = KheZoneMake(parent_node);
      for( j = 0;  j < KheNodeMeetCount(child_node);  j++ )
      {
	child_meet = KheNodeMeet(child_node, j);
	if( KheMeetAsst(child_meet) != NULL )
	{
          parent_meet = KheMeetAsst(child_meet);
	  parent_offset = KheMeetAsstOffset(child_meet);
	  for( k = 0;  k < KheMeetDuration(child_meet);  k++ )
	    KheMeetOffsetAddZone(parent_meet, parent_offset + k, zone);
	}
      }
      if( DEBUG3 )
      {
	fprintf(stderr, "  Zone %d from child node ", KheZoneIndex(zone));
	KheNodeDebug(child_node, 1, 0, stderr);
      }
    }
  }

  /* all done */
  if( DEBUG3 )
  {
    /* ***
    fprintf(stderr, "  final parent node:");
    KheNodeDebug(parent_node, 3, 2, stderr);
    *** */
    fprintf(stderr, "] KheLayerInstallZonesInParent returning\n");
  }
}
