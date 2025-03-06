/***************************************************************************

 YAM - Yet Another Mailer
 Copyright (C) 1995-2000 Marcel Beck
 Copyright (C) 2000-2025 YAM Open Source Team

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
 YAM OpenSource project    :  https://github.com/jens-maus/yam/

***************************************************************************/

#include <clib/alib_protos.h>
#include <proto/exec.h>

#include "extrasrc.h"

#include "YAM.h"
#include "YAM_read.h"

#include "mui/ClassesExtra.h"
#include "mui/ReadMailGroup.h"

#include "Config.h"
#include "MimeTypes.h"
#include "Rexx.h"

#include "Debug.h"

struct args
{
  long *part;
  char *filename;
  long overwrite;
};

void rx_readsave(UNUSED struct RexxHost *host, struct RexxParams *params, enum RexxAction action, UNUSED struct RexxMsg *rexxmsg)
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
      BOOL success = FALSE;
      struct ReadMailData *rmData = G->ActiveRexxRMData;

      if(rmData)
      {
        struct TempFile *tf;

        if(args->part)
        {
          struct Part *part;

          for(part = rmData->firstPart->Next; part; part = part->Next)
          {
            if(part->Nr == *(args->part) &&
               RE_DecodePart(part))
            {
              char file[SIZE_PATHFILE];

              AddPath(file, C->DetachDir, part->Name, sizeof(file));

              success = RE_Export(rmData,
                                  part->Filename,
                                  args->filename ? args->filename : "",
                                  part->Name,
                                  part->Nr,
                                  TRUE,
                                  args->overwrite != 0,
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
                              args->filename ? args->filename : "",
                              "",
                              0,
                              TRUE,
                              args->overwrite != 0,
                              IntMimeTypeArray[MT_TX_PLAIN].ContentType);

          CloseTempFile(tf);
        }
      }

      if(!success)
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
