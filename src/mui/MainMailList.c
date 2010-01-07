/***************************************************************************

 YAM - Yet Another Mailer
 Copyright (C) 1995-2000 by Marcel Beck <mbeck@yam.ch>
 Copyright (C) 2000-2010 by YAM Open Source Team

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

#include "YAM_addressbook.h"
#include "YAM_addressbookEntry.h"
#include "YAM_config.h"
#include "YAM_find.h"
#include "YAM_mainFolder.h"

#include "BayesFilter.h"
#include "MUIObjects.h"
#include "Themes.h"

#include "Debug.h"

/* CLASSDATA
struct Data
{
  Object *context_menu;
  Object *statusImage[si_Max];
  char context_menu_title[SIZE_DEFAULT];
};
*/

/* local prototypes */
static int MailCompare(struct Mail *entry1, struct Mail *entry2, LONG column);

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
  if(msg != NULL && G->MA != NULL)
  {
    struct Mail *entry;
    char **array;
    BOOL searchWinHook = FALSE;

    // now we set our local variables to the DisplayMessage structure ones
    entry = (struct Mail *)msg->entry;
    array = msg->strings;

    // now we check who is the parent of this DisplayHook
    if(G->FI != NULL && obj == G->FI->GUI.LV_MAILS)
      searchWinHook = TRUE;

    if(entry != NULL)
    {
      if(entry->Folder != NULL)
      {
        static char dispsta[SIZE_DEFAULT];
        static char dispsiz[SIZE_SMALL];

        // prepare the status char buffer
        dispsta[0] = '\0';
        array[0] = dispsta;

        // first we check which main status this mail has
        // and put the leftmost mail icon accordingly.
        if(hasStatusError(entry) || isPartialMail(entry)) strlcat(dispsta, SI_STR(si_Error), sizeof(dispsta));
        else if(hasStatusQueued(entry))  strlcat(dispsta, SI_STR(si_WaitSend), sizeof(dispsta));
        else if(hasStatusSent(entry))    strlcat(dispsta, SI_STR(si_Sent), sizeof(dispsta));
        else if(hasStatusNew(entry))     strlcat(dispsta, SI_STR(si_New), sizeof(dispsta));
        else if(hasStatusHold(entry))    strlcat(dispsta, SI_STR(si_Hold), sizeof(dispsta));
        else if(hasStatusRead(entry))    strlcat(dispsta, SI_STR(si_Old), sizeof(dispsta));
        else                             strlcat(dispsta, SI_STR(si_Unread), sizeof(dispsta));

        // then we add the 2. level if icons with the additional mail information
        // like importance, signed/crypted, report and attachment information
        if(C->SpamFilterEnabled == TRUE && hasStatusSpam(entry)) strlcat(dispsta, SI_STR(si_Spam), sizeof(dispsta));
        if(getImportanceLevel(entry) == IMP_HIGH)  strlcat(dispsta, SI_STR(si_Urgent), sizeof(dispsta));
        if(isMP_CryptedMail(entry))                strlcat(dispsta, SI_STR(si_Crypt), sizeof(dispsta));
        else if(isMP_SignedMail(entry))            strlcat(dispsta, SI_STR(si_Signed), sizeof(dispsta));
        if(isMP_ReportMail(entry))                 strlcat(dispsta, SI_STR(si_Report), sizeof(dispsta));
        if(isMP_MixedMail(entry))                  strlcat(dispsta, SI_STR(si_Attach), sizeof(dispsta));

        // and as the 3rd level of icons we put information on the secondary status
        // like marked, replied, forwarded
        if(hasStatusMarked(entry))     strlcat(dispsta, SI_STR(si_Mark), sizeof(dispsta));
        if(hasStatusReplied(entry))    strlcat(dispsta, SI_STR(si_Reply), sizeof(dispsta));
        if(hasStatusForwarded(entry))  strlcat(dispsta, SI_STR(si_Forward), sizeof(dispsta));

        // now we generate the proper string for the mailaddress
        if(hasMColSender(C->MessageCols) || searchWinHook == TRUE)
        {
          static char dispfro[SIZE_DEFAULT];
          BOOL toPrefix = FALSE;
          struct Person *pe;
          char *addr = NULL;

          if(((isCustomMixedFolder(entry->Folder) || isTrashFolder(entry->Folder) || isSpamFolder(entry->Folder)) &&
              (hasStatusSent(entry) || hasStatusQueued(entry) || hasStatusHold(entry) ||
               hasStatusError(entry))) || (searchWinHook && isSentMailFolder(entry->Folder)))
          {
            pe = &entry->To;

            // put a To: prefix before our sender name
            toPrefix = TRUE;
          }
          else
            pe = isSentMailFolder(entry->Folder) ? &entry->To : &entry->From;

          // in case the user wants to take the additional pain
          // of performing an addressbook lookup for every entry in the
          // list we do it right here.
          if(C->ABookLookup == TRUE)
          {
            struct MUI_NListtree_TreeNode *tn;

            set(G->AB->GUI.LV_ADDRESSES, MUIA_NListtree_FindUserDataHook, &FindAddressHook);

            if((tn = (struct MUI_NListtree_TreeNode *)DoMethod(G->AB->GUI.LV_ADDRESSES, MUIM_NListtree_FindUserData, MUIV_NListtree_FindUserData_ListNode_Root, &pe->Address[0], MUIF_NONE)) != NULL)
            {
              struct ABEntry *ab = (struct ABEntry *)tn->tn_User;

              if(ab->RealName[0] != '\0')
                addr = ab->RealName;
            }
          }

          // if we didn't perform an address book lookup then we
          // extract the address from the given information
          if(addr == NULL)
            addr = AddrName(*pe);

          // lets put the string together
          snprintf(dispfro, sizeof(dispfro), "%s%s%s%s", isMultiRCPTMail(entry) ? SI_STR(si_Group) : "",
                                                         toPrefix ? tr(MSG_MA_ToPrefix) : "",
                                                         addr,
                                                         isMultiSenderMail(entry) && toPrefix == FALSE ? ", ..." : "");

          array[1] = dispfro;
        }

        // lets set all other fields now
        if(searchWinHook == FALSE && hasMColReplyTo(C->MessageCols))
        {
          if(isMultiReplyToMail(entry))
          {
            static char dispreplyto[SIZE_DEFAULT];

            snprintf(dispreplyto, sizeof(dispreplyto), "%s, ...", AddrName(entry->ReplyTo));
            array[2] = dispreplyto;
          }
          else
            array[2] = AddrName(entry->ReplyTo);
        }

        // then the Subject
        array[3] = entry->Subject;

        // we first copy the Date Received/sent because this would probably be not
        // set by all ppl and strcpy() is costy ;)
        if((hasMColTransDate(C->MessageCols) && entry->transDate.Seconds > 0) || searchWinHook == TRUE)
        {
          static char datstr[64]; // we don't use LEN_DATSTRING as OS3.1 anyway ignores it.

          TimeVal2String(datstr, sizeof(datstr), &entry->transDate, C->DSListFormat, TZC_LOCAL);
          array[7] = datstr;
        }
        else
          array[7] = (STRPTR)"";

        if(hasMColDate(C->MessageCols) || searchWinHook == TRUE)
        {
          static char datstr[64]; // we don't use LEN_DATSTRING as OS3.1 anyway ignores it.

          DateStamp2String(datstr, sizeof(datstr), &entry->Date, C->DSListFormat, TZC_LOCAL);
          array[4] = datstr;
        }

        if(hasMColSize(C->MessageCols) || searchWinHook == TRUE)
          FormatSize(entry->Size, array[5] = dispsiz, sizeof(dispsiz), SF_AUTO);

        array[6] = entry->MailFile;
        array[8] = entry->Folder->Name;

        // depending on the mail status we set the font to bold or plain
        if(hasStatusUnread(entry) || hasStatusNew(entry))
          msg->preparses[1] = msg->preparses[2] = msg->preparses[3] = msg->preparses[4] = msg->preparses[5] = C->StyleMailUnread;
        else
          msg->preparses[1] = msg->preparses[2] = msg->preparses[3] = msg->preparses[4] = msg->preparses[5] = C->StyleMailRead;
      }
    }
    else
    {
      struct Folder *folder = FO_GetCurrentFolder();

      // first we have to make sure that the mail window has a valid folder
      if(searchWinHook == TRUE || folder != NULL)
      {
        array[0] = (STRPTR)tr(MSG_MA_TitleStatus);

        // depending on the current folder and the parent object we
        // display different titles for different columns
        if(searchWinHook == FALSE && isSentMailFolder(folder))
        {
          array[1] = (STRPTR)tr(MSG_To);
          array[7] = (STRPTR)tr(MSG_DATE_SENT);
        }
        else if(searchWinHook == TRUE || isCustomMixedFolder(folder) || isTrashFolder(folder) || isSpamFolder(folder))
        {
          array[1] = (STRPTR)tr(MSG_FROMTO);
          array[7] = (STRPTR)tr(MSG_DATE_SNTRCVD);
        }
        else
        {
          array[1] = (STRPTR)tr(MSG_From);
          array[7] = (STRPTR)tr(MSG_DATE_RECEIVED);
        }

        array[2] = (STRPTR)tr(MSG_ReturnAddress);
        array[3] = (STRPTR)tr(MSG_Subject);
        array[4] = (STRPTR)tr(MSG_Date);
        array[5] = (STRPTR)tr(MSG_Size);
        array[6] = (STRPTR)tr(MSG_Filename);

        // The Folder is just a dummy entry to serve the SearchWindowDisplayHook
        array[8] = (STRPTR)tr(MSG_Folder);
      }
    }
  }

  return 0;
}
MakeStaticHook(DisplayHook, DisplayFunc);

