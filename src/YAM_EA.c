/***************************************************************************

 YAM - Yet Another Mailer
 Copyright (C) 1995-2000 Marcel Beck
 Copyright (C) 2000-2012 YAM Open Source Team

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

// define the STACKEXT symbol here to *disable* any stack checking
// which might possibly be initiated by the SDI headers. The m68k
// gcc2 seems to enforce this as soon as it discovers a recursion.
#define STACKEXT

#include <ctype.h>
#include <stdlib.h>
#include <string.h>

#include <clib/alib_protos.h>
#include <libraries/asl.h>
#include <libraries/iffparse.h>
#include <mui/NListview_mcc.h>
#include <mui/BetterString_mcc.h>
#include <proto/dos.h>
#include <proto/intuition.h>
#include <proto/locale.h>
#include <proto/muimaster.h>

#include "extrasrc.h"

#include "SDI_hook.h"

#include "YAM.h"
#include "YAM_addressbook.h"
#include "YAM_addressbookEntry.h"
#include "YAM_config.h"
#include "YAM_error.h"
#include "YAM_utilities.h"

#include "mui/ClassesExtra.h"
#include "mui/AddrBookEntryList.h"
#include "mui/Recipientstring.h"
#include "mui/UserPortraitGroup.h"

#include "Locale.h"
#include "Logfile.h"
#include "MUIObjects.h"
#include "Requesters.h"

#include "Debug.h"

/* local protos */
static int EA_Open(int);
static struct EA_ClassData *EA_New(int, int);

/***************************************************************************
 Module: Address book entry
***************************************************************************/

/*** Init & Open ***/
/// EA_Init
//  Creates and opens an address book entry window
int EA_Init(enum ABEntry_Type type, struct ABEntry *ab)
{
  int winnum = -1;
  const char *title = "";

  ENTER();

  if((winnum = EA_Open(type)) >= 0)
  {
    struct EA_ClassData *ea;

    ea = G->EA[winnum];
    ea->ABEntry = ab;

    switch (type)
    {
      case AET_USER:
        title = (ab != NULL) ? tr(MSG_EA_EditUser) : tr(MSG_AB_AddUser);
      break;

      case AET_LIST:
        title = (ab != NULL) ? tr(MSG_EA_EditList) : tr(MSG_AB_AddList);
      break;

      case AET_GROUP:
        title = (ab != NULL) ? tr(MSG_EA_EditGroup): tr(MSG_AB_AddGroup);
      break;
    }

    set(ea->GUI.WI, MUIA_Window_Title, title);
    if(SafeOpenWindow(ea->GUI.WI) == TRUE)
    {
      set(ea->GUI.WI, MUIA_Window_ActiveObject, ea->GUI.ST_ALIAS);
    }
    else
    {
      DisposeModulePush(&G->EA[winnum]);
      winnum = -1;
    }
  }

  RETURN(winnum);
  return winnum;
}

///
/// EA_Setup
//  Setup GUI fields with data from adress book entry
void EA_Setup(int winnum, struct ABEntry *ab)
{
  struct EA_GUIData *gui = &G->EA[winnum]->GUI;

  ENTER();

  switch (ab->Type)
  {
    case AET_USER:
    {
      char dateStr[SIZE_SMALL];

      AB_ExpandBD(ab->BirthDay, dateStr, sizeof(dateStr));

      setstring(gui->ST_ALIAS, ab->Alias);
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
      setstring(gui->ST_BIRTHDAY, dateStr);
      DoMethod(gui->GR_PHOTO, MUIM_UserPortraitGroup_SetPortrait, ab->Photo);
    }
    break;

    case AET_LIST:
    {
      char *ptr;

      setstring(gui->ST_ALIAS, ab->Alias);
      setstring(gui->ST_REALNAME, ab->RealName);
      setstring(gui->ST_ADDRESS, ab->Address);
      setstring(gui->ST_COMMENT, ab->Comment);
      DoMethod(gui->LV_MEMBER, MUIM_NList_Clear);
      for(ptr = ab->Members; *ptr != '\0'; ptr++)
      {
        char *nptr;

        if((nptr = strchr(ptr, '\n')) != NULL)
          *nptr = '\0';
        else
          break;
        DoMethod(gui->LV_MEMBER, MUIM_NList_InsertSingle, ptr, MUIV_NList_Insert_Bottom);
        *nptr = '\n';
        ptr = nptr;
      }
    }
    break;

    case AET_GROUP:
    {
      setstring(gui->ST_ALIAS, ab->Alias);
      setstring(gui->ST_COMMENT, ab->Comment);
    }
    break;
  }

  LEAVE();
}

