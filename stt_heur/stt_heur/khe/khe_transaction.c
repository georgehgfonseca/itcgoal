
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
/*  FILE:         khe_transaction.c                                          */
/*  DESCRIPTION:  A transaction                                              */
/*                                                                           */
/*****************************************************************************/
#include "khe_interns.h"

#define KHE_TRANSACTION_ACTIVE -1

/*****************************************************************************/
/*                                                                           */
/*  KHE_TRANSACTION_OP - one operation of a transaction.                     */
/*                                                                           */
/*****************************************************************************/

typedef enum {
  KHE_TRANSACTION_OP_MEET_MAKE,
  KHE_TRANSACTION_OP_MEET_DELETE,
  KHE_TRANSACTION_OP_MEET_SPLIT,
  KHE_TRANSACTION_OP_MEET_MERGE,
  KHE_TRANSACTION_OP_MEET_ASSIGN,
  KHE_TRANSACTION_OP_MEET_UNASSIGN,
  KHE_TRANSACTION_OP_MEET_SET_DOMAIN,

  KHE_TRANSACTION_OP_TASK_MAKE,
  KHE_TRANSACTION_OP_TASK_DELETE,
  KHE_TRANSACTION_OP_TASK_ASSIGN,
  KHE_TRANSACTION_OP_TASK_UNASSIGN,
  KHE_TRANSACTION_OP_TASK_SET_DOMAIN,

  KHE_TRANSACTION_OP_NODE_ADD_PARENT,
  KHE_TRANSACTION_OP_NODE_DELETE_PARENT

} KHE_TRANSACTION_OP_TYPE;

typedef struct khe_transaction_op_rec {
  KHE_TRANSACTION_OP_TYPE	type;			/* operation type    */
  union {
    struct {
      KHE_MEET		res;
    } meet_make;
    struct {
      KHE_MEET		meet1;
      KHE_MEET		meet2;
    } meet_split;
    struct {
      KHE_MEET		meet;
      KHE_MEET		target_meet;
      int		target_offset;
    } meet_assign;
    struct {
      KHE_MEET		meet;
      KHE_MEET		target_meet;
      int		target_offset;
    } meet_unassign;
    struct {
      KHE_MEET	meet;
      KHE_TIME_GROUP	old_tg;
      KHE_TIME_GROUP	new_tg;
    } meet_set_domain;

    struct {
      KHE_TASK	res;
    } task_make;
    struct {
      KHE_TASK		task;
      KHE_TASK		target_task;
    } task_assign;
    struct {
      KHE_TASK		task;
      KHE_TASK		target_task;
    } task_unassign;
    struct {
      KHE_TASK		task;
      KHE_RESOURCE_GROUP old_rg;
      KHE_RESOURCE_GROUP new_rg;
    } task_set_domain;

    struct {
      KHE_NODE		parent_node;
      KHE_NODE		child_node;
    } node_add_parent;
    struct {
      KHE_NODE		parent_node;
      KHE_NODE		child_node;
    } node_delete_parent;
  } u;
} *KHE_TRANSACTION_OP;

typedef MARRAY(KHE_TRANSACTION_OP) ARRAY_KHE_TRANSACTION_OP;


/*****************************************************************************/
/*                                                                           */
/*  KHE_TRANSACTION - a transaction                                          */
/*                                                                           */
/*****************************************************************************/

struct khe_transaction_rec {
  KHE_SOLN			soln;			/* encl soln         */
  bool				loading;		/* loading now       */
  bool				may_undo;		/* undo allowed      */
  bool				may_redo;		/* redo allowed      */
  int				operations_count;	/* no of operations  */
  ARRAY_KHE_TRANSACTION_OP	operations;		/* the operations    */
};


/*****************************************************************************/
/*                                                                           */
/*  Submodule "construction and query"                                       */
/*                                                                           */
/*****************************************************************************/

/*****************************************************************************/
/*                                                                           */
/*  KHE_TRANSACTION KheTransactionMake(KHE_SOLN soln)                        */
/*                                                                           */
/*  Create a new transaction for soln.                                       */
/*                                                                           */
/*****************************************************************************/

KHE_TRANSACTION KheTransactionMake(KHE_SOLN soln)
{
  KHE_TRANSACTION res;
  res = KheSolnGetTransactionFromFreeList(soln);
  if( res == NULL )
  {
    MMake(res);
    res->soln = soln;
    MArrayInit(res->operations);
  }
  res->loading = false;
  return res;
}


/*****************************************************************************/
/*                                                                           */
/*  KHE_SOLN KheTransactionSoln(KHE_TRANSACTION t)                           */
/*                                                                           */
/*  Return the soln that transaction is for.                                 */
/*                                                                           */
/*****************************************************************************/

KHE_SOLN KheTransactionSoln(KHE_TRANSACTION t)
{
  return t->soln;
}


/*****************************************************************************/
/*                                                                           */
/*  void KheTransactionDelete(KHE_TRANSACTION t)                             */
/*                                                                           */
/*  Delete t.                                                                */
/*                                                                           */
/*****************************************************************************/

void KheTransactionDelete(KHE_TRANSACTION t)
{
  MAssert(!t->loading, "KheTransactionDelete called before KheTransactionEnd");
  KheSolnAddTransactionToFreeList(t->soln, t);
}


