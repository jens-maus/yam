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
#include <proto/dos.h>
#include <proto/intuition.h>
#include <proto/muimaster.h>

#include "YAM.h"
#include "YAM_addressbook.h"
#include "YAM_addressbookEntry.h"
#include "YAM_classes.h"
#include "YAM_config.h"
#include "YAM_error.h"
#include "YAM_hook.h"
#include "YAM_locale.h"
#include "YAM_transfer.h"
#include "YAM_utilities.h"

/* local protos */
static void EA_SetPhoto(int winnum, char *fname);
static int EA_Open(int);
static struct EA_ClassData *EA_New(int, int);

/***************************************************************************
 Module: Address book entry
***************************************************************************/

/*** Init & Open ***/
/// EA_Init
//  Creates and opens an address book entry window
int EA_Init(enum ABEntry_Type type, struct MUI_NListtree_TreeNode *tn)
{
   struct EA_ClassData *ea;
   int winnum;
   char *title = "";
   if ((winnum = EA_Open(type)) < 0) return -1;
   ea = G->EA[winnum];
   ea->EditNode   = tn;
   switch (type)
   {
      case AET_USER:  title = tn ? GetStr(MSG_EA_EditUser) : GetStr(MSG_AB_AddUser); break;
      case AET_LIST:  title = tn ? GetStr(MSG_EA_EditList) : GetStr(MSG_AB_AddList); break;
      case AET_GROUP: title = tn ? GetStr(MSG_EA_EditGroup): GetStr(MSG_AB_AddGroup);
   }
   set(ea->GUI.WI, MUIA_Window_Title, title);
   if (!SafeOpenWindow(ea->GUI.WI)) { DisposeModulePush(&G->EA[winnum]); return -1; }
   set(ea->GUI.WI, MUIA_Window_ActiveObject, ea->GUI.ST_ALIAS);
   return winnum;
}       

///
/// EA_Setup
//  Setup GUI fields with data from adress book entry
void EA_Setup(int winnum, struct ABEntry *ab)
{
   struct EA_GUIData *gui = &(G->EA[winnum]->GUI);
   char *ptr;

   switch (ab->Type)
   {
      case AET_USER:   setstring(gui->ST_ALIAS, ab->Alias);
                       setstring(gui->ST_REALNAME, ab->RealName);
                       setstring(gui->ST_ADDRESS, ab->Address);
                       setstring(gui->ST_PHONE, ab->Phone);
                       setstring(gui->ST_STREET, ab->Street);
                       setstring(gui->ST_CITY, ab->City);
                       setstring(gui->ST_COUNTRY, ab->Country);
                       nnset(gui->ST_PGPKEY,MUIA_String_Contents,ab->PGPId);
                       /* avoid triggering notification to "default security" cycle */
                       setcycle(gui->CY_DEFSECURITY,ab->DefSecurity);
                       setstring(gui->ST_HOMEPAGE, ab->Homepage);
                       setstring(gui->ST_COMMENT, ab->Comment);
                       setstring(gui->ST_BIRTHDAY, AB_ExpandBD(ab->BirthDay));
                       EA_SetPhoto(winnum, ab->Photo);
                       break;
      case AET_LIST:   setstring(gui->ST_ALIAS, ab->Alias);
                       setstring(gui->ST_REALNAME, ab->RealName);
                       setstring(gui->ST_ADDRESS, ab->Address);
                       setstring(gui->ST_COMMENT, ab->Comment);
                       DoMethod(gui->LV_MEMBER, MUIM_List_Clear);
                       for (ptr = ab->Members; *ptr; ptr++)
                       {
                          char *nptr = strchr(ptr, '\n');
                          if (nptr) *nptr = 0; else break;
                          DoMethod(gui->LV_MEMBER, MUIM_List_InsertSingle, ptr, MUIV_List_Insert_Bottom);
                          *nptr = '\n';
                          ptr = nptr;
                       }
                       break;
      case AET_GROUP:  setstring(gui->ST_ALIAS, ab->Alias);
                       setstring(gui->ST_COMMENT, ab->Comment);
                       break;
   }
}
///

