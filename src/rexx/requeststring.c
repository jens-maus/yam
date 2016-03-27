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

#include <stdlib.h>
#include <strings.h>

#include <proto/exec.h>

#include "extrasrc.h"

#include "YAM.h"

#include "mui/ClassesExtra.h"
#include "mui/StringRequestWindow.h"

#include "Locale.h"
#include "Requesters.h"
#include "Rexx.h"

#include "Debug.h"

struct args
{
  struct RexxResult varStem;
  char *body;
  char *string;
  long secret;
};

struct results
{
  char *string;
};

struct optional
{
  char string[SIZE_DEFAULT];
};

void rx_requeststring(UNUSED struct RexxHost *host, struct RexxParams *params, enum RexxAction action, UNUSED struct RexxMsg *rexxmsg)
{
  struct args *args = params->args;
  struct results *results = params->results;
  struct optional *optional = params->optional;

  ENTER();

  switch(action)
  {
    case RXIF_INIT:
    {
      params->args = AllocVecPooled(G->SharedMemPool, sizeof(*args));
      params->results = AllocVecPooled(G->SharedMemPool, sizeof(*results));
      params->optional = AllocVecPooled(G->SharedMemPool, sizeof(*optional));
      if(params->optional == NULL)
        params->rc = RETURN_ERROR;
    }
    break;

    case RXIF_ACTION:
    {
      char *reqtext = AllocReqText(args->body);

      if(args->string)
        strlcpy(optional->string, args->string, sizeof(optional->string));

      params->rc = !StringRequest(optional->string, SIZE_DEFAULT, NULL, reqtext, tr(MSG_Okay), NULL, tr(MSG_Cancel), args->secret != 0, G->MA->GUI.WI);
      results->string = optional->string;

      free(reqtext);
    }
    break;

    case RXIF_FREE:
    {
      if(args != NULL)
        FreeVecPooled(G->SharedMemPool, args);
      if(results != NULL)
        FreeVecPooled(G->SharedMemPool, results);
      if(optional != NULL)
        FreeVecPooled(G->SharedMemPool, optional);
    }
    break;
  }

  LEAVE();
}
