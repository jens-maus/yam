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

 Superclass:  MUIC_Window
 Description: Window to carry all the address book GUI elements

***************************************************************************/

#include "AddressBookWindow_cl.h"

#include <string.h>
#include <proto/dos.h>
#include <proto/expat.h>
#include <proto/muimaster.h>
#include <proto/timer.h>
#include <libraries/gadtools.h>
#include <libraries/iffparse.h>
#include <mui/NList_mcc.h>
#include <mui/NListtree_mcc.h>
#include <mui/NListview_mcc.h>
#include <mui/TheBar_mcc.h>

#include "SDI_hook.h"

#include "YAM.h"
#include "YAM_addressbook.h"
#include "YAM_addressbookEntry.h"

#include "mui/AddrBookListtree.h"
#include "mui/AddrBookToolbar.h"
#include "mui/BirthdayRequestWindow.h"
#include "mui/RecipientString.h"
#include "mui/WriteWindow.h"

#include "Busy.h"
#include "Config.h"
#include "FileInfo.h"
#include "Locale.h"
#include "MUIObjects.h"
#include "Requesters.h"

#include "Debug.h"

/* CLASSDATA
struct Data
{
  Object *BT_TO;
  Object *BT_CC;
  Object *BT_BCC;
  Object *TB_TOOLBAR;
  Object *LV_ADDRESSES;

  enum AddressbookMode mode;
  LONG windowNumber;
  Object *recipientObject;

  char windowTitle[SIZE_DEFAULT];
  char screenTitle[SIZE_DEFAULT];
};
*/

/* INCLUDE
#include "YAM_addressbook.h"
*/

