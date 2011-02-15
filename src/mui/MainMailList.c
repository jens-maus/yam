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

 Superclass:  MUIC_NList
 Description: NList class of the main mail list in the main window

***************************************************************************/

#include "MainMailList_cl.h"

#include <string.h>
#include <proto/dos.h>
#include <proto/muimaster.h>
#include <proto/timer.h>
#include <libraries/gadtools.h>
#include <mui/NList_mcc.h>

#include "SDI_hook.h"
#include "timeval.h"

#include "YAM.h"
#include "YAM_addressbook.h"
#include "YAM_addressbookEntry.h"
#include "YAM_config.h"
#include "YAM_find.h"
#include "YAM_mainFolder.h"

#include "BayesFilter.h"
#include "Locale.h"
#include "MUIObjects.h"
#include "Themes.h"

#include "mui/ImageArea.h"

#include "Debug.h"

/* CLASSDATA
struct Data
{
  Object *context_menu;
  Object *statusImage[SI_MAX];
  char fromBuffer[SIZE_DEFAULT];
  char replytoBuffer[SIZE_DEFAULT];
  char date1Buffer[64]; // we don't use LEN_DATSTRING as OS3.1 anyway ignores it.
  char date2Buffer[64]; // we don't use LEN_DATSTRING as OS3.1 anyway ignores it.
  char statusBuffer[SIZE_DEFAULT];
  char sizeBuffer[SIZE_SMALL];
  char context_menu_title[SIZE_DEFAULT];
  char bubbleInfo[SIZE_DEFAULT+SIZE_SUBJECT+2*SIZE_REALNAME+2*SIZE_ADDRESS+SIZE_MFILE];
};
*/

/* INCLUDE
#include "Themes.h"
*/

