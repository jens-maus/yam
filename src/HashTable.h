#ifndef HASH_TABLE_H
#define HASH_TABLE_H 1

/***************************************************************************

 YAM - Yet Another Mailer
 Copyright (C) 1995-2000 Marcel Beck
 Copyright (C) 2000-2014 YAM Open Source Team

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

/*
 YAM's hash table implementation is based upon Mozilla Thunderbird's hash tables.
 For further information on Thunderbird go to http://www.mozilla.com.

 The core functions of the ThunderBird 2.0.0.0 hash tables can be found here:
 http://mxr.mozilla.org/mozilla1.8/source/xpcom/glue/pldhash.c
 http://mxr.mozilla.org/mozilla1.8/source/xpcom/glue/pldhash.h
*/

#include <exec/types.h>

#include "SDI_compiler.h"

// forward declarations
struct HashTable;

// Table entry header structure.
//
// In order to allow in-line allocation of key and value, we do not declare
// either here.  Instead, the API uses const void *key as a formal parameter,
// and asks each entry for its key when necessary via a getKey callback, used
// when growing or shrinking the table.  Other callback types are defined
// below and grouped into the HashTableOps structure, for single static
// initialization per hash table sub-type.
//
// Each hash table sub-type should nest the HashEntryHeader structure at the
// front of its particular entry type.  The keyHash member contains the result
// of multiplying the hash code returned from the hashKey callback (see below)
// by HASH_GOLDEN_RATIO, then constraining the result to avoid the magic 0
// and 1 values.  The stored keyHash value is table size invariant, and it is
// maintained automatically by HashTableOperate -- users should never set
// it, and its only uses should be via the entry macros below.
//
// The HASH_ENTRY_IS_LIVE macro tests whether entry is neither free nor
// removed. An entry may be either busy or free; if busy, it may be live or
// removed. Consumers of this API should not access members of entries that
// are not live.
//
// However, use HASH_ENTRY_IS_BUSY for faster liveness testing of entries
// returned by HashTableOperate, as HashTableOperate never returns a non-live,
// busy (i.e., removed) entry pointer to its caller.
struct HashEntryHeader
{
  ULONG keyHash;
};

struct HashEntry
{
  struct HashEntryHeader header;
  void *key;
};

struct HashTableOps
{
  // Mandatory hooks. All implementations must provide these.
  void *       (* allocTable)(struct HashTable *table, ULONG capacity, ULONG entrySize);
  void         (* freeTable)(struct HashTable *table, void *ptr);
  const void * (* getKey)(struct HashTable *table, const struct HashEntryHeader *entry);
  ULONG        (* hashKey)(struct HashTable *table, const void *key);
  BOOL         (* matchEntry)(struct HashTable *table, const struct HashEntryHeader *entry, const void *key);
  void         (* moveEntry)(struct HashTable *table, const struct HashEntryHeader *from, struct HashEntryHeader *to);
  void         (* clearEntry)(struct HashTable *table, struct HashEntryHeader *entry);
  void         (* finalize)(struct HashTable *table);

  // Optional hooks start here. If NULL, these are not called.
  BOOL         (* initEntry)(struct HashTable *table, const struct HashEntryHeader *entry, const void *key);
  void         (* destroyEntry)(struct HashTable *table, const struct HashEntryHeader *entry);
};

struct HashTable
{
  const struct HashTableOps *ops;   // virtual operations
  void *data;                       // ops- and instance-specific data
  UWORD shift;                      // multiplicative hash shift
  UBYTE maxAlphaFrac;               // 8-bit fixed point max alpha
  UBYTE minAlphaFrac;               // 8-bit fixed point min alpha
  ULONG entrySize;                  // number of bytes in an entry
  ULONG entryCount;                 // number of entries in table
  ULONG removedCount;               // removed entry sentinels in table
  ULONG generation;                 // entry storage generation number
  char *entryStore;                 // entry storage
};

