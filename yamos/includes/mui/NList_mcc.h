/*
**  NList.mcc (c) Copyright 1996-2003 by Gilles Masson, C.Scholling, P.Gruchala, S.Bauer and J.Langner
**
**  Registered MUI class, Serial Num: 1d51     0x9d510030 to 0x9d5100A0 / 0x9d5100C0 to 0x9d5100FF
**  *** use only YOUR OWN Serial Number for your public custom class ***
**
**  $Id$
**
**  NList_mcc.h
*/

#ifndef MUI_NList_MCC_H
#define MUI_NList_MCC_H

#ifndef LIBRARIES_MUI_H
#include <libraries/mui.h>
#endif

#include "amiga-align.h"

/* MUI Prop and Scroller classes stuff which is still not in libraries/mui.h  (in MUI3.8) */
/* it gives to the prop object it's increment value */
#ifndef MUIA_Prop_DeltaFactor
#define MUIA_Prop_DeltaFactor 0x80427C5E
#endif


#define MUIC_NList "NList.mcc"
#define NListObject MUI_NewObject(MUIC_NList


/* Attributes */

#define MUIA_NList_TypeSelect               0x9d510030 /* GM  is.  LONG              */
#define MUIA_NList_Prop_DeltaFactor         0x9d510031 /* GM  ..gn LONG              */
#define MUIA_NList_Horiz_DeltaFactor        0x9d510032 /* GM  ..gn LONG              */

#define MUIA_NList_Horiz_First              0x9d510033 /* GM  .sgn LONG              */
#define MUIA_NList_Horiz_Visible            0x9d510034 /* GM  ..gn LONG              */
#define MUIA_NList_Horiz_Entries            0x9d510035 /* GM  ..gn LONG              */

#define MUIA_NList_Prop_First               0x9d510036 /* GM  .sgn LONG              */
#define MUIA_NList_Prop_Visible             0x9d510037 /* GM  ..gn LONG              */
#define MUIA_NList_Prop_Entries             0x9d510038 /* GM  ..gn LONG              */

#define MUIA_NList_TitlePen                 0x9d510039 /* GM  isg  LONG              */
#define MUIA_NList_ListPen                  0x9d51003a /* GM  isg  LONG              */
#define MUIA_NList_SelectPen                0x9d51003b /* GM  isg  LONG              */
#define MUIA_NList_CursorPen                0x9d51003c /* GM  isg  LONG              */
#define MUIA_NList_UnselCurPen              0x9d51003d /* GM  isg  LONG              */

#define MUIA_NList_ListBackground           0x9d51003e /* GM  isg  LONG              */
#define MUIA_NList_TitleBackground          0x9d51003f /* GM  isg  LONG              */
#define MUIA_NList_SelectBackground         0x9d510040 /* GM  isg  LONG              */
#define MUIA_NList_CursorBackground         0x9d510041 /* GM  isg  LONG              */
#define MUIA_NList_UnselCurBackground       0x9d510042 /* GM  isg  LONG              */

#define MUIA_NList_MultiClick               0x9d510043 /* GM  ..gn LONG              */

#define MUIA_NList_DefaultObjectOnClick     0x9d510044 /* GM  is.  BOOL              */

#define MUIA_NList_ClickColumn              0x9d510045 /* GM  ..g  LONG              */
#define MUIA_NList_DefClickColumn           0x9d510046 /* GM  isg  LONG              */
#define MUIA_NList_DoubleClick              0x9d510047 /* GM  ..gn LONG              */
#define MUIA_NList_DragType                 0x9d510048 /* GM  isg  LONG              */
#define MUIA_NList_Input                    0x9d510049 /* GM  isg  BOOL              */
#define MUIA_NList_MultiSelect              0x9d51004a /* GM  is.  LONG              */
#define MUIA_NList_SelectChange             0x9d51004b /* GM  ...n BOOL              */

