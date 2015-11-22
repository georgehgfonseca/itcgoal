
/*****************************************************************************/
/*                                                                           */
/*  THE KTS TIMETABLING SYSTEM                                               */
/*  COPYRIGHT (C) 2004, 2008 Jeffrey H. Kingston                             */
/*                                                                           */
/*  Jeffrey H. Kingston (jeff@it.usyd.edu.au)                                */
/*  School of Information Technologies                                       */
/*  The University of Sydney 2006                                            */
/*  AUSTRALIA                                                                */
/*                                                                           */
/*  FILE:         khe_lset.c                                                 */
/*  MODULE:       Sets represented as arbitrary length bit vectors           */
/*                                                                           */
/*****************************************************************************/
#include "khe_lset.h"
#include <limits.h>
#include <assert.h>
#include <stdlib.h>
#include <string.h>

#define INT_BIT (sizeof(unsigned int) * CHAR_BIT)

/*****************************************************************************/
/*                                                                           */
/*  LSET - a set of small non-negative integers                              */
/*                                                                           */
/*  An LSET is represented by a bit vector, preceded by a length field       */
/*  which says how many words the bit vector occupies.  Element i of         */
/*  lset s resides in s->elems[i/INT_BIT] at position i % INT_BIT,           */
/*  counting the least significant bit as position 0.  This is unaffected    */
/*  by whether the machine is big-endian or little-endian.                   */
/*                                                                           */
/*  Although the elements are expected to be small integers (typically       */
/*  up to 100 or so), there is no actual upper limit; the bit vectors        */
/*  are enlarged as required to hold the elements inserted into them.        */
/*  For this reason, LSetInsert and LSetUnion are passed the lsets           */
/*  which receive the result by reference.  This reference will be altered   */
/*  to a new lset if the existing lset is unable to hold the result.         */
/*  Care must be taken not to share lsets under these circumstances.         */
/*                                                                           */
/*****************************************************************************/

struct lset_rec {
  int		length;				/* number of words in elems  */
  unsigned int	elems[1];			/* actually length elems     */
};


/*****************************************************************************/
/*                                                                           */
/*  LSET LSetNew(void)                                                       */
/*                                                                           */
/*  Return a new, empty LSET.                                                */
/*                                                                           */
/*****************************************************************************/

LSET LSetNew(void)
{
  LSET res;
  res = (LSET) malloc(sizeof(struct lset_rec));
  res->length = 1;
  res->elems[0] = 0;
  return res;
}


/*****************************************************************************/
/*                                                                           */
/*  LSET LSetCopy(LSET s)                                                    */
/*                                                                           */
/*  Return a copy of s.                                                      */
/*                                                                           */
/*****************************************************************************/

LSET LSetCopy(LSET s)
{
  LSET res;  int i;
  res = (LSET) malloc(sizeof(struct lset_rec) + (s->length - 1) * sizeof(int));
  res->length = s->length;
  for( i = 0;  i < s->length;  i++ )
    res->elems[i] = s->elems[i];
  return res;
}


/*****************************************************************************/
/*                                                                           */
/*  void LSetShift(LSET s, LSET *res, int k, int lim)                        */
/*                                                                           */
/*  Set *res to the set obtained by adding k (which could be negative) to    */
/*  every element of a, except that no element equal to or larger than       */
/*  lim is added to *res.                                                    */
/*                                                                           */
/*****************************************************************************/

void LSetShift(LSET s, LSET *res, int k, int lim)
{
  int word, word_base_pos, pos;  int new;
  LSetClear(*res);
  word_base_pos = 0;
  for( word = 0;  word < s->length;  word++ )
  {
    if( s->elems[word] )
      for( pos = 0;  pos < INT_BIT;  pos++ )
	if( (s->elems[word] & (1 << pos)) )
	{
	  new = word_base_pos + pos + k;
	  if( new >= 0 && new < lim )
	    LSetInsert(res, new);
	}
    word_base_pos += INT_BIT;
  }
}


/*****************************************************************************/
/*                                                                           */
/*  void LSetClear(LSET s)                                                   */
/*                                                                           */
/*  Clear s.                                                                 */
/*                                                                           */
/*****************************************************************************/

void LSetClear(LSET s)
{
  int i;
  for( i = 0;  i < s->length;  i++ )
    s->elems[i] = 0;
}


