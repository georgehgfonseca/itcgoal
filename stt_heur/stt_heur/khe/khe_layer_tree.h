
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
/*  FILE:         khe_layer_tree.h                                           */
/*  DESCRIPTION:  Header file for KheLayerTreeMake() internals               */
/*                                                                           */
/*****************************************************************************/
#include <limits.h>
#include "khe.h"
#include "m.h"
#include "khe_partition.h"

#define min(a, b) ((a) < (b) ? (a) : (b))
#define max(a, b) ((a) > (b) ? (a) : (b))


/*****************************************************************************/
/*                                                                           */
/*   Typedefs                                                                */
/*                                                                           */
/*****************************************************************************/

typedef MARRAY(KHE_EVENT) ARRAY_KHE_EVENT;
typedef MARRAY(KHE_MEET) ARRAY_KHE_MEET;
/* typedef MARRAY(KHE_LAYER) ARRAY_KHE_LAYER; */
typedef MARRAY(KHE_SPLIT_EVENTS_CONSTRAINT) ARRAY_KHE_SPLIT_EVENTS_CONSTRAINT;
typedef MARRAY(KHE_DISTRIBUTE_SPLIT_EVENTS_CONSTRAINT)
  ARRAY_KHE_DISTRIBUTE_SPLIT_EVENTS_CONSTRAINT;
typedef MARRAY(KHE_SPREAD_EVENTS_CONSTRAINT) ARRAY_KHE_SPREAD_EVENTS_CONSTRAINT;
typedef MARRAY(KHE_LINK_EVENTS_CONSTRAINT) ARRAY_KHE_LINK_EVENTS_CONSTRAINT;
typedef MARRAY(KHE_TIME_GROUP) ARRAY_KHE_TIME_GROUP;

typedef struct khe_split_forest_rec *KHE_SPLIT_FOREST;

typedef struct khe_split_class_rec *KHE_SPLIT_CLASS;
typedef MARRAY(KHE_SPLIT_CLASS) ARRAY_KHE_SPLIT_CLASS;

typedef struct khe_split_layer_rec *KHE_SPLIT_LAYER;
typedef MARRAY(KHE_SPLIT_LAYER) ARRAY_KHE_SPLIT_LAYER;

/* ***
typedef struct khe_split_assigner_rec *KHE_SPLIT_ASSIGNER;
*** */

typedef enum {
  /* KHE_LAYER_SPLIT_TASK_TAG, */
  KHE_AVOID_CLASHES_SPLIT_TASK_TAG,
  KHE_PACK_SPLIT_TASK_TAG,
  KHE_PREASSIGNED_SPLIT_TASK_TAG,
  KHE_ASSIGNED_SPLIT_TASK_TAG,
  KHE_LINK_SPLIT_TASK_TAG,
  KHE_SPLIT_SPLIT_TASK_TAG,
  KHE_DISTRIBUTE_SPLIT_TASK_TAG,
  KHE_SPREAD_SPLIT_TASK_TAG,
  KHE_DOMAIN_SPLIT_TASK_TAG
} KHE_SPLIT_TASK_TAG;

typedef struct khe_split_task_rec *KHE_SPLIT_TASK;
typedef MARRAY(KHE_SPLIT_TASK) ARRAY_KHE_SPLIT_TASK;

/* typedef struct khe_layer_split_task_rec *KHE_LAYER_SPLIT_TASK; */
typedef struct khe_avoid_clashes_split_task_rec *KHE_AVOID_CLASHES_SPLIT_TASK;
typedef struct khe_pack_split_task_rec *KHE_PACK_SPLIT_TASK;
typedef struct khe_preassigned_split_task_rec *KHE_PREASSIGNED_SPLIT_TASK;
typedef struct khe_assigned_split_task_rec *KHE_ASSIGNED_SPLIT_TASK;
typedef struct khe_link_split_task_rec *KHE_LINK_SPLIT_TASK;
typedef struct khe_split_split_task_rec *KHE_SPLIT_SPLIT_TASK;
typedef struct khe_distribute_split_task_rec *KHE_DISTRIBUTE_SPLIT_TASK;
typedef struct khe_spread_split_task_rec *KHE_SPREAD_SPLIT_TASK;
typedef struct khe_domain_split_task_rec *KHE_DOMAIN_SPLIT_TASK;


