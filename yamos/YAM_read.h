#ifndef YAM_READ_H
#define YAM_READ_H

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

#include "SDI_compiler.h"

#include "YAM_addressbook.h"
#include "YAM_mainFolder.h"
#include "YAM_write.h"

// flags & macros for MDN (message disposition notification)
#define MDN_TYPEMASK  0x0F
#define MDN_AUTOACT   0x10
#define MDN_AUTOSEND  0x20
#define isAutoActMDN(v)   (isFlagSet((v), MDN_AUTOACT))
#define isAutoSendMDN(v)  (isFlagSet((v), MDN_AUTOSEND))

// special defines for Part Types
#define PART_ORIGINAL -2
#define PART_ALLTEXT  -1
#define PART_RAW      0
#define PART_LETTER   (C->LetterPart)

// PGP flags & macros
#define PGPE_MIME     (1<<0)
#define PGPE_OLD      (1<<1)
#define hasPGPEMimeFlag(v)     (isFlagSet((v)->encryptionFlags, PGPE_MIME))
#define hasPGPEOldFlag(v)      (isFlagSet((v)->encryptionFlags, PGPE_OLD))

#define PGPS_MIME     (1<<0)
#define PGPS_OLD      (1<<1)
#define PGPS_BADSIG   (1<<2)
#define PGPS_ADDRESS  (1<<3)
#define PGPS_CHECKED  (1<<4)
#define hasPGPSMimeFlag(v)     (isFlagSet((v)->signedFlags, PGPS_MIME))
#define hasPGPSOldFlag(v)      (isFlagSet((v)->signedFlags, PGPS_OLD))
#define hasPGPSBadSigFlag(v)   (isFlagSet((v)->signedFlags, PGPS_BADSIG))
#define hasPGPSAddressFlag(v)  (isFlagSet((v)->signedFlags, PGPS_ADDRESS))
#define hasPGPSCheckedFlag(v)  (isFlagSet((v)->signedFlags, PGPS_CHECKED))

enum MDNType    { MDN_IGNORE=0, MDN_DENY, MDN_READ, MDN_DISP, MDN_PROC, MDN_DELE };
enum ParseMode  { PM_ALL, PM_TEXTS, PM_NONE };
enum ReadInMode { RIM_QUIET, RIM_READ, RIM_EDIT, RIM_QUOTE, RIM_PRINT };
enum HeaderMode { HM_NOHEADER, HM_SHORTHEADER, HM_FULLHEADER };
enum SInfoMode  { SIM_OFF, SIM_DATA, SIM_ALL, SIM_PHOTO };

// ReadMailData structure which carries all necessary information
// during the read mail process. It is used while opening a read
// window, view a mail in the preview section or even scanning
// a mail in background (for Arexx stuff and so on)
// this structure uses a MinNode for making it possible to place
// all existing ReadMailData structs into the global YAM structure
// in an unlimited list allowing unlimited ReadWindows and such.
struct ReadMailData
{
  struct MinNode  node;           // required for placing it into struct Global
  Object          *readWindow;    // ptr to the associated read window or NULL
  Object          *readMailGroup; // ptr to the associated read mail group object
  struct Mail     *mail;          // ptr to the mail we are reading
  struct Part     *firstPart;     // pointer to the first MIME part of the mail
  struct TempFile *tempFile;      // in case of a virtual mail we use a tempfile
  enum ParseMode  parseMode;      // mode in which we parse the mail
  enum HeaderMode headerMode;     // mode on displaying the mail header
  enum SInfoMode  senderInfoMode; // sender info display mode
  short           signedFlags;    // flags for mail signing (i.e. PGP)
  short           encryptionFlags;// flags for encryption modes (i.e. PGP)
  BOOL            hasPGPKey;      // true if mail contains a PGP key
  BOOL            noTextstyles;   // use no Textstyles for displaying the mail
  BOOL            wrapHeaders;    // Wrap the headers if necessary
  BOOL            useFixedFont;   // use a fixed font for displaying the mail

  char readFile[SIZE_PATHFILE];   // filename from which we read the mail from
  char sigAuthor[SIZE_ADDRESS];   // the author of an existing PGP signature
};

struct Part
{
   struct Part         *Prev;
   struct Part         *Next;
   struct Part         *NextSelected;
   struct ReadMailData *rmData;             // ptr to the parent readmail Data
   char                *ContentType;
   char                *ContentDisposition;
   char                *JunkParameter;
   char                *CParName;
   char                *CParFileName;
   char                *CParBndr;
   char                *CParProt;
   char                *CParDesc;
   char                *CParRType;
   char                *CParCSet;
   LONG                 Size;
   int                  MaxHeaderLen;
   int                  Nr;
   BOOL                 HasHeaders;
   BOOL                 Printable;
   BOOL                 Decoded;
   enum Encoding        EncodingCode;

   char                 Name[SIZE_DEFAULT];
   char                 Description[SIZE_DEFAULT];
   char                 Filename[SIZE_PATHFILE];
   char                 Boundary[SIZE_DEFAULT];
};

BOOL  RE_DecodePart(struct Part *rp);
void  RE_DisplayMIME(char *fname, char *ctype);
BOOL  RE_DoMDN(enum MDNType type, struct Mail *mail, BOOL multi);

struct ReadMailData *CreateReadWindow(BOOL forceNewWindow);
struct ReadMailData *AllocPrivateRMData(struct Mail *mail, enum ParseMode pMode);
void FreePrivateRMData(struct ReadMailData *rmData);
BOOL CleanupReadMailData(struct ReadMailData *rmData, BOOL windowCleanup);
BOOL RE_LoadMessage(struct ReadMailData *rmData, enum ParseMode pMode);
char *RE_ReadInMessage(struct ReadMailData *rmData, enum ReadInMode rMode);
void RE_GetSenderInfo(struct Mail *mail, struct ABEntry *ab);
void RE_UpdateSenderInfo(struct ABEntry *old, struct ABEntry *new);
BOOL RE_DownloadPhoto(Object *win, char *url, struct ABEntry *ab);
struct ABEntry *RE_AddToAddrbook(Object *win, struct ABEntry *templ);
void RE_GetSigFromLog(struct ReadMailData *rmData, char *decrFor);
BOOL RE_FindPhotoOnDisk(struct ABEntry *ab, char *photo);
void RE_ClickedOnMessage(char *address);
void RE_PrintFile(char *filename);
struct Mail *RE_GetThread(struct Mail *srcMail, BOOL nextThread, BOOL askLoadAllFolder, Object *readWindow);
BOOL RE_Export(struct ReadMailData *rmData, char *source, char *dest, char *name, int nr, BOOL force, BOOL overwrite, char *ctype);
void RE_SaveAll(struct ReadMailData *rmData, char *path);

#endif /* YAM_READ_H */
