
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
/*  FILE:         khe_spread_events_constraint.c                             */
/*  DESCRIPTION:  A spread events constraint                                 */
/*                                                                           */
/*****************************************************************************/
#include "khe_interns.h"


/*****************************************************************************/
/*                                                                           */
/*  KHE_SPREAD_EVENTS_CONSTRAINT - a spread events constraint                */
/*                                                                           */
/*****************************************************************************/

struct khe_spread_events_constraint_rec {
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
  KHE_TIME_SPREAD		time_spread;		/* time spread       */
};


/*****************************************************************************/
/*                                                                           */
/*  KHE_TIME_SPREAD - a time spread                                          */
/*                                                                           */
/*****************************************************************************/

struct khe_time_spread_rec {
  KHE_INSTANCE			instance;
  bool				finalized;
  bool				time_groups_disjoint;
  bool				time_groups_cover_whole_cycle;
  ARRAY_KHE_LIMITED_TIME_GROUP	limited_time_groups;
};


/*****************************************************************************/
/*                                                                           */
/*  KHE_LIMITED_TIME_GROUP - a limited time group                            */
/*                                                                           */
/*****************************************************************************/

struct khe_limited_time_group_rec {
  KHE_TIME_GROUP		time_group;		/* the time group    */
  int				minimum;		/* min limit         */
  int				maximum;		/* max limit         */
};


/*****************************************************************************/
/*                                                                           */
/*  Submodule "construction and query"                                       */
/*                                                                           */
/*****************************************************************************/

/*****************************************************************************/
/*                                                                           */
/*  bool KheSpreadEventsConstraintMake(KHE_INSTANCE ins, char *id,           */
/*    char *name, bool required, int weight, KHE_COST_FUNCTION cf,           */
/*    KHE_TIME_SPREAD ts, KHE_SPREAD_EVENTS_CONSTRAINT *c)                   */
/*                                                                           */
/*  Make a new spread events constraint, add it to ins, and return it.       */
/*                                                                           */
/*****************************************************************************/

bool KheSpreadEventsConstraintMake(KHE_INSTANCE ins, char *id,
  char *name, bool required, int weight, KHE_COST_FUNCTION cf,
  KHE_TIME_SPREAD ts, KHE_SPREAD_EVENTS_CONSTRAINT *c)
{
  KHE_SPREAD_EVENTS_CONSTRAINT res;  KHE_CONSTRAINT cc;
  MAssert(!KheInstanceComplete(ins),
    "KheSpreadEventsConstraintMake called after KheInstanceMakeEnd");
  if( id != NULL && KheInstanceRetrieveConstraint(ins, id, &cc) )
  {
    *c = NULL;
    return false;
  }
  MMake(res);
  res->back = NULL;
  res->tag = KHE_SPREAD_EVENTS_CONSTRAINT_TAG;
  res->instance = ins;
  res->id = id;
  res->name = name;
  res->required = required;
  res->weight = weight;
  res->combined_weight = required ? KheCost(weight, 0) : KheCost(0, weight);
  res->cost_function = cf;
  res->index = KheInstanceConstraintCount(ins);
  MArrayInit(res->event_groups);
  res->time_spread = ts;
  KheInstanceAddConstraint(ins, (KHE_CONSTRAINT) res);
  *c = res;
  return true;
}


/*****************************************************************************/
/*                                                                           */
/*  KHE_TIME_SPREAD KheSpreadEventsConstraintTimeSpread(                     */
/*    KHE_SPREAD_EVENTS_CONSTRAINT c)                                        */
/*                                                                           */
/*  Return the time spread attribute of c.                                   */
/*                                                                           */
/*****************************************************************************/

KHE_TIME_SPREAD KheSpreadEventsConstraintTimeSpread(
  KHE_SPREAD_EVENTS_CONSTRAINT c)
{
  return c->time_spread;
}


/*****************************************************************************/
/*                                                                           */
/*  int KheSpreadEventsConstraintAppliesToCount(                             */
/*    KHE_SPREAD_EVENTS_CONSTRAINT c)                                        */
/*                                                                           */
/*  Return the number of points of application of c.                         */
/*                                                                           */
/*****************************************************************************/

int KheSpreadEventsConstraintAppliesToCount(KHE_SPREAD_EVENTS_CONSTRAINT c)
{
  return MArraySize(c->event_groups);
}


