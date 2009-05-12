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
  char **from;
  char **to;
  char **replyto;
  char **cc;
  char **bcc;
  char **resentto;
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
  char address[SIZE_ADDRESS];
  char flags[SIZE_SMALL];
  char filename[SIZE_PATHFILE];
  char date[64];
  char msgid[9];
};

void rx_mailfullinfo(UNUSED struct RexxHost *host, struct RexxParams *params, enum RexxAction action, UNUSED struct RexxMsg *rexxmsg)
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

      if(args->index != 0)
      {
        Object *lv = (Object *)xget(G->MA->GUI.PG_MAILLIST, MUIA_MainMailListGroup_MainList);
        optional->active = *args->index;
        DoMethod(lv, MUIM_NList_GetEntry, optional->active, &mail);
      }
      else
        mail = MA_GetActiveMail(NULL, &folder, (LONG *)&optional->active);

      if(mail != NULL)
      {
        struct ExtendedMail *email;

        if((email = MA_ExamineMail(folder, mail->MailFile, TRUE)) != NULL)
        {
          int pf = getPERValue(mail);
          int vf = getVOLValue(mail);
          int i;

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

          if((results->from = calloc(email->NoSFrom+2, sizeof(char *))) != NULL)
          {
            if((results->from[0] = strdup(BuildAddress(optional->address, sizeof(optional->address), mail->From.Address, mail->From.RealName))) == NULL)
              params->rc = RETURN_ERROR;
            for(i = 0; i < email->NoSFrom; i++)
            {
              if((results->from[i+1] = strdup(BuildAddress(optional->address, sizeof(optional->address), email->SFrom[i].Address, email->SFrom[i].RealName))) == NULL)
                params->rc = RETURN_ERROR;
            }
          }
          else
            params->rc = RETURN_ERROR;

          if((results->to = calloc(email->NoSTo+2, sizeof(char *))) != NULL)
          {
            if((results->to[0] = strdup(BuildAddress(optional->address, sizeof(optional->address), mail->To.Address, mail->To.RealName));
              params->rc = RETURN_ERROR;
            for(i = 0; i < email->NoSTo; i++)
            {
              if((results->to[i+1] = strdup(BuildAddress(optional->address, sizeof(optional->address), email->STo[i].Address, email->STo[i].RealName));#
                params->rc = RETURN_ERROR;
            }
          }
          else
            params->rc = RETURN_ERROR;

          if((results->replyto = calloc(email->NoSReplyTo+2, sizeof(char *))) != NULL)
          {
            if((results->replyto[0] = strdup(BuildAddress(optional->address, sizeof(optional->address), mail->ReplyTo.Address[0] != '\0' ? mail->ReplyTo.Address : mail->From.Address, mail->ReplyTo.Address[0] != '\0' ? mail->ReplyTo.RealName : mail->From.RealName))) == NULL)
              params->rc = RETURN_ERROR;
            for(i = 0; i < email->NoSReplyTo; i++)
            {
              if((results->replyto[i+1] = strdup(BuildAddress(optional->address, sizeof(optional->address), email->SReplyTo[i].Address, email->SReplyTo[i].RealName))) == NULL)
                params->rc = RETURN_ERROR;
            }
          }
          else
            params->rc = RETURN_ERROR;

          if((results->cc = calloc(email->NoCC+1, sizeof(char *))) != NULL)
          {
            for(i = 0; i < email->NoCC; i++)
            {
              if((results->cc[i] = strdup(BuildAddress(optional->address, sizeof(optional->address), email->CC[i].Address, email->CC[i].RealName))) == NULL)
                params->rc = RETURN_ERROR;
            }
          }
          else
            params->rc = RETURN_ERROR;

          if((results->bcc = calloc(email->NoBCC+1, sizeof(char *))) != NULL)
          {
            for(i = 0; i < email->NoBCC; i++)
            {
              if((results->bcc[i] = strdup(BuildAddress(optional->address, sizeof(optional->address), email->BCC[i].Address, email->BCC[i].RealName))) == NULL)
                params->rc = RETURN_ERROR;
            }
          }
          else
            params->rc = RETURN_ERROR;

          if((results->resentto = calloc(email->NoResentTo+1, sizeof(char *))) != NULL)
          {
            for(i = 0; i < email->NoResentTo; i++)
            {
              if((results->resentto[i] = strdup(BuildAddress(optional->address, sizeof(optional->address), email->ResentTo[i].Address, email->ResentTo[i].RealName))) == NULL)
                params->rc = RETURN_ERROR;
            }
          }
          else
            params->rc = RETURN_ERROR;

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

          free(email);
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
      {
        int i;

        if(results->from != NULL)
        {
          i = 0;
          while(results->from[i] != NULL)
          {
            free(results->from[i]);
            i++;
          }
          free(results->from);
        }
        if(results->to != NULL)
        {
          i = 0;
          while(results->to[i] != NULL)
          {
            free(results->to[i]);
            i++;
          }
          free(results->to);
        }
        if(results->replyto != NULL)
        {
          i = 0;
          while(results->replyto[i] != NULL)
          {
            free(results->replyto[i]);
            i++;
          }
          free(results->replyto);
        }
        if(results->cc != NULL)
        {
          i = 0;
          while(results->cc[i] != NULL)
          {
            free(results->cc[i]);
            i++;
          }
          free(results->cc);
        }
        if(results->bcc != NULL)
        {
          i = 0;
          while(results->bcc[i] != NULL)
          {
            free(results->bcc[i]);
            i++;
          }
          free(results->bcc);
        }
        if(results->resentto != NULL)
        {
          i = 0;
          while(results->resentto[i] != NULL)
          {
            free(results->resentto[i]);
            i++;
          }
          free(results->resentto);
        }
        FreeVecPooled(G->SharedMemPool, results);
      }
      if(optional != NULL)
        FreeVecPooled(G->SharedMemPool, optional);
    }
    break;
  }

  LEAVE();
}
