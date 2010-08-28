#ifndef DEFINES_MUIMASTER_H
#define DEFINES_MUIMASTER_H

#include <aros/libcall.h>
#include <exec/types.h>
#include <aros/preprocessor/variadic/cast2iptr.hpp>


#define __MUI_NewObjectA_WB(__MUIMasterBase, __arg1, __arg2) \
        AROS_LC2(Object*, MUI_NewObjectA, \
                  AROS_LCA(ClassID,(__arg1),A0), \
                  AROS_LCA(struct TagItem*,(__arg2),A1), \
        struct Library *, (__MUIMasterBase), 5, MUIMaster)

#define MUI_NewObjectA(arg1, arg2) \
    __MUI_NewObjectA_WB(MUIMasterBase, (arg1), (arg2))

#if !defined(NO_INLINE_STDARG) && !defined(MUIMASTER_NO_INLINE_STDARG)
#define MUI_NewObject(arg1, ...) \
({ \
    IPTR __args[] = { AROS_PP_VARIADIC_CAST2IPTR(__VA_ARGS__) }; \
    MUI_NewObjectA((arg1), (struct TagItem*)__args); \
})
#endif /* !NO_INLINE_STDARG */

#define __MUI_DisposeObject_WB(__MUIMasterBase, __arg1) \
        AROS_LC1NR(void, MUI_DisposeObject, \
                  AROS_LCA(Object*,(__arg1),A0), \
        struct Library *, (__MUIMasterBase), 6, MUIMaster)

#define MUI_DisposeObject(arg1) \
    __MUI_DisposeObject_WB(MUIMasterBase, (arg1))

#define __MUI_RequestA_WB(__MUIMasterBase, __arg1, __arg2, __arg3, __arg4, __arg5, __arg6, __arg7) \
        AROS_LC7(LONG, MUI_RequestA, \
                  AROS_LCA(APTR,(__arg1),D0), \
                  AROS_LCA(APTR,(__arg2),D1), \
                  AROS_LCA(LONGBITS,(__arg3),D2), \
                  AROS_LCA(CONST_STRPTR,(__arg4),A0), \
                  AROS_LCA(CONST_STRPTR,(__arg5),A1), \
                  AROS_LCA(CONST_STRPTR,(__arg6),A2), \
                  AROS_LCA(APTR,(__arg7),A3), \
        struct Library *, (__MUIMasterBase), 7, MUIMaster)

#define MUI_RequestA(arg1, arg2, arg3, arg4, arg5, arg6, arg7) \
    __MUI_RequestA_WB(MUIMasterBase, (arg1), (arg2), (arg3), (arg4), (arg5), (arg6), (arg7))

#if !defined(NO_INLINE_STDARG) && !defined(MUIMASTER_NO_INLINE_STDARG)
#define MUI_Request(arg1, arg2, arg3, arg4, arg5, arg6, ...) \
({ \
    IPTR __args[] = { AROS_PP_VARIADIC_CAST2IPTR(__VA_ARGS__) }; \
    MUI_RequestA((arg1), (arg2), (arg3), (arg4), (arg5), (arg6), (APTR)__args); \
})
#endif /* !NO_INLINE_STDARG */

#define __MUI_AllocAslRequest_WB(__MUIMasterBase, __arg1, __arg2) \
        AROS_LC2(APTR, MUI_AllocAslRequest, \
                  AROS_LCA(unsigned long,(__arg1),D0), \
                  AROS_LCA(struct TagItem*,(__arg2),A0), \
        struct Library *, (__MUIMasterBase), 8, MUIMaster)

#define MUI_AllocAslRequest(arg1, arg2) \
    __MUI_AllocAslRequest_WB(MUIMasterBase, (arg1), (arg2))

#if !defined(NO_INLINE_STDARG) && !defined(MUIMASTER_NO_INLINE_STDARG)
#define MUI_AllocAslRequestTags(arg1, ...) \
({ \
    IPTR __args[] = { AROS_PP_VARIADIC_CAST2IPTR(__VA_ARGS__) }; \
    MUI_AllocAslRequest((arg1), (struct TagItem*)__args); \
})
#endif /* !NO_INLINE_STDARG */