/*****************************************************************************/
/*                                                                           */
/*  void KheTransactionBegin(KHE_TRANSACTION t)                              */
/*                                                                           */
/*  Begin recording operations into t.                                       */
/*                                                                           */
/*****************************************************************************/

void KheTransactionBegin(KHE_TRANSACTION t)
{
  MAssert(!t->loading,
    "KheTransactionBegin called twice with no intervening KheTransactionEnd");
  t->loading = true;
  t->may_undo = true;
  t->may_redo = true;
  t->operations_count = 0;
  KheSolnBeginTransaction(t->soln, t);
}


/*****************************************************************************/
/*                                                                           */
/*  void KheTransactionMakeEnd(KHE_TRANSACTION t)                            */
/*                                                                           */
/*  End the creation of transaction t.                                       */
/*                                                                           */
/*****************************************************************************/

void KheTransactionEnd(KHE_TRANSACTION t)
{
  MAssert(t->loading,
    "KheTransactionEnd with no matching call to KheTransactionBegin");
  t->loading = false;
  KheSolnEndTransaction(t->soln, t);
}


/*****************************************************************************/
/*                                                                           */
/*  void KheTransactionFree(KHE_TRANSACTION t)                               */
/*                                                                           */
/*  Reclaim the memory used by t (internal use only).                        */
/*                                                                           */
/*****************************************************************************/

void KheTransactionFree(KHE_TRANSACTION t)
{
  while( MArraySize(t->operations) > 0 )
    MFree(MArrayRemoveLast(t->operations));
  MFree(t);
}


/*****************************************************************************/
/*                                                                           */
/*  Submodule "operation loading"                                            */
/*                                                                           */
/*****************************************************************************/

/*****************************************************************************/
/*                                                                           */
/*  KHE_TRANSACTION_OP GetOp(KHE_TRANSACTION t)                              */
/*                                                                           */
/*  Get a new operation object for t, either from t's free list or fresh.    */
/*                                                                           */
/*****************************************************************************/

static KHE_TRANSACTION_OP GetOp(KHE_TRANSACTION t)
{
  KHE_TRANSACTION_OP op;
  if( t->operations_count == MArraySize(t->operations) )
  {
    MMake(op);
    MArrayAddLast(t->operations, op);
  }
  else
    op = MArrayGet(t->operations, t->operations_count);
  t->operations_count++;
  return op;
}


/*****************************************************************************/
/*                                                                           */
/*  void KheTransactionOpMeetMake(KHE_TRANSACTION t, KHE_MEET res)           */
/*                                                                           */
/*  Add a record of a call to KheMeetMake to t.                              */
/*                                                                           */
/*****************************************************************************/

void KheTransactionOpMeetMake(KHE_TRANSACTION t, KHE_MEET res)
{
  KHE_TRANSACTION_OP op;
  op = GetOp(t);
  op->type = KHE_TRANSACTION_OP_MEET_MAKE;
  op->u.meet_make.res = res;
  t->may_redo = false;
}


/*****************************************************************************/
/*                                                                           */
/*  void KheTransactionOpMeetDelete(KHE_TRANSACTION t)                       */
/*                                                                           */
/*  Add a record of a call to KheMeetDelete to t.                            */
/*                                                                           */
/*****************************************************************************/

void KheTransactionOpMeetDelete(KHE_TRANSACTION t)
{
  KHE_TRANSACTION_OP op;
  op = GetOp(t);
  op->type = KHE_TRANSACTION_OP_MEET_DELETE;
  t->may_undo = false;
  t->may_redo = false;
}


/*****************************************************************************/
/*                                                                           */
/*  void KheTransactionOpMeetSplit(KHE_TRANSACTION t, KHE_MEET meet1,        */
/*    KHE_MEET meet2)                                                        */
/*                                                                           */
/*  Add a record of a call to KheMeetSplit to t.                             */
/*                                                                           */
/*****************************************************************************/

void KheTransactionOpMeetSplit(KHE_TRANSACTION t, KHE_MEET meet1,
  KHE_MEET meet2)
{
  KHE_TRANSACTION_OP op;
  op = GetOp(t);
  op->type = KHE_TRANSACTION_OP_MEET_SPLIT;
  op->u.meet_split.meet1 = meet1;
  op->u.meet_split.meet2 = meet2;
  t->may_redo = false;
}


/*****************************************************************************/
/*                                                                           */
/*  void KheTransactionOpMeetMerge(KHE_TRANSACTION t)                        */
/*                                                                           */
/*  Add a record of a call to KheMeetMerge to t.                             */
/*                                                                           */
/*****************************************************************************/

void KheTransactionOpMeetMerge(KHE_TRANSACTION t)
{
  KHE_TRANSACTION_OP op;
  op = GetOp(t);
  op->type = KHE_TRANSACTION_OP_MEET_MERGE;
  t->may_undo = false;
  t->may_redo = false;
}


/*****************************************************************************/
/*                                                                           */
/*  void KheTransactionOpMeetAssign(KHE_TRANSACTION t,                       */
/*    KHE_MEET meet, KHE_MEET target_meet, int target_offset)                */
/*                                                                           */
/*  Add a record of a call to KheMeetAssign to t, unless redundant.          */
/*                                                                           */
/*****************************************************************************/

