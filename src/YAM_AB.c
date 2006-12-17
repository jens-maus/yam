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

***************************************************************************/

#include <ctype.h>
#include <stdlib.h>
#include <string.h>

#include <clib/alib_protos.h>
#include <dos/datetime.h>
#include <libraries/asl.h>
#include <libraries/iffparse.h>
#include <libraries/gadtools.h>
#include <mui/BetterString_mcc.h>
#include <mui/NList_mcc.h>
#include <mui/NListview_mcc.h>
#include <proto/intuition.h>
#include <proto/muimaster.h>
#include <proto/utility.h>

#include "extrasrc.h"

#include "SDI_hook.h"

#include "YAM.h"
#include "YAM_addressbook.h"
#include "YAM_addressbookEntry.h"
#include "YAM_config.h"
#include "YAM_error.h"
#include "YAM_global.h"
#include "YAM_locale.h"
#include "YAM_main.h"
#include "YAM_utilities.h"
#include "YAM_write.h"
#include "classes/Classes.h"

#include "Debug.h"

/* local protos */
static STACKEXT BOOL AB_FindTodaysBirthdates(struct MUI_NListtree_TreeNode*, long);
static STACKEXT void AB_SaveTreeNode(FILE*, struct MUI_NListtree_TreeNode *);
static void AB_PrintField(FILE*, const char*, const char*);
static void AB_PrintShortEntry(FILE*, struct ABEntry*);
static void AB_PrintLongEntry(FILE*, struct ABEntry*);
static STACKEXT void AB_PrintLevel(struct MUI_NListtree_TreeNode*, FILE*, int);


/***************************************************************************
 Module: Address book
***************************************************************************/

/// AB_PrettyPrintAddress
STRPTR AB_PrettyPrintAddress(struct ABEntry *e)
{
  return AB_PrettyPrintAddress2(e->RealName, e->Address);
}

///
/// AB_PrettyPrintAddress2
STRPTR AB_PrettyPrintAddress2(STRPTR realname, STRPTR address)
{
  static char buf[SIZE_REALNAME + SIZE_ADDRESS + 4];
  snprintf(buf, sizeof(buf), "%." STR(SIZE_REALNAME) "s <%." STR(SIZE_ADDRESS) "s>", realname, address);
  return buf;
}

///
/// AB_GotoEntry
//  Searches an entry by alias and activates it
APTR AB_GotoEntry(char *alias)
{
  struct MUI_NListtree_TreeNode *tn = NULL;

  ENTER();

  if(alias != NULL)
  {
    if((tn = (struct MUI_NListtree_TreeNode *)DoMethod(G->AB->GUI.LV_ADDRESSES, MUIM_NListtree_FindName, MUIV_NListtree_FindName_ListNode_Root, alias, MUIF_NONE)) != NULL)
    {
      DoMethod(G->AB->GUI.LV_ADDRESSES, MUIM_NListtree_Open, MUIV_NListtree_Open_ListNode_Parent, tn, MUIF_NONE);
      set(G->AB->GUI.LV_ADDRESSES, MUIA_NListtree_Active, tn);
    }
  }

  RETURN(tn);
  return tn;
}

///
/// AB_ExpandBD
//  Converts date from numeric into textual format
char *AB_ExpandBD(long date)
{
  static char datestr[SIZE_SMALL];

  ENTER();

  // check first if it could be a valid date!
  if(!date || date/1000000 > 31 || date/1000000 < 1 || ((date/10000)%100) > 12 || ((date/10000)%100) < 1 || date%10000 < 1000)
    datestr[0] = '\0';
  else
    snprintf(datestr, sizeof(datestr), "%02ld-%s-%ld", date/1000000, months[((date/10000)%100)-1], date%10000);

  RETURN(datestr);
  return datestr;
}

///
/// AB_CompressBD
//  Connverts date from textual into numeric format
long AB_CompressBD(char *datestr)
{
  long result = 0;
  long m;

  ENTER();

  for(m = 12; m > 0; m--)
  {
    // check for a valid month name
    if(strnicmp(&datestr[3], months[m - 1], 3) == 0)
    {
      long d;

      // check for a valid day
      d = atoi(datestr);
      if(d >= 1 && d <= 31)
      {
        long y;

        // check for a valid year
        y = atoi(&datestr[7]);
        if(y >= 1800 && y <= 2100)
          result = (100 * d + m) * 10000 + y;
      }
    }
  }

  RETURN(result);
  return result;
}

///
/// AB_FindTodaysBirthdates (rec)
//  Recursively searches the address book for a given birth date
static STACKEXT BOOL AB_FindTodaysBirthdates(struct MUI_NListtree_TreeNode *list, long today)
{
  struct MUI_NListtree_TreeNode *tn;
  int wrwin, i;

  ENTER();

  for(i = 0; ; i++)
  {
    if((tn = (struct MUI_NListtree_TreeNode *)DoMethod(G->AB->GUI.LV_ADDRESSES, MUIM_NListtree_GetEntry, list, i, MUIF_NONE)))
    {
      struct ABEntry *ab = tn->tn_User;

      if(ab->Type == AET_USER && ab->BirthDay/10000 == today/10000)
      {
        char question[SIZE_LARGE];
        char *name = *ab->RealName ? ab->RealName : ab->Alias;

        snprintf(question, sizeof(question), GetStr(MSG_AB_BirthdayReq), name, today%10000-ab->BirthDay%10000);

        if(MUI_Request(G->App, G->MA->GUI.WI, 0, GetStr(MSG_AB_BirthdayReminder), GetStr(MSG_YesNoReq), question))
        {
          if((wrwin = MA_NewNew(NULL, 0)) >= 0)
          {
            setstring(G->WR[wrwin]->GUI.ST_TO, ab->Alias);
            setstring(G->WR[wrwin]->GUI.ST_SUBJECT, GetStr(MSG_AB_HappyBirthday));
          }
        }
      }
    }
    else
      break;
  }

  RETURN(TRUE);
  return TRUE;
}

///
/// AB_CheckBirthdates
//  Searches address book for todays birth days
void AB_CheckBirthdates(void)
{
  AB_FindTodaysBirthdates(MUIV_NListtree_GetEntry_ListNode_Root, DateStamp2Long(NULL));
}

///
/// AB_SearchEntry
//  Searches the address book by alias, name or address
//  it will break if there is more then one entry
int AB_SearchEntry(char *text, int mode, struct ABEntry **ab)
{
  struct MUI_NListtree_TreeNode *tn;
  struct ABEntry *ab_found;
  int i;
  int hits = 0;
  BOOL found = FALSE;
  int mode_type = mode&ASM_TYPEMASK;
  LONG tl = strlen(text);

  ENTER();

  // we scan until we are at the end of the list or
  // if we found more then one matching entry
  for(i = 0; hits <= 2; i++, found = FALSE)
  {
    tn = (struct MUI_NListtree_TreeNode *)DoMethod(G->AB->GUI.LV_ADDRESSES, MUIM_NListtree_GetEntry, MUIV_NListtree_GetEntry_ListNode_Root, i, MUIF_NONE);
    if(tn == NULL)
      break;

    // now we set the AB_Entry
    ab_found = tn->tn_User;
    if(!ab_found)
      break;

    // now we check if this entry is one of the not wished entry types
    // and then we skip it.
    if(ab_found->Type == AET_USER  && !isUserSearch(mode))
      continue;
    if(ab_found->Type == AET_LIST  && !isListSearch(mode))
      continue;
    if(ab_found->Type == AET_GROUP && !isGroupSearch(mode))
      continue;

    if(isCompleteSearch(mode))
    {
      // Now we check for the ALIAS->REALNAME->ADDRESS, so only ONE mode is allowed at a time
      if(isAliasSearch(mode_type))
        found = !Strnicmp(ab_found->Alias,    text, tl);
      else if(isRealNameSearch(mode_type))
        found = !Strnicmp(ab_found->RealName, text, tl);
      else if(isAddressSearch(mode_type))
        found = !Strnicmp(ab_found->Address,  text, tl);
    }
    else
    {
      // Now we check for the ALIAS->REALNAME->ADDRESS, so only ONE mode is allowed at a time
      if(isAliasSearch(mode_type))
        found = !Stricmp(ab_found->Alias,   text);
      else if(isRealNameSearch(mode_type))
        found = !Stricmp(ab_found->RealName,text);
      else if(isAddressSearch(mode_type))
        found = !Stricmp(ab_found->Address, text);
    }

    if(found)
    {
      *ab = ab_found;
      hits++;
    }
  }

  RETURN(hits);
  return hits;
}

///
/// AB_CompleteAlias
//  Auto-completes alias or name in recipient field
char *AB_CompleteAlias(char *text)
{
  char *compl = NULL;
  struct ABEntry *ab = NULL;

  ENTER();

  if(AB_SearchEntry(text, ASM_ALIAS|ASM_USER|ASM_LIST|ASM_COMPLETE, &ab) == 1)
  {
    compl = ab->Alias;
  }
  else if(AB_SearchEntry(text, ASM_REALNAME|ASM_USER|ASM_LIST|ASM_COMPLETE, &ab) == 1)
  {
    compl = ab->RealName;
  }
  else if(AB_SearchEntry(text, ASM_ADDRESS|ASM_USER|ASM_LIST|ASM_COMPLETE, &ab) == 1)
  {
    compl = ab->Address;
  }

  if(compl != NULL)
    compl = &compl[strlen(text)];

  RETURN(compl);
  return compl;
}

///
/// AB_InsertAddress
//  Adds a new recipient to a recipient field
void AB_InsertAddress(APTR string, const char *alias, const char *name, const char *address)
{
  char *p;

  ENTER();

  p = (char *)xget(string, MUIA_UserData);
  if(p != NULL)
  {
    p = (char *)xget(string, MUIA_String_Contents);
    if(*p != '\0')
      DoMethod(string, MUIM_BetterString_Insert, ", ", MUIV_BetterString_Insert_EndOfString);
  }
  else
    setstring(string, "");

  if(*alias != '\0')
    DoMethod(string, MUIM_BetterString_Insert, alias, MUIV_BetterString_Insert_EndOfString);
  else
  {
    if(*name != '\0')
    {
      if(strchr(name, ','))
        DoMethod(string, MUIM_BetterString_Insert, "\"", MUIV_BetterString_Insert_EndOfString);
      DoMethod(string, MUIM_BetterString_Insert, name, MUIV_BetterString_Insert_EndOfString);
      if(strchr(name, ','))
        DoMethod(string, MUIM_BetterString_Insert, "\"", MUIV_BetterString_Insert_EndOfString);
    }
    if(*address != '\0')
    {
      if(*name != '\0')
        DoMethod(string, MUIM_BetterString_Insert, " <", MUIV_BetterString_Insert_EndOfString);
      DoMethod(string, MUIM_BetterString_Insert, address, MUIV_BetterString_Insert_EndOfString);
      if(*name != '\0')
        DoMethod(string, MUIM_BetterString_Insert, ">", MUIV_BetterString_Insert_EndOfString);
    }
  }
}

