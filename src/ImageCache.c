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

#include "extrasrc.h"

#include "YAM.h"
#include "YAM_locale.h"
#include "YAM_stringsizes.h"
#include "YAM_utilities.h"

#include "classes/Classes.h"

#include "ImageCache.h"

#include "Debug.h"

/*** Static variables/functions ***/
/// imageFileArray[]
// array with all our private image filenames we define in the
// current imagelayout. Please note that as soon as you change something
// here you also have to alter the IMGLAYOUT_VERSION in the ImageCache.h
// headerfile.
static const char *imageFileArray[MAX_IMAGES] =
{
  // Status information images
  "status_unread",   "status_old",    "status_forward",  "status_reply",
  "status_waitsend", "status_error",  "status_hold",     "status_sent",
  "status_new",      "status_delete", "status_download", "status_group",
  "status_urgent",   "status_attach", "status_report",   "status_crypt",
  "status_signed",   "status_mark",   "status_spam",

  // Default images for the folder list
  "folder_fold",     "folder_unfold",       "folder_incoming", "folder_incoming_new",
  "folder_outgoing", "folder_outgoing_new", "folder_trash",    "folder_trash_new",
  "folder_sent",     "folder_spam",         "folder_spam_new",

  // Images for the YAM configuration window
  "config_firststep", "config_firststep_big",
  "config_network",   "config_network_big",
  "config_newmail",   "config_newmail_big",
  "config_filters",   "config_filters_big",
  "config_spam",      "config_spam_big",
  "config_read",      "config_read_big",
  "config_write",     "config_write_big",
  "config_answer",    "config_answer_big",
  "config_signature", "config_signature_big",
  "config_lists",     "config_lists_big",
  "config_security",  "config_security_big",
  "config_start",     "config_start_big",
  "config_mime",      "config_mime_big",
  "config_abook",     "config_abook_big",
  "config_scripts",   "config_scripts_big",
  "config_misc",      "config_misc_big",
  "config_lookfeel",  "config_lookfeel_big",
  "config_update",    "config_update_big",
};

///
/// LoadImage()
// loads a specific file via datatypes.library
static BOOL LoadImage(struct imageCacheNode *node, const char *filename)
{
  BOOL result = FALSE;

  ENTER();

  if(FileExists(filename))
  {
    Object *o;
    struct Process *myproc;
    APTR oldwindowptr;

    // tell DOS not to bother us with requesters
    myproc = (struct Process *)FindTask(NULL);
    oldwindowptr = myproc->pr_WindowPtr;
    myproc->pr_WindowPtr = (APTR)-1;

    D(DBF_IMAGE, "loading image '%s'", filename);

    // The source bitmap of the image must *NOT* be freed automatically by datatypes.library,
    // because we need the bitmap to be able to remap the image to another screen, if required.
    // This is very important if the depth of the screen ever changes, especially from hi/true
    // color to a color mapped screen (16/24/32 bit -> 8 bit). Keeping the source bitmap may
    // take a little bit more memory, but this is unavoidable if we don't want to reload the
    // images from disk again and again all the time.
    o = NewDTObject((char *)filename, DTA_GroupID,          GID_PICTURE,
                                      DTA_SourceType,       DTST_FILE,
                                      OBP_Precision,        PRECISION_EXACT,
                                      PDTA_DestMode,        PMODE_V43,
                                      PDTA_UseFriendBitMap, TRUE,
                                      PDTA_Remap,           TRUE,
                                      TAG_DONE);

    myproc->pr_WindowPtr = oldwindowptr; // restore window pointer.

    // do all the setup/layout stuff that's necessary to get a bitmap from the dto
    // note that when using V43 datatypes, this might not be a real "struct BitMap *"
    if(o != NULL)
    {
      struct FrameInfo fri;

      memset(&fri, 0, sizeof(struct FrameInfo));

      DoMethod(o, DTM_FRAMEBOX, NULL, (ULONG)&fri, (ULONG)&fri, sizeof(struct FrameInfo), 0);

      if(fri.fri_Dimensions.Depth > 0)
      {
        D(DBF_IMAGE, "loaded image '%s' (0x%08lx)", filename, o);

        node->dt_obj = o;
        node->width = fri.fri_Dimensions.Width;
        node->height = fri.fri_Dimensions.Height;

        result = TRUE;
      }
      else
      {
        E(DBF_IMAGE, "wasn't able to get frame box of image '%s'", filename);
        DisposeDTObject(o);
      }
    }
    else
      E(DBF_IMAGE, "wasn't able to load specified image '%s'. error: %d", filename, IoErr());
  }
  else
    W(DBF_IMAGE, "specified image '%s' doesn't exist.", filename);

  RETURN(result);
  return result;
}

