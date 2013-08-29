#ifndef MUIOBJECTS_H
#define MUIOBJECTS_H

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

 YAM Official Support Site :  http://www.yam.ch
 YAM OpenSource project    :  http://sourceforge.net/projects/yamos/

 $Id$

***************************************************************************/

#include "SDI_compiler.h"

struct FileRequester;

// some flags for MakeAddressField()
#define AFF_ALLOW_MULTI         (1<<0)
#define AFF_EXTERNAL_SHORTCUTS  (1<<1)
#define AFF_NOFULLNAME          (1<<2)
#define AFF_NOCACHE             (1<<3)
#define AFF_NOVALID             (1<<4)
#define AFF_RESOLVEINACTIVE     (1<<5)

Object *MakeButton(const char *txt);
Object *MakeCheck(const char *label);
Object *MakeCheckGroup(Object **check, const char *label);
Object *MakeCycle(const char *const *labels, const char *label);
Object *MakeInteger(int maxlen, const char *label);
Object *MakeNumeric(int min, int max, BOOL percent);
Object *MakePassString(const char *label);
Object *MakePGPKeyList(Object **st, BOOL secret, const char *label);
Object *MakeString(int maxlen, const char *label);
Object *MakeAddressField(Object **string, const char *label, const void *help, int abmode, int winnr, ULONG flags);
Object *MakeVarPop(Object **string, Object **popButton, Object **list, const int mode, const int size, const char *shortcut);
Object *MakeMimeTypePop(Object **string, const char *desc);
char ShortCut(const char *label);

extern struct Hook FilereqStartHook;
extern struct Hook FilereqStopHook;
extern struct Hook PO_MimeTypeListOpenHook;
extern struct Hook PO_MimeTypeListCloseHook;
extern struct Hook PO_WindowHook;

struct MimeTypeCloseObjects
{
  Object *extension;
  Object *description;
};

#define GetMUICheck(o)   (BOOL)xget((o), MUIA_Selected)
#define GetMUICycle(o)   (int)xget((o), MUIA_Cycle_Active)
#define GetMUIInteger(o) (int)xget((o), MUIA_String_Integer)
#define GetMUINumer(o)   (int)xget((o), MUIA_Numeric_Value)
#define GetMUIRadio(o)   (int)xget((o), MUIA_Radio_Active)

void GetMUIString(char *s, Object *o, size_t len);
void GetMUIText(char *s, Object *o, size_t len);

#define SetHelp(o,str)        set(o, MUIA_ShortHelp, tr(str))

// macros for more easy creation of objects
#define HBarT(str)            RectangleObject, \
                                MUIA_FixHeightTxt, (str), \
                                MUIA_Rectangle_BarTitle, (str), \
                                MUIA_Rectangle_HBar, TRUE

#define VBarT(str)            RectangleObject, \
                                MUIA_FixWidthTxt, (str), \
                                MUIA_Rectangle_BarTitle, (str), \
                                MUIA_Rectangle_VBar, TRUE

BOOL isChildOfGroup(Object *group, Object *child);
BOOL isChildOfFamily(Object *family, Object *child);
const char *CreateScreenTitle(char *dst, size_t dstlen, const char *text);

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
