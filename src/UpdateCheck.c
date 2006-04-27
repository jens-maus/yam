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
#include <proto/timer.h>

#include "YAM.h"
#include "YAM_config.h"
#include "YAM_configFile.h"
#include "YAM_error.h"
#include "YAM_global.h"
#include "YAM_mime.h"
#include "YAM_locale.h"
#include "YAM_transfer.h"

#include "classes/Classes.h"
#include "UpdateCheck.h"

#include "Debug.h"

/*** Static variables/functions ***/

/*** Update-Check mechanisms ***/
/// InitUpdateCheck
// initializes all update-check relevant stuff (during startup) so that
// our autocheck is running properly.
void InitUpdateCheck(BOOL initial)
{
  ENTER();

  // we do check when the next update check have to be issued at so
  // that we can start our timer request accordingly.
  if(C->UpdateInterval > 0)
  {
    struct TimeVal now;
    struct TimeVal nextCheck;

    // as this might be the very first call to this function we have to
    // make sure we issue an update check timer.
    nextCheck.Seconds       = C->LastUpdateCheck.Seconds + C->UpdateInterval;
    nextCheck.Microseconds  = C->LastUpdateCheck.Microseconds;

    // get the current time
    GetSysTime(&now);

    // compare it against the last checked time we have
    // in our config and if greater than we go and do an immediate update
    // check.
    if(initial && CmpTime(&now, &nextCheck) <= 0)
    {
      D(DBF_UPDATE, "update-check is due to be processed NOW.");

      // instead of calling CheckForUpdates() directly, we issue
      // a timer to timeout in 1 milliseconds
      TC_Restart(TIO_UPDATECHECK, 0, 1);
    }
    else
    {
      // we now (re)issue the next update check with the same update
      // interval as our previous one.
      D(DBF_UPDATE, "update-check is due to be processed in %d seconds.", nextCheck.Seconds-now.Seconds);
      TC_Restart(TIO_UPDATECHECK, nextCheck.Seconds-now.Seconds, 0);
    }
  }
  else
  {
    // make sure the updatecheck timer is not running anymore
    TC_Stop(TIO_UPDATECHECK);
  }

  LEAVE();
}

