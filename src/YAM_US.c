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

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <clib/alib_protos.h>
#include <libraries/asl.h>
#include <libraries/iffparse.h>
#include <mui/NList_mcc.h>
#include <mui/NListview_mcc.h>
#include <proto/exec.h>
#include <proto/intuition.h>
#include <proto/muimaster.h>

#include "extra.h"
#include "SDI_hook.h"

#include "YAM.h"
#include "YAM_config.h"
#include "YAM_error.h"
#include "YAM_locale.h"
#include "YAM_userlist.h"
#include "YAM_utilities.h"
#include "classes/Classes.h"

#include "Debug.h"

/* local protos */
static void US_SaveUsers(void);
static void US_LoadUsers(void);
static BOOL US_SaveUserList(void);
static struct US_ClassData *US_New(BOOL);

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
static void US_SaveUsers(void)
{
  FILE *fh;
  const char *fname = "PROGDIR:.users";
  int i;

  ENTER();

  for(i = 0; i < G->Users.Num; i++)
  {
    if(!G->Users.User[i].Limited)
      break;
  }

  if(i == G->Users.Num)
    G->Users.User[0].Limited = FALSE;

  if((fh = fopen(fname, "w")))
  {
    int i;

    fputs("YUS2 - YAM Users\n", fh);
    for(i = 0; i < G->Users.Num; i++)
    {
      struct User *u = &G->Users.User[i];
      if(u->Name)
        fprintf(fh, "@USER %s\n%s\n%d\n%s\n@ENDUSER\n", u->Name, u->MailDir, u->Limited*4+u->UseAddr*2+u->UseDict, Encrypt(u->Password));
    }
    fclose(fh);
    AppendLogVerbose(62, GetStr(MSG_LOG_SavingUsers));
  }
  else
    ER_NewError(GetStr(MSG_ER_CantCreateFile), fname);

  LEAVE();
}

///
/// US_LoadUsers
//  Loads user database from .users
static void US_LoadUsers(void)
{
   BOOL save = FALSE;
   FILE *fh;
   char buffer[SIZE_LARGE];
   memset(&G->Users, 0, sizeof(struct Users));
   G->Users.Num = 0;
   if ((fh = fopen("PROGDIR:.users", "r")))
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
               strlcpy(u->Name, Trim(&buffer[6]), sizeof(u->Name));
               strlcpy(u->MailDir, Trim(GetLine(fh, buffer, SIZE_LARGE)), sizeof(u->MailDir));
               if(!*u->MailDir)
               {
                 strlcpy(u->MailDir, G->MA_MailDir, sizeof(u->MailDir));
                 save = TRUE;
               }

               if(FileType(u->MailDir) != FIT_DRAWER)
               {
                  ER_NewError(GetStr(MSG_ER_UserRemoved), u->MailDir, u->Name);
                  u->Name[0] = 0;
                  save = TRUE;
               }
               else
               {
                  int flags = atoi(Trim(GetLine(fh, buffer, SIZE_LARGE)));
                  u->Limited = isFlagSet(flags, 4);
                  u->UseAddr = isFlagSet(flags, 2);
                  u->UseDict = isFlagSet(flags, 1);
                  if (!u->Limited) hasmanager = TRUE;

                  if(ver >= 2)
                    strlcpy(u->Password, Decrypt(GetLine(fh, buffer, SIZE_LARGE)), sizeof(u->Password));

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
      strlcpy(u->MailDir, G->MA_MailDir, sizeof(u->MailDir));
      u->UseAddr = u->UseDict = TRUE;
      u->ID = GetSimpleID();
      G->Users.Num = 1;
      save = TRUE;
   }
   if (save) US_SaveUsers();
}

