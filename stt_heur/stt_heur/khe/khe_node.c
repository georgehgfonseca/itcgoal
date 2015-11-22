
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
/*  FILE:         khe_node.c                                                 */
/*  DESCRIPTION:  A layer tree node                                          */
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


/*****************************************************************************/
/*                                                                           */
/*  KHE_NODE - a layer tree node                                             */
/*                                                                           */
/*****************************************************************************/

struct khe_node_rec {
  void				*back;			/* back pointer      */
  KHE_SOLN			soln;			/* enclosing soln    */
  int				index_in_soln;		/* index in soln     */
  int				visit_num;		/* visit_number      */
  KHE_NODE			parent_node;		/* optional parent   */
  ARRAY_KHE_LAYER		parent_layers;		/* parent layers     */
  ARRAY_KHE_NODE		child_nodes;		/* child nodes       */
  ARRAY_KHE_LAYER		child_layers;		/* child layers      */
  ARRAY_KHE_MEET		meets;			/* meets             */
  ARRAY_KHE_ZONE		zones;			/* zones             */
  int				duration;		/* total durn of ses */
  KHE_NODE			copy;			/* used when copying */
};


/*****************************************************************************/
/*                                                                           */
/*  Submodule "construction and query"                                       */
/*                                                                           */
/*****************************************************************************/

/*****************************************************************************/
/*                                                                           */
/*  KHE_NODE KheNodeMake(KHE_SOLN soln)                                      */
/*                                                                           */
/*  Make a new, empty layer tree node lying in soln.                         */
/*                                                                           */
/*****************************************************************************/

KHE_NODE KheNodeMake(KHE_SOLN soln)
{
  KHE_NODE res;
  MMake(res);
  res->back = NULL;
  res->soln = soln;
  KheSolnAddNode(soln, res, &res->index_in_soln);
  res->visit_num = 0;
  res->parent_node = NULL;
  MArrayInit(res->parent_layers);
  MArrayInit(res->child_nodes);
  MArrayInit(res->child_layers);
  MArrayInit(res->meets);
  MArrayInit(res->zones);
  res->duration = 0;
  res->copy = NULL;
  return res;
}


/*****************************************************************************/
/*                                                                           */
/*  void KheNodeSetBack(KHE_NODE node, void *back)                           */
/*                                                                           */
/*  Set the back pointer of node.                                            */
/*                                                                           */
/*****************************************************************************/

void KheNodeSetBack(KHE_NODE node, void *back)
{
  node->back = back;
}


/*****************************************************************************/
/*                                                                           */
/*  void *KheNodeBack(KHE_NODE node)                                         */
/*                                                                           */
/*  Return the back pointer of node.                                         */
/*                                                                           */
/*****************************************************************************/

void *KheNodeBack(KHE_NODE node)
{
  return node->back;
}


/*****************************************************************************/
/*                                                                           */
/*  KHE_SOLN KheNodeSoln(KHE_NODE node)                                      */
/*                                                                           */
/*  Return the solution containing node.                                     */
/*                                                                           */
/*****************************************************************************/

KHE_SOLN KheNodeSoln(KHE_NODE node)
{
  return node->soln;
}


/*****************************************************************************/
/*                                                                           */
/*  int KheNodeIndex(KHE_NODE node)                                          */
/*                                                                           */
/*  Return the index number of node in its solution.                         */
/*                                                                           */
/*****************************************************************************/

int KheNodeIndex(KHE_NODE node)
{
  return node->index_in_soln;
}


/*****************************************************************************/
/*                                                                           */
/*  void KheNodeSetIndex(KHE_NODE node, int index_in_soln)                   */
/*                                                                           */
/*  Set the index number of node.                                            */
/*                                                                           */
/*****************************************************************************/

void KheNodeSetIndex(KHE_NODE node, int index_in_soln)
{
  int i;  KHE_LAYER layer;
  if( index_in_soln != node->index_in_soln )
  {
    MArrayForEach(node->parent_layers, &layer, &i)
      KheLayerChangeNodeIndex(layer, node->index_in_soln, index_in_soln);
    node->index_in_soln = index_in_soln;
  }
}


/*****************************************************************************/
/*                                                                           */
/*  KHE_NODE KheNodeCopyPhase1(KHE_NODE node)                                */
/*                                                                           */
/*  Carry out Phase 1 of copying node.                                       */
/*                                                                           */
/*****************************************************************************/

KHE_NODE KheNodeCopyPhase1(KHE_NODE node)
{
  KHE_NODE copy, child_node;  KHE_MEET meet;  int i;  KHE_LAYER layer;
  KHE_ZONE zone;
  if (node==NULL)
      return;
  if( node->copy == NULL )
  {
    MMake(copy);
    node->copy = copy;
    copy->back = node->back;
    copy->soln = KheSolnCopyPhase1(node->soln);
    copy->index_in_soln = node->index_in_soln;
    copy->visit_num = node->visit_num;
    copy->parent_node = KheNodeCopyPhase1(node->parent_node);
    MArrayInit(copy->parent_layers);
    MArrayForEach(node->parent_layers, &layer, &i)
      MArrayAddLast(copy->parent_layers, KheLayerCopyPhase1(layer));
    MArrayInit(copy->child_nodes);
    MArrayForEach(node->child_nodes, &child_node, &i)
      MArrayAddLast(copy->child_nodes, KheNodeCopyPhase1(child_node));
    MArrayInit(copy->child_layers);
    MArrayForEach(node->child_layers, &layer, &i)
      MArrayAddLast(copy->child_layers, KheLayerCopyPhase1(layer));
    MArrayInit(copy->meets);
    MArrayForEach(node->meets, &meet, &i)
      MArrayAddLast(copy->meets, KheMeetCopyPhase1(meet));
    MArrayInit(copy->zones);
    MArrayForEach(node->zones, &zone, &i)
      MArrayAddLast(copy->zones, KheZoneCopyPhase1(zone));
    copy->duration = node->duration;
    copy->copy = NULL;
  }
  return node->copy;
}


/*****************************************************************************/
/*                                                                           */
/*  void KheNodeCopyPhase2(KHE_NODE node)                                    */
/*                                                                           */
/*  Carry out Phase 2 of copying node.                                       */
/*                                                                           */
/*****************************************************************************/

void KheNodeCopyPhase2(KHE_NODE node)
{
  int i;  KHE_ZONE zone;
  if( node->copy != NULL )
  {
    node->copy = NULL;
    MArrayForEach(node->zones, &zone, &i)
      KheZoneCopyPhase2(zone);
  }
}


/*****************************************************************************/
/*                                                                           */
/*  void KheNodeFree(KHE_NODE node)                                          */
/*                                                                           */
/*  Free the memory occupied by node.                                        */
/*                                                                           */
/*****************************************************************************/

static void KheNodeFree(KHE_NODE node)
{
  MArrayFree(node->parent_layers);
  MArrayFree(node->child_nodes);
  MArrayFree(node->child_layers);
  MArrayFree(node->meets);
  MArrayFree(node->zones);
  MFree(node);
}


/*****************************************************************************/
/*                                                                           */
/*  bool KheNodeDeleteCheck(KHE_NODE node)                                   */
/*                                                                           */
/*  Check whether it is safe to delete node, which is true when all of the   */
/*  meets lying in node are willing to be removed from it.                   */
/*                                                                           */
/*****************************************************************************/

bool KheNodeDeleteCheck(KHE_NODE node)
{
  KHE_MEET meet;  int i;
  MArrayForEach(node->meets, &meet, &i)
    if( !KheMeetDeleteNodeCheck(meet, node) )
      return false;
  return true;
}


/*****************************************************************************/
/*                                                                           */
/*  bool KheNodeDelete(KHE_NODE node)                                        */
/*                                                                           */
/*  Delete node.                                                             */
/*                                                                           */
/*****************************************************************************/

bool KheNodeDelete(KHE_NODE node)
{
  /* check safe to proceed */
  if( !KheNodeDeleteCheck(node) )
    return false;

  /* delete zones */
  while( MArraySize(node->zones) > 0 )
    KheZoneDelete(MArrayLast(node->zones));

  /* delete meets */
  while( MArraySize(node->meets) > 0 )
    if( !KheNodeDeleteMeet(node, MArrayLast(node->meets)) )
      MAssert(false, "KheNodeDelete internal error 2");

  /* delete from parent layers */
  while( MArraySize(node->parent_layers) > 0 )
    KheLayerDeleteChildNode(MArrayLast(node->parent_layers), node);

  /* delete from parent */
  if( node->parent_node != NULL && !KheNodeDeleteParent(node) )
    MAssert(false, "KheNodeDelete internal error 3");

  /* delete all child layers */
  KheNodeChildLayersDelete(node);

  /* delete from child_nodes */
  while( MArraySize(node->child_nodes) > 0 )
    if( !KheNodeDeleteParent(MArrayLast(node->child_nodes)) )
      MAssert(false, "KheNodeDelete internal error 4");

  /* delete and free node */
  KheSolnDeleteNode(node->soln, node);
  KheNodeFree(node);
  return true;
}


/*****************************************************************************/
/*                                                                           */
/*  Submodule "unchecked operations"                                         */
/*                                                                           */
/*  These basic operations change the state of nodes while maintaining       */
/*  that part of the invariant that relates to nodes and layers.             */
/*                                                                           */
/*  Implementation note.  These functions do not assume that the nodes       */
/*  they are given have parents.  There are always cases where this needs    */
/*  to be checked, and doing it here seems best.                             */
/*                                                                           */
/*****************************************************************************/

/*****************************************************************************/
/*                                                                           */
/*  void KheNodeUncheckedAddMeet(KHE_NODE node, KHE_MEET meet)               */
/*                                                                           */
/*  Add meet to node.                                                        */
/*                                                                           */
/*****************************************************************************/

static void KheNodeUncheckedAddMeet(KHE_NODE node, KHE_MEET meet)
{
  KHE_LAYER layer;  int i;
  MArrayAddLast(node->meets, meet);
  node->duration += KheMeetDuration(meet);
  MArrayForEach(node->parent_layers, &layer, &i)
    KheLayerAddDuration(layer, KheMeetDuration(meet));
}


/*****************************************************************************/
/*                                                                           */
/*  void KheNodeUncheckedDeleteMeet(KHE_NODE node, int meet_index)           */
/*                                                                           */
/*  Delete the meet at index meet_index from node.                           */
/*                                                                           */
/*****************************************************************************/

