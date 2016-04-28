/***************************************************************************

 YAM - Yet Another Mailer
 Copyright (C) 1995-2000 Marcel Beck
 Copyright (C) 2000-2016 YAM Open Source Team

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
 YAM OpenSource project    : http://sourceforge.net/projects/yamos/

 $Id$

 Superclass:  MUIC_Group
 Description: Provides GUI elements for selecting a user portrait.

***************************************************************************/

#include "UserPortraitGroup_cl.h"

#include <string.h>

#include <proto/dos.h>
#include <proto/muimaster.h>

#include "extrasrc.h"

#include "YAM.h"
#include "YAM_error.h"

#include "mime/md5.h"
#include "mui/ClassesExtra.h"
#include "mui/ImageArea.h"
#include "tcp/http.h"

#include "Config.h"
#include "FileInfo.h"
#include "Locale.h"
#include "MUIObjects.h"
#include "Requesters.h"
#include "Threads.h"

#include "Debug.h"

/* CLASSDATA
struct Data
{
  Object *portrait;
  Object *portraitGroup;
  Object *selectButton;
  Object *removeButton;
  Object *gravatarButton;
  char portraitName[SIZE_PATHFILE];
  char address[SIZE_ADDRESS];
  char lcAddress[SIZE_ADDRESS];
  APTR thread;
  BOOL cleared;
};
*/

/* Overloaded Methods */
/// OVERLOAD(OM_NEW)
OVERLOAD(OM_NEW)
{
  Object *portrait;
  Object *portraitGroup;
  Object *selectButton;
  Object *removeButton;
  Object *gravatarButton;

  ENTER();

  if((obj = DoSuperNew(cl, obj,

    GroupFrameT(tr(MSG_EA_Portrait)),
    MUIA_Group_Horiz, FALSE,
    MUIA_ContextMenu, FALSE,
    Child, VSpace(0),
    Child, HGroup,
      Child, HSpace(0),
      Child, portraitGroup = HGroup,
        Child, portrait = ImageAreaObject,
          ImageButtonFrame,
          MUIA_ImageArea_MaxWidth, 64,
          MUIA_ImageArea_MaxHeight, 64,
          MUIA_ImageArea_NoMinHeight, FALSE,
          MUIA_ImageArea_ShowLabel, FALSE,
        End,
      End,
      Child, HSpace(0),
    End,
    Child, VSpace(0),
    Child, selectButton = MakeButton(tr(MSG_EA_SelectPhoto)),
    Child, removeButton = MakeButton(tr(MSG_EA_REMOVEPHOTO)),
    Child, gravatarButton = MakeButton(tr(MSG_EA_SEARCH_GRAVATAR)),

    TAG_MORE, inittags(msg))) != NULL)
  {
    GETDATA;

    data->portrait = portrait;
    data->portraitGroup = portraitGroup;
    data->selectButton = selectButton;
    data->removeButton = removeButton;
    data->gravatarButton = gravatarButton;

    SetHelp(portrait, MSG_HELP_EA_BC_PHOTO);
    SetHelp(selectButton, MSG_HELP_EA_BT_SELECTPHOTO);
    SetHelp(removeButton, MSG_HELP_EA_BT_REMOVEPHOTO);

    DoMethod(selectButton, MUIM_Notify, MUIA_Pressed, FALSE, obj, 1, METHOD(SelectPortrait));
    DoMethod(removeButton, MUIM_Notify, MUIA_Pressed, FALSE, obj, 1, METHOD(RemovePortrait));
    DoMethod(gravatarButton, MUIM_Notify, MUIA_Pressed, FALSE, obj, 1, METHOD(CheckGravatar));

    set(removeButton, MUIA_Disabled, TRUE);
  }

  RETURN((IPTR)obj);
  return (IPTR)obj;
}

///
/// OVERLOAD(OM_DISPOSE)
OVERLOAD(OM_DISPOSE)
{
  GETDATA;
  IPTR result;

  // abort the gravatar thread in case it is still running
  if(data->thread != NULL)
    AbortThread(data->thread, TRUE);

  result = DoSuperMethodA(cl, obj, msg);

  RETURN(result);
  return result;
}

