
/*****************************************************************************/
/*                                                                           */
/*  THE `M' MEMORY MODULE                                                    */
/*  COPYRIGHT (C) 2009 Jeffrey H. Kingston                                   */
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
/*  FILE:         m.c                                                        */
/*  DESCRIPTION:  Memory arenas and allocation, and extensible arrays,       */
/*                strings, and symbol tables.                                */
/*                                                                           */
/*****************************************************************************/
#include "m.h"
#include <string.h>
#include <stdarg.h>

#define DEBUG1 0
#define DEBUG2 0

/*****************************************************************************/
/*                                                                           */
/*  Submodule "verification"                                                 */
/*                                                                           */
/*****************************************************************************/

/*****************************************************************************/
/*                                                                           */
/*  void MAssert(bool cond, char *fmt, ...)                                  */
/*                                                                           */
/*  Check cond, and if false, print an error message and exit.               */
/*                                                                           */
/*****************************************************************************/

void MAssert(bool cond, char *fmt, ...)
{
  va_list args;
  if( !cond )
  {
    va_start(args, fmt);
    fprintf(stderr, "KHE error: ");
    vfprintf(stderr, fmt, args);
    fprintf(stderr, "\n");
    va_end(args);
    abort();
  }
}


/*****************************************************************************/
/*                                                                           */
/*  Submodule "extensible arrays"                                            */
/*                                                                           */
/*****************************************************************************/

/*****************************************************************************/
/*                                                                           */
/*  int MArrayUniqSort(void *base, size_t nmemb, size_t size,                */
/*      int(*compar)(const void *, const void *))                            */
/*                                                                           */
/*  The parameters of MArrayUniqSort are the same as for qsort and have the  */
/*  same meaning.  This function first sorts using qsort, then uniqueifies   */
/*  the result array by removing each element which compares equal to the    */
/*  preceding element.  It returns the final number of elements.             */
/*                                                                           */
/*****************************************************************************/

int MArrayUniqSort(void *base, size_t nmemb, size_t size,
    int(*compar)(const void *, const void *))
{
  int i, j;
  if( nmemb > 0 )
  {
    if( DEBUG1 )
      fprintf(stderr, "[ MArrayUniqSort(nmemb %d, size %d)\n",
	(int) nmemb, (int) size);
    qsort(base, nmemb, size, compar);
    if( DEBUG1 )
      fprintf(stderr, "  MArrayUniqSort uniqueifying:\n");
    i = 0;
    for( j = 1;  j < nmemb;  j++ )
      if( compar(&((char *) base)[i * size], &((char *) base)[j * size]) != 0 )
      {
	i++;
	if( i != j )
	  memcpy(&((char *) base)[i * size], &((char *) base)[j * size], size);
      }
    if( DEBUG1 )
      fprintf(stderr, "] MArrayUniqSort returning %d\n", i + 1);
    return i + 1;
  }
  else
    return 0;
}


/*****************************************************************************/
/*                                                                           */
/*  void MArrayImplFind(void *a, size_t elem_size, int count, int *pos)      */
/*                                                                           */
/*  Scan a in chunks of size elem_size, looking for whatever is at the       */
/*  end of the array.                                                        */
/*                                                                           */
/*****************************************************************************/

void MArrayImplFind(char *a, size_t elem_size, int count, int *pos)
{
  char *target = &a[(count - 1) * elem_size];
  char *p;
  for( *pos = 0, p = a;  *pos < count;  (*pos)++, p += elem_size )
    if( memcmp(p, target, elem_size) == 0 )
      return;
}


/*****************************************************************************/
/*                                                                           */
/*  Submodule "string factories (char form)"                                 */
/*                                                                           */
/*****************************************************************************/

/*****************************************************************************/
/*                                                                           */
/*  void MStringImplInit(ARRAY_CHAR *ac)                                     */
/*                                                                           */
/*  Initialize *ac.                                                          */
/*                                                                           */
/*****************************************************************************/

void MStringImplInit(ARRAY_CHAR *ac)
{
  MArrayInit(*ac);
}


/*****************************************************************************/
/*                                                                           */
/*  void MStringImplAddChar(ARRAY_CHAR *ac, char ch)                         */
/*                                                                           */
/*  Add ch to *ac.                                                           */
/*                                                                           */
/*****************************************************************************/

void MStringImplAddChar(ARRAY_CHAR *ac, char ch)
{
  char buff[5];
  snprintf(buff, 5, "%c", ch);
  MStringImplAddString(ac, buff);
}


/*****************************************************************************/
/*                                                                           */
/*  void MStringImplAddInt(ARRAY_CHAR *ac, int i)                            */
/*                                                                           */
/*  Add i to *ac.                                                            */
/*                                                                           */
/*****************************************************************************/

void MStringImplAddInt(ARRAY_CHAR *ac, int i)
{
  char buff[50];
  snprintf(buff, 50, "%d", i);
  MStringImplAddString(ac, buff);
}


/*****************************************************************************/
/*                                                                           */
/*  void MStringImplAddString(ARRAY_CHAR *ac, char *s)                       */
/*                                                                           */
/*  Append s to the end of *ac.                                              */
/*                                                                           */
/*****************************************************************************/

void MStringImplAddString(ARRAY_CHAR *ac, char *s)
{
  char *p;
  for( p = s;  *p != '\0';  p++ )
    MArrayAddLast(*ac, *p);
}


/*****************************************************************************/
/*                                                                           */
/*  void MStringImplPrintf(ARRAY_CHAR *ac, size_t maxlen,                    */
/*    const char *format, ...)                                               */
/*                                                                           */
/*  Like snprintf, except the resulting characters are appended to *ac.      */
/*                                                                           */
/*****************************************************************************/

