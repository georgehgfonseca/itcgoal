
/*****************************************************************************/
/*                                                                           */
/*  THE `M' MEMORY MODULE                                                    */
/*  COPYRIGHT (C) 2008 Jeffrey H. Kingston                                   */
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
/*  FILE:         m.h                                                        */
/*  DESCRIPTION:  Memory arenas and allocation, and extensible arrays,       */
/*                strings, and symbol tables.                                */
/*                                                                           */
/*****************************************************************************/
#ifndef M_HEADER_FILE
#define M_HEADER_FILE

#define	M_VERSION   L"M Version 1.2 (December 2009)"
#include <stdbool.h>
#include <stdint.h>
#include <wchar.h>
#include <stdlib.h>
#include <string.h>
#include <stdio.h>


/*****************************************************************************/
/*                                                                           */
/*  Memory allocation                                                        */
/*                                                                           */
/*****************************************************************************/

#define MMake(x) (x = malloc(sizeof(*(x))))
#define MFree(x) free(x)


/*****************************************************************************/
/*                                                                           */
/*  Verification                                                             */
/*                                                                           */
/*****************************************************************************/

extern void MAssert(bool cond, char *fmt, ...);


/*****************************************************************************/
/*                                                                           */
/*  Extensible arrays                                                        */
/*                                                                           */
/*****************************************************************************/

/* the MARRAY macro; these struct fields should not be referred to directly */
#define MARRAY(TYPE)							\
  struct {								\
    int		msize;		/* max number of items */		\
    int		csize;		/* current number of items */		\
    TYPE	*items;		/* extensible mem holding the items */	\
  }

typedef MARRAY(bool)		ARRAY_BOOL;
typedef MARRAY(char)		ARRAY_CHAR;
typedef MARRAY(wchar_t)		ARRAY_WCHAR;
typedef MARRAY(short)		ARRAY_SHORT;
typedef MARRAY(int)		ARRAY_INT;
typedef MARRAY(int64_t)		ARRAY_INT64;

typedef MARRAY(void *)		ARRAY_VOIDP;
typedef MARRAY(char *)		ARRAY_STRING;
typedef MARRAY(wchar_t *)	ARRAY_WSTRING;

#define MArrayAddLast(a, t)						\
(									\
  (a).csize >= (a).msize ?						\
  (									\
    ((a).msize = (a).csize * 2 + 5),					\
    ((a).items = realloc((a).items, (a).msize * sizeof((a).items[0]))),	\
    ((a).items[(a).csize++] = (t))					\
  ) : ((a).items[(a).csize++] = (t))					\
)

#define MArrayInsert(a, i, t)						\
(									\
  MArrayAddLast(a, t),							\
  memmove(&(a).items[(i) + 1], &(a).items[i],				\
    ((a).csize-(i)-1)*sizeof((a).items[0])),				\
  (a).items[i] = (t)							\
)

#define MArrayRemove(a, i)						\
(									\
  memmove(&(a).items[i], &(a).items[(i) + 1],				\
    ((a).csize-(i)-1)*sizeof((a).items[0])),				\
  (a).csize--								\
)

#define MArrayRemoveAndPlug(a, i)					\
(									\
  MArrayPut(a, i, MArrayLast(a)),					\
  MArrayRemoveLast(a)							\
)

#define MArrayInit(a)		((a).msize = (a).csize = 0, (a).items=NULL)
#define MArrayFree(a) 		free((a).items)
#define MArraySize(a)		((a).csize)
#define MArrayClear(a)		((a).csize = 0)

#define MArrayAddFirst(a, t)    MArrayInsert(a, 0, t)

#define MArrayGet(a, i)		((a).items[i])
#define	MArrayFirst(a)		((a).items[0])
#define	MArrayLast(a)		((a).items[MArraySize(a) - 1])

#define MArrayPut(a, i, t)      ((a).items[i] = (t))
#define MArrayInc(a, i)		((a).items[i]++)
#define MArrayDec(a, i)		((a).items[i]--)

#define MArrayFill(a, len, t)	while(MArraySize(a) < len) MArrayAddLast(a, t)

#define MArrayRemoveFirst(a)	MArrayRemove(a, 0)
#define MArrayRemoveLast(a)	(a).items[--MArraySize(a)]
#define MArrayDropLast(a)	(MArraySize(a)--)
#define MArrayDropFromEnd(a, n)	(MArraySize(a) -= (n))
#define MArrayRemoveFirstAndPlug(a) MArrayRemoveAndPlug(a, 0)

#define MArrayAppend(d, s, index)					\
  for( index = 0;  index < MArraySize(s);  index++ )			\
    MArrayAddLast((d), MArrayGet((s), index))

