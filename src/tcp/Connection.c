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

#include <clib/alib_protos.h>
#include <proto/amissl.h>
#include <proto/amisslmaster.h>

// we include bsdsocket.h but make sure to not let
// it define a global SocketBase or ISocket so that
// our socket stuff is assured to not use a global socket
// library base
#define __NOLIBBASE__
#define __NOGLOBALIFACE__
#include <proto/bsdsocket.h>
#undef __NOGLOBALIFACE__
#undef __NOLIBBASE__

#include <proto/exec.h>
#include <proto/intuition.h>
#if !defined(__amigaos4__) && !defined(__AROS__)
#include <proto/miami.h>
#include <proto/genesis.h>
#endif

#include <netinet/ip.h>
#include <netinet/tcp.h>
#include <sys/socket.h>
#include <netdb.h>

#if defined(__AROS__)
#define _SYS_MBUF_H_
#include <libraries/bsdsocket.h>
#include <sys/ioctl.h>
#include <sys/types.h>
#else
#include <sys/filio.h>
#endif

#include <libraries/genesis.h>

#include "YAM.h"
#include "YAM_config.h"
#include "YAM_error.h"

#include "Locale.h"
#include "Threads.h"

#include "tcp/Connection.h"

#include "Debug.h"

#ifndef SHUT_RDWR
#define SHUT_RDWR 2
#endif

// SocketBase must not be shared between tasks, hence we must not use the global
// SocketBase, but the one tied to the current connection.
#if defined(__amigaos4__)
#define GET_SOCKETBASE(conn)  struct SocketIFace *ISocket = (conn)->socketIFace
#else
#define GET_SOCKETBASE(conn)  struct Library *SocketBase = (conn)->socketBase
#endif

/// SetSocketOpts
//  Sets the user specified options for the active socket
static void SetSocketOpts(struct Connection *conn)
{
  struct TagItem tags[] =
  {
    { SBTM_SETVAL(SBTC_BREAKMASK), 0 },
    { TAG_END,                     0 }
  };
  GET_SOCKETBASE(conn);

  ENTER();

  // disable CTRL-C checking
  SocketBaseTagList(tags);

  D(DBF_NET, "set options");
  if(C->SocketOptions.KeepAlive == TRUE)
  {
    int optval = C->SocketOptions.KeepAlive;

    if(setsockopt(conn->socket, SOL_SOCKET, SO_KEEPALIVE, &optval, sizeof(optval)) == -1)
    {
      E(DBF_NET, "setsockopt(SO_KEEPALIVE) error");
      ER_NewError(tr(MSG_ER_SOCKETOPTION), "SO_KEEPALIVE");
    }
  }

  if(C->SocketOptions.NoDelay == TRUE)
  {
    int optval = C->SocketOptions.NoDelay;

    if(setsockopt(conn->socket, IPPROTO_TCP, TCP_NODELAY, &optval, sizeof(optval)) == -1)
    {
      E(DBF_NET, "setsockopt(TCP_NODELAY) error");
      ER_NewError(tr(MSG_ER_SOCKETOPTION), "TCP_NODELAY");
    }
  }

  if(C->SocketOptions.LowDelay == TRUE)
  {
    int optval = IPTOS_LOWDELAY;

    if(setsockopt(conn->socket, IPPROTO_IP, IP_TOS, &optval, sizeof(optval)) == -1)
    {
      E(DBF_NET, "setsockopt(IPTOS_LOWDELAY) error");
      ER_NewError(tr(MSG_ER_SOCKETOPTION), "IPTOS_LOWDELAY");
    }
  }

  if(C->SocketOptions.SendBuffer > -1)
  {
    int optval = C->SocketOptions.SendBuffer;

    if(setsockopt(conn->socket, SOL_SOCKET, SO_SNDBUF, &optval, sizeof(optval)) == -1)
    {
      E(DBF_NET, "setsockopt(SO_SNDBUF) error");
      ER_NewError(tr(MSG_ER_SOCKETOPTION), "SO_SNDBUF");
    }
  }

  if(C->SocketOptions.RecvBuffer > -1)
  {
    int optval = C->SocketOptions.RecvBuffer;

    if(setsockopt(conn->socket, SOL_SOCKET, SO_RCVBUF, &optval, sizeof(optval)) == -1)
    {
      E(DBF_NET, "setsockopt(SO_RCVBUF) error");
      ER_NewError(tr(MSG_ER_SOCKETOPTION), "SO_RCVBUF");
    }
  }

  if(C->SocketOptions.SendLowAt > -1)
  {
    int optval = C->SocketOptions.SendLowAt;

    if(setsockopt(conn->socket, SOL_SOCKET, SO_SNDLOWAT, &optval, sizeof(optval)) == -1)
    {
      E(DBF_NET, "setsockopt(SO_SNDLOWAT) error");
      ER_NewError(tr(MSG_ER_SOCKETOPTION), "SO_SNDLOWAT");
    }
  }

  if(C->SocketOptions.RecvLowAt > -1)
  {
    int optval = C->SocketOptions.RecvLowAt;

    if(setsockopt(conn->socket, SOL_SOCKET, SO_RCVLOWAT, &optval, sizeof(optval)) == -1)
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

    if(setsockopt(conn->socket, SOL_SOCKET, SO_SNDTIMEO, &tv, sizeof(struct TimeVal)) == -1)
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

    if(setsockopt(conn->socket, SOL_SOCKET, SO_RCVTIMEO, &tv, sizeof(struct TimeVal)) == -1)
    {
      E(DBF_NET, "setsockopt(SO_RCVTIMEO) error");
      ER_NewError(tr(MSG_ER_SOCKETOPTION), "SO_RCVTIMEO");
    }
  }

  // lets print out the current socket options
  #if defined(DEBUG)
  {
    int optval;
    struct TimeVal tv;
    socklen_t optlen;
    socklen_t tvlen;

    D(DBF_NET, "opened socket %08lx", conn->socket);

    // the value of the length pointer must be updated ahead of each call, because
    // getsockopt() might have modified it.
    optlen = sizeof(optval);
    getsockopt(conn->socket, SOL_SOCKET, SO_KEEPALIVE, &optval, &optlen);
    D(DBF_NET, "SO_KEEPALIVE..: %ld", optval);

    optlen = sizeof(optval);
    getsockopt(conn->socket, IPPROTO_TCP, TCP_NODELAY, &optval, &optlen);
    D(DBF_NET, "TCP_NODELAY...: %ld", optval);

    optlen = sizeof(optval);
    getsockopt(conn->socket, IPPROTO_IP, IP_TOS, &optval, &optlen);
    D(DBF_NET, "IPTOS_LOWDELAY: %ld", hasFlag(optval, IPTOS_LOWDELAY));

    optlen = sizeof(optval);
    getsockopt(conn->socket, SOL_SOCKET, SO_SNDBUF, &optval, &optlen);
    D(DBF_NET, "SO_SNDBUF.....: %ld bytes", optval);

    optlen = sizeof(optval);
    getsockopt(conn->socket, SOL_SOCKET, SO_RCVBUF, &optval, &optlen);
    D(DBF_NET, "SO_RCVBUF.....: %ld bytes", optval);

    optlen = sizeof(optval);
    getsockopt(conn->socket, SOL_SOCKET, SO_SNDLOWAT, &optval, &optlen);
    D(DBF_NET, "SO_SNDLOWAT...: %ld", optval);

    optlen = sizeof(optval);
    getsockopt(conn->socket, SOL_SOCKET, SO_RCVLOWAT, &optval, &optlen);
    D(DBF_NET, "SO_RCVLOWAT...: %ld", optval);

    tvlen = sizeof(tv);
    getsockopt(conn->socket, SOL_SOCKET, SO_SNDTIMEO, &tv, &tvlen);
    D(DBF_NET, "SO_SNDTIMEO...: %ld", tv.Seconds);

    tvlen = sizeof(tv);
    getsockopt(conn->socket, SOL_SOCKET, SO_RCVTIMEO, &tv, &tvlen);
    D(DBF_NET, "SO_RCVTIMEO...: %ld", tv.Seconds);
  }
  #endif

  LEAVE();
}

