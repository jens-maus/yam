/***************************************************************************

 YAM - Yet Another Mailer
 Copyright (C) 1995-2000 by Marcel Beck <mbeck@yam.ch>
 Copyright (C) 2000-2005 by YAM Open Source Team

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
#include <proto/intuition.h>
#include <proto/muimaster.h>
#include <proto/timer.h>
#include <proto/utility.h>

#include "extra.h"
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
#include "classes/Classes.h"

#include "Debug.h"

/* local protos */
static BOOL FI_MatchString(struct Search*, char*);
static BOOL FI_MatchListPattern(struct Search*, char*);
static BOOL FI_MatchPerson(struct Search*, struct Person*);
static BOOL FI_SearchPatternFast(struct Search*, struct Mail*);
static BOOL FI_SearchPatternInBody(struct Search*, struct Mail*);
static BOOL FI_SearchPatternInHeader(struct Search*, struct Mail*);
static enum FastSearch FI_IsFastSearch(char*);
static void FI_GenerateListPatterns(struct Search*);
static BOOL FI_DoSearch(struct Search*, struct Mail*);
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
const char mailStatusCycleMap[10] = { 'U', 'O', 'F', 'R', 'W', 'E', 'H', 'S', 'M', '\0' };

///
/// FI_MatchString
//  Matches string against pattern
static BOOL FI_MatchString(struct Search *search, char *string)
{
   switch (search->Compare)
   {
      case 0: return (BOOL)(search->CaseSens ? MatchPattern(search->Pattern, string) : MatchPatternNoCase(search->Pattern, string));
      case 1: return (BOOL)(search->CaseSens ? !MatchPattern(search->Pattern, string) : !MatchPatternNoCase(search->Pattern, string));
      case 2: return (BOOL)(search->CaseSens ? strcmp(string, search->Match) < 0 : Stricmp(string, search->Match) < 0);
      case 3: return (BOOL)(search->CaseSens ? strcmp(string, search->Match) > 0 : Stricmp(string, search->Match) > 0);
   }

   return FALSE;
}

///
/// FI_MatchListPattern
//  Matches string against a list of patterns
static BOOL FI_MatchListPattern(struct Search *search, char *string)
{
  struct MinList *patternList = &search->patternList;
  struct MinNode *curNode;

  if(IsMinListEmpty(patternList) == TRUE)
    return FALSE;

  // Now we process the read header to set all flags accordingly
  for(curNode = patternList->mlh_Head; curNode->mln_Succ; curNode = curNode->mln_Succ)
  {
    struct SearchPatternNode *patternNode = (struct SearchPatternNode *)curNode;

    if(search->CaseSens ? MatchPattern(patternNode->pattern, string)
                        : MatchPatternNoCase(patternNode->pattern, string)) return TRUE;
  }

  return FALSE;
}
///
/// FI_MatchPerson
//  Matches string against a person's name or address
static BOOL FI_MatchPerson(struct Search *search, struct Person *pe)
{
   if (search->Compare == 4) return FI_MatchListPattern(search, search->PersMode ? pe->RealName : pe->Address);
                        else return FI_MatchString(search, search->PersMode ? pe->RealName : pe->Address);
}
///
/// FI_SearchPatternFast
//  Searches string in standard header fields
static BOOL FI_SearchPatternFast(struct Search *search, struct Mail *mail)
{
   struct ExtendedMail *email;
   int j;
   BOOL found = FALSE;

   switch (search->Fast)
   {
      case FS_FROM:
         if (FI_MatchPerson(search, &mail->From)) found = TRUE;
         break;
      case FS_TO:
         if (FI_MatchPerson(search, &mail->To)) found = TRUE;
         if(isMultiRCPTMail(mail) && (email = MA_ExamineMail(mail->Folder, mail->MailFile, TRUE)))
         {
            for (j = 0; j < email->NoSTo; j++) if (FI_MatchPerson(search, &email->STo[j])) found = TRUE;
            MA_FreeEMailStruct(email);
         }
         break;
      case FS_CC:
         if(isMultiRCPTMail(mail) && (email = MA_ExamineMail(mail->Folder, mail->MailFile, TRUE)))
         {
            for (j = 0; j < email->NoCC; j++)  if (FI_MatchPerson(search, &email->CC[j])) found = TRUE;
            MA_FreeEMailStruct(email);
         }
         break;
      case FS_REPLYTO:
         if (FI_MatchPerson(search, &mail->ReplyTo)) found = TRUE;
        break;
      case FS_SUBJECT:
         if ((search->Compare == 4) ? FI_MatchListPattern(search, mail->Subject) : FI_MatchString(search, mail->Subject)) found = TRUE;
         break;
      case FS_DATE:
         {
            struct DateStamp *pdat = (struct DateStamp *)search->Pattern;
            int cmp = (pdat->ds_Minute) ? CompareDates(&(mail->Date), pdat) : pdat->ds_Days-mail->Date.ds_Days;
            if ((search->Compare == 0 && cmp == 0) ||
                (search->Compare == 1 && cmp != 0) ||
                (search->Compare == 2 && cmp > 0) ||
                (search->Compare == 3 && cmp < 0)) found = TRUE;
            break;
         }
      case FS_SIZE:
         {
            long cmp = search->Size - mail->Size;
            if ((search->Compare == 0 && cmp == 0) ||
                (search->Compare == 1 && cmp != 0) ||
                (search->Compare == 2 && cmp > 0) ||
                (search->Compare == 3 && cmp < 0)) found = TRUE;
            break;
         }

      default:
        // nothing
      break;
   }
   return found;
}

