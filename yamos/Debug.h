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

 YAM Official Support Site :  http://www.yam.ch/
 YAM OpenSource project    :  http://sourceforge.net/projects/yamos/

 $Id$

***************************************************************************/

#ifndef DEBUG_H
#define DEBUG_H

#define DBF_ERROR       0x00000001
#define DBF_ALWAYS      0x00000002
#define DBF_IO          0x00000004
#define DBF_VERBOSE     0x00000008
#define DBF_FOLDERS     0x00000010
#define DBF_ALL         0xffffffff

#ifdef DEBUG

#ifndef EXEC_TYPES_H
#include <exec/types.h>
#endif

VOID SetupDebug (VOID);
BOOL TestDebugFlag (ULONG flag);

void kprintf(const char *formatString,...);

#define D(flag, str)			\
	if(TestDebugFlag(flag))	\
	{								\
		kprintf str ;			\
		kprintf("\n");			\
	}

#endif

#endif
