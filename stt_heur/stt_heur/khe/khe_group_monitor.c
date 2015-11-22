
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
/*  FILE:         khe_group_monitor.c                                        */
/*  DESCRIPTION:  A group monitor                                            */
/*                                                                           */
/*****************************************************************************/
#include "khe_interns.h"

#define DEBUG1 0
#define DEBUG2 0
#define DEBUG3 0

/*****************************************************************************/
/*                                                                           */
/*  KHE_GROUP_MONITOR - monitors a group of other monitors.                  */
/*                                                                           */
/*****************************************************************************/

struct khe_group_monitor_rec {

  /* inherited from KHE_MONITOR */
  KHE_SOLN			soln;			/* encl. solution    */
  int				index_in_soln;		/* index in soln     */
  unsigned char			tag;			/* tag field         */
  bool				attached;		/* true if attached  */
  void				*back;			/* back pointer      */
  KHE_GROUP_MONITOR		parent_monitor;		/* parent monitor    */
  int				parent_index;		/* index in parent   */
  int				defect_index;		/* defect index      */
  int				trace_num;		/* trace visit num   */
  KHE_COST			trace_cost;		/* at start of trace */
  KHE_COST			cost;			/* current cost      */

  /* specific to KHE_GROUP_MONITOR */
  ARRAY_KHE_MONITOR		child_monitors;		/* child monitors    */
  ARRAY_KHE_MONITOR		defects;		/* defects           */
  ARRAY_KHE_MONITOR		defects_copy;		/* copy of defects   */
  ARRAY_KHE_TRACE		traces; 		/* traces            */
  int				sub_tag;		/* sub tag           */
  char				*sub_tag_label;		/* sub tag label     */
  KHE_GROUP_MONITOR		copy;			/* used when copying */
};


/*****************************************************************************/
/*                                                                           */
/*  Submodule "construction and query"                                       */
/*                                                                           */
/*****************************************************************************/

/*****************************************************************************/
/*                                                                           */
/*  KHE_GROUP_MONITOR KheGroupMonitorMake(KHE_SOLN soln, int sub_tag,        */
/*    char *sub_tag_label)                                                   */
/*                                                                           */
/*  Make a new group monitor with these attributes, and no parent or         */
/*  children.                                                                */
/*                                                                           */
/*****************************************************************************/

KHE_GROUP_MONITOR KheGroupMonitorMake(KHE_SOLN soln, int sub_tag,
  char *sub_tag_label)
{
  KHE_GROUP_MONITOR res;  /* int i; */
  if( DEBUG1 )
    fprintf(stderr, "[ KheGroupMonitorMake(soln, %d, %s)\n",
      sub_tag, sub_tag_label);
  MMake(res);
  KheMonitorInitCommonFields((KHE_MONITOR) res, soln, KHE_GROUP_MONITOR_TAG);
  MArrayInit(res->child_monitors);
  MArrayInit(res->defects);
  MArrayInit(res->defects_copy);
  MArrayInit(res->traces);
  res->sub_tag = sub_tag;
  res->sub_tag_label = sub_tag_label;
  res->copy = NULL;
  /* *** now having no parent initially
  KheGroupMonitorAddMonitor((KHE_GROUP_MONITOR) soln, (KHE_MONITOR) res);
  *** */
  if( DEBUG1 )
    fprintf(stderr, "] KheGroupMonitorMake returning %p\n", (void *) res);
  return res;
}


/*****************************************************************************/
/*                                                                           */
/*  void KheGroupMonitorDelete(KHE_GROUP_MONITOR gm)                         */
/*                                                                           */
/*  Delete gm.  This should *not* delete its child monitors!                 */
/*                                                                           */
/*****************************************************************************/

void KheGroupMonitorDelete(KHE_GROUP_MONITOR gm)
{
  MAssert(gm != (KHE_GROUP_MONITOR) gm->soln,
    "KheGroupMonitorDelete:  gm is soln");
  MAssert(MArraySize(gm->traces) == 0,
    "KheGroupMonitorDelete:  gm is currently being traced");
  while( MArraySize(gm->child_monitors) > 0 )
    KheGroupMonitorDeleteChildMonitor(gm, MArrayLast(gm->child_monitors));
  if( gm->parent_monitor != NULL )
    KheGroupMonitorDeleteChildMonitor(gm->parent_monitor, (KHE_MONITOR) gm);
  KheSolnDeleteMonitor(gm->soln, (KHE_MONITOR) gm);
  MArrayFree(gm->child_monitors);
  MArrayFree(gm->defects);
  MArrayFree(gm->defects_copy);
  MArrayFree(gm->traces);
  MFree(gm);
}


