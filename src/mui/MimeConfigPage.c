/***************************************************************************

 YAM - Yet Another Mailer
 Copyright (C) 1995-2000 Marcel Beck
 Copyright (C) 2000-2022 YAM Open Source Team

 This program is free software; you can redistribute it and/or modify
 it under the terms of the GNU General Public License as published by
 the Free Software Foundation; either version 2 of the License, or
 (at your option) any later version.

 This program is distributed in the hope that it will be useful,
 but WITHOUT ANY WARRANTY; without even the implied warranty of
 MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.   See the
 GNU General Public License for more details.

 You should have received a copy of the GNU General Public License
 along with this program; if not, write to the Free Software
 Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA

 YAM Official Support Site : http://www.yam.ch
 YAM OpenSource project     : http://sourceforge.net/projects/yamos/

 $Id$

 Superclass:  MUIC_ConfigPage
 Description: "MIME" configuration page

***************************************************************************/

#include "MimeConfigPage_cl.h"

#include <ctype.h>
#include <string.h>

#include <proto/asl.h>
#include <proto/muimaster.h>
#include <mui/BetterString_mcc.h>
#include <mui/NBalance_mcc.h>
#include <mui/NList_mcc.h>
#include <mui/NListview_mcc.h>

#include "SDI_hook.h"

#include "YAM.h"
#include "YAM_error.h"

#include "mui/CodesetPopup.h"
#include "mui/ConfigPage.h"
#include "mui/ConfigPageList.h"
#include "mui/MimeTypeList.h"
#include "mui/MimeTypePopup.h"
#include "mui/PlaceholderPopup.h"
#include "mui/PlaceholderPopupList.h"

#include "Config.h"
#include "MimeTypes.h"
#include "MUIObjects.h"
#include "Requesters.h"

#include "Debug.h"

/* CLASSDATA
struct Data
{
  Object *LV_MIME;
  Object *BT_MADD;
  Object *BT_MDEL;
  Object *BT_MIMEIMPORT;
  Object *GR_MIME;
  Object *PO_CTYPE;
  Object *ST_EXTENS;
  Object *ST_DESCRIPTION;
  Object *PO_COMMAND;
  Object *PO_DEFVIEWER;
  Object *PO_DEFVIEWER_CODESET;
  Object *PO_MIME_CODESET;

  struct Hook MimeCommandReqStartHook;
  struct Hook MimeCommandReqStopHook;
  struct Hook MimeDefViewerReqStartHook;
  struct Hook MimeDefViewerReqStopHook;
};
*/

