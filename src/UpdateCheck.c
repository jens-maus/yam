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

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>

#include <libraries/locale.h>
#include <mui/BetterString_mcc.h>
#include <mui/NFloattext_mcc.h>
#include <mui/NListtree_mcc.h>
#include <mui/NBalance_mcc.h>
#include <mui/TextEditor_mcc.h>
#include <mui/TheBar_mcc.h>

#include <proto/exec.h>
#include <proto/muimaster.h>
#include <proto/timer.h>

#if !defined(__amigaos4__)
#include <clib/alib_protos.h> // DoMethod
#endif

#include "extrasrc.h"

#include "YAM.h"
#include "YAM_config.h"
#include "YAM_configFile.h"
#include "YAM_error.h"
#include "YAM_global.h"
#include "YAM_utilities.h"

#include "mui/Classes.h"
#include "mime/rfc1738.h"
#include "tcp/http.h"

#include "Locale.h"
#include "MUIObjects.h"
#include "Requesters.h"
#include "UpdateCheck.h"

#include "tcp/Connection.h"

#include "Debug.h"

/*** Required library bases ***/
extern struct Library *AmiSSLMasterBase;
extern struct Library *CodesetsBase;
extern struct Library *XpkBase;

/*** Static variables/functions ***/
static struct UpdateState LastUpdateState;

