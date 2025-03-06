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
 Description: Panel to edit an address book user entry

***************************************************************************/

#include "AddressBookEditUserPanel_cl.h"

#include <proto/locale.h>
#include <proto/muimaster.h>

#include "YAM.h"
#include "YAM_error.h"

#include "mui/AddressBookEditWindow.h"
#include "mui/PGPKeyPopup.h"
#include "mui/UserPortraitGroup.h"

#include "AddressBook.h"
#include "Locale.h"
#include "MUIObjects.h"

#include "Debug.h"

/* CLASSDATA
struct Data
{
  Object *ST_ALIAS;
  Object *ST_REALNAME;
  Object *ST_ADDRESS;
  Object *PO_PGPKEY;
  Object *ST_HOMEPAGE;
  Object *CY_DEFSECURITY;
  Object *ST_STREET;
  Object *ST_CITY;
  Object *ST_COUNTRY;
  Object *ST_PHONE;
  Object *ST_COMMENT;
  Object *ST_BIRTHDAY;
  Object *GR_PHOTO;
};
*/

/* Overloaded Methods */
/// OVERLOAD(OM_NEW)
OVERLOAD(OM_NEW)
{
  static const char *SecurityCycleEntries[5];
  Object *ST_ALIAS;
  Object *ST_REALNAME;
  Object *ST_ADDRESS;
  Object *PO_PGPKEY;
  Object *ST_HOMEPAGE;
  Object *CY_DEFSECURITY;
  Object *ST_STREET;
  Object *ST_CITY;
  Object *ST_COUNTRY;
  Object *ST_PHONE;
  Object *ST_COMMENT;
  Object *ST_BIRTHDAY;
  Object *GR_PHOTO;
  Object *BT_HOMEPAGE;

  ENTER();

  SecurityCycleEntries[0] = tr(MSG_WR_SecNone);
  SecurityCycleEntries[1] = tr(MSG_WR_SecSign);
  SecurityCycleEntries[2] = tr(MSG_WR_SecEncrypt);
  SecurityCycleEntries[3] = tr(MSG_WR_SecBoth);
  SecurityCycleEntries[4] = NULL;

  if((obj = DoSuperNew(cl, obj,
    MUIA_Group_Horiz, TRUE,
    MUIA_Group_SameWidth, TRUE,
    Child, VGroup,
      Child, ColGroup(2), GroupFrameT(tr(MSG_EA_ElectronicMail)),
        Child, Label2(tr(MSG_EA_Alias)),
        Child, ST_ALIAS = MakeString(SIZE_NAME, tr(MSG_EA_Alias)),
        Child, Label2(tr(MSG_EA_RealName)),
        Child, ST_REALNAME = MakeString(SIZE_REALNAME, tr(MSG_EA_RealName)),
        Child, Label2(tr(MSG_EA_EmailAddress)),
        Child, ST_ADDRESS  = MakeString(SIZE_ADDRESS, tr(MSG_EA_EmailAddress)),
        Child, Label2(tr(MSG_EA_PGPId)),
        Child, PO_PGPKEY = PGPKeyPopupObject,
          MUIA_PGPKeyPopup_Label, tr(MSG_EA_PGPId),
        End,
        Child, Label2(tr(MSG_EA_Homepage)),
        Child, HGroup,
          MUIA_Group_Spacing, 0,
          Child, ST_HOMEPAGE = MakeString(SIZE_URL, tr(MSG_EA_Homepage)),
          Child, BT_HOMEPAGE = PopButton(MUII_TapeRecord),
        End,
        Child, Label2(tr(MSG_EA_DefSecurity)),
        Child, CY_DEFSECURITY = CycleObject,
          MUIA_Cycle_Entries, SecurityCycleEntries,
          MUIA_ControlChar, ShortCut(tr(MSG_EA_DefSecurity)),
        End,
      End,
      Child, ColGroup(2), GroupFrameT(tr(MSG_EA_SnailMail)),
        Child, Label2(tr(MSG_EA_Street)),
        Child, ST_STREET = MakeString(SIZE_DEFAULT, tr(MSG_EA_Street)),
        Child, Label2(tr(MSG_EA_City)),
        Child, ST_CITY = MakeString(SIZE_DEFAULT, tr(MSG_EA_City)),
        Child, Label2(tr(MSG_EA_Country)),
        Child, ST_COUNTRY = MakeString(SIZE_DEFAULT, tr(MSG_EA_Country)),
        Child, Label2(tr(MSG_EA_Phone)),
        Child, ST_PHONE = MakeString(SIZE_DEFAULT, tr(MSG_EA_Phone)),
      End,
    End,
    Child, VGroup,
      Child, ColGroup(2), GroupFrameT(tr(MSG_EA_Miscellaneous)),
        Child, Label2(tr(MSG_EA_Description)),
        Child, ST_COMMENT = MakeString(SIZE_DEFAULT, tr(MSG_EA_Description)),
        Child, Label2(tr(MSG_EA_DOB)),
        Child, ST_BIRTHDAY = MakeString(SIZE_SMALL, tr(MSG_EA_DOB)),
      End,
      Child, GR_PHOTO = UserPortraitGroupObject,
      End,
    End,

    TAG_MORE, inittags(msg))) != NULL)
  {
    GETDATA;

    data->ST_ALIAS =       ST_ALIAS;
    data->ST_REALNAME =    ST_REALNAME;
    data->ST_ADDRESS =     ST_ADDRESS;
    data->PO_PGPKEY =      PO_PGPKEY;
    data->ST_HOMEPAGE =    ST_HOMEPAGE;
    data->CY_DEFSECURITY = CY_DEFSECURITY;
    data->ST_STREET =      ST_STREET;
    data->ST_CITY =        ST_CITY;
    data->ST_COUNTRY =     ST_COUNTRY;
    data->ST_PHONE =       ST_PHONE;
    data->ST_COMMENT =     ST_COMMENT;
    data->ST_BIRTHDAY =    ST_BIRTHDAY;
    data->GR_PHOTO =       GR_PHOTO;

    DoMethod(obj, MUIM_MultiSet, MUIA_String_Reject, ",",
      ST_ALIAS,
      ST_ADDRESS,
      NULL);

    SetHelp(ST_ALIAS,       MSG_HELP_EA_ST_ALIAS);
    SetHelp(ST_COMMENT,     MSG_HELP_EA_ST_DESCRIPTION);
    SetHelp(ST_REALNAME,    MSG_HELP_EA_ST_REALNAME);
    SetHelp(ST_ADDRESS,     MSG_HELP_EA_ST_ADDRESS);
    SetHelp(PO_PGPKEY,      MSG_HELP_EA_ST_PGPKEY);
    SetHelp(ST_HOMEPAGE,    MSG_HELP_EA_ST_HOMEPAGE);
    SetHelp(CY_DEFSECURITY, MSG_HELP_MA_CY_DEFSECURITY);
    SetHelp(ST_STREET,      MSG_HELP_EA_ST_STREET);
    SetHelp(ST_CITY,        MSG_HELP_EA_ST_CITY);
    SetHelp(ST_COUNTRY,     MSG_HELP_EA_ST_COUNTRY);
    SetHelp(ST_PHONE,       MSG_HELP_EA_ST_PHONE);
    SetHelp(ST_BIRTHDAY,    MSG_HELP_EA_ST_BIRTHDAY);

    set(ST_BIRTHDAY, MUIA_String_Accept, "0123456789.-/");

    DoMethod(ST_ADDRESS, MUIM_Notify, MUIA_String_Contents, MUIV_EveryTime, GR_PHOTO, 3, MUIM_Set, MUIA_UserPortraitGroup_Address, MUIV_TriggerValue);
    // when a key ID is selected, set default security to "encrypt"
    DoMethod(PO_PGPKEY, MUIM_Notify, MUIA_PGPKeyPopup_PGPKeyChanged, MUIV_EveryTime, CY_DEFSECURITY, 3, MUIM_Set, MUIA_Cycle_Active, 2);

    DoMethod(BT_HOMEPAGE, MUIM_Notify, MUIA_Pressed, FALSE, obj, 1, METHOD(VisitHomepage));
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
          char dateStr[SIZE_SMALL];

          BirthdayToString(abn->Birthday, dateStr, sizeof(dateStr));

          setstring(data->ST_ALIAS, abn->Alias);
          setstring(data->ST_REALNAME, abn->RealName);
          setstring(data->ST_ADDRESS, abn->Address);
          set(data->PO_PGPKEY, MUIA_PGPKeyPopup_PGPKey, abn->PGPId);
          setstring(data->ST_HOMEPAGE, abn->Homepage);
          setcycle(data->CY_DEFSECURITY, abn->DefSecurity);
          setstring(data->ST_STREET, abn->Street);
          setstring(data->ST_CITY, abn->City);
          setstring(data->ST_COUNTRY, abn->Country);
          setstring(data->ST_PHONE, abn->Phone);
          setstring(data->ST_COMMENT, abn->Comment);
          setstring(data->ST_BIRTHDAY, dateStr);
          set(data->GR_PHOTO,  MUIA_UserPortraitGroup_PortraitName, abn->Photo);
        }

        // make the superMethod call ignore those tags
        tag->ti_Tag = TAG_IGNORE;
      }
      break;

      case MUIA_AddressBookEditWindow_Address:
      {
        setstring(data->ST_ADDRESS, tag->ti_Data);

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
        char *birthday = (char *)xget(data->ST_BIRTHDAY, MUIA_String_Contents);

        // we must not use InitABookNode() here, because that would kill
        // the embedded node structure of old entries
        abn->type = ABNT_USER;
        GetMUIString(abn->Alias, data->ST_ALIAS, sizeof(abn->Alias));
        GetMUIString(abn->RealName, data->ST_REALNAME, sizeof(abn->RealName));
        GetMUIString(abn->Address, data->ST_ADDRESS, sizeof(abn->Address));
        GetMUIString(abn->Homepage, data->ST_HOMEPAGE, sizeof(abn->Homepage));
        GetMUIString(abn->Street, data->ST_STREET, sizeof(abn->Street));
        GetMUIString(abn->City, data->ST_CITY, sizeof(abn->City));
        GetMUIString(abn->Country, data->ST_COUNTRY, sizeof(abn->Country));
        GetMUIString(abn->Phone, data->ST_PHONE, sizeof(abn->Phone));
        GetMUIString(abn->Comment, data->ST_COMMENT, sizeof(abn->Comment));
        abn->Birthday = StringToBirthday(birthday);
        abn->DefSecurity = GetMUICycle(data->CY_DEFSECURITY);
        strlcpy(abn->PGPId, (char *)xget(data->PO_PGPKEY, MUIA_PGPKeyPopup_PGPKey), sizeof(abn->PGPId));
        strlcpy(abn->Photo, (char *)xget(data->GR_PHOTO, MUIA_UserPortraitGroup_PortraitName), sizeof(abn->Photo));

        if(IsStrEmpty(birthday) == FALSE && abn->Birthday == 0)
          ER_NewError(tr(MSG_ER_WRONG_DOB_FORMAT), G->Locale != NULL ? G->Locale->loc_ShortDateFormat : (STRPTR)"%d.%m.%Y");

        if(abn->Alias[0] == '\0')
          SetDefaultAlias(abn);
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
/// DECLARE(VisitHomepage)
//  Launches a browser to view the homepage of the person
DECLARE(VisitHomepage)
{
  GETDATA;
  char *url;

  ENTER();

  url = (char *)xget(data->ST_HOMEPAGE, MUIA_String_Contents);
  if(IsStrEmpty(url) == FALSE)
    GotoURL(url, FALSE);

  RETURN(0);
  return 0;
}

///
