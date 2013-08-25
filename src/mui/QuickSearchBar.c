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
 MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.   See the
 GNU General Public License for more details.

 You should have received a copy of the GNU General Public License
 along with this program; if not, write to the Free Software
 Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA

 YAM Official Support Site : http://www.yam.ch
 YAM OpenSource project     : http://sourceforge.net/projects/yamos/

 $Id$

 Superclass:  MUIC_Group
 Description: Provides functionality of a quick search bar in the main win

***************************************************************************/

#include "QuickSearchBar_cl.h"

#include <string.h>
#include <proto/muimaster.h>
#include <proto/timer.h>
#include <mui/BetterString_mcc.h>
#include <mui/NList_mcc.h>
#include <mui/NListview_mcc.h>

#include "SDI_hook.h"

#include "YAM.h"
#include "YAM_addressbook.h"
#include "YAM_addressbookEntry.h"
#include "YAM_find.h"
#include "YAM_mainFolder.h"
#include "YAM_utilities.h"

#include "BoyerMooreSearch.h"
#include "Busy.h"
#include "DynamicString.h"
#include "Locale.h"
#include "MailList.h"
#include "MUIObjects.h"

#include "mui/AddrBookListtree.h"
#include "mui/MainMailListGroup.h"

#include "Debug.h"

/* CLASSDATA
struct Data
{
  Object *CY_VIEWOPTIONS;
  Object *TX_STATUSTEXT;
  Object *PO_SEARCHOPTIONPOPUP;
  Object *NL_SEARCHOPTIONS;
  Object *ST_SEARCHSTRING;
  Object *BT_CLEARBUTTON;
  struct TimeVal last_statusupdate;
  BOOL abortSearch;
  BOOL searchInProgress;
  char statusText[SIZE_DEFAULT];
};
*/

/* INCLUDE
#include "timeval.h"
*/

/* Enumerations */
enum SearchOptions { SO_SUBJECT=0, SO_SENDER, SO_SUBJORSENDER, SO_TOORCC, SO_INMSG, SO_ENTIREMSG };
enum ViewOptions { VO_ALL=0, VO_UNREAD, VO_NEW, VO_MARKED, VO_IMPORTANT, VO_LAST5DAYS, VO_KNOWNPEOPLE, VO_HASATTACHMENTS, VO_MINSIZE };

/* Private Functions */
/// MatchMail()
// function to actually check if a struct Mail* matches
// the currently active criteria
static BOOL MatchMail(struct Mail *mail, enum ViewOptions vo,
                      enum SearchOptions so, const struct BoyerMooreContext *bmContext, struct TimeVal *curTimeUTC)
{
  BOOL foundMatch = FALSE;

  ENTER();

  // we first check for viewOption selection
  switch(vo)
  {
    // match all mails
    case VO_ALL:
      foundMatch = TRUE;
    break;

    // check for UNREAD mail status
    case VO_UNREAD:
      foundMatch = (!hasStatusRead(mail) || hasStatusNew(mail));
    break;

    // check for NEW mail status
    case VO_NEW:
      foundMatch = hasStatusNew(mail);
    break;

    // check for MARKED mail status
    case VO_MARKED:
      foundMatch = hasStatusMarked(mail);
    break;

    // check for the Important status
    case VO_IMPORTANT:
      foundMatch = getImportanceLevel(mail) == IMP_HIGH;
    break;

    // check if the mail is not older than 5 days (taken from the receive date)
    case VO_LAST5DAYS:
    {
      struct TimeVal now;

      memcpy(&now, curTimeUTC, sizeof(struct TimeVal));
      SubTime(TIMEVAL(&now), TIMEVAL(&mail->transDate));

      // check if after subtime now is <= 5 days
      foundMatch = (now.Seconds <= (5*24*60*60));
    }
    break;

    // check if the mail comes from a person we know
    case VO_KNOWNPEOPLE:
    {
      foundMatch = ((APTR)DoMethod(G->AB->GUI.LV_ADDRESSES, MUIM_AddrBookListtree_FindPerson, &mail->From) != NULL);
      if(foundMatch == FALSE && isMultiSenderMail(mail))
      {
        struct ExtendedMail *email;

        if((email = MA_ExamineMail(mail->Folder, mail->MailFile, TRUE)) != NULL)
        {
          int j;

          for(j=0; j < email->NumSFrom && foundMatch == FALSE; j++)
          {
            foundMatch = ((APTR)DoMethod(G->AB->GUI.LV_ADDRESSES, MUIM_AddrBookListtree_FindPerson, &email->SFrom[j]) != NULL);
          }

          MA_FreeEMailStruct(email);
        }
      }
    }
    break;

    // check if the mail has attachments
    case VO_HASATTACHMENTS:
    {
      foundMatch = isMP_MixedMail(mail);
    }
    break;

    // check if the mail has a size > 1MB
    case VO_MINSIZE:
    {
      foundMatch = (mail->Size > 1024*1024);
    }
    break;
  }

