/***************************************************************************

 YAM - Yet Another Mailer
 Copyright (C) 1995-2000 by Marcel Beck <mbeck@yam.ch>
 Copyright (C) 2000-2001 by YAM Open Source Team

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

#include "YAM.h"

/* local protos */
LOCAL void FI_MakeSubstringPattern(char*);
LOCAL BOOL FI_MatchString(struct Search*, char*);
LOCAL BOOL FI_MatchListPattern(struct Search*, char*);
LOCAL BOOL FI_MatchPerson(struct Search*, struct Person*);
LOCAL BOOL FI_SearchPatternFast(struct Search*, struct Mail*);
LOCAL BOOL FI_SearchPatternInBody(struct Search*, struct Mail*);
LOCAL BOOL FI_SearchPatternInHeader(struct Search*, struct Mail*);
LOCAL int FI_IsFastSearch(char*);
LOCAL void FI_GenerateListPatterns(struct Search*);
LOCAL BOOL FI_DoSearch(struct Search*, struct Mail*);
LOCAL struct FI_ClassData *FI_New(void);


/***************************************************************************
 Module: Find
***************************************************************************/

/// Global variables
int Mode2Group[12] = { 0,0,0,0,1,2,1,2,4,4,4,3 };

///
/// FI_MakeSubstringPattern
//  Creates pattern for substring search
LOCAL void FI_MakeSubstringPattern(char *pattern)
{
   char npattern[SIZE_PATTERN];
   sprintf(npattern, "#?%s#?", pattern);
   strcpy(pattern, npattern);
}

///
/// FI_MatchString
//  Matches string against pattern
LOCAL BOOL FI_MatchString(struct Search *search, char *string)
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
LOCAL BOOL FI_MatchListPattern(struct Search *search, char *string)
{
   int i;
   for (i = 0; i < search->List.Used; i++)
      if (search->CaseSens ? MatchPattern(search->List.Data[i], string)
                           : MatchPatternNoCase(search->List.Data[i], string)) return TRUE;
   return FALSE;
}
///
/// FI_MatchPerson
//  Matches string against a person's name or address
LOCAL BOOL FI_MatchPerson(struct Search *search, struct Person *pe)
{
   if (search->Compare == 4) return FI_MatchListPattern(search, search->PersMode ? pe->RealName : pe->Address);
                        else return FI_MatchString(search, search->PersMode ? pe->RealName : pe->Address);
}
///
/// FI_SearchPatternFast
//  Searches string in standard header fields
LOCAL BOOL FI_SearchPatternFast(struct Search *search, struct Mail *mail)
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
         if (mail->Flags & MFLAG_MULTIRCPT) if (email = MA_ExamineMail(mail->Folder, mail->MailFile, NULL, TRUE))
         {
            for (j = 0; j < email->NoSTo; j++) if (FI_MatchPerson(search, &email->STo[j])) found = TRUE;
            MA_FreeEMailStruct(email);
         }
         break;
      case FS_CC:
         if (mail->Flags & MFLAG_MULTIRCPT) if (email = MA_ExamineMail(mail->Folder, mail->MailFile, NULL, TRUE))
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

   }
   return found;
}

///
/// FI_SearchPatternInBody
//  Searches string in message body
LOCAL BOOL FI_SearchPatternInBody(struct Search *search, struct Mail *mail)
{
   char *rptr, *ptr, *cmsg;
   BOOL found = FALSE;

   RE_InitPrivateRC(mail, PM_TEXTS);
   rptr = cmsg = RE_ReadInMessage(4, RIM_QUIET);
   while (*rptr && !found && !G->FI->Abort)
   {
      DoMethod(G->App,MUIM_Application_InputBuffered);
      for (ptr = rptr; *ptr && *ptr != '\n'; *ptr++); *ptr = 0;
      if (FI_MatchString(search, rptr)) found = TRUE;
      rptr = ++ptr;
   }
   free(cmsg);
   RE_FreePrivateRC();
   return found;
}