/* Overloaded Methods */
/// OVERLOAD(OM_NEW)
OVERLOAD(OM_NEW)
{
  Object *LV_MIME;
  Object *BT_MADD;
  Object *BT_MDEL;
  Object *BT_MIMEIMPORT;
  Object *GR_MIME;
  Object *PO_CTYPE;
  Object *ST_EXTENS;
  Object *ST_DESCRIPTION;
  Object *PO_COMMAND;
  Object *PO_DEFVIEWER;
  Object *PO_DEFVIEWER_CODESET;
  Object *PO_MIME_CODESET;
  Object *popAsl[2];

  ENTER();

  if((obj = DoSuperNew(cl, obj,
    MUIA_HelpNode, "Configuration#MIME",
    MUIA_ConfigPage_Page, cp_MIME,
    MUIA_ConfigPage_AddSpacer, FALSE,
    MUIA_ConfigPage_Contents, VGroup,
      Child, VGroup,
        Child, HGroup,
          GroupSpacing(0),
          Child, VGroup,
            MUIA_Weight, 30,

            Child, HBarT(tr(MSG_CO_MIMETYPE_TITLE)), End,

            Child, NListviewObject,
              MUIA_CycleChain, TRUE,
              MUIA_NListview_NList, LV_MIME = MimeTypeListObject,
              End,
            End,

            Child, HGroup,
              Child, ColGroup(2),
                MUIA_Group_Spacing, 1,
                MUIA_Group_SameWidth, TRUE,
                MUIA_Weight, 1,
                Child, BT_MADD = MakeButton(MUIX_B "+" MUIX_N),
                Child, BT_MDEL = MakeButton(MUIX_B "-" MUIX_N),
              End,
              Child, HSpace(0),
              Child, BT_MIMEIMPORT = PopButton(MUII_PopFile),
            End,
          End,

          Child, NBalanceObject,
            MUIA_Balance_Quiet, TRUE,
          End,

          Child, VGroup,
            GroupFrameT(tr(MSG_Options)),

            MUIA_Weight, 70,
            Child, GR_MIME = ColGroup(2),
              Child, Label2(tr(MSG_CO_MimeType)),
              Child, PO_CTYPE = MimeTypePopupObject,
                MUIA_MimeTypePopup_ControlChar, ShortCut(tr(MSG_CO_MimeType)),
                MUIA_MimeTypePopup_InConfigWindow, TRUE,
              End,

              Child, Label2(tr(MSG_CO_Extension)),
              Child, ST_EXTENS = BetterStringObject,
                StringFrame,
                MUIA_String_Accept,      "0123456789ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz ",
                MUIA_String_MaxLen,      SIZE_NAME,
                MUIA_ControlChar,        ShortCut(tr(MSG_CO_Extension)),
                MUIA_String_AdvanceOnCR, TRUE,
                MUIA_CycleChain,         TRUE,
              End,

              Child, Label2(tr(MSG_CO_MIME_DESCRIPTION)),
              Child, ST_DESCRIPTION = MakeString(SIZE_DEFAULT, tr(MSG_CO_MIME_DESCRIPTION)),

              Child, Label2(tr(MSG_CO_MimeCmd)),
              Child, HGroup,
                MUIA_Group_Spacing, 0,
                Child, PO_COMMAND = PlaceholderPopupObject,
                  MUIA_String_MaxLen, SIZE_COMMAND,
                  MUIA_PlaceholderPopup_Mode, PHM_MIME_COMMAND,
                  MUIA_PlaceholderPopup_ControlChar, ShortCut(tr(MSG_CO_MimeCmd)),
                End,
                Child, popAsl[0] = PopaslObject,
                  MUIA_Popasl_Type,      ASL_FileRequest,
                  MUIA_Popstring_Button, PopButton(MUII_PopFile),
                End,
              End,

              Child, Label2(tr(MSG_CO_MIME_CODESET)),
              Child, PO_MIME_CODESET = CodesetPopupObject,
                MUIA_CodesetPopup_ControlChar, tr(MSG_CO_MIME_CODESET),
              End,

            End,

            Child, VSpace(0),
          End,
        End,

        Child, RectangleObject,
          MUIA_Rectangle_HBar, TRUE,
          MUIA_FixHeight,      4,
        End,

        Child, ColGroup(2),

          Child, Label2(tr(MSG_CO_DefaultViewer)),
          Child, HGroup,
            MUIA_Group_HorizSpacing, 0,
            Child, PO_DEFVIEWER = PlaceholderPopupObject,
              MUIA_String_MaxLen, SIZE_COMMAND,
              MUIA_PlaceholderPopup_Mode, PHM_MIME_DEFVIEWER,
              MUIA_PlaceholderPopup_ControlChar, ShortCut(tr(MSG_CO_DefaultViewer)),
            End,
            Child, popAsl[1] = PopaslObject,
              MUIA_Popasl_Type,      ASL_FileRequest,
              MUIA_Popstring_Button, PopButton(MUII_PopFile),
            End,
          End,

          Child, Label2(tr(MSG_CO_DEFAULTVIEWER_CODESET)),
          Child, PO_DEFVIEWER_CODESET = CodesetPopupObject,
            MUIA_CodesetPopup_ControlChar, tr(MSG_CO_DEFAULTVIEWER_CODESET),
          End,

        End,

      End,

    End,
  TAG_MORE, inittags(msg))) != NULL)
  {
    GETDATA;

    data->LV_MIME =        LV_MIME;
    data->BT_MADD =        BT_MADD;
    data->BT_MDEL =        BT_MDEL;
    data->BT_MIMEIMPORT =  BT_MIMEIMPORT;
    data->GR_MIME =        GR_MIME;
    data->PO_CTYPE =       PO_CTYPE;
    data->ST_EXTENS =      ST_EXTENS;
    data->ST_DESCRIPTION = ST_DESCRIPTION;
    data->PO_COMMAND =     PO_COMMAND;
    data->PO_DEFVIEWER =   PO_DEFVIEWER;
    data->PO_MIME_CODESET = PO_MIME_CODESET;
    data->PO_DEFVIEWER_CODESET = PO_DEFVIEWER_CODESET;

    // these are the objects that may be accessed when closing the MIME type list
    xset(PO_CTYPE,
      MUIA_MimeTypePopup_ExtensionObject, ST_EXTENS,
      MUIA_MimeTypePopup_DescriptionObject, ST_DESCRIPTION);

    InitHook(&data->MimeCommandReqStartHook, FilereqStartHook, data->PO_COMMAND);
    InitHook(&data->MimeCommandReqStopHook, FilereqStopHook, data->PO_COMMAND);
    xset(popAsl[0],
      MUIA_Popasl_StartHook, &data->MimeCommandReqStartHook,
      MUIA_Popasl_StopHook,  &data->MimeCommandReqStopHook);

    InitHook(&data->MimeDefViewerReqStartHook, FilereqStartHook, data->PO_DEFVIEWER);
    InitHook(&data->MimeDefViewerReqStopHook, FilereqStopHook, data->PO_DEFVIEWER);
    xset(popAsl[1],
      MUIA_Popasl_StartHook, &data->MimeDefViewerReqStartHook,
      MUIA_Popasl_StopHook,  &data->MimeDefViewerReqStopHook);

    SetHelp(PO_CTYPE,             MSG_HELP_CO_ST_CTYPE);
    SetHelp(ST_EXTENS,            MSG_HELP_CO_ST_EXTENS);
    SetHelp(PO_COMMAND,           MSG_HELP_CO_ST_COMMAND);
    SetHelp(BT_MADD,              MSG_HELP_CO_BT_MADD);
    SetHelp(BT_MDEL,              MSG_HELP_CO_BT_MDEL);
    SetHelp(BT_MIMEIMPORT,        MSG_HELP_CO_BT_MIMEIMPORT);
    SetHelp(PO_DEFVIEWER,         MSG_HELP_CO_ST_DEFVIEWER);
    SetHelp(ST_DESCRIPTION,       MSG_HELP_CO_ST_DESCRIPTION);
    SetHelp(PO_DEFVIEWER_CODESET, MSG_HELP_CO_TX_DEFVIEWER_CODESET);
    SetHelp(PO_MIME_CODESET,      MSG_HELP_CO_TX_MIME_CODESET);

    DoMethod(LV_MIME,              MUIM_Notify, MUIA_NList_Active,                      MUIV_EveryTime, obj, 1, METHOD(GetMimeTypeEntry));
    DoMethod(PO_CTYPE,             MUIM_Notify, MUIA_MimeTypePopup_MimeTypeChanged,     MUIV_EveryTime, obj, 1, METHOD(PutMimeTypeEntry));
    DoMethod(ST_EXTENS,            MUIM_Notify, MUIA_String_Contents,                   MUIV_EveryTime, obj, 1, METHOD(PutMimeTypeEntry));
    DoMethod(PO_COMMAND,           MUIM_Notify, MUIA_String_Contents,                   MUIV_EveryTime, obj, 1, METHOD(PutMimeTypeEntry));
    DoMethod(ST_DESCRIPTION,       MUIM_Notify, MUIA_String_Contents,                   MUIV_EveryTime, obj, 1, METHOD(PutMimeTypeEntry));
    DoMethod(PO_DEFVIEWER,         MUIM_Notify, MUIA_String_Contents,                   MUIV_EveryTime, obj, 1, METHOD(PutMimeTypeEntry));
    DoMethod(PO_DEFVIEWER_CODESET, MUIM_Notify, MUIA_CodesetPopup_CodesetChanged,       MUIV_EveryTime, obj, 1, METHOD(PutMimeTypeEntry));
    DoMethod(PO_MIME_CODESET,      MUIM_Notify, MUIA_CodesetPopup_CodesetChanged,       MUIV_EveryTime, obj, 1, METHOD(PutMimeTypeEntry));
    DoMethod(BT_MADD,              MUIM_Notify, MUIA_Pressed,                           FALSE,          obj, 1, METHOD(AddMimeTypeEntry));
    DoMethod(BT_MDEL,              MUIM_Notify, MUIA_Pressed,                           FALSE,          obj, 1, METHOD(DeleteMimeTypeEntry));
    DoMethod(BT_MIMEIMPORT,        MUIM_Notify, MUIA_Pressed,                           FALSE,          obj, 1, METHOD(ImportMimeTypes));
  }

  RETURN((IPTR)obj);
  return (IPTR)obj;
}