/* Hooks */
/// FindAddressHook
HOOKPROTONHNO(FindAddressFunc, LONG, struct MUIP_NListtree_FindUserDataMessage *msg)
{
  struct ABEntry *entry = (struct ABEntry *)msg->UserData;
  return Stricmp(msg->User, entry->Address);
}
MakeStaticHook(FindAddressHook, FindAddressFunc);

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
      status1 += (getImportanceLevel(entry1) == IMP_HIGH) ? 16 : 0;
      status2 += (getImportanceLevel(entry2) == IMP_HIGH) ? 16 : 0;

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
      if(C->ABookLookup == TRUE)
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
  ENTER();

  if((obj = DoSuperNew(cl, obj,

    MUIA_Font,                       C->FixedFontList ? MUIV_NList_Font_Fixed : MUIV_NList_Font,
    MUIA_NList_MinColSortable,       0,
    MUIA_NList_TitleClick,           TRUE,
    MUIA_NList_TitleClick2,          TRUE,
    MUIA_NList_MultiSelect,          MUIV_NList_MultiSelect_Default,
    MUIA_NList_AutoVisible,          TRUE,
    MUIA_NList_Title,                TRUE,
    MUIA_NList_TitleSeparator,       TRUE,
    MUIA_NList_ActiveObjectOnClick,  TRUE,
    MUIA_NList_DefaultObjectOnClick, FALSE,

    TAG_MORE, inittags(msg))) != NULL)
  {
    GETDATA;
    BOOL handleDoubleClick;
    ULONG i;

    // determine whether double clicks are handled by ourself or by some external stuff
    handleDoubleClick = GetTagData(ATTR(HandleDoubleClick), TRUE, inittags(msg));

    // prepare the mail status images
    data->statusImage[SI_ATTACH]   = MakeImageObject("status_attach",   G->theme.statusImages[SI_ATTACH]);
    data->statusImage[SI_CRYPT]    = MakeImageObject("status_crypt",    G->theme.statusImages[SI_CRYPT]);
    data->statusImage[SI_DELETE]   = MakeImageObject("status_delete",   G->theme.statusImages[SI_DELETE]);
    data->statusImage[SI_DOWNLOAD] = MakeImageObject("status_download", G->theme.statusImages[SI_DOWNLOAD]);
    data->statusImage[SI_ERROR]    = MakeImageObject("status_error",    G->theme.statusImages[SI_ERROR]);
    data->statusImage[SI_FORWARD]  = MakeImageObject("status_forward",  G->theme.statusImages[SI_FORWARD]);
    data->statusImage[SI_GROUP]    = MakeImageObject("status_group",    G->theme.statusImages[SI_GROUP]);
    data->statusImage[SI_HOLD]     = MakeImageObject("status_hold",     G->theme.statusImages[SI_HOLD]);
    data->statusImage[SI_MARK]     = MakeImageObject("status_mark",     G->theme.statusImages[SI_MARK]);
    data->statusImage[SI_NEW]      = MakeImageObject("status_new",      G->theme.statusImages[SI_NEW]);
    data->statusImage[SI_OLD]      = MakeImageObject("status_old",      G->theme.statusImages[SI_OLD]);
    data->statusImage[SI_REPLY]    = MakeImageObject("status_reply",    G->theme.statusImages[SI_REPLY]);
    data->statusImage[SI_REPORT]   = MakeImageObject("status_report",   G->theme.statusImages[SI_REPORT]);
    data->statusImage[SI_SENT]     = MakeImageObject("status_sent",     G->theme.statusImages[SI_SENT]);
    data->statusImage[SI_SIGNED]   = MakeImageObject("status_signed",   G->theme.statusImages[SI_SIGNED]);
    data->statusImage[SI_SPAM]     = MakeImageObject("status_spam",     G->theme.statusImages[SI_SPAM]);
    data->statusImage[SI_UNREAD]   = MakeImageObject("status_unread",   G->theme.statusImages[SI_UNREAD]);
    data->statusImage[SI_URGENT]   = MakeImageObject("status_urgent",   G->theme.statusImages[SI_URGENT]);
    data->statusImage[SI_WAITSEND] = MakeImageObject("status_waitsend", G->theme.statusImages[SI_WAITSEND]);
    for(i = 0; i < SI_MAX; i++)
    {
      if(data->statusImage[i] != NULL)
        DoMethod(obj, MUIM_NList_UseImage, data->statusImage[i], i, MUIF_NONE);
    }

    DoMethod(obj, MUIM_MainMailList_MakeFormat);
    DoMethod(obj, MUIM_Notify, MUIA_NList_Active,       MUIV_EveryTime, MUIV_Notify_Self, 1, MUIM_MainMailList_SetMailInfo);
    if(handleDoubleClick == TRUE)
      DoMethod(obj, MUIM_Notify, MUIA_NList_DoubleClick,  MUIV_EveryTime, MUIV_Notify_Self, 2, MUIM_MainMailList_DoubleClicked, MUIV_TriggerValue);
    DoMethod(obj, MUIM_Notify, MUIA_NList_SelectChange, TRUE,           MUIV_Notify_Application, 2, MUIM_CallHook, &MA_ChangeSelectedHook);

    // connect some notifies to the mainMailList group
    DoMethod(obj, MUIM_Notify, MUIA_NList_TitleClick,   MUIV_EveryTime, MUIV_Notify_Self, 4, MUIM_NList_Sort3, MUIV_TriggerValue,     MUIV_NList_SortTypeAdd_2Values, MUIV_NList_Sort3_SortType_Both);
    DoMethod(obj, MUIM_Notify, MUIA_NList_TitleClick2,  MUIV_EveryTime, MUIV_Notify_Self, 4, MUIM_NList_Sort3, MUIV_TriggerValue,     MUIV_NList_SortTypeAdd_2Values, MUIV_NList_Sort3_SortType_2);
    DoMethod(obj, MUIM_Notify, MUIA_NList_SortType,     MUIV_EveryTime, MUIV_Notify_Self, 3, MUIM_Set,         MUIA_NList_TitleMark,  MUIV_TriggerValue);
    DoMethod(obj, MUIM_Notify, MUIA_NList_SortType2,    MUIV_EveryTime, MUIV_Notify_Self, 3, MUIM_Set,         MUIA_NList_TitleMark2, MUIV_TriggerValue);
  }

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

  for(i=0; i < SI_MAX; i++)
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
/// OVERLOAD(MUIM_NList_Compare)
//  Message listview compare method
OVERLOAD(MUIM_NList_Compare)
{
  struct MUIP_NList_Compare *ncm = (struct MUIP_NList_Compare *)msg;
  struct Mail *entry1 = (struct Mail *)ncm->entry1;
  struct Mail *entry2 = (struct Mail *)ncm->entry2;
  LONG col1 = ncm->sort_type1 & MUIV_NList_TitleMark_ColMask;
  LONG col2 = ncm->sort_type2 & MUIV_NList_TitleMark2_ColMask;
  int cmp;

  ENTER();

  if(ncm->sort_type1 == (LONG)MUIV_NList_SortType_None)
  {
    RETURN(0);
    return 0;
  }

  if(ncm->sort_type1 & MUIV_NList_TitleMark_TypeMask) cmp = MailCompare(entry2, entry1, col1);
  else                                                cmp = MailCompare(entry1, entry2, col1);

  if(cmp != 0 || col1 == col2)
  {
    RETURN(cmp);
    return cmp;
  }

  if(ncm->sort_type2 & MUIV_NList_TitleMark2_TypeMask) cmp = MailCompare(entry2, entry1, col2);
  else                                                 cmp = MailCompare(entry1, entry2, col2);

  RETURN(cmp);
  return cmp;
}

