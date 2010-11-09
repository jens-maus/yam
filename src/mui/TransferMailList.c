/***************************************************************************

 YAM - Yet Another Mailer
 Copyright (C) 1995-2000 by Marcel Beck <mbeck@yam.ch>
 Copyright (C) 2000-2010 by YAM Open Source Team

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

 Superclass:  MUIC_NList
 Description: NList class of the transfer window

***************************************************************************/

#include "TransferMailList_cl.h"

#include <string.h>
#include <proto/muimaster.h>
#include <libraries/iffparse.h>
#include <mui/NList_mcc.h>

#include "YAM.h"
#include "YAM_addressbookEntry.h"
#include "YAM_config.h"
#include "YAM_mainFolder.h"
#include "YAM_utilities.h"

#include "Locale.h"
#include "MailTransferList.h"
#include "MUIObjects.h"
#include "Themes.h"

#include "mui/ImageArea.h"

#include "Debug.h"

/* CLASSDATA
struct Data
{
  Object *downloadImage;
  Object *deleteImage;
  char indexBuffer[SIZE_SMALL];
  char statusBuffer[SIZE_DEFAULT];
  char fromBuffer[SIZE_DEFAULT];
  char sizeBuffer[SIZE_SMALL];
  char dateBuffer[64];
};
*/

/* Overloaded Methods */
/// OVERLOAD(OM_NEW)
OVERLOAD(OM_NEW)
{
  ENTER();

  if((obj = DoSuperNew(cl, obj,

    InputListFrame,
    MUIA_ObjectID,             MAKE_ID('N','L','0','4'),
    MUIA_Font,                 C->FixedFontList ? MUIV_NList_Font_Fixed : MUIV_NList_Font,
    MUIA_ContextMenu,          NULL,
    MUIA_NList_MultiSelect,    MUIV_NList_MultiSelect_Default,
    MUIA_NList_Format,         "P=\033r BAR,W=-1 BAR,W=-1 MACW=9 P=\033r BAR,MICW=10 MACW=30 BAR,BAR,MICW=16 MACW=30 BAR,MICW=9 MACW=15 BAR",
    MUIA_NList_AutoVisible,    TRUE,
    MUIA_NList_Title,          TRUE,
    MUIA_NList_TitleSeparator, TRUE,
    MUIA_NList_DoubleClick,    TRUE,
    MUIA_NList_MinColSortable, 0,
    MUIA_NList_Exports,        MUIV_NList_Exports_ColWidth|MUIV_NList_Exports_ColOrder,
    MUIA_NList_Imports,        MUIV_NList_Imports_ColWidth|MUIV_NList_Imports_ColOrder,

    TAG_MORE, inittags(msg))) != NULL)
  {
    GETDATA;

    // prepare the group image
    data->downloadImage = MakeImageAltObject("status_download", G->theme.statusImages[si_Download], tr(MSG_ALTIMAGE_STATUS_DOWNLOAD));
    data->deleteImage   = MakeImageAltObject("status_delete", G->theme.statusImages[si_Delete], tr(MSG_ALTIMAGE_STATUS_DELETE));

    DoMethod(obj, MUIM_NList_UseImage, data->downloadImage, si_Download, MUIF_NONE);
    DoMethod(obj, MUIM_NList_UseImage, data->deleteImage, si_Delete, MUIF_NONE);
  }

  RETURN((IPTR)obj);
  return (IPTR)obj;
}

///
/// OVERLOAD(OM_DISPOSE)
OVERLOAD(OM_DISPOSE)
{
  GETDATA;

  if(data->downloadImage != NULL)
  {
    DoMethod(obj, MUIM_NList_UseImage, NULL, si_Download, MUIF_NONE);
    MUI_DisposeObject(data->downloadImage);
    data->downloadImage = NULL;
  }

  if(data->deleteImage != NULL)
  {
    DoMethod(obj, MUIM_NList_UseImage, NULL, si_Delete, MUIF_NONE);
    MUI_DisposeObject(data->deleteImage);
    data->deleteImage = NULL;
  }

  return DoSuperMethodA(cl,obj,msg);
}

///
/// OVERLOAD(MUIM_NList_Display)
OVERLOAD(MUIM_NList_Display)
{
  struct MUIP_NList_Display *ndm = (struct MUIP_NList_Display *)msg;
  struct MailTransferNode *entry = (struct MailTransferNode *)ndm->entry;

  ENTER();

  if(entry != NULL)
  {
    struct Mail *mail = entry->mail;
    struct Person *pe = &mail->From;
    GETDATA;

    // mail index
    snprintf(data->indexBuffer, sizeof(data->indexBuffer), "%d", entry->index);
    ndm->strings[0] = data->indexBuffer;

    // status icon display
    data->statusBuffer[0] = '\0';
    if(isFlagSet(entry->tflags, TRF_TRANSFER))
      strlcat(data->statusBuffer, SI_STR(si_Download), sizeof(data->statusBuffer));
    if(isFlagSet(entry->tflags, TRF_DELETE))
      strlcat(data->statusBuffer, SI_STR(si_Delete), sizeof(data->statusBuffer));
    ndm->strings[1] = data->statusBuffer;

    // size display
    if(C->WarnSize > 0 && mail->Size >= (C->WarnSize*1024))
    {
      strlcpy(data->sizeBuffer, MUIX_PH, sizeof(data->sizeBuffer));
      FormatSize(mail->Size, &data->sizeBuffer[strlen(data->sizeBuffer)], sizeof(data->sizeBuffer)-strlen(data->sizeBuffer), SF_AUTO);
    }
    else
      FormatSize(mail->Size, data->sizeBuffer, sizeof(data->sizeBuffer), SF_AUTO);
    ndm->strings[2] = data->sizeBuffer;

    // from address display
    strlcpy(data->fromBuffer, AddrName(*pe), sizeof(data->fromBuffer));
    ndm->strings[3] = data->fromBuffer;

    // mail subject display
    ndm->strings[4] = mail->Subject;

    // display date
    data->dateBuffer[0] = '\0';
    if(mail->Date.ds_Days != 0)
      DateStamp2String(data->dateBuffer, sizeof(data->dateBuffer), &mail->Date, (C->DSListFormat == DSS_DATEBEAT || C->DSListFormat == DSS_RELDATEBEAT) ? DSS_DATEBEAT : DSS_DATETIME, TZC_LOCAL);
    ndm->strings[5] = data->dateBuffer;
  }
  else
  {
    ndm->strings[0] = (STRPTR)tr(MSG_PRESELECT_INDEX);
    ndm->strings[1] = (STRPTR)tr(MSG_MA_TitleStatus);
    ndm->strings[2] = (STRPTR)tr(MSG_Size);
    ndm->strings[3] = (STRPTR)tr(MSG_From);
    ndm->strings[4] = (STRPTR)tr(MSG_Subject);
    ndm->strings[5] = (STRPTR)tr(MSG_Date);
  }

  LEAVE();
  return 0;
}

///

/* Private Functions */

/* Public Methods */
