/***************************************************************************

 YAM - Yet Another Mailer
 Copyright (C) 1995-2000 Marcel Beck
 Copyright (C) 2000-2013 YAM Open Source Team

 This program is free software; you can redistribute it and/or modify
 it under the terms of the GNU General Public License as published by
 the Free Software Foundation; either version 2 of the License, or
 (at your option) any later version.

 This program is distributed in the hope that it will be useful,
 but WITHOUT ANY WARRANTY; without even the implied warranty of
 MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 GNU General Public License for more details.

 You should have received a copy of the GNU General Public License
 along with this program; if not, write to the Free Software
 Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA

 YAM Official Support Site :  http://www.yam.ch/
 YAM OpenSource project    :  http://sourceforge.net/projects/yamos/

 $Id$

***************************************************************************/

#ifndef CLASSES_CLASSES_EXTRA_H
#define CLASSES_CLASSES_EXTRA_H

#if defined(INCLUDE_KITCHEN_SINK)

#include <ctype.h>
#include <stdlib.h>
#include <string.h>

#include <clib/alib_protos.h>
#include <clib/macros.h>
#include <libraries/iffparse.h>
#include <libraries/gadtools.h>
#include <mui/BetterString_mcc.h>
#include <mui/NList_mcc.h>
#include <mui/NListview_mcc.h>
#include <mui/NListtree_mcc.h>
#include <mui/TextEditor_mcc.h>
#include <mui/TheBar_mcc.h>
#include <proto/dos.h>
#include <proto/exec.h>
#include <proto/muimaster.h>
#include <proto/timer.h>
#include <proto/utility.h>

#include "extrasrc.h"

#include "newmouse.h"
#include "SDI_hook.h"
#include "SDI_stdarg.h"

#include "YAM.h"
#include "YAM_global.h"
#include "YAM_utilities.h"

#include "Config.h"
#include "Locale.h"
#include "ParseEmail.h"

#endif /* INCLUDE_KITCHEN_SINK */

#include <libraries/mui.h>

#include "YAM_main.h" // NewMode
#include "YAM_read.h" // HeaderMode,SInfoMode
#include "BayesFilter.h" // BayesClassification

// some own MUI macros (not official)
#ifndef MUIF_NONE
#define MUIF_NONE                    0
#endif

#define MenuChild					MUIA_Family_Child
#define Menuitem(t,s,e,c,u)			MenuitemObject,\
                                      MUIA_Menuitem_Title,			(t),\
                                      MUIA_Menuitem_CopyStrings,	FALSE,\
                                      MUIA_Menuitem_Shortcut,		(s),\
                                      MUIA_Menuitem_Enabled,		(e),\
                                      MUIA_Menuitem_CommandString,	(c),\
                                      MUIA_UserData,				(u),\
                                    End

#define MenuitemCheck(t,s,e,c,g,x,u)	MenuitemObject,\
                                        MUIA_Menuitem_Checkit,		 TRUE,\
                                        MUIA_Menuitem_Title,		(t),\
                                        MUIA_Menuitem_CopyStrings,	FALSE,\
                                        MUIA_Menuitem_Shortcut,		(s),\
                                        MUIA_Menuitem_Checked,		(c),\
                                        MUIA_Menuitem_Toggle,		(g),\
                                        MUIA_Menuitem_Exclude,		(x),\
                                        MUIA_Menuitem_Enabled,		(e),\
                                        MUIA_UserData,				(u),\
                                      End

#define MenuBarLabel				MenuitemObject,\
                                      MUIA_Menuitem_Title,			NM_BARLABEL,\
                                      MUIA_Menuitem_CopyStrings,	FALSE,\
                                    End


