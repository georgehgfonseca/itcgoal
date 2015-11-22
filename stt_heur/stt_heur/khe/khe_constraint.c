
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
/*  FILE:         khe_constraint.c                                           */
/*  DESCRIPTION:  A constraint (abstract supertype)                          */
/*                                                                           */
/*****************************************************************************/
#include "khe_interns.h"


/*****************************************************************************/
/*                                                                           */
/*  KHE_CONSTRAINT - a constraint (abstract supertype)                       */
/*                                                                           */
/*****************************************************************************/

struct khe_constraint_rec {
  void				*back;			/* back pointer      */
  KHE_CONSTRAINT_TAG		tag;			/* type tag          */
  KHE_INSTANCE			instance;		/* enclosing instance*/
  char				*id;			/* id                */
  char				*name;			/* name              */
  bool				required;		/* hard or soft      */
  int				weight;			/* Weight            */
  KHE_COST			combined_weight;	/* Weight            */
  KHE_COST_FUNCTION		cost_function;		/* CostFunction      */
  int				index;			/* index number      */
};


/*****************************************************************************/
/*                                                                           */
/*  Submodule "query (not construction, since abstract supertype)"           */
/*                                                                           */
/*****************************************************************************/

/*****************************************************************************/
/*                                                                           */
/*  void KheConstraintSetBack(KHE_CONSTRAINT c, void *back)                  */
/*                                                                           */
/*  Set the back pointer of c.                                               */
/*                                                                           */
/*****************************************************************************/

void KheConstraintSetBack(KHE_CONSTRAINT c, void *back)
{
  MAssert(!KheInstanceComplete(c->instance),
    "KheConstraintSetBack called after KheInstanceMakeEnd");
  c->back = back;
}


/*****************************************************************************/
/*                                                                           */
/*  void *KheConstraintBack(KHE_CONSTRAINT c)                                */
/*                                                                           */
/*  Return the back pointer of c.                                            */
/*                                                                           */
/*****************************************************************************/

void *KheConstraintBack(KHE_CONSTRAINT c)
{
  return c->back;
}


/*****************************************************************************/
/*                                                                           */
/*  KHE_INSTANCE KheConstraintInstance(KHE_CONSTRAINT c)                     */
/*                                                                           */
/*  Return the instance attribute of c.                                      */
/*                                                                           */
/*****************************************************************************/

KHE_INSTANCE KheConstraintInstance(KHE_CONSTRAINT c)
{
  return c->instance;
}


/*****************************************************************************/
/*                                                                           */
/*  char *KheConstraintId(KHE_CONSTRAINT c)                                  */
/*                                                                           */
/*  Return the id attribute of c.                                            */
/*                                                                           */
/*****************************************************************************/

char *KheConstraintId(KHE_CONSTRAINT c)
{
  return c->id;
}


/*****************************************************************************/
/*                                                                           */
/*  char *KheConstraintName(KHE_CONSTRAINT c)                                */
/*                                                                           */
/*  Return the name attribute of c.                                          */
/*                                                                           */
/*****************************************************************************/

char *KheConstraintName(KHE_CONSTRAINT c)
{
  return c->name;
}


/*****************************************************************************/
/*                                                                           */
/*  bool KheConstraintRequired(KHE_CONSTRAINT c)                             */
/*                                                                           */
/*  Return the required attribute of c.                                      */
/*                                                                           */
/*****************************************************************************/

bool KheConstraintRequired(KHE_CONSTRAINT c)
{
  return c->required;
}


/*****************************************************************************/
/*                                                                           */
/*  KHE_STRENGTH KheConstraintStrength(KHE_CONSTRAINT c)                     */
/*                                                                           */
/*  Return the strength attribute of c.                                      */
/*                                                                           */
/*****************************************************************************/

/* ***
KHE_STRENGTH KheConstraintStrength(KHE_CONSTRAINT c)
{
  return c->strength;
}
*** */


/*****************************************************************************/
/*                                                                           */
/*  int KheConstraintWeight(KHE_CONSTRAINT c)                                */
/*                                                                           */
/*  Return the weight of c.                                                  */
/*                                                                           */
/*****************************************************************************/

int KheConstraintWeight(KHE_CONSTRAINT c)
{
  return c->weight;
}


/*****************************************************************************/
/*                                                                           */
/*  KHE_COST KheConstraintCombinedWeight(KHE_CONSTRAINT c)                   */
/*                                                                           */
/*  Return the combined weight of c.                                         */
/*                                                                           */
/*****************************************************************************/

KHE_COST KheConstraintCombinedWeight(KHE_CONSTRAINT c)
{
  return c->combined_weight;
}


/*****************************************************************************/
/*                                                                           */
/*  KHE_COST_FUNCTION KheConstraintCostFunction(KHE_CONSTRAINT c)            */
/*                                                                           */
/*  Return the cost function of c.                                           */
/*                                                                           */
/*****************************************************************************/

KHE_COST_FUNCTION KheConstraintCostFunction(KHE_CONSTRAINT c)
{
  return c->cost_function;
}


/*****************************************************************************/
/*                                                                           */
/*  int KheConstraintIndex(KHE_CONSTRAINT c)                                 */
/*                                                                           */
/*  Return the index number of c.                                            */
/*                                                                           */
/*****************************************************************************/

int KheConstraintIndex(KHE_CONSTRAINT c)
{
  return c->index;
}


/*****************************************************************************/
/*                                                                           */
/*  KHE_CONSTRAINT_TAG KheConstraintTag(KHE_CONSTRAINT c)                    */
/*                                                                           */
/*  Return the tag of c, indicating which kind of constraint it is.          */
/*                                                                           */
/*****************************************************************************/

KHE_CONSTRAINT_TAG KheConstraintTag(KHE_CONSTRAINT c)
{
  return c->tag;
}


/*****************************************************************************/
/*                                                                           */
/*  int KheConstraintAppliesToCount(KHE_CONSTRAINT c)                        */
/*                                                                           */
/*  Return the number of points of application of c.                         */
/*                                                                           */
/*****************************************************************************/

int KheConstraintAppliesToCount(KHE_CONSTRAINT c)
{
  switch( c->tag )
  {
    case KHE_ASSIGN_RESOURCE_CONSTRAINT_TAG:
      return KheAssignResourceConstraintAppliesToCount(
	(KHE_ASSIGN_RESOURCE_CONSTRAINT) c);

    case KHE_ASSIGN_TIME_CONSTRAINT_TAG:
      return KheAssignTimeConstraintAppliesToCount(
	(KHE_ASSIGN_TIME_CONSTRAINT) c);

    case KHE_SPLIT_EVENTS_CONSTRAINT_TAG:
      return KheSplitEventsConstraintAppliesToCount(
	(KHE_SPLIT_EVENTS_CONSTRAINT) c);

    case KHE_DISTRIBUTE_SPLIT_EVENTS_CONSTRAINT_TAG:
      return KheDistributeSplitEventsConstraintAppliesToCount(
	(KHE_DISTRIBUTE_SPLIT_EVENTS_CONSTRAINT) c);

    case KHE_PREFER_RESOURCES_CONSTRAINT_TAG:
      return KhePreferResourcesConstraintAppliesToCount(
	(KHE_PREFER_RESOURCES_CONSTRAINT) c);

    case KHE_PREFER_TIMES_CONSTRAINT_TAG:
      return KhePreferTimesConstraintAppliesToCount(
	(KHE_PREFER_TIMES_CONSTRAINT) c);

    case KHE_AVOID_SPLIT_ASSIGNMENTS_CONSTRAINT_TAG:
      return KheAvoidSplitAssignmentsConstraintAppliesToCount(
	(KHE_AVOID_SPLIT_ASSIGNMENTS_CONSTRAINT) c);

    case KHE_SPREAD_EVENTS_CONSTRAINT_TAG:
      return KheSpreadEventsConstraintAppliesToCount(
	(KHE_SPREAD_EVENTS_CONSTRAINT) c);

    case KHE_LINK_EVENTS_CONSTRAINT_TAG:
      return KheLinkEventsConstraintAppliesToCount(
	(KHE_LINK_EVENTS_CONSTRAINT) c);

    case KHE_AVOID_CLASHES_CONSTRAINT_TAG:
      return KheAvoidClashesConstraintAppliesToCount(
	(KHE_AVOID_CLASHES_CONSTRAINT) c);

    case KHE_AVOID_UNAVAILABLE_TIMES_CONSTRAINT_TAG:
      return KheAvoidUnavailableTimesConstraintAppliesToCount(
	(KHE_AVOID_UNAVAILABLE_TIMES_CONSTRAINT) c);

    case KHE_LIMIT_IDLE_TIMES_CONSTRAINT_TAG:
      return KheLimitIdleTimesConstraintAppliesToCount(
	(KHE_LIMIT_IDLE_TIMES_CONSTRAINT) c);

    case KHE_CLUSTER_BUSY_TIMES_CONSTRAINT_TAG:
      return KheClusterBusyTimesConstraintAppliesToCount(
	(KHE_CLUSTER_BUSY_TIMES_CONSTRAINT) c);

    case KHE_LIMIT_BUSY_TIMES_CONSTRAINT_TAG:
      return KheLimitBusyTimesConstraintAppliesToCount(
	(KHE_LIMIT_BUSY_TIMES_CONSTRAINT) c);

    case KHE_LIMIT_WORKLOAD_CONSTRAINT_TAG:
      return KheLimitWorkloadConstraintAppliesToCount(
	(KHE_LIMIT_WORKLOAD_CONSTRAINT) c);

    default:
      MAssert(false, "KheConstraintFinalize internal error");
      return 0;  /* keep compiler happy */
  }
}