#define MArraySwap(a, i, j, tmp)					\
(									\
  (tmp) = MArrayGet((a), (i)),						\
  MArrayPut((a), (i), MArrayGet((a), (j))),				\
  MArrayPut((a), (j), (tmp))						\
)

#define MArrayWholeSwap(a, b, tmp) ((tmp) = (a), (a) = (b), (b) = (tmp))

#define MArraySort(a, compar)						\
  qsort((a).items, MArraySize(a), sizeof((a).items[0]), compar)

#define MArraySortUnique(a, compar)					\
  (a).csize = MArrayUniqSort((a).items, MArraySize(a), sizeof((a).items[0]), compar)

#define MArrayContains(a, t, pos)					\
  (MArrayAddLast(a, t),							\
  MArrayImplFind((char *)(a).items,sizeof((a).items[0]),MArraySize(a),(pos)),\
  MArrayDropLast(a), *(pos) < MArraySize(a))

#define MArrayForEach(a, t, i)						\
  for( *(i) = 0;							\
       *(i) < MArraySize(a) ? (*(t) = (a).items[*(i)], true) : false;	\
       (*(i))++ )

#define MArrayForEachReverse(a, t, i)					\
  for( *(i) = MArraySize(a) - 1;					\
       *(i) >= 0 ? (*(t) = (a).items[*(i)], true) : false;		\
       (*(i))-- )

/* should not be used directly */
extern int MArrayUniqSort(void *base, size_t nmemb, size_t size,
  int(*compar)(const void *, const void *));
extern void MArrayImplFind(char *a, size_t elem_size, int count, int *pos);


/*****************************************************************************/
/*                                                                           */
/*  String factories (char form)                                             */
/*                                                                           */
/*****************************************************************************/

#define MStringInit(ac)			MStringImplInit(&(ac))
#define MStringAddChar(ac, ch)		MStringImplAddChar(&(ac), ch)
#define MStringAddInt(ac, i)		MStringImplAddInt(&(ac), i)
#define MStringAddFloat(ac, fmt, x)	MStringImplAddFloat(&(ac), fmt, x)
#define MStringAddString(ac, s)		MStringImplAddString(&(ac), s)
#define MStringPrintf(ac, maxlen, ...)				\
  MStringImplPrintf(&(ac), maxlen, __VA_ARGS__)
#define MStringVal(ac)			MStringImplVal(&(ac))

extern void MStringImplInit(ARRAY_CHAR *ac);
extern void MStringImplAddChar(ARRAY_CHAR *ac, char ch);
extern void MStringImplAddInt(ARRAY_CHAR *ac, int i);
extern void MStringImplAddString(ARRAY_CHAR *ac, char *s);
extern void MStringImplPrintf(ARRAY_CHAR *ac, size_t maxlen,
  const char *format, ...);
extern char *MStringImplVal(ARRAY_CHAR *ac);

extern char *MStringCopy(char *s);


/*****************************************************************************/
/*                                                                           */
/*  String factories (wchar_t form)                                          */
/*                                                                           */
/*****************************************************************************/

#define MWStringInit(awc)		MWStringImplInit(&(awc))
#define MWStringAddChar(awc, ch)	MWStringImplAddChar(&(awc), ch)
#define MWStringAddInt(awc, i)		MWStringImplAddInt(&(awc), i)
#define MWStringAddFloat(awc, fmt, x)	MWStringImplAddFloat(&(awc), fmt, x)
#define MWStringAddString(awc, s)	MWStringImplAddString(&(awc), s)
#define MWStringPrintf(awc, maxlen, ...)				\
  MWStringImplPrintf(&(awc), maxlen, __VA_ARGS__)
#define MWStringVal(awc)		MWStringImplVal(&(awc))

extern void MWStringImplInit(ARRAY_WCHAR *awc);
extern void MWStringImplAddChar(ARRAY_WCHAR *awc, wchar_t ch);
extern void MWStringImplAddInt(ARRAY_WCHAR *awc, int i);
extern void MWStringImplAddString(ARRAY_WCHAR *awc, wchar_t *s);
extern void MWStringImplPrintf(ARRAY_WCHAR *awc, size_t maxlen,
  const wchar_t *format, ...);
extern wchar_t *MWStringImplVal(ARRAY_WCHAR *awc);
extern wchar_t *MWStringCopy(wchar_t *s);


/*****************************************************************************/
/*                                                                           */
/*  Symbol tables (char * form)                                              */
/*                                                                           */
/*****************************************************************************/

/* the MTABLE_U type; this type should not be referred to directly */
typedef struct {
  int size;			/* current size of key and value arrays      */
  int trigger;			/* trigger for rehashing                     */
  int pos;			/* temporary return value (insert and delete)*/
  int value_size;		/* size of values                            */
  char **keys;			/* extensible array of keys                  */
} MTABLE_U;

