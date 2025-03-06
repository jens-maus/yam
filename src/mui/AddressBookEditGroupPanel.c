/***************************************************************************

 YAM - Yet Another Mailer
 Copyright (C) 1995-2000 Marcel Beck
 Copyright (C) 2000-2025 YAM Open Source Team

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
 YAM OpenSource project    :  https://github.com/jens-maus/yam/

 Superclass:  MUIC_Group
 Description: Panel to edit an address book group entry

***************************************************************************/

#include "AddressBookEditGroupPanel_cl.h"

#include <proto/muimaster.h>

#include "YAM.h"

#include "mui/AddressBookEditWindow.h"

#include "AddressBook.h"
#include "Locale.h"
#include "MUIObjects.h"

#include "Debug.h"

/* CLASSDATA
struct Data
{
  Object *ST_ALIAS;
  Object *ST_COMMENT;
};
*/

/* Overloaded Methods */
/// OVERLOAD(OM_NEW)
OVERLOAD(OM_NEW)
{
  Object *ST_ALIAS;
  Object *ST_COMMENT;

  ENTER();

  if((obj = DoSuperNew(cl, obj,
    Child, ColGroup(2), GroupFrame,
      MUIA_Background, MUII_GroupBack,
      Child, Label2(tr(MSG_EA_Alias)),
      Child, ST_ALIAS = MakeString(SIZE_NAME, tr(MSG_EA_Alias)),
      Child, Label2(tr(MSG_EA_Description)),
      Child, ST_COMMENT = MakeString(SIZE_DEFAULT, tr(MSG_EA_Description)),
    End,

    TAG_MORE, inittags(msg))) != NULL)
  {
    GETDATA;

    data->ST_ALIAS =   ST_ALIAS;
    data->ST_COMMENT = ST_COMMENT;

    SetHelp(ST_ALIAS,   MSG_HELP_EA_ST_ALIAS);
    SetHelp(ST_COMMENT, MSG_HELP_EA_ST_DESCRIPTION);

    set(ST_ALIAS, MUIA_String_Reject, ",");
  }

  RETURN((IPTR)obj);
  return (IPTR)obj;
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
      case MUIA_AddressBookEditWindow_ABookNode:
      {
        struct ABookNode *abn = (struct ABookNode *)tag->ti_Data;

        if(abn != NULL)
        {
          setstring(data->ST_ALIAS, abn->Alias);
          setstring(data->ST_COMMENT, abn->Comment);
        }

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
    case MUIA_AddressBookEditWindow_ABookNode:
    {
      struct ABookNode *abn = (struct ABookNode *)store;

      if(abn != NULL)
      {
        // we must not use InitABookNode() here, because that would kill
        // the embedded node structure of old entries. The same applies
        // for the member list of groups.
        abn->type = ABNT_GROUP;
        GetMUIString(abn->Alias, data->ST_ALIAS, sizeof(abn->Alias));
        GetMUIString(abn->Comment, data->ST_COMMENT, sizeof(abn->Comment));
      }

      return TRUE;
    }
    break;
  }

  return DoSuperMethodA(cl, obj, msg);
}

///
/// OVERLOAD(MUIM_Setup)
OVERLOAD(MUIM_Setup)
{
  GETDATA;
  IPTR result;

  ENTER();

  if((result = DoSuperMethodA(cl, obj, msg)))
  {
    set(_win(obj), MUIA_Window_ActiveObject, data->ST_ALIAS);
  }

  RETURN(result);
  return result;
}

///

/* Public Methods */
