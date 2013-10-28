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

#include <ctype.h>
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
#include "YAM_error.h"

#include "mui/AddressBookEditWindow.h"
#include "mui/AddressBookListtree.h"
#include "mui/AddressBookToolbar.h"
#include "mui/RecipientString.h"
#include "mui/WriteWindow.h"

#include "Busy.h"
#include "Config.h"
#include "DynamicString.h"
#include "FileInfo.h"
#include "Locale.h"
#include "MUIObjects.h"
#include "Requesters.h"

#include "Debug.h"

/* CLASSDATA
struct Data
{
  Object *MI_EDIT;
  Object *MI_DUPLICATE;
  Object *MI_DELETE;
  Object *MI_PRINT;
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

/* EXPORT
enum AddressbookMode
{
  ABM_NONE=0,
  ABM_EDIT,
  ABM_FROM,
  ABM_TO,
  ABM_CC,
  ABM_BCC,
  ABM_REPLYTO,
  ABM_CONFIG
};
*/

/* Private functions */
/// CheckABookNode
static BOOL CheckABookNode(Object *obj, struct ABookNode *abn)
{
  BOOL ok = TRUE;

  ENTER();

  if(ok == TRUE && IsStrEmpty(abn->Alias) == TRUE)
  {
    ER_NewError(tr(MSG_ER_ErrorNoAlias));
    ok = FALSE;
  }

  if(ok == TRUE && IsStrEmpty(abn->Address) == TRUE)
  {
    ER_NewError(tr(MSG_ER_ErrorNoAddress));
    ok = FALSE;
  }

  if(ok == TRUE)
  {
    switch(abn->type)
    {
      case ABNT_USER:
      {
        switch(abn->DefSecurity)
        {
          case SEC_SIGN:
          case SEC_ENCRYPT:
          case SEC_BOTH:
          {
            // check if PGP was found to be available at all
            // or warn the user accordingly.
            if(G->PGPVersion == 0)
            {
              if(MUI_Request(_app(obj), obj, MUIF_NONE,
                             tr(MSG_AB_INVALIDSECURITY_TITLE),
                             tr(MSG_AB_INVALIDSECURITY_GADS),
                             tr(MSG_AB_INVALIDSECURITY)) != 0)
              {
                abn->DefSecurity = SEC_NONE;
              }
            }
          }
          break;

          default:
            // nothing
          break;
        }
      }
      break;

      case ABNT_GROUP:
      {
        // nothing to check
      }
      break;

      case ABNT_LIST:
      {
        // nothing to check
      }
      break;
    }
  }

  RETURN(ok);
  return ok;
}

///

