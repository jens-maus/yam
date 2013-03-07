#ifndef _INLINE_RENDER_H
#define _INLINE_RENDER_H

#ifndef __INLINE_MACROS_H
#include <inline/macros.h>
#endif

#ifndef RENDER_BASE_NAME
#define RENDER_BASE_NAME RenderBase
#endif

#define CreateRMHandlerA(taglist) \
	LP1(0x2a, APTR, CreateRMHandlerA, struct TagItem *, taglist, a1, \
	, RENDER_BASE_NAME)

#ifndef NO_INLINE_STDARG
#define CreateRMHandler(tags...) \
	({ULONG _tags[] = {tags}; CreateRMHandlerA((struct TagItem *) _tags);})
#endif

#define DeleteRMHandler(rmh) \
	LP1NR(0x30, DeleteRMHandler, APTR, rmh, a0, \
	, RENDER_BASE_NAME)

#define AllocRenderMem(rendermemhandler, size) \
	LP2(0x36, APTR, AllocRenderMem, APTR, rendermemhandler, a0, ULONG, size, d0, \
	, RENDER_BASE_NAME)

#define FreeRenderMem(rendermemhandler, mem, size) \
	LP3NR(0x3c, FreeRenderMem, APTR, rendermemhandler, a0, APTR, mem, a1, ULONG, size, d0, \
	, RENDER_BASE_NAME)

#define AllocRenderVec(rendermemhandler, size) \
	LP2(0x42, APTR, AllocRenderVec, APTR, rendermemhandler, a0, ULONG, size, d0, \
	, RENDER_BASE_NAME)

#define FreeRenderVec(mem) \
	LP1NR(0x48, FreeRenderVec, APTR, mem, a0, \
	, RENDER_BASE_NAME)

#define CreateHistogramA(taglist) \
	LP1(0x4e, APTR, CreateHistogramA, struct TagItem *, taglist, a1, \
	, RENDER_BASE_NAME)

#ifndef NO_INLINE_STDARG
#define CreateHistogram(tags...) \
	({ULONG _tags[] = {tags}; CreateHistogramA((struct TagItem *) _tags);})
#endif

#define DeleteHistogram(histogram) \
	LP1NR(0x54, DeleteHistogram, APTR, histogram, a0, \
	, RENDER_BASE_NAME)

#define QueryHistogram(histogram, tag) \
	LP2(0x5a, ULONG, QueryHistogram, APTR, histogram, a0, Tag, tag, d0, \
	, RENDER_BASE_NAME)

#define AddRGB(histogram, RGB, count) \
	LP3(0x60, ULONG, AddRGB, APTR, histogram, a0, ULONG, RGB, d0, ULONG, count, d1, \
	, RENDER_BASE_NAME)

#define AddRGBImageA(histogram, rgb, width, height, taglist) \
	LP5(0x66, ULONG, AddRGBImageA, APTR, histogram, a0, ULONG *, rgb, a1, UWORD, width, d0, UWORD, height, d1, struct TagItem *, taglist, a2, \
	, RENDER_BASE_NAME)

#ifndef NO_INLINE_STDARG
#define AddRGBImage(histogram, rgb, width, height, tags...) \
	({ULONG _tags[] = {tags}; AddRGBImageA((histogram), (rgb), (width), (height), (struct TagItem *) _tags);})
#endif

#define AddChunkyImageA(histogram, chunky, width, height, palette, taglist) \
	LP6(0x6c, ULONG, AddChunkyImageA, APTR, histogram, a0, UBYTE *, chunky, a1, UWORD, width, d0, UWORD, height, d1, APTR, palette, a2, struct TagItem *, taglist, a3, \
	, RENDER_BASE_NAME)

#ifndef NO_INLINE_STDARG
#define AddChunkyImage(histogram, chunky, width, height, palette, tags...) \
	({ULONG _tags[] = {tags}; AddChunkyImageA((histogram), (chunky), (width), (height), (palette), (struct TagItem *) _tags);})
#endif

#define ExtractPaletteA(histogram, palette, numcolors, taglist) \
	LP4(0x72, ULONG, ExtractPaletteA, APTR, histogram, a0, ULONG *, palette, a1, UWORD, numcolors, d0, struct TagItem *, taglist, a2, \
	, RENDER_BASE_NAME)

