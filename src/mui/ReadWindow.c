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

 Superclass:  MUIC_Window
 Description: Window for reading emails

***************************************************************************/

#include "ReadWindow_cl.h"

#include <string.h>
#include <proto/dos.h>
#include <proto/muimaster.h>
#include <libraries/gadtools.h>
#include <libraries/iffparse.h>
#include <mui/NList_mcc.h>
#include <mui/TextEditor_mcc.h>
#include <mui/TheBar_mcc.h>

#include "SDI_hook.h"

#include "YAM.h"
#include "YAM_addressbookEntry.h"
#include "YAM_config.h"
#include "YAM_find.h"
#include "YAM_mainFolder.h"

#include "BayesFilter.h"
#include "FolderList.h"
#include "Locale.h"
#include "Logfile.h"
#include "MailList.h"
#include "MimeTypes.h"
#include "MUIObjects.h"
#include "Requesters.h"

#include "mui/MainMailListGroup.h"
#include "mui/ReadMailGroup.h"
#include "mui/ReadWindowStatusBar.h"
#include "mui/ReadWindowToolbar.h"

#include "Debug.h"

/* CLASSDATA
struct Data
{
  Object *MI_EDIT;
  Object *MI_MOVE;
  Object *MI_DELETE;
  Object *MI_DETACH;
  Object *MI_DELETEATT;
  Object *MI_CHSUBJ;
  Object *MI_NAVIG;
  Object *MI_WRAPH;
  Object *MI_TSTYLE;
  Object *MI_FFONT;
  Object *MI_PGP;
  Object *MI_EXTKEY;
  Object *MI_CHKSIG;
  Object *MI_SAVEDEC;
  Object *MI_REPLY;
  Object *MI_FORWARD;
  Object *MI_FORWARD_ATTACH;
  Object *MI_FORWARD_INLINE;
  Object *MI_REDIRECT;
  Object *MI_NEXTTHREAD;
  Object *MI_PREVTHREAD;
  Object *MI_STATUS;
  Object *MI_TOMARKED;
  Object *MI_TOUNMARKED;
  Object *MI_TOUNREAD;
  Object *MI_TOREAD;
  Object *MI_TOSPAM;
  Object *MI_TOHAM;
  Object *MI_SEARCH;
  Object *MI_SEARCHAGAIN;
  Object *MI_TCOLOR;
  Object *windowToolbar;
  Object *statusBar;
  Object *readMailGroup;

  char  title[SIZE_SUBJECT+1];
  int   lastDirection;
  int   windowNumber;
};
*/

// menu item IDs
enum
{
  RMEN_EDIT=501,RMEN_MOVE,RMEN_COPY,RMEN_DELETE,RMEN_PRINT,RMEN_SAVE,RMEN_DISPLAY,RMEN_DETACH,
  RMEN_DELETEATT,RMEN_NEW,RMEN_REPLY,RMEN_FORWARD_ATTACH,RMEN_FORWARD_INLINE,RMEN_REDIRECT,
  RMEN_SAVEADDR,RMEN_CHSUBJ,RMEN_PREV,RMEN_NEXT,RMEN_URPREV,RMEN_URNEXT,RMEN_PREVTH,
  RMEN_NEXTTH,RMEN_EXTKEY,RMEN_CHKSIG,RMEN_SAVEDEC,RMEN_HNONE,RMEN_HSHORT,RMEN_HFULL,
  RMEN_SNONE,RMEN_SDATA,RMEN_SFULL,RMEN_WRAPH,RMEN_TSTYLE,RMEN_FFONT,RMEN_SIMAGE,RMEN_TOMARKED,
  RMEN_TOUNMARKED,RMEN_TOUNREAD,RMEN_TOREAD,RMEN_TOSPAM,RMEN_TOHAM,
  RMEN_SEARCH,RMEN_SEARCHAGAIN,RMEN_EDIT_COPY,RMEN_EDIT_SALL,RMEN_EDIT_SNONE,RMEN_TCOLOR
};

/* Private Functions */
/// SelectMessage()
//  Activates a message in the main window's message listview
INLINE LONG SelectMessage(struct Mail *mail)
{
  LONG pos = MUIV_NList_GetPos_Start;

  // make sure the folder of the mail is currently the
  // active one.
  MA_ChangeFolder(mail->Folder, TRUE);

  // get the position of the mail in the main mail listview
  DoMethod(G->MA->GUI.PG_MAILLIST, MUIM_NList_GetPos, mail, &pos);

  // if it is currently viewable we go and set it as the
  // active mail
  if(pos != MUIV_NList_GetPos_End)
    set(G->MA->GUI.PG_MAILLIST, MUIA_NList_Active, pos);

  // return the position to the caller
  return pos;
}

///

/* Hooks */
/// CloseReadWindowHook()
//  Hook that will be called as soon as a read window is closed
HOOKPROTONHNO(CloseReadWindowFunc, void, struct ReadMailData **arg)
{
  struct ReadMailData *rmData = *arg;

  ENTER();

  // only if this is not a close operation because the application
  // is getting iconified we really cleanup our readmail data
  if(rmData == G->ActiveRexxRMData ||
     xget(G->App, MUIA_Application_Iconified) == FALSE)
  {
    // calls the CleanupReadMailData to clean everything else up
    CleanupReadMailData(rmData, TRUE);
  }

  LEAVE();
}
MakeStaticHook(CloseReadWindowHook, CloseReadWindowFunc);
///

