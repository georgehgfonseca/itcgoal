
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
/*  FILE:         khe_dev_monitor.c                                          */
/*  DESCRIPTION:  A monitor for a set of deviations                          */
/*                                                                           */
/*****************************************************************************/
#include "khe_interns.h"


/*****************************************************************************/
/*                                                                           */
/*  KHE_DEV_MONITOR - monitors a set of deviations                           */
/*                                                                           */
/*****************************************************************************/

/* *** in khe_interns.h since not a pointer type
typedef struct khe_dev_monitor_rec {
  bool			devs_changed;			** true if changed   **
  ARRAY_INT		devs;				** the deviations    **
} KHE_DEV_MONITOR;
*** */


/*****************************************************************************/
/*                                                                           */
/*  void KheDevMonitorInit(KHE_DEV_MONITOR *dm)                              */
/*                                                                           */
/*  Initialize dev monitor dm.                                               */
/*                                                                           */
/*****************************************************************************/

void KheDevMonitorInit(KHE_DEV_MONITOR *dm)
{
  dm->devs_changed = false;
  MArrayInit(dm->devs);
}


/*****************************************************************************/
/*                                                                           */
/*  void KheDevMonitorCopy(KHE_DEV_MONITOR *target_dm,                       */
/*    KHE_DEV_MONITOR *source_dm)                                            */
/*                                                                           */
/*  Copy source_dm onto target_dm.                                           */
/*                                                                           */
/*****************************************************************************/

void KheDevMonitorCopy(KHE_DEV_MONITOR *target_dm,
  KHE_DEV_MONITOR *source_dm)
{
  int i;
  target_dm->devs_changed = source_dm->devs_changed;
  MArrayInit(target_dm->devs);
  MArrayAppend(target_dm->devs, source_dm->devs, i);
}


/*****************************************************************************/
/*                                                                           */
/*  void KheDevMonitorFree(KHE_DEV_MONITOR *dm)                              */
/*                                                                           */
/*  Free *dm - not the object itself, which is always an embedded struct,    */
/*  but free its two arrays.                                                 */
/*                                                                           */
/*****************************************************************************/

void KheDevMonitorFree(KHE_DEV_MONITOR *dm)
{
  MArrayFree(dm->devs);
}


/*****************************************************************************/
/*                                                                           */
/*  void KheDevMonitorAttach(KHE_DEV_MONITOR *dm, int dev)                   */
/*                                                                           */
/*  Add dev to new_devs and record the fact that there is a change.          */
/*                                                                           */
/*****************************************************************************/

void KheDevMonitorAttach(KHE_DEV_MONITOR *dm, int dev)
{
  dm->devs_changed = true;
  MArrayAddLast(dm->devs, dev);
}


/*****************************************************************************/
/*                                                                           */
/*  void KheDevMonitorDetach(KHE_DEV_MONITOR *dm, int dev)                   */
/*                                                                           */
/*  Detach dev from *dm and record the fact that there has been a change.    */
/*                                                                           */
/*****************************************************************************/

void KheDevMonitorDetach(KHE_DEV_MONITOR *dm, int dev)
{
  int pos, tmp;
  dm->devs_changed = true;
  tmp = MArrayRemoveLast(dm->devs);
  if( tmp != dev )
  {
    if( !MArrayContains(dm->devs, dev, &pos) )
      MAssert(false, "DevMonitorDetach");
    MArrayPut(dm->devs, pos, tmp);
  }
}


/*****************************************************************************/
/*                                                                           */
/*  void KheDevMonitorUpdate(KHE_DEV_MONITOR *dm, int old_dev, int new_dev)  */
/*                                                                           */
/*  Replace old_dev by new_dev.                                              */
/*                                                                           */
/*****************************************************************************/

void KheDevMonitorUpdate(KHE_DEV_MONITOR *dm, int old_dev, int new_dev)
{
  int pos;
  dm->devs_changed = true;
  if( !MArrayContains(dm->devs, old_dev, &pos) )
    MAssert(false, "DevMonitorUpdate");
  MArrayPut(dm->devs, pos, new_dev);
}


/*****************************************************************************/
/*                                                                           */
/*  ARRAY_INT *KheDevMonitorDevs(KHE_DEV_MONITOR *dm)                        */
/*                                                                           */
/*  Return the deviations of dm.                                             */
/*                                                                           */
/*****************************************************************************/

ARRAY_INT *KheDevMonitorDevs(KHE_DEV_MONITOR *dm)
{
  return &(dm->devs);
}


/*****************************************************************************/
/*                                                                           */
/*  bool KheDevMonitorHasChanged(KHE_DEV_MONITOR *dm)                        */
/*                                                                           */
/*  Return true if dm's deviations have changed since the last time it       */
/*  was flushed.                                                             */
/*                                                                           */
/*****************************************************************************/

bool KheDevMonitorHasChanged(KHE_DEV_MONITOR *dm)
{
  return dm->devs_changed;
}


/*****************************************************************************/
/*                                                                           */
/*  void KheDevMonitorFlush(KHE_DEV_MONITOR *dm)                             */
/*                                                                           */
/*  Flush dm.  Since no nodes attach directly to dm, we can't update them;   */
/*  but this code does the rest, resetting devs and marking unchanged.       */
/*                                                                           */
/*****************************************************************************/

void KheDevMonitorFlush(KHE_DEV_MONITOR *dm)
{
  dm->devs_changed = false;
}


/*****************************************************************************/
/*                                                                           */
/*  Submodule "debug"                                                        */
/*                                                                           */
/*****************************************************************************/

/*****************************************************************************/
/*                                                                           */
/*  void KheDevMonitorDebug(KHE_DEV_MONITOR *dm, FILE *fp)                   */
/*                                                                           */
/*  Debug print of *dm onto fp, on one line with no indent or newline.       */
/*                                                                           */
/*****************************************************************************/

void KheDevMonitorDebug(KHE_DEV_MONITOR *dm, FILE *fp)
{
  int i, val;
  if( dm->devs_changed )
    fprintf(fp, "*");
  MArrayForEach(dm->devs, &val, &i)
    fprintf(fp, "%s%d", i > 0 ? ":" : "", val);
}
