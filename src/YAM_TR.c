/***************************************************************************

 YAM - Yet Another Mailer
 Copyright (C) 1995-2000 by Marcel Beck <mbeck@yam.ch>
 Copyright (C) 2000-2007 by YAM Open Source Team

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

#include <netinet/ip.h>
#include <netinet/tcp.h>
#include <sys/socket.h>
#include <sys/filio.h>

#include <clib/alib_protos.h>
#include <clib/macros.h>
#include <libraries/iffparse.h>
#include <libraries/genesis.h>
#include <mui/NList_mcc.h>
#include <mui/NListview_mcc.h>
#include <proto/amissl.h>
#include <proto/bsdsocket.h>
#include <proto/dos.h>
#include <proto/exec.h>
#include <proto/intuition.h>
#include <proto/muimaster.h>
#include <proto/timer.h>
#include <proto/utility.h>

#if !defined(__amigaos4__)
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
#include "YAM_locale.h"
#include "YAM_main.h"
#include "YAM_mainFolder.h"
#include "YAM_md5.h"
#include "YAM_mime.h"
#include "YAM_utilities.h"
#include "classes/Classes.h"

#include "HashTable.h"
#include "FileInfo.h"

#include "Debug.h"

struct TransStat
{
  int   Msgs_Tot;
  int   Msgs_Done;
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

#define TS_SETMAX (-1)

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
// some general flags for the Read(Recv)/Write(Send) TCP functions
#define TCPF_NONE             (0)
#define TCPF_FLUSH            (1<<0)
#define TCPF_FLUSHONLY        (1<<1)
#define TCPF_FREEBUFFER       (1<<2)
#define hasTCP_FLUSH(v)       (isFlagSet((v), TCPF_FLUSH))
#define hasTCP_ONLYFLUSH(v)   (isFlagSet((v), TCPF_FLUSHONLY))
#define hasTCP_FREEBUFFER(v)  (isFlagSet((v), TCPF_FREEBUFFER))

/**************************************************************************/
// mail transfer structure
struct MailTransferNode
{
  struct MinNode node;      // required for placing it into "struct TR_ClassData"
  struct Mail   *mail;      // pointer to the corresponding mail
  char          *UIDL;      // an unique identifier (UIDL) in case AvoidDuplicates is used
  unsigned char tflags;     // transfer flags
  int           position;   // current position of the mail in the GUI NList
  int           index;      // the index value of the mail as told by a POP3 server
  long          importAddr; // the position (addr) within an export file to find the mail
};

// flags for the Transfer preselection stuff
#define TRF_NONE              (0)
#define TRF_LOAD              (1<<0)
#define TRF_DELETE            (1<<1)
#define hasTR_LOAD(v)         (isFlagSet((v)->tflags, TRF_LOAD))
#define hasTR_DELETE(v)       (isFlagSet((v)->tflags, TRF_DELETE))

/**************************************************************************/
// general connection/transfer error enumation values
enum ConnectError
{
  CONNECTERR_SUCCESS       = 0,
  CONNECTERR_NO_ERROR      = -1,
  CONNECTERR_UNKNOWN_ERROR = -2,
  CONNECTERR_SOCKET_IN_USE = -3,
  CONNECTERR_UNKNOWN_HOST  = -4,
  CONNECTERR_NO_SOCKET     = -5,
  CONNECTERR_NO_NONBLOCKIO = -6,
  CONNECTERR_TIMEDOUT      = -7,
  CONNECTERR_ABORTED       = -8,
  CONNECTERR_SSLFAILED     = -9,
  CONNECTERR_INVALID8BIT   = -10
};

/**************************************************************************/
// static function prototypes
static void TR_NewMailAlert(void);
static void TR_CompleteMsgList(void);
static int TR_Send(const char *ptr, int len, int flags);
static char *TR_SendSMTPCmd(const enum SMTPCommand command, const char *parmtext, const char *errorMsg);
static int  TR_ReadLine(LONG socket, char *vptr, int maxlen);
static int  TR_ReadBuffered(LONG socket, char *ptr, int maxlen, int flags);
static int  TR_WriteBuffered(LONG socket, const char *ptr, int maxlen, int flags);
static void TR_TransStat_Update(struct TransStat *ts, int size_incr);

#define TR_WriteLine(buf)       (TR_Send((buf), strlen(buf), TCPF_FLUSH))
#define TR_WriteFlush()         (TR_Send(NULL,  0, TCPF_FLUSHONLY))
#define TR_FreeTransBuffers()   { TR_ReadBuffered(0, NULL, 0, TCPF_FREEBUFFER);  \
                                  TR_WriteBuffered(0, NULL, 0, TCPF_FREEBUFFER); }

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
// TLS/SSL related variables

static SSL_CTX *ctx = NULL;
static SSL *ssl = NULL;

/**************************************************************************/
// local macros & defines
#define GetLong(p,o)  ((((unsigned char*)(p))[o]) | (((unsigned char*)(p))[o+1]<<8) | (((unsigned char*)(p))[o+2]<<16) | (((unsigned char*)(p))[o+3]<<24))

#ifndef SHUT_RDWR
#define SHUT_RDWR 2
#endif

/***************************************************************************
 Module: Transfer
***************************************************************************/

/*** TLS/SSL routines ***/
/// TR_InitTLS()
// Initialize the SSL/TLS session accordingly
static BOOL TR_InitTLS(VOID)
{
  BOOL result = FALSE;
  char tmp[24+1];
  SSL_METHOD *method = NULL;

  ENTER();

  // lets initialize the library first and load the error strings
  // these function don't return any serious error
  SSL_library_init();
  SSL_load_error_strings();

  // We have to feed the random number generator first
  D(DBF_NET, "Seeding random number generator...");
  snprintf(tmp, sizeof(tmp), "%08lx%08lx%08lx", (unsigned long)time((time_t *)0), (unsigned long)FindTask(NULL), (unsigned long)rand());
  RAND_seed(tmp, strlen(tmp));

  // prepare the SSL client methods here
  if((method = SSLv23_client_method()))
  {
    // get the SSL context
    if((ctx = SSL_CTX_new(method)))
    {
      #if 0
      char *CAfile = NULL;
      char *CApath = NULL;

      // In future we can give the user the ability to specify his own CA locations
      // in the application instead of using the default ones.
      if(CAfile || CApath)
      {
        D(DBF_NET, "CAfile = %s, CApath = %s", CAfile ? CAfile : "none", CApath ? CApath : "none");
        if((!SSL_CTX_load_verify_locations(ctx, CAfile, CApath)))
        {
          E(DBF_NET, "Error setting default verify locations !");
          return FALSE;
        }
      }
      #endif

      // set the default SSL context verify pathes
      if(SSL_CTX_set_default_verify_paths(ctx))
      {
        // set the supported cipher list
        if((SSL_CTX_set_cipher_list(ctx, "DEFAULT")))
        {
          // for now we set the SSL certificate to 'NONE' as
          // YAM can currently not manage SSL server certificates itself
          // and therefore must be set to not verify them... unsafe?
          // well, I would call it "convienent" for the moment :)
          SSL_CTX_set_verify(ctx, SSL_VERIFY_NONE, 0);

          result = TRUE;
        }
        else
          E(DBF_NET, "SSL_CTX_set_cipher_list() error !");
      }
      else
        E(DBF_NET, "Error setting default verify locations !");
    }
    else
      E(DBF_NET, "Can't create SSL_CTX object !");
  }
  else
    E(DBF_NET, "SSLv23_client_method() error !");

  // if we weren't ale to
  // init the TLS/SSL stuff we have to clear it
  // before leaving
  if(result == FALSE)
  {
    if(ctx)
    {
      SSL_CTX_free(ctx);
      ctx = NULL;
    }

    ssl = NULL;
  }

  RETURN(result);
  return result;
}

///
/// TR_EndTLS()
// function that stops all TLS context
static VOID TR_EndTLS(VOID)
{
  ENTER();

  if(ssl)
  {
    SSL_shutdown(ssl);
    SSL_free(ssl);
    ssl = NULL;
  }

  if(ctx)
  {
    SSL_CTX_free(ctx);
    ctx = NULL;
  }

  LEAVE();
}

///
/// TR_StartTLS()
// function that starts & initializes the TLS/SSL session
static BOOL TR_StartTLS(VOID)
{
  BOOL result = FALSE;

  ENTER();

  D(DBF_NET, "Initializing TLS/SSL session...");

  // check if we are ready for creating the ssl connection
  if(ctx != NULL && (ssl = SSL_new(ctx)))
  {
    // set the socket descriptor to the ssl context
    if(G->TR_Socket != TCP_NO_SOCKET &&
       SSL_set_fd(ssl, (int)G->TR_Socket))
    {
      BOOL errorState = FALSE;
      int res;

      // establish the ssl connection and take care of non-blocking IO
      while(errorState == FALSE &&
            (res = SSL_connect(ssl)) <= 0)
      {
        int err = SSL_get_error(ssl, res);

        switch(err)
        {
          case SSL_ERROR_WANT_READ:
          case SSL_ERROR_WANT_WRITE:
          {
            // we are using non-blocking socket IO so an SSL_ERROR_WANT_READ
            // signals that the SSL socket wants us to wait until data
            // is available and reissue the SSL_connect() command.
            LONG retVal = -1;
            fd_set fdset;
            struct timeval timeout;
            int timeoutSum = 0;

            // now we iterate in a do/while loop and call WaitSelect()
            // with a static timeout of 1s. We then continue todo so
            // until we reach the predefined socket timeout
            do
            {
              // check if we are here because of a second iteration
              if(retVal == 0)
              {
                W(DBF_NET, "TR_StartTLS: recoverable WaitSelect() timeout: %ld", timeoutSum);

                // give our GUI the chance to update
                DoMethod(G->App, MUIM_Application_InputBuffered);

                // if retVal is zero, then we are here in a
                // consequtive iteration, so WaitSelect()
                // seem to have timed out and we have to check
                // whether we timed out too much already
                timeoutSum += 1; // +1s

                if(timeoutSum >= C->SocketTimeout ||
                   (G->TR != NULL && G->TR->Abort == TRUE))
                {
                  break;
                }
              }

              // we do a static timeout of 1s
              timeout.tv_sec = 1;
              timeout.tv_usec = 0;

              // now we put our socket handle into a descriptor set
              // we can pass on to WaitSelect()
              FD_ZERO(&fdset);
              FD_SET(G->TR_Socket, &fdset);

              // depending on the SSL error (WANT_READ/WANT_WRITE)
              // we either do a WaitSelect() on the read or write mode
              // as with SSL both things can happen
              // see http://www.openssl.org/docs/ssl/SSL_connect.html
              if(err == SSL_ERROR_WANT_READ)
                retVal = WaitSelect(G->TR_Socket+1, &fdset, NULL, NULL, (APTR)&timeout, NULL);
              else
                retVal = WaitSelect(G->TR_Socket+1, NULL, &fdset, NULL, (APTR)&timeout, NULL);
            }
            while(retVal == 0);

            // if WaitSelect() returns 1 we successfully waited for
            // being able to write to the socket. So we go and do another
            // iteration in the while() loop as the next connect() call should
            // return EISCONN if the connection really succeeded.
            if(retVal >= 1 && FD_ISSET(G->TR_Socket, &fdset))
            {
              // everything fine
              continue;
            }
            else if(retVal == 0)
            {
              W(DBF_NET, "WaitSelect() socket timeout reached");
              errorState = TRUE;
            }
            else
            {
              // the rest should signal an error
              E(DBF_NET, "WaitSelect() returned an error: %ld", err);
              errorState = TRUE;
            }
          }
          break;

          default:
          {
            E(DBF_NET, "SSL_connect() returned an error %ld", err);
            errorState = TRUE;
          }
          break;
        }
      }

      if(errorState == FALSE)
      {
        // everything was successfully so lets set the result
        // value of that function to true
        result = TRUE;

        // Certificate info
        // only for debug reasons
        #ifdef DEBUG
        {
          char *x509buf;
          SSL_CIPHER *cipher;
          X509 *server_cert;
          cipher = SSL_get_current_cipher(ssl);

          if(cipher)
            D(DBF_NET, "%s connection using %s", SSL_CIPHER_get_version(cipher), SSL_get_cipher(ssl));

          if(!(server_cert = SSL_get_peer_certificate(ssl)))
            E(DBF_NET, "SSL_get_peer_certificate() error !");

          D(DBF_NET, "Server public key is %ld bits", EVP_PKEY_bits(X509_get_pubkey(server_cert)));

          #define X509BUFSIZE 4096

          x509buf = (char *)malloc(X509BUFSIZE);
          memset(x509buf, 0, X509BUFSIZE);

          D(DBF_NET, "Server certificate:");

          if(!(X509_NAME_oneline(X509_get_subject_name(server_cert), x509buf, X509BUFSIZE)))
            E(DBF_NET, "X509_NAME_oneline...[subject] error !");

          D(DBF_NET, "subject: %s", x509buf);

          if(!(X509_NAME_oneline(X509_get_issuer_name(server_cert), x509buf, X509BUFSIZE)))
            E(DBF_NET, "X509_NAME_oneline...[issuer] error !");

          D(DBF_NET, "issuer:  %s", x509buf);

          if(x509buf)     free(x509buf);
          if(server_cert) X509_free(server_cert);
        }
        #endif
      }
    }
    else
      E(DBF_NET, "SSL_set_fd() error. TR_Socket: %ld", G->TR_Socket);
  }
  else
    E(DBF_NET, "Can't create a new SSL structure for a connection.");

  // if we didn't succeed with our SSL
  // startup we have to cleanup everything
  if(result == FALSE)
    TR_EndTLS();

  RETURN(result);
  return result;
}