/* Overloaded Methods */
/// OVERLOAD(OM_NEW)
OVERLOAD(OM_NEW)
{
  Object *BT_TO;
  Object *BT_CC;
  Object *BT_BCC;
  Object *TB_TOOLBAR = NULL;
  Object *LV_ADDRESSES;

  enum {
    AMEN_NEW,AMEN_LOAD,AMEN_APPEND,AMEN_SAVE,AMEN_SAVEAS,
    AMEN_IMPORT_LDIF, AMEN_IMPORT_CSV, AMEN_IMPORT_TAB, AMEN_IMPORT_XML,
    AMEN_EXPORT_LDIF, AMEN_EXPORT_CSV, AMEN_EXPORT_TAB,
    AMEN_PRINTA,
    AMEN_FIND,AMEN_NEWUSER,AMEN_NEWLIST,AMEN_NEWGROUP,AMEN_EDIT,
    AMEN_DUPLICATE,AMEN_DELETE,AMEN_PRINTE,AMEN_SORTALIAS,
    AMEN_SORTLNAME,AMEN_SORTFNAME,AMEN_SORTDESC,AMEN_SORTADDR,
    AMEN_FOLD,AMEN_UNFOLD
  };

  ENTER();

  if((obj = DoSuperNew(cl, obj,

    MUIA_HelpNode, "Windows#Addressbook",
    MUIA_Window_Menustrip, MenustripObject,
      MenuChild, MenuObject,
        MUIA_Menu_Title, tr(MSG_CO_CrdABook),
        MUIA_Menu_CopyStrings, FALSE,
        MenuChild, Menuitem(tr(MSG_New), "N", TRUE, FALSE, AMEN_NEW),
        MenuChild, Menuitem(tr(MSG_Open), "O", TRUE, FALSE, AMEN_LOAD),
        MenuChild, Menuitem(tr(MSG_Append), "I", TRUE, FALSE, AMEN_APPEND),
        MenuChild, MenuBarLabel,
        MenuChild, MenuitemObject,
          MUIA_Menuitem_Title, tr(MSG_AB_IMPORT),
          MUIA_Menuitem_CopyStrings, FALSE,
          MenuChild, Menuitem(tr(MSG_AB_LDIF), NULL, TRUE, FALSE, AMEN_IMPORT_LDIF),
          MenuChild, Menuitem(tr(MSG_AB_CSV), NULL, TRUE, FALSE, AMEN_IMPORT_CSV),
          MenuChild, Menuitem(tr(MSG_AB_TAB), NULL, TRUE, FALSE, AMEN_IMPORT_TAB),
          MenuChild, Menuitem(tr(MSG_AB_XML), NULL, ExpatBase != NULL, FALSE, AMEN_IMPORT_XML),
        End,
        MenuChild, MenuitemObject,
          MUIA_Menuitem_Title, tr(MSG_AB_EXPORT),
          MUIA_Menuitem_CopyStrings, FALSE,
          MenuChild, Menuitem(tr(MSG_AB_LDIF), NULL, TRUE, FALSE, AMEN_EXPORT_LDIF),
          MenuChild, Menuitem(tr(MSG_AB_CSV), NULL, TRUE, FALSE, AMEN_EXPORT_CSV),
          MenuChild, Menuitem(tr(MSG_AB_TAB), NULL, TRUE, FALSE, AMEN_EXPORT_TAB),
        End,
        MenuChild, MenuBarLabel,
        MenuChild, Menuitem(tr(MSG_Save), "S", TRUE, FALSE, AMEN_SAVE),
        MenuChild, Menuitem(tr(MSG_SaveAs), "A", TRUE, FALSE, AMEN_SAVEAS),
        MenuChild, MenuBarLabel,
        MenuChild, Menuitem(tr(MSG_AB_MIFind), "F", TRUE, FALSE, AMEN_FIND),
        MenuChild, Menuitem(tr(MSG_Print), NULL, TRUE, FALSE,AMEN_PRINTA),
      End,
      MenuChild, MenuObject,
        MUIA_Menu_Title, tr(MSG_AB_Entry),
        MUIA_Menu_CopyStrings, FALSE,
        MenuChild, Menuitem(tr(MSG_AB_AddUser), "P", TRUE, FALSE, AMEN_NEWUSER),
        MenuChild, Menuitem(tr(MSG_AB_AddList), "L", TRUE, FALSE, AMEN_NEWLIST),
        MenuChild, Menuitem(tr(MSG_AB_AddGroup), "G", TRUE, FALSE, AMEN_NEWGROUP),
        MenuChild, MenuBarLabel,
        MenuChild, Menuitem(tr(MSG_Edit), "E", TRUE, FALSE, AMEN_EDIT),
        MenuChild, Menuitem(tr(MSG_AB_Duplicate), "D", TRUE, FALSE, AMEN_DUPLICATE),
        MenuChild, Menuitem(tr(MSG_AB_MIDelete), "Del", TRUE, TRUE, AMEN_DELETE),
        MenuChild, MenuBarLabel,
        MenuChild, Menuitem(tr(MSG_AB_MIPrint), NULL, TRUE, FALSE, AMEN_PRINTE),
      End,
      MenuChild, MenuObject,
        MUIA_Menu_Title, tr(MSG_AB_Sort),
        MUIA_Menu_CopyStrings, FALSE,
        MenuChild, Menuitem(tr(MSG_AB_SortByAlias), "1", TRUE, FALSE, AMEN_SORTALIAS),
        MenuChild, Menuitem(tr(MSG_AB_SortByName), "2", TRUE, FALSE, AMEN_SORTLNAME),
        MenuChild, Menuitem(tr(MSG_AB_SortByFirstname), "3", TRUE, FALSE, AMEN_SORTFNAME),
        MenuChild, Menuitem(tr(MSG_AB_SortByDesc), "4", TRUE, FALSE, AMEN_SORTDESC),
        MenuChild, Menuitem(tr(MSG_AB_SortByAddress), "5", TRUE, FALSE, AMEN_SORTADDR),
      End,
      MenuChild, MenuObject,
        MUIA_Menu_Title, tr(MSG_AB_View),
        MUIA_Menu_CopyStrings, FALSE,
        MenuChild, Menuitem(tr(MSG_AB_Unfold), "<", TRUE, FALSE, AMEN_UNFOLD),
        MenuChild, Menuitem(tr(MSG_AB_Fold), ">", TRUE, FALSE, AMEN_FOLD),
      End,
    End,
    MUIA_Window_ID,MAKE_ID('B','O','O','K'),
    WindowContents, VGroup,
      Child, hasHideToolBarFlag(C->HideGUIElements) ?
        (HGroup,
          MUIA_HelpNode, "Windows#AddressbookToolbar",
          Child, BT_TO  = MakeButton("_To:"),
          Child, BT_CC  = MakeButton("_CC:"),
          Child, BT_BCC = MakeButton("_BCC:"),
        End) :
        (HGroup, GroupSpacing(0),
          MUIA_HelpNode, "Windows#AddressbookToolbar",
          Child, VGroup,
            MUIA_Weight, 10,
            MUIA_Group_VertSpacing, 0,
            Child, BT_TO  = MakeButton("_To:"),
            Child, BT_CC  = MakeButton("_CC:"),
            Child, BT_BCC = MakeButton("_BCC:"),
            Child, HVSpace,
          End,
          Child, MUI_MakeObject(MUIO_VBar, 12),
          Child, HGroupV,
            Child, TB_TOOLBAR = AddrBookToolbarObject,
            End,
          End,
        End),
      Child, NListviewObject,
        MUIA_CycleChain,        TRUE,
        MUIA_Listview_DragType, MUIV_Listview_DragType_Immediate,
        MUIA_NListview_NList,   LV_ADDRESSES = AddrBookListtreeObject,
        End,
      End,
    End,

    TAG_MORE, inittags(msg))) != NULL)
  {
    GETDATA;

    DoMethod(G->App, OM_ADDMEMBER, obj);

    data->BT_TO =        BT_TO;
    data->BT_CC =        BT_CC;
    data->BT_BCC =       BT_BCC;
    data->TB_TOOLBAR =   TB_TOOLBAR;
    data->LV_ADDRESSES = LV_ADDRESSES;

    set(obj, MUIA_Window_DefaultObject, LV_ADDRESSES);
    DoMethod(obj, MUIM_MultiSet, MUIA_Disabled, TRUE,
      BT_TO,
      BT_CC,
      BT_BCC,
      NULL);

    SetHelp(BT_TO,  MSG_HELP_AB_BT_TO);
    SetHelp(BT_CC,  MSG_HELP_AB_BT_CC);
    SetHelp(BT_BCC, MSG_HELP_AB_BT_BCC);

    DoMethod(obj,          MUIM_Notify, MUIA_Window_MenuAction,     AMEN_NEW,         LV_ADDRESSES, 1, MUIM_AddrBookListtree_ClearTree);
    DoMethod(obj,          MUIM_Notify, MUIA_Window_MenuAction,     AMEN_LOAD,        obj,          1, METHOD(Load));
    DoMethod(obj,          MUIM_Notify, MUIA_Window_MenuAction,     AMEN_APPEND,      obj,          1, METHOD(Append));
    DoMethod(obj,          MUIM_Notify, MUIA_Window_MenuAction,     AMEN_IMPORT_LDIF, obj,          1, METHOD(ImportLDIF));
    DoMethod(obj,          MUIM_Notify, MUIA_Window_MenuAction,     AMEN_IMPORT_TAB,  obj,          2, METHOD(ImportTabCSV), '\t');
    DoMethod(obj,          MUIM_Notify, MUIA_Window_MenuAction,     AMEN_IMPORT_CSV,  obj,          2, METHOD(ImportTabCSV), ',');
    DoMethod(obj,          MUIM_Notify, MUIA_Window_MenuAction,     AMEN_IMPORT_XML,  obj,          1, METHOD(ImportXML));
    DoMethod(obj,          MUIM_Notify, MUIA_Window_MenuAction,     AMEN_EXPORT_LDIF, obj,          1, METHOD(ExportLDIF));
    DoMethod(obj,          MUIM_Notify, MUIA_Window_MenuAction,     AMEN_EXPORT_TAB,  obj,          2, METHOD(ExportTabCSV), '\t');
    DoMethod(obj,          MUIM_Notify, MUIA_Window_MenuAction,     AMEN_EXPORT_CSV,  obj,          2, METHOD(ExportTabCSV), ',');
    DoMethod(obj,          MUIM_Notify, MUIA_Window_MenuAction,     AMEN_SAVE,        obj,          1, METHOD(Save));
    DoMethod(obj,          MUIM_Notify, MUIA_Window_MenuAction,     AMEN_SAVEAS,      obj,          1, METHOD(SaveAs));
    DoMethod(obj,          MUIM_Notify, MUIA_Window_MenuAction,     AMEN_PRINTA,      obj,          1, METHOD(PrintAll));
    DoMethod(obj,          MUIM_Notify, MUIA_Window_MenuAction,     AMEN_NEWUSER,     LV_ADDRESSES, 2, MUIM_AddrBookListtree_AddEntry, AET_USER);
    DoMethod(obj,          MUIM_Notify, MUIA_Window_MenuAction,     AMEN_NEWLIST,     LV_ADDRESSES, 2, MUIM_AddrBookListtree_AddEntry, AET_LIST);
    DoMethod(obj,          MUIM_Notify, MUIA_Window_MenuAction,     AMEN_NEWGROUP,    LV_ADDRESSES, 2, MUIM_AddrBookListtree_AddEntry, AET_GROUP);
    DoMethod(obj,          MUIM_Notify, MUIA_Window_MenuAction,     AMEN_EDIT,        LV_ADDRESSES, 1, MUIM_AddrBookListtree_EditEntry);
    DoMethod(obj,          MUIM_Notify, MUIA_Window_MenuAction,     AMEN_DUPLICATE,   LV_ADDRESSES, 1, MUIM_AddrBookListtree_DuplicateEntry);
    DoMethod(obj,          MUIM_Notify, MUIA_Window_MenuAction,     AMEN_DELETE,      LV_ADDRESSES, 1, MUIM_AddrBookListtree_DeleteEntry);
    DoMethod(obj,          MUIM_Notify, MUIA_Window_MenuAction,     AMEN_PRINTE,      obj,          1, METHOD(PrintEntry));
    DoMethod(obj,          MUIM_Notify, MUIA_Window_MenuAction,     AMEN_FIND,        LV_ADDRESSES, 1, MUIM_AddrBookListtree_FindEntry);
    DoMethod(obj,          MUIM_Notify, MUIA_Window_MenuAction,     AMEN_SORTALIAS,   LV_ADDRESSES, 2, MUIM_AddrBookListtree_SortBy, MUIV_AddrBookListtree_SortBy_Alias);
    DoMethod(obj,          MUIM_Notify, MUIA_Window_MenuAction,     AMEN_SORTLNAME,   LV_ADDRESSES, 2, MUIM_AddrBookListtree_SortBy, MUIV_AddrBookListtree_SortBy_LastName);
    DoMethod(obj,          MUIM_Notify, MUIA_Window_MenuAction,     AMEN_SORTFNAME,   LV_ADDRESSES, 2, MUIM_AddrBookListtree_SortBy, MUIV_AddrBookListtree_SortBy_FirstName);
    DoMethod(obj,          MUIM_Notify, MUIA_Window_MenuAction,     AMEN_SORTDESC,    LV_ADDRESSES, 2, MUIM_AddrBookListtree_SortBy, MUIV_AddrBookListtree_SortBy_Coment);
    DoMethod(obj,          MUIM_Notify, MUIA_Window_MenuAction,     AMEN_SORTADDR,    LV_ADDRESSES, 2, MUIM_AddrBookListtree_SortBy, MUIV_AddrBookListtree_SortBy_Address);
    DoMethod(obj,          MUIM_Notify, MUIA_Window_MenuAction,     AMEN_FOLD,        LV_ADDRESSES, 2, MUIM_AddrBookListtree_FoldTree, FALSE);
    DoMethod(obj,          MUIM_Notify, MUIA_Window_MenuAction,     AMEN_UNFOLD,      LV_ADDRESSES, 2, MUIM_AddrBookListtree_FoldTree, TRUE);
    DoMethod(LV_ADDRESSES, MUIM_Notify, MUIA_NListtree_Active,      MUIV_EveryTime,   obj,          1, METHOD(ActiveChange));
    DoMethod(LV_ADDRESSES, MUIM_Notify, MUIA_NListtree_DoubleClick, MUIV_EveryTime,   obj,          1, METHOD(HandleDoubleClick));
    DoMethod(BT_TO,        MUIM_Notify, MUIA_Pressed,               FALSE,            obj,          1, METHOD(UseEntry), ABM_TO);
    DoMethod(BT_CC,        MUIM_Notify, MUIA_Pressed,               FALSE,            obj,          1, METHOD(UseEntry), ABM_CC);
    DoMethod(BT_BCC,       MUIM_Notify, MUIA_Pressed,               FALSE,            obj,          1, METHOD(UseEntry), ABM_BCC);
    DoMethod(obj,          MUIM_Notify, MUIA_Window_CloseRequest,   TRUE,             obj,          1, METHOD(Close));

    if(TB_TOOLBAR != NULL)
    {
      DoMethod(TB_TOOLBAR, MUIM_TheBar_Notify, TB_ABOOK_SAVE,      MUIA_Pressed, FALSE, obj,          1, METHOD(Save));
      DoMethod(TB_TOOLBAR, MUIM_TheBar_Notify, TB_ABOOK_FIND,      MUIA_Pressed, FALSE, LV_ADDRESSES, 1, MUIM_AddrBookListtree_FindEntry);
      DoMethod(TB_TOOLBAR, MUIM_TheBar_Notify, TB_ABOOK_NEWUSER,   MUIA_Pressed, FALSE, LV_ADDRESSES, 2, MUIM_AddrBookListtree_AddEntry, AET_USER);
      DoMethod(TB_TOOLBAR, MUIM_TheBar_Notify, TB_ABOOK_NEWLIST,   MUIA_Pressed, FALSE, LV_ADDRESSES, 2, MUIM_AddrBookListtree_AddEntry, AET_LIST);
      DoMethod(TB_TOOLBAR, MUIM_TheBar_Notify, TB_ABOOK_NEWGROUP,  MUIA_Pressed, FALSE, LV_ADDRESSES, 2, MUIM_AddrBookListtree_AddEntry, AET_GROUP);
      DoMethod(TB_TOOLBAR, MUIM_TheBar_Notify, TB_ABOOK_EDIT,      MUIA_Pressed, FALSE, LV_ADDRESSES, 1, MUIM_AddrBookListtree_EditEntry);
      DoMethod(TB_TOOLBAR, MUIM_TheBar_Notify, TB_ABOOK_DELETE,    MUIA_Pressed, FALSE, LV_ADDRESSES, 1, MUIM_AddrBookListtree_DeleteEntry);
      DoMethod(TB_TOOLBAR, MUIM_TheBar_Notify, TB_ABOOK_PRINT,     MUIA_Pressed, FALSE, obj,          1, METHOD(PrintAll));
      DoMethod(TB_TOOLBAR, MUIM_TheBar_Notify, TB_ABOOK_OPENTREE,  MUIA_Pressed, FALSE, LV_ADDRESSES, 2, MUIM_AddrBookListtree_FoldTree, TRUE);
      DoMethod(TB_TOOLBAR, MUIM_TheBar_Notify, TB_ABOOK_CLOSETREE, MUIA_Pressed, FALSE, LV_ADDRESSES, 2, MUIM_AddrBookListtree_FoldTree, FALSE);
    }
  }

  RETURN((IPTR)obj);
  return (IPTR)obj;
}

