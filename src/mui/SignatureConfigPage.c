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
 Description: "Signature" configuration page

***************************************************************************/

#include "SignatureConfigPage_cl.h"

#include <proto/asl.h>
#include <proto/muimaster.h>

#include <mui/NBalance_mcc.h>
#include <mui/NList_mcc.h>
#include <mui/NListview_mcc.h>
#include <mui/TextEditor_mcc.h>

#include "Signature.h"

#include "mui/ConfigPage.h"
#include "mui/SignatureList.h"
#include "mui/SignatureTextEdit.h"

#include "Debug.h"

/* CLASSDATA
struct Data
{
  Object *LV_SIGNATURE;
  Object *BT_SIGADD;
  Object *BT_SIGDEL;
  Object *BT_SIGUP;
  Object *BT_SIGDOWN;
  Object *CH_SIG_ACTIVE;
  Object *ST_SIG_DESC;
  Object *GR_SIGEDIT;
  Object *TE_SIGEDIT;
  Object *BT_SIGEDIT;
  Object *BT_INSTAG;
  Object *BT_INSENV;
  Object *CH_SIG_FILE;
  Object *PO_SIG_FILE;
  Object *ST_SIG_FILE;
  Object *ST_TAGFILE;
  Object *ST_TAGSEP;
};
*/

