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
 MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 GNU General Public License for more details.

 You should have received a copy of the GNU General Public License
 along with this program; if not, write to the Free Software
 Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA

 YAM Official Support Site :  http://www.yam.ch
 YAM OpenSource project    :  http://sourceforge.net/projects/yamos/

 $Id$

 Superclass:  MUIC_Group
 Description: Panel to edit an address book list entry

***************************************************************************/

#include "AddressBookEditListPanel_cl.h"

#include <proto/muimaster.h>
#include <mui/BetterString_mcc.h>
#include <mui/NList_mcc.h>
#include <mui/NListview_mcc.h>

#include "YAM.h"

#include "mui/AddressBookEditWindow.h"
#include "mui/AddressBookEntryList.h"
#include "mui/AddressBookWindow.h"
#include "mui/AddressField.h"
#include "mui/RecipientString.h"

#include "AddressBook.h"
#include "DynamicString.h"
#include "Locale.h"
#include "MUIObjects.h"

#include "Debug.h"

/* CLASSDATA
struct Data
{
  Object *ST_ALIAS;
  Object *ST_REALNAME;
  Object *ST_ADDRESS;
  Object *ST_COMMENT;
  Object *LV_MEMBER;
  Object *ST_MEMBER;
  Object *BT_ADD;
  Object *BT_DEL;
};
*/

