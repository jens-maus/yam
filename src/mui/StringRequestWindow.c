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

 Superclass:  MUIC_Window
 Description: Puts up a string requester

***************************************************************************/

#include "StringRequestWindow_cl.h"

#include <string.h>
#include <proto/muimaster.h>
#include <libraries/iffparse.h>

#include "YAM.h"
#include "YAM_stringsizes.h"

#include "Locale.h"
#include "MUIObjects.h"
#include "Threads.h"

#include "Debug.h"

/* CLASSDATA
struct Data
{
  Object *stringObj;
  ULONG maxLength;
  ULONG result;
  APTR thread;
  char screenTitle[SIZE_DEFAULT];
};
*/

/* Overloaded Methods */
/// OVERLOAD(OM_NEW)
OVERLOAD(OM_NEW)
{
  struct TagItem *tags = inittags(msg), *tag;
  char *titleText = NULL;
  char *bodyText = NULL;
  char *yesText = (char *)tr(MSG_Okay);
  char *noText = (char *)tr(MSG_Cancel);
  char *altText = NULL;
  char *stringContents = (char *)"";
  BOOL secret = FALSE;
  Object *stringObj = NULL;
  Object *yesButton = NULL;
  Object *noButton = NULL;
  Object *altButton = NULL;
  ULONG maxLength = SIZE_DEFAULT;
  APTR thread = NULL;

  ENTER();

  while((tag = NextTagItem((APTR)&tags)) != NULL)
  {
    switch(tag->ti_Tag)
    {
      case MUIA_Window_Title:
      {
        titleText = (char *)tag->ti_Data;
        tag->ti_Tag = TAG_IGNORE;
      }
      break;

      case ATTR(Body):
      {
        bodyText = (char *)tag->ti_Data;
        tag->ti_Tag = TAG_IGNORE;
      }
      break;

      case ATTR(Secret):
      {
        secret = tag->ti_Data;
        tag->ti_Tag = TAG_IGNORE;
      }
      break;

      case ATTR(YesText):
      {
        yesText = (char *)tag->ti_Data;
        tag->ti_Tag = TAG_IGNORE;
      }
      break;

      case ATTR(NoText):
      {
        noText = (char *)tag->ti_Data;
        tag->ti_Tag = TAG_IGNORE;
      }
      break;

      case ATTR(AlternativeText):
      {
        altText = (char *)tag->ti_Data;
        tag->ti_Tag = TAG_IGNORE;
      }
      break;

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

      case ATTR(Thread):
      {
        thread = (APTR)tag->ti_Data;
        tag->ti_Tag = TAG_IGNORE;
      }
      break;
    }
  }

  if((obj = DoSuperNew(cl, obj,

    MUIA_Window_LeftEdge,  MUIV_Window_LeftEdge_Centered,
    MUIA_Window_TopEdge,   MUIV_Window_TopEdge_Centered,
    MUIA_Window_Width,     MUIV_Window_Width_MinMax(20),
    MUIA_Window_Height,    MUIV_Window_Height_MinMax(20),
    MUIA_Window_ID,        MAKE_ID('S','R','E','Q'),
    WindowContents, VGroup,
      MUIA_Background, MUII_RequesterBack,
      Child, VGroup,
        GroupFrame,
        MUIA_Background, MUII_GroupBack,
        Child, LLabel(bodyText),
        Child, stringObj = secret ? MakePassString("") : MakeString(maxLength, ""),
      End,
      Child, ColGroup(3),
        Child, yesButton = MakeButton(yesText),
        Child, altButton = (altText != NULL) ? MakeButton(altText) : HSpace(0),
        Child, noButton = MakeButton(noText),
      End,
    End,

    TAG_MORE, inittags(msg))) != NULL)
  {
    GETDATA;

    DoMethod(G->App, OM_ADDMEMBER, obj);

    data->stringObj = stringObj;
    data->maxLength = maxLength;
    data->thread = thread;
    data->result = 0;

    DoMethod(obj, MUIM_Notify, MUIA_Window_CloseRequest, TRUE, obj, 2, METHOD(FinishInput), 0);
    DoMethod(yesButton, MUIM_Notify, MUIA_Pressed, FALSE, obj, 2, METHOD(FinishInput), 1);
    DoMethod(noButton, MUIM_Notify, MUIA_Pressed, FALSE, obj, 2, METHOD(FinishInput), 0);
    DoMethod(stringObj, MUIM_Notify, MUIA_String_Acknowledge, MUIV_EveryTime, obj, 2, METHOD(FinishInput), 1);

    if(altText != NULL)
      DoMethod(altButton, MUIM_Notify, MUIA_Pressed, FALSE, obj, 2, METHOD(FinishInput), 2);

    set(stringObj, MUIA_String_Contents, stringContents);

    xset(obj, MUIA_Window_ActiveObject, stringObj,
              MUIA_Window_Title, titleText != NULL ? titleText : "YAM",
              MUIA_Window_ScreenTitle, CreateScreenTitle(data->screenTitle, sizeof(data->screenTitle), titleText));
  }

  RETURN((IPTR)obj);
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
  set(obj, MUIA_StringRequestWindow_Result, msg->result);

  // wake up a possibly sleeping thread
  if(data->thread != NULL)
    WakeupThread(data->thread);

  RETURN(0);
  return 0;
}

///
