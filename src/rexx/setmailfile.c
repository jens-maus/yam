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

#include <string.h>

#include <clib/alib_protos.h>
#include <mui/NList_mcc.h>
#include <proto/dos.h>
#include <proto/exec.h>

#include "extrasrc.h"

#include "YAM.h"
#include "YAM_main.h"
#include "YAM_mainFolder.h"

#include "mui/ClassesExtra.h"
#include "mui/MainMailListGroup.h"

#include "MUIObjects.h"
#include "Rexx.h"

#include "Debug.h"

struct args
{
  char *mailfile;
};

void rx_setmailfile(UNUSED struct RexxHost *host, struct RexxParams *params, enum RexxAction action, UNUSED struct RexxMsg *rexxmsg)
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
      struct Mail *mail = NULL;
      Object *lv = (Object *)xget(G->MA->GUI.PG_MAILLIST, MUIA_MainMailListGroup_MainList);
      char *mfile;
      int i;

      mfile = (char *)FilePart(args->mailfile);

      for(i=0;;i++)
      {
        DoMethod(lv, MUIM_NList_GetEntry, i, &mail);
        if(!mail)
          break;

        if(stricmp(mail->MailFile, mfile) == 0)
        {
          DoMethod(lv, MUIM_NList_SetActive, i, MUIV_NList_SetActive_Jump_Center);
          break;
        }
      }

      if(!mail)
        params->rc = RETURN_WARN;
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
