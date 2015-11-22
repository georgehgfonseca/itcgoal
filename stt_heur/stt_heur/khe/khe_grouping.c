
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
/*  FILE:         khe_grouping.c                                             */
/*  DESCRIPTION:  Helper functions for grouping demand monitors              */
/*                                                                           */
/*****************************************************************************/
#include "khe.h"
#include "m.h"

#define DEBUG1 0
#define DEBUG2 0
#define DEBUG3 0

typedef MARRAY(KHE_RESOURCE_TYPE) ARRAY_KHE_RESOURCE_TYPE;


/*****************************************************************************/
/*                                                                           */
/*  Submodule "assigned_to_node" stuff                                       */
/*                                                                           */
/*****************************************************************************/

/*****************************************************************************/
/*                                                                           */
/*  bool KheMeetAssignedToNode(KHE_MEET meet, KHE_NODE assigned_to_node)     */
/*                                                                           */
/*  Return true if meet is assigned, directly or indirectly, to a meet       */
/*  of assigned_to_node (which must be non-NULL).                            */
/*                                                                           */
/*****************************************************************************/

static bool KheMeetAssignedToNode(KHE_MEET meet, KHE_NODE assigned_to_node)
{
  do
  {
    if( KheMeetNode(meet) == assigned_to_node )
      return true;
    meet = KheMeetAsst(meet);
  } while( meet != NULL );
  return false;
}


/*****************************************************************************/
/*                                                                           */
/*  bool KheEventAssignedToNode(KHE_EVENT event, KHE_NODE assigned_to_node)  */
/*                                                                           */
/*  Return true if any of event's meets are assigned, directly or            */
/*  indirectly, to a meet of assigned_to_node (which must be non-NULL).      */
/*                                                                           */
/*****************************************************************************/

static bool KheEventAssignedToNode(KHE_EVENT event, KHE_NODE assigned_to_node)
{
  KHE_SOLN soln;  KHE_MEET meet;  int i;
  soln = KheNodeSoln(assigned_to_node);
  for( i = 0;  i < KheEventMeetCount(soln, event);  i++ )
  {
    meet = KheEventMeet(soln, event, i);
    if( KheMeetAssignedToNode(meet, assigned_to_node) )
      return true;
  }
  return false;
}


/*****************************************************************************/
/*                                                                           */
/*  bool KheTaskAssignedToNode(KHE_TASK task, KHE_NODE assigned_to_node)     */
/*                                                                           */
/*  Return true if task lies in a meet that is assigned, directly or         */
/*  indirectly, to a meet of assigned_to_node (which must be non-NULL).      */
/*                                                                           */
/*****************************************************************************/

static bool KheTaskAssignedToNode(KHE_TASK task, KHE_NODE assigned_to_node)
{
  KHE_MEET meet;
  meet = KheTaskMeet(task);
  return meet != NULL && KheMeetAssignedToNode(meet, assigned_to_node);
}


/*****************************************************************************/
/*                                                                           */
/*  bool KheEventResourceAssignedToNode(KHE_EVENT_RESOURCE er,               */
/*    KHE_NODE assigned_to_node)                                             */
/*                                                                           */
/*  Return true if any of er's tasks lie in meets that are assigned,         */
/*  directly or indirectly, to a meet of assigned_to_node (which must be     */
/*  non-NULL).                                                               */
/*                                                                           */
/*****************************************************************************/

static bool KheEventResourceAssignedToNode(KHE_EVENT_RESOURCE er,
  KHE_NODE assigned_to_node)
{
  KHE_SOLN soln;  KHE_TASK task;  int i;
  soln = KheNodeSoln(assigned_to_node);
  for( i = 0;  i < KheEventResourceTaskCount(soln, er);  i++ )
  {
    task = KheEventResourceTask(soln, er, i);
    if( KheTaskAssignedToNode(task, assigned_to_node) )
      return true;
  }
  return false;
}


/*****************************************************************************/
/*                                                                           */
/*  Submodule "event classes"                                                */
/*                                                                           */
/*****************************************************************************/

typedef struct khe_event_class_rec *KHE_EVENT_CLASS;

struct khe_event_class_rec {
  KHE_EVENT			event;			/* event of class    */
  KHE_GROUP_MONITOR		group_monitor;		/* monitor of class  */
  KHE_EVENT_CLASS		parent;			/* parent class      */
};

typedef MARRAY(KHE_EVENT_CLASS) ARRAY_KHE_EVENT_CLASS;


/*****************************************************************************/
/*                                                                           */
/*  KHE_EVENT_CLASS KheEventClassMake(KHE_EVENT e)                           */
/*                                                                           */
/*  Make a new timeval class containing just e.                              */
/*                                                                           */
/*****************************************************************************/

static KHE_EVENT_CLASS KheEventClassMake(KHE_EVENT e)
{
  KHE_EVENT_CLASS res;
  MMake(res);
  res->event = e;
  res->group_monitor = NULL;
  res->parent = NULL;
  return res;
}


/*****************************************************************************/
/*                                                                           */
/*  void KheEventClassFree(KHE_EVENT_CLASS c)                                */
/*                                                                           */
/*  Free c.                                                                  */
/*                                                                           */
/*****************************************************************************/

static void KheEventClassFree(KHE_EVENT_CLASS c)
{
  MFree(c);
}


/*****************************************************************************/
/*                                                                           */
/*  void KheEventClassMerge(KHE_EVENT_CLASS c1, KHE_EVENT_CLASS c2)          */
/*                                                                           */
/*  Merge these two classes.                                                 */
/*                                                                           */
/*****************************************************************************/

static void KheEventClassMerge(KHE_EVENT_CLASS c1, KHE_EVENT_CLASS c2)
{
  while( c1->parent != NULL )
    c1 = c1->parent;
  while( c2->parent != NULL )
    c2 = c2->parent;
  if( c1 != c2 )
    c1->parent = c2;
}


/*****************************************************************************/
/*                                                                           */
/* void KheEventClassArrayMake(KHE_SOLN soln, ARRAY_KHE_EVENT_CLASS *classes)*/
/*                                                                           */
/*  Make an array of event classes, one per instance event, but filled with  */
/*  NULL entries to begin with.                                              */
/*                                                                           */
/*****************************************************************************/

static void KheEventClassArrayMake(KHE_SOLN soln,
  ARRAY_KHE_EVENT_CLASS *classes)
{
  int count = KheInstanceEventCount(KheSolnInstance(soln));
  MArrayInit(*classes);
  MArrayFill(*classes, count, NULL);
}


/*****************************************************************************/
/*                                                                           */
/*  void KheEventClassArrayMerge(ARRAY_KHE_EVENT_CLASS *classes,             */
/*    int sub_tag, char *sub_tag_label, KHE_GROUP_MONITOR prnt)              */
/*                                                                           */
/*  Merge classes, give each root class a group monitor, and free *classes.  */
/*                                                                           */
/*****************************************************************************/

void KheEventClassArrayMerge(ARRAY_KHE_EVENT_CLASS *classes,
  int sub_tag, char *sub_tag_label, KHE_GROUP_MONITOR prnt)
{
  int i, j, k;  KHE_EVENT_CLASS c, c1, c2;  KHE_MONITOR m;  KHE_SOLN soln;
  KHE_EVENT e, e1, e2;  KHE_EVENT_GROUP eg;

  /* merge events that have a spread events or link events monitor in common */
  soln = KheMonitorSoln((KHE_MONITOR) prnt);
  MArrayForEach(*classes, &c, &i) if( c != NULL )
  {
    e = c->event;
    for( j = 0;  j < KheEventMonitorCount(soln, e);  j++ )
    {
      m = KheEventMonitor(soln, e, j);
      if( KheMonitorTag(m) == KHE_SPREAD_EVENTS_MONITOR_TAG ||
          KheMonitorTag(m) == KHE_LINK_EVENTS_MONITOR_TAG )
      {
	if( KheMonitorTag(m) == KHE_SPREAD_EVENTS_MONITOR_TAG )
	  eg = KheSpreadEventsMonitorEventGroup((KHE_SPREAD_EVENTS_MONITOR) m);
	else
	  eg = KheLinkEventsMonitorEventGroup((KHE_LINK_EVENTS_MONITOR) m);
	if( KheEventGroupEventCount(eg) >= 2 )
	{
	  e1 = KheEventGroupEvent(eg, 0);
	  c1 = MArrayGet(*classes, KheEventIndex(e1));
	  if( c1 != NULL )
	    for( k = 1;  k < KheEventGroupEventCount(eg);  k++ )
	    {
	      e2 = KheEventGroupEvent(eg, k);
	      c2 = MArrayGet(*classes, KheEventIndex(e2));
	      if( c2 != NULL )
		KheEventClassMerge(c1, c2);
	    }
	}
      }
    }
  }

  /* merge events that have an assignment in common */
  /* *** link events monitors will effectively do this for us
  for( i = 0;  i < KheSolnMeetCount(soln);  i++ )
  {
    meet1 = KheSolnMeet(soln, i);
    meet2 = KheMeetAsst(meet1);
    e1 = KheMeetEvent(meet1);
    if( e1 != NULL && meet2 != NULL && KheMeetEvent(meet2) != NULL )
    {
      e2 = KheMeetEvent(meet2);
      c1 = MArrayGet(classes, KheEventIndex(e1));
      c2 = MArrayGet(classes, KheEventIndex(e2));
      KheEventClassMerge(c1, c2);
    }
  }
  *** */

  /* build the group monitors, one in each root class */
  MArrayForEach(*classes, &c1, &i) if( c1 != NULL )
  {
    /* c1 is any class, c2 is its root class */
    c2 = c1;
    while( c2->parent != NULL )
      c2 = c2->parent;

    /* add c1's event's event monitors to c2's group monitor */
    for( j = 0;  j < KheEventMonitorCount(soln, c1->event);  j++ )
    {
      m = KheEventMonitor(soln, c1->event, j);
      if( c2->group_monitor == NULL )
      {
	c2->group_monitor =
	  KheGroupMonitorMake(soln, sub_tag, sub_tag_label);
	KheGroupMonitorAddChildMonitor(prnt, (KHE_MONITOR) c2->group_monitor);
      }
      if( KheMonitorParentMonitor(m) != c2->group_monitor )
	KheGroupMonitorAddChildMonitor(c2->group_monitor, m);
    }
  }

  /* free memory */
  MArrayForEach(*classes, &c1, &i)
    KheEventClassFree(c1);
  MArrayFree(*classes);
}


