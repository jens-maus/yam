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

 Superclass:  MUIC_Window
 Description: Container window for the attachment reminder keyword setup

***************************************************************************/

#include "AttachmentKeywordWindow_cl.h"

#include <proto/muimaster.h>
#include <libraries/iffparse.h>
#include <mui/NList_mcc.h>
#include <mui/NListview_mcc.h>

#include "YAM.h"

#include "mui/AttachmentKeywordList.h"
#include "mui/YAMApplication.h"

#include "Config.h"
#include "Locale.h"
#include "MUIObjects.h"
#include "Requesters.h"

#include "Debug.h"

/* CLASSDATA
struct Data
{
  Object *NL_KEYWORDS;
  Object *BT_MODIFY;
  Object *BT_REMOVE;

  Object *parentWindow;

  char keyword[SIZE_DEFAULT];
  char screenTitle[SIZE_DEFAULT];
};
*/

/* Overloaded Methods */
/// OVERLOAD(OM_NEW)
OVERLOAD(OM_NEW)
{
  Object *NL_KEYWORDS;
  Object *BT_ADD;
  Object *BT_MODIFY;
  Object *BT_REMOVE;
  Object *BT_OK;
  Object *BT_CANCEL;

  ENTER();

  if((obj = DoSuperNew(cl, obj,

    MUIA_Window_ID, MAKE_ID('A','K','E','Y'),
    WindowContents, VGroup,
      Child, HGroup,
        Child, NListviewObject,
          MUIA_CycleChain, TRUE,
          MUIA_NListview_NList, NL_KEYWORDS = AttachmentKeywordListObject,
          End,
        End,
        Child, VGroup,
          MUIA_Weight, 0,
          Child, BT_ADD = MakeButton(tr(MSG_CO_ATTACHMENT_KEYWORDS_ADD)),
          Child, BT_MODIFY = MakeButton(tr(MSG_CO_ATTACHMENT_KEYWORDS_MODIFY)),
          Child, BT_REMOVE = MakeButton(tr(MSG_CO_ATTACHMENT_KEYWORDS_REMOVE)),
          Child, VSpace(0),
        End,
      End,
      Child, HGroup,
        Child, BT_OK = MakeButton(tr(MSG_Okay)),
        Child, HSpace(0),
        Child, BT_CANCEL = MakeButton(tr(MSG_Cancel)),
      End,
    End,

    TAG_MORE, inittags(msg))) != NULL)
  {
    GETDATA;

    data->NL_KEYWORDS = NL_KEYWORDS;
    data->BT_MODIFY   = BT_MODIFY;
    data->BT_REMOVE   = BT_REMOVE;

    DoMethod(G->App, OM_ADDMEMBER, obj);

    xset(obj,
      MUIA_Window_Title, tr(MSG_CO_ATTACHMENT_KEYWORDS_TITLE),
      MUIA_Window_ScreenTitle, CreateScreenTitle(data->screenTitle, sizeof(data->screenTitle), tr(MSG_CO_ATTACHMENT_KEYWORDS_TITLE)));

    set(NL_KEYWORDS, MUIA_AttachmentKeywordList_Keywords, CE->AttachmentKeywords);
    DoMethod(obj, MUIM_MultiSet, MUIA_Disabled, TRUE,
      data->BT_MODIFY,
      data->BT_REMOVE,
      NULL);

    DoMethod(NL_KEYWORDS, MUIM_Notify, MUIA_NList_Active,        MUIV_EveryTime, obj,                     1, METHOD(UpdateButtons));
    DoMethod(NL_KEYWORDS, MUIM_Notify, MUIA_NList_DoubleClick,   MUIV_EveryTime, obj,                     1, METHOD(ModifyKeyword));
    DoMethod(BT_ADD,      MUIM_Notify, MUIA_Pressed,             FALSE,          obj,                     1, METHOD(AddKeyword));
    DoMethod(BT_MODIFY,   MUIM_Notify, MUIA_Pressed,             FALSE,          obj,                     1, METHOD(ModifyKeyword));
    DoMethod(BT_REMOVE,   MUIM_Notify, MUIA_Pressed,             FALSE,          NL_KEYWORDS,             2, MUIM_NList_Remove, MUIV_NList_Remove_Active);
    DoMethod(BT_OK,       MUIM_Notify, MUIA_Pressed,             FALSE,          obj,                     1, METHOD(Close));
    DoMethod(BT_CANCEL,   MUIM_Notify, MUIA_Pressed,             FALSE,          MUIV_Notify_Application, 2, MUIM_YAMApplication_DisposeWindow, obj);
    DoMethod(obj,         MUIM_Notify, MUIA_Window_CloseRequest, TRUE,           MUIV_Notify_Application, 2, MUIM_YAMApplication_DisposeWindow, obj);

    data->parentWindow = (Object *)GetTagData(MUIA_Window_RefWindow, (IPTR)NULL, inittags(msg));
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

  while((tag = NextTagItem((APTR)&tags)) != NULL)
  {
    switch(tag->ti_Tag)
    {
      case MUIA_Window_Open:
      {
        // let our parent window sleep whenever we are opened
        set(data->parentWindow, MUIA_Window_Sleep, tag->ti_Data);
      }
      break;
    }
  }

  return DoSuperMethodA(cl, obj, msg);
}

///

/* Public Methods */
/// DECLARE(UpdateButtons)
DECLARE(UpdateButtons)
{
  GETDATA;
  char *activeKeyword;

  ENTER();

  DoMethod(data->NL_KEYWORDS, MUIM_NList_GetEntry, MUIV_NList_GetEntry_Active, &activeKeyword);
  DoMethod(obj, MUIM_MultiSet, MUIA_Disabled, activeKeyword == NULL,
    data->BT_MODIFY,
    data->BT_REMOVE,
    NULL);

  RETURN(0);
  return 0;
}

///
/// DECLARE(AddKeyword)
DECLARE(AddKeyword)
{
  GETDATA;

  ENTER();

  data->keyword[0] = '\0';
  if(StringRequest(data->keyword, sizeof(data->keyword), tr(MSG_CO_ATTACHMENT_KEYWORDS_ADD_KEYWORD), tr(MSG_CO_ATTACHMENT_KEYWORDS_KEYWORD), tr(MSG_Okay), NULL, tr(MSG_Cancel), FALSE, obj) != 0)
  {
    DoMethod(data->NL_KEYWORDS, MUIM_AttachmentKeywordList_AddKeyword, data->keyword);
  }

  RETURN(0);
  return 0;
}

///
/// DECLARE(ModifyKeyword)
DECLARE(ModifyKeyword)
{
  GETDATA;
  char *activeKeyword;

  ENTER();

  DoMethod(data->NL_KEYWORDS, MUIM_NList_GetEntry, MUIV_NList_GetEntry_Active, &activeKeyword);
  strlcpy(data->keyword, activeKeyword, sizeof(data->keyword));
  if(StringRequest(data->keyword, sizeof(data->keyword), tr(MSG_CO_ATTACHMENT_KEYWORDS_MODIFY_KEYWORD), tr(MSG_CO_ATTACHMENT_KEYWORDS_KEYWORD), tr(MSG_Okay), NULL, tr(MSG_Cancel), FALSE, obj) != 0)
  {
    DoMethod(data->NL_KEYWORDS, MUIM_AttachmentKeywordList_ModifyKeyword, data->keyword);
  }

  RETURN(0);
  return 0;
}

///
/// DECLARE(Close)
DECLARE(Close)
{
  GETDATA;

  ENTER();

  DoMethod(data->NL_KEYWORDS, MUIM_AttachmentKeywordList_GetKeywords, CE->AttachmentKeywords, sizeof(CE->AttachmentKeywords));
  DoMethod(_app(obj), MUIM_Application_PushMethod, _app(obj), 2, MUIM_YAMApplication_DisposeWindow, obj);

  RETURN(0);
  return 0;
}

///