///
/// AB_FromAddrBook
/*** AB_FromAddrBook - Inserts an address book entry into a recipient string ***/
HOOKPROTONHNO(AB_FromAddrBook, void, ULONG *arg)
{
  struct MUI_NListtree_TreeNode *active;

  ENTER();

  if(arg[0] != ABM_NONE &&
     (active = (struct MUI_NListtree_TreeNode *)xget(G->AB->GUI.LV_ADDRESSES, MUIA_NListtree_Active)))
  {
    int winnum = G->AB->WrWin;
    struct ABEntry *addr = (struct ABEntry *)(active->tn_User);
    BOOL openwin = winnum < 0;

    if(!openwin)
      openwin = !G->WR[winnum];

    if(openwin)
      G->AB->WrWin = winnum = MA_NewNew(NULL, 0);

    if(winnum >= 0)
    {
      Object *string;

      switch(arg[0])
      {
        case ABM_TO:      string = G->WR[winnum]->GUI.ST_TO; break;
        case ABM_CC:      string = G->WR[winnum]->GUI.ST_CC; break;
        case ABM_BCC:     string = G->WR[winnum]->GUI.ST_BCC; break;
        case ABM_REPLYTO: string = G->WR[winnum]->GUI.ST_REPLYTO; break;
        case ABM_FROM:    string = G->WR[winnum]->GUI.ST_FROM; break;

        default:
          string = (Object *)arg[0];
        break;
      }

      DoMethod(string, MUIM_Recipientstring_AddRecipient, addr->Alias ? addr->Alias : addr->RealName);
    }
  }

  LEAVE();
}
MakeStaticHook(AB_FromAddrBookHook, AB_FromAddrBook);

///
/// AB_LoadTree
//  Loads the address book from a file
BOOL AB_LoadTree(char *fname, BOOL append, BOOL sorted)
{
  struct ABEntry addr;
  struct MUI_NListtree_TreeNode *parent[8];
  char buffer[SIZE_LARGE];
  FILE *fh;
  int len, nested = 0;
  BOOL result = FALSE;

  ENTER();

  parent[nested] = MUIV_NListtree_Insert_ListNode_Root;

  if((fh = fopen(fname, "r")) != NULL && GetLine(fh, buffer, sizeof(buffer)))
  {
    if(strncmp(buffer,"YAB",3) == 0)
    {
      int version = buffer[3] - '0';

      G->AB->Modified = append;
      if(append == FALSE)
        DoMethod(G->AB->GUI.LV_ADDRESSES, MUIM_NListtree_Clear, NULL, 0);

      set(G->AB->GUI.LV_ADDRESSES, MUIA_NListtree_Quiet, TRUE);

      while(GetLine(fh, buffer, sizeof(buffer)))
      {
        memset(&addr, 0, sizeof(struct ABEntry));
        if(strncmp(buffer, "@USER", 5) == 0)
        {
          addr.Type = AET_USER;
          strlcpy(addr.Alias, Trim(&buffer[6]), sizeof(addr.Alias));
          strlcpy(addr.Address, Trim(GetLine(fh, buffer, sizeof(buffer))), sizeof(addr.Address));
          strlcpy(addr.RealName, Trim(GetLine(fh, buffer, sizeof(buffer))), sizeof(addr.RealName));
          strlcpy(addr.Comment, Trim(GetLine(fh, buffer, sizeof(buffer))), sizeof(addr.Comment));
          if(version > 2)
          {
            strlcpy(addr.Phone, Trim(GetLine(fh, buffer, sizeof(buffer))), sizeof(addr.Phone));
            strlcpy(addr.Street, Trim(GetLine(fh, buffer, sizeof(buffer))), sizeof(addr.Street));
            strlcpy(addr.City, Trim(GetLine(fh, buffer, sizeof(buffer))), sizeof(addr.City));
            strlcpy(addr.Country, Trim(GetLine(fh, buffer, sizeof(buffer))), sizeof(addr.Country));
            strlcpy(addr.PGPId, Trim(GetLine(fh, buffer, sizeof(buffer))), sizeof(addr.PGPId));
            addr.BirthDay = atol(Trim(GetLine(fh, buffer, sizeof(buffer))));
            strlcpy(addr.Photo, Trim(GetLine(fh, buffer, sizeof(buffer))), sizeof(addr.Photo));
            if(strcmp(GetLine(fh, buffer, sizeof(buffer)), "@ENDUSER"))
              strlcpy(addr.Homepage, Trim(buffer), sizeof(addr.Homepage));
          }
          if(version > 3)
          {
            addr.DefSecurity = atoi(Trim(GetLine(fh, buffer, sizeof(buffer))));
          }
          do
          {
            if(strcmp(buffer, "@ENDUSER") == 0)
              break;
          }
          while(GetLine(fh, buffer, sizeof(buffer)));

          DoMethod(G->AB->GUI.LV_ADDRESSES, MUIM_NListtree_Insert, addr.Alias[0] ? addr.Alias : addr.RealName, &addr, parent[nested], sorted ?  MUIV_NListtree_Insert_PrevNode_Sorted : MUIV_NListtree_Insert_PrevNode_Tail, MUIF_NONE);
        }
        else if(strncmp(buffer, "@LIST", 5) == 0)
        {
          char *members;

          addr.Type = AET_LIST;

          strlcpy(addr.Alias, Trim(&buffer[6]), sizeof(addr.Alias));
          if(version > 2)
          {
            strlcpy(addr.Address , Trim(GetLine(fh, buffer, sizeof(buffer))), sizeof(addr.Address));
            strlcpy(addr.RealName, Trim(GetLine(fh, buffer, sizeof(buffer))), sizeof(addr.RealName));
          }
          strlcpy(addr.Comment , Trim(GetLine(fh, buffer, sizeof(buffer))), sizeof(addr.Comment));
          members = AllocStrBuf(SIZE_DEFAULT);
          while(GetLine(fh, buffer, sizeof(buffer)))
          {
            if(strcmp(buffer, "@ENDLIST") == 0)
              break;

            if(*buffer == '\0')
              continue;

            members = StrBufCat(members, buffer);
            members = StrBufCat(members, "\n");
          }
          len = strlen(members)+1;
          addr.Members = malloc(len);
          strlcpy(addr.Members, members, len);
          FreeStrBuf(members);
          DoMethod(G->AB->GUI.LV_ADDRESSES, MUIM_NListtree_Insert, addr.Alias, &addr, parent[nested], sorted ?  MUIV_NListtree_Insert_PrevNode_Sorted : MUIV_NListtree_Insert_PrevNode_Tail, MUIF_NONE);
          free(addr.Members);
        }
        else if(strncmp(buffer, "@GROUP", 6) == 0)
        {
          addr.Type = AET_GROUP;
          strlcpy(addr.Alias  , Trim(&buffer[7]), sizeof(addr.Alias));
          strlcpy(addr.Comment, Trim(GetLine(fh, buffer, sizeof(buffer))), sizeof(addr.Comment));
          nested++;
          parent[nested] = (struct MUI_NListtree_TreeNode *)DoMethod(G->AB->GUI.LV_ADDRESSES, MUIM_NListtree_Insert, addr.Alias, &addr, parent[nested-1], MUIV_NListtree_Insert_PrevNode_Tail, TNF_LIST);
        }
        else if(strcmp(buffer,"@ENDGROUP") == 0)
        {
          nested--;
        }
      }

      set(G->AB->GUI.LV_ADDRESSES, MUIA_NListtree_Quiet, FALSE);

      // no errors happened
      result = TRUE;
    }
    else
    {
      // ask the user if he really wants to read out a non YAM
      // Addressbook file.
      if(MUI_Request(G->App, G->AB->GUI.WI, MUIF_NONE, NULL, GetStr(MSG_AB_NOYAMADDRBOOK_GADS), GetStr(MSG_AB_NOYAMADDRBOOK), fname))
      {
        G->AB->Modified = append;
        if(append == FALSE)
          DoMethod(G->AB->GUI.LV_ADDRESSES, MUIM_NListtree_Clear, NULL, 0);

        fseek(fh, 0, SEEK_SET);
        while(GetLine(fh, buffer, sizeof(buffer)))
        {
          char *p, *p2;

          memset(&addr, 0, sizeof(struct ABEntry));
          if((p = strchr(buffer, ' ')) != NULL)
            *p = '\0';
          strlcpy(addr.Address, buffer, sizeof(addr.Address));
          if(p != NULL)
          {
            strlcpy(addr.RealName, ++p, sizeof(addr.RealName));
            if((p2 = strchr(p, ' ')) != NULL)
               *p2 = '\0';
          }
          else
          {
            p = buffer;
            if((p2 = strchr(p, '@')) != NULL)
               *p2 = '\0';
          }
          strlcpy(addr.Alias, p, sizeof(addr.Alias));
          DoMethod(G->AB->GUI.LV_ADDRESSES, MUIM_NListtree_Insert, addr.Alias, &addr, parent[nested], sorted ?  MUIV_NListtree_Insert_PrevNode_Sorted : MUIV_NListtree_Insert_PrevNode_Tail, MUIF_NONE);
        }
      }

      // no errors happened
      result = TRUE;
    }
    fclose(fh);
  }
  else
  {
    ER_NewError(GetStr(MSG_ER_ADDRBOOKLOAD), fname);
    if(fh != NULL)
       fclose(fh);
  }

  RETURN(result);
  return result;
}

///
/// AB_SaveTreeNode (rec)
//  Recursively saves an address book node
static STACKEXT void AB_SaveTreeNode(FILE *fh, struct MUI_NListtree_TreeNode *list)
{
  struct MUI_NListtree_TreeNode *tn;
  struct ABEntry *ab;
  int i;

  ENTER();

  for(i = 0; ; i++)
  {
    if((tn = (struct MUI_NListtree_TreeNode *)DoMethod(G->AB->GUI.LV_ADDRESSES, MUIM_NListtree_GetEntry, list, i, MUIV_NListtree_GetEntry_Flag_SameLevel)) != NULL)
    {
      ab = tn->tn_User;
      switch (ab->Type)
      {
        case AET_USER:
          fprintf(fh, "@USER %s\n%s\n%s\n%s\n%s\n%s\n%s\n%s\n%s\n%08ld\n%s\n%s\n%d\n@ENDUSER\n", ab->Alias, ab->Address, ab->RealName, ab->Comment,
                      ab->Phone, ab->Street, ab->City, ab->Country, ab->PGPId, ab->BirthDay, ab->Photo, ab->Homepage, ab->DefSecurity);
          break;
        case AET_LIST:
          fprintf(fh, "@LIST %s\n%s\n%s\n%s\n%s\n@ENDLIST\n", ab->Alias, ab->Address, ab->RealName, ab->Comment, ab->Members);
          break;
        case AET_GROUP:
          fprintf(fh, "@GROUP %s\n%s\n", ab->Alias, ab->Comment);
          AB_SaveTreeNode(fh, tn);
          fputs("@ENDGROUP\n", fh);
          break;
      }
    }
    else
      break;
  }

  LEAVE();
}

///
/// AB_SaveTree
//  Saves the address book to a file
BOOL AB_SaveTree(char *fname)
{
  FILE *fh;
  BOOL result = FALSE;

  ENTER();

  if((fh = fopen(fname, "w")) != NULL)
  {
    fputs("YAB4 - YAM Addressbook\n", fh);
    AB_SaveTreeNode(fh, MUIV_NListtree_GetEntry_ListNode_Root);
    fclose(fh);
    AppendLogVerbose(70, GetStr(MSG_LOG_SavingABook), fname);
    result = TRUE;
  }
  else
    ER_NewError(GetStr(MSG_ER_CantCreateFile), fname);

  RETURN(result);
  return result;
}

