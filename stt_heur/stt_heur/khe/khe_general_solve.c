
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
/*  FILE:         khe_general_solve.c                                        */
/*  DESCRIPTION:  KheGeneralSolve().                                         */
/*                                                                           */
/*  If you are having trouble compiling this file, try turning off the       */
/*  debug timing by changing the 1 to 0 on line "#define KHE_TIMING 1".      */
/*                                                                           */
/*****************************************************************************/
#include "khe.h"
#include "m.h"

#define KHE_TIMING 1

#if KHE_TIMING
#include <time.h>
#include <sys/time.h>
#endif

#define DEBUG1 1

/*****************************************************************************/
/*                                                                           */
/*  void KheTestTimeOfDayBegin(struct timeval *tv)                           */
/*  float KheTestTimeOfDayEnd(struct timeval tv)                             */
/*                                                                           */
/*  Precede and follow any block of code with these calls, and the time      */
/*  in seconds taken to execute that block will be returned by               */
/*  KheTestTimeOfDayEnd().  Calls on these functions may be nested           */
/*  provided they use distinct tv vars.                                      */
/*                                                                           */
/*  It is also possible to call KheTestTimeOfDayEnd repeatedly, like this:   */
/*                                                                           */
/*    KheTestTimeOfDayBegin(&tv);                                            */
/*    KheTestTimeOfDayEnd(&tv);                                              */
/*    ...                                                                    */
/*    KheTestTimeOfDayEnd(&tv);                                              */
/*                                                                           */
/*  This gives cumulative timing.                                            */
/*                                                                           */
/*****************************************************************************/

#if KHE_TIMING
static void KheTestTimeOfDayBegin(struct timeval *tv)
{
  gettimeofday(tv, NULL);
}
#else
static void KheTestTimeOfDayBegin(int *tv)
{
}
#endif

#if KHE_TIMING
static float KheTestTimeOfDayEnd(struct timeval *tv)
{
  struct timeval end_tv;
  gettimeofday(&end_tv, NULL);
  return (float) (end_tv.tv_sec - tv->tv_sec) +
    (float) (end_tv.tv_usec - tv->tv_usec) / 1000000.0;
}
#else
static float KheTestTimeOfDayEnd(int *tv)
{
  return 0.0;
}
#endif


/*****************************************************************************/
/*                                                                           */
/*  void KheDebugStage(KHE_SOLN soln, char *stage, int verbosity,            */
/*    struct timeval *tv)                                                    */
/*                                                                           */
/*  Print a debug message showing the stage, how much time has been spent    */
/*  so far, and the solution cost.                                           */
/*                                                                           */
/*****************************************************************************/

#if KHE_TIMING
static void KheDebugStage(KHE_SOLN soln, char *stage, int verbosity,
  struct timeval *tv)
{
  fprintf(stderr, "  KheGeneralSolve %s (%.2f secs so far)%s\n",
    stage, KheTestTimeOfDayEnd(tv), soln != NULL ? ":" : "");
  if( soln != NULL )
    KheSolnDebug(soln, verbosity, 2, stderr);
}
#else
static void KheDebugStage(KHE_SOLN soln, char *stage, int verbosity, int tv)
{
  fprintf(stderr, "  KheGeneralSolve %s (untimed)%s\n",
    stage, soln != NULL ? ":" : "");
  if( soln != NULL )
    KheSolnDebug(soln, verbosity, 2, stderr);
}
#endif


/*****************************************************************************/
/*                                                                           */
/*  KHE_SOLN KheGeneralSolve(KHE_SOLN soln)                                  */
/*                                                                           */
/*  Solve soln, assuming that it has just emerged from KheSolnMake.          */
/*                                                                           */
/*****************************************************************************/

bool containsAssignResourcesConst = true;

KHE_SOLN KheGeneralSolve(KHE_SOLN soln)
{
#if KHE_TIMING
  struct timeval tv;
#else
  int tv;
#endif
  int i;  KHE_EVENT junk;  KHE_NODE cycle_node;
  bool with_evenness = false;
  printf("%d\n", containsAssignResourcesConst); fflush(stdout);
  if( DEBUG1 )
  {
    KHE_INSTANCE ins = KheSolnInstance(soln);
    fprintf(stderr, "[ KheGeneralSolve(%p %s) div %d\n", (void *) soln,
      KheInstanceId(ins) == NULL ? "-" : KheInstanceId(ins),
      KheSolnDiversifier(soln));
    KheTestTimeOfDayBegin(&tv);
    KheDebugStage(NULL, "at start", 2, &tv);
  }
  /* split initial cycle meet and make complete representation */
  KheSolnSplitCycleMeet(soln);
  if( !KheSolnMakeCompleteRepresentation(soln, &junk) )
    MAssert(false, "KheGeneralSolve: KheSolnMakeCompleteRepresentation failed");

  /* build task tree, including assigning preassigned resources */
  /* KheSolnAssignPreassignedResources(soln, NULL); */
  KheTaskTreeMake(soln, KHE_TASK_JOB_HARD_PRC | KHE_TASK_JOB_HARD_ASAC,
    false, true, true);

  /* build layer tree */
  cycle_node = KheLayerTreeMake(soln, true, true, true);

  /* attach matching and evenness monitors */
  KheSolnMatchingSetWeight(soln, KheCost(1, 0));
  KheSolnMatchingAddAllWorkloadRequirements(soln);
  KheSolnMatchingAttachAllOrdinaryDemandMonitors(soln);
  if( with_evenness )
  {
    KheSolnSetAllEvennessMonitorWeights(soln, KheCost(0, 5));
    KheSolnAttachAllEvennessMonitors(soln);
  }

  /* assign times */
  if( DEBUG1 )
    KheDebugStage(soln, "before time assignment", 2, &tv);
  KheCycleNodeAssignTimes(cycle_node);
  if( DEBUG1 )
    KheDebugStage(soln, "after time assignment", 2, &tv);

  /* assign resources */
  if(containsAssignResourcesConst) {
    for( i = 0;  i < KheSolnTaskingCount(soln);  i++ )
      KheTaskingAssignResources(KheSolnTasking(soln, i));
    if( DEBUG1 )
      KheDebugStage(soln, "after resource assignment", 2, &tv);
  }
  /* ensure that the solution cost is the official cost */
  KheSolnEnsureOfficialCost(soln);

  /* merge meets (requires split events monitors, hence this placement) */
  KheMergeMeets(soln);

  /* debug soln and return it */
  if( DEBUG1 )
  {
    KheDebugStage(soln, "at end", 2, &tv);
    fprintf(stderr, "] KheGeneralSolve returning\n");
  }
  return soln;
}
