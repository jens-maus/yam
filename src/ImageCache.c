/***************************************************************************

 YAM - Yet Another Mailer
 Copyright (C) 1995-2000 Marcel Beck
 Copyright (C) 2000-2015 YAM Open Source Team

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

#include <stdlib.h>
#include <string.h>

#include <datatypes/pictureclass.h>

#include <clib/alib_protos.h>

#include <proto/datatypes.h>
#include <proto/dos.h>
#include <proto/exec.h>
#include <proto/intuition.h>
#include <proto/graphics.h>
#include <proto/muimaster.h>

#if !defined(__amigaos4__)
#include <proto/cybergraphics.h>
#endif

#include "YAM.h"
#include "YAM_global.h"
#include "YAM_stringsizes.h"
#include "YAM_utilities.h"
#include "YAM_error.h"

#include "mui/ClassesExtra.h"
#include "mui/AddressBookToolbar.h"
#include "mui/ReadWindowToolbar.h"
#include "mui/WriteWindowToolbar.h"

#include "ImageCache.h"
#include "Locale.h"
#include "FileInfo.h"
#include "MUIObjects.h"
#include "extrasrc.h"

#include "Debug.h"

#ifndef PDTA_AlphaChannel
/* does the image contain alpha channel data? */
#define PDTA_AlphaChannel     (DTA_Dummy + 256)
#endif

/*** Static variables/functions ***/
/// LoadImage
// loads a specific file via datatypes.library
static BOOL LoadImage(struct ImageCacheNode *node)
{
  BOOL result = FALSE;

  ENTER();

  if(FileExists(node->filename) == TRUE)
  {
    Object *o;
    APTR oldWindowPtr;

    // tell DOS not to bother us with requesters
    oldWindowPtr = SetProcWindow((APTR)-1);

    D(DBF_IMAGE, "loading image '%s'", node->filename);

    // The source bitmap of the image must *NOT* be freed automatically by datatypes.library,
    // because we need the bitmap to be able to remap the image to another screen, if required.
    // This is very important if the depth of the screen ever changes, especially from hi/true
    // color to a color mapped screen (16/24/32 bit -> 8 bit). Keeping the source bitmap may
    // take a little bit more memory, but this is unavoidable if we don't want to reload the
    // images from disk again and again all the time.
    o = NewDTObject((char *)node->filename,
      DTA_GroupID,          GID_PICTURE,
      DTA_SourceType,       DTST_FILE,
      PDTA_DestMode,        PMODE_V43,
      PDTA_UseFriendBitMap, TRUE,
      PDTA_Remap, TRUE,
      OBP_Precision, PRECISION_EXACT,
      TAG_DONE);

    // restore window pointer.
    SetProcWindow(oldWindowPtr);

    // do all the setup/layout stuff that's necessary to get a bitmap from the dto
    // note that when using V43 datatypes, this might not be a real "struct BitMap *"
    if(o != NULL)
    {
      struct BitMapHeader *bmhd = NULL;

      D(DBF_IMAGE, "loaded image '%s' (0x%08lx)", node->filename, o);

      // Now we retrieve the bitmap header to get the width/height of the loaded object.
      // We do this now already, because getting the BMHD after the remap process seems
      // to result in wrong depth information which causes wrong display of color mapped
      // images.
      GetDTAttrs(o, PDTA_BitMapHeader, &bmhd, TAG_DONE);

      if(bmhd != NULL)
      {
        node->dt_obj = o;
        node->width = bmhd->bmh_Width;
        node->height = bmhd->bmh_Height;
        node->depth = bmhd->bmh_Depth;
        node->masking = bmhd->bmh_Masking;

        result = TRUE;
      }
      else
      {
        E(DBF_IMAGE, "wasn't able to get BitMapHeader of image '%s'", node->filename);
        DisposeDTObject(o);
      }
    }
    else
      E(DBF_IMAGE, "wasn't able to load specified image '%s', error: %ld", node->filename, IoErr());
  }
  else if(G->NoImageWarning == FALSE)
    W(DBF_IMAGE, "specified image '%s' doesn't exist.", node->filename);

  RETURN(result);
  return result;
}