///
/// AB_ImportTreeLDIF
//  Imports an address book in LDIF format
BOOL AB_ImportTreeLDIF(char *fname, BOOL append, BOOL sorted)
{
  FILE *fh;
  BOOL result = FALSE;

  ENTER();

  if((fh = fopen(fname, "r")) != NULL)
  {
    char buffer[SIZE_LARGE];
    struct ABEntry addr;

    G->AB->Modified = append;
    if(append == FALSE)
      DoMethod(G->AB->GUI.LV_ADDRESSES, MUIM_NListtree_Clear, NULL, 0);

    set(G->AB->GUI.LV_ADDRESSES, MUIA_NListtree_Quiet, TRUE);

    while(GetLine(fh, buffer, sizeof(buffer)))
    {
      // an empty line separates two user entries
      if(buffer[0] == '\0')
      {
        // we need at least an EMail address and a name
        if(addr.Address[0] != '\0' && addr.RealName[0] != '\0')
           DoMethod(G->AB->GUI.LV_ADDRESSES, MUIM_NListtree_Insert, addr.Alias[0] ? addr.Alias : addr.RealName, &addr, MUIV_NListtree_Insert_ListNode_Root, sorted ?  MUIV_NListtree_Insert_PrevNode_Sorted : MUIV_NListtree_Insert_PrevNode_Tail, MUIF_NONE);
      }
      else
      {
        if(strncmp(buffer, "dn:", 3) == 0)
        {
          // this is the very first line a new entry,
          // so clear the structure for further actions now
          memset(&addr, 0, sizeof(struct ABEntry));
          addr.Type = AET_USER;
        }
        else if(strncmp(buffer, "cn:", 3) == 0)
          strlcpy(addr.RealName, Trim(&buffer[3]), sizeof(addr.RealName));
        else if(strncmp(buffer, "mail:", 5) == 0)
          strlcpy(addr.Address, Trim(&buffer[5]), sizeof(addr.Address));
        else if(strncmp(buffer, "mozillaNickname:", 16) == 0)
          strlcpy(addr.Alias, Trim(&buffer[16]), sizeof(addr.Alias));
        else if(strncmp(buffer, "telephoneNumber:", 16) == 0)
        {
          if(addr.Phone[0] != '\0')
            strlcat(addr.Phone, ", ", sizeof(addr.Phone));
          strlcat(addr.Phone, Trim(&buffer[16]), sizeof(addr.Phone));
        }
        else if(strncmp(buffer, "homePhone:", 10) == 0)
        {
          if(addr.Phone[0] != '\0')
            strlcat(addr.Phone, ", ", sizeof(addr.Phone));
          strlcat(addr.Phone, Trim(&buffer[10]), sizeof(addr.Phone));
        }
        else if(strncmp(buffer, "fax:", 4) == 0)
        {
          if(addr.Phone[0] != '\0')
            strlcat(addr.Phone, ", ", sizeof(addr.Phone));
          strlcat(addr.Phone, Trim(&buffer[4]), sizeof(addr.Phone));
        }
        else if(strncmp(buffer, "pager:", 6) == 0)
        {
          if(addr.Phone[0] != '\0')
            strlcat(addr.Phone, ", ", sizeof(addr.Phone));
          strlcat(addr.Phone, Trim(&buffer[6]), sizeof(addr.Phone));
        }
        else if(strncmp(buffer, "mobile:", 7) == 0)
        {
          if(addr.Phone[0] != '\0')
            strlcat(addr.Phone, ", ", sizeof(addr.Phone));
          strlcat(addr.Phone, Trim(&buffer[7]), sizeof(addr.Phone));
        }
        else if(strncmp(buffer, "homeStreet:", 11) == 0)
        {
          if(addr.Street[0] != '\0')
            strlcat(addr.Street, ", ", sizeof(addr.Street));
          strlcat(addr.Street, Trim(&buffer[11]), sizeof(addr.Street));
        }
        else if(strncmp(buffer, "mozillaHomeStreet2:", 19) == 0)
        {
          if(addr.Street[0] != '\0')
            strlcat(addr.Street, ", ", sizeof(addr.Street));
          strlcat(addr.Street, Trim(&buffer[19]), sizeof(addr.Street));
        }
        else if(strncmp(buffer, "l:", 2) == 0)
        {
          if(addr.City[0] != '\0')
            strlcat(addr.City, ", ", sizeof(addr.City));
          strlcat(addr.City, Trim(&buffer[2]), sizeof(addr.City));
        }
        else if(strncmp(buffer, "mozillaHomeLocalityName:", 24) == 0)
        {
          if(addr.City[0] != '\0')
            strlcat(addr.City, ", ", sizeof(addr.City));
          strlcat(addr.City, Trim(&buffer[24]), sizeof(addr.City));
        }
        else if(strncmp(buffer, "postalCode:", 11) == 0)
        {
          if(addr.City[0] != '\0')
            strlcat(addr.City, ", ", sizeof(addr.City));
          strlcat(addr.City, Trim(&buffer[11]), sizeof(addr.City));
        }
        else if(strncmp(buffer, "mozillaHomePostalCode:", 22) == 0)
        {
          if(addr.City[0] != '\0')
            strlcat(addr.City, ", ", sizeof(addr.City));
          strlcat(addr.City, Trim(&buffer[22]), sizeof(addr.City));
        }
        else if(strncmp(buffer, "c:", 2) == 0)
        {
          if(addr.Country != '\0')
            strlcat(addr.Country, ", ", sizeof(addr.Country));
          strlcat(addr.Country, Trim(&buffer[2]), sizeof(addr.Country));
        }
        else if(strncmp(buffer, "mozillaHomeCountryName:", 23) == 0)
        {
          if(addr.Country != '\0')
            strlcat(addr.Country, ", ", sizeof(addr.Country));
          strlcat(addr.Country, Trim(&buffer[23]), sizeof(addr.Country));
        }
        else if(strncmp(buffer, "mozillaWorkUrl:", 15) == 0)
        {
          if(addr.Homepage[0] != '\0')
            strlcat(addr.Homepage, ", ", sizeof(addr.Homepage));
          strlcat(addr.Homepage, Trim(&buffer[15]), sizeof(addr.Homepage));
        }
        else if(strncmp(buffer, "mozillaHomeUrl:", 15) == 0)
        {
          if(addr.Homepage[0] != '\0')
            strlcat(addr.Homepage, ", ", sizeof(addr.Homepage));
          strlcat(addr.Homepage, Trim(&buffer[15]), sizeof(addr.Homepage));
        }
      }

      result = TRUE;
    }

    set(G->AB->GUI.LV_ADDRESSES, MUIA_NListtree_Quiet, FALSE);

    fclose(fh);
  }
  else
     ER_NewError(GetStr(MSG_ER_ADDRBOOKIMPORT), fname);

  RETURN(result);
  return result;
}

///
/// AB_ExportTreeNodeLDIF
//  Exports an address book as LDIF file
static STACKEXT void AB_ExportTreeNodeLDIF(FILE *fh, struct MUI_NListtree_TreeNode *list)
{
  int i;

  ENTER();

  for(i = 0; ; i++)
  {
    struct MUI_NListtree_TreeNode *tn;

    if((tn = (struct MUI_NListtree_TreeNode *)DoMethod(G->AB->GUI.LV_ADDRESSES, MUIM_NListtree_GetEntry, list, i, MUIV_NListtree_GetEntry_Flag_SameLevel)) != NULL)
    {
      struct ABEntry *ab;

      ab = tn->tn_User;
      switch (ab->Type)
      {
        case AET_USER:
          fprintf(fh, "dn: cn=%s,mail=%s\n", ab->RealName, ab->Address);
          fprintf(fh, "objectclass: top\n");
          fprintf(fh, "objectclass: person\n");
          fprintf(fh, "objectclass: organizationalPerson\n");
          fprintf(fh, "objectclass: inetOrgPerson\n");
          fprintf(fh, "objectclass: mozillaAbPersonAlpha\n");
          fprintf(fh, "cn: %s\n", ab->RealName);
          fprintf(fh, "mail: %s\n", ab->Address);
          if(ab->Alias[0] != '\0')
            fprintf(fh, "mozillaNickname: %s\n", ab->Alias);
          if(ab->Phone[0] != '\0')
            fprintf(fh, "telephoneNumber: %s\n", ab->Phone);
          if(ab->Street[0] != '\0')
            fprintf(fh, "street: %s\n", ab->Street);
          if(ab->City[0] != '\0')
            fprintf(fh, "l: %s\n", ab->City);
          if(ab->Country[0] != '\0')
            fprintf(fh, "c: %s\n", ab->Country);
          if(ab->Homepage[0] != '\0')
            fprintf(fh, "mozillaHomeUrl: %s\n", ab->Homepage);
            fprintf(fh, "\n");
          break;

        case AET_GROUP:
          AB_ExportTreeNodeLDIF(fh, tn);
          break;

        default:
          // lists cannot be exported
          break;
      }
    }
    else
      break;
  }

  LEAVE();
}

///
/// AB_ExportTreeLDIF
//  Exports an address book as LDIF file
BOOL AB_ExportTreeLDIF(char *fname)
{
  FILE *fh;
  BOOL result = FALSE;

  ENTER();

  if((fh = fopen(fname, "w")) != NULL)
  {
    AB_ExportTreeNodeLDIF(fh, MUIV_NListtree_GetEntry_ListNode_Root);
    fclose(fh);
    result = TRUE;
  }
  else
    ER_NewError(GetStr(MSG_ER_ADDRBOOKEXPORT), fname);

  RETURN(result);
  return result;
}

