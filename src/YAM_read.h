#ifndef YAM_READ_H
#define YAM_READ_H

/***************************************************************************

 YAM - Yet Another Mailer
 Copyright (C) 1995-2000 Marcel Beck
 Copyright (C) 2000-2025 YAM Open Source Team

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
 YAM OpenSource project    :  https://github.com/jens-maus/yam/

***************************************************************************/

#include "YAM_write.h"

// forward declarations
struct ABookNode;
struct TempFile;

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
#define hasPGPSMimeFlag(v)     (isFlagSet((v)->signedFlags, PGPS_MIME))
#define hasPGPSOldFlag(v)      (isFlagSet((v)->signedFlags, PGPS_OLD))
#define hasPGPSBadSigFlag(v)   (isFlagSet((v)->signedFlags, PGPS_BADSIG))
#define hasPGPSAddressFlag(v)  (isFlagSet((v)->signedFlags, PGPS_ADDRESS))

enum ReadInMode { RIM_QUIET, RIM_READ, RIM_EDIT, RIM_QUOTE, RIM_PRINT, RIM_FORWARD };
enum HeaderMode { HM_NOHEADER, HM_SHORTHEADER, HM_FULLHEADER };
enum SInfoMode  { SIM_OFF, SIM_DATA, SIM_ALL, SIM_IMAGE };
enum SigSepType { SST_BLANK, SST_DASH, SST_BAR, SST_SKIP };

// flags & macros for MDN (message disposition notification)
enum MDNMode    { MDN_MODE_DISPLAY, MDN_MODE_DELETE };
enum MDNAction  { MDN_ACTION_IGNORE, MDN_ACTION_SEND, MDN_ACTION_QUEUED, MDN_ACTION_ASK };

// for parsing a message we have different flags we can specify
// (a),(b),(c) are mutual exclusive
#define PM_ALL    (1<<0)  // (a) parse and keep all parts of the message
#define PM_TEXTS  (1<<1)  // (b) parse and keep only "text/#?" parts of the message
#define PM_NONE   (1<<2)  // (c) parse the msg but keep no additional parts
#define PM_QUIET  (1<<3)  // parse the message without issuing any warning (during filtering)

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
  ULONG           uniqueID;       // a unique identifier for this structure
  enum HeaderMode headerMode;     // mode on displaying the mail header
  enum SInfoMode  senderInfoMode; // sender info display mode
  short           parseFlags;     // flags for the parsing (e.g. be quiet, parse all, etc.)
  short           signedFlags;    // flags for mail signing (i.e. PGP)
  short           encryptionFlags;// flags for encryption modes (i.e. PGP)
  short           letterPartNum;  // the number which was considered the letter part (0=no yet defined)
  BOOL            hasPGPKey;      // true if mail contains a PGP key
  BOOL            useTextcolors;  // use Textcolors for displaying the mail
  BOOL            useTextstyles;  // use Textstyles for displaying the mail
  BOOL            wrapHeaders;    // Wrap the headers if necessary
  BOOL            useFixedFont;   // use a fixed font for displaying the mail

  char readFile[SIZE_PATHFILE];   // filename from which we read the mail from
  char sigAuthor[SIZE_ADDRESS];   // the author of an existing PGP signature
};

// flags for setting certain flags for a struct Part
#define PFLAG_SUBHEADERS    (1<<0)  // has subheaders (headerList filled)
#define PFLAG_PRINTABLE     (1<<1)  // has printable content
#define PFLAG_DECODED       (1<<2)  // part was already decoded
#define PFLAG_ALTPART       (1<<3)  // this part is an alternative part (multipart/alternative)
#define PFLAG_MIME          (1<<4)  // this part conforms to the MIME standard
#define PFLAG_ATTACHMENT    (1<<5)  // this part is explicitly declared as attachment
#define hasSubHeaders(part)     (isFlagSet((part)->Flags, PFLAG_SUBHEADERS))
#define isPrintable(part)       (isFlagSet((part)->Flags, PFLAG_PRINTABLE))
#define isDecoded(part)         (isFlagSet((part)->Flags, PFLAG_DECODED))
#define isAlternativePart(part) (isFlagSet((part)->Flags, PFLAG_ALTPART))
#define isMIMEconform(part)     (isFlagSet((part)->Flags, PFLAG_MIME))
#define isAttachment(part)      (isFlagSet((part)->Flags, PFLAG_ATTACHMENT))

