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
#include "Threads.h"

#include "tcp/Connection.h"
#include "tcp/http.h"

#include "Debug.h"

struct TransferContext
{
  struct Connection *connection;
  int hport;
  char url[SIZE_URL];
  char host[SIZE_HOST];
  char serverPath[SIZE_LINE];
  char requestResponse[SIZE_LINE];
};

/// DownloadURL()
//  Downloads a file from the web using HTTP/1.1 (RFC 2616)
BOOL DownloadURL(const char *server, const char *request, const char *filename, const ULONG flags)
{
  BOOL success = FALSE;
  struct TransferContext *tc;

  ENTER();

  if((tc = calloc(1, sizeof(*tc))) != NULL)
  {
    if((tc->connection = CreateConnection()) != NULL && ConnectionIsOnline(tc->connection) == TRUE)
    {
      BOOL noproxy = (C->ProxyServer[0] == '\0');
      char *path;
      char *bufptr;

      // extract the server address and strip the http:// part
      // of the URI
      if(strnicmp(server, "http://", 7) == 0)
        strlcpy(tc->url, &server[7], sizeof(tc->url));
      else
        strlcpy(tc->url, server, sizeof(tc->url));

      // in case an explicit request was given we
      // add it here
      if(request != NULL)
      {
        if(tc->url[strlen(tc->url)-1] != '/')
          strlcat(tc->url, "/", sizeof(tc->url));

        strlcat(tc->url, request, sizeof(tc->url));
      }

      // find the first occurance of the '/' separator in out
      // url and insert a terminating NUL character
      if((path = strchr(tc->url, '/')) != NULL)
        *path++ = '\0';
      else
        path = (char *)"";

      // extract the hostname from the URL or use the proxy server
      // address if specified.
      strlcpy(tc->host, noproxy ? tc->url : C->ProxyServer, sizeof(tc->host));

      // extract the port on which we connect if the
      // hostname contain an ':' separator
      if((bufptr = strchr(tc->host, ':')) != NULL)
      {
        *bufptr++ = '\0';
        tc->hport = atoi(bufptr);
      }
      else
        tc->hport = noproxy ? 80 : 8080;

      // open the TCP/IP connection to 'host' under the port 'hport'
      if((ConnectToHost(tc->connection, tc->host, tc->hport)) == CONNECTERR_SUCCESS)
      {
        char *serverHost;
        char *port;

        // now we build the HTTP request we send out to the HTTP
        // server
        if(noproxy == TRUE)
        {
          snprintf(tc->serverPath, sizeof(tc->serverPath), "/%s", path);
          serverHost = tc->host;
        }
        else if((port = strchr(tc->url, ':')) != NULL)
        {
          *port++ = '\0';

          snprintf(tc->serverPath, sizeof(tc->serverPath), "http://%s:%s/%s", tc->url, port, path);
          serverHost = tc->url;
        }
        else
        {
          snprintf(tc->serverPath, sizeof(tc->serverPath), "http://%s/%s", tc->url, path);
          serverHost = tc->url;
        }

        // construct the HTTP request
        // we send a HTTP/1.0 request because 1.1 implies that we have to be able
        // to deal with e.g. "Transfer-Encoding: chunked" responses which we can't handle
        // right now.
        snprintf(tc->requestResponse, sizeof(tc->requestResponse), "GET %s HTTP/1.0\r\n"
                                                                   "Host: %s\r\n"
                                                                   "User-Agent: %s\r\n"
                                                                   "Connection: close\r\n"
                                                                   "Accept: */*\r\n"
                                                                   "\r\n", tc->serverPath, serverHost, yamuseragent);

        SHOWSTRING(DBF_NET, tc->requestResponse);

        // send out the httpRequest
        if(SendLineToHost(tc->connection, tc->requestResponse) > 0)
        {
          char *p;
          int len;

          // now we read out the very first line to see if the
          // response code matches and is fine
          len = ReceiveLineFromHost(tc->connection, tc->requestResponse, sizeof(tc->requestResponse));

          SHOWSTRING(DBF_NET, tc->requestResponse);

          // check the server response
          if(len > 0 && strnicmp(tc->requestResponse, "HTTP/", 5) == 0 &&
             (p = strchr(tc->requestResponse, ' ')) != NULL && atoi(TrimStart(p)) == 200)
          {
            // we can request all further lines from our socket
            // until we reach the entity body
            while(tc->connection->error == CONNECTERR_NO_ERROR &&
                  (len = ReceiveLineFromHost(tc->connection, tc->requestResponse, sizeof(tc->requestResponse))) > 0)
            {
              // we scan for the end of the
              // response header by searching for the first '\r\n'
              // line
              if(strcmp(tc->requestResponse, "\r\n") == 0)
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
                  while(tc->connection->error == CONNECTERR_NO_ERROR &&
                        (len = ReceiveLineFromHost(tc->connection, tc->requestResponse, sizeof(tc->requestResponse))) > 0)
                  {
                    if(fwrite(tc->requestResponse, len, 1, out) != 1)
                    {
                      retrieved = -1; // signal an error!
                      break;
                    }

                    retrieved += len;
                  }

                  D(DBF_NET, "received %ld bytes", retrieved);

                  // check if we retrieved anything
                  if(tc->connection->error == CONNECTERR_NO_ERROR && retrieved >= 0)
                    success = TRUE;

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
        ER_NewError(tr(MSG_ER_ConnectHTTP), tc->host);

      DisconnectFromHost(tc->connection);
    }

    DeleteConnection(tc->connection);
    free(tc);
  }

  // wake up the calling thread if this is requested
  if(isFlagSet(flags, DLURLF_SIGNAL))
    WakeupThread(NULL);

  RETURN(success);
  return success;
}

///