///
/// OVERLOAD(MUIM_NList_Display)
OVERLOAD(MUIM_NList_Display)
{
  ENTER();

  if(G->MA != NULL)
  {
    GETDATA;
    struct MUIP_NList_Display *ndm = (struct MUIP_NList_Display *)msg;
    struct Mail *mail = (struct Mail *)ndm->entry;
    BOOL searchWinHook = FALSE;

    // now we check who is the parent of this DisplayHook
    if(G->FI != NULL && obj == G->FI->GUI.LV_MAILS)
      searchWinHook = TRUE;

    if(mail != NULL)
    {
      if(mail->Folder != NULL)
      {
        // prepare the status char buffer
        data->statusBuffer[0] = '\0';
        ndm->strings[0] = data->statusBuffer;

        // first we check which main status this mail has
        // and put the leftmost mail icon accordingly.
        if(hasStatusError(mail) || isPartialMail(mail)) strlcat(data->statusBuffer, SI_STR(SI_ERROR), sizeof(data->statusBuffer));
        else if(hasStatusQueued(mail))  strlcat(data->statusBuffer, SI_STR(SI_WAITSEND), sizeof(data->statusBuffer));
        else if(hasStatusSent(mail))    strlcat(data->statusBuffer, SI_STR(SI_SENT), sizeof(data->statusBuffer));
        else if(hasStatusNew(mail))     strlcat(data->statusBuffer, SI_STR(SI_NEW), sizeof(data->statusBuffer));
        else if(hasStatusHold(mail))    strlcat(data->statusBuffer, SI_STR(SI_HOLD), sizeof(data->statusBuffer));
        else if(hasStatusRead(mail))    strlcat(data->statusBuffer, SI_STR(SI_OLD), sizeof(data->statusBuffer));
        else                            strlcat(data->statusBuffer, SI_STR(SI_UNREAD), sizeof(data->statusBuffer));

        // then we add the 2. level if icons with the additional mail information
        // like importance, signed/crypted, report and attachment information
        if(C->SpamFilterEnabled == TRUE && hasStatusSpam(mail)) strlcat(data->statusBuffer, SI_STR(SI_SPAM), sizeof(data->statusBuffer));
        if(getImportanceLevel(mail) == IMP_HIGH)  strlcat(data->statusBuffer, SI_STR(SI_URGENT), sizeof(data->statusBuffer));
        if(isMP_CryptedMail(mail))                strlcat(data->statusBuffer, SI_STR(SI_CRYPT), sizeof(data->statusBuffer));
        else if(isMP_SignedMail(mail))            strlcat(data->statusBuffer, SI_STR(SI_SIGNED), sizeof(data->statusBuffer));
        if(isMP_ReportMail(mail))                 strlcat(data->statusBuffer, SI_STR(SI_REPORT), sizeof(data->statusBuffer));
        if(isMP_MixedMail(mail))                  strlcat(data->statusBuffer, SI_STR(SI_ATTACH), sizeof(data->statusBuffer));

        // and as the 3rd level of icons we put information on the secondary status
        // like marked, replied, forwarded
        if(hasStatusMarked(mail))     strlcat(data->statusBuffer, SI_STR(SI_MARK), sizeof(data->statusBuffer));
        if(hasStatusReplied(mail))    strlcat(data->statusBuffer, SI_STR(SI_REPLY), sizeof(data->statusBuffer));
        if(hasStatusForwarded(mail))  strlcat(data->statusBuffer, SI_STR(SI_FORWARD), sizeof(data->statusBuffer));

        // now we generate the proper string for the mailaddress
        if(hasMColSender(C->MessageCols) || searchWinHook == TRUE)
        {
          BOOL toPrefix = FALSE;
          struct Person *pe;
          char *addr = NULL;

          if(((isCustomMixedFolder(mail->Folder) || isTrashFolder(mail->Folder) || isSpamFolder(mail->Folder)) &&
              (hasStatusSent(mail) || hasStatusQueued(mail) || hasStatusHold(mail) ||
               hasStatusError(mail))) || (searchWinHook && isSentMailFolder(mail->Folder)))
          {
            pe = &mail->To;

            // put a To: prefix before our sender name
            toPrefix = TRUE;
          }
          else
            pe = isSentMailFolder(mail->Folder) ? &mail->To : &mail->From;

          // in case the user wants to take the additional pain
          // of performing an addressbook lookup for every mail in the
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
          snprintf(data->fromBuffer, sizeof(data->fromBuffer), "%s%s%s%s", isMultiRCPTMail(mail) ? SI_STR(SI_GROUP) : "",
                                                         toPrefix ? tr(MSG_MA_ToPrefix) : "",
                                                         addr,
                                                         isMultiSenderMail(mail) && toPrefix == FALSE ? ", ..." : "");

          ndm->strings[1] = data->fromBuffer;
        }

        // lets set all other fields now
        if(searchWinHook == FALSE && hasMColReplyTo(C->MessageCols))
        {
          if(isMultiReplyToMail(mail))
          {
            snprintf(data->replytoBuffer, sizeof(data->replytoBuffer), "%s, ...", AddrName(mail->ReplyTo));
            ndm->strings[2] = data->replytoBuffer;
          }
          else
            ndm->strings[2] = AddrName(mail->ReplyTo);
        }

        // then the Subject
        ndm->strings[3] = mail->Subject;

        if(hasMColDate(C->MessageCols) || searchWinHook == TRUE)
        {
          DateStamp2String(data->date1Buffer, sizeof(data->date1Buffer), &mail->Date, C->DSListFormat, TZC_LOCAL);
          ndm->strings[4] = data->date1Buffer;
        }

        if(hasMColSize(C->MessageCols) || searchWinHook == TRUE)
        {
          ndm->strings[5] = data->sizeBuffer;
          FormatSize(mail->Size, data->sizeBuffer, sizeof(data->sizeBuffer), SF_AUTO);
        }

        ndm->strings[6] = mail->MailFile;

        // we first copy the Date Received/sent because this would probably be not
        // set by all ppl and strcpy() is costy ;)
        if((hasMColTransDate(C->MessageCols) && mail->transDate.Seconds > 0) || searchWinHook == TRUE)
        {
          TimeVal2String(data->date2Buffer, sizeof(data->date2Buffer), &mail->transDate, C->DSListFormat, TZC_LOCAL);
          ndm->strings[7] = data->date2Buffer;
        }

        ndm->strings[8] = mail->Folder->Name;

        // depending on the mail status we set the font to bold or plain
        if(hasStatusUnread(mail) || hasStatusNew(mail))
        {
          ndm->preparses[1] = C->StyleMailUnread;
          ndm->preparses[2] = C->StyleMailUnread;
          ndm->preparses[3] = C->StyleMailUnread;
          ndm->preparses[4] = C->StyleMailUnread;
          ndm->preparses[5] = C->StyleMailUnread;
        }
        else
        {
          ndm->preparses[1] = C->StyleMailRead;
          ndm->preparses[2] = C->StyleMailRead;
          ndm->preparses[3] = C->StyleMailRead;
          ndm->preparses[4] = C->StyleMailRead;
          ndm->preparses[5] = C->StyleMailRead;
        }
      }
    }
    else
    {
      struct Folder *folder = GetCurrentFolder();

      // first we have to make sure that the mail window has a valid folder
      if(searchWinHook == TRUE || folder != NULL)
      {
        ndm->strings[0] = (STRPTR)tr(MSG_MA_TitleStatus);

        // depending on the current folder and the parent object we
        // display different titles for different columns
        if(searchWinHook == FALSE && isSentMailFolder(folder))
        {
          ndm->strings[1] = (STRPTR)tr(MSG_To);
          ndm->strings[7] = (STRPTR)tr(MSG_DATE_SENT);
        }
        else if(searchWinHook == TRUE || isCustomMixedFolder(folder) || isTrashFolder(folder) || isSpamFolder(folder))
        {
          ndm->strings[1] = (STRPTR)tr(MSG_FROMTO);
          ndm->strings[7] = (STRPTR)tr(MSG_DATE_SNTRCVD);
        }
        else
        {
          ndm->strings[1] = (STRPTR)tr(MSG_From);
          ndm->strings[7] = (STRPTR)tr(MSG_DATE_RECEIVED);
        }

        ndm->strings[2] = (STRPTR)tr(MSG_ReturnAddress);
        ndm->strings[3] = (STRPTR)tr(MSG_Subject);
        ndm->strings[4] = (STRPTR)tr(MSG_Date);
        ndm->strings[5] = (STRPTR)tr(MSG_Size);
        ndm->strings[6] = (STRPTR)tr(MSG_Filename);

        // The Folder is just a dummy entry to serve the SearchWindowDisplayHook
        ndm->strings[8] = (STRPTR)tr(MSG_Folder);
      }
    }
  }

  LEAVE();
  return 0;
}

