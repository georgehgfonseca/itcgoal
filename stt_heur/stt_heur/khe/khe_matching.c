
/*****************************************************************************/
/*                                                                           */
/*  THE KTS TIMETABLING SYSTEM                                               */
/*  COPYRIGHT (C) 2004, 2010 Jeffrey H. Kingston                             */
/*                                                                           */
/*  Jeffrey H. Kingston (jeff@it.usyd.edu.au)                                */
/*  School of Information Technologies                                       */
/*  The University of Sydney 2006                                            */
/*  AUSTRALIA                                                                */
/*                                                                           */
/*  FILE:         khe_matching.c                                             */
/*  MODULE:       Unweighted bipartite matching (implementation)             */
/*                                                                           */
/*****************************************************************************/
#include "khe_interns.h"
#include <limits.h>
#define DEBUG1 0

/*****************************************************************************/
/*                                                                           */
/*  Type declarations                                                        */
/*                                                                           */
/*****************************************************************************/

struct khe_matching_supply_chunk_rec {
  KHE_MATCHING				matching;
  void					*impl;
  int					base;
  ARRAY_KHE_MATCHING_SUPPLY_NODE	supply_nodes;
  KHE_MATCHING_SUPPLY_CHUNK		copy;
};

struct khe_matching_supply_node_rec {
  KHE_MATCHING_SUPPLY_CHUNK		supply_chunk;
  void					*impl;
  int					index;
  unsigned int				visit_num;
  KHE_MATCHING_DEMAND_NODE		supply_asst;
  KHE_MATCHING_HALL_SET			hall_set;
  short					demand_test_index;
  KHE_MATCHING_SUPPLY_NODE		copy;
};

struct khe_matching_demand_chunk_rec {
  KHE_MATCHING				matching;
  int					index_in_matching;
  void					*impl;
  int					base;
  int					increment;
  ARRAY_SHORT				domain;
  ARRAY_KHE_MATCHING_DEMAND_NODE	demand_nodes;
  KHE_MATCHING_DEMAND_CHUNK		copy;
};

struct khe_matching_demand_node_rec {  /* this is an abstract supertype */

  /* inherited from KHE_MONITOR */
  KHE_SOLN			soln;			/* encl. solution    */
  int				index_in_soln;		/* index in soln     */
  unsigned char			tag;			/* tag field         */
  bool				attached;		/* true if attached  */
  void				*back;			/* back pointer      */
  KHE_GROUP_MONITOR		parent_monitor;		/* parent monitor    */
  int				parent_index;		/* index in parent   */
  int				defect_index;		/* defect index      */
  int				trace_num;		/* trace visit num   */
  KHE_COST			trace_cost;		/* at start of trace */
  KHE_COST			cost;			/* current cost      */

  /* specific to KHE_MATCHING_DEMAND_NODE */
  KHE_MATCHING_DEMAND_CHUNK		demand_chunk;
  ARRAY_SHORT				domain;
  KHE_MATCHING_SUPPLY_NODE		demand_asst;
  short					demand_asst_index;
  short					unmatched_pos;
  KHE_MATCHING_DEMAND_NODE		bfs_next;
  KHE_MATCHING_DEMAND_NODE		bfs_parent;
  KHE_MATCHING_HALL_SET			hall_set;
};

struct khe_matching_rec {
  void					*impl;
  /* ARRAY_SHORT			zero_domain; */
  ARRAY_KHE_MATCHING_SUPPLY_CHUNK	supply_chunks;
  ARRAY_KHE_MATCHING_DEMAND_CHUNK	demand_chunks;
  ARRAY_KHE_MATCHING_SUPPLY_NODE	supply_nodes;
  int					demand_node_count;
  unsigned int				visit_num;
  ARRAY_KHE_MATCHING_DEMAND_NODE	unmatched_demand_nodes;
  int					unmatched_lower_bound;
  bool					active;
  ARRAY_INT				marks;
  ARRAY_KHE_MATCHING_HALL_SET		hall_sets;
  KHE_MATCHING_DEMAND_NODE		comp_bfs_first;
  KHE_MATCHING_DEMAND_NODE		comp_bfs_last;
  KHE_MATCHING				copy;
};

struct khe_matching_hall_set_rec {
  KHE_MATCHING_HALL_SET			parent_hall_set;
  ARRAY_KHE_MATCHING_SUPPLY_NODE	supply_nodes;
  ARRAY_KHE_MATCHING_DEMAND_NODE	demand_nodes;
  KHE_MATCHING_HALL_SET			copy;
};


/*****************************************************************************/
/*                                                                           */
/*  Solving (private)                                                        */
/*                                                                           */
/*****************************************************************************/

/*****************************************************************************/
/*                                                                           */
/*  void KheDemandNodeCheck(KHE_MATCHING_DEMAND_NODE dn)                     */
/*                                                                           */
/*  Check an invariant of dn.                                                */
/*                                                                           */
/*****************************************************************************/

static void KheDemandNodeCheck(KHE_MATCHING_DEMAND_NODE dn)
{
  MAssert( (dn->demand_asst == NULL) == (dn->cost > 0),
    "KheDemandNodeCheck(weight %.4f, dn->demand_asst %s NULL, dn->cost %.4f)",
    KheCostShow(KheSolnMatchingWeight(dn->soln)),
    dn->demand_asst == NULL ? "==" : "!=", KheCostShow(dn->cost));
}


/*****************************************************************************/
/*                                                                           */
/*  bool AssignBFS(KHE_MATCHING_DEMAND_NODE dn, unsigned int visit_num)      */
/*                                                                           */
/*  Try to find and apply an augmenting path from unassigned demand node     */
/*  dn, returning true if successful.  Use breadth-first search.             */
/*                                                                           */
/*****************************************************************************/

static bool AssignBFS(KHE_MATCHING_DEMAND_NODE dn, unsigned int visit_num)
{
  int i, j, base;  KHE_MATCHING_SUPPLY_NODE sn, s;
  ARRAY_KHE_MATCHING_SUPPLY_NODE supply_nodes;
  KHE_MATCHING_DEMAND_NODE bfs_first, bfs_last, d;
  KHE_MATCHING_DEMAND_CHUNK dc;

  /* initialize the queue to contain just dn */
  bfs_first = bfs_last = dn;
  bfs_last->bfs_parent = bfs_last->bfs_next = NULL;

  /* each iteration processes one item, until success or queue is empty */
  supply_nodes = bfs_first->demand_chunk->matching->supply_nodes;
  do
  {
    /* add bfs_first's successors to the queue, unless one is unassigned */
    dc = bfs_first->demand_chunk;
    for( i = 0;  i < MArraySize(dc->domain);  i++ )
    {
      base = dc->base + dc->increment * MArrayGet(dc->domain, i);
      for( j = 0;  j < MArraySize(bfs_first->domain);  j++ )
      {
	sn = MArrayGet(supply_nodes, base + MArrayGet(bfs_first->domain, j));
	if( sn->visit_num < visit_num )
	{
	  sn->visit_num = visit_num;
	  sn->demand_test_index = j;
	  if( sn->supply_asst == NULL )
	  {
	    /* unassigned, unwind tree and make the augmenting assignments */
	    d = bfs_first;  s = sn;
	    do
	    {
	      sn = d->demand_asst;
	      s->supply_asst = d;
	      d->demand_asst = s;
	      d->demand_asst_index = s->demand_test_index;
	      d = d->bfs_parent;  s = sn;
	    } while( d != NULL );
	    return true;
	  }
	  else
	  {
	    /* assigned, add its demand node to the queue to be handled later */
	    bfs_last->bfs_next = sn->supply_asst;
	    bfs_last = bfs_last->bfs_next;
	    bfs_last->bfs_next = NULL;
	    bfs_last->bfs_parent = bfs_first;
	  }
	}
      }
    }
    bfs_first = bfs_first->bfs_next;
  } while( bfs_first != NULL );
  return false;
}


/*****************************************************************************/
/*                                                                           */
/*  void AddUnMatchedDemandNode(KHE_MATCHING m, KHE_MATCHING_DEMAND_NODE dn) */
/*                                                                           */
/*  Add dn to the set of unmatched demand nodes of m.                        */
/*                                                                           */
/*****************************************************************************/

int hit_count = 0;

static void AddUnMatchedDemandNode(KHE_MATCHING m, KHE_MATCHING_DEMAND_NODE dn)
{
  KHE_COST weight;
  hit_count++;
  /* KheMonitorCheck((KHE_MONITOR) dn); */
  MAssert(dn->demand_chunk->matching == m, "AddUnMatchedDemandNode sanity");
  MAssert(dn->demand_asst == NULL, "AddUnMatchedDemandNode internal error");
  dn->unmatched_pos = MArraySize(m->unmatched_demand_nodes);
  MArrayAddLast(m->unmatched_demand_nodes, dn);
  MAssert(dn->cost == 0, "AddUnMatchedDemandNode internal error");
  weight = KheSolnMatchingWeight(dn->soln);
  /* KheMonitorCheck((KHE_MONITOR) dn); */
  MAssert(dn->demand_asst == NULL, "AddUnMatchedDemandNode internal error 2");
  if( weight > 0 )
    KheMonitorChangeCost((KHE_MONITOR) dn, weight);
  MAssert(dn->demand_asst == NULL, "AddUnMatchedDemandNode internal error 3");
  /* KheMonitorCheck((KHE_MONITOR) dn); */
  KheDemandNodeCheck(dn);
}


/*****************************************************************************/
/*                                                                           */
/*  void RemoveUnMatchedDemandNode(KHE_MATCHING m,                           */
/*    KHE_MATCHING_DEMAND_NODE dn)                                           */
/*                                                                           */
/*  Remove dn from the set of unmatched demand nodes of m.                   */
/*                                                                           */
/*  Implementation note.  When this function is called, dn->demand_asst      */
/*  may be either NULL or non-NULL, the latter case occurring because the    */
/*  node had just been assigned.                                             */
/*                                                                           */
/*****************************************************************************/

static void RemoveUnMatchedDemandNode(KHE_MATCHING m,
  KHE_MATCHING_DEMAND_NODE dn)
{
  KHE_MATCHING_DEMAND_NODE x;
  MAssert(MArrayGet(m->unmatched_demand_nodes, dn->unmatched_pos) == dn,
    "RemoveUnMatchedDemandNode internal error");
  x = MArrayRemoveAndPlug(m->unmatched_demand_nodes, dn->unmatched_pos);
  x->unmatched_pos = dn->unmatched_pos;
  if( dn->cost > 0 )
    KheMonitorChangeCost((KHE_MONITOR) dn, 0);
  /* this does not apply here! KheDemandNodeCheck(dn); */
}


/*****************************************************************************/
/*                                                                           */
/*  void CheckInvt(KHE_MATCHING m)                                           */
/*                                                                           */
/*  Check (part of) the invariant of m.                                      */
/*                                                                           */
/*****************************************************************************/

static void CheckInvt(KHE_MATCHING m)
{
  int i;  KHE_MATCHING_DEMAND_NODE dn;
  MArrayForEach(m->unmatched_demand_nodes, &dn, &i)
    MAssert(dn->demand_asst == NULL, "CheckInvt internal error");
}