/* Overloaded Methods */
/// OVERLOAD(OM_NEW)
OVERLOAD(OM_NEW)
{
  Object *LV_SIGNATURE;
  Object *BT_SIGADD;
  Object *BT_SIGDEL;
  Object *BT_SIGUP;
  Object *BT_SIGDOWN;
  Object *CH_SIG_ACTIVE;
  Object *ST_SIG_DESC;
  Object *GR_SIGEDIT;
  Object *TE_SIGEDIT;
  Object *BT_SIGEDIT;
  Object *BT_INSTAG;
  Object *BT_INSENV;
  Object *CH_SIG_FILE;
  Object *PO_SIG_FILE;
  Object *ST_SIG_FILE;
  Object *ST_TAGFILE;
  Object *ST_TAGSEP;
  Object *slider;

  ENTER();

  slider = ScrollbarObject, End;

  if((obj = DoSuperNew(cl, obj,
    MUIA_HelpNode, "Configuration#Signature",
    MUIA_ConfigPage_Page, cp_Signature,
    MUIA_ConfigPage_Contents, VGroup,
      Child, VGroup,
        Child, HGroup,
          GroupSpacing(0),
          Child, VGroup,
            MUIA_HorizWeight, 30,

            Child, HBarT(tr(MSG_CO_SIGNATURES)), End,

            Child, NListviewObject,
              MUIA_CycleChain, TRUE,
              MUIA_Weight, 60,
              MUIA_NListview_NList, LV_SIGNATURE = SignatureListObject,
              End,
            End,

            Child, HGroup,
              Child, ColGroup(2),
                GroupSpacing(1),
                MUIA_Group_SameWidth, TRUE,
                MUIA_Weight, 1,
                Child, BT_SIGADD = MakeButton(MUIX_B "+" MUIX_N),
                Child, BT_SIGDEL = MakeButton(MUIX_B "-" MUIX_N),
              End,
              Child, HSpace(0),
              Child, ColGroup(2),
                GroupSpacing(1),
                MUIA_Group_SameWidth, TRUE,
                Child, BT_SIGUP = PopButton(MUII_ArrowUp),
                Child, BT_SIGDOWN = PopButton(MUII_ArrowDown),
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
                Child, MakeCheckGroup(&CH_SIG_ACTIVE, tr(MSG_CO_SIGNATURE_ACTIVE)),

                Child, Label2(tr(MSG_CO_SIGNATURE_DESCRIPTION)),
                Child, ST_SIG_DESC = MakeString(SIZE_DEFAULT, tr(MSG_CO_SIGNATURE_DESCRIPTION)),

                Child, VGroup,
                  Child, Label2(tr(MSG_CO_SIGNATURE_TEXT)),
                  Child, VSpace(0),
                End,
                Child, VGroup,
                  Child, GR_SIGEDIT = HGroup,
                    GroupSpacing(0),
                    Child, TE_SIGEDIT = SignatureTextEditObject,
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
                  Child, BT_SIGEDIT = MakeButton(tr(MSG_CO_EditSig)),
                  Child, HGroup,
                    MUIA_Group_SameWidth, TRUE,
                    Child, BT_INSTAG = MakeButton(tr(MSG_CO_InsertTag)),
                    Child, BT_INSENV = MakeButton(tr(MSG_CO_InsertENV)),
                  End,
                End,

                Child, HSpace(1),
                Child, MakeCheckGroup(&CH_SIG_FILE, tr(MSG_CO_APPEND_SIGNATURE_FILE)),

                Child, HSpace(1),
                Child, PO_SIG_FILE = PopaslObject,
                  MUIA_Popasl_Type, ASL_FileRequest,
                  MUIA_Popstring_String, ST_SIG_FILE = MakeString(SIZE_PATHFILE, tr(MSG_CO_APPEND_SIGNATURE_FILE)),
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
            MUIA_Popstring_String, ST_TAGFILE = MakeString(SIZE_PATHFILE,tr(MSG_CO_TaglineFile)),
            MUIA_Popstring_Button, PopButton(MUII_PopFile),
          End,

          Child, Label2(tr(MSG_CO_TaglineSep)),
          Child, ST_TAGSEP = MakeString(SIZE_SMALL,tr(MSG_CO_TaglineSep)),

        End,

      End,
    End,
  TAG_MORE, inittags(msg))) != NULL)
  {
    GETDATA;

    data->LV_SIGNATURE =  LV_SIGNATURE;
    data->BT_SIGADD =     BT_SIGADD;
    data->BT_SIGDEL =     BT_SIGDEL;
    data->BT_SIGUP =      BT_SIGUP;
    data->BT_SIGDOWN =    BT_SIGDOWN;
    data->CH_SIG_ACTIVE = CH_SIG_ACTIVE;
    data->ST_SIG_DESC =   ST_SIG_DESC;
    data->GR_SIGEDIT =    GR_SIGEDIT;
    data->TE_SIGEDIT =    TE_SIGEDIT;
    data->BT_SIGEDIT =    BT_SIGEDIT;
    data->BT_INSTAG =     BT_INSTAG;
    data->BT_INSENV =     BT_INSENV;
    data->CH_SIG_FILE =   CH_SIG_FILE;
    data->PO_SIG_FILE =   PO_SIG_FILE;
    data->ST_SIG_FILE =   ST_SIG_FILE;
    data->ST_TAGFILE =    ST_TAGFILE;
    data->ST_TAGSEP =     ST_TAGSEP;

    // enhance the CycleChain
    set(BT_SIGUP,   MUIA_CycleChain, TRUE);
    set(BT_SIGDOWN, MUIA_CycleChain, TRUE);

    // set help text for gadgets
    SetHelp(LV_SIGNATURE,  MSG_HELP_CO_LV_SIGNATURE);
    SetHelp(BT_SIGADD,     MSG_HELP_CO_BT_SIGADD);
    SetHelp(BT_SIGDEL,     MSG_HELP_CO_BT_SIGDEL);
    SetHelp(BT_SIGUP,      MSG_HELP_CO_BT_SIGUP);
    SetHelp(BT_SIGDOWN,    MSG_HELP_CO_BT_SIGDOWN);
    SetHelp(CH_SIG_ACTIVE, MSG_HELP_CO_BT_SIG_ACTIVE);
    SetHelp(ST_SIG_DESC,   MSG_HELP_CO_ST_SIG_DESC);
    SetHelp(TE_SIGEDIT,    MSG_HELP_CO_TE_SIGEDIT);
    SetHelp(BT_SIGEDIT,    MSG_HELP_CO_BT_EDITSIG);
    SetHelp(BT_INSTAG,     MSG_HELP_CO_BT_INSTAG);
    SetHelp(BT_INSENV,     MSG_HELP_CO_BT_INSENV);
    SetHelp(ST_TAGFILE,    MSG_HELP_CO_ST_TAGFILE);
    SetHelp(ST_TAGSEP,     MSG_HELP_CO_ST_TAGSEP);

    // connect a notify if the user selects a different signature in the list
    DoMethod(LV_SIGNATURE, MUIM_Notify, MUIA_NList_Active, MUIV_EveryTime, obj, 1, METHOD(GetSignatureEntry));

    // connect notifies to update the SignatureNode according to the latest
    // settings in this config page
    DoMethod(CH_SIG_ACTIVE, MUIM_Notify, MUIA_Selected,        MUIV_EveryTime, obj, 1, METHOD(PutSignatureEntry));
    DoMethod(ST_SIG_DESC,   MUIM_Notify, MUIA_String_Contents, MUIV_EveryTime, obj, 1, METHOD(PutSignatureEntry));
    DoMethod(CH_SIG_FILE,   MUIM_Notify, MUIA_Selected,        MUIV_EveryTime, obj, 1, METHOD(PutSignatureEntry));

    // some button notifies
    DoMethod(BT_SIGADD,  MUIM_Notify, MUIA_Pressed, FALSE, obj, 1, METHOD(AddSignatureEntry));
    DoMethod(BT_SIGDEL,  MUIM_Notify, MUIA_Pressed, FALSE, obj, 1, METHOD(DeleteSignatureEntry));
    DoMethod(BT_SIGUP,   MUIM_Notify, MUIA_Pressed, FALSE, LV_SIGNATURE, 3, MUIM_NList_Move, MUIV_NList_Move_Selected, MUIV_NList_Move_Previous);
    DoMethod(BT_SIGDOWN, MUIM_Notify, MUIA_Pressed, FALSE, LV_SIGNATURE, 3, MUIM_NList_Move, MUIV_NList_Move_Selected, MUIV_NList_Move_Next);
    DoMethod(BT_INSTAG,  MUIM_Notify, MUIA_Pressed, FALSE, TE_SIGEDIT, 3, MUIM_TextEditor_InsertText, "%t\n", MUIV_TextEditor_InsertText_Cursor);
    DoMethod(BT_INSENV,  MUIM_Notify, MUIA_Pressed, FALSE, TE_SIGEDIT, 3, MUIM_TextEditor_InsertText, "%e\n", MUIV_TextEditor_InsertText_Cursor);
    DoMethod(BT_SIGEDIT, MUIM_Notify, MUIA_Pressed, FALSE, TE_SIGEDIT, 1, MUIM_SignatureTextEdit_EditExternally);
  }

  RETURN((IPTR)obj);
  return (IPTR)obj;
}