/*****************************************************************************/
/*                                                                           */
/*  void KheSpreadEventsConstraintFinalize(KHE_SPREAD_EVENTS_CONSTRAINT c)   */
/*                                                                           */
/*  Finalize c, since KheInstanceMakeEnd has been called.                    */
/*                                                                           */
/*****************************************************************************/
static void KheTimeSpreadFinalize(KHE_TIME_SPREAD ts);

void KheSpreadEventsConstraintFinalize(KHE_SPREAD_EVENTS_CONSTRAINT c)
{
  KheTimeSpreadFinalize(c->time_spread);
}


/*****************************************************************************/
/*                                                                           */
/*  Submodule "time spreads"                                                 */
/*                                                                           */
/*****************************************************************************/

/*****************************************************************************/
/*                                                                           */
/*  KHE_TIME_SPREAD KheTimeSpreadMake(KHE_INSTANCE ins)                      */
/*                                                                           */
/*  Make and return a new time spread object for ins.                        */
/*                                                                           */
/*****************************************************************************/

KHE_TIME_SPREAD KheTimeSpreadMake(KHE_INSTANCE ins)
{
  KHE_TIME_SPREAD res;
  MMake(res);
  res->instance = ins;
  res->finalized = false;
  res->time_groups_disjoint = false;		/* actually undefined here */
  res->time_groups_cover_whole_cycle = false;	/* actually undefined here */
  MArrayInit(res->limited_time_groups);
  return res;
}


/*****************************************************************************/
/*                                                                           */
/*  void KheTimeSpreadAddLimitedTimeGroup(KHE_TIME_SPREAD ts,                */
/*    KHE_LIMITED_TIME_GROUP ltg)                                            */
/*                                                                           */
/*  Add ltg to ts.                                                           */
/*                                                                           */
/*****************************************************************************/

void KheTimeSpreadAddLimitedTimeGroup(KHE_TIME_SPREAD ts,
  KHE_LIMITED_TIME_GROUP ltg)
{
  MArrayAddLast(ts->limited_time_groups, ltg);
}


/*****************************************************************************/
/*                                                                           */
/*  int KheTimeSpreadLimitedTimeGroupCount(KHE_TIME_SPREAD ts)               */
/*                                                                           */
/*  Return the number of limited time groups in ts.                          */
/*                                                                           */
/*****************************************************************************/

int KheTimeSpreadLimitedTimeGroupCount(KHE_TIME_SPREAD ts)
{
  return MArraySize(ts->limited_time_groups);
}


/*****************************************************************************/
/*                                                                           */
/*  KHE_LIMITED_TIME_GROUP KheTimeSpreadLimitedTimeGroup(KHE_TIME_SPREAD ts, */
/*    int i)                                                                 */
/*                                                                           */
/*  Return the i'th limited time group of ts.                                */
/*                                                                           */
/*****************************************************************************/

KHE_LIMITED_TIME_GROUP KheTimeSpreadLimitedTimeGroup(KHE_TIME_SPREAD ts,
  int i)
{
  return MArrayGet(ts->limited_time_groups, i);
}


/*****************************************************************************/
/*                                                                           */
/*  bool KheLimitedTimeGroupIsDisjoint(KHE_TIME_SPREAD ts, int count,        */
/*    KHE_LIMITED_TIME_GROUP ltg)                                            */
/*                                                                           */
/*  Return true if ltg is disjoint from the first count ltgs of ts.          */
/*                                                                           */
/*****************************************************************************/

static bool KheLimitedTimeGroupIsDisjoint(KHE_TIME_SPREAD ts, int count,
  KHE_LIMITED_TIME_GROUP ltg)
{
  KHE_LIMITED_TIME_GROUP ltg1;  int i;
  for( i = 0;  i < count;  i++ )
  {
    ltg1 = MArrayGet(ts->limited_time_groups, i);
    if( !KheTimeGroupDisjoint(ltg1->time_group, ltg->time_group) )
      return false;
  }
  return true;
}


/*****************************************************************************/
/*                                                                           */
/*  bool KheTimeSpreadContainsTime(KHE_TIME_SPREAD ts, KHE_TIME t)           */
/*                                                                           */
/*  Return true if t appears within at least one of the time groups of ts.   */
/*                                                                           */
/*****************************************************************************/

static bool KheTimeSpreadContainsTime(KHE_TIME_SPREAD ts, KHE_TIME t)
{
  int i;  KHE_LIMITED_TIME_GROUP ltg;
  MArrayForEach(ts->limited_time_groups, &ltg, &i)
    if( KheTimeGroupContains(ltg->time_group, t) )
      return true;
  return false;
}


