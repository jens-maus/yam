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

***************************************************************************/

#include <ctype.h>
#include <stdlib.h>
#include <string.h>

#include <clib/macros.h>

#include <libraries/asl.h>
#include <mui/BetterString_mcc.h>
#include <mui/NBalance_mcc.h>
#include <mui/NListview_mcc.h>
#include <mui/TextEditor_mcc.h>
#if defined(__amigaos4__)
#include <proto/application.h>
#endif
#include <proto/dos.h>
#include <proto/exec.h>
#include <proto/intuition.h>
#include <proto/muimaster.h>
#include <proto/wb.h>
#include <proto/xpkmaster.h>

#if !defined(__amigaos4__)
#include <clib/alib_protos.h> // DoMethod
#endif

#include "extrasrc.h"

#include "SDI_hook.h"

#include "YAM.h"
#include "YAM_addressbook.h"
#include "YAM_config.h"
#include "YAM_configFile.h"
#include "YAM_error.h"
#include "YAM_find.h"
#include "YAM_global.h"
#include "YAM_main.h"
#include "YAM_mainFolder.h"
#include "YAM_utilities.h"

#include "mui/AccountList.h"
#include "mui/ClassesExtra.h"
#include "mui/FilterChooser.h"
#include "mui/FilterList.h"
#include "mui/FilterRuleList.h"
#include "mui/FolderRequestListtree.h"
#include "mui/IdentityList.h"
#include "mui/ImageArea.h"
#include "mui/MailServerChooser.h"
#include "mui/MimeTypeList.h"
#include "mui/PlaceholderPopupList.h"
#include "mui/ScriptList.h"
#include "mui/SearchControlGroup.h"
#include "mui/SignatureChooser.h"
#include "mui/SignatureList.h"
#include "mui/SignatureTextEdit.h"
#include "mui/ThemeListGroup.h"
#include "mui/TZoneChooser.h"
#include "mui/TZoneInfoBar.h"
#include "mui/YAMApplication.h"

#include "BayesFilter.h"
#include "FileInfo.h"
#include "FolderList.h"
#include "ImageCache.h"
#include "Locale.h"
#include "MimeTypes.h"
#include "MailServers.h"
#include "MUIObjects.h"
#include "Requesters.h"
#include "Signature.h"
#include "Threads.h"
#include "TZone.h"
#include "UIDL.h"
#include "UserIdentity.h"

#include "Debug.h"

/* local defines */
/// ConfigPageHeaderObject()
#define ConfigPageHeaderObject(id, image, title, summary) \
  Child, HGroup,                                          \
    Child, MakeImageObject(id, image),                    \
    Child, VGroup,                                        \
      Child, TextObject,                                  \
        MUIA_Text_PreParse, "\033b",                      \
        MUIA_Text_Contents, (title),                      \
        MUIA_Text_Copy,     FALSE,                        \
        MUIA_Weight,        100,                          \
      End,                                                \
      Child, TextObject,                                  \
        MUIA_Text_Contents, (summary),                    \
        MUIA_Text_Copy,     FALSE,                        \
        MUIA_Font,          MUIV_Font_Tiny,               \
        MUIA_Weight,        100,                          \
      End,                                                \
    End,                                                  \
  End,                                                    \
  Child, RectangleObject,                                 \
    MUIA_Rectangle_HBar, TRUE,                            \
    MUIA_FixHeight,      4,                               \
  End

///

/***************************************************************************
 Module: Configuration - GUI for sections
***************************************************************************/

/*** Hooks ***/
/// PO_Text2ListFunc
//  selects the folder as active which is currently in the 'str'
//  object
HOOKPROTONH(PO_Text2List, BOOL, Object *listview, Object *str)
{
  char *s;

  ENTER();

  // get the currently set string
  s = (char *)xget(str, MUIA_Text_Contents);

  if(s != NULL && listview != NULL)
  {
    Object *list = (Object *)xget(listview, MUIA_NListview_NList);

    // now try to find the node and activate it right away
    DoMethod(list, MUIM_NListtree_FindName, MUIV_NListtree_FindName_ListNode_Root, s, MUIV_NListtree_FindName_Flag_Activate);
  }

  RETURN(TRUE);
  return TRUE;
}
MakeHook(PO_Text2ListHook, PO_Text2List);

///
/// PO_List2TextFunc
//  Copies listview selection to text gadget
HOOKPROTONH(PO_List2TextFunc, void, Object *listview, Object *text)
{
  Object *list;

  ENTER();

  if((list = (Object *)xget(listview, MUIA_NListview_NList)) != NULL && text != NULL)
  {
    struct MUI_NListtree_TreeNode *tn = (struct MUI_NListtree_TreeNode *)xget(list, MUIA_NListtree_Active);

    if(tn != NULL && tn->tn_User != NULL)
    {
      struct FolderNode *fnode = (struct FolderNode *)tn->tn_User;
      set(text, MUIA_Text_Contents, fnode->folder->Name);
    }
  }

  LEAVE();
}
MakeHook(PO_List2TextHook, PO_List2TextFunc);

///
/// PO_XPKOpenHook
//  Sets a popup listview accordingly to its string gadget
HOOKPROTONH(PO_XPKOpenFunc, BOOL, Object *listview, Object *str)
{
  char *s;
  Object *list;

  ENTER();

  if((s = (char *)xget(str, MUIA_Text_Contents)) != NULL &&
     (list = (Object *)xget(listview, MUIA_Listview_List)) != NULL)
  {
    int i;

    for(i=0;;i++)
    {
      char *x;

      DoMethod(list, MUIM_List_GetEntry, i, &x);
      if(!x)
      {
        set(list, MUIA_List_Active, MUIV_List_Active_Off);
        break;
      }
      else if(!stricmp(x, s))
      {
        set(list, MUIA_List_Active, i);
        break;
      }
    }
  }

  RETURN(TRUE);
  return TRUE;
}
MakeStaticHook(PO_XPKOpenHook, PO_XPKOpenFunc);

///
/// PO_XPKCloseHook
//  Copies XPK sublibrary id from list to string gadget
HOOKPROTONH(PO_XPKCloseFunc, void, Object *listview, Object *text)
{
  Object *list;

  ENTER();

  if((list = (Object *)xget(listview, MUIA_Listview_List)) != NULL)
  {
    char *entry = NULL;

    DoMethod(list, MUIM_List_GetEntry, MUIV_List_GetEntry_Active, &entry);
    if(entry != NULL)
      set(text, MUIA_Text_Contents, entry);
  }

  LEAVE();
}
MakeStaticHook(PO_XPKCloseHook, PO_XPKCloseFunc);

///
/// PO_MimeTypeListOpenHook
//  Sets the popup listview accordingly to the string gadget
HOOKPROTONH(PO_MimeTypeListOpenFunc, BOOL, Object *listview, Object *str)
{
  char *s;
  Object *list;

  ENTER();

  if((s = (char *)xget(str, MUIA_String_Contents)) != NULL &&
     (list = (Object *)xget(listview, MUIA_Listview_List)) != NULL)
  {
    int i;

    // we build the list totally from ground up.
    DoMethod(list, MUIM_List_Clear);

    // populate the list with the user's own defined MIME types but only if the source
    // string isn't the one in the YAM config window.
    if(G->CO == NULL || str != G->CO->GUI.ST_CTYPE)
    {
      struct MimeTypeNode *mt;

      IterateList(&C->mimeTypeList, struct MimeTypeNode *, mt)
      {
        DoMethod(list, MUIM_List_InsertSingle, mt->ContentType, MUIV_List_Insert_Sorted);
      }
    }

    // populate the MUI list with our internal MIME types but check that
    // we don't add duplicate names
    for(i=0; IntMimeTypeArray[i].ContentType != NULL; i++)
    {
      BOOL duplicateFound = FALSE;

      if(G->CO == NULL || str != G->CO->GUI.ST_CTYPE)
      {
        struct MimeTypeNode *mt;

        IterateList(&C->mimeTypeList, struct MimeTypeNode *, mt)
        {
          if(stricmp(mt->ContentType, IntMimeTypeArray[i].ContentType) == 0)
          {
            duplicateFound = TRUE;
            break;
          }
        }
      }

      if(duplicateFound == FALSE)
        DoMethod(list, MUIM_List_InsertSingle, IntMimeTypeArray[i].ContentType, MUIV_List_Insert_Sorted);
    }

    // make sure to make the current entry active
    for(i=0;;i++)
    {
      char *c;

      DoMethod(list, MUIM_List_GetEntry, i, &c);
      if(c == NULL || s[0] == '\0')
      {
        set(list, MUIA_List_Active, MUIV_List_Active_Off);
        break;
      }
      else if(!stricmp(c, s))
      {
        set(list, MUIA_List_Active, i);
        break;
      }
    }
  }

  RETURN(TRUE);
  return TRUE;
}
MakeStaticHook(PO_MimeTypeListOpenHook, PO_MimeTypeListOpenFunc);

///
/// PO_MimeTypeListCloseHook
//  Pastes an entry from the popup listview into string gadget
HOOKPROTONH(PO_MimeTypeListCloseFunc, void, Object *listview, Object *str)
{
  Object *list;

  ENTER();

  if((list = (Object *)xget(listview, MUIA_Listview_List)) != NULL)
  {
    char *entry = NULL;

    DoMethod(list, MUIM_List_GetEntry, MUIV_List_GetEntry_Active, &entry);
    if(entry)
    {
      set(str, MUIA_String_Contents, entry);

      // in case that this close function is used with the
      // string gadget in the YAM config window we have to do a deeper search
      // as we also want to set the file extension and description gadgets
      if(G->CO != NULL && str == G->CO->GUI.ST_CTYPE)
      {
        struct CO_GUIData *gui = &G->CO->GUI;
        int i;

        for(i=0; IntMimeTypeArray[i].ContentType != NULL; i++)
        {
          struct IntMimeType *mt = (struct IntMimeType *)&IntMimeTypeArray[i];

          if(stricmp(mt->ContentType, entry) == 0)
          {
            // we also set the file extension
            if(mt->Extension)
              set(gui->ST_EXTENS, MUIA_String_Contents, mt->Extension);

            // we also set the mime description
            set(gui->ST_DESCRIPTION, MUIA_String_Contents, tr(mt->Description));

            break;
          }
        }
      }
    }
  }

  LEAVE();
}
MakeStaticHook(PO_MimeTypeListCloseHook, PO_MimeTypeListCloseFunc);

///
/// PO_HandleVarHook
//  Pastes an entry from variable listview into string gadget
HOOKPROTONH(PO_HandleVarFunc, void, Object *listview, Object *string)
{
  Object *list;

  ENTER();

  if((list = (Object *)xget(listview, MUIA_NListview_NList)) != NULL)
  {
    char *var = NULL;

    DoMethod(list, MUIM_NList_GetEntry, MUIV_NList_GetEntry_Active, &var);
    if(var != NULL)
    {
      char addstr[3];
      char *str = (char *)xget(string, MUIA_String_Contents);
      LONG pos = xget(string, MUIA_String_BufferPos);

      strlcpy(addstr, var, sizeof(addstr));

      if(str != NULL && str[0] != '\0')
      {
        int len = strlen(str)+sizeof(addstr);
        char *buf;

        if((buf = calloc(1, len)) != NULL)
        {
          // append the addstr to the right position

          if(pos > 0)
            strlcpy(buf, str, MIN(len, pos + 1));

          strlcat(buf, addstr, len);

          if(pos >= 0)
            strlcat(buf, str + pos, len);

          set(string, MUIA_String_Contents, buf);

          free(buf);
        }
      }
      else
        set(string, MUIA_String_Contents, addstr);
    }
  }

  LEAVE();
}
MakeStaticHook(PO_HandleVarHook, PO_HandleVarFunc);

///
/// PO_HandleScriptsOpenHook
// Hook which is used when the arexx/dos scripts popup window will
// be opened and populate the listview.
HOOKPROTONHNP(PO_HandleScriptsOpenFunc, BOOL, Object *listview)
{
  Object *list;
  ENTER();

  if((list = (Object *)xget(listview, MUIA_NListview_NList)) != NULL)
    DoMethod(list, MUIM_PlaceholderPopupList_SetScriptEntry, xget(G->CO->GUI.LV_REXX, MUIA_NList_Active));

  RETURN(TRUE);
  return TRUE;
}
MakeStaticHook(PO_HandleScriptsOpenHook, PO_HandleScriptsOpenFunc);

///
/// ImportMimeTypesHook
//  Imports MIME viewers from a MIME.prefs file
HOOKPROTONHNONP(ImportMimeTypesFunc, void)
{
  int mode;

  ENTER();

  if((mode = MUI_Request(G->App, G->CO->GUI.WI, MUIF_NONE, tr(MSG_CO_ImportMIME), tr(MSG_CO_ImportMIMEGads), tr(MSG_CO_ImportMIMEReq))) != 0)
  {
    struct FileReqCache *frc;

    if((frc = ReqFile(ASL_ATTACH, G->CO->GUI.WI, tr(MSG_CO_IMPORTMIMETITLE), REQF_NONE, (mode == 1 ? "ENV:" : G->MA_MailDir), (mode == 1 ? "MIME.prefs" : (mode == 2 ? "mailcap" : "mime.types")))) != NULL)
    {
      char fname[SIZE_PATHFILE];
      FILE *fh;

      AddPath(fname, frc->drawer, frc->file, sizeof(fname));
      if((fh = fopen(fname, "r")) != NULL)
      {
        Object *lv = G->CO->GUI.LV_MIME;
        char *buf = NULL;
        size_t buflen = 0;

        setvbuf(fh, NULL, _IOFBF, SIZE_FILEBUF);

        set(lv, MUIA_NList_Quiet, TRUE);

        while(getline(&buf, &buflen, fh) > 0)
        {
          struct MimeTypeNode *mt = NULL;
          struct MimeTypeNode *mtNode;
          char *ctype = buf;
          const char *ext = "";
          const char *command = "";
          char *p;
          char *p2;

          if(*ctype == '\0' || isspace(*ctype))
            continue;

          if(mode == 1)
          {
            if(*ctype == ';')
              continue;

            for(p = ctype; *p && *p != ','; ++p);

            if(*p)
            {
              for(*p = '\0', ext = ++p; *p && *p != ','; ++p);

              if(*p)
              {
                for(*p++ = '\0'; *p && *p != ','; ++p);

                if(*p)
                {
                  for(command = ++p; *p && *p != ','; ++p);

                  *p = '\0';
                }
              }
            }
          }
          else if (mode == 2)
          {
            if(*ctype == '#')
              continue;

            for(p2 = p = ctype; !isspace(*p) && *p && *p != ';'; p2 = ++p);

            if((p = strpbrk(p,";")))
              ++p;

            if(p)
              command = TrimStart(p);

            *p2 = '\0';
          }
          else
          {
            if(*ctype == '#')
              continue;

            for(p2 = p = ctype; !isspace(*p) && *p; p2 = ++p);

            if(*p)
              ext = TrimStart(p);

            *p2 = '\0';
          }

          // now we try to find the content-type in our mimeTypeList
          IterateList(&C->mimeTypeList, struct MimeTypeNode *, mtNode)
          {
            if(stricmp(mtNode->ContentType, ctype) == 0)
            {
              mt = mtNode;
              break;
            }
          }

          // if we couldn't find it in our list we have to create a new mimeTypeNode
          // and put it into our list.
          if(mt == NULL && (mt = CreateNewMimeType()) != NULL)
          {
            // add the new mime type to our internal list of
            // user definable MIME types.
            AddTail((struct List *)&(C->mimeTypeList), (struct Node *)mt);

            // add the new MimeType also to the config page.
            DoMethod(lv, MUIM_NList_InsertSingle, mt, MUIV_NList_Insert_Bottom);
          }

          // if we have a valid mimeTypeNode now we can fill it with valid data
          if(mt != NULL)
          {
            strlcpy(mt->ContentType, ctype, sizeof(mt->ContentType));
            strlcpy(mt->Command, command, sizeof(mt->Command));
            strlcpy(mt->Extension, ext, sizeof(mt->Extension));

            // replace any '%f' in the command string by '%s'
            while((p = strstr(mt->Command, "%f")) != NULL)
              p[1] = 's';
          }
        }

        fclose(fh);

        free(buf);

        set(lv, MUIA_NList_Quiet, FALSE);
        DoMethod(lv, MUIM_NList_Redraw, MUIV_NList_Redraw_All);
      }
      else
        ER_NewError(tr(MSG_ER_CantOpenFile), fname);
    }
  }

  LEAVE();
}
MakeStaticHook(ImportMimeTypesHook, ImportMimeTypesFunc);

///
/// CO_PlaySoundFunc
//  Plays sound file referred by the string gadget
HOOKPROTONHNO(CO_PlaySoundFunc, void, int *arg)
{
  char *soundFile;

  ENTER();

  soundFile = (char *)xget((Object *)arg[0], MUIA_String_Contents);
  if(soundFile != NULL && soundFile[0] != '\0')
    PlaySound(soundFile);

  LEAVE();
}
MakeHook(CO_PlaySoundHook,CO_PlaySoundFunc);