///
/// RemapImage()
// remaps an image loaded via datatypes.library to a specific screen
static BOOL RemapImage(struct imageCacheNode *node, const struct Screen *scr)
{
  BOOL success = FALSE;

  ENTER();

  // first set the new screen
  SetDTAttrs(node->dt_obj, NULL, NULL, PDTA_Screen, scr,
                                       TAG_DONE);

  // either the remap must succeed or we just reset the screen
  if(DoMethod(node->dt_obj, DTM_PROCLAYOUT, NULL, 1) != 0 || scr == NULL)
    success = TRUE;

  // remember the new screen pointer
  node->screen = (struct Screen *)scr;

  RETURN(success);
  return success;
}

///
/// CreateImageCacheNode()
// create a new cache node
static struct imageCacheNode *CreateImageCacheNode(const char *filename)
{
  struct imageCacheNode *node;

  ENTER();

  if((node = (struct imageCacheNode*)calloc(sizeof(struct imageCacheNode), 1)) != NULL)
  {
    // load the datatypes image now
    if((LoadImage(node, filename)))
    {
      // success
      node->filename = strdup(filename);

      // add the new node to the global list
      AddTail((struct List *)&G->imageCacheList, (struct Node *)&node->node);
    }
    else
    {
      // failure
      MUI_Request(G->App, NULL, 0, tr(MSG_ER_LOADDT_TITLE),
                                   tr(MSG_ER_LOADDT_BUTTON),
                                   tr(MSG_ER_LOADDT_ERROR), filename);

      free(node);
      node = NULL;
    }
  }

  RETURN(node);
  return(node);
}

///

/*** Image caching mechanisms ***/
/// ImageCacheInit()
// for initializing the image caching
BOOL ImageCacheInit(const char *imagePath)
{
  BOOL success = TRUE;
  int i;

  ENTER();

  // now we walk through our imageFileArray and populate our
  // imagecachelist
  for(i = 0; i < MAX_IMAGES; i++)
  {
    char filebuf[SIZE_PATHFILE];

    strmfp(filebuf, imagePath, imageFileArray[i]);

    // check if the file exists or not.
    if(FileExists(filebuf) == FALSE)
    {
      if(G->NoImageWarning == FALSE)
      {
        int reqResult;

        if((reqResult = MUI_Request(G->App, NULL, 0, tr(MSG_ER_IMAGEOBJECT_TITLE),
                                                     tr(MSG_ER_EXITIGNOREALL),
                                                     tr(MSG_ER_IMAGEOBJECT),
                                                     imageFileArray[i], imagePath)))
        {
          if(reqResult == 2)
            G->NoImageWarning = TRUE;
          else
          {
            success = FALSE;
            break;
          }
        }
      }
    }
    else
    {
      // prepare the imageCacheNode
      struct imageCacheNode *node;

      if((node = CreateImageCacheNode(filebuf)) != NULL)
      {
        D(DBF_STARTUP, "init imageCacheNode 0x%08lx of file '%s'", node, node->filename);
      }
      else
      {
        success = FALSE;
        break;
      }
    }
  }

  RETURN(success);
  return success;
}

///
/// ImageCacheCleanup()
// for cleaning up the image cache
void ImageCacheCleanup(void)
{
  struct imageCacheNode *node;

  ENTER();

  while((node = (struct imageCacheNode*)RemHead((struct List *)&G->imageCacheList)) != NULL)
  {
    D(DBF_STARTUP, "disposing image cache node (0x%08lx) '%s'", node, node->filename ? node->filename : "<NULL>");

    #if defined(DEBUG)
    if(node->openCount > 0)
      W(DBF_STARTUP, "  openCount of image cache node still %d!!!", node->openCount);
    #endif

    if(node->dt_obj != NULL)
    {
      D(DBF_STARTUP, "  disposing dtobject 0x%08lx of node 0x%08lx", node->dt_obj, node);
      RemapImage(node, NULL);
      DisposeDTObject(node->dt_obj);
    }

    if(node->filename != NULL)
      free(node->filename);

    free(node);
  }

  LEAVE();
}

///
/// ObtainImage()
// for receiveing the imagenode object or loading it
// immediately.
struct imageCacheNode *ObtainImage(const char *filename, const struct Screen *scr)
{
  struct imageCacheNode *result = NULL;
  struct MinNode *curNode;
  BOOL absoluteFilePath = FALSE;

  ENTER();

  D(DBF_IMAGE, "trying to obtain image '%s' from cache", filename);

  // we try to find out first if we try to find an image
  // according to its absolute file path or just by the filename
  if(strpbrk(filename, ":/"))
    absoluteFilePath = TRUE;

  // walk through our global imageCacheList and try to find the specified
  // image file.
  for(curNode = G->imageCacheList.mlh_Head; curNode->mln_Succ; curNode = curNode->mln_Succ)
  {
    struct imageCacheNode *node = (struct imageCacheNode *)curNode;

    if(node->filename != NULL)
    {
      char *file;

      if(absoluteFilePath)
        file = node->filename;
      else
        file = FilePart(node->filename);

      if(file != NULL && stricmp(filename, file) == 0)
      {
        // we found the node in the cache
        result = node;
        break;
      }
    }
  }

  if(result == NULL)
  {
    D(DBF_IMAGE, "creating new cache node for image '%s'", filename);
    result = CreateImageCacheNode(filename);
  }

