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

#include <ctype.h>
#include <proto/exec.h>

#include "extrasrc.h"

#include "YAM.h"
#include "YAM_addressbook.h"
#include "YAM_addressbookEntry.h"

#include "Locale.h"
#include "Rexx.h"

#include "Debug.h"

struct rxd_addrnew
{
  long rc, rc2;
  struct {
    char *var, *stem;
    char *type;
    char *alias;
    char *name;
    char *email;
  } arg;
  struct {
    char *alias;
  } res;
};

void rx_addrnew(UNUSED struct RexxHost *host, void **rxd, enum RexxAction action, UNUSED struct RexxMsg *rexxmsg)
{
  struct rxd_addrnew *rd = *rxd;

  ENTER();

  switch(action)
  {
    case RXIF_INIT:
    {
      if((*rxd = AllocVecPooled(G->SharedMemPool, sizeof(*rd))) != NULL)
        ((struct rxd_addrnew *)(*rxd))->rc = offsetof(struct rxd_addrnew, res) / sizeof(long);
    }
    break;

    case RXIF_ACTION:
    {
      static struct ABEntry addr;

      memset(&addr, 0, sizeof(struct ABEntry));
      addr.Type = AET_USER;
      addr.Members = (char *)"";

      if(rd->arg.type)
      {
        if(tolower(*rd->arg.type) == 'g')
          addr.Type = AET_GROUP;
        else if(tolower(*rd->arg.type) == 'l')
          addr.Type = AET_LIST;
      }

      if(rd->arg.alias)    strlcpy(addr.Alias, rd->arg.alias, sizeof(addr.Alias));
      if(rd->arg.name)     strlcpy(addr.RealName, rd->arg.name, sizeof(addr.RealName));
      if(rd->arg.email)    strlcpy(addr.Address, rd->arg.email, sizeof(addr.Address));

      if(!*addr.Alias)
      {
        if(addr.Type == AET_USER)
          EA_SetDefaultAlias(&addr);
        else
          rd->rc = RETURN_ERROR;
      }

      if(!rd->rc)
      {
        EA_FixAlias(&addr, FALSE);
        rd->res.alias = addr.Alias;
        EA_InsertBelowActive(&addr, addr.Type == AET_GROUP ? TNF_LIST : 0);
        G->AB->Modified = TRUE;
        AppendToLogfile(LF_VERBOSE, 71, tr(MSG_LOG_NewAddress), addr.Alias);
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