/*****************************************************************************/
/*                                                                           */
/* KHE_GROUP_MONITOR KheEventClassArrayOne(ARRAY_KHE_EVENT_CLASS *classes,   */
/*    int sub_tag, char *sub_tag_label, KHE_GROUP_MONITOR prnt)              */
/*                                                                           */
/*  Like KheEventClassArraySingle except make just one group monitor and     */
/*  return it (or return NULL if no monitors to group).                      */
/*                                                                           */
/*****************************************************************************/

static KHE_GROUP_MONITOR KheEventClassArrayOne(ARRAY_KHE_EVENT_CLASS *classes,
  int sub_tag, char *sub_tag_label, KHE_GROUP_MONITOR prnt, void *back)
{
  int i, j;  KHE_EVENT_CLASS c;  KHE_MONITOR m;  KHE_SOLN soln;
  KHE_EVENT e;  KHE_GROUP_MONITOR res;

  /* group the event monitors of the events of *classes under res */
  res = NULL;
  soln = KheMonitorSoln((KHE_MONITOR) prnt);
  MArrayForEach(*classes, &c, &i) if( c != NULL )
  {
    e = c->event;
    for( j = 0;  j < KheEventMonitorCount(soln, e);  j++ )
    {
      m = KheEventMonitor(soln, e, j);
      if( res == NULL )
      {
	res = KheGroupMonitorMake(soln, sub_tag, sub_tag_label);
	KheMonitorSetBack((KHE_MONITOR) res, back);
	if( prnt != NULL )
	  KheGroupMonitorAddChildMonitor(prnt, (KHE_MONITOR) res);
      }
      if( KheMonitorParentMonitor(m) != res )
	KheGroupMonitorAddChildMonitor(res, m);
    }
  }

  /* free memory and return */
  MArrayForEach(*classes, &c, &i)
    KheEventClassFree(c);
  MArrayFree(*classes);
  return res;
}


/*****************************************************************************/
/*                                                                           */
/*  void KheMeetAddEvents(KHE_MEET meet, ARRAY_KHE_EVENT_CLASS *classes)     */
/*                                                                           */
/*  Ensure that the events that meet and its non-node descendants are        */
/*  derived from have classes in *classes.                                   */
/*                                                                           */
/*****************************************************************************/

static void KheMeetAddEvents(KHE_MEET meet, ARRAY_KHE_EVENT_CLASS *classes)
{
  int index, i;  KHE_EVENT e;  KHE_MEET child_meet;

  /* do the job for meet itself */
  e = KheMeetEvent(meet);
  if( e != NULL )
  {
    index = KheEventIndex(e);
    if( MArrayGet(*classes, index) == NULL )
      MArrayPut(*classes, index, KheEventClassMake(e));
  }

  /* do the job for meet's non-node proper descendants */
  for( i = 0;  i < KheMeetAssignedToCount(meet);  i++ )
  {
    child_meet = KheMeetAssignedTo(meet, i);
    if( KheMeetNode(child_meet) != NULL )
      KheMeetAddEvents(child_meet, classes);
  }
}


/*****************************************************************************/
/*                                                                           */
/*  void KheNodeAddEvents(KHE_NODE node, KHE_NODE assigned_to_node,          */
/*    ARRAY_KHE_EVENT_CLASS *classes)                                        */
/*                                                                           */
/*  Ensure that each event derived from meets of node or its descendants,    */
/*  or from any meet assigned to those meets directly or indirectly, has     */
/*  a class in the *classes array.                                           */
/*                                                                           */
/*  If assigned_to_node is non-NULL, only events which have at least one     */
/*  meet assigned (directly or indirectly) to a meet of assigned_to_node     */
/*  are included.                                                            */
/*                                                                           */
/*****************************************************************************/

static void KheNodeAddEvents(KHE_NODE node, KHE_NODE assigned_to_node,
  ARRAY_KHE_EVENT_CLASS *classes)
{
  int i;  KHE_MEET meet;

  /* do the job for the meets of node */
  for( i = 0;  i < KheNodeMeetCount(node);  i++ )
  {
    meet = KheNodeMeet(node, i);
    if( assigned_to_node == NULL ||
	KheMeetAssignedToNode(meet, assigned_to_node) )
      KheMeetAddEvents(meet, classes);
  }

  /* do the job for the meets of the descendants of node */
  for( i = 0;  i < KheNodeChildCount(node);  i++ )
    KheNodeAddEvents(KheNodeChild(node, i), assigned_to_node, classes);
}


/*****************************************************************************/
/*                                                                           */
/*  Submodule "event group monitors"                                         */
/*                                                                           */
/*****************************************************************************/

/*****************************************************************************/
/*                                                                           */
/*  KHE_GROUP_MONITOR KheSolnGroupEventMonitors(KHE_SOLN soln,               */
/*    KHE_NODE assigned_to_node, int sub_tag, char *sub_tag_label,           */
/*    KHE_GROUP_MONITOR prnt)                                                */
/*                                                                           */
/*  Group the event monitors of soln under a single group monitor.           */
/*                                                                           */
/*  If assigned_to_node is non-NULL, only monitors of events which have at   */
/*  least one meet assigned (directly or indirectly) to a meet of            */
/*  assigned_to_node are included.                                           */
/*                                                                           */
/*  Implementation note.  Event classes are not needed here, but since       */
/*  useful helper functions exist which assume them, they are used.          */
/*                                                                           */
/*****************************************************************************/

KHE_GROUP_MONITOR KheSolnGroupEventMonitors(KHE_SOLN soln,
  KHE_NODE assigned_to_node, int sub_tag, char *sub_tag_label,
  KHE_GROUP_MONITOR prnt)
{
  ARRAY_KHE_EVENT_CLASS classes;  int i;  KHE_INSTANCE ins;  KHE_EVENT event;
  KheEventClassArrayMake(soln, &classes);
  ins = KheSolnInstance(soln);
  for( i = 0;  i < KheInstanceEventCount(ins);  i++ )
  {
    event = KheInstanceEvent(ins, i);
    if( assigned_to_node == NULL ||
	KheEventAssignedToNode(event, assigned_to_node) )
      MArrayPut(classes, i, KheEventClassMake(event));
  }
  return KheEventClassArrayOne(&classes, sub_tag, sub_tag_label, prnt,
    (void *) soln);
}


/*****************************************************************************/
/*                                                                           */
/*  KHE_GROUP_MONITOR KheNodeGroupEventMonitors(KHE_NODE node,               */
/*    KHE_NODE assigned_to_node, int sub_tag, char *sub_tag_label,           */
/*    KHE_GROUP_MONITOR prnt)                                                */
/*                                                                           */
/*  Group the event monitors of node and its descendants under a single      */
/*  group monitor.                                                           */
/*                                                                           */
/*  If assigned_to_node is non-NULL, only monitors of events which have at   */
/*  least one meet assigned (directly or indirectly) to a meet of            */
/*  assigned_to_node are included.                                           */
/*                                                                           */
/*  Implementation note.  Event classes are not needed here, but since       */
/*  useful helper functions exist which assume them, they are used.          */
/*                                                                           */
/*****************************************************************************/

KHE_GROUP_MONITOR KheNodeGroupEventMonitors(KHE_NODE node,
  KHE_NODE assigned_to_node, int sub_tag, char *sub_tag_label,
  KHE_GROUP_MONITOR prnt)
{
  ARRAY_KHE_EVENT_CLASS classes;
  KheEventClassArrayMake(KheNodeSoln(node), &classes);
  KheNodeAddEvents(node, assigned_to_node, &classes);
  return KheEventClassArrayOne(&classes, sub_tag, sub_tag_label, prnt,
    (void *) node);
}


/*****************************************************************************/
/*                                                                           */
/*  KHE_GROUP_MONITOR KheLayerGroupEventMonitors(KHE_LAYER layer,            */
/*    KHE_NODE assigned_to_node, int sub_tag, char *sub_tag_label,           */
/*    KHE_GROUP_MONITOR prnt)                                                */
/*                                                                           */
/*  Group the event monitors of layer and its descendants under a single     */
/*  group monitor.                                                           */
/*                                                                           */
/*  If assigned_to_node is non-NULL, only monitors of events which have at   */
/*  least one meet assigned (directly or indirectly) to a meet of            */
/*  assigned_to_node are included.                                           */
/*                                                                           */
/*  Implementation note.  Event classes are not needed here, but since       */
/*  useful helper functions exist which assume them, they are used.          */
/*                                                                           */
/*****************************************************************************/

KHE_GROUP_MONITOR KheLayerGroupEventMonitors(KHE_LAYER layer,
  KHE_NODE assigned_to_node, int sub_tag, char *sub_tag_label,
  KHE_GROUP_MONITOR prnt)
{
  ARRAY_KHE_EVENT_CLASS classes;  int i;
  KheEventClassArrayMake(KheLayerSoln(layer), &classes);
  for( i = 0;  i < KheLayerChildNodeCount(layer);  i++ )
    KheNodeAddEvents(KheLayerChildNode(layer, i), assigned_to_node, &classes);
  return KheEventClassArrayOne(&classes, sub_tag, sub_tag_label, prnt,
    (void *) layer);
}