///
/// OVERLOAD(MUIM_ConfigPage_ConfigToGUI)
OVERLOAD(MUIM_ConfigPage_ConfigToGUI)
{
  GETDATA;
  struct MimeTypeNode *mime;

  ENTER();

  // clear the filter list first
  set(data->LV_MIME, MUIA_NList_Quiet, TRUE);
  DoMethod(data->LV_MIME, MUIM_NList_Clear);

  // iterate through our filter list and add it to our
  // MUI List
  IterateList(&CE->mimeTypeList, struct MimeTypeNode *, mime)
    DoMethod(data->LV_MIME, MUIM_NList_InsertSingle, mime, MUIV_NList_Insert_Bottom);

  // sort the list after inserting all entries
  DoMethod(data->LV_MIME, MUIM_NList_Sort);
  set(data->LV_MIME, MUIA_NList_Quiet, FALSE);

  // make sure the first entry is selected per default and the listview
  // is able to display it (jump to it)
  DoMethod(data->LV_MIME, MUIM_NList_SetActive, MUIV_NList_Active_Top, MUIV_NList_SetActive_Jump_Center);

  setstring(data->PO_DEFVIEWER, CE->DefaultMimeViewer);
  set(data->PO_DEFVIEWER_CODESET, MUIA_CodesetPopup_Codeset, CE->DefaultMimeViewerCodesetName);

  set(data->BT_MDEL, MUIA_Disabled, IsMinListEmpty(&CE->mimeTypeList));

  RETURN(0);
  return 0;
}

