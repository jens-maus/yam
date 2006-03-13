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
      msg->destName = malloc(strlen(ism->ism_Name)+1);
      if(msg->destName)
        strcpy(msg->destName, ism->ism_Name);

      return ISMACTION_Select;
    }
    else if(ism->ism_Type == WBDISK)
    {
      msg->destName = malloc(strlen(ism->ism_Name)+2);
      if(msg->destName)
      {
        strcpy(msg->destName, ism->ism_Name);
        strcat(msg->destName, ":");

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
        ATTR(MailPart)     : data->mailPart = (struct Part *)tag->ti_Data; break;
        ATTR(MaxHeight)   : data->maxHeight = (ULONG)tag->ti_Data; break;
        ATTR(MaxWidth)    : data->maxWidth  = (ULONG)tag->ti_Data; break;
      }
    }

    if(data->mailPart)
    {
      struct Part *mp = data->mailPart;

      SPrintF(data->shortHelp, GetStr(MSG_MA_ATTACHMENTINFO), mp->Nr,
                                                              mp->Name,
                                                              mp->Description,
                                                              DescribeCT(mp->ContentType),
                                                              mp->ContentType,
                                                              mp->Size);
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

  if(!DoSuperMethodA(cl, obj, msg) || mailPart == NULL)
    return 0;

  // only if we have at least icon.library >= v44 and we find deficons
  // we try to identify the file with deficons
  if(mailPart->Decoded && mailPart->Filename[0] != '\0' &&
     IconBase->lib_Version >= 44 &&  FindPort("DEFICONS"))
  {
    diskObject = GetIconTags(mailPart->Filename,
                             ICONGETA_FailIfUnavailable, FALSE,
                             ICONGETA_Screen,            _screen(obj),
                             TAG_DONE);
  }

  // if we have still not obtained the diskObject we go
  // and load a default icon for a specific ContentType
  if(!diskObject)
  {
    // with icon.library v44+ we can use GetIconTags again.
    if(IconBase->lib_Version >= 44 && mailPart->ContentType)
    {
      char *def;

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
    struct RastPort rp;
    ULONG orgWidth;
    ULONG orgHeight;
    ULONG screenDepth = GetBitMapAttr(screenBitMap, BMA_DEPTH);

    // prepare the drawIcon/GetIconRentagle tags
    #ifdef ICONDRAWA_Transparency // defined starting from icon.lib v51+
    static const struct TagItem drawIconTags[] = { { ICONDRAWA_Borderless,    TRUE  },
                                                   { ICONDRAWA_Frameless,      TRUE  },
                                                   { ICONDRAWA_Transparency, 255   },
                                                   { TAG_DONE,                FALSE } };
    #else
    static const struct TagItem drawIconTags[] = { { ICONDRAWA_Borderless, TRUE  },
                                                   { ICONDRAWA_Frameless,   TRUE  },
                                                   { TAG_DONE,              FALSE } };
    #endif

    // initialize our temporary rastport
    InitRastPort(&rp);

    // get some information about our diskObject like width/height
    if(IconBase->lib_Version >= 44)
    {
      struct Rectangle rect;
  
      GetIconRectangleA(NULL, diskObject, NULL, &rect, drawIconTags);

      orgWidth  = rect.MaxX - rect.MinX + 1;
      orgHeight = rect.MaxY - rect.MinY + 1;
    }
    else
    {
      orgWidth  = ((struct Image*)diskObject->do_Gadget.GadgetRender)->Width + 1;
      orgHeight = ((struct Image*)diskObject->do_Gadget.GadgetRender)->Height + 1;
    }

    // we first allocate a source bitmap with equal size to the icon size of the diskObject
    orgBitMap = AllocBitMap(orgWidth, orgHeight, screenDepth, BMF_CLEAR|BMF_MINPLANES, screenBitMap);
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
      data->selectedBitMap = AllocBitMap(newWidth, newHeight, screenDepth, BMF_CLEAR|BMF_MINPLANES, orgBitMap);
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

      // now that we have the selectedBitMap filled we have to scale down the unselected state
      // of the icon as well.
      if(IconBase->lib_Version >= 44)
        DrawIconStateA(&rp, diskObject, NULL, 0, 0, IDS_NORMAL, drawIconTags);
      else
        DrawImage(&rp, ((struct Image*)diskObject->do_Gadget.GadgetRender), 0, 0);

      // now we can allocate a new bitmap which should carry the scaled unselected normal image
      data->normalBitMap = AllocBitMap(newWidth, newHeight, screenDepth, BMF_CLEAR|BMF_MINPLANES, orgBitMap);
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

  return 1;

}
///
/// OVERLOAD(MUIM_Cleanup)
OVERLOAD(MUIM_Cleanup)
{
  GETDATA;

  // remove the eventhandler first
  DoMethod(_win(obj), MUIM_Window_RemEventHandler, &data->ehnode);

  if(data->normalBitMap)
  {
    FreeBitMap(data->normalBitMap);
    data->normalBitMap = NULL;
  }

  if(data->selectedBitMap)
  {
    FreeBitMap(data->selectedBitMap);
    data->selectedBitMap = NULL;
  }

  if(data->diskObject)
  {
    FreeDiskObject(data->diskObject);
    data->diskObject = NULL;
  }

  DoSuperMethodA(cl, obj, msg);
  
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
    BOOL sel = xget(obj, MUIA_Selected);

    if(sel && data->selectedBitMap)
      BltBitMapRastPort(data->selectedBitMap, 0, 0, _rp(obj), _mleft(obj), _mtop(obj), _mwidth(obj), _mheight(obj), 0xc0);
    else if(data->normalBitMap)
      BltBitMapRastPort(data->normalBitMap, 0, 0, _rp(obj), _mleft(obj), _mtop(obj), _mwidth(obj), _mheight(obj), 0xc0);
  }

  return 0;
}
///
/// OVERLOAD(MUIM_HandleEvent)
OVERLOAD(MUIM_HandleEvent)
{
  GETDATA;
  struct IntuiMessage *imsg = ((struct MUIP_HandleEvent *)msg)->imsg;

  if(!imsg)
    return 0;
  
  if(imsg->Class == IDCMP_MOUSEBUTTONS)
  {
    if(!(_isinobject(obj, imsg->MouseX, imsg->MouseY)))
    {
      data->selectSecs = 0;
      data->selectMicros = 0;

      return 0;
    }

    // in case the image is selected
    if(imsg->Code == SELECTDOWN)
    {
      ULONG secs;
      ULONG micros;

      // get the currentTime
      CurrentTime(&secs, &micros);

      // check if this has been a double click at the image
      if(DoubleClick(data->selectSecs, data->selectMicros, secs, micros))
      {
        set(obj, MUIA_AttachmentImage_DoubleClick, TRUE);
        set(obj, MUIA_Selected, TRUE);
      }
      else
        set(obj, MUIA_Selected, !xget(obj, MUIA_Selected)); // toggle selection

      // save the seconds/micros for the next handleEvent call
      data->selectSecs   = secs;
      data->selectMicros= micros;

      if(WorkbenchBase->lib_Version >= 45)
      {
        DoMethod(_win(obj), MUIM_Window_RemEventHandler, &data->ehnode);
        data->ehnode.ehn_Events |= IDCMP_MOUSEMOVE;
        DoMethod(_win(obj), MUIM_Window_AddEventHandler, &data->ehnode);
      }

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

          return MUI_EventHandlerRC_Eat;
        }
      }
      break;

      case IECODE_SPACE:
      {
        if(obj == (Object *)xget(_win(obj), MUIA_Window_ActiveObject))
        {
          set(obj, MUIA_Selected, !xget(obj, MUIA_Selected));

          return MUI_EventHandlerRC_Eat;
        }
      }
      break;
    }
  }

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

  if(data->dropPath)
  {
    free(data->dropPath);
    data->dropPath = NULL;
  }

  if((buf = (STRPTR)malloc(2048)))
  {
    char name[256];
    ULONG type = ~0;

    struct TagItem ti[] = { { WBOBJA_DrawerPath,      (ULONG)buf          },
                            { WBOBJA_DrawerPathSize, 2048                 },
                            { WBOBJA_Name,            (ULONG)name          },
                            { WBOBJA_NameSize,        (ULONG)sizeof(name) },
                            { WBOBJA_Type,            (ULONG)&type         },
                            { TAG_DONE,               FALSE                } };

    buf[0] = '\0';

    // Note that we use WhichWorkbenchObjectA() and not WhichWorkbenchObject()
    // because the latter wasn't implemented in workbench.library < 51.9
    which = WhichWorkbenchObjectA(NULL, _screen(obj)->MouseX, _screen(obj)->MouseY, ti);
    if(which == WBO_ICON)
    {
      if(type == WBDRAWER)
        AddPart(buf, name, 2048);
      else if(type == WBDISK)
      {
        strncpy(buf, name, 2047);
        buf[2047] = '\0';
        strcat(buf, ":");
      }
      
      which = WBO_DRAWER;
    }
    
    if(which == WBO_DRAWER && buf[0])
    {
      int buflen = strlen(buf);
      if(buflen > 0)
      {
        if((data->dropPath = malloc(buflen+1)))
        {
          strncpy(data->dropPath, buf, buflen);
          data->dropPath[buflen] = '\0';

          D(DBF_GUI, "found dropPath: [%s]", data->dropPath);
          
          DoMethod(_app(obj), MUIM_Application_PushMethod, obj, 3, MUIM_Set, MUIA_AttachmentImage_DropPath, data->dropPath);
        }
      }
    }

    free(buf);
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
          selMsg.drawer = malloc(strlen(n->ln_Name)+1);
          if(selMsg.drawer)
          {
            strcpy(selMsg.drawer, n->ln_Name);

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
                  strcpy(data->dropPath, selMsg.drawer);
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

        D(DBF_GUI, "found dropPath: [%s]", data->dropPath ? data->dropPath : "n/a");

        // signal other listening for the DropPath that we
        // found out where to icon has dropped at exactly.
        if(data->dropPath)
          DoMethod(_app(obj), MUIM_Application_PushMethod, obj, 3, MUIM_Set, MUIA_AttachmentImage_DropPath, data->dropPath);
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
