#ifndef _PPCINLINE_MIAMI_H
#define _PPCINLINE_MIAMI_H

#ifndef __PPCINLINE_MACROS_H
#include <ppcinline/macros.h>
#endif

#ifndef MIAMI_BASE_NAME
#define MIAMI_BASE_NAME MiamiBase
#endif

#define MiamiSysCtl(par1, par2, par3, par4, par5, last) \
	LP6(0x1e, long, MiamiSysCtl, long *, par1, a0, unsigned long, par2, d0, void *, par3, a1, unsigned long *, par4, a2, void *, par5, a3, long, last, d1, \
	, MIAMI_BASE_NAME, IF_CACHEFLUSHALL, NULL, 0, IF_CACHEFLUSHALL, NULL, 0)

#define MiamiDisallowDNS(last) \
	LP1NR(0x42, MiamiDisallowDNS, long, last, d0, \
	, MIAMI_BASE_NAME, IF_CACHEFLUSHALL, NULL, 0, IF_CACHEFLUSHALL, NULL, 0)

#define MiamiGetPid() \
	LP0(0x4e, void *, MiamiGetPid, \
	, MIAMI_BASE_NAME, IF_CACHEFLUSHALL, NULL, 0, IF_CACHEFLUSHALL, NULL, 0)

#define MiamiPFAddHook(par1, par2, last) \
	LP3(0x60, void *, MiamiPFAddHook, struct Hook *, par1, a0, unsigned char *, par2, a1, struct TagItem *, last, a2, \
	, MIAMI_BASE_NAME, IF_CACHEFLUSHALL, NULL, 0, IF_CACHEFLUSHALL, NULL, 0)

#define MiamiPFRemoveHook(last) \
	LP1NR(0x66, MiamiPFRemoveHook, void *, last, a0, \
	, MIAMI_BASE_NAME, IF_CACHEFLUSHALL, NULL, 0, IF_CACHEFLUSHALL, NULL, 0)

#define MiamiGetHardwareLen(last) \
	LP1(0x6c, long, MiamiGetHardwareLen, char *, last, a0, \
	, MIAMI_BASE_NAME, IF_CACHEFLUSHALL, NULL, 0, IF_CACHEFLUSHALL, NULL, 0)

#define MiamiOpenSSL(last) \
	LP1(0x96, struct Library *, MiamiOpenSSL, struct TagItem *, last, a0, \
	, MIAMI_BASE_NAME, IF_CACHEFLUSHALL, NULL, 0, IF_CACHEFLUSHALL, NULL, 0)

#define MiamiCloseSSL() \
	LP0NR(0x9c, MiamiCloseSSL, \
	, MIAMI_BASE_NAME, IF_CACHEFLUSHALL, NULL, 0, IF_CACHEFLUSHALL, NULL, 0)

#define MiamiSetSocksConn(par1, last) \
	LP2(0xc6, long, MiamiSetSocksConn, struct sockaddr *, par1, a0, long, last, d0, \
	, MIAMI_BASE_NAME, IF_CACHEFLUSHALL, NULL, 0, IF_CACHEFLUSHALL, NULL, 0)

#define MiamiIsOnline(last) \
	LP1(0xd2, long, MiamiIsOnline, char *, last, a0, \
	, MIAMI_BASE_NAME, IF_CACHEFLUSHALL, NULL, 0, IF_CACHEFLUSHALL, NULL, 0)

#define MiamiOnOffline(par1, last) \
	LP2NR(0xd8, MiamiOnOffline, char *, par1, a0, long, last, d0, \
	, MIAMI_BASE_NAME, IF_CACHEFLUSHALL, NULL, 0, IF_CACHEFLUSHALL, NULL, 0)

#define inet_ntop(par1, par2, par3, last) \
	LP4(0xe4, char *, inet_ntop, long, par1, d0, void *, par2, a0, char *, par3, a1, long, last, d1, \
	, MIAMI_BASE_NAME, IF_CACHEFLUSHALL, NULL, 0, IF_CACHEFLUSHALL, NULL, 0)

#define inet_aton(par1, last) \
	LP2(0xea, long, inet_aton, char *, par1, a0, struct in_addr *, last, a1, \
	, MIAMI_BASE_NAME, IF_CACHEFLUSHALL, NULL, 0, IF_CACHEFLUSHALL, NULL, 0)

#define inet_pton(par1, par2, last) \
	LP3(0xf0, long, inet_pton, long, par1, d0, char *, par2, a0, void *, last, a1, \
	, MIAMI_BASE_NAME, IF_CACHEFLUSHALL, NULL, 0, IF_CACHEFLUSHALL, NULL, 0)

#define gethostbyname2(par1, last) \
	LP2(0xf6, struct hostent *, gethostbyname2, char *, par1, a0, long, last, d0, \
	, MIAMI_BASE_NAME, IF_CACHEFLUSHALL, NULL, 0, IF_CACHEFLUSHALL, NULL, 0)

