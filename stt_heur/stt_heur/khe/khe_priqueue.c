
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
/*  FILE:         khe_priqueue.c                                             */
/*  DESCRIPTION:  Generic priority queue                                     */
/*                                                                           */
/*****************************************************************************/
#include "khe_priqueue.h"
#include "m.h"


/*****************************************************************************/
/*                                                                           */
/*  KHE_PRIQUEUE - the priority queue type                                   */
/*                                                                           */
/*****************************************************************************/

struct khe_priqueue_rec {
  KHE_PRIQUEUE_KEY_FN		key;		/* current key of entry      */
  KHE_PRIQUEUE_INDEX_GET_FN	index_get;	/* current index of entry    */
  KHE_PRIQUEUE_INDEX_SET_FN	index_set;	/* set current index of entry*/
  ARRAY_VOIDP			entries;	/* entries[0] is NULL        */
};


/*****************************************************************************/
/*                                                                           */
/*  KHE_PRIQUEUE KhePriQueueMake(KHE_PRIQUEUE_KEY_FN key,                    */
/*    KHE_PRIQUEUE_INDEX_GET_FN index_get,                                   */
/*    KHE_PRIQUEUE_INDEX_SET_FN index_set)                                   */
/*                                                                           */
/*  Make and return a new priority queue with these attributes.              */
/*                                                                           */
/*****************************************************************************/

KHE_PRIQUEUE KhePriQueueMake(KHE_PRIQUEUE_KEY_FN key,
  KHE_PRIQUEUE_INDEX_GET_FN index_get,
  KHE_PRIQUEUE_INDEX_SET_FN index_set)
{
  KHE_PRIQUEUE res;
  MMake(res);
  res->key = key;
  res->index_get = index_get;
  res->index_set = index_set;
  MArrayInit(res->entries);
  MArrayAddLast(res->entries, NULL);  /* entries[0] is always NULL */
  return res;
}


/*****************************************************************************/
/*                                                                           */
/*  void KhePriQueueDelete(KHE_PRIQUEUE p)                                   */
/*                                                                           */
/*  Delete p.                                                                */
/*                                                                           */
/*****************************************************************************/

void KhePriQueueDelete(KHE_PRIQUEUE p)
{
  MArrayFree(p->entries);
  MFree(p);
}


/*****************************************************************************/
/*                                                                           */
/*  bool KhePriQueueEmpty(KHE_PRIQUEUE p)                                    */
/*                                                                           */
/*  Return true if p is empty, else false.                                   */
/*                                                                           */
/*****************************************************************************/

bool KhePriQueueEmpty(KHE_PRIQUEUE p)
{
  return MArraySize(p->entries) == 1;
}


/*****************************************************************************/
/*                                                                           */
/*  void AddLeaf(KHE_PRIQUEUE p, int pos)                                    */
/*                                                                           */
/*  Add a leaf to p at pos.  The entry is already in p, it just needs        */
/*  heapifying.  This function does nothing if the entry is already in       */
/*  the right place with respect to its parent.                              */
/*                                                                           */
/*****************************************************************************/

static void AddLeaf(KHE_PRIQUEUE p, int pos)
{
  void *x, *y;  int j;  int64_t x_key;
  x = MArrayGet(p->entries, pos);
  x_key = p->key(x);
  j = pos / 2;
  y = MArrayGet(p->entries, j);
  while( j > 0 && p->key(y) > x_key )
  {
    MArrayPut(p->entries, pos, y);
    p->index_set(y, pos);
    pos = j;
    j = pos / 2;
    y = MArrayGet(p->entries, j);
  }
  MArrayPut(p->entries, pos, x);
  p->index_set(x, pos);
}


/*****************************************************************************/
/*                                                                           */
/*  void AddRoot(KHE_PRIQUEUE p, int pos)                                    */
/*                                                                           */
/*  Add a new root to p at pos.  The entry is already in p, it just needs    */
/*  heapifying.  This function does nothing if the entry is already in       */
/*  the right place with respect to its children.                            */
/*                                                                           */
/*****************************************************************************/

static void AddRoot(KHE_PRIQUEUE p, int pos)
{
  void *x, *y;  int j;  int64_t x_key;
  x = MArrayGet(p->entries, pos);
  x_key = p->key(x);
  j = 2 * pos;
  while( j < MArraySize(p->entries) )
  {
    /* find smallest child, y, and its position j */
    if( j < MArraySize(p->entries) - 1 &&
        p->key(MArrayGet(p->entries, j)) > p->key(MArrayGet(p->entries, j+1)) )
      j++;
    y = MArrayGet(p->entries, j);

    /* quit now if smallest child is no smaller */
    if( x_key <= p->key(y) )
      break;

    /* move y up and continue down the tree */
    MArrayPut(p->entries, pos, y);
    p->index_set(y, pos);
    pos = j;
    j = 2 * pos;
  }
  MArrayPut(p->entries, pos, x);
  p->index_set(x, pos);
}