///

/*** Private functions (member list) ***/
/// EA_AddSingleMember
//  Adds a single entry to the member list by Drag&Drop
void EA_AddSingleMember(Object *obj, struct MUI_NListtree_TreeNode *tn)
{
  struct ABEntry *ab = tn->tn_User;
  int dropmark = xget(obj, MUIA_List_DropMark);

  ENTER();

  DoMethod(obj, MUIM_List_InsertSingle, (ab->Alias != NULL) ? ab->Alias : ab->RealName, dropmark);

  LEAVE();
}

///
/// EA_AddMembers (rec)
//  Adds an entire group to the member list by Drag&Drop
void EA_AddMembers(Object *obj, struct MUI_NListtree_TreeNode *list)
{
  struct MUI_NListtree_TreeNode *tn;
  int i;

  ENTER();

  for(i=0; ; i++)
  {
    if((tn = (struct MUI_NListtree_TreeNode *)DoMethod(G->AB->GUI.LV_ADDRESSES, MUIM_NListtree_GetEntry, list, i, MUIV_NListtree_GetEntry_Flag_SameLevel)) != NULL)
    {
      if(isFlagSet(tn->tn_Flags, TNF_LIST))
        EA_AddMembers(obj, tn);
      else
        EA_AddSingleMember(obj, tn);
    }
    else
      break;
  }

  LEAVE();
}

///
/// EA_GetEntry
//  Fills string gadget with data from selected list entry
HOOKPROTONHNO(EA_GetEntry, void, int *arg)
{
  int winnum = *arg;
  char *entry = NULL;

  ENTER();

  DoMethod(G->EA[winnum]->GUI.LV_MEMBER, MUIM_NList_GetEntry, MUIV_NList_GetEntry_Active, &entry);
  if(entry != NULL)
    nnset(G->EA[winnum]->GUI.ST_MEMBER, MUIA_String_Contents, entry);

  LEAVE();
}
MakeStaticHook(EA_GetEntryHook, EA_GetEntry);

/*** EA_AddFunc - Adds a new entry to the member list ***/
HOOKPROTONHNO(EA_AddFunc, void, int *arg)
{
  struct EA_GUIData *gui = &G->EA[*arg]->GUI;
  char *buf = (char *)xget(gui->ST_MEMBER, MUIA_String_Contents);

  ENTER();

  if(buf[0] != '\0')
  {
    DoMethod(gui->LV_MEMBER, MUIM_NList_InsertSingle, buf, MUIV_NList_Insert_Bottom);
    nnset(gui->LV_MEMBER, MUIA_NList_Active, MUIV_NList_Active_Off);
    setstring(gui->ST_MEMBER, "");
  }
  set(gui->WI, MUIA_Window_ActiveObject, gui->ST_MEMBER);

  LEAVE();
}
MakeStaticHook(EA_AddHook, EA_AddFunc);

///
/// EA_PutEntry
//  Updates selected list entry
HOOKPROTONHNO(EA_PutEntry, void, int *arg)
{
  struct EA_GUIData *gui = &G->EA[*arg]->GUI;
  int active = xget(gui->LV_MEMBER, MUIA_NList_Active);

  ENTER();

  if(active == MUIV_List_Active_Off)
  {
    DoMethod(G->App, MUIM_CallHook, &EA_AddHook, *arg);
  }
  else
  {
    char *buf = (char *)xget(gui->ST_MEMBER, MUIA_String_Contents);

    DoMethod(gui->LV_MEMBER, MUIM_NList_InsertSingle, buf, active);
    DoMethod(gui->LV_MEMBER, MUIM_NList_Remove, active+1);
  }

  LEAVE();
}
MakeStaticHook(EA_PutEntryHook, EA_PutEntry);

