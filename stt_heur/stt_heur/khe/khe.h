
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
/*  FILE:         khe.h                                                      */
/*  DESCRIPTION:  Include this file whenever you use KHE                     */
/*                                                                           */
/*  This file has two parts:  type declarations and function declarations.   */
/*  Each part is organized to parallel the User's Guide exactly:             */
/*                                                                           */
/*                    Part A:  The Platform                                  */
/*                                                                           */
/*    Chapter 1.   Introduction                                              */
/*    Chapter 2.   Archives and Solution Groups                              */
/*    Chapter 3.   Instances                                                 */
/*    Chapter 4.   Solutions                                                 */
/*    Chapter 5.   Extra Types for Solving                                   */
/*    Chapter 6.   Solution Monitoring                                       */
/*    Chapter 7.   Matchings and Evenness                                    */
/*    Chapter 8.   Monitor Grouping and Tracing                              */
/*                                                                           */
/*                    Part B:  Some Solvers                                  */
/*                                                                           */
/*    Chapter 9.   Layer Tree Solvers                                        */
/*    Chapter 10.  Time Solvers                                              */
/*    Chapter 11.  Resource Solvers                                          */
/*    Chapter 12.  General solvers                                           */
/*    Chapter 13.  Ejection Chains                                           */
/*                                                                           */
/*    Appendix A.  Variable-Length Arrays with M (see file m.h)              */
/*    Appendix B.  Variable-Length Bitsets (see file khe_lset.h)             */
/*    Appendix C.  Priority Queues (see file khe_priqueue.h)                 */
/*    Appendix D.  XML Handling with KML (see file kml.h)                    */
/*                                                                           */
/*  This makes it easy to verify that KHE offers what the User's Guide says  */
/*  it offers.                                                               */
/*                                                                           */
/*****************************************************************************/
#ifndef KHE_HEADER_FILE
#define KHE_HEADER_FILE

#define	KHE_VERSION   "2012_01_17"
#define KHE_USE_PTHREAD 1
#include <stdio.h>
#include <stdint.h>
#include <stdbool.h>
#include "kml.h"

#define KHE_NO_DURATION 0


/*****************************************************************************/
/*                                                                           */
/*                        TYPE DECLARATIONS                                  */
/*                                                                           */
/*****************************************************************************/


/*****************************************************************************/
/*                                                                           */
/*                    Part A: The Platform                                   */
/*                                                                           */
/*****************************************************************************/

/*****************************************************************************/
/*                                                                           */
/*    Chapter 1.   Introduction                                              */
/*                                                                           */
/*****************************************************************************/


/*****************************************************************************/
/*                                                                           */
/*    Chapter 2.   Archives and Solution Groups                              */
/*                                                                           */
/*****************************************************************************/

typedef struct khe_archive_rec *KHE_ARCHIVE;
typedef struct khe_archive_metadata_rec *KHE_ARCHIVE_METADATA;
typedef struct khe_soln_group_rec *KHE_SOLN_GROUP;
typedef struct khe_soln_group_metadata_rec *KHE_SOLN_GROUP_METADATA;


/*****************************************************************************/
/*                                                                           */
/*    Chapter 3.   Instances                                                 */
/*                                                                           */
/*****************************************************************************/

/* 3.1 Instances */
typedef struct khe_instance_rec *KHE_INSTANCE;
typedef struct khe_instance_metadata_rec *KHE_INSTANCE_METADATA;

/* 3.2 Times */
typedef enum {
  KHE_TIME_GROUP_KIND_WEEK,
  KHE_TIME_GROUP_KIND_DAY,
  KHE_TIME_GROUP_KIND_ORDINARY
} KHE_TIME_GROUP_KIND;

typedef struct khe_time_group_rec *KHE_TIME_GROUP;
typedef struct khe_time_rec *KHE_TIME;

/* 3.3 Resources */
typedef struct khe_resource_type_rec *KHE_RESOURCE_TYPE;
typedef struct khe_resource_group_rec *KHE_RESOURCE_GROUP;
typedef struct khe_resource_rec *KHE_RESOURCE;

/* 3.4 Events */
typedef enum {
  KHE_EVENT_GROUP_KIND_COURSE,
  KHE_EVENT_GROUP_KIND_ORDINARY
} KHE_EVENT_GROUP_KIND;

typedef struct khe_event_group_rec *KHE_EVENT_GROUP;
typedef struct khe_event_rec *KHE_EVENT;
typedef struct khe_event_resource_rec *KHE_EVENT_RESOURCE;
typedef struct khe_event_resource_group_rec *KHE_EVENT_RESOURCE_GROUP;

/* 3.5 Constraints */
typedef struct khe_constraint_rec *KHE_CONSTRAINT;

typedef enum {
  KHE_SUM_STEPS_COST_FUNCTION,
  KHE_STEP_SUM_COST_FUNCTION,
  KHE_SUM_COST_FUNCTION,
  KHE_SUM_SQUARES_COST_FUNCTION,
  KHE_SQUARE_SUM_COST_FUNCTION
} KHE_COST_FUNCTION;

typedef enum {
  KHE_ASSIGN_RESOURCE_CONSTRAINT_TAG,
  KHE_ASSIGN_TIME_CONSTRAINT_TAG,
  KHE_SPLIT_EVENTS_CONSTRAINT_TAG,
  KHE_DISTRIBUTE_SPLIT_EVENTS_CONSTRAINT_TAG,
  KHE_PREFER_RESOURCES_CONSTRAINT_TAG,
  KHE_PREFER_TIMES_CONSTRAINT_TAG,
  KHE_AVOID_SPLIT_ASSIGNMENTS_CONSTRAINT_TAG,
  KHE_SPREAD_EVENTS_CONSTRAINT_TAG,
  KHE_LINK_EVENTS_CONSTRAINT_TAG,
  KHE_AVOID_CLASHES_CONSTRAINT_TAG,
  KHE_AVOID_UNAVAILABLE_TIMES_CONSTRAINT_TAG,
  KHE_LIMIT_IDLE_TIMES_CONSTRAINT_TAG,
  KHE_CLUSTER_BUSY_TIMES_CONSTRAINT_TAG,
  KHE_LIMIT_BUSY_TIMES_CONSTRAINT_TAG,
  KHE_LIMIT_WORKLOAD_CONSTRAINT_TAG,
  KHE_CONSTRAINT_TAG_COUNT
} KHE_CONSTRAINT_TAG;

typedef struct khe_limited_time_group_rec *KHE_LIMITED_TIME_GROUP;
typedef struct khe_time_spread_rec *KHE_TIME_SPREAD;

typedef struct khe_assign_resource_constraint_rec
  *KHE_ASSIGN_RESOURCE_CONSTRAINT;
typedef struct khe_assign_time_constraint_rec
  *KHE_ASSIGN_TIME_CONSTRAINT;
typedef struct khe_split_events_constraint_rec
  *KHE_SPLIT_EVENTS_CONSTRAINT;
typedef struct khe_distribute_split_events_constraint_rec
  *KHE_DISTRIBUTE_SPLIT_EVENTS_CONSTRAINT;
typedef struct khe_prefer_resources_constraint_rec
  *KHE_PREFER_RESOURCES_CONSTRAINT;
typedef struct khe_prefer_times_constraint_rec
  *KHE_PREFER_TIMES_CONSTRAINT;
typedef struct khe_avoid_split_assignments_constraint_rec
  *KHE_AVOID_SPLIT_ASSIGNMENTS_CONSTRAINT;
typedef struct khe_spread_events_constraint_rec
  *KHE_SPREAD_EVENTS_CONSTRAINT;
typedef struct khe_link_events_constraint_rec
  *KHE_LINK_EVENTS_CONSTRAINT;
typedef struct khe_avoid_clashes_constraint_rec
  *KHE_AVOID_CLASHES_CONSTRAINT;
typedef struct khe_avoid_unavailable_times_constraint_rec
  *KHE_AVOID_UNAVAILABLE_TIMES_CONSTRAINT;
typedef struct khe_limit_idle_times_constraint_rec
  *KHE_LIMIT_IDLE_TIMES_CONSTRAINT;
typedef struct khe_cluster_busy_times_constraint_rec
  *KHE_CLUSTER_BUSY_TIMES_CONSTRAINT;
typedef struct khe_limit_busy_times_constraint_rec
  *KHE_LIMIT_BUSY_TIMES_CONSTRAINT;
typedef struct khe_limit_workload_constraint_rec
  *KHE_LIMIT_WORKLOAD_CONSTRAINT;


/*****************************************************************************/
/*                                                                           */
/*    Chapter 4.   Solutions                                                 */
/*                                                                           */
/*****************************************************************************/

typedef struct khe_soln_rec *KHE_SOLN;
typedef int64_t KHE_COST;
#define KheCostMax INT64_MAX
/* typedef struct khe_time_domain_rec *KHE_TIME_DOMAIN; */
typedef struct khe_meet_rec *KHE_MEET;
typedef struct khe_task_rec *KHE_TASK;
typedef struct khe_transaction_rec *KHE_TRANSACTION;

/* ***
typedef enum {
  KHE_TASK_STATE_CYCLE,
  KHE_TASK_STATE_LEADER,
  KHE_TASK_STATE_FOLLOWER
} KHE_TASK_STATE;
*** */


/*****************************************************************************/
/*                                                                           */
/*    Chapter 5.   Extra Types for Solving                                   */
/*                                                                           */
/*****************************************************************************/

typedef struct khe_node_rec *KHE_NODE;
typedef struct khe_layer_rec *KHE_LAYER;
typedef struct khe_zone_rec *KHE_ZONE;
typedef struct khe_tasking_rec *KHE_TASKING;


/*****************************************************************************/
/*                                                                           */
/*    Chapter 6.   Solution Monitoring                                       */
/*                                                                           */
/*****************************************************************************/

typedef enum {
  KHE_ASSIGN_RESOURCE_MONITOR_TAG,
  KHE_ASSIGN_TIME_MONITOR_TAG,
  KHE_SPLIT_EVENTS_MONITOR_TAG,
  KHE_DISTRIBUTE_SPLIT_EVENTS_MONITOR_TAG,
  KHE_PREFER_RESOURCES_MONITOR_TAG,
  KHE_PREFER_TIMES_MONITOR_TAG,
  KHE_AVOID_SPLIT_ASSIGNMENTS_MONITOR_TAG,
  KHE_SPREAD_EVENTS_MONITOR_TAG,
  KHE_LINK_EVENTS_MONITOR_TAG,
  KHE_AVOID_CLASHES_MONITOR_TAG,
  KHE_AVOID_UNAVAILABLE_TIMES_MONITOR_TAG,
  KHE_LIMIT_IDLE_TIMES_MONITOR_TAG,
  KHE_CLUSTER_BUSY_TIMES_MONITOR_TAG,
  KHE_LIMIT_BUSY_TIMES_MONITOR_TAG,
  KHE_LIMIT_WORKLOAD_MONITOR_TAG,
  KHE_TIMETABLE_MONITOR_TAG,
  KHE_TIME_GROUP_MONITOR_TAG,
  KHE_ORDINARY_DEMAND_MONITOR_TAG,
  KHE_WORKLOAD_DEMAND_MONITOR_TAG,
  KHE_EVENNESS_MONITOR_TAG,
  KHE_GROUP_MONITOR_TAG,
  KHE_MONITOR_TAG_COUNT
} KHE_MONITOR_TAG;

typedef struct khe_monitor_rec *KHE_MONITOR;

typedef struct khe_assign_resource_monitor_rec
  *KHE_ASSIGN_RESOURCE_MONITOR;
typedef struct khe_assign_time_monitor_rec
  *KHE_ASSIGN_TIME_MONITOR;
typedef struct khe_split_events_monitor_rec
  *KHE_SPLIT_EVENTS_MONITOR;
typedef struct khe_distribute_split_events_monitor_rec
  *KHE_DISTRIBUTE_SPLIT_EVENTS_MONITOR;
typedef struct khe_prefer_resources_monitor_rec
  *KHE_PREFER_RESOURCES_MONITOR;
typedef struct khe_prefer_times_monitor_rec
  *KHE_PREFER_TIMES_MONITOR;
typedef struct khe_avoid_split_assignments_monitor_rec
  *KHE_AVOID_SPLIT_ASSIGNMENTS_MONITOR;
typedef struct khe_spread_events_monitor_rec
  *KHE_SPREAD_EVENTS_MONITOR;
typedef struct khe_link_events_monitor_rec
  *KHE_LINK_EVENTS_MONITOR;
typedef struct khe_avoid_clashes_monitor_rec
  *KHE_AVOID_CLASHES_MONITOR;
typedef struct khe_avoid_unavailable_times_monitor_rec
  *KHE_AVOID_UNAVAILABLE_TIMES_MONITOR;
typedef struct khe_limit_idle_times_monitor_rec
  *KHE_LIMIT_IDLE_TIMES_MONITOR;
typedef struct khe_cluster_busy_times_monitor_rec
  *KHE_CLUSTER_BUSY_TIMES_MONITOR;
typedef struct khe_limit_busy_times_monitor_rec
  *KHE_LIMIT_BUSY_TIMES_MONITOR;
typedef struct khe_limit_workload_monitor_rec
  *KHE_LIMIT_WORKLOAD_MONITOR;
typedef struct khe_timetable_monitor_rec
  *KHE_TIMETABLE_MONITOR;
typedef struct khe_time_group_monitor_rec
  *KHE_TIME_GROUP_MONITOR;
typedef struct khe_group_monitor_rec
  *KHE_GROUP_MONITOR;


/*****************************************************************************/
/*                                                                           */
/*    Chapter 7.   Matchings and Evenness                                    */
/*                                                                           */
/*****************************************************************************/

typedef struct khe_ordinary_demand_monitor_rec
  *KHE_ORDINARY_DEMAND_MONITOR;

typedef struct khe_workload_demand_monitor_rec
  *KHE_WORKLOAD_DEMAND_MONITOR;

typedef enum {
  KHE_MATCHING_TYPE_EVAL_INITIAL,
  KHE_MATCHING_TYPE_EVAL_TIMES,
  KHE_MATCHING_TYPE_EVAL_RESOURCES,
  KHE_MATCHING_TYPE_SOLVE
} KHE_MATCHING_TYPE;

typedef struct khe_evenness_monitor_rec *KHE_EVENNESS_MONITOR;


/*****************************************************************************/
/*                                                                           */
/*    Chapter 8.   Monitor Grouping and Tracing                              */
/*                                                                           */
/*****************************************************************************/

typedef enum {
  KHE_SUBTAG_EVENT,
  KHE_SUBTAG_EVENT_RESOURCE,
  KHE_SUBTAG_RESOURCE,
  KHE_SUBTAG_DEMAND,
  KHE_SUBTAG_PREASSIGNED_DEMAND,
  KHE_SUBTAG_UNPREASSIGNED_DEMAND,
  KHE_SUBTAG_WORKLOAD_DEMAND,
  KHE_SUBTAG_EVENNESS,
  KHE_SUBTAG_NODE,
  KHE_SUBTAG_LAYER,
  KHE_SUBTAG_TASKING
} KHE_SUBTAG_STANDARD_TYPE;

typedef struct khe_trace_rec *KHE_TRACE;


/*****************************************************************************/
/*                                                                           */
/*                    Part B:  Some Solvers                                  */
/*                                                                           */
/*****************************************************************************/

/*****************************************************************************/
/*                                                                           */
/*    Chapter 9.   Layer Tree Solvers                                        */
/*                                                                           */
/*****************************************************************************/

typedef struct khe_layer_match_rec *KHE_LAYER_MATCH;

/*****************************************************************************/
/*                                                                           */
/*    Chapter 10.   Time Solvers                                             */
/*                                                                           */
/*****************************************************************************/

typedef bool (*KHE_NODE_TIME_SOLVER)(KHE_NODE parent_node);
typedef bool (*KHE_LAYER_TIME_SOLVER)(KHE_LAYER layer);


/*****************************************************************************/
/*                                                                           */
/*    Chapter 11.  Resource Solvers                                          */
/*                                                                           */
/*****************************************************************************/

/* *** old stuff
typedef void (*KHE_RESOURCE_SOLVER)(KHE_SOLN soln, KHE_RESOURCE_TYPE rt);

typedef enum {
  KHE_DOMAIN_HARD,
  KHE_DOMAIN_HARD_AND_SOFT,
  KHE_DOMAIN_ANY,
  KHE_DOMAIN_PARTITION
} KHE_DOMAIN_TYPE;
*** */

typedef void (*KHE_TASKING_SOLVER)(KHE_TASKING tasking);

typedef enum {
  KHE_TASK_JOB_HARD_PRC = 1,
  KHE_TASK_JOB_SOFT_PRC = 2,
  KHE_TASK_JOB_HARD_ASAC = 4,
  KHE_TASK_JOB_SOFT_ASAC = 8,
  KHE_TASK_JOB_PARTITION = 16
} KHE_TASK_JOB_TYPE;

typedef struct khe_task_group_rec *KHE_TASK_GROUP;
typedef struct khe_task_groups_rec *KHE_TASK_GROUPS;


/*****************************************************************************/
/*                                                                           */
/*    Chapter 12.  General solvers                                           */
/*                                                                           */
/*****************************************************************************/

typedef KHE_SOLN (*KHE_GENERAL_SOLVER)(KHE_SOLN soln);


/*****************************************************************************/
/*                                                                           */
/*    Chapter 13.  Ejection Chains                                           */
/*                                                                           */
/*****************************************************************************/

typedef enum {
  KHE_EJECTOR_NONE,
  KHE_EJECTOR_FIRST_SUCCESS,
  KHE_EJECTOR_MIN_COST,
  KHE_EJECTOR_MIN_DISRUPTION_THEN_COST
} KHE_EJECTOR_SOLVE_TYPE;

typedef struct khe_ejector_schedule_rec *KHE_EJECTOR_SCHEDULE;
typedef struct khe_ejector_rec *KHE_EJECTOR;

typedef bool (*KHE_EJECTOR_AUGMENT_FN)(KHE_EJECTOR ej, KHE_MONITOR d);


/*****************************************************************************/
/*                                                                           */
/*                     FUNCTION DECLARATIONS                                 */
/*                                                                           */
/*****************************************************************************/

/*****************************************************************************/
/*                                                                           */
/*                    Part A: The Platform                                   */
/*                                                                           */
/*****************************************************************************/

/*****************************************************************************/
/*                                                                           */
/*    Chapter 1.   Introduction                                              */
/*                                                                           */
/*****************************************************************************/

/*****************************************************************************/
/*                                                                           */
/*    Chapter 2.   Archives and Solution Groups                              */
/*                                                                           */
/*****************************************************************************/

/* 2.1 archives */
extern KHE_ARCHIVE KheArchiveMake(char *id, KHE_ARCHIVE_METADATA md);
extern char *KheArchiveId(KHE_ARCHIVE archive);
extern KHE_ARCHIVE_METADATA KheArchiveMetaData(KHE_ARCHIVE archive);

extern void KheArchiveSetBack(KHE_ARCHIVE archive, void *back);
extern void *KheArchiveBack(KHE_ARCHIVE archive);

extern KHE_ARCHIVE_METADATA KheArchiveMetaDataMake(char *name,
  char *contributor, char *date, char *description, char *remarks);
