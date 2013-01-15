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
#include "YAM_write.h"

#include "Locale.h"
#include "MUIObjects.h"

#include "mui/AttachmentGroup.h"

#include "Debug.h"

/* CLASSDATA
struct Data
{
  struct DiskObject *diskObject;
  struct DiskObject *drawDiskObject;
  struct Part *mailPart;
  struct Attach *attachment;

  struct BitMap *normalBitMap;
  struct BitMap *selectedBitMap;

  struct BitMap *normalBitMask;
  struct BitMap *selectedBitMask;

  char *dropPath;

  struct MUI_EventHandlerNode ehnode;

  Object *attachmentGroup;

  BOOL lastDecodedStatus;
  BOOL eventHandlerAdded;

  ULONG selectSecs;
  ULONG selectMicros;
  ULONG scaledWidth;
  ULONG scaledHeight;
  ULONG maxWidth;
  ULONG maxHeight;

  #if defined(__amigaos4__)
  #else
  struct List *wbDrawerList;
  #endif
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
/// IsWBWindowMsg
struct IsWBWindowMsg
{
  struct Window *window;
  LONG mx;
  LONG my;

  BOOL found;
};

///
/// IsWBWindowHook
#if !defined(__amigaos4__)
HOOKPROTONO(IsWBWindowFunc, ULONG, struct IconSelectMsg *ism)
{
  struct IsWBWindowMsg *msg = (struct IsWBWindowMsg *)hook->h_Data;
  struct Window *wnd = ism->ism_DrawerWindow;

  SHOWVALUE(DBF_ALWAYS, ism->ism_DrawerWindow);
  SHOWVALUE(DBF_ALWAYS, ism->ism_ParentWindow);
  SHOWVALUE(DBF_ALWAYS, msg->window);

/*
  if(wnd == NULL)
    return ISMACTION_Stop;

  if(msg->window != wnd)
    return ISMACTION_Stop;

  if((ism->ism_Left + wnd->LeftEdge <= msg->mx) && (msg->mx <= wnd->LeftEdge + ism->ism_Left + ism->ism_Width - 1) &&
     (ism->ism_Top + wnd->TopEdge <= msg->my) && (msg->my <= wnd->TopEdge + ism->ism_Top + ism->ism_Height - 1))
  {
    if(ism->ism_Type == WBDRAWER || ism->ism_Type == WBDISK)
      msg->found = TRUE;

    return ISMACTION_Stop;
  }
*/

  return ISMACTION_Ignore;
}
MakeStaticHook(IsWBWindowHook, IsWBWindowFunc);
#endif

///
/// IsWorkbenchWindow
static BOOL IsWorkbenchWindow(struct Data *data, struct Window *win, LONG mx, LONG my)
{
  BOOL isWBWindow = FALSE;
  struct Screen *wbscreen;

  ENTER();

  if((wbscreen = LockPubScreen("Workbench")) != NULL)
  {
    if(wbscreen == win->WScreen)
    {
#if defined(__amigaos4__)
      ULONG which;
      ULONG type = ~0;

      struct TagItem ti[] =
      {
        { WBOBJA_Type, (ULONG)&type },
        { TAG_DONE,    FALSE        }
      };

      // Note that we use WhichWorkbenchObjectA() and not WhichWorkbenchObject()
      // because the latter wasn't implemented in workbench.library < 51.9
      which = WhichWorkbenchObjectA(NULL, mx, my, ti);
      if(which == WBO_ICON || which == WBO_DRAWER)
      {
        if(type == WBDRAWER || type == WBDISK)
        {
          isWBWindow = TRUE;
        }
      }
#else // __amigaos4__
      // this stuff only works with Workbench v45+
      if(data->wbDrawerList != NULL)
      {
        struct Hook hook;
        struct IsWBWindowMsg wbMsg;
        struct Node *n;

        wbMsg.window = win;
        wbMsg.mx = mx;
        wbMsg.my = my;
        wbMsg.found = FALSE;

        // initialise the hook with our data
        InitHook(&hook, IsWBWindowHook, &wbMsg);

        IterateList(data->wbDrawerList, n)
        {
          ChangeWorkbenchSelectionA(n->ln_Name, &hook, NULL);

          if(wbMsg.found == TRUE)
          {
            isWBWindow = TRUE;
            D(DBF_ALWAYS, "found window '%s'", n->ln_Name);
            break;
          }
          else
          {
            D(DBF_ALWAYS, "skip window '%s'", n->ln_Name);
          }
        }
      }
#endif // __amigaos4__
    }
    else
      W(DBF_GUI, "YAM is not running on workbench, skipping drop operation");

    UnlockPubScreen(NULL, wbscreen);
  }

  RETURN(isWBWindow);
  return isWBWindow;
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

  if(data->drawDiskObject != NULL)
  {
    if(data->diskObject == data->drawDiskObject)
      data->diskObject = NULL;

    FreeDiskObject(data->drawDiskObject);
    data->drawDiskObject = NULL;
  }

  if(data->diskObject != NULL)
  {
    FreeDiskObject(data->diskObject);
    data->diskObject = NULL;
  }
}

