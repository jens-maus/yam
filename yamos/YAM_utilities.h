#ifndef YAM_UTILITIES_H
#define YAM_UTILITIES_H

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

#include <stdio.h>
#include <time.h>

#include <dos/dos.h>
#include <intuition/classusr.h>
#include <mui/Toolbar_mcc.h>

#include "SDI_compiler.h"
#include "YAM_stringsizes.h"

#define STR(x)  STR2(x)
#define STR2(x) #x

#ifndef YAM_FOLDERCONFIG_H
struct Folder;
#endif

#ifndef YAM_MIME_H
struct TranslationTable;
#endif

enum DateStampType { DSS_DATE, DSS_TIME, DSS_WEEKDAY, DSS_DATETIME,
  DSS_USDATETIME, DSS_UNIXDATE, DSS_BEAT, DSS_DATEBEAT };

enum ReqFileType { ASL_ABOOK=0, ASL_CONFIG, ASL_DETACH, ASL_ATTACH,
  ASL_REXX, ASL_PHOTO, ASL_IMPORT, ASL_FOLDER };

struct Person
{       
   char Address[SIZE_ADDRESS];
   char RealName[SIZE_REALNAME];
};

struct ExpandTextData
{
   char *            OS_Name;
   char *            OS_Address;
   char *            OM_Subject;
   struct DateStamp *OM_Date;
   char *            OM_MessageID;
   char *            R_Name;
   char *            R_Address;
   char *            HeaderFile;
};

struct MailInfo
{
   int   Pos;
   BOOL  Display;
   char *FName;
};

struct TempFile
{
   FILE *FP;
   char  Filename[SIZE_PATHFILE];
};

struct BodyChunkData
{
   ULONG * Colors;
   UBYTE * Body;
   int     Width;
   int     Height;
   int     Depth;
   int     Compression;
   int     Masking;
   char    File[SIZE_NAME];
};

struct Data2D
{
   int Allocated;
   int Used;
   char **Data;
};

struct NewToolbarEntry
{
   APTR label;
   APTR help;
};

// misc defines
#define PGPLOGFILE    "T:PGP.log"
#define NOERRORS      16
#define KEEPLOG       32
#define hasNoErrorsFlag(v)  (isFlagSet((v), NOERRORS))
#define hasKeepLogFlag(v)   (isFlagSet((v), KEEPLOG))

// RequesterMode flags & macros
#define REQF_NONE        0
#define REQF_SAVEMODE    1
#define REQF_MULTISELECT 2
#define REQF_DRAWERSONLY 4
#define hasSaveModeFlag(v)    (isFlagSet((v), REQF_SAVEMODE))
#define hasMultiSelectFlag(v) (isFlagSet((v), REQF_MULTISELECT))
#define hasDrawersOnlyFlag(v) (isFlagSet((v), REQF_DRAWERSONLY))

// special Macros for the Busy Handling of the InfoBar.
#define BUSYLEVEL             5
#define BusyEnd               Busy("", NULL, 0, 0)
#define BusySet(c)            Busy(NULL, NULL, c, 0)
#define BusyText(t, p)        Busy(t, p, 0, 0)
#define BusyGauge(t, p, max)  Busy(t, p, 0, max)

#define OUT_DOS       ((BPTR)0)
#define OUT_NIL       ((BPTR)1)

// attachment requester flags & macros
#define ATTREQ_DISP       (1<<0)
#define ATTREQ_SAVE       (1<<1)
#define ATTREQ_PRINT      (1<<2)
#define ATTREQ_MULTI      (1<<3)
#define isDisplayReq(v)   (isFlagSet((v), ATTREQ_DISP))
#define isSaveReq(v)      (isFlagSet((v), ATTREQ_SAVE))
#define isPrintReq(v)     (isFlagSet((v), ATTREQ_PRINT))
#define isMultiReq(v)     (isFlagSet((v), ATTREQ_MULTI))

#define ARRAY_SIZE(x) (sizeof(x)/sizeof(x[0]))

// special flagging macros
#define isFlagSet(v,f)      (((v) & (f)) == (f))  // return TRUE if the flag is set
#define hasFlag(v,f)        (((v) & (f)) != 0)    // return TRUE if one of the flags in f is set in v
#define isFlagClear(v,f)    (((v) & (f)) == 0)    // return TRUE if flag f is not set in v
#define SET_FLAG(v,f)       ((v) |= (f))          // set the flag f in v
#define CLEAR_FLAG(v,f)     ((v) &= ~(f))         // clear the flag f in v
#define MASK_FLAG(v,f)      ((v) &= (f))          // mask the variable v with flag f bitwise

// some fileinfoblock handling macros
#define isFile(fib)     ((fib)->fib_DirEntryType < 0)
#define isDrawer(fib)   ((fib)->fib_DirEntryType >= 0 && \
                        (fib)->fib_DirEntryType != ST_SOFTLINK && \
                        (fib)->fib_DirEntryType != ST_LINKDIR)

