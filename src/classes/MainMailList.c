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

 Superclass:  MUIC_NList
 Description: NList class of the main mail list in the main window

***************************************************************************/

#include "MainMailList_cl.h"

#include "Debug.h"

/* CLASSDATA
struct Data
{
  Object *context_menu;
  Object *statusImage[MAX_STATUSIMG];
};
*/

/* Private Functions */
/// MailCompare
//  Compares two messages
static int MailCompare(struct Mail *entry1, struct Mail *entry2, LONG column)
{
  switch (column)
  {
    case 0:
    {
      // lets calculate each value
      int status1 = 0;
      int status2 = 0;

      // We do not sort on other things than the real status and the Importance+Marked flag of
      // the message because this would be confusing if you use "Status" as a sorting
      // criteria within the folder config. Why should a MultiPart mail be sorted with
      // other multipart messages? It`s more important to sort just for New/Unread/Read aso
      // and then be able to sort as a second criteria for the date. Sorting the message
      // depending on other stuff than importance will make it impossible to sort for
      // status+date in the folder config. Perhaps we need to have a configuable way for
      // sorting by status later, but this is future stuff..
      status1 += hasStatusNew(entry1) ? 512 : 0;
      status2 += hasStatusNew(entry2) ? 512 : 0;
      status1 += !hasStatusRead(entry1) ? 256 : 0;
      status2 += !hasStatusRead(entry2) ? 256 : 0;
      status1 += !hasStatusError(entry1) ? 256 : 0;
      status2 += !hasStatusError(entry2) ? 256 : 0;
      status1 += hasStatusHold(entry1) ? 128 : 0;
      status2 += hasStatusHold(entry2) ? 128 : 0;
      status1 += hasStatusReplied(entry1) ? 64 : 0;
      status2 += hasStatusReplied(entry2) ? 64 : 0;
      status1 += hasStatusQueued(entry1) ? 64 : 0;
      status2 += hasStatusQueued(entry2) ? 64 : 0;
      status1 += hasStatusForwarded(entry1) ? 32 : 0;
      status2 += hasStatusForwarded(entry2) ? 32 : 0;
      status1 += hasStatusSent(entry1) ? 32 : 0;
      status2 += hasStatusSent(entry2) ? 32 : 0;
      status1 += hasStatusDeleted(entry1) ? 16 : 0;
      status2 += hasStatusDeleted(entry2) ? 16 : 0;
      status1 += hasStatusMarked(entry1) ? 8  : 0;
      status2 += hasStatusMarked(entry2) ? 8  : 0;
      status1 += (getImportanceLevel(entry1) == IMP_HIGH)  ? 16 : 0;
      status2 += (getImportanceLevel(entry2) == IMP_HIGH)  ? 16 : 0;

      return -(status1)+(status2);
    }
    break;

    case 1:
    {
      if(isOutgoingFolder(entry1->Folder))
      {
        return stricmp(*entry1->To.RealName ? entry1->To.RealName : entry1->To.Address,
                       *entry2->To.RealName ? entry2->To.RealName : entry2->To.Address);
      }
      else
      {
        return stricmp(*entry1->From.RealName ? entry1->From.RealName : entry1->From.Address,
                       *entry2->From.RealName ? entry2->From.RealName : entry2->From.Address);
      }
    }
    break;

    case 2:
    {
      return stricmp(*entry1->ReplyTo.RealName ? entry1->ReplyTo.RealName : entry1->ReplyTo.Address,
                     *entry2->ReplyTo.RealName ? entry2->ReplyTo.RealName : entry2->ReplyTo.Address);
    }
    break;

    case 3:
    {
      return stricmp(MA_GetRealSubject(entry1->Subject), MA_GetRealSubject(entry2->Subject));
    }
    break;

    case 4:
    {
      return CompareDates(&entry2->Date, &entry1->Date);
    }
    break;

    case 5:
    {
      return entry1->Size-entry2->Size;
    }
    break;

    case 6:
    {
      return strcmp(entry1->MailFile, entry2->MailFile);
    }
    break;

    case 7:
    {
      return CmpTime(TIMEVAL(&entry2->transDate), TIMEVAL(&entry1->transDate));
    }
    break;

    case 8:
    {
      return stricmp(entry1->Folder->Name, entry2->Folder->Name);
    }
    break;
  }

  return 0;
}

