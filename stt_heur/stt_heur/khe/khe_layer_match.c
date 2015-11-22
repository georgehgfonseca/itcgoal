
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
/*  FILE:         khe_layer_match.c                                          */
/*  DESCRIPTION:  Layer matchings                                            */
/*                                                                           */
/*****************************************************************************/
#include "khe.h"
#include "m.h"
#include "khe_wmatch.h"
#include "khe_lset.h"
#include <limits.h>

#define CHILDLESS_MULTIPLIER 10

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

typedef struct khe_supply_node_group_rec *KHE_SUPPLY_NODE_GROUP;
typedef MARRAY(KHE_SUPPLY_NODE_GROUP) ARRAY_KHE_SUPPLY_NODE_GROUP;

typedef struct khe_supply_node_rec *KHE_SUPPLY_NODE;
typedef MARRAY(KHE_SUPPLY_NODE) ARRAY_KHE_SUPPLY_NODE;

typedef struct khe_demand_node_group_rec *KHE_DEMAND_NODE_GROUP;
typedef MARRAY(KHE_DEMAND_NODE_GROUP) ARRAY_KHE_DEMAND_NODE_GROUP;

typedef struct khe_demand_node_rec *KHE_DEMAND_NODE;
typedef MARRAY(KHE_DEMAND_NODE) ARRAY_KHE_DEMAND_NODE;

/* ***
typedef struct khe_sublayer_rec *KHE_SUBLAYER;
typedef MARRAY(KHE_SUBLAYER) ARRAY_KHE_SUBLAYER;
*** */

typedef struct khe_even_time_group_rec *KHE_EVEN_TIME_GROUP;
typedef MARRAY(KHE_EVEN_TIME_GROUP) ARRAY_KHE_EVEN_TIME_GROUP;

typedef MARRAY(KHE_NODE) ARRAY_KHE_NODE;
typedef MARRAY(KHE_LAYER) ARRAY_KHE_LAYER;
typedef MARRAY(KHE_ZONE) ARRAY_KHE_ZONE;


/*****************************************************************************/
/*                                                                           */
/*  KHE_SUBLAYER (private) - a subset of the nodes of one layer              */
/*                                                                           */
/*****************************************************************************/

/* ***
struct khe_sublayer_rec {
  KHE_LAYER		layer;			** the layer                 **
  ARRAY_KHE_NODE	nodes;			** some nodes from layer     **
};
*** */


/*****************************************************************************/
/*                                                                           */
/*  KHE_EVEN_TIME_GROUP (private) - helps calculate the unevenness           */
/*                                                                           */
/*****************************************************************************/

struct khe_even_time_group_rec {
  KHE_LAYER_MATCH	layer_match;		/* the layer match           */
  KHE_TIME_GROUP	time_group;		/* the time group            */
  int			supply_node_count;	/* supply nodes touching tg  */
};


/*****************************************************************************/
/*                                                                           */
/*  KHE_SUPPLY_NODE_GROUP (private) - all segments of a parent meet          */
/*                                                                           */
/*****************************************************************************/

struct khe_supply_node_group_rec {
  KHE_LAYER_MATCH	layer_match;		/* enclosing layer match     */
  KHE_MEET		target_meet;		/* the originating meet      */
  ARRAY_KHE_SUPPLY_NODE	supply_nodes;		/* supply nodes              */
  KHE_EVEN_TIME_GROUP	even_time_group;	/* evenness time group       */
  KHE_WMATCH_CATEGORY	wmatch_category;	/* category of this group    */
  /* ARRAY_INT		template_offsets; */	/* from first template layer */
  /* ARRAY_INT		template_durations; */	/* from first template layer */
};


/*****************************************************************************/
/*                                                                           */
/*  KHE_SUPPLY_NODE (private) - one segment of a parent meet                 */
/*                                                                           */
/*****************************************************************************/

struct khe_supply_node_rec {
  KHE_SUPPLY_NODE_GROUP	supply_node_group;	/* enclosing group           */
  int			target_offset;		/* offset in sng's meet      */
  int			duration;		/* duration                  */
  /* ARRAY_KHE_SUBLAYER	tl_sublayers; */	/* sublayers of template l's */
  /* LSET		tl_node_index_set; */	/* indexes of template nodes */
  ARRAY_KHE_ZONE	zones;			/* zones of sn               */
  LSET			zone_index_set;		/* zone indexes              */
  KHE_WMATCH_NODE	wmatch_node;		/* supply node in wmatch     */
  KHE_DEMAND_NODE	assigned_demand_node;	/* if known to be assigned   */
};


/*****************************************************************************/
/*                                                                           */
/*  KHE_RESTRICTION - a restriction to a set of zones                        */
/*                                                                           */
/*****************************************************************************/

/* ***
typedef struct khe_restriction {
  LSET			lset;			** its nodes as an lset      **
  ARRAY_KHE_NODE	array;			** its nodes as an array     **
  int			irregularity;		** its nodes' irregularity   **
} *KHE_RESTRICTION;
*** */

typedef struct khe_restriction {
  LSET			lset;			/* its zones as an lset      */
  ARRAY_KHE_ZONE	array;			/* its zones as an array     */
  int			irregularity;		/* its zones' irregularity   */
} *KHE_RESTRICTION;


/*****************************************************************************/
/*                                                                           */
/*  KHE_DEMAND_NODE_GROUP (private) - one group of demand nodes              */
/*                                                                           */
/*****************************************************************************/

struct khe_demand_node_group_rec {
  KHE_LAYER_MATCH	layer_match;		/* enclosing layer match     */
  KHE_NODE		node;			/* the originating node      */
  ARRAY_KHE_DEMAND_NODE	demand_nodes;		/* members of the group      */
  ARRAY_KHE_ZONE	zones;			/* all zones they join to    */
  /* ARRAY_KHE_SUBLAYER	tl_sublayers; */	/* sublayers of template l's */
  KHE_RESTRICTION	curr_restriction;	/* the current restriction   */
  KHE_RESTRICTION	best_restriction;	/* the best restriction      */
  KHE_WMATCH_CATEGORY	wmatch_category;	/* category of this group    */
};


/*****************************************************************************/
/*                                                                           */
/*  KHE_DEMAND_NODE (private) - one child meet                               */
/*                                                                           */
/*****************************************************************************/

struct khe_demand_node_rec {
  KHE_DEMAND_NODE_GROUP	demand_node_group;	/* enclosing demand group    */
  KHE_MEET		meet;			/* child meet                */
  KHE_WMATCH_NODE	wmatch_node;		/* demand node in wmatch     */
  KHE_SUPPLY_NODE	assigned_supply_node;	/* if known to be assigned   */
};


/*****************************************************************************/
/*                                                                           */
/*  KHE_REGULARITY_COST - cost of a restriction to a set of zones            */
/*                                                                           */
/*****************************************************************************/

typedef struct khe_regularity_cost_rec {
  int		infeasibility;			/* infeasibility             */
  KHE_COST	without_children_cost;		/* nodes without children    */
  int		zones_cost;			/* cost of zones             */
  KHE_COST	with_children_cost;		/* nodes with children       */
} KHE_REGULARITY_COST;


/*****************************************************************************/
/*                                                                           */
/*  KHE_LAYER_MATCH - one layer matching                                     */
/*                                                                           */
/*****************************************************************************/

struct khe_layer_match_rec {
  KHE_LAYER			layer;		     /* matching's layer     */
  KHE_SPREAD_EVENTS_CONSTRAINT	constraint;          /* matching's sec       */
  ARRAY_KHE_SUPPLY_NODE_GROUP	supply_node_groups;  /* supply node groups   */
  ARRAY_KHE_DEMAND_NODE_GROUP	demand_node_groups;  /* demand node groups   */
  ARRAY_KHE_DEMAND_NODE		demand_nodes;        /* demand nodes         */
  ARRAY_KHE_EVEN_TIME_GROUP	even_time_groups;    /* even time groups     */
  int				unevenness;          /* unevenness of tg's   */
  KHE_WMATCH			wmatch;		     /* the matching itself  */

  /* used by node regularity code */
  KHE_REGULARITY_COST		best_reg_cost;	/* best node regularity cost */
};


/*****************************************************************************/
/*                                                                           */
/*  Submodule "even time groups" (private)                                   */
/*                                                                           */
/*****************************************************************************/

/*****************************************************************************/
/*                                                                           */
/*  KHE_EVEN_TIME_GROUP KheEvenTimeGroupMake(KHE_LAYER_MATCH lm,             */
/*    KHE_TIME_GROUP tg)                                                     */
/*                                                                           */
/*  Make an even time group with these attributes.                           */
/*                                                                           */
/*****************************************************************************/

static KHE_EVEN_TIME_GROUP KheEvenTimeGroupMake(KHE_LAYER_MATCH lm,
  KHE_TIME_GROUP tg)
{
  KHE_EVEN_TIME_GROUP res;
  MMake(res);
  res->layer_match = lm;
  res->time_group = tg;
  res->supply_node_count = 0;
  return res;
}


/*****************************************************************************/
/*                                                                           */
/*  void KheEvenTimeGroupAddSupplyNode(KHE_EVEN_TIME_GROUP etg)              */
/*                                                                           */
/*  Inform etg that is has an extra supply node now.                         */
/*                                                                           */
/*****************************************************************************/

static void KheEvenTimeGroupAddSupplyNode(KHE_EVEN_TIME_GROUP etg)
{
  etg->supply_node_count++;
  etg->layer_match->unevenness += etg->supply_node_count;
}


/*****************************************************************************/
/*                                                                           */
/*  void KheEvenTimeGroupDeleteSupplyNode(KHE_EVEN_TIME_GROUP etg)           */
/*                                                                           */
/*  Inform etg that is has one less supply node now.                         */
/*                                                                           */
/*****************************************************************************/

static void KheEvenTimeGroupDeleteSupplyNode(KHE_EVEN_TIME_GROUP etg)
{
  etg->layer_match->unevenness -= etg->supply_node_count;
  etg->supply_node_count--;
  MAssert(etg->supply_node_count >= 0,
    "KheEvenTimeGroupDeleteSupplyNode internal error");
}


/*****************************************************************************/
/*                                                                           */
/*  void KheEvenTimeGroupFree(KHE_EVEN_TIME_GROUP etg)                       */
/*                                                                           */
/*  Free etg.                                                                */
/*                                                                           */
/*****************************************************************************/

static void KheEvenTimeGroupFree(KHE_EVEN_TIME_GROUP etg)
{
  MFree(etg);
}


/*****************************************************************************/
/*                                                                           */
/*  Submodule "sublayers" (private)                                          */
/*                                                                           */
/*****************************************************************************/

/*****************************************************************************/
/*                                                                           */
/*  KHE_SUBLAYER KheSubLayerMake(KHE_LAYER layer)                            */
/*                                                                           */
/*  Make a new, empty sublayer of layer.                                     */
/*                                                                           */
/*****************************************************************************/

/* ***
static KHE_SUBLAYER KheSubLayerMake(KHE_LAYER layer)
{
  KHE_SUBLAYER res;
  MMake(res);
  res->layer = layer;
  MArrayInit(res->nodes);
  return res;
}
*** */


/*****************************************************************************/
/*                                                                           */
/*  void KheSubLayerDelete(KHE_SUBLAYER sublayer)                            */
/*                                                                           */
/*  Delete sublayer.                                                         */
/*                                                                           */
/*****************************************************************************/

/* ***
static void KheSubLayerDelete(KHE_SUBLAYER sublayer)
{
  MArrayFree(sublayer->nodes);
  MFree(sublayer);
}
*** */


/*****************************************************************************/
/*                                                                           */
/*  void KheSubLayerAddNode(KHE_SUBLAYER sublayer, KHE_NODE node)            */
/*                                                                           */
/*  Add node to sublayer.                                                    */
/*                                                                           */
/*****************************************************************************/

/* ***
static void KheSubLayerAddNode(KHE_SUBLAYER sublayer, KHE_NODE node)
{
  MArrayAddLast(sublayer->nodes, node);
}
*** */


/*****************************************************************************/
/*                                                                           */
/*  void KheSubLayerDeleteNode(KHE_SUBLAYER sublayer, KHE_NODE node)         */
/*                                                                           */
/*  Delete node from sublayer; it must be present.                           */
/*                                                                           */
/*****************************************************************************/

/* *** not currently used
static void KheSubLayerDeleteNode(KHE_SUBLAYER sublayer, KHE_NODE node)
{
  int pos;
  if( !MArrayContains(sublayer->nodes, node, &pos) )
    MAssert(false, "KheSubLayerDeleteNode internal error");
  MArrayRemove(sublayer->nodes, pos);
}
*** */


/*****************************************************************************/
/*                                                                           */
/*  void KheSubLayerRotateLeft(KHE_SUBLAYER sublayer, int i, int j)          */
/*                                                                           */
/*  Rotate sublayer->nodes[i .. j-1] one place to the left.                  */
/*                                                                           */
/*****************************************************************************/

/* ***
static void KheSubLayerRotateLeft(KHE_SUBLAYER sublayer, int i, int j)
{
  KHE_NODE tmp;  int k;
  tmp = MArrayGet(sublayer->nodes, i);
  for( k = i + 1;  k < j;  k++ )
    MArrayPut(sublayer->nodes, k-1, MArrayGet(sublayer->nodes, k));
  MArrayPut(sublayer->nodes, k-1, tmp);
}
*** */


/*****************************************************************************/
/*                                                                           */
/*  void KheSubLayerRotateNodes(KHE_SUBLAYER sublayer, int i, int j, int r)  */
/*                                                                           */
/*  Rotate sublayer->tl_nodes[i .. j-1] r places to the left.                */
/*                                                                           */
/*****************************************************************************/

/* ***
static void KheSubLayerRotateNodes(KHE_SUBLAYER sublayer, int i, int j, int r)
{
  int k;
  for( k = 0;  k < r;  k++ )
    KheSubLayerRotateLeft(sublayer, i, j);
}
*** */


/*****************************************************************************/
/*                                                                           */
/*  int KheNodeDecreasingDurationCmp(const void *t1, const void *t2)         */
/*                                                                           */
/*  Comparison function for sorting an array of nodes by decreasing durn.    */
/*                                                                           */
/*****************************************************************************/

/* ***
static int KheNodeDecreasingDurationCmp(const void *t1, const void *t2)
{
  KHE_NODE node1 = * (KHE_NODE *) t1;
  KHE_NODE node2 = * (KHE_NODE *) t2;
  return KheNodeDuration(node2) - KheNodeDuration(node1);
}
*** */


/*****************************************************************************/
/*                                                                           */
/*  void KheSubLayerDiversify(KHE_SUBLAYER sublayer)                         */
/*                                                                           */
/*  Diversify sublayer.  After sorting its template nodes by decreasing      */
/*  duration, take each subsequence of template nodes of equal duration and  */
/*  rotate them by a number of places determined by soln's diversifier.      */
/*                                                                           */
/*****************************************************************************/

/* ***
static void KheSubLayerDiversify(KHE_SUBLAYER sublayer)
{
  KHE_SOLN soln;  int i, j;  KHE_NODE nodei, nodej;
  if( MArraySize(sublayer->nodes) > 0 )
  {
    MArraySort(sublayer->nodes, &KheNodeDecreasingDurationCmp);
    soln = KheNodeSoln(MArrayFirst(sublayer->nodes));
    for( i = 0;  i < MArraySize(sublayer->nodes);  i = j )
    {
      nodei = MArrayGet(sublayer->nodes, i);
      for( j = i + 1;  j < MArraySize(sublayer->nodes);  j++ )
      {
	nodej = MArrayGet(sublayer->nodes, j);
	if( KheNodeDuration(nodej) != KheNodeDuration(nodei) )
	  break;
      }
      if( DEBUG8 )
	fprintf(stderr, "  diversifier %d given %d choices: %d\n",
	  KheSolnDiversifier(soln), (j - i), KheSolnDiversifier(soln)%(j - i));
      KheSubLayerRotateNodes(sublayer, i, j,
	KheSolnDiversifierChoose(soln, j - i));
    }
  }
}
*** */


