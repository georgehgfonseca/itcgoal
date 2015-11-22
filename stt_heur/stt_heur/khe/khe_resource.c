
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
/*  FILE:         khe_resource.c                                             */
/*  DESCRIPTION:  A resource                                                 */
/*                                                                           */
/*****************************************************************************/
#include "khe_interns.h"

#define DEBUG1 0
#define DEBUG2 0

/*****************************************************************************/
/*                                                                           */
/*  KHE_RESOURCE - a resource                                                */
/*                                                                           */
/*****************************************************************************/

struct khe_resource_rec {
  void				*back;			/* back pointer      */
  KHE_RESOURCE_TYPE		resource_type;		/* resource type     */
  char				*id;			/* Id                */
  char				*name;			/* Name              */
  KHE_RESOURCE_GROUP		partition;		/* partition         */
  ARRAY_KHE_RESOURCE_GROUP	user_resource_groups;	/* user resource gps */
  int				index_in_instance;	/* index in instance */
  int				index_in_type;		/* index in r. type  */
  ARRAY_KHE_EVENT_RESOURCE	preassigned_event_resources;
  ARRAY_KHE_EVENT		layer_events;		/* events in layer   */
  int				layer_duration;		/* duration of layer */
  ARRAY_KHE_CONSTRAINT		constraints;		/* constraints       */
  KHE_RESOURCE_GROUP		singleton_resource_group; /* singleton       */
  KHE_TIME_GROUP		hard_unavail;		/* when r is unavail */
  KHE_TIME_GROUP		hard_and_soft_unavail;	/* when r is unavail */
};


/*****************************************************************************/
/*                                                                           */
/*  Submodule "construction and query"                                       */
/*                                                                           */
/*****************************************************************************/

/*****************************************************************************/
/*                                                                           */
/*  bool KheResourceMake(KHE_RESOURCE_TYPE rt, char *id, char *name,         */
/*    KHE_RESOURCE_GROUP partition, KHE_RESOURCE *r)                         */
/*                                                                           */
/*  Make a new resource, add it to the `all' resource group of its resource  */
/*  type, and to its partition if any, and return it.                        */
/*                                                                           */
/*  This function returns false if any resource of rt's instance has the     */
/*  same id, unlike KheResourceGroupMake, which checks only for resource     */
/*  groups of the same resource type with the same id.                       */
/*                                                                           */
/*****************************************************************************/

bool KheResourceMake(KHE_RESOURCE_TYPE rt, char *id, char *name,
  KHE_RESOURCE_GROUP partition, KHE_RESOURCE *r)
{
  KHE_RESOURCE res;
  MAssert(rt != NULL, "KheResourceMake given NULL rt");
  MAssert(!KheInstanceComplete(KheResourceTypeInstance(rt)),
    "KheResourceMake called after KheInstanceMakeEnd");
  if( id != NULL &&
      KheInstanceRetrieveResource(KheResourceTypeInstance(rt), id, &res) )
  {
    *r = NULL;
    return false;
  }
  MMake(res);
  res->back = NULL;
  res->resource_type = rt;
  res->id = id;
  res->name = name;
  res->partition = partition;
  MArrayInit(res->user_resource_groups);
  res->index_in_instance =
    KheInstanceResourceCount(KheResourceTypeInstance(rt));
  KheInstanceAddResource(KheResourceTypeInstance(rt), res);
  res->index_in_type = KheResourceTypeResourceCount(rt);
  KheResourceTypeAddResource(rt, res);
  MArrayInit(res->preassigned_event_resources);
  MArrayInit(res->layer_events);
  res->layer_duration = 0;
  MArrayInit(res->constraints);
  if( partition != NULL )
  {
    MAssert(KheResourceGroupIsPartition(partition),
      "KheResourceMake: partition attribute is not a defined partition");
    KheResourceGroupAddResourceInternal(partition, res);
  }
  KheResourceGroupAddResourceInternal(KheResourceTypeFullResourceGroup(rt),res);
  res->singleton_resource_group = KheResourceGroupMakeInternal(
    KHE_RESOURCE_GROUP_TYPE_SINGLETON, rt, NULL, NULL);
  KheResourceGroupAddResourceInternal(res->singleton_resource_group, res);
  res->hard_unavail = NULL;  /* finalized at instance end */
  res->hard_and_soft_unavail = NULL;  /* finalized at instance end */
  *r = res;
  return true;
}