/* Overloaded Methods */
/// OVERLOAD(OM_NEW)
OVERLOAD(OM_NEW)
{
  Object *ST_ALIAS;
  Object *ST_REALNAME;
  Object *ST_ADDRESS;
  Object *ST_COMMENT;
  Object *LV_MEMBER;
  Object *ST_MEMBER;
  Object *BT_ADD;
  Object *BT_DEL;
  Object *BT_SORT;

  ENTER();

  if((obj = DoSuperNew(cl, obj,
    MUIA_Group_Horiz, TRUE,
    MUIA_Group_SameWidth, TRUE,
    Child, VGroup,
      Child, ColGroup(2), GroupFrameT(tr(MSG_EA_ElectronicMail)),
        MUIA_Background, MUII_GroupBack,
        Child, Label2(tr(MSG_EA_Alias)),
        Child, ST_ALIAS = MakeString(SIZE_NAME, tr(MSG_EA_Alias)),
        Child, Label2(tr(MSG_EA_ReturnAddress)),
        Child, MakeAddressField(&ST_ADDRESS, tr(MSG_EA_ReturnAddress), MSG_HELP_EA_ST_ADDRESS_L, ABM_CONFIG, -1, MUIF_NONE),
        Child, Label2(tr(MSG_EA_MLName)),
        Child, ST_REALNAME = MakeString(SIZE_REALNAME, tr(MSG_EA_MLName)),
        Child, Label2(tr(MSG_EA_Description)),
        Child, ST_COMMENT = MakeString(SIZE_DEFAULT, tr(MSG_EA_Description)),
      End,
      Child, VSpace(0),
    End,
    Child, VGroup, GroupFrameT(tr(MSG_EA_Members)),
      Child, NListviewObject,
        MUIA_CycleChain, 1,
        MUIA_Listview_DragType, MUIV_Listview_DragType_Immediate,
        MUIA_NListview_NList, LV_MEMBER = AddressBookEntryListObject,
          InputListFrame,
          MUIA_NList_DragSortable,  TRUE,
          MUIA_NList_ConstructHook, MUIV_NList_ConstructHook_String,
          MUIA_NList_DestructHook,  MUIV_NList_DestructHook_String,
        End,
      End,
      Child, ST_MEMBER = RecipientStringObject,
        MUIA_CycleChain,               TRUE,
        MUIA_String_MaxLen,            SIZE_ADDRESS,
        MUIA_BetterString_NoShortcuts, FALSE,
      End,
      Child, HGroup,
        Child, ColGroup(2),
          MUIA_Group_Spacing, 1,
          MUIA_Group_SameWidth, TRUE,
          MUIA_Weight, 1,
          Child, BT_ADD = MakeButton(MUIX_B "+" MUIX_N),
          Child, BT_DEL = MakeButton(MUIX_B "-" MUIX_N),
        End,
        Child, HSpace(0),
        Child, BT_SORT = MakeButton(tr(MSG_EA_Sort)),
      End,
    End,

    TAG_MORE, inittags(msg))) != NULL)
  {
    GETDATA;

    data->ST_ALIAS =    ST_ALIAS;
    data->ST_REALNAME = ST_REALNAME;
    data->ST_ADDRESS =  ST_ADDRESS;
    data->ST_COMMENT =  ST_COMMENT;
    data->LV_MEMBER =   LV_MEMBER;
    data->ST_MEMBER =   ST_MEMBER;
    data->BT_ADD =      BT_ADD;
    data->BT_DEL =      BT_DEL;

    SetHelp(ST_ALIAS,    MSG_HELP_EA_ST_ALIAS);
    SetHelp(ST_COMMENT,  MSG_HELP_EA_ST_DESCRIPTION);
    SetHelp(ST_REALNAME, MSG_HELP_EA_ST_REALNAME);
    SetHelp(ST_ADDRESS,  MSG_HELP_EA_ST_ADDRESS);
    SetHelp(LV_MEMBER,   MSG_HELP_EA_LV_MEMBERS);
    SetHelp(ST_MEMBER,   MSG_HELP_EA_ST_MEMBER);
    SetHelp(BT_ADD,      MSG_HELP_EA_BT_ADD);
    SetHelp(BT_DEL,      MSG_HELP_EA_BT_DEL);
    SetHelp(BT_SORT,     MSG_HELP_EA_BT_SORT);

    DoMethod(obj, MUIM_MultiSet, MUIA_String_Reject, ",",
      ST_ALIAS,
      ST_ADDRESS,
      ST_REALNAME,
      NULL);

    DoMethod(BT_ADD,    MUIM_Notify, MUIA_Pressed,            FALSE,          obj,       1, METHOD(AddNewMember));
    DoMethod(BT_DEL,    MUIM_Notify, MUIA_Pressed,            FALSE,          LV_MEMBER, 2, MUIM_NList_Remove, MUIV_NList_Remove_Active);
    DoMethod(BT_SORT,   MUIM_Notify, MUIA_Pressed,            FALSE,          LV_MEMBER, 1, MUIM_NList_Sort);
    DoMethod(ST_MEMBER, MUIM_Notify, MUIA_String_Acknowledge, MUIV_EveryTime, obj,       1, METHOD(RenameMember));
    DoMethod(LV_MEMBER, MUIM_Notify, MUIA_NList_Active,       MUIV_EveryTime, obj,       2, METHOD(SetMember), MUIV_TriggerValue);
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
          char *ptr;

          setstring(data->ST_ALIAS, abn->Alias);
          setstring(data->ST_REALNAME, abn->RealName);
          setstring(data->ST_ADDRESS, abn->Address);
          setstring(data->ST_COMMENT, abn->Comment);

          for(ptr = abn->ListMembers; *ptr != '\0'; ptr++)
          {
            char *nptr;

            if((nptr = strchr(ptr, '\n')) != NULL)
              *nptr = '\0';
            else
              break;

            DoMethod(data->LV_MEMBER, MUIM_NList_InsertSingle, ptr, MUIV_NList_Insert_Bottom);

            *nptr = '\n';
            ptr = nptr;
          }
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
        ULONG i;

        // we must not use InitABookNode() here, because that would kill
        // the embedded node structure of old entries
        abn->type = ABNT_LIST;
        GetMUIString(abn->Alias, data->ST_ALIAS, sizeof(abn->Alias));
        GetMUIString(abn->RealName, data->ST_REALNAME, sizeof(abn->RealName));
        GetMUIString(abn->Address, data->ST_ADDRESS, sizeof(abn->Address));
        GetMUIString(abn->Comment, data->ST_COMMENT, sizeof(abn->Comment));

        // free any previous member list before setting up the current one
        dstrfree(abn->ListMembers);
        abn->ListMembers = NULL;

        for(i = 0; ; i++)
        {
          char *p;

          DoMethod(data->LV_MEMBER, MUIM_NList_GetEntry, i, &p);
          if(p == NULL)
            break;

          dstrcat(&abn->ListMembers, p);
          dstrcat(&abn->ListMembers, "\n");
        }
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
/// DECLARE(AddNewMember)
// Adds a new entry to the member list
DECLARE(AddNewMember)
{
  GETDATA;
  char *member;

  ENTER();

  member = (char *)xget(data->ST_MEMBER, MUIA_String_Contents);
  if(IsStrEmpty(member) == FALSE)
  {
    DoMethod(data->LV_MEMBER, MUIM_NList_InsertSingle, member, MUIV_NList_Insert_Bottom);
    DoMethod(data->LV_MEMBER, MUIM_NList_Jump, MUIV_NList_Jump_Bottom);
    setstring(data->ST_MEMBER, "");
  }
  set(obj, MUIA_Window_ActiveObject, data->ST_MEMBER);

  RETURN(0);
  return 0;
}

///
/// DECLARE(SetMember)
//  Fills string gadget with data from selected list entry
DECLARE(SetMember) // ULONG entry
{
  GETDATA;
  char *member = NULL;

  ENTER();

  DoMethod(data->LV_MEMBER, MUIM_NList_GetEntry, MUIV_NList_GetEntry_Active, &member);
  if(member != NULL)
    nnset(data->ST_MEMBER, MUIA_String_Contents, member);

  RETURN(0);
  return 0;
}

///
/// DECLARE(RenameMember)
//  Updates selected list entry
DECLARE(RenameMember)
{
  GETDATA;
  int active;

  ENTER();

  active = xget(data->LV_MEMBER, MUIA_NList_Active);
  if(active == MUIV_List_Active_Off)
  {
    DoMethod(obj, METHOD(AddNewMember));
  }
  else
  {
    char *member = (char *)xget(data->ST_MEMBER, MUIA_String_Contents);

    DoMethod(data->LV_MEMBER, MUIM_NList_ReplaceSingle, member, active, NOWRAP, ALIGN_LEFT);
  }

  RETURN(0);
  return 0;
}

///
