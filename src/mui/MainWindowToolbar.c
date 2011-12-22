/***************************************************************************

 YAM - Yet Another Mailer
 Copyright (C) 1995-2000 Marcel Beck
 Copyright (C) 2000-2011 YAM Open Source Team

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

#include <mui/NList_mcc.h>
#include <mui/TheBar_mcc.h>

#include "YAM.h"
#include "YAM_addressbook.h"
#include "YAM_config.h"
#include "YAM_find.h"
#include "YAM_mainFolder.h"

#include "Locale.h"
#include "Themes.h"

#include "mui/MainWindowToolbar.h"

#include "Debug.h"

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
  // prepare the buttons array which defines how our
  // toolbar looks like.
  struct MUIS_TheBar_Button buttons[TB_MAIN_NUM+5] =
  {
  #if !defined(__SASC)
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
  #else
    { TB_MAIN_READ,     TB_MAIN_READ,     NULL,                 NULL,                           0, 0, NULL, NULL },
    { TB_MAIN_EDIT,     TB_MAIN_EDIT,     NULL,                 NULL,                           0, 0, NULL, NULL },
    { TB_MAIN_MOVE,     TB_MAIN_MOVE,     NULL,                 NULL,                           0, 0, NULL, NULL },
    { TB_MAIN_DELETE,   TB_MAIN_DELETE,   NULL,                 NULL,                           0, 0, NULL, NULL },
    { TB_MAIN_GETADDR,  TB_MAIN_GETADDR,  NULL,                 NULL,                           0, 0, NULL, NULL },

    { MUIV_TheBar_BarSpacer, -1,  NULL, NULL, 0, 0, NULL, NULL },

    { TB_MAIN_NEWMAIL,  TB_MAIN_NEWMAIL,  NULL,                 NULL,                           0, 0, NULL, NULL },
    { TB_MAIN_REPLY,    TB_MAIN_REPLY,    NULL,                 NULL,                           0, 0, NULL, NULL },
    { TB_MAIN_FORWARD,  TB_MAIN_FORWARD,  NULL,                 NULL,                           0, 0, NULL, NULL },

    { MUIV_TheBar_BarSpacer, -1,  NULL, NULL, 0, 0, NULL, NULL },

    { TB_MAIN_GETMAIL,  TB_MAIN_GETMAIL,  NULL,                 NULL,                           0, 0, NULL, NULL },
    { TB_MAIN_SENDALL,  TB_MAIN_SENDALL,  NULL,                 NULL,                           0, 0, NULL, NULL },

    { MUIV_TheBar_BarSpacer, -1,  NULL, NULL, 0, 0, NULL, NULL },

    // the "Spam" button is disabled by default
    // the "not Spam" button is hidden by default
    { TB_MAIN_SPAM,     TB_MAIN_SPAM,     NULL,                 NULL,                           MUIV_TheBar_ButtonFlag_Disabled, 0, NULL, NULL },
    { TB_MAIN_HAM,      TB_MAIN_HAM,      NULL,                 NULL,                           MUIV_TheBar_ButtonFlag_Hide, 0, NULL, NULL },
    { TB_MAIN_FILTER,   TB_MAIN_FILTER,   NULL,                 NULL,                           0, 0, NULL, NULL },
    { TB_MAIN_FIND,     TB_MAIN_FIND,     NULL,                 NULL,                           0, 0, NULL, NULL },
    { TB_MAIN_ADDRBOOK, TB_MAIN_ADDRBOOK, NULL,                 NULL,                           0, 0, NULL, NULL },
    { TB_MAIN_CONFIG,   TB_MAIN_CONFIG,   NULL,                 NULL,                           0, 0, NULL, NULL },

    { MUIV_TheBar_End,       -1,  NULL, NULL, 0, 0, NULL, NULL },
  #endif
  };

  ENTER();

  #if defined(__SASC)
  buttons[ 0].text = tr(MSG_MA_TBRead);      buttons[ 0].help = tr(MSG_HELP_MA_BT_READ);
  buttons[ 1].text = tr(MSG_MA_TBEdit);      buttons[ 1].help = tr(MSG_HELP_MA_BT_EDIT);
  buttons[ 2].text = tr(MSG_MA_TBMove);      buttons[ 2].help = tr(MSG_HELP_MA_BT_MOVE);
  buttons[ 3].text = tr(MSG_MA_TBDelete);    buttons[ 3].help = tr(MSG_HELP_MA_BT_DELETE);
  buttons[ 4].text = tr(MSG_MA_TBGetAddr);   buttons[ 4].help = tr(MSG_HELP_MA_BT_GETADDRESS);

  buttons[ 6].text = tr(MSG_MA_TBWrite);     buttons[ 6].help = tr(MSG_HELP_MA_BT_WRITE);
  buttons[ 7].text = tr(MSG_MA_TBReply);     buttons[ 7].help = tr(MSG_HELP_MA_BT_REPLY);
  buttons[ 8].text = tr(MSG_MA_TBForward);   buttons[ 8].help = tr(MSG_HELP_MA_BT_FORWARD);

  buttons[10].text = tr(MSG_MA_TBGetMail);   buttons[10].help = tr(MSG_HELP_MA_BT_POPNOW);
  buttons[11].text = tr(MSG_MA_TBSendAll);   buttons[11].help = tr(MSG_HELP_MA_BT_SENDALL);

  buttons[13].text = tr(MSG_MA_TBSPAM);      buttons[13].help = tr(MSG_HELP_MA_BT_SPAM);
  buttons[14].text = tr(MSG_MA_TBNOTSPAM);   buttons[14].help = tr(MSG_HELP_MA_BT_NOTSPAM);
  buttons[15].text = tr(MSG_MA_TBFilter);    buttons[15].help = tr(MSG_HELP_MA_BT_FILTER);
  buttons[16].text = tr(MSG_MA_TBFind);      buttons[16].help = tr(MSG_HELP_MA_BT_SEARCH);
  buttons[17].text = tr(MSG_MA_TBAddrBook);  buttons[17].help = tr(MSG_HELP_MA_BT_ABOOK);
  buttons[18].text = tr(MSG_MA_TBConfig);    buttons[18].help = tr(MSG_HELP_MA_BT_CONFIG);
  #endif

  // instruct MUI to generate the object
  if((obj = DoSuperNew(cl, obj,
    MUIA_Group_Horiz,    TRUE,
    MUIA_TheBar_Buttons, buttons,
    MUIA_TheBar_Pics,    G->theme.mainWindowToolbarImages[TBIM_NORMAL],
    MUIA_TheBar_SelPics, G->theme.mainWindowToolbarImages[TBIM_SELECTED],
    MUIA_TheBar_DisPics, G->theme.mainWindowToolbarImages[TBIM_GHOSTED],
    TAG_MORE, inittags(msg))) != NULL)
  {
    // update the SPAM control buttons only if the spam filter is not enabled
    if(C->SpamFilterEnabled == FALSE)
      DoMethod(obj, MUIM_MainWindowToolbar_UpdateSpamControls);

    // connect the buttons presses
    DoMethod(obj, MUIM_TheBar_Notify, TB_MAIN_READ,    MUIA_Pressed, FALSE, MUIV_Notify_Application, 2, MUIM_CallHook, &MA_ReadMessageHook);
    DoMethod(obj, MUIM_TheBar_Notify, TB_MAIN_EDIT,    MUIA_Pressed, FALSE, MUIV_Notify_Application, 4, MUIM_CallHook, &MA_NewMessageHook, NMM_EDIT, MUIV_TheBar_Qualifier);
    DoMethod(obj, MUIM_TheBar_Notify, TB_MAIN_MOVE,    MUIA_Pressed, FALSE, MUIV_Notify_Application, 2, MUIM_CallHook, &MA_MoveMessageHook);
    DoMethod(obj, MUIM_TheBar_Notify, TB_MAIN_DELETE,  MUIA_Pressed, FALSE, MUIV_Notify_Application, 3, MUIM_CallHook, &MA_DeleteMessageHook, MUIV_TheBar_Qualifier);
    DoMethod(obj, MUIM_TheBar_Notify, TB_MAIN_GETADDR, MUIA_Pressed, FALSE, MUIV_Notify_Application, 2, MUIM_CallHook, &MA_GetAddressHook);
    DoMethod(obj, MUIM_TheBar_Notify, TB_MAIN_NEWMAIL, MUIA_Pressed, FALSE, MUIV_Notify_Application, 4, MUIM_CallHook, &MA_NewMessageHook, NMM_NEW, MUIV_TheBar_Qualifier);
    DoMethod(obj, MUIM_TheBar_Notify, TB_MAIN_REPLY,   MUIA_Pressed, FALSE, MUIV_Notify_Application, 4, MUIM_CallHook, &MA_NewMessageHook, NMM_REPLY, MUIV_TheBar_Qualifier);
    DoMethod(obj, MUIM_TheBar_Notify, TB_MAIN_FORWARD, MUIA_Pressed, FALSE, MUIV_Notify_Application, 4, MUIM_CallHook, &MA_NewMessageHook, NMM_FORWARD, MUIV_TheBar_Qualifier);
    DoMethod(obj, MUIM_TheBar_Notify, TB_MAIN_GETMAIL, MUIA_Pressed, FALSE, MUIV_Notify_Application, 8, MUIM_Application_PushMethod, G->App, 5, MUIM_CallHook, &MA_PopNowHook, -1, RECEIVEF_USER, MUIV_TheBar_Qualifier);
    DoMethod(obj, MUIM_TheBar_Notify, TB_MAIN_SENDALL, MUIA_Pressed, FALSE, MUIV_Notify_Application, 6, MUIM_Application_PushMethod, G->App, 3, MUIM_CallHook, &MA_SendHook, SENDMAIL_ALL_USER);
    DoMethod(obj, MUIM_TheBar_Notify, TB_MAIN_SPAM,    MUIA_Pressed, FALSE, MUIV_Notify_Application, 3, MUIM_CallHook, &MA_ClassifyMessageHook, BC_SPAM);
    DoMethod(obj, MUIM_TheBar_Notify, TB_MAIN_HAM,     MUIA_Pressed, FALSE, MUIV_Notify_Application, 3, MUIM_CallHook, &MA_ClassifyMessageHook, BC_HAM);
    DoMethod(obj, MUIM_TheBar_Notify, TB_MAIN_FILTER,  MUIA_Pressed, FALSE, MUIV_Notify_Application, 5, MUIM_CallHook, &ApplyFiltersHook, APPLY_USER, MUIV_TheBar_Qualifier, NULL);
    DoMethod(obj, MUIM_TheBar_Notify, TB_MAIN_FIND,    MUIA_Pressed, FALSE, MUIV_Notify_Application, 3, MUIM_CallHook, &FI_OpenHook, NULL);
    DoMethod(obj, MUIM_TheBar_Notify, TB_MAIN_ADDRBOOK,MUIA_Pressed, FALSE, MUIV_Notify_Application, 3, MUIM_CallHook, &AB_OpenHook, ABM_EDIT);
    DoMethod(obj, MUIM_TheBar_Notify, TB_MAIN_CONFIG,  MUIA_Pressed, FALSE, MUIV_Notify_Application, 2, MUIM_CallHook, &CO_OpenHook);
  }
  else
    E(DBF_STARTUP, "couldn't create MainWindowToolbar!");


  RETURN((IPTR)obj);
  return (IPTR)obj;
}

///

/* Public Methods */
/// DECLARE(UpdateSpamControls)
DECLARE(UpdateSpamControls)
{
  BOOL spamHidden, hamHidden;
  BOOL spamDisabled, hamDisabled;

  ENTER();

  // with an enabled spam filter we display just one button, either "Spam" or "no Spam"
  if(C->SpamFilterEnabled == TRUE)
  {
    Object *lv = G->MA->GUI.PG_MAILLIST;
    struct Mail *mail = NULL;
    ULONG numSelected = 0;

    // get the currently active mail entry.
    DoMethod(lv, MUIM_NList_GetEntry, MUIV_NList_GetEntry_Active, &mail);
    // ask the mail list how many entries as currently selected
    DoMethod(lv, MUIM_NList_Select, MUIV_NList_Select_All, MUIV_NList_Select_Ask, &numSelected);

    if(isGroupFolder(GetCurrentFolder()) || (mail == NULL && numSelected == 0))
    {
      // either this is a group folder or no message is selected
      // then just show the disabled "Spam" button
      spamHidden = FALSE;
      hamHidden = TRUE;
      spamDisabled = TRUE;
      hamDisabled = TRUE;
    }
    else if(mail != NULL || numSelected >= 1)
    {
      // at least one mail is selected in a regular folder
      // then show/enable the buttons depending on the mail state
      if(mail != NULL && hasStatusSpam(mail))
      {
        // definitively a spam mail, just show the "no Spam" button
        spamHidden = TRUE;
        hamHidden = FALSE;
        spamDisabled = TRUE;
        hamDisabled = FALSE;
      }
      else
      {
        // this mail is either definitively no spam, or it hasn't been classified yet
        // so just show the "Spam" button
        spamHidden = FALSE;
        hamHidden = TRUE;
        spamDisabled = FALSE;
        hamDisabled = TRUE;
      }
    }
    else
    {
      // any other case, just show the disabled "Spam" button
      // can this really happen??
      spamHidden = FALSE;
      hamHidden = TRUE;
      spamDisabled = TRUE;
      hamDisabled = TRUE;
    }
  }
  else
  {
    // the spam filter is not enabled, hide both buttons and the separator
    spamHidden = TRUE;
    hamHidden = TRUE;
    spamDisabled = TRUE;
    hamDisabled = TRUE;
  }

  DoMethod(obj, MUIM_TheBar_SetAttr, TB_MAIN_SPAM, MUIA_TheBar_Attr_Hide, spamHidden);
  DoMethod(obj, MUIM_TheBar_SetAttr, TB_MAIN_HAM,  MUIA_TheBar_Attr_Hide, hamHidden);
  DoMethod(obj, MUIM_TheBar_SetAttr, TB_MAIN_SPAM, MUIA_TheBar_Attr_Disabled, spamDisabled);
  DoMethod(obj, MUIM_TheBar_SetAttr, TB_MAIN_HAM,  MUIA_TheBar_Attr_Disabled, hamDisabled);

  RETURN(0);
  return 0;
}

///