void MStringImplPrintf(ARRAY_CHAR *ac, size_t maxlen,
  const char *format, ...)
{
  va_list args;  char *mem, buff[201];
  if( maxlen > 200 )
    mem = (char *) malloc((maxlen + 1) * sizeof(char));
  else
    mem = buff;
  va_start(args, format);
  vsnprintf(mem, maxlen, format, args);
  va_end(args);
  MStringImplAddString(ac, mem);
  if( maxlen > 200 )
    free(mem);
}


/*****************************************************************************/
/*                                                                           */
/*  char *MStringImplVal(ARRAY_CHAR *ac)                                     */
/*                                                                           */
/*  Return the string value of *ac, after adding a null character.           */
/*                                                                           */
/*****************************************************************************/

char *MStringImplVal(ARRAY_CHAR *ac)
{
  MArrayAddLast(*ac, '\0');
  return ac->items;
}


/*****************************************************************************/
/*                                                                           */
/*  char *MStringCopy(char *s)                                               */
/*                                                                           */
/*  Return a copy of s in heap memory.                                       */
/*                                                                           */
/*****************************************************************************/

char *MStringCopy(char *s)
{
  char *res;
  if( s == NULL )
    return NULL;
  else
  {
    res = (char *) malloc((strlen(s) + 1) * sizeof(char));
    strcpy(res, s);
    return res;
  }
}


/*****************************************************************************/
/*                                                                           */
/*  Submodule "string factories (wchar_t form)"                              */
/*                                                                           */
/*****************************************************************************/

/*****************************************************************************/
/*                                                                           */
/*  void MWStringImplInit(ARRAY_WCHAR *awc)                                  */
/*                                                                           */
/*  Initialize *awc.                                                         */
/*                                                                           */
/*****************************************************************************/

void MWStringImplInit(ARRAY_WCHAR *awc)
{
  MArrayInit(*awc);
}


/*****************************************************************************/
/*                                                                           */
/*  void MWStringImplAddChar(ARRAY_WCHAR *awc, wchar_t ch)                   */
/*                                                                           */
/*  Add ch to *awc.                                                          */
/*                                                                           */
/*****************************************************************************/

void MWStringImplAddChar(ARRAY_WCHAR *awc, wchar_t ch)
{
  wchar_t buff[5];
  swprintf(buff, 5, L"%lc", ch);
  MWStringImplAddString(awc, buff);
}


/*****************************************************************************/
/*                                                                           */
/*  void MWStringImplAddInt(ARRAY_WCHAR *awc, int i)                         */
/*                                                                           */
/*  Add i to *awc.                                                           */
/*                                                                           */
/*****************************************************************************/

void MWStringImplAddInt(ARRAY_WCHAR *awc, int i)
{
  wchar_t buff[50];
  swprintf(buff, 50, L"%d", i);
  MWStringImplAddString(awc, buff);
}


/*****************************************************************************/
/*                                                                           */
/*  void MWStringImplAddString(ARRAY_WCHAR *awc, wchar_t *s)                 */
/*                                                                           */
/*  Append s to the end of *awc.                                             */
/*                                                                           */
/*****************************************************************************/

void MWStringImplAddString(ARRAY_WCHAR *awc, wchar_t *s)
{
  wchar_t *p;
  for( p = s;  *p != L'\0';  p++ )
    MArrayAddLast(*awc, *p);
}


/*****************************************************************************/
/*                                                                           */
/*  void MWStringImplPrintf(ARRAY_WCHAR *awc, size_t maxlen,                 */
/*    const wchar_t *format, ...)                                            */
/*                                                                           */
/*  Like swprintf, except the resulting characters are appended to *awc.     */
/*                                                                           */
/*****************************************************************************/

void MWStringImplPrintf(ARRAY_WCHAR *awc, size_t maxlen,
  const wchar_t *format, ...)
{
  va_list args;  wchar_t *mem, buff[201];
  if( maxlen > 200 )
    mem = (wchar_t *) malloc((maxlen + 1) * sizeof(wchar_t));
  else
    mem = buff;
  va_start(args, format);
  vswprintf(mem, maxlen, format, args);
  va_end(args);
  MWStringImplAddString(awc, mem);
  if( maxlen > 200 )
    free(mem);
}


/*****************************************************************************/
/*                                                                           */
/*  wchar_t *MWStringImplVal(ARRAY_WCHAR *awc)                               */
/*                                                                           */
/*  Return the string value of awc.                                          */
/*                                                                           */
/*****************************************************************************/

wchar_t *MWStringImplVal(ARRAY_WCHAR *awc)
{
  MArrayAddLast(*awc, L'\0');
  return awc->items;
}


/*****************************************************************************/
/*                                                                           */
/*  wchar_t *MWStringCopy(wchar_t *s)                                        */
/*                                                                           */
/*  Return a copy of s in heap memory.                                       */
/*                                                                           */
/*****************************************************************************/

wchar_t *MWStringCopy(wchar_t *s)
{
  wchar_t *res;
  if( s == NULL )
    return NULL;
  else
  {
    res = (wchar_t *) malloc((wcslen(s) + 1) * sizeof(wchar_t));
    wcscpy(res, s);
    return res;
  }
}


