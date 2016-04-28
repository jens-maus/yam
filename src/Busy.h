#ifndef BUSY_H
#define BUSY_H 1

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

#include <exec/nodes.h>

struct BusyNode
{
  struct MinNode node;         // to be linked in an Exec MinList
  ULONG type;                  // busy type, see below
  ULONG progressCurrent;       // current progress (progress bars only)
  ULONG progressMax;           // maximum progress (progress bars only)
  char infoText[SIZE_DEFAULT]; // the information text to be shown
  BOOL wasVisible;             // was this action visible before already?
};

#define BUSY_TEXT             1 // a simple information text only, normal usage
#define BUSY_PROGRESS         2 // a busy bar with progress gauge, normal usage
#define BUSY_PROGRESS_ABORT   3 // a busy bar with progress gauge and an abort button, normal usage
#define BUSY_AREXX            4 // a simple information text only, ARexx usage

struct BusyNode *BusyBegin(ULONG type);
void BusyText(struct BusyNode *busy, const char *text, const char *param);
BOOL BusyProgress(struct BusyNode *busy, int progress, int max);
void BusyEnd(struct BusyNode *busy);
void BusyCleanup(void);

#endif /* BUSY_H */
