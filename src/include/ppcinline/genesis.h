/* Automatically generated header! Do not edit! */

#ifndef _PPCINLINE_GENESIS_H
#define _PPCINLINE_GENESIS_H

#ifndef __PPCINLINE_MACROS_H
#include <ppcinline/macros.h>
#endif /* !__PPCINLINE_MACROS_H */

#ifndef GENESIS_BASE_NAME
#define GENESIS_BASE_NAME GenesisBase
#endif /* !GENESIS_BASE_NAME */

#define GetUserName(__p0, __p1, __p2) \
  LP3(72, BOOL , GetUserName, \
    LONG , __p0, d0, \
    char *, __p1, a0, \
    LONG , __p2, d1, \
    , GENESIS_BASE_NAME, 0, 0, 0, 0, 0, 0)

#define FreeUser(__p0) \
  LP1NR(66, FreeUser, \
    struct genUser *, __p0, a0, \
    , GENESIS_BASE_NAME, 0, 0, 0, 0, 0, 0)

#define IsOnline(__p0) \
  LP1(120, BOOL , IsOnline, \
    LONG , __p0, d0, \
    , GENESIS_BASE_NAME, 0, 0, 0, 0, 0, 0)

#define ParseNextLine(__p0) \
  LP1(48, BOOL , ParseNextLine, \
    struct ParseConfig_Data *, __p0, a0, \
    , GENESIS_BASE_NAME, 0, 0, 0, 0, 0, 0)

#define ClearUserList() \
  LP0NR(96, ClearUserList, \
    , GENESIS_BASE_NAME, 0, 0, 0, 0, 0, 0)

#define WriteFile(__p0, __p1, __p2) \
  LP3(114, BOOL , WriteFile, \
    STRPTR , __p0, a0, \
    STRPTR , __p1, a1, \
    LONG , __p2, d0, \
    , GENESIS_BASE_NAME, 0, 0, 0, 0, 0, 0)

#define GetGlobalUser() \
  LP0(84, struct genUser *, GetGlobalUser, \
    , GENESIS_BASE_NAME, 0, 0, 0, 0, 0, 0)

#define ParseConfig(__p0, __p1) \
  LP2(36, BOOL , ParseConfig, \
    STRPTR , __p0, a0, \
    struct ParseConfig_Data *, __p1, a1, \
    , GENESIS_BASE_NAME, 0, 0, 0, 0, 0, 0)

#define ReloadUserList() \
  LP0(102, BOOL , ReloadUserList, \
    , GENESIS_BASE_NAME, 0, 0, 0, 0, 0, 0)

#define ParseEnd(__p0) \
  LP1NR(54, ParseEnd, \
    struct ParseConfig_Data *, __p0, a0, \
    , GENESIS_BASE_NAME, 0, 0, 0, 0, 0, 0)

#define ParseNext(__p0) \
  LP1(42, BOOL , ParseNext, \
    struct ParseConfig_Data *, __p0, a0, \
    , GENESIS_BASE_NAME, 0, 0, 0, 0, 0, 0)

#define GetFileSize(__p0) \
  LP1(30, LONG , GetFileSize, \
    STRPTR , __p0, a0, \
    , GENESIS_BASE_NAME, 0, 0, 0, 0, 0, 0)

#define SetGlobalUser(__p0) \
  LP1NR(90, SetGlobalUser, \
    struct genUser *, __p0, a0, \
    , GENESIS_BASE_NAME, 0, 0, 0, 0, 0, 0)

#define ReadFile(__p0, __p1, __p2) \
  LP3(108, LONG , ReadFile, \
    STRPTR , __p0, a0, \
    STRPTR , __p1, a1, \
    LONG , __p2, d0, \
    , GENESIS_BASE_NAME, 0, 0, 0, 0, 0, 0)

#define ReallocCopy(__p0, __p1) \
  LP2(60, STRPTR , ReallocCopy, \
    STRPTR *, __p0, a0, \
    STRPTR , __p1, a1, \
    , GENESIS_BASE_NAME, 0, 0, 0, 0, 0, 0)

#define GetUser(__p0, __p1, __p2, __p3) \
  LP4(78, struct genUser *, GetUser, \
    STRPTR , __p0, a0, \
    STRPTR , __p1, a1, \
    STRPTR , __p2, a2, \
    LONG , __p3, d0, \
    , GENESIS_BASE_NAME, 0, 0, 0, 0, 0, 0)

#endif /* !_PPCINLINE_GENESIS_H */