///
/// FI_SearchPatternInHeader
//  Searches string in header field(s)
LOCAL BOOL FI_SearchPatternInHeader(struct Search *search, struct Mail *mail)
{
   char *rptr, *line, fullfile[SIZE_PATHFILE];
   BOOL found = FALSE;
   FILE *fh;
   int i;

   if (StartUnpack(GetMailFile(NULL, mail->Folder, mail), fullfile, mail->Folder))
   {
      if (fh = fopen(fullfile, "r"))
      {
         MA_ReadHeader(fh);
        for (i = 0; i < Header.Used && !found; i++)
         {
            DoMethod(G->App,MUIM_Application_InputBuffered);
            rptr = line = Header.Data[i];
            if (*search->Field)
               if (Strnicmp(line, search->Field, strlen(search->Field))) continue;
               else rptr = Trim(&line[strlen(search->Field)+1]);
            if ((search->Compare == 4) ? FI_MatchListPattern(search, rptr) : FI_MatchString(search, rptr)) found = TRUE;
         }
         FreeData2D(&Header);
         fclose(fh);
      }
      FinishUnpack(fullfile);
   }
   return found;
}

///
/// FI_IsFastSearch
//  Checks if quick search is available for selected header field
LOCAL int FI_IsFastSearch(char *field)
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
LOCAL void FI_GenerateListPatterns(struct Search *search)
{
   char buf[SIZE_PATTERN], pattern[SIZE_PATTERN];
   FILE *fh;
   if (fh = fopen(search->Match, "r"))
   {
      FreeData2D(&(search->List));
      while (GetLine(fh, buf, SIZE_PATTERN)) if (*buf)
      {
         if (search->CaseSens) ParsePattern      (buf, pattern, SIZE_PATTERN);
         else                  ParsePatternNoCase(buf, pattern, SIZE_PATTERN);
         strcpy(AllocData2D(&search->List, SIZE_PATTERN), pattern);
      }
      fclose(fh);
   }
}

///
/// FI_PrepareSearch
//  Initializes Search structure
BOOL FI_PrepareSearch(struct Search *search, int mode, BOOL casesens, int persmode, int compar, int stat, BOOL substr, char *match, char *field)
{
   // return value of this function isn't used currently (21.03.2001)
   clear(search, sizeof(struct Search));
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
   switch (mode)
   {
      case 0:  search->Fast = FS_FROM; break;
      case 1:  search->Fast = FS_TO; break;
      case 2:  search->Fast = FS_CC; break;
      case 3:  search->Fast = FS_REPLYTO; break;
      case 4:  search->Fast = FS_SUBJECT; break;
      case 5:  search->Fast = FS_DATE; break;
      case 6:  search->Fast = FI_IsFastSearch(field); break;
      case 7:  search->Fast = FS_SIZE; break;
      case 8: case 10: *search->Field = 0;
   }
   if (search->Fast == FS_DATE)
   {
      char *time;
      search->DT.dat_Format = FORMAT_DOS;
      search->DT.dat_StrDate = match;
      search->DT.dat_StrTime = (time = strchr(match,' ')) ? time+1 : "00:00:00";
      if (!StrToDate(&(search->DT)))
      {
         ER_NewError(GetStr(MSG_ER_ErrorDateFormat), DateStamp2String(NULL,DSS_DATE), NULL);
         return FALSE;
      };
      search->Pattern = (char *)&(search->DT.dat_Stamp);
   }
   if (search->Fast == FS_SIZE)
   {
      search->Size = atol(match);
   }
   if (compar == 4) FI_GenerateListPatterns(search);
   else if (search->Fast != FS_DATE && search->Fast != FS_SIZE && mode != 11)
   {
      if (substr || mode >= 8) FI_MakeSubstringPattern(search->Match);
      if (casesens) ParsePattern      (search->Match, search->Pattern, SIZE_PATTERN);
      else          ParsePatternNoCase(search->Match, search->Pattern, SIZE_PATTERN);
   }

   return FALSE;
}

