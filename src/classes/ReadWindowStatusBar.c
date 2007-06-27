/***************************************************************************

 YAM - Yet Another Mailer
 Copyright (C) 1995-2000 by Marcel Beck <mbeck@yam.ch>
 Copyright (C) 2000-2007 by YAM Open Source Team

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

#include <proto/graphics.h>

#include "Debug.h"

/* CLASSDATA
struct Data
{
  Object *statusImage[MAX_STATUSIMG];
  Object *folderImage;
  Object *folderLabel;

  ULONG minHeight;
};
*/

/* Private Hooks */
/// LayoutHook
HOOKPROTONH(LayoutFunc, ULONG, Object *obj, struct MUI_LayoutMsg *lm)
{
  struct Data *data = (struct Data *)xget(obj, MUIA_UserData);

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

      RETURN(0);
      return 0;
    }
    break;

    // MUI asks' us to draw/layout the actual content of the
    // gadget, so we go and use MUI_Layout() to draw the gadget
    // components at the right position.
    case MUILM_LAYOUT:
    {
      BOOL result = TRUE;
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

      RETURN(result);
      return result;
    }
    break;
  }

  RETURN(MUILM_UNKNOWN);
  return MUILM_UNKNOWN;
}
MakeStaticHook(LayoutHook, LayoutFunc);
///

/* Private Functions */

