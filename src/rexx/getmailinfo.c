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

#include <string.h>

#include <proto/exec.h>

#include "extrasrc.h"

#include "YAM.h"
#include "YAM_main.h"
#include "YAM_mainFolder.h"

#include "Rexx.h"

#include "Debug.h"

struct args
{
  struct RexxResult varStem;
  char *item;
};

struct results
{
  char *value;
};

struct optional
{
  char result[SIZE_LARGE];
};

void rx_getmailinfo(UNUSED struct RexxHost *host, struct RexxParams *params, enum RexxAction action, UNUSED struct RexxMsg *rexxmsg)
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
    }
    break;

    case RXIF_ACTION:
    {
      struct Mail *mail;
      LONG active = 0;

      if((mail = MA_GetActiveMail(NULL, NULL, &active)))
      {
        char *key;

        results->value = optional->result;
        key = args->item;

        if(!strnicmp(key, "ACT", 3))
          snprintf(optional->result, sizeof(optional->result), "%d", (unsigned int)active);
        else if(!strnicmp(key, "STA", 3))
        {
          if(hasStatusError(mail))
            results->value = (char *)"E"; // Error status
          else if(hasStatusQueued(mail))
            results->value = (char *)"W"; // Queued (WaitForSend) status
          else if(hasStatusHold(mail))
            results->value = (char *)"H"; // Hold status
          else if(hasStatusSent(mail))
            results->value = (char *)"S"; // Sent status
          else if(hasStatusReplied(mail))
            results->value = (char *)"R"; // Replied status
          else if(hasStatusForwarded(mail))
            results->value = (char *)"F"; // Forwarded status
          else if(!hasStatusRead(mail))
          {
            if(hasStatusNew(mail))
              results->value = (char *)"N"; // New status
            else
              results->value = (char *)"U"; // Unread status
          }
          else if(!hasStatusNew(mail))
            results->value = (char *)"O"; // Old status
        }
        else if(!strnicmp(key, "FRO", 3))
          BuildAddress(optional->result, sizeof(optional->result), mail->From.Address, mail->From.RealName);
        else if(!strnicmp(key, "TO" , 2))
          BuildAddress(optional->result, sizeof(optional->result), mail->To.Address, mail->To.RealName);
        else if(!strnicmp(key, "REP", 3))
        {
          BuildAddress(optional->result, sizeof(optional->result), mail->ReplyTo.Address[0] != '\0' ? mail->ReplyTo.Address : mail->From.Address,
                                                                   mail->ReplyTo.Address[0] != '\0' ? mail->ReplyTo.RealName : mail->From.RealName);
        }
        else if(!strnicmp(key, "SUB", 3))
          results->value = mail->Subject;
        else if(!strnicmp(key, "FIL", 3))
        {
          GetMailFile(optional->result, sizeof(optional->result), NULL, mail);
          results->value = optional->result;
        }
        else
          params->rc = RETURN_ERROR;
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
      if(optional != NULL)
        FreeVecPooled(G->SharedMemPool, optional);
    }
    break;
  }

  LEAVE();
}
