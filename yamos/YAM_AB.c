/***************************************************************************

 YAM - Yet Another Mailer
 Copyright (C) 1995-2000 by Marcel Beck <mbeck@yam.ch>
 Copyright (C) 2000-2004 by YAM Open Source Team

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

#include "extra.h"
#include "SDI_hook.h"

#include "YAM.h"
#include "YAM_addressbook.h"
#include "YAM_addressbookEntry.h"
#include "YAM_classes.h"
#include "YAM_config.h"
#include "YAM_debug.h"
#include "YAM_error.h"
#include "YAM_global.h"
#include "YAM_locale.h"
#include "YAM_main.h"
#include "YAM_utilities.h"
#include "YAM_write.h"
#include "classes/Classes.h"

/* local protos */
static STACKEXT BOOL AB_FindTodaysBirthdates(struct MUI_NListtree_TreeNode*, long);
static STACKEXT void AB_SaveTreeNode(FILE*, struct MUI_NListtree_TreeNode *);
static void AB_PrintField(FILE*, char*, char*);
static void AB_PrintShortEntry(FILE*, struct ABEntry*);
static void AB_PrintLongEntry(FILE*, struct ABEntry*);
static STACKEXT void AB_PrintLevel(struct MUI_NListtree_TreeNode*, FILE*, int);


/***************************************************************************
 Module: Address book
***************************************************************************/

/// AB_PrettyPrintAddress
STRPTR AB_PrettyPrintAddress (struct ABEntry *e)
{
   return AB_PrettyPrintAddress2(e->RealName, e->Address);
}

///
/// AB_PrettyPrintAddress2
STRPTR AB_PrettyPrintAddress2 (STRPTR realname, STRPTR address)
{
   static TEXT buf[SIZE_REALNAME + SIZE_ADDRESS + 4];
   sprintf(buf, "%." STR(SIZE_REALNAME) "s <%." STR(SIZE_ADDRESS) "s>", realname, address);
   return buf;
}

///
/// AB_GotoEntry
//  Searches an entry by alias and activates it
APTR AB_GotoEntry(char *alias)
{
   struct MUI_NListtree_TreeNode *tn;

   if(!alias) return NULL;

   if ((tn = (struct MUI_NListtree_TreeNode *)DoMethod(G->AB->GUI.LV_ADDRESSES, MUIM_NListtree_FindName, MUIV_NListtree_FindName_ListNode_Root, alias, MUIF_NONE)))
   {
      DoMethod(G->AB->GUI.LV_ADDRESSES, MUIM_NListtree_Open, MUIV_NListtree_Open_ListNode_Parent, tn, MUIF_NONE);
      set(G->AB->GUI.LV_ADDRESSES, MUIA_NListtree_Active, tn);
   }

   return tn;
}

///
/// AB_ExpandBD
//  Converts date from numeric into textual format
char *AB_ExpandBD(long date)
{
   static char datestr[SIZE_SMALL];

   // check first if it could be a valid date!
   if (!date || date/1000000 > 31 || date/1000000 < 1 || ((date/10000)%100) > 12 || ((date/10000)%100) < 1 || date%10000 < 1000)
     return "";

   sprintf(datestr, "%02ld-%s-%ld", date/1000000, months[((date/10000)%100)-1], date%10000);
   return datestr;
}

///
/// AB_CompressBD
//  Connverts date from textual into numeric format
long AB_CompressBD(char *datestr)
{
   long d, m, y;
   for (m = 12; m > 0; m--) if (!strnicmp(&datestr[3], months[m-1], 3)) break;
   if (!m) return 0;
   if ((d = atoi(datestr)) < 1 || d > 31) return 0;
   if ((y = atoi(&datestr[7])) < 1800 || y > 2100) return 0;
   return (100*d+m)*10000+y;
}

///
/// AB_FindTodaysBirthdates (rec)
//  Recursively searches the address book for a given birth date
static STACKEXT BOOL AB_FindTodaysBirthdates(struct MUI_NListtree_TreeNode *list, long today)
{
  struct MUI_NListtree_TreeNode *tn;
  int wrwin, i;

  for(i=0;;i++)
  {
    if((tn = (struct MUI_NListtree_TreeNode *)DoMethod(G->AB->GUI.LV_ADDRESSES, MUIM_NListtree_GetEntry, list, i, MUIF_NONE)))
    {
      struct ABEntry *ab = tn->tn_User;

      if (ab->Type == AET_USER && ab->BirthDay/10000 == today/10000)
      {
        char question[SIZE_LARGE], *name = *ab->RealName ? ab->RealName : ab->Alias;
        sprintf(question, GetStr(MSG_AB_BirthdayReq), name, today%10000-ab->BirthDay%10000);

        if (MUI_Request(G->App, G->MA->GUI.WI, 0, GetStr(MSG_AB_BirthdayReminder), GetStr(MSG_YesNoReq), question))
        {
          if ((wrwin = MA_NewNew(NULL, 0)) >= 0)
          {
            setstring(G->WR[wrwin]->GUI.ST_TO, ab->Alias);
            setstring(G->WR[wrwin]->GUI.ST_SUBJECT, GetStr(MSG_AB_HappyBirthday));
          }
        }
      }
    }else break;
  }

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

  // we scan until we are at the end of the list or
  // if we found more then one matching entry
  for(i=0; hits <= 2; i++, found=FALSE)
  {
    tn = (struct MUI_NListtree_TreeNode *)DoMethod(G->AB->GUI.LV_ADDRESSES, MUIM_NListtree_GetEntry, MUIV_NListtree_GetEntry_ListNode_Root, i, MUIF_NONE);
    if(!tn) break;

    // now we set the AB_Entry
    ab_found = tn->tn_User;
    if(!ab_found) break;

    // now we check if this entry is one of the not wished entry types
    // and then we skip it.
    if(ab_found->Type == AET_USER  && !isUserSearch(mode))  continue;
    if(ab_found->Type == AET_LIST  && !isListSearch(mode))  continue;
    if(ab_found->Type == AET_GROUP && !isGroupSearch(mode)) continue;

    if(isCompleteSearch(mode))
    {
      // Now we check for the ALIAS->REALNAME->ADDRESS, so only ONE mode is allowed at a time
      if(isAliasSearch(mode_type))          found = !Strnicmp(ab_found->Alias,    text, tl);
      else if(isRealNameSearch(mode_type))  found = !Strnicmp(ab_found->RealName, text, tl);
      else if(isAddressSearch(mode_type))   found = !Strnicmp(ab_found->Address,  text, tl);
    }
    else
    {
      // Now we check for the ALIAS->REALNAME->ADDRESS, so only ONE mode is allowed at a time
      if(isAliasSearch(mode_type))          found = !Stricmp(ab_found->Alias,    text);
      else if(isRealNameSearch(mode_type))  found = !Stricmp(ab_found->RealName, text);
      else if(isAddressSearch(mode_type))   found = !Stricmp(ab_found->Address,  text);
    }

    if(found)
    {
      *ab = ab_found;
      hits++;
    }
  }

  return(hits);
}

///
/// AB_CompleteAlias
//  Auto-completes alias or name in recipient field
char *AB_CompleteAlias(char *text)
{
  char *compl = NULL;
  struct ABEntry *ab = NULL;

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

  if (compl) return &compl[strlen(text)];
  else return(NULL);
}

