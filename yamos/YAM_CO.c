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

#include <stdlib.h>
#include <string.h>

#include <clib/alib_protos.h>
#include <libraries/asl.h>
#include <libraries/iffparse.h>
#include <libraries/locale.h>
#include <mui/NList_mcc.h>
#include <mui/NListtree_mcc.h>
#include <mui/TextEditor_mcc.h>
#include <proto/dos.h>
#include <proto/intuition.h>
#include <proto/muimaster.h>
#include <proto/pm.h>

#include "extra.h"
#include "YAM.h"
#include "YAM_addressbook.h"
#include "YAM_classes.h"
#include "YAM_config.h"
#include "YAM_configFile.h"
#include "YAM_configGUI.h"
#include "YAM_error.h"
#include "YAM_find.h"
#include "YAM_folderconfig.h"
#include "YAM_global.h"
#include "YAM_locale.h"
#include "YAM_main.h"
#include "YAM_hook.h"
#include "YAM_utilities.h"
#include "classes/Classes.h"

struct Config *C;
struct Config *CE;

struct PageList
{
   int  Offset;
   APTR PageLabel;
};

/* local protos */
static void CO_NewPrefsFile(char*);
static int CO_DetectPGP(struct Config*);
static void CO_CopyConfig(struct Config*, struct Config*);
static APTR CO_BuildPage(struct CO_ClassData*, int);
static struct CO_ClassData *CO_New(void);

/***************************************************************************
 Module: Configuration
***************************************************************************/

/// CO_NewPrefsFile
//  Sets the name of the configuration file
static void CO_NewPrefsFile(char *fname)
{
   static char wtitle[SIZE_SMALL+SIZE_PATHFILE];
   strcpy(G->CO_PrefsFile, fname);
   sprintf(wtitle, "%s (%s)", GetStr(MSG_MA_MConfig), fname);
   set(G->CO->GUI.WI, MUIA_Window_Title, wtitle);
}
///

/**** Rules ****/
/// CO_NewRule
//  Initializes a new rule
struct Rule *CO_NewRule(void)
{
   struct Rule *rule = calloc(1, sizeof(struct Rule));
   if (rule)
   {
      strcpy(rule->Name, GetStr(MSG_NewEntry));
      rule->ApplyToNew = rule->ApplyOnReq = TRUE;
   }
   return rule;
}
///
/// CO_AddRule
//  Adds a new entry to the rule list
HOOKPROTONHNONP(CO_AddRule, void)
{
   int i, m, s;
   for (i = 0; i < MAXRU; i++)
      if (!CE->RU[i])
      {
         CE->RU[i] = CO_NewRule();
         for (s = 0; s < 4; s++) for (m = 0; m < 5; m++)
         {
            struct SearchGroup *sg = &(G->CO->GUI.GR_SEARCH[s]);
            nnset(sg->CY_COMP[m], MUIA_Cycle_Active, 0);
            if (sg->ST_MATCH[m]) nnset(sg->ST_MATCH[m], MUIA_String_Contents, "");
            else nnset(sg->CY_STATUS, MUIA_Cycle_Active, 0);
            if (sg->CH_CASESENS[m]) nnset(sg->CH_CASESENS[m], MUIA_Selected, FALSE);
            if (sg->CH_SUBSTR[m]) nnset(sg->CH_SUBSTR[m], MUIA_Selected, FALSE);
         }
         DoMethod(G->CO->GUI.LV_RULES, MUIM_List_InsertSingle, CE->RU[i], MUIV_List_Insert_Bottom);
         set(G->CO->GUI.LV_RULES, MUIA_List_Active, i);
         break; 
      }
}
MakeHook(CO_AddRuleHook,CO_AddRule);

///
/// CO_DelRule
//  Deletes an entry from the rule list
HOOKPROTONHNONP(CO_DelRule, void)
{
   int i, p;
   get(G->CO->GUI.LV_RULES, MUIA_List_Active, &p);
   if (p != MUIV_List_Active_Off)
   {
      DoMethod(G->CO->GUI.LV_RULES, MUIM_List_Remove, p);
      for (i = p+1; i < MAXRU; i++) CE->RU[i-1] = CE->RU[i];
      CE->RU[i-1] = 0;
   }
}
MakeHook(CO_DelRuleHook,CO_DelRule);

///
/// CO_RuleGhost
//  Enables/disables gadgets in rule form
void CO_RuleGhost(struct CO_GUIData *gui, struct Rule *ru)
{
   BOOL isremote = FALSE, single = FALSE;

   if (ru) { isremote = ru->Remote; single = !ru->Combine; }
   set(gui->ST_RNAME, MUIA_Disabled, !ru);
   set(gui->CH_REMOTE, MUIA_Disabled, !ru);
   set(gui->CH_APPLYNEW, MUIA_Disabled, !ru || isremote);
   set(gui->CH_APPLYREQ, MUIA_Disabled, !ru || isremote);
   set(gui->CH_APPLYSENT, MUIA_Disabled, !ru || isremote);
   set(gui->CY_COMBINE[isremote], MUIA_Disabled, !ru);
   set(gui->CH_ABOUNCE, MUIA_Disabled, !ru || isremote);
   set(gui->CH_AFORWARD, MUIA_Disabled, !ru || isremote);
   set(gui->CH_ARESPONSE, MUIA_Disabled, !ru || isremote);
   set(gui->CH_AEXECUTE, MUIA_Disabled, !ru);
   set(gui->CH_APLAY, MUIA_Disabled, !ru);
   set(gui->CH_AMOVE, MUIA_Disabled, !ru || isremote);
   set(gui->CH_ADELETE, MUIA_Disabled, !ru);
   set(gui->CH_ASKIP, MUIA_Disabled, !ru || !isremote);
   set(gui->ST_ABOUNCE, MUIA_Disabled, !ru);
   set(gui->ST_AFORWARD, MUIA_Disabled, !ru);
   set(gui->ST_ARESPONSE, MUIA_Disabled, !ru);
   set(gui->ST_AEXECUTE, MUIA_Disabled, !ru);
   set(gui->ST_APLAY, MUIA_Disabled, !ru);
   set(gui->TX_MOVETO, MUIA_Disabled, !ru);
   set(gui->BT_RDEL, MUIA_Disabled, !ru);
   FI_SearchGhost(&(gui->GR_SEARCH[2*isremote]), !ru);
   FI_SearchGhost(&(gui->GR_SEARCH[2*isremote+1]), !ru || single);

}