#ifndef NO_INLINE_STDARG
#define ExtractPalette(histogram, palette, numcolors, tags...) \
	({ULONG _tags[] = {tags}; ExtractPaletteA((histogram), (palette), (numcolors), (struct TagItem *) _tags);})
#endif

#define RenderA(rgb, width, height, chunky, palette, taglist) \
	LP6(0x78, ULONG, RenderA, ULONG *, rgb, a0, UWORD, width, d0, UWORD, height, d1, UBYTE *, chunky, a1, APTR, palette, a2, struct TagItem *, taglist, a3, \
	, RENDER_BASE_NAME)

#ifndef NO_INLINE_STDARG
#define Render(rgb, width, height, chunky, palette, tags...) \
	({ULONG _tags[] = {tags}; RenderA((rgb), (width), (height), (chunky), (palette), (struct TagItem *) _tags);})
#endif

#define Planar2ChunkyA(planetab, bytewidth, height, depth, bytesperrow, chunky, taglist) \
	LP7NR(0x7e, Planar2ChunkyA, PLANEPTR *, planetab, a0, UWORD, bytewidth, d0, UWORD, height, d1, UWORD, depth, d2, UWORD, bytesperrow, d3, UBYTE *, chunky, a1, struct TagItem *, taglist, a2, \
	, RENDER_BASE_NAME)

#ifndef NO_INLINE_STDARG
#define Planar2Chunky(planetab, bytewidth, height, depth, bytesperrow, chunky, tags...) \
	({ULONG _tags[] = {tags}; Planar2ChunkyA((planetab), (bytewidth), (height), (depth), (bytesperrow), (chunky), (struct TagItem *) _tags);})
#endif

#define Chunky2RGBA(chunky, width, height, rgb, palette, taglist) \
	LP6(0x84, ULONG, Chunky2RGBA, UBYTE *, chunky, a0, UWORD, width, d0, UWORD, height, d1, ULONG *, rgb, a1, APTR, palette, a2, struct TagItem *, taglist, a3, \
	, RENDER_BASE_NAME)

#ifndef NO_INLINE_STDARG
#define Chunky2RGB(chunky, width, height, rgb, palette, tags...) \
	({ULONG _tags[] = {tags}; Chunky2RGBA((chunky), (width), (height), (rgb), (palette), (struct TagItem *) _tags);})
#endif

#define Chunky2BitMapA(chunky, sx, sy, width, height, bitmap, dx, dy, taglist) \
	LP9NR(0x8a, Chunky2BitMapA, UBYTE *, chunky, a0, UWORD, sx, d0, UWORD, sy, d1, UWORD, width, d2, UWORD, height, d3, struct BitMap *, bitmap, a1, UWORD, dx, d4, UWORD, dy, d5, struct TagItem *, taglist, a2, \
	, RENDER_BASE_NAME)

#ifndef NO_INLINE_STDARG
#define Chunky2BitMap(chunky, sx, sy, width, height, bitmap, dx, dy, tags...) \
	({ULONG _tags[] = {tags}; Chunky2BitMapA((chunky), (sx), (sy), (width), (height), (bitmap), (dx), (dy), (struct TagItem *) _tags);})
#endif

#define CreateScaleEngineA(sourcewidth, sourceheight, destwidth, destheight, taglist) \
	LP5(0x90, APTR, CreateScaleEngineA, UWORD, sourcewidth, d0, UWORD, sourceheight, d1, UWORD, destwidth, d2, UWORD, destheight, d3, struct TagItem *, taglist, a1, \
	, RENDER_BASE_NAME)

#ifndef NO_INLINE_STDARG
#define CreateScaleEngine(sourcewidth, sourceheight, destwidth, destheight, tags...) \
	({ULONG _tags[] = {tags}; CreateScaleEngineA((sourcewidth), (sourceheight), (destwidth), (destheight), (struct TagItem *) _tags);})
#endif

#define DeleteScaleEngine(engine) \
	LP1NR(0x96, DeleteScaleEngine, APTR, engine, a0, \
	, RENDER_BASE_NAME)

#define ScaleA(engine, source, dest, taglist) \
	LP4(0x9c, ULONG, ScaleA, APTR, engine, a0, APTR, source, a1, APTR, dest, a2, struct TagItem *, taglist, a3, \
	, RENDER_BASE_NAME)