/*****************************************************************************/
/*                                                                           */
/*  void KheGroupMonitorBypassAndDelete(KHE_GROUP_MONITOR gm)                */
/*                                                                           */
/*  Move gm's child monitors to be children of gm's parent, if any, then     */
/*  delete gm.                                                               */
/*                                                                           */
/*****************************************************************************/

void KheGroupMonitorBypassAndDelete(KHE_GROUP_MONITOR gm)
{
  KHE_MONITOR m;
  if( gm->parent_monitor != NULL )
    while( MArraySize(gm->child_monitors) > 0 )
    {
      m = MArrayLast(gm->child_monitors);
      KheGroupMonitorDeleteChildMonitor(gm, m);
      KheGroupMonitorAddChildMonitor(gm->parent_monitor, m);
    }
  KheGroupMonitorDelete(gm);
}


/*****************************************************************************/
/*                                                                           */
/*  KHE_GROUP_MONITOR KheGroupMonitorCopyPhase1(KHE_GROUP_MONITOR gm)        */
/*                                                                           */
/*  Carry out Phase 1 of copying gm.                                         */
/*                                                                           */
/*****************************************************************************/

KHE_GROUP_MONITOR KheGroupMonitorCopyPhase1(KHE_GROUP_MONITOR gm)
{
  KHE_GROUP_MONITOR copy;  KHE_MONITOR m;  int i;
  if( gm->soln == (KHE_SOLN) gm )
  {
    /* gm is actually the soln object */
    return (KHE_GROUP_MONITOR) KheSolnCopyPhase1((KHE_SOLN) gm);
  }
  else
  {
    if( gm->copy == NULL )
    {
      /* KheSolnMatchingUpdate(gm->soln); */
      MAssert(MArraySize(gm->traces) == 0,
        "KheGroupMonitorCopy cannot copy:  gm is currently being traced");
      MMake(copy);
      gm->copy = copy;
      KheMonitorCopyCommonFields((KHE_MONITOR) copy, (KHE_MONITOR) gm);
      MArrayInit(copy->child_monitors);
      MArrayForEach(gm->child_monitors, &m, &i)
        MArrayAddLast(copy->child_monitors, KheMonitorCopyPhase1(m));
      MArrayInit(copy->defects);
      MArrayForEach(gm->defects, &m, &i)
        MArrayAddLast(copy->defects, KheMonitorCopyPhase1(m));
      MArrayInit(copy->defects_copy);
      MArrayForEach(gm->defects_copy, &m, &i)
        MArrayAddLast(copy->defects_copy, KheMonitorCopyPhase1(m));
      MArrayInit(copy->traces);
      copy->sub_tag = gm->sub_tag;
      copy->sub_tag_label = gm->sub_tag_label;
      copy->copy = NULL;
    }
    return gm->copy;
  }
}


/*****************************************************************************/
/*                                                                           */
/*  void KheGroupMonitorCopyPhase2(KHE_GROUP_MONITOR gm)                     */
/*                                                                           */
/*  Carry out Phase 2 of copying gm.                                         */
/*                                                                           */
/*****************************************************************************/

void KheGroupMonitorCopyPhase2(KHE_GROUP_MONITOR gm)
{
  KHE_MONITOR m;  int i;
  if( gm->soln == (KHE_SOLN) gm )
  {
    /* gm is actually the soln object */
    KheSolnCopyPhase2((KHE_SOLN) gm);
  }
  else if( gm->copy != NULL )
  {
    gm->copy = NULL;
    MArrayForEach(gm->child_monitors, &m, &i)
      KheMonitorCopyPhase2(m);
  }
}


/*****************************************************************************/
/*                                                                           */
/*  int KheGroupMonitorSubTag(KHE_GROUP_MONITOR gm)                          */
/*                                                                           */
/*  Return the sub_tag attribute of gm.                                      */
/*                                                                           */
/*****************************************************************************/

int KheGroupMonitorSubTag(KHE_GROUP_MONITOR gm)
{
  return gm->sub_tag;
}


/*****************************************************************************/
/*                                                                           */
/*  char *KheGroupMonitorSubTagLabel(KHE_GROUP_MONITOR gm)                   */
/*                                                                           */
/*  Return the sub_tag_label attribute of gm.                                */
/*                                                                           */
/*****************************************************************************/

