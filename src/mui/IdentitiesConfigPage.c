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
 Description: "Identities" configuration page

***************************************************************************/

#include "IdentitiesConfigPage_cl.h"

#include <proto/muimaster.h>
#include <mui/BetterString_mcc.h>
#include <mui/NBalance_mcc.h>
#include <mui/NList_mcc.h>
#include <mui/NListview_mcc.h>

#include "YAM.h"

#include "mui/AddressBookWindow.h"
#include "mui/ConfigPage.h"
#include "mui/ConfigPageList.h"
#include "mui/FolderRequestPopup.h"
#include "mui/IdentityList.h"
#include "mui/MailServerChooser.h"
#include "mui/PGPKeyPopup.h"
#include "mui/SignatureChooser.h"

#include "Config.h"
#include "MailServers.h"
#include "MUIObjects.h"
#include "Signature.h"
#include "UserIdentity.h"

#include "Debug.h"

/* CLASSDATA
struct Data
{
  Object *LV_IDENTITY;
  Object *BT_IADD;
  Object *BT_IDEL;
  Object *BT_IDENTITYUP;
  Object *BT_IDENTITYDOWN;
  Object *CH_IDENTITY_ENABLED;
  Object *ST_IDENTITY_DESCRIPTION;
  Object *ST_IDENTITY_REALNAME;
  Object *ST_IDENTITY_EMAIL;
  Object *ST_IDENTITY_ORGANIZATION;
  Object *CY_IDENTITY_MAILSERVER;
  Object *CY_IDENTITY_SIGNATURE;
  Object *ST_IDENTITY_CC;
  Object *ST_IDENTITY_BCC;
  Object *ST_IDENTITY_REPLYTO;
  Object *ST_IDENTITY_EXTRAHEADER;
  Object *ST_IDENTITY_PHOTOURL;
  Object *CH_IDENTITY_SENTFOLDER;
  Object *PO_IDENTITY_SENTFOLDER;
  Object *CH_IDENTITY_QUOTEMAILS;
  Object *CY_IDENTITY_QUOTEPOS;
  Object *CY_IDENTITY_SIGPOS;
  Object *CH_IDENTITY_SIGREPLY;
  Object *CH_IDENTITY_SIGFORWARD;
  Object *CH_IDENTITY_ADDINFO;
  Object *CH_IDENTITY_REQUESTMDN;
  Object *CH_IDENTITY_USEPGP;
  Object *PO_IDENTITY_PGPID;
  Object *ST_IDENTITY_PGPURL;
  Object *CH_IDENTITY_PGPSIGN_UNENC;
  Object *CH_IDENTITY_PGPSIGN_ENC;
  Object *CH_IDENTITY_PGPENC_ALL;
  Object *CH_IDENTITY_PGPENC_SELF;
};
*/

