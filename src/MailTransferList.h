#ifndef MAILTRANSFERLIST_H
#define MAILTRANSFERLIST_H 1

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

struct MailTransferList
{
  struct MinList list;
  struct SignalSemaphore *lockSemaphore;
  ULONG count;
};

struct MailTransferNode
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

void InitMailTransferList(struct MailTransferList *tlist);
void ClearMailTransferList(struct MailTransferList *tlist);
struct MailTransferList *CreateMailTransferList(void);
void DeleteMailTransferList(struct MailTransferList *tlist);
BOOL ScanMailTransferList(const struct MailTransferList *tlist, const ULONG flags, LONG *index);
struct MailTransferNode *CreateMailTransferNode(const struct Mail *mail, const ULONG flags);
void AddMailTransferNode(struct MailTransferList *tlist, struct MailTransferNode *tnode);
void RemoveMailTransferNode(struct MailTransferList *tlist, struct MailTransferNode *tnode);
void DeleteMailTransferNode(struct MailTransferNode *tnode);


// check if a mail list is empty
#define IsMailTransferListEmpty(tlist)                    IsMinListEmpty(&(tlist)->list)

// iterate through the list, the list must *NOT* be modified!
#define ForEachMailTransferNode(tlist, tnode)             for(tnode = FirstMailTransferNode(tlist); tnode != NULL; tnode = NextMailTransferNode(tnode))

// navigate in the list
#define FirstMailTransferNode(tlist)                      (struct MailTransferNode *)GetHead((struct List *)&((tlist)->list))
#define LastMailTransferNode(tlist)                       (struct MailTransferNode *)GetTail((struct List *)&((tlist)->list))
#define NextMailTransferNode(tnode)                       (struct MailTransferNode *)GetSucc((struct Node *)tnode)
#define PreviousMailTransferNode(tnode)                   (struct MailTransferNode *)GetPred((struct Node *)tnode)

// lock and unlock a Transfer list via its semaphore
#if defined(DEBUG)
void LockMailTransferList(const struct MailTransferList *tlist);
void LockMailTransferListShared(const struct MailTransferList *tlist);
void UnlockMailTransferList(const struct MailTransferList *tlist);
#else
#define LockMailTransferList(tlist)                       ObtainSemaphore((tlist)->lockSemaphore)
#define LockMailTransferListShared(tlist)                 ObtainSemaphoreShared((tlist)->lockSemaphore)
#define UnlockMailTransferList(tlist)                     ReleaseSemaphore((tlist)->lockSemaphore)
#endif

#endif /* MAILTRANSFERLIST */
