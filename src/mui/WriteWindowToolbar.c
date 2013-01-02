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

 Superclass:  MUIC_TheBarVirt
 Description: Toolbar management class of the write window

***************************************************************************/

#include "WriteWindowToolbar_cl.h"

#include "YAM.h"
#include "YAM_config.h"

#include "ImageCache.h"
#include "Locale.h"
#include "Themes.h"

#include "Debug.h"

/* EXPORT
enum { TB_WRITE_EDITOR=0,
       TB_WRITE_INSERT,
       TB_WRITE_CUT,
       TB_WRITE_COPY,
       TB_WRITE_PASTE,
       TB_WRITE_UNDO,
       TB_WRITE_BOLD,
       TB_WRITE_ITALIC,
       TB_WRITE_UNDERLINE,
       TB_WRITE_COLORED,
       TB_WRITE_SEARCH,
       TB_WRITE_NUM
      };
*/

/* Private Data */

/* Private Functions */

/* Overloaded Methods */
/// OVERLOAD(OM_NEW)
OVERLOAD(OM_NEW)
{
  ENTER();

  // depending on whether the write window toolbar
  // exists cached already we go and obtain the images
  // from the cached object instead.
  if(IsToolbarInCache(TBT_WriteWindow))
  {
    // prepare the buttons array which defines how our
    // toolbar looks like.
    struct MUIS_TheBar_Button buttons[TB_WRITE_NUM+4] =
    {
    #if !defined(__SASC)
      { TB_WRITE_EDITOR,    TB_WRITE_EDITOR,    tr(MSG_WR_TBEditor),    tr(MSG_HELP_WR_BT_EDITOR),    C->Editor[0] == '\0' ? MUIV_TheBar_ButtonFlag_Hide : 0, 0, NULL, NULL },
      { TB_WRITE_INSERT,    TB_WRITE_INSERT,    tr(MSG_WR_TBInsert),    tr(MSG_HELP_WR_BT_LOAD),      0, 0, NULL, NULL },

      { MUIV_TheBar_BarSpacer, -1,  NULL, NULL, 0, 0, NULL, NULL },

      { TB_WRITE_CUT,       TB_WRITE_CUT,       tr(MSG_WR_TBCut),       tr(MSG_HELP_WR_BT_CUT),       0, 0, NULL, NULL },
      { TB_WRITE_COPY,      TB_WRITE_COPY,      tr(MSG_WR_TBCopy),      tr(MSG_HELP_WR_BT_COPY),      0, 0, NULL, NULL },
      { TB_WRITE_PASTE,     TB_WRITE_PASTE,     tr(MSG_WR_TBPaste),     tr(MSG_HELP_WR_BT_PASTE),     0, 0, NULL, NULL },
      { TB_WRITE_UNDO,      TB_WRITE_UNDO,      tr(MSG_WR_TBUndo),      tr(MSG_HELP_WR_BT_UNDO),      0, 0, NULL, NULL },

      { MUIV_TheBar_BarSpacer, -1,  NULL, NULL, 0, 0, NULL, NULL },

      { TB_WRITE_BOLD,      TB_WRITE_BOLD,      tr(MSG_WR_TBBold),      tr(MSG_HELP_WR_BT_BOLD),      MUIV_TheBar_ButtonFlag_Toggle, 0, NULL, NULL },
      { TB_WRITE_ITALIC,    TB_WRITE_ITALIC,    tr(MSG_WR_TBItalic),    tr(MSG_HELP_WR_BT_ITALIC),    MUIV_TheBar_ButtonFlag_Toggle, 0, NULL, NULL },
      { TB_WRITE_UNDERLINE, TB_WRITE_UNDERLINE, tr(MSG_WR_TBUnderlined),tr(MSG_HELP_WR_BT_UNDERL),    MUIV_TheBar_ButtonFlag_Toggle, 0, NULL, NULL },
      { TB_WRITE_COLORED,   TB_WRITE_COLORED,   tr(MSG_WR_TBColored),   tr(MSG_HELP_WR_BT_COLOR),     MUIV_TheBar_ButtonFlag_Toggle, 0, NULL, NULL },

      { MUIV_TheBar_BarSpacer, -1,  NULL, NULL, 0, 0, NULL, NULL },

      { TB_WRITE_SEARCH,    TB_WRITE_SEARCH,    tr(MSG_WR_TBSearch),    tr(MSG_HELP_WR_BT_SEARCH),    0, 0, NULL, NULL },

      { MUIV_TheBar_End,       -1,  NULL, NULL, 0, 0, NULL, NULL },
    #else
      { TB_WRITE_EDITOR,    TB_WRITE_EDITOR,    NULL,                   NULL,                         0, 0, NULL, NULL },
      { TB_WRITE_INSERT,    TB_WRITE_INSERT,    NULL,                   NULL,                         0, 0, NULL, NULL },

      { MUIV_TheBar_BarSpacer, -1,  NULL, NULL, 0, 0, NULL, NULL },

      { TB_WRITE_CUT,       TB_WRITE_CUT,       NULL,                   NULL,                         0, 0, NULL, NULL },
      { TB_WRITE_COPY,      TB_WRITE_COPY,      NULL,                   NULL,                         0, 0, NULL, NULL },
      { TB_WRITE_PASTE,     TB_WRITE_PASTE,     NULL,                   NULL,                         0, 0, NULL, NULL },
      { TB_WRITE_UNDO,      TB_WRITE_UNDO,      NULL,                   NULL,                         0, 0, NULL, NULL },

      { MUIV_TheBar_BarSpacer, -1,  NULL, NULL, 0, 0, NULL, NULL },

      { TB_WRITE_BOLD,      TB_WRITE_BOLD,      NULL,                   NULL,                         MUIV_TheBar_ButtonFlag_Toggle, 0, NULL, NULL },
      { TB_WRITE_ITALIC,    TB_WRITE_ITALIC,    NULL,                   NULL,                         MUIV_TheBar_ButtonFlag_Toggle, 0, NULL, NULL },
      { TB_WRITE_UNDERLINE, TB_WRITE_UNDERLINE, NULL,                   NULL,                         MUIV_TheBar_ButtonFlag_Toggle, 0, NULL, NULL },
      { TB_WRITE_COLORED,   TB_WRITE_COLORED,   NULL,                   NULL,                         MUIV_TheBar_ButtonFlag_Toggle, 0, NULL, NULL },

      { MUIV_TheBar_BarSpacer, -1,  NULL, NULL, 0, 0, NULL, NULL },

      { TB_WRITE_SEARCH,    TB_WRITE_SEARCH,    NULL,                   NULL,                         0, 0, NULL, NULL },

      { MUIV_TheBar_End,       -1,  NULL, NULL, 0, 0, NULL, NULL },
    #endif
    };

    #if defined(__SASC)
    buttons[ 0].text = tr(MSG_WR_TBEditor);     buttons[ 0].help = tr(MSG_HELP_WR_BT_EDITOR); buttons[0].flags = C->Editor[0] == '\0' ? MUIV_TheBar_ButtonFlag_Hide : 0;
    buttons[ 1].text = tr(MSG_WR_TBInsert);     buttons[ 1].help = tr(MSG_HELP_WR_BT_LOAD);

    buttons[ 3].text = tr(MSG_WR_TBCut);        buttons[ 3].help = tr(MSG_HELP_WR_BT_CUT);
    buttons[ 4].text = tr(MSG_WR_TBCopy);       buttons[ 4].help = tr(MSG_HELP_WR_BT_COPY);
    buttons[ 5].text = tr(MSG_WR_TBPaste);      buttons[ 5].help = tr(MSG_HELP_WR_BT_PASTE);
    buttons[ 6].text = tr(MSG_WR_TBUndo);       buttons[ 6].help = tr(MSG_HELP_WR_BT_UNDO);

    buttons[ 8].text = tr(MSG_WR_TBBold);       buttons[ 8].help = tr(MSG_HELP_WR_BT_BOLD);
    buttons[ 9].text = tr(MSG_WR_TBItalic);     buttons[ 9].help = tr(MSG_HELP_WR_BT_ITALIC);
    buttons[10].text = tr(MSG_WR_TBUnderlined); buttons[10].help = tr(MSG_HELP_WR_BT_UNDERL);
    buttons[11].text = tr(MSG_WR_TBColored);    buttons[11].help = tr(MSG_HELP_WR_BT_COLOR);

    buttons[13].text = tr(MSG_WR_TBSearch);     buttons[13].help = tr(MSG_HELP_WR_BT_SEARCH);
    #endif

    // create TheBar object with the cached
    // toolbar images
    obj = DoSuperNew(cl, obj,
                     MUIA_Group_Horiz,      TRUE,
                     MUIA_TheBar_Buttons,   buttons,
                     MUIA_TheBar_Images,    ObtainToolbarImages(TBT_WriteWindow, TBI_Normal),
                     MUIA_TheBar_DisImages, ObtainToolbarImages(TBT_WriteWindow, TBI_Ghosted),
                     MUIA_TheBar_SelImages, ObtainToolbarImages(TBT_WriteWindow, TBI_Selected),
                     TAG_MORE, inittags(msg));
  }
  else
  {
    // create the TheBar object, but via loading the images from
    // the corresponding image files.
    obj = DoSuperNew(cl, obj,
                     MUIA_TheBar_Pics,      G->theme.writeWindowToolbarImages[TBIM_NORMAL],
                     MUIA_TheBar_SelPics,   G->theme.writeWindowToolbarImages[TBIM_SELECTED],
                     MUIA_TheBar_DisPics,   G->theme.writeWindowToolbarImages[TBIM_GHOSTED],
                     TAG_MORE, inittags(msg));
  }

  // check if the object was created correctly.
  if(obj != NULL)
  {
    // everything worked out fine.
  }
  else
    E(DBF_STARTUP, "couldn't create WriteWindowToolbar!");


  RETURN((IPTR)obj);
  return (IPTR)obj;
}

///

/* Public Methods */
