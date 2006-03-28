/***************************************************************************

 YAM - Yet Another Mailer
 Copyright (C) 1995-2000 by Marcel Beck <mbeck@yam.ch>
 Copyright (C) 2000-2006 by YAM Open Source Team

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
#include <string.h>
#include <time.h>

#include <clib/alib_protos.h>
#include <clib/macros.h>
#include <libraries/iffparse.h>
#include <libraries/genesis.h>
#include <mui/NList_mcc.h>
#include <mui/NListview_mcc.h>
#include <proto/amissl.h>
#include <proto/dos.h>
#include <proto/exec.h>
#include <proto/genesis.h>
#include <proto/intuition.h>
#include <proto/bsdsocket.h>
#include <proto/miami.h>
#include <proto/muimaster.h>
#include <proto/timer.h>
#include <proto/utility.h>

#include "extra.h"
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

#include "classes/Classes.h"
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
   struct timeval Clock_Last;
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

static const char *SMTPcmd[] =
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

// ESMTP ServerFlags & macros
#define ESMTP_FLG_AUTH_CRAM_MD5       (1<<0)
#define ESMTP_FLG_AUTH_DIGEST_MD5     (1<<1)
#define ESMTP_FLG_AUTH_LOGIN          (1<<2)
#define ESMTP_FLG_AUTH_PLAIN          (1<<3)
#define ESMTP_FLG_STARTTLS            (1<<4)
#define ESMTP_FLG_SIZE                (1<<5)
#define ESMTP_FLG_PIPELINING          (1<<6)
#define ESMTP_FLG_8BITMIME            (1<<7)
#define ESMTP_FLG_DSN                 (1<<8)
#define ESMTP_FLG_ETRN                (1<<9)
#define ESMTP_FLG_ENHANCEDSTATUSCODES (1<<10)
#define ESMTP_FLG_DELIVERBY           (1<<11)
#define ESMTP_FLG_HELP                (1<<12)
#define hasCRAM_MD5_Auth(v)           (isFlagSet((v), ESMTP_FLG_AUTH_CRAM_MD5))
#define hasDIGEST_MD5_Auth(v)         (isFlagSet((v), ESMTP_FLG_AUTH_DIGEST_MD5))
#define hasLOGIN_Auth(v)              (isFlagSet((v), ESMTP_FLG_AUTH_LOGIN))
#define hasPLAIN_Auth(v)              (isFlagSet((v), ESMTP_FLG_AUTH_PLAIN))
#define hasSTARTTLS(v)                (isFlagSet((v), ESMTP_FLG_STARTTLS))
#define hasSIZE(v)                    (isFlagSet((v), ESMTP_FLG_SIZE))
#define hasPIPELINING(v)              (isFlagSet((v), ESMTP_FLG_PIPELINING))
#define has8BITMIME(v)                (isFlagSet((v), ESMTP_FLG_8BITMIME))
#define hasDSN(v)                     (isFlagSet((v), ESMTP_FLG_DSN))
#define hasETRN(v)                    (isFlagSet((v), ESMTP_FLG_ETRN))
#define hasENHANCEDSTATUSCODES(v)     (isFlagSet((v), ESMTP_FLG_ENHANCEDSTATUSCODES))
#define hasDELIVERBY(v)               (isFlagSet((v), ESMTP_FLG_DELIVERBY))
#define hasHELP(v)                    (isFlagSet((v), ESMTP_FLG_HELP))

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

static const char *POPcmd[] =
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
// static function prototypes
static void TR_NewMailAlert(void);
static void TR_CompleteMsgList(void);
static char *TR_SendPOP3Cmd(enum POPCommand command, char *parmtext, APTR errorMsg);
static char *TR_SendSMTPCmd(enum SMTPCommand command, char *parmtext, APTR errorMsg);
static int  TR_Recv(char *vptr, int maxlen);
static int  TR_RecvToFile(FILE *fh, char *filename, struct TransStat *ts);
static int  TR_ReadLine(LONG socket, char *vptr, int maxlen);
static int  TR_ReadBuffered(LONG socket, char *ptr, int maxlen, int flags);
static int  TR_Send(char *vptr, int len, int flags);
static int  TR_WriteBuffered(LONG socket, char *ptr, int maxlen, int flags);
static void TR_TransStat_Update(struct TransStat *ts, int size_incr);

#define TR_WriteLine(buf)       (TR_Send((buf), strlen(buf), TCPF_FLUSH))
#define TR_WriteFlush()         (TR_Send(NULL,  0, TCPF_FLUSHONLY))
#define TR_FreeTransBuffers()   { TR_ReadBuffered(0, NULL, 0, TCPF_FREEBUFFER);  \
                                  TR_WriteBuffered(0, NULL, 0, TCPF_FREEBUFFER); }

/**************************************************************************/
// TLS/SSL related variables

static SSL_METHOD *method;
static SSL_CTX *ctx;
static SSL *ssl;

/**************************************************************************/
// local macros & defines
#define GetLong(p,o)  ((((unsigned char*)(p))[o]) | (((unsigned char*)(p))[o+1]<<8) | (((unsigned char*)(p))[o+2]<<16) | (((unsigned char*)(p))[o+3]<<24))

/***************************************************************************
 Module: Transfer
***************************************************************************/

/*** TLS/SSL routines ***/
/// TR_InitTLS()
// Initialize the SSL/TLS session accordingly
BOOL TR_InitTLS(VOID)
{
  char tmp[256];
  char *CAfile = NULL;
  char *CApath = NULL;

  // lets initialize the library first and load the error strings
  SSL_library_init();
  SSL_load_error_strings();

  // We have to feed the random number generator first
  D(DBF_NET, "Seeding random number generator...");
  sprintf(tmp, "%lx%lx", (unsigned long)time((time_t *)0), (unsigned long)FindTask(NULL));
  RAND_seed(tmp, strlen(tmp));

  if (!(method = SSLv23_client_method()))
  {
    E(DBF_NET, "SSLv23_client_method() error !");
    return FALSE;
  }

  if (!(ctx = SSL_CTX_new(method)))
  {
    E(DBF_NET, "Can't create SSL_CTX object !");
    return FALSE;
  }

  // In future we can give the user the ability to specify his own CA locations
  // in the application instead of using the default ones.
  if (CAfile || CApath)
  {
    D(DBF_NET, "CAfile = %s, CApath = %s", CAfile ? CAfile : "none", CApath ? CApath : "none");
    if ((!SSL_CTX_load_verify_locations(ctx, CAfile, CApath)))
    {
      E(DBF_NET, "Error setting default verify locations !");
      return FALSE;
    }
  }
  else
  {
    if((!SSL_CTX_set_default_verify_paths(ctx)))
    {
      E(DBF_NET, "Error setting default verify locations !");
      return FALSE;
    }
  }

  if (!(SSL_CTX_set_cipher_list(ctx, "DEFAULT")))
  {
    E(DBF_NET, "SSL_CTX_set_cipher_list() error !");
    return FALSE;
  }

  SSL_CTX_set_verify(ctx, SSL_VERIFY_NONE, 0);

  return TRUE;
}

///
/// TR_StartTLS()
// function that starts & initializes the TLS/SSL session
BOOL TR_StartTLS(VOID)
{
  D(DBF_NET, "Initializing TLS/SSL session...");

  if (!(ssl = SSL_new(ctx)))
  {
    E(DBF_NET, "Can't create a new SSL structure for a connection !");
    return FALSE;
  }

  if (!(SSL_set_fd(ssl, (int)G->TR_Socket)))
  {
    E(DBF_NET, "SSL_set_fd() error !");
    return FALSE;
  }

  if ((SSL_connect(ssl)) <= 0)
  {
    E(DBF_NET, "TLS/SSL handshake error !");
    return FALSE;
  }

  // Certificate info
  // only for debug reasons
#ifdef DEBUG
  {
    char *x509buf;
    SSL_CIPHER *cipher;
    X509 *server_cert;
    cipher = SSL_get_current_cipher(ssl);

    if (cipher)
    {
      D(DBF_NET, "%s connection using %s", SSL_CIPHER_get_version(cipher), SSL_get_cipher(ssl));
    }

    if (!(server_cert = SSL_get_peer_certificate(ssl)))
    {
      E(DBF_NET, "SSL_get_peer_certificate() error !");
    }

    D(DBF_NET, "Server public key is %ld bits", EVP_PKEY_bits(X509_get_pubkey(server_cert)));

    #define X509BUFSIZE 4096

    x509buf = (char *)malloc(X509BUFSIZE);
    memset(x509buf, 0, X509BUFSIZE);

    D(DBF_NET, "Server certificate:");

    if(!(X509_NAME_oneline(X509_get_subject_name(server_cert), x509buf, X509BUFSIZE)))
    {
      E(DBF_NET, "X509_NAME_oneline...[subject] error !");
    }
    D(DBF_NET, "subject: %s", x509buf);

    if(!(X509_NAME_oneline(X509_get_issuer_name(server_cert), x509buf, X509BUFSIZE)))
    {
      E(DBF_NET, "X509_NAME_oneline...[issuer] error !");
    }
    D(DBF_NET, "issuer:  %s", x509buf);

    if(x509buf)     free(x509buf);
    if(server_cert) X509_free(server_cert);
  }
#endif

  return TRUE;
}

///
/// TR_EndTLS()
// function that stops all TLS context
VOID TR_EndTLS(VOID)
{
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
}

///
/// TR_InitSTARTTLS()
// function to initiate a TLS connection to the ESMTP server via STARTTLS
static BOOL TR_InitSTARTTLS(int ServerFlags)
{
  // If this server doesn`t support TLS at all we return with an error
  if(!hasSTARTTLS(ServerFlags))
  {
    ER_NewError(GetStr(MSG_ER_NOSTARTTLS));
    return FALSE;
  }

  // If we end up here the server supports STARTTLS and we can start
  // initializing the connection
  set(G->TR->GUI.TX_STATUS, MUIA_Text_Contents, GetStr(MSG_TR_INITTLS));

  // Now we initiate the STARTTLS command (RFC 2487)
  if(!TR_SendSMTPCmd(ESMTP_STARTTLS, NULL, MSG_ER_BadResponse)) return FALSE;

  if(TR_InitTLS() && TR_StartTLS())
  {
    G->TR_UseTLS = TRUE;
  }
  else
  {
    ER_NewError(GetStr(MSG_ER_INITTLS), C->SMTP_Server);
    return FALSE;
  }

  return TRUE;
}

///

