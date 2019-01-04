/***************************************************************************

 YAM - Yet Another Mailer
 Copyright (C) 1995-2000 Marcel Beck
 Copyright (C) 2000-2019 YAM Open Source Team

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
 Description: Puts up a string requester for entering a PGP passphrase

***************************************************************************/

#include "PassphraseRequestWindow_cl.h"

#include <string.h>

#include <proto/muimaster.h>
#include <libraries/iffparse.h>

#include "YAM.h"
#include "YAM_stringsizes.h"

#include "Config.h"
#include "Locale.h"
#include "MUIObjects.h"

#include "Debug.h"

/* CLASSDATA
struct Data
{
  Object *stringObj;
  Object *rememberObj;
  ULONG maxLength;
  ULONG result;

  char screenTitle[SIZE_DEFAULT];
};
*/

/* Overloaded Methods */
/// OVERLOAD(OM_NEW)
OVERLOAD(OM_NEW)
{
  struct TagItem *tags = inittags(msg), *tag;
  char *stringContents = (char *)"";
  Object *stringObj = NULL;
  Object *rememberObj = NULL;
  Object *showCharsObj = NULL;
  Object *yesButton = NULL;
  Object *noButton = NULL;
  ULONG maxLength = SIZE_DEFAULT;
  char pgprem[SIZE_DEFAULT];

  ENTER();

  while((tag = NextTagItem((APTR)&tags)) != NULL)
  {
    switch(tag->ti_Tag)
    {
      case ATTR(StringContents):
      {
        stringContents = (char *)tag->ti_Data;
        tag->ti_Tag = TAG_IGNORE;
      }
      break;

      case ATTR(MaxLength):
      {
        maxLength = tag->ti_Data;
        tag->ti_Tag = TAG_IGNORE;
      }
      break;
    }
  }

  snprintf(pgprem, sizeof(pgprem), "%s %d %s", tr(MSG_CO_PGPPASSINTERVAL1),
                                               abs(C->PGPPassInterval),
                                               tr(MSG_CO_PGPPASSINTERVAL2));

  if((obj = DoSuperNew(cl, obj,

    MUIA_Window_LeftEdge,  MUIV_Window_LeftEdge_Centered,
    MUIA_Window_TopEdge,   MUIV_Window_TopEdge_Centered,
    MUIA_Window_Width,     MUIV_Window_Width_MinMax(20),
    MUIA_Window_Height,    MUIV_Window_Height_MinMax(20),
    MUIA_Window_ID,        MAKE_ID('P','R','E','Q'),
    WindowContents, VGroup,
      MUIA_Background, MUII_RequesterBack,
      Child, VGroup,
        GroupFrame,
        MUIA_Background, MUII_GroupBack,
        Child, LLabel(tr(MSG_UT_PGPPassReq)),
        Child, stringObj = MakePassString(""),
        Child, HGroup,
          Child, showCharsObj = MakeCheck(tr(MSG_PGPPASS_SHOW_CHARS)),
          Child, Label2(tr(MSG_PGPPASS_SHOW_CHARS)),
          Child, HSpace(0),
        End,
        Child, HGroup,
          Child, rememberObj = MakeCheck(tr(MSG_CO_PGPPASSINTERVAL1)),
          Child, Label2(pgprem),
          Child, HSpace(0),
        End,
      End,
      Child, ColGroup(3),
        Child, yesButton = MakeButton(tr(MSG_Okay)),
        Child, HSpace(0),
        Child, noButton = MakeButton(tr(MSG_Cancel)),
      End,
    End,

    TAG_MORE, inittags(msg))) != NULL)
  {
    GETDATA;

    DoMethod(G->App, OM_ADDMEMBER, obj);

    data->stringObj = stringObj;
    data->rememberObj = rememberObj;
    data->maxLength = maxLength;
    data->result = 0;

    DoMethod(obj, MUIM_Notify, MUIA_Window_CloseRequest, TRUE, obj, 2, MUIM_PassphraseRequestWindow_FinishInput, 0);
    DoMethod(showCharsObj, MUIM_Notify, MUIA_Selected, MUIV_EveryTime, stringObj, 3, MUIM_Set, MUIA_String_Secret, MUIV_NotTriggerValue);
    DoMethod(yesButton, MUIM_Notify, MUIA_Pressed, FALSE, obj, 2, MUIM_PassphraseRequestWindow_FinishInput, 1);
    DoMethod(noButton, MUIM_Notify, MUIA_Pressed, FALSE, obj, 2, MUIM_PassphraseRequestWindow_FinishInput, 0);
    DoMethod(stringObj, MUIM_Notify, MUIA_String_Acknowledge, MUIV_EveryTime, obj, 2, MUIM_PassphraseRequestWindow_FinishInput, 1);

    set(stringObj, MUIA_String_Contents, stringContents);
    set(rememberObj, MUIA_Selected, C->PGPPassInterval > 0);

    xset(obj, MUIA_Window_ActiveObject, stringObj,
              MUIA_Window_Title,        tr(MSG_UT_PGPPASSREQ_TITLE),
              MUIA_Window_ScreenTitle,  CreateScreenTitle(data->screenTitle, sizeof(data->screenTitle), tr(MSG_UT_PGPPASSREQ_TITLE)));
  }

  RETURN(obj);
  return (IPTR)obj;
}

///
/// OVERLOAD(OM_GET)
OVERLOAD(OM_GET)
{
  GETDATA;
  IPTR *store = ((struct opGet *)msg)->opg_Storage;

  switch(((struct opGet *)msg)->opg_AttrID)
  {
    case ATTR(Result): *store = data->result; return TRUE;
    case ATTR(StringContents): GetMUIString((char *)store, data->stringObj, data->maxLength); return TRUE;
    case ATTR(RememberPhrase): *store = GetMUICheck(data->rememberObj); return TRUE;
  }

  return DoSuperMethodA(cl, obj, msg);
}

///

/* Private Functions */

/* Public Methods */
/// DECLARE(FinishInput)
//
DECLARE(FinishInput) // ULONG result
{
  GETDATA;

  ENTER();

  data->result = msg->result;

  // trigger possible notifications
  set(obj, MUIA_PassphraseRequestWindow_Result, msg->result);

  RETURN(0);
  return 0;
}

///