#define gai_strerror(last) \
	LP1(0xfc, char *, gai_strerror, long, last, d0, \
	, MIAMI_BASE_NAME, IF_CACHEFLUSHALL, NULL, 0, IF_CACHEFLUSHALL, NULL, 0)

#define freeaddrinfo(last) \
	LP1NR(0x102, freeaddrinfo, struct addrinfo *, last, a0, \
	, MIAMI_BASE_NAME, IF_CACHEFLUSHALL, NULL, 0, IF_CACHEFLUSHALL, NULL, 0)

#define getaddrinfo(par1, par2, par3, last) \
	LP4(0x108, long, getaddrinfo, char *, par1, a0, char *, par2, a1, struct addrinfo *, par3, a2, struct addrinfo **, last, a3, \
	, MIAMI_BASE_NAME, IF_CACHEFLUSHALL, NULL, 0, IF_CACHEFLUSHALL, NULL, 0)

#define getnameinfo(par1, par2, par3, par4, par5, par6, last) \
	LP7(0x10e, long, getnameinfo, struct sockaddr *, par1, a0, long, par2, d0, char *, par3, a1, long, par4, d1, char *, par5, a2, long, par6, d2, long, last, d3, \
	, MIAMI_BASE_NAME, IF_CACHEFLUSHALL, NULL, 0, IF_CACHEFLUSHALL, NULL, 0)

#define if_nametoindex(last) \
	LP1(0x114, long, if_nametoindex, char *, last, a0, \
	, MIAMI_BASE_NAME, IF_CACHEFLUSHALL, NULL, 0, IF_CACHEFLUSHALL, NULL, 0)

#define if_indextoname(par1, last) \
	LP2(0x11a, char *, if_indextoname, long, par1, d0, char *, last, a0, \
	, MIAMI_BASE_NAME, IF_CACHEFLUSHALL, NULL, 0, IF_CACHEFLUSHALL, NULL, 0)

#define if_nameindex() \
	LP0(0x120, struct if_nameindex *, if_nameindex, \
	, MIAMI_BASE_NAME, IF_CACHEFLUSHALL, NULL, 0, IF_CACHEFLUSHALL, NULL, 0)

#define if_freenameindex(last) \
	LP1NR(0x126, if_freenameindex, struct if_nameindex *, last, a0, \
	, MIAMI_BASE_NAME, IF_CACHEFLUSHALL, NULL, 0, IF_CACHEFLUSHALL, NULL, 0)

#define MiamiSupportsIPV6() \
	LP0(0x12c, long, MiamiSupportsIPV6, \
	, MIAMI_BASE_NAME, IF_CACHEFLUSHALL, NULL, 0, IF_CACHEFLUSHALL, NULL, 0)

#define MiamiResGetOptions() \
	LP0(0x132, long, MiamiResGetOptions, \
	, MIAMI_BASE_NAME, IF_CACHEFLUSHALL, NULL, 0, IF_CACHEFLUSHALL, NULL, 0)

#define MiamiResSetOptions(last) \
	LP1NR(0x138, MiamiResSetOptions, long, last, d0, \
	, MIAMI_BASE_NAME, IF_CACHEFLUSHALL, NULL, 0, IF_CACHEFLUSHALL, NULL, 0)

#define sockatmark(last) \
	LP1(0x13e, long, sockatmark, long, last, d0, \
	, MIAMI_BASE_NAME, IF_CACHEFLUSHALL, NULL, 0, IF_CACHEFLUSHALL, NULL, 0)

#define MiamiSupportedCPUs(par1, par2, last) \
	LP3NR(0x144, MiamiSupportedCPUs, unsigned long *, par1, a0, unsigned long *, par2, a1, unsigned long *, last, a2, \
	, MIAMI_BASE_NAME, IF_CACHEFLUSHALL, NULL, 0, IF_CACHEFLUSHALL, NULL, 0)

#define MiamiGetFdCallback(last) \
	LP1(0x14a, long, MiamiGetFdCallback, void **, last, a0, \
	, MIAMI_BASE_NAME, IF_CACHEFLUSHALL, NULL, 0, IF_CACHEFLUSHALL, NULL, 0)

#define MiamiSetFdCallback(par1, last) \
	LP2(0x150, long, MiamiSetFdCallback, void *, par1, a0, long, last, d0, \
	, MIAMI_BASE_NAME, IF_CACHEFLUSHALL, NULL, 0, IF_CACHEFLUSHALL, NULL, 0)

#define MiamiGetCredentials() \
	LP0(0x15c, struct UserGroupCredentials *, MiamiGetCredentials, \
	, MIAMI_BASE_NAME, IF_CACHEFLUSHALL, NULL, 0, IF_CACHEFLUSHALL, NULL, 0)

#endif /*  _PPCINLINE_MIAMI_H  */