///
/// CO_GetRUEntry
//  Fills form with data from selected list entry
HOOKPROTONHNONP(CO_GetRUEntry, void)
{
   struct Rule *rule = NULL;
   struct CO_GUIData *gui = &G->CO->GUI;

   DoMethod(gui->LV_RULES, MUIM_List_GetEntry, MUIV_List_GetEntry_Active, &rule);
   if (rule)
   {      
      int m, i, k, rm = rule->Remote ? 1 : 0;
      nnset(gui->ST_RNAME,    MUIA_String_Contents, rule->Name);
      nnset(gui->CH_REMOTE,   MUIA_Selected, rm);
      nnset(gui->CH_APPLYNEW, MUIA_Selected, rule->ApplyToNew);
      nnset(gui->CH_APPLYSENT,MUIA_Selected, rule->ApplyToSent);
      nnset(gui->CH_APPLYREQ, MUIA_Selected, rule->ApplyOnReq);
      nnset(gui->CY_COMBINE[rm],MUIA_Cycle_Active, rule->Combine);
      nnset(gui->CH_ABOUNCE,   MUIA_Selected, (rule->Actions&  1)==  1);
      nnset(gui->CH_AFORWARD,  MUIA_Selected, (rule->Actions&  2)==  2);
      nnset(gui->CH_ARESPONSE, MUIA_Selected, (rule->Actions&  4)==  4);
      nnset(gui->CH_AEXECUTE,  MUIA_Selected, (rule->Actions&  8)==  8);
      nnset(gui->CH_APLAY,     MUIA_Selected, (rule->Actions& 16)== 16);
      nnset(gui->CH_AMOVE,     MUIA_Selected, (rule->Actions& 32)== 32);
      nnset(gui->CH_ADELETE,   MUIA_Selected, (rule->Actions& 64)== 64);
      nnset(gui->CH_ASKIP,     MUIA_Selected, (rule->Actions&128)==128);
      nnset(gui->ST_ABOUNCE   ,MUIA_String_Contents, rule->BounceTo);
      nnset(gui->ST_AFORWARD  ,MUIA_String_Contents, rule->ForwardTo);
      nnset(gui->ST_ARESPONSE ,MUIA_String_Contents, rule->ReplyFile);
      nnset(gui->ST_AEXECUTE  ,MUIA_String_Contents, rule->ExecuteCmd);
      nnset(gui->ST_APLAY     ,MUIA_String_Contents, rule->PlaySound);
      nnset(gui->TX_MOVETO    ,MUIA_Text_Contents, rule->MoveTo);
      set(gui->GR_LRGROUP, MUIA_Group_ActivePage, rm);
      for (i = 0; i < 2; i++)
      {
         struct SearchGroup *sg = &(gui->GR_SEARCH[i+2*rm]);
         nnset(sg->CY_MODE       ,MUIA_Cycle_Active, rule->Field[i]);
         nnset(sg->RA_ADRMODE    ,MUIA_Radio_Active, rule->SubField[i]);
         nnset(sg->ST_FIELD      ,MUIA_String_Contents, rule->CustomField[i]);
         nnset(sg->PG_SRCHOPT    ,MUIA_Group_ActivePage, m = Mode2Group[rule->Field[i]]);
         nnset(sg->CY_COMP[m], MUIA_Cycle_Active, rule->Comparison[i]);
         if (sg->ST_MATCH[m]) nnset(sg->ST_MATCH[m], MUIA_String_Contents, rule->Match[i]);
         else for (k = 0; k < 8; k++) if (!stricmp(rule->Match[i], Status[k])) nnset(sg->CY_STATUS, MUIA_Cycle_Active, k);
         if (sg->CH_CASESENS[m]) nnset(sg->CH_CASESENS[m], MUIA_Selected, rule->CaseSens[i]);
         if (sg->CH_SUBSTR[m]) nnset(sg->CH_SUBSTR[m], MUIA_Selected, rule->Substring[i]);
      }
   }
   CO_RuleGhost(gui, rule);
}
MakeHook(CO_GetRUEntryHook,CO_GetRUEntry);

///
/// CO_PutRUEntry
//  Fills form data into selected list entry
HOOKPROTONHNONP(CO_PutRUEntry, void)
{
   struct Rule *rule = NULL;
   struct CO_GUIData *gui = &G->CO->GUI;
   char *tx;

   DoMethod(gui->LV_RULES, MUIM_List_GetEntry, MUIV_List_GetEntry_Active, &rule);
   if (rule)
   {
      int i, m, rm = GetMUICheck(gui->CH_REMOTE);
      
      GetMUIString(rule->Name, gui->ST_RNAME);
      rule->Remote = rm == 1;
      rule->ApplyToNew  = GetMUICheck(gui->CH_APPLYNEW);
      rule->ApplyToSent = GetMUICheck(gui->CH_APPLYSENT);
      rule->ApplyOnReq  = GetMUICheck(gui->CH_APPLYREQ);
      rule->Combine     = GetMUICycle(gui->CY_COMBINE[rm]);
      rule->Actions = 0;
      if (GetMUICheck(gui->CH_ABOUNCE))    rule->Actions |= 1;
      if (GetMUICheck(gui->CH_AFORWARD))   rule->Actions |= 2;
      if (GetMUICheck(gui->CH_ARESPONSE))  rule->Actions |= 4;
      if (GetMUICheck(gui->CH_AEXECUTE))   rule->Actions |= 8;
      if (GetMUICheck(gui->CH_APLAY))      rule->Actions |= 16;
      if (GetMUICheck(gui->CH_AMOVE))      rule->Actions |= 32;
      if (GetMUICheck(gui->CH_ADELETE))    rule->Actions |= 64;
      if (GetMUICheck(gui->CH_ASKIP))      rule->Actions |= 128;
      GetMUIString(rule->BounceTo,   gui->ST_ABOUNCE);
      GetMUIString(rule->ForwardTo , gui->ST_AFORWARD);
      GetMUIString(rule->ReplyFile , gui->ST_ARESPONSE);
      GetMUIString(rule->ExecuteCmd, gui->ST_AEXECUTE);
      GetMUIString(rule->PlaySound,  gui->ST_APLAY);
      get(gui->TX_MOVETO, MUIA_Text_Contents, &tx); strcpy(rule->MoveTo, tx);
      for (i = 0; i < 2; i++)
      {
         struct SearchGroup *sg = &(gui->GR_SEARCH[i+2*rm]);
         get(sg->PG_SRCHOPT, MUIA_Group_ActivePage, &m);
         rule->Field[i]      = GetMUICycle(sg->CY_MODE);
         rule->SubField[i]   = GetMUIRadio(sg->RA_ADRMODE);
         GetMUIString(rule->CustomField[i], sg->ST_FIELD);
         rule->Comparison[i] = GetMUICycle(sg->CY_COMP[m]);
         if (sg->ST_MATCH[m]   ) GetMUIString(rule->Match[i], sg->ST_MATCH[m]);
                           else  strcpy(rule->Match[i], Status[GetMUICycle(sg->CY_STATUS)]);
        if (sg->CH_CASESENS[m]) rule->CaseSens[i]  = GetMUICheck(sg->CH_CASESENS[m]);
        if (sg->CH_SUBSTR[m]  ) rule->Substring[i] = GetMUICheck(sg->CH_SUBSTR[m]);
      }
      CO_RuleGhost(gui, rule);
      DoMethod(gui->LV_RULES, MUIM_List_Redraw, MUIV_List_Redraw_Active);
   }
}
MakeHook(CO_PutRUEntryHook,CO_PutRUEntry);

///
/// CO_RemoteToggleFunc
//  Enables/disables GUI elements for remote filters
HOOKPROTONHNO(CO_RemoteToggleFunc, void, int *arg)
{
   BOOL rm = *arg;
   struct CO_GUIData *gui = &G->CO->GUI;
   struct SearchGroup *src, *dst;
   int i, m;

   set(gui->GR_LRGROUP, MUIA_Group_ActivePage, rm);
   nnset(gui->CY_COMBINE[rm], MUIA_Cycle_Active, GetMUICycle(gui->CY_COMBINE[!rm]));
   for (i = 0; i < 2; i++)
   {
      src = &(gui->GR_SEARCH[i+2*(!rm)]); dst = &(gui->GR_SEARCH[i+2*rm]);
      nnset(dst->CY_MODE, MUIA_Cycle_Active, GetMUICycle(src->CY_MODE));
      nnset(dst->RA_ADRMODE, MUIA_Radio_Active, GetMUIRadio(src->RA_ADRMODE));
      nnset(dst->ST_FIELD, MUIA_String_Contents, (STRPTR)xget(src->ST_FIELD, MUIA_String_Contents));
      for (m = 0; m < 5; m++)
      {
         nnset(dst->CY_COMP[m], MUIA_Cycle_Active, GetMUICycle(src->CY_COMP[m]));
         if (src->ST_MATCH[m]) nnset(dst->ST_MATCH[m], MUIA_String_Contents, (STRPTR)xget(src->ST_MATCH[m], MUIA_String_Contents));
                          else nnset(dst->CY_STATUS, MUIA_Cycle_Active, GetMUICycle(src->CY_STATUS));
         if (src->CH_CASESENS[m]) nnset(dst->CH_CASESENS[m], MUIA_Selected, GetMUICheck(src->CH_CASESENS[m]));
         if (src->CH_SUBSTR[m]  ) nnset(dst->CH_SUBSTR[m]  , MUIA_Selected, GetMUICheck(src->CH_SUBSTR[m]));
      }
   }
   CO_PutRUEntry();
}
MakeHook(CO_RemoteToggleHook,CO_RemoteToggleFunc);
///