///
/// AB_ImportTreeTabCSV
//  Imports an address book with comma or tab separated entries
BOOL AB_ImportTreeTabCSV(char *fname, BOOL append, BOOL sorted, char delim)
{
  FILE *fh;
  BOOL result = FALSE;

  ENTER();

  if((fh = fopen(fname, "r")) != NULL)
  {
    char buffer[SIZE_LARGE];
    char delimStr[2];

    G->AB->Modified = append;
    if(append == FALSE)
      DoMethod(G->AB->GUI.LV_ADDRESSES, MUIM_NListtree_Clear, NULL, 0);

    set(G->AB->GUI.LV_ADDRESSES, MUIA_NListtree_Quiet, TRUE);

    delimStr[0] = delim;
    delimStr[1] = '\0';

    while(GetLine(fh, buffer, sizeof(buffer)))
    {
      struct ABEntry addr;
      char *item = buffer;
      int itemNumber = 0;

      memset(&addr, 0, sizeof(struct ABEntry));
      addr.Type = AET_USER;

      do
      {
        char *next;

        itemNumber++;
        if((next = strpbrk(item, delimStr)) != NULL)
          *next++ = '\0';

        // remove any nonsense like trailing spaces
        item = Trim(item);

        if(item[0] != '\0')
        {
          // Thunderbird 1.5 exports 36 items, let's look which
          switch(itemNumber)
          {
            case 1: // first name
              strlcat(addr.RealName, item, sizeof(addr.RealName));
              break;

            case 2: // last name
              if(addr.RealName[0] != '\0')
                strlcat(addr.RealName, " ", sizeof(addr.RealName));
              strlcat(addr.RealName, item, sizeof(addr.RealName));
              break;

            case 3: // complete name, preferred, if available
              strlcpy(addr.RealName, item, sizeof(addr.RealName));
              break;

            case 4: // nickname
              strlcpy(addr.Alias, item, sizeof(addr.Alias));
              break;

            case 5: // EMail address
              strlcpy(addr.Address, item, sizeof(addr.Address));
              break;

            case 6: // second EMail address, ignored
              break;

            case 7: // office phone number
            case 8: // private phone number
            case 9: // fax number
            case 10: // pager number
            case 11: // mobile phone
              if(addr.Phone[0] != '\0')
                strlcat(addr.Phone, ", ", sizeof(addr.Phone));
              strlcat(addr.Phone, item, sizeof(addr.Phone));
              break;

            case 12: // address, part 1
            case 13: // address, part 2
              if(addr.Street[0] != '\0')
                strlcat(addr.Street, " ", sizeof(addr.Street));
              strlcat(addr.Street, item, sizeof(addr.Street));
              break;

            case 14: // city
              strlcpy(addr.City, item, sizeof(addr.City));
              break;

            case 15: // province, ignored
              break;

            case 16: // ZIP code, append it to the city name
              if(addr.City[0] != '\0')
                strlcat(addr.City, ", ", sizeof(addr.City));
              strlcat(addr.City, item, sizeof(addr.City));
              break;

            case 17: // country
              strlcpy(addr.Country, item, sizeof(addr.Country));
              break;

            case 27: // office web address
            case 28: // private web address
              strlcpy(addr.Homepage, item, sizeof(addr.Homepage));
              break;

            default: // everything else is ignored
              break;
          }
        }

        item = next;
      }
      while(item != NULL);

      // we need at least an EMail address and a name
      if(addr.Address[0] != '\0' && addr.RealName[0] != '\0')
        DoMethod(G->AB->GUI.LV_ADDRESSES, MUIM_NListtree_Insert, addr.Alias[0] ? addr.Alias : addr.RealName, &addr, MUIV_NListtree_Insert_ListNode_Root, sorted ?  MUIV_NListtree_Insert_PrevNode_Sorted : MUIV_NListtree_Insert_PrevNode_Tail, MUIF_NONE);

      result = TRUE;
    }

    set(G->AB->GUI.LV_ADDRESSES, MUIA_NListtree_Quiet, FALSE);

    fclose(fh);
  }
  else
    ER_NewError(GetStr(MSG_ER_ADDRBOOKIMPORT), fname);

  RETURN(result);
  return result;
}

///
/// AB_ExportTreeNodeTabCSV
//  Exports an address book with comma or tab separated entries
static STACKEXT void AB_ExportTreeNodeTabCSV(FILE *fh, struct MUI_NListtree_TreeNode *list, char delim)
{
  int i;

  ENTER();

  for(i = 0; ; i++)
  {
    struct MUI_NListtree_TreeNode *tn;

    if((tn = (struct MUI_NListtree_TreeNode *)DoMethod(G->AB->GUI.LV_ADDRESSES, MUIM_NListtree_GetEntry, list, i, MUIV_NListtree_GetEntry_Flag_SameLevel)) != NULL)
    {
      struct ABEntry *ab;

      ab = tn->tn_User;
      switch (ab->Type)
      {
        case AET_USER:
          fprintf(fh, "%c", delim);
          fprintf(fh, "%c", delim);
          fprintf(fh, "%s%c", ab->RealName, delim);
          fprintf(fh, "%s%c", ab->Alias, delim);
          fprintf(fh, "%s%c", ab->Address, delim);
          fprintf(fh, "%c", delim);
          fprintf(fh, "%c", delim);
          fprintf(fh, "%s%c", ab->Phone, delim);
          fprintf(fh, "%c", delim);
          fprintf(fh, "%c", delim);
          fprintf(fh, "%c", delim);
          fprintf(fh, "%s%c", ab->Street, delim);
          fprintf(fh, "%c", delim);
          fprintf(fh, "%s%c", ab->City, delim);
          fprintf(fh, "%c", delim);
          fprintf(fh, "%c", delim); // ZIP code
          fprintf(fh, "%s%c", ab->Country, delim);
          fprintf(fh, "%c", delim);
          fprintf(fh, "%c", delim);
          fprintf(fh, "%c", delim);
          fprintf(fh, "%c", delim);
          fprintf(fh, "%c", delim);
          fprintf(fh, "%c", delim);
          fprintf(fh, "%c", delim);
          fprintf(fh, "%c", delim);
          fprintf(fh, "%c", delim);
          fprintf(fh, "%c", delim);
          fprintf(fh, "%s%c", ab->Homepage, delim);
          fprintf(fh, "%c", delim);
          fprintf(fh, "%c", delim);
          fprintf(fh, "%c", delim);
          fprintf(fh, "%c", delim);
          fprintf(fh, "%c", delim);
          fprintf(fh, "%c", delim);
          fprintf(fh, "%c", delim);
          fprintf(fh, "\n");
          break;

        case AET_GROUP:
          AB_ExportTreeNodeTabCSV(fh, tn, delim);
          break;

        default:
          // lists cannot be exported
          break;
      }
    }
    else
      break;
  }

  LEAVE();
}

///
/// AB_ExportTreeTabCSV
//  Exports an address book with comma or tab separated entries
BOOL AB_ExportTreeTabCSV(char *fname, char delim)
{
  FILE *fh;
  BOOL result = FALSE;

  ENTER();

  if((fh = fopen(fname, "w")) != NULL)
  {
    AB_ExportTreeNodeTabCSV(fh, MUIV_NListtree_GetEntry_ListNode_Root, delim);
    fclose(fh);
    result = TRUE;
  }
  else
    ER_NewError(GetStr(MSG_ER_ADDRBOOKEXPORT), fname);

  RETURN(result);
  return result;
}

///
/// AB_EditFunc
/*** AB_EditFunc - Modifies selected address book entry ***/
HOOKPROTONHNONP(AB_EditFunc, void)
{
  struct MUI_NListtree_TreeNode *tn;

  ENTER();

  if((tn = (struct MUI_NListtree_TreeNode *)xget(G->AB->GUI.LV_ADDRESSES, MUIA_NListtree_Active)))
  {
    struct ABEntry *ab = (struct ABEntry *)(tn->tn_User);
    int winnum = EA_Init(ab->Type, ab);

    if(winnum >= 0)
      EA_Setup(winnum, ab);
  }

  LEAVE();
}
MakeHook(AB_EditHook, AB_EditFunc);

///
/// AB_DoubleClick
/*** AB_DoubleClick - User double-clicked in the address book ***/
HOOKPROTONHNONP(AB_DoubleClick, void)
{
  ENTER();

  if(G->AB->WrWin >= 0 && G->WR[G->AB->WrWin])
  {
    struct WR_GUIData *gui = &G->WR[G->AB->WrWin]->GUI;
    Object *obj = NULL;

    switch(G->AB->Mode)
    {
      case ABM_TO:      obj = gui->ST_TO;      break;
      case ABM_CC:      obj = gui->ST_CC;      break;
      case ABM_BCC:     obj = gui->ST_BCC;     break;
      case ABM_FROM:    obj = gui->ST_FROM;    break;
      case ABM_REPLYTO: obj = gui->ST_REPLYTO; break;

      default:
        // nothing
      break;
    }

    DoMethod(G->App, MUIM_CallHook, &AB_FromAddrBookHook, obj);
    set(G->AB->GUI.WI, MUIA_Window_CloseRequest, TRUE);
  }
  else
  {
    if(G->AB->Mode == ABM_CONFIG &&
       G->AB->parentStringGadget != NULL)
    {
      struct MUI_NListtree_TreeNode *active;

      if((active = (struct MUI_NListtree_TreeNode *)xget(G->AB->GUI.LV_ADDRESSES, MUIA_NListtree_Active)))
      {
        struct ABEntry *addr = (struct ABEntry *)(active->tn_User);

        DoMethod(G->AB->parentStringGadget, MUIM_Recipientstring_AddRecipient, addr->Alias ? addr->Alias : addr->RealName);
      }

      set(G->AB->GUI.WI, MUIA_Window_CloseRequest, TRUE);
    }
    else
      AB_EditFunc();
  }

  LEAVE();
}
MakeStaticHook(AB_DoubleClickHook, AB_DoubleClick);

///
/// AB_Sort
/*** AB_Sort - Sorts the address book ***/
HOOKPROTONHNO(AB_Sort, void, int *arg)
{
  char fname[SIZE_PATHFILE];

  ENTER();

  strmfp(fname, C->TempDir, ".addressbook.tmp");
  if(AB_SaveTree(fname))
  {
    G->AB->SortBy = *arg;
    AB_LoadTree(fname, FALSE, TRUE);
    remove(fname);
    G->AB->Modified = TRUE;
  }

  LEAVE();
}
MakeStaticHook(AB_SortHook, AB_Sort);

///
/// AB_NewABookFunc
/*** AB_NewABookFunc - Clears entire address book ***/
HOOKPROTONHNONP(AB_NewABookFunc, void)
{
  ENTER();

  DoMethod(G->AB->GUI.LV_ADDRESSES, MUIM_NListtree_Remove, MUIV_NListtree_Remove_ListNode_Root, MUIV_NListtree_Remove_TreeNode_All, MUIF_NONE);
  G->AB->Modified = FALSE;

  LEAVE();
}
MakeStaticHook(AB_NewABookHook, AB_NewABookFunc);

///
/// AB_OpenABookFunc
/*** AB_OpenABookFunc - Loads selected address book ***/
HOOKPROTONHNONP(AB_OpenABookFunc, void)
{
  struct FileReqCache *frc;

  ENTER();

  if((frc = ReqFile(ASL_ABOOK,G->AB->GUI.WI, GetStr(MSG_Open), REQF_NONE, G->MA_MailDir, "")))
  {
    strmfp(G->AB_Filename, frc->drawer, frc->file);
    AB_LoadTree(G->AB_Filename, FALSE, FALSE);
  }

  LEAVE();
}
MakeStaticHook(AB_OpenABookHook, AB_OpenABookFunc);

///
/// AB_AppendABookFunc
/*** AB_AppendABookFunc - Appends selected address book ***/
HOOKPROTONHNONP(AB_AppendABookFunc, void)
{
  struct FileReqCache *frc;

  ENTER();

  if((frc = ReqFile(ASL_ABOOK,G->AB->GUI.WI, GetStr(MSG_Append), REQF_NONE, G->MA_MailDir, "")))
  {
    char aname[SIZE_PATHFILE];

    strmfp(aname, frc->drawer, frc->file);
    AB_LoadTree(aname, TRUE, FALSE);
  }

  LEAVE();
}
MakeStaticHook(AB_AppendABookHook, AB_AppendABookFunc);

///
/// AB_ImportLDIFABookFunc
/*** AB_ImportLDIFABookFunc - Imports LDIF address book ***/
HOOKPROTONHNONP(AB_ImportLDIFABookFunc, void)
{
  struct FileReqCache *frc;

  ENTER();

  if((frc = ReqFile(ASL_ABOOK_LDIF, G->AB->GUI.WI, GetStr(MSG_AB_IMPORT), REQF_NONE, G->MA_MailDir, "")) != NULL)
  {
    char ldifname[SIZE_PATHFILE];

    strmfp(ldifname, frc->drawer, frc->file);
    AB_ImportTreeLDIF(ldifname, TRUE, FALSE);
  }

  LEAVE();
}
MakeStaticHook(AB_ImportLDIFABookHook, AB_ImportLDIFABookFunc);