/* Overloaded Methods */
/// OVERLOAD(OM_NEW)
OVERLOAD(OM_NEW)
{
  ULONG i=0;
  struct Data *data;
  struct Data *tmpData;
  Object *menuStripObject;

  ENTER();

  // generate a temporarly struct Data to which we store our data and
  // copy it later on
  if((data = tmpData = calloc(1, sizeof(struct Data))) == NULL)
  {
    RETURN(0);
    return 0;
  }

  // before we create all objects of this new read window we have to
  // check which number we can set for this window. Therefore we search in our
  // current ReadMailData list and check which number we can give this window
  do
  {
    struct ReadMailData *rmData;
    BOOL found = FALSE;

    IterateList(&G->readMailDataList, struct ReadMailData *,rmData)
    {
      if(rmData->readWindow != NULL &&
         xget(rmData->readWindow, MUIA_ReadWindow_Num) == i)
      {
        found = TRUE;
        break;
      }
    }

    // if we didn't find a window with the current ID then we can choose it as
    // our ReadWindow ID
    if(found == FALSE)
    {
      D(DBF_GUI, "Free window number %ld found.", i);
      data->windowNumber = i;

      break;
    }

    i++;
  }
  while(TRUE);

  //
  // now we create the Menustrip object with all the menu items
  // and corresponding shortcuts
  //
  // The following shortcut list should help to identify the hard-coded
  // shortcuts:
  //
  //  A   Select all text (RMEN_EDIT_SALL)
  //  B   Redirect mail (RMEN_REDIRECT)
  //  C   Copy selected text (RMEN_EDIT_COPY)
  //  D   Display mail part (RMEN_DISPLAY)
  //  E   Edit mail in Editor (RMEN_EDIT)
  //  F   Search in mail (RMEN_SEARCH)
  //  G   Search again in mail (RMEN_SEARCHAGAIN)
  //  H   Enable/Disabled 'Wrap Header' (RMEN_WRAPH)
  //  I   Enable/Disbaled 'Fixed Font' (RMEN_FFONT)
  //  J   Save address (RMEN_SAVEADDR)
  //  K   Check PGP signature (RMEN_CHKSIG)
  //  L   Save all attachments (RMEN_DETACH)
  //  M   Move mail (RMEN_MOVE)
  //  N   Create new mail (RMEN_NEW)
  //  O   Delete attachments (RMEN_DELETEATT)
  //  P   Print mail part (RMEN_PRINT)
  //  Q
  //  R   Reply mail (RMEN_REPLY)
  //  S   Save mail part (RMEN_SAVE)
  //  T   Enable/Disable Text Colors (RMEN_TCOLOR)
  //  U
  //  V   Save PGP decrypted mail (RMEN_SAVEDEC)
  //  W   Forward mail (RMEN_FORWARD_ATTACH or RMEN_FORWARD_INLINE)
  //  X   Extract PGP key from mail (RMEN_EXTKEY)
  //  Y   Copy mail (RMEN_COPY)
  //  Z
  // Del  Remove mail (RMEN_DELETE)
  //  ,   Set status to 'marked' (RMEN_TOMARKED)
  //  .   Set status to 'unmarked' (RMEN_TOUNMARKED)
  //  [   Set status to 'unread' (RMEN_TOUNREAD)
  //  ]   Set status to 'read' (RMEN_TOREAD)
  //
  // InputEvent shortcuts:
  //
  // -capslock del                : delete mail (RMEN_DELETE)
  // -capslock shift del          : delete mail at once
  // -repeat -capslock space      : move one page forward in texteditor display
  // -repeat -capslock backspace  : move one page backward in texteditor display
  // -repeat -capslock left       : Display previous mail in folder (RMEN_PREV)
  // -repeat -capslock right      : Display next mail in folder (RMEN_NEXT)
  // -repeat -capslock shift left : Display previous unread mail in folder (RMEN_URPREV)
  // -repeat -capslock shift right: Display next unread mail in folder (RMEN_URNEXT)
  // -repeat -capslock alt left   : Display previous mail in message thread (RMEN_PREVTH)
  // -repeat -capslock alt right  : Display next mail in message thread (RMEN_NEXTTH)
  //
  //

  menuStripObject = MenustripObject,
    MenuChild, MenuObject,
      MUIA_Menu_Title, tr(MSG_Message),
      MUIA_Menu_CopyStrings, FALSE,
      MenuChild, data->MI_EDIT = Menuitem(tr(MSG_MA_MEDIT), "E", TRUE, FALSE, RMEN_EDIT),
      MenuChild, data->MI_MOVE = Menuitem(tr(MSG_MA_MMOVE), "M", TRUE, FALSE, RMEN_MOVE),
      MenuChild, Menuitem(tr(MSG_MA_MCOPY), "Y", TRUE, FALSE, RMEN_COPY),
      MenuChild, data->MI_DELETE = Menuitem(tr(MSG_MA_MDelete), "Del", TRUE, TRUE, RMEN_DELETE),
      MenuChild, MenuBarLabel,
      MenuChild, Menuitem(tr(MSG_Print), "P", TRUE, FALSE, RMEN_PRINT),
      MenuChild, Menuitem(tr(MSG_MA_Save), "S", TRUE, FALSE, RMEN_SAVE),
      MenuChild, MenuitemObject,
        MUIA_Menuitem_Title, tr(MSG_Attachments),
        MUIA_Menuitem_CopyStrings, FALSE,
        MenuChild, Menuitem(tr(MSG_RE_MDisplay), "D", TRUE, FALSE, RMEN_DISPLAY),
        MenuChild, data->MI_DETACH = Menuitem(tr(MSG_RE_SaveAll), "L", TRUE, FALSE, RMEN_DETACH),
        MenuChild, data->MI_DELETEATT = Menuitem(tr(MSG_MA_DELETEATT), "O", TRUE, FALSE, RMEN_DELETEATT),
      End,
      MenuChild, MenuBarLabel,
      MenuChild, Menuitem(tr(MSG_New), "N", TRUE, FALSE, RMEN_NEW),
      MenuChild, data->MI_REPLY = Menuitem(tr(MSG_MA_MREPLY), "R", TRUE, FALSE, RMEN_REPLY),
      MenuChild, data->MI_FORWARD = MenuitemObject,
        MUIA_Menuitem_Title, tr(MSG_MA_MFORWARD),
        MUIA_Menuitem_CopyStrings, FALSE,
        MenuChild, data->MI_FORWARD_ATTACH = Menuitem(tr(MSG_MA_MFORWARD_ATTACH), NULL, TRUE, FALSE, RMEN_FORWARD_ATTACH),
        MenuChild, data->MI_FORWARD_INLINE = Menuitem(tr(MSG_MA_MFORWARD_INLINE), NULL, TRUE, FALSE, RMEN_FORWARD_INLINE),
      End,
      MenuChild, data->MI_REDIRECT = Menuitem(tr(MSG_MA_MREDIRECT), "B", TRUE, FALSE, RMEN_REDIRECT),
      MenuChild, MenuBarLabel,
      MenuChild, data->MI_SEARCH = Menuitem(tr(MSG_RE_SEARCH), "F", TRUE, FALSE, RMEN_SEARCH),
      MenuChild, data->MI_SEARCHAGAIN = Menuitem(tr(MSG_RE_SEARCH_AGAIN), "G", TRUE, FALSE, RMEN_SEARCHAGAIN),
      MenuChild, MenuBarLabel,
      MenuChild, Menuitem(tr(MSG_MA_MGetAddress), "J", TRUE, FALSE, RMEN_SAVEADDR),
      MenuChild, data->MI_STATUS = MenuitemObject,
        MUIA_Menuitem_Title, tr(MSG_MA_SetStatus),
        MUIA_Menuitem_CopyStrings, FALSE,
        MenuChild, data->MI_TOMARKED = Menuitem(tr(MSG_MA_TOMARKED), ",", TRUE, FALSE, RMEN_TOMARKED),
        MenuChild, data->MI_TOUNMARKED = Menuitem(tr(MSG_MA_TOUNMARKED), ".", TRUE, FALSE, RMEN_TOUNMARKED),
        MenuChild, data->MI_TOREAD = Menuitem(tr(MSG_MA_TOREAD), "]", TRUE, FALSE, RMEN_TOREAD),
        MenuChild, data->MI_TOUNREAD = Menuitem(tr(MSG_MA_TOUNREAD), "[", TRUE, FALSE, RMEN_TOUNREAD),
      End,
      MenuChild, data->MI_CHSUBJ = Menuitem(tr(MSG_MA_ChangeSubj), NULL, TRUE, FALSE, RMEN_CHSUBJ),
    End,
    MenuChild, MenuObject,
      MUIA_Menu_Title, tr(MSG_MA_EDIT),
      MUIA_Menu_CopyStrings, FALSE,
      MenuChild, Menuitem(tr(MSG_MA_EDIT_COPY), "C", TRUE, FALSE, RMEN_EDIT_COPY),
      MenuChild, MenuBarLabel,
      MenuChild, Menuitem(tr(MSG_MA_EDIT_SALL), "A", TRUE, FALSE, RMEN_EDIT_SALL),
      MenuChild, Menuitem(tr(MSG_MA_EDIT_SNONE), NULL, TRUE, FALSE, RMEN_EDIT_SNONE),
    End,
    MenuChild, data->MI_NAVIG = MenuObject,
      MUIA_Menu_Title, tr(MSG_RE_Navigation),
      MUIA_Menu_CopyStrings, FALSE,
      MenuChild, Menuitem(tr(MSG_RE_MNext),    "right", TRUE, TRUE, RMEN_NEXT),
      MenuChild, Menuitem(tr(MSG_RE_MPrev),    "left",  TRUE, TRUE, RMEN_PREV),
      MenuChild, Menuitem(tr(MSG_RE_MURNext),  "shift right", TRUE, TRUE, RMEN_URNEXT),
      MenuChild, Menuitem(tr(MSG_RE_MURPrev),  "shift left",  TRUE, TRUE, RMEN_URPREV),
      MenuChild, data->MI_NEXTTHREAD = Menuitem(tr(MSG_RE_MNextTh), "alt right", TRUE, TRUE, RMEN_NEXTTH),
      MenuChild, data->MI_PREVTHREAD = Menuitem(tr(MSG_RE_MPrevTh), "alt left", TRUE, TRUE, RMEN_PREVTH),
    End,
    MenuChild, data->MI_PGP = MenuObject,
      MUIA_Menu_Title, "PGP",
      MUIA_Menu_CopyStrings, FALSE,
      MenuChild, data->MI_EXTKEY = Menuitem(tr(MSG_RE_ExtractKey), "X", TRUE, FALSE, RMEN_EXTKEY),
      MenuChild, data->MI_CHKSIG = Menuitem(tr(MSG_RE_SigCheck), "K", TRUE, FALSE, RMEN_CHKSIG),
      MenuChild, data->MI_SAVEDEC = Menuitem(tr(MSG_RE_SaveDecrypted), "V", TRUE, FALSE, RMEN_SAVEDEC),
    End,
    MenuChild, MenuObject,
      MUIA_Menu_Title, tr(MSG_MA_Settings),
      MUIA_Menu_CopyStrings, FALSE,
      MenuChild, MenuitemCheck(tr(MSG_RE_NoHeaders),    "0", TRUE, C->ShowHeader==HM_NOHEADER,    FALSE, 0x06, RMEN_HNONE),
      MenuChild, MenuitemCheck(tr(MSG_RE_ShortHeaders), "1", TRUE, C->ShowHeader==HM_SHORTHEADER, FALSE, 0x05, RMEN_HSHORT),
      MenuChild, MenuitemCheck(tr(MSG_RE_FullHeaders),  "2", TRUE, C->ShowHeader==HM_FULLHEADER,  FALSE, 0x03, RMEN_HFULL),
      MenuChild, MenuBarLabel,
      MenuChild, MenuitemCheck(tr(MSG_RE_NoSInfo),      "3", TRUE, C->ShowSenderInfo==SIM_OFF,    FALSE, 0xE0, RMEN_SNONE),
      MenuChild, MenuitemCheck(tr(MSG_RE_SInfo),        "4", TRUE, C->ShowSenderInfo==SIM_DATA,   FALSE, 0xD0, RMEN_SDATA),
      MenuChild, MenuitemCheck(tr(MSG_RE_SInfoImage),   "5", TRUE, C->ShowSenderInfo==SIM_ALL,    FALSE, 0x90, RMEN_SFULL),
      MenuChild, MenuitemCheck(tr(MSG_RE_SImageOnly),   "6", TRUE, C->ShowSenderInfo==SIM_IMAGE,  FALSE, 0x70, RMEN_SIMAGE),
      MenuChild, MenuBarLabel,
      MenuChild, data->MI_WRAPH  = MenuitemCheck(tr(MSG_RE_WrapHeader), "H",  TRUE, C->WrapHeader,        TRUE, 0, RMEN_WRAPH),
      MenuChild, data->MI_FFONT  = MenuitemCheck(tr(MSG_RE_FixedFont),  "I",  TRUE, C->FixedFontEdit,     TRUE, 0, RMEN_FFONT),
      MenuChild, data->MI_TCOLOR = MenuitemCheck(tr(MSG_RE_TEXTCOLORS), "T",  TRUE, C->UseTextColorsRead, TRUE, 0, RMEN_TCOLOR),
      MenuChild, data->MI_TSTYLE = MenuitemCheck(tr(MSG_RE_Textstyles), NULL, TRUE, C->UseTextStylesRead, TRUE, 0, RMEN_TSTYLE),
    End,
  End;

  // create the read window object
  if(menuStripObject != NULL && (obj = DoSuperNew(cl, obj,

    MUIA_Window_Title, "",
    MUIA_HelpNode, "Windows#Readwindow",
    MUIA_Window_ID, MAKE_ID('R','D','W', data->windowNumber),
    MUIA_Window_Menustrip, menuStripObject,
    WindowContents, VGroup,
      Child, hasHideToolBarFlag(C->HideGUIElements) ?
        (RectangleObject, MUIA_ShowMe, FALSE, End) :
        (VGroup,
          Child, HGroupV,
            Child, data->windowToolbar = ReadWindowToolbarObject,
              MUIA_HelpNode, "Windows#ReadwindowToolbar",
            End,
          End,
          Child, data->statusBar = ReadWindowStatusBarObject,
          End,
        End),
        Child, VGroup,
          Child, data->readMailGroup = ReadMailGroupObject,
            MUIA_ReadMailGroup_HGVertWeight, G->Weights[10],
            MUIA_ReadMailGroup_TGVertWeight, G->Weights[11],
          End,
        End,
      End,

    TAG_MORE, inittags(msg))) != NULL)
  {
    struct ReadMailData *rmData = (struct ReadMailData *)xget(data->readMailGroup, MUIA_ReadMailGroup_ReadMailData);

    if(rmData == NULL ||
       (data = (struct Data *)INST_DATA(cl,obj)) == NULL)
    {
      RETURN(0);
      return 0;
    }

    // copy back the data stored in our temporarly struct Data
    memcpy(data, tmpData, sizeof(struct Data));

    // place this newly created window to the readMailData structure aswell
    rmData->readWindow = obj;

    // set the MUIA_UserData attribute to our readMailData struct
    // as we might need it later on
    set(obj, MUIA_UserData, rmData);

    // Add the window to the application object
    DoMethod(G->App, OM_ADDMEMBER, obj);

    // setup the toolbar notifies
    if(data->windowToolbar != NULL)
      DoMethod(data->windowToolbar, MUIM_ReadWindowToolbar_InitNotify, obj, data->readMailGroup);

    // set the default window object
    set(obj, MUIA_Window_DefaultObject, data->readMailGroup);

    // set some Notifies
    DoMethod(obj, MUIM_Notify, MUIA_Window_MenuAction, RMEN_EDIT,           obj, 3, MUIM_ReadWindow_NewMail, NMM_EDIT, 0);
    DoMethod(obj, MUIM_Notify, MUIA_Window_MenuAction, RMEN_MOVE,           obj, 1, MUIM_ReadWindow_MoveMailRequest);
    DoMethod(obj, MUIM_Notify, MUIA_Window_MenuAction, RMEN_COPY,           obj, 1, MUIM_ReadWindow_CopyMailRequest);
    DoMethod(obj, MUIM_Notify, MUIA_Window_MenuAction, RMEN_DELETE,         obj, 2, MUIM_ReadWindow_DeleteMailRequest, 0);
    DoMethod(obj, MUIM_Notify, MUIA_Window_MenuAction, RMEN_PRINT,          data->readMailGroup, 1, MUIM_ReadMailGroup_PrintMailRequest);
    DoMethod(obj, MUIM_Notify, MUIA_Window_MenuAction, RMEN_SAVE,           data->readMailGroup, 1, MUIM_ReadMailGroup_SaveMailRequest);
    DoMethod(obj, MUIM_Notify, MUIA_Window_MenuAction, RMEN_DISPLAY,        data->readMailGroup, 1, MUIM_ReadMailGroup_DisplayMailRequest);
    DoMethod(obj, MUIM_Notify, MUIA_Window_MenuAction, RMEN_DETACH,         data->readMailGroup, 1, MUIM_ReadMailGroup_SaveAllAttachments);
    DoMethod(obj, MUIM_Notify, MUIA_Window_MenuAction, RMEN_DELETEATT,      data->readMailGroup, 1, MUIM_ReadMailGroup_DeleteAttachmentsRequest);
    DoMethod(obj, MUIM_Notify, MUIA_Window_MenuAction, RMEN_NEW,            obj, 3, MUIM_ReadWindow_NewMail, NMM_NEW, 0);
    DoMethod(obj, MUIM_Notify, MUIA_Window_MenuAction, RMEN_REPLY,          obj, 3, MUIM_ReadWindow_NewMail, NMM_REPLY, 0);
    DoMethod(obj, MUIM_Notify, MUIA_Window_MenuAction, RMEN_FORWARD_ATTACH, obj, 3, MUIM_ReadWindow_NewMail, NMM_FORWARD_ATTACH, 0);
    DoMethod(obj, MUIM_Notify, MUIA_Window_MenuAction, RMEN_FORWARD_INLINE, obj, 3, MUIM_ReadWindow_NewMail, NMM_FORWARD_INLINE, 0);
    DoMethod(obj, MUIM_Notify, MUIA_Window_MenuAction, RMEN_REDIRECT,       obj, 3, MUIM_ReadWindow_NewMail, NMM_REDIRECT, 0);
    DoMethod(obj, MUIM_Notify, MUIA_Window_MenuAction, RMEN_SAVEADDR,       obj, 1, MUIM_ReadWindow_GrabSenderAddress);
    DoMethod(obj, MUIM_Notify, MUIA_Window_MenuAction, RMEN_SEARCH,         data->readMailGroup, 2, MUIM_ReadMailGroup_Search, MUIF_NONE);
    DoMethod(obj, MUIM_Notify, MUIA_Window_MenuAction, RMEN_SEARCHAGAIN,    data->readMailGroup, 2, MUIM_ReadMailGroup_Search, MUIF_ReadMailGroup_Search_Again);

    DoMethod(obj, MUIM_Notify, MUIA_Window_MenuAction, RMEN_TOUNREAD,       obj, 3, MUIM_ReadWindow_SetStatusTo, SFLAG_NONE, SFLAG_NEW|SFLAG_READ);
    DoMethod(obj, MUIM_Notify, MUIA_Window_MenuAction, RMEN_TOREAD,         obj, 3, MUIM_ReadWindow_SetStatusTo, SFLAG_READ, SFLAG_NEW);
    DoMethod(obj, MUIM_Notify, MUIA_Window_MenuAction, RMEN_TOMARKED,       obj, 3, MUIM_ReadWindow_SetStatusTo, SFLAG_MARKED, SFLAG_NONE);
    DoMethod(obj, MUIM_Notify, MUIA_Window_MenuAction, RMEN_TOUNMARKED,     obj, 3, MUIM_ReadWindow_SetStatusTo, SFLAG_NONE, SFLAG_MARKED);
    DoMethod(obj, MUIM_Notify, MUIA_Window_MenuAction, RMEN_TOSPAM,         obj, 2, MUIM_ReadWindow_ClassifyMessage, BC_SPAM);
    DoMethod(obj, MUIM_Notify, MUIA_Window_MenuAction, RMEN_TOHAM,          obj, 2, MUIM_ReadWindow_ClassifyMessage, BC_HAM);

    DoMethod(obj, MUIM_Notify, MUIA_Window_MenuAction, RMEN_CHSUBJ,         obj, 1, MUIM_ReadWindow_ChangeSubjectRequest);
    DoMethod(obj, MUIM_Notify, MUIA_Window_MenuAction, RMEN_EDIT_COPY,      data->readMailGroup, 2, MUIM_ReadMailGroup_DoEditAction, EA_COPY, MUIF_NONE);
    DoMethod(obj, MUIM_Notify, MUIA_Window_MenuAction, RMEN_EDIT_SALL,      data->readMailGroup, 2, MUIM_ReadMailGroup_DoEditAction, EA_SELECTALL, MUIF_NONE);
    DoMethod(obj, MUIM_Notify, MUIA_Window_MenuAction, RMEN_EDIT_SNONE,     data->readMailGroup, 2, MUIM_ReadMailGroup_DoEditAction, EA_SELECTNONE, MUIF_NONE);
    DoMethod(obj, MUIM_Notify, MUIA_Window_MenuAction, RMEN_PREV,           obj, 3, MUIM_ReadWindow_SwitchMail, -1, 0);
    DoMethod(obj, MUIM_Notify, MUIA_Window_MenuAction, RMEN_NEXT,           obj, 3, MUIM_ReadWindow_SwitchMail, +1, 0);
    DoMethod(obj, MUIM_Notify, MUIA_Window_MenuAction, RMEN_URPREV,         obj, 3, MUIM_ReadWindow_SwitchMail, -1, IEQUALIFIER_LSHIFT);
    DoMethod(obj, MUIM_Notify, MUIA_Window_MenuAction, RMEN_URNEXT,         obj, 3, MUIM_ReadWindow_SwitchMail, +1, IEQUALIFIER_LSHIFT);
    DoMethod(obj, MUIM_Notify, MUIA_Window_MenuAction, RMEN_PREVTH,         obj, 2, MUIM_ReadWindow_FollowThread, -1);
    DoMethod(obj, MUIM_Notify, MUIA_Window_MenuAction, RMEN_NEXTTH,         obj, 2, MUIM_ReadWindow_FollowThread, +1);
    DoMethod(obj, MUIM_Notify, MUIA_Window_MenuAction, RMEN_EXTKEY,         data->readMailGroup, 1, MUIM_ReadMailGroup_ExtractPGPKey);
    DoMethod(obj, MUIM_Notify, MUIA_Window_MenuAction, RMEN_CHKSIG,         data->readMailGroup, 2, MUIM_ReadMailGroup_CheckPGPSignature, TRUE);
    DoMethod(obj, MUIM_Notify, MUIA_Window_MenuAction, RMEN_SAVEDEC,        data->readMailGroup, 1, MUIM_ReadMailGroup_SaveDecryptedMail);
    DoMethod(obj, MUIM_Notify, MUIA_Window_MenuAction, RMEN_HNONE,          obj, 2, MUIM_ReadWindow_ChangeHeaderMode, HM_NOHEADER);
    DoMethod(obj, MUIM_Notify, MUIA_Window_MenuAction, RMEN_HSHORT,         obj, 2, MUIM_ReadWindow_ChangeHeaderMode, HM_SHORTHEADER);
    DoMethod(obj, MUIM_Notify, MUIA_Window_MenuAction, RMEN_HFULL,          obj, 2, MUIM_ReadWindow_ChangeHeaderMode, HM_FULLHEADER);
    DoMethod(obj, MUIM_Notify, MUIA_Window_MenuAction, RMEN_SNONE,          obj, 2, MUIM_ReadWindow_ChangeSenderInfoMode, SIM_OFF);
    DoMethod(obj, MUIM_Notify, MUIA_Window_MenuAction, RMEN_SDATA,          obj, 2, MUIM_ReadWindow_ChangeSenderInfoMode, SIM_DATA);
    DoMethod(obj, MUIM_Notify, MUIA_Window_MenuAction, RMEN_SFULL,          obj, 2, MUIM_ReadWindow_ChangeSenderInfoMode, SIM_ALL);
    DoMethod(obj, MUIM_Notify, MUIA_Window_MenuAction, RMEN_SIMAGE,         obj, 2, MUIM_ReadWindow_ChangeSenderInfoMode, SIM_IMAGE);
    DoMethod(obj, MUIM_Notify, MUIA_Window_MenuAction, RMEN_WRAPH,          obj, 1, MUIM_ReadWindow_StyleOptionsChanged);
    DoMethod(obj, MUIM_Notify, MUIA_Window_MenuAction, RMEN_TCOLOR,         obj, 1, MUIM_ReadWindow_StyleOptionsChanged);
    DoMethod(obj, MUIM_Notify, MUIA_Window_MenuAction, RMEN_TSTYLE,         obj, 1, MUIM_ReadWindow_StyleOptionsChanged);
    DoMethod(obj, MUIM_Notify, MUIA_Window_MenuAction, RMEN_FFONT,          obj, 1, MUIM_ReadWindow_StyleOptionsChanged);
    DoMethod(obj, MUIM_Notify, MUIA_Window_InputEvent, "-capslock del",                 obj, 2, MUIM_ReadWindow_DeleteMailRequest, 0);
    DoMethod(obj, MUIM_Notify, MUIA_Window_InputEvent, "-capslock shift del",           obj, 2, MUIM_ReadWindow_DeleteMailRequest, IEQUALIFIER_LSHIFT);
    DoMethod(obj, MUIM_Notify, MUIA_Window_InputEvent, "-repeat -capslock space",       data->readMailGroup, 2, MUIM_TextEditor_ARexxCmd, "Next Page");
    DoMethod(obj, MUIM_Notify, MUIA_Window_InputEvent, "-repeat -capslock backspace",   data->readMailGroup, 2, MUIM_TextEditor_ARexxCmd, "Previous Page");
    DoMethod(obj, MUIM_Notify, MUIA_Window_InputEvent, "-repeat -capslock left",        obj, 3, MUIM_ReadWindow_SwitchMail, -1, 0);
    DoMethod(obj, MUIM_Notify, MUIA_Window_InputEvent, "-repeat -capslock right",       obj, 3, MUIM_ReadWindow_SwitchMail, +1, 0);
    DoMethod(obj, MUIM_Notify, MUIA_Window_InputEvent, "-repeat -capslock shift left",  obj, 3, MUIM_ReadWindow_SwitchMail, -1, IEQUALIFIER_LSHIFT);
    DoMethod(obj, MUIM_Notify, MUIA_Window_InputEvent, "-repeat -capslock shift right", obj, 3, MUIM_ReadWindow_SwitchMail, +1, IEQUALIFIER_LSHIFT);
    DoMethod(obj, MUIM_Notify, MUIA_Window_InputEvent, "-repeat -capslock alt left",    obj, 2, MUIM_ReadWindow_FollowThread, -1);
    DoMethod(obj, MUIM_Notify, MUIA_Window_InputEvent, "-repeat -capslock alt right",   obj, 2, MUIM_ReadWindow_FollowThread, +1);

    // update the "Forward" items' shortcut
    DoMethod(obj, MUIM_ReadWindow_UpdateMenuShortcuts);

    // make sure the right menus/toolbar spam button items are available
    DoMethod(obj, MUIM_ReadWindow_UpdateSpamControls);

    // before we continue we make sure we connect a notify to the new window
    // so that we get informed if the window is closed and therefore can be
    // disposed
    // However, please note that because we do kill the window upon closing it
    // we have to use MUIM_Application_PushMethod instead of calling the CloseReadWindowHook
    // directly
    DoMethod(obj, MUIM_Notify, MUIA_Window_CloseRequest, TRUE,
                  MUIV_Notify_Application, 6,
                    MUIM_Application_PushMethod, G->App, 3, MUIM_CallHook, &CloseReadWindowHook, rmData);
  }
  else
    obj = NULL;

  // free the temporary mem we allocated before
  free(tmpData);

  RETURN((IPTR)obj);
  return (IPTR)obj;
}

