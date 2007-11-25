/***************************************************************************

 YAM - Yet Another Mailer
 Copyright (C) 1995-2000 by Marcel Beck <mbeck@yam.ch>
 Copyright (C) 2000-2007 by YAM Open Source Team

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

#include "ImageCache.h"

#include "Debug.h"

/* CLASSDATA
struct Data
{
  ULONG dummy;
};
*/

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
      { TB_WRITE_EDITOR,    TB_WRITE_EDITOR,    tr(MSG_WR_TBEditor),    tr(MSG_HELP_WR_BT_EDITOR),    0, 0, NULL, NULL },
      { TB_WRITE_INSERT,    TB_WRITE_INSERT,    tr(MSG_WR_TBInsert),    tr(MSG_HELP_WR_BT_LOAD),      0, 0, NULL, NULL },

      { MUIV_TheBar_BarSpacer, -1,  NULL, NULL, 0, 0, NULL, NULL },

      { TB_WRITE_CUT,       TB_WRITE_CUT,       tr(MSG_WR_TBCut),       tr(MSG_HELP_WR_BT_CUT),       0, 0, NULL, NULL },
      { TB_WRITE_COPY,      TB_WRITE_COPY,      tr(MSG_WR_TBCopy),      tr(MSG_HELP_WR_BT_COPY),      0, 0, NULL, NULL },
      { TB_WRITE_PASTE,     TB_WRITE_PASTE,     tr(MSG_WR_TBPaste),     tr(MSG_HELP_WR_BT_PASTE),     0, 0, NULL, NULL },
      { TB_WRITE_UNDO,      TB_WRITE_UNDO,      tr(MSG_WR_TBUndo),      tr(MSG_HELP_WR_BT_UNDO),      0, 0, NULL, NULL },

      { MUIV_TheBar_BarSpacer, -1,  NULL, NULL, 0, 0, NULL, NULL },

      { TB_WRITE_BOLD,      TB_WRITE_BOLD,      tr(MSG_WR_TBBold),      tr(MSG_HELP_WR_BT_BOLD),      0, 0, NULL, NULL },
      { TB_WRITE_ITALIC,    TB_WRITE_ITALIC,    tr(MSG_WR_TBItalic),    tr(MSG_HELP_WR_BT_ITALIC),    0, 0, NULL, NULL },
      { TB_WRITE_UNDERLINE, TB_WRITE_UNDERLINE, tr(MSG_WR_TBUnderlined),tr(MSG_HELP_WR_BT_UNDERL),    0, 0, NULL, NULL },
      { TB_WRITE_COLORED,   TB_WRITE_COLORED,   tr(MSG_WR_TBColored),   tr(MSG_HELP_WR_BT_COLOR),     0, 0, NULL, NULL },

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

      { TB_WRITE_BOLD,      TB_WRITE_BOLD,      NULL,                   NULL,                         0, 0, NULL, NULL },
      { TB_WRITE_ITALIC,    TB_WRITE_ITALIC,    NULL,                   NULL,                         0, 0, NULL, NULL },
      { TB_WRITE_UNDERLINE, TB_WRITE_UNDERLINE, NULL,                   NULL,                         0, 0, NULL, NULL },
      { TB_WRITE_COLORED,   TB_WRITE_COLORED,   NULL,                   NULL,                         0, 0, NULL, NULL },

      { MUIV_TheBar_BarSpacer, -1,  NULL, NULL, 0, 0, NULL, NULL },

      { TB_WRITE_SEARCH,    TB_WRITE_SEARCH,    NULL,                   NULL,                         0, 0, NULL, NULL },

      { MUIV_TheBar_End,       -1,  NULL, NULL, 0, 0, NULL, NULL },
    #endif
    };

    #if defined(__SASC)
    buttons[ 0].text = tr(MSG_WR_TBEditor);     buttons[ 0].help = tr(MSG_HELP_WR_BT_EDITOR);
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
                     MUIA_TheBar_Pics,      G->theme.writeWindowToolbarImages[tbim_Normal],
                     MUIA_TheBar_SelPics,   G->theme.writeWindowToolbarImages[tbim_Selected],
                     MUIA_TheBar_DisPics,   G->theme.writeWindowToolbarImages[tbim_Ghosted],
                     TAG_MORE, inittags(msg));
  }

  // check if the object was created correctly.
  if(obj != NULL)
  {
    // everything worked out fine.
  }
  else
    E(DBF_STARTUP, "couldn't create WriteWindowToolbar!");


  RETURN((ULONG)obj);
  return (ULONG)obj;
}