extern char *KheArchiveMetaDataName(KHE_ARCHIVE_METADATA md);
extern char *KheArchiveMetaDataContributor(KHE_ARCHIVE_METADATA md);
extern char *KheArchiveMetaDataDate(KHE_ARCHIVE_METADATA md);
extern char *KheArchiveMetaDataDescription(KHE_ARCHIVE_METADATA md);
extern char *KheArchiveMetaDataRemarks(KHE_ARCHIVE_METADATA md);

extern int KheArchiveInstanceCount(KHE_ARCHIVE archive);
extern KHE_INSTANCE KheArchiveInstance(KHE_ARCHIVE archive, int i);
extern bool KheArchiveRetrieveInstance(KHE_ARCHIVE archive, char *id,
  KHE_INSTANCE *ins);

extern int KheArchiveSolnGroupCount(KHE_ARCHIVE archive);
extern KHE_SOLN_GROUP KheArchiveSolnGroup(KHE_ARCHIVE archive, int i);
extern bool KheArchiveRetrieveSolnGroup(KHE_ARCHIVE archive, char *id,
  KHE_SOLN_GROUP *soln_group);

/* 2.2 solution groups */
extern bool KheSolnGroupMake(KHE_ARCHIVE archive, char *id,
  KHE_SOLN_GROUP_METADATA md, KHE_SOLN_GROUP *soln_group);
extern void KheSolnGroupSetBack(KHE_SOLN_GROUP soln_group, void *back);
extern void *KheSolnGroupBack(KHE_SOLN_GROUP soln_group);

extern KHE_ARCHIVE KheSolnGroupArchive(KHE_SOLN_GROUP soln_group);
extern char *KheSolnGroupId(KHE_SOLN_GROUP soln_group);
extern KHE_SOLN_GROUP_METADATA KheSolnGroupMetaData(KHE_SOLN_GROUP soln_group);

extern KHE_SOLN_GROUP_METADATA KheSolnGroupMetaDataMake(char *contributor,
  char *date, char *description, char *remarks);
extern char *KheSolnGroupMetaDataContributor(KHE_SOLN_GROUP_METADATA md);
extern char *KheSolnGroupMetaDataDate(KHE_SOLN_GROUP_METADATA md);
extern char *KheSolnGroupMetaDataDescription(KHE_SOLN_GROUP_METADATA md);
extern char *KheSolnGroupMetaDataRemarks(KHE_SOLN_GROUP_METADATA md);

extern void KheSolnGroupAddSoln(KHE_SOLN_GROUP soln_group, KHE_SOLN soln);
extern void KheSolnGroupDeleteSoln(KHE_SOLN_GROUP soln_group, KHE_SOLN soln);

extern int KheSolnGroupSolnCount(KHE_SOLN_GROUP soln_group);
extern KHE_SOLN KheSolnGroupSoln(KHE_SOLN_GROUP soln_group, int i);

/* 2.3 reading and writing archives */
extern bool KheArchiveRead(FILE *fp, KHE_ARCHIVE *archive,
  bool infer_resource_partitions, KML_ERROR *ke);
extern bool KheArchiveReadFromString(char *str, KHE_ARCHIVE *archive,
  bool infer_resource_partitions, KML_ERROR *ke);
extern bool KheArchiveWrite(KHE_ARCHIVE archive, bool with_reports, FILE *fp);


/*****************************************************************************/
/*                                                                           */
/*    Chapter 3.   Instances                                                 */
/*                                                                           */
/*****************************************************************************/

/* 3.1 Instances - general */
extern bool KheInstanceMakeBegin(KHE_ARCHIVE archive, char *id,
  KHE_INSTANCE_METADATA md, KHE_INSTANCE *ins);
extern void KheInstanceSetBack(KHE_INSTANCE ins, void *back);
extern void *KheInstanceBack(KHE_INSTANCE ins);

extern KHE_ARCHIVE KheInstanceArchive(KHE_INSTANCE ins);
extern char *KheInstanceId(KHE_INSTANCE ins);
extern char *KheInstanceName(KHE_INSTANCE ins);
extern KHE_INSTANCE_METADATA KheInstanceMetaData(KHE_INSTANCE ins);
extern void KheInstanceMakeEnd(KHE_INSTANCE ins,
  bool infer_resource_partitions);

/* 3.1 Instances - metadata */
extern KHE_INSTANCE_METADATA KheInstanceMetaDataMake(char *name,
  char *contributor, char *date, char *country, char *description,
  char *remarks);
extern char *KheInstanceMetaDataName(KHE_INSTANCE_METADATA md);
extern char *KheInstanceMetaDataContributor(KHE_INSTANCE_METADATA md);
extern char *KheInstanceMetaDataDate(KHE_INSTANCE_METADATA md);
extern char *KheInstanceMetaDataCountry(KHE_INSTANCE_METADATA md);
extern char *KheInstanceMetaDataDescription(KHE_INSTANCE_METADATA md);
extern char *KheInstanceMetaDataRemarks(KHE_INSTANCE_METADATA md);

/* 3.1 Instances - time groups */
extern int KheInstanceTimeGroupCount(KHE_INSTANCE ins);
extern KHE_TIME_GROUP KheInstanceTimeGroup(KHE_INSTANCE ins, int i);
extern bool KheInstanceRetrieveTimeGroup(KHE_INSTANCE ins, char *id,
  KHE_TIME_GROUP *tg);

extern KHE_TIME_GROUP KheInstanceFullTimeGroup(KHE_INSTANCE ins);
extern KHE_TIME_GROUP KheInstanceEmptyTimeGroup(KHE_INSTANCE ins);
/* ***
extern KHE_TIME_GROUP KheInstancePackingTimeGroup(KHE_INSTANCE ins,
  int duration);
*** */

/* 3.1 Instances - times */
extern int KheInstanceTimeCount(KHE_INSTANCE ins);
extern KHE_TIME KheInstanceTime(KHE_INSTANCE ins, int i);
extern bool KheInstanceRetrieveTime(KHE_INSTANCE ins, char *id,
  KHE_TIME *t);

/* 3.1 Instances - resource types */
extern int KheInstanceResourceTypeCount(KHE_INSTANCE ins);
extern KHE_RESOURCE_TYPE KheInstanceResourceType(KHE_INSTANCE ins, int i);
extern bool KheInstanceRetrieveResourceType(KHE_INSTANCE ins, char *id,
  KHE_RESOURCE_TYPE *rt);

/* 3.1 Instances - resource groups and resources */
extern bool KheInstanceRetrieveResourceGroup(KHE_INSTANCE ins, char *id,
  KHE_RESOURCE_GROUP *rg);
extern bool KheInstanceRetrieveResource(KHE_INSTANCE ins, char *id,
  KHE_RESOURCE *r);
extern int KheInstanceResourceCount(KHE_INSTANCE ins);
extern KHE_RESOURCE KheInstanceResource(KHE_INSTANCE ins, int i);

/* 3.1 Instances - event groups */
extern int KheInstanceEventGroupCount(KHE_INSTANCE ins);
extern KHE_EVENT_GROUP KheInstanceEventGroup(KHE_INSTANCE ins, int i);
extern bool KheInstanceRetrieveEventGroup(KHE_INSTANCE ins, char *id,
  KHE_EVENT_GROUP *eg);
extern KHE_EVENT_GROUP KheInstanceFullEventGroup(KHE_INSTANCE ins);
extern KHE_EVENT_GROUP KheInstanceEmptyEventGroup(KHE_INSTANCE ins);

/* 3.1 Instances - events */
extern int KheInstanceEventCount(KHE_INSTANCE ins);
extern KHE_EVENT KheInstanceEvent(KHE_INSTANCE ins, int i);
extern bool KheInstanceRetrieveEvent(KHE_INSTANCE ins, char *id,
  KHE_EVENT *e);

/* 3.1 Instances - event resources */
extern int KheInstanceEventResourceCount(KHE_INSTANCE ins);
extern KHE_EVENT_RESOURCE KheInstanceEventResource(KHE_INSTANCE ins, int i);

/* 3.1 Instances - constraints */
extern int KheInstanceConstraintCount(KHE_INSTANCE ins);
extern KHE_CONSTRAINT KheInstanceConstraint(KHE_INSTANCE ins, int i);
extern bool KheInstanceRetrieveConstraint(KHE_INSTANCE ins, char *id,
  KHE_CONSTRAINT *c);

/* 3.2 Times - time groups */
extern bool KheTimeGroupMake(KHE_INSTANCE ins, KHE_TIME_GROUP_KIND kind,
  char *id, char *name, KHE_TIME_GROUP *tg);
extern void KheTimeGroupSetBack(KHE_TIME_GROUP tg, void *back);
extern void *KheTimeGroupBack(KHE_TIME_GROUP tg);

extern KHE_INSTANCE KheTimeGroupInstance(KHE_TIME_GROUP tg);
extern KHE_TIME_GROUP_KIND KheTimeGroupKind(KHE_TIME_GROUP tg);
extern char *KheTimeGroupId(KHE_TIME_GROUP tg);
extern char *KheTimeGroupName(KHE_TIME_GROUP tg);

extern void KheTimeGroupAddTime(KHE_TIME_GROUP tg, KHE_TIME t);
extern void KheTimeGroupSubTime(KHE_TIME_GROUP tg, KHE_TIME t);
extern void KheTimeGroupUnion(KHE_TIME_GROUP tg, KHE_TIME_GROUP tg2);
extern void KheTimeGroupIntersect(KHE_TIME_GROUP tg, KHE_TIME_GROUP tg2);
extern void KheTimeGroupDifference(KHE_TIME_GROUP tg, KHE_TIME_GROUP tg2);

extern int KheTimeGroupTimeCount(KHE_TIME_GROUP tg);
extern KHE_TIME KheTimeGroupTime(KHE_TIME_GROUP tg, int i);

extern bool KheTimeGroupContains(KHE_TIME_GROUP tg, KHE_TIME t);
extern bool KheTimeGroupEqual(KHE_TIME_GROUP tg1, KHE_TIME_GROUP tg2);
extern bool KheTimeGroupSubset(KHE_TIME_GROUP tg1, KHE_TIME_GROUP tg2);
extern bool KheTimeGroupDisjoint(KHE_TIME_GROUP tg1, KHE_TIME_GROUP tg2);

extern KHE_TIME_GROUP KheTimeGroupNeighbour(KHE_TIME_GROUP tg, int delta);

extern void KheTimeGroupDebug(KHE_TIME_GROUP tg, int verbosity,
  int indent, FILE *fp);

/* 3.2 Times - times */
extern bool KheTimeMake(KHE_INSTANCE ins, char *id, char *name,
  bool break_after, KHE_TIME *t);
extern void KheTimeSetBack(KHE_TIME t, void *back);
extern void *KheTimeBack(KHE_TIME t);

extern KHE_INSTANCE KheTimeInstance(KHE_TIME t);
extern char *KheTimeId(KHE_TIME t);
extern char *KheTimeName(KHE_TIME t);
extern bool KheTimeBreakAfter(KHE_TIME t);
extern int KheTimeIndex(KHE_TIME t);
extern bool KheTimeHasNeighbour(KHE_TIME t, int delta);
extern KHE_TIME KheTimeNeighbour(KHE_TIME t, int delta);

/* ***
extern int KheTimeCycleMeetIndex(KHE_TIME t);
extern int KheTimeCycleMeetOffset(KHE_TIME t);
*** */

extern KHE_TIME_GROUP KheTimeSingletonTimeGroup(KHE_TIME t);

/* 3.3 Resources - resource types */
extern bool KheResourceTypeMake(KHE_INSTANCE ins, char *id, char *name,
  bool has_partitions, KHE_RESOURCE_TYPE *rt);
extern void KheResourceTypeSetBack(KHE_RESOURCE_TYPE rt, void *back);
extern void *KheResourceTypeBack(KHE_RESOURCE_TYPE rt);

extern KHE_INSTANCE KheResourceTypeInstance(KHE_RESOURCE_TYPE rt);
extern int KheResourceTypeIndex(KHE_RESOURCE_TYPE rt);
extern char *KheResourceTypeId(KHE_RESOURCE_TYPE rt);
extern char *KheResourceTypeName(KHE_RESOURCE_TYPE rt);
extern bool KheResourceTypeHasPartitions(KHE_RESOURCE_TYPE rt);

extern int KheResourceTypeResourceGroupCount(KHE_RESOURCE_TYPE rt);
extern KHE_RESOURCE_GROUP KheResourceTypeResourceGroup(KHE_RESOURCE_TYPE rt,
  int i);
extern bool KheResourceTypeRetrieveResourceGroup(KHE_RESOURCE_TYPE rt,
  char *id, KHE_RESOURCE_GROUP *rg);

extern int KheResourceTypePartitionCount(KHE_RESOURCE_TYPE rt);
extern KHE_RESOURCE_GROUP KheResourceTypePartition(KHE_RESOURCE_TYPE rt, int i);

extern KHE_RESOURCE_GROUP KheResourceTypeFullResourceGroup(
  KHE_RESOURCE_TYPE rt);
extern KHE_RESOURCE_GROUP KheResourceTypeEmptyResourceGroup(
  KHE_RESOURCE_TYPE rt);

extern int KheResourceTypeResourceCount(KHE_RESOURCE_TYPE rt);
extern KHE_RESOURCE KheResourceTypeResource(KHE_RESOURCE_TYPE rt, int i);
extern bool KheResourceTypeRetrieveResource(KHE_RESOURCE_TYPE rt,
  char *id, KHE_RESOURCE *r);

extern bool KheResourceTypeDemandIsAllPreassigned(KHE_RESOURCE_TYPE rt);
extern int KheResourceTypeAvoidSplitAssignmentsCount(KHE_RESOURCE_TYPE rt);

/* 3.3 Resources - resource groups */
extern bool KheResourceGroupMake(KHE_RESOURCE_TYPE rt, char *id, char *name,
  bool is_partition, KHE_RESOURCE_GROUP *rg);
extern void KheResourceGroupSetBack(KHE_RESOURCE_GROUP rg, void *back);
extern void *KheResourceGroupBack(KHE_RESOURCE_GROUP rg);

extern KHE_RESOURCE_TYPE KheResourceGroupResourceType(KHE_RESOURCE_GROUP rg);
extern KHE_INSTANCE KheResourceGroupInstance(KHE_RESOURCE_GROUP rg);
extern char *KheResourceGroupId(KHE_RESOURCE_GROUP rg);
extern char *KheResourceGroupName(KHE_RESOURCE_GROUP rg);
extern bool KheResourceGroupIsPartition(KHE_RESOURCE_GROUP rg);

extern void KheResourceGroupAddResource(KHE_RESOURCE_GROUP rg, KHE_RESOURCE r);
extern void KheResourceGroupSubResource(KHE_RESOURCE_GROUP rg, KHE_RESOURCE r);
extern void KheResourceGroupUnion(KHE_RESOURCE_GROUP rg,
  KHE_RESOURCE_GROUP rg2);
extern void KheResourceGroupIntersect(KHE_RESOURCE_GROUP rg,
  KHE_RESOURCE_GROUP rg2);
extern void KheResourceGroupDifference(KHE_RESOURCE_GROUP rg,
  KHE_RESOURCE_GROUP rg2);

extern int KheResourceGroupResourceCount(KHE_RESOURCE_GROUP rg);
extern KHE_RESOURCE KheResourceGroupResource(KHE_RESOURCE_GROUP rg, int i);

extern bool KheResourceGroupContains(KHE_RESOURCE_GROUP rg,
  KHE_RESOURCE r);
extern bool KheResourceGroupEqual(KHE_RESOURCE_GROUP rg1,
  KHE_RESOURCE_GROUP rg2);
extern bool KheResourceGroupSubset(KHE_RESOURCE_GROUP rg1,
  KHE_RESOURCE_GROUP rg2);
extern bool KheResourceGroupDisjoint(KHE_RESOURCE_GROUP rg1,
  KHE_RESOURCE_GROUP rg2);

extern KHE_RESOURCE_GROUP KheResourceGroupPartition(KHE_RESOURCE_GROUP rg);

extern void KheResourceGroupDebug(KHE_RESOURCE_GROUP rg, int verbosity,
  int indent, FILE *fp);

/* 3.3 Resources - resources */
extern bool KheResourceMake(KHE_RESOURCE_TYPE rt, char *id, char *name,
  KHE_RESOURCE_GROUP partition, KHE_RESOURCE *r);
extern void KheResourceSetBack(KHE_RESOURCE r, void *back);
extern void *KheResourceBack(KHE_RESOURCE r);

extern KHE_RESOURCE_TYPE KheResourceResourceType(KHE_RESOURCE r);
extern KHE_INSTANCE KheResourceInstance(KHE_RESOURCE r);
extern char *KheResourceId(KHE_RESOURCE r);
extern char *KheResourceName(KHE_RESOURCE r);
extern KHE_RESOURCE_GROUP KheResourcePartition(KHE_RESOURCE r);
extern int KheResourceIndexInInstance(KHE_RESOURCE r);
extern int KheResourceIndexInResourceType(KHE_RESOURCE r);

extern KHE_RESOURCE_GROUP KheResourceSingletonResourceGroup(KHE_RESOURCE r);

extern int KheResourcePreassignedEventResourceCount(KHE_RESOURCE r);
extern KHE_EVENT_RESOURCE KheResourcePreassignedEventResource(KHE_RESOURCE r,
  int i);

extern void KheResourceDebug(KHE_RESOURCE r, int verbosity,
  int indent, FILE *fp);

extern int KheResourceLayerEventCount(KHE_RESOURCE r);
extern KHE_EVENT KheResourceLayerEvent(KHE_RESOURCE r, int i);
extern int KheResourceLayerDuration(KHE_RESOURCE r);

extern int KheResourceConstraintCount(KHE_RESOURCE r);
extern KHE_CONSTRAINT KheResourceConstraint(KHE_RESOURCE r, int i);

extern KHE_TIME_GROUP KheResourceHardUnavailableTimeGroup(KHE_RESOURCE r);
extern KHE_TIME_GROUP KheResourceHardAndSoftUnavailableTimeGroup(
  KHE_RESOURCE r);

extern bool KheResourceSimilar(KHE_RESOURCE r1, KHE_RESOURCE r2);

/* 3.4 Events - event groups */
extern bool KheEventGroupMake(KHE_INSTANCE ins, KHE_EVENT_GROUP_KIND kind,
  char *id, char *name, KHE_EVENT_GROUP *eg);
extern void KheEventGroupSetBack(KHE_EVENT_GROUP eg, void *back);
extern void *KheEventGroupBack(KHE_EVENT_GROUP eg);

extern KHE_INSTANCE KheEventGroupInstance(KHE_EVENT_GROUP eg);
extern KHE_EVENT_GROUP_KIND KheEventGroupKind(KHE_EVENT_GROUP eg);
extern char *KheEventGroupId(KHE_EVENT_GROUP eg);
extern char *KheEventGroupName(KHE_EVENT_GROUP eg);

extern void KheEventGroupAddEvent(KHE_EVENT_GROUP eg, KHE_EVENT e);
extern void KheEventGroupSubEvent(KHE_EVENT_GROUP eg, KHE_EVENT e);
extern void KheEventGroupUnion(KHE_EVENT_GROUP eg, KHE_EVENT_GROUP eg2);
extern void KheEventGroupIntersect(KHE_EVENT_GROUP eg, KHE_EVENT_GROUP eg2);
extern void KheEventGroupDifference(KHE_EVENT_GROUP eg, KHE_EVENT_GROUP eg2);

