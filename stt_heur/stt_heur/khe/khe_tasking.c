
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
/*  FILE:         khe_tasking.c                                              */
/*  DESCRIPTION:  Taskings                                                   */
/*                                                                           */
/*****************************************************************************/
#include "khe_interns.h"

/*****************************************************************************/
/*                                                                           */
/*  KHE_TASKING                                                              */
/*                                                                           */
/*****************************************************************************/

struct khe_tasking_rec
{
  KHE_SOLN			soln;			/* enclosing soln    */
  KHE_RESOURCE_TYPE		resource_type;		/* optional r type   */
  int				index_in_soln;		/* index in soln     */
  ARRAY_KHE_TASK		tasks;			/* soln tasks        */
  KHE_TASKING			copy;			/* used when copying */
};


/*****************************************************************************/
/*                                                                           */
/*  Submodule "construction and query"                                       */
/*                                                                           */
/*****************************************************************************/

/*****************************************************************************/
/*                                                                           */
/*  KHE_TASKING KheTaskingMake(KHE_SOLN soln, KHE_RESOURCE_TYPE rt)          */
/*                                                                           */
/*  Make a new tasking with these attributes.                                */
/*                                                                           */
/*****************************************************************************/

KHE_TASKING KheTaskingMake(KHE_SOLN soln, KHE_RESOURCE_TYPE rt)
{
  KHE_TASKING res;
  res = KheSolnGetTaskingFromFreeList(soln);
  if( res != NULL )
  {
    MArrayClear(res->tasks);
  }
  else
  {
    MMake(res);
    MArrayInit(res->tasks);
  }
  res->soln = soln;
  res->resource_type = rt;
  KheSolnAddTasking(soln, res, &res->index_in_soln);
  res->copy = NULL;
  return res;
}


/*****************************************************************************/
/*                                                                           */
/*  KHE_SOLN KheTaskingSoln(KHE_TASKING tasking)                             */
/*                                                                           */
/*  Return the soln attribute of tasking.                                    */
/*                                                                           */
/*****************************************************************************/

KHE_SOLN KheTaskingSoln(KHE_TASKING tasking)
{
  return tasking->soln;
}


/*****************************************************************************/
/*                                                                           */
/*  KHE_RESOURCE_TYPE KheTaskingResourceType(KHE_TASKING tasking)            */
/*                                                                           */
/*  Return the resource type of tasking, or NULL if none.                    */
/*                                                                           */
/*****************************************************************************/

KHE_RESOURCE_TYPE KheTaskingResourceType(KHE_TASKING tasking)
{
  return tasking->resource_type;
}


/*****************************************************************************/
/*                                                                           */
/*  KHE_TASKING KheTaskingCopyPhase1(KHE_TASKING tasking)                    */
/*                                                                           */
/*  Carry out Phase 1 of copying tasking.                                    */
/*                                                                           */
/*****************************************************************************/

KHE_TASKING KheTaskingCopyPhase1(KHE_TASKING tasking)
{
  KHE_TASKING copy;  KHE_TASK task;  int i;
  if( tasking->copy == NULL )
  {
    MMake(copy);
    tasking->copy = copy;
    copy->soln = KheSolnCopyPhase1(tasking->soln);
    copy->resource_type = tasking->resource_type;
    copy->index_in_soln = tasking->index_in_soln;
    MArrayInit(copy->tasks);
    MArrayForEach(tasking->tasks, &task, &i)
      MArrayAddLast(copy->tasks, KheTaskCopyPhase1(task));
    copy->copy = NULL;
  }
  return tasking->copy;
}


/*****************************************************************************/
/*                                                                           */
/*  void KheTaskingCopyPhase2(KHE_TASKING tasking)                           */
/*                                                                           */
/*  Carry out Phase 2 of copying tasking.                                    */
/*                                                                           */
/*****************************************************************************/

void KheTaskingCopyPhase2(KHE_TASKING tasking)
{
  if( tasking->copy != NULL )
    tasking->copy = NULL;
}


/*****************************************************************************/
/*                                                                           */
/*  int KheTaskingIndexInSoln(KHE_TASKING tasking)                           */
/*                                                                           */
/*  Return tasking's index number in the solution.                           */
/*                                                                           */
/*****************************************************************************/

int KheTaskingIndexInSoln(KHE_TASKING tasking)
{
  return tasking->index_in_soln;
}


/*****************************************************************************/
/*                                                                           */
/*  void KheTaskingSetIndexInSoln(KHE_TASKING tasking, int index)            */
/*                                                                           */
/*  Set tasking's index number in soln to index.                             */
/*                                                                           */
/*****************************************************************************/

void KheTaskingSetIndexInSoln(KHE_TASKING tasking, int index)
{
  tasking->index_in_soln = index;
}


/*****************************************************************************/
/*                                                                           */
/*  void KheTaskingDelete(KHE_TASKING tasking)                               */
/*                                                                           */
/*  Delete tasking, not including its tasks.                                 */
/*                                                                           */
/*****************************************************************************/

