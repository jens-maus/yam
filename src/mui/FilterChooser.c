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

 Superclass:  MUIC_Cycle
 Description: Cycle object to choose a filter

***************************************************************************/

#include <stdlib.h>
#include <string.h>

#include <proto/dos.h>

#include "FilterChooser_cl.h"

#include "YAM_utilities.h"

#include "MUIObjects.h"

#include "Debug.h"

/* CLASSDATA
struct Data
{
  char *searchPath;   // path to search for filter descriptions
  char **filterArray; // names of the available filters
  int numFilters;     // number of found filters
};
*/

/* Overloaded Methods */
/// OVERLOAD(OM_NEW)
OVERLOAD(OM_NEW)
{
  #if defined(__AROS__)
  // Zune must be provided a valid MUIA_Cycle_Entries pointer
  static const char *dummy[] = { "", NULL };
  #endif

  ENTER();

  if((obj = DoSuperNew(cl, obj,

    MUIA_CycleChain,    TRUE,
    MUIA_Font,          MUIV_Font_Button,
    #if defined(__AROS__)
    MUIA_Cycle_Entries, dummy,
    #endif

    TAG_MORE, inittags(msg))) != NULL)
  {
    GETDATA;

    data->searchPath = (char *)GetTagData(ATTR(SearchPath), (IPTR)"PROGDIR:Resources/spamfilters", inittags(msg));

    // search for available filter descriptions
    DoMethod(obj, METHOD(UpdateFilters));
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
  FreeStrArray(data->filterArray);

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

  while((tag = NextTagItem((APTR)&tags)) != NULL)
  {
    switch(tag->ti_Tag)
    {
      case ATTR(Filter):
      {
        const char *filter = (const char *)tag->ti_Data;
        ULONG try = 0;
        BOOL foundMatch;

        do
        {
          LONG idx = 0;

          foundMatch = FALSE;

          while(data->filterArray[idx] != NULL)
          {
            if(strcmp(data->filterArray[idx], filter) == 0)
            {
              nnset(obj, MUIA_Cycle_Active, idx);
              foundMatch = TRUE;
              break;
            }

            idx++;
          }

          if(foundMatch == FALSE)
          {
            if(try == 0)
            {
              // fall back to SpamAssassin if the requested filter does not exist
              filter = "SpamAssassin";
            }
            try++;
          }
        }
        while(try < 2 && foundMatch == FALSE);

        if(foundMatch == FALSE)
          nnset(obj, MUIA_Cycle_Active, 0);
      }
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
    case ATTR(Filter):
    {
      // return the selected filter's name or an empty string if no filters are available
      if(data->numFilters != 0)
        *store = (IPTR)data->filterArray[xget(obj, MUIA_Cycle_Active)];
      else
        *store = (IPTR)"";

      return TRUE;
    }
  }

  return DoSuperMethodA(cl, obj, msg);
}

///

/* Private Functions */
static int compareFilters(const void *f1, const void *f2)
{
  return stricmp((const char *)f1, (const char *)f2);
}

/* Public Methods */
/// DECLARE(UpdateFilters)
// updates the str array containing all filter descriptions
DECLARE(UpdateFilters)
{
  GETDATA;

  ENTER();

  // free the previous array
  FreeStrArray(data->filterArray);

  // count the number of *.sfd files
  data->numFilters = FileCount(data->searchPath, "#?.sfd");

  // allocate an array at least 2 entries
  if((data->filterArray = calloc(MAX(2, data->numFilters+1), sizeof(char *))) != NULL)
  {
    // set a single empty entry if no files were found
    // this works around a bug in MUI4 of MorphOS which uses a non-static
    // replacement entry on the stack instead otherwise
    if(data->numFilters == 0)
    {
      data->filterArray[0] = strdup("");
    }
    else
    {
      ULONG parsedPatternSize = strlen("#?.sfd") * 2 + 2;
      char *parsedPattern;

      if((parsedPattern = malloc(parsedPatternSize)) != NULL)
      {
        APTR context;

        ParsePatternNoCase("#?.sfd", parsedPattern, parsedPatternSize);

        if((context = ObtainDirContextTags(EX_StringName, (IPTR)data->searchPath,
                                           EX_DataFields, EXF_TYPE|EXF_NAME,
                                           EX_MatchString, (IPTR)parsedPattern,
                                           TAG_DONE)) != NULL)
        {
          struct ExamineData *ed;
          ULONG filterIndex = 0;
          LONG error;

          while((ed = ExamineDir(context)) != NULL)
          {
            if(EXD_IS_FILE(ed) && (data->filterArray[filterIndex] = strdup(ed->Name)) != NULL)
            {
              char *p;

              // strip the .sfd extension
              if((p = strcasestr(data->filterArray[filterIndex], ".sfd")) != NULL)
                *p = '\0';

              filterIndex++;
            }
          }

          // sort the list of filters
          qsort(data->filterArray, filterIndex, sizeof(char *), compareFilters);

          error = IoErr();
          if(error != 0 && error != ERROR_NO_MORE_ENTRIES)
          {
            E(DBF_ALWAYS, "%s failed, error %ld", __FUNCTION__, error);
         }

          ReleaseDirContext(context);
        }

        free(parsedPattern);
      }
    }

    // update the entry strings and set the active entry
    nnset(obj, MUIA_Cycle_Entries, data->filterArray);
  }

  RETURN(0);
  return 0;
}

///
