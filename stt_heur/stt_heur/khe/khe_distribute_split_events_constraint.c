
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
/*  FILE:         khs_distribute_split_events_constraint.c                   */
/*  DESCRIPTION:  A distribute split events constraint                       */
/*                                                                           */
/*****************************************************************************/
#include "khe_interns.h"


/*****************************************************************************/
/*                                                                           */
/*  KHE_DISTRIBUTE_SPLIT_EVENTS_CONSTRAINT - a distribute split events cons. */
/*                                                                           */
/*****************************************************************************/

struct khe_distribute_split_events_constraint_rec {
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
  int				duration;		/* Duration          */
  int				minimum;		/* Minimum           */
  int				maximum;		/* Maximum           */
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
/*  bool KheDistributeSplitEventsConstraintMake(KHE_INSTANCE ins, char *id,  */
/*    char *name, bool required, int weight, KHE_COST_FUNCTION cf,           */
/*    int duration, int minimum, int maximum,                                */
/*    KHE_DISTRIBUTE_SPLIT_EVENTS_CONSTRAINT *c)                             */
/*                                                                           */
/*  Make a distribute split events constraint, add it to the instance, and   */
/*  return it.                                                               */
/*                                                                           */
/*****************************************************************************/

bool KheDistributeSplitEventsConstraintMake(KHE_INSTANCE ins, char *id,
  char *name, bool required, int weight, KHE_COST_FUNCTION cf, int duration,
  int minimum, int maximum, KHE_DISTRIBUTE_SPLIT_EVENTS_CONSTRAINT *c)
{
  KHE_DISTRIBUTE_SPLIT_EVENTS_CONSTRAINT res;  KHE_CONSTRAINT cc;
  MAssert(!KheInstanceComplete(ins),
    "KheDistributeSplitEventsConstraintMake called after KheInstanceMakeEnd");
  if( id != NULL && KheInstanceRetrieveConstraint(ins, id, &cc) )
  {
    *c = NULL;
    return false;
  }
  MMake(res);
  res->back = NULL;
  res->tag = KHE_DISTRIBUTE_SPLIT_EVENTS_CONSTRAINT_TAG;
  res->instance = ins;
  res->id = id;
  res->name = name;
  res->required = required;
  res->weight = weight;
  res->combined_weight = required ? KheCost(weight, 0) : KheCost(0, weight);
  res->cost_function = cf;
  res->index = KheInstanceConstraintCount(ins);
  res->duration = duration;
  /* res->time_group = tg; */
  res->minimum = minimum;
  res->maximum = maximum;
  MArrayInit(res->events);
  MArrayInit(res->event_groups);
  KheInstanceAddConstraint(ins, (KHE_CONSTRAINT) res);
  *c = res;
  return true;
}


/*****************************************************************************/
/*                                                                           */
/*  int KheDistributeSplitEventsConstraintDuration(                          */
/*    KHE_DISTRIBUTE_SPLIT_EVENTS_CONSTRAINT c)                              */
/*                                                                           */
/*  Return the Duration attribute of c.                                      */
/*                                                                           */
/*****************************************************************************/

int KheDistributeSplitEventsConstraintDuration(
  KHE_DISTRIBUTE_SPLIT_EVENTS_CONSTRAINT c)
{
  return c->duration;
}


/*****************************************************************************/
/*                                                                           */
/*  int KheDistributeSplitEventsConstraintMinimum(                           */
/*    KHE_DISTRIBUTE_SPLIT_EVENTS_CONSTRAINT c)                              */
/*                                                                           */
/*  Return the Minimum attribute of c.                                       */
/*                                                                           */
/*****************************************************************************/

int KheDistributeSplitEventsConstraintMinimum(
  KHE_DISTRIBUTE_SPLIT_EVENTS_CONSTRAINT c)
{
  return c->minimum;
}


/*****************************************************************************/
/*                                                                           */
/*  int KheDistributeSplitEventsConstraintMaximum(                           */
/*    KHE_DISTRIBUTE_SPLIT_EVENTS_CONSTRAINT c)                              */
/*                                                                           */
/*  Return the Maximum attribute of c.                                       */
/*                                                                           */
/*****************************************************************************/

int KheDistributeSplitEventsConstraintMaximum(
  KHE_DISTRIBUTE_SPLIT_EVENTS_CONSTRAINT c)
{
  return c->maximum;
}


/*****************************************************************************/
/*                                                                           */
/*  int KheDistributeSplitEventsConstraintAppliesToCount(                    */
/*    KHE_DISTRIBUTE_SPLIT_EVENTS_CONSTRAINT c)                              */
/*                                                                           */
/*  Return the number of points of application of c.                         */
/*                                                                           */
/*****************************************************************************/

int KheDistributeSplitEventsConstraintAppliesToCount(
  KHE_DISTRIBUTE_SPLIT_EVENTS_CONSTRAINT c)
{
  int i, res;  KHE_EVENT_GROUP eg;
  res = MArraySize(c->events);
  MArrayForEach(c->event_groups, &eg, &i)
    res += KheEventGroupEventCount(eg);
  return res;
}


/*****************************************************************************/
/*                                                                           */
/*  void KheDistributeSplitEventsConstraintFinalize(                         */
/*    KHE_DISTRIBUTE_SPLIT_EVENTS_CONSTRAINT c)                              */
/*                                                                           */
/*  Finalize c, since KheInstanceMakeEnd has been called.                    */
/*                                                                           */
/*****************************************************************************/

void KheDistributeSplitEventsConstraintFinalize(
  KHE_DISTRIBUTE_SPLIT_EVENTS_CONSTRAINT c)
{
  /* nothing to do in this case */
}


/*****************************************************************************/
/*                                                                           */
/*  Submodule "events"                                                       */
/*                                                                           */
/*****************************************************************************/

/*****************************************************************************/
/*                                                                           */
/*  void KheDistributeSplitEventsConstraintAddEvent(                         */
/*    KHE_DISTRIBUTE_SPLIT_EVENTS_CONSTRAINT c, KHE_EVENT e)                 */
/*                                                                           */
/*  Add e to c.                                                              */
/*                                                                           */
/*****************************************************************************/

void KheDistributeSplitEventsConstraintAddEvent(
  KHE_DISTRIBUTE_SPLIT_EVENTS_CONSTRAINT c, KHE_EVENT e)
{
  MArrayAddLast(c->events, e);
  KheEventAddConstraint(e, (KHE_CONSTRAINT) c);
}


/*****************************************************************************/
/*                                                                           */
/*  int KheDistributeSplitEventsConstraintEventCount(                        */
/*    KHE_DISTRIBUTE_SPLIT_EVENTS_CONSTRAINT c)                              */
/*                                                                           */
/*  Return the number of events of c.                                        */
/*                                                                           */
/*****************************************************************************/

int KheDistributeSplitEventsConstraintEventCount(
  KHE_DISTRIBUTE_SPLIT_EVENTS_CONSTRAINT c)
{
  return MArraySize(c->events);
}


/*****************************************************************************/
/*                                                                           */
/*  KHE_EVENT KheDistributeSplitEventsConstraintEvent(                       */
/*    KHE_DISTRIBUTE_SPLIT_EVENTS_CONSTRAINT c, int i)                       */
/*                                                                           */
/*  Return the i'th event of c.                                              */
/*                                                                           */
/*****************************************************************************/

KHE_EVENT KheDistributeSplitEventsConstraintEvent(
  KHE_DISTRIBUTE_SPLIT_EVENTS_CONSTRAINT c, int i)
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
/*  void KheDistributeSplitEventsConstraintAddEventGroup(                    */
/*    KHE_DISTRIBUTE_SPLIT_EVENTS_CONSTRAINT c, KHE_EVENT_GROUP eg)          */
/*                                                                           */
/*  Add eg to c, and also add the corresponding event resources to c.        */
/*                                                                           */
/*****************************************************************************/

void KheDistributeSplitEventsConstraintAddEventGroup(
  KHE_DISTRIBUTE_SPLIT_EVENTS_CONSTRAINT c, KHE_EVENT_GROUP eg)
{
  int i;  KHE_EVENT e;
  MArrayAddLast(c->event_groups, eg);
  for( i = 0;  i < KheEventGroupEventCount(eg);  i++ )
  {
    e = KheEventGroupEvent(eg, i);
    KheEventAddConstraint(e, (KHE_CONSTRAINT) c);
  }
}


/*****************************************************************************/
/*                                                                           */
/*  int KheDistributeSplitEventsConstraintEventGroupCount(                   */
/*    KHE_DISTRIBUTE_SPLIT_EVENTS_CONSTRAINT c)                              */
/*                                                                           */
/*  Return the number of event groups of c.                                  */
/*                                                                           */
/*****************************************************************************/

int KheDistributeSplitEventsConstraintEventGroupCount(
  KHE_DISTRIBUTE_SPLIT_EVENTS_CONSTRAINT c)
{
  return MArraySize(c->event_groups);
}


/*****************************************************************************/
/*                                                                           */
/*  KHE_EVENT_GROUP KheDistributeSplitEventsConstraintEventGroup(            */
/*    KHE_DISTRIBUTE_SPLIT_EVENTS_CONSTRAINT c, int i)                       */
/*                                                                           */
/*  Return the ith event group of c.                                         */
/*                                                                           */
/*****************************************************************************/

KHE_EVENT_GROUP KheDistributeSplitEventsConstraintEventGroup(
  KHE_DISTRIBUTE_SPLIT_EVENTS_CONSTRAINT c, int i)
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
/*  bool KheDistributeSplitEventsConstraintMakeFromKml(KML_ELT cons_elt,     */
/*    KHE_INSTANCE ins, KML_ERROR *ke)                                       */
/*                                                                           */
/*  Make a distribute split events constraint based on cons_elt and add      */
/*  it to ins.                                                               */
/*                                                                           */
/*****************************************************************************/

bool KheDistributeSplitEventsConstraintMakeFromKml(KML_ELT cons_elt,
  KHE_INSTANCE ins, KML_ERROR *ke)
{
  char *id, *name;  bool reqd;  int wt;  KHE_COST_FUNCTION cf;
  KML_ELT elt;  KHE_DISTRIBUTE_SPLIT_EVENTS_CONSTRAINT res;
  int duration, minimum, maximum;

  /* verify cons_elt and get the common fields */
  if( !KmlCheck(cons_elt,
      "Id : $Name $Required #Weight $CostFunction AppliesTo"
      " #Duration #Minimum #Maximum", ke) )
    return false;
  if( !KheConstraintCheckKml(cons_elt, &id, &name, &reqd, &wt, &cf, ke) )
    return false;

  /* find the numeric fields */
  sscanf(KmlText(KmlChild(cons_elt, 5)), "%d", &duration);
  sscanf(KmlText(KmlChild(cons_elt, 6)), "%d", &minimum);
  sscanf(KmlText(KmlChild(cons_elt, 7)), "%d", &maximum);

  /* make the constraint object and add it to ins */
  if( !KheDistributeSplitEventsConstraintMake(ins, id, name, reqd, wt, cf,
	duration, minimum, maximum, &res) )
    return KmlErrorMake(ke, KmlLineNum(cons_elt), KmlColNum(cons_elt),
      "<DistributeSplitEventsConstraint> Id \"%s\" used previously", id);

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
/*  bool KheDistributeSplitEventsConstraintWrite(                            */
/*    KHE_DISTRIBUTE_SPLIT_EVENTS_CONSTRAINT c, KML_FILE kf)                 */
/*                                                                           */
/*  Write c to kf.                                                           */
/*                                                                           */
/*****************************************************************************/

bool KheDistributeSplitEventsConstraintWrite(
  KHE_DISTRIBUTE_SPLIT_EVENTS_CONSTRAINT c, KML_FILE kf)
{
  KHE_EVENT_GROUP eg;  KHE_EVENT e;  int i;
  if( c->id == NULL )
    return false;
  KmlBegin(kf, "DistributeSplitEventsConstraint");
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
  KmlEltPrintf(kf, "Duration", "%d", c->duration);
  KmlEltPrintf(kf, "Minimum", "%d", c->minimum);
  KmlEltPrintf(kf, "Maximum", "%d", c->maximum);
  KmlEnd(kf, "DistributeSplitEventsConstraint");
  return true;
}


/*****************************************************************************/
/*                                                                           */
/*  Submodule "debug"                                                        */
/*                                                                           */
/*****************************************************************************/

/*****************************************************************************/
/*                                                                           */
/*  void KheDistributeSplitEventsConstraintDebug(                            */
/*    KHE_DISTRIBUTE_SPLIT_EVENTS_CONSTRAINT c, int verbosity,               */
/*    int indent, FILE *fp)                                                  */
/*                                                                           */
/*  Debug print of c onto fp with the given verbosity and indent.            */
/*                                                                           */
/*****************************************************************************/

void KheDistributeSplitEventsConstraintDebug(
  KHE_DISTRIBUTE_SPLIT_EVENTS_CONSTRAINT c, int verbosity,
  int indent, FILE *fp)
{
  if( verbosity >= 1 )
  {
    if( indent >= 0 )
      fprintf(fp, "%*s", indent, "");
    fprintf(fp, "[ %s %s%d: duration %d, amount %d-%d ]",
      c->id != NULL ? c->id : "-", c->required ? "r" : "", c->weight,
      c->duration, c->minimum, c->maximum);
    if( indent >= 0 )
      fprintf(fp, "\n");
  }
}
