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

***************************************************************************/

#include <stdlib.h>
#include <string.h>

#include <clib/alib_protos.h>
#include <libraries/asl.h>
#include <libraries/iffparse.h>
#include <mui/NBalance_mcc.h>
#include <mui/NList_mcc.h>
#include <mui/NListview_mcc.h>
#include <proto/dos.h>
#include <proto/exec.h>
#include <proto/intuition.h>
#include <proto/muimaster.h>
#include <proto/timer.h>
#include <proto/utility.h>

#include "extrasrc.h"

#include "SDI_hook.h"

#include "YAM.h"
#include "YAM_config.h"
#include "YAM_configFile.h"
#include "YAM_error.h"
#include "YAM_find.h"
#include "YAM_folderconfig.h"
#include "YAM_global.h"
#include "YAM_main.h"
#include "YAM_mainFolder.h"
#include "YAM_read.h"
#include "YAM_utilities.h"
#include "mui/Classes.h"

#include "BayesFilter.h"
#include "BoyerMooreSearch.h"
#include "Locale.h"
#include "MailList.h"
#include "FolderList.h"
#include "MUIObjects.h"
#include "Requesters.h"
#include "Threads.h"

#include "Debug.h"

/* local protos */
static struct FI_ClassData *FI_New(void);
static BOOL CopySearchData(struct Search *dstSearch, struct Search *srcSearch);
static void FreeSearchPatternList(struct Search *search);

/***************************************************************************
 Module: Find & Filters
***************************************************************************/

/// Global variables
//
// The following array is a static map of the different unique statuses a mail
// can have. It is used by the Find Cycle gadget to map the different statuses:
// U - New/Unread
// O - Old/Read
// F - Forwarded
// R - Replied
// W - WaitForSend (Queued)
// E - Error
// H - Hold
// S - Sent
// M - Marked/Flagged
// X - Spam
const char mailStatusCycleMap[11] = { 'U', 'O', 'F', 'R', 'W', 'E', 'H', 'S', 'M', 'X', '\0' };

///
/// FI_MatchString
//  Matches string against pattern
static BOOL FI_MatchString(struct Search *search, char *string)
{
  BOOL match;

  ENTER();

  switch(search->Compare)
  {
    case 0:
    {
      if(search->DOSPattern == TRUE)
      {
        if(search->CaseSens == TRUE)
           match = (BOOL)MatchPattern(search->Pattern, string);
         else
           match = (BOOL)MatchPatternNoCase(search->Pattern, string);
      }
      else
      {
        match = (BOOL)(BoyerMooreSearch(search->bmContext, string) != NULL);
      }
    }
    break;

    case 1:
    {
      if(search->DOSPattern == TRUE)
      {
        if(search->CaseSens == TRUE)
           match = (BOOL)!MatchPattern(search->Pattern, string);
         else
           match = (BOOL)!MatchPatternNoCase(search->Pattern, string);
      }
      else
      {
        match = (BOOL)(BoyerMooreSearch(search->bmContext, string) == NULL);
      }
    }
    break;

    case 2:
    {
      if(search->CaseSens == TRUE)
        match = (BOOL)(strcmp(string, search->Match) < 0);
      else
        match = (BOOL)(Stricmp(string, search->Match) < 0);
    }
    break;

    case 3:
    {
      if(search->CaseSens == TRUE)
        match = (BOOL)(strcmp(string, search->Match) > 0);
      else
        match = (BOOL)(Stricmp(string, search->Match) > 0);
    }
    break;

    default:
    {
      match = FALSE;
    }
    break;

  }

  RETURN(match);
  return match;
}

///
/// FI_MatchListPattern
//  Matches string against a list of patterns
static BOOL FI_MatchListPattern(struct Search *search, char *string)
{
  BOOL match = FALSE;
  struct Node *curNode;

  ENTER();

  IterateList(&search->patternList, curNode)
  {
    struct SearchPatternNode *patternNode = (struct SearchPatternNode *)curNode;

    if(search->CaseSens == TRUE)
      match = (BOOL)MatchPattern(patternNode->pattern, string);
    else
      match = (BOOL)MatchPatternNoCase(patternNode->pattern, string);

    if(match == TRUE)
      break;
  }

  RETURN(match);
  return match;
}

///
/// FI_MatchPerson
//  Matches string against a person's name or address
static BOOL FI_MatchPerson(struct Search *search, struct Person *pe)
{
  BOOL match;

  ENTER();

  if(search->Compare == 4)
    match = FI_MatchListPattern(search, search->PersMode ? pe->RealName : pe->Address);
  else
    match = FI_MatchString(search, search->PersMode ? pe->RealName : pe->Address);

  RETURN(match);
  return match;
}

///
/// FI_SearchPatternFast
//  Searches string in standard header fields
static BOOL FI_SearchPatternFast(struct Search *search, struct Mail *mail)
{
  BOOL found = FALSE;

  ENTER();

  switch(search->Fast)
  {
    // search all "From:" addresses
    case FS_FROM:
    {
      struct ExtendedMail *email;

      if(FI_MatchPerson(search, &mail->From))
        found = TRUE;
      else if(isMultiSenderMail(mail) && (email = MA_ExamineMail(mail->Folder, mail->MailFile, TRUE)) != NULL)
      {
        int i;

        for(i=0; i < email->NoSFrom; i++)
        {
          if(FI_MatchPerson(search, &email->SFrom[i]) == TRUE)
          {
            found = TRUE;
            break;
          }
        }

        MA_FreeEMailStruct(email);
      }
    }
    break;

    // search all "To:" addresses
    case FS_TO:
    {
      struct ExtendedMail *email;

      if(FI_MatchPerson(search, &mail->To))
        found = TRUE;
      else if(isMultiRCPTMail(mail) && (email = MA_ExamineMail(mail->Folder, mail->MailFile, TRUE)) != NULL)
      {
        int i;

        for(i=0; i < email->NoSTo; i++)
        {
          if(FI_MatchPerson(search, &email->STo[i]) == TRUE)
          {
            found = TRUE;
            break;
          }
        }

        MA_FreeEMailStruct(email);
      }
    }
    break;

    // search all "Cc:" addresses
    case FS_CC:
    {
      struct ExtendedMail *email;

      if(isMultiRCPTMail(mail) && (email = MA_ExamineMail(mail->Folder, mail->MailFile, TRUE)) != NULL)
      {
        int i;

        for(i=0; i < email->NoCC; i++)
        {
          if(FI_MatchPerson(search, &email->CC[i]) == TRUE)
          {
            found = TRUE;
            break;
          }
        }

        MA_FreeEMailStruct(email);
      }
    }
    break;

    // search all "ReplyTo:" addresses
    case FS_REPLYTO:
    {
      struct ExtendedMail *email;

      if(FI_MatchPerson(search, &mail->ReplyTo))
        found = TRUE;
      else if(isMultiReplyToMail(mail) && (email = MA_ExamineMail(mail->Folder, mail->MailFile, TRUE)) != NULL)
      {
        int i;

        for(i=0; i < email->NoSReplyTo; i++)
        {
          if(FI_MatchPerson(search, &email->SReplyTo[i]) == TRUE)
          {
            found = TRUE;
            break;
          }
        }

        MA_FreeEMailStruct(email);
      }
    }
    break;

    // search the "Subject:" line
    case FS_SUBJECT:
    {
      if(search->Compare == 4)
        found = FI_MatchListPattern(search, mail->Subject);
      else
        found = FI_MatchString(search, mail->Subject);
    }
    break;

    // search the "Date:" line
    case FS_DATE:
    {
      struct DateStamp *pdat = (struct DateStamp *)search->Pattern;

      int cmp = (pdat->ds_Minute) ? CompareDates(&(mail->Date), pdat) : pdat->ds_Days-mail->Date.ds_Days;
      if((search->Compare == 0 && cmp == 0) ||
         (search->Compare == 1 && cmp != 0) ||
         (search->Compare == 2 && cmp > 0) ||
         (search->Compare == 3 && cmp < 0))
      {
        found = TRUE;
      }
    }
    break;

    // search the message size
    case FS_SIZE:
    {
      long cmp = search->Size - mail->Size;

      if((search->Compare == 0 && cmp == 0) ||
         (search->Compare == 1 && cmp != 0) ||
         (search->Compare == 2 && cmp > 0) ||
         (search->Compare == 3 && cmp < 0))
      {
        found = TRUE;
      }
    }
    break;

    default:
      // nothing
    break;
  }

  RETURN(found);
  return found;
}

///
/// FI_SearchPatternInBody
//  Searches string in message body
static BOOL FI_SearchPatternInBody(struct Search *search, struct Mail *mail)
{
  BOOL found = FALSE;
  struct ReadMailData *rmData;

  ENTER();

  if((rmData = AllocPrivateRMData(mail, PM_TEXTS|PM_QUIET)) != NULL)
  {
    char *rptr, *ptr, *cmsg;

    rptr = cmsg = RE_ReadInMessage(rmData, RIM_QUIET);

    while(*rptr != '\0' && found == FALSE && (G->FI != NULL ? !G->FI->Abort : TRUE))
    {
      for(ptr = rptr; *ptr && *ptr != '\n'; ptr++);

      *ptr = 0;
      if(FI_MatchString(search, rptr))
        found = TRUE;

      rptr = ++ptr;
    }

    if(G->FI != NULL && G->FI->Abort != FALSE)
      D(DBF_FILTER, "search was aborted");

    free(cmsg);
    FreePrivateRMData(rmData);
  }

  RETURN(found);
  return found;
}