/*****************************************************************************/
/*                                                                           */
/*  void KheSubLayerDebug(KHE_SUBLAYER sublayer, int verbosity,              */
/*    int indent, FILE *fp)                                                  */
/*                                                                           */
/*  Debug print of sublayer onto fp with the given verbosity and indent.     */
/*                                                                           */
/*****************************************************************************/

/* ***
static void KheSubLayerDebug(KHE_SUBLAYER sublayer, int verbosity,
  int indent, FILE *fp)
{
  KHE_NODE node;  int i;
  if( verbosity >= 1 )
  {
    if( indent >= 0 )
      fprintf(fp, "%*s", indent, "");
    fprintf(fp, "[ SubLayer:");
    MArrayForEach(sublayer->nodes, &node, &i)
    {
      if( i > 0 )
	fprintf(fp, ", ");
      else
	fprintf(fp, " ");
      KheNodeDebug(node, 1, -1, fp);
    }
    fprintf(fp, " ]");
    if( indent >= 0 )
      fprintf(fp, "\n");
  }
}
*** */


/*****************************************************************************/
/*                                                                           */
/*  Submodule "supply node groups" (private)                                 */
/*                                                                           */
/*****************************************************************************/

/*****************************************************************************/
/*                                                                           */
/*  KHE_SUPPLY_NODE_GROUP KheSupplyNodeGroupMake(KHE_LAYER_MATCH lm,         */
/*    KHE_MEET target_meet)                                                  */
/*                                                                           */
/*  Make a new supply node group with these attributes, but do not add       */
/*  it to lm.                                                                */
/*                                                                           */
/*****************************************************************************/

static KHE_SUPPLY_NODE_GROUP KheSupplyNodeGroupMake(KHE_LAYER_MATCH lm,
  KHE_EVEN_TIME_GROUP even_time_group, KHE_MEET target_meet)
{
  KHE_SUPPLY_NODE_GROUP res;
  MMake(res);
  res->layer_match = lm;
  res->target_meet = target_meet;
  MArrayInit(res->supply_nodes);
  res->even_time_group = even_time_group;
  res->wmatch_category = KheWMatchNewCategory(lm->wmatch);
  /* ***
  MArrayInit(res->template_offsets);
  MArrayInit(res->template_durations);
  *** */
  return res;
}


/*****************************************************************************/
/*                                                                           */
/*  void KheSupplyNodeGroupAddSupplyNode(KHE_SUPPLY_NODE_GROUP sng,          */
/*    KHE_SUPPLY_NODE sn)                                                    */
/*                                                                           */
/*  Add sn to sng.                                                           */
/*                                                                           */
/*****************************************************************************/

static void KheSupplyNodeGroupAddSupplyNode(KHE_SUPPLY_NODE_GROUP sng,
  KHE_SUPPLY_NODE sn)
{
  MArrayAddLast(sng->supply_nodes, sn);
  if( sng->even_time_group != NULL )
    KheEvenTimeGroupAddSupplyNode(sng->even_time_group);
}


/*****************************************************************************/
/*                                                                           */
/*  void KheSupplyNodeGroupDeleteSupplyNode(KHE_SUPPLY_NODE_GROUP sng,       */
/*    KHE_SUPPLY_NODE sn)                                                    */
/*                                                                           */
/*  Delete sn from sng.                                                      */
/*                                                                           */
/*****************************************************************************/

static void KheSupplyNodeGroupDeleteSupplyNode(KHE_SUPPLY_NODE_GROUP sng,
  KHE_SUPPLY_NODE sn)
{
  KHE_SUPPLY_NODE sn2;  int i;
  MArrayForEachReverse(sng->supply_nodes, &sn2, &i)
    if( sn2 == sn )
    {
      MArrayRemove(sng->supply_nodes, i);
      if( sng->even_time_group != NULL )
	KheEvenTimeGroupDeleteSupplyNode(sng->even_time_group);
      return;
    }
  MAssert(false, "KheSupplyNodeGroupDeleteSupplyNode internal error");
}


/*****************************************************************************/
/*                                                                           */
/*  void KheSupplyNodeGroupDelete(KHE_SUPPLY_NODE_GROUP sng)                 */
/*                                                                           */
/*  Delete sng and its supply nodes.                                         */
/*                                                                           */
/*****************************************************************************/
static void KheSupplyNodeDelete(KHE_SUPPLY_NODE sn);

static void KheSupplyNodeGroupDelete(KHE_SUPPLY_NODE_GROUP sng)
{
  while( MArraySize(sng->supply_nodes) > 0 )
    KheSupplyNodeDelete(MArrayLast(sng->supply_nodes));
  MArrayFree(sng->supply_nodes);
  /* ***
  MArrayFree(sng->template_offsets);
  MArrayFree(sng->template_durations);
  *** */
  MFree(sng);
}


/*****************************************************************************/
/*                                                                           */
/*  void KheSupplyNodeGroupDebug(KHE_SUPPLY_NODE_GROUP sng,                  */
/*    int verbosity, int indent, FILE *fp)                                   */
/*                                                                           */
/*  Debug printf of sng onto fp.                                             */
/*                                                                           */
/*****************************************************************************/
static void KheSupplyNodeDebug(KHE_SUPPLY_NODE sn, int verbosity,
  int indent, FILE *fp);

static void KheSupplyNodeGroupDebug(KHE_SUPPLY_NODE_GROUP sng,
  int verbosity, int indent, FILE *fp)
{
  KHE_SUPPLY_NODE sn;  int i;
  if( verbosity >= 1 && indent >= 0 )
  {
    fprintf(fp, "%*s[ SupplyNodeGroup(", indent, "");
    KheMeetDebug(sng->target_meet, 1, -1, fp);
    if( sng->even_time_group != NULL )
      fprintf(fp, ", even %s:%d",
	KheTimeGroupId(sng->even_time_group->time_group),
        sng->even_time_group->supply_node_count);
    fprintf(fp, ")\n");
    MArrayForEach(sng->supply_nodes, &sn, &i)
      KheSupplyNodeDebug(sn, verbosity, indent + 2, fp);
    /* ***
    if( MArraySize(sng->template_offsets) >= 1 )
    {
      fprintf(fp, "%*s  template offsets and durations:", indent, "");
      MArrayForEach(sng->template_offsets, &offset, &i)
      {
	if( i > 0 )
	  fprintf(fp, ", ");
	fprintf(fp, "+%dd%d", offset, MArrayGet(sng->template_durations, i));
      }
      fprintf(fp, "\n");
    }
    *** */
    fprintf(fp, "%*s]\n", indent, "");
  }
}


/*****************************************************************************/
/*                                                                           */
/*  Submodule "supply nodes" (private)                                       */
/*                                                                           */
/*****************************************************************************/

/*****************************************************************************/
/*                                                                           */
/*  KHE_SUPPLY_NODE KheSupplyNodeMake(KHE_SUPPLY_NODE_GROUP sng,             */
/*    int target_offset, int duration)                                       */
/*                                                                           */
/*  Make a new supply node with these attributes and add it to sng.          */
/*                                                                           */
/*****************************************************************************/

static KHE_SUPPLY_NODE KheSupplyNodeMake(KHE_SUPPLY_NODE_GROUP sng,
  int target_offset, int duration)
{
  KHE_SUPPLY_NODE res;
  MMake(res);
  res->supply_node_group = sng;
  res->target_offset = target_offset;
  res->duration = duration;
  res->assigned_demand_node = NULL;
  MArrayInit(res->zones);
  res->zone_index_set = NULL;
  /* ***
  MArrayInit(res->tl_sublayers);
  res->tl_node_index_set = NULL;
  *** */
  res->wmatch_node = KheWMatchSupplyNodeMake(sng->layer_match->wmatch, res,
    sng->wmatch_category);
  KheSupplyNodeGroupAddSupplyNode(sng, res);
  return res;
}


/*****************************************************************************/
/*                                                                           */
/*  void KheSupplyNodeDelete(KHE_SUPPLY_NODE sn)                             */
/*                                                                           */
/*  Delete sn, including deleting it from its supply node group.             */
/*                                                                           */
/*****************************************************************************/

static void KheSupplyNodeDelete(KHE_SUPPLY_NODE sn)
{
  KheSupplyNodeGroupDeleteSupplyNode(sn->supply_node_group, sn);
  MArrayFree(sn->zones);
  if( sn->zone_index_set != NULL )
    LSetFree(sn->zone_index_set);
  /* ***
  while( MArraySize(sn->tl_sublayers) > 0 )
    KheSubLayerDelete(MArrayRemoveLast(sn->tl_sublayers));
  if( sn->tl_node_index_set != NULL )
    LSetFree(sn->tl_node_index_set);
  *** */
  KheWMatchSupplyNodeDelete(sn->wmatch_node);
  MFree(sn);
}


/*****************************************************************************/
/*                                                                           */
/*  void KheSupplyNodeSplit(KHE_SUPPLY_NODE sn, int target_offset,           */
/*    int durn, KHE_SUPPLY_NODE *lsn, KHE_SUPPLY_NODE *rsn)                  */
/*                                                                           */
/*  Split sn as required to ensure that there is a supply node for sn's      */
/*  target_meet with this target_offset and durn.  If this function is       */
/*  called at all, it must be possible to do that.                           */
/*                                                                           */
/*****************************************************************************/

static void KheSupplyNodeSplit(KHE_SUPPLY_NODE sn, int target_offset,
  int durn, KHE_SUPPLY_NODE *lsn, KHE_SUPPLY_NODE *rsn)
{
  /* split to left of [target_offset, target_offset + durn] if required */
  MAssert(sn->assigned_demand_node == NULL,
    "KheSupplyNodeSplit internal error 1");
  MAssert(sn->target_offset <= target_offset,
    "KheSupplyNodeSplit internal error 2");
  if( sn->target_offset < target_offset )
  {
    *lsn = KheSupplyNodeMake(sn->supply_node_group, sn->target_offset,
      target_offset - sn->target_offset);
    sn->duration -= target_offset - sn->target_offset;
    sn->target_offset = target_offset;
    KheWMatchSupplyNodeNotifyDirty(sn->wmatch_node);
  }
  else
    *lsn = NULL;

  /* split to right of [target_offset, target_offset + durn] if required */
  MAssert(sn->duration >= durn, "KheSupplyNodeSplit internal error 3");
  if( sn->duration > durn )
  {
    *rsn = KheSupplyNodeMake(sn->supply_node_group, sn->target_offset + durn,
      sn->duration - durn);
    sn->duration = durn;
    KheWMatchSupplyNodeNotifyDirty(sn->wmatch_node);
  }
  else
    *rsn = NULL;

  if( DEBUG2 )
  {
    fprintf(stderr, "  KheSupplyNodeSplit: ");
    if( *lsn == NULL )
      fprintf(stderr, "-");
    else
      KheSupplyNodeDebug(*lsn, 1, -1, stderr);
    fprintf(stderr, ", ");
    KheSupplyNodeDebug(sn, 1, -1, stderr);
    fprintf(stderr, ", ");
    if( *rsn == NULL )
      fprintf(stderr, "-");
    else
      KheSupplyNodeDebug(*rsn, 1, -1, stderr);
    fprintf(stderr, "\n");
  }
}


/*****************************************************************************/
/*                                                                           */
/*  void KheSupplyNodeMerge(KHE_SUPPLY_NODE lsn, KHE_SUPPLY_NODE sn,         */
/*    KHE_SUPPLY_NODE rsn)                                                   */
/*                                                                           */
/*  Merge lsn, sn, and rsn.  Either or both of lsn and rsn could be NULL.    */
/*                                                                           */
/*****************************************************************************/

static void KheSupplyNodeMerge(KHE_SUPPLY_NODE lsn, KHE_SUPPLY_NODE sn,
  KHE_SUPPLY_NODE rsn)
{
  /* merge lsn into sn if present */
  MAssert(sn->assigned_demand_node == NULL,
    "KheSupplyNodeMerge internal error 1");
  if( lsn != NULL )
  {
    MAssert(lsn->supply_node_group == sn->supply_node_group,
      "KheSupplyNodeMerge internal error 2");
    MAssert(lsn->assigned_demand_node == NULL,
      "KheSupplyNodeMerge internal error 3");
    MAssert(lsn->target_offset + lsn->duration == sn->target_offset,
      "KheSupplyNodeMerge internal error 4");
    sn->target_offset = lsn->target_offset;
    sn->duration += lsn->duration;
    KheSupplyNodeDelete(lsn);
    KheWMatchSupplyNodeNotifyDirty(sn->wmatch_node);
  }

  /* merge rsn into sn if present */
  if( rsn != NULL )
  {
    MAssert(rsn->supply_node_group == sn->supply_node_group,
      "KheSupplyNodeMerge internal error 5");
    MAssert(rsn->assigned_demand_node == NULL,
      "KheSupplyNodeMerge internal error 6");
    MAssert(sn->target_offset + sn->duration == rsn->target_offset,
      "KheSupplyNodeMerge internal error 7");
    sn->duration += rsn->duration;
    KheSupplyNodeDelete(rsn);
    KheWMatchSupplyNodeNotifyDirty(sn->wmatch_node);
  }

  if( DEBUG2 )
  {
    fprintf(stderr, "  KheSupplyNodeMerge: ");
    KheSupplyNodeDebug(sn, 1, -1, stderr);
    fprintf(stderr, "\n");
  }
}


/*****************************************************************************/
/*                                                                           */
/*  void KheSupplyNodeDebug(KHE_SUPPLY_NODE sn, int verbosity,               */
/*    int indent, FILE *fp)                                                  */
/*                                                                           */
/*  Debug print of sn onto fp with the given verbosity and indent.           */
/*                                                                           */
/*****************************************************************************/

static void KheSupplyNodeDebug(KHE_SUPPLY_NODE sn, int verbosity,
  int indent, FILE *fp)
{
  /* KHE_SUBLAYER sublayer;  int i; */
  if( verbosity >= 1 )
  {
    if( indent >= 0 )
      fprintf(fp, "%*s", indent, "");
    KheMeetDebug(sn->supply_node_group->target_meet, 1, -1, fp);
    fprintf(fp, "+%dd%d", sn->target_offset, sn->duration);
    if( sn->zone_index_set != NULL && !LSetEmpty(sn->zone_index_set) )
      fprintf(fp, " z%s", LSetShow(sn->zone_index_set));
    /* ***
    if( sn->tl_node_index_set != NULL && !LSetEmpty(sn->tl_node_index_set) )
      fprintf(fp, " n%s", LSetShow(sn->tl_node_index_set));
    *** */
    if( indent >= 0 )
      fprintf(fp, "\n");
    /* ***
    if( indent >= 0 && verbosity >= 2 )
      MArrayForEach(sn->tl_sublayers, &sublayer, &i)
	KheSubLayerDebug(sublayer, verbosity, indent + 2, fp);
    *** */
  }
}


/*****************************************************************************/
/*                                                                           */
/*  Submodule "restrictions"                                                 */
/*                                                                           */
/*****************************************************************************/

/*****************************************************************************/
/*                                                                           */
/*  KHE_RESTRICTION KheRestrictionMake(void)                                 */
/*                                                                           */
/*  Make a new, empty restriction.                                           */
/*                                                                           */
/*****************************************************************************/

static KHE_RESTRICTION KheRestrictionMake(void)
{
  KHE_RESTRICTION res;
  MMake(res);
  res->lset = LSetNew();
  MArrayInit(res->array);
  res->irregularity = 0;
  return res;
}


/*****************************************************************************/
/*                                                                           */
/*  void KheRestrictionFree(KHE_RESTRICTION r)                               */
/*                                                                           */
/*  Free r.                                                                  */
/*                                                                           */
/*****************************************************************************/

