#ifndef THEBAR_MCC_H
#define THEBAR_MCC_H

/*
** TheBar.mcc - Next Generation Toolbar MUI Custom Class
** Copyright (C) 2003-2008 Alfonso Ranieri
**
** TheBar is developed by TheBar.mcc Open Source Team
**
** This library is free software; you can redistribute it and/or
** modify it under the terms of the GNU Lesser General Public
** License as published by the Free Software Foundation; either
** version 2.1 of the License, or (at your option) any later version.
**
** This library is distributed in the hope that it will be useful,
** but WITHOUT ANY WARRANTY; without even the implied warranty of
** MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
** Lesser General Public License for more details.
**
** TheBar class Support Site: http://www.sf.net/projects/thebar
**
** $Id: TheBar_mcc.h 332 2009-10-04 11:57:03Z thboeckel $
** $URL: https://svn.code.sf.net/p/thebar/code/trunk/include/mui/TheBar_mcc.h $
**
**/

#ifndef LIBRARIES_MUI_H
#include <libraries/mui.h>
#endif

#if !defined(__AROS__) && defined(__PPC__)
  #if defined(__GNUC__)
    #pragma pack(2)
  #elif defined(__VBCC__)
    #pragma amiga-align
  #endif
#endif

/***********************************************************************/

#ifndef STACKED
// STACKED ensures proper alignment on AROS 64 bit systems
#define STACKED
#endif

/***********************************************************************/

#define MUIC_TheButton    "TheButton.mcc"
#define MUIC_TheBar       "TheBar.mcc"
#define MUIC_TheBarVirt   "TheBarVirt.mcc"

#if defined(__AROS__) && !defined(NO_INLINE_STDARG)
#define TheButtonObject   MUIOBJMACRO_START(MUIC_TheButton)
#define TheBarObject      MUIOBJMACRO_START(MUIC_TheBar)
#define TheBarVirtObject  MUIOBJMACRO_START(MUIC_TheBarVirt)
#else
#define TheButtonObject   MUI_NewObject(MUIC_TheButton
#define TheBarObject      MUI_NewObject(MUIC_TheBar
#define TheBarVirtObject  MUI_NewObject(MUIC_TheBarVirt
#endif

#define THEBAR_VERSION     21
#define THEBARVIRT_VERSION 21
#define THEBUTTON_VERSION  21

/***********************************************************************/

#define TBUTTAGBASE 0xF76B01C8UL
#define TBTAGBASE   0xF76B022CUL

/***********************************************************************/
/*
** TheBar.mcc Methods
*/

#define MUIM_TheBar_Rebuild         (TBTAGBASE+0)   /* v11 PRIVATE */
#define MUIM_TheBar_DeActivate      (TBTAGBASE+2)   /* v11 PRIVATE */
#define MUIM_TheBar_AddButton       (TBTAGBASE+3)   /* v11         */
#define MUIM_TheBar_AddSpacer       (TBTAGBASE+4)   /* v11 PRIVATE */
#define MUIM_TheBar_GetObject       (TBTAGBASE+5)   /* v11         */
#define MUIM_TheBar_DoOnButton      (TBTAGBASE+6)   /* v11         */
#define MUIM_TheBar_SetAttr         (TBTAGBASE+7)   /* v11         */
#define MUIM_TheBar_GetAttr         (TBTAGBASE+8)   /* v11         */
#define MUIM_TheBar_Clear           (TBTAGBASE+9)   /* v11         */
#define MUIM_TheBar_Sort            (TBTAGBASE+10)  /* v11         */
#define MUIM_TheBar_Remove          (TBTAGBASE+11)  /* v11         */
#define MUIM_TheBar_GetDragImage    (TBTAGBASE+12)  /* v11         */
#define MUIM_TheBar_Notify          (TBTAGBASE+13)  /* v21         */
#define MUIM_TheBar_KillNotify      (TBTAGBASE+14)  /* v21         */
#define MUIM_TheBar_NoNotifySetAttr (TBTAGBASE+15)  /* v21         */

/***********************************************************************/
/*
** TheBar.mcc Methods structures
*/