///
/// CheckForUpdates
// contacts the 'update.yam.ch' HTTP server and asks for
// specific updates.
BOOL CheckForUpdates(void)
{
  BOOL result = FALSE;

  ENTER();

  // flag the last update to be failed per default first
  C->LastUpdateStatus = UST_NOQUERY;

  // first we check if we can start a connection or if the
  // tcp/ip stuff is busy right now so that we do not interrupt something
  if(SocketBase == NULL || G->TR_Socket == SMTP_NO_SOCKET)
  {
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
          snprintf(request, SIZE_URL, "%s&lang=%s%%20%d%%2E%d", request, buf, G->Catalog->cat_Version,
                                                                              G->Catalog->cat_Revision);

        // encode the exec version
        snprintf(request, SIZE_URL, "%s&exec=%d%%2E%d", request,
                                                        ((struct Library *)SysBase)->lib_Version,
                                                        ((struct Library *)SysBase)->lib_Revision);

        // encode the MUI version
        snprintf(request, SIZE_URL, "%s&mui=%d%%2E%d", request, MUIMasterBase->lib_Version,
                                                                MUIMasterBase->lib_Revision);

        // now we send a specific request via TR_DownloadURL() to
        // our update server
        if(TR_DownloadURL(C->UpdateServer, request, NULL, tf->Filename))
        {
          // now we parse the result.
          if((tf->FP = fopen(tf->Filename, "r")))
          {
            BOOL updatesAvailable = FALSE;
            struct UpdateComponent *comp = NULL;

            // make sure we clear an eventually existing update window
            if(G->UpdateNotifyWinObject)
              DoMethod(G->UpdateNotifyWinObject, MUIM_UpdateNotifyWindow_Clear);

            while(GetLine(tf->FP, buf, SIZE_LINE))
            {
              D(DBF_UPDATE, "%s", buf);

              // if we find an '[UPDATE]' tag, we can go and create an
              // update notify window in advance and fill it according to the
              // followed information
              if(strcmp(buf, "[UPDATE]") == 0)
              {
                // make sure that we have created the update notification
                // window in advance.
                if(!G->UpdateNotifyWinObject)
                {
                  if((G->UpdateNotifyWinObject = UpdateNotifyWindowObject, End))
                    DoMethod(G->App, OM_ADDMEMBER, G->UpdateNotifyWinObject);
                  else
                    break;
                }

                // if we still have an update component structure
                // waiting to be submitted to our window we do it right
                // away.
                if(comp != NULL)
                {
                  DoMethod(G->UpdateNotifyWinObject, MUIM_UpdateNotifyWindow_AddComponent, comp);
                  comp = NULL;
                }

                // make sure that we know that we have updates available also
                // later on.
                updatesAvailable = TRUE;

                // create a new UpdateComponent structure which we
                // are going to fill step by step
                if(!(comp = calloc(sizeof(struct UpdateComponent), 1)))
                {
                  updatesAvailable = FALSE;
                  break;
                }
              }
              else if(strcmp(buf, "[EOT]") == 0)
              {
                if(comp != NULL)
                {
                  DoMethod(G->UpdateNotifyWinObject, MUIM_UpdateNotifyWindow_AddComponent, comp);
                  comp = NULL;

                  break;
                }
              }
              else if(comp != NULL)
              {
                if(strncmp(buf, "COMPONENT: ", 11) == 0)
                  strlcpy(comp->name, buf+11, sizeof(comp->name));
                else if(strncmp(buf, "RECENT: ", 7) == 0)
                  strlcpy(comp->recent, buf+7, sizeof(comp->recent));
                else if(strncmp(buf, "INSTALLED: ", 11) == 0)
                  strlcpy(comp->installed, buf+11, sizeof(comp->installed));
                else if(strncmp(buf, "URL: ", 5) == 0)
                  strlcpy(comp->url, buf+5, sizeof(comp->url));
                else if(strncmp(buf, "CHANGES:", 8) == 0)
                {
                  // we put the changelog text into a temporary file
                  if((comp->changeLogFile = OpenTempFile("w")))
                  {
                    FILE *out = comp->changeLogFile->FP;

                    while(GetLine(tf->FP, buf, SIZE_LINE))
                    {
                      if(strcmp(buf, "[EOC]") == 0 ||
                         strcmp(buf, "[EOT]") == 0)
                      {
                        // break out
                        break;
                      }
                      else
                        fprintf(out, "%s\n", buf);
                    }

                    fclose(out);
                    comp->changeLogFile->FP = NULL;
                  }
                }
              }
            }

            // make sure we submitted all update components to our
            // notify window.
            if(comp != NULL)
            {
              DoMethod(G->UpdateNotifyWinObject, MUIM_UpdateNotifyWindow_AddComponent, comp);
              comp = NULL;
            }

            // make sure we show the update notify window.
            #if 1 // disabled until 100% finished!!
            if(updatesAvailable)
            {
              set(G->UpdateNotifyWinObject, MUIA_Window_Open, TRUE);
              C->LastUpdateStatus = UST_UPDATESUCCESS;
            }
            else
            #endif
              C->LastUpdateStatus = UST_NOUPDATE; // we didn't find any new updates.

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
  }

  // as the last operation we get the current time as the
  // last checked time for the update check and save our
  // configuration back to disk.
  GetSysTime(&C->LastUpdateCheck);

  // we save the configuration file which we currently
  // have in memory and copy the changed elements to our
  // temporar (CE) structure as well which is quite helpfull
  // in case a config window is open.
  CO_SaveConfig(C, G->CO_PrefsFile);
  if(CE)
  {
    memcpy(&CE->LastUpdateCheck, &C->LastUpdateCheck, sizeof(struct TimeVal));
    CE->LastUpdateStatus = C->LastUpdateStatus;
  }

  // make sure we reinit the updatecheck but without
  // doing any time compare
  InitUpdateCheck(FALSE);

  RETURN(result);
  return result;
}

///
