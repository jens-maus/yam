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

***************************************************************************/

#include <stdlib.h>
#include <string.h>

#include <proto/exec.h>

#include "extrasrc.h"

#include "YAM.h"
#include "YAM_global.h"
#include "YAM_utilities.h"

#include "TZone.h"

#include "Debug.h"

/// addLocation
// add a new location and a new continent if necessary
static void addLocation(const char *contName, const char *locName)
{
  struct TZoneContinent *found = NULL;
  struct Node *curNode;

  ENTER();

  // search for the continent first
  IterateList(&G->tzoneContinentList, curNode)
  {
    struct TZoneContinent *cont = (struct TZoneContinent *)curNode;

    if(strcmp(cont->name, contName) == 0)
    {
      found = cont;
      break;
    }
  }

  // create a new continent node if it was not found
  if(found == NULL)
  {
    struct TZoneContinent *cont;

    if((cont = AllocSysObjectTags(ASOT_NODE,
      ASONODE_Size, sizeof(*cont),
      ASONODE_Min, TRUE,
      TAG_DONE)) != NULL)
    {
      cont->name = strdup(contName);
      cont->numLocations = 0;
      NewMinList(&cont->locationList);

      AddTail((struct List *)&G->tzoneContinentList, (struct Node *)cont);

      found = cont;
    }
  }

  if(found != NULL)
  {
    struct TZoneLocation *loc;

    // create a new location node
    if((loc = AllocSysObjectTags(ASOT_NODE,
      ASONODE_Size, sizeof(*loc),
      ASONODE_Min, TRUE,
      TAG_DONE)) != NULL)
    {
      loc->name = strdup(locName);

      AddTail((struct List *)&found->locationList, (struct Node *)loc);
      // count the number of locations
      found->numLocations++;
    }
  }

  LEAVE();
}

///
/// compareContinents
static int compareContinents(const struct MinNode *n1, const struct MinNode *n2)
{
  return strcmp(((struct TZoneContinent *)n1)->name, ((struct TZoneContinent *)n2)->name);
}

///
/// compareLocations
static int compareLocations(const struct MinNode *n1, const struct MinNode *n2)
{
  return strcmp(((struct TZoneLocation *)n1)->name, ((struct TZoneLocation *)n2)->name);
}

///
/// sortLocations
// sort all locations alphabetically
static void sortLocations(void)
{
  struct Node *curNode;

  ENTER();

  // sort the continents first
  SortExecList(&G->tzoneContinentList, compareContinents);

  // then sort the locations of each continent
  IterateList(&G->tzoneContinentList, curNode)
  {
    struct TZoneContinent *cont = (struct TZoneContinent *)curNode;

    SortExecList(&cont->locationList, compareLocations);
  }

  LEAVE();
}

///
/// ParseZoneTabFile
// parse the zone.zab file into continents and cities
void ParseZoneTabFile(void)
{
  char filename[SIZE_PATHFILE];
  FILE *fh;

  ENTER();

  AddPath(filename, G->ProgDir, "Resources/zoneinfo/zone.tab", sizeof(filename));

  if((fh = fopen(filename, "r")) != NULL)
  {
    char *buf = NULL;
    size_t buflen = 0;

    setvbuf(fh, NULL, _IOFBF, SIZE_FILEBUF);

    while(GetLine(&buf, &buflen, fh) > 0)
    {
      // skip empty lines and comments
      if(buf[0] != '\0' && buf[0] != '#')
      {
        // the lines have the format "code coordinates TZ comments"
        char *p = buf;
        #if defined(DEBUG)
        char *code = buf;
        char *coordinates = NULL;
        char *comments = NULL;
        #endif
        char *tz = NULL;

        if((p = strpbrk(p, " \t")) != NULL)
        {
          *p++ = '\0';
          #if defined(DEBUG)
          coordinates = p;
          #endif

          if((p = strpbrk(p, " \t")) != NULL)
          {
            *p++ = '\0';
            tz = p;

            if((p = strpbrk(p, " \t")) != NULL)
            {
              *p++ = '\0';
              #if defined(DEBUG)
              comments = p;
              #endif
            }
          }
        }

        D(DBF_TZONE, "found code '%s', coordinates '%s', tz '%s', comments '%s'", SafeStr(code), SafeStr(coordinates), SafeStr(tz), SafeStr(comments));
        // we are only interested in the "TZ" part
        if(tz != NULL)
        {
          char *continent = tz;
          char *cityOrState = NULL;

          // the tz part has the format "continent/city_or_state[/city]"
          if((p = strpbrk(tz, "/")) != NULL)
          {
            *p++ = '\0';

            cityOrState = p;
            // no further parsing, we use the "city or state" part plus the optional "city" part as it is
          }

          D(DBF_TZONE, "found contintent '%s', city or state '%s'", SafeStr(continent), SafeStr(cityOrState));
          if(cityOrState != NULL)
          {
            // convert all underscores to spaces
            p = cityOrState;
            while((p = strchr(p, '_')) != NULL)
              *p++ = ' ';

            addLocation(continent, cityOrState);
          }
        }
      }
    }

    free(buf);
    fclose(fh);

    // sort all locations
    sortLocations();
  }
  else
  {
    E(DBF_TZONE, "failed to open file '%s'", filename);
  }

  LEAVE();
}