#define MUIA_NList_Active                   0x9d51004c /* GM  isgn LONG              */
#define MUIA_NList_AdjustHeight             0x9d51004d /* GM  i..  BOOL              */
#define MUIA_NList_AdjustWidth              0x9d51004e /* GM  i..  BOOL              */
#define MUIA_NList_AutoVisible              0x9d51004f /* GM  isg  BOOL              */
#define MUIA_NList_CompareHook              0x9d510050 /* GM  is.  struct Hook *     */
#define MUIA_NList_ConstructHook            0x9d510051 /* GM  is.  struct Hook *     */
#define MUIA_NList_DestructHook             0x9d510052 /* GM  is.  struct Hook *     */
#define MUIA_NList_DisplayHook              0x9d510053 /* GM  is.  struct Hook *     */
#define MUIA_NList_DragSortable             0x9d510054 /* GM  isg  BOOL              */
#define MUIA_NList_DropMark                 0x9d510055 /* GM  ..g  LONG              */
#define MUIA_NList_Entries                  0x9d510056 /* GM  ..gn LONG              */
#define MUIA_NList_First                    0x9d510057 /* GM  isgn LONG              */
#define MUIA_NList_Format                   0x9d510058 /* GM  isg  STRPTR            */
#define MUIA_NList_InsertPosition           0x9d510059 /* GM  ..gn LONG              */
#define MUIA_NList_MinLineHeight            0x9d51005a /* GM  is.  LONG              */
#define MUIA_NList_MultiTestHook            0x9d51005b /* GM  is.  struct Hook *     */
#define MUIA_NList_Pool                     0x9d51005c /* GM  i..  APTR              */
#define MUIA_NList_PoolPuddleSize           0x9d51005d /* GM  i..  ULONG             */
#define MUIA_NList_PoolThreshSize           0x9d51005e /* GM  i..  ULONG             */
#define MUIA_NList_Quiet                    0x9d51005f /* GM  .s.  BOOL              */
#define MUIA_NList_ShowDropMarks            0x9d510060 /* GM  isg  BOOL              */
#define MUIA_NList_SourceArray              0x9d510061 /* GM  i..  APTR *            */
#define MUIA_NList_Title                    0x9d510062 /* GM  isg  char *            */
#define MUIA_NList_Visible                  0x9d510063 /* GM  ..g  LONG              */
#define MUIA_NList_CopyEntryToClipHook      0x9d510064 /* GM  is.  struct Hook *     */
#define MUIA_NList_KeepActive               0x9d510065 /* GM  .s.  Obj *             */
#define MUIA_NList_MakeActive               0x9d510066 /* GM  .s.  Obj *             */
#define MUIA_NList_SourceString             0x9d510067 /* GM  i..  char *            */
#define MUIA_NList_CopyColumnToClipHook     0x9d510068 /* GM  is.  struct Hook *     */
#define MUIA_NList_ListCompatibility        0x9d510069 /* GM  ...  OBSOLETE          */
#define MUIA_NList_AutoCopyToClip           0x9d51006A /* GM  is.  BOOL              */
#define MUIA_NList_TabSize                  0x9d51006B /* GM  isg  ULONG             */
#define MUIA_NList_SkipChars                0x9d51006C /* GM  isg  char *            */
#define MUIA_NList_DisplayRecall            0x9d51006D /* GM  .g.  BOOL              */
#define MUIA_NList_PrivateData              0x9d51006E /* GM  isg  APTR              */
#define MUIA_NList_EntryValueDependent      0x9d51006F /* GM  isg  BOOL              */

#define MUIA_NList_StackCheck               0x9d510097 /* GM  i..  BOOL              */
#define MUIA_NList_WordSelectChars          0x9d510098 /* GM  isg  char *            */
#define MUIA_NList_EntryClick               0x9d510099 /* GM  ..gn LONG              */
#define MUIA_NList_DragColOnly              0x9d51009A /* GM  isg  LONG              */
#define MUIA_NList_TitleClick               0x9d51009B /* GM  isgn LONG              */
#define MUIA_NList_DropType                 0x9d51009C /* GM  ..g  LONG              */
#define MUIA_NList_ForcePen                 0x9d51009D /* GM  isg  LONG              */
#define MUIA_NList_SourceInsert             0x9d51009E /* GM  i..  struct MUIP_NList_InsertWrap *   */
#define MUIA_NList_TitleSeparator           0x9d51009F /* GM  isg  BOOL              */

