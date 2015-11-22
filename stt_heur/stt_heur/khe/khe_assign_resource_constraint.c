
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
/*  FILE:         khs_assign_resource_constraint.c                           */
/*  DESCRIPTION:  An assign resource constraint                              */
/*                                                                           */
/*****************************************************************************/
#include "khe_interns.h"


/*****************************************************************************/
/*                                                                           */
/*  KHE_ASSIGN_RESOURCE_CONSTRAINT - an assign resource constraint           */
/*                                                                           */
/*****************************************************************************/

struct khe_assign_resource_constraint_rec {
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
  ARRAY_KHE_EVENT_RESOURCE	event_resources;	/* applies to        */
  ARRAY_KHE_EVENT		events;			/* Events            */
  ARRAY_KHE_EVENT_GROUP		event_groups;		/* EventGroups       */
};


/*****************************************************************************/
/*                                                                           */
/*  Submodule "construction and query"                                       */
/*                                                                           */
/*****************************************************************************/

/*****************************************************************************/
/*                                                                           */
/*  bool KheAssignResourceConstraintMake(KHE_INSTANCE ins, char *id,         */
/*    char *name, bool required, int weight, KHE_COST_FUNCTION cf,           */
/*    char *role, KHE_ASSIGN_RESOURCE_CONSTRAINT *c)                         */
/*                                                                           */
/*  Make an assign resource constraint, add it to the instance, and          */
/*  return it.                                                               */
/*                                                                           */
/*****************************************************************************/

bool KheAssignResourceConstraintMake(KHE_INSTANCE ins, char *id,
  char *name, bool required, int weight, KHE_COST_FUNCTION cf,
  char *role, KHE_ASSIGN_RESOURCE_CONSTRAINT *c)
{
  KHE_ASSIGN_RESOURCE_CONSTRAINT res;  KHE_CONSTRAINT cc;
  MAssert(!KheInstanceComplete(ins),
    "KheAssignResourceConstraintMake called after KheInstanceMakeEnd");
  if( id != NULL && KheInstanceRetrieveConstraint(ins, id, &cc) )
  {
    *c = NULL;
    return false;
  }
  MMake(res);
  res->back = NULL;
  res->tag = KHE_ASSIGN_RESOURCE_CONSTRAINT_TAG;
  res->instance = ins;
  res->id = id;
  res->name = name;
  res->required = required;
  res->weight = weight;
  res->combined_weight = required ? KheCost(weight, 0) : KheCost(0, weight);
  res->cost_function = cf;
  res->index = KheInstanceConstraintCount(ins);
  res->role = role;
  MArrayInit(res->events);
  MArrayInit(res->event_groups);
  MArrayInit(res->event_resources);
  KheInstanceAddConstraint(ins, (KHE_CONSTRAINT) res);
  *c = res;
  return true;
}


/*****************************************************************************/
/*                                                                           */
/*  char *KheAssignResourceConstraintRole(KHE_ASSIGN_RESOURCE_CONSTRAINT c)  */
/*                                                                           */
/*  Return the role attribute of c (possibly NULL).                          */
/*                                                                           */
/*****************************************************************************/

char *KheAssignResourceConstraintRole(KHE_ASSIGN_RESOURCE_CONSTRAINT c)
{
  return c->role;
}


/*****************************************************************************/
/*                                                                           */
/*  int KheAssignResourceConstraintAppliesToCount(                           */
/*    KHE_ASSIGN_RESOURCE_CONSTRAINT c)                                      */
/*                                                                           */
/*  Return the number of points of application of c.                         */
/*                                                                           */
/*****************************************************************************/

int KheAssignResourceConstraintAppliesToCount(KHE_ASSIGN_RESOURCE_CONSTRAINT c)
{
  return MArraySize(c->event_resources);
}


/*****************************************************************************/
/*                                                                           */
/* void KheAssignResourceConstraintFinalize(KHE_ASSIGN_RESOURCE_CONSTRAINT c)*/
/*                                                                           */
/*  Finalize c, since KheInstanceMakeEnd has been called.                    */
/*                                                                           */
/*****************************************************************************/

void KheAssignResourceConstraintFinalize(KHE_ASSIGN_RESOURCE_CONSTRAINT c)
{
  /* nothing to do in this case */
}


/*****************************************************************************/
/*                                                                           */
/*  Submodule "event resources"                                              */
/*                                                                           */
/*****************************************************************************/

/*****************************************************************************/
/*                                                                           */
/*  void KheAssignResourceConstraintAddEventResource(                        */
/*    KHE_ASSIGN_RESOURCE_CONSTRAINT c, KHE_EVENT_RESOURCE er)               */
/*                                                                           */
/*  Add er to c, and also add c to er.                                       */
/*                                                                           */
/*****************************************************************************/