///
/// EA_InsertBelowActive
//  Inserts an entry into the address book tree
void EA_InsertBelowActive(struct ABEntry *addr, int flags)
{
  Object *lt = G->AB->GUI.LV_ADDRESSES;
  struct MUI_NListtree_TreeNode *node;
  struct MUI_NListtree_TreeNode *list;

  ENTER();

  // get the active node
  node = (struct MUI_NListtree_TreeNode *)xget(lt, MUIA_NListtree_Active);
  if(node == MUIV_NListtree_Active_Off)
  {
    list = MUIV_NListtree_Insert_ListNode_Root;
    node = (struct MUI_NListtree_TreeNode *)MUIV_NListtree_Insert_PrevNode_Sorted;
  }
  else
  {
    list = (struct MUI_NListtree_TreeNode *)xget(lt, MUIA_NListtree_ActiveList);
  }

  // now we insert the node in the list accordingly and set it active automatically
  DoMethod(lt, MUIM_NListtree_Insert, addr->Alias, addr, list, node, (flags | MUIV_NListtree_Insert_Flag_Active));

  LEAVE();
}

///
/// EA_FixAlias
//  Avoids ambiguos aliases
void EA_FixAlias(struct ABEntry *ab, BOOL excludemyself)
{
  char alias[SIZE_NAME];
  int c = 1, l;
  struct ABEntry *ab_found = NULL;

  ENTER();

  strlcpy(alias, ab->Alias, sizeof(alias));

  while(AB_SearchEntry(alias, ASM_ALIAS|ASM_USER|ASM_LIST|ASM_GROUP, &ab_found) > 0)
  {
    if(excludemyself == TRUE && ab == ab_found)
    {
      LEAVE();
      return;
    }

    if((l = strlen(ab->Alias)) > SIZE_NAME-2)
      l = SIZE_NAME-2;

    snprintf(&alias[l], sizeof(alias)-l, "%d", ++c);
  }

  // copy the modified string back
  strlcpy(ab->Alias, alias, sizeof(ab->Alias));

  LEAVE();
}

///
/// EA_SetDefaultAlias
//  Creates an alias from the real name if user left it empty
void EA_SetDefaultAlias(struct ABEntry *ab)
{
  char *p = ab->Alias;

  ENTER();

  memset(p, 0, SIZE_NAME);
  if(ab->RealName[0] != '\0')
  {
    char *ln;

    if((ln = strrchr(ab->RealName, ' ')) != NULL)
    {
      if(isAlNum(ab->RealName[0]))
      {
        *p++ = ab->RealName[0];
        *p++ = '_';
      }
      ln++;
    }
    else
      ln = ab->RealName;

    for(; strlen(ab->Alias) < SIZE_NAME-2 && ln[0] != '\0'; ln++)
    {
      if(isAlNum(*ln))
        *p++ = *ln;
    }
  }
  else
  {
    char *ln;

    for(ln = ab->Address; strlen(ab->Alias) < SIZE_NAME-2 && *ln != '\0' && *ln != '@'; ln++)
    {
      if(isAlNum(*ln))
        *p++ = *ln;
    }
  }

  LEAVE();
}

///

