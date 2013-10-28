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
 Description: "First steps" configuration page

***************************************************************************/

#include "FirstStepsConfigPage_cl.h"

#include <proto/muimaster.h>

#include "mui/AddressBookWindow.h"
#include "mui/AddressField.h"
#include "mui/CodesetPopup.h"
#include "mui/ConfigPage.h"
#include "mui/ConfigPageList.h"
#include "mui/TZoneChooser.h"
#include "mui/TZoneInfoBar.h"

#include "Config.h"
#include "MailServers.h"
#include "UserIdentity.h"

#include "Debug.h"

/* CLASSDATA
struct Data
{
  Object *ST_REALNAME;
  Object *ST_EMAIL;
  Object *ST_POPHOST0;
  Object *ST_USER0;
  Object *ST_PASSWD0;
  Object *PO_DEFCODESET_LOCAL;
  Object *GR_TZONE;
  Object *TX_TZONE;
};
*/

/* Overloaded Methods */
/// OVERLOAD(OM_NEW)
OVERLOAD(OM_NEW)
{
  Object *ST_REALNAME;
  Object *ST_EMAIL;
  Object *ST_POPHOST0;
  Object *ST_USER0;
  Object *ST_PASSWD0;
  Object *PO_DEFCODESET_LOCAL;
  Object *GR_TZONE;
  Object *TX_TZONE;

  ENTER();

  if((obj = DoSuperNew(cl, obj,
    MUIA_HelpNode, "Configuration#FirstSteps",
    MUIA_ConfigPage_Page, cp_FirstSteps,
    MUIA_ConfigPage_Contents, VGroup,
      Child, ColGroup(2), GroupFrameT(tr(MSG_CO_MinConfig)),

        Child, Label2(tr(MSG_CO_RealName)),
        Child, ST_REALNAME = MakeString(SIZE_REALNAME,tr(MSG_CO_RealName)),

        Child, Label2(tr(MSG_CO_EmailAddress)),
        Child, MakeAddressField(&ST_EMAIL, tr(MSG_CO_EmailAddress), MSG_HELP_CO_ST_EMAIL, ABM_CONFIG, -1, AFF_NOFULLNAME|AFF_NOCACHE|AFF_NOVALID|AFF_RESOLVEINACTIVE),

        Child, Label2(tr(MSG_CO_SERVERNAME)),
        Child, ST_POPHOST0 = MakeString(SIZE_HOST, tr(MSG_CO_SERVERNAME)),

        Child, Label2(tr(MSG_CO_POPUserID)),
        Child, ST_USER0 = MakeString(SIZE_USERID, tr(MSG_CO_POPUserID)),

        Child, Label2(tr(MSG_CO_Password)),
        Child, ST_PASSWD0 = MakePassString(tr(MSG_CO_Password)),

      End,

      Child, ColGroup(2), GroupFrameT(tr(MSG_CO_SYSTEMSETTINGS)),

        Child, Label2(tr(MSG_CO_DEFAULTCHARSET)),
        Child, PO_DEFCODESET_LOCAL = CodesetPopupObject,
          MUIA_CodesetPopup_ControlChar, tr(MSG_CO_DEFAULTCHARSET),
          MUIA_CodesetPopup_AllowMultibyteCodesets, FALSE,
        End,

        Child, Label2(tr(MSG_CO_TimeZone)),
        Child, GR_TZONE = TZoneChooserObject, End,

        Child, HSpace(-1),
        Child, TX_TZONE = TZoneInfoBarObject, End,

      End,
    End,
  TAG_MORE, inittags(msg))) != NULL)
  {
    GETDATA;

    data->ST_REALNAME =         ST_REALNAME;
    data->ST_EMAIL =            ST_EMAIL;
    data->ST_POPHOST0 =         ST_POPHOST0;
    data->ST_USER0 =            ST_USER0;
    data->ST_PASSWD0 =          ST_PASSWD0;
    data->PO_DEFCODESET_LOCAL = PO_DEFCODESET_LOCAL;
    data->GR_TZONE =            GR_TZONE;
    data->TX_TZONE =            TX_TZONE;

    SetHelp(ST_REALNAME,         MSG_HELP_CO_ST_REALNAME);
    SetHelp(ST_POPHOST0,         MSG_HELP_CO_ST_POPHOST);
    SetHelp(ST_USER0,            MSG_HELP_CO_ST_USER);
    SetHelp(ST_PASSWD0,          MSG_HELP_CO_ST_PASSWD);
    SetHelp(PO_DEFCODESET_LOCAL, MSG_HELP_CO_TX_DEFCODESET_LOCAL);
    SetHelp(GR_TZONE,            MSG_HELP_CO_GR_TZONE);

    DoMethod(ST_POPHOST0, MUIM_Notify, MUIA_String_Contents, MUIV_EveryTime, obj, 1, METHOD(GetDefaultPOP));
    DoMethod(ST_USER0, MUIM_Notify, MUIA_String_Contents, MUIV_EveryTime, obj, 1, METHOD(GetDefaultPOP));
    DoMethod(ST_PASSWD0, MUIM_Notify, MUIA_String_Contents, MUIV_EveryTime, obj, 1, METHOD(GetDefaultPOP));
    DoMethod(GR_TZONE, MUIM_Notify, MUIA_TZoneChooser_TZone, MUIV_EveryTime, TX_TZONE, 3, MUIM_Set, MUIA_TZoneInfoBar_TZone, MUIV_TriggerValue);
  }

  RETURN((IPTR)obj);
  return (IPTR)obj;
}

