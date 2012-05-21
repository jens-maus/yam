/***************************************************************************

 YAM - Yet Another Mailer
 Copyright (C) 1995-2000 Marcel Beck
 Copyright (C) 2000-2012 YAM Open Source Team

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

#include <mui/NList_mcc.h>
#include <mui/NListtree_mcc.h>

#include <clib/alib_protos.h>
#include <proto/dos.h>
#include <proto/exec.h>
#include <proto/timer.h>
#include <proto/utility.h>

#include "YAM.h"
#include "YAM_addressbookEntry.h"
#include "YAM_config.h"
#include "YAM_error.h"
#include "YAM_find.h"
#include "YAM_folderconfig.h"
#include "YAM_mainFolder.h"
#include "YAM_stringsizes.h"

#include "AppIcon.h"
#include "FolderList.h"
#include "Locale.h"
#include "Logfile.h"
#include "MailList.h"
#include "MailServers.h"
#include "MethodStack.h"
#include "MailTransferList.h"
#include "MUIObjects.h"
#include "Requesters.h"
#include "Threads.h"
#include "UIDL.h"

#include "mime/md5.h"
#include "mui/ClassesExtra.h"
#include "mui/PreselectionWindow.h"
#include "mui/StringRequestWindow.h"
#include "mui/TransferControlGroup.h"
#include "mui/YAMApplication.h"
#include "tcp/Connection.h"

#include "extrasrc.h"
#include "Debug.h"

/**************************************************************************/
// POP3 commands (RFC 1939)
// the order of the following enum & pointer array is important and have to
// match each other or weird things will happen.
enum POPCommand
{
  // POP3 standard commands
  POPCMD_CONNECT, POPCMD_USER, POPCMD_PASS, POPCMD_QUIT, POPCMD_STAT, POPCMD_LIST,
  POPCMD_RETR,    POPCMD_DELE, POPCMD_NOOP, POPCMD_RSET, POPCMD_APOP, POPCMD_TOP,
  POPCMD_UIDL,

  // POP3 extended commands
  POPCMD_STLS
};

static const char *const POPcmd[] =
{
  // POP3 standard commands
  "",     "USER", "PASS", "QUIT", "STAT", "LIST",
  "RETR", "DELE", "NOOP", "RSET", "APOP", "TOP",
  "UIDL",

  // POP3 extended commands
  "STLS"
};

// POP responses
#define POP_RESP_ERROR    "-ERR"
#define POP_RESP_OKAY     "+OK"

/**************************************************************************/
// local macros & defines

struct TransferContext
{
  struct Connection *connection;
  struct MailServerNode *msn;
  ULONG abortMask;
  ULONG wakeupMask;
  ULONG timerMask;
  Object *transferGroup;
  Object *preselectWindow;
  char pop3Buffer[SIZE_LINE];            // RFC 2821 says 1000 should be enough
  char lineBuffer[SIZE_LINE];
  char windowTitle[SIZE_DEFAULT];        // the preselection window's title
  char transferGroupTitle[SIZE_DEFAULT]; // the TransferControlGroup's title
  char host[SIZE_HOST];
  char password[SIZE_PASSWORD];
  int port;
  ULONG flags;
  struct UIDLhash *UIDLhashTable;   // for maintaining all UIDLs
  struct MinList *remoteFilters;
  struct MailTransferList *transferList;
  BOOL useTLS;
  struct DownloadResult downloadResult;
  struct FilterResult filterResult;
  int numberOfMails;
  long totalSize;
  LONG firstToPreselect;
  struct TimeVal lastUpdateTime;
};

/// ApplyRemoteFilters
//  Applies remote filters to a message
static void ApplyRemoteFilters(const struct MinList *filterList, struct MailTransferNode *tnode)
{
  struct Node *curNode;

  ENTER();

  IterateList(filterList, curNode)
  {
    struct FilterNode *filter = (struct FilterNode *)curNode;

    if(DoFilterSearch(filter, tnode->mail) == TRUE)
    {
      if(hasExecuteAction(filter) && filter->executeCmd[0] != '\0')
         LaunchCommand(filter->executeCmd, FALSE, OUT_STDOUT);

      if(hasPlaySoundAction(filter) && filter->playSound[0] != '\0')
         PlaySound(filter->playSound);

      if(hasDeleteAction(filter))
         setFlag(tnode->tflags, TRF_DELETE);
      else
         clearFlag(tnode->tflags, TRF_DELETE);

      if(hasSkipMsgAction(filter))
         clearFlag(tnode->tflags, TRF_TRANSFER);
      else
         setFlag(tnode->tflags, TRF_TRANSFER);

      // get out of this loop after a successful search
      break;
    }
  }

  LEAVE();
}

///
/// SendPOP3Command
//  Sends a command to the POP3 server
static char *SendPOP3Command(struct TransferContext *tc, const enum POPCommand command, const char *parmtext, const char *errorMsg)
{
  char *result = NULL;

  ENTER();

  // if we specified a parameter for the pop command lets add it now
  if(parmtext == NULL || parmtext[0] == '\0')
    snprintf(tc->pop3Buffer, sizeof(tc->pop3Buffer), "%s\r\n", POPcmd[command]);
  else
    snprintf(tc->pop3Buffer, sizeof(tc->pop3Buffer), "%s %s\r\n", POPcmd[command], parmtext);

  D(DBF_NET, "TCP: POP3 cmd '%s' with param '%s'", POPcmd[command], (command == POPCMD_PASS) ? "XXX" : SafeStr(parmtext));

  // send the pop command to the server and see if it was received somehow
  // and for a connect we don't send something or the server will get
  // confused.
  if(command == POPCMD_CONNECT || SendLineToHost(tc->connection, tc->pop3Buffer) > 0)
  {
    int received;

    // let us read the next line from the server and check if
    // some status message can be retrieved.
    if((received = ReceiveLineFromHost(tc->connection, tc->pop3Buffer, sizeof(tc->pop3Buffer))) > 0 &&
      strncmp(tc->pop3Buffer, POP_RESP_OKAY, strlen(POP_RESP_OKAY)) == 0)
    {
      // everything worked out fine so lets set
      // the result to our allocated buffer
      result = tc->pop3Buffer;
    }
    else
    {
      BOOL showError;

      // don't show an error message for a failed QUIT command with no answer at all
      if(command == POPCMD_QUIT && received == -1)
        showError = FALSE;
      else
        showError = TRUE;

      // only report an error if explicitly wanted
      if(showError == TRUE && errorMsg != NULL)
      {
        // if we just issued a PASS command and that failed, then overwrite the visible
        // password with X chars now, so that nobody else can read your password
        if(command == POPCMD_PASS)
        {
          char *p;

          // find the beginning of the password
          if((p = strstr(tc->pop3Buffer, POPcmd[POPCMD_PASS])) != NULL &&
             (p = strchr(p, ' ')) != NULL)
          {
            // now cross it out
            while(*p != '\0' && *p != ' ' && *p != '\n' && *p != '\r')
              *p++ = 'X';
          }
        }

        ER_NewError(errorMsg, tc->host, tc->msn->description, (char *)POPcmd[command], tc->pop3Buffer);
      }
    }
  }

  RETURN(result);
  return result;
}

