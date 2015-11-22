
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
/*  FILE:         khe_link_events_constraint.c                               */
/*  DESCRIPTION:  A link events constraint                                   */
/*                                                                           */
/*****************************************************************************/
#include "khe_interns.h"


/*****************************************************************************/
/*                                                                           */
/*  KHE_LINK_EVENTS_CONSTRAINT - a link events constraint                    */
/*                                                                           */
/*****************************************************************************/

struct khe_link_events_constraint_rec {
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
  ARRAY_KHE_EVENT_GROUP		event_groups;		/* applies to        */
};


/*****************************************************************************/
/*                                                                           */
/*  Submodule "construction and query"                                       */
/*                                                                           */
/*****************************************************************************/

/*****************************************************************************/
/*                                                                           */
/*  bool KheLinkEventsConstraintMake(KHE_INSTANCE ins, char *id,             */
/*    char *name, bool required, int weight, KHE_COST_FUNCTION cf,           */
/*    KHE_LINK_EVENTS_CONSTRAINT *c)                                         */
/*                                                                           */
/*  Make a new link events constraint, add it to ins, and return it.         */
/*                                                                           */
/*****************************************************************************/

bool KheLinkEventsConstraintMake(KHE_INSTANCE ins, char *id,
  char *name, bool required, int weight, KHE_COST_FUNCTION cf,
  KHE_LINK_EVENTS_CONSTRAINT *c)
{
  KHE_LINK_EVENTS_CONSTRAINT res;  KHE_CONSTRAINT cc;
  MAssert(!KheInstanceComplete(ins),
    "KheLinkEventsConstraintMake called after KheInstanceMakeEnd");
  if( id != NULL && KheInstanceRetrieveConstraint(ins, id, &cc) )
  {
    *c = NULL;
    return false;
  }
  MMake(res);
  res->back = NULL;
  res->tag = KHE_LINK_EVENTS_CONSTRAINT_TAG;
  res->instance = ins;
  res->id = id;
  res->name = name;
  res->required = required;
  res->weight = weight;
  res->combined_weight = required ? KheCost(weight, 0) : KheCost(0, weight);
  res->cost_function = cf;
  res->index = KheInstanceConstraintCount(ins);
  MArrayInit(res->event_groups);
  KheInstanceAddConstraint(ins, (KHE_CONSTRAINT) res);
  *c = res;
  return true;
}


/*****************************************************************************/
/*                                                                           */
/*  int KheLinkEventsConstraintAppliesToCount(KHE_LINK_EVENTS_CONSTRAINT c)  */
/*                                                                           */
/*  Return the number of points of application of c.                         */
/*                                                                           */
/*****************************************************************************/

int KheLinkEventsConstraintAppliesToCount(KHE_LINK_EVENTS_CONSTRAINT c)
{
  return MArraySize(c->event_groups);
}


/*****************************************************************************/
/*                                                                           */
/*  void KheLinkEventsConstraintFinalize(KHE_LINK_EVENTS_CONSTRAINT c)       */
/*                                                                           */
/*  Finalize c, since KheInstanceMakeEnd has been called.                    */
/*                                                                           */
/*****************************************************************************/

void KheLinkEventsConstraintFinalize(KHE_LINK_EVENTS_CONSTRAINT c)
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
/*  void KheLinkEventsConstraintAddEventGroup(                               */
/*    KHE_LINK_EVENTS_CONSTRAINT c, KHE_EVENT_GROUP eg)                      */
/*                                                                           */
/*  Add an event group to c.                                                 */
/*                                                                           */
/*****************************************************************************/

void KheLinkEventsConstraintAddEventGroup(KHE_LINK_EVENTS_CONSTRAINT c,
  KHE_EVENT_GROUP eg)
{
  KheEventGroupAddConstraint(eg, (KHE_CONSTRAINT) c);
  MArrayAddLast(c->event_groups, eg);
}


/*****************************************************************************/
/*                                                                           */
/*  int KheLinkEventsConstraintEventGroupCount(                              */
/*    KHE_LINK_EVENTS_CONSTRAINT c)                                          */
/*                                                                           */
/*  Return the number of event groups in c.                                  */
/*                                                                           */
/*****************************************************************************/

int KheLinkEventsConstraintEventGroupCount(KHE_LINK_EVENTS_CONSTRAINT c)
{
  return MArraySize(c->event_groups);
}


/*****************************************************************************/
/*                                                                           */
/*  KHE_EVENT_GROUP KheLinkEventsConstraintEventGroup(                       */
/*    KHE_LINK_EVENTS_CONSTRAINT c, int i)                                   */
/*                                                                           */
/*  Return the i'th event group of c.                                        */
/*                                                                           */
/*****************************************************************************/

KHE_EVENT_GROUP KheLinkEventsConstraintEventGroup(
  KHE_LINK_EVENTS_CONSTRAINT c, int i)
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
/*  bool KheLinkEventsConstraintMakeFromKml(KML_ELT cons_elt,                */
/*    KHE_INSTANCE ins, KML_ERROR *ke)                                       */
/*                                                                           */
/*  Make a link events constraint based on cons_elt and add it to ins.       */
/*                                                                           */
/*****************************************************************************/

bool KheLinkEventsConstraintMakeFromKml(KML_ELT cons_elt,
  KHE_INSTANCE ins, KML_ERROR *ke)
{
  char *id, *name;  bool reqd;  int wt;  KHE_COST_FUNCTION cf;
  KML_ELT elt;  KHE_LINK_EVENTS_CONSTRAINT res;

  /* verify cons_elt and get the common fields */
  if( !KmlCheck(cons_elt, "Id : $Name $Required #Weight "
      "$CostFunction AppliesTo", ke) )
    return false;
  if( !KheConstraintCheckKml(cons_elt, &id, &name, &reqd, &wt, &cf, ke) )
    return false;

  /* build and insert the constraint object */
  if( !KheLinkEventsConstraintMake(ins, id, name, reqd, wt, cf, &res) )
    return KmlErrorMake(ke, KmlLineNum(cons_elt), KmlColNum(cons_elt),
      "<LinkEventsConstraint> Id \"%s\" used previously", id);

  /* find event subgroups */
  elt = KmlChild(cons_elt, 4);
  if( !KmlCheck(elt, ": EventGroups", ke) )
    return false;
  if( !KheConstraintAddEventGroupsFromKml((KHE_CONSTRAINT) res, elt, ke) )
    return false;
  return true;
}


/*****************************************************************************/
/*                                                                           */
/*  bool KheLinkEventsConstraintWrite(KHE_LINK_EVENTS_CONSTRAINT c,          */
/*    KML_FILE kf)                                                           */
/*                                                                           */
/*  Write c to kf.                                                          */
/*                                                                           */
/*****************************************************************************/

bool KheLinkEventsConstraintWrite(KHE_LINK_EVENTS_CONSTRAINT c, KML_FILE kf)
{
  KHE_EVENT_GROUP eg;  int i;
  if( c->id == NULL )
    return false;
  KmlBegin(kf, "LinkEventsConstraint");
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
  KmlEnd(kf, "AppliesTo");
  KmlEnd(kf, "LinkEventsConstraint");
  return true;
}
