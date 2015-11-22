
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
/*  FILE:         khe_interns.h                                              */
/*  DESCRIPTION:  Internal declarations for KHE; don't include this file.    */
/*                                                                           */
/*****************************************************************************/
#include "khe.h"
#include "m.h"
#include "khe_lset.h"
#include "khe_matching.h"
#include <string.h>
#include <limits.h>
#include <math.h>

/*****************************************************************************/
/*                                                                           */
/*  Typedefs                                                                 */
/*                                                                           */
/*****************************************************************************/

typedef enum {
  KHE_TIME_GROUP_TYPE_FULL,
  KHE_TIME_GROUP_TYPE_EMPTY,
  KHE_TIME_GROUP_TYPE_SINGLETON,
  KHE_TIME_GROUP_TYPE_PACKING,
  KHE_TIME_GROUP_TYPE_NEIGHBOUR,
  KHE_TIME_GROUP_TYPE_USER,
  KHE_TIME_GROUP_TYPE_SOLN
} KHE_TIME_GROUP_TYPE;

typedef enum {
  KHE_RESOURCE_GROUP_TYPE_FULL,
  KHE_RESOURCE_GROUP_TYPE_EMPTY,
  KHE_RESOURCE_GROUP_TYPE_SINGLETON,
  KHE_RESOURCE_GROUP_TYPE_USER,
  KHE_RESOURCE_GROUP_TYPE_PARTITION,
  KHE_RESOURCE_GROUP_TYPE_SOLN
} KHE_RESOURCE_GROUP_TYPE;

typedef enum {
  KHE_EVENT_GROUP_TYPE_FULL,
  KHE_EVENT_GROUP_TYPE_EMPTY,
  KHE_EVENT_GROUP_TYPE_SINGLETON,
  KHE_EVENT_GROUP_TYPE_USER,
  KHE_EVENT_GROUP_TYPE_SOLN
} KHE_EVENT_GROUP_TYPE;

/* deviations monitors */
typedef struct khe_dev_monitor_rec {
  bool			devs_changed;			/* true if changed   */
  ARRAY_INT		devs;				/* the deviations    */
  /* ARRAY_INT		new_devs; */			/* new deviations    */
} KHE_DEV_MONITOR;

/* time group neighbourhoods */
typedef struct khe_time_group_nhood_rec *KHE_TIME_GROUP_NHOOD;

/* time domain cache */
/* typedef struct khe_time_domain_cache_rec *KHE_TIME_DOMAIN_CACHE; */

/* event in soln objects */
typedef struct khe_event_in_soln_rec *KHE_EVENT_IN_SOLN;
typedef MARRAY(KHE_EVENT_IN_SOLN) ARRAY_KHE_EVENT_IN_SOLN;

/* event resource in soln objects */
typedef struct khe_event_resource_in_soln_rec *KHE_EVENT_RESOURCE_IN_SOLN;
typedef MARRAY(KHE_EVENT_RESOURCE_IN_SOLN) ARRAY_KHE_EVENT_RESOURCE_IN_SOLN;

/* resource in soln objects */
typedef struct khe_resource_in_soln_rec *KHE_RESOURCE_IN_SOLN;
typedef MARRAY(KHE_RESOURCE_IN_SOLN) ARRAY_KHE_RESOURCE_IN_SOLN;

/* evenness handler */
typedef struct khe_evenness_handler_rec *KHE_EVENNESS_HANDLER;

/* array and symbol table types */
typedef MARRAY(KHE_SOLN_GROUP) ARRAY_KHE_SOLN_GROUP;
typedef MTABLE(KHE_SOLN_GROUP) TABLE_KHE_SOLN_GROUP;
typedef MARRAY(KHE_INSTANCE) ARRAY_KHE_INSTANCE;
typedef MTABLE(KHE_INSTANCE) TABLE_KHE_INSTANCE;
typedef MARRAY(KHE_TIME_GROUP) ARRAY_KHE_TIME_GROUP;
typedef MTABLE(KHE_TIME_GROUP) TABLE_KHE_TIME_GROUP;
typedef MARRAY(KHE_TIME) ARRAY_KHE_TIME;
typedef MTABLE(KHE_TIME) TABLE_KHE_TIME;
typedef MARRAY(KHE_RESOURCE_TYPE) ARRAY_KHE_RESOURCE_TYPE;
typedef MTABLE(KHE_RESOURCE_TYPE) TABLE_KHE_RESOURCE_TYPE;
typedef MARRAY(KHE_RESOURCE_GROUP) ARRAY_KHE_RESOURCE_GROUP;
typedef MTABLE(KHE_RESOURCE_GROUP) TABLE_KHE_RESOURCE_GROUP;
typedef MARRAY(KHE_RESOURCE) ARRAY_KHE_RESOURCE;
typedef MTABLE(KHE_RESOURCE) TABLE_KHE_RESOURCE;
typedef MARRAY(KHE_EVENT_GROUP) ARRAY_KHE_EVENT_GROUP;
typedef MTABLE(KHE_EVENT_GROUP) TABLE_KHE_EVENT_GROUP;
typedef MARRAY(KHE_EVENT) ARRAY_KHE_EVENT;
typedef MTABLE(KHE_EVENT) TABLE_KHE_EVENT;
typedef MARRAY(KHE_EVENT_RESOURCE) ARRAY_KHE_EVENT_RESOURCE;
typedef MARRAY(KHE_EVENT_RESOURCE_GROUP) ARRAY_KHE_EVENT_RESOURCE_GROUP;
typedef MARRAY(KHE_CONSTRAINT) ARRAY_KHE_CONSTRAINT;
typedef MTABLE(KHE_CONSTRAINT) TABLE_KHE_CONSTRAINT;

typedef MARRAY(KHE_PREFER_RESOURCES_CONSTRAINT)
  ARRAY_KHE_PREFER_RESOURCES_CONSTRAINT;

typedef MARRAY(KHE_TIME_SPREAD) ARRAY_KHE_TIME_SPREAD;
typedef MARRAY(KHE_LIMITED_TIME_GROUP) ARRAY_KHE_LIMITED_TIME_GROUP;

typedef MARRAY(KHE_SOLN) ARRAY_KHE_SOLN;
/* typedef MARRAY(KHE_TIME_DOMAIN) ARRAY_KHE_TIME_DOMAIN; */
typedef MARRAY(KHE_MEET) ARRAY_KHE_MEET;
typedef MARRAY(KHE_TASK) ARRAY_KHE_TASK;
typedef MARRAY(KHE_TRANSACTION) ARRAY_KHE_TRANSACTION;
typedef MARRAY(KHE_TRACE) ARRAY_KHE_TRACE;
typedef MARRAY(KHE_NODE) ARRAY_KHE_NODE;
typedef MARRAY(KHE_LAYER) ARRAY_KHE_LAYER;
typedef MARRAY(KHE_ZONE) ARRAY_KHE_ZONE;
typedef MARRAY(KHE_TASKING) ARRAY_KHE_TASKING;

/* monitor array types */
typedef MARRAY(KHE_MONITOR) ARRAY_KHE_MONITOR;
typedef MARRAY(KHE_ASSIGN_RESOURCE_MONITOR) ARRAY_KHE_ASSIGN_RESOURCE_MONITOR;
typedef MARRAY(KHE_ASSIGN_TIME_MONITOR) ARRAY_KHE_ASSIGN_TIME_MONITOR;
typedef MARRAY(KHE_SPLIT_EVENTS_MONITOR) ARRAY_KHE_SPLIT_EVENTS_MONITOR;
typedef MARRAY(KHE_DISTRIBUTE_SPLIT_EVENTS_MONITOR)
  ARRAY_KHE_DISTRIBUTE_SPLIT_EVENTS_MONITOR;
typedef MARRAY(KHE_PREFER_RESOURCES_MONITOR) ARRAY_KHE_PREFER_RESOURCES_MONITOR;
typedef MARRAY(KHE_PREFER_TIMES_MONITOR) ARRAY_KHE_PREFER_TIMES_MONITOR;
typedef MARRAY(KHE_AVOID_SPLIT_ASSIGNMENTS_MONITOR)
  ARRAY_KHE_AVOID_SPLIT_ASSIGNMENTS_MONITOR;
typedef MARRAY(KHE_SPREAD_EVENTS_MONITOR) ARRAY_KHE_SPREAD_EVENTS_MONITOR;
typedef MARRAY(KHE_LINK_EVENTS_MONITOR) ARRAY_KHE_LINK_EVENTS_MONITOR;
typedef MARRAY(KHE_AVOID_CLASHES_MONITOR) ARRAY_KHE_AVOID_CLASHES_MONITOR;
typedef MARRAY(KHE_AVOID_UNAVAILABLE_TIMES_MONITOR)
  ARRAY_KHE_AVOID_UNAVAILABLE_TIMES_MONITOR;
typedef MARRAY(KHE_LIMIT_IDLE_TIMES_MONITOR) ARRAY_KHE_LIMIT_IDLE_TIMES_MONITOR;
typedef MARRAY(KHE_CLUSTER_BUSY_TIMES_MONITOR)
  ARRAY_KHE_CLUSTER_BUSY_TIMES_MONITOR;
typedef MARRAY(KHE_LIMIT_BUSY_TIMES_MONITOR) ARRAY_KHE_LIMIT_BUSY_TIMES_MONITOR;
typedef MARRAY(KHE_LIMIT_WORKLOAD_MONITOR) ARRAY_KHE_LIMIT_WORKLOAD_MONITOR;
typedef MARRAY(KHE_TIME_GROUP_MONITOR) ARRAY_KHE_TIME_GROUP_MONITOR;
typedef MARRAY(KHE_GROUP_MONITOR) ARRAY_KHE_GROUP_MONITOR;
typedef MARRAY(KHE_ORDINARY_DEMAND_MONITOR) ARRAY_KHE_ORDINARY_DEMAND_MONITOR;
typedef MARRAY(KHE_WORKLOAD_DEMAND_MONITOR) ARRAY_KHE_WORKLOAD_DEMAND_MONITOR;


/*****************************************************************************/
/*                                                                           */
/*  khe_archive.c                                                            */
/*                                                                           */
/*****************************************************************************/

/* construction and query */
extern void KheArchiveSetMetaData(KHE_ARCHIVE archive, KHE_ARCHIVE_METADATA md);

/* instances */
extern void KheArchiveAddInstance(KHE_ARCHIVE archive, KHE_INSTANCE ins);

/* solution groups */
extern void KheArchiveAddSolnGroup(KHE_ARCHIVE archive,
  KHE_SOLN_GROUP soln_group);


/*****************************************************************************/
/*                                                                           */
/*  khe_archive_metadata.c                                                   */
/*                                                                           */
/*****************************************************************************/

/* reading and writing */
extern bool KheArchiveMetaDataMakeFromKml(KML_ELT md_elt,
  KHE_ARCHIVE archive, KML_ERROR *ke);
extern bool KheArchiveMetaDataWrite(KHE_ARCHIVE_METADATA md, KML_FILE kf);


/*****************************************************************************/
/*                                                                           */
/*  khe_soln_group.c                                                         */
/*                                                                           */
/*****************************************************************************/

/* construction and query */
extern void KheSolnGroupSetMetaData(KHE_SOLN_GROUP soln_group,
  KHE_SOLN_GROUP_METADATA md);

/* reading and writing */
extern bool KheSolnGroupMakeFromKml(KML_ELT soln_group_elt,
  KHE_ARCHIVE archive, KML_ERROR *ke);
extern bool KheSolnGroupWrite(KHE_SOLN_GROUP soln_group, bool with_reports,
  KML_FILE kf);


/*****************************************************************************/
/*                                                                           */
/*  khe_soln_group_metadata.c                                                */
/*                                                                           */
/*****************************************************************************/

/* reading and writing */
extern bool KheSolnGroupMetaDataMakeFromKml(KML_ELT md_elt,
  KHE_SOLN_GROUP soln_group, KML_ERROR *ke);
extern bool KheSolnGroupMetaDataWrite(KHE_SOLN_GROUP_METADATA md, KML_FILE kf);


/*****************************************************************************/
/*                                                                           */
/*  khe_instance.c                                                           */
/*                                                                           */
/*****************************************************************************/

/* construction and query */
extern bool KheInstanceComplete(KHE_INSTANCE ins);
extern void KheInstanceSetMetaData(KHE_INSTANCE ins, KHE_INSTANCE_METADATA md);

/* time groups */
extern void KheInstanceAddTimeGroup(KHE_INSTANCE ins, KHE_TIME_GROUP tg);
extern KHE_TIME_GROUP KheInstanceFullTimeGroupInternal(KHE_INSTANCE ins);
extern KHE_TIME_GROUP_NHOOD KheInstanceSingletonTimeGroupNeighbourhood(
  KHE_INSTANCE ins);

/* times */
extern void KheInstanceAddTime(KHE_INSTANCE ins, KHE_TIME t);
/* extern ARRAY_KHE_TIME KheInstanceTimesArray(KHE_INSTANCE ins); */

/* partitions */
extern void KheInstanceAddPartition(KHE_INSTANCE ins, KHE_RESOURCE_GROUP rg,
  int *index);
extern int KheInstancePartitionCount(KHE_INSTANCE ins);
extern KHE_RESOURCE_GROUP KheInstancePartition(KHE_INSTANCE ins, int i);

/* resource types and resources */
extern void KheInstanceAddResourceType(KHE_INSTANCE ins, KHE_RESOURCE_TYPE rt,
  int *index);
extern void KheInstanceAddResource(KHE_INSTANCE ins, KHE_RESOURCE r);

/* event groups */
extern void KheInstanceAddEventGroup(KHE_INSTANCE ins, KHE_EVENT_GROUP eg);

/* events */
extern void KheInstanceAddEvent(KHE_INSTANCE ins, KHE_EVENT e);
extern ARRAY_KHE_EVENT KheInstanceEventsArray(KHE_INSTANCE ins);
extern int KheInstanceMaxEventDuration(KHE_INSTANCE ins);

/* event resources */
extern void KheInstanceAddEventResource(KHE_INSTANCE ins,
  KHE_EVENT_RESOURCE er);

/* constraints */
extern void KheInstanceAddConstraint(KHE_INSTANCE ins, KHE_CONSTRAINT c);

/* reading and writing */
extern bool KheInstanceMakeFromKml(KML_ELT instance_elt, KHE_ARCHIVE archive,
  bool infer_resource_partitions, KML_ERROR *ke);
extern bool KheInstanceWrite(KHE_INSTANCE ins, KML_FILE kf);


/*****************************************************************************/
/*                                                                           */
/*  khe_instance_metadata.c                                                  */
/*                                                                           */
/*****************************************************************************/

/* reading and writing */
extern bool KheInstanceMetaDataMakeFromKml(KML_ELT md_elt, KHE_INSTANCE ins,
  KML_ERROR *ke);
extern bool KheInstanceMetaDataWrite(KHE_INSTANCE_METADATA md, KML_FILE kf);


/*****************************************************************************/
/*                                                                           */
/*  khe_time_group_nhood.c                                                   */
/*                                                                           */
/*****************************************************************************/

extern KHE_TIME_GROUP_NHOOD KheTimeGroupNhoodMake(KHE_TIME_GROUP tg,
  int *index_in_nhood);
extern KHE_TIME_GROUP_NHOOD KheTimeGroupNHoodMakeEmpty(int count);
extern void KheTimeGroupNHoodSetTimeGroup(KHE_TIME_GROUP_NHOOD tgn,
  int pos, KHE_TIME_GROUP tg);
extern void KheTimeGroupNHoodDelete(KHE_TIME_GROUP_NHOOD tgn, int pos);
extern KHE_TIME_GROUP KheTimeGroupNHoodNeighbour(KHE_TIME_GROUP_NHOOD tgn,
  int pos);


/*****************************************************************************/
/*                                                                           */
/*  khe_time_group.c                                                         */
/*                                                                           */
/*****************************************************************************/

/* construction and query */
extern KHE_TIME_GROUP_TYPE KheTimeGroupType(KHE_TIME_GROUP tg);
extern KHE_TIME_GROUP KheTimeGroupMakeInternal(
  KHE_TIME_GROUP_TYPE time_group_type, KHE_INSTANCE ins,
  KHE_TIME_GROUP_KIND kind, char *id, char *name, LSET times_set);
extern void KheTimeGroupAddTimeInternal(KHE_TIME_GROUP tg, KHE_TIME t);
extern void KheTimeGroupSubTimeInternal(KHE_TIME_GROUP tg, KHE_TIME t);
extern void KheTimeGroupUnionInternal(KHE_TIME_GROUP tg, KHE_TIME_GROUP tg2);
extern void KheTimeGroupIntersectInternal(KHE_TIME_GROUP tg,
  KHE_TIME_GROUP tg2);
extern void KheTimeGroupDifferenceInternal(KHE_TIME_GROUP tg,
  KHE_TIME_GROUP tg2);
extern void KheTimeGroupFinalize(KHE_TIME_GROUP tg,
  KHE_TIME_GROUP_NHOOD tgn, int pos_in_tgn);
extern void KheTimeGroupDelete(KHE_TIME_GROUP tg);

/* times queries */
extern bool KheTimeGroupContainsIndex(KHE_TIME_GROUP tg, int time_index);
/* ***
extern bool KheTimeGroupIntersectionEqual(KHE_TIME_GROUP tg1a,
  KHE_TIME_GROUP tg1b, KHE_TIME_GROUP tg2);
*** */
extern bool KheTimeGroupDomainsAllowAssignment(KHE_TIME_GROUP domain,
  KHE_TIME_GROUP target_domain, int target_offset);
