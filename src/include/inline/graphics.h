#ifndef _INLINE_GRAPHICS_H
#define _INLINE_GRAPHICS_H

#ifndef  GRAPHICS_GFX_H
#include <graphics/gfx.h>
#endif
#ifndef  GRAPHICS_DISPLAYINFO_H
#include <graphics/displayinfo.h>
#endif
#ifndef  GRAPHICS_GELS_H
#include <graphics/gels.h>
#endif
#ifndef  GRAPHICS_RASTPORT_H
#include <graphics/rastport.h>
#endif
#ifndef  GRAPHICS_VIEW_H
#include <graphics/view.h>
#endif
#ifndef  GRAPHICS_COPPER_H
#include <graphics/copper.h>
#endif
#ifndef  GRAPHICS_CLIP_H
#include <graphics/clip.h>
#endif
#ifndef  GRAPHICS_REGIONS_H
#include <graphics/regions.h>
#endif
#ifndef  GRAPHICS_SCALE_H
#include <graphics/scale.h>
#endif
#ifndef  GRAPHICS_SPRITE_H
#include <graphics/sprite.h>
#endif
#ifndef  GRAPHICS_TEXT_H
#include <graphics/text.h>
#endif
#ifndef  HARDWARE_BLIT_H
#include <hardware/blit.h>
#endif

#ifndef GRAPHICS_BASE_NAME
#define GRAPHICS_BASE_NAME GfxBase
#endif

#define BltBitMap(srcBitMap, xSrc, ySrc, destBitMap, xDest, yDest, xSize, ySize, minterm, mask, tempA) ({ \
  CONST struct BitMap * _BltBitMap_srcBitMap = (srcBitMap); \
  LONG _BltBitMap_xSrc = (xSrc); \
  LONG _BltBitMap_ySrc = (ySrc); \
  struct BitMap * _BltBitMap_destBitMap = (destBitMap); \
  LONG _BltBitMap_xDest = (xDest); \
  LONG _BltBitMap_yDest = (yDest); \
  LONG _BltBitMap_xSize = (xSize); \
  LONG _BltBitMap_ySize = (ySize); \
  ULONG _BltBitMap_minterm = (minterm); \
  ULONG _BltBitMap_mask = (mask); \
  PLANEPTR _BltBitMap_tempA = (tempA); \
  ULONG _BltBitMap__re = \
  ({ \
  register struct GfxBase * const __BltBitMap__bn __asm("a6") = (struct GfxBase *) (GRAPHICS_BASE_NAME);\
  register ULONG __BltBitMap__re __asm("d0"); \
  register CONST struct BitMap * __BltBitMap_srcBitMap __asm("a0") = (_BltBitMap_srcBitMap); \
  register LONG __BltBitMap_xSrc __asm("d0") = (_BltBitMap_xSrc); \
  register LONG __BltBitMap_ySrc __asm("d1") = (_BltBitMap_ySrc); \
  register struct BitMap * __BltBitMap_destBitMap __asm("a1") = (_BltBitMap_destBitMap); \
  register LONG __BltBitMap_xDest __asm("d2") = (_BltBitMap_xDest); \
  register LONG __BltBitMap_yDest __asm("d3") = (_BltBitMap_yDest); \
  register LONG __BltBitMap_xSize __asm("d4") = (_BltBitMap_xSize); \
  register LONG __BltBitMap_ySize __asm("d5") = (_BltBitMap_ySize); \
  register ULONG __BltBitMap_minterm __asm("d6") = (_BltBitMap_minterm); \
  register ULONG __BltBitMap_mask __asm("d7") = (_BltBitMap_mask); \
  register PLANEPTR __BltBitMap_tempA __asm("a2") = (_BltBitMap_tempA); \
  __asm volatile ("jsr a6@(-30:W)" \
  : "=r"(__BltBitMap__re) \
  : "r"(__BltBitMap__bn), "r"(__BltBitMap_srcBitMap), "r"(__BltBitMap_xSrc), "r"(__BltBitMap_ySrc), "r"(__BltBitMap_destBitMap), "r"(__BltBitMap_xDest), "r"(__BltBitMap_yDest), "r"(__BltBitMap_xSize), "r"(__BltBitMap_ySize), "r"(__BltBitMap_minterm), "r"(__BltBitMap_mask), "r"(__BltBitMap_tempA)  \
  : "d1", "a0", "a1", "fp0", "fp1", "cc", "memory"); \
  __BltBitMap__re; \
  }); \
  _BltBitMap__re; \
})

#define BltTemplate(source, xSrc, srcMod, destRP, xDest, yDest, xSize, ySize) ({ \
  CONST PLANEPTR _BltTemplate_source = (source); \
  LONG _BltTemplate_xSrc = (xSrc); \
  LONG _BltTemplate_srcMod = (srcMod); \
  struct RastPort * _BltTemplate_destRP = (destRP); \
  LONG _BltTemplate_xDest = (xDest); \
  LONG _BltTemplate_yDest = (yDest); \
  LONG _BltTemplate_xSize = (xSize); \
  LONG _BltTemplate_ySize = (ySize); \
  { \
  register struct GfxBase * const __BltTemplate__bn __asm("a6") = (struct GfxBase *) (GRAPHICS_BASE_NAME);\
  register CONST PLANEPTR __BltTemplate_source __asm("a0") = (_BltTemplate_source); \
  register LONG __BltTemplate_xSrc __asm("d0") = (_BltTemplate_xSrc); \
  register LONG __BltTemplate_srcMod __asm("d1") = (_BltTemplate_srcMod); \
  register struct RastPort * __BltTemplate_destRP __asm("a1") = (_BltTemplate_destRP); \
  register LONG __BltTemplate_xDest __asm("d2") = (_BltTemplate_xDest); \
  register LONG __BltTemplate_yDest __asm("d3") = (_BltTemplate_yDest); \
  register LONG __BltTemplate_xSize __asm("d4") = (_BltTemplate_xSize); \
  register LONG __BltTemplate_ySize __asm("d5") = (_BltTemplate_ySize); \
  __asm volatile ("jsr a6@(-36:W)" \
  : \
  : "r"(__BltTemplate__bn), "r"(__BltTemplate_source), "r"(__BltTemplate_xSrc), "r"(__BltTemplate_srcMod), "r"(__BltTemplate_destRP), "r"(__BltTemplate_xDest), "r"(__BltTemplate_yDest), "r"(__BltTemplate_xSize), "r"(__BltTemplate_ySize)  \
  : "d0", "d1", "a0", "a1", "fp0", "fp1", "cc", "memory"); \
  } \
})

#define ClearEOL(rp) ({ \
  struct RastPort * _ClearEOL_rp = (rp); \
  { \
  register struct GfxBase * const __ClearEOL__bn __asm("a6") = (struct GfxBase *) (GRAPHICS_BASE_NAME);\
  register struct RastPort * __ClearEOL_rp __asm("a1") = (_ClearEOL_rp); \
  __asm volatile ("jsr a6@(-42:W)" \
  : \
  : "r"(__ClearEOL__bn), "r"(__ClearEOL_rp)  \
  : "d0", "d1", "a0", "a1", "fp0", "fp1", "cc", "memory"); \
  } \
})

#define ClearScreen(rp) ({ \
  struct RastPort * _ClearScreen_rp = (rp); \
  { \
  register struct GfxBase * const __ClearScreen__bn __asm("a6") = (struct GfxBase *) (GRAPHICS_BASE_NAME);\
  register struct RastPort * __ClearScreen_rp __asm("a1") = (_ClearScreen_rp); \
  __asm volatile ("jsr a6@(-48:W)" \
  : \
  : "r"(__ClearScreen__bn), "r"(__ClearScreen_rp)  \
  : "d0", "d1", "a0", "a1", "fp0", "fp1", "cc", "memory"); \
  } \
})

#define TextLength(rp, string, count) ({ \
  struct RastPort * _TextLength_rp = (rp); \
  CONST_STRPTR _TextLength_string = (string); \
  ULONG _TextLength_count = (count); \
  WORD _TextLength__re = \
  ({ \
  register struct GfxBase * const __TextLength__bn __asm("a6") = (struct GfxBase *) (GRAPHICS_BASE_NAME);\
  register WORD __TextLength__re __asm("d0"); \
  register struct RastPort * __TextLength_rp __asm("a1") = (_TextLength_rp); \
  register CONST_STRPTR __TextLength_string __asm("a0") = (_TextLength_string); \
  register ULONG __TextLength_count __asm("d0") = (_TextLength_count); \
  __asm volatile ("jsr a6@(-54:W)" \
  : "=r"(__TextLength__re) \
  : "r"(__TextLength__bn), "r"(__TextLength_rp), "r"(__TextLength_string), "r"(__TextLength_count)  \
  : "d1", "a0", "a1", "fp0", "fp1", "cc", "memory"); \
  __TextLength__re; \
  }); \
  _TextLength__re; \
})

#define Text(rp, string, count) ({ \
  struct RastPort * _Text_rp = (rp); \
  CONST_STRPTR _Text_string = (string); \
  ULONG _Text_count = (count); \
  { \
  register struct GfxBase * const __Text__bn __asm("a6") = (struct GfxBase *) (GRAPHICS_BASE_NAME);\
  register struct RastPort * __Text_rp __asm("a1") = (_Text_rp); \
  register CONST_STRPTR __Text_string __asm("a0") = (_Text_string); \
  register ULONG __Text_count __asm("d0") = (_Text_count); \
  __asm volatile ("jsr a6@(-60:W)" \
  : \
  : "r"(__Text__bn), "r"(__Text_rp), "r"(__Text_string), "r"(__Text_count)  \
  : "d0", "d1", "a0", "a1", "fp0", "fp1", "cc", "memory"); \
  } \
})

#define SetFont(rp, textFont) ({ \
  struct RastPort * _SetFont_rp = (rp); \
  struct TextFont * _SetFont_textFont = (textFont); \
  { \
  register struct GfxBase * const __SetFont__bn __asm("a6") = (struct GfxBase *) (GRAPHICS_BASE_NAME);\
  register struct RastPort * __SetFont_rp __asm("a1") = (_SetFont_rp); \
  register struct TextFont * __SetFont_textFont __asm("a0") = (_SetFont_textFont); \
  __asm volatile ("jsr a6@(-66:W)" \
  : \
  : "r"(__SetFont__bn), "r"(__SetFont_rp), "r"(__SetFont_textFont)  \
  : "d0", "d1", "a0", "a1", "fp0", "fp1", "cc", "memory"); \
  } \
})

#define OpenFont(textAttr) ({ \
  CONST struct TextAttr * _OpenFont_textAttr = (textAttr); \
  struct TextFont * _OpenFont__re = \
  ({ \
  register struct GfxBase * const __OpenFont__bn __asm("a6") = (struct GfxBase *) (GRAPHICS_BASE_NAME);\
  register struct TextFont * __OpenFont__re __asm("d0"); \
  register CONST struct TextAttr * __OpenFont_textAttr __asm("a0") = (_OpenFont_textAttr); \
  __asm volatile ("jsr a6@(-72:W)" \
  : "=r"(__OpenFont__re) \
  : "r"(__OpenFont__bn), "r"(__OpenFont_textAttr)  \
  : "d1", "a0", "a1", "fp0", "fp1", "cc", "memory"); \
  __OpenFont__re; \
  }); \
  _OpenFont__re; \
})

#define CloseFont(textFont) ({ \
  struct TextFont * _CloseFont_textFont = (textFont); \
  { \
  register struct GfxBase * const __CloseFont__bn __asm("a6") = (struct GfxBase *) (GRAPHICS_BASE_NAME);\
  register struct TextFont * __CloseFont_textFont __asm("a1") = (_CloseFont_textFont); \
  __asm volatile ("jsr a6@(-78:W)" \
  : \
  : "r"(__CloseFont__bn), "r"(__CloseFont_textFont)  \
  : "d0", "d1", "a0", "a1", "fp0", "fp1", "cc", "memory"); \
  } \
})

#define AskSoftStyle(rp) ({ \
  struct RastPort * _AskSoftStyle_rp = (rp); \
  ULONG _AskSoftStyle__re = \
  ({ \
  register struct GfxBase * const __AskSoftStyle__bn __asm("a6") = (struct GfxBase *) (GRAPHICS_BASE_NAME);\
  register ULONG __AskSoftStyle__re __asm("d0"); \
  register struct RastPort * __AskSoftStyle_rp __asm("a1") = (_AskSoftStyle_rp); \
  __asm volatile ("jsr a6@(-84:W)" \
  : "=r"(__AskSoftStyle__re) \
  : "r"(__AskSoftStyle__bn), "r"(__AskSoftStyle_rp)  \
  : "d1", "a0", "a1", "fp0", "fp1", "cc", "memory"); \
  __AskSoftStyle__re; \
  }); \
  _AskSoftStyle__re; \
})

#define SetSoftStyle(rp, style, enable) ({ \
  struct RastPort * _SetSoftStyle_rp = (rp); \
  ULONG _SetSoftStyle_style = (style); \
  ULONG _SetSoftStyle_enable = (enable); \
  ULONG _SetSoftStyle__re = \
  ({ \
  register struct GfxBase * const __SetSoftStyle__bn __asm("a6") = (struct GfxBase *) (GRAPHICS_BASE_NAME);\
  register ULONG __SetSoftStyle__re __asm("d0"); \
  register struct RastPort * __SetSoftStyle_rp __asm("a1") = (_SetSoftStyle_rp); \
  register ULONG __SetSoftStyle_style __asm("d0") = (_SetSoftStyle_style); \
  register ULONG __SetSoftStyle_enable __asm("d1") = (_SetSoftStyle_enable); \
  __asm volatile ("jsr a6@(-90:W)" \
  : "=r"(__SetSoftStyle__re) \
  : "r"(__SetSoftStyle__bn), "r"(__SetSoftStyle_rp), "r"(__SetSoftStyle_style), "r"(__SetSoftStyle_enable)  \
  : "d1", "a0", "a1", "fp0", "fp1", "cc", "memory"); \
  __SetSoftStyle__re; \
  }); \
  _SetSoftStyle__re; \
})

#define AddBob(bob, rp) ({ \
  struct Bob * _AddBob_bob = (bob); \
  struct RastPort * _AddBob_rp = (rp); \
  { \
  register struct GfxBase * const __AddBob__bn __asm("a6") = (struct GfxBase *) (GRAPHICS_BASE_NAME);\
  register struct Bob * __AddBob_bob __asm("a0") = (_AddBob_bob); \
  register struct RastPort * __AddBob_rp __asm("a1") = (_AddBob_rp); \
  __asm volatile ("jsr a6@(-96:W)" \
  : \
  : "r"(__AddBob__bn), "r"(__AddBob_bob), "r"(__AddBob_rp)  \
  : "d0", "d1", "a0", "a1", "fp0", "fp1", "cc", "memory"); \
  } \
})

#define AddVSprite(vSprite, rp) ({ \
  struct VSprite * _AddVSprite_vSprite = (vSprite); \
  struct RastPort * _AddVSprite_rp = (rp); \
  { \
  register struct GfxBase * const __AddVSprite__bn __asm("a6") = (struct GfxBase *) (GRAPHICS_BASE_NAME);\
  register struct VSprite * __AddVSprite_vSprite __asm("a0") = (_AddVSprite_vSprite); \
  register struct RastPort * __AddVSprite_rp __asm("a1") = (_AddVSprite_rp); \
  __asm volatile ("jsr a6@(-102:W)" \
  : \
  : "r"(__AddVSprite__bn), "r"(__AddVSprite_vSprite), "r"(__AddVSprite_rp)  \
  : "d0", "d1", "a0", "a1", "fp0", "fp1", "cc", "memory"); \
  } \
})

#define DoCollision(rp) ({ \
  struct RastPort * _DoCollision_rp = (rp); \
  { \
  register struct GfxBase * const __DoCollision__bn __asm("a6") = (struct GfxBase *) (GRAPHICS_BASE_NAME);\
  register struct RastPort * __DoCollision_rp __asm("a1") = (_DoCollision_rp); \
  __asm volatile ("jsr a6@(-108:W)" \
  : \
  : "r"(__DoCollision__bn), "r"(__DoCollision_rp)  \
  : "d0", "d1", "a0", "a1", "fp0", "fp1", "cc", "memory"); \
  } \
})

#define DrawGList(rp, vp) ({ \
  struct RastPort * _DrawGList_rp = (rp); \
  struct ViewPort * _DrawGList_vp = (vp); \
  { \
  register struct GfxBase * const __DrawGList__bn __asm("a6") = (struct GfxBase *) (GRAPHICS_BASE_NAME);\
  register struct RastPort * __DrawGList_rp __asm("a1") = (_DrawGList_rp); \
  register struct ViewPort * __DrawGList_vp __asm("a0") = (_DrawGList_vp); \
  __asm volatile ("jsr a6@(-114:W)" \
  : \
  : "r"(__DrawGList__bn), "r"(__DrawGList_rp), "r"(__DrawGList_vp)  \
  : "d0", "d1", "a0", "a1", "fp0", "fp1", "cc", "memory"); \
  } \
})

#define InitGels(head, tail, gelsInfo) ({ \
  struct VSprite * _InitGels_head = (head); \
  struct VSprite * _InitGels_tail = (tail); \
  struct GelsInfo * _InitGels_gelsInfo = (gelsInfo); \
  { \
  register struct GfxBase * const __InitGels__bn __asm("a6") = (struct GfxBase *) (GRAPHICS_BASE_NAME);\
  register struct VSprite * __InitGels_head __asm("a0") = (_InitGels_head); \
  register struct VSprite * __InitGels_tail __asm("a1") = (_InitGels_tail); \
  register struct GelsInfo * __InitGels_gelsInfo __asm("a2") = (_InitGels_gelsInfo); \
  __asm volatile ("jsr a6@(-120:W)" \
  : \
  : "r"(__InitGels__bn), "r"(__InitGels_head), "r"(__InitGels_tail), "r"(__InitGels_gelsInfo)  \
  : "d0", "d1", "a0", "a1", "fp0", "fp1", "cc", "memory"); \
  } \
})

#define InitMasks(vSprite) ({ \
  struct VSprite * _InitMasks_vSprite = (vSprite); \
  { \
  register struct GfxBase * const __InitMasks__bn __asm("a6") = (struct GfxBase *) (GRAPHICS_BASE_NAME);\
  register struct VSprite * __InitMasks_vSprite __asm("a0") = (_InitMasks_vSprite); \
  __asm volatile ("jsr a6@(-126:W)" \
  : \
  : "r"(__InitMasks__bn), "r"(__InitMasks_vSprite)  \
  : "d0", "d1", "a0", "a1", "fp0", "fp1", "cc", "memory"); \
  } \
})

#define RemIBob(bob, rp, vp) ({ \
  struct Bob * _RemIBob_bob = (bob); \
  struct RastPort * _RemIBob_rp = (rp); \
  struct ViewPort * _RemIBob_vp = (vp); \
  { \
  register struct GfxBase * const __RemIBob__bn __asm("a6") = (struct GfxBase *) (GRAPHICS_BASE_NAME);\
  register struct Bob * __RemIBob_bob __asm("a0") = (_RemIBob_bob); \
  register struct RastPort * __RemIBob_rp __asm("a1") = (_RemIBob_rp); \
  register struct ViewPort * __RemIBob_vp __asm("a2") = (_RemIBob_vp); \
  __asm volatile ("jsr a6@(-132:W)" \
  : \
  : "r"(__RemIBob__bn), "r"(__RemIBob_bob), "r"(__RemIBob_rp), "r"(__RemIBob_vp)  \
  : "d0", "d1", "a0", "a1", "fp0", "fp1", "cc", "memory"); \
  } \
})

#define RemVSprite(vSprite) ({ \
  struct VSprite * _RemVSprite_vSprite = (vSprite); \
  { \
  register struct GfxBase * const __RemVSprite__bn __asm("a6") = (struct GfxBase *) (GRAPHICS_BASE_NAME);\
  register struct VSprite * __RemVSprite_vSprite __asm("a0") = (_RemVSprite_vSprite); \
  __asm volatile ("jsr a6@(-138:W)" \
  : \
  : "r"(__RemVSprite__bn), "r"(__RemVSprite_vSprite)  \
  : "d0", "d1", "a0", "a1", "fp0", "fp1", "cc", "memory"); \
  } \
})

#define SetCollision(num, routine, gelsInfo) ({ \
  ULONG _SetCollision_num = (num); \
  VOID (*_SetCollision_routine)( struct VSprite *gelA, struct VSprite *gelB ) = (routine); \
  struct GelsInfo * _SetCollision_gelsInfo = (gelsInfo); \
  { \
  register struct GfxBase * const __SetCollision__bn __asm("a6") = (struct GfxBase *) (GRAPHICS_BASE_NAME);\
  register ULONG __SetCollision_num __asm("d0") = (_SetCollision_num); \
  register VOID (*__SetCollision_routine)( struct VSprite *gelA, struct VSprite *gelB ) __asm("a0") = (_SetCollision_routine); \
  register struct GelsInfo * __SetCollision_gelsInfo __asm("a1") = (_SetCollision_gelsInfo); \
  __asm volatile ("jsr a6@(-144:W)" \
  : \
  : "r"(__SetCollision__bn), "r"(__SetCollision_num), "r"(__SetCollision_routine), "r"(__SetCollision_gelsInfo)  \
  : "d0", "d1", "a0", "a1", "fp0", "fp1", "cc", "memory"); \
  } \
})

#define SortGList(rp) ({ \
  struct RastPort * _SortGList_rp = (rp); \
  { \
  register struct GfxBase * const __SortGList__bn __asm("a6") = (struct GfxBase *) (GRAPHICS_BASE_NAME);\
  register struct RastPort * __SortGList_rp __asm("a1") = (_SortGList_rp); \
  __asm volatile ("jsr a6@(-150:W)" \
  : \
  : "r"(__SortGList__bn), "r"(__SortGList_rp)  \
  : "d0", "d1", "a0", "a1", "fp0", "fp1", "cc", "memory"); \
  } \
})