  // now we do a bit more complicated search if a search string
  // is specified as well
  if(foundMatch == TRUE && bmContext != NULL)
  {
    // we check which search option is currently choosen the matching
    switch(so)
    {
      // check if the searchstring matches any string in the mail's subject
      case SO_SUBJECT:
      {
        foundMatch = (BoyerMooreSearch(bmContext, mail->Subject) != NULL);
      }
      break;

      // check if the searchstring matches any string in the mail's sender address
      case SO_SENDER:
      {
        foundMatch = (BoyerMooreSearch(bmContext, mail->From.Address) != NULL ||
                      BoyerMooreSearch(bmContext, mail->From.RealName) != NULL);


        if(foundMatch == FALSE && isMultiSenderMail(mail))
        {
          struct ExtendedMail *email;

          if((email = MA_ExamineMail(mail->Folder, mail->MailFile, TRUE)) != NULL)
          {
            int j;

            for(j=0; j < email->NumSFrom && foundMatch == FALSE; j++)
            {
              struct Person *pe = &email->SFrom[j];

              foundMatch = (BoyerMooreSearch(bmContext, pe->Address) != NULL ||
                            BoyerMooreSearch(bmContext, pe->RealName) != NULL);
            }

            MA_FreeEMailStruct(email);
          }
        }
      }
      break;

      // check if the searchstring matches any string in the mail's subject or sender
      case SO_SUBJORSENDER:
      {
        foundMatch = (BoyerMooreSearch(bmContext, mail->Subject) != NULL ||
                      BoyerMooreSearch(bmContext, mail->From.Address) != NULL ||
                      BoyerMooreSearch(bmContext, mail->From.RealName) != NULL);

        if(foundMatch == FALSE && isMultiSenderMail(mail))
        {
          struct ExtendedMail *email;

          if((email = MA_ExamineMail(mail->Folder, mail->MailFile, TRUE)) != NULL)
          {
            int j;

            for(j=0; j < email->NumSFrom && foundMatch == FALSE; j++)
            {
              struct Person *pe = &email->SFrom[j];

              foundMatch = (BoyerMooreSearch(bmContext, pe->Address) != NULL ||
                            BoyerMooreSearch(bmContext, pe->RealName) != NULL);
            }

            MA_FreeEMailStruct(email);
          }
        }
      }
      break;

      // check if the searchString matches any string in the mail's TO or CC address
      case SO_TOORCC:
      {
        foundMatch = (BoyerMooreSearch(bmContext, mail->To.Address) != NULL ||
                      BoyerMooreSearch(bmContext, mail->To.RealName) != NULL);

        // if we still haven't found a match with the To: string we go
        // and do a deeper search
        if(foundMatch == FALSE && isMultiRCPTMail(mail))
        {
          struct ExtendedMail *email;

          if((email = MA_ExamineMail(mail->Folder, mail->MailFile, TRUE)) != NULL)
          {
            int j;

            // search the additional To: recipients
            for(j=0; j < email->NumSTo && foundMatch == FALSE; j++)
            {
              struct Person *to = &email->STo[j];

              foundMatch = (BoyerMooreSearch(bmContext, to->Address) != NULL ||
                            BoyerMooreSearch(bmContext, to->RealName) != NULL);
            }

            for(j=0; j < email->NumCC && foundMatch == FALSE; j++)
            {
              struct Person *cc = &email->CC[j];

              foundMatch = (BoyerMooreSearch(bmContext, cc->Address) != NULL ||
                            BoyerMooreSearch(bmContext, cc->RealName) != NULL);
            }

            MA_FreeEMailStruct(email);
          }
        }
      }
      break;

      // check if the searchString matches anything at all in the actual mail text
      case SO_INMSG:
      {
        struct ReadMailData *rmData;

        // allocate a private readmaildata object in which we readin
        // the mail text
        if((rmData = AllocPrivateRMData(mail, PM_TEXTS)) != NULL)
        {
          char *cmsg;

          if((cmsg = RE_ReadInMessage(rmData, RIM_QUIET)) != NULL)
          {
            // perform the search in the complete body
            foundMatch = (BoyerMooreSearch(bmContext, cmsg) != NULL);

            // free the allocated message text immediately
            dstrfree(cmsg);
          }

          FreePrivateRMData(rmData);
        }
      }
      break;

      // check if the searchString matches anything at all in our entire message
      case SO_ENTIREMSG:
      {
        struct Search search;

        // we use our global find function for searching in the entire message
        // (including the headers)
        if(FI_PrepareSearch(&search, SM_WHOLE,
                                     0,
                                     CP_EQUAL,
                                     0,
                                     bmContext->pattern,
                                     "",
                                     SEARCHF_SUBSTRING))
        {
          foundMatch = FI_DoSearch(&search, mail);
        }

        FreeSearchData(&search);
      }
      break;
    }
  }