///
/// FI_SearchPatternInHeader
//  Searches string in header field(s)
static BOOL FI_SearchPatternInHeader(struct Search *search, struct Mail *mail)
{
  char fullfile[SIZE_PATHFILE];
  char mailfile[SIZE_PATHFILE];
  BOOL found = FALSE;

  ENTER();

  GetMailFile(mailfile, sizeof(mailfile), NULL, mail);

  if(StartUnpack(mailfile, fullfile, mail->Folder) != NULL)
  {
    FILE *fh;

    if((fh = fopen(fullfile, "r")) != NULL)
    {
      struct MinList *headerList;

      if((headerList = AllocSysObjectTags(ASOT_LIST, ASOLIST_Min, TRUE,
                                                     TAG_DONE)) != NULL)
      {
        setvbuf(fh, NULL, _IOFBF, SIZE_FILEBUF);

        if(MA_ReadHeader(mailfile, fh, headerList, RHM_MAINHEADER) == TRUE)
        {
          struct Node *curNode;

          IterateList(headerList, curNode)
          {
            struct HeaderNode *hdrNode = (struct HeaderNode *)curNode;

            // if the field is explicitly specified we search for it or
            // otherwise skip our search
            if(search->Field[0] != '\0')
            {
              int searchLen;
              char *ptr;

              // if the field is specified we search if it was specified with a ':'
              // at the end
              if((ptr = strchr(search->Field, ':')) != NULL)
                searchLen = ptr-(search->Field);
              else
                searchLen = strlen(search->Field);

              if(strnicmp(hdrNode->name, search->Field, searchLen) != 0)
                continue;
            }

            if(search->Compare == 4)
              found = FI_MatchListPattern(search, hdrNode->content);
            else
              found = FI_MatchString(search, hdrNode->content);
          }
        }

        // free our temporary headerList
        ClearHeaderList(headerList);
        FreeSysObject(ASOT_LIST, headerList);
      }

      // close the file
      fclose(fh);
    }

    FinishUnpack(fullfile);
  }

  RETURN(found);
  return found;
}

///
/// FI_IsFastSearch
//  Checks if quick search is available for selected header field
static enum FastSearch FI_IsFastSearch(const char *field)
{
  if (!stricmp(field, "from"))     return FS_FROM;
  if (!stricmp(field, "to"))       return FS_TO;
  if (!stricmp(field, "cc"))       return FS_CC;
  if (!stricmp(field, "reply-to")) return FS_REPLYTO;
  if (!stricmp(field, "subject"))  return FS_SUBJECT;
  if (!stricmp(field, "date"))     return FS_DATE;
  return FS_NONE;
}

///
/// FI_GenerateListPatterns
//  Reads list of patterns from a file
static void FI_GenerateListPatterns(struct Search *search)
{
  FILE *fh;

  ENTER();

  if((fh = fopen(search->Match, "r")) != NULL)
  {
    char *buf = NULL;
    size_t size = 0;

    setvbuf(fh, NULL, _IOFBF, SIZE_FILEBUF);

    // make sure the pattern list is successfully freed
    FreeSearchPatternList(search);

    while(GetLine(&buf, &size, fh) >= 0)
    {
      if(buf[0] != '\0')
      {
        struct SearchPatternNode *newNode;

        // put the pattern in our search pattern list
        if((newNode = AllocSysObjectTags(ASOT_NODE, ASONODE_Size, sizeof(*newNode),
                                                    ASONODE_Min, TRUE,
                                                    TAG_DONE)) != NULL)
        {
          if(search->CaseSens == TRUE)
            ParsePattern(buf, newNode->pattern, sizeof(newNode->pattern));
          else
            ParsePatternNoCase(buf, newNode->pattern, sizeof(newNode->pattern));

          // add the pattern to our list
          AddTail((struct List *)&search->patternList, (struct Node *)newNode);
        }
      }
    }

    free(buf);

    fclose(fh);
  }

  LEAVE();
}

///
/// FI_PrepareSearch
//  Initializes Search structure
BOOL FI_PrepareSearch(struct Search *search, enum SearchMode mode,
                      BOOL casesens, int persmode, int compar,
                      char stat, BOOL substr, BOOL dosPattern, const char *match, const char *field)
{
  BOOL success = TRUE;

  ENTER();

  memset(search, 0, sizeof(struct Search));
  search->Mode       = mode;
  search->CaseSens   = casesens;
  search->PersMode   = persmode;
  search->Compare    = compar;
  search->Status     = stat;
  search->SubString  = substr;
  search->DOSPattern = dosPattern;
  strlcpy(search->Match, match, sizeof(search->Match));
  strlcpy(search->Field, field, sizeof(search->Field));
  search->Pattern = search->PatBuf;
  search->Fast = FS_NONE;
  NewMinList(&search->patternList);

  switch(mode)
  {
    case SM_FROM:
      search->Fast = FS_FROM;
    break;

    case SM_TO:
      search->Fast = FS_TO;
    break;

    case SM_CC:
      search->Fast = FS_CC;
    break;

    case SM_REPLYTO:
      search->Fast = FS_REPLYTO;
    break;

    case SM_SUBJECT:
      search->Fast = FS_SUBJECT;
    break;

    case SM_DATE:
    {
      char *time;

      search->Fast = FS_DATE;
      search->DT.dat_Format = FORMAT_DEF;
      search->DT.dat_StrDate = (STRPTR)match;
      search->DT.dat_StrTime = (STRPTR)((time = strchr(match,' ')) ? time+1 : "00:00:00");

      if(StrToDate(&(search->DT)))
      {
        search->Pattern = (char *)&(search->DT.dat_Stamp);
      }
      else
      {
        char datstr[64];

        DateStamp2String(datstr, sizeof(datstr), NULL, DSS_DATE, TZC_NONE);
        ER_NewError(tr(MSG_ER_ErrorDateFormat), datstr);

        success = FALSE;
      }
    }
    break;

    case SM_HEADLINE:
      search->Fast = FI_IsFastSearch(field);
    break;

    case SM_SIZE:
    {
      search->Fast = FS_SIZE;
      search->Size = atol(match);
    }
    break;

    case SM_HEADER:   // continue
    case SM_WHOLE:
      search->Field[0] = '\0';
    break;

    default:
      // nothing
    break;
  }

  if(success)
  {
    if(compar == 4)
      FI_GenerateListPatterns(search);
    else if (search->Fast != FS_DATE && search->Fast != FS_SIZE && mode != SM_SIZE)
    {
      if(dosPattern == TRUE)
      {
        // we are told to perform AmigaDOS pattern matching
        if(substr == TRUE || mode == SM_HEADER || mode == SM_BODY || mode == SM_WHOLE || mode == SM_STATUS)
        {
          char buffer[SIZE_PATTERN+1];

          // if substring is selected lets generate a substring out
          // of the current match string, but keep the string borders in mind.
          strlcpy(buffer, search->Match, sizeof(buffer));
          snprintf(search->Match, sizeof(search->Match), "#?%s#?", buffer);
        }

        if(casesens == TRUE)
          ParsePattern(search->Match, search->Pattern, (SIZE_PATTERN+4)*2+2);
        else
          ParsePatternNoCase(search->Match, search->Pattern, (SIZE_PATTERN+4)*2+2);
      }
      else
      {
        // a simple substring search using the Boyer/Moore algorithm
        if((search->bmContext = BoyerMooreInit(search->Match, casesens)) == NULL)
          success = FALSE;
      }
    }
  }

  RETURN(success);
  return success;
}

///
/// FI_DoSearch
//  Checks if a message fulfills the search criteria
BOOL FI_DoSearch(struct Search *search, struct Mail *mail)
{
  BOOL found = FALSE;

  ENTER();

  D(DBF_FILTER, "doing filter search in message '%s'...", mail->Subject);

  switch(search->Mode)
  {
    case SM_FROM:
    case SM_TO:
    case SM_CC:
    case SM_REPLYTO:
    case SM_SUBJECT:
    case SM_DATE:
    case SM_HEADLINE:
    case SM_SIZE:
    {
      // check whether this is a fast search or not.
      found = (search->Fast == FS_NONE) ? FI_SearchPatternInHeader(search, mail) : FI_SearchPatternFast(search, mail);

      if(found)
        D(DBF_FILTER, "  search mode %ld matched", search->Mode);
      else
        D(DBF_FILTER, "  search mode %ld NOT matched", search->Mode);
    }
    break;

    case SM_HEADER:
    {
      BOOL found0;
      int comp_bak = search->Compare;

      search->Compare = 0;
      found0 = FI_SearchPatternInHeader(search, mail);
      search->Compare = comp_bak;

      if(found0 == (search->Compare == 0))
      {
        D(DBF_FILTER, "  HEADER: search for '%s' matched", search->Pattern);
        found = TRUE;
      }
      else
        D(DBF_FILTER, "  HEADER: search for '%s' NOT matched", search->Pattern);
    }
    break;

    case SM_BODY:
    {
      BOOL found0;
      int comp_bak = search->Compare;

      search->Compare = 0;
      found0 = FI_SearchPatternInBody(search, mail);
      search->Compare = comp_bak;

      if(found0 == (search->Compare == 0))
      {
        D(DBF_FILTER, "  BODY: search for '%s' matched", search->Pattern);
        found = TRUE;
      }
      else
        D(DBF_FILTER, "  BODY: search for '%s' NOT matched", search->Pattern);
    }
    break;

    case SM_WHOLE:
    {
      BOOL found0;
      int comp_bak = search->Compare;

      search->Compare = 0;

      if(!(found0 = FI_SearchPatternInHeader(search, mail)))
        found0 = FI_SearchPatternInBody(search, mail);

      search->Compare = comp_bak;

      if(found0 == (search->Compare == 0))
      {
        D(DBF_FILTER, "  WHOLE: search for '%s' matched", search->Pattern);
        found = TRUE;
      }
      else
        D(DBF_FILTER, "  WHOLE: search for '%s' NOT matched", search->Pattern);
    }
    break;

    case SM_STATUS:
    {
      BOOL statusFound = FALSE;

      switch(search->Status)
      {
        case 'U':
          statusFound = (hasStatusNew(mail) || !hasStatusRead(mail));
        break;

        case 'O':
          statusFound = (!hasStatusNew(mail) && hasStatusRead(mail));
        break;

        case 'F':
          statusFound = hasStatusForwarded(mail);
        break;

        case 'R':
          statusFound = hasStatusReplied(mail);
        break;

        case 'W':
          statusFound = hasStatusQueued(mail);
        break;

        case 'E':
          statusFound = hasStatusError(mail);
        break;

        case 'H':
          statusFound = hasStatusHold(mail);
        break;

        case 'S':
          statusFound = hasStatusSent(mail);
        break;

        case 'M':
          statusFound = hasStatusMarked(mail);
        break;

        case 'X':
          statusFound = hasStatusSpam(mail);
        break;
      }

      if((search->Compare == 0 && statusFound == TRUE) ||
         (search->Compare == 1 && statusFound == FALSE))
      {
        D(DBF_FILTER, "  status search %ld matched", search->Status);
        found = TRUE;
      }
      else
        D(DBF_FILTER, "  status search %ld NOT matched", search->Status);
    }
    break;

    case SM_SPAM:
    {
      if(C->SpamFilterEnabled && BayesFilterClassifyMessage(mail))
      {
        D(DBF_FILTER, "  identified as SPAM");
        found = TRUE;
      }
      else
        D(DBF_FILTER, "  identified as HAM");
    }
    break;
  }

  RETURN(found);
  return found;
}

