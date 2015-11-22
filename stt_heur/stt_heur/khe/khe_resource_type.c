
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
/*  FILE:         khe_resource_type.c                                        */
/*  DESCRIPTION:  A type of resource                                         */
/*                                                                           */
/*****************************************************************************/
#include "khe_interns.h"

#define DEBUG1 0

/*****************************************************************************/
/*                                                                           */
/*  KHE_RESOURCE_TYPE - a type of resource                                   */
/*                                                                           */
/*****************************************************************************/

struct khe_resource_type_rec {
  void				*back;			/* back pointer      */
  KHE_INSTANCE			instance;		/* enclosing instance*/
  int				index;			/* index in instance */
  char				*id;			/* Id                */
  char				*name;			/* Name              */
  bool				has_partitions;		/* has partitions    */
  bool				demand_all_preassigned;	/* all preassigned   */
  int				avoid_split_assts_count; /* asacount         */
  ARRAY_KHE_RESOURCE_GROUP	resource_group_array;	/* resource groups   */
  TABLE_KHE_RESOURCE_GROUP	resource_group_table;	/* resource groups   */
  ARRAY_KHE_RESOURCE_GROUP	partition_array;	/* partitions        */
  KHE_RESOURCE_GROUP		full_resource_group;	/* full resource grp */
  KHE_RESOURCE_GROUP		empty_resource_group;	/* empty resource grp*/
  ARRAY_KHE_RESOURCE		resource_array;		/* resources         */
  TABLE_KHE_RESOURCE		resource_table;		/* resources         */
};


/*****************************************************************************/
/*                                                                           */
/*  Submodule "construction and query"                                       */
/*                                                                           */
/*****************************************************************************/

/*****************************************************************************/
/*                                                                           */
/*  bool KheResourceTypeMake(KHE_INSTANCE ins, char *id, char *name,         */
/*    bool has_partitions, KHE_RESOURCE_TYPE *rt)                            */
/*                                                                           */
/*  Create a new resource type with these attributes and add it to ins.      */
/*                                                                           */
/*****************************************************************************/

bool KheResourceTypeMake(KHE_INSTANCE ins, char *id, char *name,
  bool has_partitions, KHE_RESOURCE_TYPE *rt)
{
  KHE_RESOURCE_TYPE res;
  MAssert(!KheInstanceComplete(ins),
    "KheResourceTypeMake called after KheInstanceMakeEnd");
  if( id != NULL && KheInstanceRetrieveResourceType(ins, id, &res) )
  {
    *rt = NULL;
    return false;
  }
  MMake(res);
  res->back = NULL;
  res->instance = ins;
  res->id = id;  /* must precede KheInstanceAddResourceType! */
  KheInstanceAddResourceType(ins, res, &res->index);
  res->name = name;
  res->has_partitions = has_partitions;
  res->demand_all_preassigned = true;  /* until proved false */
  res->avoid_split_assts_count = 0;
  MArrayInit(res->resource_group_array);
  MArrayInit(res->partition_array);
  MTableInit(res->resource_group_table);
  res->full_resource_group = KheResourceGroupMakeInternal(
    KHE_RESOURCE_GROUP_TYPE_FULL, res, NULL, NULL);
  res->empty_resource_group = KheResourceGroupMakeInternal(
    KHE_RESOURCE_GROUP_TYPE_EMPTY, res, NULL, NULL);
  MArrayInit(res->resource_array);
  MTableInit(res->resource_table);
  *rt = res;
  return true;
}


/*****************************************************************************/
/*                                                                           */
/*  void KheResourceTypeSetBack(KHE_RESOURCE_TYPE rt, void *back)            */
/*                                                                           */
/*  Set the back pointer of rt.                                              */
/*                                                                           */
/*****************************************************************************/

void KheResourceTypeSetBack(KHE_RESOURCE_TYPE rt, void *back)
{
  MAssert(!KheInstanceComplete(rt->instance),
    "KheResourceTypeSetBack called after KheInstanceMakeEnd");
  rt->back = back;
}


/*****************************************************************************/
/*                                                                           */
/*  void *KheResourceTypeBack(KHE_RESOURCE_TYPE rt)                          */
/*                                                                           */
/*  Return the back pointer of rt.                                           */
/*                                                                           */
/*****************************************************************************/