///
/// OVERLOAD(MUIM_NList_ContextMenuBuild)
OVERLOAD(MUIM_NList_ContextMenuBuild)
{
  GETDATA;
  struct MUIP_NList_ContextMenuBuild *m = (struct MUIP_NList_ContextMenuBuild *)msg;
  struct Folder *fo;

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
      MenuChild, MenuObjectT(tr(MSG_MA_CTX_MAILLIST)),
        MenuChild, MenuitemObject, MUIA_Menuitem_Title, tr(MSG_Status),         MUIA_UserData, 1, MUIA_Menuitem_Enabled, FALSE, MUIA_Menuitem_Checked, TRUE, MUIA_Menuitem_Checkit, TRUE, MUIA_Menuitem_Toggle, TRUE, End,
        MenuChild, MenuitemObject, MUIA_Menuitem_Title, tr(MSG_SenderRecpt),    MUIA_UserData, 2, MUIA_Menuitem_Checked, hasMColSender(C->MessageCols),    MUIA_Menuitem_Checkit, TRUE, MUIA_Menuitem_Toggle, TRUE, End,
        MenuChild, MenuitemObject, MUIA_Menuitem_Title, tr(MSG_ReturnAddress),  MUIA_UserData, 3, MUIA_Menuitem_Checked, hasMColReplyTo(C->MessageCols),   MUIA_Menuitem_Checkit, TRUE, MUIA_Menuitem_Toggle, TRUE, End,
        MenuChild, MenuitemObject, MUIA_Menuitem_Title, tr(MSG_Subject),        MUIA_UserData, 4, MUIA_Menuitem_Checked, hasMColSubject(C->MessageCols),   MUIA_Menuitem_Checkit, TRUE, MUIA_Menuitem_Toggle, TRUE, End,
        MenuChild, MenuitemObject, MUIA_Menuitem_Title, tr(MSG_MessageDate),    MUIA_UserData, 5, MUIA_Menuitem_Checked, hasMColDate(C->MessageCols),      MUIA_Menuitem_Checkit, TRUE, MUIA_Menuitem_Toggle, TRUE, End,
        MenuChild, MenuitemObject, MUIA_Menuitem_Title, tr(MSG_Size),           MUIA_UserData, 6, MUIA_Menuitem_Checked, hasMColSize(C->MessageCols),      MUIA_Menuitem_Checkit, TRUE, MUIA_Menuitem_Toggle, TRUE, End,
        MenuChild, MenuitemObject, MUIA_Menuitem_Title, tr(MSG_Filename),       MUIA_UserData, 7, MUIA_Menuitem_Checked, hasMColFilename(C->MessageCols),  MUIA_Menuitem_Checkit, TRUE, MUIA_Menuitem_Toggle, TRUE, End,
        MenuChild, MenuitemObject, MUIA_Menuitem_Title, tr(MSG_CO_DATE_SNTRCVD),MUIA_UserData, 8, MUIA_Menuitem_Checked, hasMColTransDate(C->MessageCols), MUIA_Menuitem_Checkit, TRUE, MUIA_Menuitem_Toggle, TRUE, End,
        MenuChild, MenuitemObject, MUIA_Menuitem_Title, NM_BARLABEL, End,
        MenuChild, MenuitemObject, MUIA_Menuitem_Title, tr(MSG_MA_CTX_DEFWIDTH_THIS), MUIA_UserData, MUIV_NList_Menu_DefWidth_This, End,
        MenuChild, MenuitemObject, MUIA_Menuitem_Title, tr(MSG_MA_CTX_DEFWIDTH_ALL),  MUIA_UserData, MUIV_NList_Menu_DefWidth_All,  End,
        MenuChild, MenuitemObject, MUIA_Menuitem_Title, tr(MSG_MA_CTX_DEFORDER_THIS), MUIA_UserData, MUIV_NList_Menu_DefOrder_This, End,
        MenuChild, MenuitemObject, MUIA_Menuitem_Title, tr(MSG_MA_CTX_DEFORDER_ALL),  MUIA_UserData, MUIV_NList_Menu_DefOrder_All,  End,
      End,
    End;
  }
  else if((fo = GetCurrentFolder()) != NULL)
  {
    struct MUI_NList_TestPos_Result res;

    // Now lets find out which entry is under the mouse pointer
    DoMethod(obj, MUIM_NList_TestPos, m->mx, m->my, &res);

    if(res.entry >= 0)
    {
      struct Mail *mail = NULL;

      DoMethod(obj, MUIM_NList_GetEntry, res.entry, &mail);
      if(mail != NULL)
      {
        BOOL isOutBox = isOutgoingFolder(fo);
        BOOL isSentMail = isSentMailFolder(fo);
        BOOL hasattach = FALSE;
        ULONG numSelected = 0;
        struct Person *pers = isSentMail ? &mail->To : &mail->From;
        char address[SIZE_LARGE];
        Object *afterThis;

        fo->LastActive = xget(obj, MUIA_NList_Active);

        // Now we set this entry as active and make it visible
        // in the center of our listview
        if(fo->LastActive != res.entry)
          DoMethod(obj, MUIM_NList_SetActive, res.entry, MUIV_NList_SetActive_Jump_Center);

        if(isMultiPartMail(mail))
          hasattach = TRUE;

        DoMethod(obj, MUIM_NList_Select, MUIV_NList_Select_All, MUIV_NList_Select_Ask, &numSelected);

        // now we create the menu title of the context menu
        snprintf(data->context_menu_title, sizeof(data->context_menu_title), "%s: ", tr(isSentMail ? MSG_To : MSG_From));
        strlcat(data->context_menu_title, BuildAddress(address, sizeof(address), pers->Address, pers->RealName), 20-strlen(data->context_menu_title) > 0 ? 20-strlen(data->context_menu_title) : 0);
        strlcat(data->context_menu_title, "...", sizeof(data->context_menu_title));

        data->context_menu = MenustripObject,
          MenuChild, MenuObjectT(data->context_menu_title),
            MenuChild, Menuitem(tr(MSG_MA_MREAD), NULL, TRUE, FALSE, MMEN_READ),
            MenuChild, Menuitem(isOutBox ? tr(MSG_MA_MEDIT) : tr(MSG_MA_MEDITASNEW), NULL, TRUE, FALSE, MMEN_EDIT),
            MenuChild, Menuitem(tr(MSG_MA_MMOVE), NULL, TRUE, FALSE, MMEN_MOVE),
            MenuChild, Menuitem(tr(MSG_MA_MCOPY), NULL, TRUE, FALSE, MMEN_COPY),
            MenuChild, Menuitem(tr(MSG_MA_MDelete), NULL, TRUE, FALSE, MMEN_DELETE),
            MenuChild, MenuBarLabel,
            MenuChild, Menuitem(tr(MSG_MA_MPRINT), NULL, TRUE, FALSE, MMEN_PRINT),
            MenuChild, Menuitem(tr(MSG_MA_MSAVE), NULL, TRUE, FALSE, MMEN_SAVE),
            MenuChild, MenuitemObject,
              MUIA_Menuitem_Title, tr(MSG_Attachments),
              MUIA_Menuitem_Enabled, hasattach,
              MenuChild, Menuitem(tr(MSG_MA_MSAVEATT), NULL, hasattach, FALSE, MMEN_DETACH),
              MenuChild, Menuitem(tr(MSG_MA_MDELETEATT), NULL, hasattach, FALSE, MMEN_DELETEATT),
            End,
            MenuChild, Menuitem(tr(MSG_MESSAGE_EXPORT), NULL, TRUE, FALSE, MMEN_EXPMSG),
            MenuChild, MenuBarLabel,
            MenuChild, Menuitem(tr(MSG_MA_MNEW), NULL, TRUE, FALSE, MMEN_NEW),
            MenuChild, Menuitem(tr(MSG_MA_MREPLY), NULL, TRUE, FALSE, MMEN_REPLY),
            MenuChild, MenuitemObject,
              MUIA_Menuitem_Title, tr(MSG_MA_MFORWARD),
              MenuChild, Menuitem(tr(MSG_MA_MFORWARD_ATTACH), NULL, TRUE, FALSE, MMEN_FORWARD_ATTACH),
              MenuChild, Menuitem(tr(MSG_MA_MFORWARD_INLINE), NULL, TRUE, FALSE, MMEN_FORWARD_INLINE),
            End,
            MenuChild, Menuitem(tr(MSG_MA_MBOUNCE), NULL, TRUE, FALSE, MMEN_BOUNCE),
            MenuChild, MenuBarLabel,
            MenuChild, Menuitem(tr(MSG_MA_MSAVEADDRESS), NULL, TRUE, FALSE, MMEN_SAVEADDR),
            MenuChild, MenuitemObject,
              MUIA_Menuitem_Title, tr(MSG_MA_Select),
              MenuChild, Menuitem(tr(MSG_MA_SELECTALL), NULL, TRUE, FALSE, MMEN_SELALL),
              MenuChild, Menuitem(tr(MSG_MA_SELECTNONE), NULL, TRUE, FALSE, MMEN_SELNONE),
              MenuChild, Menuitem(tr(MSG_MA_SELECTTOGGLE), NULL, TRUE, FALSE, MMEN_SELTOGG),
            End,
            MenuChild, MenuitemObject,
              MUIA_Menuitem_Title, tr(MSG_MA_SetStatus),
              MenuChild, Menuitem(tr(MSG_MA_TOMARKED), NULL, numSelected >= 2 || !hasStatusMarked(mail), FALSE, MMEN_TOMARKED),
              MenuChild, Menuitem(tr(MSG_MA_TOUNMARKED), NULL, numSelected >= 2 ||  hasStatusMarked(mail), FALSE, MMEN_TOUNMARKED),
              MenuChild, Menuitem(tr(MSG_MA_TOREAD), NULL, !isSentMail && (numSelected >= 2 || hasStatusNew(mail) || hasStatusUnread(mail)), FALSE, MMEN_TOREAD),
              MenuChild, Menuitem(tr(MSG_MA_TOUNREAD), NULL, !isSentMail && (numSelected >= 2 || hasStatusRead(mail)), FALSE, MMEN_TOUNREAD),
              MenuChild, Menuitem(tr(MSG_MA_TOHOLD), NULL, isOutBox && !hasStatusHold(mail), FALSE, MMEN_TOHOLD),
              MenuChild, afterThis = Menuitem(tr(MSG_MA_TOQUEUED), NULL, isOutBox && !hasStatusQueued(mail), FALSE, MMEN_TOQUEUED),
              MenuChild, MenuBarLabel,
              MenuChild, Menuitem(tr(MSG_MA_ALLTOREAD), NULL, !isSentMail,  FALSE, MMEN_ALLTOREAD),
            End,
            MenuChild, Menuitem(tr(MSG_MA_ChangeSubj), NULL, TRUE, FALSE, MMEN_CHSUBJ),
            MenuChild, MenuBarLabel,
            MenuChild, Menuitem(tr(MSG_MA_MSend), NULL, isOutBox, FALSE, MMEN_SEND),
          End,
        End;

        if(data->context_menu != NULL && C->SpamFilterEnabled == TRUE)
        {
          Object *spamItem;
          Object *hamItem;

          spamItem = Menuitem(tr(MSG_MA_TOSPAM), NULL,    numSelected >= 2 || !hasStatusSpam(mail), FALSE, MMEN_TOSPAM);
          hamItem =  Menuitem(tr(MSG_MA_TONOTSPAM), NULL, numSelected >= 2 ||  hasStatusSpam(mail), FALSE, MMEN_TOHAM);

          DoMethod(data->context_menu, MUIM_Family_Insert, hamItem, afterThis);
          DoMethod(data->context_menu, MUIM_Family_Insert, spamItem, afterThis);
        }
      }
    }
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
    case MMEN_READ:           DoMethod(G->App, MUIM_CallHook, &MA_ReadMessageHook); break;
    case MMEN_EDIT:           DoMethod(G->App, MUIM_CallHook, &MA_NewMessageHook, NMM_EDIT,           0); break;
    case MMEN_NEW:            DoMethod(G->App, MUIM_CallHook, &MA_NewMessageHook, NMM_NEW,            0); break;
    case MMEN_REPLY:          DoMethod(G->App, MUIM_CallHook, &MA_NewMessageHook, NMM_REPLY,          0); break;
    case MMEN_FORWARD_ATTACH: DoMethod(G->App, MUIM_CallHook, &MA_NewMessageHook, NMM_FORWARD_ATTACH, 0); break;
    case MMEN_FORWARD_INLINE: DoMethod(G->App, MUIM_CallHook, &MA_NewMessageHook, NMM_FORWARD_INLINE, 0); break;
    case MMEN_BOUNCE:         DoMethod(G->App, MUIM_CallHook, &MA_NewMessageHook, NMM_BOUNCE,         0); break;
    case MMEN_SEND:           DoMethod(G->App, MUIM_CallHook, &MA_SendHook, SENDMAIL_ACTIVE_USER); break;
    case MMEN_CHSUBJ:         DoMethod(G->App, MUIM_CallHook, &MA_ChangeSubjectHook); break;
    case MMEN_TOUNREAD:       DoMethod(G->App, MUIM_CallHook, &MA_SetStatusToHook, SFLAG_NONE,              SFLAG_NEW|SFLAG_READ);              break;
    case MMEN_TOREAD:         DoMethod(G->App, MUIM_CallHook, &MA_SetStatusToHook, SFLAG_READ,              SFLAG_NEW);                         break;
    case MMEN_TOHOLD:         DoMethod(G->App, MUIM_CallHook, &MA_SetStatusToHook, SFLAG_HOLD|SFLAG_READ,   SFLAG_QUEUED|SFLAG_ERROR);          break;
    case MMEN_TOQUEUED:       DoMethod(G->App, MUIM_CallHook, &MA_SetStatusToHook, SFLAG_QUEUED|SFLAG_READ, SFLAG_SENT|SFLAG_HOLD|SFLAG_ERROR); break;
    case MMEN_TOMARKED:       DoMethod(G->App, MUIM_CallHook, &MA_SetStatusToHook, SFLAG_MARKED,            SFLAG_NONE);                        break;
    case MMEN_TOUNMARKED:     DoMethod(G->App, MUIM_CallHook, &MA_SetStatusToHook, SFLAG_NONE,              SFLAG_MARKED);                      break;
    case MMEN_ALLTOREAD:      DoMethod(G->App, MUIM_CallHook, &MA_SetAllStatusToHook, SFLAG_READ, SFLAG_NEW);                         break;
    case MMEN_TOSPAM:         DoMethod(G->App, MUIM_CallHook, &MA_ClassifyMessageHook, BC_SPAM); break;
    case MMEN_TOHAM:          DoMethod(G->App, MUIM_CallHook, &MA_ClassifyMessageHook, BC_HAM); break;
    case MMEN_SAVEADDR:       DoMethod(G->App, MUIM_CallHook, &MA_GetAddressHook); break;
    case MMEN_MOVE:           DoMethod(G->App, MUIM_CallHook, &MA_MoveMessageHook); break;
    case MMEN_COPY:           DoMethod(G->App, MUIM_CallHook, &MA_CopyMessageHook); break;
    case MMEN_DELETE:         DoMethod(G->App, MUIM_CallHook, &MA_DeleteMessageHook,  0); break;
    case MMEN_PRINT:          DoMethod(G->App, MUIM_CallHook, &MA_SavePrintHook, TRUE); break;
    case MMEN_SAVE:           DoMethod(G->App, MUIM_CallHook, &MA_SavePrintHook, FALSE); break;
    case MMEN_DETACH:         DoMethod(G->App, MUIM_CallHook, &MA_SaveAttachHook); break;
    case MMEN_DELETEATT:      DoMethod(G->App, MUIM_CallHook, &MA_RemoveAttachHook); break;
    case MMEN_EXPMSG:         DoMethod(G->App, MUIM_CallHook, &MA_ExportMessagesHook, FALSE); break;
    case MMEN_SELALL:         DoMethod(obj, MUIM_NList_Select, MUIV_NList_Select_All, MUIV_NList_Select_On,     NULL); break;
    case MMEN_SELNONE:        DoMethod(obj, MUIM_NList_Select, MUIV_NList_Select_All, MUIV_NList_Select_Off,    NULL); break;
    case MMEN_SELTOGG:        DoMethod(obj, MUIM_NList_Select, MUIV_NList_Select_All, MUIV_NList_Select_Toggle, NULL); break;

    default:
    {
      return DoSuperMethodA(cl, obj, msg);
    }
  }

  return 0;
}

