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

#include <stdlib.h>

#include <proto/muimaster.h>

#include "extrasrc.h"

#include "YAM_config.h"

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
  char tzone[64];
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

    DoMethod(continents, MUIM_Notify, MUIA_Cycle_Active, MUIV_EveryTime, locations, 3, MUIM_Set, MUIA_TZoneLocationChooser_Continent, MUIV_TriggerValue);
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

  while((tag = NextTagItem((APTR)&tags)) != NULL)
  {
    switch(tag->ti_Tag)
    {
      case ATTR(TZone):
      {
        char *tzone = (char *)tag->ti_Data;

        if(tzone != NULL)
        {
          ULONG continent;
          ULONG location;

          if(ParseTZoneName(tzone, &continent, &location) == TRUE)
          {
            // set the active cycle entries with notifications enabled,
            // this will automatically set the correct array of locations
            set(data->continents, MUIA_Cycle_Active, continent);
            set(data->locations, MUIA_Cycle_Active, location);
          }
        }
      }
      break;
    }
  }

  return DoSuperMethodA(cl, obj, msg);
}

///
/// OVERLOAD(OM_GET)
OVERLOAD(OM_GET)
{
  GETDATA;
  IPTR *store = ((struct opGet *)msg)->opg_Storage;

  switch(((struct opGet *)msg)->opg_AttrID)
  {
    case ATTR(TZone):
    {
      *store = (IPTR)BuildTZoneName(data->tzone, sizeof(data->tzone), xget(data->continents, MUIA_Cycle_Active), xget(data->locations, MUIA_Cycle_Active));
      return TRUE;
    }

    case ATTR(GMTOffset):
    {
      BOOL resetTZ = FALSE;
      struct TM tm;

      BuildTZoneName(data->tzone, sizeof(data->tzone), xget(data->continents, MUIA_Cycle_Active), xget(data->locations, MUIA_Cycle_Active));

      // check if our objects are representing a different location than the current configuration
      if(strcasecmp(data->tzone, C->Location) != 0)
      {
        setenv("YAMTZ", data->tzone, 1);
        tzset();
        resetTZ = TRUE;
      }

      // mktime() returns the GMT offset in seconds instead of minutes.
      // Additionally we must respect the current DST state to obtain
      // an GMT offset which complies to the system.
      mktime(&tm);
      if(tm.tm_isdst)
        *store = (tm.tm_gmtoff - 3600) / 60;
      else
        *store = tm.tm_gmtoff / 60;

      // reset the location to the former value
      if(resetTZ == TRUE)
      {
        setenv("YAMTZ", C->Location, 1);
        tzset();
      }

      return TRUE;
    }
  }

  return DoSuperMethodA(cl, obj, msg);
}

///

/* Private Functions */

/* Public Methods */