/*****************************************************************************/
/*                                                                           */
/*  char *KheConstraintTagShow(KHE_CONSTRAINT_TAG tag)                       */
/*                                                                           */
/*  Show the name of tag.                                                    */
/*                                                                           */
/*****************************************************************************/

char *KheConstraintTagShow(KHE_CONSTRAINT_TAG tag)
{
  switch( tag )
  {
    case KHE_ASSIGN_RESOURCE_CONSTRAINT_TAG:
      return "AssignResourceConstraint";

    case KHE_ASSIGN_TIME_CONSTRAINT_TAG:
      return "AssignTimeConstraint";

    case KHE_SPLIT_EVENTS_CONSTRAINT_TAG:
      return "SplitEventsConstraint";

    case KHE_DISTRIBUTE_SPLIT_EVENTS_CONSTRAINT_TAG:
      return "DistributeSplitEventsConstraint";

    case KHE_PREFER_RESOURCES_CONSTRAINT_TAG:
      return "PreferResourcesConstraint";

    case KHE_PREFER_TIMES_CONSTRAINT_TAG:
      return "PreferTimesConstraint";

    case KHE_AVOID_SPLIT_ASSIGNMENTS_CONSTRAINT_TAG:
      return "AvoidSplitAssignmentsConstraint";

    case KHE_SPREAD_EVENTS_CONSTRAINT_TAG:
      return "SpreadEventsConstraint";

    case KHE_LINK_EVENTS_CONSTRAINT_TAG:
      return "LinkEventsConstraint";

    case KHE_AVOID_CLASHES_CONSTRAINT_TAG:
      return "AvoidClashesConstraint";

    case KHE_AVOID_UNAVAILABLE_TIMES_CONSTRAINT_TAG:
      return "AvoidUnavailableTimesConstraint";

    case KHE_LIMIT_IDLE_TIMES_CONSTRAINT_TAG:
      return "LimitIdleTimesConstraint";

    case KHE_CLUSTER_BUSY_TIMES_CONSTRAINT_TAG:
      return "ClusterBusyTimesConstraint";

    case KHE_LIMIT_BUSY_TIMES_CONSTRAINT_TAG:
      return "LimitBusyTimesConstraint";

    case KHE_LIMIT_WORKLOAD_CONSTRAINT_TAG:
      return "LimitWorkloadConstraint";

    default: return "?";
  }
}


/*****************************************************************************/
/*                                                                           */
/*  char *KheConstraintTagShowSpaced(KHE_CONSTRAINT_TAG tag)                 */
/*                                                                           */
/*  Show the name of tag.                                                    */
/*                                                                           */
/*****************************************************************************/

char *KheConstraintTagShowSpaced(KHE_CONSTRAINT_TAG tag)
{
  switch( tag )
  {
    case KHE_ASSIGN_RESOURCE_CONSTRAINT_TAG:
      return "Assign Resource Constraint";

    case KHE_ASSIGN_TIME_CONSTRAINT_TAG:
      return "Assign Time Constraint";

    case KHE_SPLIT_EVENTS_CONSTRAINT_TAG:
      return "Split Events Constraint";

    case KHE_DISTRIBUTE_SPLIT_EVENTS_CONSTRAINT_TAG:
      return "Distribute Split Events Constraint";

    case KHE_PREFER_RESOURCES_CONSTRAINT_TAG:
      return "Prefer Resources Constraint";

    case KHE_PREFER_TIMES_CONSTRAINT_TAG:
      return "Prefer Times Constraint";

    case KHE_AVOID_SPLIT_ASSIGNMENTS_CONSTRAINT_TAG:
      return "Avoid Split Assignments Constraint";

    case KHE_SPREAD_EVENTS_CONSTRAINT_TAG:
      return "Spread Events Constraint";

    case KHE_LINK_EVENTS_CONSTRAINT_TAG:
      return "Link Events Constraint";

    case KHE_AVOID_CLASHES_CONSTRAINT_TAG:
      return "Avoid Clashes Constraint";

    case KHE_AVOID_UNAVAILABLE_TIMES_CONSTRAINT_TAG:
      return "Avoid Unavailable Times Constraint";

    case KHE_LIMIT_IDLE_TIMES_CONSTRAINT_TAG:
      return "Limit Idle Times Constraint";

    case KHE_CLUSTER_BUSY_TIMES_CONSTRAINT_TAG:
      return "Cluster Busy Times Constraint";

    case KHE_LIMIT_BUSY_TIMES_CONSTRAINT_TAG:
      return "Limit Busy Times Constraint";

    case KHE_LIMIT_WORKLOAD_CONSTRAINT_TAG:
      return "Limit Workload Constraint";

    default: return "?";
  }
}


/*****************************************************************************/
/*                                                                           */
/*  KHE_CONSTRAINT_TAG KheStringToConstraintTag(char *str)                   */
/*                                                                           */
/*  Return the constraint tag corresponding with str.                        */
/*                                                                           */
/*****************************************************************************/

KHE_CONSTRAINT_TAG KheStringToConstraintTag(char *str)
{
  switch( str[0] )
  {
    case 'A':

      if( strcmp(str, "AssignResourceConstraint") == 0 )
	return KHE_ASSIGN_RESOURCE_CONSTRAINT_TAG;
      else if( strcmp(str, "AssignTimeConstraint") == 0 )
	return KHE_ASSIGN_TIME_CONSTRAINT_TAG;
      else if( strcmp(str, "AvoidClashesConstraint") == 0 )
	return KHE_AVOID_CLASHES_CONSTRAINT_TAG;
      else if( strcmp(str, "AvoidSplitAssignmentsConstraint") == 0 )
	return KHE_AVOID_SPLIT_ASSIGNMENTS_CONSTRAINT_TAG;
      else if( strcmp(str, "AvoidUnavailableTimesConstraint") == 0 )
	return KHE_AVOID_UNAVAILABLE_TIMES_CONSTRAINT_TAG;
      break;

    case 'C':

      if( strcmp(str, "ClusterBusyTimesConstraint") == 0 )
	return KHE_CLUSTER_BUSY_TIMES_CONSTRAINT_TAG;
      break;

    case 'D':

      if( strcmp(str, "DistributeSplitEventsConstraint") == 0 )
	return KHE_DISTRIBUTE_SPLIT_EVENTS_CONSTRAINT_TAG;
      break;

    case 'L':

      if( strcmp(str, "LimitBusyTimesConstraint") == 0 )
	return KHE_LIMIT_BUSY_TIMES_CONSTRAINT_TAG;
      else if( strcmp(str, "LimitIdleTimesConstraint") == 0 )
	return KHE_LIMIT_IDLE_TIMES_CONSTRAINT_TAG;
      else if( strcmp(str, "LimitWorkloadConstraint") == 0 )
	return KHE_LIMIT_WORKLOAD_CONSTRAINT_TAG;
      else if( strcmp(str, "LinkEventsConstraint") == 0 )
	return KHE_LINK_EVENTS_CONSTRAINT_TAG;
      break;

    case 'P':

      if( strcmp(str, "PreferResourcesConstraint") == 0 )
	return KHE_PREFER_RESOURCES_CONSTRAINT_TAG;
      else if( strcmp(str, "PreferTimesConstraint") == 0 )
	return KHE_PREFER_TIMES_CONSTRAINT_TAG;
      break;

    case 'S':
      if( strcmp(str, "SplitEventsConstraint") == 0 )
	return KHE_SPLIT_EVENTS_CONSTRAINT_TAG;
      else if( strcmp(str, "SpreadEventsConstraint") == 0 )
	return KHE_SPREAD_EVENTS_CONSTRAINT_TAG;
      break;
  }
  MAssert(false, "KheStringToConstraintTag: unknown str");
  return 0;  /* keep compiler happy */
}