#define AddAnimOb(anOb, anKey, rp) ({ \
  struct AnimOb * _AddAnimOb_anOb = (anOb); \
  struct AnimOb ** _AddAnimOb_anKey = (anKey); \
  struct RastPort * _AddAnimOb_rp = (rp); \
  { \
  register struct GfxBase * const __AddAnimOb__bn __asm("a6") = (struct GfxBase *) (GRAPHICS_BASE_NAME);\
  register struct AnimOb * __AddAnimOb_anOb __asm("a0") = (_AddAnimOb_anOb); \
  register struct AnimOb ** __AddAnimOb_anKey __asm("a1") = (_AddAnimOb_anKey); \
  register struct RastPort * __AddAnimOb_rp __asm("a2") = (_AddAnimOb_rp); \
  __asm volatile ("jsr a6@(-156:W)" \
  : \
  : "r"(__AddAnimOb__bn), "r"(__AddAnimOb_anOb), "r"(__AddAnimOb_anKey), "r"(__AddAnimOb_rp)  \
  : "d0", "d1", "a0", "a1", "fp0", "fp1", "cc", "memory"); \
  } \
})

#define Animate(anKey, rp) ({ \
  struct AnimOb ** _Animate_anKey = (anKey); \
  struct RastPort * _Animate_rp = (rp); \
  { \
  register struct GfxBase * const __Animate__bn __asm("a6") = (struct GfxBase *) (GRAPHICS_BASE_NAME);\
  register struct AnimOb ** __Animate_anKey __asm("a0") = (_Animate_anKey); \
  register struct RastPort * __Animate_rp __asm("a1") = (_Animate_rp); \
  __asm volatile ("jsr a6@(-162:W)" \
  : \
  : "r"(__Animate__bn), "r"(__Animate_anKey), "r"(__Animate_rp)  \
  : "d0", "d1", "a0", "a1", "fp0", "fp1", "cc", "memory"); \
  } \
})

#define GetGBuffers(anOb, rp, flag) ({ \
  struct AnimOb * _GetGBuffers_anOb = (anOb); \
  struct RastPort * _GetGBuffers_rp = (rp); \
  LONG _GetGBuffers_flag = (flag); \
  BOOL _GetGBuffers__re = \
  ({ \
  register struct GfxBase * const __GetGBuffers__bn __asm("a6") = (struct GfxBase *) (GRAPHICS_BASE_NAME);\
  register BOOL __GetGBuffers__re __asm("d0"); \
  register struct AnimOb * __GetGBuffers_anOb __asm("a0") = (_GetGBuffers_anOb); \
  register struct RastPort * __GetGBuffers_rp __asm("a1") = (_GetGBuffers_rp); \
  register LONG __GetGBuffers_flag __asm("d0") = (_GetGBuffers_flag); \
  __asm volatile ("jsr a6@(-168:W)" \
  : "=r"(__GetGBuffers__re) \
  : "r"(__GetGBuffers__bn), "r"(__GetGBuffers_anOb), "r"(__GetGBuffers_rp), "r"(__GetGBuffers_flag)  \
  : "d1", "a0", "a1", "fp0", "fp1", "cc", "memory"); \
  __GetGBuffers__re; \
  }); \
  _GetGBuffers__re; \
})

#define InitGMasks(anOb) ({ \
  struct AnimOb * _InitGMasks_anOb = (anOb); \
  { \
  register struct GfxBase * const __InitGMasks__bn __asm("a6") = (struct GfxBase *) (GRAPHICS_BASE_NAME);\
  register struct AnimOb * __InitGMasks_anOb __asm("a0") = (_InitGMasks_anOb); \
  __asm volatile ("jsr a6@(-174:W)" \
  : \
  : "r"(__InitGMasks__bn), "r"(__InitGMasks_anOb)  \
  : "d0", "d1", "a0", "a1", "fp0", "fp1", "cc", "memory"); \
  } \
})

#define DrawEllipse(rp, xCenter, yCenter, a, b) ({ \
  struct RastPort * _DrawEllipse_rp = (rp); \
  LONG _DrawEllipse_xCenter = (xCenter); \
  LONG _DrawEllipse_yCenter = (yCenter); \
  LONG _DrawEllipse_a = (a); \
  LONG _DrawEllipse_b = (b); \
  { \
  register struct GfxBase * const __DrawEllipse__bn __asm("a6") = (struct GfxBase *) (GRAPHICS_BASE_NAME);\
  register struct RastPort * __DrawEllipse_rp __asm("a1") = (_DrawEllipse_rp); \
  register LONG __DrawEllipse_xCenter __asm("d0") = (_DrawEllipse_xCenter); \
  register LONG __DrawEllipse_yCenter __asm("d1") = (_DrawEllipse_yCenter); \
  register LONG __DrawEllipse_a __asm("d2") = (_DrawEllipse_a); \
  register LONG __DrawEllipse_b __asm("d3") = (_DrawEllipse_b); \
  __asm volatile ("jsr a6@(-180:W)" \
  : \
  : "r"(__DrawEllipse__bn), "r"(__DrawEllipse_rp), "r"(__DrawEllipse_xCenter), "r"(__DrawEllipse_yCenter), "r"(__DrawEllipse_a), "r"(__DrawEllipse_b)  \
  : "d0", "d1", "a0", "a1", "fp0", "fp1", "cc", "memory"); \
  } \
})

#define AreaEllipse(rp, xCenter, yCenter, a, b) ({ \
  struct RastPort * _AreaEllipse_rp = (rp); \
  LONG _AreaEllipse_xCenter = (xCenter); \
  LONG _AreaEllipse_yCenter = (yCenter); \
  LONG _AreaEllipse_a = (a); \
  LONG _AreaEllipse_b = (b); \
  LONG _AreaEllipse__re = \
  ({ \
  register struct GfxBase * const __AreaEllipse__bn __asm("a6") = (struct GfxBase *) (GRAPHICS_BASE_NAME);\
  register LONG __AreaEllipse__re __asm("d0"); \
  register struct RastPort * __AreaEllipse_rp __asm("a1") = (_AreaEllipse_rp); \
  register LONG __AreaEllipse_xCenter __asm("d0") = (_AreaEllipse_xCenter); \
  register LONG __AreaEllipse_yCenter __asm("d1") = (_AreaEllipse_yCenter); \
  register LONG __AreaEllipse_a __asm("d2") = (_AreaEllipse_a); \
  register LONG __AreaEllipse_b __asm("d3") = (_AreaEllipse_b); \
  __asm volatile ("jsr a6@(-186:W)" \
  : "=r"(__AreaEllipse__re) \
  : "r"(__AreaEllipse__bn), "r"(__AreaEllipse_rp), "r"(__AreaEllipse_xCenter), "r"(__AreaEllipse_yCenter), "r"(__AreaEllipse_a), "r"(__AreaEllipse_b)  \
  : "d1", "a0", "a1", "fp0", "fp1", "cc", "memory"); \
  __AreaEllipse__re; \
  }); \
  _AreaEllipse__re; \
})

#define LoadRGB4(vp, colors, count) ({ \
  struct ViewPort * _LoadRGB4_vp = (vp); \
  CONST UWORD * _LoadRGB4_colors = (colors); \
  ULONG _LoadRGB4_count = (count); \
  { \
  register struct GfxBase * const __LoadRGB4__bn __asm("a6") = (struct GfxBase *) (GRAPHICS_BASE_NAME);\
  register struct ViewPort * __LoadRGB4_vp __asm("a0") = (_LoadRGB4_vp); \
  register CONST UWORD * __LoadRGB4_colors __asm("a1") = (_LoadRGB4_colors); \
  register ULONG __LoadRGB4_count __asm("d0") = (_LoadRGB4_count); \
  __asm volatile ("jsr a6@(-192:W)" \
  : \
  : "r"(__LoadRGB4__bn), "r"(__LoadRGB4_vp), "r"(__LoadRGB4_colors), "r"(__LoadRGB4_count)  \
  : "d0", "d1", "a0", "a1", "fp0", "fp1", "cc", "memory"); \
  } \
})

#define InitRastPort(rp) ({ \
  struct RastPort * _InitRastPort_rp = (rp); \
  { \
  register struct GfxBase * const __InitRastPort__bn __asm("a6") = (struct GfxBase *) (GRAPHICS_BASE_NAME);\
  register struct RastPort * __InitRastPort_rp __asm("a1") = (_InitRastPort_rp); \
  __asm volatile ("jsr a6@(-198:W)" \
  : \
  : "r"(__InitRastPort__bn), "r"(__InitRastPort_rp)  \
  : "d0", "d1", "a0", "a1", "fp0", "fp1", "cc", "memory"); \
  } \
})

#define InitVPort(vp) ({ \
  struct ViewPort * _InitVPort_vp = (vp); \
  { \
  register struct GfxBase * const __InitVPort__bn __asm("a6") = (struct GfxBase *) (GRAPHICS_BASE_NAME);\
  register struct ViewPort * __InitVPort_vp __asm("a0") = (_InitVPort_vp); \
  __asm volatile ("jsr a6@(-204:W)" \
  : \
  : "r"(__InitVPort__bn), "r"(__InitVPort_vp)  \
  : "d0", "d1", "a0", "a1", "fp0", "fp1", "cc", "memory"); \
  } \
})

#define MrgCop(view) ({ \
  struct View * _MrgCop_view = (view); \
  ULONG _MrgCop__re = \
  ({ \
  register struct GfxBase * const __MrgCop__bn __asm("a6") = (struct GfxBase *) (GRAPHICS_BASE_NAME);\
  register ULONG __MrgCop__re __asm("d0"); \
  register struct View * __MrgCop_view __asm("a1") = (_MrgCop_view); \
  __asm volatile ("jsr a6@(-210:W)" \
  : "=r"(__MrgCop__re) \
  : "r"(__MrgCop__bn), "r"(__MrgCop_view)  \
  : "d1", "a0", "a1", "fp0", "fp1", "cc", "memory"); \
  __MrgCop__re; \
  }); \
  _MrgCop__re; \
})

#define MakeVPort(view, vp) ({ \
  struct View * _MakeVPort_view = (view); \
  struct ViewPort * _MakeVPort_vp = (vp); \
  ULONG _MakeVPort__re = \
  ({ \
  register struct GfxBase * const __MakeVPort__bn __asm("a6") = (struct GfxBase *) (GRAPHICS_BASE_NAME);\
  register ULONG __MakeVPort__re __asm("d0"); \
  register struct View * __MakeVPort_view __asm("a0") = (_MakeVPort_view); \
  register struct ViewPort * __MakeVPort_vp __asm("a1") = (_MakeVPort_vp); \
  __asm volatile ("jsr a6@(-216:W)" \
  : "=r"(__MakeVPort__re) \
  : "r"(__MakeVPort__bn), "r"(__MakeVPort_view), "r"(__MakeVPort_vp)  \
  : "d1", "a0", "a1", "fp0", "fp1", "cc", "memory"); \
  __MakeVPort__re; \
  }); \
  _MakeVPort__re; \
})

#define LoadView(view) ({ \
  struct View * _LoadView_view = (view); \
  { \
  register struct GfxBase * const __LoadView__bn __asm("a6") = (struct GfxBase *) (GRAPHICS_BASE_NAME);\
  register struct View * __LoadView_view __asm("a1") = (_LoadView_view); \
  __asm volatile ("jsr a6@(-222:W)" \
  : \
  : "r"(__LoadView__bn), "r"(__LoadView_view)  \
  : "d0", "d1", "a0", "a1", "fp0", "fp1", "cc", "memory"); \
  } \
})

#define WaitBlit() ({ \
  register struct GfxBase * const __WaitBlit__bn __asm("a6") = (struct GfxBase *) (GRAPHICS_BASE_NAME);\
  __asm volatile ("jsr a6@(-228:W)" \
  : \
  : "r"(__WaitBlit__bn)  \
  : "d0", "d1", "a0", "a1", "fp0", "fp1", "cc", "memory"); \
})

#define SetRast(rp, pen) ({ \
  struct RastPort * _SetRast_rp = (rp); \
  ULONG _SetRast_pen = (pen); \
  { \
  register struct GfxBase * const __SetRast__bn __asm("a6") = (struct GfxBase *) (GRAPHICS_BASE_NAME);\
  register struct RastPort * __SetRast_rp __asm("a1") = (_SetRast_rp); \
  register ULONG __SetRast_pen __asm("d0") = (_SetRast_pen); \
  __asm volatile ("jsr a6@(-234:W)" \
  : \
  : "r"(__SetRast__bn), "r"(__SetRast_rp), "r"(__SetRast_pen)  \
  : "d0", "d1", "a0", "a1", "fp0", "fp1", "cc", "memory"); \
  } \
})

#define Move(rp, x, y) ({ \
  struct RastPort * _Move_rp = (rp); \
  LONG _Move_x = (x); \
  LONG _Move_y = (y); \
  { \
  register struct GfxBase * const __Move__bn __asm("a6") = (struct GfxBase *) (GRAPHICS_BASE_NAME);\
  register struct RastPort * __Move_rp __asm("a1") = (_Move_rp); \
  register LONG __Move_x __asm("d0") = (_Move_x); \
  register LONG __Move_y __asm("d1") = (_Move_y); \
  __asm volatile ("jsr a6@(-240:W)" \
  : \
  : "r"(__Move__bn), "r"(__Move_rp), "r"(__Move_x), "r"(__Move_y)  \
  : "d0", "d1", "a0", "a1", "fp0", "fp1", "cc", "memory"); \
  } \
})

#define Draw(rp, x, y) ({ \
  struct RastPort * _Draw_rp = (rp); \
  LONG _Draw_x = (x); \
  LONG _Draw_y = (y); \
  { \
  register struct GfxBase * const __Draw__bn __asm("a6") = (struct GfxBase *) (GRAPHICS_BASE_NAME);\
  register struct RastPort * __Draw_rp __asm("a1") = (_Draw_rp); \
  register LONG __Draw_x __asm("d0") = (_Draw_x); \
  register LONG __Draw_y __asm("d1") = (_Draw_y); \
  __asm volatile ("jsr a6@(-246:W)" \
  : \
  : "r"(__Draw__bn), "r"(__Draw_rp), "r"(__Draw_x), "r"(__Draw_y)  \
  : "d0", "d1", "a0", "a1", "fp0", "fp1", "cc", "memory"); \
  } \
})

#define AreaMove(rp, x, y) ({ \
  struct RastPort * _AreaMove_rp = (rp); \
  LONG _AreaMove_x = (x); \
  LONG _AreaMove_y = (y); \
  LONG _AreaMove__re = \
  ({ \
  register struct GfxBase * const __AreaMove__bn __asm("a6") = (struct GfxBase *) (GRAPHICS_BASE_NAME);\
  register LONG __AreaMove__re __asm("d0"); \
  register struct RastPort * __AreaMove_rp __asm("a1") = (_AreaMove_rp); \
  register LONG __AreaMove_x __asm("d0") = (_AreaMove_x); \
  register LONG __AreaMove_y __asm("d1") = (_AreaMove_y); \
  __asm volatile ("jsr a6@(-252:W)" \
  : "=r"(__AreaMove__re) \
  : "r"(__AreaMove__bn), "r"(__AreaMove_rp), "r"(__AreaMove_x), "r"(__AreaMove_y)  \
  : "d1", "a0", "a1", "fp0", "fp1", "cc", "memory"); \
  __AreaMove__re; \
  }); \
  _AreaMove__re; \
})

#define AreaDraw(rp, x, y) ({ \
  struct RastPort * _AreaDraw_rp = (rp); \
  LONG _AreaDraw_x = (x); \
  LONG _AreaDraw_y = (y); \
  LONG _AreaDraw__re = \
  ({ \
  register struct GfxBase * const __AreaDraw__bn __asm("a6") = (struct GfxBase *) (GRAPHICS_BASE_NAME);\
  register LONG __AreaDraw__re __asm("d0"); \
  register struct RastPort * __AreaDraw_rp __asm("a1") = (_AreaDraw_rp); \
  register LONG __AreaDraw_x __asm("d0") = (_AreaDraw_x); \
  register LONG __AreaDraw_y __asm("d1") = (_AreaDraw_y); \
  __asm volatile ("jsr a6@(-258:W)" \
  : "=r"(__AreaDraw__re) \
  : "r"(__AreaDraw__bn), "r"(__AreaDraw_rp), "r"(__AreaDraw_x), "r"(__AreaDraw_y)  \
  : "d1", "a0", "a1", "fp0", "fp1", "cc", "memory"); \
  __AreaDraw__re; \
  }); \
  _AreaDraw__re; \
})

#define AreaEnd(rp) ({ \
  struct RastPort * _AreaEnd_rp = (rp); \
  LONG _AreaEnd__re = \
  ({ \
  register struct GfxBase * const __AreaEnd__bn __asm("a6") = (struct GfxBase *) (GRAPHICS_BASE_NAME);\
  register LONG __AreaEnd__re __asm("d0"); \
  register struct RastPort * __AreaEnd_rp __asm("a1") = (_AreaEnd_rp); \
  __asm volatile ("jsr a6@(-264:W)" \
  : "=r"(__AreaEnd__re) \
  : "r"(__AreaEnd__bn), "r"(__AreaEnd_rp)  \
  : "d1", "a0", "a1", "fp0", "fp1", "cc", "memory"); \
  __AreaEnd__re; \
  }); \
  _AreaEnd__re; \
})

#define WaitTOF() ({ \
  register struct GfxBase * const __WaitTOF__bn __asm("a6") = (struct GfxBase *) (GRAPHICS_BASE_NAME);\
  __asm volatile ("jsr a6@(-270:W)" \
  : \
  : "r"(__WaitTOF__bn)  \
  : "d0", "d1", "a0", "a1", "fp0", "fp1", "cc", "memory"); \
})

#define QBlit(blit) ({ \
  struct bltnode * _QBlit_blit = (blit); \
  { \
  register struct GfxBase * const __QBlit__bn __asm("a6") = (struct GfxBase *) (GRAPHICS_BASE_NAME);\
  register struct bltnode * __QBlit_blit __asm("a1") = (_QBlit_blit); \
  __asm volatile ("jsr a6@(-276:W)" \
  : \
  : "r"(__QBlit__bn), "r"(__QBlit_blit)  \
  : "d0", "d1", "a0", "a1", "fp0", "fp1", "cc", "memory"); \
  } \
})

#define InitArea(areaInfo, vectorBuffer, maxVectors) ({ \
  struct AreaInfo * _InitArea_areaInfo = (areaInfo); \
  APTR _InitArea_vectorBuffer = (vectorBuffer); \
  LONG _InitArea_maxVectors = (maxVectors); \
  { \
  register struct GfxBase * const __InitArea__bn __asm("a6") = (struct GfxBase *) (GRAPHICS_BASE_NAME);\
  register struct AreaInfo * __InitArea_areaInfo __asm("a0") = (_InitArea_areaInfo); \
  register APTR __InitArea_vectorBuffer __asm("a1") = (_InitArea_vectorBuffer); \
  register LONG __InitArea_maxVectors __asm("d0") = (_InitArea_maxVectors); \
  __asm volatile ("jsr a6@(-282:W)" \
  : \
  : "r"(__InitArea__bn), "r"(__InitArea_areaInfo), "r"(__InitArea_vectorBuffer), "r"(__InitArea_maxVectors)  \
  : "d0", "d1", "a0", "a1", "fp0", "fp1", "cc", "memory"); \
  } \
})

#define SetRGB4(vp, colindex, red, green, blue) ({ \
  struct ViewPort * _SetRGB4_vp = (vp); \
  ULONG _SetRGB4_colindex = (colindex); \
  ULONG _SetRGB4_red = (red); \
  ULONG _SetRGB4_green = (green); \
  ULONG _SetRGB4_blue = (blue); \
  { \
  register struct GfxBase * const __SetRGB4__bn __asm("a6") = (struct GfxBase *) (GRAPHICS_BASE_NAME);\
  register struct ViewPort * __SetRGB4_vp __asm("a0") = (_SetRGB4_vp); \
  register ULONG __SetRGB4_colindex __asm("d0") = (_SetRGB4_colindex); \
  register ULONG __SetRGB4_red __asm("d1") = (_SetRGB4_red); \
  register ULONG __SetRGB4_green __asm("d2") = (_SetRGB4_green); \
  register ULONG __SetRGB4_blue __asm("d3") = (_SetRGB4_blue); \
  __asm volatile ("jsr a6@(-288:W)" \
  : \
  : "r"(__SetRGB4__bn), "r"(__SetRGB4_vp), "r"(__SetRGB4_colindex), "r"(__SetRGB4_red), "r"(__SetRGB4_green), "r"(__SetRGB4_blue)  \
  : "d0", "d1", "a0", "a1", "fp0", "fp1", "cc", "memory"); \
  } \
})

#define QBSBlit(blit) ({ \
  struct bltnode * _QBSBlit_blit = (blit); \
  { \
  register struct GfxBase * const __QBSBlit__bn __asm("a6") = (struct GfxBase *) (GRAPHICS_BASE_NAME);\
  register struct bltnode * __QBSBlit_blit __asm("a1") = (_QBSBlit_blit); \
  __asm volatile ("jsr a6@(-294:W)" \
  : \
  : "r"(__QBSBlit__bn), "r"(__QBSBlit_blit)  \
  : "d0", "d1", "a0", "a1", "fp0", "fp1", "cc", "memory"); \
  } \
})

#define BltClear(memBlock, byteCount, flags) ({ \
  PLANEPTR _BltClear_memBlock = (memBlock); \
  ULONG _BltClear_byteCount = (byteCount); \
  ULONG _BltClear_flags = (flags); \
  { \
  register struct GfxBase * const __BltClear__bn __asm("a6") = (struct GfxBase *) (GRAPHICS_BASE_NAME);\
  register PLANEPTR __BltClear_memBlock __asm("a1") = (_BltClear_memBlock); \
  register ULONG __BltClear_byteCount __asm("d0") = (_BltClear_byteCount); \
  register ULONG __BltClear_flags __asm("d1") = (_BltClear_flags); \
  __asm volatile ("jsr a6@(-300:W)" \
  : \
  : "r"(__BltClear__bn), "r"(__BltClear_memBlock), "r"(__BltClear_byteCount), "r"(__BltClear_flags)  \
  : "d0", "d1", "a0", "a1", "fp0", "fp1", "cc", "memory"); \
  } \
})