///
/// US_Login
//  User login: puts up user list and waits for a selection
BOOL US_Login(const char *username, const char *password,
              const char *maildir, const char *prefsfile)
{
   int i, user = -1;
   struct User *u;
   BOOL loggedin = TRUE;

   // Load the .users file first
   US_LoadUsers();

   // if a username was given check if we have this username in
   // our loaded user list
   if(username)
   {
      for(i = 0; i < G->Users.Num; i++)
      {
        if(!stricmp(G->Users.User[i].Name, username))
        {
          user = i;
          break;
        }
      }
   }

   // if we didn't find the specified user in our .users file or
   // if no username was given lets query in the splash window
   // for it.
   if(user < 0)
   {
      password = NULL;
      if(G->Users.Num > 1)
      {
        if((user = DoMethod(G->SplashWinObject, MUIM_Splashwindow_SelectUser)) < 0)
          return FALSE;
      }
      else if(username)
        return FALSE;
      else
        user = 0;
   }

   u = &G->Users.User[user];
   G->Users.CurrentID = u->ID;
   strlcpy(G->MA_MailDir, maildir ? maildir : u->MailDir, sizeof(G->MA_MailDir));

   if(prefsfile)
     strlcpy(G->CO_PrefsFile, prefsfile, sizeof(G->CO_PrefsFile));
   else
     strmfp(G->CO_PrefsFile, G->MA_MailDir, ".config");

   strmfp(G->AB_Filename, u->UseAddr ? G->ProgDir : G->MA_MailDir, ".addressbook");
   strmfp(G->DI_Filename, u->UseDict ? G->ProgDir : G->MA_MailDir, ".glossary");

   if(u->Password[0])
   {
      if(password)
        loggedin = (!strcmp(password, u->Password) || *password == '\01');
      else
        loggedin = DoMethod(G->SplashWinObject, MUIM_Splashwindow_PasswordRequest, u);
   }

   return loggedin;
}

///
/// US_DelFunc
//  Removes a user from the user database
HOOKPROTONHNONP(US_DelFunc, void)
{
   int i, m;
   struct User *user;
   APTR lv = G->US->GUI.LV_USERS;

   i = xget(lv, MUIA_NList_Active);
   DoMethod(lv, MUIM_NList_GetEntry, i, &user);

   if(*user->MailDir)
   {
      if(!(m = MUI_Request(G->App, G->US->GUI.WI, 0, GetStr(MSG_MA_ConfirmReq), GetStr(MSG_US_RemoveReqGads), GetStr(MSG_US_RemoveReq))))
        return;

      if(m == 1)
        DeleteMailDir(user->MailDir, TRUE);
   }
   DoMethod(lv, MUIM_NList_Remove, i);
}
MakeStaticHook(US_DelHook, US_DelFunc);

///
/// US_AddFunc
//  Adds a new user to the user database
HOOKPROTONHNONP(US_AddFunc, void)
{
   struct US_GUIData *gui = &G->US->GUI;
   struct User user;
   int n = xget(gui->LV_USERS, MUIA_NList_Entries);
   if (n < MAXUSERS-1)
   {
      memset(&user, 0, sizeof(struct User));
      user.Limited = user.IsNew = TRUE;
      DoMethod(gui->LV_USERS, MUIM_NList_InsertSingle, &user, MUIV_NList_Insert_Bottom);
      set(gui->LV_USERS, MUIA_NList_Active, MUIV_NList_Active_Bottom);
      set(gui->WI, MUIA_Window_ActiveObject, gui->ST_USER);
   }
}
MakeStaticHook(US_AddHook, US_AddFunc);

