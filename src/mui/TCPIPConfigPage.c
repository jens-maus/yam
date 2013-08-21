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
 Description: "TCP/IP" configuration page

***************************************************************************/

#include "TCPIPConfigPage_cl.h"

#include <proto/asl.h>
#include <proto/muimaster.h>
#if defined(__amigaos4__)
#include <proto/application.h>
#endif

#include <mui/BetterString_mcc.h>
#include <mui/NBalance_mcc.h>
#include <mui/NList_mcc.h>
#include <mui/NListview_mcc.h>

#include "YAM.h"

#include "MailServers.h"
#include "UIDL.h"
#include "UserIdentity.h"

#include "mui/AccountList.h"
#include "mui/ConfigPage.h"

#include "Debug.h"

/* CLASSDATA
struct Data
{
  Object *LV_POP3;
  Object *BT_PADD;
  Object *BT_PDEL;
  Object *BT_POPUP;
  Object *BT_POPDOWN;
  Object *CH_POPENABLED;
  Object *ST_POPDESC;
  Object *ST_POPHOST;
  Object *ST_POPPORT;
  Object *CY_POPSECURE;
  Object *CY_POPAUTH;
  Object *ST_POPUSERID;
  Object *ST_PASSWD;
  Object *CY_PRESELECTION;
  Object *CH_DOWNLOADONSTARTUP;
  Object *CH_INTERVAL;
  Object *NM_INTERVAL;
  Object *CH_DLLARGE;
  Object *ST_WARNSIZE;
  Object *CH_DELETE;
  Object *CH_APPLYREMOTEFILTERS;
  Object *CH_POP3_NOTIFY_REQ;
  Object *CH_POP3_NOTIFY_SOUND;
  Object *PO_POP3_NOTIFY_SOUND;
  Object *ST_POP3_NOTIFY_SOUND;
  Object *BT_POP3_NOTIFY_SOUND;
  Object *CH_POP3_NOTIFY_CMD;
  Object *PO_POP3_NOTIFY_CMD;
  Object *ST_POP3_NOTIFY_CMD;
  Object *LV_SMTP;
  Object *BT_SADD;
  Object *BT_SDEL;
  Object *BT_SMTPUP;
  Object *BT_SMTPDOWN;
  Object *CH_SMTPENABLED;
  Object *ST_SMTPDESC;
  Object *ST_SMTPHOST;
  Object *ST_SMTPPORT;
  Object *CY_SMTPSECURE;
  Object *CY_SMTPAUTH;
  Object *ST_SMTPAUTHUSER;
  Object *ST_SMTPAUTHPASS;
  Object *CH_SMTP8BIT;
  #if defined(__amigaos4__)
  Object *CH_POP3_NOTIFY_OS41SYSTEM;
  #endif
};
*/