void KheTransactionOpMeetAssign(KHE_TRANSACTION t,
  KHE_MEET meet, KHE_MEET target_meet, int target_offset)
{
  KHE_TRANSACTION_OP op;

  /* first check whether this new op cancels the immediately preceding op */
  if( t->operations_count > 0 )
  {
    op = MArrayGet(t->operations, t->operations_count - 1);
    if( op->type == KHE_TRANSACTION_OP_MEET_UNASSIGN &&
	op->u.meet_unassign.meet == meet &&
	op->u.meet_unassign.target_meet == target_meet &&
	op->u.meet_unassign.target_offset == target_offset )
    {
      /* this new op cancels preceding op, so remove preceding op and exit */
      t->operations_count--;
      return;
    }
  }

  /* make a new op as usual */
  op = GetOp(t);
  op->type = KHE_TRANSACTION_OP_MEET_ASSIGN;
  op->u.meet_assign.meet = meet;
  op->u.meet_assign.target_meet = target_meet;
  op->u.meet_assign.target_offset = target_offset;
}


/*****************************************************************************/
/*                                                                           */
/*  void KheTransactionOpMeetUnAssign(KHE_TRANSACTION t,                     */
/*    KHE_MEET meet, KHE_MEET target_meet, int target_offset)                */
/*                                                                           */
/*  Add a record of a call to KheMeetUnAssign to t.                          */
/*                                                                           */
/*****************************************************************************/

void KheTransactionOpMeetUnAssign(KHE_TRANSACTION t,
  KHE_MEET meet, KHE_MEET target_meet, int target_offset)
{
  KHE_TRANSACTION_OP op;

  /* first check whether this new op cancels the immediately preceding op */
  if( t->operations_count > 0 )
  {
    op = MArrayGet(t->operations, t->operations_count - 1);
    if( op->type == KHE_TRANSACTION_OP_MEET_ASSIGN &&
	op->u.meet_assign.meet == meet )
    {
      /* this new op cancels preceding op, so remove preceding op and exit */
      /* ***
      if( DEBUG1 && (op->u.meet_assign.target_meet != target_meet ||
          op->u.meet_assign.target_offset != target_offset) )
      {
	fprintf(stderr, "  KheTransactionOpMeetUnAssign failing:\n");
	fprintf(stderr, "  prev op: KheMeetAssign(");
	KheMeetDebug(op->u.meet_assign.meet, 1, -1, stderr);
	fprintf(stderr, ", ");
	KheMeetDebug(op->u.meet_assign.target_meet, 1, -1, stderr);
	fprintf(stderr, ", %d)\n", op->u.meet_assign.target_offset);
	fprintf(stderr, "  this op: KheMeetUnAssign(");
	KheMeetDebug(meet, 1, -1, stderr);
	fprintf(stderr, ") from ");
	KheMeetDebug(target_meet, 1, -1, stderr);
	fprintf(stderr, " at %d\n", target_offset);
      }
      *** */
      MAssert(op->u.meet_assign.target_meet == target_meet &&
	op->u.meet_assign.target_offset == target_offset,
	"KheTransactionOpMeetUnAssign internal error");
      t->operations_count--;
      return;
    }
  }

  /* make a new op as usual */
  op = GetOp(t);
  op->type = KHE_TRANSACTION_OP_MEET_UNASSIGN;
  op->u.meet_unassign.meet = meet;
  op->u.meet_unassign.target_meet = target_meet;
  op->u.meet_unassign.target_offset = target_offset;
}


/*****************************************************************************/
/*                                                                           */
/*  void KheTransactionOpMeetSetDomain(KHE_TRANSACTION t,                    */
/*    KHE_MEET meet, KHE_TIME_GROUP old_tg, KHE_TIME_GROUP new_tg)           */
/*                                                                           */
/*  Add a record of a call to KheMeetSetDomain to t.  NB Both domains        */
/*  may be NULL.                                                             */
/*                                                                           */
/*****************************************************************************/

void KheTransactionOpMeetSetDomain(KHE_TRANSACTION t,
  KHE_MEET meet, KHE_TIME_GROUP old_tg, KHE_TIME_GROUP new_tg)
{
  KHE_TRANSACTION_OP op;

  /* first check whether this new op is mergeable with the preceding op */
  if( t->operations_count > 0 )
  {
    op = MArrayGet(t->operations, t->operations_count - 1);
    if( op->type == KHE_TRANSACTION_OP_MEET_SET_DOMAIN &&
	op->u.meet_set_domain.meet == meet )
    {
      if( op->u.meet_set_domain.old_tg == new_tg )
      {
	/* the new operation cancels the old */
	t->operations_count--;
      }
      else
      {
	/* the new operation is mergeable with the old */
	op->u.meet_set_domain.new_tg = new_tg;
      }
      return;
    }
  }

  /* make a new op as usual */
  op = GetOp(t);
  op->type = KHE_TRANSACTION_OP_MEET_SET_DOMAIN;
  op->u.meet_set_domain.meet = meet;
  op->u.meet_set_domain.old_tg = old_tg;
  op->u.meet_set_domain.new_tg = new_tg;
}


