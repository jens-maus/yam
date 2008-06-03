/***************************************************************************

 YAM - Yet Another Mailer
 Copyright (C) 1995-2000 by Marcel Beck <mbeck@yam.ch>
 Copyright (C) 2000-2008 by YAM Open Source Team

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
 YAM OpenSource project     : http://sourceforge.net/projects/yamos/

 $Id$

 Superclass:  MUIC_Group
 Description: Custom class to manage a single attachment of a mail

 Credits: This class was highly inspired by the similar attachment group &
          image functionality available in Thunderbird and SimpleMail. Large
          code portions where borrowed by the iconclass implementation of
          SimpleMail to allow loading of the default icons via icon.library
          and supporting Drag&Drop on the workbench. Thanks sba! :)

***************************************************************************/

#include "AttachmentObject_cl.h"

#include <proto/icon.h>
#include <proto/wb.h>

#include "YAM_mainFolder.h"

#include "MUIObjects.h"

#include "Debug.h"

/* CLASSDATA
struct Data
{
  Object *contextMenu;
  Object *imageObject;
  Object *textObject;
  struct Part *mailPart;
  char menuTitle[SIZE_DEFAULT];
};
*/

#define TEXTROWS  3 // how many text rows does a attachmentimage normally have?

/// Menu enumerations
enum {
  AMEN_DISPLAY=100,
  AMEN_SAVEAS,
  AMEN_PRINT
};

///

/* Overloaded Methods */
/// OVERLOAD(OM_NEW)
OVERLOAD(OM_NEW)
{
  Object *imageObject;
  Object *textObject;
  Object *attGroupObject = NULL;
  struct Part *mailPart = NULL;
  struct TagItem *tags = inittags(msg);
  struct TagItem *tag;

  ENTER();

  // check for some tags present at OM_NEW
  while((tag = NextTagItem(&tags)) != NULL)
  {
    switch(tag->ti_Tag)
    {
      ATTR(MailPart): mailPart = (struct Part *)tag->ti_Data; break;
      ATTR(Group):    attGroupObject = (Object *)tag->ti_Data; break;
    }
  }

  // create the object
  if((obj = DoSuperNew(cl, obj,
                         MUIA_ContextMenu,   TRUE,
                         MUIA_Group_Horiz,   TRUE,
                         MUIA_Group_Spacing, 0,
                         Child, imageObject = AttachmentImageObject,
                           MUIA_CycleChain,               TRUE,
                           MUIA_AttachmentImage_MailPart, mailPart,
                           MUIA_AttachmentImage_Group,    attGroupObject,
                         End,
                         Child, textObject = TextObject,
                           MUIA_Text_SetMax, FALSE,
                           MUIA_Font,        MUIV_Font_Tiny,
                         End,
                       TAG_MORE, inittags(msg))) != NULL)
  {
    GETDATA;

    data->imageObject = imageObject;
    data->textObject = textObject;
    data->mailPart = mailPart;

    // connect some notifies which we might be interested in
    DoMethod(imageObject, MUIM_Notify, MUIA_AttachmentImage_DoubleClick, TRUE,
             obj, 1, MUIM_AttachmentObject_Display);
    DoMethod(imageObject, MUIM_Notify, MUIA_AttachmentImage_DropPath, MUIV_EveryTime,
             obj, 2, MUIM_AttachmentObject_ImageDropped, MUIV_TriggerValue);

  }

  RETURN((ULONG)obj);
  return (ULONG)obj;
}

///
/// OVERLOAD(OM_DISPOSE)
OVERLOAD(OM_DISPOSE)
{
  GETDATA;

  // make sure that our context menus are also disposed
  if(data->contextMenu != NULL)
    MUI_DisposeObject(data->contextMenu);

  return DoSuperMethodA(cl, obj, msg);
}

///
/// OVERLOAD(OM_GET)
OVERLOAD(OM_GET)
{
  GETDATA;
  ULONG *store = ((struct opGet *)msg)->opg_Storage;

  switch(((struct opGet *)msg)->opg_AttrID)
  {
    ATTR(ImageObject) : *store = (ULONG)data->imageObject; return TRUE;
    ATTR(MailPart)    : *store = (ULONG)data->mailPart;    return TRUE;
  }

  return DoSuperMethodA(cl, obj, msg);
}

///
/// OVERLOAD(MUIM_Setup)
OVERLOAD(MUIM_Setup)
{
  GETDATA;
  ULONG result = 0;

  ENTER();

  if(data->mailPart != NULL && (result = DoSuperMethodA(cl, obj, msg)) != 0)
  {
    DoMethod(obj, MUIM_AttachmentObject_UpdateDescription);
    xset(data->imageObject, MUIA_AttachmentImage_MaxHeight, _font(obj) ? TEXTROWS*_font(obj)->tf_YSize+4 : 0,
                            MUIA_AttachmentImage_MaxWidth,  _font(obj) ? TEXTROWS*_font(obj)->tf_YSize+4 : 0);
  }

  RETURN(result);
  return result;
}

