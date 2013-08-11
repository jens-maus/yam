/***************************************************************************

 YAM - Yet Another Mailer
 Copyright (C) 1995-2000 Marcel Beck
 Copyright (C) 2000-2013 YAM Open Source Team

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
#include <proto/dos.h>
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
  char toBuffer[SIZE_DEFAULT];
  char sizeBuffer[SIZE_SMALL];
  char dateBuffer[64];
  LONG sizeLimit;
};
*/

/* Private Functions */
/// MailCompare
//  Compares two messages
static int MailCompare(struct MailTransferNode *entry1, struct MailTransferNode *entry2, LONG column)
{
  struct Mail *mail1 = entry1->mail;
  struct Mail *mail2 = entry2->mail;

  switch (column)
  {
    case 0:
    {
      // status
      // the transfer list cannot be sorted by status
    }
    break;

    case 1:
    {
      // mail index
      return entry1->index - entry2->index;
    }
    break;

    case 2:
    {
      // mail size
      return mail1->Size - mail2->Size;
    }
    break;

    case 3:
    {
      // sender
      char *addr1 = AddrName(mail1->From);
      char *addr2 = AddrName(mail2->From);

      return stricmp(addr1, addr2);
    }
    break;

    case 4:
    {
      // subject
      return stricmp(MA_GetRealSubject(mail1->Subject), MA_GetRealSubject(mail2->Subject));
    }
    break;

    case 5:
    {
      // date
      return CompareDates(&mail2->Date, &mail1->Date);
    }
    break;
  }

  return 0;
}

///

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
    MUIA_NList_TitleClick,     TRUE,
    MUIA_NList_MultiSelect,    MUIV_NList_MultiSelect_Default,
    MUIA_NList_Format,         "W=-1 BAR,P=\033r PCS=R BAR,W=-1 MACW=9 P=\033r PCS=R BAR,MICW=10 MACW=30 PCS=R BAR,PCS=R BAR,MICW=16 MACW=30 PCS=R BAR,MICW=9 MACW=15 PCS=R BAR",
    MUIA_NList_AutoVisible,    TRUE,
    MUIA_NList_MinColSortable, 1,
    MUIA_NList_Title,          TRUE,
    MUIA_NList_TitleSeparator, TRUE,
    MUIA_NList_DoubleClick,    TRUE,
    MUIA_NList_Exports,        MUIV_NList_Exports_ColWidth|MUIV_NList_Exports_ColOrder,
    MUIA_NList_Imports,        MUIV_NList_Imports_ColWidth|MUIV_NList_Imports_ColOrder,

    TAG_MORE, inittags(msg))) != NULL)
  {
    GETDATA;

    // prepare the group image
    data->downloadImage = MakeImageAltObject("status_download", G->theme.statusImages[SI_DOWNLOAD], tr(MSG_ALTIMAGE_STATUS_DOWNLOAD));
    data->deleteImage   = MakeImageAltObject("status_delete", G->theme.statusImages[SI_DELETE], tr(MSG_ALTIMAGE_STATUS_DELETE));

    data->sizeLimit = GetTagData(ATTR(SizeLimit), 0, inittags(msg)) * 1024;

    DoMethod(obj, MUIM_NList_UseImage, data->downloadImage, SI_DOWNLOAD, MUIF_NONE);
    DoMethod(obj, MUIM_NList_UseImage, data->deleteImage, SI_DELETE, MUIF_NONE);

    DoMethod(obj, MUIM_Notify, MUIA_NList_TitleClick,   MUIV_EveryTime, MUIV_Notify_Self, 4, MUIM_NList_Sort3, MUIV_TriggerValue,     MUIV_NList_SortTypeAdd_2Values, MUIV_NList_Sort3_SortType_Both);
    DoMethod(obj, MUIM_Notify, MUIA_NList_SortType,     MUIV_EveryTime, MUIV_Notify_Self, 3, MUIM_Set,         MUIA_NList_TitleMark,  MUIV_TriggerValue);
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
    DoMethod(obj, MUIM_NList_UseImage, NULL, SI_DOWNLOAD, MUIF_NONE);
    MUI_DisposeObject(data->downloadImage);
    data->downloadImage = NULL;
  }

  if(data->deleteImage != NULL)
  {
    DoMethod(obj, MUIM_NList_UseImage, NULL, SI_DELETE, MUIF_NONE);
    MUI_DisposeObject(data->deleteImage);
    data->deleteImage = NULL;
  }

  return DoSuperMethodA(cl,obj,msg);
}

