/* Automatically generated header! Do not edit! */

#ifndef _INLINE_GENESIS_H
#define _INLINE_GENESIS_H

#ifndef __INLINE_MACROS_H
#include <inline/macros.h>
#endif /* !__INLINE_MACROS_H */

#ifndef GENESIS_BASE_NAME
#define GENESIS_BASE_NAME GenesisBase
#endif /* !GENESIS_BASE_NAME */

#define ClearUserList() \
  LP0NR(0x60, ClearUserList, \
  , GENESIS_BASE_NAME)

#define FreeUser(user) \
  LP1NR(0x42, FreeUser, struct genUser *, user, a0, \
  , GENESIS_BASE_NAME)

#define GetFileSize(file) \
  LP1(0x1e, LONG, GetFileSize, STRPTR, file, a0, \
  , GENESIS_BASE_NAME)

#define GetGlobalUser() \
  LP0(0x54, struct genUser *, GetGlobalUser, \
  , GENESIS_BASE_NAME)

#define GetUser(name, password, title, flags) \
  LP4(0x4e, struct genUser *, GetUser, STRPTR, name, a0, STRPTR, password, a1, STRPTR, title, a2, LONG, flags, d0, \
  , GENESIS_BASE_NAME)

#define GetUserName(user_number, buffer, len) \
  LP3(0x48, BOOL, GetUserName, LONG, user_number, d0, char *, buffer, a0, LONG, len, d1, \
  , GENESIS_BASE_NAME)

#define IsOnline(flags) \
  LP1(0x78, BOOL, IsOnline, LONG, flags, d0, \
  , GENESIS_BASE_NAME)

#define ParseConfig(file, pc_data) \
  LP2(0x24, BOOL, ParseConfig, STRPTR, file, a0, struct ParseConfig_Data *, pc_data, a1, \
  , GENESIS_BASE_NAME)

#define ParseEnd(pc_data) \
  LP1NR(0x36, ParseEnd, struct ParseConfig_Data *, pc_data, a0, \
  , GENESIS_BASE_NAME)

#define ParseNext(pc_data) \
  LP1(0x2a, BOOL, ParseNext, struct ParseConfig_Data *, pc_data, a0, \
  , GENESIS_BASE_NAME)

#define ParseNextLine(pc_data) \
  LP1(0x30, BOOL, ParseNextLine, struct ParseConfig_Data *, pc_data, a0, \
  , GENESIS_BASE_NAME)

#define ReadFile(file, buffer, len) \
  LP3(0x6c, LONG, ReadFile, STRPTR, file, a0, STRPTR, buffer, a1, LONG, len, d0, \
  , GENESIS_BASE_NAME)

#define ReallocCopy(old_ptr, src) \
  LP2(0x3c, STRPTR, ReallocCopy, STRPTR *, old_ptr, a0, STRPTR, src, a1, \
  , GENESIS_BASE_NAME)

#define ReloadUserList() \
  LP0(0x66, BOOL, ReloadUserList, \
  , GENESIS_BASE_NAME)

#define SetGlobalUser(user) \
  LP1NR(0x5a, SetGlobalUser, struct genUser *, user, a0, \
  , GENESIS_BASE_NAME)

#define WriteFile(file, buffer, len) \
  LP3(0x72, BOOL, WriteFile, STRPTR, file, a0, STRPTR, buffer, a1, LONG, len, d0, \
  , GENESIS_BASE_NAME)

#endif /* !_INLINE_GENESIS_H */