extern int KheTimeGroupTimePos(KHE_TIME_GROUP tg, int time_index);
extern ARRAY_SHORT KheTimeGroupTimeIndexes(KHE_TIME_GROUP tg);
extern LSET KheTimeGroupTimeSet(KHE_TIME_GROUP tg);

/* reading and writing */
extern bool KheTimeGroupMakeFromKml(KML_ELT time_group_elt, KHE_INSTANCE ins,
  KML_ERROR *ke);
extern bool KheTimeGroupWrite(KHE_TIME_GROUP tg, KML_FILE kf);


/*****************************************************************************/
/*                                                                           */
/*  khe_time.c                                                               */
/*                                                                           */
/*****************************************************************************/

/* cycle layer index and offset */
/* ***
extern void KheTimeSetCycleMeetIndexAndOffset(KHE_TIME t, int index,
  int offset);
*** */

/* inferred breaks */
/* ***
extern void KheTimeInferNoBreak(KHE_TIME t);
extern bool KheTimeInferredBreak(KHE_TIME t);
extern void KheTimeSetBreakIfInferred(KHE_TIME t);
*** */

/* reading and writing */
extern bool KheTimeMakeFromKml(KML_ELT time_elt, KHE_INSTANCE ins,
  KML_ERROR *ke);
extern bool KheTimeWrite(KHE_TIME t, KML_FILE kf);


/*****************************************************************************/
/*                                                                           */
/*  khe_time_infer.c                                                         */
/*                                                                           */
/*****************************************************************************/

/* extern void KheTimeInferBreaks(KHE_INSTANCE ins); */


/*****************************************************************************/
/*                                                                           */
/*  khe_resource_type.c                                                      */
/*                                                                           */
/*****************************************************************************/

extern void KheResourceTypeAddResource(KHE_RESOURCE_TYPE rt, KHE_RESOURCE r);
extern void KheResourceTypeAddResourceGroup(KHE_RESOURCE_TYPE rt,
  KHE_RESOURCE_GROUP rg);
extern void KheResourceTypeFinalize(KHE_RESOURCE_TYPE rt);

/* partitions */
extern void KheResourceTypeAddPartition(KHE_RESOURCE_TYPE rt,
  KHE_RESOURCE_GROUP rg);
extern void KheResourceTypeInferPartitions(KHE_RESOURCE_TYPE rt);

/* reading and writing */
extern bool KheResourceTypeMakeFromKml(KML_ELT resource_type_elt,
  KHE_INSTANCE ins, KML_ERROR *ke);
extern bool KheResourceTypeWrite(KHE_RESOURCE_TYPE rt, KML_FILE kf);
extern bool KheResourceTypeWriteResourceGroups(KHE_RESOURCE_TYPE rt,
  KML_FILE kf);
extern bool KheResourceTypeWriteResources(KHE_RESOURCE_TYPE rt, KML_FILE kf);

/* demand and avoid split assignments count */
extern void KheResourceTypeDemandNotAllPreassigned(KHE_RESOURCE_TYPE rt);
extern void KheResourceTypeIncrementAvoidSplitAssignmentsCount(
  KHE_RESOURCE_TYPE rt);


/*****************************************************************************/
/*                                                                           */
/*  khe_resource_group.c                                                     */
/*                                                                           */
/*****************************************************************************/

extern KHE_RESOURCE_GROUP_TYPE KheResourceGroupType(KHE_RESOURCE_GROUP rg);
extern KHE_RESOURCE_GROUP KheResourceGroupMakeInternal(
  KHE_RESOURCE_GROUP_TYPE resource_group_type, /* bool is_partition, */
  KHE_RESOURCE_TYPE rt, char *id, char *name);
extern void KheResourceGroupAddResourceInternal(KHE_RESOURCE_GROUP rg,
  KHE_RESOURCE r);
extern void KheResourceGroupSubResourceInternal(KHE_RESOURCE_GROUP rg,
  KHE_RESOURCE r);
extern void KheResourceGroupUnionInternal(KHE_RESOURCE_GROUP rg,
  KHE_RESOURCE_GROUP rg2);
extern void KheResourceGroupIntersectInternal(KHE_RESOURCE_GROUP rg,
  KHE_RESOURCE_GROUP rg2);
extern void KheResourceGroupDifferenceInternal(KHE_RESOURCE_GROUP rg,
  KHE_RESOURCE_GROUP rg2);
extern void KheResourceGroupSetResourcesArrayInternal(KHE_RESOURCE_GROUP rg);
extern void KheResourceGroupDelete(KHE_RESOURCE_GROUP rg);

/* domains and partitions */
extern ARRAY_SHORT KheResourceGroupResourceIndexes(KHE_RESOURCE_GROUP rg);
extern bool KheResourceGroupPartitionAdmissible(KHE_RESOURCE_GROUP rg);
extern void KheResourceGroupDeclarePartition(KHE_RESOURCE_GROUP rg);
/* ***
extern void KheResourceGroupSetIsPartition(KHE_RESOURCE_GROUP rg,
  bool is_partition);
*** */
extern void KheResourceGroupSetPartition(KHE_RESOURCE_GROUP rg);
extern int KheResourceGroupPartitionIndex(KHE_RESOURCE_GROUP rg);

/* reading and writing */
extern bool KheResourceGroupMakeFromKml(KML_ELT resource_group_elt,
  KHE_INSTANCE ins, KML_ERROR *ke);
extern bool KheResourceGroupWrite(KHE_RESOURCE_GROUP rg, KML_FILE kf);


/*****************************************************************************/
/*                                                                           */
/*  khe_resource.c                                                           */
/*                                                                           */
/*****************************************************************************/

extern void KheResourceAddConstraint(KHE_RESOURCE r, KHE_CONSTRAINT c);
extern void KheResourceAddPreassignedEventResource(KHE_RESOURCE r,
  KHE_EVENT_RESOURCE er);
extern void KheResourceAddUserResourceGroup(KHE_RESOURCE r,
  KHE_RESOURCE_GROUP rg);

/* resource partition */
extern void KheResourceSetPartition(KHE_RESOURCE r, KHE_RESOURCE_GROUP rg);

/* finalizing */
extern void KheResourceFinalize(KHE_RESOURCE r);

/* reading and writing */
extern bool KheResourceMakeFromKml(KML_ELT resource_elt,
  KHE_INSTANCE ins, KML_ERROR *ke);
extern bool KheResourceWrite(KHE_RESOURCE r, KML_FILE kf);


/*****************************************************************************/
/*                                                                           */
/*  khe_event_group.c                                                        */
/*                                                                           */
/*****************************************************************************/

/* construction and query */
extern KHE_EVENT_GROUP_TYPE KheEventGroupType(KHE_EVENT_GROUP eg);
extern KHE_EVENT_GROUP KheEventGroupMakeInternal(
  KHE_EVENT_GROUP_TYPE event_group_type, KHE_INSTANCE ins,
  KHE_EVENT_GROUP_KIND kind, char *id, char *name);
extern void KheEventGroupAddEventInternal(KHE_EVENT_GROUP eg, KHE_EVENT e);
extern void KheEventGroupSubEventInternal(KHE_EVENT_GROUP eg, KHE_EVENT t);
extern void KheEventGroupUnionInternal(KHE_EVENT_GROUP eg, KHE_EVENT_GROUP eg2);
extern void KheEventGroupIntersectInternal(KHE_EVENT_GROUP eg,
  KHE_EVENT_GROUP eg2);
extern void KheEventGroupDifferenceInternal(KHE_EVENT_GROUP eg,
  KHE_EVENT_GROUP eg2);
extern void KheEventGroupSetEventsArrayInternal(KHE_EVENT_GROUP eg);
extern void KheEventGroupDelete(KHE_EVENT_GROUP eg);

/* constraints */
extern void KheEventGroupAddConstraint(KHE_EVENT_GROUP eg, KHE_CONSTRAINT c);

/* reading and writing */
extern bool KheEventGroupMakeFromKml(KML_ELT event_group_elt,
  KHE_INSTANCE ins, KML_ERROR *ke);
extern bool KheEventGroupWrite(KHE_EVENT_GROUP eg, KML_FILE kf);


/*****************************************************************************/
/*                                                                           */
/*  khe_event.c                                                              */
/*                                                                           */
/*****************************************************************************/

/* construction and query */
extern void KheEventAddEventResource(KHE_EVENT e, KHE_EVENT_RESOURCE er,
  int *index);
extern void KheEventAddUserEventGroup(KHE_EVENT e, KHE_EVENT_GROUP eg);
extern void KheEventAddEventResourceGroup(KHE_EVENT event,
  KHE_EVENT_RESOURCE_GROUP erg);

/* constraints */
extern void KheEventAddConstraint(KHE_EVENT e, KHE_CONSTRAINT c);

/* time domains */
extern void KheEventFinalize(KHE_EVENT e);

/* infer time breaks */
/* extern void KheEventInferTimeBreaks(KHE_EVENT e); */

/* infer resource partitions */
extern void KheEventPartitionSetAdmissible(KHE_EVENT e);
extern bool KheEventPartitionAdmissible(KHE_EVENT e);
extern bool KheEventPartitionSimilar(KHE_EVENT e1, KHE_EVENT e2,
  ARRAY_KHE_RESOURCE_GROUP *domains1, ARRAY_KHE_RESOURCE_GROUP *domains2);

/* reading and writing */
extern bool KheEventMakeFromKml(KML_ELT event_elt, KHE_INSTANCE ins,
  KML_ERROR *ke);
extern bool KheEventWrite(KHE_EVENT e, KML_FILE kf);


/*****************************************************************************/
/*                                                                           */
/*  khe_event_resource.c                                                     */
/*                                                                           */
/*****************************************************************************/

/* event resource groups */
extern void KheEventResourceSetEventResourceGroup(KHE_EVENT_RESOURCE er,
  KHE_EVENT_RESOURCE_GROUP erg);

/* constraints */
extern void KheEventResourceAddConstraint(KHE_EVENT_RESOURCE er,
  KHE_CONSTRAINT c, int eg_index);

/* finalizing */
extern void KheEventResourceFinalize(KHE_EVENT_RESOURCE er);

/* reading and writing */
extern bool KheEventResourceMakeFromKml(KML_ELT resource_elt, KHE_EVENT e,
  KML_ERROR *ke);
extern bool KheEventResourceWrite(KHE_EVENT_RESOURCE er, KML_FILE kf);


/*****************************************************************************/
/*                                                                           */
/*  Constraints                                                              */
/*                                                                           */
/*  KHE_CONSTRAINT                                                           */
/*    KHE_ASSIGN_RESOURCE_CONSTRAINT                                         */
/*    KHE_ASSIGN_TIME_CONSTRAINT                                             */
/*    KHE_SPLIT_EVENTS_CONSTRAINT                                            */
/*    KHE_DISTRIBUTE_SPLIT_EVENTS_CONSTRAINT                                 */
/*    KHE_PREFER_RESOURCES_CONSTRAINT                                        */
/*    KHE_PREFER_TIMES_CONSTRAINT                                            */
/*    KHE_AVOID_SPLIT_ASSIGNMENTS_CONSTRAINT                                 */
/*    KHE_SPREAD_EVENTS_CONSTRAINT                                           */
/*    KHE_LINK_EVENTS_CONSTRAINT                                             */
/*    KHE_AVOID_CLASHES_CONSTRAINT                                           */
/*    KHE_AVOID_UNAVAILABLE_TIMES_CONSTRAINT                                 */
/*    KHE_LIMIT_IDLE_TIMES_CONSTRAINT                                        */
/*    KHE_CLUSTER_BUSY_TIMES_CONSTRAINT                                      */
/*    KHE_LIMIT_BUSY_TIMES_CONSTRAINT                                        */
/*    KHE_LIMIT_WORKLOAD_CONSTRAINT                                          */
/*                                                                           */
/*****************************************************************************/

/*****************************************************************************/
/*                                                                           */
/*  khe_constraint.c                                                         */
/*                                                                           */
/*****************************************************************************/

/* construction and query */
extern void KheConstraintFinalize(KHE_CONSTRAINT c);

/* reading and writing */
extern bool KheConstraintMakeFromKml(KML_ELT constraint_elt, KHE_INSTANCE ins,
  KML_ERROR *ke);
extern bool KheConstraintCheckKml(KML_ELT cons_elt, char **id, char **name,
  bool *required, int *weight, KHE_COST_FUNCTION *cf, KML_ERROR *ke);
extern bool KheConstraintAddTimeGroupsFromKml(KHE_CONSTRAINT c,
  KML_ELT elt, KML_ERROR *ke);
extern bool KheConstraintAddTimesFromKml(KHE_CONSTRAINT c,
  KML_ELT elt, KML_ERROR *ke);
extern bool KheConstraintAddResourceGroupsFromKml(KHE_CONSTRAINT c,
  KML_ELT elt, KML_ERROR *ke);
extern bool KheConstraintAddResourcesFromKml(KHE_CONSTRAINT c,
  KML_ELT elt, KML_ERROR *ke);
extern bool KheConstraintAddEventGroupsFromKml(KHE_CONSTRAINT c,
  KML_ELT elt, KML_ERROR *ke);
extern bool KheConstraintAddEventsFromKml(KHE_CONSTRAINT c,
  KML_ELT elt, KML_ERROR *ke);
extern bool KheConstraintWrite(KHE_CONSTRAINT c, KML_FILE kf);
extern bool KheConstraintWriteCommonFields(KHE_CONSTRAINT c, KML_FILE kf);

/* evaluation */
extern KHE_COST KheConstraintCost(KHE_CONSTRAINT c, int dev);
extern KHE_COST KheConstraintCostMulti(KHE_CONSTRAINT c, ARRAY_INT *devs);


/*****************************************************************************/
/*                                                                           */
/*  khe_assign_resource_constraint.c                                         */
/*                                                                           */
/*****************************************************************************/

/* construction and query */
extern int KheAssignResourceConstraintAppliesToCount(
  KHE_ASSIGN_RESOURCE_CONSTRAINT c);
extern void KheAssignResourceConstraintFinalize(
  KHE_ASSIGN_RESOURCE_CONSTRAINT c);

/* reading and writing */
extern bool KheAssignResourceConstraintMakeFromKml(KML_ELT cons_elt,
  KHE_INSTANCE ins, KML_ERROR *ke);
extern bool KheAssignResourceConstraintWrite(KHE_ASSIGN_RESOURCE_CONSTRAINT c,
  KML_FILE kf);


/*****************************************************************************/
/*                                                                           */
/*  khe_assign_time_constraint.c                                             */
/*                                                                           */
/*****************************************************************************/

/* construction and query */
extern int KheAssignTimeConstraintAppliesToCount(KHE_ASSIGN_TIME_CONSTRAINT c);
extern void KheAssignTimeConstraintFinalize(KHE_ASSIGN_TIME_CONSTRAINT c);

/* reading and writing */
extern bool KheAssignTimeConstraintMakeFromKml(KML_ELT cons_elt,
  KHE_INSTANCE ins, KML_ERROR *ke);
extern bool KheAssignTimeConstraintWrite(KHE_ASSIGN_TIME_CONSTRAINT c,
  KML_FILE kf);


/*****************************************************************************/
/*                                                                           */
/*  khe_split_events_constraint.c                                            */
/*                                                                           */
/*****************************************************************************/

/* construction and query */
extern int KheSplitEventsConstraintAppliesToCount(
  KHE_SPLIT_EVENTS_CONSTRAINT c);
extern void KheSplitEventsConstraintFinalize(KHE_SPLIT_EVENTS_CONSTRAINT c);

/* reading and writing */
extern bool KheSplitEventsConstraintMakeFromKml(KML_ELT cons_elt,
  KHE_INSTANCE ins, KML_ERROR *ke);
extern bool KheSplitEventsConstraintWrite(KHE_SPLIT_EVENTS_CONSTRAINT c,
  KML_FILE kf);


/*****************************************************************************/
/*                                                                           */
/*  khe_distribute_split_events_constraint.c                                 */
/*                                                                           */
/*****************************************************************************/

/* construction and query */
extern int KheDistributeSplitEventsConstraintAppliesToCount(
  KHE_DISTRIBUTE_SPLIT_EVENTS_CONSTRAINT c);
extern void KheDistributeSplitEventsConstraintFinalize(
  KHE_DISTRIBUTE_SPLIT_EVENTS_CONSTRAINT c);

/* reading and writing */
extern bool KheDistributeSplitEventsConstraintMakeFromKml(KML_ELT cons_elt,
  KHE_INSTANCE ins, KML_ERROR *ke);
extern bool KheDistributeSplitEventsConstraintWrite(
  KHE_DISTRIBUTE_SPLIT_EVENTS_CONSTRAINT c, KML_FILE kf);


/*****************************************************************************/
/*                                                                           */
/*  khe_prefer_resources_constraint.c                                        */
/*                                                                           */
/*****************************************************************************/

/* construction and query */
extern int KhePreferResourcesConstraintAppliesToCount(
  KHE_PREFER_RESOURCES_CONSTRAINT c);
extern void KhePreferResourcesConstraintFinalize(
  KHE_PREFER_RESOURCES_CONSTRAINT c);

/* reading and writing */
extern bool KhePreferResourcesConstraintMakeFromKml(KML_ELT cons_elt,
  KHE_INSTANCE ins, KML_ERROR *ke);
extern bool KhePreferResourcesConstraintWrite(
  KHE_PREFER_RESOURCES_CONSTRAINT c, KML_FILE kf);


/*****************************************************************************/
/*                                                                           */
/*  khe_prefer_times_constraint.c                                            */
/*                                                                           */
/*****************************************************************************/

/* construction and query */
extern int KhePreferTimesConstraintAppliesToCount(
  KHE_PREFER_TIMES_CONSTRAINT c);
