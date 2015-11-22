
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
/*  FILE:         khe_event_resource.c                                       */
/*  DESCRIPTION:  A resource member of an event                              */
/*                                                                           */
/*****************************************************************************/
#include "khe_interns.h"

#define DEBUG1 0

/*****************************************************************************/
/*                                                                           */
/*  KHE_EVENT_RESOURCE - a resource member of an event                       */
/*                                                                           */
/*****************************************************************************/

struct khe_event_resource_rec {
  void			*back;			/* back pointer              */
  KHE_EVENT		event;			/* enclosing event           */
  KHE_EVENT_RESOURCE_GROUP event_resource_group; /* optional erg             */
  KHE_RESOURCE_TYPE	resource_type;		/* resource type             */
  KHE_RESOURCE		preassigned_resource;	/* preassigned resource      */
  char			*role;			/* optional role             */
  int			workload;		/* workload (compulsory!)    */
  int			index_in_event;		/* index in event            */
  int			index_in_instance;	/* index in instance         */
  ARRAY_KHE_CONSTRAINT	constraints;		/* constraints on this e.r.  */
  ARRAY_INT		constraint_eg_indexes;	/* eg_indexes of asac's      */
  KHE_RESOURCE_GROUP	hard_domain;		/* suitable domain for e.r.  */
  KHE_RESOURCE_GROUP	hard_and_soft_domain;	/* suitable domain for e.r.  */
};


/*****************************************************************************/
/*                                                                           */
/*  Submodule "construction and query"                                       */
/*                                                                           */
/*****************************************************************************/

/*****************************************************************************/
/*                                                                           */
/*  bool KheEventResourceMake(KHE_EVENT event, KHE_RESOURCE_TYPE rt,         */
/*    KHE_RESOURCE preassigned_resource, char *role, KHE_EVENT_RESOURCE *er) */
/*                                                                           */
/*  Make a new event resource and add it to event.                           */
/*                                                                           */
/*****************************************************************************/

bool KheEventResourceMake(KHE_EVENT event, KHE_RESOURCE_TYPE rt,
  KHE_RESOURCE preassigned_resource, char *role, int workload,
  KHE_EVENT_RESOURCE *er)
{
  KHE_EVENT_RESOURCE res;  KHE_INSTANCE ins;
  MAssert(event != NULL, "KheEventResourceMake: event is NULL");
  MAssert(preassigned_resource == NULL ||
    KheResourceResourceType(preassigned_resource) == rt,
    "KheEventResourceMake given preassigned resource of wrong resource type");
  if( role != NULL && KheEventRetrieveEventResource(event, role, &res) )
  {
    *er = NULL;
    return false;
  }
  MMake(res);
  res->back = NULL;
  res->event = event;
  res->event_resource_group = NULL;
  res->resource_type = rt;
  res->preassigned_resource = preassigned_resource;
  if( preassigned_resource == NULL )
    KheResourceTypeDemandNotAllPreassigned(rt);
  res->role = role;
  res->workload = workload;
  if( preassigned_resource != NULL )
    KheResourceAddPreassignedEventResource(preassigned_resource, res);
  res->hard_domain = NULL;  /* set later, when finalizing */
  res->hard_and_soft_domain = NULL;  /* set later, when finalizing */
  MArrayInit(res->constraints);
  MArrayInit(res->constraint_eg_indexes);
  KheEventAddEventResource(event, res, &res->index_in_event);
  ins = KheEventInstance(event);
  res->index_in_instance = KheInstanceEventResourceCount(ins);
  KheInstanceAddEventResource(ins, res);
  *er = res;
  return true;
}


/*****************************************************************************/
/*                                                                           */
/*  void KheEventResourceSetBack(KHE_EVENT_RESOURCE er, void *back)          */
/*                                                                           */
/*  Set the back pointer of er.                                              */
/*                                                                           */
/*****************************************************************************/

void KheEventResourceSetBack(KHE_EVENT_RESOURCE er, void *back)
{
  MAssert(!KheInstanceComplete(KheEventInstance(er->event)),
    "KheEventResourceSetBack called after KheInstanceMakeEnd");
  er->back = back;
}


