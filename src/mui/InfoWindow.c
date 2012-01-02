/***************************************************************************

 YAM - Yet Another Mailer
 Copyright (C) 1995-2000 Marcel Beck
 Copyright (C) 2000-2012 YAM Open Source Team

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

#include "InfoWindow_cl.h"

#include <proto/muimaster.h>

#include "YAM.h"

#include "Locale.h"
#include "MUIObjects.h"

#include "mui/MainWindow.h"

#include "Debug.h"

/* Overloaded Methods */
/// OVERLOAD(OM_NEW)
OVERLOAD(OM_NEW)
{
  struct TagItem *tags = inittags(msg), *tag;
  char *bodyText = NULL;
  BOOL active = TRUE;
  Object *parent = NULL;
  Object *okButton = NULL;

  ENTER();

  while((tag = NextTagItem((APTR)&tags)) != NULL)
  {
    switch(tag->ti_Tag)
    {
      case MUIA_Window_RefWindow:
      {
        parent = (Object *)tag->ti_Data;
        tag->ti_Tag = TAG_IGNORE;
      }
      break;

      case MUIA_Window_Activate:
      {
        active = tag->ti_Data;
        tag->ti_Tag = TAG_IGNORE;
      }
      break;

      case ATTR(Body):
      {
        bodyText = (char *)tag->ti_Data;
        tag->ti_Tag = TAG_IGNORE;
      }
      break;
    }
  }

  if((obj = DoSuperNew(cl, obj,

    MUIA_Window_LeftEdge,  MUIV_Window_LeftEdge_Centered,
    MUIA_Window_TopEdge,   MUIV_Window_TopEdge_Centered,
    MUIA_Window_Activate,  parent != NULL ? (active == TRUE && xget(parent, MUIA_Window_Activate) == TRUE) : active,
    WindowContents, VGroup,
      MUIA_Background, MUII_RequesterBack,
      Child, VGroup,
        GroupFrame,
        MUIA_Background, MUII_GroupBack,
        Child, LLabel(bodyText),
      End,
      Child, HCenter(okButton = MakeButton(tr(MSG_Okay))),
    End,

    TAG_MORE, inittags(msg))) != NULL)
  {
    DoMethod(G->App, OM_ADDMEMBER, obj);

    DoMethod(obj, MUIM_Notify, MUIA_Window_CloseRequest, TRUE, MUIV_Notify_Application, 5, MUIM_Application_PushMethod, parent, 2, MUIM_MainWindow_DisposeSubWindow, obj);
    DoMethod(okButton, MUIM_Notify, MUIA_Pressed, FALSE, MUIV_Notify_Application, 5, MUIM_Application_PushMethod, parent, 2, MUIM_MainWindow_DisposeSubWindow, obj);

    xset(obj, MUIA_Window_DefaultObject, okButton,
              MUIA_Window_Open, TRUE);
  }

  RETURN(obj);
  return (IPTR)obj;
}

///

/* Private Functions */

/* Public Methods */
