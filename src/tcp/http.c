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
#include <stdio.h>

#include "YAM_config.h"
#include "YAM_error.h"
#include "YAM_global.h"

#include "Locale.h"

#include "tcp/Connection.h"

#include "Debug.h"

/**************************************************************************/
// static function prototypes

/**************************************************************************/
// local macros & defines

/***************************************************************************
 Module: HTTP protocol
***************************************************************************/

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