///
/// OVERLOAD(OM_SET)
OVERLOAD(OM_SET)
{
  GETDATA;
  struct TagItem *tags = inittags(msg), *tag;
  ULONG result = FALSE;

  ENTER();

  while((tag = NextTagItem((APTR)&tags)) != NULL)
  {
    switch(tag->ti_Tag)
    {
      case ATTR(Address):
      {
        strlcpy(data->address, (char *)tag->ti_Data, sizeof(data->address));
        // make the superMethod call ignore those tags
        tag->ti_Tag = TAG_IGNORE;
      }
      break;

      case ATTR(PortraitName):
      {
        strlcpy(data->portraitName, (char *)tag->ti_Data, sizeof(data->portraitName));
        DoMethod(obj, METHOD(UpdatePortrait));
        // make the superMethod call ignore those tags
        tag->ti_Tag = TAG_IGNORE;
      }
      break;
    }
  }

  result = DoSuperMethodA(cl, obj, msg);

  RETURN(result);
  return result;
}

///
/// OVERLOAD(OM_GET)
OVERLOAD(OM_GET)
{
  GETDATA;
  IPTR *store = ((struct opGet *)msg)->opg_Storage;

  switch(((struct opGet *)msg)->opg_AttrID)
  {
    case ATTR(PortraitName):
    {
      *store = (IPTR)data->portraitName;

      return TRUE;
    }
    break;
  }

  return DoSuperMethodA(cl, obj, msg);
}

///
/// OVERLOAD(MUIM_Cleanup)
OVERLOAD(MUIM_Cleanup)
{
  IPTR rc;

  ENTER();

  DoMethod(obj, METHOD(Clear));
  rc = DoSuperMethodA(cl, obj, msg);

  RETURN(rc);
  return rc;
}

///

/* Public Methods */
/// DECLARE(UpdatePortrait)
DECLARE(UpdatePortrait)
{
  GETDATA;

  ENTER();

  if(data->portraitName[0] != '\0')
  {
    enum FType type;

    if(ObtainFileInfo(data->portraitName, FI_TYPE, &type) == TRUE && type == FIT_FILE &&
       DoMethod(data->portraitGroup, MUIM_Group_InitChange))
    {
      if((char *)xget(data->portrait, MUIA_ImageArea_Filename) != NULL)
      {
        // remove the old image from the cache
        set(data->portrait, MUIA_ImageArea_Filename, NULL);
      }

      // set the new attributes
      xset(data->portrait,
        MUIA_ImageArea_ID,       data->address,
        MUIA_ImageArea_Filename, data->portraitName);

      // and force a cleanup/setup pair
      DoMethod(data->portraitGroup, OM_REMMEMBER, data->portrait);
      DoMethod(data->portraitGroup, OM_ADDMEMBER, data->portrait);

      DoMethod(data->portraitGroup, MUIM_Group_ExitChange);

      set(data->removeButton, MUIA_Disabled, FALSE);
    }
  }

  RETURN(0);
  return 0;
}

///
/// DECLARE(SelectPortrait)
DECLARE(SelectPortrait)
{
  GETDATA;
  struct FileReqCache *frc;

  ENTER();

  if((frc = ReqFile(ASL_PHOTO, _win(obj), tr(MSG_EA_SelectPhoto_Title), REQF_NONE, C->GalleryDir, data->portraitName)) != NULL)
  {
    AddPath(data->portraitName, frc->drawer, frc->file, sizeof(data->portraitName));
    DoMethod(obj, METHOD(UpdatePortrait));
  }

  RETURN(0);
  return 0;
}

///
/// DECLARE(RemovePortrait)
DECLARE(RemovePortrait)
{
  GETDATA;

  ENTER();

  data->portraitName[0] = '\0';
  if(DoMethod(data->portraitGroup, MUIM_Group_InitChange))
  {
    // force the image to be removed from the cache
    set(data->portrait, MUIA_ImageArea_Filename, NULL);

    // and force a cleanup/setup pair
    DoMethod(data->portraitGroup, OM_REMMEMBER, data->portrait);
    DoMethod(data->portraitGroup, OM_ADDMEMBER, data->portrait);

    DoMethod(data->portraitGroup, MUIM_Group_ExitChange);

    set(data->removeButton, MUIA_Disabled, TRUE);
  }

  RETURN(0);
  return 0;
}