void *KheResourceTypeBack(KHE_RESOURCE_TYPE rt)
{
  return rt->back;
}


/*****************************************************************************/
/*                                                                           */
/*  void KheResourceTypeFinalize(KHE_RESOURCE_TYPE rt)                       */
/*                                                                           */
/*  Finalize rt.                                                             */
/*                                                                           */
/*****************************************************************************/

void KheResourceTypeFinalize(KHE_RESOURCE_TYPE rt)
{
  int i;  KHE_RESOURCE_GROUP rg;  KHE_RESOURCE r;
  MArrayForEach(rt->resource_group_array, &rg, &i)
    KheResourceGroupSetResourcesArrayInternal(rg);
  KheResourceGroupSetResourcesArrayInternal(rt->full_resource_group);
  KheResourceGroupSetResourcesArrayInternal(rt->empty_resource_group);
  MArrayForEach(rt->resource_array, &r, &i)
    KheResourceGroupSetResourcesArrayInternal(
      KheResourceSingletonResourceGroup(r));
}


/*****************************************************************************/
/*                                                                           */
/*  KHE_INSTANCE KheResourceTypeInstance(KHE_RESOURCE_TYPE rt)               */
/*                                                                           */
/*  Return the instance attribute of rt.                                     */
/*                                                                           */
/*****************************************************************************/

KHE_INSTANCE KheResourceTypeInstance(KHE_RESOURCE_TYPE rt)
{
  return rt->instance;
}


/*****************************************************************************/
/*                                                                           */
/*  int KheResourceTypeIndex(KHE_RESOURCE_TYPE rt)                           */
/*                                                                           */
/*  Return the index of rt in the enclosing instance.                        */
/*                                                                           */
/*****************************************************************************/

int KheResourceTypeIndex(KHE_RESOURCE_TYPE rt)
{
  return rt->index;
}


/*****************************************************************************/
/*                                                                           */
/*  char *KheResourceTypeId(KHE_RESOURCE_TYPE rt)                            */
/*                                                                           */
/*  Return the id attribute of rt.                                           */
/*                                                                           */
/*****************************************************************************/

char *KheResourceTypeId(KHE_RESOURCE_TYPE rt)
{
  return rt->id;
}


/*****************************************************************************/
/*                                                                           */
/*  char *KheResourceTypeName(KHE_RESOURCE_TYPE rt)                          */
/*                                                                           */
/*  Return the name attribute of rt.                                         */
/*                                                                           */
/*****************************************************************************/

char *KheResourceTypeName(KHE_RESOURCE_TYPE rt)
{
  return rt->name;
}


/*****************************************************************************/
/*                                                                           */
/*  bool KheResourceTypeHasPartitions(KHE_RESOURCE_TYPE rt)                  */
/*                                                                           */
/*  Return the has_partitions attribute of rt.                               */
/*                                                                           */
/*****************************************************************************/

bool KheResourceTypeHasPartitions(KHE_RESOURCE_TYPE rt)
{
  return rt->has_partitions;
}


/*****************************************************************************/
/*                                                                           */
/*  Submodule "resource groups"                                              */
/*                                                                           */
/*****************************************************************************/

/*****************************************************************************/
/*                                                                           */
/*  void KheResourceTypeAddResourceGroup(KHE_RESOURCE_TYPE rt,               */
/*    KHE_RESOURCE_GROUP rg)                                                 */
/*                                                                           */
/*  Add rg to rt.                                                            */
/*                                                                           */
/*****************************************************************************/

void KheResourceTypeAddResourceGroup(KHE_RESOURCE_TYPE rt,
  KHE_RESOURCE_GROUP rg)
{
  MArrayAddLast(rt->resource_group_array, rg);
  /* *** done separately now
  if( KheResourceGroupIsPartition(rg) )
    MArrayAddLast(rt->partition_array, rg);
  *** */
  if( KheResourceGroupId(rg) != NULL )
    MTableInsert(rt->resource_group_table, KheResourceGroupId(rg), rg);
}


/*****************************************************************************/
/*                                                                           */
/*  int KheResourceTypeResourceGroupCount(KHE_RESOURCE_TYPE rt)              */
/*                                                                           */
/*  Return the number of resource groups in rt.                              */
/*                                                                           */
/*****************************************************************************/

