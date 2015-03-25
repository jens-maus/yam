/***************************************************************************

 YAM - Yet Another Mailer
 Copyright (C) 1995-2000 Marcel Beck
 Copyright (C) 2000-2015 YAM Open Source Team

 This program is free software; you can redistribute it and/or modify
 it under the terms of the GNU General Public License as published by
 the Free Software Foundation; either version 2 of the License, or
 (at your option) any later version.

 This program is distributed in the hope that it will be useful,
 but WITHOUT ANY WARRANTY; without even the implied warranty of
 MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 GNU General Public License for more details.

 You should have received a copy of the GNU General Public License
 along with this program; if not, write to the Free Software
 Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA

 YAM Official Support Site :  http://www.yam.ch
 YAM OpenSource project    :  http://sourceforge.net/projects/yamos/

 $Id$

***************************************************************************/

#include <string.h>
#include <stdlib.h>

#include "YAM_utilities.h"

#include "HashTable.h"

#include "Debug.h"

/*** Macro definitions ***/
// double hashing needs the second hash code to be relatively prime to table size, so we
// simply make hash2 odd.
#define HASH1(hash0, shift)                 ((hash0) >> (shift))
#define HASH2(hash0, log2, shift)           ((((hash0) << (log2)) >> (shift)) | 1)

#define COLLISION_FLAG                      ((ULONG)1)
#define MARK_ENTRY_FREE(entry)              ((entry)->keyHash = 0)
#define MARK_ENTRY_REMOVED(entry)           ((entry)->keyHash = 1)
#define ENTRY_IS_REMOVED(entry)             ((entry)->keyHash == 1)
#define ENSURE_LIVE_KEYHASH(hash0)          if(hash0 < 2) hash0 -= 2; else (void)0;
#define MATCH_ENTRY_KEYHASH(entry, hash0)   (((entry)->keyHash & ~COLLISION_FLAG) == (hash0))
#define ADDRESS_ENTRY(table, index)         ((struct HashEntryHeader *)((table)->entryStore + (index) * (table)->entrySize))

#define MAX_LOAD(table, size)               (((table)->maxAlphaFrac * (size)) >> 8)
#define MIN_LOAD(table, size)               (((table)->minAlphaFrac * (size)) >> 8)

#define CEILING_LOG2(_log2, _n) \
do { \
  ULONG j_ = (ULONG)(_n); \
  (_log2) = 0; \
  if((j_) & (j_-1)) (_log2) +=  1; \
  if((j_) >> 16)    (_log2) += 16, (j_) >>= 16; \
  if((j_) >>  8)    (_log2) +=  8, (j_) >>=  8; \
  if((j_) >>  4)    (_log2) +=  4, (j_) >>=  4; \
  if((j_) >>  2)    (_log2) +=  2, (j_) >>=  2; \
  if((j_) >>  1)    (_log2) +=  1; \
} while(0)

/*** Static functions ***/
/// SearchTable()
//
static struct HashEntryHeader *SearchTable(struct HashTable *table, const void *key, ULONG keyHash, enum HashTableOperator op)
{
  ULONG hash1, hash2;
  LONG hashShift, sizeLog2;
  struct HashEntryHeader *entry, *firstRemoved;
  BOOL (* matchEntry)(struct HashTable *, const struct HashEntryHeader *, const void *);
  ULONG sizeMask;

  ENTER();

  // compute the primary hash address
  hashShift = table->shift;
  hash1 = HASH1(keyHash, hashShift);
  entry = ADDRESS_ENTRY(table, hash1);

  // miss: return space for a new entry
  if(HASH_ENTRY_IS_FREE(entry))
  {
    //D(DBF_HASH, "search miss, creating new entry");
    RETURN(entry);
    return entry;
  }

  // hit: return entry
  matchEntry = table->ops->matchEntry;
  if(MATCH_ENTRY_KEYHASH(entry, keyHash) && matchEntry(table, entry, key))
  {
    //D(DBF_HASH, "search hit, returning old entry");
    RETURN(entry);
    return entry;
  }