///
/// AB_ExportLDIFABookFunc
/*** AB_ExportLDIFABookFunc - Imports LDIF address book ***/
HOOKPROTONHNONP(AB_ExportLDIFABookFunc, void)
{
  struct FileReqCache *frc;

  ENTER();

  if((frc = ReqFile(ASL_ABOOK_LDIF, G->AB->GUI.WI, GetStr(MSG_AB_EXPORT), REQF_SAVEMODE, G->MA_MailDir, "")) != NULL)
  {
    char ldifname[SIZE_PATHFILE];

    strmfp(ldifname, frc->drawer, frc->file);
    AB_ExportTreeLDIF(ldifname);
  }

  LEAVE();
}
MakeStaticHook(AB_ExportLDIFABookHook, AB_ExportLDIFABookFunc);

///
/// AB_ImportTabCSVABookFunc
/*** AB_ImportTabCSVABookFunc - Imports comma or TAB separated address book ***/
HOOKPROTONHNO(AB_ImportTabCSVABookFunc, void, int *arg)
{
  char delim = (char)arg[0];
  int type;
  struct FileReqCache *frc;

  ENTER();

  switch(delim)
  {
    case '\t': type = ASL_ABOOK_TAB; break;
    case ',':  type = ASL_ABOOK_CSV; break;
    default:   type = ASL_ABOOK;     break;
  }

  if((frc = ReqFile(type, G->AB->GUI.WI, GetStr(MSG_AB_IMPORT), REQF_NONE, G->MA_MailDir, "")) != NULL)
  {
    char aname[SIZE_PATHFILE];

    strmfp(aname, frc->drawer, frc->file);
    AB_ImportTreeTabCSV(aname, TRUE, FALSE, delim);
  }

  LEAVE();
}
MakeStaticHook(AB_ImportTabCSVABookHook, AB_ImportTabCSVABookFunc);

///
/// AB_ExportTabCSVABookFunc
/*** AB_ExportTabCSVABookFunc - Exports comma or TAB separated address book ***/
HOOKPROTONHNO(AB_ExportTabCSVABookFunc, void, int *arg)
{
  char delim = (char)arg[0];
  int type;
  struct FileReqCache *frc;

  ENTER();

  switch(delim)
  {
    case '\t': type = ASL_ABOOK_TAB; break;
    case ',':  type = ASL_ABOOK_CSV; break;
    default:   type = ASL_ABOOK;     break;
  }

  if((frc = ReqFile(type, G->AB->GUI.WI, GetStr(MSG_AB_EXPORT), REQF_SAVEMODE, G->MA_MailDir, "")) != NULL)
  {
    char aname[SIZE_PATHFILE];

    strmfp(aname, frc->drawer, frc->file);
    AB_ExportTreeTabCSV(aname, delim);
  }

  LEAVE();
}

MakeStaticHook(AB_ExportTabCSVABookHook, AB_ExportTabCSVABookFunc);

///
/// AB_SaveABookFunc
/*** AB_SaveABookFunc - Saves address book using the default name ***/
HOOKPROTONHNONP(AB_SaveABookFunc, void)
{
  ENTER();

  Busy(GetStr(MSG_BusySavingAB), G->AB_Filename, 0, 0);
  AB_SaveTree(G->AB_Filename);
  G->AB->Modified = FALSE;
  BusyEnd();

  LEAVE();
}
MakeHook(AB_SaveABookHook, AB_SaveABookFunc);

///
/// AB_SaveABookAsFunc
/*** AB_SaveABookAsFunc - Saves address book under a different name ***/
HOOKPROTONHNONP(AB_SaveABookAsFunc, void)
{
  struct FileReqCache *frc;

  ENTER();

  if((frc = ReqFile(ASL_ABOOK,G->AB->GUI.WI, GetStr(MSG_SaveAs), REQF_SAVEMODE, G->MA_MailDir, "")))
  {
    strmfp(G->AB_Filename, frc->drawer, frc->file);
    AB_SaveABookFunc();
  }

  LEAVE();
}
MakeStaticHook(AB_SaveABookAsHook, AB_SaveABookAsFunc);

///
/// AB_PrintField
//  Formats and prints a single field
static void AB_PrintField(FILE *prt, const char *fieldname, const char *field)
{
  ENTER();

  if(*field != '\0')
    fprintf(prt, "%-20.20s: %-50.50s\n", StripUnderscore(fieldname), field);

  LEAVE();
}

///
/// AB_PrintShortEntry
//  Prints an address book entry in compact format
static void AB_PrintShortEntry(FILE *prt, struct ABEntry *ab)
{
  static const char types[3] = { 'P','L','G' };

  ENTER();

  fprintf(prt, "%c %-12.12s %-20.20s %-36.36s\n", types[ab->Type-AET_USER],
     ab->Alias, ab->RealName, ab->Type == AET_USER ? ab->Address : ab->Comment);

  LEAVE();
}

///
/// AB_PrintLongEntry
//  Prints an address book entry in detailed format
static void AB_PrintLongEntry(FILE *prt, struct ABEntry *ab)
{
  ENTER();

  fputs("------------------------------------------------------------------------\n", prt);
  switch (ab->Type)
  {
    case AET_USER:
      AB_PrintField(prt, GetStr(MSG_AB_PersonAlias), ab->Alias);
      AB_PrintField(prt, GetStr(MSG_EA_RealName), ab->RealName);
      AB_PrintField(prt, GetStr(MSG_EA_EmailAddress), ab->Address);
      AB_PrintField(prt, GetStr(MSG_EA_PGPId), ab->PGPId);
      AB_PrintField(prt, GetStr(MSG_EA_Homepage), ab->Homepage);
      AB_PrintField(prt, GetStr(MSG_EA_Street), ab->Street);
      AB_PrintField(prt, GetStr(MSG_EA_City), ab->City);
      AB_PrintField(prt, GetStr(MSG_EA_Country), ab->Country);
      AB_PrintField(prt, GetStr(MSG_EA_Phone), ab->Phone);
      AB_PrintField(prt, GetStr(MSG_EA_DOB), AB_ExpandBD(ab->BirthDay));
      break;
    case AET_LIST:
      AB_PrintField(prt, GetStr(MSG_AB_ListAlias), ab->Alias);
      AB_PrintField(prt, GetStr(MSG_EA_MLName), ab->RealName);
      AB_PrintField(prt, GetStr(MSG_EA_ReturnAddress), ab->Address);
      if(ab->Members != NULL)
      {
        BOOL header = FALSE;
        char *ptr;

        for(ptr = ab->Members; *ptr; ptr++)
        {
          char *nptr = strchr(ptr, '\n');

          if(nptr != NULL)
            *nptr = 0;
          else
            break;
          if(!header)
          {
            AB_PrintField(prt, GetStr(MSG_EA_Members), ptr);
            header = TRUE;
          }
          else
            fprintf(prt, "                      %s\n", ptr);
          *nptr = '\n';
          ptr = nptr;
        }
      }
      break;
    case AET_GROUP:
      AB_PrintField(prt, GetStr(MSG_AB_GroupAlias), ab->Alias);
  }
  AB_PrintField(prt, GetStr(MSG_EA_Description), ab->Comment);

  LEAVE();
}

///
/// AB_PrintLevel (rec)
//  Recursively prints an address book node
static STACKEXT void AB_PrintLevel(struct MUI_NListtree_TreeNode *list, FILE *prt, int mode)
{
  struct MUI_NListtree_TreeNode *tn;
  int i;

  ENTER();

  for(i = 0; ; i++)
  {
    if((tn = (struct MUI_NListtree_TreeNode *)DoMethod(G->AB->GUI.LV_ADDRESSES, MUIM_NListtree_GetEntry, list, i, MUIV_NListtree_GetEntry_Flag_SameLevel)) != NULL)
    {
      struct ABEntry *ab = tn->tn_User;

      if(mode == 1)
        AB_PrintLongEntry(prt, ab); else AB_PrintShortEntry(prt, ab);
      if(ab->Type == AET_GROUP)
         AB_PrintLevel(tn, prt, mode);
    }
    else
      break;
  }

  LEAVE();
}

///
/// AB_PrintABookFunc
/*** AB_PrintABookFunc - Prints the entire address book in compact or detailed format ***/
HOOKPROTONHNONP(AB_PrintABookFunc, void)
{
  int mode;

  ENTER();

  mode = MUI_Request(G->App, G->AB->GUI.WI, 0, GetStr(MSG_Print), GetStr(MSG_AB_PrintReqGads), GetStr(MSG_AB_PrintReq));
  if(mode != 0)
  {
    if(C->PrinterCheck && CheckPrinter())
    {
      FILE *prt;

      if((prt = fopen("PRT:", "w")) != NULL)
      {
        Busy(GetStr(MSG_BusyPrintingAB), "", 0, 0);
        fprintf(prt, "%s\n", G->AB_Filename);
        if(mode == 2)
        {
          fprintf(prt, "\n  %-12.12s %-20.20s %s/%s\n", GetStr(MSG_AB_AliasFld), GetStr(MSG_EA_RealName), GetStr(MSG_EA_EmailAddress), GetStr(MSG_EA_Description));
          fputs("------------------------------------------------------------------------\n", prt);
        }
        AB_PrintLevel(MUIV_NListtree_GetEntry_ListNode_Root, prt, mode);
        fclose(prt);
        BusyEnd();
      }
    }
  }

  LEAVE();
}
MakeStaticHook(AB_PrintABookHook, AB_PrintABookFunc);

///
/// AB_PrintFunc
/*** AB_PrintFunc - Prints selected address book entry in detailed format ***/
HOOKPROTONHNONP(AB_PrintFunc, void)
{
  struct MUI_NListtree_TreeNode *tn;

  ENTER();

  if((tn = (struct MUI_NListtree_TreeNode *)xget(G->AB->GUI.LV_ADDRESSES, MUIA_NListtree_Active)) != NULL)
  {
    if(C->PrinterCheck && CheckPrinter())
    {
      FILE *prt;

      if((prt = fopen("PRT:", "w")) != NULL)
      {
         struct ABEntry *ab = (struct ABEntry *)(tn->tn_User);

         set(G->App, MUIA_Application_Sleep, TRUE);
         AB_PrintLongEntry(prt, ab);
         if(ab->Type == AET_GROUP)
           AB_PrintLevel(tn, prt, 1);
         fclose(prt);
         set(G->App, MUIA_Application_Sleep, FALSE);
      }
    }
  }

  LEAVE();
}
MakeHook(AB_PrintHook, AB_PrintFunc);

///
/// AB_AddEntryFunc
/*** AB_AddEntryFunc - Add a new entry to the address book ***/
HOOKPROTONHNO(AB_AddEntryFunc, void, int *arg)
{
  ENTER();

  EA_Init(*arg, NULL);

  LEAVE();
}
MakeHook(AB_AddEntryHook, AB_AddEntryFunc);

