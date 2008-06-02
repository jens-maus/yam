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

 Superclass:  MUIC_Virtgroup
 Description: Custom class to manage the attachments of a mail

 Credits: This class was highly inspired by the similar attachment group &
          image functionality available in Thunderbird and SimpleMail. Large
          code portions where borrowed by the iconclass implementation of
          SimpleMail to allow loading of the default icons via icon.library
          and supporting Drag&Drop on the workbench. Thanks sba! :)

***************************************************************************/

#include "AttachmentGroup_cl.h"

#include <proto/graphics.h>
#include <proto/icon.h>
#include <proto/wb.h>

#include "YAM_mainFolder.h"

#include "MUIObjects.h"

#include "Debug.h"

/* CLASSDATA
struct Data
{
  struct Part *firstPart;
  struct Part *selectedPart;

  Object *context_menu;

  struct MUI_EventHandlerNode ehnode;

  char menuTitle[SIZE_DEFAULT];

  BOOL eventHandlerAdded;
};
*/

#define BORDER    2 // border around our object
#define SPACING   2 // pixels taken as space between our images/text

/* Private Hooks */
/// LayoutHook
HOOKPROTONH(LayoutFunc, ULONG, Object *obj, struct MUI_LayoutMsg *lm)
{
  ENTER();

  switch(lm->lm_Type)
  {
    // MUI want's to know the min/max of the object so we
    // need to calculate it accordingly.
    case MUILM_MINMAX:
    {
      LONG maxMinWidth;
      LONG maxMinHeight;
      Object *cstate = (Object *)lm->lm_Children->mlh_Head;
      Object *child;

      maxMinWidth = 0;
      maxMinHeight = 0;
      while((child = NextObject(&cstate)) != NULL)
      {
        maxMinWidth = MAX(_minwidth(child), maxMinWidth);
        maxMinHeight = MAX(_minheight(child), maxMinHeight);
      }

      // add the borders
      maxMinWidth += 2*BORDER;
      maxMinHeight += 2*BORDER;

      // take four lines of text in case there was no child
      maxMinHeight = MAX(_font(obj)->tf_YSize * 4, maxMinHeight);

      // then set our calculated values
      lm->lm_MinMax.MinWidth  = maxMinWidth;
      lm->lm_MinMax.MinHeight = maxMinHeight;
      lm->lm_MinMax.DefWidth  = maxMinWidth;
      lm->lm_MinMax.DefHeight = maxMinHeight;
      lm->lm_MinMax.MaxWidth  = MUI_MAXMAX;
      lm->lm_MinMax.MaxHeight = maxMinHeight + 2*SPACING;

      RETURN(0);
      return 0;
    }
    break;

    // MUI asks' us to draw/layout the actual content of the
    // gadget, so we go and use MUI_Layout() to draw the gadget
    // components at the right position.
    case MUILM_LAYOUT:
    {
      Object *cstate = (Object *)lm->lm_Children->mlh_Head;
      Object *child;
      LONG left;
      LONG top;
      LONG lastItemHeight = 0;
      LONG maxWidth = BORDER;
      BOOL first = TRUE;

      D(DBF_GUI, "attgroup layout: %08lx %ld/%ld", obj, lm->lm_Layout.Width, lm->lm_Layout.Height);

      left = BORDER;
      top = BORDER;

      // Layout function. Here, we have to call MUI_Layout() for each
      // our children. MUI wants us to place them in a rectangle
      // defined by (0,0,lm->lm_Layout.Width-1,lm->lm_Layout.Height-1)
      // We are free to put the children anywhere in this rectangle.
      while((child = NextObject(&cstate)) != NULL)
      {
        LONG mw = _minwidth(child);
        LONG mh = _minheight(child);
        struct Part *mailPart = (struct Part *)xget(child, MUIA_AttachmentObject_MailPart);

        D(DBF_GUI, "layouting child %08lx '%s'", child, mailPart->Name);

        if(first == TRUE)
        {
          first = FALSE;
        }
        else
        {
          if(left + mw + SPACING > lm->lm_Layout.Width)
          {
            D(DBF_GUI, "layout: putting object '%s' on new row", mailPart->Name);
            // the current object doesn't fit in this row anymore, start a new row
            if(left > maxWidth)
              maxWidth = left;

            left = BORDER;
            top += lastItemHeight + SPACING;
            lastItemHeight = mh;
          }
        }

        D(DBF_GUI, "layout: x=%ld y=%ld w=%ld h=%ld '%s'", left, top+(mh-_minheight(child))/2, mw, mh, mailPart->Name);
        if(!MUI_Layout(child, left, top+(mh-_minheight(child))/2, mw, _minheight(child), 0))
        {
          RETURN(FALSE);
          return FALSE;
        }

        left += mw + SPACING;
        lastItemHeight = MAX(mh, lastItemHeight);
      }

      top += lastItemHeight + SPACING + BORDER;

      // update the layout dimensions in case we used more space than expected
      if(lm->lm_Layout.Width < maxWidth)
        lm->lm_Layout.Width = maxWidth;
      if(lm->lm_Layout.Height < top)
        lm->lm_Layout.Height = top;

      D(DBF_GUI, "attgroup layout: %08lx %ld/%ld", obj, lm->lm_Layout.Width, lm->lm_Layout.Height);

      RETURN(TRUE);
      return TRUE;
    }
    break;
  }

  RETURN(MUILM_UNKNOWN);
  return MUILM_UNKNOWN;
}
MakeStaticHook(LayoutHook, LayoutFunc);
///
/// Menu enumerations
enum { AMEN_DISPLAY=100, AMEN_SAVEAS, AMEN_PRINT, AMEN_SAVEALL, AMEN_SAVESEL, AMEN_CROPALL };
///