/*****************************************************************************/
/*                                                                           */
/*  void KhePriQueueInsert(KHE_PRIQUEUE p, void *entry)                      */
/*                                                                           */
/*  Insert entry into p.                                                     */
/*                                                                           */
/*****************************************************************************/

void KhePriQueueInsert(KHE_PRIQUEUE p, void *entry)
{
  MArrayAddLast(p->entries, entry);
  AddLeaf(p, MArraySize(p->entries) - 1);
}


/*****************************************************************************/
/*                                                                           */
/*  void *KhePriQueueFindMin(KHE_PRIQUEUE p)                                 */
/*                                                                           */
/*  Assuming p is non-empty, return a minimum entry.                         */
/*                                                                           */
/*****************************************************************************/

void *KhePriQueueFindMin(KHE_PRIQUEUE p)
{
  MAssert(MArraySize(p->entries) > 1, "KhePriQueueFindMin: p is empty");
  return MArrayGet(p->entries, 1);
}


/*****************************************************************************/
/*                                                                           */
/*  void *KhePriQueueDeleteMin(KHE_PRIQUEUE p)                               */
/*                                                                           */
/*  Delete and return a minimum entry.                                       */
/*                                                                           */
/*****************************************************************************/

void *KhePriQueueDeleteMin(KHE_PRIQUEUE p)
{
  void *res, *x;
  MAssert(MArraySize(p->entries) > 1, "KhePriQueueFindMin: p is empty");
  res = MArrayGet(p->entries, 1);
  p->index_set(res, 0);
  x = MArrayRemoveLast(p->entries);
  if( MArraySize(p->entries) > 1 )
  {
    MArrayPut(p->entries, 1, x);
    AddRoot(p, 1);
  }
  return res;
}


/*****************************************************************************/
/*                                                                           */
/*  void KhePriQueueDeleteEntry(KHE_PRIQUEUE p, void *entry)                 */
/*                                                                           */
/*  Delete entry from p; it must be present.                                 */
/*                                                                           */
/*****************************************************************************/

void KhePriQueueDeleteEntry(KHE_PRIQUEUE p, void *entry)
{
  int pos;  void *x;
  pos = p->index_get(entry);
  MAssert(pos >= 1 && pos < MArraySize(p->entries),
    "KhePriQueueDeleteEntry: index returned by index_get out of range");
  MAssert(MArrayGet(p->entries, pos) == entry,
    "KhePriQueueDeleteEntry: index returned by index_get is inconsistent");
  if( pos == MArraySize(p->entries) - 1 )
  {
    /* entry just happens to be last in the array */
    MArrayDropLast(p->entries);
  }
  else
  {
    /* overwrite entry with last element, and do a key update */
    x = MArrayRemoveLast(p->entries);
    MArrayPut(p->entries, pos, x);
    AddRoot(p, pos);
    AddLeaf(p, p->index_get(x));
  }
  p->index_set(entry, 0);
}


/*****************************************************************************/
/*                                                                           */
/*  void KhePriQueueNotifyKeyChange(KHE_PRIQUEUE p, void *entry)             */
/*                                                                           */
/*  Accept a notification that the key of *entry has just changed.  It       */
/*  could increase or decrease.                                              */
/*                                                                           */
/*****************************************************************************/

void KhePriQueueNotifyKeyChange(KHE_PRIQUEUE p, void *entry)
{
  int pos;
  pos = p->index_get(entry);
  MAssert(pos >= 1 && pos < MArraySize(p->entries),
    "KhePriQueueNotifyKeyChange: index returned by index_get out of range");
  MAssert(MArrayGet(p->entries, pos) == entry,
    "KhePriQueueNotifyKeyChange: index returned by index_get is inconsistent");
  AddRoot(p, pos);
  AddLeaf(p, p->index_get(entry));
}


/*****************************************************************************/
/*                                                                           */
/*  Submodule "testing".                                                     */
/*                                                                           */
/*****************************************************************************/

/*****************************************************************************/
/*                                                                           */
/*  KHE_PRIQUEUE_TEST_ENTRY - entry type for testing.                        */
/*                                                                           */
/*****************************************************************************/