extern void KhePreferTimesConstraintFinalize(KHE_PREFER_TIMES_CONSTRAINT c);

/* reading and writing */
extern bool KhePreferTimesConstraintMakeFromKml(KML_ELT cons_elt,
  KHE_INSTANCE ins, KML_ERROR *ke);
extern bool KhePreferTimesConstraintWrite(KHE_PREFER_TIMES_CONSTRAINT c,
  KML_FILE kf);


/*****************************************************************************/
/*                                                                           */
/*  khe_avoid_split_assignments_constraint.c                                 */
/*                                                                           */
/*****************************************************************************/

/* construction and query */
extern int KheAvoidSplitAssignmentsConstraintAppliesToCount(
  KHE_AVOID_SPLIT_ASSIGNMENTS_CONSTRAINT c);
extern void KheAvoidSplitAssignmentsConstraintFinalize(
  KHE_AVOID_SPLIT_ASSIGNMENTS_CONSTRAINT c);

/* reading and writing */
extern bool KheAvoidSplitAssignmentsConstraintMakeFromKml(KML_ELT cons_elt,
  KHE_INSTANCE ins, KML_ERROR *ke);
extern bool KheAvoidSplitAssignmentsConstraintWrite(
  KHE_AVOID_SPLIT_ASSIGNMENTS_CONSTRAINT c, KML_FILE kf);


/*****************************************************************************/
/*                                                                           */
/*  khe_spread_events_constraint.c                                           */
/*                                                                           */
/*****************************************************************************/

/* construction and query */
extern int KheSpreadEventsConstraintAppliesToCount(
  KHE_SPREAD_EVENTS_CONSTRAINT c);
extern void KheSpreadEventsConstraintFinalize(KHE_SPREAD_EVENTS_CONSTRAINT c);

/* reading and writing */
extern bool KheSpreadEventsConstraintMakeFromKml(KML_ELT cons_elt,
  KHE_INSTANCE ins, KML_ERROR *ke);
extern bool KheSpreadEventsConstraintWrite(KHE_SPREAD_EVENTS_CONSTRAINT c,
  KML_FILE kf);


/*****************************************************************************/
/*                                                                           */
/*  khe_link_events_constraint.c                                             */
/*                                                                           */
/*****************************************************************************/

/* construction and query */
extern int KheLinkEventsConstraintAppliesToCount(KHE_LINK_EVENTS_CONSTRAINT c);
extern void KheLinkEventsConstraintFinalize(KHE_LINK_EVENTS_CONSTRAINT c);

/* reading and writing */
extern bool KheLinkEventsConstraintMakeFromKml(KML_ELT cons_elt,
  KHE_INSTANCE ins, KML_ERROR *ke);
extern bool KheLinkEventsConstraintWrite(KHE_LINK_EVENTS_CONSTRAINT c,
  KML_FILE kf);


/*****************************************************************************/
/*                                                                           */
/*  khe_avoid_clashes_constraint.c                                           */
/*                                                                           */
/*****************************************************************************/

/* construction and query */
extern int KheAvoidClashesConstraintAppliesToCount(
  KHE_AVOID_CLASHES_CONSTRAINT c);
extern void KheAvoidClashesConstraintFinalize(KHE_AVOID_CLASHES_CONSTRAINT c);

/* reading and writing */
extern bool KheAvoidClashesConstraintMakeFromKml(KML_ELT cons_elt,
  KHE_INSTANCE ins, KML_ERROR *ke);
extern bool KheAvoidClashesConstraintWrite(KHE_AVOID_CLASHES_CONSTRAINT c,
  KML_FILE kf);


/*****************************************************************************/
/*                                                                           */
/*  khe_avoid_unavailable_times_constraint.c                                 */
/*                                                                           */
/*****************************************************************************/

/* construction and query */
extern int KheAvoidUnavailableTimesConstraintAppliesToCount(
  KHE_AVOID_UNAVAILABLE_TIMES_CONSTRAINT c);
extern void KheAvoidUnavailableTimesConstraintFinalize(
  KHE_AVOID_UNAVAILABLE_TIMES_CONSTRAINT c);

/* reading and writing */
extern bool KheAvoidUnavailableTimesConstraintMakeFromKml(KML_ELT cons_elt,
  KHE_INSTANCE ins, KML_ERROR *ke);
extern bool KheAvoidUnavailableTimesConstraintWrite(
  KHE_AVOID_UNAVAILABLE_TIMES_CONSTRAINT c, KML_FILE kf);


/*****************************************************************************/
/*                                                                           */
/*  khe_limit_idle_times_constraint.c                                        */
/*                                                                           */
/*****************************************************************************/

/* construction and query */
extern int KheLimitIdleTimesConstraintAppliesToCount(
  KHE_LIMIT_IDLE_TIMES_CONSTRAINT c);
extern void KheLimitIdleTimesConstraintFinalize(
  KHE_LIMIT_IDLE_TIMES_CONSTRAINT c);

/* reading and writing */
extern bool KheLimitIdleTimesConstraintMakeFromKml(KML_ELT cons_elt,
  KHE_INSTANCE ins, KML_ERROR *ke);
extern bool KheLimitIdleTimesConstraintWrite(
  KHE_LIMIT_IDLE_TIMES_CONSTRAINT c, KML_FILE kf);


/*****************************************************************************/
/*                                                                           */
/*  khe_cluster_busy_times_constraint.c                                      */
/*                                                                           */
/*****************************************************************************/

/* construction and query */
extern int KheClusterBusyTimesConstraintAppliesToCount(
  KHE_CLUSTER_BUSY_TIMES_CONSTRAINT c);
extern void KheClusterBusyTimesConstraintFinalize(
  KHE_CLUSTER_BUSY_TIMES_CONSTRAINT c);

/* reading and writing */
extern bool KheClusterBusyTimesConstraintMakeFromKml(KML_ELT cons_elt,
  KHE_INSTANCE ins, KML_ERROR *ke);
extern bool KheClusterBusyTimesConstraintWrite(
  KHE_CLUSTER_BUSY_TIMES_CONSTRAINT c, KML_FILE kf);


/*****************************************************************************/
/*                                                                           */
/*  khe_limit_busy_times_constraint.c                                        */
/*                                                                           */
/*****************************************************************************/

/* construction and query */
extern int KheLimitBusyTimesConstraintAppliesToCount(
  KHE_LIMIT_BUSY_TIMES_CONSTRAINT c);
extern void KheLimitBusyTimesConstraintFinalize(
  KHE_LIMIT_BUSY_TIMES_CONSTRAINT c);

/* reading and writing */
extern bool KheLimitBusyTimesConstraintMakeFromKml(KML_ELT cons_elt,
  KHE_INSTANCE ins, KML_ERROR *ke);
extern bool KheLimitBusyTimesConstraintWrite(
  KHE_LIMIT_BUSY_TIMES_CONSTRAINT c, KML_FILE kf);


/*****************************************************************************/
/*                                                                           */
/*  khe_limit_workload_constraint.c                                          */
/*                                                                           */
/*****************************************************************************/

/* construction and query */
extern int KheLimitWorkloadConstraintAppliesToCount(
  KHE_LIMIT_WORKLOAD_CONSTRAINT c);
extern void KheLimitWorkloadConstraintFinalize(
  KHE_LIMIT_WORKLOAD_CONSTRAINT c);

/* reading and writing */
extern bool KheLimitWorkloadConstraintMakeFromKml(KML_ELT cons_elt,
  KHE_INSTANCE ins, KML_ERROR *ke);
extern bool KheLimitWorkloadConstraintWrite(KHE_LIMIT_WORKLOAD_CONSTRAINT c,
  KML_FILE kf);


/*****************************************************************************/
/*                                                                           */
/*  khe_soln.c                                                               */
/*                                                                           */
/*****************************************************************************/

/* construction and query */
extern void KheSolnSetSolnGroup(KHE_SOLN soln, KHE_SOLN_GROUP soln_group);
extern KHE_SOLN KheSolnCopyPhase1(KHE_SOLN soln);
extern void KheSolnCopyPhase2(KHE_SOLN soln);

/* evenness handler */
extern KHE_EVENNESS_HANDLER KheSolnEvennessHandler(KHE_SOLN soln);

/* traces */
extern KHE_TRACE KheSolnGetTraceFromFreeList(KHE_SOLN soln);
extern void KheSolnAddTraceToFreeList(KHE_SOLN soln, KHE_TRACE t);

/* transactions */
extern KHE_TRANSACTION KheSolnGetTransactionFromFreeList(KHE_SOLN soln);
extern void KheSolnAddTransactionToFreeList(KHE_SOLN soln, KHE_TRANSACTION t);
extern void KheSolnBeginTransaction(KHE_SOLN soln, KHE_TRANSACTION t);
extern void KheSolnEndTransaction(KHE_SOLN soln, KHE_TRANSACTION t);

/* transaction operation loading */
extern void KheSolnOpMeetMake(KHE_SOLN soln, KHE_MEET res);
extern void KheSolnOpMeetDelete(KHE_SOLN soln);
extern void KheSolnOpMeetSplit(KHE_SOLN soln, KHE_MEET meet1, KHE_MEET meet2);
extern void KheSolnOpMeetMerge(KHE_SOLN soln);
extern void KheSolnOpMeetAssign(KHE_SOLN soln, KHE_MEET meet,
  KHE_MEET target_meet, int target_offset);
extern void KheSolnOpMeetUnAssign(KHE_SOLN soln, KHE_MEET meet,
  KHE_MEET target_meet, int target_offset);
extern void KheSolnOpMeetSetDomain(KHE_SOLN soln, KHE_MEET meet,
  KHE_TIME_GROUP old_tg, KHE_TIME_GROUP new_td);

extern void KheSolnOpTaskMake(KHE_SOLN soln, KHE_TASK res);
extern void KheSolnOpTaskDelete(KHE_SOLN soln);
extern void KheSolnOpTaskAssign(KHE_SOLN soln, KHE_TASK task,
  KHE_TASK target_task);
extern void KheSolnOpTaskUnAssign(KHE_SOLN soln, KHE_TASK task,
  KHE_TASK target_task);
extern void KheSolnOpTaskSetDomain(KHE_SOLN soln, KHE_TASK task,
  KHE_RESOURCE_GROUP old_rg, KHE_RESOURCE_GROUP new_rg);

extern void KheSolnOpNodeAddParent(KHE_SOLN soln,
  KHE_NODE child_node, KHE_NODE parent_node);
extern void KheSolnOpNodeDeleteParent(KHE_SOLN soln,
  KHE_NODE child_node, KHE_NODE parent_node);

/* resource in soln objects */
extern KHE_RESOURCE_IN_SOLN KheSolnResourceInSoln(KHE_SOLN soln, int i);

/* event in soln objects */
extern KHE_EVENT_IN_SOLN KheSolnEventInSoln(KHE_SOLN soln, int i);

/* monitors */
extern void KheSolnAddMonitor(KHE_SOLN soln, KHE_MONITOR m, int *index_in_soln);
extern void KheSolnDeleteMonitor(KHE_SOLN soln, KHE_MONITOR m);

/* meets */
extern void KheSolnAddMeet(KHE_SOLN soln, KHE_MEET meet, int *index);
extern void KheSolnDeleteMeet(KHE_SOLN soln, KHE_MEET meet);

/* cycle meets */
extern void KheSolnCycleMeetMerge(KHE_SOLN soln, KHE_MEET meet1,
  KHE_MEET meet2);
extern void KheSolnCycleMeetSplit(KHE_SOLN soln, KHE_MEET meet1,
  KHE_MEET meet2);

/* nodes */
extern void KheSolnAddNode(KHE_SOLN soln, KHE_NODE node, int *index);
extern void KheSolnDeleteNode(KHE_SOLN soln, KHE_NODE node);

/* tasks */
extern void KheSolnAddTask(KHE_SOLN soln, KHE_TASK task, int *index_in_soln);
extern void KheSolnDeleteTask(KHE_SOLN soln, KHE_TASK task);
extern KHE_TASK KheSolnGetTaskFromFreeList(KHE_SOLN soln);
extern void KheSolnAddTaskToFreeList(KHE_SOLN soln, KHE_TASK task);

/* taskings */
extern void KheSolnAddTasking(KHE_SOLN soln, KHE_TASKING tasking, int *index);
extern void KheSolnDeleteTasking(KHE_SOLN soln, KHE_TASKING tasking);
extern KHE_TASKING KheSolnGetTaskingFromFreeList(KHE_SOLN soln);
extern void KheSolnAddTaskingToFreeList(KHE_SOLN soln, KHE_TASKING tasking);

/* matchings - zero domain */
extern ARRAY_SHORT KheSolnMatchingZeroDomain(KHE_SOLN soln);

/* matchings - free supply chunks */
extern KHE_MATCHING_SUPPLY_CHUNK KheSolnMatchingMakeOrdinarySupplyChunk(
  KHE_SOLN soln, KHE_MEET meet);
extern void KheSolnMatchingAddOrdinarySupplyChunkToFreeList(KHE_SOLN soln,
  KHE_MATCHING_SUPPLY_CHUNK sc);

/* matchings - general */
extern KHE_MATCHING KheSolnMatching(KHE_SOLN soln);
extern void KheSolnMatchingUpdate(KHE_SOLN soln);

/* reading and writing */
extern bool KheSolnMakeFromKml(KML_ELT soln_elt, KHE_SOLN_GROUP soln_group,
  KML_ERROR *ke);
extern bool KheSolnWrite(KHE_SOLN soln, bool with_reports, KML_FILE kf);


/*****************************************************************************/
/*                                                                           */
/*  khe_evenness_handler.c                                                   */
/*                                                                           */
/*****************************************************************************/

/* construction and query */
extern KHE_EVENNESS_HANDLER KheEvennessHandlerMake(KHE_SOLN soln);
extern KHE_EVENNESS_HANDLER KheEvennessHandlerCopyPhase1(
  KHE_EVENNESS_HANDLER eh);
extern void KheEvennessHandlerCopyPhase2(KHE_EVENNESS_HANDLER eh);
extern void KheEvennessHandlerDelete(KHE_EVENNESS_HANDLER eh);

/* configuration */
extern void KheEvennessHandlerAttachAllEvennessMonitors(
  KHE_EVENNESS_HANDLER eh);
extern void KheEvennessHandlerDetachAllEvennessMonitors(
  KHE_EVENNESS_HANDLER eh);
extern void KheEvennessHandleSetAllEvennessMonitorWeights(
  KHE_EVENNESS_HANDLER eh, KHE_COST weight);

/* monitoring calls */
extern void KheEvennessHandlerMonitorAttach(KHE_EVENNESS_HANDLER eh);
extern void KheEvennessHandlerMonitorDetach(KHE_EVENNESS_HANDLER eh);
extern void KheEvennessHandlerAddTask(KHE_EVENNESS_HANDLER eh,
  KHE_TASK task, int assigned_time_index);
extern void KheEvennessHandlerDeleteTask(KHE_EVENNESS_HANDLER eh,
  KHE_TASK task, int assigned_time_index);

/* debug */
extern void KheEvennessHandlerDebug(KHE_EVENNESS_HANDLER eh,
  int verbosity, int indent, FILE *fp);


/*****************************************************************************/
/*                                                                           */
/*  khe_time_domain.c                                                        */
/*                                                                           */
/*****************************************************************************/

/* ***
extern KHE_TIME_DOMAIN KheTimeDomainMake(KHE_INSTANCE ins);
extern void KheTimeDomainSetTimeGroup(KHE_TIME_DOMAIN td, int duration,
  KHE_TIME_GROUP tg);
extern KHE_TIME_DOMAIN KheTimeDomainTightened(KHE_TIME_DOMAIN td,
  KHE_TIME_GROUP tg, KHE_SOLN soln);
extern bool KheTimeDomainEqual(KHE_TIME_DOMAIN td1, KHE_TIME_DOMAIN td2);
extern void KheTimeDomainFree(KHE_TIME_DOMAIN td);
*** */


/*****************************************************************************/
/*                                                                           */
/*  khe_time_domain_cache.c                                                  */
/*                                                                           */
/*****************************************************************************/

/* ***
extern KHE_TIME_DOMAIN_CACHE KheTimeDomainCacheMake(void);
extern KHE_TIME_DOMAIN_CACHE KheTimeDomainCacheCopy(KHE_TIME_DOMAIN_CACHE tdc);
extern void KheTimeDomainCacheDelete(KHE_TIME_DOMAIN_CACHE tdc);
extern void KheTimeDomainCacheInsert(KHE_TIME_DOMAIN_CACHE tdc,
  KHE_TIME_DOMAIN td, KHE_TIME_GROUP tg, KHE_TIME_DOMAIN result_td);
extern bool KheTimeDomainCacheRetrieve(KHE_TIME_DOMAIN_CACHE tdc,
  KHE_TIME_DOMAIN td, KHE_TIME_GROUP tg, KHE_TIME_DOMAIN *result_td);
*** */


/*****************************************************************************/
/*                                                                           */
/*  khe_meet.c                                                               */
/*                                                                           */
/*****************************************************************************/

/* construction and query */
extern KHE_MEET KheMeetCopyPhase1(KHE_MEET meet);
extern void KheMeetCopyPhase2(KHE_MEET meet);
extern void KheMeetSetIndex(KHE_MEET meet, int index);
extern void KheMeetSetAssignedTimeIndex(KHE_MEET meet, int assigned_time_index);
extern int KheMeetAssignedTimeIndex(KHE_MEET meet);
extern KHE_EVENT_IN_SOLN KheMeetEventInSoln(KHE_MEET meet);