int KheResourceTypeResourceGroupCount(KHE_RESOURCE_TYPE rt)
{
  return MArraySize(rt->resource_group_array);
}


/*****************************************************************************/
/*                                                                           */
/*  KHE_RESOURCE_GROUP KheResourceTypeResourceGroup(KHE_RESOURCE_TYPE rt,    */
/*    int i)                                                                 */
/*                                                                           */
/*  Return the i'th resource group of rt.                                    */
/*                                                                           */
/*****************************************************************************/

KHE_RESOURCE_GROUP KheResourceTypeResourceGroup(KHE_RESOURCE_TYPE rt,
  int i)
{
  return MArrayGet(rt->resource_group_array, i);
}


/*****************************************************************************/
/*                                                                           */
/*  bool KheResourceTypeRetrieveResourceGroup(KHE_RESOURCE_TYPE rt,          */
/*    char *id, KHE_RESOURCE_GROUP *rg)                                      */
/*                                                                           */
/*  Retrieve a resource group by id from rt.                                 */
/*                                                                           */
/*****************************************************************************/

bool KheResourceTypeRetrieveResourceGroup(KHE_RESOURCE_TYPE rt,
  char *id, KHE_RESOURCE_GROUP *rg)
{
  int pos;
  return MTableRetrieve(rt->resource_group_table, id, rg, &pos);
}


/*****************************************************************************/
/*                                                                           */
/*  Submodule "partitions"                                                   */
/*                                                                           */
/*****************************************************************************/

/*****************************************************************************/
/*                                                                           */
/*  void KheResourceTypeAddPartition(KHE_RESOURCE_TYPE rt,                   */
/*    KHE_RESOURCE_GROUP rg)                                                 */
/*                                                                           */
/*  Inform rt that rg is a partition.                                        */
/*                                                                           */
/*****************************************************************************/

void KheResourceTypeAddPartition(KHE_RESOURCE_TYPE rt, KHE_RESOURCE_GROUP rg)
{
  MArrayAddLast(rt->partition_array, rg);
}


/*****************************************************************************/
/*                                                                           */
/*  int KheResourceTypePartitionCount(KHE_RESOURCE_TYPE rt)                  */
/*                                                                           */
/*  Return the number of partitions in rt.                                   */
/*                                                                           */
/*****************************************************************************/

int KheResourceTypePartitionCount(KHE_RESOURCE_TYPE rt)
{
  return MArraySize(rt->partition_array);
}


/*****************************************************************************/
/*                                                                           */
/*  KHE_RESOURCE_GROUP KheResourceTypePartition(KHE_RESOURCE_TYPE rt, int i) */
/*                                                                           */
/*  Return the i'th partition of rt.                                         */
/*                                                                           */
/*****************************************************************************/

KHE_RESOURCE_GROUP KheResourceTypePartition(KHE_RESOURCE_TYPE rt, int i)
{
  return MArrayGet(rt->partition_array, i);
}


/*****************************************************************************/
/*                                                                           */
/*  KHE_RESOURCE_GROUP KheResourceTypeFullResourceGroup(KHE_RESOURCE_TYPE rt)*/
/*                                                                           */
/*  Return the "all" resource group of rt.                                   */
/*                                                                           */
/*****************************************************************************/

KHE_RESOURCE_GROUP KheResourceTypeFullResourceGroup(KHE_RESOURCE_TYPE rt)
{
  return rt->full_resource_group;
}


/*****************************************************************************/
/*                                                                           */
/* KHE_RESOURCE_GROUP KheResourceTypeEmptyResourceGroup(KHE_RESOURCE_TYPE rt)*/
/*                                                                           */
/*  Return the empty resource group of rt.                                   */
/*                                                                           */
/*****************************************************************************/

KHE_RESOURCE_GROUP KheResourceTypeEmptyResourceGroup(KHE_RESOURCE_TYPE rt)
{
  return rt->empty_resource_group;
}


/*****************************************************************************/
/*                                                                           */
/*  Submodule "resources"                                                    */
/*                                                                           */
/*****************************************************************************/

/*****************************************************************************/
/*                                                                           */
/*  void KheResourceTypeAddResource(KHE_RESOURCE_TYPE rt, KHE_RESOURCE r)    */
/*                                                                           */
/*  Add r to rt.                                                             */
/*                                                                           */
/*****************************************************************************/

