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
#include <mui/NListtree_mcc.h>
#include <mui/NList_mcc.h>
#include <mui/NListview_mcc.h>
#include <mui/TextEditor_mcc.h>
#include <mui/BetterString_mcc.h>
#include <mui/Toolbar_mcc.h>
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
#include <proto/locale.h>
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
#define KPrintF dprintf

/*MorphOS standard netincludes don't have these*/

struct in_addr {
  u_long s_addr;
};

struct sockaddr_in {
  u_char sin_len;
  u_char sin_family;
  u_short sin_port;
  struct in_addr sin_addr;
  char sin_zero[8];
};

 struct hostent {
  char *h_name;
  char **h_aliases;
  int h_addrtype;
  int h_length;
  char **h_addr_list;
  #define h_addr h_addr_list[0]

  #define Shutdown shutdown
  #define GetHostByName gethostbyname
  #define Connect connect
  #define Recv recv
  #define Send send
  #define Socket socket

  #define SMTP_NO_SOCKET -1

  #define SOCK_STREAM 1
  #define AF_INET 2
  #define EINPROGRESS 36

};

#else
#include <clib/locale_protos.h>
#endif

#if (defined DEBUG) || (defined _MGST)
	#include "clib/debug_protos.h"
#endif

#if (defined DEBUG)
	#define DB(x) (x)
	#define DBpr(x) (KPrintF("YAM: %s",x))
#else
	#define DB(x)
	#define DBpr(x)
#endif