/* Overloaded Methods */
/// OVERLOAD(OM_NEW)
OVERLOAD(OM_NEW)
{
  Object *MI_EDIT;
  Object *MI_DUPLICATE;
  Object *MI_DELETE;
  Object *MI_PRINT;
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
        MenuChild, MI_EDIT = Menuitem(tr(MSG_Edit), "E", TRUE, FALSE, AMEN_EDIT),
        MenuChild, MI_DUPLICATE = Menuitem(tr(MSG_AB_Duplicate), "D", TRUE, FALSE, AMEN_DUPLICATE),
        MenuChild, MI_DELETE = Menuitem(tr(MSG_AB_MIDelete), "Del", TRUE, TRUE, AMEN_DELETE),
        MenuChild, MenuBarLabel,
        MenuChild, MI_PRINT = Menuitem(tr(MSG_AB_MIPrint), NULL, TRUE, FALSE, AMEN_PRINTE),
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
            Child, TB_TOOLBAR = AddressBookToolbarObject,
            End,
          End,
        End),
      Child, NListviewObject,
        MUIA_CycleChain,        TRUE,
        MUIA_Listview_DragType, MUIV_Listview_DragType_Immediate,
        MUIA_NListview_NList,   LV_ADDRESSES = AddressBookListtreeObject,
        End,
      End,
    End,

    TAG_MORE, inittags(msg))) != NULL)
  {
    GETDATA;

    DoMethod(G->App, OM_ADDMEMBER, obj);

    data->MI_EDIT =      MI_EDIT;
    data->MI_DUPLICATE = MI_DUPLICATE;
    data->MI_DELETE =    MI_DELETE;
    data->MI_PRINT =     MI_PRINT;
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

    DoMethod(obj,          MUIM_Notify, MUIA_Window_MenuAction,                      AMEN_NEW,         obj,          1, METHOD(Clear));
    DoMethod(obj,          MUIM_Notify, MUIA_Window_MenuAction,                      AMEN_LOAD,        obj,          2, METHOD(Load), TRUE);
    DoMethod(obj,          MUIM_Notify, MUIA_Window_MenuAction,                      AMEN_APPEND,      obj,          1, METHOD(Append));
    DoMethod(obj,          MUIM_Notify, MUIA_Window_MenuAction,                      AMEN_IMPORT_LDIF, obj,          1, METHOD(ImportLDIF));
    DoMethod(obj,          MUIM_Notify, MUIA_Window_MenuAction,                      AMEN_IMPORT_TAB,  obj,          2, METHOD(ImportTabCSV), '\t');
    DoMethod(obj,          MUIM_Notify, MUIA_Window_MenuAction,                      AMEN_IMPORT_CSV,  obj,          2, METHOD(ImportTabCSV), ',');
    DoMethod(obj,          MUIM_Notify, MUIA_Window_MenuAction,                      AMEN_IMPORT_XML,  obj,          1, METHOD(ImportXML));
    DoMethod(obj,          MUIM_Notify, MUIA_Window_MenuAction,                      AMEN_EXPORT_LDIF, obj,          1, METHOD(ExportLDIF));
    DoMethod(obj,          MUIM_Notify, MUIA_Window_MenuAction,                      AMEN_EXPORT_TAB,  obj,          2, METHOD(ExportTabCSV), '\t');
    DoMethod(obj,          MUIM_Notify, MUIA_Window_MenuAction,                      AMEN_EXPORT_CSV,  obj,          2, METHOD(ExportTabCSV), ',');
    DoMethod(obj,          MUIM_Notify, MUIA_Window_MenuAction,                      AMEN_SAVE,        obj,          2, METHOD(Save), NULL);
    DoMethod(obj,          MUIM_Notify, MUIA_Window_MenuAction,                      AMEN_SAVEAS,      obj,          1, METHOD(SaveAs));
    DoMethod(obj,          MUIM_Notify, MUIA_Window_MenuAction,                      AMEN_PRINTA,      obj,          1, METHOD(PrintAll));
    DoMethod(obj,          MUIM_Notify, MUIA_Window_MenuAction,                      AMEN_NEWUSER,     obj,          2, METHOD(AddNewEntry), ABNT_USER);
    DoMethod(obj,          MUIM_Notify, MUIA_Window_MenuAction,                      AMEN_NEWLIST,     obj,          2, METHOD(AddNewEntry), ABNT_LIST);
    DoMethod(obj,          MUIM_Notify, MUIA_Window_MenuAction,                      AMEN_NEWGROUP,    obj,          2, METHOD(AddNewEntry), ABNT_GROUP);
    DoMethod(obj,          MUIM_Notify, MUIA_Window_MenuAction,                      AMEN_EDIT,        obj,          2, METHOD(EditOldEntry));
    DoMethod(obj,          MUIM_Notify, MUIA_Window_MenuAction,                      AMEN_DUPLICATE,   obj,          1, METHOD(DuplicateEntry));
    DoMethod(obj,          MUIM_Notify, MUIA_Window_MenuAction,                      AMEN_DELETE,      obj,          1, METHOD(DeleteEntry));
    DoMethod(obj,          MUIM_Notify, MUIA_Window_MenuAction,                      AMEN_PRINTE,      obj,          1, METHOD(PrintEntry));
    DoMethod(obj,          MUIM_Notify, MUIA_Window_MenuAction,                      AMEN_FIND,        obj,          1, METHOD(FindEntry));
    DoMethod(obj,          MUIM_Notify, MUIA_Window_MenuAction,                      AMEN_SORTALIAS,   LV_ADDRESSES, 2, MUIM_AddressBookListtree_SortBy, MUIV_AddressBookListtree_SortBy_Alias);
    DoMethod(obj,          MUIM_Notify, MUIA_Window_MenuAction,                      AMEN_SORTLNAME,   LV_ADDRESSES, 2, MUIM_AddressBookListtree_SortBy, MUIV_AddressBookListtree_SortBy_LastName);
    DoMethod(obj,          MUIM_Notify, MUIA_Window_MenuAction,                      AMEN_SORTFNAME,   LV_ADDRESSES, 2, MUIM_AddressBookListtree_SortBy, MUIV_AddressBookListtree_SortBy_FirstName);
    DoMethod(obj,          MUIM_Notify, MUIA_Window_MenuAction,                      AMEN_SORTDESC,    LV_ADDRESSES, 2, MUIM_AddressBookListtree_SortBy, MUIV_AddressBookListtree_SortBy_Coment);
    DoMethod(obj,          MUIM_Notify, MUIA_Window_MenuAction,                      AMEN_SORTADDR,    LV_ADDRESSES, 2, MUIM_AddressBookListtree_SortBy, MUIV_AddressBookListtree_SortBy_Address);
    DoMethod(obj,          MUIM_Notify, MUIA_Window_MenuAction,                      AMEN_FOLD,        LV_ADDRESSES, 2, MUIM_AddressBookListtree_FoldTree, FALSE);
    DoMethod(obj,          MUIM_Notify, MUIA_Window_MenuAction,                      AMEN_UNFOLD,      LV_ADDRESSES, 2, MUIM_AddressBookListtree_FoldTree, TRUE);
    DoMethod(LV_ADDRESSES, MUIM_Notify, MUIA_NListtree_Active,                       MUIV_EveryTime,   obj,          2, METHOD(ActiveChange), MUIV_TriggerValue);
    DoMethod(LV_ADDRESSES, MUIM_Notify, MUIA_NListtree_DoubleClick,                  MUIV_EveryTime,   obj,          1, METHOD(HandleDoubleClick));
    DoMethod(LV_ADDRESSES, MUIM_Notify, MUIA_AddressBookListtree_DeleteEntryRequest, MUIV_EveryTime,   obj,          1, METHOD(DeleteEntry));
    DoMethod(BT_TO,        MUIM_Notify, MUIA_Pressed,                                FALSE,            obj,          1, METHOD(UseEntry), ABM_TO);
    DoMethod(BT_CC,        MUIM_Notify, MUIA_Pressed,                                FALSE,            obj,          1, METHOD(UseEntry), ABM_CC);
    DoMethod(BT_BCC,       MUIM_Notify, MUIA_Pressed,                                FALSE,            obj,          1, METHOD(UseEntry), ABM_BCC);
    DoMethod(obj,          MUIM_Notify, MUIA_Window_CloseRequest,                    TRUE,             obj,          3, MUIM_Set, MUIA_Window_Open, FALSE);

    if(TB_TOOLBAR != NULL)
    {
      DoMethod(TB_TOOLBAR, MUIM_TheBar_Notify, TB_ABOOK_SAVE,      MUIA_Pressed, FALSE, obj,          2, METHOD(Save), NULL);
      DoMethod(TB_TOOLBAR, MUIM_TheBar_Notify, TB_ABOOK_FIND,      MUIA_Pressed, FALSE, obj,          1, METHOD(FindEntry));
      DoMethod(TB_TOOLBAR, MUIM_TheBar_Notify, TB_ABOOK_NEWUSER,   MUIA_Pressed, FALSE, obj,          2, METHOD(AddNewEntry), ABNT_USER);
      DoMethod(TB_TOOLBAR, MUIM_TheBar_Notify, TB_ABOOK_NEWLIST,   MUIA_Pressed, FALSE, obj,          2, METHOD(AddNewEntry), ABNT_LIST);
      DoMethod(TB_TOOLBAR, MUIM_TheBar_Notify, TB_ABOOK_NEWGROUP,  MUIA_Pressed, FALSE, obj,          2, METHOD(AddNewEntry), ABNT_GROUP);
      DoMethod(TB_TOOLBAR, MUIM_TheBar_Notify, TB_ABOOK_EDIT,      MUIA_Pressed, FALSE, obj,          1, METHOD(EditOldEntry));
      DoMethod(TB_TOOLBAR, MUIM_TheBar_Notify, TB_ABOOK_DELETE,    MUIA_Pressed, FALSE, obj,          1, METHOD(DeleteEntry));
      DoMethod(TB_TOOLBAR, MUIM_TheBar_Notify, TB_ABOOK_PRINT,     MUIA_Pressed, FALSE, obj,          1, METHOD(PrintAll));
      DoMethod(TB_TOOLBAR, MUIM_TheBar_Notify, TB_ABOOK_OPENTREE,  MUIA_Pressed, FALSE, LV_ADDRESSES, 2, MUIM_AddressBookListtree_FoldTree, TRUE);
      DoMethod(TB_TOOLBAR, MUIM_TheBar_Notify, TB_ABOOK_CLOSETREE, MUIA_Pressed, FALSE, LV_ADDRESSES, 2, MUIM_AddressBookListtree_FoldTree, FALSE);
    }
  }

  RETURN((IPTR)obj);
  return (IPTR)obj;
}