/*****************************************************************************/
/*                                                                           */
/*  void MakeClean(KHE_MATCHING m)                                           */
/*                                                                           */
/*  Ensure that m is up to date, by attempting to assign each unassigned     */
/*  demand node.  If the matching is not already up to date, also make sure  */
/*  that there are no Hall sets.                                             */
/*                                                                           */
/*  Implementation note.  This function optimizes by trying to assign the    */
/*  unmatched nodes in reverse order of insertion, trying a previously       */
/*  successful domain element before embarking on the general assignment     */
/*  algorithm, not incrementing the visit number when the immediately        */
/*  preceding assignment attempt failed, and stopping early if the lower     */
/*  bound is achieved.                                                       */
/*                                                                           */
/*****************************************************************************/
static void KheMatchingHallSetFree(KHE_MATCHING_HALL_SET hs);

static void MakeClean(KHE_MATCHING m)
{
  int i, j, base;  KHE_MATCHING_DEMAND_NODE dn;  bool visit_inc_needed_next;
  KHE_MATCHING_DEMAND_CHUNK dc;  KHE_MATCHING_SUPPLY_NODE sn;
  ARRAY_KHE_MATCHING_SUPPLY_NODE supply_nodes;
  if( m->unmatched_lower_bound < MArraySize(m->unmatched_demand_nodes) )
  {
    /* lower bound not reached, so assigmments are required */
    if( DEBUG1 )
      CheckInvt(m);
    visit_inc_needed_next = true;
    MArrayForEachReverse(m->unmatched_demand_nodes, &dn, &i)
    {
      /* see whether a previously successful elt of dn->domain will work now */
      MAssert(dn->demand_asst == NULL, "MakeClean internal error");
      if( dn->demand_asst_index < MArraySize(dn->domain) )
      {
	dc = dn->demand_chunk;
	supply_nodes = dc->matching->supply_nodes;
	for( j = 0;  j < MArraySize(dc->domain);  j++ )
	{
	  base = dc->base + dc->increment * MArrayGet(dc->domain, j);
	  sn = MArrayGet(supply_nodes,
	    base + MArrayGet(dn->domain, dn->demand_asst_index));
	  if( sn->supply_asst == NULL )
	  {
	    sn->supply_asst = dn;
	    dn->demand_asst = sn;
	    break;
	  }
	}
      }

      /* if still unassigned, call AssignBFS */
      if( dn->demand_asst == NULL )
      {
	if( visit_inc_needed_next )
	  m->visit_num++;
	visit_inc_needed_next = AssignBFS(dn, m->visit_num);
      }

      /* if dn is assigned, report it to m and quit if lower bound reached */
      if( dn->demand_asst != NULL )
      {
	RemoveUnMatchedDemandNode(m, dn);
        if( m->unmatched_lower_bound == MArraySize(m->unmatched_demand_nodes) )
	  break;
      }
      KheDemandNodeCheck(dn);
      if( DEBUG1 )
	CheckInvt(m);
    }

    /* bring lower bound up to date and make sure there are no Hall sets */
    m->unmatched_lower_bound = MArraySize(m->unmatched_demand_nodes);
    while( MArraySize(m->hall_sets) > 0 )
      KheMatchingHallSetFree(MArrayRemoveLast(m->hall_sets));
    if( DEBUG1 )
      CheckInvt(m);
  }
}


/*****************************************************************************/
/*                                                                           */
/*  KHE_MATCHING_SUPPLY_CHUNK                                                */
/*                                                                           */
/*****************************************************************************/

/*****************************************************************************/
/*                                                                           */
/*  KHE_MATCHING_SUPPLY_CHUNK KheMatchingSupplyChunkMake(KHE_MATCHING m,     */
/*    void *impl)                                                            */
/*                                                                           */
/*  Make a supply chunk with these attributes.                               */
/*                                                                           */
/*****************************************************************************/

KHE_MATCHING_SUPPLY_CHUNK KheMatchingSupplyChunkMake(KHE_MATCHING m, void *impl)
{
  KHE_MATCHING_SUPPLY_CHUNK res;
  MMake(res);
  res->matching = m;
  res->impl = impl;
  res->base = KheMatchingSupplyNodeCount(m);
  MArrayInit(res->supply_nodes);
  MArrayAddLast(m->supply_chunks, res);
  res->copy = NULL;
  return res;
}


/*****************************************************************************/
/*                                                                           */
/*  void KheMatchingSupplyChunkFree(KHE_MATCHING_SUPPLY_CHUNK sc)            */
/*                                                                           */
/*  Free sc and its supply nodes.                                            */
/*                                                                           */
/*****************************************************************************/
static void KheMatchingSupplyNodeFree(KHE_MATCHING_SUPPLY_NODE sn);

static void KheMatchingSupplyChunkFree(KHE_MATCHING_SUPPLY_CHUNK sc)
{
  while( MArraySize(sc->supply_nodes) > 0 )
    KheMatchingSupplyNodeFree(MArrayRemoveLast(sc->supply_nodes));
  MArrayFree(sc->supply_nodes);
  MFree(sc);
}


/*****************************************************************************/
/*                                                                           */
/*  KHE_MATCHING KheMatchingSupplyChunkMatching(KHE_MATCHING_SUPPLY_CHUNK sc)*/
/*                                                                           */
/*  Return the matching attribute of sc.                                     */
/*                                                                           */
/*****************************************************************************/

KHE_MATCHING KheMatchingSupplyChunkMatching(KHE_MATCHING_SUPPLY_CHUNK sc)
{
  return sc->matching;
}


/*****************************************************************************/
/*                                                                           */
/*  void *KheMatchingSupplyChunkImpl(KHE_MATCHING_SUPPLY_CHUNK sc)           */
/*                                                                           */
/*  Return the impl attribute of sc.                                         */
/*                                                                           */
/*****************************************************************************/

void *KheMatchingSupplyChunkImpl(KHE_MATCHING_SUPPLY_CHUNK sc)
{
  return sc->impl;
}


/*****************************************************************************/
/*                                                                           */
/*  void KheMatchingSupplyChunkSetImpl(KHE_MATCHING_SUPPLY_CHUNK sc,         */
/*    void *impl)                                                            */
/*                                                                           */
/*  Reset the impl pointer of sc.                                            */
/*                                                                           */
/*****************************************************************************/

void KheMatchingSupplyChunkSetImpl(KHE_MATCHING_SUPPLY_CHUNK sc, void *impl)
{
  sc->impl = impl;
}


/*****************************************************************************/
/*                                                                           */
/*  int KheMatchingSupplyChunkBase(KHE_MATCHING_SUPPLY_CHUNK sc)             */
/*                                                                           */
/*  Return the base of sc.  This is the number of supply nodes that          */
/*  existed in sc's matching at the moment sc was created.                   */
/*                                                                           */
/*****************************************************************************/

int KheMatchingSupplyChunkBase(KHE_MATCHING_SUPPLY_CHUNK sc)
{
  return sc->base;
}


/*****************************************************************************/
/*                                                                           */
/*  KHE_MATCHING_SUPPLY_CHUNK KheMatchingSupplyChunkCopyPhase1(              */
/*    KHE_MATCHING_SUPPLY_CHUNK sc)                                          */
/*                                                                           */
/*  Carry out Phase 1 of copying sc.                                         */
/*                                                                           */
/*****************************************************************************/

KHE_MATCHING_SUPPLY_CHUNK KheMatchingSupplyChunkCopyPhase1(
  KHE_MATCHING_SUPPLY_CHUNK sc)
{
  KHE_MATCHING_SUPPLY_CHUNK copy;  KHE_MATCHING_SUPPLY_NODE sn;  int i;
  if( sc->copy == NULL )
  {
    MMake(copy);
    sc->copy = copy;
    copy->matching = KheMatchingCopyPhase1(sc->matching);
    copy->impl = sc->impl;
    copy->base = sc->base;
    MArrayInit(copy->supply_nodes);
    MArrayForEach(sc->supply_nodes, &sn, &i)
      MArrayAddLast(copy->supply_nodes, KheMatchingSupplyNodeCopyPhase1(sn));
    copy->copy = NULL;
  }
  return sc->copy;
}


/*****************************************************************************/
/*                                                                           */
/*  void KheMatchingSupplyChunkCopyPhase2(KHE_MATCHING_SUPPLY_CHUNK sc)      */
/*                                                                           */
/*  Carry out Phase 2 of copying sc.                                         */
/*                                                                           */
/*****************************************************************************/

void KheMatchingSupplyChunkCopyPhase2(KHE_MATCHING_SUPPLY_CHUNK sc)
{
  KHE_MATCHING_SUPPLY_NODE sn;  int i;
  if( sc->copy != NULL )
  {
    sc->copy = NULL;
    MArrayForEach(sc->supply_nodes, &sn, &i)
      KheMatchingSupplyNodeCopyPhase2(sn);
  }
}


/*****************************************************************************/
/*                                                                           */
/*  int KheMatchingSupplyChunkSupplyNodeCount(KHE_MATCHING_SUPPLY_CHUNK sc)  */
/*                                                                           */
/*  Return the number of supply nodes in sc.                                 */
/*                                                                           */
/*****************************************************************************/

int KheMatchingSupplyChunkSupplyNodeCount(KHE_MATCHING_SUPPLY_CHUNK sc)
{
  return MArraySize(sc->supply_nodes);
}


/*****************************************************************************/
/*                                                                           */
/*  KHE_MATCHING_SUPPLY_NODE KheMatchingSupplyChunkSupplyNode(               */
/*    KHE_MATCHING_SUPPLY_CHUNK sc, int i)                                   */
/*                                                                           */
/*  Return the i'th supply node of sc.                                       */
/*                                                                           */
/*****************************************************************************/

KHE_MATCHING_SUPPLY_NODE KheMatchingSupplyChunkSupplyNode(
  KHE_MATCHING_SUPPLY_CHUNK sc, int i)
{
  return MArrayGet(sc->supply_nodes, i);
}


/*****************************************************************************/
/*                                                                           */
/*  KHE_MATCHING_SUPPLY_NODE                                                 */
/*                                                                           */
/*****************************************************************************/

/*****************************************************************************/
/*                                                                           */
/*  KHE_MATCHING_SUPPLY_NODE KheMatchingSupplyNodeMake(                      */
/*    KHE_MATCHING_SUPPLY_CHUNK sc, void *impl)                              */
/*                                                                           */
/*  Make a supply node with these attributes and add it to sc.               */
/*                                                                           */
/*****************************************************************************/

KHE_MATCHING_SUPPLY_NODE KheMatchingSupplyNodeMake(
  KHE_MATCHING_SUPPLY_CHUNK sc, void *impl)
{
  KHE_MATCHING_SUPPLY_NODE res;
  MMake(res);
  res->supply_chunk = sc;
  res->impl = impl;
  res->index = MArraySize(sc->matching->supply_nodes);
  MArrayAddLast(sc->matching->supply_nodes, res);
  MArrayAddLast(sc->supply_nodes, res);
  res->visit_num = 0;
  res->supply_asst = NULL;
  res->hall_set = NULL;
  res->demand_test_index = 0;
  res->copy = NULL;
  return res;
}


/*****************************************************************************/
/*                                                                           */
/*  void KheMatchingSupplyNodeFree(KHE_MATCHING_SUPPLY_NODE sn)              */
/*                                                                           */
/*  Free sn.                                                                 */
/*                                                                           */
/*****************************************************************************/

static void KheMatchingSupplyNodeFree(KHE_MATCHING_SUPPLY_NODE sn)
{
  MFree(sn);
}