/*****************************************************************************/
/*                                                                           */
/*  LSET LSetEnlarge(LSET s, int len)                                        */
/*                                                                           */
/*  Copy s, returning a larger set utilizing len words, then free s.         */
/*                                                                           */
/*****************************************************************************/

static LSET LSetEnlarge(LSET s, int len)
{
  LSET res;  int i;
  res = (LSET) malloc(sizeof(struct lset_rec) + (len - 1) * sizeof(int));
  res->length = len;
  for( i = 0;  i < s->length;  i++ )
    res->elems[i] = s->elems[i];
  for( ; i < len;  i++ )
    res->elems[i] = 0;
  free(s);
  return res;
}


/*****************************************************************************/
/*                                                                           */
/*  void LSetInsert(LSET *s, unsigned int i)                                 */
/*                                                                           */
/*  Add i to s, possibly reallocating s.  The element may already be         */
/*  present, that does not matter.                                           */
/*                                                                           */
/*****************************************************************************/

void LSetInsert(LSET *s, unsigned int i)
{
  int pos = i / INT_BIT;
  if( pos >= (*s)->length )
    *s = LSetEnlarge(*s, pos+1);
  (*s)->elems[pos] |= 1 << (i % INT_BIT);
}


/*****************************************************************************/
/*                                                                           */
/*  void LSetDelete(LSET s, unsigned int i)                                  */
/*                                                                           */
/*  Delete i from s.  It may not be present, that does not matter.           */
/*                                                                           */
/*****************************************************************************/

void LSetDelete(LSET s, unsigned int i)
{
  s->elems[i/INT_BIT] &= ~(1 << (i % INT_BIT));
}


/*****************************************************************************/
/*                                                                           */
/*  void LSetAssign(LSET *target, LSET source)                               */
/*                                                                           */
/*  Update target to be equal to source, possibly reallocating target.       */
/*                                                                           */
/*****************************************************************************/

void LSetAssign(LSET *target, LSET source)
{
  int i;
  if( (*target)->length < source->length )
    *target = LSetEnlarge(*target, source->length);
  for( i = 0;  i < source->length;  i++ )
    (*target)->elems[i] = source->elems[i];
}


/*****************************************************************************/
/*                                                                           */
/*  void LSetUnion(LSET *target, LSET source)                                */
/*                                                                           */
/*  Update target to be the union of itself with source, possibly            */
/*  reallocating target.                                                     */
/*                                                                           */
/*****************************************************************************/

void LSetUnion(LSET *target, LSET source)
{
  int i;
  if( (*target)->length < source->length )
    *target = LSetEnlarge(*target, source->length);
  for( i = 0;  i < source->length;  i++ )
    (*target)->elems[i] |= source->elems[i];
}


/*****************************************************************************/
/*                                                                           */
/*  void LSetIntersection(LSET target, LSET source)                          */
/*                                                                           */
/*  Update target to be the intersection of itself with source.              */
/*                                                                           */
/*****************************************************************************/

void LSetIntersection(LSET target, LSET source)
{
  int i;
  if( target->length <= source->length )
  {
    for( i = 0;  i < target->length;  i++ )
      target->elems[i] &= source->elems[i];
  }
  else
  {
    for( i = 0;  i < source->length;  i++ )
      target->elems[i] &= source->elems[i];
    for( ;  i < target->length;  i++ )
      target->elems[i] = 0;
  }
}


/*****************************************************************************/
/*                                                                           */
/*  void LSetDifference(LSET target, LSET source)                            */
/*                                                                           */
/*  Update target to be the set difference target - source.                  */
/*                                                                           */
/*****************************************************************************/

void LSetDifference(LSET target, LSET source)
{
  int i;
  if( target->length <= source->length )
  {
    for( i = 0;  i < target->length;  i++ )
      target->elems[i] &= ~source->elems[i];
  }
  else
  {
    for( i = 0;  i < source->length;  i++ )
      target->elems[i] &= ~source->elems[i];
  }
}


/*****************************************************************************/
/*                                                                           */
/*  bool LSetEmpty(LSET s)                                                   */
/*                                                                           */
/*  Return true if s is empty.                                               */
/*                                                                           */
/*****************************************************************************/

bool LSetEmpty(LSET s)
{
  int i;
  for( i = 0;  i < s->length;  i++ )
    if( s->elems[i] != 0 )
      return false;
  return true;
}


