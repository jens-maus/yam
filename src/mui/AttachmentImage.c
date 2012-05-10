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

 Superclass:  MUIC_Area
 Description: Custom class to display an image for a specific attachment

 Credits: This class was highly inspired by the similar attachment group &
          image functionality available in Thunderbird and SimpleMail. Large
          code portions where borrowed by the iconclass implementation of
          SimpleMail to allow loading of the default icons via icon.library
          and supporting Drag&Drop on the workbench. Thanks sba! :)

***************************************************************************/

#include "AttachmentImage_cl.h"

#include "SDI_hook.h"

#include <math.h>
#include <stdlib.h>
#include <string.h>
#include <workbench/icon.h>

#if defined(__amigaos4__)
#include <graphics/blitattr.h>
#endif

#include <proto/dos.h>
#include <proto/graphics.h>
#include <proto/layers.h>
#include <proto/icon.h>
#include <proto/wb.h>

#include "YAM.h"

#include "Locale.h"
#include "MUIObjects.h"

#include "mui/AttachmentGroup.h"

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

  struct MUI_EventHandlerNode ehnode;

  Object *attachmentGroup;

  BOOL lastDecodedStatus;
  BOOL eventHandlerAdded;
  BOOL isAfAOSIconLib;

  ULONG selectSecs;
  ULONG selectMicros;
  ULONG scaledWidth;
  ULONG scaledHeight;
  ULONG maxWidth;
  ULONG maxHeight;
};
*/

/* Private Functions */
/// SelectionMsg
struct SelectionMsg
{
  struct Layer *layer;
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

  if(wnd == NULL)
    return ISMACTION_Stop;

  if(wnd->WLayer != msg->layer)
    return ISMACTION_Stop;

  msg->finish = TRUE;

  if((ism->ism_Left + wnd->LeftEdge <= msg->mx) && (msg->mx <= wnd->LeftEdge + ism->ism_Left + ism->ism_Width - 1) &&
     (ism->ism_Top + wnd->TopEdge <= msg->my) && (msg->my <= wnd->TopEdge + ism->ism_Top + ism->ism_Height - 1))
  {
    if(ism->ism_Type == WBDRAWER)
    {
      if((msg->destName = strdup(ism->ism_Name)) != NULL)
        return ISMACTION_Select;
    }
    else if(ism->ism_Type == WBDISK)
    {
      if(asprintf(&msg->destName, "%s:", ism->ism_Name) != -1)
        return ISMACTION_Select;
    }

    return ISMACTION_Stop;
  }

