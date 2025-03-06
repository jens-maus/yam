/***************************************************************************

 YAM - Yet Another Mailer
 Copyright (C) 1995-2000 Marcel Beck
 Copyright (C) 2000-2025 YAM Open Source Team

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
 YAM OpenSource project    :  https://github.com/jens-maus/yam/

***************************************************************************/

#include <clib/alib_protos.h>
#include <mui/TextEditor_mcc.h>
#include <proto/exec.h>

#include "extrasrc.h"

#include "YAM.h"
#include "YAM_write.h"

#include "mui/ClassesExtra.h"
#include "mui/WriteWindow.h"

#include "Rexx.h"

#include "Debug.h"

struct args
{
  struct RexxResult varStem;
  char *command;
};

struct results
{
  char *result;
};

void rx_writeeditor(UNUSED struct RexxHost *host, struct RexxParams *params, enum RexxAction action, UNUSED struct RexxMsg *rexxmsg)
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
      if(G->ActiveRexxWMData != NULL && G->ActiveRexxWMData->window != NULL)
      {
        ULONG p = DoMethod(G->ActiveRexxWMData->window, MUIM_WriteWindow_ArexxCommand, args->command);

        switch(p)
        {
          case FALSE: params->rc = RETURN_WARN; break;
          case TRUE:  break;
          default:    results->result = (char *)p; break;
        }
      }
      else
        params->rc = RETURN_ERROR;
    }
    break;

    case RXIF_FREE:
    {
      if(args != NULL)
        FreeVecPooled(G->SharedMemPool, args);

      if(results != NULL)
      {
        if(results->result != NULL)
          FreeVec(results->result);

        FreeVecPooled(G->SharedMemPool, results);
      }
    }
    break;
  }

  LEAVE();
}
