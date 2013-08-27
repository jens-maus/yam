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

#include <stdlib.h>
#include <string.h>

#include <clib/alib_protos.h>
#include <libraries/iffparse.h>
#include <mui/NBalance_mcc.h>
#include <mui/TextEditor_mcc.h>
#include <proto/exec.h>
#include <proto/intuition.h>
#include <proto/muimaster.h>

#include "extrasrc.h"

#include "SDI_hook.h"

#include "YAM.h"
#include "YAM_error.h"
#include "YAM_glossarydisplay.h"
#include "YAM_utilities.h"
#include "YAM_write.h"

#include "Busy.h"
#include "DynamicString.h"
#include "Locale.h"
#include "MUIObjects.h"

#include "mui/ClassesExtra.h"
#include "mui/MailTextEdit.h"
#include "mui/WriteWindow.h"

#include "Debug.h"

/* local protos */
static struct DI_ClassData *DI_New(void);

/***************************************************************************
 Module: Glossary
***************************************************************************/

/// DI_FinishEdit
//  Adds/updates changed glossary entry
static void DI_FinishEdit(void)
{
   struct DI_GUIData *gui = &G->DI->GUI;
   int modified = xget(gui->TE_EDIT, MUIA_TextEditor_HasChanged);
   if (modified && G->DI->OldEntry)
   {
      struct Dict new;
      char *edtext = (char *)DoMethod((Object *)gui->TE_EDIT, MUIM_TextEditor_ExportText);

      new.Text = NULL;
      dstrcpy(&new.Text, edtext ? edtext : "");
      if(G->DI->OldEntry->Text)
        dstrfree(G->DI->OldEntry->Text);

      GetMUIString(new.Alias, gui->ST_ALIAS, sizeof(new.Alias));
      if(*new.Alias == '\0')
        strlcpy(new.Alias, tr(MSG_NewEntry), sizeof(new.Alias));

      *(G->DI->OldEntry) = new;

      DoMethod(gui->LV_ENTRIES, MUIM_List_Redraw, MUIV_List_Redraw_All);

      G->DI->Modified = TRUE;
      FreeVec(edtext);
   }
   set(gui->TE_EDIT, MUIA_TextEditor_HasChanged, FALSE);
}

///
/// DI_Save
//  Saves glossary to disk
static void DI_Save(void)
{
   FILE *fh;

   if((fh = fopen(G->DI_Filename, "w")))
   {
     struct BusyNode *busy;
     int entries = xget(G->DI->GUI.LV_ENTRIES, MUIA_List_Entries);
     int i;

     setvbuf(fh, NULL, _IOFBF, SIZE_FILEBUF);

     busy = BusyBegin(BUSY_PROGRESS);
     BusyText(busy, tr(MSG_BusySavingDI), "");
     fputs("YDI1 - YAM Dictionary\n", fh);
     for (i = 0; ;i++)
     {
       struct Dict *entry;

       DoMethod(G->DI->GUI.LV_ENTRIES, MUIM_List_GetEntry, i, &entry);
       if(entry == NULL)
         break;

       fprintf(fh, "@ENTRY %s\n%s@ENDENTRY\n", entry->Alias, entry->Text);

       BusyProgress(busy, i+1, entries);
     }
     fclose(fh);
     G->DI->Modified = FALSE;
     BusyEnd(busy);
   }
   else
     ER_NewError(tr(MSG_ER_CantCreateFile), G->DI_Filename);
}