///

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
      struct Person *pe1;
      struct Person *pe2;
      char *addr1;
      char *addr2;

      if(isSentMailFolder(entry1->Folder))
      {
        pe1 = &entry1->To;
        pe2 = &entry2->To;
      }
      else
      {
        pe1 = &entry1->From;
        pe2 = &entry2->From;
      }

      // in case the user wants to take the additional pain
      // of performing an addressbook lookup for every entry in the
      // list we do it right here.
      if(C->ABookLookup)
      {
        struct MUI_NListtree_TreeNode *tn1;
        struct MUI_NListtree_TreeNode *tn2;

        set(G->AB->GUI.LV_ADDRESSES, MUIA_NListtree_FindUserDataHook, &FindAddressHook);

        if((tn1 = (struct MUI_NListtree_TreeNode *)DoMethod(G->AB->GUI.LV_ADDRESSES, MUIM_NListtree_FindUserData, MUIV_NListtree_FindUserData_ListNode_Root, &pe1->Address[0], MUIF_NONE)))
          addr1 = ((struct ABEntry *)tn1->tn_User)->RealName[0] ? ((struct ABEntry *)tn1->tn_User)->RealName : AddrName((*pe1));
        else
          addr1 = AddrName(*pe1);

        if((tn2 = (struct MUI_NListtree_TreeNode *)DoMethod(G->AB->GUI.LV_ADDRESSES, MUIM_NListtree_FindUserData, MUIV_NListtree_FindUserData_ListNode_Root, &pe2->Address[0], MUIF_NONE)))
          addr2 = ((struct ABEntry *)tn2->tn_User)->RealName[0] ? ((struct ABEntry *)tn2->tn_User)->RealName : AddrName((*pe2));
        else
          addr2 = AddrName(*pe2);
      }
      else
      {
        addr1 = AddrName(*pe1);
        addr2 = AddrName(*pe2);
      }

      return stricmp(addr1, addr2);
    }
    break;

    case 2:
    {
      return stricmp(AddrName(entry1->ReplyTo), AddrName(entry2->ReplyTo));
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

/* Overloaded Methods */
/// OVERLOAD(OM_NEW)
OVERLOAD(OM_NEW)
{
  struct Data *data;
  ULONG i;

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
    MUIA_NList_ActiveObjectOnClick,  TRUE,

    TAG_MORE, inittags(msg))))
  {
    RETURN(0);
    return 0;
  }

  data = (struct Data *)INST_DATA(cl,obj);

  // prepare the mail status images
  data->statusImage[si_Attach]   = MakeImageObject("status_attach",   G->theme.statusImages[si_Attach]);
  data->statusImage[si_Crypt]    = MakeImageObject("status_crypt",    G->theme.statusImages[si_Crypt]);
  data->statusImage[si_Delete]   = MakeImageObject("status_delete",   G->theme.statusImages[si_Delete]);
  data->statusImage[si_Download] = MakeImageObject("status_download", G->theme.statusImages[si_Download]);
  data->statusImage[si_Error]    = MakeImageObject("status_error",    G->theme.statusImages[si_Error]);
  data->statusImage[si_Forward]  = MakeImageObject("status_forward",  G->theme.statusImages[si_Forward]);
  data->statusImage[si_Group]    = MakeImageObject("status_group",    G->theme.statusImages[si_Group]);
  data->statusImage[si_Hold]     = MakeImageObject("status_hold",     G->theme.statusImages[si_Hold]);
  data->statusImage[si_Mark]     = MakeImageObject("status_mark",     G->theme.statusImages[si_Mark]);
  data->statusImage[si_New]      = MakeImageObject("status_new",      G->theme.statusImages[si_New]);
  data->statusImage[si_Old]      = MakeImageObject("status_old",      G->theme.statusImages[si_Old]);
  data->statusImage[si_Reply]    = MakeImageObject("status_reply",    G->theme.statusImages[si_Reply]);
  data->statusImage[si_Report]   = MakeImageObject("status_report",   G->theme.statusImages[si_Report]);
  data->statusImage[si_Sent]     = MakeImageObject("status_sent",     G->theme.statusImages[si_Sent]);
  data->statusImage[si_Signed]   = MakeImageObject("status_signed",   G->theme.statusImages[si_Signed]);
  data->statusImage[si_Spam]     = MakeImageObject("status_spam",     G->theme.statusImages[si_Spam]);
  data->statusImage[si_Unread]   = MakeImageObject("status_unread",   G->theme.statusImages[si_Unread]);
  data->statusImage[si_Urgent]   = MakeImageObject("status_urgent",   G->theme.statusImages[si_Urgent]);
  data->statusImage[si_WaitSend] = MakeImageObject("status_waitsend", G->theme.statusImages[si_WaitSend]);
  for(i = 0; i < si_Max; i++)
  {
    if(data->statusImage[i] != NULL)
      DoMethod(obj, MUIM_NList_UseImage, data->statusImage[i], i, MUIF_NONE);
  }

  // connect some notifies to the mainMailList group
  DoMethod(obj, MUIM_Notify, MUIA_NList_TitleClick,  MUIV_EveryTime, MUIV_Notify_Self, 4, MUIM_NList_Sort3, MUIV_TriggerValue,     MUIV_NList_SortTypeAdd_2Values, MUIV_NList_Sort3_SortType_Both);
  DoMethod(obj, MUIM_Notify, MUIA_NList_TitleClick2, MUIV_EveryTime, MUIV_Notify_Self, 4, MUIM_NList_Sort3, MUIV_TriggerValue,     MUIV_NList_SortTypeAdd_2Values, MUIV_NList_Sort3_SortType_2);
  DoMethod(obj, MUIM_Notify, MUIA_NList_SortType,    MUIV_EveryTime, MUIV_Notify_Self, 3, MUIM_Set,         MUIA_NList_TitleMark,  MUIV_TriggerValue);
  DoMethod(obj, MUIM_Notify, MUIA_NList_SortType2,   MUIV_EveryTime, MUIV_Notify_Self, 3, MUIM_Set,         MUIA_NList_TitleMark2, MUIV_TriggerValue);

  RETURN((IPTR)obj);
  return (IPTR)obj;
}

