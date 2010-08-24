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

#include <ctype.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

#if defined(__AROS__)
#include <sys/types.h>
#else
#include <sys/filio.h>
#endif

#include <netinet/ip.h>
#include <netinet/tcp.h>

#include <clib/alib_protos.h>
#include <clib/macros.h>
#include <libraries/iffparse.h>
#include <libraries/genesis.h>
#include <mui/NList_mcc.h>
#include <mui/NListview_mcc.h>
#include <mui/NListtree_mcc.h>
#include <proto/amissl.h>
#if defined(__amigaos4__)
#include <proto/application.h>
#endif
#include <proto/dos.h>
#include <proto/exec.h>
#include <proto/intuition.h>
#include <proto/muimaster.h>
#include <proto/timer.h>
#include <proto/utility.h>

#if !defined(__amigaos4__) && !defined(__AROS__)
#include <proto/miami.h>
#include <proto/genesis.h>
#endif

#include "extrasrc.h"

#include "SDI_hook.h"

#include "YAM.h"
#include "YAM_addressbookEntry.h"
#include "YAM_config.h"
#include "YAM_error.h"
#include "YAM_find.h"
#include "YAM_folderconfig.h"
#include "YAM_global.h"
#include "YAM_main.h"
#include "YAM_mainFolder.h"
#include "YAM_utilities.h"

#include "mui/Classes.h"
#include "mime/base64.h"
#include "mime/md5.h"

#include "AppIcon.h"
#include "HashTable.h"
#include "FileInfo.h"
#include "Locale.h"
#include "MailList.h"
#include "MailServers.h"
#include "MUIObjects.h"
#include "Requesters.h"

#include "tcp/Connection.h"

#include "Debug.h"

struct TransStat
{
  int   Msgs_Tot;
  int   Msgs_Done;
  int   Msgs_Curr;
  int   Msgs_ListPos;
  ULONG Size_Tot;
  ULONG Size_Done;
  ULONG Size_Curr;
  ULONG Size_Curr_Max;
  ULONG Clock_Start;
  struct TimeVal Clock_Last;
  char str_size_tot[SIZE_SMALL];
  char str_size_done[SIZE_SMALL];
  char str_size_curr[SIZE_SMALL];
  char str_size_curr_max[SIZE_SMALL];
  char str_speed[SIZE_SMALL];
};

#define TS_SETMAX   (-1)

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
static void TR_NewMailAlert(void);
static void TR_CompleteMsgList(void);
static char *TR_SendSMTPCmd(const enum SMTPCommand command, const char *parmtext, const char *errorMsg);
static void TR_TransStat_Update(struct TransStat *ts, int size_incr, const char *status);

// static UIDL function prototypes
static BOOL InitUIDLhash(void);
static void CleanupUIDLhash(void);
static BOOL FilterDuplicates(void);
static void AddUIDLtoHash(const char *uidl, BOOL checked);

struct UIDLtoken
{
  struct HashEntryHeader hash;
  const char *uidl;
  BOOL checked;
};

/**************************************************************************/
// local macros & defines
#define GetLong(p,o)  ((((unsigned char*)(p))[o]) | (((unsigned char*)(p))[o+1]<<8) | (((unsigned char*)(p))[o+2]<<16) | (((unsigned char*)(p))[o+3]<<24))

/***************************************************************************
 Module: Transfer
***************************************************************************/

/*** TLS/SSL routines ***/
/// TR_InitSTARTTLS()
// function to initiate a TLS connection to the ESMTP server via STARTTLS
static BOOL TR_InitSTARTTLS(void)
{
  BOOL result = FALSE;
  struct MailServerNode *msn;

  ENTER();

#warning FIXME: replace GetMailServer() usage when struct Connection is there
  if((msn = GetMailServer(&C->mailServerList, MST_SMTP, 0)) == NULL)
  {
    RETURN(FALSE);
    return FALSE;
  }

  // If this server doesn't support TLS at all we return with an error
  if(hasSTARTTLS(msn->smtpFlags))
  {
    // If we end up here the server supports STARTTLS and we can start
    // initializing the connection
    set(G->TR->GUI.TX_STATUS, MUIA_Text_Contents, tr(MSG_TR_INITTLS));

    // Now we initiate the STARTTLS command (RFC 2487)
    if(TR_SendSMTPCmd(ESMTP_STARTTLS, NULL, tr(MSG_ER_BADRESPONSE_SMTP)) != NULL)
    {
      // setup the TLS/SSL session
      if(MakeSecureConnection(G->TR->connection) == TRUE)
      {
        G->TR_UseTLS = TRUE;
        result = TRUE;
      }
      else
        ER_NewError(tr(MSG_ER_INITTLS_SMTP), msn->hostname);
    }
  }
  else
    ER_NewError(tr(MSG_ER_NOSTARTTLS_SMTP), msn->hostname);

  RETURN(result);
  return result;
}

///