/*****************************************************************************/
/*                                                                           */
/*  Submodule "symbol tables (char * form)"                                  */
/*                                                                           */
/*  In addition to representing the absence of a key by the special value    */
/*  0x0 (NULL) as usual, this code represents a recently deleted key by the  */
/*  special value 0x1.  As is well known, one cannot simply delete a key by  */
/*  setting it to 0x0, since that may cause a later retrieval to fail to     */
/*  find other entries besides the one deleted, so 0x1 is used in this case. */
/*                                                                           */
/*  A symbol table object has five fields:                                   */
/*                                                                           */
/*     size     Current size of the keys and values arrays                   */
/*                                                                           */
/*     trigger  A variable whose value satisfies                             */
/*                                                                           */
/*                 1 < trigger < number of 0x0 keys in table                 */
/*                                                                           */
/*              Note that the key in an unused position can be either 0x0    */
/*              or 0x1; trigger is related to the number of 0x0 keys.  When  */
/*              trigger might drop below 1 the table needs to be rehashed.   */
/*                                                                           */
/*     pos      A strictly temporary return value set by some functions      */
/*                                                                           */
/*     keys     The keys, an array of char* values whose length is size.     */
/*              Each element is either 0x0 or 0x1, denoting a free spot,     */
/*              or some other value, in which case it points to a key.       */
/*                                                                           */
/*     values   The values, an array of any type whose length is also size.  */
/*                                                                           */
/*****************************************************************************/

/*****************************************************************************/
/*                                                                           */
/*  int MTableHash(char *key)                                                */
/*                                                                           */
/*  Return the hash code of key, before reduction modulo the table size.     */
/*                                                                           */
/*  Implementation note.  This hash function adds the characters together,   */
/*  shifting each subtotal one place to the left.  If the total is negative  */
/*  it is negated to make the result non-negative.                           */
/*                                                                           */
/*****************************************************************************/

int MTableHash(char *key)
{
  char *p;  int res;
  res = 0;
  for( p = key;  *p != L'\0';  p++ )
    res = (res << 1) + *p;
  if( res < 0 )  res = - res;
  return res;
}


/*****************************************************************************/
/*                                                                           */
/*  void MTableImplClear(MTABLE_U *table)                                    */
/*                                                                           */
/*  Clear table, setting the trigger to 4/5 of its capacity.                 */
/*  The values are not cleared, but that does not matter.                    */
/*                                                                           */
/*****************************************************************************/

void MTableImplClear(MTABLE_U *table)
{
  int i;
  for( i = 0;  i < table->size;  i++ )
    table->keys[i] = NULL;
  table->trigger = (4 * table->size) / 5;
  table->pos = -1;
}


/*****************************************************************************/
/*                                                                           */
/*  void *MTableImplCheckRehash(MTABLE_U *table, char *values)               */
/*                                                                           */
/*  Rehash table into just over twice as much space as it occupies now,      */
/*  if the trigger requires it.                                              */
/*                                                                           */
/*  Implementation note.  Since the keys and values need to take up new      */
/*  positions, new space for keys and values is allocated, the old keys and  */
/*  values are copied over, and the old space for keys and values is freed.  */
/*                                                                           */
/*****************************************************************************/

void *MTableImplCheckRehash(MTABLE_U *table, char *values)
{
  int i, old_size;  char *key;  char **old_keys;  char *old_values;
  if( table->trigger <= 1 )
  {
    if( DEBUG2 )
      fprintf(stderr, "  [ MTableRehash(table %p)\n", (void *) table);

    /* save old size, keys, and values */
    old_size = table->size;
    old_keys = table->keys;
    old_values = values;

    /* re-initialize table to the next larger size, all clear */
    table->size = 2 * old_size + 5;
    table->trigger = (4 * table->size) / 5;
    table->pos = -1;
    table->keys = (char **) calloc(table->size, sizeof(char *));
    values = (char *) calloc(table->size, table->value_size);

    /* insert the old keys and values into table, then free their arrays */
    for( i = 0;  i < old_size;  i++ )
    {
      key = old_keys[i];
      if( key > (char *) 0x1 )
      {
	MTableImplInsert(table, key);
	memcpy(&values[table->pos * table->value_size],
	  &old_values[i * table->value_size], table->value_size);
      }
    }
    free(old_keys);
    free(old_values);

    if( DEBUG2 )
      fprintf(stderr, "  ] MTableRehash\n");
  }
  return (void *) values;
}


/*****************************************************************************/
/*                                                                           */
/*  void MTableImplInsert(TABLE_CHAR *table, char *key)                      */
/*                                                                           */
/*  Insert key into table and set table->pos to its position in the keys     */
/*  array, so that the caller can insert the value in a type-safe manner.    */
/*  Any necessary rehash with have already been done.                        */
/*                                                                           */
/*****************************************************************************/

void MTableImplInsert(MTABLE_U *table, char *key)
{
  int i;
  if( DEBUG2 )
    fprintf(stderr, "[ MTableImplInsert(%p, \"%s\")\n", (void *) table, key);
  i = MTableHash(key) % table->size;
  while( table->keys[i] > (char *) 0x1 )
    i = (i + 1) % table->size;
  if( table->keys[i] == 0x0 )
    --(table->trigger);
  table->keys[i] = key;
  table->pos = i;
  if( DEBUG2 )
    fprintf(stderr, "] MTableImplInsert returning (pos = %d)\n", i);
}


/*****************************************************************************/
/*                                                                           */
/*  bool MTableImplInsertUnique(MTABLE_U *table, char *key)                  */
/*                                                                           */
/*  If there is currently no entry in the table with this key, insert key    */
/*  into the table and set table->pos to its position in the keys array, so  */
/*  the caller can assign the value in a type-safe manner, and return true.  */
/*                                                                           */
/*  If there is currently an entry in the table with this key, set           */
/*  table->pos to the position of that entry, so that the caller             */
/*  can pass it on to the user in a type-safe manner, and return false.      */
/*                                                                           */
/*  Any necessary rehash with have already been done.                        */
/*                                                                           */
/*****************************************************************************/