/*** Private functions (member list) ***/
/// EA_AddSingleMember
//  Adds a single entry to the member list by Drag&Drop
void EA_AddSingleMember(Object *obj, struct MUI_NListtree_TreeNode *tn)
{
   struct ABEntry *ab;
   int dropmark;
   ab = tn->tn_User;
   get(obj, MUIA_List_DropMark, &dropmark);
   DoMethod(obj, MUIM_List_InsertSingle, ab->Alias ? ab->Alias : ab->RealName, dropmark);
}

///
/// EA_AddMembers (rec)
//  Adds an entire group to the member list by Drag&Drop
void STACKEXT EA_AddMembers(Object *obj, struct MUI_NListtree_TreeNode *list)
{
   struct MUI_NListtree_TreeNode *tn;
   int i;
   
   for (i=0; ; i++)
      if (tn = (struct MUI_NListtree_TreeNode *)DoMethod(G->AB->GUI.LV_ADRESSES, MUIM_NListtree_GetEntry, list, i, MUIV_NListtree_GetEntry_Flag_SameLevel))
         if (tn->tn_Flags & TNF_LIST) EA_AddMembers(obj, tn);
         else EA_AddSingleMember(obj, tn);
      else break;
}

///
/// EA_GetEntry
//  Fills string gadget with data from selected list entry
HOOKPROTONHNO(EA_GetEntry, void, int *arg)
{
   int winnum = *arg;
   char *entry = NULL;
   DoMethod(G->EA[winnum]->GUI.LV_MEMBER, MUIM_List_GetEntry, MUIV_List_GetEntry_Active, &entry);
   if (entry) nnset(G->EA[winnum]->GUI.ST_MEMBER, MUIA_String_Contents, entry);
}
MakeStaticHook(EA_GetEntryHook, EA_GetEntry);

/*** EA_AddFunc - Adds a new entry to the member list ***/
HOOKPROTONHNO(EA_AddFunc, void, int *arg)
{
   struct EA_GUIData *gui = &(G->EA[*arg]->GUI);
   char *buf;
   
   get(gui->ST_MEMBER, MUIA_String_Contents, &buf);
   if (*buf) 
   {
      DoMethod(gui->LV_MEMBER, MUIM_List_InsertSingle, buf, MUIV_List_Insert_Bottom);
      nnset(gui->LV_MEMBER, MUIA_List_Active, MUIV_List_Active_Off);
      setstring(gui->ST_MEMBER, "");
   }
   set(gui->WI, MUIA_Window_ActiveObject, gui->ST_MEMBER);
}
MakeStaticHook(EA_AddHook, EA_AddFunc);

///
/// EA_PutEntry
//  Updates selected list entry
HOOKPROTONHNO(EA_PutEntry, void, int *arg)
{
   struct EA_GUIData *gui = &(G->EA[*arg]->GUI);
   char *buf;
   int active;
   
   get(gui->LV_MEMBER, MUIA_List_Active, &active);
   if (active == MUIV_List_Active_Off) DoMethod(G->App, MUIM_CallHook, &EA_AddHook, *arg);
   else
   {
      get(gui->ST_MEMBER, MUIA_String_Contents, &buf);
      DoMethod(gui->LV_MEMBER, MUIM_List_InsertSingle, buf, active);
      DoMethod(gui->LV_MEMBER, MUIM_List_Remove, active+1);
   }
}
MakeStaticHook(EA_PutEntryHook, EA_PutEntry);

///
/// EA_InsertBelowActive
//  Inserts an entry into the address book tree
void EA_InsertBelowActive(struct ABEntry *addr, int flags)
{
  APTR lt = G->AB->GUI.LV_ADRESSES;
  struct MUI_NListtree_TreeNode *node, *list;

  // get the active node
  get(lt, MUIA_NListtree_Active, &node);

  if (node == MUIV_NListtree_Active_Off)
  {
    list = MUIV_NListtree_Insert_ListNode_Root;
    node = (struct MUI_NListtree_TreeNode *)MUIV_NListtree_Insert_PrevNode_Sorted;
  }
  else
  {
    list = (struct MUI_NListtree_TreeNode *)GetMUI(lt, MUIA_NListtree_ActiveList);
  }

  // now we insert the node in the list accordingly and set it active automatically
  DoMethod(lt, MUIM_NListtree_Insert, addr->Alias, addr, list, node, (flags | MUIV_NListtree_Insert_Flag_Active), TAG_DONE);
}

