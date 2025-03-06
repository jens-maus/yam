/***************************************************************************

 YAM - Yet Another Mailer
 Copyright (C) 1995-2000 Marcel Beck
 Copyright (C) 2000-2025 YAM Open Source Team

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
 YAM OpenSource project    :  https://github.com/jens-maus/yam/

***************************************************************************/

#include <ctype.h>
#include <string.h>

#include <clib/alib_protos.h>
#include <proto/exec.h>

#include "extrasrc.h"

#include "YAM.h"
#include "YAM_error.h"
#include "YAM_find.h"
#include "YAM_mainFolder.h"

#include "mime/base64.h"
#include "mime/md5.h"
#include "mui/ClassesExtra.h"
#include "mui/TransferControlGroup.h"
#include "mui/YAMApplication.h"
#include "tcp/Connection.h"
#include "tcp/ssl.h"

#include "Busy.h"
#include "Config.h"
#include "FolderList.h"
#include "Locale.h"
#include "Logfile.h"
#include "MailList.h"
#include "MailServers.h"
#include "MailTransferList.h"
#include "MethodStack.h"
#include "MUIObjects.h"
#include "Threads.h"
#include "UserIdentity.h"

#include "Debug.h"

/**************************************************************************/
// SMTP commands (RFC 821) and extended ESMTP.
// the order of the following enum & pointer array is important and have to
// match each other or weird things will happen.
enum SMTPCommand
{
  // SMTP commands
  SMTP_HELO, SMTP_MAIL, SMTP_RCPT, SMTP_DATA, SMTP_SEND, SMTP_SOML, SMTP_SAML, SMTP_RSET,
  SMTP_VRFY, SMTP_EXPN, SMTP_HELP, SMTP_NOOP, SMTP_QUIT, SMTP_TURN, SMTP_FINISH, SMTP_CONNECT,

  // ESMTP commands
  ESMTP_EHLO, ESMTP_STARTTLS, ESMTP_AUTH_CRAM_MD5, ESMTP_AUTH_DIGEST_MD5, ESMTP_AUTH_LOGIN,
  ESMTP_AUTH_PLAIN
};

static const char *const SMTPcmd[] =
{
  // SMTP commands
  "HELO", "MAIL", "RCPT", "DATA", "SEND", "SOML", "SAML", "RSET",
  "VRFY", "EXPN", "HELP", "NOOP", "QUIT", "TURN", "\r\n.", "",

  // ESMTP commands
  "EHLO", "STARTTLS", "AUTH CRAM-MD5", "AUTH DIGEST-MD5", "AUTH LOGIN",
  "AUTH PLAIN"
};

// SMTP Status Messages
#define SMTP_SERVICE_NOT_AVAILABLE 421
#define SMTP_ACTION_OK             250

// SMTP server capabilities flags & macros
#define SMTP_FLG_ESMTP               (1<<0)
#define SMTP_FLG_AUTH_CRAM_MD5       (1<<1)
#define SMTP_FLG_AUTH_DIGEST_MD5     (1<<2)
#define SMTP_FLG_AUTH_LOGIN          (1<<3)
#define SMTP_FLG_AUTH_PLAIN          (1<<4)
#define SMTP_FLG_STARTTLS            (1<<5)
#define SMTP_FLG_SIZE                (1<<6)
#define SMTP_FLG_PIPELINING          (1<<7)
#define SMTP_FLG_8BITMIME            (1<<8)
#define SMTP_FLG_DSN                 (1<<9)
#define SMTP_FLG_ETRN                (1<<10)
#define SMTP_FLG_ENHANCEDSTATUSCODES (1<<11)
#define SMTP_FLG_DELIVERBY           (1<<12)
#define SMTP_FLG_HELP                (1<<13)
#define hasESMTP(v)                  (isFlagSet((v), SMTP_FLG_ESMTP))
#define hasCRAM_MD5_Auth(v)          (isFlagSet((v), SMTP_FLG_AUTH_CRAM_MD5))
#define hasDIGEST_MD5_Auth(v)        (isFlagSet((v), SMTP_FLG_AUTH_DIGEST_MD5))
#define hasLOGIN_Auth(v)             (isFlagSet((v), SMTP_FLG_AUTH_LOGIN))
#define hasPLAIN_Auth(v)             (isFlagSet((v), SMTP_FLG_AUTH_PLAIN))
#define hasSTARTTLS(v)               (isFlagSet((v), SMTP_FLG_STARTTLS))
#define hasSIZE(v)                   (isFlagSet((v), SMTP_FLG_SIZE))
#define hasPIPELINING(v)             (isFlagSet((v), SMTP_FLG_PIPELINING))
#define has8BITMIME(v)               (isFlagSet((v), SMTP_FLG_8BITMIME))
#define hasDSN(v)                    (isFlagSet((v), SMTP_FLG_DSN))
#define hasETRN(v)                   (isFlagSet((v), SMTP_FLG_ETRN))
#define hasENHANCEDSTATUSCODES(v)    (isFlagSet((v), SMTP_FLG_ENHANCEDSTATUSCODES))
#define hasDELIVERBY(v)              (isFlagSet((v), SMTP_FLG_DELIVERBY))
#define hasHELP(v)                   (isFlagSet((v), SMTP_FLG_HELP))

// help macros for SMTP routines
#define getResponseCode(str)          ((int)strtol((str), NULL, 10))

struct TransferContext
{
  struct Connection *conn;
  struct MailServerNode *msn;
  struct UserIdentityNode *uin;          // ptr to user identity sending mails
  Object *transferGroup;
  struct Folder *outFolder;              // the folder to send mails from
  struct Folder *sentFolder;             // the folder to store sent mails into
  char smtpBuffer[SIZE_LINE];            // RFC 2821 says 1000 should be enough
  char challenge[SIZE_LINE];
  char tempBuffer[SIZE_LINE];
  char transferGroupTitle[SIZE_DEFAULT]; // the TransferControlGroup's title
  BOOL useTLS;
};

