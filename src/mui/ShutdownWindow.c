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
 Description: Shutdown window of the application

***************************************************************************/

#include "ShutdownWindow_cl.h"

#include <proto/dos.h>
#include <proto/muimaster.h>

#include "YAM.h"

#include "Locale.h"

#include "mui/ImageArea.h"

#include "Debug.h"

/* Overloaded Methods */
/// OVERLOAD(OM_NEW)
OVERLOAD(OM_NEW)
{
  char logopath[SIZE_PATHFILE];

  ENTER();

  AddPath(logopath, G->ProgDir, "Themes/default/logo", sizeof(logopath));

  // create the object
  if((obj = DoSuperNew(cl, obj,

    MUIA_Window_DragBar,        FALSE,
    MUIA_Window_CloseGadget,    FALSE,
    MUIA_Window_DepthGadget,    FALSE,
    MUIA_Window_SizeGadget,     FALSE,
    MUIA_Window_Activate,       FALSE,
    MUIA_Window_LeftEdge,       MUIV_Window_LeftEdge_Centered,
    MUIA_Window_TopEdge,        MUIV_Window_TopEdge_Centered,
    MUIA_Window_ActiveObject,   NULL,
    MUIA_Window_DefaultObject,  NULL,
    WindowContents, VGroup,
      MUIA_Background, MUII_GroupBack,
      Child, HGroup,
        MUIA_Group_Spacing, 0,
        Child, HSpace(0),
        Child, MakeImageObject("logo", logopath),
        Child, HSpace(0),
      End,
      Child, HCenter((VGroup,
        Child, CLabel(tr(MSG_SHUTDOWN_SHUTTING_DOWN)),
      End)),
    End,

    TAG_MORE, inittags(msg))) != NULL)
  {
    // add the window to our application
    DoMethod(G->App, OM_ADDMEMBER, obj);

    // make sure it will be opened immediately.
    set(obj, MUIA_Window_Open, TRUE);
  }

  RETURN((IPTR)obj);
  return (IPTR)obj;
}

///

/* Private Functions */

/* Public Methods */

