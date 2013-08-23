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
 Description: "Mixed" configuration page

***************************************************************************/

#include "MixedConfigPage_cl.h"

#include <proto/asl.h>
#include <proto/muimaster.h>
#include <proto/wb.h>
#include <proto/xpkmaster.h>
#include <mui/BetterString_mcc.h>

#include "SDI_hook.h"

#include "YAM.h"
#include "YAM_config.h"

#include "mui/ConfigPage.h"
#include "mui/ConfigPageList.h"
#include "mui/PlaceholderPopupList.h"

#include "Debug.h"

/* CLASSDATA
struct Data
{
  Object *ST_TEMPDIR;
  Object *ST_DETACHDIR;
  Object *ST_ATTACHDIR;
  Object *ST_UPDATEDOWNLOADPATH;
  Object *ST_EDITOR;
  Object *CH_DEFCODESET_EDITOR;
  Object *TX_DEFCODESET_EDITOR;
  Object *CH_WBAPPICON;
  Object *ST_APPICON;
  Object *CH_APPICONPOS;
  Object *ST_APPX;
  Object *ST_APPY;
  Object *BT_APPICONGETPOS;
  Object *CH_CLGADGET;
  Object *CH_CONFIRM;
  Object *NB_CONFIRMDEL;
  Object *CH_REMOVE;
  Object *TX_PACKER;
  Object *NB_PACKER;
  Object *TX_ENCPACK;
  Object *NB_ENCPACK;
  Object *ST_ARCHIVER;
  Object *CH_ARCHIVERPROGRESS;
  Object *CY_TRANSWIN;
  #if defined(__amigaos4__)
  Object *CH_DOCKYICON;
  #endif
};
*/

/* Private functions */
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
/// MakeXPKPop
// creates a popup list of available XPK sublibraries
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

