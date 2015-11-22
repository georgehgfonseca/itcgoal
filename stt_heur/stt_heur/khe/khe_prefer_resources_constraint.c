
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
/*  FILE:         khe_prefer_resources_constraint.c                          */
/*  DESCRIPTION:  A prefer resources constraint                              */
/*                                                                           */
/*****************************************************************************/
#include "khe_interns.h"


/*****************************************************************************/
/*                                                                           */
/*  KHE_PREFER_RESOURCES_CONSTRAINT - a prefer resources constraint          */
/*                                                                           */
/*****************************************************************************/

struct khe_prefer_resources_constraint_rec {
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
  KHE_RESOURCE_TYPE		resource_type;		/* resource type     */
  ARRAY_KHE_RESOURCE_GROUP	resource_groups;	/* resource groups   */
  ARRAY_KHE_RESOURCE		resources;		/* resources         */
  KHE_RESOURCE_GROUP		domain; 		/* total of above    */
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
/*  bool KhePreferResourcesConstraintMake(KHE_INSTANCE ins, char *id,        */
/*    char *name, bool required, int weight, KHE_COST_FUNCTION cf,           */
/*    char *role, KHE_PREFER_RESOURCES_CONSTRAINT *c)                        */
/*                                                                           */
/*  Make a prefer resources constraint, add it to the instance, and          */
/*  return it.                                                               */
/*                                                                           */
/*****************************************************************************/

bool KhePreferResourcesConstraintMake(KHE_INSTANCE ins, char *id,
  char *name, bool required, int weight, KHE_COST_FUNCTION cf,
  char *role, KHE_PREFER_RESOURCES_CONSTRAINT *c)
{
  KHE_PREFER_RESOURCES_CONSTRAINT res;  KHE_CONSTRAINT cc;
  MAssert(!KheInstanceComplete(ins),
    "KhePreferResourcesConstraintMake called after KheInstanceMakeEnd");
  if( id != NULL && KheInstanceRetrieveConstraint(ins, id, &cc) )
  {
    *c = NULL;
    return false;
  }
  MMake(res);
  res->back = NULL;
  res->tag = KHE_PREFER_RESOURCES_CONSTRAINT_TAG;
  res->instance = ins;
  res->id = id;
  res->name = name;
  res->required = required;
  res->weight = weight;
  res->combined_weight = required ? KheCost(weight, 0) : KheCost(0, weight);
  res->cost_function = cf;
  res->index = KheInstanceConstraintCount(ins);
  res->role = role;
  res->resource_type = NULL;
  MArrayInit(res->resource_groups);
  MArrayInit(res->resources);
  res->domain = NULL;
  MArrayInit(res->events);
  MArrayInit(res->event_groups);
  MArrayInit(res->event_resources);
  KheInstanceAddConstraint(ins, (KHE_CONSTRAINT) res);
  *c = res;
  return true;
}


/*****************************************************************************/
/*                                                                           */
/*  bool KheEventApplies(KHE_PREFER_RESOURCES_CONSTRAINT c, KHE_EVENT e,     */
/*    KHE_EVENT_RESOURCE *er)                                                */
/*                                                                           */
/*  Return true if e is applicable to c; in that case, also set *er to the   */
/*  event resource it applies to.                                            */
/*                                                                           */
/*****************************************************************************/

static bool KheEventApplies(KHE_PREFER_RESOURCES_CONSTRAINT c, KHE_EVENT e,
  KHE_EVENT_RESOURCE *er)
{
  return c->role != NULL && KheEventRetrieveEventResource(e, c->role, er) &&
    KheEventResourcePreassignedResource(*er) == NULL;
}


/*****************************************************************************/
/*                                                                           */
/*  bool KheResourceTypeIsConsistent(KHE_PREFER_RESOURCES_CONSTRAINT c,      */
/*    KHE_RESOURCE_TYPE rt)                                                  */
/*                                                                           */
/*  Return true if rt is consistent with the resource type of c, possibly    */
/*  setting the resource type of c.                                          */
/*                                                                           */
/*****************************************************************************/

static bool KheResourceTypeIsConsistent(KHE_PREFER_RESOURCES_CONSTRAINT c,
  KHE_RESOURCE_TYPE rt)
{
  if( c->resource_type == NULL )
    c->resource_type = rt;
  return c->resource_type == rt;
}


/*****************************************************************************/
/*                                                                           */
/*  int KhePreferResourcesConstraintAppliesToCount(                          */
/*    KHE_PREFER_RESOURCES_CONSTRAINT c)                                     */
/*                                                                           */
/*  Return the number of points of application of c.                         */
/*                                                                           */
/*****************************************************************************/

int KhePreferResourcesConstraintAppliesToCount(
  KHE_PREFER_RESOURCES_CONSTRAINT c)
{
  return MArraySize(c->event_resources);
}


/*****************************************************************************/
/*                                                                           */
/*  void KhePreferResourcesConstraintFinalize(                               */
/*    KHE_PREFER_RESOURCES_CONSTRAINT c)                                     */
/*                                                                           */
/*  Finalize c, since KheInstanceMakeEnd has been called.                    */
/*                                                                           */
/*  Implementation note.  What needs to be done is to set the domain of c.   */
/*                                                                           */
/*****************************************************************************/

void KhePreferResourcesConstraintFinalize(KHE_PREFER_RESOURCES_CONSTRAINT c)
{
  int i;  KHE_RESOURCE_GROUP rg;  KHE_RESOURCE r;
  if( MArraySize(c->resource_groups) == 0 && MArraySize(c->resources)==1 )
    c->domain = KheResourceSingletonResourceGroup(MArrayFirst(c->resources));
  else if( MArraySize(c->resource_groups) == 1 && MArraySize(c->resources)==0 )
    c->domain = MArrayFirst(c->resource_groups);
  else
  {
    c->domain = KheResourceGroupMakeInternal(KHE_RESOURCE_GROUP_TYPE_USER,
      c->resource_type, NULL, NULL);
    MArrayForEach(c->resource_groups, &rg, &i)
      KheResourceGroupUnionInternal(c->domain, rg);
    MArrayForEach(c->resources, &r, &i)
      KheResourceGroupAddResourceInternal(c->domain, r);
    KheResourceGroupSetResourcesArrayInternal(c->domain);
  }
}


/*****************************************************************************/
/*                                                                           */
/*  Submodule "resource groups"                                              */
/*                                                                           */
/*****************************************************************************/

/*****************************************************************************/
/*                                                                           */
/*  bool KhePreferResourcesConstraintAddResourceGroup(                       */
/*    KHE_PREFER_RESOURCES_CONSTRAINT c, KHE_RESOURCE_GROUP rg)              */
/*                                                                           */
/*  Add rg to c, unless resource type is inconsistent.                       */
/*                                                                           */
/*****************************************************************************/

bool KhePreferResourcesConstraintAddResourceGroup(
  KHE_PREFER_RESOURCES_CONSTRAINT c, KHE_RESOURCE_GROUP rg)
{
  if( !KheResourceTypeIsConsistent(c, KheResourceGroupResourceType(rg)) )
    return false;
  MArrayAddLast(c->resource_groups, rg);
  return true;
}


/*****************************************************************************/
/*                                                                           */
/*  int KhePreferResourcesConstraintResourceGroupCount(                      */
/*    KHE_PREFER_RESOURCES_CONSTRAINT c)                                     */
/*                                                                           */
/*  Return the number of resource groups of c.                               */
/*                                                                           */
/*****************************************************************************/

int KhePreferResourcesConstraintResourceGroupCount(
  KHE_PREFER_RESOURCES_CONSTRAINT c)
{
  return MArraySize(c->resource_groups);
}


/*****************************************************************************/
/*                                                                           */
/*  KHE_RESOURCE_GROUP KhePreferResourcesConstraintResourceGroup(            */
/*    KHE_PREFER_RESOURCES_CONSTRAINT c, int i)                              */
/*                                                                           */
/*  Return the i'th resource group of c.                                     */
/*                                                                           */
/*****************************************************************************/

KHE_RESOURCE_GROUP KhePreferResourcesConstraintResourceGroup(
  KHE_PREFER_RESOURCES_CONSTRAINT c, int i)
{
  return MArrayGet(c->resource_groups, i);
}


/*****************************************************************************/
/*                                                                           */
/*  Submodule "resources"                                                    */
/*                                                                           */
/*****************************************************************************/

/*****************************************************************************/
/*                                                                           */
/*  bool KhePreferResourcesConstraintAddResource(                            */
/*    KHE_PREFER_RESOURCES_CONSTRAINT c, KHE_RESOURCE r)                     */
/*                                                                           */
/*  Add r to c.                                                              */
/*                                                                           */
/*****************************************************************************/

bool KhePreferResourcesConstraintAddResource(
  KHE_PREFER_RESOURCES_CONSTRAINT c, KHE_RESOURCE r)
{
  if( !KheResourceTypeIsConsistent(c, KheResourceResourceType(r)) )
    return false;
  MArrayAddLast(c->resources, r);
  return true;
}


/*****************************************************************************/
/*                                                                           */
/*  int KhePreferResourcesConstraintResourceCount(                           */
/*    KHE_PREFER_RESOURCES_CONSTRAINT c)                                     */
/*                                                                           */
/*  Return the number of resources of c.                                     */
/*                                                                           */
/*****************************************************************************/

int KhePreferResourcesConstraintResourceCount(
  KHE_PREFER_RESOURCES_CONSTRAINT c)
{
  return MArraySize(c->resources);
}


/*****************************************************************************/
/*                                                                           */
/*  KHE_RESOURCE KhePreferResourcesConstraintResource(                       */
/*    KHE_PREFER_RESOURCES_CONSTRAINT c, int i)                              */
/*                                                                           */
/*  Return the i'th resource of c.                                           */
/*                                                                           */
/*****************************************************************************/

KHE_RESOURCE KhePreferResourcesConstraintResource(
  KHE_PREFER_RESOURCES_CONSTRAINT c, int i)
{
  return MArrayGet(c->resources, i);
}


/*****************************************************************************/
/*                                                                           */
/*  Submodule "domain"                                                       */
/*                                                                           */
/*****************************************************************************/

/*****************************************************************************/
/*                                                                           */
/*  KHE_RESOURCE_GROUP KhePreferResourcesConstraintDomain(                   */
/*    KHE_PREFER_RESOURCES_CONSTRAINT c)                                     */
/*                                                                           */
/*  Return the domain of c (the union of its resource groups and resources). */
/*                                                                           */
/*****************************************************************************/

KHE_RESOURCE_GROUP KhePreferResourcesConstraintDomain(
  KHE_PREFER_RESOURCES_CONSTRAINT c)
{
  MAssert(c->domain != NULL,
    "KhePreferResourcesConstraintDomain called before KheInstanceMakeEnd");
  return c->domain;
}


/*****************************************************************************/
/*                                                                           */
/*  Submodule "event resources"                                              */
/*                                                                           */
/*****************************************************************************/

/*****************************************************************************/
/*                                                                           */
/*  bool KhePreferResourcesConstraintAddEventResource(                       */
/*    KHE_PREFER_RESOURCES_CONSTRAINT c, KHE_EVENT_RESOURCE er)              */
/*                                                                           */
/*  Add er to c, and also add c to er.                                       */
/*                                                                           */
/*****************************************************************************/

bool KhePreferResourcesConstraintAddEventResource(
  KHE_PREFER_RESOURCES_CONSTRAINT c, KHE_EVENT_RESOURCE er)
{
  if( !KheResourceTypeIsConsistent(c, KheEventResourceResourceType(er)) )
    return false;
  MArrayAddLast(c->event_resources, er);
  KheEventResourceAddConstraint(er, (KHE_CONSTRAINT) c, -1);
  return true;
}


/*****************************************************************************/
/*                                                                           */
/*  int KhePreferResourcesConstraintEventResourceCount(                      */
/*    KHE_PREFER_RESOURCES_CONSTRAINT c)                                     */
/*                                                                           */
/*  Return the number of event resources that c applies to.                  */
/*                                                                           */
/*****************************************************************************/

int KhePreferResourcesConstraintEventResourceCount(
  KHE_PREFER_RESOURCES_CONSTRAINT c)
{
  return MArraySize(c->event_resources);
}


/*****************************************************************************/
/*                                                                           */
/*  KHE_EVENT_RESOURCE KhePreferResourcesConstraintEventResource(            */
/*    KHE_PREFER_RESOURCES_CONSTRAINT c, int i)                              */
/*                                                                           */
/*  Return the i'th event resource of c.                                     */
/*                                                                           */
/*****************************************************************************/

KHE_EVENT_RESOURCE KhePreferResourcesConstraintEventResource(
  KHE_PREFER_RESOURCES_CONSTRAINT c, int i)
{
  return MArrayGet(c->event_resources, i);
}


/*****************************************************************************/
/*                                                                           */
/*  Submodule "events"                                                       */
/*                                                                           */
/*****************************************************************************/

/*****************************************************************************/
/*                                                                           */
/*  void KhePreferResourcesConstraintAddEvent(                               */
/*    KHE_PREFER_RESOURCES_CONSTRAINT c, KHE_EVENT e)                        */
/*                                                                           */
/*  Add e to c, and also add the corresponding event resource.               */
/*                                                                           */
/*****************************************************************************/

bool KhePreferResourcesConstraintAddEvent(
  KHE_PREFER_RESOURCES_CONSTRAINT c, KHE_EVENT e)
{
  KHE_EVENT_RESOURCE er;
  if( KheEventApplies(c, e, &er) )
  {
    if( !KhePreferResourcesConstraintAddEventResource(c, er) )
      return false;
    MArrayAddLast(c->events, e);
  }
  return true;
}


/*****************************************************************************/
/*                                                                           */
/*  int KhePreferResourcesConstraintEventCount(                              */
/*    KHE_PREFER_RESOURCES_CONSTRAINT c)                                     */
/*                                                                           */
/*  Return the number of events of c.                                        */
/*                                                                           */
/*****************************************************************************/

int KhePreferResourcesConstraintEventCount(KHE_PREFER_RESOURCES_CONSTRAINT c)
{
  return MArraySize(c->events);
}


/*****************************************************************************/
/*                                                                           */
/*  KHE_EVENT KhePreferResourcesConstraintEvent(                             */
/*    KHE_PREFER_RESOURCES_CONSTRAINT c, int i)                              */
/*                                                                           */
/*  Return the i'th event of c.                                              */
/*                                                                           */
/*****************************************************************************/

KHE_EVENT KhePreferResourcesConstraintEvent(
  KHE_PREFER_RESOURCES_CONSTRAINT c, int i)
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
/*  bool KhePreferResourcesConstraintAddEventGroup(                          */
/*    KHE_PREFER_RESOURCES_CONSTRAINT c, KHE_EVENT_GROUP eg,                 */
/*    KHE_EVENT *problem_event)                                              */
/*                                                                           */
/*  Add eg to c, and also add the corresponding event resources to c.        */
/*                                                                           */
/*****************************************************************************/

bool KhePreferResourcesConstraintAddEventGroup(
  KHE_PREFER_RESOURCES_CONSTRAINT c, KHE_EVENT_GROUP eg,
  KHE_EVENT *problem_event)
{
  int i;  KHE_EVENT e;  KHE_EVENT_RESOURCE er;
  for( i = 0;  i < KheEventGroupEventCount(eg);  i++ )
  {
    e = KheEventGroupEvent(eg, i);
    if( KheEventApplies(c, e, &er) )
    {
      if( !KhePreferResourcesConstraintAddEventResource(c, er) )
      {
	*problem_event = e;
	return false;
      }
    }
  }
  MArrayAddLast(c->event_groups, eg);
  *problem_event = NULL;
  return true;
}


/*****************************************************************************/
/*                                                                           */
/*  int KhePreferResourcesConstraintEventGroupCount(                         */
/*    KHE_PREFER_RESOURCES_CONSTRAINT c)                                     */
/*                                                                           */
/*  Return the number of event groups of c.                                  */
/*                                                                           */
/*****************************************************************************/

int KhePreferResourcesConstraintEventGroupCount(
  KHE_PREFER_RESOURCES_CONSTRAINT c)
{
  return MArraySize(c->event_groups);
}


/*****************************************************************************/
/*                                                                           */
/*  KHE_EVENT_GROUP KhePreferResourcesConstraintEventGroup(                  */
/*    KHE_PREFER_RESOURCES_CONSTRAINT c, int i)                              */
/*                                                                           */
/*  Return the ith event group of c.                                         */
/*                                                                           */
/*****************************************************************************/

KHE_EVENT_GROUP KhePreferResourcesConstraintEventGroup(
  KHE_PREFER_RESOURCES_CONSTRAINT c, int i)
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
/*  bool KhePreferResourcesConstraintMakeFromKml(KML_ELT cons_elt,           */
/*    KHE_INSTANCE ins, KML_ERROR *ke)                                       */
/*                                                                           */
/*  Add a prefer resources constraint based on cons_elt to ins.              */
/*                                                                           */
/*****************************************************************************/

bool KhePreferResourcesConstraintMakeFromKml(KML_ELT cons_elt,
  KHE_INSTANCE ins, KML_ERROR *ke)
{
  char *id, *name;  bool reqd;  int wt;  KHE_COST_FUNCTION cf;
  KML_ELT elt;  KHE_PREFER_RESOURCES_CONSTRAINT res;

  /* verify cons_elt and get the common fields */
  if( !KmlCheck(cons_elt, "Id : $Name $Required #Weight "
      "$CostFunction AppliesTo +ResourceGroups +Resources $Role", ke) )
    return false;
  if( !KheConstraintCheckKml(cons_elt, &id, &name, &reqd, &wt, &cf, ke) )
    return false;

  /* make the constraint object and add it to ins */
  if( !KhePreferResourcesConstraintMake(ins, id, name, reqd, wt, cf,
	KmlExtractText(KmlChild(cons_elt, -1)), &res) )
    return KmlErrorMake(ke, KmlLineNum(cons_elt), KmlColNum(cons_elt),
      "<PreferResourcesConstraint> Id \"%s\" used previously", id);

  /* add the event groups and events */
  elt = KmlChild(cons_elt, 4);
  if( !KmlCheck(elt, ": +EventGroups +Events", ke) )
    return false;
  if( !KheConstraintAddEventGroupsFromKml((KHE_CONSTRAINT) res, elt, ke) )
    return false;
  if( !KheConstraintAddEventsFromKml((KHE_CONSTRAINT) res, elt, ke) )
    return false;

  /* add the resource groups and resources */
  if( !KheConstraintAddResourceGroupsFromKml((KHE_CONSTRAINT)res,cons_elt,ke) )
    return false;
  if( !KheConstraintAddResourcesFromKml((KHE_CONSTRAINT) res, cons_elt, ke) )
    return false;
  return true;
}


/*****************************************************************************/
/*                                                                           */
/*  bool KhePreferResourcesConstraintWrite(                                  */
/*    KHE_PREFER_RESOURCES_CONSTRAINT c, KML_FILE kf)                        */
/*                                                                           */
/*  Write c to kf.                                                           */
/*                                                                           */
/*****************************************************************************/

bool KhePreferResourcesConstraintWrite(
  KHE_PREFER_RESOURCES_CONSTRAINT c, KML_FILE kf)
{
  KHE_EVENT_GROUP eg;  KHE_EVENT e;  int i;
  KHE_RESOURCE_GROUP rg;  KHE_RESOURCE r;
  if( c->id == NULL || c->role == NULL )
    return false;
  KmlBegin(kf, "PreferResourcesConstraint");
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
  if( MArraySize(c->resource_groups) > 0 )
  {
    KmlBegin(kf, "ResourceGroups");
    MArrayForEach(c->resource_groups, &rg, &i)
      KmlEltAttribute(kf, "ResourceGroup", "Reference", KheResourceGroupId(rg));
    KmlEnd(kf, "ResourceGroups");
  }
  if( MArraySize(c->resources) > 0 )
  {
    KmlBegin(kf, "Resources");
    MArrayForEach(c->resources, &r, &i)
      KmlEltAttribute(kf, "Resource", "Reference", KheResourceId(r));
    KmlEnd(kf, "Resources");
  }
  KmlEltPrintf(kf, "Role", "%s", c->role);
  KmlEnd(kf, "PreferResourcesConstraint");
  return true;
}

char *KhePreferResourcesConstraintRole( KHE_PREFER_RESOURCES_CONSTRAINT c)
{
   return c->role;
}

