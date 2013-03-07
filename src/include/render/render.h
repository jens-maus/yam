#ifndef RENDER_H
#define RENDER_H

/*
**    $VER: render.h v40 (19.12.2002)
**    render.library definitions
*/

#ifndef UTILITY_TAGITEM_H
#include <utility/tagitem.h>
#endif


#define	RND_TAGBASE	(TAG_USER+0x1000)

/************************************************************************

	memhandler

************************************************************************/

#define	RND_MemType		(RND_TAGBASE+1)		/* type of memhandler, see below */
#define	RND_MemBlock	(RND_TAGBASE+2)		/* ptr to block of memory */
#define	RND_MemSize		(RND_TAGBASE+3)		/* size of memblock [bytes] */
#define	RND_MemFlags	(RND_TAGBASE+18)	/* memflags (exec/memory.h) */
#define	RND_RMHandler	(RND_TAGBASE+12)	/* to pass a memhandler as an argument */

/*
 *	memhandler types
 */

#define	RMHTYPE_POOL		1				/* v39 exec dynamic pool */
#define	RMHTYPE_PRIVATE		2				/* private memory pool */
#define	RMHTYPE_PUBLIC		3				/* common public memory */


/************************************************************************

	palette

************************************************************************/

#define	RND_PaletteFormat	(RND_TAGBASE+19)	/* palette import/export format */
#define	RND_EHBPalette		(RND_TAGBASE+22)	/* tag to indicate a palette is EHB */
#define	RND_FirstColor		(RND_TAGBASE+23)	/* first palette entry */
#define RND_NewPalette		(RND_TAGBASE+24)	/* dispose the old palette and load a new one */
#define	RND_RGBWeight		(RND_TAGBASE+11)	/* quantization factors */

/*
 *	palette format types
 */

#define	PALFMT_RGB32		1					/* ULONG red,green,blue */
#define PALFMT_RGB8			2					/* ULONG 0x00rrggbb */
#define	PALFMT_RGB4			3					/* UWORD 0xrgb */
#define	PALFMT_PALETTE		4					/* render.library palette */

/*
 *	palette sort mode types
 *	for the use with SortPalette()
 */

	/* no particular order */
#define	PALMODE_NONE			0x0000

	/* sort palette entries by brightness */
#define	PALMODE_BRIGHTNESS		0x0001

	/* sort palette entries by the number of pixels that they represent.
	   You must supply the RND_Histogram taglist argument. */
#define	PALMODE_POPULARITY		0x0002

	/* sort palette entries by the number of histogram entries that they
	   represent. You must supply the RND_Histogram taglist argument. */
#define	PALMODE_REPRESENTATION	0x0003

	/* sort palette entries by their optical significance for the human
	   eye. Implementation is unknown to you and may change.
	   You must supply the RND_Histogram taglist argument. */
#define PALMODE_SIGNIFICANCE	0x0004

	/* sort palette entries by color intensity */
#define	PALMODE_SATURATION		0x0005

	/* By default, sort direction is descending, i.e. the precedence is
	   more-to-less. Combine with this flag to invert the sort direction. */
#define	PALMODE_ASCENDING		0x0008


/************************************************************************

	histogram related

************************************************************************/

#define	RND_HSType			(RND_TAGBASE+4)		/* histogram type, see below */
#define	RND_Histogram		(RND_TAGBASE+9)		/* a histogram as an argument */

/*
 *	Histogram / Palette types
 *	to be specified with RND_HSType
 */

#define	HSTYPE_12BIT		4					/* 12bit dynamic histogram */
#define	HSTYPE_15BIT		5					/* 15bit dynamic histogram */
#define	HSTYPE_18BIT		6					/* 18bit dynamic histogram */
#define	HSTYPE_21BIT		7					/* 21bit dynamic histogram */
#define	HSTYPE_24BIT		8					/* 24bit dynamic histogram */
#define	HSTYPE_12BIT_TURBO	20					/* 12bit tabular histogram */
#define	HSTYPE_15BIT_TURBO	21					/* 15bit tabular histogram */
#define	HSTYPE_18BIT_TURBO	22					/* 18bit tabular histogram */