// some private (mostly undocumented) MUI stuff...
#ifndef MUIM_GoActive
#define MUIM_GoActive                       0x8042491aUL /* V8  */
#endif
#ifndef MUIM_GoInactive
#define MUIM_GoInactive                     0x80422c0cUL /* V8  */
#endif
#ifndef MUIM_Layout
#define MUIM_Layout                         0x8042845b /* V4  */
struct  MUIP_Layout                         { ULONG MethodID; LONG left; LONG top; LONG width; LONG height; ULONG flags; };
#endif
#ifndef MUIA_Window_DisableKeys
#define MUIA_Window_DisableKeys             0x80424c36UL /* V15 isg ULONG    */
#endif
#ifndef MUIA_Application_UsedClasses
#define MUIA_Application_UsedClasses        0x8042e9a7UL /* V20 isg STRPTR * */
#endif
#ifndef MUIA_String_Popup
#define MUIA_String_Popup                   0x80420d71UL /* V9  i.. Object * */
#endif
#ifndef MUIA_List_CursorType
#define MUIA_List_CursorType                0x8042c53eUL /* V4  is. LONG     */
#endif
#ifndef MUIV_List_CursorType_Bar
#define MUIV_List_CursorType_Bar 		    1
#endif
#ifndef MUIA_Text_HiIndex
#define MUIA_Text_HiIndex                   0x804214f5UL /* V11 i.. LONG     */
#endif
#ifndef MUIM_DeleteDragImage
#define MUIM_DeleteDragImage 				0x80423037UL
#endif
#ifndef MUIM_Group_MoveMember
#define MUIM_Group_MoveMember				0x8042ff4eUL /* V16 */
#endif
#ifndef MUIM_Group_ExitChange2
#define MUIM_Group_ExitChange2              0x8042e541 /* private */ /* V12 */
#endif
#if (MUIMASTER_VMIN < 18)
#ifndef MUIM_DoDrag
#define MUIM_DoDrag                         0x804216bbUL /* private */ /* V18 */
struct  MUIP_DoDrag                         { STACKED ULONG MethodID; STACKED LONG touchx; STACKED LONG touchy; STACKED ULONG flags; }; /* private */
#endif
#endif
#ifndef MUIA_Text_Copy
#define MUIA_Text_Copy                      0x80427727UL /* V20 i.. BOOL              */
#endif
#ifndef MUIO_Label_Tiny
#define MUIO_Label_Tiny                     (1<<13)
#endif
#ifndef MUIA_Scrollgroup_AutoBars
#define MUIA_Scrollgroup_AutoBars           0x8042f50eUL /* V20 isg BOOL              */
#endif
#ifndef MUIA_DoubleBuffer
#define MUIA_DoubleBuffer                   0x8042a9c7UL /* V20 isg BOOL              */
#endif
#ifndef MUIV_Window_ActiveObject_Left
#define MUIV_Window_ActiveObject_Left       MUIV_Window_ActiveObject_Prev
#endif
#ifndef MUIV_Window_ActiveObject_Right
#define MUIV_Window_ActiveObject_Right      MUIV_Window_ActiveObject_Next
#endif
#ifndef MUIA_Menu_CopyStrings
#define MUIA_Menu_CopyStrings               0x8042dbe2UL /* V20 i.. BOOL              */
#endif
#ifndef MUIA_Menuitem_CopyStrings
#define MUIA_Menuitem_CopyStrings           0x8042dc1bUL /* V20 i.. BOOL              */
#endif

enum { IECODE_SPACE = 64,
       IECODE_TAB = 66,
       IECODE_RETURN = 68,
       IECODE_ESCAPE = 69,
       IECODE_HELP = 95,
       IECODE_BACKSPACE = 65,
       IECODE_DEL = 70,
       IECODE_UP = 76,
       IECODE_DOWN = 77,
       IECODE_RIGHT = 78,
       IECODE_LEFT = 79
     };

// some own usefull MUI-style macros to check mouse positions in objects
#define _between(a,x,b) 					((x)>=(a) && (x)<=(b))
#define _isinobject(o,x,y) 				(_between(_mleft(o),(x),_mright (o)) && _between(_mtop(o) ,(y),_mbottom(o)))
#define _isinwholeobject(o,x,y) 	(_between(_left(o),(x),_right (o)) && _between(_top(o) ,(y),_bottom(o)))


// this method is invoked for an object as soon
// as a thread has finished its task
#define MUIM_ThreadFinished                        (TAG_USER | (0x2677 << 16))

// the method gets the performed action with all
// parameters and the final result value
struct MUIP_ThreadFinished { ULONG MethodID; ULONG action; LONG result; APTR actionTags; };

#endif
