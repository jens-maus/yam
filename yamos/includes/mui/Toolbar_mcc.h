/***************************************************************************

 Toolbar MCC - MUI Custom Class for Toolbar handling
 Copyright (C) 1997-2000 Benny Kjær Nielsen <floyd@amiga.dk>
 Copyright (C) 2004-2005 by Toolbar.mcc Open Source Team

 This library is free software; you can redistribute it and/or
 modify it under the terms of the GNU Lesser General Public
 License as published by the Free Software Foundation; either
 version 2.1 of the License, or (at your option) any later version.

 This library is distributed in the hope that it will be useful,
 but WITHOUT ANY WARRANTY; without even the implied warranty of
 MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 Lesser General Public License for more details.

 Toolbar class Support Site:  http://www.sf.net/projects/toolbar-mcc

 $Id$

***************************************************************************/

#ifndef TOOLBAR_MCC_H
#define TOOLBAR_MCC_H

/*** Include stuff ***/

#ifndef LIBRARIES_MUI_H
#include <libraries/mui.h>
#endif

#ifndef EXEC_TYPES_H
#include <exec/types.h>
#endif

#ifndef BKN_SERIAL
#define BKN_SERIAL 0xfcf70000
#endif

#if defined(__PPC__)
  #if defined(__GNUC__)
    #pragma pack(2)
  #elif defined(__VBCC__)
    #pragma amiga-align
  #endif
#endif

/*** MUI Defines ***/

#define MUIC_Toolbar "Toolbar.mcc"
#define ToolbarObject MUI_NewObject(MUIC_Toolbar

/*** Methods ***/

#define MUIM_Toolbar_BottomEdge      (BKN_SERIAL | 0x07)
#define MUIM_Toolbar_CheckNotify     (BKN_SERIAL | 0x0d)
#define MUIM_Toolbar_DrawButton      (BKN_SERIAL | 0x0c)
#define MUIM_Toolbar_KillNotify      (BKN_SERIAL | 0x02)
#define MUIM_Toolbar_KillNotifyObj   (BKN_SERIAL | 0x03)
#define MUIM_Toolbar_LeftEdge        (BKN_SERIAL | 0x08)
#define MUIM_Toolbar_MultiSet        (BKN_SERIAL | 0x04)
#define MUIM_Toolbar_Notify          (BKN_SERIAL | 0x01)
#define MUIM_Toolbar_Redraw          (BKN_SERIAL | 0x05)
#define MUIM_Toolbar_ReloadImages    (BKN_SERIAL | 0x0b)
#define MUIM_Toolbar_RightEdge       (BKN_SERIAL | 0x09)
#define MUIM_Toolbar_Set             (BKN_SERIAL | 0x06)
#define MUIM_Toolbar_TopEdge         (BKN_SERIAL | 0x0a)

/*** Method structs ***/

struct MUIP_Toolbar_CheckNotify      {ULONG MethodID; ULONG TrigButton; ULONG TrigAttr; ULONG TrigValue; };
struct MUIP_Toolbar_Edge             {ULONG MethodID; ULONG Button; };
struct MUIP_Toolbar_KillNotify       {ULONG MethodID; ULONG TrigButton; ULONG TrigAttr; };
struct MUIP_Toolbar_KillNotifyObj    {ULONG MethodID; };
struct MUIP_Toolbar_MultiSet         {ULONG MethodID; ULONG Flag; ULONG Value; LONG Button; /* ... */ };
struct MUIP_Toolbar_Notify           {ULONG MethodID; ULONG TrigButton; ULONG TrigAttr; ULONG TrigValue; Object *DestObj; ULONG FollowParams; /* ... */};
struct MUIP_Toolbar_Redraw           {ULONG MethodID; ULONG Changes; };
struct MUIP_Toolbar_ReloadImages     {ULONG MethodID; STRPTR Normal; STRPTR Select; STRPTR Ghost; };
struct MUIP_Toolbar_Set              {ULONG MethodID; ULONG Button; ULONG Flag; ULONG Value; };

/*** Special method values ***/

#define MUIV_Toolbar_Set_Ghosted     0x04
#define MUIV_Toolbar_Set_Gone        0x08
#define MUIV_Toolbar_Set_Selected    0x10

