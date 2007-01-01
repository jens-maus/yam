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

#include "ImageArea_cl.h"
#include "ImageCache.h"

#include <proto/datatypes.h>
#include <proto/icon.h>
#include <proto/graphics.h>

#include "Debug.h"

/* CLASSDATA
struct Data
{
  char *name;
  char *label;
  BOOL free_horiz;
  BOOL free_vert;
  BOOL show_label;
  BOOL setup;
  struct imageCacheNode *imageNode;

  int label_height;
};
*/

/* EXPORT
#define MakeImageObject(file) ImageAreaObject,\
                                MUIA_ImageArea_Filename, (file),\
                              End
*/

/* Private Functions */
/// Image_Load()
// loads an image via our datatype methods
static void Image_Load(struct Data *data, Object *obj)
{
  ENTER();

  if(data->name)
    data->imageNode = ObtainImage(data->name, _screen(obj));

  LEAVE();
}

///
/// Image_UnLoad()
// unloads a certain image from our datatypes cache methods
static void Image_Unload(struct Data *data)
{
  ENTER();

  if(data->imageNode)
  {
    DisposeImage(data->imageNode);
    data->imageNode = NULL;
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
    data->name = NULL;
    data->label = NULL;
    data->free_vert = FALSE;
    data->free_horiz = FALSE;
    data->show_label = TRUE;

    while((tag = NextTagItem(&tags)))
    {
      switch(tag->ti_Tag)
      {
        ATTR(Filename):  data->name = strdup((char *)tag->ti_Data); break;
        ATTR(Label):     data->label = strdup((char *)tag->ti_Data); break;
        ATTR(FreeVert):  data->free_vert = tag->ti_Data; break;
        ATTR(FreeHoriz): data->free_horiz = tag->ti_Data; break;
        ATTR(ShowLabel): data->show_label = tag->ti_Data; break;
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

  if(data->name)
    free(data->name);

  if(data->label)
    free(data->label);

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
    ATTR(Filename): *store = (ULONG)data->name; return TRUE;
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

      ATTR(Filename):
      {
        if(data->name)
          free(data->name);
        
        data->name = strdup((char*)tag->ti_Data);
        
        relayout = TRUE;

        if(data->setup)
        {
          Image_Unload(data);
          Image_Load(data, obj);
          MUI_Redraw(obj, MADF_DRAWOBJECT);
        }

        // make the superMethod call ignore those tags
        tag->ti_Tag = TAG_IGNORE;
      }
      break;
    }
  }

  if(relayout && data->setup)
  {
    Object *parent;

    if((parent = (Object*)xget(obj, MUIA_Parent)))
    {
      // New size if needed
      DoMethod(parent,MUIM_Group_InitChange);
      DoMethod(parent,MUIM_Group_ExitChange);
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

  ENTER();

  if(!DoSuperMethodA(cl, obj, msg))
  {
    RETURN(0);
    return 0;
  }

  Image_Load(data, obj);

  data->setup = TRUE;

  RETURN(TRUE);
  return TRUE;
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
  int minwidth;
  int minheight;

  ENTER();

  DoSuperMethodA(cl, obj, msg);

  mi = ((struct MUIP_AskMinMax *)msg)->MinMaxInfo;

  if(data->imageNode)
  {
    minwidth  = data->imageNode->width;
    minheight = data->imageNode->height;
  }
  else
  {
    minwidth = 0;
    minheight = 0;
  }
  data->label_height = 0;

  if(data->label != NULL && data->show_label)
  {
    struct RastPort rp;
    int width;
    char *str = data->label;
    char *uptr;

    data->label_height = _font(obj)->tf_YSize;

    minheight += data->label_height + (!!data->imageNode);
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

  mi->MinHeight += minheight;
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
  if(data->imageNode)
  {
    Object *dt_obj = data->imageNode->dt_obj;
    struct BitMap *bitmap = NULL;
    int imgWidth = data->imageNode->width;
    int imgHeight = data->imageNode->height;

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
                                                  imgWidth, imgHeight, 0xc0, (PLANEPTR)mask);
      }
      else
      {
        BltBitMapRastPort(bitmap, 0, 0, rp, _mleft(obj)+(_mwidth(obj) - imgWidth)/2,
                                            _mtop(obj) + (_mheight(obj) - data->label_height - imgHeight)/2,
                                            imgWidth, imgHeight, 0xc0);
      }
    }

    rel_y += data->imageNode->height;
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