///
/// RemapImage
// remaps an image loaded via datatypes.library to a specific screen
static BOOL RemapImage(struct ImageCacheNode *node, const struct Screen *scr)
{
  BOOL success = FALSE;

  ENTER();

  // remapping only works if the DT object exists
  if(node->dt_obj != NULL && scr != NULL)
  {
    // first set the new screen
    SetDTAttrs(node->dt_obj, NULL, NULL, PDTA_Screen, scr, TAG_DONE);

    // either the remap must succeed or we just reset the screen
    if(DoMethod(node->dt_obj, DTM_PROCLAYOUT, NULL, node->initialLayout) != 0)
    {
      success = TRUE;
      node->initialLayout = FALSE;
    }
  }
  else
  {
    // assume success for non-existing DT-objects
    success = TRUE;
  }

  // remember the new screen pointer
  node->screen = (struct Screen *)scr;

  RETURN(success);
  return success;
}

///
/// CreateImageCacheNode
// create a new cache node
static struct ImageCacheNode *CreateImageCacheNode(const char *id, const char *filename)
{
  struct HashEntryHeader *entry;
  struct ImageCacheNode *node = NULL;

  ENTER();

  if((entry = HashTableOperate(G->imageCacheHashTable, id, htoAdd)) != NULL)
  {
    node = (struct ImageCacheNode *)entry;
    // If the search operation returned an already initialized node, then this
    // image has been added to the cache before and we don't have to do anything
    // else. Otherwise we create a new node
    if(node->id == NULL)
    {
      BOOL success = FALSE;

      D(DBF_IMAGE, "image '%s' NOT found in cache, creating new node", id);

      // create a new node in the cache
      node->initialLayout = TRUE;
      node->delayedDispose = FALSE;
      if((node->id = strdup(id)) != NULL)
      {
        if(filename == NULL || filename[0] == '\0')
        {
          // assume success for empty filenames
          D(DBF_IMAGE, "no file given for image '%s'", id);
          success = TRUE;
        }
        else if((node->filename = strdup(filename)) != NULL)
        {
          // load the datatypes image now
          if((success = LoadImage(node)) == FALSE && FileExists(filename) == TRUE)
          {
            // the file exists, but loading it failed
            if(G->NoImageWarning == FALSE)
            {
              // show the error message only if the user did not choose to ignore
              // all warnings before
              ER_NewError(tr(MSG_ER_DATATYPE_ERROR), filename);
            }
          }
        }
      }

      if(success == FALSE)
      {
        // upon failure remove the node again
        if(node->dt_obj != NULL)
        {
          DisposeDTObject(node->dt_obj);
          node->dt_obj = NULL;
        }

        free(node->filename);
        node->filename = NULL;

        // node->id will be freed by HashTableRawRemove()
        HashTableRawRemove(G->imageCacheHashTable, entry);
        node = NULL;
      }
    }
    else
    {
      D(DBF_IMAGE, "image '%s' found in cache", id);
    }
  }

  RETURN(node);
  return(node);
}

///
/// DeleteImage
static void DeleteImage(struct ImageCacheNode *node)
{
  ENTER();

  if(node->dt_obj != NULL)
  {
    D(DBF_IMAGE, "disposing dtobject 0x%08lx of node 0x%08lx", node->dt_obj, node);
    DisposeDTObject(node->dt_obj);
    node->dt_obj = NULL;
  }

  // node->id will be free()'ed by the hash table functions! We MUST NOT free it here,
  // because this item may be addressed further on while iterating through the list.
  free(node->filename);
  node->filename = NULL;

  if(node->pixelArray != NULL)
  {
    FreeVecPooled(G->SharedMemPool, node->pixelArray);
    node->pixelArray = NULL;
  }

  LEAVE();
}

///
/// DeleteImageCacheNode
// delete a cache node
static enum HashTableOperator DeleteImageCacheNode(UNUSED struct HashTable *table, struct HashEntryHeader *entry, UNUSED ULONG number, UNUSED void *arg)
{
  struct ImageCacheNode *node = (struct ImageCacheNode *)entry;

  ENTER();

  D(DBF_STARTUP, "disposing image cache node (0x%08lx) '%s'", node, node->id);