#define RectFill(rp, xMin, yMin, xMax, yMax) ({ \
  struct RastPort * _RectFill_rp = (rp); \
  LONG _RectFill_xMin = (xMin); \
  LONG _RectFill_yMin = (yMin); \
  LONG _RectFill_xMax = (xMax); \
  LONG _RectFill_yMax = (yMax); \
  { \
  register struct GfxBase * const __RectFill__bn __asm("a6") = (struct GfxBase *) (GRAPHICS_BASE_NAME);\
  register struct RastPort * __RectFill_rp __asm("a1") = (_RectFill_rp); \
  register LONG __RectFill_xMin __asm("d0") = (_RectFill_xMin); \
  register LONG __RectFill_yMin __asm("d1") = (_RectFill_yMin); \
  register LONG __RectFill_xMax __asm("d2") = (_RectFill_xMax); \
  register LONG __RectFill_yMax __asm("d3") = (_RectFill_yMax); \
  __asm volatile ("jsr a6@(-306:W)" \
  : \
  : "r"(__RectFill__bn), "r"(__RectFill_rp), "r"(__RectFill_xMin), "r"(__RectFill_yMin), "r"(__RectFill_xMax), "r"(__RectFill_yMax)  \
  : "d0", "d1", "a0", "a1", "fp0", "fp1", "cc", "memory"); \
  } \
})

#define BltPattern(rp, mask, xMin, yMin, xMax, yMax, maskBPR) ({ \
  struct RastPort * _BltPattern_rp = (rp); \
  CONST PLANEPTR _BltPattern_mask = (mask); \
  LONG _BltPattern_xMin = (xMin); \
  LONG _BltPattern_yMin = (yMin); \
  LONG _BltPattern_xMax = (xMax); \
  LONG _BltPattern_yMax = (yMax); \
  ULONG _BltPattern_maskBPR = (maskBPR); \
  { \
  register struct GfxBase * const __BltPattern__bn __asm("a6") = (struct GfxBase *) (GRAPHICS_BASE_NAME);\
  register struct RastPort * __BltPattern_rp __asm("a1") = (_BltPattern_rp); \
  register CONST PLANEPTR __BltPattern_mask __asm("a0") = (_BltPattern_mask); \
  register LONG __BltPattern_xMin __asm("d0") = (_BltPattern_xMin); \
  register LONG __BltPattern_yMin __asm("d1") = (_BltPattern_yMin); \
  register LONG __BltPattern_xMax __asm("d2") = (_BltPattern_xMax); \
  register LONG __BltPattern_yMax __asm("d3") = (_BltPattern_yMax); \
  register ULONG __BltPattern_maskBPR __asm("d4") = (_BltPattern_maskBPR); \
  __asm volatile ("jsr a6@(-312:W)" \
  : \
  : "r"(__BltPattern__bn), "r"(__BltPattern_rp), "r"(__BltPattern_mask), "r"(__BltPattern_xMin), "r"(__BltPattern_yMin), "r"(__BltPattern_xMax), "r"(__BltPattern_yMax), "r"(__BltPattern_maskBPR)  \
  : "d0", "d1", "a0", "a1", "fp0", "fp1", "cc", "memory"); \
  } \
})

#define ReadPixel(rp, x, y) ({ \
  struct RastPort * _ReadPixel_rp = (rp); \
  LONG _ReadPixel_x = (x); \
  LONG _ReadPixel_y = (y); \
  LONG _ReadPixel__re = \
  ({ \
  register struct GfxBase * const __ReadPixel__bn __asm("a6") = (struct GfxBase *) (GRAPHICS_BASE_NAME);\
  register LONG __ReadPixel__re __asm("d0"); \
  register struct RastPort * __ReadPixel_rp __asm("a1") = (_ReadPixel_rp); \
  register LONG __ReadPixel_x __asm("d0") = (_ReadPixel_x); \
  register LONG __ReadPixel_y __asm("d1") = (_ReadPixel_y); \
  __asm volatile ("jsr a6@(-318:W)" \
  : "=r"(__ReadPixel__re) \
  : "r"(__ReadPixel__bn), "r"(__ReadPixel_rp), "r"(__ReadPixel_x), "r"(__ReadPixel_y)  \
  : "d1", "a0", "a1", "fp0", "fp1", "cc", "memory"); \
  __ReadPixel__re; \
  }); \
  _ReadPixel__re; \
})

#define WritePixel(rp, x, y) ({ \
  struct RastPort * _WritePixel_rp = (rp); \
  LONG _WritePixel_x = (x); \
  LONG _WritePixel_y = (y); \
  LONG _WritePixel__re = \
  ({ \
  register struct GfxBase * const __WritePixel__bn __asm("a6") = (struct GfxBase *) (GRAPHICS_BASE_NAME);\
  register LONG __WritePixel__re __asm("d0"); \
  register struct RastPort * __WritePixel_rp __asm("a1") = (_WritePixel_rp); \
  register LONG __WritePixel_x __asm("d0") = (_WritePixel_x); \
  register LONG __WritePixel_y __asm("d1") = (_WritePixel_y); \
  __asm volatile ("jsr a6@(-324:W)" \
  : "=r"(__WritePixel__re) \
  : "r"(__WritePixel__bn), "r"(__WritePixel_rp), "r"(__WritePixel_x), "r"(__WritePixel_y)  \
  : "d1", "a0", "a1", "fp0", "fp1", "cc", "memory"); \
  __WritePixel__re; \
  }); \
  _WritePixel__re; \
})

#define Flood(rp, mode, x, y) ({ \
  struct RastPort * _Flood_rp = (rp); \
  ULONG _Flood_mode = (mode); \
  LONG _Flood_x = (x); \
  LONG _Flood_y = (y); \
  BOOL _Flood__re = \
  ({ \
  register struct GfxBase * const __Flood__bn __asm("a6") = (struct GfxBase *) (GRAPHICS_BASE_NAME);\
  register BOOL __Flood__re __asm("d0"); \
  register struct RastPort * __Flood_rp __asm("a1") = (_Flood_rp); \
  register ULONG __Flood_mode __asm("d2") = (_Flood_mode); \
  register LONG __Flood_x __asm("d0") = (_Flood_x); \
  register LONG __Flood_y __asm("d1") = (_Flood_y); \
  __asm volatile ("jsr a6@(-330:W)" \
  : "=r"(__Flood__re) \
  : "r"(__Flood__bn), "r"(__Flood_rp), "r"(__Flood_mode), "r"(__Flood_x), "r"(__Flood_y)  \
  : "d1", "a0", "a1", "fp0", "fp1", "cc", "memory"); \
  __Flood__re; \
  }); \
  _Flood__re; \
})

#define PolyDraw(rp, count, polyTable) ({ \
  struct RastPort * _PolyDraw_rp = (rp); \
  LONG _PolyDraw_count = (count); \
  CONST WORD * _PolyDraw_polyTable = (polyTable); \
  { \
  register struct GfxBase * const __PolyDraw__bn __asm("a6") = (struct GfxBase *) (GRAPHICS_BASE_NAME);\
  register struct RastPort * __PolyDraw_rp __asm("a1") = (_PolyDraw_rp); \
  register LONG __PolyDraw_count __asm("d0") = (_PolyDraw_count); \
  register CONST WORD * __PolyDraw_polyTable __asm("a0") = (_PolyDraw_polyTable); \
  __asm volatile ("jsr a6@(-336:W)" \
  : \
  : "r"(__PolyDraw__bn), "r"(__PolyDraw_rp), "r"(__PolyDraw_count), "r"(__PolyDraw_polyTable)  \
  : "d0", "d1", "a0", "a1", "fp0", "fp1", "cc", "memory"); \
  } \
})

#define SetAPen(rp, pen) ({ \
  struct RastPort * _SetAPen_rp = (rp); \
  ULONG _SetAPen_pen = (pen); \
  { \
  register struct GfxBase * const __SetAPen__bn __asm("a6") = (struct GfxBase *) (GRAPHICS_BASE_NAME);\
  register struct RastPort * __SetAPen_rp __asm("a1") = (_SetAPen_rp); \
  register ULONG __SetAPen_pen __asm("d0") = (_SetAPen_pen); \
  __asm volatile ("jsr a6@(-342:W)" \
  : \
  : "r"(__SetAPen__bn), "r"(__SetAPen_rp), "r"(__SetAPen_pen)  \
  : "d0", "d1", "a0", "a1", "fp0", "fp1", "cc", "memory"); \
  } \
})

#define SetBPen(rp, pen) ({ \
  struct RastPort * _SetBPen_rp = (rp); \
  ULONG _SetBPen_pen = (pen); \
  { \
  register struct GfxBase * const __SetBPen__bn __asm("a6") = (struct GfxBase *) (GRAPHICS_BASE_NAME);\
  register struct RastPort * __SetBPen_rp __asm("a1") = (_SetBPen_rp); \
  register ULONG __SetBPen_pen __asm("d0") = (_SetBPen_pen); \
  __asm volatile ("jsr a6@(-348:W)" \
  : \
  : "r"(__SetBPen__bn), "r"(__SetBPen_rp), "r"(__SetBPen_pen)  \
  : "d0", "d1", "a0", "a1", "fp0", "fp1", "cc", "memory"); \
  } \
})

#define SetDrMd(rp, drawMode) ({ \
  struct RastPort * _SetDrMd_rp = (rp); \
  ULONG _SetDrMd_drawMode = (drawMode); \
  { \
  register struct GfxBase * const __SetDrMd__bn __asm("a6") = (struct GfxBase *) (GRAPHICS_BASE_NAME);\
  register struct RastPort * __SetDrMd_rp __asm("a1") = (_SetDrMd_rp); \
  register ULONG __SetDrMd_drawMode __asm("d0") = (_SetDrMd_drawMode); \
  __asm volatile ("jsr a6@(-354:W)" \
  : \
  : "r"(__SetDrMd__bn), "r"(__SetDrMd_rp), "r"(__SetDrMd_drawMode)  \
  : "d0", "d1", "a0", "a1", "fp0", "fp1", "cc", "memory"); \
  } \
})

#define InitView(view) ({ \
  struct View * _InitView_view = (view); \
  { \
  register struct GfxBase * const __InitView__bn __asm("a6") = (struct GfxBase *) (GRAPHICS_BASE_NAME);\
  register struct View * __InitView_view __asm("a1") = (_InitView_view); \
  __asm volatile ("jsr a6@(-360:W)" \
  : \
  : "r"(__InitView__bn), "r"(__InitView_view)  \
  : "d0", "d1", "a0", "a1", "fp0", "fp1", "cc", "memory"); \
  } \
})

#define CBump(copList) ({ \
  struct UCopList * _CBump_copList = (copList); \
  { \
  register struct GfxBase * const __CBump__bn __asm("a6") = (struct GfxBase *) (GRAPHICS_BASE_NAME);\
  register struct UCopList * __CBump_copList __asm("a1") = (_CBump_copList); \
  __asm volatile ("jsr a6@(-366:W)" \
  : \
  : "r"(__CBump__bn), "r"(__CBump_copList)  \
  : "d0", "d1", "a0", "a1", "fp0", "fp1", "cc", "memory"); \
  } \
})

#define CMove(copList, destoffset, data) ({ \
  struct UCopList * _CMove_copList = (copList); \
  LONG _CMove_destoffset = (destoffset); \
  LONG _CMove_data = (data); \
  { \
  register struct GfxBase * const __CMove__bn __asm("a6") = (struct GfxBase *) (GRAPHICS_BASE_NAME);\
  register struct UCopList * __CMove_copList __asm("a1") = (_CMove_copList); \
  register LONG __CMove_destoffset __asm("d0") = (_CMove_destoffset); \
  register LONG __CMove_data __asm("d1") = (_CMove_data); \
  __asm volatile ("jsr a6@(-372:W)" \
  : \
  : "r"(__CMove__bn), "r"(__CMove_copList), "r"(__CMove_destoffset), "r"(__CMove_data)  \
  : "d0", "d1", "a0", "a1", "fp0", "fp1", "cc", "memory"); \
  } \
})

#define CWait(copList, v, h) ({ \
  struct UCopList * _CWait_copList = (copList); \
  LONG _CWait_v = (v); \
  LONG _CWait_h = (h); \
  { \
  register struct GfxBase * const __CWait__bn __asm("a6") = (struct GfxBase *) (GRAPHICS_BASE_NAME);\
  register struct UCopList * __CWait_copList __asm("a1") = (_CWait_copList); \
  register LONG __CWait_v __asm("d0") = (_CWait_v); \
  register LONG __CWait_h __asm("d1") = (_CWait_h); \
  __asm volatile ("jsr a6@(-378:W)" \
  : \
  : "r"(__CWait__bn), "r"(__CWait_copList), "r"(__CWait_v), "r"(__CWait_h)  \
  : "d0", "d1", "a0", "a1", "fp0", "fp1", "cc", "memory"); \
  } \
})

#define VBeamPos() ({ \
  LONG _VBeamPos__re = \
  ({ \
  register struct GfxBase * const __VBeamPos__bn __asm("a6") = (struct GfxBase *) (GRAPHICS_BASE_NAME);\
  register LONG __VBeamPos__re __asm("d0"); \
  __asm volatile ("jsr a6@(-384:W)" \
  : "=r"(__VBeamPos__re) \
  : "r"(__VBeamPos__bn)  \
  : "d1", "a0", "a1", "fp0", "fp1", "cc", "memory"); \
  __VBeamPos__re; \
  }); \
  _VBeamPos__re; \
})

#define InitBitMap(bitMap, depth, width, height) ({ \
  struct BitMap * _InitBitMap_bitMap = (bitMap); \
  LONG _InitBitMap_depth = (depth); \
  ULONG _InitBitMap_width = (width); \
  ULONG _InitBitMap_height = (height); \
  { \
  register struct GfxBase * const __InitBitMap__bn __asm("a6") = (struct GfxBase *) (GRAPHICS_BASE_NAME);\
  register struct BitMap * __InitBitMap_bitMap __asm("a0") = (_InitBitMap_bitMap); \
  register LONG __InitBitMap_depth __asm("d0") = (_InitBitMap_depth); \
  register ULONG __InitBitMap_width __asm("d1") = (_InitBitMap_width); \
  register ULONG __InitBitMap_height __asm("d2") = (_InitBitMap_height); \
  __asm volatile ("jsr a6@(-390:W)" \
  : \
  : "r"(__InitBitMap__bn), "r"(__InitBitMap_bitMap), "r"(__InitBitMap_depth), "r"(__InitBitMap_width), "r"(__InitBitMap_height)  \
  : "d0", "d1", "a0", "a1", "fp0", "fp1", "cc", "memory"); \
  } \
})

#define ScrollRaster(rp, dx, dy, xMin, yMin, xMax, yMax) ({ \
  struct RastPort * _ScrollRaster_rp = (rp); \
  LONG _ScrollRaster_dx = (dx); \
  LONG _ScrollRaster_dy = (dy); \
  LONG _ScrollRaster_xMin = (xMin); \
  LONG _ScrollRaster_yMin = (yMin); \
  LONG _ScrollRaster_xMax = (xMax); \
  LONG _ScrollRaster_yMax = (yMax); \
  { \
  register struct GfxBase * const __ScrollRaster__bn __asm("a6") = (struct GfxBase *) (GRAPHICS_BASE_NAME);\
  register struct RastPort * __ScrollRaster_rp __asm("a1") = (_ScrollRaster_rp); \
  register LONG __ScrollRaster_dx __asm("d0") = (_ScrollRaster_dx); \
  register LONG __ScrollRaster_dy __asm("d1") = (_ScrollRaster_dy); \
  register LONG __ScrollRaster_xMin __asm("d2") = (_ScrollRaster_xMin); \
  register LONG __ScrollRaster_yMin __asm("d3") = (_ScrollRaster_yMin); \
  register LONG __ScrollRaster_xMax __asm("d4") = (_ScrollRaster_xMax); \
  register LONG __ScrollRaster_yMax __asm("d5") = (_ScrollRaster_yMax); \
  __asm volatile ("jsr a6@(-396:W)" \
  : \
  : "r"(__ScrollRaster__bn), "r"(__ScrollRaster_rp), "r"(__ScrollRaster_dx), "r"(__ScrollRaster_dy), "r"(__ScrollRaster_xMin), "r"(__ScrollRaster_yMin), "r"(__ScrollRaster_xMax), "r"(__ScrollRaster_yMax)  \
  : "d0", "d1", "a0", "a1", "fp0", "fp1", "cc", "memory"); \
  } \
})

#define WaitBOVP(vp) ({ \
  struct ViewPort * _WaitBOVP_vp = (vp); \
  { \
  register struct GfxBase * const __WaitBOVP__bn __asm("a6") = (struct GfxBase *) (GRAPHICS_BASE_NAME);\
  register struct ViewPort * __WaitBOVP_vp __asm("a0") = (_WaitBOVP_vp); \
  __asm volatile ("jsr a6@(-402:W)" \
  : \
  : "r"(__WaitBOVP__bn), "r"(__WaitBOVP_vp)  \
  : "d0", "d1", "a0", "a1", "fp0", "fp1", "cc", "memory"); \
  } \
})

#define GetSprite(sprite, num) ({ \
  struct SimpleSprite * _GetSprite_sprite = (sprite); \
  LONG _GetSprite_num = (num); \
  WORD _GetSprite__re = \
  ({ \
  register struct GfxBase * const __GetSprite__bn __asm("a6") = (struct GfxBase *) (GRAPHICS_BASE_NAME);\
  register WORD __GetSprite__re __asm("d0"); \
  register struct SimpleSprite * __GetSprite_sprite __asm("a0") = (_GetSprite_sprite); \
  register LONG __GetSprite_num __asm("d0") = (_GetSprite_num); \
  __asm volatile ("jsr a6@(-408:W)" \
  : "=r"(__GetSprite__re) \
  : "r"(__GetSprite__bn), "r"(__GetSprite_sprite), "r"(__GetSprite_num)  \
  : "d1", "a0", "a1", "fp0", "fp1", "cc", "memory"); \
  __GetSprite__re; \
  }); \
  _GetSprite__re; \
})

#define FreeSprite(num) ({ \
  LONG _FreeSprite_num = (num); \
  { \
  register struct GfxBase * const __FreeSprite__bn __asm("a6") = (struct GfxBase *) (GRAPHICS_BASE_NAME);\
  register LONG __FreeSprite_num __asm("d0") = (_FreeSprite_num); \
  __asm volatile ("jsr a6@(-414:W)" \
  : \
  : "r"(__FreeSprite__bn), "r"(__FreeSprite_num)  \
  : "d0", "d1", "a0", "a1", "fp0", "fp1", "cc", "memory"); \
  } \
})

#define ChangeSprite(vp, sprite, newData) ({ \
  struct ViewPort * _ChangeSprite_vp = (vp); \
  struct SimpleSprite * _ChangeSprite_sprite = (sprite); \
  APTR _ChangeSprite_newData = (newData); \
  { \
  register struct GfxBase * const __ChangeSprite__bn __asm("a6") = (struct GfxBase *) (GRAPHICS_BASE_NAME);\
  register struct ViewPort * __ChangeSprite_vp __asm("a0") = (_ChangeSprite_vp); \
  register struct SimpleSprite * __ChangeSprite_sprite __asm("a1") = (_ChangeSprite_sprite); \
  register APTR __ChangeSprite_newData __asm("a2") = (_ChangeSprite_newData); \
  __asm volatile ("jsr a6@(-420:W)" \
  : \
  : "r"(__ChangeSprite__bn), "r"(__ChangeSprite_vp), "r"(__ChangeSprite_sprite), "r"(__ChangeSprite_newData)  \
  : "d0", "d1", "a0", "a1", "fp0", "fp1", "cc", "memory"); \
  } \
})

#define MoveSprite(vp, sprite, x, y) ({ \
  struct ViewPort * _MoveSprite_vp = (vp); \
  struct SimpleSprite * _MoveSprite_sprite = (sprite); \
  LONG _MoveSprite_x = (x); \
  LONG _MoveSprite_y = (y); \
  { \
  register struct GfxBase * const __MoveSprite__bn __asm("a6") = (struct GfxBase *) (GRAPHICS_BASE_NAME);\
  register struct ViewPort * __MoveSprite_vp __asm("a0") = (_MoveSprite_vp); \
  register struct SimpleSprite * __MoveSprite_sprite __asm("a1") = (_MoveSprite_sprite); \
  register LONG __MoveSprite_x __asm("d0") = (_MoveSprite_x); \
  register LONG __MoveSprite_y __asm("d1") = (_MoveSprite_y); \
  __asm volatile ("jsr a6@(-426:W)" \
  : \
  : "r"(__MoveSprite__bn), "r"(__MoveSprite_vp), "r"(__MoveSprite_sprite), "r"(__MoveSprite_x), "r"(__MoveSprite_y)  \
  : "d0", "d1", "a0", "a1", "fp0", "fp1", "cc", "memory"); \
  } \
})

#define LockLayerRom(layer) ({ \
  struct Layer * _LockLayerRom_layer = (layer); \
  { \
  register struct GfxBase * const __LockLayerRom__bn __asm("a6") = (struct GfxBase *) (GRAPHICS_BASE_NAME);\
  register struct Layer * __LockLayerRom_layer __asm("d2") = (_LockLayerRom_layer); \
  __asm volatile ("exg a5,d2\n\tjsr a6@(-432:W)\n\texg a5,d2" \
  : \
  : "r"(__LockLayerRom__bn), "r"(__LockLayerRom_layer)  \
  : "d0", "d1", "a0", "a1", "fp0", "fp1", "cc", "memory"); \
  } \
})

