
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

 $Id:$

 Superclass:  MUIC_Window
 Description: Show the Birthday Requester

***************************************************************************/

#include "BirthdayRequestWindow_cl.h"

#include "MUIObjects.h"
#include "Debug.h"

/* CLASSDATA
struct Data
{
  short dummy;
};
*/


/* Overloaded Methods */
/// OVERLOAD(OM_NEW)
OVERLOAD(OM_NEW)
{
  struct TagItem *tags = inittags(msg), *tag;
  char *bodyText = NULL;
  char *alias = NULL;
  char *buf = NULL;
  char *yesText = NULL;
  char *noText = NULL;
  Object *yesButton = NULL;
  Object *noButton = NULL;

  ENTER();

  while((tag = NextTagItem((APTR)&tags)) != NULL)
  {
    switch(tag->ti_Tag)
    {
      ATTR(Body):
      {
        bodyText = (char *)tag->ti_Data;
        tag->ti_Tag = TAG_IGNORE;
      }
      break;

      ATTR(Alias):
      {
        alias = (char *)tag->ti_Data;
        tag->ti_Tag = TAG_IGNORE;
      }
      break;
    }
  }

  // split 'Yes|No' into yesText and noText
  noText = (char *)tr(MSG_YesNoReq);
  if((buf = strdup(noText)) != NULL)
  {
    yesText = buf;
    if((noText = strchr(buf, '|')) != NULL)
      *noText++ = '\0';

    if(*yesText == '*')
      yesText++;

    if((obj = DoSuperNew(cl, obj,

      MUIA_Window_Title,       tr(MSG_AB_BirthdayReminder),
      MUIA_Window_LeftEdge,    MUIV_Window_LeftEdge_Centered,
      MUIA_Window_TopEdge,     MUIV_Window_TopEdge_Centered,
      MUIA_Window_Width,       MUIV_Window_Width_MinMax(0),
      MUIA_Window_Height,      MUIV_Window_Height_MinMax(0),
      MUIA_Window_CloseGadget, FALSE,
      MUIA_Window_SizeGadget,  FALSE,
      MUIA_Window_Activate,    TRUE,
      MUIA_Window_NoMenus,     TRUE,
      WindowContents, VGroup,
         MUIA_Background, MUII_RequesterBack,
         InnerSpacing(4, 4),
         Child, HGroup,
           GroupFrame,
           MUIA_Background, MUII_GroupBack,
           Child, HSpace(0),
           Child, TextObject,
             InnerSpacing(4, 4),
             MUIA_Text_Contents, bodyText,
             MUIA_Text_SetMax,   TRUE,
           End,
           Child, HSpace(0),
         End,
        Child, HGroup,
          Child, yesButton = MakeButton(yesText),
          Child, VSpace(0),
          Child, noButton = MakeButton(noText),
        End,
      End,

      TAG_MORE, (ULONG)inittags(msg))) != NULL)
    {
      DoMethod(G->App, OM_ADDMEMBER, obj);

      DoMethod(yesButton, MUIM_Notify, MUIA_Pressed, FALSE, obj, 2, MUIM_BirthdayRequestWindow_FinishInput, alias);
      DoMethod(noButton,  MUIM_Notify, MUIA_Pressed, FALSE, obj, 2, MUIM_BirthdayRequestWindow_FinishInput, NULL);

      xset(obj,
           MUIA_Window_Activate, TRUE,
           MUIA_Window_Open,     TRUE);
      free(buf);
    }
  }
  RETURN(obj);
  return (IPTR)obj;
}

/* Private Functions */

/* Public Methods */
/// DECLARE(FinishInput)
//
DECLARE(FinishInput) // char *alias
{
  ENTER();

  // close the requester window
  set(obj, MUIA_Window_Open, FALSE);

  // remove & dispose the requester object
  DoMethod(G->App, OM_REMMEMBER, obj);
  DoMethod(G->App, MUIM_Application_PushMethod, obj, 1, OM_DISPOSE);

  if(msg->alias != NULL)
  {
    struct WriteMailData *wmData;

    if((wmData = NewWriteMailWindow(NULL, 0)) != NULL)
    {
      xset(wmData->window,
           MUIA_WriteWindow_To,      msg->alias,
           MUIA_WriteWindow_Subject, tr(MSG_AB_HappyBirthday));
    }
  }
  RETURN(0);
  return 0;
}
