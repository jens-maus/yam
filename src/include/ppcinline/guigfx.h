/* Automatically generated header! Do not edit! */

#ifndef _PPCINLINE_GUIGFX_H
#define _PPCINLINE_GUIGFX_H

#ifndef __PPCINLINE_MACROS_H
#include <ppcinline/macros.h>
#endif /* !__PPCINLINE_MACROS_H */

#ifndef GUIGFX_BASE_NAME
#define GUIGFX_BASE_NAME GuiGFXBase
#endif /* !GUIGFX_BASE_NAME */

#define DeletePenShareMap(__p0) \
	LP1NR(96, DeletePenShareMap, \
		APTR , __p0, a0, \
		, GUIGFX_BASE_NAME, 0, 0, 0, 0, 0, 0)

#define CreatePictureBitMapA(__p0, __p1, __p2) \
	LP3(132, struct BitMap *, CreatePictureBitMapA, \
		APTR , __p0, a0, \
		APTR , __p1, a1, \
		struct TagItem *, __p2, a2, \
		, GUIGFX_BASE_NAME, 0, 0, 0, 0, 0, 0)

#define CreatePictureMaskA(__p0, __p1, __p2, __p3) \
	LP4(186, BOOL , CreatePictureMaskA, \
		APTR , __p0, a0, \
		UBYTE *, __p1, a1, \
		UWORD , __p2, d0, \
		struct TagItem *, __p3, a2, \
		, GUIGFX_BASE_NAME, 0, 0, 0, 0, 0, 0)

#define ClonePictureA(__p0, __p1) \
	LP2(48, APTR , ClonePictureA, \
		APTR , __p0, a0, \
		struct TagItem *, __p1, a1, \
		, GUIGFX_BASE_NAME, 0, 0, 0, 0, 0, 0)

#define DeletePicture(__p0) \
	LP1NR(54, DeletePicture, \
		APTR , __p0, a0, \
		, GUIGFX_BASE_NAME, 0, 0, 0, 0, 0, 0)

#define UpdatePicture(__p0) \
	LP1NR(60, UpdatePicture, \
		APTR , __p0, a0, \
		, GUIGFX_BASE_NAME, 0, 0, 0, 0, 0, 0)

#define ReadPictureA(__p0, __p1, __p2, __p3, __p4, __p5, __p6) \
	LP7(42, APTR , ReadPictureA, \
		struct RastPort *, __p0, a0, \
		struct ColorMap *, __p1, a1, \
		UWORD , __p2, d0, \
		UWORD , __p3, d1, \
		UWORD , __p4, d2, \
		UWORD , __p5, d3, \
		struct TagItem *, __p6, a2, \
		, GUIGFX_BASE_NAME, 0, 0, 0, 0, 0, 0)

#define CreateDirectDrawHandleA(__p0, __p1, __p2, __p3, __p4, __p5) \
	LP6(168, APTR , CreateDirectDrawHandleA, \
		APTR , __p0, a0, \
		UWORD , __p1, d0, \
		UWORD , __p2, d1, \
		UWORD , __p3, d2, \
		UWORD , __p4, d3, \
		struct TagItem *, __p5, a1, \
		, GUIGFX_BASE_NAME, 0, 0, 0, 0, 0, 0)

#define DrawPictureA(__p0, __p1, __p2, __p3, __p4) \
	LP5(114, BOOL , DrawPictureA, \
		APTR , __p0, a0, \
		APTR , __p1, a1, \
		UWORD , __p2, d0, \
		UWORD , __p3, d1, \
		struct TagItem *, __p4, a2, \
		, GUIGFX_BASE_NAME, 0, 0, 0, 0, 0, 0)

#define MapPaletteA(__p0, __p1, __p2, __p3) \
	LP4(120, BOOL , MapPaletteA, \
		APTR , __p0, a0, \
		APTR , __p1, a1, \
		UBYTE *, __p2, a2, \
		struct TagItem *, __p3, a3, \
		, GUIGFX_BASE_NAME, 0, 0, 0, 0, 0, 0)

#define AddPictureA(__p0, __p1, __p2) \
	LP3(66, APTR , AddPictureA, \
		APTR , __p0, a0, \
		APTR , __p1, a1, \
		struct TagItem *, __p2, a2, \
		, GUIGFX_BASE_NAME, 0, 0, 0, 0, 0, 0)

#define UnLockPicture(__p0, __p1) \
	LP2NR(156, UnLockPicture, \
		APTR , __p0, a0, \
		ULONG , __p1, d0, \
		, GUIGFX_BASE_NAME, 0, 0, 0, 0, 0, 0)