static void KheRestrictionFree(KHE_RESTRICTION r)
{
  LSetFree(r->lset);
  MArrayFree(r->array);
  MFree(r);
}


/*****************************************************************************/
/*                                                                           */
/*  void KheRestrictionClear(KHE_RESTRICTION r)                              */
/*                                                                           */
/*  Clear r back to the empty set of zones.                                  */
/*                                                                           */
/*****************************************************************************/

static void KheRestrictionClear(KHE_RESTRICTION r)
{
  LSetClear(r->lset);
  MArrayClear(r->array);
  r->irregularity = 0;
}


/*****************************************************************************/
/*                                                                           */
/*  bool KheRestrictionIsEmpty(KHE_RESTRICTION r)                            */
/*                                                                           */
/*  Return true if r contains no zones.                                      */
/*                                                                           */
/*****************************************************************************/

static bool KheRestrictionIsEmpty(KHE_RESTRICTION r)
{
  return MArraySize(r->array) == 0;
}


/*****************************************************************************/
/*                                                                           */
/*  void KheRestrictionAssign(KHE_RESTRICTION target_r,                      */
/*    KHE_RESTRICTION source_r)                                              */
/*                                                                           */
/*  Copy source_r onto target_r.                                             */
/*                                                                           */
/*****************************************************************************/

static void KheRestrictionAssign(KHE_RESTRICTION target_r,
  KHE_RESTRICTION source_r)
{
  int i;
  LSetAssign(&target_r->lset, source_r->lset);
  MArrayClear(target_r->array);
  MArrayAppend(target_r->array, source_r->array, i);
  target_r->irregularity = source_r->irregularity;
}


/*****************************************************************************/
/*                                                                           */
/*  void KheRestrictionPushZone(KHE_RESTRICTION r, KHE_ZONE zone,            */
/*    int irregularity)                                                      */
/*                                                                           */
/*  Add zone, which has the given irregularity, to r.                        */
/*                                                                           */
/*****************************************************************************/

static void KheRestrictionPushZone(KHE_RESTRICTION r, KHE_ZONE zone,
  int irregularity)
{
  if( DEBUG7 )
  {
    fprintf(stderr, "    PushZone(r, ");
    KheZoneDebug(zone, 1, -1, stderr);
    fprintf(stderr, ", %d)\n", irregularity);
  }
  LSetInsert(&r->lset, KheZoneIndex(zone));
  MArrayAddLast(r->array, zone);
  r->irregularity += irregularity;
}


/*****************************************************************************/
/*                                                                           */
/*  void KheRestrictionPushNode(KHE_RESTRICTION r, KHE_NODE node,            */
/*    int irregularity)                                                      */
/*                                                                           */
/*  Add node, which has the given irregularity, to r.                        */
/*                                                                           */
/*****************************************************************************/

/* ***
static void KheRestrictionPushNode(KHE_RESTRICTION r, KHE_NODE node,
  int irregularity)
{
  if( DEBUG7 )
  {
    fprintf(stderr, "    PushNode(r, ");
    KheNodeDebug(node, 1, -1, stderr);
    fprintf(stderr, ", %d)\n", irregularity);
  }
  LSetInsert(&r->lset, KheNodeIndex(node));
  MArrayAddLast(r->array, node);
  r->irregularity += irregularity;
}
*** */


/*****************************************************************************/
/*                                                                           */
/*  void KheRestrictionPopZone(KHE_RESTRICTION r, KHE_ZONE zone,             */
/*    int irregularity)                                                      */
/*                                                                           */
/*  Pop zone, which has the given irregularity, from r.                      */
/*                                                                           */
/*****************************************************************************/

static void KheRestrictionPopZone(KHE_RESTRICTION r, KHE_ZONE zone,
  int irregularity)
{
  LSetDelete(r->lset, KheZoneIndex(zone));
  MAssert(MArrayLast(r->array) == zone, "KheRestrictionPopZone internal error");
  MArrayDropLast(r->array);
  r->irregularity -= irregularity;
  MAssert(r->irregularity >= 0, "KheRestrictionPopZone internal error 2");
  if( DEBUG7 )
  {
    fprintf(stderr, "    PopZone(r, ");
    KheZoneDebug(zone, 1, -1, stderr);
    fprintf(stderr, ", %d)\n", irregularity);
  }
}


/*****************************************************************************/
/*                                                                           */
/*  void KheRestrictionPopNode(KHE_RESTRICTION r, KHE_NODE node,             */
/*    int irregularity)                                                      */
/*                                                                           */
/*  Pop node, which has the given irregularity, from r.                      */
/*                                                                           */
/*****************************************************************************/

/* ***
static void KheRestrictionPopNode(KHE_RESTRICTION r, KHE_NODE node,
  int irregularity)
{
  LSetDelete(r->lset, KheNodeIndex(node));
  MAssert(MArrayLast(r->array) == node, "KheRestrictionPopNode internal error");
  MArrayDropLast(r->array);
  r->irregularity -= irregularity;
  MAssert(r->irregularity >= 0, "KheRestrictionPopNode internal error 2");
  if( DEBUG7 )
  {
    fprintf(stderr, "    PopNode(r, ");
    KheNodeDebug(node, 1, -1, stderr);
    fprintf(stderr, ", %d)\n", irregularity);
  }
}
*** */


/*****************************************************************************/
/*                                                                           */
/*  bool KheRestrictionContains(KHE_RESTRICTION r, KHE_ZONE zone)            */
/*                                                                           */
/*  Return true if r contains zone.                                          */
/*                                                                           */
/*****************************************************************************/

static bool KheRestrictionContains(KHE_RESTRICTION r, KHE_ZONE zone)
{
  return LSetContains(r->lset, KheZoneIndex(zone));
}


/*****************************************************************************/
/*                                                                           */
/*  bool KheRestrictionContains(KHE_RESTRICTION r, KHE_NODE node)            */
/*                                                                           */
/*  Return true if r contains node.                                          */
/*                                                                           */
/*****************************************************************************/

/* ***
static bool KheRestrictionContains(KHE_RESTRICTION r, KHE_NODE node)
{
  return LSetContains(r->lset, KheNodeIndex(node));
}
*** */


/*****************************************************************************/
/*                                                                           */
/*  Submodule "demand node groups - basic operations"                        */
/*                                                                           */
/*****************************************************************************/

/*****************************************************************************/
/*                                                                           */
/*  KHE_DEMAND_NODE_GROUP KheDemandNodeGroupMake(KHE_LAYER_MATCH lm,         */
/*    KHE_NODE node)                                                         */
/*                                                                           */
/*  Make a new demand node group for the demand nodes of node, but do not    */
/*  add it to lm.                                                            */
/*                                                                           */
/*****************************************************************************/

static KHE_DEMAND_NODE_GROUP KheDemandNodeGroupMake(KHE_LAYER_MATCH lm,
  KHE_NODE node)
{
  KHE_DEMAND_NODE_GROUP res;
  MMake(res);
  res->layer_match = lm;
  res->node = node;
  MArrayInit(res->demand_nodes);
  MArrayInit(res->zones);
  /* MArrayInit(res->tl_sublayers); */
  res->curr_restriction = NULL;
  res->best_restriction = NULL;
  res->wmatch_category = KheWMatchNewCategory(lm->wmatch);
  return res;
}


/*****************************************************************************/
/*                                                                           */
/*  void KheDemandNodeGroupAddDemandNode(KHE_DEMAND_NODE_GROUP dng,          */
/*    KHE_DEMAND_NODE dn)                                                    */
/*                                                                           */
/*  Add dn to dng.                                                           */
/*                                                                           */
/*****************************************************************************/

static void KheDemandNodeGroupAddDemandNode(KHE_DEMAND_NODE_GROUP dng,
  KHE_DEMAND_NODE dn)
{
  MArrayAddLast(dng->demand_nodes, dn);
}


/*****************************************************************************/
/*                                                                           */
/*  void KheDemandNodeGroupDeleteDemandNode(KHE_DEMAND_NODE_GROUP dng,       */
/*    KHE_DEMAND_NODE dn)                                                    */
/*                                                                           */
/*  Delete dn from dng.                                                      */
/*                                                                           */
/*****************************************************************************/

static void KheDemandNodeGroupDeleteDemandNode(KHE_DEMAND_NODE_GROUP dng,
  KHE_DEMAND_NODE dn)
{
  KHE_DEMAND_NODE dn2;  int i;
  MArrayForEachReverse(dng->demand_nodes, &dn2, &i)
    if( dn2 == dn )
    {
      MArrayRemove(dng->demand_nodes, i);
      return;
    }
  MAssert(false, "KheDemandNodeGroupDeleteDemandNode internal error");
}


/*****************************************************************************/
/*                                                                           */
/*  void KheDemandNodeGroupDelete(KHE_DEMAND_NODE_GROUP dng)                 */
/*                                                                           */
/*  Delete dng and its demand nodes, but do not delete dng from the match.   */
/*                                                                           */
/*****************************************************************************/
static void KheDemandNodeDelete(KHE_DEMAND_NODE dn);

static void KheDemandNodeGroupDelete(KHE_DEMAND_NODE_GROUP dng)
{
  while( MArraySize(dng->demand_nodes) > 0 )
    KheDemandNodeDelete(MArrayLast(dng->demand_nodes));
  MArrayFree(dng->demand_nodes);
  MArrayFree(dng->zones);
  /* ***
  while( MArraySize(dng->tl_sublayers) > 0 )
    KheSubLayerDelete(MArrayRemoveLast(dng->tl_sublayers));
  *** */
  if( dng->curr_restriction != NULL )
  {
    KheRestrictionFree(dng->curr_restriction);
    KheRestrictionFree(dng->best_restriction);
  }
  MFree(dng);
}


/*****************************************************************************/
/*                                                                           */
/*  void KheDemandNodeGroupDebug(KHE_DEMAND_NODE_GROUP dng,                  */
/*    int verbosity, int indent, FILE *fp)                                   */
/*                                                                           */
/*  Debug print of dng onto fp with the given verbosity and indent.          */
/*                                                                           */
/*****************************************************************************/

static void KheDemandNodeGroupDebug(KHE_DEMAND_NODE_GROUP dng,
  int verbosity, int indent, FILE *fp)
{
  int i;  KHE_ZONE zone;
  if( verbosity >= 1 )
  {
    if( indent >= 0 )
      fprintf(fp, "%*s", indent, "");
    KheNodeDebug(dng->node, 1, -1, fp);
    if( verbosity > 1 )
    {
      fprintf(fp, ":");
      if( dng->curr_restriction != NULL )
	fprintf(fp, " %s", LSetShow(dng->curr_restriction->lset));
      MArrayForEach(dng->zones, &zone, &i)
      {
	if( indent >= 0 )
	  fprintf(fp, "\n%*s  ", indent, "");
	else
	  fprintf(fp, " ");
	KheZoneDebug(zone, 1, -1, fp);
      }
      /* ***
      MArrayForEach(dng->tl_sublayers, &sublayer, &i)
      {
	if( indent >= 0 )
	  fprintf(fp, "\n%*s  ", indent, "");
	else
	  fprintf(fp, " ");
	KheSubLayerDebug(sublayer, 1, -1, fp);
      }
      *** */
    }
    if( indent >= 0 )
      fprintf(fp, "\n");
  }
}


/*****************************************************************************/
/*                                                                           */
/*  Submodule "demand node groups - zones"                                   */
/*                                                                           */
/*****************************************************************************/

/*****************************************************************************/
/*                                                                           */
/*  void KheDemandNodeGroupNotifyDirty(KHE_DEMAND_NODE_GROUP dng)            */
/*                                                                           */
/*  Notify wmatch that every node of this demand node group is now dirty.    */
/*                                                                           */
/*****************************************************************************/

static void KheDemandNodeGroupNotifyDirty(KHE_DEMAND_NODE_GROUP dng)
{
  int i;  KHE_DEMAND_NODE dn;
  MArrayForEach(dng->demand_nodes, &dn, &i)
    KheWMatchDemandNodeNotifyDirty(dn->wmatch_node);
}


/*****************************************************************************/
/*                                                                           */
/*  bool KheDemandNodeGroupContainsTemplateLayerNode(                        */
/*    KHE_DEMAND_NODE_GROUP dng, int layer_index, KHE_NODE node)             */
/*                                                                           */
/*  Return true if dng contains node at this index.                          */
/*                                                                           */
/*****************************************************************************/

/* ***
static bool KheDemandNodeGroupContainsTemplateLayerNode(
  KHE_DEMAND_NODE_GROUP dng, int layer_index, KHE_NODE node)
{
  KHE_SUBLAYER sublayer;  int pos;
  sublayer = MArrayGet(dng->tl_sublayers, layer_index);
  return MArrayContains(sublayer->nodes, node, &pos);
}
*** */


/*****************************************************************************/
/*                                                                           */
/*  void KheDemandNodeGroupAddTemplateLayerNode(KHE_DEMAND_NODE_GROUP dng,   */
/*    KHE_LAYER layer, KHE_NODE node)                                        */
/*                                                                           */
/*  Add node (which comes from layer) to dng's list of template layer nodes. */
/*                                                                           */
/*****************************************************************************/

/* ***
static void KheDemandNodeGroupAddTemplateLayerNode(KHE_DEMAND_NODE_GROUP dng,
  int layer_index, KHE_NODE node)
{
  KHE_SUBLAYER sublayer;  int pos;
  sublayer = MArrayGet(dng->tl_sublayers, layer_index);
  if( !MArrayContains(sublayer->nodes, node, &pos) )
    KheSubLayerAddNode(sublayer, node);
}
*** */


/*****************************************************************************/
/*                                                                           */
/*  void KheDemandNodeGroupAddZone(KHE_DEMAND_NODE_GROUP dng, KHE_ZONE zone) */
/*                                                                           */
/*  Add zone to dng, unless it's already present.                            */
/*                                                                           */
/*****************************************************************************/

static void KheDemandNodeGroupAddZone(KHE_DEMAND_NODE_GROUP dng, KHE_ZONE zone)
{
  int pos;
  if( !MArrayContains(dng->zones, zone, &pos) )
    MArrayAddLast(dng->zones, zone);
}


/*****************************************************************************/
/*                                                                           */
/*  void KheDemandNodeGroupDeleteTemplateLayerNode(KHE_DEMAND_NODE_GROUP dng,*/
/*    KHE_LAYER layer, KHE_NODE node)                                        */
/*                                                                           */
/*  Delete node from dng's list of template layer nodes, and inform the      */
/*  wmatch that all the affected demand nodes are now dirty.                 */
/*                                                                           */
/*****************************************************************************/

/* *** not currently used
static void KheDemandNodeGroupDeleteTemplateLayerNode(KHE_DEMAND_NODE_GROUP dng,
  int layer_index, KHE_NODE node)
{
  KHE_SUBLAYER sublayer;
  sublayer = MArrayGet(dng->tl_sublayers, layer_index);
  KheSubLayerDeleteNode(sublayer, node);
}
*** */


/*****************************************************************************/
/*                                                                           */
/*  bool KheZonePreviouslyCurrent(KHE_DEMAND_NODE_GROUP dng, KHE_ZONE zone)  */
/*                                                                           */
/*  Return true if zone is already in use in a previous restriction.         */
/*                                                                           */
/*****************************************************************************/

static bool KheZonePreviouslyCurrent(KHE_DEMAND_NODE_GROUP dng, KHE_ZONE zone)
{
  int i, index;  KHE_DEMAND_NODE_GROUP dng2;
  index = KheZoneIndex(zone);
  MArrayForEach(dng->layer_match->demand_node_groups, &dng2, &i)
  {
    if( dng2 == dng )
      return false;
    if( KheRestrictionContains(dng->curr_restriction, zone) )
      return true;
  }
  return false;
}


