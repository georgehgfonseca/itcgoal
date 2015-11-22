
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
/*  FILE:         khe_workload.c                                             */
/*  DESCRIPTION:  KheSolnMatchingAddAllWorkloadRequirements()                */
/*                                                                           */
/*****************************************************************************/
#include "khe.h"
#include "m.h"

#define DEBUG1 0
#define DEBUG2 0
#define DEBUG3 0
#define DEBUG4 0

typedef MARRAY(KHE_CONSTRAINT) ARRAY_KHE_CONSTRAINT;
typedef MARRAY(KHE_EVENT) ARRAY_KHE_EVENT;


/*****************************************************************************/
/*                                                                           */
/*  KHE_EVENT_CLUSTER - a set of events known to run simultaneously          */
/*                                                                           */
/*****************************************************************************/

typedef struct khe_event_cluster {
  int				unique_id;		/* ID number for sort*/
  KHE_TIME			preassigned_time;	/* optional preasst  */
  ARRAY_KHE_EVENT		events;			/* simultaneous      */
} *KHE_EVENT_CLUSTER;

typedef MARRAY(KHE_EVENT_CLUSTER) ARRAY_KHE_EVENT_CLUSTER;


/*****************************************************************************/
/*                                                                           */
/*  Submodule "event clusters" (private)                                     */
/*                                                                           */
/*****************************************************************************/

/*****************************************************************************/
/*                                                                           */
/*  KHE_EVENT_CLUSTER KheEventClusterMake(KHE_EVENT e, int unique_id)        */
/*                                                                           */
/*  Make a new event cluster containing only e.                              */
/*                                                                           */
/*****************************************************************************/

static KHE_EVENT_CLUSTER KheEventClusterMake(KHE_EVENT e, int unique_id)
{
  KHE_EVENT_CLUSTER res;
  MAssert(e != NULL, "KheEventClusterMake internal error");
  MMake(res);
  res->unique_id = unique_id;
  res->preassigned_time = KheEventPreassignedTime(e);
  MArrayInit(res->events);
  MArrayAddLast(res->events, e);
  return res;
}


/*****************************************************************************/
/*                                                                           */
/*  void KheEventClusterFree(KHE_EVENT_CLUSTER ec)                           */
/*                                                                           */
/*  Free ec.                                                                 */
/*                                                                           */
/*****************************************************************************/

static void KheEventClusterFree(KHE_EVENT_CLUSTER ec)
{
  MArrayFree(ec->events);
  MFree(ec);
}


/*****************************************************************************/
/*                                                                           */
/*  void KheEventClusterMerge(KHE_EVENT_CLUSTER ec1, KHE_EVENT_CLUSTER ec2,  */
/*    ARRAY_KHE_EVENT_CLUSTER *clusters_by_event)                            */
/*                                                                           */
/*  Merge ec1 and ec2 (assumed distinct) and free ec2.                       */
/*                                                                           */
/*****************************************************************************/

static void KheEventClusterMerge(KHE_EVENT_CLUSTER ec1, KHE_EVENT_CLUSTER ec2,
  ARRAY_KHE_EVENT_CLUSTER *clusters_by_event)
{
  int i;  KHE_EVENT e;
  MArrayAppend(ec1->events, ec2->events, i);
  MArrayForEach(ec2->events, &e, &i)
    MArrayPut(*clusters_by_event, KheEventIndex(e), ec1);
  KheEventClusterFree(ec2);
}


/*****************************************************************************/
/*                                                                           */
/*  int KheEventClusterCmp(const void *t1, const void *t2)                   */
/*                                                                           */
/*  Comparison function for sorting an array of clusters into increasing     */
/*  order by memory address.                                                 */
/*                                                                           */
/*****************************************************************************/
static void KheEventClusterDebug(KHE_EVENT_CLUSTER ec, int indent, FILE *fp);

static int KheEventClusterCmp(const void *t1, const void *t2)
{
  KHE_EVENT_CLUSTER ec1 = * (KHE_EVENT_CLUSTER *) t1;
  KHE_EVENT_CLUSTER ec2 = * (KHE_EVENT_CLUSTER *) t2;
  if( DEBUG2 )
  {
    fprintf(stderr, "    KheEventClusterCmp returning %d\n",
      ec1->unique_id - ec2->unique_id);
    KheEventClusterDebug(ec1, 6, stderr);
    KheEventClusterDebug(ec2, 6, stderr);
  }
  return ec1->unique_id - ec2->unique_id;
}