///
/// CreateConnection
// create a connection structure
struct Connection *CreateConnection(void)
{
  struct Connection *result = NULL;
  struct Connection *conn;

  ENTER();

  if((conn = calloc(1, sizeof(*conn))) != NULL)
  {
    if((conn->receiveBuffer = malloc(C->TRBufferSize)) != NULL)
    {
      if((conn->sendBuffer = malloc(C->TRBufferSize)) != NULL)
      {
        // each connection gets its own SocketBase, this is required for threads
        if((conn->socketBase = OpenLibrary("bsdsocket.library", 2L)) != NULL &&
           GETINTERFACE("main", 1, conn->socketIFace, conn->socketBase))
        {
          D(DBF_NET, "got socket interface");

          conn->socket = TCP_NO_SOCKET;

          // set to no error per default
          conn->error = CONNECTERR_NO_ERROR;

          conn->receivePtr = conn->receiveBuffer;
          conn->sendPtr = conn->sendBuffer;

          // remember the buffer sizes in case this is
          // modified as long as this connection exists
          conn->receiveBufferSize = C->TRBufferSize;
          conn->sendBufferSize = C->TRBufferSize;

          conn->connectedFromMainThread = IsMainThread();

          result = conn;
        }
      }
    }
  }

  if(result == NULL)
    DeleteConnection(conn);

  RETURN(result);
  return result;
}

///
/// DeleteConnection
// delete a connection structure
void DeleteConnection(struct Connection *conn)
{
  ENTER();

  if(conn != NULL)
  {
    DisconnectFromHost(conn);

    if(conn->socketBase != NULL)
    {
      DROPINTERFACE(conn->socketIFace);
      CloseLibrary(conn->socketBase);
      conn->socketBase = NULL;
    }

    free(conn->sendBuffer);
    free(conn->receiveBuffer);
    free(conn);
  }

  LEAVE();
}

///
/// CheckSingleInterface
// checks a single interface to be online
enum TCPIPStack
{
  TCPIP_Generic = 0,
  TCPIP_RoadShow,
  TCPIP_Miami,
  TCPIP_Genesis
};

static BOOL CheckSingleInterface(struct Connection *conn, const char *iface, const enum TCPIPStack tcpipStack, const struct Library *stackBase)
{
  BOOL isOnline = FALSE;

  ENTER();

  switch(tcpipStack)
  {
    case TCPIP_Generic:
    {
      if(stackBase == NULL && conn->socketBase != NULL)
      {
        D(DBF_NET, "assuming interface '%s' to be up", iface);
        isOnline = TRUE;
      }
    }
    break;

    case TCPIP_RoadShow:
    {
      #if defined(__amigaos4__)
      LONG onlineState = 0;
      GET_SOCKETBASE(conn);

      if(QueryInterfaceTags((char *)iface, IFQ_State, &onlineState, TAG_END) == 0)
      {
        if(onlineState == SM_Up)
        {
          D(DBF_NET, "found RoadShow interface '%s' to be UP", iface);
          isOnline = TRUE;
        }
        else
          W(DBF_NET, "found RoadShow interface '%s' to be DOWN", iface);
      }
      else
        E(DBF_NET, "couldn't query interface status. Unknown interface.");
      #endif
    }
    break;

    case TCPIP_Miami:
    {
      #if !defined(__amigaos4__) && !defined(__AROS__)
      struct Library *MiamiBase = (struct Library *)stackBase;

      if(MiamiIsOnline(iface[0] != '\0' ? (char *)iface : NULL))
      {
        D(DBF_NET, "found Miami interface '%s' to be UP", iface);
        isOnline = TRUE;
      }
      else
        W(DBF_NET, "found Miami interface '%s' to be DOWN", iface);
      #endif
    }
    break;

    case TCPIP_Genesis:
    {
      #if !defined(__amigaos4__) && !defined(__AROS__)
      struct Library *GenesisBase = (struct Library *)stackBase;

      if(IsOnline(iface[0] != '\0' ? (long)iface : 0))
      {
        D(DBF_NET, "found Genesis interface '%s' to be UP", iface);
        isOnline = TRUE;
      }
      else
        W(DBF_NET, "found Genesis interface '%s' to be DOWN", iface);
      #endif
    }
    break;
  }

  RETURN(isOnline);
  return isOnline;
}

///
/// CheckAllInterfaces
// check if any of the given interfaces is online
static BOOL CheckAllInterfaces(struct Connection *conn, const enum TCPIPStack tcpipStack, const struct Library *stackBase)
{
  BOOL anyIsOnline = FALSE;

  ENTER();

  SHOWSTRING(DBF_NET, C->IOCInterfaces);
  if(C->IOCInterfaces[0] != '\0')
  {
    char *ifaces;

    // duplicate the interfaces setting and split it into its parts
    if((ifaces = strdup(C->IOCInterfaces)) != NULL)
    {
      char *iface = ifaces;
      char *next;

      do
      {
        if((next = strpbrk(iface, ", ")) != NULL)
          *next++ = '\0';

        if(iface[0] != '\0')
        {
          // check every single interface to be online and
          // bail out as soon as we found an active interface
          D(DBF_NET, "checking interface '%s'", iface);
          if(CheckSingleInterface(conn, iface, tcpipStack, stackBase) == TRUE)
          {
            anyIsOnline = TRUE;
            break;
          }
        }

        iface = next;
      }
      while(iface != NULL);

      free(ifaces);
    }
  }
  else
  {
    // check with no interface name
    anyIsOnline = CheckSingleInterface(conn, "", tcpipStack, stackBase);
  }

  RETURN(anyIsOnline);
  return anyIsOnline;
}