///
/// EA_FixAlias
//  Avoids ambiguos aliases
void EA_FixAlias(struct ABEntry *ab, BOOL excludemyself)
{
   char alias[SIZE_NAME];
   int c = 1, l, hits = 0;
   struct MUI_NListtree_TreeNode *tn;
   strcpy(alias, ab->Alias);
   while (AB_SearchEntry(MUIV_NListtree_GetEntry_ListNode_Root, alias, ASM_ALIAS|ASM_USER|ASM_LIST|ASM_GROUP, &hits, &tn))
   {
      if (tn->tn_User == ab && excludemyself && hits == 1) break;
      if ((l = strlen(ab->Alias)) > SIZE_NAME-2) l = SIZE_NAME-2;
      sprintf(&alias[l], "%ld", ++c); hits = 0;
   }
   strcpy(ab->Alias, alias);
}

///
/// EA_SetDefaultAlias
//  Creates an alias from the real name if user left it empty
void EA_SetDefaultAlias(struct ABEntry *ab)
{
   char *p = ab->Alias, *ln;

   memset(p, 0, SIZE_NAME);
   if (*ab->RealName)
   {
      if (ln = strrchr(ab->RealName, ' '))
      {
         if (isAlNum(ab->RealName[0])) { *p++ = ab->RealName[0]; *p++ = '_'; }
         ln++;
      }
      else ln = ab->RealName;
      for (; strlen(ab->Alias)<SIZE_NAME-2 && *ln; ln++) if (isAlNum(*ln)) *p++ = *ln;
   }
   else for (ln = ab->Address; strlen(ab->Alias)<SIZE_NAME-2 && *ln && *ln != '@'; ln++) if (isAlNum(*ln)) *p++ = *ln;
}
///

/*** Buttons ***/
/// EA_Okay
//  Saves changes to the edited entry in the address book
HOOKPROTONHNO(EA_Okay, void, int *arg)
{
   static struct ABEntry newaddr;
   struct ABEntry *addr;
   char *members, *str;
   int i, winnum = *arg;
   long bdate = 0;
   struct EA_GUIData *gui = &(G->EA[winnum]->GUI);
   BOOL old = G->EA[winnum]->EditNode != NULL;

   memset(&newaddr, 0, sizeof(struct ABEntry));
   if (G->EA[winnum]->Type)
   {
      get(gui->ST_ALIAS, MUIA_String_Contents, &str);
      if (!*str) { ER_NewError(GetStr(MSG_ER_ErrorNoAlias), NULL, NULL); return; }
   }
   else
   {
      get(gui->ST_ADDRESS, MUIA_String_Contents, &str);
      if (!*str) { ER_NewError(GetStr(MSG_ER_ErrorNoAddress), NULL, NULL); return; }
      get(gui->ST_BIRTHDAY, MUIA_String_Contents, &str);
      if (*str) if (!(bdate = AB_CompressBD(str))) { ER_NewError(GetStr(MSG_ER_ErrorDOBformat), NULL, NULL); return; }
   }
   set(gui->WI, MUIA_Window_Open, FALSE);
   G->AB->Modified = TRUE;
   if (old) addr = G->EA[winnum]->EditNode->tn_User; else addr = &newaddr;
   GetMUIString(addr->Alias, gui->ST_ALIAS);
   GetMUIString(addr->Comment, gui->ST_COMMENT);
   switch (addr->Type = G->EA[winnum]->Type)
   {
      case AET_USER:  GetMUIString(addr->RealName, gui->ST_REALNAME);
                      GetMUIString(addr->Address, gui->ST_ADDRESS);
                      GetMUIString(addr->Phone, gui->ST_PHONE);
                      GetMUIString(addr->Street, gui->ST_STREET);
                      GetMUIString(addr->City, gui->ST_CITY);
                      GetMUIString(addr->Country, gui->ST_COUNTRY);
                      GetMUIString(addr->PGPId, gui->ST_PGPKEY);
                      GetMUIString(addr->Homepage, gui->ST_HOMEPAGE);
                      addr->DefSecurity = GetMUICycle(gui->CY_DEFSECURITY);
                      strcpy(addr->Photo, G->EA[winnum]->PhotoName);
                      addr->BirthDay = bdate;
                      if (!*addr->Alias) EA_SetDefaultAlias(addr);
                      EA_FixAlias(addr, old);
                      if (!old) EA_InsertBelowActive(addr, 0);
                      break;
      case AET_LIST:  GetMUIString(addr->RealName, gui->ST_REALNAME);
                      GetMUIString(addr->Address, gui->ST_ADDRESS);
                      members = AllocStrBuf(SIZE_DEFAULT);
                      for (i = 0; ; i++)
                      {
                         char *p;
                         DoMethod(gui->LV_MEMBER, MUIM_List_GetEntry, i, &p);
                         if (!p) break;
                         members = StrBufCat(members, p);
                         members = StrBufCat(members, "\n");
                      }
                      if (old) addr->Members = realloc(addr->Members, strlen(members)+1);
                      else     addr->Members = malloc(strlen(members)+1);
                      strcpy(addr->Members, members);
                      EA_FixAlias(addr, old);
                      if (!old)
                      {
                         EA_InsertBelowActive(addr, 0);
                         free(addr->Members);
                      }
                      FreeStrBuf(members);
                      break;
      case AET_GROUP: EA_FixAlias(addr, old);
                      if (!old) EA_InsertBelowActive(addr, TNF_LIST);
   }
   if (old) DoMethod(G->AB->GUI.LV_ADRESSES, MUIM_List_Redraw, MUIV_List_Redraw_All);
   else AppendLogVerbose(71, GetStr(MSG_LOG_NewAddress), addr->Alias, "", "", "");
   DisposeModulePush(&G->EA[winnum]);
}
MakeStaticHook(EA_OkayHook, EA_Okay);

