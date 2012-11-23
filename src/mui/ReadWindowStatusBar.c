/***************************************************************************

 YAM - Yet Another Mailer
 Copyright (C) 1995-2000 Marcel Beck
 Copyright (C) 2000-2012 YAM Open Source Team

 This program is free software; you can redistribute it and/or modify
 it under the terms of the GNU General Public License as published by
 the Free Software Foundation; either version 2 of the License, or
 (at your option) any later version.

 This program is distributed in the hope that it will be useful,
 but WITHOUT ANY WARRANTY; without even the implied warranty of
 MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.   See the
 GNU General Public License for more details.

 You should have received a copy of the GNU General Public License
 along with this program; if not, write to the Free Software
 Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA

 YAM Official Support Site : http://www.yam.ch
 YAM OpenSource project    : http://sourceforge.net/projects/yamos/

 $Id$

 Superclass:  MUIC_Group
 Description: Provides some GUI elements for displaying the mail
              status in the read window

***************************************************************************/

#include "ReadWindowStatusBar_cl.h"

#include <string.h>
#include <proto/graphics.h>
#include <proto/muimaster.h>

#include "SDI_hook.h"

#include "YAM.h"
#include "YAM_mainFolder.h"

#include "Locale.h"
#include "MUIObjects.h"
#include "Themes.h"

#include "mui/ImageArea.h"
#include "mui/FolderListtree.h"

#include "Debug.h"

/* CLASSDATA
struct Data
{
  Object *statusImage[SI_MAX];
  Object *folderImage;
  Object *folderLabel;

  ULONG minHeight;
};
*/

/* INCLUDE
#include "Themes.h"
*/

/* Private Hooks */
/// LayoutHook
HOOKPROTONH(LayoutFunc, ULONG, Object *obj, struct MUI_LayoutMsg *lm)
{
  struct Data *data = (struct Data *)xget(obj, MUIA_UserData);
  ULONG result = MUILM_UNKNOWN;

  ENTER();

  switch(lm->lm_Type)
  {
    // MUI want's to know the min/max of the object so we
    // need to calculate it accordingly.
    case MUILM_MINMAX:
    {
      Object *cstate = (Object *)lm->lm_Children->mlh_Head;
      Object *child;
      LONG minHeight = data->minHeight;
      LONG minWidth = 0;

      // we iterate through all our children and see how large we are getting
      while((child = NextObject(&cstate)) != NULL)
      {
        // we know our childs and that we only carry Bodychunk objects
        minHeight = MAX(_minheight(child), minHeight);
        minWidth += _minwidth(child) + 1;
      }

      // then set our calculated values
      lm->lm_MinMax.MinWidth  = minWidth;
      lm->lm_MinMax.MinHeight = minHeight;
      lm->lm_MinMax.DefWidth  = MUI_MAXMAX;
      lm->lm_MinMax.DefHeight = minHeight;
      lm->lm_MinMax.MaxWidth  = MUI_MAXMAX;
      lm->lm_MinMax.MaxHeight = minHeight;

      result = 0;
    }
    break;

    // MUI asks' us to draw/layout the actual content of the
    // gadget, so we go and use MUI_Layout() to draw the gadget
    // components at the right position.
    case MUILM_LAYOUT:
    {
      Object *cstate;
      Object *child;
      LONG left = 0;
      LONG top = 0;
      LONG height = lm->lm_Layout.Height-1;
      LONG width = lm->lm_Layout.Width-1;

      // Layout function. Here, we have to call MUI_Layout() for each
      // our children. MUI wants us to place them in a rectangle
      // defined by (0,0,lm->lm_Layout.Width-1,lm->lm_Layout.Height-1)
      // We are free to put the children anywhere in this rectangle.

      // start with "success"
      result = TRUE;

      // layout the left-aligned folder image and label
      if(data->folderImage != NULL)
      {
        result = MUI_Layout(data->folderImage, left, top+(height-_minheight(data->folderImage))/2,
                                               _minwidth(data->folderImage), _minheight(data->folderImage), 0);
        left += _minwidth(data->folderImage);
      }

      if(result == TRUE)
      {
        result = MUI_Layout(data->folderLabel, left+1, top+(height-_minheight(data->folderLabel))/2,
                                               _minwidth(data->folderLabel), _minheight(data->folderLabel), 0);
      }

      // layout the status images which we put right-aligned
      // into our rectangle
      left = width;
      cstate = (Object *)lm->lm_Children->mlh_Head;
      while((child = NextObject(&cstate)) != NULL && result == TRUE)
      {
        // make sure we left-align the folder image
        if(child != data->folderImage && child != data->folderLabel)
        {
          left -= (_minwidth(child) + 1);
          result = MUI_Layout(child, left, top+(height-_minheight(child))/2,
                                     _minwidth(child), _minheight(child), 0);
        }
      }
    }
    break;
  }

  RETURN(result);
  return result;
}
MakeStaticHook(LayoutHook, LayoutFunc);
///