char *KheGroupMonitorSubTagLabel(KHE_GROUP_MONITOR gm)
{
  return gm->sub_tag_label;
}


/*****************************************************************************/
/*                                                                           */
/*  Submodule "child monitors"                                               */
/*                                                                           */
/*****************************************************************************/

/*****************************************************************************/
/*                                                                           */
/*  bool KheGroupMonitorCycle(KHE_GROUP_MONITOR gm, KHE_MONITOR m)           */
/*                                                                           */
/*  Return true if adding m to gm would cause a monitor cycle.               */
/*                                                                           */
/*****************************************************************************/

static bool KheGroupMonitorCycle(KHE_GROUP_MONITOR gm, KHE_MONITOR m)
{
  if( KheMonitorTag(m) == KHE_GROUP_MONITOR_TAG )
  {
    do
    {
      if( gm == (KHE_GROUP_MONITOR) m )
	return true;
      gm = gm->parent_monitor;
    } while( gm != NULL );
  }
  return false;
}


/*****************************************************************************/
/*                                                                           */
/*  void KheGroupMonitorAddDefect(KHE_GROUP_MONITOR gm, KHE_MONITOR m)       */
/*                                                                           */
/*  Add m to the list of defects of gm.                                      */
/*                                                                           */
/*****************************************************************************/

static void KheGroupMonitorAddDefect(KHE_GROUP_MONITOR gm, KHE_MONITOR m)
{
  int pos;
  if( DEBUG3 )
  {
    MAssert(KheMonitorDefectIndex(m) == -1,
      "KheGroupMonitorAddDefect internal error 1");
    MAssert(!MArrayContains(gm->defects, m, &pos),
      "KheGroupMonitorAddDefect internal error 2");
  }
  KheMonitorSetDefectIndex(m, MArraySize(gm->defects));
  MArrayAddLast(gm->defects, m);
}


/*****************************************************************************/
/*                                                                           */
/*  void KheGroupMonitorDeleteDefect(KHE_GROUP_MONITOR gm, KHE_MONITOR m)    */
/*                                                                           */
/*  Delete m from the list of defects of gm.                                 */
/*                                                                           */
/*****************************************************************************/

static void KheGroupMonitorDeleteDefect(KHE_GROUP_MONITOR gm, KHE_MONITOR m)
{
  KHE_MONITOR m2;  int pos;
  if( DEBUG2 )
  {
    fprintf(stderr,
      "  [ KheGroupMonitorDeleteDefect(gm %p (%d defects), m (di %d)\n", 
      (void *) gm, MArraySize(gm->defects), KheMonitorDefectIndex(m));
    KheMonitorDebug((KHE_MONITOR) gm, 1, 4, stderr);
    KheMonitorDebug(m, 2, 4, stderr);
  }
  MAssert(MArrayGet(gm->defects, KheMonitorDefectIndex(m)) == m,
    "KheGroupMonitorDeleteDefect internal error 1");
  m2 = MArrayRemoveLast(gm->defects);
  if( m2 != m )
  {
    MArrayPut(gm->defects, KheMonitorDefectIndex(m), m2);
    KheMonitorSetDefectIndex(m2, KheMonitorDefectIndex(m));
  }
  KheMonitorSetDefectIndex(m, -1);
  if( DEBUG3 )
  {
    MAssert(KheMonitorDefectIndex(m) == -1,
      "KheGroupMonitorDeleteDefect internal error 2");
    MAssert(!MArrayContains(gm->defects, m, &pos),
      "KheGroupMonitorDeleteDefect internal error 3");
  }
  if( DEBUG2 )
    fprintf(stderr, "  ] KheGroupMonitorDeleteDefect\n");
}


/*****************************************************************************/
/*                                                                           */
/*  void KheGroupMonitorAddMonitor(KHE_GROUP_MONITOR gm, KHE_MONITOR m)      */
/*                                                                           */
/*  Internal function which adds m to gm, assuming m is currently not the    */
/*  child of any monitor.                                                    */
/*                                                                           */
/*****************************************************************************/

