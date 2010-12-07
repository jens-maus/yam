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
 MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.   See the
 GNU General Public License for more details.

 You should have received a copy of the GNU General Public License
 along with this program; if not, write to the Free Software
 Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA

 YAM Official Support Site : http://www.yam.ch
 YAM OpenSource project    : http://sourceforge.net/projects/yamos/

 $Id$

 Superclass:  MUIC_NList
 Description: a list showing all available folders

***************************************************************************/

#include "FolderRequestList_cl.h"

#include <mui/NList_mcc.h>

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
    MUIA_List_AutoVisible, TRUE,

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
      if(!isGroupFolder(fnode->folder))
      {
        // check if the folder is to be excluded
        if(fnode->folder != excludeFolder)
        {
          DoMethod(obj, MUIM_NList_InsertSingle, fnode->folder, MUIV_NList_Insert_Bottom);
          // mark the previously selected folder
          if(fnode->folder == prevFolder)
            set(obj, MUIA_NList_Active, pos);

          // count the added folders
          pos++;
        }
      }
    }

    UnlockFolderList(G->folders);
  }

  RETURN((IPTR)obj);
  return (IPTR)obj;
}

///
/// OVERLOAD(MUIM_NList_Display)
OVERLOAD(MUIM_NList_Display)
{
  struct MUIP_NList_Display *ndm = (struct MUIP_NList_Display *)msg;
  struct Folder *folder = (struct Folder *)ndm->entry;

  ENTER();

  if(folder != NULL)
  {
    ndm->strings[0] = folder->Name;
  }

  LEAVE();
  return 0;
}

///
