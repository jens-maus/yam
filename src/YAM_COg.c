/***************************************************************************

 YAM - Yet Another Mailer
 Copyright (C) 1995-2000 Marcel Beck
 Copyright (C) 2000-2011 YAM Open Source Team

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
#include "mui/FilterList.h"
#include "mui/ImageArea.h"
#include "mui/MailTextEdit.h"
#include "mui/MimeTypeList.h"
#include "mui/PlaceholderPopupList.h"
#include "mui/ScriptList.h"
#include "mui/SearchControlGroup.h"
#include "mui/ThemeListGroup.h"
#include "mui/YAMApplication.h"

#include "BayesFilter.h"
#include "FolderList.h"
#include "ImageCache.h"
#include "Locale.h"
#include "MimeTypes.h"
#include "MailServers.h"
#include "MUIObjects.h"
#include "Requesters.h"
#include "Threads.h"
#include "UIDL.h"

#include "Debug.h"

/* local defines */
/// ConfigPageHeaderObject()
#define ConfigPageHeaderObject(id, image, title, summary) \
          Child, HGroup,                              \
            Child, MakeImageObject(id, image),        \
            Child, VGroup,                            \
              Child, TextObject,                      \
                MUIA_Text_PreParse, "\033b",          \
                MUIA_Text_Contents, (title),          \
                MUIA_Weight,        100,              \
              End,                                    \
              Child, TextObject,                      \
                MUIA_Text_Contents, (summary),        \
                MUIA_Font,          MUIV_Font_Tiny,   \
                MUIA_Weight,        100,              \
              End,                                    \
            End,                                      \
          End,                                        \
          Child, RectangleObject,                     \
            MUIA_Rectangle_HBar, TRUE,                \
            MUIA_FixHeight,      4,                   \
          End
///

/***************************************************************************
 Module: Configuration - GUI for sections
***************************************************************************/

/*** Hooks ***/
/// PO_InitFolderList
//  Creates a popup list of all folders
HOOKPROTONH(PO_InitFolderList, BOOL, Object *pop, Object *str)
{
  char *s;
  struct FolderNode *fnode;
  ULONG i;

  ENTER();

  // get the currently set string
  s = (char *)xget(str, MUIA_Text_Contents);

  DoMethod(pop, MUIM_List_Clear);

  LockFolderListShared(G->folders);

  i = 0;
  ForEachFolderNode(G->folders, fnode)
  {
    struct Folder *folder = fnode->folder;

    if(isGroupFolder(folder) == FALSE)
    {
      DoMethod(pop, MUIM_List_InsertSingle, folder->Name, MUIV_List_Insert_Bottom);

      // now we check whether we make that item active or not.
      if(s != NULL && stricmp(folder->Name, s) == 0)
      {
        set(pop, MUIA_List_Active, i);
        s = NULL;
      }
    }

    i++;
  }

  UnlockFolderList(G->folders);

  RETURN(TRUE);
  return TRUE;
}
MakeStaticHook(PO_InitFolderListHook, PO_InitFolderList);

///
/// PO_List2TextFunc
//  Copies listview selection to text gadget
HOOKPROTONH(PO_List2TextFunc, void, Object *list, Object *text)
{
  char *selection = NULL;

  ENTER();

  DoMethod(list, MUIM_List_GetEntry, MUIV_List_GetEntry_Active, &selection);

  if(selection)
    set(text, MUIA_Text_Contents, selection);

  LEAVE();
}
MakeStaticHook(PO_List2TextHook, PO_List2TextFunc);

