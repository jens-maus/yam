/***************************************************************************

 YAM - Yet Another Mailer
 Copyright (C) 1995-2000 Marcel Beck
 Copyright (C) 2000-2011 YAM Open Source Team

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

#include <stdlib.h>
#include <string.h>

#if !defined(__amigaos4__)
#include <clib/alib_protos.h>
#endif

#include <proto/exec.h>

#include "YAM_stringsizes.h"
#include "YAM_utilities.h"

#include "extrasrc.h"

#include "Locale.h"
#include "MimeTypes.h"

#include "Debug.h"

/***************************************************************************
 Module: MIME-TYPE routines
***************************************************************************/

/**** MIME Types/Viewers ****/
/// IntMimeTypeArray[]
const struct IntMimeType IntMimeTypeArray[] =
{
  //                  ContentType                       Extensions          Description
  /* MT_TX_PLAIN */ { "text/plain",                     "txt asc",          MSG_CTtextplain },
  /* MT_TX_HTML  */ { "text/html",                      "html htm shtml",   MSG_CTtexthtml },
  /* MT_TX_XML   */ { "text/xml",                       "xml",              MSG_CTtextxml },
  /* MT_TX_GUIDE */ { "text/x-aguide",                  "guide",            MSG_CTtextaguide },
  /* MT_TX_GZIP  */ { "application/gzip",               "gz",               MSG_CTapplicationgz },
  /* MT_AP_HQX   */ { "application/mac-binhex40",       "hqx",              MSG_CTapplicationhqx },
  /* MT_AP_XLS   */ { "application/msexcel",            "xls xla",          MSG_CTapplicationxls },
  /* MT_AP_PPT   */ { "application/mspowerpoint",       "ppt ppz pps pot",  MSG_CTapplicationppt },
  /* MT_AP_PPT   */ { "application/msword",             "doc dot",          MSG_CTapplicationdoc },
  /* MT_AP_OCTET */ { "application/octet-stream",       "bin exe",          MSG_CTapplicationoctetstream },
  /* MT_AP_PS    */ { "application/postscript",         "ps eps ai",        MSG_CTapplicationpostscript },
  /* MT_AP_PDF   */ { "application/pdf",                "pdf",              MSG_CTapplicationpdf },
  /* MT_AP_PGP   */ { "application/pgp",                "pgp",              MSG_CTapplicationpgp },
  /* MT_AP_PGPSIG*/ { "application/pgp-signature",      "asc sig",          MSG_CTapplicationpgpsig },
  /* MT_AP_RTF   */ { "application/rtf",                "rtf",              MSG_CTapplicationrtf },
  /* MT_AP_BZ2   */ { "application/x-bzip2",            "bz2",              MSG_CTapplicationbz2 },
  /* MT_AP_Z     */ { "application/x-compress",         "z",                MSG_CTapplicationz },
  /* MT_AP_LHA   */ { "application/x-lha",              "lha",              MSG_CTapplicationlha },
  /* MT_AP_LZX   */ { "application/x-lzx",              "lzx",              MSG_CTapplicationlzx },
  /* MT_AP_TAR   */ { "application/x-tar",              "tar gtar",         MSG_CTapplicationtar },
  /* MT_AP_TGZ   */ { "application/x-tar-gz",           "tgz tar.gz",       MSG_CTapplicationtgz },
  /* MT_AP_AEXE  */ { "application/x-amiga-executable", NULL,               MSG_CTapplicationamigaexe },
  /* MT_AP_SCRIPT*/ { "application/x-amigados-script",  NULL,               MSG_CTapplicationadosscript },
  /* MT_AP_REXX  */ { "application/x-rexx",             "rexx rx",          MSG_CTapplicationrexx },
  /* MT_AP_ZIP   */ { "application/zip",                "zip",              MSG_CTapplicationzip },
  /* MT_IM_BMP   */ { "image/bmp",                      "bmp",              MSG_CTimagebmp },
  /* MT_IM_JPG   */ { "image/jpeg",                     "jpg jpeg",         MSG_CTimagejpeg },
  /* MT_IM_GIF   */ { "image/gif",                      "gif",              MSG_CTimagegif },
  /* MT_IM_PNG   */ { "image/png",                      "png",              MSG_CTimagepng },
  /* MT_IM_TIFF  */ { "image/tiff",                     "tif tiff",         MSG_CTimagetiff },
  /* MT_IM_ILBM  */ { "image/x-ilbm",                   "iff ilbm",         MSG_CTimageilbm },
  /* MT_AU_AU    */ { "audio/basic",                    "au snd",           MSG_CTaudiobasic },
  /* MT_AU_MPEG  */ { "audio/mpeg",                     "mp3 mp2",          MSG_CTaudiompeg },
  /* MT_AU_AIFF  */ { "audio/x-aiff",                   "aiff aif aifc",    MSG_CTaudioaiff },
  /* MT_AU_MIDI  */ { "audio/x-midi",                   "midi mid",         MSG_CTaudiomidi },
  /* MT_AU_8SVX  */ { "audio/x-8svx",                   "svx",              MSG_CTaudio8svx },
  /* MT_AU_WAV   */ { "audio/x-wav",                    "wav",              MSG_CTaudiowav },
  /* MT_VI_MPG   */ { "video/mpeg",                     "mpg mpeg",         MSG_CTvideompeg },
  /* MT_VI_MOV   */ { "video/quicktime",                "qt mov",           MSG_CTvideoquicktime },
  /* MT_VI_ANIM  */ { "video/x-anim",                   "anim",             MSG_CTvideoanim },
  /* MT_VI_AVI   */ { "video/x-msvideo",                "avi",              MSG_CTvideomsvideo },
  /* MT_ME_EMAIL */ { "message/rfc822",                 "eml",              MSG_CTmessagerfc822 },
                    { NULL,                             NULL,               NULL }
};

