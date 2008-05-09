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

#include <clib/alib_protos.h>
#include <mui/NListtree_mcc.h>
#include <proto/exec.h>

#include "extrasrc.h"

#include "YAM.h"
#include "YAM_addressbook.h"
#include "YAM_addressbookEntry.h"
#include "classes/Classes.h"

#include "Rexx.h"

#include "Debug.h"

struct rxd_addredit
{
  long rc, rc2;
  struct {
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
  } arg;
};

void rx_addredit(UNUSED struct RexxHost *host, void **rxd, enum RexxAction action, UNUSED struct RexxMsg *rexxmsg)
{
  struct rxd_addredit *rd = *rxd;

  ENTER();

  switch(action)
  {
    case RXIF_INIT:
    {
      if((*rxd = AllocVecPooled(G->SharedMemPool, sizeof(*rd))) != NULL)
        ((struct rxd_addredit *)(*rxd))->rc = 0;
    }
    break;

    case RXIF_ACTION:
    {
      struct MUI_NListtree_TreeNode *tn;

      if((tn = (struct MUI_NListtree_TreeNode *)xget(G->AB->GUI.LV_ADDRESSES, MUIA_NListtree_Active)))
      {
        struct ABEntry *ab = (struct ABEntry *)(tn->tn_User);

        if(rd->arg.alias)    strlcpy(ab->Alias, rd->arg.alias, sizeof(ab->Alias));
        if(rd->arg.name)     strlcpy(ab->RealName, rd->arg.name, sizeof(ab->RealName));
        if(rd->arg.email)    strlcpy(ab->Address, rd->arg.email, sizeof(ab->Address));
        if(rd->arg.pgp)      strlcpy(ab->PGPId, rd->arg.pgp, sizeof(ab->PGPId));
        if(rd->arg.homepage) strlcpy(ab->Homepage, rd->arg.homepage, sizeof(ab->Homepage));
        if(rd->arg.street)   strlcpy(ab->Street, rd->arg.street, sizeof(ab->Street));
        if(rd->arg.city)     strlcpy(ab->City, rd->arg.city, sizeof(ab->City));
        if(rd->arg.country)  strlcpy(ab->Country, rd->arg.country, sizeof(ab->Country));
        if(rd->arg.phone)    strlcpy(ab->Phone, rd->arg.phone, sizeof(ab->Phone));
        if(rd->arg.comment)  strlcpy(ab->Comment, rd->arg.comment, sizeof(ab->Comment));

        if(rd->arg.birthdate)
        {
          // if the user supplied 0 as the birthdate, he wants to "delete" the current
          // birthdate
          if(*rd->arg.birthdate == 0) ab->BirthDay = 0;
          else if(AB_ExpandBD(*rd->arg.birthdate)[0]) ab->BirthDay = *rd->arg.birthdate;
          else
          {
            rd->rc = RETURN_ERROR;
            break;
          }
        }

        if(rd->arg.image)
          strlcpy(ab->Photo, rd->arg.image, sizeof(ab->Photo));

        if(rd->arg.member && ab->Type == AET_LIST)
        {
           char **p, *memb = AllocStrBuf(SIZE_DEFAULT);
           if (rd->arg.add && ab->Members) memb = StrBufCpy(memb, ab->Members);
           for (p = rd->arg.member; *p; p++) { memb = StrBufCat(memb, *p); memb = StrBufCat(memb, "\n"); }
           if (ab->Members) free(ab->Members);
           ab->Members = strdup(memb);
           FreeStrBuf(memb);
        }

        DoMethod(G->AB->GUI.LV_ADDRESSES, MUIM_NListtree_Redraw, MUIV_NListtree_Redraw_Active, MUIF_NONE);
        G->AB->Modified = TRUE;
      }
      else
        rd->rc = RETURN_ERROR;
    }
    break;

    case RXIF_FREE:
    {
      FreeVecPooled(G->SharedMemPool, rd);
    }
    break;
  }

  LEAVE();
}