/*
 *	tags that can be queried via QueryHistogram()
 */

#define	RND_NumPixels		(RND_TAGBASE+5)		/* # pixels in a histogram */
#define	RND_NumColors		(RND_TAGBASE+6)		/* # colors in a histogram */


/************************************************************************

	rendering and conversions

************************************************************************/

#define	RND_ColorMode		(RND_TAGBASE+7)		/* color mode, see below */
#define	RND_DitherMode		(RND_TAGBASE+8)		/* dither mode, see below */
#define	RND_DitherAmount	(RND_TAGBASE+26)	/* dither amount */
#define	RND_OffsetColorZero	(RND_TAGBASE+10)	/* first color index to be output */

/*
 *	color mode types
 *	to be specified with RND_ColorMode
 */

#define	COLORMODE_CLUT		0x0000				/* normal palette lookup */
#define	COLORMODE_HAM8		0x0001				/* HAM8 mode */
#define	COLORMODE_HAM6		0x0002				/* HAM6 mode */
#define	COLORMODE_MASK		0x0003				/* mask to determine COLORMODE */


/*
 *	dither mode types
 *	to be specified with RND_DitherMode
 */

#define	DITHERMODE_NONE		0x0000				/* no dither */
#define	DITHERMODE_FS		0x0001				/* Floyd-Steinberg dither */
#define	DITHERMODE_RANDOM	0x0002				/* random dither. amount required. */
#define	DITHERMODE_EDD		0x0003				/* EDD dither */


/************************************************************************

	miscellaneous

************************************************************************/

#define	RND_ProgressHook	(RND_TAGBASE+13)	/* progress callback hook */
#define	RND_SourceWidth		(RND_TAGBASE+14)	/* total input width [pixels] */
#define	RND_DestWidth		(RND_TAGBASE+15)	/* total output width [pixels] */
#define	RND_PenTable		(RND_TAGBASE+16)	/* ptr to a chunky conversion table */
#define	RND_LeftEdge		(RND_TAGBASE+17)	/* chunky data left edge [pixels] */
#define	RND_LineHook     	(RND_TAGBASE+20)	/* line callback hook */
#define	RND_MapEngine		(RND_TAGBASE+27)	/* Mapping-Engine */
#define	RND_Interleave		(RND_TAGBASE+28)	/* Interleave */
#define	RND_Palette			(RND_TAGBASE+29)	/* Palette */
#define	RND_Weight			(RND_TAGBASE+30)	/* Weight factor */
#define	RND_ScaleEngine		(RND_TAGBASE+31)	/* ScaleEngine */
#define RND_DestCoordinates	(RND_TAGBASE+42)	/* Texture coordinates */
#define	RND_BGColor			(RND_TAGBASE+43)	/* backcolor for filling */
#define	RND_BGPen			(RND_TAGBASE+44)	/* backpen for filling */


/************************************************************************

	alpha-channel and masking

************************************************************************/

#define	RND_AlphaChannel	(RND_TAGBASE+32)	/* custom alpha-channel */
#define	RND_AlphaModulo		(RND_TAGBASE+33)	/* bytes between alpha-channel pixels */
#define	RND_AlphaWidth		(RND_TAGBASE+34)	/* width of alpha-channel array */
#define	RND_MaskRGB		(RND_TAGBASE+35)	/* masking RGB for CreateAlphaArray */
#define	RND_MaskFalse		(RND_TAGBASE+36)	/* mask value for outside color range */
#define	RND_MaskTrue		(RND_TAGBASE+37)	/* mask value for inside color range */