/*****************************************************************************/
/*                                                                           */
/*  void *KheEventResourceBack(KHE_EVENT_RESOURCE er)                        */
/*                                                                           */
/*  Return the back pointer of er.                                           */
/*                                                                           */
/*****************************************************************************/

void *KheEventResourceBack(KHE_EVENT_RESOURCE er)
{
  return er->back;
}


/*****************************************************************************/
/*                                                                           */
/*  KHE_EVENT KheEventResourceEvent(KHE_EVENT_RESOURCE er)                   */
/*                                                                           */
/*  Return the event containing er.                                          */
/*                                                                           */
/*****************************************************************************/

KHE_EVENT KheEventResourceEvent(KHE_EVENT_RESOURCE er)
{
  return er->event;
}


/*****************************************************************************/
/*                                                                           */
/*  KHE_RESOURCE_TYPE KheEventResourceResourceType(KHE_EVENT_RESOURCE er)    */
/*                                                                           */
/*  Return the resource type of er.                                          */
/*                                                                           */
/*****************************************************************************/

KHE_RESOURCE_TYPE KheEventResourceResourceType(KHE_EVENT_RESOURCE er)
{
  return er->resource_type;
}


/*****************************************************************************/
/*                                                                           */
/*  KHE_RESOURCE KheEventResourcePreassignedResource(KHE_EVENT_RESOURCE er)  */
/*                                                                           */
/*  Return the preassigned resource attribute of er.  This will be NULL      */
/*  if there is no preassigned resource.                                     */
/*                                                                           */
/*****************************************************************************/

KHE_RESOURCE KheEventResourcePreassignedResource(KHE_EVENT_RESOURCE er)
{
  return er->preassigned_resource;
}


/*****************************************************************************/
/*                                                                           */
/*  char *KheEventResourceRole(KHE_EVENT_RESOURCE er)                        */
/*                                                                           */
/*  Return the role attribute of er.                                         */
/*                                                                           */
/*****************************************************************************/

int KheEventResourceHasRole(KHE_EVENT_RESOURCE er)
{
   return er->role != NULL;
}


char *KheEventResourceRole(KHE_EVENT_RESOURCE er)
{
  return er->role;
}


/*****************************************************************************/
/*                                                                           */
/*  int KheEventResourceWorkload(KHE_EVENT_RESOURCE er)                      */
/*                                                                           */
/*  Return the workload of er.                                               */
/*                                                                           */
/*****************************************************************************/

int KheEventResourceWorkload(KHE_EVENT_RESOURCE er)
{
  return er->workload;
}


/*****************************************************************************/
/*                                                                           */
/*  int KheEventResourceIndexInEvent(KHE_EVENT_RESOURCE er)                  */
/*                                                                           */
/*  Return the index number of er in the enclosing event.                    */
/*                                                                           */
/*****************************************************************************/

int KheEventResourceIndexInEvent(KHE_EVENT_RESOURCE er)
{
  return er->index_in_event;
}


/*****************************************************************************/
/*                                                                           */
/*  int KheEventResourceIndexInInstance(KHE_EVENT_RESOURCE er)               */
/*                                                                           */
/*  Return the index number of er in the enclosing instance.                 */
/*                                                                           */
/*****************************************************************************/

int KheEventResourceIndexInInstance(KHE_EVENT_RESOURCE er)
{
  return er->index_in_instance;
}


/*****************************************************************************/
/*                                                                           */
/*  Submodule "event resource group"                                         */
/*                                                                           */
/*****************************************************************************/

/*****************************************************************************/
/*                                                                           */
/*  void KheEventResourceSetEventResourceGroup(KHE_EVENT_RESOURCE er,        */
/*    KHE_EVENT_RESOURCE_GROUP erg)                                          */
/*                                                                           */
/*  Sete the enclosing event resource group of er to erg.                    */
/*                                                                           */
/*****************************************************************************/

void KheEventResourceSetEventResourceGroup(KHE_EVENT_RESOURCE er,
  KHE_EVENT_RESOURCE_GROUP erg)
{
  er->event_resource_group = erg;
}