void KheTaskingDelete(KHE_TASKING tasking)
{
  while( MArraySize(tasking->tasks) > 0 )
    KheTaskingDeleteTask(tasking, MArrayLast(tasking->tasks));
  KheSolnDeleteTasking(tasking->soln, tasking);
  KheSolnAddTaskingToFreeList(tasking->soln, tasking);
}


/*****************************************************************************/
/*                                                                           */
/*  void KheTaskingFree(KHE_TASKING tasking)                                 */
/*                                                                           */
/*  Free the memory used by tasking.                                         */
/*                                                                           */
/*****************************************************************************/

void KheTaskingFree(KHE_TASKING tasking)
{
  MArrayFree(tasking->tasks);
  MFree(tasking);
}


/*****************************************************************************/
/*                                                                           */
/*  Submodule "tasks"                                                        */
/*                                                                           */
/*****************************************************************************/

/*****************************************************************************/
/*                                                                           */
/*  void KheTaskingAddTask(KHE_TASKING tasking, KHE_TASK task)               */
/*                                                                           */
/*  Add task to tasking.                                                     */
/*                                                                           */
/*****************************************************************************/

void KheTaskingAddTask(KHE_TASKING tasking, KHE_TASK task)
{
  MAssert(tasking->resource_type == NULL || KheTaskResourceType(task) ==
    tasking->resource_type, "KheTaskingAddTask: task has wrong resource type");
  KheTaskSetTaskingAndIndex(task, tasking, MArraySize(tasking->tasks));
  MArrayAddLast(tasking->tasks, task);
}


/*****************************************************************************/
/*                                                                           */
/*  void KheTaskingDeleteTask(KHE_TASKING tasking, KHE_TASK task)            */
/*                                                                           */
/*  Delete task from tasking.                                                */
/*                                                                           */
/*****************************************************************************/

void KheTaskingDeleteTask(KHE_TASKING tasking, KHE_TASK task)
{
  KHE_TASK tmp;  int index;
  tmp = MArrayRemoveLast(tasking->tasks);
  if( tmp != task )
  {
    index = KheTaskIndexInTasking(task);
    KheTaskSetTaskingAndIndex(tmp, tasking, index);
    MArrayPut(tasking->tasks, index, tmp);
  }
  KheTaskSetTaskingAndIndex(task, NULL, -1);
}


/*****************************************************************************/
/*                                                                           */
/*  int KheTaskingTaskCount(KHE_TASKING tasking)                             */
/*                                                                           */
/*  Return the number of tasks in tasking.                                   */
/*                                                                           */
/*****************************************************************************/

int KheTaskingTaskCount(KHE_TASKING tasking)
{
  return MArraySize(tasking->tasks);
}


/*****************************************************************************/
/*                                                                           */
/*  KHE_TASK KheTaskingTask(KHE_TASKING tasking, int i)                      */
/*                                                                           */
/*  Return the i'th task of tasking.                                         */
/*                                                                           */
/*****************************************************************************/

KHE_TASK KheTaskingTask(KHE_TASKING tasking, int i)
{
  return MArrayGet(tasking->tasks, i);
}


/*****************************************************************************/
/*                                                                           */
/*  void KheTaskingSortForTaskGroups(KHE_TASKING tasking)                    */
/*                                                                           */
/*  Sort the solution resources of the tasks of tasking, and sort the        */
/*  tasks of tasking, to make life easy for building task groups.            */
/*                                                                           */
/*****************************************************************************/

/* ***
void KheTaskingSortForTaskGroups(KHE_TASKING tasking)
{
  KHE_TASK task;  int i;

  ** make sure each separate task is sorted **
  MArrayForEach(tasking->tasks, &task, &i)
    KheTaskSort(task);

  ** sort the tasks into decreasing duration and increasing starting time **
  MArraySort(tasking->tasks, &KheTaskInterchangeableCmp);
}
*** */


/*****************************************************************************/
/*                                                                           */
/*  Submodule "debug"                                                        */
/*                                                                           */
/*****************************************************************************/

/*****************************************************************************/
/*                                                                           */
/*  void KheTaskingDebug(KHE_TASKING tasking, int verbosity, int indent,     */
/*    FILE *fp)                                                              */
/*                                                                           */
/*  Debug print of tasking onto fp with the given verbosity and indent.      */
/*                                                                           */
/*****************************************************************************/

void KheTaskingDebug(KHE_TASKING tasking, int verbosity, int indent, FILE *fp)
{
  KHE_TASK task;  int i;  char *id;
  if( verbosity >= 1 )
  {
    if( indent >= 0 )
      fprintf(fp, "%*s", indent, "");
    id = (tasking->resource_type == NULL ?  "" :
      KheResourceTypeId(tasking->resource_type) == NULL ? "-" :
      KheResourceTypeId(tasking->resource_type));
    fprintf(fp, "[ Soln Tasking %s(%d tasks)", id, MArraySize(tasking->tasks));
    if( indent >= 0 )
    {
      fprintf(fp, "\n");
      if( verbosity >= 2 )
	MArrayForEach(tasking->tasks, &task, &i)
	  KheTaskDebug(task, verbosity, indent + 2, fp);
      fprintf(fp, "%*s]\n", indent, "");
    }
    else
      fprintf(fp, " ]");
  }
}