///
/// AB_InsertAddress
//  Adds a new recipient to a recipient field
void AB_InsertAddress(APTR string, char *alias, char *name, char *address)
{
   char *p = (char *)xget(string, MUIA_UserData);
   if (p)
   {
      p = (char *)xget(string, MUIA_String_Contents);
      if (*p) DoMethod(string, MUIM_BetterString_Insert, ", ", MUIV_BetterString_Insert_EndOfString, TAG_DONE);
   }
   else setstring(string, "");
   if (*alias) DoMethod(string, MUIM_BetterString_Insert, alias, MUIV_BetterString_Insert_EndOfString, TAG_DONE);
   else
   {
      if (*name)
      {
         if (strchr(name, ',')) DoMethod(string, MUIM_BetterString_Insert, "\"", MUIV_BetterString_Insert_EndOfString, TAG_DONE);
         DoMethod(string, MUIM_BetterString_Insert, name, MUIV_BetterString_Insert_EndOfString, TAG_DONE);
         if (strchr(name, ',')) DoMethod(string, MUIM_BetterString_Insert, "\"", MUIV_BetterString_Insert_EndOfString, TAG_DONE);
      }
      if (*address)
      {
         if (*name) DoMethod(string, MUIM_BetterString_Insert, " <", MUIV_BetterString_Insert_EndOfString, TAG_DONE);
         DoMethod(string, MUIM_BetterString_Insert, address, MUIV_BetterString_Insert_EndOfString, TAG_DONE);
         if (*name) DoMethod(string, MUIM_BetterString_Insert, ">", MUIV_BetterString_Insert_EndOfString, TAG_DONE);
      }
   }
}

///
/// AB_FromAddrBook
/*** AB_FromAddrBook - Inserts an address book entry into a recipient string ***/
HOOKPROTONHNO(AB_FromAddrBook, void, ULONG *arg)
{
   APTR string;
   struct MUI_NListtree_TreeNode *active;

   if ((active = (struct MUI_NListtree_TreeNode *)xget(G->AB->GUI.LV_ADDRESSES, MUIA_NListtree_Active)))
   {
      int winnum = G->AB->WrWin;
      struct ABEntry *addr = (struct ABEntry *)(active->tn_User);
      BOOL openwin = winnum < 0;
      if (!openwin) openwin = !G->WR[winnum];
      if (openwin) G->AB->WrWin = winnum = MA_NewNew(NULL, 0);
      if (winnum >= 0)
      {
         switch (*arg)
         {
            case ABM_TO:      string = G->WR[winnum]->GUI.ST_TO; break;
            case ABM_CC:      string = G->WR[winnum]->GUI.ST_CC; break;
            case ABM_BCC:     string = G->WR[winnum]->GUI.ST_BCC; break;
            case ABM_REPLYTO: string = G->WR[winnum]->GUI.ST_REPLYTO; break;
            case ABM_FROM:    string = G->WR[winnum]->GUI.ST_FROM; break;
            default: string = (APTR)*arg;
         }
         DoMethod(string, MUIM_Recipientstring_AddRecipient, addr->Alias ? addr->Alias : addr->RealName);
      }
   }
}
MakeStaticHook(AB_FromAddrBookHook, AB_FromAddrBook);

///
/// AB_LoadTree
//  Loads the address book from a file
BOOL AB_LoadTree(char *fname, BOOL append, BOOL sorted)
{
   static struct ABEntry addr;
   struct MUI_NListtree_TreeNode *parent[8];
   char buffer[SIZE_LARGE];
   FILE *fh;
   int len, nested = 0;

   parent[nested] = MUIV_NListtree_Insert_ListNode_Root;

   if((fh = fopen(fname, "r")) && GetLine(fh, buffer, SIZE_LARGE))
   {
      if (!strncmp(buffer,"YAB",3))
      {
         int version = buffer[3]-'0';

         G->AB->Modified = append;
         if (!append) DoMethod(G->AB->GUI.LV_ADDRESSES, MUIM_NListtree_Clear, NULL, 0);

         set(G->AB->GUI.LV_ADDRESSES, MUIA_NListtree_Quiet, TRUE);
         while (GetLine(fh, buffer, SIZE_LARGE))
         {
            memset(&addr, 0, sizeof(struct ABEntry));
            if (!strncmp(buffer, "@USER", 5))
            {
               addr.Type = AET_USER;
               stccpy(addr.Alias   , Trim(&buffer[6]),SIZE_NAME);
               stccpy(addr.Address , Trim(GetLine(fh, buffer, SIZE_LARGE)),SIZE_ADDRESS);
               stccpy(addr.RealName, Trim(GetLine(fh, buffer, SIZE_LARGE)),SIZE_REALNAME);
               stccpy(addr.Comment , Trim(GetLine(fh, buffer, SIZE_LARGE)),SIZE_DEFAULT);
               if (version > 2)
               {
                  stccpy(addr.Phone   , Trim(GetLine(fh, buffer, SIZE_LARGE)),SIZE_DEFAULT);;
                  stccpy(addr.Street  , Trim(GetLine(fh, buffer, SIZE_LARGE)),SIZE_DEFAULT);
                  stccpy(addr.City    , Trim(GetLine(fh, buffer, SIZE_LARGE)),SIZE_DEFAULT);
                  stccpy(addr.Country , Trim(GetLine(fh, buffer, SIZE_LARGE)),SIZE_DEFAULT);
                  stccpy(addr.PGPId   , Trim(GetLine(fh, buffer, SIZE_LARGE)),SIZE_DEFAULT);
                  addr.BirthDay = atol(Trim(GetLine(fh, buffer, SIZE_LARGE)));
                  stccpy(addr.Photo   , Trim(GetLine(fh, buffer, SIZE_LARGE)),SIZE_PATHFILE);
                  if (strcmp(GetLine(fh, buffer, SIZE_LARGE), "@ENDUSER")) stccpy(addr.Homepage,Trim(buffer),SIZE_URL);
               }
               if (version > 3)
               {
                  addr.DefSecurity = atoi(Trim(GetLine(fh, buffer, SIZE_LARGE)));
               }
               do if (!strcmp(buffer, "@ENDUSER")) break;
               while (GetLine(fh, buffer, SIZE_LARGE));
               DoMethod(G->AB->GUI.LV_ADDRESSES, MUIM_NListtree_Insert, addr.Alias[0] ? addr.Alias : addr.RealName, &addr, parent[nested], sorted ?  MUIV_NListtree_Insert_PrevNode_Sorted : MUIV_NListtree_Insert_PrevNode_Tail, MUIF_NONE);
            }
            else if (!strncmp(buffer, "@LIST", 5))
            {
               char *members;
               addr.Type = AET_LIST;

               stccpy(addr.Alias   , Trim(&buffer[6]), SIZE_NAME);
               if (version > 2)
               {
                  stccpy(addr.Address , Trim(GetLine(fh, buffer, SIZE_LARGE)),SIZE_ADDRESS);
                  stccpy(addr.RealName, Trim(GetLine(fh, buffer, SIZE_LARGE)),SIZE_REALNAME);
               }
               stccpy(addr.Comment , Trim(GetLine(fh, buffer, SIZE_LARGE)), SIZE_DEFAULT);
               members = AllocStrBuf(SIZE_DEFAULT);
               while (GetLine(fh, buffer, SIZE_LARGE))
               {
                  if (!strcmp(buffer, "@ENDLIST")) break;
                  if (!*buffer) continue;
                  members = StrBufCat(members, buffer);
                  members = StrBufCat(members, "\n");
               }
               len = strlen(members)+1;
               addr.Members = malloc(len);
               strcpy(addr.Members, members);
               FreeStrBuf(members);
               DoMethod(G->AB->GUI.LV_ADDRESSES, MUIM_NListtree_Insert, addr.Alias, &addr, parent[nested], sorted ?  MUIV_NListtree_Insert_PrevNode_Sorted : MUIV_NListtree_Insert_PrevNode_Tail, MUIF_NONE);
               free(addr.Members);
            }
            else if (!strncmp(buffer, "@GROUP", 6))
            {
               addr.Type = AET_GROUP;
               stccpy(addr.Alias  , Trim(&buffer[7]), SIZE_NAME);
               stccpy(addr.Comment, Trim(GetLine(fh, buffer, SIZE_LARGE)), SIZE_DEFAULT);
               nested++;
               parent[nested] = (struct MUI_NListtree_TreeNode *)DoMethod(G->AB->GUI.LV_ADDRESSES, MUIM_NListtree_Insert, addr.Alias, &addr, parent[nested-1], MUIV_NListtree_Insert_PrevNode_Tail, TNF_LIST);
            }
            else if (!strcmp(buffer,"@ENDGROUP"))
            {
               nested--;
            }
         }
         set(G->AB->GUI.LV_ADDRESSES, MUIA_NListtree_Quiet, FALSE);
      }
      else
      {
         // ask the user if he really wants to read out a non YAM
         // Addressbook file.
         if(MUI_Request(G->App, G->AB->GUI.WI, MUIF_NONE, NULL, GetStr(MSG_AB_NOYAMADDRBOOK_GADS), GetStr(MSG_AB_NOYAMADDRBOOK), fname))
         {
           G->AB->Modified = append;
           if (!append) DoMethod(G->AB->GUI.LV_ADDRESSES, MUIM_NListtree_Clear, NULL, 0);

           fseek(fh, 0, SEEK_SET);
           while (GetLine(fh, buffer, SIZE_LARGE))
           {
              char *p, *p2;
              memset(&addr, 0, sizeof(struct ABEntry));
              if ((p = strchr(buffer, ' '))) *p = 0;
              stccpy(addr.Address, buffer, SIZE_ADDRESS);
              if (p)
              {
                 stccpy(addr.RealName, ++p, SIZE_REALNAME);
                 if ((p2 = strchr(p, ' '))) *p2 = 0;
              }
              else if ((p2 = strchr(p = buffer, '@'))) *p2 = 0;
              stccpy(addr.Alias, p, SIZE_NAME);
              DoMethod(G->AB->GUI.LV_ADDRESSES, MUIM_NListtree_Insert, addr.Alias, &addr, parent[nested], sorted ?  MUIV_NListtree_Insert_PrevNode_Sorted : MUIV_NListtree_Insert_PrevNode_Tail, MUIF_NONE);
           }
         }
      }
      fclose(fh);
   }
   else
   {
      ER_NewError(GetStr(MSG_ER_ADDRBOOKLOAD), fname, NULL);
      if(fh) fclose(fh);
      return FALSE;
   }

   return TRUE;
}

