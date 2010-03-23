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

#include <proto/exec.h>
#include <proto/socket.h>
#include <proto/timer.h>
#include <proto/muimaster.h>

#if !defined(__amigaos4__) && !defined(__AROS__)
#include <proto/miami.h>
#include <proto/genesis.h>
#endif

#include <netinet/tcp.h>
#include <netinet/ip.h>

#include <libraries/iffparse.h>

#if defined(__AROS__)
#include <sys/types.h>
#include <sys/socket.h>
#include <bsdsocket/socketbasetags.h>
#include <netdb.h>
#include <sys/ioctl.h>
#else
#include <sys/filio.h>
#endif

#include <mui/NList_mcc.h>
#include <mui/NListview_mcc.h>

#include <SDI_hook.h>

#include "Transfer.h"

#include "YAM.h"
#include "YAM_addressbookEntry.h"
#include "YAM_config.h"
#include "YAM_error.h"
#include "YAM_find.h"
#include "YAM_global.h"
#include "YAM_mainFolder.h"

#include "extrasrc.h"

#include "mui/Classes.h"

#include "AppIcon.h"
#include "FileInfo.h"
#include "Locale.h"
#include "MailList.h"
#include "MUIObjects.h"
#include "Debug.h"

/**************************************************************************/
#ifndef SHUT_RDWR
#define SHUT_RDWR 2
#endif

/**************************************************************************/
// local prototypes
static int TR_ReadBuffered(struct TransferNode *tfn, char *ptr, const int maxlen, const int flags);
static int TR_WriteBuffered(struct TransferNode *tfn, const char *ptr, const int maxlen, const int flags);

/*** TransferNode ***/
/// CreateNewTransfer()
// Create a new TransferNode structure with default values
struct TransferNode *CreateNewTransfer(struct MailServerNode *msn)
{
  struct TransferNode *tfn = NULL;
  ENTER();

  if((tfn = (struct TransferNode *)calloc(1, sizeof(*tfn))) != NULL)
  {
    // clear the mail list per default
    NewMinList(&tfn->mailTransferList);

    // set the socket to not being connected.
    tfn->socket = TCP_NO_SOCKET;

    if(msn != NULL)
      tfn->msn = msn;
    else
      tfn->msn = CreateNewMailServer(MST_UNKNOWN, NULL, FALSE);

    if(tfn->msn == NULL)
    {
      free(tfn);
      tfn = NULL;
    }
  }

  RETURN(tfn);
  return tfn;
}

///
/// ProcessTransferQueue()
// function that walks through the G->transferQueue and processed
// the scheduled transfer jobs
void ProcessTransferQueue(enum MailServerType mst)
{
  ENTER();

  if(IsListEmpty((struct List *)&G->transferQueue) == FALSE)
  {
    // try to open the TCP/IP stack
    if(TR_OpenTCPIP() == TRUE)
    {
      // verify that the configuration is ready for sending mail
      if(CO_IsValid() == TRUE)
      {
        #warning "check auto/user defined TR_New() opening"
        if(SetupTransferWindow(TR_GET_USER) == TRUE)
        {
          struct Node *curNode;
          struct Node *nextNode;
          BOOL preselectMode = FALSE;
          int msgs;

          // reset some global variables
          G->Error = FALSE;

          // signal the AppIcon that we are checking for
          // new mails
          G->mailChecking = TRUE;
          UpdateAppIcon();

          // we walk through the TransferQueue and process each scheduled
          // mail transfer
          for(curNode = GetHead((struct List *)&G->transferQueue); curNode != NULL; curNode = nextNode)
          {
            struct TransferNode *tfn = (struct TransferNode *)curNode;

            // as we are going to remove the node eventually we have to
            // get the next node immediately.
            nextNode = GetSucc(curNode);

            if(mst == MST_UNKNOWN || (tfn->msn->type == mst && isServerActive(tfn->msn)))
            {
              BOOL transferSuccess = FALSE;

              // before we process the transfer we have to remove it from
              // the transferQueue
              Remove(curNode);

              // depending on the mail server type we have to either send mail
              // via SMTP or receive via POP3
              switch(mst)
              {
                case MST_POP3:
                {
                  transferSuccess = ProcessPOP3Transfer(tfn);
                }
                break;

                case MST_SMTP:
                {
                  transferSuccess = ProcessSMTPTransfer(tfn);
                }
                break;

                default:
                  // nothing to do
                break;
              }

              // free the transfer node
              free(tfn);
            }
          }

          // cleanup the transferwindow stuff
          CleanupTransferWindow();
        }
      }

      // make sure to close the TCP/IP
      // connection completly
      TR_CloseTCPIP();
    }
    else
      ER_NewError(tr(MSG_ER_OPENTCPIP));
  }

  LEAVE();
}