/*****************************************************************************/
/*                                                                           */
/*  Split forests                                                            */
/*                                                                           */
/*****************************************************************************/

/* construction and query */
extern KHE_SPLIT_FOREST KheSplitForestMake(KHE_SOLN soln);
extern void KheSplitForestFree(KHE_SPLIT_FOREST sf);
extern KHE_SOLN KheSplitForestSoln(KHE_SPLIT_FOREST sf);

/* trials */
/* ***
extern bool KheSplitForestTryAddToLayer(KHE_SPLIT_FOREST sf,
  KHE_EVENT e, KHE_LAYER layer);
*** */
extern bool KheSplitForestTryAddToResource(KHE_SPLIT_FOREST sf,
  KHE_EVENT e, KHE_RESOURCE r);
extern bool KheSplitForestTryTotalAmount(KHE_SPLIT_FOREST sf,
  KHE_EVENT e, int minimum, int maximum);
extern bool KheSplitForestTryDurnAmount(KHE_SPLIT_FOREST sf,
  KHE_EVENT e, int durn, int minimum, int maximum);
extern bool KheSplitForestTryEventMerge(KHE_SPLIT_FOREST sf,
  KHE_EVENT e1, KHE_EVENT e2);
extern bool KheSplitForestTryEventAssign(KHE_SPLIT_FOREST sf,
  KHE_EVENT e1, KHE_EVENT e2);
extern bool KheSplitForestTryPackableInto(KHE_SPLIT_FOREST sf,
  KHE_EVENT e, KHE_PARTITION p);
extern bool KheSplitForestTryEventDomain(KHE_SPLIT_FOREST sf,
  KHE_EVENT e, KHE_TIME_GROUP tg, int durn);
extern bool KheSplitForestTryPreassignedTime(KHE_SPLIT_FOREST sf,
  KHE_EVENT e, KHE_TIME t);
extern bool KheSplitForestTrySpread(KHE_SPLIT_FOREST sf,
  KHE_EVENT_GROUP eg, int min_amount, int max_amount);

/* finalize */
extern KHE_NODE KheSplitForestFinalize(KHE_SPLIT_FOREST sf);

/* debug */
extern void KheSplitForestDebug(KHE_SPLIT_FOREST sf, int indent, FILE *fp);


/*****************************************************************************/
/*                                                                           */
/*  Split classes                                                            */
/*                                                                           */
/*****************************************************************************/

/* construction and query */
extern KHE_SPLIT_CLASS KheSplitClassMake(KHE_SPLIT_FOREST sf,
  int id_num, KHE_EVENT e);
/* ***
extern KHE_SPLIT_CLASS KheSplitClassCycleLayerMake(KHE_SPLIT_FOREST sf,
  int id_num, KHE_LAYER cycle_layer);
*** */
extern void KheSplitClassFree(KHE_SPLIT_CLASS sc, bool free_child_classes);
extern KHE_SPLIT_FOREST KheSplitClassForest(KHE_SPLIT_CLASS sc);

/* parent class */
extern KHE_SPLIT_CLASS KheSplitClassParentClass(KHE_SPLIT_CLASS sc);
extern bool KheSplitClassAncestor(KHE_SPLIT_CLASS sc, KHE_SPLIT_CLASS anc_sc);

/* child classes */
extern int KheSplitClassChildClassCount(KHE_SPLIT_CLASS sc);
extern KHE_SPLIT_CLASS KheSplitClassChildClass(KHE_SPLIT_CLASS sc, int i);
extern void KheSplitClassAddChildClass(KHE_SPLIT_CLASS sc,
  KHE_SPLIT_CLASS child_sc);
extern void KheSplitClassDeleteChildClass(KHE_SPLIT_CLASS sc,
  KHE_SPLIT_CLASS child_sc);

/* local requirements */
extern int KheSplitClassMinTotalAmount(KHE_SPLIT_CLASS sc);
extern int KheSplitClassMaxTotalAmount(KHE_SPLIT_CLASS sc);
extern KHE_PARTITION KheSplitClassMinPartition(KHE_SPLIT_CLASS sc);

/* events */
extern int KheSplitClassEventCount(KHE_SPLIT_CLASS sc);
extern KHE_EVENT KheSplitClassEvent(KHE_SPLIT_CLASS sc, int i);
extern bool KheSplitClassContainsEvent(KHE_SPLIT_CLASS sc, KHE_EVENT e);
extern int KheSplitClassDuration(KHE_SPLIT_CLASS sc);

