#ifndef IMAGECACHE_H
#define IMAGECACHE_H

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
 MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 GNU General Public License for more details.

 You should have received a copy of the GNU General Public License
 along with this program; if not, write to the Free Software
 Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA

 YAM Official Support Site :  http://www.yam.ch
 YAM OpenSource project    :  http://sourceforge.net/projects/yamos/

 $Id: YAM.c 2046 2006-03-13 12:13:58Z damato $

***************************************************************************/

// definition of an imageCacheNode which contains
// all information of a loaded image file, including the
// loaded image datatype object
struct imageCacheNode
{
  struct MinNode node;    // the node for adding it in a MinList
  char *filename;         // pointer to the filename
  Object *dt_obj;         // the datatypes object
  struct Screen *screen;  // pointer to the screen the image is mapped to
  int openCount;          // counter how often the image is now opened/used

  ULONG width;
  ULONG height;
  ULONG screenDepth;
};

// the prototypes for our public available functions
BOOL ImageCacheInit(const char *path);
void ImageCacheCleanup(void);
struct imageCacheNode *ObtainImage(char *filename, const struct Screen *scr);
void DisposeImage(struct imageCacheNode *node);
BOOL IsImageInCache(const char *filename);

// the imagelayout define which defines the current "version" of
// the image layout we are currently using
// Please note that as soon as you change something to the internal
// image array of the image cache you have to bump the VERSION accordingly
// to make sure the user is reminded of having the correct image layout
// installed or not.
//
// VERSIONS:
// 1 : <= YAM 2.3
// 2 : == YAM 2.4-2.4p1
// 3 : >= YAM 2.5
//
#define IMGLAYOUT_VERSION 3

#endif // IMAGECACHE_H