///

/*** TLS/SSL routines ***/
/// TR_InitTLS()
// Initialize the SSL/TLS session accordingly
BOOL TR_InitTLS(struct TransferNode *tfn)
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
    if((tfn->ssl_ctx = SSL_CTX_new(method)))
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
          E(DBF_NET, "Error setting default verify locations!");
          return FALSE;
        }
      }
      #endif

      // set the default SSL context verify pathes
      if(SSL_CTX_set_default_verify_paths(tfn->ssl_ctx))
      {
        // set the supported cipher list
        if((SSL_CTX_set_cipher_list(tfn->ssl_ctx, "DEFAULT")))
        {
          // for now we set the SSL certificate to 'NONE' as
          // YAM can currently not manage SSL server certificates itself
          // and therefore must be set to not verify them... unsafe?
          // well, I would call it "convienent" for the moment :)
          SSL_CTX_set_verify(tfn->ssl_ctx, SSL_VERIFY_NONE, 0);

          result = TRUE;
        }
        else
          E(DBF_NET, "SSL_CTX_set_cipher_list() error!");
      }
      else
        E(DBF_NET, "Error setting default verify locations!");
    }
    else
      E(DBF_NET, "Can't create SSL_CTX object!");
  }
  else
    E(DBF_NET, "SSLv23_client_method() error!");

  // if we weren't ale to
  // init the TLS/SSL stuff we have to clear it
  // before leaving
  if(result == FALSE)
  {
    if(tfn->ssl_ctx != NULL)
    {
      SSL_CTX_free(tfn->ssl_ctx);
      tfn->ssl_ctx = NULL;
    }

    tfn->ssl = NULL;
  }

  RETURN(result);
  return result;
}

///
/// TR_EndTLS()
// function that stops all TLS context
static void TR_EndTLS(struct TransferNode *tfn)
{
  ENTER();

  if(tfn->ssl)
  {
    SSL_shutdown(tfn->ssl);
    SSL_free(tfn->ssl);
    tfn->ssl = NULL;
  }

  if(tfn->ssl_ctx)
  {
    SSL_CTX_free(tfn->ssl_ctx);
    tfn->ssl_ctx = NULL;
  }

  LEAVE();
}

///
/// TR_StartTLS()
// function that starts & initializes the TLS/SSL session
BOOL TR_StartTLS(struct TransferNode *tfn)
{
  BOOL result = FALSE;

  ENTER();

  D(DBF_NET, "Initializing TLS/SSL session...");

  // check if we are ready for creating the ssl connection
  if(tfn->ssl_ctx != NULL && (tfn->ssl = SSL_new(tfn->ssl_ctx)))
  {
    // set the socket descriptor to the ssl context
    if(tfn->socket != TCP_NO_SOCKET &&
       SSL_set_fd(tfn->ssl, tfn->socket))
    {
      BOOL errorState = FALSE;
      int res;

      // establish the ssl connection and take care of non-blocking IO
      while(errorState == FALSE &&
            (res = SSL_connect(tfn->ssl)) <= 0)
      {
        int err = SSL_get_error(tfn->ssl, res);

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
                   (tfn->abort == TRUE))
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
              FD_SET(tfn->socket, &fdset);

              // depending on the SSL error (WANT_READ/WANT_WRITE)
              // we either do a WaitSelect() on the read or write mode
              // as with SSL both things can happen
              // see http://www.openssl.org/docs/ssl/SSL_connect.html
              if(err == SSL_ERROR_WANT_READ)
                retVal = WaitSelect(tfn->socket+1, &fdset, NULL, NULL, (APTR)&timeout, NULL);
              else
                retVal = WaitSelect(tfn->socket+1, NULL, &fdset, NULL, (APTR)&timeout, NULL);
            }
            while(retVal == 0);

            // if WaitSelect() returns 1 we successfully waited for
            // being able to write to the socket. So we go and do another
            // iteration in the while() loop as the next connect() call should
            // return EISCONN if the connection really succeeded.
            if(retVal >= 1 && FD_ISSET(tfn->socket, &fdset))
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
              E(DBF_NET, "WaitSelect() returned error %ld", err);
              errorState = TRUE;
            }
          }
          break;

          default:
          {
            E(DBF_NET, "SSL_connect() returned error %ld", err);
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
          cipher = SSL_get_current_cipher(tfn->ssl);

          if(cipher)
            D(DBF_NET, "%s connection using %s", SSL_CIPHER_get_version(cipher), SSL_get_cipher(tfn->ssl));

          if(!(server_cert = SSL_get_peer_certificate(tfn->ssl)))
            E(DBF_NET, "SSL_get_peer_certificate() error!");

          D(DBF_NET, "Server public key is %ld bits", EVP_PKEY_bits(X509_get_pubkey(server_cert)));

          #define X509BUFSIZE 4096

          x509buf = (char *)malloc(X509BUFSIZE);
          memset(x509buf, 0, X509BUFSIZE);

          D(DBF_NET, "Server certificate:");

          if(!(X509_NAME_oneline(X509_get_subject_name(server_cert), x509buf, X509BUFSIZE)))
            E(DBF_NET, "X509_NAME_oneline...[subject] error!");

          D(DBF_NET, "subject: %s", x509buf);

          if(!(X509_NAME_oneline(X509_get_issuer_name(server_cert), x509buf, X509BUFSIZE)))
            E(DBF_NET, "X509_NAME_oneline...[issuer] error!");

          D(DBF_NET, "issuer:  %s", x509buf);

          if(x509buf)
            free(x509buf);

          if(server_cert)
            X509_free(server_cert);
        }
        #endif
      }
    }
    else
      E(DBF_NET, "SSL_set_fd() error. TR_Socket: %ld", tfn->socket);
  }
  else
    E(DBF_NET, "Can't create a new SSL structure for a connection.");

  // if we didn't succeed with our SSL
  // startup we have to cleanup everything
  if(result == FALSE)
    TR_EndTLS(tfn);

  RETURN(result);
  return result;
}

