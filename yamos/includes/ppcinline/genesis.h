#ifndef _PPCINLINE_GENESIS_H
#define _PPCINLINE_GENESIS_H

#ifndef __PPCINLINE_MACROS_H
#include <ppcinline/macros.h>
#endif

#ifndef GENESIS_BASE_NAME
#define GENESIS_BASE_NAME GenesisBase
#endif

#define GetFileSize(last) \
	LP1(0x1e, LONG, GetFileSize, STRPTR, last, a0, \
	, GENESIS_BASE_NAME, IF_CACHEFLUSHALL, NULL, 0, IF_CACHEFLUSHALL, NULL, 0)

#define ParseConfig(par1, last) \
	LP2(0x24, BOOL, ParseConfig, STRPTR, par1, a0, struct ParseConfig_Data *, last, a1, \
	, GENESIS_BASE_NAME, IF_CACHEFLUSHALL, NULL, 0, IF_CACHEFLUSHALL, NULL, 0)

#define ParseNext(last) \
	LP1(0x2a, BOOL, ParseNext, struct ParseConfig_Data *, last, a0, \
	, GENESIS_BASE_NAME, IF_CACHEFLUSHALL, NULL, 0, IF_CACHEFLUSHALL, NULL, 0)

#define ParseNextLine(last) \
	LP1(0x30, BOOL, ParseNextLine, struct ParseConfig_Data *, last, a0, \
	, GENESIS_BASE_NAME, IF_CACHEFLUSHALL, NULL, 0, IF_CACHEFLUSHALL, NULL, 0)

#define ParseEnd(last) \
	LP1NR(0x36, ParseEnd, struct ParseConfig_Data *, last, a0, \
	, GENESIS_BASE_NAME, IF_CACHEFLUSHALL, NULL, 0, IF_CACHEFLUSHALL, NULL, 0)

#define ReallocCopy(par1, last) \
	LP2(0x3c, STRPTR, ReallocCopy, STRPTR *, par1, a0, STRPTR, last, a1, \
	, GENESIS_BASE_NAME, IF_CACHEFLUSHALL, NULL, 0, IF_CACHEFLUSHALL, NULL, 0)

#define FreeUser(last) \
	LP1NR(0x42, FreeUser, struct genUser *, last, a0, \
	, GENESIS_BASE_NAME, IF_CACHEFLUSHALL, NULL, 0, IF_CACHEFLUSHALL, NULL, 0)

#define GetUserName(par1, par2, last) \
	LP3(0x48, BOOL, GetUserName, LONG, par1, d0, char *, par2, a0, LONG, last, d1, \
	, GENESIS_BASE_NAME, IF_CACHEFLUSHALL, NULL, 0, IF_CACHEFLUSHALL, NULL, 0)

#define GetUser(par1, par2, par3, last) \
	LP4(0x4e, struct genUser *, GetUser, STRPTR, par1, a0, STRPTR, par2, a1, STRPTR, par3, a2, LONG, last, d0, \
	, GENESIS_BASE_NAME, IF_CACHEFLUSHALL, NULL, 0, IF_CACHEFLUSHALL, NULL, 0)

#define GetGlobalUser() \
	LP0(0x54, struct genUser *, GetGlobalUser, \
	, GENESIS_BASE_NAME, IF_CACHEFLUSHALL, NULL, 0, IF_CACHEFLUSHALL, NULL, 0)

#define SetGlobalUser(last) \
	LP1NR(0x5a, SetGlobalUser, struct genUser *, last, a0, \
	, GENESIS_BASE_NAME, IF_CACHEFLUSHALL, NULL, 0, IF_CACHEFLUSHALL, NULL, 0)

#define ClearUserList() \
	LP0NR(0x60, ClearUserList, \
	, GENESIS_BASE_NAME, IF_CACHEFLUSHALL, NULL, 0, IF_CACHEFLUSHALL, NULL, 0)

#define ReloadUserList() \
	LP0(0x66, BOOL, ReloadUserList, \
	, GENESIS_BASE_NAME, IF_CACHEFLUSHALL, NULL, 0, IF_CACHEFLUSHALL, NULL, 0)

#define ReadFile(par1, par2, last) \
	LP3(0x6c, LONG, ReadFile, STRPTR, par1, a0, STRPTR, par2, a1, LONG, last, d0, \
	, GENESIS_BASE_NAME, IF_CACHEFLUSHALL, NULL, 0, IF_CACHEFLUSHALL, NULL, 0)

#define WriteFile(par1, par2, last) \
	LP3(0x72, BOOL, WriteFile, STRPTR, par1, a0, STRPTR, par2, a1, LONG, last, d0, \
	, GENESIS_BASE_NAME, IF_CACHEFLUSHALL, NULL, 0, IF_CACHEFLUSHALL, NULL, 0)

#define IsOnline(last) \
	LP1(0x78, BOOL, IsOnline, LONG, last, d0, \
	, GENESIS_BASE_NAME, IF_CACHEFLUSHALL, NULL, 0, IF_CACHEFLUSHALL, NULL, 0)

#endif /*  _PPCINLINE_GENESIS_H  */
