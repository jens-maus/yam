#ifndef TransferList_H
#define TransferList_H 1

/***************************************************************************

 YAM - Yet Another Mailer
 Copyright (C) 1995-2000 by Marcel Beck <mbeck@yam.ch>
 Copyright (C) 2000-2010 by YAM Open Source Team

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

#include <exec/lists.h>
#include <exec/nodes.h>
#include <exec/types.h>
#include <proto/exec.h>

#include "YAM_utilities.h"

// forward declarations
struct SignalSemaphore;
struct Mail;

struct TransferList
{
  struct MinList list;
  struct SignalSemaphore *lockSemaphore;
  ULONG count;
};

struct TransferNode
{
  struct MinNode node;      // required for placing it into "struct TR_ClassData"
  struct Mail *mail;        // pointer to the corresponding mail
  char *uidl;               // an unique identifier (UIDL) in case AvoidDuplicates is used
  ULONG tflags;             // transfer flags
  int position;             // current position of the mail in the GUI NList
  int index;                // the index value of the mail as told by a POP3 server
  LONG importAddr;          // the position (addr) within an export file to find the mail
};

// flag values for the transfer node's flag field
#define TRF_NONE              (0)
#define TRF_TRANSFER          (1<<0) // transfer this node
#define TRF_DELETE            (1<<1) // delete this node
#define TRF_PRESELECT         (1<<2) // include this node in a preselection

void InitTransferList(struct TransferList *tlist);
void ClearTransferList(struct TransferList *tlist);
struct TransferList *CreateTransferList(void);
void DeleteTransferList(struct TransferList *tlist);
BOOL ScanTransferList(const struct TransferList *tlist, const ULONG flags);
struct TransferNode *CreateTransferNode(const struct Mail *mail, const ULONG flags);
void AddTransferNode(struct TransferList *tlist, struct TransferNode *tnode);
void RemoveTransferNode(struct TransferList *tlist, struct TransferNode *tnode);
void DeleteTransferNode(struct TransferNode *tnode);


// check if a mail list is empty
#define IsTransferListEmpty(tlist)                    IsMinListEmpty(&(tlist)->list)

// iterate through the list, the list must *NOT* be modified!
#define ForEachTransferNode(tlist, tnode)             for(tnode = FirstTransferNode(tlist); tnode != NULL; tnode = NextTransferNode(tnode))

// navigate in the list
#define FirstTransferNode(tlist)                      (struct TransferNode *)GetHead((struct List *)&((tlist)->list))
#define LastTransferNode(tlist)                       (struct TransferNode *)GetTail((struct List *)&((tlist)->list))
#define NextTransferNode(tnode)                       (struct TransferNode *)GetSucc((struct Node *)tnode)
#define PreviousTransferNode(tnode)                   (struct TransferNode *)GetPred((struct Node *)tnode)

// lock and unlock a Transfer list via its semaphore
#if defined(DEBUG)
void LockTransferList(const struct TransferList *tlist);
void LockTransferListShared(const struct TransferList *tlist);
void UnlockTransferList(const struct TransferList *tlist);
#else
#define LockTransferList(tlist)                       ObtainSemaphore((tlist)->lockSemaphore)
#define LockTransferListShared(tlist)                 ObtainSemaphoreShared((tlist)->lockSemaphore)
#define UnlockTransferList(tlist)                     ReleaseSemaphore((tlist)->lockSemaphore)
#endif

#endif /* TransferList_H */