/* *** folded into KheGroupMonitorAddChildMonitor now 
void KheGroupMonitorAddMonitor(KHE_GROUP_MONITOR gm, KHE_MONITOR m)
{
  MAssert(!KheGroupMonitorCycle(gm, m),
    "KheGroupMonitorAddChildMonitor: operation would cause a monitor cycle");
  ** *** not doing this stuff any more
  if( KheMonitorAttached(m) && !KheMonitorAttached((KHE_MONITOR) gm) )
    KheMonitorAttach((KHE_MONITOR) gm);
  *** **

  ** add m to child_monitors **
  KheMonitorSetParentMonitorAndIndex(m, gm, MArraySize(gm->child_monitors));
  MArrayAddLast(gm->child_monitors, m);

  ** change gm's cost and add m to defects, if m has non-zero cost **
  if( KheMonitorCost(m) > 0 )
    KheGroupMonitorChangeCost(gm, m, 0, KheMonitorCost(m));
}
*** */


/*****************************************************************************/
/*                                                                           */
/*  void KheGroupMonitorDeleteMonitor(KHE_GROUP_MONITOR gm, KHE_MONITOR m)   */
/*                                                                           */
/*  Internal function which deletes m from gm.                               */
/*                                                                           */
/*****************************************************************************/

/* folded into KheGroupMonitorDeleteChildMonitor now
void KheGroupMonitorDeleteMonitor(KHE_GROUP_MONITOR gm, KHE_MONITOR m)
{
  KHE_MONITOR m2;

  ** remove m from child_monitors **
  MAssert(MArrayGet(gm->child_monitors, KheMonitorParentIndex(m)) == m,
    "KheGroupMonitorDeleteMonitor internal error");
  m2 = MArrayRemoveLast(gm->child_monitors);
  if( m2 != m )
  {
    MArrayPut(gm->child_monitors, KheMonitorParentIndex(m), m2);
    KheMonitorSetParentMonitorAndIndex(m2, gm, KheMonitorParentIndex(m));
  }

  ** change gm's cost and remove m from defects, if m has non-zero cost **
  if( KheMonitorCost(m) > 0 )
    KheGroupMonitorChangeCost(gm, m, KheMonitorCost(m), 0);
}
*** */


/*****************************************************************************/
/*                                                                           */
/*  void KheGroupMonitorAddChildMonitor(KHE_GROUP_MONITOR gm, KHE_MONITOR m) */
/*                                                                           */
/*  Add m to gm.  This removes it from wherever it is now.                   */
/*                                                                           */
/*****************************************************************************/

void KheGroupMonitorAddChildMonitor(KHE_GROUP_MONITOR gm, KHE_MONITOR m)
{
  /* make sure m's cost is up to date (IMPORTANT - this is a nasty bug fix!) */
  KheMonitorCost(m);

  /* remove m from any parent it has now */
  if( KheMonitorParentMonitor(m) != NULL )
    KheGroupMonitorDeleteChildMonitor(KheMonitorParentMonitor(m), m);

  /* make sure the add would not cause a monitor cycle */
  MAssert(!KheGroupMonitorCycle(gm, m),
    "KheGroupMonitorAddChildMonitor: operation would cause a monitor cycle");

  /* add m to gm's child_monitors */
  KheMonitorSetParentMonitorAndIndex(m, gm, MArraySize(gm->child_monitors));
  MArrayAddLast(gm->child_monitors, m);

  /* change gm's cost and add m to defects, if m has non-zero cost */
  if( KheMonitorCost(m) > 0 )
    KheGroupMonitorChangeCost(gm, m, 0, KheMonitorCost(m));
}


/*****************************************************************************/
/*                                                                           */
/*  void KheGroupMonitorDeleteChildMonitor(KHE_GROUP_MONITOR gm,             */
/*    KHE_MONITOR m)                                                         */
/*                                                                           */
/*  Delete m from gm.                                                        */
/*                                                                           */
/*****************************************************************************/

void KheGroupMonitorDeleteChildMonitor(KHE_GROUP_MONITOR gm, KHE_MONITOR m)
{
  KHE_MONITOR m2;

  /* change gm's cost and remove m from defects, if m has non-zero cost */
  if( KheMonitorCost(m) > 0 )
    KheGroupMonitorChangeCost(gm, m, KheMonitorCost(m), 0);

  /* remove m from gm's child_monitors */
  MAssert(MArrayGet(gm->child_monitors, KheMonitorParentIndex(m)) == m,
    "KheGroupMonitorDeleteChildMonitor internal error");
  m2 = MArrayRemoveLast(gm->child_monitors);
  if( m2 != m )
  {
    MArrayPut(gm->child_monitors, KheMonitorParentIndex(m), m2);
    KheMonitorSetParentMonitorAndIndex(m2, gm, KheMonitorParentIndex(m));
  }
  KheMonitorSetParentMonitorAndIndex(m, NULL, -1);
}


