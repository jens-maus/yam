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
 Description: Custom class to load&display a user image in readmailgroup
              object.

***************************************************************************/

#include "ImageCache.h"
#include "UserImage_cl.h"

#include <proto/datatypes.h>
#include <proto/icon.h>
#include <proto/graphics.h>

#include "Debug.h"

/* CLASSDATA
struct Data
{
  char *address;
  char *fileName;

  struct ImageCacheNode imageNode;
  struct BitMap *scaledBitMap;

  ULONG scaledWidth;
  ULONG scaledHeight;
  ULONG maxWidth;
  ULONG maxHeight;
  BOOL noMinHeight;

  BOOL imageLoaded;
};
*/

/* Overloaded Methods */
/// OVERLOAD(OM_NEW)
OVERLOAD(OM_NEW)
{
  if((obj = DoSuperNew(cl, obj,
      MUIA_FillArea, FALSE, // do not care about background filling
    TAG_MORE, inittags(msg))))
  {
    GETDATA;
    struct TagItem *tags = inittags(msg);
    struct TagItem *tag;

    while((tag = NextTagItem(&tags)) != NULL)
    {
      switch(tag->ti_Tag)
      {
        ATTR(Address)     : if((char *)tag->ti_Data != NULL) data->address = strdup((char *)tag->ti_Data); break;
        ATTR(Filename)    : if((char *)tag->ti_Data != NULL) data->fileName = strdup((char *)tag->ti_Data); break;
        ATTR(MaxHeight)   : data->maxHeight   = (ULONG)tag->ti_Data; break;
        ATTR(MaxWidth)    : data->maxWidth    = (ULONG)tag->ti_Data; break;
        ATTR(NoMinHeight) : data->noMinHeight = (BOOL)tag->ti_Data; break;
      }
    }
  }

  return (ULONG)obj;
}
///
/// OVERLOAD(OM_DISPOSE)
OVERLOAD(OM_DISPOSE)
{
  GETDATA;

  if(data->address != NULL)
    free(data->address);
  if(data->fileName != NULL)
    free(data->fileName);

  // everything else has been freed during MUIM_Cleanup already

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

  while((tag = NextTagItem(&tags)) != NULL)
  {
    switch(tag->ti_Tag)
    {
      ATTR(Address):
      {
        char *newAddress = (char *)tag->ti_Data;

        if(data->imageLoaded == TRUE)
        {
          // remove the image from the cache
          ReleaseImage(data->address, TRUE);
          data->imageLoaded = FALSE;
          D(DBF_IMAGE, "unloaded old image '%s' from '%s'", data->address, data->fileName);
        }

        if(data->address != NULL)
        {
          free(data->address);
          data->address = NULL;
        }

        if(newAddress != NULL)
          data->address = strdup(newAddress);

        // remember to relayout the image
        relayout = TRUE;

        // make the superMethod call ignore those tags
        tag->ti_Tag = TAG_IGNORE;
      }
      break;

      ATTR(Filename):
      {
        char *newFilename = (char *)tag->ti_Data;

        if(data->imageLoaded == TRUE)
        {
          // remove the image from the cache
          ReleaseImage(data->address, TRUE);
          data->imageLoaded = FALSE;
          D(DBF_IMAGE, "unloaded old image '%s' from '%s'", data->address, data->fileName);
        }

        if(data->fileName != NULL)
        {
          free(data->fileName);
          data->fileName = NULL;
        }

        if(newFilename != NULL)
          data->fileName = strdup(newFilename);

        // remember to relayout the image
        relayout = TRUE;

        // make the superMethod call ignore those tags
        tag->ti_Tag = TAG_IGNORE;
      }
      break;
    }
  }

  if(relayout == TRUE)
  {
    if(data->imageLoaded == FALSE &&
       data->address != NULL && data->address[0] != '\0' &&
       data->fileName != NULL && data->fileName[0] != '\0')
    {
      struct ImageCacheNode *node;

      if((node = ObtainImage(data->address, data->fileName, _screen(obj))) != NULL)
      {
        Object *parent;

        memcpy(&data->imageNode, node, sizeof(data->imageNode));
        data->imageLoaded = TRUE;

        if((parent = (Object*)xget(obj, MUIA_Parent)))
        {
          // New size if needed
          if(DoMethod(parent, MUIM_Group_InitChange))
            DoMethod(parent, MUIM_Group_ExitChange);

          MUI_Redraw(obj, MADF_DRAWOBJECT);
        }
      }
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
  BOOL result = FALSE;

  ENTER();

  // call the SuperMethod() first
  if(DoSuperMethodA(cl, obj, msg) &&
     data->address != NULL && data->address[0] != '\0' &&
     data->fileName != NULL && data->fileName[0] != '\0')
  {
    if(data->imageLoaded == FALSE)
    {
      struct ImageCacheNode *node;

      if((node = ObtainImage(data->address, data->fileName, _screen(obj))) != NULL)
      {
        memcpy(&data->imageNode, node, sizeof(data->imageNode));
        data->imageLoaded = TRUE;
      }
    }

    if(data->imageLoaded == TRUE && data->imageNode.dt_obj != NULL)
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
        }
        else
        {
          newWidth  = bitMapHeader->bmh_Width;
          newHeight = bitMapHeader->bmh_Height;
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

          D(DBF_GUI, "UserImage scale (w/h) from %ld/%ld to %ld/%ld", bitMapHeader->bmh_Width,
                                                                      bitMapHeader->bmh_Height,
                                                                      data->scaledWidth,
                                                                      data->scaledHeight);

          result = TRUE;
        }
      }
    }

    if(result == FALSE)
      data->scaledBitMap = NULL;
  }

  // return always true as we don't want to get that thing hidden.
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

  // now it should be fine to free the allocated bitmap
  if(data->scaledBitMap != NULL)
  {
    FreeBitMap(data->scaledBitMap);
    data->scaledBitMap = NULL;
  }

  if(data->imageLoaded == TRUE)
  {
    ReleaseImage(data->address, FALSE);
    data->imageLoaded = FALSE;
  }

  result = DoSuperMethodA(cl, obj, msg);

  RETURN(result);
  return result;
}
///
/// OVERLOAD(MUIM_Draw)
OVERLOAD(MUIM_Draw)
{
  GETDATA;

  // call the SuperMethod
  DoSuperMethodA(cl, obj, msg);

  if(((struct MUIP_Draw *)msg)->flags & MADF_DRAWOBJECT)
  {
    if(data->scaledBitMap != NULL)
      BltBitMapRastPort(data->scaledBitMap, 0, 0, _rp(obj), _mleft(obj), _mtop(obj), _mwidth(obj), _mheight(obj), (ABC|ABNC));
    else
      DoMethod(obj, MUIM_DrawBackground, _mleft(obj), _mtop(obj), _mwidth(obj), _mheight(obj), 0, 0, MUIF_NONE);
  }

  return 0;
}
///
/// OVERLOAD(MUIM_AskMinMax)
OVERLOAD(MUIM_AskMinMax)
{
  GETDATA;
  struct MUI_MinMax *mi;

  // call the SuperMethod
  DoSuperMethodA(cl, obj, msg);

  mi = ((struct MUIP_AskMinMax *)msg)->MinMaxInfo;

  if(data->scaledBitMap != NULL)
  {
    mi->MinWidth  += data->scaledWidth;
    mi->MinHeight += data->noMinHeight ? 0 : data->scaledHeight;
    mi->DefWidth  += data->scaledWidth;
    mi->DefHeight += data->scaledHeight;
    mi->MaxWidth  += data->scaledWidth;
    mi->MaxHeight += data->scaledHeight;
  }
  else
  {
    // define a default minWidth
    mi->MinWidth  += data->maxWidth;
    mi->MinHeight += data->maxHeight;
    mi->DefWidth  += data->maxWidth;
    mi->DefHeight += data->maxHeight;
    mi->MaxWidth  += data->maxWidth;
    mi->MaxHeight += data->maxHeight;
  }

  return 0;
}
///

/* Private Functions */

/* Public Methods */

