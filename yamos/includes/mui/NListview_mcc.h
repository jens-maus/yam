/*
  NListview.mcc (c) Copyright 1996 by Gilles Masson
  Registered MUI class, Serial Number: 1d51                            0x9d510020 to 0x9d51002F
  *** use only YOUR OWN Serial Number for your public custom class ***
  NListview_mcc.h
*/

#ifndef MUI_NListview_MCC_H
#define MUI_NListview_MCC_H

#ifndef LIBRARIES_MUI_H
#include <libraries/mui.h>
#endif

#ifndef MUI_NList_MCC_H
#include <mui/NList_mcc.h>
#endif

#include "amiga-align.h"

#define MUIC_NListview "NListview.mcc"
#define NListviewObject MUI_NewObject(MUIC_NListview


/* Attributes */

#define MUIA_NListview_NList                0x9d510020 /* GM  i.g Object *          */

#define MUIA_NListview_Vert_ScrollBar       0x9d510021 /* GM  isg LONG              */
#define MUIA_NListview_Horiz_ScrollBar      0x9d510022 /* GM  isg LONG              */
#define MUIA_NListview_VSB_Width            0x9d510023 /* GM  ..g LONG              */
#define MUIA_NListview_HSB_Height           0x9d510024 /* GM  ..g LONG              */

#define MUIV_Listview_ScrollerPos_Default 0
#define MUIV_Listview_ScrollerPos_Left 1
#define MUIV_Listview_ScrollerPos_Right 2
#define MUIV_Listview_ScrollerPos_None 3

#define MUIM_NListview_QueryBeginning       MUIM_NList_QueryBeginning /* obsolete */

#define MUIV_NListview_VSB_Always      1
#define MUIV_NListview_VSB_Auto        2
#define MUIV_NListview_VSB_FullAuto    3
#define MUIV_NListview_VSB_None        4
#define MUIV_NListview_VSB_Default     5
#define MUIV_NListview_VSB_Left        6

#define MUIV_NListview_HSB_Always      1
#define MUIV_NListview_HSB_Auto        2
#define MUIV_NListview_HSB_FullAuto    3
#define MUIV_NListview_HSB_None        4
#define MUIV_NListview_HSB_Default     5

#define MUIV_NListview_VSB_On          0x0030
#define MUIV_NListview_VSB_Off         0x0010

#define MUIV_NListview_HSB_On          0x0300
#define MUIV_NListview_HSB_Off         0x0100

#include "default-align.h"

#endif /* MUI_NListview_MCC_H */