/* nodes */
extern bool KheMeetAddNodeCheck(KHE_MEET meet, KHE_NODE node);
extern void KheMeetSetNodeInternal(KHE_MEET meet, KHE_NODE node);
extern bool KheMeetDeleteNodeCheck(KHE_MEET meet, KHE_NODE node);
extern void KheMeetFindNodeTarget(KHE_MEET meet, KHE_NODE node,
  KHE_MEET *target_meet, int *target_offset);

/* layers */
/* ***
extern bool KheMeetAddLayerCheck(KHE_MEET meet, KHE_LAYER layer);
extern void KheMeetAddLayer(KHE_MEET meet, KHE_LAYER layer);
extern void KheMeetDeleteLayer(KHE_MEET meet, KHE_LAYER layer);
extern void KheMeetReplaceLayerIndex(KHE_MEET meet,
  int old_index, int new_index);
extern LSET KheMeetAllLayers(KHE_MEET meet, int i);
extern bool KheMeetTouchesLayer(KHE_MEET meet, KHE_LAYER layer);
*** */

/* tasks */
extern void KheMeetAddTask(KHE_MEET meet, KHE_TASK task, int *index_in_meet);
extern void KheMeetDeleteTask(KHE_MEET meet, int task_index);
extern void KheMeetAssignPreassignedResources(KHE_MEET meet,
  KHE_RESOURCE_TYPE rt, bool as_in_event_resource);
extern void KheMeetPartitionTaskCount(KHE_MEET meet, int offset,
  KHE_RESOURCE_GROUP partition, int *count);
extern void KheMeetPartitionTaskDebug(KHE_MEET meet, int offset,
  KHE_RESOURCE_GROUP partition, int verbosity, int indent, FILE *fp);

/* matching */
/* ***
extern void KheMeetAttachMatchingMonitor(KHE_MEET meet,
  KHE_MATCHING_MONITOR m);
extern void KheMeetChangeMatchingMonitorType(KHE_MEET meet,
  KHE_MATCHING_MONITOR m);
extern void KheMeetDetachMatchingMonitor(KHE_MEET meet,
  KHE_MATCHING_MONITOR m);
*** */
extern KHE_MATCHING_DEMAND_CHUNK KheMeetDemandChunk(KHE_MEET meet, int offset);
extern int KheMeetSupplyNodeOffset(KHE_MEET meet, KHE_MATCHING_SUPPLY_NODE sn);
/* ***
extern void KheMeetDemandNodeInfo(KHE_MEET meet,
  KHE_MATCHING_DEMAND_NODE dn, int *offset, KHE_TIME_GROUP *tg);
*** */
extern void KheMeetMatchingAttachAllOrdinaryDemandMonitors(KHE_MEET meet);
extern void KheMeetMatchingDetachAllOrdinaryDemandMonitors(KHE_MEET meet);
extern void KheMeetMatchingSetWeight(KHE_MEET meet, KHE_COST new_weight);
extern void KheMeetMatchingReset(KHE_MEET meet);

/* reading and writing */
extern bool KheMeetMakeFromKml(KML_ELT meet_elt, KHE_SOLN soln, KML_ERROR *ke);
extern int KheMeetAssignedTimeCmp(const void *t1, const void *t2);
extern bool KheMeetMustWrite(KHE_MEET meet);
extern bool KheMeetWrite(KHE_MEET meet, KML_FILE kf);

/* debug */
/* ***
extern void KheMeetDebugDemandChunks(KHE_MEET meet, int verbosity,
  int indent, FILE *fp);
*** */


/*****************************************************************************/
/*                                                                           */
/*  khe_task.c                                                               */
/*                                                                           */
/*****************************************************************************/

/* construction and query */
extern KHE_TASK KheCycleTaskMake(KHE_SOLN soln, KHE_RESOURCE r);
extern KHE_TASK KheTaskCopyPhase1(KHE_TASK task);
extern void KheTaskCopyPhase2(KHE_TASK task);
extern void KheTaskFree(KHE_TASK task);

/* relation with enclosing soln */
extern int KheTaskIndexInSoln(KHE_TASK task);
extern void KheTaskSetIndexInSoln(KHE_TASK task, int num);

/* relation with enclosing meet */
extern int KheTaskIndexInMeet(KHE_TASK task);
extern void KheTaskSetIndexInMeet(KHE_TASK task, int num);

/* relation with enclosing tasking */
extern int KheTaskIndexInTasking(KHE_TASK task);
extern void KheTaskSetTaskingAndIndex(KHE_TASK task, KHE_TASKING tasking,
  int index_in_tasking);
/* ***
extern void KheTaskSetTasking(KHE_TASK task, KHE_TASKING tasking);
extern void KheTaskSetIndexInTasking(KHE_TASK task, int val);
*** */

/* domains */
extern void KheTaskSetDomainUnchecked(KHE_TASK task, KHE_RESOURCE_GROUP rg,
  bool recursive);

/* demand monitor domains */
extern KHE_RESOURCE_GROUP KheTaskMatchingDomain(KHE_TASK task);
extern void KheTaskMatchingReset(KHE_TASK task);

/* interchangeability */
/* ***
extern void KheTaskSort(KHE_TASK task);
extern int KheTaskInterchangeableCmp(const void *t1, const void *t2);
*** */

/* assignment */
/* ***
extern void KheTaskAssignInternal(KHE_TASK task, KHE_RESOURCE r);
extern void KheTaskUnAssignInternal(KHE_TASK task);
*** */

/* splitting and merging etc. */
extern void KheTaskAssignTime(KHE_TASK task, int assigned_time_index);
extern void KheTaskUnAssignTime(KHE_TASK task, int assigned_time_index);
extern KHE_TASK KheTaskSplit(KHE_TASK task, int duration1, KHE_MEET meet2,
  int index_in_meet2);
extern bool KheTaskMergeCheck(KHE_TASK task1, KHE_TASK task2);
extern void KheTaskMerge(KHE_TASK task1, KHE_TASK task2);

/* demand monitors */
extern void KheTaskMatchingAttachAllOrdinaryDemandMonitors(KHE_TASK task);
extern void KheTaskMatchingDetachAllOrdinaryDemandMonitors(KHE_TASK task);
extern void KheTaskMatchingSetWeight(KHE_TASK task, KHE_COST new_weight);
extern void KheTaskAddDemandMonitor(KHE_TASK task,
  KHE_ORDINARY_DEMAND_MONITOR m);
extern void KheTaskDeleteDemandMonitor(KHE_TASK task,
  KHE_ORDINARY_DEMAND_MONITOR m);
extern void KheTaskAttachDemandMonitor(KHE_TASK task,
  KHE_ORDINARY_DEMAND_MONITOR m);
extern void KheTaskDetachDemandMonitor(KHE_TASK task,
  KHE_ORDINARY_DEMAND_MONITOR m);

/* reading and writing */
extern bool KheTaskMakeFromKml(KML_ELT task_elt, KHE_MEET meet,
  KML_ERROR *ke);
extern bool KheTaskMustWrite(KHE_TASK task, KHE_EVENT_RESOURCE er);
extern bool KheTaskWrite(KHE_TASK task, KHE_EVENT_RESOURCE er, KML_FILE kf);


/*****************************************************************************/
/*                                                                           */
/*  khe_transaction.c                                                        */
/*                                                                           */
/*****************************************************************************/

/* construction and query */
extern void KheTransactionFree(KHE_TRANSACTION t);

/* operation loading */
extern void KheTransactionOpMeetMake(KHE_TRANSACTION t, KHE_MEET res);
extern void KheTransactionOpMeetDelete(KHE_TRANSACTION t);
extern void KheTransactionOpMeetSplit(KHE_TRANSACTION t,
  KHE_MEET meet1, KHE_MEET meet2);
extern void KheTransactionOpMeetMerge(KHE_TRANSACTION t);
extern void KheTransactionOpMeetAssign(KHE_TRANSACTION t,
  KHE_MEET meet, KHE_MEET target_meet, int target_offset);
extern void KheTransactionOpMeetUnAssign(KHE_TRANSACTION t,
  KHE_MEET meet, KHE_MEET target_meet, int target_offset);
extern void KheTransactionOpMeetSetDomain(KHE_TRANSACTION t,
  KHE_MEET meet, KHE_TIME_GROUP old_tg, KHE_TIME_GROUP new_tg);

extern void KheTransactionOpTaskMake(KHE_TRANSACTION t,
  KHE_TASK res);
extern void KheTransactionOpTaskDelete(KHE_TRANSACTION t);
extern void KheTransactionOpTaskAssign(KHE_TRANSACTION t,
  KHE_TASK task, KHE_TASK target_task);
extern void KheTransactionOpTaskUnAssign(KHE_TRANSACTION t,
  KHE_TASK task, KHE_TASK target_task);
extern void KheTransactionOpTaskSetDomain(KHE_TRANSACTION t,
  KHE_TASK task, KHE_RESOURCE_GROUP old_rg, KHE_RESOURCE_GROUP new_rg);

extern void KheTransactionOpNodeAddParent(KHE_TRANSACTION t,
  KHE_NODE child_node, KHE_NODE parent_node);
extern void KheTransactionOpNodeDeleteParent(KHE_TRANSACTION t,
  KHE_NODE child_node, KHE_NODE parent_node);

/* cost */
/* ***
extern void KheTransactionUpdate(KHE_TRANSACTION t, KHE_MONITOR m,
  KHE_COST old_cost, KHE_COST new_cost);
*** */


/*****************************************************************************/
/*                                                                           */
/*  khe_event_in_soln.c                                                      */
/*                                                                           */
/*****************************************************************************/

/* construction and query */
extern KHE_EVENT_IN_SOLN KheEventInSolnMake(KHE_SOLN soln, KHE_EVENT e);
extern KHE_EVENT_IN_SOLN KheEventInSolnCopyPhase1(KHE_EVENT_IN_SOLN es);
extern void KheEventInSolnCopyPhase2(KHE_EVENT_IN_SOLN es);
extern bool KheEventInSolnMakeCompleteRepresentation(KHE_EVENT_IN_SOLN es,
  KHE_EVENT *problem_event);
extern KHE_SOLN KheEventInSolnSoln(KHE_EVENT_IN_SOLN es);
extern KHE_EVENT KheEventInSolnEvent(KHE_EVENT_IN_SOLN es);
extern char *KheEventInSolnId(KHE_EVENT_IN_SOLN es);
extern void KheEventInSolnDelete(KHE_EVENT_IN_SOLN es);

/* consistency check */
/* extern void KheEventInSolnConsistencyCheck(KHE_EVENT_IN_SOLN es); */

/* meets */
extern void KheEventInSolnAddMeet(KHE_EVENT_IN_SOLN es, KHE_MEET meet);
extern void KheEventInSolnDeleteMeet(KHE_EVENT_IN_SOLN es, KHE_MEET meet);
extern void KheEventInSolnSplitMeet(KHE_EVENT_IN_SOLN es,
  KHE_MEET meet1, KHE_MEET meet2);
extern void KheEventInSolnMergeMeet(KHE_EVENT_IN_SOLN es,
  KHE_MEET meet1, KHE_MEET meet2);
extern int KheEventInSolnMeetCount(KHE_EVENT_IN_SOLN es);
extern KHE_MEET KheEventInSolnMeet(KHE_EVENT_IN_SOLN es, int i);
extern void KheEventInSolnAssignTime(KHE_EVENT_IN_SOLN es,
  KHE_MEET meet, int assigned_time_index);
extern void KheEventInSolnUnAssignTime(KHE_EVENT_IN_SOLN es,
  KHE_MEET meet, int assigned_time_index);

/* event resources in soln */
extern int KheEventInSolnEventResourceInSolnCount(KHE_EVENT_IN_SOLN es);
extern KHE_EVENT_RESOURCE_IN_SOLN KheEventInSolnEventResourceInSoln(
  KHE_EVENT_IN_SOLN es, int i);

/* reading and writing */
extern bool KheEventInSolnWrite(KHE_EVENT_IN_SOLN es, KML_FILE kf);

/* monitors */
extern void KheEventInSolnAttachMonitor(KHE_EVENT_IN_SOLN es, KHE_MONITOR m);
extern void KheEventInSolnDetachMonitor(KHE_EVENT_IN_SOLN es, KHE_MONITOR m);

/* user monitors, cost, and timetables */
extern void KheEventInSolnAddMonitor(KHE_EVENT_IN_SOLN es, KHE_MONITOR m);
extern void KheEventInSolnDeleteMonitor(KHE_EVENT_IN_SOLN es, KHE_MONITOR m);
extern int KheEventInSolnMonitorCount(KHE_EVENT_IN_SOLN es);
extern KHE_MONITOR KheEventInSolnMonitor(KHE_EVENT_IN_SOLN es, int i);
extern KHE_COST KheEventInSolnCost(KHE_EVENT_IN_SOLN es);
extern KHE_COST KheEventInSolnMonitorCost(KHE_EVENT_IN_SOLN es,
  KHE_MONITOR_TAG tag);
extern KHE_TIMETABLE_MONITOR KheEventInSolnTimetableMonitor(
  KHE_EVENT_IN_SOLN es);

/* debug */
void KheEventInSolnDebug(KHE_EVENT_IN_SOLN es, int verbosity,
  int indent, FILE *fp);


/*****************************************************************************/
/*                                                                           */
/*  khe_event_resource_in_soln.c                                             */
/*                                                                           */
/*****************************************************************************/

/* construction and query */
extern KHE_EVENT_RESOURCE_IN_SOLN KheEventResourceInSolnMake(
  KHE_EVENT_IN_SOLN es, KHE_EVENT_RESOURCE er);
extern KHE_EVENT_RESOURCE_IN_SOLN KheEventResourceInSolnCopyPhase1(
  KHE_EVENT_RESOURCE_IN_SOLN ers);
extern void KheEventResourceInSolnCopyPhase2(KHE_EVENT_RESOURCE_IN_SOLN ers);
extern KHE_EVENT_IN_SOLN KheEventResourceInSolnEventInSoln(
  KHE_EVENT_RESOURCE_IN_SOLN ers);
extern KHE_EVENT_RESOURCE KheEventResourceInSolnEventResource(
  KHE_EVENT_RESOURCE_IN_SOLN ers);
extern char *EventResourceInSolnId(KHE_EVENT_RESOURCE_IN_SOLN ers);
extern void KheEventResourceInSolnDelete(KHE_EVENT_RESOURCE_IN_SOLN ers);

/* soln resources */
extern void KheEventResourceInSolnAddTask(
  KHE_EVENT_RESOURCE_IN_SOLN ers, KHE_TASK task);
extern void KheEventResourceInSolnDeleteTask(
  KHE_EVENT_RESOURCE_IN_SOLN ers, KHE_TASK task);
extern void KheEventResourceInSolnSplitTask(
  KHE_EVENT_RESOURCE_IN_SOLN ers, KHE_TASK task1, KHE_TASK task2);
extern void KheEventResourceInSolnMergeTask(
  KHE_EVENT_RESOURCE_IN_SOLN ers, KHE_TASK task1, KHE_TASK task2);
extern void KheEventResourceInSolnAssignResource(
  KHE_EVENT_RESOURCE_IN_SOLN ers, KHE_TASK task, KHE_RESOURCE r);
extern void KheEventResourceInSolnUnAssignResource(
  KHE_EVENT_RESOURCE_IN_SOLN ers, KHE_TASK task, KHE_RESOURCE r);

extern int KheEventResourceInSolnTaskCount(
  KHE_EVENT_RESOURCE_IN_SOLN ers);
extern KHE_TASK KheEventResourceInSolnTask(
  KHE_EVENT_RESOURCE_IN_SOLN ers, int i);

/* monitors */
extern void KheEventResourceInSolnAttachMonitor(KHE_EVENT_RESOURCE_IN_SOLN ers,
  KHE_MONITOR m);
extern void KheEventResourceInSolnDetachMonitor(KHE_EVENT_RESOURCE_IN_SOLN ers,
  KHE_MONITOR m);

/* user monitors and cost */
extern void KheEventResourceInSolnAddMonitor(KHE_EVENT_RESOURCE_IN_SOLN ers,
  KHE_MONITOR m);
extern void KheEventResourceInSolnDeleteMonitor(KHE_EVENT_RESOURCE_IN_SOLN ers,
  KHE_MONITOR m);
extern int KheEventResourceInSolnMonitorCount(KHE_EVENT_RESOURCE_IN_SOLN ers);
extern KHE_MONITOR KheEventResourceInSolnMonitor(KHE_EVENT_RESOURCE_IN_SOLN ers,
  int i);
extern KHE_COST KheEventResourceInSolnCost(KHE_EVENT_RESOURCE_IN_SOLN ers);
extern KHE_COST KheEventResourceInSolnMonitorCost(
  KHE_EVENT_RESOURCE_IN_SOLN ers, KHE_MONITOR_TAG tag);

/* debug */
void KheEventResourceInSolnDebug(KHE_EVENT_RESOURCE_IN_SOLN ers,
  int verbosity, int indent, FILE *fp);


/*****************************************************************************/
/*                                                                           */
/*  khe_resource_in_soln.c                                                   */
/*                                                                           */
/*****************************************************************************/

/* construction and query */
extern KHE_RESOURCE_IN_SOLN KheResourceInSolnMake(KHE_SOLN soln,
  KHE_RESOURCE r);
extern KHE_RESOURCE_IN_SOLN KheResourceInSolnCopyPhase1(
  KHE_RESOURCE_IN_SOLN rs);