///
/// EA_SetPhoto
//  Updates the portrait image
static void EA_SetPhoto(int winnum, char *fname)
{
   struct EA_GUIData *gui = &(G->EA[winnum]->GUI);

   if (fname) strcpy(G->EA[winnum]->PhotoName, fname);
   fname = G->EA[winnum]->PhotoName;
   if (*fname) if (DoMethod(gui->GR_PHOTO, MUIM_Group_InitChange))
   {
      DoMethod(gui->GR_PHOTO, OM_REMMEMBER, gui->BC_PHOTO);
      MUI_DisposeObject(gui->BC_PHOTO);
      gui->BC_PHOTO = MakePicture(fname);
      DoMethod(gui->GR_PHOTO, OM_ADDMEMBER, gui->BC_PHOTO);
      DoMethod(gui->GR_PHOTO, MUIM_Group_ExitChange);
   }
}

///
/// EA_SelectPhotoFunc
//  Lets user select an image file to be used as portrait
HOOKPROTONHNO(EA_SelectPhotoFunc, void, int *arg)
{
   int winnum = *arg;

   if (ReqFile(ASL_PHOTO,G->EA[winnum]->GUI.WI, GetStr(MSG_EA_SelectPhoto_Title), 0, C->GalleryDir, ""))
   {
      strmfp(G->EA[winnum]->PhotoName, G->ASLReq[ASL_PHOTO]->fr_Drawer, G->ASLReq[ASL_PHOTO]->fr_File);
      EA_SetPhoto(winnum, NULL);
   }
}
MakeStaticHook(EA_SelectPhotoHook, EA_SelectPhotoFunc);