/* Overloaded Methods */
/// OVERLOAD(OM_NEW)
OVERLOAD(OM_NEW)
{
  static const char *trwopt[4];
  Object *ST_TEMPDIR;
  Object *ST_DETACHDIR;
  Object *ST_ATTACHDIR;
  Object *ST_UPDATEDOWNLOADPATH;
  Object *ST_EDITOR;
  Object *CH_DEFCODESET_EDITOR;
  Object *TX_DEFCODESET_EDITOR;
  Object *CH_WBAPPICON;
  Object *ST_APPICON;
  Object *CH_APPICONPOS;
  Object *ST_APPX;
  Object *ST_APPY;
  Object *BT_APPICONGETPOS;
  Object *CH_CLGADGET;
  Object *CH_CONFIRM;
  Object *NB_CONFIRMDEL;
  Object *CH_REMOVE;
  Object *TX_PACKER;
  Object *NB_PACKER;
  Object *TX_ENCPACK;
  Object *NB_ENCPACK;
  Object *ST_ARCHIVER;
  Object *CH_ARCHIVERPROGRESS;
  Object *CY_TRANSWIN;
  #if defined(__amigaos4__)
  Object *CH_DOCKYICON;
  #endif
  Object *popButton;
  Object *codesetPopButton;

  ENTER();

  trwopt[TWM_HIDE] = tr(MSG_CO_TWNever);
  trwopt[TWM_AUTO] = tr(MSG_CO_TWAuto);
  trwopt[TWM_SHOW] = tr(MSG_CO_TWAlways);
  trwopt[3] = NULL;

  if((obj = DoSuperNew(cl, obj,
    MUIA_HelpNode, "Configuration#Miscellaneous",
    MUIA_ConfigPage_Page, cp_Mixed,
    MUIA_ConfigPage_Contents, VGroup,
      Child, ColGroup(2), GroupFrameT(tr(MSG_CO_Paths)),
        Child, Label2(tr(MSG_CO_TempDir)),
        Child, PopaslObject,
          MUIA_Popasl_Type,      ASL_FileRequest,
          MUIA_Popstring_String, ST_TEMPDIR = MakeString(SIZE_PATH, tr(MSG_CO_TempDir)),
          MUIA_Popstring_Button, PopButton(MUII_PopDrawer),
          ASLFR_DrawersOnly, TRUE,
        End,

        Child, Label2(tr(MSG_CO_Detach)),
        Child, PopaslObject,
          MUIA_Popasl_Type,      ASL_FileRequest,
          MUIA_Popstring_String, ST_DETACHDIR = MakeString(SIZE_PATH, tr(MSG_CO_Detach)),
          MUIA_Popstring_Button, PopButton(MUII_PopDrawer),
          ASLFR_DrawersOnly, TRUE,
        End,

        Child, Label2(tr(MSG_CO_Attach)),
        Child, PopaslObject,
          MUIA_Popasl_Type,      ASL_FileRequest,
          MUIA_Popstring_String, ST_ATTACHDIR = MakeString(SIZE_PATH, tr(MSG_CO_Attach)),
          MUIA_Popstring_Button, PopButton(MUII_PopDrawer),
          ASLFR_DrawersOnly, TRUE,
        End,

        Child, Label2(tr(MSG_CO_UPDATE_DOWNLOAD_PATH)),
        Child, PopaslObject,
          MUIA_Popasl_Type,      ASL_FileRequest,
          MUIA_Popstring_String, ST_UPDATEDOWNLOADPATH = MakeString(SIZE_PATH, tr(MSG_CO_UPDATE_DOWNLOAD_PATH)),
          MUIA_Popstring_Button, PopButton(MUII_PopDrawer),
          ASLFR_DrawersOnly, TRUE,
        End,

      End,

      Child, VGroup, GroupFrameT(tr(MSG_CO_EXTEDITOR)),
        Child, ColGroup(2),
          Child, Label2(tr(MSG_CO_ExternalEditor)),
          Child, PopaslObject,
            MUIA_Popasl_Type,      ASL_FileRequest,
            MUIA_Popstring_String, ST_EDITOR = MakeString(SIZE_PATHFILE, tr(MSG_CO_ExternalEditor)),
            MUIA_Popstring_Button, PopButton(MUII_PopFile),
          End,

          Child, HSpace(1),
          Child, HGroup,
            Child, HGroup,
              Child, CH_DEFCODESET_EDITOR = MakeCheck(tr(MSG_CO_EXTEDITOR_CODESET)),
              Child, Label1(tr(MSG_CO_EXTEDITOR_CODESET)),
            End,
            Child, MakeCodesetPop(&TX_DEFCODESET_EDITOR, &codesetPopButton),
          End,

        End,
      End,

      Child, VGroup, GroupFrameT(tr(MSG_CO_AppIcon)),
        Child, ColGroup(2),
          Child, CH_WBAPPICON = MakeCheck(tr(MSG_CO_WBAPPICON)),
          Child, LLabel1(tr(MSG_CO_WBAPPICON)),

          Child, HSpace(0),
          Child, ColGroup(2),

            Child, Label2(tr(MSG_CO_APPICONTEXT)),
            Child, MakeVarPop(&ST_APPICON, &popButton, PHM_MAILSTATS, SIZE_DEFAULT/2, tr(MSG_CO_APPICONTEXT)),

            Child, HGroup,
              Child, CH_APPICONPOS = MakeCheck(tr(MSG_CO_PositionX)),
              Child, Label2(tr(MSG_CO_PositionX)),
            End,
            Child, HGroup,
              Child, ST_APPX = BetterStringObject,
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
                Child, ST_APPY = BetterStringObject,
                  StringFrame,
                  MUIA_CycleChain,          TRUE,
                  MUIA_ControlChar,         ShortCut("_Y"),
                  MUIA_FixWidthTxt,         "0000",
                  MUIA_String_MaxLen,       4+1,
                  MUIA_String_AdvanceOnCR,  TRUE,
                  MUIA_String_Integer,      0,
                  MUIA_String_Accept,       "0123456789",
                End,
                Child, BT_APPICONGETPOS = PopButton(MUII_PopUp),
              End,
              Child, HSpace(0),
            End,

          End,
        End,
        #if defined(__amigaos4__)
        Child, MakeCheckGroup(&CH_DOCKYICON, tr(MSG_CO_DOCKYICON)),
        #endif
        Child, MakeCheckGroup(&CH_CLGADGET, tr(MSG_CO_CloseGadget)),
      End,

      Child, VGroup, GroupFrameT(tr(MSG_CO_SaveDelete)),
        Child, HGroup,
          Child, CH_CONFIRM = MakeCheck(tr(MSG_CO_ConfirmDelPart1)),
          Child, Label2(tr(MSG_CO_ConfirmDelPart1)),
          Child, NB_CONFIRMDEL = MakeNumeric(1, 50, FALSE),
          Child, Label2(tr(MSG_CO_ConfirmDelPart2)),
          Child, HSpace(0),
        End,
        Child, MakeCheckGroup(&CH_REMOVE, tr(MSG_CO_Remove)),
      End,
      Child, HGroup, GroupFrameT(tr(MSG_CO_XPK)),
        Child, ColGroup(2),
          Child, Label1(tr(MSG_CO_XPKPack)),
          Child, HGroup,
            Child, MakeXPKPop(&TX_PACKER, FALSE),
            Child, NB_PACKER = MakeNumeric(0, 100, TRUE),
            Child, HSpace(0),
          End,

          Child, Label1(tr(MSG_CO_XPKPackEnc)),
          Child, HGroup,
            Child, MakeXPKPop(&TX_ENCPACK, TRUE),
            Child, NB_ENCPACK = MakeNumeric(0, 100, TRUE),
            Child, HSpace(0),
          End,

          Child, Label1(tr(MSG_CO_Archiver)),
          Child, HGroup,
            Child, MakeVarPop(&ST_ARCHIVER, &popButton, PHM_ARCHIVE, SIZE_COMMAND, tr(MSG_CO_Archiver)),
            Child, MakeCheckGroup(&CH_ARCHIVERPROGRESS, tr(MSG_CO_SHOW_ARCHIVER_PROGRESS)),
          End,
        End,
      End,
      Child, ColGroup(2), GroupFrameT(tr(MSG_CO_MIXED_CONNECTIONS)),
        Child, Label(tr(MSG_CO_TransferWin)),
        Child, CY_TRANSWIN = MakeCycle(trwopt, tr(MSG_CO_TransferWin)),
      End,
    End,
  TAG_MORE, inittags(msg))) != NULL)
  {
    GETDATA;

    data->ST_TEMPDIR =            ST_TEMPDIR;
    data->ST_DETACHDIR =          ST_DETACHDIR;
    data->ST_ATTACHDIR =          ST_ATTACHDIR;
    data->ST_UPDATEDOWNLOADPATH = ST_UPDATEDOWNLOADPATH;
    data->ST_EDITOR =             ST_EDITOR;
    data->CH_DEFCODESET_EDITOR =  CH_DEFCODESET_EDITOR;
    data->TX_DEFCODESET_EDITOR =  TX_DEFCODESET_EDITOR;
    data->CH_WBAPPICON =          CH_WBAPPICON;
    data->ST_APPICON =            ST_APPICON;
    data->CH_APPICONPOS =         CH_APPICONPOS;
    data->ST_APPX =               ST_APPX;
    data->ST_APPY =               ST_APPY;
    data->BT_APPICONGETPOS =      BT_APPICONGETPOS;
    data->CH_CLGADGET =           CH_CLGADGET;
    data->CH_CONFIRM =            CH_CONFIRM;
    data->NB_CONFIRMDEL =         NB_CONFIRMDEL;
    data->CH_REMOVE =             CH_REMOVE;
    data->TX_PACKER =             TX_PACKER;
    data->NB_PACKER =             NB_PACKER;
    data->TX_ENCPACK =            TX_ENCPACK;
    data->NB_ENCPACK =            NB_ENCPACK;
    data->ST_ARCHIVER =           ST_ARCHIVER;
    data->CH_ARCHIVERPROGRESS =   CH_ARCHIVERPROGRESS;
    data->CY_TRANSWIN =           CY_TRANSWIN;
    #if defined(__amigaos4__)
    data->CH_DOCKYICON =          CH_DOCKYICON;
    #endif // __amigaos4__

    SetHelp(ST_TEMPDIR,           MSG_HELP_CO_ST_TEMPDIR);
    SetHelp(ST_DETACHDIR,         MSG_HELP_CO_ST_DETACHDIR);
    SetHelp(ST_ATTACHDIR,         MSG_HELP_CO_ST_ATTACHDIR);
    SetHelp(CH_WBAPPICON,         MSG_HELP_CO_CH_WBAPPICON);
    SetHelp(ST_APPX,              MSG_HELP_CO_ST_APP);
    SetHelp(ST_APPY,              MSG_HELP_CO_ST_APP);
    SetHelp(CH_APPICONPOS,        MSG_HELP_CO_ST_APP);
    #if defined(__amigaos4__)
    SetHelp(CH_DOCKYICON,         MSG_HELP_CO_CH_DOCKYICON);
    #endif // __amigaos4__
    SetHelp(CH_CLGADGET,          MSG_HELP_CO_CH_CLGADGET);
    SetHelp(CH_CONFIRM,           MSG_HELP_CO_CH_CONFIRM);
    SetHelp(NB_CONFIRMDEL,        MSG_HELP_CO_NB_CONFIRMDEL);
    SetHelp(CH_REMOVE,            MSG_HELP_CO_CH_REMOVE);
    SetHelp(TX_ENCPACK,           MSG_HELP_CO_TX_ENCPACK);
    SetHelp(TX_PACKER,            MSG_HELP_CO_TX_PACKER);
    SetHelp(NB_ENCPACK,           MSG_HELP_CO_NB_ENCPACK);
    SetHelp(NB_PACKER,            MSG_HELP_CO_NB_ENCPACK);
    SetHelp(ST_ARCHIVER,          MSG_HELP_CO_ST_ARCHIVER);
    SetHelp(ST_APPICON,           MSG_HELP_CO_ST_APPICON);
    SetHelp(BT_APPICONGETPOS,     MSG_HELP_CO_BT_APPICONGETPOS);
    SetHelp(CY_TRANSWIN,          MSG_HELP_CO_CH_TRANSWIN);
    SetHelp(ST_EDITOR,            MSG_HELP_CO_ST_EDITOR_EXT);
    SetHelp(CH_DEFCODESET_EDITOR, MSG_HELP_CO_CH_DEFCODESET_EDITOR);
    SetHelp(TX_DEFCODESET_EDITOR, MSG_HELP_CO_TX_DEFCODESET_EDITOR);

    DoMethod(obj, MUIM_MultiSet, MUIA_Disabled, TRUE,
      ST_APPX,
      ST_APPY,
      ST_APPICON,
      BT_APPICONGETPOS,
      TX_DEFCODESET_EDITOR,
      codesetPopButton,
      NULL);
    DoMethod(CH_WBAPPICON,         MUIM_Notify, MUIA_Selected, MUIV_EveryTime, obj,                  9, MUIM_MultiSet, MUIA_Disabled, MUIV_NotTriggerValue, ST_APPX, ST_APPY, ST_APPICON, CH_APPICONPOS, BT_APPICONGETPOS, NULL);
    DoMethod(BT_APPICONGETPOS,     MUIM_Notify, MUIA_Pressed,  FALSE,          obj,                  1, METHOD(GetAppIconPos));
    DoMethod(CH_CONFIRM,           MUIM_Notify, MUIA_Selected, MUIV_EveryTime, NB_CONFIRMDEL,        3, MUIM_Set, MUIA_Disabled, MUIV_NotTriggerValue);
    DoMethod(CH_DEFCODESET_EDITOR, MUIM_Notify, MUIA_Selected, MUIV_EveryTime, TX_DEFCODESET_EDITOR, 3, MUIM_Set, MUIA_Disabled, MUIV_NotTriggerValue);
    DoMethod(CH_DEFCODESET_EDITOR, MUIM_Notify, MUIA_Selected, MUIV_EveryTime, codesetPopButton,     3, MUIM_Set, MUIA_Disabled, MUIV_NotTriggerValue);

    #if defined(__amigaos4__)
    set(CH_DOCKYICON, MUIA_Disabled, G->applicationID == 0);
    #endif // __amigaos4__

    // disable the XPK popups if xpkmaster.library is not available
    if(XpkBase == NULL)
    {
      set(NB_PACKER, MUIA_Disabled, TRUE);
      set(NB_ENCPACK, MUIA_Disabled, TRUE);
    }
  }

  RETURN((IPTR)obj);
  return (IPTR)obj;
}

