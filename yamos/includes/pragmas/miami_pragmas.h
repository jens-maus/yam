#ifdef __PPC__
#include <ppcpragmas/miami_pragmas.h>
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
