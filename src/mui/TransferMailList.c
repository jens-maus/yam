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

#include "YAM_addressbookEntry.h"
#include "YAM_mainFolder.h"
#include "YAM_transfer.h"
#include "YAM_utilities.h"
#include "MUIObjects.h"
#include "Themes.h"

#include "Debug.h"

/* CLASSDATA
struct Data
{
  Object *downloadImage;
  Object *deleteImage;
};
*/

/* Hooks */
/// DisplayHook
//  Message listview display hook
HOOKPROTONHNO(DisplayFunc, LONG, struct NList_DisplayMessage *msg)
{
  struct MailTransferNode *entry;
  char **array = msg->strings;

  ENTER();

  if((entry = (struct MailTransferNode *)msg->entry) != NULL)
  {
    static char dispfro[SIZE_DEFAULT];
    static char dispsta[SIZE_DEFAULT];
    static char dispsiz[SIZE_SMALL];
    static char dispdate[64];
    struct Mail *mail = entry->mail;
    struct Person *pe = &mail->From;

    array[0] = dispsta;
    // status icon display
    snprintf(dispsta, sizeof(dispsta), "%3d ", entry->index);
    if(hasTR_TRANSFER(entry))
      strlcat(dispsta, SI_STR(si_Download), sizeof(dispsta));
    if(hasTR_DELETE(entry))
      strlcat(dispsta, SI_STR(si_Delete), sizeof(dispsta));

    // size display
    array[1] = dispsiz;
    if(C->WarnSize > 0 && mail->Size >= (C->WarnSize*1024))
    {
      strlcpy(dispsiz, MUIX_PH, sizeof(dispsiz));
      FormatSize(mail->Size, dispsiz+strlen(dispsiz), sizeof(dispsiz)-strlen(dispsiz), SF_AUTO);
    }
    else
      FormatSize(mail->Size, dispsiz, sizeof(dispsiz), SF_AUTO);

    // from address display
    array[2] = dispfro;
    strlcpy(dispfro, AddrName(*pe), sizeof(dispfro));

    // mail subject display
    array[3] = mail->Subject;

    // display date
    array[4] = dispdate;
    *dispdate = '\0';

    if(mail->Date.ds_Days != 0)
      DateStamp2String(dispdate, sizeof(dispdate), &mail->Date, (C->DSListFormat == DSS_DATEBEAT || C->DSListFormat == DSS_RELDATEBEAT) ? DSS_DATEBEAT : DSS_DATETIME, TZC_LOCAL);
  }
  else
  {
    array[0] = (STRPTR)tr(MSG_MA_TitleStatus);
    array[1] = (STRPTR)tr(MSG_Size);
    array[2] = (STRPTR)tr(MSG_From);
    array[3] = (STRPTR)tr(MSG_Subject);
    array[4] = (STRPTR)tr(MSG_Date);
  }

  LEAVE();
  return 0;
}
MakeStaticHook(DisplayHook, DisplayFunc);

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
    MUIA_NList_MultiSelect,    MUIV_NList_MultiSelect_Default,
    MUIA_NList_Format,         "W=-1 BAR,W=-1 MACW=9 P=\033r BAR,MICW=10 MACW=30 BAR,BAR,MICW=16 MACW=30 BAR,MICW=9 MACW=15 BAR",
    MUIA_NList_DisplayHook2,   &DisplayHook,
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
    data->downloadImage = MakeImageObject("status_download", G->theme.statusImages[si_Download]);
    data->deleteImage   = MakeImageObject("status_delete", G->theme.statusImages[si_Delete]);

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

/* Private Functions */

/* Public Methods */