///
/// OVERLOAD(OM_DISPOSE)
OVERLOAD(OM_DISPOSE)
{
  GETDATA;
  int i;

  // make sure that our context menus are also disposed
  if(data->context_menu != NULL)
    MUI_DisposeObject(data->context_menu);

  for(i=0; i < si_Max; i++)
  {
    DoMethod(obj, MUIM_NList_UseImage, NULL, i, MUIF_NONE);
    if(data->statusImage[i] != NULL)
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
  struct MUI_NList_TestPos_Result res;
  struct Mail *mail = NULL;
  struct Folder *fo = FO_GetCurrentFolder();
  BOOL isOutBox = isOutgoingFolder(fo);
  BOOL isSentMail = isSentMailFolder(fo);
  BOOL hasattach = FALSE;
  Object *afterThis;

  // dispose the old context_menu if it still exists
  if(data->context_menu != NULL)
  {
    MUI_DisposeObject(data->context_menu);
    data->context_menu = NULL;
  }

  // if this was a RMB click on the titlebar we create our own special menu
  if(m->ontop)
  {
    data->context_menu = MenustripObject,
      Child, MenuObjectT(tr(MSG_MA_CTX_MAILLIST)),
        Child, MenuitemObject, MUIA_Menuitem_Title, tr(MSG_Status),         MUIA_UserData, 1, MUIA_Menuitem_Enabled, FALSE, MUIA_Menuitem_Checked, TRUE, MUIA_Menuitem_Checkit, TRUE, MUIA_Menuitem_Toggle, TRUE, End,
        Child, MenuitemObject, MUIA_Menuitem_Title, tr(MSG_SenderRecpt),    MUIA_UserData, 2, MUIA_Menuitem_Checked, hasMColSender(C->MessageCols),  MUIA_Menuitem_Checkit, TRUE, MUIA_Menuitem_Toggle, TRUE, End,
        Child, MenuitemObject, MUIA_Menuitem_Title, tr(MSG_ReturnAddress),  MUIA_UserData, 3, MUIA_Menuitem_Checked, hasMColReplyTo(C->MessageCols), MUIA_Menuitem_Checkit, TRUE, MUIA_Menuitem_Toggle, TRUE, End,
        Child, MenuitemObject, MUIA_Menuitem_Title, tr(MSG_Subject),        MUIA_UserData, 4, MUIA_Menuitem_Checked, hasMColSubject(C->MessageCols), MUIA_Menuitem_Checkit, TRUE, MUIA_Menuitem_Toggle, TRUE, End,
        Child, MenuitemObject, MUIA_Menuitem_Title, tr(MSG_MessageDate),    MUIA_UserData, 5, MUIA_Menuitem_Checked, hasMColDate(C->MessageCols),    MUIA_Menuitem_Checkit, TRUE, MUIA_Menuitem_Toggle, TRUE, End,
        Child, MenuitemObject, MUIA_Menuitem_Title, tr(MSG_Size),           MUIA_UserData, 6, MUIA_Menuitem_Checked, hasMColSize(C->MessageCols),    MUIA_Menuitem_Checkit, TRUE, MUIA_Menuitem_Toggle, TRUE, End,
        Child, MenuitemObject, MUIA_Menuitem_Title, tr(MSG_Filename),       MUIA_UserData, 7, MUIA_Menuitem_Checked, hasMColFilename(C->MessageCols),MUIA_Menuitem_Checkit, TRUE, MUIA_Menuitem_Toggle, TRUE, End,
        Child, MenuitemObject, MUIA_Menuitem_Title, tr(MSG_CO_DATE_SNTRCVD),MUIA_UserData, 8, MUIA_Menuitem_Checked, hasMColTransDate(C->MessageCols), MUIA_Menuitem_Checkit, TRUE, MUIA_Menuitem_Toggle, TRUE, End,
        Child, MenuitemObject, MUIA_Menuitem_Title, NM_BARLABEL, End,
        Child, MenuitemObject, MUIA_Menuitem_Title, tr(MSG_MA_CTX_DEFWIDTH_THIS), MUIA_UserData, MUIV_NList_Menu_DefWidth_This, End,
        Child, MenuitemObject, MUIA_Menuitem_Title, tr(MSG_MA_CTX_DEFWIDTH_ALL),  MUIA_UserData, MUIV_NList_Menu_DefWidth_All,  End,
        Child, MenuitemObject, MUIA_Menuitem_Title, tr(MSG_MA_CTX_DEFORDER_THIS), MUIA_UserData, MUIV_NList_Menu_DefOrder_This, End,
        Child, MenuitemObject, MUIA_Menuitem_Title, tr(MSG_MA_CTX_DEFORDER_ALL),  MUIA_UserData, MUIV_NList_Menu_DefOrder_All,  End,
      End,
    End;

    return (ULONG)data->context_menu;
  }

  if(fo == NULL)
    return(0);

  // Now lets find out which entry is under the mouse pointer
  DoMethod(obj, MUIM_NList_TestPos, m->mx, m->my, &res);

  if(res.entry >= 0)
  {
    DoMethod(obj, MUIM_NList_GetEntry, res.entry, &mail);
    if(mail == NULL)
      return(0);

    fo->LastActive = xget(obj, MUIA_NList_Active);

    if(isMultiPartMail(mail))
      hasattach = TRUE;

    // Now we set this entry as activ
    if(fo->LastActive != res.entry)
      set(obj, MUIA_NList_Active, res.entry);
  }

  // now we create the menu title of the context menu
  if(mail != NULL)
  {
    struct Person *pers = isSentMail ? &mail->To : &mail->From;
    char address[SIZE_LARGE];

    snprintf(data->context_menu_title, sizeof(data->context_menu_title), "%s: ", tr(isSentMail ? MSG_To : MSG_From));
    strlcat(data->context_menu_title, BuildAddress(address, sizeof(address), pers->Address, pers->RealName), 20-strlen(data->context_menu_title) > 0 ? 20-strlen(data->context_menu_title) : 0);
    strlcat(data->context_menu_title, "...", sizeof(data->context_menu_title));
  }
  else
    strlcpy(data->context_menu_title, tr(MSG_MAIL_NONSEL), sizeof(data->context_menu_title));

  data->context_menu = MenustripObject,
    Child, MenuObjectT(data->context_menu_title),
      Child, MenuitemObject, MUIA_Menuitem_Title, tr(MSG_MA_MREAD),             MUIA_Menuitem_Enabled, mail,               MUIA_UserData, MMEN_READ,       End,
      Child, MenuitemObject, MUIA_Menuitem_Title, isOutBox ? tr(MSG_MA_MEDIT) : tr(MSG_MA_MEDITASNEW), MUIA_Menuitem_Enabled, mail,               MUIA_UserData, MMEN_EDIT,       End,
      Child, MenuitemObject, MUIA_Menuitem_Title, tr(MSG_MA_MMOVE),             MUIA_Menuitem_Enabled, mail, MUIA_UserData, MMEN_MOVE,     End,
      Child, MenuitemObject, MUIA_Menuitem_Title, tr(MSG_MA_MCOPY),             MUIA_Menuitem_Enabled, mail, MUIA_UserData, MMEN_COPY,     End,
      Child, MenuitemObject, MUIA_Menuitem_Title, tr(MSG_MA_MDelete),           MUIA_Menuitem_Enabled, mail, MUIA_UserData, MMEN_DELETE,   End,
      Child, MenuitemObject, MUIA_Menuitem_Title, NM_BARLABEL, End,
      Child, MenuitemObject, MUIA_Menuitem_Title, tr(MSG_MA_MPRINT),          MUIA_Menuitem_Enabled, mail, MUIA_UserData, MMEN_PRINT,    End,
      Child, MenuitemObject, MUIA_Menuitem_Title, tr(MSG_MA_MSAVE),           MUIA_Menuitem_Enabled, mail, MUIA_UserData, MMEN_SAVE,     End,
      Child, MenuitemObject, MUIA_Menuitem_Title, tr(MSG_Attachments),        MUIA_Menuitem_Enabled, mail && hasattach,
        Child, MenuitemObject, MUIA_Menuitem_Title, tr(MSG_MA_MSAVEATT),      MUIA_Menuitem_Enabled, mail && hasattach, MUIA_UserData, MMEN_DETACH, End,
        Child, MenuitemObject, MUIA_Menuitem_Title, tr(MSG_MA_MDELETEATT),    MUIA_Menuitem_Enabled, mail && hasattach, MUIA_UserData, MMEN_DELETEATT, End,
      End,
      Child, MenuitemObject, MUIA_Menuitem_Title, tr(MSG_MESSAGE_EXPORT),     MUIA_Menuitem_Enabled, mail, MUIA_UserData, MMEN_EXPMSG,   End,
      Child, MenuitemObject, MUIA_Menuitem_Title, NM_BARLABEL, End,
      Child, MenuitemObject, MUIA_Menuitem_Title, tr(MSG_MA_MNEW),            MUIA_Menuitem_Enabled, mail, MUIA_UserData, MMEN_NEW,      End,
      Child, MenuitemObject, MUIA_Menuitem_Title, tr(MSG_MA_MREPLY),            MUIA_Menuitem_Enabled, mail,               MUIA_UserData, MMEN_REPLY,      End,
      Child, MenuitemObject, MUIA_Menuitem_Title, tr(MSG_MA_MFORWARD),          MUIA_Menuitem_Enabled, mail,               MUIA_UserData, MMEN_FORWARD,    End,
      Child, MenuitemObject, MUIA_Menuitem_Title, tr(MSG_MA_MBOUNCE),           MUIA_Menuitem_Enabled, mail,               MUIA_UserData, MMEN_BOUNCE,     End,
      Child, MenuitemObject, MUIA_Menuitem_Title, NM_BARLABEL, End,
      Child, MenuitemObject, MUIA_Menuitem_Title, tr(MSG_MA_MSAVEADDRESS),    MUIA_Menuitem_Enabled, mail, MUIA_UserData, MMEN_SAVEADDR, End,
      Child, MenuitemObject, MUIA_Menuitem_Title, tr(MSG_MA_Select),
        Child, MenuitemObject, MUIA_Menuitem_Title, tr(MSG_MA_SELECTALL),     MUIA_UserData, MMEN_SELALL,  End,
        Child, MenuitemObject, MUIA_Menuitem_Title, tr(MSG_MA_SELECTNONE),    MUIA_UserData, MMEN_SELNONE, End,
        Child, MenuitemObject, MUIA_Menuitem_Title, tr(MSG_MA_SELECTTOGGLE),  MUIA_UserData, MMEN_SELTOGG, End,
      End,
      Child, MenuitemObject, MUIA_Menuitem_Title, tr(MSG_MA_SetStatus),         MUIA_Menuitem_Enabled, mail,
        Child, MenuitemObject, MUIA_Menuitem_Title, tr(MSG_MA_TOMARKED),        MUIA_Menuitem_Enabled, mail && !hasStatusMarked(mail), MUIA_UserData, MMEN_TOMARKED,   End,
        Child, MenuitemObject, MUIA_Menuitem_Title, tr(MSG_MA_TOUNMARKED),      MUIA_Menuitem_Enabled, mail &&  hasStatusMarked(mail), MUIA_UserData, MMEN_TOUNMARKED, End,
        Child, MenuitemObject, MUIA_Menuitem_Title, tr(MSG_MA_TOREAD),          MUIA_Menuitem_Enabled, mail && !isSentMail && (hasStatusNew(mail) || hasStatusUnread(mail)), MUIA_UserData, MMEN_TOREAD,     End,
        Child, MenuitemObject, MUIA_Menuitem_Title, tr(MSG_MA_TOUNREAD),        MUIA_Menuitem_Enabled, mail && !isSentMail && hasStatusRead(mail), MUIA_UserData, MMEN_TOUNREAD,   End,
        Child, MenuitemObject, MUIA_Menuitem_Title, tr(MSG_MA_TOHOLD),          MUIA_Menuitem_Enabled, mail && isOutBox && !hasStatusHold(mail), MUIA_UserData, MMEN_TOHOLD,     End,
        Child, afterThis = MenuitemObject, MUIA_Menuitem_Title, tr(MSG_MA_TOQUEUED),   MUIA_Menuitem_Enabled, mail && isOutBox && !hasStatusQueued(mail), MUIA_UserData, MMEN_TOQUEUED,   End,
        Child, MenuitemObject, MUIA_Menuitem_Title, NM_BARLABEL, End,
        Child, MenuitemObject, MUIA_Menuitem_Title, tr(MSG_MA_ALLTOREAD),       MUIA_Menuitem_Enabled, mail && !isSentMail,  MUIA_UserData, MMEN_ALLTOREAD,  End,
      End,
      Child, MenuitemObject, MUIA_Menuitem_Title, tr(MSG_MA_ChangeSubj),        MUIA_Menuitem_Enabled, mail,               MUIA_UserData, MMEN_CHSUBJ,     End,
      Child, MenuitemObject, MUIA_Menuitem_Title, NM_BARLABEL, End,
      Child, MenuitemObject, MUIA_Menuitem_Title, tr(MSG_MA_MSend),             MUIA_Menuitem_Enabled, mail && isOutBox,   MUIA_UserData, MMEN_SEND, End,
    End,
  End;

  if(data->context_menu != NULL && mail != NULL && C->SpamFilterEnabled == TRUE)
  {
    Object *spamItem;
    Object *hamItem;

    spamItem = MenuitemObject, MUIA_Menuitem_Title, tr(MSG_MA_TOSPAM),    MUIA_Menuitem_Enabled, !hasStatusSpam(mail),  MUIA_UserData, MMEN_TOSPAM,   End;
    hamItem =  MenuitemObject, MUIA_Menuitem_Title, tr(MSG_MA_TONOTSPAM), MUIA_Menuitem_Enabled, hasStatusSpam(mail),   MUIA_UserData, MMEN_TOHAM,    End;

    DoMethod(data->context_menu, MUIM_Family_Insert, hamItem, afterThis);
    DoMethod(data->context_menu, MUIM_Family_Insert, spamItem, afterThis);
  }

  return (IPTR)data->context_menu;
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
      ULONG flag = (1 << (xget(m->item, MUIA_UserData)-1));

      if(isFlagSet(C->MessageCols, flag))
        CLEAR_FLAG(C->MessageCols, flag);
      else
        SET_FLAG(C->MessageCols, flag);

      DoMethod(obj, MUIM_MainMailList_MakeFormat);
    }
    break;

    // or other item out of the MailListContextMenu
    case MMEN_READ:       DoMethod(G->App, MUIM_CallHook, &MA_ReadMessageHook); break;
    case MMEN_EDIT:       DoMethod(G->App, MUIM_CallHook, &MA_NewMessageHook,     NMM_EDIT,    0); break;
    case MMEN_REPLY:      DoMethod(G->App, MUIM_CallHook, &MA_NewMessageHook,     NMM_REPLY,   0); break;
    case MMEN_FORWARD:    DoMethod(G->App, MUIM_CallHook, &MA_NewMessageHook,     NMM_FORWARD, 0); break;
    case MMEN_BOUNCE:     DoMethod(G->App, MUIM_CallHook, &MA_NewMessageHook,     NMM_BOUNCE,  0); break;
    case MMEN_SEND:       DoMethod(G->App, MUIM_CallHook, &MA_SendHook,           SEND_ACTIVE_USER); break;
    case MMEN_CHSUBJ:     DoMethod(G->App, MUIM_CallHook, &MA_ChangeSubjectHook); break;
    case MMEN_TOUNREAD:   DoMethod(G->App, MUIM_CallHook, &MA_SetStatusToHook,    SFLAG_NONE,             SFLAG_NEW|SFLAG_READ);              break;
    case MMEN_TOREAD:     DoMethod(G->App, MUIM_CallHook, &MA_SetStatusToHook,    SFLAG_READ,             SFLAG_NEW);                         break;
    case MMEN_TOHOLD:     DoMethod(G->App, MUIM_CallHook, &MA_SetStatusToHook,    SFLAG_HOLD|SFLAG_READ,  SFLAG_QUEUED|SFLAG_ERROR);          break;
    case MMEN_TOQUEUED:   DoMethod(G->App, MUIM_CallHook, &MA_SetStatusToHook,    SFLAG_QUEUED|SFLAG_READ,SFLAG_SENT|SFLAG_HOLD|SFLAG_ERROR); break;
    case MMEN_TOMARKED:   DoMethod(G->App, MUIM_CallHook, &MA_SetStatusToHook,    SFLAG_MARKED,           SFLAG_NONE);                        break;
    case MMEN_TOUNMARKED: DoMethod(G->App, MUIM_CallHook, &MA_SetStatusToHook,    SFLAG_NONE,             SFLAG_MARKED);                      break;
    case MMEN_ALLTOREAD:  DoMethod(G->App, MUIM_CallHook, &MA_SetAllStatusToHook, SFLAG_READ,             SFLAG_NEW);                         break;
    case MMEN_TOSPAM:     DoMethod(G->App, MUIM_CallHook, &MA_ClassifyMessageHook, BC_SPAM); break;
    case MMEN_TOHAM:      DoMethod(G->App, MUIM_CallHook, &MA_ClassifyMessageHook, BC_HAM); break;
    case MMEN_SAVEADDR:   DoMethod(G->App, MUIM_CallHook, &MA_GetAddressHook); break;
    case MMEN_MOVE:       DoMethod(G->App, MUIM_CallHook, &MA_MoveMessageHook); break;
    case MMEN_COPY:       DoMethod(G->App, MUIM_CallHook, &MA_CopyMessageHook); break;
    case MMEN_DELETE:     DoMethod(G->App, MUIM_CallHook, &MA_DeleteMessageHook,  0); break;
    case MMEN_PRINT:      DoMethod(G->App, MUIM_CallHook, &MA_SavePrintHook,      TRUE); break;
    case MMEN_SAVE:       DoMethod(G->App, MUIM_CallHook, &MA_SavePrintHook,      FALSE); break;
    case MMEN_DETACH:     DoMethod(G->App, MUIM_CallHook, &MA_SaveAttachHook); break;
    case MMEN_DELETEATT:  DoMethod(G->App, MUIM_CallHook, &MA_RemoveAttachHook); break;
    case MMEN_EXPMSG:     DoMethod(G->App, MUIM_CallHook, &MA_ExportMessagesHook, FALSE); break;
    case MMEN_NEW:        DoMethod(G->App, MUIM_CallHook, &MA_NewMessageHook,     NMM_NEW,  0); break;
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
    if(isFlagSet(C->MessageCols, (1<<i)))
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
/// DECLARE(RemoveMail)
// removes a mail visibly from the message listview
DECLARE(RemoveMail) // struct Mail* mail
{
  IPTR result = 0;

  ENTER();

  // check if there are any mails in the list at all.
  if(xget(obj, MUIA_NList_Entries) > 0)
  {
    LONG pos = MUIV_NList_GetPos_Start;

    // now also remove the mail from the currently active list
    DoMethod(obj, MUIM_NList_GetPos, msg->mail, &pos);
    if(pos != MUIV_NList_GetPos_End)
      result = DoMethod(obj, MUIM_NList_Remove, pos);
  }

  RETURN(result);
  return result;
}

///