/* trials */
extern void KheSplitClassTryBegin(KHE_SPLIT_CLASS sc);
/* ***
extern void KheSplitClassTryAddToLayer(KHE_SPLIT_CLASS sc, KHE_LAYER layer);
*** */
extern void KheSplitClassTryAddToResource(KHE_SPLIT_CLASS sc, KHE_RESOURCE r);
extern void KheSplitClassTryEventDomain(KHE_SPLIT_CLASS sc,
  KHE_TIME_GROUP tg, int durn);
extern void KheSplitClassTryTotalAmount(KHE_SPLIT_CLASS sc,
  int min_total_amount, int max_total_amount);
extern void KheSplitClassTryDurnAmount(KHE_SPLIT_CLASS sc, int durn,
  int min_amount, int max_amount);
extern void KheSplitClassTryUnsplittable(KHE_SPLIT_CLASS sc);
extern void KheSplitClassTryPackableInto(KHE_SPLIT_CLASS sc, KHE_PARTITION p);
extern void KheSplitClassTryMerge(KHE_SPLIT_CLASS dst_sc,
  KHE_SPLIT_CLASS src_sc);
extern void KheSplitClassTryAssign(KHE_SPLIT_CLASS child_sc,
  KHE_SPLIT_CLASS prnt_sc);
extern bool KheSplitClassTryAlive(KHE_SPLIT_CLASS sc);
extern bool KheSplitClassTryEnd(KHE_SPLIT_CLASS sc, bool success);

/* finalize */
/* ***
extern void KheSplitClassFinalize(KHE_SPLIT_CLASS sc, KHE_SPLIT_ASSIGNER sa);
*** */
extern KHE_NODE KheSplitClassFinalize(KHE_SPLIT_CLASS sc, KHE_NODE parent_node);

/* debug */
void KheSplitClassDebugBrief(KHE_SPLIT_CLASS sc, FILE *fp);
void KheSplitClassDebug(KHE_SPLIT_CLASS sc, int indent, FILE *fp);


/*****************************************************************************/
/*                                                                           */
/*  Split layers                                                             */
/*                                                                           */
/*****************************************************************************/

/* construction and query */
/* ***
extern KHE_SPLIT_LAYER KheSplitLayerMake(KHE_LAYER layer, KHE_PARTITION p);
extern KHE_LAYER KheSplitLayerLayer(KHE_SPLIT_LAYER sl);
*** */
extern KHE_SPLIT_LAYER KheSplitLayerMake(/* KHE_LAYER layer */ KHE_RESOURCE r,
  KHE_PARTITION p);
extern KHE_RESOURCE KheSplitLayerResource(KHE_SPLIT_LAYER sl);
extern void KheSplitLayerFree(KHE_SPLIT_LAYER sl);

/* child classes */
extern void KheSplitLayerAddSplitClass(KHE_SPLIT_LAYER sl,
  KHE_SPLIT_CLASS sc);
extern void KheSplitLayerDeleteSplitClass(KHE_SPLIT_LAYER sl,
  KHE_SPLIT_CLASS sc);
extern bool KheSplitLayerContainsSplitClass(KHE_SPLIT_LAYER sl,
  KHE_SPLIT_CLASS sc);
extern int KheSplitLayerSplitClassCount(KHE_SPLIT_LAYER sl);
extern KHE_SPLIT_CLASS KheSplitLayerSplitClass(KHE_SPLIT_LAYER sl, int i);

/* partition */
extern KHE_PARTITION KheSplitLayerMinPartition(KHE_SPLIT_LAYER sl);

/* trials */
extern void KheSplitLayerTryBegin(KHE_SPLIT_LAYER sl);
extern void KheSplitLayerTryMinPartitionChange(KHE_SPLIT_LAYER sl,
  KHE_PARTITION old_p, KHE_PARTITION new_p);
extern void KheSplitLayerTryMerge(KHE_SPLIT_LAYER dest_sl,
  KHE_SPLIT_LAYER src_sl);
extern void KheSplitLayerTryEnd(KHE_SPLIT_LAYER sl, bool success);