///
/// OVERLOAD(OM_GET)
OVERLOAD(OM_GET)
{
  GETDATA;
  IPTR *store = ((struct opGet *)msg)->opg_Storage;

  switch(((struct opGet *)msg)->opg_AttrID)
  {
    case ATTR(ReadMailData) : *store = (ULONG)xget(data->readMailGroup, MUIA_ReadMailGroup_ReadMailData); return TRUE;
    case ATTR(Num)          : *store = data->windowNumber; return TRUE;
  }

  return DoSuperMethodA(cl, obj, msg);
}
///
/// OVERLOAD(OM_SET)
OVERLOAD(OM_SET)
{
  GETDATA;
  struct TagItem *tags = inittags(msg), *tag;

  while((tag = NextTagItem((APTR)&tags)) != NULL)
  {
    switch(tag->ti_Tag)
    {
      // we also catch foreign attributes
      case MUIA_Window_CloseRequest:
      {
        // if the window is supposed to be closed and the StatusChangeDelay is
        // active and no embeddedReadPane is active we have to cancel an eventually
        // existing timerequest to set the status of a mail to read.
        if(tag->ti_Data == TRUE &&
           C->StatusChangeDelayOn == TRUE && C->EmbeddedReadPane == FALSE &&
           xget(obj, MUIA_Window_Open) == TRUE)
        {
          StopTimer(TIMER_READSTATUSUPDATE);
        }
      }
      break;

      case MUIA_Window_DefaultObject:
      {
        // if the user clicks somewhere where the default
        // object would be set to NULL we make sure we set
        // it back to the default object of the readmail group
        if((Object *)tag->ti_Data == NULL)
          tag->ti_Data = xget(data->readMailGroup, MUIA_ReadMailGroup_DefaultObject);
      }
      break;
    }
  }

  return DoSuperMethodA(cl, obj, msg);
}
///
/// OVERLOAD(MUIM_Window_Snapshot)
OVERLOAD(MUIM_Window_Snapshot)
{
  struct MUIP_Window_Snapshot *snap = (struct MUIP_Window_Snapshot *)msg;
  GETDATA;

  // remember the weights for snapshot operations, but not for unsnapshot operations
  if(snap->flags != 0)
  {
    // on a snapshot request we save the weights of all our objects here.
    G->Weights[10] = xget(data->readMailGroup, MUIA_ReadMailGroup_HGVertWeight);
    G->Weights[11] = xget(data->readMailGroup, MUIA_ReadMailGroup_TGVertWeight);

    // make sure the layout is saved
    SaveLayout(TRUE);
  }

  return DoSuperMethodA(cl, obj, msg);
}

