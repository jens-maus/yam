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

#include <proto/dos.h>
#include <proto/exec.h>
#include <proto/utility.h>
#if defined(__amigaos4__)
#include <proto/application.h>
#endif

#include "YAM.h"
#include "YAM_config.h"

#include "mui/ClassesExtra.h"
#include "mui/WriteWindow.h"

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
  // reset the docky icon id to some sensible default
  // upon restart this makes sure that the docky icon is set to the correct state
  G->LastIconID = ii_Max;

  if(ApplicationBase != NULL)
  {
    struct ApplicationIconInfo aii;
    char filename[SIZE_PATHFILE];
    struct TagItem registerTags[] =
    {
      { REGAPP_FileName,          (uint32)filename },
      { REGAPP_UniqueApplication, TRUE },
      { REGAPP_URLIdentifier,     (uint32)"yam.ch" },
      { REGAPP_AppIconInfo,       (uint32)&aii },
      { REGAPP_Hidden,            xget(G->App, MUIA_Application_Iconified) },
      { REGAPP_Description,       (uint32)tr(MSG_APP_DESCRIPTION) },
      { TAG_DONE,                 0 }
    };
    struct TagItem portTags[] =
    {
      { APPATTR_Port, (uint32)&G->AppLibPort },
      { TAG_DONE, 0 }
    };

    // combine the full application path
    strlcpy(filename, G->ProgDir, sizeof(filename));
    AddPart(filename, G->ProgName, sizeof(filename));

    if(C->DockyIcon == FALSE)
    {
      D(DBF_STARTUP, "registering without Docky icon");
      aii.iconType = APPICONT_None;
    }
    else if(G->HideIcon == NULL)
    {
      D(DBF_STARTUP, "registering with program Docky icon");
      aii.iconType = APPICONT_ProgramIcon;
    }
    else if(G->currentAppIcon != ii_Max && G->theme.icons[G->currentAppIcon] != NULL)
    {
      D(DBF_STARTUP, "registering with custom Docky icon %ld %08lx", G->currentAppIcon, G->theme.icons[G->currentAppIcon]);
      aii.iconType = APPICONT_CustomIcon;
      aii.info.customIcon = G->theme.icons[G->currentAppIcon];
      G->LastIconID = G->currentAppIcon;
    }
    else
    {
      D(DBF_STARTUP, "registering with custom Docky icon %08lx", G->HideIcon);
      aii.iconType = APPICONT_CustomIcon;
      aii.info.customIcon = G->HideIcon;
    }

    // register YAM to application.library
    if(IApplication->Data.Version >= 2)
    {
      // the V2 interface uses regular tags
      // check whether we compile with the old V1 tag definitions, while already having
      // a new V2 interface. If yes, then we must adapt the tag values by simply
      // adding TAG_USER to the old definition.
      if(REGAPP_UniqueApplication < TAG_USER)
      {
        registerTags[0].ti_Tag += TAG_USER;
        registerTags[1].ti_Tag += TAG_USER;
        registerTags[2].ti_Tag += TAG_USER;
        registerTags[3].ti_Tag += TAG_USER;
        registerTags[4].ti_Tag += TAG_USER;
        registerTags[5].ti_Tag += TAG_USER;

        portTags[0].ti_Tag += TAG_USER;
      }
    }
    else
    {
      // we have the old V1 interface
      // the V1 interface uses non-standard tags which collide with standard
      // tags like TAG_IGNORE and TAG_MORE, thus we have to take additional
      // care here.
      // check whether we compile with the new V2 tag definitions, but still got
      // an old V1 interface. If yes, then we must adapt the tag values by simply
      // substracting TAG_USER from the new definition.
      if(REGAPP_UniqueApplication >= TAG_USER)
      {
        registerTags[0].ti_Tag -= TAG_USER;
        registerTags[1].ti_Tag -= TAG_USER;
        registerTags[2].ti_Tag -= TAG_USER;
        registerTags[3].ti_Tag -= TAG_USER;
        registerTags[4].ti_Tag -= TAG_USER;
        registerTags[5].ti_Tag -= TAG_USER;

        portTags[0].ti_Tag -= TAG_USER;
      }

      // application.lib V52.1 crashes if it sees REGAPP_Description and V53.2
      // misinterprets german umlauts, hence we require at least V53.3 for the
      // description string.
      if(LIB_VERSION_IS_AT_LEAST(ApplicationBase, 53, 3) == FALSE)
        registerTags[5].ti_Tag = TAG_IGNORE;
    }

    // the dirty work is done, let's register us
    if((G->applicationID = RegisterApplicationA("YAM", registerTags)) != 0)
    {
      GetApplicationAttrsA(G->applicationID, portTags);
      if(G->AppLibPort == NULL)
        E(DBF_STARTUP, "error on trying to retrieve application libraries MsgPort for YAM.");
    }

    D(DBF_STARTUP, "registered YAM to application.library with appID: %ld", G->applicationID);
  }
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
    G->AppLibPort = NULL;
    G->LastIconID = ii_Max;
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
  if(G->applicationID > 0 && G->LastIconID != G->currentAppIcon)
  {
    struct ApplicationIconInfo aii;
    struct TagItem iconTags[] =
    {
      { APPATTR_IconType, (uint32)&aii },
      { TAG_DONE, 0 }
    };

    if(C->DockyIcon == FALSE)
    {
      D(DBF_STARTUP, "remove Docky icon");
      aii.iconType = APPICONT_None;
    }
    else if(G->currentAppIcon == ii_Max)
    {
      D(DBF_STARTUP, "set program Docky icon");
      aii.iconType = APPICONT_ProgramIcon;
    }
    else
    {
      D(DBF_STARTUP, "set custom Docky icon %ld %08lx", G->currentAppIcon, G->theme.icons[G->currentAppIcon]);
      aii.iconType = APPICONT_CustomIcon;
      aii.info.customIcon = G->theme.icons[G->currentAppIcon];
    }

    // adapt the tag values for the different interface versions
    if(IApplication->Data.Version >= 2)
    {
      if(APPATTR_IconType < TAG_USER)
        iconTags[0].ti_Tag += TAG_USER;
    }
    else
    {
      if(APPATTR_IconType >= TAG_USER)
        iconTags[0].ti_Tag -= TAG_USER;
    }

    if(SetApplicationAttrsA(G->applicationID, iconTags))
    {
      D(DBF_STARTUP, "Docky icon changed");
      if(C->DockyIcon == TRUE)
      {
        // remember the new docky icon state
        G->LastIconID = G->currentAppIcon;
      }
      else
      {
        G->LastIconID = ii_Max;
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

  ENTER();

  #if defined(__amigaos4__)
  D(DBF_GUI, "trying to get ApplicationMsg from AppLibPort %08lx", G->AppLibPort);
  if(G->AppLibPort != NULL)
  {
    struct ApplicationMsg *msg;

    while((msg = (struct ApplicationMsg *)GetMsg(G->AppLibPort)) != NULL)
    {
      D(DBF_GUI, "got ApplicationMsg %08lx of type %ld", msg, (msg != NULL) ? msg->type : 0);
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
  }
  #endif

  RETURN(quit);
  return quit;
}

///

