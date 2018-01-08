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
#include "YAM_main.h"

#include "mui/ClassesExtra.h"
#include "mui/WriteWindow.h"

#include "Rexx.h"
#include "MUIObjects.h"

#include "Debug.h"

struct args
{
  struct RexxResult varStem;
  int *window;
  long quiet;
};

struct results
{
  int *window;
};

void rx_mailwrite(UNUSED struct RexxHost *host, struct RexxParams *params, enum RexxAction action, UNUSED struct RexxMsg *rexxmsg)
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
      static int winNumber = -1;

      if(args->window)
        winNumber = *args->window;
      else
        winNumber = -1;

      results->window = &winNumber;

      // check if there is already an open, quiet write window
      // which is linked as the current active rexx window and
      // if so we close it or otherwise the window will be "lost"
      if(G->ActiveRexxWMData != NULL &&
         G->ActiveRexxWMData->quietMode == TRUE)
      {
        CleanupWriteMailData(G->ActiveRexxWMData);
      }

      if(winNumber < 0)
      {
        struct WriteMailData *wmData;

        if((wmData = NewMessage(NMM_NEW, args->quiet ? NEWF_QUIET : 0L)) != NULL)
        {
          G->ActiveRexxWMData = wmData;

          if(wmData->window != NULL)
          {
            winNumber = xget(wmData->window, MUIA_WriteWindow_Num);

            if(args->quiet == FALSE)
              set(wmData->window, MUIA_Window_Activate, TRUE);
          }
        }
        else
          params->rc = RETURN_ERROR;
      }
      else
      {
        BOOL found = FALSE;
        struct WriteMailData *wmData;

        IterateList(&G->writeMailDataList, struct WriteMailData *, wmData)
        {
          if(wmData->window != NULL)
          {
            if(winNumber == (int)xget(wmData->window, MUIA_WriteWindow_Num))
            {
              G->ActiveRexxWMData = wmData;
              found = TRUE;

              if(args->quiet == FALSE)
                set(wmData->window, MUIA_Window_Activate, TRUE);

              break;
            }
          }
        }

        if(found == FALSE)
          params->rc = RETURN_ERROR;
      }
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