#define __MUI_AslRequest_WB(__MUIMasterBase, __arg1, __arg2) \
        AROS_LC2(BOOL, MUI_AslRequest, \
                  AROS_LCA(APTR,(__arg1),A0), \
                  AROS_LCA(struct TagItem*,(__arg2),A1), \
        struct Library *, (__MUIMasterBase), 9, MUIMaster)

#define MUI_AslRequest(arg1, arg2) \
    __MUI_AslRequest_WB(MUIMasterBase, (arg1), (arg2))

#if !defined(NO_INLINE_STDARG) && !defined(MUIMASTER_NO_INLINE_STDARG)
#define MUI_AslRequestTags(arg1, ...) \
({ \
    IPTR __args[] = { AROS_PP_VARIADIC_CAST2IPTR(__VA_ARGS__) }; \
    MUI_AslRequest((arg1), (struct TagItem*)__args); \
})
#endif /* !NO_INLINE_STDARG */

#define __MUI_FreeAslRequest_WB(__MUIMasterBase, __arg1) \
        AROS_LC1NR(void, MUI_FreeAslRequest, \
                  AROS_LCA(APTR,(__arg1),A0), \
        struct Library *, (__MUIMasterBase), 10, MUIMaster)

#define MUI_FreeAslRequest(arg1) \
    __MUI_FreeAslRequest_WB(MUIMasterBase, (arg1))

#define __MUI_Error_WB(__MUIMasterBase) \
        AROS_LC0(LONG, MUI_Error, \
        struct Library *, (__MUIMasterBase), 11, MUIMaster)

#define MUI_Error() \
    __MUI_Error_WB(MUIMasterBase)

#define __MUI_SetError_WB(__MUIMasterBase, __arg1) \
        AROS_LC1(LONG, MUI_SetError, \
                  AROS_LCA(LONG,(__arg1),D0), \
        struct Library *, (__MUIMasterBase), 12, MUIMaster)

#define MUI_SetError(arg1) \
    __MUI_SetError_WB(MUIMasterBase, (arg1))

#define __MUI_GetClass_WB(__MUIMasterBase, __arg1) \
        AROS_LC1(struct IClass*, MUI_GetClass, \
                  AROS_LCA(ClassID,(__arg1),A0), \
        struct Library *, (__MUIMasterBase), 13, MUIMaster)

#define MUI_GetClass(arg1) \
    __MUI_GetClass_WB(MUIMasterBase, (arg1))

#define __MUI_FreeClass_WB(__MUIMasterBase, __arg1) \
        AROS_LC1NR(void, MUI_FreeClass, \
                  AROS_LCA(Class*,(__arg1),A0), \
        struct Library *, (__MUIMasterBase), 14, MUIMaster)

#define MUI_FreeClass(arg1) \
    __MUI_FreeClass_WB(MUIMasterBase, (arg1))

#define __MUI_RequestIDCMP_WB(__MUIMasterBase, __arg1, __arg2) \
        AROS_LC2NR(void, MUI_RequestIDCMP, \
                  AROS_LCA(Object*,(__arg1),A0), \
                  AROS_LCA(ULONG,(__arg2),D0), \
        struct Library *, (__MUIMasterBase), 15, MUIMaster)

#define MUI_RequestIDCMP(arg1, arg2) \
    __MUI_RequestIDCMP_WB(MUIMasterBase, (arg1), (arg2))

#define __MUI_RejectIDCMP_WB(__MUIMasterBase, __arg1, __arg2) \
        AROS_LC2NR(void, MUI_RejectIDCMP, \
                  AROS_LCA(Object*,(__arg1),A0), \
                  AROS_LCA(ULONG,(__arg2),D0), \
        struct Library *, (__MUIMasterBase), 16, MUIMaster)

#define MUI_RejectIDCMP(arg1, arg2) \
    __MUI_RejectIDCMP_WB(MUIMasterBase, (arg1), (arg2))

#define __MUI_Redraw_WB(__MUIMasterBase, __arg1, __arg2) \
        AROS_LC2NR(void, MUI_Redraw, \
                  AROS_LCA(Object*,(__arg1),A0), \
                  AROS_LCA(ULONG,(__arg2),D0), \
        struct Library *, (__MUIMasterBase), 17, MUIMaster)

