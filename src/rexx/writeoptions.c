/***************************************************************************

 YAM - Yet Another Mailer
 Copyright (C) 1995-2000 Marcel Beck
 Copyright (C) 2000-2018 YAM Open Source Team

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

#include "mui/ClassesExtra.h"
#include "mui/WriteWindow.h"

#include "Rexx.h"
#include "MUIObjects.h"

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
      if(G->ActiveRexxWMData != NULL && G->ActiveRexxWMData->window != NULL)
      {
        struct WriteMailData *wmData = G->ActiveRexxWMData;

        xset(wmData->window, MUIA_WriteWindow_DelSent,    args->delete,
                             MUIA_WriteWindow_MDN,        args->receipt,
                             MUIA_WriteWindow_AddInfo,    args->addinfo);

        if(args->importance)
          set(wmData->window, MUIA_WriteWindow_Importance, *args->importance);

        if(args->sig)
          set(wmData->window, MUIA_WriteWindow_Signature, *args->sig);

        if(args->security)
          set(wmData->window, MUIA_WriteWindow_Security, *args->security);
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