/* Overloaded Methods */
/// OVERLOAD(OM_NEW)
OVERLOAD(OM_NEW)
{
  ENTER();

  obj = DoSuperNew(cl, obj,
                    MUIA_Background,       MUII_GroupBack,
                    MUIA_Group_LayoutHook, &LayoutHook,
                    MUIA_ContextMenu,      TRUE,
                  TAG_MORE, inittags(msg));

  RETURN((ULONG)obj);
  return (ULONG)obj;
}
///
/// OVERLOAD(OM_DISPOSE)
OVERLOAD(OM_DISPOSE)
{
  GETDATA;

  // make sure that our context menus are also disposed
  if(data->context_menu)
    MUI_DisposeObject(data->context_menu);

  return DoSuperMethodA(cl, obj, msg);
}
///
/// OVERLOAD(OM_SET)
OVERLOAD(OM_SET)
{
  struct TagItem *tags = inittags(msg), *tag;

  while((tag = NextTagItem(&tags)))
  {
    switch(tag->ti_Tag)
    {
      // we also catch foreign attributes
      case MUIA_ShowMe:
      {
        // if the object should be hidden we clean it up also
        if(tag->ti_Data == FALSE)
          DoMethod(obj, MUIM_AttachmentGroup_Clear);
      }
      break;
    }
  }

  return DoSuperMethodA(cl, obj, msg);
}
///
/// OVERLOAD(MUIM_Setup)
OVERLOAD(MUIM_Setup)
{
  GETDATA;
  ULONG result;

  ENTER();

  // add an event handler for clearing the selection state
  // in case someone clicks in our area
  if((result = DoSuperMethodA(cl, obj, msg)))
  {
    data->ehnode.ehn_Priority = -2; // attachmentimage has priority -1
    data->ehnode.ehn_Flags    = 0;
    data->ehnode.ehn_Object   = obj;
    data->ehnode.ehn_Class    = cl;
    data->ehnode.ehn_Events   = IDCMP_MOUSEBUTTONS;

    DoMethod(_win(obj), MUIM_Window_AddEventHandler, &data->ehnode);
    data->eventHandlerAdded = TRUE;
  }

  RETURN(result);
  return result;
}