/**** POP3 servers ****/
/// CO_NewPOP3
//  Initializes a new POP3 account
struct POP3 *CO_NewPOP3(struct Config *co, BOOL first)
{
   struct POP3 *pop3 = calloc(1, sizeof(struct POP3));
   if (pop3)
   {
      if (first)
      {
         char *p, buffer[SIZE_USERID];

         stccpy(buffer, co->EmailAddress, SIZE_USERID);
         if (p = strchr(buffer, '@')) *p = 0;
         strcpy(pop3->User, buffer);
         strcpy(pop3->Server, co->SMTP_Server);
      }
      pop3->Enabled = TRUE;
      pop3->DeleteOnServer = TRUE;
   }
   return pop3;
}

///
/// CO_AddPOP3
//  Adds a new entry to the POP3 account list
HOOKPROTONHNONP(CO_AddPOP3, void)
{
   int i;
   for (i = 0; i < MAXP3; i++)
      if (!CE->P3[i])
      {
         CE->P3[i] = CO_NewPOP3(CE, i == 0);
         DoMethod(G->CO->GUI.LV_POP3, MUIM_List_InsertSingle, CE->P3[i], MUIV_List_Insert_Bottom, TAG_DONE);
         set(G->CO->GUI.LV_POP3, MUIA_List_Active, i);
         set(G->CO->GUI.WI, MUIA_Window_ActiveObject, G->CO->GUI.ST_POPHOST);
         break; 
      }
}
MakeHook(CO_AddPOP3Hook,CO_AddPOP3);

///
/// CO_DelPOP3
//  Deletes an entry from the POP3 account list
HOOKPROTONHNONP(CO_DelPOP3, void)
{
   int i, p, e;
   struct CO_GUIData *gui = &G->CO->GUI;

   get(gui->LV_POP3, MUIA_List_Active, &p);
   get(gui->LV_POP3, MUIA_List_Entries, &e);
   if (p != MUIV_List_Active_Off && e > 1)
   {
      DoMethod(gui->LV_POP3, MUIM_List_Remove, p);
      for (i = p+1; i < MAXP3; i++) CE->P3[i-1] = CE->P3[i];
      CE->P3[i-1] = NULL;
   }
}
MakeHook(CO_DelPOP3Hook,CO_DelPOP3);

///
/// CO_GetP3Entry
//  Fills form with data from selected list entry
HOOKPROTONHNONP(CO_GetP3Entry, void)
{
   int e;
   struct POP3 *pop3 = NULL;
   struct CO_GUIData *gui = &G->CO->GUI;

   DoMethod(gui->LV_POP3, MUIM_List_GetEntry, MUIV_List_GetEntry_Active, &pop3, TAG_DONE);
   if (pop3)
   {
      nnset(gui->ST_POPHOST,   MUIA_String_Contents, pop3->Server);
      nnset(gui->ST_POPUSERID, MUIA_String_Contents, pop3->User);
      nnset(gui->ST_PASSWD,    MUIA_String_Contents, pop3->Password);
      nnset(gui->CH_POPENABLED,MUIA_Selected, pop3->Enabled);
      nnset(gui->CH_USEAPOP,   MUIA_Selected, pop3->UseAPOP);
      nnset(gui->CH_DELETE,    MUIA_Selected, pop3->DeleteOnServer);
   }
   get(gui->LV_POP3, MUIA_List_Entries, &e);
   set(gui->GR_POP3, MUIA_Disabled, !pop3);
   set(gui->BT_PDEL, MUIA_Disabled, !pop3 || e < 2);
}
MakeHook(CO_GetP3EntryHook,CO_GetP3Entry);

///
/// CO_PutP3Entry
//  Fills form data into selected list entry
HOOKPROTONHNONP(CO_PutP3Entry, void)
{
   struct POP3 *pop3 = NULL;
   struct CO_GUIData *gui = &G->CO->GUI;
   int p;

   get(gui->LV_POP3, MUIA_List_Active, &p);
   if (p != MUIV_List_Active_Off)
   {
      DoMethod(gui->LV_POP3, MUIM_List_GetEntry, p, &pop3);
      GetMUIString(pop3->Server, gui->ST_POPHOST);
      GetMUIString(pop3->User, gui->ST_POPUSERID);
      GetMUIString(pop3->Password, gui->ST_PASSWD);
      pop3->Enabled        = GetMUICheck(gui->CH_POPENABLED);
      pop3->UseAPOP        = GetMUICheck(gui->CH_USEAPOP);
      pop3->DeleteOnServer = GetMUICheck(gui->CH_DELETE);
      sprintf(pop3->Account, "%s@%s", pop3->User, pop3->Server);
      DoMethod(gui->LV_POP3, MUIM_List_Redraw, p);
   }
}
MakeHook(CO_PutP3EntryHook,CO_PutP3Entry);

///
/// CO_GetDefaultPOPFunc
//  Sets values of first POP3 account
HOOKPROTONHNONP(CO_GetDefaultPOPFunc, void)
{
   struct POP3 *pop3 = CE->P3[0];

   if (!pop3) return;
   GetMUIString(pop3->Server, G->CO->GUI.ST_POPHOST0);
   GetMUIString(pop3->Password, G->CO->GUI.ST_PASSWD0);
   sprintf(pop3->Account, "%s@%s", pop3->User, pop3->Server);
}
MakeHook(CO_GetDefaultPOPHook,CO_GetDefaultPOPFunc);
///

/**** Mime Viewers ****/
/// CO_NewMimeView
//  Initializes a new MIME viewer
struct MimeView *CO_NewMimeView(void)
{
   struct MimeView *mv = calloc(1, sizeof(struct MimeView));
   if (mv)
   {
      stccpy(mv->ContentType, "?/?", SIZE_CTYPE);
   }
   return mv;
}

///
/// CO_AddMimeView
//  Adds a new entry to the MIME viewer list
HOOKPROTONHNONP(CO_AddMimeView, void)
{
   struct CO_GUIData *gui = &G->CO->GUI;
   int i;
   for (i = 0; i < MAXMV; i++)
      if (!CE->MV[i])
      {
         CE->MV[i] = CO_NewMimeView();
         DoMethod(gui->LV_MIME, MUIM_List_InsertSingle, CE->MV[i], MUIV_List_Insert_Bottom);
         set(gui->LV_MIME, MUIA_List_Active, MUIV_List_Active_Bottom);
         set(gui->WI, MUIA_Window_ActiveObject, gui->ST_CTYPE);
         break; 
      }
}
MakeHook(CO_AddMimeViewHook,CO_AddMimeView);

///
/// CO_DelMimeView
//  Deletes an entry from the MIME viewer list
HOOKPROTONHNONP(CO_DelMimeView, void)
{
   int i, p;
   get(G->CO->GUI.LV_MIME, MUIA_List_Active, &p);
   if (p != MUIV_List_Active_Off)
   {
      DoMethod(G->CO->GUI.LV_MIME, MUIM_List_Remove, p);
      for (i = p+1; i < MAXMV-1; i++) CE->MV[i] = CE->MV[i+1];
      CE->MV[i] = 0;
   }
}
MakeHook(CO_DelMimeViewHook,CO_DelMimeView);