///
/// GetIconSize
// obtain the size of an icon
static void GetIconSize(struct DiskObject *dobj, struct TagItem *tags, LONG *width, LONG *height)
{
  ENTER();

  if(LIB_VERSION_IS_AT_LEAST(IconBase, 44, 0) == TRUE)
  {
    struct Rectangle rect;

    GetIconRectangleA(NULL, dobj, NULL, &rect, tags);

    *width  = rect.MaxX - rect.MinX + 1;
    *height = rect.MaxY - rect.MinY + 1;
  }
  else
  {
    *width  = ((struct Image *)dobj->do_Gadget.GadgetRender)->Width;
    *height = ((struct Image *)dobj->do_Gadget.GadgetRender)->Height;
  }

  LEAVE();
}

///
/// LoadImage
// function that (re)loads the images for both selected and unselected
// state
static void LoadImage(struct IClass *cl, Object *obj)
{
  #ifndef ICONGETA_SizeBounds
  #define ICONGETA_SizeBounds TAG_IGNORE
  #endif

  GETDATA;

  ENTER();

  if((data->mailPart != NULL || data->attachment != NULL) && data->maxWidth != 0 && data->maxHeight != 0)
  {
    struct Part *mailPart = data->mailPart;
    struct Attach *attachment = data->attachment;
    char *iconFile = NULL;
    struct DiskObject *diskObject = NULL;
    struct Rectangle sizeBounds;

    // we first make sure we have freed everything
    UnloadImage(data);

    if(mailPart != NULL && isDecoded(mailPart) == TRUE && mailPart->Filename[0] != '\0')
    {
      iconFile = mailPart->Filename;
    }
    else if(attachment != NULL && attachment->FilePath[0] != '\0')
    {
      iconFile = attachment->FilePath;
    }

    // Set up the minimum and maximum sizes for the icons to take the dirty work
    // of scaling the icon images to the required size from us.
    // This requires icon.library 53.7+ and will be ignored on previous versions.
    sizeBounds.MinX = 8;
    sizeBounds.MinY = 8;
    sizeBounds.MaxX = data->maxWidth;
    sizeBounds.MaxY = data->maxHeight;
    D(DBF_ALWAYS, "max %ld %ld",data->maxWidth,data->maxHeight);

    // only if we have at least icon.library >= v44 and we find deficons
    // we try to identify the file with deficons
    if(iconFile != NULL && LIB_VERSION_IS_AT_LEAST(IconBase, 44, 0) == TRUE && G->DefIconsAvailable == TRUE)
    {
      D(DBF_GUI, "retrieving diskObject via DEFICONS for '%s'", iconFile);

      diskObject = GetIconTags(iconFile,
        ICONGETA_FailIfUnavailable, FALSE,
        ICONGETA_Screen,            _screen(obj),
        ICONGETA_SizeBounds,        &sizeBounds,
        TAG_DONE);

      #if defined(DEBUG)
      if(diskObject == NULL)
        W(DBF_GUI, "retrieving diskObject via DEFICONS for '%s' failed, error %ld", iconFile, IoErr());
      #endif
    }

    // if we have still not obtained the diskObject we go
    // and load a default icon for a specific ContentType
    if(diskObject == NULL)
    {
      char *contentType = NULL;

      // use the supplied attachment file name no matter if the file
      // itself is available or not
      if(mailPart != NULL)
      {
        iconFile = mailPart->Name;
        contentType = mailPart->ContentType;
      }
      else if(attachment != NULL)
      {
        iconFile = attachment->Name;
        contentType = attachment->ContentType;
      }

      // with icon.library v44+ we can use GetIconTags again.
      if(LIB_VERSION_IS_AT_LEAST(IconBase, 44, 0) == TRUE && contentType != NULL)
      {
        const char *def = NULL;

        // build the default name now
        if(strnicmp(contentType, "image", 5) == 0)
        {
          def = "picture";
        }
        else if(strnicmp(contentType, "audio", 5) == 0)
        {
          def = "audio";
        }
        else if(strnicmp(contentType, "text", 4) == 0)
        {
          if(strlen(contentType) > 5)
          {
            if(strnicmp((contentType)+5, "html", 4) == 0)
              def = "html";
            else if(strnicmp((contentType)+5, "plain", 5) == 0)
              def = "ascii";
            else
              def = "text";
          }
          else
            def = "text";
        }
        else
        {
          // try the file name's extension if it exists
          if(iconFile != NULL)
          {
            if((def = strrchr(iconFile, '.')) != NULL)
              def++;
          }
        }

        // fall back to "attach" if nothing else was appropriate
        if(def == NULL)
          def = "attach";

        // try to retrieve the icon for that type with an automatic
        // fallback to the default project icon
        diskObject = GetIconTags(NULL,
          ICONGETA_GetDefaultName, def,
          ICONGETA_GetDefaultType, WBPROJECT,
          ICONGETA_Screen,         _screen(obj),
          ICONGETA_SizeBounds,     &sizeBounds,
          TAG_DONE);

        D(DBF_GUI, "diskobject for '%s' retrieved from default '%s' type", iconFile, def);
      }
      else
      {
        // on an old OS <= 3.1 we can only use a project icon for
        // the attachment
        diskObject = GetDefDiskObject(WBPROJECT);

        D(DBF_GUI, "diskobject for '%s' retrieved from default WBPROJECT (OS3.1) type", iconFile);
      }
    }

    // now that we should have the diskObject we get the image of it, blit it in
    // a temporary rastport so that we scan scale it down
    if(diskObject != NULL)
    {
      LONG orgWidth;
      LONG orgHeight;
      LONG scaleHeightDiff;
      LONG scaleWidthDiff;
      LONG newWidth;
      LONG newHeight;

      // prepare the drawIcon/GetIconRentagle tags

      // defined starting from icon.lib v51+
      #ifndef ICONDRAWA_Transparency
      #define ICONDRAWA_Transparency TAG_IGNORE
      #endif

      struct TagItem drawIconTags[] = { { ICONDRAWA_Borderless,      TRUE             },
                                        { ICONDRAWA_Frameless,       TRUE             },
                                        { ICONDRAWA_Transparency,    255              },
                                        { ICONDRAWA_DrawInfo,        (ULONG)_dri(obj) },
                                        { TAG_DONE,                  FALSE            } };

      // if this is an alternative part we draw it with
      // transparency of 50%
      if(isAlternativePart(mailPart))
        drawIconTags[2].ti_Data = 128;

      // get some information about our diskObject like width/height
      GetIconSize(diskObject, drawIconTags, &orgWidth, &orgHeight);

      scaleWidthDiff  = orgWidth - data->maxWidth;
      scaleHeightDiff = orgHeight - data->maxHeight;

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

      // first try to let icon.library scale the icon
      if(LIB_VERSION_IS_AT_LEAST(IconBase, 44, 0) == TRUE)
      {
        if(newWidth != orgWidth || newHeight != orgHeight)
        {
          struct DiskObject *scaledDiskObject;

          #ifndef ICONDUPA_Width
          #define ICONDUPA_Width TAG_IGNORE
          #endif
          #ifndef ICONDUPA_Height
          #define ICONDUPA_Height TAG_IGNORE
          #endif

          if((scaledDiskObject = DupDiskObject(diskObject,
            ICONDUPA_Width, newWidth,
            ICONDUPA_Height, newHeight,
            IA_Width, newWidth,
            IA_Height, newHeight,
            TAG_DONE)) != NULL)
          {
            LONG scaledWidth;
            LONG scaledHeight;

            // obtain the size of the scaled icon
            GetIconSize(scaledDiskObject, drawIconTags, &scaledWidth, &scaledHeight);

            // if the scaled icon has the demanded size then we will use this one
            // during draw operations
            if(scaledWidth == newWidth && scaledHeight == newHeight)
            {
              data->drawDiskObject = scaledDiskObject;
              data->scaledWidth = newWidth;
              data->scaledHeight = newHeight;
            }
            else
            {
              FreeDiskObject(scaledDiskObject);
            }
          }
        }
        else
        {
          data->drawDiskObject = diskObject;
          data->scaledWidth = orgWidth;
          data->scaledHeight = orgHeight;
        }
      }

      if(data->drawDiskObject == NULL)
      {
        // obtaining a scaled icon failed
        // we have to fall back to the bitmap based approach
        struct BitMap *orgBitMap;
        struct BitMap *screenBitMap = _screen(obj)->RastPort.BitMap;
        ULONG screenDepth = GetBitMapAttr(screenBitMap, BMA_DEPTH);
        PLANEPTR normalBitMask;
        PLANEPTR selectedBitMask;

        // obtain the bitmask for transparency drawing
        if(LIB_VERSION_IS_AT_LEAST(IconBase, 44, 0) == TRUE)
        {
          struct TagItem iconCtrlTags[] = { { ICONCTRLA_GetImageMask1, (ULONG)NULL },
                                            { ICONCTRLA_GetImageMask2, (ULONG)NULL },
                                            { TAG_DONE,                FALSE       } };

          iconCtrlTags[0].ti_Data = (ULONG)&normalBitMask;
          iconCtrlTags[1].ti_Data = (ULONG)&selectedBitMask;

          // query the bitmask
          IconControlA(diskObject, iconCtrlTags);
        }
        else
        {
          normalBitMask = NULL;
          selectedBitMask = NULL;
        }

        // we first allocate a source bitmap with equal size to the icon size of the diskObject
        if((orgBitMap = AllocBitMap(orgWidth, orgHeight, screenDepth, BMF_CLEAR | BMF_MINPLANES, screenBitMap)) != NULL)
        {
          // create a new layer info and a layer to get a clipping RastPort
          struct Layer_Info *li;

          if((li = NewLayerInfo()) != NULL)
          {
            struct Layer *l;

            if((l = CreateUpfrontLayer(li, orgBitMap, 0, 0, orgWidth-1, orgHeight-1, LAYERSIMPLE, NULL)) != NULL)
            {
              // the layer now provides a RastPort with active clipping
              // we use this RastPort to let icon.library render the icon image to
              struct RastPort *rp = l->rp;

              if(LIB_VERSION_IS_AT_LEAST(IconBase, 44, 0) == TRUE)
                DrawIconStateA(rp, diskObject, NULL, 0, 0, IDS_SELECTED, drawIconTags);
              else
              {
                if(isFlagSet(diskObject->do_Gadget.Flags, GFLG_GADGHIMAGE))
                  DrawImage(rp, ((struct Image *)diskObject->do_Gadget.SelectRender), 0, 0);
                else
                  DrawImage(rp, ((struct Image *)diskObject->do_Gadget.GadgetRender), 0, 0);
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
              if(LIB_VERSION_IS_AT_LEAST(IconBase, 44, 0) == TRUE)
                DrawIconStateA(rp, diskObject, NULL, 0, 0, IDS_NORMAL, drawIconTags);
              else
                DrawImage(rp, ((struct Image *)diskObject->do_Gadget.GadgetRender), 0, 0);

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

              DeleteLayer(0, l);
            }

            DisposeLayerInfo(li);
	      }

          FreeBitMap(orgBitMap);
        }
      }
    }
    else
      W(DBF_GUI, "retrieving any diskObject for file '%s' failed", iconFile);

    // store the diskObject in our instance data for later
    // reference
    data->diskObject = diskObject;
    data->lastDecodedStatus = mailPart != NULL && isDecoded(mailPart);
  }

  // there is no need to fill the background if we have no mask bitmap,
  // but with a mask we should enable double buffering to reduce flickering
  if(data->normalBitMask != NULL || data->selectedBitMask != NULL)
    SetSuperAttrs(cl, obj, MUIA_DoubleBuffer, TRUE, MUIA_FillArea, TRUE, TAG_DONE);
  else
    SetSuperAttrs(cl, obj, MUIA_DoubleBuffer, FALSE, MUIA_FillArea, FALSE, TAG_DONE);

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
        case ATTR(Attachment): data->attachment = (struct Attach *)tag->ti_Data; break;
        case ATTR(MaxHeight) : data->maxHeight = (ULONG)tag->ti_Data; break;
        case ATTR(MaxWidth)  : data->maxWidth  = (ULONG)tag->ti_Data; break;
        case ATTR(Group)     : data->attachmentGroup = (Object *)tag->ti_Data; break;
      }
    }
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
    case ATTR(Attachment)  : *store = (ULONG)data->attachment; return TRUE;
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
  BOOL refresh = FALSE;

  while((tag = NextTagItem((APTR)&tags)) != NULL)
  {
    switch(tag->ti_Tag)
    {
      case ATTR(MaxHeight):
      {
        if(data->maxHeight == 0)
          refresh = TRUE;
        data->maxHeight = (ULONG)tag->ti_Data;
      }
      break;

      case ATTR(MaxWidth):
      {
        if(data->maxWidth == 0)
          refresh = TRUE;
        data->maxWidth = (ULONG)tag->ti_Data;
      }
      break;
    }
  }

  // reload the icon image if the maximum dimensions changed from invalid to valid values
  if(refresh == TRUE)
    LoadImage(cl, obj);

  return DoSuperMethodA(cl, obj, msg);
}

