#ifndef YAM_HOOK_H
#define YAM_HOOK_H

/***************************************************************************

 YAM - Yet Another Mailer
 Copyright (C) 1995-2000 by Marcel Beck <mbeck@yam.ch>
 Copyright (C) 2000-2001 by YAM Open Source Team

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

#ifdef __MORPHOS__
  #define HOOKPROTO(name, ret, obj, param) static SAVEDS ret name(struct Hook *hook, obj, param))
  #define HOOKPROTONO(name, ret, param) static SAVEDS ret name(struct Hook *hook, param))
  #define HOOKPROTONH(name, ret, obj, param) static SAVEDS ret name(obj, param)
  #define HOOKPROTONHNO(name, ret, param) static SAVEDS ret name(param)
  #define HOOKPROTONH(name, ret, obj) static SAVEDS ret name(obj)
  #define HOOKPROTONHNONP(name, ret) static SAVEDS ret name(void)
  #define DISPATCHERPROTO(name) static ASM(ULONG) SAVEDS name(struct IClass * cl, Object * obj, Msg msg)

  #define MakeHook(hookname, funcname) \
    extern struct EmulLibEntry Gate_##funcname; \
    struct Hook hookname = { {NULL, NULL}, (void*)&Gate_##funcname, NULL, NULL }
  #define ENTRY(func) (void*)&Gate_##func
#else
  #define HOOKPROTO(name, ret, obj, param) static SAVEDS ASM(ret) name(REG(a0, struct Hook *hook), REG(a2, obj), REG(a1, param))
  #define HOOKPROTONO(name, ret, param) static SAVEDS ASM(ret) name(REG(a0, struct Hook *hook), REG(a1, param))
  #define HOOKPROTONH(name, ret, obj, param) static SAVEDS ASM(ret) name(REG(a2, obj), REG(a1, param))
  #define HOOKPROTONHNO(name, ret, param) static SAVEDS ASM(ret) name(REG(a1, param))
  #define HOOKPROTONHNP(name, ret, obj) static SAVEDS ASM(ret) name(REG(a2, obj))
  #define HOOKPROTONHNONP(name, ret) static SAVEDS ret name(void)
  #define DISPATCHERPROTO(name) static ASM(ULONG) SAVEDS name(REG(a0, struct IClass * cl), REG(a2, Object * obj), REG(a1, Msg msg))

  #define MakeHook(hookname, funcname) struct Hook hookname = { {NULL, NULL}, (void *)funcname, NULL, NULL }
  #define ENTRY(func) func
#endif
#define InitHook(hook, orighook, data) ((hook)->h_Entry = (orighook).h_Entry, (hook)->h_Data = (APTR)(data))

#endif /* YAM_HOOK_H */
