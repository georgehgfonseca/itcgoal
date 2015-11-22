
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
/*  FILE:         khe_avoid_unavailable_times_constraint.c                   */
/*  DESCRIPTION:  An avoid unavailable times constraint                      */
/*                                                                           */
/*****************************************************************************/
#include "khe_interns.h"


/*****************************************************************************/
/*                                                                           */
/*  KHE_AVOID_UNAVAILABLE_TIMES_CONSTRAINT - an avoid unavail. times constr. */
/*                                                                           */
/*****************************************************************************/

struct khe_avoid_unavailable_times_constraint_rec {
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
  ARRAY_KHE_RESOURCE_GROUP	resource_groups;	/* applies to        */
  ARRAY_KHE_RESOURCE		resources;		/* applies to        */
  ARRAY_KHE_TIME_GROUP		time_groups;		/* the times         */
  ARRAY_KHE_TIME		times;			/* the times         */
  KHE_TIME_GROUP		domain;			/* the times, united */
};


/*****************************************************************************/
/*                                                                           */
/*  Submodule "construction and query"                                       */
/*                                                                           */
/*****************************************************************************/

/*****************************************************************************/
/*                                                                           */
/*  bool KheAvoidUnavailableTimesConstraintMake(KHE_INSTANCE ins, char *id,  */
/*    char *name, bool required, int weight, KHE_COST_FUNCTION cf,           */
/*    KHE_AVOID_UNAVAILABLE_TIMES_CONSTRAINT *c)                             */
/*                                                                           */
/*  Make an avoid unavailable times constraint with these attributes, add    */
/*  it to ins, and return it.                                                */
/*                                                                           */
/*****************************************************************************/

bool KheAvoidUnavailableTimesConstraintMake(KHE_INSTANCE ins, char *id,
  char *name, bool required, int weight, KHE_COST_FUNCTION cf,
  KHE_AVOID_UNAVAILABLE_TIMES_CONSTRAINT *c)
{
  KHE_AVOID_UNAVAILABLE_TIMES_CONSTRAINT res;  KHE_CONSTRAINT cc;
  MAssert(!KheInstanceComplete(ins),
    "KheAvoidUnavailableTimesConstraintMake called after KheInstanceMakeEnd");
  if( id != NULL && KheInstanceRetrieveConstraint(ins, id, &cc) )
  {
    *c = NULL;
    return false;
  }
  MMake(res);
  res->back = NULL;
  res->tag = KHE_AVOID_UNAVAILABLE_TIMES_CONSTRAINT_TAG;
  res->instance = ins;
  res->id = id;
  res->name = name;
  res->required = required;
  res->weight = weight;
  res->combined_weight = required ? KheCost(weight, 0) : KheCost(0, weight);
  res->cost_function = cf;
  res->index = KheInstanceConstraintCount(ins);
  MArrayInit(res->resource_groups);
  MArrayInit(res->resources);
  MArrayInit(res->time_groups);
  MArrayInit(res->times);
  res->domain = NULL;  /* set when finalizing */
  KheInstanceAddConstraint(ins, (KHE_CONSTRAINT) res);
  *c = res;
  return true;
}


/*****************************************************************************/
/*                                                                           */
/*  int KheAvoidUnavailableTimesConstraintAppliesToCount(                    */
/*    KHE_AVOID_UNAVAILABLE_TIMES_CONSTRAINT c)                              */
/*                                                                           */
/*  Return the number of points of application of c.                         */
/*                                                                           */
/*****************************************************************************/

int KheAvoidUnavailableTimesConstraintAppliesToCount(
  KHE_AVOID_UNAVAILABLE_TIMES_CONSTRAINT c)
{
  int i, res;  KHE_RESOURCE_GROUP rg;
  res = MArraySize(c->resources);
  MArrayForEach(c->resource_groups, &rg, &i)
    res += KheResourceGroupResourceCount(rg);
  return res;
}


/*****************************************************************************/
/*                                                                           */
/*  void KheAvoidUnavailableTimesConstraintFinalize(                         */
/*    KHE_AVOID_UNAVAILABLE_TIMES_CONSTRAINT c)                              */
/*                                                                           */
/*  Finalize c, since KheInstanceMakeEnd has been called.                    */
/*                                                                           */
/*****************************************************************************/