/*** SMTP-AUTH routines ***/
/// TR_InitSMTPAUTH()
// function to authenticate to a ESMTP Server
static BOOL TR_InitSMTPAUTH(int ServerFlags)
{
   int rc = SMTP_SERVICE_NOT_AVAILABLE;
   char *resp;
   char buffer[SIZE_LINE];
   char challenge[SIZE_LINE];

   set(G->TR->GUI.TX_STATUS, MUIA_Text_Contents, GetStr(MSG_TR_SENDAUTH));

   // first we check if the user has supplied the User&Password
   // and if not we return with an error
   if(!C->SMTP_AUTH_User[0] || !C->SMTP_AUTH_Pass[0])
   {
      ER_NewError(GetStr(MSG_ER_NOAUTHUSERPASS));
      return FALSE;
   }

   if(hasDIGEST_MD5_Auth(ServerFlags)) // SMTP AUTH DIGEST-MD5 (RFC 2831)
   {
      D(DBF_NET, "processing AUTH DIGEST-MD5:");

      // send the AUTH command and get the response back
      if((resp = TR_SendSMTPCmd(ESMTP_AUTH_DIGEST_MD5, NULL, MSG_ER_BadResponse)))
      {
        char realm[SIZE_HOST];
        char nonce[SIZE_DEFAULT];
        char qop[SIZE_DEFAULT];
        char cnonce[SIZE_DEFAULT];
        char response[SIZE_DEFAULT];
        char *chalRet;

        // get the challenge code from the response line of the
        // AUTH command.
        strncpy(challenge, &resp[4], 511);
        challenge[511] = '\0';

        // now that we have the challange phrase we need to base64decode
        // it, but have to take care to remove the ending "\r\n" cookie.
        chalRet = strpbrk(challenge, "\r\n"); // find the first CR or LF
        if(chalRet)
          *chalRet = '\0'; // strip it

        D(DBF_NET, "received DIGEST-MD5 challenge: `%s`", challenge);

        // lets base64 decode it
        if(base64decode(challenge, (unsigned char *)challenge, strlen(challenge)) <= 0)
          return FALSE;

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
            memcpy(realm, pstart, (size_t)(pend-pstart));

            // NUL terminate the string
            realm[pend-pstart] = '\0';
          }
          else
          {
            // if the challenge don`t have a "realm" we assume our
            // choosen SMTP domain to be the realm
            strcpy(realm, C->SMTP_Domain);
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
            memcpy(nonce, pstart, (size_t)(pend-pstart));

            // NUL terminate the string
            nonce[pend-pstart] = '\0';
          }
          else
          {
            E(DBF_NET, "no `nonce=` token found!");
            return FALSE;
          }
          D(DBF_NET, "nonce: `%s`", nonce);

          // now we check the "qop" to carry "auth" so that we are
          // sure that this server really wants an authentification from us
          // RFC 2831 says that it is OK if no qop is present, because this
          // assumes the server to support at least "auth"
          if((pstart = strstr(challenge, "qop=")))
          {
            // iterate to the beginning of the nonce
            pstart += 4;

            // skip a leading "
            if(*pstart == '"')
            {
              pstart++;

              // find the end of the string
              pend = strpbrk(pstart, "\""); // find a ending "
              if(!pend)
                return FALSE; // no closing " - this seems to be an error!
            }
            else
            {
              // find the end of the string
              pend = strpbrk(pstart, "\""); // find a ending "
              if(!pend)
                pend = pstart + strlen(pstart);
            }

            // now copy the found qop into our nonce string
            memcpy(qop, pstart, (size_t)(pend-pstart));

            // NUL terminate the string
            qop[pend-pstart] = '\0';

            // then we check whether we have a plain "auth" within
            // qop or not
            pstart = qop;
            while((pstart = strstr(qop+(pstart-qop), "auth")))
            {
              if(*(pstart+1) != '-')
                break;
            }

            // check if we found a plain auth
            if(!pstart)
            {
              E(DBF_NET, "no `auth` in `qop` token found!");
              return FALSE;
            }
          }
        }

        // if we passed here, the server seems to at least support all
        // mechanisms we need for a proper DIGEST-MD5 authentication.
        // so it`s time for STEP TWO

        // let us now generate a more or less random and unique cnonce
        // identifier which we can supply to our SMTP server.
        sprintf(cnonce, "%08lx%08lx", (ULONG)rand(), (ULONG)rand());

        // the we generate the response according to RFC 2831 with A1
        // and A2 as MD5 encoded strings
        {
          unsigned char digest[16]; // 16 octets
          ULONG digest_hex[4];      // 16 octets
          struct MD5Context context;
          char buf[SIZE_DEFAULT];
          char buf2[SIZE_DEFAULT];
          char A1[SIZE_DEFAULT];
          int  A1_len = 16;         // 16 octects minimum
          char A2[SIZE_DEFAULT];

          // lets first generate the A1 string
          // A1 = { H( { username-value, ":", realm-value, ":", passwd } ),
          //      ":", nonce-value, ":", cnonce-value }
          sprintf(buf, "%s:%s:%s", C->SMTP_AUTH_User, realm, C->SMTP_AUTH_Pass);
          MD5Init(&context);
          MD5Update(&context, (unsigned char *)buf, strlen(buf));
          MD5Final(digest, &context);
          memcpy(A1, digest, 16);
          A1_len += sprintf(&A1[16], ":%s:%s", nonce, cnonce);
          D(DBF_NET, "unencoded A1: `%s` (%ld)", A1, A1_len);

          // then we directly build the hexadecimal representation
          // HEX(H(A1))
          MD5Init(&context);
          MD5Update(&context, (unsigned char *)A1, A1_len);
          MD5Final((UBYTE *)digest_hex, &context);
          sprintf(A1, "%08lx%08lx%08lx%08lx", digest_hex[0], digest_hex[1],
                                              digest_hex[2], digest_hex[3]);
          D(DBF_NET, "encoded   A1: `%s`", A1);


          // then we generate the A2 string accordingly
          // A2 = { "AUTHENTICATE:", digest-uri-value }
          sprintf(A2, "AUTHENTICATE:smtp/%s", realm);
          D(DBF_NET, "unencoded A2: `%s`", A2);

          // and also directly build the hexadecimal representation
          // HEX(H(A2))
          MD5Init(&context);
          MD5Update(&context, (unsigned char *)A2, strlen(A2));
          MD5Final((UBYTE *)digest_hex, &context);
          sprintf(A2, "%08lx%08lx%08lx%08lx", digest_hex[0], digest_hex[1],
                                              digest_hex[2], digest_hex[3]);
          D(DBF_NET, "encoded   A2: `%s`", A2);

          // now we build the string from which we also build the MD5
          // HEX(H(A1)), ":",
          // nonce-value, ":", nc-value, ":",
          // cnonce-value, ":", qop-value, ":", HEX(H(A2))
          sprintf(buf2, "%s:%s:00000001:%s:auth:%s", A1, nonce, cnonce, A2);
          D(DBF_NET, "unencoded resp: `%s`", buf2);

          // and finally build the respone-value =
          // HEX( KD( HEX(H(A1)), ":",
          //          nonce-value, ":", nc-value, ":",
          //          cnonce-value, ":", qop-value, ":", HEX(H(A2)) }))
          MD5Init(&context);
          MD5Update(&context, (unsigned char *)buf2, strlen(buf2));
          MD5Final((UBYTE *)digest_hex, &context);
          sprintf(response, "%08lx%08lx%08lx%08lx", digest_hex[0], digest_hex[1],
                                                    digest_hex[2], digest_hex[3]);
          D(DBF_NET, "encoded   resp: `%s`", response);
        }

        // form up the challenge to authenticate according to RFC 2831
        sprintf(challenge,
                "username=\"%s\","        // the username token
                "realm=\"%s\","           // the realm token
                "nonce=\"%s\","           // the nonce token
                "cnonce=\"%s\","          // the client nonce (cnonce)
                "nc=00000001,"            // the nonce count (here always 1)
                "qop=auth,"               // we just use auth
                "digest-uri=\"smtp/%s\"," // the digest-uri token
                "response=%s",            // the response
                C->SMTP_AUTH_User,
                realm,
                nonce,
                cnonce,
                realm,
                response);

        D(DBF_NET, "prepared challenge answer....: `%s`", challenge);
        base64encode(buffer, (unsigned char *)challenge, strlen(challenge));
        D(DBF_NET, "encoded  challenge answer....: `%s`", buffer);
        strcat(buffer,"\r\n");

        // now we send the SMTP AUTH response
        if(TR_WriteLine(buffer) <= 0) return FALSE;

        // get the server response and see if it was valid
        if(TR_ReadLine(G->TR_Socket, buffer, SIZE_LINE) <= 0 || (rc = getResponseCode(buffer)) != 334)
        {
          ER_NewError(GetStr(MSG_ER_BadResponse), (char *)SMTPcmd[ESMTP_AUTH_DIGEST_MD5], buffer);
        }
        else
        {
          // now that we have received the 334 code we just send a plain line
          // to signal that we don`t need any option
          if(TR_WriteLine("\r\n") <= 0) return FALSE;

          if(TR_ReadLine(G->TR_Socket, buffer, SIZE_LINE) <= 0 || (rc = getResponseCode(buffer)) != 235)
          {
            ER_NewError(GetStr(MSG_ER_BadResponse), (char *)SMTPcmd[ESMTP_AUTH_DIGEST_MD5], buffer);
          }
          else rc = SMTP_ACTION_OK;
        }
      }
   }
   else if(hasCRAM_MD5_Auth(ServerFlags)) // SMTP AUTH CRAM-MD5 (RFC 2195)
   {
      D(DBF_NET, "processing AUTH CRAM-MD5:");

      // send the AUTH command and get the response back
      if((resp = TR_SendSMTPCmd(ESMTP_AUTH_CRAM_MD5, NULL, MSG_ER_BadResponse)))
      {
        ULONG digest[4]; // 16 chars
        char buf[512];
        char *login = C->SMTP_AUTH_User;
        char *password = C->SMTP_AUTH_Pass;
        char *chalRet;

        // get the challenge code from the response line of the
        // AUTH command.
        strncpy(challenge, &resp[4], 511);
        challenge[511] = '\0';

        // now that we have the challange phrase we need to base64decode
        // it, but have to take care to remove the ending "\r\n" cookie.
        chalRet = strpbrk(challenge, "\r\n"); // find the first CR or LF
        if(chalRet)
          *chalRet = '\0'; // strip it

        D(DBF_NET, "received CRAM-MD5 challenge: `%s`", challenge);

        // lets base64 decode it
        if(base64decode(challenge, (unsigned char *)challenge, strlen(challenge)) <= 0)
          return FALSE;

        D(DBF_NET, "decoded  CRAM-MD5 challenge: `%s`", challenge);

        // compose the md5 challenge
        hmac_md5((unsigned char *)challenge, strlen(challenge), (unsigned char *)password, strlen(password), (unsigned char *)digest);
        sprintf(buf, "%s %08lx%08lx%08lx%08lx", login, digest[0], digest[1], digest[2], digest[3]);

        D(DBF_NET, "prepared CRAM-MD5 reponse..: `%s`", buf);
        // lets base64 encode the md5 challenge for the answer
        base64encode(buffer, (unsigned char *)buf, strlen(buf));
        D(DBF_NET, "encoded  CRAM-MD5 reponse..: `%s`", buffer);
        strcat(buffer, "\r\n");

        // now we send the SMTP AUTH response
        if(TR_WriteLine(buffer) <= 0) return FALSE;

        // get the server response and see if it was valid
        if(TR_ReadLine(G->TR_Socket, buffer, SIZE_LINE) <= 0 || (rc = getResponseCode(buffer)) != 235)
        {
          ER_NewError(GetStr(MSG_ER_BadResponse), (char *)SMTPcmd[ESMTP_AUTH_CRAM_MD5], buffer);
        }
        else rc = SMTP_ACTION_OK;
      }
   }
   else if(hasLOGIN_Auth(ServerFlags))  // SMTP AUTH LOGIN
   {
      D(DBF_NET, "processing AUTH LOGIN:");

      // send the AUTH command
      if((resp = TR_SendSMTPCmd(ESMTP_AUTH_LOGIN, NULL, MSG_ER_BadResponse)))
      {
         // prepare the username challenge
         D(DBF_NET, "prepared AUTH LOGIN challenge: `%s`", C->SMTP_AUTH_User);
         base64encode(buffer, (unsigned char *)C->SMTP_AUTH_User, strlen(C->SMTP_AUTH_User));
         D(DBF_NET, "encoded  AUTH LOGIN challenge: `%s`", buffer);
         strcat(buffer,"\r\n");

         // now we send the SMTP AUTH response (UserName)
         if(TR_WriteLine(buffer) <= 0) return FALSE;

         // get the server response and see if it was valid
         if(TR_ReadLine(G->TR_Socket, buffer, SIZE_LINE) > 0
            && (rc = getResponseCode(buffer)) == 334)
         {
            // prepare the password challenge
            D(DBF_NET, "prepared AUTH LOGIN challenge: `%s`", C->SMTP_AUTH_Pass);
            base64encode(buffer, (unsigned char *)C->SMTP_AUTH_Pass, strlen(C->SMTP_AUTH_Pass));
            D(DBF_NET, "encoded  AUTH LOGIN challenge: `%s`", buffer);
            strcat(buffer,"\r\n");

            // now lets send the Password
            if(TR_WriteLine(buffer) <= 0) return FALSE;

            // get the server response and see if it was valid
            if(TR_ReadLine(G->TR_Socket, buffer, SIZE_LINE) > 0
               && (rc = getResponseCode(buffer)) == 235)
            {
               rc = SMTP_ACTION_OK;
            }
         }

         if(rc != SMTP_ACTION_OK)
         {
            ER_NewError(GetStr(MSG_ER_BadResponse), (char *)SMTPcmd[ESMTP_AUTH_LOGIN], buffer);
         }
      }
   }
   else if(hasPLAIN_Auth(ServerFlags))  // SMTP AUTH PLAIN (RFC 2595)
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
      len += sprintf(challenge+len, "%s", C->SMTP_AUTH_User)+1; // authenticate-id
      len += sprintf(challenge+len, "%s", C->SMTP_AUTH_Pass);   // password

      // now we base64 encode this string and send it to the server
      base64encode(buffer, (unsigned char *)challenge, len);

      // lets now form up the AUTH PLAIN command we are going to send
      // to the SMTP server for authorization purposes:
      sprintf(challenge, "%s %s\r\n", SMTPcmd[ESMTP_AUTH_PLAIN], buffer);

      // now we send the SMTP AUTH command (UserName+Password)
      if(TR_WriteLine(challenge) <= 0) return FALSE;

      // get the server response and see if it was valid
      if(TR_ReadLine(G->TR_Socket, buffer, SIZE_LINE) <= 0 || (rc = getResponseCode(buffer)) != 235)
      {
        ER_NewError(GetStr(MSG_ER_BadResponse), (char *)SMTPcmd[ESMTP_AUTH_PLAIN], buffer);
      }
      else rc = SMTP_ACTION_OK;
   }
   else
   {
      // if we don`t have any of the Authentication Flags turned on we have to
      // exit with an error
      ER_NewError(GetStr(MSG_ER_NO_SMTP_AUTH), C->SMTP_Server);
   }

   D(DBF_NET, "Server responded with %ld", rc);

   return (BOOL)(rc == SMTP_ACTION_OK);
}
///

/*** General connecting/disconnecting & transfer ***/
/// TR_IsOnline
//  Checks if there's an online connection
BOOL TR_IsOnline(void)
{
   BOOL isonline = FALSE;

   if(C->IsOnlineCheck)
   {
      if((MiamiBase = OpenLibrary("miami.library", 10)))
      {
         if(GETINTERFACE("main", IMiami, MiamiBase))
         {
           isonline = MiamiIsOnline(*C->IOCInterface ? C->IOCInterface : NULL);

           DROPINTERFACE(IMiami);
         }

         CloseLibrary(MiamiBase);
         MiamiBase = NULL;
      }
      else if((GenesisBase = OpenLibrary("genesis.library", 1)))
      {
         if(GETINTERFACE("main", IGenesis, GenesisBase))
         {
           isonline = IsOnline(*C->IOCInterface ? (long)C->IOCInterface : 0);

           DROPINTERFACE(IGenesis);
         }

         CloseLibrary(GenesisBase);
         GenesisBase = NULL;
      }
   }

   if(isonline == FALSE)
   {
     // if no online check was selected, we just check if bsdsocket.library
     // is available.
     struct Library *socketbase = OpenLibrary("bsdsocket.library", 2L);
     if (socketbase)
     {
       CloseLibrary(socketbase);
       isonline = TRUE;
     }
   }

   return isonline;
}

///
/// TR_CloseTCPIP
//  Closes bsdsocket library
void TR_CloseTCPIP(void)
{
  if(AmiSSLBase)
  {
    CleanupAmiSSLA(NULL);
  }

  #if defined(__amigaos4__)
  if(ISocket)
  {
    DROPINTERFACE(ISocket);
  }
  #endif

  if(SocketBase)
  {
    CloseLibrary(SocketBase);
    SocketBase = NULL;
  }
}

///
/// TR_OpenTCPIP
//  Opens bsdsocket.library
BOOL TR_OpenTCPIP(void)
{
  // first do an online check
  if(!TR_IsOnline())
    return FALSE;

  // then open the bsdsocket.library and it`s OS4 interface
  if(!SocketBase)
  {
    if(!(SocketBase = OpenLibrary("bsdsocket.library", 2L)))
      return FALSE;

    if(!GETINTERFACE("main", ISocket, SocketBase))
    {
      CloseLibrary(SocketBase);
      SocketBase = NULL;

      return FALSE;
    }
  }

  // Now we have to check for TLS/SSL support
  if(G->TR_UseableTLS && AmiSSLBase && SocketBase)
  {
    #if defined(__amigaos4__)
    if(InitAmiSSL(AmiSSL_ISocket, ISocket,
                  TAG_DONE) != 0)
    #else
    if(InitAmiSSL(AmiSSL_SocketBase, SocketBase,
                  TAG_DONE) != 0)
    #endif
    {
      ER_NewError(GetStr(MSG_ER_INITAMISSL));
      G->TR_UseableTLS = G->TR_UseTLS = FALSE;
    }
  }
  else
    G->TR_UseTLS = FALSE;

  return (BOOL)(SocketBase != NULL);
}