/*****************************************************************************/
/*                                                                           */
/*  KHE_MATCHING_SUPPLY_CHUNK KheMatchingSupplyNodeChunk(                    */
/*    KHE_MATCHING_SUPPLY_NODE sn)                                           */
/*                                                                           */
/*  Return the supply_chunk attribute of sn.                                 */
/*                                                                           */
/*****************************************************************************/

KHE_MATCHING_SUPPLY_CHUNK KheMatchingSupplyNodeChunk(
  KHE_MATCHING_SUPPLY_NODE sn)
{
  return sn->supply_chunk;
}


/*****************************************************************************/
/*                                                                           */
/*  KHE_MATCHING KheMatchingSupplyNodeMatching(KHE_MATCHING_SUPPLY_NODE sn)  */
/*                                                                           */
/*  Return the matching containing sn.                                       */
/*                                                                           */
/*****************************************************************************/

KHE_MATCHING KheMatchingSupplyNodeMatching(KHE_MATCHING_SUPPLY_NODE sn)
{
  return sn->supply_chunk->matching;
}


/*****************************************************************************/
/*                                                                           */
/*  void *KheMatchingSupplyNodeImpl(KHE_MATCHING_SUPPLY_NODE sn)             */
/*                                                                           */
/*  Return the impl attribute of sn.                                         */
/*                                                                           */
/*****************************************************************************/

void *KheMatchingSupplyNodeImpl(KHE_MATCHING_SUPPLY_NODE sn)
{
  return sn->impl;
}


/*****************************************************************************/
/*                                                                           */
/*  void KheMatchingSupplyNodeSetImpl(KHE_MATCHING_SUPPLY_NODE sn,           */
/*    void *impl)                                                            */
/*                                                                           */
/*  Reset the impl attribute of sn.                                          */
/*                                                                           */
/*****************************************************************************/

void KheMatchingSupplyNodeSetImpl(KHE_MATCHING_SUPPLY_NODE sn, void *impl)
{
  sn->impl = impl;
}


/*****************************************************************************/
/*                                                                           */
/*  int KheMatchingSupplyNodeIndex(KHE_MATCHING_SUPPLY_NODE sn)              */
/*                                                                           */
/*  Return the index number attribute of sn.  This is just the number        */
/*  of supply nodes present just before sn was created.                      */
/*                                                                           */
/*****************************************************************************/

int KheMatchingSupplyNodeIndex(KHE_MATCHING_SUPPLY_NODE sn)
{
  return sn->index;
}


/*****************************************************************************/
/*                                                                           */
/*  KHE_MATCHING_SUPPLY_NODE KheMatchingSupplyNodeCopyPhase1(                */
/*    KHE_MATCHING_SUPPLY_NODE sn)                                           */
/*                                                                           */
/*  Carry out Phase 1 of copying sn.                                         */
/*                                                                           */
/*****************************************************************************/

KHE_MATCHING_SUPPLY_NODE KheMatchingSupplyNodeCopyPhase1(
  KHE_MATCHING_SUPPLY_NODE sn)
{
  KHE_MATCHING_SUPPLY_NODE copy;
  if( sn->copy == NULL )
  {
    MMake(copy);
    sn->copy = copy;
    copy->supply_chunk = KheMatchingSupplyChunkCopyPhase1(sn->supply_chunk);
    copy->impl = sn->impl;
    copy->index = sn->index;
    copy->visit_num = sn->visit_num;
    copy->supply_asst = (sn->supply_asst == NULL ? NULL :
      KheMatchingDemandNodeCopyPhase1(sn->supply_asst));
    copy->hall_set = sn->hall_set == NULL ? NULL :
      KheMatchingHallSetCopyPhase1(sn->hall_set);
    copy->demand_test_index = sn->demand_test_index;
    copy->copy = NULL;
  }
  return sn->copy;
}


/*****************************************************************************/
/*                                                                           */
/*  void KheMatchingSupplyNodeCopyPhase2(KHE_MATCHING_SUPPLY_NODE sn)        */
/*                                                                           */
/*  Carry out Phase 2 of copying sn.                                         */
/*                                                                           */
/*****************************************************************************/

void KheMatchingSupplyNodeCopyPhase2(KHE_MATCHING_SUPPLY_NODE sn)
{
  if( sn->copy != NULL )
  {
    sn->copy = NULL;
    KheMatchingSupplyChunkCopyPhase2(sn->supply_chunk);
    if( sn->supply_asst != NULL )
      KheMatchingDemandNodeCopyPhase2(sn->supply_asst);
    if( sn->hall_set != NULL )
      KheMatchingHallSetCopyPhase2(sn->hall_set);
  }
}


/*****************************************************************************/
/*                                                                           */
/*  KHE_MATCHING_DEMAND_CHUNK                                                */
/*                                                                           */
/*****************************************************************************/

/*****************************************************************************/
/*                                                                           */
/*  KHE_MATCHING_DEMAND_CHUNK KheMatchingDemandChunkMake(KHE_MATCHING m,     */
/*    void *impl, int base, int increment, ARRAY_SHORT domain)               */
/*                                                                           */
/*  Make a demand chunk with these attributes.                               */
/*                                                                           */
/*****************************************************************************/

KHE_MATCHING_DEMAND_CHUNK KheMatchingDemandChunkMake(KHE_MATCHING m,
  void *impl, int base, int increment, ARRAY_SHORT domain)
{
  KHE_MATCHING_DEMAND_CHUNK res;
  MMake(res);
  res->matching = m;
  res->index_in_matching = MArraySize(m->demand_chunks);
  res->impl = impl;
  res->base = base;
  res->increment = increment;
  res->domain = domain;
  MArrayInit(res->demand_nodes);
  MArrayAddLast(m->demand_chunks, res);
  res->copy = NULL;
  return res;
}


/*****************************************************************************/
/*                                                                           */
/*  void KheMatchingDemandChunkFree(KHE_MATCHING_DEMAND_CHUNK dc)            */
/*                                                                           */
/*  Free dc and its demand nodes, but not its domain, since that comes in    */
/*  from outside and could be shared.                                        */
/*                                                                           */
/*****************************************************************************/

/* *** see KheMatchingDemandChunkDelete below
static void KheMatchingDemandNodeFree(KHE_MATCHING_DEMAND_NODE dn);

static void KheMatchingDemandChunkFree(KHE_MATCHING_DEMAND_CHUNK dc)
{
  while( MArraySize(dc->demand_nodes) > 0 )
    KheMatchingDemandNodeFree(MArrayRemoveLast(dc->demand_nodes));
  MArrayFree(dc->demand_nodes);
  MFree(dc);
}
*** */


/*****************************************************************************/
/*                                                                           */
/*  KHE_MATCHING KheMatchingDemandChunkMatching(KHE_MATCHING_DEMAND_CHUNK dc)*/
/*                                                                           */
/*  Return the matching attribute of dc.                                     */
/*                                                                           */
/*****************************************************************************/

KHE_MATCHING KheMatchingDemandChunkMatching(KHE_MATCHING_DEMAND_CHUNK dc)
{
  return dc->matching;
}


/*****************************************************************************/
/*                                                                           */
/*  void *KheMatchingDemandChunkImpl(KHE_MATCHING_DEMAND_CHUNK dc)           */
/*                                                                           */
/*  Return the impl attribute of dc.                                         */
/*                                                                           */
/*****************************************************************************/

void *KheMatchingDemandChunkImpl(KHE_MATCHING_DEMAND_CHUNK dc)
{
  return dc->impl;
}


/*****************************************************************************/
/*                                                                           */
/*  void KheMatchingDemandChunkSetImpl(KHE_MATCHING_DEMAND_CHUNK dc,         */
/*    void *impl)                                                            */
/*                                                                           */
/*  Reset the impl attribute of dc.                                          */
/*                                                                           */
/*****************************************************************************/

void KheMatchingDemandChunkSetImpl(KHE_MATCHING_DEMAND_CHUNK dc, void *impl)
{
  dc->impl = impl;
}


/*****************************************************************************/
/*                                                                           */
/*  void KheMatchingDemandChunkDelete(KHE_MATCHING_DEMAND_CHUNK dc)          */
/*                                                                           */
/*  Delete dc and free its memory.  It must have no demand nodes.            */
/*                                                                           */
/*****************************************************************************/

void KheMatchingDemandChunkDelete(KHE_MATCHING_DEMAND_CHUNK dc)
{
  KHE_MATCHING_DEMAND_CHUNK tmp;
  MAssert(MArraySize(dc->demand_nodes) == 0,
    "KheMatchingDemandChunkDelete: dc contains demand nodes");
  tmp = MArrayRemoveAndPlug(dc->matching->demand_chunks, dc->index_in_matching);
  tmp->index_in_matching = dc->index_in_matching;
  MArrayFree(dc->demand_nodes);
  MFree(dc);
}


/*****************************************************************************/
/*                                                                           */
/*  KHE_MATCHING_DEMAND_CHUNK KheMatchingDemandChunkCopyPhase1(              */
/*    KHE_MATCHING_DEMAND_CHUNK dc)                                          */
/*                                                                           */
/*  Carry out Phase 1 of copying dc.                                         */
/*                                                                           */
/*****************************************************************************/

KHE_MATCHING_DEMAND_CHUNK KheMatchingDemandChunkCopyPhase1(
  KHE_MATCHING_DEMAND_CHUNK dc)
{
  KHE_MATCHING_DEMAND_CHUNK copy;  KHE_MATCHING_DEMAND_NODE dn;  int i;
  if( dc->copy == NULL )
  {
    MMake(copy);
    dc->copy = copy;
    copy->matching = KheMatchingCopyPhase1(dc->matching);
    copy->index_in_matching = dc->index_in_matching;
    copy->impl = dc->impl;
    copy->base = dc->base;
    copy->increment = dc->increment;
    copy->domain = dc->domain;
    MArrayInit(copy->demand_nodes);
    MArrayForEach(dc->demand_nodes, &dn, &i)
      MArrayAddLast(copy->demand_nodes, KheMatchingDemandNodeCopyPhase1(dn));
    copy->copy = NULL;
  }
  return dc->copy;
}


/*****************************************************************************/
/*                                                                           */
/*  void KheMatchingDemandChunkCopyPhase2(KHE_MATCHING_DEMAND_CHUNK dc)      */
/*                                                                           */
/*  Carry out Phase 2 of copying dc.                                         */
/*                                                                           */
/*****************************************************************************/

void KheMatchingDemandChunkCopyPhase2(KHE_MATCHING_DEMAND_CHUNK dc)
{
  KHE_MATCHING_DEMAND_NODE dn;  int i;
  if( dc->copy != NULL )
  {
    dc->copy = NULL;
    KheMatchingCopyPhase2(dc->matching);
    MArrayForEach(dc->demand_nodes, &dn, &i)
      KheMatchingDemandNodeCopyPhase2(dn);
  }
}


/*****************************************************************************/
/*                                                                           */
/*  int KheMatchingDemandChunkBase(KHE_MATCHING_DEMAND_CHUNK dc)             */
/*                                                                           */
/*  Return the base attribute of dc.                                         */
/*                                                                           */
/*****************************************************************************/

int KheMatchingDemandChunkBase(KHE_MATCHING_DEMAND_CHUNK dc)
{
  return dc->base;
}


/*****************************************************************************/
/*                                                                           */
/*  int KheMatchingDemandChunkIncrement(KHE_MATCHING_DEMAND_CHUNK dc)        */
/*                                                                           */
/*  Return the increment attribute of dc.                                    */
/*                                                                           */
/*****************************************************************************/