///

/* Public Methods */
/// DECLARE(ReadMail)
DECLARE(ReadMail) // struct Mail *mail
{
  GETDATA;
  struct Mail *mail = msg->mail;
  struct Folder *folder = mail->Folder;
  BOOL isRealMail   = !isVirtualMail(mail);
  BOOL isSentMail   = isRealMail && isSentMailFolder(folder);
  BOOL hasAttach    = isMP_MixedMail(mail);
  BOOL inSpamFolder = isRealMail && isSpamFolder(folder);
  BOOL result = FALSE;
  BOOL initialCall = data->title[0] == '\0'; // TRUE if this is the first call
  BOOL prevMailAvailable = FALSE;
  BOOL nextMailAvailable = FALSE;

  ENTER();

  D(DBF_GUI, "setting up readWindow for reading a mail");

  // check the status of the next/prev thread nagivation
  if(isRealMail == TRUE)
  {
    if(AllFolderLoaded() == TRUE)
    {
      prevMailAvailable = FindThreadInFolder(mail, mail->Folder, FALSE) != NULL;
      nextMailAvailable = FindThreadInFolder(mail, mail->Folder, TRUE) != NULL;
    }
    else
    {
      prevMailAvailable = TRUE;
      nextMailAvailable = TRUE;
    }
  }

  // change the menu item title of the
  // Edit item so that we either display "Edit" or "Edit as New"
  if(isRealMail == TRUE && isDraftsFolder(folder))
    set(data->MI_EDIT, MUIA_Menuitem_Title, tr(MSG_MA_MEDIT));
  else
    set(data->MI_EDIT, MUIA_Menuitem_Title, tr(MSG_MA_MEDITASNEW));

  // enable/disable some menuitems in advance
  set(data->MI_EDIT,      MUIA_Menuitem_Enabled, isRealMail && !inSpamFolder);
  set(data->MI_MOVE,      MUIA_Menuitem_Enabled, isRealMail);
  set(data->MI_DELETE,    MUIA_Menuitem_Enabled, isRealMail);
  set(data->MI_DETACH,    MUIA_Menuitem_Enabled, hasAttach);
  set(data->MI_DELETEATT, MUIA_Menuitem_Enabled, isRealMail && hasAttach);
  set(data->MI_CHSUBJ,    MUIA_Menuitem_Enabled, isRealMail && !inSpamFolder);
  set(data->MI_NAVIG,     MUIA_Menu_Enabled,     isRealMail);
  set(data->MI_REPLY,     MUIA_Menuitem_Enabled, !inSpamFolder && !hasStatusSpam(mail));
  set(data->MI_REDIRECT,  MUIA_Menuitem_Enabled, !isSentMail);
  set(data->MI_NEXTTHREAD,MUIA_Menuitem_Enabled, nextMailAvailable);
  set(data->MI_PREVTHREAD,MUIA_Menuitem_Enabled, prevMailAvailable);

  // Enable if:
  //  * the mail is a real (non-virtual) mail
  DoMethod(obj, MUIM_MultiSet, MUIA_Menuitem_Enabled, isRealMail, data->MI_TOMARKED,
                                                                  data->MI_TOUNMARKED,
                                                                  NULL);

  // Enable if:
  //  * the mail is a real (non-virtual) mail
  //  * NOT in the "Sent" folder
  DoMethod(obj, MUIM_MultiSet, MUIA_Menuitem_Enabled, isRealMail && !isSentMail, data->MI_TOREAD,
                                                                                 data->MI_TOUNREAD,
                                                                                 NULL);

  if(data->windowToolbar != NULL)
  {
    LONG pos = MUIV_NList_GetPos_Start;

    // query the position of the mail in the current listview
    DoMethod(G->MA->GUI.PG_MAILLIST, MUIM_NList_GetPos, mail, &pos);

    // now set some items of the toolbar ghosted/enabled
    DoMethod(data->windowToolbar, MUIM_TheBar_SetAttr, TB_READ_PREV,        MUIA_TheBar_Attr_Disabled, isRealMail ? pos == 0 : TRUE);
    DoMethod(data->windowToolbar, MUIM_TheBar_SetAttr, TB_READ_NEXT,        MUIA_TheBar_Attr_Disabled, isRealMail ? pos == (folder->Total-1) : TRUE);
    DoMethod(data->windowToolbar, MUIM_TheBar_SetAttr, TB_READ_PREVTHREAD,  MUIA_TheBar_Attr_Disabled, !prevMailAvailable);
    DoMethod(data->windowToolbar, MUIM_TheBar_SetAttr, TB_READ_NEXTTHREAD,  MUIA_TheBar_Attr_Disabled, !nextMailAvailable);
    DoMethod(data->windowToolbar, MUIM_TheBar_SetAttr, TB_READ_DELETE,      MUIA_TheBar_Attr_Disabled, !isRealMail);
    DoMethod(data->windowToolbar, MUIM_TheBar_SetAttr, TB_READ_MOVE,        MUIA_TheBar_Attr_Disabled, !isRealMail);
    DoMethod(data->windowToolbar, MUIM_TheBar_SetAttr, TB_READ_REPLY,       MUIA_TheBar_Attr_Disabled, inSpamFolder || hasStatusSpam(mail));
  }

  // update the status bar
  DoMethod(data->statusBar, MUIM_ReadWindowStatusBar_Update, mail);

  // now we read in the mail in our read mail group
  if(DoMethod(data->readMailGroup, MUIM_ReadMailGroup_ReadMail, mail,
                                   initialCall == FALSE ? MUIF_ReadMailGroup_ReadMail_StatusChangeDelay : MUIF_NONE))
  {
    size_t titleLen;
    struct ReadMailData *rmData = (struct ReadMailData *)xget(data->readMailGroup, MUIA_ReadMailGroup_ReadMailData);
    BOOL hasPGPKey = rmData->hasPGPKey;
    BOOL hasPGPSig = (hasPGPSOldFlag(rmData) || hasPGPSMimeFlag(rmData));
    BOOL isPGPEnc = isRealMail && (hasPGPEMimeFlag(rmData) || hasPGPEOldFlag(rmData));

    // if the title of the window is empty, we can assume that no previous mail was
    // displayed in this readwindow, so we can set the mailTextObject of the readmailgroup
    // as the active object so that the user can browse through the mailtext immediatley after
    // opening the window
    if(initialCall == TRUE)
      DoMethod(data->readMailGroup, MUIM_ReadMailGroup_ActivateMailText);

    // set the title of the readWindow now
    if(C->MultipleReadWindows == TRUE ||
       rmData == G->ActiveRexxRMData)
    {
      titleLen = snprintf(data->title, sizeof(data->title), "[%d] %s %s: ", data->windowNumber+1,
                                                            isSentMail ? tr(MSG_To) : tr(MSG_From),
                                                            isSentMail ? AddrName(mail->To) : AddrName(mail->From));
    }
    else
    {
      titleLen = snprintf(data->title, sizeof(data->title), "%s %s: ",
                                                            isSentMail ? tr(MSG_To) : tr(MSG_From),
                                                            isSentMail ? AddrName(mail->To) : AddrName(mail->From));
    }

    if(strlen(mail->Subject)+titleLen > sizeof(data->title)-1)
    {
      if(titleLen < sizeof(data->title)-4)
      {
        strlcat(data->title, mail->Subject, sizeof(data->title)-titleLen-4);
        strlcat(data->title, "...", sizeof(data->title)); // signals that the string was cut.
      }
      else
        strlcat(&data->title[sizeof(data->title)-5], "...", 4);
    }
    else
      strlcat(data->title, mail->Subject, sizeof(data->title));

    set(obj, MUIA_Window_Title, data->title);

    // enable some Menuitems depending on the read mail
    set(data->MI_PGP,     MUIA_Menu_Enabled, hasPGPKey || hasPGPSig || isPGPEnc);
    set(data->MI_EXTKEY,  MUIA_Menuitem_Enabled, hasPGPKey);
    set(data->MI_CHKSIG,  MUIA_Menuitem_Enabled, hasPGPSig);
    set(data->MI_SAVEDEC, MUIA_Menuitem_Enabled, isPGPEnc);

    // everything worked fine
    result = TRUE;
  }

  // update the spam/no spam menu items as well as the toolbar
  DoMethod(obj, MUIM_ReadWindow_UpdateSpamControls);

  RETURN(result);
  return result;
}

