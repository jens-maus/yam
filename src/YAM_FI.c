/***************************************************************************

 YAM - Yet Another Mailer
 Copyright (C) 1995-2000 by Marcel Beck <mbeck@yam.ch>
 Copyright (C) 2000-2007 by YAM Open Source Team

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
#include "YAM_locale.h"
#include "YAM_main.h"
#include "YAM_mainFolder.h"
#include "YAM_read.h"
#include "YAM_utilities.h"
#include "classes/Classes.h"
#include "BayesFilter.h"

#include "Debug.h"

/* local protos */
static BOOL FI_MatchString(struct Search*, char*);
static BOOL FI_MatchListPattern(struct Search*, char*);
static BOOL FI_MatchPerson(struct Search*, struct Person*);
static BOOL FI_SearchPatternFast(struct Search*, struct Mail*);
static BOOL FI_SearchPatternInBody(struct Search*, struct Mail*);
static BOOL FI_SearchPatternInHeader(struct Search*, struct Mail*);
static enum FastSearch FI_IsFastSearch(const char*);
static void FI_GenerateListPatterns(struct Search*);
static struct FI_ClassData *FI_New(void);
static void CopySearchData(struct Search *dstSearch, struct Search *srcSearch);

/***************************************************************************
 Module: Find & Filters
***************************************************************************/

