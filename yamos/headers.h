/***************************************************************************
 Headers
***************************************************************************/

//#include <mpatrol.h>
#include <ctype.h>
#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <time.h>
#include <math.h>

#include <exec/memory.h>
#include <exec/execbase.h>
#include <dos/datetime.h>
#include <dos/dostags.h>
#include <dos/doshunks.h>
#include <workbench/workbench.h>
#include <workbench/startup.h>
#include <devices/printer.h>
#include <intuition/icclass.h>
#include <intuition/gadgetclass.h>
#include <datatypes/pictureclass.h>
#include <datatypes/soundclass.h>
#include <libraries/locale.h>
#include <libraries/asl.h>
#include <libraries/mui.h>
#include <libraries/gadtools.h>
#include <libraries/openurl.h>
#include <libraries/genesis.h>
#include <libraries/cmanager.h>
#include <mui/listtree_mcc.h>
#include <mui/nlist_mcc.h>
#include <mui/nlistview_mcc.h>
#include <mui/texteditor_mcc.h>
#include <mui/betterstring_mcc.h>
#include <mui/toolbar_mcc.h>
#include <rexx/rxslib.h>
#include <rexx/storage.h>
#include <xpk/xpk.h>
#include <clib/alib_protos.h>
#ifdef __MORPHOS__
#define NO_PPCINLINE_STDARG
#include <ppcinline/locale.h>
#include <ppcinline/socket.h>
#else
#include <proto/socket.h>
#endif
#include <proto/muimaster.h>
#include <proto/dos.h>
#include <proto/exec.h>
#include <proto/utility.h>
#include <proto/icon.h>
#include <proto/intuition.h>
#include <proto/graphics.h>
#include <proto/datatypes.h>
#include <proto/wb.h>
#include <proto/iffparse.h>
#include <proto/keymap.h>
#include <proto/rexxsyslib.h>
#include <proto/xpkmaster.h>
#include <proto/openurl.h>
#include <proto/miami.h>
#include <proto/genesis.h>
#include <proto/cmanager.h>
#include <clib/locale_protos.h>
#include <clib/macros.h>
#include <NewReadArgs.h>
#include <compiler.h>
#include <extra.h>


#ifdef __MORPHOS__
#define CreateExtIO	CreateIORequest
#define DeleteExtIO	DeleteIORequest
#undef DoSuperMethod
#define DoSuperMethod(cl,obj,a,b,c) ({ LONG m[] = { (LONG)(a), (LONG)(b), (LONG)(c) }; DoSuperMethodA(cl,obj,(Msg)m); })
#define _OSERR 		IoErr()
#define CreatePort(a,b)	CreateMsgPort()
#define DeletePort(a)	DeleteMsgPort(a)
#define ABS(x) 		((x)<0?-(x):(x))
#define MAX(x,y)        ((x)<(y)?(y):(x))
#endif

#if (defined DEBUG) || (defined _MGST)
	#include "clib/debug_protos.h"
#endif

#if (defined DEBUG)
	#define DB(x) (x)
	#define DBpr(x) (KPrintf("YAM: %s",x))
#else
	#define DB(x)
	#define DBpr(x)
#endif