/*****************************************************************************/
/*                                                                           */
/*  int KheGroupMonitorChildMonitorCount(KHE_GROUP_MONITOR gm)               */
/*                                                                           */
/*  Return the number of monitors reporting to gm.                           */
/*                                                                           */
/*****************************************************************************/

int KheGroupMonitorChildMonitorCount(KHE_GROUP_MONITOR gm)
{
  return MArraySize(gm->child_monitors);
}


/*****************************************************************************/
/*                                                                           */
/*  KHE_MONITOR KheGroupMonitorChildMonitor(KHE_GROUP_MONITOR gm, int i)     */
/*                                                                           */
/*  Return the i'th monitor reporting to gm.                                 */
/*                                                                           */
/*****************************************************************************/

KHE_MONITOR KheGroupMonitorChildMonitor(KHE_GROUP_MONITOR gm, int i)
{
  return MArrayGet(gm->child_monitors, i);
}


/*****************************************************************************/
/*                                                                           */
/*  int KheGroupMonitorDefectCount(KHE_GROUP_MONITOR gm)                     */
/*                                                                           */
/*  Return the number of defects (child monitors of non-zero cost) of gm.    */
/*                                                                           */
/*****************************************************************************/

int KheGroupMonitorDefectCount(KHE_GROUP_MONITOR gm)
{
  KheSolnMatchingUpdate(gm->soln);
  return MArraySize(gm->defects);
}


/*****************************************************************************/
/*                                                                           */
/*  KHE_MONITOR KheGroupMonitorDefect(KHE_GROUP_MONITOR gm, int i)           */
/*                                                                           */
/*  Return the i'th defect (child monitor of non-zero cost) of gm.           */
/*                                                                           */
/*****************************************************************************/

KHE_MONITOR KheGroupMonitorDefect(KHE_GROUP_MONITOR gm, int i)
{
  KheSolnMatchingUpdate(gm->soln);
  return MArrayGet(gm->defects, i);
}


/*****************************************************************************/
/*                                                                           */
/*  void int KheGroupMonitorDefectSort(KHE_GROUP_MONITOR gm)                 */
/*                                                                           */
/*  Sort the defects of gm into decreasing cost order.                       */
/*                                                                           */
/*****************************************************************************/

static int KheMonitorDecreasingCostCmp(const void *t1, const void *t2)
{
  KHE_MONITOR m1 = * (KHE_MONITOR *) t1;
  KHE_MONITOR m2 = * (KHE_MONITOR *) t2;
  int cmp = KheCostCmp(KheMonitorCost(m2), KheMonitorCost(m1));
  if( cmp != 0 )
    return cmp;
  else
    return KheMonitorIndexInSoln(m1) - KheMonitorIndexInSoln(m2);
}

void KheGroupMonitorDefectSort(KHE_GROUP_MONITOR gm)
{
  KHE_MONITOR m;  int i;

  /* make sure soln's cost is up to date (IMPORTANT - nasty bug fix!) */
  KheSolnCost(gm->soln);

  MArraySort(gm->defects, &KheMonitorDecreasingCostCmp);
  MArrayForEach(gm->defects, &m, &i)
    KheMonitorSetDefectIndex(m, i);
  if( DEBUG3 )
    for( i = 1;  i < MArraySize(gm->defects);  i++ )
      MAssert(MArrayGet(gm->defects, i) != MArrayGet(gm->defects, i - 1),
	"KheGroupMonitorDefectSort internal error");
}


/*****************************************************************************/
/*                                                                           */
/*  void KheGroupMonitorCopyDefects(KHE_GROUP_MONITOR gm)                    */
/*                                                                           */
/*  Initialize the copied defect list of gm.                                 */
/*                                                                           */
/*****************************************************************************/

void KheGroupMonitorCopyDefects(KHE_GROUP_MONITOR gm)
{ 
  int i;
  MArrayClear(gm->defects_copy);
  MArrayAppend(gm->defects_copy, gm->defects, i);
}


/*****************************************************************************/
/*                                                                           */
/*  int KheGroupMonitorDefectCopyCount(KHE_GROUP_MONITOR gm)                 */
/*                                                                           */
/*  Return the number of elements on gm's copied defect list.                */
/*                                                                           */
/*****************************************************************************/

int KheGroupMonitorDefectCopyCount(KHE_GROUP_MONITOR gm)
{
  return MArraySize(gm->defects_copy);
}