bool MTableImplInsertUnique(MTABLE_U *table, char *key)
{
  int i, insert_pos;
  if( DEBUG2 )
    fprintf(stderr, "[ MTableImplInsertUnique(%p, \"%s\")\n",
      (void *) table, key);
  insert_pos = -1;
  i = MTableHash(key) % table->size;
  while( table->keys[i] != 0x0 )
  {
    if( table->keys[i] == (char *) 0x1 )
    {
      /* not a true key; we may insert here later */
      insert_pos = i;
    }
    else if( strcmp(key, table->keys[i]) == 0 )
    {
      table->pos = i;
      if( DEBUG2 )
	fprintf(stderr, "] MTableImplInsertUnique ret. false (pos %d)\n", i);
      return false;
    }
    i = (i + 1) % table->size;
  }
  if( insert_pos == -1 )
  {
    insert_pos = i;
    --(table->trigger);
  }
  table->keys[insert_pos] = key;
  table->pos = insert_pos;
  if( DEBUG2 )
    fprintf(stderr, "] MTableImplInsertUnique returning true (pos %d)\n",
      insert_pos);
  return true;
}


/*****************************************************************************/
/*                                                                           */
/*  bool MTableImplContainsHashed(MTABLE_U *table, int hash_code,            */
/*    char *key, int *pos)                                                   */
/*                                                                           */
/*  Carry out a retrieval of key from table, assuming that hash_code is      */
/*  its hash code.  If there is an entry with this key, return true and set  */
/*  *pos to its index; if not, return false and set *pos to the index of     */
/*  the gap that proves that key is not present.                             */
/*                                                                           */
/*****************************************************************************/

bool MTableImplContainsHashed(MTABLE_U *table, int hash_code,
  char *key, int *pos)
{
  int i;
  if( DEBUG2 )
    fprintf(stderr, "[ MTableImplRetrieve(%p, %d, \"%s\")\n",
      (void *) table, hash_code, key);
  i = hash_code % table->size;
  while( table->keys[i] != 0x0 )
  {
    if( table->keys[i] > (char *) 0x1 && strcmp(key, table->keys[i]) == 0 )
    {
      if( DEBUG2 )
	fprintf(stderr, "] MTableImplRetrieve returning true (pos %d)\n", i);
      *pos = i;
      return true;
    }
    i = (i + 1) % table->size;
  }
  if( DEBUG2 )
    fprintf(stderr, "] MTableImplRetrieve returning false (pos %d)\n", i);
  *pos = i;
  return false;
}


/*****************************************************************************/
/*                                                                           */
/*  bool MTableImplContainsNext(MTABLE_U *table, int *pos)                   */
/*                                                                           */
/*  It is a precondition of this function that *pos is the index of an       */
/*  entry of table.  Search onwards from that entry for another entry        */
/*  with the same key as this one.  If found, set *pos to the index of       */
/*  that entry and return true; if not found, set *pos to the index of the   */
/*  gap that proves there is no entry, and return false.                     */
/*                                                                           */
/*****************************************************************************/

bool MTableImplContainsNext(MTABLE_U *table, int *pos)
{
  char *key;   int i;
  i = *pos;
  key = table->keys[i];
  if( DEBUG2 )
    fprintf(stderr, "[ MTableImplRetrieveNext(%p, %d holding \"%s\")\n",
      (void *) table, i, key);
  i = (i + 1) % table->size;
  while( table->keys[i] != 0x0 )
  {
    if( table->keys[i] > (char *) 0x1 && strcmp(key, table->keys[i]) == 0 )
    {
      if( DEBUG2 )
	fprintf(stderr, "] MTableImplRetrieveNext returning true (pos %d)\n",i);
      *pos = i;
      return true;
    }
    i = (i + 1) % table->size;
  }
  if( DEBUG2 )
    fprintf(stderr, "] MTableImplRetrieveNext returning false (pos %d)\n", i);
  *pos = i;
  return false;
}


/*****************************************************************************/
/*                                                                           */
/*  void MTableImplDelete(MTABLE_U *table, int pos)                          */
/*                                                                           */
/*  Delete the entry at this position.  It must be present.                  */
/*                                                                           */
/*****************************************************************************/

void MTableImplDelete(MTABLE_U *table, int pos)
{
  MAssert(table->keys[pos] > (char *) 0x1, "MTableDelete: no entry at pos");
  if( DEBUG2 )
    fprintf(stderr, "[ MTableImplDelete(%p, %d \"%s\")\n", (void *) table,
      pos, table->keys[pos]);
  if( table->keys[(pos + 1) % table->size] == 0x0 )
  {
    /* safe to set this entry to NULL, since nothing follows */
    table->keys[pos] = 0x0;
    (table->trigger)++;
  }
  else
    table->keys[pos] = (char *) 0x1;
}


/*****************************************************************************/
/*                                                                           */
/*  Submodule "symbol tables (wchar_t * form)"                               */
/*                                                                           */
/*  Just like the char * form except that char is replaced by wchar_t,       */
/*  MTABLE is replaced by MWTABLE, and MTable is replaced by MWTable.        */
/*                                                                           */
/*****************************************************************************/

/*****************************************************************************/
/*                                                                           */
/*  int MWTableHash(wchar_t *key)                                            */
/*                                                                           */
/*  Return the hash code of key, before reduction modulo the table size.     */
/*                                                                           */
/*  Implementation note.  This hash function adds the characters together,   */
/*  shifting each subtotal one place to the left.  If the total is negative  */
/*  it is negated to make the result non-negative.                           */
/*                                                                           */
/*****************************************************************************/

int MWTableHash(wchar_t *key)
{
  wchar_t *p;  int res;
  res = 0;
  for( p = key;  *p != L'\0';  p++ )
    res = (res << 1) + *p;
  if( res < 0 )  res = - res;
  return res;
}


/*****************************************************************************/
/*                                                                           */
/*  void MWTableImplClear(MWTABLE_U *table)                                  */
/*                                                                           */
/*  Clear table, setting the trigger to 4/5 of its capacity.                 */
/*  The values are not cleared, but that does not matter.                    */
/*                                                                           */
/*****************************************************************************/