/*****************************************************************************/
/*                                                                           */
/*  void KheConstraintDebug(KHE_CONSTRAINT c, int verbosity,                 */
/*    int indent, FILE *fp)                                                  */
/*                                                                           */
/*  Debug print of c onto fp with the given verbosity and indent.            */
/*                                                                           */
/*****************************************************************************/

void KheConstraintDebug(KHE_CONSTRAINT c, int verbosity,
  int indent, FILE *fp)
{
  if( verbosity >= 1 )
  {
    if( indent >= 0 )
      fprintf(fp, "%*s", indent, "");
    fprintf(fp, "%s(%s, %s:%d)", KheConstraintTagShow(c->tag),
      c->id != NULL ? c->id : "-", c->required ? "true" : "false", c->weight);
    if( indent >= 0 )
      fprintf(fp, "\n");
  }
}


/*****************************************************************************/
/*                                                                           */
/*  Submodule "upcasts"                                                      */
/*                                                                           */
/*****************************************************************************/

KHE_CONSTRAINT KheFromAssignResourceConstraint(KHE_ASSIGN_RESOURCE_CONSTRAINT c)
{
  return (KHE_CONSTRAINT) c;
}

KHE_CONSTRAINT KheFromAssignTimeConstraint(KHE_ASSIGN_TIME_CONSTRAINT c)
{
  return (KHE_CONSTRAINT) c;
}

KHE_CONSTRAINT KheFromSplitEventsConstraint(KHE_SPLIT_EVENTS_CONSTRAINT c)
{
  return (KHE_CONSTRAINT) c;
}

KHE_CONSTRAINT KheFromDistributeSplitEventsConstraint(
  KHE_DISTRIBUTE_SPLIT_EVENTS_CONSTRAINT c)
{
  return (KHE_CONSTRAINT) c;
}

KHE_CONSTRAINT KheFromPreferResourcesConstraint(
  KHE_PREFER_RESOURCES_CONSTRAINT c)
{
  return (KHE_CONSTRAINT) c;
}

KHE_CONSTRAINT KheFromPreferTimesConstraint(KHE_PREFER_TIMES_CONSTRAINT c)
{
  return (KHE_CONSTRAINT) c;
}

KHE_CONSTRAINT KheFromAvoidSplitAssignmentsConstraint(
  KHE_AVOID_SPLIT_ASSIGNMENTS_CONSTRAINT c)
{
  return (KHE_CONSTRAINT) c;
}

KHE_CONSTRAINT KheFromSpreadEventsConstraint(KHE_SPREAD_EVENTS_CONSTRAINT c)
{
  return (KHE_CONSTRAINT) c;
}

KHE_CONSTRAINT KheFromLinkEventsConstraint(KHE_LINK_EVENTS_CONSTRAINT c)
{
  return (KHE_CONSTRAINT) c;
}

KHE_CONSTRAINT KheFromAvoidClashesConstraint(KHE_AVOID_CLASHES_CONSTRAINT c)
{
  return (KHE_CONSTRAINT) c;
}

KHE_CONSTRAINT KheFromAvoidUnavailableTimesConstraint(
  KHE_AVOID_UNAVAILABLE_TIMES_CONSTRAINT c)
{
  return (KHE_CONSTRAINT) c;
}

KHE_CONSTRAINT KheFromLimitIdleTimesConstraint(
  KHE_LIMIT_IDLE_TIMES_CONSTRAINT c)
{
  return (KHE_CONSTRAINT) c;
}

KHE_CONSTRAINT KheFromClusterBusyTimesConstraint(
  KHE_CLUSTER_BUSY_TIMES_CONSTRAINT c)
{
  return (KHE_CONSTRAINT) c;
}

KHE_CONSTRAINT KheFromLimitBusyTimesConstraint(
  KHE_LIMIT_BUSY_TIMES_CONSTRAINT c)
{
  return (KHE_CONSTRAINT) c;
}

KHE_CONSTRAINT KheFromLimitWorkloadConstraint(KHE_LIMIT_WORKLOAD_CONSTRAINT c)
{
  return (KHE_CONSTRAINT) c;
}


/*****************************************************************************/
/*                                                                           */
/*  Submodule "downcasts"                                                    */
/*                                                                           */
/*****************************************************************************/

KHE_ASSIGN_RESOURCE_CONSTRAINT KheToAssignResourceConstraint(KHE_CONSTRAINT c)
{
  MAssert(c->tag == KHE_ASSIGN_RESOURCE_CONSTRAINT_TAG,
    "KheToAssignResourceConstraint: downcast failed");
  return (KHE_ASSIGN_RESOURCE_CONSTRAINT) c;
}

KHE_ASSIGN_TIME_CONSTRAINT KheToAssignTimeConstraint(KHE_CONSTRAINT c)
{
  MAssert(c->tag == KHE_ASSIGN_TIME_CONSTRAINT_TAG,
    "KheToAssignTimeConstraint: downcast failed");
  return (KHE_ASSIGN_TIME_CONSTRAINT) c;
}

KHE_SPLIT_EVENTS_CONSTRAINT KheToSplitEventsConstraint(KHE_CONSTRAINT c)
{
  MAssert(c->tag == KHE_SPLIT_EVENTS_CONSTRAINT_TAG,
    "KheToSplitEventsConstraint: downcast failed");
  return (KHE_SPLIT_EVENTS_CONSTRAINT) c;
}

KHE_DISTRIBUTE_SPLIT_EVENTS_CONSTRAINT
  KheToDistributeSplitEventsConstraint(KHE_CONSTRAINT c)
{
  MAssert(c->tag == KHE_DISTRIBUTE_SPLIT_EVENTS_CONSTRAINT_TAG,
    "KheToDistributeSplitEventsConstraint: downcast failed");
  return (KHE_DISTRIBUTE_SPLIT_EVENTS_CONSTRAINT) c;
}

KHE_PREFER_RESOURCES_CONSTRAINT KheToPreferResourcesConstraint(KHE_CONSTRAINT c)
{
  MAssert(c->tag == KHE_PREFER_RESOURCES_CONSTRAINT_TAG,
    "KheToPreferResourcesConstraint: downcast failed");
  return (KHE_PREFER_RESOURCES_CONSTRAINT) c;
}

KHE_PREFER_TIMES_CONSTRAINT KheToPreferTimesConstraint(KHE_CONSTRAINT c)
{
  MAssert(c->tag == KHE_PREFER_TIMES_CONSTRAINT_TAG,
    "KheToPreferTimesConstraint: downcast failed");
  return (KHE_PREFER_TIMES_CONSTRAINT) c;
}

KHE_AVOID_SPLIT_ASSIGNMENTS_CONSTRAINT
  KheToAvoidSplitAssignmentsConstraint(KHE_CONSTRAINT c)
{
  MAssert(c->tag == KHE_AVOID_SPLIT_ASSIGNMENTS_CONSTRAINT_TAG,
    "KheToAvoidSplitAssignmentsConstraint: downcast failed");
  return (KHE_AVOID_SPLIT_ASSIGNMENTS_CONSTRAINT) c;
}

KHE_SPREAD_EVENTS_CONSTRAINT KheToSpreadEventsConstraint(KHE_CONSTRAINT c)
{
  MAssert(c->tag == KHE_SPREAD_EVENTS_CONSTRAINT_TAG,
    "KheToSpreadEventsConstraint: downcast failed");
  return (KHE_SPREAD_EVENTS_CONSTRAINT) c;
}

KHE_LINK_EVENTS_CONSTRAINT KheToLinkEventsConstraint(KHE_CONSTRAINT c)
{
  MAssert(c->tag == KHE_LINK_EVENTS_CONSTRAINT_TAG,
    "KheToLinkEventsConstraint: downcast failed");
  return (KHE_LINK_EVENTS_CONSTRAINT) c;
}

KHE_AVOID_CLASHES_CONSTRAINT KheToAvoidClashesConstraint(KHE_CONSTRAINT c)
{
  MAssert(c->tag == KHE_AVOID_CLASHES_CONSTRAINT_TAG,
    "KheToAvoidClashesConstraint: downcast failed");
  return (KHE_AVOID_CLASHES_CONSTRAINT) c;
}

KHE_AVOID_UNAVAILABLE_TIMES_CONSTRAINT
  KheToAvoidUnavailableTimesConstraint(KHE_CONSTRAINT c)
{
  MAssert(c->tag == KHE_AVOID_UNAVAILABLE_TIMES_CONSTRAINT_TAG,
    "KheToAvoidUnavailableTimesConstraint: downcast failed");
  return (KHE_AVOID_UNAVAILABLE_TIMES_CONSTRAINT) c;
}