/*****************************************************************************/
/*                                                                           */
/*  void KheSolnGroupEventMonitorsByClass(KHE_SOLN soln,                     */
/*    KHE_NODE assigned_to_node, int sub_tag, char *sub_tag_label,           */
/*    KHE_GROUP_MONITOR prnt)                                                */
/*                                                                           */
/*  Group the event monitors of soln by class.                               */
/*                                                                           */
/*  If assigned_to_node is non-NULL, only monitors of events which have at   */
/*  least one meet assigned (directly or indirectly) to a meet of            */
/*  assigned_to_node are included.                                           */
/*                                                                           */
/*****************************************************************************/

void KheSolnGroupEventMonitorsByClass(KHE_SOLN soln,
  KHE_NODE assigned_to_node, int sub_tag, char *sub_tag_label,
  KHE_GROUP_MONITOR prnt)
{
  ARRAY_KHE_EVENT_CLASS classes;  int i;  KHE_INSTANCE ins;  KHE_EVENT event;
  KheEventClassArrayMake(soln, &classes);
  ins = KheSolnInstance(soln);
  for( i = 0;  i < KheInstanceEventCount(ins);  i++ )
  {
    event = KheInstanceEvent(ins, i);
    if( assigned_to_node == NULL ||
	KheEventAssignedToNode(event, assigned_to_node) )
      MArrayPut(classes, i, KheEventClassMake(event));
  }
  /* ***
  for( i = 0;  i < KheInstanceEventCount(ins);  i++ )
    MArrayPut(classes, i, KheEventClassMake(KheInstanceEvent(ins, i)));
  *** */
  KheEventClassArrayMerge(&classes, sub_tag, sub_tag_label, prnt);
}


/*****************************************************************************/
/*                                                                           */
/*  void KheNodeGroupEventMonitorsByClass(KHE_NODE node,                     */
/*    KHE_NODE assigned_to_node, int sub_tag, char *sub_tag_label,           */
/*    KHE_GROUP_MONITOR prnt)                                                */
/*                                                                           */
/*  Group the event monitors of the events of the meets of node and          */
/*  its descendants by class.                                                */
/*                                                                           */
/*  If assigned_to_node is non-NULL, only monitors of events which have at   */
/*  least one meet assigned (directly or indirectly) to a meet of            */
/*  assigned_to_node are included.                                           */
/*                                                                           */
/*****************************************************************************/

void KheNodeGroupEventMonitorsByClass(KHE_NODE node,
  KHE_NODE assigned_to_node, int sub_tag, char *sub_tag_label,
  KHE_GROUP_MONITOR prnt)
{
  ARRAY_KHE_EVENT_CLASS classes;
  KheEventClassArrayMake(KheNodeSoln(node), &classes);
  KheNodeAddEvents(node, assigned_to_node, &classes);
  KheEventClassArrayMerge(&classes, sub_tag, sub_tag_label, prnt);
}


/*****************************************************************************/
/*                                                                           */
/*  void KheLayerGroupEventMonitorsByClass(KHE_LAYER layer,                  */
/*    KHE_NODE assigned_to_node, int sub_tag, char *sub_tag_label,           */
/*    KHE_GROUP_MONITOR prnt)                                                */
/*                                                                           */
/*  Group the event monitors of the events of the meets of the nodes of      */
/*  layer and their descendants by class.                                    */
/*                                                                           */
/*  If assigned_to_node is non-NULL, only monitors of events which have at   */
/*  least one meet assigned (directly or indirectly) to a meet of            */
/*  assigned_to_node are included.                                           */
/*                                                                           */
/*****************************************************************************/

void KheLayerGroupEventMonitorsByClass(KHE_LAYER layer,
  KHE_NODE assigned_to_node, int sub_tag, char *sub_tag_label,
  KHE_GROUP_MONITOR prnt)
{
  ARRAY_KHE_EVENT_CLASS classes;  int i;
  KheEventClassArrayMake(KheLayerSoln(layer), &classes);
  for( i = 0;  i < KheLayerChildNodeCount(layer);  i++ )
    KheNodeAddEvents(KheLayerChildNode(layer, i), assigned_to_node, &classes);
  KheEventClassArrayMerge(&classes, sub_tag, sub_tag_label, prnt);
}


/*****************************************************************************/
/*                                                                           */
/*  Submodule "event resource group monitors"                                */
/*                                                                           */
/*****************************************************************************/

typedef struct khe_event_resource_class_rec *KHE_EVENT_RESOURCE_CLASS;

struct khe_event_resource_class_rec {
  KHE_EVENT_RESOURCE		event_resource;		/* event of class    */
  KHE_GROUP_MONITOR		group_monitor;		/* monitor of class  */
  KHE_EVENT_RESOURCE_CLASS	parent;			/* parent class      */
};

typedef MARRAY(KHE_EVENT_RESOURCE_CLASS) ARRAY_KHE_EVENT_RESOURCE_CLASS;


/*****************************************************************************/
/*                                                                           */
/*  KHE_EVENT_RESOURCE_CLASS KheEventResourceClassMake(KHE_EVENT_RESOURCE er)*/
/*                                                                           */
/*  Make a new resource_val class containing just er.                        */
/*                                                                           */
/*****************************************************************************/

static KHE_EVENT_RESOURCE_CLASS KheEventResourceClassMake(KHE_EVENT_RESOURCE er)
{
  KHE_EVENT_RESOURCE_CLASS res;
  MMake(res);
  res->event_resource = er;
  res->group_monitor = NULL;
  res->parent = NULL;
  return res;
}


/*****************************************************************************/
/*                                                                           */
/*  void KheEventResourceClassFree(KHE_EVENT_RESOURCE_CLASS c)               */
/*                                                                           */
/*  Free c.                                                                  */
/*                                                                           */
/*****************************************************************************/

static void KheEventResourceClassFree(KHE_EVENT_RESOURCE_CLASS c)
{
  MFree(c);
}


/*****************************************************************************/
/*                                                                           */
/*  void KheEventResourceClassMerge(KHE_EVENT_RESOURCE_CLASS c1,             */
/*    KHE_EVENT_RESOURCE_CLASS c2)                                           */
/*                                                                           */
/*  Merge these two classes.                                                 */
/*                                                                           */
/*****************************************************************************/

static void KheEventResourceClassMerge(KHE_EVENT_RESOURCE_CLASS c1,
  KHE_EVENT_RESOURCE_CLASS c2)
{
  while( c1->parent != NULL )
    c1 = c1->parent;
  while( c2->parent != NULL )
    c2 = c2->parent;
  if( c1 != c2 )
    c1->parent = c2;
}


/*****************************************************************************/
/*                                                                           */
/*  void KheEventResourceClassArrayMake(KHE_SOLN soln,                       */
/*    ARRAY_KHE_EVENT_RESOURCE_CLASS *classes)                               */
/*                                                                           */
/*  Make an array of event resource classes, one per instance event          */
/*  resource, filled with NULL entries to begin with.                        */
/*                                                                           */
/*****************************************************************************/

static void KheEventResourceClassArrayMake(KHE_SOLN soln,
  ARRAY_KHE_EVENT_RESOURCE_CLASS *classes)
{
  int count = KheInstanceEventResourceCount(KheSolnInstance(soln));
  MArrayInit(*classes);
  MArrayFill(*classes, count, NULL);
}


/*****************************************************************************/
/*                                                                           */
/*  void KheEventResourceClassArrayMerge(                                    */
/*    ARRAY_KHE_EVENT_RESOURCE_CLASS *classes,                               */
/*    int sub_tag, char *sub_tag_label, KHE_GROUP_MONITOR prnt)              */
/*                                                                           */
/*  Merge classes, give each root class a group monitor, and free *classes.  */
/*                                                                           */
/*****************************************************************************/

static void KheEventResourceClassArrayMerge(
  ARRAY_KHE_EVENT_RESOURCE_CLASS *classes,
  int sub_tag, char *sub_tag_label, KHE_GROUP_MONITOR prnt)
{
  KHE_MONITOR m;  KHE_SOLN soln;
  KHE_EVENT_RESOURCE_CLASS c, c1, c2;  KHE_EVENT_RESOURCE er, er1, er2;
  int i, j, k, egi;  KHE_AVOID_SPLIT_ASSIGNMENTS_MONITOR asam;
  KHE_AVOID_SPLIT_ASSIGNMENTS_CONSTRAINT asac;

  /* merge classes that have an avoid split assignments monitor in common */
  soln = KheMonitorSoln((KHE_MONITOR) prnt);
  MArrayForEach(*classes, &c, &i) if( c != NULL )
  {
    er = c->event_resource;
    for( j = 0;  j < KheEventResourceMonitorCount(soln, er);  j++ )
    {
      m = KheEventResourceMonitor(soln, er, j);
      if( KheMonitorTag(m) == KHE_AVOID_SPLIT_ASSIGNMENTS_MONITOR_TAG )
      {
	asam = (KHE_AVOID_SPLIT_ASSIGNMENTS_MONITOR) m;
	asac = KheAvoidSplitAssignmentsMonitorConstraint(asam);
	egi = KheAvoidSplitAssignmentsMonitorEventGroupIndex(asam);
	if( KheAvoidSplitAssignmentsConstraintEventResourceCount(asac, egi) >= 2 )
	{
	  er1 = KheAvoidSplitAssignmentsConstraintEventResource(asac, egi, 0);
	  c1 = MArrayGet(*classes, KheEventResourceIndexInInstance(er1));
	  if( c1 != NULL )
	  for( k = 1;
	    k < KheAvoidSplitAssignmentsConstraintEventResourceCount(asac,egi);
	    k++ )
	  {
	    er2 = KheAvoidSplitAssignmentsConstraintEventResource(asac, egi, k);
	    c2 = MArrayGet(*classes, KheEventResourceIndexInInstance(er2));
	    if( c2 != NULL )
	      KheEventResourceClassMerge(c1, c2);
	  }
	}
      }
    }
  }

  /* build the group monitors, one in each root class */
  MArrayForEach(*classes, &c1, &i)  if( c1 != NULL )
  {
    /* c1 is any class, c2 is its root class */
    c2 = c1;
    while( c2->parent != NULL )
      c2 = c2->parent;

    /* add c1's event resource's resource value monitors to c2's group mon. */
    for( j=0; j < KheEventResourceMonitorCount(soln, c1->event_resource); j++ )
    {
      m = KheEventResourceMonitor(soln, c1->event_resource, j);
      if( c2->group_monitor == NULL )
      {
	c2->group_monitor = KheGroupMonitorMake(soln, sub_tag, sub_tag_label);
	KheGroupMonitorAddChildMonitor(prnt, (KHE_MONITOR) c2->group_monitor);
      }
      if( KheMonitorParentMonitor(m) != c2->group_monitor )
	KheGroupMonitorAddChildMonitor(c2->group_monitor, m);
    }
  }

  /* free memory */
  MArrayForEach(*classes, &c1, &i)
    KheEventResourceClassFree(c1);
  MArrayFree(*classes);
}