///
/// UpdateInfoHook
// update the information text about the update check
HOOKPROTONHNO(UpdateInfoFunc, void, ULONG *interval)
{
  ENTER();

  if(interval[0] == 0)
    set(G->CO->GUI.TX_UPDATEINFO, MUIA_Text_Contents, tr(MSG_CO_UPDATES_INFO_MANUAL));
  else
    set(G->CO->GUI.TX_UPDATEINFO, MUIA_Text_Contents, tr(MSG_CO_UPDATES_INFO_AUTO));

  LEAVE();
}
MakeStaticHook(UpdateInfoHook, UpdateInfoFunc);

///
/// UpdateCheckHook
// initiates an interactive update check
HOOKPROTONHNONP(UpdateCheckFunc, void)
{
  ENTER();

  // get the configuration settings
  if(G->CO->VisiblePage == cp_Update)
    CO_GetConfig();

  // now we make sure the C and CE config structure is in sync again
  C->UpdateInterval = CE->UpdateInterval;

  // let the application check for updates
  DoMethod(G->App, MUIM_YAMApplication_UpdateCheck, FALSE);

  LEAVE();
}
MakeStaticHook(UpdateCheckHook, UpdateCheckFunc);

///
/// AddMimeTypeHook
//  Adds a new MIME type structure to the internal list
HOOKPROTONHNONP(AddMimeTypeFunc, void)
{
  struct CO_GUIData *gui = &G->CO->GUI;
  struct MimeTypeNode *mt;

  ENTER();

  if((mt = CreateNewMimeType()) != NULL)
  {
    // add the new mime type to our internal list of
    // user definable MIME types.
    AddTail((struct List *)&(CE->mimeTypeList), (struct Node *)mt);

    // add the new MimeType also to the config page.
    DoMethod(gui->LV_MIME, MUIM_NList_InsertSingle, mt, MUIV_NList_Insert_Bottom);

    // make sure the new entry is the active entry and that the list
    // is also the active gadget in the window.
    set(gui->LV_MIME, MUIA_NList_Active, MUIV_NList_Active_Bottom);
    set(gui->WI, MUIA_Window_ActiveObject, gui->ST_CTYPE);
  }

  LEAVE();
}
MakeStaticHook(AddMimeTypeHook, AddMimeTypeFunc);

///
/// DelMimeTypeHook
//  Deletes an entry from the MIME type list.
HOOKPROTONHNONP(DelMimeTypeFunc, void)
{
  struct CO_GUIData *gui = &G->CO->GUI;
  int pos;

  ENTER();

  if((pos = xget(gui->LV_MIME, MUIA_NList_Active)) != MUIV_NList_Active_Off)
  {
    struct MimeTypeNode *mt;

    DoMethod(gui->LV_MIME, MUIM_NList_GetEntry, pos, &mt);
    if(mt != NULL)
    {
      // remove from MUI list
      DoMethod(gui->LV_MIME, MUIM_NList_Remove, pos);

      // remove from internal list
      Remove((struct Node *)mt);

      // free memory.
      FreeSysObject(ASOT_NODE, mt);
    }
  }

  LEAVE();
}
MakeStaticHook(DelMimeTypeHook, DelMimeTypeFunc);

///
/// GetMimeTypeEntryHook
//  Fills form with data from selected list entry
HOOKPROTONHNONP(GetMimeTypeEntryFunc, void)
{
  struct MimeTypeNode *mt;
  struct CO_GUIData *gui = &G->CO->GUI;

  ENTER();

  DoMethod(gui->LV_MIME, MUIM_NList_GetEntry, MUIV_NList_GetEntry_Active, &mt);
  if(mt != NULL)
  {
    nnset(gui->ST_CTYPE, MUIA_String_Contents, mt->ContentType);
    nnset(gui->ST_EXTENS, MUIA_String_Contents, mt->Extension);
    nnset(gui->ST_COMMAND, MUIA_String_Contents, mt->Command);
    nnset(gui->ST_DESCRIPTION, MUIA_String_Contents, mt->Description);
  }

  set(gui->GR_MIME, MUIA_Disabled, mt == NULL);
  set(gui->BT_MDEL, MUIA_Disabled, mt == NULL);

  LEAVE();
}
MakeStaticHook(GetMimeTypeEntryHook, GetMimeTypeEntryFunc);

///
/// PutMimeTypeEntryHook
//  Fills form data into selected list entry
HOOKPROTONHNONP(PutMimeTypeEntryFunc, void)
{
  struct MimeTypeNode *mt;
  struct CO_GUIData *gui = &G->CO->GUI;

  ENTER();

  DoMethod(gui->LV_MIME, MUIM_NList_GetEntry, MUIV_NList_GetEntry_Active, &mt);
  if(mt != NULL)
  {
    GetMUIString(mt->ContentType, gui->ST_CTYPE, sizeof(mt->ContentType));
    GetMUIString(mt->Extension, gui->ST_EXTENS, sizeof(mt->Extension));
    GetMUIString(mt->Command, gui->ST_COMMAND, sizeof(mt->Command));
    GetMUIString(mt->Description, gui->ST_DESCRIPTION, sizeof(mt->Description));

    DoMethod(gui->LV_MIME, MUIM_NList_Redraw, MUIV_NList_Redraw_Active);
  }

  LEAVE();
}
MakeStaticHook(PutMimeTypeEntryHook, PutMimeTypeEntryFunc);
///
/// CO_GetRXEntryHook
//  Fills form with data from selected list entry
HOOKPROTONHNONP(CO_GetRXEntryFunc, void)
{
  struct CO_GUIData *gui = &G->CO->GUI;
  struct RxHook *rh;
  int act = xget(gui->LV_REXX, MUIA_NList_Active);
  enum Macro macro = (enum Macro)act;

  ENTER();

  rh = &(CE->RX[act]);
  nnset(gui->ST_RXNAME, MUIA_String_Contents, act < 10 ? rh->Name : "");
  nnset(gui->ST_SCRIPT, MUIA_String_Contents, rh->Script);
  nnset(gui->CY_ISADOS, MUIA_Cycle_Active, rh->IsAmigaDOS ? 1 : 0);
  nnset(gui->CH_CONSOLE, MUIA_Selected, rh->UseConsole);
  nnset(gui->CH_WAITTERM, MUIA_Selected, rh->WaitTerm);
  set(gui->ST_RXNAME, MUIA_Disabled, act >= 10);

  switch(macro)
  {
    case MACRO_MEN0:
    case MACRO_MEN1:
    case MACRO_MEN2:
    case MACRO_MEN3:
    case MACRO_MEN4:
    case MACRO_MEN5:
    case MACRO_MEN6:
    case MACRO_MEN7:
    case MACRO_MEN8:
    case MACRO_MEN9:
    case MACRO_STARTUP:
    case MACRO_QUIT:
    case MACRO_PRESEND:
    case MACRO_POSTSEND:
    case MACRO_PREFILTER:
    case MACRO_POSTFILTER:
    default:
      // disable the popup button since these script don't take any parameter
      nnset(gui->PO_SCRIPT, MUIA_Disabled, TRUE);
    break;

    case MACRO_PREGET:
    case MACRO_POSTGET:
    case MACRO_NEWMSG:
    case MACRO_READ:
    case MACRO_PREWRITE:
    case MACRO_POSTWRITE:
    case MACRO_URL:
      // enable the popup button
      nnset(gui->PO_SCRIPT, MUIA_Disabled, FALSE);
    break;
  }

  DoMethod(gui->LV_REXX, MUIM_NList_Redraw, act);

  LEAVE();
}
MakeStaticHook(CO_GetRXEntryHook, CO_GetRXEntryFunc);

///
/// CO_PutRXEntryHook
//  Fills form data into selected list entry
HOOKPROTONHNONP(CO_PutRXEntryFunc, void)
{
  struct CO_GUIData *gui = &G->CO->GUI;
  int act = xget(gui->LV_REXX, MUIA_NList_Active);

  ENTER();

  if(act != MUIV_List_Active_Off)
  {
    struct RxHook *rh = &(CE->RX[act]);

    GetMUIString(rh->Name, gui->ST_RXNAME, sizeof(rh->Name));
    GetMUIString(rh->Script, gui->ST_SCRIPT, sizeof(rh->Script));
    rh->IsAmigaDOS = GetMUICycle(gui->CY_ISADOS) == 1;
    rh->UseConsole = GetMUICheck(gui->CH_CONSOLE);
    rh->WaitTerm = GetMUICheck(gui->CH_WAITTERM);

    DoMethod(gui->LV_REXX, MUIM_NList_Redraw, act);
  }

  LEAVE();
}
MakeStaticHook(CO_PutRXEntryHook, CO_PutRXEntryFunc);
///
/// FileRequestStartFunc
//  Will be executed as soon as the user wants to popup a file requester
//  for selecting files
HOOKPROTONO(FileRequestStartFunc, BOOL, struct TagItem *tags)
{
  char *str;
  Object *strObj;

  ENTER();

  switch((enum PlaceholderMode)hook->h_Data)
  {
    case PHM_SCRIPTS:
      strObj = G->CO->GUI.ST_SCRIPT;
    break;

    case PHM_MIME_DEFVIEWER:
      strObj = G->CO->GUI.ST_DEFVIEWER;
    break;

    case PHM_MIME_COMMAND:
      strObj = G->CO->GUI.ST_COMMAND;
    break;

    default:
      RETURN(FALSE);
      return FALSE;
  }

  str = (char *)xget(strObj, MUIA_String_Contents);
  if(str != NULL && str[0] != '\0')
  {
    int i=0;
    static char buf[SIZE_PATHFILE];
    char *p;

    // make sure the string is unquoted.
    strlcpy(buf, str, sizeof(buf));
    UnquoteString(buf, FALSE);

    if((p = PathPart(buf)))
    {
      static char drawer[SIZE_PATHFILE];

      strlcpy(drawer, buf, MIN(sizeof(drawer), (unsigned int)(p - buf + 1)));

      tags[i].ti_Tag = ASLFR_InitialDrawer;
      tags[i].ti_Data= (ULONG)drawer;
      i++;
    }

    tags[i].ti_Tag = ASLFR_InitialFile;
    tags[i].ti_Data= (ULONG)FilePart(buf);
    i++;

    tags[i].ti_Tag = TAG_DONE;
  }

  RETURN(TRUE);
  return TRUE;
}
MakeHookWithData(ScriptsReqStartHook,       FileRequestStartFunc, PHM_SCRIPTS);
MakeHookWithData(MimeDefViewerReqStartHook, FileRequestStartFunc, PHM_MIME_DEFVIEWER);
MakeHookWithData(MimeCommandReqStartHook,   FileRequestStartFunc, PHM_MIME_COMMAND);

///
/// FileRequestStopFunc
//  Will be executed as soon as the user selected a file
HOOKPROTONO(FileRequestStopFunc, void, struct FileRequester *fileReq)
{
  Object *strObj;

  ENTER();

  switch((enum PlaceholderMode)hook->h_Data)
  {
    case PHM_SCRIPTS:
      strObj = G->CO->GUI.ST_SCRIPT;
    break;

    case PHM_MIME_DEFVIEWER:
      strObj = G->CO->GUI.ST_DEFVIEWER;
    break;

    case PHM_MIME_COMMAND:
      strObj = G->CO->GUI.ST_COMMAND;
    break;

    default:
      LEAVE();
      return;
  }

  // check if a file was selected or not
  if(fileReq->fr_File != NULL &&
     fileReq->fr_File[0] != '\0')
  {
    char buf[SIZE_PATHFILE];

    AddPath(buf, fileReq->fr_Drawer, fileReq->fr_File, sizeof(buf));

    // check if there is any space in our path
    if(strchr(buf, ' ') != NULL)
    {
      int len = strlen(buf);

      memmove(&buf[1], buf, len+1);
      buf[0] = '"';
      buf[len+1] = '"';
      buf[len+2] = '\0';
    }

    set(strObj, MUIA_String_Contents, buf);
  }

  LEAVE();
}
MakeHookWithData(ScriptsReqStopHook,       FileRequestStopFunc, PHM_SCRIPTS);
MakeHookWithData(MimeDefViewerReqStopHook, FileRequestStopFunc, PHM_MIME_DEFVIEWER);
MakeHookWithData(MimeCommandReqStopHook,   FileRequestStopFunc, PHM_MIME_COMMAND);

///
/// AddNewFilterToList
//  Adds a new entry to the global filter list
HOOKPROTONHNONP(AddNewFilterToList, void)
{
  struct FilterNode *filterNode;

  if((filterNode = CreateNewFilter(FA_TERMINATE, 0)) != NULL)
  {
    DoMethod(G->CO->GUI.LV_RULES, MUIM_NList_InsertSingle, filterNode, MUIV_NList_Insert_Bottom);
    set(G->CO->GUI.LV_RULES, MUIA_NList_Active, MUIV_NList_Active_Bottom);

    // lets set the new string gadget active and select all text in there automatically to
    // be more handy to the user ;)
    set(_win(G->CO->GUI.LV_RULES), MUIA_Window_ActiveObject, G->CO->GUI.ST_RNAME);
    set(G->CO->GUI.ST_RNAME, MUIA_BetterString_SelectSize, -((LONG)strlen(filterNode->name)));

    // now add the filterNode to our global filterList
    AddTail((struct List *)&CE->filterList, (struct Node *)filterNode);
  }
}
MakeStaticHook(AddNewFilterToListHook, AddNewFilterToList);

///
/// RemoveActiveFilter
//  Deletes the active filter entry from the filter list
HOOKPROTONHNONP(RemoveActiveFilter, void)
{
  struct FilterNode *filterNode = NULL;

  // get the active filterNode
  DoMethod(G->CO->GUI.LV_RULES, MUIM_NList_GetEntry, MUIV_NList_GetEntry_Active, &filterNode);

  // if we got an active entry lets remove it from the GUI List
  // and also from our own global filterList
  if(filterNode != NULL)
  {
    DoMethod(G->CO->GUI.LV_RULES, MUIM_NList_Remove, MUIV_NList_Remove_Active);

    Remove((struct Node *)filterNode);
    DeleteFilterNode(filterNode);
  }
}
MakeStaticHook(RemoveActiveFilterHook, RemoveActiveFilter);

///
/// CO_AddSignature
//  Adds a new entry to the signature list
HOOKPROTONHNONP(CO_AddSignature, void)
{
  struct SignatureNode *sn;

  ENTER();

  if((sn = CreateNewSignature()) != NULL)
  {
    // create new default values
    strlcpy(sn->description, tr(MSG_NewEntry), sizeof(sn->description));

    // new signatures don't use a file by default
    sn->useSignatureFile = FALSE;

    // add the new signature to the list
    DoMethod(G->CO->GUI.LV_SIGNATURE, MUIM_NList_InsertSingle, sn, MUIV_NList_Insert_Bottom);

    // add the signature to the list
    AddTail((struct List *)&CE->signatureList, (struct Node *)sn);

    // set the new entry active and make sure that the email gadget will be
    // set as the new active object of the window as that gadget will be used
    // to automatically set the account name.
    set(G->CO->GUI.LV_SIGNATURE, MUIA_NList_Active, MUIV_List_Active_Bottom);
    set(G->CO->GUI.WI, MUIA_Window_ActiveObject, G->CO->GUI.ST_SIG_DESC);
  }

  LEAVE();
}
MakeStaticHook(CO_AddSignatureHook, CO_AddSignature);

///
/// CO_DelSignature
//  Deletes an entry from the signature list
HOOKPROTONHNONP(CO_DelSignature, void)
{
  struct CO_GUIData *gui = &G->CO->GUI;
  struct SignatureNode *sn = NULL;

  ENTER();

  DoMethod(gui->LV_SIGNATURE, MUIM_NList_GetEntry, MUIV_NList_GetEntry_Active, &sn);

  if(sn != NULL &&
     xget(gui->LV_SIGNATURE, MUIA_NList_Entries) > 1)
  {
    DoMethod(gui->LV_SIGNATURE, MUIM_NList_Remove, MUIV_NList_Remove_Active);

    // remove it from the internal user identity list as well.
    Remove((struct Node *)sn);

    FreeSysObject(ASOT_NODE, sn);
  }

  LEAVE();
}
MakeStaticHook(CO_DelSignatureHook, CO_DelSignature);

///
/// GetAppIconPos
// Retrieves the position x/y of the AppIcon and
// sets the position label accordingly
HOOKPROTONHNONP(GetAppIconPos, void)
{
  struct DiskObject *dobj;

  ENTER();

  if((dobj = G->theme.icons[G->currentAppIcon]) != NULL)
  {
    struct CO_GUIData *gui = &G->CO->GUI;

    // set the position
    set(gui->ST_APPX, MUIA_String_Integer, dobj->do_CurrentX);
    set(gui->ST_APPY, MUIA_String_Integer, dobj->do_CurrentY);

    // enable the checkbox
    setcheckmark(gui->CH_APPICONPOS, TRUE);
  }

  LEAVE();
}
MakeStaticHook(GetAppIconPosHook, GetAppIconPos);