/// SendSMTPCommand
//  Sends a command to the SMTP server and returns the response message
//  described in (RFC 2821)
static char *SendSMTPCommand(struct TransferContext *tc, const enum SMTPCommand command, const char *parmtext, const char *errorMsg)
{
  BOOL success = FALSE;
  char *result = tc->smtpBuffer;

  ENTER();

  // first we check if the socket is ready
  // now we prepare the SMTP command
  if(IsStrEmpty(parmtext))
    snprintf(tc->smtpBuffer, sizeof(tc->smtpBuffer), "%s\r\n", SMTPcmd[command]);
  else
    snprintf(tc->smtpBuffer, sizeof(tc->smtpBuffer), "%s %s\r\n", SMTPcmd[command], parmtext);

  D(DBF_NET, "TCP: send SMTP cmd '%s' with param '%s'", SMTPcmd[command], SafeStr(parmtext));

  // lets send the command via TR_WriteLine, but not if we are in connection
  // state
  if(command == SMTP_CONNECT || SendLineToHost(tc->conn, tc->smtpBuffer) > 0)
  {
    int len = 0;

    // after issuing the SMTP command we read out the server response to it
    // but only if this wasn't the SMTP_QUIT command.
    if((len = ReceiveLineFromHost(tc->conn, tc->smtpBuffer, sizeof(tc->smtpBuffer))) > 0)
    {
      // get the response code
      int rc = strtol(tc->smtpBuffer, NULL, 10);

      D(DBF_NET, "received SMTP answer '%s'", tc->smtpBuffer);

      // if the response is a multiline response we have to get out more
      // from the socket
      if(tc->smtpBuffer[3] == '-') // (RFC 2821) - section 4.2.1
      {
        char tbuf[SIZE_LINE];

        // now we concatenate the multiline reply to
        // out main buffer
        do
        {
          // lets get out the next line from the socket
          if((len = ReceiveLineFromHost(tc->conn, tbuf, sizeof(tbuf))) > 0)
          {
            // get the response code
            int rc2 = strtol(tbuf, NULL, 10);

            // check if the response code matches the one
            // of the first line
            if(rc == rc2)
            {
              // lets concatenate both strings while stripping the
              // command code and make sure we didn't reach the end
              // of the buffer
              if(strlcat(tc->smtpBuffer, tbuf, sizeof(tc->smtpBuffer)) >= sizeof(tc->smtpBuffer))
                W(DBF_NET, "buffer overrun on trying to concatenate a multiline reply!");
            }
            else
            {
              E(DBF_NET, "response codes of multiline reply doesn't match!");

              errorMsg = NULL;
              len = 0;
              break;
            }
          }
          else
          {
            errorMsg = tr(MSG_ER_CONNECTIONBROKEN);
            break;
          }
        }
        while(tbuf[3] == '-');
      }

      // check that the concatentation worked
      // out fine and that the rc is valid
      if(len > 0 && rc >= 100)
      {
        // Now we check if we got the correct response code for the command
        // we issued
        switch(command)
        {
          //  Reponse    Description (RFC 2821 - section 4.2.1)
          //  1xx        Positive Preliminary reply
          //  2xx        Positive Completion reply
          //  3xx        Positive Intermediate reply
          //  4xx        Transient Negative Completion reply
          //  5xx        Permanent Negative Completion reply

          case SMTP_HELP:    { success = (rc == 211 || rc == 214); } break;
          case SMTP_VRFY:    { success = (rc == 250 || rc == 251); } break;
          case SMTP_CONNECT: { success = (rc == 220); } break;
          case SMTP_QUIT:    { success = (rc == 221); } break;
          case SMTP_DATA:    { success = (rc == 354); } break;

          // all codes that accept 250 response code
          case SMTP_HELO:
          case SMTP_MAIL:
          case SMTP_RCPT:
          case SMTP_FINISH:
          case SMTP_RSET:
          case SMTP_SEND:
          case SMTP_SOML:
          case SMTP_SAML:
          case SMTP_EXPN:
          case SMTP_NOOP:
          case SMTP_TURN:    { success = (rc == 250); } break;

          // ESMTP commands & response codes
          case ESMTP_EHLO:            { success = (rc == 250); } break;
          case ESMTP_STARTTLS:        { success = (rc == 220); } break;

          // ESMTP_AUTH command responses
          case ESMTP_AUTH_CRAM_MD5:
          case ESMTP_AUTH_DIGEST_MD5:
          case ESMTP_AUTH_LOGIN:
          case ESMTP_AUTH_PLAIN:      { success = (rc == 334); } break;
        }
      }
    }
    else
    {
      // Unfortunately, there are broken SMTP server implementations out there
      // like the one used by "smtp.googlemail.com" or "smtp.gmail.com".
      //
      // It seems these broken SMTP servers do automatically drop the
      // data connection right after the 'QUIT' command was send and don't
      // reply with a status message like it is clearly defined in RFC 2821
      // (section 4.1.1.10). Unfortunately we can't do anything about
      // it really and have to consider this a bad and ugly workaround. :(
      if(command == SMTP_QUIT)
      {
        W(DBF_NET, "broken SMTP server implementation found on QUIT, keeping quiet...");

        success = TRUE;
        tc->smtpBuffer[0] = '\0';
      }
      else
        errorMsg = tr(MSG_ER_CONNECTIONBROKEN);
    }
  }
  else
    errorMsg = tr(MSG_ER_CONNECTIONBROKEN);

  // the rest of the responses throws an error
  if(success == FALSE)
  {
    if(errorMsg != NULL)
      ER_NewError(errorMsg, tc->msn->hostname, (char *)SMTPcmd[command], tc->smtpBuffer);

    result = NULL;
  }

  RETURN(result);
  return result;
}

///
/// ConnectToSMTP
//  Connects to a SMTP mail server - here we always try to do an ESMTP connection
//  first via an EHLO command and then check if it succeeded or not.
static BOOL ConnectToSMTP(struct TransferContext *tc)
{
  BOOL result = FALSE;

  ENTER();

  // If we did a TLS negotitaion previously we have to skip the
  // welcome message, but if it was another connection like a normal or a SSL
  // one we have wait for the welcome
  if(tc->useTLS == FALSE || hasServerSSL(tc->msn))
  {
    PushMethodOnStack(tc->transferGroup, 2, MUIM_TransferControlGroup_ShowStatus, tr(MSG_TR_WaitWelcome));

    result = (SendSMTPCommand(tc, SMTP_CONNECT, NULL, tr(MSG_ER_BADRESPONSE_SMTP)) != NULL);
  }
  else
    result = TRUE;

  // now we either send a HELO (non-ESMTP) or EHLO (ESMTP) command to
  // signal we wanting to start a session accordingly (RFC 1869 - section 4)
  if(result == TRUE)
  {
    ULONG flags = 0;
    char *resp = NULL;
    char hostName[256];

    // before we go on we retrieve the FQDN of the machine we are sending the
    // email from
    GetFQDN(tc->conn, hostName, sizeof(hostName));

    // per default we flag the SMTP to be capable of an ESMTP
    // connection.
    setFlag(flags, SMTP_FLG_ESMTP);

    // set the connection status
    PushMethodOnStack(tc->transferGroup, 2, MUIM_TransferControlGroup_ShowStatus, tr(MSG_TR_SendHello));

    D(DBF_NET, "trying ESMTP negotation");

    // in case we require SMTP-AUTH or a TLS secure connection we
    // have to force an ESMTP connection
    if(hasServerAuth(tc->msn) || hasServerTLS(tc->msn))
      resp = SendSMTPCommand(tc, ESMTP_EHLO, hostName, tr(MSG_ER_BADRESPONSE_SMTP));
    else
    {
      // in all other cases, we first try to get an ESMTP connection
      // and if that doesn't work we go and do a normal SMTP connection
      if((resp = SendSMTPCommand(tc, ESMTP_EHLO, hostName, NULL)) == NULL)
      {
        D(DBF_NET, "ESMTP negotation failed, trying normal SMTP negotation");

        // according to RFC 1869 section 4.7 we send an RSET command
        // between the two EHLO and HELO commands to play save
        SendSMTPCommand(tc, SMTP_RSET, NULL, NULL); // no error code check

        // now we send a HELO command which signals we are not
        // going to use any ESMTP stuff
        resp = SendSMTPCommand(tc, SMTP_HELO, hostName, tr(MSG_ER_BADRESPONSE_SMTP));

        // signal we are not into ESMTP stuff
        clearFlag(flags, SMTP_FLG_ESMTP);
      }
    }

    // check the EHLO/HELO answer.
    if(resp != NULL)
    {
      // check the ESMTP flags if this is an
      // ESMTP connection
      if(hasESMTP(flags))
      {
        // Now lets see what features this ESMTP Server really has
        while(resp[3] == '-')
        {
          // now lets iterate to the next line
          resp = strchr(resp, '\n');

          // if we do not find any new line or the next one would be anyway
          // too short we break here.
          if(resp == NULL || strlen(++resp) < 4)
            break;

          // lets see what features this server returns
          if(strnicmp(resp+4, "STARTTLS", 8) == 0)          // STARTTLS (RFC 2487)
            setFlag(flags, SMTP_FLG_STARTTLS);
          else if(strnicmp(resp+4, "AUTH", 4) == 0)         // SMTP-AUTH (RFC 2554)
          {
            if(NULL != strstr(resp+9,"CRAM-MD5"))
              setFlag(flags, SMTP_FLG_AUTH_CRAM_MD5);

            if(NULL != strstr(resp+9,"DIGEST-MD5"))
              setFlag(flags, SMTP_FLG_AUTH_DIGEST_MD5);

            if(NULL != strstr(resp+9,"PLAIN"))
              setFlag(flags, SMTP_FLG_AUTH_PLAIN);

            if(NULL != strstr(resp+9,"LOGIN"))
              setFlag(flags, SMTP_FLG_AUTH_LOGIN);
          }
          else if(strnicmp(resp+4, "SIZE", 4) == 0)         // STD:10 - SIZE declaration (RFC 1870)
            setFlag(flags, SMTP_FLG_SIZE);
          else if(strnicmp(resp+4, "PIPELINING", 10) == 0)  // STD:60 - PIPELINING (RFC 2920)
            setFlag(flags, SMTP_FLG_PIPELINING);
          else if(strnicmp(resp+4, "8BITMIME", 8) == 0)     // 8BITMIME support (RFC 1652)
            setFlag(flags, SMTP_FLG_8BITMIME);
          else if(strnicmp(resp+4, "DSN", 3) == 0)          // DSN - Delivery Status Notifications (RFC 1891)
            setFlag(flags, SMTP_FLG_DSN);
          else if(strnicmp(resp+4, "ETRN", 4) == 0)         // ETRN - Remote Message Queue Starting (RFC 1985)
            setFlag(flags, SMTP_FLG_ETRN);
          else if(strnicmp(resp+4, "ENHANCEDSTATUSCODES", 19) == 0) // Enhanced Status Codes (RFC 2034)
            setFlag(flags, SMTP_FLG_ENHANCEDSTATUSCODES);
          else if(strnicmp(resp+4, "DELIVERBY", 9) == 0)    // DELIVERBY Extension (RFC 2852)
            setFlag(flags, SMTP_FLG_DELIVERBY);
          else if(strnicmp(resp+4, "HELP", 4) == 0)         // HELP Extension (RFC 821)
            setFlag(flags, SMTP_FLG_HELP);
        }
      }

      #ifdef DEBUG
      D(DBF_NET, "SMTP Server '%s' provides:", tc->msn->hostname);
      D(DBF_NET, "  ESMTP..............: %s", Bool2Txt(hasESMTP(flags)));
      D(DBF_NET, "  AUTH CRAM-MD5......: %s", Bool2Txt(hasCRAM_MD5_Auth(flags)));
      D(DBF_NET, "  AUTH DIGEST-MD5....: %s", Bool2Txt(hasDIGEST_MD5_Auth(flags)));
      D(DBF_NET, "  AUTH LOGIN.........: %s", Bool2Txt(hasLOGIN_Auth(flags)));
      D(DBF_NET, "  AUTH PLAIN.........: %s", Bool2Txt(hasPLAIN_Auth(flags)));
      D(DBF_NET, "  STARTTLS...........: %s", Bool2Txt(hasSTARTTLS(flags)));
      D(DBF_NET, "  SIZE...............: %s", Bool2Txt(hasSIZE(flags)));
      D(DBF_NET, "  PIPELINING.........: %s", Bool2Txt(hasPIPELINING(flags)));
      D(DBF_NET, "  8BITMIME...........: %s", Bool2Txt(has8BITMIME(flags)));
      D(DBF_NET, "  DSN................: %s", Bool2Txt(hasDSN(flags)));
      D(DBF_NET, "  ETRN...............: %s", Bool2Txt(hasETRN(flags)));
      D(DBF_NET, "  ENHANCEDSTATUSCODES: %s", Bool2Txt(hasENHANCEDSTATUSCODES(flags)));
      D(DBF_NET, "  DELIVERBY..........: %s", Bool2Txt(hasDELIVERBY(flags)));
      D(DBF_NET, "  HELP...............: %s", Bool2Txt(hasHELP(flags)));
      #endif

      // now we check the 8BITMIME extension against
      // the user configured Allow8bit setting and if it collides
      // we raise a warning.
      if(has8BITMIME(flags) == FALSE && hasServer8bit(tc->msn) == TRUE)
        result = FALSE;
    }
    else
    {
      result = FALSE;

      W(DBF_NET, "error on SMTP server negotation");
    }

    tc->msn->smtpFlags = flags;
  }
  else
    W(DBF_NET, "SMTP connection failure!");

  RETURN(result);
  return result;
}