  return ISMACTION_Ignore;
}
MakeStaticHook(SelectionHook, SelectionFunc);
#endif
///
/// FindWriteWindow
// find an open write window which matches a given one
static BOOL FindWriteWindow(struct Window *win)
{
  BOOL found = FALSE;
  struct Node *curNode;

  ENTER();

  IterateList(&G->writeMailDataList, curNode)
  {
    struct WriteMailData *wmData = (struct WriteMailData *)curNode;

    if(wmData->window != NULL &&
       xget(wmData->window, MUIA_Window_Open) == TRUE &&
       (struct Window *)xget(wmData->window, MUIA_Window_Window) == win)
    {
      found = TRUE;
      break;
    }
  }

  RETURN(found);
  return found;
}
///
/// UnloadImage
// unload and free all memory of a formerly loaded image
static void UnloadImage(struct Data *data)
{
  if(data->normalBitMap != NULL)
  {
    FreeBitMap(data->normalBitMap);
    data->normalBitMap = NULL;
  }

  if(data->normalBitMask != NULL)
  {
    FreeBitMap(data->normalBitMask);
    data->normalBitMask = NULL;
  }

  if(data->selectedBitMap != NULL)
  {
    FreeBitMap(data->selectedBitMap);
    data->selectedBitMap = NULL;
  }

  if(data->selectedBitMask != NULL)
  {
    FreeBitMap(data->selectedBitMask);
    data->selectedBitMask = NULL;
  }

  if(data->diskObject != NULL)
  {
    FreeDiskObject(data->diskObject);
    data->diskObject = NULL;
  }
}
///
/// LoadImage
// function that (re)loads the images for both selected and unselected
// state
static void LoadImage(Object *obj, struct Data *data)
{
  #ifndef ICONGETA_SizeBounds
  #define ICONGETA_SizeBounds TAG_IGNORE
  #endif

  ENTER();

  if(data->mailPart != NULL)
  {
    struct Part *mailPart = data->mailPart;
    struct DiskObject *diskObject = NULL;
    struct Rectangle sizeBounds;

    // we first make sure we have freed everything
    UnloadImage(data);

    // Set up the minimum and maximum sizes for the icons to take the dirty work
    // of scaling the icon images to the required size from us.
    // This requires icon.library 53.7+ and will be ignored on previous versions.
    sizeBounds.MinX = 8;
    sizeBounds.MinY = 8;
    sizeBounds.MaxX = data->maxWidth;
    sizeBounds.MaxY = data->maxHeight;

    // only if we have at least icon.library >= v44 and we find deficons
    // we try to identify the file with deficons
    if(isDecoded(mailPart) == TRUE && mailPart->Filename[0] != '\0' &&
       LIB_VERSION_IS_AT_LEAST(IconBase, 44, 0) == TRUE && G->DefIconsAvailable == TRUE)
    {

      D(DBF_GUI, "retrieving diskicon via DEFICONS for '%s'", mailPart->Filename);

      diskObject = (struct DiskObject *)GetIconTags(mailPart->Filename, ICONGETA_FailIfUnavailable, FALSE,
                                                                        ICONGETA_Screen,            _screen(obj),
                                                                        ICONGETA_SizeBounds,        &sizeBounds,
                                                                        TAG_DONE);

      #if defined(DEBUG)
      if(diskObject == NULL)
        W(DBF_GUI, "wasn't able to retrieve diskObject via DEFICONS: %ld", IoErr());
      #endif
    }

    // if we have still not obtained the diskObject we go
    // and load a default icon for a specific ContentType
    if(diskObject == NULL)
    {
      // with icon.library v44+ we can use GetIconTags again.
      if(LIB_VERSION_IS_AT_LEAST(IconBase, 44, 0) == TRUE && mailPart->ContentType != NULL)
      {
        const char *def;

        // build the defaultname now
        if(strnicmp(mailPart->ContentType, "image", 5) == 0)
          def = "picture";
        else if(strnicmp(mailPart->ContentType, "audio", 5) == 0)
          def = "audio";
        else if(strnicmp(mailPart->ContentType, "text", 4) == 0)
        {
          if(strlen(mailPart->ContentType) > 5)
          {
            if(strnicmp((mailPart->ContentType)+5, "html", 4) == 0)
              def = "html";
            else if(strnicmp((mailPart->ContentType)+5, "plain", 5) == 0)
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
        diskObject = (struct DiskObject *)GetIconTags(NULL, ICONGETA_GetDefaultName, def,
                                                            ICONGETA_Screen,         _screen(obj),
                                                            ICONGETA_SizeBounds,     &sizeBounds,
                                                            TAG_DONE);

        // if we still have not retrieved any icon we
        // obtain the standard project icon
        if(diskObject == NULL)
        {
          diskObject = (struct DiskObject *)GetIconTags(NULL, ICONGETA_GetDefaultType, WBPROJECT,
                                                              ICONGETA_Screen,         _screen(obj),
                                                              ICONGETA_SizeBounds,     &sizeBounds,
                                                              TAG_DONE);

          D(DBF_GUI, "diskobject for '%s' retrieved from default WBPROJECT type", mailPart->Filename);
        }
        else
          D(DBF_GUI, "diskobject for '%s' retrieved from default '%s' type", mailPart->Filename, def);
      }
      else
      {
        // on an old OS <= 3.1 we can only use a project icon for
        // the attachment
        diskObject = GetDefDiskObject(WBPROJECT);

        D(DBF_GUI, "diskobject for '%s' retrieved from default WBPROJECT (OS3.1) type", mailPart->Filename);
      }
    }

    // now that we should have the diskObject we get the image of it, blit it in
    // a temporary rastport so that we scan scale it down
    if(diskObject != NULL)
    {
      struct BitMap *orgBitMap;
      struct BitMap *screenBitMap = _screen(obj)->RastPort.BitMap;
      PLANEPTR normalBitMask;
      PLANEPTR selectedBitMask;
      struct RastPort rp;
      ULONG orgWidth;
      ULONG orgHeight;
      ULONG screenDepth = GetBitMapAttr(screenBitMap, BMA_DEPTH);
      struct DrawInfo *dri = GetScreenDrawInfo(_screen(obj));

      // prepare the drawIcon/GetIconRentagle tags

      // defined starting from icon.lib v51+
      #ifndef ICONDRAWA_Transparency
      #define ICONDRAWA_Transparency TAG_IGNORE
      #endif

      struct TagItem drawIconTags[] = { { ICONDRAWA_Borderless,      TRUE       },
                                        { ICONDRAWA_Frameless,       TRUE       },
                                        { ICONDRAWA_Transparency,    255        },
                                        { ICONDRAWA_DrawInfo,        (ULONG)dri },
                                        { TAG_DONE,                  FALSE      } };

      // if this is an alternative part we draw it with
      // transparency of 50%
      if(isAlternativePart(mailPart))
        drawIconTags[2].ti_Data = 128;

      // initialize our temporary rastport
      InitRastPort(&rp);

      // get some information about our diskObject like width/height
      // and the bitmask for transparency drawing
      if(LIB_VERSION_IS_AT_LEAST(IconBase, 44, 0) == TRUE)
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
        orgWidth  = ((struct Image*)diskObject->do_Gadget.GadgetRender)->Width;
        orgHeight = ((struct Image*)diskObject->do_Gadget.GadgetRender)->Height;

        normalBitMask = NULL;
        selectedBitMask = NULL;
      }

      // we first allocate a source bitmap with equal size to the icon size of the diskObject
      if((orgBitMap = AllocBitMap(orgWidth, orgHeight, screenDepth, BMF_CLEAR | BMF_MINPLANES, screenBitMap)) != NULL)
      {
        LONG scaleHeightDiff = orgHeight - data->maxHeight;
        LONG scaleWidthDiff  = orgHeight - data->maxWidth;
        LONG newWidth;
        LONG newHeight;

        // prepare the rastport for drawing the icon in it
        rp.BitMap = orgBitMap;

        if(LIB_VERSION_IS_AT_LEAST(IconBase, 44, 0) == TRUE && data->isAfAOSIconLib == FALSE)
          DrawIconStateA(&rp, diskObject, NULL, 0, 0, IDS_SELECTED, drawIconTags);
        else
        {
          if(isFlagSet(diskObject->do_Gadget.Flags, GFLG_GADGHIMAGE))
            DrawImage(&rp, ((struct Image *)diskObject->do_Gadget.SelectRender), 0, 0);
          else
            DrawImage(&rp, ((struct Image *)diskObject->do_Gadget.GadgetRender), 0, 0);
        }

        // calculate the scale factors now that we have filled up our source bitmap
        if((scaleHeightDiff > 0 && data->maxHeight > 0) ||
           (scaleWidthDiff > 0 && data->maxWidth > 0))
        {
          double scaleFactor;

          // make sure we are scaling proportional
          if(scaleHeightDiff > scaleWidthDiff)
          {
            scaleFactor = (double)orgWidth / (double)orgHeight;
            newWidth = (scaleFactor * data->maxHeight) + 0.5; // round up the value
            newHeight = data->maxHeight;
          }
          else
          {
            scaleFactor = (double)orgHeight / (double)orgWidth;
            newWidth = data->maxWidth;
            newHeight = (scaleFactor * data->maxWidth) + 0.5; // round up the value
          }
        }
        else
        {
          newWidth  = orgWidth;
          newHeight = orgHeight;
        }

        // now we can allocate a new bitmap which should carry the scaled selected image
        if((data->selectedBitMap = AllocBitMap(newWidth, newHeight, screenDepth, BMF_CLEAR | BMF_MINPLANES, orgBitMap)) != NULL)
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
        if(selectedBitMask != NULL &&
           (data->selectedBitMask = AllocBitMap(newWidth, newHeight, 1L, BMF_CLEAR | BMF_MINPLANES, NULL)) != NULL)
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
        if(LIB_VERSION_IS_AT_LEAST(IconBase, 44, 0) == TRUE && data->isAfAOSIconLib == FALSE)
          DrawIconStateA(&rp, diskObject, NULL, 0, 0, IDS_NORMAL, drawIconTags);
        else
          DrawImage(&rp, ((struct Image *)diskObject->do_Gadget.GadgetRender), 0, 0);

        // now we can allocate a new bitmap which should carry the scaled unselected normal image
        if((data->normalBitMap = AllocBitMap(newWidth, newHeight, screenDepth, BMF_CLEAR | BMF_MINPLANES, orgBitMap)) != NULL)
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
        if(normalBitMask != NULL &&
           (data->normalBitMask = AllocBitMap(newWidth, newHeight, 1L, BMF_CLEAR | BMF_MINPLANES, NULL)) != NULL)
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

        if(dri != NULL)
          FreeScreenDrawInfo(_screen(obj), dri);
      }
    }
    else
      W(DBF_GUI, "wasn't able to retrieve any diskobject for file '%s'", mailPart->Filename);

    // store the diskObject in our instance data for later
    // reference
    data->diskObject = diskObject;
    data->lastDecodedStatus = isDecoded(mailPart);
  }

  LEAVE();
}

///

/* Overloaded Methods */
/// OVERLOAD(OM_NEW)
OVERLOAD(OM_NEW)
{
  ENTER();

  if((obj = DoSuperNew(cl, obj,
      MUIA_FillArea,  FALSE, // do not care about background filling
      MUIA_ShortHelp, TRUE,
    TAG_MORE, inittags(msg))) != NULL)
  {
    GETDATA;
    struct TagItem *tags = inittags(msg);
    struct TagItem *tag;

    while((tag = NextTagItem((APTR)&tags)) != NULL)
    {
      switch(tag->ti_Tag)
      {
        case ATTR(MailPart)  : data->mailPart = (struct Part *)tag->ti_Data; break;
        case ATTR(MaxHeight) : data->maxHeight = (ULONG)tag->ti_Data; break;
        case ATTR(MaxWidth)  : data->maxWidth  = (ULONG)tag->ti_Data; break;
        case ATTR(Group)     : data->attachmentGroup = (Object *)tag->ti_Data; break;
      }
    }

    #if defined(__amigaos3__)
    // Check if the AfAOS replacement of icon.library is available.
    // This patch has a broken DrawImageState() implementation which
    // might cause crashes on some systems due to memory trashing,
    // at least on my WinUAE system.
    // This library has a version of 53.4, but an ID string containing a
    // version number of 45.4. Very strange!
    if(IconBase != NULL && LIB_VERSION_IS_AT_LEAST(IconBase, 44, 0) == TRUE && strstr(IconBase->lib_IdString, "45.4") != NULL)
      data->isAfAOSIconLib = TRUE;
    #endif
  }

  RETURN((IPTR)obj);
  return (IPTR)obj;
}
///
/// OVERLOAD(OM_DISPOSE)
OVERLOAD(OM_DISPOSE)
{
  GETDATA;

  free(data->dropPath);
  data->dropPath = NULL;

  return DoSuperMethodA(cl, obj, msg);
}

///
/// OVERLOAD(OM_GET)
OVERLOAD(OM_GET)
{
  GETDATA;
  IPTR *store = ((struct opGet *)msg)->opg_Storage;

  switch(((struct opGet *)msg)->opg_AttrID)
  {
    case ATTR(DoubleClick) : *store = 1; return TRUE;
    case ATTR(DropPath)    : *store = (ULONG)data->dropPath;   return TRUE;
    case ATTR(MailPart)    : *store = (ULONG)data->mailPart;   return TRUE;
    case ATTR(DiskObject)  : *store = (ULONG)data->diskObject; return TRUE;
  }

  return DoSuperMethodA(cl, obj, msg);
}

///
/// OVERLOAD(OM_SET)
OVERLOAD(OM_SET)
{
  GETDATA;
  struct TagItem *tags = inittags(msg), *tag;

  while((tag = NextTagItem((APTR)&tags)) != NULL)
  {
    switch(tag->ti_Tag)
    {
      case ATTR(MaxHeight) : data->maxHeight = (ULONG)tag->ti_Data; break;
      case ATTR(MaxWidth)  : data->maxWidth  = (ULONG)tag->ti_Data; break;
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

  // call the supermethod of the supercall first
  result = DoSuperMethodA(cl, obj, msg);

  if(result != 0 && data->mailPart != NULL)
  {
    // make sure to load/reload the attachment image
    LoadImage(obj, data);

    // add an event handler for the drag&drop operations
    // this object supports.
    data->ehnode.ehn_Priority = -1;
    data->ehnode.ehn_Flags    = 0;
    data->ehnode.ehn_Object   = obj;
    data->ehnode.ehn_Class    = cl;
    data->ehnode.ehn_Events   = IDCMP_MOUSEBUTTONS | IDCMP_RAWKEY;

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

  UnloadImage(data);

  result = DoSuperMethodA(cl, obj, msg);

  RETURN(result);
  return result;
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

  ENTER();

  // call the super method first
  DoSuperMethodA(cl, obj, msg);

  if(((struct MUIP_Draw *)msg)->flags & MADF_DRAWOBJECT)
  {
    struct BitMap *bitmap;
    struct BitMap *bitmask;

    // we have to check whether the decoded status
    // of our mail part changed and if so we have to reload
    // the image in case
    if(data->mailPart != NULL && isDecoded(data->mailPart) == TRUE &&
       data->lastDecodedStatus == FALSE && LIB_VERSION_IS_AT_LEAST(IconBase, 44, 0) == TRUE && G->DefIconsAvailable == TRUE)
    {
      LoadImage(obj, data);
    }

    // check the selected state
    if(xget(obj, MUIA_Selected) == TRUE)
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

    if(bitmap != NULL)
    {
      #if defined(__amigaos4__)
      if(bitmask != NULL)
      {
        BltBitMapTags(BLITA_Source,     bitmap,
                      BLITA_Dest,       _rp(obj),
                      BLITA_SrcType,    BLITT_BITMAP,
                      BLITA_DestType,   BLITT_RASTPORT,
                      BLITA_DestX,      _mleft(obj),
                      BLITA_DestY,      _mtop(obj),
                      BLITA_Width,      _mwidth(obj),
                      BLITA_Height,     _mheight(obj),
                      BLITA_Minterm,    (ABC|ABNC|ANBC),
                      BLITA_MaskPlane,  bitmask->Planes[0],
                      TAG_DONE);
      }
      else
      {
        BltBitMapTags(BLITA_Source,     bitmap,
                      BLITA_Dest,       _rp(obj),
                      BLITA_SrcType,    BLITT_BITMAP,
                      BLITA_DestType,   BLITT_RASTPORT,
                      BLITA_DestX,      _mleft(obj),
                      BLITA_DestY,      _mtop(obj),
                      BLITA_Width,      _mwidth(obj),
                      BLITA_Height,     _mheight(obj),
                      BLITA_Minterm,    (ABC|ABNC),
                      TAG_DONE);
      }
      #else
      // we use an own BltMaskBitMapRastPort() implemenation to also support
      // interleaved images.
      if(bitmask != NULL)
        MyBltMaskBitMapRastPort(bitmap, 0, 0, _rp(obj), _mleft(obj), _mtop(obj), _mwidth(obj), _mheight(obj), (ABC|ABNC|ANBC), bitmask->Planes[0]);
      else
        BltBitMapRastPort(bitmap, 0, 0, _rp(obj), _mleft(obj), _mtop(obj), _mwidth(obj), _mheight(obj), (ABC|ABNC));
      #endif
    }
  }

  RETURN(0);
  return 0;
}
///
/// OVERLOAD(MUIM_HandleEvent)
OVERLOAD(MUIM_HandleEvent)
{
  GETDATA;
  struct IntuiMessage *imsg = ((struct MUIP_HandleEvent *)msg)->imsg;

  ENTER();

  if(imsg == NULL)
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
        xset(obj, MUIA_AttachmentImage_DoubleClick, TRUE,
                  MUIA_Selected, TRUE);
      }
      else
      {
        BOOL lastState = xget(obj, MUIA_Selected);

        // only clear the selection if the user hasn't used
        // the SHIFT key to select multiple items.
        if(isAnyFlagSet(imsg->Qualifier, IEQUALIFIER_RSHIFT|IEQUALIFIER_LSHIFT) == FALSE)
          DoMethod(data->attachmentGroup, MUIM_AttachmentGroup_ClearSelection);

        // invert the selection state
        set(obj, MUIA_Selected, !lastState);
      }

      // save the seconds/micros for the next handleEvent call
      data->selectSecs = imsg->Seconds;
      data->selectMicros = imsg->Micros;

      if(LIB_VERSION_IS_AT_LEAST(WorkbenchBase, 45, 0) == TRUE && data->eventHandlerAdded == TRUE)
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
      if(LIB_VERSION_IS_AT_LEAST(WorkbenchBase, 45, 0) == TRUE)
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
          BOOL lastState = xget(obj, MUIA_Selected);

          // only clear the selection if the user hasn't used
          // the SHIFT key to select multiple items.
          if(isAnyFlagSet(imsg->Qualifier, IEQUALIFIER_RSHIFT|IEQUALIFIER_LSHIFT) == FALSE)
            DoMethod(data->attachmentGroup, MUIM_AttachmentGroup_ClearSelection);

          set(obj, MUIA_Selected, !lastState);

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
  ULONG result;
  struct Screen *wbscreen;

  ENTER();

  free(data->dropPath);
  data->dropPath = NULL;

  // The icon was not dropped on YAM's own write window, so now we check whether
  // YAM is running on the workbench screen or not, because otherwise we skip our
  // further operations.
  if((wbscreen = LockPubScreen("Workbench")) != NULL)
  {
    struct Layer *layer;

    if((layer = WhichLayer(&_screen(obj)->LayerInfo, _screen(obj)->MouseX, _screen(obj)->MouseY)) != NULL)
    {
      if(FindWriteWindow(layer->Window) == FALSE)
      {
        if(wbscreen == _screen(obj))
        {
#if defined(__amigaos4__)
          char *buf;

          if((buf = (STRPTR)malloc(SIZE_PATHFILE)) != NULL)
          {
            ULONG which;
            char name[SIZE_PATH];
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
              if((data->dropPath = strdup(buf)) != NULL)
              {
                D(DBF_GUI, "found dropPath: [%s]", data->dropPath);
                DoMethod(_app(obj), MUIM_Application_PushMethod, obj, 3, MUIM_Set, MUIA_AttachmentImage_DropPath, data->dropPath);
              }
            }
            else
            {
              W(DBF_GUI, "couldn't find drop point of attachment image");
              DisplayBeep(_screen(obj));
            }

            free(buf);
          }
#else // __amigaos4__
          // this stuff only works with Workbench v45+
          if(LIB_VERSION_IS_AT_LEAST(WorkbenchBase, 45, 0) == TRUE)
          {
            struct List *path_list;

            if(WorkbenchControl(NULL, WBCTRLA_GetOpenDrawerList, &path_list, TAG_DONE))
            {
              struct Hook hook;
              struct SelectionMsg selMsg;
              struct Node *n;

              selMsg.layer = layer;
              selMsg.mx = _screen(obj)->MouseX;
              selMsg.my = _screen(obj)->MouseY;
              selMsg.destName = NULL;
              selMsg.finish = FALSE;

              // initialise the selection hook with our data
              InitHook(&hook, SelectionHook, &selMsg);

              IterateList(path_list, n)
              {
                if((selMsg.drawer = strdup(n->ln_Name)) != NULL)
                {
                  ChangeWorkbenchSelectionA(selMsg.drawer, &hook, NULL);

                  if(selMsg.finish == TRUE)
                  {
                    if(selMsg.destName == NULL)
                    {
                      data->dropPath = selMsg.drawer;
                      // don't free the path
                      selMsg.drawer = NULL;
                    }
                    else
                    {
                      int len = strlen(selMsg.destName) + strlen(selMsg.drawer) + 10;

                      if((data->dropPath = malloc(len)) != NULL)
                        AddPath(data->dropPath, selMsg.drawer, selMsg.destName, len);

                      free(selMsg.destName);
                    }
                  }

                  free(selMsg.drawer);

                  if(selMsg.finish == TRUE)
                    break;
                }
              }

              WorkbenchControl(NULL, WBCTRLA_FreeOpenDrawerList, path_list, TAG_DONE);

              if(selMsg.finish == FALSE)
              {
                selMsg.drawer = NULL;

                ChangeWorkbenchSelectionA(NULL, &hook, NULL);

                if(selMsg.finish == TRUE && selMsg.destName != NULL)
                  data->dropPath = selMsg.destName;
              }

              // signal other listening for the DropPath that we
              // found out where to icon has dropped at exactly.
              if(data->dropPath != NULL)
              {
                D(DBF_GUI, "found dropPath: [%s]", data->dropPath);
                DoMethod(_app(obj), MUIM_Application_PushMethod, obj, 3, MUIM_Set, MUIA_AttachmentImage_DropPath, data->dropPath);
              }
              else
              {
                W(DBF_GUI, "couldn't find drop point of attachment image");
                DisplayBeep(_screen(obj));
              }
            }
            else
            {
              W(DBF_GUI, "WorkbenchControl(WBCTRLA_GetOpenDrawerList) failed");
              DisplayBeep(_screen(obj));
            }
          }
#endif // __amigaos4__
        }
        else
          W(DBF_GUI, "YAM is not running on workbench, skipping drop operation");
      }
    }

    UnlockPubScreen(NULL, wbscreen);
  }

  DoMethod(_win(obj), MUIM_Window_RemEventHandler, &data->ehnode);
  data->ehnode.ehn_Events &= ~IDCMP_MOUSEMOVE;
  DoMethod(_win(obj), MUIM_Window_AddEventHandler, &data->ehnode);

  result = DoSuperMethodA(cl, obj, msg);

  RETURN(result);
  return result;
}

///
/// OVERLOAD(MUIM_CreateShortHelp)
// set up a text for the bubble help
OVERLOAD(MUIM_CreateShortHelp)
{
  GETDATA;
  char *shortHelp = NULL;

  ENTER();

  if(data->mailPart != NULL)
  {
    struct Part *mp = data->mailPart;
    char sizestr[SIZE_DEFAULT];

    FormatSize(mp->Size, sizestr, sizeof(sizestr), SF_AUTO);

    if(asprintf(&shortHelp, tr(MSG_MA_MIMEPART_INFO), mp->Nr,
                                                      mp->Name,
                                                      mp->Description,
                                                      DescribeCT(mp->ContentType),
                                                      mp->ContentType,
                                                      sizestr) == -1)
    {
      shortHelp = NULL;
    }
  }

  RETURN(shortHelp);
  return (IPTR)shortHelp;
}

///
/// OVERLOAD(MUIM_DeleteShortHelp)
// free the bubble help text
OVERLOAD(MUIM_DeleteShortHelp)
{
  struct MUIP_DeleteShortHelp *dsh = (struct MUIP_DeleteShortHelp *)msg;

  ENTER();

  free(dsh->help);

  LEAVE();
  return 0;
}

///

/* Public Methods */