///
/// InfoBarPosFunc
// update the InfoBar contents string gadget according to the position setting
HOOKPROTONHNO(InfoBarPosFunc, void, int *arg)
{
  struct CO_GUIData *gui = &G->CO->GUI;
  BOOL inactive = (arg[0] == IB_POS_OFF);

  ENTER();

  // disabling the Popstring object completely doesn't work, because on reactivation the string
  // gadget is not redrawn correctly (bug in MUI?), hence we do it separately.
  nnset(gui->ST_INFOBARTXT, MUIA_Disabled, inactive == TRUE);
  nnset((Object *)xget(gui->PO_INFOBARTXT, MUIA_Popstring_Button), MUIA_Disabled, inactive == TRUE);

  LEAVE();
}
MakeStaticHook(InfoBarPosHook, InfoBarPosFunc);
///

/*** Special object creation functions ***/
/// MakeXPKPop
//  Creates a popup list of available XPK sublibraries
static Object *MakeXPKPop(Object **text, BOOL encrypt)
{
  Object *lv;
  Object *list;
  Object *po;
  Object *but;

  ENTER();

  if((po = PopobjectObject,
    MUIA_Popstring_String, *text = TextObject,
      TextFrame,
      MUIA_Background, MUII_TextBack,
      MUIA_FixWidthTxt, "MMMM",
    End,
    MUIA_Popstring_Button, but = PopButton(MUII_PopUp),
    MUIA_Popobject_StrObjHook, &PO_XPKOpenHook,
    MUIA_Popobject_ObjStrHook, &PO_XPKCloseHook,
    MUIA_Popobject_WindowHook, &PO_WindowHook,
    MUIA_Popobject_Object, lv = ListviewObject,
      MUIA_Listview_List, list = ListObject,
        InputListFrame,
        MUIA_List_AutoVisible,   TRUE,
        MUIA_List_ConstructHook, MUIV_List_ConstructHook_String,
        MUIA_List_DestructHook,  MUIV_List_DestructHook_String,
      End,
    End,
  End))
  {
    // disable the XPK popups if xpkmaster.library is not available
    if(XpkBase == NULL)
    {
      set(po, MUIA_Disabled, TRUE);
      set(but, MUIA_Disabled, TRUE);
    }
    else
    {
      struct xpkPackerNode *xpkNode;

      IterateList(G->xpkPackerList, struct xpkPackerNode *, xpkNode)
      {
        BOOL suits = TRUE;

        D(DBF_XPK, "XPK lib '%s' has flags %08lx", xpkNode->info.xpi_Name, xpkNode->info.xpi_Flags);

        if(encrypt == TRUE && isFlagClear(xpkNode->info.xpi_Flags, XPKIF_ENCRYPTION))
        {
          D(DBF_XPK, "'%s' has no encryption capabilities, excluded from encryption list", xpkNode->info.xpi_Name);
          suits = FALSE;
        }

        if(suits == TRUE)
          DoMethod(list, MUIM_List_InsertSingle, xpkNode->info.xpi_Name, MUIV_List_Insert_Sorted);
      }

      DoMethod(lv, MUIM_Notify, MUIA_Listview_DoubleClick, TRUE, po, 2, MUIM_Popstring_Close, TRUE);
    }
  }

  RETURN(po);
  return po;
}

///
/// MakeMimeTypePop
//  Creates a popup list of available internal MIME types
Object *MakeMimeTypePop(Object **string, const char *desc)
{
  Object *lv;
  Object *po;
  Object *bt;

  ENTER();

  if((po = PopobjectObject,

    MUIA_Popstring_String, *string = BetterStringObject,
      StringFrame,
      MUIA_String_Accept,      "0123456789ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz+-/#?*",
      MUIA_String_MaxLen,      SIZE_CTYPE,
      MUIA_ControlChar,        ShortCut(desc),
      MUIA_String_AdvanceOnCR, TRUE,
      MUIA_CycleChain,         TRUE,
    End,
    MUIA_Popstring_Button, bt = PopButton(MUII_PopUp),
    MUIA_Popobject_StrObjHook, &PO_MimeTypeListOpenHook,
    MUIA_Popobject_ObjStrHook, &PO_MimeTypeListCloseHook,
    MUIA_Popobject_WindowHook, &PO_WindowHook,
    MUIA_Popobject_Object, lv = ListviewObject,
       MUIA_Listview_ScrollerPos, MUIV_Listview_ScrollerPos_Right,
       MUIA_Listview_List, ListObject,
          InputListFrame,
          MUIA_List_AutoVisible, TRUE,
          MUIA_List_ConstructHook, MUIV_List_ConstructHook_String,
          MUIA_List_DestructHook,  MUIV_List_DestructHook_String,
       End,
    End,

  End))
  {
    set(bt, MUIA_CycleChain,TRUE);
    DoMethod(lv, MUIM_Notify, MUIA_Listview_DoubleClick, TRUE, po, 2, MUIM_Popstring_Close, TRUE);
    DoMethod(*string, MUIM_Notify, MUIA_Disabled, MUIV_EveryTime, po, 3, MUIM_Set, MUIA_Disabled, MUIV_TriggerValue);
  }
  else
    *string = NULL;

  RETURN(po);
  return po;
}

///
/// MakeVarPop
//  Creates a popup list containing variables and descriptions for phrases etc.
static Object *MakeVarPop(Object **string, Object **popButton, const enum PlaceholderMode mode, const int size, const char *shortcut)
{
  Object *list;
  Object *po;

  ENTER();

  if((po = PopobjectObject,

    MUIA_Popstring_String, *string = MakeString(size, shortcut),
    MUIA_Popstring_Button, *popButton = PopButton(MUII_PopUp),
    MUIA_Popobject_ObjStrHook, &PO_HandleVarHook,
    MUIA_Popobject_WindowHook, &PO_WindowHook,
    MUIA_Popobject_StrObjHook, (mode == PHM_SCRIPTS) ? &PO_HandleScriptsOpenHook : NULL,
    MUIA_Popobject_Object, NListviewObject,
      MUIA_FixHeightTxt, "\n\n\n\n\n\n\n\n",
      MUIA_NListview_Horiz_ScrollBar, MUIV_NListview_HSB_None,
      MUIA_NListview_Vert_ScrollBar, MUIV_NListview_VSB_FullAuto,
      MUIA_NListview_NList, list = PlaceholderPopupListObject,
        MUIA_PlaceholderPopupList_Mode, mode,
      End,
    End,

  End))
  {
    DoMethod(list, MUIM_Notify, MUIA_NList_DoubleClick, TRUE, po, 2, MUIM_Popstring_Close, TRUE);
    DoMethod(*string, MUIM_Notify, MUIA_Disabled, MUIV_EveryTime, po, 3, MUIM_Set, MUIA_Disabled, MUIV_TriggerValue);
  }

  RETURN(po);
  return po;
}

///
/// MakePhraseGroup
//  Creates a cycle/string gadgets for forward and reply phrases
static Object *MakePhraseGroup(Object **hello, Object **intro, Object **bye,
                               const char *label, const char *help)
{
   Object *grp, *cycl, *pgrp;
   Object *popButton;
   static const char *cytext[4];

   cytext[0] = tr(MSG_CO_PhraseOpen);
   cytext[1] = tr(MSG_CO_PhraseIntro);
   cytext[2] = tr(MSG_CO_PhraseClose);
   cytext[3] = NULL;

   if ((grp = HGroup,
         MUIA_Group_HorizSpacing, 1,
         Child, cycl = CycleObject,
            MUIA_CycleChain, 1,
            MUIA_Font, MUIV_Font_Button,
            MUIA_Cycle_Entries, cytext,
            MUIA_ControlChar, ShortCut(label),
            MUIA_Weight, 0,
         End,
         Child, pgrp = PageGroup,
            Child, MakeVarPop(hello, &popButton, PHM_REPLYHELLO, SIZE_INTRO, ""),
            Child, MakeVarPop(intro, &popButton, PHM_REPLYINTRO, SIZE_INTRO, ""),
            Child, MakeVarPop(bye,   &popButton, PHM_REPLYBYE,   SIZE_INTRO, ""),
         End,
         MUIA_ShortHelp, help,
      End))
   {
      DoMethod(cycl, MUIM_Notify, MUIA_Cycle_Active, MUIV_EveryTime, pgrp, 3, MUIM_Set, MUIA_Group_ActivePage, MUIV_TriggerValue);
   }
   return grp;
}

///
/// MakeStaticCheck
//  Creates non-interactive checkmark gadget
static Object *MakeStaticCheck(void)
{
   return
   ImageObject,
      ImageButtonFrame,
      MUIA_Image_Spec  , MUII_CheckMark,
      MUIA_Background  , MUII_ButtonBack,
      MUIA_ShowSelState, FALSE,
      MUIA_Selected    , TRUE,
      MUIA_Disabled    , TRUE,
   End;
}

///