#define MUIA_NList_SortType2                0x9d5100ED /* GM  isgn LONG              */
#define MUIA_NList_TitleClick2              0x9d5100EE /* GM  isgn LONG              */
#define MUIA_NList_TitleMark2               0x9d5100EF /* GM  isg  LONG              */
#define MUIA_NList_MultiClickAlone          0x9d5100F0 /* GM  ..gn LONG              */
#define MUIA_NList_TitleMark                0x9d5100F1 /* GM  isg  LONG              */
#define MUIA_NList_DragSortInsert           0x9d5100F2 /* GM  ..gn LONG              */
#define MUIA_NList_MinColSortable           0x9d5100F3 /* GM  isg  LONG              */
#define MUIA_NList_Imports                  0x9d5100F4 /* GM  isg  LONG              */
#define MUIA_NList_Exports                  0x9d5100F5 /* GM  isg  LONG              */
#define MUIA_NList_Columns                  0x9d5100F6 /* GM  isgn BYTE *            */
#define MUIA_NList_LineHeight               0x9d5100F7 /* GM  ..gn LONG              */
#define MUIA_NList_ButtonClick              0x9d5100F8 /* GM  ..gn LONG              */
#define MUIA_NList_CopyEntryToClipHook2     0x9d5100F9 /* GM  is.  struct Hook *     */
#define MUIA_NList_CopyColumnToClipHook2    0x9d5100FA /* GM  is.  struct Hook *     */
#define MUIA_NList_CompareHook2             0x9d5100FB /* GM  is.  struct Hook *     */
#define MUIA_NList_ConstructHook2           0x9d5100FC /* GM  is.  struct Hook *     */
#define MUIA_NList_DestructHook2            0x9d5100FD /* GM  is.  struct Hook *     */
#define MUIA_NList_DisplayHook2             0x9d5100FE /* GM  is.  struct Hook *     */
#define MUIA_NList_SortType                 0x9d5100FF /* GM  isgn LONG              */


#define MUIA_NLIMG_EntryCurrent             MUIA_NList_First   /* LONG (special for nlist custom image object) */
#define MUIA_NLIMG_EntryHeight              MUIA_NList_Visible /* LONG (special for nlist custom image object) */

#define MUIA_NList_VertDeltaFactor          MUIA_NList_Prop_DeltaFactor   /* OBSOLETE NAME */
#define MUIA_NList_HorizDeltaFactor         MUIA_NList_Horiz_DeltaFactor  /* OBSOLETE NAME */


/* Attributes special datas */

#define MUIV_NList_TypeSelect_Line        0
#define MUIV_NList_TypeSelect_Char        1

#define MUIV_NList_Font                 -20
#define MUIV_NList_Font_Little          -21
#define MUIV_NList_Font_Fixed           -22

#define MUIV_NList_ConstructHook_String  -1
#define MUIV_NList_DestructHook_String   -1

#define MUIV_NList_Active_Off            -1
#define MUIV_NList_Active_Top            -2
#define MUIV_NList_Active_Bottom         -3
#define MUIV_NList_Active_Up             -4
#define MUIV_NList_Active_Down           -5
#define MUIV_NList_Active_PageUp         -6
#define MUIV_NList_Active_PageDown       -7

#define MUIV_NList_First_Top             -2
#define MUIV_NList_First_Bottom          -3
#define MUIV_NList_First_Up              -4
#define MUIV_NList_First_Down            -5
#define MUIV_NList_First_PageUp          -6
#define MUIV_NList_First_PageDown        -7
#define MUIV_NList_First_Up2             -8
#define MUIV_NList_First_Down2           -9
#define MUIV_NList_First_Up4            -10
#define MUIV_NList_First_Down4          -11

#define MUIV_NList_Horiz_First_Start     -2
#define MUIV_NList_Horiz_First_End       -3
#define MUIV_NList_Horiz_First_Left      -4
#define MUIV_NList_Horiz_First_Right     -5
#define MUIV_NList_Horiz_First_PageLeft  -6
#define MUIV_NList_Horiz_First_PageRight -7
#define MUIV_NList_Horiz_First_Left2     -8
#define MUIV_NList_Horiz_First_Right2    -9
#define MUIV_NList_Horiz_First_Left4    -10
#define MUIV_NList_Horiz_First_Right4   -11

#define MUIV_NList_MultiSelect_None       0
#define MUIV_NList_MultiSelect_Default    1
#define MUIV_NList_MultiSelect_Shifted    2
#define MUIV_NList_MultiSelect_Always     3

#define MUIV_NList_Insert_Top             0
#define MUIV_NList_Insert_Active         -1
#define MUIV_NList_Insert_Sorted         -2
#define MUIV_NList_Insert_Bottom         -3
#define MUIV_NList_Insert_Flag_Raw       (1<<0)