/*****************************************************************************/
/*                                                                           */
/*  void KheResourceSetBack(KHE_RESOURCE r, void *back)                      */
/*                                                                           */
/*  Set the back pointer of r.                                               */
/*                                                                           */
/*****************************************************************************/

void KheResourceSetBack(KHE_RESOURCE r, void *back)
{
  MAssert(!KheInstanceComplete(KheResourceTypeInstance(r->resource_type)),
    "KheResourceSetBack called after KheInstanceMakeEnd");
  r->back = back;
}


/*****************************************************************************/
/*                                                                           */
/*  void *KheResourceBack(KHE_RESOURCE r)                                    */
/*                                                                           */
/*  Return the back pointer of r.                                            */
/*                                                                           */
/*****************************************************************************/

void *KheResourceBack(KHE_RESOURCE r)
{
  return r->back;
}


/*****************************************************************************/
/*                                                                           */
/*  void KheResourceAddUserResourceGroup(KHE_RESOURCE r,                     */
/*    KHE_RESOURCE_GROUP rg)                                                 */
/*                                                                           */
/*  This internal function adds rg to r's list of user resource groups.      */
/*                                                                           */
/*****************************************************************************/

void KheResourceAddUserResourceGroup(KHE_RESOURCE r, KHE_RESOURCE_GROUP rg)
{
  MArrayAddLast(r->user_resource_groups, rg);
}


/*****************************************************************************/
/*                                                                           */
/*  KHE_RESOURCE_TYPE KheResourceResourceType(KHE_RESOURCE r)                */
/*                                                                           */
/*  Return the resource type of r, always non-NULL.                          */
/*                                                                           */
/*****************************************************************************/

KHE_RESOURCE_TYPE KheResourceResourceType(KHE_RESOURCE r)
{
  return r->resource_type;
}


/*****************************************************************************/
/*                                                                           */
/*  KHE_INSTANCE KheResourceInstance(KHE_RESOURCE r)                         */
/*                                                                           */
/*  Return the instance attribute of r.                                      */
/*                                                                           */
/*****************************************************************************/

KHE_INSTANCE KheResourceInstance(KHE_RESOURCE r)
{
  return KheResourceTypeInstance(r->resource_type);
}


/*****************************************************************************/
/*                                                                           */
/*  char *KheResourceId(KHE_RESOURCE r)                                      */
/*                                                                           */
/*  Return the id attribute of r.                                            */
/*                                                                           */
/*****************************************************************************/

char *KheResourceId(KHE_RESOURCE r)
{
  return r->id;
}


/*****************************************************************************/
/*                                                                           */
/*  char *KheResourceName(KHE_RESOURCE r)                                    */
/*                                                                           */
/*  Return the name attribute of r.                                          */
/*                                                                           */
/*****************************************************************************/

char *KheResourceName(KHE_RESOURCE r)
{
  return r->name;
}


/*****************************************************************************/
/*                                                                           */
/*  KHE_RESOURCE_GROUP KheResourcePartition(KHE_RESOURCE r)                  */
/*                                                                           */
/*  Return the partition containing r.  This will be NULL if r's resource    */
/*  type does not have partitions.                                           */
/*                                                                           */
/*****************************************************************************/

KHE_RESOURCE_GROUP KheResourcePartition(KHE_RESOURCE r)
{
  return r->partition;
}


/*****************************************************************************/
/*                                                                           */
/*  void KheResourceSetPartition(KHE_RESOURCE r, KHE_RESOURCE_GROUP rg)      */
/*                                                                           */
/*  Set r's partition attribute to rg.                                       */
/*                                                                           */
/*****************************************************************************/

