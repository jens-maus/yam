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
 Description: NList class of the attachment requester's list

***************************************************************************/

#include "AttachmentList_cl.h"

#include <mui/NList_mcc.h>

#include "YAM_mainFolder.h"
#include "YAM_read.h"

#include "Locale.h"

#include "Debug.h"

/* CLASSDATA
struct Data
{
  char numberBuffer[SIZE_SMALL];
  char sizeBuffer[SIZE_SMALL];
};
*/

/* Overloaded Methods */
/// OVERLOAD(OM_NEW)
OVERLOAD(OM_NEW)
{
  ENTER();

  obj = DoSuperNew(cl, obj,

    InputListFrame,
    MUIA_NList_Format,               "BAR,BAR,BAR,",
    MUIA_NList_Title,                TRUE,
    MUIA_NList_DoubleClick,          TRUE,
    MUIA_NList_DefaultObjectOnClick, FALSE,

    TAG_MORE, inittags(msg));

  RETURN((IPTR)obj);
  return (IPTR)obj;
}

///
/// OVERLOAD(MUIM_NList_Display)
OVERLOAD(MUIM_NList_Display)
{
  struct MUIP_NList_Display *ndm = (struct MUIP_NList_Display *)msg;
  struct Part *entry = (struct Part *)ndm->entry;

  ENTER();

  if(entry != NULL)
  {
    GETDATA;

    if(entry->Nr > PART_RAW)
    {
      snprintf(data->numberBuffer, sizeof(data->numberBuffer), "%d%s", entry->Nr, (entry->rmData != NULL && entry->Nr == entry->rmData->letterPartNum) ? "*" : "");
      ndm->strings[0] = data->numberBuffer;
    }
    else
      ndm->strings[0] = (STRPTR)"";

    ndm->strings[1] = entry->Name;

    if(entry->Nr <= PART_RAW)
      ndm->strings[2] = (STRPTR)tr(MSG_CTtextplain);
    else if(entry->Description[0] != '\0')
      ndm->strings[2] = entry->Description;
    else
      ndm->strings[2] = (STRPTR)DescribeCT(entry->ContentType);

    // check the alternative status
    if(isAlternativePart(entry) == TRUE && entry->Parent != NULL && entry->Parent->MainAltPart != entry)
      ndm->preparses[1] = (char *)MUIX_I;

    if(entry->Size > 0)
    {
      if(isDecoded(entry))
        FormatSize(entry->Size, data->sizeBuffer, sizeof(data->sizeBuffer), SF_AUTO);
      else
      {
        data->sizeBuffer[0] = '~';
        FormatSize(entry->Size, &data->sizeBuffer[1], sizeof(data->sizeBuffer)-1, SF_AUTO);
      }

      ndm->strings[3] = data->sizeBuffer;
    }
    else
      ndm->strings[3] = (STRPTR)"";
  }
  else
  {
    ndm->strings[0] = (STRPTR)tr(MSG_ATTACH_NO);
    ndm->strings[1] = (STRPTR)tr(MSG_ATTACH_PART);
    ndm->strings[2] = (STRPTR)tr(MSG_RE_Description);
    ndm->strings[3] = (STRPTR)tr(MSG_Size);
  }

  LEAVE();
  return 0;
}

///

/* Private Functions */

/* Public Methods */