///
/// OVERLOAD(MUIM_Cleanup)
OVERLOAD(MUIM_Cleanup)
{
  GETDATA;
  ULONG result;

  ENTER();

  if(data->eventHandlerAdded == TRUE)
  {
    // remove the eventhandler first
    DoMethod(_win(obj), MUIM_Window_RemEventHandler, &data->ehnode);
    data->eventHandlerAdded = FALSE;
  }

  result = DoSuperMethodA(cl, obj, msg);

  RETURN(result);
  return result;
}

///
/// OVERLOAD(MUIM_ContextMenuBuild)
OVERLOAD(MUIM_ContextMenuBuild)
{
  GETDATA;
  struct MUIP_ContextMenuBuild *mb = (struct MUIP_ContextMenuBuild *)msg;
  struct List *childList = (struct List *)xget(obj, MUIA_Group_ChildList);
  struct Part *mailPart = NULL;

  // dispose the old context_menu if it still exists
  if(data->context_menu)
  {
    MUI_DisposeObject(data->context_menu);
    data->context_menu = NULL;
  }

  // now we find out if the user clicked in a specific attachmentimage or
  // if it was just a click in our group
  if(childList != NULL)
  {
    Object *cstate = (Object *)childList->lh_Head;
    Object *child;

    while((child = NextObject(&cstate)) != NULL)
    {
      if(_isinobject(child, mb->mx, mb->my))
      {
        mailPart = (struct Part *)xget(child, MUIA_AttachmentObject_MailPart);

        break;
      }
    }
  }

  // generate a context menu title now
  if(mailPart != NULL)
  {
    snprintf(data->menuTitle, sizeof(data->menuTitle), tr(MSG_MA_MIMEPART_MENU), mailPart->Nr);
    data->selectedPart = mailPart;
  }
  else
  {
    strlcpy(data->menuTitle, tr(MSG_Attachments), sizeof(data->menuTitle));
    data->selectedPart = NULL;
  }

  data->context_menu = MenustripObject,
    Child, MenuObjectT(data->menuTitle),
      Child, MenuitemObject, MUIA_Menuitem_Title, tr(MSG_MA_ATTACHMENT_DISPLAY),  MUIA_Menuitem_Enabled, mailPart != NULL, MUIA_UserData, AMEN_DISPLAY,  End,
      Child, MenuitemObject, MUIA_Menuitem_Title, tr(MSG_MA_ATTACHMENT_SAVEAS),   MUIA_Menuitem_Enabled, mailPart != NULL, MUIA_UserData, AMEN_SAVEAS,    End,
      Child, MenuitemObject, MUIA_Menuitem_Title, tr(MSG_MA_ATTACHMENT_PRINT),   MUIA_Menuitem_Enabled, mailPart != NULL && isPrintable(mailPart), MUIA_UserData, AMEN_PRINT,     End,
      Child, MenuitemObject, MUIA_Menuitem_Title, NM_BARLABEL, End,
      Child, MenuitemObject, MUIA_Menuitem_Title, tr(MSG_MA_ATTACHMENT_SAVEALL), MUIA_UserData, AMEN_SAVEALL,   End,
      Child, MenuitemObject, MUIA_Menuitem_Title, tr(MSG_MA_ATTACHMENT_SAVESEL), MUIA_UserData, AMEN_SAVESEL,   End,
      Child, MenuitemObject, MUIA_Menuitem_Title, tr(MSG_MA_ATTACHMENT_CROPALL), MUIA_UserData, AMEN_CROPALL, End,
    End,
  End;

  return (ULONG)data->context_menu;
}

///
/// OVERLOAD(MUIM_ContextMenuChoice)
OVERLOAD(MUIM_ContextMenuChoice)
{
  GETDATA;
  struct MUIP_ContextMenuChoice *m = (struct MUIP_ContextMenuChoice *)msg;

  switch(xget(m->item, MUIA_UserData))
  {
    case AMEN_DISPLAY:
      DoMethod(obj, MUIM_AttachmentGroup_Display, data->selectedPart);
    break;

    case AMEN_SAVEAS:
      DoMethod(obj, MUIM_AttachmentGroup_Save, data->selectedPart);
    break;

    case AMEN_PRINT:
      DoMethod(obj, MUIM_AttachmentGroup_Print, data->selectedPart);
    break;

    case AMEN_SAVEALL:
      DoMethod(obj, MUIM_AttachmentGroup_SaveAll);
    break;

    case AMEN_SAVESEL:
      DoMethod(obj, MUIM_AttachmentGroup_SaveSelected);
    break;

    case AMEN_CROPALL:
      DoMethod(obj, MUIM_AttachmentGroup_CropAll);
    break;

    default:
      return DoSuperMethodA(cl, obj, msg);
  }

  return 0;
}