/*****************************************************************************/
/*                                                                           */
/*  bool LSetEqual(LSET s1, LSET s2)                                         */
/*                                                                           */
/*  Return true if these two sets are equal.                                 */
/*                                                                           */
/*****************************************************************************/

bool LSetEqual(LSET s1, LSET s2)
{
  int i;
  if( s1->length <= s2->length )
  {
    for( i = 0;  i < s1->length;  i++ )
      if( s1->elems[i] != s2->elems[i] )
	return false;
    for( ;  i < s2->length;  i++ )
      if( s2->elems[i] != 0 )
	return false;
  }
  else
  {
    for( i = 0;  i < s2->length;  i++ )
      if( s2->elems[i] != s1->elems[i] )
	return false;
    for( ;  i < s1->length;  i++ )
      if( s1->elems[i] != 0 )
	return false;
  }
  return true;
}


/*****************************************************************************/
/*                                                                           */
/*  bool LSetSubset(LSET s1, LSET s2)                                        */
/*                                                                           */
/*  Return true if s1 is a subset of s2.                                     */
/*                                                                           */
/*****************************************************************************/

bool LSetSubset(LSET s1, LSET s2)
{
  int i;
  if( s1->length <= s2->length )
  {
    for( i = 0;  i < s1->length;  i++ )
      if( s1->elems[i] & ~s2->elems[i] )
	return false;
  }
  else
  {
    for( i = 0;  i < s2->length;  i++ )
      if( s1->elems[i] & ~s2->elems[i] )
	return false;
    for( ;  i < s1->length;  i++ )
      if( s1->elems[i] != 0 )
	return false;
  }
  return true;
}


/*****************************************************************************/
/*                                                                           */
/*  bool LSetDisjoint(LSET s1, LSET s2)                                      */
/*                                                                           */
/*  Return true if s1 and s2 are disjoint; that is, if they have an empty    */
/*  intersection.                                                            */
/*                                                                           */
/*****************************************************************************/

bool LSetDisjoint(LSET s1, LSET s2)
{
  int i, len;
  len = s1->length <= s2->length ? s1->length : s2->length;
  for( i = 0;  i < len;  i++ )
    if( s1->elems[i] & s2->elems[i] )
      return false;
  return true;
}


/*****************************************************************************/
/*                                                                           */
/*  bool LSetDifferenceDisjoint(LSET s1a, LSET s1b, LSET s2)                 */
/*                                                                           */
/*  Return true if (s1a - s1b) and s2 are disjoint; that is, if there        */
/*  exists no element which lies in s1a and s2 but not in s1b.               */
/*                                                                           */
/*****************************************************************************/

/* ***
bool LSetDifferenceDisjoint(LSET s1a, LSET s1b, LSET s2)
{
  int i, len;
  len = s1a->length <= s2->length ? s1a->length : s2->length;
  if( len <= s1b->length )
  {
    for( i = 0;  i < len;  i++ )
      if( s1a->elems[i] & ~s1b->elems[i] & s2->elems[i] )
	return false;
  }
  else
  {
    for( i = 0;  i < s1b->length;  i++ )
      if( s1a->elems[i] & ~s1b->elems[i] & s2->elems[i] )
	return false;
    for( ;  i < len;  i++ )
      if( s1a->elems[i] & s2->elems[i] )
	return false;
  }
  return true;
}
*** */


/*****************************************************************************/
/*                                                                           */
/*  bool LSetIntersectionEqual(LSET s1a, LSET s1b, LSET s2)                  */
/*                                                                           */
/*  Return true if the intersection of s1a with s1b equals s2.               */
/*                                                                           */
/*****************************************************************************/

/* *** works but not currently used
bool LSetIntersectionEqual(LSET s1a, LSET s1b, LSET s2)
{
  int s1len, i;
  s1len = s1a->length <= s1b->length ? s1a->length : s1b->length;
  if( s1len < s2->length )
  {
    for( i = 0;  i < s1len;  i++ )
      if( (s1a->elems[i] & s1b->elems[i]) != s2->elems[i] )
	return false;
    for( ;  i < s2->length;  i++ )
      if( s2->elems[i] != 0 )
	return false;
  }
  else
  {
    for( i = 0;  i < s2->length;  i++ )
      if( (s1a->elems[i] & s1b->elems[i]) != s2->elems[i] )
	return false;
    for( ;  i < s1len;  i++ )
      if( (s1a->elems[i] & s1b->elems[i]) != 0 )
	return false;
  }
  return true;
}
*** */