///
/// DoFilterSearch()
//  Does a complex search with combined criterias based on the rules of a filter
BOOL DoFilterSearch(struct FilterNode *filter, struct Mail *mail)
{
  BOOL lastCond = FALSE;
  int i = 0;
  struct Node *curNode;

  ENTER();

  D(DBF_FILTER, "checking rules of filter '%s' for mail '%s'...", filter->name, mail->Subject);

  // we have to iterate through our ruleList and depending on the combine
  // operation we evaluate if the filter hits any mail criteria or not.
  IterateList(&filter->ruleList, curNode)
  {
    struct RuleNode *rule = (struct RuleNode *)curNode;

    if(rule->search != NULL)
    {
      BOOL actCond = FI_DoSearch(rule->search, mail);

      if(i == 0)
        rule->combine = CB_NONE;

      // if this isn't the first rule we do a compare
      switch(rule->combine)
      {
        case CB_OR:
        {
          lastCond = (actCond || lastCond);
        }
        break;

        case CB_AND:
        {
          lastCond = (actCond && lastCond);
        }
        break;

        case CB_XOR:
        {
          lastCond = (actCond+lastCond) % 2;
        }
        break;

        case CB_NONE:
        {
          lastCond = actCond;
        }
        break;
      }
    }

    i++;
  }

  RETURN(lastCond);
  return lastCond;
}

///
/// FI_SearchFunc
//  Starts the search and shows progress
HOOKPROTONHNONP(FI_SearchFunc, void)
{
  int fndmsg = 0;
  int totmsg = 0;
  int progress = 0;
  struct FI_GUIData *gui = &G->FI->GUI;
  struct FolderList *flist;

  ENTER();

  // by default we don't dispose on end
  G->FI->ClearOnEnd   = FALSE;
  G->FI->SearchActive = TRUE;
  G->FI->Abort        = FALSE;

  set(gui->WI, MUIA_Window_ActiveObject, MUIV_Window_ActiveObject_None);
  // disable some buttons while the search is going on
  DoMethod(G->App, MUIM_MultiSet, MUIA_Disabled, TRUE, gui->BT_SEARCH,
                                                       gui->BT_SELECTACTIVE,
                                                       gui->BT_SELECT,
                                                       gui->BT_READ,
                                                       NULL);
  DoMethod(gui->LV_MAILS, MUIM_NList_Clear);
  // start with forbidden immediate display of found mails, this greatly speeds
  // up the search process if many mails match the search criteria.
  set(gui->LV_MAILS, MUIA_NList_Quiet, TRUE);

  if((flist = CreateFolderList()) != NULL)
  {
    int id;

    id = MUIV_List_NextSelected_Start;
    while(TRUE)
    {
      char *name;
      struct Folder *folder;

      DoMethod(gui->LV_FOLDERS, MUIM_List_NextSelected, &id);
      if(id == MUIV_List_NextSelected_End)
        break;

      DoMethod(gui->LV_FOLDERS, MUIM_List_GetEntry, id, &name);
      if((folder = FO_GetFolderByName(name, NULL)) != NULL)
      {
        if(MA_GetIndex(folder) == TRUE)
        {
          if(AddNewFolderNode(flist, folder) != NULL)
            totmsg += folder->Total;
        }
      }
    }

    if(totmsg != 0)
    {
      struct Search search;
      static char gauge[40];
      struct TimeVal last;
      Object *ga = gui->GA_PROGRESS;
      Object *lv = gui->LV_MAILS;
      struct FolderNode *fnode;

      // lets prepare the search
      DoMethod(gui->GR_SEARCH, MUIM_SearchControlGroup_PrepareSearch, &search);

      // set the gauge
      snprintf(gauge, sizeof(gauge), tr(MSG_FI_GAUGETEXT), totmsg);
      xset(ga, MUIA_Gauge_InfoText, gauge,
               MUIA_Gauge_Max,      totmsg,
               MUIA_Gauge_Current,  0);

      set(gui->GR_PAGE, MUIA_Group_ActivePage, 1);

      memset(&last, 0, sizeof(struct TimeVal));

      ForEachFolderNode(flist, fnode)
      {
        struct Folder *folder = fnode->folder;

        LockMailListShared(folder->messages);

        if(IsMailListEmpty(folder->messages) == FALSE)
        {
          struct MailNode *mnode;

          ForEachMailNode(folder->messages, mnode)
          {
            struct Mail *mail = mnode->mail;

            if(FI_DoSearch(&search, mail) == TRUE)
            {
              DoMethod(lv, MUIM_NList_InsertSingle, mail, MUIV_NList_Insert_Sorted);
              fndmsg++;
            }

            // bail out if the search was aborted
            if(G->FI->Abort != FALSE)
              break;

            // increase the progress counter
            progress++;

            // then we update the gauge, but we take also care of not refreshing
            // it too often or otherwise it slows down the whole search process.
            if(TimeHasElapsed(&last, 250000) == TRUE)
            {
              // update the gauge
              set(ga, MUIA_Gauge_Current, progress);
              // let the list show the found mails so far
              set(lv, MUIA_NList_Quiet, FALSE);

              // signal the application to update now
              DoMethod(G->App, MUIM_Application_InputBuffered);

              // forbid immediate display again
              set(lv, MUIA_NList_Quiet, TRUE);
            }
          }
        }

        UnlockMailList(folder->messages);

        // bail out if the search was aborted
        if(G->FI->Abort != FALSE)
          break;
      }

      // to let the gauge move to 100% lets increase it accordingly.
      set(ga, MUIA_Gauge_Current, progress);

      // signal the application to update now
      DoMethod(G->App, MUIM_Application_InputBuffered);

      // free the temporary memory we allocated due to our
      // search operation
      FreeSearchPatternList(&search);
      FreeSearchData(&search);
    }

    DeleteFolderList(flist);
  }

  set(gui->LV_MAILS, MUIA_NList_Quiet, FALSE);
  set(gui->GR_PAGE, MUIA_Group_ActivePage, 0);
  // enable the buttons again
  set(gui->BT_SEARCH, MUIA_Disabled, FALSE);
  DoMethod(G->App, MUIM_MultiSet, MUIA_Disabled, fndmsg == 0, gui->BT_SELECTACTIVE,
                                                              gui->BT_SELECT,
                                                              gui->BT_READ,
                                                              NULL);

  G->FI->SearchActive = FALSE;

  // if the closeHook has set the ClearOnEnd flag we have to clear
  // the result listview
  if(G->FI->ClearOnEnd == TRUE)
    DoMethod(G->FI->GUI.LV_MAILS, MUIM_NList_Clear);
  else
    set(gui->WI, MUIA_Window_ActiveObject, xget(gui->GR_SEARCH, MUIA_SearchControlGroup_ActiveObject));

  LEAVE();
}
MakeStaticHook(FI_SearchHook,FI_SearchFunc);

///
/// CreateFilterFromSearch
//  Creates a filter from the current search options
HOOKPROTONHNONP(CreateFilterFromSearch, void)
{
  int ch;
  char name[SIZE_NAME];

  ENTER();

  name[0] = '\0';

  // request a name for that new filter from the user
  if((ch = StringRequest(name, SIZE_NAME,
                               tr(MSG_FI_AddFilter),
                               tr(MSG_FI_AddFilterReq),
                               tr(MSG_Save),
                               tr(MSG_Use),
                               tr(MSG_Cancel), FALSE,
                               G->FI->GUI.WI)))
  {
    struct FilterNode *filter;

    if((filter = CreateNewFilter()) != NULL)
    {
      struct RuleNode *rule;

      strlcpy(filter->name, name, sizeof(filter->name));

      if((rule = GetFilterRule(filter, 0)) != NULL)
        DoMethod(G->FI->GUI.GR_SEARCH, MUIM_SearchControlGroup_SetToRule, rule);

      // Now add the new filter to our list
      AddTail((struct List *)&C->filterList, (struct Node *)filter);

      // check if we should immediatly save our configuration or not
      if(ch == 1)
        CO_SaveConfig(C, G->CO_PrefsFile);
    }
  }
}
MakeStaticHook(CreateFilterFromSearchHook, CreateFilterFromSearch);