/* Overloaded Methods */
/// OVERLOAD(OM_NEW)
OVERLOAD(OM_NEW)
{
  static const char *secureSMTPMethods[4];
  static const char *securePOP3Methods[4];
  static const char *smtpAuthMethods[7];
  static const char *pop3AuthMethods[3];
  static const char *preselectionModes[5];
  static const char *rtitles[3];
  Object *LV_POP3;
  Object *BT_PADD;
  Object *BT_PDEL;
  Object *BT_POPUP;
  Object *BT_POPDOWN;
  Object *CH_POPENABLED;
  Object *ST_POPDESC;
  Object *ST_POPHOST;
  Object *ST_POPPORT;
  Object *CY_POPSECURE;
  Object *CY_POPAUTH;
  Object *ST_POPUSERID;
  Object *ST_PASSWD;
  Object *CY_PRESELECTION;
  Object *CH_DOWNLOADONSTARTUP;
  Object *CH_INTERVAL;
  Object *NM_INTERVAL;
  Object *CH_DLLARGE;
  Object *ST_WARNSIZE;
  Object *CH_DELETE;
  Object *CH_APPLYREMOTEFILTERS;
  Object *CH_POP3_NOTIFY_REQ;
  Object *CH_POP3_NOTIFY_SOUND;
  Object *PO_POP3_NOTIFY_SOUND;
  Object *ST_POP3_NOTIFY_SOUND;
  Object *BT_POP3_NOTIFY_SOUND;
  Object *CH_POP3_NOTIFY_CMD;
  Object *PO_POP3_NOTIFY_CMD;
  Object *ST_POP3_NOTIFY_CMD;
  Object *LV_SMTP;
  Object *BT_SADD;
  Object *BT_SDEL;
  Object *BT_SMTPUP;
  Object *BT_SMTPDOWN;
  Object *CH_SMTPENABLED;
  Object *ST_SMTPDESC;
  Object *ST_SMTPHOST;
  Object *ST_SMTPPORT;
  Object *CY_SMTPSECURE;
  Object *CY_SMTPAUTH;
  Object *ST_SMTPAUTHUSER;
  Object *ST_SMTPAUTHPASS;
  Object *CH_SMTP8BIT;
  #if defined(__amigaos4__)
  Object *CH_POP3_NOTIFY_OS41SYSTEM;
  #endif

  ENTER();

  secureSMTPMethods[0] = tr(MSG_CO_SMTP_SECURITY_NONE);
  secureSMTPMethods[1] = tr(MSG_CO_SMTP_SECURITY_TLS);
  secureSMTPMethods[2] = tr(MSG_CO_SMTP_SECURITY_SSL);
  secureSMTPMethods[3] = NULL;

  securePOP3Methods[0] = tr(MSG_CO_POP_SECURITY_NONE);
  securePOP3Methods[1] = tr(MSG_CO_POP_SECURITY_TLS);
  securePOP3Methods[2] = tr(MSG_CO_POP_SECURITY_SSL);
  securePOP3Methods[3] = NULL;

  smtpAuthMethods[0] = tr(MSG_CO_SMTPAUTH_NONE);
  smtpAuthMethods[1] = tr(MSG_CO_SMTPAUTH_AUTO);
  smtpAuthMethods[2] = tr(MSG_CO_SMTPAUTH_DIGEST);
  smtpAuthMethods[3] = tr(MSG_CO_SMTPAUTH_CRAM);
  smtpAuthMethods[4] = tr(MSG_CO_SMTPAUTH_LOGIN);
  smtpAuthMethods[5] = tr(MSG_CO_SMTPAUTH_PLAIN);
  smtpAuthMethods[6] = NULL;

  pop3AuthMethods[0] = tr(MSG_CO_POP_AUTH_PLAIN);
  pop3AuthMethods[1] = tr(MSG_CO_POP_AUTH_APOP);
  pop3AuthMethods[2] = NULL;

  preselectionModes[PSM_NEVER]       = tr(MSG_CO_PSNever);
  preselectionModes[PSM_LARGE]       = tr(MSG_CO_PSLarge);
  preselectionModes[PSM_ALWAYS]      = tr(MSG_CO_PSAlways);
  preselectionModes[PSM_ALWAYSLARGE] = tr(MSG_CO_PSAlwaysFast);
  preselectionModes[4] = NULL;

  rtitles[0] = tr(MSG_CO_ReceiveMail);
  rtitles[1] = tr(MSG_CO_SendMail);
  rtitles[2] = NULL;

  if((obj = DoSuperNew(cl, obj,
    MUIA_HelpNode, "Configuration#TCPIP",
    MUIA_ConfigPage_ImageID, "config_network_big",
    MUIA_ConfigPage_Image, CI_NETWORKBIG,
    MUIA_ConfigPage_Title, tr(MSG_CO_TCPIP_TITLE),
    MUIA_ConfigPage_Summary, tr(MSG_CO_TCPIP_SUMMARY),
    MUIA_ConfigPage_UseScrollgroup, FALSE,
    MUIA_ConfigPage_Contents, RegisterGroup(rtitles),
      MUIA_CycleChain, TRUE,

      // Receive MailServer List (POP3/IMAP)
      Child, ScrollgroupObject,
        MUIA_Scrollgroup_FreeHoriz, FALSE,
        MUIA_Scrollgroup_AutoBars, TRUE,
        MUIA_Scrollgroup_Contents, VGroupV,

          Child, HGroup,
            GroupSpacing(0),
            Child, VGroup,
              MUIA_HorizWeight, 30,

              Child, HBarT(tr(MSG_CO_POP_SERVERLIST)), End,

              Child, NListviewObject,
                MUIA_CycleChain, TRUE,
                MUIA_Weight,     60,
                MUIA_NListview_NList, LV_POP3 = AccountListObject,
                End,
              End,

              Child, HGroup,
                Child, ColGroup(2),
                  MUIA_Group_Spacing, 1,
                  MUIA_Group_SameWidth, TRUE,
                  MUIA_Weight, 1,
                  Child, BT_PADD = MakeButton(MUIX_B "+" MUIX_N),
                  Child, BT_PDEL = MakeButton(MUIX_B "-" MUIX_N),
                End,
                Child, HSpace(0),
                Child, ColGroup(2),
                  MUIA_Group_Spacing, 1,
                  MUIA_Group_SameWidth, TRUE,
                  Child, BT_POPUP = PopButton(MUII_ArrowUp),
                  Child, BT_POPDOWN = PopButton(MUII_ArrowDown),
                End,
              End,
            End,

            Child, NBalanceObject,
              MUIA_Balance_Quiet, TRUE,
            End,

            Child, VGroup,
              MUIA_HorizWeight, 70,

              Child, ColGroup(2), GroupFrameT(tr(MSG_CO_POP_SERVERSETTINGS)),

                Child, HSpace(1),
                Child, MakeCheckGroup(&CH_POPENABLED, tr(MSG_CO_POPActive)),

                Child, Label2(tr(MSG_CO_POP_DESC)),
                Child, ST_POPDESC = MakeString(SIZE_DEFAULT, tr(MSG_CO_POP_DESC)),

                Child, Label2(tr(MSG_CO_POP_SERVERPORT)),
                Child, HGroup,
                  MUIA_Group_Spacing, 1,
                  Child, ST_POPHOST = MakeString(SIZE_HOST, tr(MSG_CO_POP_SERVERPORT)),
                  Child, ST_POPPORT = BetterStringObject,
                    StringFrame,
                    MUIA_CycleChain,          TRUE,
                    MUIA_FixWidthTxt,         "00000",
                    MUIA_String_MaxLen,       5+1,
                    MUIA_String_AdvanceOnCR,  TRUE,
                    MUIA_String_Integer,      0,
                    MUIA_String_Accept,       "0123456789",
                  End,
                End,

                Child, HSpace(1),
                Child, HBarT(tr(MSG_CO_POP_SECURITYAUTH)), End,

                Child, Label2(tr(MSG_CO_POP_SECURITY)),
                Child, CY_POPSECURE = MakeCycle(securePOP3Methods, tr(MSG_CO_POP_SECURITY)),

                Child, Label2(tr(MSG_CO_POP_AUTH)),
                Child, CY_POPAUTH = MakeCycle(pop3AuthMethods, tr(MSG_CO_POP_AUTH)),

                Child, Label2(tr(MSG_CO_POPUserID)),
                Child, ST_POPUSERID = MakeString(SIZE_USERID,tr(MSG_CO_POPUserID)),

                Child, Label2(tr(MSG_CO_Password)),
                Child, ST_PASSWD = MakePassString(tr(MSG_CO_Password)),

                Child, HSpace(1),
                Child, HBarT(tr(MSG_CO_POP_MSGHANDLING)), End,

                Child, Label2(tr(MSG_CO_PreSelect)),
                Child, CY_PRESELECTION = MakeCycle(preselectionModes, tr(MSG_CO_PreSelect)),

                Child, HSpace(1),
                Child, MakeCheckGroup(&CH_DOWNLOADONSTARTUP, tr(MSG_CO_DOWNLOAD_ON_STARTUP)),

                Child, HSpace(1),
                Child, HGroup,
                  Child, CH_INTERVAL = MakeCheck(tr(MSG_CO_CheckMail)),
                  Child, Label2(tr(MSG_CO_CheckMail)),
                  Child, NM_INTERVAL = NumericbuttonObject,
                    MUIA_CycleChain,      TRUE,
                    MUIA_Numeric_Min,     1,
                    MUIA_Numeric_Max,     240,
                    MUIA_Numeric_Default, 5,
                  End,
                  Child, Label2(tr(MSG_CO_Minutes)),
                  Child, HSpace(0),
                End,

                Child, HSpace(1),
                Child, HGroup,
                  Child, MakeCheckGroup(&CH_DLLARGE, tr(MSG_CO_DOWNLOAD_LARGE_MAILS1)),
                  Child, ST_WARNSIZE = MakeInteger(5, tr(MSG_CO_DOWNLOAD_LARGE_MAILS1)),
                  Child, LLabel(tr(MSG_CO_DOWNLOAD_LARGE_MAILS2)),
                  Child, HSpace(0),
                End,

                Child, HSpace(1),
                Child, MakeCheckGroup(&CH_DELETE, tr(MSG_CO_DeleteServerMail)),

                Child, HSpace(1),
                Child, MakeCheckGroup(&CH_APPLYREMOTEFILTERS, tr(MSG_CO_APPLY_REMOTE_FILTERS)),

                Child, HSpace(1),
                Child, HBarT(tr(MSG_CO_POP_NOTIFICATION)), End,

                Child, HSpace(1),
                Child, HGroup,
                  Child, CH_POP3_NOTIFY_REQ = MakeCheck(tr(MSG_CO_NotiReq)),
                  Child, LLabel(tr(MSG_CO_NotiReq)),
                  Child, HSpace(0),
                End,

                #if defined(__amigaos4__)
                Child, HSpace(1),
                Child, HGroup,
                  Child, CH_POP3_NOTIFY_OS41SYSTEM = MakeCheck(tr(MSG_CO_NOTIOS41SYSTEM)),
                  Child, LLabel(tr(MSG_CO_NOTIOS41SYSTEM)),
                  Child, HSpace(0),
                End,
                #endif // __amigaos4__

                Child, HSpace(1),
                Child, HGroup,
                  Child, CH_POP3_NOTIFY_SOUND = MakeCheck(tr(MSG_CO_NotiSound)),
                  Child, LLabel(tr(MSG_CO_NotiSound)),
                  Child, HGroup,
                    MUIA_Group_HorizSpacing, 0,
                    Child, PO_POP3_NOTIFY_SOUND = PopaslObject,
                      MUIA_Popasl_Type, ASL_FileRequest,
                      MUIA_Popstring_String, ST_POP3_NOTIFY_SOUND = MakeString(SIZE_PATHFILE, ""),
                      MUIA_Popstring_Button, PopButton(MUII_PopFile),
                    End,
                    Child, BT_POP3_NOTIFY_SOUND = PopButton(MUII_TapePlay),
                  End,
                End,

                Child, HSpace(1),
                Child, HGroup,
                  Child, CH_POP3_NOTIFY_CMD = MakeCheck(tr(MSG_CO_NotiCommand)),
                  Child, LLabel(tr(MSG_CO_NotiCommand)),
                  Child, PO_POP3_NOTIFY_CMD = PopaslObject,
                    MUIA_Popasl_Type, ASL_FileRequest,
                    MUIA_Popstring_String, ST_POP3_NOTIFY_CMD = MakeString(SIZE_COMMAND, ""),
                    MUIA_Popstring_Button, PopButton(MUII_PopFile),
                  End,
                End,

                Child, HVSpace,
                Child, HVSpace,

              End,
            End,
          End,
        End,
      End,

      // Send MailServer List (SMTP)
      Child, ScrollgroupObject,
        MUIA_Scrollgroup_FreeHoriz, FALSE,
        MUIA_Scrollgroup_AutoBars, TRUE,
        MUIA_Scrollgroup_Contents, VGroupV,

          Child, HGroup,
            GroupSpacing(0),
            Child, VGroup,
              MUIA_HorizWeight, 30,

              Child, HBarT(tr(MSG_CO_SMTP_SERVERLIST)), End,

              Child, NListviewObject,
                MUIA_CycleChain, TRUE,
                MUIA_Weight,     60,
                MUIA_NListview_NList, LV_SMTP = AccountListObject,
                End,
              End,

              Child, HGroup,
                Child, ColGroup(2),
                  MUIA_Group_Spacing, 1,
                  MUIA_Group_SameWidth, TRUE,
                  MUIA_Weight, 1,
                  Child, BT_SADD = MakeButton(MUIX_B "+" MUIX_N),
                  Child, BT_SDEL = MakeButton(MUIX_B "-" MUIX_N),
                End,
                Child, HSpace(0),
                Child, ColGroup(2),
                  MUIA_Group_Spacing, 1,
                  MUIA_Group_SameWidth, TRUE,
                  Child, BT_SMTPUP = PopButton(MUII_ArrowUp),
                  Child, BT_SMTPDOWN = PopButton(MUII_ArrowDown),
                End,
              End,
            End,

            Child, NBalanceObject,
              MUIA_Balance_Quiet, TRUE,
            End,

            Child, VGroup,
              MUIA_HorizWeight, 70,

              Child, ColGroup(2), GroupFrameT(tr(MSG_CO_SMTP_SERVERSETTINGS)),

                Child, HSpace(1),
                Child, MakeCheckGroup(&CH_SMTPENABLED, tr(MSG_CO_SMTP_ACTIVE)),

                Child, Label2(tr(MSG_CO_SMTP_DESC)),
                Child, ST_SMTPDESC = MakeString(SIZE_DEFAULT, tr(MSG_CO_SMTP_DESC)),

                Child, Label2(tr(MSG_CO_SMTP_SERVERPORT)),
                Child, HGroup,
                  MUIA_Group_Spacing, 1,
                  Child, ST_SMTPHOST = MakeString(SIZE_HOST, tr(MSG_CO_SMTP_SERVERPORT)),
                  Child, ST_SMTPPORT = BetterStringObject,
                    StringFrame,
                    MUIA_CycleChain,          TRUE,
                    MUIA_FixWidthTxt,         "00000",
                    MUIA_String_MaxLen,       5+1,
                    MUIA_String_AdvanceOnCR,  TRUE,
                    MUIA_String_Integer,      0,
                    MUIA_String_Accept,       "0123456789",
                  End,
                End,

                Child, HSpace(1),
                Child, HBarT(tr(MSG_CO_SMTP_SECURITYAUTH)), End,

                Child, Label2(tr(MSG_CO_SMTP_SECURITY)),
                Child, CY_SMTPSECURE = MakeCycle(secureSMTPMethods, tr(MSG_CO_SMTP_SECURITY)),

                Child, Label2(tr(MSG_CO_SMTP_AUTH)),
                Child, CY_SMTPAUTH = MakeCycle(smtpAuthMethods, tr(MSG_CO_SMTP_AUTH)),

                Child, Label2(tr(MSG_CO_SMTPUser)),
                Child, ST_SMTPAUTHUSER = MakeString(SIZE_USERID,tr(MSG_CO_SMTPUser)),

                Child, Label2(tr(MSG_CO_SMTPPass)),
                Child, ST_SMTPAUTHPASS = MakePassString(tr(MSG_CO_SMTPPass)),

                Child, HSpace(1),
                Child, HBarT(tr(MSG_CO_SMTP_OPTIONS)), End,

                Child, HSpace(1),
                Child, HGroup,
                  Child, CH_SMTP8BIT = MakeCheck(tr(MSG_CO_Allow8bit)),
                  Child, LLabel1(tr(MSG_CO_Allow8bit)),
                  Child, HSpace(0),
                End,

                Child, HVSpace,
                Child, HVSpace,

              End,
            End,
          End,
        End,
      End,

    End,
  TAG_MORE, inittags(msg))) != NULL)
  {
    GETDATA;

    data->LV_POP3 =                   LV_POP3;
    data->BT_PADD =                   BT_PADD;
    data->BT_PDEL =                   BT_PDEL;
    data->BT_POPUP =                  BT_POPUP;
    data->BT_POPDOWN =                BT_POPDOWN;
    data->CH_POPENABLED =             CH_POPENABLED;
    data->ST_POPDESC =                ST_POPDESC;
    data->ST_POPHOST =                ST_POPHOST;
    data->ST_POPPORT =                ST_POPPORT;
    data->CY_POPSECURE =              CY_POPSECURE;
    data->CY_POPAUTH =                CY_POPAUTH;
    data->ST_POPUSERID =              ST_POPUSERID;
    data->ST_PASSWD =                 ST_PASSWD;
    data->CY_PRESELECTION =           CY_PRESELECTION;
    data->CH_DOWNLOADONSTARTUP =      CH_DOWNLOADONSTARTUP;
    data->CH_INTERVAL =               CH_INTERVAL;
    data->NM_INTERVAL =               NM_INTERVAL;
    data->CH_DLLARGE =                CH_DLLARGE;
    data->ST_WARNSIZE =               ST_WARNSIZE;
    data->CH_DELETE =                 CH_DELETE;
    data->CH_APPLYREMOTEFILTERS =     CH_APPLYREMOTEFILTERS;
    data->CH_POP3_NOTIFY_REQ =        CH_POP3_NOTIFY_REQ;
    data->CH_POP3_NOTIFY_SOUND =      CH_POP3_NOTIFY_SOUND;
    data->PO_POP3_NOTIFY_SOUND =      PO_POP3_NOTIFY_SOUND;
    data->ST_POP3_NOTIFY_SOUND =      ST_POP3_NOTIFY_SOUND;
    data->BT_POP3_NOTIFY_SOUND =      BT_POP3_NOTIFY_SOUND;
    data->CH_POP3_NOTIFY_CMD =        CH_POP3_NOTIFY_CMD;
    data->PO_POP3_NOTIFY_CMD =        PO_POP3_NOTIFY_CMD;
    data->ST_POP3_NOTIFY_CMD =        ST_POP3_NOTIFY_CMD;
    data->LV_SMTP =                   LV_SMTP;
    data->BT_SADD =                   BT_SADD;
    data->BT_SDEL =                   BT_SDEL;
    data->BT_SMTPUP =                 BT_SMTPUP;
    data->BT_SMTPDOWN =               BT_SMTPDOWN;
    data->CH_SMTPENABLED =            CH_SMTPENABLED;
    data->ST_SMTPDESC =               ST_SMTPDESC;
    data->ST_SMTPHOST =               ST_SMTPHOST;
    data->ST_SMTPPORT =               ST_SMTPPORT;
    data->CY_SMTPSECURE =             CY_SMTPSECURE;
    data->CY_SMTPAUTH =               CY_SMTPAUTH;
    data->ST_SMTPAUTHUSER =           ST_SMTPAUTHUSER;
    data->ST_SMTPAUTHPASS =           ST_SMTPAUTHPASS;
    data->CH_SMTP8BIT =               CH_SMTP8BIT;
    #if defined(__amigaos4__)
    data->CH_POP3_NOTIFY_OS41SYSTEM = CH_POP3_NOTIFY_OS41SYSTEM;
    #endif

    SetHelp(ST_SMTPHOST,               MSG_HELP_CO_ST_SMTPHOST);
    SetHelp(ST_SMTPPORT,               MSG_HELP_CO_ST_SMTPPORT);
    SetHelp(CH_SMTP8BIT,               MSG_HELP_CO_CH_SMTP8BIT);
    SetHelp(ST_SMTPAUTHUSER,           MSG_HELP_CO_ST_SMTPAUTHUSER);
    SetHelp(ST_SMTPAUTHPASS,           MSG_HELP_CO_ST_SMTPAUTHPASS);
    SetHelp(CY_SMTPAUTH,               MSG_HELP_CO_CY_SMTPAUTHMETHOD);
    SetHelp(LV_POP3,                   MSG_HELP_CO_LV_POP3);
    SetHelp(BT_PADD,                   MSG_HELP_CO_BT_PADD);
    SetHelp(BT_PDEL,                   MSG_HELP_CO_BT_PDEL);
    SetHelp(ST_POPDESC,                MSG_HELP_CO_ST_POPDESC);
    SetHelp(ST_SMTPDESC,               MSG_HELP_CO_ST_SMTPDESC);
    SetHelp(ST_POPHOST,                MSG_HELP_CO_ST_POPHOST);
    SetHelp(ST_POPPORT,                MSG_HELP_CO_ST_POPPORT);
    SetHelp(ST_POPUSERID,              MSG_HELP_CO_ST_POPUSERID);
    SetHelp(ST_PASSWD,                 MSG_HELP_CO_ST_PASSWD);
    SetHelp(CH_DELETE,                 MSG_HELP_CO_CH_DELETE);
    SetHelp(CY_POPAUTH,                MSG_HELP_CO_CY_POPAUTH);
    SetHelp(CH_POPENABLED,             MSG_HELP_CO_CH_POPENABLED);
    SetHelp(CH_DOWNLOADONSTARTUP,      MSG_HELP_CO_CH_DOWNLOAD_ON_STARTUP );
    SetHelp(CH_INTERVAL,               MSG_HELP_CO_ST_INTERVAL);
    SetHelp(NM_INTERVAL,               MSG_HELP_CO_ST_INTERVAL);
    SetHelp(CH_DLLARGE,                MSG_HELP_CO_CH_DLLARGE);
    SetHelp(ST_WARNSIZE,               MSG_HELP_CO_ST_WARNSIZE);
    SetHelp(CH_APPLYREMOTEFILTERS,     MSG_HELP_CO_CH_APPLY_REMOTE_FILTERS );
    SetHelp(CY_SMTPSECURE,             MSG_HELP_CO_CY_SMTPSECURE);
    SetHelp(CY_POPSECURE,              MSG_HELP_CO_CY_POPSECURE);
    SetHelp(CY_PRESELECTION,           MSG_HELP_CO_CY_MSGSELECT);
    SetHelp(CH_POP3_NOTIFY_REQ,        MSG_HELP_CO_CH_NOTIREQ);
    SetHelp(CH_POP3_NOTIFY_SOUND,      MSG_HELP_CO_CH_NOTISOUND);
    SetHelp(CH_POP3_NOTIFY_CMD,        MSG_HELP_CO_CH_NOTICMD);
    SetHelp(ST_POP3_NOTIFY_CMD,        MSG_HELP_CO_ST_NOTICMD);
    SetHelp(ST_POP3_NOTIFY_SOUND,      MSG_HELP_CO_ST_NOTISOUND);
    #if defined(__amigaos4__)
    SetHelp(CH_POP3_NOTIFY_OS41SYSTEM, MSG_HELP_CO_CH_NOTIOS41SYSTEM);
    #endif // __amigaos4__

    DoMethod(LV_POP3,                   MUIM_Notify, MUIA_NList_Active,     MUIV_EveryTime, obj, 1, METHOD(GetPOP3Entry));
    DoMethod(ST_POPDESC,                MUIM_Notify, MUIA_String_Contents,  MUIV_EveryTime, obj, 1, METHOD(PutPOP3Entry));
    DoMethod(ST_POPHOST,                MUIM_Notify, MUIA_String_Contents,  MUIV_EveryTime, obj, 1, METHOD(PutPOP3Entry));
    DoMethod(ST_POPPORT,                MUIM_Notify, MUIA_String_Contents,  MUIV_EveryTime, obj, 1, METHOD(PutPOP3Entry));
    DoMethod(ST_POPUSERID,              MUIM_Notify, MUIA_String_Contents,  MUIV_EveryTime, obj, 1, METHOD(PutPOP3Entry));
    DoMethod(ST_PASSWD,                 MUIM_Notify, MUIA_String_Contents,  MUIV_EveryTime, obj, 1, METHOD(PutPOP3Entry));
    DoMethod(CH_POPENABLED,             MUIM_Notify, MUIA_Selected,         MUIV_EveryTime, obj, 1, METHOD(PutPOP3Entry));
    DoMethod(CY_POPAUTH,                MUIM_Notify, MUIA_Cycle_Active,     MUIV_EveryTime, obj, 1, METHOD(PutPOP3Entry));
    DoMethod(CH_DELETE,                 MUIM_Notify, MUIA_Selected,         MUIV_EveryTime, obj, 1, METHOD(PutPOP3Entry));
    DoMethod(CH_DOWNLOADONSTARTUP,      MUIM_Notify, MUIA_Selected,         MUIV_EveryTime, obj, 1, METHOD(PutPOP3Entry));
    DoMethod(CH_INTERVAL,               MUIM_Notify, MUIA_Selected,         MUIV_EveryTime, obj, 1, METHOD(PutPOP3Entry));
    DoMethod(NM_INTERVAL,               MUIM_Notify, MUIA_Numeric_Value,    MUIV_EveryTime, obj, 1, METHOD(PutPOP3Entry));
    DoMethod(CH_DLLARGE,                MUIM_Notify, MUIA_Selected,         MUIV_EveryTime, obj, 1, METHOD(PutPOP3Entry));
    DoMethod(ST_WARNSIZE,               MUIM_Notify, MUIA_String_Contents,  MUIV_EveryTime, obj, 1, METHOD(PutPOP3Entry));
    DoMethod(CH_APPLYREMOTEFILTERS,     MUIM_Notify, MUIA_Selected,         MUIV_EveryTime, obj, 1, METHOD(PutPOP3Entry));
    DoMethod(CY_PRESELECTION,           MUIM_Notify, MUIA_Cycle_Active,     MUIV_EveryTime, obj, 1, METHOD(PutPOP3Entry));
    DoMethod(BT_PADD,                   MUIM_Notify, MUIA_Pressed,          FALSE,          obj, 1, METHOD(AddPOP3Entry));
    DoMethod(BT_PDEL,                   MUIM_Notify, MUIA_Pressed,          FALSE,          obj, 1, METHOD(DeletePOP3Entry));
    DoMethod(BT_POPUP,                  MUIM_Notify, MUIA_Pressed,          FALSE,          LV_POP3, 3, MUIM_NList_Move, MUIV_NList_Move_Selected, MUIV_NList_Move_Previous);
    DoMethod(BT_POPDOWN,                MUIM_Notify, MUIA_Pressed,          FALSE,          LV_POP3, 3, MUIM_NList_Move, MUIV_NList_Move_Selected, MUIV_NList_Move_Next);
    DoMethod(CY_POPSECURE,              MUIM_Notify, MUIA_Cycle_Active,     MUIV_EveryTime, obj, 1, METHOD(PutPOP3Entry));
    DoMethod(CH_POP3_NOTIFY_REQ,        MUIM_Notify, MUIA_Selected,         MUIV_EveryTime, obj, 1, METHOD(PutPOP3Entry));
    #if defined(__amigaos4__)
    DoMethod(CH_POP3_NOTIFY_OS41SYSTEM, MUIM_Notify, MUIA_Selected,         MUIV_EveryTime, obj, 1, METHOD(PutPOP3Entry));
    #endif // __amigaos4__
    DoMethod(CH_POP3_NOTIFY_CMD,        MUIM_Notify, MUIA_Selected,         MUIV_EveryTime, obj, 1, METHOD(PutPOP3Entry));
    DoMethod(ST_POP3_NOTIFY_CMD,        MUIM_Notify, MUIA_String_Contents,  MUIV_EveryTime, obj, 1, METHOD(PutPOP3Entry));
    DoMethod(CH_POP3_NOTIFY_SOUND,      MUIM_Notify, MUIA_Selected,         MUIV_EveryTime, obj, 1, METHOD(PutPOP3Entry));
    DoMethod(ST_POP3_NOTIFY_SOUND,      MUIM_Notify, MUIA_String_Contents,  MUIV_EveryTime, obj, 1, METHOD(PutPOP3Entry));

    // connect SMTP related stuff to the corresponding Hooks
    DoMethod(LV_SMTP              , MUIM_Notify, MUIA_NList_Active,     MUIV_EveryTime, obj, 1, METHOD(GetSMTPEntry));
    DoMethod(ST_SMTPDESC          , MUIM_Notify, MUIA_String_Contents,  MUIV_EveryTime, obj, 1, METHOD(PutSMTPEntry));
    DoMethod(ST_SMTPHOST          , MUIM_Notify, MUIA_String_Contents,  MUIV_EveryTime, obj, 1, METHOD(PutSMTPEntry));
    DoMethod(ST_SMTPPORT          , MUIM_Notify, MUIA_String_Contents,  MUIV_EveryTime, obj, 1, METHOD(PutSMTPEntry));
    DoMethod(ST_SMTPAUTHUSER      , MUIM_Notify, MUIA_String_Contents,  MUIV_EveryTime, obj, 1, METHOD(PutSMTPEntry));
    DoMethod(ST_SMTPAUTHPASS      , MUIM_Notify, MUIA_String_Contents,  MUIV_EveryTime, obj, 1, METHOD(PutSMTPEntry));
    DoMethod(CH_SMTPENABLED       , MUIM_Notify, MUIA_Selected,         MUIV_EveryTime, obj, 1, METHOD(PutSMTPEntry));
    DoMethod(CY_SMTPAUTH          , MUIM_Notify, MUIA_Cycle_Active,     MUIV_EveryTime, obj, 1, METHOD(PutSMTPEntry));
    DoMethod(CH_SMTP8BIT,           MUIM_Notify, MUIA_Selected,         MUIV_EveryTime, obj, 1, METHOD(PutSMTPEntry));
    DoMethod(BT_SADD              , MUIM_Notify, MUIA_Pressed,          FALSE,          obj, 1, METHOD(AddSMTPEntry));
    DoMethod(BT_SDEL              , MUIM_Notify, MUIA_Pressed,          FALSE,          obj, 1, METHOD(DeleteSMTPEntry));
    DoMethod(BT_SMTPUP            , MUIM_Notify, MUIA_Pressed,          FALSE,          LV_SMTP, 3, MUIM_NList_Move, MUIV_NList_Move_Selected, MUIV_NList_Move_Previous);
    DoMethod(BT_SMTPDOWN          , MUIM_Notify, MUIA_Pressed,          FALSE,          LV_SMTP, 3, MUIM_NList_Move, MUIV_NList_Move_Selected, MUIV_NList_Move_Next);
    DoMethod(CY_SMTPSECURE,         MUIM_Notify, MUIA_Cycle_Active,     MUIV_EveryTime, obj, 1, METHOD(PutSMTPEntry));

    // set some additional cyclechain data
    set(BT_POPUP,    MUIA_CycleChain, TRUE);
    set(BT_POPDOWN,  MUIA_CycleChain, TRUE);
    set(BT_SMTPUP,   MUIA_CycleChain, TRUE);
    set(BT_SMTPDOWN, MUIA_CycleChain, TRUE);

    set(BT_POP3_NOTIFY_SOUND, MUIA_CycleChain, TRUE);
    DoMethod(BT_POP3_NOTIFY_SOUND, MUIM_Notify, MUIA_Pressed, FALSE, MUIV_Notify_Application, 3, MUIM_CallHook, &CO_PlaySoundHook, ST_POP3_NOTIFY_SOUND);
  }

  RETURN((IPTR)obj);
  return (IPTR)obj;
}