/*****************************************************************************/
/*                                                                           */
/*  bool KheNodePreviouslyCurrent(KHE_DEMAND_NODE_GROUP dng,                 */
/*    KHE_NODE tl_node)                                                      */
/*                                                                           */
/*  Return true if tl_node is already in use in a previous restriction.      */
/*                                                                           */
/*****************************************************************************/

/* ***
static bool KheNodePreviouslyCurrent(KHE_DEMAND_NODE_GROUP dng,
  KHE_NODE tl_node)
{
  int i, index;  KHE_DEMAND_NODE_GROUP dng2;
  index = KheNodeIndex(tl_node);
  MArrayForEach(dng->layer_match->demand_node_groups, &dng2, &i)
  {
    if( dng2 == dng )
      return false;
    if( KheRestrictionContains(dng->curr_restriction, tl_node) )
      return true;
  }
  return false;
}
*** */


/*****************************************************************************/
/*                                                                           */
/*  int KheDemandNodeGroupZoneIrregularity(KHE_DEMAND_NODE_GROUP dng,        */
/*    KHE_ZONE zone)                                                         */
/*                                                                           */
/*  Return the irregularity of zone in dng.                                  */
/*                                                                           */
/*****************************************************************************/

static int KheDemandNodeGroupZoneIrregularity(KHE_DEMAND_NODE_GROUP dng,
  KHE_ZONE zone)
{
  return KheZoneMeetOffsetCount(zone) +
    (KheZonePreviouslyCurrent(dng, zone) ? 0 : 10);
}


/*****************************************************************************/
/*                                                                           */
/*  int KheDemandNodeGroupNodeIrregularity(KHE_DEMAND_NODE_GROUP dng,        */
/*    KHE_NODE tl_node)                                                      */
/*                                                                           */
/*  Return the irregularity of tl_node in dng.                               */
/*                                                                           */
/*****************************************************************************/

/* ***
static int KheDemandNodeGroupNodeIrregularity(KHE_DEMAND_NODE_GROUP dng,
  KHE_NODE tl_node)
{
  return KheNodeDuration(tl_node) +
    (KheNodePreviouslyCurrent(dng, tl_node) ? 0 : 10);
}
*** */


/*****************************************************************************/
/*                                                                           */
/*  void KheDemandNodeGroupReset(KHE_DEMAND_NODE_GROUP dng)                  */
/*                                                                           */
/*  Rest dng's node set to contain all its nodes.                            */
/*                                                                           */
/*****************************************************************************/

static void KheDemandNodeGroupResetCurrRestriction(KHE_DEMAND_NODE_GROUP dng)
{
  KHE_ZONE zone;  int i;
  KheRestrictionClear(dng->curr_restriction);
  MArrayForEach(dng->zones, &zone, &i)
    KheRestrictionPushZone(dng->curr_restriction, zone,
      KheDemandNodeGroupZoneIrregularity(dng, zone));
  KheDemandNodeGroupNotifyDirty(dng);
}


/*****************************************************************************/
/*                                                                           */
/*  int KheZoneDecreasingDurationCmp(const void *t1, const void *t2)         */
/*                                                                           */
/*  Comparison function for sorting an array of nodes by decreasing durn.    */
/*                                                                           */
/*****************************************************************************/

static int KheZoneDecreasingDurationCmp(const void *t1, const void *t2)
{
  KHE_ZONE zone1 = * (KHE_ZONE *) t1;
  KHE_ZONE zone2 = * (KHE_ZONE *) t2;
  return KheZoneMeetOffsetCount(zone2) - KheZoneMeetOffsetCount(zone1);
}


/*****************************************************************************/
/*                                                                           */
/*  void KheDemandNodeGroupRotateLeft(KHE_DEMAND_NODE_GROUP dng,             */
/*    int i, int j)                                                          */
/*                                                                           */
/*  Rotate dng->zones[i .. j-1] one place to the left.                       */
/*                                                                           */
/*****************************************************************************/

static void KheDemandNodeGroupRotateLeft(KHE_DEMAND_NODE_GROUP dng,
  int i, int j)
{
  KHE_ZONE tmp;  int k;
  tmp = MArrayGet(dng->zones, i);
  for( k = i + 1;  k < j;  k++ )
    MArrayPut(dng->zones, k-1, MArrayGet(dng->zones, k));
  MArrayPut(dng->zones, k-1, tmp);
}


/*****************************************************************************/
/*                                                                           */
/*  void KheDemandNodeGroupRotateZones(KHE_DEMAND_NODE_GROUP dng,            */
/*    int i, int j, int r)                                                   */
/*                                                                           */
/*  Rotate dng->zones[i .. j-1] r places to the left.                        */
/*                                                                           */
/*****************************************************************************/

static void KheDemandNodeGroupRotateZones(KHE_DEMAND_NODE_GROUP dng,
  int i, int j, int r)
{
  int k;
  for( k = 0;  k < r;  k++ )
    KheDemandNodeGroupRotateLeft(dng, i, j);
}


/*****************************************************************************/
/*                                                                           */
/*  void KheDemandNodeGroupDiversify(KHE_DEMAND_NODE_GROUP dng)              */
/*                                                                           */
/*  Diversify dng by diversifing its zones.                                  */
/*                                                                           */
/*****************************************************************************/

static void KheDemandNodeGroupDiversify(KHE_DEMAND_NODE_GROUP dng)
{
  KHE_SOLN soln;  int i, j;  KHE_ZONE zonei, zonej;
  if( MArraySize(dng->zones) > 0 )
  {
    MArraySort(dng->zones, &KheZoneDecreasingDurationCmp);
    soln = KheLayerSoln(dng->layer_match->layer);
    for( i = 0;  i < MArraySize(dng->zones);  i = j )
    {
      zonei = MArrayGet(dng->zones, i);
      for( j = i + 1;  j < MArraySize(dng->zones);  j++ )
      {
	zonej = MArrayGet(dng->zones, j);
	if( KheZoneMeetOffsetCount(zonej) != KheZoneMeetOffsetCount(zonei) )
	  break;
      }
      if( DEBUG8 )
	fprintf(stderr, "  diversifier %d given %d choices: %d\n",
	  KheSolnDiversifier(soln), (j - i), KheSolnDiversifier(soln)%(j - i));
      KheDemandNodeGroupRotateZones(dng, i, j,
	KheSolnDiversifierChoose(soln, j - i));
    }
  }
}

/* ***
static void KheDemandNodeGroupDiversify(KHE_DEMAND_NODE_GROUP dng)
{
  KHE_SUBLAYER sublayer;  int i;
  MArrayForEach(dng->tl_sublayers, &sublayer, &i)
    KheSubLayerDiversify(sublayer);
}
*** */


/*****************************************************************************/
/*                                                                           */
/*  int KheDemandNodeGroupCmp(const void *t1, const void *t2)                */
/*                                                                           */
/*  Comparison function for sorting an array of demand node groups by        */
/*  decreasing duration, then increasing number of child nodes.              */
/*                                                                           */
/*****************************************************************************/

static int KheDemandNodeGroupCmp(const void *t1, const void *t2)
{
  KHE_DEMAND_NODE_GROUP dng1 = * (KHE_DEMAND_NODE_GROUP *) t1;
  KHE_DEMAND_NODE_GROUP dng2 = * (KHE_DEMAND_NODE_GROUP *) t2;
  if( KheNodeDuration(dng1->node) != KheNodeDuration(dng2->node) )
    return KheNodeDuration(dng2->node) - KheNodeDuration(dng1->node);
  else if( KheNodeChildCount(dng1->node) != KheNodeChildCount(dng2->node) )
    return KheNodeChildCount(dng1->node) - KheNodeChildCount(dng2->node);
  else
    return KheNodeIndex(dng1->node) - KheNodeIndex(dng2->node);
}


/*****************************************************************************/
/*                                                                           */
/*  Submodule "demand nodes" (private)                                       */
/*                                                                           */
/*****************************************************************************/

/*****************************************************************************/
/*                                                                           */
/*  KHE_DEMAND_NODE KheDemandNodeMake(KHE_LAYER_MATCH lm, KHE_MEET meet,     */
/*    KHE_DEMAND_NODE_GROUP dng)                                             */
/*                                                                           */
/*  Make a new demand node with these attributes and add it to dng.          */
/*                                                                           */
/*****************************************************************************/

static KHE_DEMAND_NODE KheDemandNodeMake(KHE_DEMAND_NODE_GROUP dng,
  KHE_MEET meet)
{
  KHE_DEMAND_NODE res;
  MMake(res);
  res->demand_node_group = dng;
  res->meet = meet;
  res->wmatch_node = NULL;  /* added later */
  res->assigned_supply_node = NULL;
  KheDemandNodeGroupAddDemandNode(dng, res);
  return res;
}


/*****************************************************************************/
/*                                                                           */
/*  void KheDemandNodeDelete(KHE_DEMAND_NODE dn)                             */
/*                                                                           */
/*  Delete dn, including deleting it from its demand node group.             */
/*                                                                           */
/*****************************************************************************/

static void KheDemandNodeDelete(KHE_DEMAND_NODE dn)
{
  KheDemandNodeGroupDeleteDemandNode(dn->demand_node_group, dn);
  if( dn->wmatch_node != NULL )
    KheWMatchDemandNodeDelete(dn->wmatch_node);
  MFree(dn);
}


/*****************************************************************************/
/*                                                                           */
/*  int KheDemandNodeCmp(const void *t1, const void *t2)                     */
/*                                                                           */
/*  Comparison function for sorting demand nodes such that those with        */
/*  assigned meets come first, then by increasing domain size (counting      */
/*  automatic domains as having size 1), then by decreasing demand.          */
/*                                                                           */
/*****************************************************************************/

static int KheDemandNodeCmp(const void *t1, const void *t2)
{
  KHE_DEMAND_NODE dn1 = * (KHE_DEMAND_NODE *) t1;
  KHE_DEMAND_NODE dn2 = * (KHE_DEMAND_NODE *) t2;
  KHE_MEET meet1 = dn1->meet;
  KHE_MEET meet2 = dn2->meet;
  int count1, count2;
  if( (KheMeetAsst(meet1) == NULL) != (KheMeetAsst(meet2) == NULL) )
    return KheMeetAsst(meet1) != NULL ? -1 : 1;
  count1 = KheMeetDomain(meet1) == NULL ? 1 :
    KheTimeGroupTimeCount(KheMeetDomain(meet1));
  count2 = KheMeetDomain(meet2) == NULL ? 1 :
    KheTimeGroupTimeCount(KheMeetDomain(meet2));
  if( count1 != count2 )
    return count1 - count2;
  else if( KheMeetDemand(meet2) != KheMeetDemand(meet1) )
    return KheMeetDemand(meet2) - KheMeetDemand(meet1);
  else
    return KheMeetIndex(meet1) - KheMeetIndex(meet2);
}


/*****************************************************************************/
/*                                                                           */
/*  bool KheDemandNodeEdgeFn(void *demand_back, void *supply_back,           */
/*    KHE_COST *cost)                                                        */
/*                                                                           */
/*  Edge function called by wmatch to find out what edges there are.         */
/*                                                                           */
/*****************************************************************************/
static void KheDemandNodeDebug(KHE_DEMAND_NODE dn, int verbosity,
  int indent, FILE *fp);

static bool KheDemandNodeEdgeFn(void *demand_back, void *supply_back,
  KHE_COST *cost)
{
  KHE_SOLN soln;  bool res;
  KHE_DEMAND_NODE dn = (KHE_DEMAND_NODE) demand_back;
  KHE_SUPPLY_NODE sn = (KHE_SUPPLY_NODE) supply_back;
  KHE_DEMAND_NODE_GROUP dng = dn->demand_node_group;
  KHE_SUPPLY_NODE_GROUP sng = sn->supply_node_group;
  if( DEBUG6 )
  {
    fprintf(stderr, "  KheDemandNodeEdgeFn(");
    KheDemandNodeDebug(dn, 1, -1, stderr);
    fprintf(stderr, ", ");
    KheSupplyNodeDebug(sn, 1, -1, stderr);
    fprintf(stderr, " = ");
  }
  if( cost == NULL )
  {
    if( sn->duration != KheMeetDuration(dn->meet) )
    {
      if( DEBUG6 )
	fprintf(stderr, "false (durns)\n");
      return false;
    }
    else if( KheMeetAsst(dn->meet) != NULL )
    {
      res = sng->target_meet == KheMeetAsst(dn->meet) &&
	sn->target_offset == KheMeetAsstOffset(dn->meet);
      if( DEBUG6 )
	fprintf(stderr, "%s (asst)\n", res ? "true" : "false");
      return res;
    }
    else if( sn->zone_index_set != NULL &&
      LSetDisjoint(sn->zone_index_set, dng->curr_restriction->lset) )
    {
      if( DEBUG6 )
	fprintf(stderr, "false (lset)\n");
      return false;
    }
    else
    {
      res = KheMeetAssignCheck(dn->meet, sng->target_meet, sn->target_offset);
      if( DEBUG6 )
	fprintf(stderr, "%s (asst check)\n", res ? "true" : "false");
      return res;
    }
  }
  else
  {
    *cost = 0L;
    if( sn->duration != KheMeetDuration(dn->meet) )
    {
      if( DEBUG6 )
	fprintf(stderr, "false (durn)\n");
      return false;
    }
    else if( KheMeetAsst(dn->meet) != NULL )
    {
      res = sng->target_meet == KheMeetAsst(dn->meet) &&
	sn->target_offset == KheMeetAsstOffset(dn->meet);
      if( DEBUG6 )
	fprintf(stderr, "%s (asst) cost 0\n", res ? "true" : "false");
      return res;
    }
    else if( sn->zone_index_set != NULL &&
      LSetDisjoint(sn->zone_index_set, dng->curr_restriction->lset) )
    {
      if( DEBUG6 )
	fprintf(stderr, "false (lset)\n");
      return false;
    }
    else
    {
      soln = KheMeetSoln(dn->meet);
      if( KheMeetAssign(dn->meet, sng->target_meet, sn->target_offset) )
      {
	*cost = KheSolnCost(soln);
	if( KheNodeChildCount(dn->demand_node_group->node) == 0 )
	  *cost *= CHILDLESS_MULTIPLIER;
	KheMeetUnAssign(dn->meet);
	if( DEBUG6 )
	  fprintf(stderr, "true (cost %.4f%s)\n", KheCostShow(*cost),
	    KheNodeChildCount(dn->demand_node_group->node) == 0 ?
	    " childless" : "");
	return true;
      }
      else
      {
	if( DEBUG6 )
	  fprintf(stderr, "false (asst check)\n");
	return false;
      }
    }
  }
}

