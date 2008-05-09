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

#include <libraries/mui.h>
#include <proto/exec.h>

#include "extrasrc.h"

#include "YAM.h"
#include "YAM_main.h"

#include "Rexx.h"

#include "Debug.h"

struct rxd_info
{
  long rc, rc2;
  struct
  {
    char *var, *stem;
    char *item;
  } arg;
  struct
  {
    const char *value;
  } res;
};

void rx_info(UNUSED struct RexxHost *host, void **rxd, enum RexxAction action, UNUSED struct RexxMsg *rexxmsg)
{
  struct rxd_info *rd = *rxd;

  ENTER();

  switch(action)
  {
    case RXIF_INIT:
    {
      if((*rxd = AllocVecPooled(G->SharedMemPool, sizeof(*rd))) != NULL)
        ((struct rxd_info *)(*rxd))->rc = offsetof(struct rxd_info, res) / sizeof(long);
    }
    break;

    case RXIF_ACTION:
    {
      char *key = rd->arg.item;

      if(!key)  rd->rc = RETURN_ERROR;
      else if(!stricmp(key, "title"))       rd->res.value = (char *)xget(G->App, MUIA_Application_Title);
      else if(!stricmp(key, "author"))      rd->res.value = (char *)xget(G->App, MUIA_Application_Author);
      else if(!stricmp(key, "copyright"))   rd->res.value = (char *)xget(G->App, MUIA_Application_Copyright);
      else if(!stricmp(key, "description")) rd->res.value = (char *)xget(G->App, MUIA_Application_Description);
      else if(!stricmp(key, "version"))     rd->res.value = (char *)xget(G->App, MUIA_Application_Version);
      else if(!stricmp(key, "base"))        rd->res.value = (char *)xget(G->App, MUIA_Application_Base);
      else if(!stricmp(key, "screen"))
      {
        struct Screen *screen;
        rd->res.value = "Workbench";
        screen = (struct Screen *)xget(G->MA->GUI.WI, MUIA_Window_Screen);

        if(screen)
        {
          struct Node *pubs;
          struct List *pubscreens = LockPubScreenList();

          for(pubs = pubscreens->lh_Head; pubs->ln_Succ; pubs = pubs->ln_Succ)
          {
            if(((struct PubScreenNode *)pubs)->psn_Screen == screen)
            {
              rd->res.value = pubs->ln_Name;
              break;
            }
          }

          UnlockPubScreenList();
        }
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
