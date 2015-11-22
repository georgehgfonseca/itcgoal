
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
/*  FILE:         khe_resource_group.c                                       */
/*  DESCRIPTION:  A resource group                                           */
/*                                                                           */
/*****************************************************************************/
#include "khe_interns.h"
#define DEBUG1 0


/*****************************************************************************/
/*                                                                           */
/*  KHE_RESOURCE_GROUP - a resource group                                    */
/*                                                                           */
/*  Implementation note.  The resources_set attribute is always defined      */
/*  and it holds the true current value of the resource group as a set of    */
/*  resources, represented as the set of their indices in the enclosing      */
/*  instance.  The resource_indexes attribute is a redundant alternative     */
/*  representation of the same information, updated lazily:  when the        */
/*  resources change, it is cleared, and when a traversal is required, or    */
/*  the resource group needs to become immutable, it is made consistent      */
/*  with resources_set.                                                      */
/*                                                                           */
/*****************************************************************************/

struct khe_resource_group_rec {
  void				*back;			/* back pointer      */
  KHE_RESOURCE_GROUP_TYPE	resource_group_type;	/* user-defined etc  */
  /* bool			is_partition; */	/* true if partition */
  int				partition_index;	/* index in instance */
  KHE_RESOURCE_TYPE		resource_type;		/* resource type     */
  char				*id;			/* Id                */
  char				*name;			/* Name              */
  LSET				resources_set;		/* resources set     */
  ARRAY_SHORT			resource_indexes;	/* resources array   */
  KHE_RESOURCE_GROUP		partition;		/* partition, if any */
};


/*****************************************************************************/
/*                                                                           */
/*  Submodule "internal operations"                                          */
/*                                                                           */
/*****************************************************************************/

/*****************************************************************************/
/*                                                                           */
/*  KHE_RESOURCE_GROUP_TYPE KheResourceGroupType(KHE_RESOURCE_GROUP rg)      */
/*                                                                           */
/*  Return the resource group type of rg.                                    */
/*                                                                           */
/*****************************************************************************/

KHE_RESOURCE_GROUP_TYPE KheResourceGroupType(KHE_RESOURCE_GROUP rg)
{
  return rg->resource_group_type;
}


/*****************************************************************************/
/*                                                                           */
/*  KHE_RESOURCE_GROUP KheResourceGroupMakeInternal(                         */
/*    KHE_RESOURCE_GROUP_TYPE resource_group_type,                           */
/*    KHE_RESOURCE_TYPE rt, char *id, char *name)                            */
/*                                                                           */
/*  Make a resource group of the given type, but do not add it to rt.        */
/*                                                                           */
/*****************************************************************************/

KHE_RESOURCE_GROUP KheResourceGroupMakeInternal(
  KHE_RESOURCE_GROUP_TYPE resource_group_type,
  KHE_RESOURCE_TYPE rt, char *id, char *name)
{
  KHE_RESOURCE_GROUP res;
  MMake(res);
  res->back = NULL;
  res->resource_group_type = resource_group_type;
  /* res->is_partition = is_partition; */
  res->partition_index = -1;  /* means "not a partition" */
  res->resource_type = rt;
  res->id = id;
  res->name = name;
  res->resources_set = LSetNew();
  MArrayInit(res->resource_indexes);
  res->partition = NULL;
  return res;
}


/*****************************************************************************/
/*                                                                           */
/*  void KheResourceGroupAddResourceInternal(KHE_RESOURCE_GROUP rg,          */
/*    KHE_RESOURCE r)                                                        */
/*                                                                           */
/*  Add r to rg.                                                             */
/*                                                                           */
/*****************************************************************************/

void KheResourceGroupAddResourceInternal(KHE_RESOURCE_GROUP rg, KHE_RESOURCE r)
{
  LSetInsert(&rg->resources_set, KheResourceIndexInInstance(r));
  MArrayClear(rg->resource_indexes);
}