///
/// TR_InitSTARTTLS()
// function to initiate a TLS connection to the ESMTP server via STARTTLS
static BOOL TR_InitSTARTTLS(void)
{
  BOOL result = FALSE;

  ENTER();

  // If this server doesn`t support TLS at all we return with an error
  if(hasSTARTTLS(G->TR_SMTPflags))
  {
    // If we end up here the server supports STARTTLS and we can start
    // initializing the connection
    set(G->TR->GUI.TX_STATUS, MUIA_Text_Contents, tr(MSG_TR_INITTLS));

    // Now we initiate the STARTTLS command (RFC 2487)
    if(TR_SendSMTPCmd(ESMTP_STARTTLS, NULL, tr(MSG_ER_BADRESPONSE)))
    {
      // setup the TLS/SSL session
      if(TR_InitTLS() && TR_StartTLS())
      {
        G->TR_UseTLS = TRUE;
        result = TRUE;
      }
      else
        ER_NewError(tr(MSG_ER_INITTLS), C->SMTP_Server);
    }
  }
  else
    ER_NewError(tr(MSG_ER_NOSTARTTLS));

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
  enum SMTPAuthMethod selectedMethod = SMTPAUTH_AUTO;

  ENTER();

  set(G->TR->GUI.TX_STATUS, MUIA_Text_Contents, tr(MSG_TR_SENDAUTH));

  // first we check if the user has supplied the User&Password
  // and if not we return with an error
  if(!C->SMTP_AUTH_User[0] || !C->SMTP_AUTH_Pass[0])
  {
    ER_NewError(tr(MSG_ER_NOAUTHUSERPASS));

    RETURN(FALSE);
    return FALSE;
  }

  // now we find out which of the SMTP-AUTH methods we process and which to skip
  // the user explicitly set an auth method. However, we have to
  // check wheter the SMTP server told us that it really
  // supports that method or not
  switch(C->SMTP_AUTH_Method)
  {
    case SMTPAUTH_AUTO:
    {
      // we select the most secure one the server supports
      if(hasDIGEST_MD5_Auth(G->TR_SMTPflags))
        selectedMethod = SMTPAUTH_DIGEST;
      else if(hasCRAM_MD5_Auth(G->TR_SMTPflags))
        selectedMethod = SMTPAUTH_CRAM;
      else if(hasLOGIN_Auth(G->TR_SMTPflags))
        selectedMethod = SMTPAUTH_LOGIN;
      else if(hasPLAIN_Auth(G->TR_SMTPflags))
        selectedMethod = SMTPAUTH_PLAIN;
    }
    break;

    case SMTPAUTH_DIGEST:
    {
      if(hasDIGEST_MD5_Auth(G->TR_SMTPflags))
        selectedMethod = SMTPAUTH_DIGEST;
      else
        W(DBF_NET, "User selected SMTP-Auth 'DIGEST-MD5' but server doesn't support it!");
    }
    break;

    case SMTPAUTH_CRAM:
    {
      if(hasCRAM_MD5_Auth(G->TR_SMTPflags))
        selectedMethod = SMTPAUTH_CRAM;
      else
        W(DBF_NET, "User selected SMTP-Auth 'CRAM-MD5' but server doesn't support it!");
    }
    break;

    case SMTPAUTH_LOGIN:
    {
      if(hasLOGIN_Auth(G->TR_SMTPflags))
        selectedMethod = SMTPAUTH_LOGIN;
      else
        W(DBF_NET, "User selected SMTP-Auth 'LOGIN' but server doesn't support it!");
    }
    break;

    case SMTPAUTH_PLAIN:
    {
      if(hasCRAM_MD5_Auth(G->TR_SMTPflags))
        selectedMethod = SMTPAUTH_CRAM;
      else
        W(DBF_NET, "User selected SMTP-Auth 'DIGEST-MD5' but server doesn't support it!");
    }
    break;
  }

  // now we process the SMTP Authentication by choosing the method the user
  // or the automatic did specify
  switch(selectedMethod)
  {
    // SMTP AUTH DIGEST-MD5 (RFC 2831)
    case SMTPAUTH_DIGEST:
    {
      D(DBF_NET, "processing AUTH DIGEST-MD5:");

      // send the AUTH command and get the response back
      if((resp = TR_SendSMTPCmd(ESMTP_AUTH_DIGEST_MD5, NULL, tr(MSG_ER_BADRESPONSE))))
      {
        char *realm = NULL;
        char *nonce = NULL;
        char cnonce[16+1];
        char response[32+1];
        char *chalRet;

        // get the challenge code from the response line of the
        // AUTH command.
        strlcpy(challenge, &resp[4], sizeof(challenge));

        // now that we have the challange phrase we need to base64decode
        // it, but have to take care to remove the ending "\r\n" cookie.
        chalRet = strpbrk(challenge, "\r\n"); // find the first CR or LF
        if(chalRet)
          *chalRet = '\0'; // strip it

        D(DBF_NET, "received DIGEST-MD5 challenge: `%s`", challenge);

        // lets base64 decode it
        if(base64decode(challenge, (unsigned char *)challenge, strlen(challenge)) <= 0)
        {
          RETURN(FALSE);
          return FALSE;
        }

        D(DBF_NET, "decoded  DIGEST-MD5 challenge: `%s`", challenge);

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
            W(DBF_NET, "'realm' not found in challange. using '%s' instead", C->SMTP_Domain);

            // if the challenge doesn`t have a "realm" we assume our
            // choosen SMTP domain to be the realm
            realm = strdup(C->SMTP_Domain);
          }

          D(DBF_NET, "realm: `%s`", realm);

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
            E(DBF_NET, "no `nonce=` token found!");

            free(realm);

            RETURN(FALSE);
            return FALSE;
          }

          D(DBF_NET, "nonce: `%s`", nonce);

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
              E(DBF_NET, "no `auth` in `qop` token found!");

              free(realm);
              free(nonce);

              RETURN(FALSE);
              return FALSE;
            }
          }
        }

        // if we passed here, the server seems to at least support all
        // mechanisms we need for a proper DIGEST-MD5 authentication.
        // so it`s time for STEP TWO

        // let us now generate a more or less random and unique cnonce
        // identifier which we can supply to our SMTP server.
        snprintf(cnonce, sizeof(cnonce), "%08lx%08lx", (ULONG)rand(), (ULONG)rand());

        // the we generate the response according to RFC 2831 with A1
        // and A2 as MD5 encoded strings
        {
          unsigned char digest[16]; // 16 octets
          ULONG digest_hex[4];      // 16 octets
          struct MD5Context context;
          char buf[SIZE_LARGE];
          char A1[32+1];
          int  A1_len = 16;         // 16 octects minimum
          char A2[32+1];

          // lets first generate the A1 string
          // A1 = { H( { username-value, ":", realm-value, ":", passwd } ),
          //      ":", nonce-value, ":", cnonce-value }
          snprintf(buf, sizeof(buf), "%s:%s:%s", C->SMTP_AUTH_User, realm, C->SMTP_AUTH_Pass);
          MD5Init(&context);
          MD5Update(&context, (unsigned char *)buf, strlen(buf));
          MD5Final(digest, &context);
          memcpy(buf, digest, 16);
          A1_len += snprintf(&buf[16], sizeof(buf)-16, ":%s:%s", nonce, cnonce);
          D(DBF_NET, "unencoded A1: `%s` (%ld)", buf, A1_len);

          // then we directly build the hexadecimal representation
          // HEX(H(A1))
          MD5Init(&context);
          MD5Update(&context, (unsigned char *)buf, A1_len);
          MD5Final((UBYTE *)digest_hex, &context);
          snprintf(A1, sizeof(A1), "%08lx%08lx%08lx%08lx", digest_hex[0], digest_hex[1],
                                                           digest_hex[2], digest_hex[3]);
          D(DBF_NET, "encoded   A1: `%s`", A1);


          // then we generate the A2 string accordingly
          // A2 = { "AUTHENTICATE:", digest-uri-value }
          snprintf(buf, sizeof(buf), "AUTHENTICATE:smtp/%s", realm);
          D(DBF_NET, "unencoded A2: `%s`", buf);

          // and also directly build the hexadecimal representation
          // HEX(H(A2))
          MD5Init(&context);
          MD5Update(&context, (unsigned char *)buf, strlen(buf));
          MD5Final((UBYTE *)digest_hex, &context);
          snprintf(A2, sizeof(A2), "%08lx%08lx%08lx%08lx", digest_hex[0], digest_hex[1],
                                                           digest_hex[2], digest_hex[3]);
          D(DBF_NET, "encoded   A2: `%s`", A2);

          // now we build the string from which we also build the MD5
          // HEX(H(A1)), ":",
          // nonce-value, ":", nc-value, ":",
          // cnonce-value, ":", qop-value, ":", HEX(H(A2))
          snprintf(buf, sizeof(buf), "%s:%s:00000001:%s:auth:%s", A1, nonce, cnonce, A2);
          D(DBF_NET, "unencoded resp: `%s`", buf);

          // and finally build the respone-value =
          // HEX( KD( HEX(H(A1)), ":",
          //          nonce-value, ":", nc-value, ":",
          //          cnonce-value, ":", qop-value, ":", HEX(H(A2)) }))
          MD5Init(&context);
          MD5Update(&context, (unsigned char *)buf, strlen(buf));
          MD5Final((UBYTE *)digest_hex, &context);
          snprintf(response, sizeof(response), "%08lx%08lx%08lx%08lx", digest_hex[0], digest_hex[1],
                                                                       digest_hex[2], digest_hex[3]);
          D(DBF_NET, "encoded   resp: `%s`", response);
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
                 C->SMTP_AUTH_User,
                 realm,
                 nonce,
                 cnonce,
                 realm,
                 response);

        D(DBF_NET, "prepared challenge answer....: `%s`", challenge);
        base64encode(buffer, (unsigned char *)challenge, strlen(challenge));
        D(DBF_NET, "encoded  challenge answer....: `%s`", buffer);
        strlcat(buffer, "\r\n", sizeof(buffer));

        // now we send the SMTP AUTH response
        if(TR_WriteLine(buffer) > 0)
        {
          // get the server response and see if it was valid
          if(TR_ReadLine(G->TR_Socket, buffer, SIZE_LINE) <= 0 || (rc = getResponseCode(buffer)) != 334)
            ER_NewError(tr(MSG_ER_BADRESPONSE), C->SMTP_Server, (char *)SMTPcmd[ESMTP_AUTH_DIGEST_MD5], buffer);
          else
          {
            // now that we have received the 334 code we just send a plain line
            // to signal that we don`t need any option
            if(TR_WriteLine("\r\n") > 0)
            {
              if(TR_ReadLine(G->TR_Socket, buffer, SIZE_LINE) <= 0 || (rc = getResponseCode(buffer)) != 235)
                ER_NewError(tr(MSG_ER_BADRESPONSE), C->SMTP_Server, (char *)SMTPcmd[ESMTP_AUTH_DIGEST_MD5], buffer);
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
    case SMTPAUTH_CRAM:
    {
      D(DBF_NET, "processing AUTH CRAM-MD5:");

      // send the AUTH command and get the response back
      if((resp = TR_SendSMTPCmd(ESMTP_AUTH_CRAM_MD5, NULL, tr(MSG_ER_BADRESPONSE))))
      {
        ULONG digest[4]; // 16 chars
        char buf[512];
        char *login = C->SMTP_AUTH_User;
        char *password = C->SMTP_AUTH_Pass;
        char *chalRet;

        // get the challenge code from the response line of the
        // AUTH command.
        strlcpy(challenge, &resp[4], sizeof(challenge));

        // now that we have the challange phrase we need to base64decode
        // it, but have to take care to remove the ending "\r\n" cookie.
        chalRet = strpbrk(challenge, "\r\n"); // find the first CR or LF
        if(chalRet)
          *chalRet = '\0'; // strip it

        D(DBF_NET, "received CRAM-MD5 challenge: `%s`", challenge);

        // lets base64 decode it
        if(base64decode(challenge, (unsigned char *)challenge, strlen(challenge)) <= 0)
        {
          RETURN(FALSE);
          return FALSE;
        }

        D(DBF_NET, "decoded  CRAM-MD5 challenge: `%s`", challenge);

        // compose the md5 challenge
        hmac_md5((unsigned char *)challenge, strlen(challenge), (unsigned char *)password, strlen(password), (unsigned char *)digest);
        snprintf(buf, sizeof(buf), "%s %08lx%08lx%08lx%08lx", login, digest[0], digest[1], digest[2], digest[3]);

        D(DBF_NET, "prepared CRAM-MD5 reponse..: `%s`", buf);
        // lets base64 encode the md5 challenge for the answer
        base64encode(buffer, (unsigned char *)buf, strlen(buf));
        D(DBF_NET, "encoded  CRAM-MD5 reponse..: `%s`", buffer);
        strlcat(buffer, "\r\n", sizeof(buffer));

        // now we send the SMTP AUTH response
        if(TR_WriteLine(buffer) > 0)
        {
          // get the server response and see if it was valid
          if(TR_ReadLine(G->TR_Socket, buffer, SIZE_LINE) <= 0 || (rc = getResponseCode(buffer)) != 235)
            ER_NewError(tr(MSG_ER_BADRESPONSE), C->SMTP_Server, (char *)SMTPcmd[ESMTP_AUTH_CRAM_MD5], buffer);
          else
            rc = SMTP_ACTION_OK;
        }
      }
    }
    break;

    // SMTP AUTH LOGIN
    case SMTPAUTH_LOGIN:
    {
      D(DBF_NET, "processing AUTH LOGIN:");

      // send the AUTH command
      if((resp = TR_SendSMTPCmd(ESMTP_AUTH_LOGIN, NULL, tr(MSG_ER_BADRESPONSE))))
      {
        // prepare the username challenge
        D(DBF_NET, "prepared AUTH LOGIN challenge: `%s`", C->SMTP_AUTH_User);
        base64encode(buffer, (unsigned char *)C->SMTP_AUTH_User, strlen(C->SMTP_AUTH_User));
        D(DBF_NET, "encoded  AUTH LOGIN challenge: `%s`", buffer);
        strlcat(buffer, "\r\n", sizeof(buffer));

        // now we send the SMTP AUTH response (UserName)
        if(TR_WriteLine(buffer) > 0)
        {
          // get the server response and see if it was valid
          if(TR_ReadLine(G->TR_Socket, buffer, SIZE_LINE) > 0
             && (rc = getResponseCode(buffer)) == 334)
          {
            // prepare the password challenge
            D(DBF_NET, "prepared AUTH LOGIN challenge: `%s`", C->SMTP_AUTH_Pass);
            base64encode(buffer, (unsigned char *)C->SMTP_AUTH_Pass, strlen(C->SMTP_AUTH_Pass));
            D(DBF_NET, "encoded  AUTH LOGIN challenge: `%s`", buffer);
            strlcat(buffer, "\r\n", sizeof(buffer));

            // now lets send the Password
            if(TR_WriteLine(buffer) > 0)
            {
              // get the server response and see if it was valid
              if(TR_ReadLine(G->TR_Socket, buffer, SIZE_LINE) > 0
                 && (rc = getResponseCode(buffer)) == 235)
              {
                rc = SMTP_ACTION_OK;
              }
            }
          }

          if(rc != SMTP_ACTION_OK)
            ER_NewError(tr(MSG_ER_BADRESPONSE), C->SMTP_Server, (char *)SMTPcmd[ESMTP_AUTH_LOGIN], buffer);
        }
      }
    }
    break;

    // SMTP AUTH PLAIN (RFC 2595)
    case SMTPAUTH_PLAIN:
    {
      int len=0;
      D(DBF_NET, "processing AUTH PLAIN:");

      // The AUTH PLAIN command string is a single command string, so we go
      // and prepare the challenge first
      // According to RFC 2595 this string consists of three parts:
      // "[authorize-id] \0 authenticate-id \0 password"
      // where we can left out the first one

      // we don`t have a "authorize-id" so we set the first char to \0
      challenge[len++] = '\0';
      len += snprintf(challenge+len, sizeof(challenge)-len, "%s", C->SMTP_AUTH_User)+1; // authenticate-id
      len += snprintf(challenge+len, sizeof(challenge)-len, "%s", C->SMTP_AUTH_Pass);   // password

      // now we base64 encode this string and send it to the server
      base64encode(buffer, (unsigned char *)challenge, len);

      // lets now form up the AUTH PLAIN command we are going to send
      // to the SMTP server for authorization purposes:
      snprintf(challenge, sizeof(challenge), "%s %s\r\n", SMTPcmd[ESMTP_AUTH_PLAIN], buffer);

      // now we send the SMTP AUTH command (UserName+Password)
      if(TR_WriteLine(challenge) > 0)
      {
        // get the server response and see if it was valid
        if(TR_ReadLine(G->TR_Socket, buffer, SIZE_LINE) <= 0 || (rc = getResponseCode(buffer)) != 235)
          ER_NewError(tr(MSG_ER_BADRESPONSE), C->SMTP_Server, (char *)SMTPcmd[ESMTP_AUTH_PLAIN], buffer);
        else
          rc = SMTP_ACTION_OK;
      }
    }
    break;

    default:
    {
      W(DBF_NET, "The SMTP server seems not to support any of the selected or automatic specified SMTP-AUTH methods");

      // if we don`t have any of the Authentication Flags turned on we have to
      // exit with an error
      ER_NewError(tr(MSG_CO_ER_SMTPAUTH), C->SMTP_Server);
    }
    break;
  }

  D(DBF_NET, "Server responded with %ld", rc);

  RETURN((BOOL)(rc == SMTP_ACTION_OK));
  return (BOOL)(rc == SMTP_ACTION_OK);
}
///

/*** General connecting/disconnecting & transfer ***/
/// TR_IsOnline
//  Checks if there's an online connection
BOOL TR_IsOnline(void)
{
  BOOL isonline = FALSE;
  #if defined(__amigaos4__)
  BOOL closeSocket = FALSE;
  #endif

  ENTER();

  // on AmigaOS4 we always do an online check via the v4 version
  // of bsdsocket.library (RoadShow) as it should always be present
  #if defined(__amigaos4__)
  if(SocketBase == NULL)
    closeSocket = TRUE;

  // check if we have to open the bsdsocket.library or not
  if(SocketBase != NULL || (SocketBase = OpenLibrary("bsdsocket.library", 4L)))
  {
    // if we find a bsdsocket.library < v4 on OS4
    // we always assume it to be online
    if(SocketBase->lib_Version < 4)
      isonline = TRUE;
    else
    {
      // check if we have to get the interface or not.
      if(ISocket != NULL || GETINTERFACE("main", ISocket, SocketBase))
      {
        BOOL hasInterfaceAPI = FALSE;

        D(DBF_NET, "identified bsdsocket v4 TCP/IP stack (RoadShow)");

        // in case the user hasn't specified a specific
        // interface or set that the online check for a specific
        // interface should be disabled we just do a general query
        if(C->IsOnlineCheck == FALSE || C->IOCInterface[0] == '\0')
        {
          ULONG status = 0;

          if(SocketBaseTags(SBTM_GETREF(SBTC_SYSTEM_STATUS), &status, TAG_END) == 0)
          {
            if(hasFlag(status, SBSYSSTAT_Interfaces))
              isonline = TRUE;
          }
          else
            E(DBF_NET, "couldn't query TCP/IP stack for its system status.");
        }
        else if(SocketBaseTags(SBTM_GETREF(SBTC_HAVE_INTERFACE_API), &hasInterfaceAPI, TAG_END) == 0)
        {
          LONG onlineState = 0;

          // now that we know that we have an interface API, we can
          // go and query the interface if it is up&running correctly.
          if(QueryInterfaceTags(C->IOCInterface, IFQ_State, &onlineState, TAG_END) == 0)
          {
            if(onlineState == SM_Up)
            {
              D(DBF_NET, "found interface '%s' to be UP", C->IOCInterface);
              isonline = TRUE;
            }
            else
              W(DBF_NET, "found interface '%s' to be DOWN", C->IOCInterface);
          }
          else
            E(DBF_NET, "couldn't query interface status. Unknown interface.");
        }
        else
          E(DBF_NET, "couldn't query TCP/IP stack's interface API.");

        // drop the interface if required
        if(closeSocket)
          DROPINTERFACE(ISocket);
      }
    }

    // check if we have to close the socket or not.
    if(closeSocket)
    {
      CloseLibrary(SocketBase);
      SocketBase = NULL;
    }
  }
  #else
  if(C->IsOnlineCheck)
  {
    struct Library *MiamiBase;
    struct Library *GenesisBase;

    if((MiamiBase = OpenLibrary("miami.library", 10L)))
    {
      D(DBF_NET, "identified Miami TCP/IP stack");

      isonline = MiamiIsOnline(C->IOCInterface[0] != '\0' ? C->IOCInterface : NULL);

      CloseLibrary(MiamiBase);
      MiamiBase = NULL;
    }
    else if((GenesisBase = OpenLibrary("genesis.library", 1L)))
    {
      D(DBF_NET, "identified Genesis TCP/IP stack");

      isonline = IsOnline(C->IOCInterface[0] != '\0' ? (long)C->IOCInterface : 0);

      CloseLibrary(GenesisBase);
      GenesisBase = NULL;
    }
    else if(SocketBase == NULL)
    {
      if((SocketBase = OpenLibrary("bsdsocket.library", 2L)))
      {
        CloseLibrary(SocketBase);
        SocketBase = NULL;
        isonline = TRUE;
      }
    }
    else if(SocketBase->lib_Version >= 2)
      isonline = TRUE;
  }
  else if(SocketBase == NULL)
  {
    // if no online check was selected, we just do a simple library exists
    // check and see if we are able to open a bsdsocket.library with a
    // minimum version of 2 or not.
    if((SocketBase = OpenLibrary("bsdsocket.library", 2L)))
    {
      CloseLibrary(SocketBase);
      SocketBase = NULL;
      isonline = TRUE;
    }
  }
  else if(SocketBase->lib_Version >= 2)
    isonline = TRUE;
  #endif

  D(DBF_NET, "Found the TCP/IP stack to be %s", isonline ? "ONLINE" : "OFFLINE");

  RETURN(isonline);
  return isonline;
}

///
/// TR_CloseTCPIP
//  Closes bsdsocket library
void TR_CloseTCPIP(void)
{
  ENTER();

  if(AmiSSLBase)
    CleanupAmiSSLA(NULL);

  #if defined(__amigaos4__)
  if(ISocket)
    DROPINTERFACE(ISocket);
  #endif

  if(SocketBase)
  {
    CloseLibrary(SocketBase);
    SocketBase = NULL;
  }

  LEAVE();
}

///
/// TR_OpenTCPIP
//  Opens bsdsocket.library
BOOL TR_OpenTCPIP(void)
{
  BOOL result = FALSE;

  ENTER();

  // check if the socket was already opened
  // or not
  if(SocketBase == NULL)
  {
    if((SocketBase = OpenLibrary("bsdsocket.library", 2L)))
    {
      if(GETINTERFACE("main", ISocket, SocketBase))
        result = TRUE;
      else
      {
        CloseLibrary(SocketBase);
        SocketBase = NULL;
      }
    }
  }
  else
    result = TRUE;

  // if the SocketBase is available we do
  // an online check as well
  if(result == TRUE)
  {
    if((result = TR_IsOnline()) == FALSE)
      TR_CloseTCPIP();
  }
  else
    E(DBF_NET, "couldn't open bsdsocket.library v2+");

  // Now we have to check for TLS/SSL support
  if(result == TRUE && G->TR_UseableTLS == TRUE &&
     AmiSSLBase != NULL && SocketBase != NULL)
  {
    #if defined(__amigaos4__)
    if(InitAmiSSL(AmiSSL_ISocket, ISocket,
                  TAG_DONE) != 0)
    #else
    if(InitAmiSSL(AmiSSL_SocketBase, SocketBase,
                  TAG_DONE) != 0)
    #endif
    {
      ER_NewError(tr(MSG_ER_INITAMISSL));

      G->TR_UseableTLS = FALSE;
      G->TR_UseTLS = FALSE;
      result = FALSE;
    }
  }
  else
    G->TR_UseTLS = FALSE;

  RETURN(result);
  return result;
}

///
/// TR_Disconnect
//  Terminates a connection
static void TR_Disconnect(void)
{
  ENTER();

  D(DBF_NET, "disconnecting TCP/IP session...");

  if(G->TR_Socket != TCP_NO_SOCKET)
  {
    if(G->TR_UseTLS)
    {
      TR_EndTLS();
      G->TR_UseTLS = FALSE;
    }

    shutdown(G->TR_Socket, SHUT_RDWR);
    CloseSocket(G->TR_Socket);
    G->TR_Socket = TCP_NO_SOCKET;

    // free the transfer buffers now
    TR_FreeTransBuffers();
  }

  LEAVE();
}
///
/// TR_SetSocketOpts
//  Sets the user specified options for the active socket
static void TR_SetSocketOpts(void)
{
  ENTER();

  // disable CTRL-C checking
  SocketBaseTags(SBTM_SETVAL(SBTC_BREAKMASK), 0, TAG_END);

  if(C->SocketOptions.KeepAlive)
  {
    int optval = C->SocketOptions.KeepAlive;

    if(setsockopt(G->TR_Socket, SOL_SOCKET, SO_KEEPALIVE, &optval, sizeof(optval)) == -1)
    {
      E(DBF_NET, "setsockopt(SO_KEEPALIVE) error");
      ER_NewError(tr(MSG_ER_SOCKETOPTION), "SO_KEEPALIVE");
    }
  }

  if(C->SocketOptions.NoDelay)
  {
    int optval = C->SocketOptions.NoDelay;

    if(setsockopt(G->TR_Socket, IPPROTO_TCP, TCP_NODELAY, &optval, sizeof(optval)) == -1)
    {
      E(DBF_NET, "setsockopt(TCP_NODELAY) error");
      ER_NewError(tr(MSG_ER_SOCKETOPTION), "TCP_NODELAY");
    }
  }

  if(C->SocketOptions.LowDelay)
  {
    int optval = IPTOS_LOWDELAY;

    if(setsockopt(G->TR_Socket, IPPROTO_IP, IP_TOS, &optval, sizeof(optval)) == -1)
    {
      E(DBF_NET, "setsockopt(IPTOS_LOWDELAY) error");
      ER_NewError(tr(MSG_ER_SOCKETOPTION), "IPTOS_LOWDELAY");
    }
  }

  if(C->SocketOptions.SendBuffer > -1)
  {
    int optval = C->SocketOptions.SendBuffer;

    if(setsockopt(G->TR_Socket, SOL_SOCKET, SO_SNDBUF, &optval, sizeof(optval)) == -1)
    {
      E(DBF_NET, "setsockopt(SO_SNDBUF) error");
      ER_NewError(tr(MSG_ER_SOCKETOPTION), "SO_SNDBUF");
    }
  }

  if(C->SocketOptions.RecvBuffer > -1)
  {
    int optval = C->SocketOptions.RecvBuffer;

    if(setsockopt(G->TR_Socket, SOL_SOCKET, SO_RCVBUF, &optval, sizeof(optval)) == -1)
    {
      E(DBF_NET, "setsockopt(SO_RCVBUF) error");
      ER_NewError(tr(MSG_ER_SOCKETOPTION), "SO_RCVBUF");
    }
  }

  if(C->SocketOptions.SendLowAt > -1)
  {
    int optval = C->SocketOptions.SendLowAt;

    if(setsockopt(G->TR_Socket, SOL_SOCKET, SO_SNDLOWAT, &optval, sizeof(optval)) == -1)
    {
      E(DBF_NET, "setsockopt(SO_SNDLOWAT) error");
      ER_NewError(tr(MSG_ER_SOCKETOPTION), "SO_SNDLOWAT");
    }
  }

  if(C->SocketOptions.RecvLowAt > -1)
  {
    int optval = C->SocketOptions.RecvLowAt;

    if(setsockopt(G->TR_Socket, SOL_SOCKET, SO_RCVLOWAT, &optval, sizeof(optval)) == -1)
    {
      E(DBF_NET, "setsockopt(SO_RCVLOWAT) error");
      ER_NewError(tr(MSG_ER_SOCKETOPTION), "SO_RCVLOWAT");
    }
  }

  if(C->SocketOptions.SendTimeOut > -1)
  {
    struct TimeVal tv;

    tv.Seconds = C->SocketOptions.SendTimeOut;
    tv.Microseconds = 0;

    if(setsockopt(G->TR_Socket, SOL_SOCKET, SO_SNDTIMEO, &tv, sizeof(struct TimeVal)) == -1)
    {
      E(DBF_NET, "setsockopt(SO_SNDTIMEO) error");
      ER_NewError(tr(MSG_ER_SOCKETOPTION), "SO_SNDTIMEO");
    }
  }

  if(C->SocketOptions.RecvTimeOut > -1)
  {
    struct TimeVal tv;

    tv.Seconds = C->SocketOptions.RecvTimeOut;
    tv.Microseconds = 0;

    if(setsockopt(G->TR_Socket, SOL_SOCKET, SO_RCVTIMEO, &tv, sizeof(struct TimeVal)) == -1)
    {
      E(DBF_NET, "setsockopt(SO_RCVTIMEO) error");
      ER_NewError(tr(MSG_ER_SOCKETOPTION), "SO_RCVTIMEO");
    }
  }

  // lets print out the current socket options
  #ifdef DEBUG
  {
    int optval;
    struct TimeVal tv;
    socklen_t optlen;
    socklen_t tvlen;

    D(DBF_NET, "Opened socket: %lx", G->TR_Socket);

    // the value of the length pointer must be updated ahead of each call, because
    // getsockopt() might have modified it.
    optlen = sizeof(optval);
    getsockopt(G->TR_Socket, SOL_SOCKET, SO_KEEPALIVE, &optval, &optlen);
    D(DBF_NET, "SO_KEEPALIVE..: %ld", optval);

    optlen = sizeof(optval);
    getsockopt(G->TR_Socket, IPPROTO_TCP, TCP_NODELAY, &optval, &optlen);
    D(DBF_NET, "TCP_NODELAY...: %ld", optval);

    optlen = sizeof(optval);
    getsockopt(G->TR_Socket, IPPROTO_IP, IP_TOS, &optval, &optlen);
    D(DBF_NET, "IPTOS_LOWDELAY: %ld", hasFlag(optval, IPTOS_LOWDELAY));

    optlen = sizeof(optval);
    getsockopt(G->TR_Socket, SOL_SOCKET, SO_SNDBUF, &optval, &optlen);
    D(DBF_NET, "SO_SNDBUF.....: %ld bytes", optval);

    optlen = sizeof(optval);
    getsockopt(G->TR_Socket, SOL_SOCKET, SO_RCVBUF, &optval, &optlen);
    D(DBF_NET, "SO_RCVBUF.....: %ld bytes", optval);

    optlen = sizeof(optval);
    getsockopt(G->TR_Socket, SOL_SOCKET, SO_SNDLOWAT, &optval, &optlen);
    D(DBF_NET, "SO_SNDLOWAT...: %ld", optval);

    optlen = sizeof(optval);
    getsockopt(G->TR_Socket, SOL_SOCKET, SO_RCVLOWAT, &optval, &optlen);
    D(DBF_NET, "SO_RCVLOWAT...: %ld", optval);

    tvlen = sizeof(tv);
    getsockopt(G->TR_Socket, SOL_SOCKET, SO_SNDTIMEO, &tv, &tvlen);
    D(DBF_NET, "SO_SNDTIMEO...: %ld", tv.Seconds);

    tvlen = sizeof(tv);
    getsockopt(G->TR_Socket, SOL_SOCKET, SO_RCVTIMEO, &tv, &tvlen);
    D(DBF_NET, "SO_RCVTIMEO...: %ld", tv.Seconds);
  }
  #endif

  LEAVE();
}
///
/// TR_Connect
//  Connects to a internet service
static enum ConnectError TR_Connect(char *host, int port)
{
  enum ConnectError result = CONNECTERR_UNKNOWN_ERROR;

  ENTER();

  if(G->TR_Socket == TCP_NO_SOCKET)
  {
    struct hostent *hostaddr;

    // get the hostent out of the supplied hostname
    if((hostaddr = gethostbyname((STRPTR)host)) != NULL)
    {
      int i;

      #ifdef DEBUG
      D(DBF_NET, "Host '%s':", host);
      D(DBF_NET, "  Officially:\t%s", hostaddr->h_name);

      for(i = 0; hostaddr->h_aliases[i]; ++i)
        D(DBF_NET, "  Alias:\t%s", hostaddr->h_aliases[i]);

      D(DBF_NET, "  Type:\t%s", hostaddr->h_addrtype == AF_INET ? "AF_INET" : "AF_INET6");
      if(hostaddr->h_addrtype == AF_INET)
      {
        for(i = 0; hostaddr->h_addr_list[i]; ++i)
          D(DBF_NET, "  Address:\t%s", Inet_NtoA(((struct in_addr *)hostaddr->h_addr_list[i])->s_addr));
      }
      #endif

      // now we try a connection for every address we have for this host
      // because a hostname can have more than one IP in h_addr_list[]
      for(i = 0; hostaddr->h_addr_list[i]; i++)
      {
        // lets create a standard AF_INET socket now
        if((G->TR_Socket = socket(AF_INET, SOCK_STREAM, 0)) != TCP_NO_SOCKET)
        {
          LONG nonBlockingIO = 1;

          // now we set the socket for non-blocking I/O
          if(IoctlSocket(G->TR_Socket, FIONBIO, &nonBlockingIO) != -1)
          {
            int connectIssued = 0;
            LONG err = CONNECTERR_NO_ERROR; // set to no error per default

            D(DBF_NET, "successfully set socket to %sblocking I/O", nonBlockingIO == 1 ? "non-" : "");

            // lets set the socket options the user has defined
            // in the configuration
            TR_SetSocketOpts();

            // copy the hostaddr data in a local copy for further reference
            memset(&G->TR_INetSocketAddr, 0, sizeof(G->TR_INetSocketAddr));
            G->TR_INetSocketAddr.sin_len    = sizeof(G->TR_INetSocketAddr);
            G->TR_INetSocketAddr.sin_family = AF_INET;
            G->TR_INetSocketAddr.sin_port   = htons(port);

            memcpy(&G->TR_INetSocketAddr.sin_addr, hostaddr->h_addr_list[i], (size_t)hostaddr->h_length);

            D(DBF_NET, "trying TCP/IP connection to '%s' on port %ld", Inet_NtoA(((struct in_addr *)hostaddr->h_addr_list[i])->s_addr), port);

            // we call connect() to establish the connection to the socket. In case of
            // a non-blocking connection this call will return immediately with -1 and
            // the errno value will be EINPROGRESS, EALREADY or EISCONN
            while(err == CONNECTERR_NO_ERROR &&
                  connect(G->TR_Socket, (struct sockaddr *)&G->TR_INetSocketAddr, sizeof(G->TR_INetSocketAddr)) == -1)
            {
              LONG connerr = -1;

              // count the number of connect() processings
              connectIssued++;

              // get the error value which should normally be set by a connect()
              SocketBaseTags(SBTM_GETREF(SBTC_ERRNO), &connerr, TAG_END);

              // check the errno variable which connect() will set
              switch(connerr)
              {
                // as we are doing non-blocking socket I/O we check the error code
                // and see if it is EINPROGRESS, the connection is currently in progress.
                // so we go and call WaitSelect() to wait a specific amount of time until
                // the connection succeeds.
                case EINPROGRESS:
                case EALREADY:
                {
                  LONG retVal = -1;
                  fd_set fdset;
                  struct timeval timeout;
                  int timeoutSum = 0;

                  // now we iterate in a do/while loop and call WaitSelect()
                  // with a static timeout of 1s. We then continue todo so
                  // until we reach the predefined socket timeout
                  do
                  {
                    // check if we are here because of a second iteration
                    if(retVal == 0)
                    {
                      W(DBF_NET, "TR_Connect: recoverable WaitSelect() timeout: %ld", timeoutSum);

                      // give our GUI the chance to update
                      DoMethod(G->App, MUIM_Application_InputBuffered);

                      // if retVal is zero, then we are here in a
                      // consequtive iteration, so WaitSelect()
                      // seem to have timed out and we have to check
                      // whether we timed out too much already
                      timeoutSum += 1; // +1s

                      if(timeoutSum >= C->SocketTimeout ||
                         (G->TR != NULL && G->TR->Abort == TRUE))
                      {
                        break;
                      }
                    }

                    // we do a static timeout of 1s
                    timeout.tv_sec = 1;
                    timeout.tv_usec = 0;

                    // now we put our socket handle into a descriptor set
                    // we can pass on to WaitSelect()
                    FD_ZERO(&fdset);
                    FD_SET(G->TR_Socket, &fdset);
                  }
                  while((retVal = WaitSelect(G->TR_Socket+1, NULL, &fdset, NULL, (APTR)&timeout, NULL)) == 0);

                  // if WaitSelect() returns 1 we successfully waited for
                  // being able to write to the socket. So we can break out of the
                  // loop immediately and continue with our stuff.
                  if(retVal >= 1 &&
                     FD_ISSET(G->TR_Socket, &fdset))
                  {
                    int errval = -1;
                    socklen_t errlen = sizeof(errval);

                    D(DBF_NET, "WaitSelect() succeeded");

                    // normally we should not set an error code here but
                    // continue cleanly so that a second connect() will
                    // return EISCONN. However, it seems there are some broken
                    // TCP/IP implementations (e.g. bsdsocket of UAE) which
                    // return an error for subsequent connect() calls instead.
                    //
                    // So what we do here to workaround the issue is, we
                    // query the socket options and see if SO_ERROR is zero
                    // (no error) which sould signal that the connection
                    // worked out fine, else we return an unknown error.
                    if(getsockopt(G->TR_Socket, SOL_SOCKET, SO_ERROR, &errval, &errlen) == 0 &&
                       errval == 0)
                    {
                      err = CONNECTERR_SUCCESS;
                    }
                    else
                    {
                      E(DBF_NET, "SO_ERROR in socket options found! error value read: %ld", errval);
                      err = CONNECTERR_UNKNOWN_ERROR;
                    }
                  }
                  else if(retVal == 0)
                  {
                    // the WaitSelect() call timed out or it received a break
                    // signal
                    W(DBF_NET, "WaitSelect() socket timeout reached");
                    err = CONNECTERR_TIMEDOUT;
                  }
                  else
                  {
                    // the rest should signal an error
                    E(DBF_NET, "WaitSelect() returned an error: %ld", connerr);
                    err = CONNECTERR_UNKNOWN_ERROR;
                  }
                }
                break;

                // check for EISCONN
                case EISCONN:
                {
                  // EISCONN is only a valid reponse in case we have already issued
                  // a connect() before
                  if(connectIssued > 1)
                  {
                    D(DBF_NET, "connect() signaled an EISCONN success");
                    err = CONNECTERR_SUCCESS;
                  }
                  else
                  {
                    E(DBF_NET, "connect() signaled an EISCONN failure");
                    err = CONNECTERR_UNKNOWN_ERROR;
                  }
                }
                break;

                // all other errors are real errors
                default:
                {
                  E(DBF_NET, "connect() returned with an unrecoverable error: %ld", connerr);
                  err = CONNECTERR_UNKNOWN_ERROR;
                }
                break;
              }
            }

            // in case the connection suceeded immediately (no entered the while)
            // we flag this connection as success
            if(err == CONNECTERR_NO_ERROR)
              result = CONNECTERR_SUCCESS;
            else
              result = err;

            // in case we didn't succeed we have to disconnect
            if(result != CONNECTERR_SUCCESS)
              TR_Disconnect();
          }
          else
          {
            E(DBF_NET, "couldn't establish non-blocking IO: %ld", Errno());
            result = CONNECTERR_NO_NONBLOCKIO;
            break;
          }
        }
        else
        {
          E(DBF_NET, "socket() returned an error: %ld", Errno());
          result = CONNECTERR_NO_SOCKET; // socket() failed
          break;
        }

        // give the application the chance to refresh
        DoMethod(G->App, MUIM_Application_InputBuffered);

        // if the user pressed the abort button in the transfer
        // window we have to exit the loop
        if(G->TR != NULL && G->TR->Abort == TRUE)
        {
          result = CONNECTERR_ABORTED;
          break;
        }
        else if(result == CONNECTERR_SUCCESS)
          break;
        else
          E(DBF_NET, "connection result %ld. Trying next IP of host...", result);
      }
    }
    else
      result = CONNECTERR_UNKNOWN_HOST; // gethostbyname() failed
  }
  else
    result = CONNECTERR_SOCKET_IN_USE; // socket is already in use

  #if defined(DEBUG)
  if(result != CONNECTERR_SUCCESS)
    E(DBF_NET, "TR_Connect() connection error: %ld", result);
  else
    D(DBF_NET, "connection to %s:%ld succedded", host, port);
  #endif

  RETURN(result);
  return result;
}
///
/// TR_Recv()
// a own wrapper function for Recv()/SSL_read() that reads buffered somehow
// it reads maxlen-1 data out of the buffer and stores it into recvdata with
// a null terminated string
static int TR_Recv(char *recvdata, int maxlen)
{
  int nread = -1;

  ENTER();

  // make sure the GUI can still update itself during this
  // operation
  DoMethod(G->App, MUIM_Application_InputBuffered);

  // make sure the socket is active.
  if(G->TR_Socket != TCP_NO_SOCKET)
  {
    // we call the ReadBuffered function so that
    // we get out the data from our own buffer.
    nread = TR_ReadBuffered(G->TR_Socket, recvdata, maxlen-1, TCPF_NONE);

    if(nread <= 0)
      recvdata[0] = '\0';
    else
      recvdata[nread] = '\0';

    if(G->TR_Debug)
      printf("SERVER[%04d]: %s", nread, recvdata);

    D(DBF_NET, "TCP: received %ld of max %ld bytes", nread, maxlen);
  }
  else
    W(DBF_NET, "socket == TCP_NO_SOCKET");

  RETURN(nread);
  return nread;
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
  if((read = count = TR_Recv(buf, sizeof(buf))) <= 0)
    G->Error = TRUE;

  while(G->Error == FALSE && G->TR->Abort == FALSE)
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
          TR_TransStat_Update(ts, l);

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
            state++; // now it`s 3 or 6
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
    if((read = TR_Recv(buf, sizeof(buf))) <= 0)
      break;

    count += read;
  }

  if(done == FALSE)
    count = 0;

  RETURN(count);
  return count;
}

