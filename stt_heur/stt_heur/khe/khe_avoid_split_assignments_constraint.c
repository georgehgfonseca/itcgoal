
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
/*  FILE:         khe_avoid_split_assignments_constraint.c                   */
/*  DESCRIPTION:  An avoid split assignments constraint                      */
/*                                                                           */
/*****************************************************************************/
#include "khe_interns.h"


/*****************************************************************************/
/*                                                                           */
/*  ASAC_POINT - one point of application of an avoid split assignments c.   */
/*                                                                           */
/*****************************************************************************/

typedef struct khe_asac_point_rec {
  KHE_EVENT_GROUP		event_group;		/* event group       */
  ARRAY_KHE_EVENT_RESOURCE	event_resources;	/* event resources   */
} *ASAC_POINT;

typedef MARRAY(ASAC_POINT) ARRAY_ASAC_POINT;


/*****************************************************************************/
/*                                                                           */
/*  KHE_AVOID_SPLIT_ASSIGNMENTS_CONSTRAINT - an avoid split assts constraint */
/*                                                                           */
/*****************************************************************************/

struct khe_avoid_split_assignments_constraint_rec {
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
  char				*role;			/* Role              */
  ARRAY_ASAC_POINT		asac_points;		/* points of applc.  */
};


/*****************************************************************************/
/*                                                                           */
/*  Submodule "construction and query"                                       */
/*                                                                           */
/*****************************************************************************/

/*****************************************************************************/
/*                                                                           */
/*  bool KheAvoidSplitAssignmentsConstraintMake(KHE_INSTANCE ins, char *id,  */
/*    char *name, bool required, int weight, KHE_COST_FUNCTION cf,           */
/*    char *role, KHE_AVOID_SPLIT_ASSIGNMENTS_CONSTRAINT *c)                 */
/*                                                                           */
/*  Make an avoid split assignments constraint, add it to the instance, and  */
/*  return it.                                                               */
/*                                                                           */
/*****************************************************************************/

bool KheAvoidSplitAssignmentsConstraintMake(KHE_INSTANCE ins, char *id,
  char *name, bool required, int weight, KHE_COST_FUNCTION cf,
  char *role, KHE_AVOID_SPLIT_ASSIGNMENTS_CONSTRAINT *c)
{
  KHE_AVOID_SPLIT_ASSIGNMENTS_CONSTRAINT res;  KHE_CONSTRAINT cc;
  MAssert(!KheInstanceComplete(ins),
    "KheAvoidSplitAssignmentsConstraintMake called after KheInstanceMakeEnd");
  if( id != NULL && KheInstanceRetrieveConstraint(ins, id, &cc) )
  {
    *c = NULL;
    return false;
  }
  MMake(res);
  res->back = NULL;
  res->tag = KHE_AVOID_SPLIT_ASSIGNMENTS_CONSTRAINT_TAG;
  res->instance = ins;
  res->id = id;
  res->name = name;
  res->required = required;
  res->weight = weight;
  res->combined_weight = required ? KheCost(weight, 0) : KheCost(0, weight);
  res->cost_function = cf;
  res->index = KheInstanceConstraintCount(ins);
  res->role = role;
  MArrayInit(res->asac_points);
  KheInstanceAddConstraint(ins, (KHE_CONSTRAINT) res);
  *c = res;
  return true;
}


/*****************************************************************************/
/*                                                                           */
/*  char *KheAvoidSplitAssignmentsConstraintRole(                            */
/*    KHE_AVOID_SPLIT_ASSIGNMENTS_CONSTRAINT c)                              */
/*                                                                           */
/*  Return the role attribute of c.                                          */
/*                                                                           */
/*****************************************************************************/

char *KheAvoidSplitAssignmentsConstraintRole(
  KHE_AVOID_SPLIT_ASSIGNMENTS_CONSTRAINT c)
{
  return c->role;
}


/*****************************************************************************/
/*                                                                           */
/*  int KheAvoidSplitAssignmentsConstraintAppliesToCount(                    */
/*    KHE_AVOID_SPLIT_ASSIGNMENTS_CONSTRAINT c)                              */
/*                                                                           */
/*  Return the number of points of application of c.                         */
/*                                                                           */
/*****************************************************************************/

int KheAvoidSplitAssignmentsConstraintAppliesToCount(
  KHE_AVOID_SPLIT_ASSIGNMENTS_CONSTRAINT c)
{
  return MArraySize(c->asac_points);
}


/*****************************************************************************/
/*                                                                           */
/*  void KheAvoidSplitAssignmentsConstraintFinalize(                         */
/*    KHE_AVOID_SPLIT_ASSIGNMENTS_CONSTRAINT c)                              */
/*                                                                           */
/*  Finalize c, since KheInstanceMakeEnd has been called.                    */
/*                                                                           */
/*****************************************************************************/

