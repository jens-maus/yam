/***************************************************************************

 YAM - Yet Another Mailer
 Copyright (C) 1995-2000 by Marcel Beck <mbeck@yam.ch>
 Copyright (C) 2000-2009 by YAM Open Source Team

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
#include <mui/NList_mcc.h>
#include <proto/exec.h>

#include "extrasrc.h"

#include "YAM.h"
#include "YAM_main.h"
#include "YAM_mainFolder.h"
#include "mui/Classes.h"

#include "MUIObjects.h"
#include "Rexx.h"

#include "Debug.h"

struct args
{
  struct RexxResult varStem;
  long *index;
};

struct results
{
  long *index;
  const char *status;
  char *from;
  char *to;
  char *replyto;
  char *subject;
  char *filename;
  long *size;
  char *date;
  char *flags;
  char *msgid;
};

struct optional
{
  long active;
  char from[SIZE_ADDRESS];
  char to[SIZE_ADDRESS];
  char replyto[SIZE_ADDRESS];
  char flags[SIZE_SMALL];
  char filename[SIZE_PATHFILE];
  char date[64];
  char msgid[9];
};

void rx_mailinfo(UNUSED struct RexxHost *host, struct RexxParams *params, enum RexxAction action, UNUSED struct RexxMsg *rexxmsg)
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
      struct Mail *mail = NULL;
      struct Folder *folder = NULL;

      if(args->index)
      {
        Object *lv = (Object *)xget(G->MA->GUI.PG_MAILLIST, MUIA_MainMailListGroup_MainList);
        optional->active = *args->index;
        DoMethod(lv, MUIM_NList_GetEntry, optional->active, &mail);
      }
      else
        mail = MA_GetActiveMail(NULL, &folder, (LONG *)&optional->active);

      if(mail)
      {
        int pf = getPERValue(mail);
        int vf = getVOLValue(mail);

        GetMailFile(results->filename = optional->filename, folder, mail);
        results->index = &optional->active;

        if(hasStatusError(mail))
          results->status = "E"; // Error status
        else if(hasStatusQueued(mail))
          results->status = "W"; // Queued (WaitForSend) status
        else if(hasStatusHold(mail))
          results->status = "H"; // Hold status
        else if(hasStatusSent(mail))
          results->status = "S"; // Sent status
        else if(hasStatusReplied(mail))
          results->status = "R"; // Replied status
        else if(hasStatusForwarded(mail))
          results->status = "F"; // Forwarded status
        else if(!hasStatusRead(mail))
        {
          if(hasStatusNew(mail))
            results->status = "N"; // New status
          else
            results->status = "U"; // Unread status
        }
        else if(!hasStatusNew(mail))
          results->status = "O"; // Old status

        results->from = BuildAddress(optional->from, sizeof(optional->from), mail->From.Address, mail->From.RealName);
        results->to = BuildAddress(optional->to, sizeof(optional->to), mail->To.Address, mail->To.RealName);
        results->replyto = BuildAddress(optional->replyto, sizeof(optional->replyto), mail->ReplyTo.Address[0] != '\0' ? mail->ReplyTo.Address : mail->From.Address,
                                                                            mail->ReplyTo.Address[0] != '\0' ? mail->ReplyTo.RealName : mail->From.RealName);

        DateStamp2String(results->date = optional->date, sizeof(optional->date), &mail->Date, DSS_USDATETIME, TZC_LOCAL);
        results->subject = mail->Subject;
        results->size = &mail->Size;
        snprintf(results->msgid = optional->msgid, sizeof(optional->msgid), "%lX", mail->cMsgID);
        snprintf(results->flags = optional->flags, sizeof(optional->flags), "%c%c%c%c%c-%c%c%c",
                  isMultiRCPTMail(mail) ? 'M' : '-',
                  isMP_MixedMail(mail)  ? 'A' : '-',
                  isMP_ReportMail(mail) ? 'R' : '-',
                  isMP_CryptedMail(mail)? 'C' : '-',
                  isMP_SignedMail(mail) ? 'S' : '-',
                  pf ? pf+'0' : '-',
                  vf ? vf+'0' : '-',
                  hasStatusMarked(mail) ? 'M' : '-'
               );
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
