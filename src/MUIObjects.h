#ifndef MUIOBJECTS_H
#define MUIOBJECTS_H

/***************************************************************************

 YAM - Yet Another Mailer
 Copyright (C) 1995-2000 Marcel Beck
 Copyright (C) 2000-2012 YAM Open Source Team

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

 YAM Official Support Site :  http://www.yam.ch
 YAM OpenSource project    :  http://sourceforge.net/projects/yamos/

 $Id$

***************************************************************************/

#include "SDI_compiler.h"

// some flags for MakeAddressField()
#define AFF_ALLOW_MULTI         (1<<0)
#define AFF_EXTERNAL_SHORTCUTS  (1<<1)

Object * MakeButton(const char *txt);
Object * MakeCheck(const char *label);
Object * MakeCheckGroup(Object **check, const char *label);
Object * MakeCycle(const char *const *labels, const char *label);
Object * MakeInteger(int maxlen, const char *label);
Object * MakeNumeric(int min, int max, BOOL percent);
Object * MakePassString(const char *label);
Object * MakePGPKeyList(Object **st, BOOL secret, const char *label);
Object * MakeString(int maxlen, const char *label);
Object * MakeAddressField(Object **string, const char *label, const Object *help, int abmode, int winnr, ULONG flags);
Object * MakeCharsetPop(Object **string, Object **pop);
char ShortCut(const char *label);
BOOL GetMUICheck(Object *obj);
int GetMUICycle(Object *obj);
int GetMUIInteger(Object *obj);
int GetMUINumer(Object *obj);
int GetMUIRadio(Object *obj);

#define GetMUIString(a, o, l) strlcpy((a), (char *)xget((o), MUIA_String_Contents), (l))
#define GetMUIText(a, o, l)   strlcpy((a), (char *)xget((o), MUIA_Text_Contents), (l))
#define SetHelp(o,str)        set(o, MUIA_ShortHelp, tr(str))

BOOL isChildOfGroup(Object *group, Object *child);
BOOL isChildOfFamily(Object *family, Object *child);

#ifndef MUIA_Scrollgroup_AutoBars
#define MUIA_Scrollgroup_AutoBars           0x8042f50e /* V20 isg BOOL              */
#endif

/* ReturnID collecting macros
** every COLLECT_ have to be finished with a REISSUE_
**
** Example:
**
** COLLECT_RETURNIDS;
**
** while(running)
** {
**    static ULONG signals=0;
**    switch(DoMethod(G->App, MUIM_Application_NewInput, &signals))
**    {
**        case ID_PLAY:
**           PlaySound();
**           break;
**
**        case ID_CANCEL:
**        case MUIV_Application_ReturnID_Quit:
**           running = FALSE;
**           break;
**    }
**
**    if(running && signals)
**      signals = Wait(signals);
** }
**
** REISSUE_RETURNIDS;
*/
#define COLLECT_SIZE 32
#define COLLECT_RETURNIDS { \
                            ULONG returnID[COLLECT_SIZE], csize = COLLECT_SIZE, rpos = COLLECT_SIZE, userData, userSigs = 0; \
                            while(csize && userSigs == 0 && (userData = DoMethod(G->App, MUIM_Application_NewInput, &userSigs))) \
                              returnID[--csize] = userData

#define REISSUE_RETURNIDS   while(rpos > csize) \
                              DoMethod(G->App, MUIM_Application_ReturnID, returnID[--rpos]); \
                          }

// Here we define inline functions that should be inlined by
// the compiler, if possible.

/// xget()
//  Gets an attribute value from a MUI object
IPTR xget(Object *obj, const IPTR attr);
#if defined(__GNUC__)
  // please note that we do not evaluate the return value of GetAttr()
  // as some attributes (e.g. MUIA_Selected) always return FALSE, even
  // when they are supported by the object. But setting b=0 right before
  // the GetAttr() should catch the case when attr doesn't exist at all
  #define xget(OBJ, ATTR) ({IPTR b=0; GetAttr(ATTR, OBJ, &b); b;})
#endif

///
/// xset()
//  Sets attributes for a MUI object
ULONG xset(Object *obj, ...);
#if defined(__GNUC__) || defined(__VBCC__)
  #define xset(obj, ...)  SetAttrs((obj), __VA_ARGS__, TAG_DONE)
#endif

///

#endif /* MUIOBJECTS_H */