///
/// TR_Disconnect
//  Terminates a connection
static void TR_Disconnect(void)
{
   if (G->TR_Socket != SMTP_NO_SOCKET)
   {
      if(G->TR_UseTLS)
      {
        TR_EndTLS();
        G->TR_UseTLS = FALSE;
      }

      shutdown(G->TR_Socket, 2);
      CloseSocket(G->TR_Socket);
      G->TR_Socket = SMTP_NO_SOCKET;

      // free the transfer buffers now
      TR_FreeTransBuffers();
   }
}
///
/// TR_Connect
//  Connects to a internet service
static int TR_Connect(char *host, int port)
{
  int i;
  LONG optval;
  struct hostent *hostaddr;

  // get the hostent out of the supplied hostname
  if(!(hostaddr = gethostbyname((STRPTR)host)))
  {
    return -1;
  }

  #ifdef DEBUG
  D(DBF_NET, "Host '%s':", host);
  D(DBF_NET, "  Officially:\t%s", hostaddr->h_name);

  for(i=0; hostaddr->h_aliases[i]; ++i)
  {
    D(DBF_NET, "  Alias:\t%s", hostaddr->h_aliases[i]);
  }

  D(DBF_NET, "  Type:\t\t%s", hostaddr->h_addrtype == AF_INET ? "AF_INET" : "AF_INET6");
  if(hostaddr->h_addrtype == AF_INET)
  {
    for(i=0; hostaddr->h_addr_list[i]; ++i)
    {
      D(DBF_NET, "  Address:\t%s", Inet_NtoA(((struct in_addr *)hostaddr->h_addr_list[i])->s_addr));
    }
  }
  #endif

  // lets create a standard AF_INET socket now
  G->TR_Socket = socket(AF_INET, SOCK_STREAM, 0);
  if (G->TR_Socket == -1)
  {
    return -2;
  }

  // lets set the socket options the user has defined
  // in the configuration
  if((optval = C->SocketOptions.KeepAlive))
  {
    if(setsockopt(G->TR_Socket, SOL_SOCKET, SO_KEEPALIVE, &optval, sizeof(optval)) == -1)
    {
      E(DBF_NET, "setsockopt(SO_KEEPALIVE) error");
      ER_NewError(GetStr(MSG_ER_SOCKETOPTION), "SO_KEEPALIVE");
    }
  }

  if((optval = C->SocketOptions.NoDelay))
  {
    if(setsockopt(G->TR_Socket, IPPROTO_TCP, TCP_NODELAY, &optval, sizeof(optval)) == -1)
    {
      E(DBF_NET, "setsockopt(TCP_NODELAY) error");
      ER_NewError(GetStr(MSG_ER_SOCKETOPTION), "TCP_NODELAY");
    }
  }

  if(C->SocketOptions.LowDelay)
  {
    optval = IPTOS_LOWDELAY;
    if(setsockopt(G->TR_Socket, IPPROTO_IP, IP_TOS, &optval, sizeof(optval)) == -1)
    {
      E(DBF_NET, "setsockopt(IPTOS_LOWDELAY) error");
      ER_NewError(GetStr(MSG_ER_SOCKETOPTION), "IPTOS_LOWDELAY");
    }
  }

  if((optval = C->SocketOptions.SendBuffer) > -1)
  {
    if(setsockopt(G->TR_Socket, SOL_SOCKET, SO_SNDBUF, &optval, sizeof(optval)) == -1)
    {
      E(DBF_NET, "setsockopt(SO_SNDBUF) error");
      ER_NewError(GetStr(MSG_ER_SOCKETOPTION), "SO_SNDBUF");
    }
  }

  if((optval = C->SocketOptions.RecvBuffer) > -1)
  {
    if(setsockopt(G->TR_Socket, SOL_SOCKET, SO_RCVBUF, &optval, sizeof(optval)) == -1)
    {
      E(DBF_NET, "setsockopt(SO_RCVBUF) error");
      ER_NewError(GetStr(MSG_ER_SOCKETOPTION), "SO_RCVBUF");
    }
  }

  if((optval = C->SocketOptions.SendLowAt) > -1)
  {
    if(setsockopt(G->TR_Socket, SOL_SOCKET, SO_SNDLOWAT, &optval, sizeof(optval)) == -1)
    {
      E(DBF_NET, "setsockopt(SO_SNDLOWAT) error");
      ER_NewError(GetStr(MSG_ER_SOCKETOPTION), "SO_SNDLOWAT");
    }
  }

  if((optval = C->SocketOptions.RecvLowAt) > -1)
  {
    if(setsockopt(G->TR_Socket, SOL_SOCKET, SO_RCVLOWAT, &optval, sizeof(optval)) == -1)
    {
      E(DBF_NET, "setsockopt(SO_RCVLOWAT) error");
      ER_NewError(GetStr(MSG_ER_SOCKETOPTION), "SO_RCVLOWAT");
    }
  }

  if(C->SocketOptions.SendTimeOut > -1)
  {
    struct timeval tv;

    #if defined(__NEWLIB__)
    tv.tv_sec  = C->SocketOptions.SendTimeOut;
    tv.tv_usec = 0;
    #else
    tv.tv_secs  = C->SocketOptions.SendTimeOut;
    tv.tv_micro = 0;
    #endif

    if(setsockopt(G->TR_Socket, SOL_SOCKET, SO_SNDTIMEO, &tv, sizeof(struct timeval)) == -1)
    {
      E(DBF_NET, "setsockopt(SO_SNDTIMEO) error");
      ER_NewError(GetStr(MSG_ER_SOCKETOPTION), "SO_SNDTIMEO");
    }
  }

  if(C->SocketOptions.RecvTimeOut > -1)
  {
    struct timeval tv;

    #if defined(__NEWLIB__)
    tv.tv_sec  = C->SocketOptions.RecvTimeOut;
    tv.tv_usec = 0;
    #else
    tv.tv_secs  = C->SocketOptions.RecvTimeOut;
    tv.tv_micro = 0;
    #endif

    if(setsockopt(G->TR_Socket, SOL_SOCKET, SO_RCVTIMEO, &tv, sizeof(struct timeval)) == -1)
    {
      E(DBF_NET, "setsockopt(SO_RCVTIMEO) error");
      ER_NewError(GetStr(MSG_ER_SOCKETOPTION), "SO_RCVTIMEO");
    }
  }

  // lets print out the current socket options
  #ifdef DEBUG
  {
    LONG optlen = sizeof(optval);
    struct timeval tv;
    LONG tvlen = sizeof(struct timeval);

    D(DBF_NET, "Opened socket: %lx", G->TR_Socket);

    getsockopt(G->TR_Socket, SOL_SOCKET, SO_KEEPALIVE, &optval, &optlen);
    D(DBF_NET, "SO_KEEPALIVE..: %ld", optval);

    getsockopt(G->TR_Socket, IPPROTO_TCP, TCP_NODELAY, &optval, &optlen);
    D(DBF_NET, "TCP_NODELAY...: %ld", optval);

    getsockopt(G->TR_Socket, IPPROTO_IP, IP_TOS, &optval, &optlen);
    D(DBF_NET, "IPTOS_LOWDELAY: %ld", hasFlag(optval, IPTOS_LOWDELAY));

    getsockopt(G->TR_Socket, SOL_SOCKET, SO_SNDBUF, &optval, &optlen);
    D(DBF_NET, "SO_SNDBUF.....: %ld bytes", optval);

    getsockopt(G->TR_Socket, SOL_SOCKET, SO_RCVBUF, &optval, &optlen);
    D(DBF_NET, "SO_RCVBUF.....: %ld bytes", optval);

    getsockopt(G->TR_Socket, SOL_SOCKET, SO_SNDLOWAT, &optval, &optlen);
    D(DBF_NET, "SO_SNDLOWAT...: %ld", optval);

    getsockopt(G->TR_Socket, SOL_SOCKET, SO_RCVLOWAT, &optval, &optlen);
    D(DBF_NET, "SO_RCVLOWAT...: %ld", optval);

    getsockopt(G->TR_Socket, SOL_SOCKET, SO_SNDTIMEO, &tv, &tvlen);
    #if defined(__NEWLIB__)
    D(DBF_NET, "SO_SNDTIMEO...: %ld", tv.tv_sec);
    #else
    D(DBF_NET, "SO_SNDTIMEO...: %ld", tv.tv_secs);
    #endif

    getsockopt(G->TR_Socket, SOL_SOCKET, SO_RCVTIMEO, &tv, &tvlen);
    #if defined(__NEWLIB__)
    D(DBF_NET, "SO_RCVTIMEO...: %ld", tv.tv_sec);
    #else
    D(DBF_NET, "SO_RCVTIMEO...: %ld", tv.tv_secs);
    #endif
  }
  #endif

  // copy the hostaddr data in a local copy for further reference
  memset(&G->TR_INetSocketAddr, 0, sizeof(G->TR_INetSocketAddr));
  G->TR_INetSocketAddr.sin_len    = sizeof(G->TR_INetSocketAddr);
  G->TR_INetSocketAddr.sin_family = AF_INET;
  G->TR_INetSocketAddr.sin_port   = htons(port);

  // now we try a connection for every address we have for this host
  // because a hostname can have more than one IP in h_addr_list[]
  for(i=0; hostaddr->h_addr_list[i]; i++)
  {
    memcpy(&G->TR_INetSocketAddr.sin_addr, hostaddr->h_addr_list[i], (size_t)hostaddr->h_length);

    if(connect(G->TR_Socket, (struct sockaddr *)&G->TR_INetSocketAddr, sizeof(G->TR_INetSocketAddr)) != -1)
    {
      // if all works and we finally established a connection we can return with zero.
      return 0;
    }

    // Preparation for non-blocking I/O
    if(Errno() == EINPROGRESS) return 0;
  }

  // if we end up here something went really wrong
  TR_Disconnect();
  return -3;
}
///
/// TR_Recv()
// a own wrapper function for Recv()/SSL_read() that reads buffered somehow
// it reads maxlen-1 data out of the buffer and stores it into recvdata with
// a null terminated string
static int TR_Recv(char *recvdata, int maxlen)
{
   int len;

   DoMethod(G->App,MUIM_Application_InputBuffered);
   if (G->TR_Socket == SMTP_NO_SOCKET) return 0;

   // we call the ReadBuffered function so that
   // we get out the data from our own buffer.
   len = TR_ReadBuffered(G->TR_Socket, recvdata, maxlen-1, TCPF_NONE);

   if (len <= 0) recvdata[0] = '\0';
   else recvdata[len] = '\0';

   if (G->TR_Debug) printf("SERVER[%04d]: %s", len, recvdata);

   return len;
}