/// Global variables
const int Mode2Group[12] = { 0,0,0,0,1,2,1,2,4,4,4,3 };

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

  switch (search->Compare)
  {
    case 0:
      match = (BOOL)(search->CaseSens ? MatchPattern(search->Pattern, string) : MatchPatternNoCase(search->Pattern, string));
    break;

    case 1:
      match = (BOOL)(search->CaseSens ? !MatchPattern(search->Pattern, string) : !MatchPatternNoCase(search->Pattern, string));
    break;

    case 2:
      match = (BOOL)(search->CaseSens ? strcmp(string, search->Match) < 0 : Stricmp(string, search->Match) < 0);
    break;

    case 3:
      match = (BOOL)(search->CaseSens ? strcmp(string, search->Match) > 0 : Stricmp(string, search->Match) > 0);
    break;

    default:
      match = FALSE;
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
  struct MinList *patternList = &search->patternList;

  ENTER();

  if(IsMinListEmpty(patternList) == FALSE)
  {
    struct MinNode *curNode;

    // Now we process the read header to set all flags accordingly
    for(curNode = patternList->mlh_Head; curNode->mln_Succ; curNode = curNode->mln_Succ)
    {
      struct SearchPatternNode *patternNode = (struct SearchPatternNode *)curNode;

      if(search->CaseSens ? MatchPattern(patternNode->pattern, string)
                          : MatchPatternNoCase(patternNode->pattern, string))
      {
        match = TRUE;
        break;
      }
    }
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
      else if(isMultiSenderMail(mail) && (email = MA_ExamineMail(mail->Folder, mail->MailFile, TRUE)))
      {
        int i;

        for(i=0; i < email->NoSFrom; i++)
        {
          if(FI_MatchPerson(search, &email->SFrom[i]))
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
      else if(isMultiRCPTMail(mail) && (email = MA_ExamineMail(mail->Folder, mail->MailFile, TRUE)))
      {
        int i;

        for(i=0; i < email->NoSTo; i++)
        {
          if(FI_MatchPerson(search, &email->STo[i]))
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

      if(isMultiRCPTMail(mail) && (email = MA_ExamineMail(mail->Folder, mail->MailFile, TRUE)))
      {
        int i;

        for(i=0; i < email->NoCC; i++)
        {
          if(FI_MatchPerson(search, &email->CC[i]))
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
      else if(isMultiReplyToMail(mail) && (email = MA_ExamineMail(mail->Folder, mail->MailFile, TRUE)))
      {
        int i;

        for(i=0; i < email->NoSReplyTo; i++)
        {
          if(FI_MatchPerson(search, &email->SReplyTo[i]))
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

    while(*rptr && !found && (G->FI ? !G->FI->Abort : TRUE))
    {
      for(ptr = rptr; *ptr && *ptr != '\n'; ptr++);

      *ptr = 0;
      if(FI_MatchString(search, rptr))
        found = TRUE;

      rptr = ++ptr;
    }

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
  BOOL found = FALSE;

  ENTER();

  if(StartUnpack(GetMailFile(NULL, mail->Folder, mail), fullfile, mail->Folder))
  {
    FILE *fh;

    if((fh = fopen(fullfile, "r")) != NULL)
    {
      struct MinList *headerList;

      if((headerList = calloc(1, sizeof(struct MinList))) != NULL)
      {
        setvbuf(fh, NULL, _IOFBF, SIZE_FILEBUF);

        if(MA_ReadHeader(fh, headerList))
        {
          struct MinNode *curNode = headerList->mlh_Head;

          // search through our headerList
          for(; curNode->mln_Succ && !found; curNode = curNode->mln_Succ)
          {
            struct HeaderNode *hdrNode = (struct HeaderNode *)curNode;

            // if the field is explicitly specified we search for it or
            // otherwise skip our search
            if(*search->Field)
            {
              int searchLen;
              char *ptr = strchr(search->Field, ':');

              // if the field is specified we search if it was specified with a ':'
              // at the end
              if(ptr)
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

          // free our temporary header list
          FreeHeaderList(headerList);
        }

        // free our temporary headerList
        free(headerList);
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
    char buf[SIZE_PATTERN];

    setvbuf(fh, NULL, _IOFBF, SIZE_FILEBUF);

    // make sure the pattern list is successfully freed
    FreeSearchPatternList(search);

    while(GetLine(fh, buf, sizeof(buf)) != NULL)
    {
      if(buf[0] != '\0')
      {
        char pattern[SIZE_PATTERN*2+2]; // ParsePattern() needs at least 2*source+2 bytes buffer
        struct SearchPatternNode *newNode;

        if(search->CaseSens)
          ParsePattern(buf, pattern, sizeof(pattern));
        else
          ParsePatternNoCase(buf, pattern, sizeof(pattern));

        // put the pattern in our search pattern list
        if((newNode = malloc(sizeof(struct SearchPatternNode))) != NULL)
        {
          strlcpy(newNode->pattern, pattern, sizeof(newNode->pattern));

          // add the pattern to our list
          AddTail((struct List *)&search->patternList, (struct Node *)newNode);
        }
      }
    }
    fclose(fh);
  }

  LEAVE();
}

///
/// FI_PrepareSearch
//  Initializes Search structure
BOOL FI_PrepareSearch(struct Search *search, enum SearchMode mode,
                      BOOL casesens, int persmode, int compar,
                      char stat, BOOL substr, const char *match, const char *field)
{
  BOOL success = TRUE;

  ENTER();

  memset(search, 0, sizeof(struct Search));
  search->Mode      = mode;
  search->CaseSens  = casesens;
  search->PersMode  = persmode;
  search->Compare   = compar;
  search->Status    = stat;
  search->SubString = substr;
  strlcpy(search->Match, match, sizeof(search->Match));
  strlcpy(search->Field, field, sizeof(search->Field));
  search->Pattern = search->PatBuf;
  search->Fast = FS_NONE;
  NewList((struct List *)&search->patternList);

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
      if(substr || mode == SM_HEADER || mode == SM_BODY || mode == SM_WHOLE || mode == SM_STATUS)
      {
        char buffer[SIZE_PATTERN+1];

        // if substring is selected lets generate a substring out
        // of the current match string, but keep the string borders in mind.
        strlcpy(buffer, search->Match, sizeof(buffer));
        snprintf(search->Match, sizeof(search->Match), "#?%s#?", buffer);
      }

      if(casesens)
        ParsePattern(search->Match, search->Pattern, (SIZE_PATTERN+4)*2+2);
      else
        ParsePatternNoCase(search->Match, search->Pattern, (SIZE_PATTERN+4)*2+2);
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
   BOOL found0, found = FALSE;
   int comp_bak = search->Compare;

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
      }
      break;

      case SM_HEADER:
      {
        search->Compare = 0;
        found0 = FI_SearchPatternInHeader(search, mail);
        search->Compare = comp_bak;
        if (found0 == (search->Compare == 0)) found = TRUE;
      }
      break;

      case SM_BODY:
      {
        search->Compare = 0;
        found0 = FI_SearchPatternInBody(search, mail);
        search->Compare = comp_bak;
        if (found0 == (search->Compare == 0)) found = TRUE;
      }
      break;

      case SM_WHOLE:
      {
        search->Compare = 0;
        if (!(found0 = FI_SearchPatternInHeader(search, mail)))
        found0 = FI_SearchPatternInBody(search, mail);
        search->Compare = comp_bak;
        if(found0 == (search->Compare == 0)) found = TRUE;
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
            statusFound = hasStatusOld(mail);
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
          found = TRUE;
        }
      }
      break;

      case SM_SPAM:
      {
        D(DBF_FILTER, "search for spam");
      	if(C->SpamFilterEnabled && BayesFilterClassifyMessage(mail))
      	{
      	  found = TRUE;
      	}
      }
      break;
   }

   return found;
}

///
/// DoFilterSearch()
//  Does a complex search with combined criterias based on the rules of a filter
BOOL DoFilterSearch(struct FilterNode *filter, struct Mail *mail)
{
  BOOL lastCond = FALSE;

  ENTER();

  if(IsMinListEmpty(&filter->ruleList) == FALSE)
  {
    int i = 0;
    struct MinNode *curNode;

    // we have to iterate through our ruleList and depending on the combine
    // operation we evaluate if the filter hits any mail criteria or not.
    for(curNode = filter->ruleList.mlh_Head; curNode->mln_Succ; curNode = curNode->mln_Succ)
    {
      struct RuleNode *rule = (struct RuleNode *)curNode;

      if(rule->search)
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
  }

  RETURN(lastCond);
  return lastCond;
}

///
/// FI_SearchFunc
//  Starts the search and shows progress
HOOKPROTONHNONP(FI_SearchFunc, void)
{
   int sfonum = 0, fnr, id, i, fndmsg = 0, totmsg = 0, progress = 0;
   struct FI_GUIData *gui = &G->FI->GUI;
   struct Folder **sfo, *folder;
   char *name;
   static char gauge[40];
   struct Mail *mail;
   APTR ga = gui->GA_PROGRESS;
   struct Search search;
   struct TimeVal now;
   struct TimeVal last;

   // by default we don`t dispose on end
   G->FI->DisposeOnEnd = FALSE;
   G->FI->SearchActive = TRUE;
   G->FI->Abort        = FALSE;

   set(gui->WI, MUIA_Window_ActiveObject, MUIV_Window_ActiveObject_None);
   set(gui->BT_SELECTACTIVE, MUIA_Disabled, TRUE);
   set(gui->BT_SELECT, MUIA_Disabled, TRUE);
   set(gui->BT_READ, MUIA_Disabled, TRUE);
   DoMethod(gui->LV_MAILS, MUIM_NList_Clear);
   fnr = xget(gui->LV_FOLDERS, MUIA_List_Entries);

   if((sfo = calloc(fnr, sizeof(struct Folder *))) != NULL)
   {
     id = MUIV_List_NextSelected_Start;
     while(TRUE)
     {
       DoMethod(gui->LV_FOLDERS, MUIM_List_NextSelected, &id);
       if(id == MUIV_List_NextSelected_End)
         break;

       DoMethod(gui->LV_FOLDERS, MUIM_List_GetEntry, id, &name);
       if((folder = FO_GetFolderByName(name, NULL)) != NULL)
       {
         if(MA_GetIndex(folder))
         {
           sfo[sfonum++] = folder;
           totmsg += folder->Total;
         }
       }
     }

     if(totmsg != 0)
     {
       // lets prepare the search
       DoMethod(gui->GR_SEARCH, MUIM_SearchControlGroup_PrepareSearch, &search);

       // set the gauge
       snprintf(gauge, sizeof(gauge), tr(MSG_FI_GAUGETEXT), totmsg);
       SetAttrs(ga, MUIA_Gauge_InfoText, gauge,
                    MUIA_Gauge_Max,      totmsg,
                    MUIA_Gauge_Current,  0,
                    TAG_DONE);

       set(gui->GR_PAGE, MUIA_Group_ActivePage, 1);

       memset(&last, 0, sizeof(struct TimeVal));

       for(i = 0; i < sfonum && !G->FI->Abort; i++)
       {
         for(mail = sfo[i]->Messages; mail && !G->FI->Abort; mail = mail->Next)
         {
           if(FI_DoSearch(&search, mail))
           {
             DoMethod(gui->LV_MAILS, MUIM_NList_InsertSingle, mail, MUIV_NList_Insert_Sorted);
             fndmsg++;
           }

           // increase the progress counter
           progress++;

           // then we update the gauge, but we take also care of not refreshing
           // it too often or otherwise it slows down the whole search process.
           GetSysTime(TIMEVAL(&now));
           if(-CmpTime(TIMEVAL(&now), TIMEVAL(&last)) > 0)
           {
             struct TimeVal delta;

             // how much time has passed exactly?
             memcpy(&delta, &now, sizeof(struct TimeVal));
             SubTime(TIMEVAL(&delta), TIMEVAL(&last));

             // update the display at least twice a second
             if(delta.Seconds > 0 || delta.Microseconds > 250000)
             {
               set(ga, MUIA_Gauge_Current, progress);

               // signal the application to update now
               DoMethod(G->App, MUIM_Application_InputBuffered);

               memcpy(&last, &now, sizeof(struct TimeVal));
             }
           }
         }
       }

       // to let the gauge move to 100% lets increase it accordingly.
       set(ga, MUIA_Gauge_Current, progress);

       // signal the application to update now
       DoMethod(G->App, MUIM_Application_InputBuffered);

       FreeSearchPatternList(&search);
     }

     free(sfo);
   }

   set(gui->GR_PAGE, MUIA_Group_ActivePage, 0);
   set(gui->BT_SELECTACTIVE, MUIA_Disabled, !fndmsg);
   set(gui->BT_SELECT, MUIA_Disabled, !fndmsg);
   set(gui->BT_READ, MUIA_Disabled, !fndmsg);

   G->FI->SearchActive = FALSE;

   // if the closeHook has set the DisposeOnEnd flag we have to dispose
   // our object now.
   if(G->FI->DisposeOnEnd)
     DisposeModulePush(&G->FI);
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
  BOOL success = FALSE;

  ENTER();

  if(G->FI == NULL)
  {
    if((G->FI = FI_New()) != NULL)
    {
      struct Folder *folder;

      if((folder = FO_GetCurrentFolder()) != NULL)
      {
        struct Folder **flist;

        if((flist = FO_CreateList()) != NULL)
        {
          int i;
          int j = 0;
          int apos = 0;

          for(i = 1; i <= (int)*flist; i++)
          {
            if(isGroupFolder(flist[i]) == FALSE)
            {
              DoMethod(G->FI->GUI.LV_FOLDERS, MUIM_List_InsertSingle, flist[i]->Name, MUIV_List_Insert_Bottom);

              if(flist[i] == folder)
                apos = j;

              j++;
            }
          }
          set(G->FI->GUI.LV_FOLDERS, MUIA_List_Active, apos);
          free(flist);

          // everything went fine
          success = TRUE;
        }
      }
    }
  }

  if(success == FALSE || SafeOpenWindow(G->FI->GUI.WI) == FALSE)
    DisposeModulePush(&G->FI);

  LEAVE();
}
MakeHook(FI_OpenHook,FI_Open);

///
/// FI_SwitchFunc
//  Sets active folder according to the selected message in the results window
HOOKPROTONHNONP(FI_SwitchFunc, void)
{
  struct Mail *foundmail;

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
      SetAttrs(G->MA->GUI.PG_MAILLIST, MUIA_NList_DisplayRecall, TRUE,
                                       MUIA_NList_Active, pos,
                                       TAG_DONE);

      set(G->FI->GUI.LV_MAILS, MUIA_NList_DisplayRecall, TRUE);
    }
  }
}
MakeStaticHook(FI_SwitchHook, FI_SwitchFunc);

///
/// FI_ReadFunc
//  Reads a message listed in the results window
HOOKPROTONHNONP(FI_ReadFunc, void)
{
  struct Mail *mail;

  DoMethod(G->FI->GUI.LV_MAILS, MUIM_NList_GetEntry, MUIV_NList_GetEntry_Active, &mail);

  if(mail)
  {
    struct ReadMailData *rmData;
    if((rmData = CreateReadWindow(FALSE)))
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
}
MakeStaticHook(FI_ReadHook, FI_ReadFunc);

///
/// FI_SelectFunc
//  Selects matching messages in the main message list
HOOKPROTONHNONP(FI_SelectFunc, void)
{
   int i;
   struct Folder *folder = FO_GetCurrentFolder();

   if(!folder) return;

   DoMethod(G->MA->GUI.PG_MAILLIST, MUIM_NList_Select, MUIV_NList_Select_All, MUIV_NList_Select_Off, NULL);

   for(i=0; ;i++)
   {
      struct Mail *foundmail;

      DoMethod(G->FI->GUI.LV_MAILS, MUIM_NList_GetEntry, i, &foundmail);
      if (!foundmail) break;

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
}
MakeStaticHook(FI_SelectHook, FI_SelectFunc);

///
/// FI_Close
//  Closes find window
HOOKPROTONHNONP(FI_Close, void)
{
   if(!G->FI->SearchActive)
   {
     DisposeModulePush(&G->FI);
   }
   else
   {
     // set the abort flag so that a running search will be stopped.
     G->FI->Abort = TRUE;
     G->FI->DisposeOnEnd = TRUE;

     set(G->FI->GUI.WI, MUIA_Window_Open, FALSE);
   }
}
MakeStaticHook(FI_CloseHook, FI_Close);
///
/// FreeSearchPatternList()
// Function to make the whole pattern list is correctly cleaned up
void FreeSearchPatternList(struct Search *search)
{
  struct MinList *patternList = &search->patternList;

  ENTER();

  if(IsMinListEmpty(patternList) == FALSE)
  {
    struct MinNode *curNode;

    // Now we process the read header to set all flags accordingly
    for(curNode = patternList->mlh_Head; curNode->mln_Succ;)
    {
      struct SearchPatternNode *patternNode = (struct SearchPatternNode *)curNode;

      // before we remove the node we have to save the pointer to the next one
      curNode = curNode->mln_Succ;

      // Remove node from list
      Remove((struct Node *)patternNode);

      free(patternNode);
    }
  }

  LEAVE();
}

///
/// AllocFilterSearch()
//  Allocates and initializes search structures for filters and returns
//  the number of active filters/search structures allocated.
int AllocFilterSearch(enum ApplyFilterMode mode)
{
  int active=0;

  ENTER();

  if(IsMinListEmpty(&C->filterList) == FALSE)
  {
    struct MinNode *curNode;

    // iterate through our filter List
    for(curNode = C->filterList.mlh_Head; curNode->mln_Succ; curNode = curNode->mln_Succ)
    {
      struct FilterNode *filter = (struct FilterNode *)curNode;
      struct MinNode *curRuleNode;

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
        if(IsMinListEmpty(&filter->ruleList) == FALSE)
        {
          // make sure the current search structures of the rules of the filter
          // are freed
          for(curRuleNode = filter->ruleList.mlh_Head; curRuleNode->mln_Succ; curRuleNode = curRuleNode->mln_Succ)
          {
            struct RuleNode *rule = (struct RuleNode *)curRuleNode;

            // now we do free our search structure if it exists
            if(rule->search)
            {
              FreeSearchPatternList(rule->search);
              free(rule->search);
              rule->search = NULL;
            }
          }
        }
      }
      else
      {
        if(IsMinListEmpty(&filter->ruleList) == FALSE)
        {
          // check if the search structures are already allocated or not
          for(curRuleNode = filter->ruleList.mlh_Head; curRuleNode->mln_Succ; curRuleNode = curRuleNode->mln_Succ)
          {
            struct RuleNode *rule = (struct RuleNode *)curRuleNode;

            // check if that search structure already exists or not
            if(rule->search == NULL &&
               (rule->search = calloc(1, sizeof(struct Search))))
            {
              int stat = sizeof(mailStatusCycleMap);

              // we check the status field first and if we find a match
              // we can immediatly break up here because we don`t need to prepare the search
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
                               rule->matchPattern,
                               rule->customField);

              // save a pointer to the filter in the search structure as well.
              rule->search->filter = filter;
            }
          }

          active++;
        }
      }
    }
  }

  RETURN(active);
  return active;
}

///
/// FreeFilterSearch()
//  Frees filter search structures
void FreeFilterSearch(void)
{
  struct MinNode *curNode;

  ENTER();

  // Now we process the read header to set all flags accordingly
  for(curNode = C->filterList.mlh_Head; curNode->mln_Succ; curNode = curNode->mln_Succ)
  {
    struct FilterNode *filter = (struct FilterNode *)curNode;

    if(IsMinListEmpty(&filter->ruleList) == FALSE)
    {
      struct MinNode *curRuleNode;

      for(curRuleNode = filter->ruleList.mlh_Head; curRuleNode->mln_Succ; curRuleNode = curRuleNode->mln_Succ)
      {
        struct RuleNode *rule = (struct RuleNode *)curRuleNode;

        // now we do free our search structure if it exists
        if(rule->search != NULL)
        {
          FreeSearchPatternList(rule->search);
          free(rule->search);
          rule->search = NULL;
        }
      }
    }
  }

  LEAVE();
}

///
/// ExecuteFilterAction()
//  Applies filter action to a message and return TRUE if the filter search
//  should continue or FALSE if it should stop afterwards
BOOL ExecuteFilterAction(struct FilterNode *filter, struct Mail *mail)
{
  BOOL success = TRUE;
  struct Mail *mlist[3];

  ENTER();

  // initialize mlist
  mlist[0] = (struct Mail *)1;
  mlist[1] = NULL;
  mlist[2] = mail;

  // Bounce Action
  if(hasBounceAction(filter) && !filter->remote && *filter->bounceTo)
  {
    G->RRs.Bounced++;
    MA_NewBounce(mail, TRUE);
    setstring(G->WR[2]->GUI.ST_TO, filter->bounceTo);
    DoMethod(G->App, MUIM_CallHook, &WR_NewMailHook, WRITE_QUEUE, 2);
  }

  // Forward Action
  if(hasForwardAction(filter) && !filter->remote && *filter->forwardTo)
  {
    G->RRs.Forwarded++;
    MA_NewForward(mlist, TRUE);
    setstring(G->WR[2]->GUI.ST_TO, filter->forwardTo);
    WR_NewMail(WRITE_QUEUE, 2);
  }

  // Reply Action
  if(hasReplyAction(filter) && !filter->remote && *filter->replyFile)
  {
    MA_NewReply(mlist, TRUE);
    FileToEditor(filter->replyFile, G->WR[2]->GUI.TE_EDIT);
    WR_NewMail(WRITE_QUEUE, 2);
    G->RRs.Replied++;
  }

  // Execute Action
  if(hasExecuteAction(filter) && *filter->executeCmd)
  {
    char buf[SIZE_COMMAND+SIZE_PATHFILE];
    snprintf(buf, sizeof(buf), "%s \"%s\"", filter->executeCmd, GetRealPath(GetMailFile(NULL, NULL, mail)));
    ExecuteCommand(buf, FALSE, OUT_DOS);
    G->RRs.Executed++;
  }

  // PlaySound Action
  if(hasPlaySoundAction(filter) && *filter->playSound)
  {
    PlaySound(filter->playSound);
  }

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

        G->RRs.Moved++;

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
    G->RRs.Deleted++;

    if(isSendMDNMail(mail) &&
       (hasStatusNew(mail) || !hasStatusRead(mail)))
    {
      RE_ProcessMDN(MDN_MODE_DELETE, mail, FALSE, TRUE);
    }

    MA_DeleteSingle(mail, FALSE, FALSE, TRUE);

    // signal failure, although everything was successful yet
    // but the mail is not available anymore for other filters
    success = FALSE;
  }

  RETURN(success);
  return success;
}
///
/// ApplyFiltersFunc()
//  Apply filters
HOOKPROTONHNO(ApplyFiltersFunc, void, int *arg)
{
  struct Folder *folder;
  struct Folder *spamfolder = NULL;
  enum ApplyFilterMode mode = arg[0];

  ENTER();

  D(DBF_FILTER, "About to apply SPAM and user defined filters...");

  if((folder = (mode == APPLY_AUTO) ? FO_GetFolderByType(FT_INCOMING, NULL) : FO_GetCurrentFolder()) != NULL &&
     (C->SpamFilterEnabled == FALSE || (spamfolder = FO_GetFolderByType(FT_SPAM, NULL)) != NULL))
  {
    BOOL applyFilters = TRUE;

    // if this function was called manually by the user we ask him
    // if he really wants to apply the filters or not.
    if(mode == APPLY_USER)
    {
      char buf[SIZE_LARGE];

      snprintf(buf, sizeof(buf), tr(MSG_MA_ConfirmFilter), folder->Name);
      if(!MUI_Request(G->App, G->MA->GUI.WI, 0, tr(MSG_MA_ConfirmReq), tr(MSG_YesNoReq), buf))
        applyFilters = FALSE;
    }

    // the user has not cancelled the filter process
    if(applyFilters)
    {
      Object *lv = G->MA->GUI.PG_MAILLIST;
      struct Mail **mlist = NULL;

      memset(&G->RRs, 0, sizeof(struct RuleResult));
      set(lv, MUIA_NList_Quiet, TRUE);
      G->AppIconQuiet = TRUE;

      if(mode == APPLY_USER || mode == APPLY_RX || mode == APPLY_RX_ALL || mode == APPLY_SPAM)
      {
        int minselected = hasFlag(arg[1], (IEQUALIFIER_LSHIFT|IEQUALIFIER_RSHIFT)) ? 1 : 2;

        if((mlist = MA_CreateMarkedList(lv, mode == APPLY_RX)) != NULL && (int)mlist[0] < minselected)
        {
          free(mlist);
          mlist = NULL;
        }
      }

      if(mlist == NULL)
        mlist = MA_CreateFullList(folder, (mode == APPLY_AUTO || mode == APPLY_RX));

      if(mlist != NULL)
      {
        int m;
        int scnt;
        int matches = 0;
        struct Mail *mail;

        // for simple spam classification this will result in zero
        scnt = AllocFilterSearch(mode);

        // we use another Busy Gauge information if this is
        // a spam classification session. And we build an interruptable
        // Gauge which will report back if the user pressed the stop button
        if(mode != APPLY_SPAM)
          BusyGaugeInt(tr(MSG_BusyFiltering), "", (int)*mlist);
        else
          BusyGaugeInt(tr(MSG_FI_BUSYCHECKSPAM), "", (int)*mlist);

        for(m = 0; m < (int)*mlist; m++)
        {
          BOOL wasSpam = FALSE;
          struct MinNode *curNode;

          if((mail = mlist[m + 2]) != NULL)
          {
            if(C->SpamFilterEnabled && (mode == APPLY_AUTO || mode == APPLY_SPAM))
            {
              BOOL doClassification;

              D(DBF_FILTER, "About to apply SPAM filter to message with subject \"%s\"", mail->Subject);
              if(mode == APPLY_AUTO && C->SpamFilterForNewMail)
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

              if(doClassification)
              {
                D(DBF_FILTER, "Classifying message with subject \"%s\"", mail->Subject);

                if(BayesFilterClassifyMessage(mail))
                {
                  D(DBF_FILTER, "Message was classified as spam");

                  // set the SPAM flags, but clear the NEW and READ flags only if desired
                  if(C->SpamMarkAsRead)
                    setStatusToReadAutoSpam(mail);
                  else
                    setStatusToAutoSpam(mail);

                  // move newly recognized spam to the spam folder
                  MA_MoveCopy(mail, folder, spamfolder, FALSE, FALSE);
                  wasSpam = TRUE;
                }
              }
            }

            if(scnt > 0 && wasSpam == FALSE)
            {
              // apply all other user defined filters (if they exist) for non-spam mails
              // or if the spam filter is disabled
              G->RRs.Checked++;

              // now we process the search
              for(curNode = C->filterList.mlh_Head; curNode->mln_Succ; curNode = curNode->mln_Succ)
              {
                struct FilterNode *filter = (struct FilterNode *)curNode;

                if(DoFilterSearch(filter, mail))
                {
                  matches++;

                  // if ExecuteFilterAction returns FALSE then the filter search should be aborted
                  // completley
                  if(ExecuteFilterAction(filter, mail) == FALSE)
                    break;
                }
              }
            }

            // we update the busy gauge and
            // see if we have to exit/abort in case it returns FALSE
            if(BusySet(m+1) == FALSE)
              break;
          }
        }

        FreeFilterSearch();

        if(G->RRs.Checked)
          AppendLog(26, tr(MSG_LOG_Filtering), G->RRs.Checked, folder->Name, matches);

        BusyEnd();

        free(mlist);
      }

      set(lv, MUIA_NList_Quiet, FALSE);
      G->AppIconQuiet = FALSE;

      if(mode != APPLY_AUTO)
        DisplayStatistics(NULL, TRUE);

      if(G->RRs.Checked && mode == APPLY_USER)
      {
        char buf[SIZE_LARGE];

        snprintf(buf, sizeof(buf), tr(MSG_MA_FilterStats), G->RRs.Checked, G->RRs.Forwarded, G->RRs.Moved, G->RRs.Deleted);
        MUI_Request(G->App, G->MA->GUI.WI, 0, NULL, tr(MSG_OkayReq), buf);
      }
    }
  }

  LEAVE();
}
MakeHook(ApplyFiltersHook, ApplyFiltersFunc);
///
/// CopyFilterData()
// copy all data of a filter node (deep copy)
void CopyFilterData(struct FilterNode *dstFilter, struct FilterNode *srcFilter)
{
  ENTER();

  SHOWSTRING(DBF_FILTER, srcFilter->name);

  // raw copy all global stuff first
  memcpy(dstFilter, srcFilter, sizeof(struct FilterNode));

  // then iterate through our ruleList and copy it as well
  NewList((struct List *)&dstFilter->ruleList);

  if(IsMinListEmpty(&srcFilter->ruleList) == FALSE)
  {
    struct MinNode *curNode;

    for(curNode = srcFilter->ruleList.mlh_Head; curNode->mln_Succ; curNode = curNode->mln_Succ)
    {
      struct RuleNode *rule = (struct RuleNode *)curNode;
      struct RuleNode *newRule;

      if((newRule = calloc(1, sizeof(struct RuleNode))) != NULL)
      {
        // do a raw copy of the rule contents first
        memcpy(newRule, rule, sizeof(struct RuleNode));

        // check if the search structure exists and if so
        // so start another deep copy
        if(rule->search)
        {
          if((newRule->search = calloc(1, sizeof(struct Search))) != NULL)
            CopySearchData(newRule->search, rule->search);
        }
        else
          newRule->search = NULL;

        // add the rule to the ruleList of our desitionation filter
        AddTail((struct List *)&dstFilter->ruleList, (struct Node *)newRule);
      }
    }
  }

  LEAVE();
}
///
/// CopySearchData()
// copy all data of a search structure (deep copy)
static void CopySearchData(struct Search *dstSearch, struct Search *srcSearch)
{
  struct MinNode *curNode;

  ENTER();

  // raw copy all global stuff first
  memcpy(dstSearch, srcSearch, sizeof(struct Search));

  // then we check whether we have to copy another bunch of sub data or not
  if(srcSearch->Pattern == srcSearch->PatBuf)
    dstSearch->Pattern = dstSearch->PatBuf;
  else if(srcSearch->Pattern == (char *)&(srcSearch->DT.dat_Stamp))
    dstSearch->Pattern = (char *)&(dstSearch->DT.dat_Stamp);

  // now we have to copy the patternList as well
  NewList((struct List *)&dstSearch->patternList);
  for(curNode = srcSearch->patternList.mlh_Head; curNode->mln_Succ; curNode = curNode->mln_Succ)
  {
    struct SearchPatternNode *srcNode = (struct SearchPatternNode *)curNode;
    struct SearchPatternNode *dstNode;

    if((dstNode = memdup(srcNode, sizeof(*srcNode))) != NULL)
      AddTail((struct List *)&dstSearch->patternList, (struct Node *)dstNode);
  }

  LEAVE();
}

///
/// FreeFilterRuleList()
void FreeFilterRuleList(struct FilterNode *filter)
{
  ENTER();

  if(IsMinListEmpty(&filter->ruleList) == FALSE)
  {
    struct MinNode *curNode;

    // we do have to iterate through our ruleList and
    // free them as well
    while((curNode = (struct MinNode *)RemHead((struct List *)&filter->ruleList)) != NULL)
    {
      struct RuleNode *rule = (struct RuleNode *)curNode;

      // now we do free our search structure if it exists
      if(rule->search != NULL)
      {
        FreeSearchPatternList(rule->search);
        free(rule->search);
      }

      free(rule);
    }
  }

  // initialize the ruleList as well
  NewList((struct List *)&filter->ruleList);

  LEAVE();
}

///
/// CreateNewFilter()
//  Initializes a new filter
struct FilterNode *CreateNewFilter(void)
{
  struct FilterNode *filter;

  ENTER();

  if((filter = calloc(1, sizeof(struct FilterNode))) != NULL)
  {
    strlcpy(filter->name, tr(MSG_NewEntry), sizeof(filter->name));
    filter->applyToNew = TRUE;
    filter->applyOnReq = TRUE;

    // initialize the rule list
    NewList((struct List *)&filter->ruleList);

    // and fill in the first rule as a filter can't have less than 1 rule
    // anyway
    if(CreateNewRule(filter) == NULL)
    {
      // creating the default rule failed, so we let this operation fail, too
      free(filter);
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
  free(filter);

  LEAVE();
}

///
/// FreeFilterList
// frees a complete filter list with all embedded filters
void FreeFilterList(struct MinList *filterList)
{
  ENTER();

  if(IsMinListEmpty(filterList) == FALSE)
  {
    struct MinNode *curNode;

    // we have to free the filterList
    while((curNode = (struct MinNode *)RemHead((struct List *)filterList)) != NULL)
    {
      struct FilterNode *filter = (struct FilterNode *)curNode;

      FreeFilterNode(filter);
    }

    NewList((struct List *)filterList);
  }

  LEAVE();
}

///
/// CreateNewRule()
//  Initializes a new filter rule
struct RuleNode *CreateNewRule(struct FilterNode *filter)
{
  struct RuleNode *rule;

  ENTER();

  if((rule = calloc(1, sizeof(struct RuleNode))) != NULL)
  {
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
/// GetFilterRule()
//  return a pointer to the rule depending on the position in the ruleList
struct RuleNode *GetFilterRule(struct FilterNode *filter, int pos)
{
  struct RuleNode *rule = NULL;

  ENTER();

  if(IsMinListEmpty(&filter->ruleList) == FALSE)
  {
    // we do have to iterate through the ruleList of the filter
    // and count for rule at position 'pos'
    struct MinNode *curNode;
    int i;

    i = 0;
    for(curNode = filter->ruleList.mlh_Head; curNode->mln_Succ; curNode = curNode->mln_Succ)
    {
      if(i == pos)
      {
        rule = (struct RuleNode *)curNode;
        break;
      }
      i++;
    }
  }

  RETURN(rule);
  return rule;
}

///

/*** GUI ***/
/// InitFilterPopupList
//  Creates a popup list of configured filters
HOOKPROTONHNP(InitFilterPopupList, ULONG, Object *pop)
{
  struct MinNode *curNode;

  // clear the popup list first
  DoMethod(pop, MUIM_List_Clear);

  // Now we process the read header to set all flags accordingly
  for(curNode = C->filterList.mlh_Head; curNode->mln_Succ; curNode = curNode->mln_Succ)
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
   struct FI_ClassData *data = calloc(1, sizeof(struct FI_ClassData));
   if (data)
   {
      Object *bt_search;
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
               Child, VGroup, GroupFrameT(tr(MSG_FI_FindIn)),
                  Child, ListviewObject,
                     MUIA_Listview_Input, TRUE,
                     MUIA_Listview_MultiSelect, TRUE,
                     MUIA_CycleChain      , 1,
                     MUIA_Listview_List, data->GUI.LV_FOLDERS = ListObject,
                        InputListFrame,
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
            Child, BalanceObject,
            End,
            Child, VGroup, GroupFrameT(tr(MSG_FI_Results)),
               MUIA_Weight, 300,
               Child, NListviewObject,
                  MUIA_CycleChain, TRUE,
                  MUIA_NListview_NList, data->GUI.LV_MAILS = MainMailListObject,
                     MUIA_NList_Format, "COL=8 W=-1 BAR, COL=1 MICW=20 BAR,COL=3 MICW=16 BAR,COL=4 MICW=9 MACW=15 BAR,COL=7 MICW=9 MACW=15 BAR,COL=5 W=-1 MACW=9 P=\33r BAR",
                     MUIA_ContextMenu,  NULL,
                  End,
               End,
            End,
            Child, data->GUI.GR_PAGE = PageGroup,
               Child, HGroup,
                  Child, bt_search = MakeButton(tr(MSG_FI_StartSearch)),
                  Child, data->GUI.BT_SELECTACTIVE = MakeButton(tr(MSG_FI_SELECTACTIVE)),
                  Child, data->GUI.BT_SELECT = MakeButton(tr(MSG_FI_SelectMatched)),
                  Child, data->GUI.BT_READ = MakeButton(tr(MSG_FI_ReadMessage)),
               End,
               Child, HGroup,
                  Child, data->GUI.GA_PROGRESS = GaugeObject,
                     MUIA_HorizWeight, 200,
                     GaugeFrame,
                     MUIA_Gauge_Horiz   ,TRUE,
                  End,
                  Child, bt_abort = MakeButton(tr(MSG_FI_Abort)),
               End,
            End,
         End,
      End;
      if (data->GUI.WI)
      {
         set(data->GUI.BT_SELECTACTIVE, MUIA_Disabled, TRUE);
         set(data->GUI.BT_SELECT,       MUIA_Disabled, TRUE);
         set(data->GUI.BT_READ,         MUIA_Disabled, TRUE);

         SetHelp(data->GUI.LV_FOLDERS,       MSG_HELP_FI_LV_FOLDERS);
         SetHelp(bt_all,                     MSG_HELP_FI_BT_ALL);
         SetHelp(po_fromrule,                MSG_HELP_FI_PO_FROMRULE);
         SetHelp(bt_torule,                  MSG_HELP_FI_BT_TORULE);
         SetHelp(bt_search,                  MSG_HELP_FI_BT_SEARCH);
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
         DoMethod(bt_search,                MUIM_Notify, MUIA_Pressed,              FALSE,          MUIV_Notify_Application, 2, MUIM_CallHook, &FI_SearchHook);
         DoMethod(data->GUI.BT_SELECT,      MUIM_Notify, MUIA_Pressed,              FALSE,          MUIV_Notify_Application, 2, MUIM_CallHook, &FI_SelectHook);
         DoMethod(data->GUI.BT_SELECTACTIVE,MUIM_Notify, MUIA_Pressed,              FALSE,          MUIV_Notify_Application, 2, MUIM_CallHook, &FI_SwitchHook);
         DoMethod(data->GUI.LV_MAILS,       MUIM_Notify, MUIA_NList_DoubleClick,    MUIV_EveryTime, MUIV_Notify_Application, 2, MUIM_CallHook, &FI_ReadHook);
         DoMethod(data->GUI.BT_READ,        MUIM_Notify, MUIA_Pressed,              FALSE,          MUIV_Notify_Application, 2, MUIM_CallHook, &FI_ReadHook);
         DoMethod(data->GUI.WI,             MUIM_Notify, MUIA_Window_CloseRequest,  TRUE,           MUIV_Notify_Application, 2, MUIM_CallHook, &FI_CloseHook);

         // Lets have the Listview sorted by Reverse Date by default
         set(data->GUI.LV_MAILS, MUIA_NList_SortType, (4 | MUIV_NList_SortTypeAdd_2Values));

         return data;
      }
      free(data);
   }
   return NULL;
}
///

