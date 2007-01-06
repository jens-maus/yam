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

#include "extrasrc.h"

#include "YAM.h"
#include "YAM_locale.h"
#include "YAM_stringsizes.h"
#include "YAM_utilities.h"

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
  "folder_outgoing", "folder_outgoing_new", "folder_deleted",  "folder_deleted_new",
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
  "config_update",    "config_update_big",
};

///
/// LoadDTImage()
// loads a specific file via datatypes.library
static Object *LoadDTImage(char *filename, const struct Screen *scr)
{
  Object *o;
  struct Process *myproc;
  APTR oldwindowptr;

  ENTER();

  if(FileExists(filename) == FALSE)
  {
    W(DBF_IMAGE, "specified image file '%s' doesn't exist.", filename);

    RETURN(NULL);
    return NULL;
  }

  // tell DOS not to bother us with requesters
  myproc = (struct Process *)FindTask(NULL);
  oldwindowptr = myproc->pr_WindowPtr;
  myproc->pr_WindowPtr = (APTR)-1;

  o = NewDTObject(filename,
                  DTA_GroupID,            GID_PICTURE,
                  OBP_Precision,          PRECISION_EXACT,
                  PDTA_Screen,            scr,
                  PDTA_FreeSourceBitMap,  TRUE,
                  PDTA_DestMode,          PMODE_V43,
                  PDTA_UseFriendBitMap,   TRUE,
                  TAG_DONE);
  
  myproc->pr_WindowPtr = oldwindowptr; // restore window pointer.
  
  // do all the setup/layout stuff that's necessary to get a bitmap from the dto
  // note that when using V43 datatypes, this might not be a real "struct BitMap *"
  if(o)
  {
    struct FrameInfo fri;

    memset(&fri, 0, sizeof(struct FrameInfo));

    DoMethod(o, DTM_FRAMEBOX, NULL, (ULONG)&fri, (ULONG)&fri, sizeof(struct FrameInfo), 0);
  
    if(fri.fri_Dimensions.Depth > 0)
    {
      if(DoMethod(o, DTM_PROCLAYOUT, NULL, 1))
      {
        D(DBF_STARTUP, "loaded/processed image file '%s' (0x%08lx)", filename, o);

        RETURN(o);
        return o;
      }
    }
    DisposeDTObject(o);
  }

  E(DBF_IMAGE, "wasn't able to load/process specified image file '%s'", filename);

  RETURN(NULL);
  return NULL;
}

///

/*** Image caching mechanisms ***/
/// ImageCacheInit()
// for initializing the image caching
BOOL ImageCacheInit(const char *imagePath)
{
  int i;

  ENTER();

  // now we walk through our imageFileArray and populate our
  // imagecachelist
  for(i=0; i < MAX_IMAGES; i++)
  {
    char filebuf[SIZE_PATHFILE];

    strmfp(filebuf, imagePath, imageFileArray[i]);

    // check if the file exists or not.
    if(FileExists(filebuf) == FALSE)
    {
      if(G->NoImageWarning == FALSE)
      {
        int reqResult;

        if((reqResult = MUI_Request(G->App, NULL, 0, GetStr(MSG_ER_IMAGEOBJECT_TITLE),
                                                     GetStr(MSG_ER_EXITIGNOREALL),
                                                     GetStr(MSG_ER_IMAGEOBJECT),
                                                     imageFileArray[i], imagePath)))
        {
          if(reqResult == 2)
            G->NoImageWarning = TRUE;
          else
          {
            RETURN(FALSE);
            return FALSE;
          }
        }
      }
    }
    else
    {
      // prepare the imageCacheNode
      struct imageCacheNode *node = calloc(sizeof(struct imageCacheNode), 1);
      if(node)
      {
        node->filename = strdup(filebuf);

        D(DBF_STARTUP, "init imageCacheNode 0x%08lx of file '%s'", node, node->filename);

        AddTail((struct List *)&G->imageCacheList, (struct Node *)&node->node);
      }
      else
      {
        RETURN(FALSE);
        return FALSE;
      }
    }
  }

  RETURN(TRUE);
  return TRUE;
}

///
/// ImageCacheCleanup()
// for cleaning up the image cache
void ImageCacheCleanup(void)
{
  struct imageCacheNode *node;

  ENTER();

  while((node = (struct imageCacheNode*)RemTail((struct List *)&G->imageCacheList)))
  {
    D(DBF_STARTUP, "disposing image cache node (0x%08lx) '%s'", node, node->filename ? node->filename : "<NULL>");

    #if defined(DEBUG)
    if(node->openCount > 0)
      W(DBF_STARTUP, "  openCount of image cache node still %d!!!", node->openCount);
    #endif

    if(node->dt_obj)
    {
      D(DBF_STARTUP, "  disposing dtobject 0x%08lx of node 0x%08lx", node->dt_obj, node);
      DisposeDTObject(node->dt_obj);
    }

    if(node->filename)
      free(node->filename);

    free(node);
  }

  LEAVE();
}

///
/// ObtainImage()
// for receiveing the imagenode object or loading it
// immediately.
struct imageCacheNode *ObtainImage(char *filename, const struct Screen *scr)
{
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