KHE_LIMIT_IDLE_TIMES_CONSTRAINT KheToLimitIdleTimesConstraint(KHE_CONSTRAINT c)
{
  MAssert(c->tag == KHE_LIMIT_IDLE_TIMES_CONSTRAINT_TAG,
    "KheToLimitIdleTimesConstraint: downcast failed");
  return (KHE_LIMIT_IDLE_TIMES_CONSTRAINT) c;
}

KHE_CLUSTER_BUSY_TIMES_CONSTRAINT
  KheToClusterBusyTimesConstraint(KHE_CONSTRAINT c)
{
  MAssert(c->tag == KHE_CLUSTER_BUSY_TIMES_CONSTRAINT_TAG,
    "KheToClusterBusyTimesConstraint: downcast failed");
  return (KHE_CLUSTER_BUSY_TIMES_CONSTRAINT) c;
}

KHE_LIMIT_BUSY_TIMES_CONSTRAINT KheToLimitBusyTimesConstraint(KHE_CONSTRAINT c)
{
  MAssert(c->tag == KHE_LIMIT_BUSY_TIMES_CONSTRAINT_TAG,
    "KheToLimitBusyTimesConstraint: downcast failed");
  return (KHE_LIMIT_BUSY_TIMES_CONSTRAINT) c;
}

KHE_LIMIT_WORKLOAD_CONSTRAINT KheToLimitWorkloadConstraint(KHE_CONSTRAINT c)
{
  MAssert(c->tag == KHE_LIMIT_WORKLOAD_CONSTRAINT_TAG,
    "KheToLimitWorkloadConstraint: downcast failed");
  return (KHE_LIMIT_WORKLOAD_CONSTRAINT) c;
}


/*****************************************************************************/
/*                                                                           */
/*  Submodule "finalizing"                                                   */
/*                                                                           */
/*****************************************************************************/

/*****************************************************************************/
/*                                                                           */
/*  void KheConstraintFinalize(KHE_CONSTRAINT c)                             */
/*                                                                           */
/*  Finalize c (at end of instance read).                                    */
/*                                                                           */
/*****************************************************************************/

void KheConstraintFinalize(KHE_CONSTRAINT c)
{
  switch( c->tag )
  {
    case KHE_ASSIGN_RESOURCE_CONSTRAINT_TAG:
      KheAssignResourceConstraintFinalize(
	(KHE_ASSIGN_RESOURCE_CONSTRAINT) c);
      break;

    case KHE_ASSIGN_TIME_CONSTRAINT_TAG:
      KheAssignTimeConstraintFinalize(
	(KHE_ASSIGN_TIME_CONSTRAINT) c);
      break;

    case KHE_SPLIT_EVENTS_CONSTRAINT_TAG:
      KheSplitEventsConstraintFinalize(
	(KHE_SPLIT_EVENTS_CONSTRAINT) c);
      break;

    case KHE_DISTRIBUTE_SPLIT_EVENTS_CONSTRAINT_TAG:
      KheDistributeSplitEventsConstraintFinalize(
	(KHE_DISTRIBUTE_SPLIT_EVENTS_CONSTRAINT) c);
      break;

    case KHE_PREFER_RESOURCES_CONSTRAINT_TAG:
      KhePreferResourcesConstraintFinalize(
	(KHE_PREFER_RESOURCES_CONSTRAINT) c);
      break;

    case KHE_PREFER_TIMES_CONSTRAINT_TAG:
      KhePreferTimesConstraintFinalize(
	(KHE_PREFER_TIMES_CONSTRAINT) c);
      break;

    case KHE_AVOID_SPLIT_ASSIGNMENTS_CONSTRAINT_TAG:
      KheAvoidSplitAssignmentsConstraintFinalize(
	(KHE_AVOID_SPLIT_ASSIGNMENTS_CONSTRAINT) c);
      break;

    case KHE_SPREAD_EVENTS_CONSTRAINT_TAG:
      KheSpreadEventsConstraintFinalize(
	(KHE_SPREAD_EVENTS_CONSTRAINT) c);
      break;

    case KHE_LINK_EVENTS_CONSTRAINT_TAG:
      KheLinkEventsConstraintFinalize(
	(KHE_LINK_EVENTS_CONSTRAINT) c);
      break;

    case KHE_AVOID_CLASHES_CONSTRAINT_TAG:
      KheAvoidClashesConstraintFinalize(
	(KHE_AVOID_CLASHES_CONSTRAINT) c);
      break;

    case KHE_AVOID_UNAVAILABLE_TIMES_CONSTRAINT_TAG:
      KheAvoidUnavailableTimesConstraintFinalize(
	(KHE_AVOID_UNAVAILABLE_TIMES_CONSTRAINT) c);
      break;

    case KHE_LIMIT_IDLE_TIMES_CONSTRAINT_TAG:
      KheLimitIdleTimesConstraintFinalize(
	(KHE_LIMIT_IDLE_TIMES_CONSTRAINT) c);
      break;

    case KHE_CLUSTER_BUSY_TIMES_CONSTRAINT_TAG:
      KheClusterBusyTimesConstraintFinalize(
	(KHE_CLUSTER_BUSY_TIMES_CONSTRAINT) c);
      break;

    case KHE_LIMIT_BUSY_TIMES_CONSTRAINT_TAG:
      KheLimitBusyTimesConstraintFinalize(
	(KHE_LIMIT_BUSY_TIMES_CONSTRAINT) c);
      break;

    case KHE_LIMIT_WORKLOAD_CONSTRAINT_TAG:
      KheLimitWorkloadConstraintFinalize(
	(KHE_LIMIT_WORKLOAD_CONSTRAINT) c);
      break;

    default:
      MAssert(false, "KheConstraintFinalize internal error");
  }
}


/*****************************************************************************/
/*                                                                           */
/*  Submodule "reading and writing"                                          */
/*                                                                           */
/*****************************************************************************/

/*****************************************************************************/
/*                                                                           */
/*  bool KheConstraintMakeFromKml(KML_ELT cons_elt, KHE_INSTANCE ins,        */
/*    KML_ERROR *ke)                                                         */
/*                                                                           */
/*  Make a constraint from cons_elt and add it to ins.                       */
/*                                                                           */
/*****************************************************************************/

bool KheConstraintMakeFromKml(KML_ELT cons_elt, KHE_INSTANCE ins,
  KML_ERROR *ke)
{
  switch( KheStringToConstraintTag(KmlLabel(cons_elt)) )
  {
    case KHE_ASSIGN_RESOURCE_CONSTRAINT_TAG:
      return KheAssignResourceConstraintMakeFromKml(cons_elt, ins, ke);

    case KHE_ASSIGN_TIME_CONSTRAINT_TAG:
      return KheAssignTimeConstraintMakeFromKml(cons_elt, ins, ke);

    case KHE_SPLIT_EVENTS_CONSTRAINT_TAG:
      return KheSplitEventsConstraintMakeFromKml(cons_elt, ins, ke);

    case KHE_DISTRIBUTE_SPLIT_EVENTS_CONSTRAINT_TAG:
      return KheDistributeSplitEventsConstraintMakeFromKml(cons_elt, ins, ke);

    case KHE_PREFER_RESOURCES_CONSTRAINT_TAG:
      return KhePreferResourcesConstraintMakeFromKml(cons_elt, ins, ke);

    case KHE_PREFER_TIMES_CONSTRAINT_TAG:
      return KhePreferTimesConstraintMakeFromKml(cons_elt, ins, ke);

    case KHE_AVOID_SPLIT_ASSIGNMENTS_CONSTRAINT_TAG:
      return KheAvoidSplitAssignmentsConstraintMakeFromKml(cons_elt, ins, ke);

    case KHE_SPREAD_EVENTS_CONSTRAINT_TAG:
      return KheSpreadEventsConstraintMakeFromKml(cons_elt, ins, ke);

    case KHE_LINK_EVENTS_CONSTRAINT_TAG:
      return KheLinkEventsConstraintMakeFromKml(cons_elt, ins, ke);

    case KHE_AVOID_CLASHES_CONSTRAINT_TAG:
      return KheAvoidClashesConstraintMakeFromKml(cons_elt, ins, ke);

    case KHE_AVOID_UNAVAILABLE_TIMES_CONSTRAINT_TAG:
      return KheAvoidUnavailableTimesConstraintMakeFromKml(cons_elt, ins, ke);

    case KHE_LIMIT_IDLE_TIMES_CONSTRAINT_TAG:
      return KheLimitIdleTimesConstraintMakeFromKml(cons_elt, ins, ke);

    case KHE_CLUSTER_BUSY_TIMES_CONSTRAINT_TAG:
      return KheClusterBusyTimesConstraintMakeFromKml(cons_elt, ins, ke);

    case KHE_LIMIT_BUSY_TIMES_CONSTRAINT_TAG:
      return KheLimitBusyTimesConstraintMakeFromKml(cons_elt, ins, ke);

    case KHE_LIMIT_WORKLOAD_CONSTRAINT_TAG:
      return KheLimitWorkloadConstraintMakeFromKml(cons_elt, ins, ke);

    default:
      MAssert(false,
	"KheConstraintMakeFromKml: unexpected tag (internal error)");
      return false;
  }
}