/* ReturnID collecting macros
** every COLLECT_ have to be finished with a REISSUE_
**
** Example:
**
** COLLECT_RETURNIDS;
**
** while(running)
** {
**    ULONG signals;
**    switch (DoMethod(G->App, MUIM_Application_Input, &signals))
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
**    if (running && signals) Wait(signals);
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
#define ISpace(ch)            ((BOOL)((ch) == ' ' || ((ch) >= 9 && (ch) <= 13)))
#define FileExists(f)         FileInfo(f, NULL, NULL, NULL)
#define MyAddHead(l,m)        { (m)->Next = *(l); *(l) = (m); }
#define BuildAddrName2(p)     BuildAddrName((p)->Address, (p)->RealName)
#define SetHelp(o,str)        set(o, MUIA_ShortHelp, GetStr(str))
#define DisposeModulePush(m)  DoMethod(G->App, MUIM_Application_PushMethod, G->App, 3, MUIM_CallHook, &DisposeModuleHook, m)
#define MyStrCpy(a,b)         { strncpy((a),(b), sizeof(a)); (a)[sizeof(a)-1] = 0; }
#define FreeStrBuf(str)       ((str) ? free(((char *)(str))-sizeof(size_t)) : (void)0)

extern int            BusyLevel;
extern struct Hook    GeneralDesHook;
extern struct Hook    DisposeModuleHook;
extern long           PNum;
extern unsigned char  *PPtr[16];

// only prototypes needed for AmigaOS
#if !defined(__MORPHOS__)
Object * STDARGS DoSuperNew(struct IClass *cl, Object *obj, ...);
#endif

// all the utility prototypes
struct Mail *AddMailToList(struct Mail *mail, struct Folder *folder);
APTR     AllocCopy(APTR source, int size);
char *   AllocData2D(struct Data2D *data, size_t initsize);
char *   AllocReqText(char *s);
char *   AllocStrBuf(size_t initlen);
void     AppendLog(int id, char *text, void *a1, void *a2, void *a3, void *a4);
void     AppendLogNormal(int id, char *text, void *a1, void *a2, void *a3, void *a4);
void     AppendLogVerbose(int id, char *text, void *a1, void *a2, void *a3, void *a4);
struct Part *AttachRequest(char *title, char *body, char *yestext, char *notext, int winnum, int mode, APTR parent);
char *   BuildAddrName(char *address, char *name);
void     Busy(char *text, char *parameter, int cur, int max);
BOOL     CheckPrinter(void);
void     ClearMailList(struct Folder *folder, BOOL resetstats);
void     CloseTempFile(struct TempFile *tf);
ULONG    CRC32(void *buffer, unsigned int count, ULONG crc);
ULONG    CompressMsgID(char *msgid);
BOOL     ConvertCRLF(char *in, char *out, BOOL to);
ULONG    ConvertKey(struct IntuiMessage *imsg);
BOOL     CopyFile(char *dest, FILE *destfh, char *sour, FILE *sourfh);
char *   CreateFilename(char *file);
BOOL     CreateDirectory(char *dir);
long     DateStamp2Long(struct DateStamp *date);
void     TimeVal2DateStamp(const struct timeval *tv, struct DateStamp *ds);
void     DateStamp2TimeVal(const struct DateStamp *ds, struct timeval *tv);
char *   TimeVal2String(const struct timeval *tv, enum DateStampType mode);
char *   DateStamp2String(struct DateStamp *date, enum DateStampType mode);
char *   Decrypt(char *source);
void     DeleteMailDir(char *dir, BOOL isroot);
char *   DescribeCT(char *ct);
void     DisplayMailList(struct Folder *fo, APTR lv);
void     DisplayAppIconStatistics(void);
void     DisplayStatistics(struct Folder *fo, BOOL updateAppIcon);
void     DisposeModule(void *modptr);
BOOL     DoPack(char *file, char *newfile, struct Folder *folder);
BOOL     DumpClipboard(FILE *out);
BOOL     EditorToFile(Object *editor, char *file, struct TranslationTable *tt);
char *   Encrypt(char *source);
char *   GetRealPath(char *path);
BOOL     ExecuteCommand(char *cmd, BOOL asynch, BPTR outdef);
char *   ExpandText(char *src, struct ExpandTextData *etd);
void     ExtractAddress(char *line, struct Person *pe);
BOOL     FileInfo(char *filename, int *size, long *bits, long *type);
int      FileSize(char *filename);
BOOL     FileToEditor(char *file, Object *editor);
int      FileType(char *filename);
void     FinishUnpack(char *file);
struct Folder *FolderRequest(char *title, char *body, char *yestext, char *notext,
         struct Folder *exclude, APTR parent);
void     FormatSize(LONG size, char *buffer);
void     FreeBCImage(struct BodyChunkData *bcd);
void     FreeData2D(struct Data2D *data);
struct BodyChunkData *GetBCImage(char *fname);
time_t   GetDateStamp(void);
char *   GetFolderDir(struct Folder *fo);
char *   GetLine(FILE *fh, char *buffer, int bufsize);
char *   GetMailFile(char *string, struct Folder *folder, struct Mail *mail);
struct MailInfo *GetMailInfo(struct Mail *smail);
ULONG    xget(Object *obj, ULONG attr);
BOOL     GetMUICheck(Object *obj);
int      GetMUICycle(Object *obj);
int      GetMUIInteger(Object *obj);
int      GetMUINumer(Object *obj);
int      GetMUIRadio(Object *obj);
void     GetMUIString(char *a, Object *obj);
void     GetMUIText(char *a, Object *obj);
char *   GetNextLine(char *p1);
struct Person *GetReturnAddress(struct Mail *mail);
int      GetSimpleID(void);
char *   GetTZ(void);
void     GotoURL(char *url);
char *   IdentifyFile(char *fname);
void     InfoWindow(char *title, char *body, char *oktext, APTR parent);
void     InsertAddresses(APTR obj, char **addr, BOOL add);
BOOL     isAlNum(int c);
BOOL     IsValidMailFile(char *fname);
char *   itoa(int val);
struct BodyChunkData *LoadBCImage(char *fname);
void     LoadLayout(void);
BOOL     LoadTranslationTable(struct TranslationTable **tt, char *file);
BOOL     MailExists(struct Mail *mailptr, struct Folder *folder);
Object * MakeButton(char *txt);
Object * MakeCheck(char *label);
Object * MakeCheckGroup(Object **check, char *label);
Object * MakeCycle(char **labels, char *label);
Object * MakeInteger(int maxlen, char *label);
Object * MakeMenuitem(const UBYTE *str, ULONG ud);
Object * MakeNumeric(int min, int max, BOOL percent);
Object * MakePassString(char *label);
Object * MakePGPKeyList(APTR *st, BOOL secret, char *label);
Object * MakePicture(char *fname);
Object * MakeStatusFlag(char *fname);
Object * MakeFolderImage(char *fname);
Object * MakeString(int maxlen, char *label);
Object * MakeAddressField(APTR *string, char *label, APTR help, int abmode, int winnum, BOOL allowmulti);
BOOL     MatchNoCase(char *string, char *match);
BOOL     MatchTT(char *charset, struct TranslationTable *tt, BOOL in);
void     MyAddTail(struct Mail **list, struct Mail *new);
char *   MyStrChr(const char *s, int c);
struct TempFile *OpenTempFile(char *mode);
BOOL     AllFolderLoaded(void);
BOOL     PFExists(char *path, char *file);
void     PGPClearPassPhrase(BOOL force);
int      PGPCommand(char *progname, char *options, int flags);
void     PGPGetPassPhrase(void);
void     PlaySound(char *filename);
void     QuoteWordWrap(char *rptr, int lmax, char *prefix, char *firstprefix, FILE *out);
void     RemoveMailFromList(struct Mail *mail);
BOOL     RenameFile(char *oldname, char *newname);
BOOL     RepackMailFile(struct Mail *mail, int dstxpk, char *passwd);
int      ReqFile(enum ReqFileType num, Object *win, char *title, int mode, char *drawer, char *file);
BOOL     SafeOpenWindow(Object *obj);
void     SaveLayout(BOOL permanent);
struct DateStamp *ScanDate(char *date);
int      SelectMessage(struct Mail *mail);
void     SetupToolbar(struct MUIP_Toolbar_Description *tb, char *label, char *help, UWORD flags);
char     ShortCut(char *label);
void     SimpleWordWrap(char *filename, int wrapsize);
void STDARGS SPrintF(char *outstr, char *fmtstr, ...) VARARGS68K;
char *   StartUnpack(char *file, char *newfile, struct Folder *folder);
char *   stccat(char *a, char *b, int n);
char *   StrBufCat(char *strbuf, char *source);
char *   StrBufCpy(char *strbuf, char *source);
char *   AppendToBuffer(char *buf, int *wptr, int *len, char *add);
int      StringRequest(char *string, int size, char *title, char *body,
         char *yestext, char *alttext, char *notext, BOOL secret, APTR parent);
char *   StripUnderscore(char *label);
char *   stristr(const char *a, const char *b);
char *   strtok_r(char **s, char *brk);
char *   SWSSearch(char *str1, char*str2);
BOOL     TransferMailFile(BOOL copyit, struct Mail *mail, struct Folder *dstfolder);
char *   Trim(char *s);
char *   TrimEnd(char *s);
char *   TrimStart(char *s);
BOOL     LoadParsers(void);
void     SParse(char *);
LONG STDARGS YAMMUIRequest(APTR app, APTR win, LONG flags, char *title, char *gadgets, char *format, ...);
APTR     WhichLV(struct Folder *folder);

#endif /* YAM_UTILITIES_H */