///

/* Private Methods */
/// DECLARE(DoubleClicked)
// if the user double-clicked in the mail list we either
// have to open the message in a read window or if it is currently in
// the outgoing folder we open it for editing.
DECLARE(DoubleClicked) // LONG entryNum
{
  ENTER();

  if(msg->entryNum >= 0)
  {
    // A double click in the outgoing folder should popup a write
    // window instead.
    if(GetCurrentFolder() != NULL && isOutgoingFolder(GetCurrentFolder()))
    {
      // in case the folder is the "outgoing" folder
      // we edit the mail instead.
      DoMethod(G->App, MUIM_CallHook, &MA_NewMessageHook, NMM_EDIT, 0);
    }
    else
    {
      // if not, then we open a read window instead
      DoMethod(G->App, MUIM_CallHook, &MA_ReadMessageHook);
    }
  }

  RETURN(0);
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
/// DECLARE(SetMailInfo)
// update the mail list bubble help
DECLARE(SetMailInfo)
{
  GETDATA;
  struct Mail *mail;

  ENTER();

  if((mail = MA_GetActiveMail(NULL, NULL, NULL)) != NULL)
  {
    char datestr[64];
    char sizestr[SIZE_DEFAULT];

    // convert the datestamp of the mail to
    // well defined string
    DateStamp2String(datestr, sizeof(datestr), &mail->Date, (C->DSListFormat == DSS_DATEBEAT || C->DSListFormat == DSS_RELDATEBEAT) ? DSS_DATEBEAT : DSS_DATETIME, TZC_LOCAL);

    // use FormatSize() to prettify the size display of the mail info
    FormatSize(mail->Size, sizestr, sizeof(sizestr), SF_AUTO);

    snprintf(data->bubbleInfo, sizeof(data->bubbleInfo), tr(MSG_MA_MESSAGEINFO), mail->From.RealName,
                                                                                 mail->From.Address,
                                                                                 mail->To.RealName,
                                                                                 mail->To.Address,
                                                                                 mail->Subject,
                                                                                 datestr,
                                                                                 mail->MailFile,
                                                                                 sizestr);

    set(obj, MUIA_ShortHelp, data->bubbleInfo);
  }
  else
    set(obj, MUIA_ShortHelp, NULL);

  LEAVE();
  return 0;
}

///