///
/// PO_XPKOpenHook
//  Sets a popup listview accordingly to its string gadget
HOOKPROTONH(PO_XPKOpenFunc, BOOL, Object *list, Object *str)
{
  char *s;

  ENTER();

  if((s = (char *)xget(str, MUIA_Text_Contents)))
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
HOOKPROTONH(PO_XPKCloseFunc, void, Object *pop, Object *text)
{
  char *entry = NULL;

  ENTER();

  DoMethod(pop, MUIM_List_GetEntry, MUIV_List_GetEntry_Active, &entry);
  if(entry != NULL)
    set(text, MUIA_Text_Contents, entry);

  LEAVE();
}
MakeStaticHook(PO_XPKCloseHook, PO_XPKCloseFunc);

///
/// PO_MimeTypeListOpenHook
//  Sets the popup listview accordingly to the string gadget
HOOKPROTONH(PO_MimeTypeListOpenFunc, BOOL, Object *list, Object *str)
{
  char *s;

  ENTER();

  if((s = (char *)xget(str, MUIA_String_Contents)) != NULL)
  {
    int i;

    // we build the list totally from ground up.
    DoMethod(list, MUIM_List_Clear);

    // populate the list with the user's own defined MIME types but only if the source
    // string isn't the one in the YAM config window.
    if(G->CO == NULL || str != G->CO->GUI.ST_CTYPE)
    {
      struct Node *curNode;

      IterateList(&C->mimeTypeList, curNode)
      {
        struct MimeTypeNode *mt = (struct MimeTypeNode *)curNode;

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
        struct Node *curNode;

        IterateList(&C->mimeTypeList, curNode)
        {
          struct MimeTypeNode *mt = (struct MimeTypeNode *)curNode;

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
HOOKPROTONH(PO_MimeTypeListCloseFunc, void, Object *list, Object *str)
{
  char *entry = NULL;

  ENTER();

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

  LEAVE();
}
MakeStaticHook(PO_MimeTypeListCloseHook, PO_MimeTypeListCloseFunc);

///
/// PO_HandleVarHook
//  Pastes an entry from variable listview into string gadget
HOOKPROTONH(PO_HandleVarFunc, void, Object *pop, Object *string)
{
  char *var;

  ENTER();

  DoMethod(pop, MUIM_List_GetEntry, MUIV_List_GetEntry_Active, &var);
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

  LEAVE();
}
MakeStaticHook(PO_HandleVarHook, PO_HandleVarFunc);

///
/// PO_HandleScriptsOpenHook
// Hook which is used when the arexx/dos scripts popup window will
// be opened and populate the listview.
HOOKPROTONHNP(PO_HandleScriptsOpenFunc, BOOL, Object *list)
{
  ENTER();

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

  if((mode = MUI_Request(G->App, G->CO->GUI.WI, 0, tr(MSG_CO_ImportMIME), tr(MSG_CO_ImportMIMEGads), tr(MSG_CO_ImportMIMEReq))) != 0)
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
          struct Node *curNode;
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
          IterateList(&C->mimeTypeList, curNode)
          {
            struct MimeTypeNode *mtNode = (struct MimeTypeNode *)curNode;

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
MakeStaticHook(CO_PlaySoundHook,CO_PlaySoundFunc);
///
/// UpdateCheckHook
// initiates an interactive update check
HOOKPROTONHNONP(UpdateCheckFunc, void)
{
  ENTER();

  // get the configuration settings
  if(G->CO->VisiblePage == cp_Update)
    CO_GetConfig(FALSE);

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
/// ToggleSpamFilter
// enable/disable all spam filter relevant GUI elements according to the
// current spam filter settings
HOOKPROTONHNO(ToggleSpamFilterFunc, void, int *arg)
{
  struct CO_GUIData *gui = &G->CO->GUI;
  BOOL active = (arg[0] != 0);

  ENTER();

  if(active)
  {
    DoMethod(G->App, MUIM_MultiSet, MUIA_Disabled, FALSE, gui->BT_SPAMRESETTRAININGDATA,
                                                          gui->BT_OPTIMIZETRAININGDATA,
                                                          gui->CH_SPAMFILTERFORNEWMAIL,
                                                          gui->CH_SPAMABOOKISWHITELIST,
                                                          gui->CH_SPAMMARKONMOVE,
                                                          gui->CH_SPAMMARKASREAD,
                                                          gui->CH_MOVEHAMTOINCOMING,
                                                          NULL);
    set(gui->CH_FILTERHAM, MUIA_Disabled, xget(gui->CH_MOVEHAMTOINCOMING, MUIA_Selected) == FALSE);
  }
  else
  {
    // disable all spam filter controls
    DoMethod(G->App, MUIM_MultiSet, MUIA_Disabled, TRUE, gui->BT_SPAMRESETTRAININGDATA,
                                                         gui->BT_OPTIMIZETRAININGDATA,
                                                         gui->CH_SPAMFILTERFORNEWMAIL,
                                                         gui->CH_SPAMABOOKISWHITELIST,
                                                         gui->CH_SPAMMARKONMOVE,
                                                         gui->CH_SPAMMARKASREAD,
                                                         gui->CH_MOVEHAMTOINCOMING,
                                                         gui->CH_FILTERHAM,
                                                         NULL);
  }

  LEAVE();
}
MakeStaticHook(ToggleSpamFilterHook, ToggleSpamFilterFunc);

///
/// ResetSpamTrainingData
//  resets the spam training data
HOOKPROTONHNONP(ResetSpamTrainingDataFunc, void)
{
  ENTER();

  if(MUI_Request(G->App, G->CO->GUI.WI, 0, NULL, tr(MSG_YesNoReq2), tr(MSG_CO_SPAM_RESETTRAININGDATAASK)) != 0)
  {
    char buf[SIZE_DEFAULT];

    BayesFilterResetTrainingData();

    snprintf(buf, sizeof(buf), tr(MSG_CO_SPAM_STATISTICS), BayesFilterNumberOfHamClassifiedMails(), BayesFilterNumberOfHamClassifiedWords());
    set(G->CO->GUI.TX_SPAMGOODCOUNT, MUIA_Text_Contents, buf);
    snprintf(buf, sizeof(buf), tr(MSG_CO_SPAM_STATISTICS), BayesFilterNumberOfSpamClassifiedMails(), BayesFilterNumberOfSpamClassifiedWords());
    set(G->CO->GUI.TX_SPAMBADCOUNT, MUIA_Text_Contents, buf);
  }

  LEAVE();
}
MakeStaticHook(ResetSpamTrainingDataHook, ResetSpamTrainingDataFunc);

///
/// OptimizeSpamTrainingData
//  Optimizes the spam training data
HOOKPROTONHNONP(OptimizeSpamTrainingDataFunc, void)
{
  ENTER();

  if(MUI_Request(G->App, G->CO->GUI.WI, 0, NULL, tr(MSG_YesNoReq2), tr(MSG_CO_SPAM_OPTIMIZE_TRAININGDATA_ASK)) != 0)
  {
    char buf[SIZE_DEFAULT];

    set(G->App, MUIA_Application_Sleep, TRUE);

    BayesFilterOptimizeTrainingData();

    snprintf(buf, sizeof(buf), tr(MSG_CO_SPAM_STATISTICS), BayesFilterNumberOfHamClassifiedMails(), BayesFilterNumberOfHamClassifiedWords());
    set(G->CO->GUI.TX_SPAMGOODCOUNT, MUIA_Text_Contents, buf);
    snprintf(buf, sizeof(buf), tr(MSG_CO_SPAM_STATISTICS), BayesFilterNumberOfSpamClassifiedMails(), BayesFilterNumberOfSpamClassifiedWords());
    set(G->CO->GUI.TX_SPAMBADCOUNT, MUIA_Text_Contents, buf);

    set(G->App, MUIA_Application_Sleep, FALSE);
  }

  LEAVE();
}
MakeStaticHook(OptimizeSpamTrainingDataHook, OptimizeSpamTrainingDataFunc);

///
/// AddNewFilterToList
//  Adds a new entry to the global filter list
HOOKPROTONHNONP(AddNewFilterToList, void)
{
  struct FilterNode *filterNode;

  if((filterNode = CreateNewFilter()) != NULL)
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
/// CO_AddPOP3
//  Adds a new entry to the POP3 account list
HOOKPROTONHNONP(CO_AddPOP3, void)
{
  struct MailServerNode *msn;

  ENTER();

  if((msn = CreateNewMailServer(MST_POP3, CE, IsMinListEmpty(&CE->mailServerList))) != NULL)
  {
    if(IsMinListEmpty(&CE->mailServerList) == FALSE)
      strlcpy(msn->account, tr(MSG_NewEntry), sizeof(msn->account));

    DoMethod(G->CO->GUI.LV_POP3, MUIM_NList_InsertSingle, msn, MUIV_NList_Insert_Bottom);

    // add the server to the list
    AddTail((struct List *)&CE->mailServerList, (struct Node *)msn);

    // set the new entry active and make sure that the host gadget will be
    // set as the new active object of the window as that gadget will be used
    // to automatically set the account name.
    set(G->CO->GUI.LV_POP3, MUIA_NList_Active, MUIV_List_Active_Bottom);
    set(G->CO->GUI.WI, MUIA_Window_ActiveObject, G->CO->GUI.ST_POPHOST);
  }

  LEAVE();
}
MakeStaticHook(CO_AddPOP3Hook,CO_AddPOP3);

///
/// CO_DelPOP3
//  Deletes an entry from the POP3 account list
HOOKPROTONHNONP(CO_DelPOP3, void)
{
  struct CO_GUIData *gui = &G->CO->GUI;
  struct MailServerNode *msn = NULL;

  ENTER();

  DoMethod(gui->LV_POP3, MUIM_NList_GetEntry, MUIV_NList_GetEntry_Active, &msn);

  if(msn != NULL &&
     xget(gui->LV_POP3, MUIA_NList_Entries) > 1)
  {
    DoMethod(gui->LV_POP3, MUIM_NList_Remove, xget(gui->LV_POP3, MUIA_NList_Active));

    // remove it from the internal mail server list as well.
    Remove((struct Node *)msn);

    // delete a possibly existing UIDL database file
    DeleteUIDLfile(msn);

    FreeSysObject(ASOT_NODE, msn);
  }

  LEAVE();
}
MakeStaticHook(CO_DelPOP3Hook,CO_DelPOP3);

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
/// MDNRequestFunc
// update the MDN cycle gadgets according to the "never send" and "allow" check marks
HOOKPROTONHNO(MDNRequestFunc, void, int *arg)
{
  struct CO_GUIData *gui = &G->CO->GUI;
  BOOL active = (arg[0] != 0);

  ENTER();

  nnset(gui->CH_MDN_NEVER, MUIA_Selected, active == FALSE);
  nnset(gui->CH_MDN_ALLOW, MUIA_Selected, active == TRUE);
  DoMethod(G->App, MUIM_MultiSet, MUIA_Disabled, active == FALSE, gui->CY_MDN_NORECIPIENT,
                                                                  gui->CY_MDN_NODOMAIN,
                                                                  gui->CY_MDN_DELETE,
                                                                  gui->CY_MDN_OTHER,
                                                                  NULL);

  LEAVE();
}
MakeStaticHook(MDNRequestHook, MDNRequestFunc);
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
  Object *lv, *po, *but;

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
      MUIA_Listview_List, ListObject,
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
      struct Node *curNode;

      IterateList(G->xpkPackerList, curNode)
      {
        struct xpkPackerNode *xpkNode = (struct xpkPackerNode *)curNode;
        BOOL suits = TRUE;

        D(DBF_XPK, "XPK lib '%s' has flags %08lx", xpkNode->info.xpi_Name, xpkNode->info.xpi_Flags);

        if(encrypt == TRUE && isFlagClear(xpkNode->info.xpi_Flags, XPKIF_ENCRYPTION))
        {
          D(DBF_XPK, "'%s' has no encryption capabilities, excluded from encryption list", xpkNode->info.xpi_Name);
          suits = FALSE;
        }

        if(suits == TRUE)
          DoMethod(lv, MUIM_List_InsertSingle, xpkNode->info.xpi_Name, MUIV_List_Insert_Sorted);
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
  Object *lv;
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
      MUIA_NListview_NList, lv = PlaceholderPopupListObject,
        MUIA_PlaceholderPopupList_Mode, mode,
      End,
    End,

  End))
  {
    DoMethod(lv, MUIM_Notify, MUIA_NList_DoubleClick, TRUE, po, 2, MUIM_Popstring_Close, TRUE);
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
/// CO_PageFirstSteps
Object *CO_PageFirstSteps(struct CO_ClassData *data)
{
  Object *obj;
  static const char *tzone[34];

  ENTER();

  tzone[ 0] = tr(MSG_CO_TZoneM12);
  tzone[ 1] = tr(MSG_CO_TZoneM11);
  tzone[ 2] = tr(MSG_CO_TZoneM10);
  tzone[ 3] = tr(MSG_CO_TZoneM9);
  tzone[ 4] = tr(MSG_CO_TZoneM8);
  tzone[ 5] = tr(MSG_CO_TZoneM7);
  tzone[ 6] = tr(MSG_CO_TZoneM6);
  tzone[ 7] = tr(MSG_CO_TZoneM5);
  tzone[ 8] = tr(MSG_CO_TZoneM4);
  tzone[ 9] = tr(MSG_CO_TZoneM330);
  tzone[10] = tr(MSG_CO_TZoneM3);
  tzone[11] = tr(MSG_CO_TZoneM2);
  tzone[12] = tr(MSG_CO_TZoneM1);
  tzone[13] = tr(MSG_CO_TZone0);
  tzone[14] = tr(MSG_CO_TZone1);
  tzone[15] = tr(MSG_CO_TZone2);
  tzone[16] = tr(MSG_CO_TZone3);
  tzone[17] = tr(MSG_CO_TZone330);
  tzone[18] = tr(MSG_CO_TZone4);
  tzone[19] = tr(MSG_CO_TZone430);
  tzone[20] = tr(MSG_CO_TZone5);
  tzone[21] = tr(MSG_CO_TZone530);
  tzone[22] = tr(MSG_CO_TZone545);
  tzone[23] = tr(MSG_CO_TZone6);
  tzone[24] = tr(MSG_CO_TZone630);
  tzone[25] = tr(MSG_CO_TZone7);
  tzone[26] = tr(MSG_CO_TZone8);
  tzone[27] = tr(MSG_CO_TZone9);
  tzone[28] = tr(MSG_CO_TZone930);
  tzone[29] = tr(MSG_CO_TZone10);
  tzone[30] = tr(MSG_CO_TZone11);
  tzone[31] = tr(MSG_CO_TZone12);
  tzone[32] = tr(MSG_CO_TZone13);
  tzone[33] = NULL;

  obj = VGroup,
          MUIA_HelpNode, "CO00",

          ConfigPageHeaderObject("config_firststep_big", G->theme.configImages[CI_FIRSTSTEPBIG], tr(MSG_CO_FIRSTSTEPS_TITLE), tr(MSG_CO_FIRSTSTEPS_SUMMARY)),

          Child, ScrollgroupObject,
            MUIA_Scrollgroup_FreeHoriz, FALSE,
            MUIA_Scrollgroup_AutoBars, TRUE,
            MUIA_Scrollgroup_Contents, VGroupV,

              Child, ColGroup(2), GroupFrameT(tr(MSG_CO_MinConfig)),
                Child, Label2(tr(MSG_CO_RealName)),
                Child, data->GUI.ST_REALNAME = MakeString(SIZE_REALNAME,tr(MSG_CO_RealName)),

                Child, Label2(tr(MSG_CO_EmailAddress)),
                Child, data->GUI.ST_EMAIL    = MakeString(SIZE_ADDRESS,tr(MSG_CO_EmailAddress)),

                Child, Label2(tr(MSG_CO_POPServer)),
                Child, data->GUI.ST_POPHOST0  = MakeString(SIZE_HOST,tr(MSG_CO_POPServer)),

                Child, Label2(tr(MSG_CO_Password)),
                Child, data->GUI.ST_PASSWD0   = MakePassString(tr(MSG_CO_Password)),
              End,

              Child, ColGroup(2), GroupFrameT(tr(MSG_CO_SYSTEMSETTINGS)),
                Child, Label2(tr(MSG_CO_TimeZone)),
                Child, data->GUI.CY_TZONE = MakeCycle(tzone,tr(MSG_CO_TimeZone)),

                Child, HSpace(1),
                Child, MakeCheckGroup((Object **)&data->GUI.CH_DSTACTIVE, tr(MSG_CO_DSTACTIVE)),
              End,

              Child, HVSpace,

            End,
          End,

        End;

  if(obj != NULL)
  {
    SetHelp(data->GUI.ST_REALNAME,       MSG_HELP_CO_ST_REALNAME);
    SetHelp(data->GUI.ST_EMAIL,          MSG_HELP_CO_ST_EMAIL);
    SetHelp(data->GUI.ST_POPHOST0,       MSG_HELP_CO_ST_POPHOST);
    SetHelp(data->GUI.ST_PASSWD0,        MSG_HELP_CO_ST_PASSWD);
    SetHelp(data->GUI.CY_TZONE,          MSG_HELP_CO_CY_TZONE);
    SetHelp(data->GUI.CH_DSTACTIVE,      MSG_HELP_CO_CH_DSTACTIVE);

    DoMethod(data->GUI.ST_POPHOST0, MUIM_Notify, MUIA_String_Contents, MUIV_EveryTime, MUIV_Notify_Application, 3, MUIM_CallHook, &CO_GetDefaultPOPHook, 0);
    DoMethod(data->GUI.ST_PASSWD0,  MUIM_Notify, MUIA_String_Contents, MUIV_EveryTime, MUIV_Notify_Application, 3, MUIM_CallHook, &CO_GetDefaultPOPHook, 0);
  }

  RETURN(obj);
  return obj;
}

///
/// CO_PageTCPIP
Object *CO_PageTCPIP(struct CO_ClassData *data)
{
  Object *obj;
  static const char *secureSMTPMethods[4];
  static const char *securePOP3Methods[4];
  static const char *authMethods[6];
  static const char *preselectionModes[5];

  ENTER();

  secureSMTPMethods[0] = tr(MSG_CO_SMTPSECURE_NO);
  secureSMTPMethods[1] = tr(MSG_CO_SMTPSECURE_TLS);
  secureSMTPMethods[2] = tr(MSG_CO_SMTPSECURE_SSL);
  secureSMTPMethods[3] = NULL;

  securePOP3Methods[0] = tr(MSG_CO_POP3SECURE_NO);
  securePOP3Methods[1] = tr(MSG_CO_POP3SECURE_TLS);
  securePOP3Methods[2] = tr(MSG_CO_POP3SECURE_SSL);
  securePOP3Methods[3] = NULL;

  authMethods[0] = tr(MSG_CO_SMTPAUTH_AUTO);
  authMethods[1] = tr(MSG_CO_SMTPAUTH_DIGEST);
  authMethods[2] = tr(MSG_CO_SMTPAUTH_CRAM);
  authMethods[3] = tr(MSG_CO_SMTPAUTH_LOGIN);
  authMethods[4] = tr(MSG_CO_SMTPAUTH_PLAIN);
  authMethods[5] = NULL;

  preselectionModes[PSM_NEVER]       = tr(MSG_CO_PSNever);
  preselectionModes[PSM_LARGE]       = tr(MSG_CO_PSLarge);
  preselectionModes[PSM_ALWAYS]      = tr(MSG_CO_PSAlways);
  preselectionModes[PSM_ALWAYSLARGE] = tr(MSG_CO_PSAlwaysFast);
  preselectionModes[4] = NULL;

  obj = VGroup,
          MUIA_HelpNode, "CO01",

          ConfigPageHeaderObject("config_network_big", G->theme.configImages[CI_NETWORKBIG], tr(MSG_CO_TCPIP_TITLE), tr(MSG_CO_TCPIP_SUMMARY)),

          Child, ScrollgroupObject,
            MUIA_Scrollgroup_FreeHoriz, FALSE,
            MUIA_Scrollgroup_AutoBars, TRUE,
            MUIA_Scrollgroup_Contents, VGroupV,

              Child, VGroup, GroupFrameT(tr(MSG_CO_SendMail)),
                MUIA_Weight, 0,
                Child, HGroup,
                  Child, VGroup,
                    MUIA_HorizWeight, 80,
                    Child, ColGroup(2),
                      Child, Label2(tr(MSG_CO_SMTPSERVERPORT)),
                      Child, HGroup,
                        MUIA_Group_Spacing, 1,
                        Child, data->GUI.ST_SMTPHOST = MakeString(SIZE_HOST, tr(MSG_CO_SMTPSERVERPORT)),
                        Child, data->GUI.ST_SMTPPORT = BetterStringObject,
                          StringFrame,
                          MUIA_CycleChain,          TRUE,
                          MUIA_FixWidthTxt,         "00000",
                          MUIA_String_MaxLen,       5+1,
                          MUIA_String_AdvanceOnCR,  TRUE,
                          MUIA_String_Integer,      0,
                          MUIA_String_Accept,       "0123456789",
                        End,
                      End,

                      Child, Label2(tr(MSG_CO_Domain)),
                      Child, data->GUI.ST_DOMAIN = MakeString(SIZE_HOST,tr(MSG_CO_Domain)),

                      Child, HSpace(1),
                      Child, LLabel(tr(MSG_CO_SMTPSECURE)),

                      Child, HSpace(1),
                      Child, HGroup,
                        Child, HSpace(5),
                        Child, data->GUI.RA_SMTPSECURE = RadioObject,
                          MUIA_Radio_Entries, secureSMTPMethods,
                          MUIA_CycleChain,    TRUE,
                        End,
                        Child, HSpace(0),
                      End,
                    End,

                    Child, HVSpace,
                  End,

                  Child, VSpace(0),

                  Child, VGroup,
                    MUIA_HorizWeight, 20,
                    Child, ColGroup(2),
                      Child, data->GUI.CH_SMTP8BIT = MakeCheck(tr(MSG_CO_Allow8bit)),
                      Child, LLabel1(tr(MSG_CO_Allow8bit)),

                      Child, data->GUI.CH_USESMTPAUTH = MakeCheck(tr(MSG_CO_UseSMTPAUTH)),
                      Child, LLabel1(tr(MSG_CO_UseSMTPAUTH)),

                      Child, HSpace(0),
                      Child, ColGroup(2),
                        Child, Label2(tr(MSG_CO_SMTPUser)),
                        Child, data->GUI.ST_SMTPAUTHUSER = MakeString(SIZE_USERID,tr(MSG_CO_SMTPUser)),

                        Child, Label2(tr(MSG_CO_SMTPPass)),
                        Child, data->GUI.ST_SMTPAUTHPASS = MakePassString(tr(MSG_CO_SMTPPass)),

                        Child, Label2(tr(MSG_CO_SMTPAUTH_METHOD)),
                        Child, data->GUI.CY_SMTPAUTHMETHOD = MakeCycle(authMethods, tr(MSG_CO_SMTPAUTH_METHOD)),
                      End,
                    End,

                    Child, VSpace(0),
                  End,
                End,
              End,

              Child, HGroup, GroupFrameT(tr(MSG_CO_ReceiveMail)),
                Child, VGroup,
                  MUIA_HorizWeight, 30,
                  Child, NListviewObject,
                    MUIA_CycleChain, TRUE,
                    MUIA_Weight,     60,
                    MUIA_NListview_NList, data->GUI.LV_POP3 = AccountListObject,
                    End,
                  End,

                  Child, ColGroup(3),
                    Child, data->GUI.BT_PADD = MakeButton(tr(MSG_Add)),
                    Child, data->GUI.BT_PDEL = MakeButton(tr(MSG_Del)),
                    Child, ColGroup(2),
                      MUIA_Group_Spacing, 1,
                      Child, data->GUI.BT_POPUP = PopButton(MUII_ArrowUp),
                      Child, data->GUI.BT_POPDOWN = PopButton(MUII_ArrowDown),
                    End,
                  End,
                End,

                Child, NBalanceObject, End,

                Child, VGroup,
                  MUIA_HorizWeight, 70,
                  Child, VGroup,
                    Child, ColGroup(2),
                      Child, Label2(tr(MSG_CO_POPACCOUNT)),
                      Child, data->GUI.ST_POPACCOUNT = MakeString(SIZE_HOST,tr(MSG_CO_POPACCOUNT)),

                      Child, Label2(tr(MSG_CO_POP3SERVERPORT)),
                      Child, HGroup,
                        MUIA_Group_Spacing, 1,
                        Child, data->GUI.ST_POPHOST = MakeString(SIZE_HOST, tr(MSG_CO_POP3SERVERPORT)),
                        Child, data->GUI.ST_POPPORT = BetterStringObject,
                          StringFrame,
                          MUIA_CycleChain,          TRUE,
                          MUIA_FixWidthTxt,         "00000",
                          MUIA_String_MaxLen,       5+1,
                          MUIA_String_AdvanceOnCR,  TRUE,
                          MUIA_String_Integer,      0,
                          MUIA_String_Accept,       "0123456789",
                        End,
                      End,

                      Child, Label2(tr(MSG_CO_POPUserID)),
                      Child, data->GUI.ST_POPUSERID = MakeString(SIZE_USERID,tr(MSG_CO_POPUserID)),

                      Child, Label2(tr(MSG_CO_Password)),
                      Child, data->GUI.ST_PASSWD = MakePassString(tr(MSG_CO_Password)),

                      Child, HSpace(1),
                      Child, MakeCheckGroup((Object **)&data->GUI.CH_POPENABLED, tr(MSG_CO_POPActive)),

                      Child, HSpace(1),
                      Child, MakeCheckGroup((Object **)&data->GUI.CH_USEAPOP, tr(MSG_CO_UseAPOP)),

                      Child, HSpace(1),
                      Child, MakeCheckGroup((Object **)&data->GUI.CH_DOWNLOADONSTARTUP, tr(MSG_CO_DOWNLOAD_ON_STARTUP)),

                      Child, HSpace(1),
                      Child, MakeCheckGroup((Object **)&data->GUI.CH_APPLYREMOTEFILTERS, tr(MSG_CO_APPLY_REMOTE_FILTERS)),

                      Child, HSpace(1),
                      Child, MakeCheckGroup((Object **)&data->GUI.CH_DELETE, tr(MSG_CO_DeleteServerMail)),

                      Child, Label2(tr(MSG_CO_PreSelect)),
                      Child, data->GUI.CY_PRESELECTION = MakeCycle(preselectionModes, tr(MSG_CO_PreSelect)),

                      Child, HSpace(1),
                      Child, HGroup,
                        Child, LLabel(tr(MSG_CO_POP3SECURE)),
                        Child, HSpace(0),
                      End,

                      Child, HSpace(1),
                      Child, HGroup,
                        Child, HSpace(5),
                        Child, data->GUI.RA_POP3SECURE = RadioObject,
                          MUIA_Radio_Entries, securePOP3Methods,
                          MUIA_CycleChain,    TRUE,
                        End,
                        Child, HSpace(0),
                      End,

                    End,
                  End,
                End,

              End,

              Child, HVSpace,

            End,
          End,
        End;

  if(obj != NULL)
  {
    SetHelp(data->GUI.ST_SMTPHOST,           MSG_HELP_CO_ST_SMTPHOST             );
    SetHelp(data->GUI.ST_SMTPPORT,           MSG_HELP_CO_ST_SMTPPORT             );
    SetHelp(data->GUI.ST_DOMAIN,             MSG_HELP_CO_ST_DOMAIN               );
    SetHelp(data->GUI.CH_SMTP8BIT,           MSG_HELP_CO_CH_SMTP8BIT             );
    SetHelp(data->GUI.CH_USESMTPAUTH,        MSG_HELP_CO_CH_USESMTPAUTH          );
    SetHelp(data->GUI.ST_SMTPAUTHUSER,       MSG_HELP_CO_ST_SMTPAUTHUSER         );
    SetHelp(data->GUI.ST_SMTPAUTHPASS,       MSG_HELP_CO_ST_SMTPAUTHPASS         );
    SetHelp(data->GUI.CY_SMTPAUTHMETHOD,     MSG_HELP_CO_CY_SMTPAUTHMETHOD       );
    SetHelp(data->GUI.LV_POP3,               MSG_HELP_CO_LV_POP3                 );
    SetHelp(data->GUI.BT_PADD,               MSG_HELP_CO_BT_PADD                 );
    SetHelp(data->GUI.BT_PDEL,               MSG_HELP_CO_BT_PDEL                 );
    SetHelp(data->GUI.ST_POPACCOUNT,         MSG_HELP_CO_ST_POPACCOUNT           );
    SetHelp(data->GUI.ST_POPHOST,            MSG_HELP_CO_ST_POPHOST              );
    SetHelp(data->GUI.ST_POPPORT,            MSG_HELP_CO_ST_POPPORT              );
    SetHelp(data->GUI.ST_POPUSERID,          MSG_HELP_CO_ST_POPUSERID            );
    SetHelp(data->GUI.ST_PASSWD,             MSG_HELP_CO_ST_PASSWD               );
    SetHelp(data->GUI.CH_DELETE,             MSG_HELP_CO_CH_DELETE               );
    SetHelp(data->GUI.CH_USEAPOP,            MSG_HELP_CO_CH_USEAPOP              );
    SetHelp(data->GUI.CH_POPENABLED,         MSG_HELP_CO_CH_POPENABLED           );
    SetHelp(data->GUI.CH_DOWNLOADONSTARTUP,  MSG_HELP_CO_CH_DOWNLOAD_ON_STARTUP  );
    SetHelp(data->GUI.CH_APPLYREMOTEFILTERS, MSG_HELP_CO_CH_APPLY_REMOTE_FILTERS );
    SetHelp(data->GUI.RA_SMTPSECURE,         MSG_HELP_CO_RA_SMTPSECURE           );
    SetHelp(data->GUI.RA_POP3SECURE,         MSG_HELP_CO_RA_POP3SECURE           );
    SetHelp(data->GUI.CY_PRESELECTION,       MSG_HELP_CO_CY_MSGSELECT            );

    DoMethod(data->GUI.LV_POP3              , MUIM_Notify, MUIA_NList_Active    , MUIV_EveryTime, MUIV_Notify_Application, 3, MUIM_CallHook, &CO_GetPOP3EntryHook, 0);
    DoMethod(data->GUI.ST_POPACCOUNT        , MUIM_Notify, MUIA_String_Contents , MUIV_EveryTime, MUIV_Notify_Application, 3, MUIM_CallHook, &CO_PutPOP3EntryHook, 0);
    DoMethod(data->GUI.ST_POPHOST           , MUIM_Notify, MUIA_String_Contents , MUIV_EveryTime, MUIV_Notify_Application, 3, MUIM_CallHook, &CO_PutPOP3EntryHook, 0);
    DoMethod(data->GUI.ST_POPPORT           , MUIM_Notify, MUIA_String_Contents , MUIV_EveryTime, MUIV_Notify_Application, 3, MUIM_CallHook, &CO_PutPOP3EntryHook, 0);
    DoMethod(data->GUI.ST_POPUSERID         , MUIM_Notify, MUIA_String_Contents , MUIV_EveryTime, MUIV_Notify_Application, 3, MUIM_CallHook, &CO_PutPOP3EntryHook, 0);
    DoMethod(data->GUI.ST_PASSWD            , MUIM_Notify, MUIA_String_Contents , MUIV_EveryTime, MUIV_Notify_Application, 3, MUIM_CallHook, &CO_PutPOP3EntryHook, 0);
    DoMethod(data->GUI.CH_POPENABLED        , MUIM_Notify, MUIA_Selected        , MUIV_EveryTime, MUIV_Notify_Application, 3, MUIM_CallHook, &CO_PutPOP3EntryHook, 0);
    DoMethod(data->GUI.CH_USEAPOP           , MUIM_Notify, MUIA_Selected        , MUIV_EveryTime, MUIV_Notify_Application, 3, MUIM_CallHook, &CO_PutPOP3EntryHook, 0);
    DoMethod(data->GUI.CH_DELETE            , MUIM_Notify, MUIA_Selected        , MUIV_EveryTime, MUIV_Notify_Application, 3, MUIM_CallHook, &CO_PutPOP3EntryHook, 0);
    DoMethod(data->GUI.CH_DOWNLOADONSTARTUP , MUIM_Notify, MUIA_Selected        , MUIV_EveryTime, MUIV_Notify_Application, 3, MUIM_CallHook, &CO_PutPOP3EntryHook, 0);
    DoMethod(data->GUI.CH_APPLYREMOTEFILTERS, MUIM_Notify, MUIA_Selected        , MUIV_EveryTime, MUIV_Notify_Application, 3, MUIM_CallHook, &CO_PutPOP3EntryHook, 0);
    DoMethod(data->GUI.CY_PRESELECTION      , MUIM_Notify, MUIA_Cycle_Active    , MUIV_EveryTime, MUIV_Notify_Application, 3, MUIM_CallHook, &CO_PutPOP3EntryHook, 0);
    DoMethod(data->GUI.BT_PADD              , MUIM_Notify, MUIA_Pressed         , FALSE         , MUIV_Notify_Application, 2, MUIM_CallHook, &CO_AddPOP3Hook);
    DoMethod(data->GUI.BT_PDEL              , MUIM_Notify, MUIA_Pressed         , FALSE         , MUIV_Notify_Application, 2, MUIM_CallHook, &CO_DelPOP3Hook);
    DoMethod(data->GUI.BT_POPUP             , MUIM_Notify, MUIA_Pressed         , FALSE, data->GUI.LV_POP3, 3, MUIM_NList_Move, MUIV_NList_Move_Selected, MUIV_NList_Move_Previous);
    DoMethod(data->GUI.BT_POPDOWN           , MUIM_Notify, MUIA_Pressed         , FALSE, data->GUI.LV_POP3, 3, MUIM_NList_Move, MUIV_NList_Move_Selected, MUIV_NList_Move_Next);
    DoMethod(data->GUI.CH_USESMTPAUTH       , MUIM_Notify, MUIA_Selected        , MUIV_EveryTime, MUIV_Notify_Application , 7, MUIM_MultiSet, MUIA_Disabled, MUIV_NotTriggerValue, data->GUI.ST_SMTPAUTHUSER, data->GUI.ST_SMTPAUTHPASS, data->GUI.CY_SMTPAUTHMETHOD, NULL);

    // modify the POP3 port according to the security level selected.
    DoMethod(data->GUI.RA_POP3SECURE, MUIM_Notify, MUIA_Radio_Active, MUIV_EveryTime, MUIV_Notify_Application, 3, MUIM_CallHook, &CO_PutPOP3EntryHook, 0);

    // modify the SMTP port according to the security level selected.
    DoMethod(data->GUI.RA_SMTPSECURE, MUIM_Notify, MUIA_Radio_Active, 0, data->GUI.ST_SMTPPORT,   3, MUIM_Set, MUIA_String_Integer, 25);
    DoMethod(data->GUI.RA_SMTPSECURE, MUIM_Notify, MUIA_Radio_Active, 1, data->GUI.ST_SMTPPORT,   3, MUIM_Set, MUIA_String_Integer, 25);
    DoMethod(data->GUI.RA_SMTPSECURE, MUIM_Notify, MUIA_Radio_Active, 2, data->GUI.ST_SMTPPORT,   3, MUIM_Set, MUIA_String_Integer, 465);

    // disable some gadgets per default
    set(data->GUI.ST_SMTPAUTHUSER,   MUIA_Disabled, TRUE);
    set(data->GUI.ST_SMTPAUTHPASS,   MUIA_Disabled, TRUE);
    set(data->GUI.CY_SMTPAUTHMETHOD, MUIA_Disabled, TRUE);

    // set some additional cyclechain data
    set(data->GUI.BT_POPUP,   MUIA_CycleChain, TRUE);
    set(data->GUI.BT_POPDOWN, MUIA_CycleChain, TRUE);
  }

  RETURN(obj);
  return obj;
}

///
/// CO_PageNewMail
Object *CO_PageNewMail(struct CO_ClassData *data)
{
  static const char *trwopt[4];
  Object *pa_notisound;
  Object *bt_notisound;
  Object *pa_noticmd;
  Object *obj;

  ENTER();

  trwopt[TWM_HIDE] = tr(MSG_CO_TWNever);
  trwopt[TWM_AUTO] = tr(MSG_CO_TWAuto);
  trwopt[TWM_SHOW] = tr(MSG_CO_TWAlways);
  trwopt[3] = NULL;

  obj = VGroup,
          MUIA_HelpNode, "CO02",

          ConfigPageHeaderObject("config_newmail_big", G->theme.configImages[CI_NEWMAILBIG], tr(MSG_CO_NEWMAIL_TITLE), tr(MSG_CO_NEWMAIL_SUMMARY)),

          Child, ScrollgroupObject,
            MUIA_Scrollgroup_FreeHoriz, FALSE,
            MUIA_Scrollgroup_AutoBars, TRUE,
            MUIA_Scrollgroup_Contents, VGroupV,

              Child, HGroup, GroupFrameT(tr(MSG_CO_Download)),
                Child, ColGroup(2),
                  Child, Label(tr(MSG_CO_TransferWin)),
                  Child, data->GUI.CY_TRANSWIN = MakeCycle(trwopt, tr(MSG_CO_TransferWin)),

                  Child, Label(tr(MSG_CO_WarnSize1)),
                  Child, HGroup,
                    Child, data->GUI.ST_WARNSIZE = MakeInteger(5, tr(MSG_CO_WarnSize1)),
                    Child, LLabel(tr(MSG_CO_WarnSize2)),
                  End,
                End,

                Child, VSpace(0),

                Child, VGroup,
                  Child, MakeCheckGroup((Object **)&data->GUI.CH_UPDSTAT, tr(MSG_CO_UpdateStatus)),
                  Child, HVSpace,
                End,
              End,

              Child, VGroup, GroupFrameT(tr(MSG_CO_AutoOperation)),
                Child, HGroup,
                  Child, Label(tr(MSG_CO_CheckMail)),
                  Child, data->GUI.NM_INTERVAL = NumericbuttonObject,
                    MUIA_CycleChain,      TRUE,
                    MUIA_Numeric_Min,     0,
                    MUIA_Numeric_Max,     240,
                    MUIA_Numeric_Default, 5,
                  End,
                  Child, Label(tr(MSG_CO_Minutes)),
                  Child, HSpace(0),
                End,
                Child, MakeCheckGroup((Object **)&data->GUI.CH_DLLARGE, tr(MSG_CO_DownloadLarge)),
              End,

              Child, VGroup, GroupFrameT(tr(MSG_CO_Notification)),
                Child, ColGroup(3),
                  Child, data->GUI.CH_NOTIREQ =  MakeCheck(tr(MSG_CO_NotiReq)),
                  Child, LLabel(tr(MSG_CO_NotiReq)),
                  Child, HSpace(0),

                  Child, data->GUI.CH_NOTIOS41SYSTEM = MakeCheck(tr(MSG_CO_NOTIOS41SYSTEM)),
                  Child, LLabel(tr(MSG_CO_NOTIOS41SYSTEM)),
                  Child, HSpace(0),
                End,

                Child, ColGroup(3),
                  Child, data->GUI.CH_NOTISOUND = MakeCheck(tr(MSG_CO_NotiSound)),
                  Child, LLabel(tr(MSG_CO_NotiSound)),
                  Child, HGroup,
                    MUIA_Group_HorizSpacing, 0,
                    Child, pa_notisound = PopaslObject,
                      MUIA_Popasl_Type,      ASL_FileRequest,
                      MUIA_Popstring_String, data->GUI.ST_NOTISOUND = MakeString(SIZE_PATHFILE,""),
                      MUIA_Popstring_Button, PopButton(MUII_PopFile),
                    End,
                    Child, bt_notisound = PopButton(MUII_TapePlay),
                  End,

                  Child, data->GUI.CH_NOTICMD = MakeCheck(tr(MSG_CO_NotiCommand)),
                  Child, LLabel(tr(MSG_CO_NotiCommand)),
                  Child, pa_noticmd = PopaslObject,
                    MUIA_Popasl_Type,      ASL_FileRequest,
                    MUIA_Popstring_String,data->GUI.ST_NOTICMD = MakeString(SIZE_COMMAND,""),
                    MUIA_Popstring_Button,PopButton(MUII_PopFile),
                  End,
                End,
              End,

              Child, HVSpace,

            End,
          End,

        End;

  if(obj != NULL)
  {
    SetHelp(data->GUI.CY_TRANSWIN,       MSG_HELP_CO_CH_TRANSWIN);
    SetHelp(data->GUI.CH_UPDSTAT,        MSG_HELP_CO_CH_UPDSTAT);
    SetHelp(data->GUI.ST_WARNSIZE,       MSG_HELP_CO_ST_WARNSIZE);
    SetHelp(data->GUI.NM_INTERVAL,       MSG_HELP_CO_ST_INTERVAL);
    SetHelp(data->GUI.CH_DLLARGE,        MSG_HELP_CO_CH_DLLARGE);
    SetHelp(data->GUI.CH_NOTIREQ,        MSG_HELP_CO_CH_NOTIREQ);
    SetHelp(data->GUI.CH_NOTISOUND,      MSG_HELP_CO_CH_NOTISOUND);
    SetHelp(data->GUI.CH_NOTICMD,        MSG_HELP_CO_CH_NOTICMD);
    SetHelp(data->GUI.ST_NOTICMD,        MSG_HELP_CO_ST_NOTICMD);
    SetHelp(data->GUI.ST_NOTISOUND,      MSG_HELP_CO_ST_NOTISOUND);
    SetHelp(data->GUI.CH_NOTIOS41SYSTEM, MSG_HELP_CO_CH_NOTIOS41SYSTEM);

    DoMethod(G->App, MUIM_MultiSet, MUIA_Disabled, TRUE, pa_notisound, bt_notisound, pa_noticmd, NULL);

    #if defined(__amigaos4__)
    set(data->GUI.CH_NOTIOS41SYSTEM, MUIA_Disabled, G->applicationID == 0 || LIB_VERSION_IS_AT_LEAST(ApplicationBase, 53, 2) == FALSE);
    #else // __amigaos4__
    set(data->GUI.CH_NOTIOS41SYSTEM, MUIA_Disabled, TRUE);
    #endif // __amigaos4__

    set(bt_notisound,MUIA_CycleChain,1);
    DoMethod(bt_notisound          ,MUIM_Notify,MUIA_Pressed ,FALSE         ,MUIV_Notify_Application,3,MUIM_CallHook,&CO_PlaySoundHook,data->GUI.ST_NOTISOUND);
    DoMethod(data->GUI.CH_NOTISOUND,MUIM_Notify,MUIA_Selected,MUIV_EveryTime,pa_notisound           ,3,MUIM_Set,MUIA_Disabled,MUIV_NotTriggerValue);
    DoMethod(data->GUI.CH_NOTISOUND,MUIM_Notify,MUIA_Selected,MUIV_EveryTime,bt_notisound           ,3,MUIM_Set,MUIA_Disabled,MUIV_NotTriggerValue);
    DoMethod(data->GUI.CH_NOTICMD  ,MUIM_Notify,MUIA_Selected,MUIV_EveryTime,pa_noticmd             ,3,MUIM_Set,MUIA_Disabled,MUIV_NotTriggerValue);
  }

  RETURN(obj);
  return obj;
}

///
/// CO_PageFilters
Object *CO_PageFilters(struct CO_ClassData *data)
{
   static const char *rtitles[4];
   Object *grp;
   Object *bt_moveto;

   rtitles[0] = tr(MSG_Options);
   rtitles[1] = tr(MSG_CO_Comparison);
   rtitles[2] = tr(MSG_CO_Action);
   rtitles[3] = NULL;

   if((grp = VGroup,
         MUIA_HelpNode, "CO03",

         ConfigPageHeaderObject("config_filters_big", G->theme.configImages[CI_FILTERSBIG], tr(MSG_CO_FILTER_TITLE), tr(MSG_CO_FILTER_SUMMARY)),

          Child, ScrollgroupObject,
            MUIA_Scrollgroup_FreeHoriz, FALSE,
            MUIA_Scrollgroup_AutoBars, TRUE,
            MUIA_Scrollgroup_Contents, VGroupV,

              Child, HGroup,
                   Child, VGroup,
                      MUIA_Weight, 70,
                      Child, NListviewObject,
                         MUIA_CycleChain, TRUE,
                         MUIA_NListview_NList, data->GUI.LV_RULES = FilterListObject,
                         End,
                      End,
                      Child, ColGroup(3),
                         Child, data->GUI.BT_RADD = MakeButton(tr(MSG_Add)),
                         Child, data->GUI.BT_RDEL = MakeButton(tr(MSG_Del)),
                         Child, ColGroup(2),
                           MUIA_Group_Spacing, 1,
                           Child, data->GUI.BT_FILTERUP = PopButton(MUII_ArrowUp),
                           Child, data->GUI.BT_FILTERDOWN = PopButton(MUII_ArrowDown),
                         End,
                      End,
                   End,
                   Child, NBalanceObject, End,
                   Child, RegisterGroup(rtitles),
                      MUIA_CycleChain, TRUE,
                      Child, ColGroup(2),
                         Child, Label2(tr(MSG_CO_Name)),
                         Child, data->GUI.ST_RNAME = MakeString(SIZE_NAME,tr(MSG_CO_Name)),

                         Child, HSpace(1),
                         Child, MakeCheckGroup((Object **)&data->GUI.CH_REMOTE, tr(MSG_CO_Remote)),

                         Child, HSpace(1),
                         Child, MakeCheckGroup((Object **)&data->GUI.CH_APPLYNEW, tr(MSG_CO_ApplyToNew)),

                         Child, HSpace(1),
                         Child, MakeCheckGroup((Object **)&data->GUI.CH_APPLYSENT, tr(MSG_CO_ApplyToSent)),

                         Child, HSpace(1),
                         Child, MakeCheckGroup((Object **)&data->GUI.CH_APPLYREQ, tr(MSG_CO_ApplyOnReq)),

                         Child, HVSpace,
                         Child, HVSpace,
                      End,
                      Child, data->GUI.GR_RGROUP = VGroup,
                         Child, ScrollgroupObject,
                            GroupSpacing(3),
                            MUIA_Weight, 100,
                            MUIA_Scrollgroup_FreeHoriz, FALSE,
                            MUIA_Scrollgroup_FreeVert,  TRUE,
                            MUIA_Scrollgroup_Contents,  data->GUI.GR_SGROUP = VirtgroupObject,
                              Child, SearchControlGroupObject, // we need a minimum of one dummy control group
                              End,
                            End,
                         End,
                         Child, RectangleObject,
                            MUIA_Weight, 1,
                         End,
                          Child, RectangleObject,
                            MUIA_Rectangle_HBar, TRUE,
                            MUIA_FixHeight,      4,
                          End,
                         Child, HGroup,
                            Child, data->GUI.BT_MORE = MakeButton(tr(MSG_CO_More)),
                            Child, HVSpace,
                            Child, data->GUI.BT_LESS = MakeButton(tr(MSG_CO_Less)),
                         End,
                      End,
                      Child, VGroup,
                         Child, ColGroup(3),
                            Child, data->GUI.CH_ABOUNCE = MakeCheck(tr(MSG_CO_ActionBounce)),
                            Child, LLabel2(tr(MSG_CO_ActionBounce)),
                            Child, MakeAddressField(&data->GUI.ST_ABOUNCE, "", MSG_HELP_CO_ST_ABOUNCE, ABM_CONFIG, -1, AFF_ALLOW_MULTI),
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
                                End,
                                MUIA_Popstring_Button,bt_moveto = PopButton(MUII_PopUp),
                                MUIA_Popobject_StrObjHook, &PO_InitFolderListHook,
                                MUIA_Popobject_ObjStrHook, &PO_List2TextHook,
                                MUIA_Popobject_WindowHook, &PO_WindowHook,
                                MUIA_Popobject_Object, data->GUI.LV_MOVETO = ListviewObject,
                                   MUIA_Listview_List, ListObject,
                                     InputListFrame,
                                     MUIA_List_AutoVisible,   TRUE,
                                     MUIA_List_ConstructHook, MUIV_List_ConstructHook_String,
                                     MUIA_List_DestructHook,  MUIV_List_DestructHook_String,
                                   End,
                               End,
                             End,

                         End,
                         Child, MakeCheckGroup((Object **)&data->GUI.CH_ASTATUSTOMARKED, tr(MSG_CO_ACTION_SET_STATUS_TO_MARKED)),
                         Child, MakeCheckGroup((Object **)&data->GUI.CH_ASTATUSTOUNMARKED, tr(MSG_CO_ACTION_SET_STATUS_TO_UNMARKED)),
                         Child, MakeCheckGroup((Object **)&data->GUI.CH_ASTATUSTOREAD, tr(MSG_CO_ACTION_SET_STATUS_TO_READ)),
                         Child, MakeCheckGroup((Object **)&data->GUI.CH_ASTATUSTOUNREAD, tr(MSG_CO_ACTION_SET_STATUS_TO_UNREAD)),
                         Child, MakeCheckGroup((Object **)&data->GUI.CH_ASTATUSTOSPAM, tr(MSG_CO_ACTION_SET_STATUS_TO_SPAM)),
                         Child, MakeCheckGroup((Object **)&data->GUI.CH_ASTATUSTOHAM, tr(MSG_CO_ACTION_SET_STATUS_TO_HAM)),
                         Child, MakeCheckGroup((Object **)&data->GUI.CH_ADELETE, tr(MSG_CO_ActionDelete)),
                         Child, MakeCheckGroup((Object **)&data->GUI.CH_ASKIP, tr(MSG_CO_ActionSkip)),
                         Child, HVSpace,

                      End,
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
      SetHelp(data->GUI.CH_ABOUNCE,           MSG_HELP_CO_CH_ABOUNCE);
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
      SetHelp(data->GUI.BT_RADD ,             MSG_HELP_CO_BT_RADD);
      SetHelp(data->GUI.BT_RDEL,              MSG_HELP_CO_BT_RDEL);
      SetHelp(data->GUI.BT_MORE,              MSG_HELP_CO_BT_MORE);
      SetHelp(data->GUI.BT_LESS,              MSG_HELP_CO_BT_LESS);
      SetHelp(data->GUI.BT_FILTERUP,          MSG_HELP_CO_BT_FILTERUP);
      SetHelp(data->GUI.BT_FILTERDOWN,        MSG_HELP_CO_BT_FILTERDOWN);

      // set the cyclechain
      set(data->GUI.BT_APLAY, MUIA_CycleChain, TRUE);
      set(bt_moveto,MUIA_CycleChain, TRUE);
      set(data->GUI.BT_FILTERUP, MUIA_CycleChain, TRUE);
      set(data->GUI.BT_FILTERDOWN, MUIA_CycleChain, TRUE);

      GhostOutFilter(&(data->GUI), NULL);

      DoMethod(data->GUI.LV_RULES             ,MUIM_Notify, MUIA_NList_Active         ,MUIV_EveryTime ,MUIV_Notify_Application        ,2 ,MUIM_CallHook          ,&GetActiveFilterDataHook);
      DoMethod(data->GUI.ST_RNAME             ,MUIM_Notify, MUIA_String_Contents      ,MUIV_EveryTime ,MUIV_Notify_Application        ,2 ,MUIM_CallHook          ,&SetActiveFilterDataHook);
      DoMethod(data->GUI.CH_REMOTE            ,MUIM_Notify, MUIA_Selected             ,MUIV_EveryTime ,MUIV_Notify_Application        ,3 ,MUIM_CallHook          ,&CO_RemoteToggleHook       ,MUIV_TriggerValue);
      DoMethod(data->GUI.CH_APPLYREQ          ,MUIM_Notify, MUIA_Selected             ,MUIV_EveryTime ,MUIV_Notify_Application        ,2 ,MUIM_CallHook          ,&SetActiveFilterDataHook);
      DoMethod(data->GUI.CH_APPLYSENT         ,MUIM_Notify, MUIA_Selected             ,MUIV_EveryTime ,MUIV_Notify_Application        ,2 ,MUIM_CallHook          ,&SetActiveFilterDataHook);
      DoMethod(data->GUI.CH_APPLYNEW          ,MUIM_Notify, MUIA_Selected             ,MUIV_EveryTime ,MUIV_Notify_Application        ,2 ,MUIM_CallHook          ,&SetActiveFilterDataHook);
      DoMethod(data->GUI.CH_ABOUNCE           ,MUIM_Notify, MUIA_Selected             ,MUIV_EveryTime ,MUIV_Notify_Application        ,2 ,MUIM_CallHook          ,&SetActiveFilterDataHook);
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
      DoMethod(data->GUI.ST_ABOUNCE           ,MUIM_Notify, MUIA_String_BufferPos     ,MUIV_EveryTime ,MUIV_Notify_Application        ,2 ,MUIM_CallHook          ,&SetActiveFilterDataHook);
      DoMethod(data->GUI.ST_AFORWARD          ,MUIM_Notify, MUIA_String_BufferPos     ,MUIV_EveryTime ,MUIV_Notify_Application        ,2 ,MUIM_CallHook          ,&SetActiveFilterDataHook);
      DoMethod(data->GUI.ST_ARESPONSE         ,MUIM_Notify, MUIA_String_Contents      ,MUIV_EveryTime ,MUIV_Notify_Application        ,2 ,MUIM_CallHook          ,&SetActiveFilterDataHook);
      DoMethod(data->GUI.ST_AEXECUTE          ,MUIM_Notify, MUIA_String_Contents      ,MUIV_EveryTime ,MUIV_Notify_Application        ,2 ,MUIM_CallHook          ,&SetActiveFilterDataHook);
      DoMethod(data->GUI.ST_APLAY             ,MUIM_Notify, MUIA_String_Contents      ,MUIV_EveryTime ,MUIV_Notify_Application        ,2 ,MUIM_CallHook          ,&SetActiveFilterDataHook);
      DoMethod(data->GUI.BT_APLAY             ,MUIM_Notify, MUIA_Pressed              ,FALSE          ,MUIV_Notify_Application        ,3 ,MUIM_CallHook          ,&CO_PlaySoundHook,data->GUI.ST_APLAY);
      DoMethod(data->GUI.TX_MOVETO            ,MUIM_Notify, MUIA_Text_Contents        ,MUIV_EveryTime ,MUIV_Notify_Application        ,2 ,MUIM_CallHook          ,&SetActiveFilterDataHook);
      DoMethod(data->GUI.LV_MOVETO            ,MUIM_Notify, MUIA_Listview_DoubleClick ,TRUE           ,data->GUI.PO_MOVETO            ,2 ,MUIM_Popstring_Close   ,TRUE);
      DoMethod(data->GUI.LV_MOVETO            ,MUIM_Notify, MUIA_Listview_DoubleClick ,TRUE           ,data->GUI.CH_AMOVE             ,3 ,MUIM_Set,MUIA_Selected ,TRUE);
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
      DoMethod(data->GUI.BT_MORE              ,MUIM_Notify, MUIA_Pressed              ,FALSE          ,MUIV_Notify_Application        ,2 ,MUIM_CallHook          ,&AddNewRuleToListHook);
      DoMethod(data->GUI.BT_LESS              ,MUIM_Notify, MUIA_Pressed              ,FALSE          ,MUIV_Notify_Application        ,2 ,MUIM_CallHook          ,&RemoveLastRuleHook);
   }

   return grp;
}

///
/// CO_PageSpam
//
Object *CO_PageSpam(struct CO_ClassData *data)
{
  Object *obj;

  ENTER();

  obj = VGroup,
          MUIA_HelpNode, "CO17",

          ConfigPageHeaderObject("config_spam_big", G->theme.configImages[CI_SPAMBIG], tr(MSG_CO_SPAMFILTER_TITLE), tr(MSG_CO_SPAMFILTER_SUMMARY)),

          Child, ScrollgroupObject,
            MUIA_Scrollgroup_FreeHoriz, FALSE,
            MUIA_Scrollgroup_AutoBars, TRUE,
            MUIA_Scrollgroup_Contents, VGroupV,

              Child, VGroup, GroupFrameT(tr(MSG_CO_SPAMFILTER)),
                Child, ColGroup(4),
                  Child, data->GUI.CH_SPAMFILTERENABLED = MakeCheck(tr(MSG_CO_SPAM_FILTERENABLED)),
                  Child, LLabel(tr(MSG_CO_SPAM_FILTERENABLED)),
                  Child, HSpace(0),
                  Child, HSpace(0),

                  Child, HSpace(0),
                  Child, Label2(tr(MSG_CO_SPAM_BADCOUNT)),
                  Child, data->GUI.TX_SPAMBADCOUNT = TextObject,
                    TextFrame,
                    MUIA_Background,    MUII_TextBack,
                    MUIA_Text_SetMin,   TRUE,
                    MUIA_Text_PreParse, "\033r",
                  End,
                  Child, HSpace(0),

                  Child, HSpace(0),
                  Child, Label2(tr(MSG_CO_SPAM_GOODCOUNT)),
                  Child, data->GUI.TX_SPAMGOODCOUNT = TextObject,
                    TextFrame,
                    MUIA_Background,    MUII_TextBack,
                    MUIA_Text_SetMin,   TRUE,
                    MUIA_Text_PreParse, "\033r",
                  End,
                  Child, HSpace(0),

                  Child, HSpace(0),
                  Child, data->GUI.BT_SPAMRESETTRAININGDATA = MakeButton(tr(MSG_CO_SPAM_RESETTRAININGDATA)),
                  Child, data->GUI.BT_OPTIMIZETRAININGDATA = MakeButton(tr(MSG_CO_SPAM_OPTIMIZE_TRAININGDATA)),
                  Child, HSpace(0),
                End,
              End,

              Child, VGroup, GroupFrameT(tr(MSG_CO_SPAM_RECOGNITION)),
                Child, MakeCheckGroup((Object **)&data->GUI.CH_SPAMFILTERFORNEWMAIL, tr(MSG_CO_SPAM_FILTERFORNEWMAIL)),
                Child, MakeCheckGroup((Object **)&data->GUI.CH_SPAMABOOKISWHITELIST, tr(MSG_CO_SPAM_ADDRESSBOOKISWHITELIST)),
                Child, MakeCheckGroup((Object **)&data->GUI.CH_SPAMMARKONMOVE, tr(MSG_CO_SPAM_MARKONMOVE)),
                Child, MakeCheckGroup((Object **)&data->GUI.CH_SPAMMARKASREAD, tr(MSG_CO_SPAM_MARK_AS_READ)),
                Child, MakeCheckGroup((Object **)&data->GUI.CH_MOVEHAMTOINCOMING, tr(MSG_CO_MOVE_HAM_TO_INCOMING)),
                Child, ColGroup(2),
                  Child, HSpace(5),
                  Child, MakeCheckGroup((Object **)&data->GUI.CH_FILTERHAM, tr(MSG_CO_FILTER_HAM)),
                End,
              End,

              Child, HVSpace,

            End,
          End,

        End;

  if(obj != NULL)
  {
    DoMethod(G->App, MUIM_MultiSet, MUIA_Disabled, TRUE, data->GUI.BT_SPAMRESETTRAININGDATA,
                                                         data->GUI.BT_OPTIMIZETRAININGDATA,
                                                         data->GUI.CH_SPAMFILTERFORNEWMAIL,
                                                         data->GUI.CH_SPAMABOOKISWHITELIST,
                                                         data->GUI.CH_SPAMMARKONMOVE,
                                                         data->GUI.CH_SPAMMARKASREAD,
                                                         data->GUI.CH_MOVEHAMTOINCOMING,
                                                         data->GUI.CH_FILTERHAM,
                                                         NULL);

    SetHelp(data->GUI.CH_SPAMFILTERENABLED,     MSG_HELP_CH_SPAMFILTERENABLED);
    SetHelp(data->GUI.TX_SPAMBADCOUNT,          MSG_HELP_TX_SPAMBADCOUNT);
    SetHelp(data->GUI.TX_SPAMGOODCOUNT,         MSG_HELP_TX_SPAMGOODCOUNT);
    SetHelp(data->GUI.BT_SPAMRESETTRAININGDATA, MSG_HELP_BT_SPAMRESETTRAININGDATA);
    SetHelp(data->GUI.BT_OPTIMIZETRAININGDATA,  MSG_HELP_BT_OPTIMIZE_TRAININGDATA);
    SetHelp(data->GUI.CH_SPAMFILTERFORNEWMAIL,  MSG_HELP_CH_SPAMFILTERFORNEWMAIL);
    SetHelp(data->GUI.CH_SPAMABOOKISWHITELIST,  MSG_HELP_CH_SPAMABOOKISWHITELIST);
    SetHelp(data->GUI.CH_SPAMMARKONMOVE,        MSG_HELP_CH_SPAMMARKONMOVE);
    SetHelp(data->GUI.CH_SPAMMARKASREAD,        MSG_HELP_CH_SPAMMARKASREAD);
    SetHelp(data->GUI.CH_MOVEHAMTOINCOMING,     MSG_HELP_CH_MOVE_HAM_TO_INCOMING);
    SetHelp(data->GUI.CH_FILTERHAM,             MSG_HELP_CH_FILTER_HAM);

    DoMethod(data->GUI.CH_SPAMFILTERENABLED, MUIM_Notify, MUIA_Selected, MUIV_EveryTime,
                                             MUIV_Notify_Application, 3, MUIM_CallHook, &ToggleSpamFilterHook, MUIV_TriggerValue);
    DoMethod(data->GUI.CH_MOVEHAMTOINCOMING, MUIM_Notify, MUIA_Selected, MUIV_EveryTime,
                                             data->GUI.CH_FILTERHAM, 3, MUIM_Set, MUIA_Disabled, MUIV_NotTriggerValue);
    DoMethod(data->GUI.BT_SPAMRESETTRAININGDATA, MUIM_Notify, MUIA_Pressed,  FALSE,
                                                 MUIV_Notify_Application, 2, MUIM_CallHook, &ResetSpamTrainingDataHook);
    DoMethod(data->GUI.BT_OPTIMIZETRAININGDATA, MUIM_Notify, MUIA_Pressed,  FALSE,
                                                MUIV_Notify_Application, 2, MUIM_CallHook, &OptimizeSpamTrainingDataHook);
  }

  RETURN(obj);
  return obj;
}
///
/// CO_PageRead
Object *CO_PageRead(struct CO_ClassData *data)
{
  Object *obj;
  Object *charsetPopButton;
  static const char *headopt[4];
  static const char *siopt[5];
  static const char *slopt[5];
  static const char *rropt[5];

  headopt[0] = tr(MSG_CO_HeadNone);
  headopt[1] = tr(MSG_CO_HeadShort);
  headopt[2] = tr(MSG_CO_HeadFull);
  headopt[3] = NULL;

  siopt[0] = tr(MSG_CO_SINone);
  siopt[1] = tr(MSG_CO_SIFields);
  siopt[2] = tr(MSG_CO_SIAll);
  siopt[3] = tr(MSG_CO_SImageOnly);
  siopt[4] = NULL;

  slopt[SST_BLANK]= tr(MSG_CO_SLBlank);
  slopt[SST_DASH] = tr(MSG_CO_SLDash);
  slopt[SST_BAR]  = tr(MSG_CO_SLBar);
  slopt[SST_SKIP] = tr(MSG_CO_SLSkip);
  slopt[4] = NULL;

  rropt[0] = tr(MSG_CO_MDN_ACTION_IGNORE);
  rropt[1] = tr(MSG_CO_MDN_ACTION_SEND);
  rropt[2] = tr(MSG_CO_MDN_ACTION_QUEUE);
  rropt[3] = tr(MSG_CO_MDN_ACTION_ASK);
  rropt[4] = NULL;

  ENTER();

  obj = VGroup,
          MUIA_HelpNode, "CO04",

          ConfigPageHeaderObject("config_read_big", G->theme.configImages[CI_READBIG], tr(MSG_CO_READ_TITLE), tr(MSG_CO_READ_SUMMARY)),

          Child, ScrollgroupObject,
            MUIA_Scrollgroup_FreeHoriz, FALSE,
            MUIA_Scrollgroup_AutoBars, TRUE,
            MUIA_Scrollgroup_Contents, VGroupV,

            Child, ColGroup(3), GroupFrameT(tr(MSG_CO_HeaderLayout)),
              Child, Label2(tr(MSG_CO_Header)),
              Child, data->GUI.CY_HEADER = MakeCycle(headopt,tr(MSG_CO_Header)),
              Child, data->GUI.ST_HEADERS = MakeString(SIZE_PATTERN, ""),
              Child, Label1(tr(MSG_CO_SenderInfo)),
              Child, data->GUI.CY_SENDERINFO = MakeCycle(siopt,tr(MSG_CO_SenderInfo)),
              Child, MakeCheckGroup((Object **)&data->GUI.CH_WRAPHEAD, tr(MSG_CO_WrapHeader)),
            End,

            Child, ColGroup(3), GroupFrameT(tr(MSG_CO_BodyLayout)),
              Child, Label1(tr(MSG_CO_SignatureSep)),
              Child, HGroup,
                Child, data->GUI.CY_SIGSEPLINE = MakeCycle(slopt,tr(MSG_CO_SignatureSep)),
                Child, data->GUI.CA_COLSIG = PoppenObject,
                  MUIA_CycleChain, TRUE,
                End,
              End,
              Child, RectangleObject,
                MUIA_VertWeight,          0,
                MUIA_Rectangle_HBar,      TRUE,
                MUIA_Rectangle_BarTitle,  tr(MSG_CO_FONTSETTINGS),
              End,

              Child, Label1(tr(MSG_CO_ColoredText)),
              Child, data->GUI.CA_COLTEXT = PoppenObject,
                MUIA_CycleChain, TRUE,
              End,
              Child, MakeCheckGroup((Object **)&data->GUI.CH_FIXFEDIT, tr(MSG_CO_FixedFontEdit)),

              Child, Label1(tr(MSG_CO_OldQuotes)),
              Child, HGroup,
                Child, data->GUI.CA_COL1QUOT = PoppenObject,
                  MUIA_CycleChain, TRUE,
                End,
                Child, data->GUI.CA_COL2QUOT = PoppenObject,
                  MUIA_CycleChain, TRUE,
                End,
                Child, data->GUI.CA_COL3QUOT = PoppenObject,
                  MUIA_CycleChain, TRUE,
                End,
                Child, data->GUI.CA_COL4QUOT = PoppenObject,
                  MUIA_CycleChain, TRUE,
                End,
              End,
              Child, MakeCheckGroup((Object **)&data->GUI.CH_TEXTCOLORS_READ, tr(MSG_CO_TEXTCOLORS_READ)),

              Child, Label1(tr(MSG_CO_URLCOLOR)),
              Child, data->GUI.CA_COLURL = PoppenObject,
                MUIA_CycleChain, TRUE,
              End,
              Child, MakeCheckGroup((Object **)&data->GUI.CH_TEXTSTYLES_READ, tr(MSG_CO_TEXTSTYLES_READ)),

              Child, Label2(tr(MSG_CO_DEFAULTCHARSET)),
              Child, MakeCharsetPop((Object **)&data->GUI.TX_DEFCHARSET_READ, &charsetPopButton),
              Child, HSpace(1),

            End,

            Child, VGroup, GroupFrameT(tr(MSG_CO_MDN_TITLE)),
              Child, ColGroup(2),
                Child, LLabel(tr(MSG_CO_MDN_DESCRIPTION)),
                Child, HSpace(0),

                Child, ColGroup(4),
                  Child, HSpace(1),
                  Child, data->GUI.CH_MDN_NEVER = MakeCheck(tr(MSG_CO_MDN_DISABLED)),
                  Child, LLabel(tr(MSG_CO_MDN_DISABLED)),
                  Child, HSpace(0),

                  Child, HSpace(1),
                  Child, data->GUI.CH_MDN_ALLOW = MakeCheck(tr(MSG_CO_MDN_ENABLED)),
                  Child, LLabel(tr(MSG_CO_MDN_ENABLED)),
                  Child, HSpace(0),

                  Child, HSpace(1),
                  Child, HSpace(0),
                  Child, LLabel(tr(MSG_CO_MDN_NORECIPIENT)),
                  Child, data->GUI.CY_MDN_NORECIPIENT = MakeCycle(rropt, tr(MSG_CO_MDN_NORECIPIENT)),

                  Child, HSpace(1),
                  Child, HSpace(0),
                  Child, LLabel(tr(MSG_CO_MDN_NODOMAIN)),
                  Child, data->GUI.CY_MDN_NODOMAIN = MakeCycle(rropt, tr(MSG_CO_MDN_NODOMAIN)),

                  Child, HSpace(1),
                  Child, HSpace(0),
                  Child, LLabel(tr(MSG_CO_MDN_DELETE)),
                  Child, data->GUI.CY_MDN_DELETE = MakeCycle(rropt, tr(MSG_CO_MDN_DELETE)),

                  Child, HSpace(1),
                  Child, HSpace(0),
                  Child, LLabel(tr(MSG_CO_MDN_OTHER)),
                  Child, data->GUI.CY_MDN_OTHER = MakeCycle(rropt, tr(MSG_CO_MDN_OTHER)),

                End,
                Child, HSpace(0),

              End,
            End,

            Child, VGroup, GroupFrameT(tr(MSG_CO_OtherOptions)),
              Child, MakeCheckGroup((Object **)&data->GUI.CH_MULTIWIN, tr(MSG_CO_MultiReadWin)),
              Child, MakeCheckGroup((Object **)&data->GUI.CH_GLOBALMAILTHREADS, tr(MSG_CO_GLOBALMAILTHREADS)),
              Child, HGroup,
                Child, data->GUI.CH_DELAYEDSTATUS = MakeCheck(tr(MSG_CO_SETSTATUSDELAYED1)),
                Child, Label2(tr(MSG_CO_SETSTATUSDELAYED1)),
                Child, data->GUI.NB_DELAYEDSTATUS = NumericbuttonObject,
                  MUIA_CycleChain,  TRUE,
                  MUIA_Numeric_Min, 1,
                  MUIA_Numeric_Max, 10,
                End,
                Child, Label2(tr(MSG_CO_SETSTATUSDELAYED2)),
                Child, HSpace(0),
              End,
              Child, MakeCheckGroup((Object **)&data->GUI.CH_CONVERTHTML, tr(MSG_CO_CONVERTHTML)),
              Child, MakeCheckGroup((Object **)&data->GUI.CH_MAPFOREIGNCHARS, tr(MSG_CO_MAPFOREIGNCHARS)),
              Child, MakeCheckGroup((Object **)&data->GUI.CH_DETECTCYRILLIC, tr(MSG_CO_DETECT_CYRILLIC)),
              Child, MakeCheckGroup((Object **)&data->GUI.CH_ALLTEXTS, tr(MSG_CO_DisplayAll)),
              Child, MakeCheckGroup((Object **)&data->GUI.CH_SHOWALTPARTS, tr(MSG_CO_SHOWALTPARTS)),
            End,

            Child, HVSpace,

            End,
          End,
        End;

  if(obj != NULL)
  {
    set(data->GUI.ST_HEADERS, MUIA_Disabled, TRUE);

    set(charsetPopButton, MUIA_ControlChar, ShortCut(tr(MSG_CO_DEFAULTCHARSET)));

    SetHelp(data->GUI.CY_HEADER,          MSG_HELP_CO_CY_HEADER);
    SetHelp(data->GUI.ST_HEADERS,         MSG_HELP_CO_ST_HEADERS);
    SetHelp(data->GUI.CY_SENDERINFO,      MSG_HELP_CO_CY_SENDERINFO);
    SetHelp(data->GUI.CA_COLSIG,          MSG_HELP_CO_CA_COLSIG);
    SetHelp(data->GUI.CA_COLTEXT,         MSG_HELP_CO_CA_COLTEXT);
    SetHelp(data->GUI.CA_COL1QUOT,        MSG_HELP_CO_CA_COL1QUOT);
    SetHelp(data->GUI.CA_COL2QUOT,        MSG_HELP_CO_CA_COL2QUOT);
    SetHelp(data->GUI.CA_COL3QUOT,        MSG_HELP_CO_CA_COL3QUOT);
    SetHelp(data->GUI.CA_COL4QUOT,        MSG_HELP_CO_CA_COL4QUOT);
    SetHelp(data->GUI.CA_COLURL,          MSG_HELP_CO_CA_COLURL);
    SetHelp(data->GUI.CH_ALLTEXTS,        MSG_HELP_CO_CH_ALLTEXTS);
    SetHelp(data->GUI.CH_MULTIWIN,        MSG_HELP_CO_CH_MULTIWIN);
    SetHelp(data->GUI.CY_SIGSEPLINE,      MSG_HELP_CO_CY_SIGSEPLINE);
    SetHelp(data->GUI.CH_FIXFEDIT,        MSG_HELP_CO_CH_FIXFEDIT);
    SetHelp(data->GUI.CH_WRAPHEAD,        MSG_HELP_CO_CH_WRAPHEAD);
    SetHelp(data->GUI.CH_TEXTSTYLES_READ, MSG_HELP_CO_CH_TEXTSTYLES_READ);
    SetHelp(data->GUI.CH_TEXTCOLORS_READ, MSG_HELP_CO_CH_TEXTCOLORS_READ);
    SetHelp(data->GUI.CH_SHOWALTPARTS,    MSG_HELP_CO_CH_SHOWALTPARTS);
    SetHelp(data->GUI.CH_DELAYEDSTATUS,   MSG_HELP_CO_SETSTATUSDELAYED);
    SetHelp(data->GUI.NB_DELAYEDSTATUS,   MSG_HELP_CO_SETSTATUSDELAYED);
    SetHelp(data->GUI.CH_CONVERTHTML,     MSG_HELP_CO_CONVERTHTML);
    SetHelp(data->GUI.CH_MDN_NEVER,       MSG_HELP_CO_CH_MDN_NEVER);
    SetHelp(data->GUI.CH_MDN_ALLOW,       MSG_HELP_CO_CH_MDN_ALLOW);
    SetHelp(data->GUI.CY_MDN_NORECIPIENT, MSG_HELP_CO_CY_MDN_NORECIPIENT);
    SetHelp(data->GUI.CY_MDN_NODOMAIN,    MSG_HELP_CO_CY_MDN_NODOMAIN);
    SetHelp(data->GUI.CY_MDN_DELETE,      MSG_HELP_CO_CY_MDN_DELETE);
    SetHelp(data->GUI.CY_MDN_OTHER,       MSG_HELP_CO_CY_MDN_OTHER);
    SetHelp(data->GUI.TX_DEFCHARSET_READ, MSG_HELP_CO_TX_DEFAULTCHARSET);
    SetHelp(data->GUI.CH_MAPFOREIGNCHARS, MSG_HELP_CO_MAPFOREIGNCHARS);
    SetHelp(data->GUI.CH_DETECTCYRILLIC,  MSG_HELP_CO_DETECT_CYRILLIC);
    SetHelp(data->GUI.CH_GLOBALMAILTHREADS, MSG_HELP_CO_CH_GLOBALMAILTHREADS);

    // disable all poppen objects in case the textstyles checkbox is disabled
    DoMethod(data->GUI.CH_TEXTCOLORS_READ, MUIM_Notify, MUIA_Selected, MUIV_EveryTime,
             MUIV_Notify_Application, 11, MUIM_MultiSet, MUIA_Disabled, MUIV_NotTriggerValue, data->GUI.CA_COLSIG,
                                                                                              data->GUI.CA_COLTEXT,
                                                                                              data->GUI.CA_COL1QUOT,
                                                                                              data->GUI.CA_COL2QUOT,
                                                                                              data->GUI.CA_COL3QUOT,
                                                                                              data->GUI.CA_COL4QUOT,
                                                                                              data->GUI.CA_COLURL,
                                                                                              NULL);

    DoMethod(data->GUI.CY_HEADER, MUIM_Notify, MUIA_Cycle_Active, 0, MUIV_Notify_Application, 7, MUIM_MultiSet, MUIA_Disabled, TRUE, data->GUI.ST_HEADERS,
                                                                                                                                     data->GUI.CH_WRAPHEAD,
                                                                                                                                     data->GUI.CY_SENDERINFO,
                                                                                                                                     NULL);
    DoMethod(data->GUI.CY_HEADER, MUIM_Notify, MUIA_Cycle_Active, 1, MUIV_Notify_Application, 7, MUIM_MultiSet, MUIA_Disabled, FALSE, data->GUI.ST_HEADERS,
                                                                                                                                      data->GUI.CH_WRAPHEAD,
                                                                                                                                      data->GUI.CY_SENDERINFO,
                                                                                                                                      NULL);
    DoMethod(data->GUI.CY_HEADER, MUIM_Notify, MUIA_Cycle_Active, 2, MUIV_Notify_Application, 5, MUIM_MultiSet, MUIA_Disabled, TRUE, data->GUI.ST_HEADERS,
                                                                                                                                     NULL);
    DoMethod(data->GUI.CY_HEADER, MUIM_Notify, MUIA_Cycle_Active, 2, MUIV_Notify_Application, 6, MUIM_MultiSet, MUIA_Disabled, FALSE, data->GUI.CH_WRAPHEAD,
                                                                                                                                      data->GUI.CY_SENDERINFO,
                                                                                                                                      NULL);

    // setup the MDN stuff
    DoMethod(data->GUI.CH_MDN_NEVER, MUIM_Notify, MUIA_Selected, TRUE,  MUIV_Notify_Application, 3, MUIM_CallHook, &MDNRequestHook, FALSE);
    DoMethod(data->GUI.CH_MDN_NEVER, MUIM_Notify, MUIA_Selected, FALSE, MUIV_Notify_Application, 3, MUIM_CallHook, &MDNRequestHook, TRUE);
    DoMethod(data->GUI.CH_MDN_ALLOW, MUIM_Notify, MUIA_Selected, TRUE,  MUIV_Notify_Application, 3, MUIM_CallHook, &MDNRequestHook, TRUE);
    DoMethod(data->GUI.CH_MDN_ALLOW, MUIM_Notify, MUIA_Selected, FALSE, MUIV_Notify_Application, 3, MUIM_CallHook, &MDNRequestHook, FALSE);

    DoMethod(data->GUI.CH_DELAYEDSTATUS, MUIM_Notify, MUIA_Selected, MUIV_EveryTime, data->GUI.NB_DELAYEDSTATUS, 3, MUIM_Set, MUIA_Disabled, MUIV_NotTriggerValue);
  }

  RETURN(obj);
  return obj;
}

///
/// CO_PageWrite
Object *CO_PageWrite(struct CO_ClassData *data)
{
  Object *obj;
  Object *charsetPopButton;
  static const char *wrapmode[4];

  wrapmode[0] = tr(MSG_CO_EWOff);
  wrapmode[1] = tr(MSG_CO_EWAsYouType);
  wrapmode[2] = tr(MSG_CO_EWBeforeSend);
  wrapmode[3] = NULL;

  ENTER();

  obj = VGroup,
          MUIA_HelpNode, "CO05",

          ConfigPageHeaderObject("config_write_big", G->theme.configImages[CI_WRITEBIG], tr(MSG_CO_WRITE_TITLE), tr(MSG_CO_WRITE_SUMMARY)),

          Child, ScrollgroupObject,
            MUIA_Scrollgroup_FreeHoriz, FALSE,
            MUIA_Scrollgroup_AutoBars, TRUE,
            MUIA_Scrollgroup_Contents, VGroupV,

              Child, ColGroup(2), GroupFrameT(tr(MSG_CO_MessageHeader)),
                Child, Label2(tr(MSG_CO_ReplyTo)),
                Child, MakeAddressField(&data->GUI.ST_REPLYTO, tr(MSG_CO_ReplyTo), MSG_HELP_CO_ST_REPLYTO, ABM_CONFIG, -1, TRUE),
                Child, Label2(tr(MSG_CO_Organization)),
                Child, data->GUI.ST_ORGAN = MakeString(SIZE_DEFAULT,tr(MSG_CO_Organization)),
                Child, Label2(tr(MSG_CO_ExtraHeaders)),
                Child, data->GUI.ST_EXTHEADER = MakeString(SIZE_LARGE,tr(MSG_CO_ExtraHeaders)),
              End,

              Child, VGroup, GroupFrameT(tr(MSG_CO_MessageBody)),
                Child, ColGroup(2),
                  Child, Label2(tr(MSG_CO_Welcome)),
                  Child, data->GUI.ST_HELLOTEXT = MakeString(SIZE_INTRO,tr(MSG_CO_Welcome)),
                  Child, Label2(tr(MSG_CO_Greetings)),
                  Child, data->GUI.ST_BYETEXT = MakeString(SIZE_INTRO,tr(MSG_CO_Greetings)),
                End,
              End,

              Child, VGroup, GroupFrameT(tr(MSG_CO_Editor)),
                Child, ColGroup(3),
                  Child, Label2(tr(MSG_CO_WordWrap)),
                  Child, HGroup,
                    Child, data->GUI.ST_EDWRAP = MakeInteger(3, tr(MSG_CO_WordWrap)),
                    Child, data->GUI.CY_EDWRAP = MakeCycle(wrapmode, ""),
                  End,
                  Child, RectangleObject,
                    MUIA_VertWeight,          0,
                    MUIA_Rectangle_HBar,      TRUE,
                    MUIA_Rectangle_BarTitle,  tr(MSG_CO_FONTSETTINGS),
                  End,

                  Child, Label2(tr(MSG_CO_ExternalEditor)),
                  Child, HGroup,
                    Child, PopaslObject,
                      MUIA_Popasl_Type     ,ASL_FileRequest,
                      MUIA_Popstring_String,data->GUI.ST_EDITOR = MakeString(SIZE_PATHFILE,tr(MSG_CO_ExternalEditor)),
                      MUIA_Popstring_Button,PopButton(MUII_PopFile),
                    End,
                    Child, MakeCheckGroup((Object **)&data->GUI.CH_LAUNCH, tr(MSG_CO_Launch)),
                  End,
                  Child, MakeCheckGroup((Object **)&data->GUI.CH_FIXEDFONT_WRITE, tr(MSG_CO_FIXEDFONT_WRITE)),

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
                  Child, MakeCheckGroup((Object **)&data->GUI.CH_TEXTCOLORS_WRITE, tr(MSG_CO_TEXTCOLORS_WRITE)),

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
                  Child, MakeCheckGroup((Object **)&data->GUI.CH_TEXTSTYLES_WRITE, tr(MSG_CO_TEXTSTYLES_WRITE)),

                  Child, Label2(tr(MSG_CO_DEFAULTCHARSET)),
                  Child, MakeCharsetPop((Object **)&data->GUI.TX_DEFCHARSET_WRITE, &charsetPopButton),
                  Child, HSpace(1),

                End,
              End,

              Child, VGroup, GroupFrameT(tr(MSG_CO_OtherOptions)),
                Child, MakeCheckGroup((Object **)&data->GUI.CH_WARNSUBJECT, tr(MSG_CO_WARNSUBJECT)),
                Child, MakeCheckGroup((Object **)&data->GUI.CH_REQUESTMDN, tr(MSG_CO_REQUESTMDN)),
                Child, MakeCheckGroup((Object **)&data->GUI.CH_SAVESENT, tr(MSG_CO_SaveSent)),
              End,

              Child, HVSpace,

            End,
          End,

        End;

  if(obj != NULL)
  {
    set(charsetPopButton, MUIA_ControlChar, ShortCut(tr(MSG_CO_DEFAULTCHARSET)));

    SetHelp(data->GUI.ST_REPLYTO,          MSG_HELP_CO_ST_REPLYTO);
    SetHelp(data->GUI.ST_ORGAN,            MSG_HELP_CO_ST_ORGAN);
    SetHelp(data->GUI.ST_EXTHEADER,        MSG_HELP_CO_ST_EXTHEADER);
    SetHelp(data->GUI.ST_HELLOTEXT,        MSG_HELP_CO_ST_HELLOTEXT);
    SetHelp(data->GUI.ST_BYETEXT,          MSG_HELP_CO_ST_BYETEXT);
    SetHelp(data->GUI.CH_WARNSUBJECT,      MSG_HELP_CO_CH_WARNSUBJECT);
    SetHelp(data->GUI.ST_EDWRAP,           MSG_HELP_CO_ST_EDWRAP);
    SetHelp(data->GUI.CY_EDWRAP,           MSG_HELP_CO_CY_EDWRAP);
    SetHelp(data->GUI.ST_EDITOR,           MSG_HELP_CO_ST_EDITOR);
    SetHelp(data->GUI.CH_LAUNCH,           MSG_HELP_CO_CH_LAUNCH);
    SetHelp(data->GUI.NB_EMAILCACHE,       MSG_HELP_CO_NB_EMAILCACHE);
    SetHelp(data->GUI.NB_AUTOSAVE,         MSG_HELP_CO_NB_AUTOSAVE);
    SetHelp(data->GUI.CH_REQUESTMDN,       MSG_HELP_CO_CH_REQUESTMDN);
    SetHelp(data->GUI.CH_SAVESENT,         MSG_HELP_CO_CH_SAVESENT);
    SetHelp(data->GUI.TX_DEFCHARSET_WRITE, MSG_HELP_CO_TX_DEFAULTCHARSET);
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

  fwdmode[0] = tr(MSG_CO_FWDMSG_ATTACH);
  fwdmode[1] = tr(MSG_CO_FWDMSG_INLINE);
  fwdmode[2] = NULL;

  ENTER();

  obj = VGroup,
          MUIA_HelpNode, "CO06",

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

                  Child, Label2(tr(MSG_CO_AltRepPat)),
                  Child, data->GUI.ST_AREPLYPAT = MakeString(SIZE_PATTERN, tr(MSG_CO_AltRepPat)),

                  Child, Label2(tr(MSG_CO_MLRepInit)),
                  Child, MakePhraseGroup(&data->GUI.ST_MREPLYHI, &data->GUI.ST_MREPLYTEXT, &data->GUI.ST_MREPLYBYE, tr(MSG_CO_MLRepInit), tr(MSG_HELP_CO_ST_MREPLYTEXT)),

                  Child, HSpace(1),
                  Child, MakeCheckGroup((Object **)&data->GUI.CH_COMPADDR, tr(MSG_CO_VerifyAddress)),

                  Child, HSpace(1),
                  Child, MakeCheckGroup((Object **)&data->GUI.CH_QUOTE, tr(MSG_CO_DoQuote)),

                  Child, HSpace(1),
                  Child, ColGroup(2),
                    Child, HSpace(5),
                    Child, MakeCheckGroup((Object **)&data->GUI.CH_QUOTEEMPTY, tr(MSG_CO_QuoteEmpty)),

                    Child, HSpace(5),
                    Child, MakeCheckGroup((Object **)&data->GUI.CH_STRIPSIG, tr(MSG_CO_StripSignature)),
                  End,

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
    SetHelp(data->GUI.CH_QUOTE,       MSG_HELP_CO_CH_QUOTE);
    SetHelp(data->GUI.CH_QUOTEEMPTY,  MSG_HELP_CO_CH_QUOTEEMPTY);
    SetHelp(data->GUI.CH_COMPADDR,    MSG_HELP_CO_CH_COMPADDR);
    SetHelp(data->GUI.CH_STRIPSIG,    MSG_HELP_CO_CH_STRIPSIG);
    SetHelp(data->GUI.CY_FORWARDMODE, MSG_HELP_CO_CY_FORWARDMODE);

    DoMethod(data->GUI.CH_QUOTE, MUIM_Notify, MUIA_Selected, MUIV_EveryTime, data->GUI.CH_QUOTEEMPTY, 3, MUIM_Set, MUIA_Disabled, MUIV_NotTriggerValue);
    DoMethod(data->GUI.CH_QUOTE, MUIM_Notify, MUIA_Selected, MUIV_EveryTime, data->GUI.CH_STRIPSIG, 3, MUIM_Set, MUIA_Disabled, MUIV_NotTriggerValue);
  }

  RETURN(obj);
  return obj;
}

///
/// CO_PageSignature
Object *CO_PageSignature(struct CO_ClassData *data)
{
   static const char *signat[4];
   Object *grp;
   Object *slider = ScrollbarObject, End;

   signat[0] = tr(MSG_CO_DefSig);
   signat[1] = tr(MSG_CO_AltSig1);
   signat[2] = tr(MSG_CO_AltSig2);
   signat[3] = NULL;

   if ((grp = VGroup,
         MUIA_HelpNode, "CO07",

         ConfigPageHeaderObject("config_signature_big", G->theme.configImages[CI_SIGNATUREBIG], tr(MSG_CO_SIGNATURE_TITLE), tr(MSG_CO_SIGNATURE_SUMMARY)),

          Child, ScrollgroupObject,
            MUIA_Scrollgroup_FreeHoriz, FALSE,
            MUIA_Scrollgroup_AutoBars, TRUE,
            MUIA_Scrollgroup_Contents, VGroupV,

             Child, VGroup, GroupFrameT(tr(MSG_CO_Signature)),
                Child, MakeCheckGroup((Object **)&data->GUI.CH_USESIG, tr(MSG_CO_UseSig)),
                Child, HGroup,
                   Child, data->GUI.CY_SIGNAT = MakeCycle(signat,""),
                   Child, data->GUI.BT_SIGEDIT = MakeButton(tr(MSG_CO_EditSig)),
                End,
                Child, HGroup,
                   MUIA_Group_Spacing, 0,
                   Child, data->GUI.TE_SIGEDIT = MailTextEditObject,
                      InputListFrame,
                      MUIA_CycleChain,            TRUE,
                      MUIA_TextEditor_FixedFont,  TRUE,
                      MUIA_TextEditor_ExportHook, MUIV_TextEditor_ExportHook_EMail,
                      MUIA_TextEditor_Slider,     slider,
                      MUIA_TextEditor_WrapMode,   MUIV_TextEditor_WrapMode_HardWrap,
                      MUIA_TextEditor_WrapBorder, C->EdWrapCol,
                   End,
                   Child, slider,
                End,
                Child, ColGroup(2),
                   Child, data->GUI.BT_INSTAG = MakeButton(tr(MSG_CO_InsertTag)),
                   Child, data->GUI.BT_INSENV = MakeButton(tr(MSG_CO_InsertENV)),
                End,
             End,
             Child, VGroup, GroupFrameT(tr(MSG_CO_Taglines)),
                Child, ColGroup(2),
                   Child, Label2(tr(MSG_CO_TaglineFile)),
                   Child, PopaslObject,
                      MUIA_Popasl_Type     ,ASL_FileRequest,
                      MUIA_Popstring_String,data->GUI.ST_TAGFILE = MakeString(SIZE_PATHFILE,tr(MSG_CO_TaglineFile)),
                      MUIA_Popstring_Button,PopButton(MUII_PopFile),
                   End,
                   Child, Label2(tr(MSG_CO_TaglineSep)),
                   Child, data->GUI.ST_TAGSEP = MakeString(SIZE_SMALL,tr(MSG_CO_TaglineSep)),
                End,
             End,

             Child, HVSpace,

           End,
         End,

      End))
   {
      SetHelp(data->GUI.CH_USESIG,  MSG_HELP_CO_CH_USESIG   );
      SetHelp(data->GUI.CY_SIGNAT,  MSG_HELP_CO_CY_SIGNAT   );
      SetHelp(data->GUI.BT_SIGEDIT, MSG_HELP_CO_BT_EDITSIG  );
      SetHelp(data->GUI.BT_INSTAG,  MSG_HELP_CO_BT_INSTAG   );
      SetHelp(data->GUI.BT_INSENV,  MSG_HELP_CO_BT_INSENV   );
      SetHelp(data->GUI.ST_TAGFILE, MSG_HELP_CO_ST_TAGFILE  );
      SetHelp(data->GUI.ST_TAGSEP,  MSG_HELP_CO_ST_TAGSEP   );

      DoMethod(data->GUI.BT_INSTAG, MUIM_Notify, MUIA_Pressed,      FALSE         , data->GUI.TE_SIGEDIT   , 3, MUIM_TextEditor_InsertText, "%t\n", MUIV_TextEditor_InsertText_Cursor);
      DoMethod(data->GUI.BT_INSENV, MUIM_Notify, MUIA_Pressed,      FALSE         , data->GUI.TE_SIGEDIT   , 3, MUIM_TextEditor_InsertText, "%e\n", MUIV_TextEditor_InsertText_Cursor);
      DoMethod(data->GUI.CY_SIGNAT, MUIM_Notify, MUIA_Cycle_Active, MUIV_EveryTime, MUIV_Notify_Application, 3, MUIM_CallHook, &CO_EditSignatHook, FALSE);
      DoMethod(data->GUI.BT_SIGEDIT,MUIM_Notify, MUIA_Pressed,      FALSE         , MUIV_Notify_Application, 3, MUIM_CallHook, &CO_EditSignatHook, TRUE);
      DoMethod(data->GUI.CH_USESIG, MUIM_Notify, MUIA_Selected,     MUIV_EveryTime, MUIV_Notify_Application, 3, MUIM_CallHook, &CO_SwitchSignatHook, MUIV_NotTriggerValue);
   }

   return grp;
}

///
/// CO_PageLists
Object *CO_PageLists(struct CO_ClassData *data)
{
  static const char *folderf[6];
  Object *obj;

  ENTER();

  folderf[0] = tr(MSG_CO_FOLDERINFO01);
  folderf[1] = tr(MSG_CO_FOLDERINFO02);
  folderf[2] = tr(MSG_CO_FOLDERINFO03);
  folderf[3] = tr(MSG_CO_FOLDERINFO04);
  folderf[4] = tr(MSG_CO_FOLDERINFO05);
  folderf[5] = NULL;

  obj = VGroup,
          MUIA_HelpNode, "CO08",

          ConfigPageHeaderObject("config_lists_big", G->theme.configImages[CI_LISTSBIG], tr(MSG_CO_LISTS_TITLE), tr(MSG_CO_LISTS_SUMMARY)),

          Child, ScrollgroupObject,
            MUIA_Scrollgroup_FreeHoriz, FALSE,
            MUIA_Scrollgroup_AutoBars, TRUE,
            MUIA_Scrollgroup_Contents, VGroupV,

            Child, HGroup, GroupFrameT(tr(MSG_CO_FIELDLISTCFG)),

              Child, VGroup,
                Child, RectangleObject,
                  MUIA_VertWeight,          0,
                  MUIA_Rectangle_HBar,      TRUE,
                  MUIA_Rectangle_BarTitle,  tr(MSG_FolderList),
                End,
                Child, ColGroup(2),
                  MUIA_ShortHelp, tr(MSG_HELP_CO_CG_FO),

                  Child, MakeStaticCheck(),
                  Child, data->GUI.CY_FOLDERINFO = MakeCycle(folderf, tr(MSG_CO_FOLDERLABEL)),

                  Child, data->GUI.CH_FCOLS[1] = MakeCheck(""),
                  Child, LLabel(tr(MSG_Total)),

                  Child, data->GUI.CH_FCOLS[2] = MakeCheck(""),
                  Child, LLabel(tr(MSG_Unread)),

                  Child, data->GUI.CH_FCOLS[3] = MakeCheck(""),
                  Child, LLabel(tr(MSG_New)),

                  Child, data->GUI.CH_FCOLS[4] = MakeCheck(""),
                  Child, LLabel(tr(MSG_Size)),

                  Child, data->GUI.CH_FCNTMENU = MakeCheck(""),
                  Child, LLabel(tr(MSG_CO_CONTEXTMENU)),

                End,
                Child, HVSpace,
              End,

              Child, HSpace(0),

              Child, VGroup,
                Child, RectangleObject,
                  MUIA_VertWeight,          0,
                  MUIA_Rectangle_HBar,      TRUE,
                  MUIA_Rectangle_BarTitle,  tr(MSG_MessageList),
                End,
                Child, ColGroup(2),
                  MUIA_ShortHelp, tr(MSG_HELP_CO_CG_MA),

                  Child, MakeStaticCheck(),
                  Child, LLabel(tr(MSG_Status)),

                  Child, data->GUI.CH_MCOLS[1] = MakeCheck(""),
                  Child, LLabel(tr(MSG_SenderRecpt)),

                  Child, data->GUI.CH_MCOLS[2] = MakeCheck(""),
                  Child, LLabel(tr(MSG_ReturnAddress)),

                  Child, data->GUI.CH_MCOLS[3] = MakeCheck(""),
                  Child, LLabel(tr(MSG_Subject)),

                  Child, data->GUI.CH_MCOLS[4] = MakeCheck(""),
                  Child, LLabel(tr(MSG_MessageDate)),

                  Child, data->GUI.CH_MCOLS[5] = MakeCheck(""),
                  Child, LLabel(tr(MSG_Size)),

                  Child, data->GUI.CH_MCOLS[6] = MakeCheck(""),
                  Child, LLabel(tr(MSG_Filename)),

                  Child, data->GUI.CH_MCOLS[7] = MakeCheck(""),
                  Child, LLabel(tr(MSG_CO_DATE_SNTRCVD)),

                  Child, data->GUI.CH_MCNTMENU = MakeCheck(""),
                  Child, LLabel(tr(MSG_CO_CONTEXTMENU)),
                End,
              End,
            End,

            Child, VGroup, GroupFrameT(tr(MSG_CO_GENLISTCFG)),
              Child, MakeCheckGroup((Object **)&data->GUI.CH_FIXFLIST, tr(MSG_CO_FixedFontList)),
              Child, MakeCheckGroup((Object **)&data->GUI.CH_BEAT, tr(MSG_CO_SwatchBeat)),
              Child, MakeCheckGroup((Object **)&data->GUI.CH_RELDATETIME, tr(MSG_CO_RELDATETIME)),
              Child, MakeCheckGroup((Object **)&data->GUI.CH_ABOOKLOOKUP, tr(MSG_CO_ABOOKLOOKUP)),
              Child, MakeCheckGroup((Object **)&data->GUI.CH_FOLDERDBLCLICK, tr(MSG_CO_FOLDERDBLCLICK)),
            End,

            Child, HVSpace,
            End,
          End,
        End;

  if(obj != NULL)
  {
    SetHelp(data->GUI.CH_FIXFLIST,      MSG_HELP_CO_CH_FIXFLIST);
    SetHelp(data->GUI.CH_BEAT,          MSG_HELP_CO_CH_BEAT);
    SetHelp(data->GUI.CH_RELDATETIME,   MSG_HELP_CO_CH_RELDATETIME);
    SetHelp(data->GUI.CH_ABOOKLOOKUP,   MSG_HELP_CO_CH_ABOOKLOOKUP);
    SetHelp(data->GUI.CH_FCNTMENU,      MSG_HELP_CO_CONTEXTMENU);
    SetHelp(data->GUI.CH_MCNTMENU,      MSG_HELP_CO_CONTEXTMENU);
    SetHelp(data->GUI.CY_FOLDERINFO,    MSG_HELP_CO_CY_FOLDERINFO);
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
          MUIA_HelpNode, "CO09",

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

                  Child, Label2(tr(MSG_CO_PGPKey)),
                  Child, HGroup,
                    Child, MakePGPKeyList(&(data->GUI.ST_MYPGPID), TRUE, tr(MSG_CO_PGPKey)),
                    Child, HSpace(8),
                    Child, data->GUI.CH_ENCSELF = MakeCheck(tr(MSG_CO_EncryptToSelf)),
                    Child, Label1(tr(MSG_CO_EncryptToSelf)),
                  End,

                  Child, Label2(tr(MSG_CO_PGPURL)),
                  Child, data->GUI.ST_PGPURL = MakeString(SIZE_URL, tr(MSG_CO_PGPURL)),

                End,
                Child, HGroup,
                  Child, data->GUI.CH_PGPPASSINTERVAL = MakeCheck(tr(MSG_CO_PGPPASSINTERVAL1)),
                  Child, Label2(tr(MSG_CO_PGPPASSINTERVAL1)),
                  Child, data->GUI.NB_PGPPASSINTERVAL = MakeNumeric(1, 90, FALSE),
                  Child, Label2(tr(MSG_CO_PGPPASSINTERVAL2)),
                  Child, HSpace(0),
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
                Child, MakeCheckGroup((Object **)&data->GUI.CH_SPLITLOG, tr(MSG_CO_LogSplit)),

                Child, HSpace(1),
                Child, MakeCheckGroup((Object **)&data->GUI.CH_LOGALL, tr(MSG_CO_LogAllEvents)),

              End,

              Child, HVSpace,

            End,
          End,

        End;

  if(obj != NULL)
  {
    SetHelp(data->GUI.ST_PGPCMD    ,MSG_HELP_CO_ST_PGPCMD   );
    SetHelp(data->GUI.ST_MYPGPID   ,MSG_HELP_CO_ST_MYPGPID  );
    SetHelp(data->GUI.ST_PGPURL,    MSG_HELP_CO_ST_PGPURL   );
    SetHelp(data->GUI.CH_ENCSELF   ,MSG_HELP_CO_CH_ENCSELF  );
    SetHelp(data->GUI.CH_PGPPASSINTERVAL, MSG_HELP_CO_PGPPASSINTERVAL);
    SetHelp(data->GUI.NB_PGPPASSINTERVAL, MSG_HELP_CO_PGPPASSINTERVAL);
    SetHelp(data->GUI.CH_ENCSELF   ,MSG_HELP_CO_CH_ENCSELF  );
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
   if ((grp = VGroup,
         MUIA_HelpNode, "CO10",

         ConfigPageHeaderObject("config_start_big", G->theme.configImages[CI_STARTBIG], tr(MSG_CO_STARTUP_TITLE), tr(MSG_CO_STARTUP_SUMMARY)),

          Child, ScrollgroupObject,
            MUIA_Scrollgroup_FreeHoriz, FALSE,
            MUIA_Scrollgroup_AutoBars, TRUE,
            MUIA_Scrollgroup_Contents, VGroupV,

             Child, VGroup, GroupFrameT(tr(MSG_CO_OnStartup)),
                Child, MakeCheckGroup((Object **)&data->GUI.CH_LOADALL, tr(MSG_CO_LoadAll)),
                Child, MakeCheckGroup((Object **)&data->GUI.CH_MARKNEW, tr(MSG_CO_MarkNew)),
                Child, MakeCheckGroup((Object **)&data->GUI.CH_DELETESTART, tr(MSG_CO_DeleteOld)),
                Child, MakeCheckGroup((Object **)&data->GUI.CH_REMOVESTART, tr(MSG_CO_RemoveDel)),
                Child, MakeCheckGroup((Object **)&data->GUI.CH_CHECKBD, tr(MSG_CO_CheckDOB)),
                Child, MakeCheckGroup((Object **)&data->GUI.CH_SENDSTART, tr(MSG_CO_SendStart)),
             End,
             Child, VGroup, GroupFrameT(tr(MSG_CO_OnTermination)),
                Child, MakeCheckGroup((Object **)&data->GUI.CH_SENDQUIT, tr(MSG_CO_SendStart)),
                Child, MakeCheckGroup((Object **)&data->GUI.CH_DELETEQUIT, tr(MSG_CO_DeleteOld)),
                Child, MakeCheckGroup((Object **)&data->GUI.CH_REMOVEQUIT, tr(MSG_CO_RemoveDel)),
             End,
             Child, HVSpace,

           End,
         End,
      End))
   {
      SetHelp(data->GUI.CH_LOADALL    ,MSG_HELP_CO_CH_LOADALL   );
      SetHelp(data->GUI.CH_MARKNEW    ,MSG_HELP_CO_CH_MARKNEW   );
      SetHelp(data->GUI.CH_DELETESTART,MSG_HELP_CO_CH_DELETEOLD );
      SetHelp(data->GUI.CH_REMOVESTART,MSG_HELP_CO_CH_REMOVEDEL );
      SetHelp(data->GUI.CH_SENDSTART  ,MSG_HELP_CO_CH_SEND      );
      SetHelp(data->GUI.CH_CHECKBD    ,MSG_HELP_CO_CH_CHECKBD   );
      SetHelp(data->GUI.CH_SENDQUIT   ,MSG_HELP_CO_CH_SEND      );
      SetHelp(data->GUI.CH_DELETEQUIT ,MSG_HELP_CO_CH_DELETEOLD );
      SetHelp(data->GUI.CH_REMOVEQUIT ,MSG_HELP_CO_CH_REMOVEDEL );
   }

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
          MUIA_HelpNode, "CO11",

          ConfigPageHeaderObject("config_mime_big", G->theme.configImages[CI_MIMEBIG], tr(MSG_CO_MIME_TITLE), tr(MSG_CO_MIME_SUMMARY)),

          Child, ScrollgroupObject,
            MUIA_Scrollgroup_FreeHoriz, FALSE,
            MUIA_Scrollgroup_AutoBars, TRUE,
            MUIA_Scrollgroup_Contents, VGroupV,

              Child, VGroup,
                Child, HGroup,
                  Child, VGroup,
                    MUIA_Weight, 30,
                    Child, NListviewObject,
                       MUIA_CycleChain, TRUE,
                       MUIA_NListview_NList, data->GUI.LV_MIME = MimeTypeListObject,
                       End,
                    End,

                    Child, HGroup,
                       Child, data->GUI.BT_MADD = MakeButton(tr(MSG_Add)),
                       Child, data->GUI.BT_MDEL = MakeButton(tr(MSG_Del)),
                       Child, data->GUI.BT_MIMEIMPORT = PopButton(MUII_PopFile),
                    End,
                  End,

                  Child, NBalanceObject, End,

                  Child, VGroup,
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
/// CO_PageAddressBook
Object *CO_PageAddressBook(struct CO_ClassData *data)
{
   Object *grp;
   static const char *atab[6];

   atab[0] = tr(MSG_CO_ATABnever);
   atab[1] = tr(MSG_CO_ATABinfoask);
   atab[2] = tr(MSG_CO_ATABask);
   atab[3] = tr(MSG_CO_ATABinfo);
   atab[4] = tr(MSG_CO_ATABalways);
   atab[5] = NULL;

   if ((grp = VGroup,
         MUIA_HelpNode, "CO12",

         ConfigPageHeaderObject("config_abook_big", G->theme.configImages[CI_ABOOKBIG], tr(MSG_CO_ABOOK_TITLE), tr(MSG_CO_ABOOK_SUMMARY)),

          Child, ScrollgroupObject,
            MUIA_Scrollgroup_FreeHoriz, FALSE,
            MUIA_Scrollgroup_AutoBars, TRUE,
            MUIA_Scrollgroup_Contents, VGroupV,

             Child, HGroup, GroupFrameT(tr(MSG_Columns)),
                Child, HVSpace,
                Child, ColGroup(4),
                   MUIA_ShortHelp, tr(MSG_HELP_CO_CG_AB),
                   Child, data->GUI.CH_ACOLS[1] = MakeCheck(""),
                   Child, LLabel(tr(MSG_Realname)),
                   Child, data->GUI.CH_ACOLS[2] = MakeCheck(""),
                   Child, LLabel(tr(MSG_Description)),
                   Child, data->GUI.CH_ACOLS[3] = MakeCheck(""),
                   Child, LLabel(tr(MSG_Email)),
                   Child, data->GUI.CH_ACOLS[4] = MakeCheck(""),
                   Child, LLabel(tr(MSG_Street)),
                   Child, data->GUI.CH_ACOLS[5] = MakeCheck(""),
                   Child, LLabel(tr(MSG_City)),
                   Child, data->GUI.CH_ACOLS[6] = MakeCheck(""),
                   Child, LLabel(tr(MSG_Country)),
                   Child, data->GUI.CH_ACOLS[7] = MakeCheck(""),
                   Child, LLabel(tr(MSG_Phone)),
                   Child, data->GUI.CH_ACOLS[8] = MakeCheck(""),
                   Child, LLabel(tr(MSG_DOB)),
                End,
                Child, HVSpace,
             End,
             Child, ColGroup(2), GroupFrameT(tr(MSG_CO_InfoExc)),
                Child, Label1(tr(MSG_CO_AddInfo)),
                Child, HGroup,
                   Child, data->GUI.CH_ADDINFO = MakeCheck(tr(MSG_CO_AddInfo)),
                   Child, HSpace(0),
                End,
                Child, Label1(tr(MSG_CO_AddToAddrbook)),
                Child, data->GUI.CY_ATAB = MakeCycle(atab, tr(MSG_CO_AddToAddrbook)),
                Child, Label2(tr(MSG_CO_NewGroup)),
                Child, data->GUI.ST_NEWGROUP = MakeString(SIZE_NAME,tr(MSG_CO_NewGroup)),
                Child, Label2(tr(MSG_CO_Gallery)),
                Child, PopaslObject,
                   MUIA_Popasl_Type     ,ASL_FileRequest,
                   MUIA_Popstring_String,data->GUI.ST_GALLDIR = MakeString(SIZE_PATH,tr(MSG_CO_Gallery)),
                   MUIA_Popstring_Button,PopButton(MUII_PopDrawer),
                   ASLFR_DrawersOnly, TRUE,
                End,
                Child, Label2(tr(MSG_CO_MyURL)),
                Child, data->GUI.ST_PHOTOURL = MakeString(SIZE_URL,tr(MSG_CO_MyURL)),
                Child, Label2(tr(MSG_CO_ProxyServer)),
                Child, data->GUI.ST_PROXY = MakeString(SIZE_HOST,tr(MSG_CO_ProxyServer)),
             End,
             Child, HVSpace,
           End,
         End,

      End))
   {
      SetHelp(data->GUI.ST_GALLDIR   ,MSG_HELP_CO_ST_GALLDIR   );
      SetHelp(data->GUI.ST_PHOTOURL  ,MSG_HELP_CO_ST_PHOTOURL  );
      SetHelp(data->GUI.ST_PROXY     ,MSG_HELP_CO_ST_PROXY     );
      SetHelp(data->GUI.ST_NEWGROUP  ,MSG_HELP_CO_ST_NEWGROUP  );
      SetHelp(data->GUI.CY_ATAB      ,MSG_HELP_CO_CY_ATAB      );
      SetHelp(data->GUI.CH_ADDINFO   ,MSG_HELP_WR_CH_ADDINFO   );

      DoMethod(data->GUI.CY_ATAB, MUIM_Notify, MUIA_Cycle_Active, MUIV_EveryTime, data->GUI.ST_NEWGROUP, 3, MUIM_Set, MUIA_Disabled, MUIV_NotTriggerValue);
   }
   return grp;
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

   if ((grp = VGroup,
         MUIA_HelpNode, "CO13",

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
                   Child, MakeCheckGroup((Object **)&data->GUI.CH_CONSOLE, tr(MSG_CO_OpenConsole)),

                   Child, HSpace(1),
                   Child, MakeCheckGroup((Object **)&data->GUI.CH_WAITTERM, tr(MSG_CO_WaitTerm)),

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
   return grp;
}

///
/// CO_PageMixed
Object *CO_PageMixed(struct CO_ClassData *data)
{
  Object *obj;
  Object *popButton;

  ENTER();

  obj = VGroup,
          MUIA_HelpNode, "CO14",

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
                Child, MakeCheckGroup((Object **)&data->GUI.CH_DOCKYICON, tr(MSG_CO_DOCKYICON)),
                Child, MakeCheckGroup((Object **)&data->GUI.CH_CLGADGET, tr(MSG_CO_CloseGadget)),
              End,

              Child, VGroup, GroupFrameT(tr(MSG_CO_SaveDelete)),
                Child, HGroup,
                  Child, data->GUI.CH_CONFIRM = MakeCheck(tr(MSG_CO_ConfirmDelPart1)),
                  Child, Label2(tr(MSG_CO_ConfirmDelPart1)),
                  Child, data->GUI.NB_CONFIRMDEL = MakeNumeric(1,50,FALSE),
                  Child, Label2(tr(MSG_CO_ConfirmDelPart2)),
                  Child, HSpace(0),
                End,
                Child, MakeCheckGroup((Object **)&data->GUI.CH_REMOVE, tr(MSG_CO_Remove)),
              End,
              Child, HGroup, GroupFrameT(tr(MSG_CO_XPK)),
                Child, ColGroup(5),
                  Child, Label1(tr(MSG_CO_XPKPack)),
                  Child, MakeXPKPop(&data->GUI.TX_PACKER, FALSE),
                  Child, data->GUI.NB_PACKER = MakeNumeric(0,100,TRUE),
                  Child, HSpace(8),
                  Child, HGroup,
                    Child, LLabel(tr(MSG_CO_Archiver)),
                    Child, HSpace(0),
                  End,

                  Child, Label1(tr(MSG_CO_XPKPackEnc)),
                  Child, MakeXPKPop(&data->GUI.TX_ENCPACK, TRUE),
                  Child, data->GUI.NB_ENCPACK = MakeNumeric(0,100,TRUE),
                  Child, HSpace(8),
                  Child, MakeVarPop(&data->GUI.ST_ARCHIVER, &popButton, PHM_ARCHIVE, SIZE_COMMAND, tr(MSG_CO_Archiver)),
                End,
              End,

              Child, HVSpace,
            End,
          End,
        End;

  if(obj != NULL)
  {
    SetHelp(data->GUI.ST_TEMPDIR        ,MSG_HELP_CO_ST_TEMPDIR       );
    SetHelp(data->GUI.ST_DETACHDIR      ,MSG_HELP_CO_ST_DETACHDIR     );
    SetHelp(data->GUI.ST_ATTACHDIR      ,MSG_HELP_CO_ST_ATTACHDIR     );
    SetHelp(data->GUI.CH_WBAPPICON      ,MSG_HELP_CO_CH_WBAPPICON     );
    SetHelp(data->GUI.ST_APPX           ,MSG_HELP_CO_ST_APP           );
    SetHelp(data->GUI.ST_APPY           ,MSG_HELP_CO_ST_APP           );
    SetHelp(data->GUI.CH_APPICONPOS     ,MSG_HELP_CO_ST_APP           );
    SetHelp(data->GUI.CH_DOCKYICON      ,MSG_HELP_CO_CH_DOCKYICON     );
    SetHelp(data->GUI.CH_CLGADGET       ,MSG_HELP_CO_CH_CLGADGET      );
    SetHelp(data->GUI.CH_CONFIRM        ,MSG_HELP_CO_CH_CONFIRM       );
    SetHelp(data->GUI.NB_CONFIRMDEL     ,MSG_HELP_CO_NB_CONFIRMDEL    );
    SetHelp(data->GUI.CH_REMOVE         ,MSG_HELP_CO_CH_REMOVE        );
    SetHelp(data->GUI.TX_ENCPACK        ,MSG_HELP_CO_TX_ENCPACK       );
    SetHelp(data->GUI.TX_PACKER         ,MSG_HELP_CO_TX_PACKER        );
    SetHelp(data->GUI.NB_ENCPACK        ,MSG_HELP_CO_NB_ENCPACK       );
    SetHelp(data->GUI.NB_PACKER         ,MSG_HELP_CO_NB_ENCPACK       );
    SetHelp(data->GUI.ST_ARCHIVER       ,MSG_HELP_CO_ST_ARCHIVER      );
    SetHelp(data->GUI.ST_APPICON        ,MSG_HELP_CO_ST_APPICON       );
    SetHelp(data->GUI.BT_APPICONGETPOS  ,MSG_HELP_CO_BT_APPICONGETPOS );

    DoMethod(obj, MUIM_MultiSet, MUIA_Disabled, TRUE, data->GUI.ST_APPX, data->GUI.ST_APPY, data->GUI.ST_APPICON, data->GUI.BT_APPICONGETPOS, NULL);
    DoMethod(data->GUI.CH_WBAPPICON, MUIM_Notify, MUIA_Selected, MUIV_EveryTime, MUIV_Notify_Application, 9, MUIM_MultiSet, MUIA_Disabled, MUIV_NotTriggerValue, data->GUI.ST_APPX, data->GUI.ST_APPY, data->GUI.ST_APPICON, data->GUI.CH_APPICONPOS, data->GUI.BT_APPICONGETPOS, NULL);
    DoMethod(data->GUI.BT_APPICONGETPOS, MUIM_Notify, MUIA_Pressed, FALSE, MUIV_Notify_Application, 2, MUIM_CallHook, &GetAppIconPosHook);
    DoMethod(data->GUI.CH_CONFIRM, MUIM_Notify, MUIA_Selected, MUIV_EveryTime, data->GUI.NB_CONFIRMDEL, 3, MUIM_Set, MUIA_Disabled, MUIV_NotTriggerValue);

    #if defined(__amigaos4__)
    set(data->GUI.CH_DOCKYICON, MUIA_Disabled, G->applicationID == 0);
    #else
    set(data->GUI.CH_DOCKYICON, MUIA_Disabled, TRUE);
    #endif

    // disable the XPK popups if xpkmaster.library is not available
    if(XpkBase == NULL)
    {
      set(data->GUI.NB_PACKER, MUIA_Disabled, TRUE);
      set(data->GUI.NB_ENCPACK, MUIA_Disabled, TRUE);
    }
  }

  RETURN(obj);
  return obj;
}
///
/// CO_PageLookFeel
Object *CO_PageLookFeel(struct CO_ClassData *data)
{
  static const char *sizef[6];
  static const char *infob[5];
  Object *obj;
  Object *popButton;

  ENTER();

  sizef[0] = tr(MSG_CO_SIZEFORMAT01);
  sizef[1] = tr(MSG_CO_SIZEFORMAT02);
  sizef[2] = tr(MSG_CO_SIZEFORMAT03);
  sizef[3] = tr(MSG_CO_SIZEFORMAT04);
  sizef[4] = tr(MSG_CO_SIZEFORMAT05);
  sizef[5] = NULL;

  infob[0] = tr(MSG_CO_INFOBARPOS01);
  infob[1] = tr(MSG_CO_INFOBARPOS02);
  infob[2] = tr(MSG_CO_INFOBARPOS03);
  infob[3] = tr(MSG_CO_INFOBARPOS04);
  infob[4] = NULL;

  obj = VGroup,
          MUIA_HelpNode, "CO15",

          ConfigPageHeaderObject("config_lookfeel_big", G->theme.configImages[CI_LOOKFEELBIG], tr(MSG_CO_LOOKFEEL_TITLE), tr(MSG_CO_LOOKFEEL_SUMMARY)),

          Child, ScrollgroupObject,
            MUIA_Scrollgroup_FreeHoriz, FALSE,
            MUIA_Scrollgroup_AutoBars, TRUE,
            MUIA_Scrollgroup_Contents, VGroupV,

              Child, data->GUI.GR_THEMES = ThemeListGroupObject,
                GroupFrameT(tr(MSG_CO_LOOKFEEL_THEMES)),
                MUIA_VertWeight, 70,
              End,

              Child, ColGroup(2), GroupFrameT(tr(MSG_CO_INFOBAR)),
                Child, Label1(tr(MSG_CO_INFOBARPOS)),
                Child, data->GUI.CY_INFOBAR = MakeCycle(infob, tr(MSG_CO_INFOBARPOS)),

                Child, Label2(tr(MSG_CO_FOLDERLABEL)),
                Child, data->GUI.PO_INFOBARTXT = MakeVarPop(&data->GUI.ST_INFOBARTXT, &popButton, PHM_MAILSTATS, SIZE_DEFAULT, tr(MSG_CO_FOLDERLABEL)),
              End,

              Child, VGroup, GroupFrameT(tr(MSG_CO_GENLISTCFG)),
                Child, HGroup,
                  Child, Label1(tr(MSG_CO_SIZEFORMAT)),
                  Child, data->GUI.CY_SIZE = MakeCycle(sizef, tr(MSG_CO_SIZEFORMAT)),
                End,
                Child, MakeCheckGroup((Object **)&data->GUI.CH_EMBEDDEDREADPANE, tr(MSG_CO_SHOWEMBEDDEDREADPANE)),
                Child, MakeCheckGroup((Object **)&data->GUI.CH_QUICKSEARCHBAR, tr(MSG_CO_QUICKSEARCHBAR)),
              End,

              Child, HVSpace,

            End,
          End,

        End;

  if(obj != NULL)
  {
    SetHelp(data->GUI.CH_QUICKSEARCHBAR,   MSG_HELP_CO_CH_QUICKSEARCHBAR);
    SetHelp(data->GUI.CY_INFOBAR,          MSG_HELP_CO_CH_INFOBAR);
    SetHelp(data->GUI.ST_INFOBARTXT,       MSG_HELP_CO_ST_INFOBARTXT);
    SetHelp(data->GUI.CH_EMBEDDEDREADPANE, MSG_HELP_CO_CH_EMBEDDEDREADPANE);
    SetHelp(data->GUI.CY_SIZE,             MSG_HELP_CO_CY_SIZE);

    DoMethod(data->GUI.CY_INFOBAR, MUIM_Notify, MUIA_Cycle_Active, MUIV_EveryTime, MUIV_Notify_Application, 3, MUIM_CallHook, &InfoBarPosHook, MUIV_TriggerValue);
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
          MUIA_HelpNode, "CO16",

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
                Child, TextObject,
                  MUIA_Text_Contents, tr(MSG_CO_SEARCHFORUPDATESINFO),
                  MUIA_Font,          MUIV_Font_Tiny,
                End,
                Child, VSpace(10),
                Child, ColGroup(2),
                  Child, LLabel1(tr(MSG_CO_LASTSEARCH)),
                  Child, data->GUI.TX_UPDATESTATUS = TextObject,
                    MUIA_Text_Contents, tr(MSG_CO_LASTSTATUS_NOCHECK),
                  End,
                  Child, HSpace(1),
                  Child, data->GUI.TX_UPDATEDATE = TextObject,
                    MUIA_Text_Contents, "",
                  End,
                End,
              End,

              Child, HGroup,
                Child, Label2(tr(MSG_CO_UPDATE_DOWNLOAD_PATH)),
                Child, data->GUI.PO_UPDATEDOWNLOADPATH = PopaslObject,
                  MUIA_Popasl_Type, ASL_FileRequest,
                  MUIA_Popstring_String, data->GUI.ST_UPDATEDOWNLOADPATH = MakeString(SIZE_PATH, tr(MSG_CO_UPDATE_DOWNLOAD_PATH)),
                  MUIA_Popstring_Button, PopButton(MUII_PopDrawer),
                  ASLFR_DrawersOnly, TRUE,
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

    DoMethod(data->GUI.BT_UPDATENOW,   MUIM_Notify, MUIA_Pressed,  FALSE, MUIV_Notify_Application, 2, MUIM_CallHook, &UpdateCheckHook);
  }

  RETURN(obj);
  return obj;
}
///

