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

#include <clib/alib_protos.h>
#include <libraries/mui.h>
#include <proto/exec.h>

#include "extrasrc.h"

#include "YAM.h"
#include "YAM_main.h"
#include "YAM_read.h"
#include "classes/Classes.h"

#include "MUIObjects.h"
#include "Rexx.h"

#include "Debug.h"

struct rxd_mailread
{
  long rc, rc2;
  struct {
    char *var, *stem;
    int *window;
    long quiet;
  } arg;
  struct {
    int *window;
  } res;
};

void rx_mailread(UNUSED struct RexxHost *host, void **rxd, enum RexxAction action, UNUSED struct RexxMsg *rexxmsg)
{
  struct rxd_mailread *rd = *rxd;

  ENTER();

  switch(action)
  {
    case RXIF_INIT:
    {
      if((*rxd = AllocVecPooled(G->SharedMemPool, sizeof(*rd))) != NULL)
        ((struct rxd_mailread *)(*rxd))->rc = offsetof(struct rxd_mailread, res) / sizeof(long);
    }
    break;

    case RXIF_ACTION:
    {
      static int winNumber = -1;

      rd->res.window = &winNumber;

      if(rd->arg.window == NULL)
      {
        struct Mail *mail;

        if((mail = MA_GetActiveMail(NULL, NULL, NULL)) != NULL)
        {
          struct ReadMailData *rmData;

          if((rmData = CreateReadWindow(TRUE)) != NULL)
          {
            G->ActiveRexxRMData = rmData;

            if(rd->arg.quiet == FALSE)
              SafeOpenWindow(rmData->readWindow);

            if(DoMethod(rmData->readWindow, MUIM_ReadWindow_ReadMail, mail) == FALSE)
            {
              // on any error we make sure to delete the read window
              // immediatly again.
              CleanupReadMailData(rmData, TRUE);

              rd->rc = RETURN_ERROR;
            }
            else
            {
              // Set the active Rexx RMData again, as this might have been overwritten
              // in the meantime by the read window itself. In this case the read window
              // will try to cleanup all the stuff allocated for a previous mail.
              G->ActiveRexxRMData = rmData;
              winNumber = xget(rmData->readWindow, MUIA_ReadWindow_Num);
            }
          }
          else
            rd->rc = RETURN_ERROR;
        }
        else
          rd->rc = RETURN_WARN;
      }
      else
      {
        // if a window number was specified with the command we have to search
        // through our ReadDataList and find the window with this particular
        // number
        int winnr = *rd->arg.window;
        struct MinNode *curNode = G->readMailDataList.mlh_Head;

        for(; curNode->mln_Succ; curNode = curNode->mln_Succ)
        {
          struct ReadMailData *rmData = (struct ReadMailData *)curNode;

          if(rmData->readWindow != NULL &&
             (int)xget(rmData->readWindow, MUIA_ReadWindow_Num) == winnr)
          {
            G->ActiveRexxRMData = rmData;
            winNumber = winnr;

            // bring the window to the user's attention
            if(rd->arg.quiet == FALSE)
              set(rmData->readWindow, MUIA_Window_Activate, TRUE);

            break;
          }
        }

        // check if we successfully found the window with that
        // number or if we have to return an error message
        if(curNode->mln_Succ == NULL)
          rd->rc = RETURN_ERROR;
      }
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
