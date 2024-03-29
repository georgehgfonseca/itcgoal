
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
/*  FILE:         khe_avoid_clashes_split_task.c                             */
/*  DESCRIPTION:  One avoid clashes split task, used by KheSplitAndLink()    */
/*                                                                           */
/*****************************************************************************/
#include "khe_layer_tree.h"

#define DEBUG1 0

/*****************************************************************************/
/*                                                                           */
/*   KHE_AVOID_CLASHES_SPLIT_TASK - one avoid clashes task KheSplitAndLink   */
/*                                                                           */
/*****************************************************************************/

struct khe_avoid_clashes_split_task_rec {
  KHE_SPLIT_TASK_TAG		tag;		/* task type tag             */
  int				priority;	/* for sorting               */
  KHE_AVOID_CLASHES_CONSTRAINT	constraint;	/* the constraint            */
};


/*****************************************************************************/
/*                                                                           */
/*  KHE_AVOID_CLASHES_SPLIT_TASK KheAvoidClashesSplitTaskMake(               */
/*    KHE_AVOID_CLASHES_CONSTRAINT c)                                        */
/*                                                                           */
/*  Make a avoid_clashes split task with these attributes.                   */
/*                                                                           */
/*****************************************************************************/

KHE_AVOID_CLASHES_SPLIT_TASK KheAvoidClashesSplitTaskMake(
  KHE_AVOID_CLASHES_CONSTRAINT c)
{
  KHE_AVOID_CLASHES_SPLIT_TASK res;
  MMake(res);
  res->tag = KHE_AVOID_CLASHES_SPLIT_TASK_TAG;
  res->priority = KheConstraintCombinedWeight((KHE_CONSTRAINT) c);
  res->constraint = c;
  return res;
}


/*****************************************************************************/
/*                                                                           */
/*  void KheAvoidClashesSplitTaskFree(KHE_AVOID_CLASHES_SPLIT_TASK st)       */
/*                                                                           */
/*  Free st.                                                                 */
/*                                                                           */
/*****************************************************************************/

void KheAvoidClashesSplitTaskFree(KHE_AVOID_CLASHES_SPLIT_TASK st)
{
  MFree(st);
}


/*****************************************************************************/
/*                                                                           */
/*  bool KheLayerAddEvent(KHE_LAYER layer, KHE_EVENT e)                      */
/*                                                                           */
/*  Either add every solution event to layer and return true, or else        */
/*  add none of them and return false.                                       */
/*                                                                           */
/*****************************************************************************/

/* ***
static bool KheLayerAddEvent(KHE_LAYER layer, KHE_EVENT e)
{
  KHE_SOLN soln;  int i, j;  KHE_SOLN_EVENT se;
  soln = KheLayerSoln(layer);
  for( i = 0;  i < KheEventSolnEventCount(soln, e);  i++ )
  {
    se = KheEventSolnEvent(soln, e, i);
    if( !KheLayerAddSolnEvent(layer, se) )
    {
      ** failure, undo everything and return false **
      for( j = 0;  j < i;  j++ )
      {
	se = KheEventSolnEvent(soln, e, j);
	KheLayerDeleteSolnEvent(layer, se);
      }
      return false;
    }
  }
  return true;
}
*** */


/*****************************************************************************/
/*                                                                           */
/*  void KheLayerDeleteEvent(KHE_LAYER layer, KHE_EVENT e)                   */
/*                                                                           */
/*  Delete every solution event of e from layer.                             */
/*                                                                           */
/*****************************************************************************/

/* ***
static void KheLayerDeleteEvent(KHE_LAYER layer, KHE_EVENT e)
{
  KHE_SOLN soln;  int i;  KHE_SOLN_EVENT se;
  soln = KheLayerSoln(layer);
  for( i = 0;  i < KheEventSolnEventCount(soln, e);  i++ )
  {
    se = KheEventSolnEvent(soln, e, i);
    KheLayerDeleteSolnEvent(layer, se);
  }
}
*** */