///
/// OVERLOAD(OM_GET)
OVERLOAD(OM_GET)
{
  GETDATA;
  IPTR *store = ((struct opGet *)msg)->opg_Storage;

  switch(((struct opGet *)msg)->opg_AttrID)
  {
    //case ATTR(xxx): *store = xxx; return TRUE;
  }

  return DoSuperMethodA(cl, obj, msg);
}

///

/* Private Functions */

/* Public Methods */
/// DECLARE(Open)
// open address book window
DECLARE(Open) // enum AddressbookMode mode, LONG windowNumber, Object *recipientObj
{
  GETDATA;
  const char *md;
  BOOL nodeActive;

  ENTER();

  switch(msg->mode)
  {
    case ABM_FROM:    md = "(From)";     break;
    case ABM_TO:      md = "(To)";       break;
    case ABM_CC:      md = "(CC)";       break;
    case ABM_BCC:     md = "(BCC)";      break;
    case ABM_REPLYTO: md = "(Reply-To)"; break;
    case ABM_CONFIG:  md = "";           break;
    default:          md = "";           break;
  }

  data->mode =msg-> mode;
  data->windowNumber = (*md != '\0' ? msg->windowNumber : -1),
  set(data->LV_ADDRESSES, MUIA_AddrBookListtree_Modified, FALSE);

  // enable/disable the To/CC/BCC buttons depending on whether there is an active entry or not
  nodeActive = ((struct MUI_NListtree_TreeNode *)xget(data->LV_ADDRESSES, MUIA_NListtree_Active) != NULL);
  DoMethod(obj, MUIM_MultiSet, MUIA_Disabled, nodeActive == FALSE,
    data->BT_TO,
    data->BT_CC,
    data->BT_BCC,
    NULL);

  if(data->TB_TOOLBAR != NULL)
    DoMethod(data->TB_TOOLBAR, MUIM_AddrBookToolbar_UpdateControls);


  snprintf(data->windowTitle, sizeof(data->windowTitle), "%s %s", tr(MSG_MA_MAddrBook), md);

  xset(obj,
    MUIA_Window_Title, data->windowTitle,
    MUIA_Window_ScreenTitle, CreateScreenTitle(data->screenTitle, sizeof(data->screenTitle), data->windowTitle));

  SafeOpenWindow(obj);

  RETURN(0);
  return 0;
}