static void KheNodeUncheckedDeleteMeet(KHE_NODE node, int meet_index)
{
  KHE_MEET meet;  KHE_LAYER layer;  int i;
  meet = MArrayGet(node->meets, meet_index);
  MArrayRemove(node->meets, meet_index);
  node->duration -= KheMeetDuration(meet);
  MArrayForEach(node->parent_layers, &layer, &i)
    KheLayerSubtractDuration(layer, KheMeetDuration(meet));
}


/*****************************************************************************/
/*                                                                           */
/*  void KheNodeUncheckedAddChildNode(KHE_NODE parent_node,                  */
/*    KHE_NODE child_node)                                                   */
/*                                                                           */
/*  Add child_node to parent_node; initially, child_node has no parent.      */
/*                                                                           */
/*****************************************************************************/

static void KheNodeUncheckedAddChildNode(KHE_NODE parent_node,
  KHE_NODE child_node)
{
  MAssert(child_node->parent_node == NULL,
    "KheNodeUncheckedAddChildNode internal error");
  MArrayAddLast(parent_node->child_nodes, child_node);
  child_node->parent_node = parent_node;
}


/*****************************************************************************/
/*                                                                           */
/*  void KheNodeUncheckedDeleteChildNode(KHE_NODE parent_node,               */
/*    int child_node_index)                                                  */
/*                                                                           */
/*  Delete the child node at index child_node_index from parent_node.        */
/*                                                                           */
/*****************************************************************************/

static void KheNodeUncheckedDeleteChildNode(KHE_NODE parent_node,
  int child_node_index)
{
  KHE_NODE child_node;
  child_node = MArrayGet(parent_node->child_nodes, child_node_index);
  while( MArraySize(child_node->parent_layers) > 0 )
    KheLayerDeleteChildNode(MArrayLast(child_node->parent_layers), child_node);
  child_node->parent_node = NULL;
  MArrayRemove(parent_node->child_nodes, child_node_index);
}


/*****************************************************************************/
/*                                                                           */
/*  Submodule "parents and children"                                         */
/*                                                                           */
/*****************************************************************************/

/*****************************************************************************/
/*                                                                           */
/*  bool KheNodeAddParentCheck(KHE_NODE child_node, KHE_NODE parent_node)    */
/*                                                                           */
/*  Check whether linking these two nodes as child and parent is possible.   */
/*  There is no possibility of a violation of the node rule.                 */
/*                                                                           */
/*****************************************************************************/

bool KheNodeAddParentCheck(KHE_NODE child_node, KHE_NODE parent_node)
{
  KHE_NODE node;

  /* check for already assigned */
  MAssert(child_node->parent_node == NULL,
    "KheNodeAddParentCheck: child_node already has a parent");

  /* check for cycles, including a node being its own parent */
  for( node = parent_node;  node != NULL;  node = node->parent_node )
    if( node == child_node )
      return false;  /* cycle */
  return true;
}


/*****************************************************************************/
/*                                                                           */
/*  bool KheNodeAddParent(KHE_NODE child_node, KHE_NODE parent_node)         */
/*                                                                           */
/*  Link these two nodes as child and parent, if possible.                   */
/*                                                                           */
/*****************************************************************************/

bool KheNodeAddParent(KHE_NODE child_node, KHE_NODE parent_node)
{
  /* check safe to do */
  if( !KheNodeAddParentCheck(child_node, parent_node) )
    return false;

  /* link children and parents */
  KheSolnOpNodeAddParent(child_node->soln, child_node, parent_node);
  KheNodeUncheckedAddChildNode(parent_node, child_node);
  return true;
}


/*****************************************************************************/
/*                                                                           */
/*  bool KheNodeDeleteParentCheck(KHE_NODE child_node)                       */
/*                                                                           */
/*  Check whether deleting the link connecting child_node to its parent is   */
/*  possible.  There is no possibility of violating the cycle rule.          */
/*                                                                           */
/*****************************************************************************/

bool KheNodeDeleteParentCheck(KHE_NODE child_node)
{
  KHE_MEET meet;  int i;

  /* check already assigned */
  MAssert(child_node->parent_node != NULL,
    "KheNodeDeleteParentCheck: child_node has no parent");

  /* check for potential violations of the node rule */
  MArrayForEach(child_node->meets, &meet, &i)
    if( KheMeetAsst(meet) != NULL )
      return false;
  return true;
}


/*****************************************************************************/
/*                                                                           */
/*  bool KheNodeDeleteParent(KHE_NODE child_node)                            */
/*                                                                           */
/*  Delete the link connecting child_node to its parent, if possible.        */
/*                                                                           */
/*****************************************************************************/

bool KheNodeDeleteParent(KHE_NODE child_node)
{
  int pos;

  /* check safe to do */
  if( !KheNodeDeleteParentCheck(child_node) )
    return false;

  /* unlink children and parents */
  KheSolnOpNodeDeleteParent(child_node->soln, child_node,
    child_node->parent_node);
  if( !MArrayContains(child_node->parent_node->child_nodes, child_node, &pos) )
    MAssert(false, "KheNodeDeleteParent internal error 2");
  KheNodeUncheckedDeleteChildNode(child_node->parent_node, pos);
  return true;
}


/*****************************************************************************/
/*                                                                           */
/*  KHE_NODE KheNodeParent(KHE_NODE node)                                    */
/*                                                                           */
/*  Return the parent of node, or NULL if none.                              */
/*                                                                           */
/*****************************************************************************/

KHE_NODE KheNodeParent(KHE_NODE node)
{
  return node->parent_node;
}


/*****************************************************************************/
/*                                                                           */
/*  int KheNodeChildCount(KHE_NODE node)                                     */
/*                                                                           */
/*  Return the number of children of node.                                   */
/*                                                                           */
/*****************************************************************************/

int KheNodeChildCount(KHE_NODE node)
{
  return MArraySize(node->child_nodes);
}


/*****************************************************************************/
/*                                                                           */
/*  KHE_NODE KheNodeChild(KHE_NODE node, int i)                              */
/*                                                                           */
/*  Return the i'th child of node.                                           */
/*                                                                           */
/*****************************************************************************/

KHE_NODE KheNodeChild(KHE_NODE node, int i)
{
  return MArrayGet(node->child_nodes, i);
}


/*****************************************************************************/
/*                                                                           */
/*  Submodule "parent layers"                                                */
/*                                                                           */
/*****************************************************************************/

/*****************************************************************************/
/*                                                                           */
/*  void KheNodeAddParentLayer(KHE_NODE child_node, KHE_LAYER layer)         */
/*                                                                           */
/*  Add layer to child_node's set of parent layers.                          */
/*                                                                           */
/*****************************************************************************/

void KheNodeAddParentLayer(KHE_NODE child_node, KHE_LAYER layer)
{
  MArrayAddLast(child_node->parent_layers, layer);
}


/*****************************************************************************/
/*                                                                           */
/*  void KheNodeDeleteParentLayer(KHE_NODE child_node, KHE_LAYER layer)      */
/*                                                                           */
/*  Remove layer from child_node's set of parent layers.                     */
/*                                                                           */
/*****************************************************************************/

void KheNodeDeleteParentLayer(KHE_NODE child_node, KHE_LAYER layer)
{
  int pos;
  if( !MArrayContains(child_node->parent_layers, layer, &pos) )
    MAssert(false, "KheNodeDeleteParentLayer internal error");
  MArrayRemove(child_node->parent_layers, pos);
}


/*****************************************************************************/
/*                                                                           */
/*  int KheNodeParentLayerCount(KHE_NODE child_node)                         */
/*                                                                           */
/*  Return the number of parent layers of child_node.                        */
/*                                                                           */
/*****************************************************************************/

int KheNodeParentLayerCount(KHE_NODE child_node)
{
  return MArraySize(child_node->parent_layers);
}


/*****************************************************************************/
/*                                                                           */
/*  KHE_LAYER KheNodeParentLayer(KHE_NODE child_node, int i)                 */
/*                                                                           */
/*  Return the i                                                             */
/*                                                                           */
/*****************************************************************************/

KHE_LAYER KheNodeParentLayer(KHE_NODE child_node, int i)
{
  return MArrayGet(child_node->parent_layers, i);
}


/*****************************************************************************/
/*                                                                           */
/*  bool KheNodeSameParentLayers(KHE_NODE node1, KHE_NODE node2)             */
/*                                                                           */
/*  Return true if node1 and node2 have the same parent layers.              */
/*                                                                           */
/*****************************************************************************/

bool KheNodeSameParentLayers(KHE_NODE node1, KHE_NODE node2)
{
  KHE_LAYER layer;  int i, pos;
  if( MArraySize(node1->parent_layers) != MArraySize(node2->parent_layers) )
    return false;
  MArrayForEach(node1->parent_layers, &layer, &i)
    if( !MArrayContains(node2->parent_layers, layer, &pos) )
      return false;
  return true;
}


/*****************************************************************************/
/*                                                                           */
/*  Submodule "child layers"                                                 */
/*                                                                           */
/*****************************************************************************/

/*****************************************************************************/
/*                                                                           */
/*  void KheNodeAddChildLayer(KHE_NODE parent_node, KHE_LAYER layer,         */
/*    int *index)                                                            */
/*                                                                           */
/*  Add layer as a new child layer to parent_node.                           */
/*                                                                           */
/*****************************************************************************/

void KheNodeAddChildLayer(KHE_NODE parent_node, KHE_LAYER layer, int *index)
{
  *index = MArraySize(parent_node->child_layers);
  MArrayAddLast(parent_node->child_layers, layer);
}


/*****************************************************************************/
/*                                                                           */
/*  void KheNodeDeleteChildLayer(KHE_NODE parent_node, KHE_LAYER layer)      */
/*                                                                           */
/*  Delete this child layer from parent_node.                                */
/*                                                                           */
/*****************************************************************************/

void KheNodeDeleteChildLayer(KHE_NODE parent_node, KHE_LAYER layer)
{
  int pos, i;
  if( !MArrayContains(parent_node->child_layers, layer, &pos) )
    MAssert(false, "KheNodeDeleteChildLayer internal error");
  MArrayRemove(parent_node->child_layers, pos);
  for( i = pos;  i < MArraySize(parent_node->child_layers);  i++ )
  {
    layer = MArrayGet(parent_node->child_layers, i);
    KheLayerSetIndex(layer, i);
  }
}


/*****************************************************************************/
/*                                                                           */
/*  int KheNodeChildLayerCount(KHE_NODE parent_node)                         */
/*                                                                           */
/*  Return the number of child layers of parent_node.                        */
/*                                                                           */
/*****************************************************************************/

int KheNodeChildLayerCount(KHE_NODE parent_node)
{
  return MArraySize(parent_node->child_layers);
}


