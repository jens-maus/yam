/***************************************************************************

 YAM - Yet Another Mailer
 Copyright (C) 1995-2000 Marcel Beck
 Copyright (C) 2000-2012 YAM Open Source Team

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

 YAM Official Support Site :  http://www.yam.ch/
 YAM OpenSource project    :  http://sourceforge.net/projects/yamos/

 $Id$

***************************************************************************/

#ifndef FILEINFO_H
#define FILEINFO_H 1

#include <exec/types.h>

// possible information available for query
enum FileInfo
{
  FI_SIZE = 0 ,
  FI_PROTECTION,
  FI_COMMENT,
  FI_DATE,
  FI_TIME,
  FI_TYPE,
};

// possible return values for FI_TYPE
enum FType
{
  FIT_NONEXIST = 0,
  FIT_UNKNOWN,
  FIT_FILE,
  FIT_DRAWER
};

BOOL ObtainFileInfo(const char *name, enum FileInfo which, void *valuePtr);
BOOL FileExists(const char *filename);

#endif /* FILEINFO_H */