///
/// FI_DoSearch
//  Checks if a message fulfills the search criteria
LOCAL BOOL FI_DoSearch(struct Search *search, struct Mail *mail)
{
   BOOL found0, found = FALSE;
   int comp_bak = search->Compare, mstat;
   switch (search->Mode)
   {
      case 0: case 1: case 2: case 3: case 4: case 5: case 6:  case 7:
         found = search->Fast == FS_NONE ? FI_SearchPatternInHeader(search, mail)
                                         : FI_SearchPatternFast(search, mail);
               break;
      case 8:  search->Compare = 0;
               found0 = FI_SearchPatternInHeader(search, mail);
               search->Compare = comp_bak;
               if (found0 == (search->Compare == 0)) found = TRUE;
               break;
      case 9:  search->Compare = 0;
               found0 = FI_SearchPatternInBody(search, mail);
               search->Compare = comp_bak;
               if (found0 == (search->Compare == 0)) found = TRUE;
               break;
      case 10: search->Compare = 0;
               if (!(found0 = FI_SearchPatternInHeader(search, mail)))
               found0 = FI_SearchPatternInBody(search, mail);
               search->Compare = comp_bak;
               if (found0 == (search->Compare == 0)) found = TRUE;
               break;
      case 11: if ((mstat = mail->Status) == 8) mstat = 0; /* new=unread */
               if ((search->Compare == 0 && search->Status == mstat) || (search->Compare == 1 && search->Status != mstat)) found = TRUE;
               break;
   }
   return found;
}

///
/// FI_DoComplexSearch
//  Does a complex search with two combined criteria
BOOL FI_DoComplexSearch(struct Search *search1, int combine, struct Search *search2, struct Mail *mail)
{
   BOOL found1;
   found1 = FI_DoSearch(search1, mail);
   switch (combine)
   {
      case 0: return found1;
      case 1: if (found1) return TRUE;
              return FI_DoSearch(search2, mail);
      case 2: if (!found1) return FALSE;
              return FI_DoSearch(search2, mail);
      case 3: return (BOOL)(found1 != FI_DoSearch(search2, mail));
   }

   return FALSE;
}

///
/// FI_SearchFunc
//  Starts the search and shows progress
void SAVEDS FI_SearchFunc(void)
{
   int pg, sfonum = 0, fnr, id, i, fndmsg = 0, totmsg = 0, progress = 0;
   struct FI_GUIData *gui = &G->FI->GUI;
   struct Folder **sfo, *folder;
   char *name, *match, *field;
   static char gauge[40];
   struct Mail *mail;
   APTR ga = gui->GA_PROGRESS;
   struct SearchGroup *gdata = &gui->GR_SEARCH;
   struct Search search;

   set(gui->WI, MUIA_Window_ActiveObject, MUIV_Window_ActiveObject_None);
   set(gui->BT_SELECT, MUIA_Disabled, TRUE);
   set(gui->BT_READ, MUIA_Disabled, TRUE);
   DoMethod(gui->LV_MAILS, MUIM_NList_Clear);
   get(gui->LV_FOLDERS, MUIA_List_Entries, &fnr);
   sfo = calloc(fnr, sizeof(struct Folder *));
   id = MUIV_List_NextSelected_Start;
   while (TRUE)
   {
      DoMethod(gui->LV_FOLDERS, MUIM_List_NextSelected, &id);
      if (id == MUIV_List_NextSelected_End) break;
      DoMethod(gui->LV_FOLDERS, MUIM_List_GetEntry, id, &name);
      if (folder = FO_GetFolderByName(name, NULL)) if (MA_GetIndex(folder))
      {
         sfo[sfonum++] = folder;
         totmsg += folder->Total;
      }
   }
   if (!totmsg) { free(sfo); return; }
   get(gdata->PG_SRCHOPT, MUIA_Group_ActivePage, &pg);
   get(gdata->ST_MATCH[pg], MUIA_String_Contents, &match);
   get(gdata->ST_FIELD, MUIA_String_Contents, &field);
   FI_PrepareSearch(&search, GetMUICycle(gdata->CY_MODE),
      GetMUICheck(gdata->CH_CASESENS[pg]), GetMUIRadio(gdata->RA_ADRMODE),
      GetMUICycle(gdata->CY_COMP[pg]), GetMUICycle(gdata->CY_STATUS),
      pg < 2 ? GetMUICheck(gdata->CH_SUBSTR[pg]) : (pg == 4 ? TRUE : FALSE),
      match, field);
   SPrintF(gauge, GetStr(MSG_FI_GaugeText), totmsg);
   set(ga, MUIA_Gauge_InfoText, gauge);
   set(ga, MUIA_Gauge_Max, totmsg);
   set(ga, MUIA_Gauge_Current, 0);
   set(gui->GR_PAGE, MUIA_Group_ActivePage, 1);
   G->FI->Abort = FALSE;
   for (i = 0; i < sfonum && !G->FI->Abort; i++)
   {
      for (mail = sfo[i]->Messages; mail && !G->FI->Abort; mail = mail->Next)
      {
         DoMethod(G->App,MUIM_Application_InputBuffered);
         if (FI_DoSearch(&search, mail))
         {
            DoMethod(gui->LV_MAILS, MUIM_NList_InsertSingle, mail, MUIV_NList_Insert_Bottom);
            fndmsg++;
         }
         set(ga, MUIA_Gauge_Current, ++progress);
      }
   }
   FreeData2D(&(search.List));
   free(sfo);
   set(gui->GR_PAGE, MUIA_Group_ActivePage, 0);
   set(gui->BT_SELECT, MUIA_Disabled, !fndmsg);
   set(gui->BT_READ, MUIA_Disabled, !fndmsg);
}
MakeHook(FI_SearchHook,FI_SearchFunc);

