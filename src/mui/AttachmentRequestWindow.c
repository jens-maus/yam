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

#include "AttachmentRequestWindow_cl.h"

#include <string.h>
#include <proto/muimaster.h>
#include <libraries/iffparse.h>
#include <mui/NList_mcc.h>
#include <mui/NListview_mcc.h>

#include "YAM.h"
#include "YAM_mainFolder.h"
#include "YAM_read.h"

#include "Locale.h"
#include "MUIObjects.h"
#include "Requesters.h"

#include "mui/AttachmentList.h"

#include "Debug.h"

/* CLASSDATA
struct Data
{
  LONG result;
  Object *listObj;
};
*/

/* Overloaded Methods */
/// OVERLOAD(OM_NEW)
//
OVERLOAD(OM_NEW)
{
  struct TagItem *tags = inittags(msg), *tag;
  char *titleText = NULL;
  char *bodyText = NULL;
  char *yesText = (char *)tr(MSG_Okay);
  char *noText = (char *)tr(MSG_Cancel);
  Object *listObj = NULL;
  Object *yesButton = NULL;
  Object *noButton = NULL;
  struct ReadMailData *rmData = NULL;
  int mode = 0;

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

      case ATTR(Mode):
      {
        mode = tag->ti_Data;
        tag->ti_Tag = TAG_IGNORE;
      }
      break;

      case ATTR(ReadMailData):
      {
        rmData = (struct ReadMailData *)tag->ti_Data;
        tag->ti_Tag = TAG_IGNORE;
      }
      break;
    }
  }

  if((obj = DoSuperNew(cl, obj,

    MUIA_Window_Title,      (titleText != NULL) ? titleText : (char *)"YAM",
    MUIA_Window_ID,         MAKE_ID('A','R','E','Q'),
    MUIA_Window_RefWindow,  rmData->readWindow,
    MUIA_Window_LeftEdge,   MUIV_Window_LeftEdge_Centered,
    MUIA_Window_TopEdge,    MUIV_Window_TopEdge_Centered,
    WindowContents, VGroup,
      MUIA_Background, MUII_RequesterBack,
      Child, VGroup,
        GroupFrame,
        MUIA_Background, MUII_GroupBack,
        Child, LLabel(bodyText),
        Child, NListviewObject,
          MUIA_CycleChain, TRUE,
          MUIA_NListview_NList, listObj = AttachmentListObject,
            MUIA_NList_MultiSelect, isMultiReq(mode) ? MUIV_NList_MultiSelect_Default : MUIV_NList_MultiSelect_None,
          End,
        End,
      End,
      Child, ColGroup(3),
        Child, yesButton = MakeButton(yesText),
        Child, HSpace(0),
        Child, noButton = MakeButton(noText),
      End,
    End,

    TAG_MORE, inittags(msg))) != NULL)
  {
    GETDATA;
    struct Part *part;

    DoMethod(G->App, OM_ADDMEMBER, obj);

    data->listObj = listObj;

    // lets create the static parts of the Attachrequest entries in the NList
    G->virtualMailpart[0]->Nr = PART_ORIGINAL;
    strlcpy(G->virtualMailpart[0]->Name, tr(MSG_RE_Original), sizeof(G->virtualMailpart[0]->Name));
    G->virtualMailpart[0]->Size = rmData->mail->Size;
    SET_FLAG(G->virtualMailpart[0]->Flags, PFLAG_DECODED);
    DoMethod(listObj, MUIM_NList_InsertSingle, G->virtualMailpart[0], MUIV_NList_Insert_Top);
    set(listObj, MUIA_NList_Active, MUIV_NList_Active_Top);

    // if this AttachRequest isn't a DISPLAY request we show all the option to select the text we actually see
    if(!isDisplayReq(mode))
    {
      G->virtualMailpart[1]->Nr = PART_ALLTEXT;
      strlcpy(G->virtualMailpart[1]->Name, tr(MSG_RE_AllTexts), sizeof(G->virtualMailpart[1]->Name));
      G->virtualMailpart[1]->Size = 0;

      DoMethod(listObj, MUIM_NList_InsertSingle, G->virtualMailpart[1], MUIV_NList_Insert_Bottom);
    }

    // now we process the mail and pick every part out to the NListview
    for(part = rmData->firstPart->Next; part; part = part->Next)
    {
      if(!isPrintReq(mode) || isPrintable(part))
      {
        DoMethod(listObj, MUIM_NList_InsertSingle, part, MUIV_NList_Insert_Bottom);
      }
    }

    DoMethod(obj, MUIM_Notify, MUIA_Window_CloseRequest, TRUE, obj, 2, MUIM_AttachmentRequestWindow_FinishInput, 0);
    DoMethod(listObj, MUIM_Notify, MUIA_NList_DoubleClick, MUIV_EveryTime, obj, 2, MUIM_AttachmentRequestWindow_FinishInput, 1);
    DoMethod(yesButton, MUIM_Notify, MUIA_Pressed, FALSE, obj, 2, MUIM_AttachmentRequestWindow_FinishInput, 1);
    DoMethod(noButton, MUIM_Notify, MUIA_Pressed, FALSE, obj, 2, MUIM_AttachmentRequestWindow_FinishInput, 0);

    set(obj, MUIA_Window_DefaultObject, listObj);
  }

  RETURN(obj);
  return (IPTR)obj;
}

///
/// OVERLOAD(OM_GET)
//
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

    case ATTR(Part):
    {
      struct Part *result = NULL;
      struct Part *part;
      struct Part *prevPart = NULL;
      LONG id;

      // now we pass through every selected entry and add it to the next part.
      for(id = MUIV_NList_NextSelected_Start; ; prevPart = part)
      {
        DoMethod(data->listObj, MUIM_NList_NextSelected, &id);
        if(id == MUIV_NList_NextSelected_End)
          break;

        DoMethod(data->listObj, MUIM_NList_GetEntry, id, &part);

        // we have to set NextSelected to NULL first
        part->NextSelected = NULL;

        if(result == NULL)
          result = part;
        else
          prevPart->NextSelected = part;
      }

      *((struct Part **)store) = result;
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

  data->result = msg->result;

  // trigger possible notifications
  set(obj, MUIA_AttachmentRequestWindow_Result, msg->result);

  RETURN(0);
  return 0;
}

///