///
/// CO_GetMVEntry
//  Fills form with data from selected list entry
HOOKPROTONHNONP(CO_GetMVEntry, void)
{
   struct MimeView *mv = NULL;
   struct CO_GUIData *gui = &G->CO->GUI;
   int act = 0;

   DoMethod(gui->LV_MIME, MUIM_List_GetEntry, MUIV_List_GetEntry_Active, &mv);
   if (mv)
   {
      nnset(gui->ST_CTYPE, MUIA_String_Contents, mv->ContentType);
      nnset(gui->ST_EXTENS, MUIA_String_Contents, mv->Extension);
      nnset(gui->ST_COMMAND, MUIA_String_Contents, mv->Command);
      get(gui->LV_MIME, MUIA_List_Active, &act);
   }
   set(gui->GR_MIME, MUIA_Disabled, !mv);
   set(gui->BT_MDEL, MUIA_Disabled, !mv);
}
MakeHook(CO_GetMVEntryHook,CO_GetMVEntry);

///
/// CO_PutMVEntry
//  Fills form data into selected list entry
HOOKPROTONHNONP(CO_PutMVEntry, void)
{
   struct MimeView *mv = NULL;
   struct CO_GUIData *gui = &G->CO->GUI;

   DoMethod(gui->LV_MIME, MUIM_List_GetEntry, MUIV_List_GetEntry_Active, &mv);
   if (mv)
   {
      GetMUIString(mv->ContentType, gui->ST_CTYPE);
      GetMUIString(mv->Extension, gui->ST_EXTENS);
      GetMUIString(mv->Command, gui->ST_COMMAND);
      DoMethod(gui->LV_MIME, MUIM_List_Redraw, MUIV_List_Redraw_Active);
   }
}
MakeHook(CO_PutMVEntryHook,CO_PutMVEntry);
///

/**** ARexx Hooks ****/
/// CO_GetRXEntry
//  Fills form with data from selected list entry
HOOKPROTONHNONP(CO_GetRXEntry, void)
{
   int act;
   struct CO_GUIData *gui = &G->CO->GUI;
   struct RxHook *rh;
   get(gui->LV_REXX, MUIA_List_Active, &act);
   rh = &(CE->RX[act]);
   nnset(gui->ST_RXNAME, MUIA_String_Contents, act < 10 ? rh->Name : "");
   nnset(gui->ST_SCRIPT, MUIA_String_Contents, rh->Script);
   nnset(gui->CY_ISADOS, MUIA_Cycle_Active, rh->IsAmigaDOS ? 1 : 0);
   nnset(gui->CH_CONSOLE, MUIA_Selected, rh->UseConsole);
   nnset(gui->CH_WAITTERM, MUIA_Selected, rh->WaitTerm);
   set(gui->ST_RXNAME, MUIA_Disabled, act >= 10);
   DoMethod(gui->LV_REXX, MUIM_List_Redraw, act);
}
MakeHook(CO_GetRXEntryHook,CO_GetRXEntry);

///
/// CO_PutRXEntry
//  Fills form data into selected list entry
HOOKPROTONHNONP(CO_PutRXEntry, void)
{
   int act;
   struct CO_GUIData *gui = &G->CO->GUI;
   get(gui->LV_REXX, MUIA_List_Active, &act);
   if (act != MUIV_List_Active_Off)
   {
      struct RxHook *rh = &(CE->RX[act]);
      GetMUIString(rh->Name, gui->ST_RXNAME);
      GetMUIString(rh->Script, gui->ST_SCRIPT);
      rh->IsAmigaDOS = GetMUICycle(gui->CY_ISADOS) == 1;
      rh->UseConsole = GetMUICheck(gui->CH_CONSOLE);
      rh->WaitTerm = GetMUICheck(gui->CH_WAITTERM);
   }
}
MakeHook(CO_PutRXEntryHook,CO_PutRXEntry);
///

/// CO_IsValid
//  Verifies if the required settings have been made
BOOL CO_IsValid(void)
{
   if (G->CO_Valid) return TRUE;
   if (G->CO) set(G->CO->GUI.WI,MUIA_Window_Open,TRUE);
   else DoMethod(G->App, MUIM_CallHook, &CO_OpenHook);
   MUI_Request(G->App, G->MA->GUI.WI, 0, NULL, GetStr(MSG_OkayReq), GetStr(MSG_CO_InvalidConf));
   return FALSE;
}

///
/// CO_DetectPGP
//  Checks if PGP 2 or 5 is available
static int CO_DetectPGP(struct Config *co)
{
   if (PFExists(co->PGPCmdPath, "pgpe")) return 5;
   else if (PFExists(co->PGPCmdPath, "pgp")) return 2;
   return 0;
}

///
/// CO_FreeConfig
//  Deallocates a configuration structure
void CO_FreeConfig(struct Config *co)
{
   int i;
   for (i = 0; i < MAXP3; i++) if (co->P3[i]) free(co->P3[i]);
   for (i = 0; i < MAXRU; i++) if (co->RU[i]) free(co->RU[i]);
   for (i = 0; i < MAXMV; i++) if (co->MV[i]) free(co->MV[i]);
   memset(co, 0, sizeof(struct Config));
}  