///
/// OVERLOAD(MUIM_ConfigPage_ConfigToGUI)
OVERLOAD(MUIM_ConfigPage_ConfigToGUI)
{
  GETDATA;
  int numSignatures = 0;
  struct SignatureNode *sn;

  ENTER();

  setstring(data->ST_TAGFILE, CE->TagsFile);
  setstring(data->ST_TAGSEP, CE->TagsSeparator);

  // clear the list first
  set(data->LV_SIGNATURE, MUIA_NList_Quiet, TRUE);
  DoMethod(data->LV_SIGNATURE, MUIM_NList_Clear);

  // we iterate through our user identity list and make sure to populate
  // out NList object correctly.
  IterateList(&CE->signatureList, struct SignatureNode *, sn)
  {
    // if the description is empty we use the mail address instead
    if(sn->description[0] == '\0')
      snprintf(sn->description, sizeof(sn->description), "%s %d", tr(MSG_CO_SIGNATURE), numSignatures+1);

    DoMethod(data->LV_SIGNATURE, MUIM_NList_InsertSingle, sn, MUIV_NList_Insert_Bottom);
    numSignatures++;
  }

  // make sure the first entry is selected per default
  xset(data->LV_SIGNATURE, MUIA_NList_Quiet, FALSE,
                           MUIA_NList_Active, MUIV_NList_Active_Top);

  // set the enabled stated of the del button according to the number of available identities
  set(data->BT_SIGDEL, MUIA_Disabled, numSignatures < 2);

  RETURN(0);
  return 0;
}