///
/// OVERLOAD(MUIM_ConfigPage_ConfigToGUI)
OVERLOAD(MUIM_ConfigPage_ConfigToGUI)
{
  GETDATA;
  int numPOP = 0;
  int numSMTP = 0;
  struct MailServerNode *msn;

  ENTER();

  // clear the lists first
  set(data->LV_POP3, MUIA_NList_Quiet, TRUE);
  DoMethod(data->LV_POP3, MUIM_NList_Clear);
  set(data->LV_SMTP, MUIA_NList_Quiet, TRUE);
  DoMethod(data->LV_SMTP, MUIM_NList_Clear);

  // we iterate through our mail server list and make sure to populate
  // out NList object correctly.
  IterateList(&CE->pop3ServerList, struct MailServerNode *, msn)
  {
    DoMethod(data->LV_POP3, MUIM_NList_InsertSingle, msn, MUIV_NList_Insert_Bottom);
    numPOP++;
  }
  IterateList(&CE->smtpServerList, struct MailServerNode *, msn)
  {
    DoMethod(data->LV_SMTP, MUIM_NList_InsertSingle, msn, MUIV_NList_Insert_Bottom);
    numSMTP++;
  }

  // make sure the first entry is selected per default
  xset(data->LV_POP3, MUIA_NList_Quiet, FALSE,
                      MUIA_NList_Active, MUIV_NList_Active_Top);
  xset(data->LV_SMTP, MUIA_NList_Quiet, FALSE,
                      MUIA_NList_Active, MUIV_NList_Active_Top);

  // set the enabled stated of the del button according to the number of available accounts
  set(data->BT_PDEL, MUIA_Disabled, numPOP < 2);
  set(data->BT_SDEL, MUIA_Disabled, numSMTP < 2);

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
  SortNListToExecList(data->LV_POP3, &CE->pop3ServerList);
  SortNListToExecList(data->LV_SMTP, &CE->smtpServerList);

  RETURN(0);
  return 0;
}