/* Overloaded Methods */
/// OVERLOAD(OM_NEW)
OVERLOAD(OM_NEW)
{
  static const char *rtitles[5];
  static const char *quotePosition[3];
  static const char *signaturePosition[3];
  Object *LV_IDENTITY;
  Object *BT_IADD;
  Object *BT_IDEL;
  Object *BT_IDENTITYUP;
  Object *BT_IDENTITYDOWN;
  Object *CH_IDENTITY_ENABLED;
  Object *ST_IDENTITY_DESCRIPTION;
  Object *ST_IDENTITY_REALNAME;
  Object *ST_IDENTITY_EMAIL;
  Object *ST_IDENTITY_ORGANIZATION;
  Object *CY_IDENTITY_MAILSERVER;
  Object *CY_IDENTITY_SIGNATURE;
  Object *ST_IDENTITY_CC;
  Object *ST_IDENTITY_BCC;
  Object *ST_IDENTITY_REPLYTO;
  Object *ST_IDENTITY_EXTRAHEADER;
  Object *ST_IDENTITY_PHOTOURL;
  Object *CH_IDENTITY_SENTFOLDER;
  Object *PO_IDENTITY_SENTFOLDER;
  Object *CH_IDENTITY_QUOTEMAILS;
  Object *CY_IDENTITY_QUOTEPOS;
  Object *CY_IDENTITY_SIGPOS;
  Object *CH_IDENTITY_SIGREPLY;
  Object *CH_IDENTITY_SIGFORWARD;
  Object *CH_IDENTITY_ADDINFO;
  Object *CH_IDENTITY_REQUESTMDN;
  Object *CH_IDENTITY_USEPGP;
  Object *PO_IDENTITY_PGPID;
  Object *ST_IDENTITY_PGPURL;
  Object *CH_IDENTITY_PGPSIGN_UNENC;
  Object *CH_IDENTITY_PGPSIGN_ENC;
  Object *CH_IDENTITY_PGPENC_ALL;
  Object *CH_IDENTITY_PGPENC_SELF;

  ENTER();

  rtitles[0] = tr(MSG_CO_IDENTITY_REGISTER_SETTINGS);
  rtitles[1] = tr(MSG_CO_IDENTITY_REGISTER_COMPOSE);
  rtitles[2] = tr(MSG_CO_IDENTITY_REGISTER_PGPSEC);
  rtitles[3] = NULL;

  quotePosition[0] = tr(MSG_CO_IDENTITY_BELOWQUOTE);
  quotePosition[1] = tr(MSG_CO_IDENTITY_ABOVEQUOTE);
  quotePosition[2] = NULL;

  signaturePosition[0] = tr(MSG_CO_IDENTITY_BELOWQUOTE);
  signaturePosition[1] = tr(MSG_CO_IDENTITY_ABOVEQUOTE);
  signaturePosition[2] = NULL;

  if((obj = DoSuperNew(cl, obj,
    MUIA_HelpNode, "Configuration#Identities",
    MUIA_ConfigPage_Page, cp_TCPIP,
    MUIA_ConfigPage_UseScrollgroup, FALSE,
    MUIA_ConfigPage_AddSpacer, FALSE,
    MUIA_ConfigPage_Contents, HGroup,
      GroupSpacing(0),
      Child, VGroup,
        MUIA_HorizWeight, 30,

        Child, HBarT(tr(MSG_CO_IDENTITY_LIST)), End,

        Child, NListviewObject,
          MUIA_CycleChain, TRUE,
          MUIA_NListview_NList, LV_IDENTITY = IdentityListObject,
          End,
        End,

        Child, HGroup,
          Child, ColGroup(2),
            MUIA_Group_Spacing, 1,
            MUIA_Group_SameWidth, TRUE,
            MUIA_Weight, 1,
            Child, BT_IADD = MakeButton(MUIX_B "+" MUIX_N),
            Child, BT_IDEL = MakeButton(MUIX_B "-" MUIX_N),
          End,
          Child, HSpace(0),
          Child, ColGroup(2),
            MUIA_Group_Spacing, 1,
            MUIA_Group_SameWidth, TRUE,
            Child, BT_IDENTITYUP = PopButton(MUII_ArrowUp),
            Child, BT_IDENTITYDOWN = PopButton(MUII_ArrowDown),
          End,
        End,
      End,

      Child, NBalanceObject,
        MUIA_Balance_Quiet, TRUE,
      End,

      Child, RegisterGroup(rtitles),
        MUIA_CycleChain, TRUE,
        MUIA_HorizWeight, 70,

        // General Settings
        Child, ScrollgroupObject,
          MUIA_Scrollgroup_FreeHoriz, FALSE,
          MUIA_Scrollgroup_AutoBars,  TRUE,
          MUIA_Scrollgroup_Contents,  VGroupV,

            Child, ColGroup(2), GroupFrameT(tr(MSG_CO_IDENTITY_SETTINGS)),

              Child, HSpace(1),
              Child, MakeCheckGroup(&CH_IDENTITY_ENABLED, tr(MSG_CO_IdentityActive)),

              Child, Label2(tr(MSG_CO_IDENTITY_DESCRIPTION)),
              Child, ST_IDENTITY_DESCRIPTION = MakeString(SIZE_DEFAULT, tr(MSG_CO_IDENTITY_DESCRIPTION)),

              Child, Label2(tr(MSG_CO_RealName)),
              Child, ST_IDENTITY_REALNAME = MakeString(SIZE_REALNAME, tr(MSG_CO_RealName)),

              Child, Label2(tr(MSG_CO_EmailAddress)),
              Child, MakeAddressField(&ST_IDENTITY_EMAIL, tr(MSG_CO_EmailAddress), MSG_HELP_CO_ST_IDENTITY_EMAIL, ABM_CONFIG, -1, AFF_NOFULLNAME|AFF_NOCACHE|AFF_NOVALID|AFF_RESOLVEINACTIVE),

              Child, Label2(tr(MSG_CO_Organization)),
              Child, ST_IDENTITY_ORGANIZATION = MakeString(SIZE_DEFAULT, tr(MSG_CO_Organization)),

              Child, Label2(tr(MSG_CO_IDENTITY_MAILSERVER)),
              Child, CY_IDENTITY_MAILSERVER = MailServerChooserObject,
                MUIA_MailServerChooser_MailServerList, &CE->smtpServerList,
                MUIA_ControlChar, ShortCut(tr(MSG_CO_IDENTITY_MAILSERVER)),
              End,

              Child, Label2(tr(MSG_CO_IDENTITY_SIGNATURE)),
              Child, CY_IDENTITY_SIGNATURE = SignatureChooserObject,
                MUIA_SignatureChooser_SignatureList, &CE->signatureList,
                MUIA_ControlChar, ShortCut(tr(MSG_CO_IDENTITY_SIGNATURE)),
              End,

              Child, HVSpace,
              Child, HVSpace,

            End,
          End,
        End,

        // Compose Mail Settings
        Child, ScrollgroupObject,
          MUIA_Scrollgroup_FreeHoriz, FALSE,
          MUIA_Scrollgroup_AutoBars,  TRUE,
          MUIA_Scrollgroup_Contents,  VGroupV,

            Child, ColGroup(2), GroupFrameT(tr(MSG_CO_IDENTITY_COMPOSE)),

              Child, Label2(tr(MSG_CO_IDENTITY_CC)),
              Child, MakeAddressField(&ST_IDENTITY_CC, tr(MSG_CO_IDENTITY_CC), MSG_HELP_CO_ST_IDENTITY_CC, ABM_CONFIG, -1, AFF_ALLOW_MULTI),

              Child, Label2(tr(MSG_CO_IDENTITY_BCC)),
              Child, MakeAddressField(&ST_IDENTITY_BCC, tr(MSG_CO_IDENTITY_BCC), MSG_HELP_CO_ST_IDENTITY_BCC, ABM_CONFIG, -1, AFF_ALLOW_MULTI),

              Child, Label2(tr(MSG_CO_IDENTITY_REPLYTO)),
              Child, MakeAddressField(&ST_IDENTITY_REPLYTO, tr(MSG_CO_IDENTITY_REPLYTO), MSG_HELP_CO_ST_IDENTITY_REPLYTO, ABM_CONFIG, -1, AFF_ALLOW_MULTI),

              Child, Label2(tr(MSG_CO_ExtraHeaders)),
              Child, ST_IDENTITY_EXTRAHEADER = MakeString(SIZE_LARGE, tr(MSG_CO_ExtraHeaders)),

              Child, Label2(tr(MSG_CO_IDENTITY_PHOTOURL)),
              Child, ST_IDENTITY_PHOTOURL = MakeString(SIZE_URL, tr(MSG_CO_IDENTITY_PHOTOURL)),

              Child, HSpace(1),
              Child, HBarT(tr(MSG_CO_IDENTITY_COMPOSE_OPTIONS)), End,

              Child, HSpace(1),
              Child, VGroup,
                Child, ColGroup(2),

                  Child, CH_IDENTITY_SENTFOLDER = MakeCheck(tr(MSG_CO_IDENTITY_COMPOSE_SENTFOLDER)),
                  Child, LLabel1(tr(MSG_CO_IDENTITY_COMPOSE_SENTFOLDER)),

                  Child, HSpace(0),
                  Child, PO_IDENTITY_SENTFOLDER = FolderRequestPopupObject,
                  End,

                  Child, CH_IDENTITY_QUOTEMAILS = MakeCheck(tr(MSG_CO_IDENTITY_COMPOSE_QUOTE)),
                  Child, LLabel1(tr(MSG_CO_IDENTITY_COMPOSE_QUOTE)),

                  Child, HSpace(0),
                  Child, ColGroup(2),
                    Child, LLabel1(tr(MSG_CO_IDENTITY_COMPOSE_ANSWER)),
                    Child, CY_IDENTITY_QUOTEPOS = MakeCycle(quotePosition, tr(MSG_CO_IDENTITY_COMPOSE_ANSWER)),
                    Child, LLabel1(tr(MSG_CO_IDENTITY_COMPOSE_SIGNATURE)),
                    Child, CY_IDENTITY_SIGPOS = MakeCycle(signaturePosition, tr(MSG_CO_IDENTITY_COMPOSE_SIGNATURE)),
                  End,

                End,
              End,

              Child, HSpace(1),
              Child, HGroup,
                Child, CH_IDENTITY_SIGREPLY = MakeCheck(tr(MSG_CO_IDENTITY_USESIG_REPLY)),
                Child, LLabel1(tr(MSG_CO_IDENTITY_USESIG_REPLY)),
                Child, HSpace(0),
              End,

              Child, HSpace(1),
              Child, HGroup,
                Child, CH_IDENTITY_SIGFORWARD = MakeCheck(tr(MSG_CO_IDENTITY_USESIG_FORWARD)),
                Child, LLabel1(tr(MSG_CO_IDENTITY_USESIG_FORWARD)),
                Child, HSpace(0),
              End,

              Child, HSpace(1),
              Child, HGroup,
                Child, CH_IDENTITY_ADDINFO = MakeCheck(tr(MSG_CO_IDENTITY_ADDINFO)),
                Child, LLabel1(tr(MSG_CO_IDENTITY_ADDINFO)),
                Child, HSpace(0),
              End,

              Child, HSpace(1),
              Child, HGroup,
                Child, CH_IDENTITY_REQUESTMDN = MakeCheck(tr(MSG_CO_IDENTITY_REQUESTMDN)),
                Child, LLabel1(tr(MSG_CO_IDENTITY_REQUESTMDN)),
                Child, HSpace(0),
              End,

              Child, HVSpace,
              Child, HVSpace,

            End,
          End,
        End,

        // PGP security settings
        Child, ScrollgroupObject,
          MUIA_Scrollgroup_FreeHoriz, FALSE,
          MUIA_Scrollgroup_AutoBars,  TRUE,
          MUIA_Scrollgroup_Contents,  VGroupV,

            Child, ColGroup(2), GroupFrameT(tr(MSG_CO_IDENTITY_PGPSETTINGS)),

              Child, HSpace(1),
              Child, HGroup,
                Child, CH_IDENTITY_USEPGP = MakeCheck(tr(MSG_CO_IDENTITY_USEPGP)),
                Child, LLabel1(tr(MSG_CO_IDENTITY_USEPGP)),
                Child, HSpace(0),
              End,

              Child, Label2(tr(MSG_CO_IDENTITY_PGPKEY_ID)),
              Child, PO_IDENTITY_PGPID = PGPKeyPopupObject,
                MUIA_PGPKeyPopup_Secret, TRUE,
                MUIA_PGPKeyPopup_Label, tr(MSG_CO_IDENTITY_PGPKEY_ID),
              End,

              Child, Label2(tr(MSG_CO_IDENTITY_PGPKEY_URL)),
              Child, ST_IDENTITY_PGPURL = MakeString(SIZE_URL, tr(MSG_CO_IDENTITY_PGPKEY_URL)),

              Child, HSpace(1),
              Child, HBarT(tr(MSG_CO_IDENTITY_PGP_OPTIONS)), End,

              Child, HSpace(1),
              Child, HGroup,
                Child, CH_IDENTITY_PGPSIGN_UNENC = MakeCheck(tr(MSG_CO_IDENTITY_PGP_SIGN_UNENC)),
                Child, LLabel1(tr(MSG_CO_IDENTITY_PGP_SIGN_UNENC)),
                Child, HSpace(0),
              End,

              Child, HSpace(1),
              Child, HGroup,
                Child, CH_IDENTITY_PGPSIGN_ENC = MakeCheck(tr(MSG_CO_IDENTITY_PGP_SIGN_ENC)),
                Child, LLabel1(tr(MSG_CO_IDENTITY_PGP_SIGN_ENC)),
                Child, HSpace(0),
              End,

              Child, HSpace(1),
              Child, HGroup,
                Child, CH_IDENTITY_PGPENC_ALL = MakeCheck(tr(MSG_CO_IDENTITY_PGP_ENCRYPTALL)),
                Child, LLabel1(tr(MSG_CO_IDENTITY_PGP_ENCRYPTALL)),
                Child, HSpace(0),
              End,

              Child, HSpace(1),
              Child, HGroup,
                Child, CH_IDENTITY_PGPENC_SELF = MakeCheck(tr(MSG_CO_IDENTITY_PGP_ADDOWN)),
                Child, LLabel1(tr(MSG_CO_IDENTITY_PGP_ADDOWN)),
                Child, HSpace(0),
              End,

              Child, HVSpace,
              Child, HVSpace,

            End,
          End,
        End,
      End,

    End,
  TAG_MORE, inittags(msg))) != NULL)
  {
    GETDATA;

    data->LV_IDENTITY =                LV_IDENTITY;
    data->BT_IADD =                    BT_IADD;
    data->BT_IDEL =                    BT_IDEL;
    data->BT_IDENTITYUP =              BT_IDENTITYUP;
    data->BT_IDENTITYDOWN =            BT_IDENTITYDOWN;
    data->CH_IDENTITY_ENABLED =        CH_IDENTITY_ENABLED;
    data->ST_IDENTITY_DESCRIPTION =    ST_IDENTITY_DESCRIPTION;
    data->ST_IDENTITY_REALNAME =       ST_IDENTITY_REALNAME;
    data->ST_IDENTITY_EMAIL =          ST_IDENTITY_EMAIL;
    data->ST_IDENTITY_ORGANIZATION =   ST_IDENTITY_ORGANIZATION;
    data->CY_IDENTITY_MAILSERVER =     CY_IDENTITY_MAILSERVER;
    data->CY_IDENTITY_SIGNATURE =      CY_IDENTITY_SIGNATURE;
    data->ST_IDENTITY_CC =             ST_IDENTITY_CC;
    data->ST_IDENTITY_BCC =            ST_IDENTITY_BCC;
    data->ST_IDENTITY_REPLYTO =        ST_IDENTITY_REPLYTO;
    data->ST_IDENTITY_EXTRAHEADER =    ST_IDENTITY_EXTRAHEADER;
    data->ST_IDENTITY_PHOTOURL =       ST_IDENTITY_PHOTOURL;
    data->CH_IDENTITY_SENTFOLDER =     CH_IDENTITY_SENTFOLDER;
    data->PO_IDENTITY_SENTFOLDER =     PO_IDENTITY_SENTFOLDER;
    data->CH_IDENTITY_QUOTEMAILS =     CH_IDENTITY_QUOTEMAILS;
    data->CY_IDENTITY_QUOTEPOS =       CY_IDENTITY_QUOTEPOS;
    data->CY_IDENTITY_SIGPOS =         CY_IDENTITY_SIGPOS;
    data->CH_IDENTITY_SIGREPLY =       CH_IDENTITY_SIGREPLY;
    data->CH_IDENTITY_SIGFORWARD =     CH_IDENTITY_SIGFORWARD;
    data->CH_IDENTITY_ADDINFO =        CH_IDENTITY_ADDINFO;
    data->CH_IDENTITY_REQUESTMDN =     CH_IDENTITY_REQUESTMDN;
    data->CH_IDENTITY_USEPGP =         CH_IDENTITY_USEPGP;
    data->PO_IDENTITY_PGPID =          PO_IDENTITY_PGPID;
    data->ST_IDENTITY_PGPURL =         ST_IDENTITY_PGPURL;
    data->CH_IDENTITY_PGPSIGN_UNENC =  CH_IDENTITY_PGPSIGN_UNENC;
    data->CH_IDENTITY_PGPSIGN_ENC =    CH_IDENTITY_PGPSIGN_ENC;
    data->CH_IDENTITY_PGPENC_ALL =     CH_IDENTITY_PGPENC_ALL;
    data->CH_IDENTITY_PGPENC_SELF =    CH_IDENTITY_PGPENC_SELF;

    // enhance the CycleChain
    set(BT_IDENTITYUP,   MUIA_CycleChain, TRUE);
    set(BT_IDENTITYDOWN, MUIA_CycleChain, TRUE);

    // set help text to objects
    SetHelp(CH_IDENTITY_ENABLED,           MSG_HELP_CO_CH_IDENTITY_ENABLED);
    SetHelp(ST_IDENTITY_DESCRIPTION,       MSG_HELP_CO_ST_IDENTITY_DESCRIPTION);
    SetHelp(ST_IDENTITY_REALNAME,          MSG_HELP_CO_ST_IDENTITY_REALNAME);
    SetHelp(ST_IDENTITY_EMAIL,             MSG_HELP_CO_ST_IDENTITY_EMAIL);
    SetHelp(ST_IDENTITY_ORGANIZATION,      MSG_HELP_CO_ST_IDENTITY_ORGANIZATION);
    SetHelp(CY_IDENTITY_MAILSERVER,        MSG_HELP_CO_CY_IDENTITY_MAILSERVER);
    SetHelp(CY_IDENTITY_SIGNATURE,         MSG_HELP_CO_CY_IDENTITY_SIGNATURE);
    SetHelp(ST_IDENTITY_CC,                MSG_HELP_CO_ST_IDENTITY_CC);
    SetHelp(ST_IDENTITY_BCC,               MSG_HELP_CO_ST_IDENTITY_BCC);
    SetHelp(ST_IDENTITY_REPLYTO,           MSG_HELP_CO_ST_IDENTITY_REPLYTO);
    SetHelp(ST_IDENTITY_EXTRAHEADER,       MSG_HELP_CO_ST_IDENTITY_EXTRAHEADER);
    SetHelp(ST_IDENTITY_PHOTOURL,          MSG_HELP_CO_ST_IDENTITY_PHOTOURL);
    SetHelp(CH_IDENTITY_SENTFOLDER,        MSG_HELP_CO_CH_IDENTITY_SENTFOLDER);
    SetHelp(PO_IDENTITY_SENTFOLDER,        MSG_HELP_CO_TX_IDENTITY_SENTFOLDER);
    SetHelp(CH_IDENTITY_QUOTEMAILS,        MSG_HELP_CO_CH_IDENTITY_QUOTEMAILS);
    SetHelp(CY_IDENTITY_QUOTEPOS,          MSG_HELP_CO_CH_IDENTITY_QUOTEPOS);
    SetHelp(CY_IDENTITY_SIGPOS,            MSG_HELP_CO_CH_IDENTITY_SIGPOS);
    SetHelp(CH_IDENTITY_SIGREPLY,          MSG_HELP_CO_CH_IDENTITY_SIGREPLY);
    SetHelp(CH_IDENTITY_SIGFORWARD,        MSG_HELP_CO_CH_IDENTITY_SIGFORWARD);
    SetHelp(CH_IDENTITY_ADDINFO,           MSG_HELP_CO_CH_IDENTITY_ADDINFO);
    SetHelp(CH_IDENTITY_REQUESTMDN,        MSG_HELP_CO_CH_IDENTITY_REQUESTMDN);
    SetHelp(CH_IDENTITY_USEPGP,            MSG_HELP_CO_CH_IDENTITY_USEPGP);
    SetHelp(PO_IDENTITY_PGPID,             MSG_HELP_CO_ST_IDENTITY_PGPID);
    SetHelp(ST_IDENTITY_PGPURL,            MSG_HELP_CO_ST_IDENTITY_PGPURL);
    SetHelp(CH_IDENTITY_PGPSIGN_UNENC,     MSG_HELP_CO_CH_IDENTITY_PGPSIGN_UNENC);
    SetHelp(CH_IDENTITY_PGPSIGN_ENC,       MSG_HELP_CO_CH_IDENTITY_PGPSIGN_ENC);
    SetHelp(CH_IDENTITY_PGPENC_ALL,        MSG_HELP_CO_CH_IDENTITY_PGPENC_ALL);
    SetHelp(CH_IDENTITY_PGPENC_SELF,       MSG_HELP_CO_CH_IDENTITY_PGPENC_SELF);

    // connect a notify if the user selects a different identity in the list
    DoMethod(LV_IDENTITY, MUIM_Notify, MUIA_NList_Active, MUIV_EveryTime, obj, 1, METHOD(IdentityToGUI));

    // connect notifies to update the UserIdentityNode according to the latest
    // settings in this config page
    DoMethod(CH_IDENTITY_ENABLED,           MUIM_Notify, MUIA_Selected,                             MUIV_EveryTime, obj, 1, METHOD(GUIToIdentity));
    DoMethod(ST_IDENTITY_DESCRIPTION,       MUIM_Notify, MUIA_String_Contents,                      MUIV_EveryTime, obj, 1, METHOD(GUIToIdentity));
    DoMethod(ST_IDENTITY_REALNAME,          MUIM_Notify, MUIA_String_Contents,                      MUIV_EveryTime, obj, 1, METHOD(GUIToIdentity));
    DoMethod(ST_IDENTITY_EMAIL,             MUIM_Notify, MUIA_String_Contents,                      MUIV_EveryTime, obj, 1, METHOD(GUIToIdentity));
    DoMethod(ST_IDENTITY_ORGANIZATION,      MUIM_Notify, MUIA_String_Contents,                      MUIV_EveryTime, obj, 1, METHOD(GUIToIdentity));
    DoMethod(CY_IDENTITY_MAILSERVER,        MUIM_Notify, MUIA_Cycle_Active,                         MUIV_EveryTime, obj, 1, METHOD(GUIToIdentity));
    DoMethod(CY_IDENTITY_SIGNATURE,         MUIM_Notify, MUIA_Cycle_Active,                         MUIV_EveryTime, obj, 1, METHOD(GUIToIdentity));
    DoMethod(ST_IDENTITY_CC,                MUIM_Notify, MUIA_String_Contents,                      MUIV_EveryTime, obj, 1, METHOD(GUIToIdentity));
    DoMethod(ST_IDENTITY_BCC,               MUIM_Notify, MUIA_String_Contents,                      MUIV_EveryTime, obj, 1, METHOD(GUIToIdentity));
    DoMethod(ST_IDENTITY_REPLYTO,           MUIM_Notify, MUIA_String_Contents,                      MUIV_EveryTime, obj, 1, METHOD(GUIToIdentity));
    DoMethod(ST_IDENTITY_EXTRAHEADER,       MUIM_Notify, MUIA_String_Contents,                      MUIV_EveryTime, obj, 1, METHOD(GUIToIdentity));
    DoMethod(ST_IDENTITY_PHOTOURL,          MUIM_Notify, MUIA_String_Contents,                      MUIV_EveryTime, obj, 1, METHOD(GUIToIdentity));
    DoMethod(CH_IDENTITY_SENTFOLDER,        MUIM_Notify, MUIA_Selected,                             MUIV_EveryTime, obj, 1, METHOD(GUIToIdentity));
    DoMethod(PO_IDENTITY_SENTFOLDER,        MUIM_Notify, MUIA_FolderRequestPopup_FolderChanged,     MUIV_EveryTime, obj, 1, METHOD(GUIToIdentity));
    DoMethod(CH_IDENTITY_QUOTEMAILS,        MUIM_Notify, MUIA_Selected,                             MUIV_EveryTime, obj, 1, METHOD(GUIToIdentity));
    DoMethod(CY_IDENTITY_QUOTEPOS,          MUIM_Notify, MUIA_Cycle_Active,                         MUIV_EveryTime, obj, 1, METHOD(GUIToIdentity));
    DoMethod(CY_IDENTITY_SIGPOS,            MUIM_Notify, MUIA_Cycle_Active,                         MUIV_EveryTime, obj, 1, METHOD(GUIToIdentity));
    DoMethod(CH_IDENTITY_SIGREPLY,          MUIM_Notify, MUIA_Selected,                             MUIV_EveryTime, obj, 1, METHOD(GUIToIdentity));
    DoMethod(CH_IDENTITY_SIGFORWARD,        MUIM_Notify, MUIA_Selected,                             MUIV_EveryTime, obj, 1, METHOD(GUIToIdentity));
    DoMethod(CH_IDENTITY_ADDINFO,           MUIM_Notify, MUIA_Selected,                             MUIV_EveryTime, obj, 1, METHOD(GUIToIdentity));
    DoMethod(CH_IDENTITY_REQUESTMDN,        MUIM_Notify, MUIA_Selected,                             MUIV_EveryTime, obj, 1, METHOD(GUIToIdentity));
    DoMethod(CH_IDENTITY_USEPGP,            MUIM_Notify, MUIA_Selected,                             MUIV_EveryTime, obj, 1, METHOD(GUIToIdentity));
    DoMethod(PO_IDENTITY_PGPID,             MUIM_Notify, MUIA_PGPKeyPopup_PGPKeyChanged,            MUIV_EveryTime, obj, 1, METHOD(GUIToIdentity));
    DoMethod(ST_IDENTITY_PGPURL,            MUIM_Notify, MUIA_String_Contents,                      MUIV_EveryTime, obj, 1, METHOD(GUIToIdentity));
    DoMethod(CH_IDENTITY_PGPSIGN_UNENC,     MUIM_Notify, MUIA_Selected,                             MUIV_EveryTime, obj, 1, METHOD(GUIToIdentity));
    DoMethod(CH_IDENTITY_PGPSIGN_ENC,       MUIM_Notify, MUIA_Selected,                             MUIV_EveryTime, obj, 1, METHOD(GUIToIdentity));
    DoMethod(CH_IDENTITY_PGPENC_ALL,        MUIM_Notify, MUIA_Selected,                             MUIV_EveryTime, obj, 1, METHOD(GUIToIdentity));
    DoMethod(CH_IDENTITY_PGPENC_SELF,       MUIM_Notify, MUIA_Selected,                             MUIV_EveryTime, obj, 1, METHOD(GUIToIdentity));

    DoMethod(BT_IADD,         MUIM_Notify, MUIA_Pressed, FALSE, obj, 1, METHOD(AddIdentityEntry));
    DoMethod(BT_IDEL,         MUIM_Notify, MUIA_Pressed, FALSE, obj, 1, METHOD(DeleteIdentityEntry));
    DoMethod(BT_IDENTITYUP,   MUIM_Notify, MUIA_Pressed, FALSE, LV_IDENTITY, 3, MUIM_NList_Move, MUIV_NList_Move_Selected, MUIV_NList_Move_Previous);
    DoMethod(BT_IDENTITYDOWN, MUIM_Notify, MUIA_Pressed, FALSE, LV_IDENTITY, 3, MUIM_NList_Move, MUIV_NList_Move_Selected, MUIV_NList_Move_Next);
  }

  RETURN((IPTR)obj);
  return (IPTR)obj;
}

