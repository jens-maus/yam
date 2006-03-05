#ifndef _VBCCINLINE_GENESIS_H
#define _VBCCINLINE_GENESIS_H

#ifndef EXEC_TYPES_H
#include <exec/types.h>
#endif

LONG __GetFileSize(__reg("a0") STRPTR file, __reg("a6") APTR)="\tjsr\t-30(a6)";
#define GetFileSize(file) __GetFileSize((file), GenesisBase)

BOOL __ParseConfig(__reg("a0") STRPTR file, __reg("a1") struct ParseConfig_Data * pc_data, __reg("a6") APTR)="\tjsr\t-36(a6)";
#define ParseConfig(file, pc_data) __ParseConfig((file), (pc_data), GenesisBase)

BOOL __ParseNext(__reg("a0") struct ParseConfig_Data * pc_data, __reg("a6") APTR)="\tjsr\t-42(a6)";
#define ParseNext(pc_data) __ParseNext((pc_data), GenesisBase)

BOOL __ParseNextLine(__reg("a0") struct ParseConfig_Data * pc_data, __reg("a6") APTR)="\tjsr\t-48(a6)";
#define ParseNextLine(pc_data) __ParseNextLine((pc_data), GenesisBase)

VOID __ParseEnd(__reg("a0") struct ParseConfig_Data * pc_data, __reg("a6") APTR)="\tjsr\t-54(a6)";
#define ParseEnd(pc_data) __ParseEnd((pc_data), GenesisBase)

STRPTR __ReallocCopy(__reg("a0") STRPTR * old_ptr, __reg("a1") STRPTR src, __reg("a6") APTR)="\tjsr\t-60(a6)";
#define ReallocCopy(old_ptr, src) __ReallocCopy((old_ptr), (src), GenesisBase)

VOID __FreeUser(__reg("a0") struct genUser * user, __reg("a6") APTR)="\tjsr\t-66(a6)";
#define FreeUser(user) __FreeUser((user), GenesisBase)

BOOL __GetUserName(__reg("d0") LONG user_number, __reg("a0") char * buffer, __reg("d1") LONG len, __reg("a6") APTR)="\tjsr\t-72(a6)";
#define GetUserName(user_number, buffer, len) __GetUserName((user_number), (buffer), (len), GenesisBase)

struct genUser * __GetUser(__reg("a0") STRPTR name, __reg("a1") STRPTR password, __reg("a2") STRPTR title, __reg("d0") LONG flags, __reg("a6") APTR)="\tjsr\t-78(a6)";
#define GetUser(name, password, title, flags) __GetUser((name), (password), (title), (flags), GenesisBase)

struct genUser * __GetGlobalUser(__reg("a6") APTR)="\tjsr\t-84(a6)";
#define GetGlobalUser() __GetGlobalUser(GenesisBase)

VOID __SetGlobalUser(__reg("a0") struct genUser * user, __reg("a6") APTR)="\tjsr\t-90(a6)";
#define SetGlobalUser(user) __SetGlobalUser((user), GenesisBase)

VOID __ClearUserList(__reg("a6") APTR)="\tjsr\t-96(a6)";
#define ClearUserList() __ClearUserList(GenesisBase)

BOOL __ReloadUserList(__reg("a6") APTR)="\tjsr\t-102(a6)";
#define ReloadUserList() __ReloadUserList(GenesisBase)

LONG __ReadFile(__reg("a0") STRPTR file, __reg("a1") STRPTR buffer, __reg("d0") LONG len, __reg("a6") APTR)="\tjsr\t-108(a6)";
#define ReadFile(file, buffer, len) __ReadFile((file), (buffer), (len), GenesisBase)

BOOL __WriteFile(__reg("a0") STRPTR file, __reg("a1") STRPTR buffer, __reg("d0") LONG len, __reg("a6") APTR)="\tjsr\t-114(a6)";
#define WriteFile(file, buffer, len) __WriteFile((file), (buffer), (len), GenesisBase)

BOOL __IsOnline(__reg("d0") LONG flags, __reg("a6") APTR)="\tjsr\t-120(a6)";
#define IsOnline(flags) __IsOnline((flags), GenesisBase)

#endif /*  _VBCCINLINE_GENESIS_H  */
