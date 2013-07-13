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
 MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.   See the
 GNU General Public License for more details.

 You should have received a copy of the GNU General Public License
 along with this program; if not, write to the Free Software
 Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA

 YAM Official Support Site : http://www.yam.ch
 YAM OpenSource project    : http://sourceforge.net/projects/yamos/

 $Id$

 Superclass:  MUIC_NList
 Description: Popup a list of addresses which match a given substring

***************************************************************************/

#include "AddressmatchList_cl.h"

#include <stdlib.h>
#include <mui/NList_mcc.h>

#include "YAM_addressbookEntry.h"
#include "YAM_global.h"

#include "mui/AddressmatchPopup.h"

#include "Debug.h"

/* CLASSDATA
struct Data
{
  char matchField[SIZE_ADDRESS + 4];
};
*/

/// OVERLOAD(OM_NEW)
OVERLOAD(OM_NEW)
{
  ENTER();

  obj = DoSuperNew(cl, obj,

    InputListFrame,
    MUIA_NList_Format,          ",,",

    TAG_MORE, inittags(msg));

  RETURN((IPTR)obj);
  return (IPTR)obj;
}

///
/// OVERLOAD(MUIM_NList_Construct)
OVERLOAD(MUIM_NList_Construct)
{
  struct MUIP_NList_Construct *ncm = (struct MUIP_NList_Construct *)msg;
  struct CustomABEntry *abEntry = ncm->entry;
  struct CustomABEntry *entry;

  ENTER();

  entry = memdup(abEntry, sizeof(*abEntry));

  RETURN((IPTR)entry);
  return (IPTR)entry;
}

///
/// OVERLOAD(MUIM_NList_Destruct)
OVERLOAD(MUIM_NList_Destruct)
{
  struct MUIP_NList_Destruct *ncm = (struct MUIP_NList_Destruct *)msg;
  struct CustomABEntry *abEntry = ncm->entry;

  ENTER();

  free(abEntry);

  RETURN(0);
  return 0;
}

///
/// OVERLOAD(MUIM_NList_Display)
OVERLOAD(MUIM_NList_Display)
{
  struct MUIP_NList_Display *ndm = (struct MUIP_NList_Display *)msg;
  struct CustomABEntry *entry = (struct CustomABEntry *)ndm->entry;
  GETDATA;

  ENTER();

  ndm->strings[0] = (entry->MatchEntry->Alias[0] != '\0')    ? entry->MatchEntry->Alias    : (char *)"-";
  ndm->strings[1] = (entry->MatchEntry->RealName[0] != '\0') ? entry->MatchEntry->RealName : (char *)"-";
  ndm->strings[2] = (entry->MatchEntry->Address[0] != '\0')  ? entry->MatchEntry->Address  : (char *)"-";

  if(entry->MatchField == 0)
    snprintf(data->matchField, sizeof(data->matchField), "\033b%." STR(SIZE_NAME) "s", entry->MatchString);
  else if(entry->MatchField == 1)
    snprintf(data->matchField, sizeof(data->matchField), "\033b%." STR(SIZE_REALNAME) "s", entry->MatchString);
  else
    snprintf(data->matchField, sizeof(data->matchField), "\033b%." STR(SIZE_ADDRESS) "s", entry->MatchString);

  ndm->strings[entry->MatchField] = data->matchField;

  RETURN(0);
  return 0;
}

///
/// OVERLOAD(MUIM_NList_Compare)
//  Message listview compare method
OVERLOAD(MUIM_NList_Compare)
{
  struct MUIP_NList_Compare *ncm = (struct MUIP_NList_Compare *)msg;
  struct CustomABEntry *entry1 = (struct CustomABEntry *)ncm->entry1;
  struct CustomABEntry *entry2 = (struct CustomABEntry *)ncm->entry2;
  LONG result;

  ENTER();

  result = entry1->MatchField - entry2->MatchField;
  if(result == 0)
    result = Stricmp(entry1->MatchString, entry2->MatchString);

  RETURN(result);
  return result;
}

///