extern void KheResourceInSolnCopyPhase2(KHE_RESOURCE_IN_SOLN rs);
extern KHE_SOLN KheResourceInSolnSoln(KHE_RESOURCE_IN_SOLN rs);
extern KHE_RESOURCE KheResourceInSolnResource(KHE_RESOURCE_IN_SOLN rs);
extern char *KheResourceInSolnId(KHE_RESOURCE_IN_SOLN rs);
extern void KheResourceInSolnDelete(KHE_RESOURCE_IN_SOLN rs);

/* monitors */
extern void KheResourceInSolnAttachMonitor(KHE_RESOURCE_IN_SOLN rs,
  KHE_MONITOR m);
extern void KheResourceInSolnDetachMonitor(KHE_RESOURCE_IN_SOLN rs,
  KHE_MONITOR m);

/* monitoring calls */
extern void KheResourceInSolnSplitTask(KHE_RESOURCE_IN_SOLN rs,
  KHE_TASK task1, KHE_TASK task2);
extern void KheResourceInSolnMergeTask(KHE_RESOURCE_IN_SOLN rs,
  KHE_TASK task1, KHE_TASK task2);
extern void KheResourceInSolnAssignResource(KHE_RESOURCE_IN_SOLN rs,
  KHE_TASK task);
extern void KheResourceInSolnUnAssignResource(KHE_RESOURCE_IN_SOLN rs,
  KHE_TASK task);
extern void KheResourceInSolnAssignTime(KHE_RESOURCE_IN_SOLN rs,
  KHE_TASK task, int assigned_time_index);
extern void KheResourceInSolnUnAssignTime(KHE_RESOURCE_IN_SOLN rs,
  KHE_TASK task, int assigned_time_index);

/* workload requirements (for matchings) */
extern int KheResourceInSolnWorkloadRequirementCount(KHE_RESOURCE_IN_SOLN rs);
extern void KheResourceInSolnWorkloadRequirement(KHE_RESOURCE_IN_SOLN rs,
  int i, int *num, KHE_TIME_GROUP *tg);
extern void KheResourceInSolnBeginWorkloadRequirements(KHE_RESOURCE_IN_SOLN rs);
extern void KheResourceInSolnAddWorkloadRequirement(KHE_RESOURCE_IN_SOLN rs,
  int num, KHE_TIME_GROUP tg);
extern void KheResourceInSolnEndWorkloadRequirements(KHE_RESOURCE_IN_SOLN rs);
/* ***
extern void KheResourceInSolnMatchingSetType(KHE_RESOURCE_IN_SOLN rs,
  KHE_MATCHING_TYPE mt);
*** */
extern void KheResourceInSolnMatchingSetWeight(KHE_RESOURCE_IN_SOLN rs,
  KHE_COST new_weight);

/* assigned tasks */
extern int KheResourceInSolnAssignedTaskCount(KHE_RESOURCE_IN_SOLN rs);
extern KHE_TASK KheResourceInSolnAssignedTask(KHE_RESOURCE_IN_SOLN rs, int i);

/* user monitors, cost, and timetables */
extern void KheResourceInSolnAddMonitor(KHE_RESOURCE_IN_SOLN rs, KHE_MONITOR m);
extern void KheResourceInSolnDeleteMonitor(KHE_RESOURCE_IN_SOLN rs,
  KHE_MONITOR m);
extern int KheResourceInSolnMonitorCount(KHE_RESOURCE_IN_SOLN rs);
extern KHE_MONITOR KheResourceInSolnMonitor(KHE_RESOURCE_IN_SOLN rs, int i);
extern KHE_COST KheResourceInSolnCost(KHE_RESOURCE_IN_SOLN rs);
extern KHE_COST KheResourceInSolnMonitorCost(KHE_RESOURCE_IN_SOLN rs,
  KHE_MONITOR_TAG tag);
extern KHE_TIMETABLE_MONITOR KheResourceInSolnTimetableMonitor(
  KHE_RESOURCE_IN_SOLN rs);

/* debug */
extern void KheResourceInSolnDebug(KHE_RESOURCE_IN_SOLN rs,
  int verbosity, int indent, FILE *fp);


/*****************************************************************************/
/*                                                                           */
/*  khe_trace.c                                                              */
/*                                                                           */
/*****************************************************************************/

extern void KheTraceFree(KHE_TRACE t);
extern void KheTraceChangeCost(KHE_TRACE t, KHE_MONITOR m, KHE_COST old_cost);


/*****************************************************************************/
/*                                                                           */
/*  khe_monitor.c                                                            */
/*                                                                           */
/*****************************************************************************/

/* construction and query */
/* extern void KheMonitorCheck(KHE_MONITOR m); */
extern void KheMonitorChangeCost(KHE_MONITOR m, KHE_COST new_cost);
extern void KheMonitorInitCommonFields(KHE_MONITOR m, KHE_SOLN soln,
  KHE_MONITOR_TAG tag);
extern void KheMonitorCopyCommonFields(KHE_MONITOR copy, KHE_MONITOR orig);
extern void KheMonitorSetIndexInSoln(KHE_MONITOR m, int val);
extern KHE_MONITOR KheMonitorCopyPhase1(KHE_MONITOR m);
extern void KheMonitorCopyPhase2(KHE_MONITOR m);
extern int KheMonitorParentIndex(KHE_MONITOR m);
extern void KheMonitorSetParentMonitorAndIndex(KHE_MONITOR m,
  KHE_GROUP_MONITOR parent_monitor, int parent_index);
extern int KheMonitorDefectIndex(KHE_MONITOR m);
extern void KheMonitorSetDefectIndex(KHE_MONITOR m, int index);
extern void KheMonitorDelete(KHE_MONITOR m);

/* calls emanating from KHE_EVENT_IN_SOLN objects */
extern void KheMonitorAddMeet(KHE_MONITOR m, KHE_MEET meet);
extern void KheMonitorDeleteMeet(KHE_MONITOR m, KHE_MEET meet);
extern void KheMonitorSplitMeet(KHE_MONITOR m, KHE_MEET meet1, KHE_MEET meet2);
extern void KheMonitorMergeMeet(KHE_MONITOR m, KHE_MEET meet1, KHE_MEET meet2);
extern void KheMonitorAssignTime(KHE_MONITOR m, KHE_MEET meet,
  int assigned_time_index);
extern void KheMonitorUnAssignTime(KHE_MONITOR m, KHE_MEET meet,
  int assigned_time_index);

/* calls emanating from KHE_EVENT_RESOURCE_IN_SOLN objects */
extern void KheMonitorAddTask(KHE_MONITOR m, KHE_TASK task);
extern void KheMonitorDeleteTask(KHE_MONITOR m, KHE_TASK task);
extern void KheMonitorSplitTask(KHE_MONITOR m,
  KHE_TASK task1, KHE_TASK task2);
extern void KheMonitorMergeTask(KHE_MONITOR m,
  KHE_TASK task1, KHE_TASK task2);
extern void KheMonitorAssignResource(KHE_MONITOR m,
  KHE_TASK task, KHE_RESOURCE r);
extern void KheMonitorUnAssignResource(KHE_MONITOR m,
  KHE_TASK task, KHE_RESOURCE r);

/* calls emanating from KHE_RESOURCE_IN_SOLN objects */
extern void KheMonitorTaskAssignTime(KHE_MONITOR m,
  KHE_TASK task, int assigned_time_index);
extern void KheMonitorTaskUnAssignTime(KHE_MONITOR m,
  KHE_TASK task, int assigned_time_index);

/* calls emanating from KHE_TIMETABLE_MONITOR objects */
extern void KheMonitorAssignNonClash(KHE_MONITOR m, int assigned_time_index);
extern void KheMonitorUnAssignNonClash(KHE_MONITOR m, int assigned_time_index);
extern void KheMonitorFlush(KHE_MONITOR m);

/* calls emanating from KHE_TIME_GROUP_MONITOR objects */
extern void KheMonitorAddBusyAndIdle(KHE_MONITOR m, int busy_count,
  int idle_count);
extern void KheMonitorDeleteBusyAndIdle(KHE_MONITOR m, int busy_count,
  int idle_count);
extern void KheMonitorChangeBusyAndIdle(KHE_MONITOR m, int old_busy_count,
  int new_busy_count, int old_idle_count, int new_idle_count);

/* debug */
extern void KheMonitorDebugWithTagBegin(KHE_MONITOR m, char *tag,
  int indent, FILE *fp);
extern void KheMonitorDebugBegin(KHE_MONITOR m, int indent, FILE *fp);
extern void KheMonitorDebugEnd(KHE_MONITOR m, bool single_line,
  int indent, FILE *fp);


/*****************************************************************************/
/*                                                                           */
/*  khe_dev_monitor.c                                                        */
/*                                                                           */
/*****************************************************************************/

extern void KheDevMonitorInit(KHE_DEV_MONITOR *dm);
extern void KheDevMonitorCopy(KHE_DEV_MONITOR *target_dm,
  KHE_DEV_MONITOR *source_dm);
extern void KheDevMonitorFree(KHE_DEV_MONITOR *dm);
extern void KheDevMonitorAttach(KHE_DEV_MONITOR *dm, int dev);
extern void KheDevMonitorDetach(KHE_DEV_MONITOR *dm, int dev);
extern void KheDevMonitorUpdate(KHE_DEV_MONITOR *dm, int old_dev, int new_dev);
extern ARRAY_INT *KheDevMonitorDevs(KHE_DEV_MONITOR *dm);
extern ARRAY_INT *KheDevMonitorNewDevs(KHE_DEV_MONITOR *dm);
extern bool KheDevMonitorHasChanged(KHE_DEV_MONITOR *dm);
extern void KheDevMonitorFlush(KHE_DEV_MONITOR *dm);

/* debug */
extern void KheDevMonitorDebug(KHE_DEV_MONITOR *dm, FILE *fp);


/*****************************************************************************/
/*                                                                           */
/*  khe_assign_resource_monitor.c                                            */
/*                                                                           */
/*****************************************************************************/

/* construction and query */
extern KHE_ASSIGN_RESOURCE_MONITOR KheAssignResourceMonitorMake(
  KHE_EVENT_RESOURCE_IN_SOLN ers, KHE_ASSIGN_RESOURCE_CONSTRAINT c);
extern KHE_ASSIGN_RESOURCE_MONITOR KheAssignResourceMonitorCopyPhase1(
  KHE_ASSIGN_RESOURCE_MONITOR m);
extern void KheAssignResourceMonitorCopyPhase2(KHE_ASSIGN_RESOURCE_MONITOR m);
extern void KheAssignResourceMonitorDelete(KHE_ASSIGN_RESOURCE_MONITOR m);

/* attach and detach */
extern void KheAssignResourceMonitorAttachToSoln(KHE_ASSIGN_RESOURCE_MONITOR m);
extern void KheAssignResourceMonitorDetachFromSoln(
  KHE_ASSIGN_RESOURCE_MONITOR m);
extern void KheAssignResourceMonitorAttachCheck(KHE_ASSIGN_RESOURCE_MONITOR m);

/* monitoring calls */
extern void KheAssignResourceMonitorAddTask(
  KHE_ASSIGN_RESOURCE_MONITOR m, KHE_TASK task);
extern void KheAssignResourceMonitorDeleteTask(
  KHE_ASSIGN_RESOURCE_MONITOR m, KHE_TASK task);
extern void KheAssignResourceMonitorSplitTask(
  KHE_ASSIGN_RESOURCE_MONITOR m, KHE_TASK task1, KHE_TASK task2);
extern void KheAssignResourceMonitorMergeTask(
  KHE_ASSIGN_RESOURCE_MONITOR m, KHE_TASK task1, KHE_TASK task2);
extern void KheAssignResourceMonitorAssignResource(
  KHE_ASSIGN_RESOURCE_MONITOR m, KHE_TASK task, KHE_RESOURCE r);
extern void KheAssignResourceMonitorUnAssignResource(
  KHE_ASSIGN_RESOURCE_MONITOR m, KHE_TASK task, KHE_RESOURCE r);

/* deviations */
extern int KheAssignResourceMonitorDeviationCount(
  KHE_ASSIGN_RESOURCE_MONITOR m);
extern int KheAssignResourceMonitorDeviation(
  KHE_ASSIGN_RESOURCE_MONITOR m, int i);
extern char *KheAssignResourceMonitorDeviationDescription(
  KHE_ASSIGN_RESOURCE_MONITOR m, int i);

/* debug */
extern void KheAssignResourceMonitorDebug(
  KHE_ASSIGN_RESOURCE_MONITOR m, int verbosity, int indent, FILE *fp);


/*****************************************************************************/
/*                                                                           */
/*  khe_assign_time_monitor.c                                                */
/*                                                                           */
/*****************************************************************************/

/* construction and query */
extern KHE_ASSIGN_TIME_MONITOR KheAssignTimeMonitorMake(KHE_EVENT_IN_SOLN es,
  KHE_ASSIGN_TIME_CONSTRAINT c);
extern KHE_ASSIGN_TIME_MONITOR KheAssignTimeMonitorCopyPhase1(
  KHE_ASSIGN_TIME_MONITOR m);
extern void KheAssignTimeMonitorCopyPhase2(KHE_ASSIGN_TIME_MONITOR m);
extern void KheAssignTimeMonitorDelete(KHE_ASSIGN_TIME_MONITOR m);

/* attach and detach */
extern void KheAssignTimeMonitorAttachToSoln(KHE_ASSIGN_TIME_MONITOR m);
extern void KheAssignTimeMonitorDetachFromSoln(KHE_ASSIGN_TIME_MONITOR m);
extern void KheAssignTimeMonitorAttachCheck(KHE_ASSIGN_TIME_MONITOR m);

/* monitoring calls */
extern void KheAssignTimeMonitorAddMeet(KHE_ASSIGN_TIME_MONITOR m,
  KHE_MEET meet);
extern void KheAssignTimeMonitorDeleteMeet(KHE_ASSIGN_TIME_MONITOR m,
  KHE_MEET meet);
extern void KheAssignTimeMonitorSplitMeet(KHE_ASSIGN_TIME_MONITOR m,
  KHE_MEET meet1, KHE_MEET meet2);
extern void KheAssignTimeMonitorMergeMeet(KHE_ASSIGN_TIME_MONITOR m,
  KHE_MEET meet1, KHE_MEET meet2);
extern void KheAssignTimeMonitorAssignTime(KHE_ASSIGN_TIME_MONITOR m,
  KHE_MEET meet, int assigned_time_index);
extern void KheAssignTimeMonitorUnAssignTime(KHE_ASSIGN_TIME_MONITOR m,
  KHE_MEET meet, int assigned_time_index);

/* deviations */
extern int KheAssignTimeMonitorDeviationCount(KHE_ASSIGN_TIME_MONITOR m);
extern int KheAssignTimeMonitorDeviation(KHE_ASSIGN_TIME_MONITOR m, int i);
extern char *KheAssignTimeMonitorDeviationDescription(KHE_ASSIGN_TIME_MONITOR m,
  int i);

/* debug */
extern void KheAssignTimeMonitorDebug(KHE_ASSIGN_TIME_MONITOR m,
  int verbosity, int indent, FILE *fp);


/*****************************************************************************/
/*                                                                           */
/*  khe_split_events_monitor.c                                               */
/*                                                                           */
/*****************************************************************************/

/* construction and query */
extern KHE_SPLIT_EVENTS_MONITOR KheSplitEventsMonitorMake(KHE_EVENT_IN_SOLN es,
  KHE_SPLIT_EVENTS_CONSTRAINT c);
extern KHE_SPLIT_EVENTS_MONITOR KheSplitEventsMonitorCopyPhase1(
  KHE_SPLIT_EVENTS_MONITOR m);
extern void KheSplitEventsMonitorCopyPhase2(KHE_SPLIT_EVENTS_MONITOR m);
extern void KheSplitEventsMonitorDelete(KHE_SPLIT_EVENTS_MONITOR m);

/* attach and detach */
extern void KheSplitEventsMonitorAttachToSoln(KHE_SPLIT_EVENTS_MONITOR m);
extern void KheSplitEventsMonitorDetachFromSoln(KHE_SPLIT_EVENTS_MONITOR m);
extern void KheSplitEventsMonitorAttachCheck(KHE_SPLIT_EVENTS_MONITOR m);

/* monitoring calls */
extern void KheSplitEventsMonitorAddMeet(KHE_SPLIT_EVENTS_MONITOR m,
  KHE_MEET meet);
extern void KheSplitEventsMonitorDeleteMeet(KHE_SPLIT_EVENTS_MONITOR m,
  KHE_MEET meet);
extern void KheSplitEventsMonitorSplitMeet(KHE_SPLIT_EVENTS_MONITOR m,
  KHE_MEET meet1, KHE_MEET meet2);
extern void KheSplitEventsMonitorMergeMeet(KHE_SPLIT_EVENTS_MONITOR m,
  KHE_MEET meet1, KHE_MEET meet2);

/* deviations */
extern int KheSplitEventsMonitorDeviationCount(KHE_SPLIT_EVENTS_MONITOR m);
extern int KheSplitEventsMonitorDeviation(KHE_SPLIT_EVENTS_MONITOR m, int i);
extern char *KheSplitEventsMonitorDeviationDescription(
  KHE_SPLIT_EVENTS_MONITOR m, int i);

/* debug */
extern void KheSplitEventsMonitorDebug(KHE_SPLIT_EVENTS_MONITOR m,
  int verbosity, int indent, FILE *fp);


/*****************************************************************************/
/*                                                                           */
/*  khe_distribute_split_events_monitor.c                                    */
/*                                                                           */
/*****************************************************************************/

/* construction and query */
extern KHE_DISTRIBUTE_SPLIT_EVENTS_MONITOR KheDistributeSplitEventsMonitorMake(
  KHE_EVENT_IN_SOLN es, KHE_DISTRIBUTE_SPLIT_EVENTS_CONSTRAINT c);