  // collision: hash again
  //D(DBF_HASH, "search collision, hashing again");
  sizeLog2 = HASH_BITS - table->shift;
  hash2 = HASH2(keyHash, sizeLog2, hashShift);
  sizeMask = (1UL << sizeLog2) - 1;

  // save the first removed entry pointer so htoAdd can recycle it
  if(ENTRY_IS_REMOVED(entry))
    firstRemoved = entry;
  else
  {
    firstRemoved = NULL;
    if(op == htoAdd)
      entry->keyHash |= COLLISION_FLAG;
  }

  for(;;)
  {
    hash1 -= hash2;
    hash1 &= sizeMask;

    entry = ADDRESS_ENTRY(table, hash1);
    if(HASH_ENTRY_IS_FREE(entry))
    {
      //D(DBF_HASH, "search miss, creating new entry");
      if(firstRemoved != NULL && op == htoAdd)
        entry = firstRemoved;
      RETURN(entry);
      return entry;
    }

    if(MATCH_ENTRY_KEYHASH(entry, keyHash) && matchEntry(table, entry, key))
    {
      //D(DBF_HASH, "search hit, returning old entry");
      RETURN(entry);
      return entry;
    }

    if(ENTRY_IS_REMOVED(entry))
    {
      if(firstRemoved == NULL)
        firstRemoved = entry;
    }
    else if(op == htoAdd)
      entry->keyHash |= COLLISION_FLAG;
  }

  // this is never reached
  RETURN(NULL);
  return NULL;
}

///
/// ChangeTable()
//
static BOOL ChangeTable(struct HashTable *table, LONG deltaLog2)
{
  BOOL result = FALSE;
  LONG oldLog2, newLog2;
  ULONG oldCapacity, newCapacity;
  char *oldEntryStore, *newEntryStore, *oldEntryAddr;
  ULONG i;
  struct HashEntryHeader *oldEntry, *newEntry;

  ENTER();

  SHOWVALUE(DBF_HASH, deltaLog2);

  oldLog2 = HASH_BITS - table->shift;
  newLog2 = oldLog2 + deltaLog2;
  oldCapacity = 1UL << oldLog2;
  newCapacity = 1UL << newLog2;

  SHOWVALUE(DBF_HASH, oldCapacity);
  SHOWVALUE(DBF_HASH, newCapacity);

  if(newCapacity < HASH_SIZE_LIMIT)
  {
    ULONG entrySize;

    entrySize = table->entrySize;

    if((newEntryStore = table->ops->allocTable(table, newCapacity, entrySize)) != NULL)
    {
      const void * (* getKey)(struct HashTable *, const struct HashEntryHeader *);
      void         (* moveEntry)(struct HashTable *, const struct HashEntryHeader *, struct HashEntryHeader *);

      getKey = table->ops->getKey;
      moveEntry = table->ops->moveEntry;

      oldEntryAddr = table->entryStore;
      oldEntryStore = table->entryStore;

      // assign the new entry store to the table
      table->shift = HASH_BITS - newLog2;
      table->removedCount = 0;
      table->generation++;
      table->entryStore = newEntryStore;

      // copy all live nodes, leaving removed ones behind
      for(i = 0; i < oldCapacity; i++)
      {
        oldEntry = (struct HashEntryHeader *)oldEntryAddr;
        if(HASH_ENTRY_IS_LIVE(oldEntry))
        {
          oldEntry->keyHash &= ~COLLISION_FLAG;
          newEntry = SearchTable(table, getKey(table, oldEntry), oldEntry->keyHash, htoAdd);
          moveEntry(table, oldEntry, newEntry);
          newEntry->keyHash = oldEntry->keyHash;
        }
        oldEntryAddr += entrySize;
      }
      table->ops->freeTable(table, oldEntryStore);

      result = TRUE;
    }
  }

  RETURN(result);
  return result;
}