/*** Pages ***/
/// CO_PageFilters
Object *CO_PageFilters(struct CO_ClassData *data)
{
  static const char *rtitles[4];
  static const char *conditions[4];
  Object *grp;
  Object *bt_moveto;

  ENTER();

  rtitles[0] = tr(MSG_CO_FILTER_REGISTER_SETTINGS);
  rtitles[1] = tr(MSG_CO_FILTER_REGISTER_CONDITIONS);
  rtitles[2] = tr(MSG_CO_FILTER_REGISTER_ACTIONS);
  rtitles[3] = NULL;

  conditions[0] = tr(MSG_CO_CONDITION_ALL);
  conditions[1] = tr(MSG_CO_CONDITION_MIN_ONE);
  conditions[2] = tr(MSG_CO_CONDITION_MAX_ONE);
  conditions[3] = NULL;

  if((grp = VGroup,
    MUIA_HelpNode, "Configuration#Filters",

    ConfigPageHeaderObject("config_filters_big", G->theme.configImages[CI_FILTERSBIG], tr(MSG_CO_FILTER_TITLE), tr(MSG_CO_FILTER_SUMMARY)),

    Child, HGroup,
      GroupSpacing(0),
      Child, VGroup,
        MUIA_HorizWeight, 40,
        Child, NListviewObject,
          MUIA_CycleChain, TRUE,
          MUIA_NListview_NList, data->GUI.LV_RULES = FilterListObject,
          End,
        End,
        Child, HGroup,
          Child, ColGroup(2),
            MUIA_Group_Spacing, 1,
            MUIA_Group_SameWidth, TRUE,
            MUIA_Weight, 1,
            Child, data->GUI.BT_RADD = MakeButton(MUIX_B "+" MUIX_N),
            Child, data->GUI.BT_RDEL = MakeButton(MUIX_B "-" MUIX_N),
          End,
          Child, HSpace(0),
          Child, ColGroup(2),
            MUIA_Group_Spacing, 1,
            MUIA_Group_SameWidth, TRUE,
            Child, data->GUI.BT_FILTERUP = PopButton(MUII_ArrowUp),
            Child, data->GUI.BT_FILTERDOWN = PopButton(MUII_ArrowDown),
          End,
        End,
        Child, data->GUI.BT_FILTER_IMPORT = MakeButton(tr(MSG_CO_FILTER_IMPORT)),
      End,
      Child, NBalanceObject,
         MUIA_Balance_Quiet, TRUE,
      End,
      Child, RegisterGroup(rtitles),
        MUIA_CycleChain, TRUE,

        // general settings
        Child, ScrollgroupObject,
          MUIA_Scrollgroup_FreeHoriz, FALSE,
          MUIA_Scrollgroup_AutoBars,  TRUE,
          MUIA_Scrollgroup_Contents,  VGroupV,

            Child, ColGroup(2),
              Child, Label2(tr(MSG_CO_Name)),
              Child, data->GUI.ST_RNAME = MakeString(SIZE_NAME,tr(MSG_CO_Name)),

              Child, HSpace(1),
              Child, MakeCheckGroup(&data->GUI.CH_REMOTE, tr(MSG_CO_Remote)),

              Child, HSpace(1),
              Child, MakeCheckGroup(&data->GUI.CH_APPLYNEW, tr(MSG_CO_ApplyToNew)),

              Child, HSpace(1),
              Child, MakeCheckGroup(&data->GUI.CH_APPLYSENT, tr(MSG_CO_ApplyToSent)),

              Child, HSpace(1),
              Child, MakeCheckGroup(&data->GUI.CH_APPLYREQ, tr(MSG_CO_ApplyOnReq)),

              Child, HVSpace,
              Child, HVSpace,
            End,
          End,
        End,

        // conditions
        Child, VGroup,
          Child, HGroup,
            Child, Label2(tr(MSG_CO_CONDITION_PREPHRASE)),
            Child, data->GUI.CY_FILTER_COMBINE = MakeCycle(conditions, ""),
            Child, Label1(tr(MSG_CO_CONDITION_POSTPHRASE)),
            Child, HVSpace,
          End,
          Child, data->GUI.GR_SGROUP = FilterRuleListObject,
          End,
        End,

        // actions
        Child, ScrollgroupObject,
          MUIA_Scrollgroup_FreeHoriz, FALSE,
          MUIA_Scrollgroup_AutoBars,  TRUE,
          MUIA_Scrollgroup_Contents,  VGroupV,

            Child, ColGroup(3),
              Child, data->GUI.CH_AREDIRECT = MakeCheck(tr(MSG_CO_ACTIONREDIRECT)),
              Child, LLabel2(tr(MSG_CO_ACTIONREDIRECT)),
              Child, MakeAddressField(&data->GUI.ST_AREDIRECT, "", MSG_HELP_CO_ST_AREDIRECT, ABM_CONFIG, -1, AFF_ALLOW_MULTI),
              Child, data->GUI.CH_AFORWARD = MakeCheck(tr(MSG_CO_ActionForward)),
              Child, LLabel2(tr(MSG_CO_ActionForward)),
              Child, MakeAddressField(&data->GUI.ST_AFORWARD, "", MSG_HELP_CO_ST_AFORWARD, ABM_CONFIG, -1, AFF_ALLOW_MULTI),
              Child, data->GUI.CH_ARESPONSE = MakeCheck(tr(MSG_CO_ActionReply)),
              Child, LLabel2(tr(MSG_CO_ActionReply)),
              Child, data->GUI.PO_ARESPONSE = PopaslObject,
                MUIA_Popasl_Type,      ASL_FileRequest,
                MUIA_Popstring_String, data->GUI.ST_ARESPONSE = MakeString(SIZE_PATHFILE, ""),
                MUIA_Popstring_Button, PopButton(MUII_PopFile),
              End,
              Child, data->GUI.CH_AEXECUTE = MakeCheck(tr(MSG_CO_ActionExecute)),
              Child, LLabel2(tr(MSG_CO_ActionExecute)),
              Child, data->GUI.PO_AEXECUTE = PopaslObject,
                MUIA_Popasl_Type,      ASL_FileRequest,
                MUIA_Popstring_String, data->GUI.ST_AEXECUTE = MakeString(SIZE_PATHFILE, ""),
                MUIA_Popstring_Button, PopButton(MUII_PopFile),
              End,
              Child, data->GUI.CH_APLAY = MakeCheck(tr(MSG_CO_ActionPlay)),
              Child, LLabel2(tr(MSG_CO_ActionPlay)),
              Child, HGroup,
                MUIA_Group_HorizSpacing, 0,
                Child, data->GUI.PO_APLAY = PopaslObject,
                  MUIA_Popasl_Type,      ASL_FileRequest,
                  MUIA_Popstring_String, data->GUI.ST_APLAY = MakeString(SIZE_PATHFILE, ""),
                  MUIA_Popstring_Button, PopButton(MUII_PopFile),
                End,
                Child, data->GUI.BT_APLAY = PopButton(MUII_TapePlay),
              End,
              Child, data->GUI.CH_AMOVE = MakeCheck(tr(MSG_CO_ActionMove)),
              Child, LLabel2(tr(MSG_CO_ActionMove)),
              Child, data->GUI.PO_MOVETO = PopobjectObject,
                MUIA_Popstring_String, data->GUI.TX_MOVETO = TextObject,
                  TextFrame,
                  MUIA_Text_Copy, FALSE,
                End,
                MUIA_Popstring_Button,bt_moveto = PopButton(MUII_PopUp),
                MUIA_Popobject_StrObjHook, &PO_Text2ListHook,
                MUIA_Popobject_ObjStrHook, &PO_List2TextHook,
                MUIA_Popobject_WindowHook, &PO_WindowHook,
                MUIA_Popobject_Object, NListviewObject,
                  MUIA_NListview_NList, data->GUI.LV_MOVETO = FolderRequestListtreeObject,
                    MUIA_NList_DoubleClick, TRUE,
                  End,
                End,
              End,
            End,
            Child, MakeCheckGroup(&data->GUI.CH_ASTATUSTOMARKED, tr(MSG_CO_ACTION_SET_STATUS_TO_MARKED)),
            Child, MakeCheckGroup(&data->GUI.CH_ASTATUSTOUNMARKED, tr(MSG_CO_ACTION_SET_STATUS_TO_UNMARKED)),
            Child, MakeCheckGroup(&data->GUI.CH_ASTATUSTOREAD, tr(MSG_CO_ACTION_SET_STATUS_TO_READ)),
            Child, MakeCheckGroup(&data->GUI.CH_ASTATUSTOUNREAD, tr(MSG_CO_ACTION_SET_STATUS_TO_UNREAD)),
            Child, MakeCheckGroup(&data->GUI.CH_ASTATUSTOSPAM, tr(MSG_CO_ACTION_SET_STATUS_TO_SPAM)),
            Child, MakeCheckGroup(&data->GUI.CH_ASTATUSTOHAM, tr(MSG_CO_ACTION_SET_STATUS_TO_HAM)),
            Child, MakeCheckGroup(&data->GUI.CH_ADELETE, tr(MSG_CO_ActionDelete)),
            Child, MakeCheckGroup(&data->GUI.CH_ASKIP, tr(MSG_CO_ActionSkip)),
            Child, MakeCheckGroup(&data->GUI.CH_ATERMINATE, tr(MSG_CO_ACTION_TERMINATE_FILTER)),
            Child, HVSpace,
          End,
        End,
      End,
    End,

  End))
  {
    SetHelp(data->GUI.LV_RULES,             MSG_HELP_CO_LV_RULES);
    SetHelp(data->GUI.ST_RNAME,             MSG_HELP_CO_ST_RNAME);
    SetHelp(data->GUI.CH_REMOTE,            MSG_HELP_CO_CH_REMOTE);
    SetHelp(data->GUI.CH_APPLYNEW,          MSG_HELP_CO_CH_APPLYNEW);
    SetHelp(data->GUI.CH_APPLYSENT,         MSG_HELP_CO_CH_APPLYSENT);
    SetHelp(data->GUI.CH_APPLYREQ,          MSG_HELP_CO_CH_APPLYREQ);
    SetHelp(data->GUI.CH_AREDIRECT,         MSG_HELP_CO_CH_AREDIRECT);
    SetHelp(data->GUI.CH_AFORWARD,          MSG_HELP_CO_CH_AFORWARD);
    SetHelp(data->GUI.CH_ARESPONSE,         MSG_HELP_CO_CH_ARESPONSE);
    SetHelp(data->GUI.ST_ARESPONSE,         MSG_HELP_CO_ST_ARESPONSE);
    SetHelp(data->GUI.CH_AEXECUTE,          MSG_HELP_CO_CH_AEXECUTE);
    SetHelp(data->GUI.ST_AEXECUTE,          MSG_HELP_CO_ST_AEXECUTE);
    SetHelp(data->GUI.CH_APLAY,             MSG_HELP_CO_CH_APLAY);
    SetHelp(data->GUI.ST_APLAY,             MSG_HELP_CO_ST_APLAY);
    SetHelp(data->GUI.PO_APLAY,             MSG_HELP_CO_PO_APLAY);
    SetHelp(data->GUI.BT_APLAY,             MSG_HELP_CO_BT_APLAY);
    SetHelp(data->GUI.CH_AMOVE,             MSG_HELP_CO_CH_AMOVE);
    SetHelp(data->GUI.PO_MOVETO,            MSG_HELP_CO_PO_MOVETO);
    SetHelp(data->GUI.CH_ASTATUSTOMARKED,   MSG_HELP_CO_CH_ACTION_SET_STATUS_TO_MARKED);
    SetHelp(data->GUI.CH_ASTATUSTOUNMARKED, MSG_HELP_CO_CH_ACTION_SET_STATUS_TO_UNMARKED);
    SetHelp(data->GUI.CH_ASTATUSTOREAD,     MSG_HELP_CO_CH_ACTION_SET_STATUS_TO_READ);
    SetHelp(data->GUI.CH_ASTATUSTOUNREAD,   MSG_HELP_CO_CH_ACTION_SET_STATUS_TO_UNREAD);
    SetHelp(data->GUI.CH_ASTATUSTOSPAM,     MSG_HELP_CO_CH_ACTION_SET_STATUS_TO_SPAM);
    SetHelp(data->GUI.CH_ASTATUSTOHAM,      MSG_HELP_CO_CH_ACTION_SET_STATUS_TO_HAM);
    SetHelp(data->GUI.CH_ADELETE,           MSG_HELP_CO_CH_ADELETE);
    SetHelp(data->GUI.CH_ASKIP,             MSG_HELP_CO_CH_ASKIP);
    SetHelp(data->GUI.CH_ATERMINATE,        MSG_HELP_CO_CH_ATERMINATE);
    SetHelp(data->GUI.BT_RADD,              MSG_HELP_CO_BT_RADD);
    SetHelp(data->GUI.BT_RDEL,              MSG_HELP_CO_BT_RDEL);
    SetHelp(data->GUI.BT_FILTERUP,          MSG_HELP_CO_BT_FILTERUP);
    SetHelp(data->GUI.BT_FILTERDOWN,        MSG_HELP_CO_BT_FILTERDOWN);

    // set the cyclechain
    set(data->GUI.BT_APLAY, MUIA_CycleChain, TRUE);
    set(bt_moveto,MUIA_CycleChain, TRUE);
    set(data->GUI.BT_FILTERUP, MUIA_CycleChain, TRUE);
    set(data->GUI.BT_FILTERDOWN, MUIA_CycleChain, TRUE);
    set(data->GUI.BT_FILTER_IMPORT, MUIA_CycleChain, TRUE);

    GhostOutFilter(&(data->GUI), NULL);

    DoMethod(data->GUI.LV_RULES             ,MUIM_Notify, MUIA_NList_Active         ,MUIV_EveryTime ,MUIV_Notify_Application        ,2 ,MUIM_CallHook          ,&GetActiveFilterDataHook);
    DoMethod(data->GUI.ST_RNAME             ,MUIM_Notify, MUIA_String_Contents      ,MUIV_EveryTime ,MUIV_Notify_Application        ,2 ,MUIM_CallHook          ,&SetActiveFilterDataHook);
    DoMethod(data->GUI.CH_REMOTE            ,MUIM_Notify, MUIA_Selected             ,MUIV_EveryTime ,MUIV_Notify_Application        ,3 ,MUIM_CallHook          ,&CO_RemoteToggleHook       ,MUIV_TriggerValue);
    DoMethod(data->GUI.CH_APPLYREQ          ,MUIM_Notify, MUIA_Selected             ,MUIV_EveryTime ,MUIV_Notify_Application        ,2 ,MUIM_CallHook          ,&SetActiveFilterDataHook);
    DoMethod(data->GUI.CH_APPLYSENT         ,MUIM_Notify, MUIA_Selected             ,MUIV_EveryTime ,MUIV_Notify_Application        ,2 ,MUIM_CallHook          ,&SetActiveFilterDataHook);
    DoMethod(data->GUI.CH_APPLYNEW          ,MUIM_Notify, MUIA_Selected             ,MUIV_EveryTime ,MUIV_Notify_Application        ,2 ,MUIM_CallHook          ,&SetActiveFilterDataHook);
    DoMethod(data->GUI.CY_FILTER_COMBINE    ,MUIM_Notify, MUIA_Cycle_Active         , MUIV_EveryTime ,MUIV_Notify_Application       ,2 ,MUIM_CallHook          ,&SetActiveFilterDataHook);
    DoMethod(data->GUI.CH_AREDIRECT         ,MUIM_Notify, MUIA_Selected             ,MUIV_EveryTime ,MUIV_Notify_Application        ,2 ,MUIM_CallHook          ,&SetActiveFilterDataHook);
    DoMethod(data->GUI.CH_AFORWARD          ,MUIM_Notify, MUIA_Selected             ,MUIV_EveryTime ,MUIV_Notify_Application        ,2 ,MUIM_CallHook          ,&SetActiveFilterDataHook);
    DoMethod(data->GUI.CH_ARESPONSE         ,MUIM_Notify, MUIA_Selected             ,MUIV_EveryTime ,MUIV_Notify_Application        ,2 ,MUIM_CallHook          ,&SetActiveFilterDataHook);
    DoMethod(data->GUI.CH_AEXECUTE          ,MUIM_Notify, MUIA_Selected             ,MUIV_EveryTime ,MUIV_Notify_Application        ,2 ,MUIM_CallHook          ,&SetActiveFilterDataHook);
    DoMethod(data->GUI.CH_APLAY             ,MUIM_Notify, MUIA_Selected             ,MUIV_EveryTime ,MUIV_Notify_Application        ,2 ,MUIM_CallHook          ,&SetActiveFilterDataHook);
    DoMethod(data->GUI.CH_AMOVE             ,MUIM_Notify, MUIA_Selected             ,MUIV_EveryTime ,MUIV_Notify_Application        ,2 ,MUIM_CallHook          ,&SetActiveFilterDataHook);
    DoMethod(data->GUI.CH_ASTATUSTOMARKED   ,MUIM_Notify, MUIA_Selected             ,MUIV_EveryTime ,MUIV_Notify_Application        ,2 ,MUIM_CallHook          ,&SetActiveFilterDataHook);
    DoMethod(data->GUI.CH_ASTATUSTOUNMARKED ,MUIM_Notify, MUIA_Selected             ,MUIV_EveryTime ,MUIV_Notify_Application        ,2 ,MUIM_CallHook          ,&SetActiveFilterDataHook);
    DoMethod(data->GUI.CH_ASTATUSTOREAD     ,MUIM_Notify, MUIA_Selected             ,MUIV_EveryTime ,MUIV_Notify_Application        ,2 ,MUIM_CallHook          ,&SetActiveFilterDataHook);
    DoMethod(data->GUI.CH_ASTATUSTOUNREAD   ,MUIM_Notify, MUIA_Selected             ,MUIV_EveryTime ,MUIV_Notify_Application        ,2 ,MUIM_CallHook          ,&SetActiveFilterDataHook);
    DoMethod(data->GUI.CH_ASTATUSTOSPAM     ,MUIM_Notify, MUIA_Selected             ,MUIV_EveryTime ,MUIV_Notify_Application        ,2 ,MUIM_CallHook          ,&SetActiveFilterDataHook);
    DoMethod(data->GUI.CH_ASTATUSTOHAM      ,MUIM_Notify, MUIA_Selected             ,MUIV_EveryTime ,MUIV_Notify_Application        ,2 ,MUIM_CallHook          ,&SetActiveFilterDataHook);
    DoMethod(data->GUI.CH_ADELETE           ,MUIM_Notify, MUIA_Selected             ,MUIV_EveryTime ,MUIV_Notify_Application        ,2 ,MUIM_CallHook          ,&SetActiveFilterDataHook);
    DoMethod(data->GUI.CH_ASKIP             ,MUIM_Notify, MUIA_Selected             ,MUIV_EveryTime ,MUIV_Notify_Application        ,2 ,MUIM_CallHook          ,&SetActiveFilterDataHook);
    DoMethod(data->GUI.CH_ATERMINATE        ,MUIM_Notify, MUIA_Selected             ,MUIV_EveryTime ,MUIV_Notify_Application        ,2 ,MUIM_CallHook          ,&SetActiveFilterDataHook);
    DoMethod(data->GUI.ST_AREDIRECT         ,MUIM_Notify, MUIA_String_BufferPos     ,MUIV_EveryTime ,MUIV_Notify_Application        ,2 ,MUIM_CallHook          ,&SetActiveFilterDataHook);
    DoMethod(data->GUI.ST_AFORWARD          ,MUIM_Notify, MUIA_String_BufferPos     ,MUIV_EveryTime ,MUIV_Notify_Application        ,2 ,MUIM_CallHook          ,&SetActiveFilterDataHook);
    DoMethod(data->GUI.ST_ARESPONSE         ,MUIM_Notify, MUIA_String_Contents      ,MUIV_EveryTime ,MUIV_Notify_Application        ,2 ,MUIM_CallHook          ,&SetActiveFilterDataHook);
    DoMethod(data->GUI.ST_AEXECUTE          ,MUIM_Notify, MUIA_String_Contents      ,MUIV_EveryTime ,MUIV_Notify_Application        ,2 ,MUIM_CallHook          ,&SetActiveFilterDataHook);
    DoMethod(data->GUI.ST_APLAY             ,MUIM_Notify, MUIA_String_Contents      ,MUIV_EveryTime ,MUIV_Notify_Application        ,2 ,MUIM_CallHook          ,&SetActiveFilterDataHook);
    DoMethod(data->GUI.BT_APLAY             ,MUIM_Notify, MUIA_Pressed              ,FALSE          ,MUIV_Notify_Application        ,3 ,MUIM_CallHook          ,&CO_PlaySoundHook,data->GUI.ST_APLAY);
    DoMethod(data->GUI.TX_MOVETO            ,MUIM_Notify, MUIA_Text_Contents        ,MUIV_EveryTime ,MUIV_Notify_Application        ,2 ,MUIM_CallHook          ,&SetActiveFilterDataHook);
    DoMethod(data->GUI.LV_MOVETO            ,MUIM_Notify, MUIA_NList_DoubleClick,    TRUE           ,data->GUI.PO_MOVETO            ,2 ,MUIM_Popstring_Close   ,TRUE);
    DoMethod(data->GUI.BT_RADD              ,MUIM_Notify, MUIA_Pressed              ,FALSE          ,MUIV_Notify_Application        ,2 ,MUIM_CallHook          ,&AddNewFilterToListHook);
    DoMethod(data->GUI.BT_RDEL              ,MUIM_Notify, MUIA_Pressed              ,FALSE          ,MUIV_Notify_Application        ,2 ,MUIM_CallHook          ,&RemoveActiveFilterHook);
    DoMethod(data->GUI.BT_FILTERUP          ,MUIM_Notify, MUIA_Pressed              ,FALSE          ,data->GUI.LV_RULES             ,3 ,MUIM_NList_Move        ,MUIV_NList_Move_Selected   ,MUIV_NList_Move_Previous);
    DoMethod(data->GUI.BT_FILTERDOWN        ,MUIM_Notify, MUIA_Pressed              ,FALSE          ,data->GUI.LV_RULES             ,3 ,MUIM_NList_Move        ,MUIV_NList_Move_Selected   ,MUIV_NList_Move_Next);
    DoMethod(data->GUI.CH_AMOVE             ,MUIM_Notify, MUIA_Selected             ,TRUE           ,data->GUI.CH_ADELETE           ,3 ,MUIM_Set               ,MUIA_Selected              ,FALSE);
    DoMethod(data->GUI.CH_ADELETE           ,MUIM_Notify, MUIA_Selected             ,TRUE           ,data->GUI.CH_AMOVE             ,3 ,MUIM_Set               ,MUIA_Selected              ,FALSE);
    DoMethod(data->GUI.CH_ASTATUSTOMARKED   ,MUIM_Notify, MUIA_Selected             ,TRUE           ,data->GUI.CH_ASTATUSTOUNMARKED ,3 ,MUIM_Set               ,MUIA_Selected              ,FALSE);
    DoMethod(data->GUI.CH_ASTATUSTOUNMARKED ,MUIM_Notify, MUIA_Selected             ,TRUE           ,data->GUI.CH_ASTATUSTOMARKED   ,3 ,MUIM_Set               ,MUIA_Selected              ,FALSE);
    DoMethod(data->GUI.CH_ASTATUSTOREAD     ,MUIM_Notify, MUIA_Selected             ,TRUE           ,data->GUI.CH_ASTATUSTOUNREAD   ,3 ,MUIM_Set               ,MUIA_Selected              ,FALSE);
    DoMethod(data->GUI.CH_ASTATUSTOUNREAD   ,MUIM_Notify, MUIA_Selected             ,TRUE           ,data->GUI.CH_ASTATUSTOREAD     ,3 ,MUIM_Set               ,MUIA_Selected              ,FALSE);
    DoMethod(data->GUI.CH_ASTATUSTOSPAM     ,MUIM_Notify, MUIA_Selected             ,TRUE           ,data->GUI.CH_ASTATUSTOHAM      ,3 ,MUIM_Set               ,MUIA_Selected              ,FALSE);
    DoMethod(data->GUI.CH_ASTATUSTOHAM      ,MUIM_Notify, MUIA_Selected             ,TRUE           ,data->GUI.CH_ASTATUSTOSPAM     ,3 ,MUIM_Set               ,MUIA_Selected              ,FALSE);
    DoMethod(data->GUI.BT_FILTER_IMPORT     ,MUIM_Notify, MUIA_Pressed              ,FALSE          ,MUIV_Notify_Application        ,2 ,MUIM_CallHook          ,&ImportFilterHook);
  }

  RETURN(grp);
  return grp;
}