/*****************************************************************************/
/*                                                                           */
/*  void KheResourceGroupSubResourceInternal(KHE_RESOURCE_GROUP rg,          */
/*    KHE_RESOURCE r)                                                        */
/*                                                                           */
/*  Subtract r from rg.                                                      */
/*                                                                           */
/*****************************************************************************/

void KheResourceGroupSubResourceInternal(KHE_RESOURCE_GROUP rg, KHE_RESOURCE r)
{
  LSetDelete(rg->resources_set, KheResourceIndexInInstance(r));
  MArrayClear(rg->resource_indexes);
}


/*****************************************************************************/
/*                                                                           */
/*  void KheResourceGroupUnionInternal(KHE_RESOURCE_GROUP rg,                */
/*    KHE_RESOURCE_GROUP rg2)                                                */
/*                                                                           */
/*  Update rg's resources to be their union with rg2's resources.            */
/*                                                                           */
/*****************************************************************************/

void KheResourceGroupUnionInternal(KHE_RESOURCE_GROUP rg,
  KHE_RESOURCE_GROUP rg2)
{
  LSetUnion(&rg->resources_set, rg2->resources_set);
  MArrayClear(rg->resource_indexes);
}


/*****************************************************************************/
/*                                                                           */
/*  void KheResourceGroupIntersectInternal(KHE_RESOURCE_GROUP rg,            */
/*    KHE_RESOURCE_GROUP rg2)                                                */
/*                                                                           */
/*  Update rg's resources to be their intersection with rg2's resources.     */
/*                                                                           */
/*****************************************************************************/

void KheResourceGroupIntersectInternal(KHE_RESOURCE_GROUP rg,
  KHE_RESOURCE_GROUP rg2)
{
  LSetIntersection(rg->resources_set, rg2->resources_set);
  MArrayClear(rg->resource_indexes);
}


/*****************************************************************************/
/*                                                                           */
/*  void KheResourceGroupDifferenceInternal(KHE_RESOURCE_GROUP rg,           */
/*    KHE_RESOURCE_GROUP rg2)                                                */
/*                                                                           */
/*  Update rg's resources to be the set difference of them with rg2's        */
/*  resources.                                                               */
/*                                                                           */
/*****************************************************************************/

void KheResourceGroupDifferenceInternal(KHE_RESOURCE_GROUP rg,
  KHE_RESOURCE_GROUP rg2)
{
  LSetDifference(rg->resources_set, rg2->resources_set);
  MArrayClear(rg->resource_indexes);
}


/*****************************************************************************/
/*                                                                           */
/*  void KheResourceGroupSetResourcesArrayInternal(KHE_RESOURCE_GROUP rg)    */
/*                                                                           */
/*  Make rg's resources_array attribute consistent with its resources_set,   */
/*  if this has not already been done.                                       */
/*                                                                           */
/*  Also set rg->partition, although it may have to be done again, if        */
/*  partitions are inferred later on.                                        */
/*                                                                           */
/*****************************************************************************/

void KheResourceGroupSetResourcesArrayInternal(KHE_RESOURCE_GROUP rg)
{
  KHE_RESOURCE r;  int i;  short ix;
  if( DEBUG1 )
    fprintf(stderr, "[ KheResourceGroupSetResourcesArrayInternal(%s %p)\n",
      rg->id == NULL ? "-" : rg->id, (void *) rg);
  if( MArraySize(rg->resource_indexes) == 0 )
  {
    LSetExpand(rg->resources_set, &rg->resource_indexes);
    if( rg->resource_group_type == KHE_RESOURCE_GROUP_TYPE_USER )
      MArrayForEach(rg->resource_indexes, &ix, &i)
      {
	r = KheInstanceResource(KheResourceTypeInstance(rg->resource_type), ix);
	KheResourceAddUserResourceGroup(r, rg);
      }
  }
  KheResourceGroupSetPartition(rg);
  if( DEBUG1 )
    fprintf(stderr, "] KheResourceGroupSetResourcesArrayInternal\n");
}


