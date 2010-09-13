#ifndef YAM_UTILITIES_H
#define YAM_UTILITIES_H

/***************************************************************************

 YAM - Yet Another Mailer
 Copyright (C) 1995-2000 by Marcel Beck <mbeck@yam.ch>
 Copyright (C) 2000-2010 by YAM Open Source Team

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

#if defined(__AROS__)
#include <sys/types.h>
#endif

#include <dos/dos.h>
#include <intuition/classusr.h>

#include "SDI_compiler.h"

#include "YAM_stringsizes.h"
#include "YAM_folderconfig.h"

// forward declarations
struct ReadMailData;
struct Mail;
struct codeset;

// Types of string outputs the DateStamp2String()
// function can handle. Please note that in case the
// order of these enums are changes, the user configuration
// may get invalid!
enum DateStampType
{
  DSS_DATE=0,       // the date (e.g. DD.MM.YYYY)
  DSS_TIME,         // the time (e.g. HH:MM:SS)
  DSS_SHORTTIME,    // the time without seconds (e.g. HH:MM)
  DSS_WEEKDAY,      // the weekday (e.g. Monday, Tuesday)
  DSS_DATETIME,     // Localized date&time output (via locale.library)
  DSS_RELDATETIME,  // Localized date&time with weekday substitution
  DSS_USDATETIME,   // American date&time format (mm-dd-yy hh:mm:ss)
  DSS_UNIXDATE,     // standard UNIX format
  DSS_BEAT,         // just time (in swatch beats)
  DSS_DATEBEAT,     // swatch beat datetime format
  DSS_RELDATEBEAT   // swatch beat datetime format with weekday subst.
};

enum TZConvert
{
  TZC_NONE,
  TZC_UTC,
  TZC_LOCAL
};

enum ReqFileType
{
  ASL_ABOOK=0,
  ASL_CONFIG,
  ASL_DETACH,
  ASL_ATTACH,
  ASL_REXX,
  ASL_PHOTO,
  ASL_IMPORT,
  ASL_EXPORT,
  ASL_FOLDER,
  ASL_ABOOK_LDIF,
  ASL_ABOOK_CSV,
  ASL_ABOOK_TAB,
  ASL_ABOOK_XML,
  ASL_GENERIC,
  ASL_MAX
};

enum OutputDefType
{
  OUT_STDOUT=0,
  OUT_NIL,
  OUT_CONSOLE
};

enum SizeFormat
{
  SF_DEFAULT=0, // format sizes in old-style   1,234,567 (bytes)
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

#define TIMEVAL(x)  (APTR)(x)

#else

#define TIMEVAL(x)  (x)

#endif

// define memory flags not existing on older platforms
#ifndef MEMF_SHARED
#define MEMF_SHARED MEMF_PUBLIC
#endif

// define an invalid BPTR value
#ifndef ZERO
#define ZERO (BPTR)NULL
#endif

// calculate the offset of an element within a structure
#ifndef OFFSET_OF
#define OFFSET_OF(s, o)					(IPTR)(&((s *)NULL)->o)
#endif

// Library open/close macros
#if defined(__amigaos4__)
#define INITLIB(lname, v, r, lbase, iname, iversion, ibase, req, url)  (InitLib((lname), (v), (r), (APTR)(lbase), (iname), (iversion), (APTR)(ibase), (req), (url)))
#define CLOSELIB(lib, iface)                                           { if((iface) && (lib)) { DropInterface((APTR)(iface)); iface = NULL; CloseLibrary((struct Library *)lib); lib = NULL; } }
#define GETINTERFACE(iname, iversion, iface, base)                     ((iface) = (APTR)GetInterface((struct Library *)(base), (iname), (iversion), NULL))
#define DROPINTERFACE(iface)                                           { DropInterface((APTR)(iface)); iface = NULL; }
#else
#define INITLIB(lname, v, r, lbase, iname, iversion, ibase, req, url)  (InitLib((lname), (v), (r), (APTR)(lbase), (req), (url)))
#define CLOSELIB(lib, iface)                                           { if((lib)) { CloseLibrary((struct Library *)lib); lib = NULL; } }
#define GETINTERFACE(iname, iversion, iface, base)                     TRUE
#define DROPINTERFACE(iface)                                           ((void)0)
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

#ifndef MAX
#define MAX(a,b)              (((a) > (b)) ? (a) : (b))
#endif
#ifndef MIN
#define MIN(a,b)              (((a) < (b)) ? (a) : (b))
#endif

#define ARRAY_SIZE(x)         (sizeof(x[0]) ? sizeof(x)/sizeof(x[0]) : 0)

#define VERSION_IS_AT_LEAST(ver, rev, minver, minrev) (((ver) > (minver)) || ((ver) == (minver) && (rev) == (minrev)) || ((ver) == (minver) && (rev) > (minrev)))
#define LIB_VERSION_IS_AT_LEAST(lib, minver, minrev)  VERSION_IS_AT_LEAST(((struct Library *)(lib))->lib_Version, ((struct Library *)(lib))->lib_Revision, minver, minrev)

// special flagging macros
#define isFlagSet(v,f)        (((v) & (f)) == (f))  // return TRUE if the flag is set
#define hasFlag(v,f)          (((v) & (f)) != 0)    // return TRUE if one of the flags in f is set in v
#define isFlagClear(v,f)      (((v) & (f)) == 0)    // return TRUE if flag f is not set in v
#define SET_FLAG(v,f)         ((v) |= (f))          // set the flag f in v
#define CLEAR_FLAG(v,f)       ((v) &= ~(f))         // clear the flag f in v
#define MASK_FLAG(v,f)        ((v) &= (f))          // mask the variable v with flag f bitwise

// Wrapper define to be able to use the standard call of MUI_Request
#ifdef MUI_Request
#undef MUI_Request
#endif
#define MUI_Request YAMMUIRequest

// function macros
#define DisposeModulePush(m)  DoMethod(G->App, MUIM_Application_PushMethod, G->App, 3, MUIM_CallHook, &DisposeModuleHook, m)
#define FreeStrBuf(str)       ((str != NULL) ? free(((char *)(str))-sizeof(size_t)) : (void)0)
#define isSpace(c)            ((BOOL)(G->Locale ? (IsSpace(G->Locale, (ULONG)(c)) != 0) : (isspace((c)) != 0)))
#define isGraph(c)            ((BOOL)(G->Locale ? (IsGraph(G->Locale, (ULONG)(c)) != 0) : (isgraph((c)) != 0)))
#define isAlNum(c)            ((BOOL)(G->Locale ? (IsAlNum(G->Locale, (ULONG)(c)) != 0) : (isalnum((c)) != 0)))
#define isValidMailFile(file) (!(strlen(file) < 17 || file[12] != '.' || file[16] != ',' || !isdigit(file[13])))
#define Bool2Txt(b)           ((b) ? "Y" : "N")
#define Txt2Bool(t)           (BOOL)(toupper((int)*(t)) == 'Y' || (int)*(t) == '1')
#define SafeStr(str)          (((str) != NULL) ? (str) : "<NULL>")

#define IterateList(list, node)           for((node) = GetHead((struct List *)(list)); (node) != NULL; (node) = GetSucc(node))

#if !defined(IsMinListEmpty)
#define IsMinListEmpty(x) \
    ( (struct MinList *)((x)->mlh_TailPred) == (struct MinList *)(x) )
#endif

// LogFile enums and macros
enum LFMode
{
  LF_NONE=0,
  LF_NORMAL,
  LF_VERBOSE,
  LF_ALL
};

// external variables
extern int            BusyLevel;
extern struct Hook    GeneralDesHook;
extern struct Hook    DisposeModuleHook;
#if defined(__amigaos4__)
extern struct Hook    ExamineDirMatchHook;
#endif

// all the utility prototypes
struct Mail *AddMailToList(const struct Mail *mail, struct Folder *folder);
void     AddZombieFile(const char *fileName);
char *   AllocReqText(const char *s);
char *   AllocStrBuf(size_t initlen);
void     AppendToLogfile(enum LFMode, int id, const char *text, ...);
BOOL     Busy(const char *text, const char *parameter, int cur, int max);
BOOL     CheckPrinter(void);
void     ClearMailList(struct Folder *folder, BOOL resetstats);
BOOL     DeleteZombieFiles(BOOL force);
void     CloseTempFile(struct TempFile *tf);
ULONG    CRC32(const void *buffer, unsigned int count, ULONG crc);
char *   strippedCharsetName(const struct codeset* codeset);
BOOL     ConvertCRLF(char *in, char *out, BOOL to);
unsigned char ConvertKey(const struct IntuiMessage *imsg);
BOOL     CopyFile(const char *dest, FILE *destfh, const char *sour, FILE *sourfh);
BOOL     MoveFile(const char *oldname, const char *newname);
char *   CreateFilename(const char * const file, char *fullPath, const size_t fullPathSize);
BOOL     CreateDirectory(const char *dir);
int      TZtoMinutes(const char *tzone);
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
BOOL     String2DateStamp(struct DateStamp *dst, const char *string, enum DateStampType mode, enum TZConvert tzc);
BOOL     String2TimeVal(struct TimeVal *dst, const char *string, enum DateStampType mode, enum TZConvert tzc);
char *   Decrypt(const char *source);
BOOL     DeleteMailDir(const char *dir, BOOL isroot);
const char *DescribeCT(const char *ct);
void     DisplayMailList(struct Folder *fo, Object *lv);
void     DisplayStatistics(struct Folder *fo, BOOL updateAppIcon);
void     DisposeModule(void *modptr);
BOOL     DoPack(const char *file, const char *newfile, const struct Folder *folder);
void *   DuplicateNode(const void *node, size_t size);
BOOL     DumpClipboard(FILE *out);
BOOL     EditorToFile(Object *editor, const char *file);
char *   Encrypt(const char *source);
void     GetPubScreenName(const struct Screen *screen, char *pubName, ULONG pubNameSize);
BOOL     TimeHasElapsed(struct TimeVal *last, ULONG micros);
char *   GetRealPath(const char *path);
LONG     LaunchCommand(const char *cmd, BOOL asynch, enum OutputDefType outdef);
char *   BuildAddress(char *buffer, size_t buflen, const char *address, const char *name);
void     ExtractAddress(const char *line, struct Person *pe);
char *   FileToBuffer(const char *file);
BOOL     FileToEditor(const char *file, Object *editor, const BOOL changed, const BOOL useStyles, const BOOL useColors);
LONG     FileCount(const char *directory, const char *pattern);
char *   AddPath(char *dst, const char *src, const char *add, size_t size);
void     FinishUnpack(const char *file);
void     FormatSize(LONG size, char *buffer, int buflen, enum SizeFormat forcedPrecision);
time_t   GetDateStamp(void);
ssize_t  GetLine(char **buffer, size_t *size, FILE *fh);
void     GetMailFile(char *string, const size_t stringSize, const struct Folder *folder, const struct Mail *mail);
char *   GetNextLine(char *p1);
int      GetSimpleID(void);
BOOL     GotoURLPossible(void);
BOOL     GotoURL(const char *url, BOOL newWindow);
const char *IdentifyFile(const char *fname);
char *   itoa(int val);
void     LoadLayout(void);
BOOL     MailExists(const struct Mail *mailptr, struct Folder *folder);
BOOL     MatchNoCase(const char *string, const char *match);
char *   MyStrChr(const char *s, const char c);
struct TempFile *OpenTempFile(const char *mode);
BOOL     AllFolderLoaded(void);
void     PGPClearPassPhrase(BOOL force);
LONG     PGPCommand(const char *progname, const char *options, const int flags);
void     PGPGetPassPhrase(void);
BOOL     PlaySound(const char *filename);
void     QuoteText(FILE *out, const char *src, const int len, const int line_max);
void     RemoveMailFromList(struct Mail *mail, BOOL closeWindows);
BOOL     RenameFile(const char *oldname, const char *newname);
BOOL     RepackMailFile(struct Mail *mail, enum FolderMode dstMode, const char *passwd);
struct FileReqCache *ReqFile(enum ReqFileType num, Object *win, const char *title, int mode, const char *drawer, const char *file);
void     FreeFileReqCache(struct FileReqCache *frc);
BOOL     SafeOpenWindow(Object *obj);
void     SaveLayout(BOOL permanent);
void     SimpleWordWrap(const char *filename, int wrapsize);
char *   StartUnpack(const char *file, char *newfile, const struct Folder *folder);
char *   StrBufCat(char *strbuf, const char *source);
char *   StrBufCpy(char *strbuf, const char *source);
char *   AppendToBuffer(char *buf, int *wptr, int *len, const char *add);
char *   StripUnderscore(const char *label);
void     ReplaceInvalidChars(char *name);
char *   SWSSearch(const char *str1, const char*str2);
void     ToLowerCase(char *str);
int      TransferMailFile(BOOL copyit, struct Mail *mail, struct Folder *dstfolder);
char *   Trim(char *s);
char *   TrimEnd(char *s);
char *   TrimStart(const char *s);
char *   UnquoteString(const char *s, BOOL new);
int      ReadUInt32(FILE *stream, ULONG *value);
int      WriteUInt32(FILE *stream, ULONG value);

#if !defined(__amigaos4__)
void MyBltMaskBitMapRastPort(struct BitMap *srcBitMap, LONG xSrc, LONG ySrc, struct RastPort *destRP, LONG xDest, LONG yDest, LONG xSize, LONG ySize, ULONG minterm, APTR bltMask);
#endif

#endif /* YAM_UTILITIES_H */