extern KHE_DISTRIBUTE_SPLIT_EVENTS_MONITOR
  KheDistributeSplitEventsMonitorCopyPhase1(
  KHE_DISTRIBUTE_SPLIT_EVENTS_MONITOR m);
extern void KheDistributeSplitEventsMonitorCopyPhase2(
  KHE_DISTRIBUTE_SPLIT_EVENTS_MONITOR m);
extern void KheDistributeSplitEventsMonitorDelete(
  KHE_DISTRIBUTE_SPLIT_EVENTS_MONITOR m);

/* attach and detach */
extern void KheDistributeSplitEventsMonitorAttachToSoln(
  KHE_DISTRIBUTE_SPLIT_EVENTS_MONITOR m);
extern void KheDistributeSplitEventsMonitorDetachFromSoln(
  KHE_DISTRIBUTE_SPLIT_EVENTS_MONITOR m);
extern void KheDistributeSplitEventsMonitorAttachCheck(
  KHE_DISTRIBUTE_SPLIT_EVENTS_MONITOR m);

/* monitoring calls */
extern void KheDistributeSplitEventsMonitorAddMeet(
  KHE_DISTRIBUTE_SPLIT_EVENTS_MONITOR m, KHE_MEET meet);
extern void KheDistributeSplitEventsMonitorDeleteMeet(
  KHE_DISTRIBUTE_SPLIT_EVENTS_MONITOR m, KHE_MEET meet);
extern void KheDistributeSplitEventsMonitorSplitMeet(
  KHE_DISTRIBUTE_SPLIT_EVENTS_MONITOR m,
  KHE_MEET meet1, KHE_MEET meet2);
extern void KheDistributeSplitEventsMonitorMergeMeet(
  KHE_DISTRIBUTE_SPLIT_EVENTS_MONITOR m,
  KHE_MEET meet1, KHE_MEET meet2);

/* deviations */
extern int KheDistributeSplitEventsMonitorDeviationCount(
  KHE_DISTRIBUTE_SPLIT_EVENTS_MONITOR m);
extern int KheDistributeSplitEventsMonitorDeviation(
  KHE_DISTRIBUTE_SPLIT_EVENTS_MONITOR m, int i);
extern char *KheDistributeSplitEventsMonitorDeviationDescription(
  KHE_DISTRIBUTE_SPLIT_EVENTS_MONITOR m, int i);

/* debug */
extern void KheDistributeSplitEventsMonitorDebug(
  KHE_DISTRIBUTE_SPLIT_EVENTS_MONITOR m, int verbosity, int indent, FILE *fp);


/*****************************************************************************/
/*                                                                           */
/*  khe_prefer_resources_monitor.c                                           */
/*                                                                           */
/*****************************************************************************/

/* construction and query */
extern KHE_PREFER_RESOURCES_MONITOR KhePreferResourcesMonitorMake(
  KHE_EVENT_RESOURCE_IN_SOLN ers, KHE_PREFER_RESOURCES_CONSTRAINT c);
extern KHE_PREFER_RESOURCES_MONITOR KhePreferResourcesMonitorCopyPhase1(
  KHE_PREFER_RESOURCES_MONITOR m);
extern void KhePreferResourcesMonitorCopyPhase2(KHE_PREFER_RESOURCES_MONITOR m);
extern void KhePreferResourcesMonitorDelete(KHE_PREFER_RESOURCES_MONITOR m);

/* attach and detach */
extern void KhePreferResourcesMonitorAttachToSoln(
  KHE_PREFER_RESOURCES_MONITOR m);
extern void KhePreferResourcesMonitorDetachFromSoln(
  KHE_PREFER_RESOURCES_MONITOR m);
extern void KhePreferResourcesMonitorAttachCheck(
  KHE_PREFER_RESOURCES_MONITOR m);

/* monitoring calls */
extern void KhePreferResourcesMonitorAddTask(
  KHE_PREFER_RESOURCES_MONITOR m, KHE_TASK task);
extern void KhePreferResourcesMonitorDeleteTask(
  KHE_PREFER_RESOURCES_MONITOR m, KHE_TASK task);
extern void KhePreferResourcesMonitorSplitTask(
  KHE_PREFER_RESOURCES_MONITOR m, KHE_TASK task1, KHE_TASK task2);
extern void KhePreferResourcesMonitorMergeTask(
  KHE_PREFER_RESOURCES_MONITOR m, KHE_TASK task1, KHE_TASK task2);
extern void KhePreferResourcesMonitorAssignResource(
  KHE_PREFER_RESOURCES_MONITOR m, KHE_TASK task, KHE_RESOURCE r);
extern void KhePreferResourcesMonitorUnAssignResource(
  KHE_PREFER_RESOURCES_MONITOR m, KHE_TASK task, KHE_RESOURCE r);

/* deviations */
extern int KhePreferResourcesMonitorDeviationCount(
  KHE_PREFER_RESOURCES_MONITOR m);
extern int KhePreferResourcesMonitorDeviation(
  KHE_PREFER_RESOURCES_MONITOR m, int i);
extern char *KhePreferResourcesMonitorDeviationDescription(
  KHE_PREFER_RESOURCES_MONITOR m, int i);

/* debug */
extern void KhePreferResourcesMonitorDebug(
  KHE_PREFER_RESOURCES_MONITOR m, int verbosity, int indent, FILE *fp);


/*****************************************************************************/
/*                                                                           */
/*  khe_prefer_times_monitor.c                                               */
/*                                                                           */
/*****************************************************************************/

/* construction and query */
extern KHE_PREFER_TIMES_MONITOR KhePreferTimesMonitorMake(KHE_EVENT_IN_SOLN es,
  KHE_PREFER_TIMES_CONSTRAINT c);
extern KHE_PREFER_TIMES_MONITOR KhePreferTimesMonitorCopyPhase1(
  KHE_PREFER_TIMES_MONITOR m);
extern void KhePreferTimesMonitorCopyPhase2(KHE_PREFER_TIMES_MONITOR m);
extern void KhePreferTimesMonitorDelete(KHE_PREFER_TIMES_MONITOR m);

/* attach and detach */
extern void KhePreferTimesMonitorAttachToSoln(KHE_PREFER_TIMES_MONITOR m);
extern void KhePreferTimesMonitorDetachFromSoln(KHE_PREFER_TIMES_MONITOR m);
extern void KhePreferTimesMonitorAttachCheck(KHE_PREFER_TIMES_MONITOR m);

/* monitoring calls */
extern void KhePreferTimesMonitorAddMeet(KHE_PREFER_TIMES_MONITOR m,
  KHE_MEET meet);
extern void KhePreferTimesMonitorDeleteMeet(KHE_PREFER_TIMES_MONITOR m,
  KHE_MEET meet);
extern void KhePreferTimesMonitorSplitMeet(KHE_PREFER_TIMES_MONITOR m,
  KHE_MEET meet1, KHE_MEET meet2);
extern void KhePreferTimesMonitorMergeMeet(KHE_PREFER_TIMES_MONITOR m,
  KHE_MEET meet1, KHE_MEET meet2);
extern void KhePreferTimesMonitorAssignTime(KHE_PREFER_TIMES_MONITOR m,
  KHE_MEET meet, int assigned_time_index);
extern void KhePreferTimesMonitorUnAssignTime(KHE_PREFER_TIMES_MONITOR m,
  KHE_MEET meet, int assigned_time_index);

/* deviations */
extern int KhePreferTimesMonitorDeviationCount(KHE_PREFER_TIMES_MONITOR m);
extern int KhePreferTimesMonitorDeviation(KHE_PREFER_TIMES_MONITOR m, int i);
extern char *KhePreferTimesMonitorDeviationDescription(
  KHE_PREFER_TIMES_MONITOR m, int i);

/* debug */
extern void KhePreferTimesMonitorDebug(KHE_PREFER_TIMES_MONITOR m,
  int verbosity, int indent, FILE *fp);


/*****************************************************************************/
/*                                                                           */
/*  khe_avoid_split_assignments_monitor.c                                    */
/*                                                                           */
/*****************************************************************************/

/* construction and query */
extern KHE_AVOID_SPLIT_ASSIGNMENTS_MONITOR KheAvoidSplitAssignmentsMonitorMake(
  KHE_SOLN soln, KHE_AVOID_SPLIT_ASSIGNMENTS_CONSTRAINT c, int eg_index);
extern KHE_AVOID_SPLIT_ASSIGNMENTS_MONITOR
  KheAvoidSplitAssignmentsMonitorCopyPhase1(
  KHE_AVOID_SPLIT_ASSIGNMENTS_MONITOR m);
extern void KheAvoidSplitAssignmentsMonitorCopyPhase2(
  KHE_AVOID_SPLIT_ASSIGNMENTS_MONITOR m);
extern void KheAvoidSplitAssignmentsMonitorDelete(
  KHE_AVOID_SPLIT_ASSIGNMENTS_MONITOR m);
/* ***
extern KHE_SOLN KheAvoidSplitAssignmentsMonitorSoln(
  KHE_AVOID_SPLIT_ASSIGNMENTS_MONITOR m);
*** */
extern KHE_AVOID_SPLIT_ASSIGNMENTS_CONSTRAINT
  KheAvoidSplitAssignmentsMonitorConstraint(
  KHE_AVOID_SPLIT_ASSIGNMENTS_MONITOR m);
extern int KheAvoidSplitAssignmentsMonitorEventGroupIndex(
  KHE_AVOID_SPLIT_ASSIGNMENTS_MONITOR m);

/* attach and detach */
extern void KheAvoidSplitAssignmentsMonitorAttachToSoln(
  KHE_AVOID_SPLIT_ASSIGNMENTS_MONITOR m);
extern void KheAvoidSplitAssignmentsMonitorDetachFromSoln(
  KHE_AVOID_SPLIT_ASSIGNMENTS_MONITOR m);
extern void KheAvoidSplitAssignmentsMonitorAttachCheck(
  KHE_AVOID_SPLIT_ASSIGNMENTS_MONITOR m);

/* monitoring calls */
extern void KheAvoidSplitAssignmentsMonitorAddTask(
  KHE_AVOID_SPLIT_ASSIGNMENTS_MONITOR m, KHE_TASK task);
extern void KheAvoidSplitAssignmentsMonitorDeleteTask(
  KHE_AVOID_SPLIT_ASSIGNMENTS_MONITOR m, KHE_TASK task);
extern void KheAvoidSplitAssignmentsMonitorSplitTask(
  KHE_AVOID_SPLIT_ASSIGNMENTS_MONITOR m,
  KHE_TASK task1, KHE_TASK task2);
extern void KheAvoidSplitAssignmentsMonitorMergeTask(
  KHE_AVOID_SPLIT_ASSIGNMENTS_MONITOR m,
  KHE_TASK task1, KHE_TASK task2);
extern void KheAvoidSplitAssignmentsMonitorAssignResource(
  KHE_AVOID_SPLIT_ASSIGNMENTS_MONITOR m, KHE_TASK task, KHE_RESOURCE r);
extern void KheAvoidSplitAssignmentsMonitorUnAssignResource(
  KHE_AVOID_SPLIT_ASSIGNMENTS_MONITOR m, KHE_TASK task, KHE_RESOURCE r);

/* deviations */
extern int KheAvoidSplitAssignmentsMonitorDeviationCount(
  KHE_AVOID_SPLIT_ASSIGNMENTS_MONITOR m);
extern int KheAvoidSplitAssignmentsMonitorDeviation(
  KHE_AVOID_SPLIT_ASSIGNMENTS_MONITOR m, int i);
extern char *KheAvoidSplitAssignmentsMonitorDeviationDescription(
  KHE_AVOID_SPLIT_ASSIGNMENTS_MONITOR m, int i);

/* debug */
void KheAvoidSplitAssignmentsMonitorDebug(
  KHE_AVOID_SPLIT_ASSIGNMENTS_MONITOR m, int verbosity, int indent, FILE *fp);


/*****************************************************************************/
/*                                                                           */
/*  khe_spread_events_monitor.c                                              */
/*                                                                           */
/*****************************************************************************/

/* construction and query */
extern KHE_SPREAD_EVENTS_MONITOR KheSpreadEventsMonitorMake(KHE_SOLN soln,
  KHE_SPREAD_EVENTS_CONSTRAINT c, KHE_EVENT_GROUP eg);
extern KHE_SPREAD_EVENTS_MONITOR KheSpreadEventsMonitorCopyPhase1(
  KHE_SPREAD_EVENTS_MONITOR m);
extern void KheSpreadEventsMonitorCopyPhase2(KHE_SPREAD_EVENTS_MONITOR m);
extern void KheSpreadEventsMonitorDelete(KHE_SPREAD_EVENTS_MONITOR m);

/* attach and detach */
extern void KheSpreadEventsMonitorAttachToSoln(KHE_SPREAD_EVENTS_MONITOR m);
extern void KheSpreadEventsMonitorDetachFromSoln(KHE_SPREAD_EVENTS_MONITOR m);
extern void KheSpreadEventsMonitorAttachCheck(KHE_SPREAD_EVENTS_MONITOR m);

/* monitoring calls */
extern void KheSpreadEventsMonitorAddMeet(KHE_SPREAD_EVENTS_MONITOR m,
  KHE_MEET meet);
extern void KheSpreadEventsMonitorDeleteMeet(KHE_SPREAD_EVENTS_MONITOR m,
  KHE_MEET meet);
extern void KheSpreadEventsMonitorSplitMeet(KHE_SPREAD_EVENTS_MONITOR m,
  KHE_MEET meet1, KHE_MEET meet2);
extern void KheSpreadEventsMonitorMergeMeet(KHE_SPREAD_EVENTS_MONITOR m,
  KHE_MEET meet1, KHE_MEET meet2);
extern void KheSpreadEventsMonitorAssignTime(KHE_SPREAD_EVENTS_MONITOR m,
  KHE_MEET meet, int assigned_time_index);
extern void KheSpreadEventsMonitorUnAssignTime(KHE_SPREAD_EVENTS_MONITOR m,
  KHE_MEET meet, int assigned_time_index);

/* deviations */
extern int KheSpreadEventsMonitorDeviationCount(KHE_SPREAD_EVENTS_MONITOR m);
extern int KheSpreadEventsMonitorDeviation(KHE_SPREAD_EVENTS_MONITOR m, int i);
extern char *KheSpreadEventsMonitorDeviationDescription(
  KHE_SPREAD_EVENTS_MONITOR m, int i);

/* debug */
extern void KheSpreadEventsMonitorDebug(KHE_SPREAD_EVENTS_MONITOR m,
  int verbosity, int indent, FILE *fp);


/*****************************************************************************/
/*                                                                           */
/*  khe_link_events_monitor.c                                                */
/*                                                                           */
/*****************************************************************************/

/* construction and query */
extern KHE_LINK_EVENTS_MONITOR KheLinkEventsMonitorMake(KHE_SOLN soln,
  KHE_LINK_EVENTS_CONSTRAINT c, KHE_EVENT_GROUP eg);
extern KHE_LINK_EVENTS_MONITOR KheLinkEventsMonitorCopyPhase1(
  KHE_LINK_EVENTS_MONITOR m);
extern void KheLinkEventsMonitorCopyPhase2(KHE_LINK_EVENTS_MONITOR m);
extern void KheLinkEventsMonitorDelete(KHE_LINK_EVENTS_MONITOR m);

/* attach and detach */
extern void KheLinkEventsMonitorAttachToSoln(KHE_LINK_EVENTS_MONITOR m);
extern void KheLinkEventsMonitorDetachFromSoln(KHE_LINK_EVENTS_MONITOR m);
extern void KheLinkEventsMonitorAttachCheck(KHE_LINK_EVENTS_MONITOR m);

/* monitoring calls */
extern void KheLinkEventsMonitorAssignNonClash(KHE_LINK_EVENTS_MONITOR m,
  int assigned_time_index);
extern void KheLinkEventsMonitorUnAssignNonClash(KHE_LINK_EVENTS_MONITOR m,
  int assigned_time_index);
extern void KheLinkEventsMonitorFlush(KHE_LINK_EVENTS_MONITOR m);

/* deviations */
extern int KheLinkEventsMonitorDeviationCount(KHE_LINK_EVENTS_MONITOR m);
extern int KheLinkEventsMonitorDeviation(KHE_LINK_EVENTS_MONITOR m, int i);
extern char *KheLinkEventsMonitorDeviationDescription(
  KHE_LINK_EVENTS_MONITOR m, int i);

/* debug */
extern void KheLinkEventsMonitorDebug(KHE_LINK_EVENTS_MONITOR m,
  int verbosity, int indent, FILE *fp);


/*****************************************************************************/
/*                                                                           */
/*  khe_avoid_clashes_monitor.c                                              */
/*                                                                           */
/*****************************************************************************/

/* construction and query */
extern KHE_AVOID_CLASHES_MONITOR KheAvoidClashesMonitorMake(
  KHE_RESOURCE_IN_SOLN rs, KHE_AVOID_CLASHES_CONSTRAINT c);
extern KHE_AVOID_CLASHES_MONITOR KheAvoidClashesMonitorCopyPhase1(
  KHE_AVOID_CLASHES_MONITOR m);
extern void KheAvoidClashesMonitorCopyPhase2(KHE_AVOID_CLASHES_MONITOR m);
extern void KheAvoidClashesMonitorDelete(KHE_AVOID_CLASHES_MONITOR m);