      if(file && stricmp(filename, file) == 0)
      {
        ULONG screenDepth;

        // get the screen's depth
        screenDepth = GetBitMapAttr(scr->RastPort.BitMap, BMA_DEPTH);

        // now we check if there exists already a valid datatype
        // object and if it is also already remapped for the screen
        // we are requesting it.
        if(node->dt_obj != NULL && node->screen != scr)
        {
          SHOWVALUE(DBF_IMAGE, screenDepth);
          SHOWVALUE(DBF_IMAGE, node->screenDepth);

          // the image must be reloaded and remapped, if:
          // - the screen depth stays <= 8 bit, or
          // - the screen depth changes from <=8 bit to >8 bit, or
          // - the screen depth changes from >8 bit to <=8 bit
          // picture.datatype is supposed to return an RGB bitmap for RGB screens.
          // This bitmap can be used on any RGB screen without remapping, since there is
          // no palette for these screens. Only CLUT screens have a palette which must
          // be respected and which can be changed at any time by other applications.
          // Also the screen's depth must be remembered separately, because if the screen
          // has changed the old screen pointer may no longer be valid and hence must not
          // be accessed anymore.
          if((node->screenDepth <= 8 && screenDepth <= 8) ||
             (node->screenDepth <= 8 && screenDepth >  8) ||
             (node->screenDepth >  8 && screenDepth <= 8))
          {
            W(DBF_IMAGE, "current screen doesn't match dtobject (0x%08lx). reloading...", node->dt_obj);

            // dispose the dt_obj and let us reload it afterwards
            DisposeDTObject(node->dt_obj);
            node->dt_obj = NULL;
          }
        }

        // if the image object wasn't loaded yet, we do it now
        if(node->dt_obj == NULL)
        {
          // load the datatypes image now
          if((node->dt_obj = LoadDTImage(node->filename, scr)))
          {
            struct BitMapHeader *bmhd;

            // now we retrieve the bitmap header to
            // get the width/height of the loaded object
            GetDTAttrs(node->dt_obj, PDTA_BitMapHeader, &bmhd,
                                     TAG_DONE);

            if(bmhd)
            {
              node->width = bmhd->bmh_Width;
              node->height = bmhd->bmh_Height;
            }
            else
             W(DBF_IMAGE, "couldn't found BitMap header of file '%s'", filename);

            node->screen = (struct Screen *)scr;
            // remember the screen's depth, not the image's depth!
            node->screenDepth = screenDepth;

            D(DBF_IMAGE, "loaded image data of '%s' for the first time and put it in cache.", filename);
          }
        }
        else
          D(DBF_IMAGE, "found image '%s' already cached with dt_obj: 0x%08lx.", filename, node->dt_obj);

        // increase the open count.
        node->openCount++;

        RETURN(node);
        return node;
      }
    }
  }

  if(scr != NULL)
  {
    struct imageCacheNode *node;
    if((node = (struct imageCacheNode*)calloc(sizeof(struct imageCacheNode), 1)))
    {
      // load the datatypes image now
      if((node->dt_obj = LoadDTImage(filename, scr)))
      {
        struct BitMapHeader *bmhd;
  
        // now we retrieve the bitmap header to
        // get the width/height of the loaded object
        GetDTAttrs(node->dt_obj, PDTA_BitMapHeader, &bmhd,
                                 TAG_DONE);

        if(bmhd)
        {
          node->width = bmhd->bmh_Width;
          node->height = bmhd->bmh_Height;
        }
        else
          W(DBF_IMAGE, "couldn't find BitMap header of file '%s'", filename);

        node->filename = strdup(filename);
        node->screen = (struct Screen *)scr;
        // remember the screen's depth, not the image's depth!
        node->screenDepth = GetBitMapAttr(scr->RastPort.BitMap, BMA_DEPTH);
        node->openCount++;

        AddTail((struct List *)&G->imageCacheList, (struct Node *)&node->node);

        RETURN(node);
        return node;
      }

      free(node);
    }
  }

  RETURN(NULL);
  return NULL;
}

///
/// DisposeImage()
// for disposing an imagenode properly
void DisposeImage(struct imageCacheNode *node)
{
  ENTER();

  if(node && node->openCount > 0)
  {
    node->openCount--;
    D(DBF_IMAGE, "reduced open count of image '%s' to %d", node->filename != NULL ? node->filename : "<NULL>", node->openCount);
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

  // walk through our global imageCacheList and try to find the specified
  // image file.
  for(curNode = G->imageCacheList.mlh_Head; curNode->mln_Succ; curNode = curNode->mln_Succ)
  {
    struct imageCacheNode *node = (struct imageCacheNode *)curNode;

    if(node->filename != NULL && stricmp(filename, FilePart(node->filename)) == 0)
    {
      result = TRUE;
      break;
    }
  }

  D(DBF_IMAGE, "image file '%s' was %s in image cache at node %08lx", filename, (result ? "FOUND" : "NOT FOUND"), curNode);

  RETURN(result);
  return result;
}

///
