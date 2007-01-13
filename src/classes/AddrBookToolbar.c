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
 Description: Toolbar management class of the addressbook

***************************************************************************/

#include "AddrBookToolbar_cl.h"

#include "Debug.h"

/* CLASSDATA
struct Data
{
  ULONG dummy;
};
*/

/* EXPORT
enum { TB_ABOOK_SAVE=0,
       TB_ABOOK_FIND,
       TB_ABOOK_NEWUSER,
       TB_ABOOK_NEWLIST,
       TB_ABOOK_NEWGROUP,
       TB_ABOOK_EDIT,
       TB_ABOOK_DELETE,
       TB_ABOOK_PRINT,
       TB_ABOOK_OPENTREE,
       TB_ABOOK_CLOSETREE,
       TB_ABOOK_NUM
};
*/

/* Private Data */

/* Private Functions */

/* Overloaded Methods */
/// OVERLOAD(OM_NEW)
OVERLOAD(OM_NEW)
{
  // define the image arrays
  static const char *normalImages[TB_ABOOK_NUM+1] =
  {
    "Abook_Save",      // Save
    "Abook_Find",      // Find
    "Abook_NewUser",   // New User
    "Abook_NewList",   // New List
    "Abook_NewGroup",  // New Group
    "Abook_Edit",      // Edit
    "Abook_Delete",    // Delete
    "Abook_Print",     // Print
    "Abook_OpenTree",  // Open Tree
    "Abook_CloseTree", // Close Tree
    NULL
  };

  static const char *selectedImages[TB_ABOOK_NUM+1] =
  {
    "Abook_Save_S",      // Save
    "Abook_Find_S",      // Find
    "Abook_NewUser_S",   // New User
    "Abook_NewList_S",   // New List
    "Abook_NewGroup_S",  // New Group
    "Abook_Edit_S",      // Edit
    "Abook_Delete_S",    // Delete
    "Abook_Print_S",     // Print
    "Abook_OpenTree_S",  // Open Tree
    "Abook_CloseTree_S", // Close Tree
    NULL
  };

  static const char *ghostedImages[TB_ABOOK_NUM+1] =
  {
    "Abook_Save_G",      // Save
    "Abook_Find_G",      // Find
    "Abook_NewUser_G",   // New User
    "Abook_NewList_G",   // New List
    "Abook_NewGroup_G",  // New Group
    "Abook_Edit_G",      // Edit
    "Abook_Delete_G",    // Delete
    "Abook_Print_G",     // Print
    "Abook_OpenTree_G",  // Open Tree
    "Abook_CloseTree_G", // Close Tree
    NULL
  };

  // prepare the buttons array which defines how our
  // toolbar looks like.
  struct MUIS_TheBar_Button buttons[TB_ABOOK_NUM+3] =
  {
    { TB_ABOOK_SAVE,      TB_ABOOK_SAVE,      tr(MSG_AB_TBSave),      tr(MSG_HELP_AB_BT_SAVE),      0, 0, NULL, NULL },
    { TB_ABOOK_FIND,      TB_ABOOK_FIND,      tr(MSG_AB_TBFind),      tr(MSG_HELP_AB_BT_SEARCH),    0, 0, NULL, NULL },

    { MUIV_TheBar_BarSpacer, -1,  NULL, NULL, 0, 0, NULL, NULL },

    { TB_ABOOK_NEWUSER,   TB_ABOOK_NEWUSER,   tr(MSG_AB_TBNewUser),   tr(MSG_HELP_AB_BT_ADDUSER),   0, 0, NULL, NULL },
    { TB_ABOOK_NEWLIST,   TB_ABOOK_NEWLIST,   tr(MSG_AB_TBNewList),   tr(MSG_HELP_AB_BT_ADDMLIST),  0, 0, NULL, NULL },
    { TB_ABOOK_NEWGROUP,  TB_ABOOK_NEWGROUP,  tr(MSG_AB_TBNewGroup),  tr(MSG_HELP_AB_BT_ADDGROUP),  0, 0, NULL, NULL },
    { TB_ABOOK_EDIT,      TB_ABOOK_EDIT,      tr(MSG_AB_TBEdit),      tr(MSG_HELP_AB_BT_EDIT),      0, 0, NULL, NULL },
    { TB_ABOOK_DELETE,    TB_ABOOK_DELETE,    tr(MSG_AB_TBDelete),    tr(MSG_HELP_AB_BT_DELETE),    0, 0, NULL, NULL },
    { TB_ABOOK_PRINT,     TB_ABOOK_PRINT,     tr(MSG_AB_TBPrint),     tr(MSG_HELP_AB_BT_PRINT),     0, 0, NULL, NULL },

    { MUIV_TheBar_BarSpacer, -1,  NULL, NULL, 0, 0, NULL, NULL },

    { TB_ABOOK_OPENTREE,  TB_ABOOK_OPENTREE,  tr(MSG_AB_TBOpenTree),  tr(MSG_HELP_AB_BT_OPEN),      0, 0, NULL, NULL },
    { TB_ABOOK_CLOSETREE, TB_ABOOK_CLOSETREE, tr(MSG_AB_TBCloseTree), tr(MSG_HELP_AB_BT_CLOSE),     0, 0, NULL, NULL },

    { MUIV_TheBar_End, -1,        NULL, NULL, 0, 0, NULL, NULL },
  };

  ENTER();

  // instruct MUI to generate the object
  if((obj = DoSuperNew(cl, obj,
                       MUIA_Group_Horiz,             TRUE,
                			 MUIA_TheBar_EnableKeys,       TRUE,
                  		 MUIA_TheBar_IgnoreAppareance, FALSE,
                       MUIA_TheBar_Buttons,          buttons,
                       MUIA_TheBar_PicsDrawer,       "PROGDIR:Icons",
                       MUIA_TheBar_Pics,             normalImages,
                       MUIA_TheBar_SelPics,          selectedImages,
                       MUIA_TheBar_DisPics,          ghostedImages,
                       TAG_MORE, inittags(msg))))
  {
    Object *buttonObj;

    // now we connect the toolbar buttons with their operations
    if((buttonObj = (Object *)DoMethod(obj, MUIM_TheBar_GetObject, TB_ABOOK_SAVE)))
  		DoMethod(buttonObj, MUIM_Notify, MUIA_Pressed, FALSE, MUIV_Notify_Application, 2, MUIM_CallHook, &AB_SaveABookHook);

    if((buttonObj = (Object *)DoMethod(obj, MUIM_TheBar_GetObject, TB_ABOOK_FIND)))
  		DoMethod(buttonObj, MUIM_Notify, MUIA_Pressed, FALSE, MUIV_Notify_Application, 2, MUIM_CallHook, &AB_FindHook);

    if((buttonObj = (Object *)DoMethod(obj, MUIM_TheBar_GetObject, TB_ABOOK_NEWUSER)))
  		DoMethod(buttonObj, MUIM_Notify, MUIA_Pressed, FALSE, MUIV_Notify_Application, 3, MUIM_CallHook, &AB_AddEntryHook, AET_USER);

    if((buttonObj = (Object *)DoMethod(obj, MUIM_TheBar_GetObject, TB_ABOOK_NEWLIST)))
  		DoMethod(buttonObj, MUIM_Notify, MUIA_Pressed, FALSE, MUIV_Notify_Application, 3, MUIM_CallHook, &AB_AddEntryHook, AET_LIST);

    if((buttonObj = (Object *)DoMethod(obj, MUIM_TheBar_GetObject, TB_ABOOK_NEWGROUP)))
  		DoMethod(buttonObj, MUIM_Notify, MUIA_Pressed, FALSE, MUIV_Notify_Application, 3, MUIM_CallHook, &AB_AddEntryHook, AET_GROUP);

    if((buttonObj = (Object *)DoMethod(obj, MUIM_TheBar_GetObject, TB_ABOOK_EDIT)))
  		DoMethod(buttonObj, MUIM_Notify, MUIA_Pressed, FALSE, MUIV_Notify_Application, 2, MUIM_CallHook, &AB_EditHook);

    if((buttonObj = (Object *)DoMethod(obj, MUIM_TheBar_GetObject, TB_ABOOK_DELETE)))
  		DoMethod(buttonObj, MUIM_Notify, MUIA_Pressed, FALSE, MUIV_Notify_Application, 2, MUIM_CallHook, &AB_DeleteHook);

    if((buttonObj = (Object *)DoMethod(obj, MUIM_TheBar_GetObject, TB_ABOOK_PRINT)))
  		DoMethod(buttonObj, MUIM_Notify, MUIA_Pressed, FALSE, MUIV_Notify_Application, 2, MUIM_CallHook, &AB_PrintHook);

    if((buttonObj = (Object *)DoMethod(obj, MUIM_TheBar_GetObject, TB_ABOOK_OPENTREE)))
  		DoMethod(buttonObj, MUIM_Notify, MUIA_Pressed, FALSE, MUIV_Notify_Application, 3, MUIM_CallHook, &AB_FoldUnfoldHook, FALSE);

    if((buttonObj = (Object *)DoMethod(obj, MUIM_TheBar_GetObject, TB_ABOOK_CLOSETREE)))
  		DoMethod(buttonObj, MUIM_Notify, MUIA_Pressed, FALSE, MUIV_Notify_Application, 3, MUIM_CallHook, &AB_FoldUnfoldHook, TRUE);
  }


  RETURN((ULONG)obj);
  return (ULONG)obj;
}

///

/* Public Methods */