// a struct Part is a structure for managing certain message
// parts according to the hierarchical structuring of e-mails
struct Part
{
  struct Part         *Prev;               // ptr to previous part or NULL
  struct Part         *Next;               // ptr to next part or NULL
  struct Part         *Parent;             // ptr to the parent part or NULL
  struct Part         *NextSelected;       // ptr to next selected or NULL
  struct Part         *MainAltPart;        // ptr to the main alternative part.
  struct ReadMailData *rmData;             // ptr to the parent readmail Data
  struct MinList      *headerList;         // ptr to a list of headers or NULL
  char                *ContentType;        // ptr to the content-type "text/plain"
  char                *ContentDisposition; // ptr to the content-disposition "attachment" or "inline"
  char                *CParName;           // ptr to the content-type "name"
  char                *CParFileName;       // ptr to the content-disposition "filename"
  char                *CParBndr;           // ptr to the content-type "boundary"
  char                *CParProt;           // ptr to the content-type "protocol"
  char                *CParDesc;           // ptr to the content-type "description"
  char                *CParRType;          // ptr to the content-type "report-type"
  char                *CParCSet;           // ptr to the content-type "charset" "iso8859-1"
  long                 Size;               // the calculated size in bytes
  int                  Flags;              // PFLAG_#? flags
  int                  Nr;
  enum Encoding        EncodingCode;
  BOOL                 nameIsArtificial;

  char                 Name[SIZE_DEFAULT];
  char                 Description[SIZE_DEFAULT];
  char                 Filename[SIZE_PATHFILE];
};

struct HeaderNode
{
  struct MinNode node; // required for placing it into struct Part
  char *name;          // the name of the header - without ':'
  char *content;       // the content of the header
};

BOOL RE_DecodePart(struct Part *rp);
void RE_DisplayMIME(const char *srcfile, const char *dstfile, const char *ctype, const BOOL convertFromUTF8);
BOOL RE_ProcessMDN(const enum MDNMode mode, struct Mail *mail, const BOOL multi, const BOOL autoAction, Object *win);

struct ReadMailData *CreateReadWindow(BOOL forceNewWindow);
struct ReadMailData *AllocPrivateRMData(const struct Mail *mail, short parseFlags);
void FreePrivateRMData(struct ReadMailData *rmData);
BOOL CleanupReadMailData(struct ReadMailData *rmData, BOOL fullCleanup);
struct HeaderNode *AllocHeaderNode(void);
void FreeHeaderNode(struct HeaderNode *hdrNode);
void ClearHeaderList(struct MinList *headerList);
struct ReadMailData *GetReadMailData(const struct Mail *mail);
BOOL UpdateReadMailDataStatus(const struct Mail *mail);
char *SuggestPartFileName(const struct Part *part);
BOOL RE_LoadMessage(struct ReadMailData *rmData);
char *RE_ReadInMessage(struct ReadMailData *rmData, enum ReadInMode rMode);
void RE_UpdateSenderInfo(struct ABookNode *old, struct ABookNode *new);
struct ABookNode *RE_AddToAddrbook(Object *win, struct ABookNode *templ);
void RE_GetSigFromLog(struct ReadMailData *rmData, char *decrFor);
void RE_ClickedOnMessage(char *address, Object *win);
BOOL RE_PrintFile(const char *filename, Object *win);
BOOL RE_Export(struct ReadMailData *rmData, const char *source, const char *dest, const char *name, int nr, BOOL force, BOOL overwrite, const char *ctype);
void RE_SaveAll(struct ReadMailData *rmData, const char *path);

#endif /* YAM_READ_H */