/*****************************************************************************/
/*                                                                           */
/*  bool LSetContains(LSET s, unsigned int i)                                */
/*                                                                           */
/*  Return true if s contains i.                                             */
/*                                                                           */
/*****************************************************************************/

bool LSetContains(LSET s, unsigned int i)
{
  int pos = i / INT_BIT;
  return pos < s->length && (s->elems[pos] & (1 << (i % INT_BIT)));
}


/*****************************************************************************/
/*                                                                           */
/*  int LSetMin(LSET s)                                                      */
/*                                                                           */
/*  Return the minimum element of s, assuming s is non-empty.                */
/*                                                                           */
/*  Implementation note 1.  This function searches the words of s for the    */
/*  first non-zero word, then the bytes of that word for the first non-zero  */
/*  byte, and finally uses a table lookup to find the first non-zero bit     */
/*  of that byte.  Processors and languages should support this better!      */
/*                                                                           */
/*  Implementation note 2.  The table for finding the first non-zero bit     */
/*  of a byte was generated by a separate program I wrote (function          */
/*  LSetGenTables at the end of this file).  It can be initialized           */
/*  statically, and it is initialized statically, for the very good          */
/*  reason that doing it dynamically could go wrong with multi-threading.    */
/*                                                                           */
/*****************************************************************************/

unsigned int LSetMin(LSET s)
{
  int i, j, byte;
  static char first_nonzero_bit[1 << CHAR_BIT] = {
    8, 0, 1, 0, 2, 0, 1, 0, 3, 0, 1, 0, 2, 0, 1, 0,
    4, 0, 1, 0, 2, 0, 1, 0, 3, 0, 1, 0, 2, 0, 1, 0,
    5, 0, 1, 0, 2, 0, 1, 0, 3, 0, 1, 0, 2, 0, 1, 0,
    4, 0, 1, 0, 2, 0, 1, 0, 3, 0, 1, 0, 2, 0, 1, 0,
    6, 0, 1, 0, 2, 0, 1, 0, 3, 0, 1, 0, 2, 0, 1, 0,
    4, 0, 1, 0, 2, 0, 1, 0, 3, 0, 1, 0, 2, 0, 1, 0,
    5, 0, 1, 0, 2, 0, 1, 0, 3, 0, 1, 0, 2, 0, 1, 0,
    4, 0, 1, 0, 2, 0, 1, 0, 3, 0, 1, 0, 2, 0, 1, 0,
    7, 0, 1, 0, 2, 0, 1, 0, 3, 0, 1, 0, 2, 0, 1, 0,
    4, 0, 1, 0, 2, 0, 1, 0, 3, 0, 1, 0, 2, 0, 1, 0,
    5, 0, 1, 0, 2, 0, 1, 0, 3, 0, 1, 0, 2, 0, 1, 0,
    4, 0, 1, 0, 2, 0, 1, 0, 3, 0, 1, 0, 2, 0, 1, 0,
    6, 0, 1, 0, 2, 0, 1, 0, 3, 0, 1, 0, 2, 0, 1, 0,
    4, 0, 1, 0, 2, 0, 1, 0, 3, 0, 1, 0, 2, 0, 1, 0,
    5, 0, 1, 0, 2, 0, 1, 0, 3, 0, 1, 0, 2, 0, 1, 0,
    4, 0, 1, 0, 2, 0, 1, 0, 3, 0, 1, 0, 2, 0, 1, 0};
  for( i = 0;  i < s->length;  i++ )
    if( s->elems[i] != 0 )
    {
      for( j = 0;  j < INT_BIT;  j += CHAR_BIT )
      {
	byte = (s->elems[i] >> j) & ~(~0 << CHAR_BIT);
	if( byte != 0 )
	  return i * INT_BIT + j + first_nonzero_bit[byte];
      }
      assert(false);
    }
  assert(false);
  return 0;  /* keep compiler happy */
}


/*****************************************************************************/
/*                                                                           */
/*  unsigned int LSetMax(LSET s)                                             */
/*                                                                           */
/*  Return the maximum element of s, assuming s is non-empty.                */
/*  This function is implemented similarly to LSetMax (q.v.).                */
/*                                                                           */
/*****************************************************************************/