/*****************************************************************************/
/*                                                                           */
/*  KHE_MONITOR KheGroupMonitorDefectCopy(KHE_GROUP_MONITOR gm, int i)       */
/*                                                                           */
/*  Return the i'th element of gm's copied defect list.                      */
/*                                                                           */
/*****************************************************************************/

KHE_MONITOR KheGroupMonitorDefectCopy(KHE_GROUP_MONITOR gm, int i)
{
  return MArrayGet(gm->defects_copy, i);
}


/*****************************************************************************/
/*                                                                           */
/*  KHE_COST KheGroupMonitorCostByType(KHE_GROUP_MONITOR gm,                 */
/*    KHE_MONITOR_TAG tag, int *defect_count)                                */
/*                                                                           */
/*  Find the cost of monitors of type tag in the subtree rooted at gm,       */
/*  and the number of defects.                                               */
/*                                                                           */
/*****************************************************************************/

KHE_COST KheGroupMonitorCostByType(KHE_GROUP_MONITOR gm, KHE_MONITOR_TAG tag,
  int *defect_count)
{
  KHE_COST res;  int i, dc;  KHE_MONITOR m;
  res = 0;  *defect_count = 0;
  if( tag == KHE_GROUP_MONITOR_TAG )
    return res;
  KheSolnMatchingUpdate(gm->soln);
  MArrayForEach(gm->defects, &m, &i)
    if( KheMonitorTag(m) == tag )
    {
      res += KheMonitorCost(m);
      *defect_count += 1;
    }
    else if( KheMonitorTag(m) == KHE_GROUP_MONITOR_TAG )
    {
      res += KheGroupMonitorCostByType((KHE_GROUP_MONITOR) m, tag, &dc);
      *defect_count += dc;
    }
  return res;
}


/*****************************************************************************/
/*                                                                           */
/*  void KheGroupMonitorCostByTypeDebug(KHE_GROUP_MONITOR gm,                */
/*    int verbosity, int indent, FILE *fp)                                   */
/*                                                                           */
/*  Debug print of gm's cost onto fp.                                        */
/*                                                                           */
/*****************************************************************************/

void KheGroupMonitorCostByTypeDebug(KHE_GROUP_MONITOR gm,
  int verbosity, int indent, FILE *fp)
{
  int tag, defect_count, total_defect_count;  KHE_COST cost, total_cost;
  if( verbosity >= 1 && indent >= 0 )
  {
    fprintf(fp, "%*s%-31s %9s %13s\n", indent, "", gm->sub_tag_label,
      "Defects", "Cost");
    fprintf(fp, "%*s-------------------------------------------------------\n",
      indent, "");
    total_cost = 0;  total_defect_count = 0;
    for( tag = 0;  tag < KHE_MONITOR_TAG_COUNT;  tag++ )
    {
      cost = KheGroupMonitorCostByType(gm, tag, &defect_count);
      if( cost != 0 || defect_count != 0 )
	fprintf(fp, "%*s%-34s %6d %13.4f\n", indent, "", KheMonitorTagShow(tag),
	  defect_count, KheCostShow(cost));
      total_cost += cost;
      total_defect_count += defect_count;
    }
    fprintf(fp, "%*s-------------------------------------------------------\n",
      indent, "");
    fprintf(fp, "%*s%-34s %6d %13.4f\n", indent, "", "Total",
      total_defect_count, KheCostShow(total_cost));
    MAssert(total_cost == gm->cost, "KheGroupMonitorCostByTypeDebug "
      " internal error (total_cost %.4f, gm->cost %.4f)\n",
      KheCostShow(total_cost), KheCostShow(gm->cost));
  }
}


/*****************************************************************************/
/*                                                                           */
/*  Submodule "attach and detach"                                            */
/*                                                                           */
/*****************************************************************************/

/*****************************************************************************/
/*                                                                           */
/*  void KheGroupMonitorAttach(KHE_GROUP_MONITOR gm)                         */
/*                                                                           */
/*  Attach gm.   It is known to be currently detached with cost 0.           */
/*                                                                           */
/*****************************************************************************/

/* *** no longer used with group monitors
void KheGroupMonitorAttach(KHE_GROUP_MONITOR gm)
{
  MAssert(gm->cost == 0, "KheGroupMonitorAttach internal error");
  gm->attached = true;
}
*** */