///
/// BuildContinentEntries
// set up an array of strings to be used with a Cycle object
char **BuildContinentEntries(void)
{
  char **entries = NULL;
  size_t count;

  ENTER();

  // count the continents
  if((count = CountNodes(&G->tzoneContinentList)) != 0)
  {
    if((entries = calloc(count+1, sizeof(char *))) != NULL)
    {
      struct Node *curNode;
      char **ptr = entries;

      // copy all continent names
      IterateList(&G->tzoneContinentList, curNode)
      {
        struct TZoneContinent *cont = (struct TZoneContinent *)curNode;

        *ptr++ = cont->name;
      }
    }
  }

  RETURN(entries);
  return entries;
}

///
/// BuildLocationEntries
// set up an array of strings to be used with a Cycle object
char **BuildLocationEntries(ULONG continent)
{
  char **entries = NULL;
  struct TZoneContinent *cont;

  ENTER();

  if((cont = (struct TZoneContinent *)GetNthNode(&G->tzoneContinentList, continent)) != NULL)
  {
    if(cont->numLocations != 0 && (entries = calloc(cont->numLocations+1, sizeof(char *))) != NULL)
    {
      char **ptr = entries;
      struct Node *curNode;

      // copy all location names
      IterateList(&cont->locationList, curNode)
      {
        struct TZoneLocation *loc = (struct TZoneLocation *)curNode;

        *ptr++ = loc->name;
      }
    }
  }

  RETURN(entries);
  return entries;
}

///
/// BuildTZoneName
// build a complete time zone name
char *BuildTZoneName(char *name, size_t nameSize, ULONG continent, ULONG location)
{
  struct TZoneContinent *cont;

  ENTER();

  name[0] = '\0';

  if((cont = (struct TZoneContinent *)GetNthNode(&G->tzoneContinentList, continent)) != NULL)
  {
    struct TZoneLocation *loc;

    if((loc = (struct TZoneLocation *)GetNthNode(&cont->locationList, location)) != NULL)
    {
      char *p;

      // set up the complete time zone name
      snprintf(name, nameSize, "%s/%s", cont->name, loc->name);

      // replace all spaces by underscores
      p = name;
      while((p = strchr(p, ' ')) != NULL)
        *p++ = '_';
    }
  }

  RETURN(name);
  return name;
}

///
/// FindContinent
// find a continent by name
static struct TZoneContinent *findContinent(const char *continent, ULONG *index)
{
  struct TZoneContinent *result = NULL;
  struct Node *curNode;
  ULONG i;

  i = 0;
  IterateList(&G->tzoneContinentList, curNode)
  {
    struct TZoneContinent *cont = (struct TZoneContinent *)curNode;

    if(strcmp(cont->name, continent) == 0)
    {
      result = cont;
      *index = i;
      break;
    }

    i++;
  }

  RETURN(result);
  return result;
}

///
/// FindLocation
// find a location on a continent by name
static struct TZoneLocation *findLocation(struct TZoneContinent *continent, const char *location, ULONG *index)
{
  struct TZoneLocation *result = NULL;
  struct Node *curNode;
  ULONG i;

  i = 0;
  IterateList(&continent->locationList, curNode)
  {
    struct TZoneLocation *loc = (struct TZoneLocation *)curNode;

    if(strcmp(loc->name, location) == 0)
    {
     result = loc;
     *index = i;
      break;
    }

    i++;
  }

  RETURN(result);
  return result;
}

///
/// ParseTZoneName
// parse a time zone name into continent and location index with the respective lists
BOOL ParseTZoneName(const char *tzone, ULONG *continent, ULONG *location)
{
  BOOL result = FALSE;
  char *tmp;

  ENTER();

  if((tmp = strdup(tzone)) != NULL)
  {
    char *p;

    if((p = strchr(tmp, '/')) != NULL)
    {
      struct TZoneContinent *cont;

      // split the two parts
      *p++ = '\0';

      if((cont = findContinent(tmp, continent)) != NULL)
      {
        struct TZoneLocation *loc;

        if((loc = findLocation(cont, p, location)) != NULL)
        {
          result = TRUE;
        }
        else
        {
          E(DBF_TZONE, "cannot find location '%s' on continent '%s'", p, tmp);
        }
      }
      else
      {
        E(DBF_TZONE, "cannot find continent '%s'", tmp);
      }
    }
    else
    {
      E(DBF_TZONE, "invalid time zone name '%s'", tzone);
    }

    free(tmp);
  }

  RETURN(result);
  return result;
}