enum HashTableOperator
{
  htoLookup = (1<<0),               // lookup entry
  htoAdd    = (1<<1),               // add entry
  htoRemove = (1<<2),               // remove entry, or enumerator says remove
  htoNext   = (1<<3),               // enumerator says continue
  htoStop   = (1<<4),               // enumerator says stop
};

#define HASH_BITS                   32
// Multiplicative hash uses an unsigned 32 bit integer and the golden ratio,
// expressed as a fixed-point 32-bit fraction.
#define HASH_GOLDEN_RATIO           0x9e3778b9UL
// Minimum table size, or gross entry count (net is at most .75 loaded).
#define HASH_MIN_SIZE               16
// Table size limit, do not equal or exceed (see min&maxAlphaFrac, below).
#define HASH_SIZE_LIMIT             (1UL << 24)
// Size in entries (gross, not net of free and removed sentinels) for table.
// We store hashShift rather than sizeLog2 to optimize the collision-free case
// in SearchTable.
#define HASH_TABLE_SIZE(table)      (1UL << (HASH_BITS - (table)->shift))

#define HASH_ENTRY_IS_LIVE(entry)   ((entry)->keyHash >= 2)
#define HASH_ENTRY_IS_FREE(entry)   ((entry)->keyHash == 0)
#define HASH_ENTRY_IS_BUSY(entry)   (!HASH_ENTRY_IS_FREE(entry))

/*** Public functions ***/

// Dynamically allocate a new HashTable using malloc, initialize it using
// HashTableInit, and return its address.  Return NULL on malloc failure.
// Note that the entry storage at table->entryStore will be allocated using
// the ops->allocTable callback.
struct HashTable *HashTableNew(const struct HashTableOps *ops, void *data, ULONG entrySize, ULONG capacity);

// Finalize table's data, free its entry storage (via table->ops->freeTable),
// and return the memory starting at table to the malloc heap.
void HashTableDestroy(struct HashTable *table);

// Initialize table with ops, data, entrySize, and capacity.  Capacity is a
// guess for the smallest table size at which the table will usually be less
// than 75% loaded (the table will grow or shrink as needed; capacity serves
// only to avoid inevitable early growth from HASH_MIN_SIZE).
BOOL HashTableInit(struct HashTable *table, const struct HashTableOps *ops, void *data, ULONG entrySize, ULONG capacity);

// Set maximum and minimum alpha for table.  The defaults are 0.75 and .25.
// maxAlpha must be in [0.5, 0.9375] for the default HASH_MIN_SIZE; or if
// MinSize=HASH_MIN_SIZE <= 256, in [0.5, (float)(MinSize-1)/MinSize]; or
// else in [0.5, 255.0/256].  minAlpha must be in [0, maxAlpha / 2), so that
// we don't shrink on the very next remove after growing a table upon adding
// an entry that brings entryCount past maxAlpha * tableSize.
void HashTableSetAlphaBounds(struct HashTable *table, float maxAlpha, float minAlpha);

// Clean up table's data, free its entry storage using table->ops->freeTable,
// and leave its members unchanged from their last live values (which leaves
// pointers dangling).  If you want to burn cycles clearing table, it's up to
// your code to call memset.
void HashTableCleanup(struct HashTable *table);

// Perform an operation on the table.
struct HashEntryHeader *HashTableOperate(struct HashTable *table, const void *key, enum HashTableOperator op);

// Remove an entry already accessed via LOOKUP or ADD.
// NB: this is a "raw" or low-level routine, intended to be used only where
// the inefficiency of a full HashTableOperate (which rehashes in order
// to find the entry given its key) is not tolerable.  This function does not
// shrink the table if it is underloaded.
void HashTableRawRemove(struct HashTable *table, struct HashEntryHeader *entry);

// get the default hash table operators for your own use
const struct HashTableOps *HashTableGetDefaultOps(void);
const struct HashTableOps *HashTableGetDefaultStringOps(void);