/*****************************************************************************/
/*                                                                           */
/*  KHE_LAYER KheNodeChildLayer(KHE_NODE parent_node, int i)                 */
/*                                                                           */
/*  Return the i'th child layer of parent_node.                              */
/*                                                                           */
/*****************************************************************************/

KHE_LAYER KheNodeChildLayer(KHE_NODE parent_node, int i)
{
  return MArrayGet(parent_node->child_layers, i);
}


/*****************************************************************************/
/*                                                                           */
/*  void KheCNodehildLayersSort(KHE_NODE parent_node,                        */
/*    int(*compar)(const void *, const void *))                              */
/*                                                                           */
/*  Sort the child arrays of parent_node, using comparison function compar.  */
/*                                                                           */
/*****************************************************************************/

void KheNodeChildLayersSort(KHE_NODE parent_node,
  int(*compar)(const void *, const void *))
{
  KHE_LAYER layer;  int i;
  MArraySort(parent_node->child_layers, compar);
  MArrayForEach(parent_node->child_layers, &layer, &i)
    KheLayerSetIndex(layer, i);
}


/*****************************************************************************/
/*                                                                           */
/*  void KheNodeChildLayersDelete(KHE_NODE parent_node)                      */
/*                                                                           */
/*  Delete all the child layers of parent_node;                              */
/*                                                                           */
/*****************************************************************************/

void KheNodeChildLayersDelete(KHE_NODE parent_node)
{
  while( MArraySize(parent_node->child_layers) > 0 )
    KheLayerDelete(MArrayLast(parent_node->child_layers));
}


/*****************************************************************************/
/*                                                                           */
/*  Submodule "meets"                                                        */
/*                                                                           */
/*****************************************************************************/

/*****************************************************************************/
/*                                                                           */
/*  bool KheNodeAddMeetCheck(KHE_NODE node, KHE_MEET meet)                   */
/*                                                                           */
/*  Check whether adding meet to node is possible.                           */
/*                                                                           */
/*****************************************************************************/

bool KheNodeAddMeetCheck(KHE_NODE node, KHE_MEET meet)
{
  return KheMeetAddNodeCheck(meet, node);
}


/*****************************************************************************/
/*                                                                           */
/*  bool KheNodeAddMeet(KHE_NODE node, KHE_MEET meet)                        */
/*                                                                           */
/*  Add meet to node, if possible.                                           */
/*                                                                           */
/*****************************************************************************/

bool KheNodeAddMeet(KHE_NODE node, KHE_MEET meet)
{
  /* check safe to do */
  if( !KheNodeAddMeetCheck(node, meet) )
    return false;

  /* add meet to node, and add node to meet */
  KheNodeUncheckedAddMeet(node, meet);
  KheMeetSetNodeInternal(meet, node);
  return true;
}


/*****************************************************************************/
/*                                                                           */
/*  bool KheNodeDeleteMeetCheck(KHE_NODE node, KHE_MEET meet)                */
/*                                                                           */
/*  Check whether deleting meet from node is possible.                       */
/*                                                                           */
/*****************************************************************************/

bool KheNodeDeleteMeetCheck(KHE_NODE node, KHE_MEET meet)
{
  return KheMeetDeleteNodeCheck(meet, node);
}


/*****************************************************************************/
/*                                                                           */
/*  bool KheNodeDeleteMeet(KHE_NODE node, KHE_MEET meet)                     */
/*                                                                           */
/*  Delete meet from node, if possible.                                      */
/*                                                                           */
/*****************************************************************************/

bool KheNodeDeleteMeet(KHE_NODE node, KHE_MEET meet)
{
  int pos;

  /* check safe to do */
  if( !KheNodeDeleteMeetCheck(node, meet) )
    return false;

  /* delete meet from node, and delete node from meet */
  if( !MArrayContains(node->meets, meet, &pos) )
    MAssert(false, "KheNodeDeleteMeet internal error");
  KheNodeUncheckedDeleteMeet(node, pos);
  KheMeetSetNodeInternal(meet, NULL);
  return true;
}


/*****************************************************************************/
/*                                                                           */
/*  void KheNodeAddSplitMeet(KHE_NODE node, KHE_MEET meet)                   */
/*                                                                           */
/*  Similar to KheNodeAddMeet except that meet was created by a split,       */
/*  so no checks or changes to the duration or segments are needed.          */
/*                                                                           */
/*****************************************************************************/

void KheNodeAddSplitMeet(KHE_NODE node, KHE_MEET meet)
{
  MArrayAddLast(node->meets, meet);
}


/*****************************************************************************/
/*                                                                           */
/*  void KheNodeDeleteSplitMeet(KHE_NODE node, KHE_MEET meet)                */
/*                                                                           */
/*  Similar to KheNodeDeleteMeet except that meet is being removed by        */
/*  a merge operation, so no checks or changes to the duration or segments   */
/*  are needed.                                                              */
/*                                                                           */
/*****************************************************************************/

void KheNodeDeleteSplitMeet(KHE_NODE node, KHE_MEET meet)
{
  int pos;
  if( !MArrayContains(node->meets, meet, &pos) )
    MAssert(false, "KheNodeDeleteSplitMeet internal error");
  MArrayRemove(node->meets, pos);
}


/*****************************************************************************/
/*                                                                           */
/*  int KheNodeMeetCount(KHE_NODE node)                                      */
/*                                                                           */
/*  Return the number of meets of node.                                      */
/*                                                                           */
/*****************************************************************************/

int KheNodeMeetCount(KHE_NODE node)
{
  return MArraySize(node->meets);
}


/*****************************************************************************/
/*                                                                           */
/*  KHE_MEET KheNodeMeet(KHE_NODE node, int i)                               */
/*                                                                           */
/*  Return the i'th meet of node.                                            */
/*                                                                           */
/*****************************************************************************/

KHE_MEET KheNodeMeet(KHE_NODE node, int i)
{
  return MArrayGet(node->meets, i);
}


/*****************************************************************************/
/*                                                                           */
/*  void KheNodeMeetSort(KHE_NODE node,                                      */
/*    int(*compar)(const void *, const void *))                              */
/*                                                                           */
/*  Function for sorting the meets of node using compar for comparison.      */
/*                                                                           */
/*****************************************************************************/

void KheNodeMeetSort(KHE_NODE node, int(*compar)(const void *, const void *))
{
  MArraySort(node->meets, compar);
}


/*****************************************************************************/
/*                                                                           */
/*  int KheMeetDecreasingDurationCmp(const void *p1, const void *p2)         */
/*                                                                           */
/*  Comparison function for sorting an array of meets by decreasing          */
/*  duration order.                                                          */
/*                                                                           */
/*****************************************************************************/

int KheMeetDecreasingDurationCmp(const void *p1, const void *p2)
{
  KHE_MEET meet1 = * (KHE_MEET *) p1;
  KHE_MEET meet2 = * (KHE_MEET *) p2;
  if( KheMeetDuration(meet1) != KheMeetDuration(meet2) )
    return KheMeetDuration(meet2) - KheMeetDuration(meet1);
  else
    return KheMeetIndex(meet1) - KheMeetIndex(meet2);
}


/*****************************************************************************/
/*                                                                           */
/*  int KheMeetIncreasingAsstCmp(const void *p1, const void *p2)             */
/*                                                                           */
/*  Comparison function for sorting an array of meets by increasing          */
/*  assignment order.                                                        */
/*                                                                           */
/*****************************************************************************/

int KheMeetIncreasingAsstCmp(const void *p1, const void *p2)
{
  KHE_MEET meet1 = * (KHE_MEET *) p1;
  KHE_MEET meet2 = * (KHE_MEET *) p2;
  int index1, index2;
  if( KheMeetAsst(meet1) == NULL )
  {
    if( KheMeetAsst(meet2) == NULL )
      return 0;
    else
      return 1;
  }
  else
  {
    if( KheMeetAsst(meet2) == NULL )
      return -1;
    else
    {
      index1 = KheMeetIndex(KheMeetAsst(meet1));
      index2 = KheMeetIndex(KheMeetAsst(meet2));
      if( index1 != index2 )
	return index1 - index2;
      else
	return KheMeetAsstOffset(meet1) - KheMeetAsstOffset(meet2);
    }
  }
}


/*****************************************************************************/
/*                                                                           */
/*  int KheNodeDuration(KHE_NODE node)                                       */
/*                                                                           */
/*  Return the total duration of the meets of node.                          */
/*                                                                           */
/*****************************************************************************/

int KheNodeDuration(KHE_NODE node)
{
  return node->duration;
}


/*****************************************************************************/
/*                                                                           */
/*  int KheNodeAssignedDuration(KHE_NODE node)                               */
/*                                                                           */
/*  Return the total assigned duration of the meets of node.                 */
/*                                                                           */
/*****************************************************************************/

int KheNodeAssignedDuration(KHE_NODE node)
{
  KHE_MEET meet;  int i, res;
  res = 0;
  MArrayForEach(node->meets, &meet, &i)
    res += KheMeetAssignedDuration(meet);
  return res;
}


/*****************************************************************************/
/*                                                                           */
/*  int KheNodeDemand(KHE_NODE node)                                         */
/*                                                                           */
/*  Return the total demand of the meets of node.                            */
/*                                                                           */
/*****************************************************************************/

int KheNodeDemand(KHE_NODE node)
{
  KHE_MEET meet;  int i, res;
  res = 0;
  MArrayForEach(node->meets, &meet, &i)
    res += KheMeetDemand(meet);
  return res;
}


/*****************************************************************************/
/*                                                                           */
/*  Submodule "merging and splitting"                                        */
/*                                                                           */
/*****************************************************************************/

/*****************************************************************************/
/*                                                                           */
/*  bool KheNodeMergeCheck(KHE_NODE node1, KHE_NODE node2)                   */
/*                                                                           */
/*  Check whether node1 and node2 can be merged.                             */
/*                                                                           */
/*****************************************************************************/

bool KheNodeMergeCheck(KHE_NODE node1, KHE_NODE node2)
{
  /* must have same parent node and parent layers */
  return node1->parent_node == node2->parent_node &&
    KheNodeSameParentLayers(node1, node2);
}


/*****************************************************************************/
/*                                                                           */
/*  bool KheNodeMerge(KHE_NODE node1, KHE_NODE node2, KHE_NODE *res)         */
/*                                                                           */
/*  Merge node1 and node2 into *res.                                         */
/*                                                                           */
/*****************************************************************************/