///
/// ReceiveToFile
static int ReceiveToFile(struct TransferContext *tc, FILE *fh, const char *filename, const BOOL isTemp)
{
  int count;
  int l=0, read, state=0;
  BOOL error = FALSE;
  BOOL done = FALSE;
  char *lineptr = tc->lineBuffer;

  ENTER();

  // get the first data the pop server returns after the TOP command
  if((read = count = ReceiveFromHost(tc->connection, tc->pop3Buffer, sizeof(tc->pop3Buffer))) <= 0)
    tc->connection->error = CONNECTERR_UNKNOWN_ERROR;

  while(tc->connection->abort == FALSE && tc->connection->error == CONNECTERR_NO_ERROR)
  {
    char *bufptr;

    // now we iterate through the received string
    // and strip out the '\r' character.
    // we iterate through it because the strings we receive
    // from the socket can be splitted somehow.
    for(bufptr = tc->pop3Buffer; read > 0; bufptr++, read--)
    {
      // first we check if our buffer is full and if so we
      // write it to the file.
      if(l == sizeof(tc->lineBuffer) || done == TRUE)
      {
        // update the transfer status during the final download
        if(isTemp == FALSE)
          PushMethodOnStack(tc->transferGroup, 3, MUIM_TransferControlGroup_Update, l, tr(MSG_TR_Downloading));

        // write the line to the file now
        if(fwrite(tc->lineBuffer, 1, l, fh) != (size_t)l)
        {
          error = TRUE;
          ER_NewError(tr(MSG_ER_ErrorWriteMailfile), filename);
          break;
        }

        // if we end up here and done is true we have to break that iteration
        if(done == TRUE)
          break;

        // set l to zero so that the next char gets written to the beginning
        l = 0;
        lineptr = tc->lineBuffer;
      }

      // we have to analyze different states because we iterate through our
      // received buffer char by char.
      switch(state)
      {
        // stat==1 is only reached if a "\r" was found previously
        case 1:
        {
          // if we previously found a \r and the actual char is not a \n, then
          // we write the \r in the buffer and iterate to the next char.
          if(*bufptr != '\n')
          {
            *lineptr++ = '\r';
            l++;
            read++;
            bufptr--;
            state = 0;

            continue;
          }

          // write the actual char "\n" in the line buffer
          *lineptr++ = *bufptr;
          l++;

          state = 2; // we found a "\r\n" so we move to stat 2 on the next iteration.
          continue;
        }
        break;

        // stat==2 is only reached if we previously found a "\r\n"
        // stat==5 if we found a lonely "\n"
        case 2:
        case 5:
        {
          if(*bufptr == '.')
          {
            state++; // now it's 3 or 6
            continue;
          }

          state = 0;
        }
        break;

        // stat==3 is only reached if we previously found a "\r\n."
        // stat==6 if we found a lonely "\n."
        case 3:
        case 6:
        {
          if(state == 3 && *bufptr == '\r')
          {
            state = 4; // we found a \r directly after a "\r\n.", so it can be the start of a TERM
          }
          else if(*bufptr == '.')
          {
            // (RFC 1939) - the server handles "." as "..", so we only write "."
            *lineptr++ = '.';
            l++;
            state = 0;
          }
          else
          {
            // write the actual char in the line buffer
            *lineptr++ = '.';
            l++;
            read++;
            bufptr--;
            state = 0;
          }

          continue;
        }
        break;

        // stat==4 is only reached if we previsouly found a "\r\n.\r"
        case 4:
        {
          if(*bufptr != '\n')
          {
            *lineptr++ = '.';
            l++;
            read += 2;
            bufptr -= 2;
            state = 0;

            continue;
          }

          // so if we end up here we finally found our termination line "\r\n.\r\n"
          // and make sure the buffer is written before we break out here.
          read = 2;
          done = TRUE;

          continue;
        }
        break;
      }

      // if we find a \r we set the stat to 1 and check on the next iteration if
      // the following char is a \n and if so we have to strip it out.
      if(*bufptr == '\r')
      {
        state = 1;
        continue;
      }
      else if(*bufptr == '\n')
      {
        state=5;
      }

      // write the actual char in the line buffer
      *lineptr++ = *bufptr;
      l++;
    }

    // if we received the term octet we can exit the while loop now
    if(done == TRUE || error == TRUE || tc->connection->abort == TRUE || tc->connection->error != CONNECTERR_NO_ERROR)
      break;

    // if not, we get another bunch of data and start over again.
    if((read = ReceiveFromHost(tc->connection, tc->pop3Buffer, sizeof(tc->pop3Buffer))) <= 0)
      break;

    count += read;
  }

  if(done == FALSE || error == TRUE)
    count = 0;

  RETURN(count);
  return count;
}

///
/// GetMessageList
//  Collects messages waiting on a POP3 server
static BOOL GetMessageList(struct TransferContext *tc)
{
  BOOL success;

  ENTER();

  // we issue a LIST command without argument to get a list
  // of all messages available on the server. This command will
  // return TRUE if the server responded with a +OK
  if(SendPOP3Command(tc, POPCMD_LIST, NULL, tr(MSG_ER_BADRESPONSE_POP3)) != NULL)
  {
    success = TRUE;

    // get the first line the pop server returns after the LINE command
    if(ReceiveLineFromHost(tc->connection, tc->pop3Buffer, sizeof(tc->pop3Buffer)) > 0)
    {
      // we get the "scan listing" as long as we haven't received a a
      // finishing octet
      while(tc->connection->error == CONNECTERR_NO_ERROR && strncmp(tc->pop3Buffer, ".\r\n", 3) != 0)
      {
        int serverIndex;
        int size;
        struct Mail *newMail;

        // read the index and size of the first message
        sscanf(tc->pop3Buffer, "%d %d", &serverIndex, &size);

        if(serverIndex > 0 && (newMail = AllocMail()) != NULL)
        {
          int mode;
          struct MailTransferNode *tnode;
          static const int mode2tflags[16] =
          {
            TRF_TRANSFER,
            TRF_TRANSFER,
            TRF_TRANSFER|TRF_DELETE,
            TRF_TRANSFER|TRF_DELETE,
            TRF_TRANSFER,
            TRF_TRANSFER,
            TRF_TRANSFER|TRF_DELETE,
            TRF_TRANSFER|TRF_DELETE,
            TRF_NONE,
            TRF_TRANSFER|TRF_PRESELECT|TRF_SIZE_EXCEEDED,
            TRF_NONE,
            TRF_TRANSFER|TRF_DELETE|TRF_PRESELECT|TRF_SIZE_EXCEEDED,
            TRF_PRESELECT,
            TRF_TRANSFER|TRF_PRESELECT|TRF_SIZE_EXCEEDED,
            TRF_PRESELECT,
            TRF_TRANSFER|TRF_DELETE|TRF_PRESELECT|TRF_SIZE_EXCEEDED
          };
          int tflags;

          newMail->Size  = size;

          mode = (hasServerDownloadLargeMails(tc->msn) == TRUE ? 1 : 0) +
                 (hasServerPurge(tc->msn) == TRUE ? 2 : 0) +
                 (isFlagSet(tc->flags, RECEIVEF_USER) ? 4 : 0) +
                 ((tc->msn->preselection >= PSM_LARGE && tc->msn->largeMailSizeLimit > 0 && newMail->Size >= (tc->msn->largeMailSizeLimit*1024)) ? 8 : 0);
          tflags = mode2tflags[mode];

          // if preselection is configured then force displaying this mail in the list
          if(tc->msn->preselection >= PSM_ALWAYS)
            setFlag(tflags, TRF_PRESELECT);

          D(DBF_NET, "mail transfer mode %ld, tflags %08lx (dl large %ld, purge %ld, user %ld, warnsize %ld, size %ld, presel %ld)", mode, tflags, hasServerDownloadLargeMails(tc->msn), hasServerPurge(tc->msn), isFlagSet(tc->flags, RECEIVEF_USER), tc->msn->largeMailSizeLimit*1024, newMail->Size, tc->msn->preselection);

          // allocate a new MailTransferNode and add it to our
          // new transferlist
          if((tnode = CreateMailTransferNode(NULL, tflags)) != NULL)
          {
            tnode->mail = newMail;
            tnode->index = serverIndex;

            AddMailTransferNode(tc->transferList, tnode);
          }
          else
          {
            FreeMail(newMail);
            success = FALSE;
            break;
          }
        }

        // now read the next Line
        if(ReceiveLineFromHost(tc->connection, tc->pop3Buffer, sizeof(tc->pop3Buffer)) <= 0)
        {
          success = FALSE;
          break;
        }
      }
    }
    else
      success = FALSE;
  }
  else
    success = FALSE;

  RETURN(success);
  return success;
}