///
/// AB_SaveTreeNode (rec)
//  Recursively saves an address book node
static STACKEXT void AB_SaveTreeNode(FILE *fh, struct MUI_NListtree_TreeNode *list)
{
   struct MUI_NListtree_TreeNode *tn;
   struct ABEntry *ab;
   int i;

   for (i=0; ; i++)
      if ((tn = (struct MUI_NListtree_TreeNode *)DoMethod(G->AB->GUI.LV_ADDRESSES, MUIM_NListtree_GetEntry, list, i, MUIV_NListtree_GetEntry_Flag_SameLevel)))
      {
         ab = tn->tn_User;
         switch (ab->Type)
         {
            case AET_USER:  fprintf(fh, "@USER %s\n%s\n%s\n%s\n%s\n%s\n%s\n%s\n%s\n%08ld\n%s\n%s\n%d\n@ENDUSER\n", ab->Alias, ab->Address, ab->RealName, ab->Comment,
                               ab->Phone, ab->Street, ab->City, ab->Country, ab->PGPId, ab->BirthDay, ab->Photo, ab->Homepage, ab->DefSecurity);
                            break;
            case AET_LIST:  fprintf(fh, "@LIST %s\n%s\n%s\n%s\n%s\n@ENDLIST\n", ab->Alias, ab->Address, ab->RealName, ab->Comment, ab->Members);
                            break;
            case AET_GROUP: fprintf(fh, "@GROUP %s\n%s\n", ab->Alias, ab->Comment);
                            AB_SaveTreeNode(fh, tn);
                            fputs("@ENDGROUP\n", fh);
                            break;
         }
      }
      else break;
}

///
/// AB_SaveTree
//  Saves the address book to a file
BOOL AB_SaveTree(char *fname)
{
   FILE *fh;

   if ((fh = fopen(fname, "w")))
   {
      fputs("YAB4 - YAM Addressbook\n", fh);
      AB_SaveTreeNode(fh, MUIV_NListtree_GetEntry_ListNode_Root);
      fclose(fh);
      AppendLogVerbose(70, GetStr(MSG_LOG_SavingABook), fname, "", "", "");
      return TRUE;
   }
   ER_NewError(GetStr(MSG_ER_CantCreateFile), fname, NULL);
   return FALSE;
}

///
/// AB_EditFunc
/*** AB_EditFunc - Modifies selected address book entry ***/
HOOKPROTONHNONP(AB_EditFunc, void)
{
   struct MUI_NListtree_TreeNode *tn;
   if ((tn = (struct MUI_NListtree_TreeNode *)xget(G->AB->GUI.LV_ADDRESSES, MUIA_NListtree_Active)))
   {
      struct ABEntry *ab = (struct ABEntry *)(tn->tn_User);
      int winnum = EA_Init(ab->Type, ab);
      if (winnum >= 0) EA_Setup(winnum, ab);
   }
}
MakeStaticHook(AB_EditHook, AB_EditFunc);

///
/// AB_DoubleClick
/*** AB_DoubleClick - User double-clicked in the address book ***/
HOOKPROTONHNONP(AB_DoubleClick, void)
{
   if (G->AB->WrWin >= 0) if (G->WR[G->AB->WrWin])
   {
      struct WR_GUIData *gui = &G->WR[G->AB->WrWin]->GUI;
      APTR obj = NULL;
      switch (G->AB->Mode)
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
      DoMethod(G->App, MUIM_CallHook, &AB_FromAddrBookHook, obj, TAG_DONE);
      set(G->AB->GUI.WI, MUIA_Window_CloseRequest, TRUE);
      return;
   }
   AB_EditFunc();
}
MakeStaticHook(AB_DoubleClickHook, AB_DoubleClick);

///
/// AB_Sort
/*** AB_Sort - Sorts the address book ***/
HOOKPROTONHNO(AB_Sort, void, int *arg)
{
   char fname[SIZE_PATHFILE];
   strmfp(fname, C->TempDir, ".addressbook.tmp");
   if (AB_SaveTree(fname))
   {
      G->AB->SortBy = *arg;
      AB_LoadTree(fname, FALSE, TRUE);
      remove(fname);
      G->AB->Modified = TRUE;
   }
}
MakeStaticHook(AB_SortHook, AB_Sort);

///
/// AB_NewABookFunc
/*** AB_NewABookFunc - Clears entire address book ***/
HOOKPROTONHNONP(AB_NewABookFunc, void)
{
   DoMethod(G->AB->GUI.LV_ADDRESSES, MUIM_NListtree_Remove, MUIV_NListtree_Remove_ListNode_Root, MUIV_NListtree_Remove_TreeNode_All, MUIF_NONE);
   G->AB->Modified = FALSE;
}
MakeStaticHook(AB_NewABookHook, AB_NewABookFunc);

