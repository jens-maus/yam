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
LOCAL void US_SaveUsers(void);
LOCAL void US_LoadUsers(void);
LOCAL BOOL US_PromptForPassword(struct User*, APTR);
LOCAL BOOL US_SaveUserList(void);
LOCAL struct US_ClassData *US_New(BOOL);

/***************************************************************************
 Module: User list
***************************************************************************/

/// US_GetCurrentUser
//  Gets pointer current user from user database
struct User *US_GetCurrentUser(void)
{
   int i;
   struct User *u = NULL;
   for (i = 0; i < G->Users.Num; i++) if (G->Users.User[i].ID == G->Users.CurrentID) u = &G->Users.User[i];
   return u;
}

///
/// US_SaveUsers
//  Saves user database to .users
LOCAL void US_SaveUsers(void)
{
   FILE *fh;
   char *fname = "PROGDIR:.users";
   int i;

   for (i = 0; i < G->Users.Num; i++) if (!G->Users.User[i].Limited) break;
   if (i == G->Users.Num) G->Users.User[0].Limited = FALSE;
   if (fh = fopen(fname, "w"))
   {
      int i;
      fputs("YUS2 - YAM Users\n", fh);
      for (i = 0; i < G->Users.Num; i++)
      {
         struct User *u = &G->Users.User[i];
         if (u->Name) fprintf(fh, "@USER %s\n%s\n%ld\n%s\n@ENDUSER\n", u->Name, u->MailDir, u->Limited*4+u->UseAddr*2+u->UseDict, Encrypt(u->Password));
      }
      fclose(fh);
      AppendLogVerbose(62, GetStr(MSG_LOG_SavingUsers), "", "", "", "");
   }
   else ER_NewError(GetStr(MSG_ER_CantCreateFile), fname, NULL);
}

///
/// US_LoadUsers
//  Loads user database from .users
LOCAL void US_LoadUsers(void)
{
   BOOL save = FALSE;
   FILE *fh;
   char buffer[SIZE_LARGE];
   clear(&G->Users, sizeof(struct Users));
   G->Users.Num = 0;
   if (fh = fopen("PROGDIR:.users", "r"))
   {
      BOOL hasmanager = FALSE;
      GetLine(fh, buffer, SIZE_LARGE);
      if (!strncmp(buffer,"YUS",3))
      {
         int ver = buffer[3]-'0';
         while (GetLine(fh, buffer, SIZE_LARGE))
         {
            if (!strncmp(buffer, "@USER", 5))
            {
               struct User *u = &G->Users.User[G->Users.Num];
               stccpy(u->Name, Trim(&buffer[6]), SIZE_NAME);
               stccpy(u->MailDir, Trim(GetLine(fh, buffer, SIZE_LARGE)), SIZE_PATH);
               if (!*u->MailDir) { stccpy(u->MailDir, G->MA_MailDir, SIZE_PATH); save = TRUE; }
               if (FileType(u->MailDir) != 2)
               {
                  ER_NewError(GetStr(MSG_ER_UserRemoved), u->MailDir, u->Name);
                  u->Name[0] = 0;
                  save = TRUE;
               }
               else
               {
                  int flags = atoi(Trim(GetLine(fh, buffer, SIZE_LARGE)));
                  u->Limited = (flags&4) == 4;
                  u->UseAddr = (flags&2) == 2;
                  u->UseDict = (flags&1) == 1;
                  if (!u->Limited) hasmanager = TRUE;
                  if (ver >= 2) stccpy(u->Password, Decrypt(GetLine(fh, buffer, SIZE_LARGE)), SIZE_PASSWORD);
                  u->ID = GetSimpleID();
                  G->Users.Num++;
               }
               while (GetLine(fh, buffer, SIZE_LARGE)) if (!strcmp(buffer, "@ENDUSER")) break;
            }
         }
      }
      fclose(fh);
      if (!hasmanager) { G->Users.User[0].Limited = FALSE; save = TRUE; }
   }
   if (!G->Users.Num)
   {
      struct User *u = &G->Users.User[0];
      stccpy(u->MailDir, G->MA_MailDir, SIZE_PATH);
      u->UseAddr = u->UseDict = TRUE;
      u->ID = GetSimpleID();
      G->Users.Num = 1;
      save = TRUE;
   }
   if (save) US_SaveUsers();
}