/*****************************************************************************/
/*                                                                           */
/*  void KheResourceGroupDelete(KHE_RESOURCE_GROUP rg)                       */
/*                                                                           */
/*  Delete rg.                                                               */
/*                                                                           */
/*****************************************************************************/

void KheResourceGroupDelete(KHE_RESOURCE_GROUP rg)
{
  LSetFree(rg->resources_set);
  MArrayFree(rg->resource_indexes);
  MFree(rg);
}


/*****************************************************************************/
/*                                                                           */
/*  Submodule "domains and partitions"                                       */
/*                                                                           */
/*****************************************************************************/

/*****************************************************************************/
/*                                                                           */
/*  ARRAY_SHORT KheResourceGroupResourceIndexes(KHE_RESOURCE_GROUP rg)       */
/*                                                                           */
/*  Return a domain holding the resources of rg.                             */
/*                                                                           */
/*****************************************************************************/

ARRAY_SHORT KheResourceGroupResourceIndexes(KHE_RESOURCE_GROUP rg)
{
  return rg->resource_indexes;
}


/*****************************************************************************/
/*                                                                           */
/*  bool KheResourceGroupPartitionAdmissible(KHE_RESOURCE_GROUP rg)          */
/*                                                                           */
/*  Return true if this resource group is admissible when inferring          */
/*  resource partitions.                                                     */
/*                                                                           */
/*****************************************************************************/

bool KheResourceGroupPartitionAdmissible(KHE_RESOURCE_GROUP rg)
{
  return MArraySize(rg->resource_indexes) >= 2 &&
    3 * MArraySize(rg->resource_indexes) <=
      KheResourceTypeResourceCount(rg->resource_type);
}


/*****************************************************************************/
/*                                                                           */
/*  void KheResourceGroupSetIsPartition(KHE_RESOURCE_GROUP rg,               */
/*    bool is_partition)                                                     */
/*                                                                           */
/*  Set the is_partition attribute of rg.                                    */
/*                                                                           */
/*****************************************************************************/

/* ***
void KheResourceGroupSetIsPartition(KHE_RESOURCE_GROUP rg, bool is_partition)
{
  rg->is_partition = is_partition;
}
*** */


/*****************************************************************************/
/*                                                                           */
/*  void KheResourceGroupDeclarePartition(KHE_RESOURCE_GROUP rg)             */
/*                                                                           */
/*  Declare that rg is a partition.                                          */
/*                                                                           */
/*****************************************************************************/

void KheResourceGroupDeclarePartition(KHE_RESOURCE_GROUP rg)
{
  int i;  KHE_RESOURCE r;
  MAssert(rg->partition_index == -1,
    "KheResourceGroupDeclarePartition internal error (called twice on rg)");

  /* inform resource type */
  KheResourceTypeAddPartition(rg->resource_type, rg);

  /* inform instance and obtain index */
  KheInstanceAddPartition(KheResourceTypeInstance(rg->resource_type), rg,
    &rg->partition_index);

  /* inform rg's resources */
  for( i = 0;  i < KheResourceGroupResourceCount(rg);  i++ )
  {
    r = KheResourceGroupResource(rg, i);
    KheResourceSetPartition(r, rg);
  }
}


/*****************************************************************************/
/*                                                                           */
/*  void KheResourceGroupSetPartition(KHE_RESOURCE_GROUP rg)                 */
/*                                                                           */
/*  Set the partition attribute of rg as appropriate.                        */
/*                                                                           */
/*****************************************************************************/

void KheResourceGroupSetPartition(KHE_RESOURCE_GROUP rg)
{
  KHE_RESOURCE r;  short ix;
  rg->partition = NULL;
  if( MArraySize(rg->resource_indexes) > 0 )
  {
    ix = MArrayFirst(rg->resource_indexes);
    r = KheInstanceResource(KheResourceTypeInstance(rg->resource_type), ix);
    if( KheResourcePartition(r) != NULL &&
	KheResourceGroupSubset(rg, KheResourcePartition(r)) )
      rg->partition = KheResourcePartition(r);
  }
}