/*****************************************************************************/
/*                                                                           */
/*  void KheSolnGroupEventResourceMonitorsByClass(KHE_SOLN soln,             */
/*    KHE_NODE assigned_to_node, int sub_tag, char *sub_tag_label,           */
/*    KHE_GROUP_MONITOR prnt)                                                */
/*                                                                           */
/*  Group the event resource monitors of soln by class.                      */
/*                                                                           */
/*  If assigned_to_node is true, only monitors of event resources which      */
/*  have at least one task lying in a meet which is assigned directly or     */
/*  indirectly to assigned_to_node are included.                             */
/*                                                                           */
/*****************************************************************************/

void KheSolnGroupEventResourceMonitorsByClass(KHE_SOLN soln,
  KHE_NODE assigned_to_node, int sub_tag, char *sub_tag_label,
  KHE_GROUP_MONITOR prnt)
{
  ARRAY_KHE_EVENT_RESOURCE_CLASS classes;  int i;  KHE_INSTANCE ins;
  KHE_EVENT_RESOURCE er;
  KheEventResourceClassArrayMake(soln, &classes);
  ins = KheSolnInstance(soln);
  for( i = 0;  i < KheInstanceEventResourceCount(ins);  i++ )
  {
    er = KheInstanceEventResource(ins, i);
    if( assigned_to_node == NULL ||
	KheEventResourceAssignedToNode(er, assigned_to_node) )
      MArrayPut(classes, i, KheEventResourceClassMake(er));
  }
  KheEventResourceClassArrayMerge(&classes, sub_tag, sub_tag_label, prnt);
}


/*****************************************************************************/
/*                                                                           */
/*  void KheMeetAddEventResources(KHE_MEET meet,                             */
/*    ARRAY_KHE_EVENT_RESOURCE_CLASS *classes)                               */
/*                                                                           */
/*  Ensure that the event resources that the tasks of meet and its non-node  */
/*  descendants are derived from have classes in *classes.                   */
/*                                                                           */
/*****************************************************************************/

static void KheMeetAddEventResources(KHE_MEET meet,
  ARRAY_KHE_EVENT_RESOURCE_CLASS *classes)
{
  int index, i;  KHE_EVENT_RESOURCE er;  KHE_MEET child_meet;  KHE_TASK task;

  /* do the job for the tasks of meet itself */
  for( i = 0;  i < KheMeetTaskCount(meet);  i++ )
  {
    task = KheMeetTask(meet, i);
    er = KheTaskEventResource(task);
    if( er != NULL )
    {
      index = KheEventResourceIndexInInstance(er);
      if( MArrayGet(*classes, index) == NULL )
	MArrayPut(*classes, index, KheEventResourceClassMake(er));
    }
  }

  /* do the job for meet's non-node proper descendants */
  for( i = 0;  i < KheMeetAssignedToCount(meet);  i++ )
  {
    child_meet = KheMeetAssignedTo(meet, i);
    if( KheMeetNode(child_meet) != NULL )
      KheMeetAddEventResources(child_meet, classes);
  }
}


/*****************************************************************************/
/*                                                                           */
/*  void KheNodeAddEventResources(KHE_NODE node, KHE_NODE assigned_to_node,  */
/*    ARRAY_KHE_EVENT_RESOURCE_CLASS *classes)                               */
/*                                                                           */
/*  Ensure that each event resource derived from a task of a meet of node    */
/*  or its descendants, or from a task of a meet assigned to those meets     */
/*  directly or indirectly, has a class in the *classes array.               */
/*                                                                           */
/*  If assigned_to_node is non-NULL, only event resources which have at      */
/*  least one task lying in a meet assigned (directly or indirectly) to      */
/*  a meet of assigned_to_node are included.                                 */
/*                                                                           */
/*****************************************************************************/

static void KheNodeAddEventResources(KHE_NODE node, KHE_NODE assigned_to_node,
  ARRAY_KHE_EVENT_RESOURCE_CLASS *classes)
{
  int i;  KHE_MEET meet;

  /* do the job for the tasks of the meets of node */
  for( i = 0;  i < KheNodeMeetCount(node);  i++ )
  {
    meet = KheNodeMeet(node, i);
    if( assigned_to_node == NULL ||
	KheMeetAssignedToNode(meet, assigned_to_node) )
      KheMeetAddEventResources(KheNodeMeet(node, i), classes);
  }

  /* do the job for the tasks of the meets of the descendants of node */
  for( i = 0;  i < KheNodeChildCount(node);  i++ )
    KheNodeAddEventResources(KheNodeChild(node, i), assigned_to_node, classes);
}


/*****************************************************************************/
/*                                                                           */
/*  void KheNodeGroupEventResourceMonitorsByClass(KHE_NODE node,             */
/*    KHE_NODE assigned_to_node, int sub_tag, char *sub_tag_label,           */
/*    KHE_GROUP_MONITOR prnt)                                                */
/*                                                                           */
/*  Group by class the event resource monitors of the event resources        */
/*  derived from the tasks of the meets of node and its descendants and      */
/*  of the meets assigned to them directly or indirectly.                    */
/*                                                                           */
/*  If assigned_to_node is true, only monitors of event resources which      */
/*  have at least one task lying in a meet which is assigned directly or     */
/*  indirectly to assigned_to_node are included.                             */
/*                                                                           */
/*****************************************************************************/

void KheNodeGroupEventResourceMonitorsByClass(KHE_NODE node,
  KHE_NODE assigned_to_node, int sub_tag, char *sub_tag_label,
  KHE_GROUP_MONITOR prnt)
{
  ARRAY_KHE_EVENT_RESOURCE_CLASS classes;
  KheEventResourceClassArrayMake(KheNodeSoln(node), &classes);
  KheNodeAddEventResources(node, assigned_to_node, &classes);
  KheEventResourceClassArrayMerge(&classes, sub_tag, sub_tag_label, prnt);
}


/*****************************************************************************/
/*                                                                           */
/*  void KheLayerGroupEventResourceMonitorsByClass(KHE_LAYER layer,          */
/*    KHE_NODE assigned_to_node, int sub_tag, char *sub_tag_label,           */
/*    KHE_GROUP_MONITOR prnt)                                                */
/*                                                                           */
/*  Group by class the event resource monitors of the event resources        */
/*  derived from the tasks of the meets of the child nodes of layer and      */
/*  their descendants and the meets assigned to them directly or indirectly. */
/*                                                                           */
/*  If assigned_to_node is true, only monitors of event resources which      */
/*  have at least one task lying in a meet which is assigned directly or     */
/*  indirectly to assigned_to_node are included.                             */
/*                                                                           */
/*****************************************************************************/

void KheLayerGroupEventResourceMonitorsByClass(KHE_LAYER layer,
  KHE_NODE assigned_to_node, int sub_tag, char *sub_tag_label,
  KHE_GROUP_MONITOR prnt)
{
  ARRAY_KHE_EVENT_RESOURCE_CLASS classes;  int i;  KHE_NODE node;
  KheEventResourceClassArrayMake(KheLayerSoln(layer), &classes);
  for( i = 0;  i < KheLayerChildNodeCount(layer);  i++ )
  {
    node = KheLayerChildNode(layer, i);
    KheNodeAddEventResources(node, assigned_to_node, &classes);
  }
  KheEventResourceClassArrayMerge(&classes, sub_tag, sub_tag_label, prnt);
}


/*****************************************************************************/
/*                                                                           */
/*  void KheTaskAddEventResources(KHE_TASK task,                             */
/*    ARRAY_KHE_EVENT_RESOURCE_CLASS *classes)                               */
/*                                                                           */
/*  Ensure that classes has a class for the event resources that task        */
/*  and the tasks assigned to it, directly and indirectly, are derived from. */
/*                                                                           */
/*****************************************************************************/

static void KheTaskAddEventResources(KHE_TASK task,
  ARRAY_KHE_EVENT_RESOURCE_CLASS *classes)
{
  int index, i;  KHE_EVENT_RESOURCE er;

  /* do the job for task itself */
  er = KheTaskEventResource(task);
  if( er != NULL )
  {
    index = KheEventResourceIndexInInstance(er);
    if( MArrayGet(*classes, index) == NULL )
      MArrayPut(*classes, index, KheEventResourceClassMake(er));
  }

  /* do the job for the tasks assigned to task */
  for( i = 0;  i < KheTaskAssignedToCount(task);  i++ )
    KheTaskAddEventResources(KheTaskAssignedTo(task, i), classes);
}


/*****************************************************************************/
/*                                                                           */
/*  void KheTaskingGroupEventResourceMonitorsByClass(KHE_TASKING tasking,    */
/*    KHE_NODE assigned_to_node, int sub_tag, char *sub_tag_label,           */
/*    KHE_GROUP_MONITOR prnt)                                                */
/*                                                                           */
/*  Group by class the event resource monitors of the event resources        */
/*  derived from the tasks of tasking and the tasks assigned to them,        */
/*  directly or indirectly.                                                  */
/*                                                                           */
/*  If assigned_to_node is true, only monitors of event resources which      */
/*  have at least one task lying in a meet which is assigned directly or     */
/*  indirectly to assigned_to_node are included.                             */
/*                                                                           */
/*****************************************************************************/