/*****************************************************************************/
/*                                                                           */
/*  void KheTransactionOpTaskMake(KHE_TRANSACTION t, KHE_TASK res)           */
/*                                                                           */
/*  Add a record of a call to KheTaskMake to t.                              */
/*                                                                           */
/*****************************************************************************/

void KheTransactionOpTaskMake(KHE_TRANSACTION t, KHE_TASK res)
{
  KHE_TRANSACTION_OP op;

  op = GetOp(t);
  op->type = KHE_TRANSACTION_OP_TASK_MAKE;
  op->u.task_make.res = res;
  t->may_redo = false;
}


/*****************************************************************************/
/*                                                                           */
/*  void KheTransactionOpTaskDelete(KHE_TRANSACTION t)                       */
/*                                                                           */
/*  Add a record of a call to KheTaskDelete to t.                            */
/*                                                                           */
/*****************************************************************************/

void KheTransactionOpTaskDelete(KHE_TRANSACTION t)
{
  KHE_TRANSACTION_OP op;

  op = GetOp(t);
  op->type = KHE_TRANSACTION_OP_TASK_DELETE;
  t->may_undo = false;
  t->may_redo = false;
}


/*****************************************************************************/
/*                                                                           */
/*  void KheTransactionOpTaskAssign(KHE_TRANSACTION t, KHE_TASK task,        */
/*    KHE_TASK target_task)                                                  */
/*                                                                           */
/*  Add a record of a call to KheTaskAssign to t.                            */
/*                                                                           */
/*****************************************************************************/

void KheTransactionOpTaskAssign(KHE_TRANSACTION t, KHE_TASK task,
  KHE_TASK target_task)
{
  KHE_TRANSACTION_OP op;

  /* first check whether this new op cancels the immediately preceding op */
  if( t->operations_count > 0 )
  {
    op = MArrayGet(t->operations, t->operations_count - 1);
    if( op->type == KHE_TRANSACTION_OP_TASK_UNASSIGN &&
	op->u.task_unassign.task == task &&
	op->u.task_unassign.target_task == target_task )
    {
      /* this new op cancels preceding op, so remove preceding op and exit */
      t->operations_count--;
      return;
    }
  }

  /* make a new op as usual */
  op = GetOp(t);
  op->type = KHE_TRANSACTION_OP_TASK_ASSIGN;
  op->u.task_assign.task = task;
  op->u.task_assign.target_task = target_task;
}


/*****************************************************************************/
/*                                                                           */
/*  void KheTransactionOpTaskUnAssign(KHE_TRANSACTION t, KHE_TASK task,      */
/*    KHE_TASK target_task)                                                  */
/*                                                                           */
/*  Add a record of a call to KheTaskUnAssign to t.                          */
/*                                                                           */
/*****************************************************************************/

void KheTransactionOpTaskUnAssign(KHE_TRANSACTION t, KHE_TASK task,
  KHE_TASK target_task)
{
  KHE_TRANSACTION_OP op;

  /* first check whether this new op cancels the immediately preceding op */
  if( t->operations_count > 0 )
  {
    op = MArrayGet(t->operations, t->operations_count - 1);
    if( op->type == KHE_TRANSACTION_OP_TASK_ASSIGN &&
	op->u.task_assign.task == task )
    {
      /* this new op cancels preceding op, so remove preceding op and exit */
      MAssert(op->u.task_assign.target_task == target_task,
	"KheTransactionOpTaskUnAssign internal error");
      t->operations_count--;
      return;
    }
  }

  /* make a new op as usual */
  op = GetOp(t);
  op->type = KHE_TRANSACTION_OP_TASK_UNASSIGN;
  op->u.task_unassign.task = task;
  op->u.task_unassign.target_task = target_task;
}


/*****************************************************************************/
/*                                                                           */
/*  void KheTransactionOpTaskSetDomain(KHE_TRANSACTION t, KHE_TASK task,     */
/*    KHE_RESOURCE_GROUP old_rg, KHE_RESOURCE_GROUP new_rg)                  */
/*                                                                           */
/*  Add a record of a call to KheTaskSetDomain to t.                         */
/*                                                                           */
/*****************************************************************************/

void KheTransactionOpTaskSetDomain(KHE_TRANSACTION t, KHE_TASK task,
  KHE_RESOURCE_GROUP old_rg, KHE_RESOURCE_GROUP new_rg)
{
  KHE_TRANSACTION_OP op;

  /* first check whether this new op is mergeable with the preceding op */
  if( t->operations_count > 0 )
  {
    op = MArrayGet(t->operations, t->operations_count - 1);
    if( op->type == KHE_TRANSACTION_OP_TASK_SET_DOMAIN &&
	op->u.task_set_domain.task == task )
    {
      if( op->u.task_set_domain.old_rg == new_rg )
      {
	/* the new operation cancels the old */
	t->operations_count--;
      }
      else
      {
	/* the new operation merges with the old */
	op->u.task_set_domain.new_rg = new_rg;
      }
      return;
    }
  }

  /* make a new op as usual */
  op = GetOp(t);
  op->type = KHE_TRANSACTION_OP_TASK_SET_DOMAIN;
  op->u.task_set_domain.task = task;
  op->u.task_set_domain.old_rg = old_rg;
  op->u.task_set_domain.new_rg = new_rg;
}