///
/// OVERLOAD(MUIM_ConfigPage_ConfigToGUI)
OVERLOAD(MUIM_ConfigPage_ConfigToGUI)
{
  GETDATA;
  struct MailServerNode *msn;
  struct UserIdentityNode *uin;

  ENTER();

  // try to get the first user identity structure
  if((uin = GetUserIdentity(&CE->userIdentityList, 0, TRUE)) != NULL)
  {
    setstring(data->ST_REALNAME, uin->realname);
    setstring(data->ST_EMAIL, uin->address);
  }

  set(data->GR_TZONE, MUIA_TZoneChooser_TZone, CE->Location);

  // try to get the mailer server structure of the first POP3 server
  if((msn = GetMailServer(&CE->pop3ServerList, 0)) != NULL)
  {
    nnset(data->ST_POPHOST0, MUIA_String_Contents, msn->hostname);
    nnset(data->ST_USER0,    MUIA_String_Contents, msn->username);
    nnset(data->ST_PASSWD0,  MUIA_String_Contents, msn->password);
  }

  nnset(data->PO_DEFCODESET_LOCAL, MUIA_CodesetPopup_Codeset, CE->DefaultLocalCodeset);

  RETURN(0);
  return 0;
}

///
/// OVERLOAD(MUIM_ConfigPage_GUIToConfig)
OVERLOAD(MUIM_ConfigPage_GUIToConfig)
{
  GETDATA;
  struct UserIdentityNode *uin;

  ENTER();

  if((uin = GetUserIdentity(&CE->userIdentityList, 0, TRUE)) != NULL)
  {
    GetMUIString(uin->realname, data->ST_REALNAME, sizeof(uin->realname));
    GetMUIString(uin->address, data->ST_EMAIL, sizeof(uin->address));
  }

  strlcpy(CE->Location, (char *)xget(data->GR_TZONE, MUIA_TZoneChooser_TZone), sizeof(CE->Location));
  strlcpy(CE->DefaultLocalCodeset, (char *)xget(data->PO_DEFCODESET_LOCAL, MUIA_CodesetPopup_Codeset), sizeof(CE->DefaultLocalCodeset));

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
      struct MailServerNode *msn;

      if((msn = GetMailServer(&CE->pop3ServerList, 0)) != NULL)
      {
        nnset(data->ST_POPHOST0, MUIA_String_Contents, msn->hostname);
        nnset(data->ST_USER0,    MUIA_String_Contents, msn->username);
        nnset(data->ST_PASSWD0,  MUIA_String_Contents, msn->password);
      }
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
/// DECLARE(GetDefaultPOP)
// set values of first POP3 account
DECLARE(GetDefaultPOP)
{
  GETDATA;
  struct MailServerNode *msn;

  ENTER();

  // get the first POP3 server out of our mail server list
  msn = GetMailServer(&CE->pop3ServerList, 0);
  if(msn != NULL)
  {
    GetMUIString(msn->hostname, data->ST_POPHOST0, sizeof(msn->hostname));
    GetMUIString(msn->username, data->ST_USER0, sizeof(msn->username));
    GetMUIString(msn->password, data->ST_PASSWD0, sizeof(msn->password));

    if(msn->description[0] == '\0')
      snprintf(msn->description, sizeof(msn->description), "%s@%s", msn->username, msn->hostname);

    msn->port = 110;
  }

  RETURN(0);
  return 0;
}

///