#define UnlockLayerRom(layer) ({ \
  struct Layer * _UnlockLayerRom_layer = (layer); \
  { \
  register struct GfxBase * const __UnlockLayerRom__bn __asm("a6") = (struct GfxBase *) (GRAPHICS_BASE_NAME);\
  register struct Layer * __UnlockLayerRom_layer __asm("d2") = (_UnlockLayerRom_layer); \
  __asm volatile ("exg a5,d2\n\tjsr a6@(-438:W)\n\texg a5,d2" \
  : \
  : "r"(__UnlockLayerRom__bn), "r"(__UnlockLayerRom_layer)  \
  : "d0", "d1", "a0", "a1", "fp0", "fp1", "cc", "memory"); \
  } \
})

#define SyncSBitMap(layer) ({ \
  struct Layer * _SyncSBitMap_layer = (layer); \
  { \
  register struct GfxBase * const __SyncSBitMap__bn __asm("a6") = (struct GfxBase *) (GRAPHICS_BASE_NAME);\
  register struct Layer * __SyncSBitMap_layer __asm("a0") = (_SyncSBitMap_layer); \
  __asm volatile ("jsr a6@(-444:W)" \
  : \
  : "r"(__SyncSBitMap__bn), "r"(__SyncSBitMap_layer)  \
  : "d0", "d1", "a0", "a1", "fp0", "fp1", "cc", "memory"); \
  } \
})

#define CopySBitMap(layer) ({ \
  struct Layer * _CopySBitMap_layer = (layer); \
  { \
  register struct GfxBase * const __CopySBitMap__bn __asm("a6") = (struct GfxBase *) (GRAPHICS_BASE_NAME);\
  register struct Layer * __CopySBitMap_layer __asm("a0") = (_CopySBitMap_layer); \
  __asm volatile ("jsr a6@(-450:W)" \
  : \
  : "r"(__CopySBitMap__bn), "r"(__CopySBitMap_layer)  \
  : "d0", "d1", "a0", "a1", "fp0", "fp1", "cc", "memory"); \
  } \
})

#define OwnBlitter() ({ \
  register struct GfxBase * const __OwnBlitter__bn __asm("a6") = (struct GfxBase *) (GRAPHICS_BASE_NAME);\
  __asm volatile ("jsr a6@(-456:W)" \
  : \
  : "r"(__OwnBlitter__bn)  \
  : "d0", "d1", "a0", "a1", "fp0", "fp1", "cc", "memory"); \
})

#define DisownBlitter() ({ \
  register struct GfxBase * const __DisownBlitter__bn __asm("a6") = (struct GfxBase *) (GRAPHICS_BASE_NAME);\
  __asm volatile ("jsr a6@(-462:W)" \
  : \
  : "r"(__DisownBlitter__bn)  \
  : "d0", "d1", "a0", "a1", "fp0", "fp1", "cc", "memory"); \
})

#define InitTmpRas(tmpRas, buffer, size) ({ \
  struct TmpRas * _InitTmpRas_tmpRas = (tmpRas); \
  PLANEPTR _InitTmpRas_buffer = (buffer); \
  LONG _InitTmpRas_size = (size); \
  struct TmpRas * _InitTmpRas__re = \
  ({ \
  register struct GfxBase * const __InitTmpRas__bn __asm("a6") = (struct GfxBase *) (GRAPHICS_BASE_NAME);\
  register struct TmpRas * __InitTmpRas__re __asm("d0"); \
  register struct TmpRas * __InitTmpRas_tmpRas __asm("a0") = (_InitTmpRas_tmpRas); \
  register PLANEPTR __InitTmpRas_buffer __asm("a1") = (_InitTmpRas_buffer); \
  register LONG __InitTmpRas_size __asm("d0") = (_InitTmpRas_size); \
  __asm volatile ("jsr a6@(-468:W)" \
  : "=r"(__InitTmpRas__re) \
  : "r"(__InitTmpRas__bn), "r"(__InitTmpRas_tmpRas), "r"(__InitTmpRas_buffer), "r"(__InitTmpRas_size)  \
  : "d1", "a0", "a1", "fp0", "fp1", "cc", "memory"); \
  __InitTmpRas__re; \
  }); \
  _InitTmpRas__re; \
})

#define AskFont(rp, textAttr) ({ \
  struct RastPort * _AskFont_rp = (rp); \
  struct TextAttr * _AskFont_textAttr = (textAttr); \
  { \
  register struct GfxBase * const __AskFont__bn __asm("a6") = (struct GfxBase *) (GRAPHICS_BASE_NAME);\
  register struct RastPort * __AskFont_rp __asm("a1") = (_AskFont_rp); \
  register struct TextAttr * __AskFont_textAttr __asm("a0") = (_AskFont_textAttr); \
  __asm volatile ("jsr a6@(-474:W)" \
  : \
  : "r"(__AskFont__bn), "r"(__AskFont_rp), "r"(__AskFont_textAttr)  \
  : "d0", "d1", "a0", "a1", "fp0", "fp1", "cc", "memory"); \
  } \
})

#define AddFont(textFont) ({ \
  struct TextFont * _AddFont_textFont = (textFont); \
  { \
  register struct GfxBase * const __AddFont__bn __asm("a6") = (struct GfxBase *) (GRAPHICS_BASE_NAME);\
  register struct TextFont * __AddFont_textFont __asm("a1") = (_AddFont_textFont); \
  __asm volatile ("jsr a6@(-480:W)" \
  : \
  : "r"(__AddFont__bn), "r"(__AddFont_textFont)  \
  : "d0", "d1", "a0", "a1", "fp0", "fp1", "cc", "memory"); \
  } \
})

#define RemFont(textFont) ({ \
  struct TextFont * _RemFont_textFont = (textFont); \
  { \
  register struct GfxBase * const __RemFont__bn __asm("a6") = (struct GfxBase *) (GRAPHICS_BASE_NAME);\
  register struct TextFont * __RemFont_textFont __asm("a1") = (_RemFont_textFont); \
  __asm volatile ("jsr a6@(-486:W)" \
  : \
  : "r"(__RemFont__bn), "r"(__RemFont_textFont)  \
  : "d0", "d1", "a0", "a1", "fp0", "fp1", "cc", "memory"); \
  } \
})

#define AllocRaster(width, height) ({ \
  ULONG _AllocRaster_width = (width); \
  ULONG _AllocRaster_height = (height); \
  PLANEPTR _AllocRaster__re = \
  ({ \
  register struct GfxBase * const __AllocRaster__bn __asm("a6") = (struct GfxBase *) (GRAPHICS_BASE_NAME);\
  register PLANEPTR __AllocRaster__re __asm("d0"); \
  register ULONG __AllocRaster_width __asm("d0") = (_AllocRaster_width); \
  register ULONG __AllocRaster_height __asm("d1") = (_AllocRaster_height); \
  __asm volatile ("jsr a6@(-492:W)" \
  : "=r"(__AllocRaster__re) \
  : "r"(__AllocRaster__bn), "r"(__AllocRaster_width), "r"(__AllocRaster_height)  \
  : "d1", "a0", "a1", "fp0", "fp1", "cc", "memory"); \
  __AllocRaster__re; \
  }); \
  _AllocRaster__re; \
})

#define FreeRaster(p, width, height) ({ \
  PLANEPTR _FreeRaster_p = (p); \
  ULONG _FreeRaster_width = (width); \
  ULONG _FreeRaster_height = (height); \
  { \
  register struct GfxBase * const __FreeRaster__bn __asm("a6") = (struct GfxBase *) (GRAPHICS_BASE_NAME);\
  register PLANEPTR __FreeRaster_p __asm("a0") = (_FreeRaster_p); \
  register ULONG __FreeRaster_width __asm("d0") = (_FreeRaster_width); \
  register ULONG __FreeRaster_height __asm("d1") = (_FreeRaster_height); \
  __asm volatile ("jsr a6@(-498:W)" \
  : \
  : "r"(__FreeRaster__bn), "r"(__FreeRaster_p), "r"(__FreeRaster_width), "r"(__FreeRaster_height)  \
  : "d0", "d1", "a0", "a1", "fp0", "fp1", "cc", "memory"); \
  } \
})

#define AndRectRegion(region, rectangle) ({ \
  struct Region * _AndRectRegion_region = (region); \
  CONST struct Rectangle * _AndRectRegion_rectangle = (rectangle); \
  { \
  register struct GfxBase * const __AndRectRegion__bn __asm("a6") = (struct GfxBase *) (GRAPHICS_BASE_NAME);\
  register struct Region * __AndRectRegion_region __asm("a0") = (_AndRectRegion_region); \
  register CONST struct Rectangle * __AndRectRegion_rectangle __asm("a1") = (_AndRectRegion_rectangle); \
  __asm volatile ("jsr a6@(-504:W)" \
  : \
  : "r"(__AndRectRegion__bn), "r"(__AndRectRegion_region), "r"(__AndRectRegion_rectangle)  \
  : "d0", "d1", "a0", "a1", "fp0", "fp1", "cc", "memory"); \
  } \
})

#define OrRectRegion(region, rectangle) ({ \
  struct Region * _OrRectRegion_region = (region); \
  CONST struct Rectangle * _OrRectRegion_rectangle = (rectangle); \
  BOOL _OrRectRegion__re = \
  ({ \
  register struct GfxBase * const __OrRectRegion__bn __asm("a6") = (struct GfxBase *) (GRAPHICS_BASE_NAME);\
  register BOOL __OrRectRegion__re __asm("d0"); \
  register struct Region * __OrRectRegion_region __asm("a0") = (_OrRectRegion_region); \
  register CONST struct Rectangle * __OrRectRegion_rectangle __asm("a1") = (_OrRectRegion_rectangle); \
  __asm volatile ("jsr a6@(-510:W)" \
  : "=r"(__OrRectRegion__re) \
  : "r"(__OrRectRegion__bn), "r"(__OrRectRegion_region), "r"(__OrRectRegion_rectangle)  \
  : "d1", "a0", "a1", "fp0", "fp1", "cc", "memory"); \
  __OrRectRegion__re; \
  }); \
  _OrRectRegion__re; \
})

#define NewRegion() ({ \
  struct Region * _NewRegion__re = \
  ({ \
  register struct GfxBase * const __NewRegion__bn __asm("a6") = (struct GfxBase *) (GRAPHICS_BASE_NAME);\
  register struct Region * __NewRegion__re __asm("d0"); \
  __asm volatile ("jsr a6@(-516:W)" \
  : "=r"(__NewRegion__re) \
  : "r"(__NewRegion__bn)  \
  : "d1", "a0", "a1", "fp0", "fp1", "cc", "memory"); \
  __NewRegion__re; \
  }); \
  _NewRegion__re; \
})

#define ClearRectRegion(region, rectangle) ({ \
  struct Region * _ClearRectRegion_region = (region); \
  CONST struct Rectangle * _ClearRectRegion_rectangle = (rectangle); \
  BOOL _ClearRectRegion__re = \
  ({ \
  register struct GfxBase * const __ClearRectRegion__bn __asm("a6") = (struct GfxBase *) (GRAPHICS_BASE_NAME);\
  register BOOL __ClearRectRegion__re __asm("d0"); \
  register struct Region * __ClearRectRegion_region __asm("a0") = (_ClearRectRegion_region); \
  register CONST struct Rectangle * __ClearRectRegion_rectangle __asm("a1") = (_ClearRectRegion_rectangle); \
  __asm volatile ("jsr a6@(-522:W)" \
  : "=r"(__ClearRectRegion__re) \
  : "r"(__ClearRectRegion__bn), "r"(__ClearRectRegion_region), "r"(__ClearRectRegion_rectangle)  \
  : "d1", "a0", "a1", "fp0", "fp1", "cc", "memory"); \
  __ClearRectRegion__re; \
  }); \
  _ClearRectRegion__re; \
})

#define ClearRegion(region) ({ \
  struct Region * _ClearRegion_region = (region); \
  { \
  register struct GfxBase * const __ClearRegion__bn __asm("a6") = (struct GfxBase *) (GRAPHICS_BASE_NAME);\
  register struct Region * __ClearRegion_region __asm("a0") = (_ClearRegion_region); \
  __asm volatile ("jsr a6@(-528:W)" \
  : \
  : "r"(__ClearRegion__bn), "r"(__ClearRegion_region)  \
  : "d0", "d1", "a0", "a1", "fp0", "fp1", "cc", "memory"); \
  } \
})

#define DisposeRegion(region) ({ \
  struct Region * _DisposeRegion_region = (region); \
  { \
  register struct GfxBase * const __DisposeRegion__bn __asm("a6") = (struct GfxBase *) (GRAPHICS_BASE_NAME);\
  register struct Region * __DisposeRegion_region __asm("a0") = (_DisposeRegion_region); \
  __asm volatile ("jsr a6@(-534:W)" \
  : \
  : "r"(__DisposeRegion__bn), "r"(__DisposeRegion_region)  \
  : "d0", "d1", "a0", "a1", "fp0", "fp1", "cc", "memory"); \
  } \
})

#define FreeVPortCopLists(vp) ({ \
  struct ViewPort * _FreeVPortCopLists_vp = (vp); \
  { \
  register struct GfxBase * const __FreeVPortCopLists__bn __asm("a6") = (struct GfxBase *) (GRAPHICS_BASE_NAME);\
  register struct ViewPort * __FreeVPortCopLists_vp __asm("a0") = (_FreeVPortCopLists_vp); \
  __asm volatile ("jsr a6@(-540:W)" \
  : \
  : "r"(__FreeVPortCopLists__bn), "r"(__FreeVPortCopLists_vp)  \
  : "d0", "d1", "a0", "a1", "fp0", "fp1", "cc", "memory"); \
  } \
})

#define FreeCopList(copList) ({ \
  struct CopList * _FreeCopList_copList = (copList); \
  { \
  register struct GfxBase * const __FreeCopList__bn __asm("a6") = (struct GfxBase *) (GRAPHICS_BASE_NAME);\
  register struct CopList * __FreeCopList_copList __asm("a0") = (_FreeCopList_copList); \
  __asm volatile ("jsr a6@(-546:W)" \
  : \
  : "r"(__FreeCopList__bn), "r"(__FreeCopList_copList)  \
  : "d0", "d1", "a0", "a1", "fp0", "fp1", "cc", "memory"); \
  } \
})

#define ClipBlit(srcRP, xSrc, ySrc, destRP, xDest, yDest, xSize, ySize, minterm) ({ \
  struct RastPort * _ClipBlit_srcRP = (srcRP); \
  LONG _ClipBlit_xSrc = (xSrc); \
  LONG _ClipBlit_ySrc = (ySrc); \
  struct RastPort * _ClipBlit_destRP = (destRP); \
  LONG _ClipBlit_xDest = (xDest); \
  LONG _ClipBlit_yDest = (yDest); \
  LONG _ClipBlit_xSize = (xSize); \
  LONG _ClipBlit_ySize = (ySize); \
  ULONG _ClipBlit_minterm = (minterm); \
  { \
  register struct GfxBase * const __ClipBlit__bn __asm("a6") = (struct GfxBase *) (GRAPHICS_BASE_NAME);\
  register struct RastPort * __ClipBlit_srcRP __asm("a0") = (_ClipBlit_srcRP); \
  register LONG __ClipBlit_xSrc __asm("d0") = (_ClipBlit_xSrc); \
  register LONG __ClipBlit_ySrc __asm("d1") = (_ClipBlit_ySrc); \
  register struct RastPort * __ClipBlit_destRP __asm("a1") = (_ClipBlit_destRP); \
  register LONG __ClipBlit_xDest __asm("d2") = (_ClipBlit_xDest); \
  register LONG __ClipBlit_yDest __asm("d3") = (_ClipBlit_yDest); \
  register LONG __ClipBlit_xSize __asm("d4") = (_ClipBlit_xSize); \
  register LONG __ClipBlit_ySize __asm("d5") = (_ClipBlit_ySize); \
  register ULONG __ClipBlit_minterm __asm("d6") = (_ClipBlit_minterm); \
  __asm volatile ("jsr a6@(-552:W)" \
  : \
  : "r"(__ClipBlit__bn), "r"(__ClipBlit_srcRP), "r"(__ClipBlit_xSrc), "r"(__ClipBlit_ySrc), "r"(__ClipBlit_destRP), "r"(__ClipBlit_xDest), "r"(__ClipBlit_yDest), "r"(__ClipBlit_xSize), "r"(__ClipBlit_ySize), "r"(__ClipBlit_minterm)  \
  : "d0", "d1", "a0", "a1", "fp0", "fp1", "cc", "memory"); \
  } \
})

#define XorRectRegion(region, rectangle) ({ \
  struct Region * _XorRectRegion_region = (region); \
  CONST struct Rectangle * _XorRectRegion_rectangle = (rectangle); \
  BOOL _XorRectRegion__re = \
  ({ \
  register struct GfxBase * const __XorRectRegion__bn __asm("a6") = (struct GfxBase *) (GRAPHICS_BASE_NAME);\
  register BOOL __XorRectRegion__re __asm("d0"); \
  register struct Region * __XorRectRegion_region __asm("a0") = (_XorRectRegion_region); \
  register CONST struct Rectangle * __XorRectRegion_rectangle __asm("a1") = (_XorRectRegion_rectangle); \
  __asm volatile ("jsr a6@(-558:W)" \
  : "=r"(__XorRectRegion__re) \
  : "r"(__XorRectRegion__bn), "r"(__XorRectRegion_region), "r"(__XorRectRegion_rectangle)  \
  : "d1", "a0", "a1", "fp0", "fp1", "cc", "memory"); \
  __XorRectRegion__re; \
  }); \
  _XorRectRegion__re; \
})

#define FreeCprList(cprList) ({ \
  struct cprlist * _FreeCprList_cprList = (cprList); \
  { \
  register struct GfxBase * const __FreeCprList__bn __asm("a6") = (struct GfxBase *) (GRAPHICS_BASE_NAME);\
  register struct cprlist * __FreeCprList_cprList __asm("a0") = (_FreeCprList_cprList); \
  __asm volatile ("jsr a6@(-564:W)" \
  : \
  : "r"(__FreeCprList__bn), "r"(__FreeCprList_cprList)  \
  : "d0", "d1", "a0", "a1", "fp0", "fp1", "cc", "memory"); \
  } \
})

#define GetColorMap(entries) ({ \
  ULONG _GetColorMap_entries = (entries); \
  struct ColorMap * _GetColorMap__re = \
  ({ \
  register struct GfxBase * const __GetColorMap__bn __asm("a6") = (struct GfxBase *) (GRAPHICS_BASE_NAME);\
  register struct ColorMap * __GetColorMap__re __asm("d0"); \
  register ULONG __GetColorMap_entries __asm("d0") = (_GetColorMap_entries); \
  __asm volatile ("jsr a6@(-570:W)" \
  : "=r"(__GetColorMap__re) \
  : "r"(__GetColorMap__bn), "r"(__GetColorMap_entries)  \
  : "d1", "a0", "a1", "fp0", "fp1", "cc", "memory"); \
  __GetColorMap__re; \
  }); \
  _GetColorMap__re; \
})

#define FreeColorMap(colorMap) ({ \
  struct ColorMap * _FreeColorMap_colorMap = (colorMap); \
  { \
  register struct GfxBase * const __FreeColorMap__bn __asm("a6") = (struct GfxBase *) (GRAPHICS_BASE_NAME);\
  register struct ColorMap * __FreeColorMap_colorMap __asm("a0") = (_FreeColorMap_colorMap); \
  __asm volatile ("jsr a6@(-576:W)" \
  : \
  : "r"(__FreeColorMap__bn), "r"(__FreeColorMap_colorMap)  \
  : "d0", "d1", "a0", "a1", "fp0", "fp1", "cc", "memory"); \
  } \
})

#define GetRGB4(colorMap, entry) ({ \
  struct ColorMap * _GetRGB4_colorMap = (colorMap); \
  ULONG _GetRGB4_entry = (entry); \
  LONG _GetRGB4__re = \
  ({ \
  register struct GfxBase * const __GetRGB4__bn __asm("a6") = (struct GfxBase *) (GRAPHICS_BASE_NAME);\
  register LONG __GetRGB4__re __asm("d0"); \
  register struct ColorMap * __GetRGB4_colorMap __asm("a0") = (_GetRGB4_colorMap); \
  register ULONG __GetRGB4_entry __asm("d0") = (_GetRGB4_entry); \
  __asm volatile ("jsr a6@(-582:W)" \
  : "=r"(__GetRGB4__re) \
  : "r"(__GetRGB4__bn), "r"(__GetRGB4_colorMap), "r"(__GetRGB4_entry)  \
  : "d1", "a0", "a1", "fp0", "fp1", "cc", "memory"); \
  __GetRGB4__re; \
  }); \
  _GetRGB4__re; \
})

#define ScrollVPort(vp) ({ \
  struct ViewPort * _ScrollVPort_vp = (vp); \
  { \
  register struct GfxBase * const __ScrollVPort__bn __asm("a6") = (struct GfxBase *) (GRAPHICS_BASE_NAME);\
  register struct ViewPort * __ScrollVPort_vp __asm("a0") = (_ScrollVPort_vp); \
  __asm volatile ("jsr a6@(-588:W)" \
  : \
  : "r"(__ScrollVPort__bn), "r"(__ScrollVPort_vp)  \
  : "d0", "d1", "a0", "a1", "fp0", "fp1", "cc", "memory"); \
  } \
})

#define UCopperListInit(uCopList, n) ({ \
  struct UCopList * _UCopperListInit_uCopList = (uCopList); \
  LONG _UCopperListInit_n = (n); \
  struct CopList * _UCopperListInit__re = \
  ({ \
  register struct GfxBase * const __UCopperListInit__bn __asm("a6") = (struct GfxBase *) (GRAPHICS_BASE_NAME);\
  register struct CopList * __UCopperListInit__re __asm("d0"); \
  register struct UCopList * __UCopperListInit_uCopList __asm("a0") = (_UCopperListInit_uCopList); \
  register LONG __UCopperListInit_n __asm("d0") = (_UCopperListInit_n); \
  __asm volatile ("jsr a6@(-594:W)" \
  : "=r"(__UCopperListInit__re) \
  : "r"(__UCopperListInit__bn), "r"(__UCopperListInit_uCopList), "r"(__UCopperListInit_n)  \
  : "d1", "a0", "a1", "fp0", "fp1", "cc", "memory"); \
  __UCopperListInit__re; \
  }); \
  _UCopperListInit__re; \
})