///
/// OVERLOAD(MUIM_ConfigPage_ConfigToGUI)
OVERLOAD(MUIM_ConfigPage_ConfigToGUI)
{
  GETDATA;

  ENTER();

  setstring(data->ST_TEMPDIR, CE->TempDir);
  setstring(data->ST_DETACHDIR, CE->DetachDir);
  setstring(data->ST_ATTACHDIR, CE->AttachDir);
  setstring(data->ST_UPDATEDOWNLOADPATH, CE->UpdateDownloadPath);
  setcheckmark(data->CH_WBAPPICON, CE->WBAppIcon);
  set(data->ST_APPX, MUIA_String_Integer, abs(CE->IconPositionX));
  set(data->ST_APPY, MUIA_String_Integer, abs(CE->IconPositionY));
  setcheckmark(data->CH_APPICONPOS, CE->IconPositionX >= 0 && CE->IconPositionY >= 0);
  setstring(data->ST_APPICON, CE->AppIconText);
  #if defined(__amigaos4__)
  setcheckmark(data->CH_DOCKYICON, CE->DockyIcon);
  #endif // __amigaos4__
  setcheckmark(data->CH_CLGADGET, CE->IconifyOnQuit);
  setcheckmark(data->CH_CONFIRM, CE->Confirm);

  xset(data->NB_CONFIRMDEL, MUIA_Numeric_Value, CE->ConfirmDelete,
						   MUIA_Disabled, CE->Confirm == FALSE);

  setcheckmark(data->CH_REMOVE, CE->RemoveAtOnce);
  set(data->TX_PACKER, MUIA_Text_Contents, CE->XPKPack);
  set(data->TX_ENCPACK, MUIA_Text_Contents, CE->XPKPackEncrypt);
  setslider(data->NB_PACKER, CE->XPKPackEff);
  setslider(data->NB_ENCPACK, CE->XPKPackEncryptEff);
  setstring(data->ST_ARCHIVER, CE->PackerCommand);
  setcheckmark(data->CH_ARCHIVERPROGRESS, CE->ShowPackerProgress);

  set(data->CH_APPICONPOS, MUIA_Disabled, CE->WBAppIcon == FALSE);
  setcycle(data->CY_TRANSWIN, CE->TransferWindow);
  setstring(data->ST_EDITOR, CE->Editor);
  setcheckmark(data->CH_DEFCODESET_EDITOR, CE->ForceEditorCodeset);
  nnset(data->TX_DEFCODESET_EDITOR, MUIA_Text_Contents, CE->ForcedEditorCodeset);

  RETURN(0);
  return 0;
}