///
/// FI_ToRuleFunc
//  Creates a filter from the current search options
void SAVEDS FI_ToRuleFunc(void)
{
   int ch, i, r = -1;

   for (i = 0; i < MAXRU; i++) if (!C->RU[i]) { r = i; break; }
   if (r >= 0)
   {
      char name[SIZE_NAME];
      *name = 0;
      if (ch = StringRequest(name, SIZE_NAME, GetStr(MSG_FI_AddFilter), GetStr(MSG_FI_AddFilterReq), GetStr(MSG_Save), GetStr(MSG_Use), GetStr(MSG_Cancel), FALSE, G->FI->GUI.WI))
         if (C->RU[r] = CO_NewRule())
         {
            struct SearchGroup *grp = &(G->FI->GUI.GR_SEARCH);
            int g;
            get(grp->PG_SRCHOPT, MUIA_Group_ActivePage, &g);
            strcpy(C->RU[r]->Name, name);
            C->RU[r]->ApplyOnReq = C->RU[r]->ApplyToNew = TRUE;
            C->RU[r]->Field[0]      = GetMUICycle(grp->CY_MODE);
            C->RU[r]->SubField[0]   = GetMUIRadio(grp->RA_ADRMODE);
            GetMUIString(C->RU[r]->CustomField[0], grp->ST_FIELD);
            C->RU[r]->Comparison[0] = GetMUICycle(grp->CY_COMP[g]);
            if (grp->ST_MATCH[g]) GetMUIString(C->RU[r]->Match[0], grp->ST_MATCH[g]);
                             else strcpy(C->RU[r]->Match[0], Status[GetMUICycle(grp->CY_STATUS)]);
            if (grp->CH_CASESENS[g]) C->RU[r]->CaseSens[0]  = GetMUICheck(grp->CH_CASESENS[g]);
            if (grp->CH_SUBSTR[g]  ) C->RU[r]->Substring[0] = GetMUICheck(grp->CH_SUBSTR[g]);
            if (ch == 1) CO_SaveConfig(C, G->CO_PrefsFile);
         }
   }
   else ER_NewError(GetStr(MSG_ER_ErrorMaxFilters), NULL, NULL);
}
MakeHook(FI_ToRuleHook,FI_ToRuleFunc);

///
/// FI_Open
//  Opens find window
void SAVEDS FI_Open(void)
{
   int i, j, apos = 0;
   struct Folder **flist, *folder;

   if (!G->FI)
   {
      if (!(G->FI = FI_New())) return;
      folder = FO_GetCurrentFolder();
      flist = FO_CreateList();
      for (j = 0, i = 1; i <= (int)*flist; i++) if (flist[i]->Type != FT_SEPARATOR)
      {
         DoMethod(G->FI->GUI.LV_FOLDERS, MUIM_List_InsertSingle, flist[i]->Name, MUIV_List_Insert_Bottom);
         if (flist[i] == folder) apos = j;
         j++;
      }
      set(G->FI->GUI.LV_FOLDERS, MUIA_List_Active, apos);
      free(flist);
   }
   if (!SafeOpenWindow(G->FI->GUI.WI)) DisposeModulePush(&G->FI);
}
MakeHook(FI_OpenHook,FI_Open);