///
/// DECLARE(Load)
// load an address book
DECLARE(Load)
{
  GETDATA;
  struct FileReqCache *frc;

  ENTER();

  if((frc = ReqFile(ASL_ABOOK, obj, tr(MSG_Open), REQF_NONE, G->MA_MailDir, "")) != NULL)
  {
    AddPath(G->AB_Filename, frc->drawer, frc->file, sizeof(G->AB_Filename));
    AB_LoadTree(data->LV_ADDRESSES, G->AB_Filename, FALSE, FALSE);
  }

  RETURN(0);
  return 0;
}

///
/// DECLARE(Append)
// append an address book
DECLARE(Append)
{
  GETDATA;
  struct FileReqCache *frc;

  ENTER();

  if((frc = ReqFile(ASL_ABOOK, obj, tr(MSG_Append), REQF_NONE, G->MA_MailDir, "")) != NULL)
  {
    char aname[SIZE_PATHFILE];

    AddPath(aname, frc->drawer, frc->file, sizeof(aname));
    AB_LoadTree(data->LV_ADDRESSES, aname, TRUE, FALSE);
  }

  RETURN(0);
  return 0;
}

///
/// DECLARE(Save)
// save the address book using the default name
DECLARE(Save)
{
  GETDATA;
  struct BusyNode *busy;

  ENTER();

  busy = BusyBegin(BUSY_TEXT);
  BusyText(busy, tr(MSG_BusySavingAB), G->AB_Filename);
  AB_SaveTree(data->LV_ADDRESSES, G->AB_Filename);
  set(data->LV_ADDRESSES, MUIA_AddrBookListtree_Modified, FALSE);
  BusyEnd(busy);

  RETURN(0);
  return 0;
}

///
/// DECLARE(SaveAs)
// save the address book under a different name
DECLARE(SaveAs)
{
  struct FileReqCache *frc;

  ENTER();

  if((frc = ReqFile(ASL_ABOOK, obj, tr(MSG_SaveAs), REQF_SAVEMODE, G->MA_MailDir, "")) != NULL)
  {
    AddPath(G->AB_Filename, frc->drawer, frc->file, sizeof(G->AB_Filename));

    if(FileExists(G->AB_Filename) == FALSE ||
       MUI_Request(_app(obj), obj, MUIF_NONE, tr(MSG_MA_ConfirmReq), tr(MSG_YesNoReq), tr(MSG_FILE_OVERWRITE), frc->file) != 0)
    {
      DoMethod(obj, METHOD(Save));
    }
  }

  RETURN(0);
  return 0;
}