/*****************************************************************************/
/*                                                                           */
/*  KHE_EVENT_RESOURCE_GROUP KheEventResourceEventResourceGroup(             */
/*    KHE_EVENT_RESOURCE er)                                                 */
/*                                                                           */
/*  Return the enclosing event resource group of er, or NULL if none.        */
/*                                                                           */
/*****************************************************************************/

KHE_EVENT_RESOURCE_GROUP KheEventResourceEventResourceGroup(
  KHE_EVENT_RESOURCE er)
{
  return er->event_resource_group;
}


/*****************************************************************************/
/*                                                                           */
/*  Submodule "constraints"                                                  */
/*                                                                           */
/*****************************************************************************/

/*****************************************************************************/
/*                                                                           */
/*  void KheEventResourceAddConstraint(KHE_EVENT_RESOURCE er,                */
/*    KHE_CONSTRAINT c)                                                      */
/*                                                                           */
/*  Add c to er.                                                             */
/*                                                                           */
/*****************************************************************************/

void KheEventResourceAddConstraint(KHE_EVENT_RESOURCE er, KHE_CONSTRAINT c,
  int eg_index)
{
  MArrayAddLast(er->constraints, c);
  MArrayAddLast(er->constraint_eg_indexes, eg_index);
}


/*****************************************************************************/
/*                                                                           */
/*  int KheEventResourceConstraintCount(KHE_EVENT_RESOURCE er)               */
/*                                                                           */
/*  Return the number of constraints applicable to er.                       */
/*                                                                           */
/*****************************************************************************/

int KheEventResourceConstraintCount(KHE_EVENT_RESOURCE er)
{
  return MArraySize(er->constraints);
}


/*****************************************************************************/
/*                                                                           */
/*  KHE_CONSTRAINT KheEventResourceConstraint(KHE_EVENT_RESOURCE er, int i)  */
/*                                                                           */
/*  Return the i'th constraint applicable to er.                             */
/*                                                                           */
/*****************************************************************************/

KHE_CONSTRAINT KheEventResourceConstraint(KHE_EVENT_RESOURCE er, int i)
{
  return MArrayGet(er->constraints, i);
}


/*****************************************************************************/
/*                                                                           */
/*  int KheEventResourceConstraintEventGroupIndex(KHE_EVENT_RESOURCE er,     */
/*    int i)                                                                 */
/*                                                                           */
/*  Return the event group index of the i'th constraint applicable to er,    */
/*  if it is an avoid split assignments constraint; otherwise return -1.     */
/*                                                                           */
/*****************************************************************************/

int KheEventResourceConstraintEventGroupIndex(KHE_EVENT_RESOURCE er, int i)
{
  return MArrayGet(er->constraint_eg_indexes, i);
}


/*****************************************************************************/
/*                                                                           */
/*  Submodule "resource domains"                                             */
/*                                                                           */
/*****************************************************************************/

/*****************************************************************************/
/*                                                                           */
/*  void KheEventResourceSetDomain(KHE_EVENT_RESOURCE er, bool hard_and_soft)*/
/*                                                                           */
/*  Set the hard domain (or the hard_and_soft domain if hard_and_soft is     */
/*  true) of er and all event resources linked to er.                        */
/*                                                                           */
/*****************************************************************************/