#define FreeGBuffers(anOb, rp, flag) ({ \
  struct AnimOb * _FreeGBuffers_anOb = (anOb); \
  struct RastPort * _FreeGBuffers_rp = (rp); \
  LONG _FreeGBuffers_flag = (flag); \
  { \
  register struct GfxBase * const __FreeGBuffers__bn __asm("a6") = (struct GfxBase *) (GRAPHICS_BASE_NAME);\
  register struct AnimOb * __FreeGBuffers_anOb __asm("a0") = (_FreeGBuffers_anOb); \
  register struct RastPort * __FreeGBuffers_rp __asm("a1") = (_FreeGBuffers_rp); \
  register LONG __FreeGBuffers_flag __asm("d0") = (_FreeGBuffers_flag); \
  __asm volatile ("jsr a6@(-600:W)" \
  : \
  : "r"(__FreeGBuffers__bn), "r"(__FreeGBuffers_anOb), "r"(__FreeGBuffers_rp), "r"(__FreeGBuffers_flag)  \
  : "d0", "d1", "a0", "a1", "fp0", "fp1", "cc", "memory"); \
  } \
})

#define BltBitMapRastPort(srcBitMap, xSrc, ySrc, destRP, xDest, yDest, xSize, ySize, minterm) ({ \
  CONST struct BitMap * _BltBitMapRastPort_srcBitMap = (srcBitMap); \
  LONG _BltBitMapRastPort_xSrc = (xSrc); \
  LONG _BltBitMapRastPort_ySrc = (ySrc); \
  struct RastPort * _BltBitMapRastPort_destRP = (destRP); \
  LONG _BltBitMapRastPort_xDest = (xDest); \
  LONG _BltBitMapRastPort_yDest = (yDest); \
  LONG _BltBitMapRastPort_xSize = (xSize); \
  LONG _BltBitMapRastPort_ySize = (ySize); \
  ULONG _BltBitMapRastPort_minterm = (minterm); \
  BOOL _BltBitMapRastPort__re = \
  ({ \
  register struct GfxBase * const __BltBitMapRastPort__bn __asm("a6") = (struct GfxBase *) (GRAPHICS_BASE_NAME);\
  register BOOL __BltBitMapRastPort__re __asm("d0"); \
  register CONST struct BitMap * __BltBitMapRastPort_srcBitMap __asm("a0") = (_BltBitMapRastPort_srcBitMap); \
  register LONG __BltBitMapRastPort_xSrc __asm("d0") = (_BltBitMapRastPort_xSrc); \
  register LONG __BltBitMapRastPort_ySrc __asm("d1") = (_BltBitMapRastPort_ySrc); \
  register struct RastPort * __BltBitMapRastPort_destRP __asm("a1") = (_BltBitMapRastPort_destRP); \
  register LONG __BltBitMapRastPort_xDest __asm("d2") = (_BltBitMapRastPort_xDest); \
  register LONG __BltBitMapRastPort_yDest __asm("d3") = (_BltBitMapRastPort_yDest); \
  register LONG __BltBitMapRastPort_xSize __asm("d4") = (_BltBitMapRastPort_xSize); \
  register LONG __BltBitMapRastPort_ySize __asm("d5") = (_BltBitMapRastPort_ySize); \
  register ULONG __BltBitMapRastPort_minterm __asm("d6") = (_BltBitMapRastPort_minterm); \
  __asm volatile ("jsr a6@(-606:W)" \
  : "=r"(__BltBitMapRastPort__re) \
  : "r"(__BltBitMapRastPort__bn), "r"(__BltBitMapRastPort_srcBitMap), "r"(__BltBitMapRastPort_xSrc), "r"(__BltBitMapRastPort_ySrc), "r"(__BltBitMapRastPort_destRP), "r"(__BltBitMapRastPort_xDest), "r"(__BltBitMapRastPort_yDest), "r"(__BltBitMapRastPort_xSize), "r"(__BltBitMapRastPort_ySize), "r"(__BltBitMapRastPort_minterm)  \
  : "d1", "a0", "a1", "fp0", "fp1", "cc", "memory"); \
  __BltBitMapRastPort__re; \
  }); \
  _BltBitMapRastPort__re; \
})

#define OrRegionRegion(srcRegion, destRegion) ({ \
  CONST struct Region * _OrRegionRegion_srcRegion = (srcRegion); \
  struct Region * _OrRegionRegion_destRegion = (destRegion); \
  BOOL _OrRegionRegion__re = \
  ({ \
  register struct GfxBase * const __OrRegionRegion__bn __asm("a6") = (struct GfxBase *) (GRAPHICS_BASE_NAME);\
  register BOOL __OrRegionRegion__re __asm("d0"); \
  register CONST struct Region * __OrRegionRegion_srcRegion __asm("a0") = (_OrRegionRegion_srcRegion); \
  register struct Region * __OrRegionRegion_destRegion __asm("a1") = (_OrRegionRegion_destRegion); \
  __asm volatile ("jsr a6@(-612:W)" \
  : "=r"(__OrRegionRegion__re) \
  : "r"(__OrRegionRegion__bn), "r"(__OrRegionRegion_srcRegion), "r"(__OrRegionRegion_destRegion)  \
  : "d1", "a0", "a1", "fp0", "fp1", "cc", "memory"); \
  __OrRegionRegion__re; \
  }); \
  _OrRegionRegion__re; \
})

#define XorRegionRegion(srcRegion, destRegion) ({ \
  CONST struct Region * _XorRegionRegion_srcRegion = (srcRegion); \
  struct Region * _XorRegionRegion_destRegion = (destRegion); \
  BOOL _XorRegionRegion__re = \
  ({ \
  register struct GfxBase * const __XorRegionRegion__bn __asm("a6") = (struct GfxBase *) (GRAPHICS_BASE_NAME);\
  register BOOL __XorRegionRegion__re __asm("d0"); \
  register CONST struct Region * __XorRegionRegion_srcRegion __asm("a0") = (_XorRegionRegion_srcRegion); \
  register struct Region * __XorRegionRegion_destRegion __asm("a1") = (_XorRegionRegion_destRegion); \
  __asm volatile ("jsr a6@(-618:W)" \
  : "=r"(__XorRegionRegion__re) \
  : "r"(__XorRegionRegion__bn), "r"(__XorRegionRegion_srcRegion), "r"(__XorRegionRegion_destRegion)  \
  : "d1", "a0", "a1", "fp0", "fp1", "cc", "memory"); \
  __XorRegionRegion__re; \
  }); \
  _XorRegionRegion__re; \
})

#define AndRegionRegion(srcRegion, destRegion) ({ \
  CONST struct Region * _AndRegionRegion_srcRegion = (srcRegion); \
  struct Region * _AndRegionRegion_destRegion = (destRegion); \
  BOOL _AndRegionRegion__re = \
  ({ \
  register struct GfxBase * const __AndRegionRegion__bn __asm("a6") = (struct GfxBase *) (GRAPHICS_BASE_NAME);\
  register BOOL __AndRegionRegion__re __asm("d0"); \
  register CONST struct Region * __AndRegionRegion_srcRegion __asm("a0") = (_AndRegionRegion_srcRegion); \
  register struct Region * __AndRegionRegion_destRegion __asm("a1") = (_AndRegionRegion_destRegion); \
  __asm volatile ("jsr a6@(-624:W)" \
  : "=r"(__AndRegionRegion__re) \
  : "r"(__AndRegionRegion__bn), "r"(__AndRegionRegion_srcRegion), "r"(__AndRegionRegion_destRegion)  \
  : "d1", "a0", "a1", "fp0", "fp1", "cc", "memory"); \
  __AndRegionRegion__re; \
  }); \
  _AndRegionRegion__re; \
})

#define SetRGB4CM(colorMap, colindex, red, green, blue) ({ \
  struct ColorMap * _SetRGB4CM_colorMap = (colorMap); \
  ULONG _SetRGB4CM_colindex = (colindex); \
  ULONG _SetRGB4CM_red = (red); \
  ULONG _SetRGB4CM_green = (green); \
  ULONG _SetRGB4CM_blue = (blue); \
  { \
  register struct GfxBase * const __SetRGB4CM__bn __asm("a6") = (struct GfxBase *) (GRAPHICS_BASE_NAME);\
  register struct ColorMap * __SetRGB4CM_colorMap __asm("a0") = (_SetRGB4CM_colorMap); \
  register ULONG __SetRGB4CM_colindex __asm("d0") = (_SetRGB4CM_colindex); \
  register ULONG __SetRGB4CM_red __asm("d1") = (_SetRGB4CM_red); \
  register ULONG __SetRGB4CM_green __asm("d2") = (_SetRGB4CM_green); \
  register ULONG __SetRGB4CM_blue __asm("d3") = (_SetRGB4CM_blue); \
  __asm volatile ("jsr a6@(-630:W)" \
  : \
  : "r"(__SetRGB4CM__bn), "r"(__SetRGB4CM_colorMap), "r"(__SetRGB4CM_colindex), "r"(__SetRGB4CM_red), "r"(__SetRGB4CM_green), "r"(__SetRGB4CM_blue)  \
  : "d0", "d1", "a0", "a1", "fp0", "fp1", "cc", "memory"); \
  } \
})

#define BltMaskBitMapRastPort(srcBitMap, xSrc, ySrc, destRP, xDest, yDest, xSize, ySize, minterm, bltMask) ({ \
  struct BitMap * _BltMaskBitMapRastPort_srcBitMap = (srcBitMap); \
  LONG _BltMaskBitMapRastPort_xSrc = (xSrc); \
  LONG _BltMaskBitMapRastPort_ySrc = (ySrc); \
  struct RastPort * _BltMaskBitMapRastPort_destRP = (destRP); \
  LONG _BltMaskBitMapRastPort_xDest = (xDest); \
  LONG _BltMaskBitMapRastPort_yDest = (yDest); \
  LONG _BltMaskBitMapRastPort_xSize = (xSize); \
  LONG _BltMaskBitMapRastPort_ySize = (ySize); \
  ULONG _BltMaskBitMapRastPort_minterm = (minterm); \
  PLANEPTR _BltMaskBitMapRastPort_bltMask = (bltMask); \
  { \
  register struct GfxBase * const __BltMaskBitMapRastPort__bn __asm("a6") = (struct GfxBase *) (GRAPHICS_BASE_NAME);\
  register struct BitMap * __BltMaskBitMapRastPort_srcBitMap __asm("a0") = (_BltMaskBitMapRastPort_srcBitMap); \
  register LONG __BltMaskBitMapRastPort_xSrc __asm("d0") = (_BltMaskBitMapRastPort_xSrc); \
  register LONG __BltMaskBitMapRastPort_ySrc __asm("d1") = (_BltMaskBitMapRastPort_ySrc); \
  register struct RastPort * __BltMaskBitMapRastPort_destRP __asm("a1") = (_BltMaskBitMapRastPort_destRP); \
  register LONG __BltMaskBitMapRastPort_xDest __asm("d2") = (_BltMaskBitMapRastPort_xDest); \
  register LONG __BltMaskBitMapRastPort_yDest __asm("d3") = (_BltMaskBitMapRastPort_yDest); \
  register LONG __BltMaskBitMapRastPort_xSize __asm("d4") = (_BltMaskBitMapRastPort_xSize); \
  register LONG __BltMaskBitMapRastPort_ySize __asm("d5") = (_BltMaskBitMapRastPort_ySize); \
  register ULONG __BltMaskBitMapRastPort_minterm __asm("d6") = (_BltMaskBitMapRastPort_minterm); \
  register PLANEPTR __BltMaskBitMapRastPort_bltMask __asm("a2") = (_BltMaskBitMapRastPort_bltMask); \
  __asm volatile ("jsr a6@(-636:W)" \
  : \
  : "r"(__BltMaskBitMapRastPort__bn), "r"(__BltMaskBitMapRastPort_srcBitMap), "r"(__BltMaskBitMapRastPort_xSrc), "r"(__BltMaskBitMapRastPort_ySrc), "r"(__BltMaskBitMapRastPort_destRP), "r"(__BltMaskBitMapRastPort_xDest), "r"(__BltMaskBitMapRastPort_yDest), "r"(__BltMaskBitMapRastPort_xSize), "r"(__BltMaskBitMapRastPort_ySize), "r"(__BltMaskBitMapRastPort_minterm), "r"(__BltMaskBitMapRastPort_bltMask)  \
  : "d0", "d1", "a0", "a1", "fp0", "fp1", "cc", "memory"); \
  } \
})

#define AttemptLockLayerRom(layer) ({ \
  struct Layer * _AttemptLockLayerRom_layer = (layer); \
  BOOL _AttemptLockLayerRom__re = \
  ({ \
  register struct GfxBase * const __AttemptLockLayerRom__bn __asm("a6") = (struct GfxBase *) (GRAPHICS_BASE_NAME);\
  register BOOL __AttemptLockLayerRom__re __asm("d0"); \
  register struct Layer * __AttemptLockLayerRom_layer __asm("d2") = (_AttemptLockLayerRom_layer); \
  __asm volatile ("exg a5,d2\n\tjsr a6@(-654:W)\n\texg a5,d2" \
  : "=r"(__AttemptLockLayerRom__re) \
  : "r"(__AttemptLockLayerRom__bn), "r"(__AttemptLockLayerRom_layer)  \
  : "d1", "a0", "a1", "fp0", "fp1", "cc", "memory"); \
  __AttemptLockLayerRom__re; \
  }); \
  _AttemptLockLayerRom__re; \
})

#define GfxNew(gfxNodeType) ({ \
  ULONG _GfxNew_gfxNodeType = (gfxNodeType); \
  APTR _GfxNew__re = \
  ({ \
  register struct GfxBase * const __GfxNew__bn __asm("a6") = (struct GfxBase *) (GRAPHICS_BASE_NAME);\
  register APTR __GfxNew__re __asm("d0"); \
  register ULONG __GfxNew_gfxNodeType __asm("d0") = (_GfxNew_gfxNodeType); \
  __asm volatile ("jsr a6@(-660:W)" \
  : "=r"(__GfxNew__re) \
  : "r"(__GfxNew__bn), "r"(__GfxNew_gfxNodeType)  \
  : "d1", "a0", "a1", "fp0", "fp1", "cc", "memory"); \
  __GfxNew__re; \
  }); \
  _GfxNew__re; \
})

#define GfxFree(gfxNodePtr) ({ \
  struct ExtendedNode * _GfxFree_gfxNodePtr = (gfxNodePtr); \
  { \
  register struct GfxBase * const __GfxFree__bn __asm("a6") = (struct GfxBase *) (GRAPHICS_BASE_NAME);\
  register struct ExtendedNode * __GfxFree_gfxNodePtr __asm("a0") = (_GfxFree_gfxNodePtr); \
  __asm volatile ("jsr a6@(-666:W)" \
  : \
  : "r"(__GfxFree__bn), "r"(__GfxFree_gfxNodePtr)  \
  : "d0", "d1", "a0", "a1", "fp0", "fp1", "cc", "memory"); \
  } \
})

#define GfxAssociate(associateNode, gfxNodePtr) ({ \
  CONST APTR _GfxAssociate_associateNode = (associateNode); \
  struct ExtendedNode * _GfxAssociate_gfxNodePtr = (gfxNodePtr); \
  { \
  register struct GfxBase * const __GfxAssociate__bn __asm("a6") = (struct GfxBase *) (GRAPHICS_BASE_NAME);\
  register CONST APTR __GfxAssociate_associateNode __asm("a0") = (_GfxAssociate_associateNode); \
  register struct ExtendedNode * __GfxAssociate_gfxNodePtr __asm("a1") = (_GfxAssociate_gfxNodePtr); \
  __asm volatile ("jsr a6@(-672:W)" \
  : \
  : "r"(__GfxAssociate__bn), "r"(__GfxAssociate_associateNode), "r"(__GfxAssociate_gfxNodePtr)  \
  : "d0", "d1", "a0", "a1", "fp0", "fp1", "cc", "memory"); \
  } \
})

#define BitMapScale(bitScaleArgs) ({ \
  struct BitScaleArgs * _BitMapScale_bitScaleArgs = (bitScaleArgs); \
  { \
  register struct GfxBase * const __BitMapScale__bn __asm("a6") = (struct GfxBase *) (GRAPHICS_BASE_NAME);\
  register struct BitScaleArgs * __BitMapScale_bitScaleArgs __asm("a0") = (_BitMapScale_bitScaleArgs); \
  __asm volatile ("jsr a6@(-678:W)" \
  : \
  : "r"(__BitMapScale__bn), "r"(__BitMapScale_bitScaleArgs)  \
  : "d0", "d1", "a0", "a1", "fp0", "fp1", "cc", "memory"); \
  } \
})

#define ScalerDiv(factor, numerator, denominator) ({ \
  ULONG _ScalerDiv_factor = (factor); \
  ULONG _ScalerDiv_numerator = (numerator); \
  ULONG _ScalerDiv_denominator = (denominator); \
  UWORD _ScalerDiv__re = \
  ({ \
  register struct GfxBase * const __ScalerDiv__bn __asm("a6") = (struct GfxBase *) (GRAPHICS_BASE_NAME);\
  register UWORD __ScalerDiv__re __asm("d0"); \
  register ULONG __ScalerDiv_factor __asm("d0") = (_ScalerDiv_factor); \
  register ULONG __ScalerDiv_numerator __asm("d1") = (_ScalerDiv_numerator); \
  register ULONG __ScalerDiv_denominator __asm("d2") = (_ScalerDiv_denominator); \
  __asm volatile ("jsr a6@(-684:W)" \
  : "=r"(__ScalerDiv__re) \
  : "r"(__ScalerDiv__bn), "r"(__ScalerDiv_factor), "r"(__ScalerDiv_numerator), "r"(__ScalerDiv_denominator)  \
  : "d1", "a0", "a1", "fp0", "fp1", "cc", "memory"); \
  __ScalerDiv__re; \
  }); \
  _ScalerDiv__re; \
})

#define TextExtent(rp, string, count, textExtent) ({ \
  struct RastPort * _TextExtent_rp = (rp); \
  CONST_STRPTR _TextExtent_string = (string); \
  ULONG _TextExtent_count = (count); \
  struct TextExtent * _TextExtent_textExtent = (textExtent); \
  { \
  register struct GfxBase * const __TextExtent__bn __asm("a6") = (struct GfxBase *) (GRAPHICS_BASE_NAME);\
  register struct RastPort * __TextExtent_rp __asm("a1") = (_TextExtent_rp); \
  register CONST_STRPTR __TextExtent_string __asm("a0") = (_TextExtent_string); \
  register ULONG __TextExtent_count __asm("d0") = (_TextExtent_count); \
  register struct TextExtent * __TextExtent_textExtent __asm("a2") = (_TextExtent_textExtent); \
  __asm volatile ("jsr a6@(-690:W)" \
  : \
  : "r"(__TextExtent__bn), "r"(__TextExtent_rp), "r"(__TextExtent_string), "r"(__TextExtent_count), "r"(__TextExtent_textExtent)  \
  : "d0", "d1", "a0", "a1", "fp0", "fp1", "cc", "memory"); \
  } \
})

#define TextFit(rp, string, strLen, textExtent, constrainingExtent, strDirection, constrainingBitWidth, constrainingBitHeight) ({ \
  struct RastPort * _TextFit_rp = (rp); \
  CONST_STRPTR _TextFit_string = (string); \
  ULONG _TextFit_strLen = (strLen); \
  struct TextExtent * _TextFit_textExtent = (textExtent); \
  CONST struct TextExtent * _TextFit_constrainingExtent = (constrainingExtent); \
  LONG _TextFit_strDirection = (strDirection); \
  ULONG _TextFit_constrainingBitWidth = (constrainingBitWidth); \
  ULONG _TextFit_constrainingBitHeight = (constrainingBitHeight); \
  UWORD _TextFit__re = \
  ({ \
  register struct GfxBase * const __TextFit__bn __asm("a6") = (struct GfxBase *) (GRAPHICS_BASE_NAME);\
  register UWORD __TextFit__re __asm("d0"); \
  register struct RastPort * __TextFit_rp __asm("a1") = (_TextFit_rp); \
  register CONST_STRPTR __TextFit_string __asm("a0") = (_TextFit_string); \
  register ULONG __TextFit_strLen __asm("d0") = (_TextFit_strLen); \
  register struct TextExtent * __TextFit_textExtent __asm("a2") = (_TextFit_textExtent); \
  register CONST struct TextExtent * __TextFit_constrainingExtent __asm("a3") = (_TextFit_constrainingExtent); \
  register LONG __TextFit_strDirection __asm("d1") = (_TextFit_strDirection); \
  register ULONG __TextFit_constrainingBitWidth __asm("d2") = (_TextFit_constrainingBitWidth); \
  register ULONG __TextFit_constrainingBitHeight __asm("d3") = (_TextFit_constrainingBitHeight); \
  __asm volatile ("jsr a6@(-696:W)" \
  : "=r"(__TextFit__re) \
  : "r"(__TextFit__bn), "r"(__TextFit_rp), "r"(__TextFit_string), "r"(__TextFit_strLen), "r"(__TextFit_textExtent), "r"(__TextFit_constrainingExtent), "r"(__TextFit_strDirection), "r"(__TextFit_constrainingBitWidth), "r"(__TextFit_constrainingBitHeight)  \
  : "d1", "a0", "a1", "fp0", "fp1", "cc", "memory"); \
  __TextFit__re; \
  }); \
  _TextFit__re; \
})