///
/// OVERLOAD(MUIM_NList_Compare)
//  Message listview compare method
OVERLOAD(MUIM_NList_Compare)
{
  struct MUIP_NList_Compare *ncm = (struct MUIP_NList_Compare *)msg;
  struct MailTransferNode *entry1 = (struct MailTransferNode *)ncm->entry1;
  struct MailTransferNode *entry2 = (struct MailTransferNode *)ncm->entry2;
  LONG col1 = ncm->sort_type1 & MUIV_NList_TitleMark_ColMask;
  LONG col2 = ncm->sort_type2 & MUIV_NList_TitleMark2_ColMask;
  int cmp;

  ENTER();

  if(ncm->sort_type1 == (LONG)MUIV_NList_SortType_None)
  {
    RETURN(0);
    return 0;
  }

  if(ncm->sort_type1 & MUIV_NList_TitleMark_TypeMask) cmp = MailCompare(entry2, entry1, col1);
  else                                                cmp = MailCompare(entry1, entry2, col1);

  if(cmp != 0 || col1 == col2)
  {
    RETURN(cmp);
    return cmp;
  }

  if(ncm->sort_type2 & MUIV_NList_TitleMark2_TypeMask) cmp = MailCompare(entry2, entry1, col2);
  else                                                 cmp = MailCompare(entry1, entry2, col2);

  RETURN(cmp);
  return cmp;
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
    GETDATA;
    struct Mail *mail;

    // status icon display
    data->statusBuffer[0] = '\0';
    if(isFlagSet(entry->tflags, TRF_TRANSFER))
      strlcat(data->statusBuffer, SI_STR(SI_DOWNLOAD), sizeof(data->statusBuffer));
    if(isFlagSet(entry->tflags, TRF_DELETE))
      strlcat(data->statusBuffer, SI_STR(SI_DELETE), sizeof(data->statusBuffer));
    ndm->strings[0] = data->statusBuffer;

    // mail index
    snprintf(data->indexBuffer, sizeof(data->indexBuffer), "%d", entry->index);
    ndm->strings[1] = data->indexBuffer;

    if((mail = entry->mail) != NULL)
    {
      // size display
      if(data->sizeLimit > 0 && mail->Size >= data->sizeLimit)
      {
        strlcpy(data->sizeBuffer, MUIX_PH, sizeof(data->sizeBuffer));
        FormatSize(mail->Size, &data->sizeBuffer[strlen(data->sizeBuffer)], sizeof(data->sizeBuffer)-strlen(data->sizeBuffer), SF_AUTO);
      }
      else
        FormatSize(mail->Size, data->sizeBuffer, sizeof(data->sizeBuffer), SF_AUTO);
      ndm->strings[2] = data->sizeBuffer;

      // from address display
      strlcpy(data->fromBuffer, AddrName(mail->From), sizeof(data->fromBuffer));
      ndm->strings[3] = data->fromBuffer;

      // to address display
      strlcpy(data->toBuffer, AddrName(mail->To), sizeof(data->toBuffer));
      ndm->strings[4] = data->toBuffer;

      // mail subject display
      ndm->strings[5] = mail->Subject;

      // display date
      data->dateBuffer[0] = '\0';
      if(mail->Date.ds_Days != 0)
        DateStamp2String(data->dateBuffer, sizeof(data->dateBuffer), &mail->Date, (C->DSListFormat == DSS_DATEBEAT || C->DSListFormat == DSS_RELDATEBEAT) ? DSS_DATEBEAT : DSS_DATETIME, TZC_UTC2LOCAL);
      ndm->strings[6] = data->dateBuffer;
    }
    else
    {
      data->sizeBuffer[0] = '\0';
      ndm->strings[2] = data->sizeBuffer;

      data->fromBuffer[0] = '\0';
      ndm->strings[3] = data->fromBuffer;

      // reuse the same string again, this is no problem
      // as we just have to show an empty string
      ndm->strings[4] = data->fromBuffer;
      ndm->strings[5] = data->fromBuffer;

      data->dateBuffer[0] = '\0';
      ndm->strings[6] = data->dateBuffer;
    }
  }
  else
  {
    ndm->strings[0] = (STRPTR)tr(MSG_MA_TitleStatus);
    ndm->strings[1] = (STRPTR)tr(MSG_PRESELECT_INDEX);
    ndm->strings[2] = (STRPTR)tr(MSG_Size);
    ndm->strings[3] = (STRPTR)tr(MSG_From);
    ndm->strings[4] = (STRPTR)tr(MSG_To);
    ndm->strings[5] = (STRPTR)tr(MSG_Subject);
    ndm->strings[6] = (STRPTR)tr(MSG_Date);
  }

  RETURN(0);
  return 0;
}

///

/* Public Methods */
