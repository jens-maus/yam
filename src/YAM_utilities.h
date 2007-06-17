#ifndef YAM_UTILITIES_H
#define YAM_UTILITIES_H

/***************************************************************************

 YAM - Yet Another Mailer
 Copyright (C) 1995-2000 by Marcel Beck <mbeck@yam.ch>
 Copyright (C) 2000-2007 by YAM Open Source Team

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

#include <stdio.h>
#include <time.h>

#include <dos/dos.h>
#include <intuition/classusr.h>

#include "SDI_compiler.h"

#include "YAM_folderconfig.h"
#include "YAM_stringsizes.h"

// some forward declarations
#ifndef YAM_FOLDERCONFIG_H
struct Folder;
#endif

#ifndef YAM_READ_H
struct ReadMailData;
#endif

// Types of string outputs the DateStamp2String()
// function can handle. Please note that in case the
// order of these enums are changes, the user configuration
// may get invalid!
enum DateStampType
{
  DSS_DATE=0,       // just the date
  DSS_TIME,         // just the time
  DSS_WEEKDAY,      // just the weekday
  DSS_DATETIME,     // Localized date&time output (via locale.library)
  DSS_RELDATETIME,  // Localized date&time with weekday substitution
  DSS_USDATETIME,   // American date&time format (mm-dd-yy hh:mm:ss)
  DSS_UNIXDATE,     // standard UNIX format
  DSS_BEAT,         // just time (in swatch beats)
  DSS_DATEBEAT,     // swatch beat datetime format
  DSS_RELDATEBEAT   // swatch beat datetime format with weekday subst.
};

enum TZConvert { TZC_NONE, TZC_UTC, TZC_LOCAL };

enum ReqFileType { ASL_ABOOK=0, ASL_CONFIG, ASL_DETACH, ASL_ATTACH,
  ASL_REXX, ASL_PHOTO, ASL_IMPORT, ASL_EXPORT, ASL_FOLDER,
  ASL_ABOOK_LDIF, ASL_ABOOK_CSV, ASL_ABOOK_TAB,
  ASL_MAX };

enum OutputDefType { OUT_DOS=0, OUT_NIL };

enum FType { FIT_NONEXIST=0, FIT_FILE, FIT_DRAWER };

enum SizeFormat { SF_DEFAULT=0, // format sizes in old-style   1,234,567 (bytes)
                  SF_MIXED,     // format in mixed mode        1.234 GB - 12.34 MB - 123.4 KB - 1234 B
                  SF_1PREC,     // format in one-precision     1.2 GB - 12.3 MB - 123.4 KB - 1234 B
                  SF_2PREC,     // format in two-precision     1.23 GB - 12.34 MB - 123.45 KB - 1234 B
                  SF_3PREC,     // format in three precision   1.234 GB - 12.345 MB - 123.456 KB - 1234 B
                  SF_AUTO       // format automatically via C->SizeFormat
                };

struct Person
{
   char Address[SIZE_ADDRESS];
   char RealName[SIZE_REALNAME];
};

struct TempFile
{
   FILE *FP;
   char  Filename[SIZE_PATH+SIZE_MFILE];
};

struct NewToolbarEntry
{
  const void *label;
  const void *help;
};

struct FileReqCache
{
  char *file;     // pointer to filename string
  char *drawer;   // pointer to drawer string
  char *pattern;  // pointer to pattern string
  int  numArgs;   // if more than one file was selected > 0
  char **argList; // pointer list to arguments.
  long left_edge; // last left edge position of requester
  long top_edge;  // last top edge position of requester
  long width;     // last width of requester
  long height;    // last height of requester
  BOOL used;      // cache is in use
};

// since the Amiga's timeval structure was renamed to
// "struct TimeVal" in OS4 (to prevent clashes with the POSIX one)
// we require to define that slightly compatible structure on our
// own in case we compile YAM for something else than OS4 or in case
// an older SDK is used.
#if !defined(__amigaos4__) || !defined(__NEW_TIMEVAL_DEFINITION_USED__)
struct TimeVal
{
  ULONG Seconds;
  ULONG Microseconds;
};

struct TimeRequest
{
  struct IORequest Request;
  struct TimeVal   Time;
};

#define TIMEVAL(x)  (struct timeval *)(x)

#else

#define TIMEVAL(x)  (x)

#endif

// Library open/close macros
#if defined(__amigaos4__)
#define INITLIB(lname, v, r, lbase, iname, ibase, req, url)  (InitLib((lname), (v), (r), (APTR)(lbase), (iname), (APTR)(ibase), (req), (url)))
#define CLOSELIB(lib, iface)              { if((iface) && (lib)) { DropInterface((APTR)(iface)); iface = NULL; CloseLibrary((struct Library *)lib); lib = NULL; } }
#define GETINTERFACE(iname, iface, base)  ((iface) = (APTR)GetInterface((struct Library *)(base), (iname), 1L, NULL))
#define DROPINTERFACE(iface)              { DropInterface((APTR)(iface)); iface = NULL; }
#else
#define INITLIB(lname, v, r, lbase, iname, ibase, req, url)  (InitLib((lname), (v), (r), (APTR)(lbase), (req), (url)))
#define CLOSELIB(lib, iface)              { if((lib)) { CloseLibrary((struct Library *)lib); lib = NULL; } }
#define GETINTERFACE(iname, iface, base)  TRUE
#define DROPINTERFACE(iface)              ((void)0)
#endif

// misc defines
#define PGPLOGFILE          "T:PGP.log"
#define NOERRORS            (1<<4)
#define KEEPLOG             (1<<5)
#define hasNoErrorsFlag(v)  (isFlagSet((v), NOERRORS))
#define hasKeepLogFlag(v)   (isFlagSet((v), KEEPLOG))

// RequesterMode flags & macros
#define REQF_NONE             0
#define REQF_SAVEMODE         (1<<0)
#define REQF_MULTISELECT      (1<<1)
#define REQF_DRAWERSONLY      (1<<2)
#define hasSaveModeFlag(v)    (isFlagSet((v), REQF_SAVEMODE))
#define hasMultiSelectFlag(v) (isFlagSet((v), REQF_MULTISELECT))
#define hasDrawersOnlyFlag(v) (isFlagSet((v), REQF_DRAWERSONLY))

// special Macros for the Busy Handling of the InfoBar.
#define BUSYLEVEL             5
#define BusyEnd()             Busy("", NULL, 0, 0)
#define BusySet(c)            Busy(NULL, NULL, c, 0)
#define BusyText(t, p)        Busy(t, p, 0, 0)
#define BusyGauge(t, p, max)  Busy(t, p, 0, max)
#define BusyGaugeInt(t, p, m) Busy(t, p, -1, m)

// attachment requester flags & macros
#define ATTREQ_DISP       (1<<0)
#define ATTREQ_SAVE       (1<<1)
#define ATTREQ_PRINT      (1<<2)
#define ATTREQ_MULTI      (1<<3)
#define isDisplayReq(v)   (isFlagSet((v), ATTREQ_DISP))
#define isSaveReq(v)      (isFlagSet((v), ATTREQ_SAVE))
#define isPrintReq(v)     (isFlagSet((v), ATTREQ_PRINT))
#define isMultiReq(v)     (isFlagSet((v), ATTREQ_MULTI))

#define ARRAY_SIZE(x)     (sizeof(x[0]) ? sizeof(x)/sizeof(x[0]) : 0)

// special flagging macros
#define isFlagSet(v,f)      (((v) & (f)) == (f))  // return TRUE if the flag is set
#define hasFlag(v,f)        (((v) & (f)) != 0)    // return TRUE if one of the flags in f is set in v
#define isFlagClear(v,f)    (((v) & (f)) == 0)    // return TRUE if flag f is not set in v
#define SET_FLAG(v,f)       ((v) |= (f))          // set the flag f in v
#define CLEAR_FLAG(v,f)     ((v) &= ~(f))         // clear the flag f in v
#define MASK_FLAG(v,f)      ((v) &= (f))          // mask the variable v with flag f bitwise

// some filetype handling macros
#define isFile(etype)     (etype < 0)
#define isDrawer(etype)   (etype >= 0 && etype != ST_SOFTLINK && etype != ST_LINKDIR)

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

// Wrapper define to be able to use the standard call of MUI_Request
#ifdef MUI_Request
#undef MUI_Request
#endif
#define MUI_Request YAMMUIRequest

// function macros
#define BuildAddrName2(p)     BuildAddrName((p)->Address, (p)->RealName)
#define SetHelp(o,str)        set(o, MUIA_ShortHelp, tr(str))
#define DisposeModulePush(m)  DoMethod(G->App, MUIM_Application_PushMethod, G->App, 3, MUIM_CallHook, &DisposeModuleHook, m)
#define FreeStrBuf(str)       ((str) ? free(((char *)(str))-sizeof(size_t)) : (void)0)
#define isSpace(c)            ((BOOL)(G->Locale ? (IsSpace(G->Locale, (ULONG)(c)) != 0) : (isspace((c)) != 0)))
#define isGraph(c)            ((BOOL)(G->Locale ? (IsGraph(G->Locale, (ULONG)(c)) != 0) : (isgraph((c)) != 0)))
#define isAlNum(c)            ((BOOL)(G->Locale ? (IsAlNum(G->Locale, (ULONG)(c)) != 0) : (isalnum((c)) != 0)))
#define isValidMailFile(file) (!(strlen(file) < 17 || file[12] != '.' || file[16] != ',' || !isdigit(file[13])))
#define Bool2Txt(b)           ((b) ? "Y" : "N")
#define Txt2Bool(t)           (BOOL)(toupper((int)*(t)) == 'Y' || (int)*(t) == '1')
#define GetMUIString(a, o, l) strlcpy((a), (char *)xget((o), MUIA_String_Contents), (l))
#define GetMUIText(a, o, l)   strlcpy((a), (char *)xget((o), MUIA_Text_Contents), (l))

// LogFile enums and macros
enum LFMode { LF_NONE=0, LF_NORMAL, LF_VERBOSE, LF_ALL };

// external variables
extern int            BusyLevel;
extern struct Hook    GeneralDesHook;
extern struct Hook    DisposeModuleHook;

// all the utility prototypes
struct Mail *AddMailToList(struct Mail *mail, struct Folder *folder);
void     AddZombieFile(const char *fileName);
char *   AllocReqText(char *s);
char *   AllocStrBuf(size_t initlen);
void     AppendToLogfile(enum LFMode, int id, const char *text, ...);
struct Part *AttachRequest(const char *title, const char *body, const char *yestext, const char *notext, int mode, struct ReadMailData *rmData);
char *   BuildAddrName(const char *address, const char *name);
BOOL     Busy(const char *text, const char *parameter, int cur, int max);
BOOL     CheckPrinter(void);
LONG     CheckboxRequest(Object *win, UNUSED LONG flags, const char *tit, ULONG numBoxes, const char *text, ...);
void     ClearMailList(struct Folder *folder, BOOL resetstats);
BOOL     DeleteZombieFiles(BOOL force);
void     CloseTempFile(struct TempFile *tf);
ULONG    CRC32(void *buffer, unsigned int count, ULONG crc);
ULONG    CompressMsgID(char *msgid);
BOOL     ConvertCRLF(char *in, char *out, BOOL to);
ULONG    ConvertKey(struct IntuiMessage *imsg);
BOOL     isChildOfGroup(Object *group, Object *child);
BOOL     isChildOfFamily(Object *family, Object *child);
void     MyBltMaskBitMapRastPort(struct BitMap *srcBitMap, LONG xSrc, LONG ySrc, struct RastPort *destRP, LONG xDest, LONG yDest, LONG xSize, LONG ySize, ULONG minterm, APTR bltMask);
BOOL     CopyFile(const char *dest, FILE *destfh, const char *sour, FILE *sourfh);
BOOL     MoveFile(const char *oldname, const char *newname);
char *   CreateFilename(const char * const file);
BOOL     CreateDirectory(const char *dir);
int      TZtoMinutes(char *tzone);
void     DateStampUTC(struct DateStamp *ds);
void     GetSysTimeUTC(struct TimeVal *tv);
void     TimeValTZConvert(struct TimeVal *tv, enum TZConvert tzc);
void     DateStampTZConvert(struct DateStamp *ds, enum TZConvert tzc);
void     TimeVal2DateStamp(const struct TimeVal *tv, struct DateStamp *ds, enum TZConvert tzc);
void     DateStamp2TimeVal(const struct DateStamp *ds, struct TimeVal *tv, enum TZConvert tzc);
BOOL     TimeVal2String(char *dst, int dstlen, const struct TimeVal *tv, enum DateStampType mode, enum TZConvert tzc);
BOOL     DateStamp2String(char *dst, int dstlen,  struct DateStamp *date, enum DateStampType mode, enum TZConvert tzc);
BOOL     DateStamp2RFCString(char *dst, const int dstlen, const struct DateStamp *date, const int timeZone, const BOOL convert);
long     DateStamp2Long(struct DateStamp *date);
BOOL     String2DateStamp(struct DateStamp *dst, char *string, enum DateStampType mode, enum TZConvert tzc);
BOOL     String2TimeVal(struct TimeVal *dst, char *string, enum DateStampType mode, enum TZConvert tzc);
char *   Decrypt(char *source);
BOOL     DeleteMailDir(const char *dir, BOOL isroot);
const char *DescribeCT(const char *ct);
void     DisplayMailList(struct Folder *fo, Object *lv);
void     DisplayAppIconStatistics(void);
void     DisplayStatistics(struct Folder *fo, BOOL updateAppIcon);
void     DisposeModule(void *modptr);
BOOL     DoPack(char *file, char *newfile, struct Folder *folder);
BOOL     DumpClipboard(FILE *out);
BOOL     EditorToFile(Object *editor, char *file);
char *   Encrypt(char *source);
char *   GetRealPath(char *path);
BOOL     ExecuteCommand(char *cmd, BOOL asynch, enum OutputDefType outdef);
void     ExtractAddress(const char *line, struct Person *pe);
BOOL     FileExists(const char *filename);
int      FileSize(const char *filename);
long     FileProtection(const char *filename);
char *   FileToBuffer(const char *file);
BOOL     FileToEditor(char *file, Object *editor);
enum FType FileType(const char *filename);
char *   FileComment(char *filename);
struct DateStamp *FileDate(char *filename);
long     FileTime(const char *filename);
long     FileCount(const char *directory);
void     FinishUnpack(char *file);
struct Folder *FolderRequest(const char *title, const char *body, const char *yestext, const char *notext, struct Folder *exclude, Object *parent);
void     FormatSize(LONG size, char *buffer, int buflen, enum SizeFormat forcedPrecision);
time_t   GetDateStamp(void);
const char *GetFolderDir(const struct Folder *fo);
char *   GetLine(FILE *fh, char *buffer, int bufsize);
char *   GetMailFile(char *string, const struct Folder *folder, const struct Mail *mail);
BOOL     GetMUICheck(Object *obj);
int      GetMUICycle(Object *obj);
int      GetMUIInteger(Object *obj);
int      GetMUINumer(Object *obj);
int      GetMUIRadio(Object *obj);
char *   GetNextLine(char *p1);
int      GetSimpleID(void);
void     GotoURL(const char *url);
const char *IdentifyFile(const char *fname);
void     InfoWindow(const char *title, const char *body, const char *oktext, APTR parent);
void     InsertAddresses(Object *obj, char **addr, BOOL add);
char *   itoa(int val);
void     LoadLayout(void);
BOOL     MailExists(struct Mail *mailptr, struct Folder *folder);
Object * MakeButton(const char *txt);
Object * MakeCheck(const char *label);
Object * MakeCheckGroup(Object **check, const char *label);
Object * MakeCycle(const char *const *labels, const char *label);
Object * MakeInteger(int maxlen, const char *label);
Object * MakeNumeric(int min, int max, BOOL percent);
Object * MakePassString(const char *label);
Object * MakePGPKeyList(Object **st, BOOL secret, const char *label);
Object * MakeString(int maxlen, const char *label);
Object * MakeAddressField(Object **string, const char *label, const Object *help, int abmode, int winnum, BOOL allowmulti);
BOOL     MatchNoCase(const char *string, const char *match);
char *   MyStrChr(const char *s, const char c);
struct TempFile *OpenTempFile(const char *mode);
BOOL     AllFolderLoaded(void);
BOOL     PFExists(char *path, const char *file);
void     PGPClearPassPhrase(BOOL force);
int      PGPCommand(const char *progname, const char *options, int flags);
void     PGPGetPassPhrase(void);
void     PlaySound(char *filename);
void     QuoteText(FILE *out, const char *src, const int len, const int line_max);
void     RemoveMailFromList(struct Mail *mail, BOOL closeWindows);
BOOL     RenameFile(const char *oldname, const char *newname);
BOOL     RepackMailFile(struct Mail *mail, enum FolderMode dstMode, char *passwd);
struct FileReqCache *ReqFile(enum ReqFileType num, Object *win, const char *title, int mode, const char *drawer, const char *file);
void     FreeFileReqCache(struct FileReqCache *frc);
BOOL     SafeOpenWindow(Object *obj);
void     SaveLayout(BOOL permanent);
char     ShortCut(const char *label);
void     SimpleWordWrap(char *filename, int wrapsize);
char *   StartUnpack(const char *file, char *newfile, const struct Folder *folder);
char *   StrBufCat(char *strbuf, const char *source);
char *   StrBufCpy(char *strbuf, const char *source);
char *   AppendToBuffer(char *buf, int *wptr, int *len, const char *add);
int      StringRequest(char *string, int size, const char *title, const char *body,
                       const char *yestext, const char *alttext, const char *notext, BOOL secret, Object *parent);
char *   StripUnderscore(const char *label);
char *   stristr(const char *a, const char *b);
char *   SWSSearch(char *str1, char*str2);
void     ToLowerCase(char *str);
int      TransferMailFile(BOOL copyit, struct Mail *mail, struct Folder *dstfolder);
char *   Trim(char *s);
char *   TrimEnd(char *s);
char *   TrimStart(char *s);
LONG     YAMMUIRequest(Object *app, Object *win, UNUSED LONG flags, const char *title, const char *gadgets, const char *format, ...);
char *   UnquoteString(const char *s, BOOL new);
int      ReadUInt32(FILE *stream, ULONG *value);
int      WriteUInt32(FILE *stream, ULONG value);

// Here we define inline functions that should be inlined by
// the compiler, if possible.

/// xget()
//  Gets an attribute value from a MUI object
ULONG xget(Object *obj, const ULONG attr);
#if defined(__GNUC__)
  // please note that we do not evaluate the return value of GetAttr()
  // as some attributes (e.g. MUIA_Selected) always return FALSE, even
  // when they are supported by the object. But setting b=0 right before
  // the GetAttr() should catch the case when attr doesn't exist at all
  #define xget(OBJ, ATTR) ({ULONG b=0; GetAttr(ATTR, OBJ, &b); b;})
#endif
///

#endif /* YAM_UTILITIES_H */
