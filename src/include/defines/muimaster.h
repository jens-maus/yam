/* Automatically generated header (sfdc 1.4)! Do not edit! */

#ifndef _INLINE_MUIMASTER_H
#define _INLINE_MUIMASTER_H

#ifndef __AROS__
    #warning "this include file is for AROS only"
#endif

#ifndef _SFDC_VARARG_DEFINED
#define _SFDC_VARARG_DEFINED
#ifdef __HAVE_IPTR_ATTR__
typedef APTR _sfdc_vararg __attribute__((iptr));
#else
typedef ULONG _sfdc_vararg;
#endif /* __HAVE_IPTR_ATTR__ */
#endif /* _SFDC_VARARG_DEFINED */

#ifndef AROS_LIBCALL_H
#include <aros/libcall.h>
#endif /* !AROS_LIBCALL_H */

#ifndef MUIMASTER_BASE_NAME
#define MUIMASTER_BASE_NAME MUIMasterBase
#endif /* !MUIMASTER_BASE_NAME */

#define MUI_NewObjectA(___par1, ___last) \
	AROS_LC2(Object *, MUI_NewObjectA, \
	AROS_LCA(CONST_STRPTR, (___par1), A0), \
	AROS_LCA(struct TagItem *, (___last), A1), \
	struct Library *, MUIMASTER_BASE_NAME, 5, Muimaster)

#ifndef NO_INLINE_STDARG
#define MUI_NewObject(___par1, ___last, ...) \
	({_sfdc_vararg _tags[] = { ___last, __VA_ARGS__ }; MUI_NewObjectA((___par1), (struct TagItem *) _tags); })
#endif /* !NO_INLINE_STDARG */

#define MUI_DisposeObject(___last) \
	AROS_LC1(VOID, MUI_DisposeObject, \
	AROS_LCA(Object *, (___last), A0), \
	struct Library *, MUIMASTER_BASE_NAME, 6, Muimaster)

#define MUI_RequestA(___par1, ___par2, ___par3, ___par4, ___par5, ___par6, ___last) \
	AROS_LC7(LONG, MUI_RequestA, \
	AROS_LCA(APTR, (___par1), D0), \
	AROS_LCA(APTR, (___par2), D1), \
	AROS_LCA(LONGBITS, (___par3), D2), \
	AROS_LCA(CONST_STRPTR, (___par4), A0), \
	AROS_LCA(CONST_STRPTR, (___par5), A1), \
	AROS_LCA(CONST_STRPTR, (___par6), A2), \
	AROS_LCA(APTR, (___last), A3), \
	struct Library *, MUIMASTER_BASE_NAME, 7, Muimaster)

#ifndef NO_INLINE_VARARGS
#define MUI_Request(___par1, ___par2, ___par3, ___par4, ___par5, ___par6, ___tag1, ...) \
	({_sfdc_vararg _message[] = { ___tag1, __VA_ARGS__ }; MUI_RequestA((___par1), (___par2), (___par3), (___par4), (___par5), (___par6), (APTR) _message); })
#endif /* !NO_INLINE_VARARGS */

#define MUI_AllocAslRequest(___par1, ___last) \
	AROS_LC2(APTR, MUI_AllocAslRequest, \
	AROS_LCA(unsigned long, (___par1), D0), \
	AROS_LCA(struct TagItem *, (___last), A0), \
	struct Library *, MUIMASTER_BASE_NAME, 8, Muimaster)

#ifndef NO_INLINE_STDARG
#define MUI_AllocAslRequestTags(___par1, ___last, ...) \
	({_sfdc_vararg _tags[] = { ___last, __VA_ARGS__ }; MUI_AllocAslRequest((___par1), (struct TagItem *) _tags); })
#endif /* !NO_INLINE_STDARG */

#define MUI_AslRequest(___par1, ___last) \
	AROS_LC2(BOOL, MUI_AslRequest, \
	AROS_LCA(APTR, (___par1), A0), \
	AROS_LCA(struct TagItem *, (___last), A1), \
	struct Library *, MUIMASTER_BASE_NAME, 9, Muimaster)