/* Private Functions */
/// RemoveAllChildren
// remove all children from an object
static void RemoveAllChildren(struct Data *data, Object *obj)
{
  struct List *childList;

  ENTER();

  // we first remove all childs from our statusGroup
  if((childList = (struct List *)xget(obj, MUIA_Group_ChildList)) != NULL)
  {
    Object *cstate = (Object *)childList->lh_Head;
    Object *child;

    while((child = NextObject(&cstate)) != NULL)
    {
      ULONG i;

      for(i=0; i < ARRAY_SIZE(data->statusImage); i++)
      {
        if(data->statusImage[i] == child)
        {
          DoMethod(obj, OM_REMMEMBER, child);
          break;
        }
      }
    }
  }

  LEAVE();
}

/* Overloaded Methods */
/// OVERLOAD(OM_NEW)
OVERLOAD(OM_NEW)
{
  Object *statusImage[SI_MAX];
  Object *folderLabel;
  ULONG i;
  ULONG minHeight = 0;

  ENTER();

  // prepare the status icons for adding it later on to our statusGroup object
  // prepare the mail status images
  statusImage[SI_ATTACH]   = MakeImageObject("status_attach",   G->theme.statusImages[SI_ATTACH]);
  statusImage[SI_CRYPT]    = MakeImageObject("status_crypt",    G->theme.statusImages[SI_CRYPT]);
  statusImage[SI_DELETE]   = MakeImageObject("status_delete",   G->theme.statusImages[SI_DELETE]);
  statusImage[SI_DOWNLOAD] = MakeImageObject("status_download", G->theme.statusImages[SI_DOWNLOAD]);
  statusImage[SI_ERROR]    = MakeImageObject("status_error",    G->theme.statusImages[SI_ERROR]);
  statusImage[SI_FORWARD]  = MakeImageObject("status_forward",  G->theme.statusImages[SI_FORWARD]);
  statusImage[SI_GROUP]    = MakeImageObject("status_group",    G->theme.statusImages[SI_GROUP]);
  statusImage[SI_HOLD]     = MakeImageObject("status_hold",     G->theme.statusImages[SI_HOLD]);
  statusImage[SI_MARK]     = MakeImageObject("status_mark",     G->theme.statusImages[SI_MARK]);
  statusImage[SI_NEW]      = MakeImageObject("status_new",      G->theme.statusImages[SI_NEW]);
  statusImage[SI_OLD]      = MakeImageObject("status_old",      G->theme.statusImages[SI_OLD]);
  statusImage[SI_REPLY]    = MakeImageObject("status_reply",    G->theme.statusImages[SI_REPLY]);
  statusImage[SI_REPORT]   = MakeImageObject("status_report",   G->theme.statusImages[SI_REPORT]);
  statusImage[SI_SENT]     = MakeImageObject("status_sent",     G->theme.statusImages[SI_SENT]);
  statusImage[SI_SIGNED]   = MakeImageObject("status_signed",   G->theme.statusImages[SI_SIGNED]);
  statusImage[SI_SPAM]     = MakeImageObject("status_spam",     G->theme.statusImages[SI_SPAM]);
  statusImage[SI_UNREAD]   = MakeImageObject("status_unread",   G->theme.statusImages[SI_UNREAD]);
  statusImage[SI_URGENT]   = MakeImageObject("status_urgent",   G->theme.statusImages[SI_URGENT]);
  statusImage[SI_WAITSEND] = MakeImageObject("status_waitsend", G->theme.statusImages[SI_WAITSEND]);
  for(i=0; i < SI_MAX; i++)
  {
    if(statusImage[i] != NULL)
    {
      ULONG rawHeight = xget(statusImage[i], MUIA_ImageArea_RawHeight);

      minHeight = MAX(rawHeight, minHeight);
    }
  }

  if((obj = DoSuperNew(cl, obj,
    MUIA_ContextMenu,       FALSE,
    MUIA_Group_Horiz,       TRUE,
    MUIA_Group_Spacing,     0,
    MUIA_Group_LayoutHook,  &LayoutHook,
    Child, folderLabel = TextObject,
      MUIA_Font,          MUIV_Font_Tiny,
      MUIA_Frame,         MUIV_Frame_None,
      MUIA_Text_PreParse, "\033l",
      MUIA_Text_Copy,     FALSE,
    End,
    TAG_MORE, inittags(msg))) != NULL)
  {
    GETDATA;

    // copy data from temp object to its real place
    memcpy(&data->statusImage[0], &statusImage[0], sizeof(statusImage));
    data->minHeight = minHeight;
    data->folderLabel = folderLabel;

    // set our instance data as MUIA_UserData as we
    // require it in our LayoutHook
    set(obj, MUIA_UserData, data);
  }

  RETURN((IPTR)obj);
  return (IPTR)obj;
}
///
/// OVERLOAD(OM_DISPOSE)
OVERLOAD(OM_DISPOSE)
{
  GETDATA;
  int i;
  IPTR result;

  ENTER();

  // clear all children of the statusGroup first
  if(DoMethod(obj, MUIM_Group_InitChange))
  {
    RemoveAllChildren(data, obj);
    DoMethod(obj, MUIM_Group_ExitChange);
  }

  // now we can free all status icons
  for(i=0; i < SI_MAX; i++)
  {
    if(data->statusImage[i] != NULL)
      MUI_DisposeObject(data->statusImage[i]);

    data->statusImage[i] = NULL;
  }

  // free the folder image object
  if(data->folderImage)
  {
    DoMethod(obj, OM_REMMEMBER, data->folderImage);
    MUI_DisposeObject(data->folderImage);
    data->folderImage = NULL;
  }

  result = DoSuperMethodA(cl, obj, msg);

  RETURN(result);
  return result;
}
///