extern int KheEventGroupEventCount(KHE_EVENT_GROUP eg);
extern KHE_EVENT KheEventGroupEvent(KHE_EVENT_GROUP eg, int i);

extern bool KheEventGroupContains(KHE_EVENT_GROUP eg, KHE_EVENT e);
extern bool KheEventGroupEqual(KHE_EVENT_GROUP eg1, KHE_EVENT_GROUP eg2);
extern bool KheEventGroupSubset(KHE_EVENT_GROUP eg1, KHE_EVENT_GROUP eg2);
extern bool KheEventGroupDisjoint(KHE_EVENT_GROUP eg1, KHE_EVENT_GROUP eg2);

extern int KheEventGroupConstraintCount(KHE_EVENT_GROUP eg);
extern KHE_CONSTRAINT KheEventGroupConstraint(KHE_EVENT_GROUP eg, int i);

/* 3.4 Events - events */
extern bool KheEventMake(KHE_INSTANCE ins, char *id, char *name, char *color,
  int duration, int workload, KHE_TIME preassigned_time, KHE_EVENT *e);
extern void KheEventSetBack(KHE_EVENT e, void *back);
extern void *KheEventBack(KHE_EVENT e);

extern KHE_INSTANCE KheEventInstance(KHE_EVENT e);
extern char *KheEventId(KHE_EVENT e);
extern char *KheEventName(KHE_EVENT e);
extern char *KheEventColor(KHE_EVENT e);
extern int KheEventDuration(KHE_EVENT e);
extern int KheEventWorkload(KHE_EVENT e);
extern KHE_TIME KheEventPreassignedTime(KHE_EVENT e);

extern int KheEventIndex(KHE_EVENT e);
extern int KheEventDemand(KHE_EVENT e);

extern int KheEventResourceCount(KHE_EVENT e);
extern KHE_EVENT_RESOURCE KheEventResource(KHE_EVENT e, int i);
extern bool KheEventRetrieveEventResource(KHE_EVENT e, char *role,
  KHE_EVENT_RESOURCE *er);

extern int KheEventResourceGroupCount(KHE_EVENT e);
extern KHE_EVENT_RESOURCE_GROUP KheEventResourceGroup(KHE_EVENT e, int i);

extern KHE_EVENT_GROUP KheEventSingletonEventGroup(KHE_EVENT e);

extern int KheEventConstraintCount(KHE_EVENT e);
extern KHE_CONSTRAINT KheEventConstraint(KHE_EVENT e, int i);

extern bool KheEventSimilar(KHE_EVENT e1, KHE_EVENT e2);

extern void KheEventDebug(KHE_EVENT e, int verbosity, int indent, FILE *fp);

/* 3.4 Events - event resources */
extern bool KheEventResourceMake(KHE_EVENT event, KHE_RESOURCE_TYPE rt,
  KHE_RESOURCE preassigned_resource, char *role, int workload,
  KHE_EVENT_RESOURCE *er);
extern void KheEventResourceSetBack(KHE_EVENT_RESOURCE er, void *back);
extern void *KheEventResourceBack(KHE_EVENT_RESOURCE er);

extern KHE_EVENT KheEventResourceEvent(KHE_EVENT_RESOURCE er);
extern KHE_RESOURCE_TYPE KheEventResourceResourceType(KHE_EVENT_RESOURCE er);
extern KHE_RESOURCE KheEventResourcePreassignedResource(KHE_EVENT_RESOURCE er);
extern int KheEventResourceHasRole(KHE_EVENT_RESOURCE er);
extern char *KheEventResourceRole(KHE_EVENT_RESOURCE er);
extern int KheEventResourceWorkload(KHE_EVENT_RESOURCE er);
extern int KheEventResourceIndexInEvent(KHE_EVENT_RESOURCE er);
extern int KheEventResourceIndexInInstance(KHE_EVENT_RESOURCE er);

extern int KheEventResourceConstraintCount(KHE_EVENT_RESOURCE er);
extern KHE_CONSTRAINT KheEventResourceConstraint(KHE_EVENT_RESOURCE er, int i);
extern int KheEventResourceConstraintEventGroupIndex(KHE_EVENT_RESOURCE er,
  int i);

extern KHE_RESOURCE_GROUP KheEventResourceHardDomain(KHE_EVENT_RESOURCE er);
extern KHE_RESOURCE_GROUP KheEventResourceHardAndSoftDomain(
  KHE_EVENT_RESOURCE er);

extern void KheEventResourceDebug(KHE_EVENT_RESOURCE er, int verbosity,
  int indent, FILE *fp);

/* 3.4 Events - event resource groups */
extern KHE_EVENT_RESOURCE_GROUP KheEventResourceGroupMake(KHE_EVENT event,
  KHE_RESOURCE_GROUP rg);

extern KHE_EVENT KheEventResourceGroupEvent(KHE_EVENT_RESOURCE_GROUP erg);
extern KHE_RESOURCE_GROUP KheEventResourceGroupResourceGroup(
  KHE_EVENT_RESOURCE_GROUP erg);

extern KHE_EVENT_RESOURCE_GROUP KheEventResourceEventResourceGroup(
  KHE_EVENT_RESOURCE er);

extern void KheEventResourceGroupDebug(KHE_EVENT_RESOURCE_GROUP erg,
  int verbosity, int indent, FILE *fp);

/* 3.5 Constraints */
extern void KheConstraintSetBack(KHE_CONSTRAINT c, void *back);
extern void *KheConstraintBack(KHE_CONSTRAINT c);

extern KHE_INSTANCE KheConstraintInstance(KHE_CONSTRAINT c);
extern char *KheConstraintId(KHE_CONSTRAINT c);
extern char *KheConstraintName(KHE_CONSTRAINT c);
extern bool KheConstraintRequired(KHE_CONSTRAINT c);
extern int KheConstraintWeight(KHE_CONSTRAINT c);
extern KHE_COST KheConstraintCombinedWeight(KHE_CONSTRAINT c);
extern KHE_COST_FUNCTION KheConstraintCostFunction(KHE_CONSTRAINT c);
extern int KheConstraintIndex(KHE_CONSTRAINT c);
extern KHE_CONSTRAINT_TAG KheConstraintTag(KHE_CONSTRAINT c);

extern int KheConstraintAppliesToCount(KHE_CONSTRAINT c);

extern char *KheConstraintTagShow(KHE_CONSTRAINT_TAG tag);
extern char *KheConstraintTagShowSpaced(KHE_CONSTRAINT_TAG tag);
extern KHE_CONSTRAINT_TAG KheStringToConstraintTag(char *str);
extern char *KheCostFunctionShow(KHE_COST_FUNCTION cf);

extern void KheConstraintDebug(KHE_CONSTRAINT c, int verbosity,
  int indent, FILE *fp);

extern KHE_CONSTRAINT KheFromAssignResourceConstraint(
  KHE_ASSIGN_RESOURCE_CONSTRAINT c);
extern KHE_CONSTRAINT KheFromAssignTimeConstraint(
  KHE_ASSIGN_TIME_CONSTRAINT c);
extern KHE_CONSTRAINT KheFromSplitEventsConstraint(
  KHE_SPLIT_EVENTS_CONSTRAINT c);
extern KHE_CONSTRAINT KheFromDistributeSplitEventsConstraint(
  KHE_DISTRIBUTE_SPLIT_EVENTS_CONSTRAINT c);
extern KHE_CONSTRAINT KheFromPreferResourcesConstraint(
  KHE_PREFER_RESOURCES_CONSTRAINT c);
extern KHE_CONSTRAINT KheFromPreferTimesConstraint(
  KHE_PREFER_TIMES_CONSTRAINT c);
extern KHE_CONSTRAINT KheFromAvoidSplitAssignmentsConstraint(
  KHE_AVOID_SPLIT_ASSIGNMENTS_CONSTRAINT c);
extern KHE_CONSTRAINT KheFromSpreadEventsConstraint(
  KHE_SPREAD_EVENTS_CONSTRAINT c);
extern KHE_CONSTRAINT KheFromLinkEventsConstraint(
  KHE_LINK_EVENTS_CONSTRAINT c);
extern KHE_CONSTRAINT KheFromAvoidClashesConstraint(
  KHE_AVOID_CLASHES_CONSTRAINT c);
extern KHE_CONSTRAINT KheFromAvoidUnavailableTimesConstraint(
  KHE_AVOID_UNAVAILABLE_TIMES_CONSTRAINT c);
extern KHE_CONSTRAINT KheFromLimitIdleTimesConstraint(
  KHE_LIMIT_IDLE_TIMES_CONSTRAINT c);
extern KHE_CONSTRAINT KheFromClusterBusyTimesConstraint(
  KHE_CLUSTER_BUSY_TIMES_CONSTRAINT c);
extern KHE_CONSTRAINT KheFromLimitBusyTimesConstraint(
  KHE_LIMIT_BUSY_TIMES_CONSTRAINT c);
extern KHE_CONSTRAINT KheFromLimitWorkloadConstraint(
  KHE_LIMIT_WORKLOAD_CONSTRAINT c);

extern KHE_ASSIGN_RESOURCE_CONSTRAINT
  KheToAssignResourceConstraint(KHE_CONSTRAINT c);
extern KHE_ASSIGN_TIME_CONSTRAINT
  KheToAssignTimeConstraint(KHE_CONSTRAINT c);
extern KHE_SPLIT_EVENTS_CONSTRAINT
  KheToSplitEventsConstraint(KHE_CONSTRAINT c);
extern KHE_DISTRIBUTE_SPLIT_EVENTS_CONSTRAINT
  KheToDistributeSplitEventsConstraint(KHE_CONSTRAINT c);
extern KHE_PREFER_RESOURCES_CONSTRAINT
  KheToPreferResourcesConstraint(KHE_CONSTRAINT c);
extern KHE_PREFER_TIMES_CONSTRAINT
  KheToPreferTimesConstraint(KHE_CONSTRAINT c);
extern KHE_AVOID_SPLIT_ASSIGNMENTS_CONSTRAINT
  KheToAvoidSplitAssignmentsConstraint(KHE_CONSTRAINT c);
extern KHE_SPREAD_EVENTS_CONSTRAINT
  KheToSpreadEventsConstraint(KHE_CONSTRAINT c);
extern KHE_LINK_EVENTS_CONSTRAINT
  KheToLinkEventsConstraint(KHE_CONSTRAINT c);
extern KHE_AVOID_CLASHES_CONSTRAINT
  KheToAvoidClashesConstraint(KHE_CONSTRAINT c);
extern KHE_AVOID_UNAVAILABLE_TIMES_CONSTRAINT
  KheToAvoidUnavailableTimesConstraint(KHE_CONSTRAINT c);
extern KHE_LIMIT_IDLE_TIMES_CONSTRAINT
  KheToLimitIdleTimesConstraint(KHE_CONSTRAINT c);
extern KHE_CLUSTER_BUSY_TIMES_CONSTRAINT
  KheToClusterBusyTimesConstraint(KHE_CONSTRAINT c);
extern KHE_LIMIT_BUSY_TIMES_CONSTRAINT
  KheToLimitBusyTimesConstraint(KHE_CONSTRAINT c);
extern KHE_LIMIT_WORKLOAD_CONSTRAINT
  KheToLimitWorkloadConstraint(KHE_CONSTRAINT c);

/* 3.5.1 Assign resource constraints */
extern bool KheAssignResourceConstraintMake(KHE_INSTANCE ins, char *id,
  char *name, bool required, int weight, KHE_COST_FUNCTION cf,
  char *role, KHE_ASSIGN_RESOURCE_CONSTRAINT *c);
extern char *KheAssignResourceConstraintRole(KHE_ASSIGN_RESOURCE_CONSTRAINT c);

extern void KheAssignResourceConstraintAddEventResource(
  KHE_ASSIGN_RESOURCE_CONSTRAINT c, KHE_EVENT_RESOURCE er);
extern int KheAssignResourceConstraintEventResourceCount(
  KHE_ASSIGN_RESOURCE_CONSTRAINT c);
extern KHE_EVENT_RESOURCE KheAssignResourceConstraintEventResource(
  KHE_ASSIGN_RESOURCE_CONSTRAINT c, int i);

extern void KheAssignResourceConstraintAddEvent(
  KHE_ASSIGN_RESOURCE_CONSTRAINT c, KHE_EVENT e);
extern int KheAssignResourceConstraintEventCount(
  KHE_ASSIGN_RESOURCE_CONSTRAINT c);
extern KHE_EVENT KheAssignResourceConstraintEvent(
  KHE_ASSIGN_RESOURCE_CONSTRAINT c, int i);

extern void KheAssignResourceConstraintAddEventGroup(
  KHE_ASSIGN_RESOURCE_CONSTRAINT c, KHE_EVENT_GROUP eg);
extern int KheAssignResourceConstraintEventGroupCount(
  KHE_ASSIGN_RESOURCE_CONSTRAINT c);
extern KHE_EVENT_GROUP KheAssignResourceConstraintEventGroup(
  KHE_ASSIGN_RESOURCE_CONSTRAINT c, int i);

/* 3.5.2 Assign time constraints */
extern bool KheAssignTimeConstraintMake(KHE_INSTANCE ins, char *id,
  char *name, bool required, int weight, KHE_COST_FUNCTION cf,
  KHE_ASSIGN_TIME_CONSTRAINT *c);

extern void KheAssignTimeConstraintAddEvent(KHE_ASSIGN_TIME_CONSTRAINT c,
  KHE_EVENT e);
extern int KheAssignTimeConstraintEventCount(KHE_ASSIGN_TIME_CONSTRAINT c);
extern KHE_EVENT KheAssignTimeConstraintEvent(KHE_ASSIGN_TIME_CONSTRAINT c,
  int i);

extern void KheAssignTimeConstraintAddEventGroup(KHE_ASSIGN_TIME_CONSTRAINT c,
  KHE_EVENT_GROUP eg);
extern int KheAssignTimeConstraintEventGroupCount(KHE_ASSIGN_TIME_CONSTRAINT c);
extern KHE_EVENT_GROUP KheAssignTimeConstraintEventGroup(
  KHE_ASSIGN_TIME_CONSTRAINT c, int i);

/* 3.5.3 Split events constraints */
extern bool KheSplitEventsConstraintMake(KHE_INSTANCE ins, char *id,
  char *name, bool required, int weight, KHE_COST_FUNCTION cf,
  int min_duration, int max_duration, int min_amount, int max_amount,
  KHE_SPLIT_EVENTS_CONSTRAINT *c);

extern int KheSplitEventsConstraintMinDuration(KHE_SPLIT_EVENTS_CONSTRAINT c);
extern int KheSplitEventsConstraintMaxDuration(KHE_SPLIT_EVENTS_CONSTRAINT c);
extern int KheSplitEventsConstraintMinAmount(KHE_SPLIT_EVENTS_CONSTRAINT c);
extern int KheSplitEventsConstraintMaxAmount(KHE_SPLIT_EVENTS_CONSTRAINT c);

extern void KheSplitEventsConstraintAddEvent(KHE_SPLIT_EVENTS_CONSTRAINT c,
  KHE_EVENT e);
extern int KheSplitEventsConstraintEventCount(KHE_SPLIT_EVENTS_CONSTRAINT c);
extern KHE_EVENT KheSplitEventsConstraintEvent(KHE_SPLIT_EVENTS_CONSTRAINT c,
  int i);

extern void KheSplitEventsConstraintAddEventGroup(
  KHE_SPLIT_EVENTS_CONSTRAINT c, KHE_EVENT_GROUP eg);
extern int KheSplitEventsConstraintEventGroupCount(
  KHE_SPLIT_EVENTS_CONSTRAINT c);
extern KHE_EVENT_GROUP KheSplitEventsConstraintEventGroup(
  KHE_SPLIT_EVENTS_CONSTRAINT c, int i);

/* 3.5.4 Distribute split events constraints */
extern bool KheDistributeSplitEventsConstraintMake(KHE_INSTANCE ins, char *id,
  char *name, bool required, int weight, KHE_COST_FUNCTION cf,
  int duration, int minimum, int maximum,
  KHE_DISTRIBUTE_SPLIT_EVENTS_CONSTRAINT *c);

extern int KheDistributeSplitEventsConstraintDuration(
  KHE_DISTRIBUTE_SPLIT_EVENTS_CONSTRAINT c);
extern int KheDistributeSplitEventsConstraintMinimum(
  KHE_DISTRIBUTE_SPLIT_EVENTS_CONSTRAINT c);
extern int KheDistributeSplitEventsConstraintMaximum(
  KHE_DISTRIBUTE_SPLIT_EVENTS_CONSTRAINT c);

extern void KheDistributeSplitEventsConstraintAddEvent(
  KHE_DISTRIBUTE_SPLIT_EVENTS_CONSTRAINT c, KHE_EVENT e);
extern int KheDistributeSplitEventsConstraintEventCount(
  KHE_DISTRIBUTE_SPLIT_EVENTS_CONSTRAINT c);
extern KHE_EVENT KheDistributeSplitEventsConstraintEvent(
  KHE_DISTRIBUTE_SPLIT_EVENTS_CONSTRAINT c, int i);

extern void KheDistributeSplitEventsConstraintAddEventGroup(
  KHE_DISTRIBUTE_SPLIT_EVENTS_CONSTRAINT c, KHE_EVENT_GROUP eg);
extern int KheDistributeSplitEventsConstraintEventGroupCount(
  KHE_DISTRIBUTE_SPLIT_EVENTS_CONSTRAINT c);
extern KHE_EVENT_GROUP KheDistributeSplitEventsConstraintEventGroup(
  KHE_DISTRIBUTE_SPLIT_EVENTS_CONSTRAINT c, int i);

/* 3.5.5 Prefer resources constraints */
extern bool KhePreferResourcesConstraintMake(KHE_INSTANCE ins, char *id,
  char *name, bool required, int weight, KHE_COST_FUNCTION cf,
  char *role, KHE_PREFER_RESOURCES_CONSTRAINT *c);
extern char *KhePreferResourcesConstraintRole(
  KHE_PREFER_RESOURCES_CONSTRAINT c);

extern bool KhePreferResourcesConstraintAddResourceGroup(
  KHE_PREFER_RESOURCES_CONSTRAINT c, KHE_RESOURCE_GROUP rg);
extern int KhePreferResourcesConstraintResourceGroupCount(
  KHE_PREFER_RESOURCES_CONSTRAINT c);
extern KHE_RESOURCE_GROUP KhePreferResourcesConstraintResourceGroup(
  KHE_PREFER_RESOURCES_CONSTRAINT c, int i);

extern bool KhePreferResourcesConstraintAddResource(
  KHE_PREFER_RESOURCES_CONSTRAINT c, KHE_RESOURCE r);
extern int KhePreferResourcesConstraintResourceCount(
  KHE_PREFER_RESOURCES_CONSTRAINT c);
extern KHE_RESOURCE KhePreferResourcesConstraintResource(
  KHE_PREFER_RESOURCES_CONSTRAINT c, int i);

extern KHE_RESOURCE_GROUP KhePreferResourcesConstraintDomain(
  KHE_PREFER_RESOURCES_CONSTRAINT c);