struct MUIP_TheBar_AddButton       { STACKED ULONG MethodID; STACKED struct MUIS_TheBar_Button *button; };
struct MUIP_TheBar_AddSpacer       { STACKED ULONG MethodID; STACKED ULONG ID; STACKED ULONG type; };
struct MUIP_TheBar_GetObject       { STACKED ULONG MethodID; STACKED ULONG ID; };
struct MUIP_TheBar_DoOnButton      { STACKED ULONG MethodID; STACKED ULONG ID; STACKED ULONG method; /* ...args... */ };
struct MUIP_TheBar_SetAttr         { STACKED ULONG MethodID; STACKED ULONG ID; STACKED Tag attr; STACKED ULONG value; };
struct MUIP_TheBar_GetAttr         { STACKED ULONG MethodID; STACKED ULONG ID; STACKED Tag attr; STACKED ULONG *storage; };
struct MUIP_TheBar_Sort            { STACKED ULONG MethodID; STACKED LONG obj[1]; };
struct MUIP_TheBar_Remove          { STACKED ULONG MethodID; STACKED ULONG ID; };
struct MUIP_TheBar_GetDragImage    { STACKED ULONG MethodID; STACKED ULONG horiz; STACKED ULONG flags; };
struct MUIP_TheBar_Notify          { STACKED ULONG MethodID; STACKED ULONG ID; STACKED Tag attr; STACKED ULONG value; STACKED Object *dest; STACKED ULONG followParams; /* ... */ };
struct MUIP_TheBar_KillNotify      { STACKED ULONG MethodID; STACKED ULONG ID; STACKED Tag attr; STACKED Object *dest; };
struct MUIP_TheBar_NoNotifySetAttr { STACKED ULONG MethodID; STACKED ULONG ID; STACKED Tag attr; STACKED ULONG value; };

/* MUIM_TheBar_SetAttr, MUIM_TheBar_NoNotifySetAttr, MUIM_TheBar_GetAttr attributes */
#define MUIV_TheBar_Attr_Hide      (TBTAGBASE+0) /* v11 */
#define MUIV_TheBar_Attr_Sleep     (TBTAGBASE+1) /* v11 */
#define MUIV_TheBar_Attr_Disabled  (TBTAGBASE+2) /* v11 */
#define MUIV_TheBar_Attr_Selected  (TBTAGBASE+3) /* v11 */

/*
** Compatibility: the above are not "real" attributes,
** but just arguments of a method, so they must be MUIV_
*/
#define MUIA_TheBar_Attr_Hide      MUIV_TheBar_Attr_Hide
#define MUIA_TheBar_Attr_Sleep     MUIV_TheBar_Attr_Sleep
#define MUIA_TheBar_Attr_Disabled  MUIV_TheBar_Attr_Disabled
#define MUIA_TheBar_Attr_Selected  MUIV_TheBar_Attr_Selected

/* MUIM_Notify special Qualifier value */
/*
** This was a bad idea. Don't use it!
*/
#define MUIV_TheBar_Qualifier      (0x49893135)  /* v21 */

/***********************************************************************/
/*
** TheBar.mcc Attributes
*/