///
/// OVERLOAD(MUIM_HandleEvent)
OVERLOAD(MUIM_HandleEvent)
{
  struct IntuiMessage *imsg = ((struct MUIP_HandleEvent *)msg)->imsg;
  ULONG result = 0;

  ENTER();

  if(imsg != NULL && imsg->Class == IDCMP_MOUSEBUTTONS)
  {
    // we clear the selection state in case the user clicked in
    // our area (= not an attachment image)
    if((imsg->Code == SELECTDOWN || imsg->Code == SELECTUP) &&
       _isinobject(obj, imsg->MouseX, imsg->MouseY))
    {
      DoMethod(obj, MUIM_AttachmentGroup_ClearSelection);

      result = MUI_EventHandlerRC_Eat;
    }
  }

  RETURN(result);
  return result;
}

///

/* Private Functions */

/* Public Methods */
/// DECLARE(Clear)
DECLARE(Clear)
{
  GETDATA;
  struct List *childList = (struct List *)xget(obj, MUIA_Group_ChildList);

  ENTER();

  // iterate through our child list and remove all attachmentimages
  if(childList != NULL)
  {
    if(DoMethod(obj, MUIM_Group_InitChange))
    {
      Object *cstate = (Object *)childList->lh_Head;
      Object *child;

      while((child = NextObject(&cstate)) != NULL)
      {
        DoMethod(obj, OM_REMMEMBER, child);
        MUI_DisposeObject(child);
      }

      DoMethod(obj, MUIM_Group_ExitChange);
    }
  }

  data->firstPart = NULL;

  RETURN(0);
  return 0;
}
///
/// DECLARE(Refresh)
DECLARE(Refresh) // struct Part *firstPart
{
  GETDATA;
  ULONG addedParts = 0;

  ENTER();

  // before we are going to add some new childs we have to clean
  // out all old children
  DoMethod(obj, MUIM_AttachmentGroup_Clear);

  // prepare the group for any change
  if(DoMethod(obj, MUIM_Group_InitChange))
  {
    struct Part *rp;

    // now we iterate through our message part list and
    // generate an own attachment image for each attachment
    for(rp = msg->firstPart; rp != NULL; rp = rp->Next)
    {
      if(rp->Nr > PART_RAW && rp->Nr != rp->rmData->letterPartNum && (C->DisplayAllAltPart ||
         (isAlternativePart(rp) == FALSE || rp->Parent == NULL || rp->Parent->MainAltPart == rp)))
      {
        Object *attObject;

        if((attObject = AttachmentObjectObject,
                          MUIA_AttachmentObject_MailPart,   rp,
                        End) != NULL)
        {
          Object *imageObject = (Object *)xget(attObject, MUIA_AttachmentObject_ImageObject);

          // connect some notifies which we might be interested in
          DoMethod(imageObject, MUIM_Notify, MUIA_AttachmentImage_DoubleClick, TRUE,
                   obj, 2, MUIM_AttachmentGroup_Display, rp);
          DoMethod(imageObject, MUIM_Notify, MUIA_AttachmentImage_DropPath, MUIV_EveryTime,
                   obj, 3, MUIM_AttachmentGroup_ImageDropped, imageObject, MUIV_TriggerValue);

          DoMethod(obj, OM_ADDMEMBER, attObject);

          D(DBF_GUI, "added attachment obj %08lx for attachment: %ld:%s mp: %08lx %08lx %08lx %08lx", attObject, rp->Nr, rp->Name, rp, rp->ContentType, rp->headerList, rp->rmData);

          addedParts++;
        }
      }
    }

    if(addedParts != 0)
      data->firstPart = msg->firstPart;
    else
      data->firstPart = NULL;

    // signal that the group relayouting is finished
    DoMethod(obj, MUIM_Group_ExitChange);
  }

  RETURN(addedParts);
  return addedParts;
}