///
/// DECLARE(RereadMail)
DECLARE(RereadMail) // struct Mail *mail, ULONG flags
{
  GETDATA;

  ENTER();

  DoMethod(data->readMailGroup, MUIM_ReadMailGroup_ReadMail, msg->mail, msg->flags);

  RETURN(0);
  return 0;
}

///
/// DECLARE(NewMail)
DECLARE(NewMail) // enum NewMailMode mode, ULONG qualifier
{
  GETDATA;
  struct ReadMailData *rmData = (struct ReadMailData *)xget(data->readMailGroup, MUIA_ReadMailGroup_ReadMailData);
  struct Mail *mail = rmData->mail;

  ENTER();

  // then create a new mail depending on the current mode
  if(MailExists(mail, NULL) == TRUE)
  {
    enum NewMailMode mode;
    int flags;

    // get the newmail flags depending on the currently
    // set qualifier keys. We submit these flags to the
    // NewMessage() function later on
    mode = CheckNewMailQualifier(msg->mode, msg->qualifier, &flags);

    switch(mode)
    {
      case NMM_NEW:
        NewWriteMailWindow(mail, flags);
      break;

      case NMM_EDIT:
        NewEditMailWindow(mail, flags);
      break;

      case NMM_REDIRECT:
      {
        struct MailList *mlist;

        if((mlist = CreateMailList()) != NULL)
        {
          AddNewMailNode(mlist, mail);
          NewRedirectMailWindow(mlist, flags);
          DeleteMailList(mlist);
        }
      }
      break;

      case NMM_FORWARD:
      case NMM_FORWARD_ATTACH:
      case NMM_FORWARD_INLINE:
      {
        struct MailList *mlist;

        if((mlist = CreateMailList()) != NULL)
        {
          AddNewMailNode(mlist, mail);
          NewForwardMailWindow(mlist, flags);
          DeleteMailList(mlist);
        }
      }
      break;

      case NMM_REPLY:
      {
        struct MailList *mlist;

        // we create a fake mail list with only one entry
        // (the mail showing in this window)
        if((mlist = CreateMailList()) != NULL)
        {
          char *replytext;

          // in case there is only one mail selected we have to check wheter there is
          // text currently selected by the user
          replytext = (char *)DoMethod(data->readMailGroup, MUIM_ReadMailGroup_ExportSelectedText);

          // we only add one mail to the fake list
          AddNewMailNode(mlist, mail);

          // now we create a new reply mail window
          NewReplyMailWindow(mlist, flags, replytext);

          // free the replytext in case we go one
          if(replytext != NULL)
            FreeVec(replytext);

          // cleanup the temporarly created mail list
          DeleteMailList(mlist);
        }
      }
      break;

      default:
        // nothing
      break;
    }
  }

  RETURN(0);
  return 0;
}