#define DirectDrawTrueColorA(__p0, __p1, __p2, __p3, __p4) \
	LP5(180, BOOL , DirectDrawTrueColorA, \
		APTR , __p0, a0, \
		ULONG *, __p1, a1, \
		UWORD , __p2, d0, \
		UWORD , __p3, d1, \
		struct TagItem *, __p4, a2, \
		, GUIGFX_BASE_NAME, 0, 0, 0, 0, 0, 0)

#define RemColorHandle(__p0) \
	LP1NR(84, RemColorHandle, \
		APTR , __p0, a0, \
		, GUIGFX_BASE_NAME, 0, 0, 0, 0, 0, 0)

#define IsPictureA(__p0, __p1) \
	LP2(162, BOOL , IsPictureA, \
		char *, __p0, a0, \
		struct TagItem *, __p1, a1, \
		, GUIGFX_BASE_NAME, 0, 0, 0, 0, 0, 0)

#define LoadPictureA(__p0, __p1) \
	LP2(36, APTR , LoadPictureA, \
		STRPTR , __p0, a0, \
		struct TagItem *, __p1, a1, \
		, GUIGFX_BASE_NAME, 0, 0, 0, 0, 0, 0)

#define AddPixelArrayA(__p0, __p1, __p2, __p3, __p4) \
	LP5(78, APTR , AddPixelArrayA, \
		APTR , __p0, a0, \
		APTR , __p1, a1, \
		UWORD , __p2, d0, \
		UWORD , __p3, d1, \
		struct TagItem *, __p4, a2, \
		, GUIGFX_BASE_NAME, 0, 0, 0, 0, 0, 0)

#define DoPictureMethodA(__p0, __p1, __p2) \
	LP3(138, ULONG , DoPictureMethodA, \
		APTR , __p0, a0, \
		ULONG , __p1, d0, \
		ULONG *, __p2, a1, \
		, GUIGFX_BASE_NAME, 0, 0, 0, 0, 0, 0)

#define ObtainDrawHandleA(__p0, __p1, __p2, __p3) \
	LP4(102, APTR , ObtainDrawHandleA, \
		APTR , __p0, a0, \
		struct RastPort *, __p1, a1, \
		struct ColorMap *, __p2, a2, \
		struct TagItem *, __p3, a3, \
		, GUIGFX_BASE_NAME, 0, 0, 0, 0, 0, 0)

#define AddPaletteA(__p0, __p1, __p2) \
	LP3(72, APTR , AddPaletteA, \
		APTR , __p0, a0, \
		APTR , __p1, a1, \
		struct TagItem *, __p2, a2, \
		, GUIGFX_BASE_NAME, 0, 0, 0, 0, 0, 0)

#define MakePictureA(__p0, __p1, __p2, __p3) \
	LP4(30, APTR , MakePictureA, \
		APTR , __p0, a0, \
		UWORD , __p1, d0, \
		UWORD , __p2, d1, \
		struct TagItem *, __p3, a1, \
		, GUIGFX_BASE_NAME, 0, 0, 0, 0, 0, 0)

#define DeleteDirectDrawHandle(__p0) \
	LP1NR(174, DeleteDirectDrawHandle, \
		APTR , __p0, a0, \
		, GUIGFX_BASE_NAME, 0, 0, 0, 0, 0, 0)

#define GetPictureAttrsA(__p0, __p1) \
	LP2(144, ULONG , GetPictureAttrsA, \
		APTR , __p0, a0, \
		struct TagItem *, __p1, a1, \
		, GUIGFX_BASE_NAME, 0, 0, 0, 0, 0, 0)

#define ReleaseDrawHandle(__p0) \
	LP1NR(108, ReleaseDrawHandle, \
		APTR , __p0, a0, \
		, GUIGFX_BASE_NAME, 0, 0, 0, 0, 0, 0)

#define MapPenA(__p0, __p1, __p2) \
	LP3(126, LONG , MapPenA, \
		APTR , __p0, a0, \
		ULONG , __p1, a1, \
		struct TagItem *, __p2, a2, \
		, GUIGFX_BASE_NAME, 0, 0, 0, 0, 0, 0)

#define LockPictureA(__p0, __p1, __p2) \
	LP3(150, ULONG , LockPictureA, \
		APTR , __p0, a0, \
		ULONG , __p1, d0, \
		ULONG *, __p2, a1, \
		, GUIGFX_BASE_NAME, 0, 0, 0, 0, 0, 0)

#define CreatePenShareMapA(__p0) \
	LP1(90, APTR , CreatePenShareMapA, \
		struct TagItem *, __p0, a0, \
		, GUIGFX_BASE_NAME, 0, 0, 0, 0, 0, 0)