/*****************************************************************************/
/*                                                                           */
/*  void KheTransactionOpNodeAddParent(KHE_TRANSACTION t,                    */
/*    KHE_NODE child_node, KHE_NODE parent_node)                             */
/*                                                                           */
/*  Add a record of a call to KheNodeAddParent to t.                         */
/*                                                                           */
/*****************************************************************************/

void KheTransactionOpNodeAddParent(KHE_TRANSACTION t,
  KHE_NODE child_node, KHE_NODE parent_node)
{
  KHE_TRANSACTION_OP op;

  op = GetOp(t);
  op->type = KHE_TRANSACTION_OP_NODE_ADD_PARENT;
  op->u.node_add_parent.child_node = child_node;
  op->u.node_add_parent.parent_node = parent_node;
}


/*****************************************************************************/
/*                                                                           */
/*  void KheTransactionOpNodeDeleteParent(KHE_TRANSACTION t,                 */
/*    KHE_NODE child_node, KHE_NODE parent_node)                             */
/*                                                                           */
/*  Add a record of a call to KheNodeDeleteParent to t.                      */
/*                                                                           */
/*****************************************************************************/

void KheTransactionOpNodeDeleteParent(KHE_TRANSACTION t,
  KHE_NODE child_node, KHE_NODE parent_node)
{
  KHE_TRANSACTION_OP op;

  op = GetOp(t);
  op->type = KHE_TRANSACTION_OP_NODE_DELETE_PARENT;
  op->u.node_add_parent.child_node = child_node;
  op->u.node_add_parent.parent_node = parent_node;
}


/*****************************************************************************/
/*                                                                           */
/*  Submodule "undo, redo, and copy"                                         */
/*                                                                           */
/*****************************************************************************/

/*****************************************************************************/
/*                                                                           */
/*  void KheTransactionUndo(KHE_TRANSACTION t)                               */
/*                                                                           */
/*  Undo t.                                                                  */
/*                                                                           */
/*****************************************************************************/

void KheTransactionUndo(KHE_TRANSACTION t)
{
  KHE_TRANSACTION_OP op;  int i;  KHE_MEET junk;
  MAssert(!t->loading,
    "KheTransactionUndo called before KheTransactionMakeEnd");
  MAssert(t->may_undo,
    "KheTransactionUndo prevented by unsuitable operations");
  for( i = t->operations_count - 1;  i >= 0;  i-- )
  {
    op = MArrayGet(t->operations, i);
    switch( op->type )
    {
      case KHE_TRANSACTION_OP_MEET_MAKE:

	KheMeetDelete(op->u.meet_make.res);
	break;

      case KHE_TRANSACTION_OP_MEET_DELETE:

	MAssert(false, "KheTransactionUndo internal error (meet delete)");
	break;

      case KHE_TRANSACTION_OP_MEET_SPLIT:

	if( !KheMeetMerge(op->u.meet_split.meet1,op->u.meet_split.meet2,&junk) )
	  MAssert(false,
	    "KheTransactionUndo: failed to undo KheMeetSplit");
	break;

      case KHE_TRANSACTION_OP_MEET_MERGE:

	MAssert(false, "KheTransactionUndo internal error (meet merge)");
	break;

      case KHE_TRANSACTION_OP_MEET_ASSIGN:

	KheMeetUnAssign(op->u.meet_assign.meet);
	break;

      case KHE_TRANSACTION_OP_MEET_UNASSIGN:

	if( !KheMeetAssign(op->u.meet_unassign.meet,
	      op->u.meet_unassign.target_meet,
	      op->u.meet_unassign.target_offset) )
	  MAssert(false,
	    "KheTransactionUndo: failed to undo KheMeetUnAssign");
	break;

      case KHE_TRANSACTION_OP_MEET_SET_DOMAIN:

	if( !KheMeetSetDomain(op->u.meet_set_domain.meet,
	      op->u.meet_set_domain.old_tg) )
	  MAssert(false,
	    "KheTransactionUndo: failed to undo KheMeetSetDomain");
	break;

      case KHE_TRANSACTION_OP_TASK_MAKE:

        KheTaskDelete(op->u.task_make.res);
	break;

      case KHE_TRANSACTION_OP_TASK_DELETE:

	MAssert(false, "KheTransactionUndo internal error (task delete)");
	break;

      case KHE_TRANSACTION_OP_TASK_ASSIGN:

	KheTaskUnAssign(op->u.task_assign.task);
	break;

      case KHE_TRANSACTION_OP_TASK_UNASSIGN:

	if( !KheTaskAssign(op->u.task_assign.task,
	      op->u.task_assign.target_task) )
	  MAssert(false, "KheTransactionUndo: failed to undo KheTaskUnAssign");
	break;

      case KHE_TRANSACTION_OP_TASK_SET_DOMAIN:

	/* NB each recursive level is recorded, so we undo non-recursively */
	/* and to avoid problems with the invariant, we do so unchecked */
	KheTaskSetDomainUnchecked(op->u.task_set_domain.task,
	  op->u.task_set_domain.old_rg, false);
	break;

      case KHE_TRANSACTION_OP_NODE_ADD_PARENT:

	KheNodeDeleteParent(op->u.node_add_parent.child_node);
	break;

      case KHE_TRANSACTION_OP_NODE_DELETE_PARENT:

	KheNodeAddParent(op->u.node_delete_parent.child_node,
	  op->u.node_delete_parent.parent_node);
	break;

      default:

        MAssert(false, "KheTransactionUndo internal error");
	break;
    }
  }
}