///
/// OVERLOAD(OM_SET)
OVERLOAD(OM_SET)
{
  GETDATA;
  struct TagItem *tags = inittags(msg), *tag;
  ULONG result;

  ENTER();

  while((tag = NextTagItem((APTR)&tags)) != NULL)
  {
    switch(tag->ti_Tag)
    {
      case MUIA_Window_Open:
      {
        // rebuild the listtree upon opening the window
        if(tag->ti_Data == TRUE)
          DoMethod(data->LV_ADDRESSES, MUIM_AddressBookListtree_BuildTree);
      }
      break;

      case ATTR(WindowNumber):
      {
        data->windowNumber = tag->ti_Data;
        tag->ti_Tag = TAG_IGNORE;
      }
      break;

      case ATTR(ConfigModified):
      {
        DoMethod(data->LV_ADDRESSES, MUIM_AddressBookListtree_MakeFormat);
        tag->ti_Tag = TAG_IGNORE;
      }
      break;

      case ATTR(ActiveEntry):
      {
        set(data->LV_ADDRESSES, MUIA_AddressBookListtree_ActiveEntry, tag->ti_Data);
        tag->ti_Tag = TAG_IGNORE;
      }
      break;
    }
  }

  result = DoSuperMethodA(cl, obj, msg);

  RETURN(result);
  return result;
}