#define MUI_Redraw(arg1, arg2) \
    __MUI_Redraw_WB(MUIMasterBase, (arg1), (arg2))

#define __MUI_CreateCustomClass_WB(__MUIMasterBase, __arg1, __arg2, __arg3, __arg4, __arg5) \
        AROS_LC5(struct MUI_CustomClass*, MUI_CreateCustomClass, \
                  AROS_LCA(struct Library*,(__arg1),A0), \
                  AROS_LCA(ClassID,(__arg2),A1), \
                  AROS_LCA(struct MUI_CustomClass*,(__arg3),A2), \
                  AROS_LCA(ULONG,(__arg4),D0), \
                  AROS_LCA(APTR,(__arg5),A3), \
        struct Library *, (__MUIMasterBase), 18, MUIMaster)

#define MUI_CreateCustomClass(arg1, arg2, arg3, arg4, arg5) \
    __MUI_CreateCustomClass_WB(MUIMasterBase, (arg1), (arg2), (arg3), (arg4), (arg5))

#define __MUI_DeleteCustomClass_WB(__MUIMasterBase, __arg1) \
        AROS_LC1(BOOL, MUI_DeleteCustomClass, \
                  AROS_LCA(struct MUI_CustomClass*,(__arg1),A0), \
        struct Library *, (__MUIMasterBase), 19, MUIMaster)

#define MUI_DeleteCustomClass(arg1) \
    __MUI_DeleteCustomClass_WB(MUIMasterBase, (arg1))

#define __MUI_MakeObjectA_WB(__MUIMasterBase, __arg1, __arg2) \
        AROS_LC2(Object*, MUI_MakeObjectA, \
                  AROS_LCA(LONG,(__arg1),D0), \
                  AROS_LCA(IPTR*,(__arg2),A0), \
        struct Library *, (__MUIMasterBase), 20, MUIMaster)

#define MUI_MakeObjectA(arg1, arg2) \
    __MUI_MakeObjectA_WB(MUIMasterBase, (arg1), (arg2))

#if !defined(NO_INLINE_STDARG) && !defined(MUIMASTER_NO_INLINE_STDARG)
#define MUI_MakeObject(arg1, ...) \
({ \
    IPTR __args[] = { AROS_PP_VARIADIC_CAST2IPTR(__VA_ARGS__) }; \
    MUI_MakeObjectA((arg1), (IPTR*)__args); \
})
#endif /* !NO_INLINE_STDARG */

#define __MUI_Layout_WB(__MUIMasterBase, __arg1, __arg2, __arg3, __arg4, __arg5, __arg6) \
        AROS_LC6(BOOL, MUI_Layout, \
                  AROS_LCA(Object*,(__arg1),A0), \
                  AROS_LCA(LONG,(__arg2),D0), \
                  AROS_LCA(LONG,(__arg3),D1), \
                  AROS_LCA(LONG,(__arg4),D2), \
                  AROS_LCA(LONG,(__arg5),D3), \
                  AROS_LCA(ULONG,(__arg6),D4), \
        struct Library *, (__MUIMasterBase), 21, MUIMaster)

#define MUI_Layout(arg1, arg2, arg3, arg4, arg5, arg6) \
    __MUI_Layout_WB(MUIMasterBase, (arg1), (arg2), (arg3), (arg4), (arg5), (arg6))

#define __MUI_ObtainPen_WB(__MUIMasterBase, __arg1, __arg2, __arg3) \
        AROS_LC3(LONG, MUI_ObtainPen, \
                  AROS_LCA(struct MUI_RenderInfo*,(__arg1),A0), \
                  AROS_LCA(struct MUI_PenSpec*,(__arg2),A1), \
                  AROS_LCA(ULONG,(__arg3),D0), \
        struct Library *, (__MUIMasterBase), 22, MUIMaster)

#define MUI_ObtainPen(arg1, arg2, arg3) \
    __MUI_ObtainPen_WB(MUIMasterBase, (arg1), (arg2), (arg3))