void KheTaskingGroupEventResourceMonitorsByClass(KHE_TASKING tasking,
  KHE_NODE assigned_to_node, int sub_tag, char *sub_tag_label,
  KHE_GROUP_MONITOR prnt)
{
  ARRAY_KHE_EVENT_RESOURCE_CLASS classes;  int i;  KHE_TASK task;
  KheEventResourceClassArrayMake(KheTaskingSoln(tasking), &classes);
  for( i = 0;  i < KheTaskingTaskCount(tasking);  i++ )
  {
    task = KheTaskingTask(tasking, i);
    if( assigned_to_node == NULL ||
	KheTaskAssignedToNode(task, assigned_to_node) )
      KheTaskAddEventResources(task, &classes);
  }
  KheEventResourceClassArrayMerge(&classes, sub_tag, sub_tag_label, prnt);
}


/*****************************************************************************/
/*                                                                           */
/*  Submodule "resource group monitors"                                      */
/*                                                                           */
/*****************************************************************************/

/*****************************************************************************/
/*                                                                           */
/*  void KheMakeOneResourceMonitor(KHE_SOLN soln, KHE_RESOURCE r,            */
/*    int sub_tag, char *sub_tag_label, KHE_GROUP_MONITOR prnt)              */
/*                                                                           */
/*  Make one group monitor holding the resource monitors of r.               */
/*                                                                           */
/*****************************************************************************/

static void KheMakeOneResourceMonitor(KHE_SOLN soln, KHE_RESOURCE r,
  int sub_tag, char *sub_tag_label, KHE_GROUP_MONITOR prnt)
{
  KHE_GROUP_MONITOR gm;  int i;  KHE_MONITOR m;
  gm = NULL;
  for( i = 0;  i < KheResourceMonitorCount(soln, r);  i++ )
  {
    m = KheResourceMonitor(soln, r, i);
    if( gm == NULL )
    {
      gm = KheGroupMonitorMake(soln, sub_tag, sub_tag_label);
      KheGroupMonitorAddChildMonitor(prnt, (KHE_MONITOR) gm);
      KheMonitorSetBack((KHE_MONITOR) gm, r);
    }
    KheGroupMonitorAddChildMonitor(gm, m);
  }
}


/*****************************************************************************/
/*                                                                           */
/*  void KheSolnGroupResourceMonitorsByResource(KHE_SOLN soln,               */
/*    int sub_tag, char *sub_tag_label, KHE_GROUP_MONITOR prnt)              */
/*                                                                           */
/*  Group the monitors of the resources of soln's instance by resource.      */
/*                                                                           */
/*****************************************************************************/

void KheSolnGroupResourceMonitorsByResource(KHE_SOLN soln,
  int sub_tag, char *sub_tag_label, KHE_GROUP_MONITOR prnt)
{
  int i;  KHE_INSTANCE ins;  KHE_RESOURCE r;
  ins = KheSolnInstance(soln);
  for( i = 0;  i < KheInstanceResourceCount(ins);  i++ )
  {
    r = KheInstanceResource(ins, i);
    KheMakeOneResourceMonitor(soln, r, sub_tag, sub_tag_label, prnt);
  }
}


/*****************************************************************************/
/*                                                                           */
/*  void KheTaskingGroupResourceMonitorsByResource(KHE_TASKING tasking,      */
/*    int sub_tag, char *sub_tag_label, KHE_GROUP_MONITOR prnt)              */
/*                                                                           */
/*  Group the monitors of the resources assignable to the tasks of tasking   */
/*  by resource.                                                             */
/*                                                                           */
/*****************************************************************************/

void KheTaskingGroupResourceMonitorsByResource(KHE_TASKING tasking,
  int sub_tag, char *sub_tag_label, KHE_GROUP_MONITOR prnt)
{
  KHE_SOLN soln;  KHE_RESOURCE_TYPE rt;  int i;  KHE_RESOURCE r;
  soln = KheTaskingSoln(tasking);
  rt = KheTaskingResourceType(tasking);
  if( rt == NULL )
    KheSolnGroupResourceMonitorsByResource(soln, sub_tag, sub_tag_label, prnt);
  else
    for( i = 0;  i < KheResourceTypeResourceCount(rt);  i++ )
    {
      r = KheResourceTypeResource(rt, i);
      KheMakeOneResourceMonitor(soln, r, sub_tag, sub_tag_label, prnt);
    }
}


/*****************************************************************************/
/*                                                                           */
/*  Submodule "demand group monitors, helper functions"                      */
/*                                                                           */
/*****************************************************************************/

/*****************************************************************************/
/*                                                                           */
/*  void KheMeetAddDemandMonitors(KHE_MEET meet, bool include_preassigned,   */
/*    bool include_unpreassigned, int sub_tag, char *sub_tag_label,          */
/*    KHE_GROUP_MONITOR prnt, KHE_GROUP_MONITOR *gm)                         */
/*                                                                           */
/*  Add to *gm (which needs to be created the first time it is needed) the   */
/*  demand monitors of the tasks of meet, selected by include_preassigned    */
/*  and include_unpreassigned.                                               */
/*                                                                           */
/*****************************************************************************/

static void KheMeetAddDemandMonitors(KHE_MEET meet, bool include_preassigned,
  bool include_unpreassigned, int sub_tag, char *sub_tag_label,
  KHE_GROUP_MONITOR prnt, KHE_GROUP_MONITOR *gm)
{
  int i, j;  KHE_TASK task;  KHE_MONITOR m;  KHE_RESOURCE r;
  for( i = 0;  i < KheMeetTaskCount(meet);  i++ )
  {
    task = KheMeetTask(meet, i);
    if( KheTaskIsPreassigned(task, false, &r) ?
	include_preassigned : include_unpreassigned )
      for( j = 0;  j < KheTaskDemandMonitorCount(task);  j++ )
      {
	m = (KHE_MONITOR) KheTaskDemandMonitor(task, j);
	if( *gm == NULL )
	{
	  *gm = KheGroupMonitorMake(KheMeetSoln(meet), sub_tag, sub_tag_label);
	  if( prnt != NULL )
	    KheGroupMonitorAddChildMonitor(prnt, (KHE_MONITOR) *gm);
	}
	KheGroupMonitorAddChildMonitor(*gm, m);
      }
  }
}


/*****************************************************************************/
/*                                                                           */
/*  void KheMeetAndNonNodeDescendantsAddDemandMonitors(KHE_MEET meet,        */
/*    bool include_preassigned, bool include_unpreassigned, int sub_tag,     */
/*    char *sub_tag_label, KHE_GROUP_MONITOR prnt, KHE_GROUP_MONITOR *gm)    */
/*                                                                           */
/*  Add to *gm (which needs to be created the first time it is needed)       */
/*  the demand monitors of the tasks of meet and its non-node descendants,   */
/*  selected by include_preassigned and include_unpreassigned.               */
/*                                                                           */
/*****************************************************************************/

static void KheMeetAndNonNodeDescendantsAddDemandMonitors(KHE_MEET meet,
  bool include_preassigned, bool include_unpreassigned, int sub_tag,
  char *sub_tag_label, KHE_GROUP_MONITOR prnt, KHE_GROUP_MONITOR *gm)
{
  int i;  KHE_MEET child_meet;

  /* do the job for meet itself */
  KheMeetAddDemandMonitors(meet, include_preassigned,
    include_unpreassigned, sub_tag, sub_tag_label, prnt, gm);

  /* do the job for meet's non-node descendants */
  for( i = 0;  i < KheMeetAssignedToCount(meet);  i++ )
  {
    child_meet = KheMeetAssignedTo(meet, i);
    if( KheMeetNode(child_meet) == NULL )
      KheMeetAndNonNodeDescendantsAddDemandMonitors(child_meet,
	include_preassigned, include_unpreassigned, sub_tag,
	sub_tag_label, prnt, gm);
  }
}


/*****************************************************************************/
/*                                                                           */
/*  void KheMeetAndDescendantsAddDemandMonitors(KHE_MEET meet,               */
/*    bool include_preassigned, bool include_unpreassigned, int sub_tag,     */
/*    char *sub_tag_label, KHE_GROUP_MONITOR prnt, KHE_GROUP_MONITOR *gm)    */
/*                                                                           */
/*  Add to *gm (which needs to be created the first time it is needed)       */
/*  the demand monitors of the tasks of meet and all its descendants,        */
/*  selected by include_preassigned and include_unpreassigned.               */
/*                                                                           */
/*****************************************************************************/

static void KheMeetAndDescendantsAddDemandMonitors(KHE_MEET meet,
  bool include_preassigned, bool include_unpreassigned, int sub_tag,
  char *sub_tag_label, KHE_GROUP_MONITOR prnt, KHE_GROUP_MONITOR *gm)
{
  int i;

  /* do the job for meet itself */
  KheMeetAddDemandMonitors(meet, include_preassigned,
    include_unpreassigned, sub_tag, sub_tag_label, prnt, gm);

  /* do the job for meet's descendants */
  for( i = 0;  i < KheMeetAssignedToCount(meet);  i++ )
    KheMeetAndDescendantsAddDemandMonitors(KheMeetAssignedTo(meet, i),
      include_preassigned, include_unpreassigned, sub_tag,
      sub_tag_label, prnt, gm);
}