void KheAvoidUnavailableTimesConstraintFinalize(
  KHE_AVOID_UNAVAILABLE_TIMES_CONSTRAINT c)
{
  int i;  KHE_TIME_GROUP tg;  KHE_TIME t;
  if( MArraySize(c->time_groups) == 0 && MArraySize(c->times) == 0 )
    c->domain = KheInstanceEmptyTimeGroup(c->instance);
  else if( MArraySize(c->time_groups) == 0 && MArraySize(c->times) == 1 )
    c->domain = KheTimeSingletonTimeGroup(MArrayFirst(c->times));
  else if( MArraySize(c->time_groups) == 1 && MArraySize(c->times) == 0 )
    c->domain = MArrayFirst(c->time_groups);
  else
  {
    c->domain = KheTimeGroupMakeInternal(KHE_TIME_GROUP_TYPE_USER,
      c->instance, KHE_TIME_GROUP_KIND_ORDINARY, NULL, NULL, LSetNew());
    MArrayForEach(c->time_groups, &tg, &i)
      KheTimeGroupUnionInternal(c->domain, tg);
    MArrayForEach(c->times, &t, &i)
      KheTimeGroupAddTimeInternal(c->domain, t);
    KheTimeGroupFinalize(c->domain, NULL, -1);
  }
}


/*****************************************************************************/
/*                                                                           */
/*  Submodule "resource groups"                                              */
/*                                                                           */
/*****************************************************************************/

/*****************************************************************************/
/*                                                                           */
/*  void KheAvoidUnavailableTimesConstraintAddResourceGroup(                 */
/*    KHE_AVOID_UNAVAILABLE_TIMES_CONSTRAINT c, KHE_RESOURCE_GROUP rg)       */
/*                                                                           */
/*  Add rg to c.                                                             */
/*                                                                           */
/*****************************************************************************/

void KheAvoidUnavailableTimesConstraintAddResourceGroup(
  KHE_AVOID_UNAVAILABLE_TIMES_CONSTRAINT c, KHE_RESOURCE_GROUP rg)
{
  int i;  KHE_RESOURCE r;
  MArrayAddLast(c->resource_groups, rg);
  for( i = 0;  i < KheResourceGroupResourceCount(rg);  i++ )
  {
    r = KheResourceGroupResource(rg, i);
    KheResourceAddConstraint(r, (KHE_CONSTRAINT) c);
  }
}


/*****************************************************************************/
/*                                                                           */
/*  int KheAvoidUnavailableTimesConstraintResourceGroupCount(                */
/*    KHE_AVOID_UNAVAILABLE_TIMES_CONSTRAINT c)                              */
/*                                                                           */
/*  Return the number of resource groups in c.                               */
/*                                                                           */
/*****************************************************************************/

int KheAvoidUnavailableTimesConstraintResourceGroupCount(KHE_AVOID_UNAVAILABLE_TIMES_CONSTRAINT c)
{
  return MArraySize(c->resource_groups);
}


/*****************************************************************************/
/*                                                                           */
/*  KHE_RESOURCE_GROUP KheAvoidUnavailableTimesConstraintResourceGroup(      */
/*    KHE_AVOID_UNAVAILABLE_TIMES_CONSTRAINT c, int i)                       */
/*                                                                           */
/*  Return the i'th resource group of c.                                     */
/*                                                                           */
/*                                                                           */
/*****************************************************************************/

KHE_RESOURCE_GROUP KheAvoidUnavailableTimesConstraintResourceGroup(
  KHE_AVOID_UNAVAILABLE_TIMES_CONSTRAINT c, int i)
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
/*  void KheAvoidUnavailableTimesConstraintAddResource(                      */
/*    KHE_AVOID_UNAVAILABLE_TIMES_CONSTRAINT c, KHE_RESOURCE r)              */
/*                                                                           */
/*  Add r to c.                                                              */
/*                                                                           */
/*****************************************************************************/

void KheAvoidUnavailableTimesConstraintAddResource(
  KHE_AVOID_UNAVAILABLE_TIMES_CONSTRAINT c, KHE_RESOURCE r)
{
  MArrayAddLast(c->resources, r);
  KheResourceAddConstraint(r, (KHE_CONSTRAINT) c);
}


/*****************************************************************************/
/*                                                                           */
/*  int KheAvoidUnavailableTimesConstraintResourceCount(                     */
/*    KHE_AVOID_UNAVAILABLE_TIMES_CONSTRAINT c)                              */
/*                                                                           */
/*  Return the number of resources of c.                                     */
/*                                                                           */
/*****************************************************************************/

int KheAvoidUnavailableTimesConstraintResourceCount(
  KHE_AVOID_UNAVAILABLE_TIMES_CONSTRAINT c)
{
  return MArraySize(c->resources);
}


/*****************************************************************************/
/*                                                                           */
/*  KHE_RESOURCE KheAvoidUnavailableTimesConstraintResource(                 */
/*    KHE_AVOID_UNAVAILABLE_TIMES_CONSTRAINT c, int i)                       */
/*                                                                           */
/*  Return the i'th resource of c.                                           */
/*                                                                           */
/*****************************************************************************/

