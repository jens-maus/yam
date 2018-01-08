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

#include <strings.h>
#include <stdlib.h>

#include <clib/alib_protos.h>
#include <proto/exec.h>
#include <proto/muimaster.h>

#include "extrasrc.h"

#include "YAM.h"

#include "mui/ClassesExtra.h"
#include "mui/RecipientString.h"

#include "DynamicString.h"
#include "Rexx.h"

#include "Debug.h"

struct args
{
  struct RexxResult varStem;
  char *alias;
};

struct results
{
  char *recpt;
};

struct optional
{
  char *string;
};

void rx_addrresolve(UNUSED struct RexxHost *host, struct RexxParams *params, enum RexxAction action, UNUSED struct RexxMsg *rexxmsg)
{
  struct args *args = params->args;
  struct results *results = params->results;
  struct optional *optional = params->optional;

  ENTER();

  switch(action)
  {
    case RXIF_INIT:
    {
      args = params->args = AllocVecPooled(G->SharedMemPool, sizeof(*args));
      results = params->results = AllocVecPooled(G->SharedMemPool, sizeof(*results));
      optional = params->optional = AllocVecPooled(G->SharedMemPool, sizeof(*optional));
      if(params->optional == NULL)
        params->rc = RETURN_ERROR;
    }
    break;

    case RXIF_ACTION:
    {
      // generate a "fake" RecipientstringObject and use it on the resolve task
      Object *str = RecipientStringObject,
                      MUIA_RecipientString_MultipleRecipients, TRUE,
                      MUIA_String_Contents,                    args->alias,
                    End;

      if(str != NULL)
      {
        STRPTR res = (STRPTR)DoMethod(str, MUIM_RecipientString_Resolve, MUIF_RecipientString_Resolve_NoCache);

        // did the string change?
        if(res != NULL && strcmp(args->alias, res) != 0)
        {
          dstrcpy(&optional->string, res);
          results->recpt = optional->string;
        }
        else
          params->rc = RETURN_WARN;

        MUI_DisposeObject(str);
      }
      else
        params->rc = RETURN_FAIL;
    }
    break;

    case RXIF_FREE:
    {
      if(args != NULL)
        FreeVecPooled(G->SharedMemPool, args);
      if(results != NULL)
        FreeVecPooled(G->SharedMemPool, results);
      if(optional != NULL)
      {
        dstrfree(optional->string);
        FreeVecPooled(G->SharedMemPool, optional);
      }
    }
    break;
  }

  LEAVE();
}
