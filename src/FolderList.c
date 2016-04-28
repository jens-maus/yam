/***************************************************************************

 YAM - Yet Another Folderer
 Copyright (C) 1995-2000 Marcel Beck
 Copyright (C) 2000-2016 YAM Open Source Team

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

#include "FolderList.h"
#include "MailList.h"

#include "Debug.h"

/// InitFolderList
// initialize a folder list
void InitFolderList(struct FolderList *flist)
{
  ENTER();

  NewMinList(&flist->list);
  flist->count = 0;

  LEAVE();
}

///
/// CreateFolderList
// create a new list for folders
struct FolderList *CreateFolderList(void)
{
  struct FolderList *flist;

  ENTER();

  // at first create the list itself
  if((flist = AllocSysObjectTags(ASOT_LIST,
    ASOLIST_Size, sizeof(*flist),
    ASOLIST_Min, TRUE,
    TAG_DONE)) != NULL)
  {
    // now create the arbitration semaphore
    if((flist->lockSemaphore = AllocSysObjectTags(ASOT_SEMAPHORE, TAG_DONE)) != NULL)
    {
      // no folders in the list so far
      flist->count = 0;
    }
    else
    {
      // free the list again on failure
      FreeSysObject(ASOT_LIST, flist);
      flist = NULL;
    }
  }

  RETURN(flist);
  return flist;
}

///
/// DeleteFolderList
// remove all nodes from a list and free it completely
void DeleteFolderList(struct FolderList *flist)
{
  ENTER();

  if(flist != NULL)
  {
    if(flist->lockSemaphore != NULL)
    {
      struct FolderNode *fnode;
      struct FolderNode *next;

      // lock the list just, just for safety reasons
      LockFolderList(flist);

      // remove and free all remaining nodes in the list
      SafeIterateList(&flist->list, struct FolderNode *, fnode, next)
      {
        FreeSysObject(ASOT_NODE, fnode);
      }
      NewMinList(&flist->list);

      // unlock the list again
      UnlockFolderList(flist);

      // free the semaphore
      FreeSysObject(ASOT_SEMAPHORE, flist->lockSemaphore);
    }

    // free the list itself
    FreeSysObject(ASOT_LIST, flist);
  }

  LEAVE();
}

///
/// AddNewFolderNode
// add a new folder to an existing list
// if locking of the list is needed this must be done by the calling function
struct FolderNode *AddNewFolderNode(struct FolderList *flist, const struct Folder *folder)
{
  struct FolderNode *fnode = NULL;

  ENTER();

  // we only accept existing folders
  if(folder != NULL)
  {
    if((fnode = AllocSysObjectTags(ASOT_NODE,
      ASONODE_Size, sizeof(*fnode),
      ASONODE_Min, TRUE,
      TAG_DONE)) != NULL)
    {
      // initialize the node's contents
      fnode->folder = (struct Folder *)folder;

      // add the new folder node to the end of the list
      AddTail((struct List *)&flist->list, (struct Node *)&fnode->node);

      // and increase the counter
      flist->count++;
    }
  }

  // return the new folder node in case someone is interested in it
  RETURN(fnode);
  return fnode;
}

///
/// AddFolderNode
// add a folder node to an existing list
// if locking of the list is needed this must be done by the calling function
void AddFolderNode(struct FolderList *flist, struct FolderNode *fnode)
{
  ENTER();

  // we only accept existing folders
  if(flist != NULL && fnode != NULL)
  {
    // add the new folder node to the end of the list
    AddTail((struct List *)&flist->list, (struct Node *)&fnode->node);

    // and increase the counter
    flist->count++;
  }

  LEAVE();
}

///
/// RemoveFolderNode
// remove a folder node from the list
void RemoveFolderNode(struct FolderList *flist, struct FolderNode *fnode)
{
  ENTER();

  // remove the folder node from the list
  Remove((struct Node *)&fnode->node);

  // and decrease the counter
  flist->count--;

  LEAVE();
}

///
/// RemoveFolder
// find a folder in the list and remove and delete the node
void RemoveFolder(struct FolderList *flist, struct Folder *folder)
{
  struct FolderNode *fnode;

  ENTER();

  ForEachFolderNode(flist, fnode)
  {
    if(fnode->folder == folder)
    {
      // we found the folder, remove and delete the node
      RemoveFolderNode(flist, fnode);
      DeleteFolderNode(fnode);
      break;
    }
  }

  LEAVE();
}

///
/// DeleteFolderNode
// free a folder node that does not belong to a list
void DeleteFolderNode(struct FolderNode *fnode)
{
  ENTER();

  FreeSysObject(ASOT_NODE, fnode);

  LEAVE();
}

///
/// MoveFolderNode
// move a folder within the list to a different position
// the folder list must be locked *exclusively* before calling this function!!
void MoveFolderNode(struct FolderList *flist, struct FolderNode *fnode, struct FolderNode *afterThis)
{
  ENTER();

  // first remove the node from the list
  Remove((struct Node *)&fnode->node);

  // then insert it again either at the front or somewhere in the middle
  if(afterThis == NULL)
    AddHead((struct List *)&flist->list, (struct Node *)&fnode->node);
  else
    Insert((struct List *)&flist->list, (struct Node *)&fnode->node, (struct Node *)&afterThis->node);

  LEAVE();
}

///
/// TakeFolderNode
struct FolderNode *TakeFolderNode(struct FolderList *flist)
{
  struct FolderNode *fnode = NULL;

  ENTER();

  if(flist != NULL)
  {
    // try to remove the first node from the list
    if((fnode = (struct FolderNode *)RemHead((struct List *)&flist->list)) != NULL)
    {
      // decrease the counter
      flist->count--;
    }
  }

  RETURN(fnode);
  return fnode;
}

///
/// AllocFolder
// allocate a folder structure
struct Folder *AllocFolder(void)
{
  struct Folder *folder;

  ENTER();

  if((folder = calloc(1, sizeof(*folder))) != NULL)
  {
    if((folder->messages = CreateMailList()) != NULL)
    {
      // everything is ok
    }
    else
    {
      free(folder);
      folder = NULL;
    }
  }

  RETURN(folder);
  return folder;
}

///
/// InitFolder
// set certain default values for a folder
void InitFolder(struct Folder *folder, enum FolderType type)
{
  struct MailList *messages = folder->messages;

  ENTER();

  memset(folder, 0, sizeof(*folder));
  folder->Type = type;
  if(type != FT_GROUP)
  {
    folder->Sort[0] = 1;
    folder->Sort[1] = 3;
    folder->LastActive = -1;
    folder->JumpToUnread = TRUE;
    folder->JumpToRecent = FALSE;
    // preserve the message list
    folder->messages = messages;
  }

  LEAVE();
}

///
/// FreeFolder
// free a folder structure
void FreeFolder(struct Folder *folder)
{
  ENTER();

  DeleteMailList(folder->messages);
  free(folder);

  LEAVE();
}

///
/// MoveFolderContents
// move all mails from one folder structure to another together with all stats
void MoveFolderContents(struct Folder *to, struct Folder *from)
{
  ENTER();

  // move over all messages
  MoveMailList(to->messages, from->messages);

  // adjust the stats
  to->Size += from->Size;
  to->Total += from->Total;
  to->New += from->New;
  to->Unread += from->Unread;
  to->Sent += from->Sent;

  // erase the stats of the originating folder
  from->Size = 0;
  from->Total = 0;
  from->New = 0;
  from->Unread = 0;
  from->Sent = 0;

  LEAVE();
}

///
/// IsUniqueFolderID
// check for a unique folder ID
BOOL IsUniqueFolderID(const struct FolderList *flist, const int id)
{
  BOOL isUnique = TRUE;
  struct FolderNode *fnode;

  ENTER();

  LockFolderList(flist);
  IterateList(flist, struct FolderNode *, fnode)
  {
    if(fnode->folder->ID == id)
    {
      // we found exactly this ID, this is bad
      isUnique = FALSE;
      break;
    }
  }
  UnlockFolderList(flist);

  RETURN(isUnique);
  return isUnique;
}

///
/// FindFolderByID
// find a folder by its unique ID
struct Folder *FindFolderByID(const struct FolderList *flist, const int id)
{
  struct Folder *result = NULL;

  ENTER();

  if(id != 0)
  {
    struct FolderNode *fn;

    LockFolderListShared(flist);
    IterateList(flist, struct FolderNode *, fn)
    {
      // check if we found exactly this ID
      if(id == fn->folder->ID)
      {
        result = fn->folder;
        break;
      }
    }
    UnlockFolderList(flist);
  }

  RETURN(result);
  return result;
}

///
/// FindFolderNode
// find a folder node
struct FolderNode *FindFolderNode(const struct FolderList *flist, const struct Folder *folder)
{
  struct FolderNode *result = NULL;

  ENTER();

  if(folder != NULL)
  {
    struct FolderNode *fnode;

    LockFolderList(flist);
    IterateList(flist, struct FolderNode *, fnode)
    {
      // check if we found exactly this ID
      if(folder == fnode->folder)
      {
        result = fnode;
        break;
      }
    }
    UnlockFolderList(flist);
  }

  RETURN(result);
  return result;
}

///

#if defined(DEBUG)
static LONG folderLocks = 0;

/// LockFolderList
void LockFolderList(const struct FolderList *flist)
{
  ENTER();

  if(AttemptSemaphore(flist->lockSemaphore) == FALSE)
  {
    if(folderLocks > 0)
      E(DBF_ALWAYS, "nested (%ld) exclusive lock of folderlist %08lx", folderLocks + 1, flist);
    ObtainSemaphore(flist->lockSemaphore);
  }

  folderLocks++;

  LEAVE();
}

///
/// LockFolderListShared
void LockFolderListShared(const struct FolderList *flist)
{
  ENTER();

  if(AttemptSemaphoreShared(flist->lockSemaphore) == FALSE)
  {
    if(folderLocks > 0)
      E(DBF_ALWAYS, "nested (%ld) shared lock of folderlist %08lx", folderLocks + 1, flist);
    ObtainSemaphoreShared(flist->lockSemaphore);
  }

  folderLocks++;

  LEAVE();
}

///
/// UnlockFolderList
void UnlockFolderList(const struct FolderList *flist)
{
  ENTER();

  folderLocks--;
  if(folderLocks < 0)
    E(DBF_ALWAYS, "too many unlocks (%ld) of folderlist %08lx", folderLocks, flist);

  ReleaseSemaphore(flist->lockSemaphore);

  LEAVE();
}

///
#endif