bool KheNodeMerge(KHE_NODE node1, KHE_NODE node2, KHE_NODE *res)
{
  KHE_NODE child_node;  KHE_MEET meet;
  if( DEBUG3 )
    fprintf(stderr, "[ KheNodeMerge(Node %d, Node %d, &res)\n",
      KheNodeIndex(node1), KheNodeIndex(node2));

  if( !KheNodeMergeCheck(node1, node2) )
    return false;

  /* move the children of node2 to node1 */
  while( MArraySize(node2->child_nodes) > 0 )
  {
    child_node = MArrayFirst(node2->child_nodes);
    KheNodeUncheckedDeleteChildNode(node2, 0);
    KheNodeUncheckedAddChildNode(node1, child_node);
  }

  /* move the meets of node2 to node1 */
  while( MArraySize(node2->meets) > 0 )
  {
    meet = MArrayFirst(node2->meets);
    KheNodeUncheckedDeleteMeet(node2, 0);
    KheNodeUncheckedAddMeet(node1, meet);
    KheMeetSetNodeInternal(meet, node1);
  }

  /* delete and free node2 (must work: it has no children or meets) */
  KheNodeDelete(node2);
  *res = node1;
  if( DEBUG3 )
    fprintf(stderr, "] KheNodeMerge returning\n");
  return true;
}


/*****************************************************************************/
/*                                                                           */
/*  bool KheNodeSplitCheck(KHE_NODE node, int meet_count1, int child_count1) */
/*                                                                           */
/*  Check whether it is safe to split node into two with these attributes.   */
/*                                                                           */
/*****************************************************************************/

bool KheNodeSplitCheck(KHE_NODE node, int meet_count1, int child_count1)
{
  KHE_NODE child_node;  KHE_MEET meet, target_meet;  int i, j, pos;

  MAssert(0 <= meet_count1 && meet_count1 <= MArraySize(node->meets),
    "KheNodeSplitCheck: meet_count1 (%d) out of range 0..%d", meet_count1,
    MArraySize(node->meets));
  MAssert(0 <= child_count1 && child_count1 <= MArraySize(node->child_nodes),
    "KheNodeSplitCheck: child_count1 (%d) out of range 0..%d", child_count1,
    MArraySize(node->child_nodes));
  MArrayForEach(node->child_nodes, &child_node, &i)
    MArrayForEach(child_node->meets, &meet, &j)
    {
      target_meet = KheMeetAsst(meet);
      if( target_meet != NULL )
      {
	if( !MArrayContains(node->meets, target_meet, &pos) )
	  MAssert(false, "KheNodeSplitCheck internal error");
	if( pos < meet_count1 && j >= child_count1 )
	  return false;
      }
    }
  return true;
}


/*****************************************************************************/
/*                                                                           */
/*  bool KheNodeSplit(KHE_NODE node, int meet_count1, int child_count1,      */
/*    KHE_NODE *res1, KHE_NODE *res2)                                        */
/*                                                                           */
/*  Split node into *res1 and *res2.                                         */
/*                                                                           */
/*****************************************************************************/

bool KheNodeSplit(KHE_NODE node, int meet_count1, int child_count1,
  KHE_NODE *res1, KHE_NODE *res2)
{
  KHE_NODE res, child_node;  KHE_MEET meet;  KHE_LAYER layer;  int i;

  /* check safe to split */
  if( !KheNodeSplitCheck(node, meet_count1, child_count1) )
    return false;

  /* make res, a new node with the same parent and parent layers as node */
  res = KheNodeMake(node->soln);
  if( node->parent_node != NULL )
    KheNodeAddParent(res, node->parent_node);
  MArrayForEach(node->parent_layers, &layer, &i)
    KheLayerAddChildNode(layer, res);

  /* move the children at and after index child_count1 to res */
  while( child_count1 < MArraySize(node->child_nodes) )
  {
    child_node = MArrayGet(node->child_nodes, child_count1);
    KheNodeUncheckedDeleteChildNode(node, child_count1);
    KheNodeUncheckedAddChildNode(res, child_node);
  }

  /* move the meets at and after index meet_count1 to res */
  while( meet_count1 < MArraySize(node->meets) )
  {
    meet = MArrayGet(node->meets, meet_count1);
    KheNodeUncheckedDeleteMeet(node, meet_count1);
    KheMeetSetNodeInternal(meet, res);
    KheNodeUncheckedAddMeet(res, meet);
  }

  /* set return parameters and return */
  *res1 = node;
  *res2 = res;
  return true;
}


/*****************************************************************************/
/*                                                                           */
/*  Submodule "node meet merging and splitting"                              */
/*                                                                           */
/*****************************************************************************/

/*****************************************************************************/
/*                                                                           */
/*  void KheNodeMeetSplit(KHE_NODE node, bool recursive)                     */
/*                                                                           */
/*  Split the meets of node as much as possible.                             */
/*                                                                           */
/*  Implementation note.  KheMeetSplit causes the new meet to be added       */
/*  to the end of node's meets.  This function gets around to it there.      */
/*                                                                           */
/*****************************************************************************/

void KheNodeMeetSplit(KHE_NODE node, bool recursive)
{
  KHE_MEET meet, junk;  int i, durn;
  MArrayForEach(node->meets, &meet, &i)
    for( durn = 1;  durn < KheMeetDuration(meet);  durn++ )
      KheMeetSplit(meet, durn, recursive, &meet, &junk);
}


/*****************************************************************************/
/*                                                                           */
/*  void KheNodeMeetMerge(KHE_NODE node)                                     */
/*                                                                           */
/*  Merge the meets of node as much as possible.                             */
/*                                                                           */
/*  Implementation note.  KheMeetMerge causes the departing meet to be       */
/*  removed from the node's meets without otherwise reordering them.         */
/*                                                                           */
/*****************************************************************************/

void KheNodeMeetMerge(KHE_NODE node)
{
  KHE_MEET meet;  int i;
  KheNodeMeetSort(node, &KheMeetIncreasingAsstCmp);
  MArrayForEach(node->meets, &meet, &i)
    while( i + 1 < MArraySize(node->meets) &&
      KheMeetMerge(meet, MArrayGet(node->meets, i + 1), &meet) );
}


/*****************************************************************************/
/*                                                                           */
/*  Submodule "moving"                                                       */
/*                                                                           */
/*****************************************************************************/

/*****************************************************************************/
/*                                                                           */
/*  bool KheNodeMoveCheck(KHE_NODE child_node, KHE_NODE parent_node)         */
/*                                                                           */
/*  Check that it is safe to move child_node so that its parent node is      */
/*  parent_node.                                                             */
/*                                                                           */
/*****************************************************************************/

bool KheNodeMoveCheck(KHE_NODE child_node, KHE_NODE parent_node)
{
  KHE_NODE node;

  /* check for cycles, including a node being its own parent */
  for( node = parent_node;  node != NULL;  node = node->parent_node )
    if( node == child_node )
      return false;
  return true;
}


/*****************************************************************************/
/*                                                                           */
/*  bool KheNodeMove(KHE_NODE child_node, KHE_NODE parent_node)              */
/*                                                                           */
/*  Move child_node so that its parent node is parent_node.                  */
/*                                                                           */
/*****************************************************************************/

bool KheNodeMove(KHE_NODE child_node, KHE_NODE parent_node)
{
  KHE_NODE n;  KHE_MEET meet, target_meet;  int i, target_offset;

  if( !KheNodeMoveCheck(child_node, parent_node) )
    return false;

  n = child_node->parent_node;
  for( ;  n != NULL && n != parent_node;  n = n->parent_node);
  if( n == NULL )
  {
    /* parent_node not ancestor of child_node's parent; unassign meets */
    MArrayForEach(child_node->meets, &meet, &i)
      if( KheMeetAsst(meet) != NULL )
	KheMeetUnAssign(meet);

    /* move child_node to parent_node */
    if( !KheNodeDeleteParent(child_node) )
      MAssert(false, "KheNodeMove internal error 1");
    if( !KheNodeAddParent(child_node, parent_node) )
      MAssert(false, "KheNodeMove internal error 2");
  }
  else
  {
    /* parent_node is ancestor; unassign meets, remembering targets */
    ARRAY_KHE_MEET target_meets;  ARRAY_INT target_offsets;
    MArrayInit(target_meets);
    MArrayInit(target_offsets);
    MArrayForEach(child_node->meets, &meet, &i)
    {
      KheMeetFindNodeTarget(meet, parent_node, &target_meet, &target_offset);
      MArrayAddLast(target_meets, target_meet);
      MArrayAddLast(target_offsets, target_offset);
      if( KheMeetAsst(meet) != NULL )
	KheMeetUnAssign(meet);
    }

    /* move child_node to parent_node */
    if( !KheNodeDeleteParent(child_node) )
      MAssert(false, "KheNodeMove internal error 3");
    if( !KheNodeAddParent(child_node, parent_node) )
      MAssert(false, "KheNodeMove internal error 4");

    /* reassign child_node's meets */
    MArrayForEach(child_node->meets, &meet, &i)
    {
      target_meet = MArrayGet(target_meets, i);
      target_offset = MArrayGet(target_offsets, i);
      if( target_meet != NULL &&
	  !KheMeetAssign(meet, target_meet, target_offset) )
	MAssert(false, "KheNodeMove internal error 5");
    }
    MArrayFree(target_meets);
    MArrayFree(target_offsets);
  }
  return true;
}


/*****************************************************************************/
/*                                                                           */
/*  Submodule "meet swapping"                                                */
/*                                                                           */
/*****************************************************************************/

/*****************************************************************************/
/*                                                                           */
/*  bool KheNodeMeetSwapCheck(KHE_NODE node1, KHE_NODE node2)                */
/*                                                                           */
/*  Check whether it is possible to swap the assignments of the meets of     */
/*  node1 and node2.                                                         */
/*                                                                           */
/*****************************************************************************/

bool KheNodeMeetSwapCheck(KHE_NODE node1, KHE_NODE node2)
{
  KHE_MEET meet1, meet2;  int i;

  /* both nodes must be non-NULL */
  MAssert(node1 != NULL, "KheNodeMeetSwapCheck: node1 is NULL");
  MAssert(node2 != NULL, "KheNodeMeetSwapCheck: node2 is NULL");

  /* the nodes must be distinct */
  if( node1 == node2 )
    return false;

  /* the nodes must have the same durations */
  if( KheNodeDuration(node1) != KheNodeDuration(node2) )
    return false;

  /* the nodes must have the same number of meets */
  if( MArraySize(node1->meets) != MArraySize(node2->meets) )
    return false;

  /* corresponding meets must have equal durations and be swappable */
  for( i = 0;  i < MArraySize(node1->meets);  i++ )
  {
    meet1 = MArrayGet(node1->meets, i);
    meet2 = MArrayGet(node2->meets, i);
    if( KheMeetDuration(meet1) != KheMeetDuration(meet2) ||
	!KheMeetSwapCheck(meet1, meet2) )
      return false;
  }

  /* all in order */
  return true;
}