#define GfxLookUp(associateNode) ({ \
  CONST APTR _GfxLookUp_associateNode = (associateNode); \
  APTR _GfxLookUp__re = \
  ({ \
  register struct GfxBase * const __GfxLookUp__bn __asm("a6") = (struct GfxBase *) (GRAPHICS_BASE_NAME);\
  register APTR __GfxLookUp__re __asm("d0"); \
  register CONST APTR __GfxLookUp_associateNode __asm("a0") = (_GfxLookUp_associateNode); \
  __asm volatile ("jsr a6@(-702:W)" \
  : "=r"(__GfxLookUp__re) \
  : "r"(__GfxLookUp__bn), "r"(__GfxLookUp_associateNode)  \
  : "d1", "a0", "a1", "fp0", "fp1", "cc", "memory"); \
  __GfxLookUp__re; \
  }); \
  _GfxLookUp__re; \
})

#define VideoControl(colorMap, tagarray) ({ \
  struct ColorMap * _VideoControl_colorMap = (colorMap); \
  struct TagItem * _VideoControl_tagarray = (tagarray); \
  ULONG _VideoControl__re = \
  ({ \
  register struct GfxBase * const __VideoControl__bn __asm("a6") = (struct GfxBase *) (GRAPHICS_BASE_NAME);\
  register ULONG __VideoControl__re __asm("d0"); \
  register struct ColorMap * __VideoControl_colorMap __asm("a0") = (_VideoControl_colorMap); \
  register struct TagItem * __VideoControl_tagarray __asm("a1") = (_VideoControl_tagarray); \
  __asm volatile ("jsr a6@(-708:W)" \
  : "=r"(__VideoControl__re) \
  : "r"(__VideoControl__bn), "r"(__VideoControl_colorMap), "r"(__VideoControl_tagarray)  \
  : "d1", "a0", "a1", "fp0", "fp1", "cc", "memory"); \
  __VideoControl__re; \
  }); \
  _VideoControl__re; \
})

#ifndef NO_INLINE_STDARG
static __inline__ ULONG ___VideoControlTags(struct GfxBase * GfxBase, struct ColorMap * colorMap, ULONG tagarray, ...)
{
  return VideoControl(colorMap, (struct TagItem *) &tagarray);
}

#define VideoControlTags(colorMap, tags...) ___VideoControlTags(GRAPHICS_BASE_NAME, colorMap, tags)
#endif

#define OpenMonitor(monitorName, displayID) ({ \
  CONST_STRPTR _OpenMonitor_monitorName = (monitorName); \
  ULONG _OpenMonitor_displayID = (displayID); \
  struct MonitorSpec * _OpenMonitor__re = \
  ({ \
  register struct GfxBase * const __OpenMonitor__bn __asm("a6") = (struct GfxBase *) (GRAPHICS_BASE_NAME);\
  register struct MonitorSpec * __OpenMonitor__re __asm("d0"); \
  register CONST_STRPTR __OpenMonitor_monitorName __asm("a1") = (_OpenMonitor_monitorName); \
  register ULONG __OpenMonitor_displayID __asm("d0") = (_OpenMonitor_displayID); \
  __asm volatile ("jsr a6@(-714:W)" \
  : "=r"(__OpenMonitor__re) \
  : "r"(__OpenMonitor__bn), "r"(__OpenMonitor_monitorName), "r"(__OpenMonitor_displayID)  \
  : "d1", "a0", "a1", "fp0", "fp1", "cc", "memory"); \
  __OpenMonitor__re; \
  }); \
  _OpenMonitor__re; \
})

#define CloseMonitor(monitorSpec) ({ \
  struct MonitorSpec * _CloseMonitor_monitorSpec = (monitorSpec); \
  LONG _CloseMonitor__re = \
  ({ \
  register struct GfxBase * const __CloseMonitor__bn __asm("a6") = (struct GfxBase *) (GRAPHICS_BASE_NAME);\
  register LONG __CloseMonitor__re __asm("d0"); \
  register struct MonitorSpec * __CloseMonitor_monitorSpec __asm("a0") = (_CloseMonitor_monitorSpec); \
  __asm volatile ("jsr a6@(-720:W)" \
  : "=r"(__CloseMonitor__re) \
  : "r"(__CloseMonitor__bn), "r"(__CloseMonitor_monitorSpec)  \
  : "d1", "a0", "a1", "fp0", "fp1", "cc", "memory"); \
  __CloseMonitor__re; \
  }); \
  _CloseMonitor__re; \
})

#define FindDisplayInfo(displayID) ({ \
  ULONG _FindDisplayInfo_displayID = (displayID); \
  DisplayInfoHandle _FindDisplayInfo__re = \
  ({ \
  register struct GfxBase * const __FindDisplayInfo__bn __asm("a6") = (struct GfxBase *) (GRAPHICS_BASE_NAME);\
  register DisplayInfoHandle __FindDisplayInfo__re __asm("d0"); \
  register ULONG __FindDisplayInfo_displayID __asm("d0") = (_FindDisplayInfo_displayID); \
  __asm volatile ("jsr a6@(-726:W)" \
  : "=r"(__FindDisplayInfo__re) \
  : "r"(__FindDisplayInfo__bn), "r"(__FindDisplayInfo_displayID)  \
  : "d1", "a0", "a1", "fp0", "fp1", "cc", "memory"); \
  __FindDisplayInfo__re; \
  }); \
  _FindDisplayInfo__re; \
})

#define NextDisplayInfo(displayID) ({ \
  ULONG _NextDisplayInfo_displayID = (displayID); \
  ULONG _NextDisplayInfo__re = \
  ({ \
  register struct GfxBase * const __NextDisplayInfo__bn __asm("a6") = (struct GfxBase *) (GRAPHICS_BASE_NAME);\
  register ULONG __NextDisplayInfo__re __asm("d0"); \
  register ULONG __NextDisplayInfo_displayID __asm("d0") = (_NextDisplayInfo_displayID); \
  __asm volatile ("jsr a6@(-732:W)" \
  : "=r"(__NextDisplayInfo__re) \
  : "r"(__NextDisplayInfo__bn), "r"(__NextDisplayInfo_displayID)  \
  : "d1", "a0", "a1", "fp0", "fp1", "cc", "memory"); \
  __NextDisplayInfo__re; \
  }); \
  _NextDisplayInfo__re; \
})

#define GetDisplayInfoData(handle, buf, size, tagID, displayID) ({ \
  CONST DisplayInfoHandle _GetDisplayInfoData_handle = (handle); \
  APTR _GetDisplayInfoData_buf = (buf); \
  ULONG _GetDisplayInfoData_size = (size); \
  ULONG _GetDisplayInfoData_tagID = (tagID); \
  ULONG _GetDisplayInfoData_displayID = (displayID); \
  ULONG _GetDisplayInfoData__re = \
  ({ \
  register struct GfxBase * const __GetDisplayInfoData__bn __asm("a6") = (struct GfxBase *) (GRAPHICS_BASE_NAME);\
  register ULONG __GetDisplayInfoData__re __asm("d0"); \
  register CONST DisplayInfoHandle __GetDisplayInfoData_handle __asm("a0") = (_GetDisplayInfoData_handle); \
  register APTR __GetDisplayInfoData_buf __asm("a1") = (_GetDisplayInfoData_buf); \
  register ULONG __GetDisplayInfoData_size __asm("d0") = (_GetDisplayInfoData_size); \
  register ULONG __GetDisplayInfoData_tagID __asm("d1") = (_GetDisplayInfoData_tagID); \
  register ULONG __GetDisplayInfoData_displayID __asm("d2") = (_GetDisplayInfoData_displayID); \
  __asm volatile ("jsr a6@(-756:W)" \
  : "=r"(__GetDisplayInfoData__re) \
  : "r"(__GetDisplayInfoData__bn), "r"(__GetDisplayInfoData_handle), "r"(__GetDisplayInfoData_buf), "r"(__GetDisplayInfoData_size), "r"(__GetDisplayInfoData_tagID), "r"(__GetDisplayInfoData_displayID)  \
  : "d1", "a0", "a1", "fp0", "fp1", "cc", "memory"); \
  __GetDisplayInfoData__re; \
  }); \
  _GetDisplayInfoData__re; \
})

#define FontExtent(font, fontExtent) ({ \
  CONST struct TextFont * _FontExtent_font = (font); \
  struct TextExtent * _FontExtent_fontExtent = (fontExtent); \
  { \
  register struct GfxBase * const __FontExtent__bn __asm("a6") = (struct GfxBase *) (GRAPHICS_BASE_NAME);\
  register CONST struct TextFont * __FontExtent_font __asm("a0") = (_FontExtent_font); \
  register struct TextExtent * __FontExtent_fontExtent __asm("a1") = (_FontExtent_fontExtent); \
  __asm volatile ("jsr a6@(-762:W)" \
  : \
  : "r"(__FontExtent__bn), "r"(__FontExtent_font), "r"(__FontExtent_fontExtent)  \
  : "d0", "d1", "a0", "a1", "fp0", "fp1", "cc", "memory"); \
  } \
})

#define ReadPixelLine8(rp, xstart, ystart, width, array, tempRP) ({ \
  struct RastPort * _ReadPixelLine8_rp = (rp); \
  ULONG _ReadPixelLine8_xstart = (xstart); \
  ULONG _ReadPixelLine8_ystart = (ystart); \
  ULONG _ReadPixelLine8_width = (width); \
  UBYTE * _ReadPixelLine8_array = (array); \
  struct RastPort * _ReadPixelLine8_tempRP = (tempRP); \
  LONG _ReadPixelLine8__re = \
  ({ \
  register struct GfxBase * const __ReadPixelLine8__bn __asm("a6") = (struct GfxBase *) (GRAPHICS_BASE_NAME);\
  register LONG __ReadPixelLine8__re __asm("d0"); \
  register struct RastPort * __ReadPixelLine8_rp __asm("a0") = (_ReadPixelLine8_rp); \
  register ULONG __ReadPixelLine8_xstart __asm("d0") = (_ReadPixelLine8_xstart); \
  register ULONG __ReadPixelLine8_ystart __asm("d1") = (_ReadPixelLine8_ystart); \
  register ULONG __ReadPixelLine8_width __asm("d2") = (_ReadPixelLine8_width); \
  register UBYTE * __ReadPixelLine8_array __asm("a2") = (_ReadPixelLine8_array); \
  register struct RastPort * __ReadPixelLine8_tempRP __asm("a1") = (_ReadPixelLine8_tempRP); \
  __asm volatile ("jsr a6@(-768:W)" \
  : "=r"(__ReadPixelLine8__re) \
  : "r"(__ReadPixelLine8__bn), "r"(__ReadPixelLine8_rp), "r"(__ReadPixelLine8_xstart), "r"(__ReadPixelLine8_ystart), "r"(__ReadPixelLine8_width), "r"(__ReadPixelLine8_array), "r"(__ReadPixelLine8_tempRP)  \
  : "d1", "a0", "a1", "fp0", "fp1", "cc", "memory"); \
  __ReadPixelLine8__re; \
  }); \
  _ReadPixelLine8__re; \
})

#define WritePixelLine8(rp, xstart, ystart, width, array, tempRP) ({ \
  struct RastPort * _WritePixelLine8_rp = (rp); \
  ULONG _WritePixelLine8_xstart = (xstart); \
  ULONG _WritePixelLine8_ystart = (ystart); \
  ULONG _WritePixelLine8_width = (width); \
  UBYTE * _WritePixelLine8_array = (array); \
  struct RastPort * _WritePixelLine8_tempRP = (tempRP); \
  LONG _WritePixelLine8__re = \
  ({ \
  register struct GfxBase * const __WritePixelLine8__bn __asm("a6") = (struct GfxBase *) (GRAPHICS_BASE_NAME);\
  register LONG __WritePixelLine8__re __asm("d0"); \
  register struct RastPort * __WritePixelLine8_rp __asm("a0") = (_WritePixelLine8_rp); \
  register ULONG __WritePixelLine8_xstart __asm("d0") = (_WritePixelLine8_xstart); \
  register ULONG __WritePixelLine8_ystart __asm("d1") = (_WritePixelLine8_ystart); \
  register ULONG __WritePixelLine8_width __asm("d2") = (_WritePixelLine8_width); \
  register UBYTE * __WritePixelLine8_array __asm("a2") = (_WritePixelLine8_array); \
  register struct RastPort * __WritePixelLine8_tempRP __asm("a1") = (_WritePixelLine8_tempRP); \
  __asm volatile ("jsr a6@(-774:W)" \
  : "=r"(__WritePixelLine8__re) \
  : "r"(__WritePixelLine8__bn), "r"(__WritePixelLine8_rp), "r"(__WritePixelLine8_xstart), "r"(__WritePixelLine8_ystart), "r"(__WritePixelLine8_width), "r"(__WritePixelLine8_array), "r"(__WritePixelLine8_tempRP)  \
  : "d1", "a0", "a1", "fp0", "fp1", "cc", "memory"); \
  __WritePixelLine8__re; \
  }); \
  _WritePixelLine8__re; \
})

#define ReadPixelArray8(rp, xstart, ystart, xstop, ystop, array, temprp) ({ \
  struct RastPort * _ReadPixelArray8_rp = (rp); \
  ULONG _ReadPixelArray8_xstart = (xstart); \
  ULONG _ReadPixelArray8_ystart = (ystart); \
  ULONG _ReadPixelArray8_xstop = (xstop); \
  ULONG _ReadPixelArray8_ystop = (ystop); \
  UBYTE * _ReadPixelArray8_array = (array); \
  struct RastPort * _ReadPixelArray8_temprp = (temprp); \
  LONG _ReadPixelArray8__re = \
  ({ \
  register struct GfxBase * const __ReadPixelArray8__bn __asm("a6") = (struct GfxBase *) (GRAPHICS_BASE_NAME);\
  register LONG __ReadPixelArray8__re __asm("d0"); \
  register struct RastPort * __ReadPixelArray8_rp __asm("a0") = (_ReadPixelArray8_rp); \
  register ULONG __ReadPixelArray8_xstart __asm("d0") = (_ReadPixelArray8_xstart); \
  register ULONG __ReadPixelArray8_ystart __asm("d1") = (_ReadPixelArray8_ystart); \
  register ULONG __ReadPixelArray8_xstop __asm("d2") = (_ReadPixelArray8_xstop); \
  register ULONG __ReadPixelArray8_ystop __asm("d3") = (_ReadPixelArray8_ystop); \
  register UBYTE * __ReadPixelArray8_array __asm("a2") = (_ReadPixelArray8_array); \
  register struct RastPort * __ReadPixelArray8_temprp __asm("a1") = (_ReadPixelArray8_temprp); \
  __asm volatile ("jsr a6@(-780:W)" \
  : "=r"(__ReadPixelArray8__re) \
  : "r"(__ReadPixelArray8__bn), "r"(__ReadPixelArray8_rp), "r"(__ReadPixelArray8_xstart), "r"(__ReadPixelArray8_ystart), "r"(__ReadPixelArray8_xstop), "r"(__ReadPixelArray8_ystop), "r"(__ReadPixelArray8_array), "r"(__ReadPixelArray8_temprp)  \
  : "d1", "a0", "a1", "fp0", "fp1", "cc", "memory"); \
  __ReadPixelArray8__re; \
  }); \
  _ReadPixelArray8__re; \
})

#define WritePixelArray8(rp, xstart, ystart, xstop, ystop, array, temprp) ({ \
  struct RastPort * _WritePixelArray8_rp = (rp); \
  ULONG _WritePixelArray8_xstart = (xstart); \
  ULONG _WritePixelArray8_ystart = (ystart); \
  ULONG _WritePixelArray8_xstop = (xstop); \
  ULONG _WritePixelArray8_ystop = (ystop); \
  UBYTE * _WritePixelArray8_array = (array); \
  struct RastPort * _WritePixelArray8_temprp = (temprp); \
  LONG _WritePixelArray8__re = \
  ({ \
  register struct GfxBase * const __WritePixelArray8__bn __asm("a6") = (struct GfxBase *) (GRAPHICS_BASE_NAME);\
  register LONG __WritePixelArray8__re __asm("d0"); \
  register struct RastPort * __WritePixelArray8_rp __asm("a0") = (_WritePixelArray8_rp); \
  register ULONG __WritePixelArray8_xstart __asm("d0") = (_WritePixelArray8_xstart); \
  register ULONG __WritePixelArray8_ystart __asm("d1") = (_WritePixelArray8_ystart); \
  register ULONG __WritePixelArray8_xstop __asm("d2") = (_WritePixelArray8_xstop); \
  register ULONG __WritePixelArray8_ystop __asm("d3") = (_WritePixelArray8_ystop); \
  register UBYTE * __WritePixelArray8_array __asm("a2") = (_WritePixelArray8_array); \
  register struct RastPort * __WritePixelArray8_temprp __asm("a1") = (_WritePixelArray8_temprp); \
  __asm volatile ("jsr a6@(-786:W)" \
  : "=r"(__WritePixelArray8__re) \
  : "r"(__WritePixelArray8__bn), "r"(__WritePixelArray8_rp), "r"(__WritePixelArray8_xstart), "r"(__WritePixelArray8_ystart), "r"(__WritePixelArray8_xstop), "r"(__WritePixelArray8_ystop), "r"(__WritePixelArray8_array), "r"(__WritePixelArray8_temprp)  \
  : "d1", "a0", "a1", "fp0", "fp1", "cc", "memory"); \
  __WritePixelArray8__re; \
  }); \
  _WritePixelArray8__re; \
})

#define GetVPModeID(vp) ({ \
  CONST struct ViewPort * _GetVPModeID_vp = (vp); \
  ULONG _GetVPModeID__re = \
  ({ \
  register struct GfxBase * const __GetVPModeID__bn __asm("a6") = (struct GfxBase *) (GRAPHICS_BASE_NAME);\
  register ULONG __GetVPModeID__re __asm("d0"); \
  register CONST struct ViewPort * __GetVPModeID_vp __asm("a0") = (_GetVPModeID_vp); \
  __asm volatile ("jsr a6@(-792:W)" \
  : "=r"(__GetVPModeID__re) \
  : "r"(__GetVPModeID__bn), "r"(__GetVPModeID_vp)  \
  : "d1", "a0", "a1", "fp0", "fp1", "cc", "memory"); \
  __GetVPModeID__re; \
  }); \
  _GetVPModeID__re; \
})

#define ModeNotAvailable(modeID) ({ \
  ULONG _ModeNotAvailable_modeID = (modeID); \
  ULONG _ModeNotAvailable__re = \
  ({ \
  register struct GfxBase * const __ModeNotAvailable__bn __asm("a6") = (struct GfxBase *) (GRAPHICS_BASE_NAME);\
  register ULONG __ModeNotAvailable__re __asm("d0"); \
  register ULONG __ModeNotAvailable_modeID __asm("d0") = (_ModeNotAvailable_modeID); \
  __asm volatile ("jsr a6@(-798:W)" \
  : "=r"(__ModeNotAvailable__re) \
  : "r"(__ModeNotAvailable__bn), "r"(__ModeNotAvailable_modeID)  \
  : "d1", "a0", "a1", "fp0", "fp1", "cc", "memory"); \
  __ModeNotAvailable__re; \
  }); \
  _ModeNotAvailable__re; \
})

#define EraseRect(rp, xMin, yMin, xMax, yMax) ({ \
  struct RastPort * _EraseRect_rp = (rp); \
  LONG _EraseRect_xMin = (xMin); \
  LONG _EraseRect_yMin = (yMin); \
  LONG _EraseRect_xMax = (xMax); \
  LONG _EraseRect_yMax = (yMax); \
  { \
  register struct GfxBase * const __EraseRect__bn __asm("a6") = (struct GfxBase *) (GRAPHICS_BASE_NAME);\
  register struct RastPort * __EraseRect_rp __asm("a1") = (_EraseRect_rp); \
  register LONG __EraseRect_xMin __asm("d0") = (_EraseRect_xMin); \
  register LONG __EraseRect_yMin __asm("d1") = (_EraseRect_yMin); \
  register LONG __EraseRect_xMax __asm("d2") = (_EraseRect_xMax); \
  register LONG __EraseRect_yMax __asm("d3") = (_EraseRect_yMax); \
  __asm volatile ("jsr a6@(-810:W)" \
  : \
  : "r"(__EraseRect__bn), "r"(__EraseRect_rp), "r"(__EraseRect_xMin), "r"(__EraseRect_yMin), "r"(__EraseRect_xMax), "r"(__EraseRect_yMax)  \
  : "d0", "d1", "a0", "a1", "fp0", "fp1", "cc", "memory"); \
  } \
})

#define ExtendFont(font, fontTags) ({ \
  struct TextFont * _ExtendFont_font = (font); \
  CONST struct TagItem * _ExtendFont_fontTags = (fontTags); \
  ULONG _ExtendFont__re = \
  ({ \
  register struct GfxBase * const __ExtendFont__bn __asm("a6") = (struct GfxBase *) (GRAPHICS_BASE_NAME);\
  register ULONG __ExtendFont__re __asm("d0"); \
  register struct TextFont * __ExtendFont_font __asm("a0") = (_ExtendFont_font); \
  register CONST struct TagItem * __ExtendFont_fontTags __asm("a1") = (_ExtendFont_fontTags); \
  __asm volatile ("jsr a6@(-816:W)" \
  : "=r"(__ExtendFont__re) \
  : "r"(__ExtendFont__bn), "r"(__ExtendFont_font), "r"(__ExtendFont_fontTags)  \
  : "d1", "a0", "a1", "fp0", "fp1", "cc", "memory"); \
  __ExtendFont__re; \
  }); \
  _ExtendFont__re; \
})

#ifndef NO_INLINE_STDARG
static __inline__ ULONG ___ExtendFontTags(struct GfxBase * GfxBase, struct TextFont * font, ULONG fontTags, ...)
{
  return ExtendFont(font, (CONST struct TagItem *) &fontTags);
}

#define ExtendFontTags(font, tags...) ___ExtendFontTags(GRAPHICS_BASE_NAME, font, tags)
#endif