///
/// AB_OpenABookFunc
/*** AB_OpenABookFunc - Loads selected address book ***/
HOOKPROTONHNONP(AB_OpenABookFunc, void)
{
   if (ReqFile(ASL_ABOOK,G->AB->GUI.WI, GetStr(MSG_Open), REQF_NONE, G->MA_MailDir, ""))
   {
      strmfp(G->AB_Filename, G->ASLReq[ASL_ABOOK]->fr_Drawer, G->ASLReq[ASL_ABOOK]->fr_File);
      AB_LoadTree(G->AB_Filename, FALSE, FALSE);
   }
}
MakeStaticHook(AB_OpenABookHook, AB_OpenABookFunc);

///
/// AB_AppendABookFunc
/*** AB_AppendABookFunc - Appends selected address book ***/
HOOKPROTONHNONP(AB_AppendABookFunc, void)
{
   if (ReqFile(ASL_ABOOK,G->AB->GUI.WI, GetStr(MSG_Append), REQF_NONE, G->MA_MailDir, ""))
   {
      char aname[SIZE_PATHFILE];
      strmfp(aname, G->ASLReq[ASL_ABOOK]->fr_Drawer, G->ASLReq[ASL_ABOOK]->fr_File);
      AB_LoadTree(aname, TRUE, FALSE);
   }
}
MakeStaticHook(AB_AppendABookHook, AB_AppendABookFunc);

///
/// AB_SaveABookFunc
/*** AB_SaveABookFunc - Saves address book using the default name ***/
HOOKPROTONHNONP(AB_SaveABookFunc, void)
{
   Busy(GetStr(MSG_BusySavingAB), G->AB_Filename, 0, 0);
   AB_SaveTree(G->AB_Filename);
   G->AB->Modified = FALSE;
   BusyEnd;
}
MakeHook(AB_SaveABookHook, AB_SaveABookFunc);

///
/// AB_SaveABookAsFunc
/*** AB_SaveABookAsFunc - Saves address book under a different name ***/
HOOKPROTONHNONP(AB_SaveABookAsFunc, void)
{
   if (ReqFile(ASL_ABOOK,G->AB->GUI.WI, GetStr(MSG_SaveAs), REQF_SAVEMODE, G->MA_MailDir, ""))
   {
      strmfp(G->AB_Filename, G->ASLReq[ASL_ABOOK]->fr_Drawer, G->ASLReq[ASL_ABOOK]->fr_File);
      AB_SaveABookFunc();
   }
}
MakeStaticHook(AB_SaveABookAsHook, AB_SaveABookAsFunc);

///
/// AB_PrintField
//  Formats and prints a single field
static void AB_PrintField(FILE *prt, char *fieldname, char *field)
{
   if (*field) fprintf(prt, "%-20.20s: %-50.50s\n", StripUnderscore(fieldname), field);
}

///
/// AB_PrintShortEntry
//  Prints an address book entry in compact format
static void AB_PrintShortEntry(FILE *prt, struct ABEntry *ab)
{
   static const char types[3] = { 'P','L','G' };
   fprintf(prt, "%c %-12.12s %-20.20s %-36.36s\n", types[ab->Type-AET_USER],
      ab->Alias, ab->RealName, ab->Type == AET_USER ? ab->Address : ab->Comment);
}

///
/// AB_PrintLongEntry
//  Prints an address book entry in detailed format
static void AB_PrintLongEntry(FILE *prt, struct ABEntry *ab)
{
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
         if (ab->Members)
         {
            BOOL header = FALSE;
            char *ptr;
            for (ptr = ab->Members; *ptr; ptr++)
            {
               char *nptr = strchr(ptr, '\n');
               if (nptr) *nptr = 0; else break;
               if (!header) { AB_PrintField(prt, GetStr(MSG_EA_Members), ptr); header = TRUE; }
               else fprintf(prt, "                      %s\n", ptr);
               *nptr = '\n';
               ptr = nptr;
            }
         }
         break;
      case AET_GROUP:
         AB_PrintField(prt, GetStr(MSG_AB_GroupAlias), ab->Alias);
   }
   AB_PrintField(prt, GetStr(MSG_EA_Description), ab->Comment);
}

///
/// AB_PrintLevel (rec)
//  Recursively prints an address book node
static STACKEXT void AB_PrintLevel(struct MUI_NListtree_TreeNode *list, FILE *prt, int mode)
{
   struct MUI_NListtree_TreeNode *tn;
   int i;

   for (i=0; ; i++)
      if ((tn = (struct MUI_NListtree_TreeNode *)DoMethod(G->AB->GUI.LV_ADDRESSES, MUIM_NListtree_GetEntry, list, i, MUIV_NListtree_GetEntry_Flag_SameLevel)))
      {
         struct ABEntry *ab = tn->tn_User;
         if (mode == 1) AB_PrintLongEntry(prt, ab); else AB_PrintShortEntry(prt, ab);
         if (ab->Type == AET_GROUP) AB_PrintLevel(tn, prt, mode);
      }
      else break;
}

///
/// AB_PrintABookFunc
/*** AB_PrintABookFunc - Prints the entire address book in compact or detailed format ***/
HOOKPROTONHNONP(AB_PrintABookFunc, void)
{
   FILE *prt;
   int mode = MUI_Request(G->App, G->AB->GUI.WI, 0, GetStr(MSG_Print), GetStr(MSG_AB_PrintReqGads), GetStr(MSG_AB_PrintReq));
   if (!mode) return;
   if (C->PrinterCheck) if (!CheckPrinter()) return;
   if ((prt = fopen("PRT:", "w")))
   {
      Busy(GetStr(MSG_BusyPrintingAB), "", 0, 0);
      fprintf(prt, "%s\n", G->AB_Filename);
      if (mode == 2)
      {
         fprintf(prt, "\n  %-12.12s %-20.20s %s/%s\n", GetStr(MSG_AB_AliasFld), GetStr(MSG_EA_RealName), GetStr(MSG_EA_EmailAddress), GetStr(MSG_EA_Description));
         fputs("------------------------------------------------------------------------\n", prt);
      }
      AB_PrintLevel(MUIV_NListtree_GetEntry_ListNode_Root, prt, mode);
      fclose(prt);
      BusyEnd;
   }
}
MakeStaticHook(AB_PrintABookHook, AB_PrintABookFunc);

///
/// AB_PrintFunc
/*** AB_PrintFunc - Prints selected address book entry in detailed format ***/
HOOKPROTONHNONP(AB_PrintFunc, void)
{
   FILE *prt;
   struct MUI_NListtree_TreeNode *tn;
   if ((tn = (struct MUI_NListtree_TreeNode *)xget(G->AB->GUI.LV_ADDRESSES, MUIA_NListtree_Active)))
   {
      if (C->PrinterCheck) if (!CheckPrinter()) return;
      if ((prt = fopen("PRT:", "w")))
      {
         struct ABEntry *ab = (struct ABEntry *)(tn->tn_User);
         set(G->App, MUIA_Application_Sleep, TRUE);
         AB_PrintLongEntry(prt, ab);
         if (ab->Type == AET_GROUP) AB_PrintLevel(tn, prt, 1);
         fclose(prt);
         set(G->App, MUIA_Application_Sleep, FALSE);
      }
   }
}
MakeStaticHook(AB_PrintHook, AB_PrintFunc);

///
/// AB_AddEntryFunc
/*** AB_AddEntryFunc - Add a new entry to the address book ***/
HOOKPROTONHNO(AB_AddEntryFunc, void, int *arg)
{
   EA_Init(*arg, NULL);
}
MakeStaticHook(AB_AddEntryHook, AB_AddEntryFunc);

