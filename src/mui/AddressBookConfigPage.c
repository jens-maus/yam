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
 Description: "Address book" configuration page

***************************************************************************/

#include "AddressBookConfigPage_cl.h"

#include <proto/asl.h>
#include <proto/muimaster.h>

#include "YAM_addressbook.h"
#include "YAM_utilities.h"

#include "mui/ConfigPage.h"
#include "mui/ConfigPageList.h"

#include "Config.h"

#include "Debug.h"

/* CLASSDATA
struct Data
{
  Object *CH_ACOLS[NUMBER_ABOOK_COLUMNS];
  Object *CY_ATAB;
  Object *ST_NEWGROUP;
  Object *ST_GALLDIR;
  Object *ST_PROXY;
};
*/

/* INCLUDE
#include "mui/AddrBookListtree.h"
*/

/* Overloaded Methods */
/// OVERLOAD(OM_NEW)
OVERLOAD(OM_NEW)
{
  Object *CH_ACOLS[NUMBER_ABOOK_COLUMNS];
  Object *CY_ATAB;
  Object *ST_NEWGROUP;
  Object *ST_GALLDIR;
  Object *ST_PROXY;
  static const char *atab[6];

  ENTER();

  atab[0] = tr(MSG_CO_ATABnever);
  atab[1] = tr(MSG_CO_ATABinfoask);
  atab[2] = tr(MSG_CO_ATABask);
  atab[3] = tr(MSG_CO_ATABinfo);
  atab[4] = tr(MSG_CO_ATABalways);
  atab[5] = NULL;

  if((obj = DoSuperNew(cl, obj,
    MUIA_HelpNode, "Configuration#Addressbook",
    MUIA_ConfigPage_Page, cp_FirstSteps,
    MUIA_ConfigPage_Contents, VGroup,
      Child, HGroup, GroupFrameT(tr(MSG_Columns)),
        Child, HVSpace,
        Child, ColGroup(4),
          MUIA_ShortHelp, tr(MSG_HELP_CO_CG_AB),
          Child, CH_ACOLS[1] = MakeCheck(""),
          Child, LLabel(tr(MSG_Realname)),
          Child, CH_ACOLS[2] = MakeCheck(""),
          Child, LLabel(tr(MSG_Description)),
          Child, CH_ACOLS[3] = MakeCheck(""),
          Child, LLabel(tr(MSG_Email)),
          Child, CH_ACOLS[4] = MakeCheck(""),
          Child, LLabel(tr(MSG_Street)),
          Child, CH_ACOLS[5] = MakeCheck(""),
          Child, LLabel(tr(MSG_City)),
          Child, CH_ACOLS[6] = MakeCheck(""),
          Child, LLabel(tr(MSG_Country)),
          Child, CH_ACOLS[7] = MakeCheck(""),
          Child, LLabel(tr(MSG_Phone)),
          Child, CH_ACOLS[8] = MakeCheck(""),
          Child, LLabel(tr(MSG_DOB)),
        End,
        Child, HVSpace,
      End,
      Child, ColGroup(2), GroupFrameT(tr(MSG_CO_InfoExc)),
        Child, Label1(tr(MSG_CO_AddToAddrbook)),
        Child, CY_ATAB = MakeCycle(atab, tr(MSG_CO_AddToAddrbook)),
        Child, Label2(tr(MSG_CO_NewGroup)),
        Child, ST_NEWGROUP = MakeString(SIZE_NAME,tr(MSG_CO_NewGroup)),
        Child, Label2(tr(MSG_CO_Gallery)),
        Child, PopaslObject,
          MUIA_Popasl_Type     ,ASL_FileRequest,
          MUIA_Popstring_String,ST_GALLDIR = MakeString(SIZE_PATH,tr(MSG_CO_Gallery)),
          MUIA_Popstring_Button,PopButton(MUII_PopDrawer),
          ASLFR_DrawersOnly, TRUE,
        End,
        Child, Label2(tr(MSG_CO_ProxyServer)),
        Child, ST_PROXY = MakeString(SIZE_HOST,tr(MSG_CO_ProxyServer)),
      End,
    End,
  TAG_MORE, inittags(msg))) != NULL)
  {
    GETDATA;
    LONG i;

    for(i = 1; i < NUMBER_ABOOK_COLUMNS; i++)
      data->CH_ACOLS[i] =      CH_ACOLS[i];
    data->CY_ATAB =          CY_ATAB;
    data->ST_NEWGROUP =      ST_NEWGROUP;
    data->ST_GALLDIR =       ST_GALLDIR;
    data->ST_PROXY =         ST_PROXY;

    SetHelp(ST_GALLDIR,   MSG_HELP_CO_ST_GALLDIR);
    SetHelp(ST_PROXY,     MSG_HELP_CO_ST_PROXY);
    SetHelp(ST_NEWGROUP,  MSG_HELP_CO_ST_NEWGROUP);
    SetHelp(CY_ATAB,      MSG_HELP_CO_CY_ATAB);

    DoMethod(CY_ATAB, MUIM_Notify, MUIA_Cycle_Active, MUIV_EveryTime, ST_NEWGROUP, 3, MUIM_Set, MUIA_Disabled, MUIV_NotTriggerValue);
  }

  RETURN((IPTR)obj);
  return (IPTR)obj;
}

///
/// OVERLOAD(MUIM_ConfigPage_ConfigToGUI)
OVERLOAD(MUIM_ConfigPage_ConfigToGUI)
{
  GETDATA;
  LONG i;

  ENTER();

  setstring(data->ST_GALLDIR, CE->GalleryDir);
  setstring(data->ST_NEWGROUP, CE->NewAddrGroup);
  set(data->ST_NEWGROUP, MUIA_Disabled, CE->AddToAddrbook == 0);
  setstring(data->ST_PROXY, CE->ProxyServer);
  setcycle(data->CY_ATAB, CE->AddToAddrbook);
  for(i = 1; i < NUMBER_ABOOK_COLUMNS; i++)
    setcheckmark(data->CH_ACOLS[i], isFlagSet(CE->AddrbookCols, (1<<i)));

  RETURN(0);
  return 0;
}

///
/// OVERLOAD(MUIM_ConfigPage_GUIToConfig)
OVERLOAD(MUIM_ConfigPage_GUIToConfig)
{
  GETDATA;
  LONG i;

  ENTER();

  GetMUIString(CE->GalleryDir, data->ST_GALLDIR, sizeof(CE->GalleryDir));
  GetMUIString(CE->NewAddrGroup, data->ST_NEWGROUP, sizeof(CE->NewAddrGroup));
  GetMUIString(CE->ProxyServer, data->ST_PROXY, sizeof(CE->ProxyServer));
  CE->AddToAddrbook = GetMUICycle  (data->CY_ATAB);
  CE->AddrbookCols = 1;
  for(i = 1; i < NUMBER_ABOOK_COLUMNS; i++)
  {
    if(GetMUICheck(data->CH_ACOLS[i]))
      setFlag(CE->AddrbookCols, (1<<i));
  }

  RETURN(0);
  return 0;
}

///
