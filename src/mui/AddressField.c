/***************************************************************************

 YAM - Yet Another Mailer
 Copyright (C) 1995-2000 Marcel Beck
 Copyright (C) 2000-2025 YAM Open Source Team

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
 YAM OpenSource project     : https://github.com/jens-maus/yam/

 Superclass:  MUIC_Group
 Description: Recipient string object with an attached popup button

***************************************************************************/

#include "AddressField_cl.h"

#include <proto/muimaster.h>
#include <mui/BetterString_mcc.h>

#include "YAM_utilities.h"

#include "mui/AddressBookWindow.h"
#include "mui/RecipientString.h"
#include "mui/YAMApplication.h"

#include "Locale.h"
#include "MUIObjects.h"

#include "Debug.h"

/* CLASSDATA
struct Data
{
  Object *ST_RECIPIENTS;
};
*/

/* EXPORT
#define AFF_ALLOW_MULTI         (1<<0)
#define AFF_EXTERNAL_SHORTCUTS  (1<<1)
#define AFF_NOFULLNAME          (1<<2)
#define AFF_NOCACHE             (1<<3)
#define AFF_NOVALID             (1<<4)
#define AFF_RESOLVEINACTIVE     (1<<5)
*/

/* Overloaded Methods */
/// OVERLOAD(OM_NEW)
OVERLOAD(OM_NEW)
{
  Object *ST_RECIPIENTS;
  Object *BT_POPUP;
  struct TagItem *tstate = inittags(msg);
  struct TagItem *tag;
  ULONG flags = 0;
  char shortCut = '\0';
  const struct fcstr * help = NULL;
  enum AddressbookMode mode = ABM_NONE;
  LONG windowNumber = -1;

  ENTER();

  while((tag = NextTagItem((APTR)&tstate)) != NULL)
  {
    switch(tag->ti_Tag)
    {
      case ATTR(Flags):        flags = tag->ti_Data; break;
      case ATTR(Shortcut):     shortCut = tag->ti_Data; break;
      case ATTR(Help):         help = (const struct fcstr *)tag->ti_Data; break;
      case ATTR(Mode):         mode = tag->ti_Data; break;
      case ATTR(WindowNumber): windowNumber = tag->ti_Data; break;
    }
  }

  if((obj = DoSuperNew(cl, obj,
    MUIA_Group_Horiz, TRUE,
    MUIA_Group_Spacing, 0,
    Child, ST_RECIPIENTS = RecipientStringObject,
      MUIA_CycleChain,                         TRUE,
      MUIA_String_AdvanceOnCR,                 TRUE,
      MUIA_RecipientString_ResolveOnCR,        TRUE,
      MUIA_RecipientString_MultipleRecipients, isFlagSet(flags, AFF_ALLOW_MULTI),
      MUIA_RecipientString_NoFullName,         isFlagSet(flags, AFF_NOFULLNAME),
      MUIA_RecipientString_NoCache,            isFlagSet(flags, AFF_NOCACHE),
      MUIA_RecipientString_NoValid,            isFlagSet(flags, AFF_NOVALID),
      MUIA_RecipientString_ResolveOnInactive,  isFlagSet(flags, AFF_RESOLVEINACTIVE),
      MUIA_BetterString_NoShortcuts,           isFlagSet(flags, AFF_EXTERNAL_SHORTCUTS),
      MUIA_ControlChar,                        shortCut,
    End,
    Child, BT_POPUP = PopButton(MUII_PopUp),
  TAG_MORE, inittags(msg))) != NULL)
  {
    GETDATA;

    data->ST_RECIPIENTS = ST_RECIPIENTS;

    if(help != NULL)
      SetHelp(ST_RECIPIENTS, help);
    SetHelp(BT_POPUP, MSG_HELP_WR_BT_ADR);

    if(mode == ABM_CONFIG)
    {
      DoMethod(BT_POPUP,      MUIM_Notify, MUIA_Pressed,               FALSE, MUIV_Notify_Application, 4, MUIM_YAMApplication_OpenAddressBookWindow, ABM_CONFIG, -1, ST_RECIPIENTS);
      DoMethod(ST_RECIPIENTS, MUIM_Notify, MUIA_RecipientString_Popup, TRUE,  MUIV_Notify_Application, 4, MUIM_YAMApplication_OpenAddressBookWindow, ABM_CONFIG, -1, ST_RECIPIENTS);
    }
    else
    {
      DoMethod(BT_POPUP,      MUIM_Notify, MUIA_Pressed,               FALSE, MUIV_Notify_Application, 4, MUIM_YAMApplication_OpenAddressBookWindow, mode, windowNumber, NULL);
      DoMethod(ST_RECIPIENTS, MUIM_Notify, MUIA_RecipientString_Popup, TRUE,  MUIV_Notify_Application, 4, MUIM_YAMApplication_OpenAddressBookWindow, mode, windowNumber, NULL);
    }

    DoMethod(ST_RECIPIENTS, MUIM_Notify, MUIA_Disabled, MUIV_EveryTime, BT_POPUP, 3, MUIM_Set, MUIA_Disabled, MUIV_TriggerValue);
  }

  RETURN((IPTR)obj);
  return (IPTR)obj;
}

///
/// OVERLOAD(OM_GET)
OVERLOAD(OM_GET)
{
  GETDATA;
  IPTR *store = ((struct opGet *)msg)->opg_Storage;

  switch(((struct opGet *)msg)->opg_AttrID)
  {
    case ATTR(RecipientString): *store = (IPTR)data->ST_RECIPIENTS; return TRUE;
  }

  return DoSuperMethodA(cl, obj, msg);
}

///
