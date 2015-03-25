/***************************************************************************

 YAM - Yet Another Mailer
 Copyright (C) 1995-2000 Marcel Beck
 Copyright (C) 2000-2015 YAM Open Source Team

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
#include <proto/utility.h>
#include <utility/date.h>

#include "extrasrc.h"

#include "YAM.h"
#include "YAM_global.h"
#include "YAM_utilities.h"

#include "Config.h"
#include "Logfile.h"
#include "MethodStack.h"
#include "Threads.h"

#include "mui/YAMApplication.h"

#include "Debug.h"

/// AppendToLogfile
//  Appends a line to the logfile
void AppendToLogfile(const enum LFMode mode, const int id, const char *text, ...)
{
  ENTER();

  // check the Logfile mode
  if(C->LogfileMode != LF_NONE &&
     (mode == LF_ALL || C->LogfileMode == mode))
  {
    // check if the event in question should really be logged or
    // not.
    if(C->LogAllEvents == TRUE || (id >= 30 && id <= 49))
    {
      if(IsMainThread() == TRUE)
      {
        FILE *fh;
        char logfile[SIZE_PATHFILE];
        char filename[SIZE_FILE];

        // if the user wants to split the logfile by date
        // we go and generate the filename now.
        if(C->SplitLogfile == TRUE)
        {
          struct ClockData cd;

          Amiga2Date(GetDateStamp(), &cd);
          snprintf(filename, sizeof(filename), "YAM-%s%d.log", months[cd.month-1], cd.year);
        }
        else
          strlcpy(filename, "YAM.log", sizeof(filename));

        // add the logfile path to the filename.
        AddPath(logfile, C->LogfilePath[0] != '\0' ? C->LogfilePath : G->ProgDir, filename, sizeof(logfile));

        D(DBF_ALWAYS, "logging id %ld, text '%s' to file '%s'", id, text, logfile);

        // open the file handle in 'append' mode and output the
        // text accordingly.
        if((fh = fopen(logfile, "a")) != NULL)
        {
          char datstr[64];
          va_list args;

          DateStamp2String(datstr, sizeof(datstr), NULL, DSS_DATETIME, TZC_NONE);

          // output the header
          fprintf(fh, "%s [%02d] ", datstr, id);

          // compose the varags values
          va_start(args, text);
          vfprintf(fh, text, args);
          va_end(args);

          fprintf(fh, "\n");
          fclose(fh);
        }
      }
      else
      {
        // subthreads just build the message string and
        // let the application do the dirty work
        va_list args;
        char *logMessage;

        // compose the varags values
        va_start(args, text);
        if(vasprintf(&logMessage, text, args) != -1)
          PushMethodOnStack(G->App, 4, MUIM_YAMApplication_AppendToLogfile, mode, id, logMessage);
        va_end(args);
      }
    }
  }

  LEAVE();
}

///