///
/// DECLARE(MoveMailRequest)
DECLARE(MoveMailRequest)
{
  GETDATA;
  struct ReadMailData *rmData = (struct ReadMailData *)xget(data->readMailGroup, MUIA_ReadMailGroup_ReadMailData);
  struct Mail *mail = rmData->mail;
  struct Folder *srcfolder = mail->Folder;
  BOOL closeAfter = FALSE;

  ENTER();

  if(MailExists(mail, srcfolder) == TRUE)
  {
    struct Folder *dstfolder = FolderRequest(tr(MSG_MA_MoveMsg),
                                             tr(MSG_MA_MoveMsgReq),
                                             tr(MSG_MA_MoveGad),
                                             tr(MSG_Cancel),
                                             NULL,
                                             obj);

    if(dstfolder != NULL && dstfolder != srcfolder)
    {
      int pos = SelectMessage(mail); // select the message in the folder and return position
      int entries;

      // depending on the last move direction we
      // set it back
      if(data->lastDirection == -1)
      {
        if(pos-1 >= 0)
          set(G->MA->GUI.PG_MAILLIST, MUIA_NList_Active, --pos);
        else
          closeAfter = TRUE;
      }

      // move the mail to the selected destination folder
      MA_MoveCopy(mail, srcfolder, dstfolder, 0);

      // erase the old pointer as this has been free()ed by MA_MoveCopy()
      rmData->mail = NULL;

      // if there are still mails in the current folder we make sure
      // it is displayed in this window now or close it
      if(closeAfter == FALSE &&
         (entries = xget(G->MA->GUI.PG_MAILLIST, MUIA_NList_Entries)) >= pos+1)
      {
        DoMethod(G->MA->GUI.PG_MAILLIST, MUIM_NList_GetEntry, pos, &mail);
        if(mail)
          DoMethod(obj, MUIM_ReadWindow_ReadMail, mail);
        else
          closeAfter = TRUE;
      }
      else
        closeAfter = TRUE;

      // make sure the read window is closed in case there is no further
      // mail for deletion in this direction
      if(closeAfter)
        DoMethod(G->App, MUIM_Application_PushMethod, G->App, 3, MUIM_CallHook, &CloseReadWindowHook, rmData);

      AppendToLogfile(LF_NORMAL, 22, tr(MSG_LOG_Moving), 1, srcfolder->Name, dstfolder->Name);
    }
  }

  RETURN(0);
  return 0;
}

///
/// DECLARE(CopyMailRequest)
DECLARE(CopyMailRequest)
{
  GETDATA;
  struct ReadMailData *rmData = (struct ReadMailData *)xget(data->readMailGroup, MUIA_ReadMailGroup_ReadMailData);
  struct Mail *mail = rmData->mail;
  struct Folder *srcfolder = mail->Folder;

  ENTER();

  if(MailExists(mail, srcfolder) == TRUE)
  {
    struct Folder *dstfolder = FolderRequest(tr(MSG_MA_CopyMsg),
                                             tr(MSG_MA_MoveMsgReq),
                                             tr(MSG_MA_CopyGad),
                                             tr(MSG_Cancel),
                                             NULL,
                                             obj);
    if(dstfolder != NULL)
    {
      // if there is no source folder this is a virtual mail that we
      // export to the destination folder
      if(srcfolder != NULL)
      {
        MA_MoveCopy(mail, srcfolder, dstfolder, MVCPF_COPY);

        AppendToLogfile(LF_NORMAL, 24, tr(MSG_LOG_Copying), 1, srcfolder->Name, dstfolder->Name);
      }
      else
      {
        char mfilePath[SIZE_PATHFILE];

        if(MA_NewMailFile(dstfolder, mfilePath, sizeof(mfilePath)) == TRUE)
        {
          strlcpy(mail->MailFile, FilePart(mfilePath), sizeof(mail->MailFile));
          if(RE_Export(rmData, rmData->readFile, mfilePath, "", 0, FALSE, FALSE, IntMimeTypeArray[MT_ME_EMAIL].ContentType) == TRUE)
          {
            struct Mail *newmail;

            if((newmail = AddMailToFolder(mail, dstfolder)) != NULL)
            {
              if(GetCurrentFolder() == dstfolder)
                DoMethod(G->MA->GUI.PG_MAILLIST, MUIM_NList_InsertSingle, newmail, MUIV_NList_Insert_Sorted);

              setStatusToRead(newmail); // OLD status
            }
          }
        }
      }
    }
  }

  RETURN(0);
  return 0;
}

///
/// DECLARE(DeleteMailRequest)
DECLARE(DeleteMailRequest) // ULONG qualifier
{
  GETDATA;
  struct ReadMailData *rmData = (struct ReadMailData *)xget(data->readMailGroup, MUIA_ReadMailGroup_ReadMailData);
  struct Mail *mail = rmData->mail;
  struct Folder *folder = mail->Folder;
  struct Folder *delfolder = FO_GetFolderByType(FT_TRASH, NULL);
  BOOL delatonce = isAnyFlagSet(msg->qualifier, (IEQUALIFIER_LSHIFT|IEQUALIFIER_RSHIFT));
  BOOL closeAfter = FALSE;

  ENTER();

  if(MailExists(mail, folder))
  {
    int pos = SelectMessage(mail); // select the message in the folder and return position
    int entries;

    // depending on the last move direction we
    // set it back
    if(data->lastDirection == -1)
    {
      if(pos-1 >= 0)
        set(G->MA->GUI.PG_MAILLIST, MUIA_NList_Active, --pos);
      else
        closeAfter = TRUE;
    }

    // delete the mail
    MA_DeleteSingle(mail, delatonce ? DELF_AT_ONCE|DELF_UPDATE_APPICON|DELF_CHECK_CONNECTIONS : DELF_UPDATE_APPICON|DELF_CHECK_CONNECTIONS);

    // erase the old pointer as this has been free()ed by MA_DeleteSingle()
    rmData->mail = NULL;

    // if there are still mails in the current folder we make sure
    // it is displayed in this window now or close it
    if(closeAfter == FALSE &&
       (entries = xget(G->MA->GUI.PG_MAILLIST, MUIA_NList_Entries)) >= pos+1)
    {
      DoMethod(G->MA->GUI.PG_MAILLIST, MUIM_NList_GetEntry, pos, &mail);
      if(mail)
        DoMethod(obj, MUIM_ReadWindow_ReadMail, mail);
      else
        closeAfter = TRUE;
    }
    else
      closeAfter = TRUE;

    // make sure the read window is closed in case there is no further
    // mail for deletion in this direction
    if(closeAfter == TRUE)
      DoMethod(G->App, MUIM_Application_PushMethod, G->App, 3, MUIM_CallHook, &CloseReadWindowHook, rmData);

    if(delatonce || isSpamFolder(folder))
      AppendToLogfile(LF_NORMAL, 20, tr(MSG_LOG_Deleting), 1, folder->Name);
    else
      AppendToLogfile(LF_NORMAL, 22, tr(MSG_LOG_Moving), 1, folder->Name, delfolder->Name);
  }

  RETURN(0);
  return 0;
}

///
/// DECLARE(ClassifyMessage)
DECLARE(ClassifyMessage) // enum BayesClassification class
{
  GETDATA;
  struct ReadMailData *rmData = (struct ReadMailData *)xget(data->readMailGroup, MUIA_ReadMailGroup_ReadMailData);
  struct Mail *mail = rmData->mail;
  struct Folder *folder = mail->Folder;
  struct Folder *spamFolder = FO_GetFolderByType(FT_SPAM, NULL);
  struct Folder *incomingFolder = FO_GetFolderByType(FT_INCOMING, NULL);
  enum BayesClassification class = msg->class;

  ENTER();

  if(MailExists(mail, folder) && spamFolder != NULL && incomingFolder != NULL)
  {
    if(hasStatusSpam(mail) == FALSE && class == BC_SPAM)
    {
      BOOL closeAfter = FALSE;
      int pos = SelectMessage(mail); // select the message in the folder and return position
      int entries;

      // depending on the last move direction we
      // set it back
      if(data->lastDirection == -1)
      {
        if(pos-1 >= 0)
          set(G->MA->GUI.PG_MAILLIST, MUIA_NList_Active, --pos);
        else
          closeAfter = TRUE;
      }

      // mark the mail as user spam
      AppendToLogfile(LF_VERBOSE, 90, tr(MSG_LOG_MAILISSPAM), AddrName(mail->From), mail->Subject);
      BayesFilterSetClassification(mail, BC_SPAM);
      setStatusToUserSpam(mail);

      // move the mail
      MA_MoveCopy(mail, folder, spamFolder, 0);

      // erase the old pointer as this has been free()ed by MA_MoveCopy()
      rmData->mail = NULL;

      // if there are still mails in the current folder we make sure
      // it is displayed in this window now or close it
      if(closeAfter == FALSE &&
         (entries = xget(G->MA->GUI.PG_MAILLIST, MUIA_NList_Entries)) >= pos+1)
      {
        DoMethod(G->MA->GUI.PG_MAILLIST, MUIM_NList_GetEntry, pos, &mail);
        if(mail != NULL)
          DoMethod(obj, MUIM_ReadWindow_ReadMail, mail);
        else
          closeAfter = TRUE;
      }
      else
        closeAfter = TRUE;

      // make sure the read window is closed in case there is no further
      // mail for deletion in this direction
      if(closeAfter == TRUE)
        DoMethod(G->App, MUIM_Application_PushMethod, G->App, 3, MUIM_CallHook, &CloseReadWindowHook, rmData);
      else
      {
        // only update the menu/toolbar if we are already in the spam folder
        // otherwise a new mail will be read later or the window is closed
        if(folder == spamFolder)
          DoMethod(obj, MUIM_ReadWindow_UpdateSpamControls);

        // update the status bar
        DoMethod(obj, MUIM_ReadWindow_UpdateStatusBar);
      }
    }
    else if(hasStatusHam(mail) == FALSE && class == BC_HAM)
    {
      BOOL closeAfter = FALSE;
      int pos = SelectMessage(mail); // select the message in the folder and return position
      int entries;

      // depending on the last move direction we
      // set it back
      if(data->lastDirection == -1)
      {
        if(pos-1 >= 0)
          set(G->MA->GUI.PG_MAILLIST, MUIA_NList_Active, --pos);
        else
          closeAfter = TRUE;
      }

      // mark the mail as ham
      AppendToLogfile(LF_VERBOSE, 90, tr(MSG_LOG_MAILISNOTSPAM), AddrName(mail->From), mail->Subject);
      BayesFilterSetClassification(mail, BC_HAM);
      setStatusToHam(mail);

      if(C->MoveHamToIncoming == TRUE)
      {
        BOOL moveToIncoming = TRUE;

        // first try to apply the filters to this mail, if requested
        if(C->FilterHam == TRUE)
        {
          struct MinList *filterList;

          if((filterList = CloneFilterList(APPLY_USER)) != NULL)
          {
            // FI_FilterSingleMail() returns TRUE if the filters didn't move or delete the mail.
            // If the mail is still in place after filtering we will move it back to the incoming
            // folder later.
            moveToIncoming = FI_FilterSingleMail(filterList, mail, NULL, NULL);
            DeleteFilterList(filterList);
          }
        }

        // if the mail has not been moved to another folder before we move it to the incoming folder now.
        if(moveToIncoming == TRUE)
          MA_MoveCopy(mail, folder, incomingFolder, 0);

        // erase the old pointer as this has been free()ed by MA_MoveCopy() or by the filter action
        rmData->mail = NULL;

        // if there are still mails in the current folder we make sure
        // it is displayed in this window now or close it
        if(closeAfter == FALSE &&
           (entries = xget(G->MA->GUI.PG_MAILLIST, MUIA_NList_Entries)) >= pos+1)
        {
          DoMethod(G->MA->GUI.PG_MAILLIST, MUIM_NList_GetEntry, pos, &mail);
          if(mail != NULL)
            DoMethod(obj, MUIM_ReadWindow_ReadMail, mail);
          else
            closeAfter = TRUE;
        }
        else
          closeAfter = TRUE;

        // make sure the read window is closed in case there is no further
        // mail for deletion in this direction
        if(closeAfter == TRUE)
          DoMethod(G->App, MUIM_Application_PushMethod, G->App, 3, MUIM_CallHook, &CloseReadWindowHook, rmData);
        else
        {
          // only update the menu/toolbar if we are already in the incoming folder
          // otherwise a new mail will be read later or the window is closed
          if(folder == incomingFolder)
            DoMethod(obj, MUIM_ReadWindow_UpdateSpamControls);

          // update the status bar
          DoMethod(obj, MUIM_ReadWindow_UpdateStatusBar);
        }
      }
      else
      {
        // update the menu/toolbar
        DoMethod(obj, MUIM_ReadWindow_UpdateSpamControls);

        // update the status bar
        DoMethod(obj, MUIM_ReadWindow_UpdateStatusBar);
      }
    }
  }

  RETURN(0);
  return 0;
}