///
/// FI_SearchPatternInBody
//  Searches string in message body
static BOOL FI_SearchPatternInBody(struct Search *search, struct Mail *mail)
{
  BOOL found = FALSE;
  struct ReadMailData *rmData;

  if((rmData = AllocPrivateRMData(mail, PM_TEXTS)))
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

  return found;
}

///
/// FI_SearchPatternInHeader
//  Searches string in header field(s)
static BOOL FI_SearchPatternInHeader(struct Search *search, struct Mail *mail)
{
  char fullfile[SIZE_PATHFILE];
  BOOL found = FALSE;
  FILE *fh;

  if(StartUnpack(GetMailFile(NULL, mail->Folder, mail), fullfile, mail->Folder))
  {
    if((fh = fopen(fullfile, "r")))
    {
      struct MinList *headerList = calloc(1, sizeof(struct MinList));

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
          {
            if(FI_MatchListPattern(search, hdrNode->content))
              found = TRUE;
          }
          else if(FI_MatchString(search, hdrNode->content))
            found = TRUE;
        }

        // free our temporary header list
        FreeHeaderList(headerList);
      }

      // free our temporary headerList
      free(headerList);

      // close the file
      fclose(fh);
    }

    FinishUnpack(fullfile);
  }

  return found;
}

///
/// FI_IsFastSearch
//  Checks if quick search is available for selected header field
static enum FastSearch FI_IsFastSearch(char *field)
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
   char pattern[SIZE_PATTERN*2+2]; // ParsePattern() needs at least 2*source+2 bytes buffer
   char buf[SIZE_PATTERN];
   FILE *fh;

   if((fh = fopen(search->Match, "r")))
   {
      // make sure the pattern list is successfully freed
      FreeSearchPatternList(search);

      while(GetLine(fh, buf, sizeof(buf)))
      {
         if(buf[0])
         {
            struct SearchPatternNode *newNode;

            if (search->CaseSens) ParsePattern      (buf, pattern, sizeof(pattern));
            else                  ParsePatternNoCase(buf, pattern, sizeof(pattern));

            // put the pattern in our search pattern list
            newNode = malloc(sizeof(struct SearchPatternNode));
            if(newNode != NULL)
            {
              strncpy(newNode->pattern, pattern, sizeof(newNode->pattern));
              newNode->pattern[sizeof(newNode->pattern)] = '\0';

              // add the pattern to our list
              AddTail((struct List *)&search->patternList, (struct Node *)newNode);
            }
         }
      }
      fclose(fh);
   }
}

///
/// FI_PrepareSearch
//  Initializes Search structure
BOOL FI_PrepareSearch(struct Search *search, enum SearchMode mode,
                      BOOL casesens, int persmode, int compar,
                      char stat, BOOL substr, char *match, char *field)
{
   // return value of this function isn't used currently (21.03.2001)
   memset(search, 0, sizeof(struct Search));
   search->Mode      = mode;
   search->CaseSens  = casesens;
   search->PersMode  = persmode;
   search->Compare   = compar;
   search->Status    = stat;
   search->SubString = substr;
   stccpy(search->Match, match, SIZE_PATTERN);
   stccpy(search->Field, field, SIZE_DEFAULT);
   search->Pattern = search->PatBuf;
   search->Fast = FS_NONE;
   NewList((struct List *)&search->patternList);

   switch(mode)
   {
      case SM_FROM:     search->Fast = FS_FROM; break;
      case SM_TO:       search->Fast = FS_TO; break;
      case SM_CC:       search->Fast = FS_CC; break;
      case SM_REPLYTO:  search->Fast = FS_REPLYTO; break;
      case SM_SUBJECT:  search->Fast = FS_SUBJECT; break;

      case SM_DATE:
      {
        char *time;
        search->Fast = FS_DATE;
        search->DT.dat_Format = FORMAT_DEF;
        search->DT.dat_StrDate = match;
        search->DT.dat_StrTime = (time = strchr(match,' ')) ? time+1 : "00:00:00";

        if(!StrToDate(&(search->DT)))
        {
          char datstr[64];
          DateStamp2String(datstr, NULL, DSS_DATE, TZC_NONE);
          ER_NewError(GetStr(MSG_ER_ErrorDateFormat), datstr, NULL);

          return FALSE;
        }

        search->Pattern = (char *)&(search->DT.dat_Stamp);
      }
      break;

      case SM_HEADLINE: search->Fast = FI_IsFastSearch(field); break;

      case SM_SIZE:
      {
        search->Fast = FS_SIZE;
        search->Size = atol(match);
      }
      break;

      case SM_HEADER:   // continue
      case SM_WHOLE:    *search->Field = 0;

      default:
        // nothing
      break;
   }

   if(compar == 4)
     FI_GenerateListPatterns(search);
   else if (search->Fast != FS_DATE && search->Fast != FS_SIZE && mode != SM_SIZE)
   {
      if (substr || mode == SM_HEADER || mode == SM_BODY || mode == SM_WHOLE || mode == SM_STATUS)
      {
         char buffer[SIZE_PATTERN+1];

         // if substring is selected lets generate a substring out
         // of the current match string, but keep the string borders in mind.
         strncpy(buffer, search->Match, SIZE_PATTERN);
         buffer[SIZE_PATTERN] = '\0';
         sprintf(search->Match, "#?%s#?", buffer);
      }

      if (casesens) ParsePattern      (search->Match, search->Pattern, (SIZE_PATTERN+4)*2+2);
      else          ParsePatternNoCase(search->Match, search->Pattern, (SIZE_PATTERN+4)*2+2);
   }

   return FALSE;
}

