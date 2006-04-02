/***************************************************************************

 YAM - Yet Another Mailer
 Copyright (C) 1995-2000 by Marcel Beck <mbeck@yam.ch>
 Copyright (C) 2000-2006 by YAM Open Source Team

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

#include "Debug.h"

/* EXPORT
#define MUIV_ReadWindow_ToolbarItems 14
*/

/* CLASSDATA
struct Data
{
  Object *MI_EDIT;
  Object *MI_MOVE;
  Object *MI_DELETE;
  Object *MI_DETACH;
  Object *MI_CROP;
  Object *MI_CHSUBJ;
  Object *MI_NAVIG;
  Object *MI_WRAPH;
  Object *MI_TSTYLE;
  Object *MI_FFONT;
  Object *MI_EXTKEY;
  Object *MI_CHKSIG;
  Object *MI_SAVEDEC;
  Object *windowToolbar;
  Object *statusIconGroup;
  Object *readMailGroup;
  struct MUIP_Toolbar_Description toolbarDesc[MUIV_ReadWindow_ToolbarItems];

  char  title[SIZE_DEFAULT+1];
  int   lastDirection;
  int   windowNumber;
};
*/

/* Private Functions */
/// SelectMessage()
//  Activates a message in the main window's message listview
static inline LONG SelectMessage(struct Mail *mail)
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
/// ClosedReadWindowHook()
//  Hook that will be called as soon as a read window is closed
HOOKPROTONHNO(ClosedReadWindowFunc, void, struct ReadMailData **arg)
{
  struct ReadMailData *rmData = *arg;

  ENTER();

  // only if this is not a close operation because the application
  // is getting iconified we really cleanup our readmail data
  if(rmData == G->ActiveRexxRMData ||
     xget(G->App, MUIA_Application_Iconified) == FALSE)
  {
    // check if this rmData is the current active Rexx background
    // processing one and if so set the ptr to NULL to signal the rexx
    // commands that their active window was closed
    if(rmData == G->ActiveRexxRMData)
      G->ActiveRexxRMData = NULL;

    // calls the CleanupReadMailData to clean everything else up
    CleanupReadMailData(rmData, TRUE);
  }

  LEAVE();
}
MakeStaticHook(ClosedReadWindowHook, ClosedReadWindowFunc);
///