///
/// US_SaveUserList
//  Initializes configuration files for new users and saves the user database
static BOOL US_SaveUserList(void)
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
      {
         if (*u->MailDir)
         {
            if(FileType(u->MailDir) != FIT_DRAWER)
            {
               if (MUI_Request(G->App, G->US->GUI.WI, 0, GetStr(MSG_MA_MUsers), GetStr(MSG_YesNoReq), GetStr(MSG_US_ErrorNoDirectory)))
               {
                  if (CreateDirectory(u->MailDir)) valid = TRUE;
                  else ER_NewError(GetStr(MSG_ER_CantCreateDir), u->MailDir);
               }
            }
            else valid = TRUE;
         }
         else ER_NewError(GetStr(MSG_ER_MissingDirectory));
      }
      else ER_NewError(GetStr(MSG_ER_MissingName));
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
HOOKPROTONHNONP(US_OpenFunc, void)
{
   if (!G->US)
   {
      int i;
      struct User *u = 0;
      for (i = 0; i < G->Users.Num; i++) if (G->Users.User[i].ID == G->Users.CurrentID) u = &G->Users.User[i];

      if(!*G->Users.User[0].Name)
        strlcpy(G->Users.User[0].Name, C->RealName, sizeof(G->Users.User[0].Name));

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
HOOKPROTONHNONP(US_CloseFunc, void)
{
  if (US_SaveUserList())
  {
    DisposeModulePush(&G->US);
  }
}
MakeStaticHook(US_CloseHook, US_CloseFunc);

///
/// US_GetUSEntryFunc
//  Fills form with data from selected list entry
HOOKPROTONHNONP(US_GetUSEntryFunc, void)
{
   struct User *user;
   struct US_GUIData *gui = &G->US->GUI;
   BOOL notallowed, iscurrent, limited = !G->US->Supervisor;
   int act = xget(gui->LV_USERS, MUIA_NList_Active);

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
MakeStaticHook(US_GetUSEntryHook,US_GetUSEntryFunc);

///
/// US_PutUSEntryFunc
//  Fills form data into selected list entry
HOOKPROTONHNONP(US_PutUSEntryFunc, void)
{
   struct User *user = NULL;
   struct US_GUIData *gui = &G->US->GUI;

   DoMethod(gui->LV_USERS, MUIM_NList_GetEntry, MUIV_NList_GetEntry_Active, &user);
   if (user)
   {
      GetMUIString(user->Name, gui->ST_USER, sizeof(user->Name));
      GetMUIString(user->MailDir, gui->ST_MAILDIR, sizeof(user->MailDir));
      GetMUIString(user->Password, gui->ST_PASSWD, sizeof(user->Password));
      user->UseAddr = GetMUICheck(gui->CH_USEADDR);
      user->UseDict = GetMUICheck(gui->CH_USEDICT);
      user->Limited = !GetMUICheck(gui->CH_ROOT);
      user->Clone   = GetMUICheck(gui->CH_CLONE);
      DoMethod(gui->LV_USERS, MUIM_NList_Redraw, MUIV_NList_Redraw_Active);
   }
}
MakeStaticHook(US_PutUSEntryHook,US_PutUSEntryFunc);
///

/*** GUI ***/
/// US_LV_ConFunc
//  User listview construction hook
HOOKPROTONHNO(US_LV_ConFunc, struct User *, struct User *user)
{
   struct User *entry = malloc(sizeof(struct User));
   if (entry)
     *entry = *user;
   return entry;
}
MakeStaticHook(US_LV_ConHook, US_LV_ConFunc);

///
/// US_LV_DspFunc
//  User listview display hook
HOOKPROTONH(US_LV_DspFunc, long, char **array, struct User *entry)
{
  if(entry)
  {
    array[0] = entry->Name;
    array[1] = entry->MailDir;
    if(entry->ID == G->Users.CurrentID)
      array[DISPLAY_ARRAY_MAX] = (char *)"\0338";
  }
  else
  {
    array[0] = (STRPTR)GetStr(MSG_US_TitleUserName);
    array[1] = (STRPTR)GetStr(MSG_US_TitleMailDir);
  }

  return 0;
}
MakeStaticHook(US_LV_DspHook,US_LV_DspFunc);

///
/// US_New
//  Creates user list window
static struct US_ClassData *US_New(BOOL supervisor)
{
   struct US_ClassData *data = calloc(1, sizeof(struct US_ClassData));
   if (data)
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
//                  MUIA_NList_DisplayHook, &MA_LV_FDspFuncHook,
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