/*****************************************************************************/
/*                                                                           */
/*  void KheTimeSpreadFinalize(KHE_TIME_SPREAD ts)                           */
/*                                                                           */
/*  Finalize ts (work out whether its time groups are disjoint, and          */
/*  whether they cover the whole cycle) if not done already.                 */
/*                                                                           */
/*****************************************************************************/

static void KheTimeSpreadFinalize(KHE_TIME_SPREAD ts)
{
  KHE_LIMITED_TIME_GROUP ltg;  int i;  KHE_TIME t;
  if( !ts->finalized )
  {
    MAssert(KheInstanceComplete(ts->instance), 
      "KheTimeSpreadFinalize internal error: incomplete instance");
    ts->finalized = true;

    /* work out whether the time groups are disjoint */
    ts->time_groups_disjoint = true;
    for( i = 1;  i < MArraySize(ts->limited_time_groups);  i++ )
    {
      ltg = MArrayGet(ts->limited_time_groups, i);
      if( !KheLimitedTimeGroupIsDisjoint(ts, i, ltg) )
      {
	ts->time_groups_disjoint = false;
	break;
      }
    }

    /* work out whether the time groups cover the whole cycle */
    ts->time_groups_cover_whole_cycle = true;
    for( i = 0;  i < KheInstanceTimeCount(ts->instance);  i++ )
    {
      t = KheInstanceTime(ts->instance, i);
      if( !KheTimeSpreadContainsTime(ts, t) )
      {
	ts->time_groups_cover_whole_cycle = false;
	break;
      }
    }
  }
}


/*****************************************************************************/
/*                                                                           */
/*  bool KheTimeSpreadTimeGroupsDisjoint(KHE_TIME_SPREAD ts)                 */
/*                                                                           */
/*  Return true if the time groups of ts's limited time groups are           */
/*  pairwise disjoint.                                                       */
/*                                                                           */
/*****************************************************************************/

bool KheTimeSpreadTimeGroupsDisjoint(KHE_TIME_SPREAD ts)
{
  MAssert(KheInstanceComplete(ts->instance), 
    "KheTimeSpreadTimeGroupsDisjoint called before KheInstanceMakeEnd");
  return ts->time_groups_disjoint;
}


/*****************************************************************************/
/*                                                                           */
/*  bool KheTimeSpreadCoversWholeCycle(KHE_TIME_SPREAD ts)                   */
/*                                                                           */
/*  Return true if every time of the cycle appears in at least one of        */
/*  the time groups of ts's limited time groups.                             */
/*                                                                           */
/*****************************************************************************/

bool KheTimeSpreadCoversWholeCycle(KHE_TIME_SPREAD ts)
{
  MAssert(KheInstanceComplete(ts->instance), 
    "KheTimeSpreadCoversWholeCycle called before KheInstanceMakeEnd");
  return ts->time_groups_cover_whole_cycle;
}


/*****************************************************************************/
/*                                                                           */
/*  Submodule "limited time groups"                                          */
/*                                                                           */
/*****************************************************************************/

/*****************************************************************************/
/*                                                                           */
/*  KHE_LIMITED_TIME_GROUP KheLimitedTimeGroupMake(KHE_TIME_GROUP tg,        */
/*    int minimum, int maximum)                                              */
/*                                                                           */
/*  Make and return a new limited time group with these attributes.          */
/*                                                                           */
/*****************************************************************************/

KHE_LIMITED_TIME_GROUP KheLimitedTimeGroupMake(KHE_TIME_GROUP tg,
  int minimum, int maximum)
{
  KHE_LIMITED_TIME_GROUP res;
  MMake(res);
  res->time_group = tg;
  res->minimum = minimum;
  res->maximum = maximum;
  return res;
}


/*****************************************************************************/
/*                                                                           */
/*  KHE_TIME_GROUP KheLimitedTimeGroupTimeGroup(KHE_LIMITED_TIME_GROUP ltg)  */
/*                                                                           */
/*  Return the time group attribute of ltg.                                  */
/*                                                                           */
/*****************************************************************************/

KHE_TIME_GROUP KheLimitedTimeGroupTimeGroup(KHE_LIMITED_TIME_GROUP ltg)
{
  return ltg->time_group;
}


/*****************************************************************************/
/*                                                                           */
/*  int KheLimitedTimeGroupMinimum(KHE_LIMITED_TIME_GROUP ltg)               */
/*                                                                           */
/*  Return the minimum attribute of ltg.                                     */
/*                                                                           */
/*****************************************************************************/

