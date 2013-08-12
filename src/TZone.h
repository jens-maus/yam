#ifndef TZONE_H
#define TZONE_H 1

/***************************************************************************

 YAM - Yet Another Mailer
 Copyright (C) 1995-2000 Marcel Beck
 Copyright (C) 2000-2013 YAM Open Source Team

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

#include <exec/lists.h>
#include <exec/nodes.h>

#include "timeval.h"

struct TZoneLocation
{
  struct MinNode node;
  char *name;
  char *comment;
};

struct TZoneContinent
{
  struct MinNode node;
  char *name;
  size_t numLocations;
  struct MinList locationList;
};

BOOL ParseZoneTabFile(void);
char **BuildContinentEntries(void);
char **BuildLocationEntries(ULONG continent);
char *BuildTZoneName(char *name, size_t nameSize, ULONG continent, ULONG location);
BOOL ParseTZoneName(const char *tzone, ULONG *continent, ULONG *location, char **comment);
const char *GuessTZone(const int gmtOffset);
struct TZoneContinent *FindContinent(const char *continent);
struct TZoneLocation *FindLocation(struct TZoneContinent *continent, const char *location);
time_t FindNextDSTSwitch(const char *tzone, struct TimeVal *tv);
void SetTZone(const char *location);
void TZSet(const char *location);
void TZoneCleanup(void);

#endif /* TZONE_H */
