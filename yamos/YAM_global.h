#ifndef YAM_GLOBAL_H
#define YAM_GLOBAL_H

/***************************************************************************

 YAM - Yet Another Mailer
 Copyright (C) 1995-2000 by Marcel Beck <mbeck@yam.ch>
 Copyright (C) 2000-2001 by YAM Open Source Team

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

#include <exec/types.h>

#include "YAM_stringsizes.h"

extern char *             ContType[MAXCTYPE+1];
extern APTR               ContTypeDesc[MAXCTYPE];
extern char *             FolderNames[4];
extern char *             months[12];
extern char *             SecCodes[5];
extern char *             SigNames[3];
extern char *             Status[9];
extern struct WBStartup * WBmsg;
extern char *             wdays[7];
extern char *             yamversion;
extern char *             yamversionstring;
extern char *             yamversiondate;
extern unsigned long      yamversiondays;

#endif /* YAM_GLOBAL_H */