/*****************************************************************************/
/*                                                                           */
/*  KHE_RESOURCE_GROUP KheResourceGroupPartition(KHE_RESOURCE_GROUP rg)      */
/*                                                                           */
/*  Return rg's partition, or NULL if none.                                  */
/*                                                                           */
/*****************************************************************************/

KHE_RESOURCE_GROUP KheResourceGroupPartition(KHE_RESOURCE_GROUP rg)
{
  return rg->partition;
}


/*****************************************************************************/
/*                                                                           */
/*  Submodule "construction and query"                                       */
/*                                                                           */
/*****************************************************************************/

/*****************************************************************************/
/*                                                                           */
/*  bool KheResourceGroupMake(KHE_RESOURCE_TYPE rt, char *id, char *name,    */
/*    bool is_partition, KHE_RESOURCE_GROUP *rg)                             */
/*                                                                           */
/*  Make a user-defined resource group with these attributes.                */
/*                                                                           */
/*****************************************************************************/

bool KheResourceGroupMake(KHE_RESOURCE_TYPE rt, char *id, char *name,
  bool is_partition, KHE_RESOURCE_GROUP *rg)
{
  KHE_RESOURCE_GROUP res;
  MAssert(!KheInstanceComplete(KheResourceTypeInstance(rt)),
    "KheResourceGroupMake called after KheInstanceMakeEnd");
  MAssert(rt != NULL, "KheResourceGroupMake: rt parameter is NULL");
  MAssert(!is_partition || KheResourceTypeHasPartitions(rt),
    "KheResourceGroupMake is_partition when rt has no partitions");
  if( id != NULL && KheResourceTypeRetrieveResourceGroup(rt, id, rg) )
  {
    *rg = NULL;
    return false;
  }
  res = KheResourceGroupMakeInternal(KHE_RESOURCE_GROUP_TYPE_USER,
    /* is_partition, */ rt, id, name);
  KheResourceTypeAddResourceGroup(rt, res);
  if( is_partition )
    KheResourceGroupDeclarePartition(res);
  *rg = res;
  return true;
}


/*****************************************************************************/
/*                                                                           */
/*  void KheResourceGroupSetBack(KHE_RESOURCE_GROUP rg, void *back)          */
/*                                                                           */
/*  Set the back pointer of rg.                                              */
/*                                                                           */
/*****************************************************************************/

void KheResourceGroupSetBack(KHE_RESOURCE_GROUP rg, void *back)
{
  MAssert(rg->resource_group_type == KHE_RESOURCE_GROUP_TYPE_SOLN ||
      !KheInstanceComplete(KheResourceTypeInstance(rg->resource_type)),
    "KheResourceGroupSetBack called after KheInstanceMakeEnd");
  rg->back = back;
}


/*****************************************************************************/
/*                                                                           */
/*  void *KheResourceGroupBack(KHE_RESOURCE_GROUP rg)                        */
/*                                                                           */
/*  Return the back pointer of rg.                                           */
/*                                                                           */
/*****************************************************************************/

void *KheResourceGroupBack(KHE_RESOURCE_GROUP rg)
{
  return rg->back;
}


/*****************************************************************************/
/*                                                                           */
/*  KHE_RESOURCE_TYPE KheResourceGroupResourceType(KHE_RESOURCE_GROUP rg)    */
/*                                                                           */
/*  Return the resource_type attribute of rg.                                */
/*                                                                           */
/*****************************************************************************/

KHE_RESOURCE_TYPE KheResourceGroupResourceType(KHE_RESOURCE_GROUP rg)
{
  return rg->resource_type;
}


