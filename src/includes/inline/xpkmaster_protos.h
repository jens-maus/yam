#ifndef _VBCCINLINE_XPKMASTER_H
#define _VBCCINLINE_XPKMASTER_H

LONG __XpkExamine(__reg("a0") struct XpkFib * fib, __reg("a1") struct TagItem * tags, __reg("a6") void *)="\tjsr\t-36(a6)";
#define XpkExamine(fib, tags) __XpkExamine((fib), (tags), XpkBase)

LONG __XpkPack(__reg("a0") struct TagItem * tags, __reg("a6") void *)="\tjsr\t-42(a6)";
#define XpkPack(tags) __XpkPack((tags), XpkBase)

LONG __XpkUnpack(__reg("a0") struct TagItem * tags, __reg("a6") void *)="\tjsr\t-48(a6)";
#define XpkUnpack(tags) __XpkUnpack((tags), XpkBase)

LONG __XpkOpen(__reg("a0") struct XpkFib ** xbuf, __reg("a1") struct TagItem * tags, __reg("a6") void *)="\tjsr\t-54(a6)";
#define XpkOpen(xbuf, tags) __XpkOpen((xbuf), (tags), XpkBase)

LONG __XpkRead(__reg("a0") struct XpkFib * xbuf, __reg("a1") STRPTR buf, __reg("d0") ULONG len, __reg("a6") void *)="\tjsr\t-60(a6)";
#define XpkRead(xbuf, buf, len) __XpkRead((xbuf), (buf), (len), XpkBase)

LONG __XpkWrite(__reg("a0") struct XpkFib * xbuf, __reg("a1") STRPTR buf, __reg("d0") LONG len, __reg("a6") void *)="\tjsr\t-66(a6)";
#define XpkWrite(xbuf, buf, len) __XpkWrite((xbuf), (buf), (len), XpkBase)

LONG __XpkSeek(__reg("a0") struct XpkFib * xbuf, __reg("d0") LONG len, __reg("d1") LONG mode, __reg("a6") void *)="\tjsr\t-72(a6)";
#define XpkSeek(xbuf, len, mode) __XpkSeek((xbuf), (len), (mode), XpkBase)

LONG __XpkClose(__reg("a0") struct XpkFib * xbuf, __reg("a6") void *)="\tjsr\t-78(a6)";
#define XpkClose(xbuf) __XpkClose((xbuf), XpkBase)

LONG __XpkQuery(__reg("a0") struct TagItem * tags, __reg("a6") void *)="\tjsr\t-84(a6)";
#define XpkQuery(tags) __XpkQuery((tags), XpkBase)

APTR __XpkAllocObject(__reg("d0") ULONG type, __reg("a0") struct TagItem * tags, __reg("a6") void *)="\tjsr\t-90(a6)";
#define XpkAllocObject(type, tags) __XpkAllocObject((type), (tags), XpkBase)

void __XpkFreeObject(__reg("d0") ULONG type, __reg("a0") APTR object, __reg("a6") void *)="\tjsr\t-96(a6)";
#define XpkFreeObject(type, object) __XpkFreeObject((type), (object), XpkBase)

BOOL __XpkPrintFault(__reg("d0") LONG code, __reg("a0") STRPTR header, __reg("a6") void *)="\tjsr\t-102(a6)";
#define XpkPrintFault(code, header) __XpkPrintFault((code), (header), XpkBase)

ULONG __XpkFault(__reg("d0") LONG code, __reg("a0") STRPTR header, __reg("a1") STRPTR buffer, __reg("d1") ULONG size, __reg("a6") void *)="\tjsr\t-108(a6)";
#define XpkFault(code, header, buffer, size) __XpkFault((code), (header), (buffer), (size), XpkBase)

LONG __XpkPassRequest(__reg("a0") struct TagItem * tags, __reg("a6") void *)="\tjsr\t-114(a6)";
#define XpkPassRequest(tags) __XpkPassRequest((tags), XpkBase)

#endif /*  _VBCCINLINE_XPKMASTER_H  */