/* the MTABLE macro; these struct fields should not be referred to directly */
#define MTABLE(TYPE)							\
  struct {								\
    MTABLE_U mtu;		/* untyped part of mtable           */	\
    TYPE *values;		/* parallel typed array of values   */	\
  }

#define MTableFree(table) (free((table).mtu.keys), free((table).values))

#define MTableInit(table)						\
( (table).mtu.size = 0, (table).mtu.trigger = 0, (table).mtu.pos = 0,	\
  (table).mtu.value_size = sizeof((table).values[0]),			\
  (table).mtu.keys = NULL, (table).values = NULL,			\
  (table).values = MTableImplCheckRehash(&(table).mtu,			\
    (char *) (table).values)						\
)

#define MTableOccupiedPos(table, pos) ((table).mtu.keys[pos] >= (char *) 0x1)
#define MTableKey(table, pos) ((table).mtu.keys[pos])
#define MTableValue(table, pos) ((table).values[pos])
#define MTableSetValue(table, pos, value) ((table).values[pos] = value)

#define MTableInsert(table, key, value)					\
(									\
  (table).values = MTableImplCheckRehash(&(table).mtu,			\
    (char *) (table).values),						\
  MTableImplInsert(&(table).mtu, (key)),				\
  MTableSetValue(table, (table).mtu.pos, value)				\
)

#define MTableInsertUnique(table, key, value, other)			\
(									\
  (table).values = MTableImplCheckRehash(&(table).mtu,			\
    (char *) (table).values),						\
  MTableImplInsertUnique(&(table).mtu, (key)) ?				\
    (MTableSetValue(table, (table).mtu.pos, value), true) :		\
    (*(other) = MTableValue(table, (table).mtu.pos), false)		\
)

#define MTableClear(table) MTableImplClear(&(table).mtu)

#define MTableContains(table, key, pos)					\
  MTableImplContainsHashed(&(table).mtu, MTableHash(key), key, pos)

#define MTableContainsHashed(table, hash_code, key, pos)		\
  MTableImplContainsHashed(&(table).mtu, (hash_code), (key), (pos))

#define MTableContainsNext(table, pos)					\
  MTableImplContainsNext(&(table).mtu, pos)

#define MTableRetrieve(table, key, value, pos)				\
  MTableRetrieveHashed((table), MTableHash(key), (key), (value), (pos))

#define MTableRetrieveHashed(table, hash_code, key, value, pos)		\
(									\
  MTableContainsHashed((table), (hash_code), (key), (pos)) ?		\
  (*(value) = MTableValue((table), *(pos)), true) : 			\
  (*(value) = MTableValue((table), *(pos)), false) 			\
)

#define MTableRetrieveNext(table, value, pos)				\
(									\
  MTableContainsNext((table), (pos)) ?					\
  (*(value) = MTableValue((table), *(pos)), true) : false		\
)

#define MTableDelete(table, pos) MTableImplDelete(&(table).mtu, pos)

extern int MTableHash(char *key);

#define MTableForEachWithKey(table, key, value, pos)			\
  MTableForEachWithKeyHashed(table, MTableHash(key), key, value, pos)

#define MTableForEachWithKeyHashed(table, hash_code, key, value, pos)	\
  for( MTableRetrieveHashed((table), (hash_code), (key),(value),(pos));	\
       MTableOccupiedPos((table), *(pos));				\
       MTableRetrieveNext((table), value, (pos)) )			\

#define MTableSize(table) (table).mtu.size

#define MTableForEach(table, key, value, pos)				\
  for( *(pos) = 0;  *(pos) < MTableSize(table);  (*(pos))++ )		\
    if( !MTableOccupiedPos((table), *(pos)) ? false :			\
      (*(key) = MTableKey((table), *(pos)),				\
       *(value) = MTableValue((table), *(pos)), true) )


/* should not be used directly */
extern void MTableImplClear(MTABLE_U *table);
extern void *MTableImplCheckRehash(MTABLE_U *table, char *values);
extern void MTableImplInsert(MTABLE_U *table, char *key);
extern bool MTableImplInsertUnique(MTABLE_U *table, char *key);
extern bool MTableImplContainsHashed(MTABLE_U *table, int hash_code,
  char *key, int *pos);
extern bool MTableImplContainsNext(MTABLE_U *table, int *pos);
extern void MTableImplDelete(MTABLE_U *table, int pos);
extern void MTest(FILE *fp);


/*****************************************************************************/
/*                                                                           */
/*  Symbol tables (wchar_t * form)                                           */
/*                                                                           */
/*****************************************************************************/

/* the MWTABLE_U type; this type should not be referred to directly */
typedef struct {
  int size;			/* current size of key and value arrays      */
  int trigger;			/* trigger for rehashing                     */
  int pos;			/* temporary return value (insert and delete)*/
  int value_size;		/* size of values                            */
  wchar_t **keys;		/* extensible array of keys                  */
} MWTABLE_U;

