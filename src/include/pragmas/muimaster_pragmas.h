#ifndef POS /* private */
#ifndef _INCLUDE_PRAGMA_MUIMASTER_LIB_H
#define _INCLUDE_PRAGMA_MUIMASTER_LIB_H

#ifndef CLIB_MUIMASTER_PROTOS_H
#include <clib/muimaster_protos.h>
#endif

#if defined(AZTEC_C) || defined(__MAXON__) || defined(__STORM__)
#pragma amicall(MUIMasterBase,0x01e,MUI_NewObjectA(a0,a1))
#pragma amicall(MUIMasterBase,0x024,MUI_DisposeObject(a0))
#pragma amicall(MUIMasterBase,0x02a,MUI_RequestA(d0,d1,d2,a0,a1,a2,a3))
#pragma amicall(MUIMasterBase,0x030,MUI_AllocAslRequest(d0,a0))
#pragma amicall(MUIMasterBase,0x036,MUI_AslRequest(a0,a1))
#pragma amicall(MUIMasterBase,0x03c,MUI_FreeAslRequest(a0))
#pragma amicall(MUIMasterBase,0x042,MUI_Error())
#pragma amicall(MUIMasterBase,0x048,MUI_SetError(d0))
#pragma amicall(MUIMasterBase,0x04e,MUI_GetClass(a0))
#pragma amicall(MUIMasterBase,0x054,MUI_FreeClass(a0))
#pragma amicall(MUIMasterBase,0x05a,MUI_RequestIDCMP(a0,d0))
#pragma amicall(MUIMasterBase,0x060,MUI_RejectIDCMP(a0,d0))
#pragma amicall(MUIMasterBase,0x066,MUI_Redraw(a0,d0))
#pragma amicall(MUIMasterBase,0x06c,MUI_CreateCustomClass(a0,a1,a2,d0,a3))
#pragma amicall(MUIMasterBase,0x072,MUI_DeleteCustomClass(a0))
#pragma amicall(MUIMasterBase,0x078,MUI_MakeObjectA(d0,a0))
#pragma amicall(MUIMasterBase,0x07e,MUI_Layout(a0,d0,d1,d2,d3,d4))
#pragma amicall(MUIMasterBase,0x09c,MUI_ObtainPen(a0,a1,d0))
#pragma amicall(MUIMasterBase,0x0a2,MUI_ReleasePen(a0,d0))
#pragma amicall(MUIMasterBase,0x0a8,MUI_AddClipping(a0,d0,d1,d2,d3))
#pragma amicall(MUIMasterBase,0x0ae,MUI_RemoveClipping(a0,a1))
#pragma amicall(MUIMasterBase,0x0b4,MUI_AddClipRegion(a0,a1))
#pragma amicall(MUIMasterBase,0x0ba,MUI_RemoveClipRegion(a0,a1))
#pragma amicall(MUIMasterBase,0x0c0,MUI_BeginRefresh(a0,d0))
#pragma amicall(MUIMasterBase,0x0c6,MUI_EndRefresh(a0,d0))
#pragma amicall(MUIMasterBase,0x0d8,MUI_Show(a0))
#pragma amicall(MUIMasterBase,0x0de,MUI_Hide(a0))
#pragma amicall(MUIMasterBase,0x0e4,MUI_LayoutObj(a0,d0,d1,d2,d3,d4))
#pragma amicall(MUIMasterBase,0x0ea,MUI_Offset(a0,d0,d1))
#endif
#if defined(_DCC) || defined(__SASC)
#pragma  libcall MUIMasterBase MUI_NewObjectA         01e 9802
#pragma  libcall MUIMasterBase MUI_DisposeObject      024 801
#pragma  libcall MUIMasterBase MUI_RequestA           02a ba9821007
#pragma  libcall MUIMasterBase MUI_AllocAslRequest    030 8002
#pragma  libcall MUIMasterBase MUI_AslRequest         036 9802
#pragma  libcall MUIMasterBase MUI_FreeAslRequest     03c 801
#pragma  libcall MUIMasterBase MUI_Error              042 00
#pragma  libcall MUIMasterBase MUI_SetError           048 001
#pragma  libcall MUIMasterBase MUI_GetClass           04e 801
#pragma  libcall MUIMasterBase MUI_FreeClass          054 801
#pragma  libcall MUIMasterBase MUI_RequestIDCMP       05a 0802
#pragma  libcall MUIMasterBase MUI_RejectIDCMP        060 0802
#pragma  libcall MUIMasterBase MUI_Redraw             066 0802
#pragma  libcall MUIMasterBase MUI_CreateCustomClass  06c b0a9805
#pragma  libcall MUIMasterBase MUI_DeleteCustomClass  072 801
#pragma  libcall MUIMasterBase MUI_MakeObjectA        078 8002
#pragma  libcall MUIMasterBase MUI_Layout             07e 43210806
#pragma  libcall MUIMasterBase MUI_ObtainPen          09c 09803
#pragma  libcall MUIMasterBase MUI_ReleasePen         0a2 0802
#pragma  libcall MUIMasterBase MUI_AddClipping        0a8 3210805
#pragma  libcall MUIMasterBase MUI_RemoveClipping     0ae 9802
#pragma  libcall MUIMasterBase MUI_AddClipRegion      0b4 9802
#pragma  libcall MUIMasterBase MUI_RemoveClipRegion   0ba 9802
#pragma  libcall MUIMasterBase MUI_BeginRefresh       0c0 0802
#pragma  libcall MUIMasterBase MUI_EndRefresh         0c6 0802
#pragma  libcall MUIMasterBase MUI_Show               0d8 801
#pragma  libcall MUIMasterBase MUI_Hide               0de 801
#pragma  libcall MUIMasterBase MUI_LayoutObj          0e4 43210806
#pragma  libcall MUIMasterBase MUI_Offset             0ea 10803
#endif
#ifdef __STORM__
#pragma tagcall(MUIMasterBase,0x01e,MUI_NewObject(a0,a1))
#pragma tagcall(MUIMasterBase,0x02a,MUI_Request(d0,d1,d2,a0,a1,a2,a3))
#pragma tagcall(MUIMasterBase,0x030,MUI_AllocAslRequestTags(d0,a0))
#pragma tagcall(MUIMasterBase,0x036,MUI_AslRequestTags(a0,a1))
#pragma tagcall(MUIMasterBase,0x078,MUI_MakeObject(d0,a0))
#endif
#ifdef __SASC_60
#pragma  tagcall MUIMasterBase MUI_NewObject          01e 9802
#pragma  tagcall MUIMasterBase MUI_Request            02a ba9821007
#pragma  tagcall MUIMasterBase MUI_AllocAslRequestTags 030 8002
#pragma  tagcall MUIMasterBase MUI_AslRequestTags     036 9802
#pragma  tagcall MUIMasterBase MUI_MakeObject         078 8002
#endif

#endif  /*  _INCLUDE_PRAGMA_MUIMASTER_LIB_H  */
#endif /* private */