///
/// DI_Load
//  Load glossary from disk
static int DI_Load(void)
{
  int entries = 0;
  FILE *fh;

  ENTER();

  if((fh = fopen(G->DI_Filename, "r")) != NULL)
  {
    struct BusyNode *busy;
    char *buffer = NULL;
    size_t size = 0;

    setvbuf(fh, NULL, _IOFBF, SIZE_FILEBUF);

    busy = BusyBegin(BUSY_TEXT);
    BusyText(busy, tr(MSG_BusyLoadingDI), "");

    if(GetLine(&buffer, &size, fh) >= 3 && strncmp(buffer, "YDI", 3) == 0)
    {
      set(G->DI->GUI.LV_ENTRIES, MUIA_List_Quiet, TRUE);

      while(GetLine(&buffer, &size, fh) >= 0)
      {
        struct Dict entry;

        memset(&entry, 0, sizeof(struct Dict));
        if(strncmp(buffer, "@ENTRY", 6) == 0)
        {
          char *p;

          strlcpy(entry.Alias, Trim(&buffer[7]), sizeof(entry.Alias));
          entry.Text = dstralloc(SIZE_DEFAULT);

          while(GetLine(&buffer, &size, fh) >= 0)
          {
            if((p = strstr(buffer, "@ENDENTRY")) != NULL)
            {
              *p = 0;
              dstrcat(&entry.Text, buffer);
              break;
            }
            else
            {
              dstrcat(&entry.Text, buffer);
              dstrcat(&entry.Text, "\n");
            }
          }

          DoMethod(G->DI->GUI.LV_ENTRIES, MUIM_List_InsertSingle, &entry, MUIV_List_Insert_Bottom);
          entries++;
        }
      }

      set(G->DI->GUI.LV_ENTRIES, MUIA_List_Quiet, FALSE);
    }

    free(buffer);

    fclose(fh);
    BusyEnd(busy);
  }

  RETURN(entries);
  return entries;
}

///
/// DI_CloseFunc
//  Closes glossary window
HOOKPROTONHNONP(DI_CloseFunc, void)
{
   DI_FinishEdit();
   if (G->DI->Modified) DI_Save();
   G->Weights[4] = xget(G->DI->GUI.GR_LIST, MUIA_HorizWeight);
   G->Weights[5] = xget(G->DI->GUI.GR_TEXT, MUIA_HorizWeight);
   DisposeModulePush(&G->DI);
}
MakeStaticHook(DI_CloseHook, DI_CloseFunc);

///
/// DI_PasteFunc
//  Pastes text of selected glossary entry into the internal editors
HOOKPROTONHNONP(DI_PasteFunc, void)
{
  struct Dict *entry;
  ENTER();

  DI_FinishEdit();

  if(G->DI->writeWindow != NULL)
  {
    DoMethod(G->DI->GUI.LV_ENTRIES, MUIM_List_GetEntry, MUIV_List_GetEntry_Active, &entry);
    DoMethod(G->DI->writeWindow, MUIM_WriteWindow_InsertText, entry->Text);
  }

  DI_CloseFunc();

  LEAVE();
}
MakeStaticHook(DI_PasteHook, DI_PasteFunc);

///
/// DI_DeleteFunc
//  Removes selected entry from the glossary
HOOKPROTONHNONP(DI_DeleteFunc, void)
{
    G->DI->Modified = TRUE;
    G->DI->OldEntry = NULL;
    DoMethod(G->DI->GUI.LV_ENTRIES, MUIM_List_Remove, MUIV_List_Remove_Active);
}
MakeStaticHook(DI_DeleteHook, DI_DeleteFunc);

///
/// DI_DisplayFunc
//  Displays selected glossary entry
HOOKPROTONHNONP(DI_DisplayFunc, void)
{
   struct DI_GUIData *gui = &G->DI->GUI;
   struct Dict *entry;

   DI_FinishEdit();
   DoMethod(gui->LV_ENTRIES, MUIM_List_GetEntry, MUIV_List_GetEntry_Active, &entry);
   DoMethod(G->App, MUIM_MultiSet, MUIA_Disabled, !entry, gui->ST_ALIAS, gui->SL_EDIT, gui->TE_EDIT, gui->BT_DELETE, gui->BT_PASTE, NULL);
   nnset(gui->ST_ALIAS, MUIA_String_Contents, entry ? entry->Alias : "");
   nnset(gui->TE_EDIT, MUIA_TextEditor_Contents, entry ? entry->Text : "");
   G->DI->OldEntry = entry;
}
MakeStaticHook(DI_DisplayHook, DI_DisplayFunc);