///
/// OVERLOAD(MUIM_ConfigPage_GUIToConfig)
OVERLOAD(MUIM_ConfigPage_GUIToConfig)
{
  GETDATA;
  struct SignatureNode *sn;

  ENTER();

  GetMUIString(CE->TagsFile, data->ST_TAGFILE, sizeof(CE->TagsFile));
  GetMUIString(CE->TagsSeparator, data->ST_TAGSEP, sizeof(CE->TagsSeparator));

  if((sn = (struct SignatureNode *)xget(data->TE_SIGEDIT, MUIA_SignatureTextEdit_SignatureNode)) != NULL)
  {
    sn->useSignatureFile = GetMUICheck(data->CH_SIG_FILE);
    GetMUIString(sn->filename, data->ST_SIG_FILE, sizeof(sn->filename));
  }

  // force a signature change
  // this will copy the signature text to the current signature node
  nnset(data->TE_SIGEDIT, MUIA_SignatureTextEdit_SignatureNode, NULL);

  // as the user may have changed the order of the signatures
  // we have to make sure the order in the NList fits to the
  // exec list order of our Signature list
  SortNListToExecList(data->LV_SIGNATURE, &CE->signatureList);

  RETURN(0);
  return 0;
}

///
/// DECLARE(GetSignatureEntry)
// fills form with data from selected list entry
DECLARE(GetSignatureEntry)
{
  GETDATA;
  struct SignatureNode *sn = NULL;
  LONG pos = MUIV_NList_GetPos_Start;

  ENTER();

  // get the currently selected signature
  DoMethod(data->LV_SIGNATURE, MUIM_NList_GetEntry, MUIV_NList_GetEntry_Active, &sn);

  // make sure to disable GUI elements
  if(sn == NULL || xget(data->LV_SIGNATURE, MUIA_NList_Entries) < 2)
    set(data->BT_SIGDEL, MUIA_Disabled, TRUE);
  else
    set(data->BT_SIGDEL, MUIA_Disabled, FALSE);

  if(sn != NULL)
    DoMethod(data->LV_SIGNATURE, MUIM_NList_GetPos, sn, &pos);
  else
    pos = 0;

  set(data->BT_SIGUP, MUIA_Disabled, pos == 0);
  set(data->BT_SIGDOWN, MUIA_Disabled, pos == (LONG)xget(data->LV_SIGNATURE, MUIA_NList_Entries) - 1);

  if(sn != NULL)
  {
    // all notifies here are nnset() notifies so that we don't trigger any additional
    // notify or otherwise we would run into problems.
    nnset(data->CH_SIG_ACTIVE, MUIA_Selected, sn->active);
    nnset(data->ST_SIG_DESC, MUIA_String_Contents, sn->description);
    nnset(data->TE_SIGEDIT, MUIA_SignatureTextEdit_SignatureNode, sn);
    nnset(data->CH_SIG_FILE, MUIA_Selected, sn->useSignatureFile);
    nnset(data->ST_SIG_FILE, MUIA_String_Contents, sn->filename);
    DoMethod(_win(obj), MUIM_MultiSet, MUIA_Disabled, sn->useSignatureFile == TRUE,
      data->BT_SIGEDIT,
      data->BT_INSTAG,
      data->BT_INSENV,
      NULL);
    set(data->PO_SIG_FILE, MUIA_Disabled, sn->useSignatureFile == FALSE);
  }

  RETURN(0);
  return 0;
}

