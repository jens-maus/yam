/***************************************************************************

 YAM - Yet Another Mailer
 Copyright (C) 1995-2000 Marcel Beck
 Copyright (C) 2000-2011 YAM Open Source Team

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

#include "YAM.h"
#include "YAM_config.h"
#include "YAM_error.h"
#include "YAM_global.h"

#include "Locale.h"
#include "MethodStack.h"
#include "Threads.h"

#include "mui/TransferControlGroup.h"
#include "mui/YAMApplication.h"
#include "tcp/Connection.h"
#include "tcp/http.h"

#include "Debug.h"

struct TransferContext
{
  struct Connection *connection;
  Object *transferGroup;
  int hport;

  int (* receiveFunc)(struct Connection *, char *, const int); // binary or text based receive function
  LONG contentLength;

  char transferGroupTitle[SIZE_DEFAULT]; // the TransferControlGroup's title
  char url[SIZE_URL];
  char host[SIZE_HOST];
  char serverPath[SIZE_LINE];
  char requestResponse[SIZE_LINE];
  char redirectedURL[SIZE_URL];
};

/// ReceiveHTTPHeader
// receive a standard HTTP header
BOOL ReceiveHTTPHeader(struct TransferContext *tc)
{
  BOOL success = FALSE;
  int len;

  ENTER();

  // default to a binary receive function
  tc->receiveFunc = ReceiveFromHost;
  tc->contentLength = 0;

  // we can request all further lines from our socket
  // until we reach the entity body
  while(tc->connection->error == CONNECTERR_NO_ERROR &&
        (len = ReceiveLineFromHost(tc->connection, tc->requestResponse, sizeof(tc->requestResponse))) > 0)
  {
    SHOWSTRING(DBF_NET, tc->requestResponse);

    // we scan for the end of the response header by searching for the first '\r\n' line
    if(strnicmp(tc->requestResponse, "Content-Length:", 15) == 0)
    {
      tc->contentLength = atoi(&tc->requestResponse[16]);
    }
    else if(strnicmp(tc->requestResponse, "Content-Type:", 13) == 0)
    {
      // for text bodies we use a line based receive function
      if(strnicmp(&tc->requestResponse[14], "text", 4) == 0)
      {
        D(DBF_NET, "using line based content receive function");
        tc->receiveFunc = ReceiveLineFromHost;
      }
    }
    else if(strnicmp(tc->requestResponse, "Location:", 9) == 0)
    {
      // remember the redirected URL
      strlcpy(tc->redirectedURL, &tc->requestResponse[10], sizeof(tc->redirectedURL));
      // strip the trainling CR+LF
      if(strlen(tc->redirectedURL) > 2)
        tc->redirectedURL[strlen(tc->redirectedURL)-2] = '\0';
    }
    else if(strcmp(tc->requestResponse, "\r\n") == 0)
    {
      // this is the end...
      success = TRUE;
      break;
    }
  }

  RETURN(success);
  return success;
}

///
/// ReceiveHTTPBody
// receive the body document of a HTTP request to a file
BOOL ReceiveHTTPBody(struct TransferContext *tc, const char *filename)
{
  BOOL success = FALSE;

  if(tc->contentLength > 0)
  {
    FILE *out = NULL;

    // prepare the output file
    if(filename != NULL)
    {
      D(DBF_NET, "downloading to file '%s'", filename);
      out = fopen(filename, "w");
    }

    if(filename == NULL || out != NULL)
    {
      LONG received = -1;
      int len;

      setvbuf(out, NULL, _IOFBF, SIZE_FILEBUF);

      PushMethodOnStack(tc->transferGroup, 3, MUIM_TransferControlGroup_Start, 1, tc->contentLength);
      PushMethodOnStack(tc->transferGroup, 5, MUIM_TransferControlGroup_Next, 0, 1, tc->contentLength, tr(MSG_HTTP_RECEIVING_DATA));

      // we seem to have reached the entity body, so
      // from here we retrieve everything we can get and
      // immediately write it out to a file. that's it :)
      while(tc->connection->error == CONNECTERR_NO_ERROR &&
            (len = tc->receiveFunc(tc->connection, tc->requestResponse, sizeof(tc->requestResponse))) > 0)
      {
        PushMethodOnStack(tc->transferGroup, 3, MUIM_TransferControlGroup_Update, len, tr(MSG_HTTP_RECEIVING_DATA));

        if(out != NULL && fwrite(tc->requestResponse, len, 1, out) != 1)
        {
          received = -1; // signal an error!
          break;
        }

        // forget the initial value and sum up all further sizes
        if(received == -1)
          received = len;
        else
          received += len;
      }

      D(DBF_NET, "received %ld bytes", received);

      PushMethodOnStack(tc->transferGroup, 3, MUIM_TransferControlGroup_Update, TCG_SETMAX, tr(MSG_HTTP_RECEIVING_DATA));

      // check if we retrieved anything
      if(tc->connection->error == CONNECTERR_NO_ERROR && received == tc->contentLength)
        success = TRUE;

      PushMethodOnStack(tc->transferGroup, 1, MUIM_TransferControlGroup_Finish);

      if(out != NULL)
        fclose(out);
    }
    else
      ER_NewError(tr(MSG_ER_CantCreateFile), filename);
  }
  else
  {
    // zero content is treated as immediate success
    success = TRUE;
  }
  
  RETURN(success);
  return success;
}

///
/// DownloadURL
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

redirected:
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

      snprintf(tc->transferGroupTitle, sizeof(tc->transferGroupTitle), tr(MSG_TR_DOWNLOADING_FROM_SERVER), tc->host);

      // create a new transfer window
      if(tc->transferGroup == NULL)
        tc->transferGroup = (Object *)PushMethodOnStackWait(G->App, 6, MUIM_YAMApplication_CreateTransferGroup, CurrentThread(), tc->transferGroupTitle, tc->connection, TRUE, isFlagSet(flags, DLURLF_VISIBLE));
        
      if(tc->transferGroup != NULL)
      {
        PushMethodOnStack(tc->transferGroup, 3, MUIM_Set, MUIA_TransferControlGroup_MailMode, FALSE);
        PushMethodOnStack(tc->transferGroup, 2, MUIM_TransferControlGroup_ShowStatus, tr(MSG_HTTP_CONNECTING_TO_SERVER));

        // open the TCP/IP connection to 'host' under the port 'hport'
        if((ConnectToHost(tc->connection, tc->host, tc->hport)) == CONNECTERR_SUCCESS)
        {
          char *serverHost;
          char *port;

          // update the AppIcon now that the connection was established
          PushMethodOnStack(G->App, 1, MUIM_YAMApplication_UpdateAppIcon);

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
          PushMethodOnStack(tc->transferGroup, 2, MUIM_TransferControlGroup_ShowStatus, tr(MSG_HTTP_SENDING_REQUEST));
          if(SendLineToHost(tc->connection, tc->requestResponse) > 0)
          {
            char *p;
            int len;
            int error = 0;

            // now we read out the very first line to see if the
            // response code matches and is fine
            PushMethodOnStack(tc->transferGroup, 2, MUIM_TransferControlGroup_ShowStatus, tr(MSG_HTTP_WAITING_FOR_ANSWER));
            len = ReceiveLineFromHost(tc->connection, tc->requestResponse, sizeof(tc->requestResponse));

            SHOWSTRING(DBF_NET, tc->requestResponse);

            // check the server response
            if(len > 0 && strnicmp(tc->requestResponse, "HTTP/", 5) == 0 && (p = strchr(tc->requestResponse, ' ')) != NULL)
            {
              error = atoi(TrimStart(p));

              switch(error)
              {
                case 200: // OK
                {
                  PushMethodOnStack(tc->transferGroup, 2, MUIM_TransferControlGroup_ShowStatus, tr(MSG_HTTP_RECEIVING_DATA));

                  if(ReceiveHTTPHeader(tc) == TRUE)
                  {
                    // now receive the desired file contents
                    success = ReceiveHTTPBody(tc, filename);
                    break;
                  }
                }
                break;

                case 301: // Moved Permanently
                case 302: // Found
                {
                  // receive redirection header
                  if(ReceiveHTTPHeader(tc) == TRUE)
                  {
                    // receive the body, but ignore its contents
                    if(ReceiveHTTPBody(tc, NULL) == TRUE)
                    {
                      PushMethodOnStack(tc->transferGroup, 2, MUIM_TransferControlGroup_ShowStatus, tr(MSG_HTTP_DISCONNECTING_FROM_SERVER));
                      DisconnectFromHost(tc->connection);
                      request = NULL;
                      server = tc->redirectedURL;
                      goto redirected;
                    }
                  }
                }
                break;

                case 404: // Not Found
                {
                  if(isFlagClear(flags, DLURLF_NO_ERROR_ON_404))
                    ER_NewError(tr(MSG_ER_DocNotFound), path);
                }
                break;
              }
            }
          }
          else
            ER_NewError(tr(MSG_ER_SendHTTP));
        }
        else
          ER_NewError(tr(MSG_ER_ConnectHTTP), tc->host);

        PushMethodOnStack(tc->transferGroup, 2, MUIM_TransferControlGroup_ShowStatus, tr(MSG_HTTP_DISCONNECTING_FROM_SERVER));
        DisconnectFromHost(tc->connection);

        PushMethodOnStack(G->App, 2, MUIM_YAMApplication_DeleteTransferGroup, tc->transferGroup);
      }

      // update the AppIcon after closing down the connection
      PushMethodOnStack(G->App, 1, MUIM_YAMApplication_UpdateAppIcon);
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