///
/// TR_Send()
// a own wrapper function for Send()/SSL_write() that writes buffered somehow
// if called with flag != TCP_FLUSH - otherwise it will write and flush immediatly
static int TR_Send(const char *ptr, int len, int flags)
{
  int nwritten = -1;

  ENTER();

  // make sure the GUI can still update itself during this
  // operation
  DoMethod(G->App, MUIM_Application_InputBuffered);

  // make sure the socket is active.
  if(G->TR_Socket != TCP_NO_SOCKET)
  {
    // perform some debug output on the console if requested
    // by the user
    if(G->TR_Debug && ptr)
      printf("CLIENT[%04d]: %s", len, ptr);

    // we call the WriteBuffered() function to write this characters
    // out to the socket. the provided flag will define if it
    // really will be buffered or if we write and flush the buffer
    // immediatly
    nwritten = TR_WriteBuffered(G->TR_Socket, ptr, len, flags);

    D(DBF_NET, "TCP: sent %ld of %ld bytes (%ld)", nwritten, len, flags);
  }
  else
    W(DBF_NET, "socket == TCP_NO_SOCKET");

  RETURN(nwritten);
  return nwritten;
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
/// TR_ReadLine()
// a buffered version of readline() that reads out charwise from the
// socket via TR_ReadBuffered and returns the amount of chars copied
// into the provided character array or -1 on error.
//
// This implementation is a slightly adapted version of readline()/my_read()
// examples from (W.Richard Stevens - Unix Network Programming) - Page 80
static int TR_ReadLine(LONG socket, char *vptr, int maxlen)
{
  int result = -1;

  ENTER();

  // make sure the GUI can still update itself during this
  // operation
  DoMethod(G->App, MUIM_Application_InputBuffered);

  // make sure the socket is active.
  if(G->TR_Socket != TCP_NO_SOCKET)
  {
    int n;
    char *ptr;

    ptr = vptr;

    for(n = 1; n < maxlen; n++)
    {
      int rc;
      char c;

      // read out one buffered char only.
      rc = TR_ReadBuffered(socket, &c, 1, TCPF_NONE);
      if(rc == 1)
      {
        *ptr++ = c;
        if(c == '\n')
          break;  // newline is stored, like fgets()
      }
      else if(rc == 0)
      {
        // if n==1 then EOF, no data read
        // otherwise    EOF, some data was read
        if(n == 1)
          n = 0;

        break;
      }
      else
      {
        // error, errno set by readchar()
        n = -1;
        break;
      }
    }

    *ptr = '\0';              // null terminate like fgets()

    // perform some debug output on the console if requested
    // by the user
    if(G->TR_Debug)
      printf("SERVER[%04d]: %s", n, vptr);

    D(DBF_NET, "TCP: received %ld of max %ld bytes", n, maxlen);

    // return the number of chars we read
    result = n;
  }

  RETURN(result);
  return result;
}

///
/// TR_Read()
// an unbuffered implementation/wrapper for SSL_read/recv() where all available
// data upto maxlen will be put into the memory at ptr while taking care of
// non-blocking IO. Returns the number of bytes read or -1 on an error
static int TR_Read(LONG socket, char *ptr, int maxlen)
{
  int result;
  int nread = -1; // -1 is error
  int status = 0; // < 0 error, 0 unknown, > 0 no error

  ENTER();

  if(G->TR_UseTLS)
  {
    // use SSL methods to get/process all data
    do
    {
      // read out data and stop our loop in case there
      // isn't anything to read anymore or we have filled
      // maxlen data
      nread = SSL_read(ssl, ptr, maxlen);

      // if nread > 0 we read _some_ data and can
      // break out
      if(nread > 0)
        break;
      else // <= 0 found, check error state
      {
        int err = SSL_get_error(ssl, nread);

        switch(err)
        {
          case SSL_ERROR_WANT_READ:
          case SSL_ERROR_WANT_WRITE:
          {
            // we are using non-blocking socket IO so an SSL_ERROR_WANT_READ
            // signals that the SSL socket wants us to wait until data
            // is available and reissue the SSL_read() command.
            LONG retVal = -1;
            fd_set fdset;
            struct timeval timeout;
            int timeoutSum = 0;

            // now we iterate in a do/while loop and call WaitSelect()
            // with a static timeout of 1s. We then continue todo so
            // until we reach the predefined socket timeout
            do
            {
              // check if we are here because of a second iteration
              if(retVal == 0)
              {
                W(DBF_NET, "TR_Read: recoverable WaitSelect() timeout: %ld", timeoutSum);

                // give our GUI the chance to update
                DoMethod(G->App, MUIM_Application_InputBuffered);

                // if retVal is zero, then we are here in a
                // consequtive iteration, so WaitSelect()
                // seem to have timed out and we have to check
                // whether we timed out too much already
                timeoutSum += 1; // +1s

                if(timeoutSum >= C->SocketTimeout ||
                   (G->TR != NULL && G->TR->Abort == TRUE))
                {
                  break;
                }
              }

              // we do a static timeout of 1s
              timeout.tv_sec = 1;
              timeout.tv_usec = 0;

              // now we put our socket handle into a descriptor set
              // we can pass on to WaitSelect()
              FD_ZERO(&fdset);
              FD_SET(socket, &fdset);

              // depending on the SSL error (WANT_READ/WANT_WRITE)
              // we either do a WaitSelect() on the read or write mode
              // as with SSL both things can happen
              // see http://www.openssl.org/docs/ssl/SSL_read.html
              if(err == SSL_ERROR_WANT_READ)
                retVal = WaitSelect(socket+1, &fdset, NULL, NULL, (APTR)&timeout, NULL);
              else
                retVal = WaitSelect(socket+1, NULL, &fdset, NULL, (APTR)&timeout, NULL);
            }
            while(retVal == 0);

            // if WaitSelect() returns 1 we successfully waited for
            // being able to read from the socket. So we go and do another
            // iteration in the while() loop as the next connect() call should
            // return EISCONN if the connection really succeeded.
            if(retVal >= 1 && FD_ISSET(socket, &fdset))
            {
              // everything fine
              continue;
            }
            else if(retVal == 0)
            {
              W(DBF_NET, "WaitSelect() socket timeout reached");
              status = -1; // signal error
            }
            else
            {
              // the rest should signal an error
              E(DBF_NET, "WaitSelect() returned an error: %ld", err);
              status = -1; // signal error
            }
          }
          break;

          case SSL_ERROR_ZERO_RETURN:
          {
            // in case nread is zero the connection
            // was shut down cleanly...
            if(nread == 0)
            {
              // signal no error by +1
              status = 1;
            }
            else
            {
              E(DBF_NET, "SSL_write() returned SSL_ERROR_ZERO_RETURN");
              status = -1; // signal error
            }
          }
          break;

          default:
          {
            E(DBF_NET, "SSL_read() returned an error %ld", err);
            status = -1; // signal error
          }
          break;
        }
      }
    }
    while(status == 0);
  }
  else
  {
    // use normal socket methods to query/process data
    do
    {
      // read out data from our socket and return cleanly in
      // case zero is returned (clean shutdown), otherwise
      // read as much as possible
      nread = recv(socket, ptr, maxlen, 0);

      // if nread >= 0 we read _some_ data or reach the
      // end of the socket and can break out
      if(nread >= 0)
        break;
      else // < 0 found, check error state
      {
        LONG err = -1;

        // get the error value which should normally be set by a recv()
        SocketBaseTags(SBTM_GETREF(SBTC_ERRNO), &err, TAG_END);

        switch(err)
        {
          case EAGAIN:
          {
            // we are using non-blocking socket IO so an EAGAIN signals
            // that we should wait for more data to arrive before we
            // can issue a recv() again.
            LONG retVal = -1;
            fd_set fdset;
            struct timeval timeout;
            int timeoutSum = 0;

            // now we iterate in a do/while loop and call WaitSelect()
            // with a static timeout of 1s. We then continue todo so
            // until we reach the predefined socket timeout
            do
            {
              // check if we are here because of a second iteration
              if(retVal == 0)
              {
                W(DBF_NET, "TR_Read: recoverable WaitSelect() timeout: %ld", timeoutSum);

                // give our GUI the chance to update
                DoMethod(G->App, MUIM_Application_InputBuffered);

                // if retVal is zero, then we are here in a
                // consequtive iteration, so WaitSelect()
                // seem to have timed out and we have to check
                // whether we timed out too much already
                timeoutSum += 1; // +1s

                if(timeoutSum >= C->SocketTimeout ||
                   (G->TR != NULL && G->TR->Abort == TRUE))
                {
                  break;
                }
              }

              // we do a static timeout of 1s
              timeout.tv_sec = 1;
              timeout.tv_usec = 0;

              // now we put our socket handle into a descriptor set
              // we can pass on to WaitSelect()
              FD_ZERO(&fdset);
              FD_SET(socket, &fdset);
            }
            while((retVal = WaitSelect(socket+1, &fdset, NULL, NULL, (APTR)&timeout, NULL)) == 0);

            // if WaitSelect() returns 1 we successfully waited for
            // being able to read from the socket. So we go and do another
            // iteration in the while() loop as the next connect() call should
            // return EISCONN if the connection really succeeded.
            if(retVal >= 1 &&
               FD_ISSET(socket, &fdset))
            {
              // everything fine
              continue;
            }
            else if(retVal == 0)
            {
              W(DBF_NET, "WaitSelect() socket timeout reached");
              status = -1; // signal error
            }
            else
            {
              // the rest should signal an error
              E(DBF_NET, "WaitSelect() returned an error: %ld", err);
              status = -1; // signal error
            }
          }
          break;

          case EINTR:
          {
            // we received an interrupt signal so we issue the recv()
            // command again.
            continue;
          }
          break;

          default:
          {
            E(DBF_NET, "recv() returned an error %ld", err);
            status = -1; // signal error
          }
          break;
        }
      }
    }
    while(status == 0);
  }

  // check the status for an error
  if(status == -1)
    result = -1;
  else
    result = nread;

  RETURN(result);
  return result;
}

///
/// TR_ReadBuffered()
// a buffered implementation of read()/recv() which is somehow compatible
// to the I/O fgets() function. It will read out data from the socket in
// 4096 chunks and store them into a static buffer. This function SHOULD
// only be called by TR_ReadLine() and TR_Recv().
//
// This implementation is a slightly adapted version of readline()/my_read()
// examples from (W.Richard Stevens - Unix Network Programming) - Page 80
// However, they were adapted to non-blocking IO whereever necessary
static int TR_ReadBuffered(LONG socket, char *ptr, int maxlen, int flags)
{
  int result = -1; // -1 = error
  static int read_cnt = 0;
  static char *read_buf = NULL;
  static char *read_ptr = NULL;

  ENTER();

  // if we don`t have a buffer yet, lets allocate one
  if(read_buf != NULL || (read_buf = read_ptr = calloc(1, C->TRBufferSize*sizeof(char))) != NULL)
  {
    // if we called that function with the FREEBUFFER flag we free the buffer only
    // and return with 0
    if(hasTCP_FREEBUFFER(flags))
    {
      free(read_buf);
      read_cnt = 0;
      read_buf = NULL;
      read_ptr = NULL;

      result = 0;
    }
    else
    {
      BOOL errorState = FALSE;

      // now comes the 'real' socket read stuff where
      // we obtain as much data as possible up to the maxlen amount.

      // if the buffer is empty we fill it again from
      // data we request from the socket. Here we make sure we
      // take care of non-blocking IO
      if(read_cnt <= 0)
      {
        // read all data upto the maximum our buffer allows
        if((read_cnt = TR_Read(socket, read_buf, C->TRBufferSize)) <= 0)
          errorState = TRUE;

        // reset the read_ptr
        read_ptr = read_buf;
      }

      if(errorState == FALSE)
      {
        // we copy only the minmum of read_cnt and maxlen
        int fill_cnt = MIN(read_cnt, maxlen);

        // to speed up the ReadLine we only use memcpy()
        // for requests > 1
        if(fill_cnt > 1)
        {
          // now we copy a maximum of maxlen chars to
          // the destination
          memcpy(ptr, read_ptr, fill_cnt);
          read_cnt -= fill_cnt;
          read_ptr += fill_cnt;
        }
        else
        {
          read_cnt--;
          *ptr = *read_ptr++;
        }

        result = fill_cnt;
      }
    }
  }
  else
    E(DBF_NET, "couldn't allocate read buffer!");

  RETURN(result);
  return result;
}

///
/// TR_Write()
// an unbuffered implementation/wrapper for SSL_write/send() where the supplied
// data in ptr will be send out straight away while taking care of non-blocking IO.
// returns the number of bytes written or -1 on an error
static int TR_Write(LONG socket, const char *ptr, int len)
{
  int result;
  int towrite = len;
  int status = 0; // < 0 error, 0 unknown, > 0 no error

  ENTER();

  if(G->TR_UseTLS)
  {
    // use SSL methods to get/process all data
    do
    {
      // write data to our socket and then check how much
      // we have written and in case we weren't able to write
      // all out, we retry it with smaller chunks.
      int nwritten = SSL_write(ssl, ptr, towrite);

      // if nwritten > 0 we transfered _some_ data
      if(nwritten > 0)
      {
        towrite -= nwritten;
        ptr += nwritten;

        // check if we are finished with sending all data
        // otherwise our next iteration wiill continue
        // sending the rest.
        if(towrite <= 0)
          break;
      }
      else // if an error occurred we process it
      {
        int err = SSL_get_error(ssl, nwritten);

        switch(err)
        {
          case SSL_ERROR_WANT_READ:
          case SSL_ERROR_WANT_WRITE:
          {
            // we are using non-blocking socket IO so an SSL_ERROR_WANT_READ
            // signals that the SSL socket wants us to wait until data
            // is available and reissue the SSL_read() command.
            LONG retVal = -1;
            fd_set fdset;
            struct timeval timeout;
            int timeoutSum = 0;

            // now we iterate in a do/while loop and call WaitSelect()
            // with a static timeout of 1s. We then continue todo so
            // until we reach the predefined socket timeout
            do
            {
              // check if we are here because of a second iteration
              if(retVal == 0)
              {
                W(DBF_NET, "TR_Write: recoverable WaitSelect() timeout: %ld", timeoutSum);

                // give our GUI the chance to update
                DoMethod(G->App, MUIM_Application_InputBuffered);

                // if retVal is zero, then we are here in a
                // consequtive iteration, so WaitSelect()
                // seem to have timed out and we have to check
                // whether we timed out too much already
                timeoutSum += 1; // +1s

                if(timeoutSum >= C->SocketTimeout ||
                   (G->TR != NULL && G->TR->Abort == TRUE))
                {
                  break;
                }
              }

              // we do a static timeout of 1s
              timeout.tv_sec = 1;
              timeout.tv_usec = 0;

              // now we put our socket handle into a descriptor set
              // we can pass on to WaitSelect()
              FD_ZERO(&fdset);
              FD_SET(socket, &fdset);

              // depending on the SSL error (WANT_READ/WANT_WRITE)
              // we either do a WaitSelect() on the read or write mode
              // as with SSL both things can happen
              // see http://www.openssl.org/docs/ssl/SSL_write.html
              if(err == SSL_ERROR_WANT_READ)
                retVal = WaitSelect(socket+1, &fdset, NULL, NULL, (APTR)&timeout, NULL);
              else
                retVal = WaitSelect(socket+1, NULL, &fdset, NULL, (APTR)&timeout, NULL);
            }
            while(retVal == 0);

            // if WaitSelect() returns 1 we successfully waited for
            // being able to write to the socket. So we go and do another
            // iteration in the while() loop as the next connect() call should
            // return EISCONN if the connection really succeeded.
            if(retVal >= 1 && FD_ISSET(socket, &fdset))
            {
              // everything fine
              continue;
            }
            else if(retVal == 0)
            {
              W(DBF_NET, "WaitSelect() socket timeout reached");
              status = -1; // signal error
            }
            else
            {
              // the rest should signal an error
              E(DBF_NET, "WaitSelect() returned an error: %ld", err);
              status = -1; // signal error
            }
          }
          break;

          case SSL_ERROR_ZERO_RETURN:
          {
            // in case nwritten is zero the connection
            // was shut down cleanly...
            if(nwritten == 0)
            {
              // signal no error by +1
              status = 1;
            }
            else
            {
              E(DBF_NET, "SSL_write() returned SSL_ERROR_ZERO_RETURN");
              status = -1; // signal error
            }
          }
          break;

          default:
          {
            E(DBF_NET, "SSL_write() returned an error %ld", err);
            status = -1; // signal error
          }
          break;
        }
      }
    }
    while(status == 0);
  }
  else
  {
    // use normal socket methods to query/process data
    do
    {
      // write data to our socket and then check how much
      // we have written and in case we weren't able to write
      // all out, we retry it with smaller chunks.
      int nwritten = send(socket, (APTR)ptr, towrite, 0);

      // if nwritten > 0 we transfered _some_ data
      if(nwritten > 0)
      {
        towrite -= nwritten;
        ptr += nwritten;

        // check if we are finished with sending all data
        // otherwise our next iteration wiill continue
        // sending the rest.
        if(towrite <= 0)
          break;
      }
      else // <= 0 if an error occurred we process it
      {
        LONG err = -1;

        // get the error value which should normally be set by a send()
        SocketBaseTags(SBTM_GETREF(SBTC_ERRNO), &err, TAG_END);

        switch(err)
        {
          case EAGAIN:
          {
            // we are using non-blocking socket IO so an EAGAIN signals
            // that we should wait for more data to arrive before we
            // can issue a recv() again.
            LONG retVal = -1;
            fd_set fdset;
            struct timeval timeout;
            int timeoutSum = 0;

            // now we iterate in a do/while loop and call WaitSelect()
            // with a static timeout of 1s. We then continue todo so
            // until we reach the predefined socket timeout
            do
            {
              // check if we are here because of a second iteration
              if(retVal == 0)
              {
                W(DBF_NET, "TR_Write: recoverable WaitSelect() timeout: %ld", timeoutSum);

                // give our GUI the chance to update
                DoMethod(G->App, MUIM_Application_InputBuffered);

                // if retVal is zero, then we are here in a
                // consequtive iteration, so WaitSelect()
                // seem to have timed out and we have to check
                // whether we timed out too much already
                timeoutSum += 1; // +1s

                if(timeoutSum >= C->SocketTimeout ||
                   (G->TR != NULL && G->TR->Abort == TRUE))
                {
                  break;
                }
              }

              // we do a static timeout of 1s
              timeout.tv_sec = 1;
              timeout.tv_usec = 0;

              // now we put our socket handle into a descriptor set
              // we can pass on to WaitSelect()
              FD_ZERO(&fdset);
              FD_SET(socket, &fdset);
            }
            while((retVal = WaitSelect(socket+1, NULL, &fdset, NULL, (APTR)&timeout, NULL)) == 0);

            // if WaitSelect() returns 1 we successfully waited for
            // being able to write to the socket. So we go and do another
            // iteration in the while() loop as the next connect() call should
            // return EISCONN if the connection really succeeded.
            if(retVal >= 1 &&
               FD_ISSET(socket, &fdset))
            {
              // everything fine
              continue;
            }
            else if(retVal == 0)
            {
              W(DBF_NET, "WaitSelect() socket timeout reached");
              status = -1; // signal error
            }
            else
            {
              // the rest should signal an error
              E(DBF_NET, "WaitSelect() returned an error: %ld", err);
              status = -1; // signal error
            }
          }
          break;

          case EINTR:
          {
            // we received an interrupt signal so we issue the send()
            // command again.
            continue;
          }
          break;

          default:
          {
            E(DBF_NET, "send() returned an error %ld", err);
            status = -1; // signal error
          }
          break;
        }
      }
    }
    while(status == 0);
  }

  #if defined(DEBUG)
  if(towrite != 0)
    E(DBF_NET, "TR_Write() short item count: %ld of %ld transfered!", len-towrite, len);
  #endif

  // check the status
  if(status == -1)
    result = -1;
  else
    result = len-towrite;

  RETURN(result);
  return result;
}

///
/// TR_WriteBuffered()
// a buffered implementation of SSL_write()/send(). This function will buffer
// all write operations in a temporary buffer and as soon as the buffer is
// full or it will be explicitly flushed, it will write out the data to the
// socket. This function SHOULD only be called by TR_Send() somehow.
//
// This function will also optimize the write operations by always making
// sure that a full buffer will be written out to the socket. i.e. if the
// buffer is filled up so that the next call would flush it, we copy as
// many data to the buffer as possible and flush it immediatly.
static int TR_WriteBuffered(LONG socket, const char *ptr, int maxlen, int flags)
{
  int result = -1; // -1 = error
  static int write_cnt = 0;
  static char *write_buf = NULL;
  static char *write_ptr = NULL;

  ENTER();

  if(write_buf != NULL || (write_buf = write_ptr = calloc(1, C->TRBufferSize*sizeof(char))) != NULL)
  {
    // if we called that function with the FREEBUFFER flag we free the buffer only
    // and return with 0
    if(hasTCP_FREEBUFFER(flags))
    {
      free(write_buf);
      write_cnt = 0;
      write_buf = NULL;
      write_ptr = NULL;

      result = 0;
    }
    else
    {
      int fill_cnt = 0;
      BOOL abortProc = FALSE;

      // in case our write buffer is already filled up or
      // the caller wants to just flush the buffer we immediately use
      // the socket functions send/SSL_write() to transfer everything
      // and therefore clear the buffer.
      if(write_cnt >= C->TRBufferSize || hasTCP_ONLYFLUSH(flags))
      {
        // make sure we write out the data to the socket
        if(TR_Write(socket, write_buf, write_cnt) != write_cnt)
        {
          // abort and signal an error
          abortProc = TRUE;
        }
        else
        {
          // set the ptr to the start of the buffer
          write_ptr = write_buf;
          write_cnt = 0;

          // if this write operation was just because of a ONLYFLUSH
          // flag we can abort immediately, but make sure we don't
          // return an error but a count of zero
          if(hasTCP_ONLYFLUSH(flags))
          {
            abortProc = TRUE;
            result = 0;
          }
        }
      }

      // check if we should continue
      if(abortProc == FALSE)
      {
        // if the string we want to copy into the buffer
        // wouldn`t fit, we copy as much as we can, clear the buffer
        // and continue until there is enough space left
        while(write_cnt+maxlen > C->TRBufferSize)
        {
          int fillable = C->TRBufferSize-write_cnt;

          // after the copy we have to increase the pointer of the
          // array we want to copy, because in the next cycle we have to
          // copy the rest out of it.
          memcpy(write_ptr, ptr, fillable);
          ptr += fillable;
          write_cnt += fillable;
          fill_cnt += fillable;

          // decrease maxlen now by the amount of bytes we have written
          // to the buffer
          maxlen -= fillable;

          // now we write out the data
          if(TR_Write(socket, write_buf, write_cnt) != write_cnt)
          {
            // abort and signal an error
            abortProc = TRUE;
            break;
          }
          else
          {
            // set the ptr to the start of the buffer
            // as we flushed everything
            write_ptr = write_buf;
            write_cnt = 0;
          }
        }

        // check if we should abort
        if(abortProc == FALSE)
        {
          // if we end up here we have enough space for our
          // string in the buffer, so lets copy it in
          memcpy(write_ptr, ptr, maxlen);
          write_ptr += maxlen;
          write_cnt += maxlen;
          fill_cnt += maxlen;

          // if the user has supplied the FLUSH flag we have to clear/flush
          // the buffer immediately after having copied everything
          if(hasTCP_FLUSH(flags))
          {
            // write our whole buffer out to the socket
            if(TR_Write(socket, write_buf, write_cnt) != write_cnt)
            {
              // abort and signal an error
              abortProc = TRUE;
            }
            else
            {
              // set the ptr to the start of the buffer
              // as we flushed everything
              write_ptr = write_buf;
              write_cnt = 0;
            }
          }

          // if we still haven't received an abort signal
          // we can finally set the result to fill_cnt
          if(abortProc == FALSE)
            result = fill_cnt;
        }
      }
    }
  }
  else
    E(DBF_NET, "couldn't allocate write buffer!");

  RETURN(result);
  return result;
}

///

/*** HTTP routines ***/
/// TR_DownloadURL()
//  Downloads a file from the web using HTTP/1.1 (RFC 2616)
BOOL TR_DownloadURL(const char *server, const char *request, const char *filename)
{
  BOOL result = FALSE;
  BOOL noproxy = (C->ProxyServer[0] == '\0');
  int hport;
  char url[SIZE_URL];
  char host[SIZE_HOST];
  char *path;
  char *bufptr;

  ENTER();

  // make sure the error state is cleared
  G->Error = FALSE;

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
  if(TR_Connect(host, hport) == CONNECTERR_SUCCESS)
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
    snprintf(httpRequest, sizeof(httpRequest), "GET %s HTTP/1.1\r\n"
                                               "Host: %s\r\n"
                                               "User-Agent: %s\r\n"
                                               "Accept: */*\r\n"
                                               "\r\n", serverPath, serverHost, yamuseragent);

    SHOWSTRING(DBF_NET, httpRequest);

    // send out the httpRequest
    if(TR_WriteLine(httpRequest) > 0)
    {
      char *p;
      char serverResponse[SIZE_LINE];
      int len;

      // clear the serverResponse string
      serverResponse[0] = '\0';

      // now we read out the very first line to see if the
      // response code matches and is fine
      len = TR_ReadLine(G->TR_Socket, serverResponse, SIZE_LINE);

      SHOWSTRING(DBF_NET, serverResponse);

      // check the server response
      if(len > 0 && strnicmp(serverResponse, "HTTP/", 5) == 0 &&
         (p = strchr(serverResponse, ' ')) != NULL && atoi(TrimStart(p)) == 200)
      {
        LONG contentLength = -1; // -1 means no Content-Length found

        // we can request all further lines from our socket
        // until we reach the entity body
        while(G->Error == FALSE &&
              (len = TR_ReadLine(G->TR_Socket, serverResponse, SIZE_LINE)) > 0)
        {
          // RFC 2616 section 4.4 requires Content-Length:
          if(strnicmp(serverResponse, "Content-Length:", 15) == 0)
            contentLength = atoi(TrimStart(&serverResponse[15]));

          // if we are still scanning for the end of the
          // response header we go on searching for the empty '\r\n' line
          if(contentLength >= 0 && strcmp(serverResponse, "\r\n") == 0)
          {
            FILE *out;

            // prepare the output file.
            if((out = fopen(filename, "w")) != NULL)
            {
              LONG retrieved = 0;

              setvbuf(out, NULL, _IOFBF, SIZE_FILEBUF);

              // we seem to have reached the entity body, so
              // we can write the rest out immediately.
              while(G->Error == FALSE && retrieved < contentLength &&
                    (len = TR_Recv(serverResponse, SIZE_LINE)) > 0)
              {
                if(fwrite(serverResponse, len, 1, out) != 1)
                {
                  retrieved = -1; // signal an error!
                  break;
                }

                retrieved += len;
              }

              // check if we retrieved everything required
              if(retrieved >= contentLength)
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

    TR_Disconnect();
  }
  else
    ER_NewError(tr(MSG_ER_ConnectHTTP), host);

  RETURN((BOOL)(result == TRUE && G->Error == FALSE));
  return (BOOL)(result == TRUE && G->Error == FALSE);
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
  if(G->TR_Socket != TCP_NO_SOCKET)
  {
    static char buf[SIZE_LINE]; // SIZE_LINE should be enough for the command and reply

    // if we specified a parameter for the pop command lets add it now
    if(parmtext == NULL || parmtext[0] == '\0')
      snprintf(buf, sizeof(buf), "%s\r\n", POPcmd[command]);
    else
      snprintf(buf, sizeof(buf), "%s %s\r\n", POPcmd[command], parmtext);

    D(DBF_NET, "TCP: POP3 cmd '%s' with param '%s'", POPcmd[command], parmtext != NULL ? (command == POPCMD_PASS ? "XXX" : parmtext) : "<NULL>");

    // send the pop command to the server and see if it was received somehow
    // and for a connect we don`t send something or the server will get
    // confused.
    if(command == POPCMD_CONNECT || TR_WriteLine(buf) > 0)
    {
      // let us read the next line from the server and check if
      // some status message can be retrieved.
      if(TR_ReadLine(G->TR_Socket, buf, sizeof(buf)) > 0 &&
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

          ER_NewError(errorMsg, C->P3[G->TR->POP_Nr]->Server, (char *)POPcmd[command], buf);
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
   char passwd[SIZE_PASSWORD], host[SIZE_HOST], buf[SIZE_LINE], *p;
   char *welcomemsg = NULL;
   int pop = G->TR->POP_Nr, msgs;
   struct POP3 *pop3 = C->P3[pop];
   int port = pop3->Port;
   char *resp;
   enum ConnectError err;

   strlcpy(passwd, pop3->Password, sizeof(passwd));
   strlcpy(host, pop3->Server, sizeof(host));

   // now we have to check whether SSL/TLS is selected for that POP account,
   // but perhaps TLS is not working.
   if(pop3->SSLMode != P3SSL_OFF && !G->TR_UseableTLS)
   {
      ER_NewError(tr(MSG_ER_UNUSABLEAMISSL));
      return -1;
   }

   if(C->TransferWindow == TWM_SHOW || (C->TransferWindow == TWM_AUTO && (guilevel == POP_START || guilevel == POP_USER)))
   {
     // avoid MUIA_Window_Open's side effect of activating the window if it was already open
     if(!xget(G->TR->GUI.WI, MUIA_Window_Open))
       set(G->TR->GUI.WI, MUIA_Window_Open, TRUE);
   }
   set(G->TR->GUI.TX_STATUS, MUIA_Text_Contents, tr(MSG_TR_Connecting));

   // If the hostname has a explicit :xxxxx port statement at the end we
   // take this one, even if its not needed anymore.
   if((p = strchr(host, ':')))
   {
     *p = '\0';
     port = atoi(++p);
   }

   // set the busy text and window title to some
   // descriptive to the job. Here we use the "account"
   // name of the POP3 server as there might be more than
   // one configured accounts for the very same host
   // and as such the hostname might just be not enough
   if(pop3->Account[0] != '\0')
   {
     BusyText(tr(MSG_TR_MailTransferFrom), pop3->Account);
     TR_SetWinTitle(TRUE, pop3->Account);
   }
   else
   {
     // if the user hasn't specified any account name
     // we take the hostname instead
     BusyText(tr(MSG_TR_MailTransferFrom), host);
     TR_SetWinTitle(TRUE, host);
   }

   // now we start our connection to the POP3 server
   if((err = TR_Connect(host, port)) != CONNECTERR_SUCCESS)
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
           ER_NewError(tr(MSG_ER_CONNECTERR_SOCKET_IN_USE), host);
         break;

         // socket() execution failed
         case CONNECTERR_NO_SOCKET:
           ER_NewError(tr(MSG_ER_CONNECTERR_NO_SOCKET), host);
         break;

         // couldn't establish non-blocking IO
         case CONNECTERR_NO_NONBLOCKIO:
           ER_NewError(tr(MSG_ER_CONNECTERR_NO_NONBLOCKIO), host);
         break;

         // connection request timed out
         case CONNECTERR_TIMEDOUT:
           ER_NewError(tr(MSG_ER_CONNECTERR_TIMEDOUT), host);
         break;

         // unknown host - gethostbyname() failed
         case CONNECTERR_UNKNOWN_HOST:
           ER_NewError(tr(MSG_ER_UnknownPOP), host);
         break;

         // general connection error
         case CONNECTERR_UNKNOWN_ERROR:
           ER_NewError(tr(MSG_ER_CantConnect), host);
         break;

         case CONNECTERR_SSLFAILED:
         case CONNECTERR_INVALID8BIT:
           // can't occur, do nothing
         break;
       }
     }

     return -1;
   }

   // If this connection should be a STLS like connection we have to get the welcome
   // message now and then send the STLS command to start TLS negotiation
   if(pop3->SSLMode == P3SSL_TLS)
   {
      set(G->TR->GUI.TX_STATUS, MUIA_Text_Contents, tr(MSG_TR_WaitWelcome));

      // Initiate a connect and see if we succeed
      if((resp = TR_SendPOP3Cmd(POPCMD_CONNECT, NULL, tr(MSG_ER_POP3WELCOME))) == NULL)
        return -1;
      welcomemsg = StrBufCpy(NULL, resp);

      // If the user selected STLS support we have to first send the command
      // to start TLS negotiation (RFC 2595)
      if(TR_SendPOP3Cmd(POPCMD_STLS, NULL, tr(MSG_ER_BADRESPONSE)) == NULL)
        return -1;
   }

   // Here start the TLS/SSL Connection stuff
   if(pop3->SSLMode != P3SSL_OFF)
   {
     set(G->TR->GUI.TX_STATUS, MUIA_Text_Contents, tr(MSG_TR_INITTLS));

     // Now we have to Initialize and Start the TLS stuff if requested
     if(TR_InitTLS() && TR_StartTLS())
     {
        G->TR_UseTLS = TRUE;
     }
     else
     {
        ER_NewError(tr(MSG_ER_INITTLS), host);
        return -1;
     }
   }

   // If this was a connection on a stunnel on port 995 or a non-ssl connection
   // we have to get the welcome message now
   if(pop3->SSLMode != P3SSL_TLS)
   {
      // Initiate a connect and see if we succeed
      if((resp = TR_SendPOP3Cmd(POPCMD_CONNECT, NULL, tr(MSG_ER_POP3WELCOME))) == NULL)
        return -1;
      welcomemsg = StrBufCpy(NULL, resp);
   }

   if (!*passwd)
   {
      snprintf(buf, sizeof(buf), tr(MSG_TR_PopLoginReq), C->P3[pop]->User, host);
      if (!StringRequest(passwd, SIZE_PASSWORD, tr(MSG_TR_PopLogin), buf, tr(MSG_Okay), NULL, tr(MSG_Cancel), TRUE, G->TR->GUI.WI))
      {
        return -1;
      }
   }

   // if the user has selected APOP for that POP3 host
   // we have to process it now
   if (pop3->UseAPOP)
   {
      struct MD5Context context;
      UBYTE digest[16];
      int i, j;

      // Now we get the APOP Identifier out of the welcome
      // message
      if((p = strchr(welcomemsg, '<')))
      {
         strlcpy(buf, p, sizeof(buf));
         if ((p = strchr(buf, '>'))) p[1] = 0;

         // then we send the APOP command to authenticate via APOP
         strlcat(buf, passwd, sizeof(buf));
         MD5Init(&context);
         MD5Update(&context, (unsigned char *)buf, strlen(buf));
         MD5Final(digest, &context);
         snprintf(buf, sizeof(buf), "%s ", pop3->User);
         for(j=strlen(buf), i=0; i<16; j+=2, i++)
           snprintf(&buf[j], sizeof(buf)-j, "%02x", digest[i]);
         buf[j] = 0;
         set(G->TR->GUI.TX_STATUS, MUIA_Text_Contents, tr(MSG_TR_SendAPOPLogin));
         if (TR_SendPOP3Cmd(POPCMD_APOP, buf, tr(MSG_ER_BADRESPONSE)) == NULL)
           return -1;
      }
      else
      {
         ER_NewError(tr(MSG_ER_NoAPOP));
         return -1;
      }
   }
   else
   {
      set(G->TR->GUI.TX_STATUS, MUIA_Text_Contents, tr(MSG_TR_SendUserID));
      if (TR_SendPOP3Cmd(POPCMD_USER, pop3->User, tr(MSG_ER_BADRESPONSE)) == NULL)
        return -1;
      set(G->TR->GUI.TX_STATUS, MUIA_Text_Contents, tr(MSG_TR_SendPassword));
      if (TR_SendPOP3Cmd(POPCMD_PASS, passwd, tr(MSG_ER_BADRESPONSE)) == NULL)
        return -1;
   }

   FreeStrBuf(welcomemsg);

   set(G->TR->GUI.TX_STATUS, MUIA_Text_Contents, tr(MSG_TR_GetStats));
   if ((resp = TR_SendPOP3Cmd(POPCMD_STAT, NULL, tr(MSG_ER_BADRESPONSE))) == NULL)
     return -1;

   sscanf(&resp[4], "%d", &msgs);
   if(msgs != 0)
     AppendToLogfile(LF_VERBOSE, 31, tr(MSG_LOG_ConnectPOP), pop3->User, host, msgs);

   return msgs;
}
///
/// TR_DisplayMailList
//  Displays a list of messages ready for download
static void TR_DisplayMailList(BOOL largeonly)
{
  ENTER();

  if(IsListEmpty((struct List *)&G->TR->transferList) == FALSE)
  {
    // search through our transferList
    Object *lv = G->TR->GUI.LV_MAILS;
    struct MinNode *curNode;
    int pos=0;

    set(lv, MUIA_NList_Quiet, TRUE);
    for(curNode = G->TR->transferList.mlh_Head; curNode->mln_Succ; curNode = curNode->mln_Succ)
    {
      struct MailTransferNode *mtn = (struct MailTransferNode *)curNode;
      struct Mail *mail = mtn->mail;

      // add this mail to the transfer list in case we either
      // should show ALL mails or the mail size is >= the warning size
      if(largeonly == FALSE || mail->Size >= C->WarnSize*1024)
      {
        mtn->position = pos++;

        DoMethod(lv, MUIM_NList_InsertSingle, mtn, MUIV_NList_Insert_Bottom);
      }
    }

    xset(lv, MUIA_NList_Active, MUIV_NList_Active_Top,
             MUIA_NList_Quiet,  FALSE);
  }

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
  if(TR_SendPOP3Cmd(POPCMD_LIST, NULL, tr(MSG_ER_BADRESPONSE)) != NULL)
  {
    char buf[SIZE_LINE];

    success = TRUE;

    NewList((struct List *)&G->TR->transferList);

    // get the first line the pop server returns after the LINE command
    if(TR_ReadLine(G->TR_Socket, buf, sizeof(buf)) > 0)
    {
      // we get the "scan listing" as long as we haven`t received a a
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
          static const int mode2tflags[16] = { TRF_LOAD, TRF_LOAD, (TRF_LOAD|TRF_DELETE),
                                               (TRF_LOAD|TRF_DELETE), TRF_LOAD, TRF_LOAD,
                                               (TRF_LOAD|TRF_DELETE), (TRF_LOAD|TRF_DELETE),
                                               TRF_NONE, TRF_LOAD, TRF_NONE, (TRF_LOAD|TRF_DELETE),
                                               TRF_NONE, TRF_LOAD, TRF_NONE, (TRF_LOAD|TRF_DELETE)
                                             };

          newMail->Size  = size;

          mode = (C->DownloadLarge ? 1 : 0) +
                 (C->P3[G->TR->POP_Nr]->DeleteOnServer ? 2 : 0) +
                 (G->TR->GUIlevel == POP_USER ? 4 : 0) +
                 ((C->WarnSize && newMail->Size >= (C->WarnSize*1024)) ? 8 : 0);

          // allocate a new MailTransferNode and add it to our
          // new transferlist
          if((mtn = calloc(1, sizeof(struct MailTransferNode))) != NULL)
          {
            mtn->mail = newMail;
            mtn->tflags = mode2tflags[mode];
            mtn->index = index;

            AddTail((struct List *)&G->TR->transferList, (struct Node *)mtn);
          }
        }

        // now read the next Line
        if(TR_ReadLine(G->TR_Socket, buf, SIZE_LINE) <= 0)
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
    struct MinNode *curNode;

    // Now we process the read header to set all flags accordingly
    for(curNode = C->filterList.mlh_Head; curNode->mln_Succ; curNode = curNode->mln_Succ)
    {
      struct FilterNode *filter = (struct FilterNode *)curNode;

      if(DoFilterSearch(filter, mtn->mail))
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
    // and therefore we don`t throw any error
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

        // If we end up here because of an error, abort or the upper loop wasn`t finished
        // we exit immediatly with deleting the temp file also.
        if(G->Error == TRUE || G->TR->Abort == TRUE || done == FALSE)
          lline = -1;
        else if((email = MA_ExamineMail(NULL, FilePart(tf->Filename), TRUE)))
        {
          memcpy(&mail->From, &email->Mail.From, sizeof(mail->From));
          memcpy(&mail->To, &email->Mail.To, sizeof(mail->To));
          memcpy(&mail->ReplyTo, &email->Mail.ReplyTo, sizeof(mail->ReplyTo));
          strlcpy(mail->Subject, email->Mail.Subject, sizeof(mail->Subject));
          strlcpy(mail->MailFile, email->Mail.MailFile, sizeof(mail->MailFile));
          memcpy(&mail->Date, &email->Mail.Date, sizeof(mail->Date));

          // if thie function was called with -1, then the POP3 server
          // doesn`t have the UIDL command and we have to generate our
          // own one by using the MsgID and the Serverstring for the POP3
          // server.
          if(lline == -1)
          {
            char uidl[SIZE_DEFAULT+SIZE_HOST];

            snprintf(uidl, sizeof(uidl), "%s@%s", Trim(email->MsgID), C->P3[G->TR->POP_Nr]->Server);
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
  set(G->TR->GUI.TX_STATUS, MUIA_Text_Contents, tr(MSG_TR_Disconnecting));

  if(G->Error == FALSE)
    TR_SendPOP3Cmd(POPCMD_QUIT, NULL, tr(MSG_ER_BADRESPONSE));

  TR_Disconnect();
}

///
/// TR_GetMailFromNextPOP
//  Downloads and filters mail from a POP3 account
void TR_GetMailFromNextPOP(BOOL isfirst, int singlepop, enum GUILevel guilevel)
{
  static int laststats;
  int msgs, pop = singlepop;

  if(isfirst == TRUE) /* Init first connection */
  {
    G->LastDL.Error = TRUE;
    if(TR_OpenTCPIP() == FALSE)
    {
      if(guilevel == POP_USER)
        ER_NewError(tr(MSG_ER_OPENTCPIP));

      return;
    }

    if(CO_IsValid() == FALSE)
    {
      TR_CloseTCPIP();
      return;
    }
    if((G->TR = TR_New(guilevel == POP_USER ? TR_GET_USER : TR_GET_AUTO)) == NULL)
    {
      TR_CloseTCPIP();
      return;
    }
    G->TR->Checking = TRUE;
    DisplayAppIconStatistics();
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
      if(C->P3[pop] != NULL && C->P3[pop]->Enabled)
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
    G->TR->SearchCount = 0;
    MA_StartMacro(MACRO_POSTGET, itoa((int)G->TR->Stats.Downloaded));

    // tell the appicon that we are finished with checking mail
    // the apply rules or DisplayAppIconStatistics() function will refresh it later on
    G->TR->Checking = FALSE;

    // we only apply the filters if we downloaded something, or it`s wasted
    if(G->TR->Stats.Downloaded > 0)
    {
      struct Folder *folder;

      DoMethod(G->App, MUIM_CallHook, &ApplyFiltersHook, APPLY_AUTO, 0);

      // Now we jump to the first new mail we received if the number of messages has changed
      // after the mail transfer
      if(C->JumpToIncoming == TRUE)
        MA_JumpToNewMsg();

      // only call the DisplayStatistics() function if the actual folder wasn`t already the INCOMING
      // one or we would hav refreshed it twice
      if((folder = FO_GetCurrentFolder()) && !isIncomingFolder(folder))
        DisplayStatistics((struct Folder *)-1, TRUE);
      else
        DisplayAppIconStatistics();

      TR_NewMailAlert();
    }
    else
      DisplayAppIconStatistics();

    // lets populate the LastDL statistics variable with the stats
    // of this download.
    memcpy(&G->LastDL, &G->TR->Stats, sizeof(struct DownloadResult));

    MA_ChangeTransfer(TRUE);

    DisposeModulePush(&G->TR);
    if(G->TR_Exchange == TRUE)
    {
      G->TR_Exchange = FALSE;
      DoMethod(G->App, MUIM_Application_PushMethod, G->App, 3, MUIM_CallHook, &MA_SendHook, guilevel == POP_USER ? SEND_ALL_USER : SEND_ALL_AUTO);
    }

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

  // if the window isn`t open we don`t need to update it, do we?
  if(isfirst == FALSE && xget(G->TR->GUI.WI, MUIA_Window_Open) == TRUE)
  {
    char str_size_curr[SIZE_SMALL];
    char str_size_curr_max[SIZE_SMALL];

    // reset the statistics display
    snprintf(G->TR->CountLabel, sizeof(G->TR->CountLabel), tr(MSG_TR_MESSAGEGAUGE), 0);
    xset(G->TR->GUI.GA_COUNT, MUIA_Gauge_Max,      0,
                              MUIA_Gauge_Current,  0,
                              MUIA_Gauge_InfoText, G->TR->CountLabel);

    // and last, but not least update the gauge.
    FormatSize(0, str_size_curr, sizeof(str_size_curr), SF_AUTO);
    FormatSize(0, str_size_curr_max, sizeof(str_size_curr_max), SF_AUTO);
    snprintf(G->TR->BytesLabel, sizeof(G->TR->BytesLabel), tr(MSG_TR_TRANSFERSIZE),
                                                           str_size_curr, str_size_curr_max);

    xset(G->TR->GUI.GA_BYTES, MUIA_Gauge_Max,      0,
                              MUIA_Gauge_Current,  0,
                              MUIA_Gauge_InfoText, G->TR->BytesLabel);
  }

  if((msgs = TR_ConnectPOP(G->TR->GUIlevel)) != -1)     // connection succeeded
  {
    if(msgs > 0)                                       // there are messages on the server
    {
      if(TR_GetMessageList_GET())                     // message list read OK
      {
        BOOL preselect = FALSE;

        G->TR->Stats.OnServer += msgs;

        // do we have to do some remote filter actions?
        if(G->TR->SearchCount > 0)
        {
          struct MinNode *curNode;

          set(G->TR->GUI.TX_STATUS, MUIA_Text_Contents, tr(MSG_TR_ApplyFilters));
          for(curNode = G->TR->transferList.mlh_Head; curNode->mln_Succ; curNode = curNode->mln_Succ)
            TR_GetMessageDetails((struct MailTransferNode *)curNode, -2);
        }

        // if the user wants to avoid to receive the
        // same message from the POP3 server again
        // we have to analyze the UIDL of it
        if(G->TR->DuplicatesChecking == TRUE)
        {
          if(FilterDuplicates() == TRUE)
            C->P3[G->TR->POP_Nr]->UIDLchecked = TRUE;
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

            struct MinNode *curNode;

            for(curNode = G->TR->transferList.mlh_Head; curNode->mln_Succ; curNode = curNode->mln_Succ)
            {
              struct MailTransferNode *mtn = (struct MailTransferNode *)curNode;
              struct Mail *mail = mtn->mail;

              if(mail->Size >= C->WarnSize*1024)
              {
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
          G->TR->GMD_Mail = G->TR->transferList.mlh_Head;
          G->TR->GMD_Line = 0;
          TR_CompleteMsgList();
        }
        else
        {
          CallHookPkt(&TR_ProcessGETHook, 0, 0);
        }

        BusyEnd();
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
  result = (TR_SendPOP3Cmd(POPCMD_STAT, NULL, tr(MSG_ER_BADRESPONSE)) != NULL);

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
  if(G->TR_Socket != TCP_NO_SOCKET)
  {
    // now we prepare the SMTP command
    if(parmtext == NULL || parmtext[0] == '\0')
      snprintf(buf, sizeof(buf), "%s\r\n", SMTPcmd[command]);
    else
      snprintf(buf, sizeof(buf), "%s %s\r\n", SMTPcmd[command], parmtext);

    D(DBF_NET, "TCP: SMTP cmd '%s' with param '%s'", SMTPcmd[command], parmtext != NULL ? parmtext : "<NULL>");

    // lets send the command via TR_WriteLine, but not if we are in connection
    // state
    if(command == SMTP_CONNECT || TR_WriteLine(buf) > 0)
    {
      int len = 0;

      // after issuing the SMTP command we read out the server response to it
      // but only if this wasn't the SMTP_QUIT command.
      if((len = TR_ReadLine(G->TR_Socket, buf, sizeof(buf))) > 0)
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
            if((len = TR_ReadLine(G->TR_Socket, tbuf, sizeof(tbuf))) > 0)
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
      ER_NewError(errorMsg, C->SMTP_Server, (char *)SMTPcmd[command], buf);

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

  ENTER();

  // If we did a TLS negotitaion previously we have to skip the
  // welcome message, but if it was another connection like a normal or a SSL
  // one we have wait for the welcome
  if(G->TR_UseTLS == FALSE || C->SMTP_SecureMethod == SMTPSEC_SSL)
  {
    set(G->TR->GUI.TX_STATUS, MUIA_Text_Contents, tr(MSG_TR_WaitWelcome));

    result = (TR_SendSMTPCmd(SMTP_CONNECT, NULL, tr(MSG_ER_BADRESPONSE)) != NULL);
  }
  else
    result = TRUE;

  // now we either send a HELO (non-ESMTP) or EHLO (ESMTP) command to
  // signal we wanting to start a session accordingly (RFC 1869 - section 4)
  if(result && G->TR_Socket != TCP_NO_SOCKET)
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
    if(C->Use_SMTP_AUTH || C->SMTP_SecureMethod == SMTPSEC_TLS)
      resp = TR_SendSMTPCmd(ESMTP_EHLO, C->SMTP_Domain, tr(MSG_ER_BADRESPONSE));
    else
    {
      // in all other cases, we first try to get an ESMTP connection
      // and if that doesn't work we go and do a normal SMTP connection
      if((resp = TR_SendSMTPCmd(ESMTP_EHLO, C->SMTP_Domain, NULL)) == NULL)
      {
        D(DBF_NET, "ESMTP negotation failed, trying normal SMTP negotation");

        // according to RFC 1869 section 4.7 we send an RSET command
        // between the two EHLO and HELO commands to play save
        TR_SendSMTPCmd(SMTP_RSET, NULL, NULL); // no error code check

        // now we send a HELO command which signals we are not
        // going to use any ESMTP stuff
        resp = TR_SendSMTPCmd(SMTP_HELO, C->SMTP_Domain, tr(MSG_ER_BADRESPONSE));

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
      D(DBF_NET, "SMTP Server '%s' serves:", C->SMTP_Server);
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
      if(has8BITMIME(flags) == FALSE && C->Allow8bit == TRUE)
        result = FALSE;
    }
    else
    {
      result = FALSE;

      W(DBF_NET, "error on SMTP server negotation");
    }

    G->TR_SMTPflags = flags;
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
  ENTER();

  ts->Msgs_Tot = 0;
  ts->Size_Tot = 0;

  if(G->TR->GUI.GR_LIST)
  {
    set(G->TR->GUI.GR_PAGE, MUIA_Group_ActivePage, 1);
    DoMethod(G->TR->GUI.LV_MAILS, MUIM_NList_Select, MUIV_NList_Select_All, MUIV_NList_Select_Off, NULL);
  }

  if(IsListEmpty((struct List *)&G->TR->transferList) == FALSE)
  {
    // search through our transferList
    struct MinNode *curNode;

    for(curNode = G->TR->transferList.mlh_Head; curNode->mln_Succ; curNode = curNode->mln_Succ)
    {
      struct MailTransferNode *mtn = (struct MailTransferNode *)curNode;

      ts->Msgs_Tot++;

      if(hasTR_LOAD(mtn))
        ts->Size_Tot += mtn->mail->Size;
    }
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

  snprintf(G->TR->CountLabel, sizeof(G->TR->CountLabel), tr(MSG_TR_MESSAGEGAUGE), ts->Msgs_Tot);
  xset(G->TR->GUI.GA_COUNT, MUIA_Gauge_InfoText, G->TR->CountLabel,
                            MUIA_Gauge_Max,      ts->Msgs_Tot,
                            MUIA_Gauge_Current,  0);

  LEAVE();
}
///
/// TR_TransStat_NextMsg
//  Updates statistics display for next message
static void TR_TransStat_NextMsg(struct TransStat *ts, int index, int listpos, LONG size, const char *status)
{
  ENTER();

  ts->Size_Curr = 0;
  ts->Size_Curr_Max = size;

  // if the window isn`t open we don`t need to update it, do we?
  if(xget(G->TR->GUI.WI, MUIA_Window_Open))
  {
    // get the new time since the last nextmsg start
    GetSysTime(TIMEVAL(&ts->Clock_Last));

    // if we have a preselection window, update it.
    if(G->TR->GUI.GR_LIST && listpos >= 0)
      set(G->TR->GUI.LV_MAILS, MUIA_NList_Active, listpos);

    set(G->TR->GUI.TX_STATUS, MUIA_Text_Contents, status);
    set(G->TR->GUI.GA_COUNT, MUIA_Gauge_Current, index);

    // and last, but not least update the gauge.
    FormatSize(0, ts->str_size_curr, sizeof(ts->str_size_curr), SF_AUTO);
    FormatSize(size, ts->str_size_curr_max, sizeof(ts->str_size_curr_max), SF_AUTO);
    snprintf(G->TR->BytesLabel, sizeof(G->TR->BytesLabel), tr(MSG_TR_TRANSFERSIZE),
                                                            ts->str_size_curr, ts->str_size_curr_max);

    xset(G->TR->GUI.GA_BYTES, MUIA_Gauge_Max,      0,
                              MUIA_Gauge_Current,  0,
                              MUIA_Gauge_InfoText, G->TR->BytesLabel);
  }

  LEAVE();
}
///
/// TR_TransStat_Update
//  Updates statistics display for next block of data
static void TR_TransStat_Update(struct TransStat *ts, int size_incr)
{
  ENTER();

  if(size_incr > 0)
  {
    struct TimeVal now;

    ts->Size_Curr += size_incr;
    ts->Size_Done += size_incr;

    // if the window isn`t open we don`t need to update it, do we?
    if(xget(G->TR->GUI.WI, MUIA_Window_Open))
    {
      // now we check if should really update our
      // transfer display or if it will be overkill
      // we shouldn`t update it more than twice a second.
      GetSysTime(TIMEVAL(&now));
      if(-CmpTime(TIMEVAL(&now), TIMEVAL(&ts->Clock_Last)) > 0)
      {
        struct TimeVal delta;

        // how much time has passed exactly?
        delta = now;
        SubTime(TIMEVAL(&delta), TIMEVAL(&ts->Clock_Last));

        // update the display at least twice a second
        if(delta.Seconds > 0 || delta.Microseconds > 250000)
        {
          ULONG deltatime = now.Seconds - ts->Clock_Start;
          ULONG speed = 0;
          LONG remclock = 0;

          // first we calculate the speed in bytes/sec
          // to display to the user
          if(deltatime != 0)
            speed = ts->Size_Done / deltatime;

          // calculate the estimated remaining time
          if(speed != 0 && ((remclock = (ts->Size_Tot / speed) - deltatime) < 0))
            remclock = 0;

          // format the size done and size total strings
          FormatSize(ts->Size_Done, ts->str_size_done, sizeof(ts->str_size_done), SF_MIXED);
          FormatSize(ts->Size_Tot, ts->str_size_tot, sizeof(ts->str_size_tot), SF_MIXED);
          FormatSize(speed, ts->str_speed, sizeof(ts->str_speed), SF_MIXED);

          // now format the StatsLabel and update it
          snprintf(G->TR->StatsLabel, sizeof(G->TR->StatsLabel), tr(MSG_TR_TRANSFERSTATUS),
                                      ts->str_size_done, ts->str_size_tot, ts->str_speed,
                                      deltatime / 60, deltatime % 60,
                                      remclock / 60, remclock % 60);

          set(G->TR->GUI.TX_STATS, MUIA_Text_Contents, G->TR->StatsLabel);

          // update the gauge
          FormatSize(ts->Size_Curr, ts->str_size_curr, sizeof(ts->str_size_curr), SF_AUTO);
          snprintf(G->TR->BytesLabel, sizeof(G->TR->BytesLabel), tr(MSG_TR_TRANSFERSIZE),
                                                                 ts->str_size_curr, ts->str_size_curr_max);
          xset(G->TR->GUI.GA_BYTES, MUIA_Gauge_Max,      ts->Size_Curr_Max / 1024,
                                    MUIA_Gauge_Current,  ts->Size_Curr / 1024,
                                    MUIA_Gauge_InfoText, G->TR->BytesLabel);

          // signal the application to update now
          DoMethod(G->App, MUIM_Application_InputBuffered);

          ts->Clock_Last = now;
        }
      }
    }
  }
  else if(size_incr == TS_SETMAX)
  {
    struct TimeVal now;
    ULONG deltatime;
    ULONG speed = 0;
    LONG remclock = 0;

    // first update the total transferred size
    ts->Size_Done += ts->Size_Curr_Max - ts->Size_Curr;
    // we are done with this mail, so make sure the current size equals the final size
    ts->Size_Curr = ts->Size_Curr_Max;

    // if the window isn`t open we don`t need to update it, do we?
    if(xget(G->TR->GUI.WI, MUIA_Window_Open))
    {
      char size_done[SIZE_SMALL];
      char size_total[SIZE_SMALL];
      char speed_str[SIZE_SMALL];

      // we make sure that we, at least update the gauge at the end
      GetSysTime(TIMEVAL(&now));
      deltatime = now.Seconds - ts->Clock_Start;

      // first we calculate the speed in bytes/sec
      // to display to the user
      if(deltatime != 0)
        speed = ts->Size_Done / deltatime;

      // calculate the estimated remaining time
      if(speed != 0 && ((remclock = (ts->Size_Tot / speed)-deltatime) < 0))
        remclock = 0;

      // format the size done and size total strings
      FormatSize(ts->Size_Done, size_done, sizeof(size_done), SF_MIXED);
      FormatSize(ts->Size_Tot, size_total, sizeof(size_total), SF_MIXED);
      FormatSize(speed, speed_str, sizeof(speed_str), SF_MIXED);

      // now format the StatsLabel and update it
      snprintf(G->TR->StatsLabel, sizeof(G->TR->StatsLabel), tr(MSG_TR_TRANSFERSTATUS),
                                  size_done, size_total, speed_str,
                                  deltatime / 60, deltatime % 60,
                                  remclock / 60, remclock % 60);

      set(G->TR->GUI.TX_STATS, MUIA_Text_Contents, G->TR->StatsLabel);

      // if size_increment is a negative number it is
      // a signal that we are at the end of the transfer, so we can put up
      // the gauge to maximum and show 100% size progress
      FormatSize(ts->Size_Curr, ts->str_size_curr, sizeof(ts->str_size_curr), SF_AUTO);
      snprintf(G->TR->BytesLabel, sizeof(G->TR->BytesLabel), tr(MSG_TR_TRANSFERSIZE),
                                                             ts->str_size_curr, ts->str_size_curr_max);
      xset(G->TR->GUI.GA_BYTES, MUIA_Gauge_Max,     100,
                                MUIA_Gauge_Current, 100,
                                MUIA_Gauge_InfoText, G->TR->BytesLabel);

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
  ENTER();

  if(G->TR->GUI.LV_MAILS != NULL)
    DoMethod(G->TR->GUI.LV_MAILS, MUIM_NList_Clear);

  if(IsListEmpty((struct List *)&G->TR->transferList) == FALSE)
  {
    struct MinNode *curNode;

    while((curNode = (struct MinNode *)RemHead((struct List *)&G->TR->transferList)) != NULL)
    {
      struct MailTransferNode *mtn = (struct MailTransferNode *)curNode;

      // free the mail pointer
      if(mtn->mail != NULL)
        free(mtn->mail);

      // free the UIDL
      if(mtn->UIDL != NULL)
        free(mtn->UIDL);

      // free the node itself
      free(mtn);
    }
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
  DisposeModulePush(&G->TR);

  LEAVE();
}
///
/// TR_ApplySentFilters
//  Applies filters to a sent message
static BOOL TR_ApplySentFilters(struct Mail *mail)
{
  struct MinNode *curNode;

  // only if we have a positiv search count we start
  // our filtering at all, otherwise we return immediatly
  if(G->TR->SearchCount > 0)
  {
    // Now we process the read header to set all flags accordingly
    for(curNode = C->filterList.mlh_Head; curNode->mln_Succ; curNode = curNode->mln_Succ)
    {
      struct FilterNode *filter = (struct FilterNode *)curNode;

      if(DoFilterSearch(filter, mail))
      {
        if(!ExecuteFilterAction(filter, mail))
          return FALSE;
      }
    }
  }

  return TRUE;
}
///

/*** UIDL (Avoid duplicates) management ***/
/// InitUIDLhash()
// Initialize the UIDL list and load it from the .uidl file
static BOOL InitUIDLhash(void)
{
  BOOL result = FALSE;

  // The default UIDLhashTable operators
  static const struct HashTableOps UIDLhashTableOps =
  {
    DefaultHashAllocTable,
    DefaultHashFreeTable,
    DefaultHashGetKey,
    StringHashHashKey,
    StringHashMatchEntry,
    DefaultHashMoveEntry,
    StringHashClearEntry,
    DefaultHashFinalize,
    NULL,
  };

  ENTER();

  // make sure no other UIDLhashTable is active
  if(G->TR->UIDLhashTable != NULL)
    CleanupUIDLhash();

  // allocate a new hashtable for managing the UIDL data
  if((G->TR->UIDLhashTable = HashTableNew((struct HashTableOps *)&UIDLhashTableOps, NULL, sizeof(struct UIDLtoken), 512)) != NULL)
  {
    FILE *fh;
    char *filename = CreateFilename(".uidl");
    LONG size;

    // open the .uidl file and read in the UIDL/MsgIDs
    // line-by-line
    if(ObtainFileInfo(filename, FI_SIZE, &size) == TRUE && size > 0 && (fh = fopen(filename, "r")) != NULL)
    {
      char uidl[SIZE_DEFAULT+SIZE_HOST];

      setvbuf(fh, NULL, _IOFBF, SIZE_FILEBUF);

      while(GetLine(fh, uidl, sizeof(uidl)))
        AddUIDLtoHash(uidl, FALSE);

      fclose(fh);

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
      int i;

      for(i=0; i < MAXP3; i++)
      {
        if(C->P3[i] && C->P3[i]->UIDLchecked == FALSE &&
           strcmp(p, C->P3[i]->Server) == 0)
        {
          // if we reach here than this uidl is part of
          // a server we didn't check, so we can ignore it
          break;
        }
      }

      // if we reached MAXP3 then
      // we found an orphaned uidl which we can ignore
      if(i < MAXP3)
        saveUIDL = TRUE;
      else
        D(DBF_UIDL, "orphaned UIDL found and deleted '%s'", token->uidl);
    }
  }
  else
    saveUIDL = TRUE;

  if(saveUIDL)
  {
    fprintf(fh, "%s\n", token->uidl);
    D(DBF_UIDL, "saved UIDL '%s' to .uidl file", token->uidl);
  }

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
    // save the UIDLs only if something has been changed or if there are
    // some entries to be saved at all
  	if(G->TR->UIDLhashIsDirty == TRUE || G->TR->UIDLhashTable->entryCount > 0)
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

  ENTER();

  // we first make sure the UIDL list is loaded from disk
  if(G->TR->UIDLhashTable != NULL)
  {
    // check if there is anything to transfer at all
    if(IsListEmpty((struct List *)&G->TR->transferList) == FALSE)
    {
      // inform the user of the operation
      set(G->TR->GUI.TX_STATUS, MUIA_Text_Contents, tr(MSG_TR_CHECKUIDL));

      // before we go and request each UIDL of a message we check wheter the server
      // supports the UIDL command at all
      if(TR_SendPOP3Cmd(POPCMD_UIDL, NULL, NULL) != NULL)
      {
        char buf[SIZE_LINE];

        // get the first line the pop server returns after the UIDL command
        if(TR_ReadLine(G->TR_Socket, buf, SIZE_LINE) > 0)
        {
          // we get the "unique-id list" as long as we haven`t received a a
          // finishing octet
          while(G->TR->Abort == FALSE && G->Error == FALSE && strncmp(buf, ".\r\n", 3) != 0)
          {
            int num;
            char uidl[SIZE_DEFAULT+SIZE_HOST];
            struct MinNode *curNode;

            // now parse the line and get the message number and UIDL
            sscanf(buf, "%d %s", &num, uidl);

            // lets add our own ident to the uidl so that we can compare
            // it against our saved list
            strlcat(uidl, "@", sizeof(uidl));
            strlcat(uidl, C->P3[G->TR->POP_Nr]->Server, sizeof(uidl));

            // search through our transferList
            for(curNode = G->TR->transferList.mlh_Head; G->TR->Abort == FALSE && G->Error == FALSE && curNode->mln_Succ; curNode = curNode->mln_Succ)
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
                    MASK_FLAG(mtn->tflags, TRF_DELETE);

                    // mark the UIDLtoken as being checked
                    token->checked = TRUE;

                    D(DBF_UIDL, "mail %ld: UIDL '%s' was FOUND!", mtn->index, mtn->UIDL);
                  }
                }

                break;
              }
            }

            // now read the next Line
            if(TR_ReadLine(G->TR_Socket, buf, SIZE_LINE) <= 0)
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
        struct MinNode *curNode;

        W(DBF_UIDL, "POP3 server '%s' doesn't support UIDL command!", C->P3[G->TR->POP_Nr]->Server);

        // search through our transferList
        for(curNode = G->TR->transferList.mlh_Head; G->TR->Abort == FALSE && G->Error == FALSE && curNode->mln_Succ; curNode = curNode->mln_Succ)
        {
          struct MailTransferNode *mtn = (struct MailTransferNode *)curNode;

          // if the server doesn`t support the UIDL command we
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
              MASK_FLAG(mtn->tflags, TRF_DELETE);

              D(DBF_UIDL, "mail %ld: UIDL '%s' was FOUND!", mtn->index, mtn->UIDL);
            }
          }
        }
      }

      result = (G->TR->Abort == FALSE && G->Error == FALSE);
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

      D(DBF_UIDL, "added '%s' (%lx) to UIDLhash", uidl, token);
    }
    else
      W(DBF_UIDL, "already existing hash entry for '%s' found, skipping.", uidl);
  }
  else
    E(DBF_UIDL, "couldn't add uidl '%s' to hash", uidl);

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
BOOL TR_ProcessEXPORT(char *fname, struct Mail **mlist, BOOL append)
{
  BOOL success = FALSE;
  BOOL abort = FALSE;
  int i;

  ENTER();

  // reset our processing list
  NewList((struct List *)&G->TR->transferList);

  // temporarly copy all data out of our mlist to the
  // processing list and mark all mails to get "loaded"
  for(i = 0; i < (int)*mlist; i++)
  {
    struct Mail *mail = mlist[i + 2];

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
  }

  // if we have now something in our processing list,
  // lets go on
  if(abort == FALSE &&
     IsListEmpty((struct List *)&G->TR->transferList) == FALSE)
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
      struct MinNode *curNode;

      setvbuf(fh, NULL, _IOFBF, SIZE_FILEBUF);

      success = TRUE;

      for(curNode = G->TR->transferList.mlh_Head; curNode->mln_Succ && G->TR->Abort == FALSE && success; curNode = curNode->mln_Succ)
      {
        struct MailTransferNode *mtn = (struct MailTransferNode *)curNode;
        struct Mail *mail = mtn->mail;
        char fullfile[SIZE_PATHFILE];

        // update the transfer status
        ts.Msgs_Done++;
        TR_TransStat_NextMsg(&ts, mtn->index, -1, mail->Size, tr(MSG_TR_Exporting));

        if(StartUnpack(GetMailFile(NULL, NULL, mail), fullfile, mail->Folder))
        {
          FILE *mfh;

          // open the message file to start exporting it
          if((mfh = fopen(fullfile, "r")) != NULL)
          {
            char datstr[64];
            char buf[SIZE_LINE];
            BOOL inHeader = TRUE;

            // printf out our leading "From " MBOX format line first
            DateStamp2String(datstr, sizeof(datstr), &mail->Date, DSS_UNIXDATE, TZC_NONE);
            fprintf(fh, "From %s %s", mail->From.Address, datstr);

            // let us put out the Status: header field
            fprintf(fh, "Status: %s\n", MA_ToStatusHeader(mail));

            // let us put out the X-Status: header field
            fprintf(fh, "X-Status: %s\n", MA_ToXStatusHeader(mail));

            // initialize buf first
            buf[0] = '\0';

            // now we iterate through every line of our mail and try to substitute
            // found "From " line with quoted ones
            while(G->TR->Abort == FALSE && fgets(buf, SIZE_LINE, mfh))
            {
              char *tmp = buf;

              // check if this is a single \n so that it
              // signals the end if a line
              if(*buf == '\n')
              {
                inHeader = FALSE;
                fputs(buf, fh);
                continue;
              }

              // the mboxrd format specifies that we need to quote any
              // From, >From, >>From etc. occurance.
              // http://www.qmail.org/man/man5/mbox.html
              while(*tmp == '>')
                tmp++;

              if(strncmp(tmp, "From ", 5) == 0)
              {
                fputc('>', fh);
              }
              else if(inHeader)
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
              fputs(buf, fh);

              // make sure we have a newline at the end of the line
              if(buf[strlen(buf)-1] != '\n')
                fputc('\n', fh);

              // update the transfer status
              TR_TransStat_Update(&ts, strlen(buf));
            }

            // check why we exited the while() loop and
            // if everything is fine
            if(feof(mfh) == 0)
            {
              E(DBF_NET, "error on reading data!");

              // an error occurred, lets return -1
              success = FALSE;
            }

            // close file pointer
            fclose(mfh);

            // put the transferStat to 100%
            TR_TransStat_Update(&ts, TS_SETMAX);
          }
          else
           success = FALSE;

          FinishUnpack(fullfile);
        }
        else
          success = FALSE;
      }

      // close file pointer
      fclose(fh);

      // write the status to our logfile
      AppendToLogfile(LF_ALL, 51, tr(MSG_LOG_Exporting), ts.Msgs_Done, mlist[2]->Folder->Name, fname);
    }
  }

  TR_AbortnClose();

  RETURN(success);
  return success;
}
///

/*** SEND ***/
/// TR_SendMessage
//  Sends a single message
static int TR_SendMessage(struct TransStat *ts, struct Mail *mail)
{
  int result = 0;
  struct Folder *outfolder = FO_GetFolderByType(FT_OUTGOING, NULL);
  char *mf;
  FILE *fh;

  ENTER();

  D(DBF_NET, "about to send mail '%s' via SMTP", mail->MailFile);

  // open the mail file for reading
  if((fh = fopen(mf = GetMailFile(NULL, outfolder, mail), "r")) != NULL)
  {
    char buf[SIZE_LINE];

    setvbuf(fh, NULL, _IOFBF, SIZE_FILEBUF);

    // now we put together our parameters for our MAIL command
    // which in fact may contain serveral parameters as well according
    // to ESMTP extensions.
    snprintf(buf, sizeof(buf), "FROM:<%s>", C->EmailAddress);

    // in case the server supports the ESMTP SIZE extension lets add the
    // size
    if(hasSIZE(G->TR_SMTPflags) && mail->Size > 0)
      snprintf(buf, sizeof(buf), "%s SIZE=%ld", buf, mail->Size);

    // in case the server supports the ESMTP 8BITMIME extension we can
    // add information about the encoding mode
    if(has8BITMIME(G->TR_SMTPflags))
      snprintf(buf, sizeof(buf), "%s BODY=%s", buf, C->Allow8bit ? "8BITMIME" : "7BIT");

    // send the MAIL command with the FROM: message
    if(TR_SendSMTPCmd(SMTP_MAIL, buf, tr(MSG_ER_BADRESPONSE)))
    {
      struct ExtendedMail *email = MA_ExamineMail(outfolder, mail->MailFile, TRUE);

      if(email)
      {
        BOOL rcptok = TRUE;
        int j;

        // specify the main 'To:' recipient
        snprintf(buf, sizeof(buf), "TO:<%s>", mail->To.Address);
        if(!TR_SendSMTPCmd(SMTP_RCPT, buf, tr(MSG_ER_BADRESPONSE)))
          rcptok = FALSE;

        // now add the additional 'To:' recipients of the mail
        for(j=0; j < email->NoSTo && rcptok; j++)
        {
          snprintf(buf, sizeof(buf), "TO:<%s>", email->STo[j].Address);
          if(!TR_SendSMTPCmd(SMTP_RCPT, buf, tr(MSG_ER_BADRESPONSE)))
            rcptok = FALSE;
        }

        // add the 'Cc:' recipients
        for(j=0; j < email->NoCC && rcptok; j++)
        {
          snprintf(buf, sizeof(buf), "TO:<%s>", email->CC[j].Address);
          if(!TR_SendSMTPCmd(SMTP_RCPT, buf, tr(MSG_ER_BADRESPONSE)))
            rcptok = FALSE;
        }

        // add the 'BCC:' recipients
        for(j=0; j < email->NoBCC && rcptok; j++)
        {
          snprintf(buf, sizeof(buf), "TO:<%s>", email->BCC[j].Address);
          if(!TR_SendSMTPCmd(SMTP_RCPT, buf, tr(MSG_ER_BADRESPONSE)))
            rcptok = FALSE;
        }

        if(rcptok)
        {
          D(DBF_NET, "RCPTs accepted, sending mail data");

          // now we send the actual main data of the mail
          if(TR_SendSMTPCmd(SMTP_DATA, NULL, tr(MSG_ER_BADRESPONSE)))
          {
            BOOL lineskip = FALSE;
            BOOL inbody = FALSE;
            char sendbuf[SIZE_LINE+2];
            int sendsize;
            int cpos;
            int prevpos = ftell(fh); // get current file position
            int startpos = prevpos;
            int sentbytes = 0;

            // as long there is no abort situation we go on reading out
            // from the stream and sending it to our SMTP server
            while(G->TR->Abort == FALSE && G->Error == FALSE && fgets(buf, SIZE_LINE, fh))
            {
              sendsize = cpos = ftell(fh)-prevpos; // get the size we really read out from the stream.
              prevpos += sendsize;                 // set the new prevpos to the ftell() value.

              // as long as we process header lines we have to make differ in some ways.
              if(inbody == FALSE)
              {
                // we check if we found the body of the mail now
                // the start of a body is seperated by the header with a single
                // empty line and we have to make sure that it isn`t the beginning of the file
                if(sendsize == 1 && buf[0] == '\n' && buf[1] == '\0' && prevpos-startpos > 1)
                {
                  inbody = TRUE;
                  lineskip = FALSE;
                }
                else if(!isspace(*buf)) // headerlines don`t start with a space
                {
                  // headerlines with bcc or x-yam- will be skipped by us.
                  lineskip = (strnicmp(buf, "bcc", 3) == 0 || strnicmp(buf, "x-yam-", 6) == 0);
                }
              }

              // if we don`t skip this line we write it out to the SMTP server
              if(lineskip == FALSE)
              {
                // RFC 821 says a starting period needs a second one
                if(buf[0] == '.')
                {
                  sendbuf[0] = buf[0];
                  sendsize++;
                }

                // lets copy everything into our sendbuffer
                memcpy(&sendbuf[sendsize-cpos], &buf[0], (size_t)cpos+1);

                // RFC 2822 doesn`t allow bare CR and LF so we have to put a CR before a LF
                if(sendbuf[sendsize-1] == '\n')
                {
                  sendbuf[sendsize-1] = '\r';
                  sendbuf[sendsize]   = '\n';
                  sendbuf[sendsize+1] = '\0';
                  sendsize++;
                }

                // now lets send the data buffered to the socket.
                // we will flush it later then.
                if(TR_Send(sendbuf, sendsize, TCPF_NONE) <= 0)
                  ER_NewError(tr(MSG_ER_CONNECTIONBROKEN), C->SMTP_Server, (char *)SMTPcmd[SMTP_DATA]);
                else
                  sentbytes += sendsize;
              }

              TR_TransStat_Update(ts, cpos);
            }

            D(DBF_NET, "transfered %ld bytes (raw: %ld bytes)", sentbytes, mail->Size);

            // if buf == NULL when we arrive here, then the fgets()
            // at the top exited with an error
            if(buf == NULL)
            {
              ER_NewError(tr(MSG_ER_ErrorReadMailfile), mf);
              result = -1; // signal error
            }
            else if(G->TR->Abort == FALSE && G->Error == FALSE)
            {
              // we have to flush the write buffer if this wasn`t a error or
              // abort situation
              TR_WriteFlush();

              // send a CRLF+octet "\r\n." to signal that the data is finished.
              // we do it here because if there was an error and we send it, the message
              // will be send incomplete.
              if(TR_SendSMTPCmd(SMTP_FINISH, NULL, tr(MSG_ER_BADRESPONSE)))
              {
                // put the transferStat to 100%
                TR_TransStat_Update(ts, TS_SETMAX);

                // now that we are at 100% we have to set the transfer Date of the message
                GetSysTimeUTC(&mail->Reference->transDate);

                result = email->DelSend ? 2 : 1;
                AppendToLogfile(LF_VERBOSE, 42, tr(MSG_LOG_SendingVerbose), AddrName(mail->To), mail->Subject, mail->Size);
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
  else
    ER_NewError(tr(MSG_ER_CantOpenFile), mf);

  RETURN(result);
  return result;
}
///
/// TR_ProcessSEND
//  Sends a list of messages
BOOL TR_ProcessSEND(struct Mail **mlist, enum SendMode mode)
{
  BOOL success = FALSE;

  ENTER();

  // start the PRESEND macro first
  MA_StartMacro(MACRO_PRESEND, NULL);

  // try to open the TCP/IP stack
  if(TR_OpenTCPIP() == TRUE)
  {
    // verify that the configuration is ready for sending mail
    if(CO_IsValid() == TRUE && (G->TR = TR_New((mode == SEND_ALL_AUTO || mode == SEND_ACTIVE_AUTO) ? TR_SEND_AUTO : TR_SEND_USER)) != NULL)
    {
      // open the transfer window
      if(SafeOpenWindow(G->TR->GUI.WI) == TRUE)
      {
        int c;
        int i;

        NewList((struct List *)&G->TR->transferList);
        G->TR_Allow = FALSE;
        G->TR->Abort = FALSE;
        G->Error = FALSE;

        // now we build the list of mails which should
        // be transfered.
        for(c = i = 0; i < (int)*mlist; i++)
        {
          struct Mail *mail = mlist[i + 2];

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
                  newMail->Next = NULL;

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

        // just go on if we really have something
        if(c > 0)
        {
          // now we have to check whether SSL/TLS is selected for SMTP account,
          // and if it is usable. Or if no secure connection is requested
          // we can go on right away.
          if(C->SMTP_SecureMethod == SMTPSEC_NONE ||
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
            strlcpy(host, C->SMTP_Server, sizeof(host));

            // If the hostname has a explicit :xxxxx port statement at the end we
            // take this one, even if its not needed anymore.
            if((p = strchr(host, ':')) != NULL)
            {
              *p = '\0';
              port = atoi(++p);
            }
            else
              port = C->SMTP_Port;

            set(G->TR->GUI.TX_STATUS, MUIA_Text_Contents, tr(MSG_TR_Connecting));

            BusyText(tr(MSG_TR_MailTransferTo), host);

            TR_SetWinTitle(FALSE, host);

            if((err = TR_Connect(host, port)) == CONNECTERR_SUCCESS)
            {
              BOOL connected = FALSE;

              // first we check whether the user wants to connect to a plain SSLv3 server
              // so that we initiate the SSL connection now
              if(C->SMTP_SecureMethod == SMTPSEC_SSL)
              {
                // lets try to establish the SSL connection via AmiSSL
                if(TR_InitTLS() == TRUE && TR_StartTLS() == TRUE)
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
                if(C->SMTP_SecureMethod == SMTPSEC_TLS && connected)
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
                if(C->Use_SMTP_AUTH == TRUE && connected == TRUE)
                  connected = TR_InitSMTPAUTH();
              }

              // If we are still "connected" we can proceed with transfering the data
              if(connected == TRUE)
              {
                struct Folder *outfolder = FO_GetFolderByType(FT_OUTGOING, NULL);
                struct Folder *sentfolder = FO_GetFolderByType(FT_SENT, NULL);
                struct MinNode *curNode;

                // set the success to TRUE as everything worked out fine
                // until here.
                success = TRUE;
                AppendToLogfile(LF_VERBOSE, 41, tr(MSG_LOG_ConnectSMTP), host);

                for(curNode = G->TR->transferList.mlh_Head; curNode->mln_Succ; curNode = curNode->mln_Succ)
                {
                  struct MailTransferNode *mtn = (struct MailTransferNode *)curNode;
                  struct Mail *mail = mtn->mail;

                  if(G->TR->Abort == TRUE || G->Error == TRUE)
                    break;

                  ts.Msgs_Done++;
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
                      if(TR_ApplySentFilters(mail->Reference))
                        MA_MoveCopy(mail->Reference, outfolder, sentfolder, FALSE, TRUE);
                    }
                    break;

                    // 2 means we filter and delete afterwards
                    case 2:
                    {
                      setStatusToSent(mail->Reference);
                      if (TR_ApplySentFilters(mail->Reference))
                        MA_DeleteSingle(mail->Reference, FALSE, FALSE, FALSE);
                    }
                    break;
                  }
                }

                AppendToLogfile(LF_NORMAL, 40, tr(MSG_LOG_Sending), c, host);

                // now we can disconnect from the SMTP
                // server again
                set(G->TR->GUI.TX_STATUS, MUIA_Text_Contents, tr(MSG_TR_Disconnecting));

                // send a 'QUIT' command, but only if
                // we didn't receive any error during the transfer
                if(G->Error == FALSE)
                  TR_SendSMTPCmd(SMTP_QUIT, NULL, tr(MSG_ER_BADRESPONSE));
              }
              else
              {
                // check if we end up here cause of the 8BITMIME differences
                if(has8BITMIME(G->TR_SMTPflags) == FALSE && C->Allow8bit == TRUE)
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

            // if we got an error here, let`s throw it
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
                ER_NewError(tr(MSG_ER_CONNECTERR_SOCKET_IN_USE), host);
              break;

              // socket() execution failed
              case CONNECTERR_NO_SOCKET:
                ER_NewError(tr(MSG_ER_CONNECTERR_NO_SOCKET), host);
              break;

              // couldn't establish non-blocking IO
              case CONNECTERR_NO_NONBLOCKIO:
                ER_NewError(tr(MSG_ER_CONNECTERR_NO_NONBLOCKIO), host);
              break;

              // the specified hostname isn't valid, so
              // lets tell the user
              case CONNECTERR_UNKNOWN_HOST:
                ER_NewError(tr(MSG_ER_UnknownSMTP), host);
              break;

              // the connection request timed out, so tell
              // the user
              case CONNECTERR_TIMEDOUT:
                ER_NewError(tr(MSG_ER_CONNECTERR_TIMEDOUT), host);
              break;

              // an error occurred while checking for 8bit MIME
              // compatibility
              case CONNECTERR_INVALID8BIT:
                ER_NewError(tr(MSG_ER_NO8BITMIME), host);
              break;

              // error during initialization of an SSL connection
              case CONNECTERR_SSLFAILED:
                ER_NewError(tr(MSG_ER_INITTLS), host);
              break;

              // an unknown error occurred so lets show
              // a generic error message
              case CONNECTERR_UNKNOWN_ERROR:
                ER_NewError(tr(MSG_ER_CantConnect), host);
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

    // make sure to close the TCP/IP
    // connection completly
    TR_CloseTCPIP();
  }
  else
    ER_NewError(tr(MSG_ER_OPENTCPIP));

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

  if((email = MA_ExamineMail(NULL, tfname, FALSE)))
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

  D(DBF_IMPORT, "reading message from addr 0x%lx", addr);

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
    E(DBF_IMPORT, "Unable to seek at %x", addr);
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

    D(DBF_IMPORT, "read in %ld bytes of object at 0x%x, but index length is %ld", size, addr, length_of_idxs);

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
    E(DBF_IMPORT, "Couldn't read dbx message @ addr %x", msg_addr);

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

    E(DBF_IMPORT, "Unable to seek at %x", addr);
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
  if(G->TR->ImportFile[0] != '\0' && G->TR->ImportFolder != NULL)
  {
    char tfname[SIZE_MFILE];
    char fname[SIZE_PATHFILE];
    int c = 0;

    // clear the found mail list per default
    NewList((struct List *)&G->TR->transferList);

    // prepare the temporary filename buffers
    snprintf(tfname, sizeof(tfname), "YAMi%08lx.tmp", GetUniqueID());
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
          char buffer[SIZE_LINE];
          BOOL foundBody = FALSE;
          int size = 0;
          long addr = 0;

          setvbuf(ifh, NULL, _IOFBF, SIZE_FILEBUF);

          while(GetLine(ifh, buffer, SIZE_LINE))
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
  if(IsListEmpty((struct List *)&G->TR->transferList) == FALSE)
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
            struct MinNode *curNode;

            setvbuf(ifh, NULL, _IOFBF, SIZE_FILEBUF);

            // iterate through our transferList and seek to
            // each position/address of a mail
            for(curNode = G->TR->transferList.mlh_Head; curNode->mln_Succ && G->TR->Abort == FALSE; curNode = curNode->mln_Succ)
            {
              struct MailTransferNode *mtn = (struct MailTransferNode *)curNode;
              struct Mail *mail = mtn->mail;
              FILE *ofh = NULL;
              char mfile[SIZE_MFILE];
              char buffer[SIZE_LINE];
              BOOL foundBody = FALSE;
              unsigned int stat = SFLAG_NONE;
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
              while(GetLine(ifh, buffer, SIZE_LINE) && G->TR->Abort == FALSE)
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
                      if(ownStatusFound == TRUE)
                        stat |= MA_FromXStatusHeader(&buffer[10]);
                      else
                        stat = MA_FromXStatusHeader(&buffer[10]);

                      ownStatusFound = TRUE;
                    }
                    else if(strnicmp(buffer, "Status: ", 8) == 0)
                    {
                      if(ownStatusFound == TRUE)
                        stat |= MA_FromStatusHeader(&buffer[8]);
                      else
                        stat = MA_FromStatusHeader(&buffer[8]);

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
                TR_TransStat_Update(&ts, strlen(buffer)+1);
              }

              fclose(ofh);
              ofh = NULL;

              // after writing out the mail to a
              // new mail file we go and add it to the folder
              if(ownStatusFound == FALSE)
              {
                // define the default status flags depending on the
                // folder
                if(ftype == FT_OUTGOING)
                  stat = SFLAG_QUEUED | SFLAG_READ;
                else if(ftype == FT_SENT || ftype == FT_CUSTOMSENT)
                  stat = SFLAG_SENT | SFLAG_READ;
                else
                  stat = SFLAG_NEW;
              }

              // set the status flags now
              SET_FLAG(mail->sflags, stat);

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
                TR_TransStat_Update(&ts, TS_SETMAX);
              }
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
            struct MinNode *curNode;

            setvbuf(ifh, NULL, _IOFBF, SIZE_FILEBUF);

            // iterate through our transferList and seek to
            // each position/address of a mail
            for(curNode = G->TR->transferList.mlh_Head; curNode->mln_Succ && G->TR->Abort == FALSE; curNode = curNode->mln_Succ)
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
                E(DBF_IMPORT, "Couldn't import dbx message from addr %x", mtn->importAddr);

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
                TR_TransStat_Update(&ts, TS_SETMAX);
              }
            }

            fclose(ifh);
          }

          case IMF_UNKNOWN:
            // nothing
          break;
        }
        break;
      }

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
   // make sure the NOOP timer is definitly stopped
   TC_Stop(TIO_POP3_KEEPALIVE);

   // first set the Abort variable so that other can benefit from it
   G->TR->Abort = TRUE;

   // we can easily abort the transfer by setting the POP_Nr to the
   // highest value possible and issue a GetMailFromNextPOP command.
   // with this solution YAM will also process the filters for mails that
   // were already downloaded even if the user aborted the transfer somehow
   G->TR->POP_Nr = MAXP3;
   TR_GetMailFromNextPOP(FALSE, MAXP3, 0);
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
    if(TR_SendPOP3Cmd(POPCMD_RETR, msgnum, tr(MSG_ER_BADRESPONSE)) != NULL)
    {
      // now we call a subfunction to receive data from the POP3 server
      // and write it in the filehandle as long as there is no termination \r\n.\r\n
      if(TR_RecvToFile(fh, msgfile, ts) > 0)
        done = TRUE;
    }
    fclose(fh);

    if(G->TR->Abort == FALSE && G->Error == FALSE && done == TRUE)
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
static BOOL TR_DeleteMessage(int number)
{
  BOOL result = FALSE;
  char msgnum[SIZE_SMALL];

  ENTER();

  snprintf(msgnum, sizeof(msgnum), "%d", number);

  // inform others of the delete operation
  set(G->TR->GUI.TX_STATUS, MUIA_Text_Contents, tr(MSG_TR_DeletingServerMail));

  if(TR_SendPOP3Cmd(POPCMD_DELE, msgnum, tr(MSG_ER_BADRESPONSE)) != NULL)
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

  // show the statistics only if we downloaded some mails at all,
  // and not all of them were spam mails
  if(stats->Downloaded != 0 && rr->Checked > rr->Spam)
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
        snprintf(&buffer[strlen(buffer)], sizeof(buffer)-strlen(buffer), tr(MSG_TR_FilterStats), rr->Checked,
                                                                         rr->Bounced,
                                                                         rr->Forwarded,
                                                                         rr->Replied,
                                                                         rr->Executed,
                                                                         rr->Moved,
                                                                         rr->Deleted);
      }

      // show the info window.
      InfoWindow(tr(MSG_TR_NewMail), buffer, tr(MSG_Okay), G->MA->GUI.WI, G->TR->GUIlevel == POP_USER);
    }

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
  struct TransStat ts;

  ENTER();

  // initialize the transfer statistics
  TR_TransStat_Init(&ts);

  // make sure the NOOP timer is definitly stopped
  TC_Stop(TIO_POP3_KEEPALIVE);

  if(ts.Msgs_Tot > 0)
  {
    struct Folder *infolder = FO_GetFolderByType(FT_INCOMING, NULL);
    struct MinNode *curNode;

    if(C->TransferWindow == TWM_SHOW && !xget(G->TR->GUI.WI, MUIA_Window_Open))
      set(G->TR->GUI.WI, MUIA_Window_Open, TRUE);

    TR_TransStat_Start(&ts);

    for(curNode = G->TR->transferList.mlh_Head; curNode->mln_Succ && G->TR->Abort == FALSE && G->Error == FALSE; curNode = curNode->mln_Succ)
    {
      struct MailTransferNode *mtn = (struct MailTransferNode *)curNode;
      struct Mail *mail = mtn->mail;

      if(hasTR_LOAD(mtn))
      {
        TR_TransStat_NextMsg(&ts, mtn->index, mtn->position, mail->Size, tr(MSG_TR_Downloading));

        if(TR_LoadMessage(infolder, &ts, mtn->index) == TRUE)
        {
          // put the transferStat to 100%
          TR_TransStat_Update(&ts, TS_SETMAX);

          G->TR->Stats.Downloaded++;

          if(hasTR_DELETE(mtn))
          {
            if(TR_DeleteMessage(mtn->index) && G->TR->DuplicatesChecking == TRUE)
            {
              // remove the UIDL from the hash table and remember that change
              RemoveUIDLfromHash(mtn->UIDL);
              G->TR->UIDLhashIsDirty = TRUE;
            }
          }
          else if(G->TR->DuplicatesChecking == TRUE)
          {
            // add the UIDL to the hash table and remember that change
            AddUIDLtoHash(mtn->UIDL, TRUE);
            G->TR->UIDLhashIsDirty = TRUE;
          }
        }
      }
      else if(hasTR_DELETE(mtn))
      {
        TR_TransStat_NextMsg(&ts, mtn->index, mtn->position, mail->Size, tr(MSG_TR_Downloading));

        if(TR_DeleteMessage(mtn->index) == TRUE && G->TR->DuplicatesChecking == TRUE)
        {
          // remove the UIDL from the hash table and remember that change
          RemoveUIDLfromHash(mtn->UIDL);
          G->TR->UIDLhashIsDirty = TRUE;
        }
      }
    }

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
   line = xget(G->TR->GUI.LV_MAILS, MUIA_NList_Active);
   DoMethod(G->TR->GUI.LV_MAILS, MUIM_NList_GetEntry, line, &mtn);
   TR_GetMessageDetails(mtn, line);
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

    for(; curNode->mln_Succ && tr->Abort == FALSE && G->Error == FALSE; curNode = curNode->mln_Succ)
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
    TC_Restart(TIO_POP3_KEEPALIVE, C->KeepAliveInterval, 0);
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
    TC_Restart(TIO_POP3_KEEPALIVE, C->KeepAliveInterval, 0);
    return;
  }

  G->TR->Pause = FALSE;
  TR_CompleteMsgList();

  LEAVE();
}
MakeStaticHook(TR_PauseHook, TR_PauseFunc);
///

/*** GUI ***/
/// TR_LV_DspFunc
//  Message listview display hook
HOOKPROTONH(TR_LV_DspFunc, long, char **array, struct MailTransferNode *entry)
{
  if(entry)
  {
    static char dispfro[SIZE_DEFAULT], dispsta[SIZE_DEFAULT], dispsiz[SIZE_SMALL], dispdate[64];
    struct Mail *mail = entry->mail;
    struct Person *pe = &mail->From;

    // status icon display
    snprintf(array[0] = dispsta, sizeof(dispsta), "%3d ", entry->index);
    if(hasTR_LOAD(entry))
      strlcat(dispsta, SI_STR(si_Download), sizeof(dispsta));

    if(hasTR_DELETE(entry))
      strlcat(dispsta, SI_STR(si_Delete), sizeof(dispsta));

    // size display
    if(C->WarnSize > 0 && mail->Size >= C->WarnSize*1024)
    {
      strlcpy(array[1] = dispsiz, MUIX_PH, sizeof(dispsiz));
      FormatSize(mail->Size, dispsiz+strlen(dispsiz), sizeof(dispsiz)-strlen(dispsiz), SF_AUTO);
    }
    else
      FormatSize(mail->Size, array[1] = dispsiz, sizeof(dispsiz), SF_AUTO);

    // from address display
    array[2] = dispfro;
    strlcpy(dispfro, AddrName((*pe)), sizeof(dispfro));

    // mail subject display
    array[3] = mail->Subject;

    // display date
    array[4] = dispdate;
    *dispdate = '\0';

    if(mail->Date.ds_Days)
      DateStamp2String(dispdate, sizeof(dispdate), &mail->Date, (C->DSListFormat == DSS_DATEBEAT || C->DSListFormat == DSS_RELDATEBEAT) ? DSS_DATEBEAT : DSS_DATETIME, TZC_LOCAL);
  }
  else
  {
    array[0] = (STRPTR)tr(MSG_MA_TitleStatus);
    array[1] = (STRPTR)tr(MSG_Size);
    array[2] = (STRPTR)tr(MSG_From);
    array[3] = (STRPTR)tr(MSG_Subject);
    array[4] = (STRPTR)tr(MSG_Date);
  }

  return 0;
}
MakeStaticHook(TR_LV_DspFuncHook,TR_LV_DspFunc);
///
/// TR_New
//  Creates transfer window
struct TR_ClassData *TR_New(enum TransferType TRmode)
{
  struct TR_ClassData *data;

  ENTER();

  if((data = calloc(1, sizeof(struct TR_ClassData))) != NULL)
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
    if(fullwin)
    {
      data->GUI.GR_LIST = VGroup, GroupFrameT(TRmode==TR_IMPORT ? tr(MSG_TR_MsgInFile) : tr(MSG_TR_MsgOnServer)),
         MUIA_ShowMe, TRmode==TR_IMPORT || C->PreSelection >= PSM_ALWAYS,
         Child, NListviewObject,
            MUIA_CycleChain, TRUE,
            MUIA_NListview_NList, data->GUI.LV_MAILS = TransferMailListObject,
               MUIA_NList_MultiSelect, MUIV_NList_MultiSelect_Default,
               MUIA_NList_Format        , "W=-1 BAR,W=-1 MACW=9 P=\33r BAR,MICW=20 BAR,MICW=16 BAR,MICW=9 MACW=15",
               MUIA_NList_DisplayHook   , &TR_LV_DspFuncHook,
               MUIA_NList_AutoVisible   , TRUE,
               MUIA_NList_Title         , TRUE,
               MUIA_NList_TitleSeparator, TRUE,
               MUIA_NList_DoubleClick   , TRUE,
               MUIA_NList_MinColSortable, 0,
               MUIA_Font, C->FixedFontList ? MUIV_NList_Font_Fixed : MUIV_NList_Font,
               MUIA_ContextMenu         , NULL,
               MUIA_NList_Exports, MUIV_NList_Exports_Cols,
               MUIA_NList_Imports, MUIV_NList_Imports_Cols,
               MUIA_ObjectID, MAKE_ID('N','L','0','4'),
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
      if(fullwin)
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
      free(data);
      data = NULL;
    }
  }

  RETURN(data);
  return data;
}
///