/*****************************************************************************/
/*                                                                           */
/*  void KheTransactionRedo(KHE_TRANSACTION t)                               */
/*                                                                           */
/*  Redo t.                                                                  */
/*                                                                           */
/*****************************************************************************/

void KheTransactionRedo(KHE_TRANSACTION t)
{
  KHE_TRANSACTION_OP op;  int i;
  MAssert(!t->loading,
    "KheTransactionRedo called before KheTransactionMakeEnd");
  MAssert(t->may_redo,
    "KheTransactionRedo prevented by unsuitable operations");
  for( i = 0;  i < t->operations_count;  i++ )
  {
    op = MArrayGet(t->operations, i);
    switch( op->type )
    {
      case KHE_TRANSACTION_OP_MEET_MAKE:

	MAssert(false, "KheTransactionRedo internal error (meet make)");
	break;

      case KHE_TRANSACTION_OP_MEET_DELETE:

	MAssert(false, "KheTransactionRedo internal error (meet delete)");
	break;

      case KHE_TRANSACTION_OP_MEET_SPLIT:

	MAssert(false, "KheTransactionRedo internal error (meet split)");
	break;

      case KHE_TRANSACTION_OP_MEET_MERGE:

	MAssert(false, "KheTransactionRedo internal error (meet merge)");
	break;

      case KHE_TRANSACTION_OP_MEET_ASSIGN:

	if( !KheMeetAssign(op->u.meet_assign.meet,
	      op->u.meet_assign.target_meet, op->u.meet_assign.target_offset) )
	  MAssert(false, "KheTransactionRedo: unsuccessful KheMeetAssign");
	break;

      case KHE_TRANSACTION_OP_MEET_UNASSIGN:

	KheMeetUnAssign(op->u.meet_unassign.meet);
	break;

      case KHE_TRANSACTION_OP_MEET_SET_DOMAIN:

	if( !KheMeetSetDomain(op->u.meet_set_domain.meet,
	      op->u.meet_set_domain.new_tg) )
	  MAssert(false,
	    "KheTransactionRedo: unsuccessful KheMeetSetDomain");
	break;

      case KHE_TRANSACTION_OP_TASK_MAKE:

	MAssert(false, "KheTransactionRedo internal error (task make)");
	break;

      case KHE_TRANSACTION_OP_TASK_DELETE:

	MAssert(false, "KheTransactionRedo internal error (task delete)");
	break;

      case KHE_TRANSACTION_OP_TASK_ASSIGN:

	if( !KheTaskAssign(op->u.task_assign.task,
	      op->u.task_assign.target_task) )
	  MAssert(false, "KheTransactionRedo: unsuccessful KheTaskAssign");
	break;

      case KHE_TRANSACTION_OP_TASK_UNASSIGN:

	KheTaskUnAssign(op->u.task_unassign.task);
	break;

      case KHE_TRANSACTION_OP_TASK_SET_DOMAIN:

	/* NB each recusive level is recorded, so we redo non-recursively */
	/* and to avoid problems with the invariant, we do so unchecked */
	KheTaskSetDomainUnchecked(op->u.task_set_domain.task,
	  op->u.task_set_domain.new_rg, false);
	break;

      case KHE_TRANSACTION_OP_NODE_ADD_PARENT:

	KheNodeAddParent(op->u.node_add_parent.child_node,
	  op->u.node_add_parent.parent_node);
	break;

      case KHE_TRANSACTION_OP_NODE_DELETE_PARENT:

	KheNodeDeleteParent(op->u.node_delete_parent.child_node);
	break;

      default:

        MAssert(false, "KheTransactionRedo internal error");
	break;
    }
  }
}


/*****************************************************************************/
/*                                                                           */
/*  void KheTransactionCopy(KHE_TRANSACTION src_t, KHE_TRANSACTION dst_t)    */
/*                                                                           */
/*  Copy src_t onto dst_t.                                                   */
/*                                                                           */
/*****************************************************************************/

