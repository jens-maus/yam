#ifndef DEBUG_H
#define DEBUG_H

#define DBF_ERROR       0x00000001
#define DBF_ALWAYS      0x00000002
#define DBF_IO          0x00000002
#define DBF_ALL         0xffffffff

#ifdef DEBUG

#ifndef EXEC_TYPES_H
#include <exec/types.h>
#endif

VOID SetupDebug (VOID);
BOOL TestDebugFlag (ULONG flag);
void kprintf(const char *formatString, ...);

#define D(flag, str)			\
	if(TestDebugFlag(flag))	\
	{								\
		kprintf str ;			\
		kprintf("\n");			\
	}

#else

#define InitDebug()
#define D(flag, str) ;

#endif

#endif
