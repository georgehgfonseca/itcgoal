
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
/*  FILE:         khe_ordinary_demand_monitor.c                              */
/*  DESCRIPTION:  An ordinary demand monitor                                 */
/*                                                                           */
/*****************************************************************************/
#include "khe_interns.h"


/*****************************************************************************/
/*                                                                           */
/*  KHE_ORDINARY_DEMAND_MONITOR                                              */
/*                                                                           */
/*****************************************************************************/

struct khe_ordinary_demand_monitor_rec {

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

  /* specific to KHE_ORDINARY_DEMAND_MONITOR */
  KHE_TASK			task;
  int				offset;
  KHE_ORDINARY_DEMAND_MONITOR	copy;
};


/*****************************************************************************/
/*                                                                           */
/*  Submodule "construction and query"                                       */
/*                                                                           */
/*****************************************************************************/

/*****************************************************************************/
/*                                                                           */
/*  KHE_ORDINARY_DEMAND_MONITOR KheOrdinaryDemandMonitorMake(KHE_SOLN soln,  */
/*    KHE_MATCHING_DEMAND_CHUNK dc, KHE_TASK task, int offset)               */
/*                                                                           */
/*  Make an ordinary demand monitor with these attributes.                   */
/*  Because it is unattached initially, its domain does not matter.          */
/*                                                                           */
/*****************************************************************************/

KHE_ORDINARY_DEMAND_MONITOR KheOrdinaryDemandMonitorMake(KHE_SOLN soln,
  KHE_MATCHING_DEMAND_CHUNK dc, KHE_TASK task, int offset)
{
  KHE_ORDINARY_DEMAND_MONITOR res;
  MAssert(KheMatchingImpl(KheMatchingDemandChunkMatching(dc)) == (void *) soln,
    "KheOrdinaryDemandMonitorMake internal error (%p != %p)",
    KheMatchingImpl(KheMatchingDemandChunkMatching(dc)), (void *) soln);
  MMake(res);
  KheMonitorInitCommonFields((KHE_MONITOR) res, soln,
    KHE_ORDINARY_DEMAND_MONITOR_TAG);
  res->demand_chunk = dc;
  res->domain = KheSolnMatchingZeroDomain(soln);
  res->demand_asst = NULL;
  res->demand_asst_index = NO_PREV_ASST;
  res->unmatched_pos = -1;  /* undefined at this point */
  res->bfs_next = NULL;
  res->bfs_parent = NULL;
  res->hall_set = NULL;
  res->task = task;
  res->offset = offset;
  res->copy = NULL;
  /* KheGroupMonitorAddMonitor((KHE_GROUP_MONITOR) soln, (KHE_MONITOR) res); */
  KheTaskAddDemandMonitor(task, res);
  /* KheMonitorCheck((KHE_MONITOR) res); */
  return res;
}


/*****************************************************************************/
/*                                                                           */
/*  KHE_TASK KheOrdinaryDemandMonitorTask(KHE_ORDINARY_DEMAND_MONITOR m)     */
/*                                                                           */
/*  Return the task that m monitors.                                         */
/*                                                                           */
/*****************************************************************************/

KHE_TASK KheOrdinaryDemandMonitorTask(KHE_ORDINARY_DEMAND_MONITOR m)
{
  return m->task;
}


/*****************************************************************************/
/*                                                                           */
/*  int KheOrdinaryDemandMonitorOffset(KHE_ORDINARY_DEMAND_MONITOR m)        */
/*                                                                           */
/*  Return the offset of m in its task.                                      */
/*                                                                           */
/*****************************************************************************/

int KheOrdinaryDemandMonitorOffset(KHE_ORDINARY_DEMAND_MONITOR m)
{
  return m->offset;
}


/*****************************************************************************/
/*                                                                           */
/*  void KheOrdinaryDemandMonitorSetTaskAndOffset(                           */
/*    KHE_ORDINARY_DEMAND_MONITOR m, KHE_TASK task, int offset)              */
/*                                                                           */
/*  Set the task and offset attributes of m.                                 */
/*  This function is called only when splitting and merging solution         */
/*  resources, so the domain and matching do not change.                     */
/*                                                                           */
/*****************************************************************************/

void KheOrdinaryDemandMonitorSetTaskAndOffset(KHE_ORDINARY_DEMAND_MONITOR m,
  KHE_TASK task, int offset)
{
  m->task = task;
  m->offset = offset;
}


/*****************************************************************************/
/*                                                                           */
/*  KHE_ORDINARY_DEMAND_MONITOR KheOrdinaryDemandMonitorCopyPhase1(          */
/*    KHE_ORDINARY_DEMAND_MONITOR m)                                         */
/*                                                                           */
/*  Carry out Phase 1 of copying m.                                          */
/*                                                                           */
/*****************************************************************************/

