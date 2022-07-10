/***************************************************************************

 YAM - Yet Another Mailer
 Copyright (C) 1995-2000 Marcel Beck
 Copyright (C) 2000-2022 YAM Open Source Team

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

#if !defined(__amigaos4__)
#include <cybergraphx/cybergraphics.h>
#include <proto/cybergraphics.h>
#endif

#include <proto/datatypes.h>
#include <proto/icon.h>
#include <proto/graphics.h>

#if defined(__amigaos4__)
#include <graphics/blitattr.h>
#ifndef MINTERM_ABC
  #define MINTERM_ABC  0x80
#endif
#ifndef MINTERM_ABNC
  #define MINTERM_ABNC 0x40
#endif
#ifndef MINTERM_ANBC
  #define MINTERM_ANBC 0x20
#endif
#ifndef MINTERM_SRCMASK
  #define MINTERM_SRCMASK (MINTERM_ABC | MINTERM_ABNC | MINTERM_ANBC)
#endif
#endif

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

  struct BitMap *scaledBitMap;
  struct BitMap *scaledBitMask;

  APTR scaledPixelArray;
  ULONG scaledBytesPerRow;

  ULONG scaledWidth;
  ULONG scaledHeight;
  ULONG maxWidth;
  ULONG maxHeight;

  BOOL noMinHeight;
  BOOL free_horiz;
  BOOL free_vert;
  BOOL show_label;
  BOOL imageLoaded;
  BOOL setup;

  int label_height;
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
/// WritePixelArrayAlpha
#if defined(__MORPHOS__)
  #ifndef WritePixelArrayAlpha
    #define WritePixelArrayAlpha(__p0, __p1, __p2, __p3, __p4, __p5, __p6, __p7, __p8, __p9) \
      LP10(216, ULONG , WritePixelArrayAlpha, \
        APTR , __p0, a0, \
        UWORD , __p1, d0, \
        UWORD , __p2, d1, \
        UWORD , __p3, d2, \
        struct RastPort *, __p4, a1, \
        UWORD , __p5, d3, \
        UWORD , __p6, d4, \
        UWORD , __p7, d5, \
        UWORD , __p8, d6, \
        ULONG , __p9, d7, \
        , CYBERGRAPHICS_BASE_NAME, 0, 0, 0, 0, 0, 0)
  #endif
#elif !defined(__amigaos4__)
  #ifndef WritePixelArrayAlpha
    #if defined(__SASC)
      ULONG WritePixelArrayAlpha(APTR, UWORD, UWORD, UWORD, struct RastPort *, UWORD, UWORD, UWORD, UWORD, ULONG);
      #pragma libcall CyberGfxBase WritePixelArrayAlpha d8 76543921080A
    #else
      #define WritePixelArrayAlpha(__p0, __p1, __p2, __p3, __p4, __p5, __p6, __p7, __p8, __p9) \
        LP10(216, ULONG , WritePixelArrayAlpha, \
          APTR , __p0, a0, \
          UWORD , __p1, d0, \
          UWORD , __p2, d1, \
          UWORD , __p3, d2, \
          struct RastPort *, __p4, a1, \
          UWORD , __p5, d3, \
          UWORD , __p6, d4, \
          UWORD , __p7, d5, \
          UWORD , __p8, d6, \
          ULONG , __p9, d7, \
          , CYBERGRAPHICS_BASE_NAME)
    #endif
  #endif
#endif
///

/* Private Functions */
/// Image_Load
// loads an image via our datatype methods
static BOOL Image_Load(struct Data *data, Object *obj)
{
  ENTER();

  if(data->imageLoaded == FALSE &&
     IsStrEmpty(data->id) == FALSE &&
     IsStrEmpty(data->filename) == FALSE)
  {
    struct ImageCacheNode *node;

    if((node = ObtainImage(data->id, data->filename, _screen(obj))) != NULL)
    {
      memcpy(&data->imageNode, node, sizeof(data->imageNode));

      if(node->dt_obj != NULL)
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
    if(IsStrEmpty(data->id) == FALSE)
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
  ENTER();

  if(data->scaledBitMap != NULL)
  {
    FreeBitMap(data->scaledBitMap);
    data->scaledBitMap = NULL;
  }

  if(data->scaledBitMask != NULL)
  {
    FreeBitMap(data->scaledBitMask);
    data->scaledBitMask = NULL;
  }

  if(data->scaledPixelArray != NULL)
  {
    FreeVecPooled(G->SharedMemPool, data->scaledPixelArray);
    data->scaledPixelArray = NULL;
  }

  data->scaledWidth = 0;
  data->scaledHeight = 0;

  // check if correctly obtained the header and if it is valid
  if(data->imageNode.bitmap != NULL && data->imageNode.depth > 0)
  {
    // make sure to scale down the image if maxHeight/maxWidth is specified
    ULONG oldWidth = data->imageNode.width;
    ULONG oldHeight = data->imageNode.height;
    LONG scaleHeightDiff = oldHeight - data->maxHeight;
    LONG scaleWidthDiff  = oldWidth - data->maxWidth;

    if((scaleHeightDiff > 0 && data->maxHeight > 0) ||
       (scaleWidthDiff > 0 && data->maxWidth > 0))
    {
      ULONG newWidth;
      ULONG newHeight;
      BOOL wasScaled = FALSE;
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

      // in case we have image data with an alpha channel embedded we
      // have to use the PDTM_SCALE method of picture.datatype v45
      if(data->imageNode.pixelArray != NULL)
      {
        BOOL result;

        result = DoMethod(data->imageNode.dt_obj, PDTM_SCALE, newWidth, newHeight, 0);
        if(result == TRUE)
        {
          data->scaledBytesPerRow = newWidth * data->imageNode.bytesPerPixel;
          SHOWVALUE(DBF_IMAGE, data->scaledBytesPerRow);
          SHOWVALUE(DBF_IMAGE, data->imageNode.pixelFormat);

          if((data->scaledPixelArray = AllocVecPooled(G->SharedMemPool, data->scaledBytesPerRow * newHeight)) != NULL)
          {
            // perform a PDTM_READPIXELARRAY operation
            // for writing the image data of the image in our pixelArray
            result = DoMethod(data->imageNode.dt_obj, PDTM_READPIXELARRAY, data->scaledPixelArray, data->imageNode.pixelFormat, data->scaledBytesPerRow,
                                                                           0, 0, newWidth, newHeight);

            if(result == FALSE)
            {
              W(DBF_IMAGE, "PDTM_READPIXELARRAY on image '%s' with depth %ld failed after PDTM_SCALE!", data->imageNode.id, data->imageNode.depth);

              FreeVecPooled(G->SharedMemPool, data->scaledPixelArray);
              data->scaledPixelArray = NULL;
            }
            else
            {
              data->scaledWidth  = newWidth;
              data->scaledHeight = newHeight;

              D(DBF_IMAGE, "Scaled pixelarray in ImageArea (w/h) from %ld/%ld to %ld/%ld", data->imageNode.width,
                                                                                           data->imageNode.height,
                                                                                           data->scaledWidth,
                                                                                           data->scaledHeight);

            }
          }

          // rescale the image to its original size
          DoMethod(data->imageNode.dt_obj, PDTM_SCALE, data->imageNode.width, data->imageNode.height, 0);

          wasScaled = TRUE;
        }
        else
          W(DBF_IMAGE, "PDTM_SCALE method on image '%s' returned an error! picture.datatype >= v45?", data->imageNode.id);
      }

      // check if we generated a scaled interpretation or not
      if(wasScaled == FALSE)
      {
        // now we can allocate the new bitmap and scale it
        // if required. But we use BitMapScale() for all operations
        if((data->scaledBitMap = AllocBitMap(newWidth, newHeight, data->imageNode.depth, BMF_CLEAR|BMF_MINPLANES, data->imageNode.bitmap)) != NULL)
        {
          struct BitScaleArgs args;

          args.bsa_SrcBitMap = data->imageNode.bitmap;
          args.bsa_DestBitMap = data->scaledBitMap;
          args.bsa_Flags = 0;

          args.bsa_SrcY = 0;
          args.bsa_DestY = 0;

          args.bsa_SrcWidth = oldWidth;
          args.bsa_SrcHeight = oldHeight;

          args.bsa_XSrcFactor = oldWidth;
          args.bsa_XDestFactor = newWidth;

          args.bsa_YSrcFactor = oldHeight;
          args.bsa_YDestFactor = newHeight;

          args.bsa_SrcX = 0;
          args.bsa_DestX = 0;

          // scale the image now with the arguments set
          BitMapScale(&args);

          // read out the scaled values
          data->scaledWidth  = args.bsa_DestWidth;
          data->scaledHeight = args.bsa_DestHeight;

          D(DBF_IMAGE, "Scaled image in ImageArea (w/h) from %ld/%ld to %ld/%ld", data->imageNode.width,
                                                                                  data->imageNode.height,
                                                                                  data->scaledWidth,
                                                                                  data->scaledHeight);
        }

        // now we can allocate the new bitmap for our scale mask
        if(data->imageNode.mask != NULL &&
           (data->scaledBitMask = AllocBitMap(newWidth, newHeight, 1L, BMF_CLEAR|BMF_MINPLANES, NULL)) != NULL)
        {
          struct BitScaleArgs args;
          struct BitMap bm;

          InitBitMap(&bm, 1L, oldWidth, oldHeight);
          bm.Planes[0] = data->imageNode.mask;

          args.bsa_SrcBitMap = &bm;
          args.bsa_DestBitMap = data->scaledBitMask;
          args.bsa_Flags = 0;

          args.bsa_SrcY = 0;
          args.bsa_DestY = 0;

          args.bsa_SrcWidth = oldWidth;
          args.bsa_SrcHeight = oldHeight;

          args.bsa_XSrcFactor = oldWidth;
          args.bsa_XDestFactor = newWidth;

          args.bsa_YSrcFactor = oldHeight;
          args.bsa_YDestFactor = newHeight;

          args.bsa_SrcX = 0;
          args.bsa_DestX = 0;

          // scale the image now with the arguments set
          BitMapScale(&args);

          D(DBF_IMAGE, "Scaled transparency mask of image");
        }
        else
        {
          D(DBF_IMAGE, "no bitmask for image available, skipped scaling of bitmask");
          data->scaledBitMask = NULL;
        }
      }
    }
    else
    {
      D(DBF_IMAGE, "no image scaling required for image '%s'", data->imageNode.id);

      data->scaledBitMap = NULL;
      data->scaledBitMask = NULL;
      data->scaledPixelArray = NULL;
    }
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

        if((icnode = ObtainImage(data->id, data->filename, NULL)) != NULL)
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

        if((icnode = ObtainImage(data->id, data->filename, NULL)) != NULL)
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
          if(IsStrEmpty(newId) == TRUE)
            ReleaseImage(data->id, TRUE);

          free(data->id);
          data->id = NULL;
        }

        if(IsStrEmpty(newId) == FALSE)
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
     Image_Load(data, obj) == TRUE)
  {
    Image_Scale(data);
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
    if(Image_Load(data, obj) == TRUE)
      Image_Scale(data);

    data->setup = TRUE;
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

  // now it should be fine to free the allocated bitmap
  if(data->scaledBitMap != NULL)
  {
    FreeBitMap(data->scaledBitMap);
    data->scaledBitMap = NULL;
  }

  if(data->scaledBitMask != NULL)
  {
    FreeBitMap(data->scaledBitMask);
    data->scaledBitMask = NULL;
  }

  if(data->scaledPixelArray != NULL)
  {
    FreeVecPooled(G->SharedMemPool, data->scaledPixelArray);
    data->scaledPixelArray = NULL;
  }

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
    if(data->scaledBitMap != NULL ||
       data->scaledPixelArray != NULL)
    {
      minwidth  = data->scaledWidth;
      minheight = data->scaledHeight;
    }
    else
    {
      minwidth  = data->imageNode.width;
      minheight = data->imageNode.height;
    }
  }
  else if(IsStrEmpty(data->altText) == FALSE)
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
    if(data->imageLoaded == TRUE)
    {
      if(data->scaledBitMap != NULL)
      {
        ULONG width = MIN(data->scaledWidth, (ULONG)_mwidth(obj));
        ULONG height = MIN(data->scaledHeight, (ULONG)_mheight(obj));

        #if defined(__amigaos4__)
        if(data->scaledBitMask != NULL)
        {
          D(DBF_IMAGE, "drawing scaled/masked bitmap image '%s' (%s)", data->id, data->filename);
          BltBitMapTags(BLITA_Source,     data->scaledBitMap,
                        BLITA_Dest,       rp,
                        BLITA_SrcType,    BLITT_BITMAP,
                        BLITA_DestType,   BLITT_RASTPORT,
                        BLITA_DestX,      _mleft(obj) + (_mwidth(obj) - width) / 2,
                        BLITA_DestY,      _mtop(obj) + (_mheight(obj) - data->label_height - height) / 2,
                        BLITA_Width,      width,
                        BLITA_Height,     height,
                        BLITA_Minterm,    MINTERM_SRCMASK,
                        BLITA_MaskPlane,  data->scaledBitMask->Planes[0],
                        TAG_DONE);
        }
        else
        {
          D(DBF_IMAGE, "drawing scaled bitmap image '%s' (%s)", data->id, data->filename);
          BltBitMapTags(BLITA_Source,     data->scaledBitMap,
                        BLITA_Dest,       rp,
                        BLITA_SrcType,    BLITT_BITMAP,
                        BLITA_DestType,   BLITT_RASTPORT,
                        BLITA_DestX,      _mleft(obj) + (_mwidth(obj) - width) / 2,
                        BLITA_DestY,      _mtop(obj) + (_mheight(obj) - data->label_height - height) / 2,
                        BLITA_Width,      width,
                        BLITA_Height,     height,
                        BLITA_Minterm,    MINTERM_ABC | MINTERM_ABNC,
                        TAG_DONE);
        }
        #else
        if(data->scaledBitMask != NULL)
        {
          D(DBF_IMAGE, "drawing scaled/masked bitmap image '%s' (%s)", data->id, data->filename);
          // we use an own BltMaskBitMapRastPort() implemenation to also support
          // interleaved images.
          MyBltMaskBitMapRastPort(data->scaledBitMap,
                                  0,
                                  0,
                                  rp,
                                  _mleft(obj) + (_mwidth(obj) - width) / 2,
                                  _mtop(obj) + (_mheight(obj) - data->label_height - height) / 2,
                                  width,
                                  height,
                                  (ABC|ABNC|ANBC),
                                  data->scaledBitMask->Planes[0]);
        }
        else
        {
          D(DBF_IMAGE, "drawing scaled bitmap image '%s' (%s)", data->id, data->filename);
          BltBitMapRastPort(data->scaledBitMap,
                            0,
                            0,
                            rp,
                            _mleft(obj) + (_mwidth(obj) - width) / 2,
                            _mtop(obj) + (_mheight(obj) - data->label_height - height) / 2,
                            width,
                            height,
                            (ABC|ABNC));
        }
        #endif

        rel_y += height;
      }
      else if(data->scaledPixelArray != NULL)
      {
        ULONG width = MIN(data->scaledWidth, (ULONG)_mwidth(obj));
        ULONG height = MIN(data->scaledHeight, (ULONG)_mheight(obj));

        #if defined(__amigaos4__)
        D(DBF_IMAGE, "drawing scaled (A)RGB image '%s' (%s)", data->id, data->filename);
        BltBitMapTags(BLITA_Source,         data->scaledPixelArray,
                      BLITA_Dest,           rp,
                      BLITA_SrcType,        (data->imageNode.pixelFormat == PBPAFMT_ARGB) ? BLITT_ARGB32 : BLITT_RGB24,
                      BLITA_DestType,       BLITT_RASTPORT,
                      BLITA_DestX,          _mleft(obj) + (_mwidth(obj) - width) / 2,
                      BLITA_DestY,          _mtop(obj) + (_mheight(obj) - data->label_height - height) / 2,
                      BLITA_Width,          width,
                      BLITA_Height,         height,
                      BLITA_SrcBytesPerRow, data->scaledBytesPerRow,
                      BLITA_UseSrcAlpha,    TRUE,
                      TAG_DONE);
        #else
        // this also works for OS3, because we only have valid pixel data if
        // cybergraphics.library and picture.datatype are able to handle the
        // alpha channel correctly.
        if(data->imageNode.pixelFormat == PBPAFMT_ARGB)
        {
          D(DBF_IMAGE, "drawing scaled ARGB image '%s' (%s)", data->id, data->filename);
          WritePixelArrayAlpha(data->scaledPixelArray,
                               0,
                               0,
                               data->scaledBytesPerRow,
                               rp,
                               _mleft(obj) + (_mwidth(obj) - width) / 2,
                               _mtop(obj) + (_mheight(obj) - data->label_height - height) / 2,
                               width,
                               height,
                               0xffffffff);
        }
        else
        {
          D(DBF_IMAGE, "drawing scaled RGB image '%s' (%s)", data->id, data->filename);
          WritePixelArray(data->scaledPixelArray,
                          0,
                          0,
                          data->scaledBytesPerRow,
                          rp,
                          _mleft(obj) + (_mwidth(obj) - width) / 2,
                          _mtop(obj) + (_mheight(obj) - data->label_height - height) / 2,
                          width,
                          height,
                          RECTFMT_RGB);
        }
        #endif

        rel_y += height;
      }
      else
      {
        // blit the (A)RGB data if we retrieved them successfully.
        if(data->imageNode.pixelArray != NULL)
        {
          #if defined(__amigaos4__)
          D(DBF_IMAGE, "drawing (A)RGB image '%s' (%s)", data->id, data->filename);
          BltBitMapTags(BLITA_Source,         data->imageNode.pixelArray,
                        BLITA_Dest,           rp,
                        BLITA_SrcType,        (data->imageNode.pixelFormat == PBPAFMT_ARGB) ? BLITT_ARGB32 : BLITT_RGB24,
                        BLITA_DestType,       BLITT_RASTPORT,
                        BLITA_DestX,          _mleft(obj) + (_mwidth(obj) - data->imageNode.width) / 2,
                        BLITA_DestY,          _mtop(obj) + (_mheight(obj) - data->label_height - data->imageNode.height) / 2,
                        BLITA_Width,          data->imageNode.width,
                        BLITA_Height,         data->imageNode.height,
                        BLITA_SrcBytesPerRow, data->imageNode.bytesPerRow,
                        BLITA_UseSrcAlpha,    TRUE,
                        TAG_DONE);
          #else
          // this also works for OS3, because we only have valid pixel data if
          // cybergraphics.library and picture.datatype are able to handle the
          // alpha channel correctly.
          if(data->imageNode.pixelFormat == PBPAFMT_ARGB)
          {
            D(DBF_IMAGE, "drawing ARGB image '%s' (%s)", data->id, data->filename);
            WritePixelArrayAlpha(data->imageNode.pixelArray,
                                 0,
                                 0,
                                 data->imageNode.bytesPerRow,
                                 rp,
                                 _mleft(obj) + (_mwidth(obj) - data->imageNode.width) / 2,
                                 _mtop(obj) + (_mheight(obj) - data->label_height - data->imageNode.height) / 2,
                                 data->imageNode.width,
                                 data->imageNode.height,
                                 0xffffffff);
          }
          else
          {
            D(DBF_IMAGE, "drawing RGB image '%s' (%s)", data->id, data->filename);
            WritePixelArray(data->imageNode.pixelArray,
                            0,
                            0,
                            data->imageNode.bytesPerRow,
                            rp,
                            _mleft(obj) + (_mwidth(obj) - data->imageNode.width) / 2,
                            _mtop(obj) + (_mheight(obj) - data->label_height - data->imageNode.height) / 2,
                            data->imageNode.width,
                            data->imageNode.height,
                            RECTFMT_RGB);
          }
          #endif
        }
        // blit the bitmap if we retrieved it successfully.
        else if(data->imageNode.bitmap != NULL)
        {
          #if defined(__amigaos4__)
          if(data->imageNode.mask != NULL)
          {
            D(DBF_IMAGE, "drawing masked bitmap image '%s' (%s)", data->id, data->filename);
            BltBitMapTags(BLITA_Source,     data->imageNode.bitmap,
                          BLITA_Dest,       rp,
                          BLITA_SrcType,    BLITT_BITMAP,
                          BLITA_DestType,   BLITT_RASTPORT,
                          BLITA_DestX,      _mleft(obj) + (_mwidth(obj) - data->imageNode.width)/2,
                          BLITA_DestY,      _mtop(obj)  + (_mheight(obj) - data->label_height - data->imageNode.height)/2,
                          BLITA_Width,      data->imageNode.width,
                          BLITA_Height,     data->imageNode.height,
                          BLITA_Minterm,    MINTERM_SRCMASK,
                          BLITA_MaskPlane,  data->imageNode.mask,
                          TAG_DONE);
          }
          else
          {
            D(DBF_IMAGE, "drawing bitmap image '%s' (%s)", data->id, data->filename);
            BltBitMapTags(BLITA_Source,     data->imageNode.bitmap,
                          BLITA_Dest,       rp,
                          BLITA_SrcType,    BLITT_BITMAP,
                          BLITA_DestType,   BLITT_RASTPORT,
                          BLITA_DestX,      _mleft(obj) + (_mwidth(obj) - data->imageNode.width)/2,
                          BLITA_DestY,      _mtop(obj)  + (_mheight(obj) - data->label_height - data->imageNode.height)/2,
                          BLITA_Width,      data->imageNode.width,
                          BLITA_Height,     data->imageNode.height,
                          BLITA_Minterm,    MINTERM_ABC | MINTERM_ABNC,
                          TAG_DONE);
          }
          #else
          if(data->imageNode.mask != NULL)
          {
            D(DBF_IMAGE, "drawing masked bitmap image '%s' (%s)", data->id, data->filename);
            // we use an own BltMaskBitMapRastPort() implemenation to also support
            // interleaved images.
            MyBltMaskBitMapRastPort(data->imageNode.bitmap,
                                    0,
                                    0,
                                    rp,
                                    _mleft(obj) + (_mwidth(obj) - data->imageNode.width)/2,
                                    _mtop(obj) + (_mheight(obj) - data->label_height - data->imageNode.height)/2,
                                    data->imageNode.width,
                                    data->imageNode.height,
                                    (ABC|ABNC|ANBC),
                                    data->imageNode.mask);
          }
          else
          {
            D(DBF_IMAGE, "drawing bitmap image '%s' (%s)", data->id, data->filename);
            BltBitMapRastPort(data->imageNode.bitmap,
                              0,
                              0,
                              rp,
                              _mleft(obj) + (_mwidth(obj) - data->imageNode.width)/2,
                              _mtop(obj) + (_mheight(obj) - data->label_height - data->imageNode.height)/2,
                              data->imageNode.width,
                              data->imageNode.height,
                              (ABC|ABNC));
          }
          #endif
        }

        rel_y += data->imageNode.height;
      }
    }
    else if(IsStrEmpty(data->altText) == FALSE)
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