  // do a remapping of the image if necessary
  if(result != NULL && scr != NULL)
  {
    D(DBF_IMAGE, "setting up image '%s' for screen 0x%08lx", filename, scr);

    // we found a previously loaded node in the cache
    // now we need to remap it to the screen, if not yet done
    if(result->screen != scr)
    {
      // remap the image
      // this cannot fail for NULL screens
      if(RemapImage(result, scr))
      {
        // check if the image is to be displayed on a new screen
        if(scr != NULL)
        {
          struct BitMapHeader *bmhd = NULL;

          // now we retrieve the bitmap header to
          // get the width/height of the loaded object
          GetDTAttrs(result->dt_obj, PDTA_BitMapHeader, &bmhd,
                                     TAG_DONE);

          if(bmhd != NULL)
          {
            result->width = bmhd->bmh_Width;
            result->height = bmhd->bmh_Height;
          }
          else
            W(DBF_IMAGE, "couldn't find BitMap header of file '%s'", filename);
        }
      }
      else
      {
        D(DBF_IMAGE, "couldn't remap image '%s' to screen 0x%08lx", filename, scr);

        // let this call fail if we cannot remap the image
        result = NULL;
      }
    }
  }

  // increase the counter if everything went fine
  if(result != NULL)
    result->openCount++;

  RETURN(result);
  return result;
}

///
/// DisposeImage()
// for disposing an imagenode properly
void DisposeImage(struct imageCacheNode *node)
{
  ENTER();

  if(node != NULL)
  {
    if(node->openCount > 0)
    {
      node->openCount--;
      D(DBF_IMAGE, "reduced open count of image '%s' to %d", node->filename != NULL ? node->filename : "<NULL>", node->openCount);

      if(node->openCount == 0)
      {
        if(node->dt_obj != NULL)
        {
          // clear the DT object's screen pointer
          // this always succeeds, hence no need to check the result
          RemapImage(node, NULL);
        }
      }
    }
    else
      E(DBF_IMAGE, "couldn't reduce open count (%d) of 0x%08lx (%s)", node ? node->openCount : -1, node, (node && node->filename) ? node->filename : "<NULL>");
  }
  else
    E(DBF_IMAGE, "couldn't reduce open count (%d) of 0x%08lx (%s)", node ? node->openCount : -1, node, (node && node->filename) ? node->filename : "<NULL>");

  LEAVE();
}

///
/// IsImageInCache()
// returns TRUE if the specified image filename is found to
// be in the cache - may it be loaded or unloaded.
BOOL IsImageInCache(const char *filename)
{
  struct MinNode *curNode;
  BOOL result = FALSE;

  ENTER();

  D(DBF_IMAGE, "find image '%s' in cache", filename);

  // walk through our global imageCacheList and try to find the specified
  // image file.
  for(curNode = G->imageCacheList.mlh_Head; curNode->mln_Succ; curNode = curNode->mln_Succ)
  {
    struct imageCacheNode *node = (struct imageCacheNode *)curNode;

    if(node->filename != NULL && stricmp(filename, FilePart(node->filename)) == 0)
    {
      D(DBF_IMAGE, "found node %08lx,'%s','%s'", node, node->filename, FilePart(node->filename));
      result = TRUE;
      break;
    }
  }

  D(DBF_IMAGE, "image file '%s' was %s in image cache at node %08lx", filename, (result ? "FOUND" : "NOT FOUND"), curNode);

  RETURN(result);
  return result;
}

///

/*** TheBar toolbar image cache mechanisms ***/
/// ToolbarCacheInit()
// for initializing the toolbar caching
BOOL ToolbarCacheInit(const char *imagePath)
{
  BOOL result = FALSE;
  ENTER();

  G->ReadToolbarCacheObject = ReadWindowToolbarObject,
                                MUIA_TheBar_PicsDrawer, imagePath,
                              End;

  D(DBF_STARTUP, "init readwindow toolbar: %08lx", G->ReadToolbarCacheObject);

  G->WriteToolbarCacheObject = WriteWindowToolbarObject,
                                 MUIA_TheBar_PicsDrawer, imagePath,
                               End;

  D(DBF_STARTUP, "init writewindow toolbar: %08lx", G->WriteToolbarCacheObject);

  G->AbookToolbarCacheObject = AddrBookToolbarObject,
                                 MUIA_TheBar_PicsDrawer, imagePath,
                               End;

  D(DBF_STARTUP, "init abookwindow toolbar: %08lx", G->AbookToolbarCacheObject);


  if(G->ReadToolbarCacheObject  &&
     G->WriteToolbarCacheObject &&
     G->AbookToolbarCacheObject)
  {
    result = TRUE;
  }

  RETURN(result);
  return result;
}

///
/// ToolbarCacheCleanup()
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
/// ObtainToolbarImages()
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

  if(toolbarCacheObject != NULL)
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

  RETURN(images);
  return images;
}

///
/// IsToolbarInCache()
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
