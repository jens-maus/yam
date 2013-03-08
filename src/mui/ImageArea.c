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
 Description: Class for loading/maintaining datatype based images

***************************************************************************/

#include "ImageArea_cl.h"

#include <stdlib.h>
#include <string.h>

#if defined(__amigaos4__)
#include <graphics/blitattr.h>
#else
#include <cybergraphx/cybergraphics.h>
#include <proto/cybergraphics.h>
#endif

#include <guigfx/guigfx.h>

#include <proto/datatypes.h>
#include <proto/icon.h>
#include <proto/graphics.h>
#include <proto/guigfx.h>

#include "YAM.h"

#include "ImageCache.h"

#include "Debug.h"

/* CLASSDATA
struct Data
{
  char *id;
  char *filename;
  char *altText;
  char *label;

  struct ImageCacheNode imageNode;

  ULONG scaledWidth;
  ULONG scaledHeight;
  ULONG maxWidth;
  ULONG maxHeight;

  struct BitMap *bitmap;
  PLANEPTR mask;

  BOOL noMinHeight;
  BOOL free_horiz;
  BOOL free_vert;
  BOOL show_label;
  BOOL imageLoaded;
  BOOL setup;

  int label_height;

  APTR drawHandle;
};
*/

/* EXPORT
#define MakeImageObject(id, file) ImageAreaObject,\
                                    MUIA_ImageArea_ID, (id),\
                                    MUIA_ImageArea_Filename, (file),\
                                  End
#define MakeImageAltObject(id, file, alt) ImageAreaObject,\
                                            MUIA_ImageArea_ID, (id),\
                                            MUIA_ImageArea_Filename, (file),\
                                            MUIA_ImageArea_AltText, (alt),\
                                          End
*/

/* INCLUDE
#include "ImageCache.h"
*/

/* Defines */

/* Private Functions */
/// Image_Load
// loads an image via our datatype methods
static BOOL Image_Load(struct Data *data)
{
  ENTER();

  if(data->imageLoaded == FALSE &&
     data->id != NULL && data->id[0] != '\0' &&
     data->filename != NULL && data->filename[0] != '\0')
  {
    struct ImageCacheNode *node;

    if((node = ObtainImage(data->id, data->filename)) != NULL)
    {
      memcpy(&data->imageNode, node, sizeof(data->imageNode));

      data->imageLoaded = TRUE;
    }
  }

  RETURN(data->imageLoaded);
  return data->imageLoaded;
}

///
/// Image_Unload
// unloads a certain image from our datatypes cache methods
static void Image_Unload(struct Data *data)
{
  ENTER();

  if(data->imageLoaded == TRUE)
  {
    // releasing an image requires a valid ID
    if(data->id != NULL && data->id[0] != '\0')
    {
      D(DBF_IMAGE, "unloaded old image '%s' (%s)", data->id, data->filename);
      ReleaseImage(data->id, FALSE);
    }

    data->imageLoaded = FALSE;
  }

  LEAVE();
}

///
/// Image_Scale
// scale the image to the given size
static void Image_Scale(struct Data *data)
{
  ULONG oldWidth = data->imageNode.width;
  ULONG oldHeight = data->imageNode.height;
  LONG scaleHeightDiff = oldHeight - data->maxHeight;
  LONG scaleWidthDiff  = oldWidth - data->maxWidth;

  ENTER();

  // make sure to scale down the image if maxHeight/maxWidth is specified

  if((scaleHeightDiff > 0 && data->maxHeight > 0) ||
     (scaleWidthDiff > 0 && data->maxWidth > 0))
  {
    ULONG newWidth;
    ULONG newHeight;
    double scaleFactor;

    // make sure we are scaling proportional
    if(scaleHeightDiff > scaleWidthDiff)
    {
      scaleFactor = (double)oldWidth / (double)oldHeight;
      newWidth = scaleFactor * data->maxHeight;
      // force a width of a least one pixel
      if(newWidth < 1)
        newWidth = 1;
      newHeight = data->maxHeight;
    }
    else
    {
      scaleFactor = (double)oldHeight / (double)oldWidth;
      newWidth = data->maxWidth;
      newHeight = scaleFactor * data->maxWidth;
      // force a height of a least one pixel
      if(newHeight < 1)
        newHeight = 1;
    }

    data->scaledWidth  = newWidth;
    data->scaledHeight = newHeight;
  }
  else
  {
    data->scaledWidth  = oldWidth;
    data->scaledHeight = oldHeight;
  }

  LEAVE();
}