///
/// GetSingleMessageDetails
//  Gets header from a message stored on the POP3 server
static void GetSingleMessageDetails(struct TransferContext *tc, struct MailTransferNode *tnode, int lline)
{
  struct Mail *mail = tnode->mail;

  ENTER();

  if(mail->From.Address[0] == '\0' && tc->connection->abort == FALSE && tc->connection->error == CONNECTERR_NO_ERROR)
  {
    char cmdbuf[SIZE_SMALL];

    // we issue a TOP command with a one line message body.
    //
    // This command is optional within the RFC 1939 specification
    // and therefore we don't throw any error
    snprintf(cmdbuf, sizeof(cmdbuf), "%d 1", tnode->index);
    if(SendPOP3Command(tc, POPCMD_TOP, cmdbuf, NULL) != NULL)
    {
      struct TempFile *tf;

      // we generate a temporary file to buffer the TOP list into
      if((tf = OpenTempFile("w")) != NULL)
      {
        struct ExtendedMail *email;
        BOOL done = FALSE;

        // now we call a subfunction to receive data from the POP3 server
        // and write it in the filehandle as long as there is no termination \r\n.\r\n
        if(ReceiveToFile(tc, tf->FP, tf->Filename, TRUE) > 0)
          done = TRUE;

        // close the filehandle now
        fclose(tf->FP);
        tf->FP = NULL;

        // If we end up here because of an error, abort or the upper loop wasn't finished
        // we exit immediatly with deleting the temp file also.
        if(tc->connection->abort == TRUE || tc->connection->error != CONNECTERR_NO_ERROR || done == FALSE)
        {
          lline = -1;
        }
        else if((email = MA_ExamineMail(NULL, FilePart(tf->Filename), TRUE)) != NULL)
        {
          memcpy(&mail->From, &email->Mail.From, sizeof(mail->From));
          memcpy(&mail->To, &email->Mail.To, sizeof(mail->To));
          memcpy(&mail->ReplyTo, &email->Mail.ReplyTo, sizeof(mail->ReplyTo));
          strlcpy(mail->Subject, email->Mail.Subject, sizeof(mail->Subject));
          strlcpy(mail->MailFile, email->Mail.MailFile, sizeof(mail->MailFile));
          memcpy(&mail->Date, &email->Mail.Date, sizeof(mail->Date));

          // if this function was called with -1, then the POP3 server
          // doesn't have the UIDL command and we have to generate our
          // own one by using the MsgID.
          if(lline == -1)
            tnode->uidl = strdup(email->messageID);
          else if(lline == -2 && hasServerApplyRemoteFilters(tc->msn) == TRUE)
            ApplyRemoteFilters(tc->remoteFilters, tnode);

          MA_FreeEMailStruct(email);
        }
        else
          E(DBF_NET, "couldn't examine mail file '%s'", tf->Filename);

        CloseTempFile(tf);
      }
      else
        ER_NewError(tr(MSG_ER_CantCreateTempfile));
    }
  }

  if(lline >= 0 && tc->preselectWindow != NULL)
    PushMethodOnStack(tc->preselectWindow, 2, MUIM_PreselectionWindow_RefreshMail, lline);

  LEAVE();
}

///
/// CheckAbort
// check for an abortion while obtaining the message details
static int CheckAbort(struct TransferContext *tc)
{
  int success = 1;

  ENTER();

  // check for transmission errors
  if(tc->connection->abort == TRUE || tc->connection->error != CONNECTERR_NO_ERROR)
  {
    success = 0;
  }
  else
  {
    ULONG signals;

    // check for pending signals and clear them
    signals = SetSignal(0UL, tc->abortMask|tc->wakeupMask);

    // check for abortion
    if(success == 1 && isFlagSet(signals, tc->abortMask))
    {
      D(DBF_THREAD, "get message details aborted");
      tc->connection->abort = TRUE;
      success = 0;
    }

    // check for an early reaction from the user in the preselection window
    if(success == 1 && isFlagSet(signals, tc->wakeupMask))
    {
      ULONG result = FALSE;

      PushMethodOnStackWait(tc->preselectWindow, 3, OM_GET, MUIA_PreselectionWindow_Result, &result);
      if(result == TRUE)
        success = 2;
      else
        success = 0;
    }
  }

  RETURN(success);
  return success;
}

///
/// GetAllMessageDetails
// get the details of all mails
// success will be:
// 0: transmission failed or preselection was aborted
// 1: all details have been obtained, wait for the user to finish the preselection
// 2: preselection was aborted early by pressing "Start"
static int GetAllMessageDetails(struct TransferContext *tc)
{
  int success = 1;
  struct MailTransferNode *tnode;
  LONG line;

  ENTER();

  // start with the first node in the list
  tnode = FirstMailTransferNode(tc->transferList);
  line = 0;

  if(tc->firstToPreselect > 0)
  {
    // the first mail to be transferred is not the first in the list,
    // skip until that index
    do
    {
      tnode = NextMailTransferNode(tnode);
      line++;
      if(line == tc->firstToPreselect)
        break;
    }
    while(tnode != NULL);
  }

  // tell the preselection window to highlight the first mail to be transferred
  PushMethodOnStackWait(tc->preselectWindow, 3, MUIM_Set, MUIA_PreselectionWindow_ActiveMail, tc->firstToPreselect);

  // get all message details until the end of the list
  while(tnode != NULL)
  {
    GetSingleMessageDetails(tc, tnode, line);

    tnode = NextMailTransferNode(tnode);
    line++;

    if((success = CheckAbort(tc)) != 1)
      break;
  }

  // get any remaining message details from the beginning of the list if the
  // transfer has not yet been started or aborted
  if(success == 1 && tc->firstToPreselect > 0)
  {
    tnode = FirstMailTransferNode(tc->transferList);
    line = 0;

    while(tnode != NULL)
    {
      GetSingleMessageDetails(tc, tnode, line);

      tnode = NextMailTransferNode(tnode);
      line++;
      if(line == tc->firstToPreselect)
        break;

      if((success = CheckAbort(tc)) != 1)
        break;
    }
  }

  RETURN(success);
  return success;
}