  RETURN(foundMatch);
  return foundMatch;
}

///

/* Overloaded Methods */
/// OVERLOAD(OM_NEW)
OVERLOAD(OM_NEW)
{
  Object *viewOptionCycle;
  Object *statusText;
  Object *searchOptionPopup;
  Object *searchOptionsList;
  Object *searchString;
  Object *clearButton;
  static const char *searchOptions[7];
  static const char *viewOptions[10];

  searchOptions[0] = tr(MSG_QUICKSEARCH_SO_SUBJECT);
  searchOptions[1] = tr(MSG_QUICKSEARCH_SO_SENDER);
  searchOptions[2] = tr(MSG_QUICKSEARCH_SO_SUBJORSENDER);
  searchOptions[3] = tr(MSG_QUICKSEARCH_SO_TOORCC);
  searchOptions[4] = tr(MSG_QUICKSEARCH_SO_INMSG);
  searchOptions[5] = tr(MSG_QUICKSEARCH_SO_ENTIREMSG);
  searchOptions[6] = NULL;

  viewOptions[0] = tr(MSG_QUICKSEARCH_VO_ALL);
  viewOptions[1] = tr(MSG_QUICKSEARCH_VO_UNREAD);
  viewOptions[2] = tr(MSG_QUICKSEARCH_VO_NEW);
  viewOptions[3] = tr(MSG_QUICKSEARCH_VO_MARKED);
  viewOptions[4] = tr(MSG_QUICKSEARCH_VO_IMPORTANT);
  viewOptions[5] = tr(MSG_QUICKSEARCH_VO_LAST5DAYS);
  viewOptions[6] = tr(MSG_QUICKSEARCH_VO_KNOWNPEOPLE);
  viewOptions[7] = tr(MSG_QUICKSEARCH_VO_HASATTACHMENTS);
  viewOptions[8] = tr(MSG_QUICKSEARCH_VO_MINSIZE);
  viewOptions[9] = NULL;

  if((obj = DoSuperNew(cl, obj,

    MUIA_Group_Horiz,   TRUE,
    Child, HGroup,
      MUIA_Weight, 25,
      InnerSpacing(0,0),
      Child, MUI_MakeObject(MUIO_Label, (ULONG)tr(MSG_QUICKSEARCH_VIEW), MUIO_Label_Tiny),
      Child, viewOptionCycle = CycleObject,
        MUIA_Font,          MUIV_Font_Tiny,
        MUIA_CycleChain,    TRUE,
        MUIA_Cycle_Entries, viewOptions,
        MUIA_ControlChar,   ShortCut(tr(MSG_QUICKSEARCH_VIEW)),
      End,
    End,

    Child, HGroup,
      MUIA_Weight, 50,
      Child, statusText = TextObject,
        MUIA_Font,          MUIV_Font_Tiny,
        MUIA_Text_PreParse, "\033c",
        MUIA_Text_Contents, " ",
        MUIA_Text_Copy,     FALSE,
      End,
    End,

    Child, HGroup,
      MUIA_Weight, 25,
      InnerSpacing(0,0),
      MUIA_Group_Spacing,    0,
      MUIA_Group_SameHeight, TRUE,

      Child, searchOptionPopup = PopobjectObject,
        MUIA_Popstring_Button, PopButton(MUII_PopUp),
        MUIA_Popobject_Object, NListviewObject,
          MUIA_NListview_NList, searchOptionsList = NListObject,
            InputListFrame,
            MUIA_NList_Active,               SO_SUBJORSENDER,
            MUIA_NList_SourceArray,          searchOptions,
            MUIA_NList_AdjustHeight,         TRUE,
            MUIA_NList_AdjustWidth,          TRUE,
            MUIA_NList_DefaultObjectOnClick, FALSE,
          End,
        End,
      End,
      Child, searchString =  BetterStringObject,
        StringFrame,
        MUIA_ControlChar,                   tr(MSG_QUICKSEARCH_STR_CONTROLCHAR)[0],
        MUIA_CycleChain,                    TRUE,
        MUIA_Font,                          MUIV_Font_Tiny,
        MUIA_String_AdvanceOnCR,            FALSE,
        MUIA_BetterString_InactiveContents, tr(MSG_QUICKSEARCH_SO_SUBJORSENDER),
        MUIA_BetterString_NoShortcuts,      TRUE,
        MUIA_BetterString_SelectOnActive,   TRUE,
      End,
      Child, clearButton = TextObject,
        ButtonFrame,
        MUIA_CycleChain,     TRUE,
        MUIA_Font,           MUIV_Font_Tiny,
        MUIA_Text_Contents,  "\033bX",
        MUIA_InputMode,      MUIV_InputMode_RelVerify,
        MUIA_Background,     MUII_ButtonBack,
        MUIA_Text_SetMax,    TRUE,
        MUIA_Text_Copy,      FALSE,
      End,
    End,

    TAG_MORE, inittags(msg))) != NULL)
  {
    GETDATA;

    // per default we set the clear button as hidden
    set(clearButton, MUIA_ShowMe, FALSE);

    data->CY_VIEWOPTIONS = viewOptionCycle;
    data->TX_STATUSTEXT = statusText;
    data->BT_CLEARBUTTON = clearButton;
    data->PO_SEARCHOPTIONPOPUP = searchOptionPopup;
    data->NL_SEARCHOPTIONS = searchOptionsList;
    data->ST_SEARCHSTRING = searchString;

    // set the help text for each GUI element
    SetHelp(data->CY_VIEWOPTIONS,       MSG_HELP_QUICKSEARCH_VIEWOPTIONS);
    SetHelp(data->ST_SEARCHSTRING,      MSG_HELP_QUICKSEARCH_SEARCHSTRING);
    SetHelp(data->PO_SEARCHOPTIONPOPUP, MSG_HELP_QUICKSEARCH_SEARCHOPTIONPOPUP);

    // set notifies
    DoMethod(data->NL_SEARCHOPTIONS,MUIM_Notify, MUIA_NList_DoubleClick, MUIV_EveryTime, obj, 2, MUIM_QuickSearchBar_SearchOptionChanged, MUIV_TriggerValue);
    DoMethod(data->CY_VIEWOPTIONS,  MUIM_Notify, MUIA_Cycle_Active, MUIV_EveryTime, obj, 2, MUIM_QuickSearchBar_ViewOptionChanged, MUIV_TriggerValue);
    DoMethod(data->ST_SEARCHSTRING, MUIM_Notify, MUIA_String_Contents, MUIV_EveryTime, obj, 3, MUIM_QuickSearchBar_SearchContentChanged, MUIV_TriggerValue, FALSE);
    DoMethod(data->ST_SEARCHSTRING, MUIM_Notify, MUIA_String_Acknowledge, MUIV_EveryTime, obj, 3, MUIM_QuickSearchBar_SearchContentChanged, MUIV_TriggerValue, TRUE);
    DoMethod(data->BT_CLEARBUTTON,  MUIM_Notify, MUIA_Pressed, FALSE, data->ST_SEARCHSTRING, 3, MUIM_Set, MUIA_String_Contents, "");
  }

  return (IPTR)obj;
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
      case ATTR(AbortSearch):
      {
        data->abortSearch = tag->ti_Data;
      }
      break;

      // we only disable/enable what is really required to be disables/enabled
      case MUIA_Disabled:
      {
        set(data->CY_VIEWOPTIONS, MUIA_Disabled, tag->ti_Data);
        set(data->PO_SEARCHOPTIONPOPUP, MUIA_Disabled, tag->ti_Data);
        set(data->ST_SEARCHSTRING, MUIA_Disabled, tag->ti_Data);

        if(tag->ti_Data == TRUE)
          set(data->TX_STATUSTEXT, MUIA_Text_Contents, " ");

        // make the superMethod call ignore those tags
        tag->ti_Tag = TAG_IGNORE;
      }
      break;
    }
  }

  return DoSuperMethodA(cl, obj, msg);
}
///
/// OVERLOAD(OM_GET)
OVERLOAD(OM_GET)
{
  GETDATA;
  IPTR *store = ((struct opGet *)msg)->opg_Storage;

  switch(((struct opGet *)msg)->opg_AttrID)
  {
    case ATTR(SearchStringIsActive):
    {
      *store = (Object *)xget(_win(data->ST_SEARCHSTRING), MUIA_Window_ActiveObject) == data->ST_SEARCHSTRING;
      return TRUE;
    }
  }

  return DoSuperMethodA(cl, obj, msg);
}
///