KHE_ORDINARY_DEMAND_MONITOR KheOrdinaryDemandMonitorCopyPhase1(
  KHE_ORDINARY_DEMAND_MONITOR m)
{
  KHE_ORDINARY_DEMAND_MONITOR copy;
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
    copy->task = KheTaskCopyPhase1(m->task);
    copy->offset = m->offset;
    copy->copy = NULL;
  }
  return m->copy;
}


/*****************************************************************************/
/*                                                                           */
/*  void KheOrdinaryDemandMonitorCopyPhase2(KHE_ORDINARY_DEMAND_MONITOR m)   */
/*                                                                           */
/*  Carry out Phase 2 of copying m.                                          */
/*                                                                           */
/*****************************************************************************/

void KheOrdinaryDemandMonitorCopyPhase2(KHE_ORDINARY_DEMAND_MONITOR m)
{
  if( m->copy != NULL )
  {
    m->copy = NULL;
    KheMatchingDemandChunkCopyPhase2(m->demand_chunk);
    if( m->demand_asst != NULL )
      KheMatchingSupplyNodeCopyPhase2(m->demand_asst);
    if( m->hall_set != NULL )
      KheMatchingHallSetCopyPhase2(m->hall_set);
    KheTaskCopyPhase2(m->task);
  }
}


/*****************************************************************************/
/*                                                                           */
/*  void KheOrdinaryDemandMonitorDelete(KHE_ORDINARY_DEMAND_MONITOR m)       */
/*                                                                           */
/*  Delete m.                                                                */
/*                                                                           */
/*****************************************************************************/