/* Overloaded Methods */
/// OVERLOAD(OM_NEW)
OVERLOAD(OM_NEW)
{
  ULONG i;
  struct Data *data;
  struct Data *tmpData;

  // menu item IDs
  enum
  {
    RMEN_EDIT=501,RMEN_MOVE,RMEN_COPY,RMEN_DELETE,RMEN_PRINT,RMEN_SAVE,RMEN_DISPLAY,RMEN_DETACH,
    RMEN_CROP,RMEN_NEW,RMEN_REPLY,RMEN_FORWARD,RMEN_BOUNCE,RMEN_SAVEADDR,RMEN_SETUNREAD,RMEN_SETMARKED,
    RMEN_CHSUBJ,RMEN_PREV,RMEN_NEXT,RMEN_URPREV,RMEN_URNEXT,RMEN_PREVTH,RMEN_NEXTTH,
    RMEN_EXTKEY,RMEN_CHKSIG,RMEN_SAVEDEC,
    RMEN_HNONE,RMEN_HSHORT,RMEN_HFULL,RMEN_SNONE,RMEN_SDATA,RMEN_SFULL,RMEN_WRAPH,RMEN_TSTYLE,RMEN_FFONT,
    RMEN_SIMAGE
  };

  // Our static Toolbar description field
  static const struct NewToolbarEntry tb_butt[MUIV_ReadWindow_ToolbarItems] =
  {
    {  MSG_RE_TBPrev,    MSG_HELP_RE_BT_PREVIOUS },
    { MSG_RE_TBNext,    MSG_HELP_RE_BT_NEXT     },
    { MSG_RE_TBPrevTh,  MSG_HELP_RE_BT_QUESTION },
    { MSG_RE_TBNextTh,  MSG_HELP_RE_BT_ANSWER   },
    { MSG_Space,        NULL                    },
    { MSG_RE_TBDisplay, MSG_HELP_RE_BT_DISPLAY  },
    { MSG_RE_TBSave,    MSG_HELP_RE_BT_EXPORT   },
    { MSG_RE_TBPrint,   MSG_HELP_RE_BT_PRINT    },
    { MSG_Space,        NULL                    },
    { MSG_RE_TBDelete,  MSG_HELP_RE_BT_DELETE   },
    { MSG_RE_TBMove,    MSG_HELP_RE_BT_MOVE     },
    { MSG_RE_TBReply,   MSG_HELP_RE_BT_REPLY    },
    { MSG_RE_TBForward, MSG_HELP_RE_BT_FORWARD  },
    { NULL,             NULL                    }
  };

  // generate a temporarly struct Data to which we store our data and
  // copy it later on
  if(!(data = tmpData = calloc(1, sizeof(struct Data))))
    return 0;

  for(i=0; i < MUIV_ReadWindow_ToolbarItems; i++)
  {
    SetupToolbar(&(data->toolbarDesc[i]),
                 tb_butt[i].label ?
                    (tb_butt[i].label == MSG_Space ? "" : GetStr(tb_butt[i].label))
                    : NULL,
                 tb_butt[i].help ? GetStr(tb_butt[i].help) : NULL,
                 0);
  }

  // before we create all objects of this new read window we have to
  // check which number we can set for this window. Therefore we search in our
  // current ReadMailData list and check which number we can give this window
  i=0;
  do
  {
    struct MinNode *curNode = G->readMailDataList.mlh_Head;
    
    for(; curNode->mln_Succ; curNode = curNode->mln_Succ)
    {
      struct ReadMailData *rmData = (struct ReadMailData *)curNode;
      
      if(rmData->readWindow &&
         xget(rmData->readWindow, MUIA_ReadWindow_Num) == i)
      {
        break;
      }
    }

    // if the curNode successor is NULL we traversed through the whole
    // list without finding the proposed ID, so we can choose it as
    // our readWindow ID
    if(curNode->mln_Succ == NULL)
    {
      D(DBF_GUI, "Free window number %d found.", i);
      data->windowNumber = i;

      break;
    }

    i++;
  }
  while(1);

  if((obj = DoSuperNew(cl, obj,

    MUIA_Window_Title,   "",
    MUIA_HelpNode,       "RE_W",
    MUIA_Window_ID,     MAKE_ID('R','D','W',data->windowNumber),
    MUIA_Window_Menustrip, MenustripObject,
      MenuChild, MenuObject, MUIA_Menu_Title, GetStr(MSG_Message),
        MenuChild, data->MI_EDIT = Menuitem(GetStr(MSG_MA_MEdit), "E", TRUE, FALSE, RMEN_EDIT),
        MenuChild, data->MI_MOVE = Menuitem(GetStr(MSG_MA_MMove), "M", TRUE, FALSE, RMEN_MOVE),
        MenuChild, Menuitem(GetStr(MSG_MA_MCopy), "Y", TRUE, FALSE, RMEN_COPY),
        MenuChild, data->MI_DELETE = Menuitem(GetStr(MSG_MA_MDelete),  "Del", TRUE, TRUE,  RMEN_DELETE),
        MenuChild, MenuBarLabel,
        MenuChild, Menuitem(GetStr(MSG_Print),       "P",     TRUE, FALSE, RMEN_PRINT),
        MenuChild, Menuitem(GetStr(MSG_MA_Save),    "S",     TRUE, FALSE, RMEN_SAVE),
        MenuChild, MenuitemObject, MUIA_Menuitem_Title, GetStr(MSG_Attachments),
          MenuChild, Menuitem(GetStr(MSG_RE_MDisplay),"D",  TRUE,  FALSE, RMEN_DISPLAY),
          MenuChild, data->MI_DETACH = Menuitem(GetStr(MSG_RE_SaveAll),  "A",  TRUE, FALSE, RMEN_DETACH),
          MenuChild, data->MI_CROP =    Menuitem(GetStr(MSG_MA_Crop),    "O",  TRUE, FALSE, RMEN_CROP),
        End,
        MenuChild, MenuBarLabel,
        MenuChild, Menuitem(GetStr(MSG_New),         "N", TRUE, FALSE, RMEN_NEW),
        MenuChild, Menuitem(GetStr(MSG_MA_MReply),   "R", TRUE, FALSE, RMEN_REPLY),
        MenuChild, Menuitem(GetStr(MSG_MA_MForward), "W", TRUE, FALSE, RMEN_FORWARD),
        MenuChild, Menuitem(GetStr(MSG_MA_MBounce),   "B", TRUE, FALSE, RMEN_BOUNCE),
        MenuChild, MenuBarLabel,
        MenuChild, Menuitem(GetStr(MSG_MA_MGetAddress), "J", TRUE, FALSE, RMEN_SAVEADDR),
        MenuChild, Menuitem(GetStr(MSG_RE_SetUnread),   "U", TRUE, FALSE, RMEN_SETUNREAD),
        MenuChild, Menuitem(GetStr(MSG_RE_SETMARKED),    ",", TRUE, FALSE, RMEN_SETMARKED),
        MenuChild, data->MI_CHSUBJ = Menuitem(GetStr(MSG_MA_ChangeSubj), NULL, TRUE, FALSE, RMEN_CHSUBJ),
      End,
      MenuChild, data->MI_NAVIG = MenuObject, MUIA_Menu_Title, GetStr(MSG_RE_Navigation),
        MenuChild, Menuitem(GetStr(MSG_RE_MNext),    "right", TRUE, TRUE, RMEN_NEXT),
        MenuChild, Menuitem(GetStr(MSG_RE_MPrev),     "left",  TRUE, TRUE, RMEN_PREV),
        MenuChild, Menuitem(GetStr(MSG_RE_MURNext),  "shift right", TRUE, TRUE, RMEN_URNEXT),
        MenuChild, Menuitem(GetStr(MSG_RE_MURPrev),  "shift left",  TRUE, TRUE, RMEN_URPREV),
        MenuChild, Menuitem(GetStr(MSG_RE_MNextTh),   ">", TRUE, FALSE, RMEN_NEXTTH),
        MenuChild, Menuitem(GetStr(MSG_RE_MPrevTh),  "<", TRUE, FALSE, RMEN_PREVTH),
      End,
      MenuChild, MenuObject, MUIA_Menu_Title, "PGP",
        MenuChild, data->MI_EXTKEY = Menuitem(GetStr(MSG_RE_ExtractKey), "X", TRUE, FALSE, RMEN_EXTKEY),
        MenuChild, data->MI_CHKSIG = Menuitem(GetStr(MSG_RE_SigCheck), "K", TRUE, FALSE, RMEN_CHKSIG),
        MenuChild, data->MI_SAVEDEC = Menuitem(GetStr(MSG_RE_SaveDecrypted), "V", TRUE, FALSE, RMEN_SAVEDEC),
      End,
      MenuChild, MenuObject, MUIA_Menu_Title, GetStr(MSG_MA_Settings),
        MenuChild, MenuitemCheck(GetStr(MSG_RE_NoHeaders),     "0", C->ShowHeader==HM_NOHEADER,     FALSE, 0x06, RMEN_HNONE),
        MenuChild, MenuitemCheck(GetStr(MSG_RE_ShortHeaders), "1", C->ShowHeader==HM_SHORTHEADER, FALSE, 0x05, RMEN_HSHORT),
        MenuChild, MenuitemCheck(GetStr(MSG_RE_FullHeaders),  "2", C->ShowHeader==HM_FULLHEADER,  FALSE, 0x03, RMEN_HFULL),
        MenuChild, MenuBarLabel,
        MenuChild, MenuitemCheck(GetStr(MSG_RE_NoSInfo),     "3", C->ShowSenderInfo==SIM_OFF,    FALSE, 0xE0, RMEN_SNONE),
        MenuChild, MenuitemCheck(GetStr(MSG_RE_SInfo),        "4", C->ShowSenderInfo==SIM_DATA,   FALSE, 0xD0, RMEN_SDATA),
        MenuChild, MenuitemCheck(GetStr(MSG_RE_SInfoImage), "5", C->ShowSenderInfo==SIM_ALL,    FALSE, 0x90, RMEN_SFULL),
        MenuChild, MenuitemCheck(GetStr(MSG_RE_SImageOnly),  "6", C->ShowSenderInfo==SIM_IMAGE, FALSE, 0x70, RMEN_SIMAGE),
        MenuChild, MenuBarLabel,
        MenuChild, data->MI_WRAPH = MenuitemCheck(GetStr(MSG_RE_WrapHeader), "H", C->WrapHeader, TRUE, 0, RMEN_WRAPH),
        MenuChild, data->MI_TSTYLE = MenuitemCheck(GetStr(MSG_RE_Textstyles), "T", C->UseTextstyles, TRUE, 0, RMEN_TSTYLE),
        MenuChild, data->MI_FFONT = MenuitemCheck(GetStr(MSG_RE_FixedFont), "F", C->FixedFontEdit, TRUE, 0, RMEN_FFONT),
      End,
    End,
    WindowContents, VGroup,
      Child, hasHideToolBarFlag(C->HideGUIElements) ?
        (RectangleObject, MUIA_ShowMe, FALSE, End) :
        (HGroup, GroupSpacing(2),
          Child, HGroupV,
            Child, data->windowToolbar = ToolbarObject,
              MUIA_HelpNode, "RE_B",
              MUIA_Toolbar_ImageType,        MUIV_Toolbar_ImageType_File,
              MUIA_Toolbar_ImageNormal,      "PROGDIR:Icons/Read.toolbar",
              MUIA_Toolbar_ImageGhost,       "PROGDIR:Icons/Read_G.toolbar",
              MUIA_Toolbar_ImageSelect,      "PROGDIR:Icons/Read_S.toolbar",
              MUIA_Toolbar_Description,      &data->toolbarDesc[0],
              MUIA_Toolbar_Reusable,        TRUE,
              MUIA_Toolbar_ParseUnderscore,  TRUE,
              MUIA_Font,                     MUIV_Font_Tiny,
              MUIA_ShortHelp,               TRUE,
            End,
            Child, HSpace(0),
          End,
          Child, RectangleObject,
            MUIA_Rectangle_VBar, TRUE,
            MUIA_FixWidth,        3,
          End,
          Child, data->statusIconGroup = StatusIconGroupObject,
          End,
        End),
        Child, VGroup,
          Child, data->readMailGroup = ReadMailGroupObject,
            MUIA_ReadMailGroup_HGVertWeight, G->Weights[10],
            MUIA_ReadMailGroup_TGVertWeight, G->Weights[11],
          End,
        End,
      End,

    TAG_MORE, (ULONG)inittags(msg))))
  {
    struct ReadMailData *rmData = (struct ReadMailData *)xget(data->readMailGroup, MUIA_ReadMailGroup_ReadMailData);

    if(rmData == NULL ||
       (data = (struct Data *)INST_DATA(cl,obj)) == NULL)
    {
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

    // set some Notifies and stuff
    set(obj, MUIA_Window_DefaultObject, data->readMailGroup);
    DoMethod(obj, MUIM_Notify, MUIA_Window_MenuAction, RMEN_EDIT,     obj, 3, MUIM_ReadWindow_NewMail, NEW_EDIT, 0);
    DoMethod(obj, MUIM_Notify, MUIA_Window_MenuAction, RMEN_MOVE,     obj, 1, MUIM_ReadWindow_MoveMailRequest);
    DoMethod(obj, MUIM_Notify, MUIA_Window_MenuAction, RMEN_COPY,     obj, 1, MUIM_ReadWindow_CopyMailRequest);
    DoMethod(obj, MUIM_Notify, MUIA_Window_MenuAction, RMEN_DELETE,    obj, 2, MUIM_ReadWindow_DeleteMailRequest, 0);
    DoMethod(obj, MUIM_Notify, MUIA_Window_MenuAction, RMEN_PRINT,    obj, 1, MUIM_ReadWindow_PrintMailRequest);
    DoMethod(obj, MUIM_Notify, MUIA_Window_MenuAction, RMEN_SAVE,      obj, 1, MUIM_ReadWindow_SaveMailRequest);
    DoMethod(obj, MUIM_Notify, MUIA_Window_MenuAction, RMEN_DISPLAY,  obj, 1, MUIM_ReadWindow_DisplayMailRequest);
    DoMethod(obj,  MUIM_Notify, MUIA_Window_MenuAction, RMEN_DETACH,    data->readMailGroup, 1, MUIM_ReadMailGroup_SaveAllAttachments);
    DoMethod(obj, MUIM_Notify, MUIA_Window_MenuAction, RMEN_CROP,      obj, 1, MUIM_ReadWindow_CropAttachmentsRequest);
    DoMethod(obj, MUIM_Notify, MUIA_Window_MenuAction, RMEN_NEW,      obj, 3,  MUIM_ReadWindow_NewMail, NEW_NEW, 0);
    DoMethod(obj, MUIM_Notify, MUIA_Window_MenuAction, RMEN_REPLY,    obj, 3, MUIM_ReadWindow_NewMail, NEW_REPLY, 0);
    DoMethod(obj, MUIM_Notify, MUIA_Window_MenuAction, RMEN_FORWARD,  obj, 3, MUIM_ReadWindow_NewMail, NEW_FORWARD, 0);
    DoMethod(obj, MUIM_Notify, MUIA_Window_MenuAction, RMEN_BOUNCE,   obj, 3, MUIM_ReadWindow_NewMail, NEW_BOUNCE, 0);
    DoMethod(obj, MUIM_Notify, MUIA_Window_MenuAction, RMEN_SAVEADDR,  obj, 1, MUIM_ReadWindow_GrabSenderAddress);
    DoMethod(obj,  MUIM_Notify, MUIA_Window_MenuAction, RMEN_SETUNREAD,obj, 1, MUIM_ReadWindow_SetMailToUnread);
    DoMethod(obj, MUIM_Notify, MUIA_Window_MenuAction, RMEN_SETMARKED,obj, 1, MUIM_ReadWindow_SetMailToMarked);
    DoMethod(obj, MUIM_Notify, MUIA_Window_MenuAction, RMEN_CHSUBJ,    obj, 1, MUIM_ReadWindow_ChangeSubjectRequest);
    DoMethod(obj, MUIM_Notify, MUIA_Window_MenuAction, RMEN_PREV,      obj, 3, MUIM_ReadWindow_SwitchMail, -1, 0);
    DoMethod(obj, MUIM_Notify, MUIA_Window_MenuAction, RMEN_NEXT,      obj, 3, MUIM_ReadWindow_SwitchMail, +1, 0);
    DoMethod(obj, MUIM_Notify, MUIA_Window_MenuAction, RMEN_URPREV,    obj, 3, MUIM_ReadWindow_SwitchMail, -1, IEQUALIFIER_LSHIFT);
    DoMethod(obj, MUIM_Notify, MUIA_Window_MenuAction, RMEN_URNEXT,    obj, 3,  MUIM_ReadWindow_SwitchMail, +1, IEQUALIFIER_LSHIFT);
    DoMethod(obj, MUIM_Notify, MUIA_Window_MenuAction, RMEN_PREVTH,    obj, 2, MUIM_ReadWindow_FollowThread, -1);
    DoMethod(obj, MUIM_Notify, MUIA_Window_MenuAction, RMEN_NEXTTH,    obj, 2, MUIM_ReadWindow_FollowThread, +1);
    DoMethod(obj, MUIM_Notify, MUIA_Window_MenuAction, RMEN_EXTKEY,    data->readMailGroup, 1, MUIM_ReadMailGroup_ExtractPGPKey);
    DoMethod(obj, MUIM_Notify, MUIA_Window_MenuAction, RMEN_CHKSIG,    data->readMailGroup, 2, MUIM_ReadMailGroup_CheckPGPSignature, TRUE);
    DoMethod(obj, MUIM_Notify, MUIA_Window_MenuAction, RMEN_SAVEDEC,  data->readMailGroup, 1, MUIM_ReadMailGroup_SaveDecryptedMail);
    DoMethod(obj, MUIM_Notify, MUIA_Window_MenuAction, RMEN_HNONE,    obj, 2, MUIM_ReadWindow_ChangeHeaderMode, HM_NOHEADER);
    DoMethod(obj, MUIM_Notify, MUIA_Window_MenuAction, RMEN_HSHORT,    obj, 2, MUIM_ReadWindow_ChangeHeaderMode, HM_SHORTHEADER);
    DoMethod(obj, MUIM_Notify, MUIA_Window_MenuAction, RMEN_HFULL,    obj, 2, MUIM_ReadWindow_ChangeHeaderMode, HM_FULLHEADER);
    DoMethod(obj, MUIM_Notify, MUIA_Window_MenuAction, RMEN_SNONE,    obj, 2, MUIM_ReadWindow_ChangeSenderInfoMode, SIM_OFF);
    DoMethod(obj, MUIM_Notify, MUIA_Window_MenuAction, RMEN_SDATA,    obj, 2, MUIM_ReadWindow_ChangeSenderInfoMode, SIM_DATA);
    DoMethod(obj, MUIM_Notify, MUIA_Window_MenuAction, RMEN_SFULL,    obj, 2, MUIM_ReadWindow_ChangeSenderInfoMode, SIM_ALL);
    DoMethod(obj, MUIM_Notify, MUIA_Window_MenuAction, RMEN_SIMAGE,    obj, 2, MUIM_ReadWindow_ChangeSenderInfoMode, SIM_IMAGE);
    DoMethod(obj,  MUIM_Notify, MUIA_Window_MenuAction, RMEN_WRAPH,    obj, 1, MUIM_ReadWindow_StyleOptionsChanged);
    DoMethod(obj, MUIM_Notify, MUIA_Window_MenuAction, RMEN_TSTYLE,    obj, 1, MUIM_ReadWindow_StyleOptionsChanged);
    DoMethod(obj, MUIM_Notify, MUIA_Window_MenuAction, RMEN_FFONT,    obj, 1, MUIM_ReadWindow_StyleOptionsChanged);
    DoMethod(data->windowToolbar, MUIM_Toolbar_Notify, 0, MUIV_Toolbar_Notify_Pressed, FALSE, obj, 3, MUIM_ReadWindow_SwitchMail, -1, MUIV_Toolbar_Qualifier);
    DoMethod(data->windowToolbar, MUIM_Toolbar_Notify, 1, MUIV_Toolbar_Notify_Pressed, FALSE, obj, 3, MUIM_ReadWindow_SwitchMail, +1, MUIV_Toolbar_Qualifier);
    DoMethod(data->windowToolbar, MUIM_Toolbar_Notify, 2, MUIV_Toolbar_Notify_Pressed, FALSE, obj, 2, MUIM_ReadWindow_FollowThread, -1);
    DoMethod(data->windowToolbar, MUIM_Toolbar_Notify, 3, MUIV_Toolbar_Notify_Pressed, FALSE, obj, 2, MUIM_ReadWindow_FollowThread, +1);
    DoMethod(data->windowToolbar, MUIM_Toolbar_Notify, 5, MUIV_Toolbar_Notify_Pressed, FALSE, obj, 1, MUIM_ReadWindow_DisplayMailRequest);
    DoMethod(data->windowToolbar, MUIM_Toolbar_Notify, 6, MUIV_Toolbar_Notify_Pressed, FALSE, obj, 1, MUIM_ReadWindow_SaveMailRequest);
    DoMethod(data->windowToolbar, MUIM_Toolbar_Notify, 7, MUIV_Toolbar_Notify_Pressed, FALSE, obj, 1, MUIM_ReadWindow_PrintMailRequest);
    DoMethod(data->windowToolbar, MUIM_Toolbar_Notify, 9, MUIV_Toolbar_Notify_Pressed, FALSE, obj, 2, MUIM_ReadWindow_DeleteMailRequest, MUIV_Toolbar_Qualifier);
    DoMethod(data->windowToolbar, MUIM_Toolbar_Notify,10, MUIV_Toolbar_Notify_Pressed, FALSE, obj, 1, MUIM_ReadWindow_MoveMailRequest);
    DoMethod(data->windowToolbar, MUIM_Toolbar_Notify,11, MUIV_Toolbar_Notify_Pressed, FALSE, obj, 3, MUIM_ReadWindow_NewMail, NEW_REPLY, MUIV_Toolbar_Qualifier);
    DoMethod(data->windowToolbar, MUIM_Toolbar_Notify,12, MUIV_Toolbar_Notify_Pressed, FALSE, obj, 3, MUIM_ReadWindow_NewMail, NEW_FORWARD, MUIV_Toolbar_Qualifier);
    DoMethod(obj, MUIM_Notify, MUIA_Window_CloseRequest, TRUE, obj, 3, MUIM_Set, MUIA_Window_Open, FALSE);
    DoMethod(obj, MUIM_Notify, MUIA_Window_InputEvent, "-capslock del",                 obj, 2, MUIM_ReadWindow_DeleteMailRequest, 0);
    DoMethod(obj, MUIM_Notify, MUIA_Window_InputEvent, "-capslock shift del",            obj, 2, MUIM_ReadWindow_DeleteMailRequest, IEQUALIFIER_LSHIFT);
    DoMethod(obj, MUIM_Notify, MUIA_Window_InputEvent, "-repeat -capslock space",       data->readMailGroup, 2, MUIM_TextEditor_ARexxCmd, "Next Page");
    DoMethod(obj, MUIM_Notify, MUIA_Window_InputEvent, "-repeat -capslock backspace",   data->readMailGroup, 2, MUIM_TextEditor_ARexxCmd, "Previous Page");
    DoMethod(obj, MUIM_Notify, MUIA_Window_InputEvent, "-repeat -capslock left",         obj, 3, MUIM_ReadWindow_SwitchMail, -1, 0);
    DoMethod(obj, MUIM_Notify, MUIA_Window_InputEvent, "-repeat -capslock right",       obj, 3, MUIM_ReadWindow_SwitchMail, +1, 0);
    DoMethod(obj, MUIM_Notify, MUIA_Window_InputEvent, "-repeat -capslock shift left",   obj, 3, MUIM_ReadWindow_SwitchMail, -1, IEQUALIFIER_LSHIFT);
    DoMethod(obj, MUIM_Notify, MUIA_Window_InputEvent, "-repeat -capslock shift right",  obj, 3, MUIM_ReadWindow_SwitchMail, +1, IEQUALIFIER_LSHIFT);

    // before we continue we make sure we connect a notify to the new window
    // so that we get informed if the window is closed and therefore can be
    // disposed
    // However, please note that because we do kill the window upon closing it
    // we have to use MUIM_Application_PushMethod instead of calling the ClosedReadWindowHook
    // directly
    DoMethod(obj, MUIM_Notify, MUIA_Window_Open, FALSE,
                  MUIV_Notify_Application, 6,
                    MUIM_Application_PushMethod, G->App, 3, MUIM_CallHook, &ClosedReadWindowHook, rmData);
  }

  // free the temporary mem we allocated before
  free(tmpData);

  return (ULONG)obj;
}

///
/// OVERLOAD(OM_GET)
OVERLOAD(OM_GET)
{
  GETDATA;
  ULONG *store = ((struct opGet *)msg)->opg_Storage;

  switch(((struct opGet *)msg)->opg_AttrID)
  {
    ATTR(ReadMailData) : *store = (ULONG)xget(data->readMailGroup, MUIA_ReadMailGroup_ReadMailData); return TRUE;
    ATTR(Num)           : *store = data->windowNumber; return TRUE;
  }

  return DoSuperMethodA(cl, obj, msg);
}
///
/// OVERLOAD(OM_SET)
OVERLOAD(OM_SET)
{
  struct TagItem *tags = inittags(msg), *tag;
  
  while((tag = NextTagItem(&tags)))
  {
    switch(tag->ti_Tag)
    {
      // we also catch foreign attributes
      case MUIA_Window_Open:
      {
        // if the window is supposed to be closed and the StatusChangeDelay is
        // active and no embeddedReadPane is active we have to cancel an eventually
        // existing timerequest to set the status of a mail to read.
        if(tag->ti_Data == FALSE &&
           C->StatusChangeDelayOn == TRUE && C->EmbeddedReadPane == FALSE &&
           xget(obj, MUIA_Window_Open) == TRUE)
        {
          TC_Stop(TIO_READSTATUSUPDATE);
        }
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
  GETDATA;

  // on a snapshot request we save the weights of all our objects here.
  G->Weights[10] = xget(data->readMailGroup, MUIA_ReadMailGroup_HGVertWeight);
  G->Weights[11] = xget(data->readMailGroup, MUIA_ReadMailGroup_TGVertWeight);

  // make sure the layout is saved
  SaveLayout(TRUE);

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
  BOOL isRealMail = !isVirtualMail(mail);
  BOOL isOutgoingMail = isRealMail ? isOutgoingFolder(folder) : FALSE;
  BOOL result = FALSE;
  BOOL initialCall = data->title[0] == '\0'; // TRUE if this is the first call
  int titleLen;

  D(DBF_GUI, "setting up readWindow for reading a mail");

  // enable/disable some menuitems in advance
  set(data->MI_EDIT,     MUIA_Menuitem_Enabled, isOutgoingMail);
  set(data->MI_MOVE,     MUIA_Menuitem_Enabled, isRealMail);
  set(data->MI_DELETE,  MUIA_Menuitem_Enabled, isRealMail);
  set(data->MI_CROP,    MUIA_Menuitem_Enabled, isRealMail);
  set(data->MI_CHSUBJ,  MUIA_Menuitem_Enabled, isRealMail);
  set(data->MI_NAVIG,    MUIA_Menuitem_Enabled, isRealMail);

  if(data->windowToolbar)
  {
    LONG pos = MUIV_NList_GetPos_Start;

    // query the position of the mail in the current listview
    DoMethod(G->MA->GUI.PG_MAILLIST, MUIM_NList_GetPos, mail, &pos);

    // now set some items of the toolbar ghosted/enabled
    DoMethod(data->windowToolbar, MUIM_Toolbar_Set, 0, MUIV_Toolbar_Set_Ghosted, isRealMail ? pos == 0 : TRUE);
    DoMethod(data->windowToolbar, MUIM_Toolbar_Set, 1, MUIV_Toolbar_Set_Ghosted, isRealMail ? pos == (folder->Total-1) : TRUE);
    DoMethod(data->windowToolbar, MUIM_Toolbar_Set, 9, MUIV_Toolbar_Set_Ghosted, !isRealMail);
    DoMethod(data->windowToolbar, MUIM_Toolbar_Set,10, MUIV_Toolbar_Set_Ghosted, !isRealMail);
    DoMethod(data->windowToolbar, MUIM_Toolbar_Set,11, MUIV_Toolbar_Set_Ghosted, isOutgoingMail);
  }
  
  if(isRealMail)
  {
    if(AllFolderLoaded() &&
       data->windowToolbar)
    {
      DoMethod(data->windowToolbar, MUIM_Toolbar_Set, 2, MUIV_Toolbar_Set_Ghosted, !RE_GetThread(mail, FALSE, FALSE, obj));
      DoMethod(data->windowToolbar, MUIM_Toolbar_Set, 3, MUIV_Toolbar_Set_Ghosted, !RE_GetThread(mail, TRUE, FALSE, obj));
    }
  }
  else if(data->windowToolbar)
  {
    DoMethod(data->windowToolbar, MUIM_Toolbar_Set, 2, MUIV_Toolbar_Set_Ghosted, TRUE);
    DoMethod(data->windowToolbar, MUIM_Toolbar_Set, 3, MUIV_Toolbar_Set_Ghosted, TRUE);
  }

  // Update the status groups
  DoMethod(data->statusIconGroup, MUIM_StatusIconGroup_Update, mail);

  // now we read in the mail in our read mail group
  if(DoMethod(data->readMailGroup, MUIM_ReadMailGroup_ReadMail, mail,
                                   initialCall == FALSE ? MUIF_ReadMailGroup_ReadMail_StatusChangeDelay : MUIF_NONE))
  {
    struct ReadMailData *rmData = (struct ReadMailData *)xget(data->readMailGroup, MUIA_ReadMailGroup_ReadMailData);

    // if the title of the window is empty, we can assume that no previous mail was
    // displayed in this readwindow, so we can set the mailTextObject of the readmailgroup
    // as the active object so that the user can browse through the mailtext immediatley after
    // opening the window
    if(initialCall)
      DoMethod(data->readMailGroup, MUIM_ReadMailGroup_ActivateMailText);

    // set the title of the readWindow now
    if(C->MultipleWindows == TRUE ||
       rmData == G->ActiveRexxRMData)
    {
      titleLen = snprintf(data->title, sizeof(data->title), "[%d] %s %s: ", data->windowNumber,
                                                            isOutgoingMail ? GetStr(MSG_To) : GetStr(MSG_From),
                                                            isOutgoingMail ? AddrName(mail->To) : AddrName(mail->From));
    }
    else
    {
      titleLen = snprintf(data->title, sizeof(data->title), "%s %s: ",
                                                            isOutgoingMail ? GetStr(MSG_To) : GetStr(MSG_From),
                                                            isOutgoingMail ? AddrName(mail->To) : AddrName(mail->From));
    }

    if(strlen(mail->Subject)+titleLen > SIZE_DEFAULT)
    {
      if(titleLen < SIZE_DEFAULT-3)
      {
        strlcat(data->title, mail->Subject, SIZE_DEFAULT-titleLen-3);
        strlcat(data->title, "...", sizeof(data->title)); // signals that the string was cut.
      }
      else
        strlcat(&data->title[SIZE_DEFAULT-4], "...", 4);
    }
    else
      strlcat(data->title, mail->Subject, sizeof(data->title));

    set(obj, MUIA_Window_Title, data->title);

    // enable some Menuitems depending on the read mail
    set(data->MI_EXTKEY, MUIA_Menuitem_Enabled, rmData->hasPGPKey);
    set(data->MI_CHKSIG, MUIA_Menuitem_Enabled, hasPGPSOldFlag(rmData) ||
                                                hasPGPSMimeFlag(rmData));
    set(data->MI_SAVEDEC, MUIA_Menuitem_Enabled, isRealMail &&
                                                 (hasPGPEMimeFlag(rmData) || hasPGPEOldFlag(rmData)));

    // everything worked fine
    result = TRUE;
  }

  return result;
}

///
/// DECLARE(NewMail)
DECLARE(NewMail) // enum NewMode mode, ULONG qualifier
{
  GETDATA;
  struct ReadMailData *rmData = (struct ReadMailData *)xget(data->readMailGroup, MUIA_ReadMailGroup_ReadMailData);
  struct Mail *mail = rmData->mail;
  int flags = 0;

  // check for qualifier keys
  if(msg->mode == NEW_FORWARD)
  {
    if(hasFlag(msg->qualifier, (IEQUALIFIER_LSHIFT|IEQUALIFIER_RSHIFT)))
      msg->mode = NEW_BOUNCE;
    else if(isFlagSet(msg->qualifier, IEQUALIFIER_CONTROL))
      SET_FLAG(flags, NEWF_FWD_NOATTACH);
  }
  else if(msg->mode == NEW_REPLY)
  {
    if(hasFlag(msg->qualifier, (IEQUALIFIER_LSHIFT|IEQUALIFIER_RSHIFT)))
      SET_FLAG(flags, NEWF_REP_PRIVATE);
  
    if(hasFlag(msg->qualifier, (IEQUALIFIER_LALT|IEQUALIFIER_RALT)))
      SET_FLAG(flags, NEWF_REP_MLIST);
  
    if(isFlagSet(msg->qualifier, IEQUALIFIER_CONTROL))
      SET_FLAG(flags, NEWF_REP_NOQUOTE);
   
  }

  // then create a new mail depending on the current mode
  if(MailExists(mail, NULL))
  {
    // create some fake mail list
    struct Mail *mlist[3];
    mlist[0] = (struct Mail*)1;
    mlist[1] = NULL;
    mlist[2] = mail;

    switch(msg->mode)
    {
      case NEW_NEW:     MA_NewNew(mail, flags);       break;
      case NEW_EDIT:    MA_NewEdit(mail, flags, obj); break;
      case NEW_BOUNCE:  MA_NewBounce(mail, flags);    break;
      case NEW_FORWARD: MA_NewForward(mlist, flags);   break;
      case NEW_REPLY:   MA_NewReply(mlist, flags);     break;

      default:
       // nothing
      break;
    }
  }

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

  if(MailExists(mail, srcfolder))
  {
    struct Folder *dstfolder = FolderRequest(GetStr(MSG_MA_MoveMsg),
                                             GetStr(MSG_MA_MoveMsgReq),
                                             GetStr(MSG_MA_MoveGad),
                                             GetStr(MSG_Cancel), srcfolder, obj);

    if(dstfolder)
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
      MA_MoveCopy(mail, srcfolder, dstfolder, FALSE);

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
        set(obj, MUIA_Window_Open, FALSE);

      AppendLogNormal(22, GetStr(MSG_LOG_Moving), (void *)1, srcfolder->Name, dstfolder->Name, "");
    }
  }

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

  if(MailExists(mail, srcfolder))
  {
    struct Folder *dstfolder = FolderRequest(GetStr(MSG_MA_CopyMsg),
                                             GetStr(MSG_MA_MoveMsgReq),
                                             GetStr(MSG_MA_CopyGad),
                                             GetStr(MSG_Cancel), NULL, obj);
    if(dstfolder)
    {
      // if there is no source folder this is a virtual mail that we
      // export to the destination folder
      if(srcfolder)
      {
        MA_MoveCopy(mail, srcfolder, dstfolder, TRUE);
        
        AppendLogNormal(24, GetStr(MSG_LOG_Copying), (void *)1, srcfolder->Name, dstfolder->Name, "");
      }
      else if(RE_Export(rmData, rmData->readFile,
                MA_NewMailFile(dstfolder, mail->MailFile), "", 0, FALSE, FALSE, (char*)ContType[CT_ME_EMAIL]))
      {
        struct Mail *newmail = AddMailToList(mail, dstfolder);

        if(dstfolder == FO_GetCurrentFolder())
          DoMethod(G->MA->GUI.PG_MAILLIST, MUIM_NList_InsertSingle, newmail, MUIV_NList_Insert_Sorted);

        setStatusToRead(newmail); // OLD status
      }
    }
  }

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
  struct Folder *delfolder = FO_GetFolderByType(FT_DELETED, NULL);
  BOOL delatonce = hasFlag(msg->qualifier, (IEQUALIFIER_LSHIFT|IEQUALIFIER_RSHIFT));
  BOOL closeAfter = FALSE;

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
    MA_DeleteSingle(mail, delatonce, FALSE);

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
      set(obj, MUIA_Window_Open, FALSE);

    if(delatonce)
      AppendLogNormal(20, GetStr(MSG_LOG_Deleting), (void *)1, folder->Name, "", "");
    else
      AppendLogNormal(22, GetStr(MSG_LOG_Moving), (void *)1, folder->Name, delfolder->Name, "");
  }

  return 0;
}

///
/// DECLARE(PrintMailRequest)
DECLARE(PrintMailRequest)
{
  GETDATA;
  struct ReadMailData *rmData = (struct ReadMailData *)xget(data->readMailGroup, MUIA_ReadMailGroup_ReadMailData);
  struct Part *part;
  struct TempFile *prttmp;

  if((part = AttachRequest(GetStr(MSG_RE_PrintMsg),
                           GetStr(MSG_RE_SelectPrintPart),
                           GetStr(MSG_RE_PrintGad),
                           GetStr(MSG_Cancel), ATTREQ_PRINT|ATTREQ_MULTI, rmData)))
  {
    BusyText(GetStr(MSG_BusyDecPrinting), "");

    for(; part; part = part->NextSelected)
    {
      switch(part->Nr)
      {
        case PART_ORIGINAL:
          RE_PrintFile(rmData->readFile);
        break;

        case PART_ALLTEXT:
        {
          if((prttmp = OpenTempFile("w")))
          {
            DoMethod(data->readMailGroup, MUIM_ReadMailGroup_SaveDisplay, prttmp->FP);
            fclose(prttmp->FP);
            prttmp->FP = NULL;
            
            RE_PrintFile(prttmp->Filename);
            CloseTempFile(prttmp);
          }
        }
        break;

        default:
          RE_PrintFile(part->Filename);
      }
    }

    BusyEnd();
  }

  return 0;
}

///
/// DECLARE(SaveMailRequest)
//  Saves the current message or an attachment to disk
DECLARE(SaveMailRequest)
{
  GETDATA;
  struct ReadMailData *rmData = (struct ReadMailData *)xget(data->readMailGroup, MUIA_ReadMailGroup_ReadMailData);
  struct Part *part;
  struct TempFile *tf;

  if((part = AttachRequest(GetStr(MSG_RE_SaveMessage),
                           GetStr(MSG_RE_SelectSavePart),
                           GetStr(MSG_RE_SaveGad),
                           GetStr(MSG_Cancel), ATTREQ_SAVE|ATTREQ_MULTI, rmData)))
  {
    BusyText(GetStr(MSG_BusyDecSaving), "");
    
    for(; part; part = part->NextSelected)
    {
      switch(part->Nr)
      {
        case PART_ORIGINAL:
        {
          RE_Export(rmData, rmData->readFile, "", "", 0, FALSE, FALSE, (char*)ContType[CT_ME_EMAIL]);
        }
        break;

        case PART_ALLTEXT:
        {
          if((tf = OpenTempFile("w")))
          {
            DoMethod(data->readMailGroup, MUIM_ReadMailGroup_SaveDisplay, tf->FP);
            fclose(tf->FP);
            tf->FP = NULL;

            RE_Export(rmData, tf->Filename, "", "", 0, FALSE, FALSE, (char*)ContType[CT_TX_PLAIN]);
            CloseTempFile(tf);
          }
        }
        break;

        default:
        {
          RE_DecodePart(part);
          
          RE_Export(rmData, part->Filename, "",
                    part->CParFileName ? part->CParFileName : part->Name, part->Nr,
                    FALSE, FALSE, part->ContentType);
        }
      }
    }

    BusyEnd();
  }

  return 0;
}

///
/// DECLARE(DisplayMailRequest)
DECLARE(DisplayMailRequest)
{
  GETDATA;
  struct ReadMailData *rmData = (struct ReadMailData *)xget(data->readMailGroup, MUIA_ReadMailGroup_ReadMailData);
  struct Part *part;

  if((part = AttachRequest(GetStr(MSG_RE_DisplayMsg),
                           GetStr(MSG_RE_SelectDisplayPart),
                           GetStr(MSG_RE_DisplayGad),
                           GetStr(MSG_Cancel), ATTREQ_DISP|ATTREQ_MULTI, rmData)))
  {
    BusyText(GetStr(MSG_BusyDecDisplaying), "");

    for(; part; part = part->NextSelected)
    {
      RE_DecodePart(part);
      
      switch(part->Nr)
      {
        case PART_ORIGINAL:
        {
          RE_DisplayMIME(rmData->readFile, "text/plain");
        }
        break;

        default:
          RE_DisplayMIME(part->Filename, part->ContentType);
      }
    }
    
    BusyEnd();
  }

  return 0;
}

///
/// DECLARE(CropAttachmentsRequest)
//  Removes attachments from the current message
DECLARE(CropAttachmentsRequest)
{
  GETDATA;
  struct ReadMailData *rmData = (struct ReadMailData *)xget(data->readMailGroup, MUIA_ReadMailGroup_ReadMailData);
  struct Mail *mail = rmData->mail;

  // remove the attchments now
  MA_RemoveAttach(mail, TRUE);
  
  if(DoMethod(G->MA->GUI.PG_MAILLIST, MUIM_MainMailListGroup_RedrawMail, mail))
  {
    MA_ChangeSelected(TRUE);
    DisplayStatistics(mail->Folder, TRUE);
  }
  
  // make sure to refresh the mail of this window as we do not
  // have any attachments anymore
  DoMethod(obj, MUIM_ReadWindow_ReadMail, mail);

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

  if(MailExists(mail, mail->Folder))
  {
    struct Mail *mlist[3];
    mlist[0] = (struct Mail *)1;
    mlist[1] = NULL;
    mlist[2] = mail;
    MA_GetAddress(mlist);
  }

  return 0;
}

///
/// DECLARE(SetMailToUnread)
//  Sets the status of the current mail to unread
DECLARE(SetMailToUnread)
{
  GETDATA;
  struct ReadMailData *rmData = (struct ReadMailData *)xget(data->readMailGroup, MUIA_ReadMailGroup_ReadMailData);
  struct Mail *mail = rmData->mail;

  setStatusToUnread(mail);

  DoMethod(data->statusIconGroup, MUIM_StatusIconGroup_Update, mail);
  DisplayStatistics(NULL, TRUE);

  return 0;
}

///
/// DECLARE(SetMailToMarked)
//  Sets the flags of the current mail to marked
DECLARE(SetMailToMarked)
{
  GETDATA;
  struct ReadMailData *rmData = (struct ReadMailData *)xget(data->readMailGroup, MUIA_ReadMailGroup_ReadMailData);
  struct Mail *mail = rmData->mail;

  setStatusToMarked(mail);

  DoMethod(data->statusIconGroup, MUIM_StatusIconGroup_Update, mail);
  DisplayStatistics(NULL, TRUE);

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

  if(MailExists(mail, folder))
  {
    char subj[SIZE_SUBJECT];

    strlcpy(subj, mail->Subject, sizeof(subj));
    
    if(StringRequest(subj, SIZE_SUBJECT,
                     GetStr(MSG_MA_ChangeSubj),
                     GetStr(MSG_MA_ChangeSubjReq),
                     GetStr(MSG_Okay), NULL, GetStr(MSG_Cancel), FALSE, obj))
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

  return 0;
}

///
/// DECLARE(SwitchMail)
//  Goes to next or previous (new) message in list
DECLARE(SwitchMail) // LONG direction, ULONG qualifier
{
  GETDATA;
  struct ReadMailData *rmData = (struct ReadMailData *)xget(data->readMailGroup, MUIA_ReadMailGroup_ReadMailData);
  struct Mail *mail = rmData->mail;
  struct Folder *folder = mail->Folder;
  LONG direction = msg->direction;
  LONG act = MUIV_NList_GetPos_Start;
  BOOL onlynew = hasFlag(msg->qualifier, (IEQUALIFIER_LSHIFT|IEQUALIFIER_RSHIFT));
  BOOL found = FALSE;

  // save the direction we are going to process now
  data->lastDirection = direction;

  // we have to make sure that the folder the next/prev mail will
  // be showed from is active, that`s why we call ChangeFolder with TRUE.
  MA_ChangeFolder(folder, TRUE);

  // after changing the folder we have to get the MailInfo (Position etc.)
  DoMethod(G->MA->GUI.PG_MAILLIST, MUIM_NList_GetPos, mail, &act);

  D(DBF_GUI, "act: %d - direction: %d", act, direction);

  if(act != MUIV_NList_GetPos_End)
  {
    for(act += direction; act >= 0; act += direction)
    {
      DoMethod(G->MA->GUI.PG_MAILLIST, MUIM_NList_GetEntry, act, &mail);
      if(!mail)
        break;

      if(!onlynew ||
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
  if(!found && onlynew)
  {
    if(C->AskJumpUnread)
    {
      struct Folder **flist;

      if((flist = FO_CreateList()))
      {
        int i;

        // look for the current folder in the array
        for(i = 1; i <= (int)*flist; i++)
        {
          if(flist[i] == folder)
            break;
        }

        // look for first folder with at least one unread mail
        // and if found read that mail
        for(i += direction; i <= (int)*flist && i >= 1; i += direction)
        {
          if(flist[i]->Type != FT_GROUP && flist[i]->Unread > 0)
          {
            if(!MUI_Request(G->App, obj, 0, GetStr(MSG_MA_ConfirmReq),
                                            GetStr(MSG_YesNoReq),
                                            GetStr(MSG_RE_MoveNextFolderReq), flist[i]->Name))
            {
              break;
            }

            MA_ChangeFolder(flist[i], TRUE);
            DoMethod(G->MA->GUI.PG_MAILLIST, MUIM_NList_GetEntry, MUIV_NList_GetEntry_Active, &mail);
            if(!mail)
              break;
            
            DoMethod(obj, MUIM_ReadWindow_ReadMail, mail);

            // this is a valid break and not break because of an error
            found = TRUE;
            break;
          }
        }

        // beep if no folder with unread mails was found
        if(i > (int)*flist || i < 1)
          DisplayBeep(_screen(obj));

        free(flist);
      }
    }
    else
      DisplayBeep(_screen(obj));
  }

  // if we didn't find any next/previous mail (mail == NULL) then
  // we can close the window accordingly. This signals a user that he/she
  // reached the end of the mail list
  if(found == FALSE)
    set(obj, MUIA_Window_Open, FALSE);

  return 0;
}

///
/// DECLARE(FollowThread)
//  Follows a thread in either direction
DECLARE(FollowThread) // LONG direction
{
  GETDATA;
  struct ReadMailData *rmData = (struct ReadMailData *)xget(data->readMailGroup, MUIA_ReadMailGroup_ReadMailData);
  struct Mail *mail = rmData->mail;

  // depending on the direction we get the Question or Answer to the current Message
  struct Mail *fmail = RE_GetThread(mail, msg->direction <= 0 ? FALSE : TRUE, TRUE, obj);

  if(fmail)
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
    DisplayBeep(_screen(obj));

  return 0;
}

///
/// DECLARE(ChangeHeaderMode)
//  Changes display options (header)
DECLARE(ChangeHeaderMode) // enum HeaderMode hmode
{
  GETDATA;
  struct ReadMailData *rmData = (struct ReadMailData *)xget(data->readMailGroup, MUIA_ReadMailGroup_ReadMailData);

  // change the header display mode
  rmData->headerMode = msg->hmode;

  // issue an update of the readMailGroup
  DoMethod(data->readMailGroup, MUIM_ReadMailGroup_ReadMail, rmData->mail,
                                MUIF_ReadMailGroup_ReadMail_UpdateOnly);

  return 0;
}

///
/// DECLARE(ChangeSenderInfoMode)
//  Changes display options (sender info)
DECLARE(ChangeSenderInfoMode) // enum SInfoMode simode
{
  GETDATA;
  struct ReadMailData *rmData = (struct ReadMailData *)xget(data->readMailGroup, MUIA_ReadMailGroup_ReadMailData);

  // change the sender info mode
  rmData->senderInfoMode = msg->simode;

  // issue an update of the readMailGroup
  DoMethod(data->readMailGroup, MUIM_ReadMailGroup_ReadMail, rmData->mail,
                                MUIF_ReadMailGroup_ReadMail_UpdateOnly);

  return 0;
}

///
/// DECLARE(StyleOptionsChanged)
DECLARE(StyleOptionsChanged)
{
  GETDATA;
  struct ReadMailData *rmData = (struct ReadMailData *)xget(data->readMailGroup, MUIA_ReadMailGroup_ReadMailData);

      
  // check the menu items for the style options
  // what we are going to enable/disable in our upcoming update
  rmData->wrapHeaders    = xget(data->MI_WRAPH, MUIA_Menuitem_Checked);
  rmData->noTextstyles   = !xget(data->MI_TSTYLE, MUIA_Menuitem_Checked);
  rmData->useFixedFont  = xget(data->MI_FFONT, MUIA_Menuitem_Checked);

  // issue an update of the readMailGroup
  DoMethod(data->readMailGroup, MUIM_ReadMailGroup_ReadMail, rmData->mail,
                                MUIF_ReadMailGroup_ReadMail_UpdateOnly);

  return 0;
}

///
/// DECLARE(StatusIconRefresh)
DECLARE(StatusIconRefresh)
{
  GETDATA;
  struct ReadMailData *rmData = (struct ReadMailData *)xget(data->readMailGroup, MUIA_ReadMailGroup_ReadMailData);

  // Update the statusIconGroup
  if(rmData->mail)
    DoMethod(data->statusIconGroup, MUIM_StatusIconGroup_Update, rmData->mail);

  return 0;
}
///