void KheResourceSetPartition(KHE_RESOURCE r, KHE_RESOURCE_GROUP rg)
{
  MAssert(r->partition == NULL || r->partition == rg,
    "KheResourceSetPartition internal error");
  r->partition = rg;
}


/*****************************************************************************/
/*                                                                           */
/*  int KheResourceIndexInInstance(KHE_RESOURCE r)                           */
/*                                                                           */
/*  Return the index number of r in its instance.                            */
/*                                                                           */
/*****************************************************************************/

int KheResourceIndexInInstance(KHE_RESOURCE r)
{
  return r->index_in_instance;
}


/*****************************************************************************/
/*                                                                           */
/*  int KheResourceIndexInResourceType(KHE_RESOURCE r)                       */
/*                                                                           */
/*  Return the index number of r in its resource type.                       */
/*                                                                           */
/*****************************************************************************/

int KheResourceIndexInResourceType(KHE_RESOURCE r)
{
  return r->index_in_type;
}


/*****************************************************************************/
/*                                                                           */
/*  Submodule "similarity"                                                   */
/*                                                                           */
/*****************************************************************************/

/*****************************************************************************/
/*                                                                           */
/*  bool KheSimilar(int pos, int neg)                                        */
/*                                                                           */
/*  Return true if two resources with this positive and negative evidence    */
/*  are to be considered similar.                                            */
/*                                                                           */
/*****************************************************************************/

static bool KheSimilar(int pos, int neg)
{
  return pos >= neg + 2;
}


/*****************************************************************************/
/*                                                                           */
/*  bool KheResourceSimilar(KHE_RESOURCE r1, KHE_RESOURCE r2)                */
/*                                                                           */
/*  Return true if r1 and r2 are similar.                                    */
/*                                                                           */
/*****************************************************************************/

bool KheResourceSimilar(KHE_RESOURCE r1, KHE_RESOURCE r2)
{
  int pos, neg, i1, i2;  KHE_RESOURCE_GROUP rg1, rg2;
  KHE_EVENT_RESOURCE er;  KHE_EVENT e1, e2;  ARRAY_KHE_EVENT events1, events2;
  ARRAY_KHE_RESOURCE_GROUP resource_groups1, resource_groups2;
  if( DEBUG1 )
    fprintf(stderr, "  [ KheResourceSimilar(%s: %d, %s: %d)\n",
      r1->id==NULL ? "-" : r1->id, MArraySize(r1->preassigned_event_resources),
      r2->id==NULL ? "-" : r2->id, MArraySize(r2->preassigned_event_resources));

  /* find the admissible resource groups of the two resources */
  MArrayInit(resource_groups1);
  MArrayForEach(r1->user_resource_groups, &rg1, &i1)
    if( KheResourceGroupPartitionAdmissible(rg1) )
      MArrayAddLast(resource_groups1, rg1);
  MArrayInit(resource_groups2);
  MArrayForEach(r2->user_resource_groups, &rg2, &i2)
    if( KheResourceGroupPartitionAdmissible(rg2) )
      MArrayAddLast(resource_groups2, rg2);

  /* gather positive and negative evidence from resource groups */
  pos = neg = 0;
  MArrayForEach(resource_groups1, &rg1, &i1)
  {
    MArrayForEach(resource_groups2, &rg2, &i2)
      if( KheResourceGroupEqual(rg1, rg2) )
	break;
    if( i2 < MArraySize(resource_groups2) )
    {
      /* found a match, so delete both */
      pos += 2;
      MArrayRemove(resource_groups2, i2);
      MArrayRemove(resource_groups1, i1);
      i1--;
    }
  }
  neg += MArraySize(resource_groups1) + MArraySize(resource_groups2);

  /* find the admissible events of the two resources */
  MArrayInit(events1);
  MArrayForEach(r1->preassigned_event_resources, &er, &i1)
    if( KheEventPartitionAdmissible(KheEventResourceEvent(er)) )
      MArrayAddLast(events1, KheEventResourceEvent(er));
  MArrayInit(events2);
  MArrayForEach(r2->preassigned_event_resources, &er, &i2)
    if( KheEventPartitionAdmissible(KheEventResourceEvent(er)) )
      MArrayAddLast(events2, KheEventResourceEvent(er));

  /* gather positive and negative evidence from events */
  MArrayForEach(events1, &e1, &i1)
  {
    MArrayForEach(events2, &e2, &i2)
      if( KheEventPartitionSimilar(e1,e2,&resource_groups1,&resource_groups2) )
	break;
    if( i2 < MArraySize(events2) )
    {
      /* found a match, so delete the events concerned */
      pos += 2;
      MArrayRemove(events2, i2);
      MArrayRemove(events1, i1);
      i1--;
    }
  }
  neg += MArraySize(events1) + MArraySize(events2);

  /* return true if pos outweighs neg */
  MArrayFree(events1);
  MArrayFree(events2);
  MArrayFree(resource_groups1);
  MArrayFree(resource_groups2);
  if( DEBUG2 && KheSimilar(pos, neg) )
    fprintf(stderr, "  resources %s and %s similar (pos %d, neg %d)\n",
      r1->id==NULL ? "-" : r1->id, r2->id==NULL ? "-" : r2->id, pos, neg);
  if( DEBUG1 )
    fprintf(stderr, "  ] KheResourceSimilar returning %s (p%d, n%d)\n",
      KheSimilar(pos, neg) ? "true" : "false", pos, neg);
  return KheSimilar(pos, neg);
}