static void KheEventResourceSetDomain(KHE_EVENT_RESOURCE er, bool hard_and_soft)
{
  ARRAY_KHE_EVENT_RESOURCE event_resources;  KHE_EVENT_RESOURCE er2;
  KHE_CONSTRAINT c;  KHE_AVOID_SPLIT_ASSIGNMENTS_CONSTRAINT asac;
  KHE_RESOURCE_GROUP domain, prc_domain;  int i, j, k, egi, pos, count, x;

  if( DEBUG1 )
  {
    fprintf(stderr, "[ KheEventResourceFinalize(");
    KheEventResourceDebug(er, 1, -1, stderr);
    fprintf(stderr, ", %s)\n", hard_and_soft ? "true" : "false");
  }

  /* start out with every resource in the resource group */
  domain = KheResourceGroupMakeInternal(KHE_RESOURCE_GROUP_TYPE_USER,
    er->resource_type, NULL, NULL);
  KheResourceGroupUnionInternal(domain,
    KheResourceTypeFullResourceGroup(er->resource_type));
  count = 0;
  prc_domain = NULL;

  /* build the set of linked event resources, and their domain */
  MArrayInit(event_resources);
  MArrayAddLast(event_resources, er);
  MArrayForEach(event_resources, &er, &i)
  {
    if( DEBUG1 )
    {
      fprintf(stderr, "  [ ");
      KheEventResourceDebug(er, 1, 0, stderr);
    }
    if( er->preassigned_resource != NULL )
    {
      prc_domain = KheResourceSingletonResourceGroup(er->preassigned_resource);
      if( !KheResourceGroupSubset(domain, prc_domain) )
      {
	count++;
	KheResourceGroupIntersectInternal(domain, prc_domain);
	if( DEBUG1 )
	{
	  fprintf(stderr, "    %d: preassigned intersect ", count);
	  KheResourceGroupDebug(prc_domain, 1, 0, stderr);
	}
      }
    }
    MArrayForEach(er->constraints, &c, &j)
    {
      if( DEBUG1 )
	KheConstraintDebug(c, 1, 4, stderr);
      if( KheConstraintWeight(c) > 0 &&
	  (hard_and_soft || KheConstraintRequired(c)) )
	switch( KheConstraintTag(c) )
	{
	  case KHE_ASSIGN_RESOURCE_CONSTRAINT_TAG:

	    /* not relevant to this operation */
	    break;

	  case KHE_PREFER_RESOURCES_CONSTRAINT_TAG:

	    /* intersect the domain of this constraint with domain */
	    prc_domain = KhePreferResourcesConstraintDomain(
	      (KHE_PREFER_RESOURCES_CONSTRAINT) c);
	    if( !KheResourceGroupSubset(domain, prc_domain) )
	    {
	      count++;
	      KheResourceGroupIntersectInternal(domain, prc_domain);
	      if( DEBUG1 )
	      {
		fprintf(stderr, "    %d: intersect ", count);
		KheResourceGroupDebug(prc_domain, 1, 0, stderr);
	      }
	    }
	    break;

	  case KHE_AVOID_SPLIT_ASSIGNMENTS_CONSTRAINT_TAG:

	    /* find other event resources linked to er by this constraint */
	    asac = (KHE_AVOID_SPLIT_ASSIGNMENTS_CONSTRAINT) c;
	    egi = KheEventResourceConstraintEventGroupIndex(er, j);
	    x =KheAvoidSplitAssignmentsConstraintEventResourceCount(asac,egi);
	    for( k = 0;  k < x;  k++ )
	    {
	      er2=KheAvoidSplitAssignmentsConstraintEventResource(asac,egi,k);
	      if( !MArrayContains(event_resources, er2, &pos) )
	      {
		MArrayAddLast(event_resources, er2);
		if( DEBUG1 )
		{
		  fprintf(stderr, "    add ");
		  KheEventResourceDebug(er2, 1, 0, stderr);
		}
	      }
	    }
	    break;

	  default:

	    MAssert(false, "KheEventResourceFindDomain internal error 1");
	}
    }
    if( DEBUG1 )
      fprintf(stderr, "  ]\n");
  }

  /* choose the domain depending on how many resource groups were intersected */
  if( count == 0 )
  {
    /* no resource groups, so the full resource group is what we need */
    KheResourceGroupDelete(domain);
    domain = KheResourceTypeFullResourceGroup(er->resource_type);
    if( DEBUG1 )
    {
      fprintf(stderr, "  final count zero: ");
      KheResourceGroupDebug(domain, 1, 0, stderr);
    }
  }
  else if( count == 1 )
  {
    /* one resource group, so that one is what we need */
    KheResourceGroupDelete(domain);
    domain = prc_domain;
    if( DEBUG1 )
    {
      fprintf(stderr, "  final count one: ");
      KheResourceGroupDebug(domain, 1, 0, stderr);
    }
  }
  else
  {
    /* more than one resource group, so we need a genuinely new group */
    KheResourceGroupSetResourcesArrayInternal(domain);
    if( DEBUG1 )
    {
      fprintf(stderr, "  final count %d: ", count);
      KheResourceGroupDebug(domain, 1, 0, stderr);
    }
  }

  /* set the domain of every event resource touched here */
  if( hard_and_soft )
  {
    MArrayForEach(event_resources, &er, &i)
    {
      MAssert(er->hard_and_soft_domain == NULL,
	"KheEventResourceFindDomain internal error 2");
      er->hard_and_soft_domain = domain;
    }
  }
  else
  {
    MArrayForEach(event_resources, &er, &i)
    {
      MAssert(er->hard_domain == NULL,
	"KheEventResourceFindDomain internal error 3");
      er->hard_domain = domain;
    }
  }
  MArrayFree(event_resources);
  if( DEBUG1 )
    fprintf(stderr, "] KheEventResourceFinalize returning\n");
}


