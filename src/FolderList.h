#ifndef FOLDERLIST_H
#define FOLDERLIST_H 1

/***************************************************************************

 YAM - Yet Another Folderer
 Copyright (C) 1995-2000 Marcel Beck
 Copyright (C) 2000-2013 YAM Open Source Team

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

// forward declarations
struct SignalSemaphore;
struct Folder;

struct FolderList
{
  struct MinList list;
  struct SignalSemaphore *lockSemaphore;
  ULONG count;
};

struct FolderNode
{
  struct MinNode node;
  struct Folder *folder;
};

void InitFolderList(struct FolderList *flist);
struct FolderList *CreateFolderList(void);
void DeleteFolderList(struct FolderList *flist);
struct FolderNode *AddNewFolderNode(struct FolderList *flist, const struct Folder *folder);
void AddFolderNode(struct FolderList *flist, struct FolderNode *fnode);
void RemoveFolderNode(struct FolderList *flist, struct FolderNode *fnode);
void RemoveFolder(struct FolderList *flist, struct Folder *folder);
void DeleteFolderNode(struct FolderNode *fnode);
void MoveFolderNode(struct FolderList *flist, struct FolderNode *fnode, struct FolderNode *afterThis);
struct FolderNode *TakeFolderNode(struct FolderList *flist);
struct Folder *AllocFolder();
void FreeFolder(struct Folder *folder);
void MoveFolderContents(struct Folder *to, struct Folder *from);

// check if a folder list is empty
#define IsFolderListEmpty(flist)                  IsMinListEmpty(&(flist)->list)

// navigate in the list
#define FirstFolderNode(flist)                    (struct FolderNode *)GetHead((struct List *)&((flist)->list))
#define LastFolderNode(flist)                     (struct FolderNode *)GetTail((struct List *)&((flist)->list))
#define NextFolderNode(fnode)                     (struct FolderNode *)GetSucc((struct Node *)fnode)
#define PreviousFolderNode(fnode)                 (struct FolderNode *)GetPred((struct Node *)fnode)

// iterate through the list, the list must *NOT* be modified!
#define ForEachFolderNode(flist, fnode)           for(fnode = FirstFolderNode(flist); fnode != NULL; fnode = NextFolderNode(fnode))

// lock and unlock a folder list via its semaphore
#if defined(DEBUG)
void LockFolderList(const struct FolderList *flist);
void LockFolderListShared(const struct FolderList *flist);
void UnlockFolderList(const struct FolderList *flist);
#else
#define LockFolderList(flist)                     ObtainSemaphore((flist)->lockSemaphore)
#define LockFolderListShared(flist)               ObtainSemaphoreShared((flist)->lockSemaphore)
#define UnlockFolderList(flist)                   ReleaseSemaphore((flist)->lockSemaphore)
#endif

#endif /* FOLDERLIST_H */
