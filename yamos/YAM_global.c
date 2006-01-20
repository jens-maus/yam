/***************************************************************************

 YAM - Yet Another Mailer
 Copyright (C) 1995-2000 by Marcel Beck <mbeck@yam.ch>
 Copyright (C) 2000-2006 by YAM Open Source Team

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

#include "YAM_global.h"
#include "YAM_locale.h"

#include "SDI_compiler.h"

// some platform/compiler dependent stack definitions.
static const char USED_VAR yam_stack_size[] = "$STACK:65536\n";  // Shell v45 and later
#if defined(__amigaos4__)
  long __stack_size = 65536;             // set the minimum startup stack for clib2
  long __default_pool_size = 128*1024;   // set the pool & puddle size for the
  long __default_puddle_size = 32*1024;  // AllocPool() functions to something more reasonable.
#elif defined(__SASC) || defined(__GNUC__)
  #if defined(__libnix__) || defined(__SASC)
  /* GCC (libnix) supports the same as SAS/C! */
  long __stack = 65536;
  long __buffsize = 8192;
  long _MSTEP = 16384;
  #else
  long __stack_size = 65536;    // set the minimum startup stack for clib2
  #endif
#elif defined(__VBCC__) /* starting with VBCC 0.8 release */
  long __stack = 65536;
#else
  #error "initial stack specification failed"
#endif

/* the version stuff */
#if defined(__PPC__)
  #if defined(__amigaos4__)
    #define CPU " [OS4/PPC]"
  #elif defined(__MORPHOS__)
    #define CPU " [MOS/PPC]"
  #else
    #define CPU " [PPC]"
  #endif
#elif defined(_M68060) || defined(__M68060) || defined(__mc68060)
  #define CPU " [060]"
#elif defined(_M68040) || defined(__M68040) || defined(__mc68040)
  #define CPU " [040]"
#elif defined(_M68030) || defined(__M68030) || defined(__mc68030)
  #define CPU " [030]"
#elif defined(_M68020) || defined(__M68020) || defined(__mc68020)
  #define CPU " [020]"
#elif defined(_M68000) || defined(__M68000) || defined(__mc68000)
  #define CPU " [68k]"
#else
  #error "Unsupported CPU model - check compiler defines"
#endif

#define __YAM_VERSION   "2.5"
#define __YAM_DEVEL     "-dev"
#ifndef __YAM_BUILDID
#define __YAM_BUILDID   ""
#endif
#define __YAM_COPYRIGHT "Copyright (C) 2000-2006 YAM Open Source Team"

const char * const yamversion       = "YAM " __YAM_VERSION __YAM_DEVEL CPU;
const char * const yamversionver    = __YAM_VERSION __YAM_DEVEL CPU;
const char * const yamversionstring = "$VER: YAM " __YAM_VERSION __YAM_DEVEL CPU " (" __YAM_VERDATE ") " __YAM_COPYRIGHT;
const char * const yamverxmailer    = "YAM " __YAM_VERSION __YAM_DEVEL __YAM_BUILDID CPU \
                                      " AmigaOS E-mail Client (C) 2000-2006 YAM Open Source Team - http://www.yam.ch/";
const char * const yamcopyright     = __YAM_COPYRIGHT;
const char * const yamversiondate   = __YAM_VERDATE;
const unsigned long yamversiondays  = __YAM_VERDAYS;

#if defined(__GNUC__)
  #if defined(__GNUC_PATCHLEVEL__)
    const char * const yamcompiler = " (GCC " STR(__GNUC__) "." STR(__GNUC_MINOR__) "." STR(__GNUC_PATCHLEVEL__) ")";
  #else
    const char * const yamcompiler = " (GCC " STR(__GNUC__) "." STR(__GNUC_MINOR__) ".x)";
  #endif
#elif defined(__VBCC__)
  const char * const yamcompiler = " (VBCC)";
#elif defined(__SASC)
  const char * const yamcompiler = " (SAS/C " STR(__VERSION__) "." STR(__REVISION__) ")";
#else
  const char * const yamcompiler = " (unknown)";
  #warning "unknown compiler specification"
#endif

/* no longer external visible, this is done by proto files! */
struct Library* DataTypesBase = NULL;
struct Library* GenesisBase   = NULL;
struct Library* IconBase      = NULL;
struct Library* IFFParseBase  = NULL;
struct Library* IntuitionBase = NULL;
struct Library* KeymapBase    = NULL;
struct Library* LocaleBase    = NULL;
struct Library* MiamiBase     = NULL;
struct Library* MUIMasterBase = NULL;
struct Library* OpenURLBase   = NULL;
struct Library* RexxSysBase   = NULL;
struct Library* SocketBase    = NULL;
struct Library* UtilityBase   = NULL;
struct Library* WorkbenchBase = NULL;
struct Library* XpkBase       = NULL;
struct Library* AmiSSLBase    = NULL;
struct Device*  TimerBase     = NULL;
struct Library* GfxBase       = NULL;
struct Library* LayersBase    = NULL;
struct Library* DiskfontBase  = NULL;
#if defined(__amigaos4__)
struct Library* ApplicationBase = NULL; // application.library
#endif