///

/* Hooks */
/// FindAddressHook
HOOKPROTONHNO(FindAddressFunc, LONG, struct MUIP_NListtree_FindUserDataMessage *msg)
{
  struct ABEntry *entry = (struct ABEntry *)msg->UserData;
  return Stricmp(msg->User, entry->Address);
}
MakeStaticHook(FindAddressHook, FindAddressFunc);

///
/// CompareHook
//  Message listview sort hook
HOOKPROTONHNO(CompareFunc, LONG, struct NList_CompareMessage *ncm)
{
  struct Mail *entry1 = (struct Mail *)ncm->entry1;
  struct Mail *entry2 = (struct Mail *)ncm->entry2;
  LONG col1 = ncm->sort_type & MUIV_NList_TitleMark_ColMask;
  LONG col2 = ncm->sort_type2 & MUIV_NList_TitleMark2_ColMask;
  int cmp;

  if(ncm->sort_type == (LONG)MUIV_NList_SortType_None)
    return 0;

  if(ncm->sort_type & MUIV_NList_TitleMark_TypeMask) cmp = MailCompare(entry2, entry1, col1);
  else                                               cmp = MailCompare(entry1, entry2, col1);

  if(cmp || col1 == col2) return cmp;
  if(ncm->sort_type2 & MUIV_NList_TitleMark2_TypeMask) cmp = MailCompare(entry2, entry1, col2);
  else                                                 cmp = MailCompare(entry1, entry2, col2);

  return cmp;
}
MakeStaticHook(CompareHook, CompareFunc);

