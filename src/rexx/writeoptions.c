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

#include <libraries/mui.h>
#include <proto/exec.h>

#include "extrasrc.h"

#include "YAM.h"
#include "YAM_write.h"

#include "Rexx.h"

#include "Debug.h"

struct args
{
  long delete;
  long receipt;
  long notif;
  long addinfo;
  long *importance;
  long *sig;
  long *security;
};

void rx_writeoptions(UNUSED struct RexxHost *host, struct RexxParams *params, enum RexxAction action, UNUSED struct RexxMsg *rexxmsg)
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
      if(G->WR[G->ActiveWriteWin])
      {
        setcheckmark(G->WR[G->ActiveWriteWin]->GUI.CH_DELSEND, args->delete);
        setcheckmark(G->WR[G->ActiveWriteWin]->GUI.CH_MDN, args->receipt);
        setcheckmark(G->WR[G->ActiveWriteWin]->GUI.CH_ADDINFO, args->addinfo);
        if(args->importance) setcycle(G->WR[G->ActiveWriteWin]->GUI.CY_IMPORTANCE, *args->importance);
        if(args->sig)        setmutex(G->WR[G->ActiveWriteWin]->GUI.RA_SIGNATURE, *args->sig);
        if(args->security)   setmutex(G->WR[G->ActiveWriteWin]->GUI.RA_SECURITY, *args->security);
      }
      else
        params->rc = RETURN_ERROR;
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
