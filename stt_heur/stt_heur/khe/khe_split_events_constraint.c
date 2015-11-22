
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
/*  FILE:         khs_split_events_constraint.c                              */
/*  DESCRIPTION:  A split events constraint                                  */
/*                                                                           */
/*****************************************************************************/
#include "khe_interns.h"


/*****************************************************************************/
/*                                                                           */
/*  KHE_SPLIT_EVENTS_CONSTRAINT - a split events constraint                  */
/*                                                                           */
/*****************************************************************************/

struct khe_split_events_constraint_rec {
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
  int				min_duration;		/* MinimumDuration   */
  int				max_duration;		/* MaximumDuration   */
  int				min_amount;		/* MinimumAmount     */
  int				max_amount;		/* MaximumAmount     */
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
/*  bool KheSplitEventsConstraintMake(KHE_INSTANCE ins, char *id,            */
/*    char *name, bool required, int weight, KHE_COST_FUNCTION cf,           */
/*    int min_duration, int max_duration, int min_amount, int max_amount,    */
/*    KHE_SPLIT_EVENTS_CONSTRAINT *c)                                        */
/*                                                                           */
/*  Make an split events constraint, add it to the instance, and return it.  */
/*                                                                           */
/*****************************************************************************/

bool KheSplitEventsConstraintMake(KHE_INSTANCE ins, char *id,
  char *name, bool required, int weight, KHE_COST_FUNCTION cf,
  int min_duration, int max_duration, int min_amount, int max_amount,
  KHE_SPLIT_EVENTS_CONSTRAINT *c)
{
  KHE_SPLIT_EVENTS_CONSTRAINT res;  KHE_CONSTRAINT cc;
  MAssert(!KheInstanceComplete(ins),
    "KheSplitEventsConstraintMake called after KheInstanceMakeEnd");
  if( id != NULL && KheInstanceRetrieveConstraint(ins, id, &cc) )
  {
    *c = NULL;
    return false;
  }
  MMake(res);
  res->back = NULL;
  res->tag = KHE_SPLIT_EVENTS_CONSTRAINT_TAG;
  res->instance = ins;
  res->id = id;
  res->name = name;
  res->required = required;
  res->weight = weight;
  res->combined_weight = required ? KheCost(weight, 0) : KheCost(0, weight);
  res->cost_function = cf;
  res->index = KheInstanceConstraintCount(ins);
  res->min_duration = min_duration;
  res->max_duration = max_duration;
  res->min_amount = min_amount;
  res->max_amount = max_amount;
  MArrayInit(res->events);
  MArrayInit(res->event_groups);
  KheInstanceAddConstraint(ins, (KHE_CONSTRAINT) res);
  *c = res;
  return true;
}


/*****************************************************************************/
/*                                                                           */
/*  int KheSplitEventsConstraintMinDuration(KHE_SPLIT_EVENTS_CONSTRAINT c)   */
/*                                                                           */
/*  Return the MinDuration attribute of c.                                   */
/*                                                                           */
/*****************************************************************************/

int KheSplitEventsConstraintMinDuration(KHE_SPLIT_EVENTS_CONSTRAINT c)
{
  return c->min_duration;
}


/*****************************************************************************/
/*                                                                           */
/*  int KheSplitEventsConstraintMaxDuration(KHE_SPLIT_EVENTS_CONSTRAINT c)   */
/*                                                                           */
/*  Return the MaxDuration attribute of c.                                   */
/*                                                                           */
/*****************************************************************************/

int KheSplitEventsConstraintMaxDuration(KHE_SPLIT_EVENTS_CONSTRAINT c)
{
  return c->max_duration;
}


/*****************************************************************************/
/*                                                                           */
/*  int KheSplitEventsConstraintMinAmount(KHE_SPLIT_EVENTS_CONSTRAINT c)     */
/*                                                                           */
/*  Return the MinAmount attribute of c.                                     */
/*                                                                           */
/*****************************************************************************/

int KheSplitEventsConstraintMinAmount(KHE_SPLIT_EVENTS_CONSTRAINT c)
{
  return c->min_amount;
}


/*****************************************************************************/
/*                                                                           */
/*  int KheSplitEventsConstraintMaxAmount(KHE_SPLIT_EVENTS_CONSTRAINT c)     */
/*                                                                           */
/*  Return the MaxAmount attribute of c.                                     */
/*                                                                           */
/*****************************************************************************/

int KheSplitEventsConstraintMaxAmount(KHE_SPLIT_EVENTS_CONSTRAINT c)
{
  return c->max_amount;
}


/*****************************************************************************/
/*                                                                           */
/*  int KheSplitEventsConstraintAppliesToCount(KHE_SPLIT_EVENTS_CONSTRAINT c)*/
/*                                                                           */
/*  Return the number of points of application of c.                         */
/*                                                                           */
/*****************************************************************************/

int KheSplitEventsConstraintAppliesToCount(KHE_SPLIT_EVENTS_CONSTRAINT c)
{
  int i, res;  KHE_EVENT_GROUP eg;
  res = MArraySize(c->events);
  MArrayForEach(c->event_groups, &eg, &i)
    res += KheEventGroupEventCount(eg);
  return res;
}


/*****************************************************************************/
/*                                                                           */
/*  void KheSplitEventsConstraintFinalize(KHE_SPLIT_EVENTS_CONSTRAINT c)     */
/*                                                                           */
/*  Finalize c, since KheInstanceMakeEnd has been called.                    */
/*                                                                           */
/*****************************************************************************/

void KheSplitEventsConstraintFinalize(KHE_SPLIT_EVENTS_CONSTRAINT c)
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
/*  void KheSplitEventsConstraintAddEvent(KHE_SPLIT_EVENTS_CONSTRAINT c,     */
/*    KHE_EVENT e)                                                           */
/*                                                                           */
/*  Add e to c.                                                              */
/*                                                                           */
/*****************************************************************************/

void KheSplitEventsConstraintAddEvent(KHE_SPLIT_EVENTS_CONSTRAINT c,
  KHE_EVENT e)
{
  MArrayAddLast(c->events, e);
  KheEventAddConstraint(e, (KHE_CONSTRAINT) c);
}


/*****************************************************************************/
/*                                                                           */
/*  int KheSplitEventsConstraintEventCount(KHE_SPLIT_EVENTS_CONSTRAINT c)    */
/*                                                                           */
/*  Return the number of events of c.                                        */
/*                                                                           */
/*****************************************************************************/

int KheSplitEventsConstraintEventCount(KHE_SPLIT_EVENTS_CONSTRAINT c)
{
  return MArraySize(c->events);
}


/*****************************************************************************/
/*                                                                           */
/*  KHE_EVENT KheSplitEventsConstraintEvent(                                 */
/*    KHE_SPLIT_EVENTS_CONSTRAINT c, int i)                                  */
/*                                                                           */
/*  Return the i'th event of c.                                              */
/*                                                                           */
/*****************************************************************************/

KHE_EVENT KheSplitEventsConstraintEvent(KHE_SPLIT_EVENTS_CONSTRAINT c, int i)
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
/*  void KheSplitEventsConstraintAddEventGroup(KHE_SPLIT_EVENTS_CONSTRAINT c,*/
/*    KHE_EVENT_GROUP eg)                                                    */
/*                                                                           */
/*  Add eg to c, and also add the corresponding event resources to c.        */
/*                                                                           */
/*****************************************************************************/

void KheSplitEventsConstraintAddEventGroup(KHE_SPLIT_EVENTS_CONSTRAINT c,
  KHE_EVENT_GROUP eg)
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
/*  int KheSplitEventsConstraintEventGroupCount(                             */
/*    KHE_SPLIT_EVENTS_CONSTRAINT c)                                         */
/*                                                                           */
/*  Return the number of event groups of c.                                  */
/*                                                                           */
/*****************************************************************************/

int KheSplitEventsConstraintEventGroupCount(KHE_SPLIT_EVENTS_CONSTRAINT c)
{
  return MArraySize(c->event_groups);
}


/*****************************************************************************/
/*                                                                           */
/*  KHE_EVENT_GROUP KheSplitEventsConstraintEventGroup(                      */
/*    KHE_SPLIT_EVENTS_CONSTRAINT c, int i)                                  */
/*                                                                           */
/*  Return the ith event group of c.                                         */
/*                                                                           */
/*****************************************************************************/

KHE_EVENT_GROUP KheSplitEventsConstraintEventGroup(
  KHE_SPLIT_EVENTS_CONSTRAINT c, int i)
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
/*  bool KheSplitEventsConstraintMakeFromKml(KML_ELT cons_elt,               */
/*    KHE_INSTANCE ins, KML_ERROR *ke)                                       */
/*                                                                           */
/*  Add a split events constraint based on cons_elt to ins.                  */
/*                                                                           */
/*****************************************************************************/

bool KheSplitEventsConstraintMakeFromKml(KML_ELT cons_elt,
  KHE_INSTANCE ins, KML_ERROR *ke)
{
  char *id, *name;  bool reqd;  int wt;  KHE_COST_FUNCTION cf;
  KML_ELT elt;  KHE_SPLIT_EVENTS_CONSTRAINT res;
  int min_duration, max_duration, min_amount, max_amount;

  /* verify cons_elt and get the common fields */
  if( !KmlCheck(cons_elt,
      "Id : $Name $Required #Weight $CostFunction AppliesTo"
      " #MinimumDuration #MaximumDuration #MinimumAmount #MaximumAmount", ke) )
    return false;
  if( !KheConstraintCheckKml(cons_elt, &id, &name, &reqd, &wt, &cf, ke) )
    return false;

  /* find the numeric fields */
  sscanf(KmlText(KmlChild(cons_elt, 5)), "%d", &min_duration);
  sscanf(KmlText(KmlChild(cons_elt, 6)), "%d", &max_duration);
  sscanf(KmlText(KmlChild(cons_elt, 7)), "%d", &min_amount);
  sscanf(KmlText(KmlChild(cons_elt, 8)), "%d", &max_amount);

  /* make the constraint object and add it to ins */
  if( !KheSplitEventsConstraintMake(ins, id, name, reqd, wt, cf,
	min_duration, max_duration, min_amount, max_amount, &res) )
    return KmlErrorMake(ke, KmlLineNum(cons_elt), KmlColNum(cons_elt),
      "<SplitEventsConstraint> Id \"%s\" used previously", id);

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
/*  bool KheSplitEventsConstraintWrite(KHE_SPLIT_EVENTS_CONSTRAINT c,        */
/*    KML_FILE kf)                                                           */
/*                                                                           */
/*  Write c to kf.                                                           */
/*                                                                           */
/*****************************************************************************/

bool KheSplitEventsConstraintWrite(KHE_SPLIT_EVENTS_CONSTRAINT c,
  KML_FILE kf)
{
  KHE_EVENT_GROUP eg;  KHE_EVENT e;  int i;
  if( c->id == NULL )
    return false;
  KmlBegin(kf, "SplitEventsConstraint");
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
  KmlEltPrintf(kf, "MinimumDuration", "%d", c->min_duration);
  KmlEltPrintf(kf, "MaximumDuration", "%d", c->max_duration);
  KmlEltPrintf(kf, "MinimumAmount", "%d", c->min_amount);
  KmlEltPrintf(kf, "MaximumAmount", "%d", c->max_amount);
  KmlEnd(kf, "SplitEventsConstraint");
  return true;
}


/*****************************************************************************/
/*                                                                           */
/*  Submodule "debug"                                                        */
/*                                                                           */
/*****************************************************************************/

/*****************************************************************************/
/*                                                                           */
/*  void KheSplitEventsConstraintDebug(KHE_SPLIT_EVENTS_CONSTRAINT c,        */
/*    int verbosity, int indent, FILE *fp)                                   */
/*                                                                           */
/*  Debug print of c onto fp with the given verbosity and indent.            */
/*                                                                           */
/*****************************************************************************/

void KheSplitEventsConstraintDebug(KHE_SPLIT_EVENTS_CONSTRAINT c,
  int verbosity, int indent, FILE *fp)
{
  if( verbosity >= 1 )
  {
    if( indent >= 0 )
      fprintf(fp, "%*s", indent, "");
    fprintf(fp, "[ %s %s%d: duration %d-%d, amount %d-%d ]",
      c->id != NULL ? c->id : "-", c->required ? "r" : "", c->weight,
      c->min_duration, c->max_duration, c->min_amount, c->max_amount);
    if( indent >= 0 )
      fprintf(fp, "\n");
  }
}