///
/// FilterDuplicates
//
static BOOL FilterDuplicates(struct TransferContext *tc)
{
  BOOL result = FALSE;

  ENTER();

  // check if there is anything to transfer at all
  if(IsMailTransferListEmpty(tc->transferList) == FALSE)
  {
    // inform the user of the operation
    PushMethodOnStack(tc->transferGroup, 2, MUIM_TransferControlGroup_ShowStatus, tr(MSG_TR_CHECKUIDL));

    // before we go and request the UIDL of each message we check whether the server
    // supports the UIDL command at all
    if(SendPOP3Command(tc, POPCMD_UIDL, NULL, NULL) != NULL)
    {
      // get the first line the pop server returns after the UIDL command
      if(ReceiveLineFromHost(tc->connection, tc->lineBuffer, sizeof(tc->lineBuffer)) > 0)
      {
        // we get the "unique-id list" as long as we haven't received a
        // finishing octet
        while(tc->connection->abort == FALSE && tc->connection->error == CONNECTERR_NO_ERROR && strncmp(tc->lineBuffer, ".\r\n", 3) != 0)
        {
          char *p;

          // now parse the line and get the message number and UIDL
          // each UIDL entry is transmitted as "number uidl"
          if((p = strchr(tc->lineBuffer, ' ')) != NULL)
          {
            int num;
            char *uidl;
            struct MailTransferNode *tnode;

            // replace the space by a NUL byte and convert the first part to an integer
            *p++ = '\0';
            num = atoi(tc->lineBuffer);
            // strip the trailing CR+LF
            uidl = p;
            if((p = strchr(uidl, '\r')) != NULL)
              *p = '\0';

            // search through our transferList
            ForEachMailTransferNode(tc->transferList, tnode)
            {
              if(tnode->index == num)
              {
                if((tnode->uidl = strdup(uidl)) != NULL)
                {
                  struct UIDLtoken *token;

                  // check if this UIDL is known already
                  if((token = FindUIDL(tc->UIDLhashTable, tnode->uidl)) != NULL)
                  {
                    D(DBF_UIDL, "mail %ld: found UIDL '%s', flags=%08lx", tnode->index, tnode->uidl, token->flags);

                    // check if we knew this UIDL before
                    if(isFlagSet(token->flags, UIDLF_OLD))
                    {
                      // make sure the mail is flagged as being ignoreable
                      tc->downloadResult.dupeSkipped++;
                      // don't download this mail, because it has been downloaded before
                      clearFlag(tnode->tflags, TRF_TRANSFER);
                      // exclude this mail from preselection for the same reason
                      clearFlag(tnode->tflags, TRF_PRESELECT);
                      // mark this UIDL as old+new, thus it will be saved upon cleanup
                      setFlag(token->flags, UIDLF_NEW);
                    }
                  }
                }

                break;
              }
            }
          }

          if(tc->connection->abort == TRUE || tc->connection->error != CONNECTERR_NO_ERROR)
            break;

          // now read the next Line
          if(ReceiveLineFromHost(tc->connection, tc->lineBuffer, sizeof(tc->lineBuffer)) <= 0)
          {
            E(DBF_UIDL, "unexpected end of data stream during UIDL.");
            break;
          }
        }
      }
      else
        E(DBF_UIDL, "error on first readline!");
    }
    else
    {
      struct MailTransferNode *tnode;

      W(DBF_UIDL, "POP3 server '%s' doesn't support UIDL command!", tc->msn->hostname);

      // search through our transferList
      ForEachMailTransferNode(tc->transferList, tnode)
      {
        // if the server doesn't support the UIDL command we
        // use the TOP command and generate our own UIDL within
        // the GetMessageDetails function
        GetSingleMessageDetails(tc, tnode, -1);

        // now that we should successfully obtained the UIDL of the
        // mailtransfernode we go and check if that UIDL is already in our UIDLhash
        // and if so we go and flag the mail as a mail that should not be downloaded
        // automatically
        if(tnode->uidl != NULL)
        {
          struct UIDLtoken *token;

          if((token = AddUIDLtoHash(tc->UIDLhashTable, tnode->uidl, UIDLF_NEW)) != NULL)
          {
            D(DBF_UIDL, "mail %ld: found UIDL '%s', flags=%08lx", tnode->index, tnode->uidl, token->flags);

            // check if we knew this UIDL before
            if(isFlagSet(token->flags, UIDLF_OLD))
            {
              tc->downloadResult.dupeSkipped++;
              // don't download this mail, because it has been downloaded before
              clearFlag(tnode->tflags, TRF_TRANSFER);
              // mark this UIDL as old+new, thus it will be saved upon cleanup
              setFlag(token->flags, UIDLF_NEW);
            }
          }
        }

        if(tc->connection->abort == TRUE || tc->connection->error != CONNECTERR_NO_ERROR)
          break;
      }
    }

    result = (tc->connection->abort == FALSE && tc->connection->error == CONNECTERR_NO_ERROR);
  }
  else
    result = TRUE;

  RETURN(result);
  return result;
}