#define MUIA_TheBar_MinVer                (TBTAGBASE+10)  /* v11 ULONG,                         [I...]   */
#define MUIA_TheBar_Buttons               (TBTAGBASE+11)  /* v11 struct MUIS_TheBar_Button *,   [I...]   */
#define MUIA_TheBar_Images                (TBTAGBASE+12)  /* v11 struct MUIS_TheBar_Brush **,   [I.G.]   */
#define MUIA_TheBar_Pics                  (TBTAGBASE+13)  /* v11 STRTR *,                       [I...]   */
#define MUIA_TheBar_PicsDrawer            (TBTAGBASE+14)  /* v11 STRTR,                         [I...]   */
#define MUIA_TheBar_ViewMode              (TBTAGBASE+15)  /* v11 UWORD,                         [ISGN]   */
#define MUIA_TheBar_Borderless            (TBTAGBASE+16)  /* v11 BOOL,                          [ISGN]   */
#define MUIA_TheBar_Raised                (TBTAGBASE+17)  /* v11 BOOL,                          [ISGN]   */
#define MUIA_TheBar_Sunny                 (TBTAGBASE+18)  /* v11 BOOL,                          [ISGN]   */
#define MUIA_TheBar_Scaled                (TBTAGBASE+19)  /* v11 BOOL,                          [ISGN]   */
#define MUIA_TheBar_SpacerIndex           (TBTAGBASE+20)  /* v11 ULONG,                         [I.G.]   */
#define MUIA_TheBar_Strip                 (TBTAGBASE+21)  /* v11 STRPTR,                        [I...]   */
#define MUIA_TheBar_StripBrush            (TBTAGBASE+22)  /* v11 struct MUIS_TheBar_Brush *,    [I...]   */
#define MUIA_TheBar_EnableKeys            (TBTAGBASE+23)  /* v11 BOOL,                          [ISGN]   */
#define MUIA_TheBar_TextOnly              (TBTAGBASE+24)  /* v11 BOOL,                          [..G.]   */
#define MUIA_TheBar_LabelPos              (TBTAGBASE+25)  /* v11 ULONG,                         [ISGN]   */
#define MUIA_TheBar_BarPos                (TBTAGBASE+26)  /* v11 ULONG,                         [ISGN]   */
#define MUIA_TheBar_DragBar               (TBTAGBASE+27)  /* v11 BOOL,                          [ISGN]   */
#define MUIA_TheBar_Frame                 (TBTAGBASE+28)  /* v11 BOOL,                          [ISGN]   */
#define MUIA_TheBar_Limbo                 (TBTAGBASE+29)  /* v11 BOOL,                          [.S..]   */
#define MUIA_TheBar_Active                (TBTAGBASE+30)  /* v11 ULONG,                         [ISGN]   */
#define MUIA_TheBar_Columns               (TBTAGBASE+31)  /* v11 ULONG,                         [ISGN]   */
#define MUIA_TheBar_Rows                  (TBTAGBASE+32)  /* v11 ULONG,                         [ISGN]   */
#define MUIA_TheBar_FreeHoriz             (TBTAGBASE+33)  /* v11 ULONG,                         [ISGN]   */
#define MUIA_TheBar_FreeVert              (TBTAGBASE+34)  /* v11 ULONG,                         [ISGN]   */
#define MUIA_TheBar_Free                  (TBTAGBASE+35)  /* v11 ULONG,                         [ISGN]   */
#define MUIA_TheBar_BarSpacer             (TBTAGBASE+36)  /* v11 ULONG,                         [ISGN]   */
#define MUIA_TheBar_RemoveSpacers         (TBTAGBASE+37)  /* v11 ULONG,                         [ISGN]   */
#define MUIA_TheBar_SelImages             (TBTAGBASE+39)  /* v12 struct MUIS_TheBar_Brush **,   [I.G.]   */
#define MUIA_TheBar_DisImages             (TBTAGBASE+40)  /* v12 struct MUIS_TheBar_Brush **,   [I.G.]   */
#define MUIA_TheBar_SelPics               (TBTAGBASE+41)  /* v12 STRTR *,                       [I...]   */
#define MUIA_TheBar_DisPics               (TBTAGBASE+42)  /* v12 STRTR *,                       [I...]   */
#define MUIA_TheBar_SelStrip              (TBTAGBASE+43)  /* v12 STRPTR,                        [I...]   */
#define MUIA_TheBar_DisStrip              (TBTAGBASE+44)  /* v12 STRPTR,                        [I...]   */
#define MUIA_TheBar_SelStripBrush         (TBTAGBASE+45)  /* v12 struct MUIS_TheBar_Brush *,    [I...]   */
#define MUIA_TheBar_DisStripBrush         (TBTAGBASE+46)  /* v12 struct MUIS_TheBar_Brush *,    [I...]   */
#define MUIA_TheBar_StripRows             (TBTAGBASE+47)  /* v12 ULONG,                         [I...]   */
#define MUIA_TheBar_StripCols             (TBTAGBASE+48)  /* v12 ULONG,                         [I...]   */
#define MUIA_TheBar_StripHSpace           (TBTAGBASE+49)  /* v12 ULONG,                         [I...]   */
#define MUIA_TheBar_StripVSpace           (TBTAGBASE+50)  /* v12 ULONG,                         [I...]   */
#define MUIA_TheBar_HorizSpacing          (TBTAGBASE+51)  /* v12 ULONG,                         [I...]   */
#define MUIA_TheBar_VertSpacing           (TBTAGBASE+52)  /* v12 ULONG,                         [I...]   */
#define MUIA_TheBar_BarSpacerSpacing      (TBTAGBASE+53)  /* v12 ULONG,                         [I...]   */
#define MUIA_TheBar_HorizInnerSpacing     (TBTAGBASE+54)  /* v12 ULONG,                         [I...]   */
#define MUIA_TheBar_TopInnerSpacing       (TBTAGBASE+55)  /* v12 ULONG,                         [I...]   */
#define MUIA_TheBar_BottomInnerSpacing    (TBTAGBASE+56)  /* v12 ULONG,                         [I...]   */
#define MUIA_TheBar_LeftBarFrameSpacing   (TBTAGBASE+57)  /* v12 ULONG,                         [I...]   */
#define MUIA_TheBar_RightBarFrameSpacing  (TBTAGBASE+58)  /* v12 ULONG,                         [I...]   */
#define MUIA_TheBar_TopBarFrameSpacing    (TBTAGBASE+59)  /* v12 ULONG,                         [I...]   */
#define MUIA_TheBar_BottomBarFrameSpacing (TBTAGBASE+60)  /* v12 ULONG,                         [I...]   */
#define MUIA_TheBar_HorizTextGfxSpacing   (TBTAGBASE+61)  /* v12 ULONG,                         [I...]   */
#define MUIA_TheBar_VertTextGfxSpacing    (TBTAGBASE+62)  /* v12 ULONG,                         [I...]   */
#define MUIA_TheBar_Precision             (TBTAGBASE+63)  /* v12 ULONG,                         [I...]   */
#define MUIA_TheBar_Scale                 (TBTAGBASE+65)  /* v12 ULONG,                         [I...]   */
#define MUIA_TheBar_DisMode               (TBTAGBASE+66)  /* v12 ULONG,                         [I...]   */
#define MUIA_TheBar_SpecialSelect         (TBTAGBASE+67)  /* v12 BOOL,                          [I...]   */
#define MUIA_TheBar_TextOverUseShine      (TBTAGBASE+68)  /* v12 BOOL,                          [I...]   */
#define MUIA_TheBar_IgnoreSelImages       (TBTAGBASE+69)  /* v12 BOOL,                          [I...]   */
#define MUIA_TheBar_IgnoreDisImages       (TBTAGBASE+70)  /* v12 BOOL,                          [I...]   */
#define MUIA_TheBar_DontMove              (TBTAGBASE+71)  /* v15 BOOL,                          [I...]   */
#define MUIA_TheBar_MouseOver             (TBTAGBASE+72)  /* v18 ULONG,                         [ISGN]   */
#define MUIA_TheBar_NtRaiseActive         (TBTAGBASE+73)  /* v18 BOOL,                          [ISGN]   */
#define MUIA_TheBar_SpacersSize           (TBTAGBASE+74)  /* v18 BOOL,                          [ISGN]   */
#define MUIA_TheBar_Appearance            (TBTAGBASE+75)  /* v19 struct MUIS_TheBar_Appearance, [..G.]   */
#define MUIA_TheBar_IgnoreAppearance      (TBTAGBASE+76)  /* v19 BOOL                           [ISGN]   */

