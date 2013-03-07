#ifndef CLIB_RENDER_PROTOS_H
#define CLIB_RENDER_PROTOS_H
/*
**	$VER: render_protos.h v29.1 (19.5.99)
**
**	C prototype definitions
**
**	© TEK neoscientists
*/

#ifndef	GRAPHICS_GFX_H
#include <graphics/gfx.h>
#endif

#ifndef	UTILITY_HOOKS_H
#include <utility/hooks.h>
#endif

#ifndef UTILITY_TAGITEM_H
#include <utility/tagitem.h>
#endif

/*   private functions: */

void	TurboFillMem( APTR,ULONG,UBYTE );
void	TurboCopyMem( APTR,APTR,ULONG );
ULONG 	*CreateHistogramPointerArray( APTR );
ULONG	CountHistogram( APTR );
ULONG	GetPaletteAttrs( APTR, ULONG);
void	RemapArrayA(UBYTE *source, UWORD width, UWORD height, UBYTE *dest, UBYTE *pentab, struct TagItem *);
void	RemapArray(UBYTE *source, UWORD width, UWORD height, UBYTE *dest, UBYTE *pentab, Tag, ... );


/*   public functions:  */

APTR	CreateRMHandlerA( struct TagItem * );
APTR	CreateRMHandler( Tag, ... );
void	DeleteRMHandler( APTR );
APTR	AllocRenderMem( APTR, ULONG );
void	FreeRenderMem( APTR, APTR, ULONG );
APTR	AllocRenderVec( APTR, ULONG );
void	FreeRenderVec( APTR );
APTR	AllocRenderVecClear( APTR, ULONG );

APTR	CreateHistogramA( struct TagItem * );
APTR	CreateHistogram( Tag, ... );
void	DeleteHistogram(APTR);
ULONG	QueryHistogram( APTR, Tag );
ULONG	AddRGB( APTR, ULONG, ULONG );
ULONG	AddRGBImageA( APTR, ULONG *, UWORD, UWORD, struct TagItem * );
ULONG	AddRGBImage( APTR, ULONG *, UWORD, UWORD, Tag, ... );
ULONG	AddChunkyImageA( APTR, UBYTE *, UWORD, UWORD, APTR, struct TagItem * );
ULONG	AddChunkyImage( APTR, UBYTE *, UWORD, UWORD, APTR, Tag, ... );
ULONG	ExtractPaletteA( APTR, ULONG *, UWORD, struct TagItem * );
ULONG	ExtractPalette( APTR, ULONG *, UWORD, Tag, ... );
ULONG	RenderA( ULONG *, UWORD, UWORD, UBYTE *, APTR, struct TagItem * );
ULONG	Render( ULONG *, UWORD, UWORD, UBYTE *, APTR, Tag, ... );

void	Planar2ChunkyA( PLANEPTR *, UWORD, UWORD, UWORD, UWORD, UBYTE *, struct TagItem * );
void	Planar2Chunky( PLANEPTR *, UWORD, UWORD, UWORD, UWORD, UBYTE *, Tag, ... );
ULONG	Chunky2RGBA( UBYTE *, UWORD, UWORD, ULONG *, APTR, struct TagItem * );
ULONG	Chunky2RGB( UBYTE *, UWORD, UWORD, ULONG *, APTR, Tag, ... );
void	Chunky2BitMapA( UBYTE *, UWORD, UWORD, UWORD, UWORD, struct BitMap *, UWORD, UWORD, struct TagItem * );
void	Chunky2BitMap( UBYTE *, UWORD, UWORD, UWORD, UWORD, struct BitMap *, UWORD, UWORD, Tag, ... );

APTR	CreateScaleEngineA( UWORD, UWORD, UWORD, UWORD, struct TagItem * );
APTR	CreateScaleEngine( UWORD, UWORD, UWORD, UWORD, Tag, ... );
void	DeleteScaleEngine( APTR );
ULONG	ScaleA( APTR, APTR, APTR, struct TagItem * );
ULONG	Scale( APTR, APTR, APTR, Tag, ... );

