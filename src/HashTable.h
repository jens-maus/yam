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

struct HashTable {
    void *data;
    UWORD shift;
    UBYTE maxAlphaFrac;
    UBYTE minAlphaFrac;
    ULONG entrySize;
    ULONG entryCount;
    ULONG removedCount;
    ULONG generation;
    STRPTR entryStore;
};

typedef ULONG HashNumber;

struct HashEntryHeader {
    HashNumber keyHash;
};

struct HashEntry {
    struct HashEntryHeader header;
    void *key;
};

typedef enum {
    htoLookup = 0,
    htoAdd,
    htoRemove,
    htoNext,
    htoStop,
} HashOperator;

#define HASH_BITS                   32
#define HASH_GOLDEN_RATIO           0x9e3778b9U
#define HASH_MIN_SIZE               16
#define HASH_SIZE_LIMIT             (1L << 24)

#define HASH_TABLE_SIZE(table)      (1L << (HASH_BITS - (table)->shift))

#define HASH_ENTRY_IS_LIVE(entry)   ((entry)->keyHash >= 2)
#define HASH_ENTRY_IS_FREE(entry)   ((entry)->keyHash == 0)
#define HASH_ENTRY_IS_BUSY(entry)   (!HASH_ENTRY_IS_FREE(entry))

HashNumber HashStringKey(struct HashTable *table, CONST void *key);
BOOL HashMatchStringKey(UNUSED struct HashTable *table, struct HashEntryHeader *entry, CONST void *key);
void HashFreeStringKey(struct HashTable *table, struct HashEntryHeader *heh);
BOOL HashTableInit(struct HashTable *table, void *data, ULONG entrySize, ULONG capacity);
void HashTableSetAlphaBounds(struct HashTable *table, float maxAlpha, float minAlpha);
void HashTableCleanup(struct HashTable *table);
struct HashEntryHeader *HashTableOperate(struct HashTable *table, CONST void *key, HashOperator op);
void HashTableRawRemove(struct HashTable *table, struct HashEntryHeader *entry);

#endif /* HASH_TABLE_H */

