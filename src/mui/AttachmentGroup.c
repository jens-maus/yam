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

#include <string.h>
#include <proto/muimaster.h>

#include "SDI_hook.h"

#include "YAM_config.h"
#include "YAM_mainFolder.h"

#include "Busy.h"
#include "Locale.h"
#include "MUIObjects.h"

#include "mui/AttachmentObject.h"

#include "Debug.h"

/* CLASSDATA
struct Data
{
  struct Part *firstPart;

  Object *contextMenu;

  struct MUI_EventHandlerNode ehnode;

  char menuTitle[SIZE_DEFAULT];

  BOOL eventHandlerAdded;
};
*/

#define VERT_SPACING    4
#define HORIZ_SPACING   4

/* Private Hooks */
/// LayoutHook
HOOKPROTONH(LayoutFunc, ULONG, UNUSED Object *obj, struct MUI_LayoutMsg *lm)
{
  ENTER();

  switch(lm->lm_Type)
  {
    // MUI want's to know the min/max of the object so we
    // need to calculate it accordingly.
    case MUILM_MINMAX:
    {
      LONG maxMinWidth = 0;
      LONG maxMinHeight = 0;
      Object *cstate = (Object *)lm->lm_Children->mlh_Head;
      Object *child;

      while((child = NextObject(&cstate)) != NULL)
      {
        maxMinWidth = MAX(_minwidth(child), maxMinWidth);
        maxMinHeight = MAX(_minheight(child), maxMinHeight);
      }

      maxMinWidth += 2 * HORIZ_SPACING;
      maxMinHeight += 2 * VERT_SPACING;

      // then set our calculated values
      lm->lm_MinMax.MinWidth  = maxMinWidth;
      lm->lm_MinMax.MinHeight = maxMinHeight;
      lm->lm_MinMax.DefWidth  = maxMinWidth;
      lm->lm_MinMax.DefHeight = maxMinHeight;
      lm->lm_MinMax.MaxWidth  = MUI_MAXMAX;
      lm->lm_MinMax.MaxHeight = MUI_MAXMAX;

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
      LONG left = 0;
      LONG top = 0;
      LONG lastItemHeight = 0;
      LONG maxWidth = 0;
      BOOL first = TRUE;

      D(DBF_GUI, "attgroup layout: %08lx %ld/%ld", obj, lm->lm_Layout.Width, lm->lm_Layout.Height);

      // Layout function. Here, we have to call MUI_Layout() for each
      // our children. MUI wants us to place them in a rectangle
      // defined by (0,0,lm->lm_Layout.Width-1,lm->lm_Layout.Height-1)
      // We are free to put the children anywhere in this rectangle.
      while((child = NextObject(&cstate)) != NULL)
      {
        LONG mw = _minwidth(child);
        LONG mh = _minheight(child);
        #if defined(DEBUG)
        struct Part *mailPart = (struct Part *)xget(child, MUIA_AttachmentObject_MailPart);
        #endif

        D(DBF_GUI, "layouting child %08lx '%s'", child, mailPart->Name);

        if(first == TRUE)
          first = FALSE;
        else
        {
          if(left + mw + HORIZ_SPACING > lm->lm_Layout.Width)
          {
            // the current object doesn't fit in this row anymore, start a new row
            D(DBF_GUI, "layout: putting object '%s' on new row", mailPart->Name);

            // remember a possible new maximum width
            if(left > maxWidth)
              maxWidth = left;

            // start again from the left border, but go down one line
            left = 0;
            top += lastItemHeight + VERT_SPACING;
            lastItemHeight = mh;
          }
        }

        D(DBF_GUI, "layout: x=%ld y=%ld w=%ld h=%ld '%s'", left, top+(mh-_minheight(child))/2, mw, mh, mailPart->Name);
        if(!MUI_Layout(child, left, top+(mh-_minheight(child))/2, mw, _minheight(child), 0))
        {
          RETURN(FALSE);
          return FALSE;
        }

        left += mw + HORIZ_SPACING;
        lastItemHeight = MAX(mh, lastItemHeight);
      }

      top += lastItemHeight;

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
enum
{
  AMEN_SAVEALL=100,
  AMEN_SAVESEL,
  AMEN_DELETEALL,
  AMEN_DELETESEL
};

///

/* Overloaded Methods */
/// OVERLOAD(OM_NEW)
OVERLOAD(OM_NEW)
{
  ENTER();

  obj = DoSuperNew(cl, obj,
                    MUIA_Font,             MUIV_Font_Tiny,
                    MUIA_Group_LayoutHook, &LayoutHook,
                    MUIA_ContextMenu,      TRUE,
                  TAG_MORE, inittags(msg));

  RETURN((IPTR)obj);
  return (IPTR)obj;
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
/// OVERLOAD(OM_SET)
OVERLOAD(OM_SET)
{
  struct TagItem *tags = inittags(msg), *tag;

  while((tag = NextTagItem((APTR)&tags)) != NULL)
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
  IPTR result;

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
  IPTR result;

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

  ENTER();

  // dispose the old context_menu if it still exists
  if(data->contextMenu != NULL)
  {
    MUI_DisposeObject(data->contextMenu);
    data->contextMenu = NULL;
  }

  strlcpy(data->menuTitle, tr(MSG_Attachments), sizeof(data->menuTitle));

  data->contextMenu = MenustripObject,
    Child, MenuObjectT(data->menuTitle),
      Child, MenuitemObject, MUIA_Menuitem_Title, tr(MSG_MA_ATTACHMENT_SAVEALL), MUIA_UserData, AMEN_SAVEALL, End,
      Child, MenuitemObject, MUIA_Menuitem_Title, tr(MSG_MA_ATTACHMENT_SAVESEL), MUIA_UserData, AMEN_SAVESEL, End,
      Child, MenuitemObject, MUIA_Menuitem_Title, tr(MSG_MA_ATTACHMENT_DELETEALL), MUIA_UserData, AMEN_DELETEALL, End,
      Child, MenuitemObject, MUIA_Menuitem_Title, tr(MSG_MA_ATTACHMENT_DELETESEL), MUIA_UserData, AMEN_DELETESEL, End,
    End,
  End;

  RETURN(data->contextMenu);
  return (IPTR)data->contextMenu;
}

///
/// OVERLOAD(MUIM_ContextMenuChoice)
OVERLOAD(MUIM_ContextMenuChoice)
{
  struct MUIP_ContextMenuChoice *m = (struct MUIP_ContextMenuChoice *)msg;

  switch(xget(m->item, MUIA_UserData))
  {
    case AMEN_SAVEALL:
      DoMethod(obj, MUIM_AttachmentGroup_SaveAll);
    break;

    case AMEN_SAVESEL:
      DoMethod(obj, MUIM_AttachmentGroup_SaveSelected);
    break;

    case AMEN_DELETEALL:
      DoMethod(obj, MUIM_AttachmentGroup_DeleteAll);
    break;

    case AMEN_DELETESEL:
      DoMethod(obj, MUIM_AttachmentGroup_DeleteSelected);
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
  IPTR result = 0;

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

  // iterate through our child list and remove all attachment objects
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
                          MUIA_AttachmentObject_MailPart, rp,
                          MUIA_AttachmentObject_Group,    obj,
                        End) != NULL)
        {
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
      struct BusyNode *busy;

      busy = BusyBegin(BUSY_TEXT);
      BusyText(busy, tr(MSG_BusyDecSaving), "");
      RE_SaveAll(data->firstPart->rmData, frc->drawer);
      BusyEnd(busy);
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
  struct BusyNode *busy;

  ENTER();

  busy = BusyBegin(BUSY_TEXT);
  BusyText(busy, tr(MSG_BusyDecSaving), "");

  // iterate through our child list
  if((childList = (struct List *)xget(obj, MUIA_Group_ChildList)) != NULL)
  {
    Object *cstate = (Object *)childList->lh_Head;
    Object *child;

    // invoke the method for all selected items
    while((child = NextObject(&cstate)) != NULL)
    {
      if(xget(child, MUIA_Selected) == TRUE)
        DoMethod(child, MUIM_AttachmentObject_Save);
    }
  }

  BusyEnd(busy);

  RETURN(0);
  return 0;
}

///
/// DECLARE(DeleteAll)
DECLARE(DeleteAll)
{
  GETDATA;

  ENTER();

  if(data->firstPart != NULL)
  {
    // remove all attachments now
    MA_RemoveAttach(data->firstPart->rmData->mail, NULL, C->ConfirmRemoveAttachments);
  }

  RETURN(0);
  return 0;
}

///
/// DECLARE(DeleteSelected)
DECLARE(DeleteSelected)
{
  GETDATA;
  struct List *childList;

  ENTER();

  // iterate through our child list
  if((childList = (struct List *)xget(obj, MUIA_Group_ChildList)) != NULL)
  {
    ULONG numSelected = 0;
    Object *cstate = (Object *)childList->lh_Head;
    Object *child;
    struct Part **parts;

    // first count the number of selected attachments
    while((child = NextObject(&cstate)) != NULL)
    {
      if(xget(child, MUIA_Selected) == TRUE)
        numSelected++;
    }

    // now build a list of selected attachments
    if(numSelected > 0 && (parts = calloc(numSelected + 1, sizeof(*parts))) != NULL)
    {
      ULONG i = 0;

      cstate = (Object *)childList->lh_Head;
      while((child = NextObject(&cstate)) != NULL)
      {
        if(xget(child, MUIA_Selected) == TRUE)
          parts[i++] = (struct Part *)xget(child, MUIA_AttachmentObject_MailPart);
      }

      // and finally remove the attachments
      MA_RemoveAttach(data->firstPart->rmData->mail, parts, C->ConfirmRemoveAttachments);

      free(parts);
    }
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
      D(DBF_GUI, "clearing MUIA_Selected of object %08lx", child);
      set(child, MUIA_Selected, FALSE);
    }
  }

  RETURN(0);
  return 0;
}

///
