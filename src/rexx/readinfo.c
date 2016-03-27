/***************************************************************************

 YAM - Yet Another Mailer
 Copyright (C) 1995-2000 Marcel Beck
 Copyright (C) 2000-2016 YAM Open Source Team

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

#include <proto/exec.h>

#include "extrasrc.h"

#include "YAM.h"
#include "YAM_read.h"

#include "Rexx.h"

#include "Debug.h"

struct args
{
  struct RexxResult varStem;
};

struct results
{
  char **filename;
  char **filetype;
  long **filesize;
  char **tempfile;
};

void rx_readinfo(UNUSED struct RexxHost *host, struct RexxParams *params, enum RexxAction action, UNUSED struct RexxMsg *rexxmsg)
{
  struct args *args = params->args;
  struct results *results = params->results;

  ENTER();

  switch(action)
  {
    case RXIF_INIT:
    {
      params->args = AllocVecPooled(G->SharedMemPool, sizeof(*args));
      params->results = AllocVecPooled(G->SharedMemPool, sizeof(*results));
    }
    break;

    case RXIF_ACTION:
    {
      struct ReadMailData *rmData = G->ActiveRexxRMData;

      if(rmData)
      {
        struct Part *part;
        int i, parts;

        for(parts = 0, part = rmData->firstPart->Next; part; parts++, part = part->Next);

        results->filename = calloc(parts+1, sizeof(char *));
        results->filetype = calloc(parts+1, sizeof(char *));
        results->filesize = calloc(parts+1, sizeof(long));
        results->tempfile = calloc(parts+1, sizeof(char *));

        for(i = 0, part = rmData->firstPart->Next; part; i++, part = part->Next)
        {
          results->filename[i] = part->Name;
          results->filetype[i] = part->ContentType;
          results->filesize[i] = (long *)&part->Size;
          results->tempfile[i] = part->Filename;
        }
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
        free(results->filename);
        free(results->filetype);
        free(results->filesize);
        free(results->tempfile);
        FreeVecPooled(G->SharedMemPool, results);
      }
    }
    break;
  }

  LEAVE();
}