int KheMatchingDemandChunkIncrement(KHE_MATCHING_DEMAND_CHUNK dc)
{
  return dc->increment;
}


/*****************************************************************************/
/*                                                                           */
/*  ARRAY_SHORT KheMatchingDemandChunkDomain(KHE_MATCHING_DEMAND_CHUNK dc)   */
/*                                                                           */
/*  Return the domain attribute of dc.                                       */
/*                                                                           */
/*****************************************************************************/

ARRAY_SHORT KheMatchingDemandChunkDomain(KHE_MATCHING_DEMAND_CHUNK dc)
{
  return dc->domain;
}


/*****************************************************************************/
/*                                                                           */
/*  void KheMatchingDemandChunkSetBase(KHE_MATCHING_DEMAND_CHUNK dc,         */
/*    int base)                                                              */
/*                                                                           */
/*  Set the base attribute of dc.                                            */
/*                                                                           */
/*****************************************************************************/
static void KheMatchingDemandNodeDomainHasChanged(KHE_MATCHING_DEMAND_NODE dn,
  KHE_MATCHING_DOMAIN_CHANGE_TYPE domain_change_type);

void KheMatchingDemandChunkSetBase(KHE_MATCHING_DEMAND_CHUNK dc, int base)
{
  KHE_MATCHING_DEMAND_NODE dn;  int i;
  MAssert(!dc->matching->active, "KheMatchingDemandChunkSetBase reentry in %p",
    dc->matching->impl);
  dc->matching->active = true;
  if( base != dc->base )
  {
    dc->base = base;
    MArrayForEach(dc->demand_nodes, &dn, &i)
      KheMatchingDemandNodeDomainHasChanged(dn,
	KHE_MATCHING_DOMAIN_CHANGE_TO_OTHER);
  }
  dc->matching->active = false;
}


/*****************************************************************************/
/*                                                                           */
/*  void KheMatchingDemandChunkSetIncrement(KHE_MATCHING_DEMAND_CHUNK dc,    */
/*    int increment)                                                         */
/*                                                                           */
/*  Set the increment attribute of dc.                                       */
/*                                                                           */
/*****************************************************************************/

void KheMatchingDemandChunkSetIncrement(KHE_MATCHING_DEMAND_CHUNK dc,
  int increment)
{
  KHE_MATCHING_DEMAND_NODE dn;  int i;
  MAssert(!dc->matching->active,
    "KheMatchingDemandChunkSetIncrement reentry in %p", dc->matching->impl);
  dc->matching->active = true;
  if( increment != dc->increment )
  {
    dc->increment = increment;
    MArrayForEach(dc->demand_nodes, &dn, &i)
      KheMatchingDemandNodeDomainHasChanged(dn,
	KHE_MATCHING_DOMAIN_CHANGE_TO_OTHER);
  }
  dc->matching->active = false;
}


/*****************************************************************************/
/*                                                                           */
/*  void KheMatchingDemandChunkSetDomain(KHE_MATCHING_DEMAND_CHUNK dc,       */
/*    ARRAY_SHORT domain, KHE_MATCHING_DOMAIN_CHANGE_TYPE domain_change_type)*/
/*                                                                           */
/*  Set the domain attribute of dc.                                          */
/*                                                                           */
/*****************************************************************************/

void KheMatchingDemandChunkSetDomain(KHE_MATCHING_DEMAND_CHUNK dc,
  ARRAY_SHORT domain, KHE_MATCHING_DOMAIN_CHANGE_TYPE domain_change_type)
{
  KHE_MATCHING_DEMAND_NODE dn;  int i;
  MAssert(!dc->matching->active, "KheMatchingDemandChunkSetBase reentry in %p",
    dc->matching->impl);
  dc->matching->active = true;
  dc->domain = domain;
  MArrayForEach(dc->demand_nodes, &dn, &i)
    KheMatchingDemandNodeDomainHasChanged(dn, domain_change_type);
  dc->matching->active = false;
}


/*****************************************************************************/
/*                                                                           */
/*  int KheMatchingDemandChunkNodeCount(KHE_MATCHING_DEMAND_CHUNK dc)        */
/*                                                                           */
/*  Return the number of demand nodes in dc.                                 */
/*                                                                           */
/*****************************************************************************/

int KheMatchingDemandChunkNodeCount(KHE_MATCHING_DEMAND_CHUNK dc)
{
  return MArraySize(dc->demand_nodes);
}


/*****************************************************************************/
/*                                                                           */
/*  KHE_MATCHING_DEMAND_NODE KheMatchingDemandChunkNode(                     */
/*    KHE_MATCHING_DEMAND_CHUNK dc, int i)                                   */
/*                                                                           */
/*  Return the i'th demand node of dc.                                       */
/*                                                                           */
/*****************************************************************************/

KHE_MATCHING_DEMAND_NODE KheMatchingDemandChunkNode(
  KHE_MATCHING_DEMAND_CHUNK dc, int i)
{
  return MArrayGet(dc->demand_nodes, i);
}


/*****************************************************************************/
/*                                                                           */
/*  KHE_MATCHING_DEMAND_NODE                                                 */
/*                                                                           */
/*****************************************************************************/

/*****************************************************************************/
/*                                                                           */
/*  KHE_MATCHING_DEMAND_NODE KheMatchingDemandNodeMake(                      */
/*    KHE_MATCHING_DEMAND_CHUNK dc, void *impl, ARRAY_SHORT domain)          */
/*                                                                           */
/*  Make a demand node with these attributes.                                */
/*                                                                           */
/*****************************************************************************/

/* *** KHE_MATCHING_DEMAND_NODE is an abstract supertype now
KHE_MATCHING_DEMAND_NODE KheMatchingDemandNodeMake(
  KHE_MATCHING_DEMAND_CHUNK dc, void *impl, ARRAY_SHORT domain)
{
  KHE_MATCHING_DEMAND_NODE res;
  MMake(res);
  res->demand_chunk = dc;
  res->impl = impl;
  res->domain = domain;
  res->demand_asst = NULL;
  res->demand_asst_index = NO_PREV_ASST;
  res->unmatched_pos = -1;  ** undefined at this point **
  res->bfs_next = NULL;
  res->bfs_parent = NULL;
  res->hall_set = NULL;
  MArrayAddLast(dc->demand_nodes, res);
  dc->matching->demand_node_count++;
  AddUnMatchedDemandNode(dc->matching, res);
  return res;
}
*** */


/*****************************************************************************/
/*                                                                           */
/*  void KheMatchingDemandNodeFree(KHE_MATCHING_DEMAND_NODE dn)              */
/*                                                                           */
/*  Free dn, but not its domain, since that comes from outside so could      */
/*  be shared.                                                               */
/*                                                                           */
/*****************************************************************************/

/* *** KHE_MATCHING_DEMAND_NODE is an abstract supertype now
static void KheMatchingDemandNodeFree(KHE_MATCHING_DEMAND_NODE dn)
{
  MFree(dn);
}
*** */


/*****************************************************************************/
/*                                                                           */
/*  KHE_MATCHING_DEMAND_CHUNK KheMatchingDemandNodeChunk(                    */
/*    KHE_MATCHING_DEMAND_NODE dn)                                           */
/*                                                                           */
/*  Return the demand chunk attribute of dn.                                 */
/*                                                                           */
/*****************************************************************************/

KHE_MATCHING_DEMAND_CHUNK KheMatchingDemandNodeChunk(
  KHE_MATCHING_DEMAND_NODE dn)
{
  return dn->demand_chunk;
}


/*****************************************************************************/
/*                                                                           */
/*  KHE_MATCHING_DEMAND_NODE KheMatchingDemandNodeCopyPhase1(                */
/*    KHE_MATCHING_DEMAND_NODE dn)                                           */
/*                                                                           */
/*  Carry out Phase 1 of copying dn.                                         */
/*                                                                           */
/*****************************************************************************/

KHE_MATCHING_DEMAND_NODE KheMatchingDemandNodeCopyPhase1(
  KHE_MATCHING_DEMAND_NODE dn)
{
  if( dn->tag == KHE_ORDINARY_DEMAND_MONITOR_TAG )
    return (KHE_MATCHING_DEMAND_NODE) KheOrdinaryDemandMonitorCopyPhase1(
      (KHE_ORDINARY_DEMAND_MONITOR) dn);
  else if( dn->tag == KHE_WORKLOAD_DEMAND_MONITOR_TAG )
    return (KHE_MATCHING_DEMAND_NODE) KheWorkloadDemandMonitorCopyPhase1(
      (KHE_WORKLOAD_DEMAND_MONITOR) dn);
  else
  {
    MAssert(false, "KheMatchingDemandNodeCopyPhase1 internal error");
    return NULL;
  }
}


/*****************************************************************************/
/*                                                                           */
/*  void KheMatchingDemandNodeCopyPhase2(KHE_MATCHING_DEMAND_NODE dn)        */
/*                                                                           */
/*  Carry out Phase 2 of copying dn.                                         */
/*                                                                           */
/*****************************************************************************/

void KheMatchingDemandNodeCopyPhase2(KHE_MATCHING_DEMAND_NODE dn)
{
  if( dn->tag == KHE_ORDINARY_DEMAND_MONITOR_TAG )
    KheOrdinaryDemandMonitorCopyPhase2((KHE_ORDINARY_DEMAND_MONITOR) dn);
  else if( dn->tag == KHE_WORKLOAD_DEMAND_MONITOR_TAG )
    KheWorkloadDemandMonitorCopyPhase2((KHE_WORKLOAD_DEMAND_MONITOR) dn);
  else
    MAssert(false, "KheMatchingDemandNodeCopyPhase2 internal error");
}


/*****************************************************************************/
/*                                                                           */
/*  void *KheMatchingDemandNodeImpl(KHE_MATCHING_DEMAND_NODE dn)             */
/*                                                                           */
/*  Return the impl attribute of dn.                                         */
/*                                                                           */
/*****************************************************************************/

/* ***
void *KheMatchingDemandNodeImpl(KHE_MATCHING_DEMAND_NODE dn)
{
  return dn->impl;
}
*** */


/*****************************************************************************/
/*                                                                           */
/*  void KheMatchingDemandNodeSetImpl(KHE_MATCHING_DEMAND_NODE dn,           */
/*    void *impl)                                                            */
/*                                                                           */
/*  Reset the impl attribute of dn.                                          */
/*                                                                           */
/*****************************************************************************/

/* ***
void KheMatchingDemandNodeSetImpl(KHE_MATCHING_DEMAND_NODE dn, void *impl)
{
  dn->impl = impl;
}
*** */


/*****************************************************************************/
/*                                                                           */
/*  void KheMatchingDemandChunkAddDemandNode(KHE_MATCHING_DEMAND_CHUNK dc,   */
/*    KHE_MATCHING_DEMAND_NODE dn)                                           */
/*                                                                           */
/*  Add dn to dc.                                                            */
/*                                                                           */
/*****************************************************************************/

void KheMatchingDemandNodeAdd(KHE_MATCHING_DEMAND_NODE dn)
{
  MAssert(!dn->demand_chunk->matching->active,
    "KheMatchingDemandNodeAdd reentry in %p", dn->demand_chunk->matching->impl);
  dn->demand_chunk->matching->active = true;
  MArrayAddLast(dn->demand_chunk->demand_nodes, dn);
  dn->demand_chunk->matching->demand_node_count++;
  AddUnMatchedDemandNode(dn->demand_chunk->matching, dn);
  dn->demand_chunk->matching->active = false;
}