/*****************************************************************************/
/*                                                                           */
/*  void KheNodeAddDemandMonitors(KHE_NODE node, KHE_NODE assigned_to_node,  */
/*    bool include_preassigned, bool include_unpreassigned, int sub_tag,     */
/*    char *sub_tag_label, KHE_GROUP_MONITOR prnt, KHE_GROUP_MONITOR *gm)    */
/*                                                                           */
/*  Add to *gm (which needs to be created the first time it is needed)       */
/*  all the demand monitors of the tasks of the meets of node and its        */
/*  descendants and the meets assigned to them, directly or indirectly       */
/*  (selected by include_preassigned and include_unpreassigned).             */
/*                                                                           */
/*  If assigned_to_node is true, only demand monitors of tasks whose meets   */
/*  are assigned directly or indirectly to assigned_to_node are included.    */
/*                                                                           */
/*****************************************************************************/

static void KheNodeAddDemandMonitors(KHE_NODE node, KHE_NODE assigned_to_node,
  bool include_preassigned, bool include_unpreassigned, int sub_tag,
  char *sub_tag_label, KHE_GROUP_MONITOR prnt, KHE_GROUP_MONITOR *gm)
{
  KHE_MEET meet;  int i;

  /* do the job for the meets of node */
  for( i = 0;  i < KheNodeMeetCount(node);  i++ )
  {
    meet = KheNodeMeet(node, i);
    if( assigned_to_node == NULL ||
	KheMeetAssignedToNode(meet, assigned_to_node) )
      KheMeetAndNonNodeDescendantsAddDemandMonitors(meet, include_preassigned,
	include_unpreassigned, sub_tag, sub_tag_label, prnt, gm);
  }

  /* do the job for the meets of the descendants of node */
  for( i = 0;  i < KheNodeChildCount(node);  i++ )
    KheNodeAddDemandMonitors(KheNodeChild(node, i), assigned_to_node,
      include_preassigned, include_unpreassigned, sub_tag, sub_tag_label,
      prnt, gm);
}


/*****************************************************************************/
/*                                                                           */
/*  Submodule "demand group monitors, single monitor"                        */
/*                                                                           */
/*****************************************************************************/

/*****************************************************************************/
/*                                                                           */
/*  KHE_GROUP_MONITOR KheSolnGroupDemandMonitors(KHE_SOLN soln,              */
/*    KHE_NODE assigned_to_node, bool include_preassigned,                   */
/*    bool include_unpreassigned, bool include_workload,                     */
/*    int sub_tag, char *sub_tag_label, KHE_GROUP_MONITOR prnt)              */
/*                                                                           */
/*  Return a single group monitor holding all the demand monitors of         */
/*  soln (selected by include_preassigned, include_unpreassigned, and        */
/*  include_workload), or NULL if there are no selected monitors.            */
/*                                                                           */
/*  If assigned_to_node is true, only demand monitors of tasks whose meets   */
/*  are assigned directly or indirectly to assigned_to_node are included.    */
/*                                                                           */
/*****************************************************************************/

KHE_GROUP_MONITOR KheSolnGroupDemandMonitors(KHE_SOLN soln,
  KHE_NODE assigned_to_node, bool include_preassigned,
  bool include_unpreassigned, bool include_workload,
  int sub_tag, char *sub_tag_label, KHE_GROUP_MONITOR prnt)
{
  KHE_RESOURCE r;  KHE_INSTANCE ins;  KHE_GROUP_MONITOR gm;
  KHE_MONITOR m;  int i, j;  KHE_MEET meet;

  /* group the ordinary demand nodes, if requested */
  gm = NULL;
  if( include_preassigned || include_unpreassigned )
    for( i = 0;  i < KheSolnMeetCount(soln);  i++ )
    {
      meet = KheSolnMeet(soln, i);
      if( assigned_to_node == NULL ||
	  KheMeetAssignedToNode(meet, assigned_to_node) )
	KheMeetAddDemandMonitors(meet, include_preassigned,
	  include_unpreassigned, sub_tag, sub_tag_label, prnt, &gm);
    }

  /* group the workload demand nodes, if requested */
  if( include_workload )
  {
    ins = KheSolnInstance(soln);
    for( i = 0;  i < KheInstanceResourceCount(ins);  i++ )
    {
      r = KheInstanceResource(ins, i);
      for( j = 0;  j < KheResourceMonitorCount(soln, r);  j++ )
      {
	m = KheResourceMonitor(soln, r, j);
	if( KheMonitorTag(m) == KHE_WORKLOAD_DEMAND_MONITOR_TAG )
	{
	  if( gm == NULL )
	  {
	    gm = KheGroupMonitorMake(soln, sub_tag, sub_tag_label);
	    KheGroupMonitorAddChildMonitor(prnt, (KHE_MONITOR) gm);
	  }
	  KheGroupMonitorAddChildMonitor(gm, m);
	}
      }
    }
  }
  if( gm != NULL )
    KheMonitorSetBack((KHE_MONITOR) gm, (void *) soln);
  return gm;
}


/*****************************************************************************/
/*                                                                           */
/*  KHE_GROUP_MONITOR KheNodeGroupDemandMonitors(KHE_NODE node,              */
/*    KHE_NODE assigned_to_node, bool include_preassigned,                   */
/*    bool include_unpreassigned, int sub_tag, char *sub_tag_label,          */
/*    KHE_GROUP_MONITOR prnt)                                                */
/*                                                                           */
/*  Return a single group monitor holding all the demand monitors of the     */
/*  tasks of the meets of node and its descendants and the meets assigned    */
/*  to them, directly or indirectly (selected by include_preassigned and     */
/*  include_unpreassigned), or NULL if there are no selected monitors.       */
/*                                                                           */
/*  If assigned_to_node is true, only demand monitors of tasks whose meets   */
/*  are assigned directly or indirectly to assigned_to_node are included.    */
/*                                                                           */
/*****************************************************************************/

KHE_GROUP_MONITOR KheNodeGroupDemandMonitors(KHE_NODE node,
  KHE_NODE assigned_to_node, bool include_preassigned,
  bool include_unpreassigned, int sub_tag, char *sub_tag_label,
  KHE_GROUP_MONITOR prnt)
{
  KHE_GROUP_MONITOR gm;
  gm = NULL;
  KheNodeAddDemandMonitors(node, assigned_to_node, include_preassigned,
    include_unpreassigned, sub_tag, sub_tag_label, prnt, &gm);
  if( gm != NULL )
    KheMonitorSetBack((KHE_MONITOR) gm, (void *) node);
  return gm;
}


/*****************************************************************************/
/*                                                                           */
/*  KHE_GROUP_MONITOR KheLayerGroupDemandMonitors(KHE_LAYER layer,           */
/*    KHE_NODE assigned_to_node, bool include_preassigned,                   */
/*    bool include_unpreassigned, int sub_tag, char *sub_tag_label,          */
/*    KHE_GROUP_MONITOR prnt)                                                */
/*                                                                           */
/*  Return a single group monitor holding all the demand monitors of the     */
/*  tasks of the meets of the nodes of layer and their descendants and the   */
/*  meets assigned to them, directly or indirectly (selected by              */
/*  include_preassigned and include_unpreassigned), or NULL if there are     */
/*  no selected monitors.                                                    */
/*                                                                           */
/*  If assigned_to_node is true, only demand monitors of tasks whose meets   */
/*  are assigned directly or indirectly to assigned_to_node are included.    */
/*                                                                           */
/*****************************************************************************/

KHE_GROUP_MONITOR KheLayerGroupDemandMonitors(KHE_LAYER layer,
  KHE_NODE assigned_to_node, bool include_preassigned,
  bool include_unpreassigned, int sub_tag, char *sub_tag_label,
  KHE_GROUP_MONITOR prnt)
{
  KHE_GROUP_MONITOR gm;  int i;
  gm = NULL;
  for( i = 0;  i < KheLayerChildNodeCount(layer);  i++ )
    KheNodeAddDemandMonitors(KheLayerChildNode(layer, i), assigned_to_node,
      include_preassigned, include_unpreassigned, sub_tag, sub_tag_label,
      prnt, &gm);
  if( gm != NULL )
    KheMonitorSetBack((KHE_MONITOR) gm, (void *) layer);
  return gm;
}


/*****************************************************************************/
/*                                                                           */
/*  KHE_GROUP_MONITOR KheMeetGroupDemandMonitors(KHE_MEET meet,              */
/*    KHE_NODE assigned_to_node, bool include_preassigned,                   */
/*    bool include_unpreassigned, int sub_tag, char *sub_tag_label,          */
/*    KHE_GROUP_MONITOR prnt)                                                */
/*                                                                           */
/*  Return a single group monitor holding all the demand monitors of the     */
/*  tasks of meet and the meets assigned to its, directly or indirectly      */
/*  (selected by include_preassigned and include_unpreassigned), or NULL     */
/*  if there are no selected monitors.                                       */
/*                                                                           */
/*  If assigned_to_node is true, only demand monitors of tasks whose meets   */
/*  are assigned directly or indirectly to assigned_to_node are included.    */
/*                                                                           */
/*  Implementation note.  KheMeetAndNonNodeDescendantsAddDemandMonitors is   */
/*  no use here, because the specification calls for all descendants, not    */
/*  all non-node descendants.                                                */
/*                                                                           */
/*****************************************************************************/

KHE_GROUP_MONITOR KheMeetGroupDemandMonitors(KHE_MEET meet,
  KHE_NODE assigned_to_node, bool include_preassigned,
  bool include_unpreassigned, int sub_tag, char *sub_tag_label,
  KHE_GROUP_MONITOR prnt)
{
  KHE_GROUP_MONITOR gm;
  gm = NULL;
  if( assigned_to_node == NULL ||
      KheMeetAssignedToNode(meet, assigned_to_node) )
    KheMeetAndDescendantsAddDemandMonitors(meet, include_preassigned,
      include_unpreassigned, sub_tag, sub_tag_label, prnt, &gm);
  if( gm != NULL )
    KheMonitorSetBack((KHE_MONITOR) gm, (void *) meet);
  return gm;
}