extern bool KhePreferResourcesConstraintAddEventResource(
  KHE_PREFER_RESOURCES_CONSTRAINT c, KHE_EVENT_RESOURCE er);
extern int KhePreferResourcesConstraintEventResourceCount(
  KHE_PREFER_RESOURCES_CONSTRAINT c);
extern KHE_EVENT_RESOURCE KhePreferResourcesConstraintEventResource(
  KHE_PREFER_RESOURCES_CONSTRAINT c, int i);

extern bool KhePreferResourcesConstraintAddEvent(
  KHE_PREFER_RESOURCES_CONSTRAINT c, KHE_EVENT e);
extern int KhePreferResourcesConstraintEventCount(
  KHE_PREFER_RESOURCES_CONSTRAINT c);
extern KHE_EVENT KhePreferResourcesConstraintEvent(
  KHE_PREFER_RESOURCES_CONSTRAINT c, int i);

extern bool KhePreferResourcesConstraintAddEventGroup(
  KHE_PREFER_RESOURCES_CONSTRAINT c, KHE_EVENT_GROUP eg,
  KHE_EVENT *problem_event);
extern int KhePreferResourcesConstraintEventGroupCount(
  KHE_PREFER_RESOURCES_CONSTRAINT c);
extern KHE_EVENT_GROUP KhePreferResourcesConstraintEventGroup(
  KHE_PREFER_RESOURCES_CONSTRAINT c, int i);

/* 3.5.6 Prefer times constraints */
extern bool KhePreferTimesConstraintMake(KHE_INSTANCE ins, char *id,
  char *name, bool required, int weight, KHE_COST_FUNCTION cf,
  int duration, KHE_PREFER_TIMES_CONSTRAINT *c);
extern int KhePreferTimesConstraintDuration(KHE_PREFER_TIMES_CONSTRAINT c);

extern void KhePreferTimesConstraintAddTimeGroup(
  KHE_PREFER_TIMES_CONSTRAINT c, KHE_TIME_GROUP tg);
extern int KhePreferTimesConstraintTimeGroupCount(
  KHE_PREFER_TIMES_CONSTRAINT c);
extern KHE_TIME_GROUP KhePreferTimesConstraintTimeGroup(
  KHE_PREFER_TIMES_CONSTRAINT c, int i);

extern void KhePreferTimesConstraintAddTime(
  KHE_PREFER_TIMES_CONSTRAINT c, KHE_TIME t);
extern int KhePreferTimesConstraintTimeCount(
  KHE_PREFER_TIMES_CONSTRAINT c);
extern KHE_TIME KhePreferTimesConstraintTime(
  KHE_PREFER_TIMES_CONSTRAINT c, int i);

extern KHE_TIME_GROUP KhePreferTimesConstraintDomain(
  KHE_PREFER_TIMES_CONSTRAINT c);

extern void KhePreferTimesConstraintAddEvent(
  KHE_PREFER_TIMES_CONSTRAINT c, KHE_EVENT e);
extern int KhePreferTimesConstraintEventCount(
  KHE_PREFER_TIMES_CONSTRAINT c);
extern KHE_EVENT KhePreferTimesConstraintEvent(
  KHE_PREFER_TIMES_CONSTRAINT c, int i);

extern void KhePreferTimesConstraintAddEventGroup(
  KHE_PREFER_TIMES_CONSTRAINT c, KHE_EVENT_GROUP eg);
extern int KhePreferTimesConstraintEventGroupCount(
  KHE_PREFER_TIMES_CONSTRAINT c);
extern KHE_EVENT_GROUP KhePreferTimesConstraintEventGroup(
  KHE_PREFER_TIMES_CONSTRAINT c, int i);

/* 3.5.7 Avoid split assignments constraints */
extern bool KheAvoidSplitAssignmentsConstraintMake(KHE_INSTANCE ins, char *id,
  char *name, bool required, int weight, KHE_COST_FUNCTION cf,
  char *role, KHE_AVOID_SPLIT_ASSIGNMENTS_CONSTRAINT *c);
extern char *KheAvoidSplitAssignmentsConstraintRole(
  KHE_AVOID_SPLIT_ASSIGNMENTS_CONSTRAINT c);

extern bool KheAvoidSplitAssignmentsConstraintAddEventGroup(
  KHE_AVOID_SPLIT_ASSIGNMENTS_CONSTRAINT c, KHE_EVENT_GROUP eg,
  KHE_EVENT *problem_event);
extern int KheAvoidSplitAssignmentsConstraintEventGroupCount(
  KHE_AVOID_SPLIT_ASSIGNMENTS_CONSTRAINT c);
extern KHE_EVENT_GROUP KheAvoidSplitAssignmentsConstraintEventGroup(
  KHE_AVOID_SPLIT_ASSIGNMENTS_CONSTRAINT c, int i);

extern void KheAvoidSplitAssignmentsConstraintAddEventResource(
  KHE_AVOID_SPLIT_ASSIGNMENTS_CONSTRAINT c, int eg_index,
  KHE_EVENT_RESOURCE er);
extern int KheAvoidSplitAssignmentsConstraintEventResourceCount(
  KHE_AVOID_SPLIT_ASSIGNMENTS_CONSTRAINT c, int eg_index);
extern KHE_EVENT_RESOURCE KheAvoidSplitAssignmentsConstraintEventResource(
  KHE_AVOID_SPLIT_ASSIGNMENTS_CONSTRAINT c, int eg_index, int er_index);

/* 3.5.8 Spread events constraints */
extern bool KheSpreadEventsConstraintMake(KHE_INSTANCE ins, char *id,
  char *name, bool required, int weight, KHE_COST_FUNCTION cf,
  KHE_TIME_SPREAD ts, KHE_SPREAD_EVENTS_CONSTRAINT *c);
extern KHE_TIME_SPREAD KheSpreadEventsConstraintTimeSpread(
  KHE_SPREAD_EVENTS_CONSTRAINT c);

extern KHE_TIME_SPREAD KheTimeSpreadMake(KHE_INSTANCE ins);
extern void KheTimeSpreadAddLimitedTimeGroup(KHE_TIME_SPREAD ts,
  KHE_LIMITED_TIME_GROUP ltg);
extern int KheTimeSpreadLimitedTimeGroupCount(KHE_TIME_SPREAD ts);
extern KHE_LIMITED_TIME_GROUP KheTimeSpreadLimitedTimeGroup(KHE_TIME_SPREAD ts,
  int i);

extern KHE_LIMITED_TIME_GROUP KheLimitedTimeGroupMake(KHE_TIME_GROUP tg,
  int minimum, int maximum);
extern KHE_TIME_GROUP KheLimitedTimeGroupTimeGroup(KHE_LIMITED_TIME_GROUP ltg);
extern int KheLimitedTimeGroupMinimum(KHE_LIMITED_TIME_GROUP ltg);
extern int KheLimitedTimeGroupMaximum(KHE_LIMITED_TIME_GROUP ltg);

extern bool KheTimeSpreadTimeGroupsDisjoint(KHE_TIME_SPREAD ts);
extern bool KheTimeSpreadCoversWholeCycle(KHE_TIME_SPREAD ts);

extern void KheSpreadEventsConstraintAddEventGroup(
  KHE_SPREAD_EVENTS_CONSTRAINT c, KHE_EVENT_GROUP eg);
extern int KheSpreadEventsConstraintEventGroupCount(
  KHE_SPREAD_EVENTS_CONSTRAINT c);
extern KHE_EVENT_GROUP KheSpreadEventsConstraintEventGroup(
  KHE_SPREAD_EVENTS_CONSTRAINT c, int i);

/* 3.5.9 Link events constraints */
extern bool KheLinkEventsConstraintMake(KHE_INSTANCE ins, char *id,
  char *name, bool required, int weight, KHE_COST_FUNCTION cf,
  KHE_LINK_EVENTS_CONSTRAINT *c);

extern void KheLinkEventsConstraintAddEventGroup(KHE_LINK_EVENTS_CONSTRAINT c,
  KHE_EVENT_GROUP eg);
extern int KheLinkEventsConstraintEventGroupCount(KHE_LINK_EVENTS_CONSTRAINT c);
extern KHE_EVENT_GROUP KheLinkEventsConstraintEventGroup(
  KHE_LINK_EVENTS_CONSTRAINT c, int i);

/* 3.5.10 Avoid clashes constraints */
extern bool KheAvoidClashesConstraintMake(KHE_INSTANCE ins, char *id,
  char *name, bool required, int weight, KHE_COST_FUNCTION cf,
  KHE_AVOID_CLASHES_CONSTRAINT *c);

extern void KheAvoidClashesConstraintAddResourceGroup(
  KHE_AVOID_CLASHES_CONSTRAINT c, KHE_RESOURCE_GROUP rg);
extern int KheAvoidClashesConstraintResourceGroupCount(
  KHE_AVOID_CLASHES_CONSTRAINT c);
extern KHE_RESOURCE_GROUP KheAvoidClashesConstraintResourceGroup(
  KHE_AVOID_CLASHES_CONSTRAINT c, int i);

extern void KheAvoidClashesConstraintAddResource(
  KHE_AVOID_CLASHES_CONSTRAINT c, KHE_RESOURCE r);
extern int KheAvoidClashesConstraintResourceCount(
  KHE_AVOID_CLASHES_CONSTRAINT c);
extern KHE_RESOURCE KheAvoidClashesConstraintResource(
  KHE_AVOID_CLASHES_CONSTRAINT c, int i);

/* 3.5.11 Avoid unavailable times constraints */
extern bool KheAvoidUnavailableTimesConstraintMake(KHE_INSTANCE ins, char *id,
  char *name, bool required, int weight, KHE_COST_FUNCTION cf,
  KHE_AVOID_UNAVAILABLE_TIMES_CONSTRAINT *c);

extern void KheAvoidUnavailableTimesConstraintAddResourceGroup(
  KHE_AVOID_UNAVAILABLE_TIMES_CONSTRAINT c, KHE_RESOURCE_GROUP rg);
extern int KheAvoidUnavailableTimesConstraintResourceGroupCount(
  KHE_AVOID_UNAVAILABLE_TIMES_CONSTRAINT c);
extern KHE_RESOURCE_GROUP KheAvoidUnavailableTimesConstraintResourceGroup(
  KHE_AVOID_UNAVAILABLE_TIMES_CONSTRAINT c, int i);

extern void KheAvoidUnavailableTimesConstraintAddResource(
  KHE_AVOID_UNAVAILABLE_TIMES_CONSTRAINT c, KHE_RESOURCE r);
extern int KheAvoidUnavailableTimesConstraintResourceCount(
  KHE_AVOID_UNAVAILABLE_TIMES_CONSTRAINT c);
extern KHE_RESOURCE KheAvoidUnavailableTimesConstraintResource(
  KHE_AVOID_UNAVAILABLE_TIMES_CONSTRAINT c, int i);

extern void KheAvoidUnavailableTimesConstraintAddTimeGroup(
  KHE_AVOID_UNAVAILABLE_TIMES_CONSTRAINT c, KHE_TIME_GROUP tg);
extern int KheAvoidUnavailableTimesConstraintTimeGroupCount(
  KHE_AVOID_UNAVAILABLE_TIMES_CONSTRAINT c);
extern KHE_TIME_GROUP KheAvoidUnavailableTimesConstraintTimeGroup(
  KHE_AVOID_UNAVAILABLE_TIMES_CONSTRAINT c, int i);

extern void KheAvoidUnavailableTimesConstraintAddTime(
  KHE_AVOID_UNAVAILABLE_TIMES_CONSTRAINT c, KHE_TIME t);
extern int KheAvoidUnavailableTimesConstraintTimeCount(
  KHE_AVOID_UNAVAILABLE_TIMES_CONSTRAINT c);
extern KHE_TIME KheAvoidUnavailableTimesConstraintTime(
  KHE_AVOID_UNAVAILABLE_TIMES_CONSTRAINT c, int i);

extern KHE_TIME_GROUP KheAvoidUnavailableTimesConstraintDomain(
  KHE_AVOID_UNAVAILABLE_TIMES_CONSTRAINT c);

/* 3.5.12 Limit idle times constraints */
extern bool KheLimitIdleTimesConstraintMake(KHE_INSTANCE ins, char *id,
  char *name, bool required, int weight, KHE_COST_FUNCTION cf,
  int minimum, int maximum, KHE_LIMIT_IDLE_TIMES_CONSTRAINT *c);
extern int KheLimitIdleTimesConstraintMinimum(
  KHE_LIMIT_IDLE_TIMES_CONSTRAINT c);
extern int KheLimitIdleTimesConstraintMaximum(
  KHE_LIMIT_IDLE_TIMES_CONSTRAINT c);

extern void KheLimitIdleTimesConstraintAddTimeGroup(
  KHE_LIMIT_IDLE_TIMES_CONSTRAINT c, KHE_TIME_GROUP tg);
extern int KheLimitIdleTimesConstraintTimeGroupCount(
  KHE_LIMIT_IDLE_TIMES_CONSTRAINT c);
extern KHE_TIME_GROUP KheLimitIdleTimesConstraintTimeGroup(
  KHE_LIMIT_IDLE_TIMES_CONSTRAINT c, int i);

extern void KheLimitIdleTimesConstraintAddResourceGroup(
  KHE_LIMIT_IDLE_TIMES_CONSTRAINT c, KHE_RESOURCE_GROUP rg);
extern int KheLimitIdleTimesConstraintResourceGroupCount(
  KHE_LIMIT_IDLE_TIMES_CONSTRAINT c);
extern KHE_RESOURCE_GROUP KheLimitIdleTimesConstraintResourceGroup(
  KHE_LIMIT_IDLE_TIMES_CONSTRAINT c, int i);

extern void KheLimitIdleTimesConstraintAddResource(
  KHE_LIMIT_IDLE_TIMES_CONSTRAINT c, KHE_RESOURCE r);
extern int KheLimitIdleTimesConstraintResourceCount(
  KHE_LIMIT_IDLE_TIMES_CONSTRAINT c);
extern KHE_RESOURCE KheLimitIdleTimesConstraintResource(
  KHE_LIMIT_IDLE_TIMES_CONSTRAINT c, int i);

/* 3.5.13 Cluster busy times constraints */
extern bool KheClusterBusyTimesConstraintMake(KHE_INSTANCE ins, char *id,
  char *name, bool required, int weight, KHE_COST_FUNCTION cf,
  int minimum, int maximum, KHE_CLUSTER_BUSY_TIMES_CONSTRAINT *c);
extern int KheClusterBusyTimesConstraintMinimum(
  KHE_CLUSTER_BUSY_TIMES_CONSTRAINT c);
extern int KheClusterBusyTimesConstraintMaximum(
  KHE_CLUSTER_BUSY_TIMES_CONSTRAINT c);

extern void KheClusterBusyTimesConstraintAddTimeGroup(
  KHE_CLUSTER_BUSY_TIMES_CONSTRAINT c, KHE_TIME_GROUP tg);
extern int KheClusterBusyTimesConstraintTimeGroupCount(
  KHE_CLUSTER_BUSY_TIMES_CONSTRAINT c);
extern KHE_TIME_GROUP KheClusterBusyTimesConstraintTimeGroup(
  KHE_CLUSTER_BUSY_TIMES_CONSTRAINT c, int i);

extern void KheClusterBusyTimesConstraintAddResourceGroup(
  KHE_CLUSTER_BUSY_TIMES_CONSTRAINT c, KHE_RESOURCE_GROUP rg);
extern int KheClusterBusyTimesConstraintResourceGroupCount(
  KHE_CLUSTER_BUSY_TIMES_CONSTRAINT c);
extern KHE_RESOURCE_GROUP KheClusterBusyTimesConstraintResourceGroup(
  KHE_CLUSTER_BUSY_TIMES_CONSTRAINT c, int i);

extern void KheClusterBusyTimesConstraintAddResource(
  KHE_CLUSTER_BUSY_TIMES_CONSTRAINT c, KHE_RESOURCE r);
extern int KheClusterBusyTimesConstraintResourceCount(
  KHE_CLUSTER_BUSY_TIMES_CONSTRAINT c);
extern KHE_RESOURCE KheClusterBusyTimesConstraintResource(
  KHE_CLUSTER_BUSY_TIMES_CONSTRAINT c, int i);

/* 3.5.14 Limit busy times constraints */
extern bool KheLimitBusyTimesConstraintMake(KHE_INSTANCE ins, char *id,
  char *name, bool required, int weight, KHE_COST_FUNCTION cf,
  int minimum, int maximum, KHE_LIMIT_BUSY_TIMES_CONSTRAINT *c);
extern int KheLimitBusyTimesConstraintMinimum(
  KHE_LIMIT_BUSY_TIMES_CONSTRAINT c);
extern int KheLimitBusyTimesConstraintMaximum(
  KHE_LIMIT_BUSY_TIMES_CONSTRAINT c);

extern void KheLimitBusyTimesConstraintAddTimeGroup(
  KHE_LIMIT_BUSY_TIMES_CONSTRAINT c, KHE_TIME_GROUP tg);
extern int KheLimitBusyTimesConstraintTimeGroupCount(
  KHE_LIMIT_BUSY_TIMES_CONSTRAINT c);
extern KHE_TIME_GROUP KheLimitBusyTimesConstraintTimeGroup(
  KHE_LIMIT_BUSY_TIMES_CONSTRAINT c, int i);

extern void KheLimitBusyTimesConstraintAddResourceGroup(
  KHE_LIMIT_BUSY_TIMES_CONSTRAINT c, KHE_RESOURCE_GROUP rg);
extern int KheLimitBusyTimesConstraintResourceGroupCount(
  KHE_LIMIT_BUSY_TIMES_CONSTRAINT c);
extern KHE_RESOURCE_GROUP KheLimitBusyTimesConstraintResourceGroup(
  KHE_LIMIT_BUSY_TIMES_CONSTRAINT c, int i);

extern void KheLimitBusyTimesConstraintAddResource(
  KHE_LIMIT_BUSY_TIMES_CONSTRAINT c, KHE_RESOURCE r);
extern int KheLimitBusyTimesConstraintResourceCount(
  KHE_LIMIT_BUSY_TIMES_CONSTRAINT c);
extern KHE_RESOURCE KheLimitBusyTimesConstraintResource(
  KHE_LIMIT_BUSY_TIMES_CONSTRAINT c, int i);

/* 3.5.15 Limit workload constraints */
extern bool KheLimitWorkloadConstraintMake(KHE_INSTANCE ins, char *id,
  char *name, bool required, int weight, KHE_COST_FUNCTION cf,
  int minimum, int maximum, KHE_LIMIT_WORKLOAD_CONSTRAINT *c);
extern int KheLimitWorkloadConstraintMinimum(KHE_LIMIT_WORKLOAD_CONSTRAINT c);
extern int KheLimitWorkloadConstraintMaximum(KHE_LIMIT_WORKLOAD_CONSTRAINT c);

/* resource groups */
extern void KheLimitWorkloadConstraintAddResourceGroup(
  KHE_LIMIT_WORKLOAD_CONSTRAINT c, KHE_RESOURCE_GROUP rg);
extern int KheLimitWorkloadConstraintResourceGroupCount(
  KHE_LIMIT_WORKLOAD_CONSTRAINT c);