///
/// DECLARE(PutSignatureEntry)
// fills form data into selected list entry
DECLARE(PutSignatureEntry)
{
  GETDATA;
  int p;

  ENTER();

  p = xget(data->LV_SIGNATURE, MUIA_NList_Active);
  if(p != MUIV_NList_Active_Off)
  {
    struct SignatureNode *sn = NULL;

    DoMethod(data->LV_SIGNATURE, MUIM_NList_GetEntry, p, &sn);
    if(sn != NULL)
    {
      sn->active = GetMUICheck(data->CH_SIG_ACTIVE);
      GetMUIString(sn->description, data->ST_SIG_DESC, sizeof(sn->description));
      sn->useSignatureFile = GetMUICheck(data->CH_SIG_FILE);
      GetMUIString(sn->filename, data->ST_SIG_FILE, sizeof(sn->filename));

      // if the user hasn't yet entered an own description we generate an
      // own one
      if(sn->description[0] == '\0' || strcmp(sn->description, tr(MSG_NewEntry)) == 0)
        strlcpy(sn->description, tr(MSG_CO_Signature), sizeof(sn->description));

      DoMethod(_win(obj), MUIM_MultiSet, MUIA_Disabled, sn->useSignatureFile == TRUE,
        data->BT_SIGEDIT,
        data->BT_INSTAG,
        data->BT_INSENV,
        NULL);
      set(data->PO_SIG_FILE, MUIA_Disabled, sn->useSignatureFile == FALSE);
      set(data->TE_SIGEDIT, MUIA_SignatureTextEdit_UseSignatureFile, sn->useSignatureFile);

      // update the signature chooser in case the user changed something
      // on the Identities config page
      set(obj, MUIA_ConfigPage_ConfigUpdate, cp_Signature);

      // redraw the list
      DoMethod(data->LV_SIGNATURE, MUIM_NList_Redraw, p);
    }
  }

  RETURN(0);
  return 0;
}

///
/// DECLARE(AddSignatureEntry)
// adds a new entry to the signature list
DECLARE(AddSignatureEntry)
{
  GETDATA;
  struct SignatureNode *sn;

  ENTER();

  if((sn = CreateNewSignature()) != NULL)
  {
    // create new default values
    strlcpy(sn->description, tr(MSG_NewEntry), sizeof(sn->description));

    // new signatures don't use a file by default
    sn->useSignatureFile = FALSE;

    // add the new signature to the list
    DoMethod(data->LV_SIGNATURE, MUIM_NList_InsertSingle, sn, MUIV_NList_Insert_Bottom);

    // add the signature to the list
    AddTail((struct List *)&CE->signatureList, (struct Node *)sn);

    // set the new entry active and make sure that the email gadget will be
    // set as the new active object of the window as that gadget will be used
    // to automatically set the account name.
    set(data->LV_SIGNATURE, MUIA_NList_Active, MUIV_List_Active_Bottom);
    set(_win(obj), MUIA_Window_ActiveObject, data->ST_SIG_DESC);
  }

  RETURN(0);
  return 0;
}

///
/// DECLARE(DeleteSignatureEntry)
// deletes an entry from the signature list
DECLARE(DeleteSignatureEntry)
{
  GETDATA;
  struct SignatureNode *sn = NULL;

  ENTER();

  DoMethod(data->LV_SIGNATURE, MUIM_NList_GetEntry, MUIV_NList_GetEntry_Active, &sn);

  if(sn != NULL &&
     xget(data->LV_SIGNATURE, MUIA_NList_Entries) > 1)
  {
    DoMethod(data->LV_SIGNATURE, MUIM_NList_Remove, MUIV_NList_Remove_Active);

    // remove it from the internal user identity list as well.
    Remove((struct Node *)sn);

    FreeSysObject(ASOT_NODE, sn);
  }

  RETURN(0);
  return 0;
}

///
