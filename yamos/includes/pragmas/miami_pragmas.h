#ifdef __PPC__
#include <ppcpragmas/miami_pragmas.h>
#else
#if defined(AZTEC_C) || defined(__MAXON__) || defined(__STORM__)
#pragma amicall(MiamiBase,0x01E,MiamiSysCtl(a0,d0,a1,a2,a3,d1))
#pragma amicall(MiamiBase,0x042,MiamiDisallowDNS(d0))
#pragma amicall(MiamiBase,0x04E,MiamiGetPid())
#pragma amicall(MiamiBase,0x060,MiamiPFAddHook(a0,a1,a2))
#pragma amicall(MiamiBase,0x066,MiamiPFRemoveHook(a0))
#pragma amicall(MiamiBase,0x06C,MiamiGetHardwareLen(a0))
#pragma amicall(MiamiBase,0x096,MiamiOpenSSL(a0))
#pragma amicall(MiamiBase,0x09C,MiamiCloseSSL())
#pragma amicall(MiamiBase,0x0C6,MiamiSetSocksConn(a0,d0))
#pragma amicall(MiamiBase,0x0D2,MiamiIsOnline(a0))
#pragma amicall(MiamiBase,0x0D8,MiamiOnOffline(a0,d0))
#pragma amicall(MiamiBase,0x0E4,inet_ntop(d0,a0,a1,d1))
#pragma amicall(MiamiBase,0x0EA,inet_aton(a0,a1))
#pragma amicall(MiamiBase,0x0F0,inet_pton(d0,a0,a1))
#pragma amicall(MiamiBase,0x0F6,gethostbyname2(a0,d0))
#pragma amicall(MiamiBase,0x0FC,gai_strerror(d0))
#pragma amicall(MiamiBase,0x102,freeaddrinfo(a0))
#pragma amicall(MiamiBase,0x108,getaddrinfo(a0,a1,a2,a3))
#pragma amicall(MiamiBase,0x10E,getnameinfo(a0,d0,a1,d1,a2,d2,d3))
#pragma amicall(MiamiBase,0x114,if_nametoindex(a0))
#pragma amicall(MiamiBase,0x11A,if_indextoname(d0,a0))
#pragma amicall(MiamiBase,0x120,if_nameindex())
#pragma amicall(MiamiBase,0x126,if_freenameindex(a0))
#pragma amicall(MiamiBase,0x12C,MiamiSupportsIPV6())
#pragma amicall(MiamiBase,0x132,MiamiResGetOptions())
#pragma amicall(MiamiBase,0x138,MiamiResSetOptions(d0))
#pragma amicall(MiamiBase,0x13E,sockatmark(d0))
#pragma amicall(MiamiBase,0x144,MiamiSupportedCPUs(a0,a1,a2))
#pragma amicall(MiamiBase,0x14A,MiamiGetFdCallback(a0))
#pragma amicall(MiamiBase,0x150,MiamiSetFdCallback(a0,d0))
#else
#pragma libcall MiamiBase MiamiSysCtl 1e 1BA90806
#pragma libcall MiamiBase MiamiDisallowDNS 42 001
#pragma libcall MiamiBase MiamiGetPid 4e 0
#pragma libcall MiamiBase MiamiPFAddHook 60 A9803
#pragma libcall MiamiBase MiamiPFRemoveHook 66 801
#pragma libcall MiamiBase MiamiGetHardwareLen 6c 801
#pragma libcall MiamiBase MiamiOpenSSL 96 801
#pragma libcall MiamiBase MiamiCloseSSL 9c 0
#pragma libcall MiamiBase MiamiSetSocksConn c6 0802
#pragma libcall MiamiBase MiamiIsOnline d2 801
#pragma libcall MiamiBase MiamiOnOffline d8 0802
#pragma libcall MiamiBase inet_ntop e4 198004
#pragma libcall MiamiBase inet_aton ea 9802
#pragma libcall MiamiBase inet_pton f0 98003
#pragma libcall MiamiBase gethostbyname2 f6 0802
#pragma libcall MiamiBase gai_strerror fc 001
#pragma libcall MiamiBase freeaddrinfo 102 801
#pragma libcall MiamiBase getaddrinfo 108 BA9804
#pragma libcall MiamiBase getnameinfo 10e 32A190807
#pragma libcall MiamiBase if_nametoindex 114 801
#pragma libcall MiamiBase if_indextoname 11a 8002
#pragma libcall MiamiBase if_nameindex 120 0
#pragma libcall MiamiBase if_freenameindex 126 801
#pragma libcall MiamiBase MiamiSupportsIPV6 12c 0
#pragma libcall MiamiBase MiamiResGetOptions 132 0
#pragma libcall MiamiBase MiamiResSetOptions 138 001
#pragma libcall MiamiBase sockatmark 13e 001
#pragma libcall MiamiBase MiamiSupportedCPUs 144 A9803
#pragma libcall MiamiBase MiamiGetFdCallback 14a 801
#pragma libcall MiamiBase MiamiSetFdCallback 150 0802
#pragma libcall MiamiBase MiamiGetCredentials 15c 0
#endif
#endif