void KheAssignResourceConstraintAddEventResource(
  KHE_ASSIGN_RESOURCE_CONSTRAINT c, KHE_EVENT_RESOURCE er)
{
  MArrayAddLast(c->event_resources, er);
  KheEventResourceAddConstraint(er, (KHE_CONSTRAINT) c, -1);
}


/*****************************************************************************/
/*                                                                           */
/*  int KheAssignResourceConstraintEventResourceCount(                       */
/*    KHE_ASSIGN_RESOURCE_CONSTRAINT c)                                      */
/*                                                                           */
/*  Return the number of event resources that c applies to.                  */
/*                                                                           */
/*****************************************************************************/

int KheAssignResourceConstraintEventResourceCount(
  KHE_ASSIGN_RESOURCE_CONSTRAINT c)
{
  return MArraySize(c->event_resources);
}


/*****************************************************************************/
/*                                                                           */
/*  KHE_EVENT_RESOURCE KheAssignResourceConstraintEventResource(             */
/*    KHE_ASSIGN_RESOURCE_CONSTRAINT c, int i)                               */
/*                                                                           */
/*  Return the i'th event resource of c.                                     */
/*                                                                           */
/*****************************************************************************/

KHE_EVENT_RESOURCE KheAssignResourceConstraintEventResource(
  KHE_ASSIGN_RESOURCE_CONSTRAINT c, int i)
{
  return MArrayGet(c->event_resources, i);
}


/*****************************************************************************/
/*                                                                           */
/*  void AddEventResource(KHE_ASSIGN_RESOURCE_CONSTRAINT c, KHE_EVENT e)     */
/*                                                                           */
/*  Add the event resource corresponding to e to c, if any.                  */
/*                                                                           */
/*****************************************************************************/

static void AddEventResource(KHE_ASSIGN_RESOURCE_CONSTRAINT c, KHE_EVENT e)
{
  KHE_EVENT_RESOURCE er;
  if( c->role != NULL && KheEventRetrieveEventResource(e, c->role, &er) &&
      KheEventResourcePreassignedResource(er) == NULL )
    KheAssignResourceConstraintAddEventResource(c, er);
}


/*****************************************************************************/
/*                                                                           */
/*  Submodule "events"                                                       */
/*                                                                           */
/*****************************************************************************/

/*****************************************************************************/
/*                                                                           */
/*  void KheAssignResourceConstraintAddEvent(                                */
/*    KHE_ASSIGN_RESOURCE_CONSTRAINT c, KHE_EVENT e)                         */
/*                                                                           */
/*  Add e to c, and also add the corresponding event resource.               */
/*                                                                           */
/*****************************************************************************/

void KheAssignResourceConstraintAddEvent(
  KHE_ASSIGN_RESOURCE_CONSTRAINT c, KHE_EVENT e)
{
  MArrayAddLast(c->events, e);
  AddEventResource(c, e);
}


/*****************************************************************************/
/*                                                                           */
/*  int KheAssignResourceConstraintEventCount(                               */
/*    KHE_ASSIGN_RESOURCE_CONSTRAINT c)                                      */
/*                                                                           */
/*  Return the number of events of c.                                        */
/*                                                                           */
/*****************************************************************************/

int KheAssignResourceConstraintEventCount(
  KHE_ASSIGN_RESOURCE_CONSTRAINT c)
{
  return MArraySize(c->events);
}


/*****************************************************************************/
/*                                                                           */
/*  KHE_EVENT KheAssignResourceConstraintEvent(                              */
/*    KHE_ASSIGN_RESOURCE_CONSTRAINT c, int i)                               */
/*                                                                           */
/*  Return the i'th event of c.                                              */
/*                                                                           */
/*****************************************************************************/

KHE_EVENT KheAssignResourceConstraintEvent(
  KHE_ASSIGN_RESOURCE_CONSTRAINT c, int i)
{
  return MArrayGet(c->events, i);
}


/*****************************************************************************/
/*                                                                           */
/*  Submodule "event groups"                                                 */
/*                                                                           */
/*****************************************************************************/

/*****************************************************************************/
/*                                                                           */
/*  void KheAssignResourceConstraintAddEventGroup(                           */
/*    KHE_ASSIGN_RESOURCE_CONSTRAINT c, KHE_EVENT_GROUP eg)                  */
/*                                                                           */
/*  Add eg to c, and also add the corresponding event resources to c.        */
/*                                                                           */
/*****************************************************************************/

void KheAssignResourceConstraintAddEventGroup(
  KHE_ASSIGN_RESOURCE_CONSTRAINT c, KHE_EVENT_GROUP eg)
{
  int i;
  MArrayAddLast(c->event_groups, eg);
  for( i = 0;  i < KheEventGroupEventCount(eg);  i++ )
    AddEventResource(c, KheEventGroupEvent(eg, i));
}


