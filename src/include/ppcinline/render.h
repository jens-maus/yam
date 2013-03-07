/* Automatically generated header! Do not edit! */

#ifndef _PPCINLINE_RENDER_H
#define _PPCINLINE_RENDER_H

#ifndef __PPCINLINE_MACROS_H
#include <ppcinline/macros.h>
#endif /* !__PPCINLINE_MACROS_H */

#ifndef RENDER_BASE_NAME
#define RENDER_BASE_NAME RenderBase
#endif /* !RENDER_BASE_NAME */

#define DeleteHistogram(__p0) \
	LP1NR(84, DeleteHistogram, \
		APTR , __p0, a0, \
		, RENDER_BASE_NAME, 0, 0, 0, 0, 0, 0)

#define DeletePalette(__p0) \
	LP1NR(180, DeletePalette, \
		APTR , __p0, a0, \
		, RENDER_BASE_NAME, 0, 0, 0, 0, 0, 0)

#define DeleteMapEngine(__p0) \
	LP1NR(252, DeleteMapEngine, \
		APTR , __p0, a0, \
		, RENDER_BASE_NAME, 0, 0, 0, 0, 0, 0)

#define MapChunkyArrayA(__p0, __p1, __p2, __p3, __p4, __p5, __p6) \
	LP7(276, ULONG , MapChunkyArrayA, \
		APTR , __p0, a0, \
		UBYTE *, __p1, a1, \
		APTR , __p2, a2, \
		UWORD , __p3, d0, \
		UWORD , __p4, d1, \
		UBYTE *, __p5, a3, \
		struct TagItem *, __p6, a4, \
		, RENDER_BASE_NAME, 0, 0, 0, 0, 0, 0)

#define DeleteScaleEngine(__p0) \
	LP1NR(150, DeleteScaleEngine, \
		APTR , __p0, a0, \
		, RENDER_BASE_NAME, 0, 0, 0, 0, 0, 0)

#define AddHistogramA(__p0, __p1, __p2) \
	LP3(222, ULONG , AddHistogramA, \
		APTR , __p0, a0, \
		APTR , __p1, a1, \
		struct TagItem *, __p2, a2, \
		, RENDER_BASE_NAME, 0, 0, 0, 0, 0, 0)

#define ScaleOrdinate(__p0, __p1, __p2) \
	LP3(228, UWORD , ScaleOrdinate, \
		UWORD , __p0, d0, \
		UWORD , __p1, d1, \
		UWORD , __p2, d2, \
		, RENDER_BASE_NAME, 0, 0, 0, 0, 0, 0)

#define FreeRenderMem(__p0, __p1, __p2) \
	LP3NR(60, FreeRenderMem, \
		APTR , __p0, a0, \
		APTR , __p1, a1, \
		ULONG , __p2, d0, \
		, RENDER_BASE_NAME, 0, 0, 0, 0, 0, 0)

#define AllocRenderMem(__p0, __p1) \
	LP2(54, APTR , AllocRenderMem, \
		APTR , __p0, a0, \
		ULONG , __p1, d0, \
		, RENDER_BASE_NAME, 0, 0, 0, 0, 0, 0)

#define CreateRMHandlerA(__p0) \
	LP1(42, APTR , CreateRMHandlerA, \
		struct TagItem *, __p0, a1, \
		, RENDER_BASE_NAME, 0, 0, 0, 0, 0, 0)

#define CreateScaleEngineA(__p0, __p1, __p2, __p3, __p4) \
	LP5(144, APTR , CreateScaleEngineA, \
		UWORD , __p0, d0, \
		UWORD , __p1, d1, \
		UWORD , __p2, d2, \
		UWORD , __p3, d3, \
		struct TagItem *, __p4, a1, \
		, RENDER_BASE_NAME, 0, 0, 0, 0, 0, 0)

#define ChunkyArrayDiversityA(__p0, __p1, __p2, __p3, __p4) \
	LP5(270, LONG , ChunkyArrayDiversityA, \
		UBYTE *, __p0, a0, \
		APTR , __p1, a1, \
		UWORD , __p2, d0, \
		UWORD , __p3, d1, \
		struct TagItem *, __p4, a2, \
		, RENDER_BASE_NAME, 0, 0, 0, 0, 0, 0)

#define RGBArrayDiversityA(__p0, __p1, __p2, __p3) \
	LP4(264, LONG , RGBArrayDiversityA, \
		ULONG *, __p0, a0, \
		UWORD , __p1, d0, \
		UWORD , __p2, d1, \
		struct TagItem *, __p3, a1, \
		, RENDER_BASE_NAME, 0, 0, 0, 0, 0, 0)