///
/// OVERLOAD(MUIM_Setup)
OVERLOAD(MUIM_Setup)
{
  GETDATA;
  IPTR result;

  ENTER();

  // call the method of the superclass first
  result = DoSuperMethodA(cl, obj, msg);

  if(result != 0 && (data->mailPart != NULL || data->attachment != NULL))
  {
    // make sure to load/reload the attachment image
    LoadImage(cl, obj);

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
  struct MUIP_Draw *dmsg = (struct MUIP_Draw *)msg;

  ENTER();

  // call the super method first
  DoSuperMethodA(cl, obj, msg);

  if(isFlagSet(dmsg->flags, MADF_DRAWOBJECT) || isFlagSet(dmsg->flags, MADF_DRAWUPDATE))
  {
    LONG state;
    struct BitMap *bitmap;
    struct BitMap *bitmask;

    // we have to check whether the decoded status
    // of our mail part changed and if so we have to reload
    // the image in case
    if(data->mailPart != NULL && isDecoded(data->mailPart) == TRUE &&
       data->lastDecodedStatus == FALSE && LIB_VERSION_IS_AT_LEAST(IconBase, 44, 0) == TRUE && G->DefIconsAvailable == TRUE)
    {
      LoadImage(cl, obj);
    }

    // check the selected state
    if(xget(obj, MUIA_Selected) == TRUE)
    {
      state = IDS_SELECTED;
      bitmap = data->selectedBitMap;
      bitmask = data->selectedBitMask;
    }
    else
    {
      state = IDS_NORMAL;
      bitmap = data->normalBitMap;
      bitmask = data->normalBitMask;
    }

    if(data->drawDiskObject != NULL && LIB_VERSION_IS_AT_LEAST(IconBase, 44, 0) == TRUE)
    {
      struct TagItem drawIconTags[] = { { ICONDRAWA_Borderless,      TRUE             },
                                        { ICONDRAWA_Frameless,       TRUE             },
                                        { ICONDRAWA_Transparency,    255              },
                                        { ICONDRAWA_DrawInfo,        (ULONG)_dri(obj) },
                                        { TAG_DONE,                  FALSE            } };

      // if this is an alternative part we draw it with
      // transparency of 50%
      if(data->mailPart != NULL && isAlternativePart(data->mailPart))
        drawIconTags[2].ti_Data = 128;

      DrawIconStateA(_rp(obj), data->drawDiskObject, NULL, _mleft(obj), _mtop(obj), state, drawIconTags);
    }
    else if(bitmap != NULL)
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
  ULONG result = 0;

  ENTER();

  if(imsg != NULL)
  {
    switch(imsg->Class)
    {
      case IDCMP_MOUSEBUTTONS:
      {
        if(!(_isinobject(obj, imsg->MouseX, imsg->MouseY)))
        {
          data->selectSecs = 0;
          data->selectMicros = 0;
        }
        // in case the image is selected
        else if(imsg->Code == SELECTDOWN)
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

          result = MUI_EventHandlerRC_Eat;
        }
        // in case the image is unselected by the user
        else if(imsg->Code == SELECTUP)
        {
          if(LIB_VERSION_IS_AT_LEAST(WorkbenchBase, 45, 0) == TRUE)
          {
            DoMethod(_win(obj), MUIM_Window_RemEventHandler, &data->ehnode);
            data->ehnode.ehn_Events &= ~IDCMP_MOUSEMOVE;
            DoMethod(_win(obj), MUIM_Window_AddEventHandler, &data->ehnode);
          }

          result = MUI_EventHandlerRC_Eat;
        }
      }
      break;

      case IDCMP_MOUSEMOVE:
      {
        // in case this event is a mouse move we signal a dragging event, but only
        // if it starts within our object region.
        if(_isinobject(obj, imsg->MouseX, imsg->MouseY))
        {
          #if !defined(__amigaos4__)
          BOOL freeDrawerList;

          // this requires workbench.library V45+
          if(LIB_VERSION_IS_AT_LEAST(WorkbenchBase, 45, 0) == TRUE)
          {
            freeDrawerList = WorkbenchControl(NULL,
              WBCTRLA_GetOpenDrawerList, &data->wbDrawerList,
              TAG_DONE);
          }
          else
          {
		    freeDrawerList = FALSE;
		  }

          // forget any possibly changed pointer in case the call above failed
          if(freeDrawerList == FALSE)
            data->wbDrawerList = NULL;
          #endif

          DoMethod(obj, MUIM_DoDrag, imsg->MouseX - _mleft(obj), imsg->MouseY - _mtop(obj));

          #if !defined(__amigaos4__)
          if(freeDrawerList != FALSE)
          {
            // free the obtained list again
            WorkbenchControl(NULL, WBCTRLA_FreeOpenDrawerList, data->wbDrawerList, TAG_DONE);
            data->wbDrawerList = NULL;
          }
          #endif
        }
      }
      break;

      case IDCMP_RAWKEY:
      {
        switch(imsg->Code)
        {
          case IECODE_RETURN:
          {
            if(obj == (Object *)xget(_win(obj), MUIA_Window_ActiveObject))
            {
              set(obj, MUIA_AttachmentImage_DoubleClick, TRUE);
              result = MUI_EventHandlerRC_Eat;
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
              result = MUI_EventHandlerRC_Eat;
            }
          }
          break;
        }
      }
      break;
    }
  }

  RETURN(result);
  return result;
}

///
/// OVERLOAD(MUIM_DragEvent)
OVERLOAD(MUIM_DragEvent)
{
  GETDATA;
  struct MUIP_DragEvent *de = (struct MUIP_DragEvent *)msg;

  ENTER();

/*
  if(de->objwindow != NULL && de->obj == NULL)
  {
    if(IsWorkbenchWindow(data, de->objwindow, de->imsg->MouseX, de->imsg->MouseY) == TRUE)
    {
      de->mouseptrtype = 0;//POINTERTYPE_Normal;
      setFlag(de->flags, MUIF_DRAGEVENT_FOREIGNDROP);
      setFlag(de->flags, MUIF_DRAGEVENT_MOUSECHANGED);
    }
  }
*/

  LEAVE();
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
          if(data->wbDrawerList != NULL)
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

            IterateList(data->wbDrawerList, n)
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
  else if(data->attachment != NULL)
  {
    struct Attach *att = data->attachment;
    char sizestr[SIZE_DEFAULT];

    FormatSize(att->Size, sizestr, sizeof(sizestr), SF_AUTO);

    #warning use different string
    if(asprintf(&shortHelp, tr(MSG_MA_MIMEPART_INFO), 0,
                                                      att->Name,
                                                      att->Description,
                                                      DescribeCT(att->ContentType),
                                                      att->ContentType,
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