/*****************************************************************************/
/*                                                                           */
/*  bool KheClusterContainsSpecialWorkload(KHE_EVENT_CLUSTER ec)             */
/*                                                                           */
/*  Return true if ec contains at least one event with a special workload.   */
/*                                                                           */
/*****************************************************************************/

static bool KheClusterContainsSpecialWorkload(KHE_EVENT_CLUSTER ec)
{
  int i, j;  KHE_EVENT e;  KHE_EVENT_RESOURCE er;
  MArrayForEach(ec->events, &e, &i)
    for( j = 0;  j < KheEventResourceCount(e);  j++ )
    {
      er = KheEventResource(e, j);
      if( KheEventResourceWorkload(er) != KheEventDuration(e) )
	return true;
    }
  return false;
}


/*****************************************************************************/
/*                                                                           */
/*  void KheEventClusterDebug(KHE_EVENT_CLUSTER ec, int indent, FILE *fp)    */
/*                                                                           */
/*  Debug print of ec (possibly NULL) onto fp with the given indent.         */
/*                                                                           */
/*****************************************************************************/

static void KheEventClusterDebug(KHE_EVENT_CLUSTER ec, int indent, FILE *fp)
{
  int i;  KHE_EVENT e;
  if( ec == NULL )
    fprintf(stderr, "%*sNULL\n", indent, "");
  else
  {
    fprintf(stderr, "%*s[ Event cluster %d", indent, "", ec->unique_id);
    if( ec->preassigned_time != NULL )
      fprintf(stderr, " (%s)", KheTimeId(ec->preassigned_time) == NULL ?
	"-" : KheTimeId(ec->preassigned_time));
    MArrayForEach(ec->events, &e, &i)
      fprintf(stderr, " %s", KheEventId(e) != NULL ? KheEventId(e) : "-");
    fprintf(stderr, " ]\n");
  }
}


/*****************************************************************************/
/*                                                                           */
/*  void KheEventClustersDebug(ARRAY_KHE_EVENT_CLUSTER *clusters,            */
/*    char *header, int indent, FILE *fp)                                    */
/*                                                                           */
/*  Debug print of an array of clusters onto fp at the given indent.         */
/*                                                                           */
/*****************************************************************************/

static void KheEventClustersDebug(ARRAY_KHE_EVENT_CLUSTER *clusters,
  char *header, int indent, FILE *fp)
{
  KHE_EVENT_CLUSTER ec;  int i;
  fprintf(stderr, "%*s[ %s (%d clusters)\n", indent, "",
    header != NULL ? header : "", MArraySize(*clusters));
  MArrayForEach(*clusters, &ec, &i)
    KheEventClusterDebug(ec, indent + 2, fp);
  fprintf(stderr, "%*s]\n", indent, "");
}


/*****************************************************************************/
/*                                                                           */
/*  void BuildClusters(KHE_INSTANCE ins, ARRAY_KHE_EVENT_CLUSTER *clusters)  */
/*                                                                           */
/*  Set *clusters to those clusters of the events of ins that contain        */
/*  special workloads.                                                       */
/*                                                                           */
/*  Implementation note.  Within this function, *clusters is used to hold    */
/*  clusters indexed by event.  At the end, clusters without special         */
/*  workloads are removed, and the array is uniqueified so that each         */
/*  cluster appears just once.                                               */
/*                                                                           */
/*****************************************************************************/