///
/// AB_DeleteFunc
/*** AB_DeleteFunc - Deletes selected address book entry ***/
HOOKPROTONHNONP(AB_DeleteFunc, void)
{
   DoMethod(G->AB->GUI.LV_ADDRESSES, MUIM_NListtree_Remove, NULL, MUIV_NListtree_Remove_TreeNode_Active, MUIF_NONE);
   G->AB->Modified = TRUE;
}
MakeHook(AB_DeleteHook, AB_DeleteFunc);

///
/// AB_DuplicateFunc
/*** AB_DuplicateFunc - Duplicates selected address book entry ***/
HOOKPROTONHNONP(AB_DuplicateFunc, void)
{
   struct MUI_NListtree_TreeNode *tn;
   if ((tn = (struct MUI_NListtree_TreeNode *)xget(G->AB->GUI.LV_ADDRESSES, MUIA_NListtree_Active)))
   {
      struct ABEntry *ab = (struct ABEntry *)(tn->tn_User);
      int winnum = EA_Init(ab->Type, NULL);
      if (winnum >= 0)
      {
         char buf[SIZE_NAME];
         int len;
         EA_Setup(winnum, ab);
         strcpy(buf, ab->Alias);
         if ((len = strlen(buf)))
         {
            if (isdigit(buf[len-1])) buf[len-1]++;
            else if (len < SIZE_NAME-1) strcat(buf, "2");
            else buf[len-1] = '2';
            setstring(G->EA[winnum]->GUI.ST_ALIAS, buf);
         }
      }
   }
}
MakeStaticHook(AB_DuplicateHook, AB_DuplicateFunc);

///
/// AB_FindEntry (rec)
//  Recursively searches an address book node for a given pattern
BOOL STACKEXT AB_FindEntry(struct MUI_NListtree_TreeNode *list, char *pattern, enum AddressbookFind mode, char **result)
{
   APTR lv = G->AB->GUI.LV_ADDRESSES;
   struct MUI_NListtree_TreeNode *tn;
   int i;

   for (i=0; ; i++)
      if ((tn = (struct MUI_NListtree_TreeNode *)DoMethod(lv, MUIM_NListtree_GetEntry, list, i, MUIF_NONE)))
      {
         struct ABEntry *ab = tn->tn_User;
         if (ab->Type == AET_GROUP)
         {
            if (!AB_FindEntry(tn, pattern, mode, result)) return FALSE;
         }
         else
         {
            int found = 0, winnum;
            switch (mode)
            {
               case ABF_RX_NAME:       if (ab->Type != AET_GROUP) found |= astcsma(ab->RealName, pattern); break;
               case ABF_RX_EMAIL:      if (ab->Type != AET_GROUP) found |= astcsma(ab->Address, pattern); break;
               case ABF_RX_NAMEEMAIL:  if (ab->Type != AET_GROUP) found |= astcsma(ab->RealName, pattern) | astcsma(ab->Address, pattern); break;
               default:
                  found |= astcsma(ab->Alias, pattern) | astcsma(ab->Comment, pattern);
                  if (ab->Type != AET_GROUP) found |= astcsma(ab->RealName, pattern) | astcsma(ab->Address, pattern);
                  if (ab->Type == AET_USER) found |= astcsma(ab->Homepage, pattern) | astcsma(ab->Street, pattern) | astcsma(ab->City, pattern) | astcsma(ab->Country, pattern) | astcsma(ab->Phone, pattern);
            }
            if (found)
            {
               G->AB->Hits++;
               if (mode == ABF_USER)
               {
                  char buf[SIZE_LARGE];
                  DoMethod(lv, MUIM_NListtree_Open, MUIV_NListtree_Open_ListNode_Parent, tn, MUIF_NONE);
                  set(lv, MUIA_NListtree_Active, tn);
                  sprintf(buf, GetStr(MSG_AB_FoundEntry), ab->Alias, ab->RealName);
                  switch (MUI_Request(G->App, G->AB->GUI.WI, 0, GetStr(MSG_AB_FindEntry), GetStr(MSG_AB_FoundEntryGads), buf))
                  {
                     case 1: break;
                     case 2: if ((winnum = EA_Init(ab->Type, ab)) >= 0) EA_Setup(winnum, ab);
                     case 0: return FALSE;
                  }
               }
               else if (result) *result++ = ab->Alias;
            }
         }
      }
      else break;
   return TRUE;
}

///
/// AB_FindFunc
/*** AB_FindFunc - Searches address book ***/
HOOKPROTONHNONP(AB_FindFunc, void)
{
   static char pattern[SIZE_PATTERN] = { 0 };

   G->AB->Hits = 0;
   if (StringRequest(pattern, SIZE_PATTERN, GetStr(MSG_AB_FindEntry), GetStr(MSG_AB_FindEntryReq), GetStr(MSG_AB_StartSearch), NULL, GetStr(MSG_Cancel), FALSE, G->AB->GUI.WI))
   {
      AB_FindEntry(MUIV_NListtree_GetEntry_ListNode_Root, pattern, ABF_USER, NULL);
      if (!G->AB->Hits) MUI_Request(G->App, G->AB->GUI.WI, 0, GetStr(MSG_AB_FindEntry), GetStr(MSG_OkayReq), GetStr(MSG_AB_NoneFound));
   }
}
MakeStaticHook(AB_FindHook, AB_FindFunc);

///
/// AB_OpenFunc
/*** AB_OpenFunc - Open address book window ***/
HOOKPROTONHNO(AB_OpenFunc, void, int *arg)
{
   struct AB_ClassData *ab = G->AB;
   char *md = "";

   switch (ab->Mode = arg[0])
   {
      case ABM_TO:      md = "(To)";      break;
      case ABM_CC:      md = "(CC)";      break;
      case ABM_BCC:     md = "(BCC)";     break;
      case ABM_FROM:    md = "(From)";    break;
      case ABM_REPLYTO: md = "(Reply-To)";break;
      default:
        // nothing
      break;
   }
   ab->WrWin = *md ? arg[1] : -1;
   ab->Modified = FALSE;
   sprintf(ab->WTitle, "%s %s", GetStr(MSG_MA_MAddrBook), md);
   set(ab->GUI.WI, MUIA_Window_Title, ab->WTitle);
   set(ab->GUI.LV_ADDRESSES, MUIA_NListtree_Active, MUIV_NListtree_Active_Off);
   SafeOpenWindow(ab->GUI.WI);
}
MakeHook(AB_OpenHook, AB_OpenFunc);

///
/// AB_Close
/*** AB_Close - Closes address book window ***/
HOOKPROTONHNONP(AB_Close, void)
{
   if (G->AB->Modified) switch (MUI_Request(G->App, G->AB->GUI.WI, 0, NULL, GetStr(MSG_AB_ModifiedGads), GetStr(MSG_AB_Modified)))
   {
      case 0: return;
      case 1: AB_SaveABookFunc(); break;
      case 2: break;
      case 3: AB_LoadTree(G->AB_Filename, FALSE, FALSE);
   }
   set(G->AB->GUI.WI, MUIA_Window_Open, FALSE);
}
MakeStaticHook(AB_CloseHook, AB_Close);

///
/// AB_LV_ConFunc
/*** AB_LV_ConFunc - Address book listview construction hook ***/
HOOKPROTONHNO(AB_LV_ConFunc, struct ABEntry *, struct MUIP_NListtree_ConstructMessage *msg)
{
   struct ABEntry *entry = malloc(sizeof(struct ABEntry));

   if (entry && msg)
   {
      struct ABEntry *addr = (struct ABEntry *)msg->UserData;

      memcpy(entry, addr, sizeof(struct ABEntry));
      if (addr->Members) strcpy(entry->Members = malloc(strlen(addr->Members)+1), addr->Members);
   }

   return entry;
}
MakeStaticHook(AB_LV_ConFuncHook, AB_LV_ConFunc);