///
/// FI_SearchGhost
//  Enables/disables gadgets in search form
void FI_SearchGhost(struct SearchGroup *gdata, BOOL disabled)
{
   int mode = GetMUICycle(gdata->CY_MODE), i;
   int oper = GetMUICycle(gdata->CY_COMP[Mode2Group[mode]]);
   set(gdata->CY_MODE, MUIA_Disabled, disabled);
   set(gdata->ST_FIELD, MUIA_Disabled, disabled || mode != 6);
   set(gdata->RA_ADRMODE, MUIA_Disabled, disabled);
   set(gdata->CY_STATUS, MUIA_Disabled, disabled);
   for (i = 0; i < 5; i++)
   {
      set(gdata->CY_COMP[i], MUIA_Disabled, disabled);
      if (gdata->ST_MATCH[i]) set(gdata->ST_MATCH[i], MUIA_Disabled, disabled);
      if (gdata->CH_CASESENS[i]) set(gdata->CH_CASESENS[i], MUIA_Disabled, disabled);
      if (gdata->CH_SUBSTR[i]) set(gdata->CH_SUBSTR[i], MUIA_Disabled, disabled || oper == 4 || (i < 2 && oper > 1));
      if (gdata->BT_FILE[i]) set(gdata->BT_FILE[i], MUIA_Disabled, disabled || oper != 4);
      if (gdata->BT_EDIT[i]) set(gdata->BT_EDIT[i], MUIA_Disabled, disabled || oper != 4);
   }
}

///
/// FI_SearchOptFunc
//  Selects correct form for search mode
void SAVEDS ASM FI_SearchOptFunc(REG(a1,ULONG *arg))
{
   struct SearchGroup *gdata = (struct SearchGroup *)arg[0];
   int mode = GetMUICycle(gdata->CY_MODE);
   set(gdata->PG_SRCHOPT, MUIA_Group_ActivePage, Mode2Group[mode]);
   FI_SearchGhost(gdata, FALSE);
}
MakeHook(FI_SearchOptHook, FI_SearchOptFunc);

///
/// FI_EditFileFunc
//  Edits pattern list in text editor
void SAVEDS ASM FI_EditFileFunc(REG(a1,int *arg))
{
   if (*C->Editor)
   {
      char buffer[SIZE_COMMAND+SIZE_PATHFILE];
      sprintf(buffer,"%s \"%s\"", C->Editor, GetMUIStringPtr((struct Object *)arg[0]));
      ExecuteCommand(buffer, TRUE, OUT_NIL);
   }
}
MakeHook(FI_EditFileHook,FI_EditFileFunc);

