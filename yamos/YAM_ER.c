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
#include <libraries/iffparse.h>
#include <proto/muimaster.h>

#include "SDI_hook.h"

#include "YAM.h"
#include "YAM_config.h"
#include "YAM_debug.h"
#include "YAM_error.h"
#include "YAM_main.h"
#include "YAM_locale.h"
#include "YAM_utilities.h"

/* local protos */
static struct ER_ClassData *ER_New(void);

/***************************************************************************
 Module: Error window
***************************************************************************/

/// ER_NewError
/*** ER_NewError - Adds a new error message and displays it ***/
void ER_NewError(char *error, char *arg1, char *arg2)
{
   static char label[SIZE_SMALL];
   char buf[SIZE_LARGE];
   struct ER_GUIData *gui;
   int i;

   // we only signal an error if we really have one,
   // otherwise calling this function with error=NULL is just
   // for showing the last errors,
   if(error) G->Error = TRUE;

   if (!G->ER)
   {
      if (!(G->ER = ER_New())) return;
      if (!SafeOpenWindow(G->ER->GUI.WI)) { DisposeModule(&G->ER); return; }
   }
   gui = &(G->ER->GUI);
   if (error)
   {
      if (++G->ER_NumErr > MAXERR)
      {
         free(G->ER_Message[0]);
         for (--G->ER_NumErr, i = 1; i < G->ER_NumErr; i++) G->ER_Message[i-1] = G->ER_Message[i];
      }
      SPrintF(buf, error, arg1, arg2); strcat(buf, "\n\n(");
      strcat(buf, DateStamp2String(NULL, C->SwatchBeat ? DSS_DATEBEAT : DSS_DATETIME));
      strcat(buf, ")");

      if(G->ER_Message[G->ER_NumErr-1] = malloc(strlen(buf)+1))
      {
        strcpy(G->ER_Message[G->ER_NumErr-1], buf);
      }
   }
   SPrintF(label, "\033c%s %%ld/%ld", GetStr(MSG_ErrorReq), G->ER_NumErr);

   SetAttrs(gui->NB_ERROR,
              MUIA_Numeric_Format, label,
              MUIA_Numeric_Min,    1,
              MUIA_Numeric_Max,    G->ER_NumErr,
              MUIA_Numeric_Value,  G->ER_NumErr,
              TAG_DONE);

   if (G->MA) set(G->MA->GUI.MI_ERRORS, MUIA_Menuitem_Enabled, TRUE);
}

///
/// ER_SelectFunc
/*** ER_SelectFunc - Displays an earlier error message ***/
HOOKPROTONHNO(ER_SelectFunc, void, int *arg)
{
   int value = *arg;
   set(G->ER->GUI.BT_NEXT, MUIA_Disabled, value == G->ER_NumErr);
   set(G->ER->GUI.BT_PREV, MUIA_Disabled, value == 1);
   set(G->ER->GUI.LV_ERROR, MUIA_Floattext_Text, G->ER_Message[value-1]);
}
MakeStaticHook(ER_SelectHook, ER_SelectFunc);

///
/// ER_CloseFunc
/*** ER_CloseFunc - Closes error window ***/
HOOKPROTONHNO(ER_CloseFunc, void, int *arg)
{
   set(G->ER->GUI.WI, MUIA_Window_Open, FALSE);
   if (*arg)
   {
      while (G->ER_NumErr) free(G->ER_Message[--G->ER_NumErr]);
      if (G->MA) set(G->MA->GUI.MI_ERRORS, MUIA_Menuitem_Enabled, FALSE);
   }
   DisposeModulePush(&G->ER);
}
MakeStaticHook(ER_CloseHook, ER_CloseFunc);

///

/// ER_New
/*** ER_New - Creates error window ***/
static struct ER_ClassData *ER_New(void)
{
   struct ER_ClassData *data = calloc(1, sizeof(struct ER_ClassData));
   if (data)
   {
      APTR bt_close, bt_clear;
      data->GUI.WI = WindowObject,
         MUIA_Window_Title, GetStr(MSG_ER_ErrorMessages),
         MUIA_Window_ID, MAKE_ID('E','R','R','O'),
         WindowContents, VGroup,
            Child, HGroup,
               Child, data->GUI.BT_PREV = MakeButton(GetStr(MSG_ER_PrevError)),
               Child, data->GUI.NB_ERROR = NumericbuttonObject,
                  MUIA_Numeric_Min,   0,
                  MUIA_Numeric_Value, 0,
                  MUIA_Numeric_Format, "Error %%ld/%ld",
                  MUIA_CycleChain, TRUE,
               End,
               Child, data->GUI.BT_NEXT = MakeButton(GetStr(MSG_ER_NextError)),
            End,
            Child, ListviewObject,
               MUIA_Listview_Input, FALSE,
               MUIA_CycleChain, 1,
               MUIA_Listview_List, data->GUI.LV_ERROR = FloattextObject,
                  ReadListFrame,
               End,
            End,
            Child, ColGroup(2),
               Child, bt_clear = MakeButton(GetStr(MSG_ER_Clear)),
               Child, bt_close = MakeButton(GetStr(MSG_ER_Close)),
            End,
         End,
      End;
      if (data->GUI.WI)
      {
         DoMethod(G->App, OM_ADDMEMBER, data->GUI.WI);
         DoMethod(data->GUI.BT_PREV ,MUIM_Notify,MUIA_Pressed            ,FALSE         ,data->GUI.NB_ERROR     ,2,MUIM_Numeric_Decrease,1);
         DoMethod(data->GUI.BT_NEXT ,MUIM_Notify,MUIA_Pressed            ,FALSE         ,data->GUI.NB_ERROR     ,2,MUIM_Numeric_Increase,1);
         DoMethod(data->GUI.NB_ERROR,MUIM_Notify,MUIA_Numeric_Value      ,MUIV_EveryTime,MUIV_Notify_Application,3,MUIM_CallHook,&ER_SelectHook,MUIV_TriggerValue);
         DoMethod(bt_clear          ,MUIM_Notify,MUIA_Pressed            ,FALSE         ,MUIV_Notify_Application,3,MUIM_CallHook,&ER_CloseHook,TRUE);
         DoMethod(bt_close          ,MUIM_Notify,MUIA_Pressed            ,FALSE         ,MUIV_Notify_Application,3,MUIM_CallHook,&ER_CloseHook,FALSE);
         DoMethod(data->GUI.WI      ,MUIM_Notify,MUIA_Window_CloseRequest,TRUE          ,MUIV_Notify_Application,3,MUIM_CallHook,&ER_CloseHook,FALSE);
         return data;
      }
      free(data);
   }
   return NULL;
}
///
