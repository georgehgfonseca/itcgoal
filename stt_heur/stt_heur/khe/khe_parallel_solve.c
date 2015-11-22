
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
/*  FILE:         khe_parallel_solve.c                                       */
/*  DESCRIPTION:  KheParallelSolve().  If you can't compile this file,       */
/*                see the makefile for a workaround.                         */
/*                                                                           */
/*****************************************************************************/
#include "khe.h"
#include "m.h"
#include <limits.h>
#include <math.h>
#if KHE_USE_PTHREAD
#include <pthread.h>
#endif

#define DEBUG1 1

/*****************************************************************************/
/*                                                                           */
/*  KHE_PARALLEL_SOLN - one solution, to be solved in parallel.              */
/*                                                                           */
/*****************************************************************************/

typedef struct khe_parallel_soln_rec {
  KHE_SOLN		soln;			/* solution                  */
#if KHE_USE_PTHREAD
  pthread_t		thread;			/* its thread                */
#endif
} *KHE_PARALLEL_SOLN;

typedef MARRAY(KHE_PARALLEL_SOLN) ARRAY_KHE_PARALLEL_SOLN;


/*****************************************************************************/
/*                                                                           */
/*  KHE_PARALLEL_SOLN KheParallelSolnMake(KHE_SOLN soln, int diversifier)    */
/*                                                                           */
/*  Make one parallel solution with these attributes.                        */
/*                                                                           */
/*****************************************************************************/

static KHE_PARALLEL_SOLN KheParallelSolnMake(KHE_SOLN soln, int diversifier)
{
  KHE_PARALLEL_SOLN res;
  MMake(res);
  res->soln = soln;
  KheSolnSetDiversifier(soln, diversifier);
  /* res->thread is undefined here */
  return res;
}


/*****************************************************************************/
/*                                                                           */
/*  void KheParallelSolnStart(KHE_PARALLEL_SOLN ps,                          */
/*    KHE_GENERAL_SOLVER solver)                                             */
/*                                                                           */
/*  Start ps on solver, preferably in parallel.                              */
/*                                                                           */
/*****************************************************************************/

static void KheParallelSolnStart(KHE_PARALLEL_SOLN ps,
  KHE_GENERAL_SOLVER solver)
{
  MAssert(ps->soln != NULL && KheSolnInstance(ps->soln) != NULL,
    "KheParallelSolnStart internal error");
#if KHE_USE_PTHREAD
  pthread_create(&ps->thread, NULL, (void * (*)(void *)) solver, ps->soln);
#else
  ps->soln = solver(ps->soln);
#endif
}


/*****************************************************************************/
/*                                                                           */
/*  void KheParallelSolnStop(KHE_PARALLEL_SOLN ps)                           */
/*                                                                           */
/*  Stop ps on solver (or rather, wait until it stops), preferably in        */
/*  parallel.                                                                */
/*                                                                           */
/*****************************************************************************/

static void KheParallelSolnStop(KHE_PARALLEL_SOLN ps)
{
#if KHE_USE_PTHREAD
  pthread_join(ps->thread, (void **) &ps->soln);
#endif
}


/*****************************************************************************/
/*                                                                           */
/*  void KheParallelSolnDelete(KHE_PARALLEL_SOLN ps)                         */
/*                                                                           */
/*  Delete ps, including its soln if non-NULL.                               */
/*                                                                           */
/*****************************************************************************/

static void KheParallelSolnDelete(KHE_PARALLEL_SOLN ps)
{
  if( ps->soln != NULL )
    KheSolnDelete(ps->soln);
  MFree(ps);
}


/*****************************************************************************/
/*                                                                           */
/*  KHE_COST KheAverageCostByType(ARRAY_KHE_PARALLEL_SOLN *parallel_solns,   */
/*    KHE_MONITOR_TAG tag)                                                   */
/*                                                                           */
/*  Return the average cost of *parallel_solns by tag.                       */
/*                                                                           */
/*****************************************************************************/