/*****************************************************************************/
/*                                                                           */
/*  void KheEventResourceFinalize(KHE_EVENT_RESOURCE er)                     */
/*                                                                           */
/*  Finalize er.  This means to initialize its domain attributes.            */
/*                                                                           */
/*****************************************************************************/

void KheEventResourceFinalize(KHE_EVENT_RESOURCE er)
{
  if( er->hard_domain == NULL )
    KheEventResourceSetDomain(er, false);
  if( er->hard_and_soft_domain == NULL )
    KheEventResourceSetDomain(er, true);
}


/* *** obsolete version
void KheEventResourceFinalize(KHE_EVENT_RESOURCE er)
{
  KHE_CONSTRAINT c;  KHE_PREFER_RESOURCES_CONSTRAINT prc;  int i, count;
  if( er->preassigned_resource != NULL )
    er->domain = KheResourceSingletonResourceGroup(er->preassigned_resource);
  else
  {
    ** find the number of unbreakable prefer resources constraints, and prc **
    count = 0;
    prc = NULL;  ** keep compiler happy **
    MArrayForEach(er->constraints, &c, &i)
      if( KheConstraintTag(c) == KHE_PREFER_RESOURCES_CONSTRAINT_TAG &&
	  KheConstraintRequired(c) && KheConstraintWeight(c) > 0 )
      {
	prc = KheToPreferResourcesConstraint(c);
	count++;
      }
    if( count == 0 )
    {
      ** no constraints, domain is the full set of er's type **
      er->domain = KheResourceTypeFullResourceGroup(er->resource_type);
    }
    else if( count == 1 )
    {
      ** one constraint, domain is its resource group **
      er->domain = KhePreferResourcesConstraintDomain(prc);
    }
    else
    {
      ** have to intersect them all **
      KheResourceGroupMake(er->resource_type, NULL, NULL, false, &er->domain);
      MArrayForEach(er->constraints, &c, &i)
	if( KheConstraintTag(c) == KHE_PREFER_RESOURCES_CONSTRAINT_TAG &&
	    KheConstraintRequired(c) && KheConstraintWeight(c) > 0 )
	{
	  prc = KheToPreferResourcesConstraint(c);
	  KheResourceGroupIntersectInternal(er->domain,
            KhePreferResourcesConstraintDomain(prc));
	}
      KheResourceGroupSetResourcesArrayInternal(er->domain);
    }
  }
}
*** */


/*****************************************************************************/
/*                                                                           */
/*  KHE_RESOURCE_GROUP KheEventResourceHardDomain(KHE_EVENT_RESOURCE er)     */
/*                                                                           */
/*  Return a suitable domain for er, taking hard prefer resources and        */
/*  avoid split assignments constraints into account.  Constraints of        */
/*  weight 0 are ignored.                                                    */
/*                                                                           */
/*****************************************************************************/

KHE_RESOURCE_GROUP KheEventResourceHardDomain(KHE_EVENT_RESOURCE er)
{
  MAssert(er->hard_domain != NULL,
    "KheEventResourceHardDomain called before KheInstanceMakeEnd");
  return er->hard_domain;
}


