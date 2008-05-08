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

#include <proto/exec.h>

#include "extrasrc.h"

#include "YAM.h"

#include "FolderList.h"
#include "Rexx.h"

#include "Debug.h"

void rx_getfolderinfo(UNUSED struct RexxHost *host, void **rxd, enum RexxAction action, UNUSED struct RexxMsg *rexxmsg)
{
  struct
  {
    struct rxd_getfolderinfo rd;
    char result[SIZE_SMALL];
  } *rd = *rxd;

  ENTER();

  switch(action)
  {
    case RXIF_INIT:
    {
      *rxd = AllocVecPooled(G->SharedMemPool, sizeof(*rd));
    }
    break;

    case RXIF_ACTION:
    {
      struct Folder *fo = FO_GetCurrentFolder();
      char *key = rd->rd.arg.item;

      // this command should only act on a folder folder and
      // also only on a non-group
      if(fo != NULL && !isGroupFolder(fo))
      {
        int num;

        LockFolderListShared(G->folders);
        num = FO_GetFolderPosition(fo, FALSE);
        UnlockFolderList(G->folders);

        if(!strnicmp(key, "NUM", 3))      snprintf(rd->rd.res.value = rd->result, sizeof(rd->result), "%d", num);
        else if(!strnicmp(key, "NAM", 3)) rd->rd.res.value = fo->Name;
        else if(!strnicmp(key, "PAT", 3)) rd->rd.res.value = fo->Path;
        else if(!strnicmp(key, "MAX", 3)) snprintf(rd->rd.res.value = rd->result, sizeof(rd->result), "%d", fo->Total);
        else rd->rd.rc = RETURN_ERROR;
      }
      else
        rd->rd.rc = RETURN_ERROR;
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