#define StripFont(font) ({ \
  struct TextFont * _StripFont_font = (font); \
  { \
  register struct GfxBase * const __StripFont__bn __asm("a6") = (struct GfxBase *) (GRAPHICS_BASE_NAME);\
  register struct TextFont * __StripFont_font __asm("a0") = (_StripFont_font); \
  __asm volatile ("jsr a6@(-822:W)" \
  : \
  : "r"(__StripFont__bn), "r"(__StripFont_font)  \
  : "d0", "d1", "a0", "a1", "fp0", "fp1", "cc", "memory"); \
  } \
})

#define CalcIVG(v, vp) ({ \
  struct View * _CalcIVG_v = (v); \
  struct ViewPort * _CalcIVG_vp = (vp); \
  UWORD _CalcIVG__re = \
  ({ \
  register struct GfxBase * const __CalcIVG__bn __asm("a6") = (struct GfxBase *) (GRAPHICS_BASE_NAME);\
  register UWORD __CalcIVG__re __asm("d0"); \
  register struct View * __CalcIVG_v __asm("a0") = (_CalcIVG_v); \
  register struct ViewPort * __CalcIVG_vp __asm("a1") = (_CalcIVG_vp); \
  __asm volatile ("jsr a6@(-828:W)" \
  : "=r"(__CalcIVG__re) \
  : "r"(__CalcIVG__bn), "r"(__CalcIVG_v), "r"(__CalcIVG_vp)  \
  : "d1", "a0", "a1", "fp0", "fp1", "cc", "memory"); \
  __CalcIVG__re; \
  }); \
  _CalcIVG__re; \
})

#define AttachPalExtra(cm, vp) ({ \
  struct ColorMap * _AttachPalExtra_cm = (cm); \
  struct ViewPort * _AttachPalExtra_vp = (vp); \
  LONG _AttachPalExtra__re = \
  ({ \
  register struct GfxBase * const __AttachPalExtra__bn __asm("a6") = (struct GfxBase *) (GRAPHICS_BASE_NAME);\
  register LONG __AttachPalExtra__re __asm("d0"); \
  register struct ColorMap * __AttachPalExtra_cm __asm("a0") = (_AttachPalExtra_cm); \
  register struct ViewPort * __AttachPalExtra_vp __asm("a1") = (_AttachPalExtra_vp); \
  __asm volatile ("jsr a6@(-834:W)" \
  : "=r"(__AttachPalExtra__re) \
  : "r"(__AttachPalExtra__bn), "r"(__AttachPalExtra_cm), "r"(__AttachPalExtra_vp)  \
  : "d1", "a0", "a1", "fp0", "fp1", "cc", "memory"); \
  __AttachPalExtra__re; \
  }); \
  _AttachPalExtra__re; \
})

#define ObtainBestPenA(cm, r, g, b, tags) ({ \
  struct ColorMap * _ObtainBestPenA_cm = (cm); \
  ULONG _ObtainBestPenA_r = (r); \
  ULONG _ObtainBestPenA_g = (g); \
  ULONG _ObtainBestPenA_b = (b); \
  CONST struct TagItem * _ObtainBestPenA_tags = (tags); \
  LONG _ObtainBestPenA__re = \
  ({ \
  register struct GfxBase * const __ObtainBestPenA__bn __asm("a6") = (struct GfxBase *) (GRAPHICS_BASE_NAME);\
  register LONG __ObtainBestPenA__re __asm("d0"); \
  register struct ColorMap * __ObtainBestPenA_cm __asm("a0") = (_ObtainBestPenA_cm); \
  register ULONG __ObtainBestPenA_r __asm("d1") = (_ObtainBestPenA_r); \
  register ULONG __ObtainBestPenA_g __asm("d2") = (_ObtainBestPenA_g); \
  register ULONG __ObtainBestPenA_b __asm("d3") = (_ObtainBestPenA_b); \
  register CONST struct TagItem * __ObtainBestPenA_tags __asm("a1") = (_ObtainBestPenA_tags); \
  __asm volatile ("jsr a6@(-840:W)" \
  : "=r"(__ObtainBestPenA__re) \
  : "r"(__ObtainBestPenA__bn), "r"(__ObtainBestPenA_cm), "r"(__ObtainBestPenA_r), "r"(__ObtainBestPenA_g), "r"(__ObtainBestPenA_b), "r"(__ObtainBestPenA_tags)  \
  : "d1", "a0", "a1", "fp0", "fp1", "cc", "memory"); \
  __ObtainBestPenA__re; \
  }); \
  _ObtainBestPenA__re; \
})

#ifndef NO_INLINE_STDARG
static __inline__ LONG ___ObtainBestPen(struct GfxBase * GfxBase, struct ColorMap * cm, ULONG r, ULONG g, ULONG b, ULONG tags, ...)
{
  return ObtainBestPenA(cm, r, g, b, (CONST struct TagItem *) &tags);
}

#define ObtainBestPen(cm, r, g, b, tags...) ___ObtainBestPen(GRAPHICS_BASE_NAME, cm, r, g, b, tags)
#endif

#define SetRGB32(vp, n, r, g, b) ({ \
  struct ViewPort * _SetRGB32_vp = (vp); \
  ULONG _SetRGB32_n = (n); \
  ULONG _SetRGB32_r = (r); \
  ULONG _SetRGB32_g = (g); \
  ULONG _SetRGB32_b = (b); \
  { \
  register struct GfxBase * const __SetRGB32__bn __asm("a6") = (struct GfxBase *) (GRAPHICS_BASE_NAME);\
  register struct ViewPort * __SetRGB32_vp __asm("a0") = (_SetRGB32_vp); \
  register ULONG __SetRGB32_n __asm("d0") = (_SetRGB32_n); \
  register ULONG __SetRGB32_r __asm("d1") = (_SetRGB32_r); \
  register ULONG __SetRGB32_g __asm("d2") = (_SetRGB32_g); \
  register ULONG __SetRGB32_b __asm("d3") = (_SetRGB32_b); \
  __asm volatile ("jsr a6@(-852:W)" \
  : \
  : "r"(__SetRGB32__bn), "r"(__SetRGB32_vp), "r"(__SetRGB32_n), "r"(__SetRGB32_r), "r"(__SetRGB32_g), "r"(__SetRGB32_b)  \
  : "d0", "d1", "a0", "a1", "fp0", "fp1", "cc", "memory"); \
  } \
})

#define GetAPen(rp) ({ \
  struct RastPort * _GetAPen_rp = (rp); \
  ULONG _GetAPen__re = \
  ({ \
  register struct GfxBase * const __GetAPen__bn __asm("a6") = (struct GfxBase *) (GRAPHICS_BASE_NAME);\
  register ULONG __GetAPen__re __asm("d0"); \
  register struct RastPort * __GetAPen_rp __asm("a0") = (_GetAPen_rp); \
  __asm volatile ("jsr a6@(-858:W)" \
  : "=r"(__GetAPen__re) \
  : "r"(__GetAPen__bn), "r"(__GetAPen_rp)  \
  : "d1", "a0", "a1", "fp0", "fp1", "cc", "memory"); \
  __GetAPen__re; \
  }); \
  _GetAPen__re; \
})

#define GetBPen(rp) ({ \
  struct RastPort * _GetBPen_rp = (rp); \
  ULONG _GetBPen__re = \
  ({ \
  register struct GfxBase * const __GetBPen__bn __asm("a6") = (struct GfxBase *) (GRAPHICS_BASE_NAME);\
  register ULONG __GetBPen__re __asm("d0"); \
  register struct RastPort * __GetBPen_rp __asm("a0") = (_GetBPen_rp); \
  __asm volatile ("jsr a6@(-864:W)" \
  : "=r"(__GetBPen__re) \
  : "r"(__GetBPen__bn), "r"(__GetBPen_rp)  \
  : "d1", "a0", "a1", "fp0", "fp1", "cc", "memory"); \
  __GetBPen__re; \
  }); \
  _GetBPen__re; \
})

#define GetDrMd(rp) ({ \
  struct RastPort * _GetDrMd_rp = (rp); \
  ULONG _GetDrMd__re = \
  ({ \
  register struct GfxBase * const __GetDrMd__bn __asm("a6") = (struct GfxBase *) (GRAPHICS_BASE_NAME);\
  register ULONG __GetDrMd__re __asm("d0"); \
  register struct RastPort * __GetDrMd_rp __asm("a0") = (_GetDrMd_rp); \
  __asm volatile ("jsr a6@(-870:W)" \
  : "=r"(__GetDrMd__re) \
  : "r"(__GetDrMd__bn), "r"(__GetDrMd_rp)  \
  : "d1", "a0", "a1", "fp0", "fp1", "cc", "memory"); \
  __GetDrMd__re; \
  }); \
  _GetDrMd__re; \
})

#define GetOutlinePen(rp) ({ \
  struct RastPort * _GetOutlinePen_rp = (rp); \
  ULONG _GetOutlinePen__re = \
  ({ \
  register struct GfxBase * const __GetOutlinePen__bn __asm("a6") = (struct GfxBase *) (GRAPHICS_BASE_NAME);\
  register ULONG __GetOutlinePen__re __asm("d0"); \
  register struct RastPort * __GetOutlinePen_rp __asm("a0") = (_GetOutlinePen_rp); \
  __asm volatile ("jsr a6@(-876:W)" \
  : "=r"(__GetOutlinePen__re) \
  : "r"(__GetOutlinePen__bn), "r"(__GetOutlinePen_rp)  \
  : "d1", "a0", "a1", "fp0", "fp1", "cc", "memory"); \
  __GetOutlinePen__re; \
  }); \
  _GetOutlinePen__re; \
})

#define LoadRGB32(vp, table) ({ \
  struct ViewPort * _LoadRGB32_vp = (vp); \
  CONST ULONG * _LoadRGB32_table = (table); \
  { \
  register struct GfxBase * const __LoadRGB32__bn __asm("a6") = (struct GfxBase *) (GRAPHICS_BASE_NAME);\
  register struct ViewPort * __LoadRGB32_vp __asm("a0") = (_LoadRGB32_vp); \
  register CONST ULONG * __LoadRGB32_table __asm("a1") = (_LoadRGB32_table); \
  __asm volatile ("jsr a6@(-882:W)" \
  : \
  : "r"(__LoadRGB32__bn), "r"(__LoadRGB32_vp), "r"(__LoadRGB32_table)  \
  : "d0", "d1", "a0", "a1", "fp0", "fp1", "cc", "memory"); \
  } \
})

#define SetChipRev(want) ({ \
  ULONG _SetChipRev_want = (want); \
  ULONG _SetChipRev__re = \
  ({ \
  register struct GfxBase * const __SetChipRev__bn __asm("a6") = (struct GfxBase *) (GRAPHICS_BASE_NAME);\
  register ULONG __SetChipRev__re __asm("d0"); \
  register ULONG __SetChipRev_want __asm("d0") = (_SetChipRev_want); \
  __asm volatile ("jsr a6@(-888:W)" \
  : "=r"(__SetChipRev__re) \
  : "r"(__SetChipRev__bn), "r"(__SetChipRev_want)  \
  : "d1", "a0", "a1", "fp0", "fp1", "cc", "memory"); \
  __SetChipRev__re; \
  }); \
  _SetChipRev__re; \
})

#define SetABPenDrMd(rp, apen, bpen, drawmode) ({ \
  struct RastPort * _SetABPenDrMd_rp = (rp); \
  ULONG _SetABPenDrMd_apen = (apen); \
  ULONG _SetABPenDrMd_bpen = (bpen); \
  ULONG _SetABPenDrMd_drawmode = (drawmode); \
  { \
  register struct GfxBase * const __SetABPenDrMd__bn __asm("a6") = (struct GfxBase *) (GRAPHICS_BASE_NAME);\
  register struct RastPort * __SetABPenDrMd_rp __asm("a1") = (_SetABPenDrMd_rp); \
  register ULONG __SetABPenDrMd_apen __asm("d0") = (_SetABPenDrMd_apen); \
  register ULONG __SetABPenDrMd_bpen __asm("d1") = (_SetABPenDrMd_bpen); \
  register ULONG __SetABPenDrMd_drawmode __asm("d2") = (_SetABPenDrMd_drawmode); \
  __asm volatile ("jsr a6@(-894:W)" \
  : \
  : "r"(__SetABPenDrMd__bn), "r"(__SetABPenDrMd_rp), "r"(__SetABPenDrMd_apen), "r"(__SetABPenDrMd_bpen), "r"(__SetABPenDrMd_drawmode)  \
  : "d0", "d1", "a0", "a1", "fp0", "fp1", "cc", "memory"); \
  } \
})

#define GetRGB32(cm, firstcolor, ncolors, table) ({ \
  CONST struct ColorMap * _GetRGB32_cm = (cm); \
  ULONG _GetRGB32_firstcolor = (firstcolor); \
  ULONG _GetRGB32_ncolors = (ncolors); \
  ULONG * _GetRGB32_table = (table); \
  { \
  register struct GfxBase * const __GetRGB32__bn __asm("a6") = (struct GfxBase *) (GRAPHICS_BASE_NAME);\
  register CONST struct ColorMap * __GetRGB32_cm __asm("a0") = (_GetRGB32_cm); \
  register ULONG __GetRGB32_firstcolor __asm("d0") = (_GetRGB32_firstcolor); \
  register ULONG __GetRGB32_ncolors __asm("d1") = (_GetRGB32_ncolors); \
  register ULONG * __GetRGB32_table __asm("a1") = (_GetRGB32_table); \
  __asm volatile ("jsr a6@(-900:W)" \
  : \
  : "r"(__GetRGB32__bn), "r"(__GetRGB32_cm), "r"(__GetRGB32_firstcolor), "r"(__GetRGB32_ncolors), "r"(__GetRGB32_table)  \
  : "d0", "d1", "a0", "a1", "fp0", "fp1", "cc", "memory"); \
  } \
})

#define AllocBitMap(sizex, sizey, depth, flags, friend_bitmap) ({ \
  ULONG _AllocBitMap_sizex = (sizex); \
  ULONG _AllocBitMap_sizey = (sizey); \
  ULONG _AllocBitMap_depth = (depth); \
  ULONG _AllocBitMap_flags = (flags); \
  CONST struct BitMap * _AllocBitMap_friend_bitmap = (friend_bitmap); \
  struct BitMap * _AllocBitMap__re = \
  ({ \
  register struct GfxBase * const __AllocBitMap__bn __asm("a6") = (struct GfxBase *) (GRAPHICS_BASE_NAME);\
  register struct BitMap * __AllocBitMap__re __asm("d0"); \
  register ULONG __AllocBitMap_sizex __asm("d0") = (_AllocBitMap_sizex); \
  register ULONG __AllocBitMap_sizey __asm("d1") = (_AllocBitMap_sizey); \
  register ULONG __AllocBitMap_depth __asm("d2") = (_AllocBitMap_depth); \
  register ULONG __AllocBitMap_flags __asm("d3") = (_AllocBitMap_flags); \
  register CONST struct BitMap * __AllocBitMap_friend_bitmap __asm("a0") = (_AllocBitMap_friend_bitmap); \
  __asm volatile ("jsr a6@(-918:W)" \
  : "=r"(__AllocBitMap__re) \
  : "r"(__AllocBitMap__bn), "r"(__AllocBitMap_sizex), "r"(__AllocBitMap_sizey), "r"(__AllocBitMap_depth), "r"(__AllocBitMap_flags), "r"(__AllocBitMap_friend_bitmap)  \
  : "d1", "a0", "a1", "fp0", "fp1", "cc", "memory"); \
  __AllocBitMap__re; \
  }); \
  _AllocBitMap__re; \
})

#define FreeBitMap(bm) ({ \
  struct BitMap * _FreeBitMap_bm = (bm); \
  { \
  register struct GfxBase * const __FreeBitMap__bn __asm("a6") = (struct GfxBase *) (GRAPHICS_BASE_NAME);\
  register struct BitMap * __FreeBitMap_bm __asm("a0") = (_FreeBitMap_bm); \
  __asm volatile ("jsr a6@(-924:W)" \
  : \
  : "r"(__FreeBitMap__bn), "r"(__FreeBitMap_bm)  \
  : "d0", "d1", "a0", "a1", "fp0", "fp1", "cc", "memory"); \
  } \
})

#define GetExtSpriteA(ss, tags) ({ \
  struct ExtSprite * _GetExtSpriteA_ss = (ss); \
  CONST struct TagItem * _GetExtSpriteA_tags = (tags); \
  LONG _GetExtSpriteA__re = \
  ({ \
  register struct GfxBase * const __GetExtSpriteA__bn __asm("a6") = (struct GfxBase *) (GRAPHICS_BASE_NAME);\
  register LONG __GetExtSpriteA__re __asm("d0"); \
  register struct ExtSprite * __GetExtSpriteA_ss __asm("a2") = (_GetExtSpriteA_ss); \
  register CONST struct TagItem * __GetExtSpriteA_tags __asm("a1") = (_GetExtSpriteA_tags); \
  __asm volatile ("jsr a6@(-930:W)" \
  : "=r"(__GetExtSpriteA__re) \
  : "r"(__GetExtSpriteA__bn), "r"(__GetExtSpriteA_ss), "r"(__GetExtSpriteA_tags)  \
  : "d1", "a0", "a1", "fp0", "fp1", "cc", "memory"); \
  __GetExtSpriteA__re; \
  }); \
  _GetExtSpriteA__re; \
})

#ifndef NO_INLINE_STDARG
static __inline__ LONG ___GetExtSprite(struct GfxBase * GfxBase, struct ExtSprite * ss, ULONG tags, ...)
{
  return GetExtSpriteA(ss, (CONST struct TagItem *) &tags);
}

#define GetExtSprite(ss, tags...) ___GetExtSprite(GRAPHICS_BASE_NAME, ss, tags)
#endif

#define CoerceMode(vp, monitorid, flags) ({ \
  struct ViewPort * _CoerceMode_vp = (vp); \
  ULONG _CoerceMode_monitorid = (monitorid); \
  ULONG _CoerceMode_flags = (flags); \
  ULONG _CoerceMode__re = \
  ({ \
  register struct GfxBase * const __CoerceMode__bn __asm("a6") = (struct GfxBase *) (GRAPHICS_BASE_NAME);\
  register ULONG __CoerceMode__re __asm("d0"); \
  register struct ViewPort * __CoerceMode_vp __asm("a0") = (_CoerceMode_vp); \
  register ULONG __CoerceMode_monitorid __asm("d0") = (_CoerceMode_monitorid); \
  register ULONG __CoerceMode_flags __asm("d1") = (_CoerceMode_flags); \
  __asm volatile ("jsr a6@(-936:W)" \
  : "=r"(__CoerceMode__re) \
  : "r"(__CoerceMode__bn), "r"(__CoerceMode_vp), "r"(__CoerceMode_monitorid), "r"(__CoerceMode_flags)  \
  : "d1", "a0", "a1", "fp0", "fp1", "cc", "memory"); \
  __CoerceMode__re; \
  }); \
  _CoerceMode__re; \
})

#define ChangeVPBitMap(vp, bm, db) ({ \
  struct ViewPort * _ChangeVPBitMap_vp = (vp); \
  struct BitMap * _ChangeVPBitMap_bm = (bm); \
  struct DBufInfo * _ChangeVPBitMap_db = (db); \
  { \
  register struct GfxBase * const __ChangeVPBitMap__bn __asm("a6") = (struct GfxBase *) (GRAPHICS_BASE_NAME);\
  register struct ViewPort * __ChangeVPBitMap_vp __asm("a0") = (_ChangeVPBitMap_vp); \
  register struct BitMap * __ChangeVPBitMap_bm __asm("a1") = (_ChangeVPBitMap_bm); \
  register struct DBufInfo * __ChangeVPBitMap_db __asm("a2") = (_ChangeVPBitMap_db); \
  __asm volatile ("jsr a6@(-942:W)" \
  : \
  : "r"(__ChangeVPBitMap__bn), "r"(__ChangeVPBitMap_vp), "r"(__ChangeVPBitMap_bm), "r"(__ChangeVPBitMap_db)  \
  : "d0", "d1", "a0", "a1", "fp0", "fp1", "cc", "memory"); \
  } \
})

#define ReleasePen(cm, n) ({ \
  struct ColorMap * _ReleasePen_cm = (cm); \
  LONG _ReleasePen_n = (n); \
  { \
  register struct GfxBase * const __ReleasePen__bn __asm("a6") = (struct GfxBase *) (GRAPHICS_BASE_NAME);\
  register struct ColorMap * __ReleasePen_cm __asm("a0") = (_ReleasePen_cm); \
  register LONG __ReleasePen_n __asm("d0") = (_ReleasePen_n); \
  __asm volatile ("jsr a6@(-948:W)" \
  : \
  : "r"(__ReleasePen__bn), "r"(__ReleasePen_cm), "r"(__ReleasePen_n)  \
  : "d0", "d1", "a0", "a1", "fp0", "fp1", "cc", "memory"); \
  } \
})

#define ObtainPen(cm, n, r, g, b, f) ({ \
  struct ColorMap * _ObtainPen_cm = (cm); \
  LONG _ObtainPen_n = (n); \
  ULONG _ObtainPen_r = (r); \
  ULONG _ObtainPen_g = (g); \
  ULONG _ObtainPen_b = (b); \
  LONG _ObtainPen_f = (f); \
  LONG _ObtainPen__re = \
  ({ \
  register struct GfxBase * const __ObtainPen__bn __asm("a6") = (struct GfxBase *) (GRAPHICS_BASE_NAME);\
  register LONG __ObtainPen__re __asm("d0"); \
  register struct ColorMap * __ObtainPen_cm __asm("a0") = (_ObtainPen_cm); \
  register LONG __ObtainPen_n __asm("d0") = (_ObtainPen_n); \
  register ULONG __ObtainPen_r __asm("d1") = (_ObtainPen_r); \
  register ULONG __ObtainPen_g __asm("d2") = (_ObtainPen_g); \
  register ULONG __ObtainPen_b __asm("d3") = (_ObtainPen_b); \
  register LONG __ObtainPen_f __asm("d4") = (_ObtainPen_f); \
  __asm volatile ("jsr a6@(-954:W)" \
  : "=r"(__ObtainPen__re) \
  : "r"(__ObtainPen__bn), "r"(__ObtainPen_cm), "r"(__ObtainPen_n), "r"(__ObtainPen_r), "r"(__ObtainPen_g), "r"(__ObtainPen_b), "r"(__ObtainPen_f)  \
  : "d1", "a0", "a1", "fp0", "fp1", "cc", "memory"); \
  __ObtainPen__re; \
  }); \
  _ObtainPen__re; \
})