///
/// DECLARE(GetPOP3Entry)
// fills form with data from selected list entry
DECLARE(GetPOP3Entry)
{
  GETDATA;
  struct MailServerNode *msn = NULL;
  LONG pos = MUIV_NList_GetPos_Start;

  ENTER();

  // get the currently selected POP3 server
  DoMethod(data->LV_POP3, MUIM_NList_GetEntry, MUIV_NList_GetEntry_Active, &msn);

  // make sure to disable GUI elements
  if(msn == NULL || xget(data->LV_POP3, MUIA_NList_Entries) < 2)
    set(data->BT_PDEL, MUIA_Disabled, TRUE);
  else
    set(data->BT_PDEL, MUIA_Disabled, FALSE);

  if(msn != NULL)
    DoMethod(data->LV_POP3, MUIM_NList_GetPos, msn, &pos);
  else
    pos = 0;

  set(data->BT_POPUP, MUIA_Disabled, pos == 0);
  set(data->BT_POPDOWN, MUIA_Disabled, pos == (LONG)xget(data->LV_POP3, MUIA_NList_Entries) - 1);

  if(msn != NULL)
  {
    // all notifies here are nnset() notifies so that we don't trigger any additional
    // notify or otherwise we would run into problems.
    nnset(data->CH_POPENABLED,             MUIA_Selected,        isServerActive(msn));
    nnset(data->ST_POPDESC,                MUIA_String_Contents, msn->description);
    nnset(data->ST_POPHOST,                MUIA_String_Contents, msn->hostname);
    nnset(data->ST_POPPORT,                MUIA_String_Integer,  msn->port);
    nnset(data->ST_POPUSERID,              MUIA_String_Contents, msn->username);
    nnset(data->ST_PASSWD,                 MUIA_String_Contents, msn->password);
    nnset(data->CH_DOWNLOADONSTARTUP,      MUIA_Selected,        hasServerDownloadOnStartup(msn));
    nnset(data->CH_INTERVAL,               MUIA_Selected,        hasServerDownloadPeriodically(msn));
    nnset(data->NM_INTERVAL,               MUIA_Numeric_Value,   msn->downloadInterval);
    nnset(data->CH_DLLARGE,                MUIA_Selected,        hasServerDownloadLargeMails(msn));
    nnset(data->ST_WARNSIZE,               MUIA_String_Integer,  msn->largeMailSizeLimit);
    nnset(data->CH_APPLYREMOTEFILTERS,     MUIA_Selected,        hasServerApplyRemoteFilters(msn));
    nnset(data->CH_DELETE,                 MUIA_Selected,        hasServerPurge(msn));
    nnset(data->CY_PRESELECTION,           MUIA_Cycle_Active,    msn->preselection);
    nnset(data->CH_POP3_NOTIFY_REQ,        MUIA_Selected,        msn->notifyByRequester);
    #if defined(__amigaos4__)
    nnset(data->CH_POP3_NOTIFY_OS41SYSTEM, MUIA_Selected,        msn->notifyByOS41System);
    #endif // __amigaos4__
    nnset(data->CH_POP3_NOTIFY_SOUND,      MUIA_Selected,        msn->notifyBySound);
    nnset(data->CH_POP3_NOTIFY_CMD,        MUIA_Selected,        msn->notifyByCommand);
    nnset(data->ST_POP3_NOTIFY_SOUND,      MUIA_String_Contents, msn->notifySound);
    nnset(data->ST_POP3_NOTIFY_CMD,        MUIA_String_Contents, msn->notifyCommand);

    set(data->NM_INTERVAL, MUIA_Disabled, hasServerDownloadPeriodically(msn) == FALSE);
    set(data->ST_WARNSIZE, MUIA_Disabled, hasServerDownloadLargeMails(msn) == FALSE);

    #if defined(__amigaos4__)
    set(data->CH_POP3_NOTIFY_OS41SYSTEM, MUIA_Disabled, G->applicationID == 0 || LIB_VERSION_IS_AT_LEAST(ApplicationBase, 53, 2) == FALSE);
    #endif // __amigaos4__

    set(data->PO_POP3_NOTIFY_SOUND, MUIA_Disabled, msn == NULL || msn->notifyBySound == FALSE);
    set(data->BT_POP3_NOTIFY_SOUND, MUIA_Disabled, msn == NULL || msn->notifyBySound == FALSE);
    set(data->PO_POP3_NOTIFY_CMD, MUIA_Disabled, msn == NULL || msn->notifyByCommand == FALSE);

    if(hasServerAPOP(msn))
      nnset(data->CY_POPAUTH, MUIA_Cycle_Active, 1);
    else
      nnset(data->CY_POPAUTH, MUIA_Cycle_Active, 0);

    if(hasServerTLS(msn))
      nnset(data->CY_POPSECURE, MUIA_Cycle_Active, 1);
    else if(hasServerSSL(msn))
      nnset(data->CY_POPSECURE, MUIA_Cycle_Active, 2);
    else
      nnset(data->CY_POPSECURE, MUIA_Cycle_Active, 0);

    // we have to enabled/disable the SSL support accordingly
    set(data->CY_POPSECURE, MUIA_Disabled, G->TR_UseableTLS == FALSE);
  }

  RETURN(0);
  return 0;
}