///
/// InitSTARTTLS
// function to initiate a TLS connection to the ESMTP server via STARTTLS
static BOOL InitSTARTTLS(struct TransferContext *tc)
{
  BOOL result = FALSE;

  ENTER();

  // If this server doesn't support TLS at all we return with an error
  if(hasSTARTTLS(tc->msn->smtpFlags))
  {
    // If we end up here the server supports STARTTLS and we can start
    // initializing the connection
    PushMethodOnStack(tc->transferGroup, 2, MUIM_TransferControlGroup_ShowStatus, tr(MSG_TR_INITTLS));

    // Now we initiate the STARTTLS command (RFC 2487)
    if(SendSMTPCommand(tc, ESMTP_STARTTLS, NULL, tr(MSG_ER_BADRESPONSE_SMTP)) != NULL)
    {
      // setup the TLS/SSL session
      if(MakeSecureConnection(tc->conn) == TRUE)
      {
        tc->useTLS = TRUE;
        result = TRUE;
      }
      else
      {
        ER_NewError(tr(MSG_ER_INITTLS_SMTP), tc->msn->hostname);
      }
    }
  }
  else
  {
    ER_NewError(tr(MSG_ER_NOSTARTTLS_SMTP), tc->msn->hostname);
  }

  RETURN(result);
  return result;
}