///
/// ConnectToPOP3
//  Connects to a POP3 mail server
static int ConnectToPOP3(struct TransferContext *tc)
{
  char *p;
  char *welcomemsg = NULL;
  int msgs = -1;
  char *resp;
  enum ConnectError err;

  ENTER();

  D(DBF_NET, "connect to POP3 server '%s'", tc->msn->hostname);

  PushMethodOnStack(tc->transferGroup, 2, MUIM_TransferControlGroup_ShowStatus, tr(MSG_TR_Connecting));

  BusyText(tr(MSG_TR_MailTransferFrom), tc->msn->description);

  // now we start our connection to the POP3 server
  if((err = ConnectToHost(tc->connection, tc->host, tc->port)) != CONNECTERR_SUCCESS)
  {
    if(isFlagSet(tc->flags, RECEIVEF_USER))
    {
      switch(err)
      {
        case CONNECTERR_SUCCESS:
        case CONNECTERR_ABORTED:
        case CONNECTERR_NO_ERROR:
          // do nothing
        break;

        // socket is already in use
        case CONNECTERR_SOCKET_IN_USE:
          ER_NewError(tr(MSG_ER_CONNECTERR_SOCKET_IN_USE_POP3), tc->host, tc->msn->description);
        break;

        // socket() execution failed
        case CONNECTERR_NO_SOCKET:
          ER_NewError(tr(MSG_ER_CONNECTERR_NO_SOCKET_POP3), tc->host, tc->msn->description);
        break;

        // couldn't establish non-blocking IO
        case CONNECTERR_NO_NONBLOCKIO:
          ER_NewError(tr(MSG_ER_CONNECTERR_NO_NONBLOCKIO_POP3), tc->host, tc->msn->description);
        break;

        // connection request timed out
        case CONNECTERR_TIMEDOUT:
          ER_NewError(tr(MSG_ER_CONNECTERR_TIMEDOUT_POP3), tc->host, tc->msn->description);
        break;

        // unknown host - gethostbyname() failed
        case CONNECTERR_UNKNOWN_HOST:
          ER_NewError(tr(MSG_ER_UNKNOWN_HOST_POP3), tc->host, tc->msn->description);
        break;

        // general connection error
        case CONNECTERR_UNKNOWN_ERROR:
          ER_NewError(tr(MSG_ER_CANNOT_CONNECT_POP3), tc->host, tc->msn->description);
        break;

        case CONNECTERR_SSLFAILED:
        case CONNECTERR_INVALID8BIT:
        case CONNECTERR_NO_CONNECTION:
        case CONNECTERR_NOT_CONNECTED:
          // can't occur, do nothing
        break;
      }
    }

    goto out;
  }

  // update the AppIcon now that the connection was established
  PushMethodOnStack(G->App, 1, MUIM_YAMApplication_UpdateAppIcon);

  // If this connection should be a STLS like connection we have to get the welcome
  // message now and then send the STLS command to start TLS negotiation
  if(hasServerTLS(tc->msn))
  {
    PushMethodOnStack(tc->transferGroup, 2, MUIM_TransferControlGroup_ShowStatus, tr(MSG_TR_WaitWelcome));

    // Initiate a connect and see if we succeed
    if((resp = SendPOP3Command(tc, POPCMD_CONNECT, NULL, tr(MSG_ER_POP3WELCOME))) == NULL)
      goto out;

    welcomemsg = StrBufCpy(NULL, resp);

    // If the user selected STLS support we have to first send the command
    // to start TLS negotiation (RFC 2595)
    if(SendPOP3Command(tc, POPCMD_STLS, NULL, tr(MSG_ER_BADRESPONSE_POP3)) == NULL)
      goto out;
  }

  // Here start the TLS/SSL Connection stuff
  if(hasServerSSL(tc->msn) || hasServerTLS(tc->msn))
  {
    PushMethodOnStack(tc->transferGroup, 2, MUIM_TransferControlGroup_ShowStatus, tr(MSG_TR_INITTLS));

    // Now we have to Initialize and Start the TLS stuff if requested
    if(MakeSecureConnection(tc->connection) == TRUE)
      tc->useTLS = TRUE;
    else
    {
      ER_NewError(tr(MSG_ER_INITTLS_POP3), tc->host, tc->msn->description);
      goto out;
    }
  }

  // If this was a connection on a stunnel on port 995 or a non-ssl connection
  // we have to get the welcome message now
  if(hasServerSSL(tc->msn) == TRUE || hasServerTLS(tc->msn) == FALSE)
  {
    // Initiate a connect and see if we succeed
    if((resp = SendPOP3Command(tc, POPCMD_CONNECT, NULL, tr(MSG_ER_POP3WELCOME))) == NULL)
      goto out;

    welcomemsg = StrBufCpy(NULL, resp);
  }

  if(tc->password[0] == '\0')
  {
    Object *passwordWin;

    snprintf(tc->windowTitle, sizeof(tc->windowTitle), tr(MSG_TR_ENTER_POP3_PASSWORD), tc->msn->description);

    if((passwordWin = (Object *)PushMethodOnStackWait(G->App, 5, MUIM_YAMApplication_CreatePasswordWindow, CurrentThread(), tr(MSG_TR_PopLogin), tc->windowTitle, sizeof(tc->password))) != NULL)
    {
      PushMethodOnStack(tc->transferGroup, 2, MUIM_TransferControlGroup_ShowStatus, tr(MSG_TR_WAIT_FOR_PASSWORD));

      if(SleepThread() == TRUE)
      {
        ULONG result = 0;

        PushMethodOnStackWait(passwordWin, 3, OM_GET, MUIA_StringRequestWindow_Result, &result);
        if(result != 0)
          PushMethodOnStackWait(passwordWin, 3, OM_GET, MUIA_StringRequestWindow_StringContents, tc->password);
      }
      else
      {
        // force "no password" if we were aborted
        tc->password[0] = '\0';
      }

      PushMethodOnStack(G->App, 2, MUIM_YAMApplication_DisposeWindow, passwordWin);
    }

    // bail out if we still got no password
    if(tc->password[0] == '\0')
      goto out;
  }

  // if the user has selected APOP for that POP3 host
  // we have to process it now
  if(hasServerAPOP(tc->msn))
  {
    // Now we get the APOP Identifier out of the welcome
    // message
    if((p = strchr(welcomemsg, '<')) != NULL)
    {
      struct MD5Context context;
      unsigned char digest[16];
      char digestHex[33];

      strlcpy(tc->lineBuffer, p, sizeof(tc->lineBuffer));
      if((p = strchr(tc->lineBuffer, '>')) != NULL)
        p[1] = '\0';

      // then we send the APOP command to authenticate via APOP
      strlcat(tc->lineBuffer, tc->msn->password, sizeof(tc->lineBuffer));
      md5init(&context);
      md5update(&context, tc->lineBuffer, strlen(tc->lineBuffer));
      md5final(digest, &context);
      md5digestToHex(digest, digestHex);
      snprintf(tc->lineBuffer, sizeof(tc->lineBuffer), "%s %s", tc->msn->username, digestHex);
      PushMethodOnStack(tc->transferGroup, 2, MUIM_TransferControlGroup_ShowStatus, tr(MSG_TR_SendAPOPLogin));
      if(SendPOP3Command(tc, POPCMD_APOP, tc->lineBuffer, tr(MSG_ER_BADRESPONSE_POP3)) == NULL)
        goto out;
    }
    else
    {
      ER_NewError(tr(MSG_ER_NO_APOP), tc->host, tc->msn->description);
      goto out;
    }
  }
  else
  {
    PushMethodOnStack(tc->transferGroup, 2, MUIM_TransferControlGroup_ShowStatus, tr(MSG_TR_SendUserID));
    if(SendPOP3Command(tc, POPCMD_USER, tc->msn->username, tr(MSG_ER_BADRESPONSE_POP3)) == NULL)
      goto out;

    PushMethodOnStack(tc->transferGroup, 2, MUIM_TransferControlGroup_ShowStatus, tr(MSG_TR_SendPassword));
    if(SendPOP3Command(tc, POPCMD_PASS, tc->password, tr(MSG_ER_BADRESPONSE_POP3)) == NULL)
      goto out;
  }

  PushMethodOnStack(tc->transferGroup, 2, MUIM_TransferControlGroup_ShowStatus, tr(MSG_TR_GetStats));
  if((resp = SendPOP3Command(tc, POPCMD_STAT, NULL, tr(MSG_ER_BADRESPONSE_POP3))) == NULL)
    goto out;

  sscanf(&resp[4], "%d", &msgs);
  if(msgs != 0)
    AppendToLogfile(LF_VERBOSE, 31, tr(MSG_LOG_CONNECT_POP3), tc->msn->username, tc->host, msgs);

out:

  FreeStrBuf(welcomemsg);

  RETURN(msgs);
  return msgs;
}

