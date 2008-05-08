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
#include "YAM_config.h"
#include "YAM_write.h"

#include "Rexx.h"

#include "Debug.h"

void rx_writeletter(UNUSED struct RexxHost *host, void **rxd, enum RexxAction action, UNUSED struct RexxMsg *rexxmsg)
{
  struct rxd_writeletter *rd = *rxd;

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
      if(G->WR[G->ActiveWriteWin] && CopyFile(G->WR_Filename[G->ActiveWriteWin], 0, rd->arg.file, 0))
      {
        if(C->UseSignature == TRUE && !rd->arg.nosig)
          WR_AddSignature(G->ActiveWriteWin, -1);

        FileToEditor(G->WR_Filename[G->ActiveWriteWin], G->WR[G->ActiveWriteWin]->GUI.TE_EDIT, TRUE);
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