///
/// CO_PageWrite
Object *CO_PageWrite(struct CO_ClassData *data)
{
  Object *obj;
  Object *codesetPopButton;
  static const char *wrapmode[4];

  ENTER();

  wrapmode[0] = tr(MSG_CO_EWOff);
  wrapmode[1] = tr(MSG_CO_EWAsYouType);
  wrapmode[2] = tr(MSG_CO_EWBeforeSend);
  wrapmode[3] = NULL;

  obj = VGroup,
    MUIA_HelpNode, "Configuration#Write",

    ConfigPageHeaderObject("config_write_big", G->theme.configImages[CI_WRITEBIG], tr(MSG_CO_WRITE_TITLE), tr(MSG_CO_WRITE_SUMMARY)),

    Child, ScrollgroupObject,
      MUIA_Scrollgroup_FreeHoriz, FALSE,
      MUIA_Scrollgroup_AutoBars, TRUE,
      MUIA_Scrollgroup_Contents, VGroupV,

        Child, VGroup, GroupFrameT(tr(MSG_CO_MessageBody)),
          Child, ColGroup(2),
            Child, Label2(tr(MSG_CO_Welcome)),
            Child, data->GUI.ST_HELLOTEXT = MakeString(SIZE_INTRO,tr(MSG_CO_Welcome)),

            Child, Label2(tr(MSG_CO_Greetings)),
            Child, data->GUI.ST_BYETEXT = MakeString(SIZE_INTRO,tr(MSG_CO_Greetings)),

            Child, Label2(tr(MSG_CO_DEFAULTCODESET_WRITE)),
            Child, MakeCodesetPop(&data->GUI.TX_DEFCODESET_WRITE, &codesetPopButton),
          End,
        End,

        Child, VGroup, GroupFrameT(tr(MSG_CO_Editor)),
          Child, ColGroup(2),

            Child, Label2(tr(MSG_CO_WordWrap)),
            Child, HGroup,
              Child, data->GUI.ST_EDWRAP = MakeInteger(3, tr(MSG_CO_WordWrap)),
              Child, data->GUI.CY_EDWRAP = MakeCycle(wrapmode, ""),
            End,

            Child, Label2(tr(MSG_CO_NB_EMAILCACHE)),
            Child, HGroup,
              Child, data->GUI.NB_EMAILCACHE = NumericbuttonObject,
                MUIA_CycleChain,      TRUE,
                MUIA_Numeric_Min,     0,
                MUIA_Numeric_Max,     100,
                MUIA_Numeric_Format,  tr(MSG_CO_NB_EMAILCACHEFMT),
              End,
              Child, HSpace(0),
            End,

            Child, Label2(tr(MSG_CO_NB_AUTOSAVE)),
            Child, HGroup,
              Child, data->GUI.NB_AUTOSAVE = NumericbuttonObject,
                MUIA_CycleChain,      TRUE,
                MUIA_Numeric_Min,     0,
                MUIA_Numeric_Max,     30,
                MUIA_Numeric_Format,  tr(MSG_CO_NB_AUTOSAVEFMT),
              End,
              Child, HSpace(0),
            End,

          End,

          Child, RectangleObject,
            MUIA_VertWeight,          0,
            MUIA_Rectangle_HBar,      TRUE,
            MUIA_Rectangle_BarTitle,  tr(MSG_CO_FONTSETTINGS),
          End,
          Child, MakeCheckGroup(&data->GUI.CH_FIXEDFONT_WRITE, tr(MSG_CO_FIXEDFONT_WRITE)),
          Child, MakeCheckGroup(&data->GUI.CH_TEXTCOLORS_WRITE, tr(MSG_CO_TEXTCOLORS_WRITE)),
          Child, MakeCheckGroup(&data->GUI.CH_TEXTSTYLES_WRITE, tr(MSG_CO_TEXTSTYLES_WRITE)),

        End,

        Child, VGroup, GroupFrameT(tr(MSG_CO_OtherOptions)),
          Child, MakeCheckGroup(&data->GUI.CH_WARNSUBJECT, tr(MSG_CO_WARNSUBJECT)),
          Child, MakeCheckGroup(&data->GUI.CH_LAUNCH, tr(MSG_CO_LAUNCH_EXTEDITOR)),
        End,

        Child, HVSpace,

      End,
    End,

  End;

  if(obj != NULL)
  {
    set(codesetPopButton, MUIA_ControlChar, ShortCut(tr(MSG_CO_DEFAULTCHARSET)));

    SetHelp(data->GUI.ST_HELLOTEXT,        MSG_HELP_CO_ST_HELLOTEXT);
    SetHelp(data->GUI.ST_BYETEXT,          MSG_HELP_CO_ST_BYETEXT);
    SetHelp(data->GUI.CH_WARNSUBJECT,      MSG_HELP_CO_CH_WARNSUBJECT);
    SetHelp(data->GUI.ST_EDWRAP,           MSG_HELP_CO_ST_EDWRAP);
    SetHelp(data->GUI.CY_EDWRAP,           MSG_HELP_CO_CY_EDWRAP);
    SetHelp(data->GUI.CH_LAUNCH,           MSG_HELP_CO_CH_LAUNCH);
    SetHelp(data->GUI.NB_EMAILCACHE,       MSG_HELP_CO_NB_EMAILCACHE);
    SetHelp(data->GUI.NB_AUTOSAVE,         MSG_HELP_CO_NB_AUTOSAVE);
    SetHelp(data->GUI.TX_DEFCODESET_WRITE, MSG_HELP_CO_TX_DEFCODESET_WRITE);
    SetHelp(data->GUI.CH_TEXTSTYLES_WRITE, MSG_HELP_CO_CH_TEXTSTYLES_WRITE);
    SetHelp(data->GUI.CH_TEXTCOLORS_WRITE, MSG_HELP_CO_CH_TEXTCOLORS_WRITE);

    DoMethod(data->GUI.CY_EDWRAP, MUIM_Notify, MUIA_Cycle_Active, MUIV_EveryTime, data->GUI.ST_EDWRAP, 3, MUIM_Set, MUIA_Disabled, MUIV_NotTriggerValue);
  }

  RETURN(obj);
  return obj;
}

///
/// CO_PageReplyForward
Object *CO_PageReplyForward(struct CO_ClassData *data)
{
  Object *obj;
  Object *popButton;
  static const char *fwdmode[3];

  ENTER();

  fwdmode[0] = tr(MSG_CO_FWDMSG_ATTACH);
  fwdmode[1] = tr(MSG_CO_FWDMSG_INLINE);
  fwdmode[2] = NULL;

  obj = VGroup,
    MUIA_HelpNode, "Configuration#ReplyForward",

    ConfigPageHeaderObject("config_answer_big", G->theme.configImages[CI_ANSWERBIG], tr(MSG_CO_REPLY_TITLE), tr(MSG_CO_REPLY_SUMMARY)),

    Child, ScrollgroupObject,
      MUIA_Scrollgroup_FreeHoriz, FALSE,
      MUIA_Scrollgroup_AutoBars, TRUE,
      MUIA_Scrollgroup_Contents, VGroupV,

        Child, VGroup, GroupFrameT(tr(MSG_CO_Replying)),
          Child, ColGroup(2),
            Child, Label2(tr(MSG_CO_RepInit)),
            Child, MakePhraseGroup(&data->GUI.ST_REPLYHI, &data->GUI.ST_REPLYTEXT, &data->GUI.ST_REPLYBYE, tr(MSG_CO_RepInit), tr(MSG_HELP_CO_ST_REPLYTEXT)),

            Child, Label2(tr(MSG_CO_AltRepInit)),
            Child, MakePhraseGroup(&data->GUI.ST_AREPLYHI, &data->GUI.ST_AREPLYTEXT, &data->GUI.ST_AREPLYBYE, tr(MSG_CO_AltRepInit), tr(MSG_HELP_CO_ST_AREPLYTEXT)),

            Child, HSpace(1),
            Child, HGroup,
              Child, Label2(tr(MSG_CO_AltRepPat)),
              Child, data->GUI.ST_AREPLYPAT = MakeString(SIZE_PATTERN, tr(MSG_CO_AltRepPat)),
            End,

            Child, Label2(tr(MSG_CO_MLRepInit)),
            Child, MakePhraseGroup(&data->GUI.ST_MREPLYHI, &data->GUI.ST_MREPLYTEXT, &data->GUI.ST_MREPLYBYE, tr(MSG_CO_MLRepInit), tr(MSG_HELP_CO_ST_MREPLYTEXT)),

            Child, HSpace(1),
            Child, MakeCheckGroup(&data->GUI.CH_COMPADDR, tr(MSG_CO_VerifyAddress)),

            Child, HSpace(1),
            Child, MakeCheckGroup(&data->GUI.CH_QUOTEEMPTY, tr(MSG_CO_QuoteEmpty)),

            Child, HSpace(1),
            Child, MakeCheckGroup(&data->GUI.CH_STRIPSIG, tr(MSG_CO_StripSignature)),

          End,
        End,

        Child, ColGroup(2), GroupFrameT(tr(MSG_CO_Forwarding)),

          Child, Label2(tr(MSG_CO_FWDMSG)),
          Child, data->GUI.CY_FORWARDMODE = MakeCycle(fwdmode, tr(MSG_CO_FWDMSG)),

          Child, Label2(tr(MSG_CO_FwdInit)),
          Child, MakeVarPop(&data->GUI.ST_FWDSTART, &popButton, PHM_FORWARD, SIZE_INTRO, tr(MSG_CO_FwdInit)),

          Child, Label2(tr(MSG_CO_FwdFinish)),
          Child, MakeVarPop(&data->GUI.ST_FWDEND, &popButton, PHM_FORWARD, SIZE_INTRO, tr(MSG_CO_FwdFinish)),

        End,

        Child, HVSpace,

      End,
    End,

  End;

  if(obj != NULL)
  {
    SetHelp(data->GUI.ST_FWDSTART,    MSG_HELP_CO_ST_FWDSTART);
    SetHelp(data->GUI.ST_FWDEND,      MSG_HELP_CO_ST_FWDEND);
    SetHelp(data->GUI.ST_AREPLYPAT,   MSG_HELP_CO_ST_AREPLYPAT);
    SetHelp(data->GUI.CH_QUOTEEMPTY,  MSG_HELP_CO_CH_QUOTEEMPTY);
    SetHelp(data->GUI.CH_COMPADDR,    MSG_HELP_CO_CH_COMPADDR);
    SetHelp(data->GUI.CH_STRIPSIG,    MSG_HELP_CO_CH_STRIPSIG);
    SetHelp(data->GUI.CY_FORWARDMODE, MSG_HELP_CO_CY_FORWARDMODE);
  }

  RETURN(obj);
  return obj;
}

///
/// CO_PageSignature
Object *CO_PageSignature(struct CO_ClassData *data)
{
  Object *obj;
  Object *slider = ScrollbarObject, End;

  ENTER();

  obj = VGroup,
    MUIA_HelpNode, "Configuration#Signature",

    ConfigPageHeaderObject("config_signature_big", G->theme.configImages[CI_SIGNATUREBIG], tr(MSG_CO_SIGNATURE_TITLE), tr(MSG_CO_SIGNATURE_SUMMARY)),

    Child, ScrollgroupObject,
      MUIA_Scrollgroup_FreeHoriz, FALSE,
      MUIA_Scrollgroup_AutoBars, TRUE,
      MUIA_Scrollgroup_Contents, VGroupV,

        Child, VGroup,
          Child, HGroup,
            GroupSpacing(0),
            Child, VGroup,
              MUIA_HorizWeight, 30,

              Child, HBarT(tr(MSG_CO_SIGNATURES)), End,

              Child, NListviewObject,
                MUIA_CycleChain, TRUE,
                MUIA_Weight, 60,
                MUIA_NListview_NList, data->GUI.LV_SIGNATURE = SignatureListObject,
                End,
              End,

              Child, HGroup,
                Child, ColGroup(2),
                  GroupSpacing(1),
                  MUIA_Group_SameWidth, TRUE,
                  MUIA_Weight, 1,
                  Child, data->GUI.BT_SIGADD = MakeButton(MUIX_B "+" MUIX_N),
                  Child, data->GUI.BT_SIGDEL = MakeButton(MUIX_B "-" MUIX_N),
                End,
                Child, HSpace(0),
                Child, ColGroup(2),
                  GroupSpacing(1),
                  MUIA_Group_SameWidth, TRUE,
                  Child, data->GUI.BT_SIGUP = PopButton(MUII_ArrowUp),
                  Child, data->GUI.BT_SIGDOWN = PopButton(MUII_ArrowDown),
                End,
              End,
            End,

            Child, NBalanceObject,
              MUIA_Balance_Quiet, TRUE,
            End,

            Child, VGroup, GroupFrameT(tr(MSG_CO_Signature)),
              Child, VGroup,
                Child, ColGroup(2),

                  Child, HSpace(1),
                  Child, MakeCheckGroup(&data->GUI.CH_SIG_ACTIVE, tr(MSG_CO_SIGNATURE_ACTIVE)),

                  Child, Label2(tr(MSG_CO_SIGNATURE_DESCRIPTION)),
                  Child, data->GUI.ST_SIG_DESC = MakeString(SIZE_DEFAULT, tr(MSG_CO_SIGNATURE_DESCRIPTION)),

                  Child, VGroup,
                    Child, Label2(tr(MSG_CO_SIGNATURE_TEXT)),
                    Child, VSpace(0),
                  End,
                  Child, VGroup,
                    Child, data->GUI.GR_SIGEDIT = HGroup,
                      GroupSpacing(0),
                      Child, data->GUI.TE_SIGEDIT = SignatureTextEditObject,
                        InputListFrame,
                        MUIA_CycleChain,                     TRUE,
                        MUIA_TextEditor_FixedFont,           TRUE,
                        MUIA_TextEditor_ExportHook,          MUIV_TextEditor_ExportHook_EMail,
                        MUIA_TextEditor_Slider,              slider,
                        MUIA_TextEditor_WrapMode,            MUIV_TextEditor_WrapMode_HardWrap,
                        MUIA_TextEditor_WrapBorder,          C->EdWrapCol,
                        MUIA_TextEditor_ActiveObjectOnClick, TRUE,
                      End,
                      Child, slider,
                    End,
                    Child, data->GUI.BT_SIGEDIT = MakeButton(tr(MSG_CO_EditSig)),
                    Child, HGroup,
                      MUIA_Group_SameWidth, TRUE,
                      Child, data->GUI.BT_INSTAG = MakeButton(tr(MSG_CO_InsertTag)),
                      Child, data->GUI.BT_INSENV = MakeButton(tr(MSG_CO_InsertENV)),
                    End,
                  End,

                  Child, HSpace(1),
                  Child, MakeCheckGroup(&data->GUI.CH_SIG_FILE, tr(MSG_CO_APPEND_SIGNATURE_FILE)),

                  Child, HSpace(1),
                  Child, data->GUI.PO_SIG_FILE = PopaslObject,
                    MUIA_Popasl_Type, ASL_FileRequest,
                    MUIA_Popstring_String, data->GUI.ST_SIG_FILE = MakeString(SIZE_PATHFILE, tr(MSG_CO_APPEND_SIGNATURE_FILE)),
                    MUIA_Popstring_Button, PopButton(MUII_PopFile),
                  End,
                End,
              End,
            End,
          End,
        End,

        Child, VGroup, GroupFrameT(tr(MSG_CO_Taglines)),
          Child, ColGroup(2),

            Child, Label2(tr(MSG_CO_TaglineFile)),
            Child, PopaslObject,
              MUIA_Popasl_Type, ASL_FileRequest,
              MUIA_Popstring_String, data->GUI.ST_TAGFILE = MakeString(SIZE_PATHFILE,tr(MSG_CO_TaglineFile)),
              MUIA_Popstring_Button, PopButton(MUII_PopFile),
            End,

            Child, Label2(tr(MSG_CO_TaglineSep)),
            Child, data->GUI.ST_TAGSEP = MakeString(SIZE_SMALL,tr(MSG_CO_TaglineSep)),

          End,
        End,

      End,
    End,

  End;

  if(obj != NULL)
  {
    // enhance the CycleChain
    set(data->GUI.BT_SIGUP,   MUIA_CycleChain, TRUE);
    set(data->GUI.BT_SIGDOWN, MUIA_CycleChain, TRUE);

    // set help text for gadgets
    SetHelp(data->GUI.LV_SIGNATURE,  MSG_HELP_CO_LV_SIGNATURE);
    SetHelp(data->GUI.BT_SIGADD,     MSG_HELP_CO_BT_SIGADD);
    SetHelp(data->GUI.BT_SIGDEL,     MSG_HELP_CO_BT_SIGDEL);
    SetHelp(data->GUI.BT_SIGUP,      MSG_HELP_CO_BT_SIGUP);
    SetHelp(data->GUI.BT_SIGDOWN,    MSG_HELP_CO_BT_SIGDOWN);
    SetHelp(data->GUI.CH_SIG_ACTIVE, MSG_HELP_CO_BT_SIG_ACTIVE);
    SetHelp(data->GUI.ST_SIG_DESC,   MSG_HELP_CO_ST_SIG_DESC);
    SetHelp(data->GUI.TE_SIGEDIT,    MSG_HELP_CO_TE_SIGEDIT);
    SetHelp(data->GUI.BT_SIGEDIT,    MSG_HELP_CO_BT_EDITSIG);
    SetHelp(data->GUI.BT_INSTAG,     MSG_HELP_CO_BT_INSTAG);
    SetHelp(data->GUI.BT_INSENV,     MSG_HELP_CO_BT_INSENV);
    SetHelp(data->GUI.ST_TAGFILE,    MSG_HELP_CO_ST_TAGFILE);
    SetHelp(data->GUI.ST_TAGSEP,     MSG_HELP_CO_ST_TAGSEP);

    // connect a notify if the user selects a different signature in the list
    DoMethod(data->GUI.LV_SIGNATURE, MUIM_Notify, MUIA_NList_Active, MUIV_EveryTime, MUIV_Notify_Application, 2, MUIM_CallHook, &CO_GetSignatureEntryHook);

    // connect notifies to update the SignatureNode according to the latest
    // settings in this config page
    DoMethod(data->GUI.CH_SIG_ACTIVE, MUIM_Notify, MUIA_Selected,         MUIV_EveryTime, MUIV_Notify_Application, 2, MUIM_CallHook, &CO_PutSignatureEntryHook);
    DoMethod(data->GUI.ST_SIG_DESC,   MUIM_Notify, MUIA_String_Contents,  MUIV_EveryTime, MUIV_Notify_Application, 2, MUIM_CallHook, &CO_PutSignatureEntryHook);
    DoMethod(data->GUI.CH_SIG_FILE,   MUIM_Notify, MUIA_Selected,         MUIV_EveryTime, MUIV_Notify_Application, 2, MUIM_CallHook, &CO_PutSignatureEntryHook);

    // some button notifies
    DoMethod(data->GUI.BT_SIGADD,  MUIM_Notify, MUIA_Pressed, FALSE, MUIV_Notify_Application, 2, MUIM_CallHook, &CO_AddSignatureHook);
    DoMethod(data->GUI.BT_SIGDEL,  MUIM_Notify, MUIA_Pressed, FALSE, MUIV_Notify_Application, 2, MUIM_CallHook, &CO_DelSignatureHook);
    DoMethod(data->GUI.BT_SIGUP,   MUIM_Notify, MUIA_Pressed, FALSE, data->GUI.LV_SIGNATURE, 3, MUIM_NList_Move, MUIV_NList_Move_Selected, MUIV_NList_Move_Previous);
    DoMethod(data->GUI.BT_SIGDOWN, MUIM_Notify, MUIA_Pressed, FALSE, data->GUI.LV_SIGNATURE, 3, MUIM_NList_Move, MUIV_NList_Move_Selected, MUIV_NList_Move_Next);
    DoMethod(data->GUI.BT_INSTAG,  MUIM_Notify, MUIA_Pressed, FALSE, data->GUI.TE_SIGEDIT, 3, MUIM_TextEditor_InsertText, "%t\n", MUIV_TextEditor_InsertText_Cursor);
    DoMethod(data->GUI.BT_INSENV,  MUIM_Notify, MUIA_Pressed, FALSE, data->GUI.TE_SIGEDIT, 3, MUIM_TextEditor_InsertText, "%e\n", MUIV_TextEditor_InsertText_Cursor);
    DoMethod(data->GUI.BT_SIGEDIT, MUIM_Notify, MUIA_Pressed, FALSE, data->GUI.TE_SIGEDIT, 1, MUIM_SignatureTextEdit_EditExternally);
  }

  RETURN(obj);
  return obj;
}