/***********************************************************************/
/*
** TheBar.mcc Attributes values
*/

/* MUIA_TheBar_ViewMode */
enum
{
  MUIV_TheBar_ViewMode_TextGfx,
  MUIV_TheBar_ViewMode_Gfx,
  MUIV_TheBar_ViewMode_Text,

  MUIV_TheBar_ViewMode_Last
};

/* MUIA_TheBar_LabelPos */
enum
{
  MUIV_TheBar_LabelPos_Bottom,
  MUIV_TheBar_LabelPos_Top,
  MUIV_TheBar_LabelPos_Right,
  MUIV_TheBar_LabelPos_Left,

  MUIV_TheBar_LabelPos_Last,
};

/* MUIA_TheBar_BarPos */
enum
{
  MUIV_TheBar_BarPos_Left,
  MUIV_TheBar_BarPos_Center,
  MUIV_TheBar_BarPos_Right,

  MUIV_TheBar_BarPos_Last,
};

#define MUIV_TheBar_BarPos_Up   MUIV_TheBar_BarPos_Left
#define MUIV_TheBar_BarPos_Down MUIV_TheBar_BarPos_Right

/* MUIA_TheBar_RemoveSpacers */
enum
{
  MUIV_TheBar_RemoveSpacers_Bar    = 1<<0, /* v11 */
  MUIV_TheBar_RemoveSpacers_Button = 1<<1, /* v11 */
  MUIV_TheBar_RemoveSpacers_Image  = 1<<2, /* v11 */

