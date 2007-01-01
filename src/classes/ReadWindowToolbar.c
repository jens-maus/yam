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
 Description: Toolbar management class of the read window

***************************************************************************/

#include "ReadWindowToolbar_cl.h"

#include "Debug.h"

/* CLASSDATA
struct Data
{
  ULONG dummy;
};
*/

/* EXPORT
enum { TB_READ_PREV=0,
       TB_READ_NEXT,
       TB_READ_PREVTHREAD,
       TB_READ_NEXTTHREAD,
       TB_READ_DISPLAY,
       TB_READ_SAVE,
       TB_READ_PRINT,
       TB_READ_DELETE,
       TB_READ_MOVE,
       TB_READ_REPLY,
       TB_READ_FORWARD,
       TB_READ_SPAM,
       TB_READ_HAM,
       TB_READ_NUM
      };
*/

/* Private Data */

/* Private Functions */

/* Overloaded Methods */
/// OVERLOAD(OM_NEW)
OVERLOAD(OM_NEW)
{
  // define the image arrays
  static const char *normalImages[TB_READ_NUM+1] =
  {
    "Read_Prev",          // Prev
    "Read_Next",          // Next
    "Read_PrevThread",    // PrevThread
    "Read_NextThread",    // NextThread
    "Read_Display",       // Display
    "Read_Save",          // Save
    "Read_Print",         // Print
    "Read_Delete",        // Delete
    "Read_Move",          // Move
    "Read_Reply",         // Reply
    "Read_Forward",       // Forward
    "Read_Spam",          // Spam
    "Read_Ham",           // Ham
    NULL
  };

  static const char *selectedImages[TB_READ_NUM+1] =
  {
    "Read_Prev_S",        // Prev
    "Read_Next_S",        // Next
    "Read_PrevThread_S",  // PrevThread
    "Read_NextThread_S",  // NextThread
    "Read_Display_S",     // Display
    "Read_Save_S",        // Save
    "Read_Print_S",       // Print
    "Read_Delete_S",      // Delete
    "Read_Move_S",        // Move
    "Read_Reply_S",       // Reply
    "Read_Forward_S",     // Forward
    "Read_Spam_S",        // Spam
    "Read_Ham_S",         // Ham
    NULL
  };

  static const char *ghostedImages[TB_READ_NUM+1] =
  {
    "Read_Prev_G",        // Prev
    "Read_Next_G",        // Next
    "Read_PrevThread_G",  // PrevThread
    "Read_NextThread_G",  // NextThread
    "Read_Display_G",     // Display
    "Read_Save_G",        // Save
    "Read_Print_G",       // Print
    "Read_Delete_G",      // Delete
    "Read_Move_G",        // Move
    "Read_Reply_G",       // Reply
    "Read_Forward_G",     // Forward
    "Read_Spam_G",        // Spam
    "Read_Ham_G",         // Ham
    NULL
  };

  // prepare the buttons array which defines how our
  // toolbar looks like.
  struct MUIS_TheBar_Button buttons[TB_READ_NUM+4] =
  {
    { TB_READ_PREV,       TB_READ_PREV,       GetStr(MSG_RE_TBPrev),    GetStr(MSG_HELP_RE_BT_PREVIOUS),    0, 0, NULL, NULL },
    { TB_READ_NEXT,       TB_READ_NEXT,       GetStr(MSG_RE_TBNext),    GetStr(MSG_HELP_RE_BT_NEXT),        0, 0, NULL, NULL },
    { TB_READ_PREVTHREAD, TB_READ_PREVTHREAD, GetStr(MSG_RE_TBPrevTh),  GetStr(MSG_HELP_RE_BT_QUESTION),    0, 0, NULL, NULL },
    { TB_READ_NEXTTHREAD, TB_READ_NEXTTHREAD, GetStr(MSG_RE_TBNextTh),  GetStr(MSG_HELP_RE_BT_ANSWER),      0, 0, NULL, NULL },

    { MUIV_TheBar_BarSpacer, -1,  NULL, NULL, 0, 0, NULL, NULL },

    { TB_READ_DISPLAY,    TB_READ_DISPLAY,    GetStr(MSG_RE_TBDisplay), GetStr(MSG_HELP_RE_BT_DISPLAY),     0, 0, NULL, NULL },
    { TB_READ_SAVE,       TB_READ_SAVE,       GetStr(MSG_RE_TBSave),    GetStr(MSG_HELP_RE_BT_EXPORT),      0, 0, NULL, NULL },
    { TB_READ_PRINT,      TB_READ_PRINT,      GetStr(MSG_RE_TBPrint),   GetStr(MSG_HELP_RE_BT_PRINT),       0, 0, NULL, NULL },

    { MUIV_TheBar_BarSpacer, -1,  NULL, NULL, 0, 0, NULL, NULL },

    { TB_READ_DELETE,     TB_READ_DELETE,     GetStr(MSG_RE_TBDelete),  GetStr(MSG_HELP_RE_BT_DELETE),      0, 0, NULL, NULL },
    { TB_READ_MOVE,       TB_READ_MOVE,       GetStr(MSG_RE_TBMove),    GetStr(MSG_HELP_RE_BT_MOVE),        0, 0, NULL, NULL },
    { TB_READ_REPLY,      TB_READ_REPLY,      GetStr(MSG_RE_TBReply),   GetStr(MSG_HELP_RE_BT_REPLY),       0, 0, NULL, NULL },
    { TB_READ_FORWARD,    TB_READ_FORWARD,    GetStr(MSG_RE_TBForward), GetStr(MSG_HELP_RE_BT_FORWARD),     0, 0, NULL, NULL },

    { MUIV_TheBar_BarSpacer, TB_READ_NUM+1,  NULL, NULL, 0, 0, NULL, NULL },

    // the "Spam" button is disabled by default
    // the "not Spam" button is hidden by default
    { TB_READ_SPAM,       TB_READ_SPAM,       GetStr(MSG_RE_TBSPAM),    GetStr(MSG_HELP_RE_BT_SPAM),        MUIV_TheBar_ButtonFlag_Disabled, 0, NULL, NULL },
    { TB_READ_HAM,        TB_READ_HAM,        GetStr(MSG_RE_TBNOTSPAM), GetStr(MSG_HELP_RE_BT_NOTSPAM),     MUIV_TheBar_ButtonFlag_Hide, 0, NULL, NULL },
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
    // everything worked out fine.

    // update the SPAM controls
    DoMethod(obj, MUIM_ReadWindowToolbar_UpdateSpamControls, NULL);
  }
  else
    E(DBF_STARTUP, "couldn't create ReadWindowToolbar!");


  RETURN((ULONG)obj);
  return (ULONG)obj;
}