void KheOrdinaryDemandMonitorDelete(KHE_ORDINARY_DEMAND_MONITOR m)
{
  if( m->attached )
    KheOrdinaryDemandMonitorDetachFromSoln(m);
  if( m->parent_monitor != NULL )
    KheGroupMonitorDeleteChildMonitor(m->parent_monitor, (KHE_MONITOR) m);
  KheTaskDeleteDemandMonitor(m->task, m);
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
/*  void KheOrdinaryDemandMonitorAttachToSoln(KHE_ORDINARY_DEMAND_MONITOR m) */
/*                                                                           */
/*  Attach m.                                                                */
/*                                                                           */
/*****************************************************************************/

void KheOrdinaryDemandMonitorAttachToSoln(KHE_ORDINARY_DEMAND_MONITOR m)
{
  /* KheMonitorCheck((KHE_MONITOR) m); */
  KheTaskAttachDemandMonitor(m->task, m);
  /* KheMonitorCheck((KHE_MONITOR) m); */
  m->domain = KheResourceGroupResourceIndexes(KheTaskMatchingDomain(m->task));
  KheMatchingDemandNodeAdd((KHE_MATCHING_DEMAND_NODE) m);
  m->attached = true;
  /* KheMonitorCheck((KHE_MONITOR) m); */
}


/*****************************************************************************/
/*                                                                           */
/* void KheOrdinaryDemandMonitorDetachFromSoln(KHE_ORDINARY_DEMAND_MONITOR m)*/
/*                                                                           */
/*  Detach m.                                                                */
/*                                                                           */
/*****************************************************************************/

void KheOrdinaryDemandMonitorDetachFromSoln(KHE_ORDINARY_DEMAND_MONITOR m)
{
  KheTaskDetachDemandMonitor(m->task, m);
  KheMatchingDemandNodeDelete((KHE_MATCHING_DEMAND_NODE) m);
  m->attached = false;
  /* KheMonitorCheck((KHE_MONITOR) m); */
}


/*****************************************************************************/
/*                                                                           */
/*  void KheOrdinaryDemandMonitorAttachCheck(KHE_ORDINARY_DEMAND_MONITOR m)  */
/*                                                                           */
/*  Check the attachment of m.                                               */
/*                                                                           */
/*****************************************************************************/

void KheOrdinaryDemandMonitorAttachCheck(KHE_ORDINARY_DEMAND_MONITOR m)
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
/*  void KheOrdinaryDemandMonitorSetDomain(KHE_ORDINARY_DEMAND_MONITOR m,    */
/*    KHE_RESOURCE_GROUP rg, KHE_MATCHING_DOMAIN_CHANGE_TYPE change_type)    */
/*                                                                           */
/*  Set the domain of m to the resource group indexes of rg.                 */
/*                                                                           */
/*  This function is called when assigning, unassigning, and changing        */
/*  the domain and matching type; but only on attached monitors.             */
/*                                                                           */
/*****************************************************************************/

void KheOrdinaryDemandMonitorSetDomain(KHE_ORDINARY_DEMAND_MONITOR m,
  KHE_RESOURCE_GROUP rg, KHE_MATCHING_DOMAIN_CHANGE_TYPE change_type)
{
  MAssert(m->attached, "KheOrdinaryDemandMonitorSetDomain internal error");
  KheMatchingDemandNodeSetDomain((KHE_MATCHING_DEMAND_NODE) m,
    KheResourceGroupResourceIndexes(rg), change_type);
  /* KheMonitorCheck((KHE_MONITOR) m); */
}


/*****************************************************************************/
/*                                                                           */
/*  void KheOrdinaryDemandMonitorSetWeight(KHE_ORDINARY_DEMAND_MONITOR m,    */
/*    KHE_COST new_weight)                                                   */
/*                                                                           */
/*  Set the weight of m to new_weight.  It is known to be attached.          */
/*                                                                           */
/*****************************************************************************/

void KheOrdinaryDemandMonitorSetWeight(KHE_ORDINARY_DEMAND_MONITOR m,
  KHE_COST new_weight)
{
  MAssert(m->attached, "KheOrdinaryDemandMonitorSetWeight internal error");
  if( m->demand_asst == NULL )
    KheMonitorChangeCost((KHE_MONITOR) m, new_weight);
  /* KheMonitorCheck((KHE_MONITOR) m); */
}


/*****************************************************************************/
/*                                                                           */
/*  void KheOrdinaryDemandMonitorChangeType(KHE_ORDINARY_DEMAND_MONITOR m,   */
/*    KHE_MATCHING_TYPE old_mt, KHE_MATCHING_TYPE new_mt)                    */
/*                                                                           */
/*  Change the type of m.  It is known to be attached.                       */
/*                                                                           */
/*****************************************************************************/

/* ***
void KheOrdinaryDemandMonitorChangeType(KHE_ORDINARY_DEMAND_MONITOR m,
  KHE_MATCHING_TYPE old_mt, KHE_MATCHING_TYPE new_mt)
{
  KheMatchingDemandNodeSetDomain((KHE_MATCHING_DEMAND_NODE) m,
    KheResourceGroupResourceIndexes(KheTaskDomain(m->task)),
    KHE_MATCHING_DOMAIN_CHANGE_TO_OTHER);
}
*** */


/*****************************************************************************/
/*                                                                           */
/*  Submodule "debug"                                                        */
/*                                                                           */
/*****************************************************************************/

/*****************************************************************************/
/*                                                                           */
/*  int KheOrdinaryDemandMonitorDeviationCount(                              */
/*    KHE_ORDINARY_DEMAND_MONITOR m)                                         */
/*                                                                           */
/*  Return the number of deviations of m.                                    */
/*                                                                           */
/*****************************************************************************/

int KheOrdinaryDemandMonitorDeviationCount(KHE_ORDINARY_DEMAND_MONITOR m)
{
  return 1;
}


/*****************************************************************************/
/*                                                                           */
/*  int KheOrdinaryDemandMonitorDeviation(KHE_ORDINARY_DEMAND_MONITOR m,     */
/*    int i)                                                                 */
/*                                                                           */
/*  Return the i'th deviation of m.                                          */
/*                                                                           */
/*****************************************************************************/

int KheOrdinaryDemandMonitorDeviation(KHE_ORDINARY_DEMAND_MONITOR m, int i)
{
  MAssert(i == 0, "KheOrdinaryDemandMonitorDeviation: i out of range");
  return m->demand_asst != NULL ? 1 : 0;
}


/*****************************************************************************/
/*                                                                           */
/*  char *KheOrdinaryDemandMonitorDeviationDescription(                      */
/*    KHE_ORDINARY_DEMAND_MONITOR m, int i)                                  */
/*                                                                           */
/*  Return a description of the i'th deviation of m.                         */
/*                                                                           */
/*****************************************************************************/

char *KheOrdinaryDemandMonitorDeviationDescription(
  KHE_ORDINARY_DEMAND_MONITOR m, int i)
{
  MAssert(i == 0,
    "KheOrdinaryDemandMonitorDeviationDescription: i out of range");
  return NULL;
}


/*****************************************************************************/
/*                                                                           */
/*  Submodule "debug"                                                        */
/*                                                                           */
/*****************************************************************************/

/*****************************************************************************/
/*                                                                           */
/*  void KheOrdinaryDemandMonitorDebug(KHE_ORDINARY_DEMAND_MONITOR m,        */
/*    int verbosity, int indent, FILE *fp)                                   */
/*                                                                           */
/*  Debug print of m onto fp with the given verbosity and indent.            */
/*                                                                           */
/*****************************************************************************/

void KheOrdinaryDemandMonitorDebug(KHE_ORDINARY_DEMAND_MONITOR m,
  int verbosity, int indent, FILE *fp)
{
  if( verbosity >= 1 )
  {
    KheMonitorDebugBegin((KHE_MONITOR) m, indent, fp);
    fprintf(fp, " ");
    KheTaskDebug(m->task, 1, -1, fp);
    fprintf(fp, "+%d:", m->offset);
    KheResourceGroupDebug(KheTaskDomain(m->task), 1, -1, fp);
    KheMonitorDebugEnd((KHE_MONITOR) m, true, indent, fp);
  }
}