extern KHE_RESOURCE_GROUP KheLimitWorkloadConstraintResourceGroup(
  KHE_LIMIT_WORKLOAD_CONSTRAINT c, int i);

/* resources */
extern void KheLimitWorkloadConstraintAddResource(
  KHE_LIMIT_WORKLOAD_CONSTRAINT c, KHE_RESOURCE r);
extern int KheLimitWorkloadConstraintResourceCount(
  KHE_LIMIT_WORKLOAD_CONSTRAINT c);
extern KHE_RESOURCE KheLimitWorkloadConstraintResource(
  KHE_LIMIT_WORKLOAD_CONSTRAINT c, int i);


/*****************************************************************************/
/*                                                                           */
/*    Chapter 4.   Solutions                                                 */
/*                                                                           */
/*****************************************************************************/

/* 4.2 Solution objects */
extern KHE_SOLN KheSolnMake(KHE_INSTANCE ins, KHE_SOLN_GROUP soln_group);
/* extern void KheSolnConsistencyCheck(KHE_SOLN soln); */
extern void KheSolnDelete(KHE_SOLN soln);

extern void KheSolnSetBack(KHE_SOLN soln, void *back);
extern void *KheSolnBack(KHE_SOLN soln);

extern KHE_INSTANCE KheSolnInstance(KHE_SOLN soln);
extern KHE_SOLN_GROUP KheSolnSolnGroup(KHE_SOLN soln);
extern void *KheSolnImpl(KHE_SOLN soln);

extern KHE_SOLN KheSolnCopy(KHE_SOLN soln);

/* monitors */
extern int KheSolnMonitorCount(KHE_SOLN soln);
extern KHE_MONITOR KheSolnMonitor(KHE_SOLN soln, int i);

/* meets */
extern int KheSolnMeetCount(KHE_SOLN soln);
extern KHE_MEET KheSolnMeet(KHE_SOLN soln, int i);
extern int KheEventMeetCount(KHE_SOLN soln, KHE_EVENT e);
extern KHE_MEET KheEventMeet(KHE_SOLN soln, KHE_EVENT e, int i);

/* tasks */
extern int KheSolnTaskCount(KHE_SOLN soln);
extern KHE_TASK KheSolnTask(KHE_SOLN soln, int i);
extern int KheEventResourceTaskCount(KHE_SOLN soln, KHE_EVENT_RESOURCE er);
extern KHE_TASK KheEventResourceTask(KHE_SOLN soln, KHE_EVENT_RESOURCE er,
  int i);

/* nodes */
extern int KheSolnNodeCount(KHE_SOLN soln);
extern KHE_NODE KheSolnNode(KHE_SOLN soln, int i);

/* taskings */
extern int KheSolnTaskingCount(KHE_SOLN soln);
extern KHE_TASKING KheSolnTasking(KHE_SOLN soln, int i);

/* ***
extern int KheLayerCount(KHE_SOLN soln);
extern KHE_LAYER KheLayer(KHE_SOLN soln, int i);
*** */

extern void KheSolnDebug(KHE_SOLN soln, int verbosity, int indent, FILE *fp);

/* 4.3 Complete representation and preassignment conversion */
extern bool KheSolnMakeCompleteRepresentation(KHE_SOLN soln,
  KHE_EVENT *problem_event);
extern void KheSolnAssignPreassignedTimes(KHE_SOLN soln, bool as_in_event);
extern void KheSolnAssignPreassignedResources(KHE_SOLN soln,
  KHE_RESOURCE_TYPE rt, bool as_in_event_resource);

/* 4.4 Solution time, resource, and event groups */
extern void KheSolnTimeGroupBegin(KHE_SOLN soln);
extern void KheSolnTimeGroupAddTime(KHE_SOLN soln, KHE_TIME t);
extern void KheSolnTimeGroupSubTime(KHE_SOLN soln, KHE_TIME t);
extern void KheSolnTimeGroupUnion(KHE_SOLN soln, KHE_TIME_GROUP tg2);
extern void KheSolnTimeGroupIntersect(KHE_SOLN soln, KHE_TIME_GROUP tg2);
extern void KheSolnTimeGroupDifference(KHE_SOLN soln, KHE_TIME_GROUP tg2);
extern KHE_TIME_GROUP KheSolnTimeGroupEnd(KHE_SOLN soln);

extern void KheSolnResourceGroupBegin(KHE_SOLN soln, KHE_RESOURCE_TYPE rt);
extern void KheSolnResourceGroupAddResource(KHE_SOLN soln, KHE_RESOURCE r);
extern void KheSolnResourceGroupSubResource(KHE_SOLN soln, KHE_RESOURCE r);
extern void KheSolnResourceGroupUnion(KHE_SOLN soln, KHE_RESOURCE_GROUP rg2);
extern void KheSolnResourceGroupIntersect(KHE_SOLN soln,
  KHE_RESOURCE_GROUP rg2);
extern void KheSolnResourceGroupDifference(KHE_SOLN soln,
  KHE_RESOURCE_GROUP rg2);
extern KHE_RESOURCE_GROUP KheSolnResourceGroupEnd(KHE_SOLN soln);

extern void KheSolnEventGroupBegin(KHE_SOLN soln);
extern void KheSolnEventGroupAddEvent(KHE_SOLN soln, KHE_EVENT e);
extern void KheSolnEventGroupSubEvent(KHE_SOLN soln, KHE_EVENT e);
extern void KheSolnEventGroupUnion(KHE_SOLN soln, KHE_EVENT_GROUP eg2);
extern void KheSolnEventGroupIntersect(KHE_SOLN soln, KHE_EVENT_GROUP eg2);
extern void KheSolnEventGroupDifference(KHE_SOLN soln, KHE_EVENT_GROUP eg2);
extern KHE_EVENT_GROUP KheSolnEventGroupEnd(KHE_SOLN soln);

/* 4.5 Diversification */
extern void KheSolnSetDiversifier(KHE_SOLN soln, int val);
extern int KheSolnDiversifier(KHE_SOLN soln);
extern int KheSolnDiversifierChoose(KHE_SOLN soln, int c);

/* 4.6 Visit numbers */
extern void KheSolnSetVisitNum(KHE_SOLN soln, int num);
extern int KheSolnVisitNum(KHE_SOLN soln);
extern void KheSolnNewVisit(KHE_SOLN soln);

/* 4.7 Meets */
extern KHE_MEET KheMeetMake(KHE_SOLN soln, int duration, KHE_EVENT e);
extern void KheMeetDelete(KHE_MEET meet);

extern void KheMeetSetBack(KHE_MEET meet, void *back);
extern void *KheMeetBack(KHE_MEET meet);

extern void KheMeetSetVisitNum(KHE_MEET meet, int num);
extern int KheMeetVisitNum(KHE_MEET meet);
extern bool KheMeetVisited(KHE_MEET meet, int slack);
extern void KheMeetVisit(KHE_MEET meet);
extern void KheMeetUnVisit(KHE_MEET meet);

extern KHE_SOLN KheMeetSoln(KHE_MEET meet);
extern int KheMeetDuration(KHE_MEET meet);
extern KHE_EVENT KheMeetEvent(KHE_MEET meet);
extern int KheMeetIndex(KHE_MEET meet);

extern int KheMeetAssignedDuration(KHE_MEET meet);
extern int KheMeetDemand(KHE_MEET meet);

extern int KheMeetTaskCount(KHE_MEET meet);
extern KHE_TASK KheMeetTask(KHE_MEET meet, int i);
extern bool KheMeetRetrieveTask(KHE_MEET meet, char *role, KHE_TASK *task);
extern bool KheMeetFindTask(KHE_MEET meet, KHE_EVENT_RESOURCE er,
  KHE_TASK *task);
extern bool KheMeetContainsResourcePreassignment(KHE_MEET meet, KHE_RESOURCE r,
  bool as_in_event_resource, KHE_TASK *task);
extern bool KheMeetContainsResourceAssignment(KHE_MEET meet,
  KHE_RESOURCE r, KHE_TASK *task);

extern KHE_NODE KheMeetNode(KHE_MEET meet);

extern void KheMeetDebug(KHE_MEET meet, int verbosity, int indent, FILE *fp);

/* 4.7.1 Meets - splitting and merging */
extern bool KheMeetSplitCheck(KHE_MEET meet, int duration1, bool recursive);
extern bool KheMeetSplit(KHE_MEET meet, int duration1, bool recursive,
  KHE_MEET *meet1, KHE_MEET *meet2);
extern bool KheMeetMergeCheck(KHE_MEET meet1, KHE_MEET meet2);
extern bool KheMeetMerge(KHE_MEET meet1, KHE_MEET meet2, KHE_MEET *meet);

/* 4.7.2 Meets - hierarchical timetabling and meet assignment */
extern bool KheMeetAssignCheck(KHE_MEET meet, KHE_MEET target_meet,
  int target_offset);
extern bool KheMeetAssign(KHE_MEET meet, KHE_MEET target_meet,
  int target_offset);
extern void KheMeetUnAssign(KHE_MEET meet);
extern KHE_MEET KheMeetAsst(KHE_MEET meet);
extern int KheMeetAsstOffset(KHE_MEET meet);
extern int KheMeetAssignedToCount(KHE_MEET target_meet);
extern KHE_MEET KheMeetAssignedTo(KHE_MEET target_meet, int i);
extern KHE_MEET KheMeetRoot(KHE_MEET meet, int *offset_in_root);
extern bool KheMeetOverlap(KHE_MEET meet1, KHE_MEET meet2);

/* 4.7.3 Meets - cycle meets and time assignment */
extern bool KheMeetIsCycleMeet(KHE_MEET meet);
extern KHE_MEET KheSolnTimeCycleMeet(KHE_SOLN soln, KHE_TIME t);
extern int KheSolnTimeCycleMeetOffset(KHE_SOLN soln, KHE_TIME t);
extern KHE_TIME_GROUP KheSolnPackingTimeGroup(KHE_SOLN soln, int duration);
extern void KheSolnSplitCycleMeet(KHE_SOLN soln);

extern bool KheMeetAssignTimeCheck(KHE_MEET meet, KHE_TIME t);
extern bool KheMeetAssignTime(KHE_MEET meet, KHE_TIME t);
extern void KheMeetUnAssignTime(KHE_MEET meet);
extern KHE_TIME KheMeetAsstTime(KHE_MEET meet);

extern KHE_MEET KheMeetLeader(KHE_MEET meet, int *offset_in_leader);

/* 4.7 Meets - moving and swapping */
extern bool KheMeetMoveCheck(KHE_MEET meet, KHE_MEET target_meet,
  int target_offset);
extern bool KheMeetMove(KHE_MEET meet, KHE_MEET target_meet, int target_offset);
extern bool KheMeetMoveTimeCheck(KHE_MEET meet, KHE_TIME t);
extern bool KheMeetMoveTime(KHE_MEET meet, KHE_TIME t);

extern bool KheMeetSwapCheck(KHE_MEET meet1, KHE_MEET meet2);
extern bool KheMeetSwap(KHE_MEET meet1, KHE_MEET meet2);

extern bool KheMeetBlockSwapCheck(KHE_MEET meet1, KHE_MEET meet2);
extern bool KheMeetBlockSwap(KHE_MEET meet1, KHE_MEET meet2);

/* 4.7 Meets - time preassignments */
extern bool KheMeetIsPreassigned(KHE_MEET meet, bool as_in_event, KHE_TIME *t);
extern bool KheMeetIsAssignedPreassigned(KHE_MEET meet, bool as_in_event,
  KHE_TIME *t);

/* 4.7 Meets - time domains */
extern KHE_TIME_GROUP KheMeetDomain(KHE_MEET meet);
extern bool KheMeetSetDomainCheck(KHE_MEET meet, KHE_TIME_GROUP tg);
extern bool KheMeetSetDomain(KHE_MEET meet, KHE_TIME_GROUP tg);
extern KHE_TIME_GROUP KheMeetDescendantsDomain(KHE_MEET meet);
/* ***
extern void KheSolnTimeDomainBegin(KHE_SOLN soln);
extern void KheSolnTimeDomainSetTimeGroup(KHE_SOLN soln, int duration,
  KHE_TIME_GROUP tg);
extern KHE_TIME_DOMAIN KheSolnTimeDomainEnd(KHE_SOLN soln);

extern KHE_INSTANCE KheTimeDomainInstance(KHE_TIME_DOMAIN td);
extern KHE_TIME_GROUP KheTimeDomainTimeGroup(KHE_TIME_DOMAIN td, int duration);

extern KHE_TIME_DOMAIN KheSolnPackingTimeDomain(KHE_SOLN soln);
extern KHE_TIME_DOMAIN KheSolnPreassignedTimeDomain(KHE_SOLN soln, KHE_TIME t);

extern bool KheMeetSetDomainCheck(KHE_MEET meet, KHE_TIME_DOMAIN td);
extern bool KheMeetSetDomain(KHE_MEET meet, KHE_TIME_DOMAIN td);
extern KHE_TIME_DOMAIN KheMeetDomain(KHE_MEET meet);
extern KHE_TIME_GROUP KheMeetCurrentDomain(KHE_MEET meet);

extern KHE_TIME_DOMAIN KheSolnTimeDomainTightened(KHE_SOLN soln,
  KHE_TIME_DOMAIN td, KHE_TIME_GROUP tg);
*** */

/* 4.8 Tasks */
extern KHE_TASK KheTaskMake(KHE_SOLN soln, KHE_RESOURCE_TYPE rt,
  KHE_MEET meet, KHE_EVENT_RESOURCE er);
extern void KheTaskDelete(KHE_TASK task);
extern void KheTaskSetBack(KHE_TASK task, void *back);
extern void *KheTaskBack(KHE_TASK task);

extern void KheTaskSetVisitNum(KHE_TASK task, int num);
extern int KheTaskVisitNum(KHE_TASK task);
extern bool KheTaskVisited(KHE_TASK task, int slack);
extern void KheTaskVisit(KHE_TASK task);
extern void KheTaskUnVisit(KHE_TASK task);

extern KHE_MEET KheTaskMeet(KHE_TASK task);
extern int KheTaskIndexInMeet(KHE_TASK task);
extern int KheTaskDuration(KHE_TASK task);
extern float KheTaskWorkload(KHE_TASK task);

extern KHE_SOLN KheTaskSoln(KHE_TASK task);
extern int KheTaskIndexInSoln(KHE_TASK task);
extern KHE_RESOURCE_TYPE KheTaskResourceType(KHE_TASK task);
extern KHE_EVENT_RESOURCE KheTaskEventResource(KHE_TASK task);

extern KHE_TASKING KheTaskTasking(KHE_TASK task);
/* extern int KheTasksInterchangeable(KHE_TASK task1, KHE_TASK task2); */

extern void KheTaskDebug(KHE_TASK task, int verbosity,
  int indent, FILE *fp);

/* 4.8 Tasks - assignment */
extern bool KheTaskAssignResourceCheck(KHE_TASK task, KHE_RESOURCE r);
extern bool KheTaskAssignResource(KHE_TASK task, KHE_RESOURCE r);
extern void KheTaskUnAssignResource(KHE_TASK task);
extern KHE_RESOURCE KheTaskAsstResource(KHE_TASK task);

extern bool KheTaskAssignCheck(KHE_TASK task, KHE_TASK target_task);
extern bool KheTaskAssign(KHE_TASK task, KHE_TASK target_task);
extern void KheTaskUnAssign(KHE_TASK task);

extern KHE_TASK KheTaskAsst(KHE_TASK task);

extern int KheTaskAssignedToCount(KHE_TASK target_task);
extern KHE_TASK KheTaskAssignedTo(KHE_TASK target_task, int i);

extern int KheTaskTotalDuration(KHE_TASK task);
extern float KheTaskTotalWorkload(KHE_TASK task);

extern int KheResourceAssignedTaskCount(KHE_SOLN soln, KHE_RESOURCE r);
extern KHE_TASK KheResourceAssignedTask(KHE_SOLN soln, KHE_RESOURCE r, int i);

/* extern KHE_TASK_STATE KheTaskState(KHE_TASK task); */
extern bool KheTaskIsCycle(KHE_TASK task);
extern bool KheTaskIsLeader(KHE_TASK task);
extern bool KheTaskIsFollower(KHE_TASK task);
extern KHE_TASK KheTaskRoot(KHE_TASK task);
extern KHE_TASK KheTaskLeader(KHE_TASK task);

/* 4.8 Tasks - moving and swapping */
extern bool KheTaskMoveCheck(KHE_TASK task, KHE_TASK target_task);
extern bool KheTaskMove(KHE_TASK task, KHE_TASK target_task);
extern bool KheTaskMoveResourceCheck(KHE_TASK task, KHE_RESOURCE r);
extern bool KheTaskMoveResource(KHE_TASK task, KHE_RESOURCE r);
extern bool KheTaskSwapCheck(KHE_TASK task1, KHE_TASK task2);
extern bool KheTaskSwap(KHE_TASK task1, KHE_TASK task2);

/* 4.8 Tasks - resource preassignment and resource domains */
extern bool KheTaskPreassignCheck(KHE_TASK task, KHE_RESOURCE r);
extern bool KheTaskPreassign(KHE_TASK task, KHE_RESOURCE r);
extern bool KheTaskIsPreassigned(KHE_TASK task, bool as_in_event_resource,
  KHE_RESOURCE *r);

extern bool KheTaskSetDomainCheck(KHE_TASK task, KHE_RESOURCE_GROUP rg,
  bool recursive);
extern bool KheTaskSetDomain(KHE_TASK task, KHE_RESOURCE_GROUP rg,
  bool recursive);
extern KHE_RESOURCE_GROUP KheTaskDomain(KHE_TASK task);

extern bool KheTaskTightenDomainCheck(KHE_TASK task, KHE_RESOURCE_GROUP rg,
  bool recursive);
extern bool KheTaskTightenDomain(KHE_TASK task, KHE_RESOURCE_GROUP rg,
  bool recursive);

/* ***
extern bool KheTaskSetDomainCheck(KHE_TASK task, KHE_RESOURCE_GROUP rg);
extern bool KheTaskSetDomain(KHE_TASK task, KHE_RESOURCE_GROUP rg);
extern KHE_RESOURCE_GROUP KheTaskDomain(KHE_TASK task);

extern bool KheTaskTightenDomainCheck(KHE_TASK task, KHE_RESOURCE_GROUP rg);
extern bool KheTaskTightenDomain(KHE_TASK task, KHE_RESOURCE_GROUP rg);

extern bool KheTaskSetDomainAllCheck(KHE_TASK task, KHE_RESOURCE_GROUP rg);
extern bool KheTaskSetDomainAll(KHE_TASK task, KHE_RESOURCE_GROUP rg);
extern bool KheTaskTightenDomainAllCheck(KHE_TASK task, KHE_RESOURCE_GROUP rg);
extern bool KheTaskTightenDomainAll(KHE_TASK task, KHE_RESOURCE_GROUP rg);
*** */

/* 4.9 Transactions */
extern KHE_TRANSACTION KheTransactionMake(KHE_SOLN soln);
extern KHE_SOLN KheTransactionSoln(KHE_TRANSACTION t);
extern void KheTransactionDelete(KHE_TRANSACTION t);
extern void KheTransactionBegin(KHE_TRANSACTION t);
extern void KheTransactionEnd(KHE_TRANSACTION t);
extern void KheTransactionUndo(KHE_TRANSACTION t);
extern void KheTransactionRedo(KHE_TRANSACTION t);
extern void KheTransactionCopy(KHE_TRANSACTION src_t, KHE_TRANSACTION dst_t);
extern void KheTransactionDebug(KHE_TRANSACTION t, int verbosity,
  int indent, FILE *fp);