///
/// CO_SetDefaults
//  Sets configuration (or a part of it) to the factory settings
void CO_SetDefaults(struct Config *co, int page)
{
   int i;
   
   if (page == 0 || page < 0)
   {
      *co->RealName = *co->EmailAddress = 0;
      co->TimeZone = G->Locale ? -G->Locale->loc_GMTOffset/60 : 0;
      co->DaylightSaving = 0;
   }
   if (page == 1 || page < 0)
   {
      for (i = 0; i < MAXP3; i++) { if (co->P3[i]) free(co->P3[i]); co->P3[i] = NULL; }
      *co->SMTP_Server = *co->SMTP_Domain = 0;
      co->Allow8bit = FALSE;
      co->Use_SMTP_AUTH = FALSE;
      *co->SMTP_AUTH_User = *co->SMTP_AUTH_Pass = 0;
      co->P3[0] = CO_NewPOP3(co, TRUE); co->P3[0]->DeleteOnServer = TRUE;
   }
   if (page == 2 || page < 0)
   {
      co->AvoidDuplicates = FALSE;
      co->TransferWindow = 2;
      co->UpdateStatus = co->DownloadLarge = TRUE;
      co->PreSelection = 1;
      co->WarnSize = 100;
      co->CheckMailDelay = 0;
      co->NotifyType = 1;
      *co->NotifySound = *co->NotifyCommand = 0;
   }
   if (page == 3 || page < 0)
   {
      for (i = 0; i < MAXRU; i++) { if (co->RU[i]) free(co->RU[i]); co->RU[i] = NULL; }
   }
   if (page == 4 || page < 0)
   {
      for (i = 0; i < MAXMV; i++) { if (co->MV[i]) free(co->MV[i]); co->MV[i] = NULL; }
      co->ShowHeader = 1;
      strcpy(co->ShortHeaders,"(From|To|Date|Subject)");
      co->ShowSenderInfo = 2;
      strcpy(co->ColoredText.buf, "m6");
      strcpy(co->Color2ndLevel.buf, "m7");
      co->DisplayAllTexts = co->FixedFontEdit = co->UseTextstyles = TRUE;
      co->WrapHeader = co->MultipleWindows = FALSE;
      co->SigSepLine = 2;
      *co->TranslationIn = 0;
   }
   if (page == 5 || page < 0)
   {
      *co->ReplyTo = *co->Organization = *co->ExtraHeaders = *co->TranslationOut = 0;
      strcpy(co->NewIntro, GetStr(MSG_CO_NewIntroDef));
      strcpy(co->Greetings, GetStr(MSG_CO_GreetingsDef));
      co->WarnSubject = TRUE;
      co->EdWrapCol = 76;
      co->EdWrapMode = 2;
      strcpy(co->Editor, "C:Ed");
      co->LaunchAlways = FALSE;
      co->EmailCache = 10;
   }
   if (page == 6 || page < 0)
   {
      strcpy(co->ReplyHello, "Hello %f\\n");
      strcpy(co->ReplyIntro, "On %d, you wrote:\\n");
      strcpy(co->ReplyBye, "Regards");
      strcpy(co->AltReplyHello, GetStr(MSG_CO_AltRepHelloDef));
      strcpy(co->AltReplyIntro, GetStr(MSG_CO_AltRepIntroDef));
      strcpy(co->AltReplyBye, GetStr(MSG_CO_AltRepByeDef));
      strcpy(co->AltReplyPattern, GetStr(MSG_CO_AltRepPatternDef));
      strcpy(co->MLReplyHello, GetStr(MSG_CO_MLRepHelloDef));
      strcpy(co->MLReplyIntro, GetStr(MSG_CO_MLRepIntroDef));
      strcpy(co->MLReplyBye, GetStr(MSG_CO_MLRepByeDef));
      strcpy(co->ForwardIntro, GetStr(MSG_CO_ForwardIntroDef));
      strcpy(co->ForwardFinish, GetStr(MSG_CO_ForwardFinishDef));
      co->QuoteMessage = co->QuoteEmptyLines = co->CompareAddress = co->StripSignature = TRUE;
      strcpy(co->QuoteText, ">");
      strcpy(co->AltQuoteText, "|");
   }
   if (page == 7 || page < 0)
   {
      co->UseSignature = FALSE;
      strmfp(co->TagsFile, G->ProgDir, ".taglines");
      strcpy(co->TagsSeparator, "%%");
   }
   if (page == 8 || page < 0)
   {
      co->FolderCols = 1+2+16;
      co->MessageCols = 1+2+8+16;      
      co->FixedFontList = C->SwatchBeat = FALSE;
      co->FolderCntMenu = TRUE;
      co->MessageCntMenu = TRUE;
      co->InfoBar = IB_POS_CENTER;
      strcpy(co->InfoBarText, "- Total: %t New: %n Unread: %u");
   }
   if (page == 9 || page < 0)
   {
      G->PGPVersion = 0;
      if (GetVar("PGPPATH", co->PGPCmdPath, SIZE_PATH, 0) >= 0) G->PGPVersion = CO_DetectPGP(co);
      if (!G->PGPVersion)
      {
         strcpy(co->PGPCmdPath, "C:");
         G->PGPVersion = CO_DetectPGP(co);
      }
      *co->MyPGPID = 0;
      co->EncryptToSelf = co->LogAllEvents = TRUE;
      strcpy(co->ReMailer, "Remailer <remailer@remailer.xganon.com>");
      strcpy(co->RMCommands, "Anon-To: %s");
      strcpy(co->LogfilePath, G->ProgDir);
      co->LogfileMode = 1;
      co->SplitLogfile = FALSE;
   }
   if (page == 10 || page < 0)
   {
      co->GetOnStartup = co->SendOnStartup = co->LoadAllFolders = co->SendOnQuit = FALSE;
      co->CleanupOnStartup = co->RemoveOnStartup = FALSE;
      co->UpdateNewMail = co->CheckBirthdates = co->CleanupOnQuit = co->RemoveOnQuit = TRUE;
   }
   if (page == 11 || page < 0)
   {
      co->MV[0] = CO_NewMimeView();
      strcpy(co->MV[0]->ContentType, GetStr(MSG_Default));
      strcpy(co->MV[0]->Command, "SYS:Utilities/Multiview \"%s\"");
      co->IdentifyBin = TRUE;
      strcpy(co->DetachDir, "RAM:");
      strcpy(co->AttachDir, "RAM:");
   }
   if (page == 12 || page < 0)
   {
      strcpy(co->GalleryDir, "YAM:Gallery");
      strcpy(co->NewAddrGroup, "NEW");
      co->AddMyInfo = FALSE;
      co->AddToAddrbook = 0;
      co->AddrbookCols = 1+2+4;
   }
   if (page == 13 || page < 0)
   {
      for (i = 0; i < MAXRX; i++)
      {
         *co->RX[i].Name = *co->RX[i].Script = 0;
         co->RX[i].IsAmigaDOS = co->RX[i].UseConsole = FALSE;
         co->RX[i].WaitTerm = TRUE;
      }
   }
   if (page == 14 || page < 0)
   {
      strcpy(co->TempDir, "T:");
      strcpy(co->PackerCommand, "LhA -a -m -i%l a \"%a\"");
      co->IconPositionX = co->IconPositionY = 0;
      strcpy(co->AppIconText, "New: %n Total: %t");
      co->IconifyOnQuit = co->RemoveAtOnce = FALSE;
      co->Confirm = co->SaveSent = co->SendMDNAtOnce = TRUE;
      co->ConfirmDelete = 2;
      co->MDN_Display = co->MDN_Process = co->MDN_Delete = 2;
      co->MDN_Filter = 3;
      strcpy(co->XPKPack, "HUFF");
      strcpy(co->XPKPackEncrypt, "HUFF");
      co->XPKPackEff = 50;
      co->XPKPackEncryptEff = 50;
   }
   if (page < 0)
   {
      co->LetterPart = 1;
      co->WriteIndexes = 120;
      co->AutoSave = 600;
      strcpy(co->SupportSite, "http://www.yam.ch/");
      co->JumpToNewMsg = co->AskJumpUnread = co->PrinterCheck = co->IsOnlineCheck = TRUE;
      co->ConfirmOnQuit = FALSE;
      co->HideGUIElements = 0;
      strcpy(co->LocalCharset, "iso-8859-1");
      co->PrintMethod = PRINTMETHOD_DUMPRAW;
      co->StackSize = 40000;
   }
}

///
/// CO_CopyConfig
//  Copies a configuration structure
static void CO_CopyConfig(struct Config *dco, struct Config *sco)
{
   int i;
   memcpy(dco, sco, sizeof(struct Config));
   for (i = 0; i < MAXP3; i++) dco->P3[i] = sco->P3[i] ? (struct POP3     *)AllocCopy(sco->P3[i], sizeof(struct POP3)) : NULL;
   for (i = 0; i < MAXRU; i++) dco->RU[i] = sco->RU[i] ? (struct Rule     *)AllocCopy(sco->RU[i], sizeof(struct Rule)) : NULL;
   for (i = 0; i < MAXMV; i++) dco->MV[i] = sco->MV[i] ? (struct MimeView *)AllocCopy(sco->MV[i], sizeof(struct MimeView)) : NULL;
}