///
/// DI_ModifyFunc
//  Saves changed glossary item
HOOKPROTONHNO(DI_ModifyFunc, void, int *arg)
{
  struct Dict new;

  ENTER();

  DI_FinishEdit();
  strlcpy(new.Alias, tr(MSG_NewEntry), sizeof(new.Alias));
  new.Text = dstralloc(SIZE_DEFAULT);
  DoMethod(G->DI->GUI.LV_ENTRIES, MUIM_List_InsertSingle, &new, MUIV_List_Insert_Bottom);
  nnset(G->DI->GUI.LV_ENTRIES, MUIA_List_Active, MUIV_List_Active_Bottom);

  DI_DisplayFunc();

  if(*arg == 1 && G->DI->writeWindow != NULL)
  {
    DoMethod(G->DI->writeWindow, MUIM_WriteWindow_ArexxCommand, "Copy");
    DoMethod(G->DI->GUI.TE_EDIT, MUIM_TextEditor_ARexxCmd, "Paste");
  }

  set(G->DI->GUI.WI, MUIA_Window_ActiveObject, G->DI->GUI.ST_ALIAS);

  LEAVE();
}
MakeStaticHook(DI_ModifyHook, DI_ModifyFunc);

///
/// DI_OpenFunc
//  Opens glossary window
HOOKPROTONHNO(DI_OpenFunc, void, ULONG *arg)
{
  ENTER();

  if(G->DI == NULL && (G->DI = DI_New()) != NULL)
  {
    G->DI->writeWindow = (Object *)*arg;

    if(SafeOpenWindow(G->DI->GUI.WI) == FALSE)
      DisposeModulePush(&G->DI);
    else if(DI_Load())
      set(G->DI->GUI.LV_ENTRIES, MUIA_List_Active, 0);
  }

  LEAVE();
}
MakeHook(DI_OpenHook, DI_OpenFunc);
///

/*** GUI ***/
/// DI_LV_ConFunc
//  Glossary listview construction hook
HOOKPROTONHNO(DI_LV_ConFunc, struct Dict *, struct Dict *dict)
{
   return memdup(dict, sizeof(*dict));
}
MakeStaticHook(DI_LV_ConFuncHook, DI_LV_ConFunc);

///
/// DI_LV_DesFunc
//  Glossary listview destruction hook
HOOKPROTONHNO(DI_LV_DesFunc, long, struct Dict *entry)
{
   dstrfree(entry->Text);
   free(entry);
   return 0;
}
MakeStaticHook(DI_LV_DesFuncHook, DI_LV_DesFunc);

///
/// DI_New
//  Creates glossary window
static struct DI_ClassData *DI_New(void)
{
   struct DI_ClassData *data = calloc(1, sizeof(struct DI_ClassData));

   ENTER();