///
/// FI_Open
//  Opens find window
HOOKPROTONHNONP(FI_Open, void)
{
  ENTER();

  if(G->FI == NULL)
  {
    if((G->FI = FI_New()) == NULL)
      DisposeModulePush(&G->FI);
  }
  else if(G->FI->GUI.WI != NULL)
  {
    // clear the folder list
    DoMethod(G->FI->GUI.LV_FOLDERS, MUIM_List_Clear);
  }
  else
    DisposeModulePush(&G->FI);

  if(G->FI != NULL && G->FI->GUI.WI != NULL)
  {
    BOOL success = FALSE;
    struct Folder *folder;

    if((folder = FO_GetCurrentFolder()) != NULL)
    {
      int apos = 0;

      LockFolderListShared(G->folders);

      if(IsFolderListEmpty(G->folders) == FALSE)
      {
        struct FolderNode *fnode;
        int j = 0;

        ForEachFolderNode(G->folders, fnode)
        {
          if(isGroupFolder(fnode->folder) == FALSE)
          {
            DoMethod(G->FI->GUI.LV_FOLDERS, MUIM_List_InsertSingle, fnode->folder->Name, MUIV_List_Insert_Bottom);

            if(fnode->folder == folder)
              apos = j;

            j++;
          }
        }
      }

      UnlockFolderList(G->folders);

      set(G->FI->GUI.LV_FOLDERS, MUIA_List_Active, apos);

      // everything went fine
      success = TRUE;
    }

    if(success == TRUE)
    {
      // check if the window is already open
      if(xget(G->FI->GUI.WI, MUIA_Window_Open) == TRUE)
      {
        // bring window to front
        DoMethod(G->FI->GUI.WI, MUIM_Window_ToFront);

        // make window active
        set(G->FI->GUI.WI, MUIA_Window_Activate, TRUE);
      }
      else if(SafeOpenWindow(G->FI->GUI.WI) == FALSE)
        DisposeModulePush(&G->FI);

      // set object of last search session active as well
      set(G->FI->GUI.WI, MUIA_Window_ActiveObject, xget(G->FI->GUI.GR_SEARCH, MUIA_SearchControlGroup_ActiveObject));
    }
    else
      DisposeModulePush(&G->FI);
  }

  LEAVE();
}
MakeHook(FI_OpenHook,FI_Open);

///
/// FI_SwitchFunc
//  Sets active folder according to the selected message in the results window
HOOKPROTONHNONP(FI_SwitchFunc, void)
{
  struct Mail *foundmail;

  ENTER();

  // get the mail from the find list
  DoMethod(G->FI->GUI.LV_MAILS, MUIM_NList_GetEntry, MUIV_NList_GetEntry_Active, &foundmail);

  if(foundmail)
  {
    LONG pos = MUIV_NList_GetPos_Start;

    // make sure the folder of the found mail is set active
    MA_ChangeFolder(foundmail->Folder, TRUE);

    // now get the position of the foundmail in the real mail list and set it active
    DoMethod(G->MA->GUI.PG_MAILLIST, MUIM_NList_GetPos, foundmail, &pos);

    // if we found the mail in the currently active listview
    // we make it the active one as well.
    if(pos != MUIV_NList_GetPos_End)
    {
      // Because the NList for the maillistview and the NList for the find listview
      // are sharing the same displayhook we have to call MUIA_NList_DisplayRecall
      // twice here so that it will recognize that something has changed or
      // otherwise both NLists will show glitches.
      xset(G->MA->GUI.PG_MAILLIST, MUIA_NList_DisplayRecall, TRUE,
                                   MUIA_NList_Active, pos);

      set(G->FI->GUI.LV_MAILS, MUIA_NList_DisplayRecall, TRUE);
    }
  }

  LEAVE();
}
MakeStaticHook(FI_SwitchHook, FI_SwitchFunc);

///
/// FI_ReadFunc
//  Reads a message listed in the results window
HOOKPROTONHNONP(FI_ReadFunc, void)
{
  struct Mail *mail;

  ENTER();

  DoMethod(G->FI->GUI.LV_MAILS, MUIM_NList_GetEntry, MUIV_NList_GetEntry_Active, &mail);

  if(mail != NULL)
  {
    struct ReadMailData *rmData;

    if((rmData = CreateReadWindow(FALSE)) != NULL)
    {
      // make sure it is opened correctly and then read in a mail
      if(SafeOpenWindow(rmData->readWindow) == FALSE ||
         DoMethod(rmData->readWindow, MUIM_ReadWindow_ReadMail, mail) == FALSE)
      {
        // on any error we make sure to delete the read window
        // immediatly again.
        CleanupReadMailData(rmData, TRUE);
      }
    }
  }

  LEAVE();
}
MakeStaticHook(FI_ReadHook, FI_ReadFunc);

///
/// FI_SelectFunc
//  Selects matching messages in the main message list
HOOKPROTONHNONP(FI_SelectFunc, void)
{
  struct Folder *folder;

  ENTER();

  if((folder = FO_GetCurrentFolder()) != NULL)
  {
    int i;

    // unselect the currently selected mails first
    set(G->MA->GUI.PG_MAILLIST, MUIA_NList_Active, MUIV_NList_Active_Off);
    DoMethod(G->MA->GUI.PG_MAILLIST, MUIM_NList_Select, MUIV_NList_Select_All, MUIV_NList_Select_Off, NULL);

    for(i=0; ;i++)
    {
      struct Mail *foundmail;

      DoMethod(G->FI->GUI.LV_MAILS, MUIM_NList_GetEntry, i, &foundmail);
      if(foundmail == NULL)
        break;

      // only if the current folder is the same as this messages resists in
      if(foundmail->Folder == folder)
      {
        LONG pos = MUIV_NList_GetPos_Start;

        // get the position of the foundmail in the currently active
        // listview
        DoMethod(G->MA->GUI.PG_MAILLIST, MUIM_NList_GetPos, foundmail, &pos);

        // if we found the one in our listview we select it
        if(pos != MUIV_NList_GetPos_End)
          DoMethod(G->MA->GUI.PG_MAILLIST, MUIM_NList_Select, pos, MUIV_NList_Select_On, NULL);
      }
    }

    MA_ChangeSelected(TRUE);
  }

  LEAVE();
}
MakeStaticHook(FI_SelectHook, FI_SelectFunc);

///
/// FI_Close
//  Closes find window
HOOKPROTONHNONP(FI_Close, void)
{
  ENTER();

  // set the abort flag so that a running search will be stopped.
  if(G->FI->SearchActive)
  {
    G->FI->Abort = TRUE;
    G->FI->ClearOnEnd = TRUE;
  }
  else
  {
    // cleanup the result listview for the next session
    DoMethod(G->FI->GUI.LV_MAILS, MUIM_NList_Clear);
  }

  // close the window. all the other stuff will be
  // disposed by the application as soon as it
  // disposes itself..
  set(G->FI->GUI.WI, MUIA_Window_Open, FALSE);

  LEAVE();
}
MakeStaticHook(FI_CloseHook, FI_Close);
///
/// FI_FilterSingleMail
//  applies the configured filters on a single mail
BOOL FI_FilterSingleMail(struct Mail *mail, int *matches)
{
  BOOL success = TRUE;
  struct Node *curNode;
  int match = 0;

  ENTER();

  IterateList(&C->filterList, curNode)
  {
    struct FilterNode *filter = (struct FilterNode *)curNode;

    if(DoFilterSearch(filter, mail) == TRUE)
    {
      match++;

      // if ExecuteFilterAction returns FALSE then the filter search should be aborted
      // completley
      if(ExecuteFilterAction(filter, mail) == FALSE)
      {
        success = FALSE;
        break;
      }
    }
  }

  if(matches != NULL)
    *matches += match;

  RETURN(success);
  return success;
}

///
/// FreeSearchPatternList
// Function to make the whole pattern list is correctly cleaned up
static void FreeSearchPatternList(struct Search *search)
{
  struct Node *curNode;

  ENTER();

  // free all search pattern nodes
  while((curNode = RemHead((struct List *)&search->patternList)) != NULL)
  {
    struct SearchPatternNode *patternNode = (struct SearchPatternNode *)curNode;

    FreeSysObject(ASOT_NODE, patternNode);
  }

  LEAVE();
}

///
/// FreeSearchData
// Function to free the search data
void FreeSearchData(struct Search *search)
{
  ENTER();

  // free a possibly initalized Boyer-Moore search context
  if(search->bmContext != NULL)
  {
    BoyerMooreCleanup(search->bmContext);
    search->bmContext = NULL;
  }

  LEAVE();
}

///
/// FreeRuleSearchData
// Function to free the search data of a rule
void FreeRuleSearchData(struct RuleNode *rule)
{
  ENTER();

  if(rule->search != NULL)
  {
    FreeSearchPatternList(rule->search);
    FreeSearchData(rule->search);
    free(rule->search);
    rule->search = NULL;
  }

  LEAVE();
}

