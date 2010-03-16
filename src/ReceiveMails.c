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

#if !defined(__amigaos4__)
#include <clib/alib_protos.h>
#endif

#include <proto/dos.h>
#include <proto/exec.h>
#include <proto/utility.h>

#if defined(__amigaos4__)
#include <proto/application.h>
#endif

#include <mui/NList_mcc.h>
#include <mui/NListtree_mcc.h>

#include <SDI_hook.h>

#include "YAM.h"
#include "YAM_addressbookEntry.h"
#include "YAM_config.h"
#include "YAM_error.h"
#include "YAM_find.h"
#include "YAM_mainFolder.h"
#include "YAM_stringsizes.h"

#include "mime/md5.h"

#include "extrasrc.h"

#include "mui/Classes.h"

#include "AppIcon.h"
#include "FileInfo.h"
#include "Locale.h"
#include "MailList.h"
#include "MUIObjects.h"
#include "ReceiveMails.h"
#include "Requesters.h"
#include "Transfer.h"
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

// local macros & defines
#define GetLong(p,o)  ((((unsigned char*)(p))[o])       | \
                       (((unsigned char*)(p))[o+1]<<8)  | \
                       (((unsigned char*)(p))[o+2]<<16) | \
                       (((unsigned char*)(p))[o+3]<<24))


/**************************************************************************/
// static function prototypes
static int TR_RecvToFile(struct TransferNode *tfn, FILE *fh, const char *filename);

// static UIDL function prototypes
static void CleanupUIDLhash(void);
static BOOL FilterDuplicates(struct TransferNode *tfn);
static void AddUIDLtoHash(const char *uidl, BOOL checked);

struct UIDLtoken
{
  struct HashEntryHeader hash;
  const char *uidl;
  BOOL checked;
};

/***************************************************************************
 Module: ReceiveMails
***************************************************************************/

