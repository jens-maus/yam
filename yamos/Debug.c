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

#ifdef DEBUG

#include <stdlib.h>
#include <string.h>
#include <proto/intuition.h>
#include <proto/utility.h>

#include "Debug.h"

ULONG DebugFlags = DBF_ERROR | DBF_ALWAYS;

VOID SetupDebug (VOID)
{
	STRPTR debug;
	if((debug = getenv("YAMDebug")))
	{
		static struct { char *token; int flag; } tokens[] =
		{
			{ "io",        DBF_IO         },
			{ "verbose",   DBF_VERBOSE    },
			{ "folders",   DBF_FOLDERS    },
			{ "all",       0xffffffff     },
			{ NULL,        0              }
		};

		STRPTR tok;
		while((tok = strtok(debug, ", ;")))
		{
			ULONG i;
			for(i = 0; tokens[i].token; i++)
			{
				if(!Stricmp(tok, tokens[i].token))
					DebugFlags |= tokens[i].flag;
			}
			debug = NULL;
		}
	}

	D(DBF_VERBOSE, ("Debug flags %04x", DebugFlags))
}

BOOL TestDebugFlag (ULONG flag)
{
	if(flag & DBF_ERROR)
	   DisplayBeep(NULL);

	return (BOOL)((DebugFlags & flag) ? TRUE : FALSE);
}

#endif
