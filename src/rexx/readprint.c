/***************************************************************************

 YAM - Yet Another Mailer
 Copyright (C) 1995-2000 Marcel Beck
 Copyright (C) 2000-2018 YAM Open Source Team

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
#include <proto/exec.h>

#include "extrasrc.h"

#include "YAM.h"
#include "YAM_read.h"

#include "mui/ClassesExtra.h"
#include "mui/ReadMailGroup.h"

#include "Rexx.h"

#include "Debug.h"

struct args
{
  long *part;
};

void rx_readprint(UNUSED struct RexxHost *host, struct RexxParams *params, enum RexxAction action, UNUSED struct RexxMsg *rexxmsg)
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
      struct ReadMailData *rmData = G->ActiveRexxRMData;
      BOOL success = FALSE;

      if(rmData)
      {
        FILE *prt;

        if(args->part)
        {
          struct Part *part;

          for(part = rmData->firstPart->Next; part; part = part->Next)
          {
            if(part->Nr == *(args->part) &&
               RE_DecodePart(part))
            {
              success = CopyFile("PRT:", 0, part->Filename, 0);
            }
          }
        }
        else if((prt = fopen("PRT:", "w")))
        {
          setvbuf(prt, NULL, _IOFBF, SIZE_FILEBUF);

          DoMethod(rmData->readMailGroup, MUIM_ReadMailGroup_SaveDisplay, prt);

          if(ferror(prt) == 0)
            success = TRUE;

          fclose(prt);
        }
      }

      if(success == FALSE)
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