///
/// InitSMTPAUTH
// function to authenticate to a ESMTP Server
static BOOL InitSMTPAUTH(struct TransferContext *tc)
{
  int rc = SMTP_SERVICE_NOT_AVAILABLE;
  char *resp;
  int selectedMethod = MSF_AUTH_AUTO;

  ENTER();

  PushMethodOnStack(tc->transferGroup, 2, MUIM_TransferControlGroup_ShowStatus, tr(MSG_TR_SENDAUTH));

  // first we check if the user has supplied the User&Password
  // and if not we return with an error
  if(IsStrEmpty(tc->msn->username) || IsStrEmpty(tc->msn->password))
  {
    ER_NewError(tr(MSG_ER_NOAUTHUSERPASS));

    RETURN(FALSE);
    return FALSE;
  }

  // now we find out which of the SMTP-AUTH methods we process and which to skip
  // the user explicitly set an auth method. However, we have to
  // check wheter the SMTP server told us that it really
  // supports that method or not
  if(hasServerAuth_AUTO(tc->msn))
  {
    D(DBF_NET, "about to automatically choose which SMTP-AUTH to prefer. smtpFlags=0x%08lx", tc->msn->smtpFlags);

    // we select the most secure one the server supports
    if(hasDIGEST_MD5_Auth(tc->msn->smtpFlags))
      selectedMethod = MSF_AUTH_DIGEST;
    else if(hasCRAM_MD5_Auth(tc->msn->smtpFlags))
      selectedMethod = MSF_AUTH_CRAM;
    else if(hasLOGIN_Auth(tc->msn->smtpFlags))
      selectedMethod = MSF_AUTH_LOGIN;
    else if(hasPLAIN_Auth(tc->msn->smtpFlags))
      selectedMethod = MSF_AUTH_PLAIN;
    else
      W(DBF_NET, "Server doesn't seem to support any SMTP-AUTH method but InitSMTPAUTH function called?");
  }
  else if(hasServerAuth_DIGEST(tc->msn))
  {
    if(hasDIGEST_MD5_Auth(tc->msn->smtpFlags))
      selectedMethod = MSF_AUTH_DIGEST;
    else
      W(DBF_NET, "User selected SMTP-Auth 'DIGEST-MD5', but server doesn't support it!");
  }
  else if(hasServerAuth_CRAM(tc->msn))
  {
    if(hasCRAM_MD5_Auth(tc->msn->smtpFlags))
      selectedMethod = MSF_AUTH_CRAM;
    else
      W(DBF_NET, "User selected SMTP-Auth 'CRAM-MD5', but server doesn't support it!");
  }
  else if(hasServerAuth_LOGIN(tc->msn))
  {
    if(hasLOGIN_Auth(tc->msn->smtpFlags))
      selectedMethod = MSF_AUTH_LOGIN;
    else
      W(DBF_NET, "User selected SMTP-Auth 'LOGIN', but server doesn't support it!");
  }
  else if(hasServerAuth_PLAIN(tc->msn))
  {
    if(hasPLAIN_Auth(tc->msn->smtpFlags))
      selectedMethod = MSF_AUTH_PLAIN;
    else
      W(DBF_NET, "User selected SMTP-Auth 'PLAIN', but server doesn't support it!");
  }

  D(DBF_NET, "SMTP-AUTH method %d choosen due to server/user preference", selectedMethod);

  // now we process the SMTP Authentication by choosing the method the user
  // or the automatic did specify
  switch(selectedMethod)
  {
    // SMTP AUTH DIGEST-MD5 (RFC 2831)
    case MSF_AUTH_DIGEST:
    {
      D(DBF_NET, "processing AUTH DIGEST-MD5:");

      // send the AUTH command and get the response back
      if((resp = SendSMTPCommand(tc, ESMTP_AUTH_DIGEST_MD5, NULL, tr(MSG_ER_BADRESPONSE_SMTP))) != NULL)
      {
        char *realm = NULL;
        char *nonce = NULL;
        char cnonce[16+1];
        char response[32+1];
        char *chalRet;
        char *decResponse = NULL;
        char *enctext = NULL;

        // get the challenge code from the response line of the
        // AUTH command.
        strlcpy(tc->challenge, &resp[4], sizeof(tc->challenge));

        // now that we have the challenge phrase we need to base64decode
        // it, but have to take care to remove the ending "\r\n" cookie.
        chalRet = strpbrk(tc->challenge, "\r\n"); // find the first CR or LF
        if(chalRet != NULL)
          *chalRet = '\0'; // strip it

        D(DBF_NET, "received DIGEST-MD5 challenge: '%s'", tc->challenge);

        // lets base64 decode it
        if(base64decode(&decResponse, tc->challenge, strlen(tc->challenge)) <= 0)
        {
          RETURN(FALSE);
          return FALSE;
        }

        // now copy over decReponse to tc->challenge
        strlcpy(tc->challenge, decResponse, sizeof(tc->challenge));
        free(decResponse);

        D(DBF_NET, "decoded  DIGEST-MD5 challenge: '%s'", tc->challenge);

        // we now analyze the received challenge identifier and pick out
        // the value which we are going to need for our challenge response.
        // This is the refered STEP ONE in the RFC 2831
        {
          char *pstart;
          char *pend;

          // first we try to find out the "realm" of the challenge
          if((pstart = strstr(tc->challenge, "realm=")) != NULL)
          {
            // iterate to the beginning of the realm
            pstart += 6;

            // skip a leading "
            if(*pstart == '"')
              pstart++;

            // find the end of the string
            pend = strpbrk(pstart, "\","); // find a ending " or ,
            if(!pend)
              pend = pstart + strlen(pstart);

            // now copy the found realm into our realm string
            realm = malloc((pend-pstart)+1);
            if(realm)
              strlcpy(realm, pstart, (pend-pstart)+1);
            else
            {
              RETURN(FALSE);
              return FALSE;
            }
          }
          else
          {
            char hostName[256];

            GetFQDN(tc->conn, hostName, sizeof(hostName));

            W(DBF_NET, "'realm' not found in challenge, using '%s' instead", hostName);

            // if the challenge doesn't have a "realm" we assume our
            // choosen SMTP domain to be the realm
            realm = strdup(hostName);
          }

          D(DBF_NET, "realm: '%s'", realm);

          // grab the "nonce" token for later reference
          if((pstart = strstr(tc->challenge, "nonce=")) != NULL)
          {
            // iterate to the beginning of the nonce
            pstart += 6;

            // skip a leading "
            if(*pstart == '"')
              pstart++;

            // find the end of the string
            pend = strpbrk(pstart, "\","); // find a ending " or ,
            if(!pend)
              pend = pstart + strlen(pstart);

            // now copy the found nonce into our nonce string
            nonce = malloc((pend-pstart)+1);
            if(nonce)
              strlcpy(nonce, pstart, (pend-pstart)+1);
            else
            {
              free(realm);

              RETURN(FALSE);
              return FALSE;
            }
          }
          else
          {
            E(DBF_NET, "no 'nonce=' token found!");

            free(realm);

            RETURN(FALSE);
            return FALSE;
          }

          D(DBF_NET, "nonce: '%s'", nonce);

          // now we check the "qop" to carry "auth" so that we are
          // sure that this server really wants an authentification from us
          // RFC 2831 says that it is OK if no qop is present, because this
          // assumes the server to support at least "auth"
          if((pstart = strstr(tc->challenge, "qop=")) != NULL)
          {
            char *qop;

            // iterate to the beginning of the qop=
            pstart += 4;

            // skip a leading "
            if(*pstart == '"')
              pstart++;

            // find the end of the string
            pend = strpbrk(pstart, "\","); // find a ending " or ,
            if(!pend)
              pend = pstart + strlen(pstart);

            // now copy the found qop into our qop string
            qop = malloc((pend-pstart)+1);
            if(qop)
              strlcpy(qop, pstart, (pend-pstart)+1);
            else
            {
              free(realm);
              free(nonce);

              RETURN(FALSE);
              return FALSE;
            }

            // then we check whether we have a plain "auth" within
            // qop or not
            pstart = qop;
            while((pstart = strstr(qop+(pstart-qop), "auth")))
            {
              if(*(pstart+1) != '-')
                break;
            }

            // we don't need the qop string anymore
            free(qop);

            // check if we found a plain auth
            if(!pstart)
            {
              E(DBF_NET, "no 'auth' in 'qop' token found!");

              free(realm);
              free(nonce);

              RETURN(FALSE);
              return FALSE;
            }
          }
        }

        // if we passed here, the server seems to at least support all
        // mechanisms we need for a proper DIGEST-MD5 authentication.
        // so it's time for STEP TWO

        // let us now generate a more or less random and unique cnonce
        // identifier which we can supply to our SMTP server.
        snprintf(cnonce, sizeof(cnonce), "%08x%08x", (unsigned int)rand(), (unsigned int)rand());

        // the we generate the response according to RFC 2831 with A1
        // and A2 as MD5 encoded strings
        {
          unsigned char digest[16]; // 16 octets
          struct MD5Context context;
          char buf[SIZE_LARGE];
          char A1[32+1];
          int  A1_len = 16;         // 16 octects minimum
          char A2[32+1];

          // lets first generate the A1 string
          // A1 = { H( { username-value, ":", realm-value, ":", passwd } ),
          //      ":", nonce-value, ":", cnonce-value }
          snprintf(buf, sizeof(buf), "%s:%s:%s", tc->msn->username, realm, tc->msn->password);
          md5init(&context);
          md5update(&context, buf, strlen(buf));
          md5final(digest, &context);
          memcpy(buf, digest, 16);
          A1_len += snprintf(&buf[16], sizeof(buf)-16, ":%s:%s", nonce, cnonce);
          D(DBF_NET, "unencoded A1: '%s' (%ld)", buf, A1_len);

          // then we directly build the hexadecimal representation
          // HEX(H(A1))
          md5init(&context);
          md5update(&context, buf, A1_len);
          md5final(digest, &context);
          md5digestToHex(digest, A1);
          D(DBF_NET, "encoded   A1: '%s'", A1);


          // then we generate the A2 string accordingly
          // A2 = { "AUTHENTICATE:", digest-uri-value }
          snprintf(buf, sizeof(buf), "AUTHENTICATE:smtp/%s", realm);
          D(DBF_NET, "unencoded A2: '%s'", buf);

          // and also directly build the hexadecimal representation
          // HEX(H(A2))
          md5init(&context);
          md5update(&context, buf, strlen(buf));
          md5final(digest, &context);
          md5digestToHex(digest, A2);
          D(DBF_NET, "encoded   A2: '%s'", A2);

          // now we build the string from which we also build the MD5
          // HEX(H(A1)), ":",
          // nonce-value, ":", nc-value, ":",
          // cnonce-value, ":", qop-value, ":", HEX(H(A2))
          snprintf(buf, sizeof(buf), "%s:%s:00000001:%s:auth:%s", A1, nonce, cnonce, A2);
          D(DBF_NET, "unencoded resp: '%s'", buf);

          // and finally build the respone-value =
          // HEX( KD( HEX(H(A1)), ":",
          //          nonce-value, ":", nc-value, ":",
          //          cnonce-value, ":", qop-value, ":", HEX(H(A2)) }))
          md5init(&context);
          md5update(&context, buf, strlen(buf));
          md5final(digest, &context);
          md5digestToHex(digest, response);
          D(DBF_NET, "encoded   resp: '%s'", response);
        }

        // form up the challenge to authenticate according to RFC 2831
        snprintf(tc->challenge, sizeof(tc->challenge),
                 "username=\"%s\","        // the username token
                 "realm=\"%s\","           // the realm token
                 "nonce=\"%s\","           // the nonce token
                 "cnonce=\"%s\","          // the client nonce (cnonce)
                 "nc=00000001,"            // the nonce count (here always 1)
                 "qop=\"auth\","           // we just use auth
                 "digest-uri=\"smtp/%s\"," // the digest-uri token
                 "response=\"%s\"",        // the response
                 tc->msn->username,
                 realm,
                 nonce,
                 cnonce,
                 realm,
                 response);

        D(DBF_NET, "prepared challenge answer....: '%s'", tc->challenge);
        if(base64encode(&enctext, tc->challenge, strlen(tc->challenge)) > 0)
        {
          strlcpy(tc->tempBuffer, enctext, sizeof(tc->tempBuffer));
          free(enctext);
        }
        else
        {
          RETURN(FALSE);
          return FALSE;
        }
        D(DBF_NET, "encoded  challenge answer....: '%s'", tc->tempBuffer);
        strlcat(tc->tempBuffer, "\r\n", sizeof(tc->tempBuffer));

        // now we send the SMTP AUTH response
        if(SendLineToHost(tc->conn, tc->tempBuffer) > 0)
        {
          // get the server response and see if it was valid
          if(ReceiveLineFromHost(tc->conn, tc->tempBuffer, sizeof(tc->tempBuffer)) <= 0 || (rc = getResponseCode(tc->tempBuffer)) != 334)
            ER_NewError(tr(MSG_ER_BADRESPONSE_SMTP), tc->msn->hostname, (char *)SMTPcmd[ESMTP_AUTH_DIGEST_MD5], tc->tempBuffer);
          else
          {
            // now that we have received the 334 code we just send a plain line
            // to signal that we don't need any option
            if(SendLineToHost(tc->conn, "\r\n") > 0)
            {
              if(ReceiveLineFromHost(tc->conn, tc->tempBuffer, sizeof(tc->tempBuffer)) <= 0 || (rc = getResponseCode(tc->tempBuffer)) != 235)
                ER_NewError(tr(MSG_ER_BADRESPONSE_SMTP), tc->msn->hostname, (char *)SMTPcmd[ESMTP_AUTH_DIGEST_MD5], tc->tempBuffer);
              else
                rc = SMTP_ACTION_OK;
            }
            else
              E(DBF_NET, "couldn't write empty line...");
          }
        }
        else
          E(DBF_NET, "couldn't write empty line...");

        // free all our dynamic buffers
        free(realm);
        free(nonce);
      }
    }
    break;

    // SMTP AUTH CRAM-MD5 (RFC 2195)
    case MSF_AUTH_CRAM:
    {
      D(DBF_NET, "processing AUTH CRAM-MD5:");

      // send the AUTH command and get the response back
      if((resp = SendSMTPCommand(tc, ESMTP_AUTH_CRAM_MD5, NULL, tr(MSG_ER_BADRESPONSE_SMTP))) != NULL)
      {
        ULONG digest[4]; // 16 chars
        char buf[512];
        char *login = tc->msn->username;
        char *password = tc->msn->password;
        char *chalRet;
        char *decResponse = NULL;
        char *enctext = NULL;

        // get the challenge code from the response line of the
        // AUTH command.
        strlcpy(tc->challenge, &resp[4], sizeof(tc->challenge));

        // now that we have the challenge phrase we need to base64decode
        // it, but have to take care to remove the ending "\r\n" cookie.
        chalRet = strpbrk(tc->challenge, "\r\n"); // find the first CR or LF
        if(chalRet)
          *chalRet = '\0'; // strip it

        D(DBF_NET, "received CRAM-MD5 challenge: '%s'", tc->challenge);

        // lets base64 decode it
        if(base64decode(&decResponse, tc->challenge, strlen(tc->challenge)) <= 0)
        {
          RETURN(FALSE);
          return FALSE;
        }

        // now copy over decReponse to tc->challenge
        strlcpy(tc->challenge, decResponse, sizeof(tc->challenge));
        free(decResponse);

        D(DBF_NET, "decoded  CRAM-MD5 challenge: '%s'", tc->challenge);

        // compose the md5 challenge
        md5hmac((unsigned char *)tc->challenge, strlen(tc->challenge), (unsigned char *)password, strlen(password), (unsigned char *)digest);
        snprintf(buf, sizeof(buf), "%s %08x%08x%08x%08x", login, (unsigned int)digest[0], (unsigned int)digest[1],
                                                                 (unsigned int)digest[2], (unsigned int)digest[3]);

        D(DBF_NET, "prepared CRAM-MD5 reponse..: '%s'", buf);
        // lets base64 encode the md5 challenge for the answer
        if(base64encode(&enctext, buf, strlen(buf)) > 0)
        {
          strlcpy(tc->tempBuffer, enctext, sizeof(tc->tempBuffer));
          free(enctext);
        }
        else
        {
          RETURN(FALSE);
          return FALSE;
        }
        D(DBF_NET, "encoded  CRAM-MD5 reponse..: '%s'", tc->tempBuffer);
        strlcat(tc->tempBuffer, "\r\n", sizeof(tc->tempBuffer));

        // now we send the SMTP AUTH response
        if(SendLineToHost(tc->conn, tc->tempBuffer) > 0)
        {
          // get the server response and see if it was valid
          if(ReceiveLineFromHost(tc->conn, tc->tempBuffer, sizeof(tc->tempBuffer)) <= 0 || (rc = getResponseCode(tc->tempBuffer)) != 235)
            ER_NewError(tr(MSG_ER_BADRESPONSE_SMTP), tc->msn->hostname, (char *)SMTPcmd[ESMTP_AUTH_CRAM_MD5], tc->tempBuffer);
          else
            rc = SMTP_ACTION_OK;
        }
      }
    }
    break;

    // SMTP AUTH LOGIN
    case MSF_AUTH_LOGIN:
    {
      D(DBF_NET, "processing AUTH LOGIN:");

      // send the AUTH command
      if((resp = SendSMTPCommand(tc, ESMTP_AUTH_LOGIN, NULL, tr(MSG_ER_BADRESPONSE_SMTP))) != NULL)
      {
        char *enctext = NULL;

        // prepare the username challenge
        D(DBF_NET, "prepared AUTH LOGIN challenge: '%s'", tc->msn->username);
        if(base64encode(&enctext, tc->msn->username, strlen(tc->msn->username)) > 0)
        {
          strlcpy(tc->tempBuffer, enctext, sizeof(tc->tempBuffer));
          free(enctext);
          enctext = NULL;
        }
        else
        {
          RETURN(FALSE);
          return FALSE;
        }
        D(DBF_NET, "encoded  AUTH LOGIN challenge: '%s'", tc->tempBuffer);
        strlcat(tc->tempBuffer, "\r\n", sizeof(tc->tempBuffer));

        // now we send the SMTP AUTH response (UserName)
        if(SendLineToHost(tc->conn, tc->tempBuffer) > 0)
        {
          // get the server response and see if it was valid
          if(ReceiveLineFromHost(tc->conn, tc->tempBuffer, sizeof(tc->tempBuffer)) > 0
             && (rc = getResponseCode(tc->tempBuffer)) == 334)
          {
            // prepare the password challenge
            D(DBF_NET, "prepared AUTH LOGIN challenge: <password>");
            if(base64encode(&enctext, tc->msn->password, strlen(tc->msn->password)) > 0)
            {
              strlcpy(tc->tempBuffer, enctext, sizeof(tc->tempBuffer));
              free(enctext);
              enctext = NULL;
            }
            else
            {
              RETURN(FALSE);
              return FALSE;
            }
            D(DBF_NET, "encoded  AUTH LOGIN challenge: <encoded password>");
            strlcat(tc->tempBuffer, "\r\n", sizeof(tc->tempBuffer));

            // now lets send the Password
            if(SendLineToHost(tc->conn, tc->tempBuffer) > 0)
            {
              // get the server response and see if it was valid
              if(ReceiveLineFromHost(tc->conn, tc->tempBuffer, SIZE_LINE) > 0
                 && (rc = getResponseCode(tc->tempBuffer)) == 235)
              {
                rc = SMTP_ACTION_OK;
              }
            }
          }

          if(rc != SMTP_ACTION_OK)
            ER_NewError(tr(MSG_ER_BADRESPONSE_SMTP), tc->msn->hostname, (char *)SMTPcmd[ESMTP_AUTH_LOGIN], tc->tempBuffer);
        }
      }
    }
    break;

    // SMTP AUTH PLAIN (RFC 2595)
    case MSF_AUTH_PLAIN:
    {
      int len=0;
      char *enctext = NULL;

      D(DBF_NET, "processing AUTH PLAIN:");

      // The AUTH PLAIN command string is a single command string, so we go
      // and prepare the challenge first
      // According to RFC 2595 this string consists of three parts:
      // "[authorize-id] \0 authenticate-id \0 password"
      // where we can left out the first one

      // we don't have a "authorize-id" so we set the first char to \0
      tc->challenge[len++] = '\0';
      len += snprintf(tc->challenge+len, sizeof(tc->challenge)-len, "%s", tc->msn->username)+1; // authenticate-id
      len += snprintf(tc->challenge+len, sizeof(tc->challenge)-len, "%s", tc->msn->password);   // password

      // now we base64 encode this string and send it to the server
      if(base64encode(&enctext, tc->challenge, len) <= 0)
      {
        RETURN(FALSE);
        return FALSE;
      }

      // lets now form up the AUTH PLAIN command we are going to send
      // to the SMTP server for authorization purposes:
      snprintf(tc->challenge, sizeof(tc->challenge), "%s %s\r\n", SMTPcmd[ESMTP_AUTH_PLAIN], enctext);

      // free the encoded text
      free(enctext);

      // now we send the SMTP AUTH command (UserName+Password)
      if(SendLineToHost(tc->conn, tc->challenge) > 0)
      {
        // get the server response and see if it was valid
        if(ReceiveLineFromHost(tc->conn, tc->tempBuffer, sizeof(tc->tempBuffer)) <= 0 || (rc = getResponseCode(tc->tempBuffer)) != 235)
          ER_NewError(tr(MSG_ER_BADRESPONSE_SMTP), tc->msn->hostname, (char *)SMTPcmd[ESMTP_AUTH_PLAIN], tc->tempBuffer);
        else
          rc = SMTP_ACTION_OK;
      }
    }
    break;

    default:
    {
      W(DBF_NET, "The SMTP server seems not to support any of the selected or automatic specified SMTP-AUTH methods");

      // if we don't have any of the Authentication Flags turned on we have to
      // exit with an error
      ER_NewError(tr(MSG_CO_ER_SMTPAUTH), tc->msn->hostname);
    }
    break;
  }

  D(DBF_NET, "Server responded with %ld", rc);

  RETURN((BOOL)(rc == SMTP_ACTION_OK));
  return (BOOL)(rc == SMTP_ACTION_OK);
}