/*****************************************************************************/
/*                                                                           */
/*  bool KheConstraintCheckKml(KML_ELT cons_elt, char **id, char **name,     */
/*    bool *required, int *weight, KHE_COST_FUNCTION *cf, KML_ERROR *ke)     */
/*                                                                           */
/*  Read the common elements from cons_elt.  It has already been checked     */
/*  to conform to "Id : $Name $Required #Weight $CostFunction" at least.     */
/*                                                                           */
/*****************************************************************************/

bool KheConstraintCheckKml(KML_ELT cons_elt, char **id, char **name,
  bool *required, int *weight, KHE_COST_FUNCTION *cf, KML_ERROR *ke)
{
  KML_ELT e;  char *text;

  /* Id and Name */
  *id = KmlExtractAttributeValue(cons_elt, 0);
  *name = KmlExtractText(KmlChild(cons_elt, 0));

  /* Required */
  e = KmlChild(cons_elt, 1);
  text = KmlText(e);
  if( strcmp(text, "true") == 0 )
    *required = true;
  else if( strcmp(text, "false") == 0 )
    *required = false;
  else
    return KmlErrorMake(ke, KmlLineNum(e), KmlColNum(e),
      "<Required> is neither true nor false");

  /* Weight */
  sscanf(KmlText(KmlChild(cons_elt, 2)), "%d", weight);
  if( *weight < 0 )
    return KmlErrorMake(ke, KmlLineNum(e), KmlColNum(e),
      "<Weight> is negative");
  if( *weight > 1000 )
    return KmlErrorMake(ke, KmlLineNum(e), KmlColNum(e),
      "<Weight> is too large (maximum is 1000)");

  /* CostFunction */
  e = KmlChild(cons_elt, 3);
  text = KmlText(e);
  if( strcmp(text, "SumSteps") == 0 )
    *cf = KHE_SUM_STEPS_COST_FUNCTION;
  else if( strcmp(text, "StepSum") == 0 )
    *cf = KHE_STEP_SUM_COST_FUNCTION;
  else if( strcmp(text, "Sum") == 0 )
    *cf = KHE_SUM_COST_FUNCTION;
  else if( strcmp(text, "SumSquares") == 0 )
    *cf = KHE_SUM_SQUARES_COST_FUNCTION;
  else if( strcmp(text, "SquareSum") == 0 )
    *cf = KHE_SQUARE_SUM_COST_FUNCTION;
  else
    return KmlErrorMake(ke, KmlLineNum(e), KmlColNum(e),
      "<CostFunction> has unknown value \"%s\"", text);
  return true;
}


/*****************************************************************************/
/*                                                                           */
/*  bool KheConstraintAddTimeGroupsFromKml(KHE_CONSTRAINT c, KML_ELT elt,    */
/*    KML_ERROR *ke)                                                         */
/*                                                                           */
/*  If elt contains an TimeGroups child, add those time groups to c.         */
/*                                                                           */
/*****************************************************************************/

bool KheConstraintAddTimeGroupsFromKml(KHE_CONSTRAINT c, KML_ELT elt,
  KML_ERROR *ke)
{
  KML_ELT time_groups_elt, time_group_elt;  KHE_TIME_GROUP tg;
  int i;  char *ref;
  if( KmlContainsChild(elt, "TimeGroups", &time_groups_elt) )
  {
    if( !KmlCheck(time_groups_elt, ": *TimeGroup", ke) )
      return false;
    for( i = 0;  i < KmlChildCount(time_groups_elt);  i++ )
    {
      time_group_elt = KmlChild(time_groups_elt, i);
      if( !KmlCheck(time_group_elt, "Reference", ke) )
	return false;
      ref = KmlAttributeValue(time_group_elt, 0);
      if( !KheInstanceRetrieveTimeGroup(c->instance, ref, &tg) )
	return KmlErrorMake(ke, KmlLineNum(time_group_elt),
	  KmlColNum(time_group_elt),
	  "<TimeGroup> Reference \"%s\" unknown", ref);
      switch( c->tag )
      {
	case KHE_PREFER_TIMES_CONSTRAINT_TAG:
	  KhePreferTimesConstraintAddTimeGroup(
	    (KHE_PREFER_TIMES_CONSTRAINT) c, tg);
	  break;

	case KHE_LIMIT_IDLE_TIMES_CONSTRAINT_TAG:
	  KheLimitIdleTimesConstraintAddTimeGroup(
	    (KHE_LIMIT_IDLE_TIMES_CONSTRAINT) c, tg);
	  break;

	case KHE_AVOID_UNAVAILABLE_TIMES_CONSTRAINT_TAG:
	  KheAvoidUnavailableTimesConstraintAddTimeGroup(
	    (KHE_AVOID_UNAVAILABLE_TIMES_CONSTRAINT) c, tg);
	  break;

	case KHE_CLUSTER_BUSY_TIMES_CONSTRAINT_TAG:
	  KheClusterBusyTimesConstraintAddTimeGroup(
	    (KHE_CLUSTER_BUSY_TIMES_CONSTRAINT) c, tg);
	  break;

	case KHE_LIMIT_BUSY_TIMES_CONSTRAINT_TAG:
	  KheLimitBusyTimesConstraintAddTimeGroup(
	    (KHE_LIMIT_BUSY_TIMES_CONSTRAINT) c, tg);
	  break;

	case KHE_ASSIGN_RESOURCE_CONSTRAINT_TAG:
	case KHE_ASSIGN_TIME_CONSTRAINT_TAG:
	case KHE_SPLIT_EVENTS_CONSTRAINT_TAG:
	case KHE_DISTRIBUTE_SPLIT_EVENTS_CONSTRAINT_TAG:
	case KHE_PREFER_RESOURCES_CONSTRAINT_TAG:
	case KHE_AVOID_SPLIT_ASSIGNMENTS_CONSTRAINT_TAG:
	case KHE_SPREAD_EVENTS_CONSTRAINT_TAG:
	case KHE_LINK_EVENTS_CONSTRAINT_TAG:
	case KHE_AVOID_CLASHES_CONSTRAINT_TAG:
	case KHE_LIMIT_WORKLOAD_CONSTRAINT_TAG:
	default:

	  MAssert(false, "KheConstraintAddTimeGroup internal error");
      }
    }
  }
  return true;
}


/*****************************************************************************/
/*                                                                           */
/*  bool KheConstraintAddTimesFromKml(KHE_CONSTRAINT c, KML_ELT elt,         */
/*    KML_ERROR *ke)                                                         */
/*                                                                           */
/*  If elt contains an Times child, add those times to c.                    */
/*                                                                           */
/*****************************************************************************/

bool KheConstraintAddTimesFromKml(KHE_CONSTRAINT c, KML_ELT elt,
  KML_ERROR *ke)
{
  KML_ELT times_elt, time_elt;  KHE_TIME t;  int i;  char *ref;
  if( KmlContainsChild(elt, "Times", &times_elt) )
  {
    if( !KmlCheck(times_elt, ": *Time", ke) )
      return false;
    for( i = 0;  i < KmlChildCount(times_elt);  i++ )
    {
      time_elt = KmlChild(times_elt, i);
      if( !KmlCheck(time_elt, "Reference", ke) )
	return false;
      ref = KmlAttributeValue(time_elt, 0);
      if( !KheInstanceRetrieveTime(c->instance, ref, &t) )
	return KmlErrorMake(ke, KmlLineNum(time_elt),
	  KmlColNum(time_elt), "<Time> Reference \"%s\" unknown", ref);
      switch( c->tag )
      {
	case KHE_PREFER_TIMES_CONSTRAINT_TAG:
	  KhePreferTimesConstraintAddTime(
	    (KHE_PREFER_TIMES_CONSTRAINT) c, t);
	  break;

	case KHE_AVOID_UNAVAILABLE_TIMES_CONSTRAINT_TAG:
	  KheAvoidUnavailableTimesConstraintAddTime(
	    (KHE_AVOID_UNAVAILABLE_TIMES_CONSTRAINT) c, t);
	  break;

	case KHE_ASSIGN_RESOURCE_CONSTRAINT_TAG:
	case KHE_ASSIGN_TIME_CONSTRAINT_TAG:
	case KHE_SPLIT_EVENTS_CONSTRAINT_TAG:
	case KHE_DISTRIBUTE_SPLIT_EVENTS_CONSTRAINT_TAG:
	case KHE_PREFER_RESOURCES_CONSTRAINT_TAG:
	case KHE_AVOID_SPLIT_ASSIGNMENTS_CONSTRAINT_TAG:
	case KHE_SPREAD_EVENTS_CONSTRAINT_TAG:
	case KHE_LINK_EVENTS_CONSTRAINT_TAG:
	case KHE_AVOID_CLASHES_CONSTRAINT_TAG:
	case KHE_LIMIT_IDLE_TIMES_CONSTRAINT_TAG:
	case KHE_CLUSTER_BUSY_TIMES_CONSTRAINT_TAG:
	case KHE_LIMIT_BUSY_TIMES_CONSTRAINT_TAG:
	case KHE_LIMIT_WORKLOAD_CONSTRAINT_TAG:
	default:

	  MAssert(false, "KheConstraintAddTime internal error");
      }
    }
  }
  return true;
}