int KheLimitedTimeGroupMinimum(KHE_LIMITED_TIME_GROUP ltg)
{
  return ltg->minimum;
}


/*****************************************************************************/
/*                                                                           */
/*  int KheLimitedTimeGroupMaximum(KHE_LIMITED_TIME_GROUP ltg)               */
/*                                                                           */
/*  Return the maximum attribute of ltg.                                     */
/*                                                                           */
/*****************************************************************************/

int KheLimitedTimeGroupMaximum(KHE_LIMITED_TIME_GROUP ltg)
{
  return ltg->maximum;
}


/*****************************************************************************/
/*                                                                           */
/*  Submodule "event groups"                                                 */
/*                                                                           */
/*****************************************************************************/

/*****************************************************************************/
/*                                                                           */
/*  void KheSpreadEventsConstraintAddEventGroup(                             */
/*    KHE_SPREAD_EVENTS_CONSTRAINT c, KHE_EVENT_GROUP eg)                    */
/*                                                                           */
/*  Add an event group to c.                                                 */
/*                                                                           */
/*****************************************************************************/

void KheSpreadEventsConstraintAddEventGroup(KHE_SPREAD_EVENTS_CONSTRAINT c,
  KHE_EVENT_GROUP eg)
{
  KheEventGroupAddConstraint(eg, (KHE_CONSTRAINT) c);
  MArrayAddLast(c->event_groups, eg);
}


/*****************************************************************************/
/*                                                                           */
/*  int KheSpreadEventsConstraintEventGroupCount(                            */
/*    KHE_SPREAD_EVENTS_CONSTRAINT c)                                        */
/*                                                                           */
/*  Return the number of event groups in c.                                  */
/*                                                                           */
/*****************************************************************************/

int KheSpreadEventsConstraintEventGroupCount(KHE_SPREAD_EVENTS_CONSTRAINT c)
{
  return MArraySize(c->event_groups);
}


/*****************************************************************************/
/*                                                                           */
/*  KHE_EVENT_GROUP KheSpreadEventsConstraintEventGroup(                     */
/*    KHE_SPREAD_EVENTS_CONSTRAINT c, int i)                                 */
/*                                                                           */
/*  Return the i'th event group of c.                                        */
/*                                                                           */
/*****************************************************************************/

KHE_EVENT_GROUP KheSpreadEventsConstraintEventGroup(
  KHE_SPREAD_EVENTS_CONSTRAINT c, int i)
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
/*  bool GetTimeSpread(KML_ELT elt, KHE_INSTANCE ins, KHE_TIME_SPREAD *ts,   */
/*    KML_ERROR *ke)                                                         */
/*                                                                           */
/*  Build a time spread based on elt.                                        */
/*                                                                           */
/*****************************************************************************/

static bool GetTimeSpread(KML_ELT elt, KHE_INSTANCE ins, KHE_TIME_SPREAD *ts,
  KML_ERROR *ke)
{
  KML_ELT time_group_elt;  char *ref;  KHE_TIME_GROUP tg;
  KHE_LIMITED_TIME_GROUP ltg;  int i, min_limit, max_limit;
  if( !KmlCheck(elt, ": *TimeGroup", ke) )
    return false;
  *ts = KheTimeSpreadMake(ins);
  for( i = 0;  i < KmlChildCount(elt);  i++ )
  {
    time_group_elt = KmlChild(elt, i);
    if( !KmlCheck(time_group_elt, "Reference : #Minimum #Maximum", ke) )
      return false;
    ref = KmlAttributeValue(time_group_elt, 0);
    if( !KheInstanceRetrieveTimeGroup(ins, ref, &tg) )
      return KmlErrorMake(ke, KmlLineNum(time_group_elt),
	KmlColNum(time_group_elt), "<TimeGroup> Reference %s unknown", ref);
    sscanf(KmlText(KmlChild(time_group_elt, 0)), "%d", &min_limit);
    sscanf(KmlText(KmlChild(time_group_elt, 1)), "%d", &max_limit);
    if( min_limit < 0 )
      return KmlErrorMake(ke, KmlLineNum(time_group_elt),
	KmlColNum(time_group_elt), "<TimeGroup> Minimum is negative");
    if( min_limit > max_limit )
      return KmlErrorMake(ke, KmlLineNum(time_group_elt),
	KmlColNum(time_group_elt), "<TimeGroup> Minimum exceeds Maximum");
    ltg = KheLimitedTimeGroupMake(tg, min_limit, max_limit);
    KheTimeSpreadAddLimitedTimeGroup(*ts, ltg);
  }
  return true;
}