void MWTableImplClear(MWTABLE_U *table)
{
  int i;
  for( i = 0;  i < table->size;  i++ )
    table->keys[i] = NULL;
  table->trigger = (4 * table->size) / 5;
  table->pos = -1;
}


/*****************************************************************************/
/*                                                                           */
/*  void *MWTableImplCheckRehash(MWTABLE_U *table, char *values)             */
/*                                                                           */
/*  Rehash table into just over twice as much space as it occupies now,      */
/*  if the trigger requires it.                                              */
/*                                                                           */
/*  Implementation note.  Since the keys and values need to take up new      */
/*  positions, new space for keys and values is allocated, the old keys and  */
/*  values are copied over, and the old space for keys and values is freed.  */
/*                                                                           */
/*****************************************************************************/

void *MWTableImplCheckRehash(MWTABLE_U *table, char *values)
{
  int i, old_size;  wchar_t *key;  wchar_t **old_keys;  char *old_values;
  if( table->trigger <= 1 )
  {
    if( DEBUG2 )
      fprintf(stderr, "  [ MWTableRehash(table %p)\n", (void *) table);

    /* save old size, keys, and values */
    old_size = table->size;
    old_keys = table->keys;
    old_values = values;

    /* re-initialize table to the next larger size, all clear */
    table->size = 2 * old_size + 5;
    table->trigger = (4 * table->size) / 5;
    table->pos = -1;
    table->keys = (wchar_t **) calloc(table->size, sizeof(wchar_t *));
    values = (char *) calloc(table->size, table->value_size);

    /* insert the old keys and values into table, then free their arrays */
    for( i = 0;  i < old_size;  i++ )
    {
      key = old_keys[i];
      if( key > (wchar_t *) 0x1 )
      {
	MWTableImplInsert(table, key);
	memcpy(&values[table->pos * table->value_size],
	  &old_values[i * table->value_size], table->value_size);
      }
    }
    free(old_keys);
    free(old_values);

    if( DEBUG2 )
      fprintf(stderr, "  ] MWTableRehash\n");
  }
  return (void *) values;
}


/*****************************************************************************/
/*                                                                           */
/*  void MWTableImplInsert(TABLE_CHAR *table, wchar_t *key)                  */
/*                                                                           */
/*  Insert key into table and set table->pos to its position in the keys     */
/*  array, so that the caller can insert the value in a type-safe manner.    */
/*  Any necessary rehash with have already been done.                        */
/*                                                                           */
/*****************************************************************************/

void MWTableImplInsert(MWTABLE_U *table, wchar_t *key)
{
  int i;
  if( DEBUG2 )
    fprintf(stderr, "[ MWTableImplInsert(%p, \"%ls\")\n", (void *) table, key);
  i = MWTableHash(key) % table->size;
  while( table->keys[i] > (wchar_t *) 0x1 )
    i = (i + 1) % table->size;
  if( table->keys[i] == 0x0 )
    --(table->trigger);
  table->keys[i] = key;
  table->pos = i;
  if( DEBUG2 )
    fprintf(stderr, "] MWTableImplInsert returning (pos = %d)\n", i);
}


/*****************************************************************************/
/*                                                                           */
/*  bool MWTableImplInsertUnique(MWTABLE_U *table, wchar_t *key)             */
/*                                                                           */
/*  If there is currently no entry in the table with this key, insert key    */
/*  into the table and set table->pos to its position in the keys array, so  */
/*  the caller can assign the value in a type-safe manner, and return true.  */
/*                                                                           */
/*  If there is currently an entry in the table with this key, set           */
/*  table->pos to the position of that entry, so that the caller             */
/*  can pass it on to the user in a type-safe manner, and return false.      */
/*                                                                           */
/*  Any necessary rehash with have already been done.                        */
/*                                                                           */
/*****************************************************************************/

bool MWTableImplInsertUnique(MWTABLE_U *table, wchar_t *key)
{
  int i, insert_pos;
  if( DEBUG2 )
    fprintf(stderr, "[ MWTableImplInsertUnique(%p, \"%ls\")\n",
      (void *) table, key);
  insert_pos = -1;
  i = MWTableHash(key) % table->size;
  while( table->keys[i] != 0x0 )
  {
    if( table->keys[i] == (wchar_t *) 0x1 )
    {
      /* not a true key; we may insert here later */
      insert_pos = i;
    }
    else if( wcscmp(key, table->keys[i]) == 0 )
    {
      table->pos = i;
      if( DEBUG2 )
	fprintf(stderr, "] MWTableImplInsertUnique ret. false (pos %d)\n", i);
      return false;
    }
    i = (i + 1) % table->size;
  }
  if( insert_pos == -1 )
  {
    insert_pos = i;
    --(table->trigger);
  }
  table->keys[insert_pos] = key;
  table->pos = insert_pos;
  if( DEBUG2 )
    fprintf(stderr, "] MWTableImplInsertUnique returning true (pos %d)\n",
      insert_pos);
  return true;
}


/*****************************************************************************/
/*                                                                           */
/*  bool MWTableImplContainsHashed(MWTABLE_U *table, int hash_code,          */
/*    wchar_t *key, int *pos)                                                */
/*                                                                           */
/*  Carry out a retrieval of key from table, assuming that hash_code is      */
/*  its hash code.  If there is an entry with this key, return true and set  */
/*  *pos to its index; if not, return false and set *pos to the index of     */
/*  the gap that proves that key is not present.                             */
/*                                                                           */
/*****************************************************************************/

