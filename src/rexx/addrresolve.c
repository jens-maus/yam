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
#include <proto/muimaster.h>

#include "extrasrc.h"

#include "YAM.h"
#include "YAM_addressbook.h"
#include "YAM_addressbookEntry.h"
#include "classes/Classes.h"

#include "Rexx.h"

#include "Debug.h"

void rx_addrresolve(UNUSED struct RexxHost *host, void **rxd, enum RexxAction action, UNUSED struct RexxMsg *rexxmsg)
{
  struct
  {
    struct rxd_addrresolve rd;
    char *string;
  } *rd = (void *)*rxd;

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
      // generate a "fake" RecipientstringObject and use it on the resolve task
      Object *str = RecipientstringObject,
                      MUIA_Recipientstring_MultipleRecipients, TRUE,
                      MUIA_String_Contents,                    rd->rd.arg.alias,
                    End;

      STRPTR res = (STRPTR)DoMethod(str, MUIM_Recipientstring_Resolve, MUIF_Recipientstring_Resolve_NoCache);
      if(res && strcmp(rd->rd.arg.alias, res)) /* did the string change ? */
      {
        if((rd->rd.res.recpt = rd->string = AllocStrBuf(strlen(res)+1)))
          strlcpy(rd->string, res, strlen(res)+1);
      }
      else
        rd->rd.rc = RETURN_WARN;

      MUI_DisposeObject(str);
    }
    break;

    case RXIF_FREE:
    {
      FreeStrBuf(rd->string);

      FreeVecPooled(G->SharedMemPool, rd);
    }
    break;
  }

  LEAVE();
}