#define RenderA(__p0, __p1, __p2, __p3, __p4, __p5) \
	LP6(120, ULONG , RenderA, \
		ULONG *, __p0, a0, \
		UWORD , __p1, d0, \
		UWORD , __p2, d1, \
		UBYTE *, __p3, a1, \
		APTR , __p4, a2, \
		struct TagItem *, __p5, a3, \
		, RENDER_BASE_NAME, 0, 0, 0, 0, 0, 0)

#define AllocRenderVecClear(__p0, __p1) \
	LP2(306, APTR , AllocRenderVecClear, \
		APTR , __p0, a0, \
		ULONG , __p1, d0, \
		, RENDER_BASE_NAME, 0, 0, 0, 0, 0, 0)

#define TurboFillMem(__p0, __p1, __p2) \
	LP3NR(30, TurboFillMem, \
		APTR , __p0, a0, \
		ULONG , __p1, d0, \
		UBYTE , __p2, d1, \
		, RENDER_BASE_NAME, 0, 0, 0, 0, 0, 0)

#define CountHistogram(__p0) \
	LP1(240, ULONG , CountHistogram, \
		APTR , __p0, a0, \
		, RENDER_BASE_NAME, 0, 0, 0, 0, 0, 0)

#define FreeRenderVec(__p0) \
	LP1NR(72, FreeRenderVec, \
		APTR , __p0, a0, \
		, RENDER_BASE_NAME, 0, 0, 0, 0, 0, 0)

#define Chunky2RGBA(__p0, __p1, __p2, __p3, __p4, __p5) \
	LP6(132, ULONG , Chunky2RGBA, \
		UBYTE *, __p0, a0, \
		UWORD , __p1, d0, \
		UWORD , __p2, d1, \
		ULONG *, __p3, a1, \
		APTR , __p4, a2, \
		struct TagItem *, __p5, a3, \
		, RENDER_BASE_NAME, 0, 0, 0, 0, 0, 0)

#define AddRGB(__p0, __p1, __p2) \
	LP3(96, ULONG , AddRGB, \
		APTR , __p0, a0, \
		ULONG , __p1, d0, \
		ULONG , __p2, d1, \
		, RENDER_BASE_NAME, 0, 0, 0, 0, 0, 0)

#define AddRGBImageA(__p0, __p1, __p2, __p3, __p4) \
	LP5(102, ULONG , AddRGBImageA, \
		APTR , __p0, a0, \
		ULONG *, __p1, a1, \
		UWORD , __p2, d0, \
		UWORD , __p3, d1, \
		struct TagItem *, __p4, a2, \
		, RENDER_BASE_NAME, 0, 0, 0, 0, 0, 0)

#define AllocRenderVec(__p0, __p1) \
	LP2(66, APTR , AllocRenderVec, \
		APTR , __p0, a0, \
		ULONG , __p1, d0, \
		, RENDER_BASE_NAME, 0, 0, 0, 0, 0, 0)

#define MixAlphaChannelA(__p0, __p1, __p2, __p3, __p4, __p5) \
	LP6NR(318, MixAlphaChannelA, \
		ULONG *, __p0, a0, \
		ULONG *, __p1, a1, \
		UWORD , __p2, d0, \
		UWORD , __p3, d1, \
		ULONG *, __p4, a2, \
		struct TagItem *, __p5, a3, \
		, RENDER_BASE_NAME, 0, 0, 0, 0, 0, 0)

#define DeleteRMHandler(__p0) \
	LP1NR(48, DeleteRMHandler, \
		APTR , __p0, a0, \
		, RENDER_BASE_NAME, 0, 0, 0, 0, 0, 0)

#define TintRGBArrayA(__p0, __p1, __p2, __p3, __p4, __p5, __p6) \
	LP7NR(324, TintRGBArrayA, \
		ULONG *, __p0, a0, \
		UWORD , __p1, d0, \
		UWORD , __p2, d1, \
		ULONG , __p3, d2, \
		UWORD , __p4, d3, \
		ULONG *, __p5, a1, \
		struct TagItem *, __p6, a2, \
		, RENDER_BASE_NAME, 0, 0, 0, 0, 0, 0)

#define CreateHistogramPointerArray(__p0) \
	LP1(234, ULONG *, CreateHistogramPointerArray, \
		APTR , __p0, a0, \
		, RENDER_BASE_NAME, 0, 0, 0, 0, 0, 0)