///
/// DisplayHook
HOOKPROTONH(DisplayFunc, LONG, Object *obj, struct NList_DisplayMessage *msg)
{
  struct Mail *entry;
  char **array;
  BOOL searchWinHook = FALSE;

  if(!msg)
    return 0;

  // now we set our local variables to the DisplayMessage structure ones
  entry = (struct Mail *)msg->entry;
  array = msg->strings;

  // now we check who is the parent of this DisplayHook
  if(G->FI && obj == G->FI->GUI.LV_MAILS)
  {
    searchWinHook = TRUE;
  }
  else if(!G->MA)
    return 0;

  if(entry)
  {
    if(entry->Folder)
    {
      static char dispsta[SIZE_DEFAULT];
      static char dispsiz[SIZE_SMALL];
      struct Person *pe;
      STRPTR addr;

      // prepare the status char buffer
      dispsta[0] = '\0';
      array[0] = dispsta;

      // first we check which main status this mail has
      // and put the leftmost mail icon accordingly.
      if(hasStatusError(entry) || isPartialMail(entry)) strlcat(dispsta, SICON_ERROR, sizeof(dispsta));
      else if(hasStatusQueued(entry))  strlcat(dispsta, SICON_WAITSEND, sizeof(dispsta));
      else if(hasStatusSent(entry))    strlcat(dispsta, SICON_SENT, sizeof(dispsta));
      else if(hasStatusRead(entry))    strlcat(dispsta, SICON_OLD, sizeof(dispsta));
      else                             strlcat(dispsta, SICON_UNREAD, sizeof(dispsta));

      // then we add the 2. level if icons with the additional mail information
      // like importance, signed/crypted, report and attachment information
      if(getImportanceLevel(entry) == IMP_HIGH)  strlcat(dispsta, SICON_URGENT, sizeof(dispsta));
      if(isMP_CryptedMail(entry))                strlcat(dispsta, SICON_CRYPT, sizeof(dispsta));
      else if(isMP_SignedMail(entry))            strlcat(dispsta, SICON_SIGNED, sizeof(dispsta));
      if(isMP_ReportMail(entry))                 strlcat(dispsta, SICON_REPORT, sizeof(dispsta));
      if(isMP_MixedMail(entry))                  strlcat(dispsta, SICON_ATTACH, sizeof(dispsta));

      // and as the 3rd level of icons we put information on the secondary status
      // like replied, forwarded, hold
      if(hasStatusNew(entry))        strlcat(dispsta, SICON_NEW, sizeof(dispsta));
      else if(hasStatusHold(entry))  strlcat(dispsta, SICON_HOLD, sizeof(dispsta));
      if(hasStatusMarked(entry))     strlcat(dispsta, SICON_MARK, sizeof(dispsta));
      if(hasStatusReplied(entry))    strlcat(dispsta, SICON_REPLY, sizeof(dispsta));
      if(hasStatusForwarded(entry))  strlcat(dispsta, SICON_FORWARD, sizeof(dispsta));

      // now we generate the proper string for the mailaddress
      if(C->MessageCols & (1<<1) || searchWinHook)
      {
        static char dispfro[SIZE_DEFAULT];

        array[1] = dispfro;

        if(isMultiRCPTMail(entry))
          strlcpy(dispfro, SICON_GROUP, sizeof(dispfro));
        else
          dispfro[0] = '\0';

        if(((entry->Folder->Type == FT_CUSTOMMIXED || entry->Folder->Type == FT_DELETED) &&
            (hasStatusSent(entry) || hasStatusQueued(entry) || hasStatusHold(entry) ||
             hasStatusError(entry))) || (searchWinHook && isOutgoingFolder(entry->Folder)))
        {
          pe = &entry->To;
          strlcat(dispfro, GetStr(MSG_MA_ToPrefix), sizeof(dispfro));
        }
        else
          pe = isOutgoingFolder(entry->Folder) ? &entry->To : &entry->From;

        #ifndef DISABLE_ADDRESSBOOK_LOOKUP
        {
          struct MUI_NListtree_TreeNode *tn;

          set(G->AB->GUI.LV_ADDRESSES, MUIA_NListtree_FindUserDataHook, &FindAddressHook);

          if((tn = (struct MUI_NListtree_TreeNode *)DoMethod(G->AB->GUI.LV_ADDRESSES, MUIM_NListtree_FindUserData, MUIV_NListtree_FindUserData_ListNode_Root, &pe->Address[0], MUIF_NONE)))
          {
            addr = ((struct ABEntry *)tn->tn_User)->RealName[0] ? ((struct ABEntry *)tn->tn_User)->RealName : AddrName((*pe));
          }
          else
            addr = AddrName((*pe));
        }
        #else
        addr = AddrName((*pe));
        #endif

        // lets put the string together
        strlcat(dispfro, addr, sizeof(dispfro)-strlen(dispfro)-1);
      }

      // lets set all other fields now
      if(!searchWinHook && C->MessageCols & (1<<2))
       array[2] = AddrName((entry->ReplyTo));

      // then the Subject
      array[3] = entry->Subject;

      // we first copy the Date Received/sent because this would probably be not
      // set by all ppl and strcpy() is costy ;)
      if((C->MessageCols & (1<<7) && entry->transDate.Seconds > 0) || searchWinHook)
      {
        static char datstr[64]; // we don`t use LEN_DATSTRING as OS3.1 anyway ignores it.
        TimeVal2String(datstr, sizeof(datstr), &entry->transDate, C->SwatchBeat ? DSS_DATEBEAT : DSS_DATETIME, TZC_LOCAL);
        array[7] = datstr;
      }
      else
        array[7] = "";

      if(C->MessageCols & (1<<4) || searchWinHook)
      {
        static char datstr[64];
        DateStamp2String(datstr, sizeof(datstr), &entry->Date, C->SwatchBeat ? DSS_DATEBEAT : DSS_DATETIME, TZC_LOCAL);
        array[4] = datstr;
      }

      if(C->MessageCols & (1<<5) || searchWinHook)
      {
        FormatSize(entry->Size, array[5] = dispsiz, sizeof(dispsiz));
      }

      array[6] = entry->MailFile;
      array[8] = entry->Folder->Name;

      // depending on the mail status we set the font to bold or plain
      if(hasStatusUnread(entry) || hasStatusNew(entry))
       msg->preparses[1] = msg->preparses[2] = msg->preparses[3] = msg->preparses[4] = msg->preparses[5] = MUIX_B;
    }
  }
  else
  {
    struct Folder *folder = NULL;

    // first we have to make sure that the mail window has a valid folder
    if(!searchWinHook && !(folder = FO_GetCurrentFolder()))
      return 0;

    array[0] = GetStr(MSG_MA_TitleStatus);

    // depending on the current folder and the parent object we
    // display different titles for different columns
    if(!searchWinHook && isOutgoingFolder(folder))
    {
      array[1] = GetStr(MSG_To);
      array[7] = GetStr(MSG_DATE_SENT);
    }
    else if(searchWinHook || folder->Type == FT_CUSTOMMIXED || folder->Type == FT_DELETED)
    {
      array[1] = GetStr(MSG_FROMTO);
      array[7] = GetStr(MSG_DATE_SNTRCVD);
    }
    else
    {
      array[1] = GetStr(MSG_From);
      array[7] = GetStr(MSG_DATE_RECEIVED);
    }

    array[2] = GetStr(MSG_ReturnAddress);
    array[3] = GetStr(MSG_Subject);
    array[4] = GetStr(MSG_Date);
    array[5] = GetStr(MSG_Size);
    array[6] = GetStr(MSG_Filename);

    array[8] = GetStr(MSG_Folder); // The Folder is just a dummy entry to serve the SearchWindowDisplayHook
  }

  return 0;
}
MakeStaticHook(DisplayHook, DisplayFunc);