/* debug */
extern void KheSplitLayerDebugBrief(KHE_SPLIT_LAYER sl, int indent, FILE *fp);
extern void KheSplitLayerDebug(KHE_SPLIT_LAYER sl, int indent, FILE *fp);


/*****************************************************************************/
/*                                                                           */
/*  Split assigners                                                          */
/*                                                                           */
/*****************************************************************************/

/* ***
extern KHE_SPLIT_ASSIGNER KheSplitAssignerMake(KHE_SOLN soln);
extern void KheSplitAssignerBegin(KHE_SPLIT_ASSIGNER sa);
extern void KheSplitAssignerAddChildSolnEvent(KHE_SPLIT_ASSIGNER sa,
  KHE_MEET child_meet);
extern void KheSplitAssignerAddParentSolnEvent(KHE_SPLIT_ASSIGNER sa,
  KHE_MEET prnt_meet);
extern bool KheSplitAssignerEnd(KHE_SPLIT_ASSIGNER sa);
extern void KheSplitAssignerFree(KHE_SPLIT_ASSIGNER sa);
*** */


/*****************************************************************************/
/*                                                                           */
/*  Split tasks                                                              */
/*                                                                           */
/*****************************************************************************/

extern KHE_SPLIT_TASK_TAG KheSplitTaskTag(KHE_SPLIT_TASK st);
extern int KheSplitTaskPriority(KHE_SPLIT_TASK st);
extern int KheSplitTaskDecreasingPriorityCmp(const void *t1, const void *t2);
extern void KheSplitTaskTry(KHE_SPLIT_TASK st, KHE_SPLIT_FOREST sf);
extern void KheSplitTaskFree(KHE_SPLIT_TASK st);
extern void KheSplitTaskDebug(KHE_SPLIT_TASK st, int indent, FILE *fp);


/*****************************************************************************/
/*                                                                           */
/*  Layer split tasks                                                        */
/*                                                                           */
/*****************************************************************************/

/* ***
extern KHE_LAYER_SPLIT_TASK KheLayerSplitTaskMake(KHE_LAYER layer);
extern void KheLayerSplitTaskFree(KHE_LAYER_SPLIT_TASK st);
extern void KheLayerSplitTaskTry(KHE_LAYER_SPLIT_TASK st, KHE_SPLIT_FOREST sf);
extern void KheLayerSplitTaskDebug(KHE_LAYER_SPLIT_TASK st,
  int indent, FILE *fp);
*** */


/*****************************************************************************/
/*                                                                           */
/*  Avoid clashes split tasks                                                */
/*                                                                           */
/*****************************************************************************/

extern KHE_AVOID_CLASHES_SPLIT_TASK KheAvoidClashesSplitTaskMake(
  KHE_AVOID_CLASHES_CONSTRAINT c);
extern void KheAvoidClashesSplitTaskFree(KHE_AVOID_CLASHES_SPLIT_TASK st);
extern void KheAvoidClashesSplitTaskTry(KHE_AVOID_CLASHES_SPLIT_TASK st,
  KHE_SPLIT_FOREST sf);
extern void KheAvoidClashesSplitTaskDebug(KHE_AVOID_CLASHES_SPLIT_TASK st,
  int indent, FILE *fp);


/*****************************************************************************/
/*                                                                           */
/*  Pack split tasks                                                         */
/*                                                                           */
/*****************************************************************************/

extern KHE_PACK_SPLIT_TASK KhePackSplitTaskMake(KHE_EVENT e, KHE_PARTITION p);
extern void KhePackSplitTaskFree(KHE_PACK_SPLIT_TASK st);
extern void KhePackSplitTaskTry(KHE_PACK_SPLIT_TASK st, KHE_SPLIT_FOREST sf);
extern void KhePackSplitTaskDebug(KHE_PACK_SPLIT_TASK st, int indent, FILE *fp);


/*****************************************************************************/
/*                                                                           */
/*  Preassigned split tasks                                                  */
/*                                                                           */
/*****************************************************************************/

extern KHE_PREASSIGNED_SPLIT_TASK KhePreassignedSplitTaskMake(KHE_EVENT e);
extern void KhePreassignedSplitTaskFree(KHE_PREASSIGNED_SPLIT_TASK st);
extern void KhePreassignedSplitTaskTry(KHE_PREASSIGNED_SPLIT_TASK st,
  KHE_SPLIT_FOREST sf);
