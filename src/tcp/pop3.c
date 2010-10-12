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

#include <mui/NList_mcc.h>

#include <clib/alib_protos.h>
#include <proto/dos.h>
#include <proto/exec.h>
#include <proto/utility.h>

#include "YAM.h"
#include "YAM_addressbookEntry.h"
#include "YAM_config.h"
#include "YAM_error.h"
#include "YAM_find.h"
#include "YAM_mainFolder.h"
#include "YAM_stringsizes.h"
#include "YAM_transfer.h"

#include "AppIcon.h"
#include "MailList.h"
#include "MailServers.h"
#include "MailTransferList.h"
#include "MUIObjects.h"
#include "Locale.h"
#include "Requesters.h"
#include "UIDL.h"

#include "mime/md5.h"
#include "mui/Classes.h"
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
// static function prototypes
static BOOL FilterDuplicates(void);

/**************************************************************************/
// local macros & defines

/***************************************************************************
 Module: POP3 mail transfer
***************************************************************************/
/// TR_RecvToFile
// function that receives data from a POP3 server until it receives a \r\n.\r\n termination
// line. It automatically writes that data to the supplied filehandle and if present also
// updates the Transfer status
static int TR_RecvToFile(FILE *fh, const char *filename)
{
  int l=0, read, state=0, count;
  char buf[SIZE_LINE];
  char line[SIZE_LINE];
  BOOL done=FALSE;
  char *lineptr = line;

  ENTER();

  // get the first data the pop server returns after the TOP command
  if((read = count = ReceiveFromHost(G->TR->connection, buf, sizeof(buf))) <= 0)
    G->TR->connection->error = CONNECTERR_UNKNOWN_ERROR;

  while(G->TR->connection->error == CONNECTERR_NO_ERROR && xget(G->TR->GUI.GR_STATS, MUIA_TransferControlGroup_Aborted) == FALSE)
  {
    char *bufptr;

    // now we iterate through the received string
    // and strip out the '\r' character.
    // we iterate through it because the strings we receive
    // from the socket can be splitted somehow.
    for(bufptr = buf; read > 0; bufptr++, read--)
    {
      // first we check if our buffer is full and if so we
      // write it to the file.
      if(l == sizeof(line) || done == TRUE)
      {
        // update the transfer status
        DoMethod(G->TR->GUI.GR_STATS, MUIM_TransferControlGroup_Update, l, tr(MSG_TR_Downloading));

        // write the line to the file now
        if(fwrite(line, 1, l, fh) != (size_t)l)
        {
          ER_NewError(tr(MSG_ER_ErrorWriteMailfile), filename);
          break;
        }

        // if we end up here and done is true we have to break that iteration
        if(done == TRUE)
          break;

        // set l to zero so that the next char gets written to the beginning
        l = 0;
        lineptr = line;
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
    if(done == TRUE || G->TR->connection->error != CONNECTERR_NO_ERROR || xget(G->TR->GUI.GR_STATS, MUIA_TransferControlGroup_Aborted) == TRUE)
      break;

    // if not, we get another bunch of data and start over again.
    if((read = ReceiveFromHost(G->TR->connection, buf, sizeof(buf))) <= 0)
      break;

    count += read;
  }

  if(done == FALSE)
    count = 0;

  RETURN(count);
  return count;
}

///

/*** POP3 routines ***/
/// TR_SendPOP3Cmd
//  Sends a command to the POP3 server
static char *TR_SendPOP3Cmd(const enum POPCommand command, const char *parmtext, const char *errorMsg)
{
  char *result = NULL;

  ENTER();

  // first we check if the socket is in a valid state to proceed
  if(G->TR->connection != NULL)
  {
    static char buf[SIZE_LINE]; // SIZE_LINE should be enough for the command and reply

    // if we specified a parameter for the pop command lets add it now
    if(parmtext == NULL || parmtext[0] == '\0')
      snprintf(buf, sizeof(buf), "%s\r\n", POPcmd[command]);
    else
      snprintf(buf, sizeof(buf), "%s %s\r\n", POPcmd[command], parmtext);

    D(DBF_NET, "TCP: POP3 cmd '%s' with param '%s'", POPcmd[command], (command == POPCMD_PASS) ? "XXX" : SafeStr(parmtext));

    // send the pop command to the server and see if it was received somehow
    // and for a connect we don't send something or the server will get
    // confused.
    if(command == POPCMD_CONNECT || SendLineToHost(G->TR->connection, buf) > 0)
    {
      int received;

      // let us read the next line from the server and check if
      // some status message can be retrieved.
      if((received = ReceiveLineFromHost(G->TR->connection, buf, sizeof(buf))) > 0 &&
        strncmp(buf, POP_RESP_OKAY, strlen(POP_RESP_OKAY)) == 0)
      {
        // everything worked out fine so lets set
        // the result to our allocated buffer
        result = buf;
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
            if((p = strstr(buf, POPcmd[POPCMD_PASS])) != NULL &&
               (p = strchr(p, ' ')) != NULL)
            {
              // now cross it out
              while(*p != '\0' && *p != ' ' && *p != '\n' && *p != '\r')
                *p++ = 'X';
            }
          }

          if(G->TR->mailServer != NULL)
            ER_NewError(errorMsg, G->TR->mailServer->hostname, G->TR->mailServer->account, (char *)POPcmd[command], buf);
        }
      }
    }
  }

  RETURN(result);
  return result;
}
///
/// TR_ConnectPOP
//  Connects to a POP3 mail server
static int TR_ConnectPOP(int guilevel)
{
  char passwd[SIZE_PASSWORD];
  char host[SIZE_HOST];
  char buf[SIZE_LINE];
  char *p;
  char *welcomemsg = NULL;
  int msgs = -1;
  char *resp;
  struct MailServerNode *msn;
  int port;
  enum ConnectError err;

  ENTER();

  #warning FIXME: replace GetMailServer() usage when struct Connection is there
  if((msn = GetMailServer(&C->mailServerList, MST_POP3, G->TR->POP_Nr)) == NULL)
  {
    RETURN(-1);
    return -1;
  }

  // remember the current mail server
  G->TR->mailServer = msn;

  D(DBF_NET, "connect to POP3 server '%s'", msn->hostname);

  strlcpy(passwd, msn->password, sizeof(passwd));
  strlcpy(host, msn->hostname, sizeof(host));
  port = msn->port;

  // now we have to check whether SSL/TLS is selected for that POP account,
  // but perhaps TLS is not working.
  if((hasServerSSL(msn) || hasServerTLS(msn)) &&
     G->TR_UseableTLS == FALSE)
  {
    ER_NewError(tr(MSG_ER_UNUSABLEAMISSL));

    RETURN(-1);
    return -1;
  }

  if(C->AvoidDuplicates == TRUE)
  {
    if((G->TR->UIDLhashTable = InitUIDLhash(msn)) == NULL)
    {
      ER_NewError("Failed to init UIDL hash");

      RETURN(-1);
      return -1;
    }
  }

  if(C->TransferWindow == TWM_SHOW ||
     (C->TransferWindow == TWM_AUTO && (guilevel == POP_START || guilevel == POP_USER)))
  {
    // avoid MUIA_Window_Open's side effect of activating the window if it was already open
    if(xget(G->TR->GUI.WI, MUIA_Window_Open) == FALSE)
      set(G->TR->GUI.WI, MUIA_Window_Open, TRUE);
  }
  DoMethod(G->TR->GUI.GR_STATS, MUIM_TransferControlGroup_ShowStatus, tr(MSG_TR_Connecting));

  // If the hostname has a explicit :xxxxx port statement at the end we
  // take this one, even if its not needed anymore.
  if((p = strchr(host, ':')) != NULL)
  {
    *p = '\0';
    port = atoi(++p);
  }

  // set the busy text and window title to some
  // descriptive to the job. Here we use the "account"
  // name of the POP3 server as there might be more than
  // one configured accounts for the very same host
  // and as such the hostname might just be not enough
  if(msn->account[0] != '\0')
  {
    BusyText(tr(MSG_TR_MailTransferFrom), msn->account);
    TR_SetWinTitle(TRUE, msn->account);
  }
  else
  {
    // if the user hasn't specified any account name
    // we take the hostname instead
    BusyText(tr(MSG_TR_MailTransferFrom), host);
    TR_SetWinTitle(TRUE, host);
  }

  // now we start our connection to the POP3 server
  if((err = ConnectToHost(G->TR->connection, host, port)) != CONNECTERR_SUCCESS)
  {
    if(guilevel == POP_USER)
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
          ER_NewError(tr(MSG_ER_CONNECTERR_SOCKET_IN_USE_POP3), host, msn->account);
        break;

        // socket() execution failed
        case CONNECTERR_NO_SOCKET:
          ER_NewError(tr(MSG_ER_CONNECTERR_NO_SOCKET_POP3), host, msn->account);
        break;

        // couldn't establish non-blocking IO
        case CONNECTERR_NO_NONBLOCKIO:
          ER_NewError(tr(MSG_ER_CONNECTERR_NO_NONBLOCKIO_POP3), host, msn->account);
        break;

        // connection request timed out
        case CONNECTERR_TIMEDOUT:
          ER_NewError(tr(MSG_ER_CONNECTERR_TIMEDOUT_POP3), host, msn->account);
        break;

        // unknown host - gethostbyname() failed
        case CONNECTERR_UNKNOWN_HOST:
          ER_NewError(tr(MSG_ER_UNKNOWN_HOST_POP3), host, msn->account);
        break;

        // general connection error
        case CONNECTERR_UNKNOWN_ERROR:
          ER_NewError(tr(MSG_ER_CANNOT_CONNECT_POP3), host, msn->account);
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

  // If this connection should be a STLS like connection we have to get the welcome
  // message now and then send the STLS command to start TLS negotiation
  if(hasServerTLS(msn))
  {
    DoMethod(G->TR->GUI.GR_STATS, MUIM_TransferControlGroup_ShowStatus, tr(MSG_TR_WaitWelcome));

    // Initiate a connect and see if we succeed
    if((resp = TR_SendPOP3Cmd(POPCMD_CONNECT, NULL, tr(MSG_ER_POP3WELCOME))) == NULL)
      goto out;

    welcomemsg = StrBufCpy(NULL, resp);

    // If the user selected STLS support we have to first send the command
    // to start TLS negotiation (RFC 2595)
    if(TR_SendPOP3Cmd(POPCMD_STLS, NULL, tr(MSG_ER_BADRESPONSE_POP3)) == NULL)
      goto out;
  }

  // Here start the TLS/SSL Connection stuff
  if(hasServerSSL(msn) || hasServerTLS(msn))
  {
    DoMethod(G->TR->GUI.GR_STATS, MUIM_TransferControlGroup_ShowStatus, tr(MSG_TR_INITTLS));

    // Now we have to Initialize and Start the TLS stuff if requested
    if(MakeSecureConnection(G->TR->connection) == FALSE)
    {
      ER_NewError(tr(MSG_ER_INITTLS_POP3), host, msn->account);
      goto out;
    }
  }

  // If this was a connection on a stunnel on port 995 or a non-ssl connection
  // we have to get the welcome message now
  if(hasServerSSL(msn) == TRUE || hasServerTLS(msn) == FALSE)
  {
    // Initiate a connect and see if we succeed
    if((resp = TR_SendPOP3Cmd(POPCMD_CONNECT, NULL, tr(MSG_ER_POP3WELCOME))) == NULL)
      goto out;

    welcomemsg = StrBufCpy(NULL, resp);
  }

  if(passwd[0] == '\0')
  {
    // make sure the application isn't iconified
    if(xget(G->App, MUIA_Application_Iconified) == TRUE)
      PopUp();

    snprintf(buf, sizeof(buf), tr(MSG_LOG_CONNECT_POP3), msn->account);
    if(StringRequest(passwd, SIZE_PASSWORD, tr(MSG_TR_PopLogin), buf, tr(MSG_Okay), NULL, tr(MSG_Cancel), TRUE, G->TR->GUI.WI) == 0)
      goto out;
  }

  // if the user has selected APOP for that POP3 host
  // we have to process it now
  if(hasServerAPOP(msn))
  {
    // Now we get the APOP Identifier out of the welcome
    // message
    if((p = strchr(welcomemsg, '<')) != NULL)
    {
      struct MD5Context context;
      unsigned char digest[16];
      char digestHex[33];

      strlcpy(buf, p, sizeof(buf));
      if((p = strchr(buf, '>')) != NULL)
        p[1] = '\0';

      // then we send the APOP command to authenticate via APOP
      strlcat(buf, passwd, sizeof(buf));
      md5init(&context);
      md5update(&context, (unsigned char *)buf, strlen(buf));
      md5final(digest, &context);
      md5digestToHex(digest, digestHex);
      snprintf(buf, sizeof(buf), "%s %s", msn->username, digestHex);
      DoMethod(G->TR->GUI.GR_STATS, MUIM_TransferControlGroup_ShowStatus, tr(MSG_TR_SendAPOPLogin));
      if(TR_SendPOP3Cmd(POPCMD_APOP, buf, tr(MSG_ER_BADRESPONSE_POP3)) == NULL)
        goto out;
    }
    else
    {
      ER_NewError(tr(MSG_ER_NO_APOP), host, msn->account);
      goto out;
    }
  }
  else
  {
    DoMethod(G->TR->GUI.GR_STATS, MUIM_TransferControlGroup_ShowStatus, tr(MSG_TR_SendUserID));
    if(TR_SendPOP3Cmd(POPCMD_USER, msn->username, tr(MSG_ER_BADRESPONSE_POP3)) == NULL)
      goto out;

    DoMethod(G->TR->GUI.GR_STATS, MUIM_TransferControlGroup_ShowStatus, tr(MSG_TR_SendPassword));
    if(TR_SendPOP3Cmd(POPCMD_PASS, passwd, tr(MSG_ER_BADRESPONSE_POP3)) == NULL)
      goto out;
  }

  DoMethod(G->TR->GUI.GR_STATS, MUIM_TransferControlGroup_ShowStatus, tr(MSG_TR_GetStats));
  if((resp = TR_SendPOP3Cmd(POPCMD_STAT, NULL, tr(MSG_ER_BADRESPONSE_POP3))) == NULL)
    goto out;

  sscanf(&resp[4], "%d", &msgs);
  if(msgs != 0)
    AppendToLogfile(LF_VERBOSE, 31, tr(MSG_LOG_CONNECT_POP3), msn->account, msgs);

out:

  FreeStrBuf(welcomemsg);

  RETURN(msgs);
  return msgs;
}
///
/// TR_DisplayMailList
//  Displays a list of messages ready for download
static void TR_DisplayMailList(BOOL largeonly)
{
  Object *lv = G->TR->GUI.LV_MAILS;
  struct MailTransferNode *tnode;
  int pos=0;

  ENTER();

  set(lv, MUIA_NList_Quiet, TRUE);

  // search through our transferList
  ForEachMailTransferNode(&G->TR->transferList, tnode)
  {
    #if defined(DEBUG)
    struct Mail *mail = tnode->mail;
    #endif

    D(DBF_GUI, "checking mail with flags %08lx and subject '%s'", tnode->tflags, mail->Subject);
    // only display mails to be downloaded
    if(isFlagSet(tnode->tflags, TRF_TRANSFER) || isFlagSet(tnode->tflags, TRF_PRESELECT))
    {
      // add this mail to the transfer list in case we either
      // should show ALL mails or the mail size is >= the warning size
      if(largeonly == FALSE || isFlagSet(tnode->tflags, TRF_PRESELECT))
      {
        tnode->position = pos++;

        DoMethod(lv, MUIM_NList_InsertSingle, tnode, MUIV_NList_Insert_Bottom);
        D(DBF_GUI, "added mail with subject '%s' and size %ld to preselection list", mail->Subject, mail->Size);
      }
      else
        D(DBF_GUI, "skipped mail with subject '%s' and size %ld", mail->Subject, mail->Size);
    }
    else
      D(DBF_GUI, "skipped mail with subject '%s' and size %ld", mail->Subject, mail->Size);
  }

  xset(lv, MUIA_NList_Active, MUIV_NList_Active_Top,
           MUIA_NList_Quiet, FALSE);

  LEAVE();
}
///
/// TR_GetMessageList_GET
//  Collects messages waiting on a POP3 server
static BOOL TR_GetMessageList_GET(void)
{
  BOOL success;

  ENTER();

  // we issue a LIST command without argument to get a list
  // of all messages available on the server. This command will
  // return TRUE if the server responsed with a +OK
  if(TR_SendPOP3Cmd(POPCMD_LIST, NULL, tr(MSG_ER_BADRESPONSE_POP3)) != NULL)
  {
    char buf[SIZE_LINE];

    success = TRUE;

    ClearMailTransferList(&G->TR->transferList);

    // get the first line the pop server returns after the LINE command
    if(ReceiveLineFromHost(G->TR->connection, buf, sizeof(buf)) > 0)
    {
      // we get the "scan listing" as long as we haven't received a a
      // finishing octet
      while(G->TR->connection->error == CONNECTERR_NO_ERROR && strncmp(buf, ".\r\n", 3) != 0)
      {
        int index, size;
        struct Mail *newMail;

        // read the index and size of the first message
        sscanf(buf, "%d %d", &index, &size);

        if(index > 0 && (newMail = calloc(1, sizeof(struct Mail))) != NULL)
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
            TRF_TRANSFER|TRF_PRESELECT,
            TRF_NONE,
            TRF_TRANSFER|TRF_DELETE|TRF_PRESELECT,
            TRF_PRESELECT,
            TRF_TRANSFER|TRF_PRESELECT,
            TRF_PRESELECT,
            TRF_TRANSFER|TRF_DELETE|TRF_PRESELECT
          };
          int tflags;

          newMail->Size  = size;

          mode = (C->DownloadLarge == TRUE ? 1 : 0) +
                 (hasServerPurge(G->TR->mailServer) == TRUE ? 2 : 0) +
                 (G->TR->GUIlevel == POP_USER ? 4 : 0) +
                 ((C->WarnSize > 0 && newMail->Size >= (C->WarnSize*1024)) ? 8 : 0);
          tflags = mode2tflags[mode];

          // if preselection is configured then force displaying this mail in the list
          if(C->PreSelection >= PSM_ALWAYS)
            SET_FLAG(tflags, TRF_PRESELECT);

          D(DBF_GUI, "mail transfer mode %ld, tflags %08lx", mode, tflags);

          // allocate a new MailMailTransferNode and add it to our
          // new transferlist
          if((tnode = CreateMailTransferNode(NULL, tflags)) != NULL)
          {
            tnode->mail = newMail;
            tnode->index = index;

            AddMailTransferNode(&G->TR->transferList, tnode);
          }
        }

        // now read the next Line
        if(ReceiveLineFromHost(G->TR->connection, buf, SIZE_LINE) <= 0)
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
/// TR_GetMessageDetails
//  Gets header from a message stored on the POP3 server
void TR_GetMessageDetails(struct MailTransferNode *tnode, int lline)
{
  struct Mail *mail = tnode->mail;

  ENTER();

  if(mail->From.Address[0] == '\0' && xget(G->TR->GUI.GR_STATS, MUIA_TransferControlGroup_Aborted) == FALSE && G->TR->connection->error == CONNECTERR_NO_ERROR)
  {
    char cmdbuf[SIZE_SMALL];

    // we issue a TOP command with a one line message body.
    //
    // This command is optional within the RFC 1939 specification
    // and therefore we don't throw any error
    snprintf(cmdbuf, sizeof(cmdbuf), "%d 1", tnode->index);
    if(TR_SendPOP3Cmd(POPCMD_TOP, cmdbuf, NULL) != NULL)
    {
      struct TempFile *tf;

      // we generate a temporary file to buffer the TOP list
      // into it.
      if((tf = OpenTempFile("w")) != NULL)
      {
        struct ExtendedMail *email;
        BOOL done = FALSE;

        // now we call a subfunction to receive data from the POP3 server
        // and write it in the filehandle as long as there is no termination \r\n.\r\n
        if(TR_RecvToFile(tf->FP, tf->Filename) > 0)
          done = TRUE;

        // close the filehandle now.
        fclose(tf->FP);
        tf->FP = NULL;

        // If we end up here because of an error, abort or the upper loop wasn't finished
        // we exit immediatly with deleting the temp file also.
        if(G->TR->connection->error != CONNECTERR_NO_ERROR || xget(G->TR->GUI.GR_STATS, MUIA_TransferControlGroup_Aborted) == TRUE || done == FALSE)
          lline = -1;
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
          else if(lline == -2)
            TR_ApplyRemoteFilters(tnode);

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

  if(lline >= 0)
    DoMethod(G->TR->GUI.LV_MAILS, MUIM_NList_Redraw, lline);

  // signal the application to update now
  DoMethod(G->App, MUIM_Application_InputBuffered);

  LEAVE();
}
///
/// TR_DisconnectPOP
//  Terminates a POP3 session
static void TR_DisconnectPOP(void)
{
  ENTER();

  DoMethod(G->TR->GUI.GR_STATS, MUIM_TransferControlGroup_ShowStatus, tr(MSG_TR_Disconnecting));

  if(G->TR->connection->error == CONNECTERR_NO_ERROR)
    TR_SendPOP3Cmd(POPCMD_QUIT, NULL, tr(MSG_ER_BADRESPONSE_POP3));

  // make sure we don't send a "keep alive" signal anymore
  StopTimer(TIMER_POP3_KEEPALIVE);

  TR_Disconnect();

  LEAVE();
}

///
/// TR_GetMailFromNextPOP
//  Downloads and filters mail from a POP3 account
void TR_GetMailFromNextPOP(BOOL isfirst, int singlepop, enum GUILevel guilevel)
{
  static int laststats;
  int msgs;
  int pop = singlepop;

  ENTER();

  if(isfirst == TRUE) /* Init first connection */
  {
    struct Connection *conn;

    G->LastDL.Error = TRUE;

    if(CO_IsValid() == FALSE)
    {
      LEAVE();
      return;
    }

    if((conn = CreateConnection()) == NULL)
    {
      LEAVE();
      return;
    }

    if(ConnectionIsOnline(conn) == FALSE)
    {
      DeleteConnection(conn);

      if(guilevel == POP_USER)
        ER_NewError(tr(MSG_ER_OPENTCPIP));

      LEAVE();
      return;
    }

    if((G->TR = TR_New(guilevel == POP_USER ? TR_GET_USER : TR_GET_AUTO)) == NULL)
    {
      DeleteConnection(conn);

      LEAVE();
      return;
    }

    if((G->TR->remoteFilters = CloneFilterList(APPLY_REMOTE)) == NULL)
    {
      DeleteConnection(conn);

      LEAVE();
      return;
    }

    G->TR->connection = conn;
    G->TR->Checking = TRUE;
    UpdateAppIcon();
    G->TR->GUIlevel = guilevel;
    if(singlepop >= 0)
      G->TR->SinglePOP = TRUE;
    else
      G->TR->POP_Nr = -1;
    laststats = 0;

    set(G->TR->GUI.GR_STATS, MUIA_TransferControlGroup_Connection, G->TR->connection);
  }
  else if(G->TR->mailServer != NULL) /* Finish previous connection */
  {
    D(DBF_NET, "downloaded %ld mails from server '%s'", G->TR->Stats.Downloaded, G->TR->mailServer->hostname);

    TR_DisconnectPOP();
    TR_Cleanup();
    AppendToLogfile(LF_ALL, 30, tr(MSG_LOG_RETRIEVED_POP3), G->TR->Stats.Downloaded-laststats, G->TR->mailServer->account);
    if(G->TR->SinglePOP == TRUE)
      pop = -1;

    laststats = G->TR->Stats.Downloaded;

    // free/cleanup the UIDL hash tables
    if(C->AvoidDuplicates == TRUE)
    {
      CleanupUIDLhash(G->TR->UIDLhashTable);
      G->TR->UIDLhashTable = NULL;
    }

    // forget the current mail server again
    G->TR->mailServer = NULL;
  }

  // what is the next POP3 server we should check
  if(G->TR->SinglePOP == FALSE)
  {
    pop = -1;

    while(++G->TR->POP_Nr >= 0)
    {
      struct MailServerNode *msn = GetMailServer(&C->mailServerList, MST_POP3, G->TR->POP_Nr);

      if(msn != NULL)
      {
        if(isServerActive(msn))
        {
          pop = G->TR->POP_Nr;
          break;
        }
      }
      else
      {
        pop = -1;
        break;
      }
    }
  }

  if(pop == -1) /* Finish last connection */
  {
    // close the TCP/IP connection
    DeleteConnection(G->TR->connection);
    G->TR->connection = NULL;
    G->TR->mailServer = NULL;

    // make sure the transfer window is closed
    set(G->TR->GUI.WI, MUIA_Window_Open, FALSE);

    DeleteFilterList(G->TR->remoteFilters);
    G->TR->remoteFilters = NULL;

    MA_StartMacro(MACRO_POSTGET, itoa((int)G->TR->Stats.Downloaded));

    // tell the appicon that we are finished with checking mail
    // the apply rules or UpdateAppIcon() function will refresh it later on
    G->TR->Checking = FALSE;

    // we only apply the filters if we downloaded something, or it's wasted
    if(G->TR->Stats.Downloaded > 0)
    {
      struct Folder *folder;
      struct FilterResult filterResult;

      D(DBF_UTIL, "filter %ld/%ld downloaded mails", G->TR->downloadedMails->count, G->TR->Stats.Downloaded);
      FilterMails(FO_GetFolderByType(FT_INCOMING, NULL), G->TR->downloadedMails, APPLY_AUTO, &filterResult);

      // Now we jump to the first new mail we received if the number of messages has changed
      // after the mail transfer
      if(C->JumpToIncoming == TRUE)
        MA_JumpToNewMsg();

      // only call the DisplayStatistics() function if the actual folder wasn't already the INCOMING
      // one or we would have refreshed it twice
      if((folder = FO_GetCurrentFolder()) != NULL && !isIncomingFolder(folder))
        DisplayStatistics((struct Folder *)-1, TRUE);
      else
        UpdateAppIcon();

      TR_NewMailAlert(&filterResult);
    }
    else
      UpdateAppIcon();

    // lets populate the LastDL statistics variable with the stats
    // of this download.
    memcpy(&G->LastDL, &G->TR->Stats, sizeof(struct DownloadResult));

    MA_ChangeTransfer(TRUE);

    DeleteMailList(G->TR->downloadedMails);
    G->TR->downloadedMails = NULL;

    DisposeModulePush(&G->TR);

    LEAVE();
    return;
  }

  // lets initialize some important data first so that the transfer can
  // begin
  G->TR->POP_Nr = pop;
  G->TR->Pause = FALSE;
  G->TR->Start = FALSE;

  // if the window isn't open we don't need to update it, do we?
  if(isfirst == FALSE && xget(G->TR->GUI.WI, MUIA_Window_Open) == TRUE)
  {
    DoMethod(G->TR->GUI.GR_STATS, MUIM_TransferControlGroup_Reset);
  }

  if((msgs = TR_ConnectPOP(G->TR->GUIlevel)) != -1)
  {
    // connection succeeded
    if(msgs > 0)
    {
      // there are messages on the server
      if(TR_GetMessageList_GET() == TRUE)
      {
        // message list read OK
        BOOL preselect = FALSE;

        G->TR->Stats.OnServer += msgs;

        // apply possible remote filters
        if(IsMinListEmpty(G->TR->remoteFilters) == FALSE)
        {
          struct MailTransferNode *tnode;

          DoMethod(G->TR->GUI.GR_STATS, MUIM_TransferControlGroup_ShowStatus, tr(MSG_TR_ApplyFilters));
          ForEachMailTransferNode(&G->TR->transferList, tnode)
            TR_GetMessageDetails(tnode, -2);
        }

        // if the user wants to avoid to receive the
        // same message from the POP3 server again
        // we have to analyze the UIDL of it
        if(C->AvoidDuplicates == TRUE)
        {
          if(FilterDuplicates() == TRUE)
          {
            SET_FLAG(G->TR->mailServer->flags, MSF_UIDLCHECKED);
          }
        }

        // manually initiated transfer
        if(G->TR->GUIlevel == POP_USER)
        {
          // preselect messages if preference is "always" or "always, sizes only"
          if(C->PreSelection >= PSM_ALWAYS)
            preselect = TRUE;
          else if(C->WarnSize > 0 && C->PreSelection != PSM_NEVER)
          {
            // ...or any sort of preselection and there is a maximum size

            struct MailTransferNode *tnode;

            ForEachMailTransferNode(&G->TR->transferList, tnode)
            {
              #if defined(DEBUG)
              struct Mail *mail = tnode->mail;
              #endif

              D(DBF_GUI, "checking mail with subject '%s' and size %ld for preselection", mail->Subject, mail->Size);
              // check the size of those mails only, which are left for download
              if(isFlagSet(tnode->tflags, TRF_PRESELECT))
              {
                D(DBF_GUI, "mail with subject '%s' and size %ld exceeds size limit", mail->Subject, mail->Size);
                preselect = TRUE;
                break;
              }
            }
          }
        }

        // anything to preselect?
        if(preselect == TRUE)
        {
          // avoid MUIA_Window_Open's side effect of activating the window if it was already open
          if(xget(G->TR->GUI.WI, MUIA_Window_Open) == FALSE)
            set(G->TR->GUI.WI, MUIA_Window_Open, TRUE);

          // if preselect mode is "large only" we display the mail list
          // but make sure to display/add large emails only.
          if(C->PreSelection == PSM_LARGE)
          {
            TR_DisplayMailList(TRUE);
            set(G->TR->GUI.GR_LIST, MUIA_ShowMe, TRUE);
          }
          else
            TR_DisplayMailList(FALSE);

          DoMethod(G->TR->GUI.WI, MUIM_Window_ScreenToFront);
          DoMethod(G->TR->GUI.WI, MUIM_Window_ToFront);
          // activate window only if main window activ
          set(G->TR->GUI.WI, MUIA_Window_Activate, xget(G->MA->GUI.WI, MUIA_Window_Activate));

          set(G->TR->GUI.GR_PAGE, MUIA_Group_ActivePage, 0);
          G->TR->GMD_Mail = FirstMailTransferNode(&G->TR->transferList);
          G->TR->GMD_Line = 0;
          TR_CompleteMsgList();
        }
        else
        {
          CallHookPkt(&TR_ProcessGETHook, 0, 0);
        }

        BusyEnd();

        LEAVE();
        return;
      }
      else
        E(DBF_NET, "couldn't retrieve MessageList");
    }
    else
    {
      if(G->TR->mailServer == NULL)
      {
        LEAVE();
        return;
      }

      W(DBF_NET, "no messages found on server '%s'", G->TR->mailServer->hostname);

      // per default we flag that POP3 server as being UIDLchecked
      if(C->AvoidDuplicates == TRUE)
        SET_FLAG(G->TR->mailServer->flags, MSF_UIDLCHECKED);
    }
  }
  else
    G->TR->Stats.Error = TRUE;

  BusyEnd();

  TR_GetMailFromNextPOP(FALSE, 0, 0);

  LEAVE();
}
///
/// TR_SendPOP3KeepAlive
// Function that sends a STAT command regularly to a POP3 to
// prevent it from dropping the connection.
BOOL TR_SendPOP3KeepAlive(void)
{
  BOOL result;

  ENTER();

  // here we send a STAT command instead of a NOOP which normally
  // should do the job as well. But there are several known POP3
  // servers out there which are known to ignore the NOOP commands
  // for keepalive message, so STAT should be the better choice.
  result = (TR_SendPOP3Cmd(POPCMD_STAT, NULL, tr(MSG_ER_BADRESPONSE_POP3)) != NULL);

  RETURN(result);
  return result;
}
///
/// TR_LoadMessage
//  Retrieves a message from the POP3 server
BOOL TR_LoadMessage(struct Folder *infolder, const int number)
{
  static char mfile[SIZE_MFILE];
  char msgnum[SIZE_SMALL];
  char msgfile[SIZE_PATHFILE];
  BOOL result = FALSE;
  FILE *fh;

  ENTER();

  strlcpy(msgfile, MA_NewMailFile(infolder, mfile), sizeof(msgfile));

  // open the new mailfile for writing out the retrieved
  // data
  if((fh = fopen(msgfile, "w")) != NULL)
  {
    BOOL done = FALSE;

    setvbuf(fh, NULL, _IOFBF, SIZE_FILEBUF);

    snprintf(msgnum, sizeof(msgnum), "%d", number);
    if(TR_SendPOP3Cmd(POPCMD_RETR, msgnum, tr(MSG_ER_BADRESPONSE_POP3)) != NULL)
    {
      // now we call a subfunction to receive data from the POP3 server
      // and write it in the filehandle as long as there is no termination \r\n.\r\n
      if(TR_RecvToFile(fh, msgfile) > 0)
        done = TRUE;
    }
    fclose(fh);

    if(xget(G->TR->GUI.GR_STATS, MUIA_TransferControlGroup_Aborted) == FALSE && G->TR->connection->error == CONNECTERR_NO_ERROR && done == TRUE)
    {
      struct ExtendedMail *mail;

      if((mail = MA_ExamineMail(infolder, mfile, FALSE)) != NULL)
      {
        struct Mail *newMail;

        if((newMail = AddMailToList(&mail->Mail, infolder)) != NULL)
        {
          char mailfile[SIZE_PATHFILE];

          // we have to get the actual Time and place it in the transDate, so that we know at
          // which time this mail arrived
          GetSysTimeUTC(&newMail->transDate);

          newMail->sflags = SFLAG_NEW;
          MA_UpdateMailFile(newMail);

          LockMailList(G->TR->downloadedMails);
          AddNewMailNode(G->TR->downloadedMails, newMail);
          UnlockMailList(G->TR->downloadedMails);

          // if the current folder is the inbox we
          // can go and add the mail instantly to the maillist
          if(FO_GetCurrentFolder() == infolder)
            DoMethod(G->MA->GUI.PG_MAILLIST, MUIM_NList_InsertSingle, newMail, MUIV_NList_Insert_Sorted);

          AppendToLogfile(LF_VERBOSE, 32, tr(MSG_LOG_RetrievingVerbose), AddrName(newMail->From), newMail->Subject, newMail->Size);

          GetMailFile(mailfile, sizeof(mailfile), NULL, newMail);

          MA_StartMacro(MACRO_NEWMSG, mailfile);
          MA_FreeEMailStruct(mail);

          result = TRUE;
        }
      }
    }

    if(result == FALSE)
    {
      DeleteFile(msgfile);

      // we need to set the folder flags to modified so that the .index will be saved later.
      SET_FLAG(infolder->Flags, FOFL_MODIFY);
    }
  }
  else
    ER_NewError(tr(MSG_ER_ErrorWriteMailfile), mfile);

  RETURN(result);
  return result;
}
///
/// TR_DeleteMessage
//  Deletes a message on the POP3 server
BOOL TR_DeleteMessage(int number)
{
  BOOL result = FALSE;
  char msgnum[SIZE_SMALL];

  ENTER();

  snprintf(msgnum, sizeof(msgnum), "%d", number);

  // update the transfer status
  DoMethod(G->TR->GUI.GR_STATS, MUIM_TransferControlGroup_Update, TCG_SETMAX, tr(MSG_TR_DeletingServerMail));

  if(TR_SendPOP3Cmd(POPCMD_DELE, msgnum, tr(MSG_ER_BADRESPONSE_POP3)) != NULL)
  {
    G->TR->Stats.Deleted++;
    result = TRUE;
  }

  RETURN(result);
  return result;
}
///
/// FilterDuplicates
//
static BOOL FilterDuplicates(void)
{
  BOOL result = FALSE;

  ENTER();

  if(G->TR->mailServer == NULL)
  {
    RETURN(FALSE);
    return FALSE;
  }

  // we first make sure the UIDL list is loaded from disk
  if(G->TR->UIDLhashTable != NULL)
  {
    // check if there is anything to transfer at all
    if(IsMailTransferListEmpty(&G->TR->transferList) == FALSE)
    {
      // inform the user of the operation
      DoMethod(G->TR->GUI.GR_STATS, MUIM_TransferControlGroup_ShowStatus, tr(MSG_TR_CHECKUIDL));

      // before we go and request each UIDL of a message we check wheter the server
      // supports the UIDL command at all
      if(TR_SendPOP3Cmd(POPCMD_UIDL, NULL, NULL) != NULL)
      {
        char buf[SIZE_LINE];

        // get the first line the pop server returns after the UIDL command
        if(ReceiveLineFromHost(G->TR->connection, buf, sizeof(buf)) > 0)
        {
          // we get the "unique-id list" as long as we haven't received a
          // finishing octet
          while(xget(G->TR->GUI.GR_STATS, MUIA_TransferControlGroup_Aborted) == FALSE && G->TR->connection->error == CONNECTERR_NO_ERROR && strncmp(buf, ".\r\n", 3) != 0)
          {
            char *p;

            // now parse the line and get the message number and UIDL
            // each UIDL entry is transmitted as "number uidl"
            if((p = strchr(buf, ' ')) != NULL)
            {
              int num;
              char *uidl;
              struct MailTransferNode *tnode;

              // replace the space by a NUL byte and convert the first part to an integer
              *p++ = '\0';
              num = atoi(buf);
              // strip the trailing CR+LF
              uidl = p;
              if((p = strchr(uidl, '\r')) != NULL)
                *p = '\0';

              // search through our transferList
              ForEachMailTransferNode(&G->TR->transferList, tnode)
              {
                if(tnode->index == num)
                {
                  if((tnode->uidl = strdup(uidl)) != NULL)
                  {
                    struct UIDLtoken *token;

                    // check if this UIDL is known already
                    if((token = FindUIDL(G->TR->UIDLhashTable, tnode->uidl)) != NULL)
                    {
                      D(DBF_UIDL, "mail %ld: found UIDL '%s', flags=%08lx", tnode->index, tnode->uidl, token->flags);

                      // check if we knew this UIDL before
                      if(isFlagSet(token->flags, UIDLF_OLD))
                      {
                      // make sure the mail is flagged as being ignoreable
                        G->TR->Stats.DupSkipped++;
                        // don't download this mail, because it has been downloaded before
                        CLEAR_FLAG(tnode->tflags, TRF_TRANSFER);
                        // mark this UIDL as old+new, thus it will be saved upon cleanup
                        SET_FLAG(token->flags, UIDLF_NEW);
                      }
                    }
                  }

                  break;
                }
              }
            }

            if(xget(G->TR->GUI.GR_STATS, MUIA_TransferControlGroup_Aborted) == TRUE || G->TR->connection->error != CONNECTERR_NO_ERROR)
              break;

            // now read the next Line
            if(ReceiveLineFromHost(G->TR->connection, buf, sizeof(buf)) <= 0)
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

        W(DBF_UIDL, "POP3 server '%s' doesn't support UIDL command!", G->TR->mailServer->hostname);

        // search through our transferList
        ForEachMailTransferNode(&G->TR->transferList, tnode)
        {
          // if the server doesn't support the UIDL command we
          // use the TOP command and generate our own UIDL within
          // the GetMessageDetails function
          TR_GetMessageDetails(tnode, -1);

          // now that we should successfully obtained the UIDL of the
          // mailtransfernode we go and check if that UIDL is already in our UIDLhash
          // and if so we go and flag the mail as a mail that should not be downloaded
          // automatically
          if(tnode->uidl != NULL)
          {
            struct UIDLtoken *token;

            if((token = AddUIDLtoHash(G->TR->UIDLhashTable, tnode->uidl, UIDLF_NEW)) != NULL)
            {
              D(DBF_UIDL, "mail %ld: found UIDL '%s', flags=%08lx", tnode->index, tnode->uidl, token->flags);

              // check if we knew this UIDL before
              if(isFlagSet(token->flags, UIDLF_OLD))
              {
                G->TR->Stats.DupSkipped++;
                // don't download this mail, because it has been downloaded before
                CLEAR_FLAG(tnode->tflags, TRF_TRANSFER);
                // mark this UIDL as old+new, thus it will be saved upon cleanup
                SET_FLAG(token->flags, UIDLF_NEW);
              }
            }
          }

          if(xget(G->TR->GUI.GR_STATS, MUIA_TransferControlGroup_Aborted) == TRUE || G->TR->connection->error != CONNECTERR_NO_ERROR)
            break;
        }
      }

      result = (xget(G->TR->GUI.GR_STATS, MUIA_TransferControlGroup_Aborted) == FALSE && G->TR->connection->error == CONNECTERR_NO_ERROR);
    }
    else
      result = TRUE;
  }
  else
    E(DBF_UIDL, "UIDLhashTable isn't initialized yet!");

  RETURN(result);
  return result;
}

///