/*** Update-Check mechanisms ***/
/// InitUpdateCheck
// initializes all update-check relevant stuff (during startup) so that
// our autocheck is running properly.
void InitUpdateCheck(const BOOL initial)
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
    nextCheck.Seconds       = LastUpdateState.LastUpdateCheck.Seconds + C->UpdateInterval;
    nextCheck.Microseconds  = LastUpdateState.LastUpdateCheck.Microseconds;

    // get the current time
    GetSysTime(TIMEVAL(&now));

    // compare it against the last checked time we have
    // in our config and if greater than we go and do an immediate update
    // check.
    if(initial == TRUE && CmpTime(TIMEVAL(&now), TIMEVAL(&nextCheck)) <= 0)
    {
      D(DBF_UPDATE, "update-check is due to be processed NOW.");

      // instead of calling CheckForUpdates() directly, we issue
      // a timer to timeout in 1 milliseconds
      RestartTimer(TIMER_UPDATECHECK, 0, 1);
    }
    else
    {
      // we now (re)issue the next update check with the same update
      // interval as our previous one.
      D(DBF_UPDATE, "update-check is due to be processed in %ld seconds.", nextCheck.Seconds - now.Seconds);
      RestartTimer(TIMER_UPDATECHECK, nextCheck.Seconds - now.Seconds, 0);
    }
  }
  else
  {
    // make sure the updatecheck timer is not running anymore
    StopTimer(TIMER_UPDATECHECK);
  }

  LEAVE();
}
///
/// CheckForUpdates
// contacts the 'update.yam.ch' HTTP server and asks for
// specific updates.
BOOL CheckForUpdates(const BOOL quiet)
{
  BOOL result = FALSE;
  struct Connection *conn;

  ENTER();

  // flag the last update to be failed per default first
  LastUpdateState.LastUpdateStatus = UST_NOQUERY;

  // first we check if we can start a connection or if the
  // tcp/ip stuff is busy right now so that we do not interrupt something
  if((conn = CreateConnection()) != NULL)
  {
    // disable the transfer buttons in the toolbar
    MA_ChangeTransfer(FALSE);

    // pause the mail check timer so that we are not
    // interfering with an automatic mail check action
    PauseTimer(TIMER_CHECKMAIL);

    // now we open a new TCP/IP connection socket
    if(ConnectionIsOnline(conn) == TRUE)
    {
      struct TempFile *tf;

      if((tf = OpenTempFile(NULL)) != NULL)
      {
        char *request;

        BusyText(tr(MSG_BusyGettingVerInfo), "");

        // now we prepare our request string which we send to our update server
        // and will inform it about our configuration/YAM version and so on.
        // use a max. request buffer of 1K.
        #define REQUEST_SIZE 1024
        if((request = malloc(REQUEST_SIZE)) != NULL) // don't use stack for the request
        {
          char buf[SIZE_LINE];
          Object *mccObj;
          struct Library *base;
          unsigned short cnt = 0;

          // encode the yam version
          if(urlencode(buf, yamversion, sizeof(buf)) > 0)
            snprintf(request, REQUEST_SIZE, "?ver=%s", buf);

          // encode the yam buildid if present
          if(urlencode(buf, yambuildid, sizeof(buf)) > 0)
            snprintf(request, REQUEST_SIZE, "%s&buildid=%s", request, buf);

          // encode the yam builddate if present
          if(urlencode(buf, yamversiondate, sizeof(buf)) > 0)
            snprintf(request, REQUEST_SIZE, "%s&builddate=%s", request, buf);

          // encode the language in which YAM is running
          if(G->Catalog != NULL && urlencode(buf, G->Catalog->cat_Language, sizeof(buf)) > 0)
            snprintf(request, REQUEST_SIZE, "%s&lang=%s%%20%d%%2E%d", request, buf, G->Catalog->cat_Version,
                                                                                    G->Catalog->cat_Revision);

          // Now we add some third party components
          // information for our update server as well.

          // encode the exec version
          snprintf(request, REQUEST_SIZE, "%s&exec=%d%%2E%d", request, ((struct Library *)SysBase)->lib_Version,
                                                                       ((struct Library *)SysBase)->lib_Revision);

          // add codesets.library information
          snprintf(request, REQUEST_SIZE, "%s&lib%d=codesets-%d%%2E%d", request, cnt++, CodesetsBase->lib_Version,
                                                                                        CodesetsBase->lib_Revision);

          // add AmiSSL library information
          if(AmiSSLMasterBase != NULL)
            snprintf(request, REQUEST_SIZE, "%s&lib%d=amissl-%d%%2E%d", request, cnt++, AmiSSLMasterBase->lib_Version,
                                                                                        AmiSSLMasterBase->lib_Revision);

          // add XPK library information
          if(XpkBase != NULL)
            snprintf(request, REQUEST_SIZE, "%s&lib%d=xpk-%d%%2E%d", request, cnt++, XpkBase->lib_Version,
                                                                                     XpkBase->lib_Revision);

          // add openurl.library information
          if((base = OpenLibrary("openurl.library", 0)) != NULL)
          {
            snprintf(request, REQUEST_SIZE, "%s&lib%d=openurl-%d%%2E%d", request, cnt++, base->lib_Version,
                                                                                         base->lib_Revision);
            CloseLibrary(base);
          }

          // encode the MUI version
          cnt = 0;
          snprintf(request, REQUEST_SIZE, "%s&mui=%d%%2E%d", request, MUIMasterBase->lib_Version,
                                                                      MUIMasterBase->lib_Revision);

          // add TheBar.mcc version information
          if((mccObj = MUI_NewObject(MUIC_TheBar, TAG_DONE)) != NULL)
          {
            snprintf(request, REQUEST_SIZE, "%s&mcc%d=thebar-%ld%%2E%ld", request, cnt++, xget(mccObj, MUIA_Version),
                                                                                          xget(mccObj, MUIA_Revision));
            MUI_DisposeObject(mccObj);
          }

          // add TextEditor.mcc version information
          if((mccObj = MUI_NewObject(MUIC_TextEditor, TAG_DONE)) != NULL)
          {
            snprintf(request, REQUEST_SIZE, "%s&mcc%d=texteditor-%ld%%2E%ld", request, cnt++, xget(mccObj, MUIA_Version),
                                                                                              xget(mccObj, MUIA_Revision));
            MUI_DisposeObject(mccObj);
          }

          // add BetterString.mcc version information
          if((mccObj = MUI_NewObject(MUIC_BetterString, TAG_DONE)) != NULL)
          {
            snprintf(request, REQUEST_SIZE, "%s&mcc%d=betterstring-%ld%%2E%ld", request, cnt++, xget(mccObj, MUIA_Version),
                                                                                                xget(mccObj, MUIA_Revision));
            MUI_DisposeObject(mccObj);
          }

          // add NList.mcc version information
          if((mccObj = MUI_NewObject(MUIC_NList, TAG_DONE)) != NULL)
          {
            snprintf(request, REQUEST_SIZE, "%s&mcc%d=nlist-%ld%%2E%ld", request, cnt++, xget(mccObj, MUIA_Version),
                                                                                         xget(mccObj, MUIA_Revision));
            MUI_DisposeObject(mccObj);
          }

          // add NListview.mcc version information
          if((mccObj = MUI_NewObject(MUIC_NListview, TAG_DONE)) != NULL)
          {
            snprintf(request, REQUEST_SIZE, "%s&mcc%d=nlistview-%ld%%2E%ld", request, cnt++, xget(mccObj, MUIA_Version),
                                                                                             xget(mccObj, MUIA_Revision));
            MUI_DisposeObject(mccObj);
          }

          // add NFloattext.mcc version information
          if((mccObj = MUI_NewObject(MUIC_NFloattext, TAG_DONE)) != NULL)
          {
            snprintf(request, REQUEST_SIZE, "%s&mcc%d=nfloattext-%ld%%2E%ld", request, cnt++, xget(mccObj, MUIA_Version),
                                                                                              xget(mccObj, MUIA_Revision));
            MUI_DisposeObject(mccObj);
          }

          // add NListtree.mcc version information
          if((mccObj = MUI_NewObject(MUIC_NListtree, TAG_DONE)) != NULL)
          {
            snprintf(request, REQUEST_SIZE, "%s&mcc%d=nlisttree-%ld%%2E%ld", request, cnt++, xget(mccObj, MUIA_Version),
                                                                                             xget(mccObj, MUIA_Revision));
            MUI_DisposeObject(mccObj);
          }

          // add NBalance.mcc version information
          if((mccObj = MUI_NewObject(MUIC_NBalance, TAG_DONE)) != NULL)
          {
            snprintf(request, REQUEST_SIZE, "%s&mcc%d=nbalance-%ld%%2E%ld", request, cnt++, xget(mccObj, MUIA_Version),
                                                                                            xget(mccObj, MUIA_Revision));
            MUI_DisposeObject(mccObj);
          }

          D(DBF_UPDATE, "send update request: '%s' (%ld)", request, strlen(request));

          // now we send a specific request via TR_DownloadURL() to
          // our update server
          if(TR_DownloadURL(conn, C->UpdateServer, request, tf->Filename) == TRUE)
          {
            // now we parse the result.
            if((tf->FP = fopen(tf->Filename, "r")) != NULL)
            {
              BOOL validUpdateCheck = FALSE;
              BOOL updatesAvailable = FALSE;
              struct UpdateComponent *comp = NULL;
              char *buffer = NULL;
              size_t size = 0;

              setvbuf(tf->FP, NULL, _IOFBF, SIZE_FILEBUF);

              // make sure we clear an eventually existing update window
              if(G->UpdateNotifyWinObject != NULL)
                DoMethod(G->UpdateNotifyWinObject, MUIM_UpdateNotifyWindow_Clear);

              while(GetLine(&buffer, &size, tf->FP) >= 0)
              {
                // make sure we trim the line by stripping leading
                // and trailing spaces.
                char *p = Trim(buffer);

                D(DBF_UPDATE, "'%s'", p);

                if(stricmp(p, "<updatecheck>") == 0)
                {
                  validUpdateCheck = TRUE;
                }
                else if(stricmp(p, "</updatecheck>") == 0)
                {
                  // break out as the update check signals to exit
                  break;
                }
                else if(validUpdateCheck == FALSE)
                {
                  // we skip all lines until we found the <updatecheck> tag
                  continue;
                }
                else if(stricmp(p, "<component>") == 0)
                {
                  // if we find an '<component>' tag, we can go and create an
                  // update notify window in advance and fill it according to the
                  // followed information

                  // make sure that we have created the update notification
                  // window in advance.
                  if(G->UpdateNotifyWinObject == NULL)
                  {
                    if((G->UpdateNotifyWinObject = UpdateNotifyWindowObject, End) != NULL)
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
                  if((comp = calloc(sizeof(*comp), 1)) == NULL)
                  {
                    updatesAvailable = FALSE;
                    break;
                  }
                }
                else if(stricmp(p, "</component>") == 0)
                {
                  if(comp != NULL)
                  {
                    DoMethod(G->UpdateNotifyWinObject, MUIM_UpdateNotifyWindow_AddComponent, comp);
                    comp = NULL;
                  }
                }
                else if(comp != NULL)
                {
                  if(strnicmp(p, "NAME: ", 6) == 0)
                    strlcpy(comp->name, p+6, sizeof(comp->name));
                  else if(strnicmp(p, "RECENT: ", 7) == 0)
                    strlcpy(comp->recent, p+7, sizeof(comp->recent));
                  else if(strnicmp(p, "INSTALLED: ", 11) == 0)
                    strlcpy(comp->installed, p+11, sizeof(comp->installed));
                  else if(strnicmp(p, "URL: ", 5) == 0)
                    strlcpy(comp->url, p+5, sizeof(comp->url));
                  else if(stricmp(p, "<changelog>") == 0)
                  {
                    // we put the changelog text into a temporary file
                    if((comp->changeLogFile = OpenTempFile("w")) != NULL)
                    {
                      FILE *out = comp->changeLogFile->FP;

                      while(GetLine(&buffer, &size, tf->FP) >= 0L)
                      {
                        D(DBF_UPDATE, "%s", buffer);

                        if(stricmp(buffer, "</changelog>") == 0)
                        {
                          // break out
                          break;
                        }
                        else
                          fprintf(out, "%s\n", buffer);
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
              if(updatesAvailable == TRUE)
              {
                set(G->UpdateNotifyWinObject, MUIA_Window_Open, TRUE);
                LastUpdateState.LastUpdateStatus = UST_UPDATESUCCESS;
              }
              else
              {
                // we didn't find any new updates
                LastUpdateState.LastUpdateStatus = UST_NOUPDATE;

                // show a requester if the check was triggered by the user
                if(quiet == FALSE)
                  MUI_Request(G->App, NULL, 0, tr(MSG_UPD_NO_UPDATES_FOUND_TITLE), tr(MSG_Okay), tr(MSG_UPD_NO_UPDATES_FOUND), (C->UpdateInterval > 0) ? tr(MSG_UPD_NO_UPDATES_FOUND_HINT_AUTOCHECK) : tr(MSG_UPD_NO_UPDATES_FOUND_HINT_NOAUTOCHECK));
              }

              // the updatecheck was successfull
              result = TRUE;

              fclose(tf->FP);
              tf->FP = NULL;

              free(buffer);
            }
            else
              ER_NewError(tr(MSG_ER_CantOpenTempfile), tf->Filename);
          }

          free(request);
        }

        BusyEnd();

        CloseTempFile(tf);
      }
    }
    else
      ER_NewError(tr(MSG_ER_OPENTCPIP));

    // enable the transfer buttons in the toolbar again
    MA_ChangeTransfer(TRUE);

    // resume the mail check timer
    ResumeTimer(TIMER_CHECKMAIL);

    DeleteConnection(conn);
  }

  // as the last operation we get the current time as the
  // last checked time for the update check and save our
  // configuration back to disk.
  GetSysTime(TIMEVAL(&LastUpdateState.LastUpdateCheck));

  // now save the update state
  SaveUpdateState();
  if(CE != NULL)
  {
    // in case the updatecheck resulted in no further update or a successful update check
    // we have to check if we have to update the config page of an eventually opened YAM
    // configuration window.
    if((LastUpdateState.LastUpdateStatus == UST_NOUPDATE || LastUpdateState.LastUpdateStatus == UST_UPDATESUCCESS) &&
       G->CO != NULL && G->CO->VisiblePage == cp_Update)
    {
      CO_SetConfig();
    }
  }

  // make sure we reinit the updatecheck but without
  // doing any time compare
  InitUpdateCheck(FALSE);

  RETURN(result);
  return result;
}
///
/// LoadUpdateState
// Load update state file from disk
void LoadUpdateState(void)
{
  FILE *fh;

  ENTER();

  // we start with "no update yet" ...
  LastUpdateState.LastUpdateStatus = UST_NOQUERY;

  // ... and a zero time which will result in a possible immediate update check
  memset(&LastUpdateState.LastUpdateCheck, 0, sizeof(LastUpdateState.LastUpdateCheck));

  // the YAM executable is the same for all users, hence we need no per user state file
  if((fh = fopen("PROGDIR:.updatestate", "r")) != NULL)
  {
    char *buf = NULL;
    size_t size = 0;

    setvbuf(fh, NULL, _IOFBF, SIZE_FILEBUF);

    if(GetLine(&buf, &size, fh) >= 3)
    {
      if(strnicmp(buf, "YUP", 3) == 0)
      {
        int version;

        // we derive the version here although it is not yet needed
        version = buf[3] - '0';

        // read in all the lines
        while(GetLine(&buf, &size, fh) >= 0)
        {
          char *p;
          char *value;

          if((value = strchr(buf, '=')) != NULL)
          {
            const char *value2 = "";

            for(value2 = (++value) + 1; isspace(*value); value++);
          }

          for(p = buf; *p != '\0' && *p != '=' && !isspace(*p); p++);
          *p = '\0';

          if(*buf != '\0' && value != NULL)
          {
            if(version >= 1)
            {
              if(stricmp(buf, "LastUpdateCheck") == 0)
                String2TimeVal(&LastUpdateState.LastUpdateCheck, value, DSS_USDATETIME, TZC_NONE);
              else if(stricmp(buf, "LastUpdateStatus") == 0)
                LastUpdateState.LastUpdateStatus = atoi(value);
              else
                W(DBF_UPDATE, "unknown update option: '%s' = '%s'", buf, value);
            }
          }
        }
      }
    }

    fclose(fh);

    free(buf);
  }

  LEAVE();
}
///
/// SaveUpdateState
// Save update state file to disk
void SaveUpdateState(void)
{
  FILE *fh;

  ENTER();

  // the YAM executable is the same for all users, hence we need no per user state file
  if((fh = fopen("PROGDIR:.updatestate", "w")) != NULL)
  {
    char buf[SIZE_LARGE];

    setvbuf(fh, NULL, _IOFBF, SIZE_FILEBUF);

    fprintf(fh, "YUP1 - YAM Update state\n");
    TimeVal2String(buf, sizeof(buf), &LastUpdateState.LastUpdateCheck, DSS_USDATETIME, TZC_NONE);
    fprintf(fh, "LastUpdateCheck  = %s\n", buf);
    fprintf(fh, "LastUpdateStatus = %d\n", LastUpdateState.LastUpdateStatus);

    fclose(fh);
  }

  LEAVE();
}
///
/// GetLastUpdateState
// Get a copy of the last update state
void GetLastUpdateState(struct UpdateState *state)
{
  ENTER();

  if(state != NULL)
  {
    // copy the last state
    memcpy(state, &LastUpdateState, sizeof(*state));
  }

  LEAVE();
}
///
/// SetDefaultUpdateState
// Set a default update state
void SetDefaultUpdateState(void)
{
  ENTER();

  // flag the last update to be failed per default
  LastUpdateState.LastUpdateStatus = UST_NOQUERY;

  LEAVE();
}
///

