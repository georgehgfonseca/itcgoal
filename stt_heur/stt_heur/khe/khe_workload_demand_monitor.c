
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
/*  FILE:         khe_workload_demand_monitor.c                              */
/*  DESCRIPTION:  A workload demand monitor                                  */
/*                                                                           */
/*****************************************************************************/
#include "khe_interns.h"


/*****************************************************************************/
/*                                                                           */
/*  KHE_WORKLOAD_DEMAND_MONITOR                                              */
/*                                                                           */
/*****************************************************************************/

struct khe_workload_demand_monitor_rec {

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

  /* inherited from KHE_MATCHING_DEMAND_NODE */
  KHE_MATCHING_DEMAND_CHUNK	demand_chunk;
  ARRAY_SHORT			domain;
  KHE_MATCHING_SUPPLY_NODE	demand_asst;
  short				demand_asst_index;
  short				unmatched_pos;
  KHE_MATCHING_DEMAND_NODE	bfs_next;
  KHE_MATCHING_DEMAND_NODE	bfs_parent;
  KHE_MATCHING_HALL_SET		hall_set;

  /* specific to KHE_WORKLOAD_DEMAND_MONITOR */
  KHE_RESOURCE_IN_SOLN		resource_in_soln;
  KHE_TIME_GROUP		time_group;
  KHE_WORKLOAD_DEMAND_MONITOR	copy;
};


/*****************************************************************************/
/*                                                                           */
/*  Submodule "construction and query"                                       */
/*                                                                           */
/*****************************************************************************/

/*****************************************************************************/
/*                                                                           */
/*  KHE_WORKLOAD_DEMAND_MONITOR KheWorkloadDemandMonitorMake(                */
/*    KHE_MATCHING_DEMAND_CHUNK dc, KHE_RESOURCE_IN_SOLN rs,                 */
/*    KHE_TIME_GROUP tg)                                                     */
/*                                                                           */
/*  Make a new workload demand monitor with these attributes.                */
/*  Because it is unattached initially, its domain does not matter.          */
/*                                                                           */
/*****************************************************************************/

KHE_WORKLOAD_DEMAND_MONITOR KheWorkloadDemandMonitorMake(KHE_SOLN soln,
  KHE_MATCHING_DEMAND_CHUNK dc, KHE_RESOURCE_IN_SOLN rs, KHE_TIME_GROUP tg)
{
  KHE_WORKLOAD_DEMAND_MONITOR res;  
  MMake(res);
  KheMonitorInitCommonFields((KHE_MONITOR) res, soln,
    KHE_WORKLOAD_DEMAND_MONITOR_TAG);
  MAssert(dc != NULL, "KheWorkloadDemandMonitorMake internal error");
  res->demand_chunk = dc;
  res->domain = KheResourceGroupResourceIndexes(
    KheResourceSingletonResourceGroup(KheResourceInSolnResource(rs)));
  res->demand_asst = NULL;
  res->demand_asst_index = NO_PREV_ASST;
  res->unmatched_pos = -1;  /* undefined at this point */
  res->bfs_next = NULL;
  res->bfs_parent = NULL;
  res->hall_set = NULL;
  res->resource_in_soln = rs;
  res->time_group = tg;
  res->copy = NULL;
  /* KheGroupMonitorAddMonitor((KHE_GROUP_MONITOR) soln, (KHE_MONITOR) res); */
  KheResourceInSolnAddMonitor(rs, (KHE_MONITOR) res);
  return res;
}


/*****************************************************************************/
/*                                                                           */
/*  KHE_RESOURCE KheWorkloadDemandMonitorResource(                           */
/*    KHE_WORKLOAD_DEMAND_MONITOR m)                                         */
/*                                                                           */
/*  Return the resource attribute of m.                                      */
/*                                                                           */
/*****************************************************************************/

KHE_RESOURCE KheWorkloadDemandMonitorResource(KHE_WORKLOAD_DEMAND_MONITOR m)
{
  return KheResourceInSolnResource(m->resource_in_soln);
}


/*****************************************************************************/
/*                                                                           */
/*  KHE_TIME_GROUP KheWorkloadDemandMonitorTimeGroup(                        */
/*    KHE_WORKLOAD_DEMAND_MONITOR m)                                         */
/*                                                                           */
/*  Return the time group attribute of m.                                    */
/*                                                                           */
/*****************************************************************************/

KHE_TIME_GROUP KheWorkloadDemandMonitorTimeGroup(
  KHE_WORKLOAD_DEMAND_MONITOR m)
{
  return m->time_group;
}