///
/// DECLARE(PutPOP3Entry)
// fills form data into selected list entry
DECLARE(PutPOP3Entry)
{
  GETDATA;
  int p;

  ENTER();

  p = xget(data->LV_POP3, MUIA_NList_Active);
  if(p != MUIV_NList_Active_Off)
  {
    struct MailServerNode *msn = NULL;

    DoMethod(data->LV_POP3, MUIM_NList_GetEntry, p, &msn);
    if(msn != NULL)
    {
      unsigned int oldSSLFlags = 0;

      GetMUIString(msn->description,  data->ST_POPDESC,    sizeof(msn->description));
      GetMUIString(msn->hostname,     data->ST_POPHOST,    sizeof(msn->hostname));
      GetMUIString(msn->username,     data->ST_POPUSERID,  sizeof(msn->username));
      GetMUIString(msn->password,     data->ST_PASSWD,     sizeof(msn->password));
      msn->preselection = GetMUICycle(data->CY_PRESELECTION);

      if(GetMUICheck(data->CH_POPENABLED) == TRUE)
        setFlag(msn->flags, MSF_ACTIVE);
      else
        clearFlag(msn->flags, MSF_ACTIVE);

      switch(GetMUICycle(data->CY_POPAUTH))
      {
        case 1:
          setFlag(msn->flags, MSF_APOP);
        break;

        default:
          clearFlag(msn->flags, MSF_APOP);
        break;
      }

      if(GetMUICheck(data->CH_DOWNLOADONSTARTUP) == TRUE)
        setFlag(msn->flags, MSF_DOWNLOAD_ON_STARTUP);
      else
        clearFlag(msn->flags, MSF_DOWNLOAD_ON_STARTUP);

      if(GetMUICheck(data->CH_INTERVAL) == TRUE)
        setFlag(msn->flags, MSF_DOWNLOAD_PERIODICALLY);
      else
        clearFlag(msn->flags, MSF_DOWNLOAD_PERIODICALLY);

      set(data->NM_INTERVAL, MUIA_Disabled, hasServerDownloadPeriodically(msn) == FALSE);

      msn->downloadInterval = GetMUINumer(data->NM_INTERVAL);

      if(GetMUICheck(data->CH_DLLARGE) == TRUE)
        setFlag(msn->flags, MSF_DOWNLOAD_LARGE_MAILS);
      else
        clearFlag(msn->flags, MSF_DOWNLOAD_LARGE_MAILS);

      set(data->ST_WARNSIZE, MUIA_Disabled, hasServerDownloadLargeMails(msn) == FALSE);

      msn->largeMailSizeLimit = GetMUIInteger(data->ST_WARNSIZE);

      if(GetMUICheck(data->CH_APPLYREMOTEFILTERS) == TRUE)
        setFlag(msn->flags, MSF_APPLY_REMOTE_FILTERS);
      else
        clearFlag(msn->flags, MSF_APPLY_REMOTE_FILTERS);

      if(GetMUICheck(data->CH_DELETE) == TRUE)
        setFlag(msn->flags, MSF_PURGEMESSGAES);
      else
        clearFlag(msn->flags, MSF_PURGEMESSGAES);

      // if the user hasn't yet entered an own account name or the default
      // account name is still present we go and set an automatic generated one
      if(msn->description[0] == '\0' || strcmp(msn->description, tr(MSG_NewEntry)) == 0)
        snprintf(msn->description, sizeof(msn->description), "%s@%s", msn->username, msn->hostname);

      // remember the current flags of the server
      oldSSLFlags = msn->flags;

      switch(GetMUICycle(data->CY_POPSECURE))
      {
        // TLSv1 secure connection (STARTTLS)
        case 1:
        {
          setFlag(msn->flags, MSF_SEC_TLS);
          clearFlag(msn->flags, MSF_SEC_SSL);
        }
        break;

        // SSLv3 secure connection (SSL/TLS)
        case 2:
        {
          clearFlag(msn->flags, MSF_SEC_TLS);
          setFlag(msn->flags, MSF_SEC_SSL);
        }
        break;

        // no secure connection
        default:
        {
          clearFlag(msn->flags, MSF_SEC_TLS);
          clearFlag(msn->flags, MSF_SEC_SSL);
        }
        break;
      }

      // check if the user changed something on the SSL/TLS options and
      // update the port accordingly
      if(oldSSLFlags != msn->flags)
      {
        if(hasServerSSL(msn) == TRUE)
          nnset(data->ST_POPPORT, MUIA_String_Integer, 995);
        else
          nnset(data->ST_POPPORT, MUIA_String_Integer, 110);
      }

      msn->port = GetMUIInteger(data->ST_POPPORT);

      msn->notifyByRequester = GetMUICheck(data->CH_POP3_NOTIFY_REQ);
      #if defined(__amigaos4__)
      msn->notifyByOS41System = GetMUICheck(data->CH_POP3_NOTIFY_OS41SYSTEM);
      #endif // __amigaos4__
      msn->notifyBySound = GetMUICheck(data->CH_POP3_NOTIFY_SOUND);
      msn->notifyByCommand = GetMUICheck(data->CH_POP3_NOTIFY_CMD);
      GetMUIString(msn->notifySound, data->ST_POP3_NOTIFY_SOUND, sizeof(msn->notifySound));
      GetMUIString(msn->notifyCommand, data->ST_POP3_NOTIFY_CMD, sizeof(msn->notifyCommand));

      #if defined(__amigaos4__)
      set(data->CH_POP3_NOTIFY_OS41SYSTEM, MUIA_Disabled, G->applicationID == 0 || LIB_VERSION_IS_AT_LEAST(ApplicationBase, 53, 2) == FALSE);
      #endif // __amigaos4__

      set(data->PO_POP3_NOTIFY_SOUND, MUIA_Disabled, msn == NULL || msn->notifyBySound == FALSE);
      set(data->BT_POP3_NOTIFY_SOUND, MUIA_Disabled, msn == NULL || msn->notifyBySound == FALSE);
      set(data->PO_POP3_NOTIFY_CMD, MUIA_Disabled, msn == NULL || msn->notifyByCommand == FALSE);

      DoMethod(data->LV_POP3, MUIM_NList_Redraw, p);
    }
  }

  RETURN(0);
  return 0;
}