///
/// DisconnectFromPOP3
static void DisconnectFromPOP3(struct TransferContext *tc)
{
  ENTER();

  D(DBF_NET, "disconnecting from POP3 server '%s'", tc->msn->hostname);
  PushMethodOnStack(tc->transferGroup, 2, MUIM_TransferControlGroup_ShowStatus, tr(MSG_TR_Disconnecting));
  if(tc->connection->error == CONNECTERR_NO_ERROR)
    SendPOP3Command(tc, POPCMD_QUIT, NULL, tr(MSG_ER_BADRESPONSE_POP3));

  DisconnectFromHost(tc->connection);

  BusyEnd();

  LEAVE();
}

///
/// LoadMessage
static BOOL LoadMessage(struct TransferContext *tc, struct Folder *inFolder, const int number)
{
  BOOL result = FALSE;
  char msgfile[SIZE_PATHFILE];
  FILE *fh;

  ENTER();

  MA_NewMailFile(inFolder, msgfile, sizeof(msgfile));

  // open the new mailfile for writing out the retrieved
  // data
  if((fh = fopen(msgfile, "w")) != NULL)
  {
    char msgnum[SIZE_SMALL];
    BOOL done = FALSE;

    setvbuf(fh, NULL, _IOFBF, SIZE_FILEBUF);

    snprintf(msgnum, sizeof(msgnum), "%d", number);
    if(SendPOP3Command(tc, POPCMD_RETR, msgnum, tr(MSG_ER_BADRESPONSE_POP3)) != NULL)
    {
      // now we call a subfunction to receive data from the POP3 server
      // and write it in the filehandle as long as there is no termination \r\n.\r\n
      if(ReceiveToFile(tc, fh, msgfile, FALSE) > 0)
        done = TRUE;
    }
    fclose(fh);

    if(tc->connection->abort == FALSE && tc->connection->error == CONNECTERR_NO_ERROR && done == TRUE)
    {
      struct ExtendedMail *email;

      if((email = MA_ExamineMail(inFolder, FilePart(msgfile), FALSE)) != NULL)
      {
        struct Mail *mail;

        if((mail = AddMailToFolder(&email->Mail, inFolder)) != NULL)
        {
          // we have to get the actual Time and place it in the transDate, so that we know at
          // which time this mail arrived
          GetSysTimeUTC(&mail->transDate);

          mail->sflags = SFLAG_NEW;
          MA_UpdateMailFile(mail);

          D(DBF_THREAD, "adding mail to downloaded list");
          // add the mail to the list of downloaded mails
          LockMailList(tc->msn->downloadedMails);
          AddNewMailNode(tc->msn->downloadedMails, mail);
          UnlockMailList(tc->msn->downloadedMails);

          // if the current folder is the inbox we can go and add the mail instantly to the maillist
          if(inFolder == GetCurrentFolder())
            PushMethodOnStack(G->MA->GUI.PG_MAILLIST, 3, MUIM_NList_InsertSingle, mail, MUIV_NList_Insert_Sorted);

          AppendToLogfile(LF_VERBOSE, 32, tr(MSG_LOG_RetrievingVerbose), AddrName(mail->From), mail->Subject, mail->Size);

          PushMethodOnStackWait(G->App, 3, MUIM_YAMApplication_StartMacro, MACRO_NEWMSG, msgfile);

          result = TRUE;
        }

        MA_FreeEMailStruct(email);
      }
    }

    if(result == FALSE)
    {
      DeleteFile(msgfile);

      // we need to set the folder flags to modified so that the .index will be saved later.
      setFlag(inFolder->Flags, FOFL_MODIFY);
    }
  }
  else
    ER_NewError(tr(MSG_ER_ErrorWriteMailfile), msgfile);

  RETURN(result);
  return result;
}

///
/// DeleteMessage
static BOOL DeleteMessage(struct TransferContext *tc, const int number)
{
  BOOL result = FALSE;
  char msgnum[SIZE_SMALL];

  ENTER();

  snprintf(msgnum, sizeof(msgnum), "%d", number);

  // update the transfer status
  PushMethodOnStack(tc->transferGroup, 3, MUIM_TransferControlGroup_Update, TCG_SETMAX, tr(MSG_TR_DeletingServerMail));

  if(SendPOP3Command(tc, POPCMD_DELE, msgnum, tr(MSG_ER_BADRESPONSE_POP3)) != NULL)
  {
    tc->downloadResult.deleted++;
    result = TRUE;
  }

  RETURN(result);
  return result;
}

///
/// DownloadMails
static void DownloadMails(struct TransferContext *tc)
{
  struct Folder *inFolder = FO_GetFolderByType(FT_INCOMING, NULL);
  struct MailTransferNode *tnode;

  ENTER();

  GetSysTime(TIMEVAL(&tc->lastUpdateTime));

  ForEachMailTransferNode(tc->transferList, tnode)
  {
    struct Mail *mail = tnode->mail;

    D(DBF_NET, "download flags %08lx=%s%s%s for mail with subject '%s' and size %ld", tnode->tflags, isFlagSet(tnode->tflags, TRF_TRANSFER) ? "TR_TRANSFER " : "" , isFlagSet(tnode->tflags, TRF_DELETE) ? "TR_DELETE " : "", isFlagSet(tnode->tflags, TRF_PRESELECT) ? "TR_PRESELECT " : "", mail->Subject, mail->Size);
    if(isFlagSet(tnode->tflags, TRF_TRANSFER))
    {
      D(DBF_NET, "downloading mail with subject '%s' and size %ld", mail->Subject, mail->Size);

      // update the transfer status
      PushMethodOnStack(tc->transferGroup, 5, MUIM_TransferControlGroup_Next, tnode->index, tnode->position, mail->Size, tr(MSG_TR_Downloading));

      if(LoadMessage(tc, inFolder, tnode->index) == TRUE)
      {
        if(TimeHasElapsed(&tc->lastUpdateTime, 250000) == TRUE)
        {
          // redraw the folderentry in the listtree 4 times per second at most
          PushMethodOnStack(G->MA->GUI.NL_FOLDERS, 3, MUIM_NListtree_Redraw, inFolder->Treenode, MUIF_NONE);
        }

        // put the transferStat for this mail to 100%
        PushMethodOnStack(tc->transferGroup, 3, MUIM_TransferControlGroup_Update, TCG_SETMAX, tr(MSG_TR_Downloading));

        tc->downloadResult.downloaded++;

        // Remember the UIDL of this mail, no matter if it is going
        // to be deleted or not. Some servers don't delete a mail
        // right after the DELETE command, but only after a successful
        // QUIT command. Personal experience shows that pop.gmx.de is
        // one of these servers.
        if(hasServerAvoidDuplicates(tc->msn) == TRUE)
        {
          D(DBF_NET, "adding mail with subject '%s' to UIDL hash", mail->Subject);
          // add the UIDL to the hash table or update an existing entry
          AddUIDLtoHash(tc->UIDLhashTable, tnode->uidl, UIDLF_NEW);
        }

        if(isFlagSet(tnode->tflags, TRF_DELETE))
        {
          D(DBF_NET, "deleting mail with subject '%s' on server", mail->Subject);

          DeleteMessage(tc, tnode->index);
        }
        else
          D(DBF_NET, "leaving mail with subject '%s' and size %ld on server to be downloaded again", mail->Subject, mail->Size);
      }
    }
    else if(isFlagSet(tnode->tflags, TRF_DELETE))
    {
      D(DBF_NET, "deleting mail with subject '%s' on server", mail->Subject);

      // now we "know" that this mail had existed, don't forget this in case
      // the delete operation fails
      if(hasServerAvoidDuplicates(tc->msn) == TRUE)
      {
        D(DBF_NET, "adding mail with subject '%s' to UIDL hash", mail->Subject);
        // add the UIDL to the hash table or update an existing entry
        AddUIDLtoHash(tc->UIDLhashTable, tnode->uidl, UIDLF_NEW);
      }

      DeleteMessage(tc, tnode->index);
    }
    else
    {
      D(DBF_NET, "leaving mail with subject '%s' and size %ld on server to be downloaded again", mail->Subject, mail->Size);
      // Do not modify the UIDL hash here!
      // The mail was marked as "don't download", but here we don't know if that
      // is due to the duplicates checking or if the user did that himself.
    }

    if(tc->connection->abort == TRUE || tc->connection->error != CONNECTERR_NO_ERROR)
      break;
  }

  PushMethodOnStack(tc->transferGroup, 1, MUIM_TransferControlGroup_Finish);

  // update the stats
  PushMethodOnStack(G->App, 3, MUIM_YAMApplication_DisplayStatistics, inFolder, TRUE);

  // update the menu items and toolbars
  PushMethodOnStack(G->App, 2, MUIM_YAMApplication_ChangeSelected, inFolder, TRUE);

  LEAVE();
}