#define ScaleA(__p0, __p1, __p2, __p3) \
	LP4(156, ULONG , ScaleA, \
		APTR , __p0, a0, \
		APTR , __p1, a1, \
		APTR , __p2, a2, \
		struct TagItem *, __p3, a3, \
		, RENDER_BASE_NAME, 0, 0, 0, 0, 0, 0)

#define ImportPaletteA(__p0, __p1, __p2, __p3) \
	LP4NR(186, ImportPaletteA, \
		APTR , __p0, a0, \
		APTR , __p1, a1, \
		UWORD , __p2, d0, \
		struct TagItem *, __p3, a2, \
		, RENDER_BASE_NAME, 0, 0, 0, 0, 0, 0)

#define ExportPaletteA(__p0, __p1, __p2) \
	LP3NR(192, ExportPaletteA, \
		APTR , __p0, a0, \
		APTR , __p1, a1, \
		struct TagItem *, __p2, a2, \
		, RENDER_BASE_NAME, 0, 0, 0, 0, 0, 0)

#define CreateMapEngineA(__p0, __p1) \
	LP2(246, APTR , CreateMapEngineA, \
		APTR , __p0, a0, \
		struct TagItem *, __p1, a1, \
		, RENDER_BASE_NAME, 0, 0, 0, 0, 0, 0)

#define FlushPalette(__p0) \
	LP1NR(210, FlushPalette, \
		APTR , __p0, a0, \
		, RENDER_BASE_NAME, 0, 0, 0, 0, 0, 0)

#define ConvertChunkyA(__p0, __p1, __p2, __p3, __p4, __p5, __p6) \
	LP7(162, ULONG , ConvertChunkyA, \
		UBYTE *, __p0, a0, \
		APTR , __p1, a1, \
		UWORD , __p2, d0, \
		UWORD , __p3, d1, \
		UBYTE *, __p4, a2, \
		APTR , __p5, a3, \
		struct TagItem *, __p6, a4, \
		, RENDER_BASE_NAME, 0, 0, 0, 0, 0, 0)

#define SortPaletteA(__p0, __p1, __p2) \
	LP3(216, ULONG , SortPaletteA, \
		APTR , __p0, a0, \
		ULONG , __p1, d0, \
		struct TagItem *, __p2, a1, \
		, RENDER_BASE_NAME, 0, 0, 0, 0, 0, 0)

#define MapRGBArrayA(__p0, __p1, __p2, __p3, __p4, __p5) \
	LP6(258, ULONG , MapRGBArrayA, \
		APTR , __p0, a0, \
		ULONG *, __p1, a1, \
		UWORD , __p2, d0, \
		UWORD , __p3, d1, \
		UBYTE *, __p4, a2, \
		struct TagItem *, __p5, a3, \
		, RENDER_BASE_NAME, 0, 0, 0, 0, 0, 0)

#define CreateAlphaArrayA(__p0, __p1, __p2, __p3) \
	LP4NR(312, CreateAlphaArrayA, \
		ULONG *, __p0, a0, \
		UWORD , __p1, d0, \
		UWORD , __p2, d1, \
		struct TagItem *, __p3, a1, \
		, RENDER_BASE_NAME, 0, 0, 0, 0, 0, 0)

#define Chunky2BitMapA(__p0, __p1, __p2, __p3, __p4, __p5, __p6, __p7, __p8) \
	LP9NR(138, Chunky2BitMapA, \
		UBYTE *, __p0, a0, \
		UWORD , __p1, d0, \
		UWORD , __p2, d1, \
		UWORD , __p3, d2, \
		UWORD , __p4, d3, \
		struct BitMap *, __p5, a1, \
		UWORD , __p6, d4, \
		UWORD , __p7, d5, \
		struct TagItem *, __p8, a2, \
		, RENDER_BASE_NAME, 0, 0, 0, 0, 0, 0)

#define QueryHistogram(__p0, __p1) \
	LP2(90, ULONG , QueryHistogram, \
		APTR , __p0, a0, \
		Tag , __p1, d0, \
		, RENDER_BASE_NAME, 0, 0, 0, 0, 0, 0)

#define CreatePaletteA(__p0) \
	LP1(174, APTR , CreatePaletteA, \
		struct TagItem *, __p0, a1, \
		, RENDER_BASE_NAME, 0, 0, 0, 0, 0, 0)