/*****************************************************************************/
/*                                                                           */
/*  bool KheNodeMeetSwap(KHE_NODE node1, KHE_NODE node2)                     */
/*                                                                           */
/*  Swap the assignments of the meets of node1 and node2, if possible.       */
/*                                                                           */
/*****************************************************************************/

bool KheNodeMeetSwap(KHE_NODE node1, KHE_NODE node2)
{
  KHE_MEET meet1, meet2;  int i;
  if( DEBUG8 )
    fprintf(stderr, "[ KheNodeMeetSwap(node1, node2)\n");

  /* make sure safe to proceed */
  if( !KheNodeMeetSwapCheck(node1, node2) )
  {
    if( DEBUG8 )
      fprintf(stderr, "] KheNodeMeetSwap returning false\n");
    return false;
  }

  /* do the swaps and return true */
  for( i = 0;  i < MArraySize(node1->meets);  i++ )
  {
    meet1 = MArrayGet(node1->meets, i);
    meet2 = MArrayGet(node2->meets, i);
    if( !KheMeetSwap(meet1, meet2) )
      MAssert(false, "KheNodeMeetSwap internal error");
  }
  if( DEBUG8 )
    fprintf(stderr, "] KheNodeMeetSwap returning true\n");
  return true;
}


/*****************************************************************************/
/*                                                                           */
/*  Submodule "similarity and regularity"                                    */
/*                                                                           */
/*****************************************************************************/

/*****************************************************************************/
/*                                                                           */
/*  bool KheNodeSimilar(KHE_NODE node1, KHE_NODE node2)                      */
/*                                                                           */
/*  Return true if node1 and node2 are similar.                              */
/*                                                                           */
/*****************************************************************************/

/* ***
#define DO_DEBUG5 ((KheNodeIndex(node1)==22 && KheNodeIndex(node2)==19) \
  || (KheNodeIndex(node1)==19 && KheNodeIndex(node2)==22))
*** */

#define cleanup_and_return(cond)					     \
  return (MArrayFree(events1), MArrayFree(events2), MArrayFree(durations1),  \
    MArrayFree(durations2), MArrayFree(domains1), MArrayFree(domains2), cond)

bool KheNodeSimilar(KHE_NODE node1, KHE_NODE node2)
{
  ARRAY_KHE_EVENT events1, events2;  KHE_EVENT e1, e2, tmp;
  ARRAY_INT durations1, durations2;
  ARRAY_KHE_RESOURCE_GROUP domains1, domains2;
  KHE_MEET meet1, meet2;  int i1, i2, pos;
  if( DEBUG5 )
    fprintf(stderr, "[ KheNodeSimilar(Node %d, Node %d)\n",
      KheNodeIndex(node1), KheNodeIndex(node2));

  /* identical nodes are similar */
  if( node1 == node2 )
  {
    if( DEBUG5 )
      fprintf(stderr, "] KheNodeSimilar returning true (same node)\n");
    return true;
  }

  /* make sure the durations and number of meets match */
  if( node1->duration != node2->duration )
  {
    if( DEBUG5 )
      fprintf(stderr, "] KheNodeSimilar returning false (durations differ)\n");
    return false;
  }
  if( MArraySize(node1->meets) != MArraySize(node2->meets) )
  {
    if( DEBUG5 )
      fprintf(stderr, "] KheNodeSimilar returning false (meet counts)\n");
    return false;
  }

  /* make sure every meet has an instance event */
  MArrayForEach(node1->meets, &meet1, &i1)
    if( KheMeetEvent(meet1) == NULL )
    {
      if( DEBUG5 )
	fprintf(stderr, "] KheNodeSimilar returning false (meet1 no event)\n");
      return false;
    }
  MArrayForEach(node2->meets, &meet2, &i2)
    if( KheMeetEvent(meet2) == NULL )
    {
      if( DEBUG5 )
	fprintf(stderr, "] KheNodeSimilar returning false (meet2 no event)\n");
      return false;
    }

  /* find the instance events of each node, and their durations */
  MArrayInit(events1);
  MArrayInit(events2);
  MArrayInit(durations1);
  MArrayInit(durations2);
  MArrayInit(domains1);
  MArrayInit(domains2);
  MArrayForEach(node1->meets, &meet1, &i1)
  {
    e1 = KheMeetEvent(meet1);
    if( !MArrayContains(events1, e1, &pos) )
    {
      pos = MArraySize(events1);
      MArrayAddLast(events1, e1);
      MArrayAddLast(durations1, 0);
    }
    MArrayPut(durations1, pos,
      MArrayGet(durations1, pos) + KheMeetDuration(meet1));
  }
  MArrayForEach(node2->meets, &meet2, &i2)
  {
    e2 = KheMeetEvent(meet2);
    if( !MArrayContains(events2, e2, &pos) )
    {
      pos = MArraySize(events2);
      MArrayAddLast(events2, e2);
      MArrayAddLast(durations2, 0);
    }
    MArrayPut(durations2, pos,
      MArrayGet(durations2, pos) + KheMeetDuration(meet2));
  }

  /* make sure that every event that's here at all is here completely */
  MArrayForEach(events1, &e1, &i1)
    if( KheEventDuration(e1) != MArrayGet(durations1, i1) )
    {
      if( DEBUG5 )
	fprintf(stderr, "] KheNodeSimilar returning false (incomplete e1)\n");
      cleanup_and_return(false);
    }
  MArrayForEach(events2, &e2, &i2)
    if( KheEventDuration(e2) != MArrayGet(durations2, i2) )
    {
      if( DEBUG5 )
	fprintf(stderr, "] KheNodeSimilar returning false (incomplete e2)\n");
      cleanup_and_return(false);
    }

  /* reorder events2 so that similar events are at equal indexes */
  if( MArraySize(events1) != MArraySize(events2) )
  {
    if( DEBUG5 )
      fprintf(stderr, "] KheNodeSimilar returning false (event counts)\n");
    cleanup_and_return(false);
  }
  MArrayForEach(events1, &e1, &i1)
  {
    for( i2 = i1;  i2 < MArraySize(events2);  i2++ )
    {
      e2 = MArrayGet(events2, i2);
      if( KheEventPartitionSimilar(e1, e2, &domains1, &domains2) )
	break;
    }
    if( i2 >= MArraySize(events2) )
    {
      if( DEBUG5 )
	fprintf(stderr, "] KheNodeSimilar returning false (e1 not similar)\n");
      cleanup_and_return(false);
    }
    MArraySwap(events2, i1, i2, tmp);
  }
  if( DEBUG2 || DEBUG5 )
  {
    fprintf(stderr, "[ KheNodeSimilar(Node %d, Node %d) returning true:\n",
      KheNodeIndex(node1), KheNodeIndex(node2));
    for( i1 = 0;  i1 < MArraySize(events1);  i1++ )
    {
      e1 = MArrayGet(events1, i1);
      e2 = MArrayGet(events2, i1);
      fprintf(stderr, "    %-30s  <-->  %s\n",
	KheEventId(e1) != NULL ? KheEventId(e1) : "-",
	KheEventId(e2) != NULL ? KheEventId(e2) : "-");
    }
    fprintf(stderr, "]\n");
  }
  cleanup_and_return(true);
}


/*****************************************************************************/
/*                                                                           */
/*  bool KheMeetRegular(KHE_MEET meet1, KHE_MEET meet2)                      */
/*                                                                           */
/*  Return true if these two meets are regular; that is, if they             */
/*  have the same duration and the same time domain.                         */
/*                                                                           */
/*****************************************************************************/

static bool KheMeetRegular(KHE_MEET meet1, KHE_MEET meet2)
{
  KHE_TIME_GROUP tg1, tg2;
  if( KheMeetDuration(meet1) != KheMeetDuration(meet2) )
    return false;
  tg1 = KheMeetDomain(meet1);
  tg2 = KheMeetDomain(meet2);
  return tg1 != NULL && tg2 != NULL && KheTimeGroupEqual(tg1, tg2);
}


/*****************************************************************************/
/*                                                                           */
/*  bool KheNodeRegular(KHE_NODE node1, KHE_NODE node2, int *regular_count)  */
/*                                                                           */
/*  Return true if node1 and node2 are regular, and in any case reorder      */
/*  the meets of both nodes so that the first *regular_count soln events     */
/*  have the same durations and current time domains.                        */
/*                                                                           */
/*****************************************************************************/

bool KheNodeRegular(KHE_NODE node1, KHE_NODE node2, int *regular_count)
{
  int i, j;  KHE_MEET meet1, meet2, tmp;  bool res;
  *regular_count = MArraySize(node1->meets);
  if( node1 == node2 )
    return true;
  for( i = 0;  i < *regular_count;  i++ )
  {
    meet1 = MArrayGet(node1->meets, i);
    for( j = i;  j < MArraySize(node2->meets);  j++ )
    {
      meet2 = MArrayGet(node2->meets, j);
      if( KheMeetRegular(meet1, meet2) )
	break;
    }
    if( j < MArraySize(node2->meets) )
    {
      /* meet1 and meet2 are regular */
      MArraySwap(node2->meets, i, j, tmp);
    }
    else
    {
      /* meet1 is not regular to any node of node2 */
      (*regular_count)--;
      MArraySwap(node1->meets, *regular_count, i, tmp);
      i--;
    }
  }
  res = MArraySize(node1->meets) == MArraySize(node2->meets) &&
    MArraySize(node1->meets) == *regular_count;
  return res;
}


/*****************************************************************************/
/*                                                                           */
/*  bool KheNodeRegularAssignCheck(KHE_NODE node, KHE_NODE sibling_node)     */
/*                                                                           */
/*  Check whether it is possible to assign the meets of node, using the      */
/*  assignments of sibling_node as a template.                               */
/*                                                                           */
/*****************************************************************************/

