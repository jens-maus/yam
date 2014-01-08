/***************************************************************************

 YAM - Yet Another Mailer
 Copyright (C) 1995-2000 Marcel Beck
 Copyright (C) 2000-2014 YAM Open Source Team

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

#include <exec/types.h>

#include "SDI_compiler.h"

#include "YAM_global.h"
#include "YAM_stringsizes.h"

#include "svnrev.h"

// stack cookie for shell v45+
static const char USED_VAR yam_stack_size[] = "$STACK:" STR(SIZE_STACK) "\n";

// some platform/compiler dependent stack and memory definitions.
#if defined(__amigaos4__)
  long __stack_size = SIZE_STACK;        // set the minimum startup stack for clib2
  long __default_pool_size = 128*1024;   // set the pool & puddle size for the
  long __default_puddle_size = 32*1024;  // AllocPool() functions to something more reasonable.
#elif defined(__SASC) || defined(__GNUC__)
  #if defined(__libnix__) || defined(__SASC)
  // GCC (libnix) supports the same as SAS/C!
  long NEAR __stack = SIZE_STACK;
  long NEAR __buffsize = 8192;
  long NEAR _MSTEP = 16384;
  #else
  long __stack_size = SIZE_STACK;    // set the minimum startup stack for clib2
  #endif
#elif defined(__VBCC__) /* starting with VBCC 0.8 release */
  long __stack = SIZE_STACK;
#else
  #error "initial stack/memory specification failed"
#endif

#if defined(__SASC)
  // define the __AMIGA__ symbol if it is yet unknown
  #if !defined(__AMIGA__)
    #define __AMIGA__
  #endif
#endif

// identify the system we are compiling for
#if defined(__amigaos4__)
  #define SYSTEM      "AmigaOS4"
  #define SYSTEMSHORT "OS4"
#elif defined(__MORPHOS__)
  #define SYSTEM      "MorphOS"
  #define SYSTEMSHORT "MOS"
#elif defined(__AROS__)
  #define SYSTEM      "AROS"
  #define SYSTEMSHORT SYSTEM
#elif defined(__AMIGA__)
  #define SYSTEM      "AmigaOS3"
  #define SYSTEMSHORT "OS3"
#else
  #warning "Unsupported System - check SYSTEM define"
  #define SYSTEM      "???"
  #define SYSTEMSHORT "???"
#endif

// identify the CPU model
#if defined(__PPC__) || defined(__powerpc__)
  #define CPU "PPC"
#elif defined(_M68060) || defined(__M68060) || defined(__mc68060)
  #define CPU "m68060"
#elif defined(_M68040) || defined(__M68040) || defined(__mc68040)
  #define CPU "m68040"
#elif defined(_M68030) || defined(__M68030) || defined(__mc68030)
  #define CPU "m68030"
#elif defined(_M68020) || defined(__M68020) || defined(__mc68020)
  #define CPU "m68k"
#elif defined(_M68000) || defined(__M68000) || defined(__mc68000)
  #define CPU "m68000"
#elif defined(__i386__)
  #define CPU "x86"
#elif defined(__x86_64__)
  #define CPU "x86_64"
#elif defined(__arm__)
  #define CPU "ARM"
#else
  #warning "Unsupported CPU model - check CPU define"
  #define CPU "???"
#endif

// for defining the actual version of YAM and mapping it
// to constant variables.
#define __YAM           "YAM"
#define __YAM_VERSION   "2.10"
#ifndef __YAM_DEVEL
#define __YAM_DEVEL     "-dev"
#endif
#ifndef __YAM_BUILDID
#define __YAM_BUILDID   0
#endif
#define __YAM_COPYRIGHT     "Copyright (C) 2000-2014 YAM Open Source Team"
#define __YAM_FULLCOPYRIGHT "Copyright (C) 1995-2000 Marcel Beck\n" __YAM_COPYRIGHT

// find out something about the compiler used
#if defined(__GNUC__)
  #if defined(__GNUC_PATCHLEVEL__)
    #define __YAM_COMPILER "GCC " STR(__GNUC__) "." STR(__GNUC_MINOR__) "." STR(__GNUC_PATCHLEVEL__)
  #else
    #define __YAM_COMPILER "GCC " STR(__GNUC__) "." STR(__GNUC_MINOR__) ".x"
  #endif
#elif defined(__VBCC__)
  #define __YAM_COMPILER "VBCC"
#elif defined(__SASC)
  #define __YAM_COMPILER "SAS/C"
#else
  #define __YAM_COMPILER "unknown"
  #warning "unknown compiler specification"
#endif

// __YAM_BUILDID is 0 for non-nightly builds
#if __YAM_BUILDID == 0
const char * const yambuildid       = "";
#else
const char * const yambuildid       = STR(__YAM_BUILDID);
#endif