/*** SMTP-AUTH routines ***/
/// TR_InitSMTPAUTH()
// function to authenticate to a ESMTP Server
static BOOL TR_InitSMTPAUTH(void)
{
  int rc = SMTP_SERVICE_NOT_AVAILABLE;
  char *resp;
  char buffer[SIZE_LINE];
  char challenge[SIZE_LINE];
  int selectedMethod = MSF_AUTH_AUTO;
  struct MailServerNode *msn;

  ENTER();

  set(G->TR->GUI.TX_STATUS, MUIA_Text_Contents, tr(MSG_TR_SENDAUTH));

#warning FIXME: replace GetMailServer() usage when struct Connection is there
  if((msn = GetMailServer(&C->mailServerList, MST_SMTP, 0)) == NULL)
  {
    RETURN(FALSE);
    return FALSE;
  }

  // first we check if the user has supplied the User&Password
  // and if not we return with an error
  if(msn->username[0] == '\0' || msn->password[0] == '\0')
  {
    ER_NewError(tr(MSG_ER_NOAUTHUSERPASS));

    RETURN(FALSE);
    return FALSE;
  }

  // now we find out which of the SMTP-AUTH methods we process and which to skip
  // the user explicitly set an auth method. However, we have to
  // check wheter the SMTP server told us that it really
  // supports that method or not
  if(hasServerAuth_AUTO(msn))
  {
    D(DBF_NET, "about to automatically choose which SMTP-AUTH to prefer. smtpFlags=0x%08lx", msn->smtpFlags);

    // we select the most secure one the server supports
    if(hasDIGEST_MD5_Auth(msn->smtpFlags))
      selectedMethod = MSF_AUTH_DIGEST;
    else if(hasCRAM_MD5_Auth(msn->smtpFlags))
      selectedMethod = MSF_AUTH_CRAM;
    else if(hasLOGIN_Auth(msn->smtpFlags))
      selectedMethod = MSF_AUTH_LOGIN;
    else if(hasPLAIN_Auth(msn->smtpFlags))
      selectedMethod = MSF_AUTH_PLAIN;
    else
      W(DBF_NET, "Server doesn't seem to support any SMTP-AUTH method but InitSMTPAUTH function called?");
  }
  else if(hasServerAuth_DIGEST(msn))
  {
    if(hasDIGEST_MD5_Auth(msn->smtpFlags))
      selectedMethod = MSF_AUTH_DIGEST;
    else
      W(DBF_NET, "User selected SMTP-Auth 'DIGEST-MD5', but server doesn't support it!");
  }
  else if(hasServerAuth_CRAM(msn))
  {
    if(hasCRAM_MD5_Auth(msn->smtpFlags))
      selectedMethod = MSF_AUTH_CRAM;
    else
      W(DBF_NET, "User selected SMTP-Auth 'CRAM-MD5', but server doesn't support it!");
  }
  else if(hasServerAuth_LOGIN(msn))
  {
    if(hasLOGIN_Auth(msn->smtpFlags))
      selectedMethod = MSF_AUTH_LOGIN;
    else
      W(DBF_NET, "User selected SMTP-Auth 'LOGIN', but server doesn't support it!");
  }
  else if(hasServerAuth_PLAIN(msn))
  {
    if(hasPLAIN_Auth(msn->smtpFlags))
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
      if((resp = TR_SendSMTPCmd(ESMTP_AUTH_DIGEST_MD5, NULL, tr(MSG_ER_BADRESPONSE_SMTP))) != NULL)
      {
        char *realm = NULL;
        char *nonce = NULL;
        char cnonce[16+1];
        char response[32+1];
        char *chalRet;

        // get the challenge code from the response line of the
        // AUTH command.
        strlcpy(challenge, &resp[4], sizeof(challenge));

        // now that we have the challenge phrase we need to base64decode
        // it, but have to take care to remove the ending "\r\n" cookie.
        chalRet = strpbrk(challenge, "\r\n"); // find the first CR or LF
        if(chalRet)
          *chalRet = '\0'; // strip it

        D(DBF_NET, "received DIGEST-MD5 challenge: '%s'", challenge);

        // lets base64 decode it
        if(base64decode(challenge, (unsigned char *)challenge, strlen(challenge)) <= 0)
        {
          RETURN(FALSE);
          return FALSE;
        }

        D(DBF_NET, "decoded  DIGEST-MD5 challenge: '%s'", challenge);

        // we now analyze the received challenge identifier and pick out
        // the value which we are going to need for our challenge response.
        // This is the refered STEP ONE in the RFC 2831
        {
          char *pstart;
          char *pend;

          // first we try to find out the "realm" of the challenge
          if((pstart = strstr(challenge, "realm=")))
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
            W(DBF_NET, "'realm' not found in challenge, using '%s' instead", msn->domain);

            // if the challenge doesn't have a "realm" we assume our
            // choosen SMTP domain to be the realm
            realm = strdup(msn->domain);
          }

          D(DBF_NET, "realm: '%s'", realm);

          // grab the "nonce" token for later reference
          if((pstart = strstr(challenge, "nonce=")))
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
          if((pstart = strstr(challenge, "qop=")))
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
          snprintf(buf, sizeof(buf), "%s:%s:%s", msn->username, realm, msn->password);
          md5init(&context);
          md5update(&context, (unsigned char *)buf, strlen(buf));
          md5final(digest, &context);
          memcpy(buf, digest, 16);
          A1_len += snprintf(&buf[16], sizeof(buf)-16, ":%s:%s", nonce, cnonce);
          D(DBF_NET, "unencoded A1: '%s' (%ld)", buf, A1_len);

          // then we directly build the hexadecimal representation
          // HEX(H(A1))
          md5init(&context);
          md5update(&context, (unsigned char *)buf, A1_len);
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
          md5update(&context, (unsigned char *)buf, strlen(buf));
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
          md5update(&context, (unsigned char *)buf, strlen(buf));
          md5final(digest, &context);
          md5digestToHex(digest, response);
          D(DBF_NET, "encoded   resp: '%s'", response);
        }

        // form up the challenge to authenticate according to RFC 2831
        snprintf(challenge, sizeof(challenge),
                 "username=\"%s\","        // the username token
                 "realm=\"%s\","           // the realm token
                 "nonce=\"%s\","           // the nonce token
                 "cnonce=\"%s\","          // the client nonce (cnonce)
                 "nc=00000001,"            // the nonce count (here always 1)
                 "qop=\"auth\","           // we just use auth
                 "digest-uri=\"smtp/%s\"," // the digest-uri token
                 "response=\"%s\"",        // the response
                 msn->username,
                 realm,
                 nonce,
                 cnonce,
                 realm,
                 response);

        D(DBF_NET, "prepared challenge answer....: '%s'", challenge);
        base64encode(buffer, (unsigned char *)challenge, strlen(challenge));
        D(DBF_NET, "encoded  challenge answer....: '%s'", buffer);
        strlcat(buffer, "\r\n", sizeof(buffer));

        // now we send the SMTP AUTH response
        if(SendLineToHost(G->TR->connection, buffer) > 0)
        {
          // get the server response and see if it was valid
          if(ReceiveLineFromHost(G->TR->connection, buffer, SIZE_LINE) <= 0 || (rc = getResponseCode(buffer)) != 334)
            ER_NewError(tr(MSG_ER_BADRESPONSE_SMTP), msn->hostname, (char *)SMTPcmd[ESMTP_AUTH_DIGEST_MD5], buffer);
          else
          {
            // now that we have received the 334 code we just send a plain line
            // to signal that we don't need any option
            if(SendLineToHost(G->TR->connection, "\r\n") > 0)
            {
              if(ReceiveLineFromHost(G->TR->connection, buffer, SIZE_LINE) <= 0 || (rc = getResponseCode(buffer)) != 235)
                ER_NewError(tr(MSG_ER_BADRESPONSE_SMTP), msn->hostname, (char *)SMTPcmd[ESMTP_AUTH_DIGEST_MD5], buffer);
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
      if((resp = TR_SendSMTPCmd(ESMTP_AUTH_CRAM_MD5, NULL, tr(MSG_ER_BADRESPONSE_SMTP))) != NULL)
      {
        ULONG digest[4]; // 16 chars
        char buf[512];
        char *login = msn->username;
        char *password = msn->password;
        char *chalRet;

        // get the challenge code from the response line of the
        // AUTH command.
        strlcpy(challenge, &resp[4], sizeof(challenge));

        // now that we have the challenge phrase we need to base64decode
        // it, but have to take care to remove the ending "\r\n" cookie.
        chalRet = strpbrk(challenge, "\r\n"); // find the first CR or LF
        if(chalRet)
          *chalRet = '\0'; // strip it

        D(DBF_NET, "received CRAM-MD5 challenge: '%s'", challenge);

        // lets base64 decode it
        if(base64decode(challenge, (unsigned char *)challenge, strlen(challenge)) <= 0)
        {
          RETURN(FALSE);
          return FALSE;
        }

        D(DBF_NET, "decoded  CRAM-MD5 challenge: '%s'", challenge);

        // compose the md5 challenge
        md5hmac((unsigned char *)challenge, strlen(challenge), (unsigned char *)password, strlen(password), (unsigned char *)digest);
        snprintf(buf, sizeof(buf), "%s %08x%08x%08x%08x", login, (unsigned int)digest[0], (unsigned int)digest[1],
                                                                 (unsigned int)digest[2], (unsigned int)digest[3]);

        D(DBF_NET, "prepared CRAM-MD5 reponse..: '%s'", buf);
        // lets base64 encode the md5 challenge for the answer
        base64encode(buffer, (unsigned char *)buf, strlen(buf));
        D(DBF_NET, "encoded  CRAM-MD5 reponse..: '%s'", buffer);
        strlcat(buffer, "\r\n", sizeof(buffer));

        // now we send the SMTP AUTH response
        if(SendLineToHost(G->TR->connection, buffer) > 0)
        {
          // get the server response and see if it was valid
          if(ReceiveLineFromHost(G->TR->connection, buffer, SIZE_LINE) <= 0 || (rc = getResponseCode(buffer)) != 235)
            ER_NewError(tr(MSG_ER_BADRESPONSE_SMTP), msn->hostname, (char *)SMTPcmd[ESMTP_AUTH_CRAM_MD5], buffer);
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
      if((resp = TR_SendSMTPCmd(ESMTP_AUTH_LOGIN, NULL, tr(MSG_ER_BADRESPONSE_SMTP))) != NULL)
      {
        // prepare the username challenge
        D(DBF_NET, "prepared AUTH LOGIN challenge: '%s'", msn->username);
        base64encode(buffer, (unsigned char *)msn->username, strlen(msn->username));
        D(DBF_NET, "encoded  AUTH LOGIN challenge: '%s'", buffer);
        strlcat(buffer, "\r\n", sizeof(buffer));

        // now we send the SMTP AUTH response (UserName)
        if(SendLineToHost(G->TR->connection, buffer) > 0)
        {
          // get the server response and see if it was valid
          if(ReceiveLineFromHost(G->TR->connection, buffer, SIZE_LINE) > 0
             && (rc = getResponseCode(buffer)) == 334)
          {
            // prepare the password challenge
            D(DBF_NET, "prepared AUTH LOGIN challenge: '%s'", msn->password);
            base64encode(buffer, (unsigned char *)msn->password, strlen(msn->password));
            D(DBF_NET, "encoded  AUTH LOGIN challenge: '%s'", buffer);
            strlcat(buffer, "\r\n", sizeof(buffer));

            // now lets send the Password
            if(SendLineToHost(G->TR->connection, buffer) > 0)
            {
              // get the server response and see if it was valid
              if(ReceiveLineFromHost(G->TR->connection, buffer, SIZE_LINE) > 0
                 && (rc = getResponseCode(buffer)) == 235)
              {
                rc = SMTP_ACTION_OK;
              }
            }
          }

          if(rc != SMTP_ACTION_OK)
            ER_NewError(tr(MSG_ER_BADRESPONSE_SMTP), msn->hostname, (char *)SMTPcmd[ESMTP_AUTH_LOGIN], buffer);
        }
      }
    }
    break;

    // SMTP AUTH PLAIN (RFC 2595)
    case MSF_AUTH_PLAIN:
    {
      int len=0;
      D(DBF_NET, "processing AUTH PLAIN:");

      // The AUTH PLAIN command string is a single command string, so we go
      // and prepare the challenge first
      // According to RFC 2595 this string consists of three parts:
      // "[authorize-id] \0 authenticate-id \0 password"
      // where we can left out the first one

      // we don't have a "authorize-id" so we set the first char to \0
      challenge[len++] = '\0';
      len += snprintf(challenge+len, sizeof(challenge)-len, "%s", msn->username)+1; // authenticate-id
      len += snprintf(challenge+len, sizeof(challenge)-len, "%s", msn->password);   // password

      // now we base64 encode this string and send it to the server
      base64encode(buffer, (unsigned char *)challenge, len);

      // lets now form up the AUTH PLAIN command we are going to send
      // to the SMTP server for authorization purposes:
      snprintf(challenge, sizeof(challenge), "%s %s\r\n", SMTPcmd[ESMTP_AUTH_PLAIN], buffer);

      // now we send the SMTP AUTH command (UserName+Password)
      if(SendLineToHost(G->TR->connection, challenge) > 0)
      {
        // get the server response and see if it was valid
        if(ReceiveLineFromHost(G->TR->connection, buffer, SIZE_LINE) <= 0 || (rc = getResponseCode(buffer)) != 235)
          ER_NewError(tr(MSG_ER_BADRESPONSE_SMTP), msn->hostname, (char *)SMTPcmd[ESMTP_AUTH_PLAIN], buffer);
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
      ER_NewError(tr(MSG_CO_ER_SMTPAUTH), msn->hostname);
    }
    break;
  }

  D(DBF_NET, "Server responded with %ld", rc);

  RETURN((BOOL)(rc == SMTP_ACTION_OK));
  return (BOOL)(rc == SMTP_ACTION_OK);
}
///

/*** General connecting/disconnecting & transfer ***/
/// TR_Disconnect
//  Terminates a connection
static void TR_Disconnect(void)
{
  ENTER();

  D(DBF_NET, "disconnecting TCP/IP session...");

  DisconnectFromHost(G->TR->connection);

  LEAVE();
}
///
/// TR_RecvToFile()
// function that receives data from a POP3 server until it receives a \r\n.\r\n termination
// line. It automatically writes that data to the supplied filehandle and if present also
// updates the Transfer status
static int TR_RecvToFile(FILE *fh, const char *filename, struct TransStat *ts)
{
  int l=0, read, state=0, count;
  char buf[SIZE_LINE];
  char line[SIZE_LINE];
  BOOL done=FALSE;
  char *lineptr = line;

  ENTER();

  // get the first data the pop server returns after the TOP command
  if((read = count = ReceiveFromHost(G->TR->connection, buf, sizeof(buf))) <= 0)
    G->Error = TRUE;

  D(DBF_NET, "got %ld, expected %ld", G->TR->connection->error, CONNECTERR_NO_ERROR);
  while(G->TR->connection->error == CONNECTERR_NO_ERROR && G->TR->Abort == FALSE)
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
        if(ts != NULL)
          TR_TransStat_Update(ts, l, tr(MSG_TR_Downloading));

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
    if(done == TRUE || G->Error == TRUE || G->TR->Abort == TRUE)
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
/// TR_SetWinTitle
//  Sets the title of the transfer window
void TR_SetWinTitle(BOOL from, const char *text)
{
  // compose the window title
  snprintf(G->TR->WTitle, sizeof(G->TR->WTitle), tr(from ? MSG_TR_MailTransferFrom : MSG_TR_MailTransferTo), text);

  // set the window title
  set(G->TR->GUI.WI, MUIA_Window_Title, G->TR->WTitle);
}
///

/*** HTTP routines ***/
/// TR_DownloadURL()
//  Downloads a file from the web using HTTP/1.1 (RFC 2616)
BOOL TR_DownloadURL(struct Connection *conn, const char *server, const char *request, const char *filename)
{
  BOOL result = FALSE;
  BOOL noproxy = (C->ProxyServer[0] == '\0');
  int hport;
  char url[SIZE_URL];
  char host[SIZE_HOST];
  char *path;
  char *bufptr;

  ENTER();

  // extract the server address and strip the http:// part
  // of the URI
  if(strnicmp(server, "http://", 7) == 0)
    strlcpy(url, &server[7], sizeof(url));
  else
    strlcpy(url, server, sizeof(url));

  // in case an explicit request was given we
  // add it here
  if(request != NULL)
  {
    if(url[strlen(url)-1] != '/')
      strlcat(url, "/", sizeof(url));

    strlcat(url, request, sizeof(url));
  }

  // find the first occurance of the '/' separator in out
  // url and insert a terminating NUL character
  if((path = strchr(url, '/')) != NULL)
    *path++ = '\0';
  else
    path = (char *)"";

  // extract the hostname from the URL or use the proxy server
  // address if specified.
  strlcpy(host, noproxy ? url : C->ProxyServer, sizeof(host));

  // extract the port on which we connect if the
  // hostname contain an ':' separator
  if((bufptr = strchr(host, ':')) != NULL)
  {
    *bufptr++ = '\0';
    hport = atoi(bufptr);
  }
  else
    hport = noproxy ? 80 : 8080;

  // open the TCP/IP connection to 'host' under the port 'hport'
  if((ConnectToHost(conn, host, hport)) == CONNECTERR_SUCCESS)
  {
    char *serverHost;
    char serverPath[SIZE_LINE];
    char httpRequest[SIZE_LINE];
    char *port;

    // now we build the HTTP request we send out to the HTTP
    // server
    if(noproxy)
    {
      snprintf(serverPath, sizeof(serverPath), "/%s", path);
      serverHost = host;
    }
    else if((port = strchr(url, ':')) != NULL)
    {
      *port++ = '\0';

      snprintf(serverPath, sizeof(serverPath), "http://%s:%s/%s", url, port, path);
      serverHost = url;
    }
    else
    {
      snprintf(serverPath, sizeof(serverPath), "http://%s/%s", url, path);
      serverHost = url;
    }

    // construct the HTTP request
    // we send a HTTP/1.0 request because 1.1 implies that we have to be able
    // to deal with e.g. "Transfer-Encoding: chunked" responses which we can't handle
    // right now.
    snprintf(httpRequest, sizeof(httpRequest), "GET %s HTTP/1.0\r\n"
                                               "Host: %s\r\n"
                                               "User-Agent: %s\r\n"
                                               "Connection: close\r\n"
                                               "Accept: */*\r\n"
                                               "\r\n", serverPath, serverHost, yamuseragent);

    SHOWSTRING(DBF_NET, httpRequest);

    // send out the httpRequest
    if(SendLineToHost(conn, httpRequest) > 0)
    {
      char *p;
      char serverResponse[SIZE_LINE];
      int len;

      // clear the serverResponse string
      serverResponse[0] = '\0';

      // now we read out the very first line to see if the
      // response code matches and is fine
      len = ReceiveLineFromHost(conn, serverResponse, sizeof(serverResponse));

      SHOWSTRING(DBF_NET, serverResponse);

      // check the server response
      if(len > 0 && strnicmp(serverResponse, "HTTP/", 5) == 0 &&
         (p = strchr(serverResponse, ' ')) != NULL && atoi(TrimStart(p)) == 200)
      {
        // we can request all further lines from our socket
        // until we reach the entity body
        while(conn->error == CONNECTERR_NO_ERROR &&
              (len = ReceiveLineFromHost(conn, serverResponse, sizeof(serverResponse))) > 0)
        {
          // we scan for the end of the
          // response header by searching for the first '\r\n'
          // line
          if(strcmp(serverResponse, "\r\n") == 0)
          {
            FILE *out;

            // prepare the output file.
            if((out = fopen(filename, "w")) != NULL)
            {
              LONG retrieved = -1;

              setvbuf(out, NULL, _IOFBF, SIZE_FILEBUF);

              // we seem to have reached the entity body, so
              // from here we retrieve everything we can get and
              // immediately write it out to a file. that's it :)
              while(conn->error == CONNECTERR_NO_ERROR &&
                    (len = ReceiveLineFromHost(conn, serverResponse, sizeof(serverResponse))) > 0)
              {
                if(fwrite(serverResponse, len, 1, out) != 1)
                {
                  retrieved = -1; // signal an error!
                  break;
                }

                retrieved += len;
              }

              D(DBF_NET, "received %ld bytes", retrieved);

              // check if we retrieved anything
              if(conn->error == CONNECTERR_NO_ERROR && retrieved >= 0)
                result = TRUE;

              fclose(out);
            }
            else
              ER_NewError(tr(MSG_ER_CantCreateFile), filename);

            break;
          }
        }
      }
      else
        ER_NewError(tr(MSG_ER_DocNotFound), path);
    }
    else
      ER_NewError(tr(MSG_ER_SendHTTP));
  }
  else
    ER_NewError(tr(MSG_ER_ConnectHTTP), host);

  if(conn->error != CONNECTERR_NO_ERROR)
    result = FALSE;

  DisconnectFromHost(conn);

  RETURN(result);
  return result;
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
      // let us read the next line from the server and check if
      // some status message can be retrieved.
      if(ReceiveLineFromHost(G->TR->connection, buf, sizeof(buf)) > 0 &&
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
          struct MailServerNode *msn;

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

          #warning FIXME: replace GetMailServer() usage when struct Connection is there
          if((msn = GetMailServer(&C->mailServerList, MST_POP3, G->TR->POP_Nr)) != NULL)
            ER_NewError(errorMsg, msn->hostname, msn->account, (char *)POPcmd[command], buf);
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

  if(C->TransferWindow == TWM_SHOW ||
     (C->TransferWindow == TWM_AUTO && (guilevel == POP_START || guilevel == POP_USER)))
  {
    // avoid MUIA_Window_Open's side effect of activating the window if it was already open
    if(xget(G->TR->GUI.WI, MUIA_Window_Open) == FALSE)
      set(G->TR->GUI.WI, MUIA_Window_Open, TRUE);
  }
  set(G->TR->GUI.TX_STATUS, MUIA_Text_Contents, tr(MSG_TR_Connecting));

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
    set(G->TR->GUI.TX_STATUS, MUIA_Text_Contents, tr(MSG_TR_WaitWelcome));

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
    set(G->TR->GUI.TX_STATUS, MUIA_Text_Contents, tr(MSG_TR_INITTLS));

    // Now we have to Initialize and Start the TLS stuff if requested
    if(MakeSecureConnection(G->TR->connection) == TRUE)
      G->TR_UseTLS = TRUE;
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
    if((resp = TR_SendPOP3Cmd(POPCMD_CONNECT, NULL, tr(MSG_ER_POP3WELCOME))) == NULL)
      goto out;

    welcomemsg = StrBufCpy(NULL, resp);
  }

  if(passwd[0] == '\0')
  {
    // make sure the application isn't iconified
    if(xget(G->App, MUIA_Application_Iconified) == TRUE)
      PopUp();

    snprintf(buf, sizeof(buf), tr(MSG_TR_PopLoginReq), msn->username, host);
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
      set(G->TR->GUI.TX_STATUS, MUIA_Text_Contents, tr(MSG_TR_SendAPOPLogin));
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
    set(G->TR->GUI.TX_STATUS, MUIA_Text_Contents, tr(MSG_TR_SendUserID));
    if(TR_SendPOP3Cmd(POPCMD_USER, msn->username, tr(MSG_ER_BADRESPONSE_POP3)) == NULL)
      goto out;

    set(G->TR->GUI.TX_STATUS, MUIA_Text_Contents, tr(MSG_TR_SendPassword));
    if(TR_SendPOP3Cmd(POPCMD_PASS, passwd, tr(MSG_ER_BADRESPONSE_POP3)) == NULL)
      goto out;
  }

  set(G->TR->GUI.TX_STATUS, MUIA_Text_Contents, tr(MSG_TR_GetStats));
  if((resp = TR_SendPOP3Cmd(POPCMD_STAT, NULL, tr(MSG_ER_BADRESPONSE_POP3))) == NULL)
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
/// TR_DisplayMailList
//  Displays a list of messages ready for download
static void TR_DisplayMailList(BOOL largeonly)
{
  Object *lv = G->TR->GUI.LV_MAILS;
  struct Node *curNode;
  int pos=0;

  ENTER();

  set(lv, MUIA_NList_Quiet, TRUE);

  // search through our transferList
  IterateList(&G->TR->transferList, curNode)
  {
    struct MailTransferNode *mtn = (struct MailTransferNode *)curNode;
    #if defined(DEBUG)
    struct Mail *mail = mtn->mail;
    #endif

    D(DBF_GUI, "checking mail with flags %08lx and subject '%s'", mtn->tflags, mail->Subject);
    // only display mails to be downloaded
    if(hasTR_LOAD(mtn) || hasTR_PRESELECT(mtn))
    {
      // add this mail to the transfer list in case we either
      // should show ALL mails or the mail size is >= the warning size
      if(largeonly == FALSE || hasTR_PRESELECT(mtn))
      {
        mtn->position = pos++;

        DoMethod(lv, MUIM_NList_InsertSingle, mtn, MUIV_NList_Insert_Bottom);
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

    NewList((struct List *)&G->TR->transferList);

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
          struct MailServerNode *msn;

          #warning FIXME: replace GetMailServer() usage when struct Connection is there
          if((msn = GetMailServer(&C->mailServerList, MST_POP3, G->TR->POP_Nr)) == NULL)
          {
            RETURN(FALSE);
            return FALSE;
          }

          newMail->Size  = size;

          mode = (C->DownloadLarge == TRUE ? 1 : 0) +
                 (hasServerPurge(msn) == TRUE ? 2 : 0) +
                 (G->TR->GUIlevel == POP_USER ? 4 : 0) +
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
            mtn->tflags = tflags;
            mtn->index = index;

            AddTail((struct List *)&G->TR->transferList, (struct Node *)mtn);
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
/// TR_ApplyRemoteFilters
//  Applies remote filters to a message
static void TR_ApplyRemoteFilters(struct MailTransferNode *mtn)
{
  ENTER();

  // if there is no search count we can break out immediatly
  if(G->TR->SearchCount > 0)
  {
    struct Node *curNode;

    // Now we process the read header to set all flags accordingly
    IterateList(&C->filterList, curNode)
    {
      struct FilterNode *filter = (struct FilterNode *)curNode;

      if(DoFilterSearch(filter, mtn->mail) == TRUE)
      {
        if(hasExecuteAction(filter) && *filter->executeCmd)
           LaunchCommand(filter->executeCmd, FALSE, OUT_STDOUT);

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
static void TR_GetMessageDetails(struct MailTransferNode *mtn, int lline)
{
  struct Mail *mail = mtn->mail;

  ENTER();

  if(mail->From.Address[0] == '\0' && G->TR->Abort == FALSE && G->Error == FALSE)
  {
    char cmdbuf[SIZE_SMALL];

    // we issue a TOP command with a one line message body.
    //
    // This command is optional within the RFC 1939 specification
    // and therefore we don't throw any error
    snprintf(cmdbuf, sizeof(cmdbuf), "%d 1", mtn->index);
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
        if(TR_RecvToFile(tf->FP, tf->Filename, NULL) > 0)
          done = TRUE;

        // close the filehandle now.
        fclose(tf->FP);
        tf->FP = NULL;

        // If we end up here because of an error, abort or the upper loop wasn't finished
        // we exit immediatly with deleting the temp file also.
        if(G->TR->connection->error != CONNECTERR_NO_ERROR || G->TR->Abort == TRUE || done == FALSE)
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
            struct MailServerNode *msn;

            #warning FIXME: replace GetMailServer() usage when struct Connection is there
            if((msn = GetMailServer(&C->mailServerList, MST_POP3, G->TR->POP_Nr)) == NULL)
            {
              LEAVE();
              return;
            }

            snprintf(uidl, sizeof(uidl), "%s@%s", email->messageID, msn->hostname);
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
    DoMethod(G->TR->GUI.LV_MAILS, MUIM_NList_Redraw, lline);

  LEAVE();
}
///
/// TR_DisconnectPOP
//  Terminates a POP3 session
static void TR_DisconnectPOP(void)
{
  ENTER();

  set(G->TR->GUI.TX_STATUS, MUIA_Text_Contents, tr(MSG_TR_Disconnecting));

  if(G->Error == FALSE)
    TR_SendPOP3Cmd(POPCMD_QUIT, NULL, tr(MSG_ER_BADRESPONSE_POP3));

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

    G->TR->connection = conn;
    G->TR->Checking = TRUE;
    UpdateAppIcon();
    G->TR->GUIlevel = guilevel;
    G->TR->SearchCount = AllocFilterSearch(APPLY_REMOTE);
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
        if(GetMailServer(&C->mailServerList, MST_POP3, pop) != NULL)
          G->TR->DuplicatesChecking = TRUE;
      }
      else
      {
        struct Node *curNode;

        IterateList(&C->mailServerList, curNode)
        {
          struct MailServerNode *msn = (struct MailServerNode *)curNode;

          if(msn->type == MST_POP3)
          {
            if(isServerActive(msn))
            {
              G->TR->DuplicatesChecking = TRUE;
              break;
            }
          }
        }
      }

      if(G->TR->DuplicatesChecking == TRUE)
      {
        struct Node *curNode;

        if(InitUIDLhash() == TRUE)
        {
          IterateList(&C->mailServerList, curNode)
          {
            struct MailServerNode *msn = (struct MailServerNode *)curNode;

            if(msn->type == MST_POP3)
              CLEAR_FLAG(msn->flags, MSF_UIDLCHECKED);
          }
        }
      }
    }
  }
  else /* Finish previous connection */
  {
    struct MailServerNode *msn;

    #warning FIXME: replace GetMailServer() usage when struct Connection is there
    if((msn = GetMailServer(&C->mailServerList, MST_POP3, G->TR->POP_Nr)) == NULL)
    {
      LEAVE();
      return;
    }

    D(DBF_NET, "downloaded %ld mails from server '%s'", G->TR->Stats.Downloaded, msn->hostname);
    TR_DisconnectPOP();
    TR_Cleanup();
    AppendToLogfile(LF_ALL, 30, tr(MSG_LOG_Retrieving), G->TR->Stats.Downloaded-laststats, msn->username, msn->hostname);
    if(G->TR->SinglePOP == TRUE)
      pop = -1;

    laststats = G->TR->Stats.Downloaded;
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

    // make sure the transfer window is closed
    set(G->TR->GUI.WI, MUIA_Window_Open, FALSE);

    // free/cleanup the UIDL hash tables
    if(G->TR->DuplicatesChecking == TRUE)
      CleanupUIDLhash();

    FreeFilterSearch();
    G->TR->SearchCount = 0;

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
  G->TR_Allow = FALSE;
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

        // do we have to do some remote filter actions?
        if(G->TR->SearchCount > 0)
        {
          struct Node *curNode;

          set(G->TR->GUI.TX_STATUS, MUIA_Text_Contents, tr(MSG_TR_ApplyFilters));
          IterateList(&G->TR->transferList, curNode)
            TR_GetMessageDetails((struct MailTransferNode *)curNode, -2);
        }

        // if the user wants to avoid to receive the
        // same message from the POP3 server again
        // we have to analyze the UIDL of it
        if(G->TR->DuplicatesChecking == TRUE)
        {
          if(FilterDuplicates() == TRUE)
          {
            struct MailServerNode *msn;

            #warning FIXME: replace GetMailServer() usage when struct Connection is there
            if((msn = GetMailServer(&C->mailServerList, MST_POP3, G->TR->POP_Nr)) == NULL)
            {
              LEAVE();
              return;
            }

            SET_FLAG(msn->flags, MSF_UIDLCHECKED);
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

            struct Node *curNode;

            IterateList(&G->TR->transferList, curNode)
            {
              struct MailTransferNode *mtn = (struct MailTransferNode *)curNode;
              #if defined(DEBUG)
              struct Mail *mail = mtn->mail;
              #endif

              D(DBF_GUI, "checking mail with subject '%s' and size %ld for preselection", mail->Subject, mail->Size);              // check the size of those mails only, which are left for download
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
          G->TR->GMD_Mail = (struct MinNode *)GetHead((struct List *)&G->TR->transferList);
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
      struct MailServerNode *msn;

      #warning FIXME: replace GetMailServer() usage when struct Connection is there
      if((msn = GetMailServer(&C->mailServerList, MST_POP3, G->TR->POP_Nr)) == NULL)
      {
        LEAVE();
        return;
      }

      W(DBF_NET, "no messages found on server '%s'", msn->hostname);

      // per default we flag that POP3 server as being UIDLchecked
      if(G->TR->DuplicatesChecking == TRUE)
        SET_FLAG(msn->flags, MSF_UIDLCHECKED);
    }
  }
  else
    G->TR->Stats.Error = TRUE;

  BusyEnd();

  TR_GetMailFromNextPOP(FALSE, 0, 0);

  LEAVE();
}
///
/// TR_SendPOP3KeepAlive()
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

/*** SMTP routines ***/
/// TR_SendSMTPCmd
//  Sends a command to the SMTP server and returns the response message
//  described in (RFC 2821)
static char *TR_SendSMTPCmd(const enum SMTPCommand command, const char *parmtext, const char *errorMsg)
{
  BOOL result = FALSE;
  static char buf[SIZE_LINE]; // RFC 2821 says 1000 should be enough

  ENTER();

  // first we check if the socket is ready
  if(G->TR->connection != NULL)
  {
    // now we prepare the SMTP command
    if(parmtext == NULL || parmtext[0] == '\0')
      snprintf(buf, sizeof(buf), "%s\r\n", SMTPcmd[command]);
    else
      snprintf(buf, sizeof(buf), "%s %s\r\n", SMTPcmd[command], parmtext);

    D(DBF_NET, "TCP: SMTP cmd '%s' with param '%s'", SMTPcmd[command], SafeStr(parmtext));

    // lets send the command via TR_WriteLine, but not if we are in connection
    // state
    if(command == SMTP_CONNECT || SendLineToHost(G->TR->connection, buf) > 0)
    {
      int len = 0;

      // after issuing the SMTP command we read out the server response to it
      // but only if this wasn't the SMTP_QUIT command.
      if((len = ReceiveLineFromHost(G->TR->connection, buf, sizeof(buf))) > 0)
      {
        // get the response code
        int rc = strtol(buf, NULL, 10);

        // if the response is a multiline response we have to get out more
        // from the socket
        if(buf[3] == '-') // (RFC 2821) - section 4.2.1
        {
          char tbuf[SIZE_LINE];

          // now we concatenate the multiline reply to
          // out main buffer
          do
          {
            // lets get out the next line from the socket
            if((len = ReceiveLineFromHost(G->TR->connection, tbuf, sizeof(tbuf))) > 0)
            {
              // get the response code
              int rc2 = strtol(buf, NULL, 10);

              // check if the response code matches the one
              // of the first line
              if(rc == rc2)
              {
                // lets concatenate both strings while stripping the
                // command code and make sure we didn't reach the end
                // of the buffer
                if(strlcat(buf, tbuf, sizeof(buf)) >= sizeof(buf))
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

            case SMTP_HELP:    { result = (rc == 211 || rc == 214); } break;
            case SMTP_VRFY:    { result = (rc == 250 || rc == 251); } break;
            case SMTP_CONNECT: { result = (rc == 220); } break;
            case SMTP_QUIT:    { result = (rc == 221); } break;
            case SMTP_DATA:    { result = (rc == 354); } break;

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
            case SMTP_TURN:    { result = (rc == 250); } break;

            // ESMTP commands & response codes
            case ESMTP_EHLO:            { result = (rc == 250); } break;
            case ESMTP_STARTTLS:        { result = (rc == 220); } break;

            // ESMTP_AUTH command responses
            case ESMTP_AUTH_CRAM_MD5:
            case ESMTP_AUTH_DIGEST_MD5:
            case ESMTP_AUTH_LOGIN:
            case ESMTP_AUTH_PLAIN:      { result = (rc == 334); } break;
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

          result = TRUE;
          buf[0] = '\0';
        }
        else
          errorMsg = tr(MSG_ER_CONNECTIONBROKEN);
      }
    }
    else
      errorMsg = tr(MSG_ER_CONNECTIONBROKEN);
  }
  else
    errorMsg = tr(MSG_ER_CONNECTIONBROKEN);

  // the rest of the responses throws an error
  if(result == FALSE)
  {
    if(errorMsg != NULL)
    {
      struct MailServerNode *msn;

      #warning FIXME: replace GetMailServer() usage when struct Connection is there
      if((msn = GetMailServer(&C->mailServerList, MST_SMTP, 0)) != NULL)
        ER_NewError(errorMsg, msn->hostname, (char *)SMTPcmd[command], buf);
    }

    RETURN(NULL);
    return NULL;
  }
  else
  {
    RETURN(buf);
    return buf;
  }
}
///
/// TR_ConnectSMTP
//  Connects to a SMTP mail server - here we always try to do an ESMTP connection
//  first via an EHLO command and then check if it succeeded or not.
static BOOL TR_ConnectSMTP(void)
{
  BOOL result = FALSE;
  struct MailServerNode *msn;

  ENTER();

  #warning FIXME: replace GetMailServer() usage when struct Connection is there
  if((msn = GetMailServer(&C->mailServerList, MST_SMTP, 0)) == NULL)
  {
    RETURN(FALSE);
    return FALSE;
  }

  // If we did a TLS negotitaion previously we have to skip the
  // welcome message, but if it was another connection like a normal or a SSL
  // one we have wait for the welcome
  if(G->TR_UseTLS == FALSE || hasServerSSL(msn))
  {
    set(G->TR->GUI.TX_STATUS, MUIA_Text_Contents, tr(MSG_TR_WaitWelcome));

    result = (TR_SendSMTPCmd(SMTP_CONNECT, NULL, tr(MSG_ER_BADRESPONSE_SMTP)) != NULL);
  }
  else
    result = TRUE;

  // now we either send a HELO (non-ESMTP) or EHLO (ESMTP) command to
  // signal we wanting to start a session accordingly (RFC 1869 - section 4)
  if(result == TRUE && G->TR->connection != NULL)
  {
    ULONG flags = 0;
    char *resp = NULL;

    // per default we flag the SMTP to be capable of an ESMTP
    // connection.
    SET_FLAG(flags, SMTP_FLG_ESMTP);

    // set the connection status
    set(G->TR->GUI.TX_STATUS, MUIA_Text_Contents, tr(MSG_TR_SendHello));

    D(DBF_NET, "trying ESMTP negotation");

    // in case we require SMTP-AUTH or a TLS secure connection we
    // have to force an ESMTP connection
    if(hasServerAuth(msn) || hasServerTLS(msn))
      resp = TR_SendSMTPCmd(ESMTP_EHLO, msn->domain, tr(MSG_ER_BADRESPONSE_SMTP));
    else
    {
      // in all other cases, we first try to get an ESMTP connection
      // and if that doesn't work we go and do a normal SMTP connection
      if((resp = TR_SendSMTPCmd(ESMTP_EHLO, msn->domain, NULL)) == NULL)
      {
        D(DBF_NET, "ESMTP negotation failed, trying normal SMTP negotation");

        // according to RFC 1869 section 4.7 we send an RSET command
        // between the two EHLO and HELO commands to play save
        TR_SendSMTPCmd(SMTP_RSET, NULL, NULL); // no error code check

        // now we send a HELO command which signals we are not
        // going to use any ESMTP stuff
        resp = TR_SendSMTPCmd(SMTP_HELO, msn->domain, tr(MSG_ER_BADRESPONSE_SMTP));

        // signal we are not into ESMTP stuff
        CLEAR_FLAG(flags, SMTP_FLG_ESMTP);
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
            SET_FLAG(flags, SMTP_FLG_STARTTLS);
          else if(strnicmp(resp+4, "AUTH", 4) == 0)         // SMTP-AUTH (RFC 2554)
          {
            if(NULL != strstr(resp+9,"CRAM-MD5"))
              SET_FLAG(flags, SMTP_FLG_AUTH_CRAM_MD5);

            if(NULL != strstr(resp+9,"DIGEST-MD5"))
              SET_FLAG(flags, SMTP_FLG_AUTH_DIGEST_MD5);

            if(NULL != strstr(resp+9,"PLAIN"))
              SET_FLAG(flags, SMTP_FLG_AUTH_PLAIN);

            if(NULL != strstr(resp+9,"LOGIN"))
              SET_FLAG(flags, SMTP_FLG_AUTH_LOGIN);
          }
          else if(strnicmp(resp+4, "SIZE", 4) == 0)         // STD:10 - SIZE declaration (RFC 1870)
            SET_FLAG(flags, SMTP_FLG_SIZE);
          else if(strnicmp(resp+4, "PIPELINING", 10) == 0)  // STD:60 - PIPELINING (RFC 2920)
            SET_FLAG(flags, SMTP_FLG_PIPELINING);
          else if(strnicmp(resp+4, "8BITMIME", 8) == 0)     // 8BITMIME support (RFC 1652)
            SET_FLAG(flags, SMTP_FLG_8BITMIME);
          else if(strnicmp(resp+4, "DSN", 3) == 0)          // DSN - Delivery Status Notifications (RFC 1891)
            SET_FLAG(flags, SMTP_FLG_DSN);
          else if(strnicmp(resp+4, "ETRN", 4) == 0)         // ETRN - Remote Message Queue Starting (RFC 1985)
            SET_FLAG(flags, SMTP_FLG_ETRN);
          else if(strnicmp(resp+4, "ENHANCEDSTATUSCODES", 19) == 0) // Enhanced Status Codes (RFC 2034)
            SET_FLAG(flags, SMTP_FLG_ENHANCEDSTATUSCODES);
          else if(strnicmp(resp+4, "DELIVERBY", 9) == 0)    // DELIVERBY Extension (RFC 2852)
            SET_FLAG(flags, SMTP_FLG_DELIVERBY);
          else if(strnicmp(resp+4, "HELP", 4) == 0)         // HELP Extension (RFC 821)
            SET_FLAG(flags, SMTP_FLG_HELP);
        }
      }

      #ifdef DEBUG
      D(DBF_NET, "SMTP Server '%s' serves:", msn->hostname);
      D(DBF_NET, "  ESMTP..............: %ld", hasESMTP(flags));
      D(DBF_NET, "  AUTH CRAM-MD5......: %ld", hasCRAM_MD5_Auth(flags));
      D(DBF_NET, "  AUTH DIGEST-MD5....: %ld", hasDIGEST_MD5_Auth(flags));
      D(DBF_NET, "  AUTH LOGIN.........: %ld", hasLOGIN_Auth(flags));
      D(DBF_NET, "  AUTH PLAIN.........: %ld", hasPLAIN_Auth(flags));
      D(DBF_NET, "  STARTTLS...........: %ld", hasSTARTTLS(flags));
      D(DBF_NET, "  SIZE...............: %ld", hasSIZE(flags));
      D(DBF_NET, "  PIPELINING.........: %ld", hasPIPELINING(flags));
      D(DBF_NET, "  8BITMIME...........: %ld", has8BITMIME(flags));
      D(DBF_NET, "  DSN................: %ld", hasDSN(flags));
      D(DBF_NET, "  ETRN...............: %ld", hasETRN(flags));
      D(DBF_NET, "  ENHANCEDSTATUSCODES: %ld", hasENHANCEDSTATUSCODES(flags));
      D(DBF_NET, "  DELIVERBY..........: %ld", hasDELIVERBY(flags));
      D(DBF_NET, "  HELP...............: %ld", hasHELP(flags));
      #endif

      // now we check the 8BITMIME extension against
      // the user configured Allow8bit setting and if it collides
      // we raise a warning.
      if(has8BITMIME(flags) == FALSE && hasServer8bit(msn) == TRUE)
        result = FALSE;
    }
    else
    {
      result = FALSE;

      W(DBF_NET, "error on SMTP server negotation");
    }

    msn->smtpFlags = flags;
  }
  else
    W(DBF_NET, "SMTP connection failure!");

  RETURN(result);
  return result;
}
///
/// TR_ChangeTransFlagsFunc
//  Changes transfer flags of all selected messages
HOOKPROTONHNO(TR_ChangeTransFlagsFunc, void, int *arg)
{
  int id = MUIV_NList_NextSelected_Start;

  do
  {
    struct MailTransferNode *mtn;

    DoMethod(G->TR->GUI.LV_MAILS, MUIM_NList_NextSelected, &id);
    if(id == MUIV_NList_NextSelected_End)
      break;

    DoMethod(G->TR->GUI.LV_MAILS, MUIM_NList_GetEntry, id, &mtn);
    mtn->tflags = *arg;

    DoMethod(G->TR->GUI.LV_MAILS, MUIM_NList_Redraw, id);
  }
  while(TRUE);
}
MakeStaticHook(TR_ChangeTransFlagsHook, TR_ChangeTransFlagsFunc);
///
/// TR_TransStat_Init
//  Initializes transfer statistics
static void TR_TransStat_Init(struct TransStat *ts)
{
  struct Node *curNode;

  ENTER();

  ts->Msgs_Tot = 0;
  ts->Size_Tot = 0;

  if(G->TR->GUI.GR_LIST != NULL)
  {
    set(G->TR->GUI.GR_PAGE, MUIA_Group_ActivePage, 1);
    DoMethod(G->TR->GUI.LV_MAILS, MUIM_NList_Select, MUIV_NList_Select_All, MUIV_NList_Select_Off, NULL);
  }

  // search through our transferList
  IterateList(&G->TR->transferList, curNode)
  {
    struct MailTransferNode *mtn = (struct MailTransferNode *)curNode;

    ts->Msgs_Tot++;

    if(hasTR_LOAD(mtn))
      ts->Size_Tot += mtn->mail->Size;
  }

  LEAVE();
}
///
/// TR_TransStat_Start
//  Resets statistics display
static void TR_TransStat_Start(struct TransStat *ts)
{
  ENTER();

  ts->Msgs_Done = 0;
  ts->Size_Done = 0;

  // get the actual time we started the TransferStatus
  GetSysTime(TIMEVAL(&ts->Clock_Last));
  ts->Clock_Start = ts->Clock_Last.Seconds;

  memset(&ts->Clock_Last, 0, sizeof(ts->Clock_Last));

  snprintf(G->TR->CountLabel, sizeof(G->TR->CountLabel), tr(MSG_TR_MESSAGEGAUGE), ts->Msgs_Tot);
  xset(G->TR->GUI.GA_COUNT, MUIA_Gauge_InfoText, G->TR->CountLabel,
                            MUIA_Gauge_Max,      ts->Msgs_Tot,
                            MUIA_Gauge_Current,  0);

  LEAVE();
}
///
/// TR_TransStat_Finish
//  updates statistics display to represent the final state
static void TR_TransStat_Finish(struct TransStat *ts)
{
  ENTER();

  // make sure we have valid strings to display
  FormatSize(ts->Size_Curr_Max, ts->str_size_curr_max, sizeof(ts->str_size_curr_max), SF_AUTO);

  // show the final statistics
  snprintf(G->TR->CountLabel, sizeof(G->TR->CountLabel), tr(MSG_TR_MESSAGEGAUGE), ts->Msgs_Tot);
  xset(G->TR->GUI.GA_COUNT, MUIA_Gauge_InfoText, G->TR->CountLabel,
                            MUIA_Gauge_Max,      ts->Msgs_Tot,
                            MUIA_Gauge_Current,  ts->Msgs_Tot);

  snprintf(G->TR->BytesLabel, sizeof(G->TR->BytesLabel), tr(MSG_TR_TRANSFERSIZE),
                                                         ts->str_size_curr_max, ts->str_size_curr_max);
  xset(G->TR->GUI.GA_BYTES, MUIA_Gauge_InfoText, G->TR->BytesLabel,
                            MUIA_Gauge_Max,      100,
                            MUIA_Gauge_Current,  100);
  LEAVE();
}
///
/// TR_TransStat_NextMsg
//  Updates statistics display for next message
static void TR_TransStat_NextMsg(struct TransStat *ts, int index, int listpos, LONG size, const char *status)
{
  ENTER();

  ts->Msgs_Curr = index;
  ts->Msgs_ListPos = listpos;
  ts->Msgs_Done++;
  ts->Size_Curr = 0;
  ts->Size_Curr_Max = size;

  // format the current mail's size ahead of any refresh
  FormatSize(size, ts->str_size_curr_max, sizeof(ts->str_size_curr_max), SF_AUTO);

  TR_TransStat_Update(ts, 0, status);

  LEAVE();
}
///
/// TR_TransStat_Update
//  Updates statistics display for next block of data
static void TR_TransStat_Update(struct TransStat *ts, int size_incr, const char *status)
{
  ENTER();

  if(size_incr > 0)
  {
    ts->Size_Done += size_incr;
    ts->Size_Curr += size_incr;
  }
  else if(size_incr == TS_SETMAX)
  {
    // first update the total transferred size
    ts->Size_Done += ts->Size_Curr_Max - ts->Size_Curr;
    // we are done with this mail, so make sure the current size equals the final size
    ts->Size_Curr = ts->Size_Curr_Max;
  }

  // if the window isn't open we don't need to update it, do we?
  if(xget(G->TR->GUI.WI, MUIA_Window_Open) == TRUE)
  {
    // update the stats at most 4 times per second
    if(TimeHasElapsed(&ts->Clock_Last, 250000) == TRUE)
    {
      ULONG deltatime = ts->Clock_Last.Seconds - ts->Clock_Start;
      ULONG speed = 0;
      LONG remclock = 0;
      ULONG max;
      ULONG current;

      // if we have a preselection window, update it.
      if(G->TR->GUI.GR_LIST != NULL && ts->Msgs_ListPos >= 0)
        set(G->TR->GUI.LV_MAILS, MUIA_NList_Active, ts->Msgs_ListPos);

      // first we calculate the speed in bytes/sec
      // to display to the user
      if(deltatime != 0)
        speed = ts->Size_Done / deltatime;

      // calculate the estimated remaining time
      if(speed != 0 && ((remclock = (ts->Size_Tot / speed) - deltatime) < 0))
        remclock = 0;

      // show the current status
      set(G->TR->GUI.TX_STATUS, MUIA_Text_Contents, status);

      // show the current message index
      set(G->TR->GUI.GA_COUNT, MUIA_Gauge_Current, ts->Msgs_Curr);

      // format the size done and size total strings
      FormatSize(ts->Size_Done, ts->str_size_done, sizeof(ts->str_size_done), SF_MIXED);
      FormatSize(ts->Size_Tot, ts->str_size_tot, sizeof(ts->str_size_tot), SF_MIXED);
      FormatSize(ts->Size_Curr, ts->str_size_curr, sizeof(ts->str_size_curr), SF_AUTO);

      FormatSize(speed, ts->str_speed, sizeof(ts->str_speed), SF_MIXED);

      // now format the StatsLabel and update it
      snprintf(G->TR->StatsLabel, sizeof(G->TR->StatsLabel), tr(MSG_TR_TRANSFERSTATUS),
                                  ts->str_size_done, ts->str_size_tot, ts->str_speed,
                                  deltatime / 60, deltatime % 60,
                                  remclock / 60, remclock % 60);

      set(G->TR->GUI.TX_STATS, MUIA_Text_Contents, G->TR->StatsLabel);

      // update the gauge
      snprintf(G->TR->BytesLabel, sizeof(G->TR->BytesLabel), tr(MSG_TR_TRANSFERSIZE),
                                                             ts->str_size_curr, ts->str_size_curr_max);
      if(size_incr == TS_SETMAX)
      {
        max = 100;
        current = 100;
      }
      else if(ts->Size_Curr_Max <= 65536)
      {
        max = ts->Size_Curr_Max;
        current = ts->Size_Curr;
      }
      else
      {
        max = ts->Size_Curr_Max / 1024;
        current = ts->Size_Curr / 1024;
      }
      xset(G->TR->GUI.GA_BYTES, MUIA_Gauge_InfoText, G->TR->BytesLabel,
                                MUIA_Gauge_Max,      max,
                                MUIA_Gauge_Current,  current);

      // signal the application to update now
      DoMethod(G->App, MUIM_Application_InputBuffered);
    }
  }

  LEAVE();
}
///
/// TR_Cleanup
//  Free temporary message and UIDL lists
void TR_Cleanup(void)
{
  struct Node *curNode;

  ENTER();

  if(G->TR->GUI.LV_MAILS != NULL)
    DoMethod(G->TR->GUI.LV_MAILS, MUIM_NList_Clear);

  while((curNode = RemHead((struct List *)&G->TR->transferList)) != NULL)
  {
    struct MailTransferNode *mtn = (struct MailTransferNode *)curNode;

    // free the mail pointer
    free(mtn->mail);

    // free the UIDL
    free(mtn->UIDL);

    // free the node itself
    free(mtn);
  }

  NewList((struct List *)&G->TR->transferList);

  LEAVE();
}
///
/// TR_AbortnClose
//  Aborts a transfer
static void TR_AbortnClose(void)
{
  ENTER();

  set(G->TR->GUI.WI, MUIA_Window_Open, FALSE);
  TR_Cleanup();
  MA_ChangeTransfer(TRUE);

  DeleteMailList(G->TR->downloadedMails);
  G->TR->downloadedMails = NULL;

  DisposeModulePush(&G->TR);

  LEAVE();
}
///
/// TR_ApplySentFilters
//  Applies filters to a sent message
static BOOL TR_ApplySentFilters(struct Mail *mail)
{
  BOOL result = TRUE;

  ENTER();

  // only if we have a positiv search count we start
  // our filtering at all, otherwise we return immediatly
  if(G->TR->SearchCount > 0)
  {
    struct Node *curNode;

    // Now we process the read header to set all flags accordingly
    IterateList(&C->filterList, curNode)
    {
      struct FilterNode *filter = (struct FilterNode *)curNode;

      if(DoFilterSearch(filter, mail) == TRUE)
      {
        if(ExecuteFilterAction(filter, mail) == FALSE)
        {
          result = FALSE;
          break;
        }
      }
    }
  }

  RETURN(result);
  return result;
}
///

/*** UIDL (Avoid duplicates) management ***/
/// InitUIDLhash()
// Initialize the UIDL list and load it from the .uidl file
static BOOL InitUIDLhash(void)
{
  BOOL result = FALSE;

  ENTER();

  // make sure no other UIDLhashTable is active
  if(G->TR->UIDLhashTable != NULL)
    CleanupUIDLhash();

  // allocate a new hashtable for managing the UIDL data
  if((G->TR->UIDLhashTable = HashTableNew(HashTableGetDefaultStringOps(), NULL, sizeof(struct UIDLtoken), 512)) != NULL)
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

      free(uidl);

      // start with a clean and and so far unmodified hash table
      G->TR->UIDLhashIsDirty = FALSE;
    }
    else
      W(DBF_UIDL, "no or empty .uidl file found");

    SHOWVALUE(DBF_UIDL, G->TR->UIDLhashTable->entryCount);

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
    // out as well. Otherwise we skip the write operation
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

  if(G->TR->UIDLhashTable != NULL)
  {
    // save the UIDLs only if something has been changed
    if(G->TR->UIDLhashIsDirty == TRUE)
    {
      FILE *fh;

      // before we go and destroy the UIDL hash we have to
      // write it to the .uidl file back again.
      if((fh = fopen(CreateFilename(".uidl"), "w")) != NULL)
      {
        setvbuf(fh, NULL, _IOFBF, SIZE_FILEBUF);

        // call HashTableEnumerate with the SaveUIDLtoken callback function
        HashTableEnumerate(G->TR->UIDLhashTable, SaveUIDLtoken, fh);

        fclose(fh);
      }
      else
        E(DBF_UIDL, "couldn't open .uidl file for writing");
    }

    // now we can destroy the uidl hash
    HashTableDestroy(G->TR->UIDLhashTable);
    G->TR->UIDLhashTable = NULL;

    D(DBF_UIDL, "successfully cleaned up UIDLhash");
  }

  // forget any modification to the hash table
  G->TR->UIDLhashIsDirty = FALSE;

  LEAVE();
}
///
/// FilterDuplicates()
//
static BOOL FilterDuplicates(void)
{
  BOOL result = FALSE;
  struct MailServerNode *msn;

  ENTER();

  #warning FIXME: replace GetMailServer() usage when struct Connection is there
  if((msn = GetMailServer(&C->mailServerList, MST_POP3, G->TR->POP_Nr)) == NULL)
  {
    RETURN(FALSE);
    return FALSE;
  }

  // we first make sure the UIDL list is loaded from disk
  if(G->TR->UIDLhashTable != NULL)
  {
    // check if there is anything to transfer at all
    if(IsMinListEmpty(&G->TR->transferList) == FALSE)
    {
      // inform the user of the operation
      set(G->TR->GUI.TX_STATUS, MUIA_Text_Contents, tr(MSG_TR_CHECKUIDL));

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
          while(G->TR->Abort == FALSE && G->TR->connection->error == CONNECTERR_NO_ERROR && strncmp(buf, ".\r\n", 3) != 0)
          {
            int num;
            char uidl[SIZE_DEFAULT+SIZE_HOST];
            struct Node *curNode;

            // now parse the line and get the message number and UIDL
            sscanf(buf, "%d %s", &num, uidl);

            // lets add our own ident to the uidl so that we can compare
            // it against our saved list
            strlcat(uidl, "@", sizeof(uidl));
            strlcat(uidl, msn->hostname, sizeof(uidl));

            // search through our transferList
            IterateList(&G->TR->transferList, curNode)
            {
              struct MailTransferNode *mtn = (struct MailTransferNode *)curNode;

              if(mtn->index == num)
              {
                mtn->UIDL = strdup(uidl);

                if(G->TR->UIDLhashTable->entryCount > 0 && mtn->UIDL != NULL)
                {
                  struct HashEntryHeader *entry = HashTableOperate(G->TR->UIDLhashTable, mtn->UIDL, htoLookup);

                  // see if that hash lookup worked out fine or not.
                  if(HASH_ENTRY_IS_LIVE(entry))
                  {
                    struct UIDLtoken *token = (struct UIDLtoken *)entry;

                    // make sure the mail is flagged as being ignoreable
                    G->TR->Stats.DupSkipped++;
                    // don't download this mail, because it has been downloaded before
                    CLEAR_FLAG(mtn->tflags, TRF_LOAD);

                    // mark the UIDLtoken as being checked
                    token->checked = TRUE;

                    D(DBF_UIDL, "mail %ld: UIDL '%s' was FOUND!", mtn->index, mtn->UIDL);
                  }
                }

                break;
              }

              if(G->TR->Abort == TRUE || G->TR->connection->error != CONNECTERR_NO_ERROR)
                break;
            }

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
        struct Node *curNode;

        W(DBF_UIDL, "POP3 server '%s' doesn't support UIDL command!", msn->hostname);

        // search through our transferList
        IterateList(&G->TR->transferList, curNode)
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
          if(G->TR->UIDLhashTable->entryCount > 0 && mtn->UIDL != NULL)
          {
            struct HashEntryHeader *entry = HashTableOperate(G->TR->UIDLhashTable, mtn->UIDL, htoLookup);

            // see if that hash lookup worked out fine or not.
            if(HASH_ENTRY_IS_LIVE(entry))
            {
              G->TR->Stats.DupSkipped++;
              // don't download this mail, because it has been downloaded before
              CLEAR_FLAG(mtn->tflags, TRF_LOAD);

              D(DBF_UIDL, "mail %ld: UIDL '%s' was FOUND!", mtn->index, mtn->UIDL);
            }
          }

          if(G->TR->Abort == TRUE || G->TR->connection->error != CONNECTERR_NO_ERROR)
            break;
        }
      }

      result = (G->TR->Abort == FALSE && G->TR->connection->error == CONNECTERR_NO_ERROR);
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
/// AddUIDLtoHash()
// adds the UIDL of a mail transfer node to the hash
static void AddUIDLtoHash(const char *uidl, BOOL checked)
{
  struct UIDLtoken *token;

  ENTER();

  if((token = (struct UIDLtoken *)HashTableOperate(G->TR->UIDLhashTable, uidl, htoAdd)) != NULL)
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
  if(HashTableOperate(G->TR->UIDLhashTable, uidl, htoRemove) != NULL)
    D(DBF_UIDL, "removed UIDL '%s' from hash", uidl);
  else
    W(DBF_UIDL, "couldn't remove UIDL '%s' from hash", uidl);

  LEAVE();
}
///

/*** EXPORT ***/
/// TR_ProcessEXPORT
//  Saves a list of messages to a MBOX mailbox file
BOOL TR_ProcessEXPORT(char *fname, struct MailList *mlist, BOOL append)
{
  BOOL success = FALSE;
  BOOL abort = FALSE;
  struct MailNode *mnode;
  int i;

  ENTER();

  // reset our processing list
  NewList((struct List *)&G->TR->transferList);

  // temporarly copy all data out of our mlist to the
  // processing list and mark all mails to get "loaded"
  i = 0;

  LockMailListShared(mlist);

  ForEachMailNode(mlist, mnode)
  {
    struct Mail *mail = mnode->mail;

    if(mail != NULL)
    {
      struct MailTransferNode *mtn;

      if((mtn = calloc(1, sizeof(struct MailTransferNode))) != NULL)
      {
        if((mtn->mail = memdup(mail, sizeof(struct Mail))) != NULL)
        {
          mtn->index = i + 1;

          // set to LOAD
          mtn->tflags = TRF_LOAD;

          AddTail((struct List *)&(G->TR->transferList), (struct Node *)mtn);
        }
        else
        {
          // we end up in a low memory condition, lets exit
          // after having freed everything
          free(mtn);
          abort = TRUE;
          break;
        }
      }
      else
      {
        // we end up in a low memory condition, lets exit
        abort = TRUE;
        break;
      }
    }

    i++;
  }

  UnlockMailList(mlist);

  // if we have now something in our processing list,
  // lets go on
  if(abort == FALSE &&
     IsMinListEmpty(&G->TR->transferList) == FALSE)
  {
    FILE *fh;
    struct TransStat ts;

    TR_SetWinTitle(FALSE, (char *)FilePart(fname));
    TR_TransStat_Init(&ts);
    TR_TransStat_Start(&ts);

    // open our final destination file either in append or in a fresh
    // write mode.
    if((fh = fopen(fname, append ? "a" : "w")) != NULL)
    {
      struct Node *curNode;

      setvbuf(fh, NULL, _IOFBF, SIZE_FILEBUF);

      success = TRUE;

      IterateList(&G->TR->transferList, curNode)
      {
        struct MailTransferNode *mtn = (struct MailTransferNode *)curNode;
        struct Mail *mail = mtn->mail;
        char fullfile[SIZE_PATHFILE];

        // update the transfer status
        TR_TransStat_NextMsg(&ts, mtn->index, -1, mail->Size, tr(MSG_TR_Exporting));

        if(StartUnpack(GetMailFile(NULL, NULL, mail), fullfile, mail->Folder) != NULL)
        {
          FILE *mfh;

          // open the message file to start exporting it
          if((mfh = fopen(fullfile, "r")) != NULL)
          {
            char datstr[64];
            char *buf = NULL;
            size_t buflen = 0;
            ssize_t curlen;
            BOOL inHeader = TRUE;

            // printf out our leading "From " MBOX format line first
            DateStamp2String(datstr, sizeof(datstr), &mail->Date, DSS_UNIXDATE, TZC_NONE);
            fprintf(fh, "From %s %s", mail->From.Address, datstr);

            // let us put out the Status: header field
            fprintf(fh, "Status: %s\n", MA_ToStatusHeader(mail));

            // let us put out the X-Status: header field
            fprintf(fh, "X-Status: %s\n", MA_ToXStatusHeader(mail));

            // now we iterate through every line of our mail and try to substitute
            // found "From " line with quoted ones
            while(G->TR->Abort == FALSE &&
                  (curlen = getline(&buf, &buflen, mfh)) > 0)
            {
              char *tmp = buf;

              // check if this is a single \n so that it
              // signals the end if a line
              if(buf[0] == '\n' || (buf[0] == '\r' && buf[1] == '\n'))
              {
                inHeader = FALSE;

                if(fwrite(buf, curlen, 1, fh) != 1)
                {
                  // write error, bail out
                  break;
                }

                continue;
              }

              // the mboxrd format specifies that we need to quote any
              // From, >From, >>From etc. occurance.
              // http://www.qmail.org/man/man5/mbox.html
              while(*tmp == '>')
                tmp++;

              if(strncmp(tmp, "From ", 5) == 0)
              {
                if(fputc('>', fh) == EOF)
                {
                  // write error, bail out
                  break;
                }
              }
              else if(inHeader == TRUE)
              {
                // let us skip some specific headerlines
                // because we placed our own here
                if(strncmp(buf, "Status: ", 8) == 0 ||
                   strncmp(buf, "X-Status: ", 10) == 0)
                {
                  // skip line
                  continue;
                }
              }

              // write the line to our destination file
              if(fwrite(buf, curlen, 1, fh) != 1)
              {
                // write error, bail out
                break;
              }

              // make sure we have a newline at the end of the line
              if(buf[curlen-1] != '\n')
              {
                if(fputc('\n', fh) == EOF)
                {
                  // write error, bail out
                  break;
                }
              }

              // update the transfer status
              TR_TransStat_Update(&ts, curlen, tr(MSG_TR_Exporting));
            }

            // check why we exited the while() loop and if everything is fine
            if(ferror(fh) != 0)
            {
              E(DBF_NET, "error on writing data! ferror(fh)=%ld", ferror(fh));

              // an error occurred, lets return failure
              success = FALSE;
            }
            else if(ferror(mfh) != 0 || feof(mfh) == 0)
            {
              E(DBF_NET, "error on reading data! ferror(mfh)=%ld feof(mfh)=%ld", ferror(mfh), feof(mfh));

              // an error occurred, lets return failure
              success = FALSE;
            }

            // close file pointer
            fclose(mfh);

            free(buf);

            // put the transferStat to 100%
            TR_TransStat_Update(&ts, TS_SETMAX, tr(MSG_TR_Exporting));
          }
          else
           success = FALSE;

          FinishUnpack(fullfile);
        }
        else
          success = FALSE;

        if(G->TR->Abort == TRUE || success == FALSE)
          break;
      }

      // close file pointer
      fclose(fh);

      // write the status to our logfile
      mnode = FirstMailNode(mlist);
      AppendToLogfile(LF_ALL, 51, tr(MSG_LOG_Exporting), ts.Msgs_Done, mnode->mail->Folder->Name, fname);
    }

    TR_TransStat_Finish(&ts);
  }

  TR_AbortnClose();

  RETURN(success);
  return success;
}
///

/*** SEND ***/
/// TR_SendMessage
// Sends a single message (-1 signals an error in DATA phase, 0 signals
// error in start phase, 1 and 2 signals success)
static int TR_SendMessage(struct TransStat *ts, struct Mail *mail)
{
  int result = 0;
  struct Folder *outfolder = FO_GetFolderByType(FT_OUTGOING, NULL);
  char *mf = GetMailFile(NULL, outfolder, mail);
  FILE *fh = NULL;
  char *buf = NULL;
  size_t buflen = SIZE_LINE;
  struct MailServerNode *msn;

  ENTER();

#warning FIXME: replace GetMailServer() usage when struct Connection is there
  if((msn = GetMailServer(&C->mailServerList, MST_SMTP, 0)) == NULL)
  {
    RETURN(-1);
    return -1;
  }

  D(DBF_NET, "about to send mail '%s' via SMTP server '%s'", mf, msn->hostname);

  // open the mail file for reading
  if((buf = malloc(buflen)) != NULL &&
     mf != NULL && (fh = fopen(mf, "r")) != NULL)
  {
    setvbuf(fh, NULL, _IOFBF, SIZE_FILEBUF);

    // now we put together our parameters for our MAIL command
    // which in fact may contain serveral parameters as well according
    // to ESMTP extensions.
    snprintf(buf, buflen, "FROM:<%s>", C->EmailAddress);

    // in case the server supports the ESMTP SIZE extension lets add the
    // size
    if(hasSIZE(msn->smtpFlags) && mail->Size > 0)
      snprintf(buf, buflen, "%s SIZE=%ld", buf, mail->Size);

    // in case the server supports the ESMTP 8BITMIME extension we can
    // add information about the encoding mode
    if(has8BITMIME(msn->smtpFlags))
      snprintf(buf, buflen, "%s BODY=%s", buf, hasServer8bit(msn) ? "8BITMIME" : "7BIT");

    // send the MAIL command with the FROM: message
    if(TR_SendSMTPCmd(SMTP_MAIL, buf, tr(MSG_ER_BADRESPONSE_SMTP)) != NULL)
    {
      struct ExtendedMail *email = MA_ExamineMail(outfolder, mail->MailFile, TRUE);

      if(email != NULL)
      {
        BOOL rcptok = TRUE;
        int j;

        // if the mail in question has some "Resent-To:" mail
        // header information we use that information instead
        // of the one of the original mail
        if(email->NoResentTo > 0)
        {
          for(j=0; j < email->NoResentTo; j++)
          {
            snprintf(buf, buflen, "TO:<%s>", email->ResentTo[j].Address);
            if(TR_SendSMTPCmd(SMTP_RCPT, buf, tr(MSG_ER_BADRESPONSE_SMTP)) == NULL)
              rcptok = FALSE;
          }
        }
        else
        {
          // specify the main 'To:' recipient
          snprintf(buf, buflen, "TO:<%s>", mail->To.Address);
          if(TR_SendSMTPCmd(SMTP_RCPT, buf, tr(MSG_ER_BADRESPONSE_SMTP)) == NULL)
            rcptok = FALSE;

          // now add the additional 'To:' recipients of the mail
          for(j=0; j < email->NoSTo && rcptok; j++)
          {
            snprintf(buf, buflen, "TO:<%s>", email->STo[j].Address);
            if(TR_SendSMTPCmd(SMTP_RCPT, buf, tr(MSG_ER_BADRESPONSE_SMTP)) == NULL)
              rcptok = FALSE;
          }

          // add the 'Cc:' recipients
          for(j=0; j < email->NoCC && rcptok; j++)
          {
            snprintf(buf, buflen, "TO:<%s>", email->CC[j].Address);
            if(TR_SendSMTPCmd(SMTP_RCPT, buf, tr(MSG_ER_BADRESPONSE_SMTP)) == NULL)
              rcptok = FALSE;
          }

          // add the 'BCC:' recipients
          for(j=0; j < email->NoBCC && rcptok; j++)
          {
            snprintf(buf, buflen, "TO:<%s>", email->BCC[j].Address);
            if(TR_SendSMTPCmd(SMTP_RCPT, buf, tr(MSG_ER_BADRESPONSE_SMTP)) == NULL)
              rcptok = FALSE;
          }
        }

        if(rcptok == TRUE)
        {
          D(DBF_NET, "RCPTs accepted, sending mail data");

          // now we send the actual main data of the mail
          if(TR_SendSMTPCmd(SMTP_DATA, NULL, tr(MSG_ER_BADRESPONSE_SMTP)) != NULL)
          {
            BOOL lineskip = FALSE;
            BOOL inbody = FALSE;
            ssize_t curlen;
            ssize_t proclen = 0;
            size_t sentbytes = 0;

            // as long there is no abort situation we go on reading out
            // from the stream and sending it to our SMTP server
            while(G->TR->Abort == FALSE && G->TR->connection->error == CONNECTERR_NO_ERROR &&
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
                  // headerlines with bcc or x-yam- will be skipped by us.
                  lineskip = (strnicmp(buf, "bcc", 3) == 0 || strnicmp(buf, "x-yam-", 6) == 0);
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
                  if(SendToHost(G->TR->connection, ".", 1, TCPF_NONE) <= 0)
                  {
                    E(DBF_NET, "couldn't send single '.' to SMTP server");

                    ER_NewError(tr(MSG_ER_CONNECTIONBROKEN), msn->hostname, (char *)SMTPcmd[SMTP_DATA]);
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
                if(curlen > 0 && SendToHost(G->TR->connection, buf, curlen, TCPF_NONE) <= 0)
                {
                  E(DBF_NET, "couldn't send buffer data to SMTP server (%ld)", curlen);

                  ER_NewError(tr(MSG_ER_CONNECTIONBROKEN), msn->hostname, (char *)SMTPcmd[SMTP_DATA]);
                  break;
                }
                else
                  sentbytes += curlen;

                // now we send the final CRLF (RFC 2822)
                if(SendToHost(G->TR->connection, "\r\n", 2, TCPF_NONE) <= 0)
                {
                  E(DBF_NET, "couldn't send CRLF to SMTP server");

                  ER_NewError(tr(MSG_ER_CONNECTIONBROKEN), msn->hostname, (char *)SMTPcmd[SMTP_DATA]);
                  break;
                }
                else
                  sentbytes += 2;
              }

              TR_TransStat_Update(ts, proclen, tr(MSG_TR_Sending));
            }

            D(DBF_NET, "transfered %ld bytes (raw: %ld bytes) error: %ld/%ld", sentbytes, mail->Size, G->TR->Abort, G->Error);

            if(G->TR->Abort == FALSE && G->TR->connection->error == CONNECTERR_NO_ERROR)
            {
              // check if any of the above getline() operations caused a ferror or
              // if we didn't walk until the end of the mail file
              if(ferror(fh) != 0 || feof(fh) == 0)
              {
                E(DBF_NET, "input mail file returned error state: ferror(fh)=%ld feof(fh)=%ld", ferror(fh), feof(fh));

                ER_NewError(tr(MSG_ER_ErrorReadMailfile), mf);
                result = -1; // signal error
              }
              else
              {
                // we have to flush the write buffer if this wasn't a error or
                // abort situation
                SendToHost(G->TR->connection, NULL, 0, TCPF_FLUSHONLY);

                // send a CRLF+octet "\r\n." to signal that the data is finished.
                // we do it here because if there was an error and we send it, the message
                // will be send incomplete.
                if(TR_SendSMTPCmd(SMTP_FINISH, NULL, tr(MSG_ER_BADRESPONSE_SMTP)) != NULL)
                {
                  // put the transferStat to 100%
                  TR_TransStat_Update(ts, TS_SETMAX, tr(MSG_TR_Sending));

                  // now that we are at 100% we have to set the transfer Date of the message
                  GetSysTimeUTC(&mail->Reference->transDate);

                  result = email->DelSend ? 2 : 1;
                  AppendToLogfile(LF_VERBOSE, 42, tr(MSG_LOG_SendingVerbose), AddrName(mail->To), mail->Subject, mail->Size);
                }
              }
            }

            if(G->TR->Abort == TRUE || G->Error == TRUE)
              result = -1; // signal the caller that we aborted within the DATA part
          }
        }

        MA_FreeEMailStruct(email);
      }
      else
        ER_NewError(tr(MSG_ER_CantOpenFile), mf);
    }

    fclose(fh);
  }
  else if(buf != NULL)
    ER_NewError(tr(MSG_ER_CantOpenFile), mf);

  free(buf);

  RETURN(result);
  return result;
}
///
/// TR_ProcessSEND
//  Sends a list of messages
BOOL TR_ProcessSEND(struct MailList *mlist, enum SendMode mode)
{
  BOOL success = FALSE;
  struct MailServerNode *msn;
  struct Connection *conn;

  ENTER();

#warning FIXME: replace GetMailServer() usage when struct Connection is there
  if((msn = GetMailServer(&C->mailServerList, MST_SMTP, 0)) == NULL)
  {
    RETURN(FALSE);
    return FALSE;
  }

  // start the PRESEND macro first
  MA_StartMacro(MACRO_PRESEND, NULL);

  // try to open the TCP/IP stack
  if((conn = CreateConnection()) != NULL && ConnectionIsOnline(conn) == TRUE)
  {
    // verify that the configuration is ready for sending mail
    if(CO_IsValid() == TRUE && (G->TR = TR_New((mode == SEND_ALL_AUTO || mode == SEND_ACTIVE_AUTO) ? TR_SEND_AUTO : TR_SEND_USER)) != NULL)
    {
      G->TR->connection = conn;

      // open the transfer window
      if(SafeOpenWindow(G->TR->GUI.WI) == TRUE)
      {
        int c;
        struct MailNode *mnode;

        NewList((struct List *)&G->TR->transferList);
        G->TR_Allow = FALSE;
        G->TR->Abort = FALSE;
        G->Error = FALSE;

        // now we build the list of mails which should
        // be transfered.
        c = 0;

        LockMailListShared(mlist);

        ForEachMailNode(mlist, mnode)
        {
          struct Mail *mail = mnode->mail;

          if(mail != NULL)
          {
            if(hasStatusQueued(mail) || hasStatusError(mail))
            {
              struct MailTransferNode *mtn;

              if((mtn = calloc(1, sizeof(struct MailTransferNode))) != NULL)
              {
                struct Mail *newMail;

                if((newMail = memdup(mail, sizeof(struct Mail))) != NULL)
                {
                  newMail->Reference = mail;

                  // set index and transfer flags to LOAD
                  mtn->index = ++c;
                  mtn->tflags = TRF_LOAD;
                  mtn->mail = newMail;

                  AddTail((struct List *)&G->TR->transferList, (struct Node *)mtn);
                }
                else
                  free(mtn);
              }
            }
          }
        }

        UnlockMailList(mlist);

        // just go on if we really have something
        if(c > 0)
        {
          // now we have to check whether SSL/TLS is selected for SMTP account,
          // and if it is usable. Or if no secure connection is requested
          // we can go on right away.
          if((hasServerSSL(msn) == FALSE && hasServerTLS(msn) == FALSE) ||
             G->TR_UseableTLS == TRUE)
          {
            enum ConnectError err;
            int port;
            char *p;
            char host[SIZE_HOST];
            struct TransStat ts;

            G->TR->SearchCount = AllocFilterSearch(APPLY_SENT);
            TR_TransStat_Init(&ts);
            TR_TransStat_Start(&ts);
            strlcpy(host, msn->hostname, sizeof(host));

            // If the hostname has a explicit :xxxxx port statement at the end we
            // take this one, even if its not needed anymore.
            if((p = strchr(host, ':')) != NULL)
            {
              *p = '\0';
              port = atoi(++p);
            }
            else
              port = msn->port;

            set(G->TR->GUI.TX_STATUS, MUIA_Text_Contents, tr(MSG_TR_Connecting));

            BusyText(tr(MSG_TR_MailTransferTo), host);

            TR_SetWinTitle(FALSE, host);

            if((err = ConnectToHost(G->TR->connection, host, port)) == CONNECTERR_SUCCESS)
            {
              BOOL connected = FALSE;

              // first we check whether the user wants to connect to a plain SSLv3 server
              // so that we initiate the SSL connection now
              if(hasServerSSL(msn) == TRUE)
              {
                // lets try to establish the SSL connection via AmiSSL
                if(MakeSecureConnection(G->TR->connection) == TRUE)
                  G->TR_UseTLS = TRUE;
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
                connected = TR_ConnectSMTP();

                // Now we have to check whether the user has selected SSL/TLS
                // and then we have to initiate the STARTTLS command followed by the TLS negotiation
                if(hasServerTLS(msn) == TRUE && connected == TRUE)
                {
                  connected = TR_InitSTARTTLS();

                  // then we have to refresh the SMTPflags and check
                  // again what features we have after the STARTTLS
                  if(connected == TRUE)
                  {
                    // first we flag this connection as a sucessfull
                    // TLS session
                    G->TR_UseTLS = TRUE;

                    // now run the connect SMTP function again
                    // so that the SMTP server flags will be refreshed
                    // accordingly.
                    connected = TR_ConnectSMTP();
                  }
                }

                // If the user selected SMTP_AUTH we have to initiate
                // a AUTH connection
                if(hasServerAuth(msn) == TRUE && connected == TRUE)
                  connected = TR_InitSMTPAUTH();
              }

              // If we are still "connected" we can proceed with transfering the data
              if(connected == TRUE)
              {
                struct Folder *outfolder = FO_GetFolderByType(FT_OUTGOING, NULL);
                struct Folder *sentfolder = FO_GetFolderByType(FT_SENT, NULL);
                struct Node *curNode;

                // set the success to TRUE as everything worked out fine
                // until here.
                success = TRUE;
                AppendToLogfile(LF_VERBOSE, 41, tr(MSG_LOG_ConnectSMTP), host);

                IterateList(&G->TR->transferList, curNode)
                {
                  struct MailTransferNode *mtn = (struct MailTransferNode *)curNode;
                  struct Mail *mail = mtn->mail;

                  if(G->TR->Abort == TRUE || G->Error == TRUE)
                    break;

                  TR_TransStat_NextMsg(&ts, mtn->index, -1, mail->Size, tr(MSG_TR_Sending));

                  switch(TR_SendMessage(&ts, mail))
                  {
                    // -1 means that SendMessage was aborted within the
                    // DATA part and so we cannot issue a RSET command and have to abort
                    // immediatly by leaving the mailserver alone.
                    case -1:
                    {
                      setStatusToError(mail->Reference);
                      G->Error = TRUE;
                    }
                    break;

                    // 0 means that a error occured before the DATA part and
                    // so we can abort the transaction cleanly by a RSET and QUIT
                    case 0:
                    {
                      setStatusToError(mail->Reference);
                      TR_SendSMTPCmd(SMTP_RSET, NULL, NULL); // no error check
                      G->Error = FALSE;
                    }
                    break;

                    // 1 means we filter the mails and then copy/move the mail to the send folder
                    case 1:
                    {
                      setStatusToSent(mail->Reference);
                      if(TR_ApplySentFilters(mail->Reference) == TRUE)
                        MA_MoveCopy(mail->Reference, outfolder, sentfolder, FALSE, TRUE);
                    }
                    break;

                    // 2 means we filter and delete afterwards
                    case 2:
                    {
                      setStatusToSent(mail->Reference);
                      if (TR_ApplySentFilters(mail->Reference) == TRUE)
                        MA_DeleteSingle(mail->Reference, DELF_UPDATE_APPICON);
                    }
                    break;
                  }
                }

                if(G->Error == FALSE)
                  AppendToLogfile(LF_NORMAL, 40, tr(MSG_LOG_Sending), c, host);
                else
                  AppendToLogfile(LF_NORMAL, 40, tr(MSG_LOG_SENDING_FAILED), c, host);

                // now we can disconnect from the SMTP
                // server again
                set(G->TR->GUI.TX_STATUS, MUIA_Text_Contents, tr(MSG_TR_Disconnecting));

                // send a 'QUIT' command, but only if
                // we didn't receive any error during the transfer
                if(G->Error == FALSE)
                  TR_SendSMTPCmd(SMTP_QUIT, NULL, tr(MSG_ER_BADRESPONSE_SMTP));
              }
              else
              {
                // check if we end up here cause of the 8BITMIME differences
                if(has8BITMIME(msn->smtpFlags) == FALSE && hasServer8bit(msn) == TRUE)
                {
                  W(DBF_NET, "incorrect Allow8bit setting!");
                  err = CONNECTERR_INVALID8BIT;
                }
                else if(err != CONNECTERR_SSLFAILED)
                  err = CONNECTERR_UNKNOWN_ERROR;
              }

              // make sure to shutdown the socket
              // and all possible SSL connection stuff
              TR_Disconnect();
            }

            TR_TransStat_Finish(&ts);

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
                ER_NewError(tr(MSG_ER_CONNECTERR_SOCKET_IN_USE_SMTP), host);
              break;

              // socket() execution failed
              case CONNECTERR_NO_SOCKET:
                ER_NewError(tr(MSG_ER_CONNECTERR_NO_SOCKET_SMTP), host);
              break;

              // couldn't establish non-blocking IO
              case CONNECTERR_NO_NONBLOCKIO:
                ER_NewError(tr(MSG_ER_CONNECTERR_NO_NONBLOCKIO_SMTP), host);
              break;

              // the specified hostname isn't valid, so
              // lets tell the user
              case CONNECTERR_UNKNOWN_HOST:
                ER_NewError(tr(MSG_ER_UNKNOWN_HOST_SMTP), host);
              break;

              // the connection request timed out, so tell
              // the user
              case CONNECTERR_TIMEDOUT:
                ER_NewError(tr(MSG_ER_CONNECTERR_TIMEDOUT_SMTP), host);
              break;

              // an error occurred while checking for 8bit MIME
              // compatibility
              case CONNECTERR_INVALID8BIT:
                ER_NewError(tr(MSG_ER_NO8BITMIME_SMTP), host);
              break;

              // error during initialization of an SSL connection
              case CONNECTERR_SSLFAILED:
                ER_NewError(tr(MSG_ER_INITTLS_SMTP), host);
              break;

              // an unknown error occurred so lets show
              // a generic error message
              case CONNECTERR_UNKNOWN_ERROR:
                ER_NewError(tr(MSG_ER_CANNOT_CONNECT_SMTP), host);
              break;

              case CONNECTERR_NO_CONNECTION:
              case CONNECTERR_NOT_CONNECTED:
                // cannot happen, do nothing
              break;
            }

            FreeFilterSearch();
            G->TR->SearchCount = 0;

            BusyEnd();
          }
          else
            ER_NewError(tr(MSG_ER_UNUSABLEAMISSL));
        }
      }

      // make sure the transfer window is
      // closed, the transfer list cleanup
      // and everything disposed
      TR_AbortnClose();
    }
  }
  else
    ER_NewError(tr(MSG_ER_OPENTCPIP));

  // make sure to close the TCP/IP
  // connection completly
  DeleteConnection(conn);

  // start the POSTSEND macro so that others
  // notice that the send process finished.
  MA_StartMacro(MACRO_POSTSEND, NULL);

  RETURN(success);
  return success;
}
///

/*** IMPORT ***/
/// TR_AddMessageHeader
//  Parses downloaded message header
static struct MailTransferNode *TR_AddMessageHeader(int *count, int size, long addr, char *tfname)
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

        AddTail((struct List *)&G->TR->transferList, (struct Node *)mtn);

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
      if((mtn = TR_AddMessageHeader(mail_accu, size, msg_addr, FilePart(outFileName))))
      {
        SET_FLAG(mtn->mail->sflags, mailStatusFlags);
      }

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

        E(DBF_IMPORT, "Failed to read node at %08lx", chld);
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
  if(G->TR->ImportFile[0] != '\0' && G->TR->ImportFolder != NULL)
  {
    char tfname[SIZE_MFILE];
    char fname[SIZE_PATHFILE];
    int c = 0;

    // let the application sleep while we parse the file
    set(G->App, MUIA_Application_Sleep, TRUE);

    // clear the found mail list per default
    NewList((struct List *)&G->TR->transferList);

    // prepare the temporary filename buffers
    snprintf(tfname, sizeof(tfname), "YAMi%08x.tmp", (unsigned int)GetUniqueID());
    AddPath(fname, C->TempDir, tfname, sizeof(fname));

    // before this function is called the MA_ImportMessages() function
    // already found out which import format we can expect. So we
    // distinguish between the different known formats here
    switch(G->TR->ImportFormat)
    {
      // treat the file as a MBOX compliant file
      case IMF_MBOX:
      {
        FILE *ifh;

        D(DBF_IMPORT, "trying to retrieve mail list from MBOX compliant file");

        if((ifh = fopen(G->TR->ImportFile, "r")) != NULL)
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

                result = (TR_AddMessageHeader(&c, size, addr, tfname) != NULL);
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
            result = (TR_AddMessageHeader(&c, size, addr, tfname) != NULL);
            DeleteFile(fname);
          }
          else if(ofh != NULL)
          {
            fclose(ofh);
            ofh = NULL;
            DeleteFile(fname);
          }

          fclose(ifh);

          free(buffer);
        }
        else
          E(DBF_IMPORT, "Error on trying to open file '%s'", G->TR->ImportFile);
      }
      break;

      // treat the file as a file that contains a single
      // unencoded mail (*.eml alike file)
      case IMF_PLAIN:
      {
        if(CopyFile(fname, NULL, G->TR->ImportFile, NULL) == TRUE)
        {
          LONG size;

          ObtainFileInfo(fname, FI_SIZE, &size);
          // if the file was identified as a plain .eml file we
          // just have to go and call TR_AddMessageHeader to let
          // YAM analyze the file
          result = (TR_AddMessageHeader(&c, size, 0, tfname) != NULL);

          DeleteFile(fname);
        }
      }
      break;

      // treat the file as a DBX (Outlook Express) compliant mail archive
      case IMF_DBX:
      {
        FILE *ifh;

        // lets open the file and read out the root node of the dbx mail file
        if((ifh = fopen(G->TR->ImportFile, "rb")) != NULL)
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

    TR_DisplayMailList(FALSE);

    // if everything went fine but we didn't find any mail to import we signal failure
    if(c == 0)
      result = FALSE;

    // wake up the application again
    set(G->App, MUIA_Application_Sleep, FALSE);
  }

  RETURN(result);
  return result;
}
///
/// TR_AbortIMPORTFunc
//  Aborts import process
HOOKPROTONHNONP(TR_AbortIMPORTFunc, void)
{
   TR_AbortnClose();
}
MakeStaticHook(TR_AbortIMPORTHook, TR_AbortIMPORTFunc);
///
/// TR_ProcessIMPORTFunc
//  Imports messages from a MBOX mailbox file
HOOKPROTONHNONP(TR_ProcessIMPORTFunc, void)
{
  ENTER();

  // if there is nothing to import we can skip
  // immediately.
  if(IsMinListEmpty(&G->TR->transferList) == FALSE)
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

            // iterate through our transferList and seek to
            // each position/address of a mail
            IterateList(&G->TR->transferList, curNode)
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

            // iterate through our transferList and seek to
            // each position/address of a mail
            IterateList(&G->TR->transferList, curNode)
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

  LEAVE();
}
MakeHook(TR_ProcessIMPORTHook, TR_ProcessIMPORTFunc);
///

/*** GET ***/
/// TR_AbortGETFunc
//  Aborts a POP3 download
HOOKPROTONHNONP(TR_AbortGETFunc, void)
{
  ENTER();

  // make sure the NOOP timer is definitly stopped
  StopTimer(TIMER_POP3_KEEPALIVE);

  // first set the Abort variable so that other can benefit from it
  G->TR->Abort = TRUE;

  // we can easily abort the transfer by setting the POP_Nr to -1
  // and issue a GetMailFromNextPOP command. With this solution YAM will
  // also process the filters for mails that were already downloaded
  // even if the user aborted the transfer somehow
  G->TR->POP_Nr = -1;
  TR_GetMailFromNextPOP(FALSE, -1, 0);

  LEAVE();
}
MakeStaticHook(TR_AbortGETHook, TR_AbortGETFunc);
///
/// TR_LoadMessage
//  Retrieves a message from the POP3 server
static BOOL TR_LoadMessage(struct Folder *infolder, struct TransStat *ts, const int number)
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
      if(TR_RecvToFile(fh, msgfile, ts) > 0)
        done = TRUE;
    }
    fclose(fh);

    if(G->TR->Abort == FALSE && G->TR->connection->error == CONNECTERR_NO_ERROR && done == TRUE)
    {
      struct ExtendedMail *mail;

      if((mail = MA_ExamineMail(infolder, mfile, FALSE)) != NULL)
      {
        struct Mail *newMail;

        if((newMail = AddMailToList(&mail->Mail, infolder)) != NULL)
        {
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
static BOOL TR_DeleteMessage(struct TransStat *ts, int number)
{
  BOOL result = FALSE;
  char msgnum[SIZE_SMALL];

  ENTER();

  snprintf(msgnum, sizeof(msgnum), "%d", number);

  TR_TransStat_Update(ts, TS_SETMAX, tr(MSG_TR_DeletingServerMail));

  if(TR_SendPOP3Cmd(POPCMD_DELE, msgnum, tr(MSG_ER_BADRESPONSE_POP3)) != NULL)
  {
    G->TR->Stats.Deleted++;
    result = TRUE;
  }

  RETURN(result);
  return result;
}
///
/// TR_NewMailAlert
//  Notifies user when new mail is available
static void TR_NewMailAlert(void)
{
  struct DownloadResult *stats = &G->TR->Stats;
  struct RuleResult *rr = &G->RuleResults;

  ENTER();

  SHOWVALUE(DBF_NET, stats->Downloaded);
  SHOWVALUE(DBF_NET, rr->Spam);

  // show the statistics only if we downloaded some mails at all,
  // and not all of them were spam mails
  if(stats->Downloaded > 0 && stats->Downloaded > rr->Spam)
  {
    if(hasRequesterNotify(C->NotifyType) && G->TR->GUIlevel != POP_REXX)
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
        MUIA_Window_Title, tr(MSG_TR_NewMail),
        MUIA_Window_RefWindow, G->MA->GUI.WI,
        MUIA_Window_Activate, G->TR->GUIlevel == POP_USER,
        MUIA_InfoWindow_Body, buffer,
      End;
    }

    #if defined(__amigaos4__)
    if(hasOS41SystemNotify(C->NotifyType))
    {
      D(DBF_GUI, "appID is %ld, application.lib is V%ld.%ld (needed V%ld.%ld)", G->applicationID, ApplicationBase->lib_Version, ApplicationBase->lib_Revision, 53, 7);
      // Notify() is V53.2+, but 53.7 fixes some serious issues
      if(G->applicationID > 0 && LIB_VERSION_IS_AT_LEAST(ApplicationBase, 53, 7) == TRUE)
      {
        // 128 chars is the current maximum :(
        char message[128];
        int count = stats->Downloaded - rr->Spam;

        // distinguish between single and multiple mails
        if(count >= 2)
          snprintf(message, sizeof(message), tr(MSG_TR_NEW_MAIL_NOTIFY_MANY), count);
        else
          strlcpy(message, tr(MSG_TR_NEW_MAIL_NOTIFY_ONE), sizeof(message));

        // We require 53.7+. From this version on proper tag values are used, hence there
        // is no need to distinguish between v1 and v2 interfaces here as we have to do for
        // other application.lib functions.
        Notify(G->applicationID, APPNOTIFY_Title, (uint32)"YAM",
                                 APPNOTIFY_PubScreenName, (uint32)"FRONT",
                                 APPNOTIFY_Text, (uint32)message,
                                 APPNOTIFY_CloseOnDC, TRUE,
                                 APPNOTIFY_BackMsg, (uint32)"POPUP",
                                 TAG_DONE);
      }
    }
    #endif // __amigaos4__

    if(hasCommandNotify(C->NotifyType))
      LaunchCommand(C->NotifyCommand, FALSE, OUT_STDOUT);

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
  struct TransStat ts;

  ENTER();

  // initialize the transfer statistics
  TR_TransStat_Init(&ts);

  // make sure the NOOP timer is definitly stopped
  StopTimer(TIMER_POP3_KEEPALIVE);

  if(ts.Msgs_Tot > 0)
  {
    struct Folder *infolder = FO_GetFolderByType(FT_INCOMING, NULL);
    struct Node *curNode;
    struct MUI_NListtree_TreeNode *incomingTreeNode = FO_GetFolderTreeNode(infolder);

    if(C->TransferWindow == TWM_SHOW && xget(G->TR->GUI.WI, MUIA_Window_Open) == FALSE)
      set(G->TR->GUI.WI, MUIA_Window_Open, TRUE);

    TR_TransStat_Start(&ts);

    IterateList(&G->TR->transferList, curNode)
    {
      struct MailTransferNode *mtn = (struct MailTransferNode *)curNode;
      struct Mail *mail = mtn->mail;

      D(DBF_NET, "download flags %08lx=%s%s%s for mail with subject '%s' and size %ld",mtn->tflags, hasTR_LOAD(mtn) ? "TR_LOAD " : "" , hasTR_DELETE(mtn) ? "TR_DELETE " : "", hasTR_PRESELECT(mtn) ? "TR_PRESELECT " : "", mail->Subject, mail->Size);
      if(hasTR_LOAD(mtn))
      {
        D(DBF_NET, "downloading mail with subject '%s' and size %ld", mail->Subject, mail->Size);

        // update the transfer status
        TR_TransStat_NextMsg(&ts, mtn->index, mtn->position, mail->Size, tr(MSG_TR_Downloading));

        if(TR_LoadMessage(infolder, &ts, mtn->index) == TRUE)
        {
          // redraw the folderentry in the listtree
          DoMethod(G->MA->GUI.NL_FOLDERS, MUIM_NListtree_Redraw, incomingTreeNode, MUIF_NONE);

          // put the transferStat for this mail to 100%
          TR_TransStat_Update(&ts, TS_SETMAX, tr(MSG_TR_Downloading));

          G->TR->Stats.Downloaded++;

          if(hasTR_DELETE(mtn))
          {
            D(DBF_NET, "deleting mail with subject '%s' on server", mail->Subject);

            if(TR_DeleteMessage(&ts, mtn->index) == TRUE && G->TR->DuplicatesChecking == TRUE)
            {
              // remove the UIDL from the hash table and remember that change
              RemoveUIDLfromHash(mtn->UIDL);
              G->TR->UIDLhashIsDirty = TRUE;
            }
          }
          else if(G->TR->DuplicatesChecking == TRUE)
          {
            D(DBF_NET, "adding mail with subject '%s' to UIDL hash", mail->Subject);
            // add the UIDL to the hash table and remember that change
            AddUIDLtoHash(mtn->UIDL, TRUE);
            G->TR->UIDLhashIsDirty = TRUE;
          }
          else
            D(DBF_NET, "leaving mail with subject '%s' and size %ld on server to be downloaded again", mail->Subject, mail->Size);
        }
      }
      else if(hasTR_DELETE(mtn))
      {
        D(DBF_NET, "deleting mail with subject '%s' on server", mail->Subject);

        if(TR_DeleteMessage(&ts, mtn->index) == TRUE && G->TR->DuplicatesChecking == TRUE)
        {
          // remove the UIDL from the hash table and remember that change
          RemoveUIDLfromHash(mtn->UIDL);
          G->TR->UIDLhashIsDirty = TRUE;
        }
      }
      else
        D(DBF_NET, "leaving mail with subject '%s' and size %ld on server to be downloaded again", mail->Subject, mail->Size);

      if(G->TR->Abort == TRUE || G->TR->connection->error != CONNECTERR_NO_ERROR)
        break;
    }

    TR_TransStat_Finish(&ts);

    DisplayStatistics(infolder, TRUE);

    // in case the current folder after the download
    // is the incoming folder we have to update
    // the main toolbar and menu items
    if(FO_GetCurrentFolder() == infolder)
      MA_ChangeSelected(TRUE);
  }

  TR_GetMailFromNextPOP(FALSE, 0, 0);

  LEAVE();
}
MakeHook(TR_ProcessGETHook, TR_ProcessGETFunc);

///
/// TR_GetMessageInfoFunc
//  Requests message header of a message selected by the user
HOOKPROTONHNONP(TR_GetMessageInfoFunc, void)
{
  int line;
  struct MailTransferNode *mtn;

  ENTER();

  line = xget(G->TR->GUI.LV_MAILS, MUIA_NList_Active);
  DoMethod(G->TR->GUI.LV_MAILS, MUIM_NList_GetEntry, line, &mtn);
  TR_GetMessageDetails(mtn, line);

  LEAVE();
}
MakeStaticHook(TR_GetMessageInfoHook, TR_GetMessageInfoFunc);
///
/// TR_CompleteMsgList
//  Gets details for messages on server
static void TR_CompleteMsgList(void)
{
  struct TR_ClassData *tr = G->TR;

  ENTER();

  // first we have to set the notifies to the default values.
  // this is needed so that if we get mail from more than one POP3 at a line this
  // abort stuff works out
  set(G->TR->GUI.BT_PAUSE, MUIA_Disabled, FALSE);
  set(G->TR->GUI.BT_RESUME, MUIA_Disabled, TRUE);
  DoMethod(tr->GUI.BT_START, MUIM_KillNotify, MUIA_Pressed);
  DoMethod(tr->GUI.BT_START, MUIM_Notify, MUIA_Pressed, FALSE, MUIV_Notify_Application, 3, MUIM_WriteLong, TRUE, &(tr->Start));
  DoMethod(tr->GUI.BT_QUIT , MUIM_KillNotify, MUIA_Pressed);
  DoMethod(tr->GUI.BT_QUIT , MUIM_Notify, MUIA_Pressed, FALSE, MUIV_Notify_Application, 3, MUIM_WriteLong, TRUE, &(tr->Abort));

  if(C->PreSelection < PSM_ALWAYSLARGE)
  {
    struct MinNode *curNode = tr->GMD_Mail;

    for(; curNode->mln_Succ && tr->Abort == FALSE && tr->connection->error == CONNECTERR_NO_ERROR; curNode = curNode->mln_Succ)
    {
      struct MailTransferNode *mtn = (struct MailTransferNode *)curNode;

      if(tr->Pause == TRUE)
        break;

      if(tr->Start == TRUE)
      {
        TR_ProcessGETFunc();
        break;
      }

      if(C->PreSelection != PSM_LARGE || mtn->mail->Size >= C->WarnSize*1024)
      {
        TR_GetMessageDetails(mtn, tr->GMD_Line++);

        // set the next mail as the active one for the display,
        // so that if the user pauses we can go on here
        tr->GMD_Mail = curNode->mln_Succ;
      }
    }
  }

  set(G->TR->GUI.BT_PAUSE, MUIA_Disabled, TRUE);
  DoMethod(tr->GUI.BT_START, MUIM_KillNotify, MUIA_Pressed);
  DoMethod(tr->GUI.BT_START, MUIM_Notify, MUIA_Pressed, FALSE, MUIV_Notify_Application, 2, MUIM_CallHook, &TR_ProcessGETHook);
  DoMethod(tr->GUI.BT_QUIT , MUIM_KillNotify, MUIA_Pressed);
  DoMethod(tr->GUI.BT_QUIT , MUIM_Notify, MUIA_Pressed, FALSE, MUIV_Notify_Application, 2, MUIM_CallHook, &TR_AbortGETHook);

  if(tr->Abort == TRUE)
    TR_AbortGETFunc();
  else
  {
    // start the timer which makes sure we send
    // a regular NOOP command to the POP3 server
    // so that it doesn't drop the connection
    RestartTimer(TIMER_POP3_KEEPALIVE, C->KeepAliveInterval, 0);
  }

  LEAVE();
}
///
/// TR_PauseFunc
//  Pauses or resumes message download
HOOKPROTONHNO(TR_PauseFunc, void, int *arg)
{
  BOOL pause = *arg;

  ENTER();

  set(G->TR->GUI.BT_RESUME, MUIA_Disabled, pause == FALSE);
  set(G->TR->GUI.BT_PAUSE,  MUIA_Disabled, pause == TRUE);

  if(pause == TRUE)
  {
    // start the timer which makes sure we send
    // a regular NOOP command to the POP3 server
    // so that it doesn't drop the connection
    RestartTimer(TIMER_POP3_KEEPALIVE, C->KeepAliveInterval, 0);
  }
  else
  {
    G->TR->Pause = FALSE;
    TR_CompleteMsgList();
  }

  LEAVE();
}
MakeStaticHook(TR_PauseHook, TR_PauseFunc);
///

/*** GUI ***/
/// TR_New
//  Creates transfer window
struct TR_ClassData *TR_New(enum TransferType TRmode)
{
  struct TR_ClassData *data;

  ENTER();

  if((data = calloc(1, sizeof(struct TR_ClassData))) != NULL)
  {
    if((data->downloadedMails = CreateMailList()) != NULL)
    {
      Object *bt_all = NULL, *bt_none = NULL, *bt_loadonly = NULL, *bt_loaddel = NULL, *bt_delonly = NULL, *bt_leave = NULL;
      Object *gr_sel, *gr_proc, *gr_win;
      BOOL fullwin = (TRmode == TR_GET_USER || TRmode == TR_GET_AUTO || TRmode == TR_IMPORT);
      static char status_label[SIZE_DEFAULT];
      static char size_gauge_label[SIZE_DEFAULT];
      static char msg_gauge_label[SIZE_DEFAULT];
      static char str_size_done[SIZE_SMALL];
      static char str_size_tot[SIZE_SMALL];
      static char str_speed[SIZE_SMALL];
      static char str_size_curr[SIZE_SMALL];
      static char str_size_curr_max[SIZE_SMALL];

      NewList((struct List *)&data->transferList);

      // prepare the initial text object content
      FormatSize(0, str_size_done, sizeof(str_size_done), SF_MIXED);
      FormatSize(0, str_size_tot, sizeof(str_size_tot), SF_MIXED);
      FormatSize(0, str_speed, sizeof(str_speed), SF_MIXED);
      snprintf(status_label, sizeof(status_label), tr(MSG_TR_TRANSFERSTATUS),
                                      str_size_done, str_size_tot, str_speed, 0, 0, 0, 0);

      snprintf(msg_gauge_label, sizeof(msg_gauge_label), tr(MSG_TR_MESSAGEGAUGE), 0);

      FormatSize(0, str_size_curr, sizeof(str_size_curr), SF_AUTO);
      FormatSize(0, str_size_curr_max, sizeof(str_size_curr_max), SF_AUTO);
      snprintf(size_gauge_label, sizeof(size_gauge_label), tr(MSG_TR_TRANSFERSIZE),
                                                           str_size_curr, str_size_curr_max);

      gr_proc = ColGroup(2), GroupFrameT(tr(MSG_TR_Status)),
         Child, data->GUI.TX_STATS = TextObject,
            MUIA_Text_Contents, status_label,
            MUIA_Background,    MUII_TextBack,
            MUIA_Frame,         MUIV_Frame_Text,
            MUIA_Text_PreParse, MUIX_C,
         End,
         Child, VGroup,
            Child, data->GUI.GA_COUNT = GaugeObject,
               GaugeFrame,
               MUIA_Gauge_Horiz,    TRUE,
               MUIA_Gauge_InfoText, msg_gauge_label,
            End,
            Child, data->GUI.GA_BYTES = GaugeObject,
               GaugeFrame,
               MUIA_Gauge_Horiz,    TRUE,
               MUIA_Gauge_InfoText, size_gauge_label,
            End,
         End,
         Child, data->GUI.TX_STATUS = TextObject,
            MUIA_Background,MUII_TextBack,
            MUIA_Frame     ,MUIV_Frame_Text,
         End,
         Child, data->GUI.BT_ABORT = MakeButton(tr(MSG_TR_Abort)),
      End;

      if(fullwin == TRUE)
      {
        data->GUI.GR_LIST = VGroup, GroupFrameT(TRmode==TR_IMPORT ? tr(MSG_TR_MsgInFile) : tr(MSG_TR_MsgOnServer)),
           MUIA_ShowMe, TRmode==TR_IMPORT || C->PreSelection >= PSM_ALWAYS,
           Child, NListviewObject,
              MUIA_CycleChain, TRUE,
              MUIA_NListview_NList, data->GUI.LV_MAILS = TransferMailListObject,
              End,
           End,
        End;
        gr_sel = VGroup, GroupFrameT(tr(MSG_TR_Control)),
           Child, ColGroup(5),
              Child, bt_all = MakeButton(tr(MSG_TR_All)),
              Child, bt_loaddel = MakeButton(tr(MSG_TR_DownloadDelete)),
              Child, bt_leave = MakeButton(tr(MSG_TR_Leave)),
              Child, HSpace(0),
              Child, data->GUI.BT_PAUSE = MakeButton(tr(MSG_TR_Pause)),
              Child, bt_none = MakeButton(tr(MSG_TR_Clear)),
              Child, bt_loadonly = MakeButton(tr(MSG_TR_DownloadOnly)),
              Child, bt_delonly = MakeButton(tr(MSG_TR_DeleteOnly)),
              Child, HSpace(0),
              Child, data->GUI.BT_RESUME = MakeButton(tr(MSG_TR_Resume)),
           End,
           Child, ColGroup(2),
              Child, data->GUI.BT_START = MakeButton(tr(MSG_TR_Start)),
              Child, data->GUI.BT_QUIT = MakeButton(tr(MSG_TR_Abort)),
           End,
        End;
        gr_win = VGroup,
           Child, data->GUI.GR_LIST,
           Child, data->GUI.GR_PAGE = PageGroup,
              Child, gr_sel,
              Child, gr_proc,
           End,
        End;
      }
      else
      {
        gr_win = VGroup, MUIA_Frame, MUIV_Frame_None,
           Child, gr_proc,
        End;
      }

      data->GUI.WI = WindowObject,
         MUIA_Window_ID, MAKE_ID('T','R','A','0'+TRmode),
         MUIA_Window_CloseGadget, FALSE,
         MUIA_Window_Activate, (TRmode == TR_IMPORT || TRmode == TR_EXPORT || TRmode == TR_GET_USER || TRmode == TR_SEND_USER),
         MUIA_HelpNode, "TR_W",
         WindowContents, gr_win,
      End;

      if(data->GUI.WI != NULL)
      {
        DoMethod(G->App, OM_ADDMEMBER, data->GUI.WI);
        SetHelp(data->GUI.TX_STATUS,MSG_HELP_TR_TX_STATUS);
        SetHelp(data->GUI.BT_ABORT ,MSG_HELP_TR_BT_ABORT);

        if(fullwin == TRUE)
        {
          set(data->GUI.WI, MUIA_Window_DefaultObject, data->GUI.LV_MAILS);
          set(data->GUI.BT_RESUME, MUIA_Disabled, TRUE);

          if(TRmode == TR_IMPORT)
          {
            set(data->GUI.BT_PAUSE, MUIA_Disabled, TRUE);
            set(bt_delonly        , MUIA_Disabled, TRUE);
            set(bt_loaddel        , MUIA_Disabled, TRUE);
            DoMethod(data->GUI.BT_START, MUIM_Notify, MUIA_Pressed, FALSE, MUIV_Notify_Application, 2, MUIM_CallHook, &TR_ProcessIMPORTHook);
            DoMethod(data->GUI.BT_QUIT , MUIM_Notify, MUIA_Pressed, FALSE, MUIV_Notify_Application, 2, MUIM_CallHook, &TR_AbortIMPORTHook);
          }
          else
          {
            set(data->GUI.GR_PAGE, MUIA_Group_ActivePage, 1);
            DoMethod(data->GUI.BT_RESUME,MUIM_Notify, MUIA_Pressed, FALSE, MUIV_Notify_Application, 3, MUIM_CallHook, &TR_PauseHook, FALSE);
            DoMethod(data->GUI.BT_PAUSE ,MUIM_Notify, MUIA_Pressed, FALSE, MUIV_Notify_Application, 3, MUIM_CallHook, &TR_PauseHook, TRUE);
            DoMethod(data->GUI.BT_PAUSE, MUIM_Notify, MUIA_Pressed, FALSE, MUIV_Notify_Application, 3, MUIM_WriteLong, TRUE, &(data->Pause));
            DoMethod(data->GUI.LV_MAILS ,MUIM_Notify, MUIA_NList_DoubleClick,TRUE, MUIV_Notify_Application, 2, MUIM_CallHook, &TR_GetMessageInfoHook);
            DoMethod(bt_delonly,         MUIM_Notify, MUIA_Pressed, FALSE, MUIV_Notify_Application, 3, MUIM_CallHook, &TR_ChangeTransFlagsHook, TRF_DELETE);
            DoMethod(bt_loaddel,         MUIM_Notify, MUIA_Pressed, FALSE, MUIV_Notify_Application, 3, MUIM_CallHook, &TR_ChangeTransFlagsHook, (TRF_LOAD|TRF_DELETE));
            DoMethod(data->GUI.BT_START, MUIM_Notify, MUIA_Pressed, FALSE, MUIV_Notify_Application, 3, MUIM_WriteLong, TRUE, &(data->Start));
            DoMethod(data->GUI.BT_QUIT , MUIM_Notify, MUIA_Pressed, FALSE, MUIV_Notify_Application, 3, MUIM_WriteLong, TRUE, &(data->Abort));
          }
          DoMethod(bt_loadonly,        MUIM_Notify, MUIA_Pressed, FALSE, MUIV_Notify_Application, 3, MUIM_CallHook, &TR_ChangeTransFlagsHook, TRF_LOAD);
          DoMethod(bt_leave,           MUIM_Notify, MUIA_Pressed, FALSE, MUIV_Notify_Application, 3, MUIM_CallHook, &TR_ChangeTransFlagsHook, TRF_NONE);
          DoMethod(bt_all,             MUIM_Notify, MUIA_Pressed, FALSE, data->GUI.LV_MAILS, 4, MUIM_NList_Select, MUIV_NList_Select_All, MUIV_NList_Select_On, NULL);
          DoMethod(bt_none,            MUIM_Notify, MUIA_Pressed, FALSE, data->GUI.LV_MAILS, 4, MUIM_NList_Select, MUIV_NList_Select_All, MUIV_NList_Select_Off, NULL);
        }
        DoMethod(data->GUI.BT_ABORT, MUIM_Notify, MUIA_Pressed, FALSE, MUIV_Notify_Application, 3, MUIM_WriteLong, TRUE, &(data->Abort));
        MA_ChangeTransfer(FALSE);
      }
      else
      {
        DeleteMailList(data->downloadedMails);
        free(data);
        data = NULL;
      }
    }
    else
    {
      free(data);
      data = NULL;
    }
  }

  RETURN(data);
  return data;
}
///