///

/*** Public default operator functions ***/
/// DefaultHashAllocTable()
//
void *DefaultHashAllocTable(UNUSED struct HashTable *table, ULONG capacity, ULONG entrySize)
{
  void *result;

  ENTER();

  result = calloc(capacity, entrySize);

  RETURN(result);
  return result;
}

///
/// DefaultHashFreeTable()
//
void DefaultHashFreeTable(UNUSED struct HashTable *table, void *ptr)
{
  ENTER();

  free(ptr);

  LEAVE();
}

///
/// DefaultHashGetKey()
//
const void *DefaultHashGetKey(UNUSED struct HashTable *table, const struct HashEntryHeader *entry)
{
  struct HashEntry *stub = (struct HashEntry *)entry;
  const void *result;

  ENTER();

  result = stub->key;

  RETURN(result);
  return result;
}

///
/// DefaultHashHashKey()
//
ULONG DefaultHashHashKey(UNUSED struct HashTable *table, const void *key)
{
  ULONG result;

  ENTER();

  result = (ULONG)((ULONG)key >> 2);

  RETURN(result);
  return result;
}

///
/// DefaultHashMatchEntry()
//
BOOL DefaultHashMatchEntry(UNUSED struct HashTable *table, const struct HashEntryHeader *entry, const void *key)
{
  struct HashEntry *stub = (struct HashEntry *)entry;
  BOOL result;

  ENTER();

  result = (stub->key == key);

  RETURN(result);
  return result;
}

///
/// DefaultHashMoveEntry()
//
void DefaultHashMoveEntry(struct HashTable *table, const struct HashEntryHeader *from, struct HashEntryHeader *to)
{
  ENTER();

  memcpy(to, from, table->entrySize);

  LEAVE();
}

///
/// DefaultHashClearEntry()
// no ENTER/RETURN macro calls on purpose as this would blow up the trace log too much
void DefaultHashClearEntry(struct HashTable *table, struct HashEntryHeader *entry)
{
  memset(entry, 0, table->entrySize);
}

///
/// DefaultHashFinalize()
//
void DefaultHashFinalize(UNUSED struct HashTable *table)
{
  ENTER();

  // nop

  LEAVE();
}

///

/*** Public string operator functions ***/
/// StringHashHashKey()
//
ULONG StringHashHashKey(UNUSED struct HashTable *table, const void *key)
{
  ULONG h = 0;
  const unsigned char *s = (const unsigned char *)key;

  ENTER();

  if(s != NULL)
  {
    for(s = key; *s != '\0'; s++)
      h = (h >> (HASH_BITS - 4)) ^ (h << 4) ^ *s;
  }
  else
    E(DBF_HASH, "StringHashHashKey called with <NULL> pointer");

  RETURN(h);
  return h;
}

///
/// StringHashMatchEntry()
//
BOOL StringHashMatchEntry(UNUSED struct HashTable *table, const struct HashEntryHeader *entry, const void *key)
{
  struct HashEntry *stub = (struct HashEntry *)entry;
  BOOL result = FALSE;

  ENTER();

  if(stub->key == key || (stub->key != NULL && key != NULL && strcmp(stub->key, key) == 0))
    result = TRUE;

  RETURN(result);
  return result;
}

///
/// StringHashClearEntry()
// no ENTER/RETURN macro calls on purpose as this would blow up the trace log too much
void StringHashClearEntry(struct HashTable *table, struct HashEntryHeader *entry)
{
  struct HashEntry *stub = (struct HashEntry *)entry;

  free(stub->key);
  memset(entry, 0, table->entrySize);
}

///
/// StringHashDestroyEntry()
// no ENTER/RETURN macro calls on purpose as this would blow up the trace log too much
void StringHashDestroyEntry(UNUSED struct HashTable *table, const struct HashEntryHeader *entry)
{
  struct HashEntry *stub = (struct HashEntry *)entry;

  free(stub->key);
}

///