#define MUIV_NList_Remove_First           0
#define MUIV_NList_Remove_Active         -1
#define MUIV_NList_Remove_Last           -2
#define MUIV_NList_Remove_Selected       -3

#define MUIV_NList_Select_Off             0
#define MUIV_NList_Select_On              1
#define MUIV_NList_Select_Toggle          2
#define MUIV_NList_Select_Ask             3

#define MUIV_NList_GetEntry_Active       -1
#define MUIV_NList_GetEntryInfo_Line     -2

#define MUIV_NList_Select_Active         -1
#define MUIV_NList_Select_All            -2

#define MUIV_NList_Redraw_Active         -1
#define MUIV_NList_Redraw_All            -2
#define MUIV_NList_Redraw_Title          -3
#define MUIV_NList_Redraw_VisibleCols    -5

#define MUIV_NList_Move_Top               0
#define MUIV_NList_Move_Active           -1
#define MUIV_NList_Move_Bottom           -2
#define MUIV_NList_Move_Next             -3 /* only valid for second parameter (and not with Move_Selected) */
#define MUIV_NList_Move_Previous         -4 /* only valid for second parameter (and not with Move_Selected) */
#define MUIV_NList_Move_Selected         -5 /* only valid for first parameter */

#define MUIV_NList_Exchange_Top           0
#define MUIV_NList_Exchange_Active       -1
#define MUIV_NList_Exchange_Bottom       -2
#define MUIV_NList_Exchange_Next         -3 /* only valid for second parameter */
#define MUIV_NList_Exchange_Previous     -4 /* only valid for second parameter */

#define MUIV_NList_Jump_Top               0
#define MUIV_NList_Jump_Active           -1
#define MUIV_NList_Jump_Bottom           -2
#define MUIV_NList_Jump_Up               -4
#define MUIV_NList_Jump_Down             -3

#define MUIV_NList_NextSelected_Start    -1
#define MUIV_NList_NextSelected_End      -1

#define MUIV_NList_PrevSelected_Start    -1
#define MUIV_NList_PrevSelected_End      -1

#define MUIV_NList_DragType_None          0
#define MUIV_NList_DragType_Default       1
#define MUIV_NList_DragType_Immediate     2
#define MUIV_NList_DragType_Borders       3
#define MUIV_NList_DragType_Qualifier     4

#define MUIV_NList_CopyToClip_Active     -1
#define MUIV_NList_CopyToClip_Selected   -2
#define MUIV_NList_CopyToClip_All        -3
#define MUIV_NList_CopyToClip_Entries    -4
#define MUIV_NList_CopyToClip_Entry      -5
#define MUIV_NList_CopyToClip_Strings    -6
#define MUIV_NList_CopyToClip_String     -7

#define MUIV_NList_CopyTo_Active         -1
#define MUIV_NList_CopyTo_Selected       -2
#define MUIV_NList_CopyTo_All            -3
#define MUIV_NList_CopyTo_Entries        -4
#define MUIV_NList_CopyTo_Entry          -5

#define MUIV_NLCT_Success                 0
#define MUIV_NLCT_OpenErr                 1
#define MUIV_NLCT_WriteErr                2
#define MUIV_NLCT_Failed                  3

#define MUIV_NList_ForcePen_On            1
#define MUIV_NList_ForcePen_Off           0
#define MUIV_NList_ForcePen_Default      -1

#define MUIV_NList_DropType_Mask          0x00FF
#define MUIV_NList_DropType_None          0
#define MUIV_NList_DropType_Above         1
#define MUIV_NList_DropType_Below         2
#define MUIV_NList_DropType_Onto          3

#define MUIV_NList_DoMethod_Active       -1
#define MUIV_NList_DoMethod_Selected     -2
#define MUIV_NList_DoMethod_All          -3

#define MUIV_NList_DoMethod_Entry        -1
#define MUIV_NList_DoMethod_Self         -2
#define MUIV_NList_DoMethod_App          -3

#define MUIV_NList_EntryValue             (MUIV_TriggerValue+0x100)
#define MUIV_NList_EntryPosValue          (MUIV_TriggerValue+0x102)
#define MUIV_NList_SelfValue              (MUIV_TriggerValue+0x104)
#define MUIV_NList_AppValue               (MUIV_TriggerValue+0x106)