extern void KhePreassignedSplitTaskDebug(KHE_PREASSIGNED_SPLIT_TASK st,
  int indent, FILE *fp);


/*****************************************************************************/
/*                                                                           */
/*  Assigned split tasks                                                     */
/*                                                                           */
/*****************************************************************************/

extern KHE_ASSIGNED_SPLIT_TASK KheAssignedSplitTaskMake(KHE_EVENT child_event,
  KHE_EVENT prnt_event);
extern void KheAssignedSplitTaskFree(KHE_ASSIGNED_SPLIT_TASK st);
extern void KheAssignedSplitTaskTry(KHE_ASSIGNED_SPLIT_TASK st,
  KHE_SPLIT_FOREST sf);
extern void KheAssignedSplitTaskDebug(KHE_ASSIGNED_SPLIT_TASK st,
  int indent, FILE *fp);


/*****************************************************************************/
/*                                                                           */
/*  Link split tasks                                                         */
/*                                                                           */
/*****************************************************************************/

extern KHE_LINK_SPLIT_TASK KheLinkSplitTaskMake(KHE_LINK_EVENTS_CONSTRAINT c);
extern void KheLinkSplitTaskFree(KHE_LINK_SPLIT_TASK st);
extern void KheLinkSplitTaskTry(KHE_LINK_SPLIT_TASK st, KHE_SPLIT_FOREST sf);
extern void KheLinkSplitTaskDebug(KHE_LINK_SPLIT_TASK st,
  int indent, FILE *fp);


/*****************************************************************************/
/*                                                                           */
/*  Split split tasks                                                        */
/*                                                                           */
/*****************************************************************************/

extern KHE_SPLIT_SPLIT_TASK KheSplitSplitTaskMake(
  KHE_SPLIT_EVENTS_CONSTRAINT c);
extern void KheSplitSplitTaskFree(KHE_SPLIT_SPLIT_TASK st);
extern void KheSplitSplitTaskTry(KHE_SPLIT_SPLIT_TASK st, KHE_SPLIT_FOREST sf);
extern void KheSplitSplitTaskDebug(KHE_SPLIT_SPLIT_TASK st,
  int indent, FILE *fp);


/*****************************************************************************/
/*                                                                           */
/*  Distribute split tasks                                                   */
/*                                                                           */
/*****************************************************************************/

extern KHE_DISTRIBUTE_SPLIT_TASK KheDistributeSplitTaskMake(
  KHE_DISTRIBUTE_SPLIT_EVENTS_CONSTRAINT c);
extern void KheDistributeSplitTaskFree(KHE_DISTRIBUTE_SPLIT_TASK st);
extern void KheDistributeSplitTaskTry(KHE_DISTRIBUTE_SPLIT_TASK st,
  KHE_SPLIT_FOREST sf);
extern void KheDistributeSplitTaskDebug(KHE_DISTRIBUTE_SPLIT_TASK st,
  int indent, FILE *fp);


/*****************************************************************************/
/*                                                                           */
/*  Spread split tasks                                                       */
/*                                                                           */
/*****************************************************************************/

extern KHE_SPREAD_SPLIT_TASK KheSpreadSplitTaskMake(
  KHE_SPREAD_EVENTS_CONSTRAINT c);
extern void KheSpreadSplitTaskFree(KHE_SPREAD_SPLIT_TASK st);
extern void KheSpreadSplitTaskTry(KHE_SPREAD_SPLIT_TASK st,
  KHE_SPLIT_FOREST sf);
extern void KheSpreadSplitTaskDebug(KHE_SPREAD_SPLIT_TASK st,
  int indent, FILE *fp);


/*****************************************************************************/
/*                                                                           */
/*  Domain split tasks                                                       */
/*                                                                           */
/*****************************************************************************/

extern KHE_DOMAIN_SPLIT_TASK KheDomainSplitTaskMake(
  KHE_PREFER_TIMES_CONSTRAINT c);
extern void KheDomainSplitTaskFree(KHE_DOMAIN_SPLIT_TASK st);
extern void KheDomainSplitTaskTry(KHE_DOMAIN_SPLIT_TASK st,
  KHE_SPLIT_FOREST sf);
extern void KheDomainSplitTaskDebug(KHE_DOMAIN_SPLIT_TASK st,
  int indent, FILE *fp);