///
/// AllocFilterSearch
//  Allocates and initializes search structures for filters and returns
//  the number of active filters/search structures allocated.
int AllocFilterSearch(enum ApplyFilterMode mode)
{
  int active = 0;
  struct Node *curNode;

  ENTER();

  // iterate through our filter List
  IterateList(&C->filterList, curNode)
  {
    struct FilterNode *filter = (struct FilterNode *)curNode;

    // let's check if we can skip some filters because of the ApplyMode
    // and filter relation.
    // For spam recognition we just clear all filters and don't allocate
    // anything.
    if((mode == APPLY_AUTO && (!filter->applyToNew || filter->remote)) ||
       (mode == APPLY_USER && (!filter->applyOnReq || filter->remote)) ||
       (mode == APPLY_SENT && (!filter->applyToSent || filter->remote)) ||
       (mode == APPLY_REMOTE && !filter->remote) ||
       (mode == APPLY_SPAM))
    {
      struct Node *curRuleNode;

      IterateList(&filter->ruleList, curRuleNode)
      {
        // make sure the current search structures of the rules of the filter
        // are freed
        struct RuleNode *rule = (struct RuleNode *)curRuleNode;

        // now we do free our search structure if it exists
        FreeRuleSearchData(rule);
      }
    }
    else
    {
      struct Node *curRuleNode;

      IterateList(&filter->ruleList, curRuleNode)
      {
        // check if the search structures are already allocated or not
        struct RuleNode *rule = (struct RuleNode *)curRuleNode;

        // check if that search structure already exists or not
        if(rule->search == NULL &&
           (rule->search = calloc(1, sizeof(struct Search))))
        {
          int stat = sizeof(mailStatusCycleMap);

          // we check the status field first and if we find a match
          // we can immediatly break up here because we don't need to prepare the search
          if(rule->searchMode == SM_STATUS)
          {
            for(stat=0; stat <= (int)sizeof(mailStatusCycleMap) - 1; stat++)
            {
              if(*rule->matchPattern == mailStatusCycleMap[stat])
                break;
            }
          }

          FI_PrepareSearch(rule->search,
                           rule->searchMode,
                           rule->caseSensitive,
                           rule->subSearchMode,
                           rule->comparison,
                           mailStatusCycleMap[stat],
                           rule->subString,
                           rule->dosPattern,
                           rule->matchPattern,
                           rule->customField);

          // save a pointer to the filter in the search structure as well.
          rule->search->filter = filter;
        }

        active++;
      }
    }
  }

  RETURN(active);
  return active;
}

///
/// FreeFilterSearch
//  Frees filter search structures
void FreeFilterSearch(void)
{
  struct Node *curNode;

  ENTER();

  // Now we process the read header to set all flags accordingly
  IterateList(&C->filterList, curNode)
  {
    struct FilterNode *filter = (struct FilterNode *)curNode;
    struct Node *curRuleNode;

    IterateList(&filter->ruleList, curRuleNode)
    {
      struct RuleNode *rule = (struct RuleNode *)curRuleNode;

      // now we do free our search structure if it exists
      FreeRuleSearchData(rule);
    }
  }

  LEAVE();
}

///
/// ExecuteFilterAction
//  Applies filter action to a message and return TRUE if the filter search
//  should continue or FALSE if it should stop afterwards
BOOL ExecuteFilterAction(struct FilterNode *filter, struct Mail *mail)
{
  BOOL success = TRUE;
  struct MailList *mlist;

  ENTER();

  // initialize mlist
  if((mlist = CreateMailList()) != NULL)
  {
    AddNewMailNode(mlist, mail);

    // Bounce Action
    if(hasBounceAction(filter) && !filter->remote && *filter->bounceTo)
    {
      struct WriteMailData *wmData;

      if((wmData = NewBounceMailWindow(mail, NEWF_QUIET)) != NULL)
      {
        set(wmData->window, MUIA_WriteWindow_To, filter->bounceTo);
        DoMethod(wmData->window, MUIM_WriteWindow_ComposeMail, WRITE_QUEUE);

        G->RuleResults.Bounced++;
      }
    }

    // Forward Action
    if(hasForwardAction(filter) && !filter->remote && *filter->forwardTo)
    {
      struct WriteMailData *wmData;

      if((wmData = NewForwardMailWindow(mlist, NEWF_QUIET)) != NULL)
      {
        set(wmData->window, MUIA_WriteWindow_To, filter->forwardTo);
        DoMethod(wmData->window, MUIM_WriteWindow_ComposeMail, WRITE_QUEUE);

        G->RuleResults.Forwarded++;
      }
    }

    // Reply Action
    if(hasReplyAction(filter) && !filter->remote && *filter->replyFile)
    {
      struct WriteMailData *wmData;

      if((wmData = NewReplyMailWindow(mlist, NEWF_QUIET, NULL)) != NULL)
      {
        DoMethod(wmData->window, MUIM_WriteWindow_LoadText, filter->replyFile, TRUE);
        DoMethod(wmData->window, MUIM_WriteWindow_ComposeMail, WRITE_QUEUE);

        G->RuleResults.Replied++;
      }
    }

    // Execute Action
    if(hasExecuteAction(filter) && *filter->executeCmd)
    {
      char mailfile[SIZE_PATHFILE];
      char buf[SIZE_COMMAND + SIZE_PATHFILE];

      GetMailFile(mailfile, sizeof(mailfile), NULL, mail);
      snprintf(buf, sizeof(buf), "%s \"%s\"", filter->executeCmd, mailfile);
      LaunchCommand(buf, FALSE, OUT_STDOUT);
      G->RuleResults.Executed++;
    }

    // PlaySound Action
    if(hasPlaySoundAction(filter) && *filter->playSound)
      DoAction(TA_PlaySound, TT_PlaySound_Filename, filter->playSound, TAG_DONE);

    // Move Action
    if(hasMoveAction(filter) && !filter->remote)
    {
      struct Folder* fo;

      if((fo = FO_GetFolderByName(filter->moveTo, NULL)) != NULL)
      {
        if(mail->Folder != fo)
        {
          BOOL accessFreed = FALSE;
          enum LoadedMode oldLoadedMode = fo->LoadedMode;

          G->RuleResults.Moved++;

          // temporarily grant free access to the folder, but only if it has no free access yet
          if(fo->LoadedMode != LM_VALID && isProtectedFolder(fo) && isFreeAccess(fo) == FALSE)
          {
            SET_FLAG(fo->Flags, FOFL_FREEXS);
            accessFreed = TRUE;
          }

          MA_MoveCopy(mail, mail->Folder, fo, FALSE, TRUE);

          // restore the old access mode if it was changed before
          if(accessFreed)
          {
            // restore old index settings
            // if it was not yet loaded before, the MA_MoveCopy() call changed this to "loaded"
            fo->LoadedMode = oldLoadedMode;
            CLEAR_FLAG(fo->Flags, FOFL_FREEXS);
          }

          // signal failure, although everything was successful yet
          // but the mail is not available anymore for other filters
          success = FALSE;
        }
      }
      else
        ER_NewError(tr(MSG_ER_CANTMOVEMAIL), mail->MailFile, filter->moveTo);
    }

    // Delete Action
    if(hasDeleteAction(filter) && success == TRUE)
    {
      G->RuleResults.Deleted++;

      if(isSendMDNMail(mail) &&
         (hasStatusNew(mail) || !hasStatusRead(mail)))
      {
        RE_ProcessMDN(MDN_MODE_DELETE, mail, FALSE, TRUE);
      }

      MA_DeleteSingle(mail, DELF_CLOSE_WINDOWS|DELF_UPDATE_APPICON);

      // signal failure, although everything was successful yet
      // but the mail is not available anymore for other filters
      success = FALSE;
    }

    DeleteMailList(mlist);
  }

  RETURN(success);
  return success;
}

///
/// FilterMails
// Apply filters
void FilterMails(struct Folder *folder, struct MailList *mlist, int mode)
{
  struct Folder *spamfolder = FO_GetFolderByType(FT_SPAM, NULL);
  struct MailNode *mnode;
  ULONG m;
  int scnt;
  int matches = 0;

  ENTER();

  memset(&G->RuleResults, 0, sizeof(G->RuleResults));
  set(G->MA->GUI.PG_MAILLIST, MUIA_NList_Quiet, TRUE);
  G->AppIconQuiet = TRUE;

  // for simple spam classification this will result in zero
  scnt = AllocFilterSearch(mode);

  // we use another Busy Gauge information if this is
  // a spam classification session. And we build an interruptable
  // Gauge which will report back if the user pressed the stop button
  if(mode != APPLY_SPAM)
    BusyGaugeInt(tr(MSG_BusyFiltering), "", mlist->count);
  else
    BusyGaugeInt(tr(MSG_FI_BUSYCHECKSPAM), "", mlist->count);

    MA_StartMacro(MACRO_PREFILTER, NULL);

  m = 0;
  ForEachMailNode(mlist, mnode)
  {
    struct Mail *mail = mnode->mail;
    BOOL wasSpam = FALSE;

    if(mail != NULL)
    {
      if(C->SpamFilterEnabled == TRUE && (mode == APPLY_AUTO || mode == APPLY_SPAM))
      {
        BOOL doClassification;

        D(DBF_FILTER, "About to apply SPAM filter to message with subject \"%s\"", mail->Subject);

        if(mode == APPLY_AUTO && C->SpamFilterForNewMail == TRUE)
        {
          // classify this mail if we are allowed to check new mails automatically
          doClassification = TRUE;
        }
        else if(mode == APPLY_SPAM && hasStatusSpam(mail) == FALSE && hasStatusHam(mail) == FALSE)
        {
          // classify mails if the user triggered this and the mail is not yet classified
          doClassification = TRUE;
        }
        else
        {
          // don't try to classify this mail
          doClassification = FALSE;
        }

        if(doClassification == TRUE)
        {
          D(DBF_FILTER, "Classifying message with subject \"%s\"", mail->Subject);

          if(BayesFilterClassifyMessage(mail) == TRUE)
          {
            D(DBF_FILTER, "Message was classified as spam");

            // set the SPAM flags, but clear the NEW and READ flags only if desired
            if(C->SpamMarkAsRead == TRUE)
              setStatusToReadAutoSpam(mail);
            else
              setStatusToAutoSpam(mail);

            // move newly recognized spam to the spam folder
            MA_MoveCopy(mail, folder, spamfolder, FALSE, FALSE);
            wasSpam = TRUE;

            // update the stats
            G->RuleResults.Spam++;
            // we just checked the mail
            G->RuleResults.Checked++;
          }
        }
      }

      if(scnt > 0 && wasSpam == FALSE)
      {
        // apply all other user defined filters (if they exist) for non-spam mails
        // or if the spam filter is disabled
        G->RuleResults.Checked++;

        // now we process the search
        FI_FilterSingleMail(mail, &matches);
      }

      // we update the busy gauge and
      // see if we have to exit/abort in case it returns FALSE
      if(BusySet(++m) == FALSE)
        break;
    }
  }

  FreeFilterSearch();

  if(G->RuleResults.Checked != 0)
    AppendToLogfile(LF_ALL, 26, tr(MSG_LOG_Filtering), G->RuleResults.Checked, folder->Name, matches);

    MA_StartMacro(MACRO_POSTFILTER, NULL);

  BusyEnd();

  set(G->MA->GUI.PG_MAILLIST, MUIA_NList_Quiet, FALSE);
  G->AppIconQuiet = FALSE;

  if(mode != APPLY_AUTO)
    DisplayStatistics(NULL, TRUE);

  LEAVE();
}