/*** POP3 routines ***/
/// TR_SendPOP3Cmd
//  Sends a command to the POP3 server
static char *TR_SendPOP3Cmd(struct TransferNode *tfn, const enum POPCommand command, const char *parmtext, const char *errorMsg)
{
  char *result = NULL;

  ENTER();

  // first we check if the socket is in a valid state to proceed
  if(tfn->socket != TCP_NO_SOCKET)
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
    if(command == POPCMD_CONNECT || TR_WriteLine(tfn, buf) > 0)
    {
      // let us read the next line from the server and check if
      // some status message can be retrieved.
      if(TR_ReadLine(tfn, buf, sizeof(buf)) > 0 &&
        strncmp(buf, POP_RESP_OKAY, strlen(POP_RESP_OKAY)) == 0)
      {
        // everything worked out fine so lets set
        // the result to our allocated buffer
        result = buf;
      }
      else
      {
        // only report an error if explicitly wanted
        if(errorMsg != NULL)
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

          ER_NewError(errorMsg, tfn->msn->hostname, tfn->msn->account, (char *)POPcmd[command], buf);
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
static int TR_ConnectPOP(struct TransferNode *tfn)
{
  char passwd[SIZE_PASSWORD];
  char host[SIZE_HOST];
  char buf[SIZE_LINE];
  char *p;
  char *welcomemsg = NULL;
  int msgs = -1;
  char *resp;
  enum ConnectError err;
  struct MailServerNode *msn = tfn->msn;
  int port;

  ENTER();

  D(DBF_NET, "connect to POP3 server '%s'", msn->hostname);

  // copy the passwd and hostname as we might modify it
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

  if(C->TransferWindow == TWM_SHOW ||
     (C->TransferWindow == TWM_AUTO && (tfn->mode == RECV_AUTO_START || tfn->mode == RECV_USER)))
  {
    SafeOpenWindow(G->transferWindowObject);
  }

  xset(G->transferWindowObject, MUIA_TransferWindow_StatusLabel, tr(MSG_TR_Connecting));

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
    xset(G->transferWindowObject, MUIA_TransferWindow_Title, msn->account);
  }
  else
  {
    // if the user hasn't specified any account name
    // we take the hostname instead
    BusyText(tr(MSG_TR_MailTransferFrom), host);
    xset(G->transferWindowObject, MUIA_TransferWindow_Title, host);
  }

  // now we start our connection to the POP3 server
  if((err = TR_Connect(tfn)) != CONNECTERR_SUCCESS)
  {
    if(tfn->mode == RECV_USER)
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
    set(G->transferWindowObject, MUIA_TransferWindow_StatusLabel, tr(MSG_TR_WaitWelcome));

    // Initiate a connect and see if we succeed
    if((resp = TR_SendPOP3Cmd(tfn, POPCMD_CONNECT, NULL, tr(MSG_ER_POP3WELCOME))) == NULL)
      goto out;

    welcomemsg = StrBufCpy(NULL, resp);

    // If the user selected STLS support we have to first send the command
    // to start TLS negotiation (RFC 2595)
    if(TR_SendPOP3Cmd(tfn, POPCMD_STLS, NULL, tr(MSG_ER_BADRESPONSE_POP3)) == NULL)
      goto out;
  }

  // Here start the TLS/SSL Connection stuff
  if(hasServerSSL(msn) || hasServerTLS(msn))
  {
    set(G->transferWindowObject, MUIA_TransferWindow_StatusLabel, tr(MSG_TR_INITTLS));

    // Now we have to Initialize and Start the TLS stuff if requested
    if(TR_InitTLS(tfn) == TRUE && TR_StartTLS(tfn) == TRUE)
      tfn->useSSL = TRUE;
    else
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
    if((resp = TR_SendPOP3Cmd(tfn, POPCMD_CONNECT, NULL, tr(MSG_ER_POP3WELCOME))) == NULL)
      goto out;

    welcomemsg = StrBufCpy(NULL, resp);
  }

  if(passwd[0] == '\0')
  {
    // make sure the application isn't iconified
    if(xget(G->App, MUIA_Application_Iconified) == TRUE)
      PopUp();

    snprintf(buf, sizeof(buf), tr(MSG_TR_PopLoginReq), msn->username, host);

    if(StringRequest(passwd, SIZE_PASSWORD, tr(MSG_TR_PopLogin), buf, tr(MSG_Okay), NULL, tr(MSG_Cancel), TRUE, G->transferWindowObject) == 0)
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
      int i, j;

      strlcpy(buf, p, sizeof(buf));
      if((p = strchr(buf, '>')) != NULL)
        p[1] = '\0';

      // then we send the APOP command to authenticate via APOP
      strlcat(buf, passwd, sizeof(buf));
      md5init(&context);
      md5update(&context, (unsigned char *)buf, strlen(buf));
      md5final(digest, &context);
      snprintf(buf, sizeof(buf), "%s ", msn->username);

      for(j=strlen(buf), i=0; i<16; j+=2, i++)
        snprintf(&buf[j], sizeof(buf)-j, "%02x", digest[i]);

      buf[j] = '\0';
      set(G->transferWindowObject, MUIA_TransferWindow_StatusLabel, tr(MSG_TR_SendAPOPLogin));

      if(TR_SendPOP3Cmd(tfn, POPCMD_APOP, buf, tr(MSG_ER_BADRESPONSE_POP3)) == NULL)
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
    set(G->transferWindowObject, MUIA_TransferWindow_StatusLabel, tr(MSG_TR_SendUserID));
    if(TR_SendPOP3Cmd(tfn, POPCMD_USER, msn->username, tr(MSG_ER_BADRESPONSE_POP3)) == NULL)
      goto out;

    set(G->transferWindowObject, MUIA_TransferWindow_StatusLabel, tr(MSG_TR_SendPassword));
    if(TR_SendPOP3Cmd(tfn, POPCMD_PASS, passwd, tr(MSG_ER_BADRESPONSE_POP3)) == NULL)
      goto out;
  }

  set(G->transferWindowObject, MUIA_TransferWindow_StatusLabel, tr(MSG_TR_GetStats));
  if((resp = TR_SendPOP3Cmd(tfn, POPCMD_STAT, NULL, tr(MSG_ER_BADRESPONSE_POP3))) == NULL)
    goto out;

  sscanf(&resp[4], "%d", &msgs);
  if(msgs != 0)
    AppendToLogfile(LF_VERBOSE, 31, tr(MSG_LOG_ConnectPOP), msn->username, host, msgs);

out:

  FreeStrBuf(welcomemsg);

  RETURN(msgs);
  return msgs;
}

///
/// TR_GetMessageList_GET
//  Collects messages waiting on a POP3 server
static BOOL TR_GetMessageList_GET(struct TransferNode *tfn)
{
  BOOL success;

  ENTER();

  // we issue a LIST command without argument to get a list
  // of all messages available on the server. This command will
  // return TRUE if the server responsed with a +OK
  if(TR_SendPOP3Cmd(tfn, POPCMD_LIST, NULL, tr(MSG_ER_BADRESPONSE_POP3)) != NULL)
  {
    char buf[SIZE_LINE];

    success = TRUE;

    NewMinList(&tfn->mailTransferList);

    // get the first line the pop server returns after the LINE command
    if(TR_ReadLine(tfn, buf, sizeof(buf)) > 0)
    {
      // we get the "scan listing" as long as we haven't received a a
      // finishing octet
      while(G->Error == FALSE && strncmp(buf, ".\r\n", 3) != 0)
      {
        int index, size;
        struct Mail *newMail;

        // read the index and size of the first message
        sscanf(buf, "%d %d", &index, &size);

        if(index > 0 && (newMail = calloc(1, sizeof(struct Mail))) != NULL)
        {
          int mode;
          struct MailTransferNode *mtn;
          static const int mode2tflags[16] =
          {
            TRF_LOAD,
            TRF_LOAD,
            TRF_LOAD|TRF_DELETE,
            TRF_LOAD|TRF_DELETE,
            TRF_LOAD,
            TRF_LOAD,
            TRF_LOAD|TRF_DELETE,
            TRF_LOAD|TRF_DELETE,
            TRF_NONE,
            TRF_LOAD|TRF_PRESELECT,
            TRF_NONE,
            TRF_LOAD|TRF_DELETE|TRF_PRESELECT,
            TRF_PRESELECT,
            TRF_LOAD|TRF_PRESELECT,
            TRF_PRESELECT,
            TRF_LOAD|TRF_DELETE|TRF_PRESELECT
          };
          int tflags;

          newMail->Size  = size;

          mode = (C->DownloadLarge == TRUE ? 1 : 0) +
                 (hasServerPurge(tfn->msn) == TRUE ? 2 : 0) +
                 (tfn->mode == RECV_USER ? 4 : 0) +
                 ((C->WarnSize > 0 && newMail->Size >= (C->WarnSize*1024)) ? 8 : 0);

          tflags = mode2tflags[mode];
          // if preselection is configured then force displaying this mail in the list
          if(C->PreSelection >= PSM_ALWAYS)
            SET_FLAG(tflags, TRF_PRESELECT);

          D(DBF_GUI, "mail transfer mode %ld, tflags %08lx", mode, tflags);

          // allocate a new MailTransferNode and add it to our
          // new transferlist
          if((mtn = calloc(1, sizeof(struct MailTransferNode))) != NULL)
          {
            mtn->mail = newMail;
            mtn->tfn = tfn;
            mtn->tflags = tflags;
            mtn->index = index;

            AddTail((struct List *)&tfn->mailTransferList, (struct Node *)mtn);
          }
        }

        // now read the next Line
        if(TR_ReadLine(tfn, buf, SIZE_LINE) <= 0)
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
/// TR_ApplyRemoteFilters
//  Applies remote filters to a message
static void TR_ApplyRemoteFilters(struct MailTransferNode *mtn)
{
  ENTER();

  // if there is no search count we can break out immediatly
  if(mtn->tfn->processFilters == TRUE)
  {
    struct Node *curNode;

    // Now we process the read header to set all flags accordingly
    IterateList(&C->filterList, curNode)
    {
      struct FilterNode *filter = (struct FilterNode *)curNode;

      if(DoFilterSearch(filter, mtn->mail) == TRUE)
      {
        if(hasExecuteAction(filter) && *filter->executeCmd)
           ExecuteCommand(filter->executeCmd, FALSE, OUT_DOS);

        if(hasPlaySoundAction(filter) && *filter->playSound)
           PlaySound(filter->playSound);

        if(hasDeleteAction(filter))
           SET_FLAG(mtn->tflags, TRF_DELETE);
        else
           CLEAR_FLAG(mtn->tflags, TRF_DELETE);

        if(hasSkipMsgAction(filter))
           CLEAR_FLAG(mtn->tflags, TRF_LOAD);
        else
           SET_FLAG(mtn->tflags, TRF_LOAD);

        // get out of this loop after a successful search
        break;
      }
    }
  }

  LEAVE();
}

///
/// TR_GetMessageDetails
//  Gets header from a message stored on the POP3 server
void TR_GetMessageDetails(struct MailTransferNode *mtn, int lline)
{
  struct Mail *mail = mtn->mail;

  ENTER();

  if(mail->From.Address[0] == '\0' &&
     mtn->tfn->abort == FALSE && G->Error == FALSE)
  {
    char cmdbuf[SIZE_SMALL];

    // we issue a TOP command with a one line message body.
    //
    // This command is optional within the RFC 1939 specification
    // and therefore we don't throw any error
    snprintf(cmdbuf, sizeof(cmdbuf), "%d 1", mtn->index);
    if(TR_SendPOP3Cmd(mtn->tfn, POPCMD_TOP, cmdbuf, NULL) != NULL)
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
        if(TR_RecvToFile(mtn->tfn, tf->FP, tf->Filename) > 0)
          done = TRUE;

        // close the filehandle now.
        fclose(tf->FP);
        tf->FP = NULL;

        // If we end up here because of an error, abort or the upper loop wasn't finished
        // we exit immediatly with deleting the temp file also.
        if(G->Error == TRUE || mtn->tfn->abort == TRUE || done == FALSE)
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
          // own one by using the MsgID and the Serverstring for the POP3
          // server.
          if(lline == -1)
          {
            char uidl[SIZE_DEFAULT+SIZE_HOST];

            snprintf(uidl, sizeof(uidl), "%s@%s", email->messageID, mtn->tfn->msn->hostname);
            mtn->UIDL = strdup(uidl);
          }
          else if(lline == -2)
            TR_ApplyRemoteFilters(mtn);

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
    DoMethod(G->transferWindowObject, MUIM_TransferWindow_RedrawEntry, lline);

  LEAVE();
}

///
/// TR_DisconnectPOP
//  Terminates a POP3 session
static void TR_DisconnectPOP(struct TransferNode *tfn)
{
  ENTER();

  set(G->transferWindowObject, MUIA_TransferWindow_StatusLabel, tr(MSG_TR_Disconnecting));

  if(G->Error == FALSE)
    TR_SendPOP3Cmd(tfn, POPCMD_QUIT, NULL, tr(MSG_ER_BADRESPONSE_POP3));

  TR_Disconnect(tfn);

  BusyEnd();

  LEAVE();
}

///
/// TR_GetMailFromNextPOP
//  Downloads and filters mail from a POP3 account
#if 0
void TR_GetMailFromNextPOP(BOOL isfirst, int singlepop, enum GUILevel guilevel)
{
  static int laststats;
  int msgs, pop = singlepop;

  ENTER();

  if(isfirst == TRUE) /* Init first connection */
  {
    G->LastDL.Error = TRUE;
    if(TR_OpenTCPIP() == FALSE)
    {
      if(guilevel == POP_USER)
        ER_NewError(tr(MSG_ER_OPENTCPIP));

      LEAVE();
      return;
    }

    if(CO_IsValid() == FALSE)
    {
      TR_CloseTCPIP();

      LEAVE();
      return;
    }
    if((G->TR = TR_New(guilevel == POP_USER ? TR_GET_USER : TR_GET_AUTO)) == NULL)
    {
      TR_CloseTCPIP();

      LEAVE();
      return;
    }
    G->TR->Checking = TRUE;
    UpdateAppIcon();
    G->TR->GUIlevel = guilevel;
    G->TR->processFilters = (AllocFilterSearch(APPLY_REMOTE) > 0);
    if(singlepop >= 0)
      G->TR->SinglePOP = TRUE;
    else
      G->TR->POP_Nr = -1;
    laststats = 0;

    // now we find out if we need to check for duplicates
    // during POP3 processing or if we can skip that.
    G->TR->DuplicatesChecking = FALSE;

    if(C->AvoidDuplicates == TRUE)
    {
      if(G->TR->SinglePOP == TRUE)
      {
        if(C->P3[pop] != NULL)
          G->TR->DuplicatesChecking = TRUE;
      }
      else
      {
        int i;

        for(i=0; i < MAXP3; i++)
        {
          if(C->P3[i] != NULL && C->P3[i]->Enabled == TRUE)
          {
            G->TR->DuplicatesChecking = TRUE;
            break;
          }
        }
      }

      if(G->TR->DuplicatesChecking == TRUE)
      {
        int i;

        InitUIDLhash();

        for(i=0; i < MAXP3; i++)
        {
          if(C->P3[i] != NULL)
            C->P3[i]->UIDLchecked = FALSE;
        }
      }
    }
  }
  else /* Finish previous connection */
  {
    struct POP3 *p = C->P3[G->TR->POP_Nr];

    D(DBF_NET, "downloaded %ld mails from server '%s'", G->TR->Stats.Downloaded, p->Server);
    TR_DisconnectPOP();
    TR_Cleanup();
    AppendToLogfile(LF_ALL, 30, tr(MSG_LOG_Retrieving), G->TR->Stats.Downloaded-laststats, p->User, p->Server);
    if(G->TR->SinglePOP == TRUE)
      pop = MAXP3;
    laststats = G->TR->Stats.Downloaded;
  }

  // what is the next POP3 server we should check
  if(G->TR->SinglePOP == FALSE)
  {
    for(pop = ++G->TR->POP_Nr; pop < MAXP3; pop++)
    {
      if(C->P3[pop] != NULL && C->P3[pop]->Enabled == TRUE)
        break;
    }
  }

  if(pop >= MAXP3) /* Finish last connection */
  {
    // close the TCP/IP connection
    TR_CloseTCPIP();

    // make sure the transfer window is closed
    set(G->TR->GUI.WI, MUIA_Window_Open, FALSE);

    // free/cleanup the UIDL hash tables
    if(G->TR->DuplicatesChecking == TRUE)
      CleanupUIDLhash();

    FreeFilterSearch();
    G->TR->processFilters = FALSE;

    MA_StartMacro(MACRO_POSTGET, itoa((int)G->TR->Stats.Downloaded));

    // tell the appicon that we are finished with checking mail
    // the apply rules or UpdateAppIcon() function will refresh it later on
    G->TR->Checking = FALSE;

    // we only apply the filters if we downloaded something, or it's wasted
    if(G->TR->Stats.Downloaded > 0)
    {
      struct Folder *folder;

      D(DBF_UTIL, "filter %ld/%ld downloaded mails", G->TR->downloadedMails->count, G->TR->Stats.Downloaded);
      FilterMails(FO_GetFolderByType(FT_INCOMING, NULL), G->TR->downloadedMails, APPLY_AUTO);

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

      TR_NewMailAlert();
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
  G->TR->Abort = FALSE;
  G->TR->Pause = FALSE;
  G->TR->Start = FALSE;
  G->Error = FALSE;

  // if the window isn't open we don't need to update it, do we?
  if(isfirst == FALSE && xget(G->TR->GUI.WI, MUIA_Window_Open) == TRUE)
  {
    char str_size_curr[SIZE_SMALL];
    char str_size_curr_max[SIZE_SMALL];

    // reset the statistics display
    snprintf(G->TR->CountLabel, sizeof(G->TR->CountLabel), tr(MSG_TR_MESSAGEGAUGE), 0);
    xset(G->TR->GUI.GA_COUNT, MUIA_Gauge_InfoText, G->TR->CountLabel,
                              MUIA_Gauge_Max,      0,
                              MUIA_Gauge_Current,  0);

    // and last, but not least update the gauge.
    FormatSize(0, str_size_curr, sizeof(str_size_curr), SF_AUTO);
    FormatSize(0, str_size_curr_max, sizeof(str_size_curr_max), SF_AUTO);
    snprintf(G->TR->BytesLabel, sizeof(G->TR->BytesLabel), tr(MSG_TR_TRANSFERSIZE),
                                                           str_size_curr, str_size_curr_max);

    xset(G->TR->GUI.GA_BYTES, MUIA_Gauge_InfoText, G->TR->BytesLabel,
                              MUIA_Gauge_Max,      0,
                              MUIA_Gauge_Current,  0);
  }

  if((msgs = TR_ConnectPOP(G->TR->receiveMode)) != -1)
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

        // do we have to do some remote filter actions?
        if(G->TR->processFilters == TRUE)
        {
          struct Node *curNode;

          set(G->TR->GUI.TX_STATUS, MUIA_Text_Contents, tr(MSG_TR_ApplyFilters));
          IterateList(&tfn->mailTransferList, curNode)
            TR_GetMessageDetails((struct MailTransferNode *)curNode, -2);
        }

        // if the user wants to avoid to receive the
        // same message from the POP3 server again
        // we have to analyze the UIDL of it
        if(G->TR->DuplicatesChecking == TRUE)
        {
          if(FilterDuplicates(tfn) == TRUE)
            C->P3[G->TR->POP_Nr]->UIDLchecked = TRUE;
        }

        // manually initiated transfer
        if(G->TR->receiveMode == POP_USER)
        {
          // preselect messages if preference is "always" or "always, sizes only"
          if(C->PreSelection >= PSM_ALWAYS)
            preselect = TRUE;
          else if(C->WarnSize > 0 && C->PreSelection != PSM_NEVER)
          {
            // ...or any sort of preselection and there is a maximum size

            struct Node *curNode;

            IterateList(&tfn->mailTransferList, curNode)
            {
              struct MailTransferNode *mtn = (struct MailTransferNode *)curNode;
              #if defined(DEBUG)
              struct Mail *mail = mtn->mail;
              #endif

              D(DBF_GUI, "checking mail with subject '%s' and size %ld for preselection", mail->Subject, mail->Size);

              // check the size of those mails only, which are left for download
              if(hasTR_PRESELECT(mtn))
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
          G->TR->GMD_Mail = (struct MinNode *)GetHead((struct List *)&tfn->mailTransferList);
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
      W(DBF_NET, "no messages found on server '%s'", C->P3[G->TR->POP_Nr]->Server);

      // per default we flag that POP3 server as being UIDLchecked
      if(G->TR->DuplicatesChecking == TRUE)
        C->P3[G->TR->POP_Nr]->UIDLchecked = TRUE;
    }
  }
  else
    G->TR->Stats.Error = TRUE;

  BusyEnd();

  TR_GetMailFromNextPOP(FALSE, 0, 0);

  LEAVE();
}
#endif

///
/// TR_SendPOP3KeepAlive()
// Function that sends a STAT command regularly to a POP3 to
// prevent it from dropping the connection.
BOOL TR_SendPOP3KeepAlive(struct TransferNode *tfn)
{
  BOOL result;

  ENTER();

  // here we send a STAT command instead of a NOOP which normally
  // should do the job as well. But there are several known POP3
  // servers out there which are known to ignore the NOOP commands
  // for keepalive message, so STAT should be the better choice.
  result = (TR_SendPOP3Cmd(tfn, POPCMD_STAT, NULL, tr(MSG_ER_BADRESPONSE_POP3)) != NULL);

  RETURN(result);
  return result;
}

///
/// TR_RecvToFile()
// function that receives data from a POP3 server until it receives a \r\n.\r\n termination
// line. It automatically writes that data to the supplied filehandle and if present also
// updates the Transfer status
static int TR_RecvToFile(struct TransferNode *tfn, FILE *fh, const char *filename)
{
  int l=0, read, state=0, count;
  char buf[SIZE_LINE];
  char line[SIZE_LINE];
  BOOL done=FALSE;
  char *lineptr = line;

  ENTER();

  // get the first data the pop server returns after the TOP command
  if((read = count = TR_Recv(tfn, buf, sizeof(buf))) <= 0)
    G->Error = TRUE;

  while(G->Error == FALSE && tfn->abort == FALSE)
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
        if(G->transferWindowObject != NULL)
          DoMethod(G->transferWindowObject, MUIM_TransferWindow_TransStat_Update, l, tr(MSG_TR_Downloading));

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
    if(done == TRUE || G->Error == TRUE || tfn->abort == TRUE)
      break;

    // if not, we get another bunch of data and start over again.
    if((read = TR_Recv(tfn, buf, sizeof(buf))) <= 0)
      break;

    count += read;
  }

  if(done == FALSE)
    count = 0;

  RETURN(count);
  return count;
}

///

/*** UIDL (Avoid duplicates) management ***/
/// InitUIDLhash()
// Initialize the UIDL list and load it from the .uidl file
BOOL InitUIDLhash(void)
{
  BOOL result = FALSE;

  ENTER();

  // make sure no other UIDLhashTable is active
  if(G->uidlHashTable != NULL)
    CleanupUIDLhash();

  // allocate a new hashtable for managing the UIDL data
  if((G->uidlHashTable = HashTableNew(HashTableGetDefaultStringOps(), NULL, sizeof(struct UIDLtoken), 512)) != NULL)
  {
    FILE *fh;
    char *filename = CreateFilename(".uidl");
    LONG size;

    // open the .uidl file and read in the UIDL/MsgIDs
    // line-by-line
    if(ObtainFileInfo(filename, FI_SIZE, &size) == TRUE && size > 0 && (fh = fopen(filename, "r")) != NULL)
    {
      char *uidl = NULL;
      size_t size = 0;

      setvbuf(fh, NULL, _IOFBF, SIZE_FILEBUF);

      while(GetLine(&uidl, &size, fh) >= 0)
        AddUIDLtoHash(uidl, FALSE);

      fclose(fh);

      if(uidl != NULL)
        free(uidl);

      // start with a clean and and so far unmodified hash table
      G->uidlHashIsDirty = FALSE;
    }
    else
      W(DBF_UIDL, "no or empty .uidl file found");

    SHOWVALUE(DBF_UIDL, G->uidlHashTable->entryCount);

    result = TRUE;
  }
  else
    E(DBF_UIDL, "couldn't create new Hashtable for UIDL management");

  RETURN(result);
  return result;
}

///
/// SaveUIDLtoken()
// HashTable callback function to save an UIDLtoken
static enum HashTableOperator SaveUIDLtoken(UNUSED struct HashTable *table,
                                            struct HashEntryHeader *entry,
                                            UNUSED ULONG number,
                                            void *arg)
{
  struct UIDLtoken *token = (struct UIDLtoken *)entry;
  FILE *fh = (FILE *)arg;
  BOOL saveUIDL = FALSE;

  ENTER();

  // before we write out this uidl we have to check wheter this uidl
  // isn't outdated
  if(token->checked == FALSE)
  {
    char *p;

    // now we have to see if this uidl belongs to a POP3 server that
    // wasn't UIDL checked and if so we don't touch it and write it
    // out as well. Otherwise we skip the write operation as we
    // found an orphaned UIDL.
    if((p = strrchr(token->uidl, '@')) != NULL && *(++p) != '\0')
    {
      struct Node *curNode;

      saveUIDL = TRUE;

      IterateList(&C->mailServerList, curNode)
      {
        struct MailServerNode *msn = (struct MailServerNode *)curNode;

        if(msn->type == MST_POP3)
        {
          if(hasServerCheckedUIDL(msn) == FALSE &&
             stricmp(p, msn->hostname) == 0)
          {
            // if we reach here than this uidl is part of
            // a server we didn't check, so we can ignore it
            saveUIDL = FALSE;
            break;
          }
        }
      }
    }
  }
  else
    saveUIDL = TRUE;

  if(saveUIDL == TRUE)
  {
    fprintf(fh, "%s\n", token->uidl);
    D(DBF_UIDL, "saved UIDL '%s' to .uidl file", token->uidl);
  }
  else
    D(DBF_UIDL, "orphaned UIDL found and deleted '%s'", token->uidl);


  RETURN(htoNext);
  return htoNext;
}

///
/// CleanupUIDLhash()
// Cleanup the whole UIDL hash
static void CleanupUIDLhash(void)
{
  ENTER();

  if(G->uidlHashTable != NULL)
  {
    // save the UIDLs only if something has been changed
    if(G->uidlHashIsDirty == TRUE)
    {
      FILE *fh;

      // before we go and destroy the UIDL hash we have to
      // write it to the .uidl file back again.
      if((fh = fopen(CreateFilename(".uidl"), "w")) != NULL)
      {
        setvbuf(fh, NULL, _IOFBF, SIZE_FILEBUF);

        // call HashTableEnumerate with the SaveUIDLtoken callback function
        HashTableEnumerate(G->uidlHashTable, SaveUIDLtoken, fh);

        fclose(fh);
      }
      else
        E(DBF_UIDL, "couldn't open .uidl file for writing");
    }

    // now we can destroy the uidl hash
    HashTableDestroy(G->uidlHashTable);
    G->uidlHashTable = NULL;

    D(DBF_UIDL, "successfully cleaned up UIDLhash");
  }

  // forget any modification to the hash table
  G->uidlHashIsDirty = FALSE;

  LEAVE();
}

///
/// FilterDuplicates()
//
static BOOL FilterDuplicates(struct TransferNode *tfn)
{
  BOOL result = FALSE;

  ENTER();

  // we first make sure the UIDL list is loaded from disk
  if(G->uidlHashTable != NULL)
  {
    // check if there is anything to transfer at all
    if(IsMinListEmpty(&tfn->mailTransferList) == FALSE)
    {
      // inform the user of the operation
      set(G->transferWindowObject, MUIA_TransferWindow_StatusLabel, tr(MSG_TR_CHECKUIDL));

      // before we go and request each UIDL of a message we check wheter the server
      // supports the UIDL command at all
      if(TR_SendPOP3Cmd(tfn, POPCMD_UIDL, NULL, NULL) != NULL)
      {
        char buf[SIZE_LINE];

        // get the first line the pop server returns after the UIDL command
        if(TR_ReadLine(tfn, buf, SIZE_LINE) > 0)
        {
          // we get the "unique-id list" as long as we haven't received a
          // finishing octet
          while(tfn->abort == FALSE && G->Error == FALSE && strncmp(buf, ".\r\n", 3) != 0)
          {
            int num;
            char uidl[SIZE_DEFAULT+SIZE_HOST];
            struct Node *curNode;

            // now parse the line and get the message number and UIDL
            sscanf(buf, "%d %s", &num, uidl);

            // lets add our own ident to the uidl so that we can compare
            // it against our saved list
            strlcat(uidl, "@", sizeof(uidl));
            strlcat(uidl, tfn->msn->hostname, sizeof(uidl));

            // search through our mailTransferList
            IterateList(&tfn->mailTransferList, curNode)
            {
              struct MailTransferNode *mtn = (struct MailTransferNode *)curNode;

              if(mtn->index == num)
              {
                mtn->UIDL = strdup(uidl);

                if(G->uidlHashTable->entryCount > 0 && mtn->UIDL != NULL)
                {
                  struct HashEntryHeader *entry = HashTableOperate(G->uidlHashTable, mtn->UIDL, htoLookup);

                  // see if that hash lookup worked out fine or not.
                  if(HASH_ENTRY_IS_LIVE(entry))
                  {
                    struct UIDLtoken *token = (struct UIDLtoken *)entry;

                    // make sure the mail is flagged as being ignoreable
                    #warning "TODO: should the global DownloadResult be moved to the MailServerNode or TransferNode?"
                    G->LastDL.DupSkipped++;

                    // don't download this mail, because it has been downloaded before
                    CLEAR_FLAG(mtn->tflags, TRF_LOAD);

                    // mark the UIDLtoken as being checked
                    token->checked = TRUE;

                    D(DBF_UIDL, "mail %ld: UIDL '%s' was FOUND!", mtn->index, mtn->UIDL);
                  }
                }

                break;
              }

              if(tfn->abort == TRUE || G->Error == TRUE)
                break;
            }

            // now read the next Line
            if(TR_ReadLine(tfn, buf, SIZE_LINE) <= 0)
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
        struct Node *curNode;

        W(DBF_UIDL, "POP3 server '%s' doesn't support UIDL command!", tfn->msn->hostname);

        // search through our mailTransferList
        IterateList(&tfn->mailTransferList, curNode)
        {
          struct MailTransferNode *mtn = (struct MailTransferNode *)curNode;

          // if the server doesn't support the UIDL command we
          // use the TOP command and generate our own UIDL within
          // the GetMessageDetails function
          TR_GetMessageDetails(mtn, -1);

          // now that we should successfully obtained the UIDL of the
          // mailtransfernode we go and check if that UIDL is already in our UIDLhash
          // and if so we go and flag the mail as a mail that should not be downloaded
          // automatically
          if(G->uidlHashTable->entryCount > 0 && mtn->UIDL != NULL)
          {
            struct HashEntryHeader *entry = HashTableOperate(G->uidlHashTable, mtn->UIDL, htoLookup);

            // see if that hash lookup worked out fine or not.
            if(HASH_ENTRY_IS_LIVE(entry))
            {
              G->LastDL.DupSkipped++;

              // don't download this mail, because it has been downloaded before
              CLEAR_FLAG(mtn->tflags, TRF_LOAD);

              D(DBF_UIDL, "mail %ld: UIDL '%s' was FOUND!", mtn->index, mtn->UIDL);
            }
          }

          if(tfn->abort == TRUE || G->Error == TRUE)
            break;
        }
      }

      result = (tfn->abort == FALSE && G->Error == FALSE);
    }
    else
      result = TRUE;
  }
  else
    E(DBF_UIDL, "uidlHashTable isn't initialized yet!");

  RETURN(result);
  return result;
}

///
/// AddUIDLtoHash()
// adds the UIDL of a mail transfer node to the hash
static void AddUIDLtoHash(const char *uidl, BOOL checked)
{
  struct UIDLtoken *token;

  ENTER();

  if((token = (struct UIDLtoken *)HashTableOperate(G->uidlHashTable, uidl, htoAdd)) != NULL)
  {
    if(token->uidl == NULL)
    {
      token->uidl = strdup(uidl);
      token->checked = checked;

      D(DBF_UIDL, "added UIDL '%s' (%08lx) to hash", uidl, token);
    }
    else
      W(DBF_UIDL, "already existing hash entry for '%s' found, skipping.", uidl);
  }
  else
    E(DBF_UIDL, "couldn't add UIDL '%s' to hash", uidl);

  LEAVE();
}

///
/// RemoveUIDLfromHash()
// removes the UIDL of a mail transfer node from the hash
static void RemoveUIDLfromHash(const char *uidl)
{
  ENTER();

  // signal our hash to remove the entry with the uidl key
  if(HashTableOperate(G->uidlHashTable, uidl, htoRemove) != NULL)
    D(DBF_UIDL, "removed UIDL '%s' from hash", uidl);
  else
    W(DBF_UIDL, "couldn't remove UIDL '%s' from hash", uidl);

  LEAVE();
}

///

/*** IMPORT ***/
/// TR_AddMessageHeader
//  Parses downloaded message header
static struct MailTransferNode *TR_AddMessageHeader(struct TransferNode *tfn, int *count,
                                                    int size, long addr, char *tfname)
{
  struct MailTransferNode *ret = NULL;
  struct ExtendedMail *email;

  ENTER();

  if((email = MA_ExamineMail(NULL, tfname, FALSE)) != NULL)
  {
    struct MailTransferNode *mtn;

    if((mtn = calloc(1, sizeof(struct MailTransferNode))) != NULL)
    {
      struct Mail *mail;

      if((mail = memdup(&email->Mail, sizeof(struct Mail))) != NULL)
      {
        mail->Folder  = NULL;
        mail->Size    = size;

        // flag the mail as being transfered
        mtn->index      = ++(*count);
        mtn->tflags     = TRF_LOAD;
        mtn->mail       = mail;
        mtn->importAddr = addr;
        mtn->tfn        = tfn;

        AddTail((struct List *)&tfn->mailTransferList, (struct Node *)mtn);

        D(DBF_IMPORT, "added mail '%s' (%ld bytes) to import list.", mail->Subject, size);

        ret = mtn;
      }
      else
      {
        free(mtn);
        E(DBF_IMPORT, "Couldn't allocate enough memory for struct Mail");
      }
    }
    else
      E(DBF_IMPORT, "Couldn't allocate enough memory for struct MailTransferNode");

    MA_FreeEMailStruct(email);
  }
  else
    E(DBF_IMPORT, "MA_ExamineMail() returned an error!");

  RETURN(ret);
  return ret;
}

///
/// ReadDBXMessage()
// Extract a certain message from a dbx (Outlook Express) file into
// a separate output file.
static BOOL ReadDBXMessage(FILE *fh, FILE *out, unsigned int addr)
{
  // This can be static as this function is the leave of the recursion
  static unsigned char buf[0x210];
  unsigned int size;
  BOOL result = TRUE;

  ENTER();

  D(DBF_IMPORT, "reading message from addr 0x%08lx", addr);

  while(addr)
  {
    unsigned char *pFirst;
    unsigned char *pLast;
    unsigned char *pCur;
    unsigned char *pPrev;
    unsigned int writeSize;

    // seek to the actual message position
    if(fseek(fh, addr, SEEK_SET))
    {
      result = FALSE;
      break;
    }

    // read out the whole message at once
    if(fread(buf, 1, sizeof(buf), fh) != sizeof(buf))
    {
      result = FALSE;
      break;
    }

    // get the size of the read part and the addr
    // of the next part of the message
    size = writeSize = GetLong(buf, 8);
    addr = GetLong(buf, 12);

    // as *.dbx files are created under Windows they
    // may carry "\r\n" return sequences. But as we are
    // on amiga we do not need and want them, that's why
    // we strip them off
    pFirst = pPrev = &buf[16]; // start of the message part
    pLast = pFirst+size;       // end of the message part

    // we search for '\r' chars and strip them
    for(pCur = pPrev; pCur < pLast; pCur++)
    {
      if(*pCur != '\r')
      {
        if(pCur != pPrev)
          *pPrev = *pCur;

        pPrev++;
      }
      else
        writeSize--;
    }

    // write out the message part at one
    if(writeSize > 0)
    {
      if(fwrite(pFirst, 1, writeSize, out) != writeSize)
      {
        result = FALSE;
        break;
      }
    }
  }

  RETURN(result);
  return result;
}

///
/// ReadDBXMessageInfo()
// reads out the message info of a dbx (Outlook Express) Mail Archive file
static BOOL ReadDBXMessageInfo(FILE *fh, char *outFileName, unsigned int addr, unsigned int size, int *mail_accu, BOOL preview)
{
  BOOL rc = FALSE;
  unsigned char *buf;
  FILE *mailout = NULL;

  unsigned char *data;
  unsigned char *body;
  unsigned int i;
  unsigned int length_of_idxs;
  unsigned int num_of_idxs;
  unsigned int object_marker;
  unsigned char *entries[32];
  unsigned char *msg_entry;
  unsigned int msg_addr = 0;
  unsigned int mailStatusFlags = SFLAG_NONE;

  ENTER();

  if(size < 12)
    size = 12;

  if(!(buf = malloc(size)))
  {
    E(DBF_IMPORT, "Couldn't allocate %ld bytes", size);
    RETURN(FALSE);
    return FALSE;
  }

  // seek to the position where to find the message info object
  if(fseek(fh, addr, SEEK_SET))
  {
    E(DBF_IMPORT, "Unable to seek at %08lx", addr);
    goto out;
  }

  // read in the whole message info object
  if(fread(buf, 1, size, fh) != size)
  {
    E(DBF_IMPORT, "Unable to read %ld bytes", size);
    goto out;
  }

  // check if the object marker matches
  object_marker = GetLong(buf, 0);
  if(object_marker != addr)
  {
    E(DBF_IMPORT, "Object marker didn't match");
    goto out;
  }

  // check the number of indexes
  length_of_idxs = GetLong(buf, 4);
  num_of_idxs = buf[10];
  if(num_of_idxs > sizeof(entries)/sizeof(entries[0]))
  {
    E(DBF_IMPORT, "Too many indexes");
    goto out;
  }

  // check if we have read enough data, if not we must read more
  if(size-12 < length_of_idxs)
  {
    unsigned char *newbuf;

    D(DBF_IMPORT, "read in %ld bytes of object at 0x%08lx, but index length is %ld", size, addr, length_of_idxs);

    if(!(newbuf = malloc(length_of_idxs + 12)))
    {
      E(DBF_IMPORT, "Couldn't allocate %ld bytes", length_of_idxs+12);
      goto out;
    }

    memcpy(newbuf, buf, size);
    if(fread(&newbuf[size], 1, length_of_idxs - size, fh) != length_of_idxs - size)
    {
      E(DBF_IMPORT, "Couldn't load more bytes");

      free(newbuf);
      goto out;
    }

    free(buf);
    buf = newbuf;
  }

  body = buf + 12;
  data = body + num_of_idxs * 4;

  memset(entries, 0, sizeof(entries));

  for(i=0; i<num_of_idxs; i++)
  {
    unsigned int idx = body[0];
    unsigned int addr = body[1] | (body[2] << 8) | (body[3] << 16);

    // check the index value
    if((idx & 0x7f) > sizeof(entries)/sizeof(entries[0]))
    {
      E(DBF_IMPORT, "Wrong index");
      goto out;
    }

    if(idx & 0x80)
    {
      // overwrite the body (and enforce little endian)
      body[0] = addr & 0xff;
      body[1] = (addr & 0xff00) >> 8;
      body[2] = (addr & 0xff0000) >> 16;
      body[3] = 0;

      // is direct value
      entries[idx & 0x7f] = body;
    }
    else
    {
      entries[idx] = data + addr;
    }

    body += 4;
  }

  // Index number 1 points to flags
  if(entries[1])
  {
    unsigned int flags = GetLong(entries[1], 0);

    // check all flags and set the new mail status
    if(flags & (1UL << 5)) // mail has been marked
      SET_FLAG(mailStatusFlags, SFLAG_MARKED);

    if(flags & (1UL << 7)) // mail has been read
      SET_FLAG(mailStatusFlags, SFLAG_READ);

    if(flags & (1UL << 19)) // mail has replied status
      SET_FLAG(mailStatusFlags, SFLAG_REPLIED);
  }

  // Index number 4 points to the whole message
  if(!(msg_entry = entries[4]))
  {
    E(DBF_IMPORT, "Did not find a message");
    goto out;
  }

  // extract the message address
  msg_addr = GetLong(msg_entry, 0);

  // Open the output file
  if((mailout = fopen(outFileName, "wb")) == NULL)
  {
    E(DBF_IMPORT, "Couldn't open %s for output", outFileName);
    goto out;
  }

  // Write the message into the out file */
  if(ReadDBXMessage(fh, mailout, msg_addr) == FALSE)
  {
    E(DBF_IMPORT, "Couldn't read dbx message @ addr %08lx", msg_addr);

    fclose(mailout);
    mailout = NULL;
    DeleteFile(outFileName);
    goto out;
  }

  rc = TRUE;

out:
  if(mailout != NULL)
    fclose(mailout);

  free(buf);

  if(rc)
  {
    if(preview == TRUE)
    {
      LONG size;
      struct MailTransferNode *mtn;

      ObtainFileInfo(outFileName, FI_SIZE, &size);

      // if this is the preview run we go and
      // use the TR_AddMessageHeader method to
      // add the found mail to our mail list
      #warning "TODO"
      //if((mtn = TR_AddMessageHeader(tfn, mail_accu, size, msg_addr, FilePart(outFileName))))
      //  SET_FLAG(mtn->mail->sflags, mailStatusFlags);

      DeleteFile(outFileName);
    }
    else
      ++(*mail_accu);
  }

  RETURN(rc);
  return rc;
}

///
/// ReadDBXNode()
// Function that reads in a node within the tree of a DBX Mail archive
// file from Outlook Express.
static BOOL ReadDBXNode(FILE *fh, char *outFileName, unsigned int addr, int *mail_accu, BOOL preview)
{
  unsigned char *buf;
  unsigned char *body;
  unsigned int child;
  int object_marker;
  int entries;

  ENTER();

  // alloc enough memory to facilitate the the whole tree
  if(!(buf = malloc((0x18+0x264))))
  {
    E(DBF_IMPORT, "Couldn't allocate enough memory for node");
    RETURN(FALSE);
    return FALSE;
  }

  // seek to the start of the tree node
  if(fseek(fh, addr, SEEK_SET))
  {
    free(buf);

    E(DBF_IMPORT, "Unable to seek at %08lx", addr);
    RETURN(FALSE);
    return FALSE;
  }

  // read in the whole tree with one read operation
  if(fread(buf, 1, (0x18+0x264), fh) != (0x18+0x264))
  {
    free(buf);

    E(DBF_IMPORT, "Unable to read %ld bytes", 0x18+0x264);
    RETURN(FALSE);
    return FALSE;
  }

  object_marker = GetLong(buf, 0);
  child = GetLong(buf, 8);
  entries = buf[17];
  body = &buf[0x18];

  while(entries--)
  {
    unsigned int value = GetLong(body, 0);
    unsigned int chld = GetLong(body, 4);
    unsigned int novals = GetLong(body, 8);

    // value points to a pointer to a message
    if(value)
    {
      if(ReadDBXMessageInfo(fh, outFileName, value, novals, mail_accu, preview) == FALSE)
      {
        free(buf);

        E(DBF_IMPORT, "Failed to read the indexed info");
        RETURN(FALSE);
        return FALSE;
      }
    }

    if(chld)
    {
      if(ReadDBXNode(fh, outFileName, chld, mail_accu, preview) == FALSE)
      {
        free(buf);

        E(DBF_IMPORT, "Failed to read node at %p", chld);
        RETURN(FALSE);
        return FALSE;
      }
    }

    body += 12;
  }

  free(buf);

  if(child)
    return ReadDBXNode(fh, outFileName, child, mail_accu, preview);

  RETURN(TRUE);
  return TRUE;
}

///
/// TR_GetMessageList_IMPORT
//  Collects messages from our different supported import formats
BOOL TR_GetMessageList_IMPORT(void)
{
  BOOL result = FALSE;

  ENTER();

  // check if MA_ImportMessages() did correctly set all required
  // data
  if(G->importFile[0] != '\0' && G->importFolder != NULL)
  {
    struct TransferNode *tfn;

    // create a fake TransferNode structure and fill it accordingly
    if((tfn = CreateNewTransfer(NULL)) != NULL)
    {
      char tfname[SIZE_MFILE];
      char fname[SIZE_PATHFILE];
      int c = 0;

      // let the application sleep while we parse the file
      set(G->App, MUIA_Application_Sleep, TRUE);

      // prepare the temporary filename buffers
      snprintf(tfname, sizeof(tfname), "YAMi%08x.tmp", (unsigned int)GetUniqueID());
      AddPath(fname, C->TempDir, tfname, sizeof(fname));

      // before this function is called the MA_ImportMessages() function
      // already found out which import format we can expect. So we
      // distinguish between the different known formats here
      switch(G->importFormat)
      {
        // treat the file as a MBOX compliant file
        case IMF_MBOX:
        {
          FILE *ifh;

          D(DBF_IMPORT, "trying to retrieve mail list from MBOX compliant file");

          if((ifh = fopen(G->importFile, "r")) != NULL)
          {
            FILE *ofh = NULL;
            char *buffer = NULL;
            size_t bufsize = 0;
            BOOL foundBody = FALSE;
            int size = 0;
            long addr = 0;

            setvbuf(ifh, NULL, _IOFBF, SIZE_FILEBUF);

            while(GetLine(&buffer, &bufsize, ifh) >= 0)
            {
              // now we parse through the input file until we
              // find the "From " separator
              if(strncmp(buffer, "From ", 5) == 0)
              {
                // now we know that a new mail has started so if
                // we already found a previous mail we can add it
                // to our list
                if(foundBody == TRUE)
                {
                  D(DBF_IMPORT, "found subsequent 'From ' separator: '%s'", buffer);

                  result = (TR_AddMessageHeader(tfn, &c, size, addr, tfname) != NULL);
                  DeleteFile(fname);

                  if(result == FALSE)
                    break;
                }
                else
                  D(DBF_IMPORT, "found first 'From ' separator: '%s'", buffer);

                // as a new mail is starting we have to
                // open a new file handler
                if((ofh = fopen(fname, "w")) == NULL)
                  break;

                setvbuf(ofh, NULL, _IOFBF, SIZE_FILEBUF);

                size = 0;
                foundBody = FALSE;
                addr = ftell(ifh);

                // continue with the next iteration
                continue;
              }

              // if we already have an opened tempfile
              // and we didn't found the separating mail body
              // yet we go and write out the buffer content
              if(ofh != NULL && foundBody == FALSE)
              {
                fprintf(ofh, "%s\n", buffer);

                // if the buffer is empty we found the corresponding body
                // of the mail and can close the ofh pointer
                if(buffer[0] == '\0')
                {
                  fclose(ofh);
                  ofh = NULL;
                  foundBody = TRUE;

                  D(DBF_IMPORT, "found body part of import mail");
                }
              }

              // to sum the size we count the length of our read buffer
              if(ofh != NULL || foundBody == TRUE)
                size += strlen(buffer)+1;
            }

            // check the reason why we exited the while loop
            if(feof(ifh) == 0)
            {
              E(DBF_IMPORT, "while loop seems to have exited without having scanned until EOF!");
              result = foundBody = FALSE;
            }

            // after quiting the while() loop, we have to check
            // if there is still some data to process
            if(foundBody == TRUE)
            {
              result = (TR_AddMessageHeader(tfn, &c, size, addr, tfname) != NULL);
              DeleteFile(fname);
            }
            else if(ofh != NULL)
            {
              fclose(ofh);
              ofh = NULL;
              DeleteFile(fname);
            }

            fclose(ifh);

            if(buffer != NULL)
              free(buffer);
          }
          else
            E(DBF_IMPORT, "Error on trying to open file '%s'", G->importFile);
        }
        break;

        // treat the file as a file that contains a single
        // unencoded mail (*.eml alike file)
        case IMF_PLAIN:
        {
          if(CopyFile(fname, NULL, G->importFile, NULL) == TRUE)
          {
            LONG size;

            ObtainFileInfo(fname, FI_SIZE, &size);
            // if the file was identified as a plain .eml file we
            // just have to go and call TR_AddMessageHeader to let
            // YAM analyze the file
            result = (TR_AddMessageHeader(tfn, &c, size, 0, tfname) != NULL);

            DeleteFile(fname);
          }
        }
        break;

        // treat the file as a DBX (Outlook Express) compliant mail archive
        case IMF_DBX:
        {
          FILE *ifh;

          // lets open the file and read out the root node of the dbx mail file
          if((ifh = fopen(G->importFile, "rb")) != NULL)
          {
            unsigned char *file_header;

            setvbuf(ifh, NULL, _IOFBF, SIZE_FILEBUF);

            // read the 9404 bytes long file header for properly identifying
            // an Outlook Express database file.
            if((file_header = (unsigned char *)malloc(0x24bc)) != NULL)
            {
              if(fread(file_header, 1, 0x24bc, ifh) == 0x24bc)
              {
                // try to identify the file as a CLSID_MessageDatabase file
                if((file_header[0] == 0xcf && file_header[1] == 0xad &&
                    file_header[2] == 0x12 && file_header[3] == 0xfe) &&
                   (file_header[4] == 0xc5 && file_header[5] == 0xfd &&
                    file_header[6] == 0x74 && file_header[7] == 0x6f))
                {
                  int number_of_mails = GetLong(file_header, 0xc4);
                  unsigned int root_node = GetLong(file_header, 0xe4);

                  D(DBF_IMPORT, "number of mails in dbx file: %ld", number_of_mails);

                  // now we actually start at the root node and read in all messages
                  // accordingly
                  if(ReadDBXNode(ifh, fname, root_node, &c, TRUE) == TRUE && c == number_of_mails)
                    result = TRUE;
                  else
                    E(DBF_IMPORT, "Failed to read from root_node; c=%ld", c);
                }
              }

              free(file_header);
            }

            fclose(ifh);
          }
        }
        break;

        case IMF_UNKNOWN:
          // nothing
        break;
      };

      DoMethod(G->transferWindowObject, MUIM_TransferWindow_DisplayMailList, tfn, FALSE);

      // if everything went fine but we didn't find any mail to import we signal failure
      if(c == 0)
        result = FALSE;

      // wake up the application again
      set(G->App, MUIA_Application_Sleep, FALSE);

      free(tfn->msn);
      free(tfn);
    }
  }

  RETURN(result);
  return result;
}

///
/// TR_AbortIMPORTFunc
//  Aborts import process
HOOKPROTONHNONP(TR_AbortIMPORTFunc, void)
{
#warning "TODO"
//  TR_AbortnClose();
}
MakeHook(TR_AbortIMPORTHook, TR_AbortIMPORTFunc);

///
/// TR_ProcessIMPORTFunc
//  Imports messages from a MBOX mailbox file
HOOKPROTONHNONP(TR_ProcessIMPORTFunc, void)
{
  ENTER();
#warning "TODO"
#if 0
  // if there is nothing to import we can skip
  // immediately.
  if(IsMinListEmpty(&tfn->mailTransferList) == FALSE)
  {
    struct TransStat ts;

    TR_TransStat_Init(&ts);
    if(ts.Msgs_Tot > 0)
    {
      struct Folder *folder = G->TR->ImportFolder;
      enum FolderType ftype = folder->Type;

      TR_TransStat_Start(&ts);

      // now we distinguish between the different import format
      // and import the mails out of it
      switch(G->TR->ImportFormat)
      {
        // treat the file as a MBOX compliant file but also
        // in case of plain (*.eml) file we can that the very same
        // routines.
        case IMF_MBOX:
        case IMF_PLAIN:
        {
          FILE *ifh;

          if((ifh = fopen(G->TR->ImportFile, "r")) != NULL)
          {
            struct Node *curNode;

            setvbuf(ifh, NULL, _IOFBF, SIZE_FILEBUF);

            // iterate through our mailTransferList and seek to
            // each position/address of a mail
            IterateList(&tfn->mailTransferList, curNode)
            {
              struct MailTransferNode *mtn = (struct MailTransferNode *)curNode;
              struct Mail *mail = mtn->mail;
              FILE *ofh = NULL;
              char mfile[SIZE_MFILE];
              char *buffer = NULL;
              size_t size = 0;
              BOOL foundBody = FALSE;
              unsigned int status = SFLAG_NONE;
              unsigned int xstatus = SFLAG_NONE;
              BOOL ownStatusFound = FALSE;

              // if the mail is not flagged as 'loading' we can continue with the next
              // node
              if(hasTR_LOAD(mtn) == FALSE)
                continue;

              // seek to the file position where the mail resist
              if(fseek(ifh, mtn->importAddr, SEEK_SET) != 0)
                break;

              TR_TransStat_NextMsg(&ts, mtn->index, mtn->position, mail->Size, tr(MSG_TR_Importing));

              if((ofh = fopen(MA_NewMailFile(folder, mfile), "w")) == NULL)
                break;

              setvbuf(ofh, NULL, _IOFBF, SIZE_FILEBUF);

              // now that we seeked to the mail address we go
              // and read in line by line
              while(GetLine(&buffer, &size, ifh) >= 0 && G->TR->Abort == FALSE)
              {
                // if we did not find the message body yet
                if(foundBody == FALSE)
                {
                  if(buffer[0] == '\0')
                    foundBody = TRUE; // we found the body part
                  else
                  {
                    // we search for some interesting header lines (i.e. X-Status: etc.)
                    if(strnicmp(buffer, "X-Status: ", 10) == 0)
                    {
                      xstatus = MA_FromXStatusHeader(&buffer[10]);
                      ownStatusFound = TRUE;
                    }
                    else if(strnicmp(buffer, "Status: ", 8) == 0)
                    {
                      status = MA_FromStatusHeader(&buffer[8]);
                      ownStatusFound = TRUE;
                    }
                  }

                  fprintf(ofh, "%s\n", buffer);
                }
                else
                {
                  char *p;

                  // now that we are parsing within the message body we have to
                  // search for new "From " lines as well.
                  if(strncmp(buffer, "From ", 5) == 0)
                    break;

                  // the mboxrd format specifies that we need to unquote any >From, >>From etc. occurance.
                  // http://www.qmail.org/man/man5/mbox.html
                  p = buffer;
                  while(*p == '>')
                    p++;

                  // if we found a quoted line we need to check if there is a following "From " and if so
                  // we have to skip ONE quote.
                  if(p != buffer && strncmp(p, "From ", 5) == 0)
                    fprintf(ofh, "%s\n", &buffer[1]);
                  else
                    fprintf(ofh, "%s\n", buffer);
                }

                // update the transfer statistics
                TR_TransStat_Update(&ts, strlen(buffer)+1, tr(MSG_TR_Importing));
              }

              fclose(ofh);
              ofh = NULL;

              if(buffer != NULL)
                free(buffer);

              // after writing out the mail to a
              // new mail file we go and add it to the folder
              if(ownStatusFound == FALSE)
              {
                // define the default status flags depending on the
                // folder
                if(ftype == FT_OUTGOING)
                  status = SFLAG_QUEUED | SFLAG_READ;
                else if(ftype == FT_SENT || ftype == FT_CUSTOMSENT)
                  status = SFLAG_SENT | SFLAG_READ;
                else
                  status = SFLAG_NEW;
              }
              else
              {
                // Check whether Status and X-Status contained some contradicting flags.
                // The X-Status header line contains no explicit information about the "new"
                // state of a mail, but the Status header line does. Hence we derive this
                // flag from the Status header line only.
                if(isFlagClear(status, SFLAG_NEW) && isFlagSet(xstatus, SFLAG_NEW))
                  CLEAR_FLAG(xstatus, SFLAG_NEW);
              }

              // set the status flags now
              SET_FLAG(mail->sflags, status | xstatus);

              // depending on the Status we have to set the transDate or not
              if(!hasStatusQueued(mail) && !hasStatusHold(mail))
                GetSysTimeUTC(&mail->transDate);

              // add the mail to the folderlist now
              if((mail = AddMailToList(mail, folder)) != NULL)
              {
                // update the mailFile Path
                strlcpy(mail->MailFile, mfile, sizeof(mail->MailFile));

                // if this was a compressed/encrypted folder we need to pack the mail now
                if(folder->Mode > FM_SIMPLE)
                  RepackMailFile(mail, -1, NULL);

                // update the mailfile accordingly.
                MA_UpdateMailFile(mail);

                // put the transferStat to 100%
                TR_TransStat_Update(&ts, TS_SETMAX, tr(MSG_TR_Importing));
              }

              if(G->TR->Abort == TRUE)
                break;
            }

            fclose(ifh);
          }
        }
        break;

        // the file was previously identified as a *.dbx file which
        // was created by Outlook Express.
        case IMF_DBX:
        {
          FILE *ifh;

          if((ifh = fopen(G->TR->ImportFile, "rb")) != NULL)
          {
            struct Node *curNode;

            setvbuf(ifh, NULL, _IOFBF, SIZE_FILEBUF);

            // iterate through our mailTransferList and seek to
            // each position/address of a mail
            IterateList(&tfn->mailTransferList, curNode)
            {
              struct MailTransferNode *mtn = (struct MailTransferNode *)curNode;
              struct Mail *mail = mtn->mail;
              FILE *ofh = NULL;
              char mfile[SIZE_MFILE];

              // if the mail is not flagged as 'loading' we can continue with the next
              // node
              if(hasTR_LOAD(mtn) == FALSE)
                continue;

              // seek to the file position where the mail resist
              if(fseek(ifh, mtn->importAddr, SEEK_SET) != 0)
                break;

              TR_TransStat_NextMsg(&ts, mtn->index, mtn->position, mail->Size, tr(MSG_TR_Importing));

              if((ofh = fopen(MA_NewMailFile(folder, mfile), "wb")) == NULL)
                break;

              setvbuf(ofh, NULL, _IOFBF, SIZE_FILEBUF);

              if(ReadDBXMessage(ifh, ofh, mtn->importAddr) == FALSE)
                E(DBF_IMPORT, "Couldn't import dbx message from addr %08lx", mtn->importAddr);

              fclose(ofh);

              // after writing out the mail to a
              // new mail file we go and add it to the folder
              if(mail->sflags != SFLAG_NONE)
              {
                unsigned int stat = SFLAG_NONE;

                // define the default status flags depending on the
                // folder
                if(ftype == FT_OUTGOING)
                  stat = SFLAG_QUEUED | SFLAG_READ;
                else if(ftype == FT_SENT || ftype == FT_CUSTOMSENT)
                  stat = SFLAG_SENT | SFLAG_READ;
                else
                  stat = SFLAG_NEW;

                SET_FLAG(mail->sflags, stat);
              }

              // depending on the Status we have to set the transDate or not
              if(!hasStatusQueued(mail) && !hasStatusHold(mail))
                GetSysTimeUTC(&mail->transDate);

              // add the mail to the folderlist now
              if((mail = AddMailToList(mail, folder)) != NULL)
              {
                // update the mailFile Path
                strlcpy(mail->MailFile, mfile, sizeof(mail->MailFile));

                // if this was a compressed/encrypted folder we need to pack the mail now
                if(folder->Mode > FM_SIMPLE)
                  RepackMailFile(mail, -1, NULL);

                // update the mailfile accordingly.
                MA_UpdateMailFile(mail);

                // put the transferStat to 100%
                TR_TransStat_Update(&ts, TS_SETMAX, tr(MSG_TR_Importing));
              }

              if(G->TR->Abort == TRUE)
                break;
            }

            fclose(ifh);
          }

          case IMF_UNKNOWN:
            // nothing
          break;
        }
        break;
      }

      TR_TransStat_Finish(&ts);

      AppendToLogfile(LF_ALL, 50, tr(MSG_LOG_Importing), ts.Msgs_Done, G->TR->ImportFile, folder->Name);
      DisplayStatistics(folder, TRUE);
      MA_ChangeFolder(NULL, FALSE);
    }

    TR_AbortnClose();
  }
#endif
  LEAVE();
}
MakeHook(TR_ProcessIMPORTHook, TR_ProcessIMPORTFunc);

///

/*** GET ***/
/// ReceiveMails()
// receives all mails via POP3 from a single MailServer or if msn == NULL
// from all our servers
BOOL ReceiveMails(struct MailServerNode *msn, enum ReceiveMode mode)
{
  BOOL success = FALSE;
  int receivedMails = 0;

  ENTER();

  // if the user specified an explicit mail server we
  // only prepare a transfer for that one and put a TransferNode
  // into our global processing Queue or we walk through our
  // mail server list and process all POP3 servers
  if(msn != NULL)
  {
    struct TransferNode *tfn;

    // now it is time to create a new TransferNode with the link to
    // this mail server
    if((tfn = CreateNewTransfer(msn)) != NULL)
    {
      tfn->mode = mode;

      // add this new transfer to our global list
      AddTail((struct List *)&G->transferQueue, (struct Node *)tfn);

      success = TRUE;
    }
  }
  else
  {
    struct Node *curNode;

    IterateList(&C->mailServerList, curNode)
    {
      msn = (struct MailServerNode *)curNode;
      if(msn->type == MST_POP3 && isServerActive(msn))
      {
        struct TransferNode *tfn;

        // now it is time to create a new TransferNode with the link to
        // this mail server
        if((tfn = CreateNewTransfer(msn)) != NULL)
        {
          tfn->mode = mode;

          // add this new transfer to our global list
          AddTail((struct List *)&G->transferQueue, (struct Node *)tfn);

          success = TRUE;
        }
      }
    }
  }

  if(success == TRUE)
  {
    // start the PREGET macro first
    MA_StartMacro(MACRO_PREGET, itoa(mode));

    ProcessTransferQueue(MST_POP3);
  }

  RETURN(success);
  return success;
}

///
/// ProcessPOP3Transfer
// process a POP3 transfer
BOOL ProcessPOP3Transfer(struct TransferNode *tfn)
{
  BOOL success = FALSE;

  ENTER();

  if(SetupTransferWindow((tfn->mode == RECV_USER ? TR_GET_USER : TR_GET_AUTO)) == TRUE)
  {
    BOOL preselectMode = FALSE;
    int msgs;

    // reset some global variables
    tfn->abort = FALSE;
    tfn->pause = FALSE;
    tfn->start = FALSE;
    G->Error = FALSE;

    // make sure that we mark the new TransferWindow to currently
    // deal with the transferNode
    G->activeTransfer = tfn;

    // signal the AppIcon that we are checking for
    // new mails
    G->mailChecking = TRUE;
    UpdateAppIcon();

    // check if we have to process any remote filters
    tfn->processFilters = (AllocFilterSearch(APPLY_REMOTE) > 0);

    // now we find out if we need to check for duplicates
    // during POP3 processing or if we can skip that.
    tfn->duplicatesChecking = FALSE;
    if(C->AvoidDuplicates == TRUE)
    {
      CLEAR_FLAG(tfn->msn->flags, MSF_UIDLCHECKED);
      tfn->duplicatesChecking = TRUE;
      InitUIDLhash();
    }

    // now we connect to the POP3 server
    if((msgs = TR_ConnectPOP(tfn)) != -1)
    {
      // connection succeeded
      if(msgs > 0)
      {
        // there are messages on the server
        if(TR_GetMessageList_GET(tfn) == TRUE)
        {
          G->LastDL.OnServer += msgs;

          // do we have to do some remote filter actions?
          if(tfn->processFilters == TRUE)
          {
            struct MinNode *curNode;

            set(G->transferWindowObject, MUIA_TransferWindow_StatusLabel, tr(MSG_TR_ApplyFilters));
            for(curNode = tfn->mailTransferList.mlh_Head; curNode->mln_Succ; curNode = curNode->mln_Succ)
              TR_GetMessageDetails((struct MailTransferNode *)curNode, -2);
          }

          // if the user wants to avoid to receive the
          // same message from the POP3 server again
          // we have to analyze the UIDL of it
          if(tfn->duplicatesChecking == TRUE)
          {
            if(FilterDuplicates(tfn) == TRUE)
              SET_FLAG(tfn->msn->flags, MSF_UIDLCHECKED);
          }

          // manually initiated transfer
          if(tfn->mode == RECV_USER)
          {
            // preselect messages if preference is "always" or "always, sizes only"
            if(C->PreSelection >= PSM_ALWAYS)
              preselectMode = TRUE;
            else if(C->WarnSize > 0 && C->PreSelection != PSM_NEVER)
            {
              // ...or any sort of preselection and there is a maximum size
              struct MinNode *curNode;

              for(curNode = tfn->mailTransferList.mlh_Head; curNode->mln_Succ; curNode = curNode->mln_Succ)
              {
                struct MailTransferNode *mtn = (struct MailTransferNode *)curNode;
                #if defined(DEBUG)
                struct Mail *mail = mtn->mail;
                #endif

                D(DBF_GUI, "checking mail with subject '%s' and size %ld for preselection", mail->Subject, mail->Size);

                // check the size of those mails only, which are left for download
                if(hasTR_PRESELECT(mtn))
                {
                  D(DBF_GUI, "mail with subject '%s' and size %ld exceeds size limit", mail->Subject, mail->Size);
                  preselectMode = TRUE;
                  break;
                }
              }
            }
          }

          // anything to preselect?
          if(preselectMode == TRUE)
          {
            // make sure the transfer window is open
            SafeOpenWindow(G->transferWindowObject);

            // if preselect mode is "large only" we display the mail list
            // but make sure to display/add large emails only.
            if(C->PreSelection == PSM_LARGE)
            {
              DoMethod(G->transferWindowObject, MUIM_TransferWindow_DisplayMailList, tfn, TRUE);
              #warning "TODO: MUIA_ShowMe??"
              //set(G->TR->GUI.GR_LIST, MUIA_ShowMe, TRUE);
            }
            else
              DoMethod(G->transferWindowObject, MUIM_TransferWindow_DisplayMailList, tfn, FALSE);

            DoMethod(G->transferWindowObject, MUIM_Window_ScreenToFront);
            DoMethod(G->transferWindowObject, MUIM_Window_ToFront);

            // activate window only if main window active
            set(G->transferWindowObject, MUIA_Window_Activate, xget(G->MA->GUI.WI, MUIA_Window_Activate));

            // make the transferwindow refreshing the mail list
            // details
            DoMethod(G->transferWindowObject, MUIM_TransferWindow_CompleteMailList, &tfn->mailTransferList);
          }
          else
          {
            // we can directly start processing/receiving the
            // mails
            CallHookPkt(&TR_ProcessGETHook, 0, 0);
            success = TRUE;
          }
        }
        else
          E(DBF_NET, "couldn't retrieve MessageList");
      }
      else
      {
        W(DBF_NET, "no messages found on server '%s'", tfn->msn->hostname);

        // per default we flag that POP3 server as being UIDLchecked
        if(tfn->duplicatesChecking == TRUE)
          SET_FLAG(tfn->msn->flags, MSF_UIDLCHECKED);
      }

      if(preselectMode == FALSE)
      {
        // we finished the POP3 transfer so let cleanup everything for the next
        // iteration
        D(DBF_NET, "downloaded %ld mails from server '%s'", G->LastDL.Downloaded, tfn->msn->hostname);
        TR_DisconnectPOP(tfn);
        TR_Cleanup(tfn);

        // output to our logfile
        AppendToLogfile(LF_ALL, 30, tr(MSG_LOG_Retrieving), G->LastDL.Downloaded, tfn->msn->username, tfn->msn->hostname);
      }
    }
  }

  RETURN(success);
  return success;
}

///

#if 0
                  // anything to preselect?
                  if(preselect == TRUE)
                  {
                    // make sure the transfer window is open
                    SafeOpenWindow(G->TR->GUI.WI);

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

                    // activate window only if main window active
                    set(G->TR->GUI.WI, MUIA_Window_Activate, xget(G->MA->GUI.WI, MUIA_Window_Activate));

                    set(G->TR->GUI.GR_PAGE, MUIA_Group_ActivePage, 0);
                    G->TR->GMD_Mail = tfn->mailTransferList.mlh_Head;
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
                W(DBF_NET, "no messages found on server '%s'", msn->hostname);

                // per default we flag that POP3 server as being UIDLchecked
                if(G->TR->DuplicatesChecking == TRUE)
                  SET_FLAG(msn->flags, MSF_UIDLCHECKED);
              }
            }
            else
              G->TR->Stats.Error = TRUE;

            // we finished the POP3 transfer so let cleanup everything for the next
            // iteration
            D(DBF_NET, "downloaded %ld mails from server '%s'", G->TR->Stats.Downloaded, msn->hostname);
            TR_DisconnectPOP(tfn);
            TR_Cleanup();

            // output to our logfile
            #warning "FIXME: check Stats.Downloaded"
            AppendToLogfile(LF_ALL, 30, tr(MSG_LOG_Retrieving), G->TR->Stats.Downloaded, msn->username, msn->hostname);
          }
        }

        if(singleServer == TRUE)
          break;
      }

      // make sure the transfer window is closed
      set(G->TR->GUI.WI, MUIA_Window_Open, FALSE);

      // free/cleanup the UIDL hash tables
      if(G->TR->DuplicatesChecking == TRUE)
        CleanupUIDLhash();

      FreeFilterSearch();
      G->TR->processFilters = FALSE;

      MA_StartMacro(MACRO_POSTGET, itoa((int)G->TR->Stats.Downloaded));

      // tell the appicon that we are finished with checking mail
      // the apply rules or UpdateAppIcon() function will refresh it later on
      G->TR->Checking = FALSE;

      // we only apply the filters if we downloaded something, or it's wasted
      if(G->TR->Stats.Downloaded > 0)
      {
        struct Folder *folder;

        D(DBF_UTIL, "filter %ld/%ld downloaded mails", G->TR->downloadedMails->count, G->TR->Stats.Downloaded);
        FilterMails(FO_GetFolderByType(FT_INCOMING, NULL), G->TR->downloadedMails, APPLY_AUTO);

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

        TR_NewMailAlert();
      }
      else
        UpdateAppIcon();

      // lets populate the LastDL statistics variable with the stats
      // of this download.
      memcpy(&G->LastDL, &G->TR->Stats, sizeof(struct DownloadResult));

      // make sure the transfer window is
      // closed, the transfer list cleanup
      // and everything disposed
      TR_AbortnClose();
    }

    // make sure to close the TCP/IP
    // connection completly
    TR_CloseTCPIP();
  }
  else if(mode == RECV_USER)
    ER_NewError(tr(MSG_ER_OPENTCPIP));

  // start the POSTGET macro so that others
  // notice that the received process finished.
  MA_StartMacro(MACRO_POSTGET, itoa(receivedMails));

  RETURN(success);
  return success;
}
#endif

/// TR_LoadMessage
//  Retrieves a message from the POP3 server
static BOOL TR_LoadMessage(struct TransferNode *tfn, struct Folder *infolder, const int number)
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
    if(TR_SendPOP3Cmd(tfn, POPCMD_RETR, msgnum, tr(MSG_ER_BADRESPONSE_POP3)) != NULL)
    {
      // now we call a subfunction to receive data from the POP3 server
      // and write it in the filehandle as long as there is no termination \r\n.\r\n
      if(TR_RecvToFile(tfn, fh, msgfile) > 0)
        done = TRUE;
    }
    fclose(fh);

    if(tfn->abort == FALSE && G->Error == FALSE && done == TRUE)
    {
      struct ExtendedMail *mail;

      if((mail = MA_ExamineMail(infolder, mfile, FALSE)) != NULL)
      {
        struct Mail *newMail;

        if((newMail = AddMailToList(&mail->Mail, infolder)) != NULL)
        {
          struct MailList *downloadedMails = (struct MailList *)xget(G->transferWindowObject, MUIA_TransferWindow_DownloadedMails);

          // we have to get the actual Time and place it in the transDate, so that we know at
          // which time this mail arrived
          GetSysTimeUTC(&newMail->transDate);

          newMail->sflags = SFLAG_NEW;
          MA_UpdateMailFile(newMail);

          LockMailList(downloadedMails);
          AddNewMailNode(downloadedMails, newMail);
          UnlockMailList(downloadedMails);

          // if the current folder is the inbox we
          // can go and add the mail instantly to the maillist
          if(FO_GetCurrentFolder() == infolder)
            DoMethod(G->MA->GUI.PG_MAILLIST, MUIM_NList_InsertSingle, newMail, MUIV_NList_Insert_Sorted);

          AppendToLogfile(LF_VERBOSE, 32, tr(MSG_LOG_RetrievingVerbose), AddrName(newMail->From), newMail->Subject, newMail->Size);

          MA_StartMacro(MACRO_NEWMSG, GetRealPath(GetMailFile(NULL, infolder, newMail)));
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
static BOOL TR_DeleteMessage(struct TransferNode *tfn, const int number)
{
  BOOL result = FALSE;
  char msgnum[SIZE_SMALL];

  ENTER();

  snprintf(msgnum, sizeof(msgnum), "%d", number);

  DoMethod(G->transferWindowObject, MUIM_TransferWindow_TransStat_Update, TS_SETMAX, tr(MSG_TR_DeletingServerMail));

  if(TR_SendPOP3Cmd(tfn, POPCMD_DELE, msgnum, tr(MSG_ER_BADRESPONSE_POP3)) != NULL)
  {
    G->LastDL.Deleted++;
    result = TRUE;
  }

  RETURN(result);
  return result;
}

///
/// TR_NewMailAlert
//  Notifies user when new mail is available
static void TR_NewMailAlert(struct TransferNode *tfn)
{
  struct DownloadResult *stats = &G->LastDL;
  struct RuleResult *rr = &G->RuleResults;

  ENTER();

  SHOWVALUE(DBF_NET, stats->Downloaded);
  SHOWVALUE(DBF_NET, rr->Spam);

  // show the statistics only if we downloaded some mails at all,
  // and not all of them were spam mails
  if(stats->Downloaded > 0 && stats->Downloaded > rr->Spam)
  {
    if(hasRequesterNotify(C->NotifyType) && tfn->mode != RECV_AUTO_REXX)
    {
      static char buffer[SIZE_LARGE];

      // make sure the application isn't iconified
      if(xget(G->App, MUIA_Application_Iconified) == TRUE)
        PopUp();

      snprintf(buffer, sizeof(buffer), tr(MSG_TR_NewMailReq), stats->Downloaded, stats->OnServer-stats->Deleted, stats->DupSkipped);
      if(C->SpamFilterEnabled == TRUE)
      {
        // include the number of spam classified mails
        snprintf(&buffer[strlen(buffer)], sizeof(buffer)-strlen(buffer), tr(MSG_TR_FILTER_STATS_SPAM),
                                                                         rr->Checked,
                                                                         rr->Bounced,
                                                                         rr->Forwarded,
                                                                         rr->Replied,
                                                                         rr->Executed,
                                                                         rr->Moved,
                                                                         rr->Deleted,
                                                                         rr->Spam);
      }
      else
      {
        snprintf(&buffer[strlen(buffer)], sizeof(buffer)-strlen(buffer), tr(MSG_TR_FilterStats),
                                                                         rr->Checked,
                                                                         rr->Bounced,
                                                                         rr->Forwarded,
                                                                         rr->Replied,
                                                                         rr->Executed,
                                                                         rr->Moved,
                                                                         rr->Deleted);
      }

      // show the info window.
      InfoWindowObject,
        MUIA_Window_Title,     tr(MSG_TR_NewMail),
        MUIA_Window_RefWindow, G->MA->GUI.WI,
        MUIA_Window_Activate,  tfn->mode == RECV_USER,
        MUIA_InfoWindow_Body,  buffer,
      End;
    }

    #if defined(__amigaos4__)
    if(hasOS41SystemNotify(C->NotifyType))
    {
      // Notify() is V53.2+, 53.7 fixes some serious issues
      if(G->applicationID > 0 && LIB_VERSION_IS_AT_LEAST(ApplicationBase, 53, 7))
      {
        // 128 chars is the current maximum :(
        char message[128];

        snprintf(message, sizeof(message), tr(MSG_TR_NEW_MAIL_NOTIFY), stats->Downloaded - rr->Spam);
        Notify(G->applicationID, APPNOTIFY_Title, "YAM",
                                 APPNOTIFY_PubScreenName, "FRONT",
                                 APPNOTIFY_Text, message,
                                 APPNOTIFY_CloseOnDC, TRUE,
                                 APPNOTIFY_BackMsg, "POPUP",
                                 TAG_DONE);
      }
    }
    #endif // __amigaos4__

    if(hasCommandNotify(C->NotifyType))
      ExecuteCommand(C->NotifyCommand, FALSE, OUT_DOS);

    if(hasSoundNotify(C->NotifyType))
      PlaySound(C->NotifySound);
  }

  LEAVE();
}

///
/// TR_ProcessGETFunc
/*** TR_ProcessGETFunc - Downloads messages from a POP3 server ***/
HOOKPROTONHNONP(TR_ProcessGETFunc, void)
{
  #warning "TODO: should we pass tfn from the calling function here instead of the global one?"
  struct TransferNode *tfn = G->activeTransfer;

  ENTER();

  if(tfn != NULL)
  {
    // initialize the transfer statistics
    DoMethod(G->transferWindowObject, MUIM_TransferWindow_TransStat_Init);

    // make sure the NOOP timer is definitly stopped
    StopTimer(TIMER_POP3_KEEPALIVE);

    if(xget(G->transferWindowObject, MUIA_TransferWindow_TransStat_MsgsTot) > 0)
    {
      struct Folder *infolder = FO_GetFolderByType(FT_INCOMING, NULL);
      struct Node *curNode;
      struct MUI_NListtree_TreeNode *incomingTreeNode = FO_GetFolderTreeNode(infolder);

      if(C->TransferWindow == TWM_SHOW && xget(G->transferWindowObject, MUIA_Window_Open) == FALSE)
        set(G->transferWindowObject, MUIA_Window_Open, TRUE);

      DoMethod(G->transferWindowObject, MUIM_TransferWindow_TransStat_Start);

      IterateList(&tfn->mailTransferList, curNode)
      {
        struct MailTransferNode *mtn = (struct MailTransferNode *)curNode;
        struct Mail *mail = mtn->mail;

        D(DBF_NET, "download flags %08lx=%s%s%s for mail with subject '%s' and size %ld",mtn->tflags, hasTR_LOAD(mtn) ? "TR_LOAD " : "" , hasTR_DELETE(mtn) ? "TR_DELETE " : "", hasTR_PRESELECT(mtn) ? "TR_PRESELECT " : "", mail->Subject, mail->Size);
        if(hasTR_LOAD(mtn))
        {
          D(DBF_NET, "downloading mail with subject '%s' and size %ld", mail->Subject, mail->Size);

          // update the transfer status
          DoMethod(G->transferWindowObject, MUIM_TransferWindow_TransStat_NextMsg, mtn->index, mtn->position, mail->Size, tr(MSG_TR_Downloading));

          if(TR_LoadMessage(tfn, infolder, mtn->index) == TRUE)
          {
            // redraw the folderentry in the listtree
            DoMethod(G->MA->GUI.NL_FOLDERS, MUIM_NListtree_Redraw, incomingTreeNode, MUIF_NONE);

            // put the transferStat for this mail to 100%
            DoMethod(G->transferWindowObject, MUIM_TransferWindow_TransStat_Update, TS_SETMAX, tr(MSG_TR_Downloading));

            G->LastDL.Downloaded++;

            if(hasTR_DELETE(mtn))
            {
              D(DBF_NET, "deleting mail with subject '%s' on server", mail->Subject);
              if(TR_DeleteMessage(tfn, mtn->index) == TRUE && tfn->duplicatesChecking == TRUE)
              {
                // remove the UIDL from the hash table and remember that change
                RemoveUIDLfromHash(mtn->UIDL);
                G->uidlHashIsDirty = TRUE;
              }
            }
            else if(tfn->duplicatesChecking == TRUE)
            {
              D(DBF_NET, "adding mail with subject '%s' to UIDL hash", mail->Subject);
              // add the UIDL to the hash table and remember that change
              AddUIDLtoHash(mtn->UIDL, TRUE);
              G->uidlHashIsDirty = TRUE;
            }
            else
              D(DBF_NET, "leaving mail with subject '%s' and size %ld on server to be downloaded again", mail->Subject, mail->Size);
          }
        }
        else if(hasTR_DELETE(mtn))
        {
          D(DBF_NET, "deleting mail with subject '%s' on server", mail->Subject);

          if(TR_DeleteMessage(tfn, mtn->index) == TRUE && tfn->duplicatesChecking == TRUE)
          {
            // remove the UIDL from the hash table and remember that change
            RemoveUIDLfromHash(mtn->UIDL);
            G->uidlHashIsDirty = TRUE;
          }
        }
        else
          D(DBF_NET, "leaving mail with subject '%s' and size %ld on server to be downloaded again", mail->Subject, mail->Size);

        if(tfn->abort == TRUE || G->Error == TRUE)
          break;
      }

      DoMethod(G->transferWindowObject, MUIM_TransferWindow_TransStat_Finish);

      DisplayStatistics(infolder, TRUE);

      // in case the current folder after the download
      // is the incoming folder we have to update
      // the main toolbar and menu items
      if(FO_GetCurrentFolder() == infolder)
        MA_ChangeSelected(TRUE);
    }
  }
  else
    E(DBF_NET, "activeTransfer seems to be NULL");

  LEAVE();
}
MakeHook(TR_ProcessGETHook, TR_ProcessGETFunc);

///