typedef struct khe_priqueue_test_entry_rec {
  int index;
  int64_t key;
} *KHE_PRIQUEUE_TEST_ENTRY;


/*****************************************************************************/
/*                                                                           */
/*  int64_t KhePriQueueTestKey(KHE_PRIQUEUE_TEST_ENTRY pte)                  */
/*                                                                           */
/*  Return the key of pte.                                                   */
/*                                                                           */
/*****************************************************************************/

static int64_t KhePriQueueTestKey(KHE_PRIQUEUE_TEST_ENTRY pte)
{
  return pte->key;
}


/*****************************************************************************/
/*                                                                           */
/*  int KhePriQueueTestIndex(KHE_PRIQUEUE_TEST_ENTRY pte)                    */
/*                                                                           */
/*  Return the index of pte.                                                 */
/*                                                                           */
/*****************************************************************************/

static int KhePriQueueTestIndex(KHE_PRIQUEUE_TEST_ENTRY pte)
{
  return pte->index;
}


/*****************************************************************************/
/*                                                                           */
/*  void KhePriQueueTestIndexSet(KHE_PRIQUEUE_TEST_ENTRY pte, int index)     */
/*                                                                           */
/*  Set the index of pte.                                                    */
/*                                                                           */
/*****************************************************************************/

static void KhePriQueueTestIndexSet(KHE_PRIQUEUE_TEST_ENTRY pte, int index)
{
  pte->index = index;
}


/*****************************************************************************/
/*                                                                           */
/*  KHE_PRIQUEUE_TEST_ENTRY KhePriQueueTestEntryMake(int64_t key)            */
/*                                                                           */
/*  Return a new test entry with this key.                                   */
/*                                                                           */
/*****************************************************************************/

static KHE_PRIQUEUE_TEST_ENTRY KhePriQueueTestEntryMake(int64_t key)
{
  KHE_PRIQUEUE_TEST_ENTRY res;
  MMake(res);
  res->index = 0;
  res->key = key;
  return res;
}


/*****************************************************************************/
/*                                                                           */
/*  void KhePriQueueTest(FILE *fp)                                           */
/*                                                                           */
/*  Test this module.                                                        */
/*                                                                           */
/*****************************************************************************/

void KhePriQueueTest(FILE *fp)
{
  KHE_PRIQUEUE p;  KHE_PRIQUEUE_TEST_ENTRY pte;
  fprintf(fp, "[ KhePriQueueTest()\n");
  p = KhePriQueueMake((KHE_PRIQUEUE_KEY_FN) &KhePriQueueTestKey,
    (KHE_PRIQUEUE_INDEX_GET_FN) &KhePriQueueTestIndex,
    (KHE_PRIQUEUE_INDEX_SET_FN) &KhePriQueueTestIndexSet);
  KhePriQueueInsert(p, KhePriQueueTestEntryMake(50));
  KhePriQueueInsert(p, KhePriQueueTestEntryMake(25));
  KhePriQueueInsert(p, KhePriQueueTestEntryMake(60));
  KhePriQueueInsert(p, KhePriQueueTestEntryMake(17));
  KhePriQueueInsert(p, KhePriQueueTestEntryMake(80));
  KhePriQueueInsert(p, (pte = KhePriQueueTestEntryMake(30)));
  pte->key = 40;
  KhePriQueueNotifyKeyChange(p, pte);
  fprintf(fp, "  del %ld\n",
    ((KHE_PRIQUEUE_TEST_ENTRY) KhePriQueueDeleteMin(p))->key);
  fprintf(fp, "  del %ld\n",
    ((KHE_PRIQUEUE_TEST_ENTRY) KhePriQueueDeleteMin(p))->key);
  fprintf(fp, "  del %ld\n",
    ((KHE_PRIQUEUE_TEST_ENTRY) KhePriQueueDeleteMin(p))->key);
  fprintf(fp, "  del %ld\n",
    ((KHE_PRIQUEUE_TEST_ENTRY) KhePriQueueDeleteMin(p))->key);
  fprintf(fp, "  del %ld\n",
    ((KHE_PRIQUEUE_TEST_ENTRY) KhePriQueueDeleteMin(p))->key);
  fprintf(fp, "  del %ld\n",
    ((KHE_PRIQUEUE_TEST_ENTRY) KhePriQueueDeleteMin(p))->key);
  fprintf(fp, "  empty %s\n", KhePriQueueEmpty(p) ? "true" : "false");
  fprintf(fp, "] KhePriQueueTest returning\n");
}