#ifndef NO_INLINE_STDARG
#define Scale(engine, source, dest, tags...) \
	({ULONG _tags[] = {tags}; ScaleA((engine), (source), (dest), (struct TagItem *) _tags);})
#endif

#define ConvertChunkyA(source, oldpalette, width, height, dest, newpalette, taglist) \
	LP7A4(0xa2, ULONG, ConvertChunkyA, UBYTE *, source, a0, APTR, oldpalette, a1, UWORD, width, d0, UWORD, height, d1, UBYTE *, dest, a2, APTR, newpalette, a3, struct TagItem *, taglist, d7, \
	, RENDER_BASE_NAME)

#ifndef NO_INLINE_STDARG
#define ConvertChunky(source, oldpalette, width, height, dest, newpalette, tags...) \
	({ULONG _tags[] = {tags}; ConvertChunkyA((source), (oldpalette), (width), (height), (dest), (newpalette), (struct TagItem *) _tags);})
#endif

#define CreatePenTableA(chunky, oldpalette, width, height, newpalette, convtab, taglist) \
	LP7NRA4(0xa8, CreatePenTableA, UBYTE *, chunky, a0, APTR, oldpalette, a1, UWORD, width, d0, UWORD, height, d1, APTR, newpalette, a2, UBYTE *, convtab, a3, struct TagItem *, taglist, d7, \
	, RENDER_BASE_NAME)

#ifndef NO_INLINE_STDARG
#define CreatePenTable(chunky, oldpalette, width, height, newpalette, convtab, tags...) \
	({ULONG _tags[] = {tags}; CreatePenTableA((chunky), (oldpalette), (width), (height), (newpalette), (convtab), (struct TagItem *) _tags);})
#endif

#define CreatePaletteA(taglist) \
	LP1(0xae, APTR, CreatePaletteA, struct TagItem *, taglist, a1, \
	, RENDER_BASE_NAME)

#ifndef NO_INLINE_STDARG
#define CreatePalette(tags...) \
	({ULONG _tags[] = {tags}; CreatePaletteA((struct TagItem *) _tags);})
#endif

#define DeletePalette(palette) \
	LP1NR(0xb4, DeletePalette, APTR, palette, a0, \
	, RENDER_BASE_NAME)

#define ImportPaletteA(palette, coltab, numcols, taglist) \
	LP4NR(0xba, ImportPaletteA, APTR, palette, a0, APTR, coltab, a1, UWORD, numcols, d0, struct TagItem *, taglist, a2, \
	, RENDER_BASE_NAME)

#ifndef NO_INLINE_STDARG
#define ImportPalette(palette, coltab, numcols, tags...) \
	({ULONG _tags[] = {tags}; ImportPaletteA((palette), (coltab), (numcols), (struct TagItem *) _tags);})
#endif

#define ExportPaletteA(palette, coltab, taglist) \
	LP3NR(0xc0, ExportPaletteA, APTR, palette, a0, APTR, coltab, a1, struct TagItem *, taglist, a2, \
	, RENDER_BASE_NAME)

#ifndef NO_INLINE_STDARG
#define ExportPalette(palette, coltab, tags...) \
	({ULONG _tags[] = {tags}; ExportPaletteA((palette), (coltab), (struct TagItem *) _tags);})
#endif

#define CountRGB(histogram, rgb) \
	LP2(0xc6, ULONG, CountRGB, APTR, histogram, a0, ULONG, rgb, d0, \
	, RENDER_BASE_NAME)

#define BestPen(palette, rgb) \
	LP2(0xcc, LONG, BestPen, APTR, palette, a0, ULONG, rgb, d0, \
	, RENDER_BASE_NAME)

#define FlushPalette(palette) \
	LP1NR(0xd2, FlushPalette, APTR, palette, a0, \
	, RENDER_BASE_NAME)

#define SortPaletteA(palette, mode, taglist) \
	LP3(0xd8, ULONG, SortPaletteA, APTR, palette, a0, ULONG, mode, d0, struct TagItem *, taglist, a1, \
	, RENDER_BASE_NAME)

#ifndef NO_INLINE_STDARG
#define SortPalette(palette, mode, tags...) \
	({ULONG _tags[] = {tags}; SortPaletteA((palette), (mode), (struct TagItem *) _tags);})
#endif