/*****************************************************************************/
/*                                                                           */
/*  KHE_INSTANCE KheResourceGroupInstance(KHE_RESOURCE_GROUP rg)             */
/*                                                                           */
/*  Return the instance attribute of rg.                                     */
/*                                                                           */
/*****************************************************************************/

KHE_INSTANCE KheResourceGroupInstance(KHE_RESOURCE_GROUP rg)
{
  return KheResourceTypeInstance(rg->resource_type);
}


/*****************************************************************************/
/*                                                                           */
/*  char *KheResourceGroupId(KHE_RESOURCE_GROUP rg)                          */
/*                                                                           */
/*  Return the id attribute of rg.                                           */
/*                                                                           */
/*****************************************************************************/

char *KheResourceGroupId(KHE_RESOURCE_GROUP rg)
{
  return rg->id;
}


/*****************************************************************************/
/*                                                                           */
/*  char *KheResourceGroupName(KHE_RESOURCE_GROUP rg)                        */
/*                                                                           */
/*  Return the name attribute of rg.                                         */
/*                                                                           */
/*****************************************************************************/

char *KheResourceGroupName(KHE_RESOURCE_GROUP rg)
{
  return rg->name;
}


/*****************************************************************************/
/*                                                                           */
/*  bool KheResourceGroupIsPartition(KHE_RESOURCE_GROUP rg)                  */
/*                                                                           */
/*  Return true if rg is a partition.                                        */
/*                                                                           */
/*****************************************************************************/

bool KheResourceGroupIsPartition(KHE_RESOURCE_GROUP rg)
{
  return rg->partition_index != -1;
  /* return rg->is_partition; */
}


/*****************************************************************************/
/*                                                                           */
/*  int KheResourceGroupPartitionIndex(KHE_RESOURCE_GROUP rg)                */
/*                                                                           */
/*  If rg is a partition, return its index in the instance.  If not,         */
/*  return -1.                                                               */
/*                                                                           */
/*****************************************************************************/

int KheResourceGroupPartitionIndex(KHE_RESOURCE_GROUP rg)
{
  return rg->partition_index;
}


/*****************************************************************************/
/*                                                                           */
/*  Submodule "resources"                                                    */
/*                                                                           */
/*****************************************************************************/

/*****************************************************************************/
/*                                                                           */
/*  void KheResourceGroupAddResource(KHE_RESOURCE_GROUP rg, KHE_RESOURCE r)  */
/*                                                                           */
/*  Add r to rg, first checking that rg is a user-defined resource group.    */
/*                                                                           */
/*****************************************************************************/

void KheResourceGroupAddResource(KHE_RESOURCE_GROUP rg, KHE_RESOURCE r)
{
  MAssert(!KheInstanceComplete(KheResourceGroupInstance(rg)),
    "KheResourceGroupAddResource called after KheInstanceMakeEnd");
  MAssert(rg->resource_group_type == KHE_RESOURCE_GROUP_TYPE_USER,
    "KheResourceGroupAddResource given unchangeable resource group");
  KheResourceGroupAddResourceInternal(rg, r);
}


/*****************************************************************************/
/*                                                                           */
/*  void KheResourceGroupSubResource(KHE_RESOURCE_GROUP rg, KHE_RESOURCE r)  */
/*                                                                           */
/*  Remove r from rg, first checking that rg is user-defined.                */
/*                                                                           */
/*****************************************************************************/

void KheResourceGroupSubResource(KHE_RESOURCE_GROUP rg, KHE_RESOURCE r)
{
  MAssert(!KheInstanceComplete(KheResourceGroupInstance(rg)),
    "KheResourceGroupSubResource called after KheInstanceMakeEnd");
  MAssert(rg->resource_group_type == KHE_RESOURCE_GROUP_TYPE_USER,
    "KheResourceGroupSubResource given unchangeable resource group");
  KheResourceGroupSubResourceInternal(rg, r);
}