/*****************************************************************************/
/*                                                                           */
/*  KHE_RESOURCE_GROUP KheEventResourceHardAndSoftDomain(                    */
/*    KHE_EVENT_RESOURCE er)                                                 */
/*                                                                           */
/*  Return a suitable domain for er, taking hard and soft prefer resources   */
/*  and avoid split assignments constraints into account.  Constraints of    */
/*  weight 0 are ignored.                                                    */
/*                                                                           */
/*****************************************************************************/

KHE_RESOURCE_GROUP KheEventResourceHardAndSoftDomain(
  KHE_EVENT_RESOURCE er)
{
  MAssert(er->hard_and_soft_domain != NULL,
    "KheEventResourceHardAndSoftDomain called before KheInstanceMakeEnd");
  return er->hard_and_soft_domain;
}


/*****************************************************************************/
/*                                                                           */
/*  KHE_RESOURCE_GROUP KheEventResourceDomain(KHE_EVENT_RESOURCE er)         */
/*                                                                           */
/*  Return a suitable domain for er, in the form of a resource group.        */
/*                                                                           */
/*****************************************************************************/

/* ***
KHE_RESOURCE_GROUP KheEventResourceDomain(KHE_EVENT_RESOURCE er)
{
  MAssert(er->domain != NULL,
    "KheEventResourceDomain called before KheInstanceMakeEnd");
  return er->domain;
  KHE_RESOURCE_GROUP rg;  int i;  KHE_CONSTRAINT c;
  KHE_PREFER_RESOURCES_CONSTRAINT prc;
  KheSolnResourceGroupBegin(soln, er->resource_type, NULL);
  MArrayForEach(er->constraints, &c, &i)
    if( KheConstraintTag(c) == KHE_PREFER_RESOURCES_CONSTRAINT_TAG &&
	KheConstraintStrength(c) == KHE_STRENGTH_UNBREAKABLE )
    {
      prc = KheToPreferResourcesConstraint(c);
      rg = KhePreferResourcesConstraintResourceGroup(prc);
      KheSolnResourceGroupIntersect(soln, rg);
    }
  if( er->preassigned_resource != NULL )
  {
    rg = KheResourceGroupSingle(er->preassigned_resource);
    KheSolnResourceGroupIntersect(soln, rg);
  }
  return KheSolnResourceGroupEnd(soln);
}
*** */


/*****************************************************************************/
/*                                                                           */
/*  Submodule "reading and writing"                                          */
/*                                                                           */
/*****************************************************************************/

/*****************************************************************************/
/*                                                                           */
/*  bool KheEventResourceMakeFromKml(KML_ELT resource_elt,                   */
/*    KHE_EVENT e, KML_ERROR *ke)                                            */
/*                                                                           */
/*  Make an event resource from resource_elt and add it to e.                */
/*                                                                           */
/*****************************************************************************/