/*****************************************************************************/
/*                                                                           */
/*  bool KheConstraintAddResourceGroupsFromKml(KHE_CONSTRAINT c,             */
/*    KML_ELT elt, KML_ERROR *ke)                                            */
/*                                                                           */
/*  If elt contains an ResourceGroups child, add those resource groups to c. */
/*                                                                           */
/*****************************************************************************/

bool KheConstraintAddResourceGroupsFromKml(KHE_CONSTRAINT c, KML_ELT elt,
  KML_ERROR *ke)
{
  KML_ELT resource_groups_elt, resource_group_elt;  KHE_RESOURCE_GROUP rg;
  int i;  char *ref;
  if( KmlContainsChild(elt, "ResourceGroups", &resource_groups_elt) )
  {
    if( !KmlCheck(resource_groups_elt, ": *ResourceGroup", ke) )
      return false;
    for( i = 0;  i < KmlChildCount(resource_groups_elt);  i++ )
    {
      resource_group_elt = KmlChild(resource_groups_elt, i);
      if( !KmlCheck(resource_group_elt, "Reference", ke) )
	return false;
      ref = KmlAttributeValue(resource_group_elt, 0);
      if( !KheInstanceRetrieveResourceGroup(c->instance, ref, &rg) )
	return KmlErrorMake(ke, KmlLineNum(resource_group_elt),
	  KmlColNum(resource_group_elt),
	  "<ResourceGroup> Reference \"%s\" unknown", ref);
      switch( c->tag )
      {
	case KHE_PREFER_RESOURCES_CONSTRAINT_TAG:
	  if( !KhePreferResourcesConstraintAddResourceGroup(
	      (KHE_PREFER_RESOURCES_CONSTRAINT) c, rg) )
	    return KmlErrorMake(ke, KmlLineNum(resource_group_elt),
	      KmlColNum(resource_group_elt),
	      "<ResourceGroup> \"%s\" has inconsistent <ResourceType>", ref);
	  break;

	case KHE_AVOID_CLASHES_CONSTRAINT_TAG:
	  KheAvoidClashesConstraintAddResourceGroup(
	    (KHE_AVOID_CLASHES_CONSTRAINT) c, rg);
	  break;

	case KHE_AVOID_UNAVAILABLE_TIMES_CONSTRAINT_TAG:
	  KheAvoidUnavailableTimesConstraintAddResourceGroup(
	    (KHE_AVOID_UNAVAILABLE_TIMES_CONSTRAINT) c, rg);
	  break;

	case KHE_LIMIT_IDLE_TIMES_CONSTRAINT_TAG:
	  KheLimitIdleTimesConstraintAddResourceGroup(
	    (KHE_LIMIT_IDLE_TIMES_CONSTRAINT) c, rg);
	  break;

	case KHE_CLUSTER_BUSY_TIMES_CONSTRAINT_TAG:
	  KheClusterBusyTimesConstraintAddResourceGroup(
	    (KHE_CLUSTER_BUSY_TIMES_CONSTRAINT) c, rg);
	  break;

	case KHE_LIMIT_BUSY_TIMES_CONSTRAINT_TAG:
	  KheLimitBusyTimesConstraintAddResourceGroup(
	    (KHE_LIMIT_BUSY_TIMES_CONSTRAINT) c, rg);
	  break;

	case KHE_LIMIT_WORKLOAD_CONSTRAINT_TAG:
	  KheLimitWorkloadConstraintAddResourceGroup(
	    (KHE_LIMIT_WORKLOAD_CONSTRAINT) c, rg);
	  break;

	case KHE_ASSIGN_RESOURCE_CONSTRAINT_TAG:
	case KHE_ASSIGN_TIME_CONSTRAINT_TAG:
	case KHE_SPLIT_EVENTS_CONSTRAINT_TAG:
	case KHE_DISTRIBUTE_SPLIT_EVENTS_CONSTRAINT_TAG:
	case KHE_PREFER_TIMES_CONSTRAINT_TAG:
	case KHE_AVOID_SPLIT_ASSIGNMENTS_CONSTRAINT_TAG:
	case KHE_SPREAD_EVENTS_CONSTRAINT_TAG:
	case KHE_LINK_EVENTS_CONSTRAINT_TAG:
	default:

	  MAssert(false, "KheConstraintAddResourceGroup internal error");
      }
    }
  }
  return true;
}


/*****************************************************************************/
/*                                                                           */
/*  bool KheConstraintAddResourcesFromKml(KHE_CONSTRAINT c, KML_ELT elt,     */
/*    KML_ERROR *ke)                                                         */
/*                                                                           */
/*  If elt contains an Resources child, add those resources to c.            */
/*                                                                           */
/*****************************************************************************/

bool KheConstraintAddResourcesFromKml(KHE_CONSTRAINT c, KML_ELT elt,
  KML_ERROR *ke)
{
  KML_ELT resources_elt, resource_elt;  KHE_RESOURCE r;  int i;  char *ref;
  if( KmlContainsChild(elt, "Resources", &resources_elt) )
  {
    if( !KmlCheck(resources_elt, ": *Resource", ke) )
      return false;
    for( i = 0;  i < KmlChildCount(resources_elt);  i++ )
    {
      resource_elt = KmlChild(resources_elt, i);
      if( !KmlCheck(resource_elt, "Reference", ke) )
	return false;
      ref = KmlAttributeValue(resource_elt, 0);
      if( !KheInstanceRetrieveResource(c->instance, ref, &r) )
	return KmlErrorMake(ke, KmlLineNum(resource_elt),
	  KmlColNum(resource_elt),
	  "<Resource> Reference \"%s\" unknown", ref);
      switch( c->tag )
      {
	case KHE_PREFER_RESOURCES_CONSTRAINT_TAG:
	  if( !KhePreferResourcesConstraintAddResource(
	      (KHE_PREFER_RESOURCES_CONSTRAINT) c, r) )
	    return KmlErrorMake(ke, KmlLineNum(resource_elt),
	      KmlColNum(resource_elt),
	      "<Resource> \"%s\" has inconsistent <ResourceType>", ref);
	  break;

	case KHE_AVOID_CLASHES_CONSTRAINT_TAG:
	  KheAvoidClashesConstraintAddResource(
	    (KHE_AVOID_CLASHES_CONSTRAINT) c, r);
	  break;

	case KHE_AVOID_UNAVAILABLE_TIMES_CONSTRAINT_TAG:
	  KheAvoidUnavailableTimesConstraintAddResource(
	    (KHE_AVOID_UNAVAILABLE_TIMES_CONSTRAINT) c, r);
	  break;

	case KHE_LIMIT_IDLE_TIMES_CONSTRAINT_TAG:
	  KheLimitIdleTimesConstraintAddResource(
	    (KHE_LIMIT_IDLE_TIMES_CONSTRAINT) c, r);
	  break;

	case KHE_CLUSTER_BUSY_TIMES_CONSTRAINT_TAG:
	  KheClusterBusyTimesConstraintAddResource(
	    (KHE_CLUSTER_BUSY_TIMES_CONSTRAINT) c, r);
	  break;

	case KHE_LIMIT_BUSY_TIMES_CONSTRAINT_TAG:
	  KheLimitBusyTimesConstraintAddResource(
	    (KHE_LIMIT_BUSY_TIMES_CONSTRAINT) c, r);
	  break;

	case KHE_LIMIT_WORKLOAD_CONSTRAINT_TAG:
	  KheLimitWorkloadConstraintAddResource(
	    (KHE_LIMIT_WORKLOAD_CONSTRAINT) c, r);
	  break;

	case KHE_ASSIGN_RESOURCE_CONSTRAINT_TAG:
	case KHE_ASSIGN_TIME_CONSTRAINT_TAG:
	case KHE_SPLIT_EVENTS_CONSTRAINT_TAG:
	case KHE_DISTRIBUTE_SPLIT_EVENTS_CONSTRAINT_TAG:
	case KHE_PREFER_TIMES_CONSTRAINT_TAG:
	case KHE_AVOID_SPLIT_ASSIGNMENTS_CONSTRAINT_TAG:
	case KHE_SPREAD_EVENTS_CONSTRAINT_TAG:
	case KHE_LINK_EVENTS_CONSTRAINT_TAG:
	default:

	  MAssert(false, "KheConstraintAddResource internal error");
      }
    }
  }
  return true;
}


