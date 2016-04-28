/***************************************************************************

 YAM - Yet Another Mailer
 Copyright (C) 1995-2000 Marcel Beck
 Copyright (C) 2000-2016 YAM Open Source Team

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

#include <mui/TheBar_mcc.h>

#include "YAM.h"
#include "YAM_mainFolder.h"

#include "mui/ReadMailGroup.h"
#include "mui/ReadWindow.h"

#include "Config.h"
#include "ImageCache.h"
#include "Locale.h"
#include "Themes.h"

#include "Debug.h"

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
  ENTER();

  // depending on whether the read window toolbar
  // exists cached already we go and obtain the images
  // from the cached object instead.
  if(IsToolbarInCache(TBT_ReadWindow))
  {
    // prepare the buttons array which defines how our
    // toolbar looks like.
    struct MUIS_TheBar_Button buttons[TB_READ_NUM+4] =
    {
    #if !defined(__SASC)
      { TB_READ_PREV,       TB_READ_PREV,       tr(MSG_RE_TBPrev),    tr(MSG_HELP_RE_BT_PREVIOUS),    0, 0, NULL, NULL },
      { TB_READ_NEXT,       TB_READ_NEXT,       tr(MSG_RE_TBNext),    tr(MSG_HELP_RE_BT_NEXT),        0, 0, NULL, NULL },
      { TB_READ_PREVTHREAD, TB_READ_PREVTHREAD, tr(MSG_RE_TBPrevTh),  tr(MSG_HELP_RE_BT_QUESTION),    0, 0, NULL, NULL },
      { TB_READ_NEXTTHREAD, TB_READ_NEXTTHREAD, tr(MSG_RE_TBNextTh),  tr(MSG_HELP_RE_BT_ANSWER),      0, 0, NULL, NULL },

      { MUIV_TheBar_ButtonSpacer, -1,  NULL, NULL, 0, 0, NULL, NULL },

      { TB_READ_DISPLAY,    TB_READ_DISPLAY,    tr(MSG_RE_TBDisplay), tr(MSG_HELP_RE_BT_DISPLAY),     0, 0, NULL, NULL },
      { TB_READ_SAVE,       TB_READ_SAVE,       tr(MSG_RE_TBSave),    tr(MSG_HELP_RE_BT_EXPORT),      0, 0, NULL, NULL },
      { TB_READ_PRINT,      TB_READ_PRINT,      tr(MSG_RE_TBPrint),   tr(MSG_HELP_RE_BT_PRINT),       0, 0, NULL, NULL },

      { MUIV_TheBar_ButtonSpacer, -1,  NULL, NULL, 0, 0, NULL, NULL },

      { TB_READ_DELETE,     TB_READ_DELETE,     tr(MSG_RE_TBDelete),  tr(MSG_HELP_RE_BT_DELETE),      0, 0, NULL, NULL },
      { TB_READ_MOVE,       TB_READ_MOVE,       tr(MSG_RE_TBMove),    tr(MSG_HELP_RE_BT_MOVE),        0, 0, NULL, NULL },
      { TB_READ_REPLY,      TB_READ_REPLY,      tr(MSG_RE_TBReply),   tr(MSG_HELP_RE_BT_REPLY),       0, 0, NULL, NULL },
      { TB_READ_FORWARD,    TB_READ_FORWARD,    tr(MSG_RE_TBForward), tr(MSG_HELP_RE_BT_FORWARD),     0, 0, NULL, NULL },

      { MUIV_TheBar_ButtonSpacer, TB_READ_NUM+1,  NULL, NULL, 0, 0, NULL, NULL },

      // the "Spam" button is disabled by default
      // the "not Spam" button is hidden by default
      { TB_READ_SPAM,       TB_READ_SPAM,       tr(MSG_RE_TBSPAM),    tr(MSG_HELP_RE_BT_SPAM),        MUIV_TheBar_ButtonFlag_Disabled, 0, NULL, NULL },
      { TB_READ_HAM,        TB_READ_HAM,        tr(MSG_RE_TBNOTSPAM), tr(MSG_HELP_RE_BT_NOTSPAM),     MUIV_TheBar_ButtonFlag_Hide, 0, NULL, NULL },

      { MUIV_TheBar_End,       -1,  NULL, NULL, 0, 0, NULL, NULL },
    #else
      { TB_READ_PREV,       TB_READ_PREV,       NULL,                 NULL,                           0, 0, NULL, NULL },
      { TB_READ_NEXT,       TB_READ_NEXT,       NULL,                 NULL,                           0, 0, NULL, NULL },
      { TB_READ_PREVTHREAD, TB_READ_PREVTHREAD, NULL,                 NULL,                           0, 0, NULL, NULL },
      { TB_READ_NEXTTHREAD, TB_READ_NEXTTHREAD, NULL,                 NULL,                           0, 0, NULL, NULL },

      { MUIV_TheBar_ButtonSpacer, -1,  NULL, NULL, 0, 0, NULL, NULL },

      { TB_READ_DISPLAY,    TB_READ_DISPLAY,    NULL,                 NULL,                           0, 0, NULL, NULL },
      { TB_READ_SAVE,       TB_READ_SAVE,       NULL,                 NULL,                           0, 0, NULL, NULL },
      { TB_READ_PRINT,      TB_READ_PRINT,      NULL,                 NULL,                           0, 0, NULL, NULL },

      { MUIV_TheBar_ButtonSpacer, -1,  NULL, NULL, 0, 0, NULL, NULL },

      { TB_READ_DELETE,     TB_READ_DELETE,     NULL,                 NULL,                           0, 0, NULL, NULL },
      { TB_READ_MOVE,       TB_READ_MOVE,       NULL,                 NULL,                           0, 0, NULL, NULL },
      { TB_READ_REPLY,      TB_READ_REPLY,      NULL,                 NULL,                           0, 0, NULL, NULL },
      { TB_READ_FORWARD,    TB_READ_FORWARD,    NULL,                 NULL,                           0, 0, NULL, NULL },

      { MUIV_TheBar_ButtonSpacer, TB_READ_NUM+1,  NULL, NULL, 0, 0, NULL, NULL },

      // the "Spam" button is disabled by default
      // the "not Spam" button is hidden by default
      { TB_READ_SPAM,       TB_READ_SPAM,       NULL,                 NULL,                           0, 0, NULL, NULL },
      { TB_READ_HAM,        TB_READ_HAM,        NULL,                 NULL,                           0, 0, NULL, NULL },

      { MUIV_TheBar_End,       -1,  NULL, NULL, 0, 0, NULL, NULL },
    #endif
    };

    #if defined(__SASC)
    buttons[ 0].text = tr(MSG_RE_TBPrev);    buttons[ 0].help = tr(MSG_HELP_RE_BT_PREVIOUS),
    buttons[ 1].text = tr(MSG_RE_TBNext);    buttons[ 1].help = tr(MSG_HELP_RE_BT_NEXT),
    buttons[ 2].text = tr(MSG_RE_TBPrevTh);  buttons[ 2].help = tr(MSG_HELP_RE_BT_QUESTION),
    buttons[ 3].text = tr(MSG_RE_TBNextTh);  buttons[ 3].help = tr(MSG_HELP_RE_BT_ANSWER),

    buttons[ 5].text = tr(MSG_RE_TBDisplay); buttons[ 5].help = tr(MSG_HELP_RE_BT_DISPLAY),
    buttons[ 6].text = tr(MSG_RE_TBSave);    buttons[ 6].help = tr(MSG_HELP_RE_BT_EXPORT),
    buttons[ 7].text = tr(MSG_RE_TBPrint);   buttons[ 7].help = tr(MSG_HELP_RE_BT_PRINT),

    buttons[ 9].text = tr(MSG_RE_TBDelete);  buttons[ 9].help = tr(MSG_HELP_RE_BT_DELETE),
    buttons[10].text = tr(MSG_RE_TBMove);    buttons[10].help = tr(MSG_HELP_RE_BT_MOVE),
    buttons[11].text = tr(MSG_RE_TBReply);   buttons[11].help = tr(MSG_HELP_RE_BT_REPLY),
    buttons[12].text = tr(MSG_RE_TBForward); buttons[12].help = tr(MSG_HELP_RE_BT_FORWARD),

    buttons[14].text = tr(MSG_RE_TBSPAM);    buttons[14].help = tr(MSG_HELP_RE_BT_SPAM),
    buttons[15].text = tr(MSG_RE_TBNOTSPAM); buttons[15].help = tr(MSG_HELP_RE_BT_NOTSPAM),
    #endif

    // create TheBar object with the cached
    // toolbar images
    obj = DoSuperNew(cl, obj,
                     MUIA_Group_Horiz,      TRUE,
                     MUIA_TheBar_Buttons,   buttons,
                     MUIA_TheBar_Images,    ObtainToolbarImages(TBT_ReadWindow, TBI_Normal),
                     MUIA_TheBar_DisImages, ObtainToolbarImages(TBT_ReadWindow, TBI_Ghosted),
                     MUIA_TheBar_SelImages, ObtainToolbarImages(TBT_ReadWindow, TBI_Selected),
                     TAG_MORE, inittags(msg));
  }
  else
  {
    // create the TheBar object, but via loading the images from
    // the corresponding image files.
    obj = DoSuperNew(cl, obj,
                     MUIA_TheBar_Pics,      G->theme.readWindowToolbarImages[TBIM_NORMAL],
                     MUIA_TheBar_SelPics,   G->theme.readWindowToolbarImages[TBIM_SELECTED],
                     MUIA_TheBar_DisPics,   G->theme.readWindowToolbarImages[TBIM_GHOSTED],
                     TAG_MORE, inittags(msg));
  }

  // check if the object was created correctly.
  if(obj != NULL)
  {
    // everything worked out fine.

    // update the SPAM controls
    DoMethod(obj, MUIM_ReadWindowToolbar_UpdateSpamControls, NULL);
  }
  else
    E(DBF_STARTUP, "couldn't create ReadWindowToolbar!");


  RETURN((IPTR)obj);
  return (IPTR)obj;
}

///

/* Public Methods */
/// DECLARE(InitNotify)
// Method for connecting all notifies which deal with our
// toolbar
DECLARE(InitNotify) // Object *readWindow, Object *readMailGroup
{
  Object *readWindow = msg->readWindow;
  Object *readMailGroup = msg->readMailGroup;

  ENTER();

  // connect the buttons presses
  DoMethod(obj, MUIM_TheBar_Notify, TB_READ_PREV,       MUIA_Pressed, FALSE, readWindow, 3, MUIM_ReadWindow_SwitchMail, -1, MUIV_TheBar_Qualifier);
  DoMethod(obj, MUIM_TheBar_Notify, TB_READ_NEXT,       MUIA_Pressed, FALSE, readWindow, 3, MUIM_ReadWindow_SwitchMail, +1, MUIV_TheBar_Qualifier);
  DoMethod(obj, MUIM_TheBar_Notify, TB_READ_PREVTHREAD, MUIA_Pressed, FALSE, readWindow, 2, MUIM_ReadWindow_FollowThread, -1);
  DoMethod(obj, MUIM_TheBar_Notify, TB_READ_NEXTTHREAD, MUIA_Pressed, FALSE, readWindow, 2, MUIM_ReadWindow_FollowThread, +1);
  DoMethod(obj, MUIM_TheBar_Notify, TB_READ_DISPLAY,    MUIA_Pressed, FALSE, readMailGroup, 1, MUIM_ReadMailGroup_DisplayMailRequest);
  DoMethod(obj, MUIM_TheBar_Notify, TB_READ_SAVE,       MUIA_Pressed, FALSE, readMailGroup, 1, MUIM_ReadMailGroup_SaveMailRequest);
  DoMethod(obj, MUIM_TheBar_Notify, TB_READ_PRINT,      MUIA_Pressed, FALSE, readMailGroup, 1, MUIM_ReadMailGroup_PrintMailRequest);
  DoMethod(obj, MUIM_TheBar_Notify, TB_READ_DELETE,     MUIA_Pressed, FALSE, readWindow, 2, MUIM_ReadWindow_DeleteMailRequest, MUIV_TheBar_Qualifier);
  DoMethod(obj, MUIM_TheBar_Notify, TB_READ_MOVE,       MUIA_Pressed, FALSE, readWindow, 1, MUIM_ReadWindow_MoveMailRequest);
  DoMethod(obj, MUIM_TheBar_Notify, TB_READ_REPLY,      MUIA_Pressed, FALSE, readWindow, 3, MUIM_ReadWindow_NewMail, NMM_REPLY, MUIV_TheBar_Qualifier);
  DoMethod(obj, MUIM_TheBar_Notify, TB_READ_FORWARD,    MUIA_Pressed, FALSE, readWindow, 3, MUIM_ReadWindow_NewMail, NMM_FORWARD, MUIV_TheBar_Qualifier);
  DoMethod(obj, MUIM_TheBar_Notify, TB_READ_SPAM,       MUIA_Pressed, FALSE, readWindow, 2, MUIM_ReadWindow_ClassifyMessage, BC_SPAM);
  DoMethod(obj, MUIM_TheBar_Notify, TB_READ_HAM,        MUIA_Pressed, FALSE, readWindow, 2, MUIM_ReadWindow_ClassifyMessage, BC_HAM);

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
      hideHam = TRUE;
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
  DoMethod(obj, MUIM_TheBar_SetAttr, TB_READ_NUM+1, MUIA_TheBar_Attr_Hide,     !C->SpamFilterEnabled);
  DoMethod(obj, MUIM_TheBar_SetAttr, TB_READ_SPAM,  MUIA_TheBar_Attr_Hide,     hideSpam);
  DoMethod(obj, MUIM_TheBar_SetAttr, TB_READ_SPAM,  MUIA_TheBar_Attr_Disabled, disableSpam);
  DoMethod(obj, MUIM_TheBar_SetAttr, TB_READ_HAM,   MUIA_TheBar_Attr_Hide,     hideHam);

  RETURN(0);
  return 0;
}
///