/*****************************************************************************/
/*                                                                           */
/*  void KheMatchingDemandNodeDelete(KHE_MATCHING_DEMAND_NODE dn)            */
/*                                                                           */
/*  Delete dn, but do not free it.                                           */
/*                                                                           */
/*****************************************************************************/

void KheMatchingDemandNodeDelete(KHE_MATCHING_DEMAND_NODE dn)
{
  int pos;  KHE_MATCHING m;

  /* decrement the total number of demand nodes */
  m = dn->demand_chunk->matching;
  MAssert(!m->active, "KheMatchingDemandNodeDelete reentry in %p", m->impl);
  m->active = true;
  m->demand_node_count--;

  /* ensure dn is deassigned and not on the unmatched demand node list */
  if( dn->demand_asst != NULL )
  {
    dn->demand_asst->supply_asst = NULL;
    dn->demand_asst = NULL;
  }
  else
    RemoveUnMatchedDemandNode(m, dn);

  /* remove dn from its chunk */
  if( !MArrayContains(dn->demand_chunk->demand_nodes, dn, &pos) )
    MAssert(false, "KheMatchingDemandNodeDelete internal error");
  MArrayRemove(dn->demand_chunk->demand_nodes, pos);

  /* reduce m's lower bound */
  if( m->unmatched_lower_bound > 0 )
    m->unmatched_lower_bound--;
  m->active = false;
}


/*****************************************************************************/
/*                                                                           */
/*  ARRAY_SHORT KheMatchingDemandNodeDomain(KHE_MATCHING_DEMAND_NODE dn)     */
/*                                                                           */
/*  Return the domain attribute of dn.                                       */
/*                                                                           */
/*****************************************************************************/

ARRAY_SHORT KheMatchingDemandNodeDomain(KHE_MATCHING_DEMAND_NODE dn)
{
  return dn->domain;
}


/*****************************************************************************/
/*                                                                           */
/*  void KheMatchingDemandNodeDomainHasChanged(KHE_MATCHING_DEMAND_NODE dn,  */
/*    KHE_MATCHING_DOMAIN_CHANGE_TYPE domain_change_type)                    */
/*                                                                           */
/*  Update lower bounds and assignments appropriately, given that the        */
/*  domain of dn has just changed according to domain_change_type.           */
/*                                                                           */
/*****************************************************************************/

static void KheMatchingDemandNodeDomainHasChanged(KHE_MATCHING_DEMAND_NODE dn,
  KHE_MATCHING_DOMAIN_CHANGE_TYPE domain_change_type)
{
  KHE_MATCHING m;

  /* decrease lower bound unless new domain is subset or already at 0 */
  m = dn->demand_chunk->matching;
  if( domain_change_type != KHE_MATCHING_DOMAIN_CHANGE_TO_SUBSET &&
      m->unmatched_lower_bound > 0 )
    m->unmatched_lower_bound--;

  /* if assigned and not changing to a superset, deassign */
  if( dn->demand_asst != NULL &&
      domain_change_type != KHE_MATCHING_DOMAIN_CHANGE_TO_SUPERSET )
  {
    dn->demand_asst->supply_asst = NULL;
    dn->demand_asst = NULL;
    AddUnMatchedDemandNode(m, dn);
  }
}


/*****************************************************************************/
/*                                                                           */
/*  void KheMatchingDemandNodeSetDomain(KHE_MATCHING_DEMAND_NODE dn,         */
/*    ARRAY_SHORT domain, KHE_MATCHING_DOMAIN_CHANGE_TYPE domain_change_type)*/
/*                                                                           */
/*  Set the domain attribute of dn.                                          */
/*                                                                           */
/*****************************************************************************/

void KheMatchingDemandNodeSetDomain(KHE_MATCHING_DEMAND_NODE dn,
  ARRAY_SHORT domain, KHE_MATCHING_DOMAIN_CHANGE_TYPE domain_change_type)
{
  MAssert(!dn->demand_chunk->matching->active,
    "KheMatchingDemandNodeSetDomain reentry in %p",
    dn->demand_chunk->matching->impl);
  dn->demand_chunk->matching->active = true;
  dn->domain = domain;
  KheMatchingDemandNodeDomainHasChanged(dn, domain_change_type);
  dn->demand_chunk->matching->active = false;
}


/*****************************************************************************/
/*                                                                           */
/*  KHE_MATCHING                                                             */
/*                                                                           */
/*****************************************************************************/

/*****************************************************************************/
/*                                                                           */
/*  KHE_MATCHING KheMatchingMake(void *impl)                                 */
/*                                                                           */
/*  Make a new, empty bipartite matching graph with these attributes.        */
/*                                                                           */
/*****************************************************************************/

KHE_MATCHING KheMatchingMake(void *impl)
{
  KHE_MATCHING res;
  MMake(res);
  res->impl = impl;
  MArrayInit(res->supply_chunks);
  MArrayInit(res->demand_chunks);
  MArrayInit(res->supply_nodes);
  res->demand_node_count = 0;
  res->visit_num = 0;
  MArrayInit(res->unmatched_demand_nodes);
  res->unmatched_lower_bound = 0;
  res->active = false;
  MArrayInit(res->marks);
  MArrayInit(res->hall_sets);
  res->comp_bfs_first = NULL;
  res->comp_bfs_last = NULL;
  res->copy = NULL;
  return res;
}


/*****************************************************************************/
/*                                                                           */
/*  void *KheMatchingImpl(KHE_MATCHING m)                                    */
/*                                                                           */
/*  Return the impl attribute of m.                                          */
/*                                                                           */
/*****************************************************************************/

void *KheMatchingImpl(KHE_MATCHING m)
{
  return m->impl;
}


/*****************************************************************************/
/*                                                                           */
/*  void KheMatchingSetImpl(KHE_MATCHING m, void *impl)                      */
/*                                                                           */
/*  Reset the impl pointer of m.                                             */
/*                                                                           */
/*****************************************************************************/

void KheMatchingSetImpl(KHE_MATCHING m, void *impl)
{
  m->impl = impl;
}


/*****************************************************************************/
/*                                                                           */
/*  ARRAY_SHORT KheMatchingZeroDomain(KHE_MATCHING m)                        */
/*                                                                           */
/*  Return a domain whose value is the set {0} (not the empty set!).         */
/*                                                                           */
/*****************************************************************************/

/* ***
ARRAY_SHORT KheMatchingZeroDomain(KHE_MATCHING m)
{
  return m->zero_domain;
}
*** */


/*****************************************************************************/
/*                                                                           */
/*  void KheMatchingDelete(KHE_MATCHING m)                                   */
/*                                                                           */
/*  Free m and all its chunks and nodes.                                     */
/*                                                                           */
/*  Implementation note.  Deletion requires a plan which guarantees that     */
/*  everything is deleted exactly once.  Here is the plan for matchings:     */
/*                                                                           */
/*    KHE_MATCHING_SUPPLY_CHUNK                                              */
/*    KHE_MATCHING_SUPPLY_NODE                                               */
/*                                                                           */
/*  As solution events are deleted, these are moved onto the free list in    */
/*  the solution object, but they are not actually freed.  This function     */
/*  frees all supply chunks, and each chunk frees its supply nodes.          */
/*                                                                           */
/*    KHE_MATCHING_DEMAND_CHUNK                                              */
/*    KHE_MATCHING_DEMAND_NODE                                               */
/*                                                                           */
/*  As solution events are deleted, these are both deleted and freed.  So    */
/*  there should be none left by the time KheMatchingDelete is called.       */
/*                                                                           */
/*    KHE_MATCHING                                                           */
/*    KHE_MATCHING_HALL_SET                                                  */
/*                                                                           */
/*  Freed by KheMatchingDelete.                                              */
/*                                                                           */
/*****************************************************************************/
static void KheMatchingHallSetFree(KHE_MATCHING_HALL_SET hs);

void KheMatchingDelete(KHE_MATCHING m)
{
  /* MArrayFree(m->zero_domain); */
  MAssert(MArraySize(m->demand_chunks) == 0,
    "KheMatchingDelete internal error 1");
  MAssert(MArraySize(m->unmatched_demand_nodes) == 0,
    "KheMatchingDelete internal error 2");
  while( MArraySize(m->supply_chunks) > 0 )
    KheMatchingSupplyChunkFree(MArrayRemoveLast(m->supply_chunks));
  /* ***
  while( MArraySize(m->demand_chunks) > 0 )
    KheMatchingDemandChunkFree(MArrayRemoveLast(m->demand_chunks));
  *** */
  MArrayFree(m->supply_nodes);
  MArrayFree(m->unmatched_demand_nodes);
  MArrayFree(m->marks);
  while( MArraySize(m->hall_sets) > 0 )
    KheMatchingHallSetFree(MArrayRemoveLast(m->hall_sets));
  MFree(m);
}


/*****************************************************************************/
/*                                                                           */
/*  KHE_MATCHING KheMatchingCopyPhase1(KHE_MATCHING m)                       */
/*                                                                           */
/*  Carry out Phase 1 of copying m.                                          */
/*                                                                           */
/*****************************************************************************/

KHE_MATCHING KheMatchingCopyPhase1(KHE_MATCHING m)
{
  KHE_MATCHING copy;  KHE_MATCHING_SUPPLY_CHUNK sc;  int i;
  KHE_MATCHING_DEMAND_CHUNK dc;  KHE_MATCHING_SUPPLY_NODE sn;
  KHE_MATCHING_DEMAND_NODE dn;  KHE_MATCHING_HALL_SET hs;
  if( m->copy == NULL )
  {
    MMake(copy);
    m->copy = copy;
    copy->impl = (void *) KheSolnCopyPhase1((KHE_SOLN) m->impl);
    MArrayInit(copy->supply_chunks);
    MArrayForEach(m->supply_chunks, &sc, &i)
      MArrayAddLast(copy->supply_chunks, KheMatchingSupplyChunkCopyPhase1(sc));
    MArrayInit(copy->demand_chunks);
    MArrayForEach(m->demand_chunks, &dc, &i)
      MArrayAddLast(copy->demand_chunks, KheMatchingDemandChunkCopyPhase1(dc));
    MArrayInit(copy->supply_nodes);
    MArrayForEach(m->supply_nodes, &sn, &i)
      MArrayAddLast(copy->supply_nodes, KheMatchingSupplyNodeCopyPhase1(sn));
    copy->demand_node_count = m->demand_node_count;
    copy->visit_num = m->visit_num;
    MArrayInit(copy->unmatched_demand_nodes);
    MArrayForEach(m->unmatched_demand_nodes, &dn, &i)
      MArrayAddLast(copy->unmatched_demand_nodes,
	KheMatchingDemandNodeCopyPhase1(dn));
    copy->unmatched_lower_bound = m->unmatched_lower_bound;
    MAssert(!m->active, "KheMatchingCopyPhase1 internal error");
    copy->active = false;
    MArrayInit(copy->marks);
    MArrayAppend(copy->marks, m->marks, i);
    MArrayInit(copy->hall_sets);
    MArrayForEach(m->hall_sets, &hs, &i)
      MArrayAddLast(copy->hall_sets, KheMatchingHallSetCopyPhase1(hs));
    copy->comp_bfs_first = (m->comp_bfs_first == NULL ? NULL :
      KheMatchingDemandNodeCopyPhase1(m->comp_bfs_first));
    copy->comp_bfs_last = (m->comp_bfs_last == NULL ? NULL :
      KheMatchingDemandNodeCopyPhase1(m->comp_bfs_last));
    copy->copy = NULL;
  }
  return m->copy;
}


