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

#include <proto/exec.h>

#include "extrasrc.h"

#include "YAM.h"

#include "Rexx.h"

#include "Debug.h"

struct args
{
  char *file;
};

void rx_help(UNUSED struct RexxHost *host, struct RexxParams *params, enum RexxAction action, UNUSED struct RexxMsg *rexxmsg)
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
      struct rxs_command *rxc;
      FILE *fp = NULL;
      FILE *out = stdout;

      if(args->file != NULL && (fp = fopen(args->file, "w")) != NULL)
        out = fp;

      fprintf(out, "Commands for application \"YAM\"\n\nCommand              Template\n-------              --------\n");

      for(rxc = rxs_commandlist; rxc->command != NULL; rxc++)
      {
        fprintf(out, "%-20s%s%s%s%s%s%s\n", rxc->command,
                                            (rxc->results || rxc->args) ? " " : "", rxc->results ? "VAR/K,STEM/K" : "",
                                            (rxc->results && rxc->args) ? "," : "", rxc->args ? rxc->args : "",
                                            rxc->results ? " => " : "", rxc->results ? rxc->results : "");
      }

      if(fp != NULL)
        fclose(fp);
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
