/***************************************************************************

 YAM - Yet Another Mailer
 Copyright (C) 1995-2000 Marcel Beck
 Copyright (C) 2000-2013 YAM Open Source Team

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

#include <stdlib.h>

#include <clib/alib_protos.h>
#include <mui/NList_mcc.h>
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
  long *num;
  char *msgid;
};

void rx_setmail(UNUSED struct RexxHost *host, struct RexxParams *params, enum RexxAction action, UNUSED struct RexxMsg *rexxmsg)
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
      Object *lv = (Object *)xget(G->MA->GUI.PG_MAILLIST, MUIA_MainMailListGroup_MainList);
      int num = -1;
      int max = xget(lv, MUIA_NList_Entries);

      if(args->msgid != NULL)
      {
        struct Mail *mail;

        // find the mail with the given message id first
        if((mail = FindMailByMsgID(GetCurrentFolder(), args->msgid)) != NULL)
        {
          int idx = 0;
          struct Mail *nlistMail;

          // now find the mail in the GUI
          do
          {
            DoMethod(G->MA->GUI.PG_MAILLIST, MUIM_NList_GetEntry, idx, &nlistMail);
            if(nlistMail == mail)
            {
              num = idx;
              break;
            }
            else

            idx++;
          }
          while(nlistMail != NULL);
        }
      }
      else
      {
        num = *args->num;
      }

      if(num >= 0 && num < max)
        DoMethod(lv, MUIM_NList_SetActive, num, MUIV_NList_SetActive_Jump_Center);
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