#define GetBitMapAttr(bm, attrnum) ({ \
  CONST struct BitMap * _GetBitMapAttr_bm = (bm); \
  ULONG _GetBitMapAttr_attrnum = (attrnum); \
  ULONG _GetBitMapAttr__re = \
  ({ \
  register struct GfxBase * const __GetBitMapAttr__bn __asm("a6") = (struct GfxBase *) (GRAPHICS_BASE_NAME);\
  register ULONG __GetBitMapAttr__re __asm("d0"); \
  register CONST struct BitMap * __GetBitMapAttr_bm __asm("a0") = (_GetBitMapAttr_bm); \
  register ULONG __GetBitMapAttr_attrnum __asm("d1") = (_GetBitMapAttr_attrnum); \
  __asm volatile ("jsr a6@(-960:W)" \
  : "=r"(__GetBitMapAttr__re) \
  : "r"(__GetBitMapAttr__bn), "r"(__GetBitMapAttr_bm), "r"(__GetBitMapAttr_attrnum)  \
  : "d1", "a0", "a1", "fp0", "fp1", "cc", "memory"); \
  __GetBitMapAttr__re; \
  }); \
  _GetBitMapAttr__re; \
})

#define AllocDBufInfo(vp) ({ \
  struct ViewPort * _AllocDBufInfo_vp = (vp); \
  struct DBufInfo * _AllocDBufInfo__re = \
  ({ \
  register struct GfxBase * const __AllocDBufInfo__bn __asm("a6") = (struct GfxBase *) (GRAPHICS_BASE_NAME);\
  register struct DBufInfo * __AllocDBufInfo__re __asm("d0"); \
  register struct ViewPort * __AllocDBufInfo_vp __asm("a0") = (_AllocDBufInfo_vp); \
  __asm volatile ("jsr a6@(-966:W)" \
  : "=r"(__AllocDBufInfo__re) \
  : "r"(__AllocDBufInfo__bn), "r"(__AllocDBufInfo_vp)  \
  : "d1", "a0", "a1", "fp0", "fp1", "cc", "memory"); \
  __AllocDBufInfo__re; \
  }); \
  _AllocDBufInfo__re; \
})

#define FreeDBufInfo(dbi) ({ \
  struct DBufInfo * _FreeDBufInfo_dbi = (dbi); \
  { \
  register struct GfxBase * const __FreeDBufInfo__bn __asm("a6") = (struct GfxBase *) (GRAPHICS_BASE_NAME);\
  register struct DBufInfo * __FreeDBufInfo_dbi __asm("a1") = (_FreeDBufInfo_dbi); \
  __asm volatile ("jsr a6@(-972:W)" \
  : \
  : "r"(__FreeDBufInfo__bn), "r"(__FreeDBufInfo_dbi)  \
  : "d0", "d1", "a0", "a1", "fp0", "fp1", "cc", "memory"); \
  } \
})

#define SetOutlinePen(rp, pen) ({ \
  struct RastPort * _SetOutlinePen_rp = (rp); \
  ULONG _SetOutlinePen_pen = (pen); \
  ULONG _SetOutlinePen__re = \
  ({ \
  register struct GfxBase * const __SetOutlinePen__bn __asm("a6") = (struct GfxBase *) (GRAPHICS_BASE_NAME);\
  register ULONG __SetOutlinePen__re __asm("d0"); \
  register struct RastPort * __SetOutlinePen_rp __asm("a0") = (_SetOutlinePen_rp); \
  register ULONG __SetOutlinePen_pen __asm("d0") = (_SetOutlinePen_pen); \
  __asm volatile ("jsr a6@(-978:W)" \
  : "=r"(__SetOutlinePen__re) \
  : "r"(__SetOutlinePen__bn), "r"(__SetOutlinePen_rp), "r"(__SetOutlinePen_pen)  \
  : "d1", "a0", "a1", "fp0", "fp1", "cc", "memory"); \
  __SetOutlinePen__re; \
  }); \
  _SetOutlinePen__re; \
})

#define SetWriteMask(rp, msk) ({ \
  struct RastPort * _SetWriteMask_rp = (rp); \
  ULONG _SetWriteMask_msk = (msk); \
  ULONG _SetWriteMask__re = \
  ({ \
  register struct GfxBase * const __SetWriteMask__bn __asm("a6") = (struct GfxBase *) (GRAPHICS_BASE_NAME);\
  register ULONG __SetWriteMask__re __asm("d0"); \
  register struct RastPort * __SetWriteMask_rp __asm("a0") = (_SetWriteMask_rp); \
  register ULONG __SetWriteMask_msk __asm("d0") = (_SetWriteMask_msk); \
  __asm volatile ("jsr a6@(-984:W)" \
  : "=r"(__SetWriteMask__re) \
  : "r"(__SetWriteMask__bn), "r"(__SetWriteMask_rp), "r"(__SetWriteMask_msk)  \
  : "d1", "a0", "a1", "fp0", "fp1", "cc", "memory"); \
  __SetWriteMask__re; \
  }); \
  _SetWriteMask__re; \
})

#define SetMaxPen(rp, maxpen) ({ \
  struct RastPort * _SetMaxPen_rp = (rp); \
  ULONG _SetMaxPen_maxpen = (maxpen); \
  { \
  register struct GfxBase * const __SetMaxPen__bn __asm("a6") = (struct GfxBase *) (GRAPHICS_BASE_NAME);\
  register struct RastPort * __SetMaxPen_rp __asm("a0") = (_SetMaxPen_rp); \
  register ULONG __SetMaxPen_maxpen __asm("d0") = (_SetMaxPen_maxpen); \
  __asm volatile ("jsr a6@(-990:W)" \
  : \
  : "r"(__SetMaxPen__bn), "r"(__SetMaxPen_rp), "r"(__SetMaxPen_maxpen)  \
  : "d0", "d1", "a0", "a1", "fp0", "fp1", "cc", "memory"); \
  } \
})

#define SetRGB32CM(cm, n, r, g, b) ({ \
  struct ColorMap * _SetRGB32CM_cm = (cm); \
  ULONG _SetRGB32CM_n = (n); \
  ULONG _SetRGB32CM_r = (r); \
  ULONG _SetRGB32CM_g = (g); \
  ULONG _SetRGB32CM_b = (b); \
  { \
  register struct GfxBase * const __SetRGB32CM__bn __asm("a6") = (struct GfxBase *) (GRAPHICS_BASE_NAME);\
  register struct ColorMap * __SetRGB32CM_cm __asm("a0") = (_SetRGB32CM_cm); \
  register ULONG __SetRGB32CM_n __asm("d0") = (_SetRGB32CM_n); \
  register ULONG __SetRGB32CM_r __asm("d1") = (_SetRGB32CM_r); \
  register ULONG __SetRGB32CM_g __asm("d2") = (_SetRGB32CM_g); \
  register ULONG __SetRGB32CM_b __asm("d3") = (_SetRGB32CM_b); \
  __asm volatile ("jsr a6@(-996:W)" \
  : \
  : "r"(__SetRGB32CM__bn), "r"(__SetRGB32CM_cm), "r"(__SetRGB32CM_n), "r"(__SetRGB32CM_r), "r"(__SetRGB32CM_g), "r"(__SetRGB32CM_b)  \
  : "d0", "d1", "a0", "a1", "fp0", "fp1", "cc", "memory"); \
  } \
})

#define ScrollRasterBF(rp, dx, dy, xMin, yMin, xMax, yMax) ({ \
  struct RastPort * _ScrollRasterBF_rp = (rp); \
  LONG _ScrollRasterBF_dx = (dx); \
  LONG _ScrollRasterBF_dy = (dy); \
  LONG _ScrollRasterBF_xMin = (xMin); \
  LONG _ScrollRasterBF_yMin = (yMin); \
  LONG _ScrollRasterBF_xMax = (xMax); \
  LONG _ScrollRasterBF_yMax = (yMax); \
  { \
  register struct GfxBase * const __ScrollRasterBF__bn __asm("a6") = (struct GfxBase *) (GRAPHICS_BASE_NAME);\
  register struct RastPort * __ScrollRasterBF_rp __asm("a1") = (_ScrollRasterBF_rp); \
  register LONG __ScrollRasterBF_dx __asm("d0") = (_ScrollRasterBF_dx); \
  register LONG __ScrollRasterBF_dy __asm("d1") = (_ScrollRasterBF_dy); \
  register LONG __ScrollRasterBF_xMin __asm("d2") = (_ScrollRasterBF_xMin); \
  register LONG __ScrollRasterBF_yMin __asm("d3") = (_ScrollRasterBF_yMin); \
  register LONG __ScrollRasterBF_xMax __asm("d4") = (_ScrollRasterBF_xMax); \
  register LONG __ScrollRasterBF_yMax __asm("d5") = (_ScrollRasterBF_yMax); \
  __asm volatile ("jsr a6@(-1002:W)" \
  : \
  : "r"(__ScrollRasterBF__bn), "r"(__ScrollRasterBF_rp), "r"(__ScrollRasterBF_dx), "r"(__ScrollRasterBF_dy), "r"(__ScrollRasterBF_xMin), "r"(__ScrollRasterBF_yMin), "r"(__ScrollRasterBF_xMax), "r"(__ScrollRasterBF_yMax)  \
  : "d0", "d1", "a0", "a1", "fp0", "fp1", "cc", "memory"); \
  } \
})

#define FindColor(cm, r, g, b, maxcolor) ({ \
  struct ColorMap * _FindColor_cm = (cm); \
  ULONG _FindColor_r = (r); \
  ULONG _FindColor_g = (g); \
  ULONG _FindColor_b = (b); \
  LONG _FindColor_maxcolor = (maxcolor); \
  UWORD _FindColor__re = \
  ({ \
  register struct GfxBase * const __FindColor__bn __asm("a6") = (struct GfxBase *) (GRAPHICS_BASE_NAME);\
  register UWORD __FindColor__re __asm("d0"); \
  register struct ColorMap * __FindColor_cm __asm("a3") = (_FindColor_cm); \
  register ULONG __FindColor_r __asm("d1") = (_FindColor_r); \
  register ULONG __FindColor_g __asm("d2") = (_FindColor_g); \
  register ULONG __FindColor_b __asm("d3") = (_FindColor_b); \
  register LONG __FindColor_maxcolor __asm("d4") = (_FindColor_maxcolor); \
  __asm volatile ("jsr a6@(-1008:W)" \
  : "=r"(__FindColor__re) \
  : "r"(__FindColor__bn), "r"(__FindColor_cm), "r"(__FindColor_r), "r"(__FindColor_g), "r"(__FindColor_b), "r"(__FindColor_maxcolor)  \
  : "d1", "a0", "a1", "fp0", "fp1", "cc", "memory"); \
  __FindColor__re; \
  }); \
  _FindColor__re; \
})

#define AllocSpriteDataA(bm, tags) ({ \
  CONST struct BitMap * _AllocSpriteDataA_bm = (bm); \
  CONST struct TagItem * _AllocSpriteDataA_tags = (tags); \
  struct ExtSprite * _AllocSpriteDataA__re = \
  ({ \
  register struct GfxBase * const __AllocSpriteDataA__bn __asm("a6") = (struct GfxBase *) (GRAPHICS_BASE_NAME);\
  register struct ExtSprite * __AllocSpriteDataA__re __asm("d0"); \
  register CONST struct BitMap * __AllocSpriteDataA_bm __asm("a2") = (_AllocSpriteDataA_bm); \
  register CONST struct TagItem * __AllocSpriteDataA_tags __asm("a1") = (_AllocSpriteDataA_tags); \
  __asm volatile ("jsr a6@(-1020:W)" \
  : "=r"(__AllocSpriteDataA__re) \
  : "r"(__AllocSpriteDataA__bn), "r"(__AllocSpriteDataA_bm), "r"(__AllocSpriteDataA_tags)  \
  : "d1", "a0", "a1", "fp0", "fp1", "cc", "memory"); \
  __AllocSpriteDataA__re; \
  }); \
  _AllocSpriteDataA__re; \
})

#ifndef NO_INLINE_STDARG
static __inline__ struct ExtSprite * ___AllocSpriteData(struct GfxBase * GfxBase, CONST struct BitMap * bm, ULONG tags, ...)
{
  return AllocSpriteDataA(bm, (CONST struct TagItem *) &tags);
}

#define AllocSpriteData(bm, tags...) ___AllocSpriteData(GRAPHICS_BASE_NAME, bm, tags)
#endif

#define ChangeExtSpriteA(vp, oldsprite, newsprite, tags) ({ \
  struct ViewPort * _ChangeExtSpriteA_vp = (vp); \
  struct ExtSprite * _ChangeExtSpriteA_oldsprite = (oldsprite); \
  struct ExtSprite * _ChangeExtSpriteA_newsprite = (newsprite); \
  CONST struct TagItem * _ChangeExtSpriteA_tags = (tags); \
  LONG _ChangeExtSpriteA__re = \
  ({ \
  register struct GfxBase * const __ChangeExtSpriteA__bn __asm("a6") = (struct GfxBase *) (GRAPHICS_BASE_NAME);\
  register LONG __ChangeExtSpriteA__re __asm("d0"); \
  register struct ViewPort * __ChangeExtSpriteA_vp __asm("a0") = (_ChangeExtSpriteA_vp); \
  register struct ExtSprite * __ChangeExtSpriteA_oldsprite __asm("a1") = (_ChangeExtSpriteA_oldsprite); \
  register struct ExtSprite * __ChangeExtSpriteA_newsprite __asm("a2") = (_ChangeExtSpriteA_newsprite); \
  register CONST struct TagItem * __ChangeExtSpriteA_tags __asm("a3") = (_ChangeExtSpriteA_tags); \
  __asm volatile ("jsr a6@(-1026:W)" \
  : "=r"(__ChangeExtSpriteA__re) \
  : "r"(__ChangeExtSpriteA__bn), "r"(__ChangeExtSpriteA_vp), "r"(__ChangeExtSpriteA_oldsprite), "r"(__ChangeExtSpriteA_newsprite), "r"(__ChangeExtSpriteA_tags)  \
  : "d1", "a0", "a1", "fp0", "fp1", "cc", "memory"); \
  __ChangeExtSpriteA__re; \
  }); \
  _ChangeExtSpriteA__re; \
})

#ifndef NO_INLINE_STDARG
static __inline__ LONG ___ChangeExtSprite(struct GfxBase * GfxBase, struct ViewPort * vp, struct ExtSprite * oldsprite, struct ExtSprite * newsprite, ULONG tags, ...)
{
  return ChangeExtSpriteA(vp, oldsprite, newsprite, (CONST struct TagItem *) &tags);
}

#define ChangeExtSprite(vp, oldsprite, newsprite, tags...) ___ChangeExtSprite(GRAPHICS_BASE_NAME, vp, oldsprite, newsprite, tags)
#endif

#define FreeSpriteData(sp) ({ \
  struct ExtSprite * _FreeSpriteData_sp = (sp); \
  { \
  register struct GfxBase * const __FreeSpriteData__bn __asm("a6") = (struct GfxBase *) (GRAPHICS_BASE_NAME);\
  register struct ExtSprite * __FreeSpriteData_sp __asm("a2") = (_FreeSpriteData_sp); \
  __asm volatile ("jsr a6@(-1032:W)" \
  : \
  : "r"(__FreeSpriteData__bn), "r"(__FreeSpriteData_sp)  \
  : "d0", "d1", "a0", "a1", "fp0", "fp1", "cc", "memory"); \
  } \
})

#define SetRPAttrsA(rp, tags) ({ \
  struct RastPort * _SetRPAttrsA_rp = (rp); \
  CONST struct TagItem * _SetRPAttrsA_tags = (tags); \
  { \
  register struct GfxBase * const __SetRPAttrsA__bn __asm("a6") = (struct GfxBase *) (GRAPHICS_BASE_NAME);\
  register struct RastPort * __SetRPAttrsA_rp __asm("a0") = (_SetRPAttrsA_rp); \
  register CONST struct TagItem * __SetRPAttrsA_tags __asm("a1") = (_SetRPAttrsA_tags); \
  __asm volatile ("jsr a6@(-1038:W)" \
  : \
  : "r"(__SetRPAttrsA__bn), "r"(__SetRPAttrsA_rp), "r"(__SetRPAttrsA_tags)  \
  : "d0", "d1", "a0", "a1", "fp0", "fp1", "cc", "memory"); \
  } \
})

#ifndef NO_INLINE_STDARG
static __inline__ VOID ___SetRPAttrs(struct GfxBase * GfxBase, struct RastPort * rp, ULONG tags, ...)
{
  SetRPAttrsA(rp, (CONST struct TagItem *) &tags);
}

#define SetRPAttrs(rp, tags...) ___SetRPAttrs(GRAPHICS_BASE_NAME, rp, tags)
#endif

#define GetRPAttrsA(rp, tags) ({ \
  CONST struct RastPort * _GetRPAttrsA_rp = (rp); \
  CONST struct TagItem * _GetRPAttrsA_tags = (tags); \
  { \
  register struct GfxBase * const __GetRPAttrsA__bn __asm("a6") = (struct GfxBase *) (GRAPHICS_BASE_NAME);\
  register CONST struct RastPort * __GetRPAttrsA_rp __asm("a0") = (_GetRPAttrsA_rp); \
  register CONST struct TagItem * __GetRPAttrsA_tags __asm("a1") = (_GetRPAttrsA_tags); \
  __asm volatile ("jsr a6@(-1044:W)" \
  : \
  : "r"(__GetRPAttrsA__bn), "r"(__GetRPAttrsA_rp), "r"(__GetRPAttrsA_tags)  \
  : "d0", "d1", "a0", "a1", "fp0", "fp1", "cc", "memory"); \
  } \
})

#ifndef NO_INLINE_STDARG
static __inline__ VOID ___GetRPAttrs(struct GfxBase * GfxBase, CONST struct RastPort * rp, ULONG tags, ...)
{
  GetRPAttrsA(rp, (CONST struct TagItem *) &tags);
}

#define GetRPAttrs(rp, tags...) ___GetRPAttrs(GRAPHICS_BASE_NAME, rp, tags)
#endif

#define BestModeIDA(tags) ({ \
  CONST struct TagItem * _BestModeIDA_tags = (tags); \
  ULONG _BestModeIDA__re = \
  ({ \
  register struct GfxBase * const __BestModeIDA__bn __asm("a6") = (struct GfxBase *) (GRAPHICS_BASE_NAME);\
  register ULONG __BestModeIDA__re __asm("d0"); \
  register CONST struct TagItem * __BestModeIDA_tags __asm("a0") = (_BestModeIDA_tags); \
  __asm volatile ("jsr a6@(-1050:W)" \
  : "=r"(__BestModeIDA__re) \
  : "r"(__BestModeIDA__bn), "r"(__BestModeIDA_tags)  \
  : "d1", "a0", "a1", "fp0", "fp1", "cc", "memory"); \
  __BestModeIDA__re; \
  }); \
  _BestModeIDA__re; \
})

#ifndef NO_INLINE_STDARG
static __inline__ ULONG ___BestModeID(struct GfxBase * GfxBase, ULONG tags, ...)
{
  return BestModeIDA((CONST struct TagItem *) &tags);
}

#define BestModeID(tags...) ___BestModeID(GRAPHICS_BASE_NAME, tags)
#endif

#define WriteChunkyPixels(rp, xstart, ystart, xstop, ystop, array, bytesperrow) ({ \
  struct RastPort * _WriteChunkyPixels_rp = (rp); \
  ULONG _WriteChunkyPixels_xstart = (xstart); \
  ULONG _WriteChunkyPixels_ystart = (ystart); \
  ULONG _WriteChunkyPixels_xstop = (xstop); \
  ULONG _WriteChunkyPixels_ystop = (ystop); \
  CONST UBYTE * _WriteChunkyPixels_array = (array); \
  LONG _WriteChunkyPixels_bytesperrow = (bytesperrow); \
  { \
  register struct GfxBase * const __WriteChunkyPixels__bn __asm("a6") = (struct GfxBase *) (GRAPHICS_BASE_NAME);\
  register struct RastPort * __WriteChunkyPixels_rp __asm("a0") = (_WriteChunkyPixels_rp); \
  register ULONG __WriteChunkyPixels_xstart __asm("d0") = (_WriteChunkyPixels_xstart); \
  register ULONG __WriteChunkyPixels_ystart __asm("d1") = (_WriteChunkyPixels_ystart); \
  register ULONG __WriteChunkyPixels_xstop __asm("d2") = (_WriteChunkyPixels_xstop); \
  register ULONG __WriteChunkyPixels_ystop __asm("d3") = (_WriteChunkyPixels_ystop); \
  register CONST UBYTE * __WriteChunkyPixels_array __asm("a2") = (_WriteChunkyPixels_array); \
  register LONG __WriteChunkyPixels_bytesperrow __asm("d4") = (_WriteChunkyPixels_bytesperrow); \
  __asm volatile ("jsr a6@(-1056:W)" \
  : \
  : "r"(__WriteChunkyPixels__bn), "r"(__WriteChunkyPixels_rp), "r"(__WriteChunkyPixels_xstart), "r"(__WriteChunkyPixels_ystart), "r"(__WriteChunkyPixels_xstop), "r"(__WriteChunkyPixels_ystop), "r"(__WriteChunkyPixels_array), "r"(__WriteChunkyPixels_bytesperrow)  \
  : "d0", "d1", "a0", "a1", "fp0", "fp1", "cc", "memory"); \
  } \
})

#endif /*  _INLINE_GRAPHICS_H  */
