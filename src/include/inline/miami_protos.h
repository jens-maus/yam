#ifndef _VBCCINLINE_MIAMI_H
#define _VBCCINLINE_MIAMI_H

#ifndef EXEC_TYPES_H
#include <exec/types.h>
#endif

long __MiamiSysCtl(__reg("a0") long * name, __reg("d0") unsigned long namelen, __reg("a1") void * old, __reg("a2") unsigned long * oldlen, __reg("a3") void * neu, __reg("d1") long neulen, __reg("a6") APTR)="\tjsr\t-30(a6)";
#define MiamiSysCtl(name, namelen, old, oldlen, neu, neulen) __MiamiSysCtl((name), (namelen), (old), (oldlen), (neu), (neulen), MiamiBase)

void __MiamiDisallowDNS(__reg("d0") long v, __reg("a6") APTR)="\tjsr\t-66(a6)";
#define MiamiDisallowDNS(v) __MiamiDisallowDNS((v), MiamiBase)

void * __MiamiGetPid(__reg("a6") APTR)="\tjsr\t-78(a6)";
#define MiamiGetPid() __MiamiGetPid(MiamiBase)

void * __MiamiPFAddHook(__reg("a0") struct Hook * hook, __reg("a1") unsigned char * name, __reg("a2") struct TagItem * tags, __reg("a6") APTR)="\tjsr\t-96(a6)";
#define MiamiPFAddHook(hook, name, tags) __MiamiPFAddHook((hook), (name), (tags), MiamiBase)

void __MiamiPFRemoveHook(__reg("a0") void * handle, __reg("a6") APTR)="\tjsr\t-102(a6)";
#define MiamiPFRemoveHook(handle) __MiamiPFRemoveHook((handle), MiamiBase)

long __MiamiGetHardwareLen(__reg("a0") char * name, __reg("a6") APTR)="\tjsr\t-108(a6)";
#define MiamiGetHardwareLen(name) __MiamiGetHardwareLen((name), MiamiBase)

APTR __MiamiOpenSSL(__reg("a0") struct TagItem * tags, __reg("a6") APTR)="\tjsr\t-150(a6)";
#define MiamiOpenSSL(tags) __MiamiOpenSSL((tags), MiamiBase)

void __MiamiCloseSSL(__reg("a6") APTR)="\tjsr\t-156(a6)";
#define MiamiCloseSSL() __MiamiCloseSSL(MiamiBase)

long __MiamiSetSocksConn(__reg("a0") struct sockaddr * in, __reg("d0") long len, __reg("a6") APTR)="\tjsr\t-198(a6)";
#define MiamiSetSocksConn(in, len) __MiamiSetSocksConn((in), (len), MiamiBase)

long __MiamiIsOnline(__reg("a0") char * name, __reg("a6") APTR)="\tjsr\t-210(a6)";
#define MiamiIsOnline(name) __MiamiIsOnline((name), MiamiBase)

void __MiamiOnOffline(__reg("a0") char * name, __reg("d0") long val, __reg("a6") APTR)="\tjsr\t-216(a6)";
#define MiamiOnOffline(name, val) __MiamiOnOffline((name), (val), MiamiBase)

long __MiamiSupportsIPV6(__reg("a6") APTR)="\tjsr\t-300(a6)";
#define MiamiSupportsIPV6() __MiamiSupportsIPV6(MiamiBase)

long __MiamiResGetOptions(__reg("a6") APTR)="\tjsr\t-306(a6)";
#define MiamiResGetOptions() __MiamiResGetOptions(MiamiBase)

void __MiamiResSetOptions(__reg("d0") long options, __reg("a6") APTR)="\tjsr\t-312(a6)";
#define MiamiResSetOptions(options) __MiamiResSetOptions((options), MiamiBase)

void __MiamiSupportedCPUs(__reg("a0") unsigned long * apis, __reg("a1") unsigned long * callbacks, __reg("a2") unsigned long * kernel, __reg("a6") APTR)="\tjsr\t-324(a6)";
#define MiamiSupportedCPUs(apis, callbacks, kernel) __MiamiSupportedCPUs((apis), (callbacks), (kernel), MiamiBase)

long __MiamiGetFdCallback(__reg("a0") void ** cbptr, __reg("a6") APTR)="\tjsr\t-330(a6)";
#define MiamiGetFdCallback(cbptr) __MiamiGetFdCallback((cbptr), MiamiBase)

long __MiamiSetFdCallback(__reg("a0") void * cb, __reg("d0") long cpu, __reg("a6") APTR)="\tjsr\t-336(a6)";
#define MiamiSetFdCallback(cb, cpu) __MiamiSetFdCallback((cb), (cpu), MiamiBase)

#endif /*  _VBCCINLINE_MIAMI_H  */