// Enumerate entries in table using etor:
//
//   count = HashTableEnumerate(table, etor, arg);
//
// HashTableEnumerate calls etor like so:
//
//   op = etor(table, entry, number, arg);
//
// where number is a zero-based ordinal assigned to live entries according to
// their order in table->entryStore.
//
// The return value, op, is treated as a set of flags. If op is htoNext, then
// continue enumerating.  If op contains htoRemove, then clear (via
// table->ops->clearEntry) and free entry.  Then we check whether op contains
// htoStop; if so, stop enumerating and return the number of live entries
// that were enumerated so far.  Return the total number of live entries when
// enumeration completes normally.
//
// If etor calls HashTableOperate on table with op != htoLookup, it must
// return htoStop; otherwise undefined behavior results.
//
// If any enumerator returns htoRemove, table->entryStore may be shrunk or
// compressed after enumeration, but before HashTableEnumerate returns. Such
// an enumerator therefore can't safely set aside entry pointers, but an
// enumerator that never returns htoRemove can set pointers to entries aside,
// e.g., to avoid copying live entries into an array of the entry type.
// Copying entry pointers is cheaper, and safe so long as the caller of such a
// "stable" Enumerate doesn't use the set-aside pointers after any call either
// to HashTableOperate, or to an "unstable" form of Enumerate, which might
// grow or shrink entryStore.
//
// If your enumerator wants to remove certain entries, but set aside pointers
// to other entries that it retains, it can use HashTableRawRemove on the
// entries to be removed, returning htoNext to skip them. Likewise, if you
// want to remove entries, but for some reason you do not want entryStore
// to be shrunk or compressed, you can call HashTableRawRemove safely on the
// entry being enumerated, rather than returning htoRemove.
ULONG HashTableEnumerate(struct HashTable *table, enum HashTableOperator (* etor)(struct HashTable *table, struct HashEntryHeader *entry, ULONG number, void *arg), void *arg);

/*** Public default operator functions ***/
// Table space at entryStore is allocated and freed using these callbacks.
// The allocator should return null on error only (not if called with nbytes
// equal to 0
void *DefaultHashAllocTable(UNUSED struct HashTable *table, ULONG capacity, ULONG entrySize);
void DefaultHashFreeTable(UNUSED struct HashTable *table, void *ptr);

// When a table grows or shrinks, each entry is queried for its key using this
// callback.  NB: in that event, entry is not in table any longer; it's in the
// old entryStore vector, which is due to be freed once all entries have been
// moved via moveEntry callbacks.
const void *DefaultHashGetKey(UNUSED struct HashTable *table, const struct HashEntryHeader *entry);

// Compute the hash code for a given key to be looked up, added, or removed
// from table.  A hash code may have any value.
ULONG DefaultHashHashKey(UNUSED struct HashTable *table, const void *key);

// Compare the key identifying entry in table with the provided key parameter.
// Return TRUE if keys match, FALSE otherwise.
BOOL DefaultHashMatchEntry(UNUSED struct HashTable *table, const struct HashEntryHeader *entry, const void *key);

// Copy the data starting at from to the new entry storage at to.  Do not add
// reference counts for any strong references in the entry, however, as this
// is a "move" operation: the old entry storage at from will be freed without
// any reference-decrementing callback shortly.
void DefaultHashMoveEntry(struct HashTable *table, const struct HashEntryHeader *from, struct HashEntryHeader *to);

// Clear the entry and drop any strong references it holds.  This callback is
// invoked during a remove operation (see above for operation codes), but only
// if the given key is found in the table.
void DefaultHashClearEntry(struct HashTable *table, struct HashEntryHeader *entry);

// Called when a table (whether allocated dynamically by itself, or nested in
// a larger structure, or allocated on the stack) is finished.  This callback
// allows table->ops-specific code to finalize table->data.
void DefaultHashFinalize(UNUSED struct HashTable *table);

/*** Public string operator functions ***/
ULONG StringHashHashKey(struct HashTable *table, const void *key);
BOOL StringHashMatchEntry(UNUSED struct HashTable *table, const struct HashEntryHeader *entry, const void *key);
void StringHashClearEntry(struct HashTable *table, struct HashEntryHeader *entry);

/*
// uncomment this if you want to try the demo code
void HashTableTest(void);
*/

#endif /* HASH_TABLE_H */