/* Public Methods */
/// DECLARE(SearchContentChanged)
DECLARE(SearchContentChanged) // char *content, ULONG force
{
  GETDATA;

  ENTER();

  ASSERT(data->BT_CLEARBUTTON != NULL);
  ASSERT(G->MA->GUI.PG_MAILLIST != NULL);

  // depending on if there is something to search for
  // we have to prepare something different
  if(msg->content != NULL && msg->content[0] != '\0')
  {
    // we only start the actual search in case a minimum of two
    // characters are specified or the user pressed return explicitly
    if(msg->force == TRUE || msg->content[1] != '\0')
    {
      // make sure the clear button is shown and that
      // the correct mailview is displayed to the user
      set(data->BT_CLEARBUTTON, MUIA_ShowMe, TRUE);

      // now we issue a RestartTimer() command to schedule
      // the actual search in about 400ms from now on
      RestartTimer(TIMER_PROCESSQUICKSEARCH, 0, msg->force ? 1 : 500000, FALSE);
    }
  }
  else
  {
    // we check whether the view option is also set to "all"
    // and if so we clear the whole object
    if(xget(data->CY_VIEWOPTIONS, MUIA_Cycle_Active) == VO_ALL)
    {
      // first we make sure no waiting timer is scheduled
      StopTimer(TIMER_PROCESSQUICKSEARCH);

      // now we switch the ActivePage of the mailview pagegroup
      DoMethod(G->MA->GUI.PG_MAILLIST, MUIM_MainMailListGroup_SwitchToList, LT_MAIN);

      // now reset some other GUI elements as well.
      set(data->TX_STATUSTEXT, MUIA_Text_Contents, " ");
      set(data->BT_CLEARBUTTON, MUIA_ShowMe, FALSE);
    }
    else
    {
      // otherwise we issue a quicksearch start as well
      RestartTimer(TIMER_PROCESSQUICKSEARCH, 0, 500000, FALSE);
    }
  }

  RETURN(0);
  return 0;
}