///
/// ConnectionIsOnline
// check whether the connection can be used to connect to a host
BOOL ConnectionIsOnline(struct Connection *conn)
{
  BOOL isonline = FALSE;
  BOOL deleteConnection = FALSE;

  ENTER();

  if(conn == NULL)
  {
    conn = CreateConnection();
    deleteConnection = TRUE;
  }

  if(conn != NULL)
  {
    // on AmigaOS4 we always do an online check via the v4 version
    // of bsdsocket.library (RoadShow) as it should always be present
    #if defined(__amigaos4__)
    // if we find a bsdsocket.library < v4 on OS4
    // we always assume it to be online
    if(LIB_VERSION_IS_AT_LEAST(conn->socketBase, 4, 0) == FALSE)
    {
      isonline = TRUE;
    }
    else
    {
      D(DBF_NET, "identified bsdsocket v4 TCP/IP stack (RoadShow)");

      // in case the user hasn't specified a specific
      // interface or set that the online check for a specific
      // interface should be disabled we just do a general query
      if(C->IsOnlineCheck == FALSE || C->IOCInterfaces[0] == '\0')
      {
        ULONG status = 0;
        struct TagItem tags[] =
        {
          { SBTM_GETREF(SBTC_SYSTEM_STATUS), (IPTR)&status },
          { TAG_END,                         0             }
        };
        GET_SOCKETBASE(conn);

        if(SocketBaseTagList(tags) == 0)
        {
          if(hasFlag(status, SBSYSSTAT_Interfaces))
            isonline = TRUE;
        }
        else
          E(DBF_NET, "couldn't query TCP/IP stack for its system status.");
      }
      else
      {
        ULONG hasInterfaceAPI = FALSE;
        struct TagItem tags[] =
        {
          { SBTM_GETREF(SBTC_HAVE_INTERFACE_API), (IPTR)&hasInterfaceAPI },
          { TAG_END,                              0                      }
        };
        GET_SOCKETBASE(conn);

        if(SocketBaseTagList(tags) == 0 && hasInterfaceAPI == TRUE)
        {
          // now that we know that we have an interface API, we can
          // go and query the interfaces if any of these is up&running
          // correctly.
          isonline = CheckAllInterfaces(conn, TCPIP_RoadShow, NULL);
        }
        else
        {
          E(DBF_NET, "couldn't query TCP/IP stack's interface API (%ld).", hasInterfaceAPI);
        }
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

        isonline = CheckAllInterfaces(conn, TCPIP_Miami, MiamiBase);

        CloseLibrary(MiamiBase);
        MiamiBase = NULL;
      }
      else if((GenesisBase = OpenLibrary("genesis.library", 1L)) != NULL)
      {
        D(DBF_NET, "identified Genesis TCP/IP stack");

        isonline = CheckAllInterfaces(conn, TCPIP_Genesis, GenesisBase);

        CloseLibrary(GenesisBase);
        GenesisBase = NULL;
      }
      else if(LIB_VERSION_IS_AT_LEAST(conn->socketBase, 2, 0) == TRUE)
      {
        D(DBF_NET, "identified generic TCP/IP stack with bsdsocket.library v2+");

        isonline = CheckAllInterfaces(conn, TCPIP_Generic, NULL);
      }
    }
    else
    #endif // !__AROS__
    if(LIB_VERSION_IS_AT_LEAST(conn->socketBase, 2, 0) == TRUE)
      isonline = CheckAllInterfaces(conn, TCPIP_Generic, NULL);

    #endif // __amigaos4__
  }

  if(deleteConnection == TRUE)
    DeleteConnection(conn);

  D(DBF_NET, "found the TCP/IP stack to be %s", isonline ? "ONLINE" : "OFFLINE");

  RETURN(isonline);
  return isonline;
}

///
/// ConnectToHost
//  Creates a new connection and tries to connect to a internet service
enum ConnectError ConnectToHost(struct Connection *conn, const char *host, const int port)
{
  enum ConnectError error = CONNECTERR_NO_CONNECTION;

  ENTER();

  if(conn != NULL)
  {
    struct hostent *hostaddr;
    GET_SOCKETBASE(conn);

    // get the hostent out of the supplied hostname
    if((hostaddr = gethostbyname((char *)host)) != NULL)
    {
      int i;

      #if defined(DEBUG)
      D(DBF_NET, "Host '%s':", host);
      D(DBF_NET, "  Officially: '%s'", hostaddr->h_name);

      for(i = 0; hostaddr->h_aliases[i]; ++i)
        D(DBF_NET, "  Alias: '%s'", hostaddr->h_aliases[i]);

      D(DBF_NET, "  Type: '%s'", hostaddr->h_addrtype == AF_INET ? "AF_INET" : "AF_INET6");
      if(hostaddr->h_addrtype == AF_INET)
      {
        for(i = 0; hostaddr->h_addr_list[i]; ++i)
          D(DBF_NET, "  Address: '%s'", Inet_NtoA(((struct in_addr *)hostaddr->h_addr_list[i])->s_addr));
      }
      #endif

      // now we try a connection for every address we have for this host
      // because a hostname can have more than one IP in h_addr_list[]
      for(i = 0; hostaddr->h_addr_list[i]; i++)
      {
        // lets create a standard AF_INET socket now
        if((conn->socket = socket(AF_INET, SOCK_STREAM, 0)) != TCP_NO_SOCKET)
        {
          long nonBlockingIO = 1;

          // now we set the socket for non-blocking I/O
          if(IoctlSocket(conn->socket, FIONBIO, (void *)&nonBlockingIO) != -1)
          {
            int connectIssued = 0;

            D(DBF_NET, "successfully set socket to non-blocking I/O");

            // set the socket options the user has defined
            // in the configuration
            SetSocketOpts(conn);

            // copy the hostaddr data in a local copy for further reference
            memset(&conn->socketAddr, 0, sizeof(conn->socketAddr));
            conn->socketAddr.sin_len    = sizeof(conn->socketAddr);
            conn->socketAddr.sin_family = AF_INET;
            conn->socketAddr.sin_port   = htons(port);

            memcpy(&conn->socketAddr.sin_addr, hostaddr->h_addr_list[i], (size_t)hostaddr->h_length);

            D(DBF_NET, "trying TCP/IP connection to '%s' on port %ld", Inet_NtoA(((struct in_addr *)hostaddr->h_addr_list[i])->s_addr), port);

            // set to no error per default
            conn->error = CONNECTERR_NO_ERROR;

            // we call connect() to establish the connection to the socket. In case of
            // a non-blocking connection this call will return immediately with -1 and
            // the errno value will be EINPROGRESS, EALREADY or EISCONN
            while(conn->error == CONNECTERR_NO_ERROR &&
                  connect(conn->socket, (struct sockaddr *)&conn->socketAddr, sizeof(conn->socketAddr)) == -1)
            {
              LONG connerr = -1;
              struct TagItem tags[] =
              {
                { SBTM_GETREF(SBTC_ERRNO), (IPTR)&connerr },
                { TAG_END,                 0              }
              };

              // count the number of connect() processings
              connectIssued++;

              // get the error value which should normally be set by a connect()
              SocketBaseTagList(tags);

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
                  int timeoutSum = 0;

                  // now we iterate in a do/while loop and call WaitSelect()
                  // with a static timeout of 1s. We then continue to do so
                  // until we reach the predefined socket timeout
                  do
                  {
                    // check if we are here because of a second iteration
                    if(retVal == 0)
                    {
                      W(DBF_NET, "TR_Connect: recoverable WaitSelect() timeout: %ld", timeoutSum);

                      // give our GUI the chance to update
                      if(conn->connectedFromMainThread == TRUE)
                        DoMethod(G->App, MUIM_Application_InputBuffered);

                      // if retVal is zero, then we are here in a
                      // consequtive iteration, so WaitSelect()
                      // seem to have timed out and we have to check
                      // whether we timed out too much already
                      timeoutSum += 1; // +1s

                      if(timeoutSum >= C->SocketTimeout)
                      {
                        conn->error = CONNECTERR_TIMEDOUT;
                        break;
                      }
                      else if(conn->abort == TRUE)
                      {
                        conn->error = CONNECTERR_ABORTED;
                        break;
                      }
                    }

                    // we do a static timeout of 1s
                    conn->timeout.tv_sec = 1;
                    conn->timeout.tv_usec = 0;

                    // now we put our socket handle into a descriptor set
                    // we can pass on to WaitSelect()
                    FD_ZERO(&conn->fdset);
                    FD_SET(conn->socket, &conn->fdset);
                  }
                  while((retVal = WaitSelect(conn->socket+1, NULL, &conn->fdset, NULL, (APTR)&conn->timeout, NULL)) == 0);

                  // if WaitSelect() returns 1 we successfully waited for
                  // being able to write to the socket. So we can break out of the
                  // loop immediately and continue with our stuff.
                  if(retVal >= 1 &&
                     FD_ISSET(conn->socket, &conn->fdset))
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
                    if(getsockopt(conn->socket, SOL_SOCKET, SO_ERROR, &errval, &errlen) == 0 &&
                       errval == 0)
                    {
                      conn->error = CONNECTERR_SUCCESS;
                    }
                    else
                    {
                      E(DBF_NET, "SO_ERROR in socket options found! error value read: %ld", errval);
                      conn->error = CONNECTERR_UNKNOWN_ERROR;
                    }
                  }
                  else if(retVal == 0)
                  {
                    // the WaitSelect() call timed out or it received a break
                    // signal
                    W(DBF_NET, "WaitSelect() socket timeout reached");
                    conn->error = CONNECTERR_TIMEDOUT;
                  }
                  else
                  {
                    // the rest should signal an error
                    E(DBF_NET, "WaitSelect() returned an error: %ld", connerr);
                    conn->error = CONNECTERR_UNKNOWN_ERROR;
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
                    conn->error = CONNECTERR_SUCCESS;
                  }
                  else
                  {
                    E(DBF_NET, "connect() signaled an EISCONN failure");
                    conn->error = CONNECTERR_UNKNOWN_ERROR;
                  }
                }
                break;

                // all other errors are real errors
                default:
                {
                  E(DBF_NET, "connect() returned with an unrecoverable error: %ld", connerr);
                  conn->error = CONNECTERR_UNKNOWN_ERROR;
                }
                break;
              }
            }

            // in case the connection suceeded immediately (no entered the while)
            // we flag this connection as success
            if(conn->error == CONNECTERR_NO_ERROR)
              conn->error = CONNECTERR_SUCCESS;

            // now we are properly connected
            conn->isConnected = TRUE;

            // reset the buffer pointers
            conn->receiveCount = 0;
            conn->receivePtr = conn->receiveBuffer;
            conn->sendCount = 0;
            conn->sendPtr = conn->sendBuffer;
          }
          else
          {
            E(DBF_NET, "couldn't establish non-blocking IO: %ld", Errno());
            conn->error = CONNECTERR_NO_NONBLOCKIO;
            break;
          }
        }
        else
        {
          // socket() failed
          E(DBF_NET, "socket() returned an error: %ld", Errno());
          conn->error = CONNECTERR_NO_SOCKET;
          break;
        }

