/* Automatically generated header! Do not edit! */

#ifndef _INLINE_MUIMASTER_H
#define _INLINE_MUIMASTER_H

#ifndef __INLINE_STUB_H
#include <inline/stubs.h>
#endif /* !__INLINE_STUB_H */

#ifndef BASE_EXT_DECL
#define BASE_EXT_DECL
#define BASE_EXT_DECL0 extern struct Library *MUIMasterBase;
#endif /* !BASE_EXT_DECL */
#ifndef BASE_PAR_DECL
#define BASE_PAR_DECL
#define BASE_PAR_DECL0 void
#endif /* !BASE_PAR_DECL */
#ifndef BASE_NAME
#define BASE_NAME MUIMasterBase
#endif /* !BASE_NAME */

BASE_EXT_DECL0

extern __inline APTR
MUI_AddClipRegion(BASE_PAR_DECL struct MUI_RenderInfo *mri, struct Region *region)
{
   BASE_EXT_DECL
   register APTR res __asm("d0");
   register struct Library *a6 __asm("a6") = BASE_NAME;
   register struct MUI_RenderInfo *a0 __asm("a0") = mri;
   register struct Region *a1 __asm("a1") = region;
   __asm volatile ("jsr a6@(-0xb4:W)"
   : "=r" (res)
   : "r" (a6), "r" (a0), "r" (a1)
   : "d0", "d1", "a0", "a1", "fp0", "fp1", "cc", "memory");
   return res;
}

extern __inline APTR
MUI_AddClipping(BASE_PAR_DECL struct MUI_RenderInfo *mri, WORD l, WORD t, WORD w, WORD h)
{
   BASE_EXT_DECL
   register APTR res __asm("d0");
   register struct Library *a6 __asm("a6") = BASE_NAME;
   register struct MUI_RenderInfo *a0 __asm("a0") = mri;
   register WORD d0 __asm("d0") = l;
   register WORD d1 __asm("d1") = t;
   register WORD d2 __asm("d2") = w;
   register WORD d3 __asm("d3") = h;
   __asm volatile ("jsr a6@(-0xa8:W)"
   : "=r" (res)
   : "r" (a6), "r" (a0), "r" (d0), "r" (d1), "r" (d2), "r" (d3)
   : "d0", "d1", "a0", "a1", "fp0", "fp1", "cc", "memory");
   return res;
}

extern __inline APTR
MUI_AllocAslRequest(BASE_PAR_DECL unsigned long type, struct TagItem *tags)
{
   BASE_EXT_DECL
   register APTR res __asm("d0");
   register struct Library *a6 __asm("a6") = BASE_NAME;
   register unsigned long d0 __asm("d0") = type;
   register struct TagItem *a0 __asm("a0") = tags;
   __asm volatile ("jsr a6@(-0x30:W)"
   : "=r" (res)
   : "r" (a6), "r" (d0), "r" (a0)
   : "d0", "d1", "a0", "a1", "fp0", "fp1", "cc", "memory");
   return res;
}

#ifndef NO_INLINE_STDARG
#define MUI_AllocAslRequestTags(a0, tags...) \
	({ULONG _tags[] = { tags }; MUI_AllocAslRequest((a0), (struct TagItem *)_tags);})
#endif /* !NO_INLINE_STDARG */

extern __inline BOOL
MUI_AslRequest(BASE_PAR_DECL APTR req, struct TagItem *tags)
{
   BASE_EXT_DECL
   register BOOL res __asm("d0");
   register struct Library *a6 __asm("a6") = BASE_NAME;
   register APTR a0 __asm("a0") = req;
   register struct TagItem *a1 __asm("a1") = tags;
   __asm volatile ("jsr a6@(-0x36:W)"
   : "=r" (res)
   : "r" (a6), "r" (a0), "r" (a1)
   : "d0", "d1", "a0", "a1", "fp0", "fp1", "cc", "memory");
   return res;
}

#ifndef NO_INLINE_STDARG
#define MUI_AslRequestTags(a0, tags...) \
	({ULONG _tags[] = { tags }; MUI_AslRequest((a0), (struct TagItem *)_tags);})
#endif /* !NO_INLINE_STDARG */

extern __inline BOOL
MUI_BeginRefresh(BASE_PAR_DECL struct MUI_RenderInfo *mri, ULONG flags)
{
   BASE_EXT_DECL
   register BOOL res __asm("d0");
   register struct Library *a6 __asm("a6") = BASE_NAME;
   register struct MUI_RenderInfo *a0 __asm("a0") = mri;
   register ULONG d0 __asm("d0") = flags;
   __asm volatile ("jsr a6@(-0xc0:W)"
   : "=r" (res)
   : "r" (a6), "r" (a0), "r" (d0)
   : "d0", "d1", "a0", "a1", "fp0", "fp1", "cc", "memory");
   return res;
}