void KheResourceTypeAddResource(KHE_RESOURCE_TYPE rt, KHE_RESOURCE r)
{
  MArrayAddLast(rt->resource_array, r);
  if( KheResourceId(r) != NULL )
    MTableInsert(rt->resource_table, KheResourceId(r), r);
}


/*****************************************************************************/
/*                                                                           */
/*  ARRAY_KHE_RESOURCE KheResourceTypeResourcesArray(KHE_RESOURCE_TYPE rt)   */
/*                                                                           */
/*  Return the resources array of rt.                                        */
/*                                                                           */
/*****************************************************************************/

/* ***
ARRAY_KHE_RESOURCE KheResourceTypeResourcesArray(KHE_RESOURCE_TYPE rt)
{
  return rt->resource_array;
}
*** */


/*****************************************************************************/
/*                                                                           */
/*  int KheResourceTypeResourceCount(KHE_RESOURCE_TYPE rt)                   */
/*                                                                           */
/*  Return the number of resources in rt.                                    */
/*                                                                           */
/*****************************************************************************/

int KheResourceTypeResourceCount(KHE_RESOURCE_TYPE rt)
{
  return MArraySize(rt->resource_array);
}


/*****************************************************************************/
/*                                                                           */
/*  KHE_RESOURCE KheResourceTypeResource(KHE_RESOURCE_TYPE rt, int i)        */
/*                                                                           */
/*  Return the i'th resource of rt.                                          */
/*                                                                           */
/*****************************************************************************/

KHE_RESOURCE KheResourceTypeResource(KHE_RESOURCE_TYPE rt, int i)
{
  return MArrayGet(rt->resource_array, i);
}


/*****************************************************************************/
/*                                                                           */
/*  bool KheResourceTypeRetrieveResource(KHE_RESOURCE_TYPE rt,               */
/*    char *id, KHE_RESOURCE *r)                                             */
/*                                                                           */
/*  Retrieve a resource with the given id from rt.                           */
/*                                                                           */
/*****************************************************************************/

bool KheResourceTypeRetrieveResource(KHE_RESOURCE_TYPE rt,
  char *id, KHE_RESOURCE *r)
{
  int pos;
  return MTableRetrieve(rt->resource_table, id, r, &pos);
}


/*****************************************************************************/
/*                                                                           */
/*  Submodule "inferring partitions"                                         */
/*                                                                           */
/*****************************************************************************/

/*****************************************************************************/
/*                                                                           */
/*  KHE_RESOURCE_CLUSTER - a cluster of resources                            */
/*                                                                           */
/*****************************************************************************/

typedef struct khe_resource_cluster_rec {
  ARRAY_KHE_RESOURCE		resources;	/* resources in cluster      */
} *KHE_RESOURCE_CLUSTER;

typedef MARRAY(KHE_RESOURCE_CLUSTER) ARRAY_KHE_RESOURCE_CLUSTER;


/*****************************************************************************/
/*                                                                           */
/*  KHE_RESOURCE_CLUSTER KheResourceClusterMake(KHE_RESOURCE r)              */
/*                                                                           */
/*  Make a new cluster containing just r.                                    */
/*                                                                           */
/*****************************************************************************/

static KHE_RESOURCE_CLUSTER KheResourceClusterMake(KHE_RESOURCE r)
{
  KHE_RESOURCE_CLUSTER res;
  MMake(res);
  MArrayInit(res->resources);
  MArrayAddLast(res->resources, r);
  return res;
}


/*****************************************************************************/
/*                                                                           */
/*  void KheResourceClusterFree(KHE_RESOURCE_CLUSTER rc)                     */
/*                                                                           */
/*  Free the memory consumed by rc.                                          */
/*                                                                           */
/*****************************************************************************/

static void KheResourceClusterFree(KHE_RESOURCE_CLUSTER rc)
{
  MArrayFree(rc->resources);
  MFree(rc);
}


/*****************************************************************************/
/*                                                                           */
/*  void KheResourceClusterMerge(KHE_RESOURCE_CLUSTER rc1,                   */
/*    KHE_RESOURCE_CLUSTER rc2, ARRAY_KHE_RESOURCE_CLUSTER *clusters)        */
/*                                                                           */
/*  Merge rc2 into rc1 and update *clusters appropriately.                   */
/*                                                                           */
/*****************************************************************************/