/*****************************************************************************/
/*                                                                           */
/*  Submodule "auto-generated resource groups"                               */
/*                                                                           */
/*****************************************************************************/

/*****************************************************************************/
/*                                                                           */
/*  KHE_RESOURCE_GROUP KheResourceSingletonResourceGroup(KHE_RESOURCE r)     */
/*                                                                           */
/*  Return the automatically generated singleton resource group holding r.   */
/*                                                                           */
/*****************************************************************************/

KHE_RESOURCE_GROUP KheResourceSingletonResourceGroup(KHE_RESOURCE r)
{
  return r->singleton_resource_group;
}


/*****************************************************************************/
/*                                                                           */
/*  Submodule "preassigned event resources and layers"                       */
/*                                                                           */
/*****************************************************************************/

/*****************************************************************************/
/*                                                                           */
/*  void KheResourceAddPreassignedEventResource(KHE_RESOURCE r,              */
/*    KHE_EVENT_RESOURCE er)                                                 */
/*                                                                           */
/*  Add er to r's list of event resources that r is preassigned to.          */
/*                                                                           */
/*****************************************************************************/

void KheResourceAddPreassignedEventResource(KHE_RESOURCE r,
  KHE_EVENT_RESOURCE er)
{
  MArrayAddLast(r->preassigned_event_resources, er);
}


/*****************************************************************************/
/*                                                                           */
/*  int KheResourcePreassignedEventResourceCount(KHE_RESOURCE r)             */
/*                                                                           */
/*  Return the number of ppreassigned event resources of r.                  */
/*                                                                           */
/*****************************************************************************/

int KheResourcePreassignedEventResourceCount(KHE_RESOURCE r)
{
  return MArraySize(r->preassigned_event_resources);
}


/*****************************************************************************/
/*                                                                           */
/*  KHE_EVENT_RESOURCE KheResourcePreassignedEventResource(KHE_RESOURCE r,   */
/*    int i)                                                                 */
/*                                                                           */
/*  Return the i'th preassigned event resource of r.                         */
/*                                                                           */
/*****************************************************************************/

KHE_EVENT_RESOURCE KheResourcePreassignedEventResource(KHE_RESOURCE r, int i)
{
  return MArrayGet(r->preassigned_event_resources, i);
}