/*** Public functions ***/
/// HashTableNew()
//
struct HashTable *HashTableNew(const struct HashTableOps *ops, void *data, ULONG entrySize, ULONG capacity)
{
  struct HashTable *table;

  ENTER();

  if((table = malloc(sizeof(*table))) != NULL)
  {
    if(HashTableInit(table, ops, data, entrySize, capacity) == FALSE)
    {
      free(table);
      table = NULL;
    }
  }

  RETURN(table);
  return table;
}

///
/// HashTableDestroy()
//
void HashTableDestroy(struct HashTable *table)
{
  ENTER();

  if(table != NULL)
  {
    HashTableCleanup(table);
    free(table);
  }

  LEAVE();
}

///
/// HashTableInit()
//
BOOL HashTableInit(struct HashTable *table, const struct HashTableOps *ops, void *data, ULONG entrySize, ULONG capacity)
{
  BOOL result;
  ULONG log2;

  ENTER();

  result = FALSE;

  table->data = data;
  table->ops = ops;

  if(capacity < HASH_MIN_SIZE)
    capacity = HASH_MIN_SIZE;

  CEILING_LOG2(log2, capacity);
  capacity = 1UL << log2;
  if(capacity < HASH_SIZE_LIMIT)
  {
    table->shift = HASH_BITS - log2;
    table->maxAlphaFrac = 0xc0; // 0.75
    table->minAlphaFrac = 0x40; // 0.25
    table->entrySize = entrySize;
    table->entryCount = 0;
    table->removedCount = 0;
    table->generation = 0;

    if((table->entryStore = ops->allocTable(table, capacity, entrySize)) != NULL)
      result = TRUE;
  }

  RETURN(result);
  return result;
}

///
/// HashTableSetAlphaBound()
//
void HashTableSetAlphaBounds(struct HashTable *table, float maxAlpha, float minAlpha)
{
  ENTER();

  if(maxAlpha >= 0.5 && maxAlpha < 1.0 && minAlpha > 0.0)
  {
    if(minAlpha >= maxAlpha / 2.0)
    {
      LONG size;

      size = HASH_TABLE_SIZE(table);
      minAlpha = (size * maxAlpha - MAX(1, size / 256)) / (2 * size);
    }

    table->maxAlphaFrac = (UBYTE)(maxAlpha * 256);
    table->minAlphaFrac = (UBYTE)(minAlpha * 256);
  }

  LEAVE();
}

///
/// HashTableCleanup()
//
void HashTableCleanup(struct HashTable *table)
{
  ENTER();

  if(table->entryStore != NULL)
  {
    if(table->ops->destroyEntry != NULL)
    {
      void (* destroyEntry)(struct HashTable *table, const struct HashEntryHeader *entry);
      char *entryAddr, *entryLimit;
      ULONG entrySize;

      destroyEntry = table->ops->destroyEntry;
      entryAddr = table->entryStore;
      entrySize = table->entrySize;
      entryLimit = entryAddr + HASH_TABLE_SIZE(table) * entrySize;
      while(entryAddr < entryLimit)
      {
        struct HashEntryHeader *entry;

        entry = (struct HashEntryHeader *)entryAddr;
        if(HASH_ENTRY_IS_LIVE(entry))
          destroyEntry(table, entry);

        entryAddr += entrySize;
      }
    }

    table->ops->freeTable(table, table->entryStore);
    table->entryStore = NULL;
  }

  LEAVE();
}

///
/// HashTableOperate()
//
struct HashEntryHeader *HashTableOperate(struct HashTable *table, const void *key, enum HashTableOperator op)
{
  ULONG keyHash;
  struct HashEntryHeader *entry = NULL;

  ENTER();

  keyHash = table->ops->hashKey(table, key);
  keyHash *= HASH_GOLDEN_RATIO;

  // avoid 0 and 1 hash codes, they indicate free and removed entries
  ENSURE_LIVE_KEYHASH(keyHash);
  keyHash &= ~COLLISION_FLAG;

