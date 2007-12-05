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
 Description: Class for loading/mainting datatype based images

***************************************************************************/

#include "ImageCache.h"
#include "ImageArea_cl.h"

#include <proto/datatypes.h>
#include <proto/icon.h>
#include <proto/graphics.h>

#include "Debug.h"

/* CLASSDATA
struct Data
{
  char *id;
  char *filename;
  char *label;

  struct ImageCacheNode imageNode;
  struct BitMap *scaledBitMap;

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
*/

/* Private Functions */
/// Image_Load()
// loads an image via our datatype methods
static BOOL Image_Load(struct Data *data, Object *obj)
{
  ENTER();

  if(data->imageLoaded == FALSE &&
     data->id != NULL && data->id[0] != '\0' &&
     data->filename != NULL && data->filename[0] != '\0')
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
/// Image_Unload()
// unloads a certain image from our datatypes cache methods
static void Image_Unload(struct Data *data)
{
  ENTER();

  if(data->imageLoaded == TRUE)
  {
  	// releasing an image requires a valid ID
  	if(data->id != NULL && data->id[0] != '\0')
  	{
      D(DBF_IMAGE, "unloaded old image '%s' from '%s'", data->id, data->filename);
      ReleaseImage(data->id, FALSE);
    }
    data->imageLoaded = FALSE;
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
    TAG_MORE, inittags(msg))))
  {
    GETDATA;
    struct TagItem *tags = inittags(msg);
    struct TagItem *tag;

    // set default values
    data->id = NULL;
    data->filename = NULL;
    data->label = NULL;
    data->free_vert = FALSE;
    data->free_horiz = FALSE;
    data->show_label = TRUE;

    while((tag = NextTagItem(&tags)) != NULL)
    {
      switch(tag->ti_Tag)
      {
        ATTR(ID):          if((char *)tag->ti_Data != NULL) data->id = strdup((char *)tag->ti_Data); break;
        ATTR(Filename):    if((char *)tag->ti_Data != NULL) data->filename = strdup((char *)tag->ti_Data); break;
        ATTR(Label):       if((char *)tag->ti_Data != NULL) data->label = strdup((char *)tag->ti_Data); break;
        ATTR(FreeVert):    data->free_vert   = (BOOL)tag->ti_Data; break;
        ATTR(FreeHoriz):   data->free_horiz  = (BOOL)tag->ti_Data; break;
        ATTR(ShowLabel):   data->show_label  = (BOOL)tag->ti_Data; break;
        ATTR(MaxHeight):   data->maxHeight   = (ULONG)tag->ti_Data; break;
        ATTR(MaxWidth):    data->maxWidth    = (ULONG)tag->ti_Data; break;
        ATTR(NoMinHeight): data->noMinHeight = (BOOL)tag->ti_Data; break;
      }
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
  ULONG result;

  ENTER();

  if(data->id != NULL)
    free(data->id);

  if(data->filename != NULL)
    free(data->filename);

  if(data->label != NULL)
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
  ULONG *store = ((struct opGet *)msg)->opg_Storage;

  switch(((struct opGet *)msg)->opg_AttrID)
  {
    ATTR(ID): *store = (ULONG)data->id; return TRUE;
    ATTR(Filename): *store = (ULONG)data->filename; return TRUE;

    // return the raw image width
    ATTR(RawWidth):
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
    ATTR(RawHeight):
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

  while((tag = NextTagItem(&tags)))
  {
    switch(tag->ti_Tag)
    {
      ATTR(ShowLabel):
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

      ATTR(ID):
      {
        char *newId = (char *)tag->ti_Data;

        Image_Unload(data);

        if(data->id != NULL)
        {
          free(data->id);
          data->id = NULL;
        }

        if(newId != NULL)
          data->id = strdup(newId);

        // remember to relayout the image
        relayout = TRUE;

        // make the superMethod call ignore those tags
        tag->ti_Tag = TAG_IGNORE;
      }
      break;

      ATTR(Filename):
      {
        char *newFilename = (char *)tag->ti_Data;

        Image_Unload(data);

        if(data->filename != NULL)
        {
          free(data->filename);
          data->filename = NULL;
        }

        if(newFilename != NULL)
          data->filename = strdup(newFilename);
        else
          ReleaseImage(data->id, TRUE);

        // remember to relayout the image
        relayout = TRUE;

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
    Object *parent;

    if((parent = (Object*)xget(obj, MUIA_Parent)))
    {
      // New size if needed
      if(DoMethod(parent,MUIM_Group_InitChange))
        DoMethod(parent,MUIM_Group_ExitChange);

      MUI_Redraw(obj, MADF_DRAWOBJECT);
    }
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
  ULONG result;

  ENTER();

  if((result = DoSuperMethodA(cl, obj, msg)))
  {
    if(Image_Load(data, obj) == TRUE)
    {
      struct BitMap *orgBitMap = NULL;
      struct BitMapHeader *bitMapHeader = NULL;

      GetDTAttrs(data->imageNode.dt_obj, PDTA_BitMapHeader, &bitMapHeader,
                                         PDTA_DestBitMap,   &orgBitMap,
                                         TAG_DONE);

      // try another attribute if the other DestBitMap failed
      if(orgBitMap == NULL)
        GetDTAttrs(data->imageNode.dt_obj, PDTA_BitMap, &orgBitMap, TAG_DONE);

      // check if correctly obtained the header and if it is valid
      if(orgBitMap != NULL && bitMapHeader != NULL && bitMapHeader->bmh_Depth > 0)
      {
        // make sure to scale down the image if maxHeight/maxWidth is specified
        LONG scaleHeightDiff = bitMapHeader->bmh_Height - data->maxHeight;
        LONG scaleWidthDiff  = bitMapHeader->bmh_Width - data->maxWidth;
        LONG newWidth;
        LONG newHeight;

        if((scaleHeightDiff > 0 && data->maxHeight > 0) ||
           (scaleWidthDiff > 0 && data->maxWidth > 0))
        {
          double scaleFactor;

          // make sure we are scaling proportional
          if(scaleHeightDiff > scaleWidthDiff)
          {
            scaleFactor = (double)bitMapHeader->bmh_Width / (double)bitMapHeader->bmh_Height;
            newWidth = scaleFactor * data->maxHeight + 0.5; // roundup the value
            newHeight = data->maxHeight;
          }
          else
          {
            scaleFactor = (double)bitMapHeader->bmh_Height / (double)bitMapHeader->bmh_Width;
            newWidth = data->maxWidth;
            newHeight = scaleFactor * data->maxWidth + 0.5; // roundup the value
          }

          // now we can allocate the new bitmap and scale it
          // if required. But we use BitMapScale() for all operations
          data->scaledBitMap = AllocBitMap(newWidth, newHeight, bitMapHeader->bmh_Depth, BMF_CLEAR|BMF_MINPLANES, orgBitMap);
          if(data->scaledBitMap != NULL)
          {
            struct BitScaleArgs args;

            args.bsa_SrcBitMap = orgBitMap;
            args.bsa_DestBitMap = data->scaledBitMap;
            args.bsa_Flags = 0;

            args.bsa_SrcY = 0;
            args.bsa_DestY = 0;

            args.bsa_SrcWidth = bitMapHeader->bmh_Width;
            args.bsa_SrcHeight = bitMapHeader->bmh_Height;

            args.bsa_XSrcFactor = bitMapHeader->bmh_Width;
            args.bsa_XDestFactor = newWidth;

            args.bsa_YSrcFactor = bitMapHeader->bmh_Height;
            args.bsa_YDestFactor = newHeight;

            args.bsa_SrcX = 0;
            args.bsa_DestX = 0;

            // scale the image now with the arguments set
            BitMapScale(&args);

            // read out the scaled values
            data->scaledWidth  = args.bsa_DestWidth;
            data->scaledHeight = args.bsa_DestHeight;

            D(DBF_GUI, "Scaled image in ImageArea (w/h) from %ld/%ld to %ld/%ld", bitMapHeader->bmh_Width,
                                                                                  bitMapHeader->bmh_Height,
                                                                                  data->scaledWidth,
                                                                                  data->scaledHeight);
          }
        }
        else
        {
          D(DBF_GUI, "no image scaling required");
          data->scaledBitMap = NULL;
        }
      }
    }

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
  int minwidth;
  int minheight;

  ENTER();

  DoSuperMethodA(cl, obj, msg);

  mi = ((struct MUIP_AskMinMax *)msg)->MinMaxInfo;

  if(data->imageLoaded == TRUE)
  {
    if(data->scaledBitMap != NULL)
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
  else
  {
    minwidth = 0;
    minheight = 0;
  }
  data->label_height = 0;

  if(data->label != NULL && data->show_label == TRUE)
  {
    struct RastPort rp;
    int width;
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
    mi->MaxHeight = MUI_MAXMAX;
  else
    mi->MaxHeight += minheight;

  mi->MinWidth += minwidth;
  mi->DefWidth += minwidth;
  if(data->free_horiz)
    mi->MaxWidth = MUI_MAXMAX;
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
  LONG rel_y = 0;
  struct RastPort *rp = _rp(obj);

  ENTER();

  DoSuperMethodA(cl, obj, msg);

  // in case we successfully have an imageNode object
  // we blit the image on our rastport
  if(data->imageLoaded == TRUE)
  {
    if(data->scaledBitMap != NULL)
    {
      BltBitMapRastPort(data->scaledBitMap, 0, 0, rp, _mleft(obj)+(_mwidth(obj) - data->scaledWidth)/2,
                                                      _mtop(obj) + (_mheight(obj) - data->label_height - data->scaledHeight)/2,
                                                      data->scaledWidth, data->scaledHeight, (ABC|ABNC));

      rel_y += data->scaledHeight;
    }
    else
    {
      Object *dt_obj = data->imageNode.dt_obj;
      struct BitMap *bitmap = NULL;
      int imgWidth = data->imageNode.width;
      int imgHeight = data->imageNode.height;

      // try to get the bitmap first via PDTA_DestBitMap and
      // otherwise with PDTA_BitMap
      GetDTAttrs(dt_obj, PDTA_DestBitMap, &bitmap, TAG_DONE);
      if(bitmap == NULL)
        GetDTAttrs(dt_obj, PDTA_BitMap, &bitmap, TAG_DONE);

      // blit the bitmap if we retrieved it successfully.
      if(bitmap)
      {
        APTR mask = NULL;

        // try to obtain a mask for e.g. transparency display of an image
        GetDTAttrs(dt_obj, PDTA_MaskPlane, &mask, TAG_DONE);
        if(mask)
        {
          // we use an own BltMaskBitMapRastPort() implemenation to also support
          // interleaved images.
          MyBltMaskBitMapRastPort(bitmap, 0, 0, rp, _mleft(obj)+(_mwidth(obj) - imgWidth)/2,
                                                    _mtop(obj) + (_mheight(obj) - data->label_height - imgHeight)/2,
                                                    imgWidth, imgHeight, (ABC|ABNC|ANBC), (PLANEPTR)mask);
        }
        else
        {
          BltBitMapRastPort(bitmap, 0, 0, rp, _mleft(obj)+(_mwidth(obj) - imgWidth)/2,
                                              _mtop(obj) + (_mheight(obj) - data->label_height - imgHeight)/2,
                                              imgWidth, imgHeight, (ABC|ABNC));
        }
      }

      rel_y += data->imageNode.height;
    }
  }

  if(data->label && data->show_label)
  {
    STRPTR ufreestr = StripUnderscore(data->label);
    struct TextExtent te;
    LONG ufreelen;

    if(ufreestr != NULL)
    {
      LONG len = ufreelen = strlen(ufreestr);
      SetFont(rp,_font(obj));

      len = TextFit(rp, ufreestr, ufreelen, &te, NULL, 1, _mwidth(obj), _font(obj)->tf_YSize);
      if(len > 0)
      {
        char *str = data->label;
        char *uptr;
        LONG left = _mleft(obj);
        left += (_mwidth(obj) - te.te_Width)/2;
        Move(rp, left, _mbottom(obj)+_font(obj)->tf_Baseline - data->label_height);
        SetAPen(rp, _dri(obj)->dri_Pens[TEXTPEN]);
        SetDrMd(rp, JAM1);

        if(!(uptr = strchr(str, '_')))
        {
          Text(rp, str, strlen(str));
        }
        else
        {
          Text(rp, str, uptr - str);
          if(uptr[1])
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

  RETURN(0);
  return 0;
}

///

/* Public Methods */
