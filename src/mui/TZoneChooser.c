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

#include <proto/dos.h>
#include <proto/muimaster.h>

#include "extrasrc.h"

#include "YAM_config.h"
#include "YAM_utilities.h"

#include "Locale.h"
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
  Object *infoLabel;

  char tzone[SIZE_DEFAULT];
  char labelText[SIZE_LARGE];
};
*/

/* Overloaded Methods */
/// OVERLOAD(OM_NEW)
OVERLOAD(OM_NEW)
{
  Object *continents;
  Object *locations;
  Object *infoLabel;

  ENTER();

  if((obj = DoSuperNew(cl, obj,

    MUIA_Group_Horiz, FALSE,
    Child, HGroup,
      Child, continents = TZoneContinentChooserObject, End,
      Child, locations = TZoneLocationChooserObject, End,
    End,
    Child, infoLabel = LLabel(NULL),

    TAG_MORE, inittags(msg))) != NULL)
  {
    GETDATA;

    data->continents = continents;
    data->locations = locations;
    data->infoLabel = infoLabel;

    DoMethod(continents, MUIM_Notify, MUIA_Cycle_Active, MUIV_EveryTime, locations, 3, MUIM_Set, MUIA_TZoneLocationChooser_Continent, MUIV_TriggerValue);
    DoMethod(continents, MUIM_Notify, MUIA_Cycle_Active, MUIV_EveryTime, obj, 1, METHOD(UpdateInfoLabel));
    DoMethod(locations, MUIM_Notify, MUIA_Cycle_Active, MUIV_EveryTime, obj, 1, METHOD(UpdateInfoLabel));
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
      *store = (IPTR)BuildTZoneName(data->tzone, sizeof(data->tzone), xget(data->continents, MUIA_Cycle_Active), xget(data->locations, MUIA_Cycle_Active));

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
/// DECLARE(UpdateInfoLabel)
DECLARE(UpdateInfoLabel)
{
  GETDATA;
  struct TM tm;
  BOOL resetTZ = FALSE;
  char tzAbbr[SIZE_SMALL];
  int gmtOffset;
  int convertedGmtOffset;

  ENTER();

  // now we query some information on the timezone which we will display
  // below the cycle gadgets
  BuildTZoneName(data->tzone, sizeof(data->tzone), xget(data->continents, MUIA_Cycle_Active), xget(data->locations, MUIA_Cycle_Active));

  // get the current date/time in struct tm format
  DateStamp2tm(NULL, &tm);
 
  // check if our objects are representing a different location than the current configuration
  if(strcasecmp(data->tzone, C->Location) != 0)
  {
    tzset(data->tzone);
    resetTZ = TRUE;
  }
 
  // call mktime() so that struct tm will be set correctly.
  mktime(&tm);

  // copy the timezone abbreviation string and the
  // gmtoffset
  strlcpy(tzAbbr, tm.tm_zone, sizeof(tzAbbr));
  gmtOffset = tm.tm_gmtoff / 60;

  // reset the location to the former value
  if(resetTZ == TRUE)
    tzset(C->Location);

  // convert the GMT offset to a human readable value
  convertedGmtOffset = (gmtOffset/60)*100 + (gmtOffset%60);

  // lets prepare the infotext we want to show to the user
  snprintf(data->labelText, sizeof(data->labelText), "%s %+05d (%s)\n%s", tr(MSG_CO_TZONE_GMTOFFSET),
                                                                          convertedGmtOffset, 
                                                                          tzAbbr,
                                                                          tr(MSG_CO_TZONE_NEXTDSTSWITCH));

  // set the labeltext
  set(data->infoLabel, MUIA_Text_Contents, data->labelText);

  RETURN(0);
  return 0;
}

///