bool MWTableImplContainsHashed(MWTABLE_U *table, int hash_code,
  wchar_t *key, int *pos)
{
  int i;
  if( DEBUG2 )
    fprintf(stderr, "[ MWTableImplRetrieve(%p, %d, \"%ls\")\n",
      (void *) table, hash_code, key);
  i = hash_code % table->size;
  while( table->keys[i] != 0x0 )
  {
    if( table->keys[i] > (wchar_t *) 0x1 && wcscmp(key, table->keys[i]) == 0 )
    {
      if( DEBUG2 )
	fprintf(stderr, "] MWTableImplRetrieve returning true (pos %d)\n", i);
      *pos = i;
      return true;
    }
    i = (i + 1) % table->size;
  }
  if( DEBUG2 )
    fprintf(stderr, "] MWTableImplRetrieve returning false (pos %d)\n", i);
  *pos = i;
  return false;
}


/*****************************************************************************/
/*                                                                           */
/*  bool MWTableImplContainsNext(MWTABLE_U *table, int *pos)                 */
/*                                                                           */
/*  It is a precondition of this function that *pos is the index of an       */
/*  entry of table.  Search onwards from that entry for another entry        */
/*  with the same key as this one.  If found, set *pos to the index of       */
/*  that entry and return true; if not found, set *pos to the index of the   */
/*  gap that proves there is no entry, and return false.                     */
/*                                                                           */
/*****************************************************************************/

bool MWTableImplContainsNext(MWTABLE_U *table, int *pos)
{
  wchar_t *key;   int i;
  i = *pos;
  key = table->keys[i];
  if( DEBUG2 )
    fprintf(stderr, "[ MWTableImplRetrieveNext(%p, %d holding \"%ls\")\n",
      (void *) table, i, key);
  i = (i + 1) % table->size;
  while( table->keys[i] != 0x0 )
  {
    if( table->keys[i] > (wchar_t *) 0x1 && wcscmp(key, table->keys[i]) == 0 )
    {
      if( DEBUG2 )
	fprintf(stderr,"] MWTableImplRetrieveNext returning true (pos %d)\n",i);
      *pos = i;
      return true;
    }
    i = (i + 1) % table->size;
  }
  if( DEBUG2 )
    fprintf(stderr, "] MWTableImplRetrieveNext returning false (pos %d)\n", i);
  *pos = i;
  return false;
}


/*****************************************************************************/
/*                                                                           */
/*  void MWTableImplDelete(MWTABLE_U *table, int pos)                        */
/*                                                                           */
/*  Delete the entry at this position.  It must be present.                  */
/*                                                                           */
/*****************************************************************************/

void MWTableImplDelete(MWTABLE_U *table, int pos)
{
  MAssert(table->keys[pos] > (wchar_t *) 0x1, "MWTableDelete: no entry at pos");
  if( DEBUG2 )
    fprintf(stderr, "[ MWTableImplDelete(%p, %d \"%ls\")\n", (void *) table,
      pos, table->keys[pos]);
  if( table->keys[(pos + 1) % table->size] == 0x0 )
  {
    /* safe to set this entry to NULL, since nothing follows */
    table->keys[pos] = 0x0;
    (table->trigger)++;
  }
  else
    table->keys[pos] = (wchar_t *) 0x1;
}


/*****************************************************************************/
/*                                                                           */
/*  Submodule "testing"                                                      */
/*                                                                           */
/*****************************************************************************/

/*****************************************************************************/
/*                                                                           */
/*  void MTestAddToString(ARRAY_CHAR *ac, int n)                             */
/*                                                                           */
/*  Test string factory (char * form)                                        */
/*                                                                           */
/*****************************************************************************/

static void MTestAddToString(ARRAY_CHAR *ac, int n)
{
  int i;
  for( i = 1;  i <= n;  i++ )
    MStringPrintf(*ac, 10, "%s%d", i == 1 ? "{" : ", ", i);
  MStringAddString(*ac, "}");
}


/*****************************************************************************/
/*                                                                           */
/*  void MWTestAddToString(ARRAY_WCHAR *awc, int n)                          */
/*                                                                           */
/*  Test string factory (wchar_t * form)                                     */
/*                                                                           */
/*****************************************************************************/

static void MWTestAddToString(ARRAY_WCHAR *awc, int n)
{
  int i;
  for( i = 1;  i <= n;  i++ )
    MWStringPrintf(*awc, 10, L"%ls%d", i == 1 ? L"{" : L", ", i);
  MWStringAddString(*awc, L"}");
}


/*****************************************************************************/
/*                                                                           */
/*  MTEST MTestMake(int a, int b)                                            */
/*                                                                           */
/*  Make a non-trivial value for storing in a symbol table.                  */
/*                                                                           */
/*****************************************************************************/

typedef struct test_rec {
  int a;
  int b;
} MTEST;

static MTEST MTestMake(int a, int b)
{
  MTEST res;
  res.a = a;
  res.b = b;
  return res;
}


/*****************************************************************************/
/*                                                                           */
/*  void MTestInit(TABLE_MTEST *table, FILE *fp)                             */
/*  void MTestClear(TABLE_MTEST *table, FILE *fp)                            */
/*  void MTestInsert(TABLE_MTEST *table, char *key, int a, int b, FILE *fp)  */
/*  void MTestInsertUnique(TABLE_MTEST *table, char *key, int a, int b,      */
/*    FILE *fp)                                                              */
/*  void MTestRetrieve(TABLE_MTEST *table, char *key, int *pos, FILE *fp)    */
/*  void MTestRetrieveNext(TABLE_MTEST *table, int *pos, FILE *fp)           */
/*  void MTestDelete(TABLE_MTEST *table, int pos, FILE *fp)                  */
/*  void MTestForEachWithKey(TABLE_MTEST *table, char *key, FILE *fp)        */
/*  void MTestForEach(TABLE_MTEST *table, FILE *fp)                          */
/*                                                                           */
/*  Function for testing symbol tables (char * form).                        */
/*                                                                           */
/*****************************************************************************/