///
/// FI_ConstructSearchGroup
//  Creates search form
APTR FI_ConstructSearchGroup(struct SearchGroup *gdata, BOOL remote)
{
   static char *fldopt[2][13], *compopt[14], *statopt[9], *amode[3];
   APTR grp;
   int f = remote ? 1 : 0;

   amode[0] = GetStr(MSG_Address);
   amode[1] = GetStr(MSG_Name);
   amode[2] = NULL;
   statopt[0] = GetStr(MSG_FI_StatNew);
   statopt[1] = GetStr(MSG_FI_StatRead);
   statopt[2] = GetStr(MSG_FI_StatForwarded);
   statopt[3] = GetStr(MSG_FI_StatReplied);
   statopt[4] = GetStr(MSG_FI_StatQueued);
   statopt[5] = GetStr(MSG_FI_StatFailed);
   statopt[6] = GetStr(MSG_FI_StatHold);
   statopt[7] = GetStr(MSG_FI_StatSent);
   statopt[8] = NULL;
   compopt[0] = compopt[5] = compopt[ 8] = " = ";
   compopt[1] = compopt[6] = compopt[ 9] = " <> ";
   compopt[2] =              compopt[10] = " < ";
   compopt[3] =              compopt[11] = " > ";
                             compopt[12] = " IN ";
   compopt[4] = compopt[7] = compopt[13] = NULL;
   fldopt[f][0] = GetStr(MSG_FI_FromField);
   fldopt[f][1] = GetStr(MSG_FI_ToField);
   fldopt[f][2] = GetStr(MSG_FI_CCField);
   fldopt[f][3] = GetStr(MSG_FI_ReplyToField);
   fldopt[f][4] = GetStr(MSG_FI_SubjectField);
   fldopt[f][5] = GetStr(MSG_FI_DateField);
   fldopt[f][6] = GetStr(MSG_FI_OtherField);
   fldopt[f][7] = GetStr(MSG_FI_MessageSize);
   fldopt[f][8] = GetStr(MSG_FI_MessageHeader);
   fldopt[f][9] = remote ? NULL : GetStr(MSG_FI_MessageBody);
   fldopt[f][10]= GetStr(MSG_FI_WholeMessage);
   fldopt[f][11]= GetStr(MSG_Status);
   fldopt[f][12]= NULL;
   grp = VGroup,
      MUIA_HelpNode, "FI_K",
      Child, HGroup,
         Child, Label1(GetStr(MSG_FI_SearchIn)),
            Child, gdata->CY_MODE = MakeCycle(fldopt[f],GetStr(MSG_FI_SearchIn)),
            Child, gdata->ST_FIELD = MakeString(SIZE_DEFAULT, ""),
         End,
         Child, gdata->PG_SRCHOPT = PageGroup,
            Child, VGroup, /* 0  from, to, cc, reply-to */
               Child, HGroup,
                  MUIA_Group_HorizSpacing, 0,
                  Child, gdata->CY_COMP[0] = MakeCycle(&compopt[8],""),
                  Child, HSpace(4),
                  Child, PopaslObject,
                     MUIA_Popasl_Type     ,ASL_FileRequest,
                     MUIA_Popstring_String,gdata->ST_MATCH[0] = MakeString(SIZE_PATTERN,""),
                     MUIA_Popstring_Button,gdata->BT_FILE[0] = PopButton(MUII_PopFile),
                  End,
                  Child, gdata->BT_EDIT[0] = PopButton(MUII_PopUp),
               End,
               Child, HGroup,
                  Child, VGroup,
                     Child, MakeCheckGroup((Object **)&gdata->CH_CASESENS[0], GetStr(MSG_FI_CaseSensitive)),
                     Child, MakeCheckGroup((Object **)&gdata->CH_SUBSTR[0], GetStr(MSG_FI_SubString)),
                  End,
                  Child, gdata->RA_ADRMODE = Radio(NULL, amode),
               End,
            End,
            Child, VGroup, /* 1  subject, other field */
               Child, HGroup,
                  MUIA_Group_HorizSpacing, 0,
                  Child, gdata->CY_COMP[1] = MakeCycle(&compopt[8],""),
                  Child, HSpace(4),
                  Child, PopaslObject,
                     MUIA_Popasl_Type     ,ASL_FileRequest,
                     MUIA_Popstring_String,gdata->ST_MATCH[1] = MakeString(SIZE_PATTERN,""),
                     MUIA_Popstring_Button,gdata->BT_FILE[1] = PopButton(MUII_PopFile),
                  End,
                  Child, gdata->BT_EDIT[1] = PopButton(MUII_PopUp),
               End,
               Child, VGroup,
                  Child, MakeCheckGroup((Object **)&gdata->CH_CASESENS[1], GetStr(MSG_FI_CaseSensitive)),
                  Child, MakeCheckGroup((Object **)&gdata->CH_SUBSTR[1], GetStr(MSG_FI_SubString)),
               End,
            End,
            Child, VGroup, /* 2  date, size */
               Child, HGroup,
                  Child, gdata->CY_COMP[2] = MakeCycle(compopt,""),
                  Child, gdata->ST_MATCH[2] = MakeString(SIZE_PATTERN,""),
               End,
               Child, HVSpace,
            End,
            Child, VGroup, /* 3  status */
               Child, HGroup,
                  Child, gdata->CY_COMP[3] = MakeCycle(&compopt[5],""),
                  Child, gdata->CY_STATUS = MakeCycle(statopt,""),
                  Child, HSpace(0),
               End,
               Child, HVSpace,
            End,
            Child, VGroup, /* 4  message header/body */
               Child, HGroup,
                  Child, gdata->CY_COMP[4] = MakeCycle(&compopt[5],""),
                  Child, gdata->ST_MATCH[4] = MakeString(SIZE_PATTERN,""),
               End,
               Child, MakeCheckGroup((Object **)&gdata->CH_CASESENS[4], GetStr(MSG_FI_CaseSensitive)),
               Child, HVSpace,
            End,
         End,
      End;
   if (grp)
   {
      int i;
      set(gdata->RA_ADRMODE, MUIA_CycleChain, 1);
      SetHelp(gdata->CY_MODE,   MSG_HELP_FI_CY_MODE);
      SetHelp(gdata->ST_FIELD,  MSG_HELP_FI_ST_FIELD);
      SetHelp(gdata->RA_ADRMODE,MSG_HELP_FI_RA_ADRMODE);
      SetHelp(gdata->CY_STATUS, MSG_HELP_FI_CY_STATUS);
      for (i = 0; i < 5; i++)
      {
         if (gdata->ST_MATCH[i])    SetHelp(gdata->ST_MATCH[i], MSG_HELP_FI_ST_MATCH);
         if (gdata->CH_CASESENS[i]) SetHelp(gdata->CH_CASESENS[i], MSG_HELP_FI_CH_CASESENS);
         if (gdata->CH_SUBSTR[i])   SetHelp(gdata->CH_SUBSTR[i], MSG_HELP_FI_CH_SUBSTR);
         if (gdata->BT_EDIT[i])     DoMethod(gdata->BT_EDIT[i], MUIM_Notify, MUIA_Pressed, FALSE, MUIV_Notify_Application, 3, MUIM_CallHook, &FI_EditFileHook, gdata->ST_MATCH[i]);
         SetHelp(gdata->CY_COMP[i], MSG_HELP_FI_CY_COMP);
         set(gdata->CY_COMP[i], MUIA_HorizWeight, 0);
         DoMethod(gdata->CY_COMP[i], MUIM_Notify, MUIA_Cycle_Active, MUIV_EveryTime, MUIV_Notify_Application, 3, MUIM_CallHook, &FI_SearchOptHook, gdata);
      }
      FI_SearchGhost(gdata, FALSE);
      DoMethod(gdata->CY_MODE, MUIM_Notify, MUIA_Cycle_Active, MUIV_EveryTime, MUIV_Notify_Application, 3, MUIM_CallHook, &FI_SearchOptHook, gdata);
   }
   return grp;
}