/* ***
static bool KheDemandNodeEdgeFn(void *demand_back, void *supply_back,
  KHE_COST *cost)
{
  KHE_SOLN soln;  bool res;
  KHE_DEMAND_NODE dn = (KHE_DEMAND_NODE) demand_back;
  KHE_SUPPLY_NODE sn = (KHE_SUPPLY_NODE) supply_back;
  KHE_DEMAND_NODE_GROUP dng = dn->demand_node_group;
  KHE_SUPPLY_NODE_GROUP sng = sn->supply_node_group;
  if( DEBUG6 )
  {
    fprintf(stderr, "  KheDemandNodeEdgeFn(");
    KheDemandNodeDebug(dn, 1, -1, stderr);
    fprintf(stderr, ", ");
    KheSupplyNodeDebug(sn, 1, -1, stderr);
    fprintf(stderr, " = ");
  }
  if( cost == NULL )
  {
    if( sn->duration != KheMeetDuration(dn->meet) )
    {
      if( DEBUG6 )
	fprintf(stderr, "false (durns)\n");
      return false;
    }
    else if( sn->tl_node_index_set != NULL &&
      LSetDisjoint(sn->tl_node_index_set, dng->curr_restriction->lset) )
    {
      if( DEBUG6 )
	fprintf(stderr, "false (lset)\n");
      return false;
    }
    else if( KheMeetAsst(dn->meet) != NULL )
    {
      res = sng->target_meet == KheMeetAsst(dn->meet) &&
	sn->target_offset == KheMeetAsstOffset(dn->meet);
      if( DEBUG6 )
	fprintf(stderr, "%s (asst)\n", res ? "true" : "false");
      return res;
    }
    else
    {
      res = KheMeetAssignCheck(dn->meet, sng->target_meet, sn->target_offset);
      if( DEBUG6 )
	fprintf(stderr, "%s (asst check)\n", res ? "true" : "false");
      return res;
    }
  }
  else
  {
    *cost = 0L;
    if( sn->duration != KheMeetDuration(dn->meet) )
    {
      if( DEBUG6 )
	fprintf(stderr, "false (durn)\n");
      return false;
    }
    else if( sn->tl_node_index_set != NULL &&
      LSetDisjoint(sn->tl_node_index_set, dng->curr_restriction->lset) )
    {
      if( DEBUG6 )
	fprintf(stderr, "false (lset)\n");
      return false;
    }
    else if( KheMeetAsst(dn->meet) != NULL )
    {
      res = sng->target_meet == KheMeetAsst(dn->meet) &&
	sn->target_offset == KheMeetAsstOffset(dn->meet);
      if( DEBUG6 )
	fprintf(stderr, "%s (asst) cost 0\n", res ? "true" : "false");
      return res;
    }
    else
    {
      soln = KheMeetSoln(dn->meet);
      if( KheMeetAssign(dn->meet, sng->target_meet, sn->target_offset) )
      {
	*cost = KheSolnCost(soln);
	if( KheNodeChildCount(dn->demand_node_group->node) == 0 )
	  *cost *= CHILDLESS_MULTIPLIER;
	KheMeetUnAssign(dn->meet);
	if( DEBUG6 )
	  fprintf(stderr, "true (cost %.4f%s)\n", KheCostShow(*cost),
	    KheNodeChildCount(dn->demand_node_group->node) == 0 ?
	    " childless" : "");
	return true;
      }
      else
      {
	if( DEBUG6 )
	  fprintf(stderr, "false (asst check)\n");
	return false;
      }
    }
  }
}
*** */


/*****************************************************************************/
/*                                                                           */
/*  void KheDemandNodeDebug(KHE_DEMAND_NODE dn, int verbosity,               */
/*    int indent, FILE *fp)                                                  */
/*                                                                           */
/*  Debug print of dn onto fp with the given verbosity and indent.           */
/*                                                                           */
/*****************************************************************************/

static void KheDemandNodeDebug(KHE_DEMAND_NODE dn, int verbosity,
  int indent, FILE *fp)
{
  if( verbosity >= 1 )
  {
    if( indent >= 0 )
      fprintf(fp, "%*s", indent, "");
    fprintf(fp, "N%d:", KheNodeIndex(dn->demand_node_group->node));
    KheMeetDebug(dn->meet, 1, -1, fp);
    if( indent >= 0 )
      fprintf(fp, "\n");
  }
}


/*****************************************************************************/
/*                                                                           */
/*  Submodule "regularity cost"                                              */
/*                                                                           */
/*****************************************************************************/

/*****************************************************************************/
/*                                                                           */
/*  void KheRegularityCostInitToLarge(KHE_REGULARITY_COST *rc,               */
/*    int infeasibility)                                                     */
/*                                                                           */
/*  Assign *rc an initial value which has the given infeasibility and is     */
/*  otherwise very large.                                                    */
/*                                                                           */
/*****************************************************************************/

static void KheRegularityCostInitToLarge(KHE_REGULARITY_COST *rc,
  int infeasibility)
{
  rc->infeasibility = infeasibility;
  rc->without_children_cost = KheCostMax;
  rc->zones_cost = INT_MAX;
  rc->with_children_cost = KheCostMax;
}


/*****************************************************************************/
/*                                                                           */
/*  bool KheRegularityCostLessThan(KHE_REGULARITY_COST *rc1,                 */
/*    KHE_REGULARITY_COST *rc2)                                              */
/*                                                                           */
/*  Return true if rc1 < rc2.                                                */
/*                                                                           */
/*****************************************************************************/

static bool KheRegularityCostLessThan(KHE_REGULARITY_COST *rc1,
  KHE_REGULARITY_COST *rc2)
{
  if( rc1->infeasibility != rc2->infeasibility )
    return rc1->infeasibility < rc2->infeasibility;
  else if( rc1->without_children_cost != rc2->without_children_cost )
    return rc1->without_children_cost < rc2->without_children_cost;
  else if( rc1->zones_cost != rc2->zones_cost )
    return rc1->zones_cost < rc2->zones_cost;
  else
    return rc1->with_children_cost < rc2->with_children_cost;
}


/*****************************************************************************/
/*                                                                           */
/*  void KheRegularityCostDebug(KHE_REGULARITY_COST *rc, int verbosity,      */
/*    int indent, FILE *fp)                                                  */
/*                                                                           */
/*  Debug print of *rc onto fp with the given verbosity and indent.          */
/*                                                                           */
/*****************************************************************************/

static void KheRegularityCostDebug(KHE_REGULARITY_COST *rc, int verbosity,
  int indent, FILE *fp)
{
  if( verbosity >= 1 )
  {
    if( indent >= 0 )
      fprintf(fp, "%*s", indent, "");
    fprintf(fp, "(%d, %.4f, %d, %.4f)", rc->infeasibility,
      KheCostShow(rc->without_children_cost), rc->zones_cost,
      KheCostShow(rc->with_children_cost));
    if( indent >= 0 )
      fprintf(fp, "\n");
  }
}


/*****************************************************************************/
/*                                                                           */
/*  Submodule "layer matching construction"                                  */
/*                                                                           */
/*****************************************************************************/

/*****************************************************************************/
/*                                                                           */
/*  void KheLayerMatchCostDebug(KHE_COST cost, FILE *fp)                     */
/*                                                                           */
/*  Debug print of cost onto fp.                                             */
/*                                                                           */
/*****************************************************************************/

static void KheLayerMatchCostDebug(KHE_COST cost, FILE *fp)
{
  fprintf(fp, "%.4f", KheCostShow(cost));
}


/*****************************************************************************/
/*                                                                           */
/*  void KheLayerMatchEnsureSupply(KHE_LAYER_MATCH lm, KHE_DEMAND_NODE dn)   */
/*                                                                           */
/*  Try to ensure that there is a supply node for assigned dn in lm.         */
/*                                                                           */
/*****************************************************************************/

static void KheLayerMatchEnsureSupply(KHE_LAYER_MATCH lm, KHE_DEMAND_NODE dn)
{
  KHE_SUPPLY_NODE_GROUP sng;  KHE_SUPPLY_NODE sn, lsn, rsn;
  KHE_MEET target_meet;  int target_offset, durn, i, j;

  MAssert(KheMeetAsst(dn->meet) != NULL,
    "KheLayerMatchEnsureSupply: no target");
  target_meet = KheMeetAsst(dn->meet);
  target_offset = KheMeetAsstOffset(dn->meet);
  durn = KheMeetDuration(dn->meet);

  MArrayForEach(lm->supply_node_groups, &sng, &i)
    if( sng->target_meet == target_meet )
      MArrayForEach(sng->supply_nodes, &sn, &j)
	if( sn->assigned_demand_node == NULL &&
	    sn->target_offset <= target_offset &&
	    sn->target_offset + sn->duration >= target_offset + durn )
	{
	  /* split sn and record the fact that dn is assigned to sn */
	  KheSupplyNodeSplit(sn, target_offset, durn, &lsn, &rsn);
	  sn->assigned_demand_node = dn;
	  dn->assigned_supply_node = sn;
	  return;
	}
}


/*****************************************************************************/
/*                                                                           */
/*  bool KheLayerMatchSplitInTwo(KHE_SUPPLY_NODE sn, int offset, int durn)   */
/*                                                                           */
/*  Return true if splitting sn at this offset would split it into two,      */
/*  not three fragments.  The fragment has duration durn.                    */
/*                                                                           */
/*****************************************************************************/

static bool KheLayerMatchSplitInTwo(KHE_SUPPLY_NODE sn, int offset, int durn)
{
  return offset == sn->target_offset ||
    offset == sn->target_offset + sn->duration - durn;
}


/*****************************************************************************/
/*                                                                           */
/*  bool KheLayerMatchSplitInTemplate(KHE_SUPPLY_NODE sn, int offset,        */
/*    int durn)                                                              */
/*                                                                           */
/*  Return true if splitting sn at this offset would be regular with the     */
/*  template layer.  The fragment has duration durn.                         */
/*                                                                           */
/*****************************************************************************/

/* *** old version uses template layers 
static bool KheLayerMatchSplitInTemplate(KHE_SUPPLY_NODE sn, int offset,
  int durn)
{
  int to, i;  KHE_SUPPLY_NODE_GROUP sng;
  sng = sn->supply_node_group;
  MArrayForEach(sng->template_offsets, &to, &i)
    if( to == offset && MArrayGet(sng->template_durations, i) == durn )
      return true;
  return false;
}
*** */


/*****************************************************************************/
/*                                                                           */
/*  bool KheLayerMatchSplitHasZoneType(KHE_SUPPLY_NODE sn, int offset,       */
/*    int durn, KHE_ZONE_TYPE zone_type)                                     */
/*    int durn)                                                              */
/*                                                                           */
/*  Return true if splitting sn at this offset would produce the given       */
/*  zone type.  The fragment has duration durn.                              */
/*                                                                           */
/*****************************************************************************/

typedef enum {
  KHE_ZONE_EXACT,
  KHE_ZONE_INEXACT,
  KHE_ZONE_IGNORE,
} KHE_ZONE_TYPE;

static bool KheLayerMatchSplitHasZoneType(KHE_SUPPLY_NODE sn, int offset,
  int durn, KHE_ZONE_TYPE zone_type)
{
  KHE_ZONE zone;  KHE_MEET m;  int i;
  switch( zone_type )
  {
    case KHE_ZONE_EXACT:

      /* ensure sn[offset .. offset + durn -1] covers a single zone */
      m = sn->supply_node_group->target_meet;
      zone = KheMeetOffsetZone(m, offset);
      for( i = 1;  i < durn;  i++ )
	if( KheMeetOffsetZone(m, offset + i) != zone )
	  return false;

      /* if there is a preceding zone, it must be different */
      if( offset > 0 && KheMeetOffsetZone(m, offset - 1) == zone )
	return false;

      /* if there is a following zone, it must be different */
      if( offset + durn < KheMeetDuration(m) &&
          KheMeetOffsetZone(m, offset + durn) == zone )
	return false;

      /* all in order */
      return true;

    case KHE_ZONE_INEXACT:

      /* return true if not exact */
      return !KheLayerMatchSplitHasZoneType(sn, offset, durn, KHE_ZONE_EXACT);

    case KHE_ZONE_IGNORE:

      /* ignore zones and return true */
      return true;

    default:

      MAssert(false, "KheLayerMatchSplitHasZoneType internal error");
      return false;  /* keep compiler happy */
  }
}


/*****************************************************************************/
/*                                                                           */
/*  char *KheZoneTypeShow(KHE_ZONE_TYPE zone_type)                           */
/*                                                                           */
/*  Brief display of a zone type.                                            */
/*                                                                           */
/*****************************************************************************/

static char *KheZoneTypeShow(KHE_ZONE_TYPE zone_type)
{
  switch( zone_type )
  {
    case KHE_ZONE_EXACT:	return "zn+";
    case KHE_ZONE_INEXACT:	return "zn-";
    case KHE_ZONE_IGNORE:	return "---";

    default:

      MAssert(false, "KheZoneTypeShow internal error");
      return NULL;  /* keep compiler happy */
  }
}


/*****************************************************************************/
/*                                                                           */
/*  void TrySplit(KHE_SUPPLY_NODE sn, int offset, int durn,                  */
/*    bool in_two, KHE_ZONE_TYPE zone_type, int pre_infeas,                  */
/*    KHE_SUPPLY_NODE *best_sn, int *best_offset, KHE_COST *best_cost,       */
/*    int *best_unevenness)                                                  */
/*                                                                           */
/*  Try splitting sn at offset and durn, but only if in_two and in_one_zone  */
/*  allow.  Update *best_sn, *best_offset, *best_cost, and *best_unevenness  */
/*  if new best.                                                             */
/*                                                                           */
/*****************************************************************************/

static void TrySplit(KHE_SUPPLY_NODE sn, int offset, int durn,
  bool in_two, KHE_ZONE_TYPE zone_type, int pre_infeas,
  KHE_SUPPLY_NODE *best_sn, int *best_offset, KHE_COST *best_cost,
  int *best_unevenness)
{
  KHE_SUPPLY_NODE lsn, rsn;  KHE_COST cost;  int post_infeas, unevenness;

  if( in_two == KheLayerMatchSplitInTwo(sn, offset, durn) &&
      KheLayerMatchSplitHasZoneType(sn, offset, durn, zone_type) )
  {
    /* try splitting sn at offset */
    if( DEBUG10 )
    {
      fprintf(stderr, "  try %s %s split ", in_two ? "in2" : "in3",
        KheZoneTypeShow(zone_type));
      KheSupplyNodeDebug(sn, 1, -1, stderr);
      fprintf(stderr, " at offset %d: ", offset);
    }
    KheSupplyNodeSplit(sn, offset, durn, &lsn, &rsn);
    KheWMatchEval(sn->supply_node_group->layer_match->wmatch,
      &post_infeas, &cost);
    unevenness = sn->supply_node_group->layer_match->unevenness;
    if( post_infeas < pre_infeas && (cost < *best_cost ||
        (cost == *best_cost && unevenness < *best_unevenness)) )
    {
      *best_sn = sn;
      *best_offset = offset;
      *best_cost = cost;
      *best_unevenness = unevenness;
      if( DEBUG10 )
	fprintf(stderr, "new best (%.4f, %d)\n", KheCostShow(cost),
	  unevenness);
    }
    else
    {
      if( DEBUG10 )
      {
	if( post_infeas >= pre_infeas )
	  fprintf(stderr, "infeasible\n");
	else
	  fprintf(stderr, "uncompet (%.4f, %d)\n", KheCostShow(cost),
	    unevenness);
      }
    }
    KheSupplyNodeMerge(lsn, sn, rsn);
  }
}


/*****************************************************************************/
/*                                                                           */
/*  bool KheLayerMatchRepair(KHE_LAYER_MATCH lm, KHE_DEMAND_NODE dn,         */
/*    bool in_two, KHE_ZONE_TYPE zone_type, int shift)                       */
/*                                                                           */
/*  Try to repair lm so that dn can match, returning true if successful.     */
/*  The repair method is to try each possible way in which a supply node     */
/*  can be split into two or three fragments, one of which has the same      */
/*  duration as dn, and to retain the best that matches.                     */
/*                                                                           */
/*  If in_two is true, the split must be into two fragments, otherwise it    */
/*  must be in three.  If in_one_zone is true, the split must lie within     */
/*  one zone, otherwise it must not.  Parameter shift is used to vary the    */
/*  starting point of the repair.                                            */
/*                                                                           */
/*****************************************************************************/

