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
#include <mui/NListtree_mcc.h>
#include <mui/NListview_mcc.h>
#include <proto/dos.h>
#include <proto/exec.h>
#include <proto/intuition.h>
#include <proto/muimaster.h>
#include <proto/timer.h>
#include <proto/utility.h>

#include "extrasrc.h"

#include "SDI_hook.h"
#include "timeval.h"

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

#include "mui/ClassesExtra.h"
#include "mui/FilterPopupList.h"
#include "mui/FolderRequestListtree.h"
#include "mui/MainMailList.h"
#include "mui/ReadWindow.h"
#include "mui/SearchControlGroup.h"
#include "mui/WriteWindow.h"

#include "BayesFilter.h"
#include "BoyerMooreSearch.h"
#include "Busy.h"
#include "FolderList.h"
#include "Locale.h"
#include "Logfile.h"
#include "MailList.h"
#include "MethodStack.h"
#include "MUIObjects.h"
#include "Requesters.h"
#include "StrBuf.h"
#include "Threads.h"

#include "Debug.h"

/* local protos */
static struct FI_ClassData *FI_New(void);
static BOOL CopySearchData(struct Search *dstSearch, struct Search *srcSearch);

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
static BOOL FI_MatchString(const struct Search *search, const char *string)
{
  BOOL match = FALSE;

  ENTER();

  switch(search->Compare)
  {
    case CP_EQUAL:
    case CP_NOTEQUAL:
    case CP_INPUT:
    {
      if(isFlagSet(search->flags, SEARCHF_DOS_PATTERN))
      {
        struct Node *curNode;

        // match the string against all patterns in the list
        IterateList(&search->patternList, curNode)
        {
          struct SearchPatternNode *patternNode = (struct SearchPatternNode *)curNode;

          if(isFlagSet(search->flags, SEARCHF_CASE_SENSITIVE))
            match = (BOOL)MatchPattern(patternNode->pattern, (STRPTR)string);
          else
            match = (BOOL)MatchPatternNoCase(patternNode->pattern, (STRPTR)string);

          D(DBF_FILTER, "did DOS pattern search of pattern '%s' against '%s', result %ld", patternNode->pattern, string, match);

          if(match == TRUE)
            break;
        }
      }
      else if(isFlagSet(search->flags, SEARCHF_SUBSTRING))
      {
        match = (BOOL)(BoyerMooreSearch(search->bmContext, string) != NULL);
        D(DBF_FILTER, "did Boyer-Moore substring search of '%s' in '%s', result %ld", search->bmContext->pattern, string, match);
      }
      else
      {
        if(isFlagSet(search->flags, SEARCHF_CASE_SENSITIVE))
          match = (BOOL)(strcmp(string, search->Match) == 0);
        else
          match = (BOOL)(Stricmp(string, search->Match) == 0);
        D(DBF_FILTER, "did string comparison of '%s' against '%s', result %ld", search->Match, string, match);
	  }

      // check for non-matching search
      if(search->Compare == CP_NOTEQUAL)
        match = !match;
    }
    break;

    case CP_LOWER:
    {
      if(isFlagSet(search->flags, SEARCHF_CASE_SENSITIVE))
        match = (BOOL)(strcmp(string, search->Match) < 0);
      else
        match = (BOOL)(Stricmp(string, search->Match) < 0);
      D(DBF_FILTER, "did string lower comparison of '%s' against '%s', result %ld", search->Match, string, match);
    }
    break;

    case CP_GREATER:
    {
      if(isFlagSet(search->flags, SEARCHF_CASE_SENSITIVE))
        match = (BOOL)(strcmp(string, search->Match) > 0);
      else
        match = (BOOL)(Stricmp(string, search->Match) > 0);
      D(DBF_FILTER, "did string greater comparison of '%s' against '%s', result %ld", search->Match, string, match);
    }
    break;

    default:
    {
      E(DBF_FILTER, "unknown comparison mode %ld", search->Compare);
      match = FALSE;
    }
    break;
  }

  RETURN(match);
  return match;
}

///
/// FI_MatchPerson
//  Matches string against a person's name or address
static BOOL FI_MatchPerson(const struct Search *search, const struct Person *pe)
{
  BOOL match;

  ENTER();

  match = FI_MatchString(search, search->PersMode ? pe->RealName : pe->Address);

  RETURN(match);
  return match;
}