#define AddHistogramA(histogram1, histogram2, taglist) \
	LP3(0xde, ULONG, AddHistogramA, APTR, histogram1, a0, APTR, histogram2, a1, struct TagItem *, taglist, a2, \
	, RENDER_BASE_NAME)

#ifndef NO_INLINE_STDARG
#define AddHistogram(histogram1, histogram2, tags...) \
	({ULONG _tags[] = {tags}; AddHistogramA((histogram1), (histogram2), (struct TagItem *) _tags);})
#endif

#define ScaleOrdinate(source, dest, ordinate) \
	LP3(0xe4, UWORD, ScaleOrdinate, UWORD, source, d0, UWORD, dest, d1, UWORD, ordinate, d2, \
	, RENDER_BASE_NAME)

#define CreateMapEngineA(palette, taglist) \
	LP2(0xf6, APTR, CreateMapEngineA, APTR, palette, a0, struct TagItem *, taglist, a1, \
	, RENDER_BASE_NAME)

#ifndef NO_INLINE_STDARG
#define CreateMapEngine(palette, tags...) \
	({ULONG _tags[] = {tags}; CreateMapEngineA((palette), (struct TagItem *) _tags);})
#endif

#define DeleteMapEngine(engine) \
	LP1NR(0xfc, DeleteMapEngine, APTR, engine, a0, \
	, RENDER_BASE_NAME)

#define MapRGBArrayA(engine, rgb, width, height, chunky, taglist) \
	LP6(0x102, ULONG, MapRGBArrayA, APTR, engine, a0, ULONG *, rgb, a1, UWORD, width, d0, UWORD, height, d1, UBYTE *, chunky, a2, struct TagItem *, taglist, a3, \
	, RENDER_BASE_NAME)

#ifndef NO_INLINE_STDARG
#define MapRGBArray(engine, rgb, width, height, chunky, tags...) \
	({ULONG _tags[] = {tags}; MapRGBArrayA((engine), (rgb), (width), (height), (chunky), (struct TagItem *) _tags);})
#endif

#define RGBArrayDiversityA(rgb, width, height, taglist) \
	LP4(0x108, LONG, RGBArrayDiversityA, ULONG *, rgb, a0, UWORD, width, d0, UWORD, height, d1, struct TagItem *, taglist, a1, \
	, RENDER_BASE_NAME)

#ifndef NO_INLINE_STDARG
#define RGBArrayDiversity(rgb, width, height, tags...) \
	({ULONG _tags[] = {tags}; RGBArrayDiversityA((rgb), (width), (height), (struct TagItem *) _tags);})
#endif

#define ChunkyArrayDiversityA(chunky, palette, width, height, taglist) \
	LP5(0x10e, LONG, ChunkyArrayDiversityA, UBYTE *, chunky, a0, APTR, palette, a1, UWORD, width, d0, UWORD, height, d1, struct TagItem *, taglist, a2, \
	, RENDER_BASE_NAME)

#ifndef NO_INLINE_STDARG
#define ChunkyArrayDiversity(chunky, palette, width, height, tags...) \
	({ULONG _tags[] = {tags}; ChunkyArrayDiversityA((chunky), (palette), (width), (height), (struct TagItem *) _tags);})
#endif

#define MapChunkyArrayA(engine, source, palette, width, height, dest, taglist) \
	LP7A4(0x114, ULONG, MapChunkyArrayA, APTR, engine, a0, UBYTE *, source, a1, APTR, palette, a2, UWORD, width, d0, UWORD, height, d1, UBYTE *, dest, a3, struct TagItem *, taglist, d7, \
	, RENDER_BASE_NAME)

#ifndef NO_INLINE_STDARG
#define MapChunkyArray(engine, source, palette, width, height, dest, tags...) \
	({ULONG _tags[] = {tags}; MapChunkyArrayA((engine), (source), (palette), (width), (height), (dest), (struct TagItem *) _tags);})
#endif

#define InsertAlphaChannelA(maskarray, width, height, rgbarray, taglist) \
	LP5NR(0x11a, InsertAlphaChannelA, UBYTE *, maskarray, a0, UWORD, width, d0, UWORD, height, d1, ULONG *, rgbarray, a1, struct TagItem *, taglist, a2, \
	, RENDER_BASE_NAME)

#ifndef NO_INLINE_STDARG
#define InsertAlphaChannel(maskarray, width, height, rgbarray, tags...) \
	({ULONG _tags[] = {tags}; InsertAlphaChannelA((maskarray), (width), (height), (rgbarray), (struct TagItem *) _tags);})
