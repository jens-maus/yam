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
#include "YAM_config.h"
#include "YAM_error.h"

#include "mui/ConfigPage.h"
#include "mui/ConfigPageList.h"
#include "mui/MimeTypeList.h"
#include "mui/PlaceholderPopupList.h"

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
  Object *ST_CTYPE;
  Object *ST_EXTENS;
  Object *ST_DESCRIPTION;
  Object *ST_COMMAND;
  Object *ST_DEFVIEWER;
  Object *TX_DEFVIEWER_CODESET;
  Object *TX_MIME_CODESET;

  struct Hook MimeTypeListOpenHook;
  struct Hook MimeTypeListCloseHook;
  struct Hook MimeCommandReqStartHook;
  struct Hook MimeCommandReqStopHook;
  struct Hook MimeDefViewerReqStartHook;
  struct Hook MimeDefViewerReqStopHook;

  struct MimeTypeCloseObjects closeObjs;
};
*/

/* INCLUDE
#include "MUIObjects.h" // for struct MimeTypeCloseObjects
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
  Object *ST_CTYPE;
  Object *ST_EXTENS;
  Object *ST_DESCRIPTION;
  Object *ST_COMMAND;
  Object *ST_DEFVIEWER;
  Object *TX_DEFVIEWER_CODESET;
  Object *TX_MIME_CODESET;
  Object *popButton;
  Object *list;
  Object *popMime;
  Object *popAsl[2];
  Object *mimeCodesetPopButton;
  Object *defaultCodesetPopButton;

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
              Child, popMime = MakeMimeTypePop(&ST_CTYPE, tr(MSG_CO_MimeType)),

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
                MUIA_Group_HorizSpacing, 0,
                Child, MakeVarPop(&ST_COMMAND, &popButton, &list, PHM_MIME_COMMAND, SIZE_COMMAND, tr(MSG_CO_MimeCmd)),
                Child, popAsl[0] = PopaslObject,
                  MUIA_Popasl_Type,      ASL_FileRequest,
                  MUIA_Popstring_Button, PopButton(MUII_PopFile),
                End,
              End,

              Child, Label2(tr(MSG_CO_MIME_CODESET)),
              Child, MakeCodesetPop(&TX_MIME_CODESET, &mimeCodesetPopButton),

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
            Child, MakeVarPop(&ST_DEFVIEWER, &popButton, &list, PHM_MIME_DEFVIEWER, SIZE_COMMAND, tr(MSG_CO_DefaultViewer)),
            Child, popAsl[1] = PopaslObject,
              MUIA_Popasl_Type,      ASL_FileRequest,
              MUIA_Popstring_Button, PopButton(MUII_PopFile),
            End,
          End,

          Child, Label2(tr(MSG_CO_DEFAULTVIEWER_CODESET)),
          Child, MakeCodesetPop(&TX_DEFVIEWER_CODESET, &defaultCodesetPopButton),

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
    data->ST_CTYPE =       ST_CTYPE;
    data->ST_EXTENS =      ST_EXTENS;
    data->ST_DESCRIPTION = ST_DESCRIPTION;
    data->ST_COMMAND =     ST_COMMAND;
    data->ST_DEFVIEWER =   ST_DEFVIEWER;
    data->TX_MIME_CODESET = TX_MIME_CODESET;
    data->TX_DEFVIEWER_CODESET = TX_DEFVIEWER_CODESET;

    // these are the objects that may be accessed when closing the MIME type list
    data->closeObjs.extension = ST_EXTENS;
    data->closeObjs.description = ST_DESCRIPTION;

    // hook->h_Data=TRUE tells the hook that the string object does belong to the config window
    // I know, this is a quite ugly hack, but unfortunately MUI does not
    // offer methods for this purpose which could do this stuff in a much
    // more sophisticated way :(
    InitHook(&data->MimeTypeListOpenHook, PO_MimeTypeListOpenHook, TRUE);
    InitHook(&data->MimeTypeListCloseHook, PO_MimeTypeListCloseHook, &data->closeObjs);
    xset(popMime,
      MUIA_Popobject_StrObjHook, &data->MimeTypeListOpenHook,
      MUIA_Popobject_ObjStrHook, &data->MimeTypeListCloseHook);

    InitHook(&data->MimeCommandReqStartHook, FilereqStartHook, data->ST_COMMAND);
    InitHook(&data->MimeCommandReqStopHook, FilereqStopHook, data->ST_COMMAND);
    xset(popAsl[0],
      MUIA_Popasl_StartHook, &data->MimeCommandReqStartHook,
      MUIA_Popasl_StopHook,  &data->MimeCommandReqStopHook);

    InitHook(&data->MimeDefViewerReqStartHook, FilereqStartHook, data->ST_DEFVIEWER);
    InitHook(&data->MimeDefViewerReqStopHook, FilereqStopHook, data->ST_DEFVIEWER);
    xset(popAsl[1],
      MUIA_Popasl_StartHook, &data->MimeDefViewerReqStartHook,
      MUIA_Popasl_StopHook,  &data->MimeDefViewerReqStopHook);

    SetHelp(ST_CTYPE,       MSG_HELP_CO_ST_CTYPE);
    SetHelp(ST_EXTENS,      MSG_HELP_CO_ST_EXTENS);
    SetHelp(ST_COMMAND,     MSG_HELP_CO_ST_COMMAND);
    SetHelp(BT_MADD,        MSG_HELP_CO_BT_MADD);
    SetHelp(BT_MDEL,        MSG_HELP_CO_BT_MDEL);
    SetHelp(BT_MIMEIMPORT,  MSG_HELP_CO_BT_MIMEIMPORT);
    SetHelp(ST_DEFVIEWER,   MSG_HELP_CO_ST_DEFVIEWER);
    SetHelp(ST_DESCRIPTION, MSG_HELP_CO_ST_DESCRIPTION);
    SetHelp(TX_DEFVIEWER_CODESET, MSG_HELP_CO_TX_DEFVIEWER_CODESET);
    SetHelp(TX_MIME_CODESET, MSG_HELP_CO_TX_MIME_CODESET);

    DoMethod(obj, MUIM_MultiSet, MUIA_Disabled, TRUE,
      GR_MIME,
      BT_MDEL,
      NULL);

    DoMethod(LV_MIME,        MUIM_Notify, MUIA_NList_Active,    MUIV_EveryTime, obj, 1, METHOD(GetMimeTypeEntry));
    DoMethod(ST_CTYPE,       MUIM_Notify, MUIA_String_Contents, MUIV_EveryTime, obj, 1, METHOD(PutMimeTypeEntry));
    DoMethod(ST_EXTENS,      MUIM_Notify, MUIA_String_Contents, MUIV_EveryTime, obj, 1, METHOD(PutMimeTypeEntry));
    DoMethod(ST_COMMAND,     MUIM_Notify, MUIA_String_Contents, MUIV_EveryTime, obj, 1, METHOD(PutMimeTypeEntry));
    DoMethod(ST_DESCRIPTION, MUIM_Notify, MUIA_String_Contents, MUIV_EveryTime, obj, 1, METHOD(PutMimeTypeEntry));
    DoMethod(ST_DEFVIEWER,   MUIM_Notify, MUIA_String_Contents, MUIV_EveryTime, obj, 1, METHOD(PutMimeTypeEntry));
    DoMethod(TX_DEFVIEWER_CODESET, MUIM_Notify, MUIA_Text_Contents, MUIV_EveryTime, obj, 1, METHOD(PutMimeTypeEntry));
    DoMethod(TX_MIME_CODESET,MUIM_Notify, MUIA_Text_Contents,   MUIV_EveryTime, obj, 1, METHOD(PutMimeTypeEntry));
    DoMethod(BT_MADD,        MUIM_Notify, MUIA_Pressed,         FALSE,          obj, 1, METHOD(AddMimeTypeEntry));
    DoMethod(BT_MDEL,        MUIM_Notify, MUIA_Pressed,         FALSE,          obj, 1, METHOD(DeleteMimeTypeEntry));
    DoMethod(BT_MIMEIMPORT,  MUIM_Notify, MUIA_Pressed,         FALSE,          obj, 1, METHOD(ImportMimeTypes));
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
  set(data->LV_MIME, MUIA_NList_Active, MUIV_NList_Active_Top);
  DoMethod(data->LV_MIME, MUIM_NList_Jump, MUIV_NList_Jump_Active);

  setstring(data->ST_DEFVIEWER, CE->DefaultMimeViewer);
  set(data->TX_DEFVIEWER_CODESET, MUIA_Text_Contents, CE->DefaultMimeViewerCodesetName);

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
  GetMUIString(CE->DefaultMimeViewer, data->ST_DEFVIEWER, sizeof(CE->DefaultMimeViewer));
  GetMUIText(CE->DefaultMimeViewerCodesetName, data->TX_DEFVIEWER_CODESET, sizeof(CE->DefaultMimeViewerCodesetName));

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
    nnset(data->ST_CTYPE, MUIA_String_Contents, mt->ContentType);
    nnset(data->ST_EXTENS, MUIA_String_Contents, mt->Extension);
    nnset(data->ST_COMMAND, MUIA_String_Contents, mt->Command);
    nnset(data->ST_DESCRIPTION, MUIA_String_Contents, mt->Description);
    nnset(data->TX_MIME_CODESET, MUIA_Text_Contents, mt->CodesetName);
  }

  set(data->GR_MIME, MUIA_Disabled, mt == NULL);
  set(data->BT_MDEL, MUIA_Disabled, mt == NULL);

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
    GetMUIString(mt->ContentType, data->ST_CTYPE, sizeof(mt->ContentType));
    GetMUIString(mt->Extension, data->ST_EXTENS, sizeof(mt->Extension));
    GetMUIString(mt->Command, data->ST_COMMAND, sizeof(mt->Command));
    GetMUIString(mt->Description, data->ST_DESCRIPTION, sizeof(mt->Description));
    GetMUIText(mt->CodesetName, data->TX_MIME_CODESET, sizeof(mt->CodesetName));

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

    // make sure the new entry is the active entry and that the list
    // is also the active gadget in the window.
    set(data->LV_MIME, MUIA_NList_Active, MUIV_NList_Active_Bottom);
    set(_win(obj), MUIA_Window_ActiveObject, data->ST_CTYPE);
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
