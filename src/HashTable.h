#ifndef HASH_TABLE_H
#define HASH_TABLE_H 1

/***************************************************************************

 YAM - Yet Another Mailer
 Copyright (C) 1995-2000 by Marcel Beck <mbeck@yam.ch>
 Copyright (C) 2000-2006 by YAM Open Source Team

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

#include <exec/types.h>

#include "SDI_compiler.h"

// forward declaration of struct HashTable, because we need it all along the way
// before we really can define it.
struct HashTable;

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
  STRPTR entryStore;                // entry storage
};

enum HashTableOperator
{
  htoLookup = 0,
  htoAdd,
  htoRemove,
  htoNext,
  htoStop,
};

#define HASH_BITS                   32
// Multiplicative hash uses an unsigned 32 bit integer and the golden ratio,
// expressed as a fixed-point 32-bit fraction.
#define HASH_GOLDEN_RATIO           0x9e3778b9U
// Minimum table size, or gross entry count (net is at most .75 loaded).
#define HASH_MIN_SIZE               16
// Table size limit, do not equal or exceed (see min&maxAlphaFrac, below).
#define HASH_SIZE_LIMIT             (1L << 24)
// Size in entries (gross, not net of free and removed sentinels) for table.
// We store hashShift rather than sizeLog2 to optimize the collision-free case
// in SearchTable.
#define HASH_TABLE_SIZE(table)      (1L << (HASH_BITS - (table)->shift))

#define HASH_ENTRY_IS_LIVE(entry)   ((entry)->keyHash >= 2)
#define HASH_ENTRY_IS_FREE(entry)   ((entry)->keyHash == 0)
#define HASH_ENTRY_IS_BUSY(entry)   (!HASH_ENTRY_IS_FREE(entry))

/*** Public functions ***/

// Dynamically allocate a new HashTable using malloc, initialize it using
// HashTableInit, and return its address.  Return NULL on malloc failure.
// Note that the entry storage at table->entryStore will be allocated using
// the ops->allocTable callback.
struct HashTable *HashTableNew(struct HashTableOps *ops, void *data, ULONG entrySize, ULONG capacity);

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
const struct HashTableOps *HashTableGetDefaultOps(void);

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

#endif /* HASH_TABLE_H */

