/***************************************************************************

 YAM - Yet Another Mailer
 Copyright (C) 1995-2000 by Marcel Beck <mbeck@yam.ch>
 Copyright (C) 2000-2010 by YAM Open Source Team

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
 YAM OpenSource project    : http://sourceforge.net/projects/yamos/

 $Id$

 Superclass:  MUIC_Dataspace
 Description: Dataspace object using a base64 encoded string in memory

***************************************************************************/

#include "Base64Dataspace_cl.h"

#include "mime/base64.h"

#include "Debug.h"

/* CLASSDATA
struct Data
{
  char *base64String;
};
*/

/* Private Functions */
/// EncodeData
// build a base64 encoded string of the data to be added looking like this:
// llllllll;iiiiiiii;ddddddddd......
// l = data length
// i = object id
// d = data
static char *EncodeData(APTR data, LONG len, ULONG id)
{
  char *base64String;

  ENTER();

  // our header has a size of 18
  // base64 encoding will expand each 3 bytes to 4 bytes plus up to 3 padding bytes
  // plus one byte for the trailing NUL
  if((base64String = malloc(18 + (len*4)/3+3 + 1)) != NULL)
  {
    snprintf(base64String, 18+1, "%08d;%08x;", (int)len, (int)id);
    base64encode(&base64String[18], data, len);
  }

  RETURN(base64String);
  return base64String;
}

///
/// DecodeData
static BOOL DecodeData(const char *base64String, APTR *pdata, LONG *plen, ULONG *pid)
{
  BOOL success = FALSE;

  ENTER();

  if(base64String != NULL)
  {
    char *dupe;

    // we are going to modify the string so we must operated on a copy
    if((dupe = strdup(base64String)) != NULL)
    {
      unsigned char *data = NULL;
      LONG len = 0;
      ULONG id = 0;
      char *word = dupe;
      char *next;
      int i = 0;

      // parse the string, there must be 3 substrings
      do
      {
        // split the string into words separated by semicolons
        if((next = strpbrk(word, ";")) != NULL)
          *next++ = '\0';

        switch(i)
        {
          case 0:
            len = strtol(word, NULL, 10);
          break;

          case 1:
            id = strtol(word, NULL, 16);
          break;

          case 2:
            data = (unsigned char *)word;
          break;
        }

        word = next;
        i++;
      }
      while(word != NULL);

      if(len > 0 && id != 0 && data != NULL)
      {
        char *raw;

        if((raw = malloc((len*3)/4)) != NULL)
        {
          // now convert the base64 string back to raw data
          // everything is ok if the decoded amount of data matches the predicted size
          if(base64decode(raw, data, strlen((char *)data)) == len)
          {
            *pdata = raw;
            *plen = len;
            *pid = id;

            success = TRUE;
          }
          else
            free(raw);
        }
      }

      free(dupe);
    }
  }

  RETURN(success);
  return success;
}

///
/// ImportBase64String
static BOOL ImportBase64String(struct IClass *cl, Object *obj, const char *base64String)
{
  GETDATA;
  BOOL success = FALSE;
  APTR rawData;
  LONG len;
  ULONG id;

  ENTER();

  // decode the string
  if(DecodeData(base64String, &rawData, &len, &id) == TRUE)
  {
    // now pass the data to our superclass
    if(DoSuperMethod(cl, obj, MUIM_Dataspace_Add, rawData, len, id) != (IPTR)NULL)
    {
      // free any previous contents and remember the new one
      free(data->base64String);
      data->base64String = strdup(base64String);

      success = TRUE;
    }

    // the parsed raw data are no longer needed
    free(rawData);
  }

  RETURN(success);
  return success;
}

///

/* Overloaded Methods */
/// OVERLOAD(OM_NEW)
OVERLOAD(OM_NEW)
{
  ENTER();

  if((obj = DoSuperNew(cl, obj,
    TAG_MORE, inittags(msg))) != NULL)
  {
    char *base64String;

    if((base64String = (char *)GetTagData(ATTR(Base64String), (IPTR)NULL, inittags(msg))) != NULL)
      ImportBase64String(cl, obj, base64String);
  }

  RETURN((IPTR)obj);
  return (IPTR)obj;
}

///
/// OVERLOAD(OM_DISPOSE)
OVERLOAD(OM_DISPOSE)
{
  GETDATA;

  free(data->base64String);
  data->base64String = NULL;

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
    case ATTR(Base64String):
    {
      // we always return a valid string
      if(data->base64String != NULL)
        *store = (IPTR)data->base64String;
      else
        *store = (IPTR)"0&0&";

      return TRUE;
    }
  }

  return DoSuperMethodA(cl, obj, msg);
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
      case ATTR(Base64String):
      {
        free(data->base64String);
        data->base64String = NULL;

        ImportBase64String(cl, obj, (char *)tag->ti_Data);

        tag->ti_Tag = TAG_IGNORE;
      }
      break;
    }
  }

  return DoSuperMethodA(cl, obj, msg);
}

///
/// OVERLOAD(MUIM_Dataspace_Add)
OVERLOAD(MUIM_Dataspace_Add)
{
  GETDATA;
  struct MUIP_Dataspace_Add *add = (struct MUIP_Dataspace_Add *)msg;
  char *base64String;
  APTR result = NULL;

  ENTER();

  // first build our private encoded string
  if((base64String = EncodeData(add->data, add->len, add->id)) != NULL)
  {
    // then let Dataspace.mui add the data for the given ID
    if((result = (APTR)DoSuperMethodA(cl, obj, msg)) != NULL)
    {
      free(data->base64String);
      data->base64String = base64String;
    }
    else
    {
      free(base64String);
    }
  }

  RETURN((IPTR)result);
  return (IPTR)result;
}

///
/// OVERLOAD(MUIM_Dataspace_Remove)
OVERLOAD(MUIM_Dataspace_Remove)
{
  GETDATA;
  APTR result;

  ENTER();

  if((result = (APTR)DoSuperMethodA(cl, obj, msg)) != NULL)
  {
    // erase the encoded string if removing the ID was successful
    free(data->base64String);
    data->base64String = NULL;
  }

  RETURN((IPTR)result);
  return (IPTR)result;
}

///
