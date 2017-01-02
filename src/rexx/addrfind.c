/***************************************************************************

 YAM - Yet Another Mailer
 Copyright (C) 1995-2000 Marcel Beck
 Copyright (C) 2000-2017 YAM Open Source Team

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
#include <proto/exec.h>

#include "extrasrc.h"

#include "YAM.h"

#include "Rexx.h"

#include "Debug.h"

struct args
{
  struct RexxResult varStem;
  char *pattern;
  long nameonly;
  long emailonly;
};

struct results
{
  char **alias;
};

void rx_addrfind(UNUSED struct RexxHost *host, struct RexxParams *params, enum RexxAction action, UNUSED struct RexxMsg *rexxmsg)
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
      ULONG mode;
      ULONG hits;

      if(args->nameonly)
        mode = args->emailonly ? ASM_ADDRESS|ASM_REALNAME|ASM_USER|ASM_LIST : ASM_REALNAME|ASM_USER|ASM_LIST;
      else
        mode = args->emailonly ? ASM_ADDRESS|ASM_USER|ASM_LIST : ASM_ALIAS|ASM_COMMENT|ASM_REALNAME|ASM_ADDRESS|ASM_USERINFO|ASM_USER|ASM_LIST;

      if((hits = PatternSearchABook(&G->abook, args->pattern, mode, NULL)) != 0)
      {
        if((results->alias = calloc(hits+1, sizeof(char *))) != NULL)
          PatternSearchABook(&G->abook, args->pattern, mode, results->alias);
        else
          params->rc = RETURN_ERROR;
      }
      else
        params->rc = RETURN_WARN;
    }
    break;

    case RXIF_FREE:
    {
      if(args != NULL)
        FreeVecPooled(G->SharedMemPool, args);
      if(results != NULL)
      {
        free(results->alias);
        FreeVecPooled(G->SharedMemPool, results);
      }
    }
    break;
  }

  LEAVE();
}