typedef MTABLE(MTEST) TABLE_MTEST;

static void MTestInit(TABLE_MTEST *table, FILE *fp)
{
  fprintf(fp, "  MTableInit(table)\n");
  MTableInit(*table);
}

static void MTestClear(TABLE_MTEST *table, FILE *fp)
{
  fprintf(fp, "  MTableClear(table)\n");
  MTableClear(*table);
}

static void MTestInsert(TABLE_MTEST *table, char *key, int a, int b, FILE *fp)
{
  fprintf(fp, "  MTableInsert(table, \"%s\", (%d,%d))\n", key, a, b);
  MTableInsert(*table, key, MTestMake(a, b));
}

static void MTestInsertUnique(TABLE_MTEST *table, char *key, int a, int b,
  FILE *fp)
{
  MTEST mt;
  fprintf(fp, "  MTestInsertUnique(table, \"%s\", (%d,%d)) = %s\n", key, a, b,
    MTableInsertUnique(*table, key, MTestMake(a, b), &mt) ? "true" : "false");
}

static void MTestRetrieve(TABLE_MTEST *table, char *key, int *pos, FILE *fp)
{
  MTEST mt;
  fprintf(fp, "  MTableRetrieve(table, \"%s\", &mt, &pos) = ", key);
  if( MTableRetrieve(*table, key, &mt, pos) )
    fprintf(fp, "true, mt = (%d,%d), pos = %d\n", mt.a, mt.b, *pos);
  else
    fprintf(fp, "false\n");
}

static void MTestRetrieveNext(TABLE_MTEST *table, int *pos, FILE *fp)
{
  MTEST mt;
  fprintf(fp, "  MTableRetrieveNext(table, &mt, &pos %d) = ", *pos);
  if( MTableRetrieveNext(*table, &mt, pos) )
    fprintf(fp, " true, mt = (%d,%d), pos = %d\n", mt.a, mt.b, *pos);
  else
    fprintf(fp, " false\n");
}

static void MTestDelete(TABLE_MTEST *table, int pos, FILE *fp)
{
  fprintf(fp, "  MTableDelete(table, %d)\n", pos);
  MTableDelete(*table, pos);
}

static void MTestForEachWithKey(TABLE_MTEST *table, char *key, FILE *fp)
{
  MTEST mt;  int pos;
  fprintf(fp, "  MTableForEachWithKey(table, %s, value, pos):\n", key);
  MTableForEachWithKey(*table, key, &mt, &pos)
    fprintf(fp, "    %d: (\"%s\", (%d,%d))\n", pos, key, mt.a, mt.b);
}

static void MTestForEach(TABLE_MTEST *table, FILE *fp)
{
  char *key;  MTEST mt;  int pos;
  fprintf(fp, "  MTableForEach(table, key, value, pos):\n");
  MTableForEach(*table, &key, &mt, &pos)
    fprintf(fp, "    %d: (\"%s\", (%d,%d))\n", pos, key, mt.a, mt.b);
}


/*****************************************************************************/
/*                                                                           */
/*  void MWTestInit(WTABLE_MTEST *table, FILE *fp)                           */
/*  void MWTestClear(WTABLE_MTEST *table, FILE *fp)                          */
/*  void MWTestInsert(WTABLE_MTEST *table, wchar_t *key, int a, int b,       */
/*    FILE *fp)                                                              */
/*  void MWTestInsertUnique(WTABLE_MTEST *table, wchar_t *key, int a, int b, */
/*    FILE *fp)                                                              */
/*  void MWTestRetrieve(WTABLE_MTEST *table, wchar_t *key, int *pos,FILE *fp)*/
/*  void MWTestRetrieveNext(WTABLE_MTEST *table, int *pos, FILE *fp)         */
/*  void MWTestDelete(WTABLE_MTEST *table, int pos, FILE *fp)                */
/*  void MWTestForEachWithKey(WTABLE_MTEST *table, wchar_t *key, FILE *fp)   */
/*  void MWTestForEach(WTABLE_MTEST *table, FILE *fp)                        */
/*                                                                           */
/*  Functions for testing symbol tables (wchar_t * form).                    */
/*                                                                           */
/*****************************************************************************/

typedef MWTABLE(MTEST) WTABLE_MTEST;

static void MWTestInit(WTABLE_MTEST *table, FILE *fp)
{
  fprintf(fp, "  MWTableInit(table)\n");
  MWTableInit(*table);
}

static void MWTestClear(WTABLE_MTEST *table, FILE *fp)
{
  fprintf(fp, "  MWTableClear(table)\n");
  MWTableClear(*table);
}

static void MWTestInsert(WTABLE_MTEST *table, wchar_t *key, int a, int b,
  FILE *fp)
{
  fprintf(fp, "  MWTableInsert(table, L\"%ls\", (%d,%d))\n", key, a, b);
  MWTableInsert(*table, key, MTestMake(a, b));
}

static void MWTestInsertUnique(WTABLE_MTEST *table, wchar_t *key, int a, int b,
  FILE *fp)
{
  MTEST mt;
  fprintf(fp, "  MWTestInsertUnique(table, L\"%ls\", (%d,%d)) = %s\n", key,a,b,
    MWTableInsertUnique(*table, key, MTestMake(a, b), &mt) ? "true" : "false");
}

static void MWTestRetrieve(WTABLE_MTEST *table, wchar_t *key, int *pos, FILE *fp)
{
  MTEST mt;
  fprintf(fp, "  MWTableRetrieve(table, L\"%ls\", &mt, &pos) = ", key);
  if( MWTableRetrieve(*table, key, &mt, pos) )
    fprintf(fp, "true, mt = (%d,%d), pos = %d\n", mt.a, mt.b, *pos);
  else
    fprintf(fp, "false\n");
}

