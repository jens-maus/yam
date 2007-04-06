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
 YAM OpenSource project     : http://sourceforge.net/projects/yamos/

 $Id$

 Superclass:  MUIC_Area
 Description: Custom class to display an image for a specific attachment

 Credits: This class was highly inspired by the similar attachment group &
          image functionality available in Thunderbird and SimpleMail. Large
          code portions where borrowed by the iconclass implementation of
          SimpleMail to allow loading of the default icons via icon.library
          and supporting Drag&Drop on the workbench. Thanks sba! :)

***************************************************************************/

#include "AttachmentImage_cl.h"

#include <workbench/icon.h>

#include <proto/graphics.h>
#include <proto/layers.h>
#include <proto/icon.h>
#include <proto/wb.h>

#include "Debug.h"

/* CLASSDATA
struct Data
{
  struct DiskObject *diskObject;
  struct Part *mailPart;

  struct BitMap *normalBitMap;
  struct BitMap *selectedBitMap;

  struct BitMap *normalBitMask;
  struct BitMap *selectedBitMask;

  char *dropPath;
  char shortHelp[SIZE_LARGE];

  struct MUI_EventHandlerNode ehnode;

  ULONG selectSecs;
  ULONG selectMicros;
  ULONG scaledWidth;
  ULONG scaledHeight;
  ULONG maxWidth;
  ULONG maxHeight;
};
*/

/// SelectionMsg
struct SelectionMsg
{
  struct Layer *l;
  LONG mx;
  LONG my;

  char *drawer;
  char *destName;
  
  BOOL finish;
};
///
/// SelectionHook
#if !defined(__amigaos4__)
HOOKPROTONO(SelectionFunc, ULONG, struct IconSelectMsg *ism)
{
  struct SelectionMsg *msg = (struct SelectionMsg *)hook->h_Data;
  struct Window *wnd = ism->ism_ParentWindow;

  if(!wnd)
    return ISMACTION_Stop;
  
  if(wnd->WLayer != msg->l)
    return ISMACTION_Stop;
  
  msg->finish = TRUE;

  if((ism->ism_Left + wnd->LeftEdge <= msg->mx) && (msg->mx <= wnd->LeftEdge + ism->ism_Left + ism->ism_Width - 1) &&
     (ism->ism_Top + wnd->TopEdge <= msg->my) && (msg->my <= wnd->TopEdge + ism->ism_Top + ism->ism_Height - 1))
  {
    if(ism->ism_Type == WBDRAWER)
    {
      msg->destName = strdup(ism->ism_Name);
      return ISMACTION_Select;
    }
    else if(ism->ism_Type == WBDISK)
    {
      msg->destName = malloc(strlen(ism->ism_Name)+2);
      if(msg->destName)
      {
        snprintf(msg->destName, strlen(ism->ism_Name)+2, "%s:", ism->ism_Name);
        return ISMACTION_Select;
      }
    }

    return ISMACTION_Stop;
  }

  return ISMACTION_Ignore;
}
MakeStaticHook(SelectionHook, SelectionFunc);
#endif
///