///
/// TR_RecvToFile()
// function that receives data from a POP3 server until it receives a \r\n.\r\n termination
// line. It automatically writes that data to the supplied filehandle and if present also
// updates the Transfer status
static int TR_RecvToFile(FILE *fh, char *filename, struct TransStat *ts)
{
  int l=0, read, state=0, count;
  char buf[SIZE_LINE];
  char line[SIZE_LINE];
  char *bufptr;
  BOOL done=FALSE;

  // get the first data the pop server returns after the TOP command
  if((read = count = TR_Recv(buf, SIZE_LINE)) <= 0) G->Error = TRUE;

  while(!G->Error && !G->TR->Abort)
  {
    // now we iterate through the received string
    // and strip out the '\r' character.
    // we iterate through it because the strings we receive
    // from the socket can be splitted somehow.
    for(bufptr = buf; read > 0; bufptr++, read--)
    {
      // first we check if out buffer is full and if so we
      // write it to the file.
      if(l == SIZE_LINE || done == TRUE)
      {
        // update the transferstatus
        if(ts) TR_TransStat_Update(ts, l);

        // write the line to the file now
        if(fwrite(line, 1, l, fh) != (size_t)l)
        {
          ER_NewError(GetStr(MSG_ER_ErrorWriteMailfile), filename);
          break;
        }

        // if we end up here and done is true we have to break that iteration
        if(done) break;

        // set l to zero so that the next char gets written to the beginning
        l=0;
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
            line[l++] = '\r';
            read++;
            bufptr--;
            state = 0;

            continue;
          }

          // write the actual char "\n" in the line buffer
          line[l++] = *bufptr;

          state = 2; // we found a "\r\n" so we move to stat 2 on the next iteration.
          continue;
        }
        break;

        // stat==2 is only reached if we previously found a "\r\n"
        // stat==5 if we found a lonely "\n"
        case 2:
        case 5:
        {
          if(*bufptr == '.') { state++; continue; } // now it`s 3 or 6

          state = 0;
        }
        break;

        // stat==3 is only reached if we previously found a "\r\n."
        // stat==6 if we found a lonely "\n."
        case 3:
        case 6:
        {
          if(state == 3 && *bufptr == '\r') state = 4; // we found a \r directly after a "\r\n.", so it can be the start of a TERM
          else if(*bufptr == '.')
          {
            // (RFC 1939) - the server handles "." as "..", so we only write "."
            line[l++] = '.';
            state = 0;
          }
          else
          {
            // write the actual char in the line buffer
            line[l++] = '.';
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
            line[l++] = '.';
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
      if(*bufptr == '\r')       { state=1; continue; }
      else if(*bufptr == '\n')    state=5;

      // write the actual char in the line buffer
      line[l++] = *bufptr;
    }

    // if we received the term octet we can exit the while loop now
    if(done || G->Error || G->TR->Abort) break;

    // if not, we get another bunch of data and start over again.
    if((read = TR_Recv(buf, SIZE_LINE)) <= 0) break;
    count += read;
  }

  if(done) return count;
  else     return 0;
}

///
/// TR_Send()
// a own wrapper function for Send()/SSL_write() that writes buffered somehow
// if called with flag != TCP_FLUSH - otherwise it will write and flush immediatly
static int TR_Send(char *ptr, int len, int flags)
{
  int nwritten;

  DoMethod(G->App,MUIM_Application_InputBuffered);
  if (G->TR_Socket == SMTP_NO_SOCKET) return -1;
  if (G->TR_Debug && ptr) printf("CLIENT[%04d]: %s", len, ptr);

  // we call the WriteBuffered() function to write this characters
  // out to the socket. the provided flag will define if it
  // really will be buffered or if we write and flush the buffer
  // immediatly
  nwritten = TR_WriteBuffered(G->TR_Socket, ptr, len, flags);

  return nwritten;
}

///
/// TR_SetWinTitle
//  Sets the title of the transfer window
void TR_SetWinTitle(BOOL from, char *host)
{
   sprintf(G->TR->WTitle, GetStr(from ? MSG_TR_MailTransferFrom : MSG_TR_MailTransferTo), host);
   set(G->TR->GUI.WI, MUIA_Window_Title, G->TR->WTitle);
}
///
/// TR_ReadLine()
// a buffered version of readline() that reads out charwise from the
// socket via TR_ReadBuffered and returns the amount of chars copied
// into the provided character array.
//
// This implementation is a slightly adapted version of readline()/my_read()
// examples from (W.Richard Stevens - Unix Network Programming) - Page 80
static int TR_ReadLine(LONG socket, char *vptr, int maxlen)
{
  int n, rc;
  char c, *ptr;

  DoMethod(G->App,MUIM_Application_InputBuffered);
  if (G->TR_Socket == SMTP_NO_SOCKET) return FALSE;

  ptr = vptr;
  for(n = 1; n < maxlen; n++)
  {
    // read out one buffered char only.
    if((rc = TR_ReadBuffered(socket, &c, 1, TCPF_NONE)) == 1)
    {
      *ptr++ = c;
      if(c == '\n') break;  // newline is stored, like fgets()
    }
    else if(rc == 0)
    {
      if(n == 1) return 0;  // EOF, no data read
      else       break;     // EOF, some data was read
    }
    else return -1;         // error, errno set by readchar()
  }

  *ptr = 0;                 // null terminate like fgets()

  if(G->TR_Debug) printf("SERVER[%04d]: %s", n, vptr);

  return n; // return the number of chars we read
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
static int TR_ReadBuffered(LONG socket, char *ptr, int maxlen, int flags)
{
  static int read_cnt = 0;
  static char *read_buf = NULL;
  static char *read_ptr;
  int fill_cnt;

  // if we don`t have a buffer yet, lets allocate own
  if(!read_buf && !(read_buf = calloc(1, C->TRBufferSize*sizeof(char)))) return -1;

  // if we called that function with the FREEBUFFER flag we free the buffer only
  // and return with 0
  if(hasTCP_FREEBUFFER(flags))
  {
    free(read_buf);
    read_buf = NULL;
    read_cnt = 0;
    return 0;
  }

  // if the buffer is empty we fill
  // it again
  if(read_cnt <= 0)
  {
    again:

      if(G->TR_UseTLS) read_cnt = SSL_read(ssl, read_buf, C->TRBufferSize);
      else             read_cnt = recv(socket, read_buf, (LONG)C->TRBufferSize, 0);

      if(read_cnt < 0)
      {
        if(Errno() == EINTR) goto again;
        return -1;
      }
      else if(read_cnt == 0) return 0;

      read_ptr = read_buf;
  }

  // we copy only the minumu of read_cnt and maxlen
  fill_cnt = MIN(read_cnt, maxlen);

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

  return fill_cnt;
}

///
/// TR_WriteBuffered()
// a buffered implementation of write()/send(). This function will buffer
// all write operations in a temporary buffer and as soon as the buffer is
// full or it will be explicitly flushed, it will write out the data to the
// socket. This function SHOULD only be called by TR_Send() somehow.
//
// This function will also optimize the write operations by always making
// sure that a full buffer will be written out to the socket. i.e. if the
// buffer is filled up so that the next call would flush it, we copy as
// many data to the buffer as possible and flush it immediatly.
static int TR_WriteBuffered(UNUSED LONG socket, char *ptr, int maxlen, int flags)
{
  static int write_cnt = 0;
  static char *write_buf = NULL;
  static char *write_ptr;
  int fill_cnt = 0;
  int nwritten;

  // if we don`t have a buffer yet, lets allocate own
  if(!write_buf)
  {
    if(!(write_buf = calloc(1, C->TRBufferSize*sizeof(char))))
      return -1;
    write_ptr = write_buf;
  }

  // if we called that function with the FREEBUFFER flag we free the buffer only
  // and return with 0
  if(hasTCP_FREEBUFFER(flags))
  {
    free(write_buf);
    write_buf = NULL;
    write_cnt = 0;
    return 0;
  }

  // if we haven`t enough space in our buffer,
  // lets flush it first
  if(write_cnt >= C->TRBufferSize || hasTCP_ONLYFLUSH(flags))
  {
    again:

    write_ptr = write_buf;

    // lets make the buffer empty
    while(write_cnt > 0)
    {
      if(G->TR_UseTLS)  nwritten = SSL_write(ssl, write_ptr, write_cnt);
      else              nwritten = send(G->TR_Socket, write_ptr, (LONG)write_cnt, 0);

      if(nwritten <= 0)
      {
        if(Errno() == EINTR) continue; // and call write() again
        else return -1;
      }

      write_cnt -= nwritten;
      write_ptr += nwritten;
    }

    write_ptr = write_buf;

    // if we only flushed this buffered we can return immediatly
    if(hasTCP_ONLYFLUSH(flags)) return 0;
  }

  // if the string we want to copy into the buffer
  // wouldn`t fit, we copy as many as we can and clear the buffer
  if(write_cnt+maxlen > C->TRBufferSize)
  {
    int fillable = C->TRBufferSize-write_cnt;

    memcpy(write_ptr, ptr, fillable);

    // after the copy we have to increase the pointer of the
    // array we want to copy, because in the next cycle we have to
    // copy the rest out of it.
    ptr += fillable;
    write_cnt += fillable;
    fill_cnt += fillable;

    // decrease maxlen now by the amount of bytes we have written
    // to the buffer
    maxlen -= fillable;

    goto again; // I also don`t like goto, but sometimes it`s better. :)
  }

  // if we end up here we have enough space for our
  // string in the buffer, so lets copy it in
  memcpy(write_ptr, ptr, maxlen);
  write_ptr += maxlen;
  write_cnt += maxlen;
  fill_cnt += maxlen;

  // if the user has supplied the FLUSH flag we have to clear
  // the buffer now
  if(hasTCP_FLUSH(flags))
  {
    write_ptr = write_buf;

    // lets make the buffer empty
    while(write_cnt > 0)
    {
      if(G->TR_UseTLS)  nwritten = SSL_write(ssl, write_ptr, write_cnt);
      else              nwritten = send(G->TR_Socket, write_ptr, (LONG)write_cnt, 0);

      if(nwritten <= 0)
      {
        if(Errno() == EINTR) continue; // and call write() again
        else return -1;
      }

      write_cnt -= nwritten;
      write_ptr += nwritten;
    }

    write_ptr = write_buf;
  }

  return fill_cnt;  // return the amount of data written (should normally always be the original maxlen)
}

///

/*** HTTP routines ***/
/// TR_DownloadURL
//  Downloads a file from the web using HTTP
BOOL TR_DownloadURL(char *url0, char *url1, char *url2, char *filename)
{
   BOOL success = FALSE, done = FALSE, noproxy = !*C->ProxyServer;
   int l = 0, len, hport;
   char buf[SIZE_LINE], url[SIZE_URL], host[SIZE_HOST], *port, *path, line[SIZE_DEFAULT], *bufptr;
   FILE *out;

   G->Error = FALSE;
   if (!strnicmp(url0,"http://",7)) strcpy(url, &url0[7]); else strcpy(url, url0);
   if (url1)
   {
      if (url[strlen(url)-1] != '/') strcat(url, "/");
      strcat(url, url1);
   }
   if (url2)
   {
      if (url[strlen(url)-1] != '/') strcat(url, "/");
      strcat(url, url2);
   }
   if ((path = strchr(url,'/'))) *path++ = 0; else path = "";
   strcpy(host, noproxy ? url : C->ProxyServer);
   if ((bufptr = strchr(host, ':'))) { *bufptr++ = 0; hport = atoi(bufptr); }
   else hport = noproxy ? 80 : 8080;
   if (!TR_Connect(host, hport))
   {
/*
      if (noproxy) sprintf(buf, "GET /%s HTTP/1.0\r\nHost: http://%s\r\n", path, host);
      else if (port = strchr(url, ':'))
      {
         *port++ = 0;
         sprintf(buf, "GET http://%s:%s/%s HTTP/1.0\r\nHost: http://%s\r\n", url, port, path, url);
      }
      else sprintf(buf, "GET http://%s/%s HTTP/1.0\r\nHost: http://%s\r\n", url, path, url);
      sprintf(&buf[strlen(buf)], "From: %s\r\nUser-Agent: %s\r\n\r\n", BuildAddrName(C->EmailAddress, C->RealName), yamversion);
*/
      if (noproxy) sprintf(buf, "GET /%s HTTP/1.0\r\nHost: %s\r\n", path, host);
      else if ((port = strchr(url, ':')))
      {
         *port++ = 0;
         sprintf(buf, "GET http://%s:%s/%s HTTP/1.0\r\nHost: %s\r\n", url, port, path, url);
      }
      else sprintf(buf, "GET http://%s/%s HTTP/1.0\r\nHost: %s\r\n", url, path, url);
      sprintf(&buf[strlen(buf)], "From: %s\r\nUser-Agent: %s\r\n\r\n", BuildAddrName(C->EmailAddress, C->RealName), yamversion);
      if(TR_WriteLine(buf) > 0)
      {
         len = TR_Recv(buf, SIZE_LINE);
         if (atoi(&buf[9]) == 200)
         {
            if ((bufptr = strstr(buf, "\r\n"))) bufptr += 2;
            while (!G->Error)
            {
               for (; *bufptr; bufptr++)
               {
                  if (*bufptr != '\r') if (l < SIZE_DEFAULT-1) line[l++] = *bufptr;
                  if (*bufptr != '\n') continue;
                  line[l] = 0; l = 0;
                  if (line[0] == '\n') { done = TRUE; break; }
               }
               if (done) break;
               if ((len = TR_Recv(buf, SIZE_LINE)) <= 0) break;
               bufptr = buf;
            }
            if ((out = fopen(filename, "w")))
            {
               ++bufptr;
               fwrite(bufptr, (size_t)(len-(bufptr-buf)), 1, out);
               while ((len = TR_Recv(buf, SIZE_LINE)) > 0) fwrite(buf, len, 1, out);
               fclose(out);
               success = TRUE;
            }
            else ER_NewError(GetStr(MSG_ER_CantCreateFile), filename);
         }
         else ER_NewError(GetStr(MSG_ER_DocNotFound), path);
      }
      else ER_NewError(GetStr(MSG_ER_SendHTTP));
      TR_Disconnect();
   }
   else ER_NewError(GetStr(MSG_ER_ConnectHTTP), host);
   return success;
}
///

/*** POP3 routines ***/
/// TR_SendPOP3Cmd
//  Sends a command to the POP3 server
static char *TR_SendPOP3Cmd(enum POPCommand command, char *parmtext, APTR errorMsg)
{
   static char *buf = NULL;

   // first we check if the socket is in a valid state to proceed
   if (G->TR_Socket == SMTP_NO_SOCKET) return NULL;

   // if we are here for the first time lets generate a minimum buffer
   if(buf == NULL)
   {
      // by lookin at the RFC a buffer of 1000 chars for one line
      // should really be enough
      if(!(buf = AllocStrBuf(SIZE_LINE))) return NULL;
   }

   // if we specified a parameter for the pop command lets add it now
   if(!parmtext || !*parmtext) sprintf(buf, "%s\r\n", POPcmd[command]);
   else sprintf(buf, "%s %s\r\n", POPcmd[command], parmtext);

   // send the pop command to the server and see if it was received somehow
   // and for a connect we don`t send something or the server will get
   // confused.
   if(command != POPCMD_CONNECT && TR_WriteLine(buf) <= 0) return NULL;

   // let us read the next line from the server and check if
   // some status message can be retrieved.
   if(TR_ReadLine(G->TR_Socket, buf, SIZE_LINE) <= 0 ||
      strncmp(buf, POP_RESP_OKAY, strlen(POP_RESP_OKAY)) != 0)
   {
      // only report a error if wished
      if(errorMsg) ER_NewError(GetStr(errorMsg), (char *)POPcmd[command], buf);
      FreeStrBuf(buf);
      buf = NULL;

      return NULL;
   }

   return buf;
}
///
/// TR_ConnectPOP
//  Connects to a POP3 mail server
static int TR_ConnectPOP(int guilevel)
{
   char passwd[SIZE_PASSWORD], host[SIZE_HOST], buf[SIZE_LINE], *p;
   char *welcomemsg = NULL;
   int err, pop = G->TR->POP_Nr, msgs;
   int port = C->P3[pop]->Port;
   char *resp;

   strcpy(passwd, C->P3[pop]->Password);
   strcpy(host, C->P3[pop]->Server);

   // now we have to check whether SSL/TLS is selected for that POP account,
   // but perhaps TLS is not working.
   if(C->P3[pop]->SSLMode != P3SSL_OFF && !G->TR_UseableTLS)
   {
      ER_NewError(GetStr(MSG_ER_UNUSABLEAMISSL));
      return -1;
   }

   if (C->TransferWindow == 2 || (C->TransferWindow == 1 && (guilevel == POP_START || guilevel == POP_USER)))
   {
      // avoid MUIA_Window_Open's side effect of activating the window if it was already open
      if(!xget(G->TR->GUI.WI, MUIA_Window_Open)) set(G->TR->GUI.WI, MUIA_Window_Open, TRUE);
   }
   set(G->TR->GUI.TX_STATUS  , MUIA_Text_Contents,GetStr(MSG_TR_Connecting));

   // If the hostname has a explicit :xxxxx port statement at the end we
   // take this one, even if its not needed anymore.
   if ((p = strchr(host, ':'))) { *p = 0; port = atoi(++p); }

   BusyText(GetStr(MSG_TR_MailTransferFrom), host);
   TR_SetWinTitle(TRUE, host);

   if ((err = TR_Connect(host, port)))
   {
      if (guilevel == POP_USER) switch (err)
      {
         case -1: ER_NewError(GetStr(MSG_ER_UnknownPOP), C->P3[pop]->Server); break;
         default: ER_NewError(GetStr(MSG_ER_CantConnect), C->P3[pop]->Server);
      }
      return -1;
   }

   // If this connection should be a STLS like connection we have to get the welcome
   // message now and then send the STLS command to start TLS negotiation
   if(C->P3[pop]->SSLMode == P3SSL_STLS)
   {
      set(G->TR->GUI.TX_STATUS, MUIA_Text_Contents, GetStr(MSG_TR_WaitWelcome));

      // Initiate a connect and see if we succeed
      if(!(resp = TR_SendPOP3Cmd(POPCMD_CONNECT, NULL, MSG_ER_POPWELCOME))) return -1;
      welcomemsg = StrBufCpy(NULL, resp);

      // If the user selected STLS support we have to first send the command
      // to start TLS negotiation (RFC 2595)
      if(!TR_SendPOP3Cmd(POPCMD_STLS, NULL, MSG_ER_BadResponse)) return -1;
   }

   // Here start the TLS/SSL Connection stuff
   if(C->P3[pop]->SSLMode != P3SSL_OFF)
   {
     set(G->TR->GUI.TX_STATUS, MUIA_Text_Contents, GetStr(MSG_TR_INITTLS));

     // Now we have to Initialize and Start the TLS stuff if requested
     if(TR_InitTLS() && TR_StartTLS())
     {
        G->TR_UseTLS = TRUE;
     }
     else
     {
        ER_NewError(GetStr(MSG_ER_INITTLS), host);
        return -1;
     }
   }

   // If this was a connection on a stunnel on port 995 or a non-ssl connection
   // we have to get the welcome message now
   if(C->P3[pop]->SSLMode != P3SSL_STLS)
   {
      // Initiate a connect and see if we succeed
      if(!(resp = TR_SendPOP3Cmd(POPCMD_CONNECT, NULL, MSG_ER_POPWELCOME))) return -1;
      welcomemsg = StrBufCpy(NULL, resp);
   }

   if (!*passwd)
   {
      sprintf(buf, GetStr(MSG_TR_PopLoginReq), C->P3[pop]->User, host);
      if (!StringRequest(passwd, SIZE_PASSWORD, GetStr(MSG_TR_PopLogin), buf, GetStr(MSG_Okay), NULL, GetStr(MSG_Cancel), TRUE, G->TR->GUI.WI))
      {
        return -1;
      }
   }

   // if the user has selected APOP for that POP3 host
   // we have to process it now
   if (C->P3[pop]->UseAPOP)
   {
      struct MD5Context context;
      UBYTE digest[16];
      int i, j;

      // Now we get the APOP Identifier out of the welcome
      // message
      if((p = strchr(welcomemsg, '<')))
      {
         strcpy(buf, p);
         if ((p = strchr(buf, '>'))) p[1] = 0;

         // then we send the APOP command to authenticate via APOP
         strcat(buf, passwd);
         MD5Init(&context);
         MD5Update(&context, (unsigned char *)buf, strlen(buf));
         MD5Final(digest, &context);
         sprintf(buf, "%s ", C->P3[pop]->User);
         for(j=strlen(buf), i=0; i<16; j+=2, i++) sprintf(&buf[j], "%02x", digest[i]);
         buf[j] = 0;
         set(G->TR->GUI.TX_STATUS, MUIA_Text_Contents, GetStr(MSG_TR_SendAPOPLogin));
         if (!TR_SendPOP3Cmd(POPCMD_APOP, buf, MSG_ER_BadResponse)) return -1;
      }
      else
      {
         ER_NewError(GetStr(MSG_ER_NoAPOP));
         return -1;
      }
   }
   else
   {
      set(G->TR->GUI.TX_STATUS, MUIA_Text_Contents, GetStr(MSG_TR_SendUserID));
      if (!TR_SendPOP3Cmd(POPCMD_USER, C->P3[pop]->User, MSG_ER_BadResponse)) return -1;
      set(G->TR->GUI.TX_STATUS, MUIA_Text_Contents, GetStr(MSG_TR_SendPassword));
      if (!TR_SendPOP3Cmd(POPCMD_PASS, passwd, MSG_ER_BadResponse)) return -1;
   }

   FreeStrBuf(welcomemsg);

   set(G->TR->GUI.TX_STATUS, MUIA_Text_Contents, GetStr(MSG_TR_GetStats));
   if (!(resp = TR_SendPOP3Cmd(POPCMD_STAT, NULL, MSG_ER_BadResponse))) return -1;
   sscanf(&resp[4], "%d", &msgs);
   if (msgs) AppendLogVerbose(31, GetStr(MSG_LOG_ConnectPOP), C->P3[pop]->User, host, (void *)msgs, "");

   return msgs;
}
///
/// TR_DisplayMailList
//  Displays a list of messages ready for download
static void TR_DisplayMailList(BOOL largeonly)
{
  Object *lv = G->TR->GUI.LV_MAILS;

  if(IsMinListEmpty(&G->TR->transferList) == FALSE)
  {
    // search through our transferList
    struct MinNode *curNode;
    int pos=0;

    set(lv, MUIA_NList_Quiet, TRUE);
    for(curNode = G->TR->transferList.mlh_Head; curNode->mln_Succ; curNode = curNode->mln_Succ)
    {
      struct MailTransferNode *mtn = (struct MailTransferNode *)curNode;
      struct Mail *mail = mtn->mail;

      if(mail->Size >= C->WarnSize<<10 || !largeonly)
      {
        mtn->position = pos++;

        DoMethod(lv, MUIM_NList_InsertSingle, mtn, MUIV_NList_Insert_Bottom);
      }
    }
    set(lv, MUIA_NList_Quiet, FALSE);
  }
}
///
/// TR_GetMessageList_GET
//  Collects messages waiting on a POP3 server
static BOOL TR_GetMessageList_GET(void)
{
   // we issue a LIST command without argument to get a list
   // of all messages available on the server. This command will
   // return TRUE if the server responsed with a +OK
   if(TR_SendPOP3Cmd(POPCMD_LIST, NULL, MSG_ER_BadResponse))
   {
      char buf[SIZE_LINE];

      NewList((struct List *)&G->TR->transferList);

      // get the first line the pop server returns after the LINE command
      if(TR_ReadLine(G->TR_Socket, buf, SIZE_LINE) <= 0) return FALSE;

      // we get the "scan listing" as long as we haven`t received a a
      // finishing octet
      while(!G->Error && strncmp(buf, ".\r\n", 3) != 0)
      {
         int index, size;
         struct Mail *newMail;

         // read the index and size of the first message
         sscanf(buf, "%d %d", &index, &size);

         if(index > 0 && (newMail = calloc(1, sizeof(struct Mail))))
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
                   ((C->WarnSize && newMail->Size >= (C->WarnSize<<10)) ? 8 : 0);

            // allocate a new MailTransferNode and add it to our
            // new transferlist
            if((mtn = calloc(1, sizeof(struct MailTransferNode))))
            {
              mtn->mail = newMail;
              mtn->tflags = mode2tflags[mode];
              mtn->index = index;

              AddTail((struct List *)&G->TR->transferList, (struct Node *)mtn);
            }
         }

         // now read the next Line
         if(TR_ReadLine(G->TR_Socket, buf, SIZE_LINE) <= 0)
           return FALSE;
      }

      return TRUE;
   }
   else
     return FALSE;
}
///
/// TR_AppendUIDL
//  Appends a UIDL to the .uidl file
static void TR_AppendUIDL(char *uidl)
{
   FILE *fh;
   if ((fh = fopen(CreateFilename(".uidl"), "a")))
   {
      fprintf(fh, "%s\n", uidl);
      fclose(fh);
   }
}
///
/// TR_FindUIDL
//  Searches UIDL list for a given UIDL
static BOOL TR_FindUIDL(char *uidl)
{
   int l = strlen(uidl);
   char *p = G->TR->UIDLloc;
   if (p) while (*p)
   {
      if (!strncmp(p, uidl, l)) return TRUE;
      while (*p) if (*p++ == '\n') break;
   }
   return FALSE;
}
///
/// TR_GetUIDLonDisk
//  Loads local UIDL list from disk
static char *TR_GetUIDLonDisk(void)
{
   FILE *fh;
   char *text = NULL, *file = CreateFilename(".uidl");
   int size;

   if ((size = FileSize(file)) > 0)
      if ((text = calloc(size+1,1)))
         if ((fh = fopen(file, "r")))
         {
            fread(text, 1, size, fh);
            fclose(fh);
         }
   return text;
}
///
/// TR_GetUIDLonServer
//  Gets remote UIDL list from the POP3 server
static BOOL TR_GetUIDLonServer(void)
{
   // issue the UDIL pop command to get a unique-id list
   // of all mail avaiable at a server
   //
   // This command is optional within the RFC 1939 specification
   // and therefore we don`t throw any error and just return
   // with FALSE to signal that this server doesn`t support UIDL
   if(TR_SendPOP3Cmd(POPCMD_UIDL, NULL, NULL))
   {
      char buf[SIZE_LINE];

      // get the first line the pop server returns after the UIDL command
      if(TR_ReadLine(G->TR_Socket, buf, SIZE_LINE) <= 0) return FALSE;

      // we get the "unique-id list" as long as we haven`t received a a
      // finishing octet
      while(!G->Error && strncmp(buf, ".\r\n", 3) != 0)
      {
         int num;
         char uidl[SIZE_DEFAULT+SIZE_HOST];

         // now parse the line and get the message number and UIDL
         sscanf(buf, "%d %s", &num, uidl);

         // lets add our own ident to the uidl so that we can compare
         // it against our saved list
         strcat(uidl, "@");
         strcat(uidl, C->P3[G->TR->POP_Nr]->Server);

         // iterate to the mail the UIDL is for and save it in the variable
         if(IsMinListEmpty(&G->TR->transferList) == FALSE)
         {
            // search through our transferList
            struct MinNode *curNode;
            for(curNode = G->TR->transferList.mlh_Head; curNode->mln_Succ; curNode = curNode->mln_Succ)
            {
              struct MailTransferNode *mtn = (struct MailTransferNode *)curNode;

              if(mtn->index == num)
              {
                mtn->mail->UIDL = AllocCopy(uidl, strlen(uidl)+1);
                break;
              }
            }
         }

         // now read the next Line
         if(TR_ReadLine(G->TR_Socket, buf, SIZE_LINE) <= 0) return FALSE;
      }

      return TRUE;
   }
   else return FALSE;
}
///
/// TR_ApplyRemoteFilters
//  Applies remote filters to a message
static void TR_ApplyRemoteFilters(struct MailTransferNode *mtn)
{
  struct MinNode *curNode;

  // if there is no search count we can break out immediatly
  if(G->TR->SearchCount <= 0)
    return;

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

      return;
    }
  }
}
///
/// TR_GetMessageDetails
//  Gets header from a message stored on the POP3 server
static void TR_GetMessageDetails(struct MailTransferNode *mtn, int lline)
{
   struct Mail *mail = mtn->mail;

   if(!*mail->From.Address && !G->TR->Abort && !G->Error)
   {
      char cmdbuf[SIZE_SMALL];

      // we issue a TOP command with a one line message body.
      //
      // This command is optional within the RFC 1939 specification
      // and therefore we don`t throw any error
      sprintf(cmdbuf, "%d 1", mtn->index);
      if(TR_SendPOP3Cmd(POPCMD_TOP, cmdbuf, NULL))
      {
         char *tfname = "yamTOP.msg";
         char fname[SIZE_PATHFILE];
         FILE *f;

         // we generate a temporary file to buffer the TOP list
         // into it.
         strmfp(fname, C->TempDir, tfname);
         if((f = fopen(fname, "w")))
         {
            struct ExtendedMail *email;
            BOOL done = FALSE;

            // now we call a subfunction to receive data from the POP3 server
            // and write it in the filehandle as long as there is no termination \r\n.\r\n
            if(TR_RecvToFile(f, fname, NULL) > 0) done = TRUE;

            // close the filehandle now.
            fclose(f);

            // If we end up here because of an error, abort or the upper loop wasn`t finished
            // we exit immediatly with deleting the temp file also.
            if(G->Error || G->TR->Abort || done == FALSE)
            {
               DeleteFile(fname);
               return;
            }

            if((email = MA_ExamineMail(NULL, tfname, TRUE)))
            {
               mail->From    = email->Mail.From;
               mail->To      = email->Mail.To;
               mail->ReplyTo = email->Mail.ReplyTo;
               strcpy(mail->Subject, email->Mail.Subject);
               strcpy(mail->MailFile, email->Mail.MailFile);
               mail->Date = email->Mail.Date;

               // if thie function was called with -1, then the POP3 server
               // doesn`t have the UIDL command and we have to generate our
               // own one by using the MsgID and the Serverstring for the POP3
               // server.
               if(lline == -1)
               {
                  char uidl[SIZE_DEFAULT+SIZE_HOST];
                  sprintf(uidl, "%s@%s", Trim(email->MsgID), C->P3[G->TR->POP_Nr]->Server);
                  mail->UIDL = AllocCopy(uidl, strlen(uidl)+1);
               }
               else if(lline == -2)
                  TR_ApplyRemoteFilters(mtn);

               MA_FreeEMailStruct(email);
            }

            DeleteFile(fname);
         }
         else ER_NewError(GetStr(MSG_ER_ErrorWriteMailfile), fname);
      }
   }

   if (lline >= 0) DoMethod(G->TR->GUI.LV_MAILS, MUIM_NList_Redraw, lline);
}
///
/// TR_GetUIDL
//  Filters out duplicate messages
static void TR_GetUIDL(void)
{
  G->TR->supportUIDL = TR_GetUIDLonServer();
  G->TR->UIDLloc = TR_GetUIDLonDisk();

  if(IsMinListEmpty(&G->TR->transferList) == FALSE)
  {
    // search through our transferList
    struct MinNode *curNode;
    for(curNode = G->TR->transferList.mlh_Head; curNode->mln_Succ; curNode = curNode->mln_Succ)
    {
      struct MailTransferNode *mtn = (struct MailTransferNode *)curNode;
      struct Mail *mail = mtn->mail;

      // if the server doesn`t support the UIDL command we
      // use the TOP command and generate our own UIDL within
      // the GetMessageDetails function
      if(!G->TR->supportUIDL)
         TR_GetMessageDetails(mtn, -1);

      if(TR_FindUIDL(mail->UIDL))
      {
        G->TR->Stats.DupSkipped++;
        MASK_FLAG(mtn->tflags, TRF_DELETE);
      }
    }
  }
}
///
/// TR_DisconnectPOP
//  Terminates a POP3 session
static void TR_DisconnectPOP(void)
{
   set(G->TR->GUI.TX_STATUS, MUIA_Text_Contents, GetStr(MSG_TR_Disconnecting));
   if(!G->Error) TR_SendPOP3Cmd(POPCMD_QUIT, NULL, MSG_ER_BadResponse);
   TR_Disconnect();
}