///
/// FI_SwitchFunc
//  Sets active folder according to the selected message in the results window
void SAVEDS FI_SwitchFunc(void)
{
   struct Mail *mail;
   struct MailInfo *mi;
   DoMethod(G->FI->GUI.LV_MAILS, MUIM_NList_GetEntry, MUIV_NList_GetEntry_Active, &mail);
   if (mail)
   {
      MA_ChangeFolder(mail->Folder);
      mi = GetMailInfo(mail);
      set(G->MA->GUI.NL_MAILS, MUIA_NList_Active, mi->Pos);
   }
}
MakeHook(FI_SwitchHook, FI_SwitchFunc);

///
/// FI_ReadFunc
//  Reads a message listed in the results window
void SAVEDS FI_ReadFunc(void)
{
   struct Mail *mail;
   int winnum;
   DoMethod(G->FI->GUI.LV_MAILS, MUIM_NList_GetEntry, MUIV_NList_GetEntry_Active, &mail);
   if (mail) if ((winnum = RE_Open(-1, TRUE)) != -1)
   {
      if (SafeOpenWindow(G->RE[winnum]->GUI.WI)) RE_ReadMessage(winnum, mail); else DisposeModulePush(G->RE[winnum]);
   }
}
MakeHook(FI_ReadHook, FI_ReadFunc);

///
/// FI_SelectFunc
//  Selects matching messages in the main message list
void SAVEDS FI_SelectFunc(void)
{
   int i;
   struct Folder *folder = FO_GetCurrentFolder();

   DoMethod(G->MA->GUI.NL_MAILS, MUIM_NList_Select, MUIV_NList_Select_All, MUIV_NList_Select_Off, NULL);
   for (i = 0; ; i++)
   {
      struct Mail *mail;
      DoMethod(G->FI->GUI.LV_MAILS, MUIM_NList_GetEntry, i, &mail);
      if (!mail) break;
      if (mail->Folder == folder)
      {
         struct MailInfo *mi = GetMailInfo(mail);
         DoMethod(G->MA->GUI.NL_MAILS, MUIM_NList_Select, mi->Pos, MUIV_NList_Select_On, NULL);
      }
   }
}
MakeHook(FI_SelectHook, FI_SelectFunc);