KHE_RESOURCE KheAvoidUnavailableTimesConstraintResource(
  KHE_AVOID_UNAVAILABLE_TIMES_CONSTRAINT c, int i)
{
  return MArrayGet(c->resources, i);
}


/*****************************************************************************/
/*                                                                           */
/*  Submodule "time groups"                                                  */
/*                                                                           */
/*****************************************************************************/

/*****************************************************************************/
/*                                                                           */
/*  void KheAvoidUnavailableTimesConstraintAddTimeGroup(                     */
/*    KHE_AVOID_UNAVAILABLE_TIMES_CONSTRAINT c, KHE_TIME_GROUP tg)           */
/*                                                                           */
/*  Add tg to c.                                                             */
/*                                                                           */
/*****************************************************************************/

void KheAvoidUnavailableTimesConstraintAddTimeGroup(
  KHE_AVOID_UNAVAILABLE_TIMES_CONSTRAINT c, KHE_TIME_GROUP tg)
{
  MArrayAddLast(c->time_groups, tg);
}


/*****************************************************************************/
/*                                                                           */
/*  int KheAvoidUnavailableTimesConstraintTimeGroupCount(                    */
/*    KHE_AVOID_UNAVAILABLE_TIMES_CONSTRAINT c)                              */
/*                                                                           */
/*  Return the number of time groups in c.                                   */
/*                                                                           */
/*****************************************************************************/

int KheAvoidUnavailableTimesConstraintTimeGroupCount(
  KHE_AVOID_UNAVAILABLE_TIMES_CONSTRAINT c)
{
  return MArraySize(c->time_groups);
}


/*****************************************************************************/
/*                                                                           */
/*  KHE_TIME_GROUP KheAvoidUnavailableTimesConstraintTimeGroup(              */
/*    KHE_AVOID_UNAVAILABLE_TIMES_CONSTRAINT c, int i)                       */
/*                                                                           */
/*  Return the i'th time group of c.                                         */
/*                                                                           */
/*****************************************************************************/

KHE_TIME_GROUP KheAvoidUnavailableTimesConstraintTimeGroup(
  KHE_AVOID_UNAVAILABLE_TIMES_CONSTRAINT c, int i)
{
  return MArrayGet(c->time_groups, i);
}


/*****************************************************************************/
/*                                                                           */
/*  Submodule "times"                                                        */
/*                                                                           */
/*****************************************************************************/

/*****************************************************************************/
/*                                                                           */
/*  void KheAvoidUnavailableTimesConstraintAddTime(                          */
/*    KHE_AVOID_UNAVAILABLE_TIMES_CONSTRAINT c, KHE_TIME t)                  */
/*                                                                           */
/*  Add t to c.                                                              */
/*                                                                           */
/*****************************************************************************/

void KheAvoidUnavailableTimesConstraintAddTime(
  KHE_AVOID_UNAVAILABLE_TIMES_CONSTRAINT c, KHE_TIME t)
{
  MArrayAddLast(c->times, t);
}


/*****************************************************************************/
/*                                                                           */
/*  int KheAvoidUnavailableTimesConstraintTimeCount(                         */
/*    KHE_AVOID_UNAVAILABLE_TIMES_CONSTRAINT c)                              */
/*                                                                           */
/*  Return the number of times of c.                                         */
/*                                                                           */
/*****************************************************************************/

int KheAvoidUnavailableTimesConstraintTimeCount(
  KHE_AVOID_UNAVAILABLE_TIMES_CONSTRAINT c)
{
  return MArraySize(c->times);
}


/*****************************************************************************/
/*                                                                           */
/*  KHE_TIME KheAvoidUnavailableTimesConstraintTime(                         */
/*    KHE_AVOID_UNAVAILABLE_TIMES_CONSTRAINT c, int i)                       */
/*                                                                           */
/*  Return the i'th time of c.                                               */
/*                                                                           */
/*****************************************************************************/

KHE_TIME KheAvoidUnavailableTimesConstraintTime(
  KHE_AVOID_UNAVAILABLE_TIMES_CONSTRAINT c, int i)
{
  return MArrayGet(c->times, i);
}


/*****************************************************************************/
/*                                                                           */
/*  Submodule "domain"                                                       */
/*                                                                           */
/*****************************************************************************/

/*****************************************************************************/
/*                                                                           */
/*  KHE_TIME_GROUP KheAvoidUnavailableTimesConstraintDomain(                 */
/*    KHE_AVOID_UNAVAILABLE_TIMES_CONSTRAINT c)                              */
/*                                                                           */
/*  Return the domain of c.                                                  */
/*                                                                           */
/*****************************************************************************/

