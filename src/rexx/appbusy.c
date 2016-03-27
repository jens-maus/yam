/***************************************************************************

 YAM - Yet Another Mailer
 Copyright (C) 1995-2000 Marcel Beck
 Copyright (C) 2000-2016 YAM Open Source Team

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
#include <proto/muimaster.h>

#include "extrasrc.h"

#include "YAM.h"

#include "Busy.h"
#include "Rexx.h"

#include "Debug.h"

struct args
{
  char *text;
};

void rx_appbusy(UNUSED struct RexxHost *host, struct RexxParams *params, enum RexxAction action, UNUSED struct RexxMsg *rexxmsg)
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
      struct BusyNode *busy;

      // return 0 if YAM was idle, 1 otherwise
      params->rc = IsMinListEmpty(&G->arexxBusyList) ? 0 : 1;

      busy = BusyBegin(BUSY_AREXX);

      // we don't make a text a requirement. If the user hasn't supplied
      // a text for the busytext we simply use an empty string.
      if(args->text != NULL)
        BusyText(busy, args->text, "");
      else
        BusyText(busy, " ", "");

      // put YAM to sleep only once
      if(params->rc == 0)
        nnset(G->App, MUIA_Application_Sleep, TRUE);
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
