/***************************************************************************

 YAM - Yet Another Mailer
 Copyright (C) 1995-2000 by Marcel Beck <mbeck@yam.ch>
 Copyright (C) 2000-2010 by YAM Open Source Team

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
#include "YAM_config.h"
#include "YAM_main.h"

#include "MailServers.h"
#include "Rexx.h"

#include "Debug.h"

struct args
{
  struct RexxResult varStem;
  long *pop;
  long manual;
};

struct results
{
  long *downloaded;
  long *onserver;
  long *dupskipped;
  long *deleted;
};

void rx_mailcheck(UNUSED struct RexxHost *host, struct RexxParams *params, enum RexxAction action, UNUSED struct RexxMsg *rexxmsg)
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
      int popnr = -2;

      if(args->pop)
      {
        int pop;
        int maxpop=0;
        struct Node *curNode;

        IterateList(&C->mailServerList, curNode)
        {
          struct MailServerNode *msn = (struct MailServerNode *)curNode;

          if(isPOP3Server(msn))
            maxpop++;
        }

        if((pop = *args->pop) >= 0 && pop < maxpop)
          popnr = pop;
      }
      else
        popnr = -1;

      if(popnr > -2)
      {
        static long remaining;

        MA_PopNow(args->manual ? POP_USER : POP_REXX, popnr);

        remaining = G->LastDL.OnServer - G->LastDL.Deleted;

        results->downloaded = &G->LastDL.Downloaded;
        results->onserver = &remaining;
        results->dupskipped = &G->LastDL.DupSkipped;
        results->deleted = &G->LastDL.Deleted;

        if(G->LastDL.Error)
          params->rc = RETURN_WARN;
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
        FreeVecPooled(G->SharedMemPool, results);
    }
    break;
  }

  LEAVE();
}