///
/// OVERLOAD(MUIM_ConfigPage_ConfigToGUI)
OVERLOAD(MUIM_ConfigPage_ConfigToGUI)
{
  GETDATA;
  int numIdentities = 0;
  struct UserIdentityNode *uin;

  ENTER();

  // clear the lists first
  set(data->LV_IDENTITY, MUIA_NList_Quiet, TRUE);
  DoMethod(data->LV_IDENTITY, MUIM_NList_Clear);

  // we iterate through our user identity list and make sure to populate
  // out NList object correctly.
  IterateList(&CE->userIdentityList, struct UserIdentityNode *, uin)
  {
	// if the description is empty we use the mail address instead
	if(uin->description[0] == '\0')
	  strlcpy(uin->description, uin->address, sizeof(uin->description));

	DoMethod(data->LV_IDENTITY, MUIM_NList_InsertSingle, uin, MUIV_NList_Insert_Bottom);
	numIdentities++;
  }

  // make sure the first entry is selected per default
  xset(data->LV_IDENTITY, MUIA_NList_Quiet, FALSE,
						  MUIA_NList_Active, MUIV_NList_Active_Top);

  // set the enabled stated of the del button according to the number of available identities
  set(data->BT_IDEL, MUIA_Disabled, numIdentities < 2);

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
  SortNListToExecList(data->LV_IDENTITY, &CE->userIdentityList);

  RETURN(0);
  return 0;
}