extern __inline struct MUI_CustomClass *
MUI_CreateCustomClass(BASE_PAR_DECL struct Library *base, char *supername, struct MUI_CustomClass *supermcc, int datasize, APTR dispatcher)
{
   BASE_EXT_DECL
   register struct MUI_CustomClass *res __asm("d0");
   register struct Library *a6 __asm("a6") = BASE_NAME;
   register struct Library *a0 __asm("a0") = base;
   register char *a1 __asm("a1") = supername;
   register struct MUI_CustomClass *a2 __asm("a2") = supermcc;
   register int d0 __asm("d0") = datasize;
   register APTR a3 __asm("a3") = dispatcher;
   __asm volatile ("jsr a6@(-0x6c:W)"
   : "=r" (res)
   : "r" (a6), "r" (a0), "r" (a1), "r" (a2), "r" (d0), "r" (a3)
   : "d0", "d1", "a0", "a1", "fp0", "fp1", "cc", "memory");
   return res;
}

extern __inline BOOL
MUI_DeleteCustomClass(BASE_PAR_DECL struct MUI_CustomClass *mcc)
{
   BASE_EXT_DECL
   register BOOL res __asm("d0");
   register struct Library *a6 __asm("a6") = BASE_NAME;
   register struct MUI_CustomClass *a0 __asm("a0") = mcc;
   __asm volatile ("jsr a6@(-0x72:W)"
   : "=r" (res)
   : "r" (a6), "r" (a0)
   : "d0", "d1", "a0", "a1", "fp0", "fp1", "cc", "memory");
   return res;
}

extern __inline VOID
MUI_DisposeObject(BASE_PAR_DECL Object *obj)
{
   BASE_EXT_DECL
   register struct Library *a6 __asm("a6") = BASE_NAME;
   register Object *a0 __asm("a0") = obj;
   __asm volatile ("jsr a6@(-0x24:W)"
   : /* No Output */
   : "r" (a6), "r" (a0)
   : "d0", "d1", "a0", "a1", "fp0", "fp1", "cc", "memory");
}

extern __inline VOID
MUI_EndRefresh(BASE_PAR_DECL struct MUI_RenderInfo *mri, ULONG flags)
{
   BASE_EXT_DECL
   register struct Library *a6 __asm("a6") = BASE_NAME;
   register struct MUI_RenderInfo *a0 __asm("a0") = mri;
   register ULONG d0 __asm("d0") = flags;
   __asm volatile ("jsr a6@(-0xc6:W)"
   : /* No Output */
   : "r" (a6), "r" (a0), "r" (d0)
   : "d0", "d1", "a0", "a1", "fp0", "fp1", "cc", "memory");
}

extern __inline LONG
MUI_Error(BASE_PAR_DECL0)
{
   BASE_EXT_DECL
   register LONG res __asm("d0");
   register struct Library *a6 __asm("a6") = BASE_NAME;
   __asm volatile ("jsr a6@(-0x42:W)"
   : "=r" (res)
   : "r" (a6)
   : "d0", "d1", "a0", "a1", "fp0", "fp1", "cc", "memory");
   return res;
}

extern __inline VOID
MUI_FreeAslRequest(BASE_PAR_DECL APTR req)
{
   BASE_EXT_DECL
   register struct Library *a6 __asm("a6") = BASE_NAME;
   register APTR a0 __asm("a0") = req;
   __asm volatile ("jsr a6@(-0x3c:W)"
   : /* No Output */
   : "r" (a6), "r" (a0)
   : "d0", "d1", "a0", "a1", "fp0", "fp1", "cc", "memory");
}

extern __inline VOID
MUI_FreeClass(BASE_PAR_DECL struct IClass *cl)
{
   BASE_EXT_DECL
   register struct Library *a6 __asm("a6") = BASE_NAME;
   register struct IClass *a0 __asm("a0") = cl;
   __asm volatile ("jsr a6@(-0x54:W)"
   : /* No Output */
   : "r" (a6), "r" (a0)
   : "d0", "d1", "a0", "a1", "fp0", "fp1", "cc", "memory");
}

extern __inline struct IClass *
MUI_GetClass(BASE_PAR_DECL char *name)
{
   BASE_EXT_DECL
   register struct IClass *res __asm("d0");
   register struct Library *a6 __asm("a6") = BASE_NAME;
   register char *a0 __asm("a0") = name;
   __asm volatile ("jsr a6@(-0x4e:W)"
   : "=r" (res)
   : "r" (a6), "r" (a0)
   : "d0", "d1", "a0", "a1", "fp0", "fp1", "cc", "memory");
   return res;
}

