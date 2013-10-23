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
#include "YAM_find.h"
#include "YAM_mainFolder.h"
#include "YAM_utilities.h"

#include "BoyerMooreSearch.h"
#include "Busy.h"
#include "DynamicString.h"
#include "Locale.h"
#include "MailList.h"
#include "MUIObjects.h"

#include "mui/AddressBookWindow.h"
#include "mui/MainMailListGroup.h"

#include "Debug.h"

/* CLASSDATA
struct Data
{
  Object *CY_VIEWOPTIONS;
  Object *TX_STATUSTEXT;
  Object *ST_SEARCHSTRING;
  Object *BT_CLEAR;
  Object *GR_WHERE;
  Object *BT_FROM;
  Object *BT_TO;
  Object *BT_SUBJECT;
  Object *BT_BODY;
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
enum ViewOptions
{
  VO_ALL=0,
  VO_UNREAD,
  VO_NEW,
  VO_MARKED,
  VO_IMPORTANT,
  VO_LAST5DAYS,
  VO_KNOWNPEOPLE,
  VO_HASATTACHMENTS,
  VO_MINSIZE
};

enum SearchFlags
{
  SF_FROM=(1<<0),
  SF_TO=(1<<1),
  SF_SUBJECT=(1<<2),
  SF_BODY=(1<<3)
};

/* Private Functions */
/// MatchMail()
// function to actually check if a struct Mail* matches
// the currently active criteria
static BOOL MatchMail(const struct Mail *mail, enum ViewOptions vo,
                      ULONG searchFlags, const struct BoyerMooreContext *bmContext, struct TimeVal *curTimeUTC)
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
      foundMatch = (FindPersonInABook(&G->abook, &mail->From) != NULL);
      if(foundMatch == FALSE && isMultiSenderMail(mail))
      {
        struct ExtendedMail *email;

        if((email = MA_ExamineMail(mail->Folder, mail->MailFile, TRUE)) != NULL)
        {
          int j;

          for(j=0; j < email->NumSFrom && foundMatch == FALSE; j++)
          {
            foundMatch = (FindPersonInABook(&G->abook, &email->SFrom[j]) != NULL);
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
    foundMatch = FALSE;

    // first check the simple things
    if(foundMatch == FALSE && isFlagSet(searchFlags, SF_FROM))
    {
      foundMatch = (BoyerMooreSearch(bmContext, mail->From.Address) != NULL ||
                    BoyerMooreSearch(bmContext, mail->From.RealName) != NULL);
    }
    if(foundMatch == FALSE && isFlagSet(searchFlags, SF_TO))
    {
      foundMatch = (BoyerMooreSearch(bmContext, mail->To.Address) != NULL ||
                    BoyerMooreSearch(bmContext, mail->To.RealName) != NULL);
    }
    if(foundMatch == FALSE && isFlagSet(searchFlags, SF_SUBJECT))
    {
      foundMatch = (BoyerMooreSearch(bmContext, mail->Subject) != NULL);
    }

    // now check the slightly more complex things
    if(foundMatch == FALSE)
    {
      if((isFlagSet(searchFlags, SF_FROM) && isMultiSenderMail(mail)) ||
         (isFlagSet(searchFlags, SF_TO) && isMultiRCPTMail(mail)))
      {
        struct ExtendedMail *email;

        if((email = MA_ExamineMail(mail->Folder, mail->MailFile, TRUE)) != NULL)
        {
          if(isFlagSet(searchFlags, SF_FROM))
          {
            int j;

            // search the additional From: addresses

            for(j=0; j < email->NumSFrom && foundMatch == FALSE; j++)
            {
              struct Person *pe = &email->SFrom[j];

              foundMatch = (BoyerMooreSearch(bmContext, pe->Address) != NULL ||
                            BoyerMooreSearch(bmContext, pe->RealName) != NULL);
            }
          }
          if(isFlagSet(searchFlags, SF_TO))
          {
            int j;

            // search the additional To: addresses
            for(j=0; j < email->NumSTo && foundMatch == FALSE; j++)
            {
              struct Person *to = &email->STo[j];

              foundMatch = (BoyerMooreSearch(bmContext, to->Address) != NULL ||
                            BoyerMooreSearch(bmContext, to->RealName) != NULL);
            }
            // search the CC: addresses
            for(j=0; j < email->NumCC && foundMatch == FALSE; j++)
            {
              struct Person *cc = &email->CC[j];

              foundMatch = (BoyerMooreSearch(bmContext, cc->Address) != NULL ||
                            BoyerMooreSearch(bmContext, cc->RealName) != NULL);
            }
          }

          MA_FreeEMailStruct(email);
        }
      }
    }

    // finally the most complex part, check the message contents
    if(foundMatch == FALSE && isFlagSet(searchFlags, SF_BODY))
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
  }

  RETURN(foundMatch);
  return foundMatch;
}

///

/* Overloaded Methods */
/// OVERLOAD(OM_NEW)
OVERLOAD(OM_NEW)
{
  Object *CY_VIEWOPTIONS;
  Object *TX_STATUSTEXT;
  Object *ST_SEARCHSTRING;
  Object *BT_CLEAR;
  Object *GR_WHERE;
  Object *BT_FROM;
  Object *BT_TO;
  Object *BT_SUBJECT;
  Object *BT_BODY;
  static const char *viewOptions[10];

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

    MUIA_Group_Horiz, FALSE,
    Child, HGroup,
      Child, HGroup,
        MUIA_Weight, 25,
        InnerSpacing(0,0),
        Child, MUI_MakeObject(MUIO_Label, (ULONG)tr(MSG_QUICKSEARCH_VIEW), MUIO_Label_Tiny),
        Child, CY_VIEWOPTIONS = CycleObject,
          MUIA_Font,          MUIV_Font_Tiny,
          MUIA_CycleChain,    TRUE,
          MUIA_Cycle_Entries, viewOptions,
          MUIA_ControlChar,   ShortCut(tr(MSG_QUICKSEARCH_VIEW)),
        End,
      End,

      Child, HGroup,
        MUIA_Weight, 50,
        Child, TX_STATUSTEXT = TextObject,
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

        Child, ST_SEARCHSTRING =  BetterStringObject,
          StringFrame,
          MUIA_ControlChar,                   tr(MSG_QUICKSEARCH_STR_CONTROLCHAR)[0],
          MUIA_CycleChain,                    TRUE,
          MUIA_Font,                          MUIV_Font_Tiny,
          MUIA_String_AdvanceOnCR,            FALSE,
          MUIA_BetterString_InactiveContents, tr(MSG_QUICKSEARCH_FILTER_LIST),
          MUIA_BetterString_NoShortcuts,      TRUE,
          MUIA_BetterString_SelectOnActive,   TRUE,
        End,
        Child, BT_CLEAR = TextObject,
          ButtonFrame,
          MUIA_ShowMe,         FALSE,
          MUIA_CycleChain,     TRUE,
          MUIA_Font,           MUIV_Font_Tiny,
          MUIA_Text_PreParse,  "\033b",
          MUIA_Text_Contents,  "X",
          MUIA_InputMode,      MUIV_InputMode_RelVerify,
          MUIA_Background,     MUII_ButtonBack,
          MUIA_Text_SetMax,    TRUE,
          MUIA_Text_Copy,      FALSE,
        End,
      End,
    End,
    Child, GR_WHERE = HGroup,
      MUIA_ShowMe, FALSE,
      Child, HSpace(0),
      Child, BT_FROM = TextObject,
        ButtonFrame,
        MUIA_CycleChain,    TRUE,
        MUIA_Font,          MUIV_Font_Tiny,
        MUIA_InputMode,     MUIV_InputMode_Toggle,
        MUIA_Background,    MUII_ButtonBack,
        MUIA_Selected,      TRUE,
        MUIA_Text_Contents, tr(MSG_QUICKSEARCH_FROM),
        MUIA_Text_SetMax,   TRUE,
        MUIA_Text_Copy,     FALSE,
      End,
      Child, BT_TO = TextObject,
        ButtonFrame,
        MUIA_CycleChain,    TRUE,
        MUIA_Font,          MUIV_Font_Tiny,
        MUIA_InputMode,     MUIV_InputMode_Toggle,
        MUIA_Background,    MUII_ButtonBack,
        MUIA_Selected,      TRUE,
        MUIA_Text_Contents, tr(MSG_QUICKSEARCH_TO),
        MUIA_Text_SetMax,   TRUE,
        MUIA_Text_Copy,     FALSE,
      End,
      Child, BT_SUBJECT = TextObject,
        ButtonFrame,
        MUIA_CycleChain,    TRUE,
        MUIA_Font,          MUIV_Font_Tiny,
        MUIA_InputMode,     MUIV_InputMode_Toggle,
        MUIA_Background,    MUII_ButtonBack,
        MUIA_Selected,      TRUE,
        MUIA_Text_Contents, tr(MSG_QUICKSEARCH_SUBJECT),
        MUIA_Text_SetMax,   TRUE,
        MUIA_Text_Copy,     FALSE,
      End,
      Child, BT_BODY = TextObject,
        ButtonFrame,
        MUIA_CycleChain,    TRUE,
        MUIA_Font,          MUIV_Font_Tiny,
        MUIA_InputMode,     MUIV_InputMode_Toggle,
        MUIA_Background,    MUII_ButtonBack,
        MUIA_Selected,      FALSE,
        MUIA_Text_Contents, tr(MSG_QUICKSEARCH_BODY),
        MUIA_Text_SetMax,   TRUE,
        MUIA_Text_Copy,     FALSE,
      End,
    End,

    TAG_MORE, inittags(msg))) != NULL)
  {
    GETDATA;

    data->CY_VIEWOPTIONS  = CY_VIEWOPTIONS;
    data->TX_STATUSTEXT   = TX_STATUSTEXT;
    data->ST_SEARCHSTRING = ST_SEARCHSTRING;
    data->BT_CLEAR        = BT_CLEAR;
    data->GR_WHERE        = GR_WHERE;
    data->BT_FROM         = BT_FROM;
    data->BT_TO           = BT_TO;
    data->BT_SUBJECT      = BT_SUBJECT;
    data->BT_BODY         = BT_BODY;

    // set the help text for each GUI element
    SetHelp(CY_VIEWOPTIONS,  MSG_HELP_QUICKSEARCH_VIEWOPTIONS);
    SetHelp(ST_SEARCHSTRING, MSG_HELP_QUICKSEARCH_SEARCHSTRING);

    // set notifies
    DoMethod(CY_VIEWOPTIONS,  MUIM_Notify, MUIA_Cycle_Active,       MUIV_EveryTime, obj,             2, METHOD(ViewOptionChanged), MUIV_TriggerValue);
    DoMethod(ST_SEARCHSTRING, MUIM_Notify, MUIA_String_Contents,    MUIV_EveryTime, obj,             3, METHOD(SearchContentChanged), MUIV_TriggerValue, FALSE);
    DoMethod(ST_SEARCHSTRING, MUIM_Notify, MUIA_String_Acknowledge, MUIV_EveryTime, obj,             3, METHOD(SearchContentChanged), MUIV_TriggerValue, TRUE);
    DoMethod(BT_CLEAR,        MUIM_Notify, MUIA_Pressed,            FALSE,          ST_SEARCHSTRING, 3, MUIM_Set, MUIA_String_Contents, "");
    DoMethod(BT_FROM,         MUIM_Notify, MUIA_Selected,           MUIV_EveryTime, obj,             1, METHOD(SearchFlagsChanged));
    DoMethod(BT_TO,           MUIM_Notify, MUIA_Selected,           MUIV_EveryTime, obj,             1, METHOD(SearchFlagsChanged));
    DoMethod(BT_SUBJECT,      MUIM_Notify, MUIA_Selected,           MUIV_EveryTime, obj,             1, METHOD(SearchFlagsChanged));
    DoMethod(BT_BODY,         MUIM_Notify, MUIA_Selected,           MUIV_EveryTime, obj,             1, METHOD(SearchFlagsChanged));
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

        // make the superMethod call ignore those tags
        tag->ti_Tag = TAG_IGNORE;
      }
      break;

      // we only disable/enable what is really required to be disables/enabled
      case MUIA_Disabled:
      {
        set(data->CY_VIEWOPTIONS, MUIA_Disabled, tag->ti_Data);
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

  // abort any running search process
  set(obj, ATTR(AbortSearch), TRUE);

  // depending on if there is something to search for
  // we have to prepare something different
  if(IsStrEmpty(msg->content) == FALSE)
  {
    // we only start the actual search in case a minimum of two
    // characters are specified or the user pressed return explicitly
    if(msg->force == TRUE || msg->content[1] != '\0')
    {
      // make sure the clear button is shown and that
      // the correct mailview is displayed to the user
      set(data->BT_CLEAR, MUIA_ShowMe, TRUE);
      set(data->GR_WHERE, MUIA_ShowMe, TRUE);

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
      set(data->BT_CLEAR, MUIA_ShowMe, FALSE);
      set(data->GR_WHERE, MUIA_ShowMe, FALSE);
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
/// DECLARE(ViewOptionChanged)
DECLARE(ViewOptionChanged) // int activeCycle
{
  GETDATA;
  char *searchContent = (char *)xget(data->ST_SEARCHSTRING, MUIA_String_Contents);

  ENTER();

  // abort any running search process
  set(obj, ATTR(AbortSearch), TRUE);

  // set the active group of the MAILVIEW pageGroup to 1 if one of the view
  // options is selected by the user
  if(msg->activeCycle == VO_ALL && (searchContent == NULL || searchContent[0] == '\0'))
  {
    DoMethod(obj, METHOD(Clear));
  }
  else
  {
    // immediately process the search, but make sure there is no
    // pending timerIO waiting already
    StopTimer(TIMER_PROCESSQUICKSEARCH);
    DoMethod(obj, METHOD(ProcessSearch));
  }

  RETURN(0);
  return 0;
}

///
/// DECLARE(SearchFlagsChanged)
DECLARE(SearchFlagsChanged)
{
  ENTER();

  // abort any running search process
  set(obj, ATTR(AbortSearch), TRUE);

  // immediately process the search, but make sure there is no
  // pending timerIO waiting already
  StopTimer(TIMER_PROCESSQUICKSEARCH);
  DoMethod(obj, METHOD(ProcessSearch));

  RETURN(0);
  return 0;
}

///
/// DECLARE(ProcessSearch)
DECLARE(ProcessSearch)
{
  GETDATA;
  struct Folder *curFolder;

  ENTER();

  curFolder = GetCurrentFolder();

  // a use of the quicksearchbar is only possible on
  // normal folders
  if(!isGroupFolder(curFolder))
  {
    struct MailNode *mnode;
    enum ViewOptions viewOption = xget(data->CY_VIEWOPTIONS, MUIA_Cycle_Active);
    ULONG searchFlags;
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

    searchFlags = 0;
    if(xget(data->BT_FROM, MUIA_Selected) == TRUE)
      setFlag(searchFlags, SF_FROM);
    if(xget(data->BT_TO, MUIA_Selected) == TRUE)
      setFlag(searchFlags, SF_TO);
    if(xget(data->BT_SUBJECT, MUIA_Selected) == TRUE)
      setFlag(searchFlags, SF_SUBJECT);
    if(xget(data->BT_BODY, MUIA_Selected) == TRUE)
      setFlag(searchFlags, SF_BODY);

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
      if(MatchMail(curMail, viewOption, searchFlags, bmContext, &curTimeUTC) == TRUE)
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
      DoMethod(obj, METHOD(UpdateStats), TRUE);

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
  ULONG searchFlags;
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

  searchFlags = 0;
  if(xget(data->BT_FROM, MUIA_Selected) == TRUE)
    setFlag(searchFlags, SF_FROM);
  if(xget(data->BT_TO, MUIA_Selected) == TRUE)
    setFlag(searchFlags, SF_TO);
  if(xget(data->BT_SUBJECT, MUIA_Selected) == TRUE)
    setFlag(searchFlags, SF_SUBJECT);
  if(xget(data->BT_BODY, MUIA_Selected) == TRUE)
    setFlag(searchFlags, SF_BODY);

  // now we check that a match is really required and if so we process it
  match = (ULONG)((viewOption != VO_ALL || searchString != NULL) &&
                  MatchMail(msg->mail, viewOption, searchFlags, bmContext, &curTimeUTC) == TRUE);

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
  set(data->BT_CLEAR, MUIA_ShowMe, FALSE);

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