KHE_TIME_GROUP KheAvoidUnavailableTimesConstraintDomain(
  KHE_AVOID_UNAVAILABLE_TIMES_CONSTRAINT c)
{
  MAssert(c->domain != NULL,
   "KheAvoidUnavailableTimesConstraintDomain called before KheInstanceMakeEnd");
  return c->domain;
}


/*****************************************************************************/
/*                                                                           */
/*  Submodule "reading and writing"                                          */
/*                                                                           */
/*****************************************************************************/

/*****************************************************************************/
/*                                                                           */
/*  bool KheAvoidUnavailableTimesConstraintMakeFromKml(KML_ELT cons_elt,     */
/*    KHE_INSTANCE ins, KML_ERROR *ke)                                       */
/*                                                                           */
/*  Make an avoid unavailable times constraint based on cons_elt and         */
/*  add it to ins.                                                           */
/*                                                                           */
/*****************************************************************************/

bool KheAvoidUnavailableTimesConstraintMakeFromKml(KML_ELT cons_elt,
  KHE_INSTANCE ins, KML_ERROR *ke)
{
  char *id, *name;  bool reqd;  int wt;  KHE_COST_FUNCTION cf;
  KML_ELT elt;  KHE_AVOID_UNAVAILABLE_TIMES_CONSTRAINT res;

  /* verify cons_elt and get the common fields */
  if( !KmlCheck(cons_elt, "Id : $Name $Required #Weight "
      "$CostFunction AppliesTo +TimeGroups +Times", ke) )
    return false;
  if( !KheConstraintCheckKml(cons_elt, &id, &name, &reqd, &wt, &cf, ke) )
    return false;

  /* build and insert the constraint object */
  if( !KheAvoidUnavailableTimesConstraintMake(ins, id, name, reqd, wt,cf,&res) )
    return KmlErrorMake(ke, KmlLineNum(cons_elt), KmlColNum(cons_elt),
      "<AvoidUnavailableTimesConstraint> Id \"%s\" used previously", id);

  /* add the resource groups and resources */
  elt = KmlChild(cons_elt, 4);
  if( !KmlCheck(elt, ": +ResourceGroups +Resources", ke) )
    return false;
  if( !KheConstraintAddResourceGroupsFromKml((KHE_CONSTRAINT) res, elt, ke) )
    return false;
  if( !KheConstraintAddResourcesFromKml((KHE_CONSTRAINT) res, elt, ke) )
    return false;

  /* add the time groups and times */
  if( !KheConstraintAddTimeGroupsFromKml((KHE_CONSTRAINT) res, cons_elt, ke) )
    return false;
  if( !KheConstraintAddTimesFromKml((KHE_CONSTRAINT) res, cons_elt, ke) )
    return false;
  return true;

}


/*****************************************************************************/
/*                                                                           */
/*  bool KheAvoidUnavailableTimesConstraintWrite(                            */
/*    KHE_AVOID_UNAVAILABLE_TIMES_CONSTRAINT c, KML_FILE kf)                 */
/*                                                                           */
/*  Write c to kf.                                                           */
/*                                                                           */
/*****************************************************************************/

bool KheAvoidUnavailableTimesConstraintWrite(
  KHE_AVOID_UNAVAILABLE_TIMES_CONSTRAINT c, KML_FILE kf)
{
  KHE_RESOURCE_GROUP rg;  KHE_RESOURCE r;  int i;
  KHE_TIME_GROUP tg;  KHE_TIME t;
  if( c->id == NULL )
    return false;
  KmlBegin(kf, "AvoidUnavailableTimesConstraint");
  KmlAttribute(kf, "Id", c->id);
  if( !KheConstraintWriteCommonFields((KHE_CONSTRAINT) c, kf) )
    return false;
  KmlBegin(kf, "AppliesTo");
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
  KmlEnd(kf, "AppliesTo");
  if( MArraySize(c->time_groups) > 0 )
  {
    KmlBegin(kf, "TimeGroups");
    MArrayForEach(c->time_groups, &tg, &i)
      KmlEltAttribute(kf, "TimeGroup", "Reference", KheTimeGroupId(tg));
    KmlEnd(kf, "TimeGroups");
  }
  if( MArraySize(c->times) > 0 )
  {
    KmlBegin(kf, "Times");
    MArrayForEach(c->times, &t, &i)
      KmlEltAttribute(kf, "Time", "Reference", KheTimeId(t));
    KmlEnd(kf, "Times");
  }
  KmlEnd(kf, "AvoidUnavailableTimesConstraint");
  return true;
}