/*****************************************************************************/
/*                                                                           */
/*  int KheResourceLayerEventCount(KHE_RESOURCE r)                           */
/*                                                                           */
/*  Return the number of events in r's layer.                                */
/*                                                                           */
/*****************************************************************************/

int KheResourceLayerEventCount(KHE_RESOURCE r)
{
  return MArraySize(r->layer_events);
}


/*****************************************************************************/
/*                                                                           */
/*  KHE_EVENT KheResourceLayerEvent(KHE_RESOURCE r, int i)                   */
/*                                                                           */
/*  Return the i'th event of r's layer.                                      */
/*                                                                           */
/*****************************************************************************/

KHE_EVENT KheResourceLayerEvent(KHE_RESOURCE r, int i)
{
  return MArrayGet(r->layer_events, i);
}


/*****************************************************************************/
/*                                                                           */
/*  int KheResourceLayerDuration(KHE_RESOURCE r)                             */
/*                                                                           */
/*  Return the duration of r's layer.                                        */
/*                                                                           */
/*****************************************************************************/

int KheResourceLayerDuration(KHE_RESOURCE r)
{
  return r->layer_duration;
}


/*****************************************************************************/
/*                                                                           */
/*  Submodule "constraints"                                                  */
/*                                                                           */
/*****************************************************************************/

/*****************************************************************************/
/*                                                                           */
/*  void KheResourceAddConstraint(KHE_RESOURCE r, KHE_CONSTRAINT c)          */
/*                                                                           */
/*  Add c to r.                                                              */
/*                                                                           */
/*****************************************************************************/

void KheResourceAddConstraint(KHE_RESOURCE r, KHE_CONSTRAINT c)
{
  KHE_EVENT_RESOURCE er;  KHE_EVENT e;  int i, pos;
  MArrayAddLast(r->constraints, c);

  /* build the layer of r, if required but not built yet */
  if( KheConstraintTag(c) == KHE_AVOID_CLASHES_CONSTRAINT_TAG &&
      KheConstraintRequired(c) && KheResourceLayerDuration(r) == 0 )
  {
    MArrayForEach(r->preassigned_event_resources, &er, &i)
    {
      e = KheEventResourceEvent(er);
      if( !MArrayContains(r->layer_events, e, &pos) )
      {
	MArrayAddLast(r->layer_events, e);
	r->layer_duration += KheEventDuration(e);
      }
    }
  }
}


/*****************************************************************************/
/*                                                                           */
/*  int KheResourceConstraintCount(KHE_RESOURCE r)                           */
/*                                                                           */
/*  Return the number of constraints applicable to r.                        */
/*                                                                           */
/*****************************************************************************/

int KheResourceConstraintCount(KHE_RESOURCE r)
{
  return MArraySize(r->constraints);
}


/*****************************************************************************/
/*                                                                           */
/*  KHE_CONSTRAINT KheResourceConstraint(KHE_RESOURCE r, int i)              */
/*                                                                           */
/*  Return the i'th constraint applicable to r.                              */
/*                                                                           */
/*****************************************************************************/

KHE_CONSTRAINT KheResourceConstraint(KHE_RESOURCE r, int i)
{
  return MArrayGet(r->constraints, i);
}


/*****************************************************************************/
/*                                                                           */
/*  Submodule "finalize"                                                     */
/*                                                                           */
/*****************************************************************************/

/*****************************************************************************/
/*                                                                           */
/*  KHE_TIME_GROUP KheResourceFindUnavail(KHE_RESOURCE r, bool hard_and_soft)*/
/*                                                                           */
/*  Make a single time group for the unavailable times of r, either just     */
/*  the hard ones (hard_and_soft == false) or hard and soft (.. == true).    */
/*                                                                           */
/*****************************************************************************/