///
/// DECLARE(GrabSenderAddress)
//  Stores sender address of current message in the address book
DECLARE(GrabSenderAddress)
{
  GETDATA;
  struct ReadMailData *rmData = (struct ReadMailData *)xget(data->readMailGroup, MUIA_ReadMailGroup_ReadMailData);
  struct Mail *mail = rmData->mail;

  ENTER();

  if(MailExists(mail, mail->Folder) == TRUE)
  {
    struct MailList *mlist;

    if((mlist = CreateMailList()) != NULL)
    {
      AddNewMailNode(mlist, mail);
      MA_GetAddress(mlist);
      DeleteMailList(mlist);
    }
  }

  RETURN(0);
  return 0;
}

///
/// DECLARE(SetStatusTo)
//  Sets the status of the current mail to the define value
DECLARE(SetStatusTo) // int addflags, int clearflags
{
  GETDATA;
  struct ReadMailData *rmData = (struct ReadMailData *)xget(data->readMailGroup, MUIA_ReadMailGroup_ReadMailData);
  struct Mail *mail = rmData->mail;

  ENTER();

  MA_ChangeMailStatus(mail, msg->addflags, msg->clearflags);

  DoMethod(data->statusBar, MUIM_ReadWindowStatusBar_Update, mail);
  DisplayStatistics(NULL, TRUE);

  RETURN(0);
  return 0;
}
///
/// DECLARE(ChangeSubjectRequest)
//  Changes the subject of the current message
DECLARE(ChangeSubjectRequest)
{
  GETDATA;
  struct ReadMailData *rmData = (struct ReadMailData *)xget(data->readMailGroup, MUIA_ReadMailGroup_ReadMailData);
  struct Mail *mail = rmData->mail;
  struct Folder *folder = mail->Folder;

  ENTER();

  if(MailExists(mail, folder))
  {
    char subj[SIZE_SUBJECT];

    strlcpy(subj, mail->Subject, sizeof(subj));

    if(StringRequest(subj, SIZE_SUBJECT,
                     tr(MSG_MA_ChangeSubj),
                     tr(MSG_MA_ChangeSubjReq),
                     tr(MSG_Okay), NULL, tr(MSG_Cancel), FALSE, obj))
    {
      MA_ChangeSubject(mail, subj);

      if(DoMethod(G->MA->GUI.PG_MAILLIST, MUIM_MainMailListGroup_RedrawMail, mail))
      {
        MA_ChangeSelected(TRUE);
        DisplayStatistics(mail->Folder, TRUE);
      }

      // update this window
      DoMethod(obj, MUIM_ReadWindow_ReadMail, mail);
    }
  }

  RETURN(0);
  return 0;
}

///
/// DECLARE(SwitchMail)
//  Goes to next or previous (new) message in list
DECLARE(SwitchMail) // LONG direction, ULONG qualifier
{
  GETDATA;
  struct ReadMailData *rmData;
  struct Mail *mail;
  struct Folder *folder;
  LONG direction;
  LONG act = MUIV_NList_GetPos_Start;
  BOOL onlynew;
  BOOL found = FALSE;

  ENTER();

  rmData = (struct ReadMailData *)xget(data->readMailGroup, MUIA_ReadMailGroup_ReadMailData);
  mail = rmData->mail;
  folder = mail->Folder;
  direction = msg->direction;
  onlynew = isAnyFlagSet(msg->qualifier, (IEQUALIFIER_LSHIFT|IEQUALIFIER_RSHIFT));

  // save the direction we are going to process now
  data->lastDirection = direction;

  // we have to make sure that the folder the next/prev mail will
  // be showed from is active, that`s why we call ChangeFolder with TRUE.
  MA_ChangeFolder(folder, TRUE);

  // after changing the folder we have to get the MailInfo (Position etc.)
  DoMethod(G->MA->GUI.PG_MAILLIST, MUIM_NList_GetPos, mail, &act);

  D(DBF_GUI, "act: %ld - direction: %ld", act, direction);

  if(act != MUIV_NList_GetPos_End)
  {
    for(act += direction; act >= 0; act += direction)
    {
      DoMethod(G->MA->GUI.PG_MAILLIST, MUIM_NList_GetEntry, act, &mail);
      if(mail == NULL)
        break;

      if(onlynew == FALSE ||
        (hasStatusNew(mail) || !hasStatusRead(mail)))
      {
         set(G->MA->GUI.PG_MAILLIST, MUIA_NList_Active, act);
         DoMethod(obj, MUIM_ReadWindow_ReadMail, mail);

         // this is a valid break and not break because of an error
         found = TRUE;
         break;
      }
    }
  }

  // check if there are following/previous folders with unread
  // mails and change to there if the user wants
  if(found == FALSE && onlynew == TRUE)
  {
    if(C->AskJumpUnread == TRUE)
    {
      struct FolderNode *fnode;
      BOOL abortJump;
      BOOL turnOver;

      LockFolderListShared(G->folders);

      // look for the current folder in the array
      ForEachFolderNode(G->folders, fnode)
      {
        if(fnode->folder == folder)
          break;
      }

      abortJump = FALSE;
      turnOver = FALSE;
      while(found == FALSE && abortJump == FALSE)
      {
        struct FolderNode *fnode2;

        // look for first folder with at least one unread mail
        // and if found read that mail
        do
        {
          // either go forward or backward
          if(direction > 0)
            fnode2 = (fnode == NULL) ? FirstFolderNode(G->folders) : NextFolderNode(fnode);
          else
            fnode2 = (fnode == NULL) ? LastFolderNode(G->folders) : PreviousFolderNode(fnode);

          if(fnode2 != NULL)
          {
            struct Folder *fo = fnode2->folder;

            // skip group folders, outgoing, trash and spam folder when looking for still unread mail
            if(!isGroupFolder(fo) &&
               !isOutgoingFolder(fo) &&
               !isTrashFolder(fo) &&
               !isSpamFolder(fo) &&
               fo->Unread > 0)
            {
              if(fo != folder)
              {
                if(MUI_Request(G->App, obj, MUIF_NONE, tr(MSG_MA_ConfirmReq),
                                                       tr(MSG_YesNoReq),
                                                       tr(MSG_RE_MoveNextFolderReq), fo->Name) == 0)
                {
                  abortJump = TRUE;
                  break;
                }

                MA_ChangeFolder(fo, TRUE);
              }
              else
              {
                MA_JumpToNewMsg();
              }

              DoMethod(G->MA->GUI.PG_MAILLIST, MUIM_NList_GetEntry, MUIV_NList_GetEntry_Active, &mail);
              if(mail == NULL)
                break;

              DoMethod(obj, MUIM_ReadWindow_ReadMail, mail);

              // this is a valid break and not break because of an error
              found = TRUE;
              break;
            }

            fnode = fnode2;
          }
        }
        while(fnode2 != NULL);

        // beep if no folder with unread mails was found
        if(found == FALSE && fnode2 == NULL)
        {
          if(turnOver == TRUE)
          {
            // we already run through the complete list and found nothing
            DisplayBeep(_screen(obj));
            // get out of this loop
            break;
          }
          else
          {
            // we just reached the end of the folder list for the first time,
            // so let's try it again from the opposite side
            turnOver = TRUE;
            fnode = NULL;
          }
        }
      }

      UnlockFolderList(G->folders);
    }
    else
      DisplayBeep(_screen(obj));
  }

  // if we didn't find any next/previous mail (mail == NULL) then
  // we can close the window accordingly. This signals a user that he/she
  // reached the end of the mail list
  if(found == FALSE)
    DoMethod(G->App, MUIM_Application_PushMethod, G->App, 3, MUIM_CallHook, &CloseReadWindowHook, rmData);

  RETURN(0);
  return 0;
}