/*****************************************************************************/
/*                                                                           */
/*  int KheAssignResourceConstraintEventGroupCount(                          */
/*    KHE_ASSIGN_RESOURCE_CONSTRAINT c)                                      */
/*                                                                           */
/*  Return the number of event groups of c.                                  */
/*                                                                           */
/*****************************************************************************/

int KheAssignResourceConstraintEventGroupCount(
  KHE_ASSIGN_RESOURCE_CONSTRAINT c)
{
  return MArraySize(c->event_groups);
}


/*****************************************************************************/
/*                                                                           */
/*  KHE_EVENT_GROUP KheAssignResourceConstraintEventGroup(                   */
/*    KHE_ASSIGN_RESOURCE_CONSTRAINT c, int i)                               */
/*                                                                           */
/*  Return the ith event group of c.                                         */
/*                                                                           */
/*****************************************************************************/

KHE_EVENT_GROUP KheAssignResourceConstraintEventGroup(
  KHE_ASSIGN_RESOURCE_CONSTRAINT c, int i)
{
  return MArrayGet(c->event_groups, i);
}


/*****************************************************************************/
/*                                                                           */
/*  Submodule "reading and writing"                                          */
/*                                                                           */
/*****************************************************************************/

/*****************************************************************************/
/*                                                                           */
/*  bool KheAssignResourceConstraintMakeFromKml(KML_ELT cons_elt,            */
/*    KHE_INSTANCE ins, KML_ERROR *ke)                                       */
/*                                                                           */
/*  Make an assign resource constraint from cons_elt and add it to ins.      */
/*                                                                           */
/*****************************************************************************/

bool KheAssignResourceConstraintMakeFromKml(KML_ELT cons_elt,
  KHE_INSTANCE ins, KML_ERROR *ke)
{
  char *id, *name;  bool reqd;  int wt;  KHE_COST_FUNCTION cf;
  KML_ELT elt;  KHE_ASSIGN_RESOURCE_CONSTRAINT res;

  /* check cons_elt and get the common fields */
  if( !KmlCheck(cons_elt,
	"Id : $Name $Required #Weight $CostFunction AppliesTo $Role", ke) )
    return false;
  if( !KheConstraintCheckKml(cons_elt, &id, &name, &reqd, &wt, &cf, ke) )
    return false;

  /* make the constraint object and add it to ins */
  if( !KheAssignResourceConstraintMake(ins, id, name, reqd, wt, cf,
	KmlExtractText(KmlChild(cons_elt, 5)), &res) )
    return KmlErrorMake(ke, KmlLineNum(cons_elt), KmlColNum(cons_elt),
      "<AssignResourceConstraint> Id \"%s\" used previously", id);

  /* add the event groups and events */
  elt = KmlChild(cons_elt, 4);
  if( !KmlCheck(elt, ": +EventGroups +Events", ke) )
    return false;
  if( !KheConstraintAddEventGroupsFromKml((KHE_CONSTRAINT) res, elt, ke) )
    return false;
  if( !KheConstraintAddEventsFromKml((KHE_CONSTRAINT) res, elt, ke) )
    return false;
  return true;
}


/*****************************************************************************/
/*                                                                           */
/*  bool KheAssignResourceConstraintWrite(KHE_ASSIGN_RESOURCE_CONSTRAINT c,  */
/*    KML_FILE kf)                                                           */
/*                                                                           */
/*  Write c to kf.                                                           */
/*                                                                           */
/*****************************************************************************/

bool KheAssignResourceConstraintWrite(KHE_ASSIGN_RESOURCE_CONSTRAINT c,
  KML_FILE kf)
{
  KHE_EVENT_GROUP eg;  KHE_EVENT e;  int i;
  if( c->id == NULL || c->role == NULL )
    return false;
  KmlBegin(kf, "AssignResourceConstraint");
  KmlAttribute(kf, "Id", c->id);
  if( !KheConstraintWriteCommonFields((KHE_CONSTRAINT) c, kf) )
    return false;
  KmlBegin(kf, "AppliesTo");
  if( MArraySize(c->event_groups) > 0 )
  {
    KmlBegin(kf, "EventGroups");
    MArrayForEach(c->event_groups, &eg, &i)
      KmlEltAttribute(kf, "EventGroup", "Reference", KheEventGroupId(eg));
    KmlEnd(kf, "EventGroups");
  }
  if( MArraySize(c->events) > 0 )
  {
    KmlBegin(kf, "Events");
    MArrayForEach(c->events, &e, &i)
      KmlEltAttribute(kf, "Event", "Reference", KheEventId(e));
    KmlEnd(kf, "Events");
  }
  KmlEnd(kf, "AppliesTo");
  KmlEltPrintf(kf, "Role", "%s", c->role);
  KmlEnd(kf, "AssignResourceConstraint");
  return true;
}
