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

#include <clib/alib_protos.h>
#include <proto/exec.h>
#include <proto/muimaster.h>
#include <proto/utility.h>

#include "extrasrc.h"

#include "YAM.h"

#include "mui/AddressBookWindow.h"

#include "Rexx.h"

#include "Debug.h"

struct args
{
  char *alias;
};

void rx_addrdelete(UNUSED struct RexxHost *host, struct RexxParams *params, enum RexxAction action, UNUSED struct RexxMsg *rexxmsg)
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
      // if the command was called without any parameter it will delete the active entry
      // if not we search for the one in question and if found delete it.
      if(args->alias != NULL)
      {
        if(SearchABook(&G->abook, args->alias, ASM_ALIAS|ASM_USER|ASM_LIST|ASM_GROUP, &G->abook.arexxABN) != 0)
        {
          if(G->ABookWinObject != NULL)
            set(G->ABookWinObject, MUIA_AddressBookWindow_ActiveEntry, G->abook.arexxABN);
        }
      }

      if(G->abook.arexxABN != NULL)
      {
        RemoveABookNode(G->abook.arexxABN);
        DeleteABookNode(G->abook.arexxABN);
        G->abook.arexxABN = NULL;
        G->abook.modified = TRUE;

        // update an existing address book window as well
        if(G->ABookWinObject != NULL)
          DoMethod(G->ABookWinObject, MUIM_AddressBookWindow_RebuildTree);
      }
      else
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