///
/// DECLARE(FollowThread)
//  Follows a thread in either direction
DECLARE(FollowThread) // LONG direction
{
  GETDATA;
  struct ReadMailData *rmData;
  struct Mail *mail;
  struct Mail *fmail;

  ENTER();

  rmData = (struct ReadMailData *)xget(data->readMailGroup, MUIA_ReadMailGroup_ReadMailData);
  mail = rmData->mail;

  // depending on the direction we get the Question or Answer to the current Message
  if((fmail = FindThread(mail, msg->direction > 0)) != NULL)
  {
    LONG pos = MUIV_NList_GetPos_Start;

    // we have to make sure that the folder where the message will be showed
    // from is active and ready to display the mail
    MA_ChangeFolder(fmail->Folder, TRUE);

    // get the position of the mail in the currently active listview
    DoMethod(G->MA->GUI.PG_MAILLIST, MUIM_NList_GetPos, fmail, &pos);

    // if the mail is displayed we make it the active one
    if(pos != MUIV_NList_GetPos_End)
      set(G->MA->GUI.PG_MAILLIST, MUIA_NList_Active, pos);

    DoMethod(obj, MUIM_ReadWindow_ReadMail, fmail);
  }
  else
  {
    // set the correct toolbar image and menuitem ghosted
    if(msg->direction <= 0)
    {
      DoMethod(data->windowToolbar, MUIM_TheBar_SetAttr, TB_READ_PREVTHREAD, MUIA_TheBar_Attr_Disabled, TRUE);
      set(data->MI_PREVTHREAD, MUIA_Menuitem_Enabled, FALSE);
    }
    else
    {
      DoMethod(data->windowToolbar, MUIM_TheBar_SetAttr, TB_READ_NEXTTHREAD, MUIA_TheBar_Attr_Disabled, TRUE);
      set(data->MI_NEXTTHREAD, MUIA_Menuitem_Enabled, FALSE);
    }

    DisplayBeep(_screen(obj));
  }

  RETURN(0);
  return 0;
}

///
/// DECLARE(ChangeHeaderMode)
//  Changes display options (header)
DECLARE(ChangeHeaderMode) // enum HeaderMode hmode
{
  GETDATA;

  ENTER();

  // forward this method to the readMailGroup, it will update itself if necessary
  DoMethod(data->readMailGroup, MUIM_ReadMailGroup_ChangeHeaderMode, msg->hmode);

  RETURN(0);
  return 0;
}

///
/// DECLARE(ChangeSenderInfoMode)
//  Changes display options (sender info)
DECLARE(ChangeSenderInfoMode) // enum SInfoMode simode
{
  GETDATA;

  ENTER();

  // forward this method to the readMailGroup, it will update itself if necessary
  DoMethod(data->readMailGroup, MUIM_ReadMailGroup_ChangeSenderInfoMode, msg->simode);

  RETURN(0);
  return 0;
}

///
/// DECLARE(StyleOptionsChanged)
DECLARE(StyleOptionsChanged)
{
  GETDATA;
  struct ReadMailData *rmData = (struct ReadMailData *)xget(data->readMailGroup, MUIA_ReadMailGroup_ReadMailData);
  BOOL updateHeader = FALSE;
  BOOL updateText = FALSE;
  BOOL tmp;

  ENTER();

  // check wrapHeaders diff
  if((tmp = xget(data->MI_WRAPH, MUIA_Menuitem_Checked)) != rmData->wrapHeaders)
  {
    rmData->wrapHeaders = tmp;
    updateHeader = TRUE;
  }

  // check useTextcolors diff
  if((tmp = xget(data->MI_TCOLOR, MUIA_Menuitem_Checked)) != rmData->useTextcolors)
  {
    rmData->useTextcolors = tmp;
    updateText = TRUE;
  }

  // check useTextstyles diff
  if((tmp = xget(data->MI_TSTYLE, MUIA_Menuitem_Checked)) != rmData->useTextstyles)
  {
    rmData->useTextstyles = tmp;
    updateText = TRUE;
  }

  // check useFixedFont diff
  if((tmp = xget(data->MI_FFONT, MUIA_Menuitem_Checked)) != rmData->useFixedFont)
  {
    rmData->useFixedFont = tmp;

    // update the font settings of TE_EDIT
    set(data->readMailGroup, MUIA_TextEditor_FixedFont, rmData->useFixedFont);
  }

  // issue an update of the readMailGroup's components
  if(updateHeader && updateText)
  {
    DoMethod(data->readMailGroup, MUIM_ReadMailGroup_ReadMail, rmData->mail,
                                  MUIF_ReadMailGroup_ReadMail_UpdateOnly);
  }
  else if(updateText)
  {
    DoMethod(data->readMailGroup, MUIM_ReadMailGroup_ReadMail, rmData->mail,
                                  (MUIF_ReadMailGroup_ReadMail_UpdateOnly |
                                   MUIF_ReadMailGroup_ReadMail_UpdateTextOnly));
  }
  else if(updateHeader)
  {
    DoMethod(data->readMailGroup, MUIM_ReadMailGroup_UpdateHeaderDisplay,
                                  MUIF_ReadMailGroup_ReadMail_UpdateOnly);
  }

  RETURN(0);
  return 0;
}
///
/// DECLARE(UpdateStatusBar)
DECLARE(UpdateStatusBar)
{
  GETDATA;
  struct ReadMailData *rmData = (struct ReadMailData *)xget(data->readMailGroup, MUIA_ReadMailGroup_ReadMailData);

  ENTER();

  // Update the status bar
  if(rmData->mail != NULL)
    DoMethod(data->statusBar, MUIM_ReadWindowStatusBar_Update, rmData->mail);

  RETURN(0);
  return 0;
}
///
/// DECLARE(UpdateSpamControls)
DECLARE(UpdateSpamControls)
{
  GETDATA;
  struct ReadMailData *rmData = (struct ReadMailData *)xget(data->readMailGroup, MUIA_ReadMailGroup_ReadMailData);
  struct Mail *mail = rmData->mail;

  ENTER();

  if(C->SpamFilterEnabled)
  {
    BOOL isSpamMail = (mail != NULL) && !isVirtualMail(mail) && hasStatusSpam(mail);

    // for each entry check if it exists and if it is part of the menu
    // if not, create a new entry and add it to the current layout
    if(data->MI_TOHAM == NULL || isChildOfFamily(data->MI_STATUS, data->MI_TOHAM) == FALSE)
    {
      if((data->MI_TOHAM = Menuitem(tr(MSG_MA_TONOTSPAM), NULL, TRUE, FALSE, RMEN_TOHAM)) != NULL)
        DoMethod(data->MI_STATUS, MUIM_Family_Insert, data->MI_TOHAM, data->MI_TOUNREAD);
    }
    if(data->MI_TOHAM != NULL)
      set(data->MI_TOHAM, MUIA_Menuitem_Enabled, isSpamMail);

    if(data->MI_TOSPAM == NULL || isChildOfFamily(data->MI_STATUS, data->MI_TOSPAM) == FALSE)
    {
      if((data->MI_TOSPAM = Menuitem(tr(MSG_MA_TOSPAM), NULL, TRUE, FALSE, RMEN_TOSPAM)) != NULL)
        DoMethod(data->MI_STATUS, MUIM_Family_Insert, data->MI_TOSPAM, data->MI_TOUNREAD);
    }
    if(data->MI_TOSPAM != NULL)
      set(data->MI_TOSPAM, MUIA_Menuitem_Enabled, !isSpamMail);
  }
  else
  {
    // for each entry check if it exists and if it is part of the menu
    // if yes, then remove the entry and dispose it
    if(data->MI_TOSPAM != NULL && isChildOfFamily(data->MI_STATUS, data->MI_TOSPAM))
    {
      DoMethod(data->MI_STATUS, MUIM_Family_Remove, data->MI_TOSPAM);
      MUI_DisposeObject(data->MI_TOSPAM);
      data->MI_TOSPAM = NULL;
    }
    if(data->MI_TOHAM != NULL && isChildOfFamily(data->MI_STATUS, data->MI_TOHAM))
    {
      DoMethod(data->MI_STATUS, MUIM_Family_Remove, data->MI_TOHAM);
      MUI_DisposeObject(data->MI_TOHAM);
      data->MI_TOHAM = NULL;
    }
  }

  // update the toolbar as well, so lets delegate
  // the method call to it.
  if(data->windowToolbar != NULL)
    DoMethod(data->windowToolbar, MUIM_ReadWindowToolbar_UpdateSpamControls, mail);

  RETURN(0);
  return 0;
}

///
/// DECLARE(UpdateMenuShortcuts)
DECLARE(UpdateMenuShortcuts)
{
  GETDATA;

  ENTER();

  // update the "Forward" items' shortcut
  if(C->ForwardMode == FWM_ATTACH)
  {
    set(data->MI_FORWARD_ATTACH, MUIA_Menuitem_Shortcut, "W");
    set(data->MI_FORWARD_INLINE, MUIA_Menuitem_Shortcut, NULL);
  }
  else if(C->ForwardMode == FWM_INLINE)
  {
    set(data->MI_FORWARD_ATTACH, MUIA_Menuitem_Shortcut, NULL);
    set(data->MI_FORWARD_INLINE, MUIA_Menuitem_Shortcut, "W");
  }

  RETURN(0);
  return 0;
}

///