/* Overloaded Methods */
/// OVERLOAD(OM_NEW)
OVERLOAD(OM_NEW)
{
  // *don't* add MAXBCSTATUSIMG as size since it would fill the missing
  // entries with NULL values...
  static const struct { const int status; const char *const name; } image[] =
  {
    { SICON_ID_UNREAD,   "status_unread" },
    { SICON_ID_OLD,      "status_old" },
    { SICON_ID_FORWARD,  "status_forward" },
    { SICON_ID_REPLY,    "status_reply" },
    { SICON_ID_WAITSEND, "status_waitsend" },
    { SICON_ID_ERROR,    "status_error" },
    { SICON_ID_HOLD,     "status_hold" },
    { SICON_ID_SENT,     "status_sent" },
    { SICON_ID_NEW,      "status_new" },
    { SICON_ID_DELETE,   "status_delete" },
    { SICON_ID_DOWNLOAD, "status_download" },
    { SICON_ID_GROUP,    "status_group" },
    { SICON_ID_URGENT,   "status_urgent" },
    { SICON_ID_ATTACH,   "status_attach" },
    { SICON_ID_REPORT,   "status_report" },
    { SICON_ID_CRYPT,    "status_crypt" },
    { SICON_ID_SIGNED,   "status_signed" },
    { SICON_ID_MARK,     "status_mark" },
    { SICON_ID_SPAM,     "status_spam" }
  };
  Object *statusImage[MAX_STATUSIMG];
  Object *folderLabel;
  ULONG i;
  ULONG minHeight = 0;

  ENTER();

  // make sure that all icons are listed!
  ASSERT(ARRAY_SIZE(image) == MAX_STATUSIMG);

  // prepare the status icons for adding it later on to our statusGroup object
  for(i=0; i < ARRAY_SIZE(image); i++)
  {
    Object *newImage;

    if((newImage = MakeImageObject(image[i].name, image[i].name)) != NULL)
    {
      ULONG rawHeight = xget(newImage, MUIA_ImageArea_RawHeight);

      minHeight = MAX(rawHeight, minHeight);
    }

    statusImage[image[i].status] = newImage;
  }

  // should be a compile-time nop!
  for(i = ARRAY_SIZE(image); i < MAX_STATUSIMG; i++)
    statusImage[i] = NULL;

  obj = DoSuperNew(cl, obj,
                    MUIA_ContextMenu,       FALSE,
                    MUIA_Group_Horiz,       TRUE,
                    MUIA_Group_Spacing,     0,
                    MUIA_Group_LayoutHook,  &LayoutHook,
                    Child, folderLabel = TextObject,
                      MUIA_Font,          MUIV_Font_Tiny,
                      MUIA_Frame,         MUIV_Frame_None,
                      MUIA_Text_PreParse, "\033l",
                    End,
                  TAG_MORE, inittags(msg));

  if(obj != NULL)
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

  RETURN((ULONG)obj);
  return (ULONG)obj;
}
///
/// OVERLOAD(OM_DISPOSE)
OVERLOAD(OM_DISPOSE)
{
  GETDATA;
  int i;
  ULONG result;

  ENTER();

  // clear all children of the statusGroup first
  if(DoMethod(obj, MUIM_Group_InitChange))
  {
    struct List *childList;

    // we first remove all childs from our statusGroup
    if((childList = (struct List *)xget(obj, MUIA_Group_ChildList)) != NULL)
    {
      Object *cstate = (Object *)childList->lh_Head;
      Object *child;

      while((child = NextObject(&cstate)))
      {
        for(i=0; i < MAX_STATUSIMG; i++)
        {
          if(data->statusImage[i] == child)
          {
            DoMethod(obj, OM_REMMEMBER, child);
            continue;
          }
        }
      }
    }

    DoMethod(obj, MUIM_Group_ExitChange);
  }

  // now we can free all status icons
  for(i=0; i < MAX_STATUSIMG; i++)
  {
    if(data->statusImage[i])
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
  struct Mail *mail = msg->mail;

  ENTER();

  // update the statusgroup by removing/adding items accordingly
  if(DoMethod(obj, MUIM_Group_InitChange))
  {
    struct Folder *folder = mail->Folder;

    struct List *childList;

    // we first remove all childs from our statusGroup
    if((childList = (struct List *)xget(obj, MUIA_Group_ChildList)) != NULL)
    {
      Object *cstate = (Object *)childList->lh_Head;
      Object *child;

      while((child = NextObject(&cstate)) != NULL)
      {
        ULONG i;

        for(i = 0; i < ARRAY_SIZE(data->statusImage); i++)
        {
          if(data->statusImage[i] == child)
          {
            DoMethod(obj, OM_REMMEMBER, child);

            continue;
          }
        }

      }

      // now we can add the status icons depending on the set status flags of
      // the mail (sort upside-down)

      // StatusGroup 9 (Spam status)
      if(hasStatusSpam(mail) && data->statusImage[SICON_ID_SPAM])
        DoMethod(obj, OM_ADDMEMBER, data->statusImage[SICON_ID_SPAM]);

      // StatusGroup 8 (Forwarded status)
      if(hasStatusForwarded(mail) && data->statusImage[SICON_ID_FORWARD])
        DoMethod(obj, OM_ADDMEMBER, data->statusImage[SICON_ID_FORWARD]);

      // StatusGroup 7 (Replied status)
      if(hasStatusReplied(mail) && data->statusImage[SICON_ID_REPLY])
        DoMethod(obj, OM_ADDMEMBER, data->statusImage[SICON_ID_REPLY]);

      // StatusGroup 6 (marked flag)
      if(hasStatusMarked(mail) && data->statusImage[SICON_ID_MARK])
        DoMethod(obj, OM_ADDMEMBER, data->statusImage[SICON_ID_MARK]);

      // StatusGroup 5 (New/Hold info)
      if(hasStatusNew(mail) && data->statusImage[SICON_ID_NEW])
        DoMethod(obj, OM_ADDMEMBER, data->statusImage[SICON_ID_NEW]);
      else if(hasStatusHold(mail) && data->statusImage[SICON_ID_HOLD])
        DoMethod(obj, OM_ADDMEMBER, data->statusImage[SICON_ID_HOLD]);

      // StatusGroup 4 (multipart info)
      if(isMP_MixedMail(mail) && data->statusImage[SICON_ID_ATTACH])
        DoMethod(obj, OM_ADDMEMBER, data->statusImage[SICON_ID_ATTACH]);

      // StatusGroup 3 (report mail info)
      if(isMP_ReportMail(mail) && data->statusImage[SICON_ID_REPORT])
        DoMethod(obj, OM_ADDMEMBER, data->statusImage[SICON_ID_REPORT]);

      // StatusGroup 2 (signed/crypted status)
      if(isMP_CryptedMail(mail) && data->statusImage[SICON_ID_CRYPT])
        DoMethod(obj, OM_ADDMEMBER, data->statusImage[SICON_ID_CRYPT]);
      else if(isMP_SignedMail(mail) && data->statusImage[SICON_ID_SIGNED])
        DoMethod(obj, OM_ADDMEMBER, data->statusImage[SICON_ID_SIGNED]);

      // StatusGroup 1 (importance level)
      if(getImportanceLevel(mail) == IMP_HIGH && data->statusImage[SICON_ID_URGENT])
        DoMethod(obj, OM_ADDMEMBER, data->statusImage[SICON_ID_URGENT]);

      // StatusGroup 0 (main mail status)
      if((hasStatusError(mail) || isPartialMail(mail)) && data->statusImage[SICON_ID_ERROR])
        DoMethod(obj, OM_ADDMEMBER, data->statusImage[SICON_ID_ERROR]);
      else if(hasStatusQueued(mail) && data->statusImage[SICON_ID_WAITSEND])
        DoMethod(obj, OM_ADDMEMBER, data->statusImage[SICON_ID_WAITSEND]);
      else if(hasStatusSent(mail) && data->statusImage[SICON_ID_SENT])
        DoMethod(obj, OM_ADDMEMBER, data->statusImage[SICON_ID_SENT]);
      else if(hasStatusRead(mail) && data->statusImage[SICON_ID_OLD])
        DoMethod(obj, OM_ADDMEMBER, data->statusImage[SICON_ID_OLD]);
      else if(data->statusImage[SICON_ID_UNREAD])
        DoMethod(obj, OM_ADDMEMBER, data->statusImage[SICON_ID_UNREAD]);

      // cleanup an eventually existing folder image
      if(data->folderImage)
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
        SetAttrs(data->folderLabel, MUIA_Text_PreParse, "\033l",
                                    MUIA_Text_Contents, folder->Name,
                                    TAG_DONE);

        // get/create the folder image
        if(folder->imageObject)
        {
          char *imageID = (char *)xget(folder->imageObject, MUIA_ImageArea_ID);
          char *imageName = (char *)xget(folder->imageObject, MUIA_ImageArea_Filename);

          data->folderImage = MakeImageObject(imageID, imageName);
          D(DBF_GUI, "init imagearea: id '%s', file '%s'", imageName);
        }
        else if(folder->ImageIndex >= 0 && folder->ImageIndex <= MAX_FOLDERIMG)
        {
          Object **imageArray = (Object **)xget(G->MA->GUI.NL_FOLDERS, MUIA_MainFolderListtree_ImageArray);

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
        SetAttrs(data->folderLabel, MUIA_Text_PreParse, "\033l\033i",
                                    MUIA_Text_Contents, tr(MSG_RE_VIRTUALMAIL),
                                    TAG_DONE);
      }
    }

    // signal that we have added/modified the status Group successfully
    DoMethod(obj, MUIM_Group_ExitChange);
  }

  RETURN(0);
  return 0;
}
///