///
/// OVERLOAD(MUIM_ConfigPage_ConfigUpdate)
OVERLOAD(MUIM_ConfigPage_ConfigUpdate)
{
  GETDATA;
  enum ConfigPage sourcePage = ((struct MUIP_ConfigPage_ConfigUpdate *)msg)->sourcePage;

  ENTER();

  switch(sourcePage)
  {
    case cp_TCPIP:
    {
      DoMethod(data->CY_IDENTITY_MAILSERVER, MUIM_MailServerChooser_UpdateMailServers);
    }
    break;

    case cp_Signature:
    {
      DoMethod(data->CY_IDENTITY_SIGNATURE, MUIM_SignatureChooser_UpdateSignatures);
    }
    break;

    default:
    {
      // ignore all other pages for the moment
    }
    break;
  }

  RETURN(0);
  return 0;
}

///
/// DECLARE(IdentityToGUI)
// fills form with data from selected list entry
DECLARE(IdentityToGUI)
{
  GETDATA;
  struct UserIdentityNode *uin = NULL;
  LONG pos = MUIV_NList_GetPos_Start;

  ENTER();

  // get the currently selected user identity
  DoMethod(data->LV_IDENTITY, MUIM_NList_GetEntry, MUIV_NList_GetEntry_Active, &uin);

  // make sure to disable GUI elements
  if(uin == NULL || xget(data->LV_IDENTITY, MUIA_NList_Entries) < 2)
    set(data->BT_IDEL, MUIA_Disabled, TRUE);
  else
    set(data->BT_IDEL, MUIA_Disabled, FALSE);

  if(uin != NULL)
    DoMethod(data->LV_IDENTITY, MUIM_NList_GetPos, uin, &pos);
  else
    pos = 0;

  set(data->BT_IDENTITYUP, MUIA_Disabled, pos == 0);
  set(data->BT_IDENTITYDOWN, MUIA_Disabled, pos == (LONG)xget(data->LV_IDENTITY, MUIA_NList_Entries) - 1);

  if(uin != NULL)
  {
    // all notifies here are nnset() notifies so that we don't trigger any additional
    // notify or otherwise we would run into problems.

    nnset(data->CH_IDENTITY_ENABLED,       MUIA_Selected,                      uin->active);
    nnset(data->ST_IDENTITY_DESCRIPTION,   MUIA_String_Contents,               uin->description);
    nnset(data->ST_IDENTITY_REALNAME,      MUIA_String_Contents,               uin->realname);
    nnset(data->ST_IDENTITY_EMAIL,         MUIA_String_Contents,               uin->address);
    nnset(data->ST_IDENTITY_ORGANIZATION,  MUIA_String_Contents,               uin->organization);
    nnset(data->ST_IDENTITY_CC,            MUIA_String_Contents,               uin->mailCC);
    nnset(data->ST_IDENTITY_BCC,           MUIA_String_Contents,               uin->mailBCC);
    nnset(data->ST_IDENTITY_REPLYTO,       MUIA_String_Contents,               uin->mailReplyTo);
    nnset(data->ST_IDENTITY_EXTRAHEADER,   MUIA_String_Contents,               uin->extraHeaders);
    nnset(data->ST_IDENTITY_PHOTOURL,      MUIA_String_Contents,               uin->photoURL);
    nnset(data->CH_IDENTITY_SENTFOLDER,    MUIA_Selected,                      uin->saveSentMail);
    nnset(data->PO_IDENTITY_SENTFOLDER,    MUIA_FolderRequestPopup_Folder,     uin->sentFolder);
    nnset(data->CH_IDENTITY_QUOTEMAILS,    MUIA_Selected,                      uin->quoteMails);
    nnset(data->CY_IDENTITY_QUOTEPOS,      MUIA_Cycle_Active,                  uin->quotePosition);
    nnset(data->CY_IDENTITY_SIGPOS,        MUIA_Cycle_Active,                  uin->signaturePosition);
    nnset(data->CH_IDENTITY_SIGREPLY,      MUIA_Selected,                      uin->sigReply);
    nnset(data->CH_IDENTITY_SIGFORWARD,    MUIA_Selected,                      uin->sigForwarding);
    nnset(data->CH_IDENTITY_ADDINFO,       MUIA_Selected,                      uin->addPersonalInfo);
    nnset(data->CH_IDENTITY_REQUESTMDN,    MUIA_Selected,                      uin->requestMDN);
    nnset(data->CH_IDENTITY_USEPGP,        MUIA_Selected,                      uin->usePGP);
    nnset(data->PO_IDENTITY_PGPID,         MUIA_PGPKeyPopup_PGPKey,            uin->pgpKeyID);
    nnset(data->ST_IDENTITY_PGPURL,        MUIA_String_Contents,               uin->pgpKeyURL);
    nnset(data->CH_IDENTITY_PGPSIGN_UNENC, MUIA_Selected,                      uin->pgpSignUnencrypted);
    nnset(data->CH_IDENTITY_PGPSIGN_ENC,   MUIA_Selected,                      uin->pgpSignEncrypted);
    nnset(data->CH_IDENTITY_PGPENC_ALL,    MUIA_Selected,                      uin->pgpEncryptAll);
    nnset(data->CH_IDENTITY_PGPENC_SELF,   MUIA_Selected,                      uin->pgpSelfEncrypt);

    // we have to set the correct mail server in the GUI so we browse through
    // the SMTP server list and match the ids
    if(uin->smtpServer != NULL)
    {
      struct MailServerNode *msn;

      // we match the ids because the pointers may be different
      if((msn = FindMailServer(&CE->smtpServerList, uin->smtpServer->id)) != NULL)
        nnset(data->CY_IDENTITY_MAILSERVER, MUIA_MailServerChooser_MailServer, msn);
    }

    // we have to set the correct signature in the GUI so we browse through
    // the signature list and match the ids
    if(uin->signature != NULL)
    {
      struct SignatureNode *sn;

      // we match the ids because the pointers may be different
      if((sn = FindSignatureByID(&CE->signatureList, uin->signature->id)) != NULL)
        nnset(data->CY_IDENTITY_SIGNATURE, MUIA_SignatureChooser_Signature, sn);
    }
    else
      nnset(data->CY_IDENTITY_SIGNATURE, MUIA_SignatureChooser_Signature, NULL);
  }

  set(data->CY_IDENTITY_QUOTEPOS, MUIA_Disabled, uin == NULL || uin->quoteMails == FALSE);
  set(data->CY_IDENTITY_SIGPOS, MUIA_Disabled, uin == NULL || uin->quotePosition == QPOS_BELOW || uin->quoteMails == FALSE);

  DoMethod(_win(obj), MUIM_MultiSet, MUIA_Disabled, uin == NULL || uin->usePGP == FALSE,
    data->PO_IDENTITY_PGPID,
    data->ST_IDENTITY_PGPURL,
    data->CH_IDENTITY_PGPSIGN_UNENC,
    data->CH_IDENTITY_PGPSIGN_ENC,
    data->CH_IDENTITY_PGPENC_ALL,
    data->CH_IDENTITY_PGPENC_SELF,
    NULL);

  DoMethod(_win(obj), MUIM_MultiSet, MUIA_Disabled, uin == NULL || uin->saveSentMail == FALSE,
    data->PO_IDENTITY_SENTFOLDER,
    NULL);

  RETURN(0);
  return 0;
}

