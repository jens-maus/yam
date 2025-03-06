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
 MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 GNU General Public License for more details.

 You should have received a copy of the GNU General Public License
 along with this program; if not, write to the Free Software
 Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA

 YAM Official Support Site :  http://www.yam.ch
 YAM OpenSource project    :  https://github.com/jens-maus/yam/

 Superclass:  MUIC_Cycle
 Description: Cycle object to choose a time zone location

***************************************************************************/

#include "TZoneLocationChooser_cl.h"

#include <stdlib.h>

#include <proto/muimaster.h>

#include "MUIObjects.h"
#include "TZone.h"

#include "Debug.h"

/* CLASSDATA
struct Data
{
  char **locationArray;               // names of the different ls that can be selected
  ULONG continent;
};
*/

/* Overloaded Methods */
/// OVERLOAD(OM_NEW)
OVERLOAD(OM_NEW)
{
  char **locationArray;
  ULONG continent;

  ENTER();

  continent = GetTagData(ATTR(Continent), 0, inittags(msg));
  locationArray = BuildLocationEntries(continent);

  if((obj = DoSuperNew(cl, obj,

    MUIA_CycleChain,    TRUE,
    MUIA_Font,          MUIV_Font_Button,
    MUIA_Cycle_Entries, locationArray,

    TAG_MORE, inittags(msg))) != NULL)
  {
    GETDATA;

    data->locationArray = locationArray;
    data->continent = continent;
  }
  else
  {
    free(locationArray);
  }

  RETURN((IPTR)obj);
  return (IPTR)obj;
}

///
/// OVERLOAD(OM_DISPOSE)
OVERLOAD(OM_DISPOSE)
{
  GETDATA;
  ULONG result;

  ENTER();

  // free the string array
  free(data->locationArray);

  // signal the super class to dispose as well
  result = DoSuperMethodA(cl, obj, msg);

  RETURN(result);
  return result;
}

///
/// OVERLOAD(OM_SET)
OVERLOAD(OM_SET)
{
  GETDATA;
  struct TagItem *tags = inittags(msg), *tag;
  ULONG result = FALSE;

  ENTER();

  while((tag = NextTagItem((APTR)&tags)) != NULL)
  {
    switch(tag->ti_Tag)
    {
      case ATTR(Continent):
      {
        if(tag->ti_Data != data->continent)
        {
          data->continent = tag->ti_Data;
          DoMethod(obj, METHOD(UpdateLocations));
        }

        // make the superMethod call ignore those tags
        tag->ti_Tag = TAG_IGNORE;
      }
      break;
    }
  }

  result = DoSuperMethodA(cl, obj, msg);

  RETURN(result);
  return result;
}

///

/* Private Functions */

/* Public Methods */
/// DECLARE(UpdateLocations)
// updates the str array containing all locations of a continent
DECLARE(UpdateLocations)
{
  GETDATA;

  ENTER();

  set(obj, MUIA_Cycle_Entries, NULL);

  if(data->locationArray != NULL)
    free(data->locationArray);

  data->locationArray = BuildLocationEntries(data->continent);
  set(obj, MUIA_Cycle_Entries, data->locationArray);

  RETURN(0);
  return 0;
}

///
