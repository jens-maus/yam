/***************************************************************************

 YAM - Yet Another Mailer
 Copyright (C) 1995-2000 by Marcel Beck <mbeck@yam.ch>
 Copyright (C) 2000-2004 by YAM Open Source Team

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

 Superclass:  MUIC_NList
 Description: NList class of the main mail list in the main window

***************************************************************************/

#include "MainMailList_cl.h"

/* CLASSDATA
struct Data
{
	Object *context_menu;
};
*/

/* Overloaded Methods */
/// OVERLOAD(OM_DISPOSE)
OVERLOAD(OM_DISPOSE)
{
	GETDATA;

	// make sure that our context menus are also disposed
	if(data->context_menu)
		MUI_DisposeObject(data->context_menu);

	return DoSuperMethodA(cl,obj,msg);
}

///
/// OVERLOAD(MUIM_NList_ContextMenuBuild)
OVERLOAD(MUIM_NList_ContextMenuBuild)
{
	GETDATA;
	struct MUIP_NList_ContextMenuBuild *m = (struct MUIP_NList_ContextMenuBuild *)msg;
  static char menutitle[SIZE_DEFAULT];
  struct MUI_NList_TestPos_Result res;
  struct Mail *mail = NULL;
  struct MA_GUIData *gui = &G->MA->GUI;
  struct Folder *fo = FO_GetCurrentFolder();
  BOOL isOutBox = isOutgoingFolder(fo);
  BOOL beingedited = FALSE, hasattach = FALSE;

  // dispose the old context_menu if it still exists
  if(data->context_menu)
  {
    MUI_DisposeObject(data->context_menu);
    data->context_menu = NULL;
  }

  // if this was a RMB click on the titlebar we create our own special menu
	if(m->ontop)
  {
    data->context_menu = MenustripObject,
      Child, MenuObjectT(GetStr(MSG_MA_CTX_MAILLIST)),
        Child, MenuitemObject, MUIA_Menuitem_Title, GetStr(MSG_Status),         MUIA_UserData, 1, MUIA_Menuitem_Enabled, FALSE, MUIA_Menuitem_Checked, isFlagSet(C->MessageCols, (1<<0)), MUIA_Menuitem_Checkit, TRUE, MUIA_Menuitem_Toggle, TRUE, End,
        Child, MenuitemObject, MUIA_Menuitem_Title, GetStr(MSG_SenderRecpt),    MUIA_UserData, 2, MUIA_Menuitem_Checked, isFlagSet(C->MessageCols, (1<<1)), MUIA_Menuitem_Checkit, TRUE, MUIA_Menuitem_Toggle, TRUE, End,
        Child, MenuitemObject, MUIA_Menuitem_Title, GetStr(MSG_ReturnAddress),  MUIA_UserData, 3, MUIA_Menuitem_Checked, isFlagSet(C->MessageCols, (1<<2)), MUIA_Menuitem_Checkit, TRUE, MUIA_Menuitem_Toggle, TRUE, End,
        Child, MenuitemObject, MUIA_Menuitem_Title, GetStr(MSG_Subject),        MUIA_UserData, 4, MUIA_Menuitem_Checked, isFlagSet(C->MessageCols, (1<<3)), MUIA_Menuitem_Checkit, TRUE, MUIA_Menuitem_Toggle, TRUE, End,
        Child, MenuitemObject, MUIA_Menuitem_Title, GetStr(MSG_MessageDate),    MUIA_UserData, 5, MUIA_Menuitem_Checked, isFlagSet(C->MessageCols, (1<<4)), MUIA_Menuitem_Checkit, TRUE, MUIA_Menuitem_Toggle, TRUE, End,
        Child, MenuitemObject, MUIA_Menuitem_Title, GetStr(MSG_Size),           MUIA_UserData, 6, MUIA_Menuitem_Checked, isFlagSet(C->MessageCols, (1<<5)), MUIA_Menuitem_Checkit, TRUE, MUIA_Menuitem_Toggle, TRUE, End,
        Child, MenuitemObject, MUIA_Menuitem_Title, GetStr(MSG_Filename),       MUIA_UserData, 7, MUIA_Menuitem_Checked, isFlagSet(C->MessageCols, (1<<6)), MUIA_Menuitem_Checkit, TRUE, MUIA_Menuitem_Toggle, TRUE, End,
        Child, MenuitemObject, MUIA_Menuitem_Title, GetStr(MSG_CO_DATE_SNTRCVD),MUIA_UserData, 8, MUIA_Menuitem_Checked, isFlagSet(C->MessageCols, (1<<7)), MUIA_Menuitem_Checkit, TRUE, MUIA_Menuitem_Toggle, TRUE, End,
        Child, MenuitemObject, MUIA_Menuitem_Title, NM_BARLABEL, End,
        Child, MenuitemObject, MUIA_Menuitem_Title, GetStr(MSG_MA_CTX_DEFWIDTH_THIS), MUIA_UserData, MUIV_NList_Menu_DefWidth_This, End,
        Child, MenuitemObject, MUIA_Menuitem_Title, GetStr(MSG_MA_CTX_DEFWIDTH_ALL),  MUIA_UserData, MUIV_NList_Menu_DefWidth_All,  End,
        Child, MenuitemObject, MUIA_Menuitem_Title, GetStr(MSG_MA_CTX_DEFORDER_THIS), MUIA_UserData, MUIV_NList_Menu_DefOrder_This, End,
        Child, MenuitemObject, MUIA_Menuitem_Title, GetStr(MSG_MA_CTX_DEFORDER_ALL),  MUIA_UserData, MUIV_NList_Menu_DefOrder_All,  End,
      End,
    End;

    return (ULONG)data->context_menu;
  }

	if(!fo)
		return(0);

  // Now lets find out which entry is under the mouse pointer
	DoMethod(gui->NL_MAILS, MUIM_NList_TestPos, m->mx, m->my, &res);

  if(res.entry >= 0)
  {
    int i;
		
		DoMethod(gui->NL_MAILS, MUIM_NList_GetEntry, res.entry, &mail);
		if(!mail)
			return(0);

    fo->LastActive = xget(gui->NL_MAILS, MUIA_NList_Active);

    if(isMultiPartMail(mail)) hasattach = TRUE;

    for (i = 0; i < MAXWR; i++)
    {
      if (G->WR[i] && G->WR[i]->Mail == mail) beingedited = TRUE;
    }

    // Now we set this entry as activ
    if(fo->LastActive != res.entry) set(gui->NL_MAILS, MUIA_NList_Active, res.entry);
  }

  // now we create the menu title of the context menu
  if(mail)
  {
    struct Person *pers = isOutBox ? &mail->To : &mail->From;

    strcpy(menutitle, GetStr(isOutBox ? MSG_To : MSG_From));
    strcat(menutitle, ": ");
    strncat(menutitle, BuildAddrName2(pers), 20-strlen(menutitle));
    strcat(menutitle, "...");
  }
  else
  {
    strcpy(menutitle, GetStr(MSG_MAIL_NONSEL));
  }

  data->context_menu = MenustripObject,
    Child, MenuObjectT(menutitle),
      Child, MenuitemObject, MUIA_Menuitem_Title, GetStripStr(MSG_MA_MRead),        MUIA_Menuitem_Enabled, mail,               MUIA_UserData, MMEN_READ,       End,
      Child, MenuitemObject, MUIA_Menuitem_Title, GetStripStr(MSG_MESSAGE_EDIT),    MUIA_Menuitem_Enabled, mail && isOutBox && !beingedited,   MUIA_UserData, MMEN_EDIT,       End,
      Child, MenuitemObject, MUIA_Menuitem_Title, GetStripStr(MSG_MESSAGE_REPLY),   MUIA_Menuitem_Enabled, mail,               MUIA_UserData, MMEN_REPLY,      End,
      Child, MenuitemObject, MUIA_Menuitem_Title, GetStripStr(MSG_MESSAGE_FORWARD), MUIA_Menuitem_Enabled, mail,               MUIA_UserData, MMEN_FORWARD,    End,
      Child, MenuitemObject, MUIA_Menuitem_Title, GetStripStr(MSG_MESSAGE_BOUNCE),  MUIA_Menuitem_Enabled, mail,               MUIA_UserData, MMEN_BOUNCE,     End,
      Child, MenuitemObject, MUIA_Menuitem_Title, GetStripStr(MSG_MA_MSend),        MUIA_Menuitem_Enabled, mail && (fo->Type != FT_OUTGOING), MUIA_UserData, MMEN_SEND, End,
      Child, MenuitemObject, MUIA_Menuitem_Title, GetStripStr(MSG_MA_ChangeSubj),   MUIA_Menuitem_Enabled, mail,               MUIA_UserData, MMEN_CHSUBJ,     End,
      Child, MenuitemObject, MUIA_Menuitem_Title, GetStr(MSG_MA_SetStatus),         MUIA_Menuitem_Enabled, mail,
        Child, MenuitemObject, MUIA_Menuitem_Title, GetStripStr(MSG_MA_TOMARKED),   MUIA_Menuitem_Enabled, mail,               MUIA_UserData, MMEN_TOMARKED,   End,
        Child, MenuitemObject, MUIA_Menuitem_Title, GetStripStr(MSG_MA_TOUNMARKED), MUIA_Menuitem_Enabled, mail,               MUIA_UserData, MMEN_TOUNMARKED, End,
        Child, MenuitemObject, MUIA_Menuitem_Title, GetStripStr(MSG_MA_ToUnread),   MUIA_Menuitem_Enabled, mail && !isOutBox,  MUIA_UserData, MMEN_TOUNREAD,   End,
        Child, MenuitemObject, MUIA_Menuitem_Title, GetStripStr(MSG_MA_ToRead),     MUIA_Menuitem_Enabled, mail && !isOutBox,  MUIA_UserData, MMEN_TOREAD,     End,
        Child, MenuitemObject, MUIA_Menuitem_Title, GetStripStr(MSG_MA_ToHold),     MUIA_Menuitem_Enabled, mail && isOutBox,   MUIA_UserData, MMEN_TOHOLD,     End,
        Child, MenuitemObject, MUIA_Menuitem_Title, GetStripStr(MSG_MA_ToQueued),   MUIA_Menuitem_Enabled, mail && isOutBox,   MUIA_UserData, MMEN_TOQUEUED,   End,
      End,
      Child, MenuitemObject, MUIA_Menuitem_Title, NM_BARLABEL, End,
        Child, MenuitemObject, MUIA_Menuitem_Title, GetStripStr(MSG_MESSAGE_GETADDRESS),  MUIA_Menuitem_Enabled, mail, MUIA_UserData, MMEN_SAVEADDR, End,
        Child, MenuitemObject, MUIA_Menuitem_Title, GetStripStr(MSG_MESSAGE_MOVE),        MUIA_Menuitem_Enabled, mail, MUIA_UserData, MMEN_MOVE,     End,
        Child, MenuitemObject, MUIA_Menuitem_Title, GetStripStr(MSG_MESSAGE_COPY),        MUIA_Menuitem_Enabled, mail, MUIA_UserData, MMEN_COPY,     End,
        Child, MenuitemObject, MUIA_Menuitem_Title, GetStripStr(MSG_MA_MDelete),          MUIA_Menuitem_Enabled, mail, MUIA_UserData, MMEN_DELETE,   End,
        Child, MenuitemObject, MUIA_Menuitem_Title, NM_BARLABEL, End,
        Child, MenuitemObject, MUIA_Menuitem_Title, GetStripStr(MSG_MESSAGE_PRINT),       MUIA_Menuitem_Enabled, mail, MUIA_UserData, MMEN_PRINT,    End,
        Child, MenuitemObject, MUIA_Menuitem_Title, GetStripStr(MSG_MESSAGE_SAVE),        MUIA_Menuitem_Enabled, mail, MUIA_UserData, MMEN_SAVE,     End,
        Child, MenuitemObject, MUIA_Menuitem_Title, GetStr(MSG_Attachments),              MUIA_Menuitem_Enabled, mail && hasattach,
          Child, MenuitemObject, MUIA_Menuitem_Title, GetStripStr(MSG_MESSAGE_SAVEATT),     MUIA_Menuitem_Enabled, mail && hasattach, MUIA_UserData, MMEN_DETACH, End,
          Child, MenuitemObject, MUIA_Menuitem_Title, GetStripStr(MSG_MESSAGE_CROP),        MUIA_Menuitem_Enabled, mail && hasattach, MUIA_UserData, MMEN_CROP,   End,
        End,
        Child, MenuitemObject, MUIA_Menuitem_Title, GetStripStr(MSG_MESSAGE_EXPORT),      MUIA_Menuitem_Enabled, mail, MUIA_UserData, MMEN_EXPMSG,   End,
        Child, MenuitemObject, MUIA_Menuitem_Title, NM_BARLABEL, End,
        Child, MenuitemObject, MUIA_Menuitem_Title, GetStripStr(MSG_MESSAGE_NEW),         MUIA_Menuitem_Enabled, mail, MUIA_UserData, MMEN_NEW,      End,
        Child, MenuitemObject, MUIA_Menuitem_Title, GetStr(MSG_MA_Select),
          Child, MenuitemObject, MUIA_Menuitem_Title, GetStripStr(MSG_MA_SelectAll),      MUIA_UserData, MMEN_SELALL,  End,
          Child, MenuitemObject, MUIA_Menuitem_Title, GetStripStr(MSG_MA_SelectNone),     MUIA_UserData, MMEN_SELNONE, End,
          Child, MenuitemObject, MUIA_Menuitem_Title, GetStripStr(MSG_MA_SelectToggle),   MUIA_UserData, MMEN_SELTOGG, End,
        End,
      End,
    End;

  return (ULONG)data->context_menu;
}