/*****************************************************************************/
/*                                                                           */
/*  void KheResourceGroupUnion(KHE_RESOURCE_GROUP rg, KHE_RESOURCE_GROUP rg2)*/
/*                                                                           */
/*  Set rg's set of resources to its union with rg2's.                       */
/*                                                                           */
/*****************************************************************************/

void KheResourceGroupUnion(KHE_RESOURCE_GROUP rg, KHE_RESOURCE_GROUP rg2)
{
  MAssert(!KheInstanceComplete(KheResourceGroupInstance(rg)),
    "KheResourceGroupUnion called after KheInstanceMakeEnd");
  MAssert(rg->resource_group_type == KHE_RESOURCE_GROUP_TYPE_USER,
    "KheResourceGroupUnion given unchangeable resource group");
  KheResourceGroupUnionInternal(rg, rg2);
}


/*****************************************************************************/
/*                                                                           */
/*  void KheResourceGroupIntersect(KHE_RESOURCE_GROUP rg,                    */
/*    KHE_RESOURCE_GROUP rg2)                                                */
/*                                                                           */
/*  Set rg's set of resources to its intersection with rg2's.                */
/*                                                                           */
/*****************************************************************************/

void KheResourceGroupIntersect(KHE_RESOURCE_GROUP rg, KHE_RESOURCE_GROUP rg2)
{
  MAssert(!KheInstanceComplete(KheResourceGroupInstance(rg)),
    "KheResourceGroupIntersect called after KheInstanceMakeEnd");
  MAssert(rg->resource_group_type == KHE_RESOURCE_GROUP_TYPE_USER,
    "KheResourceGroupIntersect given unchangeable resource group");
  KheResourceGroupIntersectInternal(rg, rg2);
}


/*****************************************************************************/
/*                                                                           */
/*  void KheResourceGroupDifference(KHE_RESOURCE_GROUP rg,                   */
/*    KHE_RESOURCE_GROUP rg2)                                                */
/*                                                                           */
/*  Set rg's set of resources to its difference with rg2's.                  */
/*                                                                           */
/*****************************************************************************/

void KheResourceGroupDifference(KHE_RESOURCE_GROUP rg, KHE_RESOURCE_GROUP rg2)
{
  MAssert(!KheInstanceComplete(KheResourceGroupInstance(rg)),
    "KheResourceGroupDifference called after KheInstanceMakeEnd");
  MAssert(rg->resource_group_type == KHE_RESOURCE_GROUP_TYPE_USER,
    "KheResourceGroupDifference given unchangeable resource group");
  KheResourceGroupDifferenceInternal(rg, rg2);
}


/*****************************************************************************/
/*                                                                           */
/*  int KheResourceGroupResourceCount(KHE_RESOURCE_GROUP rg)                 */
/*                                                                           */
/*  Return the number of resources in rg.                                    */
/*                                                                           */
/*****************************************************************************/

int KheResourceGroupResourceCount(KHE_RESOURCE_GROUP rg)
{
  if( MArraySize(rg->resource_indexes) == 0 )
    KheResourceGroupSetResourcesArrayInternal(rg);
  return MArraySize(rg->resource_indexes);
}


/*****************************************************************************/
/*                                                                           */
/*  KHE_RESOURCE KheResourceGroupResource(KHE_RESOURCE_GROUP rg, int i)      */
/*                                                                           */
/*  Return the i'th resource of rg.                                          */
/*                                                                           */
/*****************************************************************************/

KHE_RESOURCE KheResourceGroupResource(KHE_RESOURCE_GROUP rg, int i)
{
  if( MArraySize(rg->resource_indexes) == 0 )
    KheResourceGroupSetResourcesArrayInternal(rg);
  return KheInstanceResource(KheResourceTypeInstance(rg->resource_type),
    MArrayGet(rg->resource_indexes, i));
}


/*****************************************************************************/
/*                                                                           */
/*  Submodule "set operations"                                               */
/*                                                                           */
/*****************************************************************************/

