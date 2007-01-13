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
 Description: Toolbar management class of the main window

***************************************************************************/

#include "MainWindowToolbar_cl.h"

#include "Debug.h"

/* CLASSDATA
struct Data
{
  ULONG dummy;
};
*/

/* EXPORT
enum { TB_MAIN_READ=0,
       TB_MAIN_EDIT,
       TB_MAIN_MOVE,
       TB_MAIN_DELETE,
       TB_MAIN_GETADDR,
       TB_MAIN_NEWMAIL,
       TB_MAIN_REPLY,
       TB_MAIN_FORWARD,
       TB_MAIN_GETMAIL,
       TB_MAIN_SENDALL,
       TB_MAIN_SPAM,
       TB_MAIN_HAM,
       TB_MAIN_FILTER,
       TB_MAIN_FIND,
       TB_MAIN_ADDRBOOK,
       TB_MAIN_CONFIG,
       TB_MAIN_NUM
      };
*/

/* Private Data */

/* Private Functions */

/* Overloaded Methods */
/// OVERLOAD(OM_NEW)
OVERLOAD(OM_NEW)
{
  // define the image arrays
  static const char *normalImages[TB_MAIN_NUM+1] =
  {
    "Main_Read",          // Read
    "Main_Edit",          // Edit
    "Main_Move",          // Move
    "Main_Delete",        // Delete
    "Main_GetAddr",       // GetAddr
    "Main_NewMail",       // NewMail
    "Main_Reply",         // Reply
    "Main_Forward",       // Forward
    "Main_GetMail",       // GetMail
    "Main_SendAll",       // SendAll
    "Main_Spam",          // Spam
    "Main_Ham",           // Ham
    "Main_Filter",        // Filter
    "Main_Find",          // Find
    "Main_AddrBook",      // AddrBook
    "Main_Config",        // Config
    NULL
  };

  static const char *selectedImages[TB_MAIN_NUM+1] =
  {
    "Main_Read_S",        // Read
    "Main_Edit_S",        // Edit
    "Main_Move_S",        // Move
    "Main_Delete_S",      // Delete
    "Main_GetAddr_S",     // GetAddr
    "Main_NewMail_S",     // NewMail
    "Main_Reply_S",       // Reply
    "Main_Forward_S",     // Forward
    "Main_GetMail_S",     // GetMail
    "Main_SendAll_S",     // SendAll
    "Main_Spam_S",        // Spam
    "Main_Ham_S",         // Ham
    "Main_Filter_S",      // Filter
    "Main_Find_S",        // Find
    "Main_AddrBook_S",    // AddrBook
    "Main_Config_S",      // Config
    NULL
  };

  static const char *ghostedImages[TB_MAIN_NUM+1] =
  {
    "Main_Read_G",        // Read
    "Main_Edit_G",        // Edit
    "Main_Move_G",        // Move
    "Main_Delete_G",      // Delete
    "Main_GetAddr_G",     // GetAddr
    "Main_NewMail_G",     // NewMail
    "Main_Reply_G",       // Reply
    "Main_Forward_G",     // Forward
    "Main_GetMail_G",     // GetMail
    "Main_SendAll_G",     // SendAll
    "Main_Spam_G",        // Spam
    "Main_Ham_G",         // Ham
    "Main_Filter_G",      // Filter
    "Main_Find_G",        // Find
    "Main_AddrBook_G",    // AddrBook
    "Main_Config_G",      // Config
    NULL
  };

  // prepare the buttons array which defines how our
  // toolbar looks like.
  struct MUIS_TheBar_Button buttons[TB_MAIN_NUM+5] =
  {
    { TB_MAIN_READ,     TB_MAIN_READ,     tr(MSG_MA_TBRead),    tr(MSG_HELP_MA_BT_READ),        0, 0, NULL, NULL },
    { TB_MAIN_EDIT,     TB_MAIN_EDIT,     tr(MSG_MA_TBEdit),    tr(MSG_HELP_MA_BT_EDIT),        0, 0, NULL, NULL },
    { TB_MAIN_MOVE,     TB_MAIN_MOVE,     tr(MSG_MA_TBMove),    tr(MSG_HELP_MA_BT_MOVE),        0, 0, NULL, NULL },
    { TB_MAIN_DELETE,   TB_MAIN_DELETE,   tr(MSG_MA_TBDelete),  tr(MSG_HELP_MA_BT_DELETE),      0, 0, NULL, NULL },
    { TB_MAIN_GETADDR,  TB_MAIN_GETADDR,  tr(MSG_MA_TBGetAddr), tr(MSG_HELP_MA_BT_GETADDRESS),  0, 0, NULL, NULL },

    { MUIV_TheBar_BarSpacer, -1,  NULL, NULL, 0, 0, NULL, NULL },

    { TB_MAIN_NEWMAIL,  TB_MAIN_NEWMAIL,  tr(MSG_MA_TBWrite),   tr(MSG_HELP_MA_BT_WRITE),       0, 0, NULL, NULL },
    { TB_MAIN_REPLY,    TB_MAIN_REPLY,    tr(MSG_MA_TBReply),   tr(MSG_HELP_MA_BT_REPLY),       0, 0, NULL, NULL },
    { TB_MAIN_FORWARD,  TB_MAIN_FORWARD,  tr(MSG_MA_TBForward), tr(MSG_HELP_MA_BT_FORWARD),     0, 0, NULL, NULL },

    { MUIV_TheBar_BarSpacer, -1,  NULL, NULL, 0, 0, NULL, NULL },

    { TB_MAIN_GETMAIL,  TB_MAIN_GETMAIL,  tr(MSG_MA_TBGetMail), tr(MSG_HELP_MA_BT_POPNOW),      0, 0, NULL, NULL },
    { TB_MAIN_SENDALL,  TB_MAIN_SENDALL,  tr(MSG_MA_TBSendAll), tr(MSG_HELP_MA_BT_SENDALL),     0, 0, NULL, NULL },

    { MUIV_TheBar_BarSpacer, -1,  NULL, NULL, 0, 0, NULL, NULL },

    // the "Spam" button is disabled by default
    // the "not Spam" button is hidden by default
    { TB_MAIN_SPAM,     TB_MAIN_SPAM,     tr(MSG_MA_TBSPAM),    tr(MSG_HELP_MA_BT_SPAM),        MUIV_TheBar_ButtonFlag_Disabled, 0, NULL, NULL },
    { TB_MAIN_HAM,      TB_MAIN_HAM,      tr(MSG_MA_TBNOTSPAM), tr(MSG_HELP_MA_BT_NOTSPAM),     MUIV_TheBar_ButtonFlag_Hide, 0, NULL, NULL },
    { TB_MAIN_FILTER,   TB_MAIN_FILTER,   tr(MSG_MA_TBFilter),  tr(MSG_HELP_MA_BT_FILTER),      0, 0, NULL, NULL },
    { TB_MAIN_FIND,     TB_MAIN_FIND,     tr(MSG_MA_TBFind),    tr(MSG_HELP_MA_BT_SEARCH),      0, 0, NULL, NULL },
    { TB_MAIN_ADDRBOOK, TB_MAIN_ADDRBOOK, tr(MSG_MA_TBAddrBook),tr(MSG_HELP_MA_BT_ABOOK),       0, 0, NULL, NULL },
    { TB_MAIN_CONFIG,   TB_MAIN_CONFIG,   tr(MSG_MA_TBConfig),  tr(MSG_HELP_MA_BT_CONFIG),      0, 0, NULL, NULL },

    { MUIV_TheBar_End,       -1,  NULL, NULL, 0, 0, NULL, NULL },
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

    // update the SPAM control buttons only if the spam filter is not enabled
    if(!C->SpamFilterEnabled)
      DoMethod(obj, MUIM_MainWindowToolbar_UpdateSpamControls);

    // everything worked out fine.
    #warning "Toolbar_Qualifier missing!";

    // connect the buttons presses
    if((buttonObj = (Object *)DoMethod(obj, MUIM_TheBar_GetObject, TB_MAIN_READ)))
      DoMethod(buttonObj, MUIM_Notify, MUIA_Pressed, FALSE, MUIV_Notify_Application, 2, MUIM_CallHook, &MA_ReadMessageHook);

    if((buttonObj = (Object *)DoMethod(obj, MUIM_TheBar_GetObject, TB_MAIN_EDIT)))
      DoMethod(buttonObj, MUIM_Notify, MUIA_Pressed, FALSE, MUIV_Notify_Application, 4, MUIM_CallHook, &MA_NewMessageHook, NEW_EDIT, 0); //MUIV_Toolbar_Qualifier);

    if((buttonObj = (Object *)DoMethod(obj, MUIM_TheBar_GetObject, TB_MAIN_MOVE)))
      DoMethod(buttonObj, MUIM_Notify, MUIA_Pressed, FALSE, MUIV_Notify_Application, 2, MUIM_CallHook, &MA_MoveMessageHook);

    if((buttonObj = (Object *)DoMethod(obj, MUIM_TheBar_GetObject, TB_MAIN_DELETE)))
      DoMethod(buttonObj, MUIM_Notify, MUIA_Pressed, FALSE, MUIV_Notify_Application, 3, MUIM_CallHook, &MA_DeleteMessageHook, 0); //MUIV_Toolbar_Qualifier);

    if((buttonObj = (Object *)DoMethod(obj, MUIM_TheBar_GetObject, TB_MAIN_GETADDR)))
      DoMethod(buttonObj, MUIM_Notify, MUIA_Pressed, FALSE, MUIV_Notify_Application, 2, MUIM_CallHook, &MA_GetAddressHook);

    if((buttonObj = (Object *)DoMethod(obj, MUIM_TheBar_GetObject, TB_MAIN_NEWMAIL)))
      DoMethod(buttonObj, MUIM_Notify, MUIA_Pressed, FALSE, MUIV_Notify_Application, 4, MUIM_CallHook, &MA_NewMessageHook, NEW_NEW, 0); //MUIV_Toolbar_Qualifier);

    if((buttonObj = (Object *)DoMethod(obj, MUIM_TheBar_GetObject, TB_MAIN_REPLY)))
      DoMethod(buttonObj, MUIM_Notify, MUIA_Pressed, FALSE, MUIV_Notify_Application, 4, MUIM_CallHook, &MA_NewMessageHook, NEW_REPLY, 0); //MUIV_Toolbar_Qualifier);

    if((buttonObj = (Object *)DoMethod(obj, MUIM_TheBar_GetObject, TB_MAIN_FORWARD)))
      DoMethod(buttonObj, MUIM_Notify, MUIA_Pressed, FALSE, MUIV_Notify_Application, 4, MUIM_CallHook, &MA_NewMessageHook, NEW_FORWARD, 0); //MUIV_Toolbar_Qualifier);

    if((buttonObj = (Object *)DoMethod(obj, MUIM_TheBar_GetObject, TB_MAIN_GETMAIL)))
      DoMethod(buttonObj, MUIM_Notify, MUIA_Pressed, FALSE, MUIV_Notify_Application, 8, MUIM_Application_PushMethod, G->App, 5, MUIM_CallHook, &MA_PopNowHook, POP_USER, -1, 0); //MUIV_Toolbar_Qualifier);

    if((buttonObj = (Object *)DoMethod(obj, MUIM_TheBar_GetObject, TB_MAIN_SENDALL)))
      DoMethod(buttonObj, MUIM_Notify, MUIA_Pressed, FALSE, MUIV_Notify_Application, 6, MUIM_Application_PushMethod, G->App, 3, MUIM_CallHook, &MA_SendHook, SEND_ALL);

    if((buttonObj = (Object *)DoMethod(obj, MUIM_TheBar_GetObject, TB_MAIN_SPAM)))
      DoMethod(buttonObj, MUIM_Notify, MUIA_Pressed, FALSE, MUIV_Notify_Application, 3, MUIM_CallHook, &MA_ClassifyMessageHook, BC_SPAM);

    if((buttonObj = (Object *)DoMethod(obj, MUIM_TheBar_GetObject, TB_MAIN_HAM)))
      DoMethod(buttonObj, MUIM_Notify, MUIA_Pressed, FALSE, MUIV_Notify_Application, 3, MUIM_CallHook, &MA_ClassifyMessageHook, BC_HAM);

    if((buttonObj = (Object *)DoMethod(obj, MUIM_TheBar_GetObject, TB_MAIN_FILTER)))
      DoMethod(buttonObj, MUIM_Notify, MUIA_Pressed, FALSE, MUIV_Notify_Application, 4, MUIM_CallHook, &ApplyFiltersHook, APPLY_USER, 0); //MUIV_Toolbar_Qualifier);

    if((buttonObj = (Object *)DoMethod(obj, MUIM_TheBar_GetObject, TB_MAIN_FIND)))
      DoMethod(buttonObj, MUIM_Notify, MUIA_Pressed, FALSE, MUIV_Notify_Application, 2, MUIM_CallHook, &FI_OpenHook);

    if((buttonObj = (Object *)DoMethod(obj, MUIM_TheBar_GetObject, TB_MAIN_ADDRBOOK)))
      DoMethod(buttonObj, MUIM_Notify, MUIA_Pressed, FALSE, MUIV_Notify_Application, 3, MUIM_CallHook, &AB_OpenHook, ABM_EDIT);

    if((buttonObj = (Object *)DoMethod(obj, MUIM_TheBar_GetObject, TB_MAIN_CONFIG)))
      DoMethod(buttonObj, MUIM_Notify, MUIA_Pressed, FALSE, MUIV_Notify_Application, 2, MUIM_CallHook, &CO_OpenHook);
  }
  else
    E(DBF_STARTUP, "couldn't create MainWindowToolbar!");


  RETURN((ULONG)obj);
  return (ULONG)obj;
}

///

/* Public Methods */
/// DECLARE(UpdateSpamControls)
DECLARE(UpdateSpamControls)
{
  BOOL hide = !C->SpamFilterEnabled;

  ENTER();

  DoMethod(obj, MUIM_TheBar_SetAttr, TB_MAIN_SPAM,  MUIV_TheBar_Attr_Hide, hide);
  DoMethod(obj, MUIM_TheBar_SetAttr, TB_MAIN_HAM,   MUIV_TheBar_Attr_Hide, hide);

  RETURN(0);
  return 0;
}
///
