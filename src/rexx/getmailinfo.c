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
#include "YAM_main.h"
#include "YAM_mainFolder.h"

#include "Rexx.h"

#include "Debug.h"

struct rxd_getmailinfo
{
  long rc, rc2;
  struct {
    char *var, *stem;
    char *item;
  } arg;
  struct {
    char *value;
  } res;
};

void rx_getmailinfo(UNUSED struct RexxHost *host, void **rxd, enum RexxAction action, UNUSED struct RexxMsg *rexxmsg)
{
  struct
  {
    struct rxd_getmailinfo rd;
    char result[SIZE_LARGE];
  } *rd = *rxd;

  ENTER();

  switch(action)
  {
    case RXIF_INIT:
    {
      if((*rxd = AllocVecPooled(G->SharedMemPool, sizeof(*rd))) != NULL)
        ((struct rxd_getmailinfo *)(*rxd))->rc = offsetof(struct rxd_getmailinfo, res) / sizeof(long);
    }
    break;

    case RXIF_ACTION:
    {
      struct Mail *mail;
      LONG active = 0;

      if((mail = MA_GetActiveMail(NULL, NULL, &active)))
      {
        char *key;

        rd->rd.res.value = rd->result;
        key = rd->rd.arg.item;

        if(!strnicmp(key, "ACT", 3))
          snprintf(rd->result, sizeof(rd->result), "%ld", active);
        else if(!strnicmp(key, "STA", 3))
        {
          if(hasStatusError(mail))
            rd->rd.res.value = (char *)"E"; // Error status
          else if(hasStatusQueued(mail))
            rd->rd.res.value = (char *)"W"; // Queued (WaitForSend) status
          else if(hasStatusHold(mail))
            rd->rd.res.value = (char *)"H"; // Hold status
          else if(hasStatusSent(mail))
            rd->rd.res.value = (char *)"S"; // Sent status
          else if(hasStatusReplied(mail))
            rd->rd.res.value = (char *)"R"; // Replied status
          else if(hasStatusForwarded(mail))
            rd->rd.res.value = (char *)"F"; // Forwarded status
          else if(!hasStatusRead(mail))
          {
            if(hasStatusNew(mail))
              rd->rd.res.value = (char *)"N"; // New status
            else
              rd->rd.res.value = (char *)"U"; // Unread status
          }
          else if(!hasStatusNew(mail))
            rd->rd.res.value = (char *)"O"; // Old status
        }
        else if(!strnicmp(key, "FRO", 3))
          BuildAddress(rd->result, sizeof(rd->result), mail->From.Address, mail->From.RealName);
        else if(!strnicmp(key, "TO" , 2))
          BuildAddress(rd->result, sizeof(rd->result), mail->To.Address, mail->To.RealName);
        else if(!strnicmp(key, "REP", 3))
        {
          BuildAddress(rd->result, sizeof(rd->result), mail->ReplyTo.Address[0] != '\0' ? mail->ReplyTo.Address : mail->From.Address,
                                                       mail->ReplyTo.Address[0] != '\0' ? mail->ReplyTo.RealName : mail->From.RealName);
        }
        else if(!strnicmp(key, "SUB", 3))
          rd->rd.res.value = mail->Subject;
        else if(!strnicmp(key, "FIL", 3))
        {
          GetMailFile(rd->result, mail->Folder, mail);
          rd->rd.res.value = rd->result;
        }
        else
          rd->rd.rc = RETURN_ERROR;
      }
      else
        rd->rd.rc = RETURN_ERROR;
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