///
/// OVERLOAD(MUIM_ConfigPage_GUIToConfig)
OVERLOAD(MUIM_ConfigPage_GUIToConfig)
{
  GETDATA;

  ENTER();

  // bring NList elements and Exec list elements into sync
  SortNListToExecList(data->LV_MIME, &CE->mimeTypeList);

  // retrieve the info on the default mime viewer
  GetMUIString(CE->DefaultMimeViewer, data->PO_DEFVIEWER, sizeof(CE->DefaultMimeViewer));
  strlcpy(CE->DefaultMimeViewerCodesetName, (char *)xget(data->PO_DEFVIEWER_CODESET, MUIA_CodesetPopup_Codeset), sizeof(CE->DefaultMimeViewerCodesetName));

  RETURN(0);
  return 0;
}

///
/// DECLARE(GetMimeTypeEntry)
// fills form with data from selected list entry
DECLARE(GetMimeTypeEntry)
{
  GETDATA;
  struct MimeTypeNode *mt;

  ENTER();

  DoMethod(data->LV_MIME, MUIM_NList_GetEntry, MUIV_NList_GetEntry_Active, &mt);
  if(mt != NULL)
  {
    nnset(data->PO_CTYPE, MUIA_MimeTypePopup_MimeType, mt->ContentType);
    nnset(data->ST_EXTENS, MUIA_String_Contents, mt->Extension);
    nnset(data->PO_COMMAND, MUIA_String_Contents, mt->Command);
    nnset(data->ST_DESCRIPTION, MUIA_String_Contents, mt->Description);
    nnset(data->PO_MIME_CODESET, MUIA_CodesetPopup_Codeset, mt->CodesetName);
  }

  DoMethod(obj, MUIM_MultiSet, MUIA_Disabled, mt == NULL,
    data->PO_CTYPE,
    data->ST_EXTENS,
    data->PO_COMMAND,
    data->ST_DESCRIPTION,
    data->PO_MIME_CODESET,
    NULL);

  RETURN(0);
  return 0;
}

///
/// DECLARE(PutMimeTypeEntry)
// fills form data into selected list entry
DECLARE(PutMimeTypeEntry)
{
  GETDATA;
  struct MimeTypeNode *mt;

  ENTER();

  DoMethod(data->LV_MIME, MUIM_NList_GetEntry, MUIV_NList_GetEntry_Active, &mt);
  if(mt != NULL)
  {
    strlcpy(mt->ContentType, (char *)xget(data->PO_CTYPE, MUIA_MimeTypePopup_MimeType), sizeof(mt->ContentType));
    GetMUIString(mt->Extension, data->ST_EXTENS, sizeof(mt->Extension));
    GetMUIString(mt->Command, data->PO_COMMAND, sizeof(mt->Command));
    GetMUIString(mt->Description, data->ST_DESCRIPTION, sizeof(mt->Description));
    strlcpy(mt->CodesetName, (char *)xget(data->PO_MIME_CODESET, MUIA_CodesetPopup_Codeset), sizeof(mt->CodesetName));

    DoMethod(data->LV_MIME, MUIM_NList_Redraw, MUIV_NList_Redraw_Active);
  }

  RETURN(0);
  return 0;
}

