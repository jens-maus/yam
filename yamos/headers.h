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
#ifdef __MORPHOS__
#include <unistd.h>
#else
#include <error.h>
#endif
#include <dos.h>
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
#include <ppcinline/locale.h>
#include <ppcinline/socket.h>
#else
#include <clib/muimaster_protos.h>
#include <clib/dos_protos.h>
#include <clib/exec_protos.h>
#include <clib/utility_protos.h>
#include <clib/locale_protos.h>
#include <clib/icon_protos.h>
#include <clib/graphics_protos.h>
#include <clib/datatypes_protos.h>
#include <clib/wb_protos.h>
#include <clib/macros.h>
#include <clib/wb_protos.h>
#include <clib/iffparse_protos.h>
#include <clib/keymap_protos.h>
#include <clib/rexxsyslib_protos.h>
#include <clib/xpkmaster_protos.h>
#include <clib/openurl_protos.h>
#include <clib/miami_protos.h>
#include <clib/genesis_protos.h>
#include <clib/cmanager_protos.h>
#include <pragmas/muimaster_pragmas.h>
#include <pragmas/openurl_pragmas.h>
#include <pragmas/xpkmaster_pragmas.h>
#include <pragmas/miami_pragmas.h>
#include <pragmas/genesis_pragmas.h>
#include <pragmas/cmanager_pragmas.h>
#endif
#include <NewReadArgs.h>

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

