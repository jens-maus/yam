#ifndef IMAGECACHE_H
#define IMAGECACHE_H

/***************************************************************************

 YAM - Yet Another Mailer
 Copyright (C) 1995-2000 Marcel Beck
 Copyright (C) 2000-2018 YAM Open Source Team

 This program is free software; you can redistribute it and/or modify
 it under the terms of the GNU General Public License as published by
 the Free Software Foundation; either version 2 of the License, or
 (at your option) any later version.

 This program is distributed in the hope that it will be useful,
 but WITHOUT ANY WARRANTY; without even the implied warranty of
 MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 GNU General Public License for more details.

 You should have received a copy of the GNU General Public License
 along with this program; if not, write to the Free Software
 Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA

 YAM Official Support Site :  http://www.yam.ch
 YAM OpenSource project    :  http://sourceforge.net/projects/yamos/

 $Id$

***************************************************************************/

#include <exec/nodes.h>
#include <mui/TheBar_mcc.h>

#include "HashTable.h"

// forward declarations
struct Screen;
struct BitMap;

// definition of an imageCacheNode which contains
// all information of a loaded image file, including the
// loaded image datatype object
struct ImageCacheNode
{
  struct HashEntryHeader hash; // standard hash table header
  char *id;                    // pointer to the id
  char *filename;              // pointer to the filename
  Object *dt_obj;              // the datatypes object
  struct Screen *screen;       // pointer to the screen the image is mapped to
  int openCount;               // counter how often the image is now opened/used
  APTR pixelArray;             // pointer to a pixel array read by PDTM_READPIXELARRAY
  ULONG pixelFormat;           // the pixel format of the array (e.g. PBPAFMT_ARGB)
  struct BitMap *bitmap;
  PLANEPTR mask;

  ULONG width;
  ULONG height;
  ULONG depth;
  ULONG bytesPerPixel;
  ULONG bytesPerRow;

  UBYTE masking;

  BOOL initialLayout;          // did we call DTM_PROCLAYOUT before already?
  BOOL delayedDispose;         // do we wish to remove the image from the cache if openCount reaches zero?
};

// the prototypes for our public available functions
BOOL ImageCacheSetup(void);
void ImageCacheCleanup(void);
struct ImageCacheNode *ObtainImage(const char *id, const char *filename, const struct Screen *scr);
void ReleaseImage(const char *id, BOOL dispose);
BOOL IsImageInCache(const char *id);

#if defined(DEBUG)
void DumpImageCache(void);
#endif

// the prototypes/enums for our specialized Toolbarimage
// cache
enum TBType  { TBT_ReadWindow, TBT_WriteWindow, TBT_AbookWindow };
enum TBImage { TBI_Normal, TBI_Ghosted, TBI_Selected };

BOOL ToolbarCacheInit(void);
void ToolbarCacheCleanup(void);
struct MUIS_TheBar_Brush **ObtainToolbarImages(const enum TBType toolbar, const enum TBImage image);
BOOL IsToolbarInCache(const enum TBType toolbar);

#endif // IMAGECACHE_H