  #if defined(DEBUG)
  if(node->openCount > 0)
    W(DBF_STARTUP, "  openCount of image cache node still %ld!!!", node->openCount);
  #endif

  DeleteImage(node);

  RETURN(htoNext);
  return htoNext;
}

///

/*** Image caching mechanisms ***/
/// ImageCacheSetup
//
BOOL ImageCacheSetup(void)
{
  BOOL result = FALSE;

  ENTER();

  #if defined(__amigaos3__)
  PictureDTBase = OpenLibrary("picture.datatype", 0);
  #endif

  if((G->imageCacheHashTable = HashTableNew(HashTableGetDefaultStringOps(), NULL, sizeof(struct ImageCacheNode), 128)) != NULL)
    result = TRUE;

  // obtain the Workbench screen pointer
  // it is safe to call UnlockPubScreen() with a NULL screen pointer
  G->workbenchScreen = LockPubScreen("Workbench");
  UnlockPubScreen(NULL, G->workbenchScreen);

  RETURN(result);
  return result;
}

///
/// ImageCacheCleanup
// for cleaning up the image cache
void ImageCacheCleanup(void)
{
  ENTER();

  if(G->imageCacheHashTable != NULL)
  {
    HashTableEnumerate(G->imageCacheHashTable, DeleteImageCacheNode, NULL);
    HashTableDestroy(G->imageCacheHashTable);
  }

  #if defined(__amigaos3__)
  if(PictureDTBase != NULL)
  {
    CloseLibrary(PictureDTBase);
    PictureDTBase = NULL;
  }
  #endif

  LEAVE();
}

///
/// ObtainImage
// for receiveing the imagenode object or loading it
// immediately.
struct ImageCacheNode *ObtainImage(const char *id, const char *filename, const struct Screen *scr)
{
  struct ImageCacheNode *node;

  ENTER();

  D(DBF_IMAGE, "obtain image '%s' from cache", id);

  if((node = CreateImageCacheNode(id, filename)) == NULL)
  {
    // check if the file exists or not.
    if(IsStrEmpty(filename) == FALSE && FileExists(filename) == FALSE)
    {
      if(G->NoImageWarning == FALSE)
      {
        char *path;

        if((path = strdup(filename)) != NULL)
        {
          char *p;

          if((p = PathPart(path)) != NULL)
            *p = '\0';

          ER_NewError(tr(MSG_ER_IMAGEOBJECT_WARNING), FilePart(filename), path);

          free(path);
        }
      }
    }
  }

