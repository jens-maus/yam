#ifndef UPDATECHECK_H
#define UPDATECHECK_H

/***************************************************************************

 YAM - Yet Another Mailer
 Copyright (C) 1995-2000 Marcel Beck
 Copyright (C) 2000-2022 YAM Open Source Team

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

#include "timeval.h"

// forward declarations
struct TempFile;

// enumeration values for the update check
// status
enum UpdateCheckStatus
{
  UST_NOCHECK=0,
  UST_NOUPDATE,
  UST_NOQUERY,
  UST_UPDATESUCCESS
};

// when an update for a specific component is found
// a structure is filled and sent to the update notification window
struct UpdateComponent
{
  char name[SIZE_DEFAULT];        // name of the component
  char recent[SIZE_DEFAULT];      // recent version string (available)
  char installed[SIZE_DEFAULT];   // currently installed version
  char url[SIZE_URL];             // the URL where the component is available
  struct TempFile *changeLogFile; // the file with the text about the changes.
};

struct UpdateState
{
  struct TimeVal LastUpdateCheck;
  enum UpdateCheckStatus LastUpdateStatus;
};

// externally accessible functions
void InitUpdateCheck(const BOOL initial);
void CheckForUpdates(const BOOL quiet);
char *BuildUpdateRequest(void);
BOOL ParseUpdateFile(const char *filename, const BOOL quiet);
BOOL ExtractUpdateFilename(const char *url, char *file, size_t fileSize);
void LoadUpdateState(void);
void SaveUpdateState(void);
void GetLastUpdateState(struct UpdateState *state);
void SetDefaultUpdateState(void);

#endif // UPDATECHECK_H