/* Overloaded Methods */
/// OVERLOAD(OM_NEW)
OVERLOAD(OM_NEW)
{
  ENTER();

  if((obj = DoSuperNew(cl, obj,
      MUIA_FillArea,    FALSE, // do not care about background filling
    TAG_MORE, inittags(msg))))
  {
    GETDATA;
    struct TagItem *tags = inittags(msg);
    struct TagItem *tag;

    while((tag = NextTagItem(&tags)))
    {
      switch(tag->ti_Tag)
      {
        ATTR(MailPart)  : data->mailPart = (struct Part *)tag->ti_Data; break;
        ATTR(MaxHeight) : data->maxHeight = (ULONG)tag->ti_Data; break;
        ATTR(MaxWidth)  : data->maxWidth  = (ULONG)tag->ti_Data; break;
      }
    }

    if(data->mailPart)
    {
      struct Part *mp = data->mailPart;
      char sizestr[SIZE_DEFAULT];

      FormatSize(mp->Size, sizestr, sizeof(sizestr), SF_AUTO);

      snprintf(data->shortHelp, sizeof(data->shortHelp), tr(MSG_MA_MIMEPART_INFO), mp->Nr,
                                                                                       mp->Name,
                                                                                       mp->Description,
                                                                                       DescribeCT(mp->ContentType),
                                                                                       mp->ContentType,
                                                                                       sizestr);
      set(obj, MUIA_ShortHelp, data->shortHelp);
    }
  }

  RETURN((ULONG)obj);
  return (ULONG)obj;
}
///
/// OVERLOAD(OM_DISPOSE)
OVERLOAD(OM_DISPOSE)
{
  GETDATA;

  if(data->dropPath)
  {
    free(data->dropPath);
    data->dropPath = NULL;
  }

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
    ATTR(DoubleClick) : *store = 1; return TRUE;
    ATTR(DropPath)    : *store = (ULONG)data->dropPath;   return TRUE;
    ATTR(MailPart)    : *store = (ULONG)data->mailPart;   return TRUE;
    ATTR(DiskObject)  : *store = (ULONG)data->diskObject; return TRUE;
  }

  return DoSuperMethodA(cl, obj, msg);
}
///
/// OVERLOAD(MUIM_Setup)
OVERLOAD(MUIM_Setup)
{
  GETDATA;
  struct DiskObject *diskObject = NULL;
  struct Part *mailPart = data->mailPart;
  ULONG result = 0;

  ENTER();

  // call the supermethod of the supercall first
  if(DoSuperMethodA(cl, obj, msg) && mailPart != NULL)
  {
    // only if we have at least icon.library >= v44 and we find deficons
    // we try to identify the file with deficons
    if(mailPart->Decoded && mailPart->Filename[0] != '\0' &&
       IconBase->lib_Version >= 44 &&  FindPort("DEFICONS"))
    {
      D(DBF_GUI, "retrieving diskicon via DEFICONS for '%s'", mailPart->Filename);

      diskObject = GetIconTags(mailPart->Filename,
                               ICONGETA_FailIfUnavailable, FALSE,
                               ICONGETA_Screen,            _screen(obj),
                               TAG_DONE);

      #if defined(DEBUG)
      if(diskObject == NULL)
        W(DBF_GUI, "wasn't able to retrieve diskObject via DEFICONS: %d", IoErr());
      #endif
    }

    // if we have still not obtained the diskObject we go
    // and load a default icon for a specific ContentType
    if(!diskObject)
    {
      // with icon.library v44+ we can use GetIconTags again.
      if(IconBase->lib_Version >= 44 && mailPart->ContentType)
      {
        const char *def;

        // build the defaultname now
        if(!strnicmp(mailPart->ContentType, "image", 5))
          def = "picture";
        else if(!strnicmp(mailPart->ContentType, "audio", 5))
          def = "audio";
        else if(!strnicmp(mailPart->ContentType, "text", 4))
        {
          if(strlen(mailPart->ContentType) > 5)
          {
            if(!strnicmp((mailPart->ContentType)+5, "html", 4))
              def = "html";
            else if(!strnicmp((mailPart->ContentType)+5, "plain", 5))
              def = "ascii";
            else
              def = "text";
          }
          else
            def = "text";
        }
        else
          def = "attach";

        // try to retrieve the icon for that type
        diskObject = GetIconTags(NULL,
                                 ICONGETA_GetDefaultName, def,
                                 ICONGETA_Screen,         _screen(obj),
                                 TAG_DONE);

        // if we still have not retrieved any icon we
        // obtain the standard project icon
        if(!diskObject)
        {
          diskObject = GetIconTags(NULL,
                                   ICONGETA_GetDefaultType, WBPROJECT,
                                   ICONGETA_Screen,         _screen(obj),
                                   TAG_DONE);
        }
      }
      else
      {
        // on an old OS <= 3.1 we can only use a project icon for
        // the attachment
        diskObject = GetDefDiskObject(WBPROJECT);
      }
    }
    else
    {
      D(DBF_GUI, "DiskObject for file [%s] retrieved with DefIcons", mailPart->Filename);
    }

    // now that we should have the diskObject we get the image of it, blit it in
    // a temporary rastport so that we scan scale it down
    if(diskObject)
    {
      struct BitMap *orgBitMap;
      struct BitMap *screenBitMap = _screen(obj)->RastPort.BitMap;
      PLANEPTR normalBitMask;
      PLANEPTR selectedBitMask;
      struct RastPort rp;
      ULONG orgWidth;
      ULONG orgHeight;
      ULONG screenDepth = GetBitMapAttr(screenBitMap, BMA_DEPTH);

      // prepare the drawIcon/GetIconRentagle tags
      #ifdef ICONDRAWA_Transparency // defined starting from icon.lib v51+
      static const struct TagItem drawIconTags[] = { { ICONDRAWA_Borderless,      TRUE  },
                                                     { ICONDRAWA_Frameless,       TRUE  },
                                                     { ICONDRAWA_Transparency,    255   },
                                                     { TAG_DONE,                  FALSE } };
      #else
      static const struct TagItem drawIconTags[] = { { ICONDRAWA_Borderless,      TRUE  },
                                                     { ICONDRAWA_Frameless,       TRUE  },
                                                     { TAG_DONE,                  FALSE } };
      #endif

      // initialize our temporary rastport
      InitRastPort(&rp);

      // get some information about our diskObject like width/height
      // and the bitmask for transparency drawing
      if(IconBase->lib_Version >= 44)
      {
        struct Rectangle rect;
        struct TagItem iconCtrlTags[] = { { ICONCTRLA_GetImageMask1, (ULONG)NULL },
                                          { ICONCTRLA_GetImageMask2, (ULONG)NULL },
                                          { TAG_DONE,                FALSE       } };
    
        iconCtrlTags[0].ti_Data = (ULONG)&normalBitMask;
        iconCtrlTags[1].ti_Data = (ULONG)&selectedBitMask;

        GetIconRectangleA(NULL, diskObject, NULL, &rect, drawIconTags);

        orgWidth  = rect.MaxX - rect.MinX + 1;
        orgHeight = rect.MaxY - rect.MinY + 1;

        // query the bitmask
        IconControlA(diskObject, iconCtrlTags);
      }
      else
      {
        orgWidth  = ((struct Image*)diskObject->do_Gadget.GadgetRender)->Width + 1;
        orgHeight = ((struct Image*)diskObject->do_Gadget.GadgetRender)->Height + 1;

        normalBitMask = NULL;
        selectedBitMask = NULL;
      }

      // we first allocate a source bitmap with equal size to the icon size of the diskObject
      orgBitMap = AllocBitMap(orgWidth, orgHeight, screenDepth, 0, screenBitMap);
      if(orgBitMap)
      {
        LONG scaleHeightDiff = orgHeight - data->maxHeight;
        LONG scaleWidthDiff  = orgHeight - data->maxWidth;
        LONG newWidth;
        LONG newHeight;

        // prepare the rastport for drawing the icon in it
        rp.BitMap = orgBitMap;

        if(IconBase->lib_Version >= 44)
          DrawIconStateA(&rp, diskObject, NULL, 0, 0, IDS_SELECTED, drawIconTags);
        else
        {
          if(diskObject->do_Gadget.Flags & GFLG_GADGHIMAGE)
            DrawImage(&rp, ((struct Image*)diskObject->do_Gadget.SelectRender), 0, 0);
          else
            DrawImage(&rp, ((struct Image*)diskObject->do_Gadget.GadgetRender), 0, 0);
        }

        // calculate the scale factors now that we have fillup our source bitmap
        if((scaleHeightDiff > 0 && data->maxHeight > 0) ||
           (scaleWidthDiff > 0 && data->maxWidth > 0))
        {
          double scaleFactor;

          // make sure we are scaling proportional
          if(scaleHeightDiff > scaleWidthDiff)
          {
            scaleFactor = (double)orgWidth / (double)orgHeight;
            newWidth = scaleFactor * data->maxHeight + 0.5; // roundup the value
            newHeight = data->maxHeight;
          }
          else
          {
            scaleFactor = (double)orgHeight / (double)orgWidth;
            newWidth = data->maxWidth;
            newHeight = scaleFactor * data->maxWidth + 0.5; // roundup the value
          }
        }
        else
        {
          newWidth  = orgWidth;
          newHeight = orgHeight;
        }

        // now we can allocate a new bitmap which should carry the scaled selected image
        data->selectedBitMap = AllocBitMap(newWidth, newHeight, screenDepth, 0, orgBitMap);
        if(data->selectedBitMap)
        {
          struct BitScaleArgs args;

          args.bsa_SrcBitMap = orgBitMap;
          args.bsa_DestBitMap = data->selectedBitMap;
          args.bsa_Flags = 0;

          args.bsa_SrcY = 0;
          args.bsa_DestY = 0;

          args.bsa_SrcWidth = orgWidth;
          args.bsa_SrcHeight = orgHeight;

          args.bsa_XSrcFactor = orgWidth;
          args.bsa_XDestFactor = newWidth;

          args.bsa_YSrcFactor = orgHeight;
          args.bsa_YDestFactor = newHeight;

          args.bsa_SrcX = 0;
          args.bsa_DestX = 0;

          // scale the image now with the arguments set
          BitMapScale(&args);

          // read out the scaled values
          data->scaledWidth  = args.bsa_DestWidth;
          data->scaledHeight = args.bsa_DestHeight;

          D(DBF_GUI, "AttachmentImage selected scale (w/h) from %ld/%ld to %ld/%ld", orgWidth,
                                                                                     orgHeight,
                                                                                     data->scaledWidth,
                                                                                     data->scaledHeight);
        }

        // now we also scale the selected BitMask down, if it exists
        if(selectedBitMask &&
           (data->selectedBitMask = AllocBitMap(newWidth, newHeight, 1L, 0, NULL)))
        {
          struct BitScaleArgs args;
          struct BitMap bm;

          InitBitMap(&bm, 1L, orgWidth, orgHeight);
          bm.Planes[0] = selectedBitMask;

          args.bsa_SrcBitMap = &bm;
          args.bsa_DestBitMap = data->selectedBitMask;
          args.bsa_Flags = 0;

          args.bsa_SrcY = 0;
          args.bsa_DestY = 0;

          args.bsa_SrcWidth = orgWidth;
          args.bsa_SrcHeight = orgHeight;

          args.bsa_XSrcFactor = orgWidth;
          args.bsa_XDestFactor = newWidth;

          args.bsa_YSrcFactor = orgHeight;
          args.bsa_YDestFactor = newHeight;

          args.bsa_SrcX = 0;
          args.bsa_DestX = 0;

          // scale the image now with the arguments set
          BitMapScale(&args);
        }
        else
          data->selectedBitMask = NULL;

        // now that we have the selectedBitMap filled we have to scale down the unselected state
        // of the icon as well.
        if(IconBase->lib_Version >= 44)
          DrawIconStateA(&rp, diskObject, NULL, 0, 0, IDS_NORMAL, drawIconTags);
        else
          DrawImage(&rp, ((struct Image*)diskObject->do_Gadget.GadgetRender), 0, 0);

        // now we can allocate a new bitmap which should carry the scaled unselected normal image
        data->normalBitMap = AllocBitMap(newWidth, newHeight, screenDepth, 0, orgBitMap);
        if(data->normalBitMap)
        {
          struct BitScaleArgs args;

          args.bsa_SrcBitMap = orgBitMap;
          args.bsa_DestBitMap = data->normalBitMap;
          args.bsa_Flags = 0;

          args.bsa_SrcY = 0;
          args.bsa_DestY = 0;

          args.bsa_SrcWidth = orgWidth;
          args.bsa_SrcHeight = orgHeight;

          args.bsa_XSrcFactor = orgWidth;
          args.bsa_XDestFactor = newWidth;

          args.bsa_YSrcFactor = orgHeight;
          args.bsa_YDestFactor = newHeight;

          args.bsa_SrcX = 0;
          args.bsa_DestX = 0;

          // scale the image now with the arguments set
          BitMapScale(&args);

          // read out the scaled values
          data->scaledWidth  = args.bsa_DestWidth;
          data->scaledHeight = args.bsa_DestHeight;

          D(DBF_GUI, "AttachmentImage normal scale (w/h) from %ld/%ld to %ld/%ld", orgWidth,
                                                                                   orgHeight,
                                                                                   data->scaledWidth,
                                                                                   data->scaledHeight);
        }

        // now we also scale the normal BitMask down, if it exists
        if(normalBitMask &&
           (data->normalBitMask = AllocBitMap(newWidth, newHeight, 1L, 0, NULL)))
        {
          struct BitScaleArgs args;
          struct BitMap bm;

          InitBitMap(&bm, 1L, orgWidth, orgHeight);
          bm.Planes[0] = normalBitMask;

          args.bsa_SrcBitMap = &bm;
          args.bsa_DestBitMap = data->normalBitMask;
          args.bsa_Flags = 0;

          args.bsa_SrcY = 0;
          args.bsa_DestY = 0;

          args.bsa_SrcWidth = orgWidth;
          args.bsa_SrcHeight = orgHeight;

          args.bsa_XSrcFactor = orgWidth;
          args.bsa_XDestFactor = newWidth;

          args.bsa_YSrcFactor = orgHeight;
          args.bsa_YDestFactor = newHeight;

          args.bsa_SrcX = 0;
          args.bsa_DestX = 0;

          // scale the image now with the arguments set
          BitMapScale(&args);
        }
        else
          data->normalBitMask = NULL;

        FreeBitMap(orgBitMap);
      }
    }

    // store the diskObject in our instance data for later
    // reference
    data->diskObject = diskObject;

    // add an event handler for the drag&drop operations
    // this object supports.
    data->ehnode.ehn_Priority = -1;
    data->ehnode.ehn_Flags    = 0;
    data->ehnode.ehn_Object   = obj;
    data->ehnode.ehn_Class    = cl;
    data->ehnode.ehn_Events   = IDCMP_MOUSEBUTTONS | IDCMP_RAWKEY;

    DoMethod(_win(obj), MUIM_Window_AddEventHandler, &data->ehnode);

    result = 1;
  }

  RETURN(result);
  return result;
}
///
/// OVERLOAD(MUIM_Cleanup)
OVERLOAD(MUIM_Cleanup)
{
  GETDATA;

  ENTER();

  // remove the eventhandler first
  DoMethod(_win(obj), MUIM_Window_RemEventHandler, &data->ehnode);

  if(data->normalBitMap)
  {
    FreeBitMap(data->normalBitMap);
    data->normalBitMap = NULL;
  }

  if(data->normalBitMask)
  {
    FreeBitMap(data->normalBitMask);
    data->normalBitMask = NULL;
  }

  if(data->selectedBitMap)
  {
    FreeBitMap(data->selectedBitMap);
    data->selectedBitMap = NULL;
  }

  if(data->selectedBitMask)
  {
    FreeBitMap(data->selectedBitMask);
    data->selectedBitMask = NULL;
  }

  if(data->diskObject)
  {
    FreeDiskObject(data->diskObject);
    data->diskObject = NULL;
  }

  DoSuperMethodA(cl, obj, msg);
  
  RETURN(0);
  return 0;
}
///
/// OVERLOAD(MUIM_AskMinMax)
OVERLOAD(MUIM_AskMinMax)
{
  GETDATA;
  struct MUI_MinMax *mi;
  
  // call the supermethod first
  DoSuperMethodA(cl, obj, msg);

  mi = ((struct MUIP_AskMinMax *)msg)->MinMaxInfo;

  mi->MinWidth += data->scaledWidth;
  mi->DefWidth += data->scaledWidth;
  mi->MaxWidth += data->scaledWidth;

  mi->MinHeight += data->scaledHeight;
  mi->DefHeight += data->scaledHeight;
  mi->MaxHeight += data->scaledHeight;
  
  return 0;
}
///
/// OVERLOAD(MUIM_Draw)
OVERLOAD(MUIM_Draw)
{
  GETDATA;

  // call the super method first
  DoSuperMethodA(cl, obj, msg);

  if(((struct MUIP_Draw *)msg)->flags & MADF_DRAWOBJECT)
  {
    struct BitMap *bitmap;
    struct BitMap *bitmask;

    // check the selected state
    if(xget(obj, MUIA_Selected))
    {
      bitmap = data->selectedBitMap;
      bitmask = data->selectedBitMask;
    }
    else
    {
      bitmap = data->normalBitMap;
      bitmask = data->normalBitMask;
    }

    // draw the background first.
    DoMethod(obj, MUIM_DrawBackground, _mleft(obj), _mtop(obj), _mwidth(obj), _mheight(obj), 0, 0, MUIF_NONE);

    if(bitmask)
    {
      // we use an own BltMaskBitMapRastPort() implemenation to also support
      // interleaved images.
      MyBltMaskBitMapRastPort(bitmap, 0, 0, _rp(obj), _mleft(obj), _mtop(obj), _mwidth(obj), _mheight(obj), 0xc0, bitmask->Planes[0]);
    }
    else
    {
      BltBitMapRastPort(bitmap, 0, 0, _rp(obj), _mleft(obj), _mtop(obj), _mwidth(obj), _mheight(obj), 0xc0);
    }
  }

  return 0;
}
///
/// OVERLOAD(MUIM_HandleEvent)
OVERLOAD(MUIM_HandleEvent)
{
  GETDATA;
  struct IntuiMessage *imsg = ((struct MUIP_HandleEvent *)msg)->imsg;

  ENTER();

  if(!imsg)
  {
    RETURN(0);
    return 0;
  }
  
  if(imsg->Class == IDCMP_MOUSEBUTTONS)
  {
    if(!(_isinobject(obj, imsg->MouseX, imsg->MouseY)))
    {
      data->selectSecs = 0;
      data->selectMicros = 0;

      RETURN(0);
      return 0;
    }

    // in case the image is selected
    if(imsg->Code == SELECTDOWN)
    {
      // check if this has been a double click at the image
      if(DoubleClick(data->selectSecs, data->selectMicros, imsg->Seconds, imsg->Micros))
      {
        set(obj, MUIA_AttachmentImage_DoubleClick, TRUE);
        set(obj, MUIA_Selected, TRUE);
      }
      else
      {
        Object *parent = (Object *)xget(obj, MUIA_Parent);

        // only clear the selection if the user hasn't used
        // the SHIFT key to select multiple items.
        if(parent && hasFlag(imsg->Qualifier, IEQUALIFIER_RSHIFT|IEQUALIFIER_LSHIFT) == FALSE)
          DoMethod(parent, MUIM_AttachmentGroup_ClearSelection);

        // invert the selection state
        set(obj, MUIA_Selected, !xget(obj, MUIA_Selected));
      }

      // save the seconds/micros for the next handleEvent call
      if(xget(obj, MUIA_Selected) == TRUE)
      {
        data->selectSecs  = imsg->Seconds;
        data->selectMicros= imsg->Micros;
      }
      else
      {
        data->selectSecs  = 0;
        data->selectMicros= 0;
      }

      if(WorkbenchBase->lib_Version >= 45)
      {
        DoMethod(_win(obj), MUIM_Window_RemEventHandler, &data->ehnode);
        data->ehnode.ehn_Events |= IDCMP_MOUSEMOVE;
        DoMethod(_win(obj), MUIM_Window_AddEventHandler, &data->ehnode);
      }

      RETURN(MUI_EventHandlerRC_Eat);
      return MUI_EventHandlerRC_Eat;
    }

    // in case the image is unselected by the user
    if(imsg->Code == SELECTUP)
    {
      if(WorkbenchBase->lib_Version >= 45)
      {
        DoMethod(_win(obj), MUIM_Window_RemEventHandler, &data->ehnode);
        data->ehnode.ehn_Events &= ~IDCMP_MOUSEMOVE;
        DoMethod(_win(obj), MUIM_Window_AddEventHandler, &data->ehnode);
      }

      RETURN(MUI_EventHandlerRC_Eat);
      return MUI_EventHandlerRC_Eat;
    }

  }

  // in case this event is a mouse move we signal a dragging event, but only
  // if it starts within our object region.
  if(imsg->Class == IDCMP_MOUSEMOVE &&
     _isinobject(obj, imsg->MouseX, imsg->MouseY))
  {
    DoMethod(obj, MUIM_DoDrag, imsg->MouseX - _mleft(obj), imsg->MouseY - _mtop(obj));
  }

  if(imsg->Class == IDCMP_RAWKEY)
  {
    switch(imsg->Code)
    {
      case IECODE_RETURN:
      {
        if(obj == (Object *)xget(_win(obj), MUIA_Window_ActiveObject))
        {
          set(obj, MUIA_AttachmentImage_DoubleClick, TRUE);

          RETURN(MUI_EventHandlerRC_Eat);
          return MUI_EventHandlerRC_Eat;
        }
      }
      break;

      case IECODE_SPACE:
      {
        if(obj == (Object *)xget(_win(obj), MUIA_Window_ActiveObject))
        {
          Object *parent = (Object *)xget(obj, MUIA_Parent);

          // only clear the selection if the user hasn't used
          // the SHIFT key to select multiple items.
          if(parent && hasFlag(imsg->Qualifier, IEQUALIFIER_RSHIFT|IEQUALIFIER_LSHIFT) == FALSE)
            DoMethod(parent, MUIM_AttachmentGroup_ClearSelection);

          set(obj, MUIA_Selected, !xget(obj, MUIA_Selected));

          RETURN(MUI_EventHandlerRC_Eat);
          return MUI_EventHandlerRC_Eat;
        }
      }
      break;
    }
  }

  RETURN(0);
  return 0;
}
///
/// OVERLOAD(MUIM_DeleteDragImage)
OVERLOAD(MUIM_DeleteDragImage)
{
  GETDATA;

#if defined(__amigaos4__)
  ULONG which;
  char *buf;
  struct Screen *wbscreen = LockPubScreen("Workbench");

  if(data->dropPath)
  {
    free(data->dropPath);
    data->dropPath = NULL;
  }

  // now we check whether YAM is running on the workbench screen or
  // not, because otherwise we skip our further operations.
  if(wbscreen)
  {
    if(wbscreen == _screen(obj) && (buf = (STRPTR)malloc(SIZE_PATHFILE)))
    {
      char name[256];
      ULONG type = ~0;

      struct TagItem ti[] = { { WBOBJA_DrawerPath,      (ULONG)buf          },
                              { WBOBJA_DrawerPathSize,  SIZE_PATHFILE       },
                              { WBOBJA_Name,            (ULONG)name         },
                              { WBOBJA_NameSize,        (ULONG)sizeof(name) },
                              { WBOBJA_Type,            (ULONG)&type        },
                              { TAG_DONE,               FALSE               } };

      buf[0] = '\0';

      // Note that we use WhichWorkbenchObjectA() and not WhichWorkbenchObject()
      // because the latter wasn't implemented in workbench.library < 51.9
      which = WhichWorkbenchObjectA(NULL, _screen(obj)->MouseX, _screen(obj)->MouseY, ti);
      if(which == WBO_ICON)
      {
        if(type == WBDRAWER)
          AddPart(buf, name, SIZE_PATHFILE);
        else if(type == WBDISK)
          snprintf(buf, SIZE_PATHFILE, "%s:", name);
      
        which = WBO_DRAWER;
      }
    
      if(which == WBO_DRAWER && buf[0] != '\0')
      {
        if((data->dropPath = strdup(buf)))
        {
          D(DBF_GUI, "found dropPath: [%s]", data->dropPath);
          
          DoMethod(_app(obj), MUIM_Application_PushMethod, obj, 3, MUIM_Set, MUIA_AttachmentImage_DropPath, data->dropPath);
        }
      }
      else
      {
        W(DBF_GUI, "couldn't found drop point of attachment image");
        DisplayBeep(_screen(obj));
      }

      free(buf);
    }
    else
      W(DBF_GUI, "YAM is not running on workbench, skipping drop operation");

    UnlockPubScreen(NULL, wbscreen);
  }

#else
  // this stuff only works with Workbench v45+
  if(WorkbenchBase->lib_Version >= 45)
  {
    struct Layer *l = WhichLayer(&_screen(obj)->LayerInfo, _screen(obj)->MouseX, _screen(obj)->MouseY);
    if(l)
    {
      struct List *path_list;
      if(WorkbenchControl(NULL, WBCTRLA_GetOpenDrawerList, &path_list, TAG_DONE))
      {
        struct Hook hook;
        struct SelectionMsg selMsg;
        struct Node *n;

        selMsg.l = l;
        selMsg.mx = _screen(obj)->MouseX;
        selMsg.my = _screen(obj)->MouseY;
        selMsg.destName = NULL;
        selMsg.finish = FALSE;

        // initialise the selection hook with our data
        InitHook(&hook, SelectionHook, &selMsg);

        if(data->dropPath)
        {
          free(data->dropPath);
          data->dropPath = NULL;
        }

        for(n = path_list->lh_Head; n->ln_Succ; n = n->ln_Succ)
        {
          selMsg.drawer = strdup(n->ln_Name);
          if(selMsg.drawer)
          {
            ChangeWorkbenchSelectionA(selMsg.drawer, &hook, NULL);

            if(selMsg.finish)
            {
              if(!selMsg.destName)
              {
                data->dropPath = selMsg.drawer;
                selMsg.drawer = NULL;
              }
              else
              {
                int len = strlen(selMsg.destName) + strlen(selMsg.drawer) + 10;

                data->dropPath = malloc(len);
                if(data->dropPath)
                {
                  strlcpy(data->dropPath, selMsg.drawer, len);
                  AddPart(data->dropPath, selMsg.destName, len);
                }

                free(selMsg.destName);
              }
            }
            
            if(selMsg.drawer)
              free(selMsg.drawer);
            
            if(selMsg.finish)
              break;
          }
        }

        WorkbenchControl(NULL, WBCTRLA_FreeOpenDrawerList, path_list, TAG_DONE);

        if(!selMsg.finish)
        {
          selMsg.drawer = NULL;
          
          ChangeWorkbenchSelectionA(NULL, &hook, NULL);
          
          if(selMsg.finish && selMsg.destName)
            data->dropPath = selMsg.destName;
        }

        // signal other listening for the DropPath that we
        // found out where to icon has dropped at exactly.
        if(data->dropPath)
        {
          D(DBF_GUI, "found dropPath: [%s]", data->dropPath);
          DoMethod(_app(obj), MUIM_Application_PushMethod, obj, 3, MUIM_Set, MUIA_AttachmentImage_DropPath, data->dropPath);
        }
        else
        {
          W(DBF_GUI, "couldn't found drop point of attachment image");
          DisplayBeep(_screen(obj));
        }
      }
    }
  }
#endif

  DoMethod(_win(obj), MUIM_Window_RemEventHandler, &data->ehnode);
  data->ehnode.ehn_Events &= ~IDCMP_MOUSEMOVE;
  DoMethod(_win(obj), MUIM_Window_AddEventHandler, &data->ehnode);

  return DoSuperMethodA(cl, obj, msg);

}
///

/* Private Functions */

/* Public Methods */