///

/* Public Methods */
/// DECLARE(InitNotify)
// Method for connecting all notifies which deal with our
// toolbar
DECLARE(InitNotify) // Object *readWindow, Object *readMailGroup
{
  Object *buttonObj;
  Object *readWindow = msg->readWindow;
  Object *readMailGroup = msg->readMailGroup;

  ENTER();

  #warning "Toolbar_Qualifier missing!";

  // connect the buttons presses
  if((buttonObj = (Object *)DoMethod(obj, MUIM_TheBar_GetObject, TB_READ_PREV)))
    DoMethod(buttonObj, MUIM_Notify, MUIA_Pressed, FALSE, readWindow, 3, MUIM_ReadWindow_SwitchMail, -1, 0); //MUIV_Toolbar_Qualifier);

  if((buttonObj = (Object *)DoMethod(obj, MUIM_TheBar_GetObject, TB_READ_NEXT)))
    DoMethod(buttonObj, MUIM_Notify, MUIA_Pressed, FALSE, readWindow, 3, MUIM_ReadWindow_SwitchMail, +1, 0); //MUIV_Toolbar_Qualifier);

  if((buttonObj = (Object *)DoMethod(obj, MUIM_TheBar_GetObject, TB_READ_PREVTHREAD)))
    DoMethod(buttonObj, MUIM_Notify, MUIA_Pressed, FALSE, readWindow, 2, MUIM_ReadWindow_FollowThread, -1);

  if((buttonObj = (Object *)DoMethod(obj, MUIM_TheBar_GetObject, TB_READ_NEXTTHREAD)))
    DoMethod(buttonObj, MUIM_Notify, MUIA_Pressed, FALSE, readWindow, 2, MUIM_ReadWindow_FollowThread, +1);

  if((buttonObj = (Object *)DoMethod(obj, MUIM_TheBar_GetObject, TB_READ_DISPLAY)))
    DoMethod(buttonObj, MUIM_Notify, MUIA_Pressed, FALSE, readMailGroup, 1, MUIM_ReadMailGroup_DisplayMailRequest);

  if((buttonObj = (Object *)DoMethod(obj, MUIM_TheBar_GetObject, TB_READ_SAVE)))
    DoMethod(buttonObj, MUIM_Notify, MUIA_Pressed, FALSE, readMailGroup, 1, MUIM_ReadMailGroup_SaveMailRequest);

  if((buttonObj = (Object *)DoMethod(obj, MUIM_TheBar_GetObject, TB_READ_PRINT)))
    DoMethod(buttonObj, MUIM_Notify, MUIA_Pressed, FALSE, readMailGroup, 1, MUIM_ReadMailGroup_PrintMailRequest);

  if((buttonObj = (Object *)DoMethod(obj, MUIM_TheBar_GetObject, TB_READ_DELETE)))
    DoMethod(buttonObj, MUIM_Notify, MUIA_Pressed, FALSE, readWindow, 2, MUIM_ReadWindow_DeleteMailRequest, 0); //MUIV_Toolbar_Qualifier);

  if((buttonObj = (Object *)DoMethod(obj, MUIM_TheBar_GetObject, TB_READ_MOVE)))
    DoMethod(buttonObj, MUIM_Notify, MUIA_Pressed, FALSE, readWindow, 1, MUIM_ReadWindow_MoveMailRequest);

  if((buttonObj = (Object *)DoMethod(obj, MUIM_TheBar_GetObject, TB_READ_REPLY)))
    DoMethod(buttonObj, MUIM_Notify, MUIA_Pressed, FALSE, readWindow, 3, MUIM_ReadWindow_NewMail, NEW_REPLY, 0); //MUIV_Toolbar_Qualifier);

  if((buttonObj = (Object *)DoMethod(obj, MUIM_TheBar_GetObject, TB_READ_FORWARD)))
    DoMethod(buttonObj, MUIM_Notify, MUIA_Pressed, FALSE, readWindow, 3, MUIM_ReadWindow_NewMail, NEW_FORWARD, 0); //MUIV_Toolbar_Qualifier);

  if((buttonObj = (Object *)DoMethod(obj, MUIM_TheBar_GetObject, TB_READ_SPAM)))
    DoMethod(buttonObj, MUIM_Notify, MUIA_Pressed, FALSE, readWindow, 2, MUIM_ReadWindow_ClassifyMessage, BC_SPAM);

  if((buttonObj = (Object *)DoMethod(obj, MUIM_TheBar_GetObject, TB_READ_HAM)))
    DoMethod(buttonObj, MUIM_Notify, MUIA_Pressed, FALSE, readWindow, 2, MUIM_ReadWindow_ClassifyMessage, BC_HAM);

  RETURN(0);
  return 0;
}
///
/// DECLARE(UpdateSpamControls)
// update the "Spam" and "not Spam" buttons
DECLARE(UpdateSpamControls) // struct Mail *mail
{
  BOOL hideSpam;
  BOOL hideHam;
  BOOL disableSpam;

  ENTER();

  if(C->SpamFilterEnabled)
  {
    // the spam filter is enabled, now check the mail state
    if(msg->mail == NULL || isVirtualMail(msg->mail))
    {
      // this is no real mail, so just show an disabled "Spam" button
      hideSpam = FALSE;
      hideHam = FALSE;
      disableSpam = TRUE;
    }
    else if(hasStatusSpam(msg->mail))
    {
      // the mail is spam, so just show the "not Spam" button
      hideSpam = TRUE;
      hideHam = FALSE;
      disableSpam = FALSE;
    }
    else
    {
      // the mail is either no spam or yet classified, so just show the "Spam" button
      hideSpam = FALSE;
      hideHam = TRUE;
      disableSpam = FALSE;
    }
  }
  else
  {
    // the spam filter is disabled, so hide both buttons and the separator
    hideSpam = TRUE;
    hideHam = TRUE;
    disableSpam = TRUE;
  }

  // now set the attributes
  DoMethod(obj, MUIM_TheBar_SetAttr, TB_READ_NUM+1, MUIV_TheBar_Attr_Hide,     !C->SpamFilterEnabled);
  DoMethod(obj, MUIM_TheBar_SetAttr, TB_READ_SPAM,  MUIV_TheBar_Attr_Hide,     hideSpam);
  DoMethod(obj, MUIM_TheBar_SetAttr, TB_READ_SPAM,  MUIV_TheBar_Attr_Disabled, disableSpam);
  DoMethod(obj, MUIM_TheBar_SetAttr, TB_READ_HAM,   MUIV_TheBar_Attr_Hide,     hideHam);

  RETURN(0);
  return 0;
}
///
