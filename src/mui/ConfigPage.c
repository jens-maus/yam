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
 MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.   See the
 GNU General Public License for more details.

 You should have received a copy of the GNU General Public License
 along with this program; if not, write to the Free Software
 Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA

 YAM Official Support Site : http://www.yam.ch
 YAM OpenSource project     : http://sourceforge.net/projects/yamos/

 $Id$

 Superclass:  MUIC_Group
 Description: Base class for pages in the config window

***************************************************************************/

#include "ConfigPage_cl.h"

#include <proto/muimaster.h>

#include "YAM.h"

#include "mui/ImageArea.h"

#include "Debug.h"

/* INCLUDE
#include "YAM_config.h"
#include "Locale.h"
#include "MUIObjects.h"
#include "Themes.h"
*/

/* Overloaded Methods */
/// OVERLOAD(OM_NEW)
OVERLOAD(OM_NEW)
{
  ULONG imageID;
  ULONG image;
  char *title;
  char *summary;
  Object *contents;

  ENTER();

  imageID = GetTagData(ATTR(ImageID), (IPTR)0, inittags(msg));
  image = GetTagData(ATTR(Image), (IPTR)0, inittags(msg));
  title = (char *)GetTagData(ATTR(Title), (IPTR)NULL, inittags(msg));
  summary = (char *)GetTagData(ATTR(Summary), (IPTR)NULL, inittags(msg));
  contents = (Object *)GetTagData(ATTR(Contents), (IPTR)NULL, inittags(msg));

  if((obj = DoSuperNew(cl, obj,
    MUIA_Group_Horiz, FALSE,
    Child, HGroup,
      Child, MakeImageObject(imageID, G->theme.configImages[image]),
      Child, VGroup,
        Child, TextObject,
          MUIA_Text_PreParse, "\033b",
          MUIA_Text_Contents, title,
          MUIA_Text_Copy,     FALSE,
        End,
        Child, TextObject,
          MUIA_Text_Contents, summary,
          MUIA_Text_Copy,     FALSE,
          MUIA_Font,          MUIV_Font_Tiny,
        End,
      End,
    End,
    Child, RectangleObject,
      MUIA_Rectangle_HBar, TRUE,
      MUIA_FixHeight,      4,
    End,
  TAG_MORE, inittags(msg))) != NULL)
  {
    if(GetTagData(ATTR(UseScrollgroup), FALSE, inittags(msg)) == TRUE)
    {
      // add a scrollgroup with the given contents as virtual group
      DoMethod(obj, OM_ADDMEMBER, ScrollgroupObject,
        MUIA_Scrollgroup_FreeHoriz, FALSE,
        MUIA_Scrollgroup_AutoBars, TRUE,
        MUIA_Scrollgroup_Contents, VGroupV,
          Child, contents,
        End,
      End);
    }
    else
    {
      // just add the contents and a spacer object at the bottom
      DoMethod(obj, OM_ADDMEMBER, contents);
      DoMethod(obj, OM_ADDMEMBER, HVSpace);
    }
  }

  RETURN((IPTR)obj);
  return (IPTR)obj;
}

///
/// DECLARE(ConfigToGUI)
DECLARE(ConfigToGUI)
{
  ENTER();

  E(DBF_GUI, "derived class did not overload MUIM_ConfigPage_ConfigToGUI method");

  RETURN(0);
  return 0;
}

///
/// DECLARE(GUIToConfig)
DECLARE(GUIToConfig)
{
  ENTER();

  E(DBF_GUI, "derived class did not overload MUIM_ConfigPage_GUIToConfig method");

  RETURN(0);
  return 0;
}

///