///
/// OVERLOAD(MUIM_ContextMenuBuild)
OVERLOAD(MUIM_ContextMenuBuild)
{
  GETDATA;

  // dispose the old context_menu if it still exists
  if(data->contextMenu != NULL)
  {
    MUI_DisposeObject(data->contextMenu);
    data->contextMenu = NULL;
  }

  if(data->mailPart != NULL)
  {
    snprintf(data->menuTitle, sizeof(data->menuTitle), tr(MSG_MA_MIMEPART_MENU), data->mailPart->Nr);

    data->contextMenu = MenustripObject,
      Child, MenuObjectT(data->menuTitle),
        Child, MenuitemObject, MUIA_Menuitem_Title, tr(MSG_MA_ATTACHMENT_DISPLAY), MUIA_UserData, AMEN_DISPLAY, End,
        Child, MenuitemObject, MUIA_Menuitem_Title, tr(MSG_MA_ATTACHMENT_SAVEAS),  MUIA_UserData, AMEN_SAVEAS,  End,
        Child, MenuitemObject, MUIA_Menuitem_Title, tr(MSG_MA_ATTACHMENT_PRINT),   MUIA_Menuitem_Enabled, isPrintable(data->mailPart), MUIA_UserData, AMEN_PRINT, End,
      End,
    End;
  }

  return (ULONG)data->contextMenu;
}

///
/// OVERLOAD(MUIM_ContextMenuChoice)
OVERLOAD(MUIM_ContextMenuChoice)
{
  struct MUIP_ContextMenuChoice *m = (struct MUIP_ContextMenuChoice *)msg;

  switch(xget(m->item, MUIA_UserData))
  {
    case AMEN_DISPLAY:
      DoMethod(obj, MUIM_AttachmentObject_Display);
    break;

    case AMEN_SAVEAS:
      DoMethod(obj, MUIM_AttachmentObject_Save);
    break;

    case AMEN_PRINT:
      DoMethod(obj, MUIM_AttachmentObject_Print);
    break;

    default:
      return DoSuperMethodA(cl, obj, msg);
  }

  return 0;
}

///

/* Private Functions */

/* Public Methods */
/// DECLARE(Display)
DECLARE(Display)
{
  GETDATA;

  ENTER();

  if(data->mailPart != NULL)
  {
    BOOL oldDecoded = isDecoded(data->mailPart);

    BusyText(tr(MSG_BusyDecDisplaying), "");

    // try to decode the message part
    if(RE_DecodePart(data->mailPart) == TRUE)
    {
      // run our MIME routines for displaying the part
      // to the user
      RE_DisplayMIME(data->mailPart->Filename, data->mailPart->ContentType);

      // if the part was decoded in RE_DecodePart() then
      // we issue a full refresh of the attachment image
      if(oldDecoded == FALSE && isDecoded(data->mailPart) == TRUE)
      {
        // issue a full redraw of the group which in fact
        // will issue a refresh of all images as well in
        // case they have changed.
        MUI_Redraw(data->imageObject, MADF_DRAWOBJECT);
        DoMethod(obj, MUIM_AttachmentObject_UpdateDescription);
      }
    }

    BusyEnd();
  }

  RETURN(0);
  return 0;
}

///
/// DECLARE(Save)
DECLARE(Save)
{
  GETDATA;

  ENTER();

  if(data->mailPart != NULL)
  {
    BOOL oldDecoded = isDecoded(data->mailPart);

    BusyText(tr(MSG_BusyDecSaving), "");

    RE_DecodePart(data->mailPart);
    RE_Export(data->mailPart->rmData,
              data->mailPart->Filename, "",
              data->mailPart->CParFileName ? data->mailPart->CParFileName : data->mailPart->Name,
              data->mailPart->Nr,
              FALSE,
              FALSE,
              data->mailPart->ContentType);

    if(oldDecoded == FALSE && isDecoded(data->mailPart) == TRUE)
    {
      // now we know the exact size of the file and can redraw ourself
      MUI_Redraw(data->imageObject, MADF_DRAWOBJECT);
      DoMethod(obj, MUIM_AttachmentObject_UpdateDescription);
    }

    BusyEnd();
  }

  RETURN(0);
  return 0;
}

