#ifndef YAM_MIME_H
#define YAM_MIME_H

/***************************************************************************

 YAM - Yet Another Mailer
 Copyright (C) 1995-2000 by Marcel Beck <mbeck@yam.ch>
 Copyright (C) 2000-2008 by YAM Open Source Team

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

#include <libraries/codesets.h>

#include "YAM_stringsizes.h"

// codesets.library helper functions
char *strippedCharsetName(const struct codeset* codeset);

// base64 encoding/decoding routines
int base64encode(char *to, const unsigned char *from, unsigned int len);
int base64decode(char *to, const unsigned char *from, unsigned int len);
long base64encode_file(FILE *in, FILE *out, BOOL convLF);
long base64decode_file(FILE *in, FILE *out,
                       struct codeset *srcCodeset, BOOL convCRLF);

// quoted-printable encoding/decoding routines
long qpencode_file(FILE *in, FILE *out);
long qpdecode_file(FILE *in, FILE *out, struct codeset *srcCodeset);

// uucode encoding/decoding routines
long uuencode_file(FILE *in, FILE *out);
long uudecode_file(FILE *in, FILE *out, struct codeset *srcCodeset);

// rfc2047 encoding/decoding routines
int rfc2047_encode_file(FILE *fh, const char *str);
int rfc2047_decode(char *dst, const char *src, unsigned int maxlen);

// rfc1738 URL encoding/decoding routines
int urlencode(char *to, const char *from, unsigned int len);

// rfc2231 encoding/decoding routines
int rfc2231_encode_file(FILE *fh, const char *paramName, const char *str);
int rfc2231_decode(char *attr, char *value, char **result, struct codeset **cs);

// MimeType management
struct MimeTypeNode
{
  struct MinNode node;            // required for placing it into the mimeTypeList
  char ContentType[SIZE_CTYPE];   // IANA conform content-type (e.g. 'application/pdf')
  char Extension[SIZE_NAME];      // space separated string list of extensions
  char Description[SIZE_DEFAULT]; // a short description of the MIME type
  char Command[SIZE_COMMAND];     // command spec for viewing files of that mime Type
};

struct MimeTypeNode *CreateNewMimeType(void);
void FreeMimeTypeList(struct MinList *mimeTypeList);
BOOL CompareMimeTypeLists(const struct MinList *mtl1, const struct MinList *mtl2);

// for maintaing our own internal (hardcoded)
// MimeType array for identifying files. Please make sure
// that the IntMimeTypeID enumeration matches the layout of
// the IntMimeTypes array.
struct IntMimeType
{
  const char *ContentType;  // IANA conform content-type (e.g. 'application/pdf')
  const char *Extension;    // space separated string list of extensions
  const char *Description;  // a short description of the MIME type
};

extern const struct IntMimeType IntMimeTypeArray[];

enum IntMimeTypeID
{
  MT_TX_PLAIN=0, // text/plain
  MT_TX_HTML,    // text/html
  MT_TX_XML,     // text/xml
  MT_TX_GUIDE,   // text/x-aguide
  MT_AP_GZIP,    // application/gzip
  MT_AP_HQX,     // application/mac-binhex40
  MT_AP_XLS,     // application/msexcel
  MT_AP_PPT,     // application/mspowerpoint
  MT_AP_DOC,     // application/msword
  MT_AP_OCTET,   // application/octet-stream
  MT_AP_PS,      // application/postscript
  MT_AP_PDF,     // application/pdf
  MT_AP_PGP,     // application/pgp
  MT_AP_RTF,     // application/rtf
  MT_AP_BZ2,     // application/x-bzip2
  MT_AP_Z,       // application/x-compress
  MT_AP_LHA,     // application/x-lha
  MT_AP_LZX,     // application/x-lzx
  MT_AP_TAR,     // application/x-tar
  MT_AP_TGZ,     // application/x-tar-gz
  MT_AP_AEXE,    // application/x-amiga-executable
  MT_AP_SCRIPT,  // application/x-amigados-script
  MT_AP_REXX,    // application/x-rexx
  MT_AP_ZIP,     // application/zip
  MT_IM_BMP,     // image/bmp
  MT_IM_JPG,     // image/jpeg
  MT_IM_GIF,     // image/gif
  MT_IM_PNG,     // image/png
  MT_IM_TIFF,    // image/tiff
  MT_IM_ILBM,    // image/x-ilbm
  MT_AU_AU,      // audio/basic
  MT_AU_MPEG,    // audio/mpeg
  MT_AU_AIFF,    // audio/x-aiff
  MT_AU_MIDI,    // audio/x-midi
  MT_AU_8SVX,    // audio/x-8svx
  MT_AU_WAV,     // audio/x-wav
  MT_VI_MPG,     // video/mpeg
  MT_VI_MOV,     // video/quicktime
  MT_VI_ANIM,    // video/x-anim
  MT_VI_AVI,     // video/x-msvideo
  MT_ME_EMAIL    // message/rfc822
};

#endif /* YAM_MIME_H */
