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

static void TR_NewMailAlert(void);
static void TR_CompleteMsgList(void);

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

#define POPCMD_WAITEOL 1
#define POPCMD_NOERROR 2

/**************************************************************************/
// TLS/SSL related variables

struct Library *AmiSSLBase = NULL;

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

	if (!(SSL_set_fd(ssl, G->TR_Socket)))
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

		DB(kprintf("Server public key is %d bits\n", EVP_PKEY_bits(X509_get_pubkey(server_cert)));)

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
  DB(kprintf("TR_EndTLS()!\n");)

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

/*** General connecting/disconnecting & transfer ***/
/// TR_IsOnline
//  Checks if there's an online connection
BOOL TR_IsOnline(void)
{
   struct Library *socketbase;
   BOOL isonline = FALSE;

   if (C->IsOnlineCheck)
   {
      if (MiamiBase = OpenLibrary(MIAMINAME, 10))
      {
         isonline = MiamiIsOnline(*C->IOCInterface ? C->IOCInterface : NULL); CloseLibrary(MiamiBase);
         return isonline;
      }
      else if (GenesisBase = OpenLibrary("genesis.library", 1))
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
	  CloseLibrary(AmiSSLBase);
	  AmiSSLBase = NULL;
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
  if(G->TR_UseableTLS && SocketBase && !AmiSSLBase)
  {
    if((AmiSSLBase = OpenLibrary("amissl.library", 1L)))
    {
	    if(InitAmiSSL(AmiSSL_Version,     AmiSSL_CurrentVersion,
			  	          AmiSSL_Revision,    AmiSSL_CurrentRevision,
				  	        AmiSSL_SocketBase,  SocketBase,
					          /*	AmiSSL_VersionOverride, TRUE,*/ /* If you insist */
					          TAG_DONE) != 0)
      {
	  		CloseLibrary(AmiSSLBase);
		  	AmiSSLBase = NULL;
  		}
	  }

    if(!AmiSSLBase)
    {
      ER_NewError(GetStr(MSG_ER_INITAMISSL), NULL, NULL);

      G->TR_UseableTLS = G->TR_UseTLS = FALSE;
    }
	}

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
   }
}
///
/// TR_Connect
//  Connects to a internet service
static int TR_Connect(char *host, int port)
{
   struct hostent *hostaddr;

   if (!(hostaddr = GetHostByName((STRPTR)host))) return -1;
   G->TR_INetSocketAddr.sin_len = sizeof(G->TR_INetSocketAddr);
   G->TR_INetSocketAddr.sin_family = AF_INET;
   G->TR_INetSocketAddr.sin_port = port;
   G->TR_INetSocketAddr.sin_addr.s_addr = 0;
   memcpy(&G->TR_INetSocketAddr.sin_addr, hostaddr->h_addr, hostaddr->h_length);

   G->TR_Socket = Socket(hostaddr->h_addrtype, SOCK_STREAM, 0);
   if (G->TR_Socket == -1) {
      TR_Disconnect();
      return -2;
   }

   if (Connect(G->TR_Socket, (struct sockaddr *)&G->TR_INetSocketAddr, sizeof(G->TR_INetSocketAddr)) != -1) {
      return 0;
   }

   /* Preparation for non-blocking I/O */
   if (Errno() == EINPROGRESS) return 0;

   TR_Disconnect();
   return -3;
}
///
/// TR_RecvDat
//  Receives data from a TCP/IP connection
static int TR_RecvDat(char *recvdata)                   /* success? */
{
   int len;

   DoMethod(G->App,MUIM_Application_InputBuffered, TAG_DONE);
   if (G->TR_Socket == SMTP_NO_SOCKET) return 0;

   if(G->TR_UseTLS)
   {
      len = SSL_read(ssl, (STRPTR)recvdata, SIZE_LINE-1);
   }
   else
   {
      len = Recv(G->TR_Socket, (STRPTR)recvdata, SIZE_LINE-1, 0);
   }

   if (len <= 0) recvdata[0]=0;
   else recvdata[len] = '\0';
   if (G->TR_Debug) printf("SERVER: %s", recvdata);
   return len;
}
///
/// TR_SendDat
//  Sends data through a TCP/IP connection
static BOOL TR_SendDat(char *senddata)                  /* success? */
{
   DoMethod(G->App,MUIM_Application_InputBuffered, TAG_DONE);
   if (G->TR_Socket == SMTP_NO_SOCKET) return FALSE;
   if (!senddata) return TRUE;
   if (G->TR_Debug) printf("CLIENT: %s", senddata);

   if(G->TR_UseTLS)
   {
      if(SSL_write(ssl, (STRPTR)senddata, strlen(senddata)) != -1) return TRUE;
   }
   else
   {
     if (Send(G->TR_Socket, (STRPTR)senddata, strlen(senddata), 0) != -1) return TRUE;
   }

   return FALSE;
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
   if (path = strchr(url,'/')) *path++ = 0; else path = "";
   strcpy(host, noproxy ? url : C->ProxyServer);
   if (bufptr = strchr(host, ':')) { *bufptr++ = 0; hport = atoi(bufptr); }
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
      else if (port = strchr(url, ':'))
      {
         *port++ = 0;
         sprintf(buf, "GET http://%s:%s/%s HTTP/1.0\r\nHost: %s\r\n", url, port, path, url);
      }
      else sprintf(buf, "GET http://%s/%s HTTP/1.0\r\nHost: %s\r\n", url, path, url);
      sprintf(&buf[strlen(buf)], "From: %s\r\nUser-Agent: %s\r\n\r\n", BuildAddrName(C->EmailAddress, C->RealName), yamversion);
      if (TR_SendDat(buf))
      {
         len = TR_RecvDat(buf);
         if (atoi(&buf[9]) == 200)
         {
            if (bufptr = strstr(buf, "\r\n")) bufptr += 2;
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
               if ((len = TR_RecvDat(buf)) <= 0) break;
               bufptr = buf;
            }
            if (out = fopen(filename, "w"))
            {
               ++bufptr;
               fwrite(bufptr, len-(bufptr-buf), 1, out);
               while ((len = TR_RecvDat(buf)) > 0) fwrite(buf, len, 1, out);
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
/// TR_SendPopCmd
//  Sends a command to the POP3 server
static BOOL TR_SendPopCmd(char *buf, char *cmdtext, char *parmtext, int flags)
{
   char cmdbuf[SIZE_COMMAND];
   int len, ln;

   if (G->TR_Socket == SMTP_NO_SOCKET) return FALSE;
   if (!parmtext || !*parmtext) sprintf(cmdbuf, "%s\r\n", cmdtext);
   else sprintf(cmdbuf, "%s %s\r\n", cmdtext, parmtext);
   if (!TR_SendDat(cmdbuf)) return FALSE;
   len = TR_RecvDat(buf);
   if (len <= 0)  return FALSE;
   if (flags & POPCMD_WAITEOL)
   {
      while (buf[len-1] != '\n')
      {
         ln = TR_RecvDat(&buf[len]);
         if (ln > 0) len += ln; else return FALSE;
      }
   }
   if (!strncmp(buf, "-ERR", 4))
   {
      if (!(flags & POPCMD_NOERROR)) ER_NewError(GetStr(MSG_ER_BadResponse), cmdtext, buf);
      return FALSE;
   }
   return TRUE;
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

   strcpy(passwd, C->P3[pop]->Password);
   strcpy(host, C->P3[pop]->Server);

   if (C->TransferWindow == 2 || (C->TransferWindow == 1 && (guilevel == POP_START || guilevel == POP_USER)))
   {
      LONG wstate;

      get(G->TR->GUI.WI, MUIA_Window_Open, &wstate);            // avoid MUIA_Window_Open's side effect of
      if(!wstate) set(G->TR->GUI.WI, MUIA_Window_Open, TRUE);   // activating the window if it was already open
   }
   set(G->TR->GUI.TX_STATUS  , MUIA_Text_Contents,GetStr(MSG_TR_Connecting));

   // If the hostname has a explicit :xxxxx port statement at the end we
   // take this one, even if its not needed anymore.
   if (p = strchr(host, ':')) { *p = 0; port = atoi(++p); }

   TR_SetWinTitle(TRUE, host);

   if (err = TR_Connect(host, port))
   {
      if (guilevel == POP_USER) switch (err)
      {
         case -1: ER_NewError(GetStr(MSG_ER_UnknownPOP), C->P3[pop]->Server, NULL); break;
         default: ER_NewError(GetStr(MSG_ER_CantConnect), C->P3[pop]->Server, NULL);
      }
      return -1;
   }

   // If this connection should be a STLS like connection we have to get the welcome
   // message now and then send the STLS command to start TLS negotiation
   if(G->TR_UseableTLS && C->P3[pop]->SSLMode == P3SSL_STLS)
   {
      set(G->TR->GUI.TX_STATUS, MUIA_Text_Contents, GetStr(MSG_TR_WaitWelcome));

      if(TR_RecvDat(buf) <= 0) return -1;
      welcomemsg = StrBufCpy(NULL, buf);

      // in STLS mode we get a welcome message and we have to get it out first
      while (!strstr(buf, "\n"))
      {
        if(TR_RecvDat(buf) <= 0) return -1;
        welcomemsg = StrBufCat(welcomemsg, buf);
      }
      *buf = 0;

      // If the user selected STLS support we have to first send the command
      // to start TLS negotiation (RFC 2595)
      if(!TR_SendPopCmd(buf, "STLS", NULL, POPCMD_WAITEOL)) return -1;
   }

   // Here start the TLS/SSL Connection stuff
   if(G->TR_UseableTLS && C->P3[pop]->SSLMode != P3SSL_OFF)
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
        return -1;
     }
   }
   else G->TR_UseTLS = FALSE;

   // If this was a connection on a stunnel on port 995 or a non-ssl connection
   // we have to get the welcome message now
   if(!G->TR_UseableTLS || C->P3[pop]->SSLMode != P3SSL_STLS)
   {
      if(TR_RecvDat(buf) <= 0) return -1;
      welcomemsg = StrBufCpy(NULL, buf);

      while (!strstr(buf, "\n"))
      {
        if (TR_RecvDat(buf) <= 0) return -1;
        welcomemsg = StrBufCat(welcomemsg, buf);
      }
      *buf = 0;
   }

   if (!*passwd)
   {
      sprintf(buf, GetStr(MSG_TR_PopLoginReq), C->P3[pop]->User, host);
      if (!StringRequest(passwd, SIZE_PASSWORD, GetStr(MSG_TR_PopLogin), buf, GetStr(MSG_Okay), NULL, GetStr(MSG_Cancel), TRUE, G->TR->GUI.WI)) return -1;
   }

   if (C->P3[pop]->UseAPOP)
   {
      struct MD5Context context;
      UBYTE digest[16];
      int i, j;

      if (p = strchr(welcomemsg, '<'))
      {
         strcpy(buf, p);
         if (p = strchr(buf, '>')) p[1] = 0;
      }
      else ER_NewError(GetStr(MSG_ER_NoAPOP), NULL, NULL);
      strcat(buf, passwd);
      MD5Init(&context);
      MD5Update(&context, buf, strlen(buf));
      MD5Final(digest, &context);
      sprintf(buf, "%s ", C->P3[pop]->User);
      for (j=strlen(buf), i=0; i<16; j+=2, i++) sprintf(&buf[j], "%02x", digest[i]);
      buf[j] = 0;
      set(G->TR->GUI.TX_STATUS, MUIA_Text_Contents, GetStr(MSG_TR_SendAPOPLogin));
      if (!TR_SendPopCmd(buf, "APOP", buf, POPCMD_WAITEOL)) return -1;
   }
   else
   {
      set(G->TR->GUI.TX_STATUS, MUIA_Text_Contents, GetStr(MSG_TR_SendUserID));
      if (!TR_SendPopCmd(buf, "USER", C->P3[pop]->User, POPCMD_WAITEOL)) return -1;
      set(G->TR->GUI.TX_STATUS, MUIA_Text_Contents, GetStr(MSG_TR_SendPassword));
      if (!TR_SendPopCmd(buf, "PASS", passwd, POPCMD_WAITEOL)) return -1;
   }

   if(welcomemsg) FreeStrBuf(welcomemsg);

   set(G->TR->GUI.TX_STATUS, MUIA_Text_Contents, GetStr(MSG_TR_GetStats));
   if (!TR_SendPopCmd(buf, "STAT", NULL, POPCMD_WAITEOL)) return -1;
   sscanf(&buf[4], "%ld", &msgs);
   if (msgs) AppendLogVerbose(31, GetStr(MSG_LOG_ConnectPOP), C->P3[pop]->User, host, (void *)msgs, "");
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

   if (email = MA_ExamineMail((struct Folder *)-1, tfname, NULL, FALSE))
   {
      struct Mail *mail = malloc(sizeof(struct Mail));
      if (mail)
      {
        *mail = email->Mail;
        mail->Folder  = NULL;
        mail->Status  = 1;
        mail->Index   = ++(*count);
        mail->Size    = size;
        MA_FreeEMailStruct(email);
        MyAddTail(&(G->TR->List), mail);
      }
   }
}
///
/// TR_GetMessageList_IMPORT
//  Collects messages from a UUCP mailbox file
void TR_GetMessageList_IMPORT(FILE *fh)
{
   BOOL body = FALSE;
   int c = 0, size = 0;
   char buffer[SIZE_LINE], *ptr, *tfname = "yamIMP.tmp", fname[SIZE_PATHFILE];
   FILE *f = NULL;

   strmfp(fname, C->TempDir, tfname);
   G->TR->List = NULL;
   fseek(fh, 0, SEEK_SET);
   while (fgets(buffer, SIZE_LINE, fh))
   {
      if (f || body) size += strlen(buffer);
      if (ptr = strpbrk(buffer, "\r\n")) *ptr = 0;
      if (!f && !strncmp(buffer, "From ", 5))
      {
         if (body) { TR_AddMessageHeader(&c, size, tfname); DeleteFile(fname); }
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
   if (body) { TR_AddMessageHeader(&c, size, tfname); DeleteFile(fname); }
   TR_DisplayMailList(FALSE);
}
///
/// TR_GetMessageList_GET
//  Collects messages waiting on a POP3 server
static BOOL TR_GetMessageList_GET(void)
{
   char buf[SIZE_LINE];

   if (TR_SendPopCmd(buf, "LIST", NULL, 0))
   {
      int mode, l = 0, index, size;
      char line[SIZE_DEFAULT], *bufptr;
      BOOL done = FALSE;
      struct Mail *new;

      G->TR->List = NULL;
      if (bufptr = strstr(buf, "\r\n")) bufptr += 2;
      else if (TR_RecvDat(buf) > 0) if (bufptr = strstr(buf, "\r\n")) bufptr += 2;
      if (!bufptr) return FALSE;
      while (!G->Error)
      {
         for (; *bufptr; bufptr++)
         {
            if (*bufptr != '\r') if (l < SIZE_DEFAULT-1) line[l++] = *bufptr;
            if (*bufptr != '\n') continue;
            line[l] = 0; l = 0;
            if (line[0] == '.' && line[1] == '\n') { done = TRUE; break; }
            sscanf(line, "%ld %ld", &index, &size);
            if (index) if (new = calloc(1, sizeof(struct Mail)))
            {
               static const int mode2status[16] = { 1,1,3,3,1,1,3,3,0,1,0,3,0,1,0,3 };
               new->Index = index; new->Size = size; new->Folder = NULL;
               mode = (C->DownloadLarge ? 1 : 0) +
                      (C->P3[G->TR->POP_Nr]->DeleteOnServer ? 2 : 0) +
                      (G->TR->GUIlevel == POP_USER ? 4 : 0) +
                      ((C->WarnSize && new->Size >= (C->WarnSize<<10)) ? 8 : 0);
               new->Status = mode2status[mode];
               MyAddTail(&(G->TR->List), new);
            }
         }
         if (done) break;
         if (TR_RecvDat(buf) <= 0) break;
         bufptr = buf;
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
   if (fh = fopen(CreateFilename(".uidl"), "a"))
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
      if (text = calloc(size+1,1))
         if (fh = fopen(file, "r"))
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
   char buf[SIZE_LINE];

   if (TR_SendPopCmd(buf, "UIDL", NULL, POPCMD_NOERROR))
   {
      int num, l = 0;
      struct Mail *mail;
      char  uidl[SIZE_DEFAULT+SIZE_HOST], line[SIZE_DEFAULT], *bufptr;
      BOOL done = FALSE;

      if (bufptr = strstr(buf, "\r\n")) bufptr += 2;
      while (!G->Error)
      {
         for (; *bufptr; bufptr++)
         {
            if (*bufptr != '\r') if (l < SIZE_DEFAULT-1) line[l++] = *bufptr;
            if (*bufptr != '\n') continue;
            line[l] = 0; l = 0;
            if (line[0] == '.' && line[1] == '\n') { done = TRUE; break; }
            sscanf(line, "%ld %s", &num, uidl);
            strcat(uidl, "@"); strcat(uidl, C->P3[G->TR->POP_Nr]->Server);
            for (mail = G->TR->List; mail; mail = mail->Next)
               if (mail->Index == num) { mail->UIDL = AllocCopy(uidl, strlen(uidl)+1); break; }
         }
         if (done) break;
         if (TR_RecvDat(buf) <= 0) break;
         bufptr = buf;
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

   for (i = 0; i < G->TR->Scnt; i++) if (FI_DoComplexSearch(G->TR->Search[i], G->TR->Search[i]->Rule->Combine, G->TR->Search[i+MAXRU], mail))
   {
      struct Rule *rule = G->TR->Search[i]->Rule;
      if (rule->Actions &   8) if (*rule->ExecuteCmd) ExecuteCommand(rule->ExecuteCmd, FALSE, OUT_DOS);
      if (rule->Actions &  16) if (*rule->PlaySound) PlaySound(rule->PlaySound);
      if (rule->Actions &  64) mail->Status |= 2; else mail->Status &= ~2;
      if (rule->Actions & 128) mail->Status &= ~1; else mail->Status |= 1;
      return;
   }
}
///
/// TR_GetMessageDetails
//  Gets header from a message stored on the POP3 server
static void TR_GetMessageDetails(struct Mail *mail, int lline)
{
   if (!*mail->From.Address)
   {
      char buf[SIZE_LINE], cmdbuf[SIZE_SMALL], *tfname = "yamTOP.tmp";
      sprintf(cmdbuf, "%ld 1", mail->Index);
      if (TR_SendPopCmd(buf, "TOP", cmdbuf, POPCMD_NOERROR))
      {
         char fname[SIZE_PATHFILE];
         FILE *f;
         strmfp(fname, C->TempDir, tfname);
         if (f = fopen(fname, "w"))
         {
            struct ExtendedMail *email;
            int l = 0;
            char line[SIZE_LINE], *bufptr;
            BOOL done = FALSE;

            if (bufptr = strstr(buf, "\r\n")) bufptr += 2;
            while (!G->Error && !G->TR->Abort)
            {
               for (; *bufptr; bufptr++)
               {
                  if (*bufptr != '\r') line[l++] = *bufptr;
                  if (l == SIZE_LINE-1)
                  {
                     line[l] = 0; l = 0;
                     if (fputs(line, f) == EOF) { ER_NewError(GetStr(MSG_ER_ErrorWriteMailfile), fname, NULL); break; }
                  }
                  if (*bufptr != '\n') continue;
                  line[l] = 0; l = 0;
                  if (line[0] == '.') if (line[1] == '\n') { done = TRUE; break; }
                  if (fputs(line, f) == EOF) { ER_NewError(GetStr(MSG_ER_ErrorWriteMailfile), fname, NULL); break; }
               }
               if (done) break;
               if (TR_RecvDat(buf) <= 0) break;
               bufptr = buf;
            }
            fclose(f);
            if (email = MA_ExamineMail(NULL, tfname, NULL, TRUE))
            {
               mail->From    = email->Mail.From;
               mail->To      = email->Mail.To;
               mail->ReplyTo = email->Mail.ReplyTo;
               strcpy(mail->Subject, email->Mail.Subject);
               strcpy(mail->MailFile, email->Mail.MailFile);
               mail->Date = email->Mail.Date;
               if (lline == -1)
               {
                  char uidl[SIZE_DEFAULT+SIZE_HOST];
                  sprintf(uidl, "%s@%s", Trim(email->MsgID), C->P3[G->TR->POP_Nr]->Server);
                  mail->UIDL = AllocCopy(uidl, strlen(uidl)+1);
               }
               if (lline == -2) TR_ApplyRemoteFilters(mail);
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
      if (!G->TR->supportUIDL) TR_GetMessageDetails(mail, -1);
      if (TR_FindUIDL(mail->UIDL)) { G->TR->Stats.DupSkipped++; mail->Status &= 2; }
   }
}
///
/// TR_DisconnectPOP
//  Terminates a POP3 session
static void TR_DisconnectPOP(void)
{
   char buf[SIZE_LINE];

   set(G->TR->GUI.TX_STATUS, MUIA_Text_Contents, GetStr(MSG_TR_Disconnecting));
   if (!G->TR->Abort) TR_SendPopCmd(buf, "QUIT", NULL, POPCMD_WAITEOL);
   TR_Disconnect();
}

///
/// TR_GetMailFromNextPOP
//  Downloads and filters mail from a POP3 account
void TR_GetMailFromNextPOP(BOOL isfirst, int singlepop, int guilevel)
{
   struct Mail *mail;
   static int laststats;
	 static int msgsInCurrentFolder;
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
			msgsInCurrentFolder = FO_GetCurrentFolder()->Total;
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

   if (pop == MAXP3) /* Finish last connection */
   {
      TR_CloseTCPIP();
      set(G->TR->GUI.WI, MUIA_Window_Open, FALSE);
      MA_FreeRules(G->TR->Search, G->TR->Scnt);
      MA_StartMacro(MACRO_POSTGET, itoa(G->TR->Stats.Downloaded));

      DoMethod(G->App, MUIM_CallHook, &MA_ApplyRulesHook, APPLY_AUTO, 0, FALSE);

			// Now we jump to the first new mail we received if the number of messages has changed
			// after the mail transfer
			if(C->JumpToIncoming && msgsInCurrentFolder < FO_GetCurrentFolder()->Total) MA_JumpToNewMsg();

      G->TR->Checking = FALSE;
      DisplayStatistics((struct Folder *)-1, TRUE);
      TR_NewMailAlert();
      MA_ChangeTransfer(TRUE);

      DisposeModulePush(&G->TR);
      if (G->TR_Exchange)
      {
         G->TR_Exchange = FALSE;
         DoMethod(G->App, MUIM_Application_PushMethod, G->App, 3, MUIM_CallHook, &MA_SendHook, SEND_ALL);
      }

      return;
   }

   G->TR->POP_Nr = pop;
   G->TR_Allow = G->TR->Abort = G->Error = FALSE;
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
               if (C->WarnSize && C->PreSelection)                   // ...or any sort of preselection and there is a maximum size
                  for (mail = G->TR->List; mail; mail = mail->Next)  // ...and one of the messages is at least this big
                     if (mail->Size >= C->WarnSize<<10) { preselect = TRUE; break; }
            }
            if (preselect)                               // anything to preselect?
            {
               set(G->TR->GUI.WI, MUIA_Window_Open, TRUE);
               if (C->PreSelection == 1)
               {
                  TR_DisplayMailList(TRUE);                          // add entries to list
                  set(G->TR->GUI.GR_LIST, MUIA_ShowMe, TRUE);        // ...and show it
                  set(G->TR->GUI.WI, MUIA_Window_Activate, TRUE);    // activate window
                  DoMethod(G->TR->GUI.WI, MUIM_Window_ScreenToFront);
               }
               else TR_DisplayMailList(FALSE);
               set(G->TR->GUI.GR_PAGE, MUIA_Group_ActivePage, 0);
               G->TR->GMD_Mail = G->TR->List;
               G->TR->GMD_Line = 0;
               TR_CompleteMsgList();
               return;
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
//  Sends a command to the SMTP server
static BOOL TR_SendSMTPCmd(char *cmdtext, char *parmtext)
{
   int len;
   static char buffer[SIZE_LINE];
   char cont;

   if (G->TR_Socket == SMTP_NO_SOCKET) return FALSE;
   if (cmdtext) 
   {
      sprintf(buffer, "%s\r\n", cmdtext);
      if (parmtext) if (*parmtext) sprintf(buffer, "%s %s\r\n", cmdtext, parmtext);
   }
   else *buffer = 0;
   if (!TR_SendDat(buffer)) return FALSE;
   if ((len = TR_RecvDat(buffer)) <= 0) return FALSE;

   switch (atoi(buffer))
   {
      case 211: case 214: case 220: case 221: 
      case 250: case 251: case 354: break;
      default:  ER_NewError(GetStr(MSG_ER_BadResponse), cmdtext, buffer); return FALSE;
   }

   cont = buffer[3];
   while (buffer[len-1] != '\n') if ((len = TR_RecvDat(buffer)) <= 0) return FALSE;
   if (cont == '-') while (buffer[len-1] != '\n') if ((len = TR_RecvDat(buffer)) <= 0) break;

   return TRUE;
}
///
/// TR_ConnectSMTP
//  Connects to a SMTP mail server
static BOOL TR_ConnectSMTP(void)
{
  if(!G->TR_UseTLS)
  {
    set(G->TR->GUI.TX_STATUS, MUIA_Text_Contents, GetStr(MSG_TR_WaitWelcome));
    if (!TR_SendSMTPCmd(NULL, NULL)) return FALSE;
  }

  set(G->TR->GUI.TX_STATUS,MUIA_Text_Contents, GetStr(MSG_TR_SendHello));
  if (!TR_SendSMTPCmd("HELO", C->SMTP_Domain)) return FALSE;

  return TRUE;
}
///
/// TR_ConnectESMTP
//  Connects to a ESMTP mail server, checks some ESMTP features and sends SMTP AUTH
/* umsrfc ReadLine() */
/*
 * Read one line from client. The buffer may hold a maximum of "len"
 * characters including string terminator. The routine returns the return
 * code if available or 0 when the connection to the client was lost
 * during reading the line. The buffer will always contain a valid C
 * string after the call.
.*/
static int ReadLine(LONG Socket, char *buf, LONG len)
{
  char *p = buf;

  DoMethod(G->App,MUIM_Application_InputBuffered);
  if (G->TR_Socket == SMTP_NO_SOCKET) return FALSE;

  /* Is enough space left in buffer to read more characters? */
  while (--len > 0)
  {
    /* Yes, read one character from socket */
    if(G->TR_UseTLS)
    {
      if(SSL_read(ssl, p, 1) <= 0)
      {
        len = -1;
        break;
      }
    }
    else
    {
      if (Recv(Socket, p, 1, 0) <= 0)
      {
        /* Error or end of input reached */
        len = -1;

        /* Leave loop */
        break;
      }
    }

    /* End of line reached? */
    if (*p == '\n')
    {
      /* Yes, strip CR if line end was marked by CRLF */
      if ((p != buf) && (*(p - 1) == '\r')) p--;

      /* Leave loop */
      break;

    }
    else
    {
      /* No, normal character */
      p++;
    }
  }

  /* Add string terminator */
  *p = '\0';

  if (G->TR_Debug) printf("SERVER: %s\n", buf);

  return (len >= 0) ? strtol(buf, &p, 10) : 0;
}

#define SMTP_SERVICE_NOT_AVAILABLE 421
#define SMTP_ACTION_OK             250
#define AUTH_CRAM_MD5   1
#define AUTH_DIGEST_MD5 2
#define AUTH_LOGIN      4
#define AUTH_PLAIN      8

static BOOL TR_ConnectESMTP(void)
{
   int len,rc=0;
   char buffer[SIZE_LINE];
   char challenge[SIZE_LINE];
   UBYTE ESMTPAuth=0;
   int SMTPSocket=G->TR_Socket;

   if(!G->TR_UseTLS)
   {
      set(G->TR->GUI.TX_STATUS, MUIA_Text_Contents, GetStr(MSG_TR_WaitWelcome));
      if (!TR_SendSMTPCmd(NULL, NULL)) return FALSE;
   }

   set(G->TR->GUI.TX_STATUS,MUIA_Text_Contents, GetStr(MSG_TR_SendHello));

   if (G->TR_Socket == SMTP_NO_SOCKET) return FALSE;
   sprintf(buffer, "EHLO %s\r\n", C->SMTP_Domain);
   if (!TR_SendDat(buffer)) return FALSE;
   if(!(ReadLine(SMTPSocket, buffer, SIZE_LINE))) return FALSE;

   if(SMTP_ACTION_OK!=atoi(buffer))
   {
      ER_NewError(GetStr(MSG_ER_BadResponse), "EHLO", buffer);
      return FALSE;
   }

   while (buffer[3] == '-')
   {
      if(!(ReadLine(SMTPSocket, buffer, SIZE_LINE))) return FALSE;
      if (strnicmp(buffer+4, "AUTH", 4) == 0) /* SMTP AUTH */
      {
         if (NULL != strstr(buffer+9,"CRAM-MD5")) ESMTPAuth|=AUTH_CRAM_MD5;
         if (NULL != strstr(buffer+9,"DIGEST-MD5")) ESMTPAuth|=AUTH_DIGEST_MD5;
         if (NULL != strstr(buffer+9,"PLAIN")) ESMTPAuth|=AUTH_PLAIN;
         if (NULL != strstr(buffer+9,"LOGIN")) ESMTPAuth|=AUTH_LOGIN;
      }
   }

   if(0==ESMTPAuth)
   {
      ER_NewError(GetStr(MSG_ER_NO_SMTP_AUTH), C->SMTP_Server, NULL);
      return FALSE;
   }

   set(G->TR->GUI.TX_STATUS, MUIA_Text_Contents, GetStr(MSG_TR_SENDAUTH));

   if(ESMTPAuth & AUTH_CRAM_MD5) /* js_umsrfc SMTP AUTH */
   {
      /* Send AUTH command */
      if(!(TR_SendDat("AUTH CRAM-MD5\r\n"))) return FALSE;
  
      /* Get return code. */
      if(ReadLine(SMTPSocket, buffer, SIZE_LINE) == 334)
      {
        unsigned char digest[16];
        int i;
  
        strncpy(challenge, &buffer[4], 511);
        challenge[511]=0;
        decode64(challenge, challenge, challenge+strlen(challenge));
  
        hmac_md5(challenge, strlen(challenge), C->SMTP_AUTH_Pass, strlen(C->SMTP_AUTH_Pass), digest);

        len=sprintf(challenge,"%s ", C->SMTP_AUTH_User);

        for(i = 0; i < 16; ++i)
        {
          len += sprintf(challenge+len, "%02x", digest[i]);
        }
        challenge[len] = 0;
        challenge[len+1] = 0;
        challenge[len+2] = 0;
        encode64(challenge,buffer,len);
        strcat(buffer,"\r\n");
  
        /* Send AUTH response */
        if(!(TR_SendDat(buffer))) return FALSE;

        /* Get return code. */
        if(ReadLine(SMTPSocket, buffer, SIZE_LINE) != 235 )
        {
          ER_NewError(GetStr(MSG_ER_BadResponse), "AUTH CRAM-MD5", buffer);
          rc = SMTP_SERVICE_NOT_AVAILABLE;
        }
        else rc = SMTP_ACTION_OK;
      }
      else
      {
        ER_NewError(GetStr(MSG_ER_BadResponse), "AUTH CRAM-MD5", buffer);
        rc = SMTP_SERVICE_NOT_AVAILABLE;
      }
   }
   else if(ESMTPAuth & AUTH_DIGEST_MD5)
   {
      /* Send AUTH command */
      if(!(TR_SendDat("AUTH DIGEST-MD5\r\n"))) return FALSE;
  
      /* Get return code. */
      if(ReadLine(SMTPSocket, buffer, SIZE_LINE) == 334)
      {
        ULONG digest[4];
        struct MD5Context context;
  
        strncpy(challenge, &buffer[4], 511);
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
  
        /* Send AUTH response */
        if(!(TR_SendDat(buffer))) return FALSE;
  
        /* Get return code. */
        if(ReadLine(SMTPSocket, buffer, SIZE_LINE) != 235)
        {
          ER_NewError(GetStr(MSG_ER_BadResponse), "AUTH DIGEST-MD5", buffer);
          rc = SMTP_SERVICE_NOT_AVAILABLE;
        }
        else rc = SMTP_ACTION_OK;
      }
      else
      {
        ER_NewError(GetStr(MSG_ER_BadResponse), "AUTH DIGEST-MD5", buffer);
        rc = SMTP_SERVICE_NOT_AVAILABLE;
      }
   }
   else if(ESMTPAuth & AUTH_LOGIN)
   {
      /* Send AUTH command */
      if(!(TR_SendDat("AUTH LOGIN\r\n"))) return FALSE;

      /* Get return code */
      if(ReadLine(SMTPSocket, buffer, SIZE_LINE) == 334)
      {
         len=sprintf(challenge,"%s%c%c",C->SMTP_AUTH_User,0,0)-2;
         encode64(challenge,buffer,len);
         strcat(buffer,"\r\n");
         /* Send AUTH response Username: */
         if(!(TR_SendDat(buffer))) return FALSE;

         /* Get return code */
         if(ReadLine(SMTPSocket, buffer, SIZE_LINE) == 334)
         {
            len=sprintf(challenge,"%s%c%c",C->SMTP_AUTH_Pass,0,0)-2;
            encode64(challenge,buffer,len);
            strcat(buffer,"\r\n");
            /* Send AUTH response Password: */
            if(!(TR_SendDat(buffer))) return FALSE;

            /* Get return code */
            if(ReadLine(SMTPSocket, buffer, SIZE_LINE) != 235 )
            {
               ER_NewError(GetStr(MSG_ER_BadResponse), "AUTH LOGIN", buffer);
               rc = SMTP_SERVICE_NOT_AVAILABLE;
            }
            else rc = SMTP_ACTION_OK;
         }
         else
         {
            ER_NewError(GetStr(MSG_ER_BadResponse), "AUTH LOGIN", buffer);
            rc = SMTP_SERVICE_NOT_AVAILABLE;
         }
      }
      else
      {
         ER_NewError(GetStr(MSG_ER_BadResponse), "AUTH LOGIN", buffer);
         rc = SMTP_SERVICE_NOT_AVAILABLE;
      }
   }
   else if(ESMTPAuth & AUTH_PLAIN)
   {
      /* Send AUTH command */
      if(!(TR_SendDat("AUTH PLAIN\r\n"))) return FALSE;

      /* Get return code */
      if(ReadLine(SMTPSocket, buffer, SIZE_LINE) == 334)
      {
         len=0;
         challenge[len++]=0;
         len+=sprintf(challenge+len,"%s",C->SMTP_AUTH_User);
         len++;
         len+=sprintf(challenge+len,"%s",C->SMTP_AUTH_Pass);
         encode64(challenge,buffer,len);
         strcat(buffer,"\r\n");
         /* Send AUTH response */
         if(!(TR_SendDat(buffer))) return FALSE;

         /* Get return code */
         if(ReadLine(SMTPSocket, buffer, SIZE_LINE) != 235)
         {
            ER_NewError(GetStr(MSG_ER_BadResponse), "AUTH LOGIN", buffer);
            rc = SMTP_SERVICE_NOT_AVAILABLE;
         }
         else rc = SMTP_ACTION_OK;
      }
      else
      {
         ER_NewError(GetStr(MSG_ER_BadResponse), "AUTH LOGIN", buffer);
         rc = SMTP_SERVICE_NOT_AVAILABLE;
      }
   }

   return (BOOL)(rc == SMTP_ACTION_OK);
}
///
/// TR_DisconnectSMTP
//  Terminates a SMTP session
static void TR_DisconnectSMTP(void)
{
   set(G->TR->GUI.TX_STATUS, MUIA_Text_Contents, GetStr(MSG_TR_Disconnecting));
   if (!G->TR->Abort) TR_SendSMTPCmd("QUIT", NULL);
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
      if (mail->Status & 1) ts->Size_Tot += mail->Size;
   }
}
///
/// TR_TransStat_Start
//  Resets statistics display
static void TR_TransStat_Start(struct TransStat *ts)
{
   ts->Msgs_Done = ts->Size_Done = 0;
   SPrintF(G->TR->CountLabel, GetStr(MSG_TR_MessageGauge), "%ld", ts->Msgs_Tot);
   set(G->TR->GUI.GA_COUNT, MUIA_Gauge_InfoText, G->TR->CountLabel);
   set(G->TR->GUI.GA_COUNT, MUIA_Gauge_Max, ts->Msgs_Tot);
   ts->Clock_Start = TR_GetSeconds();
}
///
/// TR_TransStat_NextMsg
//  Updates statistics display for next message
static void TR_TransStat_NextMsg(struct TransStat *ts, int index, int listpos, int size, char *status)
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
   set(G->TR->GUI.GA_BYTES, MUIA_Gauge_InfoText, G->TR->BytesLabel);
   set(G->TR->GUI.GA_BYTES, MUIA_Gauge_Max, size);
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
//  Saves a list of messages to a UUCP mailbox file
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
         new->Index = ++c; new->Status = 1;
         MyAddTail(&(G->TR->List), new);
      }
   }
   if (c)
   {
      TR_SetWinTitle(FALSE, FilePart(fname));
      TR_TransStat_Init(&ts);
      TR_TransStat_Start(&ts);
      if (fh = fopen(fname, append ? "a" : "w"))
      {
         success = TRUE;
         for (mail = G->TR->List; mail && !G->TR->Abort; mail = mail->Next)
         {
            ts.Msgs_Done++;
            TR_TransStat_NextMsg(&ts, mail->Index, -1, mail->Size, GetStr(MSG_TR_Exporting));
            if (StartUnpack(GetMailFile(NULL, NULL, mail), fullfile, mail->Folder))
            {
               if (mfh = fopen(fullfile, "r"))
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

   if (f = fopen(mf = GetMailFile(NULL, outfolder, mail), "r"))
   {
      char buf[SIZE_LINE];
      sprintf(buf, "FROM:<%s>", C->EmailAddress);
      if (TR_SendSMTPCmd("MAIL", buf))
      {
         int j;
         BOOL rcptok = TRUE;
         struct ExtendedMail *email = MA_ExamineMail(outfolder, mail->MailFile, NULL, TRUE);
         sprintf(buf, "TO:<%s>", mail->To.Address);
         if (!TR_SendSMTPCmd("RCPT", buf)) rcptok = FALSE;
         for (j = 0; j < email->NoSTo && rcptok; j++)
         {
            sprintf(buf, "TO:<%s>", email->STo[j].Address);
            if (!TR_SendSMTPCmd("RCPT", buf)) rcptok = FALSE;
         }
         for (j = 0; j < email->NoCC && rcptok; j++)
         {
            sprintf(buf, "TO:<%s>", email->CC[j].Address);
            if (!TR_SendSMTPCmd("RCPT", buf)) rcptok = FALSE;
         }
         for (j = 0; j < email->NoBCC && rcptok; j++)
         {
            sprintf(buf, "TO:<%s>", email->BCC[j].Address);
            if (!TR_SendSMTPCmd("RCPT", buf)) rcptok = FALSE;
         }
         if (rcptok)
         {
            if (TR_SendSMTPCmd("DATA", NULL))
            {
               BOOL infield = FALSE, inbody = FALSE;
               while (fgets(buf, SIZE_LINE-1, f) && !G->TR->Abort && !G->Error)
               {
                  char *p, sendbuf[SIZE_LINE+2];
                  int sb = strlen(buf);
                  if (p = strpbrk(buf, "\r\n")) *p = 0;
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
                     if (!TR_SendDat(sendbuf)) ER_NewError(GetStr(MSG_ER_ConnectionBroken), NULL, NULL);
                  }
                  TR_TransStat_Update(ts, sb);
               }
               if (IoErr()) ER_NewError(GetStr(MSG_ER_ErrorReadMailfile), mf, NULL);
               else if (!G->TR->Abort && !G->Error)
               {
                  result = email->DelSend ? 2 : 1;
                  AppendLogVerbose(41, GetStr(MSG_LOG_SendingVerbose), AddrName(mail->To), mail->Subject, (void *)mail->Size, "");
               }
               TR_SendSMTPCmd("\r\n.", NULL);
            }
         }
         MA_FreeEMailStruct(email);
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
      if (mail->Status == STATUS_WFS || mail->Status == STATUS_ERR) if (new = malloc(sizeof(struct Mail)))
      {
         *new = *mail;
         new->Index = ++c;
         new->Status = 1;
         new->Reference = mail;
         new->Next = NULL;
         MyAddTail(&(G->TR->List), new);
      }
   }

   if (c)
   {
      char host[SIZE_HOST];
      G->TR->Scnt = MA_AllocRules(G->TR->Search, APPLY_SENT);
      TR_TransStat_Init(&ts);
      TR_TransStat_Start(&ts);
      strcpy(host, C->SMTP_Server);

      // If the hostname has a explicit :xxxxx port statement at the end we
      // take this one, even if its not needed anymore.
      if (p = strchr(host, ':')) { *p = 0; port = atoi(++p); }

      TR_SetWinTitle(FALSE, host);

      if (!(err = TR_Connect(host, port)))
      {
         BOOL connected;

         // Now we have to check whether the user has selected SSL/TLS
         // and then we have to initiate the STARTTLS command followed by the TLS negotiation
         if(G->TR_UseableTLS && C->Use_SMTP_TLS)
         {
           set(G->TR->GUI.TX_STATUS, MUIA_Text_Contents, GetStr(MSG_TR_WaitWelcome));
           if(!TR_SendSMTPCmd(NULL, NULL)) return FALSE; // This is needed to skip the welcome msg

           set(G->TR->GUI.TX_STATUS, MUIA_Text_Contents, GetStr(MSG_TR_INITTLS));

           // Now we initiate the STARTTLS command (RFC 2487)
           if(!TR_SendSMTPCmd("STARTTLS", NULL)) return FALSE;

           if(TR_InitTLS() && TR_StartTLS())
           {
              G->TR_UseTLS = TRUE;
           }
           else
           {
              ER_NewError(GetStr(MSG_ER_INITTLS), host, NULL);
              return FALSE;
           }
         }
         else G->TR_UseTLS = FALSE;

         if (C->Use_SMTP_AUTH && C->SMTP_AUTH_User[0]) connected=TR_ConnectESMTP();
         else connected=TR_ConnectSMTP();

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
                  case 0: MA_SetMailStatus(mail->Reference, STATUS_ERR);
                          TR_SendSMTPCmd("RSET", NULL);
                          break;
                  case 1: MA_SetMailStatus(mail->Reference, STATUS_SNT);
                          if (TR_ApplySentFilters(mail->Reference)) MA_MoveCopy(mail->Reference, outfolder, sentfolder, FALSE);
                          break;
                  case 2: MA_SetMailStatus(mail->Reference, STATUS_SNT);
                          if (TR_ApplySentFilters(mail->Reference)) MA_DeleteSingle(mail->Reference, FALSE);
               }
            }
            AppendLogNormal(40, GetStr(MSG_LOG_Sending), (void *)c, host, "", "");
         }
         TR_DisconnectSMTP();
      }
      else switch (err)
      {
         case -1: ER_NewError(GetStr(MSG_ER_UnknownSMTP), C->SMTP_Server, NULL); break;
         default: ER_NewError(GetStr(MSG_ER_CantConnect), C->SMTP_Server, NULL);
      }
      MA_FreeRules(G->TR->Search, G->TR->Scnt);
   }

   TR_AbortnClose();
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
//  Imports messages from a UUCP mailbox file
HOOKPROTONHNONP(TR_ProcessIMPORTFunc, void)
{
   struct TransStat ts;
   FILE *fh, *f = NULL;

   TR_TransStat_Init(&ts);
   if (ts.Msgs_Tot)
   {
      TR_TransStat_Start(&ts);
      if (fh = fopen(G->TR->ImportFile, "r"))
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
                     if (email = MA_ExamineMail(folder, mfile, stat, FALSE))
                     {
                        AddMailToList((struct Mail *)email, folder);
                        MA_FreeEMailStruct(email);
                     }
                  }
                  mail = mail->Next;
               }
               header = TRUE; body = FALSE;
               if (mail->Status & 1)
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
            if (email = MA_ExamineMail(folder, mfile, stat, FALSE))
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
   MA_FreeRules(G->TR->Search, G->TR->Scnt);
   TR_AbortnClose();
   TR_CloseTCPIP();
   G->TR->Checking = FALSE;

   DisplayStatistics((struct Folder *)-1, TRUE);
}
MakeStaticHook(TR_AbortGETHook, TR_AbortGETFunc);
///
/// TR_LoadMessage
//  Retrieves a message from the POP3 server
static BOOL TR_LoadMessage(struct TransStat *ts, int number)
{
   static char mfile[SIZE_MFILE];
   struct Folder *infolder = FO_GetFolderByType(FT_INCOMING, NULL);
   char msgnum[SIZE_SMALL], buf[SIZE_LINE], msgfile[SIZE_PATHFILE];
   FILE *f;

   MyStrCpy(msgfile, MA_NewMailFile(infolder, mfile, 0));
   if (f = fopen(msgfile, "w"))
   {
      sprintf(msgnum, "%ld", number);
      if (TR_SendPopCmd(buf, "RETR", msgnum, 0))
      {
         int l = 0;
         char line[SIZE_LINE], *bufptr;
         BOOL done = FALSE;

         if (bufptr = strstr(buf, "\r\n")) bufptr += 2;
         while (!G->Error && !G->TR->Abort)
         {
            for (; *bufptr; bufptr++)
            {
               if (*bufptr != '\r') line[l++] = *bufptr;
               if (l == SIZE_LINE-1)
               {
                  TR_TransStat_Update(ts, l);
                  line[l] = 0; l = 0;
                  if (fputs(line, f) == EOF) { ER_NewError(GetStr(MSG_ER_ErrorWriteMailfile), mfile, NULL); break; }
               }
               if (*bufptr != '\n') continue;
               TR_TransStat_Update(ts, l+1);
               line[l] = 0; l = 0;
               if (line[0] == '.')
                  if (line[1] == '\n') { done = TRUE; break; }
                  else l = 1;  /* RFC 1725 */
               if (fputs(&line[l], f) == EOF) { ER_NewError(GetStr(MSG_ER_ErrorWriteMailfile), mfile, NULL); break; }
               l = 0;
            }
            if (done) break;
            if (TR_RecvDat(buf) <= 0) break;
            bufptr = buf;
         }
      }
      else ER_NewError(GetStr(MSG_ER_ErrorWriteMailfile), (char *)"", NULL);
      fclose(f);
      if (!G->TR->Abort && !G->Error)
      {
         struct ExtendedMail *mail;
         if (mail = MA_ExamineMail(infolder, mfile, " ", FALSE))
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
   return FALSE;
}
///
/// TR_DeleteMessage
//  Deletes a message on the POP3 server
static void TR_DeleteMessage(int number)
{
   char msgnum[SIZE_SMALL], buf[SIZE_LINE];

   sprintf(msgnum, "%ld", number);
   set(G->TR->GUI.TX_STATUS, MUIA_Text_Contents, GetStr(MSG_TR_DeletingServerMail));
   if (TR_SendPopCmd(buf, "DELE", msgnum, POPCMD_WAITEOL)) G->TR->Stats.Deleted++;
}
///
/// TR_NewMailAlert
//  Notifies user when new mail is available
static void TR_NewMailAlert(void)
{
   struct DownloadResult *stats = &G->TR->Stats;

   memcpy(&G->LastDL, stats, sizeof(struct DownloadResult));
   if (!stats->Downloaded) return;
   if ((C->NotifyType & NOTI_REQ) && G->TR->GUIlevel != POP_REXX)
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
   if (C->NotifyType & NOTI_CMD)   ExecuteCommand(C->NotifyCommand, FALSE, OUT_DOS);
   if (C->NotifyType & NOTI_SOUND) PlaySound(C->NotifySound);
}

///
/// TR_ProccessGETFunc
/*** TR_ProcessGETFunc - Downloads messages from a POP3 server ***/
HOOKPROTONHNONP(TR_ProcessGETFunc, void)
{
   struct TransStat ts;
   struct Mail *mail;

   TR_TransStat_Init(&ts);
   if (ts.Msgs_Tot)
   {
      if (C->TransferWindow == 2) set(G->TR->GUI.WI, MUIA_Window_Open, TRUE);
      TR_TransStat_Start(&ts);
      for (mail = G->TR->List; mail && !G->TR->Abort && !G->Error; mail = mail->Next)
      {
         TR_TransStat_NextMsg(&ts, mail->Index, mail->Position, mail->Size, GetStr(MSG_TR_Downloading));
         if (mail->Status & 1)
         {
            if (TR_LoadMessage(&ts, mail->Index))
            {
               G->TR->Stats.Downloaded++;
               if (C->AvoidDuplicates) TR_AppendUIDL(mail->UIDL);
               if (mail->Status & 2) TR_DeleteMessage(mail->Index);
            }
         }
         else if (mail->Status & 2)
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

   if (C->PreSelection < 3) while (mail && !tr->Abort)
   {
      if (tr->Pause) return;
      if (tr->Start) { TR_ProcessGETFunc(); return; }
      if (C->PreSelection != 1 || mail->Size >= C->WarnSize*1024) TR_GetMessageDetails(mail, tr->GMD_Line++);
      mail = mail->Next;
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
      sprintf(array[0] = dispsta, "%3ld ", entry->Index);
      if (entry->Status & 1) strcat(dispsta, "\033o[10]");
      if (entry->Status & 2) strcat(dispsta, "\033o[9]");
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
      else gr_win = gr_proc;
      data->GUI.WI = WindowObject,
         MUIA_Window_ID, MAKE_ID('T','R','A','0'+TRmode-TR_IMPORT),
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
               DoMethod(data->GUI.BT_RESUME,MUIM_Notify, MUIA_Pressed, FALSE, MUIV_Notify_Application, 3, MUIM_CallHook, &TR_PauseHook,0);
               DoMethod(data->GUI.BT_PAUSE ,MUIM_Notify, MUIA_Pressed, FALSE, MUIV_Notify_Application, 3, MUIM_CallHook, &TR_PauseHook,1);
               DoMethod(data->GUI.BT_PAUSE, MUIM_Notify, MUIA_Pressed, FALSE, MUIV_Notify_Application, 3, MUIM_WriteLong, TRUE, &(data->Pause));
               DoMethod(data->GUI.LV_MAILS ,MUIM_Notify, MUIA_NList_DoubleClick,TRUE, MUIV_Notify_Application, 2, MUIM_CallHook, &TR_GetMessageInfoHook);
               DoMethod(bt_delonly,         MUIM_Notify, MUIA_Pressed, FALSE, MUIV_Notify_Application, 3, MUIM_CallHook, &TR_ChangeStatusHook,2);
               DoMethod(bt_loaddel,         MUIM_Notify, MUIA_Pressed, FALSE, MUIV_Notify_Application, 3, MUIM_CallHook, &TR_ChangeStatusHook,3);
               DoMethod(data->GUI.BT_START, MUIM_Notify, MUIA_Pressed, FALSE, MUIV_Notify_Application, 3, MUIM_WriteLong, TRUE, &(data->Start));
               DoMethod(data->GUI.BT_QUIT , MUIM_Notify, MUIA_Pressed, FALSE, MUIV_Notify_Application, 3, MUIM_WriteLong, TRUE, &(data->Abort));
            }
            DoMethod(bt_loadonly,        MUIM_Notify, MUIA_Pressed, FALSE, MUIV_Notify_Application, 3, MUIM_CallHook, &TR_ChangeStatusHook,1);
            DoMethod(bt_leave,           MUIM_Notify, MUIA_Pressed, FALSE, MUIV_Notify_Application, 3, MUIM_CallHook, &TR_ChangeStatusHook,0);
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