///
/// CO_Validate
//  Validates a configuration, update GUI etc.
void CO_Validate(struct Config *co, BOOL update)
{
   char *p, buffer[SIZE_USERID];
   int i;
   if (!*co->SMTP_Server) strcpy(co->SMTP_Server, co->P3[0]->Server);
   if (!*co->SMTP_Domain) { p = strchr(co->EmailAddress, '@'); strcpy(co->SMTP_Domain, p ? p+1 : ""); }
   for (i = 0; i < MAXP3; i++) if (co->P3[i])
   {
      if (!*co->P3[i]->Server) strcpy(co->P3[i]->Server, co->SMTP_Server);
      if (!*co->P3[i]->User)
      {
         stccpy(buffer, co->EmailAddress, SIZE_USERID);
         if (p = strchr(buffer, '@')) *p = 0;
         strcpy(co->P3[i]->User, buffer);
      }
      sprintf(co->P3[i]->Account, "%s@%s", co->P3[i]->User, co->P3[i]->Server);
   }
   if (G->CO_DST) co->DaylightSaving = G->CO_DST==2;
   G->PGPVersion = CO_DetectPGP(co);
   CreateDirectory(co->TempDir);
   strmfp(G->WR_Filename[0], co->TempDir, "NewLetter.yam");
   strmfp(G->WR_Filename[1], co->TempDir, "NewLetter.1.yam");
   strmfp(G->WR_Filename[2], co->TempDir, "NewLetter.2.yam");
   LoadTranslationTable(&(G->TTin), co->TranslationIn);
   LoadTranslationTable(&(G->TTout), co->TranslationOut);
   G->CO_Valid = (*co->SMTP_Server && *co->EmailAddress && *co->RealName);
   if (update && G->CO)
   {
      switch (G->CO->VisiblePage)
      {
         case 0:
            setstring(G->CO->GUI.ST_POPHOST0, co->P3[0]->Server);
            break;
         case 1:
            setstring(G->CO->GUI.ST_SMTPHOST, co->SMTP_Server);
            setstring(G->CO->GUI.ST_DOMAIN, co->SMTP_Domain);
            setstring(G->CO->GUI.ST_SMTPAUTHUSER, co->SMTP_AUTH_User);
            setstring(G->CO->GUI.ST_SMTPAUTHPASS, co->SMTP_AUTH_Pass);
            DoMethod(G->CO->GUI.LV_POP3, MUIM_List_Redraw, MUIV_List_Redraw_All);
            break;
      }
      if (G->CO->Visited[1] || G->CO->Visited[13] || G->CO->UpdateAll) MA_SetupDynamicMenus();
      if (G->CO->Visited[8] || G->CO->UpdateAll)
      {
         // First we set the NL_MAILS and NL_FOLDER Quiet
         set(G->MA->GUI.NL_MAILS,   MUIA_NList_Quiet,     TRUE);
         set(G->MA->GUI.NL_FOLDERS, MUIA_NListtree_Quiet, TRUE);

         // Modify the ContextMenu flags
         set(G->MA->GUI.NL_MAILS,   MUIA_ContextMenu, (C->MessageCntMenu && PopupMenuBase));
         set(G->MA->GUI.NL_FOLDERS, MUIA_ContextMenu, (C->FolderCntMenu && PopupMenuBase));

         // Now we reorder the Maingroup accordingly to the InfoBar setting
         MA_SortWindow();

         // Now we update the InfoBar because the text could have been changed
         DoMethod(G->MA->GUI.IB_INFOBAR, MUIM_InfoBar_SetFolder, FO_GetCurrentFolder());

         SaveLayout(FALSE);
         MA_MakeFOFormat(G->MA->GUI.NL_FOLDERS);
         MA_MakeMAFormat(G->MA->GUI.NL_MAILS);
         LoadLayout();

         // Now we give the control back to the NLists
         set(G->MA->GUI.NL_MAILS,   MUIA_NList_Quiet,     FALSE);
         set(G->MA->GUI.NL_FOLDERS, MUIA_NListtree_Quiet, FALSE);
      }
      if (G->CO->Visited[12] || G->CO->UpdateAll) AB_MakeABFormat(G->AB->GUI.LV_ADDRESSES);
      if (G->CO->Visited[14] || G->CO->UpdateAll) { SetupAppIcons(); DisplayStatistics((struct Folder *)-1, TRUE); }
   }
}

///
/// CO_ImportCTypes
//  Imports MIME viewers from a MIME.prefs file
HOOKPROTONHNONP(CO_ImportCTypesFunc, void)
{
   int mode;

   if (mode = MUI_Request(G->App, G->CO->GUI.WI, 0, GetStr(MSG_CO_ImportMIME), GetStr(MSG_CO_ImportMIMEGads), GetStr(MSG_CO_ImportMIMEReq)))
      if (ReqFile(ASL_CONFIG,G->CO->GUI.WI, "Import MIME viewers & types", 0, (mode == 1 ? "ENV:" : G->MA_MailDir), (mode == 1 ? "MIME.prefs" : (mode == 2 ? "mailcap" : "mime.types"))))
      {
         char buffer[SIZE_LARGE], fname[SIZE_PATHFILE], *p, *p2;
         struct MimeView *mv;
         APTR lv = G->CO->GUI.LV_MIME;
         int i;
         FILE *fh;
         strmfp(fname, G->ASLReq[ASL_CONFIG]->fr_Drawer, G->ASLReq[ASL_CONFIG]->fr_File);
         if (fh = fopen(fname, "r"))
         {
            set(lv, MUIA_List_Quiet, TRUE);
            while (fgets(buffer, SIZE_LARGE, fh))
            {
               char *ctype = buffer, *ext = "", *command = "";
               if (p = strpbrk(ctype, "\r\n")) *p = 0;
               if (!*ctype || ISpace(*ctype)) continue;
               if (mode == 1)
               {
                  if (*ctype == ';') continue;
                  for (p = ctype; *p && *p != ','; ++p);
                  if (*p)
                  {
                     for (*p = 0, ext = ++p; *p && *p != ','; ++p);
                     if (*p)
                     {
                        for (*p++ = 0; *p && *p != ','; ++p);
                        if (*p)
                        {
                           for (command = ++p; *p && *p != ','; ++p);
                           *p = 0;
                        }
                     }
                  }
               }
               else if (mode == 2)
               {
                  if (*ctype == '#') continue;
                  for (p2 = p = ctype; !ISpace(*p) && *p && *p != ';'; p2 = ++p);
                  if (p = strpbrk(p,";")) ++p;
                  if (p) command = stpblk(p);
                  *p2 = 0;
               }
               else
               {
                  if (*ctype == '#') continue;
                  for (p2 = p = ctype; !ISpace(*p) && *p; p2 = ++p);
                  if (*p) ext = stpblk(p);
                  *p2 = 0;
               }
               for (mv = NULL, i = 0; i < MAXMV; i++) if (CE->MV[i]) if (!stricmp(CE->MV[i]->ContentType, ctype)) { mv = CE->MV[i]; break; }
               if (!mv) for (i = 0; i < MAXMV; i++) if (!CE->MV[i])
               {
                  mv = CE->MV[i] = CO_NewMimeView();
                  DoMethod(lv, MUIM_List_InsertSingle, mv, MUIV_List_Insert_Bottom);
                  break;
               }
               if (mv)
               {
                  for (p = mv->ContentType; *ctype && strlen(mv->ContentType) < SIZE_CTYPE; ctype++)
                     if (*ctype == '*') { *p++ = '#'; *p++ = '?'; } else *p++ = *ctype;
                  *p = 0;
                  if (*command)
                  {
                     for (p = mv->Command; *command && strlen(mv->Command) < SIZE_COMMAND; command++)
                        if (*command == '%' && command[1] == 'f') { *p++ = *command++; *p++ = 's'; } else *p++ = *command;
                     *p = 0;
                  }
                  if (*ext) stccpy(mv->Extension, ext, SIZE_NAME);
               }
            }
            fclose(fh);
            set(lv, MUIA_List_Quiet, FALSE);
            DoMethod(lv, MUIM_List_Redraw, MUIV_List_Redraw_All);
         }
         else ER_NewError(GetStr(MSG_ER_CantOpenFile), fname, NULL);
      }
}
MakeStaticHook(CO_ImportCTypesHook, CO_ImportCTypesFunc);

///
/// CO_EditSignatFunc
//  Edits the signature file
HOOKPROTONHNO(CO_EditSignatFunc, void, int *arg)
{
   int sig = GetMUICycle(G->CO->GUI.CY_SIGNAT), modified;
   char buffer[SIZE_COMMAND+SIZE_PATHFILE];
   APTR ed = G->CO->GUI.TE_SIGEDIT;

   get(ed, MUIA_TextEditor_HasChanged, &modified);
   if (modified) EditorToFile(ed, CreateFilename(SigNames[G->CO->LastSig]), NULL);
   if (*arg) if (*(CE->Editor))
   {
      sprintf(buffer,"%s \"%s\"", CE->Editor, CreateFilename(SigNames[sig]));
      ExecuteCommand(buffer, FALSE, OUT_NIL);
   } else return;
   if (!FileToEditor(CreateFilename(SigNames[sig]), ed)) DoMethod(ed, MUIM_TextEditor_ClearText);
   set(ed, MUIA_TextEditor_HasChanged, FALSE);
   G->CO->LastSig = sig;
}
MakeHook(CO_EditSignatHook,CO_EditSignatFunc);

