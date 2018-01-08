#ifndef MAILLIST_H
#define MAILLIST_H 1

/***************************************************************************

 YAM - Yet Another Mailer
 Copyright (C) 1995-2000 Marcel Beck
 Copyright (C) 2000-2018 YAM Open Source Team

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

struct MailList
{
  struct MinList list;                   // ptr to MinList to put it in exec lists
  struct SignalSemaphore *lockSemaphore; // semaphore for locking the list
  ULONG count;                           // number of entries in list
};

struct MailNode
{
  struct MinNode node;
  struct Mail *mail;
};

void InitMailList(struct MailList *mlist);
struct MailList *CreateMailList(void);
void ClearMailList(struct MailList *mlist);
void DeleteMailList(struct MailList *mlist);
struct MailList *CloneMailList(const struct MailList *mlist);
void MoveMailList(struct MailList *to, struct MailList *from);
struct MailNode *AddNewMailNode(struct MailList *mlist, struct Mail *mail);
void AddMailNode(struct MailList *mlist, struct MailNode *mnode);
void RemoveMailNode(struct MailList *mlist, struct MailNode *mnode);
void DeleteMailNode(struct MailNode *mnode);
void SortMailList(struct MailList *mlist, int (* compare)(const struct MailNode *m1, const struct MailNode *m2));
struct Mail **MailListToMailArray(const struct MailList *mlist);
struct MailNode *FindMailByAddress(const struct MailList *mlist, const struct Mail *mail);
struct MailNode *FindMailByFilename(const struct MailList *mlist, const char *filename);
struct MailNode *TakeMailNode(struct MailList *mlist);
struct Mail *AllocMail(void);
struct Mail *CloneMail(const struct Mail *mail);
void ReferenceMail(struct Mail *mail);
void DereferenceMail(struct Mail *mail);
void FreeMail(struct Mail *mail);

// public comparison functions
int CompareMailsByDate(const struct MailNode *m1, const struct MailNode *m2);

// check if a mail list is empty
#define IsMailListEmpty(mlist)                    IsMinListEmpty(&(mlist)->list)

// navigate in the list
#define FirstMailNode(mlist)                      (struct MailNode *)GetHead((struct List *)&((mlist)->list))
#define LastMailNode(mlist)                       (struct MailNode *)GetTail((struct List *)&((mlist)->list))
#define NextMailNode(mnode)                       (struct MailNode *)GetSucc((struct Node *)mnode)
#define PreviousMailNode(mnode)                   (struct MailNode *)GetPred((struct Node *)mnode)

// iterate through the list, the list must *NOT* be modified!
#define ForEachMailNode(mlist, mnode)             for(mnode = FirstMailNode(mlist); mnode != NULL; mnode = NextMailNode(mnode))

// lock and unlock a mail list via its semaphore
#if defined(DEBUG)
void LockMailList(const struct MailList *mlist);
void LockMailListShared(const struct MailList *mlist);
void UnlockMailList(const struct MailList *mlist);
#else
#define LockMailList(mlist)                       ObtainSemaphore((mlist)->lockSemaphore)
#define LockMailListShared(mlist)                 ObtainSemaphoreShared((mlist)->lockSemaphore)
#define UnlockMailList(mlist)                     ReleaseSemaphore((mlist)->lockSemaphore)
#endif

#endif /* MAILLIST_H */