/*****************************************************************************/
/*                                                                           */
/*  KHE_WORKLOAD_DEMAND_MONITOR KheWorkloadDemandMonitorCopyPhase1(          */
/*    KHE_WORKLOAD_DEMAND_MONITOR m)                                         */
/*                                                                           */
/*  Carry out Phase 1 of copying m.                                          */
/*                                                                           */
/*****************************************************************************/

KHE_WORKLOAD_DEMAND_MONITOR KheWorkloadDemandMonitorCopyPhase1(
  KHE_WORKLOAD_DEMAND_MONITOR m)
{
  KHE_WORKLOAD_DEMAND_MONITOR copy;
  if( m->copy == NULL )
  {
    MMake(copy);
    m->copy = copy;
    KheMonitorCopyCommonFields((KHE_MONITOR) copy, (KHE_MONITOR) m);
    copy->demand_chunk = KheMatchingDemandChunkCopyPhase1(m->demand_chunk);
    copy->domain = m->domain;
    copy->demand_asst = (m->demand_asst == NULL ? NULL :
      KheMatchingSupplyNodeCopyPhase1(m->demand_asst));
    copy->demand_asst_index = m->demand_asst_index;
    copy->unmatched_pos = m->unmatched_pos;
    copy->bfs_next = (m->bfs_next == NULL ? NULL :
      KheMatchingDemandNodeCopyPhase1(m->bfs_next));
    copy->bfs_parent = (m->bfs_parent == NULL ? NULL :
      KheMatchingDemandNodeCopyPhase1(m->bfs_parent));
    copy->hall_set = (m->hall_set == NULL ? NULL :
      KheMatchingHallSetCopyPhase1(m->hall_set));
    copy->resource_in_soln = KheResourceInSolnCopyPhase1(m->resource_in_soln);
    copy->time_group = m->time_group;
    copy->copy = NULL;
  }
  return m->copy;
}


/*****************************************************************************/
/*                                                                           */
/*  void KheWorkloadDemandMonitorCopyPhase2(KHE_WORKLOAD_DEMAND_MONITOR m)   */
/*                                                                           */
/*  Carry out Phase 2 of copying m.                                          */
/*                                                                           */
/*****************************************************************************/

void KheWorkloadDemandMonitorCopyPhase2(KHE_WORKLOAD_DEMAND_MONITOR m)
{
  if( m->copy != NULL )
  {
    m->copy = NULL;
    KheMatchingDemandChunkCopyPhase2(m->demand_chunk);
    if( m->demand_asst != NULL )
      KheMatchingSupplyNodeCopyPhase2(m->demand_asst);
    if( m->hall_set != NULL )
      KheMatchingHallSetCopyPhase2(m->hall_set);
    KheResourceInSolnCopyPhase2(m->resource_in_soln);
  }
}


/*****************************************************************************/
/*                                                                           */
/*  void KheWorkloadDemandMonitorDelete(KHE_WORKLOAD_DEMAND_MONITOR m)       */
/*                                                                           */
/*  Delete m.                                                                */
/*                                                                           */
/*****************************************************************************/

void KheWorkloadDemandMonitorDelete(KHE_WORKLOAD_DEMAND_MONITOR m)
{
  if( m->attached )
    KheWorkloadDemandMonitorDetachFromSoln(m);
  if( m->parent_monitor != NULL )
    KheGroupMonitorDeleteChildMonitor(m->parent_monitor, (KHE_MONITOR) m);
  KheResourceInSolnDeleteMonitor(m->resource_in_soln, (KHE_MONITOR) m);
  KheSolnDeleteMonitor(m->soln, (KHE_MONITOR) m);
  MFree(m);
}


/*****************************************************************************/
/*                                                                           */
/*  Submodule "attach and detach"                                            */
/*                                                                           */
/*****************************************************************************/

/*****************************************************************************/
/*                                                                           */
/*  void KheWorkloadDemandMonitorAttachToSoln(KHE_WORKLOAD_DEMAND_MONITOR m) */
/*                                                                           */
/*  Attach m.                                                                */
/*                                                                           */
/*****************************************************************************/

void KheWorkloadDemandMonitorAttachToSoln(KHE_WORKLOAD_DEMAND_MONITOR m)
{
  /* NB we don't attach m to m->resource_in_soln */
  KheMatchingDemandNodeAdd((KHE_MATCHING_DEMAND_NODE) m);
  m->attached = true;
}


/*****************************************************************************/
/*                                                                           */
/* void KheWorkloadDemandMonitorDetachFromSoln(KHE_WORKLOAD_DEMAND_MONITOR m)*/
/*                                                                           */
/*  Detach m.                                                                */
/*                                                                           */
/*****************************************************************************/