#ifndef NO_INLINE_STDARG
#define MUI_AslRequestTags(___par1, ___last, ...) \
	({_sfdc_vararg _tags[] = { ___last, __VA_ARGS__ }; MUI_AslRequest((___par1), (struct TagItem *) _tags); })
#endif /* !NO_INLINE_STDARG */

#define MUI_FreeAslRequest(___last) \
	AROS_LC1(VOID, MUI_FreeAslRequest, \
	AROS_LCA(APTR, (___last), A0), \
	struct Library *, MUIMASTER_BASE_NAME, 10, Muimaster)

#define MUI_Error() \
	AROS_LC0(LONG, MUI_Error, \
	struct Library *, MUIMASTER_BASE_NAME, 11, Muimaster)

#define MUI_SetError(___last) \
	AROS_LC1(LONG, MUI_SetError, \
	AROS_LCA(LONG, (___last), D0), \
	struct Library *, MUIMASTER_BASE_NAME, 12, Muimaster)

#define MUI_GetClass(___last) \
	AROS_LC1(struct IClass *, MUI_GetClass, \
	AROS_LCA(CONST_STRPTR, (___last), A0), \
	struct Library *, MUIMASTER_BASE_NAME, 13, Muimaster)

#define MUI_FreeClass(___last) \
	AROS_LC1(VOID, MUI_FreeClass, \
	AROS_LCA(struct IClass *, (___last), A0), \
	struct Library *, MUIMASTER_BASE_NAME, 14, Muimaster)

#define MUI_RequestIDCMP(___par1, ___last) \
	AROS_LC2(VOID, MUI_RequestIDCMP, \
	AROS_LCA(Object *, (___par1), A0), \
	AROS_LCA(ULONG, (___last), D0), \
	struct Library *, MUIMASTER_BASE_NAME, 15, Muimaster)

#define MUI_RejectIDCMP(___par1, ___last) \
	AROS_LC2(VOID, MUI_RejectIDCMP, \
	AROS_LCA(Object *, (___par1), A0), \
	AROS_LCA(ULONG, (___last), D0), \
	struct Library *, MUIMASTER_BASE_NAME, 16, Muimaster)

#define MUI_Redraw(___par1, ___last) \
	AROS_LC2(VOID, MUI_Redraw, \
	AROS_LCA(Object *, (___par1), A0), \
	AROS_LCA(ULONG, (___last), D0), \
	struct Library *, MUIMASTER_BASE_NAME, 17, Muimaster)

#define MUI_CreateCustomClass(___par1, ___par2, ___par3, ___par4, ___last) \
	AROS_LC5(struct MUI_CustomClass *, MUI_CreateCustomClass, \
	AROS_LCA(struct Library *, (___par1), A0), \
	AROS_LCA(CONST_STRPTR, (___par2), A1), \
	AROS_LCA(struct MUI_CustomClass *, (___par3), A2), \
	AROS_LCA(int, (___par4), D0), \
	AROS_LCA(APTR, (___last), A3), \
	struct Library *, MUIMASTER_BASE_NAME, 18, Muimaster)

#define MUI_DeleteCustomClass(___last) \
	AROS_LC1(BOOL, MUI_DeleteCustomClass, \
	AROS_LCA(struct MUI_CustomClass *, (___last), A0), \
	struct Library *, MUIMASTER_BASE_NAME, 19, Muimaster)

#define MUI_MakeObjectA(___par1, ___last) \
	AROS_LC2(Object *, MUI_MakeObjectA, \
	AROS_LCA(LONG, (___par1), D0), \
	AROS_LCA(ULONG *, (___last), A0), \
	struct Library *, MUIMASTER_BASE_NAME, 20, Muimaster)

