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

#include <proto/bsdsocket.h>
#include <proto/exec.h>
#include <proto/muimaster.h>
#include <proto/locale.h>

#include "YAM.h"
#include "YAM_config.h"
#include "YAM_error.h"
#include "YAM_global.h"
#include "YAM_mime.h"
#include "YAM_locale.h"
#include "YAM_transfer.h"

#include "UpdateCheck.h"

#include "Debug.h"

/*** Static variables/functions ***/

/*** Update-Check mechanisms ***/
/// CheckForUpdates
// contacts the 'update.yam.ch' HTTP server and asks for
// specific updates.
BOOL CheckForUpdates(void)
{
  BOOL result = FALSE;

  ENTER();

  // first we check if we can start a connection or if the
  // tcp/ip stuff is busy right now so that we do not interrupt something
  if(SocketBase && G->TR_Socket != SMTP_NO_SOCKET)
  {
    RETURN(result);
    return result;
  }

  // now we open a new TCP/IP connection socket
  if(TR_OpenTCPIP())
  {
    struct TempFile *tf = OpenTempFile(NULL);
    if(tf != NULL)
    {
      char buf[SIZE_LINE];
      char request[SIZE_URL];

      BusyText(GetStr(MSG_BusyGettingVerInfo), "");

      // now we prepare our request string which we send to our update server
      // and will inform it about our configuration/YAM version and so on.

      // encode the yam version
      if(urlencode(buf, yamversion, SIZE_LINE) > 0)
        snprintf(request, SIZE_URL, "?ver=%s", buf);

      // encode the yam buildid if present
      if(urlencode(buf, yambuildid, SIZE_LINE) > 0)
        snprintf(request, SIZE_URL, "%s&buildid=%s", request, buf);

      // encode the yam builddate if present
      if(urlencode(buf, yamversiondate, SIZE_LINE) > 0)
        snprintf(request, SIZE_URL, "%s&builddate=%s", request, buf);

      // encode the language in which YAM is running
      if(G->Catalog && urlencode(buf, G->Catalog->cat_Language, SIZE_LINE) > 0)
        snprintf(request, SIZE_URL, "%s&lang=%s%%20%d%%2E%d", request, buf, G->Catalog->cat_Version, G->Catalog->cat_Revision);

      // encode the exec version
      snprintf(request, SIZE_URL, "%s&exec=%d%%2E%d", request, SysBase->lib_Version, SysBase->lib_Revision);

      // encode the MUI version
      snprintf(request, SIZE_URL, "%s&mui=%d%%2E%d", request, MUIMasterBase->lib_Version, MUIMasterBase->lib_Revision);

      // now we send a specific request via TR_DownloadURL() to
      // our update server
      if(TR_DownloadURL(C->UpdateServer, request, NULL, tf->Filename))
      {
        // now we parse the result.
        if((tf->FP = fopen(tf->Filename, "r")))
        {
          while(GetLine(tf->FP, buf, SIZE_LINE))
          {
            D(DBF_STARTUP, "%s", buf);
          }

          fclose(tf->FP);
          tf->FP = NULL;
        }
        else
          ER_NewError(GetStr(MSG_ER_CantOpenTempfile), tf->Filename);
      }

      BusyEnd();

      CloseTempFile(tf);
    }

    TR_CloseTCPIP();
  }
  else
    ER_NewError(GetStr(MSG_ER_OPENTCPIP));

  RETURN(result);
  return result;
}

///
