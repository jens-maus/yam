/***************************************************************************

 YAM - Yet Another Mailer
 Copyright (C) 1995-2000 by Marcel Beck <mbeck@yam.ch>
 Copyright (C) 2000-2002 by YAM Open Source Team

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

#include <clib/alib_protos.h>
#include <clib/socket_protos.h>
#include <clib/macros.h>
#include <libraries/iffparse.h>
#include <mui/NList_mcc.h>
#include <mui/NListview_mcc.h>
#include <proto/dos.h>
#include <proto/exec.h>
#include <proto/genesis.h>
#include <proto/intuition.h>
#include <proto/miami.h>
#include <proto/muimaster.h>
#include <proto/utility.h>
#include <proto/amissl.h>

#include "extra.h"
#include "YAM.h"
#include "YAM_addressbookEntry.h"
#include "YAM_config.h"
#include "YAM_debug.h"
#include "YAM_error.h"
#include "YAM_find.h"
#include "YAM_folderconfig.h"
#include "YAM_global.h"
#include "YAM_hook.h"
#include "YAM_locale.h"
#include "YAM_main.h"
#include "YAM_mainFolder.h"
#include "YAM_md5.h"
#include "YAM_mime.h"

struct TransStat
{
   int  Msgs_Tot;
   int  Msgs_Done;
   long Size_Tot;
   long Size_Done;
   long Size_Curr;
   long Delay;
   long Clock_Start;
   long Clock_Last;
};

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
// flags for the Transfer preselection stuff
#define TRF_NONE              (0)
#define TRF_LOAD              (1<<0)
#define TRF_DELETE            (1<<1)
#define hasTR_LOAD(v)         (isFlagSet((v)->Status, TRF_LOAD))
#define hasTR_DELETE(v)       (isFlagSet((v)->Status, TRF_DELETE))

/**************************************************************************/
// static function prototypes
static void TR_NewMailAlert(void);
static void TR_CompleteMsgList(void);
static char *TR_SendPOP3Cmd(enum POPCommand command, char *parmtext, APTR errorMsg);
static char *TR_SendSMTPCmd(enum SMTPCommand command, char *parmtext, APTR errorMsg);
static int  TR_Recv(char *vptr, int maxlen);
static int  TR_ReadLine(LONG socket, char *vptr, int maxlen);
static int  TR_ReadBuffered(LONG socket, char *ptr, int maxlen, int flags);
static int  TR_Send(char *vptr, int flags);
static int  TR_WriteBuffered(LONG socket, char *ptr, int maxlen, int flags);

#define TR_WriteLine(buf)       (TR_Send((buf), TCPF_FLUSH))
#define TR_WriteFlush()         (TR_Send(NULL,  TCPF_FLUSHONLY))
#define TR_FreeTransBuffers()   { TR_ReadBuffered(0, NULL, 0, TCPF_FREEBUFFER);  \
                                  TR_WriteBuffered(0, NULL, 0, TCPF_FREEBUFFER); }

/**************************************************************************/
// TLS/SSL related variables