  switch(op)
  {
    case htoLookup:
    {
      entry = SearchTable(table, key, keyHash, op);
    }
    break;

    case htoAdd:
    {
      ULONG size;
      LONG deltaLog2;
      BOOL goOn = TRUE;

      // if alpha is >= 0.75 grow or compress the table. If key is already in the table,
      // we may grow once more, but only if we are on the edge of being overloaded
      size = HASH_TABLE_SIZE(table);
      if(table->entryCount + table->removedCount >= MAX_LOAD(table, size))
      {
        // compress if a quarter or more of all entries are removed
        if(table->removedCount >= size >> 2)
          deltaLog2 = 0; // compress
        else
          deltaLog2 = 1; // grow

        if(ChangeTable(table, deltaLog2) == FALSE && table->entryCount + table->removedCount == size - 1)
          goOn = FALSE;
      }

      if(goOn == TRUE)
      {
        // look for entry after possibly growing, so we don't have to add it, then skip it
        // while growing the table and readd it after
        entry = SearchTable(table, key, keyHash, op);
        if(!HASH_ENTRY_IS_LIVE(entry))
        {
          // initialize the entry, indicating it is no longer free
          if(ENTRY_IS_REMOVED(entry))
          {
            table->removedCount--;
            keyHash |= COLLISION_FLAG;
          }
          if(table->ops->initEntry != NULL && table->ops->initEntry(table, entry, key) == FALSE)
          {
            // we haven't claimed entry yet; fail with NULL return.
            memset(entry + 1, 0, table->entrySize - sizeof(*entry));
            entry = NULL;
          }
          else
          {
            entry->keyHash = keyHash;
            table->entryCount++;
          }
        }
      }
    }
    break;

    case htoRemove:
    {
      entry = SearchTable(table, key, keyHash, op);
      if(HASH_ENTRY_IS_LIVE(entry))
      {
        ULONG size;

        // clear this entry and mark it as removed
        HashTableRawRemove(table, entry);

        // shrink, if alpha is <= 0.25 and table is not too small already
        size = HASH_TABLE_SIZE(table);
        if(size > HASH_MIN_SIZE && table->entryCount <= MIN_LOAD(table, size))
          ChangeTable(table, -1);

        entry = NULL;
      }
    }
    break;

    default:
    {
      // nothing
    }
    break;
  }

  RETURN(entry);
  return entry;
}

///
/// HashTableRawRemove()
//
void HashTableRawRemove(struct HashTable *table, struct HashEntryHeader *entry)
{
  ULONG keyHash;

  // copy the hash value before we free/clear the entry
  keyHash = entry->keyHash;

  table->ops->clearEntry(table, entry);

  if(keyHash & COLLISION_FLAG)
  {
    // this entry collided with another entry, so it must not be marked
    // as free, but as removed instead. HashTableOperate() knows how to
    // handle this case.
    MARK_ENTRY_REMOVED(entry);
    table->removedCount++;
  }
  else
  {
    // this entry didn't cause a collision, so it can be marked as free
    MARK_ENTRY_FREE(entry);
  }

  table->entryCount--;
}

///
/// HashTableGetDefaultOps()
//
const struct HashTableOps *HashTableGetDefaultOps(void)
{
  static const struct HashTableOps defaultOps =
  {
    DefaultHashAllocTable,
    DefaultHashFreeTable,
    DefaultHashGetKey,
    DefaultHashHashKey,
    DefaultHashMatchEntry,
    DefaultHashMoveEntry,
    DefaultHashClearEntry,
    DefaultHashFinalize,
    NULL,
    NULL
  };

  ENTER();
  RETURN(&defaultOps);
  return &defaultOps;
}