static bool KheLayerMatchRepair(KHE_LAYER_MATCH lm, KHE_DEMAND_NODE dn,
  bool in_two, KHE_ZONE_TYPE zone_type, int shift)
{
  KHE_SUPPLY_NODE_GROUP sng;  KHE_SUPPLY_NODE sn, best_sn, lsn, rsn;
  KHE_COST cost, best_cost;
  int i, j, durn, offset, best_offset, best_unevenness, pre_infeas, sngs;

  /* ***
  if( DEBUG10 )
    fprintf(stderr, "[ KheLayerMatchRepair(lm, dn (durn %d), %s, %s, %d)\n",
      KheMeetDuration(dn->meet), in_two ? "true" : "false",
      KheZoneTypeShow(zone_type), shift);
  *** */

  /* try all the possible repairs and remember the best */
  KheWMatchEval(lm->wmatch, &pre_infeas, &cost);
  durn = KheMeetDuration(dn->meet);
  best_cost = KheCostMax;
  best_sn = NULL;
  best_offset = -1;
  best_unevenness = INT_MAX;
  sngs = MArraySize(lm->supply_node_groups);
  for( i = 0;  i < MArraySize(lm->supply_node_groups);  i++ )
  {
    sng = MArrayGet(lm->supply_node_groups, (i + shift) % sngs);
    MArrayForEach(sng->supply_nodes, &sn, &j)
      if( sn->assigned_demand_node == NULL && durn < sn->duration )
      {
	/* split from the back if durn is 1, from the front otherwise */
	if( durn == 1 )
	{
	  for( offset = sn->target_offset + sn->duration - durn;
	       offset >= sn->target_offset;  offset-- )
	    TrySplit(sn, offset, durn, in_two, zone_type, pre_infeas,
	      &best_sn, &best_offset, &best_cost, &best_unevenness);
	}
	else
	{
	  for( offset = sn->target_offset;
	       offset + durn <= sn->target_offset + sn->duration;  offset++ )
	    TrySplit(sn, offset, durn, in_two, zone_type, pre_infeas,
	      &best_sn, &best_offset, &best_cost, &best_unevenness);
	}
      }
  }

  /* make the best assignment, if any */
  if( best_sn != NULL )
  {
    KheSupplyNodeSplit(best_sn, best_offset, durn, &lsn, &rsn);
    if( DEBUG10 )
    {
      fprintf(stderr, "  split ");
      KheSupplyNodeDebug(best_sn, 1, -1, stderr);
      fprintf(stderr, " at offset %d cost (%.4f, %d)\n", best_offset,
	KheCostShow(KheLayerMatchCost(lm)), best_unevenness);
      /* fprintf(stderr, "] KheLayerMatchRepair returning true\n"); */
    }
    return true;
  }
  else
  {
    /* ***
    if( DEBUG10 )
      fprintf(stderr, "] KheLayerMatchRepair returning false\n");
    *** */
    return false;
  }
}


/*****************************************************************************/
/*                                                                           */
/*  void KheLayerMatchAddDemandNodeToWMatch(KHE_LAYER_MATCH lm,              */
/*    KHE_DEMAND_NODE dn, int shift)                                         */
/*                                                                           */
/*  Add dn and its edges to the matching graph, and try to repair if         */
/*  necessary.  Use shift to vary the starting position of the repair.       */
/*                                                                           */
/*****************************************************************************/

static void KheLayerMatchAddDemandNodeToWMatch(KHE_LAYER_MATCH lm,
  KHE_DEMAND_NODE dn, int shift)
{
  KHE_COST cost;  int pre_infeas, post_infeas;  bool junk;
  KheWMatchEval(lm->wmatch, &pre_infeas, &cost);
  dn->wmatch_node = KheWMatchDemandNodeMake(lm->wmatch, dn,
    dn->demand_node_group->wmatch_category, 1);
  KheWMatchEval(lm->wmatch, &post_infeas, &cost);
  if( DEBUG1 )
  {
    fprintf(stderr, "[ KheAddDemand(lm, ");
    KheMeetDebug(dn->meet, 1, -1, stderr);
    fprintf(stderr, ", %d) pre %d post %d\n", shift, pre_infeas, post_infeas);
  }
  if( KheMeetAsst(dn->meet) != NULL )
    KheLayerMatchEnsureSupply(lm, dn);
  else
  {
    if( post_infeas > pre_infeas )
    {
      if( KheNodeZoneCount(KheLayerParentNode(lm->layer)) > 0 )
      {
	/* have zones, so try first to obey them, then not */
	junk = KheLayerMatchRepair(lm, dn, true, KHE_ZONE_EXACT, shift) ||
	  KheLayerMatchRepair(lm, dn, true, KHE_ZONE_INEXACT, shift) ||
	  KheLayerMatchRepair(lm, dn, false, KHE_ZONE_EXACT, shift) ||
	  KheLayerMatchRepair(lm, dn, false, KHE_ZONE_INEXACT, shift);
      }
      else
      {
	/* no zones, so don't take them into accoutn when splitting */
	junk = KheLayerMatchRepair(lm, dn, true, KHE_ZONE_IGNORE, shift) ||
	  KheLayerMatchRepair(lm, dn, false, KHE_ZONE_IGNORE, shift);
      }
    }
  }
  if( DEBUG1 )
    fprintf(stderr, "]\n");
}


/*****************************************************************************/
/*                                                                           */
/*  void KheAddTemplateOffsetAndDuration(KHE_LAYER_MATCH lm,                 */
/*    KHE_MEET target_meet, int target_offset, int durn)                     */
/*                                                                           */
/*  The initial template layer contains an assignment of a meet of the       */
/*  given duration to target_meet at target_offset, so add this to the       */
/*  appropriate supply node group.                                           */
/*                                                                           */
/*****************************************************************************/

/* ***
static void KheAddTemplateOffsetAndDuration(KHE_LAYER_MATCH lm,
  KHE_MEET target_meet, int target_offset, int durn)
{
  KHE_SUPPLY_NODE_GROUP sng;  int i;
  MArrayForEach(lm->supply_node_groups, &sng, &i)
    if( sng->target_meet == target_meet )
    {
      MArrayAddLast(sng->template_offsets, target_offset);
      MArrayAddLast(sng->template_durations, durn);
      return;
    }
  MAssert(false, "KheAddTemplateOffsetAndDuration internal error");
}
*** */


/*****************************************************************************/
/*                                                                           */
/*  KHE_EVEN_TIME_GROUP FindEvenTimeGroup(KHE_LAYER_MATCH lm, KHE_MEET meet) */
/*                                                                           */
/*  Return the even time group that meet begins in, or NULL if none.         */
/*                                                                           */
/*****************************************************************************/

static KHE_EVEN_TIME_GROUP FindEvenTimeGroup(KHE_LAYER_MATCH lm, KHE_MEET meet)
{
  KHE_TIME time;  KHE_EVEN_TIME_GROUP etg;  int i;
  time = KheMeetAsstTime(meet);
  if( time == NULL )
    return NULL;
  MArrayForEach(lm->even_time_groups, &etg, &i)
    if( KheTimeGroupContains(etg->time_group, time) )
      return etg;
  return NULL;
}


/*****************************************************************************/
/*                                                                           */
/*  KHE_LAYER_MATCH KheLayerMatchMake(KHE_LAYER layer,                       */
/*    KHE_SPREAD_EVENTS_CONSTRAINT sec)                                      */
/*                                                                           */
/*  Make a new soln layer match object for layer, guided by sec if present.  */
/*                                                                           */
/*****************************************************************************/
static void KheLayerMatchDebugSegmentation(KHE_LAYER_MATCH lm,
  int verbosity, int indent, FILE *fp);

KHE_LAYER_MATCH KheLayerMatchMake(KHE_LAYER layer,
  KHE_SPREAD_EVENTS_CONSTRAINT sec)
{
  KHE_LAYER_MATCH res;  int i, j, max_durn;  KHE_SUPPLY_NODE sn, lsn, rsn;
  KHE_NODE n;  KHE_MEET meet;  KHE_DEMAND_NODE dn;  KHE_DEMAND_NODE_GROUP dng;
  KHE_SUPPLY_NODE_GROUP sng;  KHE_TIME_SPREAD time_spread;
  KHE_LIMITED_TIME_GROUP ltg;  KHE_TIME_GROUP tg;  KHE_EVEN_TIME_GROUP etg;

  if( DEBUG1 || DEBUG3 )
  {
    fprintf(stderr, "[ KheLayerMatchMake(");
    KheLayerDebug(layer, 1, -1, stderr);
    fprintf(stderr, "%s)\n", sec != NULL ? "sec" : "-");
  }

  /* build the initial object */
  MMake(res);
  res->layer = layer;
  res->constraint = sec;
  MArrayInit(res->supply_node_groups);
  MArrayInit(res->demand_node_groups);
  MArrayInit(res->demand_nodes);
  MArrayInit(res->even_time_groups);
  res->unevenness = 0;
  res->wmatch = KheWMatchMake(layer, (GENERIC_DEBUG_FN) &KheLayerDebug,
    (GENERIC_DEBUG_FN) &KheDemandNodeDebug,
    (GENERIC_DEBUG_FN) &KheSupplyNodeDebug,
    &KheDemandNodeEdgeFn, &KheLayerMatchCostDebug, KheCost(1, 0));
  KheRegularityCostInitToLarge(&res->best_reg_cost, 0);

  /* add one even time group for each time group of sec */
  if( sec != NULL )
  {
    time_spread = KheSpreadEventsConstraintTimeSpread(sec);
    for( i = 0;  i < KheTimeSpreadLimitedTimeGroupCount(time_spread);  i++ )
    {
      ltg = KheTimeSpreadLimitedTimeGroup(time_spread, i);
      tg = KheLimitedTimeGroupTimeGroup(ltg);
      MAssert(tg != NULL, "KheLayerMatchMake internal error");
      MArrayAddLast(res->even_time_groups, KheEvenTimeGroupMake(res, tg));
    }
  }

  /* add one supply node group and one supply node for each parent meet */
  n = KheLayerParentNode(layer);
  for( j = 0;  j < KheNodeMeetCount(n);  j++ )
  {
    meet = KheNodeMeet(n, j);
    etg = FindEvenTimeGroup(res, meet);
    sng = KheSupplyNodeGroupMake(res, etg, meet);
    MArrayAddLast(res->supply_node_groups, sng);
    KheSupplyNodeMake(sng, 0, KheMeetDuration(meet));
  }

  /* add template offsets and durations */
  /* ***
  for( i = 0;  i < KheLayerChildNodeCount(template_layer);  i++ )
  {
    n = KheLayerChildNode(template_layer, i);
    for( j = 0;  j < KheNodeMeetCount(n);  j++ )
    {
      meet = KheNodeMeet(n, j);
      if( KheMeetAsst(meet) != NULL )
      {
	KheAddTemplateOffsetAndDuration(res, KheMeetAsst(meet),
	  KheMeetAsstOffset(meet), KheMeetDuration(meet));
	if( DEBUG10 )
	{
	  fprintf(stderr, "  template ");
	  KheMeetDebug(KheMeetAsst(meet), 1, -1, stderr);
	  fprintf(stderr, "+%dd%d\n", KheMeetAsstOffset(meet),
	    KheMeetDuration(meet));
	}
      }
    }
  }
  *** */
  
  /* make one demand node group for each child node, and one demand node */
  /* for each child meet, and sort the demand nodes */
  for( i = 0;  i < KheLayerChildNodeCount(layer);  i++ )
  {
    n = KheLayerChildNode(layer, i);
    dng = KheDemandNodeGroupMake(res, n);
    MArrayAddLast(res->demand_node_groups, dng);
    for( j = 0;  j < KheNodeMeetCount(n);  j++ )
    {
      meet = KheNodeMeet(n, j);
      dn = KheDemandNodeMake(dng, meet);
      MArrayAddLast(res->demand_nodes, dn);
      if( DEBUG1 )
      {
	fprintf(stderr, "  initial meet(durn %d, %s): ",
	  KheMeetDuration(dn->meet),
	  KheMeetAsst(dn->meet) == NULL ? "unassigned" : "assigned");
	KheMeetDebug(dn->meet, 1, 0, stderr);
	if( KheMeetAsst(dn->meet) != NULL )
	  KheMeetDebug(KheMeetAsst(dn->meet), 1, 4, stderr);
      }
    }
  }
  MArraySort(res->demand_nodes, &KheDemandNodeCmp);

  /* main loop: find supply for each demand node */
  MArrayForEach(res->demand_nodes, &dn, &i)
    KheLayerMatchAddDemandNodeToWMatch(res, dn, i);

  /* further splits of oversized supply nodes */
  max_durn = 0;
  MArrayForEach(res->demand_nodes, &dn, &i)
  {
    if( DEBUG1 )
    {
      fprintf(stderr, "  final meet(durn %d, %s): ", KheMeetDuration(dn->meet),
	KheMeetAsst(dn->meet) == NULL ? "unassigned" : "assigned");
      KheMeetDebug(dn->meet, 1, 0, stderr);
      if( KheMeetAsst(dn->meet) != NULL )
        KheMeetDebug(KheMeetAsst(dn->meet), 1, 4, stderr);
    }
    if( KheMeetAsst(dn->meet) == NULL && KheMeetDuration(dn->meet) > max_durn )
      max_durn = KheMeetDuration(dn->meet);
  }
  if( max_durn > 0 )
  {
    if( DEBUG1 )
      fprintf(stderr, "  splitting to max_durn %d\n", max_durn);
    MArrayForEach(res->supply_node_groups, &sng, &i)
      MArrayForEach(sng->supply_nodes, &sn, &j)
	if( sn->duration > max_durn && sn->assigned_demand_node == NULL )
	{
	  if( DEBUG1 )
	  {
	    fprintf(stderr, "  splitting ");
	    KheSupplyNodeDebug(sn, 1, -1, stderr);
	    fprintf(stderr, " (%d > %d && !%s)\n", sn->duration, max_durn,
	      sn->assigned_demand_node != NULL ? "true" : "false");
	  }
	  KheSupplyNodeSplit(sn, sn->target_offset, max_durn, &lsn, &rsn);
	}
  }

  if( DEBUG1 || DEBUG3 )
  {
    if( DEBUG1 )
      KheLayerMatchDebug(res, 3, 2, stderr);
    if( DEBUG3 )
      KheLayerMatchDebugSegmentation(res, 1, 2, stderr);
    fprintf(stderr, "] KheLayerMatchMake returning\n");
  }
  return res;
}


/*****************************************************************************/
/*                                                                           */
/*  void KheLayerMatchAddTemplateLayer(KHE_LAYER_MATCH lm,                   */
/*    KHE_LAYER template_layer)                                              */
/*                                                                           */
/*  Add a further template layer to lm.                                      */
/*                                                                           */
/*****************************************************************************/

/* ***
void KheLayerMatchAddTemplateLayer(KHE_LAYER_MATCH lm,
  KHE_LAYER template_layer)
{
  MAssert(KheLayerParentNode(template_layer) == KheLayerParentNode(lm->layer),
    "KheLayerMatchAddTemplateLayer: template layer has wrong parent node");
  MArrayAddLast(lm->template_layers, template_layer);
}
*** */


/*****************************************************************************/
/*                                                                           */
/*  KHE_LAYER KheLayerMatchLayer(KHE_LAYER_MATCH lm)                         */
/*                                                                           */
/*  Return the soln layer that lm is for.                                    */
/*                                                                           */
/*****************************************************************************/

KHE_LAYER KheLayerMatchLayer(KHE_LAYER_MATCH lm)
{
  return lm->layer;
}


/*****************************************************************************/
/*                                                                           */
/*  KHE_SPREAD_EVENTS_CONSTRAINT KheLayerMatchConstraint(KHE_LAYER_MATCH lm) */
/*                                                                           */
/*  Return lm's spread events constraint, possibly NULL.                     */
/*                                                                           */
/*****************************************************************************/

KHE_SPREAD_EVENTS_CONSTRAINT KheLayerMatchConstraint(KHE_LAYER_MATCH lm)
{
  return lm->constraint;
}


/*****************************************************************************/
/*                                                                           */
/*  int KheLayerMatchTemplateLayerCount(KHE_LAYER_MATCH lm)                  */
/*                                                                           */
/*  Return the number of template layers in lm.                              */
/*                                                                           */
/*****************************************************************************/

/* ***
int KheLayerMatchTemplateLayerCount(KHE_LAYER_MATCH lm)
{
  return MArraySize(lm->template_layers);
}
*** */