/* attach and detach */
extern void KheAvoidClashesMonitorAttachToSoln(KHE_AVOID_CLASHES_MONITOR m);
extern void KheAvoidClashesMonitorDetachFromSoln(KHE_AVOID_CLASHES_MONITOR m);
extern void KheAvoidClashesMonitorAttachCheck(KHE_AVOID_CLASHES_MONITOR m);

/* monitoring calls */
extern void KheAvoidClashesMonitorChangeClashCount(KHE_AVOID_CLASHES_MONITOR m,
  int old_clash_count, int new_clash_count);
extern void KheAvoidClashesMonitorFlush(KHE_AVOID_CLASHES_MONITOR m);

/* deviations */
extern int KheAvoidClashesMonitorDeviationCount(KHE_AVOID_CLASHES_MONITOR m);
extern int KheAvoidClashesMonitorDeviation(KHE_AVOID_CLASHES_MONITOR m, int i);
extern char *KheAvoidClashesMonitorDeviationDescription(
  KHE_AVOID_CLASHES_MONITOR m, int i);

/* debug */
extern void KheAvoidClashesMonitorDebug(KHE_AVOID_CLASHES_MONITOR m,
  int verbosity, int indent, FILE *fp);


/*****************************************************************************/
/*                                                                           */
/*  khe_avoid_unavailable_times_monitor.c                                    */
/*                                                                           */
/*****************************************************************************/

/* construction and query */
extern KHE_AVOID_UNAVAILABLE_TIMES_MONITOR KheAvoidUnavailableTimesMonitorMake(
  KHE_RESOURCE_IN_SOLN rs, KHE_AVOID_UNAVAILABLE_TIMES_CONSTRAINT c);
extern KHE_AVOID_UNAVAILABLE_TIMES_MONITOR
  KheAvoidUnavailableTimesMonitorCopyPhase1(
    KHE_AVOID_UNAVAILABLE_TIMES_MONITOR m);
extern void KheAvoidUnavailableTimesMonitorCopyPhase2(
  KHE_AVOID_UNAVAILABLE_TIMES_MONITOR m);
extern void KheAvoidUnavailableTimesMonitorDelete(
  KHE_AVOID_UNAVAILABLE_TIMES_MONITOR m);

/* attach and detach */
extern void KheAvoidUnavailableTimesMonitorAttachToSoln(
  KHE_AVOID_UNAVAILABLE_TIMES_MONITOR m);
extern void KheAvoidUnavailableTimesMonitorDetachFromSoln(
  KHE_AVOID_UNAVAILABLE_TIMES_MONITOR m);
extern void KheAvoidUnavailableTimesMonitorAttachCheck(
  KHE_AVOID_UNAVAILABLE_TIMES_MONITOR m);

/* monitoring calls */
extern void KheAvoidUnavailableTimesMonitorAddBusyAndIdle(
  KHE_AVOID_UNAVAILABLE_TIMES_MONITOR m, int busy_count, int idle_count);
extern void KheAvoidUnavailableTimesMonitorDeleteBusyAndIdle(
  KHE_AVOID_UNAVAILABLE_TIMES_MONITOR m, int busy_count, int idle_count);
extern void KheAvoidUnavailableTimesMonitorChangeBusyAndIdle(
  KHE_AVOID_UNAVAILABLE_TIMES_MONITOR m, int old_busy_count,
  int new_busy_count, int old_idle_count, int new_idle_count);

/* deviations */
extern int KheAvoidUnavailableTimesMonitorDeviationCount(
  KHE_AVOID_UNAVAILABLE_TIMES_MONITOR m);
extern int KheAvoidUnavailableTimesMonitorDeviation(
  KHE_AVOID_UNAVAILABLE_TIMES_MONITOR m, int i);
extern char *KheAvoidUnavailableTimesMonitorDeviationDescription(
  KHE_AVOID_UNAVAILABLE_TIMES_MONITOR m, int i);

/* debug */
extern void KheAvoidUnavailableTimesMonitorDebug(
  KHE_AVOID_UNAVAILABLE_TIMES_MONITOR m, int verbosity, int indent, FILE *fp);


/*****************************************************************************/
/*                                                                           */
/*  khe_limit_idle_times_monitor.c                                           */
/*                                                                           */
/*****************************************************************************/

/* construction and query */
extern KHE_LIMIT_IDLE_TIMES_MONITOR KheLimitIdleTimesMonitorMake(
  KHE_RESOURCE_IN_SOLN rs, KHE_LIMIT_IDLE_TIMES_CONSTRAINT c);
extern KHE_LIMIT_IDLE_TIMES_MONITOR KheLimitIdleTimesMonitorCopyPhase1(
  KHE_LIMIT_IDLE_TIMES_MONITOR m);
extern void KheLimitIdleTimesMonitorCopyPhase2(KHE_LIMIT_IDLE_TIMES_MONITOR m);
extern void KheLimitIdleTimesMonitorDelete(KHE_LIMIT_IDLE_TIMES_MONITOR m);
extern KHE_RESOURCE_IN_SOLN KheLimitIdleTimesMonitorResourceInSoln(
  KHE_LIMIT_IDLE_TIMES_MONITOR m);

/* attach and detach */
extern void KheLimitIdleTimesMonitorAttachToSoln(
  KHE_LIMIT_IDLE_TIMES_MONITOR m);
extern void KheLimitIdleTimesMonitorDetachFromSoln(
  KHE_LIMIT_IDLE_TIMES_MONITOR m);
extern void KheLimitIdleTimesMonitorAttachCheck(
  KHE_LIMIT_IDLE_TIMES_MONITOR m);

/* monitoring calls */
extern void KheLimitIdleTimesMonitorAddBusyAndIdle(
  KHE_LIMIT_IDLE_TIMES_MONITOR m, int busy_count, int idle_count);
extern void KheLimitIdleTimesMonitorDeleteBusyAndIdle(
  KHE_LIMIT_IDLE_TIMES_MONITOR m, int busy_count, int idle_count);
extern void KheLimitIdleTimesMonitorChangeBusyAndIdle(
  KHE_LIMIT_IDLE_TIMES_MONITOR m, int old_busy_count,
  int new_busy_count, int old_idle_count, int new_idle_count);

/* deviations */
extern int KheLimitIdleTimesMonitorDeviationCount(
  KHE_LIMIT_IDLE_TIMES_MONITOR m);
extern int KheLimitIdleTimesMonitorDeviation(
  KHE_LIMIT_IDLE_TIMES_MONITOR m, int i);
extern char *KheLimitIdleTimesMonitorDeviationDescription(
  KHE_LIMIT_IDLE_TIMES_MONITOR m, int i);

/* debug */
extern void KheLimitIdleTimesMonitorDebug(KHE_LIMIT_IDLE_TIMES_MONITOR m,
  int verbosity, int indent, FILE *fp);


/*****************************************************************************/
/*                                                                           */
/*  khe_cluster_busy_times_monitor.c                                         */
/*                                                                           */
/*****************************************************************************/

/* construction and query */
extern KHE_CLUSTER_BUSY_TIMES_MONITOR KheClusterBusyTimesMonitorMake(
  KHE_RESOURCE_IN_SOLN rs, KHE_CLUSTER_BUSY_TIMES_CONSTRAINT c);
extern KHE_CLUSTER_BUSY_TIMES_MONITOR KheClusterBusyTimesMonitorCopyPhase1(
  KHE_CLUSTER_BUSY_TIMES_MONITOR m);
extern void KheClusterBusyTimesMonitorCopyPhase2(
  KHE_CLUSTER_BUSY_TIMES_MONITOR m);
extern void KheClusterBusyTimesMonitorDelete(KHE_CLUSTER_BUSY_TIMES_MONITOR m);
extern KHE_RESOURCE_IN_SOLN KheClusterBusyTimesMonitorResourceInSoln(
  KHE_CLUSTER_BUSY_TIMES_MONITOR m);

/* attach and detach */
extern void KheClusterBusyTimesMonitorAttachToSoln(
  KHE_CLUSTER_BUSY_TIMES_MONITOR m);
extern void KheClusterBusyTimesMonitorDetachFromSoln(
  KHE_CLUSTER_BUSY_TIMES_MONITOR m);
extern void KheClusterBusyTimesMonitorAttachCheck(
  KHE_CLUSTER_BUSY_TIMES_MONITOR m);

/* monitoring calls */
extern void KheClusterBusyTimesMonitorAddBusyAndIdle(
  KHE_CLUSTER_BUSY_TIMES_MONITOR m, int busy_count, int idle_count);
extern void KheClusterBusyTimesMonitorDeleteBusyAndIdle(
  KHE_CLUSTER_BUSY_TIMES_MONITOR m, int busy_count, int idle_count);
extern void KheClusterBusyTimesMonitorChangeBusyAndIdle(
  KHE_CLUSTER_BUSY_TIMES_MONITOR m, int old_busy_count,
  int new_busy_count, int old_idle_count, int new_idle_count);

/* deviations */
extern int KheClusterBusyTimesMonitorDeviationCount(
  KHE_CLUSTER_BUSY_TIMES_MONITOR m);
extern int KheClusterBusyTimesMonitorDeviation(
  KHE_CLUSTER_BUSY_TIMES_MONITOR m, int i);
extern char *KheClusterBusyTimesMonitorDeviationDescription(
  KHE_CLUSTER_BUSY_TIMES_MONITOR m, int i);

/* debug */
extern void KheClusterBusyTimesMonitorDebug(KHE_CLUSTER_BUSY_TIMES_MONITOR m,
  int verbosity, int indent, FILE *fp);


/*****************************************************************************/
/*                                                                           */
/*  khe_limit_busy_times_monitor.c                                           */
/*                                                                           */
/*****************************************************************************/

/* construction and query */
extern KHE_LIMIT_BUSY_TIMES_MONITOR KheLimitBusyTimesMonitorMake(
  KHE_RESOURCE_IN_SOLN rs, KHE_LIMIT_BUSY_TIMES_CONSTRAINT c);
extern KHE_LIMIT_BUSY_TIMES_MONITOR KheLimitBusyTimesMonitorCopyPhase1(
  KHE_LIMIT_BUSY_TIMES_MONITOR m);
extern void KheLimitBusyTimesMonitorCopyPhase2(KHE_LIMIT_BUSY_TIMES_MONITOR m);
extern void KheLimitBusyTimesMonitorDelete(KHE_LIMIT_BUSY_TIMES_MONITOR m);
extern KHE_RESOURCE_IN_SOLN KheLimitBusyTimesMonitorResourceInSoln(
  KHE_LIMIT_BUSY_TIMES_MONITOR m);

/* attach and detach */
extern void KheLimitBusyTimesMonitorAttachToSoln(
  KHE_LIMIT_BUSY_TIMES_MONITOR m);
extern void KheLimitBusyTimesMonitorDetachFromSoln(
  KHE_LIMIT_BUSY_TIMES_MONITOR m);
extern void KheLimitBusyTimesMonitorAttachCheck(
  KHE_LIMIT_BUSY_TIMES_MONITOR m);

/* monitoring calls */
extern void KheLimitBusyTimesMonitorAddBusyAndIdle(
  KHE_LIMIT_BUSY_TIMES_MONITOR m, int busy_count, int idle_count);
extern void KheLimitBusyTimesMonitorDeleteBusyAndIdle(
  KHE_LIMIT_BUSY_TIMES_MONITOR m, int busy_count, int idle_count);
extern void KheLimitBusyTimesMonitorChangeBusyAndIdle(
  KHE_LIMIT_BUSY_TIMES_MONITOR m, int old_busy_count,
  int new_busy_count, int old_idle_count, int new_idle_count);

/* deviations */
extern int KheLimitBusyTimesMonitorDeviationCount(
  KHE_LIMIT_BUSY_TIMES_MONITOR m);
extern int KheLimitBusyTimesMonitorDeviation(
  KHE_LIMIT_BUSY_TIMES_MONITOR m, int i);
extern char *KheLimitBusyTimesMonitorDeviationDescription(
  KHE_LIMIT_BUSY_TIMES_MONITOR m, int i);

/* debug */
extern void KheLimitBusyTimesMonitorDebug(KHE_LIMIT_BUSY_TIMES_MONITOR m,
  int verbosity, int indent, FILE *fp);


/*****************************************************************************/
/*                                                                           */
/*  khe_limit_workload_monitor.c                                             */
/*                                                                           */
/*****************************************************************************/

/* construction and query */
extern KHE_LIMIT_WORKLOAD_MONITOR KheLimitWorkloadMonitorMake(
  KHE_RESOURCE_IN_SOLN rs, KHE_LIMIT_WORKLOAD_CONSTRAINT c);
extern KHE_LIMIT_WORKLOAD_MONITOR KheLimitWorkloadMonitorCopyPhase1(
  KHE_LIMIT_WORKLOAD_MONITOR m);
extern void KheLimitWorkloadMonitorCopyPhase2(KHE_LIMIT_WORKLOAD_MONITOR m);
extern void KheLimitWorkloadMonitorDelete(KHE_LIMIT_WORKLOAD_MONITOR m);

/* attach and detach */
extern void KheLimitWorkloadMonitorAttachToSoln(KHE_LIMIT_WORKLOAD_MONITOR m);
extern void KheLimitWorkloadMonitorDetachFromSoln(KHE_LIMIT_WORKLOAD_MONITOR m);
extern void KheLimitWorkloadMonitorAttachCheck(KHE_LIMIT_WORKLOAD_MONITOR m);

/* monitoring calls */
extern void KheLimitWorkloadMonitorAssignResource(
  KHE_LIMIT_WORKLOAD_MONITOR m, KHE_TASK task, KHE_RESOURCE r);
extern void KheLimitWorkloadMonitorUnAssignResource(
  KHE_LIMIT_WORKLOAD_MONITOR m, KHE_TASK task, KHE_RESOURCE r);

/* deviations */
extern int KheLimitWorkloadMonitorDeviationCount(KHE_LIMIT_WORKLOAD_MONITOR m);
extern int KheLimitWorkloadMonitorDeviation(
  KHE_LIMIT_WORKLOAD_MONITOR m, int i);
extern char *KheLimitWorkloadMonitorDeviationDescription(
  KHE_LIMIT_WORKLOAD_MONITOR m, int i);

/* debug */
extern void KheLimitWorkloadMonitorDebug(KHE_LIMIT_WORKLOAD_MONITOR m,
  int verbosity, int indent, FILE *fp);


/*****************************************************************************/
/*                                                                           */
/*  khe_timetable_monitor.c                                                  */
/*                                                                           */
/*****************************************************************************/

/* construction and query */
extern KHE_TIMETABLE_MONITOR KheTimetableMonitorMake(KHE_SOLN soln,
  KHE_RESOURCE_IN_SOLN rs, KHE_EVENT_IN_SOLN es);
extern KHE_TIMETABLE_MONITOR KheTimetableMonitorCopyPhase1(
  KHE_TIMETABLE_MONITOR tm);
extern void KheTimetableMonitorCopyPhase2(KHE_TIMETABLE_MONITOR tm);
extern void KheTimetableMonitorDelete(KHE_TIMETABLE_MONITOR tm);
extern bool KheTimetableMonitorIsOccupied(KHE_TIMETABLE_MONITOR tm,
  int time_index);

/* monitors */
extern void KheTimetableMonitorAttachMonitor(KHE_TIMETABLE_MONITOR tm,
  KHE_MONITOR m);
extern void KheTimetableMonitorDetachMonitor(KHE_TIMETABLE_MONITOR tm,
  KHE_MONITOR m);
extern bool KheTimetableMonitorContainsTimeGroupMonitor(
  KHE_TIMETABLE_MONITOR tm, KHE_TIME_GROUP tg, KHE_TIME_GROUP_MONITOR *tgm);

/* attach and detach */
extern void KheTimetableMonitorAttachToSoln(KHE_TIMETABLE_MONITOR tm);
extern void KheTimetableMonitorDetachFromSoln(KHE_TIMETABLE_MONITOR tm);
extern void KheTimetableMonitorAttachCheck(KHE_TIMETABLE_MONITOR tm);

/* monitoring calls from KHE_EVENT_IN_SOLN */
extern void KheTimetableMonitorAddMeet(KHE_TIMETABLE_MONITOR tm,
  KHE_MEET meet);
extern void KheTimetableMonitorDeleteMeet(KHE_TIMETABLE_MONITOR tm,
  KHE_MEET meet);
extern void KheTimetableMonitorSplitMeet(KHE_TIMETABLE_MONITOR tm,
  KHE_MEET meet1, KHE_MEET meet2);
extern void KheTimetableMonitorMergeMeet(KHE_TIMETABLE_MONITOR tm,
  KHE_MEET meet1, KHE_MEET meet2);
extern void KheTimetableMonitorAssignTime(KHE_TIMETABLE_MONITOR tm,
  KHE_MEET meet, int assigned_time_index);
extern void KheTimetableMonitorUnAssignTime(KHE_TIMETABLE_MONITOR tm,
  KHE_MEET meet, int assigned_time_index);

/* monitoring calls from KHE_RESOURCE_IN_SOLN */
extern void KheTimetableMonitorSplitTask(KHE_TIMETABLE_MONITOR tm,
  KHE_TASK task1, KHE_TASK task2);
extern void KheTimetableMonitorMergeTask(KHE_TIMETABLE_MONITOR tm,
  KHE_TASK task1, KHE_TASK task2);
extern void KheTimetableMonitorTaskAssignTime(KHE_TIMETABLE_MONITOR tm,
  KHE_TASK task, int assigned_time_index);
extern void KheTimetableMonitorTaskUnAssignTime(
  KHE_TIMETABLE_MONITOR tm, KHE_TASK task, int assigned_time_index);
extern void KheTimetableMonitorAssignResource(KHE_TIMETABLE_MONITOR tm,
  KHE_TASK task, KHE_RESOURCE r);
