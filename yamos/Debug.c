#ifdef DEBUG

#include <stdlib.h>
#include <string.h>
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
	return (BOOL)(DebugFlags & flag) ? TRUE : FALSE;
}

#endif