/*****************************************************************************/
/*                                                                           */
/*  bool KheConstraintAddEventGroupsFromKml(KHE_CONSTRAINT c, KML_ELT elt,   */
/*    KML_ERROR *ke)                                                         */
/*                                                                           */
/*  If elt contains an EventGroups child, add those event groups to c.       */
/*                                                                           */
/*****************************************************************************/

bool KheConstraintAddEventGroupsFromKml(KHE_CONSTRAINT c, KML_ELT elt,
  KML_ERROR *ke)
{
  KML_ELT event_groups_elt, event_group_elt;  KHE_EVENT_GROUP eg;
  KHE_EVENT e;  int i;  char *ref;
  if( KmlContainsChild(elt, "EventGroups", &event_groups_elt) )
  {
    if( !KmlCheck(event_groups_elt, ": *EventGroup", ke) )
      return false;
    for( i = 0;  i < KmlChildCount(event_groups_elt);  i++ )
    {
      event_group_elt = KmlChild(event_groups_elt, i);
      if( !KmlCheck(event_group_elt, "Reference", ke) )
	return false;
      ref = KmlAttributeValue(event_group_elt, 0);
      if( !KheInstanceRetrieveEventGroup(c->instance, ref, &eg) )
	return KmlErrorMake(ke, KmlLineNum(event_group_elt),
	  KmlColNum(event_group_elt),
	  "<EventGroup> Reference \"%s\" unknown", ref);
      switch( c->tag )
      {
	case KHE_ASSIGN_RESOURCE_CONSTRAINT_TAG:
	  KheAssignResourceConstraintAddEventGroup(
	    (KHE_ASSIGN_RESOURCE_CONSTRAINT) c, eg);
	  break;

	case KHE_ASSIGN_TIME_CONSTRAINT_TAG:
	  KheAssignTimeConstraintAddEventGroup(
	    (KHE_ASSIGN_TIME_CONSTRAINT) c, eg);
	  break;

	case KHE_SPLIT_EVENTS_CONSTRAINT_TAG:
	  KheSplitEventsConstraintAddEventGroup(
	    (KHE_SPLIT_EVENTS_CONSTRAINT) c, eg);
	  break;

	case KHE_DISTRIBUTE_SPLIT_EVENTS_CONSTRAINT_TAG:
	  KheDistributeSplitEventsConstraintAddEventGroup(
	    (KHE_DISTRIBUTE_SPLIT_EVENTS_CONSTRAINT) c,eg);
	  break;

	case KHE_PREFER_RESOURCES_CONSTRAINT_TAG:
	  if( !KhePreferResourcesConstraintAddEventGroup(
	    (KHE_PREFER_RESOURCES_CONSTRAINT) c, eg, &e) )
	    return KmlErrorMake(ke, KmlLineNum(event_group_elt),
	      KmlColNum(event_group_elt), "<EventGroup> \"%s\" contains event "
	      "\"%s\" whose Resource has inconsistent <ResourceType>", ref,
	      KheEventId(e));
	  break;

	case KHE_PREFER_TIMES_CONSTRAINT_TAG:
	  KhePreferTimesConstraintAddEventGroup(
	    (KHE_PREFER_TIMES_CONSTRAINT) c, eg);
	  break;

	case KHE_AVOID_SPLIT_ASSIGNMENTS_CONSTRAINT_TAG:
	  if( !KheAvoidSplitAssignmentsConstraintAddEventGroup(
	    (KHE_AVOID_SPLIT_ASSIGNMENTS_CONSTRAINT) c, eg, &e) )
	    return KmlErrorMake(ke, KmlLineNum(event_group_elt),
	      KmlColNum(event_group_elt), "<EventGroup> \"%s\" contains "
	      "event \"%s\" without the given role", ref, KheEventId(e));
	  break;

	case KHE_SPREAD_EVENTS_CONSTRAINT_TAG:
	  KheSpreadEventsConstraintAddEventGroup(
	    (KHE_SPREAD_EVENTS_CONSTRAINT) c, eg);
	  break;

	case KHE_LINK_EVENTS_CONSTRAINT_TAG:
	  KheLinkEventsConstraintAddEventGroup(
	    (KHE_LINK_EVENTS_CONSTRAINT) c, eg);
	  break;

	case KHE_AVOID_CLASHES_CONSTRAINT_TAG:
	case KHE_AVOID_UNAVAILABLE_TIMES_CONSTRAINT_TAG:
	case KHE_LIMIT_IDLE_TIMES_CONSTRAINT_TAG:
	case KHE_CLUSTER_BUSY_TIMES_CONSTRAINT_TAG:
	case KHE_LIMIT_BUSY_TIMES_CONSTRAINT_TAG:
	case KHE_LIMIT_WORKLOAD_CONSTRAINT_TAG:
	default:

	  MAssert(false, "KheConstraintAddEventGroup internal error");
	  return true;  /* keep compiler happy */
      }
    }
  }
  return true;
}


/*****************************************************************************/
/*                                                                           */
/*  bool KheConstraintAddEventsFromKml(KHE_CONSTRAINT c, KML_ELT elt,        */
/*    KML_ERROR *ke)                                                         */
/*                                                                           */
/*  If elt contains an Events child, add those events to c.                  */
/*                                                                           */
/*****************************************************************************/

bool KheConstraintAddEventsFromKml(KHE_CONSTRAINT c, KML_ELT elt,
  KML_ERROR *ke)
{
  KML_ELT events_elt, event_elt;  KHE_EVENT e;  int i;  char *ref;
  if( KmlContainsChild(elt, "Events", &events_elt) )
  {
    if( !KmlCheck(events_elt, ": *Event", ke) )
      return false;
    for( i = 0;  i < KmlChildCount(events_elt);  i++ )
    {
      event_elt = KmlChild(events_elt, i);
      if( !KmlCheck(event_elt, "Reference", ke) )
	return false;
      ref = KmlAttributeValue(event_elt, 0);
      if( !KheInstanceRetrieveEvent(c->instance, ref, &e) )
	return KmlErrorMake(ke, KmlLineNum(event_elt),
	  KmlColNum(event_elt),
	  "<Event> Reference \"%s\" unknown", ref);
      switch( c->tag )
      {
	case KHE_ASSIGN_RESOURCE_CONSTRAINT_TAG:
	  KheAssignResourceConstraintAddEvent(
	    (KHE_ASSIGN_RESOURCE_CONSTRAINT) c, e);
	  break;

	case KHE_ASSIGN_TIME_CONSTRAINT_TAG:
	  KheAssignTimeConstraintAddEvent((KHE_ASSIGN_TIME_CONSTRAINT) c, e);
	  break;

	case KHE_SPLIT_EVENTS_CONSTRAINT_TAG:
	  KheSplitEventsConstraintAddEvent((KHE_SPLIT_EVENTS_CONSTRAINT) c, e);
	  break;

	case KHE_DISTRIBUTE_SPLIT_EVENTS_CONSTRAINT_TAG:
	  KheDistributeSplitEventsConstraintAddEvent(
	    (KHE_DISTRIBUTE_SPLIT_EVENTS_CONSTRAINT) c, e);
	  break;

	case KHE_PREFER_RESOURCES_CONSTRAINT_TAG:
	  if( !KhePreferResourcesConstraintAddEvent(
	      (KHE_PREFER_RESOURCES_CONSTRAINT) c, e) )
	    return KmlErrorMake(ke, KmlLineNum(event_elt), KmlColNum(event_elt),
	      "<Event> \"%s\" Resource has inconsistent <ResourceType>", ref);
	  break;

	case KHE_PREFER_TIMES_CONSTRAINT_TAG:
	  KhePreferTimesConstraintAddEvent((KHE_PREFER_TIMES_CONSTRAINT) c, e);
	  break;

	case KHE_AVOID_SPLIT_ASSIGNMENTS_CONSTRAINT_TAG:
	case KHE_SPREAD_EVENTS_CONSTRAINT_TAG:
	case KHE_LINK_EVENTS_CONSTRAINT_TAG:
	case KHE_AVOID_CLASHES_CONSTRAINT_TAG:
	case KHE_AVOID_UNAVAILABLE_TIMES_CONSTRAINT_TAG:
	case KHE_LIMIT_IDLE_TIMES_CONSTRAINT_TAG:
	case KHE_CLUSTER_BUSY_TIMES_CONSTRAINT_TAG:
	case KHE_LIMIT_BUSY_TIMES_CONSTRAINT_TAG:
	case KHE_LIMIT_WORKLOAD_CONSTRAINT_TAG:
	default:

	  MAssert(false, "KheConstraintAddEventGroup internal error");
      }
    }
  }
  return true;
}


