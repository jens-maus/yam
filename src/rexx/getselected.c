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

#include "extrasrc.h"

#include "YAM.h"
#include "YAM_main.h"
#include "YAM_mainFolder.h"
#include "classes/Classes.h"

#include "MailList.h"
#include "Rexx.h"

#include "Debug.h"

void rx_getselected(UNUSED struct RexxHost *host, void **rxd, enum RexxAction action, UNUSED struct RexxMsg *rexxmsg)
{
  struct rxd_getselected *rd = *rxd;

  ENTER();

  switch(action)
  {
    case RXIF_INIT:
    {
      *rxd = AllocVecPooled(G->SharedMemPool, sizeof(*rd));
    }
    break;

    case RXIF_ACTION:
    {
      struct MailList *mlist;
      Object *lv = (Object *)xget(G->MA->GUI.PG_MAILLIST, MUIA_MainMailListGroup_MainList);

      if((mlist = MA_CreateMarkedList(lv, FALSE)) != NULL)
      {
        if((rd->res.num = calloc(mlist->count + 1, sizeof(long))))
        {
          struct MailNode *mnode;
          ULONG i;

          i = 0;
          ForEachMailNode(mlist, mnode)
          {
            struct Mail *mail = mnode->mail;

            if(mail != NULL)
              rd->res.num[i] = &mail->position;
            else
              rd->res.num[i] = 0;

            i++;
          }
        }

        DeleteMailList(mlist);
      }
      else
      {
        rd->res.num    = calloc(1, sizeof(long));
        rd->res.num[0] = 0;
      }
    }
    break;

    case RXIF_FREE:
    {
      if(rd->res.num)
        free(rd->res.num);

      FreeVecPooled(G->SharedMemPool, rd);
    }
    break;
  }

  LEAVE();
}