///
/// OVERLOAD(MUIM_ConfigPage_GUIToConfig)
OVERLOAD(MUIM_ConfigPage_GUIToConfig)
{
  GETDATA;

  ENTER();

  GetMUIString(CE->TempDir, data->ST_TEMPDIR, sizeof(CE->TempDir));
  GetMUIString(CE->DetachDir, data->ST_DETACHDIR, sizeof(CE->DetachDir));
  GetMUIString(CE->AttachDir, data->ST_ATTACHDIR, sizeof(CE->AttachDir));
  GetMUIString(CE->UpdateDownloadPath, data->ST_UPDATEDOWNLOADPATH, sizeof(CE->UpdateDownloadPath));
  CE->WBAppIcon         = GetMUICheck(data->CH_WBAPPICON);
  CE->IconPositionX     = GetMUIInteger(data->ST_APPX);
  CE->IconPositionY     = GetMUIInteger(data->ST_APPY);

  if(GetMUICheck(data->CH_APPICONPOS) == FALSE)
  {
	CE->IconPositionX = -CE->IconPositionX;
	CE->IconPositionY = -CE->IconPositionY;
  }

  GetMUIString(CE->AppIconText, data->ST_APPICON, sizeof(CE->AppIconText));
  #if defined(__amigaos4__)
  CE->DockyIcon         = GetMUICheck(data->CH_DOCKYICON);
  #endif // __amigaos4__
  CE->IconifyOnQuit     = GetMUICheck(data->CH_CLGADGET);
  CE->Confirm           = GetMUICheck(data->CH_CONFIRM);
  CE->ConfirmDelete     = GetMUINumer(data->NB_CONFIRMDEL);
  CE->RemoveAtOnce      = GetMUICheck(data->CH_REMOVE);
  GetMUIText(CE->XPKPack, data->TX_PACKER, sizeof(CE->XPKPack));
  GetMUIText(CE->XPKPackEncrypt, data->TX_ENCPACK, sizeof(CE->XPKPackEncrypt));
  CE->XPKPackEff        = GetMUINumer(data->NB_PACKER);
  CE->XPKPackEncryptEff = GetMUINumer(data->NB_ENCPACK);
  GetMUIString(CE->PackerCommand, data->ST_ARCHIVER, sizeof(CE->PackerCommand));
  CE->ShowPackerProgress = GetMUICheck(data->CH_ARCHIVERPROGRESS);
  CE->TransferWindow = GetMUICycle(data->CY_TRANSWIN);
  GetMUIString(CE->Editor, data->ST_EDITOR, sizeof(CE->Editor));
  CE->ForceEditorCodeset = GetMUICheck(data->CH_DEFCODESET_EDITOR);
  GetMUIText(CE->ForcedEditorCodeset, data->TX_DEFCODESET_EDITOR, sizeof(CE->ForcedEditorCodeset));

  RETURN(0);
  return 0;
}

///
/// DECLARE(GetAppIconPos)
// Retrieves the position x/y of the AppIcon and
// sets the position label accordingly
DECLARE(GetAppIconPos)
{
  GETDATA;
  struct DiskObject *dobj;

  ENTER();

  if((dobj = G->theme.icons[G->currentAppIcon]) != NULL)
  {
    // set the position
    set(data->ST_APPX, MUIA_String_Integer, dobj->do_CurrentX);
    set(data->ST_APPY, MUIA_String_Integer, dobj->do_CurrentY);

    // enable the checkbox
    setcheckmark(data->CH_APPICONPOS, TRUE);
  }

  RETURN(0);
  return 0;
}

///
