#ifndef YAM_HOOK_H
#define YAM_HOOK_H

/***************************************************************************

 YAM - Yet Another Mailer
 Copyright (C) 1995-2000 by Marcel Beck <mbeck@yam.ch>
 Copyright (C) 2000-2002 by YAM Open Source Team

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

/*
** Special general hook macros to handle the creation of Hooks/Dispatchers
** for different Operating System versions.
** Currently AmigaOSv3 and MorphOS is supported.
**
** Example:
**
** Creates a hook with the name "GeneralDesHook" that calls a coresponding
** function "GeneralDesFunc" that will be called with a pointer "entry"
** (REG_A1) and returns a long.
**
** HOOKPROTONHNO(GeneralDesFunc, long, void *entry)
** {
**   free(entry);
**   return 0;
** }
** MakeHook(GeneralDesHook, GeneralDesFunc);
**
** Every function that is created with HOOKPROTO* >HAVE TO< be finished
** with either MakeHook() or MakeStaticHook() so that the macros will
** be closed correctly.
**
** The naming convention for the Hook Prototype macros is as followed:
**
** HOOKPROTO[NH][NO][NP]
**           ^^  ^^  ^^
**      NoHook   |    NoParameter
**            NoObject
**
** So a plain HOOKPROTO() creates you a Hook function that requires
** 4 parameters, the "name" of the hookfunction, the "obj" in REG_A2,
** the "param" in REG_A1 and a "hook" in REG_A0.
*/

#ifdef __MORPHOS__
  #include <emul/emulinterface.h>
  #include <emul/emulregs.h>
  #define HOOKPROTO(name, ret, obj, param) static ret name(void) { struct Hook *hook = REG_A0; obj = REG_A2; param = REG_A1;
  #define HOOKPROTONO(name, ret, param) static ret name(void) { struct Hook *hook = REG_A0; param = REG_A1;
  #define HOOKPROTONH(name, ret, obj, param) static ret name(void) { obj = REG_A2; param = REG_A1;
  #define HOOKPROTONHNO(name, ret, param) static ret name(void) { param = REG_A1;
  #define HOOKPROTONHNP(name, ret, obj) static ret name(void) { obj = REG_A2;
  #define HOOKPROTONHNONP(name, ret) static ret name(void) {
  #define DISPATCHERPROTO(name) \
    struct IClass; \
    static ULONG name(struct IClass * cl, Object * obj, Msg msg); \
    static ULONG Trampoline_##name(void) { return name((struct IClass *) REG_A0, (Object *) REG_A2, (Msg) REG_A1); } \
    static const struct EmulLibEntry Gate_##name = { TRAP_LIB, 0, (void(*)())Trampoline_##name }; \
    static ULONG name(struct IClass * cl, Object * obj, Msg msg)

  #define MakeHook(hookname, funcname) \
    } static const struct EmulLibEntry Gate_##funcname = { TRAP_LIB, 0, (void(*)()) funcname }; \
    struct Hook hookname = { {NULL, NULL}, (HOOKFUNC)&Gate_##funcname, NULL, NULL }
  #define MakeStaticHook(hookname, funcname) \
    } static const struct EmulLibEntry Gate_##funcname = { TRAP_LIB, 0, (void(*)()) funcname }; \
    static struct Hook hookname = { {NULL, NULL}, (HOOKFUNC)&Gate_##funcname, NULL, NULL }
  #define ENTRY(func) (APTR)&Gate_##func
#else
  #define HOOKPROTO(name, ret, obj, param) static SAVEDS ASM(ret) name(REG(a0, struct Hook *hook), REG(a2, obj), REG(a1, param))
  #define HOOKPROTONO(name, ret, param) static SAVEDS ASM(ret) name(REG(a0, struct Hook *hook), REG(a1, param))
  #define HOOKPROTONH(name, ret, obj, param) static SAVEDS ASM(ret) name(REG(a2, obj), REG(a1, param))
  #define HOOKPROTONHNO(name, ret, param) static SAVEDS ASM(ret) name(REG(a1, param))
  #define HOOKPROTONHNP(name, ret, obj) static SAVEDS ASM(ret) name(REG(a2, obj))
  #define HOOKPROTONHNONP(name, ret) static SAVEDS ret name(void)
  #define DISPATCHERPROTO(name) static SAVEDS ASM(ULONG) name(REG(a0, struct IClass * cl), REG(a2, Object * obj), REG(a1, Msg msg))

  #define MakeHook(hookname, funcname) struct Hook hookname = { {NULL, NULL}, (HOOKFUNC)funcname, NULL, NULL }
  #define MakeStaticHook(hookname, funcname) static struct Hook hookname = { {NULL, NULL}, (HOOKFUNC)funcname, NULL, NULL }
  #define ENTRY(func) (APTR)func
#endif

#define InitHook(hook, orighook, data) ((hook)->h_Entry = (orighook).h_Entry, (hook)->h_Data = (APTR)(data))

#endif /* YAM_HOOK_H */
