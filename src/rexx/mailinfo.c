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

#include <clib/alib_protos.h>
#include <mui/NList_mcc.h>

#include "extrasrc.h"

#include "YAM.h"
#include "YAM_main.h"
#include "YAM_mainFolder.h"
#include "classes/Classes.h"

#include "Rexx.h"

#include "Debug.h"

void rx_mailinfo(UNUSED struct RexxHost *host, void **rxd, enum RexxAction action, UNUSED struct RexxMsg *rexxmsg)
{
  struct
  {
    struct rxd_mailinfo rd;
    long active;
    char from[SIZE_ADDRESS], to[SIZE_ADDRESS], replyto[SIZE_ADDRESS], flags[SIZE_SMALL];
    char filename[SIZE_PATHFILE], date[64], msgid[9];
  } *rd = *rxd;

  ENTER();

  switch(action)
  {
    case RXIF_INIT:
    {
      *rxd = AllocVecPooled(G->SharedMemPool, sizeof(*rd));
    }
    break;

    case RXIF_ACTION:
    {
      struct Mail *mail = NULL;
      struct Folder *folder = NULL;

      if(rd->rd.arg.index)
      {
        Object *lv = (Object *)xget(G->MA->GUI.PG_MAILLIST, MUIA_MainMailListGroup_MainList);
        rd->active = *rd->rd.arg.index;
        DoMethod(lv, MUIM_NList_GetEntry, rd->active, &mail);
      }
      else
        mail = MA_GetActiveMail(NULL, &folder, (LONG *)&rd->active);

      if(mail)
      {
        int pf = getPERValue(mail);
        int vf = getVOLValue(mail);

        GetMailFile(rd->rd.res.filename = rd->filename, folder, mail);
        rd->rd.res.index = &rd->active;

        if(hasStatusError(mail))
          rd->rd.res.status = "E"; // Error status
        else if(hasStatusQueued(mail))
          rd->rd.res.status = "W"; // Queued (WaitForSend) status
        else if(hasStatusHold(mail))
          rd->rd.res.status = "H"; // Hold status
        else if(hasStatusSent(mail))
          rd->rd.res.status = "S"; // Sent status
        else if(hasStatusReplied(mail))
          rd->rd.res.status = "R"; // Replied status
        else if(hasStatusForwarded(mail))
          rd->rd.res.status = "F"; // Forwarded status
        else if(!hasStatusRead(mail))
        {
          if(hasStatusNew(mail))
            rd->rd.res.status = "N"; // New status
          else
            rd->rd.res.status = "U"; // Unread status
        }
        else if(!hasStatusNew(mail))
          rd->rd.res.status = "O"; // Old status

        rd->rd.res.from = BuildAddress(rd->from, sizeof(rd->from), mail->From.Address, mail->From.RealName);
        rd->rd.res.to = BuildAddress(rd->to, sizeof(rd->to), mail->To.Address, mail->To.RealName);
        rd->rd.res.replyto = BuildAddress(rd->replyto, sizeof(rd->replyto), mail->ReplyTo.Address[0] != '\0' ? mail->ReplyTo.Address : mail->From.Address,
                                                                            mail->ReplyTo.Address[0] != '\0' ? mail->ReplyTo.RealName : mail->From.RealName);

        DateStamp2String(rd->rd.res.date = rd->date, sizeof(rd->date), &mail->Date, DSS_USDATETIME, TZC_LOCAL);
        rd->rd.res.subject = mail->Subject;
        rd->rd.res.size = &mail->Size;
        snprintf(rd->rd.res.msgid = rd->msgid, sizeof(rd->msgid), "%lX", mail->cMsgID);
        snprintf(rd->rd.res.flags = rd->flags, sizeof(rd->flags), "%c%c%c%c%c-%c%c%c",
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