///
/// DECLARE(SearchOptionChanged)
DECLARE(SearchOptionChanged) // int activeSearchOption
{
  GETDATA;
  char *searchContent = (char *)xget(data->ST_SEARCHSTRING, MUIA_String_Contents);
  const char *inactiveContents;

  ENTER();

  ASSERT(data->NL_SEARCHOPTIONS != NULL);
  ASSERT(G->MA->GUI.PG_MAILLIST != NULL);

  // make sure the popup window is closed
  DoMethod(data->PO_SEARCHOPTIONPOPUP, MUIM_Popstring_Close, TRUE);

  // update the inactive search string accordingly
  switch(msg->activeSearchOption)
  {
    case 0: inactiveContents = tr(MSG_QUICKSEARCH_SO_SUBJECT); break;
    case 1: inactiveContents = tr(MSG_QUICKSEARCH_SO_SENDER); break;
    case 2: inactiveContents = tr(MSG_QUICKSEARCH_SO_SUBJORSENDER); break;
    case 3: inactiveContents = tr(MSG_QUICKSEARCH_SO_TOORCC); break;
    case 4: inactiveContents = tr(MSG_QUICKSEARCH_SO_INMSG); break;
    case 5: inactiveContents = tr(MSG_QUICKSEARCH_SO_ENTIREMSG); break;
    default: inactiveContents = NULL; break;
  }
  set(data->ST_SEARCHSTRING, MUIA_BetterString_InactiveContents, inactiveContents);

  // now we check whether the there is something to search for or not.
  if(searchContent != NULL && searchContent[0] != '\0')
  {
    // immediately process the search, but make sure there is no
    // pending timerIO waiting already
    StopTimer(TIMER_PROCESSQUICKSEARCH);
    DoMethod(obj, MUIM_QuickSearchBar_ProcessSearch);
  }

  RETURN(0);
  return 0;
}