/* Transaction monitoring */
/* ***
extern KHE_COST KheTransactionStartCost(KHE_TRANSACTION t);
extern KHE_COST KheTransactionEndCost(KHE_TRANSACTION t);
extern KHE_COST KheTransactionCostChange(KHE_TRANSACTION t);

extern int KheTransactionMonitorCount(KHE_TRANSACTION t);
extern KHE_MONITOR KheTransactionMonitor(KHE_TRANSACTION t, int i);
extern KHE_COST KheTransactionMonitorStartCost(KHE_TRANSACTION t, int i);
extern KHE_COST KheTransactionMonitorEndCost(KHE_TRANSACTION t, int i);
extern KHE_COST KheTransactionMonitorCostChange(KHE_TRANSACTION t, int i);
*** */


/*****************************************************************************/
/*                                                                           */
/*    Chapter 5.   Extra Types for Solving                                   */
/*                                                                           */
/*****************************************************************************/

/* 5.2 Layers */
/* ***
extern KHE_LAYER KheLayerMake(KHE_SOLN soln, KHE_RESOURCE r);
extern void KheLayerSetBack(KHE_LAYER layer, void *back);
extern void *KheLayerBack(KHE_LAYER layer);

extern KHE_SOLN KheLayerSoln(KHE_LAYER layer);
extern KHE_RESOURCE KheLayerResource(KHE_LAYER layer);
extern int KheLayerIndex(KHE_LAYER layer);

extern void KheLayerDelete(KHE_LAYER layer);

extern bool KheLayerAddMeetCheck(KHE_LAYER layer, KHE_MEET meet);
extern bool KheLayerAddMeet(KHE_LAYER layer, KHE_MEET meet);
extern void KheLayerDeleteMeet(KHE_LAYER layer, KHE_MEET meet);

extern int KheLayerMeetCount(KHE_LAYER layer);
extern KHE_MEET KheLayerMeet(KHE_LAYER layer, int i);

extern int KheLayerDuration(KHE_LAYER layer);
extern int KheLayerAssignedDuration(KHE_LAYER layer);
extern int KheLayerDemand(KHE_LAYER layer);

extern bool KheLayerContainsMeet(KHE_LAYER layer, KHE_MEET meet);
extern bool KheLayerTouchesMeet(KHE_LAYER layer, KHE_MEET meet);

extern bool KheLayerIsRedundant(KHE_LAYER layer);

extern void KheLayerDebug(KHE_LAYER layer, int verbosity,
  int indent, FILE *fp);
*** */

/* 5.2 Nodes */
extern KHE_NODE KheNodeMake(KHE_SOLN soln);
extern void KheNodeSetBack(KHE_NODE node, void *back);
extern void *KheNodeBack(KHE_NODE node);

extern void KheNodeSetVisitNum(KHE_NODE n, int num);
extern int KheNodeVisitNum(KHE_NODE n);
extern bool KheNodeVisited(KHE_NODE n, int slack);
extern void KheNodeVisit(KHE_NODE n);
extern void KheNodeUnVisit(KHE_NODE n);

extern KHE_SOLN KheNodeSoln(KHE_NODE node);
extern int KheNodeIndex(KHE_NODE node);

extern bool KheNodeDeleteCheck(KHE_NODE node);
extern bool KheNodeDelete(KHE_NODE node);

extern bool KheNodeAddParentCheck(KHE_NODE child_node, KHE_NODE parent_node);
extern bool KheNodeAddParent(KHE_NODE child_node, KHE_NODE parent_node);
extern bool KheNodeDeleteParentCheck(KHE_NODE child_node);
extern bool KheNodeDeleteParent(KHE_NODE child_node);
extern KHE_NODE KheNodeParent(KHE_NODE node);

extern int KheNodeChildCount(KHE_NODE node);
extern KHE_NODE KheNodeChild(KHE_NODE node, int i);

extern bool KheNodeAddMeetCheck(KHE_NODE node, KHE_MEET meet);
extern bool KheNodeAddMeet(KHE_NODE node, KHE_MEET meet);
extern bool KheNodeDeleteMeetCheck(KHE_NODE node, KHE_MEET meet);
extern bool KheNodeDeleteMeet(KHE_NODE node, KHE_MEET meet);

extern int KheNodeMeetCount(KHE_NODE node);
extern KHE_MEET KheNodeMeet(KHE_NODE node, int i);
extern void KheNodeMeetSort(KHE_NODE node,
  int(*compar)(const void *, const void *));
extern int KheMeetDecreasingDurationCmp(const void *p1, const void *p2);
extern int KheMeetIncreasingAsstCmp(const void *p1, const void *p2);

extern int KheNodeDuration(KHE_NODE node);
extern int KheNodeAssignedDuration(KHE_NODE node);
extern int KheNodeDemand(KHE_NODE node);

extern bool KheNodeSimilar(KHE_NODE node1, KHE_NODE node2);
extern bool KheNodeRegular(KHE_NODE node1, KHE_NODE node2, int *regular_count);
extern int KheNodeResourceDuration(KHE_NODE node, KHE_RESOURCE r);

void KheNodeDebug(KHE_NODE node, int verbosity, int indent, FILE *fp);
extern void KheNodePrintTimetable(KHE_NODE node, int cell_width,
  int indent, FILE *fp);
/* ***
void KheNodeMatchingDebug(KHE_NODE node, int verbosity, int indent, FILE *fp);
*** */

/* 5.2.1 Nodes - node moving */
extern bool KheNodeMoveCheck(KHE_NODE child_node, KHE_NODE parent_node);
extern bool KheNodeMove(KHE_NODE child_node, KHE_NODE parent_node);

/* 5.2.2 Nodes - node meet swapping and assignment */
extern bool KheNodeMeetSwapCheck(KHE_NODE node1, KHE_NODE node2);
extern bool KheNodeMeetSwap(KHE_NODE node1, KHE_NODE node2);
extern bool KheNodeRegularAssignCheck(KHE_NODE node, KHE_NODE sibling_node);
extern bool KheNodeRegularAssign(KHE_NODE node, KHE_NODE sibling_node);
extern void KheNodeUnAssign(KHE_NODE node);

/* 5.2.3 Nodes - node merging and splitting */
extern bool KheNodeMergeCheck(KHE_NODE node1, KHE_NODE node2);
extern bool KheNodeMerge(KHE_NODE node1, KHE_NODE node2, KHE_NODE *res);
extern bool KheNodeSplitCheck(KHE_NODE node, int meet_count1, int child_count1);
extern bool KheNodeSplit(KHE_NODE node, int meet_count1, int child_count1,
  KHE_NODE *res1, KHE_NODE *res2);

/* 5.2.4 Nodes- Node meet merging and splitting */
extern void KheNodeMeetSplit(KHE_NODE node, bool recursive);
extern void KheNodeMeetMerge(KHE_NODE node);

/* 5.3 Layers */
extern KHE_LAYER KheLayerMake(KHE_NODE parent_node);
extern void KheLayerSetBack(KHE_LAYER layer, void *back);
extern void *KheLayerBack(KHE_LAYER layer);

extern void KheLayerSetVisitNum(KHE_LAYER layer, int num);
extern int KheLayerVisitNum(KHE_LAYER layer);
extern bool KheLayerVisited(KHE_LAYER layer, int slack);
extern void KheLayerVisit(KHE_LAYER layer);
extern void KheLayerUnVisit(KHE_LAYER layer);

extern KHE_NODE KheLayerParentNode(KHE_LAYER layer);
extern KHE_SOLN KheLayerSoln(KHE_LAYER layer);
extern void KheLayerDelete(KHE_LAYER layer);

extern void KheLayerAddChildNode(KHE_LAYER layer, KHE_NODE n);
extern void KheLayerDeleteChildNode(KHE_LAYER layer, KHE_NODE n);
extern int KheNodeParentLayerCount(KHE_NODE child_node);
extern KHE_LAYER KheNodeParentLayer(KHE_NODE child_node, int i);
extern bool KheNodeSameParentLayers(KHE_NODE node1, KHE_NODE node2);

extern int KheNodeChildLayerCount(KHE_NODE parent_node);
extern KHE_LAYER KheNodeChildLayer(KHE_NODE parent_node, int i);
extern int KheLayerIndex(KHE_LAYER layer);
extern void KheNodeChildLayersSort(KHE_NODE parent_node,
  int(*compar)(const void *, const void *));
extern void KheNodeChildLayersDelete(KHE_NODE parent_node);

extern void KheLayerAddResource(KHE_LAYER layer, KHE_RESOURCE r);
extern void KheLayerDeleteResource(KHE_LAYER layer, KHE_RESOURCE r);
extern int KheLayerResourceCount(KHE_LAYER layer);
extern KHE_RESOURCE KheLayerResource(KHE_LAYER layer, int i);

extern int KheLayerDuration(KHE_LAYER layer);
extern int KheLayerChildNodeCount(KHE_LAYER layer);
extern KHE_NODE KheLayerChildNode(KHE_LAYER layer, int i);
extern int KheLayerAssignedDuration(KHE_LAYER layer);
extern int KheLayerDemand(KHE_LAYER layer);

extern bool KheLayerEqual(KHE_LAYER layer1, KHE_LAYER layer2);
extern bool KheLayerSubset(KHE_LAYER layer1, KHE_LAYER layer2);
extern bool KheLayerDisjoint(KHE_LAYER layer1, KHE_LAYER layer2);
extern bool KheLayerContains(KHE_LAYER layer, KHE_NODE n);

extern bool KheLayerSame(KHE_LAYER layer1, KHE_LAYER layer2, int *same_count);
extern bool KheLayerSimilar(KHE_LAYER layer1, KHE_LAYER layer2,
  int *similar_count);
extern bool KheLayerRegular(KHE_LAYER layer1, KHE_LAYER layer2,
  int *regular_count);
extern bool KheLayerAlign(KHE_LAYER layer1, KHE_LAYER layer2,
  bool (*node_equiv)(KHE_NODE node1, KHE_NODE node2), int *count);

extern void KheLayerMerge(KHE_LAYER layer1, KHE_LAYER layer2, KHE_LAYER *res);
extern void KheLayerDebug(KHE_LAYER layer, int verbosity, int indent, FILE *fp);

/* 5.4 Zones */
extern KHE_ZONE KheZoneMake(KHE_NODE node);
extern KHE_NODE KheZoneNode(KHE_ZONE zone);
extern void KheZoneSetBack(KHE_ZONE zone, void *back);
extern void *KheZoneBack(KHE_ZONE zone);
extern void KheZoneSetVisitNum(KHE_ZONE zone, int num);
extern int KheZoneVisitNum(KHE_ZONE zone);
extern bool KheZoneVisited(KHE_ZONE zone, int slack);
extern void KheZoneVisit(KHE_ZONE zone);
extern void KheZoneUnVisit(KHE_ZONE zone);
extern void KheZoneDelete(KHE_ZONE zone);
extern void KheNodeDeleteZones(KHE_NODE node);

extern void KheMeetOffsetAddZone(KHE_MEET meet, int offset, KHE_ZONE zone);
extern void KheMeetOffsetDeleteZone(KHE_MEET meet, int offset);
extern KHE_ZONE KheMeetOffsetZone(KHE_MEET meet, int offset);
extern int KheNodeZoneCount(KHE_NODE node);
extern KHE_ZONE KheNodeZone(KHE_NODE node, int i);
extern int KheZoneIndex(KHE_ZONE zone);
extern int KheZoneMeetOffsetCount(KHE_ZONE zone);
extern void KheZoneMeetOffset(KHE_ZONE zone, int i,
  KHE_MEET *meet, int *offset);
extern void KheZoneDebug(KHE_ZONE zone, int verbosity, int indent, FILE *fp);
extern void KheLayerInstallZonesInParent(KHE_LAYER layer);
extern bool KheMeetMovePreservesZones(KHE_MEET meet1, int offset1,
  KHE_MEET meet2, int offset2, int durn);

/* 5.5 Taskings */
extern KHE_TASKING KheTaskingMake(KHE_SOLN soln, KHE_RESOURCE_TYPE rt);
extern KHE_SOLN KheTaskingSoln(KHE_TASKING tasking);
extern KHE_RESOURCE_TYPE KheTaskingResourceType(KHE_TASKING tasking);
extern void KheTaskingDelete(KHE_TASKING tasking);

extern void KheTaskingAddTask(KHE_TASKING tasking, KHE_TASK task);
extern void KheTaskingDeleteTask(KHE_TASKING tasking, KHE_TASK task);
extern int KheTaskingTaskCount(KHE_TASKING tasking);
extern KHE_TASK KheTaskingTask(KHE_TASKING tasking, int i);

extern void KheTaskingDebug(KHE_TASKING tasking, int verbosity,
  int indent, FILE *fp);
/* extern void KheTaskingSortForTaskGroups(KHE_TASKING tasking); */


/*****************************************************************************/
/*                                                                           */
/*    Chapter 6.   Solution Monitoring                                       */
/*                                                                           */
/*****************************************************************************/

/* 6.1 Measuring cost */
extern KHE_COST KheSolnCost(KHE_SOLN soln);
extern KHE_COST KheCost(int hard_cost, int soft_cost);
extern int KheHardCost(KHE_COST combined_cost);
extern int KheSoftCost(KHE_COST combined_cost);
extern int KheCostCmp(KHE_COST cost1, KHE_COST cost2);
extern double KheCostShow(KHE_COST combined_cost);

/* 6.2 Monitors */
extern int KheMonitorIndexInSoln(KHE_MONITOR m);
extern void KheMonitorSetBack(KHE_MONITOR m, void *back);
extern void *KheMonitorBack(KHE_MONITOR m);
extern KHE_SOLN KheMonitorSoln(KHE_MONITOR m);
extern KHE_COST KheMonitorCost(KHE_MONITOR m);
extern KHE_MONITOR_TAG KheMonitorTag(KHE_MONITOR m);
extern KHE_CONSTRAINT KheMonitorConstraint(KHE_MONITOR m);
extern char *KheMonitorAppliesToName(KHE_MONITOR m);

extern int KheMonitorDeviationCount(KHE_MONITOR m);
extern int KheMonitorDeviation(KHE_MONITOR m, int i);
extern char *KheMonitorDeviationDescription(KHE_MONITOR m, int i);

extern void KheMonitorDebug(KHE_MONITOR m, int verbosity, int indent, FILE *fp);
extern char *KheMonitorTagShow(KHE_MONITOR_TAG tag);
extern char *KheMonitorLabel(KHE_MONITOR m);

/* 6.3 Attaching and detaching */
extern void KheMonitorDetachFromSoln(KHE_MONITOR m);
extern void KheMonitorAttachToSoln(KHE_MONITOR m);
extern bool KheMonitorAttachedToSoln(KHE_MONITOR m);
extern void KheSolnEnsureOfficialCost(KHE_SOLN soln);
extern void KheMonitorAttachCheck(KHE_MONITOR m);

/* 6.4 Event monitors */
extern int KheEventMonitorCount(KHE_SOLN soln, KHE_EVENT e);
extern KHE_MONITOR KheEventMonitor(KHE_SOLN soln, KHE_EVENT e, int i);

extern KHE_COST KheEventCost(KHE_SOLN soln, KHE_EVENT e);
extern KHE_COST KheEventMonitorCost(KHE_SOLN soln, KHE_EVENT e,
  KHE_MONITOR_TAG tag);

extern KHE_ASSIGN_TIME_CONSTRAINT KheAssignTimeMonitorConstraint(
  KHE_ASSIGN_TIME_MONITOR m);
extern KHE_EVENT KheAssignTimeMonitorEvent(KHE_ASSIGN_TIME_MONITOR m);

extern KHE_PREFER_TIMES_CONSTRAINT KhePreferTimesMonitorConstraint(
  KHE_PREFER_TIMES_MONITOR m);
extern KHE_EVENT KhePreferTimesMonitorEvent(KHE_PREFER_TIMES_MONITOR m);

extern KHE_SPLIT_EVENTS_CONSTRAINT KheSplitEventsMonitorConstraint(
  KHE_SPLIT_EVENTS_MONITOR m);
extern KHE_EVENT KheSplitEventsMonitorEvent(KHE_SPLIT_EVENTS_MONITOR m);

extern KHE_DISTRIBUTE_SPLIT_EVENTS_CONSTRAINT
  KheDistributeSplitEventsMonitorConstraint(
  KHE_DISTRIBUTE_SPLIT_EVENTS_MONITOR m);
extern KHE_EVENT KheDistributeSplitEventsMonitorEvent(
  KHE_DISTRIBUTE_SPLIT_EVENTS_MONITOR m);

extern KHE_SPREAD_EVENTS_CONSTRAINT KheSpreadEventsMonitorConstraint(
  KHE_SPREAD_EVENTS_MONITOR m);
extern KHE_EVENT_GROUP KheSpreadEventsMonitorEventGroup(
  KHE_SPREAD_EVENTS_MONITOR m);
extern int KheSpreadEventsMonitorTimeGroupCount(KHE_SPREAD_EVENTS_MONITOR m);
extern void KheSpreadEventsMonitorTimeGroup(KHE_SPREAD_EVENTS_MONITOR m, int i,
  KHE_TIME_GROUP *time_group, int *minimum, int *maximum, int *incidences);

extern KHE_LINK_EVENTS_CONSTRAINT KheLinkEventsMonitorConstraint(
  KHE_LINK_EVENTS_MONITOR m);
extern KHE_EVENT_GROUP KheLinkEventsMonitorEventGroup(
  KHE_LINK_EVENTS_MONITOR m);

/* 6.5 Event resource monitors */
extern int KheEventResourceMonitorCount(KHE_SOLN soln, KHE_EVENT_RESOURCE er);
extern KHE_MONITOR KheEventResourceMonitor(KHE_SOLN soln,
  KHE_EVENT_RESOURCE er, int i);

extern KHE_COST KheEventResourceCost(KHE_SOLN soln, KHE_EVENT_RESOURCE er);
extern KHE_COST KheEventResourceMonitorCost(KHE_SOLN soln,
  KHE_EVENT_RESOURCE er, KHE_MONITOR_TAG tag);

extern KHE_ASSIGN_RESOURCE_CONSTRAINT KheAssignResourceMonitorConstraint(
  KHE_ASSIGN_RESOURCE_MONITOR m);
extern KHE_EVENT_RESOURCE KheAssignResourceMonitorEventResource(
  KHE_ASSIGN_RESOURCE_MONITOR m);

extern KHE_PREFER_RESOURCES_CONSTRAINT KhePreferResourcesMonitorConstraint(
  KHE_PREFER_RESOURCES_MONITOR m);
extern KHE_EVENT_RESOURCE KhePreferResourcesMonitorEventResource(
  KHE_PREFER_RESOURCES_MONITOR m);

extern KHE_AVOID_SPLIT_ASSIGNMENTS_CONSTRAINT
  KheAvoidSplitAssignmentsMonitorConstraint(
  KHE_AVOID_SPLIT_ASSIGNMENTS_MONITOR m);