void KheWorkloadDemandMonitorDetachFromSoln(KHE_WORKLOAD_DEMAND_MONITOR m)
{
  /* NB we don't detach m from m->resource_in_soln */
  KheMatchingDemandNodeDelete((KHE_MATCHING_DEMAND_NODE) m);
  m->attached = false;
}


/*****************************************************************************/
/*                                                                           */
/*  void KheWorkloadDemandMonitorAttachCheck(KHE_WORKLOAD_DEMAND_MONITOR m)  */
/*                                                                           */
/*  Check the attachment of m.                                               */
/*                                                                           */
/*****************************************************************************/

void KheWorkloadDemandMonitorAttachCheck(KHE_WORKLOAD_DEMAND_MONITOR m)
{
  if( !KheMonitorAttachedToSoln((KHE_MONITOR) m) )
    KheMonitorAttachToSoln((KHE_MONITOR) m);
}


/*****************************************************************************/
/*                                                                           */
/*  Submodule "monitoring calls"                                             */
/*                                                                           */
/*****************************************************************************/

/*****************************************************************************/
/*                                                                           */
/*  void KheWorkloadDemandMonitorSetWeight(KHE_WORKLOAD_DEMAND_MONITOR m,    */
/*    KHE_COST new_weight)                                                   */
/*                                                                           */
/*  Change the weight of m.  It is known to be attached.                     */
/*                                                                           */
/*****************************************************************************/

void KheWorkloadDemandMonitorSetWeight(KHE_WORKLOAD_DEMAND_MONITOR m,
  KHE_COST new_weight)
{
  if( m->demand_asst == NULL )
    KheMonitorChangeCost((KHE_MONITOR) m, new_weight);
}


/*****************************************************************************/
/*                                                                           */
/*  Submodule "deviations"                                                   */
/*                                                                           */
/*****************************************************************************/

/*****************************************************************************/
/*                                                                           */
/*  int KheWorkloadDemandMonitorDeviationCount(KHE_WORKLOAD_DEMAND_MONITOR m)*/
/*                                                                           */
/*  Return the number of deviations of m.                                    */
/*                                                                           */
/*****************************************************************************/

int KheWorkloadDemandMonitorDeviationCount(KHE_WORKLOAD_DEMAND_MONITOR m)
{
  return 1;
}


/*****************************************************************************/
/*                                                                           */
/*  int KheWorkloadDemandMonitorDeviation(KHE_WORKLOAD_DEMAND_MONITOR m,     */
/*    int i)                                                                 */
/*                                                                           */
/*  Return the i'th deviation of m.                                          */
/*                                                                           */
/*****************************************************************************/

int KheWorkloadDemandMonitorDeviation(KHE_WORKLOAD_DEMAND_MONITOR m, int i)
{
  MAssert(i == 0, "KheWorkloadDemandMonitorDeviation: i out of range");
  return m->demand_asst != NULL ? 1 : 0;
}


/*****************************************************************************/
/*                                                                           */
/*  char *KheWorkloadDemandMonitorDeviationDescription(                      */
/*    KHE_WORKLOAD_DEMAND_MONITOR m, int i)                                  */
/*                                                                           */
/*  Return a description of the i'th deviation of m.                         */
/*                                                                           */
/*****************************************************************************/

char *KheWorkloadDemandMonitorDeviationDescription(
  KHE_WORKLOAD_DEMAND_MONITOR m, int i)
{
  MAssert(i == 0,
    "KheWorkloadDemandMonitorDeviationDescription: i out of range");
  return NULL;
}


/*****************************************************************************/
/*                                                                           */
/*  Submodule "debug"                                                        */
/*                                                                           */
/*****************************************************************************/

/*****************************************************************************/
/*                                                                           */
/*  void KheWorkloadDemandMonitorDebug(KHE_WORKLOAD_DEMAND_MONITOR m,        */
/*    int verbosity, int indent, FILE *fp)                                   */
/*                                                                           */
/*  Debug print of m onto fp with the given verbosity and indent.            */
/*                                                                           */
/*****************************************************************************/

void KheWorkloadDemandMonitorDebug(KHE_WORKLOAD_DEMAND_MONITOR m,
  int verbosity, int indent, FILE *fp)
{
  if( verbosity >= 1 )
  {
    KheMonitorDebugBegin((KHE_MONITOR) m, indent, fp);
    fprintf(fp, " %p ", (void *) m);
    KheResourceDebug(KheResourceInSolnResource(m->resource_in_soln), 1, -1, fp);
    fprintf(fp, ":");
    KheTimeGroupDebug(m->time_group, 1, -1, fp);
    KheMonitorDebugEnd((KHE_MONITOR) m, true, indent, fp);
  }
}