///
/// CO_PageSecurity
Object *CO_PageSecurity(struct CO_ClassData *data)
{
  static const char *logfmode[4];
  Object *obj;

  ENTER();

  logfmode[0] = tr(MSG_CO_LogNone);
  logfmode[1] = tr(MSG_CO_LogNormal);
  logfmode[2] = tr(MSG_CO_LogVerbose);
  logfmode[3] = NULL;

  obj = VGroup,
    MUIA_HelpNode, "Configuration#Security",

    ConfigPageHeaderObject("config_security_big", G->theme.configImages[CI_SECURITYBIG], tr(MSG_CO_SECURITY_TITLE), tr(MSG_CO_SECURITY_SUMMARY)),

    Child, ScrollgroupObject,
      MUIA_Scrollgroup_FreeHoriz, FALSE,
      MUIA_Scrollgroup_AutoBars, TRUE,
      MUIA_Scrollgroup_Contents, VGroupV,

        Child, VGroup, GroupFrameT("PGP (Pretty Good Privacy)"),
          Child, ColGroup(2),

            Child, Label2(tr(MSG_CO_PGPExe)),
            Child, PopaslObject,
              MUIA_Popasl_Type     ,ASL_FileRequest,
              MUIA_Popstring_String,data->GUI.ST_PGPCMD= MakeString(SIZE_PATHFILE,tr(MSG_CO_PGPExe)),
              MUIA_Popstring_Button,PopButton(MUII_PopDrawer),
              ASLFR_DrawersOnly, TRUE,
            End,

            Child, HSpace(1),
            Child, HGroup,
              Child, data->GUI.CH_PGPPASSINTERVAL = MakeCheck(tr(MSG_CO_PGPPASSINTERVAL1)),
              Child, Label2(tr(MSG_CO_PGPPASSINTERVAL1)),
              Child, data->GUI.NB_PGPPASSINTERVAL = MakeNumeric(1, 90, FALSE),
              Child, Label2(tr(MSG_CO_PGPPASSINTERVAL2)),
              Child, HSpace(0),
            End,

          End,
        End,

        Child, ColGroup(2), GroupFrameT(tr(MSG_CO_Logfiles)),
          Child, Label1(tr(MSG_CO_LogMode)),
          Child, data->GUI.CY_LOGMODE = MakeCycle(logfmode, tr(MSG_CO_LogMode)),

          Child, Label2(tr(MSG_CO_LogPath)),
          Child, data->GUI.PO_LOGFILE = PopaslObject,
            MUIA_Popasl_Type, ASL_FileRequest,
            MUIA_Popstring_String, data->GUI.ST_LOGFILE = MakeString(SIZE_PATH,tr(MSG_CO_LogPath)),
            MUIA_Popstring_Button, PopButton(MUII_PopDrawer),
            ASLFR_DrawersOnly, TRUE,
          End,

          Child, HSpace(1),
          Child, MakeCheckGroup(&data->GUI.CH_SPLITLOG, tr(MSG_CO_LogSplit)),

          Child, HSpace(1),
          Child, MakeCheckGroup(&data->GUI.CH_LOGALL, tr(MSG_CO_LogAllEvents)),

        End,

        Child, HVSpace,

      End,
    End,

  End;

  if(obj != NULL)
  {
    SetHelp(data->GUI.ST_PGPCMD    ,MSG_HELP_CO_ST_PGPCMD   );
    SetHelp(data->GUI.CH_PGPPASSINTERVAL, MSG_HELP_CO_PGPPASSINTERVAL);
    SetHelp(data->GUI.NB_PGPPASSINTERVAL, MSG_HELP_CO_PGPPASSINTERVAL);
    SetHelp(data->GUI.ST_LOGFILE   ,MSG_HELP_CO_ST_LOGFILE  );
    SetHelp(data->GUI.CH_SPLITLOG  ,MSG_HELP_CO_CH_SPLITLOG );
    SetHelp(data->GUI.CY_LOGMODE   ,MSG_HELP_CO_CY_LOGMODE  );
    SetHelp(data->GUI.CH_LOGALL    ,MSG_HELP_CO_CH_LOGALL   );

    DoMethod(data->GUI.CY_LOGMODE, MUIM_Notify, MUIA_Cycle_Active, MUIV_EveryTime, MUIV_Notify_Application, 7, MUIM_MultiSet, MUIA_Disabled, MUIV_NotTriggerValue, data->GUI.PO_LOGFILE,
                                                                                                                                                                   data->GUI.CH_SPLITLOG,
                                                                                                                                                                   data->GUI.CH_LOGALL,
                                                                                                                                                                   NULL);

    DoMethod(data->GUI.CH_PGPPASSINTERVAL, MUIM_Notify, MUIA_Selected, MUIV_EveryTime, data->GUI.NB_PGPPASSINTERVAL, 3, MUIM_Set, MUIA_Disabled, MUIV_NotTriggerValue);
  }

  RETURN(obj);
  return obj;
}

///
/// CO_PageStartupQuit
Object *CO_PageStartupQuit(struct CO_ClassData *data)
{
  Object *grp;

  ENTER();

  if((grp = VGroup,
    MUIA_HelpNode, "Configuration#StartQuit",

    ConfigPageHeaderObject("config_start_big", G->theme.configImages[CI_STARTBIG], tr(MSG_CO_STARTUP_TITLE), tr(MSG_CO_STARTUP_SUMMARY)),

     Child, ScrollgroupObject,
       MUIA_Scrollgroup_FreeHoriz, FALSE,
       MUIA_Scrollgroup_AutoBars, TRUE,
       MUIA_Scrollgroup_Contents, VGroupV,

        Child, VGroup, GroupFrameT(tr(MSG_CO_OnStartup)),
          Child, MakeCheckGroup(&data->GUI.CH_LOADALL, tr(MSG_CO_LoadAll)),
          Child, MakeCheckGroup(&data->GUI.CH_MARKNEW, tr(MSG_CO_MarkNew)),
          Child, MakeCheckGroup(&data->GUI.CH_DELETESTART, tr(MSG_CO_DeleteOld)),
          Child, MakeCheckGroup(&data->GUI.CH_REMOVESTART, tr(MSG_CO_RemoveDel)),
          Child, MakeCheckGroup(&data->GUI.CH_CHECKBD, tr(MSG_CO_CheckDOB)),
          Child, MakeCheckGroup(&data->GUI.CH_SENDSTART, tr(MSG_CO_SendStart)),
        End,
        Child, VGroup, GroupFrameT(tr(MSG_CO_OnTermination)),
          Child, MakeCheckGroup(&data->GUI.CH_SENDQUIT, tr(MSG_CO_SendStart)),
          Child, MakeCheckGroup(&data->GUI.CH_DELETEQUIT, tr(MSG_CO_DeleteOld)),
          Child, MakeCheckGroup(&data->GUI.CH_REMOVEQUIT, tr(MSG_CO_RemoveDel)),
        End,
        Child, HVSpace,

      End,
    End,
  End))
  {
    SetHelp(data->GUI.CH_LOADALL,     MSG_HELP_CO_CH_LOADALL  );
    SetHelp(data->GUI.CH_MARKNEW,     MSG_HELP_CO_CH_MARKNEW  );
    SetHelp(data->GUI.CH_DELETESTART, MSG_HELP_CO_CH_DELETEOLD);
    SetHelp(data->GUI.CH_REMOVESTART, MSG_HELP_CO_CH_REMOVEDEL);
    SetHelp(data->GUI.CH_SENDSTART,   MSG_HELP_CO_CH_SEND     );
    SetHelp(data->GUI.CH_CHECKBD,     MSG_HELP_CO_CH_CHECKBD  );
    SetHelp(data->GUI.CH_SENDQUIT,    MSG_HELP_CO_CH_SEND     );
    SetHelp(data->GUI.CH_DELETEQUIT,  MSG_HELP_CO_CH_DELETEOLD);
    SetHelp(data->GUI.CH_REMOVEQUIT,  MSG_HELP_CO_CH_REMOVEDEL);
  }

  RETURN(grp);
  return grp;
}

///
/// CO_PageMIME
Object *CO_PageMIME(struct CO_ClassData *data)
{
  Object *obj;
  Object *popButton;

  ENTER();

  obj = VGroup,
    MUIA_HelpNode, "Configuration#MIME",

    ConfigPageHeaderObject("config_mime_big", G->theme.configImages[CI_MIMEBIG], tr(MSG_CO_MIME_TITLE), tr(MSG_CO_MIME_SUMMARY)),

    Child, ScrollgroupObject,
      MUIA_Scrollgroup_FreeHoriz, FALSE,
      MUIA_Scrollgroup_AutoBars, TRUE,
      MUIA_Scrollgroup_Contents, VGroupV,

        Child, VGroup,
          Child, HGroup,
            GroupSpacing(0),
            Child, VGroup,
              MUIA_Weight, 30,

              Child, HBarT(tr(MSG_CO_MIMETYPE_TITLE)), End,

              Child, NListviewObject,
                MUIA_CycleChain, TRUE,
                MUIA_NListview_NList, data->GUI.LV_MIME = MimeTypeListObject,
                End,
              End,

              Child, HGroup,
                Child, ColGroup(2),
                  MUIA_Group_Spacing, 1,
                  MUIA_Group_SameWidth, TRUE,
                  MUIA_Weight, 1,
                  Child, data->GUI.BT_MADD = MakeButton(MUIX_B "+" MUIX_N),
                  Child, data->GUI.BT_MDEL = MakeButton(MUIX_B "-" MUIX_N),
                End,
                Child, HSpace(0),
                Child, data->GUI.BT_MIMEIMPORT = PopButton(MUII_PopFile),
              End,
            End,

            Child, NBalanceObject,
              MUIA_Balance_Quiet, TRUE,
            End,

            Child, VGroup,
              GroupFrameT(tr(MSG_Options)),

              MUIA_Weight, 70,
              Child, data->GUI.GR_MIME = ColGroup(2),
                Child, Label2(tr(MSG_CO_MimeType)),
                Child, MakeMimeTypePop(&data->GUI.ST_CTYPE, tr(MSG_CO_MimeType)),

                Child, Label2(tr(MSG_CO_Extension)),
                Child, data->GUI.ST_EXTENS = BetterStringObject,
                  StringFrame,
                  MUIA_String_Accept,      "0123456789ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz ",
                  MUIA_String_MaxLen,      SIZE_NAME,
                  MUIA_ControlChar,        ShortCut(tr(MSG_CO_Extension)),
                  MUIA_String_AdvanceOnCR, TRUE,
                  MUIA_CycleChain,         TRUE,
                End,

                Child, Label2(tr(MSG_CO_MIME_DESCRIPTION)),
                Child, data->GUI.ST_DESCRIPTION = MakeString(SIZE_DEFAULT, tr(MSG_CO_MIME_DESCRIPTION)),

                Child, Label2(tr(MSG_CO_MimeCmd)),
                Child, HGroup,
                  MUIA_Group_HorizSpacing, 0,
                  Child, MakeVarPop(&data->GUI.ST_COMMAND, &popButton, PHM_MIME_COMMAND, SIZE_COMMAND, tr(MSG_CO_MimeCmd)),
                  Child, PopaslObject,
                    MUIA_Popasl_StartHook, &MimeCommandReqStartHook,
                    MUIA_Popasl_StopHook,  &MimeCommandReqStopHook,
                    MUIA_Popstring_Button, PopButton(MUII_PopFile),
                  End,
                End,
              End,

              Child, VSpace(0),
            End,
          End,

          Child, RectangleObject,
            MUIA_Rectangle_HBar, TRUE,
            MUIA_FixHeight,      4,
          End,

          Child, HGroup,
            Child, Label2(tr(MSG_CO_DefaultViewer)),
            Child, HGroup,
              MUIA_Group_HorizSpacing, 0,
              Child, MakeVarPop(&data->GUI.ST_DEFVIEWER, &popButton, PHM_MIME_DEFVIEWER, SIZE_COMMAND, tr(MSG_CO_DefaultViewer)),
              Child, PopaslObject,
                MUIA_Popasl_StartHook, &MimeDefViewerReqStartHook,
                MUIA_Popasl_StopHook,  &MimeDefViewerReqStopHook,
                MUIA_Popstring_Button, PopButton(MUII_PopFile),
              End,
            End,
          End,

        End,
      End,

    End,
  End;

  if(obj != NULL)
  {
    SetHelp(data->GUI.ST_CTYPE       ,MSG_HELP_CO_ST_CTYPE       );
    SetHelp(data->GUI.ST_EXTENS      ,MSG_HELP_CO_ST_EXTENS      );
    SetHelp(data->GUI.ST_COMMAND     ,MSG_HELP_CO_ST_COMMAND     );
    SetHelp(data->GUI.BT_MADD        ,MSG_HELP_CO_BT_MADD        );
    SetHelp(data->GUI.BT_MDEL        ,MSG_HELP_CO_BT_MDEL        );
    SetHelp(data->GUI.BT_MIMEIMPORT  ,MSG_HELP_CO_BT_MIMEIMPORT  );
    SetHelp(data->GUI.ST_DEFVIEWER   ,MSG_HELP_CO_ST_DEFVIEWER   );
    SetHelp(data->GUI.ST_DESCRIPTION ,MSG_HELP_CO_ST_DESCRIPTION );

    DoMethod(obj, MUIM_MultiSet, MUIA_Disabled, TRUE, data->GUI.GR_MIME, data->GUI.BT_MDEL, NULL);
    DoMethod(data->GUI.LV_MIME     ,MUIM_Notify, MUIA_NList_Active   ,MUIV_EveryTime, MUIV_Notify_Application, 2, MUIM_CallHook, &GetMimeTypeEntryHook);
    DoMethod(data->GUI.ST_CTYPE    ,MUIM_Notify, MUIA_String_Contents,MUIV_EveryTime, MUIV_Notify_Application, 2, MUIM_CallHook, &PutMimeTypeEntryHook);
    DoMethod(data->GUI.ST_EXTENS   ,MUIM_Notify, MUIA_String_Contents,MUIV_EveryTime, MUIV_Notify_Application, 2, MUIM_CallHook, &PutMimeTypeEntryHook);
    DoMethod(data->GUI.ST_COMMAND  ,MUIM_Notify, MUIA_String_Contents,MUIV_EveryTime, MUIV_Notify_Application, 2, MUIM_CallHook, &PutMimeTypeEntryHook);
    DoMethod(data->GUI.ST_DESCRIPTION,MUIM_Notify,MUIA_String_Contents,MUIV_EveryTime, MUIV_Notify_Application, 2, MUIM_CallHook, &PutMimeTypeEntryHook);
    DoMethod(data->GUI.ST_DEFVIEWER,MUIM_Notify, MUIA_String_Contents,MUIV_EveryTime, MUIV_Notify_Application, 2, MUIM_CallHook, &PutMimeTypeEntryHook);
    DoMethod(data->GUI.BT_MADD     ,MUIM_Notify, MUIA_Pressed        ,FALSE         , MUIV_Notify_Application, 2, MUIM_CallHook, &AddMimeTypeHook);
    DoMethod(data->GUI.BT_MDEL     ,MUIM_Notify, MUIA_Pressed        ,FALSE         , MUIV_Notify_Application, 2, MUIM_CallHook, &DelMimeTypeHook);
    DoMethod(data->GUI.BT_MIMEIMPORT, MUIM_Notify, MUIA_Pressed, FALSE, MUIV_Notify_Application, 2, MUIM_CallHook, &ImportMimeTypesHook);
  }

  RETURN(obj);
  return obj;
}

