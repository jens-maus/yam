/***************************************************************************

 YAM - Yet Another Mailer
 Copyright (C) 1995-2000 by Marcel Beck <mbeck@yam.ch>
 Copyright (C) 2000-2008 by YAM Open Source Team

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

#include <proto/exec.h>

#include "extrasrc.h"

#include "YAM.h"
#include "YAM_addressbook.h"
#include "YAM_addressbookEntry.h"

#include "Rexx.h"

#include "Debug.h"

struct rxd_addrinfo
{
  long rc, rc2;
  struct
  {
    char *var, *stem;
    char *alias;
  } arg;
  struct
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
  } res;
};

void rx_addrinfo(UNUSED struct RexxHost *host, void **rxd, enum RexxAction action, UNUSED struct RexxMsg *rexxmsg)
{
  struct
  {
    struct rxd_addrinfo rd;
    char *members, **memberptr;
  } *rd = *rxd;

  ENTER();

  switch(action)
  {
    case RXIF_INIT:
    {
      if((*rxd = AllocVecPooled(G->SharedMemPool, sizeof(*rd))) != NULL)
        ((struct rxd_addrinfo *)(*rxd))->rc = offsetof(struct rxd_addrinfo, res) / sizeof(long);
    }
    break;

    case RXIF_ACTION:
    {
      struct ABEntry *ab = NULL;

      if(AB_SearchEntry(rd->rd.arg.alias, ASM_ALIAS|ASM_USER|ASM_LIST|ASM_GROUP, &ab) && (ab != NULL))
      {
        switch(ab->Type)
        {
          case AET_USER:
            rd->rd.res.type = "P";
          break;

          case AET_LIST:
            rd->rd.res.type = "L";
          break;

          case AET_GROUP:
            rd->rd.res.type = "G";
          break;
        }

        rd->rd.res.name = ab->RealName;
        rd->rd.res.email = ab->Address;
        rd->rd.res.pgp = ab->PGPId;
        rd->rd.res.homepage = ab->Homepage;
        rd->rd.res.street = ab->Street;
        rd->rd.res.city = ab->City;
        rd->rd.res.country = ab->Country;
        rd->rd.res.phone = ab->Phone;
        rd->rd.res.comment = ab->Comment;
        rd->rd.res.birthdate = &ab->BirthDay;
        rd->rd.res.image = ab->Photo;

        if(ab->Members && (rd->members = strdup(ab->Members)))
        {
          char *ptr;
          int i;
          int j;

          for(j = 0, ptr = rd->members; *ptr; j++, ptr++)
          {
            if((ptr = strchr(ptr, '\n')))
              *ptr = '\0';
            else
              break;
          }

          rd->rd.res.members = rd->memberptr = calloc(j+1, sizeof(char *));
          for(i = 0, ptr = rd->members; i < j; ptr += strlen(ptr)+1)
            rd->memberptr[i++] = ptr;
        }
      }
      else
        rd->rd.rc = RETURN_ERROR;
    }
    break;

    case RXIF_FREE:
    {
      if (rd->members) free(rd->members);
      if (rd->memberptr) free(rd->memberptr);

      FreeVecPooled(G->SharedMemPool, rd);
    }
    break;
  }

  LEAVE();
}