///

/*** General connecting/disconnecting ***/
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
      //G->TR_UseTLS = FALSE;
      result = FALSE;
    }
  }
//  else
//    G->TR_UseTLS = FALSE;
#warning "FIXME: check TR_UseTLS"

  RETURN(result);
  return result;
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
/// TR_SetSocketOpts
//  Sets the user specified options for the active socket
static void TR_SetSocketOpts(struct TransferNode *tfn)
{
  ENTER();

  // disable CTRL-C checking
  #if !defined(__AROS__)
  SocketBaseTags(SBTM_SETVAL(SBTC_BREAKMASK), 0, TAG_END);
  #else
  {
    struct TagItem tags[] = { { SBTM_SETVAL(SBTC_BREAKMASK), 0 },
                              { TAG_END,                     0 } };

    SocketBaseTagList(tags);
  }
  #endif

  if(C->SocketOptions.KeepAlive)
  {
    int optval = C->SocketOptions.KeepAlive;

    if(setsockopt(tfn->socket, SOL_SOCKET, SO_KEEPALIVE, &optval, sizeof(optval)) == -1)
    {
      E(DBF_NET, "setsockopt(SO_KEEPALIVE) error");
      ER_NewError(tr(MSG_ER_SOCKETOPTION), "SO_KEEPALIVE");
    }
  }

  if(C->SocketOptions.NoDelay)
  {
    int optval = C->SocketOptions.NoDelay;

    if(setsockopt(tfn->socket, IPPROTO_TCP, TCP_NODELAY, &optval, sizeof(optval)) == -1)
    {
      E(DBF_NET, "setsockopt(TCP_NODELAY) error");
      ER_NewError(tr(MSG_ER_SOCKETOPTION), "TCP_NODELAY");
    }
  }

  if(C->SocketOptions.LowDelay)
  {
    int optval = IPTOS_LOWDELAY;

    if(setsockopt(tfn->socket, IPPROTO_IP, IP_TOS, &optval, sizeof(optval)) == -1)
    {
      E(DBF_NET, "setsockopt(IPTOS_LOWDELAY) error");
      ER_NewError(tr(MSG_ER_SOCKETOPTION), "IPTOS_LOWDELAY");
    }
  }

  if(C->SocketOptions.SendBuffer > -1)
  {
    int optval = C->SocketOptions.SendBuffer;

    if(setsockopt(tfn->socket, SOL_SOCKET, SO_SNDBUF, &optval, sizeof(optval)) == -1)
    {
      E(DBF_NET, "setsockopt(SO_SNDBUF) error");
      ER_NewError(tr(MSG_ER_SOCKETOPTION), "SO_SNDBUF");
    }
  }

  if(C->SocketOptions.RecvBuffer > -1)
  {
    int optval = C->SocketOptions.RecvBuffer;

    if(setsockopt(tfn->socket, SOL_SOCKET, SO_RCVBUF, &optval, sizeof(optval)) == -1)
    {
      E(DBF_NET, "setsockopt(SO_RCVBUF) error");
      ER_NewError(tr(MSG_ER_SOCKETOPTION), "SO_RCVBUF");
    }
  }

  if(C->SocketOptions.SendLowAt > -1)
  {
    int optval = C->SocketOptions.SendLowAt;

    if(setsockopt(tfn->socket, SOL_SOCKET, SO_SNDLOWAT, &optval, sizeof(optval)) == -1)
    {
      E(DBF_NET, "setsockopt(SO_SNDLOWAT) error");
      ER_NewError(tr(MSG_ER_SOCKETOPTION), "SO_SNDLOWAT");
    }
  }

  if(C->SocketOptions.RecvLowAt > -1)
  {
    int optval = C->SocketOptions.RecvLowAt;

    if(setsockopt(tfn->socket, SOL_SOCKET, SO_RCVLOWAT, &optval, sizeof(optval)) == -1)
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

    if(setsockopt(tfn->socket, SOL_SOCKET, SO_SNDTIMEO, &tv, sizeof(struct TimeVal)) == -1)
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

    if(setsockopt(tfn->socket, SOL_SOCKET, SO_RCVTIMEO, &tv, sizeof(struct TimeVal)) == -1)
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

    D(DBF_NET, "Opened socket: %lx", tfn->socket);

    // the value of the length pointer must be updated ahead of each call, because
    // getsockopt() might have modified it.
    optlen = sizeof(optval);
    getsockopt(tfn->socket, SOL_SOCKET, SO_KEEPALIVE, &optval, &optlen);
    D(DBF_NET, "SO_KEEPALIVE..: %ld", optval);

    optlen = sizeof(optval);
    getsockopt(tfn->socket, IPPROTO_TCP, TCP_NODELAY, &optval, &optlen);
    D(DBF_NET, "TCP_NODELAY...: %ld", optval);

    optlen = sizeof(optval);
    getsockopt(tfn->socket, IPPROTO_IP, IP_TOS, &optval, &optlen);
    D(DBF_NET, "IPTOS_LOWDELAY: %ld", hasFlag(optval, IPTOS_LOWDELAY));

    optlen = sizeof(optval);
    getsockopt(tfn->socket, SOL_SOCKET, SO_SNDBUF, &optval, &optlen);
    D(DBF_NET, "SO_SNDBUF.....: %ld bytes", optval);

    optlen = sizeof(optval);
    getsockopt(tfn->socket, SOL_SOCKET, SO_RCVBUF, &optval, &optlen);
    D(DBF_NET, "SO_RCVBUF.....: %ld bytes", optval);

    optlen = sizeof(optval);
    getsockopt(tfn->socket, SOL_SOCKET, SO_SNDLOWAT, &optval, &optlen);
    D(DBF_NET, "SO_SNDLOWAT...: %ld", optval);

    optlen = sizeof(optval);
    getsockopt(tfn->socket, SOL_SOCKET, SO_RCVLOWAT, &optval, &optlen);
    D(DBF_NET, "SO_RCVLOWAT...: %ld", optval);

    tvlen = sizeof(tv);
    getsockopt(tfn->socket, SOL_SOCKET, SO_SNDTIMEO, &tv, &tvlen);
    D(DBF_NET, "SO_SNDTIMEO...: %ld", tv.Seconds);

    tvlen = sizeof(tv);
    getsockopt(tfn->socket, SOL_SOCKET, SO_RCVTIMEO, &tv, &tvlen);
    D(DBF_NET, "SO_RCVTIMEO...: %ld", tv.Seconds);
  }
  #endif

  LEAVE();
}
///
/// TR_Connect
//  Connects to a internet service
enum ConnectError TR_Connect(struct TransferNode *tfn)
{
  enum ConnectError result = CONNECTERR_UNKNOWN_ERROR;

