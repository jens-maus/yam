/* Automatically generated header! Do not edit! */

#ifndef _INLINE_MIAMI_H
#define _INLINE_MIAMI_H

#ifndef __INLINE_MACROS_H
#include <inline/macros.h>
#endif /* !__INLINE_MACROS_H */

#ifndef MIAMI_BASE_NAME
#define MIAMI_BASE_NAME MiamiBase
#endif /* !MIAMI_BASE_NAME */

#define MiamiCloseSSL() \
  LP0NR(0x9c, MiamiCloseSSL, \
  , MIAMI_BASE_NAME)

#define MiamiDisallowDNS(v) \
  LP1NR(0x42, MiamiDisallowDNS, long, v, d0, \
  , MIAMI_BASE_NAME)

#define MiamiGetFdCallback(cbptr) \
  LP1(0x14a, long, MiamiGetFdCallback, void **, cbptr, a0, \
  , MIAMI_BASE_NAME)

#define MiamiGetHardwareLen(name) \
  LP1(0x6c, long, MiamiGetHardwareLen, char *, name, a0, \
  , MIAMI_BASE_NAME)

#define MiamiGetPid() \
  LP0(0x4e, void *, MiamiGetPid, \
  , MIAMI_BASE_NAME)

#define MiamiIsOnline(name) \
  LP1(0xd2, long, MiamiIsOnline, char *, name, a0, \
  , MIAMI_BASE_NAME)

#define MiamiOnOffline(name, val) \
  LP2NR(0xd8, MiamiOnOffline, char *, name, a0, long, val, d0, \
  , MIAMI_BASE_NAME)

#define MiamiOpenSSL(tags) \
  LP1(0x96, struct Library *, MiamiOpenSSL, struct TagItem *, tags, a0, \
  , MIAMI_BASE_NAME)

#ifndef NO_INLINE_STDARG
#define MiamiOpenSSLTags(tags...) \
  ({ULONG _tags[] = { tags }; MiamiOpenSSL((struct TagItem *)_tags);})
#endif /* !NO_INLINE_STDARG */

#define MiamiPFAddHook(hook, name, tags) \
  LP3(0x60, void *, MiamiPFAddHook, struct Hook *, hook, a0, unsigned char *, name, a1, struct TagItem *, tags, a2, \
  , MIAMI_BASE_NAME)

#ifndef NO_INLINE_STDARG
#define MiamiPFAddHookTags(a0, a1, tags...) \
  ({ULONG _tags[] = { tags }; MiamiPFAddHook((a0), (a1), (struct TagItem *)_tags);})
#endif /* !NO_INLINE_STDARG */

#define MiamiPFRemoveHook(handle) \
  LP1NR(0x66, MiamiPFRemoveHook, void *, handle, a0, \
  , MIAMI_BASE_NAME)

#define MiamiResGetOptions() \
  LP0(0x132, long, MiamiResGetOptions, \
  , MIAMI_BASE_NAME)

#define MiamiResSetOptions(options) \
  LP1NR(0x138, MiamiResSetOptions, long, options, d0, \
  , MIAMI_BASE_NAME)

#define MiamiSetFdCallback(cb, cpu) \
  LP2(0x150, long, MiamiSetFdCallback, void *, cb, a0, long, cpu, d0, \
  , MIAMI_BASE_NAME)

#define MiamiSetSocksConn(in, len) \
  LP2(0xc6, long, MiamiSetSocksConn, struct sockaddr *, in, a0, long, len, d0, \
  , MIAMI_BASE_NAME)

#define MiamiSupportedCPUs(apis, callbacks, kernel) \
  LP3NR(0x144, MiamiSupportedCPUs, unsigned long *, apis, a0, unsigned long *, callbacks, a1, unsigned long *, kernel, a2, \
  , MIAMI_BASE_NAME)

#define MiamiSupportsIPV6() \
  LP0(0x12c, long, MiamiSupportsIPV6, \
  , MIAMI_BASE_NAME)

#define MiamiSysCtl(name, namelen, old, oldlen, new, newlen) \
  LP6(0x1e, long, MiamiSysCtl, long *, name, a0, unsigned long, namelen, d0, void *, old, a1, unsigned long *, oldlen, a2, void *, new, a3, long, newlen, d1, \
  , MIAMI_BASE_NAME)

#endif /* !_INLINE_MIAMI_H */