///
/// DECLARE(GUIToIdentity)
// fills form data into selected list entry
DECLARE(GUIToIdentity)
{
  GETDATA;
  int p;

  ENTER();

  p = xget(data->LV_IDENTITY, MUIA_NList_Active);
  if(p != MUIV_NList_Active_Off)
  {
    struct UserIdentityNode *uin = NULL;

    DoMethod(data->LV_IDENTITY, MUIM_NList_GetEntry, p, &uin);
    if(uin != NULL)
    {
      uin->active = GetMUICheck(data->CH_IDENTITY_ENABLED);
      GetMUIString(uin->description,  data->ST_IDENTITY_DESCRIPTION, sizeof(uin->description));
      GetMUIString(uin->realname,     data->ST_IDENTITY_REALNAME,    sizeof(uin->realname));
      GetMUIString(uin->address,      data->ST_IDENTITY_EMAIL,       sizeof(uin->address));
      GetMUIString(uin->organization, data->ST_IDENTITY_ORGANIZATION,sizeof(uin->organization));
      uin->smtpServer = (struct MailServerNode *)xget(data->CY_IDENTITY_MAILSERVER, MUIA_MailServerChooser_MailServer);
      uin->signature = (struct SignatureNode *)xget(data->CY_IDENTITY_SIGNATURE, MUIA_SignatureChooser_Signature);
      GetMUIString(uin->mailCC,       data->ST_IDENTITY_CC,          sizeof(uin->mailCC));
      GetMUIString(uin->mailBCC,      data->ST_IDENTITY_BCC,         sizeof(uin->mailBCC));
      GetMUIString(uin->mailReplyTo,  data->ST_IDENTITY_REPLYTO,     sizeof(uin->mailReplyTo));
      GetMUIString(uin->extraHeaders, data->ST_IDENTITY_EXTRAHEADER, sizeof(uin->extraHeaders));
      GetMUIString(uin->photoURL,     data->ST_IDENTITY_PHOTOURL,    sizeof(uin->photoURL));
      strlcpy(uin->sentFolder, (char *)xget(data->PO_IDENTITY_SENTFOLDER, MUIA_FolderRequestPopup_Folder), sizeof(uin->sentFolder));
      uin->saveSentMail = GetMUICheck(data->CH_IDENTITY_SENTFOLDER);
      uin->quoteMails = GetMUICheck(data->CH_IDENTITY_QUOTEMAILS);
      uin->quotePosition = GetMUICycle(data->CY_IDENTITY_QUOTEPOS);
      uin->signaturePosition = GetMUICycle(data->CY_IDENTITY_SIGPOS);
      uin->sigReply = GetMUICheck(data->CH_IDENTITY_SIGREPLY);
      uin->sigForwarding = GetMUICheck(data->CH_IDENTITY_SIGFORWARD);
      uin->addPersonalInfo = GetMUICheck(data->CH_IDENTITY_ADDINFO);
      uin->requestMDN = GetMUICheck(data->CH_IDENTITY_REQUESTMDN);

      uin->usePGP = GetMUICheck(data->CH_IDENTITY_USEPGP);
      strlcpy(uin->pgpKeyID, (char *)xget(data->PO_IDENTITY_PGPID, MUIA_PGPKeyPopup_PGPKey), sizeof(uin->pgpKeyID));
      GetMUIString(uin->pgpKeyURL, data->ST_IDENTITY_PGPURL, sizeof(uin->pgpKeyURL));
      uin->pgpSignUnencrypted = GetMUICheck(data->CH_IDENTITY_PGPSIGN_UNENC);
      uin->pgpSignEncrypted = GetMUICheck(data->CH_IDENTITY_PGPSIGN_ENC);
      uin->pgpEncryptAll = GetMUICheck(data->CH_IDENTITY_PGPENC_ALL);
      uin->pgpSelfEncrypt = GetMUICheck(data->CH_IDENTITY_PGPENC_SELF);

      set(data->CY_IDENTITY_QUOTEPOS, MUIA_Disabled, uin->quoteMails == FALSE);
      set(data->CY_IDENTITY_SIGPOS, MUIA_Disabled, uin->quotePosition == QPOS_BELOW || uin->quoteMails == FALSE);

      DoMethod(_win(obj), MUIM_MultiSet, MUIA_Disabled, uin->usePGP == FALSE,
        data->PO_IDENTITY_PGPID,
        data->ST_IDENTITY_PGPURL,
        data->CH_IDENTITY_PGPSIGN_UNENC,
        data->CH_IDENTITY_PGPSIGN_ENC,
        data->CH_IDENTITY_PGPENC_ALL,
        data->CH_IDENTITY_PGPENC_SELF,
        NULL);

      DoMethod(_win(obj), MUIM_MultiSet, MUIA_Disabled, uin->saveSentMail == FALSE,
        data->PO_IDENTITY_SENTFOLDER,
        NULL);

      // if the user hasn't yet entered an own account name or the default
      // account name is still present we go and set an automatic generated one
      if(uin->description[0] == '\0' || strcmp(uin->description, tr(MSG_NewEntry)) == 0)
        strlcpy(uin->description, uin->address, sizeof(uin->description));

      // redraw the list
      DoMethod(data->LV_IDENTITY, MUIM_NList_Redraw, p);
    }
  }

  RETURN(0);
  return 0;
}