///
/// DECLARE(PrintAll)
// prints the entire address book in compact or detailed format
DECLARE(PrintAll)
{
  GETDATA;
  int mode;

  ENTER();

  mode = MUI_Request(_app(obj), obj, MUIF_NONE, tr(MSG_Print), tr(MSG_AB_PrintReqGads), tr(MSG_AB_PrintReq));
  if(mode != 0)
  {
    if(CheckPrinter(obj) == TRUE)
    {
      BOOL success = FALSE;
      FILE *prt;

      if((prt = fopen("PRT:", "w")) != NULL)
      {
        struct BusyNode *busy;

        setvbuf(prt, NULL, _IOFBF, SIZE_FILEBUF);

        busy = BusyBegin(BUSY_TEXT);
        BusyText(busy, tr(MSG_BusyPrintingAB), "");
        fprintf(prt, "%s\n", G->AB_Filename);

        if(mode == 2)
        {
          fprintf(prt, "\n  %-12.12s %-20.20s %s/%s\n", tr(MSG_AB_AliasFld), tr(MSG_EA_RealName), tr(MSG_EA_EmailAddress), tr(MSG_EA_Description));
          fputs("------------------------------------------------------------------------\n", prt);
        }
        AB_PrintLevel(data->LV_ADDRESSES, MUIV_NListtree_GetEntry_ListNode_Root, prt, mode);
        BusyEnd(busy);

        // before we close the file
        // handle we check the error state
        if(ferror(prt) == 0)
          success = TRUE;

        fclose(prt);
      }

      // signal the failure to the user
      // in case we were not able to print something
      if(success == FALSE)
        MUI_Request(_app(obj), obj, MUIF_NONE, tr(MSG_ErrorReq), tr(MSG_OkayReq), tr(MSG_ER_PRINTER_FAILED));
    }
  }

  RETURN(0);
  return 0;
}

///
/// DECLARE(PrintEntry)
// print selected address book entry in detailed format
DECLARE(PrintEntry)
{
  GETDATA;
  struct MUI_NListtree_TreeNode *tn;

  ENTER();

  if((tn = (struct MUI_NListtree_TreeNode *)xget(data->LV_ADDRESSES, MUIA_NListtree_Active)) != NULL)
  {
    if(CheckPrinter(obj) == TRUE)
    {
      BOOL success = FALSE;
      FILE *prt;

      if((prt = fopen("PRT:", "w")) != NULL)
      {
        struct ABEntry *ab = (struct ABEntry *)(tn->tn_User);

        setvbuf(prt, NULL, _IOFBF, SIZE_FILEBUF);

        set(_app(obj), MUIA_Application_Sleep, TRUE);

        AB_PrintLongEntry(prt, ab);
        if(ab->Type == AET_GROUP)
          AB_PrintLevel(data->LV_ADDRESSES, tn, prt, 1);

        // before we close the file
        // handle we check the error state
        if(ferror(prt) == 0)
          success = TRUE;

        fclose(prt);
        set(_app(obj), MUIA_Application_Sleep, FALSE);
      }

      // signal the failure to the user
      // in case we were not able to print something
      if(success == FALSE)
        MUI_Request(_app(obj), obj, MUIF_NONE, tr(MSG_ErrorReq), tr(MSG_OkayReq), tr(MSG_ER_PRINTER_FAILED));
    }
  }

  RETURN(0);
  return 0;
}

///
/// DECLARE(ActiveChange)
// the active entry in the address book listtree has changed
DECLARE(ActiveChange)
{
  GETDATA;
  BOOL disabled;

  ENTER();

  disabled = ((struct MUI_NListtree_TreeNode *)xget(data->LV_ADDRESSES, MUIA_NListtree_Active) == NULL);
  DoMethod(obj, MUIM_MultiSet, MUIA_Disabled, disabled,
    data->BT_TO,
    data->BT_CC,
    data->BT_BCC,
    NULL);

  if(data->TB_TOOLBAR != NULL)
    DoMethod(data->TB_TOOLBAR, MUIM_AddrBookToolbar_UpdateControls);

  RETURN(0);
  return 0;
}

///
/// DECLARE(HandleDoubleClick)
// the user double-clicked in the address book
DECLARE(HandleDoubleClick)
{
  GETDATA;

  ENTER();

  if(data->windowNumber != -1)
  {
    if(DoMethod(obj, METHOD(UseEntry), data->mode) == TRUE)
      DoMethod(obj, METHOD(Close));
  }
  else
  {
    struct MUI_NListtree_TreeNode *active;

    if((active = (struct MUI_NListtree_TreeNode *)xget(data->LV_ADDRESSES, MUIA_NListtree_Active)) != NULL &&
       isFlagClear(active->tn_Flags, TNF_LIST))
    {
      if(data->mode == ABM_CONFIG && data->recipientObject != NULL)
      {
        struct ABEntry *addr = (struct ABEntry *)active->tn_User;
        char *recipient;

        // check if the recipient string object has the NoFullName tag set
        // and if so we only add the recipient by email address
        if(xget(data->recipientObject, MUIA_Recipientstring_NoFullName) == TRUE)
          recipient = addr->Address;
        else if(addr->Alias != NULL)
          recipient = addr->Alias;
        else if(addr->RealName != NULL)
          recipient = addr->RealName;
        else
          recipient = addr->Address;

        // send the found recipient to the recipientstring object
        DoMethod(data->recipientObject, MUIM_Recipientstring_AddRecipient, recipient);

        // close the addressbook again.
        set(obj, MUIA_Window_CloseRequest, TRUE);

        // make sure to set the parentString as the new active object in
        // the window it belongs to because the user will return to it.
        set(_win(data->recipientObject), MUIA_Window_ActiveObject, data->recipientObject);
      }
      else
      {
        DoMethod(data->LV_ADDRESSES, MUIM_AddrBookListtree_EditEntry);
      }
    }
  }

  RETURN(0);
  return 0;
}