static KHE_TIME_GROUP KheResourceFindUnavail(KHE_RESOURCE r, bool hard_and_soft)
{
  KHE_TIME_GROUP res, tg;  KHE_CONSTRAINT c;  int i, count;

  /* start out with the empty time group */
  res = KheTimeGroupMakeInternal(KHE_TIME_GROUP_TYPE_USER,
    KheResourceInstance(r), KHE_TIME_GROUP_KIND_ORDINARY, NULL,NULL,LSetNew());

  /* add in every hard (or hard or soft) prefer times domain */
  count = 0;
  tg = NULL;
  MArrayForEach(r->constraints, &c, &i)
    if( KheConstraintTag(c) == KHE_PREFER_TIMES_CONSTRAINT_TAG &&
	KheConstraintWeight(c) > 0 &&
	(hard_and_soft || KheConstraintRequired(c)) )
    {
      tg = KhePreferTimesConstraintDomain((KHE_PREFER_TIMES_CONSTRAINT) c);
      if( !KheTimeGroupSubset(tg, res) )
      {
	count++;
	KheTimeGroupUnionInternal(res, tg);
      }
    }

  /* sort out the final value, depending on count */
  if( count == 0 )
  {
    KheTimeGroupDelete(res);
    res = KheInstanceEmptyTimeGroup(KheResourceInstance(r));
  }
  else if( count == 1 )
  {
    KheTimeGroupDelete(res);
    MAssert(tg != NULL, "KheResourceFindUnavail internal error");
    res = tg;
  }
  else
  {
    KheTimeGroupFinalize(res, NULL, -1);
  }
  return res;
}


/*****************************************************************************/
/*                                                                           */
/*  void KheResourceFinalize(KHE_RESOURCE r)                                 */
/*                                                                           */
/*  Finalize r.                                                              */
/*                                                                           */
/*****************************************************************************/

void KheResourceFinalize(KHE_RESOURCE r)
{
  MAssert(r->hard_unavail == NULL, "KheResourceFinalize called twice");
  r->hard_unavail = KheResourceFindUnavail(r, false);
  r->hard_and_soft_unavail = KheResourceFindUnavail(r, true);
}


/*****************************************************************************/
/*                                                                           */
/*  KHE_TIME_GROUP KheResourceHardUnavailableTimeGroup(KHE_RESOURCE r)       */
/*                                                                           */
/*  Return a time group containing the union of the domains of the hard      */
/*  prefer times constraints of r.                                           */
/*                                                                           */
/*****************************************************************************/

KHE_TIME_GROUP KheResourceHardUnavailableTimeGroup(KHE_RESOURCE r)
{
  MAssert(r->hard_unavail != NULL,
    "KheResourceHardUnavailableTimeGroup called before KheInstanceEnd");
  return r->hard_unavail;
}


/*****************************************************************************/
/*                                                                           */
/*  KHE_TIME_GROUP KheResourceHardAndSoftUnavailableTimeGroup(KHE_RESOURCE r)*/
/*                                                                           */
/*  Return a time group containing the union of the domains of the hard      */
/*  and soft prefer times constraints of r.                                  */
/*                                                                           */
/*****************************************************************************/

KHE_TIME_GROUP KheResourceHardAndSoftUnavailableTimeGroup(KHE_RESOURCE r)
{
  MAssert(r->hard_and_soft_unavail != NULL,
    "KheResourceHardAndSoftUnavailableTimeGroup called before KheInstanceEnd");
  return r->hard_and_soft_unavail;
}


/*****************************************************************************/
/*                                                                           */
/*  Submodule "reading and writing"                                          */
/*                                                                           */
/*****************************************************************************/

/*****************************************************************************/
/*                                                                           */
/*  bool KheResourceMakeFromKml(KML_ELT resource_elt, KHE_INSTANCE ins,      */
/*    KML_ERROR *ke)                                                         */
/*                                                                           */
/*  Add a resource based on resource_elt to ins.                             */
/*                                                                           */
/*****************************************************************************/

