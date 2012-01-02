/***************************************************************************

 YAM - Yet Another Mailer
 Copyright (C) 1995-2000 Marcel Beck
 Copyright (C) 2000-2012 YAM Open Source Team

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

#include <strings.h>
#include <stdlib.h>

#include <proto/exec.h>

#include "extrasrc.h"

#include "YAM.h"
#include "YAM_addressbook.h"
#include "YAM_addressbookEntry.h"

#include "Rexx.h"

#include "Debug.h"

struct args
{
  struct RexxResult varStem;
  char *alias;
};

struct results
{
  const char *type;
  char *name;
  char *email;
  char *pgp;
  char *homepage;
  char *street;
  char *city;
  char *country;
  char *phone;
  char *comment;
  long *birthdate;
  char *image;
  char **members;
};

struct optional
{
  char *members;
  char **memberptr;
};

void rx_addrinfo(UNUSED struct RexxHost *host, struct RexxParams *params, enum RexxAction action, UNUSED struct RexxMsg *rexxmsg)
{
  struct args *args = params->args;
  struct results *results = params->results;
  struct optional *optional = params->optional;

  ENTER();

  switch(action)
  {
    case RXIF_INIT:
    {
      params->args = AllocVecPooled(G->SharedMemPool, sizeof(*args));
      params->results = AllocVecPooled(G->SharedMemPool, sizeof(*results));
      params->optional = AllocVecPooled(G->SharedMemPool, sizeof(*optional));
    }
    break;

    case RXIF_ACTION:
    {
      struct ABEntry *ab = NULL;

      if(AB_SearchEntry(args->alias, ASM_ALIAS|ASM_USER|ASM_LIST|ASM_GROUP, &ab) && (ab != NULL))
      {
        switch(ab->Type)
        {
          case AET_USER:
            results->type = "P";
          break;

          case AET_LIST:
            results->type = "L";
          break;

          case AET_GROUP:
            results->type = "G";
          break;
        }

        results->name = ab->RealName;
        results->email = ab->Address;
        results->pgp = ab->PGPId;
        results->homepage = ab->Homepage;
        results->street = ab->Street;
        results->city = ab->City;
        results->country = ab->Country;
        results->phone = ab->Phone;
        results->comment = ab->Comment;
        results->birthdate = &ab->BirthDay;
        results->image = ab->Photo;

        if(ab->Members && (optional->members = strdup(ab->Members)))
        {
          char *ptr;
          int i;
          int j;

          for(j = 0, ptr = optional->members; *ptr; j++, ptr++)
          {
            if((ptr = strchr(ptr, '\n')))
              *ptr = '\0';
            else
              break;
          }

          results->members = optional->memberptr = calloc(j+1, sizeof(char *));
          for(i = 0, ptr = optional->members; i < j; ptr += strlen(ptr)+1)
            optional->memberptr[i++] = ptr;
        }
      }
      else
        params->rc = RETURN_ERROR;
    }
    break;

    case RXIF_FREE:
    {
      if(args != NULL)
        FreeVecPooled(G->SharedMemPool, args);
      if(results != NULL)
        FreeVecPooled(G->SharedMemPool, results);
      if(optional != NULL)
      {
        free(optional->members);
        free(optional->memberptr);
        FreeVecPooled(G->SharedMemPool, optional);
      }
    }
    break;
  }

  LEAVE();
}