#ifndef NO_INLINE_VARARGS
#define MUI_MakeObject(___par1, ___tag1, ...) \
	({_sfdc_vararg _message[] = { ___tag1, __VA_ARGS__ }; MUI_MakeObjectA((___par1), (ULONG *) _message); })
#endif /* !NO_INLINE_VARARGS */

#define MUI_Layout(___par1, ___par2, ___par3, ___par4, ___par5, ___last) \
	AROS_LC6(BOOL, MUI_Layout, \
	AROS_LCA(Object *, (___par1), A0), \
	AROS_LCA(LONG, (___par2), D0), \
	AROS_LCA(LONG, (___par3), D1), \
	AROS_LCA(LONG, (___par4), D2), \
	AROS_LCA(LONG, (___par5), D3), \
	AROS_LCA(ULONG, (___last), D4), \
	struct Library *, MUIMASTER_BASE_NAME, 21, Muimaster)

#define MUI_ObtainPen(___par1, ___par2, ___last) \
	AROS_LC3(LONG, MUI_ObtainPen, \
	AROS_LCA(struct MUI_RenderInfo *, (___par1), A0), \
	AROS_LCA(struct MUI_PenSpec *, (___par2), A1), \
	AROS_LCA(ULONG, (___last), D0), \
	struct Library *, MUIMASTER_BASE_NAME, 22, Muimaster)

#define MUI_ReleasePen(___par1, ___last) \
	AROS_LC2(VOID, MUI_ReleasePen, \
	AROS_LCA(struct MUI_RenderInfo *, (___par1), A0), \
	AROS_LCA(LONG, (___last), D0), \
	struct Library *, MUIMASTER_BASE_NAME, 23, Muimaster)

#define MUI_AddClipping(___par1, ___par2, ___par3, ___par4, ___last) \
	AROS_LC5(APTR, MUI_AddClipping, \
	AROS_LCA(struct MUI_RenderInfo *, (___par1), A0), \
	AROS_LCA(WORD, (___par2), D0), \
	AROS_LCA(WORD, (___par3), D1), \
	AROS_LCA(WORD, (___par4), D2), \
	AROS_LCA(WORD, (___last), D3), \
	struct Library *, MUIMASTER_BASE_NAME, 24, Muimaster)

#define MUI_RemoveClipping(___par1, ___last) \
	AROS_LC2(VOID, MUI_RemoveClipping, \
	AROS_LCA(struct MUI_RenderInfo *, (___par1), A0), \
	AROS_LCA(APTR, (___last), A1), \
	struct Library *, MUIMASTER_BASE_NAME, 25, Muimaster)

#define MUI_AddClipRegion(___par1, ___last) \
	AROS_LC2(APTR, MUI_AddClipRegion, \
	AROS_LCA(struct MUI_RenderInfo *, (___par1), A0), \
	AROS_LCA(struct Region *, (___last), A1), \
	struct Library *, MUIMASTER_BASE_NAME, 26, Muimaster)

#define MUI_RemoveClipRegion(___par1, ___last) \
	AROS_LC2(VOID, MUI_RemoveClipRegion, \
	AROS_LCA(struct MUI_RenderInfo *, (___par1), A0), \
	AROS_LCA(APTR, (___last), A1), \
	struct Library *, MUIMASTER_BASE_NAME, 27, Muimaster)

#define MUI_BeginRefresh(___par1, ___last) \
	AROS_LC2(BOOL, MUI_BeginRefresh, \
	AROS_LCA(struct MUI_RenderInfo *, (___par1), A0), \
	AROS_LCA(ULONG, (___last), D0), \
	struct Library *, MUIMASTER_BASE_NAME, 28, Muimaster)

#define MUI_EndRefresh(___par1, ___last) \
	AROS_LC2(VOID, MUI_EndRefresh, \
	AROS_LCA(struct MUI_RenderInfo *, (___par1), A0), \
	AROS_LCA(ULONG, (___last), D0), \
	struct Library *, MUIMASTER_BASE_NAME, 29, Muimaster)

#endif /* !_INLINE_MUIMASTER_H */