/*****************************************************************************/
/*                                                                           */
/*  bool KheSpreadEventsConstraintMakeFromKml(KML_ELT cons_elt,              */
/*    KHE_INSTANCE ins, KML_ERROR *ke)                                       */
/*                                                                           */
/*  Make a spread events constraint based on cons_elt and add it to ins.     */
/*                                                                           */
/*****************************************************************************/

bool KheSpreadEventsConstraintMakeFromKml(KML_ELT cons_elt,
  KHE_INSTANCE ins, KML_ERROR *ke)
{
  char *id, *name;  bool reqd;  int wt;  KHE_COST_FUNCTION cf;
  KML_ELT elt;  KHE_SPREAD_EVENTS_CONSTRAINT res;  KHE_TIME_SPREAD ts;

  /* verify cons_elt and get the common fields */
  if( !KmlCheck(cons_elt, "Id : $Name $Required #Weight "
      "$CostFunction AppliesTo TimeGroups", ke) )
    return false;
  if( !KheConstraintCheckKml(cons_elt, &id, &name, &reqd, &wt, &cf, ke) )
    return false;

  /* build the time spread */
  if( !GetTimeSpread(KmlChild(cons_elt, -1), ins, &ts, ke) )
    return false;

  /* build and insert the constraint object */
  if( !KheSpreadEventsConstraintMake(ins, id, name, reqd, wt, cf, ts, &res) )
    return KmlErrorMake(ke, KmlLineNum(cons_elt), KmlColNum(cons_elt),
      "<SpreadEventsConstraint> Id \"%s\" used previously", id);
  
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
/*  bool KheSpreadEventsConstraintWrite(KHE_SPREAD_EVENTS_CONSTRAINT c,      */
/*    KML_FILE kf)                                                           */
/*                                                                           */
/*  Write c to kf.                                                           */
/*                                                                           */
/*****************************************************************************/

bool KheSpreadEventsConstraintWrite(KHE_SPREAD_EVENTS_CONSTRAINT c,
  KML_FILE kf)
{
  KHE_EVENT_GROUP eg;  int i;  KHE_LIMITED_TIME_GROUP ltg;
  if( c->id == NULL )
    return false;
  KmlBegin(kf, "SpreadEventsConstraint");
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
  KmlBegin(kf, "TimeGroups");
  MArrayForEach(c->time_spread->limited_time_groups, &ltg, &i)
  {
    KmlBegin(kf, "TimeGroup");
    KmlAttribute(kf, "Reference", KheTimeGroupId(ltg->time_group));
    KmlEltPrintf(kf, "Minimum", "%d", ltg->minimum);
    KmlEltPrintf(kf, "Maximum", "%d", ltg->maximum);
    KmlEnd(kf, "TimeGroup");
  }
  KmlEnd(kf, "TimeGroups");
  KmlEnd(kf, "SpreadEventsConstraint");
  return true;
}


/*****************************************************************************/
/*                                                                           */
/*  Submodule "debug"                                                        */
/*                                                                           */
/*****************************************************************************/

/*****************************************************************************/
/*                                                                           */
/*  void KheSpreadEventsConstraintDebug(KHE_SPREAD_EVENTS_CONSTRAINT c,      */
/*    int verbosity, int indent, FILE *fp)                                   */
/*                                                                           */
/*  Debug print of c onto fp with the given verbosity and indent.            */
/*                                                                           */
/*****************************************************************************/

void KheSpreadEventsConstraintDebug(KHE_SPREAD_EVENTS_CONSTRAINT c,
  int verbosity, int indent, FILE *fp)
{
  KHE_LIMITED_TIME_GROUP ltg;  int i;
  if( verbosity >= 1 )
  {
    if( indent >= 0 )
      fprintf(fp, "%*s", indent, "");
    fprintf(fp, "[ %s %s%d:", c->id != NULL ? c->id : "-",
      c->required ? "r" : "", c->weight);
    MArrayForEach(c->time_spread->limited_time_groups, &ltg, &i)
      fprintf(fp, "%s %d-%d", i > 0 ? "," : "", ltg->minimum, ltg->maximum);
      /* ***
      fprintf(fp, "%s %s %d..%d", i > 0 ? "," : "",
	KheTimeGroupId(ltg->time_group) != NULL ?
	KheTimeGroupId(ltg->time_group) : "-", ltg->minimum, ltg->maximum);
      *** */
    fprintf(fp, " ]");
    if( indent >= 0 )
      fprintf(fp, "\n");
  }
}
