#ifndef PRAGMAS_XPKMASTER_PRAGMAS_H
#define PRAGMAS_XPKMASTER_PRAGMAS_H

#ifndef CLIB_XPKMASTER_PROTOS_H
#include <clib/xpkmaster_protos.h>
#endif

#if defined(AZTEC_C) || defined(__MAXON__) || defined(__STORM__)
#pragma amicall(XpkBase,0x024,XpkExamine(a0,a1))
#pragma amicall(XpkBase,0x02A,XpkPack(a0))
#pragma amicall(XpkBase,0x030,XpkUnpack(a0))
#pragma amicall(XpkBase,0x036,XpkOpen(a0,a1))
#pragma amicall(XpkBase,0x03C,XpkRead(a0,a1,d0))
#pragma amicall(XpkBase,0x042,XpkWrite(a0,a1,d0))
#pragma amicall(XpkBase,0x048,XpkSeek(a0,d0,d1))
#pragma amicall(XpkBase,0x04E,XpkClose(a0))
#pragma amicall(XpkBase,0x054,XpkQuery(a0))
#pragma amicall(XpkBase,0x05A,XpkAllocObject(d0,a0))
#pragma amicall(XpkBase,0x060,XpkFreeObject(d0,a0))
#pragma amicall(XpkBase,0x066,XpkPrintFault(d0,a0))
#pragma amicall(XpkBase,0x06C,XpkFault(d0,a0,a1,d1))
#pragma amicall(XpkBase,0x072,XpkPassRequest(a0))
#endif
#if defined(_DCC) || defined(__SASC)
#pragma libcall XpkBase XpkExamine           024 9802
#pragma libcall XpkBase XpkPack              02A 801
#pragma libcall XpkBase XpkUnpack            030 801
#pragma libcall XpkBase XpkOpen              036 9802
#pragma libcall XpkBase XpkRead              03C 09803
#pragma libcall XpkBase XpkWrite             042 09803
#pragma libcall XpkBase XpkSeek              048 10803
#pragma libcall XpkBase XpkClose             04E 801
#pragma libcall XpkBase XpkQuery             054 801
#pragma libcall XpkBase XpkAllocObject       05A 8002
#pragma libcall XpkBase XpkFreeObject        060 8002
#pragma libcall XpkBase XpkPrintFault        066 8002
#pragma libcall XpkBase XpkFault             06C 198004
#pragma libcall XpkBase XpkPassRequest       072 801
#endif
#ifdef __STORM__
#pragma tagcall(XpkBase,0x024,XpkExamineTags(a0,a1))
#pragma tagcall(XpkBase,0x02A,XpkPackTags(a0))
#pragma tagcall(XpkBase,0x030,XpkUnpackTags(a0))
#pragma tagcall(XpkBase,0x036,XpkOpenTags(a0,a1))
#pragma tagcall(XpkBase,0x054,XpkQueryTags(a0))
#pragma tagcall(XpkBase,0x05A,XpkAllocObjectTags(d0,a0))
#pragma tagcall(XpkBase,0x072,XpkPassRequestTags(a0))
#endif
#ifdef __SASC_60
#pragma tagcall XpkBase XpkExamineTags       024 9802
#pragma tagcall XpkBase XpkPackTags          02A 801
#pragma tagcall XpkBase XpkUnpackTags        030 801
#pragma tagcall XpkBase XpkOpenTags          036 9802
#pragma tagcall XpkBase XpkQueryTags         054 801
#pragma tagcall XpkBase XpkAllocObjectTags   05A 8002
#pragma tagcall XpkBase XpkPassRequestTags   072 801
#endif

#endif  /*  PRAGMAS_XPKMASTER_PRAGMAS_H  */