/*****************************************************************************/
/*                                                                           */
/*  void KheGroupMonitorDetach(KHE_GROUP_MONITOR gm)                         */
/*                                                                           */
/*  Detach gm.  It is known to be currently attached.                        */
/*                                                                           */
/*****************************************************************************/

/* *** no longer used with group monitors
void KheGroupMonitorDetach(KHE_GROUP_MONITOR gm)
{
  int i;  KHE_MONITOR m;
  MArrayForEach(gm->child_monitors, &m, &i)
    if( KheMonitorAttached(m) )
      KheMonitorDetach(m);
  gm->attached = false;
}
*** */


/*****************************************************************************/
/*                                                                           */
/*  Submodule "update and tracing"                                           */
/*                                                                           */
/*****************************************************************************/

/*****************************************************************************/
/*                                                                           */
/*  void KheGroupMonitorBeginTrace(KHE_GROUP_MONITOR gm, KHE_TRACE t)        */
/*                                                                           */
/*  Begin tracing t.                                                         */
/*                                                                           */
/*****************************************************************************/

void KheGroupMonitorBeginTrace(KHE_GROUP_MONITOR gm, KHE_TRACE t)
{
  MArrayAddLast(gm->traces, t);
}


/*****************************************************************************/
/*                                                                           */
/*  void KheGroupMonitorEndTrace(KHE_GROUP_MONITOR gm, KHE_TRACE t)          */
/*                                                                           */
/*  End tracing t.                                                           */
/*                                                                           */
/*****************************************************************************/

void KheGroupMonitorEndTrace(KHE_GROUP_MONITOR gm, KHE_TRACE t)
{
  int pos;
  if( !MArrayContains(gm->traces, t, &pos) )
    MAssert(false, "KheGroupMonitorEndTrace internal error");
  MArrayRemove(gm->traces, pos);
}


/*****************************************************************************/
/*                                                                           */
/*  void KheGroupMonitorChangeCost(KHE_GROUP_MONITOR gm, KHE_MONITOR m,      */
/*    KHE_COST old_cost, KHE_COST new_cost)                                  */
/*                                                                           */
/*  Let gm know that its child monitor m has changed its cost from old_cost  */
/*  to new_cost.                                                             */
/*                                                                           */
/*****************************************************************************/

void KheGroupMonitorChangeCost(KHE_GROUP_MONITOR gm, KHE_MONITOR m,
  KHE_COST old_cost, KHE_COST new_cost)
{
  KHE_TRACE t;  int i;  KHE_COST delta_cost;
  if( DEBUG2 )
  {
    /* important: can't call KheMonitorCost in this function! */
    fprintf(stderr,
      "[ KheGroupMonitorChangeCost(gm(%.4f), m, %.4f, %.4f)\n",
      KheCostShow((gm->cost)), KheCostShow(old_cost), KheCostShow(new_cost));
  }
  if( DEBUG3 )
  {
    if( !MArrayContains(gm->child_monitors, m, &i) )
      MAssert(false, "KheGroupMonitorChangeCost internal error 1");
    if( old_cost > 0 && !MArrayContains(gm->defects, m, &i) )
      MAssert(false, "KheGroupMonitorChangeCost internal error 2");
    MAssert(new_cost >= 0, "KheGroupMonitorChangeCost internal error 3");
    MAssert(new_cost != old_cost, "KheGroupMonitorChangeCost internal error 4");
  }
  MArrayForEach(gm->traces, &t, &i)
    KheTraceChangeCost(t, m, old_cost);
  delta_cost = new_cost - old_cost;
  gm->cost += delta_cost;
  if( old_cost == 0 )
    KheGroupMonitorAddDefect(gm, m);
  else if( new_cost == 0 )
    KheGroupMonitorDeleteDefect(gm, m);
  if( gm->parent_monitor != NULL )
    KheGroupMonitorChangeCost(gm->parent_monitor, (KHE_MONITOR) gm,
      gm->cost - delta_cost, gm->cost);
  if( DEBUG2 )
    fprintf(stderr, "] KheGroupMonitorChangeCost returning\n");
}


/*****************************************************************************/
/*                                                                           */
/*  Submodule "deviations"                                                   */
/*                                                                           */
/*****************************************************************************/

/*****************************************************************************/
/*                                                                           */
/*  int KheGroupMonitorDeviationCount(KHE_GROUP_MONITOR m)                   */
/*                                                                           */
/*  Return the deviations of m (0 in this case).                             */
/*                                                                           */
/*****************************************************************************/

int KheGroupMonitorDeviationCount(KHE_GROUP_MONITOR m)
{
  return 0;
}