unsigned int LSetMax(LSET s)
{
  int i, j, byte;
  static char last_nonzero_bit[1 << CHAR_BIT] = {
    8, 0, 1, 1, 2, 2, 2, 2, 3, 3, 3, 3, 3, 3, 3, 3,
    4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4,
    5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5,
    5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5,
    6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6,
    6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6,
    6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6,
    6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6,
    7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7,
    7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7,
    7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7,
    7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7,
    7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7,
    7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7,
    7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7,
    7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7};
  for( i = s->length - 1;  i >= 0;  i-- )
    if( s->elems[i] != 0 )
    {
      for( j = INT_BIT - CHAR_BIT;  j >= 0;  j -= CHAR_BIT )
      {
	byte = (s->elems[i] >> j) & ~(~0 << CHAR_BIT);
	if( byte != 0 )
	  return i * INT_BIT + j + last_nonzero_bit[byte];
      }
      assert(false);
    }
  assert(false);
  return 0;  /* keep compiler happy */
}


/*****************************************************************************/
/*                                                                           */
/*  int LSetLexicalCmp(LSET s1, LSET s2)                                     */
/*                                                                           */
/*  Return an integer which is negative, zero, or positive accordingly       */
/*  as s1 is less than, equal to, or greater than s2, in lexicographical     */
/*  order.                                                                   */
/*                                                                           */
/*  Implementation note.  It would be more efficient to use unsigned         */
/*  word-length comparisons, but efficiency is not currently needed for      */
/*  this function and the idea is too frightening.                           */
/*                                                                           */
/*****************************************************************************/

int LSetLexicalCmp(LSET s1, LSET s2)
{
  int i, len;
  len = INT_BIT * (s1->length <= s2->length ? s2->length : s1->length);
  for( i = 0;  i < len;  i++ )
  {
    if( LSetContains(s1, i) )
    {
      if( !LSetContains(s2, i) )
	return 1;
    }
    else if( LSetContains(s2, i) )
      return -1;
  }
  return 0;
}


/*****************************************************************************/
/*                                                                           */
/*  ARRAY_INT64 LSetToIntArray(LSET s)                                       */
/*                                                                           */
/*  Convert s to a static array of integers.                                 */
/*                                                                           */
/*****************************************************************************/

/* *** not currently used
ARRAY_INT64 LSetToIntArray(LSET s)
{
  int i;
  ARRAY_INT64 res = NULL;
  ArrayFresh(res);
  for( i = 0;  i < s->length * INT_BIT;  i++ )
    if( LSetContains(s, i) )
      ArrayAddLast(res, i);
  return res;
}
*** */


/*****************************************************************************/
/*                                                                           */
/*  void LSetSelect(LSET s, ARRAY_VOIDP *select_from, ARRAY_VOIDP *add_to)   */
/*                                                                           */
/*  Add to add_to each element of select_from whose index is in s.           */
/*                                                                           */
/*****************************************************************************/

/* ***
void LSetSelect(LSET s, ARRAY_VOIDP *select_from, ARRAY_VOIDP *add_to)
{
  int word, word_base_pos, pos;
  word_base_pos = 0;
  for( word = 0;  word < s->length;  word++ )
  {
    if( s->elems[word] )
      for( pos = 0;  pos < INT_BIT;  pos++ )
	if( s->elems[word] & (1 << pos) )
	  MArrayAddLast(*add_to, MArrayGet(*select_from, word_base_pos + pos));
    word_base_pos += INT_BIT;
  }
}
*** */


/*****************************************************************************/
/*                                                                           */
/*  void LSetExpand(LSET s, ARRAY_SHORT *add_to)                             */
/*                                                                           */
/*  Add to add_to each index in s.                                           */
/*                                                                           */
/*****************************************************************************/

void LSetExpand(LSET s, ARRAY_SHORT *add_to)
{
  short word, word_base_pos, pos;
  word_base_pos = 0;
  for( word = 0;  word < s->length;  word++ )
  {
    if( s->elems[word] )
      for( pos = 0;  pos < INT_BIT;  pos++ )
	if( s->elems[word] & (1 << pos) )
	  MArrayAddLast(*add_to, word_base_pos + pos);
    word_base_pos += INT_BIT;
  }
}