  ENTER();

  if(tfn->socket == TCP_NO_SOCKET)
  {
    struct hostent *hostaddr;

    // get the hostent out of the supplied hostname
    if((hostaddr = gethostbyname(tfn->msn->hostname)) != NULL)
    {
      int i;

      #ifdef DEBUG
      D(DBF_NET, "Host '%s':", tfn->msn->hostname);
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
        if((tfn->socket = socket(AF_INET, SOCK_STREAM, 0)) != TCP_NO_SOCKET)
        {
          char nonBlockingIO = 1;

          // now we set the socket for non-blocking I/O
          if(IoctlSocket(tfn->socket, FIONBIO, &nonBlockingIO) != -1)
          {
            int connectIssued = 0;
            LONG err = CONNECTERR_NO_ERROR; // set to no error per default
            struct sockaddr_in socketAddr;

            D(DBF_NET, "successfully set socket to %sblocking I/O", nonBlockingIO == 1 ? "non-" : "");

            // lets set the socket options the user has defined
            // in the configuration
            TR_SetSocketOpts(tfn);

            // copy the hostaddr data in a local copy for further reference
            memset(&socketAddr, 0, sizeof(socketAddr));
            socketAddr.sin_len    = sizeof(socketAddr);
            socketAddr.sin_family = AF_INET;
            socketAddr.sin_port   = htons(tfn->msn->port);

            memcpy(&socketAddr.sin_addr, hostaddr->h_addr_list[i], (size_t)hostaddr->h_length);

            D(DBF_NET, "trying TCP/IP connection to '%s' on port %ld", Inet_NtoA(((struct in_addr *)hostaddr->h_addr_list[i])->s_addr), tfn->msn->port);

            // we call connect() to establish the connection to the socket. In case of
            // a non-blocking connection this call will return immediately with -1 and
            // the errno value will be EINPROGRESS, EALREADY or EISCONN
            while(err == CONNECTERR_NO_ERROR &&
                  connect(tfn->socket, (struct sockaddr *)&socketAddr, sizeof(socketAddr)) == -1)
            {
              LONG connerr = -1;

              // count the number of connect() processings
              connectIssued++;

              // get the error value which should normally be set by a connect()
              SocketBaseTags(SBTM_GETREF(SBTC_ERRNO), (ULONG)&connerr, TAG_END);

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
                         (tfn->abort == TRUE))
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
                    FD_SET(tfn->socket, &fdset);
                  }
                  while((retVal = WaitSelect(tfn->socket+1, NULL, &fdset, NULL, (APTR)&timeout, NULL)) == 0);

                  // if WaitSelect() returns 1 we successfully waited for
                  // being able to write to the socket. So we can break out of the
                  // loop immediately and continue with our stuff.
                  if(retVal >= 1 &&
                     FD_ISSET(tfn->socket, &fdset))
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
                    if(getsockopt(tfn->socket, SOL_SOCKET, SO_ERROR, &errval, &errlen) == 0 &&
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
              TR_Disconnect(tfn);
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
        if(tfn->abort == TRUE)
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
    D(DBF_NET, "connection to %s:%ld succedded", tfn->msn->hostname, tfn->msn->port);
  #endif

  RETURN(result);
  return result;
}
///
/// TR_Disconnect
//  Terminates a connection
void TR_Disconnect(struct TransferNode *tfn)
{
  ENTER();

  D(DBF_NET, "disconnecting TCP/IP session...");

  if(tfn->socket != TCP_NO_SOCKET)
  {
    if(tfn->useSSL == TRUE)
    {
      TR_EndTLS(tfn);
      tfn->useSSL = FALSE;
    }

    shutdown(tfn->socket, SHUT_RDWR);
    CloseSocket(tfn->socket);
    tfn->socket = TCP_NO_SOCKET;

    // free the transfer buffers now
    TR_FreeTransBuffers(tfn);
  }

  LEAVE();
}
///
/// TR_Cleanup
//  Free temporary message and UIDL lists
void TR_Cleanup(struct TransferNode *tfn)
{
  struct Node *curNode;

  ENTER();

  if(G->transferWindowObject != NULL)
    DoMethod(G->transferWindowObject, MUIM_TransferWindow_Clear);

  while((curNode = RemHead((struct List *)&tfn->mailTransferList)) != NULL)
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

  NewMinList(&tfn->mailTransferList);

  LEAVE();
}
///
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
  #if !defined(__AROS__)
  if(C->IsOnlineCheck == TRUE)
  {
    struct Library *MiamiBase;
    struct Library *GenesisBase;

    if((MiamiBase = OpenLibrary("miami.library", 10L)) != NULL)
    {
      D(DBF_NET, "identified Miami TCP/IP stack");

      isonline = MiamiIsOnline(C->IOCInterface[0] != '\0' ? C->IOCInterface : NULL);

      CloseLibrary(MiamiBase);
      MiamiBase = NULL;
    }
    else if((GenesisBase = OpenLibrary("genesis.library", 1L)) != NULL)
    {
      D(DBF_NET, "identified Genesis TCP/IP stack");

      isonline = IsOnline(C->IOCInterface[0] != '\0' ? (long)C->IOCInterface : 0);

      CloseLibrary(GenesisBase);
      GenesisBase = NULL;
    }
    else if(SocketBase == NULL)
    {
      if((SocketBase = OpenLibrary("bsdsocket.library", 2L)) != NULL)
      {
        CloseLibrary(SocketBase);
        SocketBase = NULL;
        isonline = TRUE;
      }
    }
    else if(SocketBase->lib_Version >= 2)
      isonline = TRUE;
  }
  else
  #endif
  if(SocketBase == NULL)
  {
    // if no online check was selected, we just do a simple library exists
    // check and see if we are able to open a bsdsocket.library with a
    // minimum version of 2 or not.
    if((SocketBase = OpenLibrary("bsdsocket.library", 2L)) != NULL)
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

/*** TCP/IP transfer routines ***/
/// TR_Recv()
// a own wrapper function for Recv()/SSL_read() that reads buffered somehow
// it reads maxlen-1 data out of the buffer and stores it into recvdata with
// a null terminated string
int TR_Recv(struct TransferNode *tfn, char *recvdata, const int maxlen)
{
  int nread = -1;

  ENTER();

  // make sure the GUI can still update itself during this
  // operation
  DoMethod(G->App, MUIM_Application_InputBuffered);

  // make sure the socket is active.
  if(tfn->socket != TCP_NO_SOCKET)
  {
    // we call the ReadBuffered function so that
    // we get out the data from our own buffer.
    nread = TR_ReadBuffered(tfn, recvdata, maxlen-1, TCPF_NONE);

    if(nread <= 0)
      recvdata[0] = '\0';
    else
      recvdata[nread] = '\0';

    if(G->TR_Debug == TRUE)
    {
      printf("SERVER[%04d]: %s", nread, recvdata);

      // add a linefeed in case of an error
      if(nread <= 0 || strlen(recvdata) == 0)
        printf("\n");
    }

    D(DBF_NET, "TCP: received %ld of max %ld bytes", nread, maxlen);
  }
  else
    W(DBF_NET, "socket == TCP_NO_SOCKET");

  RETURN(nread);
  return nread;
}

///
/// TR_Send()
// a own wrapper function for Send()/SSL_write() that writes buffered somehow
// if called with flag != TCP_FLUSH - otherwise it will write and flush immediatly
int TR_Send(struct TransferNode *tfn, const char *ptr, const int len, const int flags)
{
  int nwritten = -1;

  ENTER();

  // make sure the GUI can still update itself during this
  // operation
  DoMethod(G->App, MUIM_Application_InputBuffered);

  // make sure the socket is active.
  if(tfn->socket != TCP_NO_SOCKET)
  {
    // perform some debug output on the console if requested
    // by the user
    if(G->TR_Debug == TRUE && ptr != NULL)
      printf("CLIENT[%04d]: %s", len, ptr);

    // we call the WriteBuffered() function to write this characters
    // out to the socket. the provided flag will define if it
    // really will be buffered or if we write and flush the buffer
    // immediatly
    nwritten = TR_WriteBuffered(tfn, ptr, len, flags);

    D(DBF_NET, "TCP: sent %ld of %ld bytes (%ld)", nwritten, len, flags);
  }
  else
    W(DBF_NET, "socket == TCP_NO_SOCKET");

  RETURN(nwritten);
  return nwritten;
}

///
/// TR_ReadLine()
// a buffered version of readline() that reads out charwise from the
// socket via TR_ReadBuffered and returns the amount of chars copied
// into the provided character array or -1 on error.
//
// This implementation is a slightly adapted version of readline()/my_read()
// examples from (W.Richard Stevens - Unix Network Programming) - Page 80
int TR_ReadLine(struct TransferNode *tfn, char *vptr, const int maxlen)
{
  int result = -1;

  ENTER();

  // make sure the GUI can still update itself during this
  // operation
  DoMethod(G->App, MUIM_Application_InputBuffered);

  // make sure the socket is active.
  if(tfn->socket != TCP_NO_SOCKET)
  {
    int n;
    char *ptr;

    ptr = vptr;

    for(n = 1; n < maxlen; n++)
    {
      int rc;
      char c;

      // read out one buffered char only.
      rc = TR_ReadBuffered(tfn, &c, 1, TCPF_NONE);
      if(rc == 1)
      {
        *ptr++ = c;
        if(c == '\n')
          break;  // newline is stored, like with getline()
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

    *ptr = '\0'; // null terminate like getline()

    // perform some debug output on the console if requested
    // by the user
    if(G->TR_Debug == TRUE)
    {
      printf("SERVER[%04d]: %s", n, vptr);

      // add a linefeed in case of an error
      if(n <= 0 || strlen(vptr) == 0)
        printf("\n");
    }

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
static int TR_Read(struct TransferNode *tfn, char *ptr, int maxlen)
{
  int result;
  int nread = -1; // -1 is error
  int status = 0; // < 0 error, 0 unknown, > 0 no error

  ENTER();

  if(tfn->useSSL == TRUE)
  {
    // use SSL methods to get/process all data
    do
    {
      // read out data and stop our loop in case there
      // isn't anything to read anymore or we have filled
      // maxlen data
      nread = SSL_read(tfn->ssl, ptr, maxlen);

      // if nread > 0 we read _some_ data and can
      // break out
      if(nread > 0)
        break;
      else // <= 0 found, check error state
      {
        int err = SSL_get_error(tfn->ssl, nread);

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
                   (tfn->abort == TRUE))
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
              FD_SET(tfn->socket, &fdset);

              // depending on the SSL error (WANT_READ/WANT_WRITE)
              // we either do a WaitSelect() on the read or write mode
              // as with SSL both things can happen
              // see http://www.openssl.org/docs/ssl/SSL_read.html
              if(err == SSL_ERROR_WANT_READ)
                retVal = WaitSelect(tfn->socket+1, &fdset, NULL, NULL, (APTR)&timeout, NULL);
              else
                retVal = WaitSelect(tfn->socket+1, NULL, &fdset, NULL, (APTR)&timeout, NULL);
            }
            while(retVal == 0);

            // if WaitSelect() returns 1 we successfully waited for
            // being able to read from the socket. So we go and do another
            // iteration in the while() loop as the next connect() call should
            // return EISCONN if the connection really succeeded.
            if(retVal >= 1 && FD_ISSET(tfn->socket, &fdset))
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
      nread = recv(tfn->socket, ptr, maxlen, 0);

      // if nread >= 0 we read _some_ data or reach the
      // end of the socket and can break out
      if(nread >= 0)
        break;
      else // < 0 found, check error state
      {
        LONG err = -1;

        // get the error value which should normally be set by a recv()
        SocketBaseTags(SBTM_GETREF(SBTC_ERRNO), (ULONG)&err, TAG_END);

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
                   (tfn->abort == TRUE))
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
              FD_SET(tfn->socket, &fdset);
            }
            while((retVal = WaitSelect(tfn->socket+1, &fdset, NULL, NULL, (APTR)&timeout, NULL)) == 0);

            // if WaitSelect() returns 1 we successfully waited for
            // being able to read from the socket. So we go and do another
            // iteration in the while() loop as the next connect() call should
            // return EISCONN if the connection really succeeded.
            if(retVal >= 1 &&
               FD_ISSET(tfn->socket, &fdset))
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
              E(DBF_NET, "WaitSelect() returned error: %ld", err);
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
            E(DBF_NET, "recv() returned error %ld", err);
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
static int TR_ReadBuffered(struct TransferNode *tfn, char *ptr, const int maxlen, const int flags)
{
  int result = -1; // -1 = error
  static int read_cnt = 0;
  static char *read_buf = NULL;
  static char *read_ptr = NULL;

  ENTER();

  // if we don't have a buffer yet, lets allocate one
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
        if((read_cnt = TR_Read(tfn, read_buf, C->TRBufferSize)) <= 0)
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
static int TR_Write(struct TransferNode *tfn, const char *ptr, int len)
{
  int result;
  int towrite = len;
  int status = 0; // < 0 error, 0 unknown, > 0 no error

  ENTER();

  if(tfn->useSSL == TRUE)
  {
    // use SSL methods to get/process all data
    do
    {
      // write data to our socket and then check how much
      // we have written and in case we weren't able to write
      // all out, we retry it with smaller chunks.
      int nwritten = SSL_write(tfn->ssl, ptr, towrite);

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
        int err = SSL_get_error(tfn->ssl, nwritten);

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
                   (tfn->abort == TRUE))
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
              FD_SET(tfn->socket, &fdset);

              // depending on the SSL error (WANT_READ/WANT_WRITE)
              // we either do a WaitSelect() on the read or write mode
              // as with SSL both things can happen
              // see http://www.openssl.org/docs/ssl/SSL_write.html
              if(err == SSL_ERROR_WANT_READ)
                retVal = WaitSelect(tfn->socket+1, &fdset, NULL, NULL, (APTR)&timeout, NULL);
              else
                retVal = WaitSelect(tfn->socket+1, NULL, &fdset, NULL, (APTR)&timeout, NULL);
            }
            while(retVal == 0);

            // if WaitSelect() returns 1 we successfully waited for
            // being able to write to the socket. So we go and do another
            // iteration in the while() loop as the next connect() call should
            // return EISCONN if the connection really succeeded.
            if(retVal >= 1 && FD_ISSET(tfn->socket, &fdset))
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
              E(DBF_NET, "WaitSelect() returned error %ld", err);
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
            E(DBF_NET, "SSL_write() returned error %ld", err);
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
      int nwritten = send(tfn->socket, (APTR)ptr, towrite, 0);

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
        SocketBaseTags(SBTM_GETREF(SBTC_ERRNO), (ULONG)&err, TAG_END);

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
                   (tfn->abort == TRUE))
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
              FD_SET(tfn->socket, &fdset);
            }
            while((retVal = WaitSelect(tfn->socket+1, NULL, &fdset, NULL, (APTR)&timeout, NULL)) == 0);

            // if WaitSelect() returns 1 we successfully waited for
            // being able to write to the socket. So we go and do another
            // iteration in the while() loop as the next connect() call should
            // return EISCONN if the connection really succeeded.
            if(retVal >= 1 &&
               FD_ISSET(tfn->socket, &fdset))
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
              E(DBF_NET, "WaitSelect() returned error %ld", err);
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
            E(DBF_NET, "send() returned error %ld", err);
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
static int TR_WriteBuffered(struct TransferNode *tfn, const char *ptr, const int maxlen, const int flags)
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
        if(TR_Write(tfn, write_buf, write_cnt) != write_cnt)
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
        int max_cnt = maxlen;

        // if the string we want to copy into the buffer
        // wouldn't fit, we copy as much as we can, clear the buffer
        // and continue until there is enough space left
        while(write_cnt+max_cnt > C->TRBufferSize)
        {
          int fillable = C->TRBufferSize-write_cnt;

          // after the copy we have to increase the pointer of the
          // array we want to copy, because in the next cycle we have to
          // copy the rest out of it.
          memcpy(write_ptr, ptr, fillable);
          ptr += fillable;
          write_cnt += fillable;
          fill_cnt += fillable;

          // decrease max_cnt now by the amount of bytes we have written
          // to the buffer
          max_cnt -= fillable;

          // now we write out the data
          if(TR_Write(tfn, write_buf, write_cnt) != write_cnt)
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
          memcpy(write_ptr, ptr, max_cnt);
          write_ptr += max_cnt;
          write_cnt += max_cnt;
          fill_cnt += max_cnt;

          // if the user has supplied the FLUSH flag we have to clear/flush
          // the buffer immediately after having copied everything
          if(hasTCP_FLUSH(flags))
          {
            // write our whole buffer out to the socket
            if(TR_Write(tfn, write_buf, write_cnt) != write_cnt)
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
  struct TransferNode *tfn;

  ENTER();

  // make sure the error state is cleared
  G->Error = FALSE;

  // we create a new Transfer and bind it to the information we received
  if((tfn = CreateNewTransfer(NULL)) != NULL)
  {
    BOOL noproxy = (C->ProxyServer[0] == '\0');
    int hport;
    char url[SIZE_URL];
    char host[SIZE_HOST];
    char *path;
    char *bufptr;

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

    // fill the mailserver node structure
    strlcpy(tfn->msn->hostname, host, sizeof(tfn->msn->hostname));
    tfn->msn->port = hport;

    // open the TCP/IP connection to 'host' under the port 'hport'
    if(TR_Connect(tfn) == CONNECTERR_SUCCESS)
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
      if(TR_WriteLine(tfn, httpRequest) > 0)
      {
        char *p;
        char serverResponse[SIZE_LINE];
        int len;

        // clear the serverResponse string
        serverResponse[0] = '\0';

        // now we read out the very first line to see if the
        // response code matches and is fine
        len = TR_ReadLine(tfn, serverResponse, sizeof(serverResponse));

        SHOWSTRING(DBF_NET, serverResponse);

        // check the server response
        if(len > 0 && strnicmp(serverResponse, "HTTP/", 5) == 0 &&
           (p = strchr(serverResponse, ' ')) != NULL && atoi(TrimStart(p)) == 200)
        {
          // we can request all further lines from our socket
          // until we reach the entity body
          while(G->Error == FALSE &&
                (len = TR_ReadLine(tfn, serverResponse, sizeof(serverResponse))) > 0)
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
                while(G->Error == FALSE &&
                      (len = TR_Recv(tfn, serverResponse, sizeof(serverResponse))) > 0)
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
                if(G->Error == FALSE && retrieved >= 0)
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

      TR_Disconnect(tfn);
    }
    else
      ER_NewError(tr(MSG_ER_ConnectHTTP), host);

    // now we free the temporary transfer node and the mail
    // server node as we have generated it temporarly.
    free(tfn->msn);
    free(tfn);
  }

  RETURN((BOOL)(result == TRUE && G->Error == FALSE));
  return (BOOL)(result == TRUE && G->Error == FALSE);
}

///

/*** GUI ***/
/// SetupTransferWindow
//
BOOL SetupTransferWindow(enum TransferType type)
{
  BOOL result = FALSE;
  ENTER();

#warning "TODO: we should propagate the transfertype to the window (even if it is open already)"

  if(G->transferWindowObject == NULL)
    G->transferWindowObject = TransferWindowObject,
                              End;

  if(G->transferWindowObject != NULL)
    result = TRUE;

  RETURN(result);
  return result;
}

///
/// CleanupTransferWindow
//
void CleanupTransferWindow(void)
{
  ENTER();

  if(G->transferWindowObject != NULL)
  {
    xset(G->transferWindowObject, MUIA_Window_Open, FALSE);
    DoMethod(G->App, OM_REMMEMBER, G->transferWindowObject);
    MUI_DisposeObject(G->transferWindowObject);
    G->transferWindowObject = NULL;
  }

  LEAVE();
}

///