///
/// OVERLOAD(MUIM_ContextMenuChoice)
OVERLOAD(MUIM_ContextMenuChoice)
{
	struct MUIP_ContextMenuChoice *m = (struct MUIP_ContextMenuChoice *)msg;
	struct MA_GUIData *gui = &G->MA->GUI;

	switch(xget(m->item, MUIA_UserData))
  {
    // if the user selected a TitleContextMenu item
    case 1:
    case 2:
    case 3:
    case 4:
    case 5:
    case 6:
    case 7:
    case 8:
    {
			ULONG col = xget(m->item, MUIA_UserData)-1;

      if(isFlagSet(C->MessageCols, (1<<col))) CLEAR_FLAG(C->MessageCols, (1<<col));
      else                                    SET_FLAG(C->MessageCols, (1<<col));

      MA_MakeMAFormat(G->MA->GUI.NL_MAILS);
    }
    break;

    // or other item out of the MailListContextMenu
    case MMEN_READ:       DoMethod(G->App, MUIM_CallHook, &MA_ReadMessageHook); break;
    case MMEN_EDIT:       DoMethod(G->App, MUIM_CallHook, &MA_NewMessageHook,     NEW_EDIT,    0); break;
    case MMEN_REPLY:      DoMethod(G->App, MUIM_CallHook, &MA_NewMessageHook,     NEW_REPLY,   0); break;
    case MMEN_FORWARD:    DoMethod(G->App, MUIM_CallHook, &MA_NewMessageHook,     NEW_FORWARD, 0); break;
    case MMEN_BOUNCE:     DoMethod(G->App, MUIM_CallHook, &MA_NewMessageHook,     NEW_BOUNCE,  0); break;
    case MMEN_SEND:       DoMethod(G->App, MUIM_CallHook, &MA_SendHook,           SEND_ACTIVE); break;
    case MMEN_CHSUBJ:     DoMethod(G->App, MUIM_CallHook, &MA_ChangeSubjectHook); break;
    case MMEN_TOUNREAD:   DoMethod(G->App, MUIM_CallHook, &MA_SetStatusToHook,    SFLAG_NONE,   SFLAG_NEW|SFLAG_READ); break;
    case MMEN_TOREAD:     DoMethod(G->App, MUIM_CallHook, &MA_SetStatusToHook,    SFLAG_READ,   SFLAG_NEW); break;
    case MMEN_TOHOLD:     DoMethod(G->App, MUIM_CallHook, &MA_SetStatusToHook,    SFLAG_HOLD,   SFLAG_NEW); break;
    case MMEN_TOQUEUED:   DoMethod(G->App, MUIM_CallHook, &MA_SetStatusToHook,    SFLAG_QUEUED, SFLAG_SENT); break;
    case MMEN_TOMARKED:   DoMethod(G->App, MUIM_CallHook, &MA_SetStatusToHook,    SFLAG_MARKED, SFLAG_NONE); break;
    case MMEN_TOUNMARKED: DoMethod(G->App, MUIM_CallHook, &MA_SetStatusToHook,    SFLAG_NONE,   SFLAG_MARKED); break;
    case MMEN_SAVEADDR:   DoMethod(G->App, MUIM_CallHook, &MA_GetAddressHook); break;
    case MMEN_MOVE:       DoMethod(G->App, MUIM_CallHook, &MA_MoveMessageHook); break;
    case MMEN_COPY:       DoMethod(G->App, MUIM_CallHook, &MA_CopyMessageHook); break;
    case MMEN_DELETE:     DoMethod(G->App, MUIM_CallHook, &MA_DeleteMessageHook,  0); break;
    case MMEN_PRINT:      DoMethod(G->App, MUIM_CallHook, &MA_SavePrintHook,      TRUE); break;
    case MMEN_SAVE:       DoMethod(G->App, MUIM_CallHook, &MA_SavePrintHook,      FALSE); break;
    case MMEN_DETACH:     DoMethod(G->App, MUIM_CallHook, &MA_SaveAttachHook); break;
    case MMEN_CROP:       DoMethod(G->App, MUIM_CallHook, &MA_RemoveAttachHook); break;
    case MMEN_EXPMSG:     DoMethod(G->App, MUIM_CallHook, &MA_ExportMessagesHook, FALSE); break;
    case MMEN_NEW:        DoMethod(G->App, MUIM_CallHook, &MA_NewMessageHook,     NEW_NEW,  0); break;
    case MMEN_SELALL:     DoMethod(gui->NL_MAILS, MUIM_NList_Select, MUIV_NList_Select_All, MUIV_NList_Select_On,     NULL); break;
    case MMEN_SELNONE:    DoMethod(gui->NL_MAILS, MUIM_NList_Select, MUIV_NList_Select_All, MUIV_NList_Select_Off,    NULL); break;
    case MMEN_SELTOGG:    DoMethod(gui->NL_MAILS, MUIM_NList_Select, MUIV_NList_Select_All, MUIV_NList_Select_Toggle, NULL); break;

    default:
    {
			return DoSuperMethodA(cl, obj, msg);
    }
  }

  return 0;
}

///

/* Private Functions */

/* Public Methods */