static void KheResourceClusterMerge(KHE_RESOURCE_CLUSTER rc1,
  KHE_RESOURCE_CLUSTER rc2, ARRAY_KHE_RESOURCE_CLUSTER *clusters)
{
  KHE_RESOURCE r;  int i;
  MArrayForEach(rc2->resources, &r, &i)
    MArrayPut(*clusters, KheResourceIndexInResourceType(r), rc1);
  MArrayAppend(rc1->resources, rc2->resources, i);
}


/*****************************************************************************/
/*                                                                           */
/*  void KheResourceClusterDebug(KHE_RESOURCE_CLUSTER rc, FILE *fp)          */
/*                                                                           */
/*  Debug print of rc onto fp.                                               */
/*                                                                           */
/*****************************************************************************/

static void KheResourceClusterDebug(KHE_RESOURCE_CLUSTER rc, FILE *fp)
{
  KHE_RESOURCE r;  int i;
  fprintf(fp, "{");
  MArrayForEach(rc->resources, &r, &i)
  {
    if( i > 0 )
      fprintf(fp, ", ");
    fprintf(fp, "%s", KheResourceId(r) == NULL ? "-" : KheResourceId(r));
  }
  fprintf(fp, "}");
}


/*****************************************************************************/
/*                                                                           */
/*  bool KheResourceGroupEqualsCluster(KHE_RESOURCE_GROUP rg,                */
/*    KHE_RESOURCE_CLUSTER rc)                                               */
/*                                                                           */
/*  Return true if the resources of rc equal the resources of rg.            */
/*                                                                           */
/*****************************************************************************/

bool KheResourceGroupEqualsCluster(KHE_RESOURCE_GROUP rg,
  KHE_RESOURCE_CLUSTER rc)
{
  KHE_RESOURCE r;  int i;
  if( MArraySize(rc->resources) != KheResourceGroupResourceCount(rg) )
    return false;
  MArrayForEach(rc->resources, &r, &i)
    if( !KheResourceGroupContains(rg, r) )
      return false;
  return true;
}


/*****************************************************************************/
/*                                                                           */
/*  bool KheResourceTypeClusterIsResourceGroup(KHE_RESOURCE_TYPE rt,         */
/*    KHE_RESOURCE_CLUSTER rc, KHE_RESOURCE_GROUP *rg)                       */
/*                                                                           */
/*  If rt contains a resource group whose elements are equal to the          */
/*  elements of rc, return true and set *rg to that resource group,          */
/*  else return false.                                                       */
/*                                                                           */
/*****************************************************************************/

static bool KheResourceTypeClusterIsResourceGroup(KHE_RESOURCE_TYPE rt,
  KHE_RESOURCE_CLUSTER rc, KHE_RESOURCE_GROUP *rg)
{
  int i;
  MArrayForEach(rt->resource_group_array, rg, &i)
    if( KheResourceGroupEqualsCluster(*rg, rc) )
      return true;
  return false;
}


/*****************************************************************************/
/*                                                                           */
/*  void KheResourceTypeInferPartitions(KHE_RESOURCE_TYPE rt)                */
/*                                                                           */
/*  Infer partitions for rt.                                                 */
/*                                                                           */
/*****************************************************************************/