extern void KheTimetableMonitorUnAssignResource(KHE_TIMETABLE_MONITOR tm,
  KHE_TASK task, KHE_RESOURCE r);

/* deviations */
extern int KheTimetableMonitorDeviationCount(KHE_TIMETABLE_MONITOR m);
extern int KheTimetableMonitorDeviation(KHE_TIMETABLE_MONITOR m, int i);
extern char *KheTimetableMonitorDeviationDescription(
  KHE_TIMETABLE_MONITOR m, int i);

/* debug */
extern void KheTimetableMonitorDebug(KHE_TIMETABLE_MONITOR tm, int verbosity,
  int indent, FILE *fp);


/*****************************************************************************/
/*                                                                           */
/*  khe_time_group_monitor.c                                                 */
/*                                                                           */
/*****************************************************************************/

/* construction and query */
extern KHE_TIME_GROUP_MONITOR KheTimeGroupMonitorMake(KHE_TIMETABLE_MONITOR tm,
  KHE_TIME_GROUP tg);
extern KHE_TIME_GROUP_MONITOR KheTimeGroupMonitorCopyPhase1(
  KHE_TIME_GROUP_MONITOR tgm);
extern void KheTimeGroupMonitorCopyPhase2(KHE_TIME_GROUP_MONITOR tgm);
extern void KheTimeGroupMonitorDelete(KHE_TIME_GROUP_MONITOR tgm);
extern KHE_TIMETABLE_MONITOR KheTimeGroupMonitorTimetableMonitor(KHE_TIME_GROUP_MONITOR tgm);
extern KHE_TIME_GROUP KheTimeGroupMonitorTimeGroup(KHE_TIME_GROUP_MONITOR tgm);

/* attach and detach */
extern void KheTimeGroupMonitorAttachToSoln(KHE_TIME_GROUP_MONITOR tgm);
extern void KheTimeGroupMonitorDetachFromSoln(KHE_TIME_GROUP_MONITOR tgm);
extern void KheTimeGroupMonitorAttachCheck(KHE_TIME_GROUP_MONITOR tgm);

/* monitors */
extern void KheTimeGroupMonitorAttachMonitor(KHE_TIME_GROUP_MONITOR tgm,
  KHE_MONITOR m);
extern void KheTimeGroupMonitorDetachMonitor(KHE_TIME_GROUP_MONITOR tgm,
  KHE_MONITOR m);

/* monitoring calls */
extern void KheTimeGroupMonitorAssignNonClash(KHE_TIME_GROUP_MONITOR tgm,
  int assigned_time_index);
extern void KheTimeGroupMonitorUnAssignNonClash(KHE_TIME_GROUP_MONITOR tgm,
  int assigned_time_index);
extern void KheTimeGroupMonitorFlush(KHE_TIME_GROUP_MONITOR tgm);
extern int KheTimeGroupMonitorIdleTimes(KHE_TIME_GROUP_MONITOR tgm);
extern int KheTimeGroupMonitorBusyTimes(KHE_TIME_GROUP_MONITOR tgm);

/* deviations */
extern int KheTimeGroupMonitorDeviationCount(KHE_TIME_GROUP_MONITOR m);
extern int KheTimeGroupMonitorDeviation(KHE_TIME_GROUP_MONITOR m, int i);
extern char *KheTimeGroupMonitorDeviationDescription(
  KHE_TIME_GROUP_MONITOR m, int i);

/* debug */
extern void KheTimeGroupMonitorDebug(KHE_TIME_GROUP_MONITOR tgm,
  int verbosity, int indent, FILE *fp);


/*****************************************************************************/
/*                                                                           */
/*  khe_group_monitor.c                                                      */
/*                                                                           */
/*****************************************************************************/

/* construction and query */
extern KHE_GROUP_MONITOR KheGroupMonitorCopyPhase1(KHE_GROUP_MONITOR gm);
extern void KheGroupMonitorCopyPhase2(KHE_GROUP_MONITOR gm);

/* ***
extern void KheGroupMonitorAddMonitor(KHE_GROUP_MONITOR gm, KHE_MONITOR m);
extern void KheGroupMonitorDeleteMonitor(KHE_GROUP_MONITOR gm, KHE_MONITOR m);
*** */

/* attach and detach */
/* *** no longer used with group monitors
extern void KheGroupMonitorAttach(KHE_GROUP_MONITOR gm);
extern void KheGroupMonitorDetach(KHE_GROUP_MONITOR gm);
*** */

/* update */
extern void KheGroupMonitorBeginTrace(KHE_GROUP_MONITOR gm, KHE_TRACE t);
extern void KheGroupMonitorEndTrace(KHE_GROUP_MONITOR gm, KHE_TRACE t);
extern void KheGroupMonitorChangeCost(KHE_GROUP_MONITOR gm, KHE_MONITOR m,
  KHE_COST old_cost, KHE_COST new_cost);
/* ***
extern void KheGroupMonitorChangeCost(KHE_GROUP_MONITOR gm, KHE_MONITOR m,
  KHE_MONITOR_TAG orig_tag, KHE_COST delta_cost, bool first_trace);
extern void KheGroupMonitorUpdate(KHE_GROUP_MONITOR gm, KHE_MONITOR m,
  KHE_COST old_cost, KHE_COST new_cost);
*** */

/* deviations */
extern int KheGroupMonitorDeviationCount(KHE_GROUP_MONITOR m);
extern int KheGroupMonitorDeviation(KHE_GROUP_MONITOR m, int i);
extern char *KheGroupMonitorDeviationDescription(KHE_GROUP_MONITOR m, int i);

/* debug */
extern void KheGroupMonitorDebug(KHE_GROUP_MONITOR gm,
  int verbosity, int indent, FILE *fp);


/*****************************************************************************/
/*                                                                           */
/*  khe_ordinary_demand_monitor.c                                            */
/*                                                                           */
/*****************************************************************************/

/* construction and query */
extern KHE_ORDINARY_DEMAND_MONITOR KheOrdinaryDemandMonitorMake(KHE_SOLN soln,
  KHE_MATCHING_DEMAND_CHUNK dc, KHE_TASK task, int offset);
extern void KheOrdinaryDemandMonitorSetTaskAndOffset(
  KHE_ORDINARY_DEMAND_MONITOR m, KHE_TASK task, int offset);
/* ***
extern void KheOrdinaryDemandMonitorSetOffset(KHE_ORDINARY_DEMAND_MONITOR m,
  int offset);
*** */
extern KHE_ORDINARY_DEMAND_MONITOR KheOrdinaryDemandMonitorCopyPhase1(
  KHE_ORDINARY_DEMAND_MONITOR m);
extern void KheOrdinaryDemandMonitorCopyPhase2(KHE_ORDINARY_DEMAND_MONITOR m);
extern void KheOrdinaryDemandMonitorDelete(KHE_ORDINARY_DEMAND_MONITOR m);

/* attach and detach */
extern void KheOrdinaryDemandMonitorAttachToSoln(
  KHE_ORDINARY_DEMAND_MONITOR m);
extern void KheOrdinaryDemandMonitorDetachFromSoln(
  KHE_ORDINARY_DEMAND_MONITOR m);
extern void KheOrdinaryDemandMonitorAttachCheck(
  KHE_ORDINARY_DEMAND_MONITOR m);

/* monitoring calls */
extern void KheOrdinaryDemandMonitorSetDomain(KHE_ORDINARY_DEMAND_MONITOR m,
  KHE_RESOURCE_GROUP rg, KHE_MATCHING_DOMAIN_CHANGE_TYPE change_type);
extern void KheOrdinaryDemandMonitorSetWeight(KHE_ORDINARY_DEMAND_MONITOR m,
  KHE_COST new_weight);
/* ***
extern void KheOrdinaryDemandMonitorChangeType(KHE_ORDINARY_DEMAND_MONITOR m,
  KHE_MATCHING_TYPE old_mt, KHE_MATCHING_TYPE new_mt);
*** */

/* deviations */
extern int KheOrdinaryDemandMonitorDeviationCount(
  KHE_ORDINARY_DEMAND_MONITOR m);
extern int KheOrdinaryDemandMonitorDeviation(
  KHE_ORDINARY_DEMAND_MONITOR m, int i);
extern char *KheOrdinaryDemandMonitorDeviationDescription(
  KHE_ORDINARY_DEMAND_MONITOR m, int i);

/* debug */
extern void KheOrdinaryDemandMonitorDebug(KHE_ORDINARY_DEMAND_MONITOR m,
  int verbosity, int indent, FILE *fp);


/*****************************************************************************/
/*                                                                           */
/*  khe_workload_demand_monitor.c                                            */
/*                                                                           */
/*****************************************************************************/

/* construction and query */
extern KHE_WORKLOAD_DEMAND_MONITOR KheWorkloadDemandMonitorMake(KHE_SOLN soln,
  KHE_MATCHING_DEMAND_CHUNK dc, KHE_RESOURCE_IN_SOLN rs, KHE_TIME_GROUP tg);
extern KHE_WORKLOAD_DEMAND_MONITOR KheWorkloadDemandMonitorCopyPhase1(
  KHE_WORKLOAD_DEMAND_MONITOR m);
extern void KheWorkloadDemandMonitorCopyPhase2(KHE_WORKLOAD_DEMAND_MONITOR m);
extern void KheWorkloadDemandMonitorDelete(KHE_WORKLOAD_DEMAND_MONITOR m);

/* attach and detach */
extern void KheWorkloadDemandMonitorAttachToSoln(
  KHE_WORKLOAD_DEMAND_MONITOR m);
extern void KheWorkloadDemandMonitorDetachFromSoln(
  KHE_WORKLOAD_DEMAND_MONITOR m);
extern void KheWorkloadDemandMonitorAttachCheck(
  KHE_WORKLOAD_DEMAND_MONITOR m);

/* monitoring calls */
extern void KheWorkloadDemandMonitorSetWeight(KHE_WORKLOAD_DEMAND_MONITOR m,
  KHE_COST new_weight);

/* deviations */
extern int KheWorkloadDemandMonitorDeviationCount(
  KHE_WORKLOAD_DEMAND_MONITOR m);
extern int KheWorkloadDemandMonitorDeviation(
  KHE_WORKLOAD_DEMAND_MONITOR m, int i);
extern char *KheWorkloadDemandMonitorDeviationDescription(
  KHE_WORKLOAD_DEMAND_MONITOR m, int i);

/* debug */
extern void KheWorkloadDemandMonitorDebug(KHE_WORKLOAD_DEMAND_MONITOR m,
  int verbosity, int indent, FILE *fp);


/*****************************************************************************/
/*                                                                           */
/*  khe_evenness_monitor.c                                                   */
/*                                                                           */
/*****************************************************************************/

/* construction and query */
extern KHE_EVENNESS_MONITOR KheEvennessMonitorMake(KHE_SOLN soln,
  KHE_RESOURCE_GROUP partition, KHE_TIME time);
extern KHE_EVENNESS_MONITOR KheEvennessMonitorCopyPhase1(
  KHE_EVENNESS_MONITOR m);
extern void KheEvennessMonitorCopyPhase2(KHE_EVENNESS_MONITOR m);
extern void KheEvennessMonitorDelete(KHE_EVENNESS_MONITOR m);

/* attach and detach */
extern void KheEvennessMonitorAttachToSoln(KHE_EVENNESS_MONITOR m);
extern void KheEvennessMonitorDetachFromSoln(KHE_EVENNESS_MONITOR m);
extern void KheEvennessMonitorAttachCheck(KHE_EVENNESS_MONITOR m);

/* monitoring calls */
void KheEvennessMonitorAddTask(KHE_EVENNESS_MONITOR m);
void KheEvennessMonitorDeleteTask(KHE_EVENNESS_MONITOR m);

/* deviations */
extern int KheEvennessMonitorDeviationCount(KHE_EVENNESS_MONITOR m);
extern int KheEvennessMonitorDeviation(KHE_EVENNESS_MONITOR m, int i);
extern char *KheEvennessMonitorDeviationDescription(KHE_EVENNESS_MONITOR m,
  int i);

/* debug */
extern void KheEvennessMonitorDebug(KHE_EVENNESS_MONITOR m,
  int verbosity, int indent, FILE *fp);


/*****************************************************************************/
/*                                                                           */
/*  khe_matching_monitor.c                                                   */
/*                                                                           */
/*****************************************************************************/

/* construction and query */
/* ***
extern KHE_MATCHING_MONITOR KheMatchingMonitorMake(KHE_SOLN soln);
extern ARRAY_SHORT KheMatchingMonitorZeroDomain(KHE_MATCHING_MONITOR m);
extern KHE_MATCHING_MONITOR KheMatchingMonitorCopyPhase1(
  KHE_MATCHING_MONITOR m);
extern void KheMatchingMonitorCopyPhase2(KHE_MATCHING_MONITOR m);
extern void KheMatchingMonitorDelete(KHE_MATCHING_MONITOR m);
extern KHE_MATCHING KheMatchingMonitorMatching(KHE_MATCHING_MONITOR m);
extern void KheMatchingMonitorUpdate(KHE_MATCHING_MONITOR m);
*** */

/* attach and detach */
/* ***
extern void KheMatchingMonitorAttachToSoln(KHE_MATCHING_MONITOR m);
extern void KheMatchingMonitorDetachFromSoln(KHE_MATCHING_MONITOR m);
*** */

/* ordinary supply chunks */
/* ***
extern KHE_MATCHING_SUPPLY_CHUNK KheMatchingMonitorMakeOrdinarySupplyChunk(
  KHE_MATCHING_MONITOR m);
extern void KheMatchingMonitorAddOrdinarySupplyChunkToFreeList(
  KHE_MATCHING_MONITOR m, KHE_MATCHING_SUPPLY_CHUNK sc);
*** */

/* debug */
/* ***
extern void KheMatchingMonitorDemandNodeShow(KHE_MATCHING_DEMAND_NODE dn,
  int col_width, FILE *fp);
extern void KheMatchingMonitorSupplyNodeShow(KHE_MATCHING_SUPPLY_NODE sn,
  int col_width, FILE *fp);
extern void KheMatchingMonitorDebug(KHE_MATCHING_MONITOR m,
  int verbosity, int indent, FILE *fp);
*** */


/*****************************************************************************/
/*                                                                           */
/*  khe_node.c                                                               */
/*                                                                           */
/*****************************************************************************/

/* construction and query */
extern void KheNodeSetIndex(KHE_NODE node, int index);
extern KHE_NODE KheNodeCopyPhase1(KHE_NODE node);
extern void KheNodeCopyPhase2(KHE_NODE node);

/* unchecked state-changing operations */
/* ***
extern void KheNodeUncheckedAddLayer(KHE_NODE node, KHE_MEET meet,
  KHE_LAYER layer);
extern void KheNodeUncheckedDeleteLayer(KHE_NODE node, KHE_MEET meet,
  KHE_LAYER layer);
*** */

/* parent layers */
extern void KheNodeAddParentLayer(KHE_NODE child_node, KHE_LAYER layer);
extern void KheNodeDeleteParentLayer(KHE_NODE child_node, KHE_LAYER layer);

/* child layers */
extern void KheNodeAddChildLayer(KHE_NODE parent_node, KHE_LAYER layer,
  int *index);
extern void KheNodeDeleteChildLayer(KHE_NODE parent_node, KHE_LAYER layer);

/* meets */
extern void KheNodeAddSplitMeet(KHE_NODE node, KHE_MEET meet);
extern void KheNodeDeleteSplitMeet(KHE_NODE node, KHE_MEET meet);

/* zones */
extern void KheNodeAddZone(KHE_NODE node, KHE_ZONE zone, int *index);
extern void KheNodeDeleteZone(KHE_NODE node, KHE_ZONE zone, int index);


/*****************************************************************************/
/*                                                                           */
/*  khe_layer.c                                                              */
/*                                                                           */
/*****************************************************************************/

/* construction and query */
extern void KheLayerSetIndex(KHE_LAYER layer, int index);
extern void KheLayerChangeNodeIndex(KHE_LAYER layer, int old_index,
  int new_index);
extern KHE_LAYER KheLayerCopyPhase1(KHE_LAYER layer);
extern void KheLayerCopyPhase2(KHE_LAYER layer);

extern void KheLayerAddDuration(KHE_LAYER layer, int durn);
extern void KheLayerSubtractDuration(KHE_LAYER layer, int durn);


/*****************************************************************************/
/*                                                                           */
/*  khe_zone.c                                                               */
/*                                                                           */
/*****************************************************************************/

/* construction and query */
extern void KheZoneSetIndex(KHE_ZONE zone, int index);
extern KHE_ZONE KheZoneCopyPhase1(KHE_ZONE zone);
extern void KheZoneCopyPhase2(KHE_ZONE zone);
extern void KheZoneAddMeetOffset(KHE_ZONE zone, KHE_MEET meet, int offset);
extern void KheZoneDeleteMeetOffset(KHE_ZONE zone, KHE_MEET meet, int offset);
extern void KheZoneUpdateMeetOffset(KHE_ZONE zone, KHE_MEET old_meet,
  int old_offset, KHE_MEET new_meet, int new_offset);


/*****************************************************************************/
/*                                                                           */
/*  khe_tasking.c                                                            */
/*                                                                           */
/*****************************************************************************/

extern KHE_TASKING KheTaskingCopyPhase1(KHE_TASKING tasking);
extern void KheTaskingCopyPhase2(KHE_TASKING tasking);
extern int KheTaskingIndexInSoln(KHE_TASKING tasking);
extern void KheTaskingSetIndexInSoln(KHE_TASKING tasking, int val);
extern void KheTaskingFree(KHE_TASKING tasking);