///
/// DECLARE(AddPOP3Entry)
// adds a new entry to the POP3 account list
DECLARE(AddPOP3Entry)
{
  GETDATA;
  struct MailServerNode *msn;

  ENTER();

  if((msn = CreateNewMailServer(MST_POP3, CE, IsMinListEmpty(&CE->pop3ServerList))) != NULL)
  {
    if(IsMinListEmpty(&CE->pop3ServerList) == FALSE)
      strlcpy(msn->description, tr(MSG_NewEntry), sizeof(msn->description));

    DoMethod(data->LV_POP3, MUIM_NList_InsertSingle, msn, MUIV_NList_Insert_Bottom);

    // add the server to the list
    AddTail((struct List *)&CE->pop3ServerList, (struct Node *)msn);

    // set the new entry active and make sure that the host gadget will be
    // set as the new active object of the window as that gadget will be used
    // to automatically set the account name.
    set(data->LV_POP3, MUIA_NList_Active, MUIV_List_Active_Bottom);
    set(_win(obj), MUIA_Window_ActiveObject, data->ST_POPHOST);
  }

  RETURN(0);
  return 0;
}

///
/// DECLARE(DeletePOP3Entry)
// deletes an entry from the POP3 account list
DECLARE(DeletePOP3Entry)
{
  GETDATA;
  struct MailServerNode *msn = NULL;

  ENTER();

  DoMethod(data->LV_POP3, MUIM_NList_GetEntry, MUIV_NList_GetEntry_Active, &msn);

  if(msn != NULL &&
     xget(data->LV_POP3, MUIA_NList_Entries) > 1)
  {
    DoMethod(data->LV_POP3, MUIM_NList_Remove, MUIV_NList_Remove_Active);

    // remove it from the internal mail server list as well.
    Remove((struct Node *)msn);

    // delete a possibly existing UIDL database file
    DeleteUIDLfile(msn);

    FreeSysObject(ASOT_NODE, msn);
  }

  RETURN(0);
  return 0;
}