///
/// WaitForPreselection
// wait for the user to finish the preselection
static BOOL WaitForPreselection(struct TransferContext *tc)
{
  BOOL success = FALSE;

  ENTER();

  // start the "keep alive" timer
  StartThreadTimer(C->KeepAliveInterval, 0);

  while(TRUE)
  {
    ULONG signals;

    signals = Wait(tc->abortMask|tc->wakeupMask|tc->timerMask);
    if(isFlagSet(signals, tc->abortMask))
    {
      // we were aborted
      D(DBF_THREAD, "preselection aborted");
      tc->connection->abort = TRUE;
      break;
    }
    if(isFlagSet(signals, tc->wakeupMask))
    {
      // the user finished the preselection
      ULONG result = FALSE;

      PushMethodOnStackWait(tc->preselectWindow, 3, OM_GET, MUIA_PreselectionWindow_Result, &result);
      if(result == TRUE)
        success = TRUE;

      break;
    }
    if(isFlagSet(signals, tc->timerMask))
    {
      // here we send a STAT command instead of a NOOP which normally
      // should do the job as well. But there are several known POP3
      // servers out there which are known to ignore the NOOP commands
      // for keepalive message, so STAT should be the better choice.
      if(SendPOP3Command(tc, POPCMD_STAT, NULL, tr(MSG_ER_BADRESPONSE_POP3)) != NULL)
      {
        // restart the "keep alive" timer
        StartThreadTimer(C->KeepAliveInterval, 0);
      }
      else
      {
        // bail out if sending the "keep alive" failed
        break;
      }
    }
  }

  StopThreadTimer();

  RETURN(success);
  return success;
}

///
/// SumUpMails
static void SumUpMails(struct TransferContext *tc)
{
  struct MailTransferNode *tnode;

  ENTER();

  tc->numberOfMails = 0;
  tc->totalSize = 0;

  // search through our transferList
  ForEachMailTransferNode(tc->transferList, tnode)
  {
    tc->numberOfMails++;
    if(isFlagSet(tnode->tflags, TRF_TRANSFER))
    {
      tc->totalSize += tnode->mail->Size;
    }
  }

  LEAVE();
}