///
/// SendMessage
// Sends a single message (-1 signals an error in DATA phase, 0 signals
// error in start phase, 1 and 2 signals success)
static int SendMessage(struct TransferContext *tc, struct Mail *mail)
{
  int result = 0;
  char mailfile[SIZE_PATHFILE];
  FILE *fh = NULL;
  char *buf = NULL;
  size_t buflen = SIZE_LINE;

  ENTER();

  GetMailFile(mailfile, sizeof(mailfile), tc->outFolder, mail);

  D(DBF_NET, "about to send mail '%s' via SMTP server '%s'", mailfile, tc->msn->hostname);

  // open the mail file for reading
  if((buf = malloc(buflen)) != NULL &&
     (fh = fopen(mailfile, "r")) != NULL)
  {
    setvbuf(fh, NULL, _IOFBF, SIZE_FILEBUF);

    // now we put together our parameters for our MAIL command
    // which in fact may contain serveral parameters as well according
    // to ESMTP extensions.
    snprintf(buf, buflen, "FROM:<%s>", tc->uin->address);

    // in case the server supports the ESMTP SIZE extension lets add the
    // size
    if(hasSIZE(tc->msn->smtpFlags) && mail->Size > 0)
      snprintf(buf, buflen, "%s SIZE=%ld", buf, mail->Size);

    // in case the server supports the ESMTP 8BITMIME extension we can
    // add information about the encoding mode
    if(has8BITMIME(tc->msn->smtpFlags))
      snprintf(buf, buflen, "%s BODY=%s", buf, hasServer8bit(tc->msn) ? "8BITMIME" : "7BIT");

    // send the MAIL command with the FROM: message
    if(SendSMTPCommand(tc, SMTP_MAIL, buf, tr(MSG_ER_BADRESPONSE_SMTP)) != NULL)
    {
      struct ExtendedMail *email = MA_ExamineMail(tc->outFolder, mail->MailFile, TRUE);

      if(email != NULL)
      {
        BOOL rcptok = TRUE;
        int j;

        // if the mail in question has some "Resent-To:" mail
        // header information we use that information instead
        // of the one of the original mail
        if(email->NumResentTo > 0)
        {
          for(j=0; j < email->NumResentTo; j++)
          {
            snprintf(buf, buflen, "TO:<%s>", email->ResentTo[j].Address);
            if(SendSMTPCommand(tc, SMTP_RCPT, buf, tr(MSG_ER_BADRESPONSE_SMTP)) == NULL)
              rcptok = FALSE;
          }
        }
        else
        {
          // specify the main 'To:' recipient
          snprintf(buf, buflen, "TO:<%s>", mail->To.Address);
          if(SendSMTPCommand(tc, SMTP_RCPT, buf, tr(MSG_ER_BADRESPONSE_SMTP)) == NULL)
            rcptok = FALSE;

          // now add the additional 'To:' recipients of the mail
          for(j=0; j < email->NumSTo && rcptok; j++)
          {
            snprintf(buf, buflen, "TO:<%s>", email->STo[j].Address);
            if(SendSMTPCommand(tc, SMTP_RCPT, buf, tr(MSG_ER_BADRESPONSE_SMTP)) == NULL)
              rcptok = FALSE;
          }
        }

        // Add "Resent-CC:" recipients first
        if(email->NumResentCC > 0)
        {
          for(j=0; j < email->NumResentCC; j++)
          {
            snprintf(buf, buflen, "TO:<%s>", email->ResentCC[j].Address);
            if(SendSMTPCommand(tc, SMTP_RCPT, buf, tr(MSG_ER_BADRESPONSE_SMTP)) == NULL)
              rcptok = FALSE;
          }
        }
        else
        {
          // add the 'Cc:' recipients
          for(j=0; j < email->NumCC && rcptok; j++)
          {
            snprintf(buf, buflen, "TO:<%s>", email->CC[j].Address);
            if(SendSMTPCommand(tc, SMTP_RCPT, buf, tr(MSG_ER_BADRESPONSE_SMTP)) == NULL)
              rcptok = FALSE;
          }
        }

        // Add "Resent-BCC" recipients first
        if(email->NumResentBCC > 0)
        {
          for(j=0; j < email->NumResentBCC; j++)
          {
            snprintf(buf, buflen, "TO:<%s>", email->ResentBCC[j].Address);
            if(SendSMTPCommand(tc, SMTP_RCPT, buf, tr(MSG_ER_BADRESPONSE_SMTP)) == NULL)
              rcptok = FALSE;
          }
        }
        else
        {
          // add the 'BCC:' recipients
          for(j=0; j < email->NumBCC && rcptok; j++)
          {
            snprintf(buf, buflen, "TO:<%s>", email->BCC[j].Address);
            if(SendSMTPCommand(tc, SMTP_RCPT, buf, tr(MSG_ER_BADRESPONSE_SMTP)) == NULL)
              rcptok = FALSE;
          }
        }

        if(rcptok == TRUE)
        {
          D(DBF_NET, "RCPTs accepted, sending mail data");

          // now we send the actual main data of the mail
          if(SendSMTPCommand(tc, SMTP_DATA, NULL, tr(MSG_ER_BADRESPONSE_SMTP)) != NULL)
          {
            BOOL lineskip = FALSE;
            BOOL inbody = FALSE;
            ssize_t curlen;
            ssize_t proclen = 0;
            size_t sentbytes = 0;

            // as long there is no abort situation we go on reading out
            // from the stream and sending it to our SMTP server

            while(tc->conn->abort == FALSE && tc->conn->error == CONNECTERR_NO_ERROR &&
                  (curlen = getline(&buf, &buflen, fh)) > 0)
            {
              #if defined(DEBUG)
              if(curlen > 998)
                W(DBF_NET, "RFC2822 violation: line length in source file is too large: %ld", curlen);
              #endif

              // as long as we process header lines we have to make differ in some ways.
              if(inbody == FALSE)
              {
                // we check if we found the body of the mail now
                // the start of a body is seperated by the header with a single
                // empty line and we have to make sure that it isn't the beginning of the file
                if(curlen == 1 && buf[0] == '\n' && proclen > 0)
                {
                  inbody = TRUE;
                  lineskip = FALSE;
                }
                else if(isspace(*buf) == FALSE) // headerlines don't start with a space
                {
                  // we make sure we don't send out BCC:, Resent-BCC: and X-YAM-#? headerlines
                  // because these lines should never be seen by others.
                  if(strnicmp(buf, "bcc", 3) == 0 ||
                     strnicmp(buf, "x-yam-", 6) == 0 ||
                     strnicmp(buf, "resent-bcc", 10) == 0)
                  {
                    lineskip = TRUE;
                  }
                  else
                    lineskip = FALSE;
                }
              }

              // lets save the length we have processed already
              proclen = curlen;

              // if we don't skip this line we write it out to the SMTP server
              if(lineskip == FALSE)
              {
                // RFC 821 says a starting period needs a second one
                // so we send out a period in advance
                if(buf[0] == '.')
                {
                  if(SendToHost(tc->conn, ".", 1, TCPF_NONE) <= 0)
                  {
                    E(DBF_NET, "couldn't send single '.' to SMTP server");

                    ER_NewError(tr(MSG_ER_CONNECTIONBROKEN), tc->msn->hostname, (char *)SMTPcmd[SMTP_DATA]);
                    break;
                  }
                  else
                    sentbytes++;
                }

                // if the last char is a LF we have to skip it right now
                // as we have to send a CRLF per definition of the RFC
                if(buf[curlen-1] == '\n')
                  curlen--;

                // now lets send the data buffered to the socket.
                // we will flush it later then.
                if(curlen > 0 && SendToHost(tc->conn, buf, curlen, TCPF_NONE) <= 0)
                {
                  E(DBF_NET, "couldn't send buffer data to SMTP server (%ld)", curlen);

                  ER_NewError(tr(MSG_ER_CONNECTIONBROKEN), tc->msn->hostname, (char *)SMTPcmd[SMTP_DATA]);
                  break;
                }
                else
                  sentbytes += curlen;

                // now we send the final CRLF (RFC 2822)
                if(SendToHost(tc->conn, "\r\n", 2, TCPF_NONE) <= 0)
                {
                  E(DBF_NET, "couldn't send CRLF to SMTP server");

                  ER_NewError(tr(MSG_ER_CONNECTIONBROKEN), tc->msn->hostname, (char *)SMTPcmd[SMTP_DATA]);
                  break;
                }
                else
                  sentbytes += 2;
              }

              PushMethodOnStack(tc->transferGroup, 3, MUIM_TransferControlGroup_Update, proclen, tr(MSG_TR_Sending));
            }

            D(DBF_NET, "transfered %ld bytes (raw: %ld bytes) error: %ld/%ld", sentbytes, mail->Size, tc->conn->abort, tc->conn->error);

            if(tc->conn->abort == FALSE && tc->conn->error == CONNECTERR_NO_ERROR)
            {
              // check if any of the above getline() operations caused a ferror or
              // if we didn't walk until the end of the mail file
              if(ferror(fh) != 0 || feof(fh) == 0)
              {
                E(DBF_NET, "input mail file returned error state: ferror(fh)=%ld feof(fh)=%ld", ferror(fh), feof(fh));

                ER_NewError(tr(MSG_ER_ErrorReadMailfile), mailfile);
                result = -1; // signal error
              }
              else
              {
                // we have to flush the write buffer if this wasn't a error or
                // abort situation
                SendToHost(tc->conn, NULL, 0, TCPF_FLUSHONLY);

                // send a CRLF+octet "\r\n." to signal that the data is finished.
                // we do it here because if there was an error and we send it, the message
                // will be send incomplete.
                if(SendSMTPCommand(tc, SMTP_FINISH, NULL, tr(MSG_ER_BADRESPONSE_SMTP)) != NULL)
                {
                  // put the transferStat to 100%
                  PushMethodOnStack(tc->transferGroup, 3, MUIM_TransferControlGroup_Update, TCG_SETMAX, tr(MSG_TR_Sending));

                  // now that we are at 100% we have to set the transfer Date of the message
                  GetSysTimeUTC(&mail->transDate);

                  result = email->DelSent ? 2 : 1;
                  AppendToLogfile(LF_VERBOSE, 42, tr(MSG_LOG_SendingVerbose), AddrName(mail->To), mail->Subject, mail->Size);
                }
              }
            }

            if(tc->conn->abort == TRUE || tc->conn->error != CONNECTERR_NO_ERROR)
              result = -1; // signal the caller that we aborted within the DATA part
          }
        }

        MA_FreeEMailStruct(email);
      }
      else
        ER_NewError(tr(MSG_ER_CantOpenFile), mailfile);
    }

    fclose(fh);
  }
  else if(buf != NULL)
    ER_NewError(tr(MSG_ER_CantOpenFile), mailfile);

  free(buf);

  RETURN(result);
  return result;
}