/* Public Methods */
/// DECLARE(Update)
DECLARE(Update) // struct Mail *mail
{
  GETDATA;

  ENTER();

  // update the statusgroup by removing/adding items accordingly
  if(DoMethod(obj, MUIM_Group_InitChange))
  {
    struct Mail *mail = msg->mail;
    struct Folder *folder = mail->Folder;

    // we first remove all childs from our statusGroup
    RemoveAllChildren(data, obj);

    // now we can add the status icons depending on the set status flags of
    // the mail (sort upside-down)

    // StatusGroup 8 (Spam status)
    if(hasStatusSpam(mail) && data->statusImage[SI_SPAM] != NULL)
      DoMethod(obj, OM_ADDMEMBER, data->statusImage[SI_SPAM]);

    // StatusGroup 7 (Forwarded status)
    if(hasStatusForwarded(mail) && data->statusImage[SI_FORWARD] != NULL)
      DoMethod(obj, OM_ADDMEMBER, data->statusImage[SI_FORWARD]);

    // StatusGroup 6 (Replied status)
    if(hasStatusReplied(mail) && data->statusImage[SI_REPLY] != NULL)
      DoMethod(obj, OM_ADDMEMBER, data->statusImage[SI_REPLY]);

    // StatusGroup 5 (marked flag)
    if(hasStatusMarked(mail) && data->statusImage[SI_MARK] != NULL)
      DoMethod(obj, OM_ADDMEMBER, data->statusImage[SI_MARK]);

    // StatusGroup 4 (multipart info)
    if(isMP_MixedMail(mail) && data->statusImage[SI_ATTACH] != NULL)
      DoMethod(obj, OM_ADDMEMBER, data->statusImage[SI_ATTACH]);

    // StatusGroup 3 (report mail info)
    if(isMP_ReportMail(mail) && data->statusImage[SI_REPORT] != NULL)
      DoMethod(obj, OM_ADDMEMBER, data->statusImage[SI_REPORT]);

    // StatusGroup 2 (signed/crypted status)
    if(isMP_CryptedMail(mail) && data->statusImage[SI_CRYPT] != NULL)
      DoMethod(obj, OM_ADDMEMBER, data->statusImage[SI_CRYPT]);
    else if(isMP_SignedMail(mail) && data->statusImage[SI_SIGNED] != NULL)
      DoMethod(obj, OM_ADDMEMBER, data->statusImage[SI_SIGNED]);

    // StatusGroup 1 (importance level)
    if(getImportanceLevel(mail) == IMP_HIGH && data->statusImage[SI_URGENT] != NULL)
      DoMethod(obj, OM_ADDMEMBER, data->statusImage[SI_URGENT]);

    // StatusGroup 0 (main mail status)
    if((hasStatusError(mail) || isPartialMail(mail)) && data->statusImage[SI_ERROR] != NULL)
      DoMethod(obj, OM_ADDMEMBER, data->statusImage[SI_ERROR]);
    else if(hasStatusQueued(mail) && data->statusImage[SI_WAITSEND] != NULL)
      DoMethod(obj, OM_ADDMEMBER, data->statusImage[SI_WAITSEND]);
    else if(hasStatusSent(mail) && data->statusImage[SI_SENT] != NULL)
      DoMethod(obj, OM_ADDMEMBER, data->statusImage[SI_SENT]);
    else if(hasStatusNew(mail) && data->statusImage[SI_NEW] != NULL)
      DoMethod(obj, OM_ADDMEMBER, data->statusImage[SI_NEW]);
    else if(hasStatusHold(mail) && data->statusImage[SI_HOLD] != NULL)
      DoMethod(obj, OM_ADDMEMBER, data->statusImage[SI_HOLD]);
    else if(hasStatusRead(mail) && data->statusImage[SI_OLD] != NULL)
      DoMethod(obj, OM_ADDMEMBER, data->statusImage[SI_OLD]);
    else if(data->statusImage[SI_UNREAD] != NULL)
      DoMethod(obj, OM_ADDMEMBER, data->statusImage[SI_UNREAD]);

    // cleanup an eventually existing folder image
    if(data->folderImage != NULL)
    {
      DoMethod(obj, OM_REMMEMBER, data->folderImage);
      MUI_DisposeObject(data->folderImage);
      data->folderImage = NULL;
    }

    // in case the mail is part of a folder we go and
    // catch the folder image and name
    if(folder != NULL)
    {
      // set the folderLabel
      xset(data->folderLabel, MUIA_Text_PreParse, "\033l",
                              MUIA_Text_Contents, folder->Name);

      // get/create the folder image
      if(folder->imageObject)
      {
        char *imageID = (char *)xget(folder->imageObject, MUIA_ImageArea_ID);
        char *imageName = (char *)xget(folder->imageObject, MUIA_ImageArea_Filename);

        data->folderImage = MakeImageObject(imageID, imageName);
        D(DBF_GUI, "init imagearea: id '%s', file '%s'", imageID, imageName);
      }
      else if(folder->ImageIndex >= 0 && folder->ImageIndex < FICON_ID_MAX)
      {
        Object **imageArray = (Object **)xget(G->MA->GUI.NL_FOLDERS, MUIA_FolderListtree_ImageArray);

        D(DBF_GUI, "init imagearea: 0x%08lx[%ld]", imageArray, folder->ImageIndex);

        if(imageArray != NULL && imageArray[folder->ImageIndex] != NULL)
          data->folderImage = MakeImageObject(xget(imageArray[folder->ImageIndex], MUIA_ImageArea_ID), xget(imageArray[folder->ImageIndex], MUIA_ImageArea_Filename));
      }

      if(data->folderImage != NULL)
        DoMethod(obj, OM_ADDMEMBER, data->folderImage);

      D(DBF_GUI, "init finished..: 0x%08lx %ld", data->folderImage, folder->ImageIndex);
    }
    else
    {
      xset(data->folderLabel, MUIA_Text_PreParse, "\033l\033i",
                              MUIA_Text_Contents, tr(MSG_RE_VIRTUALMAIL));
    }

    // signal that we have added/modified the status Group successfully
    DoMethod(obj, MUIM_Group_ExitChange);
  }

  RETURN(0);
  return 0;
}
///