///
/// AB_DeleteFunc
/*** AB_DeleteFunc - Deletes selected address book entry ***/
HOOKPROTONHNONP(AB_DeleteFunc, void)
{
  ENTER();

  DoMethod(G->AB->GUI.LV_ADDRESSES, MUIM_NListtree_Remove, NULL, MUIV_NListtree_Remove_TreeNode_Active, MUIF_NONE);
  G->AB->Modified = TRUE;

  LEAVE();
}
MakeHook(AB_DeleteHook, AB_DeleteFunc);

///
/// AB_DuplicateFunc
/*** AB_DuplicateFunc - Duplicates selected address book entry ***/
HOOKPROTONHNONP(AB_DuplicateFunc, void)
{
  struct MUI_NListtree_TreeNode *tn;

  ENTER();

  if((tn = (struct MUI_NListtree_TreeNode *)xget(G->AB->GUI.LV_ADDRESSES, MUIA_NListtree_Active)) != NULL)
  {
    struct ABEntry *ab = (struct ABEntry *)(tn->tn_User);
    int winnum = EA_Init(ab->Type, NULL);

    if(winnum >= 0)
    {
      char buf[SIZE_NAME];
      int len;

      EA_Setup(winnum, ab);
      strlcpy(buf, ab->Alias, sizeof(buf));
      if((len = strlen(buf)) > 0)
      {
        if(isdigit(buf[len - 1]))
          buf[len - 1]++;
        else if(len < SIZE_NAME - 1)
          strlcat(buf, "2", sizeof(buf));
        else
          buf[len - 1] = '2';

        setstring(G->EA[winnum]->GUI.ST_ALIAS, buf);
      }
    }
  }

  LEAVE();
}
MakeStaticHook(AB_DuplicateHook, AB_DuplicateFunc);

///
/// AB_FindEntry
// Searches an address book node for a given pattern
int AB_FindEntry(char *pattern, enum AddressbookFind mode, char **result)
{
  Object *lv = G->AB->GUI.LV_ADDRESSES;
  int res = 0;
  int i;

  ENTER();

  for(i = 0; ; i++)
  {
    struct MUI_NListtree_TreeNode *tn;

    if((tn = (struct MUI_NListtree_TreeNode *)DoMethod(lv, MUIM_NListtree_GetEntry, MUIV_NListtree_GetEntry_ListNode_Root, i, MUIF_NONE)))
    {
      struct ABEntry *ab = tn->tn_User;

      if(ab->Type == AET_GROUP)
        continue;
      else
      {
        BOOL found = FALSE;
        int winnum;

        switch(mode)
        {
          case ABF_RX_NAME:
            found = MatchNoCase(ab->RealName, pattern);
          break;

          case ABF_RX_EMAIL:
            found = MatchNoCase(ab->Address, pattern);
          break;

          case ABF_RX_NAMEEMAIL:
            found = MatchNoCase(ab->RealName, pattern) | MatchNoCase(ab->Address, pattern);
          break;

          default:
          {
            if((found = MatchNoCase(ab->Alias, pattern) | MatchNoCase(ab->Comment, pattern)) == FALSE)
            {
              if((found = MatchNoCase(ab->RealName, pattern) | MatchNoCase(ab->Address, pattern)) == FALSE && ab->Type == AET_USER)
              {
                found = MatchNoCase(ab->Homepage, pattern)|
                        MatchNoCase(ab->Street, pattern)  |
                        MatchNoCase(ab->City, pattern)    |
                        MatchNoCase(ab->Country, pattern) |
                        MatchNoCase(ab->Phone, pattern);
              }
            }
          }
        }

        if(found)
        {
          res++;

          if(mode == ABF_USER)
          {
            char buf[SIZE_LARGE];

            DoMethod(lv, MUIM_NListtree_Open, MUIV_NListtree_Open_ListNode_Parent, tn, MUIF_NONE);
            set(lv, MUIA_NListtree_Active, tn);

            snprintf(buf, sizeof(buf), GetStr(MSG_AB_FoundEntry), ab->Alias, ab->RealName);

            switch(MUI_Request(G->App, G->AB->GUI.WI, 0, GetStr(MSG_AB_FindEntry), GetStr(MSG_AB_FoundEntryGads), buf))
            {
              case 1:
                // nothing
              break;

              case 2:
              {
                if((winnum = EA_Init(ab->Type, ab)) >= 0)
                  EA_Setup(winnum, ab);
              }
              // continue

              case 0:
              {
                RETURN(-1);
                return -1;
              }
            }
          }
          else if(result)
            *result++ = ab->Alias;
        }
      }
    }
    else
      break;
  }

  RETURN(res);
  return res;
}

///
/// AB_FindFunc
/*** AB_FindFunc - Searches address book ***/
HOOKPROTONHNONP(AB_FindFunc, void)
{
  static char pattern[SIZE_PATTERN+1] = "";

  ENTER();

  if(StringRequest(pattern, SIZE_PATTERN, GetStr(MSG_AB_FindEntry), GetStr(MSG_AB_FindEntryReq), GetStr(MSG_AB_StartSearch), NULL, GetStr(MSG_Cancel), FALSE, G->AB->GUI.WI))
  {
    char searchPattern[SIZE_PATTERN+5];

    snprintf(searchPattern, sizeof(searchPattern), "#?%s#?", pattern);

    if(AB_FindEntry(searchPattern, ABF_USER, NULL) == 0)
      MUI_Request(G->App, G->AB->GUI.WI, 0, GetStr(MSG_AB_FindEntry), GetStr(MSG_OkayReq), GetStr(MSG_AB_NoneFound));
  }

  LEAVE();
}
MakeHook(AB_FindHook, AB_FindFunc);

///
/// AB_FoldUnfoldFunc
HOOKPROTONHNO(AB_FoldUnfoldFunc, void, int *arg)
{
  ENTER();

  if(G->AB->GUI.LV_ADDRESSES)
  {
    if(*arg <= 0)
      DoMethod(G->AB->GUI.LV_ADDRESSES, MUIM_NListtree_Open, MUIV_NListtree_Open_ListNode_Root, MUIV_NListtree_Open_TreeNode_All, MUIF_NONE);
    else
      DoMethod(G->AB->GUI.LV_ADDRESSES, MUIM_NListtree_Close, MUIV_NListtree_Close_ListNode_Root, MUIV_NListtree_Close_TreeNode_All, MUIF_NONE);
  }

  LEAVE();
}
MakeHook(AB_FoldUnfoldHook, AB_FoldUnfoldFunc);

///
/// AB_OpenFunc
/*** AB_OpenFunc - Open address book window ***/
HOOKPROTONHNO(AB_OpenFunc, void, LONG *arg)
{
  struct AB_ClassData *ab = G->AB;
  const char *md = "";

  ENTER();

  switch((ab->Mode = arg[0]))
  {
    case ABM_TO:      md = "(To)";      break;
    case ABM_CC:      md = "(CC)";      break;
    case ABM_BCC:     md = "(BCC)";     break;
    case ABM_FROM:    md = "(From)";    break;
    case ABM_REPLYTO: md = "(Reply-To)";break;
    case ABM_CONFIG:  ab->parentStringGadget = (Object *)arg[1]; break;
    default:
      // nothing
    break;
  }

  ab->WrWin = *md ? arg[1] : -1;
  ab->Modified = FALSE;

  // disable some GUI components if necessary.
  set(ab->GUI.BT_TO, MUIA_Disabled, ab->Mode == ABM_CONFIG);
  set(ab->GUI.BT_CC, MUIA_Disabled, ab->Mode == ABM_CONFIG);
  set(ab->GUI.BT_BCC,MUIA_Disabled, ab->Mode == ABM_CONFIG);

  snprintf(ab->WTitle, sizeof(ab->WTitle), "%s %s", GetStr(MSG_MA_MAddrBook), md);
  set(ab->GUI.WI, MUIA_Window_Title, ab->WTitle);
  set(ab->GUI.LV_ADDRESSES, MUIA_NListtree_Active, MUIV_NListtree_Active_Off);

  SafeOpenWindow(ab->GUI.WI);

  LEAVE();
}
MakeHook(AB_OpenHook, AB_OpenFunc);

///
/// AB_Close
/*** AB_Close - Closes address book window ***/
HOOKPROTONHNONP(AB_Close, void)
{
  BOOL closeWin = TRUE;

  ENTER();

  if(G->AB->Modified)
  {
    switch(MUI_Request(G->App, G->AB->GUI.WI, 0, NULL, GetStr(MSG_AB_ModifiedGads), GetStr(MSG_AB_Modified)))
    {
      case 0: closeWin = FALSE; break;
      case 1: AB_SaveABookFunc(); break;
      case 2: break;
      case 3: AB_LoadTree(G->AB_Filename, FALSE, FALSE); break;
    }
  }

  if(closeWin)
    set(G->AB->GUI.WI, MUIA_Window_Open, FALSE);

  LEAVE();
}
MakeStaticHook(AB_CloseHook, AB_Close);

///
/// AB_LV_ConFunc
/*** AB_LV_ConFunc - Address book listview construction hook ***/
HOOKPROTONHNO(AB_LV_ConFunc, struct ABEntry *, struct MUIP_NListtree_ConstructMessage *msg)
{
  struct ABEntry *entry;

  ENTER();

  entry = malloc(sizeof(struct ABEntry));
  if(entry != NULL && msg != NULL)
  {
    struct ABEntry *addr = (struct ABEntry *)msg->UserData;

    memcpy(entry, addr, sizeof(struct ABEntry));
    if(addr->Members)
      entry->Members = strdup(addr->Members);
  }

  RETURN(entry);
  return entry;
}
MakeStaticHook(AB_LV_ConFuncHook, AB_LV_ConFunc);

///
/// AB_LV_DesFunc
/*** AB_LV_DesFunc - Address book listview destruction hook ***/
HOOKPROTONHNO(AB_LV_DesFunc, long, struct MUIP_NListtree_DestructMessage *msg)
{
  struct ABEntry *entry;

  ENTER();

  if(msg != NULL)
  {
    entry = (struct ABEntry *)msg->UserData;

    if(entry != NULL)
    {
      if(entry->Members)
        free(entry->Members);
      free(entry);
    }
  }

  RETURN(0);
  return 0;
}
MakeStaticHook(AB_LV_DesFuncHook, AB_LV_DesFunc);