///
/// DECLARE(AddIdentityEntry)
// adds a new entry to the user identity list
DECLARE(AddIdentityEntry)
{
  GETDATA;
  struct UserIdentityNode *uin;

  ENTER();

  if((uin = CreateNewUserIdentity(CE)) != NULL)
  {
    if(IsMinListEmpty(&CE->userIdentityList) == FALSE)
      strlcpy(uin->description, tr(MSG_NewEntry), sizeof(uin->description));

    DoMethod(data->LV_IDENTITY, MUIM_NList_InsertSingle, uin, MUIV_NList_Insert_Bottom);

    // add the server to the list
    AddTail((struct List *)&CE->userIdentityList, (struct Node *)uin);

    // set the new entry active and make sure that the email gadget will be
    // set as the new active object of the window as that gadget will be used
    // to automatically set the account name.
    set(data->LV_IDENTITY, MUIA_NList_Active, MUIV_List_Active_Bottom);
    set(_win(obj), MUIA_Window_ActiveObject, data->ST_IDENTITY_EMAIL);
  }

  RETURN(0);
  return 0;
}

///
/// DECLARE(DeleteIdentityEntry)
// deletes an entry from the user identity list
DECLARE(DeleteIdentityEntry)
{
  GETDATA;
  struct UserIdentityNode *uin = NULL;

  ENTER();

  DoMethod(data->LV_IDENTITY, MUIM_NList_GetEntry, MUIV_NList_GetEntry_Active, &uin);

  if(uin != NULL &&
     xget(data->LV_IDENTITY, MUIA_NList_Entries) > 1)
  {
    DoMethod(data->LV_IDENTITY, MUIM_NList_Remove, MUIV_NList_Remove_Active);

    // remove it from the internal user identity list as well.
    Remove((struct Node *)uin);

    FreeSysObject(ASOT_NODE, uin);
  }

  RETURN(0);
  return 0;
}

///