///
/// DECLARE(ViewOptionChanged)
DECLARE(ViewOptionChanged) // int activeCycle
{
  GETDATA;
  char *searchContent = (char *)xget(data->ST_SEARCHSTRING, MUIA_String_Contents);

  ENTER();

  ASSERT(data->CY_VIEWOPTIONS != NULL);
  ASSERT(G->MA->GUI.PG_MAILLIST != NULL);

  // set the active group of the MAILVIEW pageGroup to 1 if one of the view
  // options is selected by the user
  if(msg->activeCycle == VO_ALL && (searchContent == NULL || searchContent[0] == '\0'))
  {
    DoMethod(obj, MUIM_QuickSearchBar_Clear);
  }
  else
  {
    // immediately process the search, but make sure there is no
    // pending timerIO waiting already
    StopTimer(TIMER_PROCESSQUICKSEARCH);
    DoMethod(obj, MUIM_QuickSearchBar_ProcessSearch);
  }

  RETURN(0);
  return 0;
}

///
/// DECLARE(ProcessSearch)
DECLARE(ProcessSearch)
{
  GETDATA;
  struct Folder *curFolder = GetCurrentFolder();

  ENTER();

  ASSERT(curFolder != NULL);

  // a use of the quicksearchbar is only possible on
  // normal folders
  if(!isGroupFolder(curFolder))
  {
    struct MailNode *mnode;
    enum ViewOptions viewOption = xget(data->CY_VIEWOPTIONS, MUIA_Cycle_Active);
    enum SearchOptions searchOption = xget(data->NL_SEARCHOPTIONS, MUIA_NList_Active);
    char *searchString = (char *)xget(data->ST_SEARCHSTRING, MUIA_String_Contents);
    struct TimeVal curTimeUTC;
    struct BoyerMooreContext *bmContext;
    struct BusyNode *busy;

    // get the current time in UTC
    GetSysTimeUTC(&curTimeUTC);

    // check the searchString settings for an empty string
    if(searchString != NULL && searchString[0] == '\0')
      searchString = NULL;

    // initialize a case insensitive Boyer/Moore search, searchString may be NULL
    bmContext = BoyerMooreInit(searchString, FALSE);

    // make sure the correct mailview list is visible and quiet
    DoMethod(G->MA->GUI.PG_MAILLIST, MUIM_MainMailListGroup_SwitchToList, LT_QUICKVIEW);
    set(G->MA->GUI.PG_MAILLIST, MUIA_NList_Quiet, TRUE);

    // reset any previous abortion
    data->abortSearch = FALSE;
    data->searchInProgress = TRUE;

    // now we can process the search/sorting by searching the mail list of the
    // current folder querying different criterias of a mail
    LockMailListShared(curFolder->messages);

    busy = BusyBegin(BUSY_TEXT);
    BusyText(busy, tr(MSG_BUSY_SEARCHINGFOLDER), curFolder->Name);

    ForEachMailNode(curFolder->messages, mnode)
    {
      struct Mail *curMail = mnode->mail;

      // check if that mail matches the search/view criteria
      if(MatchMail(curMail, viewOption, searchOption, bmContext, &curTimeUTC) == TRUE)
        DoMethod(G->MA->GUI.PG_MAILLIST, MUIM_MainMailListGroup_AddMailToList, LT_QUICKVIEW, curMail);

      DoMethod(_app(obj), MUIM_Application_InputBuffered);

      if(data->abortSearch == TRUE)
        break;
    }
    BusyEnd(busy);

    UnlockMailList(curFolder->messages);

    BoyerMooreCleanup(bmContext);

    // only update the GUI if this search was not aborted
    if(data->abortSearch == FALSE)
    {
      struct Mail *lastActiveMail;
      LONG pos = MUIV_NList_GetPos_Start;

      // make sure the statistics are updated as well
      DoMethod(obj, MUIM_QuickSearchBar_UpdateStats, TRUE);

      // get the last active mail in the maillistgroup
      lastActiveMail = (struct Mail *)xget(G->MA->GUI.PG_MAILLIST, MUIA_MainMailListGroup_LastActiveMail);
      if(lastActiveMail != NULL)
      {
        // retrieve the number of the lastActive entry within the main mail listview
        DoMethod(G->MA->GUI.PG_MAILLIST, MUIM_NList_GetPos, lastActiveMail, &pos);
      }

      // make sure to set a new message so that the mail view is updated
      xset(G->MA->GUI.PG_MAILLIST, MUIA_NList_Active,       pos != MUIV_NList_GetPos_End && pos != MUIV_NList_GetPos_Start ? pos : MUIV_NList_Active_Top,
                                   MUIA_NList_SelectChange, TRUE);
    }

    // finally de-quiet the list again
    set(G->MA->GUI.PG_MAILLIST, MUIA_NList_Quiet, FALSE);
    data->searchInProgress = FALSE;
  }
  else
    E(DBF_ALL, "curFolder->Type == FT_GROUP ?????");

  RETURN(0);
  return 0;
}

