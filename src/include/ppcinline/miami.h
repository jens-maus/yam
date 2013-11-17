/* Automatically generated header! Do not edit! */

#ifndef _PPCINLINE_MIAMI_H
#define _PPCINLINE_MIAMI_H

#ifndef __PPCINLINE_MACROS_H
#include <ppcinline/macros.h>
#endif /* !__PPCINLINE_MACROS_H */

#ifndef MIAMI_BASE_NAME
#define MIAMI_BASE_NAME MiamiBase
#endif /* !MIAMI_BASE_NAME */

#define MiamiPFRemoveHook(__p0) \
  LP1NR(102, MiamiPFRemoveHook, \
    void *, __p0, a0, \
    , MIAMI_BASE_NAME, 0, 0, 0, 0, 0, 0)

#define MiamiSupportsIPV6() \
  LP0(300, long , MiamiSupportsIPV6, \
    , MIAMI_BASE_NAME, 0, 0, 0, 0, 0, 0)

#define MiamiResGetOptions() \
  LP0(306, long , MiamiResGetOptions, \
    , MIAMI_BASE_NAME, 0, 0, 0, 0, 0, 0)

#define MiamiGetFdCallback(__p0) \
  LP1(330, long , MiamiGetFdCallback, \
    void **, __p0, a0, \
    , MIAMI_BASE_NAME, 0, 0, 0, 0, 0, 0)

#define MiamiGetPid() \
  LP0(78, void *, MiamiGetPid, \
    , MIAMI_BASE_NAME, 0, 0, 0, 0, 0, 0)

#define MiamiSetSocksConn(__p0, __p1) \
  LP2(198, long , MiamiSetSocksConn, \
    struct sockaddr *, __p0, a0, \
    long , __p1, d0, \
    , MIAMI_BASE_NAME, 0, 0, 0, 0, 0, 0)

#define MiamiOnOffline(__p0, __p1) \
  LP2NR(216, MiamiOnOffline, \
    char *, __p0, a0, \
    long , __p1, d0, \
    , MIAMI_BASE_NAME, 0, 0, 0, 0, 0, 0)

#define MiamiIsOnline(__p0) \
  LP1(210, long , MiamiIsOnline, \
    char *, __p0, a0, \
    , MIAMI_BASE_NAME, 0, 0, 0, 0, 0, 0)

#define MiamiSupportedCPUs(__p0, __p1, __p2) \
  LP3NR(324, MiamiSupportedCPUs, \
    unsigned long *, __p0, a0, \
    unsigned long *, __p1, a1, \
    unsigned long *, __p2, a2, \
    , MIAMI_BASE_NAME, 0, 0, 0, 0, 0, 0)

#define MiamiSysCtl(__p0, __p1, __p2, __p3, __p4, __p5) \
  LP6(30, long , MiamiSysCtl, \
    long *, __p0, a0, \
    unsigned , __p1, d0, \
    void *, __p2, a1, \
    unsigned long *, __p3, a2, \
    void *, __p4, a3, \
    long , __p5, d1, \
    , MIAMI_BASE_NAME, 0, 0, 0, 0, 0, 0)

#define MiamiResSetOptions(__p0) \
  LP1NR(312, MiamiResSetOptions, \
    long , __p0, d0, \
    , MIAMI_BASE_NAME, 0, 0, 0, 0, 0, 0)

#define MiamiSetFdCallback(__p0, __p1) \
  LP2(336, long , MiamiSetFdCallback, \
    void *, __p0, a0, \
    long , __p1, d0, \
    , MIAMI_BASE_NAME, 0, 0, 0, 0, 0, 0)

#define MiamiOpenSSL(__p0) \
  LP1(150, struct Library *, MiamiOpenSSL, \
    struct TagItem *, __p0, a0, \
    , MIAMI_BASE_NAME, 0, 0, 0, 0, 0, 0)

#define MiamiDisallowDNS(__p0) \
  LP1NR(66, MiamiDisallowDNS, \
    long , __p0, d0, \
    , MIAMI_BASE_NAME, 0, 0, 0, 0, 0, 0)

#define MiamiCloseSSL() \
  LP0NR(156, MiamiCloseSSL, \
    , MIAMI_BASE_NAME, 0, 0, 0, 0, 0, 0)

#define MiamiGetHardwareLen(__p0) \
  LP1(108, long , MiamiGetHardwareLen, \
    char *, __p0, a0, \
    , MIAMI_BASE_NAME, 0, 0, 0, 0, 0, 0)

#define MiamiGetCredentials() \
  LP0(348, struct UserGroupCredentials *, MiamiGetCredentials, \
    , MIAMI_BASE_NAME, 0, 0, 0, 0, 0, 0)

#define MiamiPFAddHook(__p0, __p1, __p2) \
  LP3(96, void *, MiamiPFAddHook, \
    struct Hook *, __p0, a0, \
    unsigned char *, __p1, a1, \
    struct TagItem *, __p2, a2, \
    , MIAMI_BASE_NAME, 0, 0, 0, 0, 0, 0)

#endif /* !_PPCINLINE_MIAMI_H */