/*****************************************************************************/
/*                                                                           */
/*  int KheGroupMonitorDeviation(KHE_GROUP_MONITOR m, int i)                 */
/*                                                                           */
/*  Return the i'th deviation of m.  There are none it's an error.           */
/*                                                                           */
/*****************************************************************************/

int KheGroupMonitorDeviation(KHE_GROUP_MONITOR m, int i)
{
  MAssert(false, "KheGroupMonitorDeviation: i out of range");
  return 0;  /* keep compiler happy */
}


/*****************************************************************************/
/*                                                                           */
/*  char *KheGroupMonitorDeviationDescription(KHE_GROUP_MONITOR m, int i)    */
/*                                                                           */
/*  Return a description of the i'th deviation of m.  There are no           */
/*  deviations so it's an error.                                             */
/*                                                                           */
/*****************************************************************************/

char *KheGroupMonitorDeviationDescription(KHE_GROUP_MONITOR m, int i)
{
  MAssert(false, "KheGroupMonitorDeviationDescription: i out of range");
  return NULL;  /* keep compiler happy */
}


/*****************************************************************************/
/*                                                                           */
/*  Submodule "debug"                                                        */
/*                                                                           */
/*****************************************************************************/

/*****************************************************************************/
/*                                                                           */
/*  void KheGroupMonitorDefectDebug(KHE_GROUP_MONITOR gm,                    */
/*    int verbosity, int indent, FILE *fp)                                   */
/*                                                                           */
/*  Debug print of gm, showing only the defective child monitors.            */
/*                                                                           */
/*****************************************************************************/

void KheGroupMonitorDefectDebug(KHE_GROUP_MONITOR gm,
  int verbosity, int indent, FILE *fp)
{
  int i;  KHE_MONITOR m;
  if( verbosity >= 1 )
  {
    KheSolnMatchingUpdate(gm->soln);
    KheMonitorDebugWithTagBegin((KHE_MONITOR) gm,
      gm->sub_tag_label != NULL ? gm->sub_tag_label : "GroupMonitor",
      indent, fp);
    fprintf(fp, " (sub_tag %d) %d %s", gm->sub_tag, MArraySize(gm->defects),
      MArraySize(gm->defects) == 1 ? "defect" : "defects");
    if( indent >= 0 && verbosity >= 2 && MArraySize(gm->defects) > 0 )
    {
      fprintf(fp, "\n");
      MArrayForEach(gm->defects, &m, &i)
	if( KheMonitorTag(m) == KHE_GROUP_MONITOR_TAG )
	  KheGroupMonitorDefectDebug((KHE_GROUP_MONITOR) m,
	    verbosity, indent + 2, fp);
	else
	  KheMonitorDebug(m, verbosity, indent + 2, fp);
      KheMonitorDebugEnd((KHE_MONITOR) gm, false, indent, fp);
    }
    else
      KheMonitorDebugEnd((KHE_MONITOR) gm, true, indent, fp);
  }
}


/*****************************************************************************/
/*                                                                           */
/*  void KheGroupMonitorDebug(KHE_GROUP_MONITOR gm,                          */
/*    int verbosity, int indent, FILE *fp)                                   */
/*                                                                           */
/*  Debug print of m onto fp with the given verbosity and indent.            */
/*                                                                           */
/*****************************************************************************/

void KheGroupMonitorDebug(KHE_GROUP_MONITOR gm, int verbosity,
  int indent, FILE *fp)
{
  int i;  KHE_MONITOR m;
  if( verbosity >= 1 )
  {
    KheMonitorDebugWithTagBegin((KHE_MONITOR) gm,
      gm->sub_tag_label != NULL ? gm->sub_tag_label : "GroupMonitor",
      indent, fp);
    fprintf(fp, " (sub_tag %d) %d %s", gm->sub_tag,
      MArraySize(gm->child_monitors),
      MArraySize(gm->child_monitors) == 1 ? "child" : "children");
    if( indent >= 0 && verbosity >= 2 && MArraySize(gm->child_monitors) > 0 )
    {
      fprintf(fp, "\n");
      MArrayForEach(gm->child_monitors, &m, &i)
        KheMonitorDebug(m, verbosity, indent + 2, fp);
      KheMonitorDebugEnd((KHE_MONITOR) gm, false, indent, fp);
    }
    else
      KheMonitorDebugEnd((KHE_MONITOR) gm, true, indent, fp);
  }
}
