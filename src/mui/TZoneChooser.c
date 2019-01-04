/***************************************************************************

 YAM - Yet Another Mailer
 Copyright (C) 1995-2000 Marcel Beck
 Copyright (C) 2000-2019 YAM Open Source Team

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

 Superclass:  MUIC_Group
 Description: Group of two cycle object to choose a time zone

***************************************************************************/

#include "TZoneChooser_cl.h"

#include <string.h>

#include <proto/muimaster.h>

#include "extrasrc.h"

#include "MUIObjects.h"
#include "TZone.h"

#include "mui/TZoneContinentChooser.h"
#include "mui/TZoneLocationChooser.h"

#include "Debug.h"

/* CLASSDATA
struct Data
{
  Object *continents;
  Object *locations;
  ULONG continent;
  ULONG location;
  BOOL selfSet;

  char tzone[SIZE_DEFAULT];
};
*/

/* Overloaded Methods */
/// OVERLOAD(OM_NEW)
OVERLOAD(OM_NEW)
{
  Object *continents;
  Object *locations;

  ENTER();

  if((obj = DoSuperNew(cl, obj,

    MUIA_Group_Horiz, TRUE,
    Child, continents = TZoneContinentChooserObject, End,
    Child, locations = TZoneLocationChooserObject, End,

    TAG_MORE, inittags(msg))) != NULL)
  {
    GETDATA;

    data->continents = continents;
    data->locations = locations;

    DoMethod(continents, MUIM_Notify, MUIA_Cycle_Active, MUIV_EveryTime, obj, 3, METHOD(BuildTZoneName), continents, MUIV_TriggerValue);
    DoMethod(locations, MUIM_Notify, MUIA_Cycle_Active, MUIV_EveryTime, obj, 3, METHOD(BuildTZoneName), locations, MUIV_TriggerValue);
  }

  RETURN((IPTR)obj);
  return (IPTR)obj;
}

///
/// OVERLOAD(OM_SET)
OVERLOAD(OM_SET)
{
  GETDATA;
  struct TagItem *tags = inittags(msg), *tag;
  ULONG result;

  ENTER();

  while((tag = NextTagItem((APTR)&tags)) != NULL)
  {
    switch(tag->ti_Tag)
    {
      case ATTR(TZone):
      {
        if(data->selfSet == FALSE)
        {
          char *tzone = (char *)tag->ti_Data;

          if(tzone != NULL)
          {
            ULONG continent;
            ULONG location;

            if(ParseTZoneName(tzone, &continent, &location, NULL) == TRUE)
            {
              nnset(data->continents, MUIA_Cycle_Active, continent);
              nnset(data->locations, MUIA_TZoneLocationChooser_Continent, continent);
              nnset(data->locations, MUIA_Cycle_Active, location);

              strlcpy(data->tzone, tzone, sizeof(data->tzone));
              data->continent = continent;
              data->location = location;
            }
          }
        }

        // don't set the tag value to TAG_IGNORE to allow notifications to work
      }
      break;
    }
  }

  result = DoSuperMethodA(cl, obj, msg);

  RETURN(result);
  return result;
}

///
/// OVERLOAD(OM_GET)
OVERLOAD(OM_GET)
{
  GETDATA;
  IPTR *store = ((struct opGet *)msg)->opg_Storage;
  ULONG result = FALSE;

  ENTER();

  switch(((struct opGet *)msg)->opg_AttrID)
  {
    case ATTR(TZone):
    {
      *store = (IPTR)data->tzone;
      result = TRUE;
    }
    break;
  }

  if(result == FALSE)
    result = DoSuperMethodA(cl, obj, msg);

  RETURN(result);
  return result;
}

///

/* Private Functions */

/* Public Methods */
/// DECLARE(BuildTZoneName)
DECLARE(BuildTZoneName) // Object *origin, ULONG entry
{
  GETDATA;

  ENTER();

  if(msg->origin == data->continents)
  {
    data->continent = msg->entry;
    data->location = 0;
    nnset(data->locations, MUIA_TZoneLocationChooser_Continent, msg->entry);
  }
  else if(msg->origin == data->locations)
  {
    data->location = msg->entry;
  }

  // rebuild the time zone name
  BuildTZoneName(data->tzone, sizeof(data->tzone), data->continent, data->location);

  // don't do a recursive set() of the new time zone name,
  // but trigger notifications
  data->selfSet = TRUE;
  set(obj, ATTR(TZone), data->tzone);
  data->selfSet = FALSE;

  RETURN(0);
  return 0;
}

///