   if (data)
   {
      data->GUI.SL_EDIT = ScrollbarObject, End;
      data->GUI.WI = WindowObject,
         MUIA_Window_Title, tr(MSG_WR_Dictionary),
         MUIA_Window_ScreenTitle, CreateScreenTitle(data->ScreenTitle, sizeof(data->ScreenTitle), tr(MSG_WR_Dictionary)),
         MUIA_HelpNode, "Windows#WritewindowGlossary",
         MUIA_Window_ID, MAKE_ID('D','I','C','T'),
         WindowContents, VGroup,
            Child, HGroup,
               GroupSpacing(0),
               Child, data->GUI.GR_LIST = ListviewObject,
                  MUIA_HorizWeight, G->Weights[4],
                  MUIA_Listview_DragType, MUIV_Listview_DragType_Immediate,
                  MUIA_Listview_Input, TRUE,
                  MUIA_Listview_DoubleClick, TRUE,
                  MUIA_CycleChain, 1,
                  MUIA_Listview_List, data->GUI.LV_ENTRIES = ListObject,
                    InputListFrame,
                    MUIA_List_ConstructHook, &DI_LV_ConFuncHook,
                    MUIA_List_DestructHook , &DI_LV_DesFuncHook,
                    MUIA_List_DragSortable, TRUE,
                  End,
               End,
               Child, NBalanceObject,
                  MUIA_Balance_Quiet, TRUE,
               End,
               Child, data->GUI.GR_TEXT = VGroup,
                  MUIA_HorizWeight, G->Weights[5],
                  Child, HGroup,
                     Child, Label2(tr(MSG_DI_Alias)),
                     Child, data->GUI.ST_ALIAS = MakeString(SIZE_NAME, tr(MSG_DI_Alias)),
                  End,
                  Child, HGroup,
                     MUIA_Group_Spacing, 0,
                     Child, data->GUI.TE_EDIT = MailTextEditObject,
                        InputListFrame,
                        MUIA_CycleChain, TRUE,
                        MUIA_TextEditor_Slider, data->GUI.SL_EDIT,
                     End,
                     Child, data->GUI.SL_EDIT,
                  End,
               End,
            End,
            Child, ColGroup(4),
               Child, data->GUI.BT_NEW       = MakeButton(tr(MSG_DI_New)),
               Child, data->GUI.BT_ADDSELECT = MakeButton(tr(MSG_DI_AddSelect)),
               Child, data->GUI.BT_DELETE    = MakeButton(tr(MSG_DI_Delete)),
               Child, data->GUI.BT_PASTE     = MakeButton(tr(MSG_DI_Paste)),
            End,
         End,
      End;

      if (data->GUI.WI)
      {
         DoMethod(G->App, OM_ADDMEMBER, data->GUI.WI);
         DoMethod(G->App, MUIM_MultiSet, MUIA_Disabled, TRUE, data->GUI.ST_ALIAS, data->GUI.SL_EDIT, data->GUI.TE_EDIT, data->GUI.BT_DELETE, data->GUI.BT_PASTE, NULL);
         set(data->GUI.WI, MUIA_Window_ActiveObject, data->GUI.GR_LIST);
         SetHelp(data->GUI.LV_ENTRIES,   MSG_HELP_DI_LV_ENTRIES);
         SetHelp(data->GUI.ST_ALIAS,     MSG_HELP_DI_ST_ALIAS);
         SetHelp(data->GUI.BT_NEW,       MSG_HELP_DI_BT_NEW);
         SetHelp(data->GUI.BT_ADDSELECT, MSG_HELP_DI_BT_ADDSELECT);
         SetHelp(data->GUI.BT_DELETE,    MSG_HELP_DI_BT_DELETE);
         SetHelp(data->GUI.BT_PASTE,     MSG_HELP_DI_BT_PASTE);
         DoMethod(data->GUI.ST_ALIAS    ,MUIM_Notify,MUIA_String_Contents     ,MUIV_EveryTime,data->GUI.TE_EDIT      ,3,MUIM_Set,MUIA_TextEditor_HasChanged,TRUE);
         DoMethod(data->GUI.ST_ALIAS    ,MUIM_Notify,MUIA_String_Acknowledge  ,MUIV_EveryTime,MUIV_Notify_Application,2,MUIM_CallHook,&DI_DisplayHook);
         DoMethod(data->GUI.BT_NEW      ,MUIM_Notify,MUIA_Pressed             ,FALSE         ,MUIV_Notify_Application,3,MUIM_CallHook,&DI_ModifyHook,0);
         DoMethod(data->GUI.BT_ADDSELECT,MUIM_Notify,MUIA_Pressed             ,FALSE         ,MUIV_Notify_Application,3,MUIM_CallHook,&DI_ModifyHook,1);
         DoMethod(data->GUI.BT_DELETE   ,MUIM_Notify,MUIA_Pressed             ,FALSE         ,MUIV_Notify_Application,3,MUIM_CallHook,&DI_DeleteHook,0);
         DoMethod(data->GUI.BT_PASTE    ,MUIM_Notify,MUIA_Pressed             ,FALSE         ,MUIV_Notify_Application,3,MUIM_CallHook,&DI_PasteHook,0);
         DoMethod(data->GUI.LV_ENTRIES  ,MUIM_Notify,MUIA_Listview_DoubleClick,TRUE          ,MUIV_Notify_Application,3,MUIM_CallHook,&DI_PasteHook,0);
         DoMethod(data->GUI.LV_ENTRIES  ,MUIM_Notify,MUIA_List_Active         ,MUIV_EveryTime,MUIV_Notify_Application,2,MUIM_CallHook,&DI_DisplayHook);
         DoMethod(data->GUI.WI          ,MUIM_Notify,MUIA_Window_CloseRequest ,TRUE          ,MUIV_Notify_Application,3,MUIM_CallHook,&DI_CloseHook,0);
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