///
/// DECLARE(MatchMail)
// method to query the quick search bar to make a match if a certain mail matches
// the currently active criteria
DECLARE(MatchMail) // struct Mail *mail
{
  GETDATA;
  enum ViewOptions viewOption = xget(data->CY_VIEWOPTIONS, MUIA_Cycle_Active);
  enum SearchOptions searchOption = xget(data->NL_SEARCHOPTIONS, MUIA_NList_Active);
  char *searchString = (char *)xget(data->ST_SEARCHSTRING, MUIA_String_Contents);
  struct TimeVal curTimeUTC;
  struct BoyerMooreContext *bmContext;
  ULONG match;

  // get the current time in UTC
  GetSysTimeUTC(&curTimeUTC);

  // check the searchString settings for an empty string
  if(searchString != NULL && searchString[0] == '\0')
    searchString = NULL;

  // initialize a case insensitive Boyer/Moore search, searchString may be NULL
  bmContext = BoyerMooreInit(searchString, FALSE);

  // now we check that a match is really required and if so we process it
  match = (ULONG)((viewOption != VO_ALL || searchString != NULL) &&
                  MatchMail(msg->mail, viewOption, searchOption, bmContext, &curTimeUTC) == TRUE);

  BoyerMooreCleanup(bmContext);

  return match;
}

///
/// DECLARE(Clear)
// This method makes sure all quickbar entries are cleared and that the
// search NList is cleared as well
DECLARE(Clear)
{
  GETDATA;

  ENTER();

  // first we make sure no waiting timer is scheduled
  StopTimer(TIMER_PROCESSQUICKSEARCH);

  // now we switch the ActivePage of the mailview pagegroup
  DoMethod(G->MA->GUI.PG_MAILLIST, MUIM_MainMailListGroup_SwitchToList, LT_MAIN);

  // now we reset the quickbar's GUI elements
  nnset(data->ST_SEARCHSTRING, MUIA_String_Contents, "");
  nnset(data->CY_VIEWOPTIONS, MUIA_Cycle_Active, VO_ALL);
  set(data->TX_STATUSTEXT, MUIA_Text_Contents, " ");
  set(data->BT_CLEARBUTTON, MUIA_ShowMe, FALSE);
  set(data->NL_SEARCHOPTIONS, MUIA_NList_Active, SO_SUBJORSENDER);

  // make sure our objects are not disabled
  set(obj, MUIA_Disabled, FALSE);

  RETURN(0);
  return 0;
}

