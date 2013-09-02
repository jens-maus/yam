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

#include <strings.h>
#include <ctype.h>

#include <proto/exec.h>

#include "extrasrc.h"

#include "YAM.h"
#include "YAM_addressbook.h"
#include "YAM_addressbookEntry.h"

#include "Locale.h"
#include "Logfile.h"
#include "Rexx.h"

#include "mui/AddressBookWindow.h"

#include "Debug.h"

struct args
{
  struct RexxResult varStem;
  char *type;
  char *alias;
  char *name;
  char *email;
};

struct results
{
  char *alias;
};

void rx_addrnew(UNUSED struct RexxHost *host, struct RexxParams *params, enum RexxAction action, UNUSED struct RexxMsg *rexxmsg)
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
      static struct ABEntry addr;

      memset(&addr, 0, sizeof(struct ABEntry));
      addr.Type = AET_USER;
      addr.Members = (char *)"";

      if(args->type)
      {
        if(tolower(*args->type) == 'g')
          addr.Type = AET_GROUP;
        else if(tolower(*args->type) == 'l')
          addr.Type = AET_LIST;
      }

      if(args->alias)    strlcpy(addr.Alias, args->alias, sizeof(addr.Alias));
      if(args->name)     strlcpy(addr.RealName, args->name, sizeof(addr.RealName));
      if(args->email)    strlcpy(addr.Address, args->email, sizeof(addr.Address));

      if(!*addr.Alias)
      {
        if(addr.Type == AET_USER)
          EA_SetDefaultAlias(&addr);
        else
          params->rc = RETURN_ERROR;
      }

      if(params->rc == 0)
      {
        EA_FixAlias(&addr, FALSE);
        results->alias = addr.Alias;
        EA_InsertBelowActive(&addr, addr.Type == AET_GROUP ? TNF_LIST : 0);
        set(G->ABookWinObject, MUIA_AddressBookWindow_Modified, TRUE);
        AppendToLogfile(LF_VERBOSE, 71, tr(MSG_LOG_NewAddress), addr.Alias);
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