static void BuildClusters(KHE_INSTANCE ins, ARRAY_KHE_EVENT_CLUSTER *clusters)
{
  ARRAY_KHE_EVENT_CLUSTER clusters_by_time;  KHE_EVENT_CLUSTER ec, prev_ec;
  int i, j, k, index, unique_id;  KHE_EVENT e, prev_e;  KHE_EVENT_GROUP eg;
  KHE_CONSTRAINT c;  KHE_LINK_EVENTS_CONSTRAINT lec;

  if( DEBUG1 )
    fprintf(stderr, "[ BuildClusters(%s, clusters)\n",
      KheInstanceId(ins) == NULL ? "-" : KheInstanceId(ins));

  /* one cluster holding the events preassigned each time, as required */
  unique_id = 0;
  MArrayInit(clusters_by_time);
  MArrayFill(clusters_by_time, KheInstanceTimeCount(ins), NULL);
  for( i = 0;  i < KheInstanceEventCount(ins);  i++ )
  {
    e = KheInstanceEvent(ins, i);
    if( KheEventPreassignedTime(e) != NULL )
    {
      index = KheTimeIndex(KheEventPreassignedTime(e));
      ec = MArrayGet(clusters_by_time, index);
      if( ec == NULL )
	MArrayPut(clusters_by_time, index,
	  KheEventClusterMake(e, unique_id++));
      else
	MArrayAddLast(ec->events, e);
    }
  }

  /* one cluster for each event not yet clustered; build *clusters by event */
  MArrayInit(*clusters);
  for( i = 0;  i < KheInstanceEventCount(ins);  i++ )
  {
    e = KheInstanceEvent(ins, i);
    if( KheEventPreassignedTime(e) != NULL )
    {
      /* e is already in a cluster, get that cluster */
      index = KheTimeIndex(KheEventPreassignedTime(e));
      ec = MArrayGet(clusters_by_time, index);
    }
    else
    {
      /* make a fresh cluster for e */
      ec = KheEventClusterMake(e, unique_id++);
    }
    MArrayAddLast(*clusters, ec);
  }
  MArrayFree(clusters_by_time);

  /* merge clusters as indicated by required link events constraints */
  for( i = 0;  i < KheInstanceConstraintCount(ins);  i++ )
  {
    c = KheInstanceConstraint(ins, i);
    if( KheConstraintTag(c) == KHE_LINK_EVENTS_CONSTRAINT_TAG &&
	KheConstraintRequired(c) && KheConstraintWeight(c) > 0 )
    {
      lec = (KHE_LINK_EVENTS_CONSTRAINT) c;
      for( j = 0;  j < KheLinkEventsConstraintEventGroupCount(lec);  j++ )
      {
	eg = KheLinkEventsConstraintEventGroup(lec, j);
	for( k = 1;  k < KheEventGroupEventCount(eg);  k++ )
	{
	  prev_e = KheEventGroupEvent(eg, k - 1);
	  prev_ec = MArrayGet(*clusters, KheEventIndex(prev_e));
	  e = KheEventGroupEvent(eg, k);
	  ec = MArrayGet(*clusters, KheEventIndex(e));
	  if( ec != prev_ec )
            KheEventClusterMerge(prev_ec, ec, clusters);
	}
      }
    }
  }

  /* uniqueify clusters */
  if( DEBUG1 )
    KheEventClustersDebug(clusters, "Clusters before uniquefying", 0, stderr);
  MArraySort(*clusters, &KheEventClusterCmp);
  if( DEBUG1 )
    KheEventClustersDebug(clusters, "Clusters after sorting", 0, stderr);
  MArraySortUnique(*clusters, &KheEventClusterCmp);
  if( DEBUG1 )
    KheEventClustersDebug(clusters, "Clusters after uniquefying", 0, stderr);

  /* remove clusters that have no special workloads */
  MArrayForEach(*clusters, &ec, &i)
    if( !KheClusterContainsSpecialWorkload(ec) )
    {
      MArrayRemove(*clusters, i);
      i--;
      KheEventClusterFree(ec);
    }
  if( DEBUG1 )
    KheEventClustersDebug(clusters, "Final clusters", 0, stderr);
  if( DEBUG1 )
    fprintf(stderr, "] BuildClusters returning\n");
}