bool KheNodeRegularAssignCheck(KHE_NODE node, KHE_NODE sibling_node)
{
  int junk, i;  KHE_MEET meet, sibling_meet;

  /* the nodes must have the same duration */
  if( KheNodeDuration(node) != KheNodeDuration(sibling_node) )
  {
    if( DEBUG6 )
      fprintf(stderr, "    KheNodeRegularAssignCheck(Node %d, Node %d) durn\n",
	KheNodeIndex(node), KheNodeIndex(sibling_node));
    return false;
  }

  /* every meet of sibling_node must be assigned */
  if( KheNodeAssignedDuration(sibling_node) != KheNodeDuration(sibling_node) )
  {
    if( DEBUG6 )
      fprintf(stderr,
	"    KheNodeRegularAssignCheck(Node %d, Node %d) ad %d != d %d",
	KheNodeIndex(node), KheNodeIndex(sibling_node),
	KheNodeAssignedDuration(sibling_node), KheNodeDuration(sibling_node));
    return false;
  }

  /* the nodes must be regular */
  if( !KheNodeRegular(node, sibling_node, &junk) )
  {
    if( DEBUG6 )
      fprintf(stderr, "    KheNodeRegularAssignCheck(Node %d, Node %d) regl\n",
	KheNodeIndex(node), KheNodeIndex(sibling_node));
    return false;
  }

  /* the corresponding assignments must be possible or already done */
  for( i = 0;  i < MArraySize(node->meets);  i++ )
  {
    meet = MArrayGet(node->meets, i);
    sibling_meet = MArrayGet(sibling_node->meets, i);
    if( KheMeetAsst(meet) != NULL )
    {
      if( KheMeetAsst(sibling_meet) != KheMeetAsst(meet) ||
	  KheMeetAsstOffset(sibling_meet) != KheMeetAsstOffset(meet) )
      {
	if( DEBUG6 )
	  fprintf(stderr, "    KheNodeRegularAssignCheck(Node %d, Node %d) a\n",
	    KheNodeIndex(node), KheNodeIndex(sibling_node));
	return false;
      }
    }
    else
    {
      if( !KheMeetAssignCheck(meet, KheMeetAsst(sibling_meet),
	    KheMeetAsstOffset(sibling_meet)) )
      {
	if( DEBUG6 )
	  fprintf(stderr, "    KheNodeRegularAssignCheck(Node %d, Node %d) c\n",
	    KheNodeIndex(node), KheNodeIndex(sibling_node));
	return false;
      }
    }
  }
  if( DEBUG6 )
    fprintf(stderr, "    KheNodeRegularAssignCheck(Node %d, Node %d) OK\n",
      KheNodeIndex(node), KheNodeIndex(sibling_node));
  return true;
}


/*****************************************************************************/
/*                                                                           */
/*  bool KheNodeRegularAssign(KHE_NODE node, KHE_NODE sibling_node)          */
/*                                                                           */
/*  If possible, assign the meets of node, using the assignments of          */
/*  sibling_node as a template.                                              */
/*                                                                           */
/*****************************************************************************/

bool KheNodeRegularAssign(KHE_NODE node, KHE_NODE sibling_node)
{
  int i;  KHE_MEET meet, sibling_meet;
  if( !KheNodeRegularAssignCheck(node, sibling_node) )
    return false;
  if( DEBUG7 )
    fprintf(stderr, "    [ KheNodeRegularAssign(Node %d, Node %d)\n",
      node->index_in_soln, sibling_node->index_in_soln);
  for( i = 0;  i < MArraySize(node->meets);  i++ )
  {
    meet = MArrayGet(node->meets, i);
    sibling_meet = MArrayGet(sibling_node->meets, i);
    if( KheMeetAsst(meet) == NULL )
    {
      if( !KheMeetAssign(meet, KheMeetAsst(sibling_meet),
	    KheMeetAsstOffset(sibling_meet)) )
	MAssert(false, "KheNodeRegularAssign internal error");
    }
    if( DEBUG7 )
    {
      KHE_TIME time = KheMeetAsstTime(sibling_meet);
      fprintf(stderr, "      assigning %s to ",
	time == NULL ? "?" : KheTimeId(time) == NULL ? "-" : KheTimeId(time));
      KheMeetDebug(meet, 1, 0, stderr);
    }
  }
  if( DEBUG7 )
    fprintf(stderr, "    ] KheNodeRegularAssign\n");
  return true;
}


/*****************************************************************************/
/*                                                                           */
/*  void KheNodeUnAssign(KHE_NODE node)                                      */
/*                                                                           */
/*  Unassign the meets of node.                                              */
/*                                                                           */
/*****************************************************************************/

void KheNodeUnAssign(KHE_NODE node)
{
  int i;  KHE_MEET meet;
  MArrayForEach(node->meets, &meet, &i)
    if( KheMeetAsst(meet) != NULL )
      KheMeetUnAssign(meet);
}


/*****************************************************************************/
/*                                                                           */
/*  int KheNodeResourceDuration(KHE_NODE node, KHE_RESOURCE r)               */
/*                                                                           */
/*  Return the total duration of meets lying in node and its descendants     */
/*  that contain a preassignment of r.                                       */
/*                                                                           */
/*****************************************************************************/

int KheNodeResourceDuration(KHE_NODE node, KHE_RESOURCE r)
{
  int i, res;  KHE_MEET meet;  KHE_NODE child_node;  KHE_TASK task;
  res = 0;
  MArrayForEach(node->meets, &meet, &i)
    if( KheMeetContainsResourcePreassignment(meet, r, false, &task) )
      res += KheMeetDuration(meet);
  MArrayForEach(node->child_nodes, &child_node, &i)
    res += KheNodeResourceDuration(child_node, r);
  return res;
}


/*****************************************************************************/
/*                                                                           */
/*  Submodule "zones"                                                        */
/*                                                                           */
/*****************************************************************************/

/*****************************************************************************/
/*                                                                           */
/*  void KheNodeAddZone(KHE_NODE node, KHE_ZONE zone, int *index)            */
/*                                                                           */
/*  Add zone to node, setting *index to its position in node.                */
/*                                                                           */
/*****************************************************************************/

void KheNodeAddZone(KHE_NODE node, KHE_ZONE zone, int *index)
{
  *index = MArraySize(node->zones);
  MArrayAddLast(node->zones, zone);
}


/*****************************************************************************/
/*                                                                           */
/*  void KheNodeDeleteZone(KHE_NODE node, KHE_ZONE zone, int index)          */
/*                                                                           */
/*  Delete zone from node.  It lies at position index.                       */
/*                                                                           */
/*****************************************************************************/

void KheNodeDeleteZone(KHE_NODE node, KHE_ZONE zone, int index)
{
  KHE_ZONE tmp;
  MAssert(MArrayGet(node->zones, index) == zone,
    "KheNodeDeleteZone internal error");
  tmp = MArrayRemoveLast(node->zones);
  if( tmp != zone )
  {
    MArrayPut(node->zones, index, tmp);
    KheZoneSetIndex(tmp, index);
  }
}


/*****************************************************************************/
/*                                                                           */
/*  int KheNodeZoneCount(KHE_NODE node)                                      */
/*                                                                           */
/*  Return the number of zones of node.                                      */
/*                                                                           */
/*****************************************************************************/

int KheNodeZoneCount(KHE_NODE node)
{
  return MArraySize(node->zones);
}


/*****************************************************************************/
/*                                                                           */
/*  KHE_ZONE KheNodeZone(KHE_NODE node, int i)                               */
/*                                                                           */
/*  Return the i'th zone of node.                                            */
/*                                                                           */
/*****************************************************************************/

KHE_ZONE KheNodeZone(KHE_NODE node, int i)
{
  return MArrayGet(node->zones, i);
}


/*****************************************************************************/
/*                                                                           */
/*  void KheNodeDeleteZones(KHE_NODE node)                                   */
/*                                                                           */
/*  Delete the zones of node.                                                */
/*                                                                           */
/*****************************************************************************/

void KheNodeDeleteZones(KHE_NODE node)
{
  while( KheNodeZoneCount(node) > 0 )
    KheZoneDelete(KheNodeZone(node, 0));
}


/*****************************************************************************/
/*                                                                           */
/*  Submodule "visit numbers"                                                */
/*                                                                           */
/*****************************************************************************/

/*****************************************************************************/
/*                                                                           */
/*  void KheNodeSetVisitNum(KHE_NODE n, int num)                             */
/*                                                                           */
/*  Set the visit number of n.                                               */
/*                                                                           */
/*****************************************************************************/

void KheNodeSetVisitNum(KHE_NODE n, int num)
{
  n->visit_num = num;
}


/*****************************************************************************/
/*                                                                           */
/*  int KheNodeVisitNum(KHE_NODE n)                                          */
/*                                                                           */
/*  Return the visit number of n.                                            */
/*                                                                           */
/*****************************************************************************/

int KheNodeVisitNum(KHE_NODE n)
{
  return n->visit_num;
}


/*****************************************************************************/
/*                                                                           */
/*  bool KheNodeVisited(KHE_NODE n, int slack)                               */
/*                                                                           */
/*  Return true if n has been visited recently.                              */
/*                                                                           */
/*****************************************************************************/

bool KheNodeVisited(KHE_NODE n, int slack)
{
  return KheSolnVisitNum(n->soln) - n->visit_num <= slack;
}


/*****************************************************************************/
/*                                                                           */
/*  void KheNodeVisit(KHE_NODE n)                                            */
/*                                                                           */
/*  Visit n.                                                                 */
/*                                                                           */
/*****************************************************************************/

void KheNodeVisit(KHE_NODE n)
{
  n->visit_num = KheSolnVisitNum(n->soln);
}


/*****************************************************************************/
/*                                                                           */
/*  void KheNodeUnVisit(KHE_NODE n)                                          */
/*                                                                           */
/*  Unvisit n.                                                               */
/*                                                                           */
/*****************************************************************************/

void KheNodeUnVisit(KHE_NODE n)
{
  n->visit_num = KheSolnVisitNum(n->soln) - 1;
}


/*****************************************************************************/
/*                                                                           */
/*  Submodule "debug"                                                        */
/*                                                                           */
/*****************************************************************************/

/*****************************************************************************/
/*                                                                           */
/*  KHE_MEET KheMeetNamedMeet(KHE_MEET meet)                                 */
/*                                                                           */
/*  Return a named meet of maximum duration among meet and the meets         */
/*  assigned to it, directly or indirectly.                                  */
/*                                                                           */
/*****************************************************************************/

static KHE_MEET KheMeetNamedMeet(KHE_MEET meet)
{
  KHE_MEET res, child_meet;  int i;

  /* try meet itself */
  if( KheMeetEvent(meet) != NULL && KheEventId(KheMeetEvent(meet)) != NULL )
    return meet;

  /* try the meets assigned to meet, directly or indirectly */
  res = NULL;
  for( i = 0;  i < KheMeetAssignedToCount(meet);  i++ )
  {
    child_meet = KheMeetNamedMeet(KheMeetAssignedTo(meet, i));
    if( child_meet != NULL )
    {
      if( res == NULL || KheMeetDuration(child_meet) > KheMeetDuration(res) )
	res = child_meet;
    }
  }
  return res;
}