///

/* Overloaded Methods */
/// OVERLOAD(OM_NEW)
OVERLOAD(OM_NEW)
{
  struct Data *data;
  int i;

  ENTER();

  if(!(obj = DoSuperNew(cl, obj,

    MUIA_Font,                       C->FixedFontList ? MUIV_NList_Font_Fixed : MUIV_NList_Font,

    MUIA_NList_MinColSortable,       0,
    MUIA_NList_TitleClick,           TRUE,
    MUIA_NList_TitleClick2,          TRUE,
    MUIA_NList_MultiSelect,          MUIV_NList_MultiSelect_Default,
    MUIA_NList_CompareHook2,         &CompareHook,
    MUIA_NList_DisplayHook2,         &DisplayHook,
    MUIA_NList_AutoVisible,          TRUE,
    MUIA_NList_Title,                TRUE,
    MUIA_NList_TitleSeparator,       TRUE,
    MUIA_NList_DefaultObjectOnClick, FALSE,
  
    TAG_MORE, inittags(msg))))
  {
    RETURN(0);
    return 0;
  }

  data = (struct Data *)INST_DATA(cl,obj);

  // prepare the mail status images
  data->statusImage[SICON_ID_UNREAD]   = MakeImageObject("status_unread");
  data->statusImage[SICON_ID_OLD]      = MakeImageObject("status_old");
  data->statusImage[SICON_ID_FORWARD]  = MakeImageObject("status_forward");
  data->statusImage[SICON_ID_REPLY]    = MakeImageObject("status_reply");
  data->statusImage[SICON_ID_WAITSEND] = MakeImageObject("status_waitsend");
  data->statusImage[SICON_ID_ERROR]    = MakeImageObject("status_error");
  data->statusImage[SICON_ID_HOLD]     = MakeImageObject("status_hold");
  data->statusImage[SICON_ID_SENT]     = MakeImageObject("status_sent");
  data->statusImage[SICON_ID_NEW]      = MakeImageObject("status_new");
  data->statusImage[SICON_ID_DELETE]   = MakeImageObject("status_delete");
  data->statusImage[SICON_ID_DOWNLOAD] = MakeImageObject("status_download");
  data->statusImage[SICON_ID_GROUP]    = MakeImageObject("status_group");
  data->statusImage[SICON_ID_URGENT]   = MakeImageObject("status_urgent");
  data->statusImage[SICON_ID_ATTACH]   = MakeImageObject("status_attach");
  data->statusImage[SICON_ID_REPORT]   = MakeImageObject("status_report");
  data->statusImage[SICON_ID_CRYPT]    = MakeImageObject("status_crypt");
  data->statusImage[SICON_ID_SIGNED]   = MakeImageObject("status_signed");
  data->statusImage[SICON_ID_MARK]     = MakeImageObject("status_mark");
  for(i=0; i < MAX_STATUSIMG; i++)
    DoMethod(obj, MUIM_NList_UseImage, data->statusImage[i], i, MUIF_NONE);

  // connect some notifies to the mainMailList group
  DoMethod(obj, MUIM_Notify, MUIA_NList_TitleClick,  MUIV_EveryTime, MUIV_Notify_Self, 4, MUIM_NList_Sort3, MUIV_TriggerValue,     MUIV_NList_SortTypeAdd_2Values, MUIV_NList_Sort3_SortType_Both);
  DoMethod(obj, MUIM_Notify, MUIA_NList_TitleClick2, MUIV_EveryTime, MUIV_Notify_Self, 4, MUIM_NList_Sort3, MUIV_TriggerValue,     MUIV_NList_SortTypeAdd_2Values, MUIV_NList_Sort3_SortType_2);
  DoMethod(obj, MUIM_Notify, MUIA_NList_SortType,    MUIV_EveryTime, MUIV_Notify_Self, 3, MUIM_Set,         MUIA_NList_TitleMark,  MUIV_TriggerValue);
  DoMethod(obj, MUIM_Notify, MUIA_NList_SortType2,   MUIV_EveryTime, MUIV_Notify_Self, 3, MUIM_Set,         MUIA_NList_TitleMark2, MUIV_TriggerValue);

  RETURN((ULONG)obj);
  return (ULONG)obj;
}

