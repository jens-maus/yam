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

#include "extrasrc.h"

#include "YAM.h"
#include "YAM_config.h"
#include "YAM_read.h"
#include "classes/Classes.h"

#include "Mime.h"
#include "Rexx.h"

#include "Debug.h"

void rx_readsave(UNUSED struct RexxHost *host, void **rxd, enum RexxAction action, UNUSED struct RexxMsg *rexxmsg)
{
  struct rxd_readsave *rd = *rxd;

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
      BOOL success = FALSE;
      struct ReadMailData *rmData = G->ActiveRexxRMData;

      if(rmData)
      {
        struct TempFile *tf;

        if(rd->arg.part)
        {
          struct Part *part;

          for(part = rmData->firstPart->Next; part; part = part->Next)
          {
            if(part->Nr == *(rd->arg.part) &&
               RE_DecodePart(part))
            {
              char file[SIZE_PATHFILE];

              AddPath(file, C->DetachDir, part->Name, sizeof(file));

              success = RE_Export(rmData,
                                  part->Filename,
                                  rd->arg.filename ? rd->arg.filename : "",
                                  part->Name,
                                  part->Nr,
                                  TRUE,
                                  (BOOL)rd->arg.overwrite,
                                  part->ContentType);
            }
          }
        }
        else if((tf = OpenTempFile("w")))
        {
          DoMethod(rmData->readMailGroup, MUIM_ReadMailGroup_SaveDisplay, tf->FP);

          fclose(tf->FP);
          tf->FP = NULL;

          success = RE_Export(rmData,
                              tf->Filename,
                              rd->arg.filename ? rd->arg.filename : "",
                              "",
                              0,
                              TRUE,
                              (BOOL)rd->arg.overwrite,
                              IntMimeTypeArray[MT_TX_PLAIN].ContentType);

          CloseTempFile(tf);
        }
      }

      if(!success)
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