#define	RND_SourceWidth2	(RND_TAGBASE+38)	/* total source width for 3channel operations */
#define	RND_AlphaChannel2	(RND_TAGBASE+39)	/* second custom alpha-channel */
#define	RND_AlphaModulo2	(RND_TAGBASE+40)	/* pixel modulo for a second alpha-channel */
#define	RND_AlphaWidth2		(RND_TAGBASE+41)	/* width of a second alpha-channel array */


/************************************************************************

	PixelFormat

************************************************************************/

#define	RND_PixelFormat    (RND_TAGBASE+25)		/* pixel format, see below */

#define	PIXFMTB_CHUNKY		3
#define	PIXFMTB_BITMAP		4
#define	PIXFMTB_RGB			5

#define	PIXFMT_CHUNKY_CLUT	((1L << PIXFMTB_CHUNKY) + COLORMODE_CLUT)
#define	PIXFMT_0RGB_32		((1L << PIXFMTB_RGB) + 0)

/*
 *	these types are currently not used by render.library, but
 *	some of them are applicable for guigfx.library functions:
 */

#define	PIXFMT_CHUNKY_HAM8	((1L << PIXFMTB_CHUNKY) + COLORMODE_HAM8)
#define	PIXFMT_CHUNKY_HAM6	((1L << PIXFMTB_CHUNKY) + COLORMODE_HAM6)
#define	PIXFMT_BITMAP_CLUT	((1L << PIXFMTB_BITMAP) + COLORMODE_CLUT)
#define	PIXFMT_BITMAP_HAM8	((1L << PIXFMTB_BITMAP) + COLORMODE_HAM8)
#define	PIXFMT_BITMAP_HAM6	((1L << PIXFMTB_BITMAP) + COLORMODE_HAM6)

#define	PIXFMT_RGB_24		((1L << PIXFMTB_RGB) + 1)

/*
 *	strictly internal:
 */

#define PIXFMT_BITMAP_RGB	((1L << PIXFMTB_BITMAP) + (1L << PIXFMTB_RGB))


/************************************************************************

	ExtractPalette return codes

	You must at least check for EXTP_SUCCESS.
	EXTP_NO_DATA indicates that there were no colors
	in the histogram.

************************************************************************/

#define	EXTP_SUCCESS			0
#define	EXTP_NOT_ENOUGH_MEMORY	1
#define	EXTP_CALLBACK_ABORTED	2
#define	EXTP_NO_DATA			3


/************************************************************************

	AddRGB, AddRGBImage and AddChunkyImage return codes

	You must at least check for ADDH_SUCCESS.
	If not delivered, the histogram might be
	inaccurate.

************************************************************************/

#define	ADDH_SUCCESS				0
#define	ADDH_NOT_ENOUGH_MEMORY		1
#define	ADDH_CALLBACK_ABORTED		2
#define ADDH_NO_DATA				3


/************************************************************************

	Render return codes

	You must at least check for REND_SUCCESS.
	If not delivered, the image has not been
	rendered completely.

************************************************************************/

#define	REND_SUCCESS				0
#define	REND_NOT_ENOUGH_MEMORY		1
#define	REND_CALLBACK_ABORTED		2
#define	REND_NO_VALID_PALETTE		3
#define	REND_NO_DATA				3


/************************************************************************

	SortPalette return codes

	You must at least check for SORTP_SUCCESS.
	SORTP_NO_DATA indicates that there were data missing,
	e.g. you specified no histogram or the histogram was empty.

************************************************************************/

#define	SORTP_SUCCESS				0
#define	SORTP_NO_DATA				1
#define	SORTP_NOT_ENOUGH_MEMORY		2
#define	SORTP_NOT_IMPLEMENTED		3


/************************************************************************

	conversion return codes

	These return codes apply to conversion functions
	such as Chunky2RGB and ConvertChunky.

************************************************************************/

#define	CONV_SUCCESS			0
#define	CONV_CALLBACK_ABORTED	1
#define CONV_NOT_ENOUGH_MEMORY	2
#define CONV_NO_DATA			3


/***********************************************************************/

#endif