/*****************************************************************************/
/*                                                                           */
/*  bool KheResourceGroupContains(KHE_RESOURCE_GROUP rg, KHE_RESOURCE r)     */
/*                                                                           */
/*  Return true if rg contains r.                                            */
/*                                                                           */
/*****************************************************************************/

bool KheResourceGroupContains(KHE_RESOURCE_GROUP rg, KHE_RESOURCE r)
{
  return KheResourceResourceType(r) == rg->resource_type &&
    LSetContains(rg->resources_set, KheResourceIndexInInstance(r));
}


/*****************************************************************************/
/*                                                                           */
/* bool KheResourceGroupEqual(KHE_RESOURCE_GROUP rg1, KHE_RESOURCE_GROUP rg2)*/
/*                                                                           */
/*  Return true if these two resource groups are equal.                      */
/*                                                                           */
/*****************************************************************************/

bool KheResourceGroupEqual(KHE_RESOURCE_GROUP rg1, KHE_RESOURCE_GROUP rg2)
{
  return rg1 == rg2 || LSetEqual(rg1->resources_set, rg2->resources_set);
}


/*****************************************************************************/
/*                                                                           */
/*  bool KheResourceGroupSubset(KHE_RESOURCE_GROUP rg1,                      */
/*    KHE_RESOURCE_GROUP rg2)                                                */
/*                                                                           */
/*  Return true if rg1 is a subset of rg2.                                   */
/*                                                                           */
/*****************************************************************************/

bool KheResourceGroupSubset(KHE_RESOURCE_GROUP rg1, KHE_RESOURCE_GROUP rg2)
{
  return LSetSubset(rg1->resources_set, rg2->resources_set);
}


/*****************************************************************************/
/*                                                                           */
/*  bool KheResourceGroupDisjoint(KHE_RESOURCE_GROUP rg1,                    */
/*    KHE_RESOURCE_GROUP rg2)                                                */
/*                                                                           */
/*  Return true if rg1 and rg2 are disjoint.                                 */
/*                                                                           */
/*****************************************************************************/

bool KheResourceGroupDisjoint(KHE_RESOURCE_GROUP rg1, KHE_RESOURCE_GROUP rg2)
{
  return LSetDisjoint(rg1->resources_set, rg2->resources_set);
}


/*****************************************************************************/
/*                                                                           */
/*  Submodule "reading and writing"                                          */
/*                                                                           */
/*****************************************************************************/

/*****************************************************************************/
/*                                                                           */
/*  bool KheResourceGroupMakeFromKml(KML_ELT resource_group_elt,             */
/*    KHE_INSTANCE ins, KML_ERROR *ke)                                       */
/*                                                                           */
/*  Make a resource group and add it to ins.                                 */
/*                                                                           */
/*****************************************************************************/

bool KheResourceGroupMakeFromKml(KML_ELT resource_group_elt,
  KHE_INSTANCE ins, KML_ERROR *ke)
{
  char *id, *name, *rt_ref;  KML_ELT resource_type_elt;  KHE_RESOURCE_TYPE rt;
  KHE_RESOURCE_GROUP rg;
  if( !KmlCheck(resource_group_elt, "Id : $Name ResourceType", ke) )
      return false;
  id = KmlExtractAttributeValue(resource_group_elt, 0);
  name = KmlExtractText(KmlChild(resource_group_elt, 0));
  resource_type_elt = KmlChild(resource_group_elt, 1);
  if( !KmlCheck(resource_type_elt, "Reference", ke) )
    return false;
  rt_ref = KmlAttributeValue(resource_type_elt, 0);
  if( !KheInstanceRetrieveResourceType(ins, rt_ref, &rt) )
    return KmlErrorMake(ke, KmlLineNum(resource_type_elt),
      KmlColNum(resource_type_elt),
      "<ResourceType> Reference %s unknown", rt_ref);
  if( !KheResourceGroupMake(rt, id, name, false, &rg) )
    return KmlErrorMake(ke, KmlLineNum(resource_group_elt),
      KmlColNum(resource_group_elt),
      "<ResourceGroup> Id \"%s\" used previously", id);
  return true;
}


