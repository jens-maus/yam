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

#include <stdlib.h>
#include <strings.h>

#include <clib/alib_protos.h>
#include <mui/NList_mcc.h>
#include <proto/exec.h>

#include "extrasrc.h"

#include "YAM.h"
#include "YAM_main.h"
#include "YAM_mainFolder.h"

#include "mui/ClassesExtra.h"
#include "mui/MainMailListGroup.h"

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
  char *cc;
  char *bcc;
  char *resentto;
  char *subject;
  char *filename;
  long *size;
  char *date;
  char *flags;
  char *msgid;
  char **fromall;
  char **toall;
  char **replytoall;
  char **ccall;
  char **bccall;
  char **resenttoall;
};

struct optional
{
  long active;
  char address[SIZE_ADDRESS];
  char flags[SIZE_SMALL];
  char filename[SIZE_PATHFILE];
  char date[64];
  char *msgid;
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

      if(args->index != NULL)
      {
        Object *lv = (Object *)xget(G->MA->GUI.PG_MAILLIST, MUIA_MainMailListGroup_MainList);

        optional->active = args->index[0];
        DoMethod(lv, MUIM_NList_GetEntry, optional->active, &mail);
        folder = mail->Folder;
      }
      else
        mail = MA_GetActiveMail(NULL, &folder, (APTR)&optional->active);

      if(mail != NULL)
      {
        struct ExtendedMail *email;

        if((email = MA_ExamineMail(folder, mail->MailFile, TRUE)) != NULL)
        {
          int pf = getPERValue(mail);
          int vf = getVOLValue(mail);
          int i;

          GetMailFile(optional->filename, sizeof(optional->filename), NULL, mail);
          results->filename = optional->filename;
          results->index = &optional->active;

          if(hasStatusError(mail))
            results->status = "E"; // Error status
          else if(isOutgoingFolder(mail->Folder))
            results->status = "W"; // Queued (WaitForSend) status
          else if(isDraftsFolder(mail->Folder))
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

          if((results->fromall = calloc(email->NumSFrom+2, sizeof(char *))) != NULL)
          {
            if((results->fromall[0] = strdup(BuildAddress(optional->address, sizeof(optional->address), mail->From.Address, mail->From.RealName))) == NULL)
              params->rc = RETURN_ERROR;

            for(i = 0; i < email->NumSFrom; i++)
            {
              if((results->fromall[i+1] = strdup(BuildAddress(optional->address, sizeof(optional->address), email->SFrom[i].Address, email->SFrom[i].RealName))) == NULL)
                params->rc = RETURN_ERROR;
            }
            results->from = results->fromall[0];
          }
          else
            params->rc = RETURN_ERROR;

          if((results->toall = calloc(email->NumSTo+2, sizeof(char *))) != NULL)
          {
            if((results->toall[0] = strdup(BuildAddress(optional->address, sizeof(optional->address), mail->To.Address, mail->To.RealName))) == NULL)
              params->rc = RETURN_ERROR;

            for(i = 0; i < email->NumSTo; i++)
            {
              if((results->toall[i+1] = strdup(BuildAddress(optional->address, sizeof(optional->address), email->STo[i].Address, email->STo[i].RealName))) == NULL)
                params->rc = RETURN_ERROR;
            }
            results->to = results->toall[0];
          }
          else
            params->rc = RETURN_ERROR;

          if((results->replytoall = calloc(email->NumSReplyTo+2, sizeof(char *))) != NULL)
          {
            if((results->replytoall[0] = strdup(BuildAddress(optional->address, sizeof(optional->address), mail->ReplyTo.Address[0] != '\0' ? mail->ReplyTo.Address : mail->From.Address, mail->ReplyTo.Address[0] != '\0' ? mail->ReplyTo.RealName : mail->From.RealName))) == NULL)
              params->rc = RETURN_ERROR;

            for(i = 0; i < email->NumSReplyTo; i++)
            {
              if((results->replytoall[i+1] = strdup(BuildAddress(optional->address, sizeof(optional->address), email->SReplyTo[i].Address, email->SReplyTo[i].RealName))) == NULL)
                params->rc = RETURN_ERROR;
            }
            results->replyto = results->replytoall[0];
          }
          else
            params->rc = RETURN_ERROR;

          if((results->ccall = calloc(email->NumCC+1, sizeof(char *))) != NULL)
          {
            for(i = 0; i < email->NumCC; i++)
            {
              if((results->ccall[i] = strdup(BuildAddress(optional->address, sizeof(optional->address), email->CC[i].Address, email->CC[i].RealName))) == NULL)
                params->rc = RETURN_ERROR;
            }
            results->cc = results->ccall[0];
          }
          else
            params->rc = RETURN_ERROR;

          if((results->bccall = calloc(email->NumBCC+1, sizeof(char *))) != NULL)
          {
            for(i = 0; i < email->NumBCC; i++)
            {
              if((results->bccall[i] = strdup(BuildAddress(optional->address, sizeof(optional->address), email->BCC[i].Address, email->BCC[i].RealName))) == NULL)
                params->rc = RETURN_ERROR;
            }
            results->bcc = results->bccall[0];
          }
          else
            params->rc = RETURN_ERROR;

          if((results->resenttoall = calloc(email->NumResentTo+1, sizeof(char *))) != NULL)
          {
            for(i = 0; i < email->NumResentTo; i++)
            {
              if((results->resenttoall[i] = strdup(BuildAddress(optional->address, sizeof(optional->address), email->ResentTo[i].Address, email->ResentTo[i].RealName))) == NULL)
                params->rc = RETURN_ERROR;
            }
            results->resentto = results->resenttoall[0];
          }
          else
            params->rc = RETURN_ERROR;

          DateStamp2String(results->date = optional->date, sizeof(optional->date), &mail->Date, DSS_USDATETIME, TZC_LOCAL);
          results->subject = mail->Subject;
          results->size = &mail->Size;
          results->msgid = strdup(email->messageID);
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

          MA_FreeEMailStruct(email);
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

        if(results->fromall != NULL)
        {
          i = 0;
          while(results->fromall[i] != NULL)
          {
            free(results->fromall[i]);
            i++;
          }
          free(results->fromall);
        }
        if(results->toall != NULL)
        {
          i = 0;
          while(results->toall[i] != NULL)
          {
            free(results->toall[i]);
            i++;
          }
          free(results->toall);
        }
        if(results->replytoall != NULL)
        {
          i = 0;
          while(results->replytoall[i] != NULL)
          {
            free(results->replytoall[i]);
            i++;
          }
          free(results->replytoall);
        }
        if(results->ccall != NULL)
        {
          i = 0;
          while(results->ccall[i] != NULL)
          {
            free(results->ccall[i]);
            i++;
          }
          free(results->ccall);
        }
        if(results->bccall != NULL)
        {
          i = 0;
          while(results->bccall[i] != NULL)
          {
            free(results->bccall[i]);
            i++;
          }
          free(results->bccall);
        }
        if(results->resenttoall != NULL)
        {
          i = 0;
          while(results->resenttoall[i] != NULL)
          {
            free(results->resenttoall[i]);
            i++;
          }
          free(results->resenttoall);
        }
        free(results->msgid);
        FreeVecPooled(G->SharedMemPool, results);
      }
      if(optional != NULL)
        FreeVecPooled(G->SharedMemPool, optional);
    }
    break;
  }

  LEAVE();
}