#define CountRGB(__p0, __p1) \
	LP2(198, ULONG , CountRGB, \
		APTR , __p0, a0, \
		ULONG , __p1, d0, \
		, RENDER_BASE_NAME, 0, 0, 0, 0, 0, 0)

#define TurboCopyMem(__p0, __p1, __p2) \
	LP3NR(36, TurboCopyMem, \
		APTR , __p0, a0, \
		APTR , __p1, a1, \
		ULONG , __p2, d0, \
		, RENDER_BASE_NAME, 0, 0, 0, 0, 0, 0)

#define MixRGBArrayA(__p0, __p1, __p2, __p3, __p4, __p5) \
	LP6NR(300, MixRGBArrayA, \
		ULONG *, __p0, a0, \
		UWORD , __p1, d0, \
		UWORD , __p2, d1, \
		ULONG *, __p3, a1, \
		UWORD , __p4, d2, \
		struct TagItem *, __p5, a2, \
		, RENDER_BASE_NAME, 0, 0, 0, 0, 0, 0)

#define AddChunkyImageA(__p0, __p1, __p2, __p3, __p4, __p5) \
	LP6(108, ULONG , AddChunkyImageA, \
		APTR , __p0, a0, \
		UBYTE *, __p1, a1, \
		UWORD , __p2, d0, \
		UWORD , __p3, d1, \
		APTR , __p4, a2, \
		struct TagItem *, __p5, a3, \
		, RENDER_BASE_NAME, 0, 0, 0, 0, 0, 0)

#define CreatePenTableA(__p0, __p1, __p2, __p3, __p4, __p5, __p6) \
	LP7NR(168, CreatePenTableA, \
		UBYTE *, __p0, a0, \
		APTR , __p1, a1, \
		UWORD , __p2, d0, \
		UWORD , __p3, d1, \
		APTR , __p4, a2, \
		UBYTE *, __p5, a3, \
		struct TagItem *, __p6, a4, \
		, RENDER_BASE_NAME, 0, 0, 0, 0, 0, 0)

#define CreateHistogramA(__p0) \
	LP1(78, APTR , CreateHistogramA, \
		struct TagItem *, __p0, a1, \
		, RENDER_BASE_NAME, 0, 0, 0, 0, 0, 0)

#define Planar2ChunkyA(__p0, __p1, __p2, __p3, __p4, __p5, __p6) \
	LP7NR(126, Planar2ChunkyA, \
		PLANEPTR *, __p0, a0, \
		UWORD , __p1, d0, \
		UWORD , __p2, d1, \
		UWORD , __p3, d2, \
		UWORD , __p4, d3, \
		UBYTE *, __p5, a1, \
		struct TagItem *, __p6, a2, \
		, RENDER_BASE_NAME, 0, 0, 0, 0, 0, 0)

#define ApplyAlphaChannelA(__p0, __p1, __p2, __p3, __p4) \
	LP5NR(294, ApplyAlphaChannelA, \
		ULONG *, __p0, a0, \
		UWORD , __p1, d0, \
		UWORD , __p2, d1, \
		ULONG *, __p3, a1, \
		struct TagItem *, __p4, a2, \
		, RENDER_BASE_NAME, 0, 0, 0, 0, 0, 0)

#define ExtractPaletteA(__p0, __p1, __p2, __p3) \
	LP4(114, ULONG , ExtractPaletteA, \
		APTR , __p0, a0, \
		ULONG *, __p1, a1, \
		UWORD , __p2, d0, \
		struct TagItem *, __p3, a2, \
		, RENDER_BASE_NAME, 0, 0, 0, 0, 0, 0)

#define BestPen(__p0, __p1) \
	LP2(204, LONG , BestPen, \
		APTR , __p0, a0, \
		ULONG , __p1, d0, \
		, RENDER_BASE_NAME, 0, 0, 0, 0, 0, 0)

#define RemapArrayA(__p0, __p1, __p2, __p3, __p4, __p5) \
	LP6NR(336, RemapArrayA, \
		UBYTE *, __p0, a0, \
		UWORD , __p1, d0, \
		UWORD , __p2, d1, \
		UBYTE *, __p3, a1, \
		UBYTE *, __p4, a2, \
		struct TagItem *, __p5, a3, \
		, RENDER_BASE_NAME, 0, 0, 0, 0, 0, 0)

