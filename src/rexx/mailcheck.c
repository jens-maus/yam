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

#include <clib/alib_protos.h>
#include <proto/exec.h>

#include "extrasrc.h"

#include "YAM.h"
#include "YAM_main.h"

#include "mui/YAMApplication.h"

#include "Config.h"
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

struct optional
{
  struct DownloadResult dlResult;
  long remaining;
};

void rx_mailcheck(UNUSED struct RexxHost *host, struct RexxParams *params, enum RexxAction action, UNUSED struct RexxMsg *rexxmsg)
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
      int pop;

      optional->dlResult.downloaded = 0;
      optional->dlResult.onServer = 0;
      optional->dlResult.dupeSkipped = 0;
      optional->dlResult.deleted = 0;
      optional->dlResult.error = FALSE;
      optional->remaining = 0;

      if(args->pop == NULL)
        pop = -1;
      else
        pop = args->pop[0];

      if(pop >= -1)
      {
        DoMethod(G->App, MUIM_YAMApplication_StartMacro, MACRO_PREGET, "3");

        if(pop == -1)
        {
          struct MailServerNode *msn;
          struct DownloadResult dlResult;

          pop = 0;
          while((msn = GetMailServer(&C->pop3ServerList, pop)) != NULL)
          {
            // fetch mails from active servers only
            if(isServerActive(msn) == TRUE)
            {
              if(ReceiveMailsFromPOP(msn, args->manual ? RECEIVEF_USER|RECEIVEF_SIGNAL : RECEIVEF_AREXX|RECEIVEF_SIGNAL, &dlResult) == TRUE)
              {
                MiniMainLoop();

                optional->dlResult.downloaded += dlResult.downloaded;
                optional->dlResult.onServer += dlResult.onServer;
                optional->dlResult.dupeSkipped += dlResult.dupeSkipped;
                optional->dlResult.deleted += dlResult.deleted;
                optional->dlResult.error |= dlResult.error;
                optional->remaining += dlResult.onServer - dlResult.deleted;

                if(dlResult.error == TRUE)
                  params->rc = RETURN_WARN;
              }
              else
              {
                params->rc = RETURN_ERROR;
                break;
              }
            }

            pop++;
          }
        }
        else
        {
          struct MailServerNode *msn;

          if((msn = GetMailServer(&C->pop3ServerList, pop)) != NULL)
          {
            if(ReceiveMailsFromPOP(msn, args->manual ? RECEIVEF_USER|RECEIVEF_SIGNAL : RECEIVEF_AREXX|RECEIVEF_SIGNAL, &optional->dlResult) == TRUE)
            {
              MiniMainLoop();

              if(optional->dlResult.error == TRUE)
                params->rc = RETURN_WARN;
            }
            else
              params->rc = RETURN_ERROR;
          }
        }
      }
      else
        params->rc = RETURN_ERROR;

      results->downloaded = &optional->dlResult.downloaded;
      results->onserver = &optional->dlResult.onServer;
      results->dupskipped = &optional->dlResult.dupeSkipped;
      results->deleted = &optional->dlResult.deleted;
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