void KheTransactionCopy(KHE_TRANSACTION src_t, KHE_TRANSACTION dst_t)
{
  KHE_TRANSACTION_OP op;  int i;
  KheTransactionBegin(dst_t);
  for( i = 0;  i < src_t->operations_count;  i++ )
  {
    op = MArrayGet(src_t->operations, i);
    switch( op->type )
    {
      case KHE_TRANSACTION_OP_MEET_MAKE:

	KheTransactionOpMeetMake(dst_t, op->u.meet_make.res);
	break;

      case KHE_TRANSACTION_OP_MEET_DELETE:

	KheTransactionOpMeetDelete(dst_t);
	break;

      case KHE_TRANSACTION_OP_MEET_SPLIT:

	KheTransactionOpMeetSplit(dst_t,
	  op->u.meet_split.meet1, op->u.meet_split.meet2);
	break;

      case KHE_TRANSACTION_OP_MEET_MERGE:

	KheTransactionOpMeetMerge(dst_t);
	break;

      case KHE_TRANSACTION_OP_MEET_ASSIGN:

	KheTransactionOpMeetAssign(dst_t, op->u.meet_assign.meet,
	  op->u.meet_assign.target_meet, op->u.meet_assign.target_offset);
	break;

      case KHE_TRANSACTION_OP_MEET_UNASSIGN:

	KheTransactionOpMeetUnAssign(dst_t, op->u.meet_unassign.meet,
	  op->u.meet_unassign.target_meet, op->u.meet_unassign.target_offset);
	break;

      case KHE_TRANSACTION_OP_MEET_SET_DOMAIN:

	KheTransactionOpMeetSetDomain(dst_t, op->u.meet_set_domain.meet,
	  op->u.meet_set_domain.old_tg, op->u.meet_set_domain.new_tg);
	break;

      case KHE_TRANSACTION_OP_TASK_MAKE:

	KheTransactionOpTaskMake(dst_t, op->u.task_make.res);
	break;

      case KHE_TRANSACTION_OP_TASK_DELETE:

	KheTransactionOpTaskDelete(dst_t);
	break;

      case KHE_TRANSACTION_OP_TASK_ASSIGN:

	KheTransactionOpTaskAssign(dst_t,
	  op->u.task_assign.task, op->u.task_assign.target_task);
	break;

      case KHE_TRANSACTION_OP_TASK_UNASSIGN:

	KheTransactionOpTaskUnAssign(dst_t,
	  op->u.task_unassign.task, op->u.task_unassign.target_task);
	break;

      case KHE_TRANSACTION_OP_TASK_SET_DOMAIN:

	KheTransactionOpTaskSetDomain(dst_t, op->u.task_set_domain.task,
	  op->u.task_set_domain.old_rg, op->u.task_set_domain.new_rg);
	break;

      case KHE_TRANSACTION_OP_NODE_ADD_PARENT:

	KheTransactionOpNodeAddParent(dst_t, op->u.node_add_parent.child_node,
	  op->u.node_add_parent.parent_node);
	break;

      case KHE_TRANSACTION_OP_NODE_DELETE_PARENT:

	KheTransactionOpNodeDeleteParent(dst_t,
	  op->u.node_delete_parent.child_node,
	  op->u.node_delete_parent.parent_node);
	break;

      default:

	MAssert(false, "KheTransactionCopy internal error");
	break;
    }
  }
  KheTransactionEnd(dst_t);
}


/*****************************************************************************/
/*                                                                           */
/*  Submodule "debug"                                                        */
/*                                                                           */
/*****************************************************************************/

/*****************************************************************************/
/*                                                                           */
/*  void KheTransactionOpDebug(KHE_TRANSACTION_OP op,                        */
/*    int verbosity, FILE *fp)                                               */
/*                                                                           */
/*  Print one transaction op onto fp with the given verbosity and no extra   */
/*  space.                                                                   */
/*                                                                           */
/*****************************************************************************/