/*****************************************************************************/
/*                                                                           */
/*  bool KheConstraintWrite(KHE_CONSTRAINT c, KML_FILE kf)                   */
/*                                                                           */
/*  Write c onto kf.                                                        */
/*                                                                           */
/*****************************************************************************/

bool KheConstraintWrite(KHE_CONSTRAINT c, KML_FILE kf)
{
  switch( c->tag )
  {
    case KHE_ASSIGN_RESOURCE_CONSTRAINT_TAG:
      return KheAssignResourceConstraintWrite(
	(KHE_ASSIGN_RESOURCE_CONSTRAINT) c, kf);

    case KHE_ASSIGN_TIME_CONSTRAINT_TAG:
      return KheAssignTimeConstraintWrite((KHE_ASSIGN_TIME_CONSTRAINT) c, kf);

    case KHE_SPLIT_EVENTS_CONSTRAINT_TAG:
      return KheSplitEventsConstraintWrite((KHE_SPLIT_EVENTS_CONSTRAINT) c,kf);

    case KHE_DISTRIBUTE_SPLIT_EVENTS_CONSTRAINT_TAG:
      return KheDistributeSplitEventsConstraintWrite(
	(KHE_DISTRIBUTE_SPLIT_EVENTS_CONSTRAINT) c, kf);

    case KHE_PREFER_RESOURCES_CONSTRAINT_TAG:
      return KhePreferResourcesConstraintWrite(
	(KHE_PREFER_RESOURCES_CONSTRAINT) c, kf);

    case KHE_PREFER_TIMES_CONSTRAINT_TAG:
      return KhePreferTimesConstraintWrite((KHE_PREFER_TIMES_CONSTRAINT) c,kf);

    case KHE_AVOID_SPLIT_ASSIGNMENTS_CONSTRAINT_TAG:
      return KheAvoidSplitAssignmentsConstraintWrite(
	(KHE_AVOID_SPLIT_ASSIGNMENTS_CONSTRAINT) c, kf);

    case KHE_SPREAD_EVENTS_CONSTRAINT_TAG:
      return KheSpreadEventsConstraintWrite(
	(KHE_SPREAD_EVENTS_CONSTRAINT) c, kf);

    case KHE_LINK_EVENTS_CONSTRAINT_TAG:
      return KheLinkEventsConstraintWrite((KHE_LINK_EVENTS_CONSTRAINT) c, kf);

    case KHE_AVOID_CLASHES_CONSTRAINT_TAG:
      return KheAvoidClashesConstraintWrite(
	(KHE_AVOID_CLASHES_CONSTRAINT) c, kf);

    case KHE_AVOID_UNAVAILABLE_TIMES_CONSTRAINT_TAG:
      return KheAvoidUnavailableTimesConstraintWrite(
	(KHE_AVOID_UNAVAILABLE_TIMES_CONSTRAINT) c, kf);

    case KHE_LIMIT_IDLE_TIMES_CONSTRAINT_TAG:
      return KheLimitIdleTimesConstraintWrite(
	(KHE_LIMIT_IDLE_TIMES_CONSTRAINT) c, kf);

    case KHE_CLUSTER_BUSY_TIMES_CONSTRAINT_TAG:
      return KheClusterBusyTimesConstraintWrite(
	(KHE_CLUSTER_BUSY_TIMES_CONSTRAINT) c, kf);

    case KHE_LIMIT_BUSY_TIMES_CONSTRAINT_TAG:
      return KheLimitBusyTimesConstraintWrite(
	(KHE_LIMIT_BUSY_TIMES_CONSTRAINT) c, kf);

    case KHE_LIMIT_WORKLOAD_CONSTRAINT_TAG:
      return KheLimitWorkloadConstraintWrite(
	(KHE_LIMIT_WORKLOAD_CONSTRAINT) c, kf);

    default:
      MAssert(false, "KheConstraintWrite: unknown tag (internal error)");
      return false; /* keep compiler happy */
  }
}


/*****************************************************************************/
/*                                                                           */
/*  char *KheCostFunctionShow(KHE_COST_FUNCTION cf)                          */
/*                                                                           */
/*  Return the XML string value of cf.                                       */
/*                                                                           */
/*****************************************************************************/

char *KheCostFunctionShow(KHE_COST_FUNCTION cf)
{
  switch( cf )
  {
    case KHE_SUM_STEPS_COST_FUNCTION:	  return "SumSteps";
    case KHE_STEP_SUM_COST_FUNCTION:	  return "StepsSum";
    case KHE_SUM_COST_FUNCTION:	  	  return "Sum";
    case KHE_SUM_SQUARES_COST_FUNCTION:	  return "SumSquares";
    case KHE_SQUARE_SUM_COST_FUNCTION:	  return "SquareSum";

    default:	MAssert(false, "unknown cost function value");
		return NULL;  /* keep compiler happy */
  }
}


/*****************************************************************************/
/*                                                                           */
/*  bool KheConstraintWriteCommonFields(KHE_CONSTRAINT c, KML_FILE kf)       */
/*                                                                           */
/*  Write the fields of c common to all constraints onto kf.                 */
/*                                                                           */
/*****************************************************************************/

bool KheConstraintWriteCommonFields(KHE_CONSTRAINT c, KML_FILE kf)
{
  if( c->name == NULL )
    return false;
  KmlEltPrintf(kf, "Name", "%s", c->name);
  KmlEltPrintf(kf, "Required", c->required ? "true" : "false");
  KmlEltPrintf(kf, "Weight", "%d", c->weight);
  KmlEltPrintf(kf, "CostFunction", KheCostFunctionShow(c->cost_function));
  return true;
}


/*****************************************************************************/
/*                                                                           */
/*  Submodule "evaluation"                                                   */
/*                                                                           */
/*****************************************************************************/

/*****************************************************************************/
/*                                                                           */
/*  KHE_COST KheConstraintCost(KHE_CONSTRAINT c, int dev)                    */
/*                                                                           */
/*  Calculate the cost when c has dev.                                       */
/*                                                                           */
/*****************************************************************************/

KHE_COST KheConstraintCost(KHE_CONSTRAINT c, int dev)
{
  switch( c->cost_function )
  {
    case KHE_SUM_STEPS_COST_FUNCTION:
    case KHE_STEP_SUM_COST_FUNCTION:

        return dev > 0 ? c->combined_weight : 0;

    case KHE_SUM_COST_FUNCTION:

        return dev * c->combined_weight;

    case KHE_SUM_SQUARES_COST_FUNCTION:
    case KHE_SQUARE_SUM_COST_FUNCTION:

	return dev * dev * c->combined_weight;

    default:

	MAssert(false, "KheConstraintCost internal error");
	return 0;  /* keep compiler happy */
  }
}


/*****************************************************************************/
/*                                                                           */
/*  KHE_COST KheConstraintCostMulti(KHE_CONSTRAINT c, ARRAY_INT *devs)       */
/*                                                                           */
/*  Calculate the cost when c has devs.                                      */
/*                                                                           */
/*****************************************************************************/

KHE_COST KheConstraintCostMulti(KHE_CONSTRAINT c, ARRAY_INT *devs)
{
  int d, i; KHE_COST cost;
  switch( c->cost_function )
  {
    case KHE_SUM_STEPS_COST_FUNCTION:

        cost = 0;
	MArrayForEach(*devs, &d, &i)
	  if( d > 0 )
	    cost += c->combined_weight;
	return cost;

        
    case KHE_STEP_SUM_COST_FUNCTION:

	MArrayForEach(*devs, &d, &i)
	  if( d > 0 )
	    return c->combined_weight;
	return 0;


    case KHE_SUM_COST_FUNCTION:

        cost = 0;
	MArrayForEach(*devs, &d, &i)
	  cost += d;
	return cost * c->combined_weight;


    case KHE_SUM_SQUARES_COST_FUNCTION:

        cost = 0;
	MArrayForEach(*devs, &d, &i)
	  cost += d * d;
	return cost * c->combined_weight;
        

    case KHE_SQUARE_SUM_COST_FUNCTION:

        cost = 0;
	MArrayForEach(*devs, &d, &i)
	  cost += d;
	return cost * cost * c->combined_weight;

    default:

	MAssert(false, "KheConstraintCostMulti internal error");
	return 0;  /* keep compiler happy */
  }
}