/*****************************************************************************/
/*                                                                           */
/*  int KheEventClusterResourceAdjustment(KHE_EVENT_CLUSTER ec,              */
/*    KHE_RESOURCE r)                                                        */
/*                                                                           */
/*  Return the adjustment needed to r's overall workload limit to take       */
/*  account of event cluster ec.  Precisely, this is as follows:             */
/*                                                                           */
/*    * If r must be assigned to at least one event resource of an event     */
/*      of ec, then return the sum of d(e) - w(e) over all those events      */
/*      e of ec that r must be assigned to.                                  */
/*                                                                           */
/*    * Otherwise, if r may be assigned to at least one event resource of    */
/*      an event of ec, then return the maximum value of d(e) - w(e) over    */
/*      all those events e of ec that r may be assigned to.                  */
/*                                                                           */
/*    * Otherwise (if r may not be assigned to any event of ec) return 0.    */
/*                                                                           */
/*  Implementation note.  This function tests assignability of r to each     */
/*  event resource separately.  This is correct if r cannot be assigned to   */
/*  any event, but to distinguish "must" from "may" it would be better to    */
/*  match all the event resources against the resources of the instance      */
/*  with and without r.  However, that would be slower and much more work,   */
/*  probably not worth the trouble given that the return value is much the   */
/*  same in the two cases.                                                   */
/*                                                                           */
/*****************************************************************************/

static int KheEventClusterResourceAdjustment(KHE_EVENT_CLUSTER ec,
  KHE_RESOURCE r)
{
  KHE_EVENT e;  bool must, may;  int i, j, must_sum, may_max;
  KHE_RESOURCE_GROUP domain;  KHE_EVENT_RESOURCE er;
  must_sum = may_max = 0;  must = may = false;
  MArrayForEach(ec->events, &e, &i)
    for( j = 0;  j < KheEventResourceCount(e);  j++ )
    {
      er = KheEventResource(e, j);
      domain = KheEventResourceHardDomain(er);
      if( KheResourceGroupContains(domain, r) )
      {
	if( KheResourceGroupResourceCount(domain) == 1 )
	{
	  /* r must be assigned to er */
	  must_sum += (KheEventDuration(e) - KheEventResourceWorkload(er));
	  must = true;
	}
	else
	{
	  /* r may be assigned to er */
	  if( KheEventDuration(e) - KheEventResourceWorkload(er) > may_max )
	    may_max = KheEventDuration(e) - KheEventResourceWorkload(er);
	  may = true;
	}
      }
    }
  if( DEBUG3 )
  {
    fprintf(stderr, "    KheEventClusterResourceAdjustment(ec, %s): %s %d for ",
      KheResourceId(r) == NULL ? "-" : KheResourceId(r),
    must ? "must" : may ? "may" : "not", must ? must_sum : may ? may_max : 0);
    KheEventClusterDebug(ec, 0, stderr);
  }
  return must ? must_sum : may ? may_max : 0;
}


/*****************************************************************************/
/*                                                                           */
/*  Submodule "main algorithm"                                               */
/*                                                                           */
/*****************************************************************************/

/*****************************************************************************/
/*                                                                           */
/*  int KheConstraintCmp(const void *t1, const void *t2)                     */
/*                                                                           */
/*  Comparison function for sorting an array of constraints by decreasing    */
/*  combined weight.                                                         */
/*                                                                           */
/*****************************************************************************/

static int KheConstraintCmp(const void *t1, const void *t2)
{
  KHE_CONSTRAINT c1 = * (KHE_CONSTRAINT *) t1;
  KHE_CONSTRAINT c2 = * (KHE_CONSTRAINT *) t2;
  int cost_cmp = KheCostCmp(KheConstraintCombinedWeight(c2),
    KheConstraintCombinedWeight(c1));
  if( cost_cmp != 0 )
    return cost_cmp;
  else
    return KheConstraintIndex(c1) - KheConstraintIndex(c2);
}


/*****************************************************************************/
/*                                                                           */
/*  void KheWorkloadAvoidTimeGroup(KHE_SOLN soln,                            */
/*    KHE_RESOURCE r, KHE_TIME_GROUP tg)                                     */
/*                                                                           */
/*  Add requirements that cause r to avoid tg completely.                    */
/*                                                                           */
/*****************************************************************************/

static void KheWorkloadAvoidTimeGroup(KHE_SOLN soln,
  KHE_RESOURCE r, KHE_TIME_GROUP tg)
{
  KHE_TIME t;  int i;
  for( i = 0;  i < KheTimeGroupTimeCount(tg);  i++ )
  {
    t = KheTimeGroupTime(tg, i);
    KheSolnMatchingAddWorkloadRequirement(soln, r, 0,
      KheTimeSingletonTimeGroup(t));
  }
}