extern __inline BOOL
MUI_Layout(BASE_PAR_DECL Object *obj, LONG l, LONG t, LONG w, LONG h, ULONG flags)
{
   BASE_EXT_DECL
   register BOOL res __asm("d0");
   register struct Library *a6 __asm("a6") = BASE_NAME;
   register Object *a0 __asm("a0") = obj;
   register LONG d0 __asm("d0") = l;
   register LONG d1 __asm("d1") = t;
   register LONG d2 __asm("d2") = w;
   register LONG d3 __asm("d3") = h;
   register ULONG d4 __asm("d4") = flags;
   __asm volatile ("jsr a6@(-0x7e:W)"
   : "=r" (res)
   : "r" (a6), "r" (a0), "r" (d0), "r" (d1), "r" (d2), "r" (d3), "r" (d4)
   : "d0", "d1", "a0", "a1", "fp0", "fp1", "cc", "memory");
   return res;
}

extern __inline Object *
MUI_MakeObjectA(BASE_PAR_DECL LONG type, ULONG *params)
{
   BASE_EXT_DECL
   register Object *res __asm("d0");
   register struct Library *a6 __asm("a6") = BASE_NAME;
   register LONG d0 __asm("d0") = type;
   register ULONG *a0 __asm("a0") = params;
   __asm volatile ("jsr a6@(-0x78:W)"
   : "=r" (res)
   : "r" (a6), "r" (d0), "r" (a0)
   : "d0", "d1", "a0", "a1", "fp0", "fp1", "cc", "memory");
   return res;
}

#ifndef NO_INLINE_STDARG
#define MUI_MakeObject(a0, tags...) \
	({ULONG _tags[] = { tags }; MUI_MakeObjectA((a0), (ULONG *)_tags);})
#endif /* !NO_INLINE_STDARG */

extern __inline Object *
MUI_NewObjectA(BASE_PAR_DECL char *class, struct TagItem *tags)
{
   BASE_EXT_DECL
   register Object *res __asm("d0");
   register struct Library *a6 __asm("a6") = BASE_NAME;
   register char *a0 __asm("a0") = class;
   register struct TagItem *a1 __asm("a1") = tags;
   __asm volatile ("jsr a6@(-0x1e:W)"
   : "=r" (res)
   : "r" (a6), "r" (a0), "r" (a1)
   : "d0", "d1", "a0", "a1", "fp0", "fp1", "cc", "memory");
   return res;
}

#ifndef NO_INLINE_STDARG
#define MUI_NewObject(a0, tags...) \
	({ULONG _tags[] = { tags }; MUI_NewObjectA((a0), (struct TagItem *)_tags);})
#endif /* !NO_INLINE_STDARG */

extern __inline LONG
MUI_ObtainPen(BASE_PAR_DECL struct MUI_RenderInfo *mri, struct MUI_PenSpec *spec, ULONG flags)
{
   BASE_EXT_DECL
   register LONG res __asm("d0");
   register struct Library *a6 __asm("a6") = BASE_NAME;
   register struct MUI_RenderInfo *a0 __asm("a0") = mri;
   register struct MUI_PenSpec *a1 __asm("a1") = spec;
   register ULONG d0 __asm("d0") = flags;
   __asm volatile ("jsr a6@(-0x9c:W)"
   : "=r" (res)
   : "r" (a6), "r" (a0), "r" (a1), "r" (d0)
   : "d0", "d1", "a0", "a1", "fp0", "fp1", "cc", "memory");
   return res;
}

extern __inline VOID
MUI_Redraw(BASE_PAR_DECL Object *obj, ULONG flags)
{
   BASE_EXT_DECL
   register struct Library *a6 __asm("a6") = BASE_NAME;
   register Object *a0 __asm("a0") = obj;
   register ULONG d0 __asm("d0") = flags;
   __asm volatile ("jsr a6@(-0x66:W)"
   : /* No Output */
   : "r" (a6), "r" (a0), "r" (d0)
   : "d0", "d1", "a0", "a1", "fp0", "fp1", "cc", "memory");
}

extern __inline VOID
MUI_RejectIDCMP(BASE_PAR_DECL Object *obj, ULONG flags)
{
   BASE_EXT_DECL
   register struct Library *a6 __asm("a6") = BASE_NAME;
   register Object *a0 __asm("a0") = obj;
   register ULONG d0 __asm("d0") = flags;
   __asm volatile ("jsr a6@(-0x60:W)"
   : /* No Output */
   : "r" (a6), "r" (a0), "r" (d0)
   : "d0", "d1", "a0", "a1", "fp0", "fp1", "cc", "memory");
}