/*****************************************************************************/
/*                                                                           */
/*  void KheMatchingCopyPhase2(KHE_MATCHING m)                               */
/*                                                                           */
/*  Carry out Phase 2 of copying m.                                          */
/*                                                                           */
/*****************************************************************************/

void KheMatchingCopyPhase2(KHE_MATCHING m)
{
  KHE_MATCHING_SUPPLY_CHUNK sc;  int i;
  KHE_MATCHING_DEMAND_CHUNK dc;  KHE_MATCHING_SUPPLY_NODE sn;
  KHE_MATCHING_DEMAND_NODE dn;  KHE_MATCHING_HALL_SET hs;
  if( m->copy != NULL )
  {
    m->copy = NULL;
    MArrayForEach(m->supply_chunks, &sc, &i)
      KheMatchingSupplyChunkCopyPhase2(sc);
    MArrayForEach(m->demand_chunks, &dc, &i)
      KheMatchingDemandChunkCopyPhase2(dc);
    MArrayForEach(m->supply_nodes, &sn, &i)
      KheMatchingSupplyNodeCopyPhase2(sn);
    MArrayForEach(m->unmatched_demand_nodes, &dn, &i)
      KheMatchingDemandNodeCopyPhase2(dn);
    MArrayForEach(m->hall_sets, &hs, &i)
      KheMatchingHallSetCopyPhase2(hs);
  }
}


/*****************************************************************************/
/*                                                                           */
/*  int KheMatchingSupplyNodeCount(KHE_MATCHING m)                           */
/*                                                                           */
/*  Return the number of supply nodes in m.                                  */
/*                                                                           */
/*****************************************************************************/

int KheMatchingSupplyNodeCount(KHE_MATCHING m)
{
  return MArraySize(m->supply_nodes);
}


/*****************************************************************************/
/*                                                                           */
/*  int KheMatchingDemandNodeCount(KHE_MATCHING m)                           */
/*                                                                           */
/*  Return the number of demand nodes in m.                                  */
/*                                                                           */
/*****************************************************************************/

int KheMatchingDemandNodeCount(KHE_MATCHING m)
{
  return m->demand_node_count;
}


/*****************************************************************************/
/*                                                                           */
/*  int KheMatchingSupplyChunkCount(KHE_MATCHING m)                          */
/*                                                                           */
/*  Return the number of supply chunks in m.                                 */
/*                                                                           */
/*****************************************************************************/

int KheMatchingSupplyChunkCount(KHE_MATCHING m)
{
  return MArraySize(m->supply_chunks);
}


/*****************************************************************************/
/*                                                                           */
/*  KHE_MATCHING_SUPPLY_CHUNK KheMatchingSupplyChunk(KHE_MATCHING m, int i)  */
/*                                                                           */
/*  Return the i'th supply chunk of m.                                       */
/*                                                                           */
/*****************************************************************************/

KHE_MATCHING_SUPPLY_CHUNK KheMatchingSupplyChunk(KHE_MATCHING m, int i)
{
  return MArrayGet(m->supply_chunks, i);
}


/*****************************************************************************/
/*                                                                           */
/*  int KheMatchingDemandChunkCount(KHE_MATCHING m)                          */
/*                                                                           */
/*  Return the number of demand chunks in m.                                 */
/*                                                                           */
/*****************************************************************************/

int KheMatchingDemandChunkCount(KHE_MATCHING m)
{
  return MArraySize(m->demand_chunks);
}


/*****************************************************************************/
/*                                                                           */
/*  KHE_MATCHING_DEMAND_CHUNK KheMatchingDemandChunk(KHE_MATCHING m, int i)  */
/*                                                                           */
/*  Return the i'th demand chunk of m.                                       */
/*                                                                           */
/*****************************************************************************/

KHE_MATCHING_DEMAND_CHUNK KheMatchingDemandChunk(KHE_MATCHING m, int i)
{
  return MArrayGet(m->demand_chunks, i);
}


/*****************************************************************************/
/*                                                                           */
/*  int KheMatchingUnmatchedDemandNodeCount(KHE_MATCHING m)                  */
/*                                                                           */
/*  Return the number of unmatched demand nodes in a maximum matching of m.  */
/*                                                                           */
/*****************************************************************************/

int KheMatchingUnmatchedDemandNodeCount(KHE_MATCHING m)
{
  MAssert(!m->active, "KheMatchingUnmatchedDemandNodeCount reentry in %p",
    m->impl);
  m->active = true;
  MakeClean(m);
  m->active = false;
  return m->unmatched_lower_bound;
}


/*****************************************************************************/
/*                                                                           */
/*  KHE_MATCHING_DEMAND_NODE KheMatchingUnmatchedDemandNode(KHE_MATCHING m,  */
/*    int i)                                                                 */
/*                                                                           */
/*  Return the i'th unmatched demand node in a maximum matching of m.        */
/*                                                                           */
/*****************************************************************************/

KHE_MATCHING_DEMAND_NODE KheMatchingUnmatchedDemandNode(KHE_MATCHING m, int i)
{
  MAssert(!m->active, "KheMatchingUnmatchedDemandNode reentry in %p",
    m->impl);
  m->active = true;
  MakeClean(m);
  m->active = false;
  return MArrayGet(m->unmatched_demand_nodes, i);
}


/*****************************************************************************/
/*                                                                           */
/*  void KheMatchingMarkBegin(KHE_MATCHING m)                                */
/*                                                                           */
/*  Begin a bracketed sequence of operations on m.                           */
/*                                                                           */
/*****************************************************************************/

void KheMatchingMarkBegin(KHE_MATCHING m)
{
  MArrayAddLast(m->marks, KheMatchingUnmatchedDemandNodeCount(m));
}


/*****************************************************************************/
/*                                                                           */
/*  void KheMatchingMarkEnd(KHE_MATCHING m, bool undo)                       */
/*                                                                           */
/*  End a bracketed sequence of operations on m.  If undo is true, assume    */
/*  that this returns m to the same state it was in when the corresponding   */
/*  call to KheMatchingMarkBegin was called.                                 */
/*                                                                           */
/*****************************************************************************/

void KheMatchingMarkEnd(KHE_MATCHING m, bool undo)
{
  int lower_bound;
  lower_bound = MArrayRemoveLast(m->marks);
  if( undo )
  {
    MAssert(!m->active, "KheMatchingMarkEnd reentry in %p", m->impl);
    m->active = true;
    m->unmatched_lower_bound = lower_bound;
    MakeClean(m);
    MAssert(m->unmatched_lower_bound == lower_bound,
      "KheMatchingMarkEnd: true undo parameter but state is different");
    m->active = false;
  }
}


/*****************************************************************************/
/*                                                                           */
/*  void KheMatchingEnsureHallSetsUpToDate(KHE_MATCHING m)                   */
/*                                                                           */
/*  Ensure that m's Hall sets are up to date.                                */
/*                                                                           */
/*  Implementation note.  This function is based on this global invariant:   */
/*                                                                           */
/*    (1) At all times, m->hall_sets is a well-defined array, but it may     */
/*        be not up to date (it may refer to non-existent nodes, etc.).      */
/*                                                                           */
/*    (2) If m is not up to date, then m->hall_sets is not up to date.       */
/*                                                                           */
/*    (3) If m is up to date, then                                           */
/*        (a) If m->hall_sets is empty, it may be not up to date;            */
/*        (b) If m->hall_sets is non-empty, then it is up to date.           */
/*                                                                           */
/*  First, it calls MakeClean to make sure that m is up to date, so (3)      */
/*  applies.  Then, if m->hall_sets is empty, clearly it is up to date if    */
/*  and only if there are no unmatched demand nodes.  So BuildHallSets is    */
/*  called if m->hall_sets is empty and there are unmatched demand nodes.    */
/*                                                                           */
/*****************************************************************************/
static void KheMatchingBuildHallSets(KHE_MATCHING m);

static void KheMatchingEnsureHallSetsUpToDate(KHE_MATCHING m)
{
  MAssert(!m->active, "KheMatchingMarkEnd reentry in %p", m->impl);
  m->active = true;
  MakeClean(m);
  if( MArraySize(m->hall_sets)==0 && MArraySize(m->unmatched_demand_nodes) > 0 )
    KheMatchingBuildHallSets(m);
  m->active = false;
}


/*****************************************************************************/
/*                                                                           */
/*  int KheMatchingHallSetCount(KHE_MATCHING m)                              */
/*                                                                           */
/*  Return the number of Hall sets in m, making sure they are up to date.    */
/*                                                                           */
/*****************************************************************************/

int KheMatchingHallSetCount(KHE_MATCHING m)
{
  KheMatchingEnsureHallSetsUpToDate(m);
  return MArraySize(m->hall_sets);
}


/*****************************************************************************/
/*                                                                           */
/*  KHE_MATCHING_HALL_SET KheMatchingHallSet(KHE_MATCHING m, int i)          */
/*                                                                           */
/*  Return the i'th Hall set of m, making sure the Hall sets are up to date. */
/*                                                                           */
/*****************************************************************************/

KHE_MATCHING_HALL_SET KheMatchingHallSet(KHE_MATCHING m, int i)
{
  KheMatchingEnsureHallSetsUpToDate(m);
  return MArrayGet(m->hall_sets, i);
}


/*****************************************************************************/
/*                                                                           */
/*  Submodule "competitors"                                                  */
/*                                                                           */
/*****************************************************************************/

/*****************************************************************************/
/*                                                                           */
/*  KHE_MATCHING_DEMAND_NODE KheMatchingFirstCompetitor(KHE_MATCHING m,      */
/*    KHE_MATCHING_DEMAND_NODE dn)                                           */
/*                                                                           */
/*  Return the first competitor of dn in m.                                  */
/*                                                                           */
/*****************************************************************************/

KHE_MATCHING_DEMAND_NODE KheMatchingFirstCompetitor(KHE_MATCHING m,
  KHE_MATCHING_DEMAND_NODE dn)
{
  /* bring up to date and ensure that dn is unmatched */
  KheMatchingUnmatchedDemandNodeCount(m);
  MAssert(dn->demand_asst == NULL,
    "KheMatchingFirstCompetitor: demand node is not unmatched");

  /* initialize the queue to contain just dn, then return the *second* elt  */
  m->visit_num++;
  m->comp_bfs_first = m->comp_bfs_last = dn;
  dn->bfs_next = NULL;
  KheMatchingNextCompetitor(m);
  return KheMatchingNextCompetitor(m);
}


/*****************************************************************************/
/*                                                                           */
/*  KHE_MATCHING_DEMAND_NODE KheMatchingNextCompetitor(KHE_MATCHING m)       */
/*                                                                           */
/*  Return the next competitor in m.                                         */
/*                                                                           */
/*****************************************************************************/