  MUIV_TheBar_RemoveSpacers_All    = MUIV_TheBar_RemoveSpacers_Bar|\
                                     MUIV_TheBar_RemoveSpacers_Button|\
                                     MUIV_TheBar_RemoveSpacers_Image,
};

/* MUIA_TheBar_Precision */
enum
{
  MUIV_TheBar_Precision_GUI,
  MUIV_TheBar_Precision_Icon,
  MUIV_TheBar_Precision_Image,
  MUIV_TheBar_Precision_Exact,

  MUIV_TheBar_Precision_Last,
};

/* MUIA_TheBar_DisMode */
enum
{
  MUIV_TheBar_DisMode_Shape,
  MUIV_TheBar_DisMode_Grid,
  MUIV_TheBar_DisMode_FullGrid,
  MUIV_TheBar_DisMode_Sunny,
  MUIV_TheBar_DisMode_Blend,
  MUIV_TheBar_DisMode_BlendGrey,

  MUIV_TheBar_DisMode_Last,
};

/* MUIA_TheBar_SpacersSize */
enum
{
  MUIV_TheBar_SpacersSize_Quarter,
  MUIV_TheBar_SpacersSize_Half,
  MUIV_TheBar_SpacersSize_One,
  MUIV_TheBar_SpacersSize_None,
  MUIV_TheBar_SpacersSize_OnePoint,
  MUIV_TheBar_SpacersSize_TwoPoint,

  MUIV_TheBar_SpacersSize_Last,
};

/* These are private for now */
#define MUIV_TheBar_SpacersSize_PointsFlag   0x40
#define MUIV_TheBar_SpacersSize_Points(x)    (MUIV_TheBar_SpacersSize_PointsFlag | (((ULONG)x) & 0x3f))
#define MUIV_TheBar_SpacersSize_GetPoints(x) (((ULONG)x) & 0x3f)
#define MUIV_TheBar_SpacersSize_IsValid(x)   ((((ULONG)x) & MUIV_TheBar_SpacersSize_PointsFlag) ? ((((ULONG)x) & 0xffffffbf)<=0x3f) : (((ULONG)x)<MUIV_TheBar_SpacersSize_Last))

#define MUIV_TheBar_SkipPic ((STRPTR)(-1))

/***********************************************************************/
/*
** Structures
*/

/*
** MUIA_TheButton_Image is a pointer to this.
** MUIA_TheBar_Images is an array of pointers to this.
**/
struct MUIS_TheBar_Brush
{
  APTR  data;             /* Source data - actually it may be only a UBYTE *            */
  UWORD dataWidth;        /* Width of data                                              */
  UWORD dataHeight;       /* Height of data                                             */
  UWORD dataTotalWidth;   /* Total width of data                                        */
  UWORD left;             /* Left offset in data of this brush                          */
  UWORD top;              /* Top offset in data of this brush                           */
  UWORD width;            /* Width of this brush                                        */
  UWORD height;           /* Height of this brush                                       */
  ULONG *colors;          /* R,G,B or 0x00RRGGBB ULONG table                            */
  ULONG numColors;        /* Number of colors in colors                                 */
  ULONG trColor;          /* Transparent color number; 0<=trColor<256 !                 */
  ULONG compressedSize;   /* If data is byte run 1 compressed, it is its POSITIVE size  */
  ULONG flags;            /* As it says                                                 */
  ULONG reserved[4];      /* Avoid recompilation                                        */
};

enum
{
  BRFLG_ARGB      = 1<<0,
  BRFLG_AlphaMask = 1<<1,
  BRFLG_ColorRGB8 = 1<<2,

  BRFLG_EmptyAlpha = 1<<16,
};

// typo in previous versions
#define BRFLG_EmpytAlpha BRFLG_EmptyAlpha

/*
** MUIA_TheButton_Strip is a pointer to this.
**/
struct MUIS_TheBar_Strip
{
  struct BitMap *normalBM;    /* Normal BitMap        */
  struct BitMap *greyBM;      /* Grey normal BitMap   */
  struct BitMap *mask;        /* Normal mask          */