///
/// DECLARE(UseEntry)
// inserts an address book entry into a recipient string
DECLARE(UseEntry) // enum AddressbookMode mode
{
  GETDATA;
  BOOL result = FALSE;

  ENTER();

  if(msg->mode != ABM_NONE)
  {
    Object *writeWindow = NULL;

    if(data->windowNumber == -1)
    {
      struct WriteMailData *wmData = NewWriteMailWindow(NULL, 0);
      if(wmData != NULL)
        writeWindow = wmData->window;
    }
    else
    {
      struct WriteMailData *wmData;

      // find the write window object by iterating through the
      // global write window list and identify it via its window number
      IterateList(&G->writeMailDataList, struct WriteMailData *, wmData)
      {
        if(wmData->window != NULL &&
           (int)xget(wmData->window, MUIA_WriteWindow_Num) == data->windowNumber)
        {
          writeWindow = wmData->window;
          break;
        }
      }
    }

    if(writeWindow != NULL)
    {
      enum RcptType type = MUIV_WriteWindow_RcptType_To;
      struct MUI_NListtree_TreeNode *tn = (struct MUI_NListtree_TreeNode *)MUIV_NListtree_NextSelected_Start;

      switch(msg->mode)
      {
        case ABM_FROM:    type = MUIV_WriteWindow_RcptType_FromOverride; break;
        case ABM_TO:      type = MUIV_WriteWindow_RcptType_To; break;
        case ABM_CC:      type = MUIV_WriteWindow_RcptType_CC; break;
        case ABM_BCC:     type = MUIV_WriteWindow_RcptType_BCC; break;
        case ABM_REPLYTO: type = MUIV_WriteWindow_RcptType_ReplyTo; break;

        case ABM_NONE:
        case ABM_EDIT:
        case ABM_CONFIG:
          // nothing
        break;
      }

      do
      {
        DoMethod(data->LV_ADDRESSES, MUIM_NListtree_NextSelected, &tn);
        if(tn == (struct MUI_NListtree_TreeNode *)MUIV_NListtree_NextSelected_End || tn == NULL)
          break;
        else
          AB_InsertAddressTreeNode(writeWindow, type, data->LV_ADDRESSES, tn);
      }
      while(TRUE);

      result = TRUE;
    }
  }

  RETURN(result);
  return result;
}

///
/// DECLARE(Close)
// close the address book window
DECLARE(Close)
{
  GETDATA;
  BOOL closeWin = TRUE;

  ENTER();

  if(xget(data->LV_ADDRESSES, MUIA_AddrBookListtree_Modified) == TRUE)
  {
    switch(MUI_Request(_app(obj), obj, MUIF_NONE, NULL, tr(MSG_AB_ModifiedGads), tr(MSG_AB_Modified)))
    {
      case 0: closeWin = FALSE; break;
      case 1: DoMethod(obj, METHOD(Save)); break;
      case 2: break;
      case 3: AB_LoadTree(data->LV_ADDRESSES, G->AB_Filename, FALSE, FALSE); break;
    }
  }

  if(closeWin == TRUE)
    set(obj, MUIA_Window_Open, FALSE);

  RETURN(0);
  return 0;
}

///
/// DECLARE(ImportLDIF)
// imports an LDIF address book
DECLARE(ImportLDIF)
{
  GETDATA;
  struct FileReqCache *frc;

  ENTER();

  if((frc = ReqFile(ASL_ABOOK_LDIF, obj, tr(MSG_AB_IMPORT), REQF_NONE, G->MA_MailDir, "")) != NULL)
  {
    char ldifname[SIZE_PATHFILE];

    AddPath(ldifname, frc->drawer, frc->file, sizeof(ldifname));
    AB_ImportTreeLDIF(data->LV_ADDRESSES, ldifname, TRUE, FALSE);
  }

  RETURN(0);
  return 0;
}

///
/// DECLARE(ExportLDIF)
// exports an LDIF address book
DECLARE(ExportLDIF)
{
  GETDATA;
  struct FileReqCache *frc;

  ENTER();

  if((frc = ReqFile(ASL_ABOOK_LDIF, obj, tr(MSG_AB_EXPORT), REQF_SAVEMODE, G->MA_MailDir, "")) != NULL)
  {
    char ldifname[SIZE_PATHFILE];

    AddPath(ldifname, frc->drawer, frc->file, sizeof(ldifname));

    if(FileExists(ldifname) == FALSE ||
       MUI_Request(_app(obj), obj, MUIF_NONE, tr(MSG_MA_ConfirmReq), tr(MSG_YesNoReq), tr(MSG_FILE_OVERWRITE), frc->file) != 0)
    {
      AB_ExportTreeLDIF(data->LV_ADDRESSES, ldifname);
    }
  }

  RETURN(0);
  return 0;
}

///
/// DECLARE(ImportTabCSV)
// imports a comma or TAB separated address book
DECLARE(ImportTabCSV) // ULONG delim
{
  GETDATA;
  char delim = (char)msg->delim;
  int type;
  struct FileReqCache *frc;

  ENTER();

  switch(delim)
  {
    case '\t': type = ASL_ABOOK_TAB; break;
    case ',':  type = ASL_ABOOK_CSV; break;
    default:   type = ASL_ABOOK;     break;
  }

  if((frc = ReqFile(type, obj, tr(MSG_AB_IMPORT), REQF_NONE, G->MA_MailDir, "")) != NULL)
  {
    char aname[SIZE_PATHFILE];

    AddPath(aname, frc->drawer, frc->file, sizeof(aname));
    AB_ImportTreeTabCSV(data->LV_ADDRESSES, aname, TRUE, FALSE, delim);
  }

  RETURN(0);
  return 0;
}

