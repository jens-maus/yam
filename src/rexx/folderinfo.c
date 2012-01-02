/***************************************************************************

 YAM - Yet Another Mailer
 Copyright (C) 1995-2000 Marcel Beck
 Copyright (C) 2000-2012 YAM Open Source Team

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

struct args
{
  struct RexxResult varStem;
  char *folder;
};

struct results
{
  int *number;
  char *name;
  char *path;
  int *total;
  int *new;
  int *unread;
  LONG *size;
  int *type;
};

void rx_folderinfo(UNUSED struct RexxHost *host, struct RexxParams *params, enum RexxAction action, UNUSED struct RexxMsg *rexxmsg)
{
  struct args *args = params->args;
  struct results *results = params->results;

  ENTER();

  switch(action)
  {
    case RXIF_INIT:
    {
      params->args = AllocVecPooled(G->SharedMemPool, sizeof(*args));
      params->results = AllocVecPooled(G->SharedMemPool, sizeof(*results));
    }
    break;

    case RXIF_ACTION:
    {
      struct Folder *fo = args->folder ? FO_GetFolderRexx(args->folder, NULL) : GetCurrentFolder();

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

        results->number = &num;
        results->name = fo->Name;
        results->path = fo->Path;
        results->total = &total;
        results->new = &new;
        results->unread = &unread;
        results->size = &size;
        results->type = &type;
      }
      else
        params->rc = RETURN_ERROR;
    }
    break;

    case RXIF_FREE:
    {
      if(args != NULL)
        FreeVecPooled(G->SharedMemPool, args);
      if(results != NULL)
        FreeVecPooled(G->SharedMemPool, results);
    }
    break;
  }

  LEAVE();
}