void KheAvoidSplitAssignmentsConstraintFinalize(
  KHE_AVOID_SPLIT_ASSIGNMENTS_CONSTRAINT c)
{
  /* nothing to do in this case */
}


/*****************************************************************************/
/*                                                                           */
/*  Submodule "event groups"                                                 */
/*                                                                           */
/*****************************************************************************/

/*****************************************************************************/
/*                                                                           */
/*  bool KheAvoidSplitAssignmentsConstraintAddEventGroup(                    */
/*    KHE_AVOID_SPLIT_ASSIGNMENTS_CONSTRAINT c, KHE_EVENT_GROUP eg,          */
/*    KHE_EVENT *problem_event)                                              */
/*                                                                           */
/*  Add eg to c.                                                             */
/*                                                                           */
/*****************************************************************************/

bool KheAvoidSplitAssignmentsConstraintAddEventGroup(
  KHE_AVOID_SPLIT_ASSIGNMENTS_CONSTRAINT c, KHE_EVENT_GROUP eg,
  KHE_EVENT *problem_event)
{
  ASAC_POINT ap;  int i;  KHE_EVENT e;  KHE_EVENT_RESOURCE er;
  MMake(ap);
  ap->event_group = eg;  /* could be NULL */
  MArrayInit(ap->event_resources);
  MArrayAddLast(c->asac_points, ap);
  if( eg != NULL )
  {
    KheEventGroupAddConstraint(eg, (KHE_CONSTRAINT) c);
    if( c->role != NULL )
    {
      for( i = 0;  i < KheEventGroupEventCount(eg);  i++ )
      {
	e = KheEventGroupEvent(eg, i);
	if( !KheEventRetrieveEventResource(e, c->role, &er) )
	{
	  *problem_event = e;
	  return false;
	}
	KheAvoidSplitAssignmentsConstraintAddEventResource(c,
	  MArraySize(c->asac_points) - 1, er);
      }
    }
  }
  *problem_event = NULL;
  return true;
}


/*****************************************************************************/
/*                                                                           */
/*  int KheAvoidSplitAssignmentsConstraintEventGroupCount(                   */
/*    KHE_AVOID_SPLIT_ASSIGNMENTS_CONSTRAINT c)                              */
/*                                                                           */
/*  Return the number of event groups of c.                                  */
/*                                                                           */
/*****************************************************************************/

int KheAvoidSplitAssignmentsConstraintEventGroupCount(
  KHE_AVOID_SPLIT_ASSIGNMENTS_CONSTRAINT c)
{
  return MArraySize(c->asac_points);
}


/*****************************************************************************/
/*                                                                           */
/*  KHE_EVENT_GROUP KheAvoidSplitAssignmentsConstraintEventGroup(            */
/*    KHE_AVOID_SPLIT_ASSIGNMENTS_CONSTRAINT c, int i)                       */
/*                                                                           */
/*  Return the                                                               */
/*                                                                           */
/*****************************************************************************/

KHE_EVENT_GROUP KheAvoidSplitAssignmentsConstraintEventGroup(
  KHE_AVOID_SPLIT_ASSIGNMENTS_CONSTRAINT c, int i)
{
  return MArrayGet(c->asac_points, i)->event_group;
}


/*****************************************************************************/
/*                                                                           */
/*  Submodule "event resources"                                              */
/*                                                                           */
/*****************************************************************************/

/*****************************************************************************/
/*                                                                           */
/*  void KheAvoidSplitAssignmentsConstraintAddEventResource(                 */
/*    KHE_AVOID_SPLIT_ASSIGNMENTS_CONSTRAINT c, int eg_index,                */
/*    KHE_EVENT_RESOURCE er)                                                 */
/*                                                                           */
/*  Add er to the eg_index's set of event resources of c.                    */
/*                                                                           */
/*****************************************************************************/

void KheAvoidSplitAssignmentsConstraintAddEventResource(
  KHE_AVOID_SPLIT_ASSIGNMENTS_CONSTRAINT c, int eg_index,
  KHE_EVENT_RESOURCE er)
{
  ASAC_POINT ap;
  ap = MArrayGet(c->asac_points, eg_index);
  MArrayAddLast(ap->event_resources, er);
  if( MArraySize(ap->event_resources) == 1 )
    KheResourceTypeIncrementAvoidSplitAssignmentsCount(
      KheEventResourceResourceType(er));

  KheEventResourceAddConstraint(er, (KHE_CONSTRAINT) c, eg_index);
}


/*****************************************************************************/
/*                                                                           */
/*  int KheAvoidSplitAssignmentsConstraintEventResourceCount(                */
/*    KHE_AVOID_SPLIT_ASSIGNMENTS_CONSTRAINT c, int eg_index)                */
/*                                                                           */
/*  Return the number of event resources of the eg_index'th set of event     */
/*  resources of c.                                                          */
/*                                                                           */
/*****************************************************************************/