static KHE_COST KheAverageCostByType(ARRAY_KHE_PARALLEL_SOLN *parallel_solns,
  KHE_MONITOR_TAG tag, float *defect_count)
{
  int i, num, defects;  KHE_PARALLEL_SOLN ps;  KHE_COST cost;
  int hard_cost, soft_cost;
  defects = hard_cost = soft_cost = 0;
  MArrayForEach(*parallel_solns, &ps, &i)
  {
    cost = KheSolnCostByType(ps->soln, tag, &num);
    hard_cost += KheHardCost(cost);
    soft_cost += KheSoftCost(cost);
    defects += num;
  }
  *defect_count = (float) defects / MArraySize(*parallel_solns);
  return KheCost(hard_cost / MArraySize(*parallel_solns),
    soft_cost / MArraySize(*parallel_solns));
}


/*****************************************************************************/
/*                                                                           */
/*  void KheParallelDebugAverages(ARRAY_KHE_PARALLEL_SOLN *parallel_solns,   */
/*    int verbosity, int indent, FILE *fp)                                   */
/*                                                                           */
/*  Debug print of the average cost of *parallel_solns, by type.             */
/*                                                                           */
/*****************************************************************************/

static void KheParallelDebugAverages(ARRAY_KHE_PARALLEL_SOLN *parallel_solns,
  int verbosity, int indent, FILE *fp)
{
  int tag;  KHE_COST cost, total_cost;  char buff[30];
  float defect_count, total_defect_count;
  MAssert(MArraySize(*parallel_solns) >= 1, "KheParallelDebugAverages!");
  sprintf(buff, "Average of %d solutions", MArraySize(*parallel_solns));
  if( verbosity >= 1 && indent >= 0 )
  {
    fprintf(fp, "%*s%-31s %9s %13s\n", indent, "", buff, "Defects", "Cost");
    fprintf(fp, "%*s-------------------------------------------------------\n",
      indent, "");
    total_cost = 0;  total_defect_count = 0;
    for( tag = 0;  tag < KHE_MONITOR_TAG_COUNT;  tag++ )
    {
      cost = KheAverageCostByType(parallel_solns, tag, &defect_count);
      if( cost != 0 || defect_count != 0 )
	fprintf(fp, "%*s%-34s %6.1f %13.4f\n", indent, "",
	  KheMonitorTagShow(tag), defect_count, KheCostShow(cost));
      total_cost += cost;
      total_defect_count += defect_count;
    }
    fprintf(fp, "%*s-------------------------------------------------------\n",
      indent, "");
    fprintf(fp, "%*s%-34s %6.1f %13.4f\n", indent, "", "Total",
      total_defect_count, KheCostShow(total_cost));
  }
}


/*****************************************************************************/
/*                                                                           */
/*  void KheParallelDebugHistogram(ARRAY_KHE_PARALLEL_SOLN *parallel_solns,  */
/*    int verbosity, int indent, FILE *fp)                                   */
/*                                                                           */
/*  Debug print of histogram of solution costs.                              */
/*                                                                           */
/*****************************************************************************/

static void KheParallelDebugHistogram(ARRAY_KHE_PARALLEL_SOLN *parallel_solns,
  int verbosity, int indent, FILE *fp)
{
  float min_cost, max_cost, cost;
  int min_index, max_index, index, freq, total_freq, i, j, increment, med;
  KHE_PARALLEL_SOLN ps;  ARRAY_INT frequencies;

  if( verbosity >= 1 )
  {
    /* find min and max costs */
    ps = MArrayFirst(*parallel_solns);
    min_cost = max_cost = KheCostShow(KheSolnCost(ps->soln));
    for( i = 1;  i < MArraySize(*parallel_solns);  i++ )
    {
      ps = MArrayGet(*parallel_solns, i);
      cost = KheCostShow(KheSolnCost(ps->soln));
      if( cost < min_cost )
	min_cost = cost;
      else if( cost > max_cost )
	max_cost = cost;
    }

    /* find min and max indexes (rounding the costs down and up) */
    min_index = (int) floor(min_cost);
    max_index = (int) ceil(max_cost);

    /* find increment */
    if( max_index - min_index <= 20 )
      increment = 1;
    else if( max_index - min_index <= 50 )
      increment = 2;
    else if( max_index - min_index <= 100 )
      increment = 5;
    else if( max_index - min_index <= 200 )
      increment = 10;
    else if( max_index - min_index <= 500 )
      increment = 20;
    else if( max_index - min_index <= 1000 )
      increment = 50;
    else
      increment = 200;

    /* round min_index down, and max_index up, to multiples of increment */
    min_index = increment * (min_index / increment);
    max_index = increment * ((max_index + increment - 1) / increment);

    /* initialize one array index for each index, max_index exclusive */
    MArrayInit(frequencies);
    MArrayFill(frequencies, (max_index - min_index) / increment, 0);

    /* fill the array */
    MArrayForEach(*parallel_solns, &ps, &i)
    {
      cost = KheCostShow(KheSolnCost(ps->soln));
      index = ((int) floor(cost) - min_index) / increment;
      MArrayInc(frequencies, index);
    }

    /* print it */
    total_freq = 0;
    med = MArraySize(*parallel_solns) / 2;
    MArrayForEachReverse(frequencies, &freq, &i)
    {
      fprintf(fp, "%*s%7.4f %c ", indent, "", (float) min_index + i*increment,
	total_freq <= med && total_freq + freq >= med ? 'M' : '|');
      for( j = 0;  j < freq;  j++ )
	fprintf(fp, "*");
      fprintf(fp, "\n");
      total_freq += freq;
    }
  }
}