///

/* Public Methods */
/// DECLARE(InitNotify)
// Method for connecting all notifies which deal with our
// toolbar
DECLARE(InitNotify) // struct WR_ClassData *wrData
{
  struct WR_ClassData *wrData = msg->wrData;

  ENTER();

  // connect the buttons presses
  DoMethod(obj, MUIM_TheBar_Notify, TB_WRITE_EDITOR,    MUIA_Pressed, FALSE, MUIV_Notify_Application, 3, MUIM_CallHook, &WR_EditHook, wrData->winnum);
  DoMethod(obj, MUIM_TheBar_Notify, TB_WRITE_INSERT,    MUIA_Pressed, FALSE, MUIV_Notify_Application, 4, MUIM_CallHook, &WR_EditorCmdHook, ED_INSERT, wrData->winnum);
  DoMethod(obj, MUIM_TheBar_Notify, TB_WRITE_CUT,       MUIA_Pressed, FALSE, wrData->GUI.TE_EDIT, 2, MUIM_TextEditor_ARexxCmd, "CUT");
  DoMethod(obj, MUIM_TheBar_Notify, TB_WRITE_COPY,      MUIA_Pressed, FALSE, wrData->GUI.TE_EDIT, 2, MUIM_TextEditor_ARexxCmd, "COPY");
  DoMethod(obj, MUIM_TheBar_Notify, TB_WRITE_PASTE,     MUIA_Pressed, FALSE, wrData->GUI.TE_EDIT, 2, MUIM_TextEditor_ARexxCmd, "PASTE");
  DoMethod(obj, MUIM_TheBar_Notify, TB_WRITE_UNDO,      MUIA_Pressed, FALSE, wrData->GUI.TE_EDIT, 2, MUIM_TextEditor_ARexxCmd, "UNDO");
  DoMethod(obj, MUIM_TheBar_Notify, TB_WRITE_BOLD,      MUIA_Pressed, FALSE, MUIV_Notify_Application, 4, MUIM_CallHook, &WR_SetSoftStyleHook, SSM_BOLD, wrData->winnum);
  DoMethod(obj, MUIM_TheBar_Notify, TB_WRITE_ITALIC,    MUIA_Pressed, FALSE, MUIV_Notify_Application, 4, MUIM_CallHook, &WR_SetSoftStyleHook, SSM_ITALIC, wrData->winnum);
  DoMethod(obj, MUIM_TheBar_Notify, TB_WRITE_UNDERLINE, MUIA_Pressed, FALSE, MUIV_Notify_Application, 4, MUIM_CallHook, &WR_SetSoftStyleHook, SSM_UNDERLINE, wrData->winnum);
  DoMethod(obj, MUIM_TheBar_Notify, TB_WRITE_COLORED,   MUIA_Pressed, FALSE, MUIV_Notify_Application, 4, MUIM_CallHook, &WR_SetSoftStyleHook, SSM_COLOR, wrData->winnum);
  DoMethod(obj, MUIM_TheBar_Notify, TB_WRITE_SEARCH,    MUIA_Pressed, FALSE, MUIV_Notify_Application, 4, MUIM_CallHook, &WR_SearchHook, wrData->GUI.TE_EDIT, MUIF_NONE);

  // connect attributes to button disables
  DoMethod(wrData->GUI.TE_EDIT, MUIM_Notify, MUIA_TextEditor_AreaMarked, MUIV_EveryTime, obj, 4, MUIM_TheBar_SetAttr, TB_WRITE_CUT, MUIA_TheBar_Attr_Disabled, MUIV_NotTriggerValue);
  DoMethod(wrData->GUI.TE_EDIT, MUIM_Notify, MUIA_TextEditor_AreaMarked, MUIV_EveryTime, obj, 4, MUIM_TheBar_SetAttr, TB_WRITE_COPY, MUIA_TheBar_Attr_Disabled, MUIV_NotTriggerValue);
  DoMethod(wrData->GUI.TE_EDIT, MUIM_Notify, MUIA_TextEditor_UndoAvailable, MUIV_EveryTime, obj, 4, MUIM_TheBar_SetAttr, TB_WRITE_UNDO, MUIA_TheBar_Attr_Disabled, MUIV_NotTriggerValue);

  // connect attributes to button selections
  DoMethod(wrData->GUI.TE_EDIT, MUIM_Notify, MUIA_TextEditor_StyleBold, MUIV_EveryTime, obj, 4, MUIM_TheBar_SetAttr, TB_WRITE_BOLD, MUIA_TheBar_Attr_Selected, MUIV_TriggerValue);
  DoMethod(wrData->GUI.TE_EDIT, MUIM_Notify, MUIA_TextEditor_StyleItalic, MUIV_EveryTime, obj, 4, MUIM_TheBar_SetAttr, TB_WRITE_ITALIC, MUIA_TheBar_Attr_Selected, MUIV_TriggerValue);
  DoMethod(wrData->GUI.TE_EDIT, MUIM_Notify, MUIA_TextEditor_StyleUnderline, MUIV_EveryTime, obj, 4, MUIM_TheBar_SetAttr, TB_WRITE_UNDERLINE, MUIA_TheBar_Attr_Selected, MUIV_TriggerValue);
  DoMethod(wrData->GUI.TE_EDIT, MUIM_Notify, MUIA_TextEditor_Pen, 7, obj, 4, MUIM_TheBar_SetAttr, TB_WRITE_COLORED, MUIA_TheBar_Attr_Selected, TRUE);
  DoMethod(wrData->GUI.TE_EDIT, MUIM_Notify, MUIA_TextEditor_Pen, 0, obj, 4, MUIM_TheBar_SetAttr, TB_WRITE_COLORED, MUIA_TheBar_Attr_Selected, FALSE);
  DoMethod(wrData->GUI.MI_BOLD, MUIM_Notify, MUIA_Menuitem_Checked, MUIV_EveryTime, obj, 4, MUIM_TheBar_SetAttr, TB_WRITE_BOLD, MUIA_TheBar_Attr_Selected, MUIV_TriggerValue);
  DoMethod(wrData->GUI.MI_ITALIC, MUIM_Notify, MUIA_Menuitem_Checked, MUIV_EveryTime, obj, 4, MUIM_TheBar_SetAttr, TB_WRITE_ITALIC, MUIA_TheBar_Attr_Selected, MUIV_TriggerValue);
  DoMethod(wrData->GUI.MI_UNDERLINE, MUIM_Notify, MUIA_Menuitem_Checked, MUIV_EveryTime, obj, 4, MUIM_TheBar_SetAttr, TB_WRITE_UNDERLINE, MUIA_TheBar_Attr_Selected, MUIV_TriggerValue);
  DoMethod(wrData->GUI.MI_COLORED, MUIM_Notify, MUIA_Menuitem_Checked, MUIV_EveryTime, obj, 4, MUIM_TheBar_SetAttr, TB_WRITE_COLORED, MUIA_TheBar_Attr_Selected, MUIV_TriggerValue);

  RETURN(0);
  return 0;
}
///