extern int KheAvoidSplitAssignmentsMonitorEventGroupIndex(
  KHE_AVOID_SPLIT_ASSIGNMENTS_MONITOR m);
extern int KheAvoidSplitAssignmentsMonitorResourceCount(
  KHE_AVOID_SPLIT_ASSIGNMENTS_MONITOR m);
extern KHE_RESOURCE KheAvoidSplitAssignmentsMonitorResource(
  KHE_AVOID_SPLIT_ASSIGNMENTS_MONITOR m, int i);
extern int KheAvoidSplitAssignmentsMonitorResourceMultiplicity(
  KHE_AVOID_SPLIT_ASSIGNMENTS_MONITOR m, int i);

/* 6.6 Resource monitors */
extern int KheResourceMonitorCount(KHE_SOLN soln, KHE_RESOURCE r);
extern KHE_MONITOR KheResourceMonitor(KHE_SOLN soln, KHE_RESOURCE r, int i);

extern KHE_COST KheResourceCost(KHE_SOLN soln, KHE_RESOURCE r);
extern KHE_COST KheResourceMonitorCost(KHE_SOLN soln, KHE_RESOURCE r,
  KHE_MONITOR_TAG tag);

extern KHE_AVOID_CLASHES_CONSTRAINT KheAvoidClashesMonitorConstraint(
  KHE_AVOID_CLASHES_MONITOR m);
extern KHE_RESOURCE KheAvoidClashesMonitorResource(KHE_AVOID_CLASHES_MONITOR m);

extern KHE_AVOID_UNAVAILABLE_TIMES_CONSTRAINT
  KheAvoidUnavailableTimesMonitorConstraint(
  KHE_AVOID_UNAVAILABLE_TIMES_MONITOR m);
extern KHE_RESOURCE KheAvoidUnavailableTimesMonitorResource(
  KHE_AVOID_UNAVAILABLE_TIMES_MONITOR m);

extern KHE_LIMIT_IDLE_TIMES_CONSTRAINT KheLimitIdleTimesMonitorConstraint(
  KHE_LIMIT_IDLE_TIMES_MONITOR m);
extern KHE_RESOURCE KheLimitIdleTimesMonitorResource(
  KHE_LIMIT_IDLE_TIMES_MONITOR m);

extern KHE_CLUSTER_BUSY_TIMES_CONSTRAINT KheClusterBusyTimesMonitorConstraint(
  KHE_CLUSTER_BUSY_TIMES_MONITOR m);
extern KHE_RESOURCE KheClusterBusyTimesMonitorResource(
  KHE_CLUSTER_BUSY_TIMES_MONITOR m);

extern KHE_LIMIT_BUSY_TIMES_CONSTRAINT KheLimitBusyTimesMonitorConstraint(
  KHE_LIMIT_BUSY_TIMES_MONITOR m);
extern KHE_RESOURCE KheLimitBusyTimesMonitorResource(
  KHE_LIMIT_BUSY_TIMES_MONITOR m);

extern KHE_LIMIT_WORKLOAD_CONSTRAINT KheLimitWorkloadMonitorConstraint(
  KHE_LIMIT_WORKLOAD_MONITOR m);
extern KHE_RESOURCE KheLimitWorkloadMonitorResource(
  KHE_LIMIT_WORKLOAD_MONITOR m);
extern float KheLimitWorkloadMonitorWorkload(KHE_LIMIT_WORKLOAD_MONITOR m);

/* 6.7 Timetable monitors */
extern KHE_TIMETABLE_MONITOR KheResourceTimetableMonitor(KHE_SOLN soln,
  KHE_RESOURCE r);
extern KHE_TIMETABLE_MONITOR KheEventTimetableMonitor(KHE_SOLN soln, KHE_EVENT e);

extern int KheTimetableMonitorTimeMeetCount(KHE_TIMETABLE_MONITOR tt,
  KHE_TIME time);
extern KHE_MEET KheTimetableMonitorTimeMeet(KHE_TIMETABLE_MONITOR tt,
  KHE_TIME time, int i);
extern void KheTimetableMonitorPrintTimetable(KHE_TIMETABLE_MONITOR tm,
  int cell_width, int indent, FILE *fp);


/*****************************************************************************/
/*                                                                           */
/*    Chapter 7.   Matchings and Evenness                                    */
/*                                                                           */
/*****************************************************************************/

/* 7.2 Setting up */
extern void KheSolnMatchingSetWeight(KHE_SOLN soln, KHE_COST weight);
extern KHE_COST KheSolnMatchingWeight(KHE_SOLN soln);
extern KHE_COST KheSolnMinMatchingWeight(KHE_SOLN soln);
extern KHE_MATCHING_TYPE KheSolnMatchingType(KHE_SOLN soln);
extern void KheSolnMatchingSetType(KHE_SOLN soln, KHE_MATCHING_TYPE mt);
extern void KheSolnMatchingMarkBegin(KHE_SOLN soln);
extern void KheSolnMatchingMarkEnd(KHE_SOLN soln, bool undo);
extern void KheSolnMatchingDebug(KHE_SOLN soln, int verbosity,
  int indent, FILE *fp);

/* 7.3 Ordinary supply and demand nodes */
extern int KheTaskDemandMonitorCount(KHE_TASK task);
extern KHE_ORDINARY_DEMAND_MONITOR KheTaskDemandMonitor(KHE_TASK task, int i);
extern KHE_TASK KheOrdinaryDemandMonitorTask(KHE_ORDINARY_DEMAND_MONITOR m);
extern int KheOrdinaryDemandMonitorOffset(KHE_ORDINARY_DEMAND_MONITOR m);
extern void KheSolnMatchingAttachAllOrdinaryDemandMonitors(KHE_SOLN soln);
extern void KheSolnMatchingDetachAllOrdinaryDemandMonitors(KHE_SOLN soln);

/* *** these work, but there are now good reasons not to use them
extern void KheSolnMatchingAttachPreassignedDemandMonitors(KHE_SOLN soln,
  KHE_RESOURCE r);
extern void KheSolnMatchingDetachPreassignedDemandMonitors(KHE_SOLN soln,
  KHE_RESOURCE r);
extern void KheSolnMatchingAttachEligiblePreassignedDemandMonitors(
  KHE_SOLN soln);
extern void KheSolnMatchingDetachEligiblePreassignedDemandMonitors(
  KHE_SOLN soln);
*** */

/* 7.4 Workload demand nodes */
extern void KheSolnMatchingAddAllWorkloadRequirements(KHE_SOLN soln);
extern int KheSolnMatchingWorkloadRequirementCount(KHE_SOLN soln,
  KHE_RESOURCE r);
extern void KheSolnMatchingWorkloadRequirement(KHE_SOLN soln,
  KHE_RESOURCE r, int i, int *num, KHE_TIME_GROUP *tg);
extern void KheSolnMatchingBeginWorkloadRequirements(KHE_SOLN soln,
  KHE_RESOURCE r);
extern void KheSolnMatchingAddWorkloadRequirement(KHE_SOLN soln,
  KHE_RESOURCE r, int num, KHE_TIME_GROUP tg);
extern void KheSolnMatchingEndWorkloadRequirements(KHE_SOLN soln,
  KHE_RESOURCE r);
extern KHE_RESOURCE KheWorkloadDemandMonitorResource(
  KHE_WORKLOAD_DEMAND_MONITOR m);
extern KHE_TIME_GROUP KheWorkloadDemandMonitorTimeGroup(
  KHE_WORKLOAD_DEMAND_MONITOR m);

/* 7.5 Diagnosing failure to match */
extern int KheSolnMatchingDefectCount(KHE_SOLN soln);
extern KHE_MONITOR KheSolnMatchingDefect(KHE_SOLN soln, int i);

extern int KheSolnMatchingHallSetCount(KHE_SOLN soln);
extern int KheSolnMatchingHallSetSupplyNodeCount(KHE_SOLN soln, int i);
extern int KheSolnMatchingHallSetDemandNodeCount(KHE_SOLN soln, int i);
extern bool KheSolnMatchingHallSetSupplyNodeIsOrdinary(KHE_SOLN soln,
  int i, int j, KHE_MEET *meet, int *meet_offset, KHE_RESOURCE *r);
extern KHE_MONITOR KheSolnMatchingHallSetDemandNode(KHE_SOLN soln,
  int i, int j);
/* ***
extern void KheSolnMatchingHallSetDebug(KHE_SOLN soln, int i,
  int verbosity, int indent, FILE *fp);
*** */
extern void KheSolnMatchingHallSetsDebug(KHE_SOLN soln,
  int verbosity, int indent, FILE *fp);

extern KHE_MONITOR KheMonitorFirstCompetitor(KHE_MONITOR m);
extern KHE_MONITOR KheMonitorNextCompetitor(KHE_MONITOR m);

/* 7.6 Evenness monitoring */
extern void KheSolnAttachAllEvennessMonitors(KHE_SOLN soln);
extern void KheSolnDetachAllEvennessMonitors(KHE_SOLN soln);
extern KHE_RESOURCE_GROUP KheEvennessMonitorPartition(KHE_EVENNESS_MONITOR m);
extern KHE_TIME KheEvennessMonitorTime(KHE_EVENNESS_MONITOR m);
extern int KheEvennessMonitorCount(KHE_EVENNESS_MONITOR m);
extern int KheEvennessMonitorLimit(KHE_EVENNESS_MONITOR m);
extern void KheEvennessMonitorSetLimit(KHE_EVENNESS_MONITOR m, int limit);
extern KHE_COST KheEvennessMonitorWeight(KHE_EVENNESS_MONITOR m);
extern void KheEvennessMonitorSetWeight(KHE_EVENNESS_MONITOR m,
  KHE_COST weight);
extern void KheSolnSetAllEvennessMonitorWeights(KHE_SOLN soln, KHE_COST weight);


/*****************************************************************************/
/*                                                                           */
/*    Chapter 8.   Monitor Grouping and Tracing                              */
/*                                                                           */
/*****************************************************************************/

/* 8.2 Group monitors */
extern KHE_GROUP_MONITOR KheMonitorParentMonitor(KHE_MONITOR m);
extern bool KheMonitorDescendant(KHE_MONITOR m1, KHE_MONITOR m2);

extern KHE_GROUP_MONITOR KheGroupMonitorMake(KHE_SOLN soln, int sub_tag,
  char *sub_tag_label);
extern int KheGroupMonitorSubTag(KHE_GROUP_MONITOR gm);
extern char *KheGroupMonitorSubTagLabel(KHE_GROUP_MONITOR gm);
extern void KheGroupMonitorDelete(KHE_GROUP_MONITOR gm);

extern void KheGroupMonitorAddChildMonitor(KHE_GROUP_MONITOR gm, KHE_MONITOR m);
extern void KheGroupMonitorDeleteChildMonitor(KHE_GROUP_MONITOR gm,
  KHE_MONITOR m);

extern int KheGroupMonitorChildMonitorCount(KHE_GROUP_MONITOR gm);
extern KHE_MONITOR KheGroupMonitorChildMonitor(KHE_GROUP_MONITOR gm, int i);

extern int KheSolnChildMonitorCount(KHE_SOLN soln);
extern KHE_MONITOR KheSolnChildMonitor(KHE_SOLN soln, int i);

extern void KheGroupMonitorBypassAndDelete(KHE_GROUP_MONITOR gm);

/* 8.3 Defects */
extern int KheGroupMonitorDefectCount(KHE_GROUP_MONITOR gm);
extern KHE_MONITOR KheGroupMonitorDefect(KHE_GROUP_MONITOR gm, int i);
extern void KheGroupMonitorDefectSort(KHE_GROUP_MONITOR gm);
extern int KheSolnDefectCount(KHE_SOLN soln);
extern KHE_MONITOR KheSolnDefect(KHE_SOLN soln, int i);
extern void KheSolnDefectSort(KHE_SOLN soln);

extern void KheGroupMonitorCopyDefects(KHE_GROUP_MONITOR gm);
extern int KheGroupMonitorDefectCopyCount(KHE_GROUP_MONITOR gm);
extern KHE_MONITOR KheGroupMonitorDefectCopy(KHE_GROUP_MONITOR gm, int i);
extern void KheSolnCopyDefects(KHE_SOLN soln);
extern int KheSolnDefectCopyCount(KHE_SOLN soln);
extern KHE_MONITOR KheSolnDefectCopy(KHE_SOLN soln, int i);

extern void KheGroupMonitorDefectDebug(KHE_GROUP_MONITOR gm,
  int verbosity, int indent, FILE *fp);

extern KHE_COST KheGroupMonitorCostByType(KHE_GROUP_MONITOR gm,
  KHE_MONITOR_TAG tag, int *defect_count);
extern KHE_COST KheSolnCostByType(KHE_SOLN soln, KHE_MONITOR_TAG tag,
  int *defect_count);

extern void KheGroupMonitorCostByTypeDebug(KHE_GROUP_MONITOR gm,
  int verbosity, int indent, FILE *fp);
extern void KheSolnCostByTypeDebug(KHE_SOLN soln,
  int verbosity, int indent, FILE *fp);

/* 8.4 Tracing */
extern KHE_TRACE KheTraceMake(KHE_GROUP_MONITOR gm);
extern void KheTraceDelete(KHE_TRACE t);
extern void KheTraceBegin(KHE_TRACE t);
extern void KheTraceEnd(KHE_TRACE t);

extern KHE_COST KheTraceInitCost(KHE_TRACE t);
extern int KheTraceMonitorCount(KHE_TRACE t);
extern KHE_MONITOR KheTraceMonitor(KHE_TRACE t, int i);
extern KHE_COST KheTraceMonitorInitCost(KHE_TRACE t, int i);

/* 8.5 Helper functions for grouping monitors */
extern KHE_GROUP_MONITOR KheSolnGroupEventMonitors(KHE_SOLN soln,
  KHE_NODE assigned_to_node, int sub_tag, char *sub_tag_label,
  KHE_GROUP_MONITOR prnt);
extern KHE_GROUP_MONITOR KheNodeGroupEventMonitors(KHE_NODE node,
  KHE_NODE assigned_to_node, int sub_tag, char *sub_tag_label,
  KHE_GROUP_MONITOR prnt);
extern KHE_GROUP_MONITOR KheLayerGroupEventMonitors(KHE_LAYER layer,
  KHE_NODE assigned_to_node, int sub_tag, char *sub_tag_label,
  KHE_GROUP_MONITOR prnt);

extern void KheSolnGroupEventMonitorsByClass(KHE_SOLN soln,
  KHE_NODE assigned_to_node, int sub_tag, char *sub_tag_label,
  KHE_GROUP_MONITOR prnt);
extern void KheNodeGroupEventMonitorsByClass(KHE_NODE node,
  KHE_NODE assigned_to_node, int sub_tag, char *sub_tag_label,
  KHE_GROUP_MONITOR prnt);
extern void KheLayerGroupEventMonitorsByClass(KHE_LAYER layer,
  KHE_NODE assigned_to_node, int sub_tag, char *sub_tag_label,
  KHE_GROUP_MONITOR prnt);

extern void KheSolnGroupEventResourceMonitorsByClass(KHE_SOLN soln,
  KHE_NODE assigned_to_node, int sub_tag, char *sub_tag_label,
  KHE_GROUP_MONITOR prnt);
extern void KheNodeGroupEventResourceMonitorsByClass(KHE_NODE node,
  KHE_NODE assigned_to_node, int sub_tag, char *sub_tag_label,
  KHE_GROUP_MONITOR prnt);
extern void KheLayerGroupEventResourceMonitorsByClass(KHE_LAYER layer,
  KHE_NODE assigned_to_node, int sub_tag, char *sub_tag_label,
  KHE_GROUP_MONITOR prnt);
extern void KheTaskingGroupEventResourceMonitorsByClass(KHE_TASKING tasking,
  KHE_NODE assigned_to_node, int sub_tag, char *sub_tag_label,
  KHE_GROUP_MONITOR prnt);

extern void KheSolnGroupResourceMonitorsByResource(KHE_SOLN soln,
  int sub_tag, char *sub_tag_label, KHE_GROUP_MONITOR prnt);
extern void KheTaskingGroupResourceMonitorsByResource(KHE_TASKING tasking,
  int sub_tag, char *sub_tag_label, KHE_GROUP_MONITOR prnt);

extern KHE_GROUP_MONITOR KheSolnGroupDemandMonitors(KHE_SOLN soln,
  KHE_NODE assigned_to_node, bool include_preassigned,
  bool include_unpreassigned, bool include_workload,
  int sub_tag, char *sub_tag_label, KHE_GROUP_MONITOR prnt);
extern KHE_GROUP_MONITOR KheNodeGroupDemandMonitors(KHE_NODE node,
  KHE_NODE assigned_to_node, bool include_preassigned,
  bool include_unpreassigned, int sub_tag, char *sub_tag_label,
  KHE_GROUP_MONITOR prnt);
extern KHE_GROUP_MONITOR KheLayerGroupDemandMonitors(KHE_LAYER layer,
  KHE_NODE assigned_to_node, bool include_preassigned,
  bool include_unpreassigned, int sub_tag, char *sub_tag_label,
  KHE_GROUP_MONITOR prnt);
extern KHE_GROUP_MONITOR KheMeetGroupDemandMonitors(KHE_MEET meet,
  KHE_NODE assigned_to_node, bool include_preassigned,
  bool include_unpreassigned, int sub_tag, char *sub_tag_label,
  KHE_GROUP_MONITOR prnt);

extern void KheNodeGroupDemandMonitorsByLeaderMeet(KHE_NODE node,
  KHE_NODE assigned_to_node, bool include_preassigned,
  bool include_unpreassigned, int sub_tag, char *sub_tag_label,
  KHE_GROUP_MONITOR prnt);
extern void KheLayerGroupDemandMonitorsByLeaderMeet(KHE_LAYER layer,
  KHE_NODE assigned_to_node, bool include_preassigned,
  bool include_unpreassigned, int sub_tag, char *sub_tag_label,
  KHE_GROUP_MONITOR prnt);

extern void KheNodeGroupDemandMonitorsByChildNode(KHE_NODE parent_node,
  KHE_NODE assigned_to_node, bool include_preassigned,
  bool include_unpreassigned, int sub_tag, char *sub_tag_label,
  KHE_GROUP_MONITOR prnt);
extern void KheLayerGroupDemandMonitorsByChildNode(KHE_LAYER layer,
  KHE_NODE assigned_to_node, bool include_preassigned,
  bool include_unpreassigned, int sub_tag, char *sub_tag_label,
  KHE_GROUP_MONITOR prnt);

extern KHE_GROUP_MONITOR KheNodeGroupMonitorsForMeetRepair(KHE_NODE parent_node,
  KHE_NODE assigned_to_node, int event_sub_tag, char *event_sub_tag_label,
  int preassigned_demand_sub_tag, char *preassigned_demand_sub_tag_label,
  int unpreassigned_demand_sub_tag, char *unpreassigned_demand_sub_tag_label,
  int sub_tag, char *sub_tag_label, KHE_GROUP_MONITOR prnt);
extern KHE_GROUP_MONITOR KheLayerGroupMonitorsForMeetRepair(KHE_LAYER layer,
  KHE_NODE assigned_to_node, int event_sub_tag, char *event_sub_tag_label,
  int preassigned_demand_sub_tag, char *preassigned_demand_sub_tag_label,
  int unpreassigned_demand_sub_tag, char *unpreassigned_demand_sub_tag_label,
  int sub_tag, char *sub_tag_label, KHE_GROUP_MONITOR prnt);