KHE_MATCHING_DEMAND_NODE KheMatchingNextCompetitor(KHE_MATCHING m)
{
  int i, j, base;  KHE_MATCHING_SUPPLY_NODE sn;
  ARRAY_KHE_MATCHING_SUPPLY_NODE supply_nodes;
  KHE_MATCHING_DEMAND_NODE dn;  KHE_MATCHING_DEMAND_CHUNK dc;

  /* if queue is empty, return NULL */
  if( m->comp_bfs_first == NULL )
    return NULL;

  /* add bfs_first's successors to the queue; all must be assigned */
  dn = m->comp_bfs_first;
  dc = dn->demand_chunk;
  supply_nodes = dc->matching->supply_nodes;
  for( i = 0;  i < MArraySize(dc->domain);  i++ )
  {
    base = dc->base + dc->increment * MArrayGet(dc->domain, i);
    for( j = 0;  j < MArraySize(dn->domain);  j++ )
    {
      sn = MArrayGet(supply_nodes, base + MArrayGet(dn->domain, j));
      if( sn->visit_num < m->visit_num )
      {
	sn->visit_num = m->visit_num;
	MAssert(sn->supply_asst != NULL,
	  "KheMatchingNextCompetitor internal error");
	m->comp_bfs_last->bfs_next = sn->supply_asst;
	m->comp_bfs_last = m->comp_bfs_last->bfs_next;
	m->comp_bfs_last->bfs_next = NULL;
      }
    }
  }

  /* remove dn (m->bfs_first) from the queue and return it */
  m->comp_bfs_first = dn->bfs_next;
  return dn;
}


/*****************************************************************************/
/*                                                                           */
/*  Submodule "debug"                                                        */
/*                                                                           */
/*****************************************************************************/

/*****************************************************************************/
/*                                                                           */
/*  void KheMatchingDebugBegin(KHE_MATCHING m,                               */
/*    KHE_MATCHING_SUPPLY_NODE_DEBUG_FN supply_node_debug_fn,                */
/*    KHE_MATCHING_DEMAND_NODE_DEBUG_FN demand_node_debug_fn,                */
/*    int verbosity, int indent, FILE *fp)                                   */
/*                                                                           */
/*  Begin a matching debug.                                                  */
/*                                                                           */
/*****************************************************************************/

void KheMatchingDebugBegin(KHE_MATCHING m,
  KHE_MATCHING_SUPPLY_NODE_DEBUG_FN supply_node_debug_fn,
  KHE_MATCHING_DEMAND_NODE_DEBUG_FN demand_node_debug_fn,
  int verbosity, int indent, FILE *fp)
{
  if( verbosity >= 1 )
  {
    if( indent >= 0 )
      fprintf(fp, "%*s", indent, "");
    fprintf(fp, "[ Matching (%slower bound %d, unmatched %d)",
      verbosity == 2 ? "unmatched demand nodes only, " : "",
      m->unmatched_lower_bound, MArraySize(m->unmatched_demand_nodes));
    if( indent >= 0 )
      fprintf(fp, "\n");
  }
}


/*****************************************************************************/
/*                                                                           */
/*  void KheMatchingDebugDemandChunk(KHE_MATCHING_DEMAND_CHUNK dc,           */
/*    KHE_MATCHING_SUPPLY_NODE_DEBUG_FN supply_node_debug_fn,                */
/*    KHE_MATCHING_DEMAND_NODE_DEBUG_FN demand_node_debug_fn,                */
/*    int verbosity, int indent, FILE *fp)                                   */
/*                                                                           */
/*  Continue a matching debug by debugging one demand chunk.                 */
/*                                                                           */
/*****************************************************************************/

void KheMatchingDebugDemandChunk(KHE_MATCHING_DEMAND_CHUNK dc,
  KHE_MATCHING_SUPPLY_NODE_DEBUG_FN supply_node_debug_fn,
  KHE_MATCHING_DEMAND_NODE_DEBUG_FN demand_node_debug_fn,
  int verbosity, int indent, FILE *fp)
{
  KHE_MATCHING_DEMAND_NODE dn;  int j, k;  short d;
  if( verbosity >= 2 && indent >= 0 )
  {
    if( verbosity >= 4 )
    {
      fprintf(fp, "%*s  chunk: %d + %d*[", indent, "", dc->base, dc->increment);
      MArrayForEach(dc->domain, &d, &j)
      {
	if( j > 0 )
	  fprintf(fp, ", ");
	fprintf(fp, "%d", d);
      }
      fprintf(fp, "]\n");
    }
    MArrayForEach(dc->demand_nodes, &dn, &j)
      if( verbosity >= 3 || dn->demand_asst == NULL )
      {
	fprintf(fp, "%*s  ", indent, "");
	demand_node_debug_fn(dn, verbosity, -1, fp);
	if( dn->demand_asst != NULL )
	{
	  fprintf(fp, " -> ");
	  if( verbosity >= 4 )
	    fprintf(stderr, "[%d] ", dn->demand_asst->index);
	  supply_node_debug_fn(dn->demand_asst, verbosity, -1, fp);
	}
	fprintf(fp, "\n");
	if( verbosity >= 4 )
	{
	  fprintf(fp, "%*s    node: [", indent, "");
	  MArrayForEach(dn->domain, &d, &k)
	  {
	    if( k > 0 )
	      fprintf(fp, ", ");
	    fprintf(fp, "%d", d);
	  }
	  fprintf(fp, "]\n");
	}
      }
  }
}


/*****************************************************************************/
/*                                                                           */
/*  void KheMatchingDebugEnd(KHE_MATCHING m,                                 */
/*    KHE_MATCHING_SUPPLY_NODE_DEBUG_FN supply_node_debug_fn,                */
/*    KHE_MATCHING_DEMAND_NODE_DEBUG_FN demand_node_debug_fn,                */
/*    int verbosity, int indent, FILE *fp)                                   */
/*                                                                           */
/*  End a matching debug.                                                    */
/*                                                                           */
/*****************************************************************************/

void KheMatchingDebugEnd(KHE_MATCHING m,
  KHE_MATCHING_SUPPLY_NODE_DEBUG_FN supply_node_debug_fn,
  KHE_MATCHING_DEMAND_NODE_DEBUG_FN demand_node_debug_fn,
  int verbosity, int indent, FILE *fp)
{
  if( verbosity >= 1 )
  {
    if( indent >= 0 )
      fprintf(fp, "%*s", indent, "");
    else
      fprintf(fp, " ");
    fprintf(fp, "]");
    if( indent >= 0 )
      fprintf(fp, "\n");
  }
}


/*****************************************************************************/
/*                                                                           */
/*  void KheMatchingDebug(KHE_MATCHING m,                                    */
/*    KHE_MATCHING_SUPPLY_NODE_DEBUG_FN supply_node_debug_fn,                */
/*    KHE_MATCHING_DEMAND_NODE_DEBUG_FN demand_node_debug_fn,                */
/*    int verbosity, int indent, FILE *fp)                                   */
/*                                                                           */
/*  Debug print of m onto fp with the given verbosity and indent.            */
/*                                                                           */
/*****************************************************************************/

void KheMatchingDebug(KHE_MATCHING m,
  KHE_MATCHING_SUPPLY_NODE_DEBUG_FN supply_node_debug_fn,
  KHE_MATCHING_DEMAND_NODE_DEBUG_FN demand_node_debug_fn,
  int verbosity, int indent, FILE *fp)
{
  KHE_MATCHING_DEMAND_CHUNK dc;  int i;
  KheMatchingDebugBegin(m, supply_node_debug_fn,
    demand_node_debug_fn, verbosity, indent, fp);
  MArrayForEach(m->demand_chunks, &dc, &i)
    KheMatchingDebugDemandChunk(dc, supply_node_debug_fn,
      demand_node_debug_fn, verbosity, indent, fp);
  KheMatchingDebugEnd(m, supply_node_debug_fn,
    demand_node_debug_fn, verbosity, indent, fp);
}


/*****************************************************************************/
/*                                                                           */
/*  KHE_MATCHING_HALL_SET                                                    */
/*                                                                           */
/*****************************************************************************/

/*****************************************************************************/
/*                                                                           */
/*  KHE_MATCHING_HALL_SET KheMatchingHallSetMake(void)                       */
/*                                                                           */
/*  Return a new, empty Hall set, not yet loaded into m.                     */
/*                                                                           */
/*  When there is no parent set, the easiest arrangement is to let the       */
/*  parent_set field point to self rather than be NULL.                      */
/*                                                                           */
/*****************************************************************************/

static KHE_MATCHING_HALL_SET KheMatchingHallSetMake(void)
{
  KHE_MATCHING_HALL_SET res;
  MMake(res);
  res->parent_hall_set = res;
  MArrayInit(res->supply_nodes);
  MArrayInit(res->demand_nodes);
  res->copy = NULL;
  return res;
}


/*****************************************************************************/
/*                                                                           */
/*  void KheMatchingHallSetFree(KHE_MATCHING_HALL_SET hs)                    */
/*                                                                           */
/*  Free the memory consumed by hs.                                          */
/*                                                                           */
/*****************************************************************************/

static void KheMatchingHallSetFree(KHE_MATCHING_HALL_SET hs)
{
  MArrayFree(hs->supply_nodes);
  MArrayFree(hs->demand_nodes);
  MFree(hs);
}


/*****************************************************************************/
/*                                                                           */
/*  KHE_MATCHING_HALL_SET KheMatchingHallSetRoot(KHE_MATCHING_HALL_SET hs)   */
/*                                                                           */
/*  Return the root hall set of hs.                                          */
/*                                                                           */
/*****************************************************************************/

static KHE_MATCHING_HALL_SET KheMatchingHallSetRoot(KHE_MATCHING_HALL_SET hs)
{
  MAssert(hs != NULL, "HallSetRoot internal error");
  while( hs->parent_hall_set != hs )
    hs = hs->parent_hall_set;
  return hs;
}


/*****************************************************************************/
/*                                                                           */
/*  void KheMatchingHallSetCompressPath(KHE_MATCHING_HALL_SET *hall_set)     */
/*                                                                           */
/*  Compress the path from *hall_set to its root Hall set, including         */
/*  changing *hall_set to its own root.                                      */
/*                                                                           */
/*****************************************************************************/

static void KheMatchingHallSetCompressPath(KHE_MATCHING_HALL_SET *hall_set)
{
  KHE_MATCHING_HALL_SET hs, hs_parent, root;
  root = KheMatchingHallSetRoot(*hall_set);
  hs = *hall_set;
  *hall_set = root;
  while( hs->parent_hall_set != root )
  {
    hs_parent = hs->parent_hall_set;
    hs->parent_hall_set = root;
    hs = hs_parent;
  }
}


/*****************************************************************************/
/*                                                                           */
/*  void KheMatchingHallSetUnion(KHE_MATCHING_HALL_SET root,                 */
/*    KHE_MATCHING_HALL_SET *hs)                                             */
/*                                                                           */
/*  Union *hs into root.                                                     */
/*                                                                           */
/*****************************************************************************/

static void KheMatchingHallSetUnion(KHE_MATCHING_HALL_SET root,
  KHE_MATCHING_HALL_SET *hs)
{
  KHE_MATCHING_HALL_SET other_root;
  other_root = KheMatchingHallSetRoot(*hs);
  other_root->parent_hall_set = root;
  KheMatchingHallSetCompressPath(hs);
}