/*****************************************************************************/
/*                                                                           */
/*  KHE_MEET KheNamedMeet(KHE_NODE node)                                     */
/*                                                                           */
/*  Return a named meet of maximum duration among the meets of node, or      */
/*  NULL if none.                                                            */
/*                                                                           */
/*****************************************************************************/

static KHE_MEET KheNodeNamedMeet(KHE_NODE node)
{
  KHE_MEET res, child_meet;  int i;
  res = NULL;
  for( i = 0;  i < KheNodeMeetCount(node);  i++ )
  {
    child_meet = KheMeetNamedMeet(KheNodeMeet(node, i));
    if( child_meet != NULL )
    {
      if( res == NULL || KheMeetDuration(child_meet) > KheMeetDuration(res) )
	res = child_meet;
    }
  }
  return res;
}


/*****************************************************************************/
/*                                                                           */
/*  void KheNodeDebug(KHE_NODE node, int indent, FILE *fp)                   */
/*                                                                           */
/*  Debug print of node onto fp with the given indent.                       */
/*                                                                           */
/*****************************************************************************/

void KheNodeDebug(KHE_NODE node, int verbosity, int indent, FILE *fp)
{
  KHE_MEET meet;  int i;  KHE_NODE child_node;  KHE_ZONE zone;
  if( verbosity >= 2 && indent >= 0 )
  {
    fprintf(fp, "%*s[ Node %d (duration %d):\n", indent, "",
      node->index_in_soln, node->duration);
    if( verbosity >= 3 )
      MArrayForEach(node->zones, &zone, &i)
	KheZoneDebug(zone, verbosity, indent + 2, fp);
    MArrayForEach(node->meets, &meet, &i)
      KheMeetDebug(meet, verbosity, indent + 2, fp);
    if( verbosity >= 4 )
      MArrayForEach(node->child_nodes, &child_node, &i)
	KheNodeDebug(child_node, verbosity, indent + 2, fp);
    fprintf(fp, "%*s]\n", indent, "");
  }
  else
  {
    if( indent >= 0 )
      fprintf(fp, "%*s", indent, "");
    fprintf(fp, "Node_%d", node->index_in_soln);
    meet = KheNodeNamedMeet(node);
    if( meet != NULL )
    {
      fprintf(fp, "_");
      KheMeetDebug(meet, 1, -1, fp);
    }
    if( indent >= 0 )
      fprintf(fp, "\n");
  }
}


/*****************************************************************************/
/*                                                                           */
/*  Submodule "timetable printing"                                           */
/*                                                                           */
/*****************************************************************************/

/*****************************************************************************/
/*                                                                           */
/*  KHE_CELL - one cell in the timetable (may span)                          */
/*                                                                           */
/*****************************************************************************/

typedef struct khe_cell_rec {
  KHE_MEET		target_meet;		/* lies under meet           */
  int			target_offset;          /* at this offset            */
  int			span_count;		/* columns spanned           */
  ARRAY_KHE_MEET	meets;			/* the meets in this entry   */
} *KHE_CELL;

typedef MARRAY(KHE_CELL) ARRAY_KHE_CELL;


/*****************************************************************************/
/*                                                                           */
/*  KHE_ROW  - one row of cells                                              */
/*                                                                           */
/*****************************************************************************/

typedef struct khe_row_rec {
  KHE_LAYER		layer;			/* the layer represented     */
  int			width;			/* the initial no. of cells  */
  ARRAY_KHE_CELL	cells;			/* the cells                 */
} *KHE_ROW;

typedef MARRAY(KHE_ROW) ARRAY_KHE_ROW;


/*****************************************************************************/
/*                                                                           */
/*  KHE_TABLE - the table                                                    */
/*                                                                           */
/*****************************************************************************/

typedef struct khe_table_rec {
  KHE_NODE		node;			/* the node represented      */
  KHE_ROW		header_row;		/* the header row            */
  ARRAY_KHE_ROW		rows;			/* the rows                  */
} *KHE_TABLE;


/*****************************************************************************/
/*                                                                           */
/*  void KhePrint(char *str, bool in_cell, int span_count,                   */
/*    int cell_width, FILE *fp)                                              */
/*                                                                           */
/*  Print str onto fp, with a margin, taking care not to overrun.            */
/*                                                                           */
/*****************************************************************************/

static void KhePrint(char *str, bool in_cell, int span_count,
  int cell_width, FILE *fp)
{
  char buff[200];  int width;
  width = span_count * cell_width - 3;
  snprintf(buff, width, "%s", str);
  fprintf(fp, "%c %-*s ", in_cell ? '|' : ' ', width, buff);
}


/*****************************************************************************/
/*                                                                           */
/*  void KhePrintRule(bool major, int cell_width, FILE *fp)                  */
/*                                                                           */
/*  Print a rule of the given cell_width onto fp.                            */
/*                                                                           */
/*****************************************************************************/

static void KhePrintRule(bool major, int cell_width, FILE *fp)
{
  int i;
  fprintf(fp, "+");
  for( i = 0;  i < cell_width - 1;  i++ )
    fprintf(fp, major ? "=" : "-");
}


/*****************************************************************************/
/*                                                                           */
/*  void KhePrintRuleLine(int cells, bool major, int cell_width,             */
/*    int indent, FILE *fp)                                                  */
/*                                                                           */
/*  Print a full-width rule, for this many cells of this width, onto fp      */
/*  with the given indent.                                                   */
/*                                                                           */
/*****************************************************************************/

static void KhePrintRuleLine(int cells, bool major, int cell_width,
  int indent, FILE *fp)
{
  int i;
  fprintf(fp, "%*s", indent, "");
  for( i = 0;  i < cells;  i++ )
    KhePrintRule(major, cell_width, fp);
  fprintf(fp, "+\n");
}


/*****************************************************************************/
/*                                                                           */
/*  KHE_CELL KheCellMake(KHE_MEET target_meet, int target_offset)            */
/*                                                                           */
/*  Make a new cell with these attributes.                                   */
/*                                                                           */
/*****************************************************************************/

static KHE_CELL KheCellMake(KHE_MEET target_meet, int target_offset)
{
  KHE_CELL res;
  MMake(res);
  res->target_meet = target_meet;
  res->target_offset = target_offset;
  res->span_count = 1;
  MArrayInit(res->meets);
  return res;
}


/*****************************************************************************/
/*                                                                           */
/*  void KheCellDelete(KHE_CELL cell)                                        */
/*                                                                           */
/*  Delete cell.                                                             */
/*                                                                           */
/*****************************************************************************/

static void KheCellDelete(KHE_CELL cell)
{
  MArrayFree(cell->meets);
  MFree(cell);
}


/*****************************************************************************/
/*                                                                           */
/*  void KheCellCheckAndAcceptMeet(KHE_CELL cell, KHE_MEET meet)             */
/*                                                                           */
/*  Check whether cell can accept meet, and add it if so.                    */
/*                                                                           */
/*****************************************************************************/

static void KheCellCheckAndAcceptMeet(KHE_CELL cell, KHE_MEET meet)
{
  if( KheMeetAsst(meet) == cell->target_meet &&
      KheMeetAsstOffset(meet) <= cell->target_offset &&
      KheMeetAsstOffset(meet) + KheMeetDuration(meet) > cell->target_offset )
    MArrayAddLast(cell->meets, meet);
}


/*****************************************************************************/
/*                                                                           */
/*  bool KheCellMergeable(KHE_CELL cell1, KHE_CELL cell2)                    */
/*                                                                           */
/*  Return true if cell1 and cell2 are mergeable, because they contain       */
/*  the same meets.                                                          */
/*                                                                           */
/*****************************************************************************/

static bool KheCellMergeable(KHE_CELL cell1, KHE_CELL cell2)
{
  int i;
  if( MArraySize(cell1->meets) != MArraySize(cell2->meets) )
    return false;
  for( i = 0;  i < MArraySize(cell1->meets);  i++ )
    if( MArrayGet(cell1->meets, i) != MArrayGet(cell2->meets, i) )
      return false;
  return true;
}


/*****************************************************************************/
/*                                                                           */
/*  void KheCellMerge(KHE_CELL cell1, KHE_CELL cell2)                        */
/*                                                                           */
/*  Merge cell2 into cell1 and delete cell2.                                 */
/*                                                                           */
/*****************************************************************************/

static void KheCellMerge(KHE_CELL cell1, KHE_CELL cell2)
{
  cell1->span_count += cell2->span_count;
  KheCellDelete(cell2);
}


/*****************************************************************************/
/*                                                                           */
/*  void KheCellPrint(KHE_CELL cell, int line, int cell_width, FILE *fp)     */
/*                                                                           */
/*  Print the line'th line of cell.  This will be a meet name if there       */
/*  is one, or an empty space otherwise.                                     */
/*                                                                           */
/*****************************************************************************/

static void KheCellPrint(KHE_CELL cell, int line, int cell_width, FILE *fp)
{
  KHE_MEET meet;  char *str;  char buff[20];
  if( line < MArraySize(cell->meets) )
  {
    meet = MArrayGet(cell->meets, line);
    if( KheMeetEvent(meet) == NULL || KheEventId(KheMeetEvent(meet)) == NULL )
    {
      sprintf(buff, "#%d#", KheMeetIndex(meet));
      str = buff;
    }
    else
      str = KheEventId(KheMeetEvent(meet));
  }
  else
    str = "";
  KhePrint(str, true, cell->span_count, cell_width, fp);
}


/*****************************************************************************/
/*                                                                           */
/*  KHE_ROW KheRowMake(KHE_LAYER layer)                                      */
/*                                                                           */
/*  Make a new row with these attributes.                                    */
/*                                                                           */
/*****************************************************************************/

static KHE_ROW KheRowMake(KHE_LAYER layer)
{
  KHE_ROW res;
  MMake(res);
  res->layer = layer;
  res->width = 0;
  MArrayInit(res->cells);
  return res;
}


/*****************************************************************************/
/*                                                                           */
/*  void KheRowDelete(KHE_ROW row)                                           */
/*                                                                           */
/*  Delete row, including deleting its cells.                                */
/*                                                                           */
/*****************************************************************************/

static void KheRowDelete(KHE_ROW row)
{
  while( MArraySize(row->cells) > 0 )
    KheCellDelete(MArrayRemoveLast(row->cells));
  MArrayFree(row->cells);
  MFree(row);
}


/*****************************************************************************/
/*                                                                           */
/*  void KheRowAddMeet(KHE_ROW row, KHE_MEET meet)                           */
/*                                                                           */
/*  Add meet to the appropriate cells of row.                                */
/*                                                                           */
/*****************************************************************************/