const char * const yamver           = __YAM_VERSION __YAM_DEVEL;
const char * const yamversion       = __YAM " " __YAM_VERSION __YAM_DEVEL " [" SYSTEMSHORT "/" CPU ", r" STR(SVN_REV) ", " __YAM_COMPILER "]";
const char * const yamversionver    = __YAM_VERSION __YAM_DEVEL " [" SYSTEMSHORT "/" CPU "]";
const char * const yamversionstring = "$VER: " __YAM " " __YAM_VERSION __YAM_DEVEL " (" __YAM_VERDATE ") " __YAM_COPYRIGHT " [" SYSTEMSHORT "/" CPU ", r" STR(SVN_REV) "]";
const char * const yamuseragent     = __YAM "/" __YAM_VERSION __YAM_DEVEL " (" SYSTEM "; " CPU "; rv:" __YAM_BUILDDATE "r" STR(SVN_REV) ")";
const char * const yamcopyright     = __YAM_COPYRIGHT;
const char * const yamfullcopyright = __YAM_FULLCOPYRIGHT;
const char * const yamversiondate   = __YAM_VERDATE;
const char * const yamcompiler      = __YAM_COMPILER;
const char * const yamurl           = "http://yam.ch/";
const unsigned long yamversiondays  = __YAM_VERDAYS;
const unsigned long yamsvnrev       = SVN_REV;

/* no longer external visible, this is done by proto files! */
struct Library* DataTypesBase     = NULL;
struct Library* IconBase          = NULL;
struct Library* IFFParseBase      = NULL;
struct Library* IntuitionBase     = NULL;
struct Library* KeymapBase        = NULL;
struct Library* LocaleBase        = NULL;
struct Library* MUIMasterBase     = NULL;
struct Library* OpenURLBase       = NULL;
struct Library* RexxSysBase       = NULL;
struct Library* WorkbenchBase     = NULL;
struct Library* xadMasterBase     = NULL;
struct Library* XpkBase           = NULL;
struct Library* AmiSSLMasterBase  = NULL;
struct Library* AmiSSLBase        = NULL;
struct Device*  TimerBase         = NULL;
struct Library* GfxBase           = NULL;
struct Library* LayersBase        = NULL;
struct Library* DiskfontBase      = NULL;
struct Library* CodesetsBase      = NULL;
#if !defined(__amigaos4__)
struct Library* CyberGfxBase      = NULL;
#endif
struct Library* ExpatBase         = NULL;
#if !defined(__NEWLIB__)
struct Library* UtilityBase       = NULL;
#endif
#if defined(__amigaos4__)
struct Library* ApplicationBase   = NULL; // application.library
#endif
#if defined(__amigaos3__)
struct Library* PictureDTBase     = NULL;
#endif

/* AmigaOS4 style interfaces */
#if defined(__amigaos4__)
struct DataTypesIFace*    IDataTypes    = NULL;
struct IconIFace*         IIcon         = NULL;
struct IFFParseIFace*     IIFFParse     = NULL;
struct IntuitionIFace*    IIntuition    = NULL;
struct KeyMapIFace*       IKeymap       = NULL;
struct LocaleIFace*       ILocale       = NULL;
struct MUIMasterIFace*    IMUIMaster    = NULL;
struct OpenURLIFace*      IOpenURL      = NULL;
struct RexxSysIFace*      IRexxSys      = NULL;
struct WorkbenchIFace*    IWorkbench    = NULL;
struct xadMasterIFace*    IxadMaster    = NULL;
struct XpkIFace*          IXpk          = NULL;
struct AmiSSLMasterIFace* IAmiSSLMaster = NULL;
struct AmiSSLIFace*       IAmiSSL       = NULL;
struct TimerIFace*        ITimer        = NULL;
struct GraphicsIFace*     IGraphics     = NULL;
struct LayersIFace*       ILayers       = NULL;
struct DiskfontIFace*     IDiskfont     = NULL;
struct CodesetsIFace*     ICodesets     = NULL;
#if !defined(__amigaos4__)
struct CyberGfxIFace*     ICyberGfx     = NULL;
#endif
struct ExpatIFace*        IExpat        = NULL;
struct ApplicationIFace*  IApplication  = NULL;
struct TimezoneIFace*     ITimezone     = NULL;
#if !defined(__NEWLIB__)
struct UtilityIFace*      IUtility      = NULL;
#endif
#endif /* __amigaos4__ */

struct WBStartup *WBmsg = NULL;

const char* const wdays[7] = { "Sun","Mon","Tue","Wed","Thu","Fri","Sat" };
const char* const months[12] = { "Jan","Feb","Mar","Apr","May","Jun","Jul","Aug","Sep","Oct","Nov","Dec" };
const char* const SecCodes[5] = { "none","sign","encrypt","sign+encrypt","standard" };