        // if the user pressed the abort button in the transfer
        // window we have to exit the loop
        if(conn->abort == TRUE)
        {
          conn->error = CONNECTERR_ABORTED;
          break;
        }
        else if(conn->error == CONNECTERR_SUCCESS)
          break;
        else
          E(DBF_NET, "connection result %ld, trying next IP of host...", conn->error);
      }
    }
    else
    {
      // gethostbyname() failed
      conn->error = CONNECTERR_UNKNOWN_HOST;
    }

    if(conn->error != CONNECTERR_SUCCESS)
      E(DBF_NET, "ConnectToHost() connection error: %ld", conn->error);
    else
      D(DBF_NET, "connection to %s:%ld succedded", host, port);

    error = conn->error;
  }

  RETURN(error);
  return error;
}

///
/// DisconnectFromHost
//  Terminate and free a connection
void DisconnectFromHost(struct Connection *conn)
{
  ENTER();

  D(DBF_NET, "disconnecting TCP/IP session %08lx", conn);

  if(conn != NULL)
  {
    if(conn->isConnected == TRUE)
    {
      GET_SOCKETBASE(conn);

      // shut down the SSL stuff
      if(conn->ssl != NULL)
      {
        SSL_shutdown(conn->ssl);
        SSL_free(conn->ssl);
        conn->ssl = NULL;
      }

      if(conn->sslCtx != NULL)
      {
        SSL_CTX_free(conn->sslCtx);
        conn->sslCtx = NULL;
      }

      // close the connection
      shutdown(conn->socket, SHUT_RDWR);
      CloseSocket(conn->socket);
      conn->socket = TCP_NO_SOCKET;

      if(AmiSSLBase != NULL)
        CleanupAmiSSLA(NULL);

      // we are no longer connected
      conn->isConnected = FALSE;
    }
  }

  LEAVE();
}