///
/// US_PromptForPassword
//  User login: asks for user password
LOCAL BOOL US_PromptForPassword(struct User *u, APTR win)
{
   char passwd[SIZE_PASSWORD];

   do {
      *passwd = 0;
      if (!StringRequest(passwd, SIZE_PASSWORD, GetStr(MSG_US_WaitLogin), GetStr(MSG_US_EnterPassword), GetStr(MSG_Okay), NULL, GetStr(MSG_Cancel), TRUE, win)) return FALSE;
   } while (strcmp(passwd, u->Password));
   return TRUE;
}

///
/// US_Login
//  User login: puts up user list and waits for a selection
BOOL US_Login(char *username, char *password, char *maildir, char *prefsfile)
{
   int i, user = -1;
   APTR button, button0, group;
   struct User *u;
   BOOL loggedin = TRUE;

   US_LoadUsers();
   if (username)
   {
      for (i = 0; i < G->Users.Num; i++) if (!stricmp(G->Users.User[i].Name, username)) user = i;
   }
   if (user < 0)
   {
      password = NULL;
      if (G->Users.Num > 1)
         if (DoMethod(G->AY_List, MUIM_Group_InitChange))
         {
            group = ColGroup(2), End;
            for (i = 0; i < G->Users.Num; i++)
            {
               button = MakeButton(G->Users.User[i].Name);
               if (!i) button0 = button;
               DoMethod(group, OM_ADDMEMBER, button);
               DoMethod(button, MUIM_Notify, MUIA_Pressed, FALSE, MUIV_Notify_Application, 2, MUIM_Application_ReturnID, ID_LOGIN+i);
            }
            if (i%2 == 1) DoMethod(group, OM_ADDMEMBER, HSpace(0));
            DoMethod(G->AY_List, OM_ADDMEMBER, group);
            DoMethod(G->AY_List, MUIM_Group_ExitChange);
            set(G->AY_Text, MUIA_ShowMe, TRUE);
            set(G->AY_Text, MUIA_Gauge_InfoText, GetStr(MSG_US_WaitLogin));
            set(G->AY_Group, MUIA_Group_ActivePage, 1);
            set(G->AY_Win, MUIA_Window_ActiveObject, button0);
            set(G->AY_Win, MUIA_Window_Open, TRUE);
            while (user == -1)
            {
               ULONG signals;
               long winopen, iconified;
               get(G->AY_Win, MUIA_Window_Open, &winopen);
               get(G->App, MUIA_Application_Iconified, &iconified);
               if (!winopen && !iconified) return FALSE;
               if ((i = DoMethod(G->App, MUIM_Application_Input, &signals)-ID_LOGIN) >= 0 && i < G->Users.Num) user = i;
               else if (signals) Wait(signals);
            }
            set(G->AY_Group, MUIA_Group_ActivePage, 0);
            DoMethod(G->AY_List, MUIM_Group_InitChange);
            DoMethod(G->AY_List, OM_REMMEMBER, group);
            DoMethod(G->AY_List, MUIM_Group_ExitChange);
         }
         else user = 0;
      else user = 0;
   }
   u = &G->Users.User[user];
   G->Users.CurrentID = u->ID;
   strcpy(G->MA_MailDir, maildir ? maildir : u->MailDir);
   if (prefsfile) strcpy(G->CO_PrefsFile, prefsfile); else strmfp(G->CO_PrefsFile, G->MA_MailDir, ".config");
   strmfp(G->AB_Filename, u->UseAddr ? G->ProgDir : G->MA_MailDir, ".addressbook");
   strmfp(G->DI_Filename, u->UseDict ? G->ProgDir : G->MA_MailDir, ".glossary");
   if (u->Password[0])
      if (password) loggedin = (!strcmp(password, u->Password) || *password == '\01');
      else loggedin = US_PromptForPassword(u, G->AY_Win);
   return loggedin;
}