///
/// FI_SearchPatternFast
//  Searches string in standard header fields
static BOOL FI_SearchPatternFast(const struct Search *search, const struct Mail *mail)
{
  BOOL found = FALSE;

  ENTER();

  switch(search->Fast)
  {
    // search all "From:" addresses
    case FS_FROM:
    {
      struct ExtendedMail *email;

      if(FI_MatchPerson(search, &mail->From) == TRUE)
      {
        found = TRUE;
      }
      else if(isMultiSenderMail(mail) && (email = MA_ExamineMail(mail->Folder, mail->MailFile, TRUE)) != NULL)
      {
        int i;

        for(i=0; i < email->NumSFrom; i++)
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

      if(FI_MatchPerson(search, &mail->To) == TRUE)
      {
        found = TRUE;
      }
      else if(isMultiRCPTMail(mail) && (email = MA_ExamineMail(mail->Folder, mail->MailFile, TRUE)) != NULL)
      {
        int i;

        for(i=0; i < email->NumSTo; i++)
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

        for(i=0; i < email->NumCC; i++)
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

      if(FI_MatchPerson(search, &mail->ReplyTo) == TRUE)
      {
        found = TRUE;
      }
      else if(isMultiReplyToMail(mail) && (email = MA_ExamineMail(mail->Folder, mail->MailFile, TRUE)) != NULL)
      {
        int i;

        for(i=0; i < email->NumSReplyTo; i++)
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
      found = FI_MatchString(search, mail->Subject);
    }
    break;

    // search the "Date:" line
    case FS_DATE:
    {
      long cmp;

      if(search->dateTime.dat_Stamp.ds_Minute != 0)
        cmp = CompareDates(&mail->Date, &search->dateTime.dat_Stamp);
      else
        cmp = search->dateTime.dat_Stamp.ds_Days - mail->Date.ds_Days;

      switch(search->Compare)
      {
        case CP_EQUAL:
        {
          if(cmp == 0)
            found = TRUE;
        }
        break;

        case CP_NOTEQUAL:
        {
          if(cmp != 0)
            found = TRUE;
        }
        break;

        case CP_LOWER:
        {
          if(cmp > 0)
            found = TRUE;
        }
        break;

        case CP_GREATER:
        {
          if(cmp < 0)
            found = TRUE;
        }
        break;

        case CP_INPUT:
        {
          // this cannot happen
          E(DBF_FILTER, "CP_INPUT comparison for FS_DATE?");
        }
        break;
      }
    }
    break;

    // search the message size
    case FS_SIZE:
    {
      long cmp = search->Size - mail->Size;

      switch(search->Compare)
      {
        case CP_EQUAL:
        {
          if(cmp == 0)
            found = TRUE;
        }
        break;

        case CP_NOTEQUAL:
        {
          if(cmp != 0)
            found = TRUE;
        }
        break;

        case CP_LOWER:
        {
          if(cmp > 0)
            found = TRUE;
        }
        break;

        case CP_GREATER:
        {
          if(cmp < 0)
            found = TRUE;
        }
        break;

        case CP_INPUT:
        {
          // this cannot happen
          E(DBF_FILTER, "CP_INPUT comparison for FS_SIZE?");
        }
        break;
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
static BOOL FI_SearchPatternInBody(const struct Search *search, const struct Mail *mail)
{
  BOOL found = FALSE;
  struct ReadMailData *rmData;

  ENTER();

  if((rmData = AllocPrivateRMData(mail, PM_TEXTS|PM_QUIET)) != NULL)
  {
    char *cmsg;

    if((cmsg = RE_ReadInMessage(rmData, RIM_QUIET)) != NULL)
    {
      char *rptr = cmsg;
      char *ptr;

      while(*rptr != '\0' && found == FALSE && (G->FI != NULL ? !G->FI->Abort : TRUE))
      {
        for(ptr = rptr; *ptr && *ptr != '\n'; ptr++);

        *ptr = 0;
        if(FI_MatchString(search, rptr) == TRUE)
          found = TRUE;

        rptr = ++ptr;
      }

      if(G->FI != NULL && G->FI->Abort != FALSE)
        D(DBF_FILTER, "search was aborted");

      FreeStrBuf(cmsg);
    }

    FreePrivateRMData(rmData);
  }

  RETURN(found);
  return found;
}

///
/// FI_SearchPatternInHeader
//  Searches string in header field(s)
static BOOL FI_SearchPatternInHeader(const struct Search *search, const struct Mail *mail)
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
          int searchLen = 0;
          struct Node *curNode;

          // prepare the search length ahead of the iteration
          if(search->Field[0] != '\0')
          {
            char *ptr;

            // if the field is specified we search if it was specified with a ':'
            // at the end
            if((ptr = strchr(search->Field, ':')) != NULL)
              searchLen = ptr-(search->Field);
            else
              searchLen = strlen(search->Field);
          }

          IterateList(headerList, curNode)
          {
            struct HeaderNode *hdrNode = (struct HeaderNode *)curNode;

            // if the field is explicitly specified we search for it or
            // otherwise skip our search
            if(search->Field[0] != '\0')
            {
              // the search length has been calculated before
              if(strnicmp(hdrNode->name, search->Field, searchLen) != 0)
                continue;
            }

            found = FI_MatchString(search, hdrNode->content);

            // bail out as soon as we found a matching string
            if(found == TRUE)
              break;
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
  if(stricmp(field, "from") == 0)
    return FS_FROM;
  else if(stricmp(field, "to") == 0)
    return FS_TO;
  else if(stricmp(field, "cc") == 0)
    return FS_CC;
  else if(stricmp(field, "reply-to") == 0)
    return FS_REPLYTO;
  else if(stricmp(field, "subject") == 0)
    return FS_SUBJECT;
  else if(stricmp(field, "date") == 0)
    return FS_DATE;
  else
    return FS_NONE;
}

///
/// AllocSearchPatternNode
static struct SearchPatternNode *AllocSearchPatternNode(const char *pattern, const int flags)
{
  struct SearchPatternNode *spn;

  ENTER();

  if((spn = AllocSysObjectTags(ASOT_NODE, ASONODE_Size, sizeof(*spn),
                                          ASONODE_Min, TRUE,
                                          TAG_DONE)) != NULL)
  {
    BOOL parseOk = FALSE;

    if(isFlagSet(flags, SEARCHF_CASE_SENSITIVE))
    {
      if(ParsePattern(pattern, spn->pattern, sizeof(spn->pattern)) != -1)
        parseOk = TRUE;
    }
    else
    {
      if(ParsePatternNoCase(pattern, spn->pattern, sizeof(spn->pattern)) != -1)
        parseOk = TRUE;
    }

    if(parseOk == FALSE)
    {
      // parsing the pattern failed
      FreeSysObject(ASOT_NODE, spn);
      spn = NULL;
    }
  }

  RETURN(spn);
  return spn;
}

///
/// FI_GenerateListPatterns
//  Reads list of patterns from a file
static BOOL FI_GenerateListPatterns(struct Search *search)
{
  BOOL success = FALSE;
  FILE *fh;

  ENTER();

  if((fh = fopen(search->Match, "r")) != NULL)
  {
    char *buf = NULL;
    size_t size = 0;
    int numPatterns = 0;

    setvbuf(fh, NULL, _IOFBF, SIZE_FILEBUF);

    // we succeed only if we were able to open the pattern file at least
    success = TRUE;

    while(GetLine(&buf, &size, fh) >= 0)
    {
      if(buf[0] != '\0')
      {
        struct SearchPatternNode *spn;

        if((spn = AllocSearchPatternNode(buf, search->flags)) != NULL)
        {
          // add the pattern node to the list if parsing the pattern was successful
          AddTail((struct List *)&search->patternList, (struct Node *)spn);
          numPatterns++;
        }
        else
          break;
      }
    }

    free(buf);

    fclose(fh);

    // signal failure if we did not read a single pattern
    if(numPatterns == 0)
      success = FALSE;
  }

  RETURN(success);
  return success;
}

///
/// FI_PrepareSearch
//  Initializes Search structure
BOOL FI_PrepareSearch(struct Search *search, const enum SearchMode mode,
                      const int persmode, const enum Comparison compare,
                      const char stat, const char *match, const char *field, const int flags)
{
  BOOL success = TRUE;

  ENTER();

  memset(search, 0, sizeof(*search));
  search->Mode       = mode;
  search->flags      = flags;
  search->PersMode   = persmode;
  search->Compare    = compare;
  search->Status     = stat;
  strlcpy(search->Match, match, sizeof(search->Match));
  strlcpy(search->Field, field, sizeof(search->Field));
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
      char *timeStr;

      if((timeStr = strchr(match, ' ')) != NULL)
        timeStr++;
      else
        timeStr = (char *)"00:00:00";

      search->Fast = FS_DATE;
      search->dateTime.dat_Format = FORMAT_DEF;
      search->dateTime.dat_StrDate = (STRPTR)match;
      search->dateTime.dat_StrTime = (STRPTR)timeStr;

      if(StrToDate(&search->dateTime) == FALSE)
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

  if(success == TRUE)
  {
    if(compare == CP_INPUT)
    {
      success = FI_GenerateListPatterns(search);
    }
    else if(search->Fast != FS_DATE && search->Fast != FS_SIZE && mode != SM_SIZE)
    {
      if(isFlagSet(flags, SEARCHF_DOS_PATTERN))
      {
        struct SearchPatternNode *spn;

        // we are told to perform AmigaDOS pattern matching
        if(isFlagSet(flags, SEARCHF_SUBSTRING) || mode == SM_HEADER || mode == SM_BODY || mode == SM_WHOLE || mode == SM_STATUS)
        {
          char buffer[SIZE_PATTERN+1];

          // if substring is selected lets generate a substring from
          // the current match string, but keep the string borders in mind.
          strlcpy(buffer, search->Match, sizeof(buffer));
          snprintf(search->Match, sizeof(search->Match), "#?%s#?", buffer);
        }

        if((spn = AllocSearchPatternNode(search->Match, flags)) != NULL)
        {
          // add the pattern node to the list if parsing the pattern was successful
          AddTail((struct List *)&search->patternList, (struct Node *)spn);
        }
        else
        {
          success = FALSE;
        }
      }
      else if(isFlagSet(search->flags, SEARCHF_SUBSTRING))
      {
        // do a substring search using the Boyer/Moore algorithm
        if((search->bmContext = BoyerMooreInit(search->Match, isFlagSet(flags, SEARCHF_CASE_SENSITIVE))) == NULL)
          success = FALSE;
      }
      else
      {
        // do an exact string match
        // there is nothing to prepare here
	  }
    }
  }

  RETURN(success);
  return success;
}

///
/// FI_DoSearch
//  Checks if a message fulfills the search criteria
BOOL FI_DoSearch(struct Search *search, const struct Mail *mail)
{
  BOOL found = FALSE;
  #if defined(DEBUG)
  const char *searchString;
  #endif // DEBUG

  ENTER();

  #if defined(DEBUG)
  if(isFlagSet(search->flags, SEARCHF_DOS_PATTERN))
  {
    struct Node *node;

    if((node = GetHead((struct List *)&search->patternList)) != NULL)
    {
      struct SearchPatternNode *spn = (struct SearchPatternNode *)node;

      searchString = spn->pattern;
    }
    else
      searchString = search->Match;

    D(DBF_FILTER, "performing DOS pattern search for '%s'", searchString);
  }
  else if(isFlagSet(search->flags, SEARCHF_SUBSTRING) && search->bmContext != NULL)
  {
    searchString = search->bmContext->pattern;
    D(DBF_FILTER, "performing Boyer-Moore substring search for '%s'", searchString);
  }
  else
  {
    searchString = search->Match;
    E(DBF_FILTER, "performing simple string search for '%s'", searchString);
  }
  #endif // DEBUG

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
      if(search->Fast == FS_NONE)
        found = FI_SearchPatternInHeader(search, mail);
      else
        found = FI_SearchPatternFast(search, mail);

      if(found == TRUE)
        D(DBF_FILTER, "  search mode %ld matched", search->Mode);
      else
        D(DBF_FILTER, "  search mode %ld NOT matched", search->Mode);
    }
    break;

    case SM_HEADER:
    {
      enum Comparison oldCompare = search->Compare;

      // always perform a matching search
      search->Compare = CP_EQUAL;
      found = FI_SearchPatternInHeader(search, mail);
      search->Compare = oldCompare;

      // invert the result in case a non-matching search was requested
      if(oldCompare == CP_NOTEQUAL)
        found = !found;

      if(found == TRUE)
        D(DBF_FILTER, "  HEADER: search for '%s' matched", searchString);
      else
        D(DBF_FILTER, "  HEADER: search for '%s' NOT matched", searchString);
    }
    break;

    case SM_BODY:
    {
      enum Comparison oldCompare = search->Compare;

      // always perform a matching search
      search->Compare = CP_EQUAL;
      found = FI_SearchPatternInBody(search, mail);
      search->Compare = oldCompare;

      // invert the result in case a non-matching search was requested
      if(oldCompare == CP_NOTEQUAL)
        found = !found;

      if(found == TRUE)
        D(DBF_FILTER, "  BODY: search for '%s' matched", searchString);
      else
        D(DBF_FILTER, "  BODY: search for '%s' NOT matched", searchString);
    }
    break;

    case SM_WHOLE:
    {
      enum Comparison oldCompare = search->Compare;

      // always perform a matching search
      search->Compare = CP_EQUAL;
      found = FI_SearchPatternInHeader(search, mail);
      if(found == FALSE)
        found = FI_SearchPatternInBody(search, mail);
      search->Compare = oldCompare;

      // invert the result in case a non-matching search was requested
      if(oldCompare == CP_NOTEQUAL)
        found = !found;

      if(found == TRUE)
        D(DBF_FILTER, "  WHOLE: search for '%s' matched", searchString);
      else
        D(DBF_FILTER, "  WHOLE: search for '%s' NOT matched", searchString);
    }
    break;

    case SM_STATUS:
    {
      switch(search->Status)
      {
        case 'U':
          found = (hasStatusNew(mail) || !hasStatusRead(mail));
        break;

        case 'O':
          found = (!hasStatusNew(mail) && hasStatusRead(mail));
        break;

        case 'F':
          found = hasStatusForwarded(mail);
        break;

        case 'R':
          found = hasStatusReplied(mail);
        break;

        case 'W':
          found = hasStatusQueued(mail);
        break;

        case 'E':
          found = hasStatusError(mail);
        break;

        case 'H':
          found = hasStatusHold(mail);
        break;

        case 'S':
          found = hasStatusSent(mail);
        break;

        case 'M':
          found = hasStatusMarked(mail);
        break;

        case 'X':
          found = hasStatusSpam(mail);
        break;
      }

      // invert the result in case a non-matching search was requested
      if(search->Compare == CP_NOTEQUAL)
        found = !found;

      if(found == TRUE)
        D(DBF_FILTER, "  status search %ld matched", search->Status);
      else
        D(DBF_FILTER, "  status search %ld NOT matched", search->Status);
    }
    break;

    case SM_SPAM:
    {
      if(C->SpamFilterEnabled == TRUE && BayesFilterClassifyMessage(mail) == TRUE)
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
BOOL DoFilterSearch(const struct FilterNode *filter, const struct Mail *mail)
{
  BOOL lastCond = FALSE;
  int ruleIndex = 0;
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

      if(ruleIndex == 0)
      {
        // there is nothing to combine for the first rule of a filter
        lastCond = actCond;
      }
      else
      {
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

    ruleIndex++;
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
    struct MUI_NListtree_TreeNode *tn = (struct MUI_NListtree_TreeNode *)MUIV_NListtree_NextSelected_Start;

    while(TRUE)
    {
      struct Folder *folder;

      DoMethod(gui->LV_FOLDERS, MUIM_NListtree_NextSelected, &tn);
      if(tn == (struct MUI_NListtree_TreeNode *)MUIV_NListtree_NextSelected_End)
        break;

      if((folder = ((struct FolderNode *)tn->tn_User)->folder) != NULL)
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
        struct MailList *folderMessages;

        if((folderMessages = CloneMailList(folder->messages)) != NULL)
        {
          struct MailNode *mnode;

          // no need to lock the cloned list as it is private
          ForEachMailNode(folderMessages, mnode)
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

              // handle the possibly received methods and messages
              CheckMethodStack();
              HandleThreads(TRUE);

              // signal the application to update now
              DoMethod(G->App, MUIM_Application_InputBuffered);

              // forbid immediate display again
              set(lv, MUIA_NList_Quiet, TRUE);
            }
          }

          DeleteMailList(folderMessages);
        }

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

    if((filter = CreateNewFilter(FA_TERMINATE)) != NULL)
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
HOOKPROTONHNO(FI_OpenFunc, void, LONG *arg)
{
  ENTER();

  if(G->FI == NULL)
  {
    if((G->FI = FI_New()) == NULL)
      DisposeModulePush(&G->FI);
  }
  else if(G->FI->GUI.WI == NULL)
    DisposeModulePush(&G->FI);

  if(G->FI != NULL && G->FI->GUI.WI != NULL)
  {
    struct Folder *selectFolder = (struct Folder *)arg[0];

    // formerly a folder was marked to be searched in only if one was given
    // as this is quite unintuitive we now fall back to the current folder
    if(selectFolder == NULL && (struct MUI_NListtree_TreeNode *)xget(G->FI->GUI.LV_FOLDERS, MUIA_NListtree_Active) == NULL)
      selectFolder = GetCurrentFolder();

    if(selectFolder != NULL)
    {
      struct MUI_NListtree_TreeNode *tn;
      int i=0;

      // now we have to walk through the nlisttree and find
      // the folder so that we can set it as active
      while((tn = (struct MUI_NListtree_TreeNode *)DoMethod(G->FI->GUI.LV_FOLDERS, MUIM_NListtree_GetEntry, MUIV_NListtree_GetEntry_ListNode_Root, i, MUIF_NONE)) != NULL)
      {
        struct FolderNode *fnode = tn->tn_User;

        if(fnode->folder == selectFolder)
        {
          set(G->FI->GUI.LV_FOLDERS, MUIA_NListtree_Active, tn);
          break;
        }

        i++;
      }
    }

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

  LEAVE();
}
MakeHook(FI_OpenHook, FI_OpenFunc);

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
  int i;

  ENTER();

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
    if(foundmail->Folder == GetCurrentFolder())
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
BOOL FI_FilterSingleMail(const struct MinList *filterList, struct Mail *mail, int *matches, struct FilterResult *result)
{
  BOOL success = TRUE;
  struct Node *curNode;
  int match = 0;

  ENTER();

  IterateList(filterList, curNode)
  {
    struct FilterNode *filter = (struct FilterNode *)curNode;

    if(DoFilterSearch(filter, mail) == TRUE)
    {
      match++;

      // if ExecuteFilterAction returns FALSE then the filter search should be aborted
      // completley
      if(ExecuteFilterAction(filter, mail, result) == FALSE)
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
/// FreeSearchData
// Function to free the search data
void FreeSearchData(struct Search *search)
{
  struct Node *curNode;

  ENTER();

  // free all search pattern nodes
  while((curNode = RemHead((struct List *)&search->patternList)) != NULL)
  {
    struct SearchPatternNode *patternNode = (struct SearchPatternNode *)curNode;

    FreeSysObject(ASOT_NODE, patternNode);
  }

  // free a possibly initalized Boyer-Moore search context
  if(search->bmContext != NULL)
  {
    BoyerMooreCleanup(search->bmContext);
    search->bmContext = NULL;
  }

  LEAVE();
}

///
/// CloneFilterNode
// clone a filter including all its rules
struct FilterNode *CloneFilterNode(struct FilterNode *filter)
{
  struct FilterNode *clonedFilter;

  ENTER();

  if((clonedFilter = DuplicateNode(filter, sizeof(*filter))) != NULL)
  {
    struct Node *node;

    // clone the filter's rules
    NewMinList(&clonedFilter->ruleList);
    IterateList(&filter->ruleList, node)
    {
      struct RuleNode *rule = (struct RuleNode *)node;
      struct RuleNode *clonedRule;

      if((clonedRule = DuplicateNode(rule, sizeof(*rule))) != NULL)
      {
        AddTail((struct List *)&clonedFilter->ruleList, (struct Node *)clonedRule);
      }
      else
      {
        // delete the cloned filter again if anything fails
        DeleteFilterNode(clonedFilter);
        clonedFilter = NULL;
        break;
      }
    }
  }

  RETURN(clonedFilter);
  return clonedFilter;
}

///
/// DeleteRuleNode
// delete a rule node
void DeleteRuleNode(struct RuleNode *rule)
{
  ENTER();

  // free possible search data
  if(rule->search != NULL)
  {
    FreeSearchData(rule->search);
    free(rule->search);
    rule->search = NULL;
  }

  // and finally free the rule itself
  FreeSysObject(ASOT_NODE, rule);

  LEAVE();
}

///
/// DeleteFilterNode
// delete a filter node
void DeleteFilterNode(struct FilterNode *filter)
{
  ENTER();

  // free this filter's rules
  FreeFilterRuleList(filter);

  // and finally free the filter itself
  FreeSysObject(ASOT_NODE, filter);

  LEAVE();
}

///
/// DeleteFilterList
// delete a cloned filter list
void DeleteFilterList(struct MinList *filterList)
{
  ENTER();

  FreeFilterList(filterList);
  FreeSysObject(ASOT_LIST, filterList);

  LEAVE();
}

///
/// CloneFilterList
// clone the configured filter list including on those filters which apply for a certain mode only
struct MinList *CloneFilterList(enum ApplyFilterMode mode)
{
  struct MinList *clonedList;

  ENTER();

  if((clonedList = AllocSysObjectTags(ASOT_LIST, ASOLIST_Min, TRUE,
                                                 TAG_DONE)) != NULL)
  {
    struct Node *node;

    IterateList(&C->filterList, node)
    {
      struct FilterNode *filter = (struct FilterNode *)node;
      BOOL include;

      // check if the filter needs to be included in the cloned list
      switch(mode)
      {
        case APPLY_AUTO:
          include = (filter->applyToNew == TRUE && filter->remote == FALSE);
        break;

        case APPLY_USER:
          include = (filter->applyOnReq == TRUE && filter->remote == FALSE);
        break;

        case APPLY_SENT:
          include = (filter->applyToSent == TRUE && filter->remote == FALSE);
        break;

        case APPLY_REMOTE:
          include = (filter->remote == TRUE);
        break;

        case APPLY_SPAM:
          include = FALSE;
        break;

        default:
          include = TRUE;
        break;
      }

      if(include == TRUE)
      {
        struct FilterNode *clonedFilter;

        // now clone the filter including its rules
        if((clonedFilter = CloneFilterNode(filter)) != NULL)
        {
          D(DBF_FILTER, "cloned filter '%s'", clonedFilter->name);
          AddTail((struct List *)clonedList, (struct Node *)clonedFilter);
        }
        else
        {
          // delete the list again if anything fails
          DeleteFilterList(clonedList);
          clonedList = NULL;
        }
      }
    }

    if(clonedList != NULL)
    {
      // now that we have cloned the filter list we go ahead and prepare the
      // search data for each rule
      struct Node *filterNode;

      IterateList(clonedList, filterNode)
      {
        struct FilterNode *filter = (struct FilterNode *)filterNode;
        struct Node *ruleNode;

        IterateList(&filter->ruleList, ruleNode)
        {
          struct RuleNode *rule = (struct RuleNode *)ruleNode;

          if((rule->search = calloc(1, sizeof(*rule->search))) != NULL)
          {
            int stat;

            // we check the status field first and if we find a match
            // we can immediatly break up here because we don't need to prepare the search
            if(rule->searchMode == SM_STATUS)
            {
              for(stat=0; stat <= (int)sizeof(mailStatusCycleMap) - 1; stat++)
              {
                if(rule->matchPattern[0] == mailStatusCycleMap[stat])
                  break;
              }
            }
            else
            {
              stat = sizeof(mailStatusCycleMap);
            }

            FI_PrepareSearch(rule->search,
                             rule->searchMode,
                             rule->subSearchMode,
                             rule->comparison,
                             mailStatusCycleMap[stat],
                             rule->matchPattern,
                             rule->customField,
                             rule->flags);

            // save a pointer to the filter in the search structure as well.
            rule->search->filter = filter;
          }
        }
      }
    }
  }


  RETURN(clonedList);
  return clonedList;
}

///
/// ExecuteFilterAction
//  Applies filter action to a message and return TRUE if the filter search
//  should continue or FALSE if it should stop afterwards
BOOL ExecuteFilterAction(const struct FilterNode *filter, struct Mail *mail, struct FilterResult *result)
{
  BOOL success = TRUE;

  ENTER();

  // Bounce Action
  if(hasBounceAction(filter) && filter->remote == FALSE && *filter->bounceTo)
  {
    struct WriteMailData *wmData;

    if((wmData = NewBounceMailWindow(mail, NEWF_QUIET)) != NULL)
    {
      set(wmData->window, MUIA_WriteWindow_To, filter->bounceTo);
      DoMethod(wmData->window, MUIM_WriteWindow_ComposeMail, WRITE_QUEUE);

      if(result != NULL)
        result->Bounced++;
    }
  }

  // Forward Action
  if(hasForwardAction(filter) && filter->remote == FALSE && *filter->forwardTo)
  {
    struct MailList *mlist;

    if((mlist = CreateMailList()) != NULL)
    {
      struct WriteMailData *wmData;

      AddNewMailNode(mlist, mail);

      if((wmData = NewForwardMailWindow(mlist, NEWF_QUIET)) != NULL)
      {
        set(wmData->window, MUIA_WriteWindow_To, filter->forwardTo);
        DoMethod(wmData->window, MUIM_WriteWindow_ComposeMail, WRITE_QUEUE);

        if(result != NULL)
          result->Forwarded++;
      }

      DeleteMailList(mlist);
    }
  }

  // Reply Action
  if(hasReplyAction(filter) && filter->remote == FALSE && *filter->replyFile)
  {
    struct MailList *mlist;

    if((mlist = CreateMailList()) != NULL)
    {
      struct WriteMailData *wmData;

      AddNewMailNode(mlist, mail);

      if((wmData = NewReplyMailWindow(mlist, NEWF_QUIET, NULL)) != NULL)
      {
        DoMethod(wmData->window, MUIM_WriteWindow_LoadText, filter->replyFile, TRUE);
        DoMethod(wmData->window, MUIM_WriteWindow_ComposeMail, WRITE_QUEUE);

        if(result != NULL)
          result->Replied++;
      }

      DeleteMailList(mlist);
    }
  }

  // Execute Action
  if(hasExecuteAction(filter) && *filter->executeCmd)
  {
    char mailfile[SIZE_PATHFILE];
    char buf[SIZE_COMMAND + SIZE_PATHFILE];

    GetMailFile(mailfile, sizeof(mailfile), NULL, mail);
    snprintf(buf, sizeof(buf), "%s \"%s\"", filter->executeCmd, mailfile);
    LaunchCommand(buf, 0, OUT_STDOUT);
    if(result != NULL)
      result->Executed++;
  }

  // PlaySound Action
  if(hasPlaySoundAction(filter) && *filter->playSound)
    PlaySound(filter->playSound);

  // Status to "marked/unmarked" action
  if(hasStatusToMarkedAction(filter) && filter->remote == FALSE)
  {
    setStatusToMarked(mail);
  }
  else if(hasStatusToUnmarkedAction(filter) && filter->remote == FALSE)
  {
    setStatusToUnmarked(mail);
  }

  // Status to "read/unread" action
  if(hasStatusToReadAction(filter) && filter->remote == FALSE)
  {
    setStatusToRead(mail);
  }
  else if(hasStatusToUnreadAction(filter) && filter->remote == FALSE)
  {
    setStatusToUnread(mail);
  }

  // Status to "spam/not spam" action
  if(hasStatusToSpamAction(filter) && filter->remote == FALSE)
  {
    setStatusToAutoSpam(mail);
  }
  else if(hasStatusToHamAction(filter) && filter->remote == FALSE)
  {
    setStatusToHam(mail);
  }

  // Move Action
  if(hasMoveAction(filter) && filter->remote == FALSE)
  {
    struct Folder* fo;

    if((fo = FO_GetFolderByName(filter->moveTo, NULL)) != NULL)
    {
      if(mail->Folder != fo)
      {
        BOOL accessFreed = FALSE;
        enum LoadedMode oldLoadedMode = fo->LoadedMode;

        if(result != NULL)
        result->Moved++;

        // temporarily grant free access to the folder, but only if it has no free access yet
        if(fo->LoadedMode != LM_VALID && isProtectedFolder(fo) && isFreeAccess(fo) == FALSE)
        {
          setFlag(fo->Flags, FOFL_FREEXS);
          accessFreed = TRUE;
        }

        MA_MoveCopy(mail, mail->Folder, fo, MVCPF_CLOSE_WINDOWS|MVCPF_QUIET);

        // restore the old access mode if it was changed before
        if(accessFreed)
        {
          // restore old index settings
          // if it was not yet loaded before, the MA_MoveCopy() call changed this to "loaded"
          fo->LoadedMode = oldLoadedMode;
          clearFlag(fo->Flags, FOFL_FREEXS);
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
    if(result != NULL)
      result->Deleted++;

    if(isSendMDNMail(mail) &&
       (hasStatusNew(mail) || !hasStatusRead(mail)))
    {
      RE_ProcessMDN(MDN_MODE_DELETE, mail, FALSE, TRUE, G->MA->GUI.WI);
    }

    MA_DeleteSingle(mail, DELF_CLOSE_WINDOWS|DELF_QUIET);

    // signal failure, although everything was successful yet
    // but the mail is not available anymore for other filters
    success = FALSE;
  }

  // Termiante Action
  if(hasTerminateAction(filter))
  {
    // just signal failure to terminate further handling of this mail
    success = FALSE;
  }

  RETURN(success);
  return success;
}

///
/// FilterMails
// Apply filters
void FilterMails(struct Folder *folder, const struct MailList *mlist, const int mode, struct FilterResult *result)
{
  struct MinList *filterList;

  ENTER();

  // clear the result statistics first
  memset(result, 0, sizeof(*result));

  if((filterList = CloneFilterList(mode)) != NULL)
  {
    struct BusyNode *busy;
    struct Folder *spamfolder = FO_GetFolderByType(FT_SPAM, NULL);
    struct MailNode *mnode;
    ULONG m;
    int matches = 0;
    BOOL noFilters = IsMinListEmpty(filterList);
    struct TimeVal lastStatsUpdate;
    struct FilterResult lastResult;

    set(G->MA->GUI.PG_MAILLIST, MUIA_NList_Quiet, TRUE);
    G->AppIconQuiet = TRUE;

    LockMailList(mlist);

    busy = BusyBegin(BUSY_PROGRESS_ABORT);
    // we use another Busy Gauge information if this is
    // a spam classification session. And we build an interruptable
    // Gauge which will report back if the user pressed the stop button
    if(mode != APPLY_SPAM)
      BusyText(busy, tr(MSG_BusyFiltering), "");
    else
      BusyText(busy, tr(MSG_FI_BUSYCHECKSPAM), "");

    MA_StartMacro(MACRO_PREFILTER, NULL);

    memset(&lastStatsUpdate, 0, sizeof(lastStatsUpdate));
    memset(&lastResult, 0, sizeof(lastResult));

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
              MA_MoveCopy(mail, folder, spamfolder, MVCPF_QUIET);
              wasSpam = TRUE;

              // update the stats
              result->Spam++;
              // we just checked the mail
              result->Checked++;
            }
          }
        }

        if(noFilters == FALSE && wasSpam == FALSE)
        {
          // apply all other user defined filters (if they exist) for non-spam mails
          // or if the spam filter is disabled
          result->Checked++;

          // now we process the search
          FI_FilterSingleMail(filterList, mail, &matches, result);
        }

        // we update the busy gauge and
        // see if we have to exit/abort in case it returns FALSE
        if(BusyProgress(busy, ++m, mlist->count) == FALSE)
          break;

        // check if some mails were deleted, moved or recognized as spam
        if((lastResult.Moved != result->Moved || lastResult.Deleted != result->Deleted || lastResult.Spam != result->Spam))
        {
          // now check if enough time has passed since the last update
          if(TimeHasElapsed(&lastStatsUpdate, 500000) == TRUE)
          {
            // update the tree and remember the new stats
            FolderTreeUpdate();
            memcpy(&lastResult, result, sizeof(lastResult));
          }
        }
      }
    }

    UnlockMailList(mlist);

    DeleteFilterList(filterList);

    if(result->Checked != 0)
      AppendToLogfile(LF_ALL, 26, tr(MSG_LOG_Filtering), result->Checked, folder->Name, matches);

    MA_StartMacro(MACRO_POSTFILTER, NULL);

    BusyEnd(busy);

    set(G->MA->GUI.PG_MAILLIST, MUIA_NList_Quiet, FALSE);
    G->AppIconQuiet = FALSE;

    // update the folder tree once more to get the stats correct
    FolderTreeUpdate();

    MA_ChangeSelected(FALSE);

    if(mode != APPLY_AUTO)
      DisplayStatistics(NULL, TRUE);
  }

  LEAVE();
}

///
/// ApplyFiltersFunc
//  Apply filters
HOOKPROTONHNO(ApplyFiltersFunc, void, int *arg)
{
  struct Folder *folder;
  enum ApplyFilterMode mode = arg[0];
  struct FilterResult filterResult;

  ENTER();

  D(DBF_FILTER, "About to apply SPAM and user defined filters in mode %ld...", mode);

  memset(&filterResult, 0, sizeof(filterResult));

  folder = (mode == APPLY_AUTO) ? FO_GetFolderByType(FT_INCOMING, NULL) : GetCurrentFolder();

  if(folder != NULL && (C->SpamFilterEnabled == FALSE || FO_GetFolderByType(FT_SPAM, NULL) != NULL))
  {
    struct MailList *mlist = NULL;
    BOOL processAllMails = TRUE;

    // query how many mails are currently selected/marked
    if(mode == APPLY_USER || mode == APPLY_RX || mode == APPLY_RX_ALL || mode == APPLY_SPAM)
    {
      ULONG minselected = isAnyFlagSet(arg[1], (IEQUALIFIER_LSHIFT|IEQUALIFIER_RSHIFT)) ? 1 : 2;

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

        if(MUI_Request(G->App, G->MA->GUI.WI, MUIF_NONE, tr(MSG_MA_ConfirmReq), tr(MSG_YesNoReq), buf) == 0)
          applyFilters = FALSE;
      }

      // the user has not cancelled the filter process
      if(applyFilters == TRUE)
      {
        FilterMails(folder, mlist, mode, &filterResult);

        if(filterResult.Checked != 0 && mode == APPLY_USER)
        {
          if(C->ShowFilterStats == TRUE)
          {
            char buf[SIZE_LARGE];

            if(C->SpamFilterEnabled == TRUE)
            {
              // include the number of spam classified mails
              snprintf(buf, sizeof(buf), tr(MSG_MA_FILTER_STATS_SPAM), filterResult.Checked,
                                                                       filterResult.Forwarded,
                                                                       filterResult.Moved,
                                                                       filterResult.Deleted,
                                                                       filterResult.Spam);
            }
            else
            {
              snprintf(buf, sizeof(buf), tr(MSG_MA_FilterStats), filterResult.Checked,
                                                                 filterResult.Forwarded,
                                                                 filterResult.Moved,
                                                                 filterResult.Deleted);
            }

            MUI_Request(G->App, G->MA->GUI.WI, MUIF_NONE, NULL, tr(MSG_OkayReq), buf);
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

  // copy the results if anybody is interested in them
  if(arg[2] != 0)
  {
    memcpy((struct FilterResult *)arg[2], &filterResult, sizeof(filterResult));
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
        if((newRule->search = malloc(sizeof(*newRule->search))) != NULL)
        {
          if(CopySearchData(newRule->search, rule->search) == FALSE)
          {
            // let the cloned search data point to the correct filter
            newRule->search->filter = dstFilter;
          }
          else
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

      // add the rule to the ruleList of our destination filter
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
  memcpy(dstSearch, srcSearch, sizeof(*dstSearch));

  dstSearch->bmContext = NULL;

  // now we have to copy the patternList as well
  NewMinList(&dstSearch->patternList);

  IterateList(&srcSearch->patternList, curNode)
  {
    struct SearchPatternNode *srcNode = (struct SearchPatternNode *)curNode;
    struct SearchPatternNode *dstNode;

    if((dstNode = DuplicateNode(srcNode, sizeof(*srcNode))) != NULL)
    {
      AddTail((struct List *)&dstSearch->patternList, (struct Node *)dstNode);
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
/// FreeFilterRuleList
void FreeFilterRuleList(struct FilterNode *filter)
{
  struct Node *curNode;

  ENTER();

  // we do have to iterate through our ruleList and
  // free them as well
  while((curNode = RemHead((struct List *)&filter->ruleList)) != NULL)
    DeleteRuleNode((struct RuleNode *)curNode);

  // initialize the ruleList as well
  NewMinList(&filter->ruleList);

  LEAVE();
}

///
/// CompareRuleNodes
static BOOL CompareRuleNodes(const struct Node *n1, const struct Node *n2)
{
  BOOL equal = TRUE;
  const struct RuleNode *rn1 = (const struct RuleNode *)n1;
  const struct RuleNode *rn2 = (const struct RuleNode *)n2;

  ENTER();

  // compare every single member of the structure
  if(rn1->combine           != rn2->combine ||
     rn1->searchMode        != rn2->searchMode ||
     rn1->subSearchMode     != rn2->subSearchMode ||
     rn1->comparison        != rn2->comparison ||
     rn1->flags             != rn2->flags ||
     strcmp(rn1->matchPattern, rn2->matchPattern) != 0 ||
     strcmp(rn1->customField,  rn2->customField) != 0)
  {
    equal = FALSE;
  }

  RETURN(equal);
  return equal;
}

///
/// CompareRuleLists
// compare two rule lists to be equal
static BOOL CompareRuleLists(const struct MinList *rl1, const struct MinList *rl2)
{
  BOOL equal;

  ENTER();

  equal = CompareLists((const struct List *)rl1, (const struct List *)rl2, CompareRuleNodes);

  RETURN(equal);
  return equal;
}

///
/// CompareFilterNodes
static BOOL CompareFilterNodes(const struct Node *n1, const struct Node *n2)
{
  BOOL equal = TRUE;
  const struct FilterNode *fn1 = (const struct FilterNode *)n1;
  const struct FilterNode *fn2 = (const struct FilterNode *)n2;

  ENTER();

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
    equal = FALSE;
  }

  RETURN(equal);
  return equal;
}

///
/// CompareFilterLists
// performs a deep compare of two filter lists and returns TRUE if they are equal
BOOL CompareFilterLists(const struct MinList *fl1, const struct MinList *fl2)
{
  BOOL equal;

  ENTER();

  equal = CompareLists((const struct List *)fl1, (const struct List *)fl2, CompareFilterNodes);

  RETURN(equal);
  return equal;
}

///
/// CreateNewFilter
//  Initializes a new filter
struct FilterNode *CreateNewFilter(const int actions)
{
  struct FilterNode *filter;

  ENTER();

  if((filter = AllocSysObjectTags(ASOT_NODE, ASONODE_Size, sizeof(*filter),
                                             ASONODE_Min, TRUE,
                                             TAG_DONE)) != NULL)
  {
    filter->actions = actions;
    filter->isVolatile = FALSE;
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
    if(CreateNewRule(filter, 0) == NULL)
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

    DeleteFilterNode(filter);
  }

  NewMinList(filterList);

  LEAVE();
}

///
/// CreateNewRule
//  Initializes a new filter rule
struct RuleNode *CreateNewRule(struct FilterNode *filter, const int flags)
{
  struct RuleNode *rule;

  ENTER();

  if((rule = AllocSysObjectTags(ASOT_NODE,
    ASONODE_Size, sizeof(*rule),
    ASONODE_Min, TRUE,
    TAG_DONE)) != NULL)
  {
    // set the default search mode (plain string search or DOS patterns)
    rule->search = NULL;
    rule->combine = CB_OR;
    rule->searchMode = SM_FROM;
    rule->subSearchMode = SSM_ADDRESS;
    rule->comparison = CP_EQUAL;
    rule->flags = flags;
    rule->matchPattern[0] = '\0';
    rule->customField[0] = '\0';

    // if a filter was specified we immediately add this new rule to it
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
      clearFlag(filter->actions, FA_MOVE);

      // remember the modified configuration, but don't save it yet
      C->ConfigIsSaved = FALSE;
    }
  }

  LEAVE();
}

///
/// ImportFilter
// import filters from Thunderbird's .sfd file
BOOL ImportFilter(const char *fileName, const BOOL isVolatile, struct MinList *filterList)
{
  BOOL success = FALSE;
  FILE *fh;

  ENTER();

  D(DBF_FILTER, "import filters from file '%s'", fileName);

  if((fh = fopen(fileName, "r")) != NULL)
  {
    char *buf = NULL;
    size_t size = 0;
    struct FilterNode *filter = NULL;
    char *lastAction = NULL;

    setvbuf(fh, NULL, _IOFBF, SIZE_FILEBUF);

    while(GetLine(&buf, &size, fh) >= 0)
    {
      char *eq;

      // each line looks like this
      // keyword="value"
      if((eq = strchr(buf, '=')) != NULL)
      {
        char *value;

        *eq++ = '\0';
        value = UnquoteString(eq, FALSE);

        // every "name" line introduces a new filter
        if(stricmp(buf, "name") == 0)
        {
          // push a previous filter to the configuration
          if(filter != NULL)
          {
            if(G->CO != NULL && isVolatile == FALSE)
            {
              DoMethod(G->CO->GUI.LV_RULES, MUIM_NList_InsertSingle, filter, MUIV_NList_Insert_Bottom);
              set(G->CO->GUI.LV_RULES, MUIA_NList_Active, MUIV_NList_Active_Bottom);
            }

            // volatile filters are added at the top
            if(isVolatile == TRUE)
            {
              AddHead((struct List *)filterList, (struct Node *)filter);
              D(DBF_FILTER, "imported volatile filter '%s'", filter->name);
            }
            else
            {
              AddTail((struct List *)filterList, (struct Node *)filter);
              D(DBF_FILTER, "imported filter '%s'", filter->name);
            }

            success = TRUE;
          }

          // create a new filter node and remember the name
          if((filter = CreateNewFilter(0)) != NULL)
          {
            strlcpy(filter->name, eq, sizeof(filter->name));
            filter->isVolatile = isVolatile;
          }
          else
          {
            success = FALSE;
            break;
          }
        }
        else if(stricmp(buf, "enabled") == 0)
        {
          // if the filter is not enabled we keep it nevertheless, but it will never be applied
          if(stricmp(value, "yes") != 0)
          {
            filter->remote = FALSE;
            filter->applyOnReq = FALSE;
            filter->applyToNew = FALSE;
            filter->applyToSent = FALSE;
          }
        }
        else if(stricmp(buf, "action") == 0)
        {
          // transform actions without further information into direct actions of our filter
          if(filter != NULL)
          {
            if(stricmp(buf, "mark read") == 0)
            {
              setFlag(filter->actions, FA_STATUSTOREAD);
            }
            else if(stricmp(buf, "mark unread") == 0)
            {
              setFlag(filter->actions, FA_STATUSTOUNREAD);
            }
            else if(stricmp(buf, "mark flagged") == 0)
            {
              setFlag(filter->actions, FA_STATUSTOMARKED);
            }
            else if(stricmp(buf, "delete") == 0)
            {
              setFlag(filter->actions, FA_DELETE);
              setFlag(filter->actions, FA_TERMINATE);
            }
            else if(stricmp(buf, "stop execution") == 0)
            {
              setFlag(filter->actions, FA_TERMINATE);
            }

            // remember this action
            free(lastAction);
            lastAction = strdup(value);
          }
        }
        else if(stricmp(buf, "actionvalue") == 0)
        {
          // handle further action values
          if(lastAction != NULL && filter != NULL)
          {
            if(stricmp(lastAction, "forward") == 0)
            {
              setFlag(filter->actions, FA_FORWARD);
              strlcpy(filter->forwardTo, value, sizeof(filter->forwardTo));
            }
            else if(stricmp(lastAction, "junkscore") == 0)
            {
              // spam filters are applied to new mails only and not on user request
              // this allows filters to be applied again when false negatives are to
              // be filtered again
              filter->applyOnReq = FALSE;

              // usually only values of 0 and 100 are used, but just to be sure
              // we treat every value greater than 50% as true junk
              if(atoi(value) >= 50)
              {
                struct Folder *folder;

                if((folder = FO_GetFolderByType(FT_SPAM, NULL)) != NULL)
                {
                  // move the mail to the spam folder if it exists
                  strlcpy(filter->moveTo, folder->Name, sizeof(filter->moveTo));
                  setFlag(filter->actions, FA_MOVE);
                }
                else
                {
                  // otherwise mark the mail as spam only
                  setFlag(filter->actions, FA_STATUSTOSPAM);
                }
                // and terminate the filter processing
                setFlag(filter->actions, FA_TERMINATE);
              }
              else
              {
                setFlag(filter->actions, FA_STATUSTOHAM);
              }
            }
          }
        }
        else if(stricmp(buf, "condition") == 0)
        {
          int ruleCount = 0;

          // parse the conditions. These look like
          // AND/OR (field,comparison,string) [AND/OR (field,comparison,string) ...]
          while(value != NULL && value[0] != '\0')
          {
            struct RuleNode *rule;
            char *p;
            char *q;

            while((rule = GetFilterRule(filter, ruleCount)) == NULL)
              CreateNewRule(filter, 0);

            // transform the combination into rule combinations
            if(strnicmp(value, "and", 3) == 0)
              rule->combine = CB_AND;
            else if(strnicmp(value, "or", 2) == 0)
              rule->combine = CB_OR;

            p = strchr(value, '(');
            q = strchr(value, ')');
            if(p != NULL && q != NULL && q > p)
            {
              char *s = p+1;
              int part = 0;
              char *comparison = NULL;

              *q = '\0';

              while(s != NULL && s[0] != '\0')
              {
                if((p = strchr(s, ',')) != NULL)
                  *p++ = '\0';

                switch(part)
                {
                  case 0:
                  {
                    if(stricmp(s, "subject") == 0)
                    {
                      rule->searchMode = SM_SUBJECT;
                    }
                    else if(stricmp(s, "from") == 0)
                    {
                      rule->searchMode = SM_FROM;
                    }
                    else if(stricmp(s, "to") == 0)
                    {
                      rule->searchMode = SM_TO;
                    }
                    else if(stricmp(s, "cc") == 0)
                    {
                      rule->searchMode = SM_CC;
                    }
                    else if(stricmp(s, "body") == 0)
                    {
                      rule->searchMode = SM_BODY;
                    }
                    else if(stricmp(s, "date") == 0)
                    {
                      rule->searchMode = SM_DATE;
                    }
                    else if(stricmp(s, "size") == 0)
                    {
                      rule->searchMode = SM_SIZE;
                    }
                    else if(s[0] == '"')
                    {
                      // comparisons within special header lines are included in quotes
                      char *t;

                      if((t = strchr(s+1, '"')) != NULL)
                      {
                        *t = '\0';
		                    rule->searchMode = SM_HEADLINE;
                        strlcpy(rule->customField, s+1, sizeof(rule->customField));
                      }
                    }
                    else
                    {
                      rule->searchMode = SM_WHOLE;
                    }
                  }
                  break;

                  case 1:
                  {
                    comparison = s;
                  }
                  break;

                  case 2:
                  {
                    if(comparison != NULL)
                    {
                      if(rule->searchMode == SM_SIZE)
                      {
                        strlcpy(rule->matchPattern, s, sizeof(rule->matchPattern));
                        if(stricmp(comparison, "is") == 0)
                        {
                          rule->comparison = CP_EQUAL;
                        }
                        else if(stricmp(comparison, "is greater than") == 0)
                        {
                          rule->comparison = CP_GREATER;
                        }
                        else if(stricmp(comparison, "is less than") == 0)
                        {
                          rule->comparison = CP_LOWER;
                        }
                      }
                      else if(rule->searchMode == SM_DATE)
                      {
                        strlcpy(rule->matchPattern, s, sizeof(rule->matchPattern));
                        if(stricmp(comparison, "is") == 0)
                        {
                          rule->comparison = CP_EQUAL;
                        }
                        else if(stricmp(comparison, "isn't") == 0)
                        {
                          rule->comparison = CP_NOTEQUAL;
                        }
                        else if(stricmp(comparison, "is before") == 0)
                        {
                          rule->comparison = CP_LOWER;
                        }
                        else if(stricmp(comparison, "is after") == 0)
                        {
                          rule->comparison = CP_GREATER;
                        }
                      }
                      else
                      {
                        if(stricmp(comparison, "is") == 0)
                        {
                          setFlag(rule->flags, SEARCHF_CASE_SENSITIVE);
                          strlcpy(rule->matchPattern, s, sizeof(rule->matchPattern));
                          rule->comparison = CP_EQUAL;
                        }
                        else if(stricmp(comparison, "isn't") == 0)
                        {
                          setFlag(rule->flags, SEARCHF_CASE_SENSITIVE);
                          strlcpy(rule->matchPattern, s, sizeof(rule->matchPattern));
                          rule->comparison = CP_NOTEQUAL;
                        }
                        else if(stricmp(comparison, "contains") == 0)
                        {
                          setFlag(rule->flags, SEARCHF_CASE_SENSITIVE);
                          setFlag(rule->flags, SEARCHF_SUBSTRING);
                          strlcpy(rule->matchPattern, s, sizeof(rule->matchPattern));
                          rule->comparison = CP_EQUAL;
                        }
                        else if(stricmp(comparison, "doesn't contain") == 0)
                        {
                          setFlag(rule->flags, SEARCHF_CASE_SENSITIVE);
                          setFlag(rule->flags, SEARCHF_SUBSTRING);
                          strlcpy(rule->matchPattern, s, sizeof(rule->matchPattern));
                          rule->comparison = CP_NOTEQUAL;
                        }
                        else if(stricmp(comparison, "begins with") == 0)
                        {
                          setFlag(rule->flags, SEARCHF_CASE_SENSITIVE);
                          setFlag(rule->flags, SEARCHF_DOS_PATTERN);
                          snprintf(rule->matchPattern, sizeof(rule->matchPattern), "%s#?", s);
                        }
                        else if(stricmp(comparison, "ends with") == 0)
                        {
                          setFlag(rule->flags, SEARCHF_CASE_SENSITIVE);
                          setFlag(rule->flags, SEARCHF_DOS_PATTERN);
                          snprintf(rule->matchPattern, sizeof(rule->matchPattern), "#?%s", s);
                        }
                      }
                    }
                  }
                  break;

                  default:
                  {
                  }
                  break;
                }

                s = p;
                part++;
              }

              // skip the closing brace
              q++;
              // skip spaces either until the next condition or until the end of the string
              while(*q == ' ' && *q != '\0')
                q++;
            }

            value = q;
            ruleCount++;
          }
        }
        else
        {
          D(DBF_FILTER, "skipping keyword '%s' value '%s'", buf, value);
        }
      }
	  }

    free(buf);

    // free the last remembered action
    free(lastAction);

    // push the last created filter to the configuration
    if(filter != NULL)
    {
      if(G->CO != NULL && isVolatile == FALSE)
      {
        DoMethod(G->CO->GUI.LV_RULES, MUIM_NList_InsertSingle, filter, MUIV_NList_Insert_Bottom);
        set(G->CO->GUI.LV_RULES, MUIA_NList_Active, MUIV_NList_Active_Bottom);
      }

      // volatile filters are added at the top
      if(isVolatile == TRUE)
      {
        AddHead((struct List *)filterList, (struct Node *)filter);
        D(DBF_FILTER, "imported volatile filter '%s'", filter->name);
      }
      else
      {
        AddTail((struct List *)filterList, (struct Node *)filter);
        D(DBF_FILTER, "imported filter '%s'", filter->name);
      }
    }

    fclose(fh);
  }
  else
  {
    E(DBF_FILTER, "filter import from '%s' failed", fileName);
  }

  RETURN(success);
  return success;
}

///

/*** GUI ***/
/// InitFilterPopupList
//  Creates a popup list of configured filters
HOOKPROTONHNP(InitFilterPopupList, ULONG, Object *listview)
{
  Object *list;

  ENTER();

  if((list = (Object *)xget(listview, MUIA_NListview_NList)) != NULL)
    DoMethod(list, MUIM_FilterPopupList_Popup);

  RETURN(TRUE);
  return TRUE;
}
MakeStaticHook(InitFilterPopupListHook, InitFilterPopupList);

///
/// SearchOptFromFilterPopup
//  Gets search options from selected filter
HOOKPROTONHNP(SearchOptFromFilterPopup, void, Object *listview)
{
  Object *list;

  ENTER();

  if((list = (Object *)xget(listview, MUIA_NListview_NList)) != NULL)
  {
    struct FilterNode *filter;

    // get the currently active filter
    DoMethod(list, MUIM_NList_GetEntry, MUIV_NList_GetEntry_Active, &filter);
    if(filter != NULL)
    {
      struct RuleNode *rule;

      if((rule = GetFilterRule(filter, 0)) != NULL)
        DoMethod(G->FI->GUI.GR_SEARCH, MUIM_SearchControlGroup_GetFromRule, rule);
    }
  }

  LEAVE();
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
      MUIA_HelpNode, "Windows#Searchwindow",
      MUIA_Window_ID, MAKE_ID('F','I','N','D'),
      WindowContents, VGroup,
        Child, HGroup,
          GroupSpacing(0),
          MUIA_VertWeight, 1,
          Child, VGroup, GroupFrameT(tr(MSG_FI_FindIn)),
            MUIA_HorizWeight, 1,
            Child, NListviewObject,
              MUIA_CycleChain, 1,
              MUIA_NListview_NList, data->GUI.LV_FOLDERS = FolderRequestListtreeObject,
                InputListFrame,
                MUIA_NList_AutoVisible, TRUE,
                MUIA_NList_AdjustWidth, TRUE,
                MUIA_NListtree_MultiSelect, TRUE,
              End,
            End,
            Child, HGroup,
              Child, bt_all = MakeButton(tr(MSG_FI_AllFolders)),
              Child, bt_none = MakeButton(tr(MSG_FI_NOFOLDERS)),
            End,
          End,
          Child, NBalanceObject,
            MUIA_ObjectID, MAKE_ID('B','0','0','5'),
            MUIA_Balance_Quiet, TRUE,
          End,
          Child, VGroup, GroupFrameT(tr(MSG_FI_FindWhat)),
            MUIA_HorizWeight, 10,
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
                MUIA_Popobject_Object, lv_fromrule = NListviewObject,
                  MUIA_NListview_NList, FilterPopupListObject,
                  End,
                End,
              End,
              Child, bt_torule = MakeButton(tr(MSG_FI_AddAsFilter)),
            End,
            Child, VSpace(0),
          End,
        End,
        Child, NBalanceObject,
          MUIA_ObjectID, MAKE_ID('B','0','0','6'),
          MUIA_Balance_Quiet, TRUE,
        End,
        Child, VGroup, GroupFrameT(tr(MSG_FI_Results)),
          MUIA_VertWeight, 100,
          Child, NListviewObject,
            MUIA_CycleChain, TRUE,
            MUIA_NListview_NList, data->GUI.LV_MAILS = MainMailListObject,
              //MUIA_ObjectID,       MAKE_ID('N','L','0','3'),
              MUIA_ContextMenu,    NULL,
              MUIA_NList_Format,   "COL=8 W=-1 MIW=-1 PCS=C BAR, COL=1 W=35 PCS=R BAR, COL=3 W=55 PCS=R BAR, COL=4 W=-1 MIW=-1 BAR, COL=7 W=-1 MIW=-1 BAR, COL=5 W=10 MIW=-1 P=\33r BAR",
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
      DoMethod(bt_all,                   MUIM_Notify, MUIA_Pressed,              FALSE,          data->GUI.LV_FOLDERS   , 5, MUIM_NListtree_Select, MUIV_NListtree_Select_All, MUIV_NListtree_Select_On, MUIF_NONE, NULL);
      DoMethod(bt_none,                  MUIM_Notify, MUIA_Pressed,              FALSE,          data->GUI.LV_FOLDERS   , 5, MUIM_NListtree_Select, MUIV_NListtree_Select_All, MUIV_NListtree_Select_Off, MUIF_NONE, NULL);
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