///
/// AB_LV_DspFunc
/*** AB_LV_DspFunc - Address book listview display hook ***/
HOOKPROTONHNO(AB_LV_DspFunc, long, struct MUIP_NListtree_DisplayMessage *msg)
{
  ENTER();

  if(msg != NULL)
  {
    if(msg->TreeNode)
    {
      struct ABEntry *entry = msg->TreeNode->tn_User;

      if(entry != NULL)
      {
        msg->Array[0] = entry->Alias;
        msg->Array[1] = entry->RealName;
        msg->Array[2] = entry->Comment;
        msg->Array[3] = entry->Address;
        msg->Array[4] = entry->Street;
        msg->Array[5] = entry->City;
        msg->Array[6] = entry->Country;
        msg->Array[7] = entry->Phone;
        msg->Array[8] = AB_ExpandBD(entry->BirthDay);
        msg->Array[9] = entry->PGPId;
        msg->Array[10]= entry->Homepage;

        switch (entry->Type)
        {
           case AET_LIST:
           {
             static char dispal[SIZE_DEFAULT];
             snprintf(msg->Array[0] = dispal, sizeof(dispal), "\033o[0]%s", entry->Alias);
           }
           break;

           case AET_GROUP:
           {
             msg->Preparse[0] = (char *)MUIX_B;
             msg->Preparse[2] = (char *)MUIX_B;
           }
           break;

           default:
             // nothing
           break;
        }
      }
    }
    else
    {
      msg->Array[0] = (STRPTR)GetStr(MSG_AB_TitleAlias);
      msg->Array[1] = (STRPTR)GetStr(MSG_AB_TitleName);
      msg->Array[2] = (STRPTR)GetStr(MSG_AB_TitleDescription);
      msg->Array[3] = (STRPTR)GetStr(MSG_AB_TitleAddress);
      msg->Array[4] = (STRPTR)GetStr(MSG_AB_TitleStreet);
      msg->Array[5] = (STRPTR)GetStr(MSG_AB_TitleCity);
      msg->Array[6] = (STRPTR)GetStr(MSG_AB_TitleCountry);
      msg->Array[7] = (STRPTR)GetStr(MSG_AB_TitlePhone);
      msg->Array[8] = (STRPTR)GetStr(MSG_AB_TitleBirthDate);
      msg->Array[9] = (STRPTR)GetStr(MSG_AB_TitlePGPId);
      msg->Array[10]= (STRPTR)GetStr(MSG_AB_TitleHomepage);
    }
  }

  RETURN(0);
  return 0;
}
MakeHook(AB_LV_DspFuncHook, AB_LV_DspFunc);

///
/// AB_LV_CmpFunc
/*** AB_LV_CmpFunc - Address book listview compare hook ***/
HOOKPROTONHNO(AB_LV_CmpFunc, long, struct MUIP_NListtree_CompareMessage *msg)
{
  struct MUI_NListtree_TreeNode *entry1, *entry2;
  struct ABEntry *ab1, *ab2;
  char *n1, *n2;
  int cmp;

  if(msg == NULL)
    return 0;

  // now we get the entries
  entry1 = msg->TreeNode1;
  entry2 = msg->TreeNode2;
  if(!entry1 || !entry2)
    return 0;

  ab1 = (struct ABEntry *)entry1->tn_User;
  ab2 = (struct ABEntry *)entry2->tn_User;
  if(!ab1 || !ab2)
    return 0;

  switch(G->AB->SortBy)
  {
     case 1:
       if(!(n1 = strrchr(ab1->RealName,' '))) n1 = ab1->RealName;
       if(!(n2 = strrchr(ab2->RealName,' '))) n2 = ab2->RealName;

       if((cmp = Stricmp(n1, n2)))
         return cmp;
     break;

     case 2:
       if((cmp = Stricmp(ab1->RealName, ab2->RealName)))
         return cmp;
     break;

     case 3:
       if((cmp = Stricmp(ab1->Comment, ab2->Comment)))
         return cmp;
     break;

     case 4:
       if((cmp = Stricmp(ab1->Address, ab2->Address)))
         return cmp;
     break;
  }

  return Stricmp(ab1->Alias, ab2->Alias);
}
MakeStaticHook(AB_LV_CmpFuncHook, AB_LV_CmpFunc);

///
/// AB_MakeABFormat
//  Creates format definition for address book listview
void AB_MakeABFormat(APTR lv)
{
  int i;
  char format[SIZE_LARGE];
  BOOL first = TRUE;

  *format = '\0';

  for(i = 0; i < ABCOLNUM; i++)
  {
    if(C->AddrbookCols & (1<<i))
    {
      int p;

      if(first)
        first = FALSE;
      else
        strlcat(format, " BAR,", sizeof(format));

      p = strlen(format);

      snprintf(&format[p], sizeof(format)-p, "COL=%d W=-1", i);
    }
  }

  set(lv, MUIA_NListtree_Format, format);
}

///