/*****************************************************************************/
/*                                                                           */
/*  void KheAvoidClashesSplitTaskTryResource(KHE_RESOURCE r,                 */
/*    KHE_SPLIT_FOREST sf)                                                   */
/*                                                                           */
/*  Try to carry out st on r.                                                */
/*                                                                           */
/*  If some events won't add, at present they just get left out.             */
/*                                                                           */
/*****************************************************************************/

static void KheAvoidClashesSplitTaskTryResource(KHE_RESOURCE r,
  KHE_SPLIT_FOREST sf)
{
  KHE_EVENT e;  int i;  bool success;
  if( DEBUG1 )
    fprintf(stderr, "  [ KheAvoidClashesSplitTaskTryResource(%s, sf)\n",
      KheResourceId(r) != NULL ? KheResourceId(r) : "-");
  for( i = 0;  i < KheResourcePreassignedEventResourceCount(r);  i++ )
  {
    e = KheEventResourceEvent(KheResourcePreassignedEventResource(r, i));
    if( DEBUG1 )
      fprintf(stderr, "    trying event %s (duration %d):\n",
	KheEventId(e) != NULL ? KheEventId(e) : "-", KheEventDuration(e));

    /* add e's layer membership to the forest, if it will accept it */
    success = KheSplitForestTryAddToResource(sf, e, r);
    if( DEBUG1 )
      fprintf(stderr, "  %s\n", success ? "success" : "failure");
  }
  if( DEBUG1 )
    fprintf(stderr, "  ] KheAvoidClashesSplitTaskTryResource returning\n");
}


/*****************************************************************************/
/*                                                                           */
/*  void KheAvoidClashesSplitTaskTryResource(KHE_RESOURCE r,                 */
/*    KHE_SPLIT_FOREST sf, ARRAY_KHE_LAYER *layers)                          */
/*                                                                           */
/*  Try to carry out st on r.  Parameter *layers is a scratch temp.          */
/*                                                                           */
/*****************************************************************************/

/* ***
static void KheAvoidClashesSplitTaskTryResource(KHE_RESOURCE r,
  KHE_SPLIT_FOREST sf, ARRAY_KHE_LAYER *layers)
{
  KHE_LAYER layer;  KHE_EVENT e;  int i, j;  bool done;
  if( DEBUG1 )
    fprintf(stderr, "  [ KheAvoidClashesSplitTaskTryResource(st, %s, sf)\n",
      KheResourceId(r) != NULL ? KheResourceId(r) : "-");
  MArrayClear(*layers);
  for( i = 0;  i < KheResourcePreassignedEventResourceCount(r);  i++ )
  {
    e = KheEventResourceEvent(KheResourcePreassignedEventResource(r, i));
    if( DEBUG1 )
      fprintf(stderr, "    trying event %s (duration %d):\n",
	KheEventId(e) != NULL ? KheEventId(e) : "-", KheEventDuration(e));

    ** add e to the first existing layer that will accept it **
    done = false;
    MArrayForEach(*layers, layer, j)
    {
      ** try to add e to layer, in soln and forest **
      if( DEBUG1 )
	fprintf(stderr, "      layer %d (duration %d) ", j,
	  KheLayerDuration(layer));
      if( KheLayerAddEvent(layer, e) )
      {
	if( !KheSplitForestTryAddToLayer(sf, e, layer) )
	{
	  if( DEBUG1 )
	    fprintf(stderr, "  KheSplitForestTryAddToLayer failed\n");
          KheLayerDeleteEvent(layer, e);
	}
	else
	{
	  if( DEBUG1 )
	    fprintf(stderr, "  success\n");
	  done = true;
	  break;
	}
      }
      else
	if( DEBUG1 )
	  fprintf(stderr, "  KheLayerAddEvent failed\n");
    }

    ** not done, so make a new layer and try to add e to that **
    if( !done )
    {
      layer = KheLayerMake(KheSplitForestSoln(sf), r);
      if( DEBUG1 )
	fprintf(stderr, "      new layer (duration %d) ",
	  KheLayerDuration(layer));
      if( KheLayerAddEvent(layer, e) )
      {
	if( !KheSplitForestTryAddToLayer(sf, e, layer) )
	{
          KheLayerDeleteEvent(layer, e);
	  KheLayerDelete(layer);
	  if( DEBUG1 )
	    fprintf(stderr, "  KheSplitForestTryAddToLayer failed\n");
	}
	else
	{
	  if( DEBUG1 )
	    fprintf(stderr, "  success\n");
	  MArrayAddLast(*layers, layer);
	}
      }
      else
      {
	if( DEBUG1 )
	  fprintf(stderr, "  KheLayerAddEvent failed\n");
	KheLayerDelete(layer);
      }
    }
  }
  if( DEBUG1 )
    fprintf(stderr, "  ] KheAvoidClashesSplitTaskTryResource returning\n");
}
*** */