/*****************************************************************************/
/*                                                                           */
/*  Submodule "demand group monitors, grouped by leader meet"                */
/*                                                                           */
/*****************************************************************************/

/*****************************************************************************/
/*                                                                           */
/*  void KheNodeGroupDemandMonitorsByLeaderMeet(KHE_NODE node,               */
/*    KHE_NODE assigned_to_node, bool include_preassigned,                   */
/*    bool include_unpreassigned, int sub_tag, char *sub_tag_label,          */
/*    KHE_GROUP_MONITOR prnt)                                                */
/*                                                                           */
/*  Make one group monitor for each non-cycle meet lying directly within     */
/*  node or any of its descendant nodes (each leader meet), containing the   */
/*  demand monitors of the tasks of that meet and of all meets assigned to   */
/*  that meet, directly or indirectly, that do not lie in nodes themselves.  */
/*                                                                           */
/*  If assigned_to_node is true, only demand monitors of tasks whose meets   */
/*  are assigned directly or indirectly to assigned_to_node are included.    */
/*                                                                           */
/*****************************************************************************/

void KheNodeGroupDemandMonitorsByLeaderMeet(KHE_NODE node,
  KHE_NODE assigned_to_node, bool include_preassigned,
  bool include_unpreassigned, int sub_tag, char *sub_tag_label,
  KHE_GROUP_MONITOR prnt)
{
  int i;  KHE_MEET meet;  KHE_GROUP_MONITOR gm;

  /* do the job for the meets of node itself */
  for( i = 0;  i < KheNodeMeetCount(node);  i++ )
  {
    meet = KheNodeMeet(node, i);
    gm = NULL;
    if( assigned_to_node == NULL ||
	KheMeetAssignedToNode(meet, assigned_to_node) )
      KheMeetAndNonNodeDescendantsAddDemandMonitors(meet, include_preassigned,
	include_unpreassigned, sub_tag, sub_tag_label, prnt, &gm);
    if( gm != NULL )
      KheMonitorSetBack((KHE_MONITOR) gm, (void *) meet);
  }

  /* do the job for the meets of node's descendants */
  for( i = 0;  i < KheNodeChildCount(node);  i++ )
    KheNodeGroupDemandMonitorsByLeaderMeet(KheNodeChild(node, i),
      assigned_to_node, include_preassigned, include_unpreassigned,
      sub_tag, sub_tag_label, prnt);
}


/*****************************************************************************/
/*                                                                           */
/*  void KheLayerGroupDemandMonitorsByLeaderMeet(KHE_LAYER layer,            */
/*    KHE_NODE assigned_to_node, bool include_preassigned,                   */
/*    bool include_unpreassigned, int sub_tag, char *sub_tag_label,          */
/*    KHE_GROUP_MONITOR prnt)                                                */
/*                                                                           */
/*  Make one group monitor for each non-cycle meet lying directly within     */
/*  the child nodes of layer or any of their descendant nodes (each leader   */
/*  meet), containing the demand monitors of the tasks of that meet and of   */
/*  all meets assigned to that meet, directly or indirectly, that do not     */
/*  lie in nodes themselves.                                                 */
/*                                                                           */
/*  If assigned_to_node is true, only demand monitors of tasks whose meets   */
/*  are assigned directly or indirectly to assigned_to_node are included.    */
/*                                                                           */
/*****************************************************************************/

void KheLayerGroupDemandMonitorsByLeaderMeet(KHE_LAYER layer,
  KHE_NODE assigned_to_node, bool include_preassigned,
  bool include_unpreassigned, int sub_tag, char *sub_tag_label,
  KHE_GROUP_MONITOR prnt)
{
  int i;
  for( i = 0;  i < KheLayerChildNodeCount(layer);  i++ )
    KheNodeGroupDemandMonitorsByLeaderMeet(KheLayerChildNode(layer, i),
      assigned_to_node, include_preassigned, include_unpreassigned,
      sub_tag, sub_tag_label, prnt);
}


/*****************************************************************************/
/*                                                                           */
/*  Submodule "demand group monitors, grouped by node"                       */
/*                                                                           */
/*****************************************************************************/

/*****************************************************************************/
/*                                                                           */
/*  void KheNodeGroupDemandMonitorsByChildNode(KHE_NODE parent_node,         */
/*    KHE_NODE assigned_to_node, bool include_preassigned,                   */
/*    bool include_unpreassigned, int sub_tag, char *sub_tag_label,          */
/*    KHE_GROUP_MONITOR prnt)                                                */
/*                                                                           */
/*  Make one group monitor for each child node of parent_node, containing    */
/*  the ordinary demand monitors of the meets lying in the child node and    */
/*  its descendants, and of meets assigned to those meets.  Back pointers    */
/*  are set to the child nodes.                                              */
/*                                                                           */
/*  If assigned_to_node is true, only demand monitors of tasks whose meets   */
/*  are assigned directly or indirectly to assigned_to_node are included.    */
/*                                                                           */
/*****************************************************************************/

void KheNodeGroupDemandMonitorsByChildNode(KHE_NODE parent_node,
  KHE_NODE assigned_to_node, bool include_preassigned,
  bool include_unpreassigned, int sub_tag, char *sub_tag_label,
  KHE_GROUP_MONITOR prnt)
{
  int i;
  for( i = 0;  i < KheNodeChildCount(parent_node);  i++ )
    KheNodeGroupDemandMonitors(KheNodeChild(parent_node, i), assigned_to_node,
      include_preassigned, include_unpreassigned, sub_tag,sub_tag_label,prnt);
}


/*****************************************************************************/
/*                                                                           */
/*  void KheLayerGroupDemandMonitorsByChildNode(KHE_LAYER layer,             */
/*    KHE_NODE assigned_to_node, bool include_preassigned,                   */
/*    bool include_unpreassigned, int sub_tag, char *sub_tag_label,          */
/*    KHE_GROUP_MONITOR prnt)                                                */
/*                                                                           */
/*  Make one group monitor for each child node of layer, containing the      */
/*  ordinary demand monitors of the meets lying in the child node node       */
/*  and its descendants, and of meets assigned to those meets.  Back         */
/*  pointers are set to the child nodes.                                     */
/*                                                                           */
/*  If assigned_to_node is true, only demand monitors of tasks whose meets   */
/*  are assigned directly or indirectly to assigned_to_node are included.    */
/*                                                                           */
/*****************************************************************************/

void KheLayerGroupDemandMonitorsByChildNode(KHE_LAYER layer,
  KHE_NODE assigned_to_node, bool include_preassigned,
  bool include_unpreassigned, int sub_tag, char *sub_tag_label,
  KHE_GROUP_MONITOR prnt)
{
  int i;
  for( i = 0;  i < KheLayerChildNodeCount(layer);  i++ )
    KheNodeGroupDemandMonitors(KheLayerChildNode(layer, i), assigned_to_node,
      include_preassigned, include_unpreassigned, sub_tag,sub_tag_label,prnt);
}


/*****************************************************************************/
/*                                                                           */
/*  Submodule "higher level grouping and ungrouping"                         */
/*                                                                           */
/*****************************************************************************/

/*****************************************************************************/
/*                                                                           */
/*  KHE_GROUP_MONITOR KheNodeGroupMonitorsForMeetRepair(KHE_NODE parent_node,*/
/*    KHE_NODE assigned_to_node, int event_sub_tag,char *event_sub_tag_label,*/
/*    int preassigned_demand_sub_tag, char *preassigned_demand_sub_tag_label,*/
/*    int unpreassigned_demand_sub_tag,                                      */
/*    char *unpreassigned_demand_sub_tag_label,                              */
/*    int sub_tag, char *sub_tag_label, KHE_GROUP_MONITOR prnt)              */
/*                                                                           */
/*  Build a group monitor suitable for repairing the time assignments of     */
/*  node.                                                                    */
/*                                                                           */
/*  If assigned_to_node is true, only monitors that monitor meets which      */
/*  are assigned directly or indirectly to assigned_to_node are included.    */
/*                                                                           */
/*****************************************************************************/

KHE_GROUP_MONITOR KheNodeGroupMonitorsForMeetRepair(KHE_NODE parent_node,
  KHE_NODE assigned_to_node, int event_sub_tag, char *event_sub_tag_label,
  int preassigned_demand_sub_tag, char *preassigned_demand_sub_tag_label,
  int unpreassigned_demand_sub_tag, char *unpreassigned_demand_sub_tag_label,
  int sub_tag, char *sub_tag_label, KHE_GROUP_MONITOR prnt)
{
  KHE_GROUP_MONITOR res;
  if( DEBUG2 )
    fprintf(stderr, "[ KheNodeGroupMonitorsForMeetRepair(parent_node, ...)\n");
  res = KheGroupMonitorMake(KheNodeSoln(parent_node), sub_tag, sub_tag_label);
  if( prnt != NULL )
    KheGroupMonitorAddChildMonitor(prnt, (KHE_MONITOR) res);
  KheNodeGroupEventMonitorsByClass(parent_node, assigned_to_node,
    event_sub_tag, event_sub_tag_label, res);
  KheNodeGroupDemandMonitorsByLeaderMeet(parent_node, assigned_to_node, true,
    false, preassigned_demand_sub_tag, preassigned_demand_sub_tag_label, res);
  KheNodeGroupDemandMonitorsByLeaderMeet(parent_node, assigned_to_node, false,
    true, unpreassigned_demand_sub_tag, unpreassigned_demand_sub_tag_label,res);
  if( DEBUG2 )
  {
    KheGroupMonitorDefectDebug(res, 2, 2, stderr);
    fprintf(stderr, "] KheNodeGroupMonitorsForMeetRepair\n");
  }
  return res;
}