///
/// SendMails
BOOL SendMails(struct UserIdentityNode *uin, struct MailList *mailsToSend, enum SendMailMode mode, const ULONG flags)
{
  BOOL success = FALSE;
  struct TransferContext *tc;
  struct MailServerNode *msn = uin->smtpServer;

  ENTER();

  // make sure the mail server node does not vanish
  ObtainSemaphoreShared(G->configSemaphore);

  if((tc = calloc(1, sizeof(*tc))) != NULL)
  {
    // link the user identity and the mail server in
    // our transfercontext structure
    tc->uin = uin;
    tc->msn = msn;

    tc->outFolder = FO_GetFolderByType(FT_OUTGOING, NULL);

    // depending on the sentfolder settings we store the mail in a
    // different folder. That said the following order of sent folder
    // settings is applied:
    //
    // 1. configured 'Sent' folder in user identity
    // 2. if not set, configured 'Sent' folder in SMTP settings
    // 3. if not set, default 'Sent' folder (first SENT folder found)
    if(tc->uin->sentFolderID == 0 || (tc->sentFolder = FindFolderByID(G->folders, tc->uin->sentFolderID)) == NULL)
    {
      if(tc->msn->mailStoreFolderID == 0 || (tc->sentFolder = FindFolderByID(G->folders, tc->msn->mailStoreFolderID)) == NULL)
        tc->sentFolder = FO_GetFolderByType(FT_SENT, NULL);
    }

    if(tc->sentFolder != NULL)
    {
      // try to open the TCP/IP stack
      if((tc->conn = CreateConnection(TRUE)) != NULL && ConnectionIsOnline(tc->conn) == TRUE)
      {
        struct MailTransferList *transferList;

        // copy a link to the mailservernode for which we created
        // the connection
        tc->conn->server = tc->msn;

        if((transferList = CreateMailTransferList()) != NULL)
        {
          ULONG totalSize = 0;
          struct MailNode *mnode;

          if(isFlagClear(flags, SENDF_TEST_CONNECTION))
          {
            // start the PRESEND macro first and wait for it to terminate
            PushMethodOnStackWait(G->App, 3, MUIM_YAMApplication_StartMacro, MACRO_PRESEND, NULL);

            // now we build the list of mails to be transfered.
            LockMailListShared(mailsToSend);

            ForEachMailNode(mailsToSend, mnode)
            {
              struct Mail *mail = mnode->mail;
              struct MailTransferNode *tnode;

              if((tnode = CreateMailTransferNode(mail, TRF_TRANSFER)) != NULL)
              {
                AddMailTransferNode(transferList, tnode);

                tnode->index = transferList->count;
                totalSize += mail->Size;
              }
            }

            UnlockMailList(mailsToSend);

            D(DBF_NET, "prepared %ld mails for sending, %ld bytes", transferList->count, totalSize);
          }

          // just go on if we really have something to send
          if(isFlagSet(flags, SENDF_TEST_CONNECTION) || transferList->count > 0)
          {
            ULONG twFlags;

            snprintf(tc->transferGroupTitle, sizeof(tc->transferGroupTitle), tr(MSG_TR_MailTransferTo), msn->hostname);

            twFlags = TWF_OPEN;
            if(mode == SENDMAIL_ALL_USER || mode == SENDMAIL_ACTIVE_USER)
              setFlag(twFlags, TWF_ACTIVATE);

            D(DBF_GUI, "create transfer control group");
            if((tc->transferGroup = (Object *)PushMethodOnStackWait(G->App, 5, MUIM_YAMApplication_CreateTransferGroup, CurrentThread(), tc->transferGroupTitle, tc->conn, twFlags)) != NULL)
            {
              struct MinList *sentMailFilters;

              D(DBF_NET, "clone sent mail filters");
              if((sentMailFilters = CloneFilterList(APPLY_SENT)) != NULL)
              {
                struct BusyNode *busy;
                enum ConnectError err;

                PushMethodOnStack(tc->transferGroup, 3, MUIM_TransferControlGroup_Start, transferList->count, totalSize);

                PushMethodOnStack(tc->transferGroup, 2, MUIM_TransferControlGroup_ShowStatus, tr(MSG_TR_Connecting));

                busy = BusyBegin(BUSY_TEXT);
                BusyText(busy, tr(MSG_TR_MailTransferTo), msn->hostname);

                D(DBF_NET, "connecting to host '%s' port %ld", msn->hostname, msn->port);
                if((err = ConnectToHost(tc->conn, tc->msn)) == CONNECTERR_SUCCESS)
                {
                  BOOL connected = FALSE;

                  // first we check whether the user wants to connect to a plain SSLv3 server
                  // so that we initiate the SSL connection now
                  if(hasServerSSL(tc->msn) == TRUE)
                  {
                    // lets try to establish the SSL connection via AmiSSL
                    if(MakeSecureConnection(tc->conn) == TRUE)
                      tc->useTLS = TRUE;
                    else
                      err = CONNECTERR_SSLFAILED; // special SSL connection error
                  }

                  // first we have to check whether the TCP/IP connection could
                  // be successfully opened so that we can init the SMTP connection
                  // and query the SMTP server for its capabilities now.
                  if(err == CONNECTERR_SUCCESS)
                  {
                    // initialize the SMTP connection which will also
                    // query the SMTP server for its capabilities
                    connected = ConnectToSMTP(tc);

                    // Now we have to check whether the user has selected SSL/TLS
                    // and then we have to initiate the STARTTLS command followed by the TLS negotiation
                    if(connected == TRUE && hasServerTLS(tc->msn) == TRUE)
                    {
                      connected = InitSTARTTLS(tc);

                      // then we have to refresh the SMTPflags and check
                      // again what features we have after the STARTTLS
                      if(connected == TRUE)
                      {
                        // first we flag this connection as a sucessfull
                        // TLS session
                        tc->useTLS = TRUE;

                        // now run the connect SMTP function again
                        // so that the SMTP server flags will be refreshed
                        // accordingly.
                        connected = ConnectToSMTP(tc);
                      }
                    }

                    // If the user selected SMTP_AUTH we have to initiate
                    // a AUTH connection
                    if(connected == TRUE && hasServerAuth(tc->msn) == TRUE)
                      connected = InitSMTPAUTH(tc);
                  }

                  // If we are still "connected" we can proceed with transfering the data
                  if(connected == TRUE)
                  {
                    struct MailTransferNode *tn;

                    // set the success to TRUE as everything worked out fine
                    // until here.
                    success = TRUE;
                    AppendToLogfile(LF_VERBOSE, 41, tr(MSG_LOG_ConnectSMTP), msn->hostname);

                    if(isFlagClear(flags, SENDF_TEST_CONNECTION))
                    {
                      ForEachMailTransferNode(transferList, tn)
                      {
                        struct Mail *mail = tn->mail;

                        if(tc->conn->abort == TRUE || tc->conn->error != CONNECTERR_NO_ERROR)
                          break;

                        PushMethodOnStack(tc->transferGroup, 5, MUIM_TransferControlGroup_Next, tn->index, -1, mail->Size, tr(MSG_TR_Sending));

                        switch(SendMessage(tc, mail))
                        {
                          // -1 means that SendMessage was aborted within the
                          // DATA part and so we cannot issue a RSET command and have to abort
                          // immediatly by leaving the mailserver alone.
                          case -1:
                          {
                            setStatusToError(mail);
                            tc->conn->error = CONNECTERR_UNKNOWN_ERROR;
                          }
                          break;

                          // 0 means that an error occured before the DATA part and
                          // so we can abort the transaction cleanly by a RSET and QUIT
                          case 0:
                          {
                            setStatusToError(mail);
                            SendSMTPCommand(tc, SMTP_RSET, NULL, NULL); // no error check
                            tc->conn->error = CONNECTERR_NO_ERROR;
                          }
                          break;

                          // 1 means we filter the mails and then copy/move the mail to the send folder
                          case 1:
                          {
                            setStatusToSent(mail);
                            if(PushMethodOnStackWait(G->App, 3, MUIM_YAMApplication_FilterMail, sentMailFilters, mail) == TRUE)
                            {
                              // the filter process did not move the mail, hence we do it now
                              PushMethodOnStackWait(G->App, 5, MUIM_YAMApplication_MoveCopyMail, mail, tc->sentFolder, "sent mail", MVCPF_CLOSE_WINDOWS);
                            }
                            else
                            {
                              // update the Outgoing folder's stats as the mail just got (re)moved
                              PushMethodOnStack(G->App, 3, MUIM_YAMApplication_DisplayStatistics, tc->outFolder, TRUE);
                            }
                          }
                          break;

                          // 2 means we filter and delete afterwards
                          case 2:
                          {
                            setStatusToSent(mail);
                            if(PushMethodOnStackWait(G->App, 3, MUIM_YAMApplication_FilterMail, sentMailFilters, mail) == TRUE)
                            {
                              // the filter process did not delete the mail, hence we do it now
                              PushMethodOnStackWait(G->App, 3, MUIM_YAMApplication_DeleteMail, mail, DELF_UPDATE_APPICON);
                            }
                          }
                          break;
                        }
                      }
                    }

                    PushMethodOnStack(tc->transferGroup, 1, MUIM_TransferControlGroup_Finish);

                    if(tc->conn->error == CONNECTERR_NO_ERROR)
                      AppendToLogfile(LF_NORMAL, 40, tr(MSG_LOG_Sending), transferList->count, msn->hostname);
                    else
                      AppendToLogfile(LF_NORMAL, 40, tr(MSG_LOG_SENDING_FAILED), transferList->count, msn->hostname);

                    // now we can disconnect from the SMTP
                    // server again
                    PushMethodOnStack(tc->transferGroup, 2, MUIM_TransferControlGroup_ShowStatus, tr(MSG_TR_Disconnecting));

                    // send a 'QUIT' command, but only if
                    // we didn't receive any error during the transfer
                    if(tc->conn->error == CONNECTERR_NO_ERROR)
                      SendSMTPCommand(tc, SMTP_QUIT, NULL, tr(MSG_ER_BADRESPONSE_SMTP));
                  }
                  else
                  {
                    // check if we end up here cause of the 8BITMIME differences
                    if(has8BITMIME(tc->msn->smtpFlags) == FALSE && hasServer8bit(tc->msn) == TRUE)
                    {
                      W(DBF_NET, "incorrect Allow8bit setting!");
                      err = CONNECTERR_INVALID8BIT;
                    }
                    else if(err != CONNECTERR_SSLFAILED)
                      err = CONNECTERR_UNKNOWN_ERROR;
                  }
                }

                // make sure to shutdown the socket and all possible SSL connection stuff
                DisconnectFromHost(tc->conn);

                // if we got an error here, let's throw it
                switch(err)
                {
                  case CONNECTERR_SUCCESS:
                  case CONNECTERR_ABORTED:
                  case CONNECTERR_NO_ERROR:
                    // do nothing
                  break;

                  // a socket is already in use so we return
                  // a specific error to the user
                  case CONNECTERR_SOCKET_IN_USE:
                    ER_NewError(tr(MSG_ER_CONNECTERR_SOCKET_IN_USE_SMTP), msn->hostname);
                  break;

                  // socket() execution failed
                  case CONNECTERR_NO_SOCKET:
                    ER_NewError(tr(MSG_ER_CONNECTERR_NO_SOCKET_SMTP), msn->hostname);
                  break;

                  // couldn't establish non-blocking IO
                  case CONNECTERR_NO_NONBLOCKIO:
                    ER_NewError(tr(MSG_ER_CONNECTERR_NO_NONBLOCKIO_SMTP), msn->hostname);
                  break;

                  // the specified hostname isn't valid, so
                  // lets tell the user
                  case CONNECTERR_UNKNOWN_HOST:
                    ER_NewError(tr(MSG_ER_UNKNOWN_HOST_SMTP), msn->hostname);
                  break;

                  // the connection request timed out, so tell
                  // the user
                  case CONNECTERR_TIMEDOUT:
                    ER_NewError(tr(MSG_ER_CONNECTERR_TIMEDOUT_SMTP), msn->hostname);
                  break;

                  // an error occurred while checking for 8bit MIME
                  // compatibility
                  case CONNECTERR_INVALID8BIT:
                    ER_NewError(tr(MSG_ER_NO8BITMIME_SMTP), msn->hostname);
                  break;

                  // error during initialization of an SSL connection
                  case CONNECTERR_SSLFAILED:
                    ER_NewError(tr(MSG_ER_INITTLS_SMTP), msn->hostname);
                  break;

                  // an unknown error occurred so lets show
                  // a generic error message
                  case CONNECTERR_UNKNOWN_ERROR:
                    ER_NewError(tr(MSG_ER_CANNOT_CONNECT_SMTP), msn->hostname);
                  break;

                  case CONNECTERR_NO_CONNECTION:
                  case CONNECTERR_NOT_CONNECTED:
                    // cannot happen, do nothing
                  break;
                }

                BusyEnd(busy);

                DeleteFilterList(sentMailFilters);
              }
            }

            PushMethodOnStack(G->App, 2, MUIM_YAMApplication_DeleteTransferGroup, tc->transferGroup);
          }

          // remove any mail transfer nodes from the list and delete the list
          DeleteMailTransferList(transferList);

          // start the POSTSEND macro so that others notice that the
          // send process has finished. No need to wait for termination.
          PushMethodOnStack(G->App, 3, MUIM_YAMApplication_StartMacro, MACRO_POSTSEND, NULL);
        }
      }

      DeleteConnection(tc->conn);
    }
    else
    {
      E(DBF_FOLDER, "could not resolve sent folder of user identity '%s and SMTP server '%s'", tc->uin->description, tc->msn->description);
    }

    free(tc);
  }

  CleanMailsInTransfer(mailsToSend);
  // delete the list of mails
  DeleteMailList(mailsToSend);

  // mark the server as being no longer "in use"
  LockMailServer(msn);
  msn->useCount--;
  UnlockMailServer(msn);

  // now we are done
  ReleaseSemaphore(G->configSemaphore);

  // wake up the calling thread if this is requested
  if(isFlagSet(flags, SENDF_SIGNAL))
    WakeupThread(NULL);

  RETURN(success);
  return success;
}

///
/// CleanMailsInTransfer
// remove all mails in a list from the global list of mails in transfer
void CleanMailsInTransfer(const struct MailList *mlist)
{
  struct MailNode *mnode;

  ENTER();

  LockMailList(G->mailsInTransfer);
  ForEachMailNode(mlist, mnode)
  {
    struct MailNode *mnode2;

    if((mnode2 = FindMailByAddress(G->mailsInTransfer, mnode->mail)) != NULL)
    {
      RemoveMailNode(G->mailsInTransfer, mnode2);
      DeleteMailNode(mnode2);
    }
  }
  UnlockMailList(G->mailsInTransfer);

  LEAVE();
}

///
