#if defined(AZTEC_C) || defined(__MAXON__) || defined(__STORM__)
#pragma amicall(SocketBase,0x01E,Socket(d0,d1,d2))
#pragma amicall(SocketBase,0x036,Connect(d0,a0,d1))
#pragma amicall(SocketBase,0x042,Send(d0,a0,d1,d2))
#pragma amicall(SocketBase,0x04E,Recv(d0,a0,d1,d2))
#pragma amicall(SocketBase,0x054,Shutdown(d0,d1))
#pragma amicall(SocketBase,0x072,IoctlSocket(d0,d1,a0))
#pragma amicall(SocketBase,0x078,CloseSocket(d0))
#pragma amicall(SocketBase,0x090,ObtainSocket(d0,d1,d2,d3))
#pragma amicall(SocketBase,0x096,ReleaseSocket(d0,d1))
#pragma amicall(SocketBase,0x09C,ReleaseCopyOfSocket(d0,d1))
#pragma amicall(SocketBase,0x0A2,Errno())
#pragma amicall(SocketBase,0x0A8,SetErrnoPtr(a0,d0))
#pragma amicall(SocketBase,0x0AE,Inet_NtoA(d0))
#pragma amicall(SocketBase,0x0BA,Inet_LnaOf(d0))
#pragma amicall(SocketBase,0x0C0,Inet_NetOf(d0))
#pragma amicall(SocketBase,0x0C6,Inet_MakeAddr(d0,d1))
#pragma amicall(SocketBase,0x0D2,GetHostByName(a0))
#pragma amicall(SocketBase,0x108,Dup2Socket(d0,d1))
#pragma amicall(SocketBase,0x126,SocketBaseTagList(a0))
#pragma amicall(SocketBase,0x12C,GetSocketEvents(a0))
#ifdef __STORM__
#pragma tagcall(SocketBase,0x126,SocketBaseTags(a0))
#endif
#else
#pragma  libcall SocketBase Socket               01E 21003
#pragma  libcall SocketBase Connect              036 18003
#pragma  libcall SocketBase Send                 042 218004
#pragma  libcall SocketBase Recv                 04E 218004
#pragma  libcall SocketBase Shutdown             054 1002
#pragma  libcall SocketBase IoctlSocket          072 81003
#pragma  libcall SocketBase CloseSocket          078 001
#pragma  libcall SocketBase ObtainSocket         090 321004
#pragma  libcall SocketBase ReleaseSocket        096 1002
#pragma  libcall SocketBase ReleaseCopyOfSocket  09C 1002
#pragma  libcall SocketBase Errno                0A2 00
#pragma  libcall SocketBase SetErrnoPtr          0A8 0802
#pragma  libcall SocketBase Inet_NtoA            0AE 001
#pragma  libcall SocketBase Inet_LnaOf           0BA 001
#pragma  libcall SocketBase Inet_NetOf           0C0 001
#pragma  libcall SocketBase Inet_MakeAddr        0C6 1002
#pragma  libcall SocketBase GetHostByName        0D2 801
#pragma  libcall SocketBase Dup2Socket           108 1002
#pragma  libcall SocketBase SocketBaseTagList    126 801
#pragma  libcall SocketBase GetSocketEvents      12C 801
#ifdef __SASC_60
#pragma  tagcall SocketBase SocketBaseTags       126 801
#endif
#endif
