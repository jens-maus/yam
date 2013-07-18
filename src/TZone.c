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
  struct TZoneContinent *cont;
  struct TZoneContinent *found = NULL;

  ENTER();

  // search for the continent first
  for(cont = (struct TZoneContinent *)GetHead((struct List *)&G->tzoneContinentList);
      cont != NULL;
      cont = (struct TZoneContinent *)GetSucc((struct Node *)cont))
  {
    if(strcmp(cont->name, contName) == 0)
    {
      found = cont;
      break;
    }
  }

  // create a new continent if it was not found
  if(found == NULL)
  {
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

    if((loc = AllocSysObjectTags(ASOT_NODE,
      ASONODE_Size, sizeof(*loc),
      ASONODE_Min, TRUE,
      TAG_DONE)) != NULL)
    {
      loc->name = strdup(locName);

      AddTail((struct List *)&found->locationList, (struct Node *)loc);
    }
  }

  LEAVE();
}

///
/// compareContinents
static int compareContinents(const struct Node *n1, const struct Node *n2)
{
  return strcmp(((struct TZoneContinent *)n1)->name, ((struct TZoneContinent *)n2)->name);
}

///
/// compareLocations
static int compareLocations(const struct Node *n1, const struct Node *n2)
{
  return strcmp(((struct TZoneLocation *)n1)->name, ((struct TZoneLocation *)n2)->name);
}

///
/// sortLocations
// sort all locations alphabetically
static void sortLocations(void)
{
  struct TZoneContinent *cont;

  ENTER();

  // sort the continents first
  SortExecList((struct List *)&G->tzoneContinentList, compareContinents);

  // then sort the locations of each continent
  for(cont = (struct TZoneContinent *)GetHead((struct List *)&G->tzoneContinentList);
      cont != NULL;
      cont = (struct TZoneContinent *)GetSucc((struct Node *)cont))
  {
    SortExecList((struct List *)&cont->locationList, compareLocations);
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
            addLocation(continent, cityOrState);
		}
	  }
    }

    free(buf);
    fclose(fh);

    // sort all locations
    sortLocations();
  }

  LEAVE();
}

///
/// BuildContinentEntries
// set up an array of strings to be used with a Cycle object
char **BuildContinentEntries(void)
{
  char **entries;
  size_t count;
  struct TZoneContinent *cont;

  ENTER();

  // free any previous array first
  free(G->tzoneContinentEntries);

  // count the continents
  count = CountNodes(&G->tzoneContinentList);
  if((entries = malloc((count+1) * sizeof(char *))) != NULL)
  {
    char **ptr = entries;

    // copy all continent names
    for(cont = (struct TZoneContinent *)GetHead((struct List *)&G->tzoneContinentList);
        cont != NULL;
        cont = (struct TZoneContinent *)GetSucc((struct Node *)cont))
    {
      *ptr++ = cont->name;
    }

    // terminate the array with a NULL entry
    *ptr = NULL;
  }

  G->tzoneContinentEntries = entries;

  RETURN(entries);
  return entries;
}

///
/// BuildLocationEntries
// set up an array of strings to be used with a Cycle object
char **BuildLocationEntries(int contNumber)
{
  char **entries = NULL;
  int i;
  struct TZoneContinent *cont;

  ENTER();

  // free any previous array first
  free(G->tzoneLocationEntries);

  i = 0;
  for(cont = (struct TZoneContinent *)GetHead((struct List *)&G->tzoneContinentList);
      cont != NULL;
      cont = (struct TZoneContinent *)GetSucc((struct Node *)cont))
  {
    if(i == contNumber)
      break;
    i++;
  }

  if(cont != NULL)
  {
    if((entries = malloc((cont->numLocations+1) * sizeof(char *))) != NULL)
    {
      char **ptr = entries;
      struct TZoneLocation *loc;

      // copy all location names
      for(loc = (struct TZoneLocation *)GetHead((struct List *)&cont->locationList);
          loc != NULL;
          loc = (struct TZoneLocation *)GetSucc((struct Node *)loc))
      {
        *ptr++ = loc->name;
      }

      // terminate the array with a NULL entry
      *ptr = NULL;
    }
  }

  G->tzoneLocationEntries = entries;

  RETURN(entries);
  return entries;
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

  free(G->tzoneContinentEntries);
  free(G->tzoneLocationEntries);

  LEAVE();
}

///