  // do a remapping of the image if necessary
  if(node != NULL && scr != NULL)
  {
    D(DBF_IMAGE, "setting up image '%s' for screen 0x%08lx", id, scr);

    // we found a previously loaded node in the cache
    // now we need to remap it to the screen, if not yet done
    if(node->screen != scr)
    {
      // remap the image
      // this cannot fail for NULL screens
      if(RemapImage(node, scr) == TRUE)
      {
        // check if the image is to be displayed on a new screen
        if(scr != NULL && node->dt_obj != NULL)
        {
          node->bitmap = NULL;

          // if we are asked to display a hi/truecolor image on a CLUT screen, then we
          // let datatypes.library do the dirty dithering work
          if(node->depth > 8 && GetBitMapAttr(scr->RastPort.BitMap, BMA_DEPTH) <= 8)
            node->depth = 8;

          #if defined(__amigaos4__) || defined(__MORPHOS__) || defined(__AROS__)
          // OS4 and MorphOS can handle the alpha channel correctly
          if(node->pixelArray == NULL)
          #else
          // for OS3 we check for CGX V45+ and picture.datatype V46+
          // older versions cannot handle the alpha channel correctly
          if(CyberGfxBase != NULL && LIB_VERSION_IS_AT_LEAST(CyberGfxBase, 45, 0) == TRUE &&
             PictureDTBase != NULL && LIB_VERSION_IS_AT_LEAST(PictureDTBase, 46, 0) == TRUE &&
             node->pixelArray == NULL)
          #endif
          {
            BOOL hasAlphaChannel = FALSE;

            // the datatypes system tells us about the alpha channel either
            // by setting the correct masking type or by the PDTA_AlphaChannel
            // attribute.
            if(node->masking == mskHasAlpha)
              hasAlphaChannel = TRUE;
            else
            {
              ULONG alphaChannel = 0;

              GetDTAttrs(node->dt_obj, PDTA_AlphaChannel, &alphaChannel, TAG_DONE);
              if(alphaChannel != 0)
                hasAlphaChannel = TRUE;
            }

            D(DBF_IMAGE, "image '%s' has %ld bit depth and %s alpha channel (%ld)", node->id, node->depth, (hasAlphaChannel == TRUE) ? "AN" : "NO", node->masking);

            // check if the bitmap may have alpha channel data or not.
            if(node->depth > 8 && node->masking != mskHasTransparentColor)
            {
              node->bytesPerPixel = (hasAlphaChannel == TRUE) ? 4 : 3;
              node->bytesPerRow = node->width * node->bytesPerPixel;
              node->pixelFormat = (hasAlphaChannel == TRUE) ? PBPAFMT_ARGB : PBPAFMT_RGB;

              if((node->pixelArray = AllocVecPooled(G->SharedMemPool, node->bytesPerRow * node->height)) != NULL)
              {
                BOOL result;

                // perform a PDTM_READPIXELARRAY operation
                // for writing the image data of the image in our pixelArray
                result = DoMethod(node->dt_obj, PDTM_READPIXELARRAY, node->pixelArray, node->pixelFormat, node->bytesPerRow,
                                                                     0, 0, node->width, node->height);
                #if defined(__MORPHOS__)
                // MorphOS < v2.0 doesn't return a valid value for the PDTM_READPIXELARRAY method
                // so ignore it
                result = TRUE;
                #endif

                if(result == FALSE)
                {
                  W(DBF_IMAGE, "PDTM_READPIXELARRAY on image '%s' failed!", node->id);

                  FreeVecPooled(G->SharedMemPool, node->pixelArray);
                  node->pixelArray = NULL;
                }
                else
                  D(DBF_IMAGE, "PDTM_READPIXELARRAY on image '%s' succeeded", node->id);
              }
            }
            else
              D(DBF_IMAGE, "PDTM_READPIXELARRAY not required - no alpha data in image '%s'", node->id);
          }

          // get the normal bitmaps supplied by datatypes.library if either this is
          // an 8bit image or we could not get the hi/truecolor pixel data
          #if defined(__amigaos4__) || defined(__MORPHOS__) || defined(__AROS__)
          if(node->pixelArray == NULL)
          #else
          if(node->pixelArray == NULL || CyberGfxBase == NULL)
          #endif
          {
            node->bytesPerPixel = 1;
            node->bytesPerRow = node->width;
            node->pixelFormat = PBPAFMT_LUT8;
            node->pixelArray = NULL;
          }

          // get the image bitmap
          GetDTAttrs(node->dt_obj, PDTA_DestBitMap, &node->bitmap, TAG_DONE);
          if(node->bitmap == NULL)
            GetDTAttrs(node->dt_obj, PDTA_BitMap, &node->bitmap, TAG_DONE);

          // get the mask plane for transparency display of the image if it exists
          if(node->masking == mskHasMask || node->masking == mskHasTransparentColor)
            GetDTAttrs(node->dt_obj, PDTA_MaskPlane, &node->mask, TAG_DONE);

          if(node->mask == NULL)
            D(DBF_IMAGE, "no maskplane bitmask found for image '%s'", id);
        }
      }
      else
      {
        D(DBF_IMAGE, "couldn't remap image '%s' to screen 0x%08lx", id, scr);

        // let this call fail if we cannot remap the image
        node = NULL;
      }
    }
  }

  // increase the counter if everything went fine
  if(node != NULL)
    node->openCount++;

  RETURN(node);
  return node;
}

