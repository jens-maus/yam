#ifndef _PPCINLINE_MUIMASTER_H
#define _PPCINLINE_MUIMASTER_H

#ifndef __PPCINLINE_MACROS_H
#include <ppcinline/macros.h>
#endif

#ifndef MUIMASTER_BASE_NAME
#define MUIMASTER_BASE_NAME MUIMasterBase
#endif

#define MUI_NewObjectA(par1, last) \
	LP2(0x1e, Object *, MUI_NewObjectA, char *, par1, a0, struct TagItem *, last, a1, \
	, MUIMASTER_BASE_NAME, IF_CACHEFLUSHALL, NULL, 0, IF_CACHEFLUSHALL, NULL, 0)

#ifndef NO_PPCINLINE_STDARG
__inline Object * MUI_NewObject(char * par1, Tag last, ...)
{
  return MUI_NewObjectA(par1, (struct TagItem *) &last);
}

#endif

#define MUI_DisposeObject(last) \
	LP1NR(0x24, MUI_DisposeObject, Object *, last, a0, \
	, MUIMASTER_BASE_NAME, IF_CACHEFLUSHALL, NULL, 0, IF_CACHEFLUSHALL, NULL, 0)

#define MUI_RequestA(par1, par2, par3, par4, par5, par6, last) \
	LP7(0x2a, LONG, MUI_RequestA, APTR, par1, d0, APTR, par2, d1, LONGBITS, par3, d2, char *, par4, a0, char *, par5, a1, char *, par6, a2, APTR, last, a3, \
	, MUIMASTER_BASE_NAME, IF_CACHEFLUSHALL, NULL, 0, IF_CACHEFLUSHALL, NULL, 0)

#ifndef NO_PPCINLINE_STDARG
#define MUI_Request(par1, par2, par3, par4, par5, par6, tags...) \
	({ULONG _tags[] = {tags}; MUI_RequestA((par1), (par2), (par3), (par4), (par5), (par6), (APTR) _tags);})
#endif

#define MUI_AllocAslRequest(par1, last) \
	LP2(0x30, APTR, MUI_AllocAslRequest, unsigned long, par1, d0, struct TagItem *, last, a0, \
	, MUIMASTER_BASE_NAME, IF_CACHEFLUSHALL, NULL, 0, IF_CACHEFLUSHALL, NULL, 0)

#ifndef NO_PPCINLINE_STDARG
#define MUI_AllocAslRequestTags(par1, tags...) \
	({ULONG _tags[] = {tags}; MUI_AllocAslRequest((par1), (struct TagItem *) _tags);})
#endif

#define MUI_AslRequest(par1, last) \
	LP2(0x36, BOOL, MUI_AslRequest, APTR, par1, a0, struct TagItem *, last, a1, \
	, MUIMASTER_BASE_NAME, IF_CACHEFLUSHALL, NULL, 0, IF_CACHEFLUSHALL, NULL, 0)

#ifndef NO_PPCINLINE_STDARG
#define MUI_AslRequestTags(par1, tags...) \
	({ULONG _tags[] = {tags}; MUI_AslRequest((par1), (struct TagItem *) _tags);})
#endif

#define MUI_FreeAslRequest(last) \
	LP1NR(0x3c, MUI_FreeAslRequest, APTR, last, a0, \
	, MUIMASTER_BASE_NAME, IF_CACHEFLUSHALL, NULL, 0, IF_CACHEFLUSHALL, NULL, 0)

#define MUI_Error() \
	LP0(0x42, LONG, MUI_Error, \
	, MUIMASTER_BASE_NAME, IF_CACHEFLUSHALL, NULL, 0, IF_CACHEFLUSHALL, NULL, 0)

#define MUI_SetError(last) \
	LP1(0x48, LONG, MUI_SetError, LONG, last, d0, \
	, MUIMASTER_BASE_NAME, IF_CACHEFLUSHALL, NULL, 0, IF_CACHEFLUSHALL, NULL, 0)

#define MUI_GetClass(last) \
	LP1(0x4e, struct IClass *, MUI_GetClass, char *, last, a0, \
	, MUIMASTER_BASE_NAME, IF_CACHEFLUSHALL, NULL, 0, IF_CACHEFLUSHALL, NULL, 0)

#define MUI_FreeClass(last) \
	LP1NR(0x54, MUI_FreeClass, struct IClass *, last, a0, \
	, MUIMASTER_BASE_NAME, IF_CACHEFLUSHALL, NULL, 0, IF_CACHEFLUSHALL, NULL, 0)

#define MUI_RequestIDCMP(par1, last) \
	LP2NR(0x5a, MUI_RequestIDCMP, Object *, par1, a0, ULONG, last, d0, \
	, MUIMASTER_BASE_NAME, IF_CACHEFLUSHALL, NULL, 0, IF_CACHEFLUSHALL, NULL, 0)

#define MUI_RejectIDCMP(par1, last) \
	LP2NR(0x60, MUI_RejectIDCMP, Object *, par1, a0, ULONG, last, d0, \
	, MUIMASTER_BASE_NAME, IF_CACHEFLUSHALL, NULL, 0, IF_CACHEFLUSHALL, NULL, 0)