///
/// DECALRE(UpdateStats)
DECLARE(UpdateStats) // ULONG force
{
  GETDATA;
  BOOL doUpdate = FALSE;

  ENTER();

  // now we check whether the user forces the update or if
  // we have to check that the display is not update too often
  if(msg->force == TRUE)
    doUpdate = TRUE;
  else
  {
    // then we update the gauge, but we take also care of not refreshing
    // it too often or otherwise it slows down the whole search process.
    doUpdate = TimeHasElapsed(&data->last_statusupdate, 250000);
  }

  if(doUpdate == TRUE)
  {
    ULONG numEntries = xget(G->MA->GUI.PG_MAILLIST, MUIA_NList_Entries);
    struct Folder *curFolder = GetCurrentFolder();

    snprintf(data->statusText, sizeof(data->statusText), tr(MSG_QUICKSEARCH_SHOWNMSGS), numEntries, curFolder->Total);

    // if the list is quiet then leave that state
    if(data->searchInProgress == TRUE)
      set(G->MA->GUI.PG_MAILLIST, MUIA_NList_Quiet, FALSE);
    set(data->TX_STATUSTEXT, MUIA_Text_Contents, data->statusText);
    // and restore the previous state if we changed it
    if(data->searchInProgress == TRUE)
      set(G->MA->GUI.PG_MAILLIST, MUIA_NList_Quiet, TRUE);
  }

  RETURN(0);
  return 0;
}

///
/// DECLARE(DoEditAction)
DECLARE(DoEditAction) // enum EditAction editAction
{
  GETDATA;
  Object *selectedObj = NULL;
  Object *windowObj = _win(obj);
  BOOL result = FALSE;

  ENTER();

  // we first check which object is current selected
  // as the 'active' Object
  if(windowObj != NULL)
  {
    selectedObj = (Object *)xget(windowObj, MUIA_Window_ActiveObject);
    if(selectedObj == NULL)
      selectedObj = (Object *)xget(windowObj, MUIA_Window_DefaultObject);
  }

  // if we still haven't got anything selected
  // something must be extremly strange ;)
  if(selectedObj != NULL)
  {
    // check which action we got
    switch(msg->editAction)
    {
      case EA_CUT:
      {
        if(selectedObj == data->ST_SEARCHSTRING)
        {
          DoMethod(selectedObj, MUIM_BetterString_DoAction, MUIV_BetterString_DoAction_Cut);
          result = TRUE;
        }
      }
      break;

      case EA_COPY:
      {
        if(selectedObj == data->ST_SEARCHSTRING)
        {
          DoMethod(selectedObj, MUIM_BetterString_DoAction, MUIV_BetterString_DoAction_Copy);
          result = TRUE;
        }
      }
      break;

      case EA_PASTE:
      {
        if(selectedObj == data->ST_SEARCHSTRING)
        {
          DoMethod(selectedObj, MUIM_BetterString_DoAction, MUIV_BetterString_DoAction_Paste);
          result = TRUE;
        }
      }
      break;

      case EA_DELETE:
      {
        if(selectedObj == data->ST_SEARCHSTRING)
        {
          DoMethod(selectedObj, MUIM_BetterString_DoAction, MUIV_BetterString_DoAction_Delete);
          result = TRUE;
        }
      }
      break;

      case EA_UNDO:
      {
        if(selectedObj == data->ST_SEARCHSTRING)
        {
          DoMethod(selectedObj, MUIM_BetterString_DoAction, MUIV_BetterString_DoAction_Undo);
          result = TRUE;
        }
      }
      break;

      case EA_REDO:
      {
        if(selectedObj == data->ST_SEARCHSTRING)
        {
          DoMethod(selectedObj, MUIM_BetterString_DoAction, MUIV_BetterString_DoAction_Redo);
          result = TRUE;
        }
      }
      break;

      case EA_SELECTALL:
      {
        if(selectedObj == data->ST_SEARCHSTRING)
        {
          DoMethod(selectedObj, MUIM_BetterString_DoAction, MUIV_BetterString_DoAction_SelectAll);
          result = TRUE;
        }
      }
      break;

      case EA_SELECTNONE:
      {
        if(selectedObj == data->ST_SEARCHSTRING)
        {
          DoMethod(selectedObj, MUIM_BetterString_DoAction, MUIV_BetterString_DoAction_SelectNone);
          result = TRUE;
        }
      }
      break;
    }
  }

  RETURN(result);
  return result;
}

///

