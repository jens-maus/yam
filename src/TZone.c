/***************************************************************************

 YAM - Yet Another Mailer
 Copyright (C) 1995-2000 Marcel Beck
 Copyright (C) 2000-2015 YAM Open Source Team

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

#include <clib/alib_protos.h>
#include <proto/exec.h>
#include <proto/dos.h>

#include "extrasrc.h"

#include "YAM.h"
#include "YAM_error.h"
#include "YAM_global.h"
#include "YAM_utilities.h"

#include "Config.h"
#include "FileInfo.h"
#include "Locale.h"
#include "TZone.h"

#include "Debug.h"

/// addLocation
// add a new location and a new continent if necessary
static void addLocation(const char *contName, const char *locName, const char *locComment)
{
  struct TZoneContinent *found = NULL;
  struct TZoneContinent *cont;

  ENTER();

  // search for the continent first
  IterateList(&G->tzoneContinentList, struct TZoneContinent *, cont)
  {
    if(strcasecmp(cont->name, contName) == 0)
    {
      found = cont;
      break;
    }
  }

  // create a new continent node if it was not found
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

    // create a new location node
    if((loc = AllocSysObjectTags(ASOT_NODE,
      ASONODE_Size, sizeof(*loc),
      ASONODE_Min, TRUE,
      TAG_DONE)) != NULL)
    {
      loc->name = strdup(locName);

      if(locComment != NULL)
        loc->comment = strdup(locComment);
      else
        loc->comment = NULL;

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
  struct TZoneContinent *cont;

  ENTER();

  // sort the continents first
  SortExecList(&G->tzoneContinentList, compareContinents);

  // then sort the locations of each continent
  IterateList(&G->tzoneContinentList, struct TZoneContinent *, cont)
  {
    SortExecList(&cont->locationList, compareLocations);
  }

  LEAVE();
}

///
/// ParseZoneTabFile
// parse the zone.zab file into continents and cities
BOOL ParseZoneTabFile(void)
{
  BOOL success = FALSE;
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
        #endif
        char *comments = NULL;
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
              comments = p;
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

            addLocation(continent, cityOrState, comments);
          }
        }
      }
    }

    free(buf);

    fclose(fh);

    // sort all locations
    sortLocations();

    success = TRUE;
  }

  RETURN(success);
  return success;
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
      struct TZoneContinent *cont;
      char **ptr = entries;

      // copy all continent names
      IterateList(&G->tzoneContinentList, struct TZoneContinent *, cont)
      {
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
      struct TZoneLocation *loc;

      // copy all location names
      IterateList(&cont->locationList, struct TZoneLocation *, loc)
      {
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

  D(DBF_TZONE, "continent index %ld, location index %ld", continent, location);
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

  D(DBF_TZONE, "built tzone name '%s'", name);

  RETURN(name);
  return name;
}

///
/// FindContinent
// find a continent by name
static struct TZoneContinent *findContinent(const char *continent, ULONG *cindex)
{
  struct TZoneContinent *result = NULL;
  char *contStr;

  ENTER();

  // convert all underscores to spaces
  if((contStr = strdup(continent)) != NULL)
  {
    char *p = contStr;
    struct TZoneContinent *cont;
    ULONG i;

    while((p = strchr(p, '_')) != NULL)
      *p++ = ' ';

    i = 0;
    IterateList(&G->tzoneContinentList, struct TZoneContinent *, cont)
    {
      if(strcasecmp(cont->name, contStr) == 0)
      {
        result = cont;
        *cindex = i;
        break;
      }

      i++;
    }

    free(contStr);
  }

  RETURN(result);
  return result;
}

///
/// FindLocation
// find a location on a continent by name
static struct TZoneLocation *findLocation(struct TZoneContinent *continent, const char *location, ULONG *lindex)
{
  struct TZoneLocation *result = NULL;
  char *locStr;

  ENTER();

  // convert all underscores to spaces
  if((locStr = strdup(location)) != NULL)
  {
    char *p = locStr;
    struct TZoneLocation *loc;
    ULONG i;

    while((p = strchr(p, '_')) != NULL)
      *p++ = ' ';

    i = 0;
    IterateList(&continent->locationList, struct TZoneLocation *, loc)
    {
      if(strcasecmp(loc->name, locStr) == 0)
      {
        result = loc;
        *lindex = i;
        break;
      }

      i++;
    }

    free(locStr);
  }

  RETURN(result);
  return result;
}

///
/// ParseTZoneName
// parse a time zone name into continent and location index with the respective lists
BOOL ParseTZoneName(const char *tzone, ULONG *continent, ULONG *location, char **comment)
{
  BOOL result = FALSE;
  char *tmp;

  ENTER();

  D(DBF_TZONE, "parse tzone name '%s'", tzone);
  if((tmp = strdup(tzone)) != NULL)
  {
    char *p;

    if((p = strchr(tmp, '/')) != NULL)
    {
      struct TZoneContinent *cont;

      // split the two parts
      *p++ = '\0';

      D(DBF_TZONE, "continent '%s', location '%s'", tmp, p);
      if((cont = findContinent(tmp, continent)) != NULL)
      {
        struct TZoneLocation *loc;

        if((loc = findLocation(cont, p, location)) != NULL)
        {
          if(comment != NULL)
            *comment = loc->comment;

          D(DBF_TZONE, "continent index %ld, location index %ld", *continent, *location);
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
const char *GuessTZone(const int gmtOffset)
{
  const char *tzone;

  ENTER();

  switch(gmtOffset)
  {
    case -720: // GMT-12:00
      tzone = "Pacific/Kwajalein";
    break;

    case -660: // GMT-11:00
      tzone = "Pacific/Midway";
    break;

    case -600: // GMT-10:00
      tzone = "Pacific/Honolulu";
    break;

    case -540: // GMT-09:00
      tzone = "America/Anchorage";
    break;

    case -480: // GMT-08:00
      tzone = "America/Los_Angeles";
    break;

    case -420: // GMT-07:00
      tzone = "America/Denver";
    break;

    case -360: // GMT-06:00
      tzone = "America/Chicago";
    break;

    case -300: // GMT-05:00
      tzone = "America/New_York";
    break;

    case -240: // GMT-04:00
      tzone = "America/Caracas";
    break;

    case -210: // GMT-03:30
      tzone = "America/St_Johns";
    break;

    case -180: // GMT-03:00
      tzone = "America/Argentina/Buenos_Aires";
    break;

    case -120: // GMT-02:00
      tzone = "America/Fortaleza";
    break;

    case  -60: // GMT-01:00
      tzone = "Atlantic/Azores";
    break;

    default:
    case    0: // GMT 00:00
      tzone = "Europe/London";
    break;

    case   60: // GMT+01:00
      tzone = "Europe/Berlin";
    break;

    case  120: // GMT+02:00
      tzone = "Europe/Helsinki";
    break;

    case  180: // GMT+03:00
      tzone = "Europe/Moscow";
    break;

    case  210: // GMT+03:30
      tzone = "Asia/Tehran";
    break;

    case  240: // GMT+04:00
      tzone = "Asia/Dubai";
    break;

    case  270: // GMT+04:30
      tzone = "Asia/Kabul";
    break;

    case  300: // GMT+05:00
      tzone = "Asia/Karachi";
    break;

    case  330: // GMT+05:30
      tzone = "Asia/Kolkata";
    break;

    case  345: // GMT+05:45
      tzone = "Asia/Kathmandu";
    break;

    case  360: // GMT+06:00
      tzone = "Asia/Tashkent";
    break;

    case  390: // GMT+06:30
      tzone = "Asia/Rangoon";
    break;

    case  420: // GMT+07:00
      tzone = "Asia/Bangkok";
    break;

    case  480: // GMT+08:00
      tzone = "Asia/Singapore";
    break;

    case  540: // GMT+09:00
      tzone = "Asia/Tokyo";
    break;

    case  570: // GMT+09:30
      tzone = "Australia/Adelaide";
    break;

    case  600: // GMT+10:00
      tzone = "Australia/Sydney";
    break;

    case  660: // GMT+11:00
      tzone = "Pacific/Noumea";
    break;

    case  720: // GMT+12:00
      tzone = "Pacific/Auckland";
    break;

    case  780: // GMT+13:00
      tzone = "Pacific/Tongatapu";
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
  struct TZoneContinent *nextc;

  ENTER();

  SafeIterateList(&G->tzoneContinentList, struct TZoneContinent *, cont, nextc)
  {
    struct TZoneLocation *loc;
    struct TZoneLocation *nextl;

    SafeIterateList(&cont->locationList, struct TZoneLocation *, loc, nextl)
    {
      free(loc->name);
      free(loc->comment);

      FreeSysObject(ASOT_NODE, loc);
    }
    NewMinList(&cont->locationList);

    free(cont->name);
    FreeSysObject(ASOT_NODE, cont);
  }
  NewMinList(&G->tzoneContinentList);

  LEAVE();
}

///
/// FindNextDSTSwitch()
time_t FindNextDSTSwitch(const char *tzone, struct TimeVal *tv)
{
  time_t result = 0;
  struct TM newtm;
  struct TM lasttm;
  time_t now;
  time_t newt;
  int curdst=-1;
  time_t startt;
  time_t maxdiff;
  time_t step;
  int iter=0;

  ENTER();

  // during the next lines we try to mimic the behaviour of the zdump
  // tool provided by the tzcode package of iana.org. This tool allows
  // to output information on a timezone and we are interested when
  // the next DST switch will actually happen.

  // the user wants us to return information based on
  // another timezone than the default one.
  if(tzone != NULL)
    TZSet(tzone);

  // get current time
  time(&now);

  #define SECSPERMINUTE (60)
  #define SECSPERHOUR   (60*SECSPERMINUTE)
  #define SECSPERDAY    (24*SECSPERHOUR)
  #define SECSPERMONTH  (30*SECSPERDAY)

  // in the outer loop we iterate per DAY with a maximum of
  // one year
  startt = now;
  step = 3*SECSPERMONTH;
  // we look ahead half a year plus 3 month at most
  maxdiff = (365/2)*SECSPERDAY + step;

  #if defined(DEBUG)
  if(localtime_r(&now, &lasttm) != NULL)
  {
    newt = now + maxdiff;
    if(localtime_r(&newt, &newtm) != NULL)
    {
      struct TM fromUTC;
      struct TM toUTC;
      char fromTime[SIZE_DEFAULT];
      char toTime[SIZE_DEFAULT];

      gmtime_r(&now, &fromUTC);
      gmtime_r(&newt, &toUTC);
      strftime(fromTime, sizeof(fromTime), "%a, %d %b %Y %T %z", &fromUTC);
      strftime(toTime, sizeof(toTime), "%a, %d %b %Y %T %z", &toUTC);
      D(DBF_TZONE, "searching for DST switch between UTC %s %s and UTC %s %s", fromTime, lasttm.tm_zone, toTime, newtm.tm_zone);
    }
  }
  #endif

  for(newt=startt; maxdiff == 0 || (time_t)abs(newt - startt) < maxdiff; newt += step)
  {
    if(localtime_r(&newt, &newtm) != NULL)
    {
      #if defined(DEBUG)
      char gmtimestr[SIZE_DEFAULT];
      char lctimestr[SIZE_DEFAULT];
      struct TM gmtm;

      gmtime_r(&newt, &gmtm);
      strftime(lctimestr, sizeof(lctimestr), "%a, %d %b %Y %T %z", &newtm);
      strftime(gmtimestr, sizeof(gmtimestr), "%a, %d %b %Y %T %z", &gmtm);

      D(DBF_TZONE, "%ld: %s UTC = %s %s isdst=%d gmtoff=%ld", newt, gmtimestr, lctimestr, newtm.tm_zone, newtm.tm_isdst, newtm.tm_gmtoff);
      #endif

      if(curdst < 0)
        curdst = newtm.tm_isdst;
      else
      {
        if(newtm.tm_isdst != curdst)
        {
          D(DBF_TZONE, "%d. iteration result: %s", iter+1, gmtimestr);

          if(iter == 0)
          {
            // now we seem to have found the quarter of the year when the DST change happened.
            // in the next interation loop we now search backward in 1 month steps
            step = -(1*SECSPERMONTH);
          }
          else if(iter == 1)
          {
            // now we seem to have found the time of dst switch within 1 month
            // so lets iterate 1-day wise from here
            step = SECSPERDAY;
          }
          else if(iter == 2)
          {
            // now we seem to have found the day when the DST change happened.
            // in the next interation loop we now search backward in 1 hour steps
            step = -(1*SECSPERHOUR);
          }
          else if(iter == 3)
          {
            // now we seem to have found the time of dst switch within 1 hours
            // so lets iterate 10-minute wise from here
            step = 10*SECSPERMINUTE;
          }
          else if(iter == 4)
          {
            // now we seem to have found the time of dst switch within 10 minutes
            // so lets iterate minute wise from here
            step = -SECSPERMINUTE;
          }
          else if(iter == 5)
          {
            // now we seem to have found the time of dst switch within 3 minutes
            // so lets iterate second-wise from here
            step = 1;
          }
          else
          {
            // lets convert the final switch time to a struct TimeVal now
            if(tv != NULL)
            {
              tm2TimeVal(&lasttm, tv);

              // lasttm points to the second before the actual DST switch,
              // so lets set it +1
              tv->Seconds += 1;
            }

            result = startt;
            break;
          }

          startt = newt;
          maxdiff = 0; // maxdiff = 0 means run forever
          curdst = newtm.tm_isdst;
          iter++;
        }

        memcpy(&lasttm, &newtm, sizeof(lasttm));
      }
    }
    else
      break;
  }

  // set back the orginal timezone used
  if(tzone != NULL)
    TZSet(C->Location);

  RETURN(result);
  return result;
}

///
/// SetTZone
// function that sets a new location string "Europe/Berlin" as the new default
// and updates all global information accordingly.
void SetTZone(const char *location)
{
  struct TM tm;

  ENTER();

  // set the new location
  TZSet(location);

  // get the current date/time in struct tm format
  TimeVal2tm(NULL, &tm);

  // call mktime() so that struct tm will be set correctly
  if(mktime(&tm) != -1)
  {
    // copy the timezone abbreviation string and the
    // gmtoffset to our global structure
    strlcpy(G->tzAbbr, (tm.tm_zone != NULL) ? tm.tm_zone : "", sizeof(G->tzAbbr));
    G->gmtOffset = tm.tm_gmtoff / 60;
  }
  else
  {
    E(DBF_TZONE, "mktime() failed, errno=%ld\n", errno);
  }

  // some debug information/output
  D(DBF_TZONE, "new TimeZone: '%s'", location);
  D(DBF_TZONE, "new TimeZone abbreviation: '%s'", G->tzAbbr);
  D(DBF_TZONE, "new GMT offset: %d", G->gmtOffset);

  // find out when the next DST switch will happen and
  // save it in the global config accordingly.
  if(FindNextDSTSwitch(location, &G->nextDSTSwitch) <= 0)
  {
    memset(&G->nextDSTSwitch, 0, sizeof(G->nextDSTSwitch));
    D(DBF_TZONE, "no DST switch");

    StopTimer(TIMER_DSTSWITCH);
  }
  else
  {
    #if defined(DEBUG)
    char nextDSTstr[SIZE_DEFAULT];
    struct DateStamp ds;

    TimeVal2DateStamp(&G->nextDSTSwitch, &ds, TZC_NONE);
    DateStamp2RFCString(nextDSTstr, sizeof(nextDSTstr), &ds, G->gmtOffset, G->tzAbbr, FALSE);
    D(DBF_TZONE, "next DST switch @ %s", nextDSTstr);
    #endif

    // make sure the update the DSTSWITCH Timer accordingly
    RestartTimer(TIMER_DSTSWITCH, G->nextDSTSwitch.Seconds, G->nextDSTSwitch.Microseconds, TRUE);
  }

  LEAVE();
}

///
/// TZSet
// wrapper function for tzset() which checks for the existance of
// the corresponding location files in Resources/zoneinfo
void TZSet(const char *location)
{
  char locationFile[SIZE_PATHFILE];
  ULONG type;

  ENTER();

  AddPath(locationFile, G->ProgDir, "Resources/zoneinfo", sizeof(locationFile));
  AddPart(locationFile, location, sizeof(locationFile));

  // check if the file exists and really is a file (i.e. not a directory)
  if(ObtainFileInfo(locationFile, FI_TYPE, &type) == FALSE || type != FIT_FILE)
  {
    ER_NewError(tr(MSG_TZONE_LOCATION_FILE_MISSING), locationFile, G->ProgDir);

    // call tzset so that it is resets to GMT/UTC
    tzset("UTC");
  }
  else
  {
    // call the tzset() function of our libtz so that the timezone information
    // is setup accordingly.
    tzset(location);
  }

  LEAVE();
}

///
