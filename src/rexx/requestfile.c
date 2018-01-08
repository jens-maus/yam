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

#include <stdlib.h>
#include <string.h>

#include <proto/exec.h>
#include <proto/dos.h>

#include "extrasrc.h"

#include "YAM.h"

#include "Locale.h"
#include "Rexx.h"

#include "Debug.h"

struct args
{
  struct RexxResult varStem;
  char *title;
  char *drawer;
  char *file;
  long *multiSelect;
  long *drawersOnly;
  long *saveMode;
  long *noicons;
};

struct results
{
  char *drawer;
  char **files;
};

void rx_requestfile(UNUSED struct RexxHost *host, struct RexxParams *params, enum RexxAction action, UNUSED struct RexxMsg *rexxmsg)
{
  struct args *args = params->args;
  struct results *results = params->results;

  ENTER();

  switch( action )
  {
    case RXIF_INIT:
    {
      params->args = AllocVecPooled(G->SharedMemPool, sizeof(*args));
      params->results = AllocVecPooled(G->SharedMemPool, sizeof(*results));
    }
    break;

    case RXIF_ACTION:
    {
      int mode;
      struct FileReqCache *frc;

      mode = REQF_NONE;
      if(args->multiSelect != NULL)
        setFlag(mode, REQF_MULTISELECT);
      if(args->drawersOnly != NULL)
        setFlag(mode, REQF_DRAWERSONLY);
      if(args->saveMode != NULL)
        setFlag(mode, REQF_SAVEMODE);
      if(args->noicons != NULL)
        setFlag(mode, REQF_NOICONS);

      if((frc = ReqFile(ASL_GENERIC, NULL, args->title, mode, args->drawer, args->file)) != NULL)
      {
        // frc->numArgs will be != 0 only if multiselection is enabled,
        // hence we limit the number to be at least 1
        int numArgs = MAX(1, frc->numArgs);

        if((results->files = calloc(numArgs+1, sizeof(char *))) != NULL)
        {
          int i;

          results->drawer = frc->drawer;

          // copy over the file names
          if(frc->numArgs == 0)
            results->files[0] = frc->file;
          else
          {
            for(i=0; i < frc->numArgs; i++)
              results->files[i] = frc->argList[i];
          }
        }
        else
          params->rc = RETURN_ERROR;
      }
      else
        params->rc = RETURN_ERROR;
    }
    break;

    case RXIF_FREE:
    {
      if(args != NULL)
        FreeVecPooled(G->SharedMemPool, args);

      if(results != NULL)
      {
        free(results->files);
        FreeVecPooled(G->SharedMemPool, results);
      }
    }
    break;
  }

  LEAVE();
}
