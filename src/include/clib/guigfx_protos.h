#ifndef CLIB_GUIGFX_H
#define CLIB_GUIGFX_H
/*
**	$VER: guigfx_protos.h v16.0 (23.5.99)
**
**	C prototype definitions
**
**	© TEK neoscientists
*/

#ifndef  EXEC_TYPES_H
#include <exec/types.h>
#endif


APTR MakePicture(APTR array, UWORD width, UWORD height, ...);
APTR MakePictureA(APTR array, UWORD width, UWORD height, struct TagItem *tags);
APTR LoadPicture(STRPTR filename, ...);
APTR LoadPictureA(STRPTR filename, struct TagItem *tags);
APTR ReadPicture(struct RastPort *rp, struct ColorMap *cm, UWORD x, UWORD y, UWORD width, UWORD height, ...);
APTR ReadPictureA(struct RastPort *rp, struct ColorMap *cm, UWORD x, UWORD y, UWORD width, UWORD height, struct TagItem *tags);
APTR ClonePicture(APTR pic, ...);
APTR ClonePictureA(APTR pic, struct TagItem *tags);

void DeletePicture(APTR pic);

BOOL UpdatePicture(APTR pic);

APTR AddPicture(APTR psm, APTR pic, ...);
APTR AddPictureA(APTR psm, APTR pic, struct TagItem *tags);
APTR AddPalette(APTR psm, APTR palette, ...);
APTR AddPaletteA(APTR psm, APTR palette, struct TagItem *tags);
APTR AddPixelArray(APTR psm, APTR array, UWORD width, UWORD height, ...);
APTR AddPixelArrayA(APTR psm, APTR array, UWORD width, UWORD height, struct TagItem *tags);

void RemColorHandle(APTR colorhandle);

APTR CreatePenShareMap(Tag tag1, ...);
APTR CreatePenShareMapA(struct TagItem *tags);
void DeletePenShareMap(APTR psm);

APTR ObtainDrawHandle(APTR psm, struct RastPort *rp, struct ColorMap *cm, ...);
APTR ObtainDrawHandleA(APTR psm, struct RastPort *rp, struct ColorMap *cm, struct TagItem *tags);

void ReleaseDrawHandle(APTR drawhandle);


BOOL DrawPicture(APTR drawhandle, APTR pic, UWORD x, UWORD y, ...);
BOOL DrawPictureA(APTR drawhandle, APTR pic, UWORD x, UWORD y, struct TagItem *tags);

BOOL MapPalette(APTR drawhandle, APTR palette, UBYTE *pentab, ...);
BOOL MapPaletteA(APTR drawhandle, APTR palette, UBYTE *pentab, struct TagItem *tags);

LONG MapPen(APTR drawhandle, ULONG rgb, ...);
LONG MapPenA(APTR drawhandle, ULONG rgb, struct TagItem *tags);

struct BitMap *CreatePictureBitMap(APTR drawhandle, APTR pic, ...);
struct BitMap *CreatePictureBitMapA(APTR drawhandle, APTR pic, struct TagItem *tags);


ULONG DoPictureMethod(APTR pic, ULONG method, ...);
ULONG DoPictureMethodA(APTR pic, ULONG method, ULONG *arguments);

ULONG GetPictureAttrs(APTR pic, ...);
ULONG GetPictureAttrsA(APTR pic, struct TagItem *tags);

ULONG LockPicture(APTR pic, ULONG mode, ...);
ULONG LockPictureA(APTR pic, ULONG mode, ULONG *arguments);
void UnLockPicture(APTR pic, ULONG mode);

BOOL IsPicture(char *filename, ...);
BOOL IsPictureA(char *filename, struct TagItem *tags);

APTR CreateDirectDrawHandle(APTR drawhandle, UWORD sw, UWORD sh, UWORD dw, UWORD dh, ...);
APTR CreateDirectDrawHandleA(APTR drawhandle, UWORD sw, UWORD sh, UWORD dw, UWORD dh, struct TagItem *tags);
void DeleteDirectDrawHandle(APTR ddh);
BOOL DirectDrawTrueColor(APTR ddh, ULONG *array, UWORD x, UWORD y, ...);
BOOL DirectDrawTrueColorA(APTR ddh, ULONG *array, UWORD x, UWORD y, struct TagItem *tags);

BOOL CreatePictureMask(APTR pic, UBYTE *array, UWORD arraywidth, ...);
BOOL CreatePictureMaskA(APTR pic, UBYTE *array, UWORD arraywidth, struct TagItem *tags);

#endif