/* AmigaOS4 style interfaces */
#if defined(__amigaos4__)
struct DataTypesIFace*    IDataTypes    = NULL;
struct GenesisIFace*      IGenesis      = NULL;
struct IconIFace*         IIcon         = NULL;
struct IFFParseIFace*     IIFFParse     = NULL;
struct IntuitionIFace*    IIntuition    = NULL;
struct KeyMapIFace*       IKeymap       = NULL;
struct LocaleIFace*       ILocale       = NULL;
struct MiamiIFace*        IMiami        = NULL;
struct MUIMasterIFace*    IMUIMaster    = NULL;
struct OpenURLIFace*      IOpenURL      = NULL;
struct RexxSysIFace*      IRexxSys      = NULL;
struct SocketIFace*       ISocket       = NULL;
struct UtilityIFace*      IUtility      = NULL;
struct WorkbenchIFace*    IWorkbench    = NULL;
struct XpkIFace*          IXpk          = NULL;
struct AmiSSLIFace*       IAmiSSL       = NULL;
struct TimerIFace*        ITimer        = NULL;
struct GraphicsIFace*     IGraphics     = NULL;
struct LayersIFace*       ILayers       = NULL;
struct DiskfontIFace*     IDiskfont     = NULL;
struct ApplicationIFace*  IApplication  = NULL;
#endif /* __amigaos4__ */

struct WBStartup *WBmsg;

const char* const SigNames[3] = { ".signature", ".altsignature1", ".altsignature2" };
const char* const FolderNames[4] = { "incoming", "outgoing", "sent", "deleted" };

const char* const ContType[] =
{
   /*CT_TX_PLAIN */ "text/plain",
   /*CT_TX_HTML  */ "text/html",
   /*CT_TX_GUIDE */ "text/x-aguide",
   /*CT_AP_OCTET */ "application/octet-stream",
   /*CT_AP_PS    */ "application/postscript",
   /*CT_AP_PDF   */ "application/pdf",
   /*CT_AP_RTF   */ "application/rtf",
   /*CT_AP_LHA   */ "application/x-lha",
   /*CT_AP_LZX   */ "application/x-lzx",
   /*CT_AP_ZIP   */ "application/x-zip",
   /*CT_AP_AEXE  */ "application/x-amiga-executable",
   /*CT_AP_SCRIPT*/ "application/x-amigados-script",
   /*CT_AP_REXX  */ "application/x-rexx",
   /*CT_IM_JPG   */ "image/jpeg",
   /*CT_IM_GIF   */ "image/gif",
   /*CT_IM_PNG   */ "image/png",
   /*CT_IM_TIFF  */ "image/tiff",
   /*CT_IM_ILBM  */ "image/x-ilbm",
   /*CT_AU_AU    */ "audio/basic",
   /*CT_AU_8SVX  */ "audio/x-8svx",
   /*CT_AU_WAV   */ "audio/x-wav",
   /*CT_VI_MPG   */ "video/mpeg",
   /*CT_VI_MOV   */ "video/quicktime",
   /*CT_VI_ANIM  */ "video/x-anim",
   /*CT_VI_AVI   */ "video/x-msvideo",
   /*CT_ME_EMAIL */ "message/rfc822",
   NULL,
};

const char* const ContTypeDesc[] =
{
   MSG_CTtextplain,
   MSG_CTtexthtml,
   MSG_CTtextaguide,
   MSG_CTapplicationoctetstream,
   MSG_CTapplicationpostscript,
   MSG_CTapplicationpdf,
   MSG_CTapplicationrtf,
   MSG_CTapplicationlha,
   MSG_CTapplicationlzx,
   MSG_CTapplicationzip,
   MSG_CTapplicationamigaexe,
   MSG_CTapplicationadosscript,
   MSG_CTapplicationrexx,
   MSG_CTimagejpeg,
   MSG_CTimagegif,
   MSG_CTimagepng,
   MSG_CTimagetiff,
   MSG_CTimageilbm,
   MSG_CTaudiobasic,
   MSG_CTaudio8svx,
   MSG_CTaudiowav,
   MSG_CTvideompeg,
   MSG_CTvideoquicktime,
   MSG_CTvideoanim,
   MSG_CTvideomsvideo,
   MSG_CTmessagerfc822,
   NULL,
};

const char* const wdays[7] = { "Sun","Mon","Tue","Wed","Thu","Fri","Sat" };
const char* const months[12] = { "Jan","Feb","Mar","Apr","May","Jun","Jul","Aug","Sep","Oct","Nov","Dec" };
const char* const SecCodes[5] = { "none","sign","encrypt","sign+encrypt","anonymous" };