///
/// HashTableGetDefaultStringOps()
//
const struct HashTableOps *HashTableGetDefaultStringOps(void)
{
  static const struct HashTableOps defaultStringOps =
  {
    DefaultHashAllocTable,
    DefaultHashFreeTable,
    DefaultHashGetKey,
    StringHashHashKey,
    StringHashMatchEntry,
    DefaultHashMoveEntry,
    StringHashClearEntry,
    DefaultHashFinalize,
    NULL,
    StringHashDestroyEntry
  };

  ENTER();
  RETURN(&defaultStringOps);
  return &defaultStringOps;
}

///
/// HashTableEnumerate()
//
ULONG HashTableEnumerate(struct HashTable *table, enum HashTableOperator (* etor)(struct HashTable *, struct HashEntryHeader *, ULONG, void *), void *arg)
{
  char *entryAddr, *entryLimit;
  ULONG i, capacity, entrySize, ceiling;
  BOOL didRemove;
  struct HashEntryHeader *entry;
  enum HashTableOperator op;

  ENTER();

  entryAddr = table->entryStore;
  entrySize = table->entrySize;
  capacity = HASH_TABLE_SIZE(table);
  entryLimit = entryAddr + capacity * entrySize;
  i = 0;
  didRemove = FALSE;

  while(entryAddr < entryLimit)
  {
    entry = (struct HashEntryHeader *)entryAddr;
    if(HASH_ENTRY_IS_LIVE(entry))
    {
      op = etor(table, entry, i++, arg);
      if(isAnyFlagSet(op, htoRemove))
      {
        HashTableRawRemove(table, entry);
        didRemove = TRUE;
      }
      if(isAnyFlagSet(op, htoStop))
        break;
    }

    entryAddr += entrySize;
  }

  // Shrink or compress if a quarter or more of all entries are removed, or
  // if the table is underloaded according to the configured minimum alpha,
  // and is not minimal-size already.  Do this only if we removed above, so
  // non-removing enumerations can count on stable table->entryStore until
  // the next non-lookup-Operate or removing-Enumerate.
  if(didRemove == TRUE &&
     (table->removedCount >= capacity >> 2 ||
      (capacity > HASH_MIN_SIZE && table->entryCount <= MIN_LOAD(table, capacity))))
  {
    capacity = table->entryCount;
    capacity += capacity >> 1;
    if(capacity < HASH_MIN_SIZE)
      capacity = HASH_MIN_SIZE;

    CEILING_LOG2(ceiling, capacity);
    ceiling -= HASH_BITS - table->shift;

    ChangeTable(table, ceiling);
  }

  RETURN(i);
  return i;
}

///

