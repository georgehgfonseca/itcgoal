
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
/*  FILE:         khe_split_layer.c                                          */
/*  DESCRIPTION:  a layer of split classes                                   */
/*                                                                           */
/*****************************************************************************/
#include "khe_layer_tree.h"


/*****************************************************************************/
/*                                                                           */
/*   KHE_SPLIT_LAYER - a layer of equivalence classes                        */
/*                                                                           */
/*   Implementation note.  Although this module is held alongside all        */
/*   the other split modules, for simplicity, it is in reality used only     */
/*   by KHE_SPLIT_CLASS, and should be considered private to that module.    */
/*                                                                           */
/*****************************************************************************/

struct khe_split_layer_rec {

  /* basic structural fields */
  /* KHE_LAYER			layer; */		/* layer represented */
  KHE_RESOURCE			resource;		/* preass resource   */
  ARRAY_KHE_SPLIT_CLASS		child_classes;		/* child classes     */

  /* local requirements */
  KHE_PARTITION			min_partition;

  /* saved copy of local requirements, used when trying tasks */
  bool				save_active;
  ARRAY_KHE_SPLIT_CLASS		save_child_classes;
  KHE_PARTITION			save_min_partition;
};


/*****************************************************************************/
/*                                                                           */
/*  Submodule "construction and query"                                       */
/*                                                                           */
/*****************************************************************************/

/*****************************************************************************/
/*                                                                           */
/*  KHE_SPLIT_LAYER KheSplitLayerMake(KHE_LAYER layer, KHE_PARTITION p)      */
/*                                                                           */
/*  Make a new split layer with these attributes.                            */
/*                                                                           */
/*****************************************************************************/

KHE_SPLIT_LAYER KheSplitLayerMake(/* KHE_LAYER layer */ KHE_RESOURCE r,
  KHE_PARTITION p)
{
  KHE_SPLIT_LAYER res;
  MMake(res);

  /* basic structural fields */
  /* res->layer = layer; */
  res->resource = r;
  MArrayInit(res->child_classes);

  /* local requirements */
  res->min_partition = p;

  /* saved copy of local requirements, used when trying tasks */
  res->save_active = false;
  MArrayInit(res->save_child_classes);
  res->save_min_partition = KhePartitionMake();
  return res;
}


/*****************************************************************************/
/*                                                                           */
/*  KHE_LAYER KheSplitLayerLayer(KHE_SPLIT_LAYER sl)                         */
/*                                                                           */
/*  Return the solution layer attribute of sl.                               */
/*                                                                           */
/*****************************************************************************/

/* ***
KHE_LAYER KheSplitLayerLayer(KHE_SPLIT_LAYER sl)
{
  return sl->layer;
}
*** */


/*****************************************************************************/
/*                                                                           */
/*  KHE_RESOURCE KheSplitLayerResource(KHE_SPLIT_LAYER sl)                   */
/*                                                                           */
/*  Return the resource which the soln events of this layer all share.       */
/*                                                                           */
/*****************************************************************************/

KHE_RESOURCE KheSplitLayerResource(KHE_SPLIT_LAYER sl)
{
  return sl->resource;
}


/*****************************************************************************/
/*                                                                           */
/*  void KheSplitLayerFree(KHE_SPLIT_LAYER sl)                               */
/*                                                                           */
/*  Free sw and all its arrays.                                              */
/*                                                                           */
/*****************************************************************************/

void KheSplitLayerFree(KHE_SPLIT_LAYER sl)
{
  MArrayFree(sl->child_classes);
  KhePartitionFree(sl->min_partition);
  MArrayFree(sl->save_child_classes);
  KhePartitionFree(sl->save_min_partition);
  MFree(sl);
}


/*****************************************************************************/
/*                                                                           */
/*  Submodule "child classes"                                                */
/*                                                                           */
/*****************************************************************************/

