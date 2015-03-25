/***************************************************************************

 YAM - Yet Another Mailer
 Copyright (C) 1995-2000 Marcel Beck
 Copyright (C) 2000-2015 YAM Open Source Team

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

 Superclass:  MUIC_Window
 Description: Window to edit an address book entry

***************************************************************************/

#include "AddressBookEditWindow_cl.h"

#include <proto/muimaster.h>
#include <libraries/iffparse.h>

#include "YAM.h"
#include "YAM_error.h"

#include "mui/AddressBookEditGroupPanel.h"
#include "mui/AddressBookEditListPanel.h"
#include "mui/AddressBookEditUserPanel.h"
#include "mui/YAMApplication.h"

#include "AddressBook.h"
#include "Locale.h"
#include "MUIObjects.h"
#include "Requesters.h"

#include "Debug.h"

/* CLASSDATA
struct Data
{
  Object *editPanel;

  enum ABookNodeType type;

  char screenTitle[SIZE_DEFAULT];
};
*/

/* INCLUDE
#include "AddressBook.h"
*/

/* Overloaded Methods */
/// OVERLOAD(OM_NEW)
OVERLOAD(OM_NEW)
{
  Object *BT_OKAY;
  Object *BT_CANCEL;
  enum ABookNodeType type;
  Object *editPanel;

  ENTER();

  type = (enum ABookNodeType)GetTagData(ATTR(Type), ABNT_USER, inittags(msg));
  switch(type)
  {
    default:
    case ABNT_USER:  editPanel = AddressBookEditUserPanelObject, End; break;
    case ABNT_GROUP: editPanel = AddressBookEditGroupPanelObject, End; break;
    case ABNT_LIST:  editPanel = AddressBookEditListPanelObject, End; break;
  }

  if((obj = DoSuperNew(cl, obj,
    MUIA_HelpNode, "Windows/Addressbook#Enteringaddresses",
    MUIA_Window_ID, MAKE_ID('E','D','A','D'),

    WindowContents, VGroup,
      Child, editPanel,
      Child, ColGroup(3),
        Child, BT_OKAY = MakeButton(tr(MSG_Okay)),
        Child, HSpace(0),
        Child, BT_CANCEL = MakeButton(tr(MSG_Cancel)),
      End,
    End,

    TAG_MORE, inittags(msg))) != NULL)
  {
    GETDATA;

    data->editPanel = editPanel;
    data->type = type;

    DoMethod(G->App, OM_ADDMEMBER, obj);

    SetHelp(BT_OKAY,   MSG_HELP_EA_BT_OKAY);
    SetHelp(BT_CANCEL, MSG_HELP_EA_BT_CANCEL);

    DoMethod(BT_CANCEL,   MUIM_Notify, MUIA_Pressed,             FALSE, obj, 1, METHOD(Close));
    DoMethod(BT_OKAY,     MUIM_Notify, MUIA_Pressed,             FALSE, obj, 3, MUIM_Set, ATTR(SaveContents), TRUE);
    DoMethod(obj,         MUIM_Notify, MUIA_Window_CloseRequest, TRUE,  obj, 1, METHOD(Close));
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
      case ATTR(ABookNode):
      {
        struct ABookNode *abn = (struct ABookNode *)tag->ti_Data;
        const char *title;

        // forward this attribute to the edit panel
        set(data->editPanel, tag->ti_Tag, tag->ti_Data);

        switch(data->type)
        {
          default:
          case ABNT_USER:
            title = (abn != NULL) ? tr(MSG_EA_EditUser) : tr(MSG_AB_AddUser);
          break;

          case ABNT_GROUP:
            title = (abn != NULL) ? tr(MSG_EA_EditGroup) : tr(MSG_AB_AddGroup);
          break;

          case ABNT_LIST:
            title = (abn != NULL) ? tr(MSG_EA_EditList) : tr(MSG_AB_AddList);
          break;
        }

        xset(obj,
          MUIA_Window_Title, title,
          MUIA_Window_ScreenTitle, CreateScreenTitle(data->screenTitle, sizeof(data->screenTitle), title));

        // make the superMethod call ignore those tags
        tag->ti_Tag = TAG_IGNORE;
      }
      break;

      case ATTR(Address):
      {
        // forward this attribute to the edit panel
        set(data->editPanel, tag->ti_Tag, tag->ti_Data);
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
      // forward this method to the edit panel
      DoMethodA(data->editPanel, msg);
      return TRUE;
    }
    break;

    case ATTR(SaveContents):
    {
      *store = TRUE;
      return TRUE;
    }
    break;
  }

  return DoSuperMethodA(cl, obj, msg);
}

///

/* Public Methods */
/// DECLARE(Close)
DECLARE(Close)
{
  ENTER();

  DoMethod(_app(obj), MUIM_Application_PushMethod, _app(obj), 2, MUIM_YAMApplication_DisposeWindow, obj);

  RETURN(0);
  return 0;
}

///