  struct BitMap *snormalBM;   /* Selected BitMap      */
  struct BitMap *sgreyBM;     /* Selected grey BitMap */
  struct BitMap *smask;       /* Selected mask        */

  struct BitMap *dnormalBM;   /* Disabled BitMap      */
  struct BitMap *dgreyBM;     /* Grey disabled BitMap */
  struct BitMap *dmask;       /* Grey mask            */

  UBYTE *nchunky;
  UBYTE *gchunky;
  UBYTE *snchunky;
  UBYTE *sgchunky;
  UBYTE *dnchunky;
  UBYTE *dgchunky;
};


/*
** MUIA_TheBar_Buttons is an array of this.
*/
struct MUIS_TheBar_Button
{
  ULONG         img;     /* Image index                                          */
  ULONG         ID;      /* Button ID                                            */
  const char    *text;   /* Button label (max TB_MAXLABELLEN) not copied!        */
  const char    *help;   /* Button help not copied!                              */
  ULONG         flags;   /* See below                                            */
  ULONG         exclude; /* Exclude mask                                         */
  struct IClass *_class; /* Easy way of getting a bar of subclassed buttons      */
  Object        *obj;    /* Filled when the button is created                    */
};

/* flags */
enum
{
  MUIV_TheBar_ButtonFlag_NoClick   = 1<<0, /* v11 MUIA_InputMode is MUIV_InputMode_None      */
  MUIV_TheBar_ButtonFlag_Immediate = 1<<1, /* v11 MUIA_InputMode is MUIV_InputMode_Immediate */
  MUIV_TheBar_ButtonFlag_Toggle    = 1<<2, /* v11 MUIA_InputMode is MUIV_InputMode_Toggle    */
  MUIV_TheBar_ButtonFlag_Disabled  = 1<<3, /* v11 MUIA_Disabled is TRUE                      */
  MUIV_TheBar_ButtonFlag_Selected  = 1<<4, /* v11 MUIA_Selected is TRUE                      */
  MUIV_TheBar_ButtonFlag_Sleep     = 1<<5, /* v11 MUIA_ShowMe is FALSE                       */
  MUIV_TheBar_ButtonFlag_Hide      = 1<<6, /* v11 MUIA_ShowMe is FALSE                       */
};

/* Special img values */
#define MUIV_TheBar_End            ((ULONG)-1) /* v11 Ends a MUIS_TheBar_Button array    */
#define MUIV_TheBar_BarSpacer      ((ULONG)-2) /* v11 Add a spacer                       */
#define MUIV_TheBar_ButtonSpacer   ((ULONG)-3) /* v11 Add a space spacer                 */
#define MUIV_TheBar_ImageSpacer    ((ULONG)-4) /* v11 Add an image  spacer               */

/* Returned by MUIM_TheBar_GetDragImage */
struct MUIS_TheBar_DragImage
{
  ULONG                width;
  ULONG                height;
  struct BitMap        *bitMap;
  struct MUI_DragImage *di;       /* Defined in MUIundoc.h */
  ULONG                dummy[8];  /* Avoid recompilation   */
};

/* MUIA_TheBar_Appearance */
struct MUIS_TheBar_Appearance
{
  ULONG viewMode;
  ULONG flags;
  ULONG labelPos;
  ULONG dummy[2];
};

/* flags */
enum
{
  MUIV_TheBar_Appearance_Borderless = 1<<0,
  MUIV_TheBar_Appearance_Raised     = 1<<1,
  MUIV_TheBar_Appearance_Sunny      = 1<<2,
  MUIV_TheBar_Appearance_Scaled     = 1<<3,
  MUIV_TheBar_Appearance_BarSpacer  = 1<<4,
  MUIV_TheBar_Appearance_EnableKeys = 1<<5,
};

/***********************************************************************/
/*
** Methods
*/

#define MUIM_TheButton_Build               (TBUTTAGBASE+0)   /* v13         */
#define MUIM_TheButton_SendNotify          (TBUTTAGBASE+1)   /* v21 PRIVATE */

struct MUIP_TheButton_SendNotify           { STACKED ULONG MethodID; STACKED APTR notify; STACKED ULONG trigVal;};