///
/// AB_LV_DesFunc
/*** AB_LV_DesFunc - Address book listview destruction hook ***/
HOOKPROTONHNO(AB_LV_DesFunc, long, struct MUIP_NListtree_DestructMessage *msg)
{
   struct ABEntry *entry;

   if(msg)
   {
      entry = (struct ABEntry *)msg->UserData;

      if(entry)
      {
        if (entry->Members) free(entry->Members);
        free(entry);
      }
   }

   return 0;
}
MakeStaticHook(AB_LV_DesFuncHook, AB_LV_DesFunc);

///
/// AB_LV_DspFunc
/*** AB_LV_DspFunc - Address book listview display hook ***/
HOOKPROTONHNO(AB_LV_DspFunc, long, struct MUIP_NListtree_DisplayMessage *msg)
{
   static char dispal[SIZE_DEFAULT];

   if (msg && msg->TreeNode)
   {
      struct ABEntry *entry = msg->TreeNode->tn_User;

      if (entry)
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
              sprintf(msg->Array[0] = dispal, "\033o[0] %s", entry->Alias);
            }
            break;

            case AET_GROUP:
            {
              msg->Preparse[0] = MUIX_B;
              msg->Preparse[2] = MUIX_B;
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
      msg->Array[0] = GetStr(MSG_AB_TitleAlias);
      msg->Array[1] = GetStr(MSG_AB_TitleName);
      msg->Array[2] = GetStr(MSG_AB_TitleDescription);
      msg->Array[3] = GetStr(MSG_AB_TitleAddress);
      msg->Array[4] = GetStr(MSG_AB_TitleStreet);
      msg->Array[5] = GetStr(MSG_AB_TitleCity);
      msg->Array[6] = GetStr(MSG_AB_TitleCountry);
      msg->Array[7] = GetStr(MSG_AB_TitlePhone);
      msg->Array[8] = GetStr(MSG_AB_TitleBirthDate);
      msg->Array[9] = GetStr(MSG_AB_TitlePGPId);
      msg->Array[10]= GetStr(MSG_AB_TitleHomepage);
   }
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

   if(!msg) return 0;

   // now we get the entries
   entry1 = msg->TreeNode1;
   entry2 = msg->TreeNode2;
   if(!entry1 || !entry2) return 0;

   ab1 = (struct ABEntry *)entry1->tn_User;
   ab2 = (struct ABEntry *)entry2->tn_User;
   if(!ab1 || !ab2) return 0;

   switch (G->AB->SortBy)
   {
      case 1: if (!(n1 = strrchr(ab1->RealName,' '))) n1 = ab1->RealName;
              if (!(n2 = strrchr(ab2->RealName,' '))) n2 = ab2->RealName;
              if ((cmp = Stricmp(n1, n2))) return cmp;
              break;
      case 2: if ((cmp = Stricmp(ab1->RealName, ab2->RealName))) return cmp;
              break;
      case 3: if ((cmp = Stricmp(ab1->Comment, ab2->Comment))) return cmp;
              break;
      case 4: if ((cmp = Stricmp(ab1->Address, ab2->Address))) return cmp;
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
   *format = 0;
   for (i = 0; i < ABCOLNUM; i++) if (C->AddrbookCols & (1<<i))
   {
      if (first) first = FALSE; else strcat(format, " BAR,");
      sprintf(&format[strlen(format)], "COL=%d W=-1", i);
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
      enum {
        AMEN_NEW,AMEN_OPEN,AMEN_APPEND,AMEN_SAVE,AMEN_SAVEAS,AMEN_PRINTA,
        AMEN_FIND,AMEN_NEWUSER,AMEN_NEWLIST,AMEN_NEWGROUP,AMEN_EDIT,
        AMEN_DUPLICATE,AMEN_DELETE,AMEN_PRINTE,AMEN_SORTALIAS,
        AMEN_SORTLNAME,AMEN_SORTFNAME,AMEN_SORTDESC,AMEN_SORTADDR,
        AMEN_FOLD,AMEN_UNFOLD
      };

      static const struct NewToolbarEntry tb_butt[ARRAY_SIZE(data->GUI.TB_TOOLBAR)] = {
        { MSG_AB_TBSave,      MSG_HELP_AB_BT_SAVE     },
        { MSG_AB_TBFind,      MSG_HELP_AB_BT_SEARCH   },
        { MSG_Space,          NULL                    },
        { MSG_AB_TBNewUser,   MSG_HELP_AB_BT_ADDUSER  },
        { MSG_AB_TBNewList,   MSG_HELP_AB_BT_ADDMLIST },
        { MSG_AB_TBNewGroup,  MSG_HELP_AB_BT_ADDGROUP },
        { MSG_AB_TBEdit,      MSG_HELP_AB_BT_EDIT     },
        { MSG_AB_TBDelete,    MSG_HELP_AB_BT_DELETE,  },
        { MSG_AB_TBPrint,     MSG_HELP_AB_BT_PRINT,   },
        { MSG_Space,          NULL                    },
        { MSG_AB_TBOpenTree,  MSG_HELP_AB_BT_OPEN     },
        { MSG_AB_TBCloseTree, MSG_HELP_AB_BT_CLOSE    },
        { NULL              , NULL                    }
      };
      APTR list;
      ULONG i;

      for(i = 0; i < ARRAY_SIZE(data->GUI.TB_TOOLBAR); i++)
      {
        SetupToolbar(&(data->GUI.TB_TOOLBAR[i]), tb_butt[i].label?(tb_butt[i].label==MSG_Space?"":GetStr(tb_butt[i].label)):NULL, tb_butt[i].help?GetStr(tb_butt[i].help):NULL, 0);
      }

      data->GUI.WI = WindowObject,
         MUIA_HelpNode,"AB_W",
         MUIA_Window_Menustrip, MenustripObject,
            MUIA_Family_Child, MenuObject, MUIA_Menu_Title, GetStr(MSG_CO_CrdABook),
               MUIA_Family_Child, MenuitemObject, MUIA_Menuitem_Title,GetStr(MSG_New), MUIA_Menuitem_Shortcut,"N", MUIA_UserData,AMEN_NEW, End,
               MUIA_Family_Child, MenuitemObject, MUIA_Menuitem_Title,GetStr(MSG_Open), MUIA_Menuitem_Shortcut,"O", MUIA_UserData,AMEN_OPEN, End,
               MUIA_Family_Child, MenuitemObject, MUIA_Menuitem_Title,GetStr(MSG_Append), MUIA_Menuitem_Shortcut,"I", MUIA_UserData,AMEN_APPEND, End,
               MUIA_Family_Child, MenuitemObject, MUIA_Menuitem_Title,NM_BARLABEL, End,
               MUIA_Family_Child, MenuitemObject, MUIA_Menuitem_Title,GetStr(MSG_Save), MUIA_Menuitem_Shortcut,"S", MUIA_UserData,AMEN_SAVE, End,
               MUIA_Family_Child, MenuitemObject, MUIA_Menuitem_Title,GetStr(MSG_SaveAs), MUIA_Menuitem_Shortcut,"A", MUIA_UserData,AMEN_SAVEAS, End,
               MUIA_Family_Child, MenuitemObject, MUIA_Menuitem_Title,NM_BARLABEL, End,
               MUIA_Family_Child, MenuitemObject, MUIA_Menuitem_Title,GetStr(MSG_AB_MIFind), MUIA_Menuitem_Shortcut,"F", MUIA_UserData,AMEN_FIND, End,
               MUIA_Family_Child, MenuitemObject, MUIA_Menuitem_Title,GetStr(MSG_Print), MUIA_UserData,AMEN_PRINTA, End,
            End,
            MUIA_Family_Child, MenuObject, MUIA_Menu_Title, GetStr(MSG_AB_Entry),
               MUIA_Family_Child, MenuitemObject, MUIA_Menuitem_Title,GetStr(MSG_AB_AddUser), MUIA_Menuitem_Shortcut,"P", MUIA_UserData,AMEN_NEWUSER, End,
               MUIA_Family_Child, MenuitemObject, MUIA_Menuitem_Title,GetStr(MSG_AB_AddList), MUIA_Menuitem_Shortcut,"L", MUIA_UserData,AMEN_NEWLIST, End,
               MUIA_Family_Child, MenuitemObject, MUIA_Menuitem_Title,GetStr(MSG_AB_AddGroup), MUIA_Menuitem_Shortcut,"G", MUIA_UserData,AMEN_NEWGROUP, End,
               MUIA_Family_Child, MenuitemObject, MUIA_Menuitem_Title,NM_BARLABEL, End,
               MUIA_Family_Child, MenuitemObject, MUIA_Menuitem_Title,GetStr(MSG_Edit), MUIA_Menuitem_Shortcut,"E", MUIA_UserData,AMEN_EDIT, End,
               MUIA_Family_Child, MenuitemObject, MUIA_Menuitem_Title,GetStr(MSG_AB_Duplicate), MUIA_Menuitem_Shortcut,"D", MUIA_UserData,AMEN_DUPLICATE, End,
               MUIA_Family_Child, MenuitemObject, MUIA_Menuitem_Title,GetStr(MSG_AB_MIDelete), MUIA_Menuitem_Shortcut,"Del", MUIA_Menuitem_CommandString,TRUE, MUIA_UserData,AMEN_DELETE, End,
               MUIA_Family_Child, MenuitemObject, MUIA_Menuitem_Title,NM_BARLABEL, End,
               MUIA_Family_Child, MenuitemObject, MUIA_Menuitem_Title,GetStr(MSG_AB_MIPrint), MUIA_UserData,AMEN_PRINTE, End,
            End,
            MUIA_Family_Child, MenuObject, MUIA_Menu_Title, GetStr(MSG_AB_Sort),
               MUIA_Family_Child, MenuitemObject, MUIA_Menuitem_Title,GetStr(MSG_AB_SortByAlias), MUIA_Menuitem_Shortcut,"1", MUIA_UserData,AMEN_SORTALIAS, End,
               MUIA_Family_Child, MenuitemObject, MUIA_Menuitem_Title,GetStr(MSG_AB_SortByName), MUIA_Menuitem_Shortcut,"2", MUIA_UserData,AMEN_SORTLNAME, End,
               MUIA_Family_Child, MenuitemObject, MUIA_Menuitem_Title,GetStr(MSG_AB_SortByFirstname), MUIA_Menuitem_Shortcut,"3", MUIA_UserData,AMEN_SORTFNAME, End,
               MUIA_Family_Child, MenuitemObject, MUIA_Menuitem_Title,GetStr(MSG_AB_SortByDesc), MUIA_Menuitem_Shortcut,"4", MUIA_UserData,AMEN_SORTDESC, End,
               MUIA_Family_Child, MenuitemObject, MUIA_Menuitem_Title,GetStr(MSG_AB_SortByAddress), MUIA_Menuitem_Shortcut,"5", MUIA_UserData,AMEN_SORTADDR, End,
            End,
            MUIA_Family_Child, MenuObject, MUIA_Menu_Title, GetStr(MSG_AB_View),
               MUIA_Family_Child, MenuitemObject, MUIA_Menuitem_Title,GetStr(MSG_AB_Unfold), MUIA_Menuitem_Shortcut,"<", MUIA_UserData,AMEN_UNFOLD, End,
               MUIA_Family_Child, MenuitemObject, MUIA_Menuitem_Title,GetStr(MSG_AB_Fold), MUIA_Menuitem_Shortcut,">", MUIA_UserData,AMEN_FOLD, End,
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
                  Child, HGroupV,
                     Child, data->GUI.TO_TOOLBAR = ToolbarObject,
                        MUIA_Toolbar_ImageType,      MUIV_Toolbar_ImageType_File,
                        MUIA_Toolbar_ImageNormal,    "PROGDIR:Icons/Address.toolbar",
                        MUIA_Toolbar_ImageGhost,     "PROGDIR:Icons/Address_G.toolbar",
                        MUIA_Toolbar_ImageSelect,    "PROGDIR:Icons/Address_S.toolbar",
                        MUIA_Toolbar_Description,    data->GUI.TB_TOOLBAR,
                        MUIA_Toolbar_ParseUnderscore,TRUE,
                        MUIA_Font,                   MUIV_Font_Tiny,
                        MUIA_ShortHelp, TRUE,
                     End,
                     Child, HSpace(0),
                  End,
               End),

            Child, list = NListviewObject,
               MUIA_CycleChain,         TRUE,
               MUIA_Listview_DragType,  MUIV_Listview_DragType_Immediate,
               MUIA_NListview_NList,    data->GUI.LV_ADDRESSES = NewObject(CL_AddressList->mcc_Class, NULL,
                  InputListFrame,
                  MUIA_NListtree_CompareHook,     &AB_LV_CmpFuncHook,
                  MUIA_NListtree_DragDropSort,    TRUE,
                  MUIA_NListtree_Title,           TRUE,
                  MUIA_NListtree_ConstructHook,   &AB_LV_ConFuncHook,
                  MUIA_NListtree_DestructHook,    &AB_LV_DesFuncHook,
                  MUIA_NListtree_DisplayHook,     &AB_LV_DspFuncHook,
                  MUIA_NListtree_EmptyNodes,      TRUE,
                  MUIA_Font,                      C->FixedFontList ? MUIV_Font_Fixed : MUIV_Font_List,
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

        // Now we add the group image to the NListtree
        DoMethod(data->GUI.LV_ADDRESSES, MUIM_NList_UseImage, G->MA->GUI.BC_STAT[11], 0, MUIF_NONE);

        DoMethod(data->GUI.WI         ,MUIM_Notify,MUIA_Window_MenuAction   ,AMEN_NEW      ,MUIV_Notify_Application,3,MUIM_CallHook,&AB_NewABookHook,0);
        DoMethod(data->GUI.WI         ,MUIM_Notify,MUIA_Window_MenuAction   ,AMEN_OPEN     ,MUIV_Notify_Application,3,MUIM_CallHook,&AB_OpenABookHook,0);
        DoMethod(data->GUI.WI         ,MUIM_Notify,MUIA_Window_MenuAction   ,AMEN_APPEND   ,MUIV_Notify_Application,3,MUIM_CallHook,&AB_AppendABookHook,0);
        DoMethod(data->GUI.WI         ,MUIM_Notify,MUIA_Window_MenuAction   ,AMEN_SAVE     ,MUIV_Notify_Application,3,MUIM_CallHook,&AB_SaveABookHook,0);
        DoMethod(data->GUI.WI         ,MUIM_Notify,MUIA_Window_MenuAction   ,AMEN_SAVEAS   ,MUIV_Notify_Application,3,MUIM_CallHook,&AB_SaveABookAsHook,0);
        DoMethod(data->GUI.WI         ,MUIM_Notify,MUIA_Window_MenuAction   ,AMEN_PRINTA   ,MUIV_Notify_Application,3,MUIM_CallHook,&AB_PrintABookHook,0);
        DoMethod(data->GUI.WI         ,MUIM_Notify,MUIA_Window_MenuAction   ,AMEN_NEWUSER  ,MUIV_Notify_Application,3,MUIM_CallHook,&AB_AddEntryHook,AET_USER);
        DoMethod(data->GUI.WI         ,MUIM_Notify,MUIA_Window_MenuAction   ,AMEN_NEWLIST  ,MUIV_Notify_Application,3,MUIM_CallHook,&AB_AddEntryHook,AET_LIST);
        DoMethod(data->GUI.WI         ,MUIM_Notify,MUIA_Window_MenuAction   ,AMEN_NEWGROUP ,MUIV_Notify_Application,3,MUIM_CallHook,&AB_AddEntryHook,AET_GROUP);
        DoMethod(data->GUI.WI         ,MUIM_Notify,MUIA_Window_MenuAction   ,AMEN_EDIT     ,MUIV_Notify_Application,3,MUIM_CallHook,&AB_EditHook,0);
        DoMethod(data->GUI.WI         ,MUIM_Notify,MUIA_Window_MenuAction   ,AMEN_DUPLICATE,MUIV_Notify_Application,3,MUIM_CallHook,&AB_DuplicateHook,0);
        DoMethod(data->GUI.WI         ,MUIM_Notify,MUIA_Window_MenuAction   ,AMEN_DELETE   ,MUIV_Notify_Application,3,MUIM_CallHook,&AB_DeleteHook,0);
        DoMethod(data->GUI.WI         ,MUIM_Notify,MUIA_Window_MenuAction   ,AMEN_PRINTE   ,MUIV_Notify_Application,3,MUIM_CallHook,&AB_PrintHook,0);
        DoMethod(data->GUI.WI         ,MUIM_Notify,MUIA_Window_MenuAction   ,AMEN_FIND     ,MUIV_Notify_Application,3,MUIM_CallHook,&AB_FindHook,0);
        DoMethod(data->GUI.WI         ,MUIM_Notify,MUIA_Window_MenuAction   ,AMEN_SORTALIAS,MUIV_Notify_Application,3,MUIM_CallHook,&AB_SortHook,0);
        DoMethod(data->GUI.WI         ,MUIM_Notify,MUIA_Window_MenuAction   ,AMEN_SORTLNAME,MUIV_Notify_Application,3,MUIM_CallHook,&AB_SortHook,1);
        DoMethod(data->GUI.WI         ,MUIM_Notify,MUIA_Window_MenuAction   ,AMEN_SORTFNAME,MUIV_Notify_Application,3,MUIM_CallHook,&AB_SortHook,2);
        DoMethod(data->GUI.WI         ,MUIM_Notify,MUIA_Window_MenuAction   ,AMEN_SORTDESC ,MUIV_Notify_Application,3,MUIM_CallHook,&AB_SortHook,3);
        DoMethod(data->GUI.WI         ,MUIM_Notify,MUIA_Window_MenuAction   ,AMEN_SORTADDR ,MUIV_Notify_Application,3,MUIM_CallHook,&AB_SortHook,4);
        DoMethod(data->GUI.WI         ,MUIM_Notify,MUIA_Window_MenuAction   ,AMEN_FOLD     ,data->GUI.LV_ADDRESSES,4,MUIM_NListtree_Close ,NULL,MUIV_NListtree_Close_TreeNode_All, MUIF_NONE);
        DoMethod(data->GUI.WI         ,MUIM_Notify,MUIA_Window_MenuAction   ,AMEN_UNFOLD   ,data->GUI.LV_ADDRESSES,4,MUIM_NListtree_Open  ,NULL,MUIV_NListtree_Open_TreeNode_All, MUIF_NONE);
        DoMethod(data->GUI.LV_ADDRESSES,MUIM_Notify,MUIA_NListtree_DoubleClick,MUIV_EveryTime,MUIV_Notify_Application,3,MUIM_CallHook,&AB_DoubleClickHook,0);
        DoMethod(data->GUI.BT_TO      ,MUIM_Notify,MUIA_Pressed    ,FALSE,MUIV_Notify_Application           ,3,MUIM_CallHook       ,&AB_FromAddrBookHook,ABM_TO);
        DoMethod(data->GUI.BT_CC      ,MUIM_Notify,MUIA_Pressed    ,FALSE,MUIV_Notify_Application           ,3,MUIM_CallHook       ,&AB_FromAddrBookHook,ABM_CC);
        DoMethod(data->GUI.BT_BCC     ,MUIM_Notify,MUIA_Pressed    ,FALSE,MUIV_Notify_Application           ,3,MUIM_CallHook       ,&AB_FromAddrBookHook,ABM_BCC);

        if (data->GUI.TO_TOOLBAR)
        {
          DoMethod(data->GUI.TO_TOOLBAR ,MUIM_Toolbar_Notify, 0, MUIV_Toolbar_Notify_Pressed,FALSE,G->App,3,MUIM_CallHook,&AB_SaveABookHook,0);
          DoMethod(data->GUI.TO_TOOLBAR ,MUIM_Toolbar_Notify, 1, MUIV_Toolbar_Notify_Pressed,FALSE,G->App,3,MUIM_CallHook,&AB_FindHook,0);
          DoMethod(data->GUI.TO_TOOLBAR ,MUIM_Toolbar_Notify, 3, MUIV_Toolbar_Notify_Pressed,FALSE,G->App,3,MUIM_CallHook,&AB_AddEntryHook,AET_USER);
          DoMethod(data->GUI.TO_TOOLBAR ,MUIM_Toolbar_Notify, 4, MUIV_Toolbar_Notify_Pressed,FALSE,G->App,3,MUIM_CallHook,&AB_AddEntryHook,AET_LIST);
          DoMethod(data->GUI.TO_TOOLBAR ,MUIM_Toolbar_Notify, 5, MUIV_Toolbar_Notify_Pressed,FALSE,G->App,3,MUIM_CallHook,&AB_AddEntryHook,AET_GROUP);
          DoMethod(data->GUI.TO_TOOLBAR ,MUIM_Toolbar_Notify, 6, MUIV_Toolbar_Notify_Pressed,FALSE,G->App,3,MUIM_CallHook,&AB_EditHook,0);
          DoMethod(data->GUI.TO_TOOLBAR ,MUIM_Toolbar_Notify, 7, MUIV_Toolbar_Notify_Pressed,FALSE,G->App,3,MUIM_CallHook,&AB_DeleteHook,0);
          DoMethod(data->GUI.TO_TOOLBAR ,MUIM_Toolbar_Notify, 8, MUIV_Toolbar_Notify_Pressed,FALSE,G->App,3,MUIM_CallHook,&AB_PrintHook,0);
          DoMethod(data->GUI.TO_TOOLBAR ,MUIM_Toolbar_Notify,10, MUIV_Toolbar_Notify_Pressed,FALSE,data->GUI.LV_ADDRESSES,4,MUIM_NListtree_Open  ,NULL,MUIV_NListtree_Open_TreeNode_All, MUIF_NONE);
          DoMethod(data->GUI.TO_TOOLBAR ,MUIM_Toolbar_Notify,11, MUIV_Toolbar_Notify_Pressed,FALSE,data->GUI.LV_ADDRESSES,4,MUIM_NListtree_Close ,NULL,MUIV_NListtree_Close_TreeNode_All, MUIF_NONE);
        }

        DoMethod(data->GUI.WI,MUIM_Notify,MUIA_Window_InputEvent   ,"-repeat -capslock del" ,MUIV_Notify_Application  ,2,MUIM_CallHook       ,&AB_DeleteHook);
        DoMethod(data->GUI.WI,MUIM_Notify,MUIA_Window_CloseRequest ,TRUE ,MUIV_Notify_Application           ,2,MUIM_CallHook       ,&AB_CloseHook);

        return data;
      }
      free(data);
   }
   return NULL;
}
///