///
/// OVERLOAD(OM_GET)
OVERLOAD(OM_GET)
{
  GETDATA;
  IPTR *store = ((struct opGet *)msg)->opg_Storage;

  switch(((struct opGet *)msg)->opg_AttrID)
  {
    case ATTR(WindowNumber): *store = data->windowNumber; return TRUE;
    case ATTR(Modified):     *store = G->abook.modified; return TRUE;
    case ATTR(Listtree):     *store = (IPTR)data->LV_ADDRESSES; return TRUE;
    case ATTR(ActiveGroup):  *store = (IPTR)xget(data->LV_ADDRESSES, MUIA_AddressBookListtree_ActiveGroup); return TRUE;
    case ATTR(ActiveEntry):  *store = (IPTR)xget(data->LV_ADDRESSES, MUIA_AddressBookListtree_ActiveEntry); return TRUE;
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
  struct MUI_NListtree_TreeNode *tn;

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

  data->mode = msg->mode;
  data->windowNumber = (*md != '\0' ? msg->windowNumber : -1);

  // enable/disable the To/CC/BCC buttons depending on whether there is an active entry or not
  tn = (struct MUI_NListtree_TreeNode *)xget(data->LV_ADDRESSES, MUIA_NListtree_Active);
  DoMethod(obj, MUIM_MultiSet, MUIA_Disabled, data->mode != ABM_CONFIG || tn == NULL,
    data->BT_TO,
    data->BT_CC,
    data->BT_BCC,
    NULL);
  DoMethod(obj, MUIM_MultiSet, MUIA_Menuitem_Enabled, tn != NULL,
    data->MI_EDIT,
    data->MI_DUPLICATE,
    data->MI_DELETE,
    data->MI_PRINT,
    NULL);
  if(data->TB_TOOLBAR != NULL)
  {
    DoMethod(data->TB_TOOLBAR, MUIM_TheBar_SetAttr, TB_ABOOK_EDIT,   MUIA_TheBar_Attr_Disabled, tn == NULL);
    DoMethod(data->TB_TOOLBAR, MUIM_TheBar_SetAttr, TB_ABOOK_DELETE, MUIA_TheBar_Attr_Disabled, tn == NULL);
  }

  snprintf(data->windowTitle, sizeof(data->windowTitle), "%s %s", tr(MSG_MA_MAddrBook), md);

  xset(obj,
    MUIA_Window_Title, data->windowTitle,
    MUIA_Window_ScreenTitle, CreateScreenTitle(data->screenTitle, sizeof(data->screenTitle), data->windowTitle));

  SafeOpenWindow(obj);

  RETURN(0);
  return 0;
}

///
/// DECLARE(Clear)
DECLARE(Clear)
{
  GETDATA;

  ENTER();

  ClearABook(&G->abook);
  G->abook.modified = TRUE;
  DoMethod(data->LV_ADDRESSES, MUIM_NListtree_Clear);

  RETURN(0);
  return 0;
}

///
/// DECLARE(Load)
// load an address book
DECLARE(Load) // ULONG request
{
  GETDATA;
  BOOL load = FALSE;

  ENTER();

  if(msg->request == TRUE)
  {
    struct FileReqCache *frc;

    if((frc = ReqFile(ASL_ABOOK, obj, tr(MSG_Open), REQF_NONE, G->MA_MailDir, "")) != NULL)
    {
      AddPath(G->abookFilename, frc->drawer, frc->file, sizeof(G->abookFilename));
      load = TRUE;
    }
  }
  else
  {
    load = TRUE;
  }

  if(load == TRUE)
  {
    if(LoadABook(G->abookFilename, &G->abook, FALSE) == TRUE)
      DoMethod(data->LV_ADDRESSES, MUIM_AddressBookListtree_BuildTree);
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
    if(LoadABook(aname, &G->abook, TRUE) == TRUE)
      DoMethod(data->LV_ADDRESSES, MUIM_AddressBookListtree_BuildTree);
  }

  RETURN(0);
  return 0;
}

///
/// DECLARE(Save)
// save the address book using the default name
DECLARE(Save) // const char *filename;
{
  const char *filename;
  struct BusyNode *busy;
  ULONG success;

  ENTER();

  if(msg->filename != NULL)
    filename = msg->filename;
  else
    filename = G->abookFilename;

  busy = BusyBegin(BUSY_TEXT);
  BusyText(busy, tr(MSG_BusySavingAB), filename);
  success = SaveABook(filename, &G->abook);
  if(msg->filename == NULL)
    G->abook.modified = FALSE;
  BusyEnd(busy);

  RETURN(success);
  return success;
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
    AddPath(G->abookFilename, frc->drawer, frc->file, sizeof(G->abookFilename));

    if(FileExists(G->abookFilename) == FALSE ||
       MUI_Request(_app(obj), obj, MUIF_NONE, tr(MSG_MA_ConfirmReq), tr(MSG_YesNoReq), tr(MSG_FILE_OVERWRITE), frc->file) != 0)
    {
      DoMethod(obj, METHOD(Save), G->abookFilename);
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
        fprintf(prt, "%s\n", G->abookFilename);

        if(mode == 2)
        {
          fprintf(prt, "\n  %-12.12s %-20.20s %s/%s\n", tr(MSG_AB_AliasFld), tr(MSG_EA_RealName), tr(MSG_EA_EmailAddress), tr(MSG_EA_Description));
          fputs("------------------------------------------------------------------------\n", prt);
        }

        PrintABook(&G->abook, prt, mode);
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
        struct ABookNode *abn = (struct ABookNode *)tn->tn_User;

        setvbuf(prt, NULL, _IOFBF, SIZE_FILEBUF);

        set(_app(obj), MUIA_Application_Sleep, TRUE);

        if(abn->type == ABNT_GROUP)
          PrintABookGroup(abn, prt, 1);
        else
          PrintLongABookEntry(abn, prt);

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
/// DECLARE(FindEntry)
// searches address book
DECLARE(FindEntry)
{
  GETDATA;
  char pattern[SIZE_PATTERN];

  ENTER();

  pattern[0] = '\0';
  if(StringRequest(pattern, sizeof(pattern), tr(MSG_AB_FindEntry), tr(MSG_AB_FindEntryReq), tr(MSG_AB_StartSearch), NULL, tr(MSG_Cancel), FALSE, obj) != 0)
  {
    char searchPattern[SIZE_PATTERN+4];
    struct ABookNode *abn = NULL;
    ULONG iterator;
    ULONG foundEntries = 0;
    BOOL continueSearch = TRUE;

    snprintf(searchPattern, sizeof(searchPattern), "#?%s#?", pattern);

    iterator = 0;
    while(continueSearch == TRUE && (abn = (struct ABookNode *)DoMethod(data->LV_ADDRESSES, MUIM_AddressBookListtree_IncrementalSearch, searchPattern, &iterator)) != NULL)
    {
      char buf[SIZE_LARGE];

      foundEntries++;

      snprintf(buf, sizeof(buf), tr(MSG_AB_FoundEntry), abn->Alias, abn->RealName);

      switch(MUI_Request(_app(obj), obj, MUIF_NONE, tr(MSG_AB_FindEntry), tr(MSG_AB_FoundEntryGads), buf))
      {
        case 1:
          // continue search
        break;

        case 2:
        {
          // edit this entry, it is already the active one
          DoMethod(obj, METHOD(EditOldEntry));
          // abort search
          continueSearch = FALSE;
        }
        break;

        case 0:
        {
          // abort search
          continueSearch = FALSE;
        }
        break;
      }
    }

    if(foundEntries == 0)
      MUI_Request(_app(obj), obj, MUIF_NONE, tr(MSG_AB_FindEntry), tr(MSG_OkayReq), tr(MSG_AB_NoneFound));
  }

  RETURN(0);
  return 0;
}

///
/// DECLARE(AddNewEntry)
// add a new entry to the address book
DECLARE(AddNewEntry) // ULONG type
{
  Object *editWin;

  ENTER();

  if((editWin = AddressBookEditWindowObject,
    MUIA_AddressBookEditWindow_Type, msg->type,
  End) != NULL)
  {
    set(editWin, MUIA_AddressBookEditWindow_ABookNode, NULL);
    DoMethod(editWin, MUIM_Notify, MUIA_AddressBookEditWindow_SaveContents, MUIV_EveryTime, obj, 3, METHOD(InsertNewEntry), editWin, msg->type);
    SafeOpenWindow(editWin);
  }

  RETURN(0);
  return 0;
}

///
/// DECLARE(InsertNewEntry)
// add a new entry to the address book
DECLARE(InsertNewEntry) // Object *editWindow, ULONG type
{
  GETDATA;
  struct ABookNode abn;

  ENTER();

  get(msg->editWindow, MUIA_AddressBookEditWindow_ABookNode, &abn);
  if(CheckABookNode(obj, &abn) == TRUE)
  {
    struct MUI_NListtree_TreeNode *predTN;
    struct MUI_NListtree_TreeNode *groupTN;
    struct ABookNode *dup;

    FixAlias(&G->abook, &abn, NULL);

    // find the preceeding and the group node of the active entry
    predTN = (struct MUI_NListtree_TreeNode *)xget(data->LV_ADDRESSES, MUIA_NListtree_Active);
    if(predTN == (struct MUI_NListtree_TreeNode *)MUIV_NListtree_Active_Off)
    {
      predTN = (struct MUI_NListtree_TreeNode *)MUIV_NListtree_Insert_PrevNode_Tail;
      groupTN = (struct MUI_NListtree_TreeNode *)MUIV_NListtree_Insert_ListNode_Root;
    }
    else
    {
      groupTN = (struct MUI_NListtree_TreeNode *)DoMethod(data->LV_ADDRESSES, MUIM_NListtree_GetEntry, predTN, MUIV_NListtree_GetEntry_Position_Parent, MUIF_NONE);
    }

    if((dup = DuplicateNode(&abn, sizeof(abn))) != NULL)
    {
      // insert the new node in both the address book and the listtree
      // the listtree already does the dirty work to insert the node in the address book
      DoMethod(data->LV_ADDRESSES, MUIM_NListtree_Insert, dup->Alias, dup, groupTN, predTN, MUIV_NListtree_Insert_Flag_Active);
      DoMethod(msg->editWindow, MUIM_AddressBookEditWindow_Close);
      AppendToLogfile(LF_VERBOSE, 71, tr(MSG_LOG_NewAddress), abn.Alias);
      G->abook.modified = TRUE;
    }
    else
    {
      dstrfree(abn.ListMembers);
    }
  }

  RETURN(0);
  return 0;
}

///
/// DECLARE(EditNewEntry)
// edit an already setup entry and add it to the address book
DECLARE(EditNewEntry) // struct ABookNode *abn
{
  Object *editWin;

  ENTER();

  if((editWin = AddressBookEditWindowObject,
    MUIA_AddressBookEditWindow_Type, msg->abn->type,
  End) != NULL)
  {
    set(editWin, MUIA_AddressBookEditWindow_ABookNode, msg->abn);
    DoMethod(editWin, MUIM_Notify, MUIA_AddressBookEditWindow_SaveContents, MUIV_EveryTime, obj, 3, METHOD(InsertNewEntry), editWin, msg->abn->type);
    SafeOpenWindow(editWin);
  }

  RETURN(0);
  return 0;
}

///
/// DECLARE(EditOldEntry)
// edit the selected address book entry
DECLARE(EditOldEntry)
{
  GETDATA;
  struct MUI_NListtree_TreeNode *tn;

  ENTER();

  if((tn = (struct MUI_NListtree_TreeNode *)xget(data->LV_ADDRESSES, MUIA_NListtree_Active)) != NULL)
  {
    struct ABookNode *abn = (struct ABookNode *)tn->tn_User;
    Object *editWin;

	if((editWin = AddressBookEditWindowObject,
	  MUIA_AddressBookEditWindow_Type, abn->type,
	End) != NULL)
	{
      set(editWin, MUIA_AddressBookEditWindow_ABookNode, abn);
      DoMethod(editWin, MUIM_Notify, MUIA_AddressBookEditWindow_SaveContents, MUIV_EveryTime, obj, 3, METHOD(UpdateOldEntry), editWin, tn);
      SafeOpenWindow(editWin);
    }
  }

  RETURN(0);
  return 0;
}

///
/// DECLARE(UpdateOldEntry)
// edit the selected address book entry
DECLARE(UpdateOldEntry) // Object *editWindow, struct MUI_NListtree_TreeNode *tn
{
  GETDATA;
  struct ABookNode *oldABN;
  struct ABookNode abn;

  ENTER();

  // create a copy of the old entry and let the edit window update the contents
  oldABN = (struct ABookNode *)msg->tn->tn_User;
  memcpy(&abn, oldABN, sizeof(abn));
  get(msg->editWindow, MUIA_AddressBookEditWindow_ABookNode, &abn);

  // check if everything is ok
  if(CheckABookNode(obj, &abn) == TRUE)
  {
    FixAlias(&G->abook, &abn, oldABN);

    // check if something has changed
    if(CompareABookNodes(&abn, oldABN) == FALSE)
    {
      // copy everything back
      memcpy(oldABN, &abn, sizeof(*oldABN));

      // update the listtree and mark the address book as modified
      DoMethod(data->LV_ADDRESSES, MUIM_NListtree_Redraw, msg->tn, MUIF_NONE);
      G->abook.modified = TRUE;
    }

    // close the edit window
    DoMethod(msg->editWindow, MUIM_AddressBookEditWindow_Close);
  }

  RETURN(0);
  return 0;
}

///
/// DECLARE(DuplicateEntry)
// duplicate the selected address book entry
DECLARE(DuplicateEntry)
{
  GETDATA;
  struct MUI_NListtree_TreeNode *tn;

  ENTER();

  if((tn = (struct MUI_NListtree_TreeNode *)xget(data->LV_ADDRESSES, MUIA_NListtree_Active)) != NULL)
  {
    struct ABookNode *abn = (struct ABookNode *)tn->tn_User;
    Object *editWin;

	if((editWin = AddressBookEditWindowObject,
	  MUIA_AddressBookEditWindow_Type, abn->type,
	End) != NULL)
	{
      char buf[SIZE_NAME];
      size_t len;

      strlcpy(buf, abn->Alias, sizeof(buf));
      if((len = strlen(buf)) != 0)
      {
        if(isdigit(buf[len-1]))
          buf[len-1]++;
        else if(len < sizeof(buf)-1)
          strlcat(buf, "2", sizeof(buf));
        else
          buf[len-1] = '2';
      }

	  xset(editWin,
	    MUIA_AddressBookEditWindow_ABookNode, abn,
	    MUIA_AddressBookEditWindow_Address, buf);
      DoMethod(editWin, MUIM_Notify, MUIA_AddressBookEditWindow_SaveContents, MUIV_EveryTime, obj, 3, METHOD(InsertNewEntry), editWin, abn->type);
      SafeOpenWindow(editWin);
    }
  }

  RETURN(0);
  return 0;
}

///
/// DECLARE(DeleteEntry)
// delete the selected address book entry
DECLARE(DeleteEntry)
{
  GETDATA;
  struct MUI_NListtree_TreeNode *activeTN;

  ENTER();

  if((activeTN = (struct MUI_NListtree_TreeNode *)xget(data->LV_ADDRESSES, MUIA_NListtree_Active)) != NULL)
  {
    struct MUI_NListtree_TreeNode *groupTN;
    struct ABookNode *abn = (struct ABookNode *)activeTN->tn_User;

    groupTN = (struct MUI_NListtree_TreeNode *)DoMethod(data->LV_ADDRESSES, MUIM_NListtree_GetEntry, activeTN, MUIV_NListtree_GetEntry_Position_Parent, MUIF_NONE);
    if(groupTN == NULL)
    {
      groupTN = (struct MUI_NListtree_TreeNode *)MUIV_NListtree_Remove_ListNode_Root;
    }

    DoMethod(data->LV_ADDRESSES, MUIM_NListtree_Remove, groupTN, activeTN, MUIF_NONE);
    RemoveABookNode(abn);
    DeleteABookNode(abn);
    G->abook.modified = TRUE;
  }

  RETURN(0);
  return 0;
}

///
/// DECLARE(ActiveChange)
// the active entry in the address book listtree has changed
DECLARE(ActiveChange) // struct MUI_NListtree_TreeNode *tn
{
  GETDATA;

  ENTER();

  DoMethod(obj, MUIM_MultiSet, MUIA_Disabled, data->mode != ABM_CONFIG || msg->tn == NULL,
    data->BT_TO,
    data->BT_CC,
    data->BT_BCC,
    NULL);
  DoMethod(obj, MUIM_MultiSet, MUIA_Menuitem_Enabled, msg->tn != NULL,
    data->MI_EDIT,
    data->MI_DUPLICATE,
    data->MI_DELETE,
    data->MI_PRINT,
    NULL);
  if(data->TB_TOOLBAR != NULL)
  {
    DoMethod(data->TB_TOOLBAR, MUIM_TheBar_SetAttr, TB_ABOOK_EDIT,   MUIA_TheBar_Attr_Disabled, msg->tn == NULL);
    DoMethod(data->TB_TOOLBAR, MUIM_TheBar_SetAttr, TB_ABOOK_DELETE, MUIA_TheBar_Attr_Disabled, msg->tn == NULL);
  }

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
      set(obj, MUIA_Window_Open, FALSE);
  }
  else
  {
    struct MUI_NListtree_TreeNode *active;

    if((active = (struct MUI_NListtree_TreeNode *)xget(data->LV_ADDRESSES, MUIA_NListtree_Active)) != NULL &&
       isFlagClear(active->tn_Flags, TNF_LIST))
    {
      if(data->mode == ABM_CONFIG && data->recipientObject != NULL)
      {
        struct ABookNode *addr = (struct ABookNode *)active->tn_User;
        char *recipient;

        // check if the recipient string object has the NoFullName tag set
        // and if so we only add the recipient by email address
        if(xget(data->recipientObject, MUIA_RecipientString_NoFullName) == TRUE)
          recipient = addr->Address;
        else if(addr->Alias != NULL)
          recipient = addr->Alias;
        else if(addr->RealName != NULL)
          recipient = addr->RealName;
        else
          recipient = addr->Address;

        // send the found recipient to the recipientstring object
        DoMethod(data->recipientObject, MUIM_RecipientString_AddRecipient, recipient);

        // close the addressbook again.
        set(obj, MUIA_Window_CloseRequest, TRUE);

        // make sure to set the parentString as the new active object in
        // the window it belongs to because the user will return to it.
        set(_win(data->recipientObject), MUIA_Window_ActiveObject, data->recipientObject);
      }
      else
      {
        DoMethod(obj, METHOD(EditOldEntry));
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
          DoMethod(writeWindow, MUIM_WriteWindow_InsertABookTreenode, type, data->LV_ADDRESSES, tn);
      }
      while(TRUE);

      result = TRUE;
    }
  }

  RETURN(result);
  return result;
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
    if(ImportLDIFABook(ldifname, &G->abook, TRUE) == TRUE)
      DoMethod(data->LV_ADDRESSES, MUIM_AddressBookListtree_BuildTree);
  }

  RETURN(0);
  return 0;
}

///
/// DECLARE(ExportLDIF)
// exports an LDIF address book
DECLARE(ExportLDIF)
{
  struct FileReqCache *frc;

  ENTER();

  if((frc = ReqFile(ASL_ABOOK_LDIF, obj, tr(MSG_AB_EXPORT), REQF_SAVEMODE, G->MA_MailDir, "")) != NULL)
  {
    char ldifname[SIZE_PATHFILE];

    AddPath(ldifname, frc->drawer, frc->file, sizeof(ldifname));

    if(FileExists(ldifname) == FALSE ||
       MUI_Request(_app(obj), obj, MUIF_NONE, tr(MSG_MA_ConfirmReq), tr(MSG_YesNoReq), tr(MSG_FILE_OVERWRITE), frc->file) != 0)
    {
      ExportLDIFABook(ldifname, &G->abook);
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
    if(ImportCSVABook(aname, &G->abook, TRUE, delim) == TRUE)
      DoMethod(data->LV_ADDRESSES, MUIM_AddressBookListtree_BuildTree);
  }

  RETURN(0);
  return 0;
}

///
/// DECLARE(ExportTabCSV)
// exports a comma or TAB separated address book
DECLARE(ExportTabCSV) // ULONG delim
{
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
      ExportCSVABook(aname, &G->abook, delim);
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
    if(ImportXMLABook(xmlname, &G->abook, TRUE) == TRUE)
      DoMethod(data->LV_ADDRESSES, MUIM_AddressBookListtree_BuildTree);
  }

  RETURN(0);
  return 0;
}

///
/// DECLARE(CompleteAlias)
// auto-completes alias or name in recipient field
DECLARE(CompleteAlias) // const char *text
{
  char *compl = NULL;
  struct ABookNode *abn = NULL;

  ENTER();

  if(SearchABook(&G->abook, msg->text, ASM_ALIAS|ASM_USER|ASM_LIST|ASM_COMPLETE, &abn) == 1)
  {
    compl = abn->Alias;
  }
  else if(SearchABook(&G->abook, msg->text, ASM_REALNAME|ASM_USER|ASM_LIST|ASM_COMPLETE, &abn) == 1)
  {
    compl = abn->RealName;
  }
  else if(SearchABook(&G->abook, msg->text, ASM_ADDRESS|ASM_USER|ASM_LIST|ASM_COMPLETE, &abn) == 1)
  {
    compl = abn->Address;
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
/// DECLARE(RebuildTree)
DECLARE(RebuildTree)
{
  GETDATA;

  ENTER();

  DoMethod(data->LV_ADDRESSES, MUIM_AddressBookListtree_BuildTree);

  RETURN(0);
  return 0;
}

///
/// DECLARE(RedrawActiveEntry)
DECLARE(RedrawActiveEntry)
{
  GETDATA;

  ENTER();

  DoMethod(data->LV_ADDRESSES, MUIM_NListtree_Redraw, MUIV_NListtree_Redraw_Active);

  RETURN(0);
  return 0;
}

///