///
/// ReleaseImage
// for releasing an imagenode properly
void ReleaseImage(const char *id, BOOL dispose)
{
  struct HashEntryHeader *entry;

  ENTER();

  entry = HashTableOperate(G->imageCacheHashTable, id, htoLookup);
  if(HASH_ENTRY_IS_LIVE(entry))
  {
    struct ImageCacheNode *node = (struct ImageCacheNode *)entry;

    if(node->openCount > 0)
    {
      node->openCount--;
      D(DBF_IMAGE, "reduced open count of image '%s' (%s) to %ld", id, node->filename, node->openCount);
    }
    else if(dispose == FALSE)
      E(DBF_IMAGE, "couldn't reduce open count (%ld) of image '%s' (%s)", node->openCount, id, node->filename);

    if(node->openCount == 0)
    {
      if(node->screen != NULL && node->screen != G->workbenchScreen)
      {
        // enforce a disposing in case the image was remapped to a colormapped screen
        // picture.datatypes seems to keep a pointer to the screen even if that was set to
        // NULL before. This pointer will then later be accessed although the screen most
        // probably does not exist anymore and hence causes crashes. The Workbench screen
        // is considered to be safe as it is closed *very* rarely, if at all. See ticket
        // #389 for details.
        if(dispose == FALSE && GetBitMapAttr(node->screen->RastPort.BitMap, BMA_DEPTH) <= 8)
        {
          D(DBF_IMAGE, "enforcing dispose due to colormapped screen");
          dispose = TRUE;
        }
      }

      if(dispose == TRUE || node->delayedDispose == TRUE)
      {
        D(DBF_IMAGE, "removing image '%s' from cache", node->id);

        DeleteImage(node);
        // node->id will be freed by HashTableRawRemove()

        // remove the image from the cache
        HashTableRawRemove(G->imageCacheHashTable, entry);
      }
    }
    else
    {
      // The image is still in use although it should be removed from the cache.
      // To accomplish this we remember this and remove it as soon as the open
      // counter reaches zero.
      if(dispose == TRUE)
      {
        node->delayedDispose = TRUE;

        D(DBF_IMAGE, "flagged as delayedDispose");
      }
    }
  }
  else
    E(DBF_IMAGE, "image '%s' not found in cache", id);

  LEAVE();
}

///
/// IsImageInCache
// returns TRUE if the specified image filename is found to
// be in the cache - may it be loaded or unloaded.
BOOL IsImageInCache(const char *id)
{
//  struct MinNode *curNode;
  struct HashEntryHeader *entry;
  BOOL result = FALSE;

  ENTER();

  D(DBF_IMAGE, "find image '%s' in cache", id);

  // look up the image named 'id' in our hash table
  entry = HashTableOperate(G->imageCacheHashTable, id, htoLookup);
  if(HASH_ENTRY_IS_LIVE(entry))
  {
    D(DBF_IMAGE, "found node %08lx,'%s','%s'", entry, ((struct ImageCacheNode *)entry)->id, ((struct ImageCacheNode *)entry)->filename);
    result = TRUE;
  }

  RETURN(result);
  return result;
}

///
/// DumpImageCache
// print out the complete image cache with all necessary information
#if defined(DEBUG)
static enum HashTableOperator DumpImageCacheNode(UNUSED struct HashTable *table, struct HashEntryHeader *entry, UNUSED ULONG number, UNUSED void *arg)
{
  struct ImageCacheNode *node = (struct ImageCacheNode *)entry;

  ENTER();

  D(DBF_IMAGE, "  node %08lx", node);
  D(DBF_IMAGE, "    hash key         %08lx", node->hash.keyHash);
  D(DBF_IMAGE, "    id               '%s'", node->id);
  D(DBF_IMAGE, "    file             '%s'", node->filename);
  D(DBF_IMAGE, "    openCount        %ld", node->openCount);
  D(DBF_IMAGE, "    dtobj            %08lx", node->dt_obj);
  D(DBF_IMAGE, "    screen           %08lx", node->screen);
  D(DBF_IMAGE, "    width            %ld", node->width);
  D(DBF_IMAGE, "    height           %ld", node->height);
  D(DBF_IMAGE, "    delayed dispose  %ld", node->delayedDispose);

  RETURN(htoNext);
  return htoNext;
}

void DumpImageCache(void)
{
  ENTER();

  D(DBF_IMAGE, "current image cache contents");
  HashTableEnumerate(G->imageCacheHashTable, DumpImageCacheNode, NULL);

  LEAVE();
}
#endif