///
/// TR_GetMailFromNextPOP
//  Downloads and filters mail from a POP3 account
void TR_GetMailFromNextPOP(BOOL isfirst, int singlepop, int guilevel)
{
   static int laststats;
   int msgs, pop = singlepop;

   if (isfirst) /* Init first connection */
   {
      G->LastDL.Error = TRUE;
      if (!TR_OpenTCPIP()) { if (guilevel == POP_USER) ER_NewError(GetStr(MSG_ER_NoTCP)); return; }
      if (!CO_IsValid()) { TR_CloseTCPIP(); return; }
      if (!(G->TR = TR_New(TR_GET))) { TR_CloseTCPIP(); return; }
      G->TR->Checking = TRUE;
      DisplayAppIconStatistics();
      G->TR->GUIlevel = guilevel;
      G->TR->SearchCount = AllocFilterSearch(APPLY_REMOTE);
      if (singlepop >= 0) G->TR->SinglePOP = TRUE;
      else G->TR->POP_Nr = -1;
      laststats = 0;
   }
   else /* Finish previous connection */
   {
      struct POP3 *p = C->P3[G->TR->POP_Nr];

      TR_DisconnectPOP();
      TR_Cleanup();
      AppendLogNormal(30, GetStr(MSG_LOG_Retrieving), (void *)(G->TR->Stats.Downloaded-laststats), p->User, p->Server, "");
      if (G->TR->SinglePOP) pop = MAXP3;
      laststats = G->TR->Stats.Downloaded;
   }
   if (!G->TR->SinglePOP) for (pop = ++G->TR->POP_Nr; pop < MAXP3; pop++)
                             if (C->P3[pop]) if (C->P3[pop]->Enabled) break;

   if (pop >= MAXP3) /* Finish last connection */
   {
      TR_CloseTCPIP();
      set(G->TR->GUI.WI, MUIA_Window_Open, FALSE);
      FreeFilterSearch();
      G->TR->SearchCount = 0;
      MA_StartMacro(MACRO_POSTGET, itoa((int)G->TR->Stats.Downloaded));

      // tell the appicon that we are finished with checking mail
      // the apply rules or DisplayAppIconStatistics() function will refresh it later on
      G->TR->Checking = FALSE;

      // we only apply the filters if we downloaded something, or it`s wasted
      if(G->TR->Stats.Downloaded > 0)
      {
        DoMethod(G->App, MUIM_CallHook, &ApplyFiltersHook, APPLY_AUTO, 0);

        // Now we jump to the first new mail we received if the number of messages has changed
        // after the mail transfer
        if(C->JumpToIncoming) MA_JumpToNewMsg();

        // only call the DisplayStatistics() function if the actual folder wasn`t already the INCOMING
        // one or we would hav refreshed it twice
        if(FO_GetCurrentFolder()->Type != FT_INCOMING) DisplayStatistics((struct Folder *)-1, TRUE);
        else DisplayAppIconStatistics();

        TR_NewMailAlert();
      }
      else DisplayAppIconStatistics();

      // lets populate the LastDL statistics variable with the stats
      // of this download.
      memcpy(&G->LastDL, &G->TR->Stats, sizeof(struct DownloadResult));

      MA_ChangeTransfer(TRUE);

      DisposeModulePush(&G->TR);
      if (G->TR_Exchange)
      {
         G->TR_Exchange = FALSE;
         DoMethod(G->App, MUIM_Application_PushMethod, G->App, 3, MUIM_CallHook, &MA_SendHook, SEND_ALL);
      }

      return;
   }

   // lets initialize some important data first so that the transfer can
   // begin
   G->TR->POP_Nr = pop;
   G->TR_Allow = G->TR->Abort = G->TR->Pause = G->TR->Start = G->Error = FALSE;

   if ((msgs = TR_ConnectPOP(G->TR->GUIlevel)) != -1)    // connection succeeded
   {
      if (msgs)                                          // there are messages on the server
      {
         if (TR_GetMessageList_GET())                    /* message list read OK */
         {
            BOOL preselect = FALSE;
            G->TR->Stats.OnServer += msgs;
            if(G->TR->SearchCount > 0)                   // filter messages on server?
            {
               struct MinNode *curNode;

               set(G->TR->GUI.TX_STATUS, MUIA_Text_Contents, GetStr(MSG_TR_ApplyFilters));
               for(curNode = G->TR->transferList.mlh_Head; curNode->mln_Succ; curNode = curNode->mln_Succ)
                  TR_GetMessageDetails((struct MailTransferNode *)curNode, -2);
            }
            if (C->AvoidDuplicates) TR_GetUIDL();        // read UIDL file to compare against already received messages
            if (G->TR->GUIlevel == POP_USER)             // manually initiated transfer
            {
               if(C->PreSelection >= 2)
                  preselect = TRUE;           // preselect messages if preference is "always [sizes only]"
               else if(C->WarnSize && C->PreSelection) // ...or any sort of preselection and there is a maximum size
               {
                  struct MinNode *curNode;
                  for(curNode = G->TR->transferList.mlh_Head; curNode->mln_Succ; curNode = curNode->mln_Succ)
                  {
                    struct MailTransferNode *mtn = (struct MailTransferNode *)curNode;
                    struct Mail *mail = mtn->mail;

                    if(mail->Size >= C->WarnSize<<10)
                    {
                      preselect = TRUE;
                      break;
                    }
                  }
               }
            }
            if (preselect)                               // anything to preselect?
            {
               // avoid MUIA_Window_Open's side effect of activating the window if it was already open
               if(!xget(G->TR->GUI.WI, MUIA_Window_Open)) set(G->TR->GUI.WI, MUIA_Window_Open, TRUE);

               // if preselect mode is "always [sizes only]"
               if (C->PreSelection == 1)
               {
                  TR_DisplayMailList(TRUE);                          // add entries to list
                  set(G->TR->GUI.GR_LIST, MUIA_ShowMe, TRUE);        // ...and show it
               }
               else
               {
                  TR_DisplayMailList(FALSE);
               }

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
      }
   }
   else G->TR->Stats.Error = TRUE;

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
  result = (TR_SendPOP3Cmd(POPCMD_STAT, NULL, MSG_ER_BadResponse) != NULL);

  RETURN(result);
  return result;
}
///

/*** SMTP routines ***/
/// TR_SendSMTPCmd
//  Sends a command to the SMTP server and returns the response message
//  described in (RFC 821)
static char *TR_SendSMTPCmd(enum SMTPCommand command, char *parmtext, APTR errorMsg)
{
   int len, rc;
   static char *buf = NULL;

   // first we check if the socket is ready
   if(G->TR_Socket == SMTP_NO_SOCKET) { errorMsg = MSG_ER_ConnectionBroken; goto clean_exit; }

   // if we are here for the first time lets generate a minimum buffer
   if(buf == NULL)
   {
      // by lookin at RFC 821 a buffer of 1000 chars for one line
      // should really be enough
      if(!(buf = AllocStrBuf(SIZE_LINE))) { errorMsg = NULL; goto clean_exit; }
   }

   // now we prepare the SMTP command
   if(!parmtext || !*parmtext) sprintf(buf, "%s\r\n", SMTPcmd[command]);
   else sprintf(buf, "%s %s\r\n", SMTPcmd[command], parmtext);

   // lets issue the command, but not if we connect
   if(command != SMTP_CONNECT && TR_WriteLine(buf) <= 0) { errorMsg = MSG_ER_ConnectionBroken; goto clean_exit; }

   // after issuing the SMTP command we read out the server response to it
   if((len = TR_ReadLine(G->TR_Socket, buf, SIZE_LINE)) <= 0) { errorMsg = MSG_ER_ConnectionBroken; goto clean_exit; }

   // get the response code
   rc = strtol(buf, NULL, 10);

   // if the response is a multiline response we have to get out more
   // from the socket
   if(buf[3] == '-') // (RFC 821) - Page 50
   {
      char tbuf[SIZE_LINE];

      do
      {
         // lets get out the next line from the socket
         if((len = TR_ReadLine(G->TR_Socket, tbuf, SIZE_LINE)) <= 0) { errorMsg = MSG_ER_ConnectionBroken; goto clean_exit; }

         // lets concatenate the both strings
         if(StrBufCat(buf, tbuf) == NULL) { errorMsg = NULL; goto clean_exit; }

      }while(tbuf[3] == '-');
   }

   // Now we check if we got the correct response code for the command
   // we issued
   switch(command)
   {
      //  Reponse    Description (RFC 821)
      //  2xx        command accepted and processed
      //  3xx        general flow control
      //  4xx        critical system or transfer failure
      //  5xx        errors with the SMTP command

      case SMTP_HELP:    { if(rc == 211 || rc == 214) return buf; } break;
      case SMTP_VRFY:    { if(rc == 250 || rc == 251) return buf; } break;
      case SMTP_CONNECT: { if(rc == 220) return buf; } break;
      case SMTP_QUIT:    { if(rc == 221) return buf; } break;
      case SMTP_DATA:    { if(rc == 354) return buf; } break;

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
      case SMTP_TURN:    { if(rc == 250) return buf; } break;

      // ESMTP commands & response codes
      case ESMTP_EHLO:            { if(rc == 250) return buf; } break;
      case ESMTP_STARTTLS:        { if(rc == 220) return buf; } break;

      // ESMTP_AUTH command responses
      case ESMTP_AUTH_CRAM_MD5:
      case ESMTP_AUTH_DIGEST_MD5:
      case ESMTP_AUTH_LOGIN:
      case ESMTP_AUTH_PLAIN:      { if(rc == 334) return buf; } break;
   }

clean_exit:

   // the rest of the responses throws an error
   if(errorMsg) ER_NewError(GetStr(errorMsg), (char *)SMTPcmd[command], buf);

   // if we end up with an error we can free our buffer now
   FreeStrBuf(buf);
   buf = NULL;

   return NULL;
}
///
/// TR_ConnectSMTP
//  Connects to a SMTP mail server
static BOOL TR_ConnectSMTP(void)
{
  // If we did a TLS negotitaion previously we have to skip the
  // welcome message, but if it was another connection like a normal or a SSL
  // one we have wait for the welcome
  if(!G->TR_UseTLS || C->SMTP_SecureMethod == SMTPSEC_SSL)
  {
    set(G->TR->GUI.TX_STATUS, MUIA_Text_Contents, GetStr(MSG_TR_WaitWelcome));
    if(!TR_SendSMTPCmd(SMTP_CONNECT, NULL, MSG_ER_BadResponse)) return FALSE;
  }

  set(G->TR->GUI.TX_STATUS,MUIA_Text_Contents, GetStr(MSG_TR_SendHello));
  if(!TR_SendSMTPCmd(SMTP_HELO, C->SMTP_Domain, MSG_ER_BadResponse)) return FALSE;

  return TRUE;
}
///
/// TR_ConnectESMTP
//  Connects to a ESMTP mail server, checks some ESMTP features and returns the
//  supported features in a Flag variable
static int TR_ConnectESMTP(void)
{
   char *resp;
   int ServerFlags = 0;

   // If we did a TLS negotitaion previously we have to skip the
   // welcome message, but if it was another connection like a normal or a SSL
   // one we have wait for the welcome
   if(!G->TR_UseTLS || C->SMTP_SecureMethod == SMTPSEC_SSL)
   {
      set(G->TR->GUI.TX_STATUS, MUIA_Text_Contents, GetStr(MSG_TR_WaitWelcome));
      if(!TR_SendSMTPCmd(SMTP_CONNECT, NULL, MSG_ER_BadResponse)) return 0;
   }

   // Now we send the EHLO command to get the list of features returned.
   if (G->TR_Socket == SMTP_NO_SOCKET) return 0;

   // Now send the EHLO ESMTP command to log in
   set(G->TR->GUI.TX_STATUS,MUIA_Text_Contents, GetStr(MSG_TR_SendHello));
   if(!(resp = TR_SendSMTPCmd(ESMTP_EHLO, C->SMTP_Domain, MSG_ER_BadResponse))) return 0;

   // Now lets see what features this ESMTP Server really has
   while(resp && resp[3] == '-')
   {
      // now lets iterate to the next line
      resp = strchr(resp, '\n');

      // if we do not find any new line or the next one would be anyway
      // too short we break here.
      if(resp == NULL || strlen(++resp) < 4) break;

      // lets see what features this server returns
      if(strnicmp(resp+4, "STARTTLS", 8) == 0)          // STARTTLS (RFC 2487)
      {
         SET_FLAG(ServerFlags, ESMTP_FLG_STARTTLS);
      }
      else if(strnicmp(resp+4, "AUTH", 4) == 0)         // SMTP-AUTH (RFC 2554)
      {
         if (NULL != strstr(resp+9,"CRAM-MD5"))   SET_FLAG(ServerFlags, ESMTP_FLG_AUTH_CRAM_MD5);
         if (NULL != strstr(resp+9,"DIGEST-MD5")) SET_FLAG(ServerFlags, ESMTP_FLG_AUTH_DIGEST_MD5);
         if (NULL != strstr(resp+9,"PLAIN"))      SET_FLAG(ServerFlags, ESMTP_FLG_AUTH_PLAIN);
         if (NULL != strstr(resp+9,"LOGIN"))      SET_FLAG(ServerFlags, ESMTP_FLG_AUTH_LOGIN);
      }
      else if(strnicmp(resp+4, "SIZE", 4) == 0)         // STD:10 - SIZE declaration (RFC 1870)
      {
        SET_FLAG(ServerFlags, ESMTP_FLG_SIZE);
      }
      else if(strnicmp(resp+4, "PIPELINING", 10) == 0)  // STD:60 - PIPELINING (RFC 2920)
      {
        SET_FLAG(ServerFlags, ESMTP_FLG_PIPELINING);
      }
      else if(strnicmp(resp+4, "8BITMIME", 8) == 0)     // 8BITMIME support (RFC 1652)
      {
        SET_FLAG(ServerFlags, ESMTP_FLG_8BITMIME);
      }
      else if(strnicmp(resp+4, "DSN", 3) == 0)          // DSN - Delivery Status Notifications (RFC 1891)
      {
        SET_FLAG(ServerFlags, ESMTP_FLG_DSN);
      }
      else if(strnicmp(resp+4, "ETRN", 4) == 0)         // ETRN - Remote Message Queue Starting (RFC 1985)
      {
        SET_FLAG(ServerFlags, ESMTP_FLG_ETRN);
      }
      else if(strnicmp(resp+4, "ENHANCEDSTATUSCODES", 19) == 0) // Enhanced Status Codes (RFC 2034)
      {
        SET_FLAG(ServerFlags, ESMTP_FLG_ENHANCEDSTATUSCODES);
      }
      else if(strnicmp(resp+4, "DELIVERBY", 9) == 0)    // DELIVERBY Extension (RFC 2852)
      {
        SET_FLAG(ServerFlags, ESMTP_FLG_DELIVERBY);
      }
      else if(strnicmp(resp+4, "HELP", 4) == 0)         // HELP Extension (RFC 821)
      {
        SET_FLAG(ServerFlags, ESMTP_FLG_HELP);
      }
   }

#ifdef DEBUG
   D(DBF_NET, "ESMTP Server '%s' serves:", C->SMTP_Server);
   D(DBF_NET, "  AUTH CRAM-MD5......: %ld", hasCRAM_MD5_Auth(ServerFlags));
   D(DBF_NET, "  AUTH DIGEST-MD5....: %ld", hasDIGEST_MD5_Auth(ServerFlags));
   D(DBF_NET, "  AUTH LOGIN.........: %ld", hasLOGIN_Auth(ServerFlags));
   D(DBF_NET, "  AUTH PLAIN.........: %ld", hasPLAIN_Auth(ServerFlags));
   D(DBF_NET, "  STARTTLS...........: %ld", hasSTARTTLS(ServerFlags));
   D(DBF_NET, "  SIZE...............: %ld", hasSIZE(ServerFlags));
   D(DBF_NET, "  PIPELINING.........: %ld", hasPIPELINING(ServerFlags));
   D(DBF_NET, "  8BITMIME...........: %ld", has8BITMIME(ServerFlags));
   D(DBF_NET, "  DSN................: %ld", hasDSN(ServerFlags));
   D(DBF_NET, "  ETRN...............: %ld", hasETRN(ServerFlags));
   D(DBF_NET, "  ENHANCEDSTATUSCODES: %ld", hasENHANCEDSTATUSCODES(ServerFlags));
   D(DBF_NET, "  DELIVERBY..........: %ld", hasDELIVERBY(ServerFlags));
   D(DBF_NET, "  HELP...............: %ld", hasHELP(ServerFlags));
#endif

   return ServerFlags;
}

///
/// TR_DisconnectSMTP
//  Terminates a SMTP session
static void TR_DisconnectSMTP(void)
{
   set(G->TR->GUI.TX_STATUS, MUIA_Text_Contents, GetStr(MSG_TR_Disconnecting));
   if (!G->Error) TR_SendSMTPCmd(SMTP_QUIT, NULL, MSG_ER_BadResponse);
   TR_Disconnect();
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
  ts->Msgs_Tot = ts->Size_Tot = 0;

  if(G->TR->GUI.GR_LIST)
  {
    set(G->TR->GUI.GR_PAGE, MUIA_Group_ActivePage, 1);
    DoMethod(G->TR->GUI.LV_MAILS, MUIM_NList_Select, MUIV_NList_Select_All, MUIV_NList_Select_Off, NULL);
  }

  if(IsMinListEmpty(&G->TR->transferList) == FALSE)
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
}
///
/// TR_TransStat_Start
//  Resets statistics display
static void TR_TransStat_Start(struct TransStat *ts)
{
   ts->Msgs_Done = ts->Size_Done = 0;

   // get the actual time we started the TransferStatus
   GetSysTime(&ts->Clock_Last);
   #if defined(__NEWLIB__)
   ts->Clock_Start = ts->Clock_Last.tv_sec;
   #else
   ts->Clock_Start = ts->Clock_Last.tv_secs;
   #endif

   SPrintF(G->TR->CountLabel, GetStr(MSG_TR_MessageGauge), "%ld", ts->Msgs_Tot);
   SetAttrs(G->TR->GUI.GA_COUNT, MUIA_Gauge_InfoText, G->TR->CountLabel,
                                 MUIA_Gauge_Max,      ts->Msgs_Tot,
                                 TAG_DONE);
}
///
/// TR_TransStat_NextMsg
//  Updates statistics display for next message
static void TR_TransStat_NextMsg(struct TransStat *ts, int index, int listpos, LONG size, char *status)
{
   ts->Size_Curr = 0;
   ts->Size_Curr_Max = size;

   // if the window isn`t open we don`t need to update it, do we?
   if(!xget(G->TR->GUI.WI, MUIA_Window_Open)) return;

   // get the new time since the last nextmsg start
   GetSysTime(&ts->Clock_Last);

   // if we have a preselection window, update it.
   if(G->TR->GUI.GR_LIST && listpos >= 0)
     set(G->TR->GUI.LV_MAILS, MUIA_NList_Active, listpos);

   set(G->TR->GUI.TX_STATUS, MUIA_Text_Contents, status);
   set(G->TR->GUI.GA_COUNT, MUIA_Gauge_Current, index);

   // and last, but not least update the gauge.
   SPrintF(G->TR->BytesLabel, GetStr(MSG_TR_SizeGauge), size);
   SetAttrs(G->TR->GUI.GA_BYTES, MUIA_Gauge_Current,  0,
                                 MUIA_Gauge_InfoText, G->TR->BytesLabel,
                                 MUIA_Gauge_Max,      size,
                                 TAG_DONE);
}
///
/// TR_TransStat_Update
//  Updates statistics display for next block of data
static void TR_TransStat_Update(struct TransStat *ts, int size_incr)
{
   if(size_incr > 0)
   {
     struct timeval now;

     ts->Size_Curr += size_incr;
     ts->Size_Done += size_incr;

     // if the window isn`t open we don`t need to update it, do we?
     if(!xget(G->TR->GUI.WI, MUIA_Window_Open)) return;

     // now we check if should really update our
     // transfer display or if it will be overkill
     // we shouldn`t update it more than twice a second.
     GetSysTime(&now);
     if(-CmpTime(&now, &ts->Clock_Last) > 0)
     {
        struct timeval delta;

        // how much time has passed exactly?
        delta = now;
        SubTime(&delta, &ts->Clock_Last);

        // update the display at least twice a second
        #if defined(__NEWLIB__)
        if(delta.tv_sec > 0 || delta.tv_usec > 250000)
        #else
        if(delta.tv_secs > 0 || delta.tv_micro > 250000)
        #endif
        {
           #if defined(__NEWLIB__)
           ULONG deltatime = now.tv_sec - ts->Clock_Start;
           #else
           ULONG deltatime = now.tv_secs - ts->Clock_Start;
           #endif
           ULONG speed = 0;
           LONG remclock = 0;

           // first we calculate the speed in bytes/sec
           // to display to the user
           if(deltatime)
              speed = ts->Size_Done/deltatime;

           // calculate the estimated remaining time
           if(speed && ((remclock = (ts->Size_Tot/speed)-deltatime) < 0))
              remclock = 0;

           // now format the StatsLabel and update it aswell as the gauge
           SPrintF(G->TR->StatsLabel, GetStr(MSG_TR_TransferStats),
                   ts->Size_Done/1024, ts->Size_Tot/1024, speed, remclock/60, remclock%60);
           set(G->TR->GUI.TX_STATS, MUIA_Text_Contents, G->TR->StatsLabel);
           set(G->TR->GUI.GA_BYTES, MUIA_Gauge_Current, ts->Size_Curr);

           // signal the application to update now
           DoMethod(G->App, MUIM_Application_InputBuffered);

           ts->Clock_Last = now;
        }
     }
   }
   else if(size_incr == TS_SETMAX)
   {
     struct timeval now;
     ULONG deltatime;
     ULONG speed = 0;
     LONG remclock = 0;

     ts->Size_Done += ts->Size_Curr_Max - ts->Size_Curr;

     // if the window isn`t open we don`t need to update it, do we?
     if(!xget(G->TR->GUI.WI, MUIA_Window_Open)) return;

     // we make sure that we, at least update the gauge at the end
     GetSysTime(&now);
     #if defined(__NEWLIB__)
     deltatime = now.tv_sec - ts->Clock_Start;
     #else
     deltatime = now.tv_secs - ts->Clock_Start;
     #endif

     // first we calculate the speed in bytes/sec
     // to display to the user
     if(deltatime)
       speed = ts->Size_Done/deltatime;

     // calculate the estimated remaining time
     if(speed && ((remclock = (ts->Size_Tot/speed)-deltatime) < 0))
       remclock = 0;

     // now format the StatsLabel and update it aswell as the gauge
     SPrintF(G->TR->StatsLabel, GetStr(MSG_TR_TransferStats),
             ts->Size_Done/1024, ts->Size_Tot/1024, speed, remclock/60, remclock%60);
     set(G->TR->GUI.TX_STATS, MUIA_Text_Contents, G->TR->StatsLabel);

     // if size_increment is a negative number it is
     // a signal that we are at the end of the transfer, so we can put up
     // the gauge to maximum
     set(G->TR->GUI.GA_BYTES, MUIA_Gauge_Current, ts->Size_Curr_Max);

     // signal the application to update now
     DoMethod(G->App, MUIM_Application_InputBuffered);
   }
}
///
/// TR_Cleanup
//  Free temporary message and UIDL lists
void TR_Cleanup(void)
{
  if(G->TR->GUI.LV_MAILS)
    DoMethod(G->TR->GUI.LV_MAILS, MUIM_NList_Clear);

  if(IsMinListEmpty(&G->TR->transferList) == FALSE)
  {
    struct MinNode *curNode;
    for(curNode = G->TR->transferList.mlh_Head; curNode->mln_Succ;)
    {
      struct MailTransferNode *mtn = (struct MailTransferNode *)curNode;
      struct Mail *mail = mtn->mail;

      // before we remove the node we have to save the pointer to the next one
      curNode = curNode->mln_Succ;

      // Remove node from list
      Remove((struct Node *)mtn);

      // Free everything of the node
      if(mail->UIDL)
        free(mail->UIDL);

      // free the mail pointer
      free(mail);

      // free the node itself
      free(mtn);
    }
  }

  if(G->TR->UIDLloc)
    free(G->TR->UIDLloc);

  G->TR->UIDLloc = NULL;

  NewList((struct List *)&G->TR->transferList);
}
///
/// TR_AbortnClose
//  Aborts a transfer
static void TR_AbortnClose(void)
{
   set(G->TR->GUI.WI, MUIA_Window_Open, FALSE);
   TR_Cleanup();
   MA_ChangeTransfer(TRUE);
   DisposeModulePush(&G->TR);
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

/*** EXPORT ***/
/// TR_ProcessEXPORT
//  Saves a list of messages to a MBOX mailbox file
BOOL TR_ProcessEXPORT(char *fname, struct Mail **mlist, BOOL append)
{
   BOOL success = FALSE;
   int i;

   // reset our processing list
   NewList((struct List *)&G->TR->transferList);

   // temporarly copy all data out of our mlist to the
   // processing list and mark all mails to get "loaded"
   for(i=0; i < (int)*mlist; i++)
   {
      struct MailTransferNode *mtn;

      if((mtn = calloc(1, sizeof(struct MailTransferNode))))
      {
        if((mtn->mail = malloc(sizeof(struct Mail))))
        {
           memcpy(mtn->mail, mlist[i+2], sizeof(struct Mail));
           mtn->index  = i+1;

           // set to LOAD
           mtn->tflags = TRF_LOAD;

           AddTail((struct List *)&(G->TR->transferList), (struct Node *)mtn);
        }
        else
          return FALSE;
      }
      else
        return FALSE;
   }

   // if we have now something in our processing list,
   // lets go on
   if(IsMinListEmpty(&G->TR->transferList) == FALSE)
   {
      FILE *fh;
      struct TransStat ts;

      TR_SetWinTitle(FALSE, (char *)FilePart(fname));
      TR_TransStat_Init(&ts);
      TR_TransStat_Start(&ts);

      // open our final destination file either in append or in a fresh
      // write mode.
      if((fh = fopen(fname, append ? "a" : "w")))
      {
         struct MinNode *curNode;

         success = TRUE;

         for(curNode = G->TR->transferList.mlh_Head; curNode->mln_Succ && !G->TR->Abort && success; curNode = curNode->mln_Succ)
         {
            struct MailTransferNode *mtn = (struct MailTransferNode *)curNode;
            struct Mail *mail = mtn->mail;
            char fullfile[SIZE_PATHFILE];

            // update the transfer status
            ts.Msgs_Done++;
            TR_TransStat_NextMsg(&ts, mtn->index, -1, mail->Size, GetStr(MSG_TR_Exporting));

            if(StartUnpack(GetMailFile(NULL, NULL, mail), fullfile, mail->Folder))
            {
               FILE *mfh;

               // open the message file to start exporting it
               if((mfh = fopen(fullfile, "r")))
               {
                  char datstr[64];
                  char buf[SIZE_LINE];
                  BOOL inHeader = TRUE;

                  // printf out our leading "From " MBOX format line first
                  DateStamp2String(datstr, &mail->Date, DSS_UNIXDATE, TZC_NONE);
                  fprintf(fh, "From %s %s", mail->From.Address, datstr);

                  // let us put out the Status: header field
                  fprintf(fh, "Status: %s\n", MA_ToStatusHeader(mail));

                  // let us put out the X-Status: header field
                  fprintf(fh, "X-Status: %s\n", MA_ToXStatusHeader(mail));

                  // initialize buf first
                  buf[0] = '\0';

                  // now we iterate through every line of our mail and try to substitute
                  // found "From " line with quoted ones
                  while(!G->TR->Abort && fgets(buf, SIZE_LINE, mfh))
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
                     while(*tmp == '>') tmp++;
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
         AppendLog(51, GetStr(MSG_LOG_Exporting), (void *)ts.Msgs_Done, mlist[2]->Folder->Name, fname, "");
      }
   }

   TR_AbortnClose();
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
   FILE *f;

   if ((f = fopen(mf = GetMailFile(NULL, outfolder, mail), "r")))
   {
      char buf[SIZE_LINE];
      sprintf(buf, "FROM:<%s>", C->EmailAddress);
      if (TR_SendSMTPCmd(SMTP_MAIL, buf, MSG_ER_BadResponse))
      {
         int j;
         BOOL rcptok = TRUE;
         struct ExtendedMail *email = MA_ExamineMail(outfolder, mail->MailFile, TRUE);

         if(email)
         {
            sprintf(buf, "TO:<%s>", mail->To.Address);
            if (!TR_SendSMTPCmd(SMTP_RCPT, buf, MSG_ER_BadResponse)) rcptok = FALSE;
            for (j = 0; j < email->NoSTo && rcptok; j++)
            {
              sprintf(buf, "TO:<%s>", email->STo[j].Address);
              if (!TR_SendSMTPCmd(SMTP_RCPT, buf, MSG_ER_BadResponse)) rcptok = FALSE;
            }
            for (j = 0; j < email->NoCC && rcptok; j++)
            {
              sprintf(buf, "TO:<%s>", email->CC[j].Address);
              if (!TR_SendSMTPCmd(SMTP_RCPT, buf, MSG_ER_BadResponse)) rcptok = FALSE;
            }
            for (j = 0; j < email->NoBCC && rcptok; j++)
            {
              sprintf(buf, "TO:<%s>", email->BCC[j].Address);
              if (!TR_SendSMTPCmd(SMTP_RCPT, buf, MSG_ER_BadResponse)) rcptok = FALSE;
            }

            if (rcptok)
            {
              if (TR_SendSMTPCmd(SMTP_DATA, NULL, MSG_ER_BadResponse))
              {
                BOOL lineskip = FALSE, inbody = FALSE;
                char sendbuf[SIZE_LINE+2];
                int sendsize, cpos;
                int prevpos = ftell(f); // get current file position
                int startpos = prevpos;

                // as long there is no abort situation we go on reading out
                // from the stream and sending it to our SMTP server
                while(!G->TR->Abort && !G->Error && fgets(buf, SIZE_LINE, f))
                {
                  sendsize = cpos = ftell(f)-prevpos; // get the size we really read out from the stream.
                  prevpos += sendsize;                // set the new prevpos to the ftell() value.

                  // as long as we process header lines we have to make differ in some ways.
                  if(!inbody)
                  {
                    // we check if we found the body of the mail now
                    // the start of a body is seperated by the header with a single
                    // empty line and we have to make sure that it isn`t the beginning of the file
                    if(sendsize == 1 && buf[0] == '\n' && buf[1] == '\0' && prevpos-startpos > 1)
                    {
                      inbody = TRUE;
                      lineskip = FALSE;
                    }
                    else if(!ISpace(*buf)) // headerlines don`t start with a space
                    {
                      // headerlines with bcc or x-yam- will be skipped by us.
                      lineskip = !strnicmp(buf, "bcc", 3) || !strnicmp(buf, "x-yam-", 6);
                    }
                  }

                  // if we don`t skip this line we write it out to the SMTP server
                  if(!lineskip)
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
                    if(TR_Send(sendbuf, sendsize, TCPF_NONE) <= 0) ER_NewError(GetStr(MSG_ER_ConnectionBroken));
                  }

                  TR_TransStat_Update(ts, cpos);
                }

                // if buf == NULL when we arrive here, then the fgets()
                // at the top exited with an error
                if(buf == NULL) { ER_NewError(GetStr(MSG_ER_ErrorReadMailfile), mf); result = -1; }
                else if(!G->TR->Abort && !G->Error)
                {
                  // we have to flush the write buffer if this wasn`t a error or
                  // abort situation
                  TR_WriteFlush();

                  // send a CRLF+octet "\r\n." to signal that the data is finished.
                  // we do it here because if there was an error and we send it, the message
                  // will be send incomplete.
                  if(TR_SendSMTPCmd(SMTP_FINISH, NULL, MSG_ER_BadResponse))
                  {
                    // put the transferStat to 100%
                    TR_TransStat_Update(ts, TS_SETMAX);

                    // now that we are at 100% we have to set the transfer Date of the message
                    GetSysTimeUTC(&mail->Reference->transDate);

                    result = email->DelSend ? 2 : 1;
                    AppendLogVerbose(42, GetStr(MSG_LOG_SendingVerbose), AddrName(mail->To), mail->Subject, (void *)mail->Size, "");
                  }
                }

                if(G->TR->Abort || G->Error) result = -1; // signal the caller that we aborted within the DATA part
              }
            }
            MA_FreeEMailStruct(email);
          }
          else ER_NewError(GetStr(MSG_ER_CantOpenFile), mf);
      }
      fclose(f);
   }
   else ER_NewError(GetStr(MSG_ER_CantOpenFile), mf);

   return result;
}
///
/// TR_ProcessSEND
//  Sends a list of messages
BOOL TR_ProcessSEND(struct Mail **mlist)
{
   struct TransStat ts;
   int c, i, err;
   int port = C->SMTP_Port;
   struct Folder *outfolder = FO_GetFolderByType(FT_OUTGOING, NULL);
   struct Folder *sentfolder = FO_GetFolderByType(FT_SENT, NULL);
   BOOL success = FALSE;
   char *p;

   NewList((struct List *)&G->TR->transferList);
   G->TR_Allow = G->TR->Abort = G->Error = FALSE;

   for(c = i = 0; i < (int)*mlist; i++)
   {
      struct Mail *mail;
      mail = mlist[i+2];

      if(hasStatusQueued(mail) || hasStatusError(mail))
      {
        struct MailTransferNode *mtn;
        if((mtn = calloc(1, sizeof(struct MailTransferNode))))
        {
          struct Mail *newMail;
          if((newMail = malloc(sizeof(struct Mail))))
          {
            memcpy(newMail, mail, sizeof(struct Mail));
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

   if (c)
   {
      char host[SIZE_HOST];

      // now we have to check whether SSL/TLS is selected for SMTP account,
      // but perhaps TLS is not working.
      if(C->SMTP_SecureMethod != SMTPSEC_NONE &&
         !G->TR_UseableTLS)
      {
        ER_NewError(GetStr(MSG_ER_UNUSABLEAMISSL));
        return FALSE;
      }

      G->TR->SearchCount = AllocFilterSearch(APPLY_SENT);
      TR_TransStat_Init(&ts);
      TR_TransStat_Start(&ts);
      strcpy(host, C->SMTP_Server);

      // If the hostname has a explicit :xxxxx port statement at the end we
      // take this one, even if its not needed anymore.
      if ((p = strchr(host, ':'))) { *p = 0; port = atoi(++p); }

      set(G->TR->GUI.TX_STATUS, MUIA_Text_Contents, GetStr(MSG_TR_Connecting));

      BusyText(GetStr(MSG_TR_MailTransferTo), host);

      TR_SetWinTitle(FALSE, host);

      if (!(err = TR_Connect(host, port)))
      {
         BOOL connected = TRUE;

         // first we check whether the user wants to connect to a plain SSLv3 server
         // so that we initiate the SSL connection now
         if(C->SMTP_SecureMethod == SMTPSEC_SSL)
         {
            // lets try to establish the SSL connection via AmiSSL
            if(TR_InitTLS() && TR_StartTLS())
            {
              G->TR_UseTLS = TRUE;
            }
            else
            {
              ER_NewError(GetStr(MSG_ER_INITTLS), host);
              return FALSE;
            }
         }

         // first we have to check whether the user requested some
         // feature that requires a ESMTP Server, and if so we connect via ESMTP
         if(C->Use_SMTP_AUTH || C->SMTP_SecureMethod == SMTPSEC_TLS)
         {
            int ServerFlags = TR_ConnectESMTP();

            // Now we have to check whether the user has selected SSL/TLS
            // and then we have to initiate the STARTTLS command followed by the TLS negotiation
            if(C->SMTP_SecureMethod == SMTPSEC_TLS)
            {
              connected = TR_InitSTARTTLS(ServerFlags);

              // then we have to refresh the ServerFlags and check
              // again what features we have after the STARTTLS
              if(connected)
              {
                // first we flag this connection as a sucessfull
                // TLS session
                G->TR_UseTLS = TRUE;

                ServerFlags = TR_ConnectESMTP();
              }
            }

            // If the user selected SMTP_AUTH we have to initiate
            // a AUTH connection
            if(connected && C->Use_SMTP_AUTH)
            {
              connected = TR_InitSMTPAUTH(ServerFlags);
            }
         }
         else
         {
            // Init a normal non-ESMTP connection by sending a HELO
            connected = TR_ConnectSMTP();
         }

         // If we are still "connected" we can proceed with transfering the data
         if(connected)
         {
            struct MinNode *curNode;

            success = TRUE;
            AppendLogVerbose(41, GetStr(MSG_LOG_ConnectSMTP), host, "", "", "");

            for(curNode = G->TR->transferList.mlh_Head; curNode->mln_Succ; curNode = curNode->mln_Succ)
            {
               struct MailTransferNode *mtn = (struct MailTransferNode *)curNode;
               struct Mail *mail = mtn->mail;

               if(G->TR->Abort || G->Error)
                 break;

               ts.Msgs_Done++;
               TR_TransStat_NextMsg(&ts, mtn->index, -1, mail->Size, GetStr(MSG_TR_Sending));

               switch (TR_SendMessage(&ts, mail))
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
                    TR_SendSMTPCmd(SMTP_RSET, NULL, MSG_ER_BadResponse);
                  }
                  break;

                  // 1 means we filter the mails and then copy/move the mail to the send folder
                  case 1:
                  {
                    setStatusToSent(mail->Reference);
                    if(TR_ApplySentFilters(mail->Reference))
                      MA_MoveCopy(mail->Reference, outfolder, sentfolder, FALSE);
                  }
                  break;

                  // 2 means we filter and delete afterwards
                  case 2:
                  {
                    setStatusToSent(mail->Reference);
                    if (TR_ApplySentFilters(mail->Reference))
                      MA_DeleteSingle(mail->Reference, FALSE, FALSE);
                  }
                  break;
               }
            }
            AppendLogNormal(40, GetStr(MSG_LOG_Sending), (void *)c, host, "", "");
         }
         else err = 1;

         TR_DisconnectSMTP();
      }

      // if we got an error here, let`s throw it
      if(err != 0)
      {
        switch (err)
        {
          case -1: ER_NewError(GetStr(MSG_ER_UnknownSMTP), C->SMTP_Server); break;
          default: ER_NewError(GetStr(MSG_ER_CantConnect), C->SMTP_Server);
        }
      }

      FreeFilterSearch();
      G->TR->SearchCount = 0;
   }

   TR_AbortnClose();

   BusyEnd();
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

    if((mtn = calloc(1, sizeof(struct MailTransferNode))))
    {
      struct Mail *mail;
      if((mail = malloc(sizeof(struct Mail))))
      {
        memcpy(mail, &email->Mail, sizeof(struct Mail));
        mail->Folder  = NULL;
        mail->Size    = size;

        // flag the mail as being transfered
        mtn->index      = ++(*count);
        mtn->tflags     = TRF_LOAD;
        mtn->mail       = mail;
        mtn->importAddr = addr;

        AddTail((struct List *)&G->TR->transferList, (struct Node *)mtn);

        D(DBF_IMPORT, "added mail '%s' (%d bytes) to import list.", mail->Subject, size);

        ret = mtn;
      }
      else
        E(DBF_IMPORT, "Couldn't allocate enough memory for struct Mail");
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
    E(DBF_IMPORT, "Couldn't allocate %d bytes", size);
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
    E(DBF_IMPORT, "Unable to read %d bytes", size);
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

    D(DBF_IMPORT, "read in %d bytes of object at 0x%x, but index length is %d", size, addr, length_of_idxs);

    if(!(newbuf = malloc(length_of_idxs + 12)))
    {
      E(DBF_IMPORT, "Couldn't allocate %d bytes", length_of_idxs+12);
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
  if(!(mailout = fopen(outFileName, "wb")))
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
  if(mailout)
    fclose(mailout);

  free(buf);
  
  if(rc)
  {
    if(preview == TRUE)
    {
      struct MailTransferNode *mtn;

      // if this is the preview run we go and
      // use the TR_AddMessageHeader method to
      // add the found mail to our mail list
      if((mtn = TR_AddMessageHeader(mail_accu, FileSize(outFileName), msg_addr, FilePart(outFileName))))
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

    E(DBF_IMPORT, "Unable to read %d bytes", 0x18+0x264);
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
BOOL TR_GetMessageList_IMPORT()
{
  BOOL result = FALSE;
  char tfname[SIZE_MFILE];
  char fname[SIZE_PATHFILE];
  int c = 0;

  ENTER();

  // check if MA_ImportMessages() did correctly set all required
  // data
  if(G->TR->ImportFile[0] == '\0' || G->TR->ImportFolder == NULL)
  {
    RETURN(FALSE);
    return FALSE;
  }

  // clear the found mail list per default
  NewList((struct List *)&G->TR->transferList);

  // prepare the temporary filename buffers
  sprintf(tfname, "YAMi%02d.tmp", G->RexxHost->portnumber);
  strmfp(fname, C->TempDir, tfname);

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

      if((ifh = fopen(G->TR->ImportFile, "r")))
      {
        FILE *ofh = NULL;
        char buffer[SIZE_LINE];
        BOOL foundBody = FALSE;
        int size = 0;
        long addr = 0;

        while(GetLine(ifh, buffer, SIZE_LINE))
        {
          // now we parse through the input file until we
          // find the "From " separator
          if(strncmp(buffer, "From ", 5) == 0)
          {
            // now we know that a new mail has started so if
            // we already found a previous mail we can add it
            // to our list
            if(foundBody)
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

            size = 0;
            foundBody = FALSE;
            addr = ftell(ifh);

            // continue with the next iteration
            continue;
          }

          // if we already have an opened tempfile
          // and we didn't found the separating mail body
          // yet we go and write out the buffer content
          if(ofh && foundBody == FALSE)
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
          if(ofh || foundBody)
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
        if(foundBody)
        {
          result = (TR_AddMessageHeader(&c, size, addr, tfname) != NULL);
          DeleteFile(fname);
        }
        else if(ofh)
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
      if(CopyFile(fname, NULL, G->TR->ImportFile, NULL))
      {
        // if the file was identified as a plain .eml file we
        // just have to go and call TR_AddMessageHeader to let
        // YAM analyze the file
        result = (TR_AddMessageHeader(&c, FileSize(fname), 0, tfname) != NULL);

        DeleteFile(fname);
      }
    }
    break;

    // treat the file as a DBX (Outlook Express) compliant mail archive
    case IMF_DBX:
    {
      FILE *ifh;

      // lets open the file and read out the root node of the dbx mail file
      if((ifh = fopen(G->TR->ImportFile, "rb")))
      {
        unsigned char *file_header;

        // read the 9404 bytes long file header for properly identifying
        // an Outlook Express database file.
        if((file_header = (unsigned char *)malloc(0x24bc)))
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

              D(DBF_IMPORT, "number of mails in dbx file: %d", number_of_mails);

              // now we actually start at the root node and read in all messages
              // accordingly
              if(ReadDBXNode(ifh, fname, root_node, &c, TRUE) && c == number_of_mails)
                result = TRUE;
              else
                E(DBF_IMPORT, "Failed to read from root_node; c=%d", c);
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

  RETURN(result && c > 0);
  return result && c > 0;
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
  struct TransStat ts;

  // if there is nothing to import we can skip
  // immediately.
  if(IsMinListEmpty(&G->TR->transferList))
    return;

  TR_TransStat_Init(&ts);
  if(ts.Msgs_Tot)
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

        if((ifh = fopen(G->TR->ImportFile, "r")))
        {
          struct MinNode *curNode;

          // iterate through our transferList and seek to
          // each position/address of a mail
          for(curNode = G->TR->transferList.mlh_Head; curNode->mln_Succ && !G->TR->Abort; curNode = curNode->mln_Succ)
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

            TR_TransStat_NextMsg(&ts, mtn->index, mtn->position, mail->Size, GetStr(MSG_TR_Importing));

            if((ofh = fopen(MA_NewMailFile(folder, mfile), "w")) == NULL)
              break;

            // now that we seeked to the mail address we go
            // and read in line by line
            while(GetLine(ifh, buffer, SIZE_LINE) && !G->TR->Abort)
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
                    if(ownStatusFound)
                      stat |= MA_FromXStatusHeader(&buffer[10]);
                    else
                      stat = MA_FromXStatusHeader(&buffer[10]);

                     ownStatusFound = TRUE;
                  }
                  else if(strnicmp(buffer, "Status: ", 8) == 0)
                  {
                    if(ownStatusFound)
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
                stat = SFLAG_QUEUED;
              else if(ftype == FT_SENT || ftype == FT_CUSTOMSENT)
                stat = SFLAG_SENT;
              else
                stat = SFLAG_NEW;
            }

            SET_FLAG(mail->sflags, stat);

            // depending on the Status we have to set the transDate or not
            if(!hasStatusQueued(mail) && !hasStatusHold(mail))
              GetSysTimeUTC(&mail->transDate);

            // add the mail to the folderlist now
            if((mail = AddMailToList(mail, folder)))
            {
              // update the mailFile Path
              memcpy(mail->MailFile, mfile, SIZE_MFILE*sizeof(char));

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

        if((ifh = fopen(G->TR->ImportFile, "rb")))
        {
          struct MinNode *curNode;

          // iterate through our transferList and seek to
          // each position/address of a mail
          for(curNode = G->TR->transferList.mlh_Head; curNode->mln_Succ && !G->TR->Abort; curNode = curNode->mln_Succ)
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

            TR_TransStat_NextMsg(&ts, mtn->index, mtn->position, mail->Size, GetStr(MSG_TR_Importing));

            if((ofh = fopen(MA_NewMailFile(folder, mfile), "wb")) == NULL)
              break;

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
                stat = SFLAG_QUEUED;
              else if(ftype == FT_SENT || ftype == FT_CUSTOMSENT)
                stat = SFLAG_SENT;
              else
                stat = SFLAG_NEW;

              SET_FLAG(mail->sflags, stat);
            }

            // depending on the Status we have to set the transDate or not
            if(!hasStatusQueued(mail) && !hasStatusHold(mail))
              GetSysTimeUTC(&mail->transDate);

            // add the mail to the folderlist now
            if((mail = AddMailToList(mail, folder)))
            {
              // update the mailFile Path
              memcpy(mail->MailFile, mfile, SIZE_MFILE*sizeof(char));

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

    DisplayMailList(folder, G->MA->GUI.PG_MAILLIST);
    AppendLog(50, GetStr(MSG_LOG_Importing), (void *)ts.Msgs_Done, G->TR->ImportFile, folder->Name, "");
    DisplayStatistics(folder, TRUE);
  }

  TR_AbortnClose();
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
static BOOL TR_LoadMessage(struct TransStat *ts, int number)
{
   static char mfile[SIZE_MFILE];
   struct Folder *infolder = FO_GetFolderByType(FT_INCOMING, NULL);
   char msgnum[SIZE_SMALL], msgfile[SIZE_PATHFILE];
   FILE *f;

   MyStrCpy(msgfile, MA_NewMailFile(infolder, mfile));

   if ((f = fopen(msgfile, "w")))
   {
      BOOL done = FALSE;

      sprintf(msgnum, "%d", number);
      if(TR_SendPOP3Cmd(POPCMD_RETR, msgnum, MSG_ER_BadResponse))
      {
         // now we call a subfunction to receive data from the POP3 server
         // and write it in the filehandle as long as there is no termination \r\n.\r\n
         if(TR_RecvToFile(f, msgfile, ts) > 0) done = TRUE;
      }
      fclose(f);

      if(!G->TR->Abort && !G->Error && done)
      {
         struct ExtendedMail *mail;
         if((mail = MA_ExamineMail(infolder, mfile, FALSE)))
         {
            struct Mail *new = AddMailToList((struct Mail *)mail, infolder);

            // we have to get the actual Time and place it in the transDate, so that we know at
            // which time this mail arrived
            GetSysTimeUTC(&new->transDate);

            new->sflags = SFLAG_NEW;
            MA_UpdateMailFile(new);

            if(FO_GetCurrentFolder() == infolder)
              DoMethod(G->MA->GUI.PG_MAILLIST, MUIM_NList_InsertSingle, new, MUIV_NList_Insert_Sorted);

            AppendLogVerbose(32, GetStr(MSG_LOG_RetrievingVerbose), AddrName(new->From), new->Subject, (void *)new->Size, "");
            MA_StartMacro(MACRO_NEWMSG, mfile);
            MA_FreeEMailStruct(mail);
         }
         return TRUE;
      }

      DeleteFile(msgfile);
      SET_FLAG(infolder->Flags, FOFL_MODIFY); // we need to set the folder flags to modified so that the .index will be saved later.
   }
   else ER_NewError(GetStr(MSG_ER_ErrorWriteMailfile), mfile);

   return FALSE;
}
///
/// TR_DeleteMessage
//  Deletes a message on the POP3 server
static void TR_DeleteMessage(int number)
{
   char msgnum[SIZE_SMALL];

   sprintf(msgnum, "%d", number);
   set(G->TR->GUI.TX_STATUS, MUIA_Text_Contents, GetStr(MSG_TR_DeletingServerMail));
   if(TR_SendPOP3Cmd(POPCMD_DELE, msgnum, MSG_ER_BadResponse)) G->TR->Stats.Deleted++;
}
///
/// TR_NewMailAlert
//  Notifies user when new mail is available
static void TR_NewMailAlert(void)
{
   struct DownloadResult *stats = &G->TR->Stats;

   if (!stats->Downloaded) return;
   if(hasRequesterNotify(C->NotifyType) && G->TR->GUIlevel != POP_REXX)
   {
      int iconified;
      static char buffer[SIZE_LARGE];
      struct RuleResult *rr = &G->RRs;
      iconified = xget(G->App, MUIA_Application_Iconified);
      if (iconified) { PopUp(); Delay(50L); }
      sprintf(buffer, GetStr(MSG_TR_NewMailReq),
         stats->Downloaded, stats->OnServer-stats->Deleted, stats->DupSkipped);
      sprintf(&buffer[strlen(buffer)], GetStr(MSG_TR_FilterStats),
         rr->Checked, rr->Bounced, rr->Forwarded, rr->Replied, rr->Executed, rr->Moved, rr->Deleted);
      InfoWindow(GetStr(MSG_TR_NewMail), buffer, GetStr(MSG_Okay), G->MA->GUI.WI);
   }
   if(hasCommandNotify(C->NotifyType)) ExecuteCommand(C->NotifyCommand, FALSE, OUT_DOS);
   if(hasSoundNotify(C->NotifyType))   PlaySound(C->NotifySound);
}

///
/// TR_ProcessGETFunc
/*** TR_ProcessGETFunc - Downloads messages from a POP3 server ***/
HOOKPROTONHNONP(TR_ProcessGETFunc, void)
{
  struct TransStat ts;
  TR_TransStat_Init(&ts);

  // make sure the NOOP timer is definitly stopped
  TC_Stop(TIO_POP3_KEEPALIVE);

  if(ts.Msgs_Tot)
  {
    struct MinNode *curNode;

    if(C->TransferWindow == 2 && !xget(G->TR->GUI.WI, MUIA_Window_Open))
      set(G->TR->GUI.WI, MUIA_Window_Open, TRUE);

    TR_TransStat_Start(&ts);

    for(curNode = G->TR->transferList.mlh_Head; curNode->mln_Succ && !G->TR->Abort && !G->Error; curNode = curNode->mln_Succ)
    {
      struct MailTransferNode *mtn = (struct MailTransferNode *)curNode;
      struct Mail *mail = mtn->mail;

      TR_TransStat_NextMsg(&ts, mtn->index, mtn->position, mail->Size, GetStr(MSG_TR_Downloading));
      if(hasTR_LOAD(mtn))
      {
        if(TR_LoadMessage(&ts, mtn->index))
        {
          // put the transferStat to 100%
          TR_TransStat_Update(&ts, TS_SETMAX);

          G->TR->Stats.Downloaded++;
          if(C->AvoidDuplicates)
            TR_AppendUIDL(mail->UIDL);

          if(hasTR_DELETE(mtn))
            TR_DeleteMessage(mtn->index);
        }
      }
      else if(hasTR_DELETE(mtn))
        TR_DeleteMessage(mtn->index);
    }

    DisplayStatistics((struct Folder *)-1, TRUE);
  }

  TR_GetMailFromNextPOP(FALSE, 0, 0);
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

   // first we have to set the notifies to the default values.
   // this is needed so that if we get mail from more than one POP3 at a line this
   // abort stuff works out
   set(G->TR->GUI.BT_PAUSE, MUIA_Disabled, FALSE);
   set(G->TR->GUI.BT_RESUME, MUIA_Disabled, TRUE);
   DoMethod(tr->GUI.BT_START, MUIM_KillNotify, MUIA_Pressed);
   DoMethod(tr->GUI.BT_START, MUIM_Notify, MUIA_Pressed, FALSE, MUIV_Notify_Application, 3, MUIM_WriteLong, TRUE, &(tr->Start));
   DoMethod(tr->GUI.BT_QUIT , MUIM_KillNotify, MUIA_Pressed);
   DoMethod(tr->GUI.BT_QUIT , MUIM_Notify, MUIA_Pressed, FALSE, MUIV_Notify_Application, 3, MUIM_WriteLong, TRUE, &(tr->Abort));

   if(C->PreSelection < 3)
   {
      struct MinNode *curNode = tr->GMD_Mail;

      for(; curNode->mln_Succ && !tr->Abort && !G->Error; curNode = curNode->mln_Succ)
      {
        struct MailTransferNode *mtn = (struct MailTransferNode *)curNode;

        if(tr->Pause)
          break;

        if(tr->Start)
        {
          TR_ProcessGETFunc();
          break;
        }

        if(C->PreSelection != 1 || mtn->mail->Size >= C->WarnSize*1024)
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

   if(tr->Abort)
     TR_AbortGETFunc();
   else
   {
     // start the timer which makes sure we send
     // a regular NOOP command to the POP3 server
     // so that it doesn't drop the connection
     TC_Restart(TIO_POP3_KEEPALIVE, C->KeepAliveInterval, 0);
   }
}
///
/// TR_PauseFunc
//  Pauses or resumes message download
HOOKPROTONHNO(TR_PauseFunc, void, int *arg)
{
  BOOL pause = *arg;

  set(G->TR->GUI.BT_RESUME, MUIA_Disabled, !pause);
  set(G->TR->GUI.BT_PAUSE,  MUIA_Disabled, pause);
  if(pause)
  {
    // start the timer which makes sure we send
    // a regular NOOP command to the POP3 server
    // so that it doesn't drop the connection
    TC_Restart(TIO_POP3_KEEPALIVE, C->KeepAliveInterval, 0);
    return;
  }

  G->TR->Pause = FALSE;
  TR_CompleteMsgList();
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

    sprintf(array[0] = dispsta, "%3d ", entry->index);
    if(hasTR_LOAD(entry))   strcat(dispsta, SICON_DOWNLOAD);
    if(hasTR_DELETE(entry)) strcat(dispsta, SICON_DELETE);
    if(mail->Size >= C->WarnSize<<10) strcat(dispsiz, MUIX_PH);
    FormatSize(mail->Size, array[1] = dispsiz);
    array[2] = dispfro;
    MyStrCpy(dispfro, AddrName((*pe)));
    array[3] = mail->Subject;
    array[4] = dispdate;
    *dispdate = '\0';

    if(mail->Date.ds_Days)
      DateStamp2String(dispdate, &mail->Date, C->SwatchBeat ? DSS_DATEBEAT : DSS_DATETIME, TZC_LOCAL);
  }
  else
  {
    array[0] = GetStr(MSG_MA_TitleStatus);
    array[1] = GetStr(MSG_Size);
    array[2] = GetStr(MSG_From);
    array[3] = GetStr(MSG_Subject);
    array[4] = GetStr(MSG_Date);
  }

  return 0;
}
MakeStaticHook(TR_LV_DspFuncHook,TR_LV_DspFunc);
///
/// TR_New
//  Creates transfer window
struct TR_ClassData *TR_New(enum TransferType TRmode)
{
   struct TR_ClassData *data = calloc(1, sizeof(struct TR_ClassData));
   if (data)
   {
      Object *bt_all = NULL, *bt_none = NULL, *bt_loadonly = NULL, *bt_loaddel = NULL, *bt_delonly = NULL, *bt_leave = NULL;
      Object *gr_sel, *gr_proc, *gr_win;
      BOOL fullwin = (TRmode == TR_GET || TRmode == TR_IMPORT);

      NewList((struct List *)&data->transferList);

      gr_proc = ColGroup(2), GroupFrameT(GetStr(MSG_TR_Status)),
         Child, data->GUI.TX_STATS = TextObject,
            MUIA_Text_Contents, GetStr(MSG_TR_TransferStats0),
            MUIA_Background,MUII_TextBack,
            MUIA_Frame     ,MUIV_Frame_Text,
            MUIA_Text_PreParse, MUIX_C,
         End,
         Child, VGroup,
            Child, data->GUI.GA_COUNT = GaugeObject,
               GaugeFrame,
               MUIA_Gauge_Horiz   ,TRUE,
               MUIA_Gauge_InfoText,GetStr(MSG_TR_MessageGauge0),
            End,
            Child, data->GUI.GA_BYTES = GaugeObject,
               GaugeFrame,
               MUIA_Gauge_Horiz   ,TRUE,
               MUIA_Gauge_InfoText,GetStr(MSG_TR_BytesGauge0),
            End,
         End,
         Child, data->GUI.TX_STATUS = TextObject,
            MUIA_Background,MUII_TextBack,
            MUIA_Frame     ,MUIV_Frame_Text,
         End,
         Child, data->GUI.BT_ABORT = MakeButton(GetStr(MSG_TR_Abort)),
      End;
      if (fullwin)
      {
         data->GUI.GR_LIST = VGroup, GroupFrameT(TRmode==TR_IMPORT ? GetStr(MSG_TR_MsgInFile) : GetStr(MSG_TR_MsgOnServer)),
            MUIA_ShowMe, TRmode==TR_IMPORT || C->PreSelection>=2,
            Child, NListviewObject,
               MUIA_CycleChain,1,
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
         gr_sel = VGroup, GroupFrameT(GetStr(MSG_TR_Control)),
            Child, ColGroup(5),
               Child, bt_all = MakeButton(GetStr(MSG_TR_All)),
               Child, bt_loaddel = MakeButton(GetStr(MSG_TR_DownloadDelete)),
               Child, bt_leave = MakeButton(GetStr(MSG_TR_Leave)),
               Child, HSpace(0),
               Child, data->GUI.BT_PAUSE = MakeButton(GetStr(MSG_TR_Pause)),
               Child, bt_none = MakeButton(GetStr(MSG_TR_Clear)),
               Child, bt_loadonly = MakeButton(GetStr(MSG_TR_DownloadOnly)),
               Child, bt_delonly = MakeButton(GetStr(MSG_TR_DeleteOnly)),
               Child, HSpace(0),
               Child, data->GUI.BT_RESUME = MakeButton(GetStr(MSG_TR_Resume)),
            End,
            Child, ColGroup(2),
               Child, data->GUI.BT_START = MakeButton(GetStr(MSG_TR_Start)),
               Child, data->GUI.BT_QUIT = MakeButton(GetStr(MSG_TR_Abort)),
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
         MUIA_Window_Activate, (TRmode == TR_IMPORT || TRmode == TR_EXPORT),
         MUIA_HelpNode, "TR_W",
         WindowContents, gr_win,
      End;

      if (data->GUI.WI)
      {
         DoMethod(G->App, OM_ADDMEMBER, data->GUI.WI);
         SetHelp(data->GUI.TX_STATUS,MSG_HELP_TR_TX_STATUS);
         SetHelp(data->GUI.BT_ABORT ,MSG_HELP_TR_BT_ABORT);
         if (fullwin)
         {
            set(data->GUI.BT_RESUME, MUIA_Disabled, TRUE);
            if (TRmode == TR_IMPORT)
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
         return data;
      }
      free(data);
   }
   return NULL;
}
///