///
/// EA_DownloadPhotoFunc
//  Downloads a portrait from the YAM user gallery
HOOKPROTONHNO(EA_DownloadPhotoFunc, void, int *arg)
{
   int winnum = *arg, c;
   struct EA_GUIData *gui = &(G->EA[winnum]->GUI);
   char *p, dbfile[SIZE_PATHFILE], *name, *addr, *homepage, newname[SIZE_DEFAULT], dbentry[5][SIZE_DEFAULT];
   BOOL success = FALSE;
   FILE *db;

   get(gui->ST_REALNAME, MUIA_String_Contents, &name);
   get(gui->ST_ADDRESS, MUIA_String_Contents, &addr);
   get(gui->ST_HOMEPAGE, MUIA_String_Contents, &homepage);
   if (*addr || *name)
   {
      strcpy(dbfile, name);
      if (p = strrchr(dbfile, ' ')) { *p = 0; sprintf(newname, "%s, %s", p+1, dbfile); }
      else strcpy(newname, name);
      strmfp(dbfile, C->TempDir, "gallery.db");
      if (TR_OpenTCPIP())
      {
         Busy(GetStr(MSG_BusyDownloadingPic), name, 0, 0);
         if (!(db = fopen(dbfile, "r")))
            if (TR_DownloadURL(C->SupportSite, "gallery", "database", dbfile))
               db = fopen(dbfile, "r");
         if (db)
         {
            for (c = 0; GetLine(db, dbentry[c], SIZE_DEFAULT); c++) if (c == 4)
            {
               c = -1;
               if (!stricmp(dbentry[0], name) || !stricmp(dbentry[0], newname) || !stricmp(dbentry[1], addr))
               {
                  if (!*name) setstring(gui->ST_REALNAME, dbentry[0]);
                  if (!*addr) setstring(gui->ST_ADDRESS, dbentry[1]);
                  if (!*homepage) setstring(gui->ST_HOMEPAGE, dbentry[3]);
                  CreateDirectory(C->GalleryDir);
                  strmfp(dbfile, C->GalleryDir, FilePart(dbentry[2]));
                  if (TR_DownloadURL(C->SupportSite, "gallery", dbentry[2], dbfile)) EA_SetPhoto(winnum, dbfile);
                  success = TRUE; break;
               }
            }
            fclose(db);
            if (!success) ER_NewError(GetStr(MSG_ER_NotInGallery), NULL, NULL);
         }
         BusyEnd;
         TR_CloseTCPIP();
      }
      else ER_NewError(GetStr(MSG_ER_NoTCP), NULL, NULL);
   }
}
MakeStaticHook(EA_DownloadPhotoHook, EA_DownloadPhotoFunc);

///
/// EA_HomepageFunc
//  Launches a browser to view the homepage of the person
HOOKPROTONHNO(EA_HomepageFunc, void, int *arg)
{
   char *url;
   get(G->EA[*arg]->GUI.ST_HOMEPAGE, MUIA_String_Contents, &url);
   if (*url) GotoURL(url);
}
MakeStaticHook(EA_HomepageHook, EA_HomepageFunc);

///
/// EA_Open
//  Assigns a number for a new window
static int EA_Open(int type)
{
   int winnum;
   for (winnum = 0; winnum < 4; winnum++) if (!G->EA[winnum]) break;
   if (winnum == 4) return -1;
   if (!(G->EA[winnum] = EA_New(winnum, type))) return -1;
   return winnum;
}
///
/// EA_CloseFunc
//  Closes address book entry window
HOOKPROTONHNO(EA_CloseFunc, void, int *arg)
{
   int winnum = *arg;
   DisposeModulePush(&G->EA[winnum]);
}
MakeStaticHook(EA_CloseHook, EA_CloseFunc);
///

/*** GUI ***/
/// EA_New
//  Creates address book entry window
static struct EA_ClassData *EA_New(int winnum, int type)
{
   struct EA_ClassData *data;