///
/// CO_PageScripts
Object *CO_PageScripts(struct CO_ClassData *data)
{
  Object *grp;
  static const char *const stype[3] =
  {
    "ARexx", "AmigaDOS", NULL
  };

  ENTER();

  if((grp = VGroup,
    MUIA_HelpNode, "Configuration#Scripts",

    ConfigPageHeaderObject("config_scripts_big", G->theme.configImages[CI_SCRIPTSBIG], tr(MSG_CO_SCRIPTS_TITLE), tr(MSG_CO_SCRIPTS_SUMMARY)),

     Child, ScrollgroupObject,
       MUIA_Scrollgroup_FreeHoriz, FALSE,
       MUIA_Scrollgroup_AutoBars, TRUE,
       MUIA_Scrollgroup_Contents, VGroupV,

        Child, VGroup,
          Child, NListviewObject,
            MUIA_CycleChain, TRUE,
            MUIA_NListview_NList, data->GUI.LV_REXX = ScriptListObject,
            End,
          End,
          Child, ColGroup(2),

            Child, Label2(tr(MSG_CO_Name)),
            Child, HGroup,
              Child, data->GUI.ST_RXNAME = MakeString(SIZE_NAME,tr(MSG_CO_Name)),
              Child, data->GUI.CY_ISADOS = CycleObject,
                MUIA_CycleChain,    TRUE,
                MUIA_Weight,        25,
                MUIA_Font,          MUIV_Font_Button,
                MUIA_Cycle_Entries, stype,
              End,
            End,

            Child, Label2(tr(MSG_CO_Script)),
            Child, HGroup,
              MUIA_Group_HorizSpacing, 0,
              Child, MakeVarPop(&data->GUI.ST_SCRIPT, &data->GUI.PO_SCRIPT, PHM_SCRIPTS, SIZE_PATHFILE, tr(MSG_CO_Script)),
              Child, PopaslObject,
                 MUIA_Popasl_Type,       ASL_FileRequest,
                 MUIA_Popasl_StartHook,  &ScriptsReqStartHook,
                 MUIA_Popasl_StopHook,   &ScriptsReqStopHook,
                 MUIA_Popstring_Button,  PopButton(MUII_PopFile),
              End,
            End,

            Child, HSpace(1),
            Child, MakeCheckGroup(&data->GUI.CH_CONSOLE, tr(MSG_CO_OpenConsole)),

            Child, HSpace(1),
            Child, MakeCheckGroup(&data->GUI.CH_WAITTERM, tr(MSG_CO_WaitTerm)),

          End,
        End,

      End,
    End,

  End))
  {
    int i;

    for(i = 1; i <= MAXRX; i++)
      DoMethod(data->GUI.LV_REXX, MUIM_NList_InsertSingle, i, MUIV_NList_Insert_Bottom);

    SetHelp(data->GUI.ST_RXNAME    ,MSG_HELP_CO_ST_RXNAME    );
    SetHelp(data->GUI.ST_SCRIPT    ,MSG_HELP_CO_ST_SCRIPT    );
    SetHelp(data->GUI.CY_ISADOS    ,MSG_HELP_CO_CY_ISADOS    );
    SetHelp(data->GUI.CH_CONSOLE   ,MSG_HELP_CO_CH_CONSOLE   );
    SetHelp(data->GUI.CH_WAITTERM  ,MSG_HELP_CO_CH_WAITTERM  );
    SetHelp(data->GUI.LV_REXX      ,MSG_HELP_CO_LV_REXX      );

    DoMethod(data->GUI.LV_REXX     ,MUIM_Notify,MUIA_NList_Active   ,MUIV_EveryTime,MUIV_Notify_Application,2,MUIM_CallHook,&CO_GetRXEntryHook);
    DoMethod(data->GUI.ST_RXNAME   ,MUIM_Notify,MUIA_String_Contents,MUIV_EveryTime,MUIV_Notify_Application,2,MUIM_CallHook,&CO_PutRXEntryHook);
    DoMethod(data->GUI.ST_SCRIPT   ,MUIM_Notify,MUIA_String_Contents,MUIV_EveryTime,MUIV_Notify_Application,2,MUIM_CallHook,&CO_PutRXEntryHook);
    DoMethod(data->GUI.CY_ISADOS   ,MUIM_Notify,MUIA_Cycle_Active   ,MUIV_EveryTime,MUIV_Notify_Application,2,MUIM_CallHook,&CO_PutRXEntryHook);
    DoMethod(data->GUI.CH_CONSOLE  ,MUIM_Notify,MUIA_Selected       ,MUIV_EveryTime,MUIV_Notify_Application,2,MUIM_CallHook,&CO_PutRXEntryHook);
    DoMethod(data->GUI.CH_WAITTERM ,MUIM_Notify,MUIA_Selected       ,MUIV_EveryTime,MUIV_Notify_Application,2,MUIM_CallHook,&CO_PutRXEntryHook);
  }

  RETURN(grp);
  return grp;
}