/*****************************************************************************/
/*                                                                           */
/*  void LSetFree(LSET s)                                                    */
/*                                                                           */
/*  Free s.                                                                  */
/*                                                                           */
/*****************************************************************************/

void LSetFree(LSET s)
{
  free(s);
}


/*****************************************************************************/
/*                                                                           */
/*  char *LSetShow(LSET s)                                                   */
/*                                                                           */
/*  Static display of s.                                                     */
/*                                                                           */
/*****************************************************************************/
#define end_buff &buff[bp][strlen(buff[bp])]

static void show_interval(char *buff, bool first, int from, int to)
{
  if( from == to )
    sprintf(buff, "%s%d", first ? "" : ", ", from);
  else
    sprintf(buff, "%s%d-%d", first ? "" : ", ", from, to);
}

typedef enum { INTERVAL_INSIDE, INTERVAL_OUTSIDE } INTERVAL_STATE;
char *LSetShow(LSET s)
{
  static char buff[4][200];
  static int bp = 0;
  int i, card, start_interval;  bool first;  INTERVAL_STATE state;
  bp = (bp + 1) % 4;
  card = s->length * INT_BIT;
  sprintf(buff[bp], "{");
  first = true;
  state = INTERVAL_OUTSIDE;
  start_interval = 0;  /* really undefined */
  for( i = 0;  i < card;  i++ ) switch( state )
  {
    case INTERVAL_INSIDE:

      if( !LSetContains(s, i) )
      {
	show_interval(end_buff, first, start_interval, i-1);
	first = false;
	state = INTERVAL_OUTSIDE;
      }
      break;

    case INTERVAL_OUTSIDE:

      if( LSetContains(s, i) )
      {
	start_interval = i;
	state = INTERVAL_INSIDE;
      }
      break;
  }
  if( state == INTERVAL_INSIDE )
    show_interval(end_buff, first, start_interval, i-1);
  sprintf(end_buff, "}");
  return buff[bp];
}


/*****************************************************************************/
/*                                                                           */
/*  char *show_bool(bool val)                                                */
/*                                                                           */
/*  Return a string representation of val.                                   */
/*                                                                           */
/*****************************************************************************/

static char *show_bool(bool val)
{
  return val ? "true" : "false";
}


/*****************************************************************************/
/*                                                                           */
/*  void LSetGenerateTables(FILE *fp)                                        */
/*                                                                           */
/*  Generate the tables that given the index of the first and last non-zero  */
/*  bit of a given non-zero byte.                                            */
/*                                                                           */
/*****************************************************************************/

/* *** no longer needed now that tables have been generated
static int LSetFirstNonZeroBit(int byte)
{
  int i;
  for( i = 0;  i < CHAR_BIT;  i++ )
    if( byte & (1 << i) )
      return i;
  return CHAR_BIT;
}

static int LSetLastNonZeroBit(int byte)
{
  int i;
  for( i = CHAR_BIT - 1;  i >= 0;  i-- )
    if( byte & (1 << i) )
      return i;
  return CHAR_BIT;
}

static void LSetGenerateTables(FILE *fp)
{
  int i, j;

  ** generate first_nonzero_bit **
  fprintf(fp, "static char first_nonzero_bit[1 << CHAR_BIT] = {\n");
  for( i = 0;  i < (1 << CHAR_BIT);  i += (1 << (CHAR_BIT/2)) )
  {
    fprintf(fp, "  ");
    for( j = 0;  j < (1 << (CHAR_BIT/2));  j++ )
    {
      fprintf(fp, "%d%s", LSetFirstNonZeroBit(i + j),
	i + j + 1 == (1 << CHAR_BIT) ? "};\n" :
	j + 1 == (1 << (CHAR_BIT/2)) ? ",\n" : ", ");
    }
  }

  ** generate last_nonzero_bit **
  fprintf(fp, "static char last_nonzero_bit[1 << CHAR_BIT] = {\n");
  for( i = 0;  i < (1 << CHAR_BIT);  i += (1 << (CHAR_BIT/2)) )
  {
    fprintf(fp, "  ");
    for( j = 0;  j < (1 << (CHAR_BIT/2));  j++ )
    {
      fprintf(fp, "%d%s", LSetLastNonZeroBit(i + j),
	i + j + 1 == (1 << CHAR_BIT) ? "};\n" :
	j + 1 == (1 << (CHAR_BIT/2)) ? ",\n" : ", ");
    }
  }
}
*** */