///
/// CO_OpenConfig
//  Opens a different configuration file
HOOKPROTONHNONP(CO_OpenConfig, void)
{
   if (ReqFile(ASL_CONFIG,G->CO->GUI.WI, GetStr(MSG_CO_Open), 0, G->MA_MailDir, ""))
   {
      char cname[SIZE_PATHFILE];
      strmfp(cname, G->ASLReq[ASL_CONFIG]->fr_Drawer, G->ASLReq[ASL_CONFIG]->fr_File);
      if (CO_LoadConfig(CE, cname, NULL)) CO_NewPrefsFile(cname);
      CO_SetConfig();
      G->CO->UpdateAll = TRUE;
   }
}
MakeStaticHook(CO_OpenConfigHook, CO_OpenConfig);

///
/// CO_SaveConfigAs
//  Saves configuration to a file using an alternative name
HOOKPROTONHNONP(CO_SaveConfigAs, void)
{
   if (ReqFile(ASL_CONFIG,G->CO->GUI.WI, GetStr(MSG_CO_SaveAs), 1, G->MA_MailDir, ""))
   {
      char cname[SIZE_PATHFILE];
      strmfp(cname, G->ASLReq[ASL_CONFIG]->fr_Drawer, G->ASLReq[ASL_CONFIG]->fr_File);
      CO_GetConfig();
      CO_Validate(CE, TRUE);
      CO_NewPrefsFile(cname);
      CO_SaveConfig(CE, cname);
   }
}
MakeStaticHook(CO_SaveConfigAsHook, CO_SaveConfigAs);

///
/// CO_Restore
//  Makes all changes undone
HOOKPROTONHNONP(CO_Restore, void)
{                    
   CO_FreeConfig(CE);
   CO_CopyConfig(CE, C);
   CO_SetConfig();
}
MakeStaticHook(CO_RestoreHook,CO_Restore);

///
/// CO_LastSaved
//  Reloads configuration from file
HOOKPROTONHNONP(CO_LastSaved, void)
{
   CO_LoadConfig(CE, G->CO_PrefsFile, NULL);
   CO_SetConfig();
   G->CO->UpdateAll = TRUE;
}
MakeStaticHook(CO_LastSavedHook,CO_LastSaved);

///
/// CO_ResetToDefaultFunc
//  Resets configuration (or a part of it)
HOOKPROTONHNO(CO_ResetToDefaultFunc, void, int *arg)
{
   if (*arg) { CO_SetDefaults(CE, -1); G->CO->UpdateAll = TRUE; }
   else CO_SetDefaults(CE, G->CO->VisiblePage);
   CO_SetConfig();
}
MakeStaticHook(CO_ResetToDefaultHook,CO_ResetToDefaultFunc);

///
/// CO_BuildPage
//  Creates a GUI section of the configuration
static APTR CO_BuildPage(struct CO_ClassData *data, int page)
{
   switch (page)
   {
      case  0: return CO_Page0(data);
      case  1: return CO_Page1(data);
      case  2: return CO_Page2(data);
      case  3: return CO_Page3(data);
      case  4: return CO_Page4(data);
      case  5: return CO_Page5(data);
      case  6: return CO_Page6(data);
      case  7: return CO_Page7(data);
      case  8: return CO_Page8(data);
      case  9: return CO_Page9(data);
      case 10: return CO_Page10(data);
      case 11: return CO_Page11(data);
      case 12: return CO_Page12(data);
      case 13: return CO_Page13(data);
      case 14: return CO_Page14(data);
   }

   return NULL;
}

///
/// CO_ChangePageFunc
//  Selects a different section of the configuration
HOOKPROTONHNO(CO_ChangePageFunc, void, int *arg)
{
   struct CO_GUIData *gui = &G->CO->GUI;
   if (*arg < 0 || *arg >= MAXCPAGES) return;
   set(gui->WI, MUIA_Window_Sleep, TRUE);
   CO_GetConfig();
   if (DoMethod(gui->GR_PAGE, MUIM_Group_InitChange))
   {
      DoMethod(gui->GR_PAGE, OM_REMMEMBER, gui->GR_SUBPAGE);
      MUI_DisposeObject(gui->GR_SUBPAGE);
      if (gui->GR_SUBPAGE = CO_BuildPage(G->CO, *arg))
      {
         DoMethod(gui->GR_PAGE, OM_ADDMEMBER, gui->GR_SUBPAGE);
         G->CO->VisiblePage = *arg;
         G->CO->Visited[*arg] = TRUE;
      }
      DoMethod(gui->GR_PAGE, MUIM_Group_ExitChange);
      set(gui->MI_IMPMIME, MUIA_Menuitem_Enabled, *arg == 11);
      CO_SetConfig();
   }
   set(gui->WI, MUIA_Window_Sleep, FALSE);
}
MakeStaticHook(CO_ChangePageHook,CO_ChangePageFunc);

///
/// CO_CloseFunc
//  Closes configuration window
HOOKPROTONHNO(CO_CloseFunc, void, int *arg)
{
   if (*arg >= 1)
   {
      CO_GetConfig();
      CO_FreeConfig(C);
      CO_CopyConfig(C, CE);
      CO_Validate(C, TRUE);
      if (*arg == 2) CO_SaveConfig(C, G->CO_PrefsFile);
   }
   CO_FreeConfig(CE);
   free(CE);

   DisposeModulePush(&G->CO);
}
MakeStaticHook(CO_CloseHook,CO_CloseFunc);

///
/// CO_OpenFunc
//  Opens configuration window
HOOKPROTONHNONP(CO_OpenFunc, void)
{
   if (!G->CO)
   {
      if (!(G->CO = CO_New())) return;
      CE = malloc(sizeof(struct Config));
      CO_CopyConfig(CE, C);
      CO_SetConfig();
      CO_NewPrefsFile(G->CO_PrefsFile);
   }
   if (!SafeOpenWindow(G->CO->GUI.WI)) CO_CloseFunc(0);
}
MakeHook(CO_OpenHook,CO_OpenFunc);
///

/*** GUI ***/

/// CO_PL_DspFunc
//  Section listview displayhook
HOOKPROTO(CO_PL_DspFunc, long, char **array, struct PageList *entry)
{
   static char page[SIZE_DEFAULT];
   struct PL_Data *data = (APTR)hook->h_Data;
   sprintf(array[0] = page, "\033O[%08lx] %s", data->Image[entry->Offset], GetStr(entry->PageLabel));
   return 0;
}
MakeHook(CO_PL_DspFuncHook,CO_PL_DspFunc);
///

/// CO_New
//  Creates configuration window
enum { CMEN_OPEN = 1201, CMEN_SAVEAS, CMEN_DEF, CMEN_DEFALL, CMEN_LAST, CMEN_REST, CMEN_MIME };