/*** Buttons ***/
/// EA_Okay
//  Saves changes to the edited entry in the address book
HOOKPROTONHNO(EA_Okay, void, int *arg)
{
  static struct ABEntry newaddr;
  struct ABEntry *addr;
  char *members;
  int winnum = *arg;
  long bdate = 0;
  struct EA_GUIData *gui = &G->EA[winnum]->GUI;
  BOOL old = G->EA[winnum]->ABEntry != NULL;

  ENTER();

  memset(&newaddr, 0, sizeof(struct ABEntry));
  if(G->EA[winnum]->Type != 0)
  {
    char *str = (char *)xget(gui->ST_ALIAS, MUIA_String_Contents);

    if(str[0] == '\0')
    {
      ER_NewError(tr(MSG_ER_ErrorNoAlias));
      LEAVE();
      return;
    }
  }
  else
  {
    char *str = (char *)xget(gui->ST_ADDRESS, MUIA_String_Contents);

    if(str[0] == '\0')
    {
      ER_NewError(tr(MSG_ER_ErrorNoAddress));
      LEAVE();
      return;
    }

    str = (char *)xget(gui->ST_BIRTHDAY, MUIA_String_Contents);
    if(str[0] != '\0' && (bdate = AB_CompressBD(str)) == 0)
    {
      ER_NewError(tr(MSG_ER_WRONG_DOB_FORMAT), G->Locale != NULL ? G->Locale->loc_ShortDateFormat : (STRPTR)"%d.%m.%Y");
      LEAVE();
      return;
    }
  }

  G->AB->Modified = TRUE;
  if(old == TRUE)
    addr = G->EA[winnum]->ABEntry;
  else
    addr = &newaddr;

  GetMUIString(addr->Alias, gui->ST_ALIAS, sizeof(addr->Alias));
  GetMUIString(addr->Comment, gui->ST_COMMENT, sizeof(addr->Comment));

  addr->Type = G->EA[winnum]->Type;

  switch(addr->Type)
  {
    case AET_USER:
    {
      GetMUIString(addr->RealName, gui->ST_REALNAME, sizeof(addr->RealName));
      GetMUIString(addr->Address, gui->ST_ADDRESS, sizeof(addr->Address));
      GetMUIString(addr->Phone, gui->ST_PHONE, sizeof(addr->Phone));
      GetMUIString(addr->Street, gui->ST_STREET, sizeof(addr->Street));
      GetMUIString(addr->City, gui->ST_CITY, sizeof(addr->City));
      GetMUIString(addr->Country, gui->ST_COUNTRY, sizeof(addr->Country));
      GetMUIString(addr->PGPId, gui->ST_PGPKEY, sizeof(addr->PGPId));
      GetMUIString(addr->Homepage, gui->ST_HOMEPAGE, sizeof(addr->Homepage));

      // get the default security setting and check if
      // it is valid or not.
      addr->DefSecurity = GetMUICycle(gui->CY_DEFSECURITY);
      switch(addr->DefSecurity)
      {
        case SEC_SIGN:
        case SEC_ENCRYPT:
        case SEC_BOTH:
        {
          // check if PGP was found to be available at all
          // or warn the user accordingly.
          if(G->PGPVersion == 0)
          {
            if(MUI_Request(G->App, G->AB != NULL ? G->AB->GUI.WI : NULL, MUIF_NONE, 
                           tr(MSG_AB_INVALIDSECURITY_TITLE),
                           tr(MSG_AB_INVALIDSECURITY_GADS),
                           tr(MSG_AB_INVALIDSECURITY)) != 0)
            {
              addr->DefSecurity = SEC_NONE;
            }
          }
        }
        break;

        default:
          // nothing
        break;
      }

      strlcpy(addr->Photo, G->EA[winnum]->PhotoName, sizeof(addr->Photo));
      addr->BirthDay = bdate;

      if(addr->Alias[0] == '\0')
        EA_SetDefaultAlias(addr);

      EA_FixAlias(addr, old);
      if(old == FALSE)
        EA_InsertBelowActive(addr, 0);
    }
    break;

    case AET_LIST:
    {
      int i;

      GetMUIString(addr->RealName, gui->ST_REALNAME, sizeof(addr->RealName));
      GetMUIString(addr->Address, gui->ST_ADDRESS, sizeof(addr->Address));
      members = AllocStrBuf(SIZE_DEFAULT);
      for(i = 0; ; i++)
      {
        char *p;

        DoMethod(gui->LV_MEMBER, MUIM_NList_GetEntry, i, &p);
        if(p == NULL)
          break;

        StrBufCat(&members, p);
        StrBufCat(&members, "\n");
      }

      if(old == TRUE)
      {
        if((addr->Members = realloc(addr->Members, strlen(members) + 1)) != NULL)
          memcpy(addr->Members, members, strlen(members) + 1);
      }
      else
        addr->Members = strdup(members);

      EA_FixAlias(addr, old);
      if(old == FALSE)
      {
        EA_InsertBelowActive(addr, 0);
        free(addr->Members);
      }

      FreeStrBuf(members);
    }
    break;

    case AET_GROUP:
    {
      EA_FixAlias(addr, old);
      if(old == FALSE)
        EA_InsertBelowActive(addr, TNF_LIST);
    }
    break;
  }

  set(gui->WI, MUIA_Window_Open, FALSE);

  if(old == TRUE)
    DoMethod(G->AB->GUI.LV_ADDRESSES, MUIM_List_Redraw, MUIV_List_Redraw_All);
  else
    AppendToLogfile(LF_VERBOSE, 71, tr(MSG_LOG_NewAddress), addr->Alias);

  DisposeModulePush(&G->EA[winnum]);

  LEAVE();
}
MakeStaticHook(EA_OkayHook, EA_Okay);