static void MWTestRetrieveNext(WTABLE_MTEST *table, int *pos, FILE *fp)
{
  MTEST mt;
  fprintf(fp, "  MWTableRetrieveNext(table, &mt, &pos %d) = ", *pos);
  if( MWTableRetrieveNext(*table, &mt, pos) )
    fprintf(fp, " true, mt = (%d,%d), pos = %d\n", mt.a, mt.b, *pos);
  else
    fprintf(fp, " false\n");
}

static void MWTestDelete(WTABLE_MTEST *table, int pos, FILE *fp)
{
  fprintf(fp, "  MWTableDelete(table, %d)\n", pos);
  MWTableDelete(*table, pos);
}

static void MWTestForEachWithKey(WTABLE_MTEST *table, wchar_t *key, FILE *fp)
{
  MTEST mt;  int pos;
  fprintf(fp, "  MWTableForEachWithKey(table, L\"%ls\", value, pos):\n", key);
  MWTableForEachWithKey(*table, key, &mt, &pos)
    fprintf(fp, "    %d: (L\"%ls\", (%d,%d))\n", pos, key, mt.a, mt.b);
}

static void MWTestForEach(WTABLE_MTEST *table, FILE *fp)
{
  wchar_t *key;  MTEST mt;  int pos;
  fprintf(fp, "  MWTableForEach(table, key, value, pos):\n");
  MWTableForEach(*table, &key, &mt, &pos)
    fprintf(fp, "    %d: (L\"%ls\", (%d,%d))\n", pos, key, mt.a, mt.b);
}


/*****************************************************************************/
/*                                                                           */
/*  void MTest(FILE *fp)                                                     */
/*                                                                           */
/*  Test the M module, especially the symbol table code.                     */
/*                                                                           */
/*****************************************************************************/

void MTest(FILE *fp)
{
  TABLE_MTEST table;  int pos;  ARRAY_CHAR ac;  ARRAY_WCHAR awc;
  WTABLE_MTEST wtable;

  fprintf(fp, "[ MTest(fp)\n");

  /* test string factory (char * form) */
  fprintf(fp, "string factory test (char * form):\n");
  MStringInit(ac);
  MTestAddToString(&ac, 100);
  fprintf(fp, "  %s\n", MStringVal(ac));
  MArrayFree(ac);

  /* test string factory (wchar_t * form) */
  fprintf(fp, "string factory test (wchar_t * form):\n");
  MWStringInit(awc);
  MWTestAddToString(&awc, 100);
  fprintf(fp, "  %ls\n", MWStringVal(awc));
  MArrayFree(awc);

  /* symbol table test (char * form): build the table */
  fprintf(fp, "\n  symbol table test (char * form):\n");
  MTestInit(&table, fp);
  MTestInsertUnique(&table, "12", 1, 2, fp);
  MTestInsertUnique(&table, "23", 2, 3, fp);
  MTestInsertUnique(&table, "34", 3, 4, fp);
  MTestInsertUnique(&table, "12", 1, 2, fp);
  MTestInsert(&table, "23", 2, 3, fp);
  MTestInsert(&table, "45", 4, 5, fp);
  MTestInsert(&table, "56", 5, 6, fp);
  MTestInsert(&table, "67", 6, 7, fp);
  MTestInsert(&table, "45", 4, 5, fp);

  /* show it all */
  MTestForEach(&table, fp);

  /* do some retrievals */
  MTestRetrieve(&table, "56", &pos, fp);
  MTestRetrieveNext(&table, &pos, fp);
  MTestRetrieve(&table, "67", &pos, fp);
  MTestDelete(&table, pos, fp);
  MTestRetrieve(&table, "78", &pos, fp);

  /* do some traversals */
  MTestForEachWithKey(&table, "23", fp);
  MTestForEachWithKey(&table, "67", fp);
  MTestForEachWithKey(&table, "78", fp);

  /* clear and show again */
  MTestClear(&table, fp);
  MTestForEach(&table, fp);

  /* symbol table test (wchar_t * form): build the table */
  fprintf(fp, "\n  symbol table test (wchar_t * form):\n");
  MWTestInit(&wtable, fp);
  MWTestInsertUnique(&wtable, L"12", 1, 2, fp);
  MWTestInsertUnique(&wtable, L"23", 2, 3, fp);
  MWTestInsertUnique(&wtable, L"34", 3, 4, fp);
  MWTestInsertUnique(&wtable, L"12", 1, 2, fp);
  MWTestInsert(&wtable, L"23", 2, 3, fp);
  MWTestInsert(&wtable, L"45", 4, 5, fp);
  MWTestInsert(&wtable, L"56", 5, 6, fp);
  MWTestInsert(&wtable, L"67", 6, 7, fp);
  MWTestInsert(&wtable, L"45", 4, 5, fp);

  /* show it all */
  MWTestForEach(&wtable, fp);

  /* do some retrievals */
  MWTestRetrieve(&wtable, L"56", &pos, fp);
  MWTestRetrieveNext(&wtable, &pos, fp);
  MWTestRetrieve(&wtable, L"67", &pos, fp);
  MWTestDelete(&wtable, pos, fp);
  MWTestRetrieve(&wtable, L"78", &pos, fp);

  /* do some traversals */
  MWTestForEachWithKey(&wtable, L"23", fp);
  MWTestForEachWithKey(&wtable, L"67", fp);
  MWTestForEachWithKey(&wtable, L"78", fp);

  /* clear and show again */
  MWTestClear(&wtable, fp);
  MWTestForEach(&wtable, fp);

  fprintf(fp, "]\n");
}
