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

#include "YAM_stringsizes.h"

#define STR(x)  STR2(x)
#define STR2(x) #x

#ifndef YAM_FOLDERCONFIG_H
struct Folder;
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

#define PGPLOGFILE    "T:PGP.log"
#define NOERRORS      16
#define KEEPLOG       32

// special Macros for the Busy Handling of the InfoBar.
#define BusyEnd               Busy("", NULL, 0, 0)
#define BusySet(c)            Busy(NULL, NULL, c, 0)
#define BusyText(t, p)        Busy(t, p, 0, 0)
#define BusyGauge(t, p, max)  Busy(t, p, 0, max)

#define OUT_DOS       ((BPTR)0)
#define OUT_NIL       ((BPTR)1)

#define ATTREQ_DISP   0
#define ATTREQ_SAVE   1
#define ATTREQ_PRINT  2
#define ATTREQ_MULTI  32

#define ARRAY_SIZE(x) (sizeof(x)/sizeof(x[0]))

extern int            BusyLevel;
extern struct Hook    GeneralDesHook;

struct Mail *AddMailToList(struct Mail *mail, struct Folder *folder);
APTR     AllocCopy(APTR source, int size);
char *   AllocData2D(struct Data2D *data, int initsize);
char *   AllocReqText(char *s);
char *   AllocStrBuf(long initlen);
void     AppendLog(int id, char *text, void *a1, void *a2, void *a3, void *a4);
void     AppendLogNormal(int id, char *text, void *a1, void *a2, void *a3, void *a4);
void     AppendLogVerbose(int id, char *text, void *a1, void *a2, void *a3, void *a4);
struct Part *AttachRequest(char *title, char *body, char *yestext, char *notext, int winnum, int mode, APTR parent);
char *   BuildAddrName(char *address, char *name);
char *   BuildAddrName2(struct Person *pe);
void     Busy(char *text, char *parameter, int cur, int max);
BOOL     CheckPrinter(void);
void     ClearMailList(struct Folder *folder, BOOL resetstats);
void     CloseTempFile(struct TempFile *tf);
int      CompressMsgID(char *msgid);
BOOL     ConvertCRLF(char *in, char *out, BOOL to);
ULONG    ConvertKey(struct IntuiMessage *imsg);
BOOL     CopyFile(char *dest, FILE *destfh, char *sour, FILE *sourfh);
char *   CreateFilename(char *file);
BOOL     CreateDirectory(char *dir);
long     DateStamp2Long(struct DateStamp *date);
char *   DateStamp2String(struct DateStamp *date, enum DateStampType mode);
char *   Decrypt(char *source);
void     DeleteMailDir(char *dir, BOOL isroot);
char *   DescribeCT(char *ct);
void     DisplayMailList(struct Folder *fo, APTR lv);
void     DisplayAppIconStatistics(void);
void     DisplayStatistics(struct Folder *fo, BOOL updateAppIcon);
void     DisposeModulePush(void *module);
void     DisposeModule(void *modptr);
BOOL     DoPack(char *file, char *newfile, struct Folder *folder);
Object * DoSuperNew(struct IClass *cl, Object *obj, ULONG tag1, ...);
BOOL     DumpClipboard(FILE *out);
BOOL     EditorToFile(Object *editor, char *file, struct TranslationTable *tt);
char *   Encrypt(char *source);
BOOL     ExecuteCommand(char *cmd, BOOL asynch, BPTR outdef);
char *   ExpandText(char *src, struct ExpandTextData *etd);
void     ExtractAddress(char *line, struct Person *pe);
BOOL     FileExists(char *filename);
int      FileSize(char *filename);
BOOL     FileToEditor(char *file, Object *editor);
int      FileType(char *filename);
void     FinishUnpack(char *file);
struct Folder *FolderRequest(char *title, char *body, char *yestext, char *notext,
         struct Folder *exclude, APTR parent);
void     FormatSize(int size, char *buffer);
void     FreeBCImage(struct BodyChunkData *bcd);
void     FreeData2D(struct Data2D *data);
void     FreeStrBuf(char *strbuf);
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
BOOL     ISpace(char ch);
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
int      MatchNoCase(char *string, char *match);
BOOL     MatchTT(char *charset, struct TranslationTable *tt, BOOL in);
void     MyAddHead(struct Mail **list, struct Mail *new);
void     MyAddTail(struct Mail **list, struct Mail *new);
char *   MyStrChr(const char *s, int c);
struct TempFile *OpenTempFile(char *mode);
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
void     SetHelp(APTR object, APTR strnum);
void     SetupToolbar(struct MUIP_Toolbar_Description *tb, char *label, char *help, UWORD flags);
char     ShortCut(char *label);
void     SimpleWordWrap(char *filename, int wrapsize);
void     SPrintF(char *outstr, char *fmtstr, ...);
char *   StartUnpack(char *file, char *newfile, struct Folder *folder);
char *   stccat(char *a, char *b, int n);
char *   StrBufCat(char *strbuf, char *source);
char *   StrBufCpy(char *strbuf, char *source);
int      StringRequest(char *string, int size, char *title, char *body,
         char *yestext, char *alttext, char *notext, BOOL secret, APTR parent);
char *   StripUnderscore(char *label);
char *   stristr(const char *a, const char *b);
char *   strtok_r(char **s, char *brk);
BOOL     TransferMailFile(BOOL copyit, struct Mail *mail, struct Folder *dstfolder);
char *   Trim(char *s);
char *   TrimEnd(char *s);
char *   TrimStart(char *s);
APTR     WhichLV(struct Folder *folder);

#define MyStrCpy(a,b) { strncpy((a),(b), sizeof(a)); (a)[sizeof(a)-1] = 0; }

#endif /* YAM_UTILITIES_H */