/*****************************************************************************/
/*                                                                           */
/*  void KheWorkloadAddAvoidUnavailableTimesRequirement(KHE_SOLN soln,       */
/*    KHE_RESOURCE r, KHE_AVOID_UNAVAILABLE_TIMES_CONSTRAINT c)              */
/*                                                                           */
/*  Add workload requirements for c to r's workload requirements.            */
/*                                                                           */
/*****************************************************************************/

static void KheWorkloadAddAvoidUnavailableTimesRequirement(KHE_SOLN soln,
  KHE_RESOURCE r, KHE_AVOID_UNAVAILABLE_TIMES_CONSTRAINT c)
{
  KHE_TIME_GROUP tg;  KHE_TIME t;  int i;

  /* add demands for 0 of the times of each time group */
  for( i = 0;  i < KheAvoidUnavailableTimesConstraintTimeGroupCount(c);  i++ )
  {
    tg = KheAvoidUnavailableTimesConstraintTimeGroup(c, i);
    KheWorkloadAvoidTimeGroup(soln, r, tg);
  }

  /* add demands for 0 of each time */
  for( i = 0;  i < KheAvoidUnavailableTimesConstraintTimeCount(c);  i++ )
  {
    t = KheAvoidUnavailableTimesConstraintTime(c, i);
    KheSolnMatchingAddWorkloadRequirement(soln, r, 0,
      KheTimeSingletonTimeGroup(t));
  }
}


/*****************************************************************************/
/*                                                                           */
/*  void KheWorkloadAddLimitBusyTimesRequirement(KHE_SOLN soln,              */
/*    KHE_RESOURCE r, KHE_LIMIT_BUSY_TIMES_CONSTRAINT c)                     */
/*                                                                           */
/*  Add workload requirements for c to r's workload requirements.            */
/*                                                                           */
/*****************************************************************************/

static void KheWorkloadAddLimitBusyTimesRequirement(KHE_SOLN soln,
  KHE_RESOURCE r, KHE_LIMIT_BUSY_TIMES_CONSTRAINT c)
{
  KHE_TIME_GROUP tg;  int i, limit;
  limit = KheLimitBusyTimesConstraintMaximum(c);
  for( i = 0;  i < KheLimitBusyTimesConstraintTimeGroupCount(c);  i++ )
  {
    tg = KheLimitBusyTimesConstraintTimeGroup(c, i);
    if( DEBUG4 )
    {
      fprintf(stderr, "  limit busy times constraint %d: ", i);
      KheTimeGroupDebug(tg, 3, 0, stderr);
    }
    if( limit == 0 )
      KheWorkloadAvoidTimeGroup(soln, r, tg);
    else if( limit < KheTimeGroupTimeCount(tg) )
      KheSolnMatchingAddWorkloadRequirement(soln, r, limit, tg);
  }
}


/*****************************************************************************/
/*                                                                           */
/*  void KheWorkloadAddLimitWorkloadRequirement(KHE_SOLN soln,               */
/*    KHE_RESOURCE r, KHE_LIMIT_WORKLOAD_CONSTRAINT c,                       */
/*    ARRAY_KHE_EVENT_CLUSTER *clusters)                                     */
/*                                                                           */
/*  Add workload requirements for c to r's workload requirements.            */
/*                                                                           */
/*****************************************************************************/

static void KheWorkloadAddLimitWorkloadRequirement(KHE_SOLN soln,
  KHE_RESOURCE r, KHE_LIMIT_WORKLOAD_CONSTRAINT c,
  ARRAY_KHE_EVENT_CLUSTER *clusters)
{
  int limit, i;  KHE_EVENT_CLUSTER ec;  KHE_TIME_GROUP full_tg;
  limit = KheLimitWorkloadConstraintMaximum(c);
  full_tg = KheInstanceFullTimeGroup(KheResourceInstance(r));
  if( DEBUG3 )
  {
    fprintf(stderr, "[ KheWorkloadAddLimitWorkloadRequirement(%s, %d c)\n",
      KheResourceId(r) == NULL ? "-" : KheResourceId(r), limit);
    KheTimeGroupDebug(full_tg, 3, 2, stderr);
  }
  MArrayForEach(*clusters, &ec, &i)
    limit += KheEventClusterResourceAdjustment(ec, r);
  if( limit < 0 )
    limit = 0;
  if( limit < KheTimeGroupTimeCount(full_tg) )
    KheSolnMatchingAddWorkloadRequirement(soln, r, limit, full_tg);
  if( DEBUG3 )
    fprintf(stderr, "] KheWorkloadAddLimitWorkloadRequirement (final: %d c)\n",
      limit);
}


