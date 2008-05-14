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

#include "Requesters.h"
#include "Rexx.h"

#include "Debug.h"

struct rxd_request
{
  long rc, rc2;
  struct {
    char *var, *stem;
    char *body;
    char *gadgets;
  } arg;
  struct {
    long *result;
  } res;
};

void rx_request(UNUSED struct RexxHost *host, void **rxd, enum RexxAction action, UNUSED struct RexxMsg *rexxmsg)
{
  struct
  {
     struct rxd_request rd;
     long result;
  } *rd = *rxd;

  ENTER();

  switch(action)
  {
    case RXIF_INIT:
    {
      if((*rxd = AllocVecPooled(G->SharedMemPool, sizeof(*rd))) != NULL)
        ((struct rxd_request *)(*rxd))->rc = offsetof(struct rxd_request, res) / sizeof(long);
    }
    break;

    case RXIF_ACTION:
    {
      char *reqtext = AllocReqText(rd->rd.arg.body);

      rd->result = MUI_Request(G->App, NULL, 0, NULL, rd->rd.arg.gadgets, reqtext);
      rd->rd.res.result = &rd->result;

      free(reqtext);
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
