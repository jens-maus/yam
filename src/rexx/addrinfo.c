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

***************************************************************************/

#include <strings.h>
#include <stdlib.h>

#include <proto/exec.h>

#include "extrasrc.h"

#include "YAM.h"

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
  ULONG *birthdate;
  char *image;
  char **members;
};

struct optional
{
  struct ABookNode abn;
  char **memberArray;
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
      if(params->optional == NULL)
        params->rc = RETURN_ERROR;
    }
    break;

    case RXIF_ACTION:
    {
      struct ABookNode *abn = NULL;

      if(SearchABook(&G->abook, args->alias, ASM_ALIAS|ASM_USER|ASM_LIST|ASM_GROUP, &abn) != 0)
      {
        switch(abn->type)
        {
          case ABNT_USER:
            results->type = "P";
          break;

          case ABNT_LIST:
            results->type = "L";
          break;

          case ABNT_GROUP:
            results->type = "G";
          break;
        }

        memcpy(&optional->abn, abn, sizeof(optional->abn));
        results->name = optional->abn.RealName;
        results->email = optional->abn.Address;
        results->pgp = optional->abn.PGPId;
        results->homepage = optional->abn.Homepage;
        results->street = optional->abn.Street;
        results->city = optional->abn.City;
        results->country = optional->abn.Country;
        results->phone = optional->abn.Phone;
        results->comment = optional->abn.Comment;
        results->birthdate = &optional->abn.Birthday;
        results->image = optional->abn.Photo;

        if(abn->ListMembers != NULL)
        {
          if((optional->abn.ListMembers = strdup(abn->ListMembers)) != NULL)
          {
            char *ptr;
            int j;

            for(j = 0, ptr = optional->abn.ListMembers; *ptr; j++, ptr++)
            {
              if((ptr = strchr(ptr, '\n')) != NULL)
                *ptr = '\0';
              else
                break;
            }

            if((optional->memberArray = calloc(j+1, sizeof(char *))) != NULL)
            {
              int i;

              for(i = 0, ptr = optional->abn.ListMembers; i < j; ptr += strlen(ptr)+1)
                optional->memberArray[i++] = ptr;

              results->members = optional->memberArray;
            }
            else
              params->rc = RETURN_ERROR;
          }
          else
            params->rc = RETURN_ERROR;
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
        free(optional->abn.ListMembers);
        free(optional->memberArray);
        FreeVecPooled(G->SharedMemPool, optional);
      }
    }
    break;
  }

  LEAVE();
}
