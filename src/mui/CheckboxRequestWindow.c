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

 Superclass:  MUIC_Window
 Description:

***************************************************************************/

#include "CheckboxRequestWindow_cl.h"

#include "Locale.h"
#include "MUIObjects.h"

#include "Debug.h"

/* CLASSDATA
struct Data
{
  LONG result;
  ULONG flags;
};
*/

/// CheckboxFunc
//
struct MUIP_CheckboxMsg
{
  ULONG active;
  ULONG *valuePtr;
  ULONG mask;
};

HOOKPROTONHNO(CheckboxFunc, void, struct MUIP_CheckboxMsg *msg)
{
  ENTER();

  if(msg->active)
    *msg->valuePtr |= msg->mask;
  else
    *msg->valuePtr &= ~msg->mask;

  LEAVE();
}
MakeStaticHook(CheckboxHook, CheckboxFunc);

///

/* Overloaded Methods */
/// OVERLOAD(OM_NEW)
OVERLOAD(OM_NEW)
{
  struct TagItem *tags = inittags(msg), *tag;
  char *titleText = NULL;
  char *bodyText = NULL;
  char **entries = NULL;
  Object *checkboxGroup = NULL;
  Object *useButton = NULL;
  Object *cancelButton = NULL;

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

      case ATTR(Entries):
      {
        entries = (char **)tag->ti_Data;
        tag->ti_Tag = TAG_IGNORE;
      }
      break;
    }
  }

  if((obj = DoSuperNew(cl, obj,

    MUIA_Window_Title,    (titleText != NULL) ? titleText : (char *)"YAM",
    MUIA_Window_LeftEdge, MUIV_Window_LeftEdge_Centered,
    MUIA_Window_TopEdge,  MUIV_Window_TopEdge_Centered,
    MUIA_Window_Width,    MUIV_Window_Width_MinMax(20),
    MUIA_Window_Height,   MUIV_Window_Height_MinMax(20),
    WindowContents, VGroup,
      MUIA_Background, MUII_RequesterBack,
      Child, VGroup,
        GroupFrame,
        MUIA_Background, MUII_GroupBack,
        Child, TextObject,
          MUIA_Text_Contents, bodyText,
          MUIA_Text_SetMax,   TRUE,
        End,
        Child, VSpace(4),
        Child, checkboxGroup = ColGroup(3),
          MUIA_Background, MUII_GroupBack,
        End,
      End,
      Child, ColGroup(3),
        Child, useButton = MakeButton(tr(MSG_Use)),
        Child, HSpace(0),
        Child, cancelButton = MakeButton(tr(MSG_Cancel)),
      End,
    End,

    TAG_MORE, inittags(msg))) != NULL)
  {
    GETDATA;

    DoMethod(G->App, OM_ADDMEMBER, obj);

    // prepare the group for the change.
    if(entries != NULL && DoMethod(checkboxGroup, MUIM_Group_InitChange))
    {
      ULONG bit = 0;

      while(*entries != NULL)
      {
        Object *checkboxObj;
        Object *labelObj;
        Object *spaceObj;

        // create the checkbox object now
        checkboxObj = MakeCheck(*entries);
        labelObj = LLabel(*entries);
        spaceObj = HSpace(0);

        if(checkboxObj != NULL && labelObj != NULL)
        {
          set(checkboxObj, MUIA_Selected, TRUE);
          DoMethod(checkboxObj, MUIM_Notify, MUIA_Selected, MUIV_EveryTime, G->App, 5, MUIM_CallHook, &CheckboxHook, MUIV_TriggerValue, &data->flags, (1 << bit));
          DoMethod(checkboxGroup, OM_ADDMEMBER, checkboxObj);
          DoMethod(checkboxGroup, OM_ADDMEMBER, labelObj);
          DoMethod(checkboxGroup, OM_ADDMEMBER, spaceObj);

          // the checkbox is active per default, so we set the corresponding bit in the flags value
          data->flags |= (1 << bit);
          bit++;
        }

        entries++;
      }

      DoMethod(checkboxGroup, MUIM_Group_ExitChange);
    }

    DoMethod(obj, MUIM_Notify, MUIA_Window_CloseRequest, TRUE, obj, 2, MUIM_CheckboxRequestWindow_FinishInput, 0);
    DoMethod(useButton, MUIM_Notify, MUIA_Pressed, FALSE, obj, 2, MUIM_CheckboxRequestWindow_FinishInput, 1);
    DoMethod(cancelButton, MUIM_Notify, MUIA_Pressed, FALSE, obj, 2, MUIM_CheckboxRequestWindow_FinishInput, 0);

    set(obj, MUIA_Window_DefaultObject, useButton);
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
    case ATTR(Result):
    {
      *store = data->result;
      return TRUE;
    }

    case ATTR(Flags):
    {
      *store = data->flags;
      return TRUE;
    }
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

  if(msg->result == 0)
    data->flags = -1;

  data->result = msg->result;

  // trigger possible notifications
  set(obj, MUIA_CheckboxRequestWindow_Result, msg->result);

  RETURN(0);
  return 0;
}

///