///
/// DECLARE(GetSMTPEntry)
// fills form with data from selected list entry
DECLARE(GetSMTPEntry)
{
  GETDATA;
  struct MailServerNode *msn = NULL;
  LONG pos = MUIV_NList_GetPos_Start;

  ENTER();

  // get the currently selected SMTP server
  DoMethod(data->LV_SMTP, MUIM_NList_GetEntry, MUIV_NList_GetEntry_Active, &msn);

  // make sure to disable GUI elements
  if(msn == NULL || xget(data->LV_SMTP, MUIA_NList_Entries) < 2)
    set(data->BT_SDEL, MUIA_Disabled, TRUE);
  else
    set(data->BT_SDEL, MUIA_Disabled, FALSE);

  if(msn != NULL)
    DoMethod(data->LV_SMTP, MUIM_NList_GetPos, msn, &pos);
  else
    pos = 0;

  set(data->BT_SMTPUP, MUIA_Disabled, pos == 0);
  set(data->BT_SMTPDOWN, MUIA_Disabled, pos == (LONG)xget(data->LV_SMTP, MUIA_NList_Entries) - 1);

  if(msn != NULL)
  {
    // all notifies here are nnset() notifies so that we don't trigger any additional
    // notify or otherwise we would run into problems.

    nnset(data->CH_SMTPENABLED,   MUIA_Selected,        isServerActive(msn));
    nnset(data->ST_SMTPDESC,      MUIA_String_Contents, msn->description);
    nnset(data->ST_SMTPHOST,      MUIA_String_Contents, msn->hostname);
    nnset(data->ST_SMTPPORT,      MUIA_String_Integer,  msn->port);
    nnset(data->ST_SMTPAUTHUSER,  MUIA_String_Contents, msn->username);
    nnset(data->ST_SMTPAUTHPASS,  MUIA_String_Contents, msn->password);
    nnset(data->CH_SMTP8BIT,      MUIA_Selected,        hasServer8bit(msn));

    xset(data->CY_SMTPSECURE, MUIA_NoNotify,     TRUE,
                              MUIA_Cycle_Active, MSF2SMTPSecMethod(msn),
                              MUIA_Disabled,     G->TR_UseableTLS == FALSE);

    nnset(data->CY_SMTPAUTH, MUIA_Cycle_Active, hasServerAuth(msn) ? MSF2SMTPAuthMethod(msn)+1 : 0);

    set(data->ST_SMTPAUTHUSER, MUIA_Disabled, hasServerAuth(msn) == FALSE);
    set(data->ST_SMTPAUTHPASS, MUIA_Disabled, hasServerAuth(msn) == FALSE);
  }

  RETURN(0);
  return 0;
}