/// AB_New
//  Creates address book window
struct AB_ClassData *AB_New(void)
{
   struct AB_ClassData *data = calloc(1, sizeof(struct AB_ClassData));
   if (data)
   {
      Object *list;

      enum {
        AMEN_NEW,AMEN_OPEN,AMEN_APPEND,AMEN_SAVE,AMEN_SAVEAS,
        AMEN_IMPORT_LDIF, AMEN_IMPORT_CSV, AMEN_IMPORT_TAB,
        AMEN_EXPORT_LDIF, AMEN_EXPORT_CSV, AMEN_EXPORT_TAB,
        AMEN_PRINTA,
        AMEN_FIND,AMEN_NEWUSER,AMEN_NEWLIST,AMEN_NEWGROUP,AMEN_EDIT,
        AMEN_DUPLICATE,AMEN_DELETE,AMEN_PRINTE,AMEN_SORTALIAS,
        AMEN_SORTLNAME,AMEN_SORTFNAME,AMEN_SORTDESC,AMEN_SORTADDR,
        AMEN_FOLD,AMEN_UNFOLD
      };

      data->GUI.WI = WindowObject,
         MUIA_HelpNode,"AB_W",
         MUIA_Window_Menustrip, MenustripObject,
            MUIA_Family_Child, MenuObject, MUIA_Menu_Title, GetStr(MSG_CO_CrdABook),
               MUIA_Family_Child, MenuitemObject, MUIA_Menuitem_Title, GetStr(MSG_New), MUIA_Menuitem_Shortcut,"N", MUIA_UserData,AMEN_NEW, End,
               MUIA_Family_Child, MenuitemObject, MUIA_Menuitem_Title, GetStr(MSG_Open), MUIA_Menuitem_Shortcut,"O", MUIA_UserData,AMEN_OPEN, End,
               MUIA_Family_Child, MenuitemObject, MUIA_Menuitem_Title, GetStr(MSG_Append), MUIA_Menuitem_Shortcut,"I", MUIA_UserData,AMEN_APPEND, End,
               MUIA_Family_Child, MenuitemObject, MUIA_Menuitem_Title, NM_BARLABEL, End,
               MUIA_Family_Child, MenuitemObject, MUIA_Menuitem_Title, GetStr(MSG_AB_IMPORT),
                  MUIA_Family_Child, MenuitemObject, MUIA_Menuitem_Title, GetStr(MSG_AB_LDIF), MUIA_UserData, AMEN_IMPORT_LDIF, End,
                  MUIA_Family_Child, MenuitemObject, MUIA_Menuitem_Title, GetStr(MSG_AB_CSV), MUIA_UserData, AMEN_IMPORT_CSV, End,
                  MUIA_Family_Child, MenuitemObject, MUIA_Menuitem_Title, GetStr(MSG_AB_TAB), MUIA_UserData, AMEN_IMPORT_TAB, End,
               End,
               MUIA_Family_Child, MenuitemObject, MUIA_Menuitem_Title, GetStr(MSG_AB_EXPORT),
                  MUIA_Family_Child, MenuitemObject, MUIA_Menuitem_Title, GetStr(MSG_AB_LDIF), MUIA_UserData, AMEN_EXPORT_LDIF, End,
                  MUIA_Family_Child, MenuitemObject, MUIA_Menuitem_Title, GetStr(MSG_AB_CSV), MUIA_UserData, AMEN_EXPORT_CSV, End,
                  MUIA_Family_Child, MenuitemObject, MUIA_Menuitem_Title, GetStr(MSG_AB_TAB), MUIA_UserData, AMEN_EXPORT_TAB, End,
               End,
               MUIA_Family_Child, MenuitemObject, MUIA_Menuitem_Title, NM_BARLABEL, End,
               MUIA_Family_Child, MenuitemObject, MUIA_Menuitem_Title, GetStr(MSG_Save), MUIA_Menuitem_Shortcut,"S", MUIA_UserData,AMEN_SAVE, End,
               MUIA_Family_Child, MenuitemObject, MUIA_Menuitem_Title, GetStr(MSG_SaveAs), MUIA_Menuitem_Shortcut,"A", MUIA_UserData,AMEN_SAVEAS, End,
               MUIA_Family_Child, MenuitemObject, MUIA_Menuitem_Title, NM_BARLABEL, End,
               MUIA_Family_Child, MenuitemObject, MUIA_Menuitem_Title, GetStr(MSG_AB_MIFind), MUIA_Menuitem_Shortcut,"F", MUIA_UserData,AMEN_FIND, End,
               MUIA_Family_Child, MenuitemObject, MUIA_Menuitem_Title, GetStr(MSG_Print), MUIA_UserData,AMEN_PRINTA, End,
            End,
            MUIA_Family_Child, MenuObject, MUIA_Menu_Title, GetStr(MSG_AB_Entry),
               MUIA_Family_Child, MenuitemObject, MUIA_Menuitem_Title, GetStr(MSG_AB_AddUser), MUIA_Menuitem_Shortcut,"P", MUIA_UserData,AMEN_NEWUSER, End,
               MUIA_Family_Child, MenuitemObject, MUIA_Menuitem_Title, GetStr(MSG_AB_AddList), MUIA_Menuitem_Shortcut,"L", MUIA_UserData,AMEN_NEWLIST, End,
               MUIA_Family_Child, MenuitemObject, MUIA_Menuitem_Title, GetStr(MSG_AB_AddGroup), MUIA_Menuitem_Shortcut,"G", MUIA_UserData,AMEN_NEWGROUP, End,
               MUIA_Family_Child, MenuitemObject, MUIA_Menuitem_Title, NM_BARLABEL, End,
               MUIA_Family_Child, MenuitemObject, MUIA_Menuitem_Title, GetStr(MSG_Edit), MUIA_Menuitem_Shortcut,"E", MUIA_UserData,AMEN_EDIT, End,
               MUIA_Family_Child, MenuitemObject, MUIA_Menuitem_Title, GetStr(MSG_AB_Duplicate), MUIA_Menuitem_Shortcut,"D", MUIA_UserData,AMEN_DUPLICATE, End,
               MUIA_Family_Child, MenuitemObject, MUIA_Menuitem_Title, GetStr(MSG_AB_MIDelete), MUIA_Menuitem_Shortcut,"Del", MUIA_Menuitem_CommandString,TRUE, MUIA_UserData,AMEN_DELETE, End,
               MUIA_Family_Child, MenuitemObject, MUIA_Menuitem_Title, NM_BARLABEL, End,
               MUIA_Family_Child, MenuitemObject, MUIA_Menuitem_Title, GetStr(MSG_AB_MIPrint), MUIA_UserData,AMEN_PRINTE, End,
            End,
            MUIA_Family_Child, MenuObject, MUIA_Menu_Title, GetStr(MSG_AB_Sort),
               MUIA_Family_Child, MenuitemObject, MUIA_Menuitem_Title, GetStr(MSG_AB_SortByAlias), MUIA_Menuitem_Shortcut,"1", MUIA_UserData,AMEN_SORTALIAS, End,
               MUIA_Family_Child, MenuitemObject, MUIA_Menuitem_Title, GetStr(MSG_AB_SortByName), MUIA_Menuitem_Shortcut,"2", MUIA_UserData,AMEN_SORTLNAME, End,
               MUIA_Family_Child, MenuitemObject, MUIA_Menuitem_Title, GetStr(MSG_AB_SortByFirstname), MUIA_Menuitem_Shortcut,"3", MUIA_UserData,AMEN_SORTFNAME, End,
               MUIA_Family_Child, MenuitemObject, MUIA_Menuitem_Title, GetStr(MSG_AB_SortByDesc), MUIA_Menuitem_Shortcut,"4", MUIA_UserData,AMEN_SORTDESC, End,
               MUIA_Family_Child, MenuitemObject, MUIA_Menuitem_Title, GetStr(MSG_AB_SortByAddress), MUIA_Menuitem_Shortcut,"5", MUIA_UserData,AMEN_SORTADDR, End,
            End,
            MUIA_Family_Child, MenuObject, MUIA_Menu_Title, GetStr(MSG_AB_View),
               MUIA_Family_Child, MenuitemObject, MUIA_Menuitem_Title, GetStr(MSG_AB_Unfold), MUIA_Menuitem_Shortcut,"<", MUIA_UserData,AMEN_UNFOLD, End,
               MUIA_Family_Child, MenuitemObject, MUIA_Menuitem_Title, GetStr(MSG_AB_Fold), MUIA_Menuitem_Shortcut,">", MUIA_UserData,AMEN_FOLD, End,
            End,
         End,
         MUIA_Window_ID,MAKE_ID('B','O','O','K'),
         WindowContents, VGroup,
            Child, hasHideToolBarFlag(C->HideGUIElements) ?
               (HGroup,
                  MUIA_HelpNode, "AB_B",
                  Child, data->GUI.BT_TO  = MakeButton("_To:"),
                  Child, data->GUI.BT_CC  = MakeButton("_CC:"),
                  Child, data->GUI.BT_BCC = MakeButton("_BCC:"),
               End) :
               (HGroup, GroupSpacing(0),
                  MUIA_HelpNode, "AB_B",
                  Child, VGroup,
                     MUIA_Weight, 10,
                     MUIA_Group_VertSpacing, 0,
                     Child, data->GUI.BT_TO  = MakeButton("_To:"),
                     Child, data->GUI.BT_CC  = MakeButton("_CC:"),
                     Child, data->GUI.BT_BCC = MakeButton("_BCC:"),
                     Child, HVSpace,
                  End,
                  Child, MUI_MakeObject(MUIO_VBar, 12),
                  Child, AddrBookToolbarObject,
                  End,
               End),

            Child, list = NListviewObject,
               MUIA_CycleChain,         TRUE,
               MUIA_Listview_DragType,  MUIV_Listview_DragType_Immediate,
               MUIA_NListview_NList,    data->GUI.LV_ADDRESSES = AddrBookListtreeObject,
                  InputListFrame,
                  MUIA_NListtree_CompareHook,     &AB_LV_CmpFuncHook,
                  MUIA_NListtree_DragDropSort,    TRUE,
                  MUIA_NListtree_Title,           TRUE,
                  MUIA_NListtree_ConstructHook,   &AB_LV_ConFuncHook,
                  MUIA_NListtree_DestructHook,    &AB_LV_DesFuncHook,
                  MUIA_NListtree_DisplayHook,     &AB_LV_DspFuncHook,
                  MUIA_NListtree_EmptyNodes,      TRUE,
                  MUIA_Font,                      C->FixedFontList ? MUIV_NList_Font_Fixed : MUIV_NList_Font,
               End,
            End,
         End,
      End;

      // If we successfully created the WindowObject
      if (data->GUI.WI)
      {
        AB_MakeABFormat(data->GUI.LV_ADDRESSES);
        DoMethod(G->App, OM_ADDMEMBER, data->GUI.WI);
        set(data->GUI.WI, MUIA_Window_DefaultObject, list);
        SetHelp(data->GUI.BT_TO ,MSG_HELP_AB_BT_TO );
        SetHelp(data->GUI.BT_CC ,MSG_HELP_AB_BT_CC );
        SetHelp(data->GUI.BT_BCC,MSG_HELP_AB_BT_BCC);

        DoMethod(data->GUI.WI          , MUIM_Notify, MUIA_Window_MenuAction    , AMEN_NEW        , MUIV_Notify_Application, 2, MUIM_CallHook, &AB_NewABookHook);
        DoMethod(data->GUI.WI          , MUIM_Notify, MUIA_Window_MenuAction    , AMEN_OPEN       , MUIV_Notify_Application, 2, MUIM_CallHook, &AB_OpenABookHook);
        DoMethod(data->GUI.WI          , MUIM_Notify, MUIA_Window_MenuAction    , AMEN_APPEND     , MUIV_Notify_Application, 2, MUIM_CallHook, &AB_AppendABookHook);
        DoMethod(data->GUI.WI          , MUIM_Notify, MUIA_Window_MenuAction    , AMEN_IMPORT_LDIF, MUIV_Notify_Application, 2, MUIM_CallHook, &AB_ImportLDIFABookHook);
        DoMethod(data->GUI.WI          , MUIM_Notify, MUIA_Window_MenuAction    , AMEN_IMPORT_TAB , MUIV_Notify_Application, 3, MUIM_CallHook, &AB_ImportTabCSVABookHook, '\t');
        DoMethod(data->GUI.WI          , MUIM_Notify, MUIA_Window_MenuAction    , AMEN_IMPORT_CSV , MUIV_Notify_Application, 3, MUIM_CallHook, &AB_ImportTabCSVABookHook, ',');
        DoMethod(data->GUI.WI          , MUIM_Notify, MUIA_Window_MenuAction    , AMEN_EXPORT_LDIF, MUIV_Notify_Application, 2, MUIM_CallHook, &AB_ExportLDIFABookHook);
        DoMethod(data->GUI.WI          , MUIM_Notify, MUIA_Window_MenuAction    , AMEN_EXPORT_TAB , MUIV_Notify_Application, 3, MUIM_CallHook, &AB_ExportTabCSVABookHook, '\t');
        DoMethod(data->GUI.WI          , MUIM_Notify, MUIA_Window_MenuAction    , AMEN_EXPORT_CSV , MUIV_Notify_Application, 3, MUIM_CallHook, &AB_ExportTabCSVABookHook, ',');
        DoMethod(data->GUI.WI          , MUIM_Notify, MUIA_Window_MenuAction    , AMEN_SAVE       , MUIV_Notify_Application, 2, MUIM_CallHook, &AB_SaveABookHook);
        DoMethod(data->GUI.WI          , MUIM_Notify, MUIA_Window_MenuAction    , AMEN_SAVEAS     , MUIV_Notify_Application, 2, MUIM_CallHook, &AB_SaveABookAsHook);
        DoMethod(data->GUI.WI          , MUIM_Notify, MUIA_Window_MenuAction    , AMEN_PRINTA     , MUIV_Notify_Application, 2, MUIM_CallHook, &AB_PrintABookHook);
        DoMethod(data->GUI.WI          , MUIM_Notify, MUIA_Window_MenuAction    , AMEN_NEWUSER    , MUIV_Notify_Application, 3, MUIM_CallHook, &AB_AddEntryHook, AET_USER);
        DoMethod(data->GUI.WI          , MUIM_Notify, MUIA_Window_MenuAction    , AMEN_NEWLIST    , MUIV_Notify_Application, 3, MUIM_CallHook, &AB_AddEntryHook, AET_LIST);
        DoMethod(data->GUI.WI          , MUIM_Notify, MUIA_Window_MenuAction    , AMEN_NEWGROUP   , MUIV_Notify_Application, 3, MUIM_CallHook, &AB_AddEntryHook, AET_GROUP);
        DoMethod(data->GUI.WI          , MUIM_Notify, MUIA_Window_MenuAction    , AMEN_EDIT       , MUIV_Notify_Application, 2, MUIM_CallHook, &AB_EditHook);
        DoMethod(data->GUI.WI          , MUIM_Notify, MUIA_Window_MenuAction    , AMEN_DUPLICATE  , MUIV_Notify_Application, 2, MUIM_CallHook, &AB_DuplicateHook);
        DoMethod(data->GUI.WI          , MUIM_Notify, MUIA_Window_MenuAction    , AMEN_DELETE     , MUIV_Notify_Application, 2, MUIM_CallHook, &AB_DeleteHook);
        DoMethod(data->GUI.WI          , MUIM_Notify, MUIA_Window_MenuAction    , AMEN_PRINTE     , MUIV_Notify_Application, 2, MUIM_CallHook, &AB_PrintHook);
        DoMethod(data->GUI.WI          , MUIM_Notify, MUIA_Window_MenuAction    , AMEN_FIND       , MUIV_Notify_Application, 2, MUIM_CallHook, &AB_FindHook);
        DoMethod(data->GUI.WI          , MUIM_Notify, MUIA_Window_MenuAction    , AMEN_SORTALIAS  , MUIV_Notify_Application, 3, MUIM_CallHook, &AB_SortHook, 0);
        DoMethod(data->GUI.WI          , MUIM_Notify, MUIA_Window_MenuAction    , AMEN_SORTLNAME  , MUIV_Notify_Application, 3, MUIM_CallHook, &AB_SortHook, 1);
        DoMethod(data->GUI.WI          , MUIM_Notify, MUIA_Window_MenuAction    , AMEN_SORTFNAME  , MUIV_Notify_Application, 3, MUIM_CallHook, &AB_SortHook, 2);
        DoMethod(data->GUI.WI          , MUIM_Notify, MUIA_Window_MenuAction    , AMEN_SORTDESC   , MUIV_Notify_Application, 3, MUIM_CallHook, &AB_SortHook, 3);
        DoMethod(data->GUI.WI          , MUIM_Notify, MUIA_Window_MenuAction    , AMEN_SORTADDR   , MUIV_Notify_Application, 3, MUIM_CallHook, &AB_SortHook, 4);
        DoMethod(data->GUI.WI          , MUIM_Notify, MUIA_Window_MenuAction    , AMEN_FOLD       , MUIV_Notify_Application, 3, MUIM_CallHook, &AB_FoldUnfoldHook, TRUE);
        DoMethod(data->GUI.WI          , MUIM_Notify, MUIA_Window_MenuAction    , AMEN_UNFOLD     , MUIV_Notify_Application, 3, MUIM_CallHook, &AB_FoldUnfoldHook, FALSE);
        DoMethod(data->GUI.LV_ADDRESSES, MUIM_Notify, MUIA_NListtree_DoubleClick, MUIV_EveryTime  , MUIV_Notify_Application, 2, MUIM_CallHook, &AB_DoubleClickHook);
        DoMethod(data->GUI.BT_TO       , MUIM_Notify, MUIA_Pressed              , FALSE           , MUIV_Notify_Application, 3, MUIM_CallHook, &AB_FromAddrBookHook, ABM_TO);
        DoMethod(data->GUI.BT_CC       , MUIM_Notify, MUIA_Pressed              , FALSE           , MUIV_Notify_Application, 3, MUIM_CallHook, &AB_FromAddrBookHook, ABM_CC);
        DoMethod(data->GUI.BT_BCC      , MUIM_Notify, MUIA_Pressed              , FALSE           , MUIV_Notify_Application, 3, MUIM_CallHook, &AB_FromAddrBookHook, ABM_BCC);

        DoMethod(data->GUI.WI,MUIM_Notify,MUIA_Window_InputEvent   ,"-capslock del" ,MUIV_Notify_Application  ,2,MUIM_CallHook       ,&AB_DeleteHook);
        DoMethod(data->GUI.WI,MUIM_Notify,MUIA_Window_CloseRequest ,TRUE ,MUIV_Notify_Application           ,2,MUIM_CallHook       ,&AB_CloseHook);

        return data;
      }
      free(data);
   }
   return NULL;
}
///