///
/// ApplyFiltersFunc
//  Apply filters
HOOKPROTONHNO(ApplyFiltersFunc, void, int *arg)
{
  struct Folder *folder;
  enum ApplyFilterMode mode = arg[0];

  ENTER();

  D(DBF_FILTER, "About to apply SPAM and user defined filters in mode %ld...", mode);

  if((folder = (mode == APPLY_AUTO) ? FO_GetFolderByType(FT_INCOMING, NULL) : FO_GetCurrentFolder()) != NULL &&
     (C->SpamFilterEnabled == FALSE || FO_GetFolderByType(FT_SPAM, NULL) != NULL))
  {
    struct MailList *mlist = NULL;
    BOOL processAllMails = TRUE;

    // query how many mails are currently selected/marked
    if(mode == APPLY_USER || mode == APPLY_RX || mode == APPLY_RX_ALL || mode == APPLY_SPAM)
    {
      ULONG minselected = hasFlag(arg[1], (IEQUALIFIER_LSHIFT|IEQUALIFIER_RSHIFT)) ? 1 : 2;

      if((mlist = MA_CreateMarkedList(G->MA->GUI.PG_MAILLIST, mode == APPLY_RX)) != NULL)
      {
        if(mlist->count < minselected)
        {
          W(DBF_FILTER, "number of selected mails < required minimum (%ld < %ld)", mlist->count, minselected);

          DeleteMailList(mlist);
          mlist = NULL;
        }
        else
          processAllMails = FALSE;
      }
    }

    // if we haven't got any mail list, we
    // go and create one over all mails in the folder
    if(mlist == NULL)
      mlist = MA_CreateFullList(folder, (mode == APPLY_AUTO || mode == APPLY_RX));

    // check that we can continue
    if(mlist != NULL)
    {
      BOOL applyFilters = TRUE;

      // if this function was called manually by the user we ask him
      // if he really wants to apply the filters or not.
      if(mode == APPLY_USER)
      {
        char buf[SIZE_LARGE];

        if(processAllMails == TRUE)
          snprintf(buf, sizeof(buf), tr(MSG_MA_CONFIRMFILTER_ALL), folder->Name);
        else
          snprintf(buf, sizeof(buf), tr(MSG_MA_CONFIRMFILTER_SELECTED), folder->Name);

        if(MUI_Request(G->App, G->MA->GUI.WI, 0, tr(MSG_MA_ConfirmReq), tr(MSG_YesNoReq), buf) == 0)
          applyFilters = FALSE;
      }

      // the user has not cancelled the filter process
      if(applyFilters == TRUE)
      {
        FilterMails(folder, mlist, mode);

        if(G->RuleResults.Checked != 0 && mode == APPLY_USER)
        {
          if(C->ShowFilterStats == TRUE)
          {
            char buf[SIZE_LARGE];

            if(C->SpamFilterEnabled == TRUE)
            {
              // include the number of spam classified mails
              snprintf(buf, sizeof(buf), tr(MSG_MA_FILTER_STATS_SPAM), G->RuleResults.Checked,
                                                                       G->RuleResults.Forwarded,
                                                                       G->RuleResults.Moved,
                                                                       G->RuleResults.Deleted,
                                                                       G->RuleResults.Spam);
            }
            else
            {
              snprintf(buf, sizeof(buf), tr(MSG_MA_FilterStats), G->RuleResults.Checked,
                                                                 G->RuleResults.Forwarded,
                                                                 G->RuleResults.Moved,
                                                                 G->RuleResults.Deleted);
            }
            MUI_Request(G->App, G->MA->GUI.WI, 0, NULL, tr(MSG_OkayReq), buf);
          }
        }
      }
      else
        D(DBF_FILTER, "filtering rejected by user.");

      DeleteMailList(mlist);
    }
    else
      W(DBF_FILTER, "folder empty or error on creating list of mails.");
  }

  LEAVE();
}
MakeHook(ApplyFiltersHook, ApplyFiltersFunc);
///
/// CopyFilterData
// copy all data of a filter node (deep copy)
BOOL CopyFilterData(struct FilterNode *dstFilter, struct FilterNode *srcFilter)
{
  BOOL success = TRUE;
  struct Node *curNode;

  ENTER();

  SHOWSTRING(DBF_FILTER, srcFilter->name);

  // raw copy all global stuff first
  memcpy(dstFilter, srcFilter, sizeof(struct FilterNode));

  // then iterate through our ruleList and copy it as well
  NewMinList(&dstFilter->ruleList);

  IterateList(&srcFilter->ruleList, curNode)
  {
    struct RuleNode *rule = (struct RuleNode *)curNode;
    struct RuleNode *newRule;

    // do a raw copy of the rule contents first
    if((newRule = DuplicateNode(rule, sizeof(*rule))) != NULL)
    {
      // check if the search structure exists and if so
      // so start another deep copy
      if(rule->search != NULL)
      {
        if((newRule->search = calloc(1, sizeof(struct Search))) != NULL)
        {
          if(CopySearchData(newRule->search, rule->search) == FALSE)
          {
            success = FALSE;
            // bail out, no need to copy further data
            break;
          }
        }
        else
        {
          success = FALSE;
          // bail out, no need to copy further data
          break;
        }
      }
      else
        newRule->search = NULL;

      // add the rule to the ruleList of our desitionation filter
      AddTail((struct List *)&dstFilter->ruleList, (struct Node *)newRule);
    }
    else
    {
      success = FALSE;
      // bail out, no need to copy further data
      break;
    }
  }

  RETURN(success);
  return success;
}

///
/// CopySearchData
// copy all data of a search structure (deep copy)
static BOOL CopySearchData(struct Search *dstSearch, struct Search *srcSearch)
{
  BOOL success = TRUE;
  struct Node *curNode;

  ENTER();

  // raw copy all global stuff first
  memcpy(dstSearch, srcSearch, sizeof(struct Search));

  // then we check whether we have to copy another bunch of sub data or not
  if(srcSearch->Pattern == srcSearch->PatBuf)
    dstSearch->Pattern = dstSearch->PatBuf;
  else if(srcSearch->Pattern == (char *)&(srcSearch->DT.dat_Stamp))
    dstSearch->Pattern = (char *)&(dstSearch->DT.dat_Stamp);

  // now we have to copy the patternList as well
  NewMinList(&dstSearch->patternList);
  IterateList(&srcSearch->patternList, curNode)
  {
    struct SearchPatternNode *srcNode = (struct SearchPatternNode *)curNode;
    struct SearchPatternNode *dstNode;

    if((dstNode = DuplicateNode(srcNode, sizeof(*srcNode))) != NULL)
      AddTail((struct List *)&dstSearch->patternList, (struct Node *)dstNode);
    else
    {
      success = FALSE;
      // bail out, no need to copy further data
      break;
    }
  }

  // no need to handle srcSearch->bmContext as this will be allocation when
  // performing the actual search

  RETURN(success);
  return success;
}

///
/// FreeFilterRuleList
void FreeFilterRuleList(struct FilterNode *filter)
{
  struct Node *curNode;

  ENTER();

  // we do have to iterate through our ruleList and
  // free them as well
  while((curNode = RemHead((struct List *)&filter->ruleList)) != NULL)
  {
    struct RuleNode *rule = (struct RuleNode *)curNode;

    // now we do free our search structure if it exists
    FreeRuleSearchData(rule);

    FreeSysObject(ASOT_NODE, rule);
  }

  // initialize the ruleList as well
  NewMinList(&filter->ruleList);

  LEAVE();
}

///
/// CompareRuleLists
// compare two rule lists to be equal
static BOOL CompareRuleLists(const struct MinList *rl1, const struct MinList *rl2)
{
  BOOL equal = TRUE;
  BOOL empty1;
  BOOL empty2;

  ENTER();

  empty1 = IsMinListEmpty(rl1);
  empty2 = IsMinListEmpty(rl2);
  if(empty1 == FALSE && empty2 == FALSE)
  {
    struct Node *mln1 = GetHead((struct List *)rl1);
    struct Node *mln2 = GetHead((struct List *)rl2);

    while(mln1 != NULL && mln2 != NULL)
    {
      struct RuleNode *rn1 = (struct RuleNode *)mln1;
      struct RuleNode *rn2 = (struct RuleNode *)mln2;

      if(rn1->combine           != rn2->combine ||
         rn1->searchMode        != rn2->searchMode ||
         rn1->subSearchMode     != rn2->subSearchMode ||
         rn1->comparison        != rn2->comparison ||
         rn1->caseSensitive     != rn2->caseSensitive ||
         rn1->subString         != rn2->subString ||
         rn1->dosPattern        != rn2->dosPattern ||
         strcmp(rn1->matchPattern, rn2->matchPattern) != 0 ||
         strcmp(rn1->customField,  rn2->customField) != 0)
      {
        // something of this rule does not match
        equal = FALSE;
        break;
      }

      mln1 = GetSucc(mln1);
      mln2 = GetSucc(mln2);
    }

    // if there are any nodes left then the two lists cannot be equal
    if(mln1 != NULL || mln2 != NULL)
      equal = FALSE;
  }
  else if((empty1 == TRUE && empty2 == FALSE) || (empty1 == FALSE && empty2 == TRUE))
  {
    // if one list is empty while the other is not the two lists cannot be equal
    equal = FALSE;
  }

  RETURN(equal);
  return equal;
}