///
/// DECLARE(Display)
DECLARE(Display) // struct Part *part
{
  ENTER();

  if(msg->part != NULL)
  {
    BOOL oldDecoded = isDecoded(msg->part);

    BusyText(tr(MSG_BusyDecDisplaying), "");

    // try to decode the message part
    if(RE_DecodePart(msg->part) == TRUE)
    {
      // run our MIME routines for displaying the part
      // to the user
      RE_DisplayMIME(msg->part->Filename, msg->part->ContentType);

      // if the part was decoded in RE_DecodePart() then
      // we issue a full refresh of the attachment image
      if(oldDecoded == FALSE && isDecoded(msg->part) == TRUE)
      {
        // issue a full redraw of the group which in fact
        // will issue a refresh of all images as well in
        // case they have changed.
        MUI_Redraw(obj, MADF_DRAWOBJECT);
      }
    }

    BusyEnd();
  }

  RETURN(0);
  return 0;
}
///
/// DECLARE(Save)
DECLARE(Save) // struct Part *part
{
  ENTER();

  if(msg->part != NULL)
  {
    BOOL oldDecoded = isDecoded(msg->part);

    BusyText(tr(MSG_BusyDecSaving), "");

    RE_DecodePart(msg->part);
    RE_Export(msg->part->rmData,
              msg->part->Filename, "",
              msg->part->CParFileName ? msg->part->CParFileName : msg->part->Name,
              msg->part->Nr,
              FALSE,
              FALSE,
              msg->part->ContentType);

    if(oldDecoded == FALSE && isDecoded(msg->part) == TRUE)
    {
      // now we know the exact size of the file and can redraw ourself
      MUI_Redraw(obj, MADF_DRAWOBJECT);
    }

    BusyEnd();
  }

  RETURN(0);
  return 0;
}
///
/// DECLARE(SaveAll)
DECLARE(SaveAll)
{
  GETDATA;

  ENTER();

  if(data->firstPart != NULL && data->firstPart->Next != NULL)
  {
    struct FileReqCache *frc;

    if((frc = ReqFile(ASL_DETACH, _win(obj), tr(MSG_RE_SaveMessage), (REQF_SAVEMODE|REQF_DRAWERSONLY), C->DetachDir, "")) != NULL)
    {
      BusyText(tr(MSG_BusyDecSaving), "");
      RE_SaveAll(data->firstPart->rmData, frc->drawer);
      BusyEnd();
    }
  }

  RETURN(0);
  return 0;
}
///
/// DECLARE(SaveSelected)
DECLARE(SaveSelected)
{
  struct List *childList;

  ENTER();

  BusyText(tr(MSG_BusyDecSaving), "");

  // iterate through our child list and remove all attachmentimages
  if((childList = (struct List *)xget(obj, MUIA_Group_ChildList)) != NULL)
  {
    Object *cstate = (Object *)childList->lh_Head;
    Object *child;
    BOOL oldDecoded = TRUE;
    BOOL newDecoded = FALSE;

    while((child = NextObject(&cstate)) != NULL)
    {
      Object *imageObject = (Object *)xget(child, MUIA_AttachmentObject_ImageObject);

      if(xget(imageObject, MUIA_Selected))
      {
        struct Part *mailPart;

        if((mailPart = (struct Part *)xget(imageObject, MUIA_AttachmentImage_MailPart)) != NULL)
        {
          oldDecoded &= isDecoded(mailPart);

          RE_DecodePart(mailPart);
          RE_Export(mailPart->rmData,
                    mailPart->Filename, "",
                    mailPart->CParFileName ? mailPart->CParFileName : mailPart->Name,
                    mailPart->Nr,
                    FALSE,
                    FALSE,
                    mailPart->ContentType);

          // at least one part has been decoded
          newDecoded = TRUE;
        }
      }
    }

    if(oldDecoded == FALSE && newDecoded == TRUE)
    {
      // At least one attachment was not decoded before this operation but is now.
      // Now we know the exact size of the file and can redraw ourself.
      MUI_Redraw(obj, MADF_DRAWOBJECT);
    }
  }

  BusyEnd();

  RETURN(0);
  return 0;
}
///
/// DECLARE(Print)
DECLARE(Print) // struct Part *part
{
  ENTER();

  if(msg->part != NULL)
  {
    BusyText(tr(MSG_BusyDecPrinting), "");
    RE_PrintFile(msg->part->Filename);
    BusyEnd();
  }

  RETURN(0);
  return 0;
}
///
/// DECLARE(CropAll)
DECLARE(CropAll)
{
  GETDATA;

  ENTER();

  if(data->firstPart != NULL)
  {
    struct ReadMailData *rmData = data->firstPart->rmData;
    struct Mail *mail = rmData->mail;

    // remove the attchments now
    MA_RemoveAttach(mail, TRUE);

    // make sure the listview is properly redrawn
    if(DoMethod(G->MA->GUI.PG_MAILLIST, MUIM_MainMailListGroup_RedrawMail, mail))
    {
      MA_ChangeSelected(TRUE);
      DisplayStatistics(mail->Folder, TRUE);
    }

    // the redraw of the mail is already done by MA_RemoveAttach()
  }

  RETURN(0);
  return 0;
}
///
/// DECLARE(ImageDropped)
DECLARE(ImageDropped) // Object *imageObject, char *dropPath
{
  struct Part *mailPart;

  ENTER();

  if((mailPart = (struct Part *)xget(msg->imageObject, MUIA_AttachmentImage_MailPart)) != NULL && msg->dropPath != NULL)
  {
    BOOL result;
    char *fileName;
    char filePathBuf[SIZE_PATHFILE];

    D(DBF_GUI, "Image of Part %ld was dropped at [%s]", mailPart->Nr, msg->dropPath);

    BusyText(tr(MSG_BusyDecSaving), "");

    // make sure the drawer is opened upon the drag operation
    if(WorkbenchBase->lib_Version >= 44)
      OpenWorkbenchObjectA(msg->dropPath, NULL);

    // prepare the final path
    fileName = mailPart->CParFileName;
    if(fileName == NULL || strlen(fileName) == 0)
    {
      fileName = mailPart->Name;
      if(fileName == NULL || strlen(fileName) == 0)
        fileName = mailPart->Filename;
    }
    AddPath(filePathBuf, msg->dropPath, fileName, sizeof(filePathBuf));

    RE_DecodePart(mailPart);
    result = RE_Export(mailPart->rmData,
                       mailPart->Filename,
                       filePathBuf,
                       "",
                       mailPart->Nr,
                       FALSE,
                       FALSE,
                       mailPart->ContentType);

    // let the workbench know about the change
    if(result == TRUE)
    {
      struct DiskObject *diskObject = (struct DiskObject *)xget(msg->imageObject, MUIA_AttachmentImage_DiskObject);

      // make sure to write out the diskObject of our attachment as well
      // but only if the filename doesn't end with a ".info" itself or it
      // clearly suggests that this might be a diskobject itself.
      if(diskObject != NULL)
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
  }

  RETURN(0);
  return 0;
}
///
/// DECLARE(ClearSelection)
DECLARE(ClearSelection)
{
  struct List *childList;

  ENTER();

  if((childList = (struct List *)xget(obj, MUIA_Group_ChildList)) != NULL)
  {
    Object *cstate = (Object *)childList->lh_Head;
    Object *child;

    while((child = NextObject(&cstate)) != NULL)
    {
      Object *imageObject = (Object *)xget(child, MUIA_AttachmentObject_ImageObject);

      set(imageObject, MUIA_Selected, FALSE);
    }
  }

  RETURN(0);
  return 0;
}

///
