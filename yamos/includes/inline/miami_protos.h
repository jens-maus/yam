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

char * __inet_ntop(__reg("d0") long family, __reg("a0") void * addrptr, __reg("a1") char * strptr, __reg("d1") long len, __reg("a6") APTR)="\tjsr\t-228(a6)";
#define inet_ntop(family, addrptr, strptr, len) __inet_ntop((family), (addrptr), (strptr), (len), MiamiBase)

long __inet_aton(__reg("a0") char * cp, __reg("a1") struct in_addr * addr, __reg("a6") APTR)="\tjsr\t-234(a6)";
#define inet_aton(cp, addr) __inet_aton((cp), (addr), MiamiBase)

long __inet_pton(__reg("d0") long family, __reg("a0") char * strptr, __reg("a1") void * addrptr, __reg("a6") APTR)="\tjsr\t-240(a6)";
#define inet_pton(family, strptr, addrptr) __inet_pton((family), (strptr), (addrptr), MiamiBase)

struct hostent * __gethostbyname2(__reg("a0") char * name, __reg("d0") long fam, __reg("a6") APTR)="\tjsr\t-246(a6)";
#define gethostbyname2(name, fam) __gethostbyname2((name), (fam), MiamiBase)

char * __gai_strerror(__reg("d0") long err, __reg("a6") APTR)="\tjsr\t-252(a6)";
#define gai_strerror(err) __gai_strerror((err), MiamiBase)

void __freeaddrinfo(__reg("a0") struct addrinfo * aihead, __reg("a6") APTR)="\tjsr\t-258(a6)";
#define freeaddrinfo(aihead) __freeaddrinfo((aihead), MiamiBase)

long __getaddrinfo(__reg("a0") char * hostname, __reg("a1") char * servname, __reg("a2") struct addrinfo * hintsp, __reg("a3") struct addrinfo ** result, __reg("a6") APTR)="\tjsr\t-264(a6)";
#define getaddrinfo(hostname, servname, hintsp, result) __getaddrinfo((hostname), (servname), (hintsp), (result), MiamiBase)

long __getnameinfo(__reg("a0") struct sockaddr * sa, __reg("d0") long salen, __reg("a1") char * host, __reg("d1") long hostlen, __reg("a2") char * serv, __reg("d2") long servlen, __reg("d3") long flags, __reg("a6") APTR)="\tjsr\t-270(a6)";
#define getnameinfo(sa, salen, host, hostlen, serv, servlen, flags) __getnameinfo((sa), (salen), (host), (hostlen), (serv), (servlen), (flags), MiamiBase)

long __if_nametoindex(__reg("a0") char * ifname, __reg("a6") APTR)="\tjsr\t-276(a6)";
#define if_nametoindex(ifname) __if_nametoindex((ifname), MiamiBase)

char * __if_indextoname(__reg("d0") long ifindex, __reg("a0") char * ifname, __reg("a6") APTR)="\tjsr\t-282(a6)";
#define if_indextoname(ifindex, ifname) __if_indextoname((ifindex), (ifname), MiamiBase)

struct if_nameindex * __if_nameindex(__reg("a6") APTR)="\tjsr\t-288(a6)";
#define if_nameindex() __if_nameindex(MiamiBase)

void __if_freenameindex(__reg("a0") struct if_nameindex * ptr, __reg("a6") APTR)="\tjsr\t-294(a6)";
#define if_freenameindex(ptr) __if_freenameindex((ptr), MiamiBase)

long __MiamiSupportsIPV6(__reg("a6") APTR)="\tjsr\t-300(a6)";
#define MiamiSupportsIPV6() __MiamiSupportsIPV6(MiamiBase)

long __MiamiResGetOptions(__reg("a6") APTR)="\tjsr\t-306(a6)";
#define MiamiResGetOptions() __MiamiResGetOptions(MiamiBase)

void __MiamiResSetOptions(__reg("d0") long options, __reg("a6") APTR)="\tjsr\t-312(a6)";
#define MiamiResSetOptions(options) __MiamiResSetOptions((options), MiamiBase)

long __sockatmark(__reg("d0") long fd, __reg("a6") APTR)="\tjsr\t-318(a6)";
#define sockatmark(fd) __sockatmark((fd), MiamiBase)

void __MiamiSupportedCPUs(__reg("a0") unsigned long * apis, __reg("a1") unsigned long * callbacks, __reg("a2") unsigned long * kernel, __reg("a6") APTR)="\tjsr\t-324(a6)";
#define MiamiSupportedCPUs(apis, callbacks, kernel) __MiamiSupportedCPUs((apis), (callbacks), (kernel), MiamiBase)

long __MiamiGetFdCallback(__reg("a0") void ** cbptr, __reg("a6") APTR)="\tjsr\t-330(a6)";
#define MiamiGetFdCallback(cbptr) __MiamiGetFdCallback((cbptr), MiamiBase)

long __MiamiSetFdCallback(__reg("a0") void * cb, __reg("d0") long cpu, __reg("a6") APTR)="\tjsr\t-336(a6)";
#define MiamiSetFdCallback(cb, cpu) __MiamiSetFdCallback((cb), (cpu), MiamiBase)

#endif /*  _VBCCINLINE_MIAMI_H  */