///
/// FI_DoSearch
//  Checks if a message fulfills the search criteria
static BOOL FI_DoSearch(struct Search *search, struct Mail *mail)
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
        // check wheter this is a fast search or not.
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
        }

        if((search->Compare == 0 && statusFound == TRUE) ||
           (search->Compare == 1 && statusFound == FALSE))
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
  int i;
  struct MinNode *curNode;
  BOOL lastCond = FALSE;

  // we have to iterate through our ruleList and depending on the combine
  // operation we evaluate if the filter hits any mail criteria or not.
  for(i=0, curNode = filter->ruleList.mlh_Head; curNode->mln_Succ; curNode = curNode->mln_Succ, i++)
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
  }

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
   struct timeval now;
   struct timeval last;

   // by default we don`t dispose on end
   G->FI->DisposeOnEnd = FALSE;
   G->FI->SearchActive = TRUE;
   G->FI->Abort        = FALSE;

   set(gui->WI, MUIA_Window_ActiveObject, MUIV_Window_ActiveObject_None);
   set(gui->BT_SELECT, MUIA_Disabled, TRUE);
   set(gui->BT_READ, MUIA_Disabled, TRUE);
   DoMethod(gui->LV_MAILS, MUIM_NList_Clear);
   fnr = xget(gui->LV_FOLDERS, MUIA_List_Entries);
   sfo = calloc(fnr, sizeof(struct Folder *));
   id = MUIV_List_NextSelected_Start;
   while (TRUE)
   {
      DoMethod(gui->LV_FOLDERS, MUIM_List_NextSelected, &id);
      if (id == MUIV_List_NextSelected_End) break;
      DoMethod(gui->LV_FOLDERS, MUIM_List_GetEntry, id, &name);
      if ((folder = FO_GetFolderByName(name, NULL))) if (MA_GetIndex(folder))
      {
         sfo[sfonum++] = folder;
         totmsg += folder->Total;
      }
   }

   if(!totmsg)
   {
     free(sfo);
     return;
   }

   // lets prepare the search
   DoMethod(gui->GR_SEARCH, MUIM_SearchControlGroup_PrepareSearch, &search);

   SPrintF(gauge, GetStr(MSG_FI_GaugeText), totmsg);

   SetAttrs(ga, MUIA_Gauge_InfoText, gauge,
                MUIA_Gauge_Max,      totmsg,
                MUIA_Gauge_Current,  0,
                TAG_DONE);

   set(gui->GR_PAGE, MUIA_Group_ActivePage, 1);

   memset(&last, 0, sizeof(struct timeval));

   for (i = 0; i < sfonum && !G->FI->Abort; i++)
   {
      for (mail = sfo[i]->Messages; mail && !G->FI->Abort; mail = mail->Next)
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
         GetSysTime(&now);
         if(-CmpTime(&now, &last) > 0)
         {
            struct timeval delta;

            // how much time has passed exactly?
            memcpy(&delta, &now, sizeof(struct timeval));
            SubTime(&delta, &last);

            // update the display at least twice a second
            if(delta.tv_secs > 0 || delta.tv_micro > 250000)
            {
              set(ga, MUIA_Gauge_Current, progress);

              // signal the application to update now
              DoMethod(G->App, MUIM_Application_InputBuffered);

              memcpy(&last, &now, sizeof(struct timeval));
            }

         }
      }
   }

   // to let the gauge move to 100% lets increase it accordingly.
   set(ga, MUIA_Gauge_Current, progress);

   // signal the application to update now
   DoMethod(G->App, MUIM_Application_InputBuffered);

   FreeSearchPatternList(&search);
   free(sfo);
   set(gui->GR_PAGE, MUIA_Group_ActivePage, 0);
   set(gui->BT_SELECT, MUIA_Disabled, !fndmsg);
   set(gui->BT_READ, MUIA_Disabled, !fndmsg);

   G->FI->SearchActive = FALSE;

   // if the closeHook has set the DisposeOnEnd flag we have to dispose
   // our object now.
   if(G->FI->DisposeOnEnd) DisposeModulePush(&G->FI);
}
MakeStaticHook(FI_SearchHook,FI_SearchFunc);