/***********************************************************************/
/*
** TheButton.mcc Attributes
*/

#define MUIA_TheButton_MinVer              (TBUTTAGBASE+0)   /* v11  ULONG,                         [I...]    */
#define MUIA_TheButton_MouseOver           (TBUTTAGBASE+1)   /* v11  BOOL                           [I...]    */ /* PRIVATE */
#define MUIA_TheButton_Quiet               (TBUTTAGBASE+2)   /* v11  BOOL                           [.S..]    */
#define MUIA_TheButton_Spacer              (TBUTTAGBASE+3)   /* v11  BOOL                           [I.G.]    */ /* PRIVATE */
#define MUIA_TheButton_TheBar              (TBUTTAGBASE+4)   /* v11  Object *,                      [ISG.]    */
#define MUIA_TheButton_Image               (TBUTTAGBASE+5)   /* v11  struct MUIS_TheBar_Brush  *,   [I...]    */
#define MUIA_TheButton_Label               (TBUTTAGBASE+6)   /* v11  STRPTR,                        [I...]    */
#define MUIA_TheButton_InVirtgroup         (TBUTTAGBASE+7)   /* v11  BOOL,                          [I...]    */
#define MUIA_TheButton_ViewMode            (TBUTTAGBASE+8)   /* v11  ULONG,                         [ISGN]    */
#define MUIA_TheButton_Borderless          (TBUTTAGBASE+9)   /* v11  BOOL,                          [I...]    */
#define MUIA_TheButton_Raised              (TBUTTAGBASE+10)  /* v11  BOOL,                          [ISGN]    */
#define MUIA_TheButton_Sunny               (TBUTTAGBASE+11)  /* v11  BOOL,                          [I...]    */
#define MUIA_TheButton_Scaled              (TBUTTAGBASE+12)  /* v11  BOOL,                          [ISGN]    */
#define MUIA_TheButton_NoClick             (TBUTTAGBASE+13)  /* v11  BOOL,                          [I...]    */
#define MUIA_TheButton_Toggle              (TBUTTAGBASE+14)  /* v11  BOOL,                          [I...]    */
#define MUIA_TheButton_Immediate           (TBUTTAGBASE+15)  /* v11  BOOL,                          [I...]    */
#define MUIA_TheButton_EnableKey           (TBUTTAGBASE+16)  /* v11  BOOL,                          [ISG.]    */
#define MUIA_TheButton_LabelPos            (TBUTTAGBASE+17)  /* v11  ULONG,                         [ISGN]    */
#define MUIA_TheButton_SelImage            (TBUTTAGBASE+18)  /* v12  struct MUIS_TheBar_Brush  *,   [I...]    */
#define MUIA_TheButton_DisImage            (TBUTTAGBASE+19)  /* v12  struct MUIS_TheBar_Brush  *,   [I...]    */
#define MUIA_TheButton_HorizTextGfxSpacing (TBUTTAGBASE+20)  /* v12  ULONG,                         [I...]    */
#define MUIA_TheButton_VertTextGfxSpacing  (TBUTTAGBASE+21)  /* v12  ULONG,                         [I...]    */
#define MUIA_TheButton_HorizInnerSpacing   (TBUTTAGBASE+22)  /* v12  ULONG,                         [I...]    */
#define MUIA_TheButton_TopInnerSpacing     (TBUTTAGBASE+23)  /* v12  ULONG,                         [I...]    */
#define MUIA_TheButton_BottomInnerSpacing  (TBUTTAGBASE+24)  /* v12  ULONG,                         [I...]    */
#define MUIA_TheButton_Precision           (TBUTTAGBASE+25)  /* v12  ULONG,                         [I...]    */
#define MUIA_TheButton_Event               (TBUTTAGBASE+26)  /* v12  ULONG,                         [I...]    */
#define MUIA_TheButton_Scale               (TBUTTAGBASE+27)  /* v12  ULONG,                         [I...]    */
#define MUIA_TheButton_DisMode             (TBUTTAGBASE+28)  /* v12  ULONG,                         [I...]    */
#define MUIA_TheButton_SpecialSelect       (TBUTTAGBASE+29)  /* v12  BOOL,                          [I...]    */
#define MUIA_TheButton_TextOverUseShine    (TBUTTAGBASE+30)  /* v12  BOOL,                          [I...]    */
#define MUIA_TheButton_IgnoreSelImages     (TBUTTAGBASE+31)  /* v12  BOOL,                          [I...]    */
#define MUIA_TheButton_IgnoreDisImages     (TBUTTAGBASE+32)  /* v12  BOOL,                          [I...]    */
#define MUIA_TheButton_Strip               (TBUTTAGBASE+33)  /* v13  struct MUIS_TheBar_Strip *,    [I...]    */
#define MUIA_TheButton_DontMove            (TBUTTAGBASE+34)  /* v15  BOOL,                          [I...]    */
#define MUIA_TheButton_ID                  (TBUTTAGBASE+35)  /* v18  ULONG,                         [I...]    */
#define MUIA_TheButton_NtRaiseActive       (TBUTTAGBASE+36)  /* v18  ULONG,                         [I...]    */
#define MUIA_TheButton_StripRows           (TBUTTAGBASE+37)  /* v20  ULONG,                         [I...]    */
#define MUIA_TheButton_StripCols           (TBUTTAGBASE+38)  /* v20  ULONG,                         [I...]    */
#define MUIA_TheButton_StripHorizSpace     (TBUTTAGBASE+39)  /* v20  ULONG,                         [I...]    */
#define MUIA_TheButton_StripVertSpace      (TBUTTAGBASE+40)  /* v20  ULONG,                         [I...]    */
#define MUIA_TheButton_NotifyList          (TBUTTAGBASE+41)  /* v21  struct MinList *,              [..G.]    */ /* PRIVATE */