///
/// DECLARE(ExportTabCSV)
// exports a comma or TAB separated address book
DECLARE(ExportTabCSV) // ULONG delim
{
  GETDATA;
  char delim = (char)msg->delim;
  int type;
  struct FileReqCache *frc;

  ENTER();

  switch(delim)
  {
    case '\t': type = ASL_ABOOK_TAB; break;
    case ',':  type = ASL_ABOOK_CSV; break;
    default:   type = ASL_ABOOK;     break;
  }

  if((frc = ReqFile(type, obj, tr(MSG_AB_EXPORT), REQF_SAVEMODE, G->MA_MailDir, "")) != NULL)
  {
    char aname[SIZE_PATHFILE];

    AddPath(aname, frc->drawer, frc->file, sizeof(aname));

    if(FileExists(aname) == FALSE ||
       MUI_Request(_app(obj), obj, MUIF_NONE, tr(MSG_MA_ConfirmReq), tr(MSG_YesNoReq), tr(MSG_FILE_OVERWRITE), frc->file) != 0)
    {
      AB_ExportTreeTabCSV(data->LV_ADDRESSES, aname, delim);
    }
  }

  RETURN(0);
  return 0;
}

///
/// DECLARE(ImportXML)
// imports an XML address book
DECLARE(ImportXML)
{
  GETDATA;
  struct FileReqCache *frc;

  ENTER();

  if((frc = ReqFile(ASL_ABOOK_XML, obj, tr(MSG_AB_IMPORT), REQF_NONE, G->MA_MailDir, "")) != NULL)
  {
    char xmlname[SIZE_PATHFILE];

    AddPath(xmlname, frc->drawer, frc->file, sizeof(xmlname));
    AB_ImportTreeXML(data->LV_ADDRESSES, xmlname, TRUE, FALSE);
  }

  RETURN(0);
  return 0;
}

///
/// DECLARE(Find)
// Searches an address book node for a given pattern
DECLARE(Find) // const char *pattern, enum AddressbookFind mode, char **result
{
  GETDATA;
  int res = 0;
  int i;
  BOOL goOn = TRUE;

  ENTER();

  D(DBF_ALWAYS, "searching for pattern '%s' in abook, mode=%ld", msg->pattern, msg->mode);

  for(i = 0; goOn == TRUE; i++)
  {
    struct MUI_NListtree_TreeNode *tn;

    if((tn = (struct MUI_NListtree_TreeNode *)DoMethod(data->LV_ADDRESSES, MUIM_NListtree_GetEntry, MUIV_NListtree_GetEntry_ListNode_Root, i, MUIF_NONE)) != NULL)
    {
      struct ABEntry *ab = tn->tn_User;

      if(ab->Type == AET_GROUP)
        continue;
      else
      {
        BOOL found = FALSE;
        int winnum;

        switch(msg->mode)
        {
          case ABF_RX_NAME:
            found = MatchNoCase(ab->RealName, msg->pattern);
          break;

          case ABF_RX_EMAIL:
            found = MatchNoCase(ab->Address, msg->pattern);
          break;

          case ABF_RX_NAMEEMAIL:
            found = MatchNoCase(ab->RealName, msg->pattern) || MatchNoCase(ab->Address, msg->pattern);
          break;

          default:
          {
            if((found = MatchNoCase(ab->Alias, msg->pattern) || MatchNoCase(ab->Comment, msg->pattern)) == FALSE)
            {
              if((found = MatchNoCase(ab->RealName, msg->pattern) || MatchNoCase(ab->Address, msg->pattern)) == FALSE && ab->Type == AET_USER)
              {
                found = MatchNoCase(ab->Homepage, msg->pattern) ||
                        MatchNoCase(ab->Street, msg->pattern)   ||
                        MatchNoCase(ab->City, msg->pattern)     ||
                        MatchNoCase(ab->Country, msg->pattern)  ||
                        MatchNoCase(ab->Phone, msg->pattern);
              }
            }
          }
        }

        if(found == TRUE)
        {
          D(DBF_ALWAYS, "found pattern '%s' in entry with address '%s'", msg->pattern, ab->Address);

          res++;

          if(msg->mode == ABF_USER)
          {
            char buf[SIZE_LARGE];

            DoMethod(data->LV_ADDRESSES, MUIM_NListtree_Open, MUIV_NListtree_Open_ListNode_Parent, tn, MUIF_NONE);
            set(data->LV_ADDRESSES, MUIA_NListtree_Active, tn);

            snprintf(buf, sizeof(buf), tr(MSG_AB_FoundEntry), ab->Alias, ab->RealName);

            switch(MUI_Request(_app(obj), obj, MUIF_NONE, tr(MSG_AB_FindEntry), tr(MSG_AB_FoundEntryGads), buf))
            {
              case 1:
                // nothing
              break;

              case 2:
              {
                if((winnum = EA_Init(ab->Type, ab)) >= 0)
                  EA_Setup(winnum, ab);
              }
              // fall through

              case 0:
              {
                res = -1;
                goOn = FALSE;
              }
              break;
            }
          }
          else if(msg->result != NULL)
            *msg->result++ = ab->Alias;
        }
      }
    }
    else
    {
      goOn = FALSE;
    }
  }

  RETURN(res);
  return res;
}

