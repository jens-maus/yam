/***************************************************************************

 YAM - Yet Another Mailer
 Copyright (C) 1995-2000 by Marcel Beck <mbeck@yam.ch>
 Copyright (C) 2000-2004 by YAM Open Source Team

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
#else
  #define CPU ""
#endif

/* the version stuff */
#define __YAM_VERSION   "2.5"
#define __YAM_DEVEL     "-dev"
#define __YAM_COPYRIGHT "Copyright (C) 2000-2004 YAM Open Source Team"

char * yamversion       = "YAM " __YAM_VERSION __YAM_DEVEL CPU;
char * yamversionver    = __YAM_VERSION __YAM_DEVEL CPU;
char * yamversionstring = "$VER: YAM " __YAM_VERSION __YAM_DEVEL CPU " (" __YAM_VERDATE ") " __YAM_COPYRIGHT;
char * yamcopyright     = __YAM_COPYRIGHT;
char * yamversiondate   = __YAM_VERDATE;
unsigned long yamversiondays = __YAM_VERDAYS;

#if defined(__amigaos4__)
static const STRPTR Stack = "$STACK:65536\n";
#elif defined(__SASC) || (defined(__GNUC__) && defined(__libnix__))
  /* GCC (libnix) supports the same as SAS/C! */
  long __stack = 65536;
  long __buffsize = 8192;
  long _MSTEP = 16384;
#elif defined(__VBCC__) /* starting with VBCC 0.8 release */
  long __stack = 65536;
#else
  #error "initial stack specification failed"
#endif

struct WBStartup *WBmsg;

/* no longer external visible, this is done by proto files! */
struct Library *       CManagerBase   = NULL;
struct Library *       DataTypesBase  = NULL;
struct Library *       GenesisBase    = NULL;
struct Library *       IconBase       = NULL;
struct Library *       IFFParseBase   = NULL;
struct IntuitionBase * IntuitionBase  = NULL;
struct Library *       KeymapBase     = NULL;
struct LocaleBase *    LocaleBase     = NULL;
struct Library *       MiamiBase      = NULL;
struct Library *       MUIMasterBase  = NULL;
struct Library *       OpenURLBase    = NULL;
struct RxsLib *        RexxSysBase    = NULL;
struct Library *       SocketBase     = NULL;
struct UtilityBase *   UtilityBase    = NULL;
struct Library *       WorkbenchBase  = NULL;
struct Library *       XpkBase        = NULL;
struct Library *       AmiSSLBase     = NULL;
struct Device *        TimerBase      = NULL;

// lets defined the AmigaOS4 style interfaces of
// our used libraries
#if defined(__amigaos4__)
struct ExecIFace*       IExec         = NULL;
struct CManagerIFace*   ICManager     = NULL;
struct DataTypesIFace*  IDataTypes    = NULL;
struct GenesisIFace*    IGenesis      = NULL;
struct IconIFace*       IIcon         = NULL;
struct IFFParseIFace*   IIFFParse     = NULL;
struct IntuitionIFace*  IIntuition    = NULL;
struct KeyMapIFace*     IKeymap       = NULL;
struct LocaleIFace*     ILocale       = NULL;
struct MiamiIFace*      IMiami        = NULL;
struct MUIMasterIFace*  IMUIMaster    = NULL;
struct OpenURLIFace*    IOpenURL      = NULL;
struct RexxSysIFace*    IRexxSys      = NULL;
struct SocketIFace*     ISocket       = NULL;
struct UtilityIFace*    IUtility      = NULL;
struct WorkbenchIFace*  IWorkbench    = NULL;
struct XpkIFace*        IXpk          = NULL;
struct AmiSSLIFace*     IAmiSSL       = NULL;
struct TimerIFace*      ITimer        = NULL;
#endif

char *Status[9] = { "U","O","F","R","W","E","H","S","N" };
char *SigNames[3] = { ".signature", ".altsignature1", ".altsignature2" };
char *FolderNames[4] = { "incoming", "outgoing", "sent", "deleted" };

char *ContType[] =
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

char *ContTypeDesc[] =
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

char *wdays[7] = { "Sun","Mon","Tue","Wed","Thu","Fri","Sat" };
char *months[12] = { "Jan","Feb","Mar","Apr","May","Jun","Jul","Aug","Sep","Oct","Nov","Dec" };
char *SecCodes[5] = { "none","sign","encrypt","sign+encrypt","anonymous" };