///
/// FI_Close
//  Closes find window
void SAVEDS FI_Close(void)
{
   DisposeModulePush(&G->FI);
}
MakeHook(FI_CloseHook, FI_Close);
///

/*** GUI ***/
/// FI_PO_InitRuleListFunc
//  Creates a popup list of configured filters
long SAVEDS ASM FI_PO_InitRuleListFunc(REG(a2,Object *pop))
{  
   int i;
   DoMethod(pop, MUIM_List_Clear);
   for (i = 0; i < MAXRU; i++) if (C->RU[i])
      DoMethod(pop, MUIM_List_InsertSingle, C->RU[i], MUIV_List_Insert_Bottom);
   return TRUE;
}
MakeHook(FI_PO_InitRuleListHook, FI_PO_InitRuleListFunc);

///
/// FI_PO_FromRuleFunc
//  Gets search options from selected filter
void SAVEDS ASM FI_PO_FromRuleFunc(REG(a2,Object *pop))
{
   struct Rule *rule;
   DoMethod(pop, MUIM_List_GetEntry, MUIV_List_GetEntry_Active, &rule);
   if (rule) 
   {
      struct SearchGroup *grp = &(G->FI->GUI.GR_SEARCH);
      int i, g = Mode2Group[rule->Field[0]];
      setcycle (grp->CY_MODE,   rule->Field[0]);
      setmutex (grp->RA_ADRMODE,rule->SubField[0]);
      setstring(grp->ST_FIELD,  rule->CustomField[0]);
      setcycle (grp->CY_COMP[g],rule->Comparison[0]);
      if (grp->ST_MATCH[g]) setstring(grp->ST_MATCH[g], rule->Match[0]);
      else for (i = 0; i < 8; i++)
         if (!stricmp(rule->Match[0], Status[i])) setcycle(grp->CY_STATUS, i);
      if (grp->CH_CASESENS[g]) setcheckmark(grp->CH_CASESENS[g],rule->CaseSens[0]);
      if (grp->CH_SUBSTR[g]  ) setcheckmark(grp->CH_SUBSTR[g],  rule->Substring[0]);
   }
}
MakeHook(FI_PO_FromRuleHook, FI_PO_FromRuleFunc);

///
/// FI_New
//  Creates find window
LOCAL struct FI_ClassData *FI_New(void)
{
   struct FI_ClassData *data;

   if (data = calloc(1,sizeof(struct FI_ClassData)))
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
                  Child, FI_ConstructSearchGroup(&data->GUI.GR_SEARCH, FALSE),
                  Child, ColGroup(2),
                     Child, po_fromrule = PopobjectObject,
                        MUIA_Popstring_Button, MakeButton(GetStr(MSG_FI_UseFilter)),
                        MUIA_Popobject_ObjStrHook, &FI_PO_FromRuleHook,
                        MUIA_Popobject_StrObjHook, &FI_PO_InitRuleListHook,
                        MUIA_Popobject_WindowHook, &PO_WindowHook,
                        MUIA_Popobject_Object, lv_fromrule = ListviewObject,
                           MUIA_Listview_List, ListObject, InputListFrame,
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
                     MUIA_NList_Format        , "COL=7 W=-1 BAR,COL=1 MICW=20 BAR,COL=3 MICW=16 BAR,COL=4 MICW=9 MACW=15 BAR,COL=5 W=-1 MACW=9 P=\33r BAR",
                     MUIA_NList_DoubleClick   , TRUE,
                     MUIA_NList_DisplayHook   , &MA_LV_DspFuncHook,
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
         DoMethod(bt_torule          ,MUIM_Notify,MUIA_Pressed             ,FALSE         ,MUIV_Notify_Application,2,MUIM_CallHook,&FI_ToRuleHook);
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
         return data;
      }
      free(data);
   }
   return NULL;
}
///