///
/// DECLARE(Search)
// searches the address book by alias, name or address
// it will break if there is more then one entry
DECLARE(Search) // const char *text, int mode, struct ABEntry **ab
{
  GETDATA;
  struct MUI_NListtree_TreeNode *tn;
  struct ABEntry *ab_found;
  int i;
  ULONG hits = 0;
  BOOL found = FALSE;
  int mode_type = msg->mode&ASM_TYPEMASK;
  LONG tl;

  ENTER();

  tl = strlen(msg->text);

  // we scan until we are at the end of the list or
  // if we found more then one matching entry
  for(i = 0; hits <= 2; i++, found = FALSE)
  {
    tn = (struct MUI_NListtree_TreeNode *)DoMethod(data->LV_ADDRESSES, MUIM_NListtree_GetEntry, MUIV_NListtree_GetEntry_ListNode_Root, i, MUIF_NONE);
    if(tn == NULL)
      break;

    // now we set the AB_Entry
    ab_found = tn->tn_User;
    if(ab_found == NULL)
      break;

    // now we check if this entry is one of the not wished entry types
    // and then we skip it.
    if(ab_found->Type == AET_USER  && !isUserSearch(msg->mode))
      continue;
    if(ab_found->Type == AET_LIST  && !isListSearch(msg->mode))
      continue;
    if(ab_found->Type == AET_GROUP && !isGroupSearch(msg->mode))
      continue;

    if(isCompleteSearch(msg->mode))
    {
      // Now we check for the ALIAS->REALNAME->ADDRESS, so only ONE mode is allowed at a time
      if(isAliasSearch(mode_type))
        found = !Strnicmp(ab_found->Alias,    msg->text, tl);
      else if(isRealNameSearch(mode_type))
        found = !Strnicmp(ab_found->RealName, msg->text, tl);
      else if(isAddressSearch(mode_type))
        found = !Strnicmp(ab_found->Address,  msg->text, tl);
    }
    else
    {
      // Now we check for the ALIAS->REALNAME->ADDRESS, so only ONE mode is allowed at a time
      if(isAliasSearch(mode_type))
        found = !Stricmp(ab_found->Alias,    msg->text);
      else if(isRealNameSearch(mode_type))
        found = !Stricmp(ab_found->RealName, msg->text);
      else if(isAddressSearch(mode_type))
        found = !Stricmp(ab_found->Address,  msg->text);
    }

    if(found == TRUE)
    {
      *msg->ab = ab_found;
      hits++;
    }
  }

  RETURN(hits);
  return hits;
}

///
/// DECLARE(CompleteAlias)
// auto-completes alias or name in recipient field
DECLARE(CompleteAlias) // const char *text
{
  char *compl = NULL;
  struct ABEntry *ab = NULL;

  ENTER();

  if(DoMethod(obj, METHOD(Search), msg->text, ASM_ALIAS|ASM_USER|ASM_LIST|ASM_COMPLETE, &ab) == 1)
  {
    compl = ab->Alias;
  }
  else if(DoMethod(obj, METHOD(Search), msg->text, ASM_REALNAME|ASM_USER|ASM_LIST|ASM_COMPLETE, &ab) == 1)
  {
    compl = ab->RealName;
  }
  else if(DoMethod(obj, METHOD(Search), msg->text, ASM_ADDRESS|ASM_USER|ASM_LIST|ASM_COMPLETE, &ab) == 1)
  {
    compl = ab->Address;
  }

  if(compl != NULL)
    compl = &compl[strlen(msg->text)];

  RETURN((IPTR)compl);
  return (IPTR)compl;
}

///
/// DECLARE(Goto)
// searches an entry by alias and activates it
DECLARE(Goto) // const char *alias
{
  GETDATA;
  struct MUI_NListtree_TreeNode *tn = NULL;

  ENTER();

  if(msg->alias != NULL)
  {
    if((tn = (struct MUI_NListtree_TreeNode *)DoMethod(data->LV_ADDRESSES, MUIM_NListtree_FindName, MUIV_NListtree_FindName_ListNode_Root, msg->alias, MUIF_NONE)) != NULL)
    {
      DoMethod(data->LV_ADDRESSES, MUIM_NListtree_Open, MUIV_NListtree_Open_ListNode_Parent, tn, MUIF_NONE);
      set(data->LV_ADDRESSES, MUIA_NListtree_Active, tn);
    }
  }

  RETURN((IPTR)tn);
  return (IPTR)tn;
}

///
/// DECLARE(CheckBirthdates)
// searches the address book for today's birthdays
DECLARE(CheckBirthdates) // ULONG check
{
  GETDATA;
  struct TimeVal nowTV;
  struct TimeVal nextTV;
  struct DateStamp nextDS;

  ENTER();

  // perform the check only if we are instructed to do it
  if(msg->check == TRUE)
  {
    ldiv_t today = ldiv(DateStamp2Long(NULL), 10000);
    int i = 0;
    struct MUI_NListtree_TreeNode *tn;

    while((tn = (struct MUI_NListtree_TreeNode *)DoMethod(data->LV_ADDRESSES, MUIM_NListtree_GetEntry, MUIV_NListtree_GetEntry_ListNode_Root, i, MUIF_NONE)) != NULL)
    {
      struct ABEntry *ab = tn->tn_User;

      if(ab->Type == AET_USER && ab->BirthDay != 0)
      {
        ldiv_t birthday = ldiv(ab->BirthDay, 10000);

        if(birthday.quot == today.quot)
        {
          char question[SIZE_LARGE];
          char *name = *ab->RealName ? ab->RealName : ab->Alias;
          char dateString[64];

          DateStamp2String(dateString, sizeof(dateString), NULL, DSS_DATE, TZC_NONE);
          snprintf(question, sizeof(question), tr(MSG_AB_BirthdayReqBody), dateString, name, today.rem - birthday.rem);

          // show the Birthday Requester
          BirthdayRequestWindowObject,
            MUIA_BirthdayRequestWindow_Body, question,
            MUIA_BirthdayRequestWindow_Alias, ab->Alias,
          End;
        }
      }

      i++;
    }
  }

  // reschedule the birthday check for the configured check time
  DateStamp(&nextDS);
  nextDS.ds_Minute = C->BirthdayCheckTime.ds_Minute;
  nextDS.ds_Tick = 0;

  DateStamp2TimeVal(&nextDS, &nextTV, TZC_NONE);

  GetSysTime(TIMEVAL(&nowTV));
  if(CmpTime(TIMEVAL(&nowTV), TIMEVAL(&nextTV)) < 0)
  {
    // if the check time is already over for today we schedule the next check
    // for tomorrow
    nextDS.ds_Days++;
    DateStamp2TimeVal(&nextDS, &nextTV, TZC_NONE);
  }

  #if defined(DEBUG)
  {
    char dateString[64];

    DateStamp2String(dateString, sizeof(dateString), &nextDS, DSS_DATETIME, TZC_NONE);
    D(DBF_TIMER, "next birthday check @ %s", dateString);
  }
  #endif

  RestartTimer(TIMER_CHECKBIRTHDAYS, nextTV.Seconds, nextTV.Microseconds, TRUE);

  RETURN(0);
  return 0;
}

///
