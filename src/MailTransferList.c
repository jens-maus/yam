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

#include <stdlib.h>
#include <string.h>
#include <clib/alib_protos.h>
#include <proto/exec.h>

#include "extrasrc.h"

#include "YAM_mainFolder.h"

#include "MailList.h"
#include "MailTransferList.h"

#include "Debug.h"

/// InitMailTransferList
// initialize a transfer list
// NOTE: the embedded semaphore must NOT be used for such a list
void InitMailTransferList(struct MailTransferList *tlist)
{
  ENTER();

  NewMinList(&tlist->list);
  tlist->count = 0;

  LEAVE();
}

///
/// ClearMailTransferList
// remove all nodes from a transfer list
void ClearMailTransferList(struct MailTransferList *tlist)
{
  struct MailTransferNode *tnode;
  struct MailTransferNode *succ;

  ENTER();

  SafeIterateList(&tlist->list, struct MailTransferNode *, tnode, succ)
  {
    DeleteMailTransferNode(tnode);
  }
  InitMailTransferList(tlist);

  LEAVE();
}

///
/// CreateMailTransferList
// create a new list for transfers
struct MailTransferList *CreateMailTransferList(void)
{
  struct MailTransferList *tlist;

  ENTER();

  // at first create the list itself
  if((tlist = AllocSysObjectTags(ASOT_LIST,
    ASOLIST_Size, sizeof(*tlist),
    ASOLIST_Min,  TRUE,
    TAG_DONE)) != NULL)
  {
    // now create the arbitration semaphore
    if((tlist->lockSemaphore = AllocSysObjectTags(ASOT_SEMAPHORE, TAG_DONE)) != NULL)
    {
      // no transfers in the list so far
      tlist->count = 0;
    }
    else
    {
      // free the list again on failure
      FreeSysObject(ASOT_LIST, tlist);
      tlist = NULL;
    }
  }

  RETURN(tlist);
  return tlist;
}

///
/// DeleteMailTransferList
// remove all nodes from a list and free it completely
void DeleteMailTransferList(struct MailTransferList *tlist)
{
  ENTER();

  if(tlist != NULL)
  {
    struct MailTransferNode *tnode;
    struct MailTransferNode *next;

    // lock the list just, just for safety reasons
    LockMailTransferList(tlist);

    // remove and free all remaining nodes in the list
    SafeIterateList(&tlist->list, struct MailTransferNode *, tnode, next)
    {
      DeleteMailTransferNode(tnode);
    }
    NewMinList(&tlist->list);

    // unlock the list again
    UnlockMailTransferList(tlist);

    // free the semaphore
    FreeSysObject(ASOT_SEMAPHORE, tlist->lockSemaphore);
    tlist->lockSemaphore = NULL;

    // free the list itself
    FreeSysObject(ASOT_LIST, tlist);
  }

  LEAVE();
}

///
/// ScanMailTransferList
// iterate over a transfer list and return TRUE if at least one node has
// either one or all of the given flags set.
// Optionally the node's index can be returned
struct MailTransferNode *ScanMailTransferList(const struct MailTransferList *tlist, const ULONG maskFlags, const ULONG wantedFlags, const BOOL allFlags)
{
  struct MailTransferNode *result = NULL;
  struct MailTransferNode *tnode;

  ENTER();

  LockMailTransferList(tlist);

  ForEachMailTransferNode(tlist, tnode)
  {
    // check if either at least one flag matches or if all flags match
    if((allFlags == FALSE && isAnyFlagSet(tnode->tflags, maskFlags)) ||
       (allFlags == TRUE  && (tnode->tflags & maskFlags) == wantedFlags))
    {
      result = tnode;
      break;
    }
  }

  UnlockMailTransferList(tlist);

  RETURN(result);
  return result;
}

///
/// CreateMailTransferNode
// create a new transfer node, a given mail pointer will be memdup()'ed
struct MailTransferNode *CreateMailTransferNode(struct Mail *mail, const ULONG flags)
{
  struct MailTransferNode *tnode;

  ENTER();

  if((tnode = AllocSysObjectTags(ASOT_NODE,
    ASONODE_Size, sizeof(*tnode),
    ASONODE_Min, TRUE,
    TAG_DONE)) != NULL)
  {
    // clear the structure, ASOT() does not do that for us
    memset(tnode, 0, sizeof(*tnode));

    tnode->tflags = flags;
    tnode->mail = mail;

    ReferenceMail(mail);
  }

  RETURN(tnode);
  return tnode;
}

///
/// AddMailTransferNode
// add a transfer node to an existing list
// if locking of the list is needed this must be done by the calling function
void AddMailTransferNode(struct MailTransferList *tlist, struct MailTransferNode *tnode)
{
  ENTER();

  // we only accept existing transfers
  if(tlist != NULL && tnode != NULL && tnode->mail != NULL)
  {
    // add the new transfer node to the end of the list
    AddTail((struct List *)&tlist->list, (struct Node *)&tnode->node);

    // and increase the counter
    tlist->count++;
  }

  LEAVE();
}

///
/// DeleteMailTransferNode
// free a transfer node that does not belong to a list
void DeleteMailTransferNode(struct MailTransferNode *tnode)
{
  ENTER();

  DereferenceMail(tnode->mail);

  free(tnode->uidl);
  FreeSysObject(ASOT_NODE, tnode);

  LEAVE();
}

///
/// CountMailTransferNodes
// count the number of nodes with specific flags set
ULONG CountMailTransferNodes(const struct MailTransferList *tlist, const ULONG flags)
{
  ULONG count = 0;
  struct MailTransferNode *tnode;

  ENTER();

  LockMailTransferList(tlist);

  ForEachMailTransferNode(tlist, tnode)
  {
    if(isAnyFlagSet(tnode->tflags, flags))
      count++;
  }

  UnlockMailTransferList(tlist);

  RETURN(count);
  return count;
}

///

#if defined(DEBUG)
static LONG transferLocks = 0;

/// LockMailTransferList()
void LockMailTransferList(const struct MailTransferList *tlist)
{
  ENTER();

  if(AttemptSemaphore(tlist->lockSemaphore) == FALSE)
  {
    if(transferLocks > 0)
      E(DBF_ALWAYS, "nested (%ld) exclusive lock of MailTransferList %08lx", transferLocks + 1, tlist);
    ObtainSemaphore(tlist->lockSemaphore);
  }

  transferLocks++;

  LEAVE();
}

///
/// LockMailTransferListShared()
void LockMailTransferListShared(const struct MailTransferList *tlist)
{
  ENTER();

  if(AttemptSemaphoreShared(tlist->lockSemaphore) == FALSE)
  {
    if(transferLocks > 0)
      E(DBF_ALWAYS, "nested (%ld) shared lock of MailTransferList %08lx", transferLocks + 1, tlist);
    ObtainSemaphoreShared(tlist->lockSemaphore);
  }

  transferLocks++;

  LEAVE();
}

///
/// UnlockMailTransferList()
void UnlockMailTransferList(const struct MailTransferList *tlist)
{
  ENTER();

  transferLocks--;
  if(transferLocks < 0)
    E(DBF_ALWAYS, "too many unlocks (%ld) of MailTransferList %08lx", transferLocks, tlist);

  ReleaseSemaphore(tlist->lockSemaphore);

  LEAVE();
}

///
#endif
