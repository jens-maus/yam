/***************************************************************************

 YAM - Yet Another Mailer
 Copyright (C) 1995-2000 Marcel Beck
 Copyright (C) 2000-2011 YAM Open Source Team

 This program is free software; you can redistribute it and/or modify
 it under the terms of the GNU General Public License as published by
 the Free Software Foundation; either version 2 of the License, or
 (at your option) any later version.

 This program is distributed in the hope that it will be useful,
 but WITHOUT ANY WARRANTY; without even the implied warranty of
 MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.   See the
 GNU General Public License for more details.

 You should have received a copy of the GNU General Public License
 along with this program; if not, write to the Free Software
 Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA

 YAM Official Support Site : http://www.yam.ch
 YAM OpenSource project    : http://sourceforge.net/projects/yamos/

 $Id$

 Superclass:  MUIC_FolderListtree
 Description: a listtree showing all available folders

***************************************************************************/

#include "FolderRequestListtree_cl.h"

#include <mui/NList_mcc.h>
#include <mui/NListtree_mcc.h>

#include "YAM.h"
#include "YAM_mainFolder.h"

#include "FolderList.h"

#include "Debug.h"

/* Overloaded Methods */
/// OVERLOAD(OM_NEW)
OVERLOAD(OM_NEW)
{
  ENTER();

  if((obj = DoSuperNew(cl, obj,

    InputListFrame,
    MUIA_NList_AutoVisible, TRUE,

    TAG_MORE, inittags(msg))) != NULL)
  {
    struct Folder *prevFolder;
    struct Folder *excludeFolder;
    struct FolderNode *fnode;
    ULONG pos;

    prevFolder = (struct Folder *)GetTagData(ATTR(Folder), (IPTR)NULL, inittags(msg));
    excludeFolder = (struct Folder *)GetTagData(ATTR(Exclude), (IPTR)NULL, inittags(msg));

    LockFolderListShared(G->folders);

    pos = 0;
    ForEachFolderNode(G->folders, fnode)
    {
      struct Folder *folder = fnode->folder;
      ULONG tnflags = MUIF_NONE;
      struct MUI_NListtree_TreeNode *tn_parent;

      if(folder != excludeFolder)
      {
        if(isGroupFolder(folder))
          tnflags = TNF_LIST|TNF_OPEN;

        // we first have to get the parent folder treenode
        if(folder->parent != NULL &&
           (tn_parent = (struct MUI_NListtree_TreeNode *)DoMethod(obj, MUIM_NListtree_FindUserData, MUIV_NListtree_FindUserData_ListNode_Root, folder->parent, MUIF_NONE)) != NULL)
        {
          DoMethod(obj, MUIM_NListtree_Insert, fnode->folder->Name, fnode, tn_parent, MUIV_NListtree_Insert_PrevNode_Tail, tnflags);
        }
        else
        {
          DoMethod(obj, MUIM_NListtree_Insert, fnode->folder->Name, fnode, MUIV_NListtree_Insert_ListNode_Root, MUIV_NListtree_Insert_PrevNode_Tail, tnflags);
        }

        // mark the previously selected folder
        if(fnode->folder == prevFolder)
          set(obj, MUIA_NList_Active, pos);

        // count the added folders
        pos++;
      }
    }

    UnlockFolderList(G->folders);
  }

  RETURN((IPTR)obj);
  return (IPTR)obj;
}

///