#define MUIV_NList_ColWidth_All          -1
#define MUIV_NList_ColWidth_Default      -1
#define MUIV_NList_ColWidth_Get          -2

#define MUIV_NList_ContextMenu_Default    0x9d510031
#define MUIV_NList_ContextMenu_TopOnly    0x9d510033
#define MUIV_NList_ContextMenu_BarOnly    0x9d510035
#define MUIV_NList_ContextMenu_Bar_Top    0x9d510037
#define MUIV_NList_ContextMenu_Always     0x9d510039
#define MUIV_NList_ContextMenu_Never      0x9d51003b

#define MUIV_NList_Menu_DefWidth_This     0x9d51003d
#define MUIV_NList_Menu_DefWidth_All      0x9d51003f
#define MUIV_NList_Menu_DefOrder_This     0x9d510041
#define MUIV_NList_Menu_DefOrder_All      0x9d510043
#define MUIV_NList_Menu_Default_This      MUIV_NList_Menu_DefWidth_This
#define MUIV_NList_Menu_Default_All       MUIV_NList_Menu_DefWidth_All

#define MUIV_NList_SortType_None          0xF0000000
#define MUIV_NList_SortTypeAdd_None       0x00000000
#define MUIV_NList_SortTypeAdd_2Values    0x80000000
#define MUIV_NList_SortTypeAdd_4Values    0x40000000
#define MUIV_NList_SortTypeAdd_Mask       0xC0000000
#define MUIV_NList_SortTypeValue_Mask     0x3FFFFFFF

#define MUIV_NList_Sort3_SortType_Both    0x00000000
#define MUIV_NList_Sort3_SortType_1       0x00000001
#define MUIV_NList_Sort3_SortType_2       0x00000002

#define MUIV_NList_Quiet_None             0
#define MUIV_NList_Quiet_Full            -1
#define MUIV_NList_Quiet_Visual          -2

#define MUIV_NList_Imports_Active         (1 << 0)
#define MUIV_NList_Imports_Selected       (1 << 1)
#define MUIV_NList_Imports_First          (1 << 2)
#define MUIV_NList_Imports_ColWidth       (1 << 3)
#define MUIV_NList_Imports_ColOrder       (1 << 4)
#define MUIV_NList_Imports_TitleMark      (1 << 7)
#define MUIV_NList_Imports_Cols           0x000000F8
#define MUIV_NList_Imports_All            0x0000FFFF

#define MUIV_NList_Exports_Active         (1 << 0)
#define MUIV_NList_Exports_Selected       (1 << 1)
#define MUIV_NList_Exports_First          (1 << 2)
#define MUIV_NList_Exports_ColWidth       (1 << 3)
#define MUIV_NList_Exports_ColOrder       (1 << 4)
#define MUIV_NList_Exports_TitleMark      (1 << 7)
#define MUIV_NList_Exports_Cols           0x000000F8
#define MUIV_NList_Exports_All            0x0000FFFF

#define MUIV_NList_TitleMark_ColMask      0x000000FF
#define MUIV_NList_TitleMark_TypeMask     0xF0000000
#define MUIV_NList_TitleMark_None         0xF0000000
#define MUIV_NList_TitleMark_Down         0x00000000
#define MUIV_NList_TitleMark_Up           0x80000000
#define MUIV_NList_TitleMark_Box          0x40000000
#define MUIV_NList_TitleMark_Circle       0xC0000000

#define MUIV_NList_TitleMark2_ColMask     0x000000FF
#define MUIV_NList_TitleMark2_TypeMask    0xF0000000
#define MUIV_NList_TitleMark2_None        0xF0000000
#define MUIV_NList_TitleMark2_Down        0x00000000
#define MUIV_NList_TitleMark2_Up          0x80000000
#define MUIV_NList_TitleMark2_Box         0x40000000
#define MUIV_NList_TitleMark2_Circle      0xC0000000

#define MUIV_NList_SetColumnCol_Default  -1

#define MUIV_NList_GetPos_Start          -1
#define MUIV_NList_GetPos_End            -1

#define	MUIV_NList_SelectChange_Flag_Multi (1 << 0)

#define MUIV_NList_UseImage_All         (-1)

/* Structs */

struct BitMapImage
{
  ULONG    control;   /* should be == to MUIM_NList_CreateImage for a valid BitMapImage struct */
  WORD     width;     /* if control == MUIA_Image_Spec then obtainpens is a pointer to an Object */
  WORD     height;
  WORD    *obtainpens;
  PLANEPTR mask;
  struct BitMap imgbmp;
  LONG     flags;
};