///
/// ReceiveMails
BOOL ReceiveMails(struct MailServerNode *msn, const ULONG flags, struct DownloadResult *dlResult)
{
  BOOL success = FALSE;
  struct TransferContext *tc;

  ENTER();

  // make sure the mail server node does not vanish
  ObtainSemaphoreShared(G->configSemaphore);

  if((tc = calloc(1, sizeof(*tc))) != NULL)
  {
    tc->msn = msn;
    tc->useTLS = FALSE;
    tc->flags = flags;
    // assume an error at first
    tc->downloadResult.error = TRUE;
    tc->abortMask = (1UL << ThreadAbortSignal());
    tc->wakeupMask = (1UL << ThreadWakeupSignal());

    // try to open the TCP/IP stack
    if((tc->connection = CreateConnection()) != NULL && ConnectionIsOnline(tc->connection) == TRUE)
    {
      if((tc->transferList = CreateMailTransferList()) != NULL)
      {
        if(hasServerApplyRemoteFilters(tc->msn) == FALSE || (tc->remoteFilters = CloneFilterList(APPLY_REMOTE)) != NULL)
        {
          if(InitThreadTimer() == TRUE)
          {
            BOOL uidlOk = FALSE;

            tc->timerMask = (1UL << ThreadTimerSignal());

            if(hasServerAvoidDuplicates(tc->msn) == TRUE)
            {
              if((tc->UIDLhashTable = InitUIDLhash(tc->msn)) != NULL)
                uidlOk = TRUE;
              else
                ER_NewError("Failed to init UIDL hash");
            }
            else
              uidlOk = TRUE;

            if(uidlOk == TRUE)
            {
              char *p;

              strlcpy(tc->host, msn->hostname, sizeof(tc->host));

              // If the hostname has a explicit :xxxxx port statement at the end we
              // take this one, even if its not needed anymore.
              if((p = strchr(tc->host, ':')) != NULL)
              {
                *p = '\0';
                tc->port = atoi(++p);
              }
              else
                tc->port = msn->port;

              strlcpy(tc->password, msn->password, sizeof(tc->password));

              snprintf(tc->transferGroupTitle, sizeof(tc->transferGroupTitle), tr(MSG_TR_MailTransferFrom), msn->description);

              if((tc->transferGroup = (Object *)PushMethodOnStackWait(G->App, 6, MUIM_YAMApplication_CreateTransferGroup, CurrentThread(), tc->transferGroupTitle, tc->connection, TRUE, isFlagSet(tc->flags, RECEIVEF_USER))) != NULL)
              {
                int msgs;

                if((msgs = ConnectToPOP3(tc)) != -1)
                {
                  // connection succeeded
                  success = TRUE;

                  // but we continue only if there is something to be downloaded at all
                  if(msgs > 0)
                  {
                    // there are messages on the server
                    if(GetMessageList(tc) == TRUE)
                    {
                      BOOL doDownload;

                      // apply possible remote filters
                      if(hasServerApplyRemoteFilters(tc->msn) == TRUE && IsMinListEmpty(tc->remoteFilters) == FALSE)
                      {
                        struct MailTransferNode *tnode;

                        PushMethodOnStack(tc->transferGroup, 2, MUIM_TransferControlGroup_ShowStatus, tr(MSG_TR_ApplyFilters));

                        ForEachMailTransferNode(tc->transferList, tnode)
                          GetSingleMessageDetails(tc, tnode, -2);
                      }

                      // if the user wants to avoid to receive the same message from the
                      // POP3 server again we have to analyze the UIDL of it
                      if(hasServerAvoidDuplicates(tc->msn) == TRUE)
                        FilterDuplicates(tc);

                      // check the list of mails if some kind of preselection is required
                      if(isFlagSet(tc->flags, RECEIVEF_USER) && ScanMailTransferList(tc->transferList, TRF_PRESELECT, TRUE, NULL) == TRUE)
                      {
                        // show the preselection window in case user interaction is requested
                        D(DBF_NET, "preselection is required");
                        doDownload = FALSE;

                        snprintf(tc->windowTitle, sizeof(tc->windowTitle), tr(MSG_TR_MailTransferFrom), tc->msn->description);

                        if((tc->preselectWindow = (Object *)PushMethodOnStackWait(G->App, 5, MUIM_YAMApplication_CreatePreselectionWindow, CurrentThread(), tc->msn->largeMailSizeLimit, tc->windowTitle, PRESELWINMODE_DOWNLOAD, tc->transferList)) != NULL)
                        {
                          int mustWait;

                          PushMethodOnStack(tc->transferGroup, 2, MUIM_TransferControlGroup_ShowStatus, tr(MSG_TR_WAIT_FOR_PRESELECTION));

                          if(ThreadWasAborted() == FALSE)
                          {
                            // scan the list for the first mail to be transferred
                            tc->firstToPreselect = -1;
                            // first try to find a to be downloaded mail that exceeds
                            // the automatic download size limitation
                            ScanMailTransferList(tc->transferList, TRF_TRANSFER|TRF_SIZE_EXCEEDED, TRUE, &tc->firstToPreselect);
                            // then fall back to the first mail to be preselected
                            if(tc->firstToPreselect < 0)
                              ScanMailTransferList(tc->transferList, TRF_TRANSFER, TRUE, &tc->firstToPreselect);

                            if((mustWait = GetAllMessageDetails(tc)) != 0)
                            {
                              if(mustWait == 1)
                              {
                                // we got all details, now wait for the user to finish the preselection
                                doDownload = WaitForPreselection(tc);
                              }
                              else
                              {
                                // getting the details has been aborted early by pressing the "Start" button
                                doDownload = TRUE;
                              }
                            }
                            else
                              D(DBF_NET, "getting message details failed/was aborted, no preselection");

                            PushMethodOnStack(G->App, 2, MUIM_YAMApplication_DisposeWindow, tc->preselectWindow);
                          }
                        }
                      }
                      else
                      {
                        D(DBF_NET, "no preselection required");
                        doDownload = TRUE;
                      }

                      // is there anything left to transfer or delete?
                      if(ThreadWasAborted() == FALSE && doDownload == TRUE && ScanMailTransferList(tc->transferList, TRF_TRANSFER|TRF_DELETE, FALSE, NULL) == TRUE)
                      {
                        SumUpMails(tc);
                        PushMethodOnStack(tc->transferGroup, 3, MUIM_TransferControlGroup_Start, tc->numberOfMails, tc->totalSize);

                        DownloadMails(tc);

                        PushMethodOnStack(tc->transferGroup, 1, MUIM_TransferControlGroup_Finish);

                        if(tc->connection->abort == FALSE && tc->connection->error == CONNECTERR_NO_ERROR)
                          tc->downloadResult.error = FALSE;
                      }
                      else
                        W(DBF_NET, "no mails to be transferred");
                    }
                    else
                      E(DBF_NET, "couldn't retrieve MessageList");
                  }
                  else
                    W(DBF_NET, "no messages found on server '%s'", tc->msn->hostname);
                }

                // disconnect no matter if the connect operation succeeded or not
                DisconnectFromPOP3(tc);

                PushMethodOnStack(G->App, 2, MUIM_YAMApplication_DeleteTransferGroup, tc->transferGroup);
              }

              CleanupUIDLhash(tc->UIDLhashTable);
            }

            CleanupThreadTimer();
          }

          if(tc->remoteFilters != NULL)
            DeleteFilterList(tc->remoteFilters);
        }
        else
          E(DBF_THREAD, "could not clone remote filters");

        // perform the finalizing actions only if we haven't been aborted externally
        if(ThreadWasAborted() == FALSE)
        {
          char downloadedStr[SIZE_SMALL];

          snprintf(downloadedStr, sizeof(downloadedStr), "%d", (int)tc->downloadResult.downloaded);
          PushMethodOnStackWait(G->App, 3, MUIM_YAMApplication_StartMacro, MACRO_POSTGET, downloadedStr);

          AppendToLogfile(LF_ALL, 30, tr(MSG_LOG_RETRIEVED_POP3), tc->downloadResult.downloaded, msn->description);

          // we only apply the filters if we downloaded something, or it's wasted
          D(DBF_THREAD, "filter %ld downloaded mails", tc->downloadResult.downloaded);
          if(tc->downloadResult.downloaded > 0)
          {
            PushMethodOnStackWait(G->App, 3, MUIM_YAMApplication_FilterNewMails, tc->msn->downloadedMails, &tc->filterResult);
            PushMethodOnStackWait(G->App, 4, MUIM_YAMApplication_NewMailAlert, tc->msn->description, &tc->downloadResult, &tc->filterResult, tc->flags);
          }
          else
          {
            PushMethodOnStack(G->App, 1, MUIM_YAMApplication_UpdateAppIcon);
          }

          // forget about the downloaded mails again, but don't free the mails
          LockMailList(tc->msn->downloadedMails);
          ClearMailList(tc->msn->downloadedMails, FALSE);
          UnlockMailList(tc->msn->downloadedMails);
        }
        else
        {
          // update the AppIcon to get rid of the "transferring" icon
          PushMethodOnStack(G->App, 1, MUIM_YAMApplication_UpdateAppIcon);
          // signal failure
          success = FALSE;
        }

        // clean up the transfer list
        DeleteMailTransferList(tc->transferList);
      }
    }

    DeleteConnection(tc->connection);

    // finally copy the download stats of this operation if someone is interested in them
    if(dlResult != NULL)
      memcpy(dlResult, &tc->downloadResult, sizeof(*dlResult));

    free(tc);
  }

  // mark the server as being no longer "in use"
  clearFlag(msn->flags, MSF_IN_USE);

  // now we are done
  ReleaseSemaphore(G->configSemaphore);

  // wake up the calling thread if this is requested
  if(isFlagSet(tc->flags, RECEIVEF_SIGNAL))
    WakeupThread(NULL);

  RETURN(success);
  return success;
}

///