///
/// Image_Layout
// scale the image to the given size
static void Image_Layout(struct Data *data)
{
  ENTER();

  D(DBF_IMAGE, "scale image '%s' %08lx %3ldx%3ld -> %3ldx%3ld", data->id, data->imageNode.guigfxPicture, data->imageNode.width, data->imageNode.height, data->scaledWidth, data->scaledHeight);
  if((data->bitmap = CreatePictureBitMap(data->drawHandle, data->imageNode.guigfxPicture,
    GGFX_DestWidth, data->scaledWidth,
    GGFX_DestHeight, data->scaledHeight,
    GGFX_SourceWidth, data->imageNode.width,
    GGFX_SourceHeight, data->imageNode.height,
    TAG_DONE)) != NULL)
  {
    ULONG hasMask;

    GetPictureAttrs(data->imageNode.guigfxPicture, PICATTR_AlphaPresent, &hasMask, TAG_DONE);
    if(hasMask)
    {
      ULONG maskWidth = (data->scaledWidth + 15) & ~0x0f;
      PLANEPTR mask;

      if((mask = AllocRaster(maskWidth, data->scaledHeight)) != NULL)
      {
        if(CreatePictureMask(data->imageNode.guigfxPicture, mask, maskWidth >> 3,
		    GGFX_Ratio, 0x80,
    	    GGFX_DestWidth, data->scaledWidth,
    	    GGFX_DestHeight, data->scaledHeight,
    	    GGFX_SourceWidth, data->imageNode.width,
    	    GGFX_SourceHeight, data->imageNode.height,
    	    TAG_DONE))
        {
          data->mask = mask;
        }
        else
        {
          E(DBF_IMAGE, "CreatePictureMask() failed for image '%s'", data->id);
          FreeRaster(mask, maskWidth, data->scaledHeight);
        }
      }
	}
  }
  else
  {
    E(DBF_IMAGE, "CreatePictureBitMap() failed for image '%s'", data->id);
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
    TAG_MORE, inittags(msg))) != NULL)
  {
    GETDATA;
    struct TagItem *tags = inittags(msg);
    struct TagItem *tag;

    // set default values
    data->id = NULL;
    data->filename = NULL;
    data->altText = NULL;
    data->label = NULL;
    data->free_vert = FALSE;
    data->free_horiz = FALSE;
    data->show_label = TRUE;
    data->noMinHeight = FALSE;
    data->maxHeight = 0;
    data->maxWidth = 0;

    while((tag = NextTagItem((APTR)&tags)) != NULL)
    {
      switch(tag->ti_Tag)
      {
        case ATTR(ID):          if((char *)tag->ti_Data != NULL) { free(data->id); data->id = strdup((char *)tag->ti_Data); } break;
        case ATTR(Filename):    if((char *)tag->ti_Data != NULL) { free(data->filename); data->filename = strdup((char *)tag->ti_Data); } break;
        case ATTR(AltText):     if((char *)tag->ti_Data != NULL) { free(data->altText); data->altText = strdup((char *)tag->ti_Data); } break;
        case ATTR(Label):       if((char *)tag->ti_Data != NULL) { free(data->label); data->label = strdup((char *)tag->ti_Data); } break;
        case ATTR(FreeVert):    data->free_vert   = (BOOL)tag->ti_Data; break;
        case ATTR(FreeHoriz):   data->free_horiz  = (BOOL)tag->ti_Data; break;
        case ATTR(ShowLabel):   data->show_label  = (BOOL)tag->ti_Data; break;
        case ATTR(MaxHeight):   data->maxHeight   = (ULONG)tag->ti_Data; break;
        case ATTR(MaxWidth):    data->maxWidth    = (ULONG)tag->ti_Data; break;
        case ATTR(NoMinHeight): data->noMinHeight = (BOOL)tag->ti_Data; break;
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
  ULONG result;

  ENTER();

  free(data->id);
  free(data->filename);
  free(data->altText);
  free(data->label);

  // everything else has been freed during MUIM_Cleanup already
  result = DoSuperMethodA(cl, obj, msg);

  RETURN(result);
  return result;
}

///
/// OVERLOAD(OM_GET)
// get info about the instance data
OVERLOAD(OM_GET)
{
  GETDATA;
  IPTR *store = ((struct opGet *)msg)->opg_Storage;

  switch(((struct opGet *)msg)->opg_AttrID)
  {
    case ATTR(ID): *store = (IPTR)data->id; return TRUE;
    case ATTR(Filename): *store = (IPTR)data->filename; return TRUE;

    // return the raw image width
    case ATTR(RawWidth):
    {
      BOOL result = FALSE;

      if(data->setup == TRUE && data->imageLoaded == TRUE)
      {
        *store = data->imageNode.width;
        result = TRUE;
      }
      else
      {
        struct ImageCacheNode *icnode;

        if((icnode = ObtainImage(data->id, data->filename)) != NULL)
        {
          *store = icnode->width;

          // release the image again
          ReleaseImage(data->id, FALSE);
          result = TRUE;
        }
      }

      return result;
    }
    break;

    // return the raw image height
    case ATTR(RawHeight):
    {
      BOOL result = FALSE;

      if(data->setup == TRUE && data->imageLoaded == TRUE)
      {
        *store = data->imageNode.height;
        result = TRUE;
      }
      else
      {
        struct ImageCacheNode *icnode;

        if((icnode = ObtainImage(data->id, data->filename)) != NULL)
        {
          *store = icnode->height;

          // release the image again
          ReleaseImage(data->id, FALSE);
          result = TRUE;
        }
      }

      return result;
    }
    break;
  }

  return DoSuperMethodA(cl, obj, msg);
}

///
/// OVERLOAD(OM_SET)
OVERLOAD(OM_SET)
{
  GETDATA;

  ULONG result;
  BOOL relayout = FALSE;
  struct TagItem *tags = inittags(msg), *tag;

  ENTER();

  while((tag = NextTagItem((APTR)&tags)) != NULL)
  {
    switch(tag->ti_Tag)
    {
      case ATTR(ShowLabel):
      {
        if(data->show_label != (BOOL)tag->ti_Data)
        {
          data->show_label = tag->ti_Data;
          relayout = TRUE;
        }

        // make the superMethod call ignore those tags
        tag->ti_Tag = TAG_IGNORE;
      }
      break;

      case ATTR(ID):
      {
        char *newId = (char *)tag->ti_Data;

        Image_Unload(data);

        if(data->id != NULL)
        {
          if(newId == NULL || newId[0] != '\0')
            ReleaseImage(data->id, TRUE);

          free(data->id);
          data->id = NULL;
        }

        if(newId != NULL && newId[0] != '\0')
        {
          data->id = strdup(newId);

          // remember to relayout the image
          relayout = TRUE;
        }

        // make the superMethod call ignore those tags
        tag->ti_Tag = TAG_IGNORE;
      }
      break;

      case ATTR(Filename):
      {
        char *newFilename = (char *)tag->ti_Data;

        Image_Unload(data);

        free(data->filename);
        data->filename = NULL;

        if(newFilename != NULL)
        {
          data->filename = strdup(newFilename);
          // remember to relayout the image
          relayout = TRUE;
        }
        else
        {
          if(data->id != NULL)
            ReleaseImage(data->id, TRUE);
        }

        // make the superMethod call ignore those tags
        tag->ti_Tag = TAG_IGNORE;
      }
      break;
    }
  }

  if(relayout == TRUE &&
     data->setup == TRUE &&
     Image_Load(data) == TRUE)
  {
  //Image_Scale(data);
  }

  result = DoSuperMethodA(cl, obj, msg);

  RETURN(result);
  return result;
}

///
/// OVERLOAD(MUIM_Setup)
OVERLOAD(MUIM_Setup)
{
  GETDATA;
  IPTR result;

  ENTER();

  if((result = DoSuperMethodA(cl, obj, msg)))
  {
    data->setup = TRUE;
    Image_Load(data);
    Image_Scale(data);
  }

  RETURN(result);
  return result;
}

///
/// OVERLOAD(MUIM_Show)
OVERLOAD(MUIM_Show)
{
  GETDATA;
  IPTR result;

  ENTER();

  if((result = DoSuperMethodA(cl, obj, msg)))
  {
    if((data->drawHandle = ObtainDrawHandle(G->imageCachePenShareMap, _rp(obj), _screen(obj)->ViewPort.ColorMap, TAG_DONE)) != NULL)
    {
      SHOWVALUE(DBF_IMAGE, data->drawHandle);
      Image_Layout(data);
    }
  }

  RETURN(result);
  return result;
}

///
/// OVERLOAD(MUIM_Hide)
OVERLOAD(MUIM_Hide)
{
  GETDATA;
  ULONG result;

  ENTER();

  if(data->drawHandle != NULL)
  {
    ReleaseDrawHandle(data->drawHandle);
    data->drawHandle = NULL;
  }

  if(data->bitmap != NULL)
  {
    FreeBitMap(data->bitmap);
    data->bitmap = NULL;
  }

  if(data->mask != NULL)
  {
    FreeRaster(data->mask, (data->scaledWidth + 15) & ~0x0f, data->scaledHeight);
    data->mask = NULL;
  }

  result = DoSuperMethodA(cl, obj, msg);

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

  Image_Unload(data);

  data->setup = FALSE;

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
  ULONG minwidth;
  ULONG minheight;

  ENTER();

  DoSuperMethodA(cl, obj, msg);

  mi = ((struct MUIP_AskMinMax *)msg)->MinMaxInfo;

  if(data->imageLoaded == TRUE)
  {
    minwidth  = data->scaledWidth;
    minheight = data->scaledHeight;
  }
  else if(data->altText != NULL && data->altText[0] != '\0')
  {
    struct RastPort rp;

    InitRastPort(&rp);
    SetFont(&rp, _font(obj));
    minwidth = TextLength(&rp, data->altText, strlen(data->altText));
    minheight = _font(obj)->tf_YSize;
  }
  else
  {
    minwidth = 0;
    minheight = 0;
  }
  data->label_height = 0;

  if(data->label != NULL && data->show_label == TRUE)
  {
    struct RastPort rp;
    ULONG width;
    char *str = data->label;
    char *uptr;

    data->label_height = _font(obj)->tf_YSize;

    minheight += data->label_height + (!!data->imageLoaded);
    InitRastPort(&rp);
    SetFont(&rp, _font(obj));

    if(!(uptr = strchr(str,'_')))
    {
      width = TextLength(&rp, str, strlen(str));
    }
    else
    {
      width = TextLength(&rp, str, uptr - str);
      width += TextLength(&rp, uptr+1, strlen(uptr+1));
    }

    if(width > minwidth)
      minwidth = width;
  }

  mi->MinHeight += data->noMinHeight ? 0 : minheight;
  mi->DefHeight += minheight;
  if(data->free_vert)
    mi->MaxHeight += MUI_MAXMAX;
  else
    mi->MaxHeight += minheight;

  mi->MinWidth += minwidth;
  mi->DefWidth += minwidth;
  if(data->free_horiz)
    mi->MaxWidth += MUI_MAXMAX;
  else
    mi->MaxWidth += minwidth;

  RETURN(0);
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
    LONG rel_y = 0;
    struct RastPort *rp = _rp(obj);

    // in case we successfully have an imageNode object
    // we blit the image on our rastport
    D(DBF_IMAGE, "draw image '%s' loaded %ld bitmap %08lx mask %08lx", data->id, data->imageLoaded, data->bitmap, data->mask);
    if(data->imageLoaded == TRUE)
    {
      if(data->bitmap != NULL)
      {
        if(data->mask != NULL)
        {
          BltMaskBitMapRastPort(data->bitmap,
                            0,
                            0,
                            rp,
                            _mleft(obj) + (_mwidth(obj) - data->scaledWidth) / 2,
                            _mtop(obj) + (_mheight(obj) - data->label_height - data->scaledHeight) / 2,
                            data->scaledWidth,
                            data->scaledHeight,
                            (ABC|ABNC|ANBC),
                            data->mask);
        }
        else
        {
          BltBitMapRastPort(data->bitmap,
                            0,
                            0,
                            rp,
                            _mleft(obj) + (_mwidth(obj) - data->scaledWidth) / 2,
                            _mtop(obj) + (_mheight(obj) - data->label_height - data->scaledHeight) / 2,
                            data->scaledWidth,
                            data->scaledHeight,
                            (ABC|ABNC));
        }

        rel_y += data->scaledHeight;
      }
    }
    else if(data->altText != NULL && data->altText[0] != '\0')
    {
      LONG len;
      struct TextExtent te;

      if((len = TextFit(rp, data->altText, strlen(data->altText), &te, NULL, 1, _mwidth(obj), _mheight(obj))) > 0)
      {
        Move(rp, _mleft(obj) + (_mwidth(obj) - te.te_Width)/2, _mtop(obj) + (_mheight(obj) - te.te_Height)/2 + _font(obj)->tf_Baseline);
        SetAPen(rp, _dri(obj)->dri_Pens[TEXTPEN]);
        SetDrMd(rp, JAM1);
        Text(rp, data->altText, len);
      }
    }

    if(data->label != NULL && data->show_label == TRUE)
    {
      STRPTR ufreestr = StripUnderscore(data->label);

      if(ufreestr != NULL)
      {
        LONG len;
        struct TextExtent te;

        SetFont(rp, _font(obj));

        if((len = TextFit(rp, ufreestr, strlen(ufreestr), &te, NULL, 1, _mwidth(obj), _font(obj)->tf_YSize)) > 0)
        {
          char *str = data->label;
          char *uptr;

          Move(rp, _mleft(obj) + (_mwidth(obj) - te.te_Width)/2, _mbottom(obj)+_font(obj)->tf_Baseline - data->label_height);
          SetAPen(rp, _dri(obj)->dri_Pens[TEXTPEN]);
          SetDrMd(rp, JAM1);

          if((uptr = strchr(str, '_')) == NULL)
          {
            Text(rp, str, strlen(str));
          }
          else
          {
            Text(rp, str, uptr - str);
            if(uptr[1] != '\0')
            {
              SetSoftStyle(rp, FSF_UNDERLINED, AskSoftStyle(rp));
              Text(rp, uptr+1, 1);
              SetSoftStyle(rp, FS_NORMAL, 0xffff);
              Text(rp, uptr+2, strlen(uptr+2));
            }
          }
        }
      }
    }
  }

  RETURN(0);
  return 0;
}

///

/* Public Methods */