/*****************************************************************************/
/*                                                                           */
/*  KHE_LAYER KheLayerMatchTemplateLayer(KHE_LAYER_MATCH lm, int i)          */
/*                                                                           */
/*  Return the i'th template layer of lm.                                    */
/*                                                                           */
/*****************************************************************************/

/* ***
KHE_LAYER KheLayerMatchTemplateLayer(KHE_LAYER_MATCH lm, int i)
{
  return MArrayGet(lm->template_layers, i);
}
*** */


/*****************************************************************************/
/*                                                                           */
/*  int KheLayerMatchDemandNodeCount(KHE_LAYER_MATCH lm)                     */
/*                                                                           */
/*  Return the number of demand nodes in lm.                                 */
/*                                                                           */
/*****************************************************************************/

int KheLayerMatchDemandNodeCount(KHE_LAYER_MATCH lm)
{
  return MArraySize(lm->demand_nodes);
}


/*****************************************************************************/
/*                                                                           */
/*  KHE_MEET KheLayerMatchDemandNode(KHE_LAYER_MATCH lm, int i)              */
/*                                                                           */
/*  Return the i'th demand node of lm.                                       */
/*                                                                           */
/*****************************************************************************/

KHE_MEET KheLayerMatchDemandNode(KHE_LAYER_MATCH lm, int i)
{
  return MArrayGet(lm->demand_nodes, i)->meet;
}


/*****************************************************************************/
/*                                                                           */
/*  bool KheLayerMatchFindDemandNode(KHE_LAYER_MATCH lm,                     */
/*    KHE_MEET meet, KHE_DEMAND_NODE *dn)                                    */
/*                                                                           */
/*  Find the demand node of meet in lm.                                      */
/*                                                                           */
/*****************************************************************************/

static bool KheLayerMatchFindDemandNode(KHE_LAYER_MATCH lm,
  KHE_MEET meet, KHE_DEMAND_NODE *dn)
{
  KHE_DEMAND_NODE res;  int i;
  MArrayForEach(lm->demand_nodes, &res, &i)
    if( res->meet == meet )
    {
      *dn = res;
      return true;
    }
  *dn = NULL;
  return false;
}


/*****************************************************************************/
/*                                                                           */
/*  bool KheLayerMatchBestEdge(KHE_LAYER_MATCH lm, KHE_MEET meet,            */
/*    KHE_MEET *target_meet, int *target_offset, KHE_COST *cost)             */
/*                                                                           */
/*  If meet has an edge in the maximum matching, return true and pass        */
/*  back the attributes of the edge, otherwise return false.                 */
/*                                                                           */
/*****************************************************************************/

bool KheLayerMatchBestEdge(KHE_LAYER_MATCH lm, KHE_MEET meet,
  KHE_MEET *target_meet, int *target_offset, KHE_COST *cost)
{
  KHE_DEMAND_NODE dn;  KHE_SUPPLY_NODE sn;
  if( !KheLayerMatchFindDemandNode(lm, meet, &dn) )
    MAssert(false, "KheLayerMatchBestEdge: meet is not a demand node of lm");
  sn = (KHE_SUPPLY_NODE) KheWMatchDemandNodeAssignedTo(dn->wmatch_node, cost);
  if( sn == NULL )
  {
    *target_meet = NULL;
    *target_offset = -1;
    *cost = 0;
    return false;
  }
  else
  {
    *target_meet = sn->supply_node_group->target_meet;
    *target_offset = sn->target_offset;
    return true;
  }
}


/*****************************************************************************/
/*                                                                           */
/*  bool KheLayerMatchAssignBestEdges(KHE_LAYER_MATCH lm)                    */
/*                                                                           */
/*  Make the assignments indicated by the layer matching.                    */
/*                                                                           */
/*****************************************************************************/

bool KheLayerMatchAssignBestEdges(KHE_LAYER_MATCH lm)
{
  bool res;  int i, target_offset;  KHE_COST c;  KHE_MEET meet, target_meet;
  if( DEBUG9 )
    fprintf(stderr, "[ KheLayerMatchAssignBestEdges(lm)\n");
  res = true;
  for( i = 0;  i < KheLayerMatchDemandNodeCount(lm);  i++ )
  {
    meet = KheLayerMatchDemandNode(lm, i);
    if( KheMeetAsst(meet) == NULL &&
	KheLayerMatchBestEdge(lm, meet, &target_meet, &target_offset, &c) )
    {
      if( !KheMeetAssign(meet, target_meet, target_offset) )
	MAssert(false, "KheLayerMatchMakeBestAssignments internal error");
      if( DEBUG9 )
      {
	fprintf(stderr, "  KheMeetAssign(");
	KheMeetDebug(meet, 1, -1, stderr);
	fprintf(stderr, ", ");
	KheMeetDebug(target_meet, 1, -1, stderr);
	fprintf(stderr, ", %d)\n", target_offset);
      }
    }
    if( KheMeetAsst(meet) == NULL )
    {
      if( DEBUG9 )
      {
	fprintf(stderr, "  no best edge for ");
	KheMeetDebug(meet, 1, -1, stderr);
	fprintf(stderr, "\n");
      }
      res = false;
    }
  }
  if( DEBUG9 )
    fprintf(stderr, "] KheLayerMatchAssignBestEdges returning %s\n",
      res ? "true" : "false");
  return res;
}


/*****************************************************************************/
/*                                                                           */
/*  KHE_COST KheLayerMatchCost(KHE_LAYER_MATCH lm)                           */
/*                                                                           */
/*  Return the current cost of lm.  This is a sum of solution costs, so      */
/*  it is not very meaningful.                                               */
/*                                                                           */
/*****************************************************************************/

KHE_COST KheLayerMatchCost(KHE_LAYER_MATCH lm)
{
  int junk;  KHE_COST cost;
  KheWMatchEval(lm->wmatch, &junk, &cost);
  return cost;
}


/*****************************************************************************/
/*                                                                           */
/*  void KheLayerMatchDelete(KHE_LAYER_MATCH lm)                             */
/*                                                                           */
/*  Delete lm, freeing the memory it consumed.                               */
/*                                                                           */
/*****************************************************************************/

void KheLayerMatchDelete(KHE_LAYER_MATCH lm)
{
  /* MArrayFree(lm->template_layers); */
  while( MArraySize(lm->supply_node_groups) > 0 )
    KheSupplyNodeGroupDelete(MArrayRemoveLast(lm->supply_node_groups));
  MArrayFree(lm->supply_node_groups);
  while( MArraySize(lm->demand_node_groups) > 0 )
    KheDemandNodeGroupDelete(MArrayRemoveLast(lm->demand_node_groups));
  MArrayFree(lm->demand_node_groups);
  MArrayFree(lm->demand_nodes);
  while( MArraySize(lm->even_time_groups) > 0 )
    KheEvenTimeGroupFree(MArrayRemoveLast(lm->even_time_groups));
  KheWMatchDelete(lm->wmatch);
  MFree(lm);
}


/*****************************************************************************/
/*                                                                           */
/*  void KheLayerMatchDebugSegmentation(KHE_LAYER_MATCH lm,                  */
/*    int verbosity, int indent, FILE *fp)                                   */
/*                                                                           */
/*  Debug the segmentation of lm.                                            */
/*                                                                           */
/*****************************************************************************/

static void KheLayerMatchDebugSegmentation(KHE_LAYER_MATCH lm,
  int verbosity, int indent, FILE *fp)
{
  int i, j, durn, count, max_durn;  KHE_SUPPLY_NODE_GROUP sng;
  KHE_SUPPLY_NODE sn;  KHE_TIME time;
  if( verbosity >= 1 && indent >= 0 )
  {
    /* print the segmentation and find max_durn */
    fprintf(stderr, "%*ssegmentation: ", indent, "");
    max_durn = 0;
    MArrayForEach(lm->supply_node_groups, &sng, &i)
      MArrayForEach(sng->supply_nodes, &sn, &j)
      {
	if( i + j > 0 )
	  fprintf(stderr, ", ");
	if( KheMeetIsCycleMeet(sng->target_meet) )
	{
	  time = KheTimeNeighbour(KheMeetAsstTime(sng->target_meet),
	    sn->target_offset);
	  fprintf(stderr, "%s", KheTimeId(time));
	}
	else
	{
	  KheMeetDebug(sng->target_meet, 1, -1, stderr);
	  fprintf(stderr, "+%d", sn->target_offset);
	}
	fprintf(stderr, "d%d", sn->duration);
	if( sn->duration > max_durn )
	  max_durn = sn->duration;
      }
    fprintf(stderr, "\n");

    /* print the number of segments of each duration */
    fprintf(stderr, "%*sdurations: ", indent, "");
    for( durn = max_durn;  durn >= 1;  durn-- )
    {
      count = 0;
      MArrayForEach(lm->supply_node_groups, &sng, &i)
	MArrayForEach(sng->supply_nodes, &sn, &j)
	  if( sn->duration == durn )
	    count++;
      if( durn != max_durn )
	fprintf(stderr, ", ");
      fprintf(stderr, "%d x d%d", count, durn);
    }
    fprintf(stderr, "\n");
  }
}


/*****************************************************************************/
/*                                                                           */
/*  void KheLayerMatchDebug(KHE_LAYER_MATCH lm, int verbosity,               */
/*    int indent, FILE *fp)                                                  */
/*                                                                           */
/*  Debug print of lm with the given verbosity and indent.                   */
/*                                                                           */
/*****************************************************************************/

void KheLayerMatchDebug(KHE_LAYER_MATCH lm, int verbosity,
  int indent, FILE *fp)
{
  int i, infeasibility;  KHE_COST badness;  KHE_DEMAND_NODE_GROUP dng;
  KHE_SUPPLY_NODE_GROUP sng;  KHE_EVEN_TIME_GROUP etg;
  if( verbosity >= 1 && indent >= 0 )
  {
    fprintf(fp, "%*s[ Layer Match for ", indent, "");
    KheLayerDebug(lm->layer, 2, 0, stderr);
    MArrayForEach(lm->supply_node_groups, &sng, &i)
      KheSupplyNodeGroupDebug(sng, verbosity, indent + 2, fp);
    MArrayForEach(lm->demand_node_groups, &dng, &i)
      KheDemandNodeGroupDebug(dng, verbosity, indent + 2, fp);
    if( MArraySize(lm->even_time_groups) > 0 )
    {
      fprintf(fp, "%*s  unevenness (", indent, "");
      MArrayForEach(lm->even_time_groups, &etg, &i)
      {
	if( i > 0 )
	  fprintf(fp, ", ");
	fprintf(fp, "%s:%d", KheTimeGroupId(etg->time_group),
	  etg->supply_node_count);
      }
      fprintf(fp, ") %d\n", lm->unevenness);
    }
    if( verbosity >= 2 )
    {
      KheWMatchEval(lm->wmatch, &infeasibility, &badness);
      KheWMatchDebug(lm->wmatch, verbosity, indent + 2, fp);
    }
    fprintf(fp, "%*s]\n", indent, "");
  }
}


/*****************************************************************************/
/*                                                                           */
/*  Submodule "node regularity and main solver function"                     */
/*                                                                           */
/*****************************************************************************/

/*****************************************************************************/
/*                                                                           */
/*  bool KheMeetSegmentsOverlap(int offset1, int durn1,                      */
/*    int offset2, int durn2)                                                */
/*                                                                           */
/*  Return true if two segments of the same meet, one with offset offset1    */
/*  and duration durn1, and the other with offset offset2 and duration       */
/*  durn2, overlap.                                                          */
/*                                                                           */
/*****************************************************************************/

/* ***
static bool KheMeetSegmentsOverlap(int offset1, int durn1,
  int offset2, int durn2)
{
  return !(offset1 + durn1 <= offset2 || offset2 + durn2 <= offset1);
}
*** */


/*****************************************************************************/
/*                                                                           */
/*  void KheLayerMatchAddSupplyNodeIndexes(KHE_LAYER_MATCH lm)               */
/*                                                                           */
/*  Add template layer nodes and node indexes to the supply nodes of lm,     */
/*  showing their overlaps with nodes in all template layers.                */
/*                                                                           */
/*****************************************************************************/

/* ***
static void KheLayerMatchAddSupplyNodeIndexes(KHE_LAYER_MATCH lm)
{
  int i, j, k, n, m;  KHE_NODE node;  KHE_MEET meet;  KHE_SUPPLY_NODE sn;
  KHE_SUPPLY_NODE_GROUP sng;  KHE_LAYER tl;  KHE_SUBLAYER sublayer;

  ** add a supply node index set to each supply node **
  MArrayForEach(lm->supply_node_groups, &sng, &k)
    MArrayForEach(sng->supply_nodes, &sn, &n)
    {
      MAssert(sn->tl_node_index_set == NULL,
	"KheLayerMatchAddSupplyNodeIndexes internal error");
      sn->tl_node_index_set = LSetNew();
    }

  ** add one sublayer to each supply node for each template layer **
  MArrayForEach(lm->supply_node_groups, &sng, &k)
    MArrayForEach(lm->template_layers, &tl, &m)
    {
      ** add a sublayer based on tl to each supply node of sng **
      MArrayForEach(sng->supply_nodes, &sn, &n)
        MArrayAddLast(sn->tl_sublayers, KheSubLayerMake(tl));

      ** add nodes to these sublayers **
      for( i = 0;  i < KheLayerChildNodeCount(tl);  i++ )
      {
	node = KheLayerChildNode(tl, i);
	for( j = 0;  j < KheNodeMeetCount(node);  j++ )
	{
	  meet = KheNodeMeet(node, j);
	  if( sng->target_meet == KheMeetAsst(meet) )
	    MArrayForEach(sng->supply_nodes, &sn, &n)
	      if( KheMeetSegmentsOverlap(sn->target_offset, sn->duration,
		    KheMeetAsstOffset(meet), KheMeetDuration(meet)) )
	      {
		sublayer = MArrayGet(sn->tl_sublayers, m);
		KheSubLayerAddNode(sublayer, node);
		LSetInsert(&sn->tl_node_index_set, KheNodeIndex(node));
	      }
	}
      }
    }
}
*** */


/*****************************************************************************/
/*                                                                           */
/*  void KheLayerMatchAddZones(KHE_LAYER_MATCH lm)                           */
/*                                                                           */
/*  Add zones and zone index sets to the supply nodes of lm, showing their   */
/*  overlaps with zones.                                                     */
/*                                                                           */
/*****************************************************************************/

static void KheLayerMatchAddZones(KHE_LAYER_MATCH lm)
{
  int i, j, k, pos;  KHE_SUPPLY_NODE sn;  KHE_SUPPLY_NODE_GROUP sng;
  KHE_ZONE zone;
  MArrayForEach(lm->supply_node_groups, &sng, &i)
    MArrayForEach(sng->supply_nodes, &sn, &j)
    {
      /* add a zone index set and zones to sn */
      MAssert(sn->zone_index_set==NULL,"KheLayerMatchAddZones internal error");
      sn->zone_index_set = LSetNew();
      for( k = 0;  k < sn->duration;  k++ )
      {
	zone = KheMeetOffsetZone(sng->target_meet, sn->target_offset + k);
	if( zone != NULL && !MArrayContains(sn->zones, zone, &pos) )
	{
	  MArrayAddLast(sn->zones, zone);
	  LSetInsert(&sn->zone_index_set, KheZoneIndex(zone));
	}
      }
    }
}


/*****************************************************************************/
/*                                                                           */
/*  bool KheLayerMatchEdge(KHE_DEMAND_NODE dn, KHE_SUPPLY_NODE sn)           */
/*                                                                           */
/*  Return true if an edge can be drawn between dn and sn, not counting      */
/*  cost and not concerned with template layer restrictions.                 */
/*                                                                           */
/*****************************************************************************/