#define __MUI_ReleasePen_WB(__MUIMasterBase, __arg1, __arg2) \
        AROS_LC2NR(void, MUI_ReleasePen, \
                  AROS_LCA(struct MUI_RenderInfo*,(__arg1),A0), \
                  AROS_LCA(LONG,(__arg2),D0), \
        struct Library *, (__MUIMasterBase), 23, MUIMaster)

#define MUI_ReleasePen(arg1, arg2) \
    __MUI_ReleasePen_WB(MUIMasterBase, (arg1), (arg2))

#define __MUI_AddClipping_WB(__MUIMasterBase, __arg1, __arg2, __arg3, __arg4, __arg5) \
        AROS_LC5(APTR, MUI_AddClipping, \
                  AROS_LCA(struct MUI_RenderInfo*,(__arg1),A0), \
                  AROS_LCA(WORD,(__arg2),D0), \
                  AROS_LCA(WORD,(__arg3),D1), \
                  AROS_LCA(WORD,(__arg4),D2), \
                  AROS_LCA(WORD,(__arg5),D3), \
        struct Library *, (__MUIMasterBase), 24, MUIMaster)

#define MUI_AddClipping(arg1, arg2, arg3, arg4, arg5) \
    __MUI_AddClipping_WB(MUIMasterBase, (arg1), (arg2), (arg3), (arg4), (arg5))

#define __MUI_RemoveClipping_WB(__MUIMasterBase, __arg1, __arg2) \
        AROS_LC2NR(void, MUI_RemoveClipping, \
                  AROS_LCA(struct MUI_RenderInfo*,(__arg1),A0), \
                  AROS_LCA(APTR,(__arg2),A1), \
        struct Library *, (__MUIMasterBase), 25, MUIMaster)

#define MUI_RemoveClipping(arg1, arg2) \
    __MUI_RemoveClipping_WB(MUIMasterBase, (arg1), (arg2))

#define __MUI_AddClipRegion_WB(__MUIMasterBase, __arg1, __arg2) \
        AROS_LC2(APTR, MUI_AddClipRegion, \
                  AROS_LCA(struct MUI_RenderInfo*,(__arg1),A0), \
                  AROS_LCA(struct Region*,(__arg2),A1), \
        struct Library *, (__MUIMasterBase), 26, MUIMaster)

#define MUI_AddClipRegion(arg1, arg2) \
    __MUI_AddClipRegion_WB(MUIMasterBase, (arg1), (arg2))

#define __MUI_RemoveClipRegion_WB(__MUIMasterBase, __arg1, __arg2) \
        AROS_LC2NR(void, MUI_RemoveClipRegion, \
                  AROS_LCA(struct MUI_RenderInfo*,(__arg1),A0), \
                  AROS_LCA(APTR,(__arg2),A1), \
        struct Library *, (__MUIMasterBase), 27, MUIMaster)

#define MUI_RemoveClipRegion(arg1, arg2) \
    __MUI_RemoveClipRegion_WB(MUIMasterBase, (arg1), (arg2))

#define __MUI_BeginRefresh_WB(__MUIMasterBase, __arg1, __arg2) \
        AROS_LC2(BOOL, MUI_BeginRefresh, \
                  AROS_LCA(struct MUI_RenderInfo*,(__arg1),A0), \
                  AROS_LCA(ULONG,(__arg2),D0), \
        struct Library *, (__MUIMasterBase), 28, MUIMaster)

#define MUI_BeginRefresh(arg1, arg2) \
    __MUI_BeginRefresh_WB(MUIMasterBase, (arg1), (arg2))

#define __MUI_EndRefresh_WB(__MUIMasterBase, __arg1, __arg2) \
        AROS_LC2NR(void, MUI_EndRefresh, \
                  AROS_LCA(struct MUI_RenderInfo*,(__arg1),A0), \
                  AROS_LCA(ULONG,(__arg2),D0), \
        struct Library *, (__MUIMasterBase), 29, MUIMaster)

#define MUI_EndRefresh(arg1, arg2) \
    __MUI_EndRefresh_WB(MUIMasterBase, (arg1), (arg2))

#endif /* DEFINES_MUIMASTER_H*/