int KheAvoidSplitAssignmentsConstraintEventResourceCount(
  KHE_AVOID_SPLIT_ASSIGNMENTS_CONSTRAINT c, int eg_index)
{
  ASAC_POINT ap;
  ap = MArrayGet(c->asac_points, eg_index);
  return MArraySize(ap->event_resources);
}


/*****************************************************************************/
/*                                                                           */
/*  KHE_EVENT_RESOURCE KheAvoidSplitAssignmentsConstraintEventResource(      */
/*    KHE_AVOID_SPLIT_ASSIGNMENTS_CONSTRAINT c, int eg_index, int er_index)  */
/*                                                                           */
/*  Return the er_index'th event resource of the eg_index'th set of event    */
/*  resources of c.                                                          */
/*                                                                           */
/*****************************************************************************/

KHE_EVENT_RESOURCE KheAvoidSplitAssignmentsConstraintEventResource(
  KHE_AVOID_SPLIT_ASSIGNMENTS_CONSTRAINT c, int eg_index, int er_index)
{
  ASAC_POINT ap;
  ap = MArrayGet(c->asac_points, eg_index);
  return MArrayGet(ap->event_resources, er_index);
}


/*****************************************************************************/
/*                                                                           */
/*  Submodule "reading and writing"                                          */
/*                                                                           */
/*****************************************************************************/

/*****************************************************************************/
/*                                                                           */
/*  bool KheAvoidSplitAssignmentsConstraintMakeFromKml(KML_ELT cons_elt,     */
/*    KHE_INSTANCE ins, KML_ERROR *ke)                                       */
/*                                                                           */
/*  Make an avoid split assignments constraint based on cons_elt and         */
/*  add it to ins.                                                           */
/*                                                                           */
/*****************************************************************************/

bool KheAvoidSplitAssignmentsConstraintMakeFromKml(KML_ELT cons_elt,
  KHE_INSTANCE ins, KML_ERROR *ke)
{
  char *id, *name;  bool reqd;  int wt;  KHE_COST_FUNCTION cf;
  KML_ELT elt;  KHE_AVOID_SPLIT_ASSIGNMENTS_CONSTRAINT res;

  /* verify cons_elt and get the common fields */
  if( !KmlCheck(cons_elt, "Id : $Name $Required #Weight "
      "$CostFunction AppliesTo $Role", ke) )
    return false;
  if( !KheConstraintCheckKml(cons_elt, &id, &name, &reqd, &wt, &cf, ke) )
    return false;

  /* build and insert the constraint object */
  if( !KheAvoidSplitAssignmentsConstraintMake(ins, id, name, reqd, wt, cf,
        KmlExtractText(KmlChild(cons_elt, 5)), &res) )
    return KmlErrorMake(ke, KmlLineNum(cons_elt), KmlColNum(cons_elt),
      "<AvoidSplitAssignmentsConstraint> Id \"%s\" used previously", id);

  /* add the event groups */
  elt = KmlChild(cons_elt, 4);
  if( !KmlCheck(elt, ": EventGroups", ke) )
    return false;
  if( !KheConstraintAddEventGroupsFromKml((KHE_CONSTRAINT) res, elt, ke) )
    return false;
  return true;
}


/*****************************************************************************/
/*                                                                           */
/*  bool KheAvoidSplitAssignmentsConstraintWrite(                            */
/*    KHE_AVOID_SPLIT_ASSIGNMENTS_CONSTRAINT c, KML_FILE kf)                 */
/*                                                                           */
/*  Write c to kf.                                                           */
/*                                                                           */
/*****************************************************************************/

bool KheAvoidSplitAssignmentsConstraintWrite(
  KHE_AVOID_SPLIT_ASSIGNMENTS_CONSTRAINT c, KML_FILE kf)
{
  ASAC_POINT ap;  int i;
  if( c->id == NULL || c->role == NULL )
    return false;
  KmlBegin(kf, "AvoidSplitAssignmentsConstraint");
  KmlAttribute(kf, "Id", c->id);
  if( !KheConstraintWriteCommonFields((KHE_CONSTRAINT) c, kf) )
    return false;
  KmlBegin(kf, "AppliesTo");
  if( MArraySize(c->asac_points) > 0 )
  {
    KmlBegin(kf, "EventGroups");
    MArrayForEach(c->asac_points, &ap, &i)
    {
      if( ap->event_group == NULL )
	return false;
      KmlEltAttribute(kf, "EventGroup", "Reference",
	KheEventGroupId(ap->event_group));
    }
    KmlEnd(kf, "EventGroups");
  }
  KmlEnd(kf, "AppliesTo");
  KmlEltPrintf(kf, "Role", "%s", c->role);
  KmlEnd(kf, "AvoidSplitAssignmentsConstraint");
  return true;
}