extern __inline VOID
MUI_ReleasePen(BASE_PAR_DECL struct MUI_RenderInfo *mri, LONG pen)
{
   BASE_EXT_DECL
   register struct Library *a6 __asm("a6") = BASE_NAME;
   register struct MUI_RenderInfo *a0 __asm("a0") = mri;
   register LONG d0 __asm("d0") = pen;
   __asm volatile ("jsr a6@(-0xa2:W)"
   : /* No Output */
   : "r" (a6), "r" (a0), "r" (d0)
   : "d0", "d1", "a0", "a1", "fp0", "fp1", "cc", "memory");
}

extern __inline VOID
MUI_RemoveClipRegion(BASE_PAR_DECL struct MUI_RenderInfo *mri, APTR region)
{
   BASE_EXT_DECL
   register struct Library *a6 __asm("a6") = BASE_NAME;
   register struct MUI_RenderInfo *a0 __asm("a0") = mri;
   register APTR a1 __asm("a1") = region;
   __asm volatile ("jsr a6@(-0xba:W)"
   : /* No Output */
   : "r" (a6), "r" (a0), "r" (a1)
   : "d0", "d1", "a0", "a1", "fp0", "fp1", "cc", "memory");
}

extern __inline VOID
MUI_RemoveClipping(BASE_PAR_DECL struct MUI_RenderInfo *mri, APTR h)
{
   BASE_EXT_DECL
   register struct Library *a6 __asm("a6") = BASE_NAME;
   register struct MUI_RenderInfo *a0 __asm("a0") = mri;
   register APTR a1 __asm("a1") = h;
   __asm volatile ("jsr a6@(-0xae:W)"
   : /* No Output */
   : "r" (a6), "r" (a0), "r" (a1)
   : "d0", "d1", "a0", "a1", "fp0", "fp1", "cc", "memory");
}

extern __inline LONG
MUI_RequestA(BASE_PAR_DECL APTR app, APTR win, LONGBITS flags, char *title, char *gadgets, char *format, APTR params)
{
   BASE_EXT_DECL
   register LONG res __asm("d0");
   register struct Library *a6 __asm("a6") = BASE_NAME;
   register APTR d0 __asm("d0") = app;
   register APTR d1 __asm("d1") = win;
   register LONGBITS d2 __asm("d2") = flags;
   register char *a0 __asm("a0") = title;
   register char *a1 __asm("a1") = gadgets;
   register char *a2 __asm("a2") = format;
   register APTR a3 __asm("a3") = params;
   __asm volatile ("jsr a6@(-0x2a:W)"
   : "=r" (res)
   : "r" (a6), "r" (d0), "r" (d1), "r" (d2), "r" (a0), "r" (a1), "r" (a2), "r" (a3)
   : "d0", "d1", "a0", "a1", "fp0", "fp1", "cc", "memory");
   return res;
}

#ifndef NO_INLINE_STDARG
#define MUI_Request(a0, a1, a2, a3, a4, a5, tags...) \
	({ULONG _tags[] = { tags }; MUI_RequestA((a0), (a1), (a2), (a3), (a4), (a5), (APTR)_tags);})
#endif /* !NO_INLINE_STDARG */

extern __inline VOID
MUI_RequestIDCMP(BASE_PAR_DECL Object *obj, ULONG flags)
{
   BASE_EXT_DECL
   register struct Library *a6 __asm("a6") = BASE_NAME;
   register Object *a0 __asm("a0") = obj;
   register ULONG d0 __asm("d0") = flags;
   __asm volatile ("jsr a6@(-0x5a:W)"
   : /* No Output */
   : "r" (a6), "r" (a0), "r" (d0)
   : "d0", "d1", "a0", "a1", "fp0", "fp1", "cc", "memory");
}

extern __inline LONG
MUI_SetError(BASE_PAR_DECL LONG errnum)
{
   BASE_EXT_DECL
   register LONG res __asm("d0");
   register struct Library *a6 __asm("a6") = BASE_NAME;
   register LONG d0 __asm("d0") = errnum;
   __asm volatile ("jsr a6@(-0x48:W)"
   : "=r" (res)
   : "r" (a6), "r" (d0)
   : "d0", "d1", "a0", "a1", "fp0", "fp1", "cc", "memory");
   return res;
}

#undef BASE_EXT_DECL
#undef BASE_EXT_DECL0
#undef BASE_PAR_DECL
#undef BASE_PAR_DECL0
#undef BASE_NAME

#endif /* !_INLINE_MUIMASTER_H */