ULONG	ConvertChunkyA( UBYTE *, APTR, UWORD, UWORD, UBYTE *, APTR, struct TagItem * );
ULONG	ConvertChunky( UBYTE *, APTR, UWORD, UWORD, UBYTE *, APTR, Tag, ... );
void	CreatePenTableA( UBYTE *, APTR, UWORD, UWORD, APTR, UBYTE *, struct TagItem * );
void	CreatePenTable( UBYTE *, APTR, UWORD, UWORD, APTR, UBYTE *, Tag, ... );

APTR	CreatePaletteA( struct TagItem * );
APTR	CreatePalette( Tag, ... );
void	DeletePalette( APTR );
void	ImportPaletteA( APTR, APTR, UWORD, struct TagItem * );
void	ImportPalette( APTR, APTR, UWORD, Tag, ... );
void	ExportPaletteA( APTR, APTR, struct TagItem * );
void	ExportPalette( APTR, APTR, Tag, ... );
void	FlushPalette( APTR );
ULONG	SortPaletteA( APTR, ULONG, struct TagItem * );
ULONG	SortPalette( APTR, ULONG, Tag, ... );

ULONG	CountRGB( APTR, ULONG );
LONG	BestPen( APTR, ULONG );

ULONG	AddHistogramA( APTR, APTR, struct TagItem * );
ULONG	AddHistogram( APTR, APTR, Tag, ... );

UWORD	ScaleOrdinate( UWORD, UWORD, UWORD );

APTR	CreateMapEngineA( APTR, struct TagItem * );
APTR	CreateMapEngine( APTR, Tag, ... );
void	DeleteMapEngine( APTR );
ULONG	MapRGBArrayA( APTR, ULONG *, UWORD, UWORD, UBYTE *, struct TagItem * );
ULONG	MapRGBArray( APTR, ULONG *, UWORD, UWORD, UBYTE *, Tag, ... );
ULONG	MapChunkyArrayA( APTR, UBYTE *, APTR, UWORD, UWORD, UBYTE *, struct TagItem * );
ULONG	MapChunkyArray( APTR, UBYTE *, APTR, UWORD, UWORD, UBYTE *, Tag, ... );

LONG	RGBArrayDiversityA(ULONG *, UWORD, UWORD, struct TagItem * );
LONG	RGBArrayDiversity(ULONG *, UWORD, UWORD, Tag, ... );

LONG	ChunkyArrayDiversityA(UBYTE *, APTR, UWORD, UWORD, struct TagItem *);
LONG	ChunkyArrayDiversity(UBYTE *, APTR, UWORD, UWORD, Tag, ... );

void	InsertAlphaChannelA(UBYTE *, UWORD, UWORD, ULONG *, struct TagItem *);
void	InsertAlphaChannel(UBYTE *, UWORD, UWORD, ULONG *, Tag, ... );
void	ExtractAlphaChannelA(ULONG *, UWORD, UWORD, UBYTE *, struct TagItem *);
void	ExtractAlphaChannel(ULONG *, UWORD, UWORD, UBYTE *, Tag, ... );
void	ApplyAlphaChannelA(ULONG *, UWORD, UWORD, ULONG *, struct TagItem *);
void	ApplyAlphaChannel(ULONG *, UWORD, UWORD, ULONG *, Tag, ... );
void	MixRGBArrayA(ULONG *, UWORD, UWORD, ULONG *, UWORD, struct TagItem *);
void	MixRGBArray(ULONG *, UWORD, UWORD, ULONG *, UWORD, Tag, ... );
void	CreateAlphaArrayA(ULONG *, UWORD, UWORD, struct TagItem *);
void	CreateAlphaArray(ULONG *, UWORD, UWORD, Tag, ... );
void	MixAlphaChannelA(ULONG *, ULONG *, UWORD, UWORD, ULONG *, struct TagItem *);
void	MixAlphaChannel(ULONG *, ULONG *, UWORD, UWORD, ULONG *, Tag, ... );
void	TintRGBArrayA(ULONG *, UWORD, UWORD, ULONG, UWORD, ULONG *, struct TagItem *);
void	TintRGBArray(ULONG *, UWORD, UWORD, ULONG, UWORD, ULONG *, Tag, ... );

#endif