///
/// CreateNewMimeType
//  Initializes a new MIME type structure
struct MimeTypeNode *CreateNewMimeType(void)
{
  struct MimeTypeNode *mt;

  ENTER();

  if((mt = AllocSysObjectTags(ASOT_NODE, ASONODE_Size, sizeof(*mt),
                                         ASONODE_Min, TRUE,
                                         TAG_DONE)) != NULL)
  {
    strlcpy(mt->ContentType, "?/?", sizeof(mt->ContentType));
    mt->Extension[0] = '\0';
    mt->Description[0] = '\0';
    mt->Command[0] = '\0';
  }

  RETURN(mt);
  return mt;
}

///
/// FreeMimeTypeList
void FreeMimeTypeList(struct MinList *mimeTypeList)
{
  struct Node *curNode;

  ENTER();

  // we have to free the mimeTypeList
  while((curNode = RemHead((struct List *)mimeTypeList)) != NULL)
  {
    struct MimeTypeNode *mt = (struct MimeTypeNode *)curNode;

    FreeSysObject(ASOT_NODE, mt);
  }

  NewMinList(mimeTypeList);

  LEAVE();
}

///
/// CompareMimeTypeNodes
static BOOL CompareMimeTypeNodes(const struct Node *n1, const struct Node *n2)
{
  BOOL equal = TRUE;
  const struct MimeTypeNode *mtn1 = (const struct MimeTypeNode *)n1;
  const struct MimeTypeNode *mtn2 = (const struct MimeTypeNode *)n2;

  ENTER();

  // compare every single member of the structure
  if(strcmp(mtn1->ContentType, mtn2->ContentType) != 0 ||
     strcmp(mtn1->Extension,   mtn2->Extension) != 0 ||
     strcmp(mtn1->Description, mtn2->Description) != 0 ||
     strcmp(mtn1->Command,     mtn2->Command) != 0)
  {
    // something does not match
    equal = FALSE;
  }

  RETURN(equal);
  return equal;
}

///
/// CompareMimeTypeLists
// compare two MIME type lists
BOOL CompareMimeTypeLists(const struct MinList *mtl1, const struct MinList *mtl2)
{
  BOOL equal;

  ENTER();

  equal = CompareLists((const struct List *)mtl1, (const struct List *)mtl2, CompareMimeTypeNodes);

  RETURN(equal);
  return equal;
}

///