///
/// DECLARE(CheckGravatar)
DECLARE(CheckGravatar)
{
  GETDATA;

  ENTER();

  if(data->address[0] != '\0')
  {
    struct MD5Context md5ctx;
    unsigned char digest[16];
    char hdigest[SIZE_DEFAULT];
    char imagePath[SIZE_PATHFILE];
    BOOL doDownload;

    // create a lower case copy of the user's address
    strlcpy(data->lcAddress, data->address, sizeof(data->lcAddress));
    ToLowerCase(data->lcAddress);

    // build the MD5 checksum of the address
    md5init(&md5ctx);
    md5update(&md5ctx, data->lcAddress, strlen(data->lcAddress));
    md5final(digest, &md5ctx);

    // convert the digest into a hexdump
    md5digestToHex(digest, hdigest);

    // we request JPEG images, including a 404 error response if no image is available
    strlcat(hdigest, ".jpg?d=404", sizeof(hdigest));

    // build the final file name
    AddPath(imagePath, C->GalleryDir, data->address, sizeof(imagePath));
    strlcat(imagePath, ".jpg", sizeof(imagePath));

    // replace possibly invalid characters within the file name at last
    ReplaceInvalidChars((char *)FilePart(imagePath));

    doDownload = TRUE;
    while(FileExists(imagePath) == TRUE)
    {
      if(MUI_Request(_app(obj), _win(obj), MUIF_NONE, tr(MSG_MA_ConfirmReq), tr(MSG_YesNoReq), tr(MSG_FILE_OVERWRITE), imagePath) == 0)
      {
        struct FileReqCache *frc;

        // user chose not to overwrite the existing file, let him choose another one
        if((frc = ReqFile(ASL_PHOTO, obj, tr(MSG_RE_SAVE_FILE), REQF_SAVEMODE, C->GalleryDir, FilePart(imagePath))) != NULL)
        {
          AddPath(imagePath, frc->drawer, frc->file, sizeof(imagePath));
        }
        else
        {
          doDownload = FALSE;
        }
      }
      else
      {
        // download the image to the selected file and overwrite the existing one
        break;
      }
    }

    if(doDownload == TRUE)
    {
      if(strcasecmp(imagePath, data->portraitName) == 0)
      {
        // if we are checking for the same image again we must make sure that the current
        // image is no longer in use
        DoMethod(obj, METHOD(Clear));
        data->cleared = TRUE;
      }

      // try to download the user portrait
      data->thread = DoAction(obj, TA_DownloadURL, TT_DownloadURL_Server, "http://www.gravatar.com/avatar",
                                                   TT_DownloadURL_Request, hdigest,
                                                   TT_DownloadURL_Filename, imagePath,
                                                   TT_DownloadURL_Flags, DLURLF_VISIBLE|DLURLF_NO_ERROR_ON_404,
                                                   TAG_DONE);
    }
  }

  RETURN(0);
  return 0;
}

///
/// OVERLOAD(MUIM_ThreadFinished)
OVERLOAD(MUIM_ThreadFinished)
{
  GETDATA;
  struct MUIP_ThreadFinished *tf = (struct MUIP_ThreadFinished *)msg;

  ENTER();

  // the thread which did the download has finished
  // now use the originally supplied file name as new portrait file
  if(tf->result == TRUE)
  {
    strlcpy(data->portraitName, (char *)GetTagData(TT_DownloadURL_Filename, (IPTR)"", tf->actionTags), sizeof(data->portraitName));
    DoMethod(obj, METHOD(UpdatePortrait));
  }
  else
  {
    // restore the previous portrait if it was cleared before
    if(data->cleared == TRUE)
    {
      data->cleared = FALSE;
      DoMethod(obj, METHOD(UpdatePortrait));
    }

    ER_NewError(tr(MSG_ER_NO_GRAVATAR_FOUND), data->address);
  }

  // forget about the thread
  data->thread = NULL;

  RETURN(0);
  return 0;
}

///
/// DECLARE(Clear)
DECLARE(Clear)
{
  GETDATA;

  ENTER();

  if(data->address[0] != '\0')
  {
    // update the user image ID and remove it from the cache
    // it will be reloaded when necessary
    xset(data->portrait,
      MUIA_ImageArea_ID,       data->address,
      MUIA_ImageArea_Filename, NULL);
  }

  RETURN(0);
  return 0;
}

///