#define MUI_Redraw(par1, last) \
	LP2NR(0x66, MUI_Redraw, Object *, par1, a0, ULONG, last, d0, \
	, MUIMASTER_BASE_NAME, IF_CACHEFLUSHALL, NULL, 0, IF_CACHEFLUSHALL, NULL, 0)

#define MUI_CreateCustomClass(par1, par2, par3, par4, last) \
	LP5(0x6c, struct MUI_CustomClass *, MUI_CreateCustomClass, struct Library *, par1, a0, char *, par2, a1, struct MUI_CustomClass *, par3, a2, int, par4, d0, APTR, last, a3, \
	, MUIMASTER_BASE_NAME, IF_CACHEFLUSHALL, NULL, 0, IF_CACHEFLUSHALL, NULL, 0)

#define MUI_DeleteCustomClass(last) \
	LP1(0x72, BOOL, MUI_DeleteCustomClass, struct MUI_CustomClass *, last, a0, \
	, MUIMASTER_BASE_NAME, IF_CACHEFLUSHALL, NULL, 0, IF_CACHEFLUSHALL, NULL, 0)

#define MUI_MakeObjectA(par1, last) \
	LP2(0x78, Object *, MUI_MakeObjectA, LONG, par1, d0, ULONG *, last, a0, \
	, MUIMASTER_BASE_NAME, IF_CACHEFLUSHALL, NULL, 0, IF_CACHEFLUSHALL, NULL, 0)

#ifndef NO_PPCINLINE_STDARG
#define MUI_MakeObject(par1, tags...) \
	({ULONG _tags[] = {tags}; MUI_MakeObjectA((par1), (ULONG *) _tags);})
#endif

#define MUI_Layout(par1, par2, par3, par4, par5, last) \
	LP6(0x7e, BOOL, MUI_Layout, Object *, par1, a0, LONG, par2, d0, LONG, par3, d1, LONG, par4, d2, LONG, par5, d3, ULONG, last, d4, \
	, MUIMASTER_BASE_NAME, IF_CACHEFLUSHALL, NULL, 0, IF_CACHEFLUSHALL, NULL, 0)

#define MUI_ObtainPen(par1, par2, last) \
	LP3(0x9c, LONG, MUI_ObtainPen, struct MUI_RenderInfo *, par1, a0, struct MUI_PenSpec *, par2, a1, ULONG, last, d0, \
	, MUIMASTER_BASE_NAME, IF_CACHEFLUSHALL, NULL, 0, IF_CACHEFLUSHALL, NULL, 0)

#define MUI_ReleasePen(par1, last) \
	LP2NR(0xa2, MUI_ReleasePen, struct MUI_RenderInfo *, par1, a0, LONG, last, d0, \
	, MUIMASTER_BASE_NAME, IF_CACHEFLUSHALL, NULL, 0, IF_CACHEFLUSHALL, NULL, 0)

#define MUI_AddClipping(par1, par2, par3, par4, last) \
	LP5(0xa8, APTR, MUI_AddClipping, struct MUI_RenderInfo *, par1, a0, WORD, par2, d0, WORD, par3, d1, WORD, par4, d2, WORD, last, d3, \
	, MUIMASTER_BASE_NAME, IF_CACHEFLUSHALL, NULL, 0, IF_CACHEFLUSHALL, NULL, 0)

#define MUI_RemoveClipping(par1, last) \
	LP2NR(0xae, MUI_RemoveClipping, struct MUI_RenderInfo *, par1, a0, APTR, last, a1, \
	, MUIMASTER_BASE_NAME, IF_CACHEFLUSHALL, NULL, 0, IF_CACHEFLUSHALL, NULL, 0)

#define MUI_AddClipRegion(par1, last) \
	LP2(0xb4, APTR, MUI_AddClipRegion, struct MUI_RenderInfo *, par1, a0, struct Region *, last, a1, \
	, MUIMASTER_BASE_NAME, IF_CACHEFLUSHALL, NULL, 0, IF_CACHEFLUSHALL, NULL, 0)

#define MUI_RemoveClipRegion(par1, last) \
	LP2NR(0xba, MUI_RemoveClipRegion, struct MUI_RenderInfo *, par1, a0, APTR, last, a1, \
	, MUIMASTER_BASE_NAME, IF_CACHEFLUSHALL, NULL, 0, IF_CACHEFLUSHALL, NULL, 0)

#define MUI_BeginRefresh(par1, last) \
	LP2(0xc0, BOOL, MUI_BeginRefresh, struct MUI_RenderInfo *, par1, a0, ULONG, last, d0, \
	, MUIMASTER_BASE_NAME, IF_CACHEFLUSHALL, NULL, 0, IF_CACHEFLUSHALL, NULL, 0)

#define MUI_EndRefresh(par1, last) \
	LP2NR(0xc6, MUI_EndRefresh, struct MUI_RenderInfo *, par1, a0, ULONG, last, d0, \
	, MUIMASTER_BASE_NAME, IF_CACHEFLUSHALL, NULL, 0, IF_CACHEFLUSHALL, NULL, 0)

#endif /*  _PPCINLINE_MUIMASTER_H  */