/*****************************************************************************/
/*                                                                           */
/*  void KheSplitLayerAddSplitClass(KHE_SPLIT_LAYER sl, KHE_SPLIT_CLASS sc)  */
/*                                                                           */
/*  Add sc as a child class of sl.                                           */
/*                                                                           */
/*****************************************************************************/

void KheSplitLayerAddSplitClass(KHE_SPLIT_LAYER sl, KHE_SPLIT_CLASS sc)
{
  MArrayAddLast(sl->child_classes, sc);
  KhePartitionSum(sl->min_partition, KheSplitClassMinPartition(sc));
}


/*****************************************************************************/
/*                                                                           */
/* void KheSplitLayerDeleteSplitClass(KHE_SPLIT_LAYER sl, KHE_SPLIT_CLASS sc)*/
/*                                                                           */
/*  Delete sc from sl.                                                       */
/*                                                                           */
/*****************************************************************************/

void KheSplitLayerDeleteSplitClass(KHE_SPLIT_LAYER sl, KHE_SPLIT_CLASS sc)
{
  int pos;
  if( !MArrayContains(sl->child_classes, sc, &pos) )
    MAssert(false, "KheSplitLayerDeleteChildClass internal error");
  MArrayRemove(sl->child_classes, pos);
  KhePartitionDifference(sl->min_partition, KheSplitClassMinPartition(sc));
}


/*****************************************************************************/
/*                                                                           */
/*  bool KheSplitLayerContainsSplitClass(KHE_SPLIT_LAYER sl,                 */
/*    KHE_SPLIT_CLASS sc)                                                    */
/*                                                                           */
/*  Return true if sl contains sc.                                           */
/*                                                                           */
/*****************************************************************************/

bool KheSplitLayerContainsSplitClass(KHE_SPLIT_LAYER sl, KHE_SPLIT_CLASS sc)
{
  int pos;
  return MArrayContains(sl->child_classes, sc, &pos);
}


/*****************************************************************************/
/*                                                                           */
/*  int KheSplitLayerSplitClassCount(KHE_SPLIT_LAYER sl)                     */
/*                                                                           */
/*  Return the number of split classes of sl.                                */
/*                                                                           */
/*****************************************************************************/

int KheSplitLayerSplitClassCount(KHE_SPLIT_LAYER sl)
{
  return MArraySize(sl->child_classes);
}


/*****************************************************************************/
/*                                                                           */
/*  KHE_SPLIT_CLASS KheSplitLayerSplitClass(KHE_SPLIT_LAYER sl, int i)       */
/*                                                                           */
/*  Return the i'th split class of sl.                                       */
/*                                                                           */
/*****************************************************************************/

KHE_SPLIT_CLASS KheSplitLayerSplitClass(KHE_SPLIT_LAYER sl, int i)
{
  return MArrayGet(sl->child_classes, i);
}


/*****************************************************************************/
/*                                                                           */
/*  Submodule "partition"                                                    */
/*                                                                           */
/*****************************************************************************/

/*****************************************************************************/
/*                                                                           */
/*  KHE_PARTITION KheSplitLayerMinPartition(KHE_SPLIT_LAYER sl)              */
/*                                                                           */
/*  Return the min partition of sl.                                          */
/*                                                                           */
/*****************************************************************************/

KHE_PARTITION KheSplitLayerMinPartition(KHE_SPLIT_LAYER sl)
{
  return sl->min_partition;
}


/*****************************************************************************/
/*                                                                           */
/*  Submodule "trials"                                                       */
/*                                                                           */
/*****************************************************************************/

/*****************************************************************************/
/*                                                                           */
/*  void KheSplitLayerTryBegin(KHE_SPLIT_LAYER sl)                           */
/*                                                                           */
/*  Begin a trial which might change the partition held in sl.               */
/*                                                                           */
/*****************************************************************************/

void KheSplitLayerTryBegin(KHE_SPLIT_LAYER sl)
{
  int i;
  if( !sl->save_active )
  {
    sl->save_active = true;
    MArrayClear(sl->save_child_classes);
    MArrayAppend(sl->save_child_classes, sl->child_classes, i);
    KhePartitionAssign(sl->save_min_partition, sl->min_partition);
  }
}