///
/// CompareFilterLists
// performs a deep compare of two filter lists and returns TRUE if they are
// equal
BOOL CompareFilterLists(const struct MinList *fl1, const struct MinList *fl2)
{
  BOOL equal = TRUE;
  BOOL empty1;
  BOOL empty2;

  ENTER();

  empty1 = IsMinListEmpty(fl1);
  empty2 = IsMinListEmpty(fl2);
  if(empty1 == FALSE && empty2 == FALSE)
  {
    struct Node *mln1 = GetHead((struct List *)fl1);
    struct Node *mln2 = GetHead((struct List *)fl2);

    // walk through both lists in parallel and compare the single nodes
    while(mln1 != NULL && mln2 != NULL)
    {
      struct FilterNode *fn1 = (struct FilterNode *)mln1;
      struct FilterNode *fn2 = (struct FilterNode *)mln2;

      // compare every single member of the structure
      if(fn1->actions         != fn2->actions ||
         fn1->remote          != fn2->remote ||
         fn1->applyToNew      != fn2->applyToNew ||
         fn1->applyOnReq      != fn2->applyOnReq ||
         fn1->applyToSent     != fn2->applyToSent ||
         strcmp(fn1->name,       fn2->name) != 0 ||
         strcmp(fn1->bounceTo,   fn2->bounceTo) != 0 ||
         strcmp(fn1->forwardTo,  fn2->forwardTo) != 0 ||
         strcmp(fn1->replyFile,  fn2->replyFile) != 0 ||
         strcmp(fn1->executeCmd, fn2->executeCmd) != 0 ||
         strcmp(fn1->playSound,  fn2->playSound) != 0 ||
         strcmp(fn1->moveTo,     fn2->moveTo) != 0 ||
         CompareRuleLists(&fn1->ruleList, &fn2->ruleList) == FALSE)
      {
        // something does not match
        equal = FALSE;
        break;
      }

      mln1 = GetSucc(mln1);
      mln2 = GetSucc(mln2);
    }

    // if there are any nodes left then the two lists cannot be equal
    if(mln1 != NULL || mln2 != NULL)
      equal = FALSE;
  }
  else if((empty1 == TRUE && empty2 == FALSE) || (empty1 == FALSE && empty2 == TRUE))
  {
    // if one list is empty while the other is not the two lists cannot be equal
    equal = FALSE;
  }

  RETURN(equal);
  return equal;
}

///
/// CreateNewFilter
//  Initializes a new filter
struct FilterNode *CreateNewFilter(void)
{
  struct FilterNode *filter;

  ENTER();

  if((filter = AllocSysObjectTags(ASOT_NODE, ASONODE_Size, sizeof(*filter),
                                             ASONODE_Min, TRUE,
                                             TAG_DONE)) != NULL)
  {
    filter->actions = 0;
    filter->remote = FALSE;
    filter->applyToNew = TRUE;
    filter->applyOnReq = TRUE;
    filter->applyToSent = FALSE;
    strlcpy(filter->name, tr(MSG_NewEntry), sizeof(filter->name));
    filter->bounceTo[0] = '\0';
    filter->forwardTo[0] = '\0';
    filter->replyFile[0] = '\0';
    filter->executeCmd[0] = '\0';
    filter->playSound[0] = '\0';
    filter->moveTo[0] = '\0';

    // initialize the rule list
    NewMinList(&filter->ruleList);

    // and fill in the first rule as a filter can't have less than 1 rule
    // anyway
    if(CreateNewRule(filter, FALSE) == NULL)
    {
      // creating the default rule failed, so we let this operation fail, too
      FreeSysObject(ASOT_NODE, filter);
      filter = NULL;
    }
  }

  RETURN(filter);
  return filter;
}

///
/// FreeFilterNode
// frees a complete filter with all embedded rules
void FreeFilterNode(struct FilterNode *filter)
{
  ENTER();

  // free this filter's rules
  FreeFilterRuleList(filter);

  // and finally free the filter itself
  FreeSysObject(ASOT_NODE, filter);

  LEAVE();
}

///
/// FreeFilterList
// frees a complete filter list with all embedded filters
void FreeFilterList(struct MinList *filterList)
{
  struct Node *curNode;

  ENTER();

  // we have to free the filterList
  while((curNode = RemHead((struct List *)filterList)) != NULL)
  {
    struct FilterNode *filter = (struct FilterNode *)curNode;

    FreeFilterNode(filter);
  }

  NewMinList(filterList);

  LEAVE();
}

///
/// CreateNewRule
//  Initializes a new filter rule
struct RuleNode *CreateNewRule(struct FilterNode *filter, const BOOL dosPattern)
{
  struct RuleNode *rule;

  ENTER();

  if((rule = AllocSysObjectTags(ASOT_NODE, ASONODE_Size, sizeof(*rule),
  	                                       ASONODE_Min, TRUE,
  	                                       TAG_DONE)) != NULL)
  {
    // set the default search mode (plain string search or DOS patterns)
    rule->search = NULL;
    rule->combine = CB_NONE;
    rule->searchMode = SM_FROM;
    rule->subSearchMode = SSM_ADDRESS;
    rule->comparison = CP_EQUAL;
    rule->caseSensitive = FALSE;
    rule->subString = FALSE;
    rule->dosPattern = dosPattern;
    rule->matchPattern[0] = '\0';
    rule->customField[0] = '\0';

    // if a filter was specified we immediatley add this new rule to it
    if(filter != NULL)
    {
      SHOWSTRING(DBF_FILTER, filter->name);
      AddTail((struct List *)&filter->ruleList, (struct Node *)rule);
    }
  }

  RETURN(rule);
  return rule;
}

///
/// GetFilterRule
//  return a pointer to the rule depending on the position in the ruleList
struct RuleNode *GetFilterRule(struct FilterNode *filter, int pos)
{
  struct RuleNode *rule = NULL;
  struct Node *curNode;
  int i = 0;

  ENTER();

  // we do have to iterate through the ruleList of the filter
  // and count for rule at position 'pos'
  IterateList(&filter->ruleList, curNode)
  {
    if(i == pos)
    {
      rule = (struct RuleNode *)curNode;
      break;
    }
    i++;
  }

  RETURN(rule);
  return rule;
}

///
/// FolderUsedByFilters
// check if the folder is used by any filter as "move to" folder
BOOL FolderIsUsedByFilters(const char *folder)
{
  struct Node *fNode;
  BOOL folderIsUsed = FALSE;

  ENTER();

  // iterate over all filters and replace any occurence of
  // the old folder name by the new one
  IterateList(&C->filterList, fNode)
  {
    struct FilterNode *filter = (struct FilterNode *)fNode;

    if(hasMoveAction(filter) == TRUE && stricmp(filter->moveTo, folder) == 0)
    {
      folderIsUsed = TRUE;
      break;
    }
  }

  RETURN(folderIsUsed);
  return folderIsUsed;
}

///
/// RenameFolderInFilters
// modify the destination folder of all filters
void RenameFolderInFilters(const char *oldFolder, const char *newFolder)
{
  struct Node *fNode;

  ENTER();

  // iterate over all filters and replace any occurence of
  // the old folder name by the new one
  IterateList(&C->filterList, fNode)
  {
    struct FilterNode *filter = (struct FilterNode *)fNode;

    if(hasMoveAction(filter) == TRUE && stricmp(filter->moveTo, oldFolder) == 0)
    {
      D(DBF_FILTER, "changing MoveTo folder of filer '%s' to '%s'", filter->name, newFolder);
      strlcpy(filter->moveTo, newFolder, sizeof(filter->moveTo));

      // remember the modified configuration, but don't save it yet
      C->ConfigIsSaved = FALSE;
    }
  }

  LEAVE();
}

///
/// RemoveFolderFromFilters
// remove a folder from a filter in case it is its "move to" folder
void RemoveFolderFromFilters(const char *folder)
{
  struct Node *fNode;

  ENTER();

  // iterate over all filters and replace any occurence of
  // the old folder name by the new one
  IterateList(&C->filterList, fNode)
  {
    struct FilterNode *filter = (struct FilterNode *)fNode;

    if(hasMoveAction(filter) == TRUE && stricmp(filter->moveTo, folder) == 0)
    {
      D(DBF_FILTER, "removing MoveTo folder '%s' of filer '%s'", folder, filter->name);
      filter->moveTo[0] = '\0';
      CLEAR_FLAG(filter->actions, FA_MOVE);

      // remember the modified configuration, but don't save it yet
      C->ConfigIsSaved = FALSE;
    }
  }

  LEAVE();
}

///

/*** GUI ***/
/// InitFilterPopupList
//  Creates a popup list of configured filters
HOOKPROTONHNP(InitFilterPopupList, ULONG, Object *pop)
{
  struct Node *curNode;

  // clear the popup list first
  DoMethod(pop, MUIM_List_Clear);

  // Now we process the read header to set all flags accordingly
  IterateList(&C->filterList, curNode)
  {
    DoMethod(pop, MUIM_List_InsertSingle, curNode, MUIV_List_Insert_Bottom);
  }

  return TRUE;
}
MakeStaticHook(InitFilterPopupListHook, InitFilterPopupList);

///
/// FilterPopupDisplayHook
HOOKPROTONH(FilterPopupDisplayFunc, ULONG, char **array, struct FilterNode *filter)
{
  array[0] = filter->name;
  return 0;
}
MakeStaticHook(FilterPopupDisplayHook, FilterPopupDisplayFunc);

