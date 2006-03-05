#ifndef  CLIB_XPKMASTER_PROTOS_H
#define  CLIB_XPKMASTER_PROTOS_H

/*
**	$VER: clib/xpkmaster_protos.h 4.2 (28.10.1998) by SDI
**
**	(C) Copyright 1991-1996 by 
**          Urban Dominik Mueller, Bryan Ford,
**          Christian Schneider, Christian von Roques,
**	    Dirk Stöcker
**	    All Rights Reserved
*/

#ifndef EXEC_TYPES_H
#include <exec/types.h>
#endif

#ifndef XPK_XPK_H
#include <xpk/xpk.h>
#endif

LONG  XpkExamine	(struct XpkFib *fib, struct TagItem *tags);
LONG  XpkExamineTags	(struct XpkFib *fib, ULONG tag1Type, ...);
LONG  XpkPack		(struct TagItem *tags);
LONG  XpkPackTags	(ULONG tag1Type, ...);
LONG  XpkUnpack		(struct TagItem *tags);
LONG  XpkUnpackTags	(ULONG tag1Type, ...);
LONG  XpkOpen		(struct XpkFib **fib, struct TagItem *tags);
LONG  XpkOpenTags	(struct XpkFib **fib, ULONG tag1Type, ...);
LONG  XpkRead		(struct XpkFib *fib, STRPTR buf, ULONG len);
LONG  XpkWrite		(struct XpkFib *fib, STRPTR buf, LONG ulen);
LONG  XpkSeek		(struct XpkFib *fib, LONG dist, LONG mode);
LONG  XpkClose		(struct XpkFib *fib);
LONG  XpkQuery		(struct TagItem *tags);
LONG  XpkQueryTags	(ULONG tag1Type, ...);

/* here start version 4 functions */

APTR  XpkAllocObject	(ULONG type, struct TagItem *tags);
APTR  XpkAllocObjectTags(ULONG type, ULONG tag1Type, ...);
void  XpkFreeObject	(ULONG type, APTR object);
BOOL  XpkPrintFault	(LONG code, STRPTR header);
ULONG XpkFault		(LONG code, STRPTR header, STRPTR buffer, ULONG size);
LONG  XpkPassRequest	(struct TagItem *tags);
LONG  XpkPassRequestTags(ULONG tag1Type, ...);

#endif	/* CLIB_XPKMASTER_PROTOS_H */