/*****************************************************************************/
/*                                                                           */
/*  void KheSplitLayerTryMinPartitionChange(KHE_SPLIT_LAYER sl,              */
/*    KHE_PARTITION old_p, KHE_PARTITION new_p)                              */
/*                                                                           */
/*  Try this partition change.                                               */
/*                                                                           */
/*****************************************************************************/

void KheSplitLayerTryMinPartitionChange(KHE_SPLIT_LAYER sl,
  KHE_PARTITION old_p, KHE_PARTITION new_p)
{
  MAssert(sl->save_active, "KheSplitLayerTryMinPartitionChange internal error");
  KhePartitionDifference(sl->min_partition, old_p);
  KhePartitionSum(sl->min_partition, new_p);
}


/*****************************************************************************/
/*                                                                           */
/*  void KheSplitLayerTryMerge(KHE_SPLIT_LAYER dest_sl,                      */
/*    KHE_SPLIT_LAYER src_sl)                                                */
/*                                                                           */
/*  Try merging src_sl into dest_sl.                                         */
/*                                                                           */
/*****************************************************************************/

void KheSplitLayerTryMerge(KHE_SPLIT_LAYER dest_sl, KHE_SPLIT_LAYER src_sl)
{
  int i;
  KhePartitionSum(dest_sl->min_partition, src_sl->min_partition);
  MArrayAppend(dest_sl->child_classes, src_sl->child_classes, i);
}


/*****************************************************************************/
/*                                                                           */
/*  void KheSplitLayerTryEnd(KHE_SPLIT_LAYER sl, bool success)               */
/*                                                                           */
/*  A trial is ending, successful or otherwise.                              */
/*                                                                           */
/*****************************************************************************/

void KheSplitLayerTryEnd(KHE_SPLIT_LAYER sl, bool success)
{
  int i;
  if( sl->save_active )
  {
    sl->save_active = false;
    if( !success )
    {
      MArrayClear(sl->child_classes);
      MArrayAppend(sl->child_classes, sl->save_child_classes, i);
      KhePartitionAssign(sl->min_partition, sl->save_min_partition);
    }
  }
}


/*****************************************************************************/
/*                                                                           */
/*  Submodule "debug"                                                        */
/*                                                                           */
/*****************************************************************************/

/*****************************************************************************/
/*                                                                           */
/*  void KheSplitLayerDebugBrief(KHE_SPLIT_LAYER sl, int indent, FILE *fp)   */
/*                                                                           */
/*  Brief debug print of sl onto fp with the given indent.                   */
/*                                                                           */
/*****************************************************************************/

void KheSplitLayerDebugBrief(KHE_SPLIT_LAYER sl, int indent, FILE *fp)
{
  int i;  KHE_SPLIT_CLASS sc;
  fprintf(fp, "%*s[ Split Layer %s: ", indent, "",
    KhePartitionShow(sl->min_partition));
  MArrayForEach(sl->child_classes, &sc, &i)
  {
    if( i > 0 )
      fprintf(fp, ", ");
    KheSplitClassDebugBrief(sc, fp);
  }
  fprintf(fp, " ]\n");
}


/*****************************************************************************/
/*                                                                           */
/*  void KheSplitLayerDebug(KHE_SPLIT_LAYER sl, int indent, FILE *fp)        */
/*                                                                           */
/*  Debug print of sl onto fp with the given indent.                         */
/*                                                                           */
/*****************************************************************************/

void KheSplitLayerDebug(KHE_SPLIT_LAYER sl, int indent, FILE *fp)
{
  int i;  KHE_SPLIT_CLASS sc;
  fprintf(fp, "%*s[ Split Layer %s\n", indent, "",
    KhePartitionShow(sl->min_partition));
  MArrayForEach(sl->child_classes, &sc, &i)
    KheSplitClassDebug(sc, indent + 2, fp);
  fprintf(fp, "%*s]\n", indent, "");
}