   if (data = calloc(1,sizeof(struct EA_ClassData)))
   {
      static STRPTR SecurityCycleEntries[6] = {NULL};
      APTR group = NULL, bt_homepage, bt_sort;

      data->Type = type;
      switch (type)
      {
         case AET_USER:
          /* initialize string array for cycle object on first invocation */
          if(NULL == SecurityCycleEntries[0])
          {
            static const APTR SecurityCycleStrings[] = {MSG_WR_SecNone,MSG_WR_SecSign,MSG_WR_SecEncrypt,MSG_WR_SecBoth,MSG_WR_SecAnon};
            int i;

            for(i=0; i<5; i++) SecurityCycleEntries[i] = GetStr(SecurityCycleStrings[i]);
          }

          /* build MUI object tree */
          group = HGroup,
               MUIA_Group_SameWidth, TRUE,
               Child, VGroup,
                  Child, ColGroup(2), GroupFrameT(GetStr(MSG_EA_ElectronicMail)),
                     Child, Label2(GetStr(MSG_EA_Alias)),
                     Child, data->GUI.ST_ALIAS = MakeString(SIZE_NAME,GetStr(MSG_EA_Alias)),
                     Child, Label2(GetStr(MSG_EA_RealName)),
                     Child, data->GUI.ST_REALNAME = MakeString(SIZE_REALNAME,GetStr(MSG_EA_RealName)),
                     Child, Label2(GetStr(MSG_EA_EmailAddress)),
                     Child, data->GUI.ST_ADDRESS  = MakeString(SIZE_ADDRESS,GetStr(MSG_EA_EmailAddress)),
                     Child, Label2(GetStr(MSG_EA_PGPId)),
                     Child, MakePGPKeyList(&(data->GUI.ST_PGPKEY), FALSE, GetStr(MSG_EA_PGPId)),
                     Child, Label2(GetStr(MSG_EA_Homepage)),
                     Child, HGroup,
                        MUIA_Group_HorizSpacing, 1,
                        Child, data->GUI.ST_HOMEPAGE = MakeString(SIZE_URL,GetStr(MSG_EA_Homepage)),
                        Child, bt_homepage = PopButton(MUII_TapeRecord),
                     End,
              Child, Label2(GetStr(MSG_EA_DefSecurity)),
              Child, data->GUI.CY_DEFSECURITY = CycleObject,
                MUIA_Cycle_Entries, SecurityCycleEntries,
              End,
                  End,
                  Child, ColGroup(2), GroupFrameT(GetStr(MSG_EA_SnailMail)),
                     Child, Label2(GetStr(MSG_EA_Street)),
                     Child, data->GUI.ST_STREET = MakeString(SIZE_DEFAULT,GetStr(MSG_EA_Street)),
                     Child, Label2(GetStr(MSG_EA_City)),
                     Child, data->GUI.ST_CITY = MakeString(SIZE_DEFAULT,GetStr(MSG_EA_City)),
                     Child, Label2(GetStr(MSG_EA_Country)),
                     Child, data->GUI.ST_COUNTRY = MakeString(SIZE_DEFAULT,GetStr(MSG_EA_Country)),
                     Child, Label2(GetStr(MSG_EA_Phone)),
                     Child, data->GUI.ST_PHONE = MakeString(SIZE_DEFAULT,GetStr(MSG_EA_Phone)),
                  End,
               End,
               Child, VGroup,
                  Child, ColGroup(2), GroupFrameT(GetStr(MSG_EA_Miscellaneous)),
                     Child, Label2(GetStr(MSG_EA_Description)),
                     Child, data->GUI.ST_COMMENT = MakeString(SIZE_DEFAULT,GetStr(MSG_EA_Description)),
                     Child, Label2(GetStr(MSG_EA_DOB)),
                     Child, data->GUI.ST_BIRTHDAY = MakeString(SIZE_SMALL,GetStr(MSG_EA_DOB)),
                  End,
                  Child, VGroupV, GroupFrameT(GetStr(MSG_EA_Portrait)),
                     Child, ColGroup(2),
                        Child, data->GUI.BT_SELECTPHOTO = MakeButton(GetStr(MSG_EA_SelectPhoto)),
                        Child, data->GUI.BT_LOADPHOTO = MakeButton(GetStr(MSG_EA_LoadPhoto)),
                     End,
                     Child, HGroup,
                        Child, HSpace(0),
                        Child, data->GUI.GR_PHOTO = HGroup,
                           Child, data->GUI.BC_PHOTO = RectangleObject, MUIA_FixWidth, 100, MUIA_FixHeight, 80, End,
                           ImageButtonFrame,
                        End,
                        Child, HSpace(0),
                     End,
                     Child, VSpace(0),
                  End,
               End,
            End; 
            if (group) 
            {
               set(data->GUI.BT_LOADPHOTO, MUIA_Disabled, !*C->GalleryDir);
               SetHelp(data->GUI.ST_REALNAME   ,MSG_HELP_EA_ST_REALNAME   );
               SetHelp(data->GUI.ST_ADDRESS    ,MSG_HELP_EA_ST_ADDRESS    );
               SetHelp(data->GUI.ST_PGPKEY     ,MSG_HELP_EA_ST_PGPKEY     );
               SetHelp(data->GUI.ST_HOMEPAGE   ,MSG_HELP_EA_ST_HOMEPAGE   );
               SetHelp(data->GUI.CY_DEFSECURITY,MSG_HELP_MA_CY_DEFSECURITY);
               SetHelp(data->GUI.ST_STREET     ,MSG_HELP_EA_ST_STREET     );
               SetHelp(data->GUI.ST_CITY       ,MSG_HELP_EA_ST_CITY       );
               SetHelp(data->GUI.ST_COUNTRY    ,MSG_HELP_EA_ST_COUNTRY    );
               SetHelp(data->GUI.ST_PHONE      ,MSG_HELP_EA_ST_PHONE      );
               SetHelp(data->GUI.ST_BIRTHDAY   ,MSG_HELP_EA_ST_BIRTHDAY   );
               SetHelp(data->GUI.BC_PHOTO      ,MSG_HELP_EA_BC_PHOTO      );
               SetHelp(data->GUI.BT_SELECTPHOTO,MSG_HELP_EA_BT_SELECTPHOTO);
               SetHelp(data->GUI.BT_LOADPHOTO  ,MSG_HELP_EA_BT_LOADPHOTO  );

               DoMethod(data->GUI.BT_SELECTPHOTO, MUIM_Notify, MUIA_Pressed, FALSE, MUIV_Notify_Application, 3, MUIM_CallHook, &EA_SelectPhotoHook, winnum);
               DoMethod(data->GUI.BT_LOADPHOTO,   MUIM_Notify, MUIA_Pressed, FALSE, MUIV_Notify_Application, 3, MUIM_CallHook, &EA_DownloadPhotoHook, winnum);

               // when a key ID is selected, set default security to "encrypt"
               DoMethod(data->GUI.ST_PGPKEY, MUIM_Notify, MUIA_String_Contents, MUIV_EveryTime, data->GUI.CY_DEFSECURITY, 3, MUIM_Set, MUIA_Cycle_Active, 2);

               DoMethod(bt_homepage, MUIM_Notify, MUIA_Pressed, FALSE, MUIV_Notify_Application, 3, MUIM_CallHook, &EA_HomepageHook, winnum);
            }
            break;
         case AET_GROUP: group = ColGroup(2), GroupFrame,
               MUIA_Background, MUII_GroupBack,
               Child, Label2(GetStr(MSG_EA_Alias)),
               Child, data->GUI.ST_ALIAS = MakeString(SIZE_NAME,GetStr(MSG_EA_Alias)),
               Child, Label2(GetStr(MSG_EA_Description)),
               Child, data->GUI.ST_COMMENT = MakeString(SIZE_DEFAULT,GetStr(MSG_EA_Description)),
            End; break;
         case AET_LIST: group = HGroup,
               MUIA_Group_SameWidth, TRUE,
               Child, VGroup,
                  Child, ColGroup(2), GroupFrameT(GetStr(MSG_EA_ElectronicMail)),
                     MUIA_Background, MUII_GroupBack,
                     Child, Label2(GetStr(MSG_EA_Alias)),
                     Child, data->GUI.ST_ALIAS = MakeString(SIZE_NAME,GetStr(MSG_EA_Alias)),
                     Child, Label2(GetStr(MSG_EA_ReturnAddress)),
                     Child, data->GUI.ST_ADDRESS = MakeString(SIZE_ADDRESS,GetStr(MSG_EA_ReturnAddress)),
                     Child, Label2(GetStr(MSG_EA_MLName)),
                     Child, data->GUI.ST_REALNAME = MakeString(SIZE_REALNAME,GetStr(MSG_EA_MLName)),
                     Child, Label2(GetStr(MSG_EA_Description)),
                     Child, data->GUI.ST_COMMENT = MakeString(SIZE_DEFAULT,GetStr(MSG_EA_Description)),
                  End,
                  Child, VSpace(0),
               End,
               Child, VGroup, GroupFrameT(GetStr(MSG_EA_Members)),
                  Child, ListviewObject,
                     MUIA_CycleChain, 1,
                     MUIA_Listview_DragType, 1,
                     MUIA_Listview_List, data->GUI.LV_MEMBER = NewObject(CL_DDList->mcc_Class,NULL,
                        InputListFrame,
                        MUIA_List_DragSortable ,TRUE,
                        MUIA_List_ConstructHook,MUIV_List_ConstructHook_String,
                        MUIA_List_DestructHook ,MUIV_List_DestructHook_String,
                     End,
                  End,
                  Child, data->GUI.ST_MEMBER = MakeString(SIZE_ADDRESS,""),
                  Child, ColGroup(3), GroupSpacing(0),
                     Child, data->GUI.BT_ADD = MakeButton(GetStr(MSG_Add)),
                     Child, data->GUI.BT_DEL = MakeButton(GetStr(MSG_Del)),
                     Child, bt_sort          = MakeButton(GetStr(MSG_EA_Sort)),
                  End,
               End,
            End; 
            if (group)
            {
               SetHelp(data->GUI.ST_ALIAS   ,MSG_HELP_EA_ST_ALIAS      );
               SetHelp(data->GUI.ST_COMMENT ,MSG_HELP_EA_ST_DESCRIPTION);
               SetHelp(data->GUI.ST_REALNAME,MSG_HELP_EA_ST_REALNAME_L );
               SetHelp(data->GUI.ST_ADDRESS ,MSG_HELP_EA_ST_ADDRESS_L  );
               SetHelp(data->GUI.LV_MEMBER  ,MSG_HELP_EA_LV_MEMBERS    );
               SetHelp(data->GUI.ST_MEMBER  ,MSG_HELP_EA_ST_MEMBER     );
               SetHelp(data->GUI.BT_ADD     ,MSG_HELP_EA_BT_ADD        );
               SetHelp(data->GUI.BT_DEL     ,MSG_HELP_EA_BT_DEL        );
               SetHelp(bt_sort              ,MSG_HELP_EA_BT_SORT       );
               DoMethod(data->GUI.BT_ADD   ,MUIM_Notify,MUIA_Pressed            ,FALSE         ,MUIV_Notify_Application,3,MUIM_CallHook   ,&EA_AddHook,winnum);
               DoMethod(data->GUI.BT_DEL   ,MUIM_Notify,MUIA_Pressed            ,FALSE         ,data->GUI.LV_MEMBER,2,MUIM_List_Remove,MUIV_List_Remove_Active);
               DoMethod(bt_sort            ,MUIM_Notify,MUIA_Pressed            ,FALSE         ,data->GUI.LV_MEMBER,1,MUIM_List_Sort);
               DoMethod(data->GUI.ST_MEMBER,MUIM_Notify,MUIA_String_Acknowledge ,MUIV_EveryTime,MUIV_Notify_Application,3,MUIM_CallHook   ,&EA_PutEntryHook,winnum);
               DoMethod(data->GUI.LV_MEMBER,MUIM_Notify,MUIA_List_Active        ,MUIV_EveryTime,MUIV_Notify_Application,3,MUIM_CallHook   ,&EA_GetEntryHook,winnum);
            }
            break;
      }
      data->GUI.WI = WindowObject,
         MUIA_Window_Title, "",
         MUIA_HelpNode, "EA_W",
         MUIA_Window_ID, MAKE_ID('E','D','A','D'),
         WindowContents, VGroup,
            Child, group,
            Child, ColGroup(3),
               Child, data->GUI.BT_OKAY   = MakeButton(GetStr(MSG_Okay)),
               Child, HSpace(0),
               Child, data->GUI.BT_CANCEL = MakeButton(GetStr(MSG_Cancel)),
            End,
         End,
      End;
     if (data->GUI.WI)
     {
         DoMethod(G->App, OM_ADDMEMBER, data->GUI.WI);
         SetHelp(data->GUI.ST_ALIAS   ,MSG_HELP_EA_ST_ALIAS      );
         SetHelp(data->GUI.ST_COMMENT ,MSG_HELP_EA_ST_DESCRIPTION);
         SetHelp(data->GUI.BT_OKAY    ,MSG_HELP_EA_BT_OKAY       );
         SetHelp(data->GUI.BT_CANCEL  ,MSG_HELP_EA_BT_CANCEL     );
         set(data->GUI.ST_BIRTHDAY, MUIA_String_Accept, "0123456789-AaBbCcDdEeFfGgJjLlMmNnOoPpRrSsTtUuVvYy");
         DoMethod(data->GUI.BT_CANCEL,MUIM_Notify,MUIA_Pressed            ,FALSE         ,MUIV_Notify_Application,3,MUIM_CallHook,&EA_CloseHook,winnum);
         DoMethod(data->GUI.BT_OKAY  ,MUIM_Notify,MUIA_Pressed            ,FALSE         ,MUIV_Notify_Application,3,MUIM_CallHook,&EA_OkayHook,winnum);
         DoMethod(data->GUI.WI       ,MUIM_Notify,MUIA_Window_CloseRequest,TRUE          ,MUIV_Notify_Application,3,MUIM_CallHook,&EA_CloseHook,winnum);
         return data;
      }
      free(data);
   }
   return NULL;
}
///
