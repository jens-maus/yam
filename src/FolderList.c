/***************************************************************************

 YAM - Yet Another Folderer
 Copyright (C) 1995-2000 by Marcel Beck <mbeck@yam.ch>
 Copyright (C) 2000-2008 by YAM Open Source Team

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
#include <clib/alib_protos.h>
#include <proto/exec.h>

#include "extrasrc.h"

#include "FolderList.h"

#include "Debug.h"

/// InitFolderList
// initialize a folder list
void InitFolderList(struct FolderList *flist)
{
  ENTER();

  NewList((struct List *)&flist->list);
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
  if((flist = AllocSysObjectTags(ASOT_LIST, ASOLIST_Size, sizeof(*flist),
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

      // lock the list just, just for safety reasons
      LockFolderList(flist);

      // remove and free all remaining nodes in the list
      while((fnode = (struct FolderNode *)RemHead((struct List *)&flist->list)) != NULL)
        FreeSysObject(ASOT_NODE, fnode);

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
struct FolderNode *AddNewFolderNode(struct FolderList *flist, struct Folder *folder)
{
  struct FolderNode *fnode = NULL;

  ENTER();

  // we only accept existing folders
  if(folder != NULL)
  {
    if((fnode = AllocSysObjectTags(ASOT_NODE, ASONODE_Size, sizeof(*fnode),
                                              ASONODE_Min, TRUE,
                                              TAG_DONE)) != NULL)
    {
      // initialize the node's contents
      fnode->folder = folder;

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
/// DeleteFolderNode
// free a folder node that does not belong to a list
void DeleteFolderNode(struct FolderNode *fnode)
{
  ENTER();

  FreeSysObject(ASOT_NODE, fnode);

  LEAVE();
}

///

#if defined(DEBUG)
static LONG folderLocks = 0;

/// LockFolderList()
void LockFolderList(struct FolderList *flist)
{
  ENTER();

  if(++folderLocks != 1)
    E(DBF_ALWAYS, "nested (%ld) exclusive lock of folderlist %08lx", folderLocks, flist);
  else
    ObtainSemaphore(flist->lockSemaphore);

  LEAVE();
}

///
/// LockFolderListShared()
void LockFolderListShared(struct FolderList *flist)
{
  ENTER();

  if(++folderLocks != 1)
    E(DBF_ALWAYS, "nested (%ld) shared lock of folderlist %08lx", folderLocks, flist);
  else
    ObtainSemaphoreShared(flist->lockSemaphore);

  LEAVE();
}

///
/// UnlockFolderList()
void UnlockFolderList(struct FolderList *flist)
{
  ENTER();

  folderLocks--;
  if(folderLocks < 0)
    E(DBF_ALWAYS, "too many unlocks (%ld) of folderlist %08lx", folderLocks, flist);
  else if(folderLocks == 0)
    ReleaseSemaphore(flist->lockSemaphore);

  LEAVE();
}

///
#endif