#define InsertAlphaChannelA(__p0, __p1, __p2, __p3, __p4) \
	LP5NR(282, InsertAlphaChannelA, \
		UBYTE *, __p0, a0, \
		UWORD , __p1, d0, \
		UWORD , __p2, d1, \
		ULONG *, __p3, a1, \
		struct TagItem *, __p4, a2, \
		, RENDER_BASE_NAME, 0, 0, 0, 0, 0, 0)

#define ExtractAlphaChannelA(__p0, __p1, __p2, __p3, __p4) \
	LP5NR(288, ExtractAlphaChannelA, \
		ULONG *, __p0, a0, \
		UWORD , __p1, d0, \
		UWORD , __p2, d1, \
		UBYTE *, __p3, a1, \
		struct TagItem *, __p4, a2, \
		, RENDER_BASE_NAME, 0, 0, 0, 0, 0, 0)

#define GetPaletteAttrs(__p0, __p1) \
	LP2(330, ULONG , GetPaletteAttrs, \
		APTR , __p0, a0, \
		ULONG , __p1, d0, \
		, RENDER_BASE_NAME, 0, 0, 0, 0, 0, 0)

#if defined(USE_INLINE_STDARG) && !defined(__STRICT_ANSI__)

#include <stdarg.h>

#define Render(__p0, __p1, __p2, __p3, __p4, ...) \
	({ULONG _tags[] = { __VA_ARGS__ }; \
	RenderA(__p0, __p1, __p2, __p3, __p4, (struct TagItem *)_tags);})

#define ApplyAlphaChannel(__p0, __p1, __p2, __p3, ...) \
	({ULONG _tags[] = { __VA_ARGS__ }; \
	ApplyAlphaChannelA(__p0, __p1, __p2, __p3, (struct TagItem *)_tags);})

#define ImportPalette(__p0, __p1, __p2, ...) \
	({ULONG _tags[] = { __VA_ARGS__ }; \
	ImportPaletteA(__p0, __p1, __p2, (struct TagItem *)_tags);})

#define ConvertChunky(__p0, __p1, __p2, __p3, __p4, __p5, ...) \
	({ULONG _tags[] = { __VA_ARGS__ }; \
	ConvertChunkyA(__p0, __p1, __p2, __p3, __p4, __p5, (struct TagItem *)_tags);})

#define TintRGBArray(__p0, __p1, __p2, __p3, __p4, __p5, ...) \
	({ULONG _tags[] = { __VA_ARGS__ }; \
	TintRGBArrayA(__p0, __p1, __p2, __p3, __p4, __p5, (struct TagItem *)_tags);})

#define CreatePalette(...) \
	({ULONG _tags[] = { __VA_ARGS__ }; \
	CreatePaletteA((struct TagItem *)_tags);})

#define ExportPalette(__p0, __p1, ...) \
	({ULONG _tags[] = { __VA_ARGS__ }; \
	ExportPaletteA(__p0, __p1, (struct TagItem *)_tags);})

#define RemapArray(__p0, __p1, __p2, __p3, __p4, ...) \
	({ULONG _tags[] = { __VA_ARGS__ }; \
	RemapArrayA(__p0, __p1, __p2, __p3, __p4, (struct TagItem *)_tags);})

#define InsertAlphaChannel(__p0, __p1, __p2, __p3, ...) \
	({ULONG _tags[] = { __VA_ARGS__ }; \
	InsertAlphaChannelA(__p0, __p1, __p2, __p3, (struct TagItem *)_tags);})

#define Chunky2RGB(__p0, __p1, __p2, __p3, __p4, ...) \
	({ULONG _tags[] = { __VA_ARGS__ }; \
	Chunky2RGBA(__p0, __p1, __p2, __p3, __p4, (struct TagItem *)_tags);})

#define AddRGBImage(__p0, __p1, __p2, __p3, ...) \
	({ULONG _tags[] = { __VA_ARGS__ }; \
	AddRGBImageA(__p0, __p1, __p2, __p3, (struct TagItem *)_tags);})

#define Scale(__p0, __p1, __p2, ...) \
	({ULONG _tags[] = { __VA_ARGS__ }; \
	ScaleA(__p0, __p1, __p2, (struct TagItem *)_tags);})

#define CreateAlphaArray(__p0, __p1, __p2, ...) \
	({ULONG _tags[] = { __VA_ARGS__ }; \
	CreateAlphaArrayA(__p0, __p1, __p2, (struct TagItem *)_tags);})

#define RGBArrayDiversity(__p0, __p1, __p2, ...) \
	({ULONG _tags[] = { __VA_ARGS__ }; \
	RGBArrayDiversityA(__p0, __p1, __p2, (struct TagItem *)_tags);})