/*****************************************************************************/
/*                                                                           */
/*  void LSetTest(FILE *fp)                                                  */
/*                                                                           */
/*  Test this module.                                                        */
/*                                                                           */
/*****************************************************************************/

void LSetTest(FILE *fp)
{
  LSET s0, s1, s2, s3, cs0, cs1, cs2, cs3;
  fprintf(fp, "[ LSetTest()\n");

  /* set s0 to the empty set */
  s0 = LSetNew();
  fprintf(fp, "  s0 := %s\n", LSetShow(s0));

  /* set s1 to {0, 10, 20, 30, 40, 50, 60, 70, 80, 90} */
  s1 = LSetNew();
  LSetInsert(&s1,  0);
  LSetInsert(&s1, 10);
  LSetInsert(&s1, 20);
  LSetInsert(&s1, 30);
  LSetInsert(&s1, 40);
  LSetInsert(&s1, 50);
  LSetInsert(&s1, 60);
  LSetInsert(&s1, 70);
  LSetInsert(&s1, 80);
  LSetInsert(&s1, 90);
  fprintf(fp, "  s1 := %s\n", LSetShow(s1));

  /* set s2 to {10, 20, 30} */
  s2 = LSetNew();
  LSetInsert(&s2, 10);
  LSetInsert(&s2, 20);
  LSetInsert(&s2, 30);
  fprintf(fp, "  s2 := %s\n", LSetShow(s2));

  /* set s3 to {80, 85, 90, 95} */
  s3 = LSetNew();
  LSetInsert(&s3, 80);
  LSetInsert(&s3, 85);
  LSetInsert(&s3, 90);
  LSetInsert(&s3, 95);
  fprintf(fp, "  s3 := %s\n", LSetShow(s3));
  fprintf(fp, "\n");

  /* test empty */
  fprintf(fp, "  LSetEmpty(s0) == %s\n", show_bool(LSetEmpty(s0)));
  fprintf(fp, "  LSetEmpty(s1) == %s\n", show_bool(LSetEmpty(s1)));
  fprintf(fp, "\n");

  /* test equal */
  fprintf(fp, "  LSetEqual(s0, s0) == %s\n", show_bool(LSetEqual(s0, s0)));
  fprintf(fp, "  LSetEqual(s1, s1) == %s\n", show_bool(LSetEqual(s1, s1)));
  fprintf(fp, "  LSetEqual(s1, s2) == %s\n", show_bool(LSetEqual(s1, s2)));
  fprintf(fp, "  LSetEqual(s2, s3) == %s\n", show_bool(LSetEqual(s2, s3)));
  fprintf(fp, "\n");

  /* test subset */
  fprintf(fp, "  LSetSubset(s0, s0) == %s\n", show_bool(LSetSubset(s0, s0)));
  fprintf(fp, "  LSetSubset(s1, s1) == %s\n", show_bool(LSetSubset(s1, s1)));
  fprintf(fp, "  LSetSubset(s0, s1) == %s\n", show_bool(LSetSubset(s0, s1)));
  fprintf(fp, "  LSetSubset(s1, s0) == %s\n", show_bool(LSetSubset(s1, s0)));
  fprintf(fp, "  LSetSubset(s1, s2) == %s\n", show_bool(LSetSubset(s1, s2)));
  fprintf(fp, "  LSetSubset(s2, s1) == %s\n", show_bool(LSetSubset(s2, s1)));
  fprintf(fp, "  LSetSubset(s2, s3) == %s\n", show_bool(LSetSubset(s2, s3)));
  fprintf(fp, "  LSetSubset(s3, s2) == %s\n", show_bool(LSetSubset(s3, s2)));
  fprintf(fp, "\n");

  /* test disjoint */
  fprintf(fp,"  LSetDisjoint(s0, s0) == %s\n",show_bool(LSetDisjoint(s0,s0)));
  fprintf(fp,"  LSetDisjoint(s1, s1) == %s\n",show_bool(LSetDisjoint(s1,s1)));
  fprintf(fp,"  LSetDisjoint(s1, s2) == %s\n",show_bool(LSetDisjoint(s1,s2)));
  fprintf(fp,"  LSetDisjoint(s2, s3) == %s\n",show_bool(LSetDisjoint(s2,s3)));
  fprintf(fp,"\n");

  /* test contains */
  fprintf(fp,"  LSetContains(s0, 0) == %s\n", show_bool(LSetContains(s0, 0)));
  fprintf(fp,"  LSetContains(s1, 0) == %s\n", show_bool(LSetContains(s1, 0)));
  fprintf(fp,"  LSetContains(s1, 3) == %s\n", show_bool(LSetContains(s1, 3)));
  fprintf(fp,"  LSetContains(s3, 94) == %s\n",show_bool(LSetContains(s3,94)));
  fprintf(fp,"  LSetContains(s3, 95) == %s\n",show_bool(LSetContains(s3,95)));
  fprintf(fp,"  LSetContains(s3, 96) == %s\n",show_bool(LSetContains(s3,96)));
  fprintf(fp,"\n");

  /* test copy */
  cs0 = LSetCopy(s0);
  fprintf(fp, "  LSetCopy(s0) = %s\n", LSetShow(cs0));
  cs1 = LSetCopy(s1);
  fprintf(fp, "  LSetCopy(s1) = %s\n", LSetShow(cs1));
  cs2 = LSetCopy(s2);
  fprintf(fp, "  LSetCopy(s2) = %s\n", LSetShow(cs2));
  cs3 = LSetCopy(s3);
  fprintf(fp, "  LSetCopy(s3) = %s\n", LSetShow(cs3));
  fprintf(fp, "\n");

  /* test union */
  LSetUnion(&cs1, cs0);
  fprintf(fp, "  LSetUnion(s1, s0) == %s\n", LSetShow(cs1));
  cs1 = LSetCopy(s1);
  LSetUnion(&cs1, cs1);
  fprintf(fp, "  LSetUnion(s1, s1) == %s\n", LSetShow(cs1));
  cs1 = LSetCopy(s1);
  LSetUnion(&cs1, cs2);
  fprintf(fp, "  LSetUnion(s1, s2) == %s\n", LSetShow(cs1));
  cs1 = LSetCopy(s1);
  LSetUnion(&cs1, cs3);
  fprintf(fp, "  LSetUnion(s1, s3) == %s\n", LSetShow(cs1));
  cs1 = LSetCopy(s1);
  fprintf(fp, "\n");

  /* test intersection */
  LSetIntersection(cs1, cs3);
  fprintf(fp, "  LSetIntersection(s1, s3) == %s\n", LSetShow(cs1));
  cs1 = LSetCopy(s1);
  LSetIntersection(cs1, cs2);
  fprintf(fp, "  LSetIntersection(s1, s2) == %s\n", LSetShow(cs1));
  cs1 = LSetCopy(s1);
  LSetIntersection(cs1, cs0);
  fprintf(fp, "  LSetIntersection(s1, s0) == %s\n", LSetShow(cs1));
  cs1 = LSetCopy(s1);
  fprintf(fp, "\n");

  /* test difference */
  LSetDifference(cs2, cs0);
  fprintf(fp, "  LSetDifference(s2, s0) == %s\n", LSetShow(cs2));
  cs2 = LSetCopy(s2);
  LSetDifference(cs3, cs2);
  fprintf(fp, "  LSetDifference(s3, s2) == %s\n", LSetShow(cs3));
  cs3 = LSetCopy(s3);
  LSetDifference(cs3, cs1);
  fprintf(fp, "  LSetDifference(s3, s1) == %s\n", LSetShow(cs3));
  cs3 = LSetCopy(s3);
  fprintf(fp, "\n");

  /* test min */
  fprintf(fp, "  LSetMin(s1) == %d\n", LSetMin(s1));
  fprintf(fp, "  LSetMin(s2) == %d\n", LSetMin(s2));
  fprintf(fp, "  LSetMin(s3) == %d\n", LSetMin(s3));
  fprintf(fp, "\n");

  /* test max */
  fprintf(fp, "  LSetMax(s1) == %d\n", LSetMax(s1));
  fprintf(fp, "  LSetMax(s2) == %d\n", LSetMax(s2));
  fprintf(fp, "  LSetMax(s3) == %d\n", LSetMax(s3));
  fprintf(fp, "\n");

  /* generate tables */
  /* LSetGenerateTables(fp); */

  fprintf(fp, "] LSetTest()\n");
}