#if defined(USE_INLINE_STDARG) && !defined(__STRICT_ANSI__)

#include <stdarg.h>

#define LockPicture(__p0, __p1, ...) \
	({ULONG _tags[] = { __VA_ARGS__ }; \
	LockPictureA(__p0, __p1, (ULONG *)_tags);})

#define CreateDirectDrawHandle(__p0, __p1, __p2, __p3, __p4, ...) \
	({ULONG _tags[] = { __VA_ARGS__ }; \
	CreateDirectDrawHandleA(__p0, __p1, __p2, __p3, __p4, (struct TagItem *)_tags);})

#define ReadPicture(__p0, __p1, __p2, __p3, __p4, __p5, ...) \
	({ULONG _tags[] = { __VA_ARGS__ }; \
	ReadPictureA(__p0, __p1, __p2, __p3, __p4, __p5, (struct TagItem *)_tags);})

#define AddPixelArray(__p0, __p1, __p2, __p3, ...) \
	({ULONG _tags[] = { __VA_ARGS__ }; \
	AddPixelArrayA(__p0, __p1, __p2, __p3, (struct TagItem *)_tags);})

#define CreatePictureBitMap(__p0, __p1, ...) \
	({ULONG _tags[] = { __VA_ARGS__ }; \
	CreatePictureBitMapA(__p0, __p1, (struct TagItem *)_tags);})

#define MakePicture(__p0, __p1, __p2, ...) \
	({ULONG _tags[] = { __VA_ARGS__ }; \
	MakePictureA(__p0, __p1, __p2, (struct TagItem *)_tags);})

#define DirectDrawTrueColor(__p0, __p1, __p2, __p3, ...) \
	({ULONG _tags[] = { __VA_ARGS__ }; \
	DirectDrawTrueColorA(__p0, __p1, __p2, __p3, (struct TagItem *)_tags);})

#define GetPictureAttrs(__p0, ...) \
	({ULONG _tags[] = { __VA_ARGS__ }; \
	GetPictureAttrsA(__p0, (struct TagItem *)_tags);})

#define ClonePicture(__p0, ...) \
	({ULONG _tags[] = { __VA_ARGS__ }; \
	ClonePictureA(__p0, (struct TagItem *)_tags);})

#define IsPicture(__p0, ...) \
	({ULONG _tags[] = { __VA_ARGS__ }; \
	IsPictureA(__p0, (struct TagItem *)_tags);})

#define CreatePictureMask(__p0, __p1, __p2, ...) \
	({ULONG _tags[] = { __VA_ARGS__ }; \
	CreatePictureMaskA(__p0, __p1, __p2, (struct TagItem *)_tags);})

#define DoPictureMethod(__p0, __p1, ...) \
	({ULONG _tags[] = { __VA_ARGS__ }; \
	DoPictureMethodA(__p0, __p1, (ULONG *)_tags);})

#define AddPalette(__p0, __p1, ...) \
	({ULONG _tags[] = { __VA_ARGS__ }; \
	AddPaletteA(__p0, __p1, (struct TagItem *)_tags);})

#define MapPen(__p0, __p1, ...) \
	({ULONG _tags[] = { __VA_ARGS__ }; \
	MapPenA(__p0, __p1, (struct TagItem *)_tags);})

#define MapPalette(__p0, __p1, __p2, ...) \
	({ULONG _tags[] = { __VA_ARGS__ }; \
	MapPaletteA(__p0, __p1, __p2, (struct TagItem *)_tags);})

#define ObtainDrawHandle(__p0, __p1, __p2, ...) \
	({ULONG _tags[] = { __VA_ARGS__ }; \
	ObtainDrawHandleA(__p0, __p1, __p2, (struct TagItem *)_tags);})

#define AddPicture(__p0, __p1, ...) \
	({ULONG _tags[] = { __VA_ARGS__ }; \
	AddPictureA(__p0, __p1, (struct TagItem *)_tags);})

#define CreatePenShareMap(...) \
	({ULONG _tags[] = { __VA_ARGS__ }; \
	CreatePenShareMapA((struct TagItem *)_tags);})

#define LoadPicture(__p0, ...) \
	({ULONG _tags[] = { __VA_ARGS__ }; \
	LoadPictureA(__p0, (struct TagItem *)_tags);})

#define DrawPicture(__p0, __p1, __p2, __p3, ...) \
	({ULONG _tags[] = { __VA_ARGS__ }; \
	DrawPictureA(__p0, __p1, __p2, __p3, (struct TagItem *)_tags);})

#endif

#endif /* !_PPCINLINE_GUIGFX_H */