/*****************************************************************************/
/*                                                                           */
/*  KHE_GROUP_MONITOR KheLayerGroupMonitorsForMeetRepair(KHE_LAYER layer,    */
/*    KHE_NODE assigned_to_node, int event_sub_tag,char *event_sub_tag_label,*/
/*    int preassigned_demand_sub_tag, char *preassigned_demand_sub_tag_label,*/
/*    int unpreassigned_demand_sub_tag,                                      */
/*    char *unpreassigned_demand_sub_tag_label,                              */
/*    int sub_tag, char *sub_tag_label)                                      */
/*                                                                           */
/*  Build a group monitor suitable for repairing the time assignments of     */
/*  layer.                                                                   */
/*                                                                           */
/*  If assigned_to_node is true, only monitors that monitor meets which      */
/*  are assigned directly or indirectly to assigned_to_node are included.    */
/*                                                                           */
/*****************************************************************************/

KHE_GROUP_MONITOR KheLayerGroupMonitorsForMeetRepair(KHE_LAYER layer,
  KHE_NODE assigned_to_node, int event_sub_tag, char *event_sub_tag_label,
  int preassigned_demand_sub_tag, char *preassigned_demand_sub_tag_label,
  int unpreassigned_demand_sub_tag, char *unpreassigned_demand_sub_tag_label,
  int sub_tag, char *sub_tag_label, KHE_GROUP_MONITOR prnt)
{
  KHE_GROUP_MONITOR res;
  if( DEBUG2 )
    fprintf(stderr, "[ KheLayerGroupMonitorsForMeetRepair(layer, ...)\n");
  res = KheGroupMonitorMake(KheLayerSoln(layer), sub_tag, sub_tag_label);
  if( prnt != NULL )
    KheGroupMonitorAddChildMonitor(prnt, (KHE_MONITOR) res);
  KheLayerGroupEventMonitorsByClass(layer, assigned_to_node, event_sub_tag,
    event_sub_tag_label, res);
  KheLayerGroupDemandMonitorsByLeaderMeet(layer, assigned_to_node, true, false,
    preassigned_demand_sub_tag, preassigned_demand_sub_tag_label, res);
  KheLayerGroupDemandMonitorsByLeaderMeet(layer, assigned_to_node, false, true,
    unpreassigned_demand_sub_tag, unpreassigned_demand_sub_tag_label, res);
  if( DEBUG2 )
  {
    KheGroupMonitorDefectDebug(res, 2, 2, stderr);
    fprintf(stderr, "] KheLayerGroupMonitorsForMeetRepair\n");
  }
  return res;
}


/*****************************************************************************/
/*                                                                           */
/*  KHE_GROUP_MONITOR KheNodeGroupMonitorsForNodeRepair(KHE_NODE parent_node,*/
/*    KHE_NODE assigned_to_node, int unpreassigned_demand_sub_tag,           */
/*    char *unpreassigned_demand_sub_tag_label, int sub_tag,                 */
/*    char *sub_tag_label, KHE_GROUP_MONITOR prnt)                           */
/*                                                                           */
/*  Build a group monitor suitable for repairing the time assignments        */
/*  of parent_node's child nodes by node swaps.                              */
/*                                                                           */
/*  If assigned_to_node is true, only monitors that monitor meets which      */
/*  are assigned directly or indirectly to assigned_to_node are included.    */
/*                                                                           */
/*****************************************************************************/

KHE_GROUP_MONITOR KheNodeGroupMonitorsForNodeRepair(KHE_NODE parent_node,
  KHE_NODE assigned_to_node, int unpreassigned_demand_sub_tag,
  char *unpreassigned_demand_sub_tag_label, int sub_tag,
  char *sub_tag_label, KHE_GROUP_MONITOR prnt)
{
  KHE_GROUP_MONITOR res;
  if( DEBUG2 )
    fprintf(stderr, "[ KheNodeGroupMonitorsForNodeRepair(parent_node, ...)\n");
  res = KheGroupMonitorMake(KheNodeSoln(parent_node), sub_tag, sub_tag_label);
  if( prnt != NULL )
    KheGroupMonitorAddChildMonitor(prnt, (KHE_MONITOR) res);
  KheNodeGroupDemandMonitorsByChildNode(parent_node, assigned_to_node, false,
    true, unpreassigned_demand_sub_tag, unpreassigned_demand_sub_tag_label,res);
  if( DEBUG2 )
  {
    KheGroupMonitorDefectDebug(res, 2, 2, stderr);
    fprintf(stderr, "] KheNodeGroupMonitorsForNodeRepair\n");
  }
  return res;
}


/*****************************************************************************/
/*                                                                           */
/*  KHE_GROUP_MONITOR KheSolnGroupMonitorsForTaskRepair(                     */
/*    KHE_SOLN soln, KHE_NODE assigned_to_node,                              */
/*    int event_resource_sub_tag, char *event_resource_sub_tag_label,        */
/*    int resource_sub_tag, char *resource_sub_tag_label,                    */
/*    int sub_tag, char *sub_tag_label, KHE_GROUP_MONITOR prnt)              */
/*                                                                           */
/*  Build a group monitor suitable for repairing the resource assignments    */
/*  of soln.                                                                 */
/*                                                                           */
/*  If assigned_to_node is true, only monitors that monitor meets which      */
/*  are assigned directly or indirectly to assigned_to_node are included.    */
/*                                                                           */
/*****************************************************************************/

KHE_GROUP_MONITOR KheSolnGroupMonitorsForTaskRepair(
  KHE_SOLN soln, KHE_NODE assigned_to_node,
  int event_resource_sub_tag, char *event_resource_sub_tag_label,
  int resource_sub_tag, char *resource_sub_tag_label,
  int sub_tag, char *sub_tag_label, KHE_GROUP_MONITOR prnt)
{
  KHE_GROUP_MONITOR res;
  res = KheGroupMonitorMake(soln, sub_tag, sub_tag_label);
  if( prnt != NULL )
    KheGroupMonitorAddChildMonitor(prnt, (KHE_MONITOR) res);
  KheSolnGroupEventResourceMonitorsByClass(soln, assigned_to_node,
    event_resource_sub_tag, event_resource_sub_tag_label, res);
  KheSolnGroupResourceMonitorsByResource(soln,
    resource_sub_tag, resource_sub_tag_label, res);
  return res;
}


/*****************************************************************************/
/*                                                                           */
/*  KHE_GROUP_MONITOR KheTaskingGroupMonitorsForTaskRepair(                  */
/*    KHE_TASKING tasking, KHE_NODE assigned_to_node,                        */
/*    int event_resource_sub_tag, char *event_resource_sub_tag_label,        */
/*    int resource_sub_tag, char *resource_sub_tag_label,                    */
/*    int sub_tag, char *sub_tag_label, KHE_GROUP_MONITOR prnt)              */
/*                                                                           */
/*  Build a group monitor suitable for repairing the resource assignments    */
/*  of tasking.                                                              */
/*                                                                           */
/*  If assigned_to_node is true, only monitors that monitor meets which      */
/*  are assigned directly or indirectly to assigned_to_node are included.    */
/*                                                                           */
/*****************************************************************************/

KHE_GROUP_MONITOR KheTaskingGroupMonitorsForTaskRepair(
  KHE_TASKING tasking, KHE_NODE assigned_to_node,
  int event_resource_sub_tag, char *event_resource_sub_tag_label,
  int resource_sub_tag, char *resource_sub_tag_label,
  int sub_tag, char *sub_tag_label, KHE_GROUP_MONITOR prnt)
{
  KHE_GROUP_MONITOR res;
  res = KheGroupMonitorMake(KheTaskingSoln(tasking), sub_tag, sub_tag_label);
  if( prnt != NULL )
    KheGroupMonitorAddChildMonitor(prnt, (KHE_MONITOR) res);
  KheTaskingGroupEventResourceMonitorsByClass(tasking, assigned_to_node,
    event_resource_sub_tag, event_resource_sub_tag_label, res);
  KheTaskingGroupResourceMonitorsByResource(tasking,
    resource_sub_tag, resource_sub_tag_label, res);
  return res;
}


/*****************************************************************************/
/*                                                                           */
/*  void KheReLink(KHE_GROUP_MONITOR gm, KHE_GROUP_MONITOR prnt)             */
/*                                                                           */
/*  Ensure that all the group monitors which are descendants of gm           */
/*  (including gm itself) are deleted, and that all the non-group monitors   */
/*  which are descendants of gm are moved to be children of prnt, if any.    */
/*                                                                           */
/*****************************************************************************/

static void KheReLink(KHE_GROUP_MONITOR gm, KHE_GROUP_MONITOR prnt)
{
  KHE_MONITOR m;

  /* delete the group monitors below gm and relink the non-group ones */
  while( KheGroupMonitorChildMonitorCount(gm) > 0 )
  {
    m = KheGroupMonitorChildMonitor(gm, KheGroupMonitorChildMonitorCount(gm)-1);
    if( KheMonitorTag(m) == KHE_GROUP_MONITOR_TAG )
      KheReLink((KHE_GROUP_MONITOR) m, prnt);
    else if( prnt != NULL )
      KheGroupMonitorAddChildMonitor(prnt, m);
    else
      KheGroupMonitorDeleteChildMonitor(gm, m);
  }

  /* delete gm */
  KheGroupMonitorDelete(gm);
}


/*****************************************************************************/
/*                                                                           */
/*  void KheUnGroupMonitors(KHE_GROUP_MONITOR gm)                            */
/*                                                                           */
/*  Delete gm (which may not be the soln object) and all group monitors      */
/*  which are proper descendants of gm, while ensuring that the non-group    */
/*  monitors which are descendants of gm become children of gm's parent,     */
/*  if it has one.                                                           */
/*                                                                           */
/*****************************************************************************/

void KheUnGroupMonitors(KHE_GROUP_MONITOR gm)
{
  MAssert(gm != (KHE_GROUP_MONITOR) KheMonitorSoln((KHE_MONITOR) gm),
    "KheUnGroupMonitors: gm is the soln object");
  KheReLink(gm, KheMonitorParentMonitor((KHE_MONITOR) gm));
}