/*****************************************************************************/
/*                                                                           */
/*  void KheWorkloadAddRequirements(KHE_SOLN soln,                           */
/*    KHE_RESOURCE r, ARRAY_KHE_EVENT_CLUSTER *clusters)                     */
/*                                                                           */
/*  Add workload requirements for resource r.                                */
/*                                                                           */
/*****************************************************************************/

static void KheWorkloadAddRequirements(KHE_SOLN soln,
  KHE_RESOURCE r, ARRAY_KHE_EVENT_CLUSTER *clusters)
{
  ARRAY_KHE_CONSTRAINT constraints;  KHE_CONSTRAINT c;  int i;
  if( DEBUG4 )
    fprintf(stderr, "[ KheWorkloadAddRequirements(m, %s)\n",
      KheResourceId(r) == NULL ? "-" : KheResourceId(r));

  /* select the relevant constraints and sort by decreasing weight */
  MArrayInit(constraints);
  for( i = 0;  i < KheResourceConstraintCount(r);  i++ )
  {
    c = KheResourceConstraint(r, i);
    if( KheConstraintRequired(c) && KheConstraintWeight(c) > 0 &&
	(
	  KheConstraintTag(c) == KHE_AVOID_UNAVAILABLE_TIMES_CONSTRAINT_TAG ||
	  KheConstraintTag(c) == KHE_LIMIT_BUSY_TIMES_CONSTRAINT_TAG ||
	  KheConstraintTag(c) == KHE_LIMIT_WORKLOAD_CONSTRAINT_TAG )
	)
      MArrayAddLast(constraints, c);
  }
  MArraySort(constraints, &KheConstraintCmp);

  /* add the requirements, taking constraints in decreasing weight order */
  KheSolnMatchingBeginWorkloadRequirements(soln, r);
  MArrayForEach(constraints, &c, &i)
    switch( KheConstraintTag(c) )
    {
      case KHE_AVOID_UNAVAILABLE_TIMES_CONSTRAINT_TAG:

	KheWorkloadAddAvoidUnavailableTimesRequirement(soln, r,
	  (KHE_AVOID_UNAVAILABLE_TIMES_CONSTRAINT) c);
	break;

      case KHE_LIMIT_BUSY_TIMES_CONSTRAINT_TAG:

	KheWorkloadAddLimitBusyTimesRequirement(soln, r,
	  (KHE_LIMIT_BUSY_TIMES_CONSTRAINT) c);
	break;

      case KHE_LIMIT_WORKLOAD_CONSTRAINT_TAG:

	KheWorkloadAddLimitWorkloadRequirement(soln, r,
	  (KHE_LIMIT_WORKLOAD_CONSTRAINT) c, clusters);
	break;

      default:

	MAssert(false, "KheWorkloadAddRequirements internal error");

    }
  KheSolnMatchingEndWorkloadRequirements(soln, r);
  if( DEBUG4 )
    fprintf(stderr, "] KheWorkloadAddRequirements returning\n");
}


/*****************************************************************************/
/*                                                                           */
/*  void KheSolnMatchingAddAllWorkloadRequirements(KHE_SOLN soln)            */
/*                                                                           */
/*  Add all workload requirements to soln.                                   */
/*                                                                           */
/*****************************************************************************/

void KheSolnMatchingAddAllWorkloadRequirements(KHE_SOLN soln)
{
  int i;  KHE_INSTANCE ins;  ARRAY_KHE_EVENT_CLUSTER clusters;
  ins = KheSolnInstance(soln);
  BuildClusters(ins, &clusters);
  for( i = 0;  i < KheInstanceResourceCount(ins);  i++ )
    KheWorkloadAddRequirements(soln, KheInstanceResource(ins, i), &clusters);
}
