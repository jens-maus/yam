/***************************************************************************

 YAM - Yet Another Mailer
 Copyright (C) 1995-2000 Marcel Beck
 Copyright (C) 2000-2013 YAM Open Source Team

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

#include <proto/muimaster.h>

#include <mui/NList_mcc.h>
#include <mui/NListtree_mcc.h>

#include "YAM.h"
#include "YAM_mainFolder.h"

#include "FolderList.h"
#include "MUIObjects.h"

#include "mui/ImageArea.h"

#include "Debug.h"

/* CLASSDATA
struct Data
{
  Object *userImage[SIZE_DEFAULT];
  int userImageIndex[SIZE_DEFAULT];
};
*/

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

    prevFolder = (struct Folder *)GetTagData(ATTR(Folder), (IPTR)NULL, inittags(msg));
    excludeFolder = (struct Folder *)GetTagData(ATTR(Exclude), (IPTR)NULL, inittags(msg));

    LockFolderListShared(G->folders);

    ForEachFolderNode(G->folders, fnode)
    {
      struct Folder *folder = fnode->folder;
      ULONG tnflags = MUIF_NONE;
      struct MUI_NListtree_TreeNode *tn_parent;

      if(folder != excludeFolder)
      {
        if(isGroupFolder(folder))
          tnflags = TNF_LIST|TNF_OPEN;

        // set the previously selected folder as active
        if(folder == prevFolder)
          setFlag(tnflags, MUIV_NListtree_Insert_Flag_Active);

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
      }
    }

    UnlockFolderList(G->folders);
  }

  RETURN((IPTR)obj);
  return (IPTR)obj;
}

///
/// OVERLOAD(OM_DISPOSE)
OVERLOAD(OM_DISPOSE)
{
  IPTR result;
  GETDATA;
  ULONG i;

  ENTER();

  for(i=0; i < ARRAY_SIZE(data->userImage); i++)
  {
    DoMethod(obj, MUIM_NList_UseImage, NULL, data->userImageIndex[i], MUIF_NONE);
    if(data->userImage[i] != NULL)
    {
      MUI_DisposeObject(data->userImage[i]);
      data->userImage[i] = NULL;
      data->userImageIndex[i] = 0;
    }
  }

  // dispose ourself
  result = DoSuperMethodA(cl, obj, msg);

  RETURN(result);
  return result;
}

///
/// OVERLOAD(MUIM_NListtree_Insert)
OVERLOAD(MUIM_NListtree_Insert)
{
  GETDATA;
  struct MUI_NListtree_TreeNode *tn;

  ENTER();

  // first let the list tree class do the actual insertion of the tree nodes
  if((tn = (struct MUI_NListtree_TreeNode *)DoSuperMethodA(cl, obj, msg)) != NULL &&
     tn->tn_User != NULL)
  {
    struct Folder *folder = ((struct FolderNode *)tn->tn_User)->folder;

    // now we check wheter we should create an image object or not
    if(folder->imageObject != NULL && folder->ImageIndex >= FICON_ID_MAX)
    {
      char *id = (char *)xget(folder->imageObject, MUIA_ImageArea_ID);
      char *filename = (char *)xget(folder->imageObject, MUIA_ImageArea_Filename);

      if(id != NULL && filename != NULL)
      {
        int i = 0;

        // find a free slot in userImage[]
        while(i < SIZE_DEFAULT && data->userImage[i] != NULL)
          i++;

        if(i < SIZE_DEFAULT)
        {
          if((data->userImage[i] = MakeImageObject(id, filename)) != NULL)
          {
            data->userImageIndex[i] = folder->ImageIndex;

            DoMethod(obj, MUIM_NList_UseImage, data->userImage[i], data->userImageIndex[i]);
          }
        }
      }
    }
  }

  RETURN(tn);
  return (IPTR)tn;
}

///