static struct CO_ClassData *CO_New(void)
{
   struct CO_ClassData *data = calloc(1, sizeof(struct CO_ClassData));
   if (data)
   {
      static struct PageList page[MAXCPAGES], *pages[MAXCPAGES+1];
      APTR lv;
      int i;

      for (i = 0; i < MAXCPAGES; i++) { page[i].Offset = i; pages[i] = &page[i]; }
      pages[i] = NULL;
      page[ 0].PageLabel = MSG_CO_CrdFirstSteps;
      page[ 1].PageLabel = MSG_CO_CrdTCPIP;
      page[ 2].PageLabel = MSG_CO_CrdNewMail;
      page[ 3].PageLabel = MSG_CO_CrdFilters;
      page[ 4].PageLabel = MSG_CO_CrdRead;
      page[ 5].PageLabel = MSG_CO_CrdWrite;
      page[ 6].PageLabel = MSG_CO_CrdReply;
      page[ 7].PageLabel = MSG_CO_CrdSignature;
      page[ 8].PageLabel = MSG_CO_CrdLists;
      page[ 9].PageLabel = MSG_CO_CrdSecurity;
      page[10].PageLabel = MSG_CO_CrdStartQuit;
      page[11].PageLabel = MSG_CO_CrdMIME;
      page[12].PageLabel = MSG_CO_CrdABook;
      page[13].PageLabel = MSG_CO_CrdScripts;
      page[14].PageLabel = MSG_CO_CrdMixed;
      data->GUI.WI = WindowObject,
         MUIA_Window_Title, GetStr(MSG_MA_MConfig),
         MUIA_HelpNode,"CO_W",
         MUIA_Window_Menustrip, MenustripObject,
            MUIA_Family_Child, MenuObject, MUIA_Menu_Title, GetStr(MSG_MA_Project),
               MUIA_Family_Child, MenuitemObject, MUIA_Menuitem_Title,GetStr(MSG_CO_Open), MUIA_Menuitem_Shortcut,"O", MUIA_UserData,CMEN_OPEN, End,
               MUIA_Family_Child, MenuitemObject, MUIA_Menuitem_Title,GetStr(MSG_CO_SaveAs), MUIA_Menuitem_Shortcut,"A", MUIA_UserData,CMEN_SAVEAS, End,
            End,
            MUIA_Family_Child, MenuObject, MUIA_Menu_Title, GetStr(MSG_CO_Edit),
               MUIA_Family_Child, MenuitemObject, MUIA_Menuitem_Title,GetStr(MSG_CO_ResetDefaults), MUIA_Menuitem_Shortcut,"D", MUIA_UserData,CMEN_DEF, End,
               MUIA_Family_Child, MenuitemObject, MUIA_Menuitem_Title,GetStr(MSG_CO_ResetAll), MUIA_Menuitem_Shortcut,"E", MUIA_UserData,CMEN_DEFALL, End,
               MUIA_Family_Child, MenuitemObject, MUIA_Menuitem_Title,GetStr(MSG_CO_LastSaved), MUIA_Menuitem_Shortcut,"L", MUIA_UserData,CMEN_LAST, End,
               MUIA_Family_Child, MenuitemObject, MUIA_Menuitem_Title,GetStr(MSG_CO_Restore), MUIA_Menuitem_Shortcut,"R", MUIA_UserData,CMEN_REST, End,
            End,
            MUIA_Family_Child, MenuObject, MUIA_Menu_Title, GetStr(MSG_CO_Extras),
               MUIA_Family_Child, data->GUI.MI_IMPMIME = MenuitemObject, MUIA_Menuitem_Enabled,FALSE, MUIA_Menuitem_Title,GetStr(MSG_CO_ImportMIME), MUIA_UserData,CMEN_MIME, End,
            End,
         End,
         MUIA_Window_ID, MAKE_ID('C','O','N','F'),
         WindowContents, VGroup,
            Child, HGroup,
               Child, lv = ListviewObject,
                  MUIA_CycleChain,1,
                  MUIA_Listview_List, data->GUI.LV_PAGE = NewObject(CL_PageList->mcc_Class,NULL,
                     InputListFrame,
                     MUIA_List_AdjustWidth, TRUE,
                     MUIA_List_MinLineHeight, 16,
                     MUIA_List_SourceArray, pages,
                     MUIA_List_Active, 0,
                  End,
               End,
               Child, data->GUI.GR_PAGE = VGroup,
                  TextFrame,
                  InnerSpacing(6,6),
                  MUIA_Background, MUII_PageBack,
                  Child, data->GUI.GR_SUBPAGE = CO_BuildPage(data, 0),
               End,
            End,
            Child, HGroup,
               MUIA_Group_SameWidth, TRUE,
               Child, data->GUI.BT_SAVE   = MakeButton(GetStr(MSG_CO_Save)),
               Child, data->GUI.BT_USE    = MakeButton(GetStr(MSG_CO_Use)),
               Child, data->GUI.BT_CANCEL = MakeButton(GetStr(MSG_CO_Cancel)),
            End,
         End,
      End;
      if (data->GUI.WI)
      {
         DoMethod(G->App, OM_ADDMEMBER, data->GUI.WI);
         set(data->GUI.WI, MUIA_Window_DefaultObject, lv);
         SetHelp(data->GUI.BT_SAVE,   MSG_HELP_CO_BT_SAVE);
         SetHelp(data->GUI.BT_USE,    MSG_HELP_CO_BT_USE);
         SetHelp(data->GUI.BT_CANCEL, MSG_HELP_CO_BT_CANCEL);
         DoMethod(data->GUI.WI          ,MUIM_Notify,MUIA_Window_MenuAction  ,CMEN_OPEN     ,MUIV_Notify_Application,2,MUIM_CallHook,&CO_OpenConfigHook);
         DoMethod(data->GUI.WI          ,MUIM_Notify,MUIA_Window_MenuAction  ,CMEN_SAVEAS   ,MUIV_Notify_Application,2,MUIM_CallHook,&CO_SaveConfigAsHook);
         DoMethod(data->GUI.WI          ,MUIM_Notify,MUIA_Window_MenuAction  ,CMEN_DEF      ,MUIV_Notify_Application,3,MUIM_CallHook,&CO_ResetToDefaultHook,FALSE);
         DoMethod(data->GUI.WI          ,MUIM_Notify,MUIA_Window_MenuAction  ,CMEN_DEFALL   ,MUIV_Notify_Application,3,MUIM_CallHook,&CO_ResetToDefaultHook,TRUE);
         DoMethod(data->GUI.WI          ,MUIM_Notify,MUIA_Window_MenuAction  ,CMEN_LAST     ,MUIV_Notify_Application,2,MUIM_CallHook,&CO_LastSavedHook);
         DoMethod(data->GUI.WI          ,MUIM_Notify,MUIA_Window_MenuAction  ,CMEN_REST     ,MUIV_Notify_Application,2,MUIM_CallHook,&CO_RestoreHook);
         DoMethod(data->GUI.WI          ,MUIM_Notify,MUIA_Window_MenuAction  ,CMEN_MIME     ,MUIV_Notify_Application,3,MUIM_CallHook,&CO_ImportCTypesHook,FALSE);
         DoMethod(data->GUI.LV_PAGE     ,MUIM_Notify,MUIA_List_Active        ,MUIV_EveryTime,MUIV_Notify_Application,3,MUIM_CallHook,&CO_ChangePageHook,MUIV_TriggerValue);
         DoMethod(data->GUI.BT_SAVE     ,MUIM_Notify,MUIA_Pressed            ,FALSE         ,MUIV_Notify_Application,3,MUIM_CallHook,&CO_CloseHook,2);
         DoMethod(data->GUI.BT_USE      ,MUIM_Notify,MUIA_Pressed            ,FALSE         ,MUIV_Notify_Application,3,MUIM_CallHook,&CO_CloseHook,1);
         DoMethod(data->GUI.BT_CANCEL   ,MUIM_Notify,MUIA_Pressed            ,FALSE         ,MUIV_Notify_Application,3,MUIM_CallHook,&CO_CloseHook,0);
         DoMethod(data->GUI.WI          ,MUIM_Notify,MUIA_Window_CloseRequest,TRUE          ,MUIV_Notify_Application,3,MUIM_CallHook,&CO_CloseHook,0);
         return data;
      }
      free(data);
   }
   return NULL;
}
////
