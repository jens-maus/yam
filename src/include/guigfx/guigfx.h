#ifndef GUIGFX_H
#define GUIGFX_H	1
/*
**    $VER: guigfx.h 20 (14.2.2003)
**    guigfx.library definitions
**    © 1997-2003 TEK neoscientists
*/

#ifndef UTILITY_TAGITEM_H
#include <utility/tagitem.h>
#endif

#ifndef RENDER_H
#include <render/render.h>
#endif

/*
 *	Tags
 */

#define	GGFX_Dummy		(4567+TAG_USER)
#define	GGFX_Owner		(GGFX_Dummy+0)		/* strictly private */
#define	GGFX_HSType		(GGFX_Dummy+1)
#define	GGFX_DitherMode		(GGFX_Dummy+2)
#define	GGFX_DitherAmount	(GGFX_Dummy+3)
#define	GGFX_AutoDither		(GGFX_Dummy+4)
#define	GGFX_DitherThreshold	(GGFX_Dummy+5)
#define	GGFX_AspectX		(GGFX_Dummy+6)
#define	GGFX_AspectY		(GGFX_Dummy+7)
#define	GGFX_PixelFormat	(GGFX_Dummy+8)
#define	GGFX_Palette		(GGFX_Dummy+9)
#define	GGFX_PaletteFormat	(GGFX_Dummy+10)
#define	GGFX_NumColors		(GGFX_Dummy+11)
#define	GGFX_Precision		(GGFX_Dummy+12)
#define	GGFX_Weight		(GGFX_Dummy+13)
#define	GGFX_Ratio		(GGFX_Dummy+14)
#define GGFX_SourceWidth	(GGFX_Dummy+15)
#define GGFX_SourceHeight	(GGFX_Dummy+16)
#define GGFX_SourceX		(GGFX_Dummy+17)
#define GGFX_SourceY		(GGFX_Dummy+18)
#define GGFX_DestWidth		(GGFX_Dummy+19)
#define GGFX_DestHeight		(GGFX_Dummy+20)
#define GGFX_DestX		(GGFX_Dummy+21)
#define GGFX_DestY		(GGFX_Dummy+22)
#define	GGFX_CallBackHook	(GGFX_Dummy+23)
#define	GGFX_ErrorCode		(GGFX_Dummy+24)
#define	GGFX_MaxAllocPens	(GGFX_Dummy+25)
#define	GGFX_BufferSize		(GGFX_Dummy+26)
#define	GGFX_AlphaPresent	(GGFX_Dummy+27)
#define	GGFX_Independent	(GGFX_Dummy+28)
#define	GGFX_ModeID		(GGFX_Dummy+29)
#define GGFX_PenTable		(GGFX_Dummy+30)
#define GGFX_License		(GGFX_Dummy+31)		/* obsolete */
#define GGFX_BGColor		(GGFX_Dummy+32)
#define GGFX_UseMask		(GGFX_Dummy+33)
#define	GGFX_RastLock		(GGFX_Dummy+34)
#define GGFX_FormatName		(GGFX_Dummy+35)

/*
 *	Picture Attributes
 */

#define PICATTR_Dummy		(123+TAG_USER)
#define PICATTR_Width		(PICATTR_Dummy+0)
#define PICATTR_Height		(PICATTR_Dummy+1)
#define PICATTR_RawData		(PICATTR_Dummy+2)
#define PICATTR_PixelFormat	(PICATTR_Dummy+3)
#define PICATTR_AspectX		(PICATTR_Dummy+4)
#define PICATTR_AspectY		(PICATTR_Dummy+5)
#define	PICATTR_AlphaPresent	(PICATTR_Dummy+6)
#define	PICATTR_NumPaletteEntries	(PICATTR_Dummy+7)
#define	PICATTR_Palette		(PICATTR_Dummy+8)


/*
 *	Picture Methods
 */

#define	PICMTHD_CROP		1
#define	PICMTHD_RENDER		2
#define	PICMTHD_SCALE		3
#define	PICMTHD_MIX		4
#define	PICMTHD_SETALPHA	5
#define	PICMTHD_MIXALPHA	6
#define	PICMTHD_MAPDRAWHANDLE	7
#define	PICMTHD_CREATEALPHAMASK	8
#define	PICMTHD_TINT		9
#define	PICMTHD_TEXTURE		10
#define	PICMTHD_SET		11
#define	PICMTHD_TINTALPHA	12
#define	PICMTHD_INSERT		13
#define	PICMTHD_FLIPX		14
#define	PICMTHD_FLIPY		15
#define	PICMTHD_CHECKAUTODITHER	16
#define	PICMTHD_NEGATIVE	17
#define	PICMTHD_AUTOCROP	18
#define	PICMTHD_CONVOLVE	19


/*
 *	hook message types
 */

#define	GGFX_MSGTYPE_LINEDRAWN		1


/*
 *	picture locking
 */

#define LOCKMODE_DRAWHANDLE		1
#define	LOCKMODE_FORCE			(1<<8)
#define	LOCKMODE_MASK			(0xff)


/*
 *	useful types
 */

typedef void PICTURE;



/*
 *	bitmap attributes
 *	(strictly internal)
 */


#define BMAPATTR_Width			(0+TAG_USER)
#define BMAPATTR_Height			(1+TAG_USER)
#define BMAPATTR_Depth			(2+TAG_USER)
#define BMAPATTR_CyberGFX		(3+TAG_USER)
#define BMAPATTR_BitMapFormat	(4+TAG_USER)
#define BMAPATTR_PixelFormat	(5+TAG_USER)
#define BMAPATTR_Flags			(6+TAG_USER)


#endif
