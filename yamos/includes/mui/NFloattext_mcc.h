/*
  NFloattext.mcc (c) Copyright 1996 by Gilles Masson
  Registered MUI class, Serial Number: 1d51                            0x9d5100a1 to 0x9d5100aF
  *** use only YOUR OWN Serial Number for your public custom class ***
  NFloattext_mcc.h
*/

#ifndef MUI_NFloattext_MCC_H
#define MUI_NFloattext_MCC_H

#ifndef LIBRARIES_MUI_H
#include <libraries/mui.h>
#endif

#ifndef MUI_NListview_MCC_H
#include <mui/NListview_mcc.h>
#endif

#include "amiga-align.h"

#define MUIC_NFloattext "NFloattext.mcc"
#define NFloattextObject MUI_NewObject(MUIC_NFloattext


/* Attributes */

#define MUIA_NFloattext_Text                0x9d5100a1 /* GM  isg STRPTR             */
#define MUIA_NFloattext_SkipChars           0x9d5100a2 /* GM  isg char *             */
#define MUIA_NFloattext_TabSize             0x9d5100a3 /* GM  isg ULONG              */
#define MUIA_NFloattext_Justify             0x9d5100a4 /* GM  isg BOOL               */
#define MUIA_NFloattext_Align               0x9d5100a5 /* GM  isg LONG               */

#define MUIM_NFloattext_GetEntry            0x9d5100aF /* GM */
struct  MUIP_NFloattext_GetEntry            { ULONG MethodID; LONG pos; APTR *entry; };

#include "default-align.h"

#endif /* MUI_NFloattext_MCC_H */
