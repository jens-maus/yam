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

#include <stdlib.h>
#include <strings.h>

#include <clib/alib_protos.h>
#include <mui/NListtree_mcc.h>
#include <proto/exec.h>

#include "extrasrc.h"

#include "YAM.h"
#include "YAM_addressbook.h"
#include "YAM_addressbookEntry.h"

#include "mui/ClassesExtra.h"

#include "MUIObjects.h"
#include "Rexx.h"
#include "StrBuf.h"

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
      struct MUI_NListtree_TreeNode *tn;

      if((tn = (struct MUI_NListtree_TreeNode *)xget(G->AB->GUI.LV_ADDRESSES, MUIA_NListtree_Active)) != NULL)
      {
        struct ABEntry *ab = (struct ABEntry *)(tn->tn_User);

        if(args->alias != NULL)
          strlcpy(ab->Alias, args->alias, sizeof(ab->Alias));
        if(args->name != NULL)
          strlcpy(ab->RealName, args->name, sizeof(ab->RealName));
        if(args->email != NULL)
          strlcpy(ab->Address, args->email, sizeof(ab->Address));
        if(args->pgp != NULL)
          strlcpy(ab->PGPId, args->pgp, sizeof(ab->PGPId));
        if(args->homepage != NULL)
          strlcpy(ab->Homepage, args->homepage, sizeof(ab->Homepage));
        if(args->street != NULL)
          strlcpy(ab->Street, args->street, sizeof(ab->Street));
        if(args->city != NULL)
          strlcpy(ab->City, args->city, sizeof(ab->City));
        if(args->country != NULL)
          strlcpy(ab->Country, args->country, sizeof(ab->Country));
        if(args->phone != NULL)
          strlcpy(ab->Phone, args->phone, sizeof(ab->Phone));
        if(args->comment != NULL)
          strlcpy(ab->Comment, args->comment, sizeof(ab->Comment));

        if(args->birthdate != NULL)
        {
          char dateStr[SIZE_SMALL];

          // if the user supplied 0 as the birthdate, he wants to "delete" the current
          // birthdate
          if(*args->birthdate == 0)
            ab->BirthDay = 0;
          else if(AB_ExpandBD(*args->birthdate, dateStr, sizeof(dateStr)) == TRUE)
            ab->BirthDay = *args->birthdate;
          else
          {
            params->rc = RETURN_ERROR;
            break;
          }
        }

        if(args->image != NULL)
          strlcpy(ab->Photo, args->image, sizeof(ab->Photo));

        if(args->member != NULL && ab->Type == AET_LIST)
        {
          char *memb = NULL;
          char **p;

          if(args->add && ab->Members != NULL)
            StrBufCpy(&memb, ab->Members);

          for(p = args->member; *p; p++)
          {
            StrBufCat(&memb, *p);
            StrBufCat(&memb, "\n");
          }

          free(ab->Members);
          ab->Members = strdup(memb);

          FreeStrBuf(memb);
        }

        DoMethod(G->AB->GUI.LV_ADDRESSES, MUIM_NListtree_Redraw, MUIV_NListtree_Redraw_Active, MUIF_NONE);
        G->AB->Modified = TRUE;
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
