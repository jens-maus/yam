/***************************************************************************

 YAM - Yet Another Mailer
 Copyright (C) 1995-2000 by Marcel Beck <mbeck@yam.ch>
 Copyright (C) 2000-2009 by YAM Open Source Team

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

#include <proto/dos.h>
#include <proto/exec.h>
#include <proto/utility.h>
#if defined(__amigaos4__)
#include <proto/application.h>
#endif

#include "YAM.h"
#include "YAM_config.h"

#include "mui/Classes.h"

#include "DockyIcon.h"
#include "Locale.h"
#include "MUIObjects.h"

#include "Debug.h"

/// InitDockyIcon
// initalize all DockyIcon related stuff
void InitDockyIcon(void)
{
  ENTER();

  #if defined(__amigaos4__)
  G->AppLibPort = NULL;
  G->applicationID = 0;

  if(ApplicationBase != NULL)
  {
    struct ApplicationIconInfo aii;

    aii.iconType = C->DockyIcon ? APPICONT_CustomIcon : APPICONT_None;
    aii.info.customIcon = G->HideIcon;

    // register YAM to application.library
    if((G->applicationID = RegisterApplication("YAM", REGAPP_UniqueApplication, TRUE,
                                                      REGAPP_URLIdentifier,     "yam.ch",
                                                      REGAPP_AppIconInfo,       (uint32)&aii,
                                                      REGAPP_Hidden,            xget(G->App, MUIA_Application_Iconified),
                                                      REGAPP_Description,       tr(MSG_APP_DESCRIPTION),
                                                      TAG_DONE)) != 0)
    {
      GetApplicationAttrs(G->applicationID,
                          APPATTR_Port,     (uint32)&G->AppLibPort,
                          TAG_DONE);
      if(G->AppLibPort == NULL)
        E(DBF_STARTUP, "Error on trying to retrieve application libraries MsgPort for YAM.");
    }

    D(DBF_STARTUP, "Registered YAM to application.library with appID: %ld", G->applicationID);
  }

  // reset the docky icon id to some sensible default
  // upon restart this makes sure that the docky icon is set to the correct state
  G->LastIconID = -1;
  #endif

  LEAVE();
}

///
/// FreeDockyIcon
// deinitalize all DockyIcon related stuff
void FreeDockyIcon(void)
{
  ENTER();

  #if defined(__amigaos4__)
  D(DBF_STARTUP, "unregister from application.library...");
  if(G->applicationID > 0)
  {
    UnregisterApplication(G->applicationID, NULL);
    G->applicationID = 0;
  }
  #endif

  LEAVE();
}

///
/// UpdateDockyIcon
//  Calculates AppIconStatistic and update the AppIcon
void UpdateDockyIcon(void)
{
  ENTER();

  #if defined(__amigaos4__)
  // check if application.library is used and then
  // we also notify it about the AppIcon change
  if(G->applicationID > 0 && C->DockyIcon == TRUE)
  {
    if(G->LastIconID != G->currentAppIcon)
    {
      struct ApplicationIconInfo aii;

      aii.iconType = APPICONT_CustomIcon;
      aii.info.customIcon = G->theme.icons[G->currentAppIcon];

      if(SetApplicationAttrs(G->applicationID, APPATTR_IconType, (uint32)&aii, TAG_DONE))
      {
        // remember the new docky icon state
        G->LastIconID = G->currentAppIcon;
      }
    }
  }
  #endif

  LEAVE();
}

///
/// DockyIconSignal
// return the signal of the docky icon port
ULONG DockyIconSignal(void)
{
  ULONG signal = 0;

  ENTER();

  #if defined(__amigaos4__)
  if(G->AppLibPort != NULL)
    signal = (1UL << G->AppLibPort->mp_SigBit);
  #endif

  RETURN(signal);
  return signal;
}

///
/// HandleDockyIcon
// handle all messages from the docky icon
BOOL HandleDockyIcon(void)
{
  BOOL quit = FALSE;
  #if defined(__amigaos4__)
  struct ApplicationMsg *msg;
  #endif

  ENTER();

  #if defined(__amigaos4__)
  while((msg = (struct ApplicationMsg *)GetMsg(G->AppLibPort)) != NULL)
  {
    switch(msg->type)
    {
      // ask the user if he really wants to quit the application
      case APPLIBMT_Quit:
      {
        quit = !StayInProg();
      }
      break;

      // exit without bothering the user at all.
      case APPLIBMT_ForceQuit:
      {
        quit = TRUE;
      }
      break;

      // simply make sure YAM will be iconified/hidden
      case APPLIBMT_Hide:
      {
        set(G->App, MUIA_Application_Iconified, TRUE);
      }
      break;

      // simply make sure YAM will be uniconified
      case APPLIBMT_Unhide:
      {
        set(G->App, MUIA_Application_Iconified, FALSE);
      }
      break;

      // make sure the GUI of YAM is in front
      // and open with the latest document.
      case APPLIBMT_ToFront:
      {
        PopUp();
      }
      break;

      // make sure YAM is in front and open
      // the configuration window
      case APPLIBMT_OpenPrefs:
      {
        PopUp();
        CallHookPkt(&CO_OpenHook, 0, 0);
      }
      break;

      // open YAM in front of everyone and
      // import the passed document in
      // a new or existing write window.
      case APPLIBMT_OpenDoc:
      {
        struct ApplicationOpenPrintDocMsg* appmsg = (struct ApplicationOpenPrintDocMsg*)msg;
        struct WriteMailData *wmData;

        // open a new write window
        if((wmData = NewWriteMailWindow(NULL, 0)) != NULL)
        {
          PopUp();
          DoMethod(wmData->window, MUIM_WriteWindow_DroppedFile, appmsg->fileName);
        }
      }
      break;

      // make sure YAM is in front and open
      // a new write window.
      case APPLIBMT_NewBlankDoc:
      {
        PopUp();
        NewWriteMailWindow(NULL, 0);
      }
      break;

      // probably a message from the notification window to
      // let YAM show itself.
      case APPLIBMT_CustomMsg:
      {
        struct ApplicationCustomMsg* appmsg = (struct ApplicationCustomMsg*)msg;

        if(appmsg->customMsg != NULL && strcmp(appmsg->customMsg, "POPUP") == 0)
          PopUp();
      }
      break;
    }

    ReplyMsg((struct Message *)msg);
  }
  #endif

  RETURN(quit);
  return quit;
}

///

