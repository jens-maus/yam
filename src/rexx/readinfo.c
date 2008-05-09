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
#include "YAM_read.h"

#include "Rexx.h"

#include "Debug.h"

struct rxd_readinfo
{
  long rc, rc2;
  struct {
    char *var, *stem;
  } arg;
  struct {
    char **filename;
    char **filetype;
    long **filesize;
    char **tempfile;
  } res;
};

void rx_readinfo(UNUSED struct RexxHost *host, void **rxd, enum RexxAction action, UNUSED struct RexxMsg *rexxmsg)
{
  struct rxd_readinfo *rd = *rxd;

  ENTER();

  switch(action)
  {
    case RXIF_INIT:
    {
      if((*rxd = AllocVecPooled(G->SharedMemPool, sizeof(*rd))) != NULL)
        ((struct rxd_readinfo *)(*rxd))->rc = offsetof(struct rxd_readinfo, res) / sizeof(long);
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

        rd->res.filename = calloc(parts+1, sizeof(char *));
        rd->res.filetype = calloc(parts+1, sizeof(char *));
        rd->res.filesize = calloc(parts+1, sizeof(long));
        rd->res.tempfile = calloc(parts+1, sizeof(char *));

        for(i = 0, part = rmData->firstPart->Next; part; i++, part = part->Next)
        {
          rd->res.filename[i] = part->Name;
          rd->res.filetype[i] = part->ContentType;
          rd->res.filesize[i] = (long *)&part->Size;
          rd->res.tempfile[i] = part->Filename;
        }
      }
      else
        rd->rc = RETURN_ERROR;
    }
    break;

    case RXIF_FREE:
    {
      if(rd->res.filename) free(rd->res.filename);
      if(rd->res.filetype) free(rd->res.filetype);
      if(rd->res.filesize) free(rd->res.filesize);
      if(rd->res.tempfile) free(rd->res.tempfile);

      FreeVecPooled(G->SharedMemPool, rd);
    }
    break;
  }

  LEAVE();
}