void KheResourceTypeInferPartitions(KHE_RESOURCE_TYPE rt)
{
  KHE_RESOURCE r1, r2;  int i1, i2;  KHE_RESOURCE_CLUSTER rc1, rc2;
  ARRAY_KHE_RESOURCE_CLUSTER all_clusters, clusters_by_resource;
  KHE_RESOURCE_GROUP rg;
  if( DEBUG1 )
    fprintf(stderr, "[ KheResourceTypeInferPartitions(%s)\n",
      rt->id == NULL ? "-" : rt->id);

  /* initialize to one cluster per resource */
  MArrayInit(all_clusters);
  MArrayInit(clusters_by_resource);
  MArrayForEach(rt->resource_array, &r1, &i1)
  {
    rc1 = KheResourceClusterMake(r1);
    MArrayAddLast(all_clusters, rc1);
    MArrayAddLast(clusters_by_resource, rc1);
  }

  /* merge clusters that contain similar resources */
  for( i1 = 0;  i1 < MArraySize(rt->resource_array);  i1++ )
  {
    r1 = MArrayGet(rt->resource_array, i1);
    rc1 = MArrayGet(clusters_by_resource, i1);
    for( i2 = i1 + 1;  i2 < MArraySize(rt->resource_array);  i2++ )
    {
      r2 = MArrayGet(rt->resource_array, i2);
      rc2 = MArrayGet(clusters_by_resource, i2);
      if( rc1 != rc2 && KheResourceSimilar(r1, r2) )
        KheResourceClusterMerge(rc1, rc2, &clusters_by_resource);
    }
  }

  /* add partitions to rt, one partition for each cluster */
  rt->has_partitions = true;
  MArrayForEach(clusters_by_resource, &rc1, &i1)
    if( MArrayGet(rt->resource_array, i1) == MArrayFirst(rc1->resources) )
    {
      /* at this point we are visiting the clusters once each */
      if( DEBUG1 )
      {
	fprintf(stderr, "  adding partition ");
	KheResourceClusterDebug(rc1, stderr);
	fprintf(stderr, "\n");
      }

      /* find or make a resource group rg with rc1's resources */
      if( MArraySize(rc1->resources) == 1 )
	rg = KheResourceSingletonResourceGroup(MArrayFirst(rc1->resources));
      else if( MArraySize(rc1->resources) == MArraySize(rt->resource_array) )
	rg = KheResourceTypeFullResourceGroup(rt);
      else if( !KheResourceTypeClusterIsResourceGroup(rt, rc1, &rg) )
      {
	/* make a new resource group with these resources */
	rg = KheResourceGroupMakeInternal(KHE_RESOURCE_GROUP_TYPE_PARTITION,
	  rt, NULL, NULL);
	MArrayForEach(rc1->resources, &r2, &i2)
	  KheResourceGroupAddResourceInternal(rg, r2);
	KheResourceGroupSetResourcesArrayInternal(rg);
      }

      /* declare rg to be a partition */
      KheResourceGroupDeclarePartition(rg);

      /* ***
      KheResourceGroupSetIsPartition(rg, true);
      MArrayAddLast(rt->partition_array, rg);
      MArrayForEach(rc1->resources, &r2, &i2)
	KheResourceSetPartition(r2, rg);
      *** */
    }

  /* recalculate the partitions of resource groups */
  MArrayForEach(rt->resource_group_array, &rg, &i1)
    KheResourceGroupSetPartition(rg);

  /* free the clusters and arrays of clusters */
  MArrayForEach(all_clusters, &rc1, &i1)
    KheResourceClusterFree(rc1);
  MArrayFree(all_clusters);
  MArrayFree(clusters_by_resource);
  if( DEBUG1 )
    fprintf(stderr, "] KheResourceTypeInferPartitions returning\n");
}


/*****************************************************************************/
/*                                                                           */
/*  Submodule "reading and writing"                                          */
/*                                                                           */
/*****************************************************************************/

/*****************************************************************************/
/*                                                                           */
/*  bool KheResourceTypeMakeFromKml(KML_ELT resource_type_elt,               */
/*    KHE_INSTANCE ins, KML_ERROR *ke)                                       */
/*                                                                           */
/*  Add a resource type based on resource_type_elt to ins.                   */
/*                                                                           */
/*****************************************************************************/

bool KheResourceTypeMakeFromKml(KML_ELT resource_type_elt,
  KHE_INSTANCE ins, KML_ERROR *ke)
{
  char *id, *name;  KHE_RESOURCE_TYPE rt;
  if( !KmlCheck(resource_type_elt, "Id : $Name", ke) )
    return false;
  id = KmlExtractAttributeValue(resource_type_elt, 0);
  name = KmlExtractText(KmlChild(resource_type_elt, 0));
  if( !KheResourceTypeMake(ins, id, name, false, &rt) )
    return KmlErrorMake(ke, KmlLineNum(resource_type_elt),
      KmlColNum(resource_type_elt),
      "<ResourceType> Id \"%s\" used previously", id);
  return true;
}


/*****************************************************************************/
/*                                                                           */
/*  bool KheResourceTypeWrite(KHE_RESOURCE_TYPE rt, KML_FILE kf)             */
/*                                                                           */
/*  Write rt (just its name, not its members) to kf.                         */
/*                                                                           */
/*****************************************************************************/