#define MUIV_Toolbar_Notify_Pressed    0
#define MUIV_Toolbar_Notify_Active     1
#define MUIV_Toolbar_Notify_Ghosted    2
#define MUIV_Toolbar_Notify_Gone       3
#define MUIV_Toolbar_Notify_LeftEdge   4
#define MUIV_Toolbar_Notify_RightEdge  5
#define MUIV_Toolbar_Notify_TopEdge    6
#define MUIV_Toolbar_Notify_BottomEdge 7

/*** Special value for MUIM_Toolbar_Notify ***/

#define MUIV_Toolbar_Qualifier 0x49893135

/*** Special method flags ***/

/*** Attributes ***/

#define MUIA_Toolbar_Description     (BKN_SERIAL | 0x16 )
#define MUIA_Toolbar_HelpString      (BKN_SERIAL | 0x17 )
#define MUIA_Toolbar_Horizontal      (BKN_SERIAL | 0x15 )
#define MUIA_Toolbar_ImageGhost      (BKN_SERIAL | 0x13 )
#define MUIA_Toolbar_ImageNormal     (BKN_SERIAL | 0x11 )
#define MUIA_Toolbar_ImageSelect     (BKN_SERIAL | 0x12 )
#define MUIA_Toolbar_ImageType       (BKN_SERIAL | 0x10 )
#define MUIA_Toolbar_ParseUnderscore (BKN_SERIAL | 0x18 )
#define MUIA_Toolbar_Path            (BKN_SERIAL | 0x1b )
#define MUIA_Toolbar_Permutation     (BKN_SERIAL | 0x1a )
#define MUIA_Toolbar_Qualifier       (BKN_SERIAL | 0x1c )
#define MUIA_Toolbar_Reusable        (BKN_SERIAL | 0x19 )

/*** Special attribute values ***/

#define MUIV_Toolbar_ImageType_File     0
#define MUIV_Toolbar_ImageType_Memory   1
#define MUIV_Toolbar_ImageType_Object   2

/*** Structures, Flags & Values ***/

#define TP_SPACE -2
#define TP_END   -1

struct MUIP_Toolbar_Description
{
  UBYTE   Type;          /* Type of button - see possible values below (TDT_). */
  UBYTE   Key;           /* Hotkey */
  UWORD   Flags;         /* The buttons current setting - see the TDF_ flags */
  STRPTR  ToolText;      /* The text beneath the icons. */
  STRPTR  HelpString;    /* The string used for help-bubbles or MUIA_Toolbar_HelpString */
  ULONG   MutualExclude; /* Buttons to be released when this button is pressed down */
};

/*** Toolbar Description Types ***/

#define TDT_BUTTON  0
#define TDT_SPACE   1
#define TDT_IGNORE  2 // Obsolete
#define TDT_END     3

#define TDT_IGNORE_FLAG 128

/*** Toolbar Description Flags ***/

#define TDF_TOGGLE      0x01 /* Set this if it's a toggle-button */
#define TDF_RADIO       0x02 /* AND this if it's also a radio-button */
#define TDF_GHOSTED     0x04
#define TDF_GONE        0x08 /* Make the button temporarily go away */
#define TDF_SELECTED    0x10 /* State of a toggle-button */

#define TDF_RADIOTOGGLE (TDF_TOGGLE|TDF_RADIO) /* A practical definition */

/* TDF_RADIO and TDF_SELECTED only makes sense
   if you have set the TDF_TOGGLE flag.          */

/*** Toolbar Macros ***/

#define Toolbar_Button(flags, text)          { TDT_BUTTON, NULL, flags, text, NULL, NULL}
#define Toolbar_KeyButton(flags, text, key)  { TDT_BUTTON, key,  flags, text, NULL, NULL}
#define Toolbar_Space                        { TDT_SPACE,  NULL, NULL,  NULL, NULL, NULL}
#define Toolbar_End                          { TDT_END,    NULL, NULL,  NULL, NULL, NULL}

#if defined(__PPC__)
  #if defined(__GNUC__)
    #pragma pack()
  #elif defined(__VBCC__)
    #pragma default-align
  #endif
#endif

#endif /* TOOLBAR_MCC_H */