///
/// DECLARE(AddMimeTypeEntry)
// adds a new MIME type structure to the internal list
DECLARE(AddMimeTypeEntry)
{
  GETDATA;
  struct MimeTypeNode *mt;

  ENTER();

  if((mt = CreateNewMimeType()) != NULL)
  {
    // add the new mime type to our internal list of
    // user definable MIME types.
    AddTail((struct List *)&(CE->mimeTypeList), (struct Node *)mt);

    // add the new MimeType also to the config page.
    DoMethod(data->LV_MIME, MUIM_NList_InsertSingle, mt, MUIV_NList_Insert_Bottom);
    set(data->BT_MDEL, MUIA_Disabled, IsMinListEmpty(&CE->mimeTypeList));

    // make sure the new entry is the active entry and that the list
    // is also the active gadget in the window.
    set(data->LV_MIME, MUIA_NList_Active, MUIV_NList_Active_Bottom);
    set(_win(obj), MUIA_Window_ActiveObject, data->PO_CTYPE);
  }

  RETURN(0);
  return 0;
}

///
/// DECLARE(DeleteMimeTypeEntry)
// deletes an entry from the MIME type list.
DECLARE(DeleteMimeTypeEntry)
{
  GETDATA;
  int pos;

  ENTER();

  if((pos = xget(data->LV_MIME, MUIA_NList_Active)) != MUIV_NList_Active_Off)
  {
    struct MimeTypeNode *mt;

    DoMethod(data->LV_MIME, MUIM_NList_GetEntry, pos, &mt);
    if(mt != NULL)
    {
      // remove from MUI list
      DoMethod(data->LV_MIME, MUIM_NList_Remove, pos);

      // remove from internal list
      Remove((struct Node *)mt);

      // free memory.
      FreeSysObject(ASOT_NODE, mt);

      set(data->BT_MDEL, MUIA_Disabled, IsMinListEmpty(&CE->mimeTypeList));
    }
  }

  RETURN(0);
  return 0;
}

///
/// DECLARE(ImportMimeTypes)
// imports MIME viewers from a MIME.prefs file
DECLARE(ImportMimeTypes)
{
  GETDATA;
  int mode;

  ENTER();

  if((mode = MUI_Request(_app(obj), _win(obj), MUIF_NONE, tr(MSG_CO_ImportMIME), tr(MSG_CO_ImportMIMEGads), tr(MSG_CO_ImportMIMEReq))) != 0)
  {
    struct FileReqCache *frc;

    if((frc = ReqFile(ASL_ATTACH, _win(obj), tr(MSG_CO_IMPORTMIMETITLE), REQF_NONE, (mode == 1 ? "ENV:" : G->MA_MailDir), (mode == 1 ? "MIME.prefs" : (mode == 2 ? "mailcap" : "mime.types")))) != NULL)
    {
      char fname[SIZE_PATHFILE];
      FILE *fh;

      AddPath(fname, frc->drawer, frc->file, sizeof(fname));
      if((fh = fopen(fname, "r")) != NULL)
      {
        char *buf = NULL;
        size_t buflen = 0;

        setvbuf(fh, NULL, _IOFBF, SIZE_FILEBUF);

        set(data->LV_MIME, MUIA_NList_Quiet, TRUE);

        while(getline(&buf, &buflen, fh) > 0)
        {
          struct MimeTypeNode *mt = NULL;
          struct MimeTypeNode *mtNode;
          char *ctype = buf;
          const char *ext = "";
          const char *command = "";
          char *p;
          char *p2;

          if(IsStrEmpty(ctype) || isspace(*ctype))
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
            DoMethod(data->LV_MIME, MUIM_NList_InsertSingle, mt, MUIV_NList_Insert_Bottom);
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

        set(data->LV_MIME, MUIA_NList_Quiet, FALSE);
        DoMethod(data->LV_MIME, MUIM_NList_Redraw, MUIV_NList_Redraw_All);
      }
      else
        ER_NewError(tr(MSG_ER_CantOpenFile), fname);
    }
  }

  RETURN(0);
  return 0;
}

///