///
/// US_DelFunc
//  Removes a user from the user database
void SAVEDS US_DelFunc(void)
{
   int i, m;
   struct User *user;
   APTR lv = G->US->GUI.LV_USERS;
   get(lv, MUIA_NList_Active, &i);
   DoMethod(lv, MUIM_NList_GetEntry, i, &user);
   if (*user->MailDir)
   {
      if (!(m = MUI_Request(G->App, G->US->GUI.WI, 0, GetStr(MSG_MA_ConfirmReq), GetStr(MSG_US_RemoveReqGads), GetStr(MSG_US_RemoveReq)))) return;
      if (m == 1) DeleteMailDir(user->MailDir, TRUE);
   }
   DoMethod(lv, MUIM_NList_Remove, i);
}
MakeHook(US_DelHook, US_DelFunc);

///
/// US_AddFunc
//  Adds a new user to the user database
void SAVEDS US_AddFunc(void)
{
   struct US_GUIData *gui = &G->US->GUI;
   struct User user;
   int n;
   get(gui->LV_USERS, MUIA_NList_Entries, &n);
   if (n < MAXUSERS-1)
   {
      clear(&user, sizeof(struct User));
      user.Limited = user.IsNew = TRUE;
      DoMethod(gui->LV_USERS, MUIM_NList_InsertSingle, &user, MUIV_NList_Insert_Bottom);
      set(gui->LV_USERS, MUIA_NList_Active, MUIV_NList_Active_Bottom);
      set(gui->WI, MUIA_Window_ActiveObject, gui->ST_USER);
   }
}
MakeHook(US_AddHook, US_AddFunc);

///
/// US_SaveUserList
//  Initializes configuration files for new users and saves the user database
LOCAL BOOL US_SaveUserList(void)
{
   int i;

   get(G->US->GUI.LV_USERS, MUIA_NList_Entries, &G->Users.Num);
   for (i = 0; i < G->Users.Num; i++)
   {
      struct User *u;
      BOOL valid = FALSE;
      DoMethod(G->US->GUI.LV_USERS, MUIM_NList_GetEntry, i, &u);
      G->Users.User[i] = *u;
      if (*u->Name)
         if (*u->MailDir)
            if (FileType(u->MailDir) != 2)
            {
               if (MUI_Request(G->App, G->US->GUI.WI, 0, GetStr(MSG_MA_MUsers), GetStr(MSG_YesNoReq), GetStr(MSG_US_ErrorNoDirectory)))
                  if (CreateDirectory(u->MailDir)) valid = TRUE;
                  else ER_NewError(GetStr(MSG_ER_CantCreateDir), u->MailDir, NULL);
            }
            else valid = TRUE;
         else ER_NewError(GetStr(MSG_ER_MissingDirectory), NULL, NULL);
      else ER_NewError(GetStr(MSG_ER_MissingName), NULL, NULL);
      if (!valid)
      {
         set(G->US->GUI.LV_USERS, MUIA_NList_Active, i);
         return FALSE;
      }
      if (u->Clone && u->IsNew)
      {
         char dest[SIZE_PATHFILE];
         strmfp(dest, u->MailDir, ".addressbook"); CopyFile(dest, NULL, G->AB_Filename, NULL);
         strmfp(dest, u->MailDir, ".glossary");    CopyFile(dest, NULL, G->DI_Filename, NULL);
         strmfp(dest, u->MailDir, ".config");      CopyFile(dest, NULL, G->CO_PrefsFile, NULL);
      }
   }
   US_SaveUsers();
   return TRUE;
}

