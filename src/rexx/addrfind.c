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

#include <proto/exec.h>

#include "extrasrc.h"

#include "YAM.h"
#include "YAM_addressbook.h"
#include "YAM_addressbookEntry.h"

#include "Rexx.h"

#include "Debug.h"

struct rxd_addrfind
{
  long rc, rc2;
  struct {
    char *var, *stem;
    char *pattern;
    long nameonly;
    long emailonly;
  } arg;
  struct {
    char **alias;
  } res;
};

void rx_addrfind(UNUSED struct RexxHost *host, void **rxd, enum RexxAction action, UNUSED struct RexxMsg *rexxmsg)
{
  struct rxd_addrfind *rd = *rxd;

  ENTER();

  switch(action)
  {
    case RXIF_INIT:
    {
      if((*rxd = AllocVecPooled(G->SharedMemPool, sizeof(*rd))) != NULL)
        ((struct rxd_addrfind *)(*rxd))->rc = offsetof(struct rxd_addrfind, res) / sizeof(long);
    }
    break;

    case RXIF_ACTION:
    {
      int hits;
      int mode;

      if(rd->arg.nameonly)
        mode = rd->arg.emailonly ? ABF_RX_NAMEEMAIL : ABF_RX_NAME;
      else
        mode = rd->arg.emailonly ? ABF_RX_EMAIL     : ABF_RX;

      if((hits = AB_FindEntry(rd->arg.pattern, mode, NULL)) > 0)
      {
        rd->res.alias = calloc(hits+1, sizeof(char *));
        if(AB_FindEntry(rd->arg.pattern, mode, rd->res.alias) == 0)
          rd->rc = RETURN_WARN;
      }
      else
        rd->rc = RETURN_WARN;
    }
    break;

    case RXIF_FREE:
    {
      if(rd->res.alias)
        free(rd->res.alias);

      FreeVecPooled(G->SharedMemPool, rd);
    }
    break;
  }

  LEAVE();
}