///
/// CreateFilterFromSearch
//  Creates a filter from the current search options
HOOKPROTONHNONP(CreateFilterFromSearch, void)
{
  int ch;
  char name[SIZE_NAME];
  *name = '\0';

  // request a name for that new filter from the user
  if((ch = StringRequest(name, SIZE_NAME,
                               GetStr(MSG_FI_AddFilter),
                               GetStr(MSG_FI_AddFilterReq),
                               GetStr(MSG_Save),
                               GetStr(MSG_Use),
                               GetStr(MSG_Cancel), FALSE,
                               G->FI->GUI.WI)))
  {
    struct FilterNode *filter;

    if((filter = CreateNewFilter()))
    {
      struct RuleNode *rule = GetFilterRule(filter, 0);

      strcpy(filter->name, name);

      if(rule)
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
   int i, j, apos = 0;
   struct Folder **flist, *folder;

   if (!G->FI)
   {
      if (!(G->FI = FI_New())) return;
      folder = FO_GetCurrentFolder();
      if(!folder) return;
      flist = FO_CreateList();
      for (j = 0, i = 1; i <= (int)*flist; i++) if (flist[i]->Type != FT_GROUP)
      {
         DoMethod(G->FI->GUI.LV_FOLDERS, MUIM_List_InsertSingle, flist[i]->Name, MUIV_List_Insert_Bottom);
         if (flist[i] == folder) apos = j;
         j++;
      }
      set(G->FI->GUI.LV_FOLDERS, MUIA_List_Active, apos);
      free(flist);
   }
   if (!SafeOpenWindow(G->FI->GUI.WI))
   {
     DisposeModulePush(&G->FI);
   }
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
      int i;

      MA_ChangeFolder(foundmail->Folder, TRUE);

      // now get the position of the foundmail in the real mail list and set it active
      // this is faster than using GetMailInfo()
      for(i=0; ;i++)
      {
        struct Mail *mail;

        // get the real mail out of the mail list
        DoMethod(G->MA->GUI.NL_MAILS, MUIM_NList_GetEntry, i, &mail);
        if(!mail) break;

        // if both are the same we set the mail list now.
        if(mail == foundmail)
        {
          // Because the NList for the maillistview and the NList for the find listview
          // are sharing the same displayhook we have to call MUIA_NList_DisplayRecall
          // twice here to that it will recognize that something has changed or
          // otherwise both NLists will show glitches.
          SetAttrs(G->MA->GUI.NL_MAILS,  MUIA_NList_DisplayRecall, TRUE,
                                         MUIA_NList_Active, i,
                                         TAG_DONE
                  );
          set(G->FI->GUI.LV_MAILS, MUIA_NList_DisplayRecall, TRUE);

          break;
        }
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

   DoMethod(G->MA->GUI.NL_MAILS, MUIM_NList_Select, MUIV_NList_Select_All, MUIV_NList_Select_Off, NULL);

   for(i=0; ;i++)
   {
      struct Mail *foundmail;

      DoMethod(G->FI->GUI.LV_MAILS, MUIM_NList_GetEntry, i, &foundmail);
      if (!foundmail) break;

      // only if the current folder is the same as this messages resists in
      if(foundmail->Folder == folder)
      {
        int j;

        // now get the position of the foundmail in the real mail list and select it
        // this is faster than using GetMailInfo()
        for(j=0; ;j++)
        {
          struct Mail *mail;

          // get the real mail out of the mail list
          DoMethod(G->MA->GUI.NL_MAILS, MUIM_NList_GetEntry, j, &mail);
          if (!mail) break;

          // if both are the same we set the mail list now.
          if(mail == foundmail)
          {
            DoMethod(G->MA->GUI.NL_MAILS, MUIM_NList_Select, j, MUIV_NList_Select_On, NULL);
            break;
          }
        }
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
  struct MinNode *curNode;

  if(IsMinListEmpty(patternList) == TRUE)
    return;

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

///
/// AllocFilterSearch()
//  Allocates and initializes search structures for filters and returns
//  the number of active filters/search structures allocated.
int AllocFilterSearch(enum ApplyFilterMode mode)
{
  int active=0;
  struct MinNode *curNode;

  // iterate through our filter List
  for(curNode = C->filterList.mlh_Head; curNode->mln_Succ; curNode = curNode->mln_Succ)
  {
    struct FilterNode *filter = (struct FilterNode *)curNode;
    struct MinNode *curRuleNode;

    // lets check if we can skip some filters because of the ApplyMode
    // and filter relation.
    if((mode == APPLY_AUTO && (!filter->applyToNew || filter->remote)) ||
       (mode == APPLY_USER && (!filter->applyOnReq || filter->remote)) ||
       (mode == APPLY_SENT && (!filter->applyToSent || filter->remote)) ||
       (mode == APPLY_REMOTE && !filter->remote))
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
    else
    {
      // check if the search structures are already allocated or not
      for(curRuleNode = filter->ruleList.mlh_Head; curRuleNode->mln_Succ; curRuleNode = curRuleNode->mln_Succ)
      {
        struct RuleNode *rule = (struct RuleNode *)curRuleNode;

        // check if that search structure already exists or not
        if(rule->search == NULL &&
           (rule->search = calloc(1, sizeof(struct Search))))
        {
          int stat = 9;

          // we check the status field first and if we find a match
          // we can immediatly break up here because we don`t need to prepare the search
          if(rule->searchMode == SM_STATUS)
          {
            for(stat=0; stat <= 8; stat++)
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

  return active;
}

///
/// FreeFilterSearch()
//  Frees filter search structures
void FreeFilterSearch(void)
{
  struct MinNode *curNode;

  // Now we process the read header to set all flags accordingly
  for(curNode = C->filterList.mlh_Head; curNode->mln_Succ; curNode = curNode->mln_Succ)
  {
    struct FilterNode *filter = (struct FilterNode *)curNode;
    struct MinNode *curRuleNode;

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

///
/// ExecuteFilterAction()
//  Applies filter action to a message and return TRUE if the filter search
//  should continue or FALSE if it should stop afterwards
BOOL ExecuteFilterAction(struct FilterNode *filter, struct Mail *mail)
{
  struct Mail *mlist[3];
  struct Folder* fo;

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
    sprintf(buf, "%s \"%s\"", filter->executeCmd, GetRealPath(GetMailFile(NULL, NULL, mail)));
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
    if((fo = FO_GetFolderByName(filter->moveTo, NULL)))
    {
      if(mail->Folder != fo)
      {
        G->RRs.Moved++;

        if(fo->LoadedMode != LM_VALID && isProtectedFolder(fo))
          SET_FLAG(fo->Flags, FOFL_FREEXS);

        MA_MoveCopy(mail, mail->Folder, fo, FALSE);

        return FALSE;
      }
    }
    else
      ER_NewError(GetStr(MSG_ER_CANTMOVEMAIL), mail->MailFile, filter->moveTo);
  }

  // Delete Action
  if(hasDeleteAction(filter))
  {
    G->RRs.Deleted++;

    if(isSendMDNMail(mail) &&
       (hasStatusNew(mail) || !hasStatusRead(mail)))
    {
      RE_DoMDN(MDN_DELE|MDN_AUTOACT, mail, FALSE);
    }

    MA_DeleteSingle(mail, FALSE, FALSE);

    return FALSE;
  }

  return TRUE;
}

///
/// ApplyFiltersFunc()
//  Apply filters
HOOKPROTONHNO(ApplyFiltersFunc, void, int *arg)
{
  struct Mail *mail;
  struct Mail **mlist = NULL;
  struct Folder *folder;
  int scnt;
  int matches = 0;
  int minselected = hasFlag(arg[1], (IEQUALIFIER_LSHIFT|IEQUALIFIER_RSHIFT)) ? 1 : 2;
  enum ApplyFilterMode mode = arg[0];
  APTR lv = G->MA->GUI.NL_MAILS;
  char buf[SIZE_LARGE];

  folder = (mode == APPLY_AUTO) ? FO_GetFolderByType(FT_INCOMING, NULL) : FO_GetCurrentFolder();
  if(!folder)
    return;

  // if this function was called manually by the user we ask him
  // if he really wants to apply the filters or not.
  if(mode == APPLY_USER)
  {
    sprintf(buf, GetStr(MSG_MA_ConfirmFilter), folder->Name);
    if(!MUI_Request(G->App, G->MA->GUI.WI, 0, GetStr(MSG_MA_ConfirmReq), GetStr(MSG_YesNoReq), buf))
      return;
  }

  memset(&G->RRs, 0, sizeof(struct RuleResult));
  set(lv, MUIA_NList_Quiet, TRUE);
  G->AppIconQuiet = TRUE;

  if((scnt = AllocFilterSearch(mode)))
  {
    if(mode == APPLY_USER || mode == APPLY_RX || mode == APPLY_RX_ALL)
    {
      if((mlist = MA_CreateMarkedList(lv, mode == APPLY_RX)) && (int)mlist[0] < minselected)
      {
        free(mlist);
        mlist = NULL;
      }
    }

    if(!mlist)
      mlist = MA_CreateFullList(folder, (mode == APPLY_AUTO || mode == APPLY_RX));

    if(mlist)
    {
      int m;

      BusyGauge(GetStr(MSG_BusyFiltering), "", (int)*mlist);
      for(m = 0; m < (int)*mlist; m++)
      {
        struct MinNode *curNode;

        mail = mlist[m+2];

        if((mode == APPLY_AUTO || mode == APPLY_RX) && !hasStatusNew(mail))
          continue;

        G->RRs.Checked++;

        // now we process the search
        for(curNode = C->filterList.mlh_Head; curNode->mln_Succ; curNode = curNode->mln_Succ)
        {
          struct FilterNode *filter = (struct FilterNode *)curNode;

          if(DoFilterSearch(filter, mail))
          {
            matches++;

            // if ExecuteFilterAction return FALSE then the filter search should be aborted
            // completley
            if(ExecuteFilterAction(filter, mail) == FALSE)
              break;
          }
        }

        BusySet(m+1);
      }
      free(mlist);

      if(G->RRs.Moved)
        MA_FlushIndexes(FALSE);

      if(G->RRs.Checked)
        AppendLog(26, GetStr(MSG_LOG_Filtering), (void *)(G->RRs.Checked), folder->Name, (void *)matches, "");

      BusyEnd();
    }

    FreeFilterSearch();
  }

  set(lv, MUIA_NList_Quiet, FALSE);
  G->AppIconQuiet = FALSE;

  if(mode != APPLY_AUTO)
    DisplayStatistics(NULL, TRUE);

  if(G->RRs.Checked && mode == APPLY_USER)
  {
    sprintf(buf, GetStr(MSG_MA_FilterStats), G->RRs.Checked, G->RRs.Forwarded, G->RRs.Moved, G->RRs.Deleted);
    MUI_Request(G->App, G->MA->GUI.WI, 0, NULL, GetStr(MSG_OkayReq), buf);
  }
}
MakeHook(ApplyFiltersHook, ApplyFiltersFunc);

///
/// CopyFilterData()
// copy all data of a filter node (deep copy)
void CopyFilterData(struct FilterNode *dstFilter, struct FilterNode *srcFilter)
{
  struct MinNode *curNode;

  ENTER();
  SHOWSTRING(DBF_FILTER, srcFilter->name);

  // raw copy all global stuff first
  memcpy(dstFilter, srcFilter, sizeof(struct FilterNode));

  // then iterate through our ruleList and copy it as well
  NewList((struct List *)&dstFilter->ruleList);
  for(curNode = srcFilter->ruleList.mlh_Head; curNode->mln_Succ; curNode = curNode->mln_Succ)
  {
    struct RuleNode *rule = (struct RuleNode *)curNode;
    struct RuleNode *newRule = calloc(1, sizeof(struct RuleNode));

    // do a raw copy of the rule contents first
    memcpy(newRule, rule, sizeof(struct RuleNode));

    // check if the search structure exists and if so
    // so start another deep copy
    if(rule->search)
    {
      newRule->search = calloc(1, sizeof(struct Search));
      CopySearchData(newRule->search, rule->search);
    }
    else
      newRule->search = NULL;

    // add the rule to the ruleList of our desitionation filter
    AddTail((struct List *)&dstFilter->ruleList, (struct Node *)newRule);
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
    struct SearchPatternNode *dstNode = calloc(1, sizeof(struct SearchPatternNode));

    memcpy(dstNode, srcNode, sizeof(struct SearchPatternNode));

    AddTail((struct List *)&dstSearch->patternList, (struct Node *)dstNode);
  }

  LEAVE();
}

///
/// FreeFilterRuleList()
void FreeFilterRuleList(struct FilterNode *filter)
{
  struct MinNode *curNode;

  // we do have to iterate through our ruleList and
  // free them as well
  for(curNode = filter->ruleList.mlh_Head; curNode->mln_Succ;)
  {
    struct RuleNode *rule = (struct RuleNode *)curNode;

    // now we do free our search structure if it exists
    if(rule->search)
    {
      FreeSearchPatternList(rule->search);
      free(rule->search);
      rule->search = NULL;
    }

    // before we remove the node we have to save the pointer to the next one
    curNode = curNode->mln_Succ;

    // Remove node from list
    Remove((struct Node *)rule);
    free(rule);
  }

  // initialize the ruleList as well
  NewList((struct List *)&filter->ruleList);
}

///
/// CreateNewFilter()
//  Initializes a new filter
struct FilterNode *CreateNewFilter(void)
{
  struct FilterNode *filter = calloc(1, sizeof(struct FilterNode));

  if(filter)
  {
    strcpy(filter->name, GetStr(MSG_NewEntry));
    filter->applyToNew = TRUE;
    filter->applyOnReq = TRUE;

    // initialize the rule list
    NewList((struct List *)&filter->ruleList);

    // and fill in the first rule as a filter can't have less than 1 rule
    // anyway
    CreateNewRule(filter);
  }

  return filter;
}

///
/// CreateNewRule()
//  Initializes a new filter rule
struct RuleNode *CreateNewRule(struct FilterNode *filter)
{
  struct RuleNode *rule = calloc(1, sizeof(struct RuleNode));

  ENTER();
  SHOWSTRING(DBF_FILTER, filter->name);

  if(rule)
  {
    // if a filter was specified we immediatley add this new rule to it
    if(filter)
      AddTail((struct List *)&filter->ruleList, (struct Node *)rule);
  }

  RETURN(rule);
  return rule;
}

///
/// GetFilterRule()
//  return a pointer to the rule depending on the position in the ruleList
struct RuleNode *GetFilterRule(struct FilterNode *filter, int pos)
{
  int i;
  struct MinNode *curNode = NULL;

  // we do have to iterate through the ruleList of the filter
  // and count for rule at position 'pos'
  for(i=0, curNode = filter->ruleList.mlh_Head; curNode->mln_Succ; curNode = curNode->mln_Succ, i++)
  {
    if(i == pos)
      return (struct RuleNode *)curNode;
  }

  return NULL;
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

  if(filter)
  {
    struct RuleNode *rule = GetFilterRule(filter, 0);

    if(rule)
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
      APTR bt_search, bt_abort, lv_fromrule, bt_torule, po_fromrule, bt_all;
      data->GUI.WI = WindowObject,
         MUIA_Window_Title, GetStr(MSG_FI_FindMessages),
         MUIA_HelpNode, "FI_W",
         MUIA_Window_ID, MAKE_ID('F','I','N','D'),
         WindowContents, VGroup,
            Child, HGroup,
               Child, VGroup, GroupFrameT(GetStr(MSG_FI_FindIn)),
                  Child, ListviewObject,
                     MUIA_Listview_Input, TRUE,
                     MUIA_Listview_MultiSelect, TRUE,
                     MUIA_CycleChain      , 1,
                     MUIA_Listview_List, data->GUI.LV_FOLDERS = ListObject,
                        InputListFrame,
                        MUIA_List_AdjustWidth, TRUE,
                     End,
                  End,
                  Child, bt_all = MakeButton(GetStr(MSG_FI_AllFolders)),
               End,
               Child, VGroup, GroupFrameT(GetStr(MSG_FI_FindWhat)),
                  Child, data->GUI.GR_SEARCH = SearchControlGroupObject,
                    MUIA_SearchControlGroup_RemoteFilterMode, FALSE,
                  End,
                  Child, ColGroup(2),
                     Child, po_fromrule = PopobjectObject,
                        MUIA_Popstring_Button, MakeButton(GetStr(MSG_FI_UseFilter)),
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
                     Child, bt_torule = MakeButton(GetStr(MSG_FI_AddAsFilter)),
                  End,
               End,
            End,
            Child, VGroup, GroupFrameT(GetStr(MSG_FI_Results)),
               Child, NListviewObject,
                  MUIA_CycleChain,1,
                  MUIA_NListview_NList, data->GUI.LV_MAILS = NListObject,
                     MUIA_NList_MinColSortable, 0,
                     MUIA_NList_TitleClick    , TRUE,
                     MUIA_NList_TitleClick2   , TRUE,
                     MUIA_NList_MultiSelect   , MUIV_NList_MultiSelect_Default,
                     MUIA_NList_CompareHook2  , &MA_LV_Cmp2Hook,
                     MUIA_NList_Format        , "COL=8 W=-1 BAR,COL=1 MICW=20 BAR,COL=3 MICW=16 BAR,COL=4 MICW=9 MACW=15 BAR,COL=7 MICW=9 MACW=15 BAR,COL=5 W=-1 MACW=9 P=\33r BAR",
                     MUIA_NList_DoubleClick   , TRUE,
                     MUIA_NList_DisplayHook2  , &MA_LV_DspFuncHook,
                     MUIA_NList_AutoVisible   , TRUE,
                     MUIA_NList_Title         , TRUE,
                     MUIA_NList_TitleSeparator, TRUE,
                     MUIA_Font, C->FixedFontList ? MUIV_NList_Font_Fixed : MUIV_NList_Font,
                  End,
               End,
            End,
            Child, data->GUI.GR_PAGE = PageGroup,
               Child, ColGroup(3),
                  Child, bt_search = MakeButton(GetStr(MSG_FI_StartSearch)),
                  Child, data->GUI.BT_SELECT = MakeButton(GetStr(MSG_FI_SelectMatched)),
                  Child, data->GUI.BT_READ = MakeButton(GetStr(MSG_FI_ReadMessage)),
               End,
               Child, HGroup,
                  Child, data->GUI.GA_PROGRESS = GaugeObject,
                     MUIA_HorizWeight, 200,
                     GaugeFrame,
                     MUIA_Gauge_Horiz   ,TRUE,
                  End,
                  Child, bt_abort = MakeButton(GetStr(MSG_FI_Abort)),
               End,
            End,
         End,
      End;
      if (data->GUI.WI)
      {
         set(data->GUI.BT_SELECT, MUIA_Disabled, TRUE);
         set(data->GUI.BT_READ, MUIA_Disabled, TRUE);
         SetHelp(data->GUI.LV_FOLDERS   ,MSG_HELP_FI_LV_FOLDERS);
         SetHelp(bt_all                 ,MSG_HELP_FI_BT_ALL);
         SetHelp(po_fromrule            ,MSG_HELP_FI_PO_FROMRULE);
         SetHelp(bt_torule              ,MSG_HELP_FI_BT_TORULE);
         SetHelp(bt_search              ,MSG_HELP_FI_BT_SEARCH);
         SetHelp(data->GUI.BT_SELECT    ,MSG_HELP_FI_BT_SELECT);
         SetHelp(data->GUI.BT_READ      ,MSG_HELP_FI_BT_READ);
         SetHelp(bt_abort               ,MSG_HELP_FI_BT_ABORT);
         DoMethod(G->App, OM_ADDMEMBER, data->GUI.WI);
         DoMethod(bt_abort           ,MUIM_Notify,MUIA_Pressed             ,FALSE         ,MUIV_Notify_Application,3,MUIM_WriteLong,TRUE,&(data->Abort));
         DoMethod(lv_fromrule        ,MUIM_Notify,MUIA_Listview_DoubleClick,TRUE          ,po_fromrule            ,2,MUIM_Popstring_Close,TRUE);
         DoMethod(bt_all             ,MUIM_Notify,MUIA_Pressed             ,FALSE         ,data->GUI.LV_FOLDERS   ,5,MUIM_List_Select,MUIV_List_Select_All,MUIV_List_Select_On,NULL);
         DoMethod(bt_torule          ,MUIM_Notify,MUIA_Pressed             ,FALSE         ,MUIV_Notify_Application,2,MUIM_CallHook,&CreateFilterFromSearchHook);
         DoMethod(bt_search          ,MUIM_Notify,MUIA_Pressed             ,FALSE         ,MUIV_Notify_Application,2,MUIM_CallHook,&FI_SearchHook);
         DoMethod(data->GUI.BT_SELECT,MUIM_Notify,MUIA_Pressed             ,FALSE         ,MUIV_Notify_Application,2,MUIM_CallHook,&FI_SelectHook);
         DoMethod(data->GUI.LV_MAILS ,MUIM_Notify,MUIA_NList_TitleClick    ,MUIV_EveryTime,MUIV_Notify_Self       ,4,MUIM_NList_Sort3,MUIV_TriggerValue,MUIV_NList_SortTypeAdd_2Values,MUIV_NList_Sort3_SortType_Both);
         DoMethod(data->GUI.LV_MAILS ,MUIM_Notify,MUIA_NList_TitleClick2   ,MUIV_EveryTime,MUIV_Notify_Self       ,4,MUIM_NList_Sort3,MUIV_TriggerValue,MUIV_NList_SortTypeAdd_2Values,MUIV_NList_Sort3_SortType_2);
         DoMethod(data->GUI.LV_MAILS ,MUIM_Notify,MUIA_NList_SortType      ,MUIV_EveryTime,MUIV_Notify_Self       ,3,MUIM_Set,MUIA_NList_TitleMark,MUIV_TriggerValue);
         DoMethod(data->GUI.LV_MAILS ,MUIM_Notify,MUIA_NList_SortType2     ,MUIV_EveryTime,MUIV_Notify_Self       ,3,MUIM_Set,MUIA_NList_TitleMark2,MUIV_TriggerValue);
         DoMethod(data->GUI.LV_MAILS ,MUIM_Notify,MUIA_NList_Active        ,MUIV_EveryTime,MUIV_Notify_Application,2,MUIM_CallHook,&FI_SwitchHook);
         DoMethod(data->GUI.LV_MAILS ,MUIM_Notify,MUIA_NList_DoubleClick   ,MUIV_EveryTime,MUIV_Notify_Application,2,MUIM_CallHook,&FI_ReadHook);
         DoMethod(data->GUI.BT_READ  ,MUIM_Notify,MUIA_Pressed             ,FALSE         ,MUIV_Notify_Application,2,MUIM_CallHook,&FI_ReadHook);
         DoMethod(data->GUI.WI       ,MUIM_Notify,MUIA_Window_CloseRequest ,TRUE          ,MUIV_Notify_Application,2,MUIM_CallHook,&FI_CloseHook);

         // Lets have the Listview sorted by Reverse Date by default
         set(data->GUI.LV_MAILS, MUIA_NList_SortType, (4 | MUIV_NList_SortTypeAdd_2Values));

         return data;
      }
      free(data);
   }
   return NULL;
}
///
