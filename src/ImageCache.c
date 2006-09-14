/***************************************************************************

 YAM - Yet Another Mailer
 Copyright (C) 1995-2000 by Marcel Beck <mbeck@yam.ch>
 Copyright (C) 2000-2006 by YAM Open Source Team

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
  "status_signed",   "status_mark",

  // Default images for the folder list
  "folder_fold",     "folder_unfold",       "folder_incoming", "folder_incoming_new",
  "folder_outgoing", "folder_outgoing_new", "folder_deleted",  "folder_deleted_new",
  "folder_sent",

  // Images for the YAM configuration window
  "config_firststep", "config_firststep_big",
  "config_network",   "config_network_big",
  "config_newmail",   "config_newmail_big",
  "config_filters",   "config_filters_big",
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
  "config_update",    "config_update_big"
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
        D(DBF_IMAGE, "successfully loaded/processed image file '%s'", filename);

        RETURN(o);
        return o;
      }
    }
    DisposeDTObject(o);
  }

  E(DBF_IMAGE, "Wasn't able to load/process specified image file '%s'", filename);

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
    if(node->dt_obj)
      DisposeDTObject(node->dt_obj);

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

  ENTER();

  // walk through our global imageCacheList and try to find the specified
  // image file.
  for(curNode = G->imageCacheList.mlh_Head; curNode->mln_Succ; curNode = curNode->mln_Succ)
  {
    struct imageCacheNode *node = (struct imageCacheNode *)curNode;

    if(node->filename != NULL && stricmp(filename, FilePart(node->filename)) == 0)
    {
      // if the image object wasn't loaded yet, we do it now
      if(node->openCount == 0 && node->dt_obj == NULL)
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

          node->scr = (struct Screen *)scr;
          node->openCount++;

          D(DBF_IMAGE, "loaded image data of '%s' for the first time and put it in cache.", filename);
        }
      }
      else
        D(DBF_IMAGE, "found image '%s' already cached with dt_obj: %lx.", filename, node->dt_obj);

      RETURN(node);
      return node;
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
          W(DBF_IMAGE, "couldn't found BitMap header of file '%s'", filename);

        node->filename = strdup(filename);
        node->scr = (struct Screen *)scr;
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
    D(DBF_IMAGE, "reduced open count of image '%s' : %d", node->filename != NULL ? node->filename : "<NULL>", node->openCount);
  }
  else
    W(DBF_IMAGE, "couldn't reduce opencount of imageCacheNode 0x%08lx/openCount=%d", node, node ? (int)node->openCount : -1);

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

  RETURN(result);
  return result;
}

///