bool KheResourceTypeWrite(KHE_RESOURCE_TYPE rt, KML_FILE kf)
{
  if( rt->id == NULL || rt->name == NULL )
    return false;
  KmlEltAttributeEltPrintf(kf, "ResourceType", "Id", rt->id,
    "Name", "%s", rt->name);
  /* ***
  KmlBegin(kf, "ResourceType");
  KmlAttribute(kf, "Id", rt->id);
  KmlEltPrintf(kf, "Name", "%s", rt->name);
  KmlEnd(kf, "ResourceType");
  *** */
  return true;
}


/*****************************************************************************/
/*                                                                           */
/*  bool KheResourceTypeWriteResourceGroups(KHE_RESOURCE_TYPE rt,            */
/*    KML_FILE kf)                                                           */
/*                                                                           */
/*  Write the resource groups (just their names) of rt to kf.                */
/*                                                                           */
/*****************************************************************************/

bool KheResourceTypeWriteResourceGroups(KHE_RESOURCE_TYPE rt, KML_FILE kf)
{
  KHE_RESOURCE_GROUP rg;  int i;
  MArrayForEach(rt->resource_group_array, &rg, &i)
    if( !KheResourceGroupWrite(rg, kf) )
      return false;
  return true;
}


/*****************************************************************************/
/*                                                                           */
/*  bool KheResourceTypeWriteResources(KHE_RESOURCE_TYPE rt, KML_FILE kf)    */
/*                                                                           */
/*  Write the resources of rt to kf.                                        */
/*                                                                           */
/*****************************************************************************/

bool KheResourceTypeWriteResources(KHE_RESOURCE_TYPE rt, KML_FILE kf)
{
  KHE_RESOURCE r;  int i;
  MArrayForEach(rt->resource_array, &r, &i)
    if( !KheResourceWrite(r, kf) )
      return false;
  return true;
}


/*****************************************************************************/
/*                                                                           */
/*  Submodule "demand and avoid split assignments count"                     */
/*                                                                           */
/*****************************************************************************/

/*****************************************************************************/
/*                                                                           */
/*  void KheResourceTypeDemandNotAllPreassigned(KHE_RESOURCE_TYPE rt)        */
/*                                                                           */
/*  Called to inform rt that demands for its resources are not all           */
/*  preassigned, as it initially assumes.                                    */
/*                                                                           */
/*****************************************************************************/

void KheResourceTypeDemandNotAllPreassigned(KHE_RESOURCE_TYPE rt)
{
  rt->demand_all_preassigned = false;
}


/*****************************************************************************/
/*                                                                           */
/*  bool KheResourceTypeDemandIsAllPreassigned(KHE_RESOURCE_TYPE rt)         */
/*                                                                           */
/*  Return true if all resource demands which could be satisfied by          */
/*  resources of type rt are preassignments.                                 */
/*                                                                           */
/*****************************************************************************/

bool KheResourceTypeDemandIsAllPreassigned(KHE_RESOURCE_TYPE rt)
{
  MAssert(KheInstanceComplete(rt->instance),
    "KheResourceTypeDemandIsAllPreassigned called before KheInstanceMakeEnd");
  return rt->demand_all_preassigned;
}


/*****************************************************************************/
/*                                                                           */
/*  void KheResourceTypeIncrementAvoidSplitAssignmentsCount(                 */
/*    KHE_RESOURCE_TYPE rt)                                                  */
/*                                                                           */
/*  Increment the record of the number of avoid split assignments            */
/*  constraints of type rt.                                                  */
/*                                                                           */
/*****************************************************************************/

void KheResourceTypeIncrementAvoidSplitAssignmentsCount(KHE_RESOURCE_TYPE rt)
{
  rt->avoid_split_assts_count++;
}


/*****************************************************************************/
/*                                                                           */
/*  int KheResourceTypeAvoidSplitAssignmentsCount(KHE_RESOURCE_TYPE rt)      */
/*                                                                           */
/*  Return the number of points of application of avoid split assignments    */
/*  constraints that apply to event resources of type rt.                    */
/*                                                                           */
/*****************************************************************************/

int KheResourceTypeAvoidSplitAssignmentsCount(KHE_RESOURCE_TYPE rt)
{
  return rt->avoid_split_assts_count;
}