static void KheTransactionOpDebug(KHE_TRANSACTION_OP op,
  int verbosity, FILE *fp)
{
  switch( op->type )
  {
    case KHE_TRANSACTION_OP_MEET_MAKE:

      if( verbosity >= 2 )
      {
	KheMeetDebug(op->u.meet_make.res, 1, -1, fp);
	fprintf(fp, " = ");
      }
      fprintf(fp, "KheMeetMake");
      if( verbosity >= 2 )
	fprintf(fp, "(-, -, -)");
      break;

    case KHE_TRANSACTION_OP_MEET_DELETE:

      fprintf(fp, "KheMeetDelete");
      if( verbosity >= 2 )
	fprintf(fp, "(-)");
      break;

    case KHE_TRANSACTION_OP_MEET_SPLIT:

      fprintf(fp, "MeetSplit");
      if( verbosity >= 2 )
      {
	fprintf(fp, "(-, -, ");
	KheMeetDebug(op->u.meet_split.meet1, 1, -1, fp);
	fprintf(fp, ", ");
	KheMeetDebug(op->u.meet_split.meet2, 1, -1, fp);
	fprintf(fp, ")");
      }
      break;

    case KHE_TRANSACTION_OP_MEET_MERGE:

      fprintf(fp, "MeetMerge");
      if( verbosity >= 2 )
	fprintf(fp, "(-, -, -)");
      break;

    case KHE_TRANSACTION_OP_MEET_ASSIGN:

      fprintf(fp, "MeetAssign");
      if( verbosity >= 2 )
      {
	fprintf(fp, "(");
	KheMeetDebug(op->u.meet_assign.meet, 1, -1, fp);
	fprintf(fp, ", ");
	KheMeetDebug(op->u.meet_assign.target_meet, 1, -1, fp);
	fprintf(fp, ", %d)", op->u.meet_assign.target_offset);
      }
      break;

    case KHE_TRANSACTION_OP_MEET_UNASSIGN:

      fprintf(fp, "MeetUnAssign");
      if( verbosity >= 2 )
      {
	fprintf(fp, "(");
	KheMeetDebug(op->u.meet_assign.meet, 1, -1, fp);
	fprintf(fp, ") was (");
	KheMeetDebug(op->u.meet_assign.target_meet, 1, -1, fp);
	fprintf(fp, ", %d)", op->u.meet_assign.target_offset);
      }
      break;

    case KHE_TRANSACTION_OP_MEET_SET_DOMAIN:

      fprintf(fp, "MeetSetDomain");
      if( verbosity >= 2 )
      {
	fprintf(fp, "(");
	KheMeetDebug(op->u.meet_set_domain.meet, 1, -1, fp);
	fprintf(fp, ", td)");
      }
      break;

    case KHE_TRANSACTION_OP_TASK_MAKE:

      if( verbosity >= 2 )
      {
	KheTaskDebug(op->u.task_make.res, 1, -1, fp);
	fprintf(fp, " = ");
      }
      fprintf(fp, "KheTaskMake");
      if( verbosity >= 2 )
	fprintf(fp, "(-, -, -)");
      break;

    case KHE_TRANSACTION_OP_TASK_DELETE:

      if( verbosity >= 2 )
	fprintf(fp, "KheTaskDelete");
      fprintf(fp, "(-)");
      break;

    case KHE_TRANSACTION_OP_TASK_ASSIGN:

      fprintf(fp, "TaskAssign");
      if( verbosity >= 2 )
      {
	fprintf(fp, "(");
	KheTaskDebug(op->u.task_assign.task, 1, -1, fp);
	fprintf(fp, ", ");
	KheTaskDebug(op->u.task_assign.target_task, 1, -1, fp);
	fprintf(fp, ")");
      }
      break;

    case KHE_TRANSACTION_OP_TASK_UNASSIGN:

      fprintf(fp, "TaskUnAssign");
      if( verbosity >= 2 )
      {
	fprintf(fp, "(");
	KheTaskDebug(op->u.task_unassign.task, 1, -1, fp);
	fprintf(fp, ") was ");
	KheTaskDebug(op->u.task_unassign.target_task, 1, -1, fp);
      }
      break;

    case KHE_TRANSACTION_OP_TASK_SET_DOMAIN:

      fprintf(fp, "TaskSetDomain");
      if( verbosity >= 2 )
      {
	fprintf(fp, "(");
	KheTaskDebug(op->u.task_set_domain.task, 1, -1, fp);
	fprintf(fp, ", ");
	KheResourceGroupDebug(op->u.task_set_domain.new_rg, 1, -1, fp);
	fprintf(fp, ") was ");
	KheResourceGroupDebug(op->u.task_set_domain.old_rg, 1, -1, fp);
      }
      break;

      case KHE_TRANSACTION_OP_NODE_ADD_PARENT:

	fprintf(fp, "NodeAddParent");
	if( verbosity >= 2 )
	{
	  fprintf(fp, "(");
	  KheNodeDebug(op->u.node_add_parent.child_node, 1, -1, fp);
	  fprintf(fp, ", ");
	  KheNodeDebug(op->u.node_add_parent.parent_node, 1, -1, fp);
	  fprintf(fp, ")");
	}
	break;

      case KHE_TRANSACTION_OP_NODE_DELETE_PARENT:

	fprintf(fp, "NodeDeleteParent");
	if( verbosity >= 2 )
	{
	  fprintf(fp, "(");
	  KheNodeDebug(op->u.node_add_parent.child_node, 1, -1, fp);
	  fprintf(fp, ") was ");
	  KheNodeDebug(op->u.node_add_parent.parent_node, 1, -1, fp);
	}
	break;

    default:

      MAssert(false, "KheTransactionOpDebug illegal op type");
      break;
  }
}


/*****************************************************************************/
/*                                                                           */
/*  void KheTransactionDebug(KHE_TRANSACTION t, int verbosity,               */
/*    int indent, FILE *fp)                                                  */
/*                                                                           */
/*  Debug print of t onto fp with the given verbosity and indent.            */
/*                                                                           */
/*****************************************************************************/

void KheTransactionDebug(KHE_TRANSACTION t, int verbosity,
  int indent, FILE *fp)
{
  KHE_TRANSACTION_OP op;  int i;
  if( verbosity >= 1 )
  {
    if( indent >= 0 )
      fprintf(fp, "%*s", indent, "");
    fprintf(fp, "[ Transaction");
    if( verbosity >= 2 )
    {
      if( t->may_undo || t->may_redo || t->loading )
      {
	fprintf(fp, " (");
	if( t->may_undo )
	  fprintf(fp, "may_undo");
	if( t->may_redo )
	  fprintf(fp, "%smay_redo", t->may_undo ? ", " : "");
	if( t->loading )
	  fprintf(fp, "%sloading",
	    t->may_undo || t->may_redo ? ", " : "");
	fprintf(fp, ")");
      }
    }
    for( i = 0;  i < t->operations_count;  i++ )
    {
      op = MArrayGet(t->operations, i);
      if( indent >= 0 )
	fprintf(fp, "\n%*s", indent + 2, "");
      else if( i > 0 )
	fprintf(fp, ", ");
      else
	fprintf(fp, " ");
      KheTransactionOpDebug(op, verbosity, fp);
    }
    if( indent >= 0 )
      fprintf(fp, "\n%*s]\n", indent, "");
    else
      fprintf(fp, " ]");
  }
}