/* the MWTABLE macro; these struct fields should not be referred to directly */
#define MWTABLE(TYPE)							\
  struct {								\
    MWTABLE_U mtu;		/* untyped part of mtable           */	\
    TYPE *values;		/* parallel typed array of values   */	\
  }

#define MWTableInit(table)						\
( (table).mtu.size = 0, (table).mtu.trigger = 0, (table).mtu.pos = 0,	\
  (table).mtu.value_size = sizeof((table).values[0]),			\
  (table).mtu.keys = NULL, (table).values = NULL,			\
  (table).values = MWTableImplCheckRehash(&(table).mtu,			\
    (char *) (table).values)						\
)

#define MWTableFree(table) (free((table).mtu.keys), free((table).values))

#define MWTableOccupiedPos(table, pos) ((table).mtu.keys[pos] >= (wchar_t*)0x1)
#define MWTableKey(table, pos) ((table).mtu.keys[pos])
#define MWTableValue(table, pos) ((table).values[pos])
#define MWTableSetValue(table, pos, value) ((table).values[pos] = value)

#define MWTableInsert(table, key, value)				\
(									\
  (table).values = MWTableImplCheckRehash(&(table).mtu,			\
    (char *) (table).values),						\
  MWTableImplInsert(&(table).mtu, (key)),				\
  MWTableSetValue(table, (table).mtu.pos, value)			\
)

#define MWTableInsertUnique(table, key, value, other)			\
(									\
  (table).values = MWTableImplCheckRehash(&(table).mtu,			\
    (char *) (table).values),						\
  MWTableImplInsertUnique(&(table).mtu, (key)) ?			\
    (MWTableSetValue(table, (table).mtu.pos, value), true) :		\
    (*(other) = MWTableValue(table, (table).mtu.pos), false)		\
)

#define MWTableClear(table) MWTableImplClear(&(table).mtu)

#define MWTableContains(table, key, pos)				\
  MWTableImplContainsHashed(&(table).mtu, MWTableHash(key), key, pos)

#define MWTableContainsHashed(table, hash_code, key, pos)		\
  MWTableImplContainsHashed(&(table).mtu, (hash_code), (key), (pos))

#define MWTableContainsNext(table, pos)					\
  MWTableImplContainsNext(&(table).mtu, pos)

#define MWTableRetrieve(table, key, value, pos)				\
  MWTableRetrieveHashed((table), MWTableHash(key), (key), (value), (pos))

#define MWTableRetrieveHashed(table, hash_code, key, value, pos)	\
(									\
  MWTableContainsHashed((table), (hash_code), (key), (pos)) ?		\
  (*(value) = MWTableValue((table), *(pos)), true) : 			\
  (*(value) = MWTableValue((table), *(pos)), false) 			\
)

#define MWTableRetrieveNext(table, value, pos)				\
(									\
  MWTableContainsNext((table), (pos)) ?					\
  (*(value) = MWTableValue((table), *(pos)), true) : false		\
)

#define MWTableDelete(table, pos) MWTableImplDelete(&(table).mtu, pos)

extern int MWTableHash(wchar_t *key);

#define MWTableForEachWithKey(table, key, value, pos)			\
  MWTableForEachWithKeyHashed(table, MWTableHash(key), key, value, pos)

#define MWTableForEachWithKeyHashed(table, hash_code, key, value, pos)	\
  for( MWTableRetrieveHashed((table), (hash_code),(key),(value),(pos)); \
       MWTableOccupiedPos((table), *(pos));				\
       MWTableRetrieveNext((table), value, (pos)) )			\

#define MWTableSize(table) (table).mtu.size

#define MWTableForEach(table, key, value, pos)				\
  for( *(pos) = 0;  *(pos) < MWTableSize(table);  (*(pos))++ )		\
    if( !MWTableOccupiedPos((table), *(pos)) ? false :			\
      (*(key) = MWTableKey((table), *(pos)),				\
       *(value) = MWTableValue((table), *(pos)), true) )


/* should not be used directly */
extern void MWTableImplClear(MWTABLE_U *table);
extern void *MWTableImplCheckRehash(MWTABLE_U *table, char *values);
extern void MWTableImplInsert(MWTABLE_U *table, wchar_t *key);
extern bool MWTableImplInsertUnique(MWTABLE_U *table, wchar_t *key);
extern bool MWTableImplContainsHashed(MWTABLE_U *table, int hash_code,
  wchar_t *key, int *pos);
extern bool MWTableImplContainsNext(MWTABLE_U *table, int *pos);
extern void MWTableImplDelete(MWTABLE_U *table, int pos);
#endif