static bool KheLayerMatchEdge(KHE_DEMAND_NODE dn, KHE_SUPPLY_NODE sn)
{
  if( sn->duration != KheMeetDuration(dn->meet) )
    return false;
  else if( KheMeetAsst(dn->meet) != NULL )
    return sn->supply_node_group->target_meet == KheMeetAsst(dn->meet) &&
      sn->target_offset == KheMeetAsstOffset(dn->meet);
  else
    return KheMeetAssignCheck(dn->meet, sn->supply_node_group->target_meet,
      sn->target_offset);
}


/*****************************************************************************/
/*                                                                           */
/*  void KheAddRestrictions(KHE_LAYER_MATCH lm)                              */
/*                                                                           */
/*  Add template layer restrictions to lm's demand node groups.              */
/*                                                                           */
/*****************************************************************************/

/* *** old version that uses template layers
static void KheAddRestrictions(KHE_LAYER_MATCH lm)
{
  KHE_DEMAND_NODE dn;  KHE_SUPPLY_NODE sn;  KHE_NODE node;
  int i, j, k, n, m, p;  KHE_SUBLAYER sublayer;  KHE_LAYER layer;
  KHE_DEMAND_NODE_GROUP dng;  KHE_SUPPLY_NODE_GROUP sng;
  MArrayForEach(lm->demand_node_groups, &dng, &i)
  {
    ** add sublayers and an lset to dng **
    MAssert(dng->curr_restriction == NULL,
      "KheLayerMatchAddDemandNodeIndexes internal error");
    dng->curr_restriction = KheRestrictionMake();
    dng->best_restriction = KheRestrictionMake();
    MArrayForEach(lm->template_layers, &layer, &m)
      MArrayAddLast(dng->tl_sublayers, KheSubLayerMake(layer));

    ** add nodes to dng's sublayers **
    MArrayForEach(dng->demand_nodes, &dn, &j)
      MArrayForEach(lm->supply_node_groups, &sng, &k)
        MArrayForEach(sng->supply_nodes, &sn, &n)
	  if( KheLayerMatchEdge(dn, sn) )
	  {
	    ** sn's template nodes get added to dng's, if not already present **
	    MArrayForEach(sn->tl_sublayers, &sublayer, &m)
	      MArrayForEach(sublayer->nodes, &node, &p)
		KheDemandNodeGroupAddTemplateLayerNode(dng, m, node);
	  }
  }
}
*** */

static void KheAddRestrictions(KHE_LAYER_MATCH lm)
{
  KHE_DEMAND_NODE_GROUP dng;  KHE_DEMAND_NODE dn;  int i, j, k, n, m;
  KHE_SUPPLY_NODE_GROUP sng;  KHE_SUPPLY_NODE sn;  KHE_ZONE zone;
  MArrayForEach(lm->demand_node_groups, &dng, &i)
  {
    /* add restrictions to dng */
    MAssert(dng->curr_restriction == NULL,
      "KheAddRestrictions internal error");
    dng->curr_restriction = KheRestrictionMake();
    dng->best_restriction = KheRestrictionMake();

    /* add zones to dng's demand nodes */
    MArrayForEach(dng->demand_nodes, &dn, &j)
      MArrayForEach(lm->supply_node_groups, &sng, &k)
        MArrayForEach(sng->supply_nodes, &sn, &n)
	  if( KheLayerMatchEdge(dn, sn) )
	  {
	    /* sn's zones get added to dng's, if not already present */
	    MArrayForEach(sn->zones, &zone, &m)
	      KheDemandNodeGroupAddZone(dng, zone);
	  }

    /* initialize curr restriction to everything */
    KheDemandNodeGroupResetCurrRestriction(dng);
  }
}


/*****************************************************************************/
/*                                                                           */
/*  void SetRegularityCost(KHE_DEMAND_NODE_GROUP dng,                        */
/*    KHE_REGULARITY_COST *rc)                                               */
/*                                                                           */
/*  Set *rc to the regularity cost of dng.                                   */
/*                                                                           */
/*****************************************************************************/

static void SetRegularityCost(KHE_DEMAND_NODE_GROUP dng,
  KHE_REGULARITY_COST *rc)
{
  int i, target_offset;  KHE_COST cost;  KHE_DEMAND_NODE dn;
  KHE_MEET target_meet;  KHE_LAYER_MATCH lm;
  lm = dng->layer_match;
  KheDemandNodeGroupNotifyDirty(dng);
  KheWMatchEval(lm->wmatch, &rc->infeasibility, &cost);
  rc->without_children_cost = rc->with_children_cost = 0;
  MArrayForEach(lm->demand_nodes, &dn, &i)
    if( KheMeetAsst(dn->meet) == NULL &&
      KheLayerMatchBestEdge(lm, dn->meet, &target_meet, &target_offset, &cost) )
    {
      if( KheNodeChildCount(dn->demand_node_group->node) == 0 )
	rc->without_children_cost += cost;
      else
	rc->with_children_cost += cost;
    }
  rc->zones_cost = dng->curr_restriction->irregularity;
}


/*****************************************************************************/
/*                                                                           */
/*  void DoTrySetsOfSize(KHE_DEMAND_NODE_GROUP dng, int zone_index,          */
/*    int count)                                                             */
/*                                                                           */
/*  Try all subsets of size count from zone_index onwards.                   */
/*                                                                           */
/*****************************************************************************/

static void DoTrySetsOfSize(KHE_DEMAND_NODE_GROUP dng, int zone_index,
  int count)
{
  KHE_ZONE zone;  int irregularity, i;  KHE_REGULARITY_COST rc;
  if( zone_index + count > MArraySize(dng->zones) )
  {
    /* not enough zones left to make a set of size count, so do nothing */
    return;
  }
  else if( count == 0 )
  {
    /* only subset now is the empty set, so time to test */
    SetRegularityCost(dng, &rc);
    if( DEBUG5 )
    {
      fprintf(stderr, "    trying {");
      KheRegularityCostDebug(&rc, 1, -1, stderr);
      fprintf(stderr, ": ");
      MArrayForEach(dng->curr_restriction->array, &zone, &i)
      {
	if( i > 0 )
	  fprintf(stderr, ", ");
	/* KheZoneDebug(zone, 1, -1, stderr); */
	fprintf(stderr, "%d", KheZoneIndex(zone));
      }
      fprintf(stderr, "}%s\n",
        KheRegularityCostLessThan(&rc, &dng->layer_match->best_reg_cost) ?
	  " (new best)" : "");
    }
    if( KheRegularityCostLessThan(&rc, &dng->layer_match->best_reg_cost) )
    {
      KheRestrictionAssign(dng->best_restriction, dng->curr_restriction);
      dng->layer_match->best_reg_cost = rc;
    }
  }
  else
  {
    /* try with zone, which the previous tests prove must exist */
    zone = MArrayGet(dng->zones, zone_index);
    irregularity = KheDemandNodeGroupZoneIrregularity(dng, zone);
    KheRestrictionPushZone(dng->curr_restriction, zone, irregularity);
    DoTrySetsOfSize(dng, zone_index + 1, count - 1);
    KheRestrictionPopZone(dng->curr_restriction, zone, irregularity);

    /* try without zone */
    DoTrySetsOfSize(dng, zone_index + 1, count);
  }
}

/* ***
static void DoTrySetsOfSize(KHE_DEMAND_NODE_GROUP dng, int layer_index,
  int tl_node_index, int count)
{
  KHE_NODE tl_node;  KHE_SUBLAYER sublayer;  int irregularity, i;
  KHE_REGULARITY_COST rc;
  sublayer = MArrayGet(dng->tl_sublayers, layer_index);
  if( tl_node_index + count > MArraySize(sublayer->nodes) )
  {
    ** not enough nodes left to make a set of size count, so do nothing **
    return;
  }
  else if( count == 0 )
  {
    ** only subset now is the empty set, so time to test **
    SetRegularityCost(dng, &rc);
    if( DEBUG5 )
    {
      fprintf(stderr, "    trying {");
      KheRegularityCostDebug(&rc, 1, -1, stderr);
      fprintf(stderr, ": ");
      MArrayForEach(dng->curr_restriction->array, &tl_node, &i)
      {
	if( i > 0 )
	  fprintf(stderr, ", ");
	KheNodeDebug(tl_node, 1, -1, stderr);
      }
      fprintf(stderr, "}%s\n",
        KheRegularityCostLessThan(&rc, &dng->layer_match->best_reg_cost) ?
	  " (new best)" : "");
    }
    if( KheRegularityCostLessThan(&rc, &dng->layer_match->best_reg_cost) )
    {
      KheRestrictionAssign(dng->best_restriction, dng->curr_restriction);
      dng->layer_match->best_reg_cost = rc;
    }
  }
  else
  {
    ** try with tl_node, which the previous tests prove must exist **
    tl_node = MArrayGet(sublayer->nodes, tl_node_index);
    irregularity = KheDemandNodeGroupNodeIrregularity(dng, tl_node);
    KheRestrictionPushNode(dng->curr_restriction, tl_node, irregularity);
    DoTrySetsOfSize(dng, layer_index, tl_node_index + 1, count - 1);
    KheRestrictionPopNode(dng->curr_restriction, tl_node, irregularity);

    ** try without tl_node **
    DoTrySetsOfSize(dng, layer_index, tl_node_index + 1, count);
  }
}
*** */


/*****************************************************************************/
/*                                                                           */
/*  void TryFullSet(KHE_DEMAND_NODE_GROUP dng)                               */
/*                                                                           */
/*  As a backstop, try the full set of demand node group for dng.            */
/*                                                                           */
/*****************************************************************************/

static void TryFullSet(KHE_DEMAND_NODE_GROUP dng)
{
  KHE_REGULARITY_COST rc;
  KheDemandNodeGroupResetCurrRestriction(dng);
  SetRegularityCost(dng, &rc);
  if( DEBUG5 )
  {
    fprintf(stderr, "    trying full ");
    KheRegularityCostDebug(&rc, 1, -1, stderr);
    fprintf(stderr, ": %s%s\n", LSetShow(dng->curr_restriction->lset),
      KheRegularityCostLessThan(&rc, &dng->layer_match->best_reg_cost) ?
	" (new best)" : "");
  }
  if( KheRegularityCostLessThan(&rc, &dng->layer_match->best_reg_cost) )
  {
    KheRestrictionAssign(dng->best_restriction, dng->curr_restriction);
    dng->layer_match->best_reg_cost = rc;
  }
}

/* ***
static void TryFullSet(KHE_DEMAND_NODE_GROUP dng)
{
  KHE_SUBLAYER sublayer;  KHE_NODE tl_node;  int i, j, irregularity;
  KHE_REGULARITY_COST rc;
  KheRestrictionClear(dng->curr_restriction);
  MArrayForEach(dng->tl_sublayers, &sublayer, &i)
    MArrayForEach(sublayer->nodes, &tl_node, &j)
    {
      irregularity = KheDemandNodeGroupNodeIrregularity(dng, tl_node);
      KheRestrictionPushNode(dng->curr_restriction, tl_node, irregularity);
    }
  SetRegularityCost(dng, &rc);
  if( DEBUG5 )
  {
    fprintf(stderr, "    trying full ");
    KheRegularityCostDebug(&rc, 1, -1, stderr);
    fprintf(stderr, ": %s%s\n", LSetShow(dng->curr_restriction->lset),
      KheRegularityCostLessThan(&rc, &dng->layer_match->best_reg_cost) ?
	" (new best)" : "");
  }
  if( KheRegularityCostLessThan(&rc, &dng->layer_match->best_reg_cost) )
  {
    KheRestrictionAssign(dng->best_restriction, dng->curr_restriction);
    dng->layer_match->best_reg_cost = rc;
  }
}
*** */


/*****************************************************************************/
/*                                                                           */
/*  void KheLayerMatchImproveNodeRegularity(KHE_LAYER_MATCH lm)              */
/*                                                                           */
/*  Improve the node regularity of lm with respect to the template layers.   */
/*                                                                           */
/*****************************************************************************/

void KheLayerMatchImproveNodeRegularity(KHE_LAYER_MATCH lm)
{
  KHE_DEMAND_NODE_GROUP dng;  int i, j, count, infeas;  KHE_COST junk;
  KHE_ZONE zone;
  if( DEBUG4 )
    fprintf(stderr, "[ KheLayerMatchImproveNodeRegularity(lm)");
  if( KheNodeZoneCount(KheLayerParentNode(lm->layer)) > 0 )
  {
    /* add zones and zone index sets to the supply and demand nodes of lm */
    KheWMatchSpecialModeBegin(lm->wmatch);
    /* KheLayerMatchAddSupplyNodeIndexes(lm); */
    KheLayerMatchAddZones(lm);
    KheAddRestrictions(lm);

    /* sort demand node groups by decreasing duration, increasing children */
    MArraySort(lm->demand_node_groups, &KheDemandNodeGroupCmp);

    /* diversify */
    MArrayForEach(lm->demand_node_groups, &dng, &i)
      KheDemandNodeGroupDiversify(dng);

    /* search */
    KheWMatchEval(lm->wmatch, &infeas, &junk);
    if( DEBUG4 )
      fprintf(stderr, "  infeas %d\n", infeas);
    MArrayForEach(lm->demand_node_groups, &dng, &i)
    {
      if( DEBUG4 )
      {
	fprintf(stderr, "  [ restricting ");
	KheNodeDebug(dng->node, 1, -1, stderr);
	fprintf(stderr, " (%d children)\n", KheNodeChildCount(dng->node));
      }
      KheRestrictionClear(dng->best_restriction);
      KheRestrictionClear(dng->curr_restriction);
      KheRegularityCostInitToLarge(&lm->best_reg_cost, infeas);
      for( count = 1;  count <= 2;  count++ )
	DoTrySetsOfSize(dng, 0, count);
      /* ***
      for( count = 1;  count <= 2;  count++ )
	for( j = 0;  j < MArraySize(dng->tl_sublayers);  j++ )
	  DoTrySetsOfSize(dng, j, 0, count);
      *** */
      TryFullSet(dng);
      //I COMMENTED THIS CODE!!!
     //MAssert(!KheRestrictionIsEmpty(dng->best_restriction),
	//"KheLayerMatchImproveNodeRegularity internal error");
      KheRestrictionAssign(dng->curr_restriction, dng->best_restriction);
      KheDemandNodeGroupNotifyDirty(dng);
      if( DEBUG4 )
      {
	fprintf(stderr, "  ] best ");
	KheRegularityCostDebug(&lm->best_reg_cost, 1, -1, stderr);
	fprintf(stderr, ": %s ", LSetShow(dng->curr_restriction->lset));
	MArrayForEach(dng->curr_restriction->array, &zone, &j)
	{
	  if( j > 0 )
	    fprintf(stderr, ", ");
	  fprintf(stderr, "%d", KheZoneIndex(zone));
	  /* KheZoneDebug(zone, 1, -1, stderr); */
	}
	fprintf(stderr, "\n");
      }
    }
    KheWMatchSpecialModeEnd(lm->wmatch);
  }
  if( DEBUG4 )
    fprintf(stderr, "] KheLayerMatchImproveNodeRegularity returning\n");
}


/*****************************************************************************/
/*                                                                           */
/*  bool KheLayerMatchAssignTimes(KHE_LAYER layer,                           */
/*    KHE_SPREAD_EVENTS_CONSTRAINT sec)                                      */
/*                                                                           */
/*  Assign layer using layer matching.  If present, sec influences the       */
/*  assignment to spread itself evenly through the time groups of sec.       */
/*                                                                           */
/*****************************************************************************/

bool KheLayerMatchAssignTimes(KHE_LAYER layer,
  KHE_SPREAD_EVENTS_CONSTRAINT sec)
{
  KHE_LAYER_MATCH lm;  bool res;
  lm = KheLayerMatchMake(layer, sec);
  KheLayerMatchImproveNodeRegularity(lm);
  res = KheLayerMatchAssignBestEdges(lm);
  KheLayerMatchDelete(lm);
  return res;
}