static SSL_METHOD *method;
static SSL_CTX *ctx;
static SSL *ssl;

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
  DB(kprintf("Seeding random number generator...\n");)
  sprintf(tmp, "%lx%lx", (unsigned long)time((time_t *)0), (unsigned long)FindTask(NULL));
  RAND_seed(tmp, strlen(tmp));

  if (!(method = SSLv23_client_method()))
  {
    DB(kprintf("SSLv23_client_method() error !\n");)
    return FALSE;
  }

  if (!(ctx = SSL_CTX_new(method)))
  {
    DB(kprintf("Can't create SSL_CTX object !\n");)
    return FALSE;
  }

  // In future we can give the user the ability to specify his own CA locations
  // in the application instead of using the default ones.
  if (CAfile || CApath)
  {
    DB(kprintf("CAfile = %s, CApath = %s\n", CAfile ? CAfile : "none", CApath ? CApath : "none");)
    if ((!SSL_CTX_load_verify_locations(ctx, CAfile, CApath)))
    {
      DB(kprintf("Error setting default verify locations !\n");)
      return FALSE;
    }
  }
  else
  {
    if((!SSL_CTX_set_default_verify_paths(ctx)))
    {
      DB(kprintf("Error setting default verify locations !\n");)
      return FALSE;
    }
  }

  if (!(SSL_CTX_set_cipher_list(ctx, "DEFAULT")))
  {
    DB(kprintf("SSL_CTX_set_cipher_list() error !\n");)
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
  DB(kprintf("Initializing TLS/SSL session...\n");)

  if (!(ssl = SSL_new(ctx)))
  {
    DB(kprintf("Can't create a new SSL structure for a connection !\n");)
    return FALSE;
  }

  if (!(SSL_set_fd(ssl, (int)G->TR_Socket)))
  {
    DB(kprintf("SSL_set_fd() error !\n");)
    return FALSE;
  }

  if ((SSL_connect(ssl)) <= 0)
  {
    DB(kprintf("TLS/SSL handshake error !\n");)
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
      DB(kprintf("%s connection using %s\n", SSL_CIPHER_get_version(cipher), SSL_get_cipher(ssl));)
    }

    if (!(server_cert = SSL_get_peer_certificate(ssl)))
    {
      DB(kprintf("SSL_get_peer_certificate() error !\n");)
    }

    DB(kprintf("Server public key is %ld bits\n", EVP_PKEY_bits(X509_get_pubkey(server_cert)));)

    #define X509BUFSIZE 4096

    x509buf = (char *)malloc(X509BUFSIZE);
    memset(x509buf, 0, X509BUFSIZE);

    DB(kprintf("Server certificate:\n");)

    if(!(X509_NAME_oneline(X509_get_subject_name(server_cert), x509buf, X509BUFSIZE)))
    {
      DB(kprintf("X509_NAME_oneline...[subject] error !\n");)
    }
    DB(kprintf("subject: %s\n", x509buf);)

    if(!(X509_NAME_oneline(X509_get_issuer_name(server_cert), x509buf, X509BUFSIZE)))
    {
      DB(kprintf("X509_NAME_oneline...[issuer] error !\n");)
    }
    DB(kprintf("issuer:  %s\n", x509buf);)

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
    ER_NewError(GetStr(MSG_ER_NOSTARTTLS), NULL, NULL);
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
    ER_NewError(GetStr(MSG_ER_INITTLS), C->SMTP_Server, NULL);
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
   int len, rc = SMTP_SERVICE_NOT_AVAILABLE;
   char *resp;
   char buffer[SIZE_LINE];
   char challenge[SIZE_LINE];

   set(G->TR->GUI.TX_STATUS, MUIA_Text_Contents, GetStr(MSG_TR_SENDAUTH));

   // first we check if the user has supplied the User&Password
   // and if not we return with an error
   if(!C->SMTP_AUTH_User[0] || !C->SMTP_AUTH_Pass[0])
   {
      ER_NewError(GetStr(MSG_ER_NOAUTHUSERPASS), NULL, NULL);
      return FALSE;
   }

   if(hasCRAM_MD5_Auth(ServerFlags)) // SMTP AUTH CRAM-MD5
   {
      // send the AUTH command and get the response back
      if((resp = TR_SendSMTPCmd(ESMTP_AUTH_CRAM_MD5, NULL, MSG_ER_BadResponse)))
      {
        ULONG digest[4]; // 16 chars
        char buf[512];

        char *login = C->SMTP_AUTH_User;
        char *password = C->SMTP_AUTH_Pass;

        // get the challenge code from the response line of the
        // AUTH command.
        strncpy(challenge, &resp[4], 511);
        challenge[511]=0;

        decode64(challenge, challenge, challenge+strlen(challenge));

        hmac_md5(challenge, strlen(challenge), password, strlen(password), (char *)digest);

        sprintf(buf, "%s %08lx%08lx%08lx%08lx%c%c", login, digest[0], digest[1], digest[2], digest[3], 0, 0);

        encode64(buf, buffer, strlen(buf));
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
   else if(hasDIGEST_MD5_Auth(ServerFlags)) // SMTP AUTH DIGEST-MD5
   {
      // send the AUTH command and get the response back
      if((resp = TR_SendSMTPCmd(ESMTP_AUTH_DIGEST_MD5, NULL, MSG_ER_BadResponse)))
      {
        ULONG digest[4];
        struct MD5Context context;

        strncpy(challenge, &resp[4], 511);
        challenge[511]=0;
        decode64(challenge, challenge, challenge+strlen(challenge));

        strcat(challenge, C->SMTP_AUTH_Pass);
        MD5Init(&context);
        MD5Update(&context, challenge, strlen(challenge));
        MD5Final((UBYTE *)digest, &context);

        len=sprintf(challenge,"%s %08lx%08lx%08lx%08lx%c%c",C->SMTP_AUTH_User,
                    digest[0],digest[1],digest[2],digest[3],0,0)-2;
        encode64(challenge,buffer,len);
        strcat(buffer,"\r\n");

        // now we send the SMTP AUTH response
        if(TR_WriteLine(buffer) <= 0) return FALSE;

        // get the server response and see if it was valid
        if(TR_ReadLine(G->TR_Socket, buffer, SIZE_LINE) <= 0 || (rc = getResponseCode(buffer)) != 235)
        {
          ER_NewError(GetStr(MSG_ER_BadResponse), (char *)SMTPcmd[ESMTP_AUTH_DIGEST_MD5], buffer);
        }
        else rc = SMTP_ACTION_OK;
      }
   }
   else if(hasLOGIN_Auth(ServerFlags))  // SMTP AUTH LOGIN
   {
      // send the AUTH command
      if(TR_SendSMTPCmd(ESMTP_AUTH_LOGIN, NULL, MSG_ER_BadResponse))
      {
         len=sprintf(challenge,"%s%c%c",C->SMTP_AUTH_User,0,0)-2;
         encode64(challenge,buffer,len);
         strcat(buffer,"\r\n");

         // now we send the SMTP AUTH response (UserName)
         if(TR_WriteLine(buffer) <= 0) return FALSE;

         // get the server response and see if it was valid
         if(TR_ReadLine(G->TR_Socket, buffer, SIZE_LINE) > 0 && (rc = getResponseCode(buffer)) == 334)
         {
            len=sprintf(challenge,"%s%c%c",C->SMTP_AUTH_Pass,0,0)-2;
            encode64(challenge,buffer,len);
            strcat(buffer,"\r\n");

            // now lets send the Password
            if(TR_WriteLine(buffer) <= 0) return FALSE;

            // get the server response and see if it was valid
            if(TR_ReadLine(G->TR_Socket, buffer, SIZE_LINE) > 0 && (rc = getResponseCode(buffer)) == 235 )
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
   else if(hasPLAIN_Auth(ServerFlags))  // SMTP AUTH PLAIN
   {
      // send the AUTH command
      if(TR_SendSMTPCmd(ESMTP_AUTH_PLAIN, NULL, MSG_ER_BadResponse))
      {
         len=0;
         challenge[len++]=0;
         len+=sprintf(challenge+len,"%s",C->SMTP_AUTH_User);
         len++;
         len+=sprintf(challenge+len,"%s",C->SMTP_AUTH_Pass);
         encode64(challenge,buffer,len);
         strcat(buffer,"\r\n");

         // now we send the SMTP AUTH response (UserName+Password)
         if(TR_WriteLine(buffer) <= 0) return FALSE;

         // get the server response and see if it was valid
         if(TR_ReadLine(G->TR_Socket, buffer, SIZE_LINE) <= 0 || (rc = getResponseCode(buffer)) != 235)
         {
            ER_NewError(GetStr(MSG_ER_BadResponse), (char *)SMTPcmd[ESMTP_AUTH_PLAIN], buffer);
         }
         else rc = SMTP_ACTION_OK;
      }
   }
   else
   {
      // if we don`t have any of the Authentication Flags turned on we have to
      // exit with an error
      ER_NewError(GetStr(MSG_ER_NO_SMTP_AUTH), C->SMTP_Server, NULL);
   }

   return (BOOL)(rc == SMTP_ACTION_OK);
}
///

/*** General connecting/disconnecting & transfer ***/
/// TR_IsOnline
//  Checks if there's an online connection
BOOL TR_IsOnline(void)
{
   struct Library *socketbase;
   BOOL isonline = FALSE;

   if (C->IsOnlineCheck)
   {
      if ((MiamiBase = OpenLibrary(MIAMINAME, 10)))
      {
         isonline = MiamiIsOnline(*C->IOCInterface ? C->IOCInterface : NULL); CloseLibrary(MiamiBase);
         return isonline;
      }
      else if ((GenesisBase = OpenLibrary("genesis.library", 1)))
      {
         isonline = IsOnline(*C->IOCInterface ? (long)C->IOCInterface : 0); CloseLibrary(GenesisBase);
         return isonline;
      }
   }
   if((socketbase = OpenLibrary("bsdsocket.library", 2L)))
   {
      isonline = TRUE;
      CloseLibrary(socketbase);
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
    CleanupAmiSSL(TAG_DONE);
  }

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
  if(!TR_IsOnline()) return FALSE;
  if(!SocketBase) SocketBase = OpenLibrary("bsdsocket.library", 2L);

  // Now we have to check for TLS/SSL support
  if(G->TR_UseableTLS && AmiSSLBase && SocketBase)
  {
    if(InitAmiSSL(AmiSSL_Version,     AmiSSL_CurrentVersion,
                  AmiSSL_Revision,    AmiSSL_CurrentRevision,
                  AmiSSL_SocketBase,  SocketBase,
                  /*AmiSSL_VersionOverride, TRUE,*/ /* If you insist */
                  TAG_DONE) != 0)
    {
      ER_NewError(GetStr(MSG_ER_INITAMISSL), NULL, NULL);
      G->TR_UseableTLS = G->TR_UseTLS = FALSE;
    }
  }
  else G->TR_UseTLS = FALSE;

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

      Shutdown(G->TR_Socket, 2);
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
  struct hostent *hostaddr;

  // get the hostent out of the supplied hostname
  if(!(hostaddr = GetHostByName((STRPTR)host)))
  {
    return -1;
  }

#ifdef DEBUG
  kprintf("Host %s :\n", host);
  kprintf("  Officially:\t%s\n", hostaddr->h_name);
  kprintf("  Aliases:\t");

  for(i=0; hostaddr->h_aliases[i]; ++i)
  {
    if(i) kprintf(", ");
    kprintf("%s", hostaddr->h_aliases[i]);
  }

  kprintf("\n  Type:\t\t%s\n", hostaddr->h_addrtype == AF_INET ? "AF_INET" : "AF_INET6");
  if(hostaddr->h_addrtype == AF_INET)
  {
    for(i=0; hostaddr->h_addr_list[i]; ++i)
    {
      kprintf("  Address:\t%s\n", Inet_NtoA(((struct in_addr *)hostaddr->h_addr_list[i])->s_addr));
    }
  }
#endif

  // lets create a standard AF_INET socket now
  G->TR_Socket = Socket(AF_INET, SOCK_STREAM, 0);
  if (G->TR_Socket == -1)
  {
    return -2;
  }

  // copy the hostaddr data in a local copy for further reference
  memset(&G->TR_INetSocketAddr, 0, sizeof(G->TR_INetSocketAddr));
  G->TR_INetSocketAddr.sin_len          = sizeof(G->TR_INetSocketAddr);
  G->TR_INetSocketAddr.sin_family       = AF_INET;
  G->TR_INetSocketAddr.sin_port         = htons(port);

  // now we try a connection for every address we have for this host
  // because a hostname can have more than one IP in h_addr_list[]
  for(i=0; hostaddr->h_addr_list[i]; i++)
  {
    memcpy(&G->TR_INetSocketAddr.sin_addr, hostaddr->h_addr_list[i], hostaddr->h_length);

    if(Connect(G->TR_Socket, (struct sockaddr *)&G->TR_INetSocketAddr, sizeof(G->TR_INetSocketAddr)) != -1)
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

   if (G->TR_Debug) printf("SERVER: %s", recvdata);

   return len;
}

///
/// TR_Send()
// a own wrapper function for Send()/SSL_write() that writes buffered somehow
// if called with flag != TCP_FLUSH - otherwise it will write and flush immediatly
static int TR_Send(char *ptr, int flags)
{
  int nwritten;

  DoMethod(G->App,MUIM_Application_InputBuffered);
  if (G->TR_Socket == SMTP_NO_SOCKET) return -1;
  if (G->TR_Debug && ptr) printf("CLIENT: %s", ptr);

  // we call the WriteBuffered() function to write this characters
  // out to the socket. the provided flag will define if it
  // really will be buffered or if we write and flush the buffer
  // immediatly
  nwritten = TR_WriteBuffered(G->TR_Socket, ptr, ptr ? strlen(ptr) : 0, flags);

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

  if(G->TR_Debug) printf("SERVER: %s", vptr);

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
  int fill_cnt = 0;

  // if we don`t have a buffer yet, lets allocate own
  if(!read_buf && !(read_buf = calloc(1, SIZE_BUFSIZE*sizeof(char)))) return -1;

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

      if(G->TR_UseTLS) read_cnt = SSL_read(ssl, read_buf, SIZE_BUFSIZE);
      else             read_cnt = Recv(socket, read_buf, SIZE_BUFSIZE, 0);

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
static int TR_WriteBuffered(LONG socket, char *ptr, int maxlen, int flags)
{
  static int write_cnt = 0;
  static char *write_buf = NULL;
  static char *write_ptr;
  int fill_cnt = 0;
  int nwritten;

  // if we don`t have a buffer yet, lets allocate own
  if(!write_buf)
  {
    if(!(write_buf = calloc(1, SIZE_BUFSIZE*sizeof(char)))) return -1;
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
  if(write_cnt >= SIZE_BUFSIZE || hasTCP_ONLYFLUSH(flags))
  {
    again:

    write_ptr = write_buf;

    // lets make the buffer empty
    while(write_cnt > 0)
    {
      if(G->TR_UseTLS)  nwritten = SSL_write(ssl, write_ptr, write_cnt);
      else              nwritten = Send(G->TR_Socket, write_ptr, (LONG)write_cnt, 0);

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
  if(write_cnt+maxlen > SIZE_BUFSIZE)
  {
    int fillable = SIZE_BUFSIZE-write_cnt;

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
      else              nwritten = Send(G->TR_Socket, write_ptr, (LONG)write_cnt, 0);

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
            else ER_NewError(GetStr(MSG_ER_CantCreateFile), filename, NULL);
         }
         else ER_NewError(GetStr(MSG_ER_DocNotFound), path, NULL);
      }
      else ER_NewError(GetStr(MSG_ER_SendHTTP), NULL, NULL);
      TR_Disconnect();
   }
   else ER_NewError(GetStr(MSG_ER_ConnectHTTP), host, NULL);
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

   // now the server should return with a return code
   if(TR_ReadLine(G->TR_Socket, buf, SIZE_LINE) <= 0) return NULL;

   // if the server answered with an error we return FALSE
   if(strncmp(buf, POP_RESP_ERROR, strlen(POP_RESP_ERROR)) == 0)
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

   // now we have to check wheter SSL/TLS is selected for that POP account,
   // but perhaps TLS is not working.
   if(C->P3[pop]->SSLMode != P3SSL_OFF && !G->TR_UseableTLS)
   {
      ER_NewError(GetStr(MSG_ER_UNUSABLEAMISSL), NULL, NULL);
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
         case -1: ER_NewError(GetStr(MSG_ER_UnknownPOP), C->P3[pop]->Server, NULL); break;
         default: ER_NewError(GetStr(MSG_ER_CantConnect), C->P3[pop]->Server, NULL);
      }
      BusyEnd;
      return -1;
   }

   // If this connection should be a STLS like connection we have to get the welcome
   // message now and then send the STLS command to start TLS negotiation
   if(C->P3[pop]->SSLMode == P3SSL_STLS)
   {
      set(G->TR->GUI.TX_STATUS, MUIA_Text_Contents, GetStr(MSG_TR_WaitWelcome));

      // Initiate a connect and see if we succeed
      if(!(resp = TR_SendPOP3Cmd(POPCMD_CONNECT, NULL, MSG_ER_POPWELCOME))) { BusyEnd; return -1; }
      welcomemsg = StrBufCpy(NULL, resp);

      // If the user selected STLS support we have to first send the command
      // to start TLS negotiation (RFC 2595)
      if(!TR_SendPOP3Cmd(POPCMD_STLS, NULL, MSG_ER_BadResponse)) { BusyEnd; return -1; }
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
        ER_NewError(GetStr(MSG_ER_INITTLS), host, NULL);
        BusyEnd;
        return -1;
     }
   }

   // If this was a connection on a stunnel on port 995 or a non-ssl connection
   // we have to get the welcome message now
   if(C->P3[pop]->SSLMode != P3SSL_STLS)
   {
      // Initiate a connect and see if we succeed
      if(!(resp = TR_SendPOP3Cmd(POPCMD_CONNECT, NULL, MSG_ER_POPWELCOME))) { BusyEnd; return -1; }
      welcomemsg = StrBufCpy(NULL, resp);
   }

   if (!*passwd)
   {
      sprintf(buf, GetStr(MSG_TR_PopLoginReq), C->P3[pop]->User, host);
      if (!StringRequest(passwd, SIZE_PASSWORD, GetStr(MSG_TR_PopLogin), buf, GetStr(MSG_Okay), NULL, GetStr(MSG_Cancel), TRUE, G->TR->GUI.WI))
      {
        BusyEnd;
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
         MD5Update(&context, buf, strlen(buf));
         MD5Final(digest, &context);
         sprintf(buf, "%s ", C->P3[pop]->User);
         for(j=strlen(buf), i=0; i<16; j+=2, i++) sprintf(&buf[j], "%02x", digest[i]);
         buf[j] = 0;
         set(G->TR->GUI.TX_STATUS, MUIA_Text_Contents, GetStr(MSG_TR_SendAPOPLogin));
         if (!TR_SendPOP3Cmd(POPCMD_APOP, buf, MSG_ER_BadResponse)) { BusyEnd; return -1; }
      }
      else
      {
         ER_NewError(GetStr(MSG_ER_NoAPOP), NULL, NULL);
         BusyEnd;
         return -1;
      }
   }
   else
   {
      set(G->TR->GUI.TX_STATUS, MUIA_Text_Contents, GetStr(MSG_TR_SendUserID));
      if (!TR_SendPOP3Cmd(POPCMD_USER, C->P3[pop]->User, MSG_ER_BadResponse)) { BusyEnd; return -1; }
      set(G->TR->GUI.TX_STATUS, MUIA_Text_Contents, GetStr(MSG_TR_SendPassword));
      if (!TR_SendPOP3Cmd(POPCMD_PASS, passwd, MSG_ER_BadResponse)) { BusyEnd; return -1; }
   }

   if(welcomemsg) FreeStrBuf(welcomemsg);

   set(G->TR->GUI.TX_STATUS, MUIA_Text_Contents, GetStr(MSG_TR_GetStats));
   if (!(resp = TR_SendPOP3Cmd(POPCMD_STAT, NULL, MSG_ER_BadResponse))) { BusyEnd; return -1; }
   sscanf(&resp[4], "%d", &msgs);
   if (msgs) AppendLogVerbose(31, GetStr(MSG_LOG_ConnectPOP), C->P3[pop]->User, host, (void *)msgs, "");

   BusyEnd;
   return msgs;
}
///
/// TR_DisplayMailList
//  Displays a list of messages ready for download
static void TR_DisplayMailList(BOOL largeonly)
{
   struct Mail *mail;
   APTR lv = G->TR->GUI.LV_MAILS;
   int pos = 0;
   set(lv, MUIA_NList_Quiet, TRUE);
   for (mail = G->TR->List; mail; mail = mail->Next)
      if (mail->Size >= C->WarnSize<<10 || !largeonly)
      {
         mail->Position = pos++;
         DoMethod(lv, MUIM_NList_InsertSingle, mail, MUIV_NList_Insert_Bottom);
      }
   set(lv, MUIA_NList_Quiet, FALSE);
}
///
/// TR_AddMessageHeader
//  Parses downloaded message header
static void TR_AddMessageHeader(int *count, int size, char *tfname)
{
   struct ExtendedMail *email;

   if ((email = MA_ExamineMail((struct Folder *)-1, tfname, NULL, FALSE)))
   {
      struct Mail *mail = malloc(sizeof(struct Mail));
      if (mail)
      {
        *mail = email->Mail;
        mail->Folder  = NULL;
        mail->Status  = TRF_LOAD;
        mail->Index   = ++(*count);
        mail->Size    = size;
        MyAddTail(&(G->TR->List), mail);
      }
      MA_FreeEMailStruct(email);
   }
}
///
/// TR_GetMessageList_IMPORT
//  Collects messages from a MBOX mailbox file
BOOL TR_GetMessageList_IMPORT(FILE *fh)
{
   BOOL body = FALSE;
   int c = 0, size = 0;
   char *tfname = "yamIMP.tmp";
   char buffer[SIZE_LINE], *ptr;
   char fname[SIZE_PATHFILE];
   FILE *f = NULL;

   strmfp(fname, C->TempDir, tfname);
   G->TR->List = NULL;
   fseek(fh, 0, SEEK_SET);
   while (fgets(buffer, SIZE_LINE, fh))
   {
      if (f || body) size += strlen(buffer);
      if ((ptr = strpbrk(buffer, "\r\n"))) *ptr = 0;
      if (!f && !strncmp(buffer, "From ", 5))
      {
         if (body)
         {
            TR_AddMessageHeader(&c, size, tfname);
            DeleteFile(fname);
         }
         if (!(f = fopen(fname, "w"))) break;
         size = 0; body = FALSE;
      }
      if (f)
      {
         fputs(buffer, f); fputc('\n', f);
         if (!*buffer)
         { 
            fclose(f); f = NULL;
            body = TRUE;
         }
      }
   }

   if(body)
   {
      TR_AddMessageHeader(&c, size, tfname);
      DeleteFile(fname);
   }
   TR_DisplayMailList(FALSE);

   if(c > 0) return TRUE;

   return FALSE;
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

      G->TR->List = NULL;

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
            static const int mode2status[16] = { TRF_LOAD, TRF_LOAD, (TRF_LOAD&TRF_DELETE),
                                                 (TRF_LOAD|TRF_DELETE), TRF_LOAD, TRF_LOAD,
                                                 (TRF_LOAD|TRF_DELETE), (TRF_LOAD|TRF_DELETE),
                                                 TRF_NONE, TRF_LOAD, TRF_NONE, (TRF_LOAD|TRF_DELETE),
                                                 TRF_NONE, TRF_LOAD, TRF_NONE, (TRF_LOAD|TRF_DELETE)
                                               };

            newMail->Index = index;
            newMail->Size  = size;

            mode = (C->DownloadLarge ? 1 : 0) +
                   (C->P3[G->TR->POP_Nr]->DeleteOnServer ? 2 : 0) +
                   (G->TR->GUIlevel == POP_USER ? 4 : 0) +
                   ((C->WarnSize && newMail->Size >= (C->WarnSize<<10)) ? 8 : 0);

            newMail->Status = mode2status[mode];
            MyAddTail(&(G->TR->List), newMail);
         }

         // now read the next Line
         if(TR_ReadLine(G->TR_Socket, buf, SIZE_LINE) <= 0) return FALSE;
      }

      return TRUE;
   }
   else return FALSE;
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
         struct Mail *mail;

         // now parse the line and get the message number and UIDL
         sscanf(buf, "%d %s", &num, uidl);

         // lets add our own ident to the uidl so that we can compare
         // it against our saved list
         strcat(uidl, "@");
         strcat(uidl, C->P3[G->TR->POP_Nr]->Server);

         // iterate to the mail the UIDL is for and save it in the variable
         for(mail = G->TR->List; mail; mail = mail->Next)
         {
            if(mail->Index == num)
            {
               mail->UIDL = AllocCopy(uidl, strlen(uidl)+1);
               break;
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
static void TR_ApplyRemoteFilters(struct Mail *mail)
{
   int i;

   for (i = 0; i < G->TR->Scnt; i++)
   {
    if (FI_DoComplexSearch(G->TR->Search[i], G->TR->Search[i]->Rule->Combine, G->TR->Search[i+MAXRU], mail))
    {
      struct Rule *rule = G->TR->Search[i]->Rule;
      if(hasExecuteAction(rule) && *rule->ExecuteCmd) ExecuteCommand(rule->ExecuteCmd, FALSE, OUT_DOS);
      if(hasPlaySoundAction(rule) && *rule->PlaySound) PlaySound(rule->PlaySound);

      if(hasDeleteAction(rule)) SET_FLAG(mail->Status, TRF_DELETE);
      else CLEAR_FLAG(mail->Status, TRF_DELETE);

      if(hasSkipMsgAction(rule)) CLEAR_FLAG(mail->Status, TRF_LOAD);
      else SET_FLAG(mail->Status, TRF_LOAD);

      return;
    }
   }
}
///
/// TR_GetMessageDetails
//  Gets header from a message stored on the POP3 server
static void TR_GetMessageDetails(struct Mail *mail, int lline)
{
   if(!*mail->From.Address && !G->TR->Abort && !G->Error)
   {
      char cmdbuf[SIZE_SMALL];

      // we issue a TOP command with a one line message body.
      //
      // This command is optional within the RFC 1939 specification
      // and therefore we don`t throw any error
      sprintf(cmdbuf, "%d 1", mail->Index);
      if(TR_SendPOP3Cmd(POPCMD_TOP, cmdbuf, NULL))
      {
         char buf[SIZE_LINE];
         char *tfname = "yamTOP.msg";
         char fname[SIZE_PATHFILE];
         FILE *f;

         // we generate a temporary file to buffer the TOP list
         // into it.
         strmfp(fname, C->TempDir, tfname);
         if((f = fopen(fname, "w")))
         {
            struct ExtendedMail *email;
            int l = 0;
            char line[SIZE_LINE];
            char *bufptr;
            BOOL done = FALSE;

            // get the first data the pop server returns after the TOP command
            if(TR_Recv(buf, SIZE_LINE) <= 0) G->Error = TRUE;

            // we get the message until we reach the end or an error
            // or abort situation occurs.
            while(!G->Error && !G->TR->Abort)
            {
               // now we iterate through the received string
               // and strip out the '\r' character.
               // we iterate through it because the strings we receive
               // from the socket can be splitted somehow.
               for(bufptr = buf; *bufptr; bufptr++)
               {
                  // lets strip out any '\r'
                  if(*bufptr == '\r') continue;
                  
                  // write the actual char in the line buffer
                  line[l++] = *bufptr;

                  // if the line buffer is full lets write it to the file
                  if(l == SIZE_LINE-1)
                  {
                     // add the termination 0 to the linebuffer first and set the counter to zero
                     line[l] = '\0'; l = 0;

                     // write the line to the file now
                     if (fputs(line, f) == EOF) { ER_NewError(GetStr(MSG_ER_ErrorWriteMailfile), fname, NULL); break; }

                     continue; // then we continue to the next character in the buffer
                  }

                  // if we end up here, then the buffer was not full
                  // and we just have to check wheter this is the end of the line
                  if(*bufptr != '\n') continue;

                  // so, it`s the end of a line and now we check if this is the termination
                  // line .CRLF and if so we exit without writing it to the file
                  if(l == 2 && line[0] == '.' && line[1] == '\n') { done = TRUE; break; }

                  // if it`s not the termination line we can write it out to the file
                  line[l] = '\0'; l = 0;
                  if(fputs(line, f) == EOF) { ER_NewError(GetStr(MSG_ER_ErrorWriteMailfile), fname, NULL); break; }
               }

               // if we received the term octet we can exit the while loop now
               if (done) break;

               // if not, we get another bunch of data and start over again.
               if (TR_Recv(buf, SIZE_LINE) <= 0) break;
               bufptr = buf;
            }
            fclose(f);

            // If we end up here because of an error, abort or the upper loop wasn`t finished
            // we exit immediatly with deleting the temp file also.
            if(G->Error || G->TR->Abort || done == FALSE)
            {
               DeleteFile(fname);
               return;
            }

            if ((email = MA_ExamineMail(NULL, tfname, NULL, TRUE)))
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
               else if(lline == -2) TR_ApplyRemoteFilters(mail);

               DeleteFile(fname);
               MA_FreeEMailStruct(email);
            }
         }
         else ER_NewError(GetStr(MSG_ER_ErrorWriteMailfile), fname, NULL);
      }
   }

   if (lline >= 0) DoMethod(G->TR->GUI.LV_MAILS, MUIM_NList_Redraw, lline);
}
///
/// TR_GetUIDL
//  Filters out duplicate messages
static void TR_GetUIDL(void)
{
   struct Mail *mail;
   G->TR->supportUIDL = TR_GetUIDLonServer();
   G->TR->UIDLloc = TR_GetUIDLonDisk();
   for (mail = G->TR->List; mail; mail = mail->Next)
   {
      // if the server doesn`t support the UIDL command we
      // use the TOP command and generate our own UIDL within
      // the GetMessageDetails function
      if (!G->TR->supportUIDL) TR_GetMessageDetails(mail, -1);
      if (TR_FindUIDL(mail->UIDL)) { G->TR->Stats.DupSkipped++; MASK_FLAG(mail->Status, TRF_DELETE); }
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
   struct Mail *mail;
   static int laststats;
   int msgs, pop = singlepop;

   if (isfirst) /* Init first connection */
   {
      G->LastDL.Error = TRUE;
      if (!TR_OpenTCPIP()) { if (guilevel == POP_USER) ER_NewError(GetStr(MSG_ER_NoTCP), NULL, NULL); return; }
      if (!CO_IsValid()) { TR_CloseTCPIP(); return; }
      if (!(G->TR = TR_New(TR_GET))) { TR_CloseTCPIP(); return; }
      G->TR->Checking = TRUE;
      DisplayAppIconStatistics();
      G->TR->GUIlevel = guilevel;
      G->TR->Scnt = MA_AllocRules(G->TR->Search, APPLY_REMOTE);
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
      MA_FreeRules(G->TR->Search, G->TR->Scnt);
      MA_StartMacro(MACRO_POSTGET, itoa((int)G->TR->Stats.Downloaded));

      // tell the appicon that we are finished with checking mail
      // the apply rules or DisplayAppIconStatistics() function will refresh it later on
      G->TR->Checking = FALSE;

      // we only apply the filters if we downloaded something, or it`s wasted
      if(G->TR->Stats.Downloaded > 0)
      {
        DoMethod(G->App, MUIM_CallHook, &MA_ApplyRulesHook, APPLY_AUTO, 0, FALSE);

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
            if (G->TR->Scnt)                             // filter messages on server?
            {
               set(G->TR->GUI.TX_STATUS, MUIA_Text_Contents, GetStr(MSG_TR_ApplyFilters));
               for (mail = G->TR->List; mail; mail = mail->Next)
                  TR_GetMessageDetails(mail, -2);
            }
            if (C->AvoidDuplicates) TR_GetUIDL();        // read UIDL file to compare against already received messages
            if (G->TR->GUIlevel == POP_USER)             // manually initiated transfer
            {
               if (C->PreSelection >= 2) preselect = TRUE;           // preselect messages if preference is "always [sizes only]"
               else if (C->WarnSize && C->PreSelection)              // ...or any sort of preselection and there is a maximum size
                  for (mail = G->TR->List; mail; mail = mail->Next)  // ...and one of the messages is at least this big
                     if (mail->Size >= C->WarnSize<<10) { preselect = TRUE; break; }
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
               G->TR->GMD_Mail = G->TR->List;
               G->TR->GMD_Line = 0;
               TR_CompleteMsgList();
            }
            else
            {
               CallHookPkt(&TR_ProcessGETHook, 0, 0);
            }

            return;
         }
      }
   }
   else G->TR->Stats.Error = TRUE;

   TR_GetMailFromNextPOP(FALSE, 0, 0);
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
  // If we did a TLS negotitaion previously we don`t have to skip the
  // welcome message.
  if(!G->TR_UseTLS)
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

   // If we did a TLS negotitaion previously we don`t have to skip the
   // welcome message.
   if(!G->TR_UseTLS)
   {
      set(G->TR->GUI.TX_STATUS, MUIA_Text_Contents, GetStr(MSG_TR_WaitWelcome));
      if(!TR_SendSMTPCmd(SMTP_CONNECT, NULL, MSG_ER_BadResponse)) return 0;

      set(G->TR->GUI.TX_STATUS,MUIA_Text_Contents, GetStr(MSG_TR_SendHello));
   }

   // Now we send the EHLO command to get the list of features returned.
   if (G->TR_Socket == SMTP_NO_SOCKET) return 0;

   // Now send the EHLO ESMTP command to log in
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
      else if(strnicmp(resp+4, "SIZE", 4) == 0)         // STD:10 - SIZE declaration (RFC 1860)
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
   kprintf("ESMTP Server '%s' serves:\n", C->SMTP_Server);
   kprintf("  AUTH CRAM-MD5......: %ld\n", hasCRAM_MD5_Auth(ServerFlags));
   kprintf("  AUTH DIGEST-MD5....: %ld\n", hasDIGEST_MD5_Auth(ServerFlags));
   kprintf("  AUTH LOGIN.........: %ld\n", hasLOGIN_Auth(ServerFlags));
   kprintf("  AUTH PLAIN.........: %ld\n", hasPLAIN_Auth(ServerFlags));
   kprintf("  STARTTLS...........: %ld\n", hasSTARTTLS(ServerFlags));
   kprintf("  SIZE...............: %ld\n", hasSIZE(ServerFlags));
   kprintf("  PIPELINING.........: %ld\n", hasPIPELINING(ServerFlags));
   kprintf("  8BITMIME...........: %ld\n", has8BITMIME(ServerFlags));
   kprintf("  DSN................: %ld\n", hasDSN(ServerFlags));
   kprintf("  ETRN...............: %ld\n", hasETRN(ServerFlags));
   kprintf("  ENHANCEDSTATUSCODES: %ld\n", hasENHANCEDSTATUSCODES(ServerFlags));
   kprintf("  DELIVERBY..........: %ld\n", hasDELIVERBY(ServerFlags));
   kprintf("  HELP...............: %ld\n", hasHELP(ServerFlags));
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
/// TR_ChangeStatusFunc
//  Changes status of selected messages
HOOKPROTONHNO(TR_ChangeStatusFunc, void, int *arg)
{
   int id = MUIV_NList_NextSelected_Start;
   struct Mail *mail;
   for (;;)
   {
      DoMethod(G->TR->GUI.LV_MAILS, MUIM_NList_NextSelected, &id);
      if (id == MUIV_NList_NextSelected_End) break;
      DoMethod(G->TR->GUI.LV_MAILS, MUIM_NList_GetEntry, id, &mail);
      mail->Status = *arg;
      DoMethod(G->TR->GUI.LV_MAILS, MUIM_NList_Redraw, id);
   }
}
MakeStaticHook(TR_ChangeStatusHook, TR_ChangeStatusFunc);
///
/// TR_GetSeconds
//  Gets current date and time in seconds
static long TR_GetSeconds(void)
{
   struct DateStamp ds;
   DateStamp(&ds);
   return ((86400*ds.ds_Days) + (60*ds.ds_Minute) + (ds.ds_Tick/50));
}
///
/// TR_TransStat_Init
//  Initializes transfer statistics
static void TR_TransStat_Init(struct TransStat *ts)
{
   struct Mail *mail;

   ts->Msgs_Tot = ts->Size_Tot = 0;
   if (G->TR->GUI.GR_LIST)
   {
      set(G->TR->GUI.GR_PAGE, MUIA_Group_ActivePage, 1);
      DoMethod(G->TR->GUI.LV_MAILS, MUIM_NList_Select, MUIV_NList_Select_All, MUIV_NList_Select_Off, NULL);
   }
   for (mail = G->TR->List; mail; mail = mail->Next)
   {
      ts->Msgs_Tot++;
      if(hasTR_LOAD(mail)) ts->Size_Tot += mail->Size;
   }
}
///
/// TR_TransStat_Start
//  Resets statistics display
static void TR_TransStat_Start(struct TransStat *ts)
{
   ts->Msgs_Done = ts->Size_Done = 0;
   SPrintF(G->TR->CountLabel, GetStr(MSG_TR_MessageGauge), "%ld", ts->Msgs_Tot);

   SetAttrs(G->TR->GUI.GA_COUNT, MUIA_Gauge_InfoText, G->TR->CountLabel,
                                 MUIA_Gauge_Max, ts->Msgs_Tot,
                                 TAG_DONE);

   ts->Clock_Start = TR_GetSeconds();
}
///
/// TR_TransStat_NextMsg
//  Updates statistics display for next message
static void TR_TransStat_NextMsg(struct TransStat *ts, int index, int listpos, LONG size, char *status)
{
   ts->Size_Curr = 0;
   ts->Clock_Last = 0;
   ts->Delay = 0;
   if (!xget(G->TR->GUI.WI, MUIA_Window_Open)) return;
   else if (size <    2500) ts->Delay = 256;
   else if (size <   25000) ts->Delay = 512;
   else if (size <  250000) ts->Delay = 1024;
   else if (size < 2500000) ts->Delay = 2048;
   else                     ts->Delay = 4096;
   if (G->TR->GUI.GR_LIST && listpos >= 0) set(G->TR->GUI.LV_MAILS, MUIA_NList_Active, listpos);
   set(G->TR->GUI.TX_STATUS, MUIA_Text_Contents, status);
   set(G->TR->GUI.GA_COUNT, MUIA_Gauge_Current, index);

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
   long clock;
   int speed = 0, remclock = 0;
   static long size_done = 0;

   if (!ts->Size_Done) size_done = 0;
   ts->Size_Curr += size_incr;
   ts->Size_Done += size_incr;
   if (!ts->Delay) return;
   if (ts->Size_Done-size_done > ts->Delay)
   {
      set(G->TR->GUI.GA_BYTES, MUIA_Gauge_Current, ts->Size_Curr);
      DoMethod(G->App, MUIM_Application_InputBuffered);
      size_done = ts->Size_Done;
   }
   if ((clock = (TR_GetSeconds()-ts->Clock_Start)) == ts->Clock_Last) return;
   ts->Clock_Last = clock;
   if (clock) speed = ts->Size_Done/clock;
   if(speed)
   {
     if((remclock = ts->Size_Tot/speed-clock) < 0)
       remclock = 0;
   }
   SPrintF(G->TR->StatsLabel, GetStr(MSG_TR_TransferStats),
      ts->Size_Done>>10, ts->Size_Tot>>10, speed, remclock/60, remclock%60);
   set(G->TR->GUI.TX_STATS, MUIA_Text_Contents, G->TR->StatsLabel);
}
///
/// TR_Cleanup
//  Free temporary message and UIDL lists
void TR_Cleanup(void)
{
   struct Mail *work, *next;

   if (G->TR->GUI.LV_MAILS) DoMethod(G->TR->GUI.LV_MAILS, MUIM_NList_Clear);

   for (work = G->TR->List; work; work = next)
   {
      next = work->Next;

      if (work->UIDL) free(work->UIDL);
      free(work);

   }
   if (G->TR->UIDLloc) free(G->TR->UIDLloc);
   G->TR->UIDLloc = NULL;
   G->TR->List = NULL;
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
   int i;
   for (i = 0; i < G->TR->Scnt; i++)
      if (FI_DoComplexSearch(G->TR->Search[i], G->TR->Search[i]->Rule->Combine, G->TR->Search[i+MAXRU], mail))
         if (!MA_ExecuteRuleAction(G->TR->Search[i]->Rule, mail)) return FALSE;
   return TRUE;
}
///

/*** EXPORT ***/
/// TR_ProcessEXPORT
//  Saves a list of messages to a MBOX mailbox file
BOOL TR_ProcessEXPORT(char *fname, struct Mail **mlist, BOOL append)
{
   BOOL success = FALSE;
   struct TransStat ts;
   int i, c;
   char buf[SIZE_LINE], fullfile[SIZE_PATHFILE];
   FILE *fh, *mfh;
   struct Mail *mail;

   G->TR->List = NULL;
   for (c = i = 0; i < (int)*mlist; i++)
   {
      struct Mail *new = malloc(sizeof(struct Mail));
      if (new)
      {
         *new = *mlist[i+2];
         new->Index  = ++c;
         new->Status = TRF_LOAD;
         MyAddTail(&(G->TR->List), new);
      }
   }
   if (c)
   {
      TR_SetWinTitle(FALSE, FilePart(fname));
      TR_TransStat_Init(&ts);
      TR_TransStat_Start(&ts);
      if ((fh = fopen(fname, append ? "a" : "w")))
      {
         success = TRUE;
         for (mail = G->TR->List; mail && !G->TR->Abort; mail = mail->Next)
         {
            ts.Msgs_Done++;
            TR_TransStat_NextMsg(&ts, mail->Index, -1, mail->Size, GetStr(MSG_TR_Exporting));
            if (StartUnpack(GetMailFile(NULL, NULL, mail), fullfile, mail->Folder))
            {
               if ((mfh = fopen(fullfile, "r")))
               {
                  fprintf(fh, "From %s %s", mail->From.Address, DateStamp2String(&mail->Date, DSS_UNIXDATE));
                  while (fgets(buf, SIZE_LINE, mfh) && !G->TR->Abort)
                  {
                     if (!strncmp(buf, "From ", 5)) fputc('>', fh);
                     fputs(buf, fh);
                     TR_TransStat_Update(&ts, strlen(buf));
                  }
                  if (*buf) if (buf[strlen(buf)-1] != '\n') fputc('\n', fh);
                  fclose(mfh);

                  // put the transferStat to 100%
                  TR_TransStat_Update(&ts, (int)mail->Size);
               }
               FinishUnpack(fullfile);
            }
         }
      }
      fclose(fh);
      AppendLog(51, GetStr(MSG_LOG_Exporting), (void *)ts.Msgs_Done, G->TR->List->Folder->Name, fname, "");
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
         struct ExtendedMail *email = MA_ExamineMail(outfolder, mail->MailFile, NULL, TRUE);

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
                BOOL infield = FALSE, inbody = FALSE;
                while(!G->TR->Abort && !G->Error && fgets(buf, SIZE_LINE-1, f))
                {
                  char *p, sendbuf[SIZE_LINE+2];
                  int sb = strlen(buf);
                  if ((p = strpbrk(buf, "\r\n"))) *p = 0;
                  if (!*buf && !inbody)
                  {
                     inbody = TRUE; infield = FALSE;
                  }
                  if (!ISpace(*buf) && !inbody) infield = !strnicmp(buf, "bcc", 3) || !strnicmp(buf, "x-yam-", 6);
                  if (!infield)
                  {
                     *sendbuf = 0;
                     if (*buf == '.') strcat(sendbuf, "."); /* RFC 821 */
                     strcat(sendbuf, buf);
                     strcat(sendbuf, "\r\n");

                     // now lets send the data buffered to the socket.
                     // we will flush it later then.
                     if(TR_Send(sendbuf, TCPF_NONE) <= 0) ER_NewError(GetStr(MSG_ER_ConnectionBroken), NULL, NULL);
                  }
                  TR_TransStat_Update(ts, sb);
                }

                // if buf == NULL when we arrive here, then the fgets()
                // at the top exited with an error
                if(buf == NULL) { ER_NewError(GetStr(MSG_ER_ErrorReadMailfile), mf, NULL); result = -1; }
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
                    TR_TransStat_Update(ts, (int)mail->Size);

                    result = email->DelSend ? 2 : 1;
                    AppendLogVerbose(42, GetStr(MSG_LOG_SendingVerbose), AddrName(mail->To), mail->Subject, (void *)mail->Size, "");
                  }
                }

                if(G->TR->Abort || G->Error) result = -1; // signal the caller that we aborted within the DATA part
              }
            }
            MA_FreeEMailStruct(email);
          }
          else ER_NewError(GetStr(MSG_ER_CantOpenFile), mf, NULL);
      }
      fclose(f);
   }
   else ER_NewError(GetStr(MSG_ER_CantOpenFile), mf, NULL);

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
   struct Mail *mail, *new;
   struct Folder *outfolder = FO_GetFolderByType(FT_OUTGOING, NULL);
   struct Folder *sentfolder = FO_GetFolderByType(FT_SENT, NULL);
   BOOL success = FALSE;
   char *p;

   G->TR->List = NULL;
   G->TR_Allow = G->TR->Abort = G->Error = FALSE;
   for (c = i = 0; i < (int)*mlist; i++)
   {
      mail = mlist[i+2];
      if (mail->Status == STATUS_WFS || mail->Status == STATUS_ERR) if ((new = malloc(sizeof(struct Mail))))
      {
         *new = *mail;
         new->Index = ++c;
         new->Status = TRF_LOAD;
         new->Reference = mail;
         new->Next = NULL;
         MyAddTail(&(G->TR->List), new);
      }
   }

   if (c)
   {
      char host[SIZE_HOST];

      // now we have to check wheter SSL/TLS is selected for SMTP account,
      // but perhaps TLS is not working.
      if(C->Use_SMTP_TLS && !G->TR_UseableTLS)
      {
        ER_NewError(GetStr(MSG_ER_UNUSABLEAMISSL), NULL, NULL);
        return FALSE;
      }

      G->TR->Scnt = MA_AllocRules(G->TR->Search, APPLY_SENT);
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

         // first we have to check wheter the user requested some
         // feature that requires a ESMTP Server, and if so we connect via ESMTP
         if(C->Use_SMTP_AUTH || C->Use_SMTP_TLS)
         {
            int ServerFlags = TR_ConnectESMTP();

            // Now we have to check whether the user has selected SSL/TLS
            // and then we have to initiate the STARTTLS command followed by the TLS negotiation
            if(C->Use_SMTP_TLS)
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
         if (connected)
         {
            success = TRUE;
            AppendLogVerbose(41, GetStr(MSG_LOG_ConnectSMTP), host, "", "", "");

            for (mail = G->TR->List; mail; mail = mail->Next)
            {
               if (G->TR->Abort || G->Error) break;
               ts.Msgs_Done++;
               TR_TransStat_NextMsg(&ts, mail->Index, -1, mail->Size, GetStr(MSG_TR_Sending));

               switch (TR_SendMessage(&ts, mail))
               {
                  // -1 means that SendMessage was aborted within the
                  // DATA part and so we cannot issue a RSET command and have to abort
                  // immediatly by leaving the mailserver alone.
                  case -1:
                  {
                    MA_SetMailStatus(mail->Reference, STATUS_ERR);
                    G->Error = TRUE;
                  }
                  break;

                  // 0 means that a error occured before the DATA part and
                  // so we can abort the transaction cleanly by a RSET and QUIT
                  case 0:
                  {
                    MA_SetMailStatus(mail->Reference, STATUS_ERR);
                    TR_SendSMTPCmd(SMTP_RSET, NULL, MSG_ER_BadResponse);
                  }
                  break;

                  // 1 means we filter the mails and then copy/move the mail to the send folder
                  case 1:
                  {
                    MA_SetMailStatus(mail->Reference, STATUS_SNT);
                    if (TR_ApplySentFilters(mail->Reference)) MA_MoveCopy(mail->Reference, outfolder, sentfolder, FALSE);
                  }
                  break;

                  // 2 means we filter and delete afterwards
                  case 2:
                  {
                    MA_SetMailStatus(mail->Reference, STATUS_SNT);
                    if (TR_ApplySentFilters(mail->Reference)) MA_DeleteSingle(mail->Reference, FALSE);
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
          case -1: ER_NewError(GetStr(MSG_ER_UnknownSMTP), C->SMTP_Server, NULL); break;
          default: ER_NewError(GetStr(MSG_ER_CantConnect), C->SMTP_Server, NULL);
        }
      }

      MA_FreeRules(G->TR->Search, G->TR->Scnt);
   }

   TR_AbortnClose();

   BusyEnd;
   return success;
}
///

/*** IMPORT ***/
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
   FILE *fh, *f = NULL;

   TR_TransStat_Init(&ts);
   if (ts.Msgs_Tot)
   {
      TR_TransStat_Start(&ts);
      if ((fh = fopen(G->TR->ImportFile, "r")))
      {
         struct ExtendedMail *email;
         struct Mail *mail = G->TR->List;
         static char mfile[SIZE_MFILE];
         BOOL header = FALSE, body = FALSE;
         struct Folder *folder = G->TR->ImportBox;
         int btype = folder->Type;
         char buffer[SIZE_LINE], *stat;

         if (btype == FT_OUTGOING) stat = Status[STATUS_WFS];
         else if (btype == FT_SENT || btype == FT_CUSTOMSENT) stat = Status[STATUS_SNT];
         else stat = " ";
         while (fgets(buffer, SIZE_LINE, fh) && !G->TR->Abort)
         {
            if (!header && !strncmp(buffer, "From ", 5))
            {
               if (body)
               {
                  if (f)
                  {
                     fclose(f); f = NULL;
                     if ((email = MA_ExamineMail(folder, mfile, stat, FALSE)))
                     {
                        AddMailToList((struct Mail *)email, folder);
                        MA_FreeEMailStruct(email);
                     }
                  }
                  mail = mail->Next;
               }
               header = TRUE; body = FALSE;
               if(hasTR_LOAD(mail))
               {
                  ts.Msgs_Done++;
                  TR_TransStat_NextMsg(&ts, mail->Index, mail->Position, mail->Size, GetStr(MSG_TR_Importing));
                  f = fopen(MA_NewMailFile(folder, mfile, 0), "w");
               }
            } 
            else if (f && (header || body))
            { 
               fputs(buffer, f);
               TR_TransStat_Update(&ts, strlen(buffer));
            }
            if (header && !buffer[1]) { body = TRUE; header = FALSE; }
         }
         if (body && f)
         {
            fclose(f);
            if ((email = MA_ExamineMail(folder, mfile, stat, FALSE)))
            {
               AddMailToList((struct Mail *)email, folder);
               MA_FreeEMailStruct(email);
            }
         }
         fclose(fh);
         DisplayMailList(folder, G->MA->GUI.NL_MAILS);
         AppendLog(50, GetStr(MSG_LOG_Importing), (void *)ts.Msgs_Done, G->TR->ImportFile, folder->Name, "");
         DisplayStatistics(folder, TRUE);
      }
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

   MyStrCpy(msgfile, MA_NewMailFile(infolder, mfile, 0));
   if ((f = fopen(msgfile, "w")))
   {
      BOOL done = FALSE;

      sprintf(msgnum, "%d", number);
      if(TR_SendPOP3Cmd(POPCMD_RETR, msgnum, MSG_ER_BadResponse))
      {
         int l = 0;
         char buf[SIZE_LINE];
         char line[SIZE_LINE], *bufptr;

         // get the first data the pop server returns after the TOP command
         if(TR_Recv(buf, SIZE_LINE) <= 0) return FALSE;

         while (!G->Error && !G->TR->Abort)
         {
            // now we iterate through the received string
            // and strip out the '\r' character.
            // we iterate through it because the strings we receive
            // from the socket can be splitted somehow.
            for(bufptr = buf; *bufptr; bufptr++)
            {
               // lets strip out any '\r'
               if(*bufptr == '\r') continue;

               // write the actual char in the line buffer
               line[l++] = *bufptr;

               // if the line buffer is full lets write it to the file
               if(l == SIZE_LINE-1)
               {
                  // update the transferstatus
                  TR_TransStat_Update(ts, l+1);

                  // add the termination 0 to the linebuffer first and set the counter to zero
                  line[l] = '\0'; l = 0;

                  // write the line to the file now
                  if (fputs(line, f) == EOF) { ER_NewError(GetStr(MSG_ER_ErrorWriteMailfile), mfile, NULL); break; }

                  continue; // then we continue to the next character in the buffer
               }

               // if we end up here, then the buffer was not full
               // and we just have to check wheter this is the end of the line
               if(*bufptr != '\n') continue;

               // lets add the null-termination to the line
               line[l] = '\0';

               // update the transferstatus
               TR_TransStat_Update(ts, l+1);

               // so, it`s the end of a line and now we check if this is the termination
               // line .CRLF and if so we exit without writing it to the file
               if(line[0] == '.')
               {
                  if (line[1] == '\n') { done = TRUE; break; }
                  else l = 1; // (RFC 1939) - the server handles "." as "..", so we only write "."
               }
               else l = 0;

               // if it`s not the termination line we can write it out to the file
               if(fputs(&line[l], f) == EOF) { ER_NewError(GetStr(MSG_ER_ErrorWriteMailfile), mfile, NULL); break; }
               l = 0;
            }

            // if we received the term octet we can exit the while loop now
            if (done) break;

            // if not, we get another bunch of data and start over again.
            if (TR_Recv(buf, SIZE_LINE) <= 0) break;
            bufptr = buf;
         }
      }
      fclose(f);

      if(!G->TR->Abort && !G->Error && done)
      {
         struct ExtendedMail *mail;
         if ((mail = MA_ExamineMail(infolder, mfile, " ", FALSE)))
         {
            struct Mail *new = AddMailToList((struct Mail *)mail, infolder);
            if (FO_GetCurrentFolder() == infolder) DoMethod(G->MA->GUI.NL_MAILS, MUIM_NList_InsertSingle, new, MUIV_NList_Insert_Sorted);
            AppendLogVerbose(32, GetStr(MSG_LOG_RetrievingVerbose), AddrName(new->From), new->Subject, (void *)new->Size, "");
            MA_StartMacro(MACRO_NEWMSG, mfile);
            MA_FreeEMailStruct(mail);
         }
         return TRUE;
      }
      DeleteFile(msgfile);
   }
   else ER_NewError(GetStr(MSG_ER_ErrorWriteMailfile), mfile, NULL);

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

   memcpy(&G->LastDL, stats, sizeof(struct DownloadResult));
   if (!stats->Downloaded) return;
   if(hasRequesterNotify(C->NotifyType) && G->TR->GUIlevel != POP_REXX)
   {
      int iconified;
      static char buffer[SIZE_LARGE];
      struct RuleResult *rr = &G->RRs;
      get(G->App, MUIA_Application_Iconified, &iconified);
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
   struct Mail *mail;

   TR_TransStat_Init(&ts);
   if (ts.Msgs_Tot)
   {
      if (C->TransferWindow == 2 && !xget(G->TR->GUI.WI, MUIA_Window_Open)) set(G->TR->GUI.WI, MUIA_Window_Open, TRUE);
      TR_TransStat_Start(&ts);
      for (mail = G->TR->List; mail && !G->TR->Abort && !G->Error; mail = mail->Next)
      {
         TR_TransStat_NextMsg(&ts, mail->Index, mail->Position, mail->Size, GetStr(MSG_TR_Downloading));
         if(hasTR_LOAD(mail))
         {
            if (TR_LoadMessage(&ts, mail->Index))
            {
               // put the transferStat to 100%
               TR_TransStat_Update(&ts, (int)mail->Size);

               G->TR->Stats.Downloaded++;
               if (C->AvoidDuplicates) TR_AppendUIDL(mail->UIDL);
               if(hasTR_DELETE(mail)) TR_DeleteMessage(mail->Index);
            }
         }
         else if(hasTR_DELETE(mail))
         {
            TR_DeleteMessage(mail->Index);
         }
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
   struct Mail *mail;
   get(G->TR->GUI.LV_MAILS, MUIA_NList_Active, &line);
   DoMethod(G->TR->GUI.LV_MAILS, MUIM_NList_GetEntry, line, &mail);
   TR_GetMessageDetails(mail, line);
}
MakeStaticHook(TR_GetMessageInfoHook, TR_GetMessageInfoFunc);
///
/// TR_CompleteMsgList
//  Gets details for messages on server
static void TR_CompleteMsgList(void)
{
   struct TR_ClassData *tr = G->TR;
   struct Mail *mail = tr->GMD_Mail;

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
      while(mail && !tr->Abort)
      {
        if(tr->Pause) break;
        if(tr->Start) { TR_ProcessGETFunc(); break; }
        if(C->PreSelection != 1 || mail->Size >= C->WarnSize*1024)
        {
          TR_GetMessageDetails(mail, tr->GMD_Line++);

          // set the next mail as the active one for the display,
          // so that if the user pauses we can go on here
          tr->GMD_Mail = mail->Next;
        }
        mail = mail->Next;
      }
   }

   set(G->TR->GUI.BT_PAUSE, MUIA_Disabled, TRUE);
   DoMethod(tr->GUI.BT_START, MUIM_KillNotify, MUIA_Pressed);
   DoMethod(tr->GUI.BT_START, MUIM_Notify, MUIA_Pressed, FALSE, MUIV_Notify_Application, 2, MUIM_CallHook, &TR_ProcessGETHook);
   DoMethod(tr->GUI.BT_QUIT , MUIM_KillNotify, MUIA_Pressed);
   DoMethod(tr->GUI.BT_QUIT , MUIM_Notify, MUIA_Pressed, FALSE, MUIV_Notify_Application, 2, MUIM_CallHook, &TR_AbortGETHook);
   if (tr->Abort) TR_AbortGETFunc();
}
///
/// TR_PauseFunc
//  Pauses or resumes message download
HOOKPROTONHNO(TR_PauseFunc, void, int *arg)
{
   BOOL pause = *arg;

   set(G->TR->GUI.BT_RESUME, MUIA_Disabled, !pause);
   set(G->TR->GUI.BT_PAUSE,  MUIA_Disabled, pause);
   if (pause) return;
   G->TR->Pause = FALSE;
   TR_CompleteMsgList();
}
MakeStaticHook(TR_PauseHook, TR_PauseFunc);
///

/*** GUI ***/
/// TR_LV_DspFunc
//  Message listview display hook
HOOKPROTO(TR_LV_DspFunc, long, char **array, struct Mail *entry)
{
   if (entry)
   {
      static char dispfro[SIZE_DEFAULT], dispsta[SIZE_DEFAULT], dispsiz[SIZE_SMALL], dispdate[32];
      struct Person *pe = &entry->From;
      sprintf(array[0] = dispsta, "%3d ", entry->Index);
      if(hasTR_LOAD(entry))   strcat(dispsta, "\033o[10]");
      if(hasTR_DELETE(entry)) strcat(dispsta, "\033o[9]");
      if (entry->Size >= C->WarnSize<<10) strcat(dispsiz, MUIX_PH);
      array[1] = dispsiz; *dispsiz = 0;
      FormatSize(entry->Size, dispsiz);
      array[2] = dispfro;
      MyStrCpy(dispfro, AddrName((*pe)));
      array[3] = entry->Subject;
      array[4] = dispdate; *dispdate = 0;
      if(entry->Date.ds_Days)
      {
        MyStrCpy(dispdate, DateStamp2String(&entry->Date, C->SwatchBeat ? DSS_DATEBEAT : DSS_DATETIME));
      }
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
      APTR bt_all = NULL, bt_none = NULL, bt_loadonly = NULL, bt_loaddel = NULL, bt_delonly = NULL, bt_leave = NULL;
      APTR gr_sel, gr_proc, gr_win;
      BOOL fullwin = (TRmode == TR_GET || TRmode == TR_IMPORT);

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
               MUIA_NListview_NList, data->GUI.LV_MAILS = NListObject,
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
				gr_win = 	VGroup,
										MUIA_Frame, MUIV_Frame_None,
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
               DoMethod(bt_delonly,         MUIM_Notify, MUIA_Pressed, FALSE, MUIV_Notify_Application, 3, MUIM_CallHook, &TR_ChangeStatusHook, TRF_DELETE);
               DoMethod(bt_loaddel,         MUIM_Notify, MUIA_Pressed, FALSE, MUIV_Notify_Application, 3, MUIM_CallHook, &TR_ChangeStatusHook, (TRF_LOAD|TRF_DELETE));
               DoMethod(data->GUI.BT_START, MUIM_Notify, MUIA_Pressed, FALSE, MUIV_Notify_Application, 3, MUIM_WriteLong, TRUE, &(data->Start));
               DoMethod(data->GUI.BT_QUIT , MUIM_Notify, MUIA_Pressed, FALSE, MUIV_Notify_Application, 3, MUIM_WriteLong, TRUE, &(data->Abort));
            }
            DoMethod(bt_loadonly,        MUIM_Notify, MUIA_Pressed, FALSE, MUIV_Notify_Application, 3, MUIM_CallHook, &TR_ChangeStatusHook, TRF_LOAD);
            DoMethod(bt_leave,           MUIM_Notify, MUIA_Pressed, FALSE, MUIV_Notify_Application, 3, MUIM_CallHook, &TR_ChangeStatusHook, TRF_NONE);
            DoMethod(bt_all,             MUIM_Notify, MUIA_Pressed, FALSE, data->GUI.LV_MAILS, 4, MUIM_NList_Select, MUIV_NList_Select_All, MUIV_NList_Select_On, NULL);
            DoMethod(bt_none,            MUIM_Notify, MUIA_Pressed, FALSE, data->GUI.LV_MAILS, 4, MUIM_NList_Select, MUIV_NList_Select_All, MUIV_NList_Select_Off, NULL);
            DoMethod(data->GUI.LV_MAILS, MUIM_NList_UseImage, G->MA->GUI.BC_STAT[9], 9, 0);
            DoMethod(data->GUI.LV_MAILS, MUIM_NList_UseImage, G->MA->GUI.BC_STAT[10], 10, 0);
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