#endif

#define ExtractAlphaChannelA(rgbarray, width, height, chunkyarray, taglist) \
	LP5NR(0x120, ExtractAlphaChannelA, ULONG *, rgbarray, a0, UWORD, width, d0, UWORD, height, d1, UBYTE *, chunkyarray, a1, struct TagItem *, taglist, a2, \
	, RENDER_BASE_NAME)

#ifndef NO_INLINE_STDARG
#define ExtractAlphaChannel(rgbarray, width, height, chunkyarray, tags...) \
	({ULONG _tags[] = {tags}; ExtractAlphaChannelA((rgbarray), (width), (height), (chunkyarray), (struct TagItem *) _tags);})
#endif

#define ApplyAlphaChannelA(sourcearray, width, height, destarray, taglist) \
	LP5NR(0x126, ApplyAlphaChannelA, ULONG *, sourcearray, a0, UWORD, width, d0, UWORD, height, d1, ULONG *, destarray, a1, struct TagItem *, taglist, a2, \
	, RENDER_BASE_NAME)

#ifndef NO_INLINE_STDARG
#define ApplyAlphaChannel(sourcearray, width, height, destarray, tags...) \
	({ULONG _tags[] = {tags}; ApplyAlphaChannelA((sourcearray), (width), (height), (destarray), (struct TagItem *) _tags);})
#endif

#define MixRGBArrayA(sourcearray, width, height, destarray, ratio, taglist) \
	LP6NR(0x12c, MixRGBArrayA, ULONG *, sourcearray, a0, UWORD, width, d0, UWORD, height, d1, ULONG *, destarray, a1, UWORD, ratio, d2, struct TagItem *, taglist, a2, \
	, RENDER_BASE_NAME)

#ifndef NO_INLINE_STDARG
#define MixRGBArray(sourcearray, width, height, destarray, ratio, tags...) \
	({ULONG _tags[] = {tags}; MixRGBArrayA((sourcearray), (width), (height), (destarray), (ratio), (struct TagItem *) _tags);})
#endif

#define AllocRenderVecClear(rendermemhandler, size) \
	LP2(0x132, APTR, AllocRenderVecClear, APTR, rendermemhandler, a0, ULONG, size, d0, \
	, RENDER_BASE_NAME)

#define CreateAlphaArrayA(rgbarray, width, height, taglist) \
	LP4NR(0x138, CreateAlphaArrayA, ULONG *, rgbarray, a0, UWORD, width, d0, UWORD, height, d1, struct TagItem *, taglist, a1, \
	, RENDER_BASE_NAME)

#ifndef NO_INLINE_STDARG
#define CreateAlphaArray(rgbarray, width, height, tags...) \
	({ULONG _tags[] = {tags}; CreateAlphaArrayA((rgbarray), (width), (height), (struct TagItem *) _tags);})
#endif

#define MixAlphaChannelA(source1, source2, width, height, dest, taglist) \
	LP6NR(0x13e, MixAlphaChannelA, ULONG *, source1, a0, ULONG *, source2, a1, UWORD, width, d0, UWORD, height, d1, ULONG *, dest, a2, struct TagItem *, taglist, a3, \
	, RENDER_BASE_NAME)

#ifndef NO_INLINE_STDARG
#define MixAlphaChannel(source1, source2, width, height, dest, tags...) \
	({ULONG _tags[] = {tags}; MixAlphaChannelA((source1), (source2), (width), (height), (dest), (struct TagItem *) _tags);})
#endif

#define TintRGBArrayA(source, width, height, RGB, ratio, dest, taglist) \
	LP7NR(0x144, TintRGBArrayA, ULONG *, source, a0, UWORD, width, d0, UWORD, height, d1, ULONG, RGB, d2, UWORD, ratio, d3, ULONG *, dest, a1, struct TagItem *, taglist, a2, \
	, RENDER_BASE_NAME)

#ifndef NO_INLINE_STDARG
#define TintRGBArray(source, width, height, RGB, ratio, dest, tags...) \
	({ULONG _tags[] = {tags}; TintRGBArrayA((source), (width), (height), (RGB), (ratio), (dest), (struct TagItem *) _tags);})
#endif

#endif /*  _INLINE_RENDER_H  */
