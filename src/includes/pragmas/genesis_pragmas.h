#if defined(AZTEC_C) || defined(__MAXON__) || defined(__STORM__)
#pragma amicall(GenesisBase,0x01E,GetFileSize(a0))
#pragma amicall(GenesisBase,0x024,ParseConfig(a0,a1))
#pragma amicall(GenesisBase,0x02A,ParseNext(a0))
#pragma amicall(GenesisBase,0x030,ParseNextLine(a0))
#pragma amicall(GenesisBase,0x036,ParseEnd(a0))
#pragma amicall(GenesisBase,0x03C,ReallocCopy(a0,a1))
#pragma amicall(GenesisBase,0x042,FreeUser(a0))
#pragma amicall(GenesisBase,0x048,GetUserName(d0,a0,d1))
#pragma amicall(GenesisBase,0x04E,GetUser(a0,a1,a2,d0))
#pragma amicall(GenesisBase,0x054,GetGlobalUser())
#pragma amicall(GenesisBase,0x05A,SetGlobalUser(a0))
#pragma amicall(GenesisBase,0x060,ClearUserList())
#pragma amicall(GenesisBase,0x066,ReloadUserList())
#pragma amicall(GenesisBase,0x06C,ReadFile(a0,a1,d0))
#pragma amicall(GenesisBase,0x072,WriteFile(a0,a1,d0))
#pragma amicall(GenesisBase,0x078,IsOnline(d0))
#else
#pragma libcall GenesisBase GetFileSize 1e 801
#pragma libcall GenesisBase ParseConfig 24 9802
#pragma libcall GenesisBase ParseNext 2a 801
#pragma libcall GenesisBase ParseNextLine 30 801
#pragma libcall GenesisBase ParseEnd 36 801
#pragma libcall GenesisBase ReallocCopy 3c 9802
#pragma libcall GenesisBase FreeUser 42 801
#pragma libcall GenesisBase GetUserName 48 18003
#pragma libcall GenesisBase GetUser 4e 0A9804
#pragma libcall GenesisBase GetGlobalUser 54 0
#pragma libcall GenesisBase SetGlobalUser 5a 801
#pragma libcall GenesisBase ClearUserList 60 0
#pragma libcall GenesisBase ReloadUserList 66 0
#pragma libcall GenesisBase ReadFile 6c 09803
#pragma libcall GenesisBase WriteFile 72 09803
#pragma libcall GenesisBase IsOnline 78 001
#endif