/*****************************************************************************/
/*                                                                           */
/*  KHE_SOLN KheParallelSolve(KHE_SOLN soln, int thread_count,               */
/*    KHE_GENERAL_SOLVER solver)                                             */
/*                                                                           */
/*  Make thread_count - 1 copies of soln, solve the resulting thread_count   */
/*  solutions in parallel, return the best solution, and delete the others.  */
/*                                                                           */
/*****************************************************************************/

KHE_SOLN KheParallelSolve(KHE_SOLN soln, int thread_count,
  KHE_GENERAL_SOLVER solver)
{
  ARRAY_KHE_PARALLEL_SOLN parallel_solns;  KHE_PARALLEL_SOLN ps, best_ps;
  int diversifier, i;  KHE_COST best_cost;  KHE_SOLN res;

  if( DEBUG1 )
    fprintf(stderr, "[ KheParallelSolve(soln, %d, solver) %s\n", thread_count,
      KHE_USE_PTHREAD ? "with threads" : "no threads, so sequential");

  /* make a total of thread_count diversified solutions */
  MAssert(thread_count >= 1, "KheParallelSolve: thread_count (%d) out of range",
    thread_count);
  MArrayInit(parallel_solns);
  diversifier = KheSolnDiversifier(soln);
  for( i = 0;  i < thread_count;  i++ )
  {
    ps = KheParallelSolnMake(i==0 ? soln : KheSolnCopy(soln), diversifier + i);
    MAssert(ps->soln != NULL && KheSolnInstance(ps->soln) != NULL,
      "KheParallelSolve internal error (%d)", i);
    MArrayAddLast(parallel_solns, ps);
  }

  /* start the solve threads on solver */
  MArrayForEach(parallel_solns, &ps, &i)
    KheParallelSolnStart(ps, solver);

  /* stop the solve threads (or rather, wait for them to end) */
  MArrayForEach(parallel_solns, &ps, &i)
    KheParallelSolnStop(ps);

  /* find the best solution */
  best_ps = NULL;
  best_cost = KheCostMax;
  MArrayForEach(parallel_solns, &ps, &i)
  {
    if( DEBUG1 )
      fprintf(stderr, "  soln %d has cost %.4f%s\n",
	i, KheCostShow(KheSolnCost(ps->soln)),
        KheSolnCost(ps->soln) < best_cost ? " (new best)" : "");
    if( KheSolnCost(ps->soln) < best_cost )
    {
      best_ps = ps;
      best_cost = KheSolnCost(ps->soln);
    }
  }

  /* print a histogram of solutions */
  if( DEBUG1 )
  {
    KheParallelDebugHistogram(&parallel_solns, 2, 4, stderr);
    fprintf(stderr, "\n");
    KheParallelDebugAverages(&parallel_solns, 2, 4, stderr);
    fprintf(stderr, "\n");
  }

  /* free everything except the best solution */
  MAssert(best_ps != NULL, "KheParallelSolve internal error");
  res = best_ps->soln;
  best_ps->soln = NULL;
  while( MArraySize(parallel_solns) > 0 )
    KheParallelSolnDelete(MArrayRemoveLast(parallel_solns));
  MArrayFree(parallel_solns);

  /* return the best soln */
  if( DEBUG1 )
  {
    KheSolnDebug(res, 2, 2, stderr);
    fprintf(stderr, "] KheParallelSolve returning (res has cost %.4f)\n",
      KheCostShow(KheSolnCost(res)));
  }
  return res;
}