bool KheResourceMakeFromKml(KML_ELT resource_elt, KHE_INSTANCE ins,
  KML_ERROR *ke)
{
  KML_ELT resource_groups_elt, resource_group_elt, rt_elt;
  int j;  char *name, *id, *ref;
  KHE_RESOURCE r;  KHE_RESOURCE_GROUP rg;  KHE_RESOURCE_TYPE rt;

  if( !KmlCheck(resource_elt, "Id : $Name ResourceType +ResourceGroups", ke) )
    return false;
  id = KmlExtractAttributeValue(resource_elt, 0);
  name = KmlExtractText(KmlChild(resource_elt, 0));
  rt_elt = KmlChild(resource_elt, 1);
  if( !KmlCheck(rt_elt, "Reference", ke) )
    return false;
  ref = KmlAttributeValue(rt_elt, 0);
  if( !KheInstanceRetrieveResourceType(ins, ref, &rt) )
    return KmlErrorMake(ke, KmlLineNum(rt_elt), KmlColNum(rt_elt),
      "in <ResourceType>, unknown Reference \"%s\"", ref);
  if( !KheResourceMake(rt, id, name, NULL, &r) )
    return KmlErrorMake(ke, KmlLineNum(resource_elt), KmlColNum(resource_elt),
      "in <Resource>, Id \"%s\" used previously", id);

  /* connect r to its subgroups, if any */
  if( KmlContainsChild(resource_elt, "ResourceGroups", &resource_groups_elt) )
  {
    if( !KmlCheck(resource_groups_elt, ": *ResourceGroup", ke) )
      return false;
    for( j = 0;  j < KmlChildCount(resource_groups_elt);  j++ )
    {
      resource_group_elt = KmlChild(resource_groups_elt, j);
      if( !KmlCheck(resource_group_elt, "Reference", ke) )
	return false;
      ref = KmlAttributeValue(resource_group_elt, 0);
      if( !KheInstanceRetrieveResourceGroup(ins, ref, &rg) )
	return KmlErrorMake(ke, KmlLineNum(resource_group_elt),
	  KmlColNum(resource_group_elt),
	  "in <ResourceGroup>, unknown Reference \"%s\"", ref);
      KheResourceGroupAddResource(rg, r);
    }
  }
  return true;
}


/*****************************************************************************/
/*                                                                           */
/*  bool KheResourceWrite(KHE_RESOURCE r, KML_FILE kf)                       */
/*                                                                           */
/*  Write r to kf.                                                           */
/*                                                                           */
/*****************************************************************************/

bool KheResourceWrite(KHE_RESOURCE r, KML_FILE kf)
{
  KHE_RESOURCE_GROUP rg;  int i;
  if( r->id == NULL || r->name == NULL )
    return false;
  KmlBegin(kf, "Resource");
  KmlAttribute(kf, "Id", r->id);
  KmlEltPrintf(kf, "Name", "%s", r->name);
  KmlEltAttribute(kf, "ResourceType", "Reference",
    KheResourceTypeId(r->resource_type));
  if( MArraySize(r->user_resource_groups) > 0 )
  {
    KmlBegin(kf, "ResourceGroups");
    MArrayForEach(r->user_resource_groups, &rg, &i)
      KmlEltAttribute(kf, "ResourceGroup", "Reference", KheResourceGroupId(rg));
    KmlEnd(kf, "ResourceGroups");
  }
  KmlEnd(kf, "Resource");
  return true;
}


/*****************************************************************************/
/*                                                                           */
/*  Submodule "debug"                                                        */
/*                                                                           */
/*****************************************************************************/

/*****************************************************************************/
/*                                                                           */
/*  void KheResourceDebug(KHE_RESOURCE r, int verbosity,                     */
/*    int indent, FILE *fp)                                                  */
/*                                                                           */
/*  Debug print of r onto fp with the given verbosity and indent.            */
/*                                                                           */
/*****************************************************************************/

void KheResourceDebug(KHE_RESOURCE r, int verbosity,
  int indent, FILE *fp)
{
  if( verbosity >= 1 )
  {
    if( indent >= 0 )
      fprintf(fp, "%*s", indent, "");
    fprintf(fp, "%s", r->id != NULL ? r->id : "-");
    if( indent >= 0 )
      fprintf(fp, "\n");
  }
}
