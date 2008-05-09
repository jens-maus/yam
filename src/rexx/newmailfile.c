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
#include "YAM_main.h"
#include "YAM_mainFolder.h"

#include "Rexx.h"

#include "Debug.h"

struct rxd_newmailfile
{
  long rc, rc2;
  struct {
    char *var, *stem;
    char *folder;
  } arg;
  struct {
    char *filename;
  } res;
};

void rx_newmailfile(UNUSED struct RexxHost *host, void **rxd, enum RexxAction action, UNUSED struct RexxMsg *rexxmsg)
{
  struct
  {
    struct rxd_newmailfile rd;
    char result[SIZE_PATHFILE];
  } *rd = *rxd;

  ENTER();

  switch(action)
  {
    case RXIF_INIT:
    {
      if((*rxd = AllocVecPooled(G->SharedMemPool, sizeof(*rd))) != NULL)
        ((struct rxd_newmailfile *)(*rxd))->rc = offsetof(struct rxd_newmailfile, res) / sizeof(long);
    }
    break;

    case RXIF_ACTION:
    {
      struct Folder *folder;

      if(rd->rd.arg.folder)
        folder = FO_GetFolderRexx(rd->rd.arg.folder, NULL);
      else
        folder = FO_GetCurrentFolder();

      if(folder && !isGroupFolder(folder))
      {
        char mfile[SIZE_MFILE];
        strlcpy(rd->rd.res.filename = rd->result, MA_NewMailFile(folder, mfile), sizeof(rd->result));
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