extern KHE_GROUP_MONITOR KheNodeGroupMonitorsForNodeRepair(KHE_NODE parent_node,
  KHE_NODE assigned_to_node, int unpreassigned_demand_sub_tag,
  char *unpreassigned_demand_sub_tag_label, int sub_tag, char *sub_tag_label,
  KHE_GROUP_MONITOR prnt);

extern KHE_GROUP_MONITOR KheSolnGroupMonitorsForTaskRepair(
  KHE_SOLN soln, KHE_NODE assigned_to_node,
  int event_resource_sub_tag, char *event_resource_sub_tag_label,
  int resource_sub_tag, char *resource_sub_tag_label,
  int sub_tag, char *sub_tag_label, KHE_GROUP_MONITOR prnt);
extern KHE_GROUP_MONITOR KheTaskingGroupMonitorsForTaskRepair(
  KHE_TASKING tasking, KHE_NODE assigned_to_node,
  int event_resource_sub_tag, char *event_resource_sub_tag_label,
  int resource_sub_tag, char *resource_sub_tag_label,
  int sub_tag, char *sub_tag_label, KHE_GROUP_MONITOR prnt);

extern void KheUnGroupMonitors(KHE_GROUP_MONITOR gm);

/* *** old versions
extern void KheGroupEventMonitors(KHE_SOLN soln, int sub_tag,
  char *sub_tag_label);
extern void KheGroupEventResourceMonitors(KHE_SOLN soln, int sub_tag,
  char *sub_tag_label);
extern void KheGroupResourceMonitors(KHE_SOLN soln,
  int sub_tag, char *sub_tag_label);
extern KHE_GROUP_MONITOR KheGroupDemandMonitors(KHE_SOLN soln,
  bool include_preassigned, bool include_unpreassigned,
  bool include_workload, int sub_tag, char *sub_tag_label);
extern void KheGroupOrdinaryDemandMonitorsByMeet(KHE_SOLN soln,
  bool include_preassigned, bool include_unpreassigned,
  int sub_tag, char *sub_tag_label);
extern void KheGroupOrdinaryDemandMonitorsByChildNode(KHE_NODE parent_node,
  bool include_preassigned, bool include_unpreassigned,
  int sub_tag, char *sub_tag_label);
extern KHE_GROUP_MONITOR KheNodeGroupEventMonitors(KHE_NODE parent_node,
  int event_sub_tag, char *event_sub_tag_label,
  int preassigned_demand_sub_tag, char *preassigned_demand_sub_tag_label,
  int unpreassigned_demand_sub_tag, char *unpreassigned_demand_sub_tag_label,
  int sub_tag, char *sub_tag_label);
extern KHE_GROUP_MONITOR KheLayerGroupEventMonitors(KHE_LAYER layer,
  int event_sub_tag, char *event_sub_tag_label,
  int preassigned_demand_sub_tag, char *preassigned_demand_sub_tag_label,
  int unpreassigned_demand_sub_tag, char *unpreassigned_demand_sub_tag_label,
  int sub_tag, char *sub_tag_label);
extern KHE_GROUP_MONITOR KheTaskingGroupMonitors(KHE_TASKING tasking,
  int event_resource_sub_tag, char *event_resource_sub_tag_label,
  int resource_sub_tag, char *resource_sub_tag_label,
  int sub_tag, char *sub_tag_label);
extern void KheUnGroupAllMonitors(KHE_SOLN soln);
*** */


/*****************************************************************************/
/*                                                                           */
/*                    Part B:  Some Solvers                                  */
/*                                                                           */
/*****************************************************************************/

/*****************************************************************************/
/*                                                                           */
/*    Chapter 9.   Layer Tree Solvers                                        */
/*                                                                           */
/*****************************************************************************/

/* 9.1 Layer tree construction */
extern KHE_NODE KheLayerTreeMake(KHE_SOLN soln,
  bool check_prefer_times_monitors, bool check_split_events_monitors,
  bool check_link_events_monitors);

/* 9.2 Vizier nodes */
extern KHE_NODE KheNodeInsertVizierNode(KHE_NODE parent_node);
extern void KheNodeRemoveVizierNode(KHE_NODE vizier_node);

/* 9.3.1 Layer construction */
extern KHE_LAYER KheLayerMakeFromResource(KHE_NODE parent_node,
  KHE_RESOURCE r);
extern void KheNodeChildLayersMake(KHE_NODE parent_node);
extern int KheLayerDefaultCmp(const void *t1, const void *t2);

/* 9.3.2 Layer coordination */
extern void KheCoordinateLayers(KHE_NODE parent_node, bool with_domination);

/* 9.4 Runarounds */
extern bool KheMinimumRunaroundDuration(KHE_NODE parent_node,
  KHE_NODE_TIME_SOLVER time_solver, int *duration);
extern void KheBuildRunarounds(KHE_NODE parent_node,
  KHE_NODE_TIME_SOLVER mrd_solver, KHE_NODE_TIME_SOLVER runaround_solver);

/* 9.5 Flattening */
extern void KheNodeBypass(KHE_NODE node);
extern void KheNodeFlatten(KHE_NODE parent_node);

/* 9.6 Merging adjacent meets */
extern void KheMergeMeets(KHE_SOLN soln);


/*****************************************************************************/
/*                                                                           */
/*    Chapter 10.   Time Solvers                                             */
/*                                                                           */
/*****************************************************************************/

/* 10.2 Basic time solvers */
extern bool KheNodeUnAssignTimes(KHE_NODE parent_node);
extern bool KheLayerUnAssignTimes(KHE_LAYER layer);
extern bool KheNodeAllChildMeetsAssigned(KHE_NODE parent_node);
extern bool KheLayerAllChildMeetsAssigned(KHE_LAYER layer);
/* extern bool KheLayerAssignTimes(KHE_LAYER layer); */
extern bool KheNodeSimpleAssignTimes(KHE_NODE parent_node);
extern bool KheLayerSimpleAssignTimes(KHE_LAYER layer);
extern bool KheNodeRecursiveAssignTimes(KHE_NODE parent_node,
  KHE_NODE_TIME_SOLVER solver);

/* 10.3 Preassigned meet assignment */
extern bool KheNodePreassignedAssignTimes(KHE_NODE root_node);
extern bool KheLayerPreassignedAssignTimes(KHE_LAYER layer);

/* 10.4 A solver for runarounds */
extern bool KheRunaroundNodeAssignTimes(KHE_NODE parent_node);

/* 10.5 Kempe time moves */
extern bool KheKempeMeetMove(KHE_MEET meet, KHE_MEET target_meet,
  int target_offset, int *demand);
extern bool KheKempeMeetMoveTime(KHE_MEET meet, KHE_TIME t, int *demand);
/* ***
extern bool KheMeetKempeMove(KHE_MEET meet, KHE_MEET target_meet,
  int target_offset);
extern bool KheMeetKempeMoveTime(KHE_MEET meet, KHE_TIME t);
*** */

/* 10.6 Layered time assignment */
extern bool KheLayerParallelAssignTimes(KHE_LAYER layer);
extern bool KheNodeLayeredAssignTimes(KHE_NODE parent_node,
  bool vizier_splits, bool regular);
/* ***
extern bool KheNodeChildLayersAssignTimes(KHE_NODE parent_node);
extern int KheLayerSaturationDegreeCmp(const void *t1, const void *t2);
extern bool KheFirstLayerAssignTimes(KHE_LAYER layer);
extern bool KheOtherLayerAssignTimes(KHE_LAYER layer,
  KHE_LAYER *template_layer);
*** */

/* 10.7 Layer matching */
extern bool KheLayerMatchAssignTimes(KHE_LAYER layer,
  KHE_SPREAD_EVENTS_CONSTRAINT sec);
extern KHE_LAYER_MATCH KheLayerMatchMake(KHE_LAYER layer,
  KHE_SPREAD_EVENTS_CONSTRAINT sec);
extern KHE_LAYER KheLayerMatchLayer(KHE_LAYER_MATCH lm);
extern KHE_SPREAD_EVENTS_CONSTRAINT KheLayerMatchConstraint(KHE_LAYER_MATCH lm);
extern void KheLayerMatchDelete(KHE_LAYER_MATCH lm);
extern int KheLayerMatchDemandNodeCount(KHE_LAYER_MATCH lm);
extern KHE_MEET KheLayerMatchDemandNode(KHE_LAYER_MATCH lm, int i);
extern bool KheLayerMatchBestEdge(KHE_LAYER_MATCH lm, KHE_MEET meet,
  KHE_MEET *target_meet, int *target_offset, KHE_COST *cost);
extern bool KheLayerMatchAssignBestEdges(KHE_LAYER_MATCH lm);
extern KHE_COST KheLayerMatchCost(KHE_LAYER_MATCH lm);
extern void KheLayerMatchDebug(KHE_LAYER_MATCH lm, int verbosity,
  int indent, FILE *fp);
extern void KheLayerMatchImproveNodeRegularity(KHE_LAYER_MATCH lm);

/* 10.8 Regular time repair */
extern void KheLayerNodeMatchingNodeRepairTimes(KHE_NODE parent_node);
extern void KheLayerNodeMatchingLayerRepairTimes(KHE_LAYER layer);
extern bool KheNodeMeetSwapRepairTimes(KHE_NODE parent_node);
extern void KheLayerZoneRepairTimes(KHE_LAYER layer);
extern void KheNodeGlobalSwapRepairTimes(KHE_NODE parent_node);
extern bool KheEjectionChainRepairTimes(KHE_NODE parent_node);
extern bool KheEjectionChainLongRepairTimes(KHE_NODE parent_node);
extern bool KheSimpleRepairTimes(KHE_NODE parent_node,
  KHE_COST min_cost_improvement);

/* 10.9 Putting it all together */
extern bool KheCycleNodeAssignTimes(KHE_NODE cycle_node);


/*****************************************************************************/
/*                                                                           */
/*    Chapter 11.  Resource Solvers                                          */
/*                                                                           */
/*****************************************************************************/

/* 11.2 The resource assignment invariant */
extern void KheInvariantTransactionBegin(KHE_TRANSACTION t, int *init_count,
  bool preserve_invariant);
extern bool KheInvariantTransactionEnd(KHE_TRANSACTION t,
  int *init_count, bool preserve_invariant, bool success);

/* 11.3.1 Task tree construction */
extern void KheTaskTreeMake(KHE_SOLN soln, KHE_TASK_JOB_TYPE tjt,
  bool preserve_invariant, bool check_prefer_resources_monitors,
  bool check_avoid_split_assignments_monitors);
extern KHE_TASKING KheTaskingMakeFromResourceType(KHE_SOLN soln,
  KHE_RESOURCE_TYPE rt);
extern void KheTaskingMakeTaskTree(KHE_TASKING tasking, KHE_TASK_JOB_TYPE tjt,
  bool preserve_invariant, bool check_prefer_resources_monitors,
  bool check_avoid_split_assignments_monitors);

/* 11.3.2 Task tree reorganization */
extern void KheTaskingTightenToPartition(KHE_TASKING tasking);
extern void KheTaskingAllowSplitAssignments(KHE_TASKING tasking,
  bool unassigned_only);
extern void KheTaskingEnlargeDomains(KHE_TASKING tasking, bool unassigned_only);
extern void KheTaskingMonitorAttachCheck(KHE_TASKING tasking,
  KHE_MONITOR_TAG tag, bool unassigned_only);
/* ***
extern void KheTaskTreeDelete(KHE_SOLN soln);
extern void KheAvoidSplitAssignmentsAssignTasks(KHE_SOLN soln,
  KHE_RESOURCE_TYPE rt, bool detach_constraints);
extern KHE_TASKING KheTaskingMakeSelected(KHE_SOLN soln, KHE_RESOURCE_TYPE rt,
  bool unassigned_only);
extern KHE_TASKING KheTaskingMakeFromUnassignedTasks(KHE_SOLN soln,
  KHE_RESOURCE_TYPE rt);
extern KHE_TASKING KheTaskingMakeFromSoln(KHE_SOLN soln, KHE_RESOURCE_TYPE rt);
*** */

/* 11.4 Task groups */
extern KHE_TASK_GROUPS KheTaskGroupsMakeFromTasking(KHE_TASKING tasking);
extern void KheTaskGroupsDelete(KHE_TASK_GROUPS task_groups);
extern int KheTaskGroupsTaskGroupCount(KHE_TASK_GROUPS task_groups);
extern KHE_TASK_GROUP KheTaskGroupsTaskGroup(KHE_TASK_GROUPS task_groups,
  int i);
extern int KheTaskGroupTaskCount(KHE_TASK_GROUP task_group);
extern KHE_TASK KheTaskGroupTask(KHE_TASK_GROUP task_group, int i);

extern int KheTaskGroupTotalDuration(KHE_TASK_GROUP task_group);
extern float KheTaskGroupTotalWorkload(KHE_TASK_GROUP task_group);
extern KHE_RESOURCE_GROUP KheTaskGroupDomain(KHE_TASK_GROUP task_group);
extern int KheTaskGroupDecreasingDurationCmp(KHE_TASK_GROUP tg1,
  KHE_TASK_GROUP tg2);
extern int KheTaskGroupUnassignedTaskCount(KHE_TASK_GROUP task_group);

extern bool KheTaskGroupAssignCheck(KHE_TASK_GROUP task_group, KHE_RESOURCE r);
extern bool KheTaskGroupAssign(KHE_TASK_GROUP task_group, KHE_RESOURCE r);
extern void KheTaskGroupUnAssign(KHE_TASK_GROUP task_group, KHE_RESOURCE r);

extern void KheTaskGroupDebug(KHE_TASK_GROUP task_group, int verbosity,
  int indent, FILE *fp);
extern void KheTaskGroupsDebug(KHE_TASK_GROUPS task_groups, int verbosity,
  int indent, FILE *fp);

/* 11.5 Most-constrained-first assignment */
extern void KheMostConstrainedFirstAssignResources(KHE_TASKING tasking);

/* 11.6 Resource packing */
extern void KheResourcePackAssignResources(KHE_TASKING tasking);

/* 11.7 Split assignments */
extern void KheFindSplitResourceAssignments(KHE_TASKING tasking);

/* 11.8 Kempe resource assignments */
extern bool KheTaskKempeAssignResource(KHE_TASK task, KHE_RESOURCE r);

/* 11.9 Repairing resource assignments */
extern void KheEjectionChainRepairResources(KHE_TASKING tasking,
  bool preserve_invariant);
extern bool KheResourceRepairEventResourceMonitorAugment(KHE_EJECTOR ej,
  KHE_MONITOR d);
extern bool KheResourceRepairResourceMonitorAugment(KHE_EJECTOR ej,
  KHE_MONITOR d);
extern void KheTwoColourRepairSplitAssignments(KHE_TASKING tasking,
  bool preserve_invariant);
extern bool KheTwoColourReassign(KHE_SOLN soln, KHE_RESOURCE r1,
  KHE_RESOURCE r2, bool preserve_invariant);

/* 11.10 Putting it all together */
extern void KheTaskingAssignResources(KHE_TASKING tasking);


/*****************************************************************************/
/*                                                                           */
/*    Chapter 12.  General solvers                                           */
/*                                                                           */
/*****************************************************************************/

/* 12.1 Parallel solving */
extern KHE_SOLN KheParallelSolve(KHE_SOLN soln, int thread_count,
  KHE_GENERAL_SOLVER solver);

/* 12.2 A general solver */
extern KHE_SOLN KheGeneralSolve(KHE_SOLN soln);


/*****************************************************************************/
/*                                                                           */
/*    Chapter 13.  Ejection Chains                                           */
/*                                                                           */
/*****************************************************************************/

/* 13.2.1 Ejectors - construction and query */
extern KHE_EJECTOR KheEjectorMake(KHE_SOLN soln);
extern KHE_SOLN KheEjectorSoln(KHE_EJECTOR ej);
extern void KheEjectorDelete(KHE_EJECTOR ej);

extern void KheEjectorAddSchedule(KHE_EJECTOR ej, int max_depth,
  int max_disruption, bool may_revisit);
extern int KheEjectorScheduleCount(KHE_EJECTOR ej);
extern KHE_EJECTOR_SCHEDULE KheEjectorSchedule(KHE_EJECTOR ej, int i);
extern int KheEjectorScheduleMaxDepth(KHE_EJECTOR_SCHEDULE ejs);
extern int KheEjectorScheduleMaxDisruption(KHE_EJECTOR_SCHEDULE ejs);
extern bool KheEjectorScheduleMayRevisit(KHE_EJECTOR_SCHEDULE ejs);

extern void KheEjectorAddAugment(KHE_EJECTOR ej,
  KHE_MONITOR_TAG tag, KHE_EJECTOR_AUGMENT_FN augment_fn);
extern void KheEjectorAddGroupAugment(KHE_EJECTOR ej,
  int sub_tag, KHE_EJECTOR_AUGMENT_FN augment_fn);

extern void KheEjectorAddMonitorCostLimit(KHE_EJECTOR ej,
  KHE_MONITOR m, KHE_COST cost_limit);
extern int KheEjectorMonitorCostLimitCount(KHE_EJECTOR ej);
extern void KheEjectorMonitorCostLimit(KHE_EJECTOR ej, int i,
  KHE_MONITOR *m, KHE_COST *cost_limit);

/* 13.2.2 Ejectors - solving */
extern void KheEjectorSolve(KHE_EJECTOR ej, KHE_EJECTOR_SOLVE_TYPE solve_type,
  KHE_GROUP_MONITOR gm);
extern KHE_EJECTOR_SOLVE_TYPE KheEjectorSolveType(KHE_EJECTOR ej);
extern KHE_GROUP_MONITOR KheEjectorGroupMonitor(KHE_EJECTOR ej);
extern KHE_COST KheEjectorTargetCost(KHE_EJECTOR ej);
extern KHE_EJECTOR_SCHEDULE KheEjectorCurrSchedule(KHE_EJECTOR ej);
extern int KheEjectorCurrDepth(KHE_EJECTOR ej);
extern int KheEjectorCurrDisruption(KHE_EJECTOR ej);
extern bool KheEjectorSuccess(KHE_EJECTOR ej, KHE_TRACE tc, int disruption);

/* 13.3 An ejection chain algorithm for time repair */
extern bool KheTimeRepairEventMonitorAugment(KHE_EJECTOR ej, KHE_MONITOR d);
extern bool KheTimeRepairDemandMonitorAugment(KHE_EJECTOR ej, KHE_MONITOR d);

/* 13.3 Some helper augment functions */
extern bool KheEventAugment(KHE_EJECTOR ej, KHE_MONITOR d);
extern bool KheNodeEventAugment(KHE_EJECTOR ej, KHE_MONITOR d);
extern bool KheEventResourceAugment(KHE_EJECTOR ej, KHE_MONITOR d);
extern bool KheResourceAugment(KHE_EJECTOR ej, KHE_MONITOR d);
extern bool KheDemandAugment(KHE_EJECTOR ej, KHE_MONITOR d);
extern bool KheNodeDemandAugment(KHE_EJECTOR ej, KHE_MONITOR d);

#endif