/*****************************************************************************/
/*                                                                           */
/*  bool KheResourceGroupWrite(KHE_RESOURCE_GROUP rg, KML_FILE kf)           */
/*                                                                           */
/*  Write rg (just its name) to kf.                                          */
/*                                                                           */
/*****************************************************************************/

bool KheResourceGroupWrite(KHE_RESOURCE_GROUP rg, KML_FILE kf)
{
  if( rg->id == NULL || rg->name == NULL )
    return false;
  KmlBegin(kf, "ResourceGroup");
  KmlAttribute(kf, "Id", rg->id);
  KmlEltPrintf(kf, "Name", "%s", rg->name);
  KmlEltAttribute(kf, "ResourceType", "Reference",
    KheResourceTypeId(rg->resource_type));
  KmlEnd(kf, "ResourceGroup");
  return true;
}


/*****************************************************************************/
/*                                                                           */
/*  Submodule "debug"                                                        */
/*                                                                           */
/*****************************************************************************/

/*****************************************************************************/
/*                                                                           */
/*  void KheResourceGroupDebug(KHE_RESOURCE_GROUP rg, int verbosity,         */
/*    int indent, FILE *fp)                                                  */
/*                                                                           */
/*  Debug print of rg onto fp with the given verbosity and indent.           */
/*                                                                           */
/*****************************************************************************/

void KheResourceGroupDebug(KHE_RESOURCE_GROUP rg, int verbosity,
  int indent, FILE *fp)
{
  KHE_RESOURCE r1, r2;  int i;  short ix;  KHE_INSTANCE ins;
  ins = KheResourceTypeInstance(rg->resource_type);
  if( verbosity == 1 )
  {
    if( indent >= 0 )
      fprintf(fp, "%*s", indent, "");
    if( KheResourceGroupResourceCount(rg) == 0 )
      fprintf(fp, "{}");
    else if( KheResourceGroupResourceCount(rg) == 1 )
    {
      r1 = KheResourceGroupResource(rg, 0);
      if( KheResourceId(r1) != NULL )
	fprintf(fp, "{%s}", KheResourceId(r1));
      else if( KheResourceGroupId(rg) != NULL )
	fprintf(fp, "%s", KheResourceGroupId(rg));
      else
	fprintf(fp, "{-}");
    }
    else if( KheResourceGroupId(rg) != NULL )
      fprintf(fp, "%s", KheResourceGroupId(rg));
    else
    {
      r1 = KheResourceGroupResource(rg, 0);
      r2 = KheResourceGroupResource(rg, KheResourceGroupResourceCount(rg) - 1);
      fprintf(fp, "{%s%s%s}",
	KheResourceId(r1) != NULL ? KheResourceId(r1) : "-",
	KheResourceGroupResourceCount(rg) == 2 ? "," : "..",
	KheResourceId(r2) != NULL ? KheResourceId(r2) : "-");
    }
    if( indent >= 0 )
      fprintf(fp, "\n");
  }
  else if( verbosity >= 2 )
  {
    if( indent >= 0 )
      fprintf(fp, "%*s", indent, "");
    if( KheResourceGroupId(rg) != NULL )
      fprintf(fp, "%s", KheResourceGroupId(rg));
    fprintf(fp, "{");
    MArrayForEach(rg->resource_indexes, &ix, &i)
    {
      r1 = KheInstanceResource(ins, ix);
      if( i > 0 )
	fprintf(fp, ", ");
      fprintf(fp, KheResourceId(r1) != NULL ? KheResourceId(r1) : "-");
    }
    fprintf(fp, "}");
    if( indent >= 0 )
      fprintf(fp, "\n");
  }
}