///
/// DECLARE(Print)
DECLARE(Print)
{
  GETDATA;

  ENTER();

  if(data->mailPart != NULL)
  {
    BusyText(tr(MSG_BusyDecPrinting), "");
    RE_PrintFile(data->mailPart->Filename);
    BusyEnd();
  }

  RETURN(0);
  return 0;
}

///
/// DECLARE(ImageDropped)
DECLARE(ImageDropped) // char *dropPath
{
  GETDATA;
  BOOL result;
  char *fileName;
  char filePathBuf[SIZE_PATHFILE];

  ENTER();

  D(DBF_GUI, "image of Part %ld was dropped at [%s]", data->mailPart->Nr, msg->dropPath);

  BusyText(tr(MSG_BusyDecSaving), "");

  // make sure the drawer is opened upon the drag operation
  if(WorkbenchBase->lib_Version >= 44)
    OpenWorkbenchObjectA(msg->dropPath, NULL);

  // prepare the final path
  fileName = data->mailPart->CParFileName;
  if(fileName == NULL || strlen(fileName) == 0)
  {
    fileName = data->mailPart->Name;
    if(fileName == NULL || strlen(fileName) == 0)
      fileName = data->mailPart->Filename;
  }
  AddPath(filePathBuf, msg->dropPath, fileName, sizeof(filePathBuf));

  RE_DecodePart(data->mailPart);
  result = RE_Export(data->mailPart->rmData,
                     data->mailPart->Filename,
                     filePathBuf,
                     "",
                     data->mailPart->Nr,
                     FALSE,
                     FALSE,
                     data->mailPart->ContentType);

  // let the workbench know about the change
  if(result == TRUE)
  {
    struct DiskObject *diskObject;

    // make sure to write out the diskObject of our attachment as well
    // but only if the filename doesn't end with a ".info" itself or it
    // clearly suggests that this might be a diskobject itself.
    if((diskObject = (struct DiskObject *)xget(data->imageObject, MUIA_AttachmentImage_DiskObject)) != NULL)
    {
      char ext[SIZE_FILE];

      // extract the file extension.
      stcgfe(ext, fileName);

      if(stricmp(ext, "info") != 0)
        PutDiskObject(filePathBuf, diskObject);
      #if defined(__amigaos4__)
      else
      {
        // the following code makes sure that the workbench
        // gets notified of the .info file
        BPTR dlock;

        if((dlock = Lock(msg->dropPath, SHARED_LOCK)) != 0)
        {
          char *p;

          // strip an eventually existing extension
          if((p = strrchr(fileName, '.')) != NULL)
            *p = '\0';

          // UpdateWorkbench() seems to be only supported
          // by OS4. That's why this stuff is OS4 only.
          UpdateWorkbench(fileName, dlock, UPDATEWB_ObjectAdded);

          UnLock(dlock);
        }
      }
      #endif
    }

    // Now that the workbench knows about the new object we also have to make sure the icon
    // is actually visible in the window
    if(WorkbenchBase->lib_Version >= 44)
      MakeWorkbenchObjectVisibleA(filePathBuf, NULL);
  }
  else
    DisplayBeep(_screen(obj));

  BusyEnd();

  RETURN(0);
  return 0;
}

///
/// DECLARE(UpdateDescription)
DECLARE(UpdateDescription)
{
  GETDATA;
  char buffer[SIZE_PATH + SIZE_DEFAULT * 2];
  char sizeBuffer[SIZE_DEFAULT];

  ENTER();

  // first line: the attachment name
  if(isAlternativePart(data->mailPart))
  {
    strlcpy(buffer, MUIX_I "multipart/alternative" MUIX_N, sizeof(buffer));
  }
  else
  {
    if(data->mailPart->Name[0] != '\0')
      strlcpy(buffer, data->mailPart->Name, sizeof(buffer));
    else
      strlcpy(buffer, data->mailPart->Description, sizeof(buffer));
  }
  strlcat(buffer, "\n", sizeof(buffer));

  // second line: the attachment size
  if(isDecoded(data->mailPart))
  {
    FormatSize(data->mailPart->Size, sizeBuffer, sizeof(sizeBuffer), SF_AUTO);
  }
  else
  {
    sizeBuffer[0] = '~';
    FormatSize(data->mailPart->Size, &sizeBuffer[1], sizeof(sizeBuffer)-1, SF_AUTO);
  }
  strlcat(buffer, sizeBuffer, sizeof(buffer));
  strlcat(buffer, "\n", sizeof(buffer));

  // third line: the attachment description
  strlcat(buffer, DescribeCT(data->mailPart->ContentType), sizeof(buffer));

  set(data->textObject, MUIA_Text_Contents, buffer);

  RETURN(0);
  return 0;
}

///