///
/// DECLARE(PutSMTPEntry)
// fills form data into selected list entry
DECLARE(PutSMTPEntry)
{
  GETDATA;
  int p;

  ENTER();

  p = xget(data->LV_SMTP, MUIA_NList_Active);
  if(p != MUIV_NList_Active_Off)
  {
    struct MailServerNode *msn = NULL;

    DoMethod(data->LV_SMTP, MUIM_NList_GetEntry, p, &msn);
    if(msn != NULL)
    {
      unsigned int oldSSLFlags = 0;

      GetMUIString(msn->description,  data->ST_SMTPDESC,    sizeof(msn->description));
      GetMUIString(msn->hostname,     data->ST_SMTPHOST,    sizeof(msn->hostname));
      GetMUIString(msn->username,     data->ST_SMTPAUTHUSER,sizeof(msn->username));
      GetMUIString(msn->password,     data->ST_SMTPAUTHPASS,sizeof(msn->password));

      if(GetMUICheck(data->CH_SMTPENABLED) == TRUE)
        setFlag(msn->flags, MSF_ACTIVE);
      else
        clearFlag(msn->flags, MSF_ACTIVE);

      // if the user hasn't yet entered an own account name or the default
      // account name is still present we go and set an automatic generated one
      if(msn->description[0] == '\0' || strcmp(msn->description, tr(MSG_NewEntry)) == 0)
        snprintf(msn->description, sizeof(msn->description), "%s@%s", msn->username, msn->hostname);

      if(GetMUICheck(data->CH_SMTP8BIT) == TRUE)
        setFlag(msn->flags, MSF_ALLOW_8BIT);
      else
        clearFlag(msn->flags, MSF_ALLOW_8BIT);

      // remember the current flags of the server
      oldSSLFlags = msn->flags;

      switch(GetMUICycle(data->CY_SMTPSECURE))
      {
        // TLSv1 secure connection (STARTTLS)
        case 1:
        {
          setFlag(msn->flags, MSF_SEC_TLS);
          clearFlag(msn->flags, MSF_SEC_SSL);
        }
        break;

        // SSLv3 secure connection (SSL/TLS)
        case 2:
        {
          clearFlag(msn->flags, MSF_SEC_TLS);
          setFlag(msn->flags, MSF_SEC_SSL);
        }
        break;

        // no secure connection
        default:
        {
          clearFlag(msn->flags, MSF_SEC_TLS);
          clearFlag(msn->flags, MSF_SEC_SSL);
        }
        break;
      }

      // check if the user changed something on the SSL/TLS options and
      // update the port accordingly
      if(oldSSLFlags != msn->flags)
      {
        if(hasServerSSL(msn) == TRUE)
          nnset(data->ST_SMTPPORT, MUIA_String_Integer, 465);
        else
          nnset(data->ST_SMTPPORT, MUIA_String_Integer, 25);
      }

      // get port number
      msn->port = GetMUIInteger(data->ST_SMTPPORT);

      switch(GetMUICycle(data->CY_SMTPAUTH))
      {
        // No Authentication
        case 0:
        {
          clearFlag(msn->flags, MSF_AUTH);
          clearFlag(msn->flags, MSF_AUTH_AUTO);
          clearFlag(msn->flags, MSF_AUTH_DIGEST);
          clearFlag(msn->flags, MSF_AUTH_CRAM);
          clearFlag(msn->flags, MSF_AUTH_LOGIN);
          clearFlag(msn->flags, MSF_AUTH_PLAIN);
        }
        break;

        // Auto
        case 1:
        {
          setFlag(msn->flags, MSF_AUTH);
          setFlag(msn->flags, MSF_AUTH_AUTO);
          clearFlag(msn->flags, MSF_AUTH_DIGEST);
          clearFlag(msn->flags, MSF_AUTH_CRAM);
          clearFlag(msn->flags, MSF_AUTH_LOGIN);
          clearFlag(msn->flags, MSF_AUTH_PLAIN);
        }
        break;

        // DIGEST-MD5
        case 2:
        {
          setFlag(msn->flags, MSF_AUTH);
          clearFlag(msn->flags, MSF_AUTH_AUTO);
          setFlag(msn->flags, MSF_AUTH_DIGEST);
          clearFlag(msn->flags, MSF_AUTH_CRAM);
          clearFlag(msn->flags, MSF_AUTH_LOGIN);
          clearFlag(msn->flags, MSF_AUTH_PLAIN);
        }
        break;

        // CRAM-MD5
        case 3:
        {
          setFlag(msn->flags, MSF_AUTH);
          clearFlag(msn->flags, MSF_AUTH_AUTO);
          clearFlag(msn->flags, MSF_AUTH_DIGEST);
          setFlag(msn->flags, MSF_AUTH_CRAM);
          clearFlag(msn->flags, MSF_AUTH_LOGIN);
          clearFlag(msn->flags, MSF_AUTH_PLAIN);
        }
        break;

        // LOGIN
        case 4:
        {
          setFlag(msn->flags, MSF_AUTH);
          clearFlag(msn->flags, MSF_AUTH_AUTO);
          clearFlag(msn->flags, MSF_AUTH_DIGEST);
          clearFlag(msn->flags, MSF_AUTH_CRAM);
          setFlag(msn->flags, MSF_AUTH_LOGIN);
          clearFlag(msn->flags, MSF_AUTH_PLAIN);
        }
        break;

        // PLAIN
        case 5:
        {
          setFlag(msn->flags, MSF_AUTH);
          clearFlag(msn->flags, MSF_AUTH_AUTO);
          clearFlag(msn->flags, MSF_AUTH_DIGEST);
          clearFlag(msn->flags, MSF_AUTH_CRAM);
          clearFlag(msn->flags, MSF_AUTH_LOGIN);
          setFlag(msn->flags, MSF_AUTH_PLAIN);
        }
        break;
      }

      if(GetMUICycle(data->CY_SMTPAUTH) > 0)
      {
        set(data->ST_SMTPAUTHUSER, MUIA_Disabled, FALSE);
        set(data->ST_SMTPAUTHPASS, MUIA_Disabled, FALSE);
      }
      else
      {
        set(data->ST_SMTPAUTHUSER, MUIA_Disabled, TRUE);
        set(data->ST_SMTPAUTHPASS, MUIA_Disabled, TRUE);
      }

      // we also have to update the SMTP Server Array
      // in case the user changes to the Identities
      // config page
      #warning object on other page
   // DoMethod(data->CY_IDENTITY_MAILSERVER, MUIM_MailServerChooser_UpdateMailServers);

      // redraw the list
      DoMethod(data->LV_SMTP, MUIM_NList_Redraw, p);
    }
  }

  RETURN(0);
  return 0;
}

///
/// DECLARE(AddSMTPEntry)
// adds a new entry to the SMTP account list
DECLARE(AddSMTPEntry)
{
  GETDATA;
  struct MailServerNode *msn;

  ENTER();

  if((msn = CreateNewMailServer(MST_SMTP, CE, IsMinListEmpty(&CE->smtpServerList))) != NULL)
  {
    if(IsMinListEmpty(&CE->smtpServerList) == FALSE)
      strlcpy(msn->description, tr(MSG_NewEntry), sizeof(msn->description));

    DoMethod(data->LV_SMTP, MUIM_NList_InsertSingle, msn, MUIV_NList_Insert_Bottom);

    // add the server to the list
    AddTail((struct List *)&CE->smtpServerList, (struct Node *)msn);

    // set the new entry active and make sure that the host gadget will be
    // set as the new active object of the window as that gadget will be used
    // to automatically set the account name.
    set(data->LV_SMTP, MUIA_NList_Active, MUIV_List_Active_Bottom);
    set(_win(obj), MUIA_Window_ActiveObject, data->ST_SMTPHOST);
  }

  RETURN(0);
  return 0;
}

///
/// DECLARE(DeleteSMTPEntry)
// deletes an entry from the SMTP account list
DECLARE(DeleteSMTPEntry)
{
  GETDATA;
  struct MailServerNode *msn = NULL;

  ENTER();

  DoMethod(data->LV_SMTP, MUIM_NList_GetEntry, MUIV_NList_GetEntry_Active, &msn);

  if(msn != NULL &&
     xget(data->LV_SMTP, MUIA_NList_Entries) > 1)
  {
    DoMethod(data->LV_SMTP, MUIM_NList_Remove, MUIV_NList_Remove_Active);

    // remove it from the internal mail server list as well.
    Remove((struct Node *)msn);

    FreeSysObject(ASOT_NODE, msn);
  }

  RETURN(0);
  return 0;
}

///