struct MUI_NList_TestPos_Result
{
  LONG  entry;   /* number of entry, -1 if mouse not over valid entry */
  WORD  column;  /* numer of column, -1 if no valid column */
  UWORD flags;   /* not in the list, see below */
  WORD  xoffset; /* x offset in column */
  WORD  yoffset; /* y offset of mouse click from center of line */
  WORD  preparse;     /* 2 if in column preparse string, 1 if in entry preparse string, else 0 */
  WORD  char_number;  /* the number of the clicked char in column, -1 if no valid */
  WORD  char_xoffset; /* x offset of mouse clicked from left of char if positive */
};                    /* and left of next char if negative. If there is no char there */
                      /* negative if from left of first char else from right of last one */

#define MUI_NLPR_ABOVE  (1<<0)
#define MUI_NLPR_BELOW  (1<<1)
#define MUI_NLPR_LEFT   (1<<2)
#define MUI_NLPR_RIGHT  (1<<3)
#define MUI_NLPR_BAR    (1<<4)  /* if between two columns you'll get the left
                                   column number of both, and that flag */
#define MUI_NLPR_TITLE  (1<<5)  /* if clicked on title, only column, xoffset and yoffset (and MUI_NLPR_BAR)
                                    are valid (you'll get MUI_NLPR_ABOVE too) */
#define MUI_NLPR_ONTOP  (1<<6)  /* it is on title/half of first visible entry */


struct MUI_NList_GetEntryInfo
{
  LONG pos;             /* num of entry you want info about */
  LONG line;            /* real line number */
  LONG entry_pos;       /* entry num of returned entry ptr */
  APTR entry;           /* entry pointer */
  LONG wrapcol;         /* NOWRAP, WRAPCOLx, or WRAPPED|WRAPCOLx */
  LONG charpos;         /* start char number in string (unused if NOWRAP) */
  LONG charlen;         /* string lenght (unused if NOWRAP) */
};

#define NOWRAP          0x00
#define WRAPCOL0        0x01
#define WRAPCOL1        0x02
#define WRAPCOL2        0x04
#define WRAPCOL3        0x08
#define WRAPCOL4        0x10
#define WRAPCOL5        0x20
#define WRAPCOL6        0x40
#define WRAPPED         0x80


struct MUI_NList_GetSelectInfo
{
  LONG start;        /* num of first selected *REAL* entry/line (first of wrapped from which start is issued) */
  LONG end;          /* num of last selected *REAL* entry/line (first of wrapped from which start is issued) */
  LONG num;          /* not used */
  LONG start_column; /* column of start of selection in 'start' entry */
  LONG end_column;   /* column of end of selection in 'end' entry */
  LONG start_pos;    /* char pos of start of selection in 'start_column' entry */
  LONG end_pos;      /* char pos of end of selection in 'end_column' entry */
  LONG vstart;       /* num of first visually selected entry */
  LONG vend;         /* num of last visually selected entry */
  LONG vnum;         /* number of visually selected entries */
};
/* NOTE that vstart==start, vend==end in all cases if no wrapping is used */

/* Methods */

#define MUIM_NList_Clear              0x9d510070 /* GM */
#define MUIM_NList_CreateImage        0x9d510071 /* GM */
#define MUIM_NList_DeleteImage        0x9d510072 /* GM */
#define MUIM_NList_Exchange           0x9d510073 /* GM */
#define MUIM_NList_GetEntry           0x9d510074 /* GM */
#define MUIM_NList_Insert             0x9d510075 /* GM */
#define MUIM_NList_InsertSingle       0x9d510076 /* GM */
#define MUIM_NList_Jump               0x9d510077 /* GM */
#define MUIM_NList_Move               0x9d510078 /* GM */
#define MUIM_NList_NextSelected       0x9d510079 /* GM */
#define MUIM_NList_Redraw             0x9d51007a /* GM */
#define MUIM_NList_Remove             0x9d51007b /* GM */
#define MUIM_NList_Select             0x9d51007c /* GM */
#define MUIM_NList_Sort               0x9d51007d /* GM */
#define MUIM_NList_TestPos            0x9d51007e /* GM */
#define MUIM_NList_CopyToClip         0x9d51007f /* GM */
#define MUIM_NList_UseImage           0x9d510080 /* GM */
#define MUIM_NList_ReplaceSingle      0x9d510081 /* GM */
#define MUIM_NList_InsertWrap         0x9d510082 /* GM */
#define MUIM_NList_InsertSingleWrap   0x9d510083 /* GM */
#define MUIM_NList_GetEntryInfo       0x9d510084 /* GM */
#define MUIM_NList_QueryBeginning     0x9d510085 /* Obsolete */
#define MUIM_NList_GetSelectInfo      0x9d510086 /* GM */
#define MUIM_NList_CopyTo             0x9d510087 /* GM */
#define MUIM_NList_DropType           0x9d510088 /* GM */
#define MUIM_NList_DropDraw           0x9d510089 /* GM */
#define MUIM_NList_RedrawEntry        0x9d51008a /* GM */
#define MUIM_NList_DoMethod           0x9d51008b /* GM */
#define MUIM_NList_ColWidth           0x9d51008c /* GM */
#define MUIM_NList_ContextMenuBuild   0x9d51008d /* GM */
#define MUIM_NList_DropEntryDrawErase 0x9d51008e /* GM */
#define MUIM_NList_ColToColumn        0x9d51008f /* GM */
#define MUIM_NList_ColumnToCol        0x9d510091 /* GM */
#define MUIM_NList_Sort2              0x9d510092 /* GM */
#define MUIM_NList_PrevSelected       0x9d510093 /* GM */
#define MUIM_NList_SetColumnCol       0x9d510094 /* GM */
#define MUIM_NList_Sort3              0x9d510095 /* GM */
#define MUIM_NList_GetPos             0x9d510096 /* GM */
#define MUIM_NList_SelectChange       0x9d5100A0 /* GM */
#define MUIM_NList_Construct          0x9d5100A1 /* GM */
#define MUIM_NList_Destruct           0x9d5100A2 /* GM */
#define MUIM_NList_Compare            0x9d5100A3 /* GM */
#define MUIM_NList_Display            0x9d5100A4 /* GM */
struct  MUIP_NList_Clear              { ULONG MethodID; };
struct  MUIP_NList_CreateImage        { ULONG MethodID; Object *obj; ULONG flags; };
struct  MUIP_NList_DeleteImage        { ULONG MethodID; APTR listimg; };
struct  MUIP_NList_Exchange           { ULONG MethodID; LONG pos1; LONG pos2; };
struct  MUIP_NList_GetEntry           { ULONG MethodID; LONG pos; APTR *entry; };
struct  MUIP_NList_Insert             { ULONG MethodID; APTR *entries; LONG count; LONG pos; ULONG flags; };
struct  MUIP_NList_InsertSingle       { ULONG MethodID; APTR entry; LONG pos; };
struct  MUIP_NList_Jump               { ULONG MethodID; LONG pos; };
struct  MUIP_NList_Move               { ULONG MethodID; LONG from; LONG to; };
struct  MUIP_NList_NextSelected       { ULONG MethodID; LONG *pos; };
struct  MUIP_NList_Redraw             { ULONG MethodID; LONG pos; };
struct  MUIP_NList_Remove             { ULONG MethodID; LONG pos; };
struct  MUIP_NList_Select             { ULONG MethodID; LONG pos; LONG seltype; LONG *state; };
struct  MUIP_NList_Sort               { ULONG MethodID; };
struct  MUIP_NList_TestPos            { ULONG MethodID; LONG x; LONG y; struct MUI_NList_TestPos_Result *res; };
struct  MUIP_NList_CopyToClip         { ULONG MethodID; LONG pos; ULONG clipnum; APTR *entries; struct Hook *hook; };
struct  MUIP_NList_UseImage           { ULONG MethodID; Object *obj; LONG imgnum; ULONG flags; };
struct  MUIP_NList_ReplaceSingle      { ULONG MethodID; APTR entry; LONG pos; LONG wrapcol; LONG align; };
struct  MUIP_NList_InsertWrap         { ULONG MethodID; APTR *entries; LONG count; LONG pos; LONG wrapcol; LONG align; ULONG flags; };
struct  MUIP_NList_InsertSingleWrap   { ULONG MethodID; APTR entry; LONG pos; LONG wrapcol; LONG align; };
struct  MUIP_NList_GetEntryInfo       { ULONG MethodID; struct MUI_NList_GetEntryInfo *res; };
struct  MUIP_NList_QueryBeginning     { ULONG MethodID; };
struct  MUIP_NList_GetSelectInfo      { ULONG MethodID; struct MUI_NList_GetSelectInfo *res; };
struct  MUIP_NList_CopyTo             { ULONG MethodID; LONG pos; char *filename; APTR *result; APTR *entries; };
struct  MUIP_NList_DropType           { ULONG MethodID; LONG *pos; LONG *type; LONG minx, maxx, miny, maxy; LONG mousex, mousey; };
struct  MUIP_NList_DropDraw           { ULONG MethodID; LONG pos; LONG type; LONG minx, maxx, miny, maxy; };
struct  MUIP_NList_RedrawEntry        { ULONG MethodID; APTR entry; };
struct  MUIP_NList_DoMethod           { ULONG MethodID; LONG pos; APTR DestObj; ULONG FollowParams; /* ... */  };
struct  MUIP_NList_ColWidth           { ULONG MethodID; LONG col; LONG width; };
struct  MUIP_NList_ContextMenuBuild   { ULONG MethodID; LONG mx; LONG my; LONG pos; LONG column; LONG flags; LONG ontop; };
struct  MUIP_NList_DropEntryDrawErase { ULONG MethodID; LONG type; LONG drawpos; LONG erasepos; };
struct  MUIP_NList_ColToColumn        { ULONG MethodID; LONG col; };
struct  MUIP_NList_ColumnToCol        { ULONG MethodID; LONG column; };
struct  MUIP_NList_Sort2              { ULONG MethodID; LONG sort_type; LONG sort_type_add; };
struct  MUIP_NList_PrevSelected       { ULONG MethodID; LONG *pos; };
struct  MUIP_NList_SetColumnCol       { ULONG MethodID; LONG column; LONG col; };
struct  MUIP_NList_Sort3              { ULONG MethodID; LONG sort_type; LONG sort_type_add; LONG which; };
struct  MUIP_NList_GetPos             { ULONG MethodID; APTR entry; LONG *pos; };
struct  MUIP_NList_SelectChange       { ULONG MethodID; LONG pos; LONG state; ULONG flags; };
struct  MUIP_NList_Construct          { ULONG MethodID; APTR entry; APTR pool; };
struct  MUIP_NList_Destruct           { ULONG MethodID; APTR entry; APTR pool; };
struct  MUIP_NList_Compare            { ULONG MethodID; APTR entry1; APTR entry2; LONG sort_type1; LONG sort_type2; };
struct  MUIP_NList_Display            { ULONG MethodID; APTR entry; LONG entry_pos; STRPTR *strings; STRPTR *preparses; };

#define DISPLAY_ARRAY_MAX 64

#define ALIGN_LEFT      0x0000
#define ALIGN_CENTER    0x0100
#define ALIGN_RIGHT     0x0200
#define ALIGN_JUSTIFY   0x0400


/*
 *  Be carrefull ! the 'sort_type2' member don't exist in releases before 19.96
 *  where MUIM_NList_Sort3, MUIA_NList_SortType2, MUIA_NList_TitleClick2 and
 *  MUIA_NList_TitleMark2 have appeared !
 *  You can safely use get(obj,MUIA_NList_SortType2,&st2) instead if you are not
 *  sure of the NList.mcc release which is used.
 */
struct NList_CompareMessage
{
  APTR entry1;
  APTR entry2;
  LONG sort_type;
  LONG sort_type2;
};

struct NList_ConstructMessage
{
  APTR entry;
  APTR pool;
};

struct NList_DestructMessage
{
  APTR entry;
  APTR pool;
};

struct NList_DisplayMessage
{
  APTR entry;
  LONG entry_pos;
  char *strings[DISPLAY_ARRAY_MAX];
  char *preparses[DISPLAY_ARRAY_MAX];
};

struct NList_CopyEntryToClipMessage
{
  APTR entry;
  LONG entry_pos;
  char *str_result;
  LONG column1;
  LONG column1_pos;
  LONG column2;
  LONG column2_pos;
  LONG column1_pos_type;
  LONG column2_pos_type;
};

struct NList_CopyColumnToClipMessage
{
  char *string;
  LONG entry_pos;
  char *str_result;
  LONG str_pos1;
  LONG str_pos2;
};

#include "default-align.h"

#endif /* MUI_NList_MCC_H */