static void KheRowAddMeet(KHE_ROW row, KHE_MEET meet)
{
  KHE_CELL cell;  int i;
  if( KheMeetAsst(meet) != NULL )
    MArrayForEach(row->cells, &cell, &i)
      KheCellCheckAndAcceptMeet(cell, meet);
}


/*****************************************************************************/
/*                                                                           */
/*  KHE_ROW KheRowBuild(KHE_LAYER layer)                                     */
/*                                                                           */
/*  Build a new row for layer.                                               */
/*                                                                           */
/*****************************************************************************/

static KHE_ROW KheRowBuild(KHE_LAYER layer)
{
  KHE_ROW res;  KHE_NODE parent_node, child_node;
  KHE_MEET meet;  int offset, i, j;

  /* build the row with its cells, initially empty */
  res = KheRowMake(layer);
  parent_node = KheLayerParentNode(layer);
  for( i = 0;  i < KheNodeMeetCount(parent_node);  i++ )
  {
    meet = KheNodeMeet(parent_node, i);
    for( offset = 0;  offset < KheMeetDuration(meet);  offset++ )
      MArrayAddLast(res->cells, KheCellMake(meet, offset));
  }
  res->width = MArraySize(res->cells);

  /* add the meets of the layer's child nodes to the row */
  for( i = 0;  i < KheLayerChildNodeCount(layer);  i++ )
  {
    child_node = KheLayerChildNode(layer, i);
    for( j = 0;  j < KheNodeMeetCount(child_node);  j++ )
      KheRowAddMeet(res, KheNodeMeet(child_node, j));
  }
  return res;
}


/*****************************************************************************/
/*                                                                           */
/*  KHE_ROW KheHeaderRowBuild(KHE_NODE parent_node)                          */
/*                                                                           */
/*  Build a header row holding the meets of parent_node.                     */
/*                                                                           */
/*****************************************************************************/

static KHE_ROW KheHeaderRowBuild(KHE_NODE parent_node)
{
  KHE_ROW res;  KHE_MEET meet;  int i;  KHE_CELL cell;

  res = KheRowMake(NULL);
  for( i = 0;  i < KheNodeMeetCount(parent_node);  i++ )
  {
    meet = KheNodeMeet(parent_node, i);
    cell = KheCellMake(NULL, 0);
    cell->span_count = KheMeetDuration(meet);
    MArrayAddLast(cell->meets, meet);
    MArrayAddLast(res->cells, cell);
  }
  res->width = KheNodeDuration(parent_node);
  return res;
}


/*****************************************************************************/
/*                                                                           */
/*  void KheRowMergeCells(KHE_ROW row)                                       */
/*                                                                           */
/*  Merge identical cells in row.                                            */
/*                                                                           */
/*****************************************************************************/

static void KheRowMergeCells(KHE_ROW row)
{
  KHE_CELL cell, prev_cell;  int i;
  prev_cell = NULL;
  MArrayForEach(row->cells, &cell, &i)
  {
    if( prev_cell != NULL && KheCellMergeable(prev_cell, cell) )
    {
      MArrayRemove(row->cells, i);
      i--;
      KheCellMerge(prev_cell, cell);
    }
    else
      prev_cell = cell;
  }
}


/*****************************************************************************/
/*                                                                           */
/*  void KheRowPrint(KHE_ROW row, bool major, int cell_width, int indent,    */
/*    FILE *fp)                                                              */
/*                                                                           */
/*  Print row onto fp with the given indent.                                 */
/*                                                                           */
/*****************************************************************************/

static void KheRowPrint(KHE_ROW row, bool major, int cell_width,
  int indent, FILE *fp)
{
  int max_lines, i, line;  KHE_CELL cell;  char *str;  KHE_RESOURCE r;

  /* find the maximum number of lines in any cell of the row */
  max_lines = 0;
  MArrayForEach(row->cells, &cell, &i)
    if( MArraySize(cell->meets) > max_lines )
      max_lines = MArraySize(cell->meets);

  /* print those lines followed by a rule */
  if( max_lines > 0 )
  {
    for( line = 0;  line < max_lines;  line++ )
    {
      /* print the first cell, which is the layer's resources if first */
      fprintf(fp, "%*s", indent, "");
      if( line == 0 && KheLayerResourceCount(row->layer) > 0 )
      {
	r = KheLayerResource(row->layer, 0);
	str = KheResourceId(r) == NULL ? "-" : KheResourceId(r);
      }
      else
	str = "";
      KhePrint(str, true, 1, cell_width, fp);

      /* print subsequent cells, or rather one line of each */
      MArrayForEach(row->cells, &cell, &i)
        KheCellPrint(cell, line, cell_width, fp);
      fprintf(fp, "|\n");
    }
    KhePrintRuleLine(row->width + 1, major, cell_width, indent, fp);
  }
}


/*****************************************************************************/
/*                                                                           */
/*  void KheHeaderRowPrint(KHE_ROW row, KHE_NODE node, int cell_width,       */
/*    int indent, FILE *fp)                                                  */
/*                                                                           */
/*  Print row (a header row) onto fp with the given indent.                  */
/*                                                                           */
/*****************************************************************************/

static void KheHeaderRowPrint(KHE_ROW row, KHE_NODE node, int cell_width,
  int indent, FILE *fp)
{
  int i;  KHE_CELL cell;  char buff[20];

  /* print a rule above the row */
  KhePrintRuleLine(row->width + 1, true, cell_width, indent, fp);

  /* print the first cell, which is node's index */
  fprintf(fp, "%*s", indent, "");
  sprintf(buff, "Node %d", KheNodeIndex(node));
  KhePrint(buff, true, 1, cell_width, fp);

  /* print subsequent cells */
  MArrayForEach(row->cells, &cell, &i)
    KheCellPrint(cell, 0, cell_width, fp);
  fprintf(fp, "|\n");

  /* print a rule below the row */
  KhePrintRuleLine(row->width + 1, true, cell_width, indent, fp);
}


/*****************************************************************************/
/*                                                                           */
/*  KHE_TABLE KheTableMake(KHE_NODE node)                                    */
/*                                                                           */
/*  Make a new, empty table for node.                                        */
/*                                                                           */
/*****************************************************************************/

static KHE_TABLE KheTableMake(KHE_NODE node)
{
  KHE_TABLE res;
  MMake(res);
  res->node = node;
  res->header_row = NULL;
  MArrayInit(res->rows);
  return res;
}


/*****************************************************************************/
/*                                                                           */
/*  void KheTableDelete(KHE_TABLE table)                                     */
/*                                                                           */
/*  Delete table, including deleting its rows.                               */
/*                                                                           */
/*****************************************************************************/

static void KheTableDelete(KHE_TABLE table)
{
  while( MArraySize(table->rows) > 0 )
    KheRowDelete(MArrayRemoveLast(table->rows));
  MArrayFree(table->rows);
  MFree(table);
}


/*****************************************************************************/
/*                                                                           */
/*  KHE_TABLE KheTableBuild(KHE_NODE node)                                   */
/*                                                                           */
/*  Assuming that node has layers, build a table for it.                     */
/*                                                                           */
/*****************************************************************************/

static KHE_TABLE KheTableBuild(KHE_NODE node)
{
  KHE_TABLE res;  int i;
  res = KheTableMake(node);
  res->header_row = KheHeaderRowBuild(node);
  for( i = 0;  i < KheNodeChildLayerCount(node);  i++ )
    MArrayAddLast(res->rows, KheRowBuild(KheNodeChildLayer(node, i)));
  return res;
}


/*****************************************************************************/
/*                                                                           */
/*  void KheTableMergeCells(KHE_TABLE table)                                 */
/*                                                                           */
/*  Merge adjacent identical cells in table.                                 */
/*                                                                           */
/*****************************************************************************/

static void KheTableMergeCells(KHE_TABLE table)
{
  KHE_ROW row;  int i;
  MArrayForEach(table->rows, &row, &i)
    KheRowMergeCells(row);
}


/*****************************************************************************/
/*                                                                           */
/*  void KheTablePrint(KHE_TABLE table, int cell_width,                      */
/*    int indent, FILE *fp)                                                  */
/*                                                                           */
/*  Print table onto fp with the given indent.                               */
/*                                                                           */
/*****************************************************************************/

static void KheTablePrint(KHE_TABLE table, int cell_width,
  int indent, FILE *fp)
{
  KHE_ROW row;  int i;

  /* print header row */
  KheHeaderRowPrint(table->header_row, table->node, cell_width, indent, fp);

  /* print ordinary rows */
  MArrayForEach(table->rows, &row, &i)
    KheRowPrint(row, i == MArraySize(table->rows) - 1, cell_width, indent, fp);
}


/*****************************************************************************/
/*                                                                           */
/*  void KheNodePrintTimetable(KHE_NODE node, int cell_width,                */
/*    int indent, FILE *fp)                                                  */
/*                                                                           */
/*  Print the timetable of node.                                             */
/*                                                                           */
/*****************************************************************************/

void KheNodePrintTimetable(KHE_NODE node, int cell_width,
  int indent, FILE *fp)
{
  bool make_layers;  KHE_TABLE table;

  /* make layers if not already present */
  make_layers = (KheNodeChildLayerCount(node) == 0);
  if( make_layers )
    KheNodeChildLayersMake(node);

  /* build the table, merge identical cells, print it, and delete it */
  table = KheTableBuild(node);
  KheTableMergeCells(table);
  KheTablePrint(table, cell_width, indent, fp);
  KheTableDelete(table);

  /* remove layers if we added them */
  if( make_layers )
    KheNodeChildLayersDelete(node);
}


/*****************************************************************************/
/*                                                                           */
/*  void KheNodeMatchingDebug(KHE_NODE node, int verbosity,                  */
/*    int indent, FILE *fp)                                                  */
/*                                                                           */
/*  Debug print of that part of the matching at node, with the given         */
/*  verbosity and indent.                                                    */
/*                                                                           */
/*****************************************************************************/

/* ***
void KheNodeMatchingDebug(KHE_NODE node, int verbosity, int indent, FILE *fp)
{
  KHE_MATCHING m;  KHE_MEET meet;  int i;
  m = KheSolnMatching(node->soln);
  if( verbosity >= 2 && indent >= 0 )
  {
    KheMatchingDebugBegin(m, 34, &KheSolnMatchingSupplyNodeShow,
      &KheSolnMatchingDemandNodeShow, verbosity, indent, fp);
    MArrayForEach(node->meets, &meet, &i)
      KheMeetDebugDemandChunks(meet, verbosity, indent, fp);
    KheMatchingDebugEnd(m, 34, &KheSolnMatchingSupplyNodeShow,
      &KheSolnMatchingDemandNodeShow, verbosity, indent, fp);
  }
}
*** */