///
/// GuessTZone
// guess the time zone based on the GMT offset
const char *GuessTZone(LONG gmtOffset)
{
  const char *tzone;

  ENTER();

  switch(gmtOffset)
  {
    case -720: // GMT-12:00
    {
      tzone = "Pacific/Kwajalein";
    }
    break;

    case -660: // GMT-11:00
    {
      tzone = "Pacific/Midway";
    }
    break;

    case -600: // GMT-10:00
    {
      tzone = "Pacific/Honolulu";
    }
    break;

    case -540: // GMT-09:00
    {
      tzone = "America/Anchorage";
    }
    break;

    case -480: // GMT-08:00
    {
      tzone = "America/Los_Angeles";
    }
    break;

    case -420: // GMT-07:00
    {
      tzone = "America/Denver";
    }
    break;

    case -360: // GMT-06:00
    {
      tzone = "America/Chicago";
    }
    break;

    case -300: // GMT-05:00
    {
      tzone = "America/New_York";
    }
    break;

    case -240: // GMT-04:00
    {
      tzone = "America/Caracas";
    }
    break;

    case -210: // GMT-03:30
    {
      tzone = "America/St_Johns";
    }
    break;

    case -180: // GMT-03:00
    {
      tzone = "America/Argentina/Buenos_Aires";
    }
    break;

    case -120: // GMT-02:00
    {
      tzone = "America/Fortaleza";
    }
    break;

    case  -60: // GMT-01:00
    {
      tzone = "Atlantic/Azores";
    }
    break;

    default:
    case    0: // GMT 00:00
    {
      tzone = "Europe/London";
    }
    break;

    case   60: // GMT+01:00
    {
      tzone = "Europe/Berlin";
    }
    break;

    case  120: // GMT+02:00
    {
      tzone = "Europe/Helsinki";
    }
    break;

    case  180: // GMT+03:00
    {
      tzone = "Europe/Moscow";
    }
    break;

    case  210: // GMT+03:30
    {
      tzone = "Asia/Tehran";
    }
    break;

    case  240: // GMT+04:00
    {
      tzone = "Asia/Dubai";
    }
    break;

    case  270: // GMT+04:30
    {
      tzone = "Asia/Kabul";
    }
    break;

    case  300: // GMT+05:00
    {
      tzone = "Asia/Karachi";
    }
    break;

    case  330: // GMT+05:30
    {
      tzone = "Asia/Kolkata";
    }
    break;

    case  345: // GMT+05:45
    {
      tzone = "Asia/Kathmandu";
    }
    break;

    case  360: // GMT+06:00
    {
      tzone = "Asia/Tashkent";
    }
    break;

    case  390: // GMT+06:30
    {
      tzone = "Asia/Rangoon";
    }
    break;

    case  420: // GMT+07:00
    {
      tzone = "Asia/Bangkok";
    }
    break;

    case  480: // GMT+08:00
    {
      tzone = "Asia/Singapore";
    }
    break;

    case  540: // GMT+09:00
    {
      tzone = "Asia/Tokyo";
    }
    break;

    case  570: // GMT+09:30
    {
      tzone = "Australia/Adelaide";
    }
    break;

    case  600: // GMT+10:00
    {
      tzone = "Australia/Sydney";
    }
    break;

    case  660: // GMT+11:00
    {
      tzone = "Pacific/Noumea";
    }
    break;

    case  720: // GMT+12:00
    {
      tzone = "Pacific/Auckland";
    }
    break;

    case  780: // GMT+13:00
    {
      tzone = "Pacific/Tongatapu";
    }
    break;
  }

  RETURN(tzone);
  return tzone;
}

///
/// TZoneCleanup
// free the parsed tzone infos
void TZoneCleanup(void)
{
  struct TZoneContinent *cont;

  ENTER();

  while((cont = (struct TZoneContinent *)RemHead((struct List *)&G->tzoneContinentList)) != NULL)
  {
    struct TZoneLocation *loc;

    while((loc = (struct TZoneLocation *)RemHead((struct List *)&cont->locationList)) != NULL)
    {
      free(loc->name);
      FreeSysObject(ASOT_NODE, loc);
    }

    free(cont->name);
    FreeSysObject(ASOT_NODE, cont);
  }

  LEAVE();
}

///