///
/// SearchOptFromFilterPopup
//  Gets search options from selected filter
HOOKPROTONHNP(SearchOptFromFilterPopup, void, Object *pop)
{
  struct FilterNode *filter;

  // get the currently active filter
  DoMethod(pop, MUIM_List_GetEntry, MUIV_List_GetEntry_Active, &filter);

  if(filter != NULL)
  {
    struct RuleNode *rule;

    if((rule = GetFilterRule(filter, 0)) != NULL)
      DoMethod(G->FI->GUI.GR_SEARCH, MUIM_SearchControlGroup_GetFromRule, rule);
  }
}
MakeStaticHook(SearchOptFromFilterPopupHook, SearchOptFromFilterPopup);

///
/// FI_New
//  Creates find window
static struct FI_ClassData *FI_New(void)
{
  struct FI_ClassData *data;

  ENTER();

  if((data = calloc(1, sizeof(struct FI_ClassData))) != NULL)
  {
    Object *bt_abort;
    Object *lv_fromrule;
    Object *bt_torule;
    Object *po_fromrule;
    Object *bt_all;
    Object *bt_none;

    data->GUI.WI = WindowObject,
       MUIA_Window_Title, tr(MSG_FI_FindMessages),
       MUIA_HelpNode, "FI_W",
       MUIA_Window_ID, MAKE_ID('F','I','N','D'),
       WindowContents, VGroup,
          Child, HGroup,
             MUIA_VertWeight, 1,
             Child, VGroup, GroupFrameT(tr(MSG_FI_FindIn)),
                Child, ListviewObject,
                   MUIA_Listview_Input, TRUE,
                   MUIA_Listview_MultiSelect, TRUE,
                   MUIA_CycleChain      , 1,
                   MUIA_Listview_List, data->GUI.LV_FOLDERS = ListObject,
                      InputListFrame,
                      MUIA_List_AutoVisible, TRUE,
                      MUIA_List_AdjustWidth, TRUE,
                   End,
                End,
                Child, HGroup,
                  Child, bt_all = MakeButton(tr(MSG_FI_AllFolders)),
                  Child, bt_none = MakeButton(tr(MSG_FI_NOFOLDERS)),
                End,
             End,
             Child, VGroup, GroupFrameT(tr(MSG_FI_FindWhat)),
                Child, VSpace(0),
                Child, data->GUI.GR_SEARCH = SearchControlGroupObject,
                  MUIA_SearchControlGroup_RemoteFilterMode, FALSE,
                End,
                Child, ColGroup(2),
                   Child, po_fromrule = PopobjectObject,
                      MUIA_Popstring_Button, MakeButton(tr(MSG_FI_UseFilter)),
                      MUIA_Popobject_ObjStrHook, &SearchOptFromFilterPopupHook,
                      MUIA_Popobject_StrObjHook, &InitFilterPopupListHook,
                      MUIA_Popobject_WindowHook, &PO_WindowHook,
                      MUIA_Popobject_Object, lv_fromrule = ListviewObject,
                         MUIA_Listview_List, ListObject,
                           InputListFrame,
                           MUIA_List_DisplayHook, &FilterPopupDisplayHook,
                         End,
                      End,
                   End,
                   Child, bt_torule = MakeButton(tr(MSG_FI_AddAsFilter)),
                End,
                Child, VSpace(0),
             End,
          End,
          Child, NBalanceObject,
          End,
          Child, VGroup, GroupFrameT(tr(MSG_FI_Results)),
             MUIA_VertWeight, 100,
             Child, NListviewObject,
                MUIA_CycleChain, TRUE,
                MUIA_NListview_NList, data->GUI.LV_MAILS = MainMailListObject,
                   //MUIA_ObjectID,       MAKE_ID('N','L','0','3'),
                   MUIA_ContextMenu,    NULL,
                   MUIA_NList_Format,   "COL=8 W=-1 MIW=-1 BAR, COL=1 W=35 BAR, COL=3 W=55 BAR, COL=4 W=-1 MIW=-1 BAR, COL=7 W=-1 MIW=-1 BAR, COL=5 W=10 MIW=-1 P=\33r BAR",
                   //MUIA_NList_Format,   "COL=8 W=-1 BAR, COL=1 W=-1 BAR, COL=3 W=-1 BAR, COL=4 W=-1 BAR, COL=7 W=-1 BAR, COL=5 W=-1 P=\33r BAR",
                   //MUIA_NList_Exports,  MUIV_NList_Exports_ColWidth|MUIV_NList_Exports_ColOrder,
                   //MUIA_NList_Imports,  MUIV_NList_Imports_ColWidth|MUIV_NList_Imports_ColOrder,
                   MUIA_MainMailList_HandleDoubleClick, FALSE,
                End,
             End,
          End,
          Child, data->GUI.GR_PAGE = PageGroup,
             Child, HGroup,
                Child, data->GUI.BT_SEARCH = MakeButton(tr(MSG_FI_StartSearch)),
                Child, data->GUI.BT_SELECTACTIVE = MakeButton(tr(MSG_FI_SELECTACTIVE)),
                Child, data->GUI.BT_SELECT = MakeButton(tr(MSG_FI_SelectMatched)),
                Child, data->GUI.BT_READ = MakeButton(tr(MSG_FI_ReadMessage)),
             End,
             Child, HGroup,
                Child, data->GUI.GA_PROGRESS = GaugeObject,
                   MUIA_HorizWeight, 200,
                   GaugeFrame,
                   MUIA_Gauge_Horiz,TRUE,
                End,
                Child, bt_abort = MakeButton(tr(MSG_FI_Abort)),
             End,
          End,
       End,
    End;

    if(data->GUI.WI != NULL)
    {
       set(data->GUI.BT_SELECTACTIVE, MUIA_Disabled, TRUE);
       set(data->GUI.BT_SELECT,       MUIA_Disabled, TRUE);
       set(data->GUI.BT_READ,         MUIA_Disabled, TRUE);

       xset(data->GUI.WI, MUIA_Window_ActiveObject,  xget(data->GUI.GR_SEARCH, MUIA_SearchControlGroup_ActiveObject),
                          MUIA_Window_DefaultObject, data->GUI.LV_MAILS);

       SetHelp(data->GUI.LV_FOLDERS,       MSG_HELP_FI_LV_FOLDERS);
       SetHelp(bt_all,                     MSG_HELP_FI_BT_ALL);
       SetHelp(bt_none,                    MSG_HELP_FI_BT_NONE);
       SetHelp(po_fromrule,                MSG_HELP_FI_PO_FROMRULE);
       SetHelp(bt_torule,                  MSG_HELP_FI_BT_TORULE);
       SetHelp(data->GUI.BT_SEARCH,        MSG_HELP_FI_BT_SEARCH);
       SetHelp(data->GUI.BT_SELECTACTIVE,  MSG_HELP_FI_BT_SELECTACTIVE);
       SetHelp(data->GUI.BT_SELECT,        MSG_HELP_FI_BT_SELECT);
       SetHelp(data->GUI.BT_READ,          MSG_HELP_FI_BT_READ);
       SetHelp(bt_abort,                   MSG_HELP_FI_BT_ABORT);

       DoMethod(G->App, OM_ADDMEMBER, data->GUI.WI);

       DoMethod(bt_abort,                 MUIM_Notify, MUIA_Pressed,              FALSE,          MUIV_Notify_Application, 3, MUIM_WriteLong, TRUE, &(data->Abort));
       DoMethod(lv_fromrule,              MUIM_Notify, MUIA_Listview_DoubleClick, TRUE,           po_fromrule            , 2, MUIM_Popstring_Close, TRUE);
       DoMethod(bt_all,                   MUIM_Notify, MUIA_Pressed,              FALSE,          data->GUI.LV_FOLDERS   , 5, MUIM_List_Select, MUIV_List_Select_All, MUIV_List_Select_On, NULL);
       DoMethod(bt_none,                  MUIM_Notify, MUIA_Pressed,              FALSE,          data->GUI.LV_FOLDERS   , 5, MUIM_List_Select, MUIV_List_Select_All, MUIV_List_Select_Off, NULL);
       DoMethod(bt_torule,                MUIM_Notify, MUIA_Pressed,              FALSE,          MUIV_Notify_Application, 2, MUIM_CallHook, &CreateFilterFromSearchHook);
       DoMethod(data->GUI.BT_SEARCH,      MUIM_Notify, MUIA_Pressed,              FALSE,          MUIV_Notify_Application, 2, MUIM_CallHook, &FI_SearchHook);
       DoMethod(data->GUI.BT_SELECT,      MUIM_Notify, MUIA_Pressed,              FALSE,          MUIV_Notify_Application, 2, MUIM_CallHook, &FI_SelectHook);
       DoMethod(data->GUI.BT_SELECTACTIVE,MUIM_Notify, MUIA_Pressed,              FALSE,          MUIV_Notify_Application, 2, MUIM_CallHook, &FI_SwitchHook);
       DoMethod(data->GUI.LV_MAILS,       MUIM_Notify, MUIA_NList_DoubleClick,    MUIV_EveryTime, MUIV_Notify_Application, 2, MUIM_CallHook, &FI_ReadHook);
       DoMethod(data->GUI.BT_READ,        MUIM_Notify, MUIA_Pressed,              FALSE,          MUIV_Notify_Application, 2, MUIM_CallHook, &FI_ReadHook);
       DoMethod(data->GUI.WI,             MUIM_Notify, MUIA_Window_CloseRequest,  TRUE,           MUIV_Notify_Application, 2, MUIM_CallHook, &FI_CloseHook);

       // Lets have the Listview sorted by Reverse Date by default
       set(data->GUI.LV_MAILS, MUIA_NList_SortType, (4 | MUIV_NList_SortTypeAdd_2Values));
    }
    else
    {
      free(data);
      data = NULL;
    }
  }

  RETURN(data);
  return data;
}

///