///
/// EA_HomepageFunc
//  Launches a browser to view the homepage of the person
HOOKPROTONHNO(EA_HomepageFunc, void, int *arg)
{
  char *url;

  ENTER();

  url = (char *)xget(G->EA[*arg]->GUI.ST_HOMEPAGE, MUIA_String_Contents);
  if(url != NULL && url[0] != '\0')
    GotoURL(url, FALSE);

  LEAVE();
}
MakeStaticHook(EA_HomepageHook, EA_HomepageFunc);

///
/// EA_Open
//  Assigns a number for a new window
static int EA_Open(int type)
{
  int winnum;

  ENTER();

  for(winnum = 0; winnum < 4; winnum++)
  {
    if(G->EA[winnum] == NULL)
      break;
  }

  if(winnum == 4)
    winnum = -1;
  else
  {
    if((G->EA[winnum] = EA_New(winnum, type)) == NULL)
      winnum = -1;
  }

  RETURN(winnum);
  return winnum;
}
///
/// EA_CloseFunc
//  Closes address book entry window
HOOKPROTONHNO(EA_CloseFunc, void, int *arg)
{
  int winnum = *arg;
  struct EA_GUIData *gui = &(G->EA[winnum]->GUI);

  ENTER();

  // update the user image ID and remove it from the cache
  // it will be reloaded when necessary
  DoMethod(gui->GR_PHOTO, MUIM_UserPortraitGroup_Clear);

  DisposeModulePush(&G->EA[winnum]);

  LEAVE();
}
MakeStaticHook(EA_CloseHook, EA_CloseFunc);

///

/*** GUI ***/
/// EA_New
//  Creates address book entry window
static struct EA_ClassData *EA_New(int winnum, int type)
{
  struct EA_ClassData *data;

  ENTER();