///
/// US_OpenFunc
//  Opens and initializes user list window
void SAVEDS US_OpenFunc(void)
{
   if (!G->US)
   {
      int i;
      struct User *u;
      for (i = 0; i < G->Users.Num; i++) if (G->Users.User[i].ID == G->Users.CurrentID) u = &G->Users.User[i];
      if (!*G->Users.User[0].Name) stccpy(G->Users.User[0].Name, C->RealName, SIZE_NAME);
      if (!(G->US = US_New(!u->Limited))) return;
      if (!SafeOpenWindow(G->US->GUI.WI)) { DisposeModulePush(&G->US); return; }
      for (i = 0; i < G->Users.Num; i++) DoMethod(G->US->GUI.LV_USERS, MUIM_NList_InsertSingle, &G->Users.User[i], MUIV_NList_Insert_Bottom);
      set(G->US->GUI.LV_USERS, MUIA_NList_Active, 0);
   }
}
MakeHook(US_OpenHook, US_OpenFunc);

///
/// US_CloseFunc
//  Closes user list window
void SAVEDS US_CloseFunc(void)
{
   if (US_SaveUserList()) DisposeModulePush(&G->US);
}
MakeHook(US_CloseHook, US_CloseFunc);

///
/// US_GetUSEntryFunc
//  Fills form with data from selected list entry
void SAVEDS US_GetUSEntryFunc(void)
{
   struct User *user;
   struct US_GUIData *gui = &G->US->GUI;
   BOOL notallowed, iscurrent, limited = !G->US->Supervisor;
   int act;

   get(gui->LV_USERS, MUIA_NList_Active, &act);
   if (act != MUIV_NList_Active_Off)
   {
      DoMethod(gui->LV_USERS, MUIM_NList_GetEntry, act, &user);
      iscurrent = user->ID == G->Users.CurrentID;
      nnset(gui->ST_USER,    MUIA_String_Contents, user->Name);
      nnset(gui->ST_MAILDIR, MUIA_String_Contents, user->MailDir);
      nnset(gui->ST_PASSWD,  MUIA_String_Contents, user->Password);
      nnset(gui->CH_USEADDR, MUIA_Selected, user->UseAddr);
      nnset(gui->CH_USEDICT, MUIA_Selected, user->UseDict);
      nnset(gui->CH_ROOT,    MUIA_Selected, !user->Limited);
      nnset(gui->CH_CLONE,   MUIA_Selected, user->Clone);
      notallowed = limited && !iscurrent;
      set(gui->ST_USER,    MUIA_Disabled, notallowed);
      set(gui->CH_USEDICT, MUIA_Disabled, notallowed || !act);
      set(gui->CH_USEADDR, MUIA_Disabled, notallowed || !act);
      set(gui->CH_CLONE,   MUIA_Disabled, !user->IsNew || !act);
      set(gui->PO_MAILDIR, MUIA_Disabled, limited || !act);
      set(gui->CH_ROOT,    MUIA_Disabled, limited);
      set(gui->ST_PASSWD,  MUIA_Disabled, notallowed);
      set(gui->BT_DEL,     MUIA_Disabled, !act || iscurrent);
   }
   else DoMethod(G->App, MUIM_MultiSet, MUIA_Disabled, TRUE, gui->ST_USER, gui->ST_PASSWD, gui->PO_MAILDIR, gui->CH_USEDICT, gui->CH_USEADDR, gui->CH_ROOT, gui->CH_CLONE, gui->BT_DEL, NULL);
}
MakeHook(US_GetUSEntryHook,US_GetUSEntryFunc);

///
/// US_PutUSEntryFunc
//  Fills form data into selected list entry
void SAVEDS US_PutUSEntryFunc(void)
{
   struct User *user = NULL;
   struct US_GUIData *gui = &G->US->GUI;

   DoMethod(gui->LV_USERS, MUIM_NList_GetEntry, MUIV_NList_GetEntry_Active, &user);
   if (user)
   {
      GetMUIString(user->Name, gui->ST_USER);
      GetMUIString(user->MailDir, gui->ST_MAILDIR);
      GetMUIString(user->Password, gui->ST_PASSWD);
      user->UseAddr = GetMUICheck(gui->CH_USEADDR);
      user->UseDict = GetMUICheck(gui->CH_USEDICT);
      user->Limited = !GetMUICheck(gui->CH_ROOT);
      user->Clone   = GetMUICheck(gui->CH_CLONE);
      DoMethod(gui->LV_USERS, MUIM_NList_Redraw, MUIV_NList_Redraw_Active);
   }
}
MakeHook(US_PutUSEntryHook,US_PutUSEntryFunc);
///

