/* Automatically generated header! Do not edit! */

#ifndef _PPCINLINE_MIAMI_H
#define _PPCINLINE_MIAMI_H

#ifndef __PPCINLINE_MACROS_H
#include <ppcinline/macros.h>
#endif /* !__PPCINLINE_MACROS_H */

#ifndef MIAMI_BASE_NAME
#define MIAMI_BASE_NAME MiamiBase
#endif /* !MIAMI_BASE_NAME */

#define inet_aton(__p0, __p1) \
	LP2(234, long , inet_aton, \
		char *, __p0, a0, \
		struct in_addr *, __p1, a1, \
		, MIAMI_BASE_NAME, 0, 0, 0, 0, 0, 0)

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

#define getaddrinfo(__p0, __p1, __p2, __p3) \
	LP4(264, long , getaddrinfo, \
		char *, __p0, a0, \
		char *, __p1, a1, \
		struct addrinfo *, __p2, a2, \
		struct addrinfo **, __p3, a3, \
		, MIAMI_BASE_NAME, 0, 0, 0, 0, 0, 0)

#define if_nameindex() \
	LP0(288, struct if_nameindex *, if_nameindex, \
		, MIAMI_BASE_NAME, 0, 0, 0, 0, 0, 0)

#define if_freenameindex(__p0) \
	LP1NR(294, if_freenameindex, \
		struct if_nameindex *, __p0, a0, \
		, MIAMI_BASE_NAME, 0, 0, 0, 0, 0, 0)

#define freeaddrinfo(__p0) \
	LP1NR(258, freeaddrinfo, \
		struct addrinfo *, __p0, a0, \
		, MIAMI_BASE_NAME, 0, 0, 0, 0, 0, 0)

#define if_indextoname(__p0, __p1) \
	LP2(282, char *, if_indextoname, \
		long , __p0, d0, \
		char *, __p1, a0, \
		, MIAMI_BASE_NAME, 0, 0, 0, 0, 0, 0)

#define if_nametoindex(__p0) \
	LP1(276, long , if_nametoindex, \
		char *, __p0, a0, \
		, MIAMI_BASE_NAME, 0, 0, 0, 0, 0, 0)

#define MiamiGetPid() \
	LP0(78, void *, MiamiGetPid, \
		, MIAMI_BASE_NAME, 0, 0, 0, 0, 0, 0)

#define MiamiSetSocksConn(__p0, __p1) \
	LP2(198, long , MiamiSetSocksConn, \
		struct sockaddr *, __p0, a0, \
		long , __p1, d0, \
		, MIAMI_BASE_NAME, 0, 0, 0, 0, 0, 0)

#define gethostbyname2(__p0, __p1) \
	LP2(246, struct hostent *, gethostbyname2, \
		char *, __p0, a0, \
		long , __p1, d0, \
		, MIAMI_BASE_NAME, 0, 0, 0, 0, 0, 0)

#define MiamiOnOffline(__p0, __p1) \
	LP2NR(216, MiamiOnOffline, \
		char *, __p0, a0, \
		long , __p1, d0, \
		, MIAMI_BASE_NAME, 0, 0, 0, 0, 0, 0)

#define getnameinfo(__p0, __p1, __p2, __p3, __p4, __p5, __p6) \
	LP7(270, long , getnameinfo, \
		struct sockaddr *, __p0, a0, \
		long , __p1, d0, \
		char *, __p2, a1, \
		long , __p3, d1, \
		char *, __p4, a2, \
		long , __p5, d2, \
		long , __p6, d3, \
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

#define inet_pton(__p0, __p1, __p2) \
	LP3(240, long , inet_pton, \
		long , __p0, d0, \
		char *, __p1, a0, \
		void *, __p2, a1, \
		, MIAMI_BASE_NAME, 0, 0, 0, 0, 0, 0)

#define inet_ntop(__p0, __p1, __p2, __p3) \
	LP4(228, char *, inet_ntop, \
		long , __p0, d0, \
		void *, __p1, a0, \
		char *, __p2, a1, \
		long , __p3, d1, \
		, MIAMI_BASE_NAME, 0, 0, 0, 0, 0, 0)

#define sockatmark(__p0) \
	LP1(318, long , sockatmark, \
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

#define gai_strerror(__p0) \
	LP1(252, char *, gai_strerror, \
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
