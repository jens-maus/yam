/***************************************************************************

 YAM - Yet Another Mailer
 Copyright (C) 1995-2000 Marcel Beck
 Copyright (C) 2000-2013 YAM Open Source Team

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

#include <stdlib.h>
#include <strings.h>

#include <clib/alib_protos.h>
#include <workbench/startup.h>
#include <workbench/workbench.h>
#include <proto/dos.h>
#include <proto/exec.h>
#include <proto/intuition.h>
#include <proto/wb.h>
#if defined(__amigaos4__)
#include <proto/application.h>
#endif

#include "extrasrc.h"

#include "YAM.h"
#include "YAM_config.h"
#include "YAM_configFile.h"

#include "mui/ClassesExtra.h"
#include "mui/WriteWindow.h"

#include "AppIcon.h"
#include "DockyIcon.h"
#include "FolderList.h"

#include "Debug.h"

/// InitAppIcon
// initalize all AppIcon related stuff
BOOL InitAppIcon(void)
{
  BOOL success = FALSE;

  ENTER();

  G->currentAppIcon = II_MAX;
  if((G->AppPort = AllocSysObjectTags(ASOT_PORT, TAG_DONE)) != NULL)
    success = TRUE;

  RETURN(success);
  return success;
}

///
/// FreeAppIcon
// deinitalize all AppIcon related stuff
void FreeAppIcon(void)
{
  ENTER();

  D(DBF_STARTUP, "freeing AppIcon...");
  if(G->AppIcon != NULL)
    RemoveAppIcon(G->AppIcon);

  D(DBF_STARTUP, "freeing AppPort...");
  if(G->AppPort != NULL)
  {
    FreeSysObject(ASOT_PORT, G->AppPort);
  }

  LEAVE();
}

///
/// UpdateAppIcon
//  Calculates AppIconStatistic and update the AppIcon
void UpdateAppIcon(void)
{
  int activeConnections;
  static char apptit[SIZE_DEFAULT/2];
  int mode;
  int new_msg = 0;
  int unr_msg = 0;
  int tot_msg = 0;
  int snt_msg = 0;
  int del_msg = 0;

  ENTER();

  // if the user wants to show an AppIcon on the workbench,
  // we go and calculate the mail stats for all folders out there.
  LockFolderListShared(G->folders);

  if(IsFolderListEmpty(G->folders) == FALSE)
  {
    struct FolderNode *fnode;

    ForEachFolderNode(G->folders, fnode)
    {
      struct Folder *fo = fnode->folder;

      if(fo->Stats == TRUE)
      {
        new_msg += fo->New;
        unr_msg += fo->Unread;
        tot_msg += fo->Total;
        snt_msg += fo->Sent;
        del_msg += fo->Deleted;
      }
    }
  }

  UnlockFolderList(G->folders);

  // clear AppIcon Label first before we create it new
  apptit[0] = '\0';

  if(C->WBAppIcon == TRUE)
  {
    char *src;

    // Lets create the label of the AppIcon now
    for(src = C->AppIconText; *src; src++)
    {
      char dst[10];

      if(*src == '%')
      {
        src++;

        switch (*src)
        {
          case '%': strlcpy(dst, "%", sizeof(dst));            break;
          case 'n': snprintf(dst, sizeof(dst), "%d", new_msg); break;
          case 'u': snprintf(dst, sizeof(dst), "%d", unr_msg); break;
          case 't': snprintf(dst, sizeof(dst), "%d", tot_msg); break;
          case 's': snprintf(dst, sizeof(dst), "%d", snt_msg); break;
          case 'd': snprintf(dst, sizeof(dst), "%d", del_msg); break;
        }
      }
      else
        snprintf(dst, sizeof(dst), "%c", *src);

      strlcat(apptit, dst, sizeof(apptit));
    }
  }

  ObtainSemaphoreShared(G->connectionSemaphore);
  activeConnections = G->activeConnections;
  ReleaseSemaphore(G->connectionSemaphore);

  // we set the mode accordingly to the status of the folder (new/check/old)
  if(activeConnections > 0)
    mode = II_CHECK;
  else if(tot_msg == 0)
    mode = II_EMPTY;
  else if(unr_msg == 0)
    mode = II_OLD;
  else
    mode = II_NEW;


  // We first have to remove the appicon before we can change it
  if(G->AppIcon != NULL)
  {
    RemoveAppIcon(G->AppIcon);
    G->AppIcon = NULL;
  }

  // Now we create the new AppIcon and display it
  if(G->theme.icons[mode] != NULL)
  {
    struct DiskObject *dobj = G->theme.icons[mode];

    // NOTE:
    // 1.) Using the VARARGS version is better for GCC/68k and it doesn't
    //     hurt other compilers
    // 2.) Using "zero" as lock parameter avoids a header compatibility
    //     issue (old: "struct FileLock *"; new: "BPTR")
    if(C->WBAppIcon == TRUE)
    {
      // set the icon position
      dobj->do_CurrentX = C->IconPositionX < 0 ? (LONG)NO_ICON_POSITION : C->IconPositionX;
      dobj->do_CurrentY = C->IconPositionY < 0 ? (LONG)NO_ICON_POSITION : C->IconPositionY;

      // add the AppIcon accordingly. Here we use v44+ tags, however older
      // workbench versions should perfectly ignore them.
      G->AppIcon = AddAppIcon(0, 0, apptit, G->AppPort, 0, dobj,
        WBAPPICONA_SupportsOpen,       TRUE,
        WBAPPICONA_SupportsSnapshot,   TRUE,
        WBAPPICONA_SupportsUnSnapshot, TRUE,
        WBAPPICONA_SupportsEmptyTrash, TRUE,
        WBAPPICONA_PropagatePosition,  TRUE,
        TAG_DONE);
      SHOWVALUE(DBF_GUI, G->AppIcon);
    }

    // remember this icon pointer for later use
    G->currentAppIcon = mode;

    UpdateDockyIcon();
  }

  LEAVE();
}

///
/// SnapshotAppIcon
// remember the current position of the AppIcon
static void SnapshotAppIcon(void)
{
  struct DiskObject *dobj;

  ENTER();

  if(G->currentAppIcon != II_MAX && (dobj = G->theme.icons[G->currentAppIcon]) != NULL)
  {
    // remember the position.
    C->IconPositionX = dobj->do_CurrentX;
    C->IconPositionY = dobj->do_CurrentY;

    // we also save the configuration here, even if that
    // will trigger that other configurations will
    // be saved as well. However, such a snapshot action
    // is done very rarely and the user would definitly
    // expect that the position will be saved immediately.
    CO_SaveConfig(C, G->CO_PrefsFile);
  }

  LEAVE();
}

///
/// UnsnapshotAppIcon
// UnsnapshotAppIcon
// set the AppIcon's position to no specific position
static void UnsnapshotAppIcon(void)
{
  ENTER();

  // for unsnapshotting the icon position we negate the
  // IconPosition values. So negative values mean they
  // are disabled.
  C->IconPositionX = -abs(C->IconPositionX);
  C->IconPositionY = -abs(C->IconPositionY);

  // we also save the configuration here, even if that
  // will trigger that other configurations will
  // be saved as well. However, such a snapshot action
  // is done very rarely and the user would definitly
  // expect that the position will be saved immediately.
  CO_SaveConfig(C, G->CO_PrefsFile);

  // refresh the AppIcon
  UpdateAppIcon();

  LEAVE();
}

///
/// HandleAppIcon
// handle all messages from the AppIcon
void HandleAppIcon(void)
{
  struct AppMessage *apmsg;

  ENTER();

  while((apmsg = (struct AppMessage *)GetMsg(G->AppPort)) != NULL)
  {
    if(apmsg->am_Type == AMTYPE_APPICON)
    {
      ULONG action = AMCLASSICON_Open;

      // now we catch the am_Class member of the APPICON message
      // which will be set by workbench.library v44+. However,
      // older workbench versions doesn't seem to have the Class
      // member and may have it uninitialized, therefore we
      // check here for the v44+ workbench
      if(WorkbenchBase != NULL && LIB_VERSION_IS_AT_LEAST(WorkbenchBase, 44, 0) == TRUE)
        action = apmsg->am_Class;

      // check the action
      switch(action)
      {
        // user has pressed "Open" or double-clicked on the
        // AppIcon, so we popup YAM and eventually load the
        // drag&dropped file into a new write window.
        case AMCLASSICON_Open:
        {
          // bring all windows of YAM to front.
          PopUp();

          // check if something was dropped onto the AppIcon
          if(apmsg->am_NumArgs != 0)
          {
            // open a new write window
            struct WriteMailData *wmData;

            if((wmData = NewWriteMailWindow(NULL, 0)) != NULL)
            {
              int arg;

              // lets walk through all arguments in the appMessage
              for(arg = 0; arg < apmsg->am_NumArgs; arg++)
              {
                char buf[SIZE_PATHFILE];
                struct WBArg *ap = &apmsg->am_ArgList[arg];

                NameFromLock(ap->wa_Lock, buf, sizeof(buf));
                AddPart(buf, (char *)ap->wa_Name, sizeof(buf));

                // call WR_App to let it put in the text of the file
                // to the write window
                DoMethod(wmData->window, MUIM_WriteWindow_DroppedFile, buf);
              }
            }
          }
        }
        break;

        // user has pressed "Snapshot" on the AppIcon
        case AMCLASSICON_Snapshot:
        {
          SnapshotAppIcon();
        }
        break;

        // user has pressed "UnSnapshot" on the AppIcon
        case AMCLASSICON_UnSnapshot:
        {
          UnsnapshotAppIcon();
        }
        break;

        // user has pressed "Empty Trash" on the AppIcon,
        // so we go and empty the trash folder accordingly.
        case AMCLASSICON_EmptyTrash:
        {
          // empty the "deleted" folder
          DoMethod(G->App, MUIM_CallHook, &MA_DeleteDeletedHook, FALSE);
        }
        break;
      }
    }

    ReplyMsg(&apmsg->am_Message);
  }
}

///