/*** Testcase ***/
/// Testcase
// a little bit of demonstration code on how to use the hash table functions
/*
enum StatusImages
{
  si_First = 0,
  si_Attach = 0,
  si_Crypt,
  si_Delete,
  si_Download,
  si_Error,
  si_Forward,
  si_Group,
  si_Hold,
  si_Mark,
  si_New,
  si_Old,
  si_Reply,
  si_Report,
  si_Sent,
  si_Signed,
  si_Spam,
  si_Unread,
  si_Urgent,
  si_WaitSend,
  si_Max
};
static const char * const statusImageIDs[si_Max] =
{
  "status_attach",
  "status_crypt",
  "status_delete",
  "status_download",
  "status_error",
  "status_forward",
  "status_group",
  "status_hold",
  "status_mark",
  "status_new",
  "status_old",
  "status_reply",
  "status_report",
  "status_sent",
  "status_signed",
  "status_spam",
  "status_unread",
  "status_urgent",
  "status_waitsend",
};
struct TestHashNode
{
  struct HashEntryHeader hash;
  char *str;
  int cnt;
};

static struct HashTable *table;

static enum HashTableOperator HashTableTestDump(UNUSED struct HashTable *table, struct HashEntryHeader *entry, UNUSED ULONG number, UNUSED void *arg)
{
  struct TestHashNode *node = (struct TestHashNode *)entry;

  ENTER();

  D(DBF_HASH, "  node %08lx", node);
  D(DBF_HASH, "    hash key         %08lx", node->hash.keyHash);
  D(DBF_HASH, "    str              '%s'", node->str);
  D(DBF_HASH, "    cnt              %ld", node->cnt);

  RETURN(htoNext);
  return htoNext;
}
static void HashTableTestSearch(void)
{
  int j;

  D(DBF_HASH, "search: test table contents");
  for(j = 0; j < si_Max; j++)
  {
    struct TestHashNode *node;

    node = (struct TestHashNode *)HashTableOperate(table, statusImageIDs[j], htoLookup);
    if(HASH_ENTRY_IS_LIVE((struct HashEntryHeader *)node))
    {
      D(DBF_HASH, "  found '%s': str='%s', cnt=%ld", statusImageIDs[j], node->str, node->cnt);
    }
     else
    {
       D(DBF_HASH, "  cannot find '%s'", statusImageIDs[j]);
    }
  }

  D(DBF_HASH, "enumerate: test table contents");
  HashTableEnumerate(table, HashTableTestDump, NULL);
}

void HashTableTest(void)
{
  static const struct HashTableOps hashTableOps =
  {
    DefaultHashAllocTable,
    DefaultHashFreeTable,
    DefaultHashGetKey,
    StringHashHashKey,
    StringHashMatchEntry,
    DefaultHashMoveEntry,
    StringHashClearEntry,
    DefaultHashFinalize,
    NULL,
    NULL
  };

  if((table = HashTableNew((struct HashTableOps *)&hashTableOps, NULL, sizeof(struct TestHashNode), 32)) != NULL)
  {
    int j;
    struct TestHashNode *node;

    for(j = 0; j < 5; j++)
    {
      int i;

      for(i = 0; i < si_Max; i++)
      {
        if((node = (struct TestHashNode *)HashTableOperate(table, statusImageIDs[i], htoAdd)) != NULL)
        {
          if(node->str == NULL)
          {
            node->str = strdup(statusImageIDs[i]);
            node->cnt = 1;
          }
          else
          {
            node->cnt++;
          }
        }
      }
    }

    HashTableTestSearch();

    node = (struct TestHashNode *)HashTableOperate(table, statusImageIDs[si_Delete], htoLookup);
    if(HASH_ENTRY_IS_LIVE((struct HashEntryHeader *)node))
    {
      node->cnt--;
      D(DBF_HASH, "<cnt> of \"delete\" reduced");
    }

    HashTableTestSearch();

    node = (struct TestHashNode *)HashTableOperate(table, statusImageIDs[si_Old], htoLookup);
    if(HASH_ENTRY_IS_LIVE((struct HashEntryHeader *)node))
    {
      HashTableRawRemove(table, (struct HashEntryHeader *)node);
      D(DBF_HASH, "\"old\" removed");
    }

    HashTableTestSearch();

    node = (struct TestHashNode *)HashTableOperate(table, statusImageIDs[si_New], htoLookup);
    if(HASH_ENTRY_IS_LIVE((struct HashEntryHeader *)node))
    {
      HashTableRawRemove(table, (struct HashEntryHeader *)node);
      D(DBF_HASH, "\"new\" removed");
    }

    HashTableTestSearch();

    node = (struct TestHashNode *)HashTableOperate(table, statusImageIDs[si_Delete], htoLookup);
    if(HASH_ENTRY_IS_LIVE((struct HashEntryHeader *)node))
    {
      HashTableRawRemove(table, (struct HashEntryHeader *)node);
      D(DBF_HASH, "\"delete\" removed");
    }

    HashTableTestSearch();

    node = (struct TestHashNode *)HashTableOperate(table, statusImageIDs[si_Urgent], htoLookup);
    if(HASH_ENTRY_IS_LIVE((struct HashEntryHeader *)node))
    {
      HashTableRawRemove(table, (struct HashEntryHeader *)node);
      D(DBF_HASH, "\"urgent\" removed");
    }

    HashTableTestSearch();

    HashTableDestroy(table);
  }
}
*/
///