/***********************************************************************/
/*
** TheButton.mcc Attributes values
*/

/* MUIA_TheButton_ViewMode */
enum
{
  MUIV_TheButton_ViewMode_TextGfx,
  MUIV_TheButton_ViewMode_Gfx,
  MUIV_TheButton_ViewMode_Text,

  MUIV_TheButton_ViewMode_Last
};

/* MUIA_TheButton_LabelPos */
enum
{
  MUIV_TheButton_LabelPos_Bottom,
  MUIV_TheButton_LabelPos_Top,
  MUIV_TheButton_LabelPos_Right,
  MUIV_TheButton_LabelPos_Left,

  MUIV_TheButton_LabelPos_Last
};

/* MUIA_TheButton_Spacer */
enum
{
  MUIV_TheButton_Spacer_None,
  MUIV_TheButton_Spacer_Bar,
  MUIV_TheButton_Spacer_Button,
  MUIV_TheButton_Spacer_Image,
  MUIV_TheButton_Spacer_DragBar,
};

/* MUICFG_TheButton_FrameStyle */
enum
{
  MUIV_TheButton_FrameStyle_Recessed,
  MUIV_TheButton_FrameStyle_Normal,
};

/* MUIA_TheButton_Event */
enum
{
  MUIV_TheButton_Event_IntuiTicks,
  MUIV_TheButton_Event_MouseMove,
  MUIV_TheButton_Event_MouseObject,

  MUIV_TheButton_Event_Last,
};

/* MUIA_TheButton_Precision */
enum
{
  MUIV_TheButton_Precision_GUI,
  MUIV_TheButton_Precision_Icon,
  MUIV_TheButton_Precision_Image,
  MUIV_TheButton_Precision_Exact,

  MUIV_TheButton_Precision_Last,
};

/* MUIA_TheButton_DisMode */
enum
{
  MUIV_TheButton_DisMode_Shape,
  MUIV_TheButton_DisMode_Grid,
  MUIV_TheButton_DisMode_FullGrid,
  MUIV_TheButton_DisMode_Sunny,
  MUIV_TheButton_DisMode_Blend,
  MUIV_TheButton_DisMode_BlendGrey,

  MUIV_TheButton_DisMode_Last,
};

/***********************************************************************/
/*
** TheButton.mcc Misc
*/

/* MUIA_TheButton_Label max size */
#define TB_MAXLABELLEN 32

/***********************************************************************/

#if !defined(__AROS__) && defined(__PPC__)
  #if defined(__GNUC__)
    #pragma pack()
  #elif defined(__VBCC__)
    #pragma default-align
  #endif
#endif

#endif /* THEBAR_MCC_H */