  if((data = calloc(1, sizeof(struct EA_ClassData))) != NULL)
  {
    Object *group = NULL;
    Object *bt_homepage;
    Object *bt_sort;

    data->Type = type;
    switch(type)
    {
      case AET_USER:
      {
        static const char *SecurityCycleEntries[5];

        SecurityCycleEntries[0] = tr(MSG_WR_SecNone);
        SecurityCycleEntries[1] = tr(MSG_WR_SecSign);
        SecurityCycleEntries[2] = tr(MSG_WR_SecEncrypt);
        SecurityCycleEntries[3] = tr(MSG_WR_SecBoth);
        SecurityCycleEntries[4] = NULL;

        /* build MUI object tree */
        group = HGroup,
           MUIA_Group_SameWidth, TRUE,
           Child, VGroup,
              Child, ColGroup(2), GroupFrameT(tr(MSG_EA_ElectronicMail)),
                 Child, Label2(tr(MSG_EA_Alias)),
                 Child, data->GUI.ST_ALIAS = MakeString(SIZE_NAME,tr(MSG_EA_Alias)),
                 Child, Label2(tr(MSG_EA_RealName)),
                 Child, data->GUI.ST_REALNAME = MakeString(SIZE_REALNAME,tr(MSG_EA_RealName)),
                 Child, Label2(tr(MSG_EA_EmailAddress)),
                 Child, data->GUI.ST_ADDRESS  = MakeString(SIZE_ADDRESS,tr(MSG_EA_EmailAddress)),
                 Child, Label2(tr(MSG_EA_PGPId)),
                 Child, MakePGPKeyList(&(data->GUI.ST_PGPKEY), FALSE, tr(MSG_EA_PGPId)),
                 Child, Label2(tr(MSG_EA_Homepage)),
                 Child, HGroup,
                    MUIA_Group_HorizSpacing, 1,
                    Child, data->GUI.ST_HOMEPAGE = MakeString(SIZE_URL,tr(MSG_EA_Homepage)),
                    Child, bt_homepage = PopButton(MUII_TapeRecord),
                 End,
                 Child, Label2(tr(MSG_EA_DefSecurity)),
                 Child, data->GUI.CY_DEFSECURITY = CycleObject,
                    MUIA_Cycle_Entries, SecurityCycleEntries,
                    MUIA_ControlChar, ShortCut(tr(MSG_EA_DefSecurity)),
                 End,
              End,
              Child, ColGroup(2), GroupFrameT(tr(MSG_EA_SnailMail)),
                 Child, Label2(tr(MSG_EA_Street)),
                 Child, data->GUI.ST_STREET = MakeString(SIZE_DEFAULT,tr(MSG_EA_Street)),
                 Child, Label2(tr(MSG_EA_City)),
                 Child, data->GUI.ST_CITY = MakeString(SIZE_DEFAULT,tr(MSG_EA_City)),
                 Child, Label2(tr(MSG_EA_Country)),
                 Child, data->GUI.ST_COUNTRY = MakeString(SIZE_DEFAULT,tr(MSG_EA_Country)),
                 Child, Label2(tr(MSG_EA_Phone)),
                 Child, data->GUI.ST_PHONE = MakeString(SIZE_DEFAULT,tr(MSG_EA_Phone)),
              End,
           End,
           Child, VGroup,
              Child, ColGroup(2), GroupFrameT(tr(MSG_EA_Miscellaneous)),
                 Child, Label2(tr(MSG_EA_Description)),
                 Child, data->GUI.ST_COMMENT = MakeString(SIZE_DEFAULT,tr(MSG_EA_Description)),
                 Child, Label2(tr(MSG_EA_DOB)),
                 Child, data->GUI.ST_BIRTHDAY = MakeString(SIZE_SMALL,tr(MSG_EA_DOB)),
              End,
              Child, data->GUI.GR_PHOTO = UserPortraitGroupObject,
                 MUIA_UserPortraitGroup_WindowNumber, winnum,
              End,
           End,
        End;
        if(group != NULL)
        {
          DoMethod(group, MUIM_MultiSet, MUIA_String_Reject, ",", data->GUI.ST_ALIAS,
                                                                  data->GUI.ST_ADDRESS,
                                                                  NULL);
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

          // when a key ID is selected, set default security to "encrypt"
          DoMethod(data->GUI.ST_PGPKEY, MUIM_Notify, MUIA_String_Contents, MUIV_EveryTime, data->GUI.CY_DEFSECURITY, 3, MUIM_Set, MUIA_Cycle_Active, 2);

          DoMethod(bt_homepage, MUIM_Notify, MUIA_Pressed, FALSE, MUIV_Notify_Application, 3, MUIM_CallHook, &EA_HomepageHook, winnum);
        }
      }
      break;

      case AET_GROUP:
      {
        group = ColGroup(2), GroupFrame,
           MUIA_Background, MUII_GroupBack,
           Child, Label2(tr(MSG_EA_Alias)),
           Child, data->GUI.ST_ALIAS = MakeString(SIZE_NAME,tr(MSG_EA_Alias)),
           Child, Label2(tr(MSG_EA_Description)),
           Child, data->GUI.ST_COMMENT = MakeString(SIZE_DEFAULT,tr(MSG_EA_Description)),
        End;
        set(data->GUI.ST_ALIAS, MUIA_String_Reject, ",");
      }
      break;

      case AET_LIST:
      {
        group = HGroup,
           MUIA_Group_SameWidth, TRUE,
           Child, VGroup,
              Child, ColGroup(2), GroupFrameT(tr(MSG_EA_ElectronicMail)),
                 MUIA_Background, MUII_GroupBack,
                 Child, Label2(tr(MSG_EA_Alias)),
                 Child, data->GUI.ST_ALIAS = MakeString(SIZE_NAME,tr(MSG_EA_Alias)),
                 Child, Label2(tr(MSG_EA_ReturnAddress)),
                 Child, MakeAddressField(&data->GUI.ST_ADDRESS, tr(MSG_EA_ReturnAddress), MSG_HELP_EA_ST_ADDRESS_L, ABM_CONFIG, -1, MUIF_NONE),
                 Child, Label2(tr(MSG_EA_MLName)),
                 Child, data->GUI.ST_REALNAME = MakeString(SIZE_REALNAME,tr(MSG_EA_MLName)),
                 Child, Label2(tr(MSG_EA_Description)),
                 Child, data->GUI.ST_COMMENT = MakeString(SIZE_DEFAULT,tr(MSG_EA_Description)),
              End,
              Child, VSpace(0),
           End,
           Child, VGroup, GroupFrameT(tr(MSG_EA_Members)),
              Child, NListviewObject,
                 MUIA_CycleChain, 1,
                 MUIA_Listview_DragType, MUIV_Listview_DragType_Immediate,
                 MUIA_NListview_NList, data->GUI.LV_MEMBER = AddrBookEntryListObject,
                    InputListFrame,
                    MUIA_NList_DragSortable,  TRUE,
                    MUIA_NList_ConstructHook, MUIV_NList_ConstructHook_String,
                    MUIA_NList_DestructHook,  MUIV_NList_DestructHook_String,
                 End,
              End,
              Child, data->GUI.ST_MEMBER = RecipientstringObject,
                 MUIA_CycleChain,               TRUE,
                 MUIA_String_MaxLen,            SIZE_ADDRESS,
                 MUIA_BetterString_NoShortcuts, FALSE,
              End,
              Child, HGroup,
                 Child, ColGroup(2),
                    MUIA_Group_Spacing, 1,
                    MUIA_Group_SameWidth, TRUE,
                    MUIA_Weight, 1,
                    Child, data->GUI.BT_ADD = MakeButton(MUIX_B "+" MUIX_N),
                    Child, data->GUI.BT_DEL = MakeButton(MUIX_B "-" MUIX_N),
                 End,
                 Child, HSpace(0),
                 Child, bt_sort = MakeButton(tr(MSG_EA_Sort)),
              End,
           End,
        End;
        if(group != NULL)
        {
          DoMethod(group, MUIM_MultiSet, MUIA_String_Reject, ",<>", data->GUI.ST_ALIAS, data->GUI.ST_ADDRESS, data->GUI.ST_REALNAME, NULL);
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
          DoMethod(data->GUI.BT_DEL   ,MUIM_Notify,MUIA_Pressed            ,FALSE         ,data->GUI.LV_MEMBER,2,MUIM_NList_Remove,MUIV_NList_Remove_Active);
          DoMethod(bt_sort            ,MUIM_Notify,MUIA_Pressed            ,FALSE         ,data->GUI.LV_MEMBER,1,MUIM_NList_Sort);
          DoMethod(data->GUI.ST_MEMBER,MUIM_Notify,MUIA_String_Acknowledge ,MUIV_EveryTime,MUIV_Notify_Application,3,MUIM_CallHook   ,&EA_PutEntryHook,winnum);
          DoMethod(data->GUI.LV_MEMBER,MUIM_Notify,MUIA_NList_Active       ,MUIV_EveryTime,MUIV_Notify_Application,3,MUIM_CallHook   ,&EA_GetEntryHook,winnum);
        }
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
             Child, data->GUI.BT_OKAY   = MakeButton(tr(MSG_Okay)),
             Child, HSpace(0),
             Child, data->GUI.BT_CANCEL = MakeButton(tr(MSG_Cancel)),
          End,
       End,
    End;

    if(data->GUI.WI != NULL)
    {
      DoMethod(G->App, OM_ADDMEMBER, data->GUI.WI);
      SetHelp(data->GUI.ST_ALIAS   ,MSG_HELP_EA_ST_ALIAS      );
      SetHelp(data->GUI.ST_COMMENT ,MSG_HELP_EA_ST_DESCRIPTION);
      SetHelp(data->GUI.BT_OKAY    ,MSG_HELP_EA_BT_OKAY       );
      SetHelp(data->GUI.BT_CANCEL  ,MSG_HELP_EA_BT_CANCEL     );
      set(data->GUI.ST_BIRTHDAY, MUIA_String_Accept, "0123456789.-/");
      DoMethod(data->GUI.BT_CANCEL,MUIM_Notify,MUIA_Pressed            ,FALSE         ,MUIV_Notify_Application,3,MUIM_CallHook,&EA_CloseHook,winnum);
      DoMethod(data->GUI.BT_OKAY  ,MUIM_Notify,MUIA_Pressed            ,FALSE         ,MUIV_Notify_Application,3,MUIM_CallHook,&EA_OkayHook,winnum);
      DoMethod(data->GUI.WI       ,MUIM_Notify,MUIA_Window_CloseRequest,TRUE          ,MUIV_Notify_Application,3,MUIM_CallHook,&EA_CloseHook,winnum);
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