#define Chunky2BitMap(__p0, __p1, __p2, __p3, __p4, __p5, __p6, __p7, ...) \
	({ULONG _tags[] = { __VA_ARGS__ }; \
	Chunky2BitMapA(__p0, __p1, __p2, __p3, __p4, __p5, __p6, __p7, (struct TagItem *)_tags);})

#define CreateHistogram(...) \
	({ULONG _tags[] = { __VA_ARGS__ }; \
	CreateHistogramA((struct TagItem *)_tags);})

#define Planar2Chunky(__p0, __p1, __p2, __p3, __p4, __p5, ...) \
	({ULONG _tags[] = { __VA_ARGS__ }; \
	Planar2ChunkyA(__p0, __p1, __p2, __p3, __p4, __p5, (struct TagItem *)_tags);})

#define CreateMapEngine(__p0, ...) \
	({ULONG _tags[] = { __VA_ARGS__ }; \
	CreateMapEngineA(__p0, (struct TagItem *)_tags);})

#define ExtractPalette(__p0, __p1, __p2, ...) \
	({ULONG _tags[] = { __VA_ARGS__ }; \
	ExtractPaletteA(__p0, __p1, __p2, (struct TagItem *)_tags);})

#define ChunkyArrayDiversity(__p0, __p1, __p2, __p3, ...) \
	({ULONG _tags[] = { __VA_ARGS__ }; \
	ChunkyArrayDiversityA(__p0, __p1, __p2, __p3, (struct TagItem *)_tags);})

#define MapRGBArray(__p0, __p1, __p2, __p3, __p4, ...) \
	({ULONG _tags[] = { __VA_ARGS__ }; \
	MapRGBArrayA(__p0, __p1, __p2, __p3, __p4, (struct TagItem *)_tags);})

#define ExtractAlphaChannel(__p0, __p1, __p2, __p3, ...) \
	({ULONG _tags[] = { __VA_ARGS__ }; \
	ExtractAlphaChannelA(__p0, __p1, __p2, __p3, (struct TagItem *)_tags);})

#define MixRGBArray(__p0, __p1, __p2, __p3, __p4, ...) \
	({ULONG _tags[] = { __VA_ARGS__ }; \
	MixRGBArrayA(__p0, __p1, __p2, __p3, __p4, (struct TagItem *)_tags);})

#define MixAlphaChannel(__p0, __p1, __p2, __p3, __p4, ...) \
	({ULONG _tags[] = { __VA_ARGS__ }; \
	MixAlphaChannelA(__p0, __p1, __p2, __p3, __p4, (struct TagItem *)_tags);})

#define SortPalette(__p0, __p1, ...) \
	({ULONG _tags[] = { __VA_ARGS__ }; \
	SortPaletteA(__p0, __p1, (struct TagItem *)_tags);})

#define CreateScaleEngine(__p0, __p1, __p2, __p3, ...) \
	({ULONG _tags[] = { __VA_ARGS__ }; \
	CreateScaleEngineA(__p0, __p1, __p2, __p3, (struct TagItem *)_tags);})

#define CreatePenTable(__p0, __p1, __p2, __p3, __p4, __p5, ...) \
	({ULONG _tags[] = { __VA_ARGS__ }; \
	CreatePenTableA(__p0, __p1, __p2, __p3, __p4, __p5, (struct TagItem *)_tags);})

#define CreateRMHandler(...) \
	({ULONG _tags[] = { __VA_ARGS__ }; \
	CreateRMHandlerA((struct TagItem *)_tags);})

#define AddChunkyImage(__p0, __p1, __p2, __p3, __p4, ...) \
	({ULONG _tags[] = { __VA_ARGS__ }; \
	AddChunkyImageA(__p0, __p1, __p2, __p3, __p4, (struct TagItem *)_tags);})

#define MapChunkyArray(__p0, __p1, __p2, __p3, __p4, __p5, ...) \
	({ULONG _tags[] = { __VA_ARGS__ }; \
	MapChunkyArrayA(__p0, __p1, __p2, __p3, __p4, __p5, (struct TagItem *)_tags);})

#define AddHistogram(__p0, __p1, ...) \
	({ULONG _tags[] = { __VA_ARGS__ }; \
	AddHistogramA(__p0, __p1, (struct TagItem *)_tags);})

#endif

#endif /* !_PPCINLINE_RENDER_H */