/*****************************************************************************/
/*                                                                           */
/*  void KheMatchingHallSetAssign(KHE_MATCHING m,                            */
/*    KHE_MATCHING_DEMAND_NODE dn, KHE_MATCHING_HALL_SET root)               */
/*                                                                           */
/*  Explore the matching graph from demand_node, as an assignment would      */
/*  do, and add all the nodes encountered to root, either directly or by     */
/*  changing the roots of their own Hall sets to point to this one.          */
/*                                                                           */
/*  Since this traversal starts at an unmatched demand node at a time when   */
/*  the matching is maximal, it can never encounter an unassigned supply     */
/*  node.  This is checked.                                                  */
/*                                                                           */
/*  Instead of visit_num, this code uses the presence of a non-NULL Hall     */
/*  set in a demand or supply node to determine whether the node has been    */
/*  visited.  If there is a Hall set, the search can terminate, although     */
/*  the Hall set does need to be unioned with root.                          */
/*                                                                           */
/*****************************************************************************/

static void KheMatchingHallSetAssign(KHE_MATCHING m,
  KHE_MATCHING_DEMAND_NODE dn, KHE_MATCHING_HALL_SET root)
{
  KHE_MATCHING_SUPPLY_NODE sn;  int i, j, base;
  KHE_MATCHING_DEMAND_CHUNK dc;  ARRAY_KHE_MATCHING_SUPPLY_NODE supply_nodes;
  if( dn->hall_set != NULL )
  {
    /* dn is already visited, so just make sure the Hall sets are unioned */
    if( dn->hall_set != root )
      KheMatchingHallSetUnion(root, &dn->hall_set);
  }
  else
  {
    /* add dn to root and recurse to all supply nodes */
    dn->hall_set = root;
    supply_nodes = dn->demand_chunk->matching->supply_nodes;
    dc = dn->demand_chunk;
    for( i = 0;  i < MArraySize(dc->domain);  i++ )
    {
      base = dc->base + dc->increment * MArrayGet(dc->domain, i);
      for( j = 0;  j < MArraySize(dn->domain);  j++ )
      {
	sn = MArrayGet(supply_nodes, base + MArrayGet(dn->domain, j));
	MAssert(sn->supply_asst != NULL, "HallSetAssign internal error");
	if( sn->hall_set != NULL )
	{
	  /* sn already visited; make sure Hall sets are unioned */
	  if( sn->hall_set != root )
	    KheMatchingHallSetUnion(root, &sn->hall_set);
	}
	else
	{
	  /* add sn to root and recurse to assigned demand node */
	  sn->hall_set = root;
	  KheMatchingHallSetAssign(m, sn->supply_asst, root);
	}
      }
    }
  }
}


/*****************************************************************************/
/*                                                                           */
/*  void KheMatchingBuildHallSets(KHE_MATCHING m)                            */
/*                                                                           */
/*  Build the Hall sets for m, first bringing m up to date.                  */
/*                                                                           */
/*****************************************************************************/

static void KheMatchingBuildHallSets(KHE_MATCHING m)
{
  KHE_MATCHING_DEMAND_NODE dn;  KHE_MATCHING_SUPPLY_NODE sn;
  KHE_MATCHING_HALL_SET hs, junk;  KHE_MATCHING_DEMAND_CHUNK dc;  int i, j;

  /* bring m up to date and remove any traces of previous Hall sets */
  MakeClean(m);
  MArrayClear(m->hall_sets);
  MArrayForEach(m->supply_nodes, &sn, &i)
    sn->hall_set = NULL;
  MArrayForEach(m->demand_chunks, &dc, &i)
    MArrayForEach(dc->demand_nodes, &dn, &j)
      dn->hall_set = NULL;

  /* assign nodes to Hall sets (including merging overlapping sets) */
  MArrayForEach(m->unmatched_demand_nodes, &dn, &i)
    if( dn->hall_set == NULL )
    {
      MAssert(dn->demand_asst == NULL, "BuildHallSets internal error 1");
      hs = KheMatchingHallSetMake();
      MArrayAddLast(m->hall_sets, hs);
      KheMatchingHallSetAssign(m, dn, hs);
    }

  /* compress the path at each node, and add each node to its root set */
  MArrayForEach(m->supply_nodes, &sn, &i)
    if( sn->hall_set != NULL )
    {
      KheMatchingHallSetCompressPath(&sn->hall_set);
      MArrayAddLast(sn->hall_set->supply_nodes, sn);
    }
  MArrayForEach(m->demand_chunks, &dc, &i)
    MArrayForEach(dc->demand_nodes, &dn, &j)
    if( dn->hall_set != NULL )
    {
      KheMatchingHallSetCompressPath(&dn->hall_set);
      MArrayAddLast(dn->hall_set->demand_nodes, dn);
    }
  
  /* remove and free the defunct Hall sets */
  MArrayForEach(m->hall_sets, &hs, &i)
    if( hs->parent_hall_set == hs )
    {
      MAssert(MArraySize(hs->demand_nodes) > MArraySize(hs->supply_nodes),
	"KheMatchingBuildHallSets internal error 2");
    }
    else
    {
      MAssert(MArraySize(hs->demand_nodes) == 0,
	"KheMatchingBuildHallSets internal error 3");
      MAssert(MArraySize(hs->supply_nodes) == 0,
	"KheMatchingBuildHallSets internal error 4");
      junk = MArrayRemoveAndPlug(m->hall_sets, i);
      KheMatchingHallSetFree(hs);
      i--;
    }
}


/*****************************************************************************/
/*                                                                           */
/*  KHE_MATCHING_HALL_SET KheMatchingHallSetCopyPhase1(                      */
/*    KHE_MATCHING_HALL_SET hs)                                              */
/*                                                                           */
/*  Carry out Phase 1 of copying hs.                                         */
/*                                                                           */
/*****************************************************************************/

KHE_MATCHING_HALL_SET KheMatchingHallSetCopyPhase1(KHE_MATCHING_HALL_SET hs)
{
  KHE_MATCHING_HALL_SET copy;  KHE_MATCHING_SUPPLY_NODE sn;
  KHE_MATCHING_DEMAND_NODE dn;  int i;
  if( hs->copy == NULL )
  {
    MMake(copy);
    hs->copy = copy;
    copy->parent_hall_set = (hs->parent_hall_set == NULL ? NULL :
      KheMatchingHallSetCopyPhase1(hs->parent_hall_set));
    MArrayInit(copy->supply_nodes);
    MArrayForEach(hs->supply_nodes, &sn, &i)
      MArrayAddLast(copy->supply_nodes, KheMatchingSupplyNodeCopyPhase1(sn));
    MArrayInit(copy->demand_nodes);
    MArrayForEach(hs->demand_nodes, &dn, &i)
      MArrayAddLast(copy->demand_nodes, KheMatchingDemandNodeCopyPhase1(dn));
    copy->copy = NULL;
  }
  return hs->copy;
}


/*****************************************************************************/
/*                                                                           */
/*  void KheMatchingHallSetCopyPhase2(KHE_MATCHING_HALL_SET hs)              */
/*                                                                           */
/*  Carry out Phase 2 of copying hs.                                         */
/*                                                                           */
/*****************************************************************************/

void KheMatchingHallSetCopyPhase2(KHE_MATCHING_HALL_SET hs)
{
  KHE_MATCHING_SUPPLY_NODE sn;  KHE_MATCHING_DEMAND_NODE dn;  int i;
  if( hs->copy != NULL )
  {
    hs->copy = NULL;
    if( hs->parent_hall_set != NULL )
      KheMatchingHallSetCopyPhase1(hs->parent_hall_set);
    MArrayForEach(hs->supply_nodes, &sn, &i)
      KheMatchingSupplyNodeCopyPhase2(sn);
    MArrayForEach(hs->demand_nodes, &dn, &i)
      KheMatchingDemandNodeCopyPhase2(dn);
  }
}


/*****************************************************************************/
/*                                                                           */
/*  int KheMatchingHallSetSupplyNodeCount(KHE_MATCHING_HALL_SET hs)          */
/*                                                                           */
/*  Return the number of supply nodes in hs.                                 */
/*                                                                           */
/*****************************************************************************/

int KheMatchingHallSetSupplyNodeCount(KHE_MATCHING_HALL_SET hs)
{
  return MArraySize(hs->supply_nodes);
}


/*****************************************************************************/
/*                                                                           */
/*  KHE_MATCHING_SUPPLY_NODE KheMatchingHallSetSupplyNode(                   */
/*    KHE_MATCHING_HALL_SET hs, int i)                                       */
/*                                                                           */
/*  Return the i'th supply node of hs.                                       */
/*                                                                           */
/*****************************************************************************/

KHE_MATCHING_SUPPLY_NODE KheMatchingHallSetSupplyNode(
  KHE_MATCHING_HALL_SET hs, int i)
{
  return MArrayGet(hs->supply_nodes, i);
}


/*****************************************************************************/
/*                                                                           */
/*  int KheMatchingHallSetDemandNodeCount(KHE_MATCHING_HALL_SET hs)          */
/*                                                                           */
/*  Return the number of demand nodes in hs.                                 */
/*                                                                           */
/*****************************************************************************/

int KheMatchingHallSetDemandNodeCount(KHE_MATCHING_HALL_SET hs)
{
  return MArraySize(hs->demand_nodes);
}


/*****************************************************************************/
/*                                                                           */
/*  KHE_MATCHING_DEMAND_NODE KheMatchingHallSetDemandNode(                   */
/*    KHE_MATCHING_HALL_SET hs, int i)                                       */
/*                                                                           */
/*  Return the i'th demand node of hs.                                       */
/*                                                                           */
/*****************************************************************************/

KHE_MATCHING_DEMAND_NODE KheMatchingHallSetDemandNode(
  KHE_MATCHING_HALL_SET hs, int i)
{
  return MArrayGet(hs->demand_nodes, i);
}


/*****************************************************************************/
/*                                                                           */
/*  void KheMatchingHallSetDebug(KHE_MATCHING_HALL_SET hs,                   */
/*    KHE_MATCHING_SUPPLY_NODE_DEBUG_FN supply_node_debug_fn,                */
/*    KHE_MATCHING_DEMAND_NODE_DEBUG_FN demand_node_debug_fn,                */
/*    int verbosity, int indent, FILE *fp)                                   */
/*                                                                           */
/*  Debug print of hs onto fp with the given verbosity and indent.           */
/*                                                                           */
/*****************************************************************************/

void KheMatchingHallSetDebug(KHE_MATCHING_HALL_SET hs,
  KHE_MATCHING_SUPPLY_NODE_DEBUG_FN supply_node_debug_fn,
  KHE_MATCHING_DEMAND_NODE_DEBUG_FN demand_node_debug_fn,
  int verbosity, int indent, FILE *fp)
{
  KHE_MATCHING_DEMAND_NODE dn;  KHE_MATCHING_SUPPLY_NODE sn;  int i;
  if( verbosity >= 1 && indent >= 0 )
  {
    fprintf(fp, "%*s[ Hall set (demand %d, supply %d)\n", indent, "",
      MArraySize(hs->demand_nodes), MArraySize(hs->supply_nodes));
    MAssert(MArraySize(hs->demand_nodes) > MArraySize(hs->supply_nodes),
      "KheMatchingHallSetDebug internal error");
    MArrayForEach(hs->demand_nodes, &dn, &i)
    {
      fprintf(fp, "%*s  ", indent, "");
      demand_node_debug_fn(dn, verbosity, -1, fp);
      if( i < MArraySize(hs->supply_nodes) )
      {
	fprintf(fp, "  |  ");
	sn = MArrayGet(hs->supply_nodes, i);
	supply_node_debug_fn(sn, verbosity, -1, fp);
      }
      fprintf(fp, "\n");
    }
    fprintf(fp, "%*s]\n", indent, "");
  }
}