/*****************************************************************************/
/*                                                                           */
/*  void KheAvoidClashesSplitTaskTry(KHE_AVOID_CLASHES_SPLIT_TASK st,        */
/*    KHE_SPLIT_FOREST sf)                                                   */
/*                                                                           */
/*  Try st on sf.                                                            */
/*                                                                           */
/*****************************************************************************/

void KheAvoidClashesSplitTaskTry(KHE_AVOID_CLASHES_SPLIT_TASK st,
  KHE_SPLIT_FOREST sf)
{
  int i, j;  KHE_RESOURCE r;  KHE_RESOURCE_GROUP rg;
  /* ARRAY_KHE_LAYER layers; */
  /* MArrayInit(layers); */
  for( i=0; i<KheAvoidClashesConstraintResourceGroupCount(st->constraint); i++ )
  {
    rg = KheAvoidClashesConstraintResourceGroup(st->constraint, i);
    if( DEBUG1 )
      fprintf(stderr, "[ KheAvoidClashesSplitTaskTry at rg %s\n",
	KheResourceGroupId(rg) == NULL ? "-" : KheResourceGroupId(rg));
    for( j = 0;  j < KheResourceGroupResourceCount(rg);  j++ )
    {
      r = KheResourceGroupResource(rg, j);
      KheAvoidClashesSplitTaskTryResource(r, sf /* , &layers */);
    }
    if( DEBUG1 )
      fprintf(stderr, "]\n");
  }
  for( i = 0; i < KheAvoidClashesConstraintResourceCount(st->constraint);  i++ )
  {
    r = KheAvoidClashesConstraintResource(st->constraint, i);
    if( DEBUG1 )
      fprintf(stderr, "[ KheAvoidClashesSplitTaskTry at r %s\n",
	KheResourceId(r) == NULL ? "-" : KheResourceId(r));
    KheAvoidClashesSplitTaskTryResource(r, sf /* , &layers */);
    if( DEBUG1 )
      fprintf(stderr, "]\n");
  }
  /* MArrayFree(layers); */
}


/*****************************************************************************/
/*                                                                           */
/*  void KheAvoidClashesSplitTaskDebug(KHE_AVOID_CLASHES_SPLIT_TASK st,      */
/*    int indent, FILE *fp)                                                  */
/*                                                                           */
/*  Debug print of task st onto fp with the given indent.                    */
/*                                                                           */
/*****************************************************************************/

void KheAvoidClashesSplitTaskDebug(KHE_AVOID_CLASHES_SPLIT_TASK st,
  int indent, FILE *fp)
{
  fprintf(fp, "%*s[ %d Avoid Clashes Split Task %s ]\n", indent, "",
    st->priority, KheConstraintId((KHE_CONSTRAINT) st->constraint));
}