///
/// CO_PageMixed
Object *CO_PageMixed(struct CO_ClassData *data)
{
  static const char *trwopt[4];
  Object *obj;
  Object *popButton;
  Object *codesetPopButton;

  ENTER();

  trwopt[TWM_HIDE] = tr(MSG_CO_TWNever);
  trwopt[TWM_AUTO] = tr(MSG_CO_TWAuto);
  trwopt[TWM_SHOW] = tr(MSG_CO_TWAlways);
  trwopt[3] = NULL;

  obj = VGroup,
    MUIA_HelpNode, "Configuration#Miscellaneous",

    ConfigPageHeaderObject("config_misc_big", G->theme.configImages[CI_MISCBIG], tr(MSG_CO_MIXED_TITLE), tr(MSG_CO_MIXED_SUMMARY)),

    Child, ScrollgroupObject,
      MUIA_Scrollgroup_FreeHoriz, FALSE,
      MUIA_Scrollgroup_AutoBars, TRUE,
      MUIA_Scrollgroup_Contents, VGroupV,

        Child, ColGroup(2), GroupFrameT(tr(MSG_CO_Paths)),
          Child, Label2(tr(MSG_CO_TempDir)),
          Child, PopaslObject,
            MUIA_Popasl_Type     ,ASL_FileRequest,
            MUIA_Popstring_String,data->GUI.ST_TEMPDIR = MakeString(SIZE_PATH, tr(MSG_CO_TempDir)),
            MUIA_Popstring_Button,PopButton(MUII_PopDrawer),
            ASLFR_DrawersOnly, TRUE,
          End,

          Child, Label2(tr(MSG_CO_Detach)),
          Child, PopaslObject,
            MUIA_Popasl_Type     ,ASL_FileRequest,
            MUIA_Popstring_String,data->GUI.ST_DETACHDIR = MakeString(SIZE_PATH, tr(MSG_CO_Detach)),
            MUIA_Popstring_Button,PopButton(MUII_PopDrawer),
            ASLFR_DrawersOnly, TRUE,
          End,

          Child, Label2(tr(MSG_CO_Attach)),
          Child, PopaslObject,
            MUIA_Popasl_Type     ,ASL_FileRequest,
            MUIA_Popstring_String,data->GUI.ST_ATTACHDIR = MakeString(SIZE_PATH, tr(MSG_CO_Attach)),
            MUIA_Popstring_Button,PopButton(MUII_PopDrawer),
            ASLFR_DrawersOnly, TRUE,
          End,

          Child, Label2(tr(MSG_CO_UPDATE_DOWNLOAD_PATH)),
          Child, PopaslObject,
            MUIA_Popasl_Type, ASL_FileRequest,
            MUIA_Popstring_String, data->GUI.ST_UPDATEDOWNLOADPATH = MakeString(SIZE_PATH, tr(MSG_CO_UPDATE_DOWNLOAD_PATH)),
            MUIA_Popstring_Button, PopButton(MUII_PopDrawer),
            ASLFR_DrawersOnly, TRUE,
          End,

        End,

        Child, VGroup, GroupFrameT(tr(MSG_CO_EXTEDITOR)),
          Child, ColGroup(2),
            Child, Label2(tr(MSG_CO_ExternalEditor)),
            Child, PopaslObject,
              MUIA_Popasl_Type, ASL_FileRequest,
              MUIA_Popstring_String,data->GUI.ST_EDITOR = MakeString(SIZE_PATHFILE, tr(MSG_CO_ExternalEditor)),
              MUIA_Popstring_Button, PopButton(MUII_PopFile),
            End,

            Child, HSpace(1),
            Child, HGroup,
              Child, HGroup,
                Child, data->GUI.CH_DEFCODESET_EDITOR = MakeCheck(tr(MSG_CO_EXTEDITOR_CODESET)),
                Child, Label1(tr(MSG_CO_EXTEDITOR_CODESET)),
              End,
              Child, MakeCodesetPop(&data->GUI.TX_DEFCODESET_EDITOR, &codesetPopButton),
            End,

          End,
        End,

        Child, VGroup, GroupFrameT(tr(MSG_CO_AppIcon)),
          Child, ColGroup(2),
            Child, data->GUI.CH_WBAPPICON = MakeCheck(tr(MSG_CO_WBAPPICON)),
            Child, LLabel1(tr(MSG_CO_WBAPPICON)),

            Child, HSpace(0),
            Child, ColGroup(2),

              Child, Label2(tr(MSG_CO_APPICONTEXT)),
              Child, MakeVarPop(&data->GUI.ST_APPICON, &popButton, PHM_MAILSTATS, SIZE_DEFAULT/2, tr(MSG_CO_APPICONTEXT)),

              Child, HGroup,
                Child, data->GUI.CH_APPICONPOS = MakeCheck(tr(MSG_CO_PositionX)),
                Child, Label2(tr(MSG_CO_PositionX)),
              End,
              Child, HGroup,
                Child, data->GUI.ST_APPX = BetterStringObject,
                  StringFrame,
                  MUIA_CycleChain,          TRUE,
                  MUIA_ControlChar,         ShortCut("_X"),
                  MUIA_FixWidthTxt,         "0000",
                  MUIA_String_MaxLen,       4+1,
                  MUIA_String_AdvanceOnCR,  TRUE,
                  MUIA_String_Integer,      0,
                  MUIA_String_Accept,       "0123456789",
                End,
                Child, Label2("_Y"),
                Child, HGroup,
                  MUIA_Group_Spacing, 1,
                  Child, data->GUI.ST_APPY = BetterStringObject,
                    StringFrame,
                    MUIA_CycleChain,          TRUE,
                    MUIA_ControlChar,         ShortCut("_Y"),
                    MUIA_FixWidthTxt,         "0000",
                    MUIA_String_MaxLen,       4+1,
                    MUIA_String_AdvanceOnCR,  TRUE,
                    MUIA_String_Integer,      0,
                    MUIA_String_Accept,       "0123456789",
                  End,
                  Child, data->GUI.BT_APPICONGETPOS = PopButton(MUII_PopUp),
                End,
                Child, HSpace(0),
              End,

            End,
          End,
          #if defined(__amigaos4__)
          Child, MakeCheckGroup(&data->GUI.CH_DOCKYICON, tr(MSG_CO_DOCKYICON)),
          #endif
          Child, MakeCheckGroup(&data->GUI.CH_CLGADGET, tr(MSG_CO_CloseGadget)),
        End,

        Child, VGroup, GroupFrameT(tr(MSG_CO_SaveDelete)),
          Child, HGroup,
            Child, data->GUI.CH_CONFIRM = MakeCheck(tr(MSG_CO_ConfirmDelPart1)),
            Child, Label2(tr(MSG_CO_ConfirmDelPart1)),
            Child, data->GUI.NB_CONFIRMDEL = MakeNumeric(1, 50, FALSE),
            Child, Label2(tr(MSG_CO_ConfirmDelPart2)),
            Child, HSpace(0),
          End,
          Child, MakeCheckGroup(&data->GUI.CH_REMOVE, tr(MSG_CO_Remove)),
        End,
        Child, HGroup, GroupFrameT(tr(MSG_CO_XPK)),
          Child, ColGroup(2),
            Child, Label1(tr(MSG_CO_XPKPack)),
            Child, HGroup,
              Child, MakeXPKPop(&data->GUI.TX_PACKER, FALSE),
              Child, data->GUI.NB_PACKER = MakeNumeric(0, 100, TRUE),
              Child, HSpace(0),
            End,

            Child, Label1(tr(MSG_CO_XPKPackEnc)),
            Child, HGroup,
              Child, MakeXPKPop(&data->GUI.TX_ENCPACK, TRUE),
              Child, data->GUI.NB_ENCPACK = MakeNumeric(0, 100, TRUE),
              Child, HSpace(0),
            End,

            Child, Label1(tr(MSG_CO_Archiver)),
            Child, HGroup,
              Child, MakeVarPop(&data->GUI.ST_ARCHIVER, &popButton, PHM_ARCHIVE, SIZE_COMMAND, tr(MSG_CO_Archiver)),
              Child, MakeCheckGroup(&data->GUI.CH_ARCHIVERPROGRESS, tr(MSG_CO_SHOW_ARCHIVER_PROGRESS)),
            End,
          End,
        End,
        Child, ColGroup(2), GroupFrameT(tr(MSG_CO_MIXED_CONNECTIONS)),
          Child, Label(tr(MSG_CO_TransferWin)),
          Child, data->GUI.CY_TRANSWIN = MakeCycle(trwopt, tr(MSG_CO_TransferWin)),
        End,

        Child, HVSpace,

      End,
    End,
  End;

  if(obj != NULL)
  {
    set(codesetPopButton, MUIA_ControlChar, ShortCut(tr(MSG_CO_EXTEDITOR_CODESET)));

    SetHelp(data->GUI.ST_TEMPDIR,       MSG_HELP_CO_ST_TEMPDIR);
    SetHelp(data->GUI.ST_DETACHDIR,     MSG_HELP_CO_ST_DETACHDIR);
    SetHelp(data->GUI.ST_ATTACHDIR,     MSG_HELP_CO_ST_ATTACHDIR);
    SetHelp(data->GUI.CH_WBAPPICON,     MSG_HELP_CO_CH_WBAPPICON);
    SetHelp(data->GUI.ST_APPX,          MSG_HELP_CO_ST_APP);
    SetHelp(data->GUI.ST_APPY,          MSG_HELP_CO_ST_APP);
    SetHelp(data->GUI.CH_APPICONPOS,    MSG_HELP_CO_ST_APP);
    #if defined(__amigaos4__)
    SetHelp(data->GUI.CH_DOCKYICON,     MSG_HELP_CO_CH_DOCKYICON);
    #endif // __amigaos4__
    SetHelp(data->GUI.CH_CLGADGET,      MSG_HELP_CO_CH_CLGADGET);
    SetHelp(data->GUI.CH_CONFIRM,       MSG_HELP_CO_CH_CONFIRM);
    SetHelp(data->GUI.NB_CONFIRMDEL,    MSG_HELP_CO_NB_CONFIRMDEL);
    SetHelp(data->GUI.CH_REMOVE,        MSG_HELP_CO_CH_REMOVE);
    SetHelp(data->GUI.TX_ENCPACK,       MSG_HELP_CO_TX_ENCPACK);
    SetHelp(data->GUI.TX_PACKER,        MSG_HELP_CO_TX_PACKER);
    SetHelp(data->GUI.NB_ENCPACK,       MSG_HELP_CO_NB_ENCPACK);
    SetHelp(data->GUI.NB_PACKER,        MSG_HELP_CO_NB_ENCPACK);
    SetHelp(data->GUI.ST_ARCHIVER,      MSG_HELP_CO_ST_ARCHIVER);
    SetHelp(data->GUI.ST_APPICON,       MSG_HELP_CO_ST_APPICON);
    SetHelp(data->GUI.BT_APPICONGETPOS, MSG_HELP_CO_BT_APPICONGETPOS);
    SetHelp(data->GUI.CY_TRANSWIN,      MSG_HELP_CO_CH_TRANSWIN);
    SetHelp(data->GUI.ST_EDITOR,        MSG_HELP_CO_ST_EDITOR_EXT);
    SetHelp(data->GUI.CH_DEFCODESET_EDITOR, MSG_HELP_CO_CH_DEFCODESET_EDITOR);
    SetHelp(data->GUI.TX_DEFCODESET_EDITOR, MSG_HELP_CO_TX_DEFCODESET_EDITOR);

    DoMethod(obj, MUIM_MultiSet, MUIA_Disabled, TRUE, data->GUI.ST_APPX, data->GUI.ST_APPY, data->GUI.ST_APPICON, data->GUI.BT_APPICONGETPOS, NULL);
    DoMethod(data->GUI.CH_WBAPPICON, MUIM_Notify, MUIA_Selected, MUIV_EveryTime, MUIV_Notify_Application, 9, MUIM_MultiSet, MUIA_Disabled, MUIV_NotTriggerValue, data->GUI.ST_APPX, data->GUI.ST_APPY, data->GUI.ST_APPICON, data->GUI.CH_APPICONPOS, data->GUI.BT_APPICONGETPOS, NULL);
    DoMethod(data->GUI.BT_APPICONGETPOS, MUIM_Notify, MUIA_Pressed, FALSE, MUIV_Notify_Application, 2, MUIM_CallHook, &GetAppIconPosHook);
    DoMethod(data->GUI.CH_CONFIRM, MUIM_Notify, MUIA_Selected, MUIV_EveryTime, data->GUI.NB_CONFIRMDEL, 3, MUIM_Set, MUIA_Disabled, MUIV_NotTriggerValue);
    DoMethod(data->GUI.CH_DEFCODESET_EDITOR, MUIM_Notify, MUIA_Selected, MUIV_EveryTime, data->GUI.TX_DEFCODESET_EDITOR, 3, MUIM_Set, MUIA_Disabled, MUIV_NotTriggerValue);
    DoMethod(data->GUI.CH_DEFCODESET_EDITOR, MUIM_Notify, MUIA_Selected, MUIV_EveryTime, codesetPopButton, 3, MUIM_Set, MUIA_Disabled, MUIV_NotTriggerValue);

    #if defined(__amigaos4__)
    set(data->GUI.CH_DOCKYICON, MUIA_Disabled, G->applicationID == 0);
    #endif // __amigaos4__

    // disable the XPK popups if xpkmaster.library is not available
    if(XpkBase == NULL)
    {
      set(data->GUI.NB_PACKER, MUIA_Disabled, TRUE);
      set(data->GUI.NB_ENCPACK, MUIA_Disabled, TRUE);
    }

    // disabled the codeset select items
    set(data->GUI.TX_DEFCODESET_EDITOR, MUIA_Disabled, TRUE);
    set(codesetPopButton, MUIA_Disabled, TRUE);
  }

  RETURN(obj);
  return obj;
}
///
/// CO_PageLookFeel
Object *CO_PageLookFeel(struct CO_ClassData *data)
{
  static const char *rtitles[3];
  static const char *sizef[6];
  static const char *infob[5];
  static const char *qsearchb[4];
  static const char *folderf[6];
  Object *obj;
  Object *popButton;

  ENTER();

  rtitles[0] = tr(MSG_CO_LOOKFEEL_THEMES);
  rtitles[1] = tr(MSG_CO_LOOKFEEL_MAINWINDOW);
  rtitles[2] = NULL;

  sizef[0] = tr(MSG_CO_SIZEFORMAT01);
  sizef[1] = tr(MSG_CO_SIZEFORMAT02);
  sizef[2] = tr(MSG_CO_SIZEFORMAT03);
  sizef[3] = tr(MSG_CO_SIZEFORMAT04);
  sizef[4] = tr(MSG_CO_SIZEFORMAT05);
  sizef[5] = NULL;

  infob[0] = tr(MSG_CO_INFOBARPOS04);
  infob[1] = tr(MSG_CO_INFOBARPOS01);
  infob[2] = tr(MSG_CO_INFOBARPOS02);
  infob[3] = tr(MSG_CO_INFOBARPOS03);
  infob[4] = NULL;

  qsearchb[0] = tr(MSG_CO_QUICKSEARCHBARPOS01);
  qsearchb[1] = tr(MSG_CO_QUICKSEARCHBARPOS02);
  qsearchb[2] = tr(MSG_CO_QUICKSEARCHBARPOS03);
  qsearchb[3] = NULL;

  folderf[0] = tr(MSG_CO_FOLDERINFO01);
  folderf[1] = tr(MSG_CO_FOLDERINFO02);
  folderf[2] = tr(MSG_CO_FOLDERINFO03);
  folderf[3] = tr(MSG_CO_FOLDERINFO04);
  folderf[4] = tr(MSG_CO_FOLDERINFO05);
  folderf[5] = NULL;

  obj = VGroup,
    MUIA_HelpNode, "Configuration#LookFeel",

    ConfigPageHeaderObject("config_lookfeel_big", G->theme.configImages[CI_LOOKFEELBIG], tr(MSG_CO_LOOKFEEL_TITLE), tr(MSG_CO_LOOKFEEL_SUMMARY)),

    Child, RegisterGroup(rtitles),
      MUIA_CycleChain, TRUE,

      // Themes settings
      Child, ScrollgroupObject,
        MUIA_Scrollgroup_FreeHoriz, FALSE,
        MUIA_Scrollgroup_AutoBars, TRUE,
        MUIA_Scrollgroup_Contents, VGroupV,

          Child, data->GUI.GR_THEMES = ThemeListGroupObject,
          End,

        End,
      End,

      // Main window settings
      Child, ScrollgroupObject,
        MUIA_Scrollgroup_FreeHoriz, FALSE,
        MUIA_Scrollgroup_AutoBars, TRUE,
        MUIA_Scrollgroup_Contents, VGroupV,

          // List column settings
          Child, HGroup, GroupFrameT(tr(MSG_CO_LISTCOLUMNSETTINGS)),

            // Folder list columns
            Child, VGroup,

              Child, HBarT(tr(MSG_FolderList)), End,

              Child, ColGroup(3),
                MUIA_ShortHelp, tr(MSG_HELP_CO_CG_FO),

                Child, MakeStaticCheck(),
                Child, data->GUI.CY_FOLDERINFO = MakeCycle(folderf, tr(MSG_CO_FOLDERLABEL)),
                Child, HSpace(0),

                Child, data->GUI.CH_FCOLS[1] = MakeCheck(""),
                Child, LLabel(tr(MSG_Total)),
                Child, HSpace(0),

                Child, data->GUI.CH_FCOLS[2] = MakeCheck(""),
                Child, LLabel(tr(MSG_Unread)),
                Child, HSpace(0),

                Child, data->GUI.CH_FCOLS[3] = MakeCheck(""),
                Child, LLabel(tr(MSG_New)),
                Child, HSpace(0),

                Child, data->GUI.CH_FCOLS[4] = MakeCheck(""),
                Child, LLabel(tr(MSG_Size)),
                Child, HSpace(0),

                Child, data->GUI.CH_FCNTMENU = MakeCheck(""),
                Child, LLabel(tr(MSG_CO_CONTEXTMENU)),
                Child, HSpace(0),

              End,

              Child, HVSpace,

            End,

            Child, HSpace(8),

            // Message list columns
            Child, VGroup,

              Child, HBarT(tr(MSG_MessageList)), End,

              Child, ColGroup(3),
                MUIA_ShortHelp, tr(MSG_HELP_CO_CG_MA),

                Child, MakeStaticCheck(),
                Child, LLabel(tr(MSG_Status)),
                Child, HSpace(0),

                Child, data->GUI.CH_MCOLS[1] = MakeCheck(""),
                Child, LLabel(tr(MSG_SenderRecpt)),
                Child, HSpace(0),

                Child, data->GUI.CH_MCOLS[2] = MakeCheck(""),
                Child, LLabel(tr(MSG_ReturnAddress)),
                Child, HSpace(0),

                Child, data->GUI.CH_MCOLS[3] = MakeCheck(""),
                Child, LLabel(tr(MSG_Subject)),
                Child, HSpace(0),

                Child, data->GUI.CH_MCOLS[4] = MakeCheck(""),
                Child, LLabel(tr(MSG_MessageDate)),
                Child, HSpace(0),

                Child, data->GUI.CH_MCOLS[5] = MakeCheck(""),
                Child, LLabel(tr(MSG_Size)),
                Child, HSpace(0),

                Child, data->GUI.CH_MCOLS[6] = MakeCheck(""),
                Child, LLabel(tr(MSG_Filename)),
                Child, HSpace(0),

                Child, data->GUI.CH_MCOLS[7] = MakeCheck(""),
                Child, LLabel(tr(MSG_CO_DATE_SNTRCVD)),
                Child, HSpace(0),

                Child, data->GUI.CH_MCNTMENU = MakeCheck(""),
                Child, LLabel(tr(MSG_CO_CONTEXTMENU)),
                Child, HSpace(0),
              End,
            End,
          End,

          // InfoBar settings
          Child, ColGroup(2), GroupFrameT(tr(MSG_CO_INFOBAR)),
            Child, Label1(tr(MSG_CO_INFOBARPOS)),
            Child, data->GUI.CY_INFOBARPOS = MakeCycle(infob, tr(MSG_CO_INFOBARPOS)),

            Child, Label2(tr(MSG_CO_FOLDERLABEL)),
            Child, data->GUI.PO_INFOBARTXT = MakeVarPop(&data->GUI.ST_INFOBARTXT, &popButton, PHM_MAILSTATS, SIZE_DEFAULT, tr(MSG_CO_FOLDERLABEL)),
          End,

          // QuicksearchBar settings
          Child, ColGroup(2), GroupFrameT(tr(MSG_CO_QUICKSEARCHBAR)),
            Child, Label1(tr(MSG_CO_QUICKSEARCHBARPOS)),
            Child, data->GUI.CY_QUICKSEARCHBARPOS = MakeCycle(qsearchb, tr(MSG_CO_QUICKSEARCHBARPOS)),
          End,

          Child, ColGroup(2),
            GroupFrameT(tr(MSG_CO_GENLISTCFG)),

            Child, Label1(tr(MSG_CO_SIZEFORMAT)),
            Child, data->GUI.CY_SIZE = MakeCycle(sizef, tr(MSG_CO_SIZEFORMAT)),

            Child, HSpace(1),
            Child, MakeCheckGroup(&data->GUI.CH_EMBEDDEDREADPANE, tr(MSG_CO_SHOWEMBEDDEDREADPANE)),

            Child, HSpace(1),
            Child, MakeCheckGroup(&data->GUI.CH_FIXFLIST, tr(MSG_CO_FixedFontList)),

            Child, HSpace(1),
            Child, MakeCheckGroup(&data->GUI.CH_BEAT, tr(MSG_CO_SwatchBeat)),

            Child, HSpace(1),
            Child, MakeCheckGroup(&data->GUI.CH_RELDATETIME, tr(MSG_CO_RELDATETIME)),

            Child, HSpace(1),
            Child, MakeCheckGroup(&data->GUI.CH_ABOOKLOOKUP, tr(MSG_CO_ABOOKLOOKUP)),

            Child, HSpace(1),
            Child, MakeCheckGroup(&data->GUI.CH_FOLDERDBLCLICK, tr(MSG_CO_FOLDERDBLCLICK)),

          End,

          Child, HVSpace,

        End,
      End,
    End,

  End;

  if(obj != NULL)
  {
    SetHelp(data->GUI.CY_INFOBARPOS,        MSG_HELP_CO_CH_INFOBAR);
    SetHelp(data->GUI.ST_INFOBARTXT,        MSG_HELP_CO_ST_INFOBARTXT);
    SetHelp(data->GUI.CY_QUICKSEARCHBARPOS, MSG_HELP_CO_CH_QUICKSEARCHBAR);
    SetHelp(data->GUI.CH_EMBEDDEDREADPANE,  MSG_HELP_CO_CH_EMBEDDEDREADPANE);
    SetHelp(data->GUI.CY_SIZE,              MSG_HELP_CO_CY_SIZE);
    SetHelp(data->GUI.CH_FIXFLIST,          MSG_HELP_CO_CH_FIXFLIST);
    SetHelp(data->GUI.CH_BEAT,              MSG_HELP_CO_CH_BEAT);
    SetHelp(data->GUI.CH_RELDATETIME,       MSG_HELP_CO_CH_RELDATETIME);
    SetHelp(data->GUI.CH_ABOOKLOOKUP,       MSG_HELP_CO_CH_ABOOKLOOKUP);
    SetHelp(data->GUI.CH_FCNTMENU,          MSG_HELP_CO_CONTEXTMENU);
    SetHelp(data->GUI.CH_MCNTMENU,          MSG_HELP_CO_CONTEXTMENU);
    SetHelp(data->GUI.CY_FOLDERINFO,        MSG_HELP_CO_CY_FOLDERINFO);

    DoMethod(data->GUI.CY_INFOBARPOS, MUIM_Notify, MUIA_Cycle_Active, MUIV_EveryTime, MUIV_Notify_Application, 3, MUIM_CallHook, &InfoBarPosHook, MUIV_TriggerValue);
  }

  RETURN(obj);
  return obj;
}
///
/// CO_PageUpdate
Object *CO_PageUpdate(struct CO_ClassData *data)
{
  Object *obj;
  static const char *updateInterval[5];

  ENTER();

  updateInterval[0] = tr(MSG_CO_UPDATE_NEVER);
  updateInterval[1] = tr(MSG_CO_UPDATE_DAILY);
  updateInterval[2] = tr(MSG_CO_UPDATE_WEEKLY);
  updateInterval[3] = tr(MSG_CO_UPDATE_MONTHLY);
  updateInterval[4] = NULL;

  obj = VGroup,
    MUIA_HelpNode, "Configuration#Updates",

    ConfigPageHeaderObject("config_update_big", G->theme.configImages[CI_UPDATEBIG], tr(MSG_CO_UPDATE_TITLE), tr(MSG_CO_UPDATE_SUMMARY)),

    Child, ScrollgroupObject,
      MUIA_Scrollgroup_FreeHoriz, FALSE,
      MUIA_Scrollgroup_AutoBars, TRUE,
      MUIA_Scrollgroup_Contents, VGroupV,

        Child, VGroup, GroupFrameT(tr(MSG_CO_SOFTWAREUPDATE)),
          Child, HGroup,
            Child, LLabel1(tr(MSG_CO_SEARCHFORUPDATES)),
            Child, data->GUI.CY_UPDATEINTERVAL = MakeCycle(updateInterval, tr(MSG_CO_SEARCHFORUPDATES)),
            Child, data->GUI.BT_UPDATENOW = MakeButton(tr(MSG_CO_SEARCHNOW)),
          End,
          Child, data->GUI.TX_UPDATEINFO = TextObject,
            MUIA_Font, MUIV_Font_Tiny,
            MUIA_Text_Copy, FALSE,
          End,
          Child, VSpace(10),
          Child, ColGroup(2),
            Child, LLabel1(tr(MSG_CO_LASTSEARCH)),
            Child, data->GUI.TX_UPDATESTATUS = TextObject,
              MUIA_Text_Contents, tr(MSG_CO_LASTSTATUS_NOCHECK),
              MUIA_Text_Copy, FALSE,
            End,
            Child, HSpace(1),
            Child, data->GUI.TX_UPDATEDATE = TextObject,
            End,
          End,
        End,

        Child, HVSpace,

      End,
    End,

  End;

  if(obj != NULL)
  {
    SetHelp(data->GUI.CY_UPDATEINTERVAL, MSG_HELP_CH_UPDATECHECK);
    SetHelp(data->GUI.BT_UPDATENOW,      MSG_HELP_CO_BT_UPDATENOW);

    DoMethod(data->GUI.CY_UPDATEINTERVAL, MUIM_Notify, MUIA_Cycle_Active, MUIV_EveryTime, MUIV_Notify_Application, 3, MUIM_CallHook, &UpdateInfoHook, MUIV_TriggerValue);
    DoMethod(data->GUI.BT_UPDATENOW,      MUIM_Notify, MUIA_Pressed,      FALSE,          MUIV_Notify_Application, 2, MUIM_CallHook, &UpdateCheckHook);
  }

  RETURN(obj);
  return obj;
}

///