/*** GUI ***/
/// US_LV_ConFunc
//  User listview construction hook
struct User * SAVEDS ASM US_LV_ConFunc(REG(a1,struct User *user))
{
   struct User *entry = malloc(sizeof(struct User));
   *entry = *user;
   return entry;
}
MakeHook(US_LV_ConHook, US_LV_ConFunc);

///
/// US_LV_DspFunc
//  User listview display hook
long SAVEDS ASM US_LV_DspFunc(REG(a2,char **array), REG(a1,struct User *entry))
{
   if (entry)
   {
      array[0] = entry->Name;
      array[1] = entry->MailDir;
      if (entry->ID == G->Users.CurrentID) array[DISPLAY_ARRAY_MAX] = "\0338";
   }
   else
   {
      array[0] = GetStr(MSG_US_TitleUserName);
      array[1] = GetStr(MSG_US_TitleMailDir);
   }
   return 0;
}
MakeHook(US_LV_DspHook,US_LV_DspFunc);

///
/// US_New
//  Creates user list window
LOCAL struct US_ClassData *US_New(BOOL supervisor)
{
   struct US_ClassData *data;

   if (data = calloc(1, sizeof(struct US_ClassData)))
   {
      data->Supervisor = supervisor;
      data->GUI.WI = WindowObject,
         MUIA_Window_Title, GetStr(MSG_MA_MUsers),
         MUIA_HelpNode, "US_W",
         MUIA_Window_ID, MAKE_ID('U','S','E','R'),
         WindowContents, VGroup,
            Child, data->GUI.LV_USERS = NListviewObject,
               MUIA_CycleChain, 1,
               MUIA_NListview_NList, NListObject,
                  InputListFrame,
                  MUIA_NList_ConstructHook, &US_LV_ConHook,
                  MUIA_NList_DestructHook, &GeneralDesHook,
                  MUIA_NList_DisplayHook, &US_LV_DspHook,
                  MUIA_NList_DisplayHook, &MA_LV_FDspFuncHook,
                  MUIA_NList_TitleSeparator, TRUE,
                  MUIA_NList_Title, TRUE,
                  MUIA_NList_Format, "BAR,",
               End,
            End,
            Child, VGroup, GroupFrameT(GetStr(MSG_MA_Settings)),
               Child, ColGroup(2),
                  Child, Label2(GetStr(MSG_US_UserName)),
                  Child, data->GUI.ST_USER = MakeString(SIZE_NAME, GetStr(MSG_US_UserName)),
                  Child, Label2(GetStr(MSG_US_Password)),
                  Child, data->GUI.ST_PASSWD = MakePassString(GetStr(MSG_US_Password)),
                  Child, Label2(GetStr(MSG_US_MailDirectory)),
                  Child, data->GUI.PO_MAILDIR = PopaslObject,
                     MUIA_Popasl_Type,ASL_FileRequest,
                     MUIA_Popstring_String,data->GUI.ST_MAILDIR = MakeString(SIZE_PATH,GetStr(MSG_US_MailDirectory)),
                     MUIA_Popstring_Button,PopButton(MUII_PopDrawer),
                     ASLFR_DrawersOnly, TRUE,
                  End,
               End,
               Child, VGroup,
                  Child, MakeCheckGroup((Object **)&data->GUI.CH_USEADDR, GetStr(MSG_US_GlobalAddrBook)),
                  Child, MakeCheckGroup((Object **)&data->GUI.CH_USEDICT, GetStr(MSG_US_GlobalDict)),
                  Child, MakeCheckGroup((Object **)&data->GUI.CH_ROOT,GetStr(MSG_US_SuperVisor)),
               End,
               Child, VGroup,
                  MUIA_ShowMe, supervisor,
                  Child, MakeCheckGroup((Object **)&data->GUI.CH_CLONE, GetStr(MSG_US_CopyConfig)),
               End,
            End,
            Child, ColGroup(3),
               MUIA_ShowMe, supervisor,
               Child, data->GUI.BT_ADD = MakeButton(GetStr(MSG_US_AddUser)),
               Child, data->GUI.BT_DEL = MakeButton(GetStr(MSG_US_DelUser)),
            End,
         End,
      End;
      if (data->GUI.WI)
      {
         SetHelp(data->GUI.ST_USER   ,MSG_HELP_US_ST_USER);
         SetHelp(data->GUI.ST_MAILDIR,MSG_HELP_US_ST_MAILDIR);
         SetHelp(data->GUI.ST_PASSWD ,MSG_HELP_US_ST_PASSWD);
         SetHelp(data->GUI.CH_USEADDR,MSG_HELP_US_CH_USEADDR);
         SetHelp(data->GUI.CH_USEDICT,MSG_HELP_US_CH_USEDICT);
         SetHelp(data->GUI.CH_CLONE  ,MSG_HELP_US_CH_CLONE);
         SetHelp(data->GUI.CH_ROOT   ,MSG_HELP_US_CH_ROOT);
         SetHelp(data->GUI.BT_ADD    ,MSG_HELP_US_BT_ADD);
         SetHelp(data->GUI.BT_DEL    ,MSG_HELP_US_BT_DEL);
         DoMethod(G->App, OM_ADDMEMBER, data->GUI.WI);
         DoMethod(data->GUI.LV_USERS,  MUIM_Notify,MUIA_List_Active,        MUIV_EveryTime,MUIV_Notify_Application,2,MUIM_CallHook,&US_GetUSEntryHook);
         DoMethod(data->GUI.ST_USER,   MUIM_Notify,MUIA_String_Contents,    MUIV_EveryTime,MUIV_Notify_Application,2,MUIM_CallHook,&US_PutUSEntryHook);
         DoMethod(data->GUI.ST_MAILDIR,MUIM_Notify,MUIA_String_Contents,    MUIV_EveryTime,MUIV_Notify_Application,2,MUIM_CallHook,&US_PutUSEntryHook);
         DoMethod(data->GUI.ST_PASSWD, MUIM_Notify,MUIA_String_Contents,    MUIV_EveryTime,MUIV_Notify_Application,2,MUIM_CallHook,&US_PutUSEntryHook);
         DoMethod(data->GUI.CH_USEADDR,MUIM_Notify,MUIA_Selected,           MUIV_EveryTime,MUIV_Notify_Application,2,MUIM_CallHook,&US_PutUSEntryHook);
         DoMethod(data->GUI.CH_USEDICT,MUIM_Notify,MUIA_Selected,           MUIV_EveryTime,MUIV_Notify_Application,2,MUIM_CallHook,&US_PutUSEntryHook);
         DoMethod(data->GUI.CH_ROOT,   MUIM_Notify,MUIA_Selected,           MUIV_EveryTime,MUIV_Notify_Application,2,MUIM_CallHook,&US_PutUSEntryHook);
         DoMethod(data->GUI.CH_CLONE,  MUIM_Notify,MUIA_Selected,           MUIV_EveryTime,MUIV_Notify_Application,2,MUIM_CallHook,&US_PutUSEntryHook);
         DoMethod(data->GUI.BT_ADD,    MUIM_Notify,MUIA_Pressed,            FALSE         ,MUIV_Notify_Application,2,MUIM_CallHook,&US_AddHook);
         DoMethod(data->GUI.BT_DEL,    MUIM_Notify,MUIA_Pressed,            FALSE         ,MUIV_Notify_Application,2,MUIM_CallHook,&US_DelHook);
         DoMethod(data->GUI.WI,        MUIM_Notify,MUIA_Window_CloseRequest,TRUE          ,MUIV_Notify_Application,2,MUIM_CallHook,&US_CloseHook);
         return data;
      }
      free(data);
   }
   return NULL;
}
///
