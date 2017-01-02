/***************************************************************************

 YAM - Yet Another Mailer
 Copyright (C) 1995-2000 Marcel Beck
 Copyright (C) 2000-2017 YAM Open Source Team

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
#include <strings.h>

#include <clib/alib_protos.h>
#include <mui/NListtree_mcc.h>
#include <proto/exec.h>

#include "extrasrc.h"

#include "YAM.h"

#include "mui/ClassesExtra.h"
#include "mui/AddressBookWindow.h"

#include "DynamicString.h"
#include "MUIObjects.h"
#include "Rexx.h"

#include "Debug.h"

struct args
{
  char *alias;
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
  char **member;
  long add;
};

void rx_addredit(UNUSED struct RexxHost *host, struct RexxParams *params, enum RexxAction action, UNUSED struct RexxMsg *rexxmsg)
{
  struct args *args = params->args;

  ENTER();

  switch(action)
  {
    case RXIF_INIT:
    {
      params->args = AllocVecPooled(G->SharedMemPool, sizeof(*args));
    }
    break;

    case RXIF_ACTION:
    {
      struct ABookNode *abn;

      abn = G->abook.arexxABN;
      if(abn == NULL && G->ABookWinObject != NULL)
        abn = (struct ABookNode *)xget(G->ABookWinObject, MUIA_AddressBookWindow_ActiveEntry);

      SHOWVALUE(DBF_ABOOK, abn);
      if(abn != NULL)
      {
        if(args->alias != NULL)
          strlcpy(abn->Alias, args->alias, sizeof(abn->Alias));
        if(args->name != NULL)
          strlcpy(abn->RealName, args->name, sizeof(abn->RealName));
        if(args->email != NULL)
          strlcpy(abn->Address, args->email, sizeof(abn->Address));
        if(args->pgp != NULL)
          strlcpy(abn->PGPId, args->pgp, sizeof(abn->PGPId));
        if(args->homepage != NULL)
          strlcpy(abn->Homepage, args->homepage, sizeof(abn->Homepage));
        if(args->street != NULL)
          strlcpy(abn->Street, args->street, sizeof(abn->Street));
        if(args->city != NULL)
          strlcpy(abn->City, args->city, sizeof(abn->City));
        if(args->country != NULL)
          strlcpy(abn->Country, args->country, sizeof(abn->Country));
        if(args->phone != NULL)
          strlcpy(abn->Phone, args->phone, sizeof(abn->Phone));
        if(args->comment != NULL)
          strlcpy(abn->Comment, args->comment, sizeof(abn->Comment));

        if(args->birthdate != NULL)
        {
          char dateStr[SIZE_SMALL];

          // if the user supplied 0 as the birthdate, he wants to "delete" the current
          // birthdate
          if(*args->birthdate == 0)
            abn->Birthday = 0;
          else if(BirthdayToString(*args->birthdate, dateStr, sizeof(dateStr)) == TRUE)
            abn->Birthday = *args->birthdate;
          else
          {
            params->rc = RETURN_ERROR;
            break;
          }
        }

        if(args->image != NULL)
          strlcpy(abn->Photo, args->image, sizeof(abn->Photo));

        if(args->member != NULL && abn->type == ABNT_LIST)
        {
          char **p;

          // free the list members in case we are told to replace them
          if(args->add == FALSE)
          {
            dstrfree(abn->ListMembers);
            abn->ListMembers = NULL;
          }

          for(p = args->member; *p != NULL; p++)
          {
            dstrcat(&abn->ListMembers, *p);
            dstrcat(&abn->ListMembers, "\n");
          }
        }

        G->abook.arexxABN = abn;
        G->abook.modified = TRUE;

        // update an existing address book window as well
        if(G->ABookWinObject != NULL)
          DoMethod(G->ABookWinObject, MUIM_AddressBookWindow_RedrawActiveEntry);
      }
      else
        params->rc = RETURN_ERROR;
    }
    break;

    case RXIF_FREE:
    {
      if(args != NULL)
        FreeVecPooled(G->SharedMemPool, args);
    }
    break;
  }

  LEAVE();
}
