/***************************************************************************

 YAM - Yet Another Mailer
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

#include "extrasrc.h"

#include "YAM.h"

#include "FolderList.h"
#include "Rexx.h"

#include "Debug.h"

void rx_folderinfo(UNUSED struct RexxHost *host, void **rxd, enum RexxAction action, UNUSED struct RexxMsg *rexxmsg)
{
  struct rxd_folderinfo *rd = *rxd;

  switch(action)
  {
    case RXIF_INIT:
    {
      *rxd = AllocVecPooled(G->SharedMemPool, sizeof(*rd));
    }
    break;

    case RXIF_ACTION:
    {
      struct Folder *fo = rd->arg.folder ? FO_GetFolderRexx(rd->arg.folder, NULL) : FO_GetCurrentFolder();

      // this command should only act on a folder and
      // only on a non-group
      if(fo && !isGroupFolder(fo))
      {
        static int num;
        static int total;
        static int new;
        static int unread;
        static LONG size;
        static int type;

        LockFolderListShared(G->folders);
        num = FO_GetFolderPosition(fo, FALSE);
        UnlockFolderList(G->folders);

        total = fo->Total;
        new = fo->New;
        unread = fo->Unread;
        size = fo->Size;
        type = fo->Type;

        rd->res.number = &num;
        rd->res.name = fo->Name;
        rd->res.path = fo->Path;
        rd->res.total = &total;
        rd->res.new = &new;
        rd->res.unread = &unread;
        rd->res.size = &size;
        rd->res.type = &type;
      }
      else
        rd->rc = RETURN_ERROR;
    }
    break;

    case RXIF_FREE:
    {
      FreeVecPooled(G->SharedMemPool, rd);
    }
    break;
  }

  LEAVE();
}