bool KheEventResourceMakeFromKml(KML_ELT resource_elt, KHE_EVENT e,
  KML_ERROR *ke)
{
  KML_ELT resource_type_elt, role_elt, workload_elt;  char *ref, *role;
  KHE_RESOURCE preassigned_resource;  KHE_RESOURCE_TYPE rt;  int workload;
  KHE_EVENT_RESOURCE er;
  if( !KmlCheck(resource_elt,
	"+Reference : +$Role +ResourceType +#Workload", ke) )
    return false;

  /* optional preassigned resource */
  if( KmlAttributeCount(resource_elt) == 1 )
  {
    ref = KmlAttributeValue(resource_elt, 0);
    if( !KheInstanceRetrieveResource(KheEventInstance(e), ref,
	  &preassigned_resource) )
      return KmlErrorMake(ke, KmlLineNum(resource_elt),
	KmlColNum(resource_elt),
	"in <Resource>, Reference \"%s\" unknown", ref);
  }
  else
    preassigned_resource = NULL;

  /* optional role */
  if( KmlContainsChild(resource_elt, "Role", &role_elt) )
    role = KmlExtractText(role_elt);
  else
  {
    if( preassigned_resource == NULL )
      return KmlErrorMake(ke, KmlLineNum(resource_elt),
	KmlColNum(resource_elt),
	"<Resource> with no preassignment and no <Role>");
    role = NULL;
  }

  /* optional ResourceType */
  if( KmlContainsChild(resource_elt, "ResourceType", &resource_type_elt) )
  {
    if( !KmlCheck(resource_type_elt, "Reference", ke) )
      return false;
    ref = KmlAttributeValue(resource_type_elt, 0);
    if( !KheInstanceRetrieveResourceType(KheEventInstance(e), ref, &rt) )
      return KmlErrorMake(ke, KmlLineNum(resource_type_elt),
	KmlColNum(resource_type_elt),
	"in <ResourceType>, Reference \"%s\" unknown", ref);
    if( preassigned_resource != NULL &&
	KheResourceResourceType(preassigned_resource) != rt )
      return KmlErrorMake(ke, KmlLineNum(resource_type_elt),
	KmlColNum(resource_type_elt),
      "<ResourceType> \"%s\" inconsistent with preassigned resource \"%s\"",
	ref, KheResourceId(preassigned_resource));
  }
  else
  {
    if( preassigned_resource == NULL )
      return KmlErrorMake(ke, KmlLineNum(resource_elt),
	KmlColNum(resource_elt),
	"<Resource> with no preassignment and no <ResourceType>");
    rt = KheResourceResourceType(preassigned_resource);
  }

  /* optional workload */
  if( KmlContainsChild(resource_elt, "Workload", &workload_elt) )
    sscanf(KmlText(workload_elt), "%d", &workload);
  else
    workload = KheEventWorkload(e);

  /* make and add the event resource */
  if( !KheEventResourceMake(e, rt, preassigned_resource, role, workload, &er) )
    return KmlErrorMake(ke, KmlLineNum(resource_elt), KmlColNum(resource_elt),
      "<Event> Role \"%s\" used previously", role);
  return true;
}


/*****************************************************************************/
/*                                                                           */
/*  bool KheEventResourceWrite(KHE_EVENT_RESOURCE er, KML_FILE kf)           */
/*                                                                           */
/*  Write er onto kf.                                                        */
/*                                                                           */
/*****************************************************************************/

bool KheEventResourceWrite(KHE_EVENT_RESOURCE er, KML_FILE kf)
{
  if( er->role == NULL && er->workload == KheEventWorkload(er->event) )
  {
    if( er->preassigned_resource == NULL )
      return false;
    KmlEltAttribute(kf, "Resource", "Reference",
      KheResourceId(er->preassigned_resource));
  }
  else
  {
    KmlBegin(kf, "Resource");
    if( er->preassigned_resource != NULL )
      KmlAttribute(kf, "Reference", KheResourceId(er->preassigned_resource));
    if( er->role != NULL )
    {
      KmlEltPrintf(kf, "Role", "%s", er->role);
      KmlEltAttribute(kf, "ResourceType", "Reference",
	KheResourceTypeId(er->resource_type));
    }
    if( er->workload != KheEventWorkload(er->event) )
      KmlEltPrintf(kf, "Workload", "%d", er->workload);
    KmlEnd(kf, "Resource");
  }
  return true;
}


/*****************************************************************************/
/*                                                                           */
/*  Submodule "debug"                                                        */
/*                                                                           */
/*****************************************************************************/

/*****************************************************************************/
/*                                                                           */
/*  void KheEventResourceDebug(KHE_EVENT_RESOURCE er, int verbosity,         */
/*    int indent, FILE *fp)                                                  */
/*                                                                           */
/*  Debug print of er onto fp with the given verbosity and indent.           */
/*                                                                           */
/*****************************************************************************/

void KheEventResourceDebug(KHE_EVENT_RESOURCE er, int verbosity,
  int indent, FILE *fp)
{
  if( verbosity >= 1 )
  {
    if( indent >= 0 )
      fprintf(fp, "%*s", indent, "");
    KheEventDebug(er->event, 1, -1, fp);
      fprintf(fp, ".%d", er->index_in_event);
    if( indent >= 0 )
      fprintf(fp, "\n");
  }
}
