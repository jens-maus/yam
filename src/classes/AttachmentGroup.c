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

#include "Debug.h"

/* CLASSDATA
struct Data
{
  struct Part *firstPart;
  struct Part *selectedPart;

  Object *context_menu;

  char menuTitle[SIZE_DEFAULT];

  ULONG minHeight;
  BOOL resizePushed;
};
*/

#define BORDER    2 // border around our object
#define SPACING    2 // pixels taken as space between our images/text
#define TEXTROWS  3 // how many text rows does a attachmentimage normally have?

/// LayoutHook
HOOKPROTONH(LayoutFunc, ULONG, Object *obj, struct MUI_LayoutMsg *lm)
{
  ENTER();

  switch(lm->lm_Type)
  {
    case MUILM_MINMAX:
    {
      LONG maxMinHeight;
      LONG objMinHeight = xget(obj, MUIA_AttachmentGroup_MinHeight);
      Object *cstate = (Object *)lm->lm_Children->mlh_Head;
      Object *child;
      LONG childs = 0;

      maxMinHeight = MAX(2*BORDER+_font(obj)->tf_YSize, objMinHeight);
      while((child = NextObject(&cstate)))
      {
        // we know our childs and that we only carry AttachmentImages
        // normally
        struct Part *mailPart = (struct Part *)xget(child, MUIA_AttachmentImage_MailPart);
        if(mailPart)
        {
          maxMinHeight = MAX(_minheight(child)+2*BORDER, maxMinHeight);
          childs++;
        }
      }

      if(childs)
        maxMinHeight = MAX(2*BORDER+TEXTROWS*_font(obj)->tf_YSize, maxMinHeight);

      if(objMinHeight != maxMinHeight)
        set(obj, MUIA_AttachmentGroup_MinHeightNoPush, maxMinHeight);

      // then set our calculated values
      lm->lm_MinMax.MinWidth  = 0;
      lm->lm_MinMax.MinHeight = maxMinHeight;
      lm->lm_MinMax.DefWidth  = MUI_MAXMAX;
      lm->lm_MinMax.DefHeight = maxMinHeight;
      lm->lm_MinMax.MaxWidth  = MUI_MAXMAX;
      lm->lm_MinMax.MaxHeight = maxMinHeight;

      RETURN(0);
      return 0;
    }
    break;

    case MUILM_LAYOUT:
    {
      struct RastPort rp;
      Object *cstate = (Object *)lm->lm_Children->mlh_Head;
      Object *child;
      LONG left = BORDER;
      LONG top = BORDER;
      LONG mainLabelWidth;
      LONG itemsInRow = 0;
      LONG lastItemHeight = 0;
      LONG usedHeight = BORDER+_font(obj)->tf_YSize;
      LONG objMinHeight = xget(obj, MUIA_AttachmentGroup_MinHeight);

      D(DBF_GUI, "attgroup layout: %lx %ld/%ld %ld/%ld", obj, _mwidth(obj), _mheight(obj), lm->lm_Layout.Width, lm->lm_Layout.Height);

      InitRastPort(&rp);
      SetFont(&rp, _font(obj));
      SetSoftStyle(&rp, FSF_BOLD, AskSoftStyle(&rp));
      mainLabelWidth = TextLength(&rp, GetStr(MSG_MA_ATTACHMENTS), strlen(GetStr(MSG_MA_ATTACHMENTS))) + SPACING;
      left += mainLabelWidth;

      // Layout function. Here, we have to call MUI_Layout() for each
      // our children. MUI wants us to place them in a rectangle
      // defined by (0,0,lm->lm_Layout.Width-1,lm->lm_Layout.Height-1)
      // We are free to put the children anywhere in this rectangle.
      SetSoftStyle(&rp, FS_NORMAL, AskSoftStyle(&rp));
      while((child = NextObject(&cstate)))
      {
        LONG mw = _minwidth(child);
        LONG mh = _minheight(child);
        struct Part *mailPart = (struct Part *)xget(child, MUIA_AttachmentImage_MailPart);

        D(DBF_GUI, "layouting child %08lx - mp: %08lx %08lx %08lx %08lx", child, mailPart, mailPart->ContentType, mailPart->headerList, mailPart->rmData);

        if(mailPart)
        {
          const char *ctDescr = DescribeCT(mailPart->ContentType);
          LONG partNameLen = TextLength(&rp, mailPart->Name, strlen(mailPart->Name));
          LONG contentTypeLen = TextLength(&rp, ctDescr, strlen(ctDescr));
          LONG sizeLabelLen;
          LONG largestLabelLen = MAX(partNameLen, contentTypeLen) + 10;
          LONG labelHeight = TEXTROWS*_font(obj)->tf_YSize;
          char buf[SIZE_DEFAULT];

          if(mailPart->Decoded == FALSE)
          {
            buf[0] = '~';
            FormatSize(mailPart->Size, &buf[1], sizeof(buf)-1, SF_AUTO);
          }
          else
            FormatSize(mailPart->Size, buf, sizeof(buf), SF_AUTO);

          sizeLabelLen = TextLength(&rp, buf, strlen(buf));
          largestLabelLen = MAX(sizeLabelLen, largestLabelLen);

          mh = MAX(mh, labelHeight);

          // before we are going to layout anything at all, we have to evaluate
          // if the object fits in the current row or if we have to put it in a second row
          if(left+mw+SPACING+largestLabelLen > _mwidth(obj))
          {
            LONG requiredHeight = top+lastItemHeight+SPACING+mh;

            D(DBF_GUI, "obj [%s] doesn't fit in current row! %ld %ld %ld", mailPart->Name, itemsInRow, requiredHeight, objMinHeight);
            
            // the objects doesn't seem to fit into the current line,
            // so we have to put it in another line.
            // but for that we have to check if our group has currently enough
            // space to take that new row.
            if(requiredHeight > objMinHeight)
            {
              // before we signal our group to relayout to a better height we
              // check if we have at least one more item in the row or
              // if we have to forget the relayout because there isn't enough space
              // for another height increase anyway
              if(itemsInRow > 0)
              {
                D(DBF_GUI, "group isn't high enough, signal relayout to get a height of %ld", requiredHeight);

                // our group doesn't seem to be high enough so we have to signal
                // it that it should relayout the whole thing
                set(obj, MUIA_AttachmentGroup_MinHeight, requiredHeight);

                RETURN(TRUE);
                return TRUE;
              }
            }
            else
            {
              if(lastItemHeight > 0)
              {
                top += lastItemHeight + SPACING;
                left = BORDER + mainLabelWidth;
              }

              itemsInRow = 0;
            }
          }
          
          D(DBF_GUI, "layout: %ld %ld %ld", mh, _minheight(child), _font(obj)->tf_YSize);
          if(!MUI_Layout(child, left, top+(mh-_minheight(child))/2, mw, _minheight(child), 0))
          {
            RETURN(FALSE);
            return FALSE;
          }

          lastItemHeight = mh;
          itemsInRow++;
          usedHeight = MAX(top+mh, usedHeight);

          left += mw + SPACING + largestLabelLen;
        }
      }

      // Now that we end up here we have to check whether our object used all
      // of its provided height space or if we have to reduce the minHeight here
      if(lm->lm_Layout.Height-1 > usedHeight+BORDER)
      {
        D(DBF_GUI, "group uses too much space, reducing and relayouting.. %ld > %ld", lm->lm_Layout.Height-1, usedHeight+BORDER);
        set(obj, MUIA_AttachmentGroup_MinHeight, usedHeight+BORDER);
      }

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
                    MUIA_Font,             MUIV_Font_Tiny,
                    MUIA_Group_LayoutHook, &LayoutHook,
                    MUIA_ContextMenu,       TRUE,
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
/// OVERLOAD(OM_GET)
OVERLOAD(OM_GET)
{
  GETDATA;
  ULONG *store = ((struct opGet *)msg)->opg_Storage;

  switch(((struct opGet *)msg)->opg_AttrID)
  {
    ATTR(MinHeight) : *store = data->minHeight; return TRUE;
  }

  return DoSuperMethodA(cl, obj, msg);
}
///
/// OVERLOAD(OM_SET)
OVERLOAD(OM_SET)
{
  GETDATA;

  struct TagItem *tags = inittags(msg), *tag;
  while((tag = NextTagItem(&tags)))
  {
    switch(tag->ti_Tag)
    {
      ATTR(MinHeight):
      {
        data->minHeight = tag->ti_Data;

        if(data->resizePushed == FALSE)
        {
          data->resizePushed = TRUE;
          DoMethod(_app(obj), MUIM_Application_PushMethod, obj, 1, MUIM_AttachmentGroup_Relayout);
        }

        // make the superMethod call ignore those tags
        tag->ti_Tag = TAG_IGNORE;
      }
      break;

      ATTR(MinHeightNoPush):
      {
        data->minHeight = tag->ti_Data;

        // make the superMethod call ignore those tags
        tag->ti_Tag = TAG_IGNORE;
      }
      break;

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
/// OVERLOAD(MUIM_Draw)
OVERLOAD(MUIM_Draw)
{
  // call the supermethod first
  DoSuperMethodA(cl, obj, msg);

  // now we can start our draw operation where we have to
  // draw different text objects into our group
  if(((struct MUIP_Draw *)msg)->flags & MADF_DRAWOBJECT)
  {
    const char *attachmentLabel = GetStr(MSG_MA_ATTACHMENTS);
    struct List *childList = (struct List *)xget(obj, MUIA_Group_ChildList);
    struct TextExtent te;
    int cnt;

    // make sure we do not draw outside
    if(_mleft(obj) <= 0 || _mtop(obj) < 10)
      return 0;

    // let us first draw the "Attachments:" label
    SetAPen(_rp(obj), _dri(obj)->dri_Pens[TEXTPEN]);
    SetFont(_rp(obj), _font(obj));
    Move(_rp(obj), _mleft(obj) + BORDER, _mtop(obj) + _font(obj)->tf_Baseline + BORDER);
    SetSoftStyle(_rp(obj), FSF_BOLD, AskSoftStyle(_rp(obj)));
    cnt = TextFit(_rp(obj), attachmentLabel, strlen(attachmentLabel), &te, NULL, 1, _mwidth(obj)-2*BORDER, _mheight(obj)-2*BORDER);
    if(cnt)
      Text(_rp(obj), attachmentLabel, cnt);

    // then we have to place the other labels for our images right beside
    // them.
    SetSoftStyle(_rp(obj), FS_NORMAL, AskSoftStyle(_rp(obj)));
    if(childList)
    {
      Object *cstate = (Object *)childList->lh_Head;
      Object *child;

      while((child = NextObject(&cstate)))
      {
        struct Part *mailPart = (struct Part *)xget(child, MUIA_AttachmentImage_MailPart);

        // make sure this child is valid and does not draw outside
        if(mailPart && _mtop(child) > 10 && _mleft(child) > 10)
        {
          LONG maxHeight = MAX(_mheight(child), TEXTROWS*_font(obj)->tf_YSize);
          LONG topPosition = _mtop(child)+_font(obj)->tf_Baseline-(maxHeight-_mheight(child))/2;
          LONG textSpaceWidth  = _mwidth(obj)-((_mright(child)+SPACING)-_mleft(obj))-BORDER;
          LONG textSpaceHeight = _mheight(obj)-((topPosition-_font(obj)->tf_Baseline)-_mtop(obj));

          if(textSpaceWidth > 0 && textSpaceHeight > 0)
          {
            cnt = TextFit(_rp(obj), mailPart->Name, strlen(mailPart->Name), &te, NULL, 1, textSpaceWidth, textSpaceHeight);
            if(cnt > 0)
            {
              // move the rastport to the start where the text should be placed
              Move(_rp(obj), _mright(child)+SPACING, topPosition);
              Text(_rp(obj), mailPart->Name, cnt);
            }

            textSpaceHeight -= _font(obj)->tf_YSize;
            topPosition += _font(obj)->tf_YSize;
            if(textSpaceHeight > 0)
            {
              char buf[SIZE_DEFAULT];

              if(mailPart->Decoded == FALSE)
              {
                buf[0] = '~';
                FormatSize(mailPart->Size, &buf[1], sizeof(buf)-1, SF_AUTO);
              }
              else
                FormatSize(mailPart->Size, buf, sizeof(buf), SF_AUTO);

              cnt = TextFit(_rp(obj), buf, strlen(buf), &te, NULL, 1, textSpaceWidth, textSpaceHeight);
              if(cnt > 0)
              {
                Move(_rp(obj), _mright(child)+SPACING, topPosition);
                Text(_rp(obj), buf, cnt);
              }

              textSpaceHeight -= _font(obj)->tf_YSize;
              topPosition += _font(obj)->tf_YSize;
              if(textSpaceHeight > 0)
              {
                const char *ctDescr = DescribeCT(mailPart->ContentType);
                cnt = TextFit(_rp(obj), ctDescr, strlen(ctDescr), &te, NULL, 1, textSpaceWidth, textSpaceHeight);
                if(cnt > 0)
                {
                  Move(_rp(obj), _mright(child)+SPACING, topPosition);
                  Text(_rp(obj), ctDescr, cnt);
                }
              }
            }
          }
        }
      }
    }
  }

  return 0;
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
  if(childList)
  {
    Object *cstate = (Object *)childList->lh_Head;
    Object *child;

    while((child = NextObject(&cstate)))
    {
      if(_isinobject(child, mb->mx, mb->my))
      {
        mailPart = (struct Part *)xget(child, MUIA_AttachmentImage_MailPart);

        break;
      }
    }
  }

  // generate a context menu title now
  if(mailPart)
  {
    snprintf(data->menuTitle, sizeof(data->menuTitle), GetStr(MSG_MA_MIMEPART_MENU), mailPart->Nr);
    data->selectedPart = mailPart;
  }
  else
  {
    strlcpy(data->menuTitle, GetStr(MSG_Attachments), sizeof(data->menuTitle));
    data->selectedPart = NULL;
  }

  data->context_menu = MenustripObject,
    Child, MenuObjectT(data->menuTitle),
      Child, MenuitemObject, MUIA_Menuitem_Title, GetStripStr(MSG_MA_ATTACHMENT_DISPLAY),  MUIA_Menuitem_Enabled, mailPart != NULL, MUIA_UserData, AMEN_DISPLAY,  End,
      Child, MenuitemObject, MUIA_Menuitem_Title, GetStripStr(MSG_MA_ATTACHMENT_SAVEAS),   MUIA_Menuitem_Enabled, mailPart != NULL, MUIA_UserData, AMEN_SAVEAS,    End,
      Child, MenuitemObject, MUIA_Menuitem_Title, GetStripStr(MSG_MA_ATTACHMENT_PRINT),   MUIA_Menuitem_Enabled, mailPart != NULL && mailPart->Printable, MUIA_UserData, AMEN_PRINT,     End,
      Child, MenuitemObject, MUIA_Menuitem_Title, NM_BARLABEL, End,
      Child, MenuitemObject, MUIA_Menuitem_Title, GetStripStr(MSG_MA_ATTACHMENT_SAVEALL), MUIA_UserData, AMEN_SAVEALL,   End,
      Child, MenuitemObject, MUIA_Menuitem_Title, GetStripStr(MSG_MA_ATTACHMENT_SAVESEL), MUIA_UserData, AMEN_SAVESEL,   End,
      Child, MenuitemObject, MUIA_Menuitem_Title, GetStripStr(MSG_MA_ATTACHMENT_CROPALL), MUIA_UserData, AMEN_CROPALL, End,
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

/* Private Functions */

/* Public Methods */
/// DECLARE(Clear)
DECLARE(Clear)
{
  GETDATA;
  struct List *childList = (struct List *)xget(obj, MUIA_Group_ChildList);

  ENTER();

  // iterate through our child list and remove all attachmentimages
  if(childList)
  {
    Object *cstate = (Object *)childList->lh_Head;
    Object *child;

    DoMethod(obj, MUIM_Group_InitChange);

    while((child = NextObject(&cstate)))
    {
      DoMethod(obj, OM_REMMEMBER, child);
      MUI_DisposeObject(child);
    }

    DoMethod(obj, MUIM_Group_ExitChange);
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
  struct Part *rp;

  ENTER();
  D(DBF_GUI, "%lx %ld/%ld", obj, _mwidth(obj), _mheight(obj));

  // before we are going to add some new childs we have to clean
  // out all old children
  DoMethod(obj, MUIM_AttachmentGroup_Clear);

  // prepare the group for any change
  DoMethod(obj, MUIM_Group_InitChange);

  // now we iterate through our message part list and
  // generate an own attachment image for each attachment
  for(rp = msg->firstPart; rp; rp = rp->Next)
  {
    if(rp->Nr > PART_RAW && rp->Nr != rp->rmData->letterPartNum)
    {
      Object *newImage = AttachmentImageObject,
                           MUIA_CycleChain,                 TRUE,
                           MUIA_AttachmentImage_MailPart,  rp,
                           MUIA_AttachmentImage_MaxHeight, _font(obj) ? TEXTROWS*_font(obj)->tf_YSize : 0,
                           MUIA_AttachmentImage_MaxWidth,   _font(obj) ? TEXTROWS*_font(obj)->tf_YSize : 0,
                         End;

      // connect some notifies which we might be interested in
      DoMethod(newImage, MUIM_Notify, MUIA_AttachmentImage_DoubleClick, TRUE,
               obj, 2, MUIM_AttachmentGroup_Display, rp);
      DoMethod(newImage, MUIM_Notify, MUIA_AttachmentImage_DropPath, MUIV_EveryTime,
               obj, 3, MUIM_AttachmentGroup_ImageDropped, newImage, MUIV_TriggerValue);

      DoMethod(obj, OM_ADDMEMBER, newImage);
      D(DBF_GUI, "added image obj %08lx for attachment: %ld:%s mp: %08lx %08lx %08lx %08lx", newImage, rp->Nr, rp->Name, rp, rp->ContentType, rp->headerList, rp->rmData);

      addedParts++;
    }
  }

  if(addedParts)
    data->firstPart = msg->firstPart;
  else
    data->firstPart = NULL;

  // signal that the group relayouting is finished
  DoMethod(obj, MUIM_Group_ExitChange);

  RETURN(addedParts);
  return addedParts;
}
///
/// DECLARE(Relayout)
DECLARE(Relayout)
{
  GETDATA;
  Object *parent = (Object *)xget(obj, MUIA_Parent);

  DoMethod(parent, MUIM_Group_InitChange);
  DoMethod(parent, OM_REMMEMBER, obj);
  data->resizePushed = FALSE;
  DoMethod(parent, OM_ADDMEMBER, obj);
  DoMethod(parent, MUIM_Group_ExitChange);

  return 0;
}
///
/// DECLARE(Display)
DECLARE(Display) // struct Part *part
{
  if(msg->part)
  {
    BusyText(GetStr(MSG_BusyDecDisplaying), "");

    RE_DecodePart(msg->part);
    RE_DisplayMIME(msg->part->Filename, msg->part->ContentType);

    BusyEnd();
  }

  return 0;
}
///
/// DECLARE(Save)
DECLARE(Save) // struct Part *part
{
  if(msg->part)
  {
    BusyText(GetStr(MSG_BusyDecSaving), "");

    RE_DecodePart(msg->part);
    RE_Export(msg->part->rmData,
              msg->part->Filename, "",
              msg->part->CParFileName ? msg->part->CParFileName : msg->part->Name,
              msg->part->Nr,
              FALSE,
              FALSE,
              msg->part->ContentType);

    BusyEnd();
  }

  return 0;
}
///
/// DECLARE(SaveAll)
DECLARE(SaveAll)
{
  GETDATA;
  struct Part *part = data->firstPart->Next;

  SHOWPOINTER(DBF_MAIL, part);

  if(part)
  {
    if(ReqFile(ASL_DETACH, _win(obj), GetStr(MSG_RE_SaveMessage), (REQF_SAVEMODE|REQF_DRAWERSONLY), C->DetachDir, ""))
    {
      BusyText(GetStr(MSG_BusyDecSaving), "");
      RE_SaveAll(part->rmData, G->ASLReq[ASL_DETACH]->fr_Drawer);
      BusyEnd();
    }
  }

  return 0;
}
///
/// DECLARE(SaveSelected)
DECLARE(SaveSelected)
{
  struct List *childList = (struct List *)xget(obj, MUIA_Group_ChildList);

  BusyText(GetStr(MSG_BusyDecSaving), "");

  // iterate through our child list and remove all attachmentimages
  if(childList)
  {
    Object *cstate = (Object *)childList->lh_Head;
    Object *child;

    while((child = NextObject(&cstate)))
    {
      if(xget(child, MUIA_Selected))
      {
        struct Part *mailPart = (struct Part *)xget(child, MUIA_AttachmentImage_MailPart);
        
        if(mailPart)
        {
          RE_DecodePart(mailPart);
          RE_Export(mailPart->rmData,
                    mailPart->Filename, "",
                    mailPart->CParFileName ? mailPart->CParFileName : mailPart->Name,
                    mailPart->Nr,
                    FALSE,
                    FALSE,
                    mailPart->ContentType);
        }
      }
    }
  }

  BusyEnd();

  return 0;
}
///
/// DECLARE(Print)
DECLARE(Print) // struct Part *part
{
  if(msg->part)
  {
    BusyText(GetStr(MSG_BusyDecPrinting), "");
    RE_PrintFile(msg->part->Filename);
    BusyEnd();
  }

  return 0;
}
///
/// DECLARE(CropAll)
DECLARE(CropAll)
{
  GETDATA;
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

  // make sure to refresh the mail of this window as we do not
  // have any attachments anymore
  if(rmData->readWindow)
    DoMethod(rmData->readWindow, MUIM_ReadWindow_ReadMail, mail);
  else
    DoMethod(rmData->readMailGroup, MUIM_ReadMailGroup_ReadMail, mail, MUIF_ReadMailGroup_ReadMail_UpdateTextOnly);

  return 0;
}
///
/// DECLARE(ImageDropped)
DECLARE(ImageDropped) // Object *imageObject, char *dropPath
{
  struct Part *mailPart = (struct Part *)xget(msg->imageObject, MUIA_AttachmentImage_MailPart);

  if(mailPart && msg->dropPath)
  {
    BOOL result;
    char *fileName;
    char filePathBuf[SIZE_PATHFILE];
    
    D(DBF_GUI, "Image of Part %d was dropped at [%s]", mailPart->Nr, msg->dropPath);

    BusyText(GetStr(MSG_BusyDecSaving), "");

    // make sure the drawer is opened upon the drag operation
    if(WorkbenchBase->lib_Version >= 44)
      OpenWorkbenchObjectA(msg->dropPath, NULL);

    // prepare the final path
    fileName = mailPart->CParFileName ? mailPart->CParFileName : mailPart->Name;
    strmfp(filePathBuf, msg->dropPath, fileName);

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
    if(result)
    {
      struct DiskObject *diskObject = (struct DiskObject *)xget(msg->imageObject, MUIA_AttachmentImage_DiskObject);

      // make sure to write out the diskObject of our attachment as well
      // but only if the filename doesn't end with a ".info" itself or it
      // clearly suggests that this might be a diskobject itself.
      if(diskObject)
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

          if((dlock = Lock(msg->dropPath, SHARED_LOCK)))
          {
            char *p;

            // strip an eventually existing extension
            if((p = strrchr(fileName, '.')))
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

  return 0;
}
///
