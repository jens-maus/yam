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
 MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 GNU General Public License for more details.

 You should have received a copy of the GNU General Public License
 along with this program; if not, write to the Free Software
 Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA

 YAM Official Support Site :  http://www.yam.ch
 YAM OpenSource project    :  http://sourceforge.net/projects/yamos/

 $Id$

***************************************************************************/

#include <ctype.h>
#include <proto/exec.h>

#include "extrasrc.h"

#include "YAM.h"
#include "YAM_main.h"

#include "Rexx.h"

#include "Debug.h"

struct args
{
  char *status;
};

void rx_mailstatus(UNUSED struct RexxHost *host, struct RexxParams *params, enum RexxAction action, UNUSED struct RexxMsg *rexxmsg)
{
  struct args *args = params->args;

  ENTER();

  switch(action)
  {
    case RXIF_INIT:
    {
      params->args = AllocVecPooled(G->SharedMemPool, sizeof(*args));
    }
    break;

    case RXIF_ACTION:
    {
      switch(toupper(args->status[0]))
      {
        case 'N': // new
        {
          MA_SetStatusTo(SFLAG_NEW, SFLAG_READ, FALSE);
        }
        break;

        case 'O': // old
        case 'R': // read
        {
          MA_SetStatusTo(SFLAG_READ, SFLAG_NEW, FALSE);
        }
        break;

        case 'U': // unread
        {
          MA_SetStatusTo(SFLAG_NONE, SFLAG_NEW|SFLAG_READ, FALSE);
        }
        break;

        case 'H': // hold
        {
          MA_SetStatusTo(SFLAG_HOLD|SFLAG_READ, SFLAG_QUEUED|SFLAG_ERROR, FALSE);
        }
        break;

        case 'Q': // queued
        case 'W': // wait to be sent
        {
          MA_SetStatusTo(SFLAG_QUEUED|SFLAG_READ, SFLAG_SENT|SFLAG_HOLD|SFLAG_ERROR, FALSE);
        }
        break;

        default:
        {
          params->rc = RETURN_WARN;
        }
        break;
      }
    }
    break;

    case RXIF_FREE:
    {
      if(args != NULL)
        FreeVecPooled(G->SharedMemPool, args);
    }
    break;
  }

  LEAVE();
}