///
/// MakeSecureConnection
// Initialize an SSL/TLS session
BOOL MakeSecureConnection(struct Connection *conn)
{
  BOOL secure = FALSE;

  ENTER();

  if(conn != NULL && conn->isConnected == TRUE)
  {
    if(AmiSSLBase != NULL)
    {
      long error;

      #if defined(__amigaos4__)
      error = InitAmiSSL(AmiSSL_ISocket, conn->socketIFace, TAG_DONE);
      #else
      error = InitAmiSSL(AmiSSL_SocketBase, conn->socketBase, TAG_DONE);
      #endif

      SHOWVALUE(DBF_NET, error);
      if(error == 0)
      {
        char tmp[24+1];
        SSL_METHOD *method = NULL;

        // lets initialize the library first and load the error strings
        // these function don't return any serious error
        SSL_library_init();
        SSL_load_error_strings();

        // We have to feed the random number generator first
        D(DBF_NET, "Seeding random number generator...");
        snprintf(tmp, sizeof(tmp), "%08lx%08lx%08lx", (unsigned long)time((time_t *)NULL), (unsigned long)FindTask(NULL), (unsigned long)rand());
        RAND_seed(tmp, strlen(tmp));

        // prepare the SSL client methods here
        if((method = SSLv23_client_method()) != NULL)
        {
          // get the SSL context
          if((conn->sslCtx = SSL_CTX_new(method)) != NULL)
          {
            #if 0
            char *CAfile = NULL;
            char *CApath = NULL;

            // In future we can give the user the ability to specify his own CA locations
            // in the application instead of using the default ones.
            if(CAfile != NULL || CApath != NULL)
            {
              D(DBF_NET, "CAfile = %s, CApath = %s", SafeStr(CAfile), SafeStr(CApath));
              if((!SSL_CTX_load_verify_locations(conn->sslCtx, CAfile, CApath)))
              {
                E(DBF_NET, "Error setting default verify locations!");
                RETURN(FALSE);
                return FALSE;
              }
            }
            #endif

            // set the default SSL context verify pathes
            if(SSL_CTX_set_default_verify_paths(conn->sslCtx))
            {
              // set the supported cipher list
              if((SSL_CTX_set_cipher_list(conn->sslCtx, "DEFAULT")))
              {
                // for now we set the SSL certificate to 'NONE' as
                // YAM can currently not manage SSL server certificates itself
                // and therefore must be set to not verify them... unsafe?
                // well, I would call it "convienent" for the moment :)
                SSL_CTX_set_verify(conn->sslCtx, SSL_VERIFY_NONE, 0);

                D(DBF_NET, "Initializing TLS/SSL session...");

                // check if we are ready for creating the ssl connection
                if((conn->ssl = SSL_new(conn->sslCtx)) != NULL)
                {
                  // set the socket descriptor to the ssl context
                  if(SSL_set_fd(conn->ssl, (int)conn->socket))
                  {
                    BOOL errorState = FALSE;
                    int res;

                    // establish the ssl connection and take care of non-blocking IO
                    while(errorState == FALSE && (res = SSL_connect(conn->ssl)) <= 0)
                    {
                      int err = SSL_get_error(conn->ssl, res);

                      switch(err)
                      {
                        case SSL_ERROR_WANT_READ:
                        case SSL_ERROR_WANT_WRITE:
                        {
                          // we are using non-blocking socket IO so an SSL_ERROR_WANT_READ
                          // signals that the SSL socket wants us to wait until data
                          // is available and reissue the SSL_connect() command.
                          LONG retVal = -1;
                          int timeoutSum = 0;
                          GET_SOCKETBASE(conn);

                          // now we iterate in a do/while loop and call WaitSelect()
                          // with a static timeout of 1s. We then continue to do so
                          // until we reach the predefined socket timeout
                          do
                          {
                            // check if we are here because of a second iteration
                            if(retVal == 0)
                            {
                              W(DBF_NET, "TCPMakeSecure: recoverable WaitSelect() timeout: %ld", timeoutSum);

                              // give our GUI the chance to update
                              if(conn->connectedFromMainThread == TRUE)
                                DoMethod(G->App, MUIM_Application_InputBuffered);

                              // if retVal is zero, then we are here in a
                              // consequtive iteration, so WaitSelect()
                              // seem to have timed out and we have to check
                              // whether we timed out too much already
                              timeoutSum += 1; // +1s

                              if(timeoutSum >= C->SocketTimeout)
                              {
                                conn->error = CONNECTERR_TIMEDOUT;
                                break;
                              }
                              else if(conn->abort == TRUE)
                              {
                                conn->error = CONNECTERR_ABORTED;
                                break;
                              }
                            }

                            // we do a static timeout of 1s
                            conn->timeout.tv_sec = 1;
                            conn->timeout.tv_usec = 0;

                            // now we put our socket handle into a descriptor set
                            // we can pass on to WaitSelect()
                            FD_ZERO(&conn->fdset);
                            FD_SET(conn->socket, &conn->fdset);

                            // depending on the SSL error (WANT_READ/WANT_WRITE)
                            // we either do a WaitSelect() on the read or write mode
                            // as with SSL both things can happen
                            // see http://www.openssl.org/docs/ssl/SSL_connect.html
                            if(err == SSL_ERROR_WANT_READ)
                              retVal = WaitSelect(conn->socket+1, &conn->fdset, NULL, NULL, (APTR)&conn->timeout, NULL);
                            else
                              retVal = WaitSelect(conn->socket+1, NULL, &conn->fdset, NULL, (APTR)&conn->timeout, NULL);
                          }
                          while(retVal == 0);

                          // if WaitSelect() returns 1 we successfully waited for
                          // being able to write to the socket. So we go and do another
                          // iteration in the while() loop as the next connect() call should
                          // return EISCONN if the connection really succeeded.
                          if(retVal >= 1 && FD_ISSET(conn->socket, &conn->fdset))
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
                      secure = TRUE;

                      // Certificate info
                      // only for debug reasons
                      #if defined(DEBUG)
                      {
                        char *x509buf;
                        SSL_CIPHER *cipher;
                        X509 *server_cert;
                        cipher = SSL_get_current_cipher(conn->ssl);

                        if(cipher != NULL)
                          D(DBF_NET, "%s connection using %s", SSL_CIPHER_get_version(cipher), SSL_get_cipher(conn->ssl));

                        if((server_cert = SSL_get_peer_certificate(conn->ssl)) == NULL)
                          E(DBF_NET, "SSL_get_peer_certificate() error!");

                        D(DBF_NET, "Server public key is %ld bits", EVP_PKEY_bits(X509_get_pubkey(server_cert)));

                        #define X509BUFSIZE 4096

                        if((x509buf = calloc(1, X509BUFSIZE)) != NULL)
                        {
                          D(DBF_NET, "Server certificate:");

                          if(!(X509_NAME_oneline(X509_get_subject_name(server_cert), x509buf, X509BUFSIZE)))
                            E(DBF_NET, "X509_NAME_oneline...[subject] error!");

                          D(DBF_NET, "subject: %s", x509buf);

                          if(!(X509_NAME_oneline(X509_get_issuer_name(server_cert), x509buf, X509BUFSIZE)))
                            E(DBF_NET, "X509_NAME_oneline...[issuer] error!");

                          D(DBF_NET, "issuer:  %s", x509buf);

                          free(x509buf);
                        }

                        if(server_cert != NULL)
                          X509_free(server_cert);
                      }
                      #endif
                    }
                  }
                  else
                    E(DBF_NET, "SSL_set_fd() error, socket %ld", conn->socket);
                }
                else
                  E(DBF_NET, "can't create a new SSL structure for a connection.");
              }
              else
                E(DBF_NET, "SSL_CTX_set_cipher_list() error!");
            }
            else
              E(DBF_NET, "error setting default verify locations!");
          }
          else
            E(DBF_NET, "can't create SSL_CTX object!");
        }
        else
          E(DBF_NET, "SSLv23_client_method() error!");
      }
      else
      {
        E(DBF_NET, "InitAmiSSL() failed");

        ER_NewError(tr(MSG_ER_INITAMISSL));

        G->TR_UseableTLS = FALSE;
      }
    }
    else
      W(DBF_NET, "AmiSSLBase == NULL");

    // if we weren't ale to initialize the TLS/SSL stuff we have to clear it
    // before leaving
    if(secure == FALSE)
    {
      if(conn->sslCtx != NULL)
      {
        SSL_CTX_free(conn->sslCtx);
        conn->sslCtx = NULL;
      }

      conn->ssl = NULL;
      conn->error = CONNECTERR_SSLFAILED;
    }
  }

  RETURN(secure);
  return secure;
}

///
/// ReadFromHost
// an unbuffered implementation/wrapper for SSL_read/recv() where all available
// data upto maxlen will be put into the memory at ptr while taking care of
// non-blocking IO. Returns the number of bytes read or -1 on an error
static int ReadFromHost(struct Connection *conn, char *ptr, const int maxlen)
{
  int result;
  int nread = -1; // -1 is error
  int status = 0; // < 0 error, 0 unknown, > 0 no error
  GET_SOCKETBASE(conn);

  ENTER();

  if(conn->ssl != NULL)
  {
    // use SSL methods to get/process all data
    do
    {
      // read out data and stop our loop in case there
      // isn't anything to read anymore or we have filled
      // maxlen data
      nread = SSL_read(conn->ssl, ptr, maxlen);

      // if nread > 0 we read _some_ data and can
      // break out
      if(nread > 0)
        break;
      else // <= 0 found, check error state
      {
        int err = SSL_get_error(conn->ssl, nread);

        switch(err)
        {
          case SSL_ERROR_WANT_READ:
          case SSL_ERROR_WANT_WRITE:
          {
            // we are using non-blocking socket IO so an SSL_ERROR_WANT_READ
            // signals that the SSL socket wants us to wait until data
            // is available and reissue the SSL_read() command.
            LONG retVal = -1;
            int timeoutSum = 0;

            // now we iterate in a do/while loop and call WaitSelect()
            // with a static timeout of 1s. We then continue to do so
            // until we reach the predefined socket timeout
            do
            {
              // check if we are here because of a second iteration
              if(retVal == 0)
              {
                W(DBF_NET, "TCPRead: recoverable WaitSelect() timeout: %ld", timeoutSum);

                // give our GUI the chance to update
                if(conn->connectedFromMainThread == TRUE)
                  DoMethod(G->App, MUIM_Application_InputBuffered);

                // if retVal is zero, then we are here in a
                // consequtive iteration, so WaitSelect()
                // seem to have timed out and we have to check
                // whether we timed out too much already
                timeoutSum += 1; // +1s

                if(timeoutSum >= C->SocketTimeout)
                {
                  conn->error = CONNECTERR_TIMEDOUT;
                  break;
                }
                else if(conn->abort == TRUE)
                {
                  conn->error = CONNECTERR_ABORTED;
                  break;
                }
              }

              // we do a static timeout of 1s
              conn->timeout.tv_sec = 1;
              conn->timeout.tv_usec = 0;

              // now we put our socket handle into a descriptor set
              // we can pass on to WaitSelect()
              FD_ZERO(&conn->fdset);
              FD_SET(conn->socket, &conn->fdset);

              // depending on the SSL error (WANT_READ/WANT_WRITE)
              // we either do a WaitSelect() on the read or write mode
              // as with SSL both things can happen
              // see http://www.openssl.org/docs/ssl/SSL_read.html
              if(err == SSL_ERROR_WANT_READ)
                retVal = WaitSelect(conn->socket+1, &conn->fdset, NULL, NULL, (APTR)&conn->timeout, NULL);
              else
                retVal = WaitSelect(conn->socket+1, NULL, &conn->fdset, NULL, (APTR)&conn->timeout, NULL);
            }
            while(retVal == 0);

            // if WaitSelect() returns 1 we successfully waited for
            // being able to read from the socket. So we go and do another
            // iteration in the while() loop as the next connect() call should
            // return EISCONN if the connection really succeeded.
            if(retVal >= 1 && FD_ISSET(conn->socket, &conn->fdset))
            {
              // everything fine
              continue;
            }
            else if(retVal == 0)
            {
              W(DBF_NET, "WaitSelect() socket timeout reached");
              status = -1; // signal error
              conn->error = CONNECTERR_TIMEDOUT;
            }
            else
            {
              // the rest should signal an error
              E(DBF_NET, "WaitSelect() returned an error: %ld", err);
              status = -1; // signal error
              conn->error = CONNECTERR_UNKNOWN_ERROR;
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
              conn->error = CONNECTERR_UNKNOWN_ERROR;
            }
          }
          break;

          default:
          {
            E(DBF_NET, "SSL_read() returned an error %ld", err);
            status = -1; // signal error
            conn->error = CONNECTERR_UNKNOWN_ERROR;
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
      nread = recv(conn->socket, ptr, maxlen, 0);

      // if nread >= 0 we read _some_ data or reach the
      // end of the socket and can break out
      if(nread >= 0)
        break;
      else // < 0 found, check error state
      {
        LONG err = -1;
        struct TagItem tags[] =
        {
          { SBTM_GETREF(SBTC_ERRNO), (IPTR)&err },
          { TAG_END,                 0          }
        };

        // get the error value which should normally be set by a recv()
        SocketBaseTagList(tags);

        switch(err)
        {
          case EAGAIN:
          {
            // we are using non-blocking socket IO so an EAGAIN signals
            // that we should wait for more data to arrive before we
            // can issue a recv() again.
            LONG retVal = -1;
            int timeoutSum = 0;

            // now we iterate in a do/while loop and call WaitSelect()
            // with a static timeout of 1s. We then continue to do so
            // until we reach the predefined socket timeout
            do
            {
              // check if we are here because of a second iteration
              if(retVal == 0)
              {
                W(DBF_NET, "TCPRead: recoverable WaitSelect() timeout: %ld", timeoutSum);

                // give our GUI the chance to update
                if(conn->connectedFromMainThread == TRUE)
                  DoMethod(G->App, MUIM_Application_InputBuffered);

                // if retVal is zero, then we are here in a
                // consequtive iteration, so WaitSelect()
                // seem to have timed out and we have to check
                // whether we timed out too much already
                timeoutSum += 1; // +1s

                if(timeoutSum >= C->SocketTimeout)
                {
                  conn->error = CONNECTERR_TIMEDOUT;
                  break;
                }
                else if(conn->abort == TRUE)
                {
                  conn->error = CONNECTERR_ABORTED;
                  break;
                }
              }

              // we do a static timeout of 1s
              conn->timeout.tv_sec = 1;
              conn->timeout.tv_usec = 0;

              // now we put our socket handle into a descriptor set
              // we can pass on to WaitSelect()
              FD_ZERO(&conn->fdset);
              FD_SET(conn->socket, &conn->fdset);
            }
            while((retVal = WaitSelect(conn->socket+1, &conn->fdset, NULL, NULL, (APTR)&conn->timeout, NULL)) == 0);

            // if WaitSelect() returns 1 we successfully waited for
            // being able to read from the socket. So we go and do another
            // iteration in the while() loop as the next connect() call should
            // return EISCONN if the connection really succeeded.
            if(retVal >= 1 && FD_ISSET(conn->socket, &conn->fdset))
            {
              // everything fine
              continue;
            }
            else if(retVal == 0)
            {
              W(DBF_NET, "WaitSelect() socket timeout reached");
              status = -1; // signal error
              conn->error = CONNECTERR_TIMEDOUT;
            }
            else
            {
              // the rest should signal an error
              E(DBF_NET, "WaitSelect() returned error: %ld", err);
              status = -1; // signal error
              conn->error = CONNECTERR_UNKNOWN_ERROR;
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
            conn->error = CONNECTERR_UNKNOWN_ERROR;
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
/// ReadFromHostBuffered
// a buffered implementation of read()/recv() which is somehow compatible
// to the I/O fgets() function. It will read out data from the socket in
// 4096 chunks and store them into a static buffer. This function SHOULD
// only be called by TCPReceiveLine() and TCPReceive().
//
// This implementation is a slightly adapted version of readline()/my_read()
// examples from (W.Richard Stevens - Unix Network Programming) - Page 80
// However, they were adapted to non-blocking IO whereever necessary
static int ReadFromHostBuffered(struct Connection *conn, char *ptr, const int maxlen)
{
  int result = -1; // -1 = error

  ENTER();

  // now comes the 'real' socket read stuff where
  // we obtain as much data as possible up to the maxlen amount.

  // if the buffer is empty we fill it again from
  // data we request from the socket. Here we make sure we
  // take care of non-blocking IO
  if(conn->receiveCount <= 0)
  {
    // read all data upto the maximum our buffer allows
    conn->receiveCount = ReadFromHost(conn, conn->receiveBuffer, conn->receiveBufferSize);

    // reset the read_ptr
    conn->receivePtr = conn->receiveBuffer;
  }

  if(conn->receiveCount > 0)
  {
    // we copy only the minmum of read_cnt and maxlen
    int fillCount = MIN(conn->receiveCount, maxlen);

    // to speed up the ReadLine we only use memcpy()
    // for requests > 1
    if(fillCount > 1)
    {
      // now we copy a maximum of maxlen chars to
      // the destination
      memcpy(ptr, conn->receivePtr, fillCount);
      conn->receiveCount -= fillCount;
      conn->receivePtr += fillCount;
    }
    else
    {
      conn->receiveCount--;
      *ptr = *conn->receivePtr++;
    }

    result = fillCount;
  }

  RETURN(result);
  return result;
}

///
/// ReceiveLineFromHost
// a buffered version of readline() that reads out charwise from the
// socket via TCPReadBuffered and returns the amount of chars copied
// into the provided character array or -1 on error.
//
// This implementation is a slightly adapted version of readline()/my_read()
// examples from (W.Richard Stevens - Unix Network Programming) - Page 80
int ReceiveLineFromHost(struct Connection *conn, char *vptr, const int maxlen)
{
  int result = -1;

  ENTER();

  if(conn != NULL)
  {
    // make sure the socket is active.
    if(conn->isConnected == TRUE)
    {
      int n;
      char *ptr = vptr;

      conn->error = CONNECTERR_NO_ERROR;

      for(n = 1; n < maxlen; n++)
      {
        int rc;
        char c;

        // read out one buffered char only.
        rc = ReadFromHostBuffered(conn, &c, 1);
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
    else
    {
      conn->error = CONNECTERR_NOT_CONNECTED;
    }
  }

  RETURN(result);
  return result;
}

///
/// ReceiveFromHost
// a own wrapper function for recv()/SSL_read() that reads buffered somehow
// it reads maxlen-1 data out of the buffer and stores it into recvdata with
// a null terminated string
int ReceiveFromHost(struct Connection *conn, char *recvdata, const int maxlen)
{
  int nread = -1;

  ENTER();

  if(conn != NULL)
  {
    // make sure the socket is active.
    if(conn->isConnected == TRUE)
    {
      conn->error = CONNECTERR_NO_ERROR;

      // we call the ReadBuffered function so that
      // we get out the data from our own buffer.
      nread = ReadFromHostBuffered(conn, recvdata, maxlen-1);

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
    {
      W(DBF_NET, "socket not connected");
      conn->error = CONNECTERR_NOT_CONNECTED;
    }
  }

  RETURN(nread);
  return nread;
}

///
/// WriteToHost
// an unbuffered implementation/wrapper for SSL_write/send() where the supplied
// data in ptr will be send out straight away while taking care of non-blocking IO.
// returns the number of bytes written or -1 on an error
static int WriteToHost(struct Connection *conn, const char *ptr, const int len)
{
  int result;
  int towrite = len;
  int status = 0; // < 0 error, 0 unknown, > 0 no error
  GET_SOCKETBASE(conn);

  ENTER();

  if(conn->ssl != NULL)
  {
    // use SSL methods to get/process all data
    do
    {
      // write data to our socket and then check how much
      // we have written and in case we weren't able to write
      // all out, we retry it with smaller chunks.
      int nwritten = SSL_write(conn->ssl, ptr, towrite);

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
        int err = SSL_get_error(conn->ssl, nwritten);

        switch(err)
        {
          case SSL_ERROR_WANT_READ:
          case SSL_ERROR_WANT_WRITE:
          {
            // we are using non-blocking socket IO so an SSL_ERROR_WANT_READ
            // signals that the SSL socket wants us to wait until data
            // is available and reissue the SSL_read() command.
            LONG retVal = -1;
            int timeoutSum = 0;

            // now we iterate in a do/while loop and call WaitSelect()
            // with a static timeout of 1s. We then continue to do so
            // until we reach the predefined socket timeout
            do
            {
              // check if we are here because of a second iteration
              if(retVal == 0)
              {
                W(DBF_NET, "TCPWrite: recoverable WaitSelect() timeout: %ld", timeoutSum);

                // give our GUI the chance to update
                if(conn->connectedFromMainThread == TRUE)
                  DoMethod(G->App, MUIM_Application_InputBuffered);

                // if retVal is zero, then we are here in a
                // consequtive iteration, so WaitSelect()
                // seem to have timed out and we have to check
                // whether we timed out too much already
                timeoutSum += 1; // +1s

                if(timeoutSum >= C->SocketTimeout)
                {
                  conn->error = CONNECTERR_TIMEDOUT;
                  break;
                }
                else if(conn->abort == TRUE)
                {
                  conn->error = CONNECTERR_ABORTED;
                  break;
                }
              }

              // we do a static timeout of 1s
              conn->timeout.tv_sec = 1;
              conn->timeout.tv_usec = 0;

              // now we put our socket handle into a descriptor set
              // we can pass on to WaitSelect()
              FD_ZERO(&conn->fdset);
              FD_SET(conn->socket, &conn->fdset);

              // depending on the SSL error (WANT_READ/WANT_WRITE)
              // we either do a WaitSelect() on the read or write mode
              // as with SSL both things can happen
              // see http://www.openssl.org/docs/ssl/SSL_write.html
              if(err == SSL_ERROR_WANT_READ)
                retVal = WaitSelect(conn->socket+1, &conn->fdset, NULL, NULL, (APTR)&conn->timeout, NULL);
              else
                retVal = WaitSelect(conn->socket+1, NULL, &conn->fdset, NULL, (APTR)&conn->timeout, NULL);
            }
            while(retVal == 0);

            // if WaitSelect() returns 1 we successfully waited for
            // being able to write to the socket. So we go and do another
            // iteration in the while() loop as the next connect() call should
            // return EISCONN if the connection really succeeded.
            if(retVal >= 1 && FD_ISSET(conn->socket, &conn->fdset))
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
      int nwritten = send(conn->socket, (APTR)ptr, towrite, 0);

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
        struct TagItem tags[] =
        {
          { SBTM_GETREF(SBTC_ERRNO), (IPTR)&err },
          { TAG_END,                0          }
        };

        // get the error value which should normally be set by a send()
        SocketBaseTagList(tags);

        switch(err)
        {
          case EAGAIN:
          {
            // we are using non-blocking socket IO so an EAGAIN signals
            // that we should wait for more data to arrive before we
            // can issue a recv() again.
            LONG retVal = -1;
            int timeoutSum = 0;

            // now we iterate in a do/while loop and call WaitSelect()
            // with a static timeout of 1s. We then continue to do so
            // until we reach the predefined socket timeout
            do
            {
              // check if we are here because of a second iteration
              if(retVal == 0)
              {
                W(DBF_NET, "TCPWrite: recoverable WaitSelect() timeout: %ld", timeoutSum);

                // give our GUI the chance to update
                if(conn->connectedFromMainThread == TRUE)
                  DoMethod(G->App, MUIM_Application_InputBuffered);

                // if retVal is zero, then we are here in a
                // consequtive iteration, so WaitSelect()
                // seem to have timed out and we have to check
                // whether we timed out too much already
                timeoutSum += 1; // +1s

                if(timeoutSum >= C->SocketTimeout)
                {
                  conn->error = CONNECTERR_TIMEDOUT;
                  break;
                }
                else if(conn->abort == TRUE)
                {
                  conn->error = CONNECTERR_ABORTED;
                  break;
                }
              }

              // we do a static timeout of 1s
              conn->timeout.tv_sec = 1;
              conn->timeout.tv_usec = 0;

              // now we put our socket handle into a descriptor set
              // we can pass on to WaitSelect()
              FD_ZERO(&conn->fdset);
              FD_SET(conn->socket, &conn->fdset);
            }
            while((retVal = WaitSelect(conn->socket+1, NULL, &conn->fdset, NULL, (APTR)&conn->timeout, NULL)) == 0);

            // if WaitSelect() returns 1 we successfully waited for
            // being able to write to the socket. So we go and do another
            // iteration in the while() loop as the next connect() call should
            // return EISCONN if the connection really succeeded.
            if(retVal >= 1 && FD_ISSET(conn->socket, &conn->fdset))
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
    E(DBF_NET, "TCPWrite() short item count: %ld of %ld transfered!", len-towrite, len);
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
/// WriteToHostBuffered
// a buffered implementation of SSL_write()/send(). This function will buffer
// all write operations in a temporary buffer and as soon as the buffer is
// full or it will be explicitly flushed, it will write out the data to the
// socket. This function SHOULD only be called by TCPSend() somehow.
//
// This function will also optimize the write operations by always making
// sure that a full buffer will be written out to the socket. i.e. if the
// buffer is filled up so that the next call would flush it, we copy as
// many data to the buffer as possible and flush it immediately.
static int WriteToHostBuffered(struct Connection *conn, const char *ptr, int maxlen, const int flags)
{
  int result = -1; // -1 = error
  int fillCount = 0;
  BOOL abortProc = FALSE;

  ENTER();

  // in case our write buffer is already filled up or
  // the caller wants to just flush the buffer we immediately use
  // the socket functions send/SSL_write() to transfer everything
  // and therefore clear the buffer.
  if(conn->sendCount >= conn->sendBufferSize || hasTCP_ONLYFLUSH(flags))
  {
    // make sure we write out the data to the socket
    if(WriteToHost(conn, conn->sendBuffer, conn->sendCount) != conn->sendCount)
    {
      // abort and signal an error
      abortProc = TRUE;
    }
    else
    {
      // set the ptr to the start of the buffer
      conn->sendPtr = conn->sendBuffer;
      conn->sendCount = 0;

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
    // wouldn't fit, we copy as much as we can, clear the buffer
    // and continue until there is enough space left
    while(conn->sendCount+maxlen > conn->sendBufferSize)
    {
      int fillable = conn->sendBufferSize - conn->sendCount;

      // after the copy we have to increase the pointer of the
      // array we want to copy, because in the next cycle we have to
      // copy the rest out of it.
      memcpy(conn->sendPtr, ptr, fillable);
      ptr += fillable;
      conn->sendCount += fillable;
      fillCount += fillable;

      // decrease maxlen now by the amount of bytes we have written
      // to the buffer
      maxlen -= fillable;

      // now we write out the data
      if(WriteToHost(conn, conn->sendBuffer, conn->sendCount) != conn->sendCount)
      {
        // abort and signal an error
        abortProc = TRUE;
        break;
      }
      else
      {
        // set the ptr to the start of the buffer
        // as we flushed everything
        conn->sendPtr = conn->sendBuffer;
        conn->sendCount = 0;
      }
    }

    // check if we should abort
    if(abortProc == FALSE)
    {
      // if we end up here we have enough space for our
      // string in the buffer, so lets copy it in
      memcpy(conn->sendPtr, ptr, maxlen);
      conn->sendPtr += maxlen;
      conn->sendCount += maxlen;
      fillCount += maxlen;

      // if the user has supplied the FLUSH flag we have to clear/flush
      // the buffer immediately after having copied everything
      if(hasTCP_FLUSH(flags))
      {
        // write our whole buffer out to the socket
        if(WriteToHost(conn, conn->sendBuffer, conn->sendCount) != conn->sendCount)
        {
          // abort and signal an error
          abortProc = TRUE;
        }
        else
        {
          // set the ptr to the start of the buffer
          // as we flushed everything
          conn->sendPtr = conn->sendBuffer;
          conn->sendCount = 0;
        }
      }

      // if we still haven't received an abort signal
      // we can finally set the result to fillCount
      if(abortProc == FALSE)
        result = fillCount;
    }
  }

  RETURN(result);
  return result;
}

///
/// SendToHost
// a own wrapper function for send()/SSL_write() that writes buffered somehow
// if called with flag != TCP_FLUSH - otherwise it will write and flush immediately
int SendToHost(struct Connection *conn, const char *ptr, const int len, const int flags)
{
  int nwritten = -1;

  ENTER();

  if(conn != NULL)
  {
    // make sure the socket is active.
    if(conn->isConnected == TRUE)
    {
      conn->error = CONNECTERR_NO_ERROR;

      // perform some debug output on the console if requested
      // by the user
      if(G->TR_Debug == TRUE && ptr != NULL)
        printf("CLIENT[%04d]: %s", len, ptr);

      // we call the WriteBuffered() function to write this characters
      // out to the socket. the provided flag will define if it
      // really will be buffered or if we write and flush the buffer
      // immediatly
      nwritten = WriteToHostBuffered(conn, ptr, len, flags);

      D(DBF_NET, "TCP: sent %ld of %ld bytes (%ld)", nwritten, len, flags);
    }
    else
    {
      W(DBF_NET, "socket not connected");
      conn->error = CONNECTERR_NOT_CONNECTED;
    }
  }

  RETURN(nwritten);
  return nwritten;
}

///
/// SendLineToHost
// send out a line of text
int SendLineToHost(struct Connection *conn, const char *vptr)
{
  int result;

  ENTER();

  result = SendToHost(conn, vptr, strlen(vptr), TCPF_FLUSH);

  RETURN(result);
  return result;
}

///
/// FlushConnection
// send out any still pending data
int FlushConnection(struct Connection *conn)
{
  int result;

  ENTER();

  result = SendToHost(conn, NULL, 0, TCPF_FLUSHONLY);

  RETURN(result);
  return result;
}

///