///
/// OVERLOAD(OM_DISPOSE)
OVERLOAD(OM_DISPOSE)
{
  GETDATA;
  int i;

  // make sure that our context menus are also disposed
  if(data->context_menu)
    MUI_DisposeObject(data->context_menu);

  for(i=0; i < MAX_STATUSIMG; i++)
  {
    DoMethod(obj, MUIM_NList_UseImage, NULL, i, MUIF_NONE);
    if(data->statusImage[i])
    {
      MUI_DisposeObject(data->statusImage[i]);
      data->statusImage[i] = NULL;
    }
  }

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
  DoMethod(obj, MUIM_NList_TestPos, m->mx, m->my, &res);

  if(res.entry >= 0)
  {
    int i;
    
    DoMethod(obj, MUIM_NList_GetEntry, res.entry, &mail);
    if(!mail)
      return(0);

    fo->LastActive = xget(obj, MUIA_NList_Active);

    if(isMultiPartMail(mail))
      hasattach = TRUE;

    for (i = 0; i < MAXWR; i++)
    {
      if (G->WR[i] && G->WR[i]->Mail == mail) beingedited = TRUE;
    }

    // Now we set this entry as activ
    if(fo->LastActive != res.entry) set(obj, MUIA_NList_Active, res.entry);
  }

  // now we create the menu title of the context menu
  if(mail)
  {
    struct Person *pers = isOutBox ? &mail->To : &mail->From;

    snprintf(menutitle, sizeof(menutitle), "%s: ", GetStr(isOutBox ? MSG_To : MSG_From));
    strlcat(menutitle, BuildAddrName2(pers), 20-strlen(menutitle) > 0 ? 20-strlen(menutitle) : 0);
    strlcat(menutitle, "...", sizeof(menutitle));
  }
  else
    strlcpy(menutitle, GetStr(MSG_MAIL_NONSEL), sizeof(menutitle));

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
        Child, MenuitemObject, MUIA_Menuitem_Title, NM_BARLABEL, End,
        Child, MenuitemObject, MUIA_Menuitem_Title, GetStripStr(MSG_MA_ALLTOREAD),  MUIA_Menuitem_Enabled, mail && !isOutBox,  MUIA_UserData, MMEN_ALLTOREAD,  End,
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

      DoMethod(obj, MUIM_MainMailList_MakeFormat);
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
    case MMEN_TOUNREAD:   DoMethod(G->App, MUIM_CallHook, &MA_SetStatusToHook,    SFLAG_NONE,             SFLAG_NEW|SFLAG_READ);              break;
    case MMEN_TOREAD:     DoMethod(G->App, MUIM_CallHook, &MA_SetStatusToHook,    SFLAG_READ,             SFLAG_NEW);                         break;
    case MMEN_TOHOLD:     DoMethod(G->App, MUIM_CallHook, &MA_SetStatusToHook,    SFLAG_HOLD|SFLAG_READ,  SFLAG_QUEUED|SFLAG_ERROR);          break;
    case MMEN_TOQUEUED:   DoMethod(G->App, MUIM_CallHook, &MA_SetStatusToHook,    SFLAG_QUEUED|SFLAG_READ,SFLAG_SENT|SFLAG_HOLD|SFLAG_ERROR); break;
    case MMEN_TOMARKED:   DoMethod(G->App, MUIM_CallHook, &MA_SetStatusToHook,    SFLAG_MARKED,           SFLAG_NONE);                        break;
    case MMEN_TOUNMARKED: DoMethod(G->App, MUIM_CallHook, &MA_SetStatusToHook,    SFLAG_NONE,             SFLAG_MARKED);                      break;
    case MMEN_ALLTOREAD:  DoMethod(G->App, MUIM_CallHook, &MA_SetAllStatusToHook, SFLAG_READ,             SFLAG_NEW);                         break;
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
    case MMEN_SELALL:     DoMethod(obj, MUIM_NList_Select, MUIV_NList_Select_All, MUIV_NList_Select_On,     NULL); break;
    case MMEN_SELNONE:    DoMethod(obj, MUIM_NList_Select, MUIV_NList_Select_All, MUIV_NList_Select_Off,    NULL); break;
    case MMEN_SELTOGG:    DoMethod(obj, MUIM_NList_Select, MUIV_NList_Select_All, MUIV_NList_Select_Toggle, NULL); break;

    default:
    {
      return DoSuperMethodA(cl, obj, msg);
    }
  }

  return 0;
}

///

/* Public Methods */
/// DECLARE(MakeFormat)
//  Creates format definition for message listview
DECLARE(MakeFormat)
{
  static const int defwidth[MACOLNUM] = { -1,-1,-1,-1,-1,-1,-1,-1 };
  char format[SIZE_LARGE];
  BOOL first = TRUE;
  int i;

  *format = '\0';

  for(i = 0; i < MACOLNUM; i++)
  {
    if(C->MessageCols & (1<<i))
    {
      int p;

      if(first)
        first = FALSE;
      else
        strlcat(format, " BAR,", sizeof(format));

      p = strlen(format);
      snprintf(&format[p], sizeof(format)-p, "COL=%d W=%d", i, defwidth[i]);

      if(i == 5)
        strlcat(format, " P=\033r", sizeof(format));
    }
  }
  strlcat(format, " BAR", sizeof(format));

  // set the new NList_Format to our object
  set(obj, MUIA_NList_Format, format);

  return 0;
}

///