///

/*** TheBar toolbar image cache mechanisms ***/
/// ToolbarCacheInit
// for initializing the toolbar caching
BOOL ToolbarCacheInit(void)
{
  BOOL result = FALSE;

  ENTER();

  G->ReadToolbarCacheObject = ReadWindowToolbarObject, End;
  D(DBF_IMAGE, "init readwindow toolbar: %08lx", G->ReadToolbarCacheObject);

  G->WriteToolbarCacheObject = WriteWindowToolbarObject, End;
  D(DBF_IMAGE, "init writewindow toolbar: %08lx", G->WriteToolbarCacheObject);

  G->AbookToolbarCacheObject = AddressBookToolbarObject, End;
  D(DBF_IMAGE, "init abookwindow toolbar: %08lx", G->AbookToolbarCacheObject);


  if(G->ReadToolbarCacheObject != NULL  &&
     G->WriteToolbarCacheObject != NULL &&
     G->AbookToolbarCacheObject != NULL)
  {
    result = TRUE;
  }

  RETURN(result);
  return result;
}

///
/// ToolbarCacheCleanup
// for cleaning up the toolbar caches
void ToolbarCacheCleanup(void)
{
  ENTER();

  if(G->AbookToolbarCacheObject)
  {
    MUI_DisposeObject(G->AbookToolbarCacheObject);
    G->AbookToolbarCacheObject = NULL;
  }

  if(G->WriteToolbarCacheObject)
  {
    MUI_DisposeObject(G->WriteToolbarCacheObject);
    G->WriteToolbarCacheObject = NULL;
  }

  if(G->ReadToolbarCacheObject)
  {
    MUI_DisposeObject(G->ReadToolbarCacheObject);
    G->ReadToolbarCacheObject = NULL;
  }

  LEAVE();
}

///
/// ObtainToolbarImages
// for receiveing the various images (normal/ghosted/selected) of a
// cached toolbar object
struct MUIS_TheBar_Brush **ObtainToolbarImages(const enum TBType toolbar, const enum TBImage image)
{
  struct MUIS_TheBar_Brush **images = NULL;
  Object *toolbarCacheObject = NULL;

  ENTER();

  switch(toolbar)
  {
    case TBT_ReadWindow:
      toolbarCacheObject = G->ReadToolbarCacheObject;
    break;

    case TBT_WriteWindow:
      toolbarCacheObject = G->WriteToolbarCacheObject;
    break;

    case TBT_AbookWindow:
      toolbarCacheObject = G->AbookToolbarCacheObject;
    break;
  }
  D(DBF_IMAGE, "cached toolbar object %ld %08lx", toolbar, toolbarCacheObject);

  if(toolbarCacheObject != NULL && xget(toolbarCacheObject, MUIA_TheBar_TextOnly) == FALSE)
  {
    switch(image)
    {
      case TBI_Normal:
        images = (struct MUIS_TheBar_Brush **)xget(toolbarCacheObject, MUIA_TheBar_Images);
      break;

      case TBI_Ghosted:
        images = (struct MUIS_TheBar_Brush **)xget(toolbarCacheObject, MUIA_TheBar_DisImages);
      break;

      case TBI_Selected:
        images = (struct MUIS_TheBar_Brush **)xget(toolbarCacheObject, MUIA_TheBar_SelImages);
      break;
    }
  }
  D(DBF_IMAGE, "cached toolbar images %ld %08lx", toolbar, images);

  RETURN(images);
  return images;
}

///
/// IsToolbarInCache
// returns TRUE if the specified toolbar is found to
// be in the cache
BOOL IsToolbarInCache(const enum TBType toolbar)
{
  BOOL result = FALSE;

  ENTER();

  switch(toolbar)
  {
    case TBT_ReadWindow:
      result = (G->ReadToolbarCacheObject != NULL);
    break;

    case TBT_WriteWindow:
      result = (G->WriteToolbarCacheObject != NULL);
    break;

    case TBT_AbookWindow:
      result = (G->AbookToolbarCacheObject != NULL);
    break;
  }

  RETURN(result);
  return result;
}

///

